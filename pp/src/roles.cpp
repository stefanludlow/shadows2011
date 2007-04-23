/*------------------------------------------------------------------------\
|  roles.c : coded support for the Role OLC system    www.middle-earth.us | 
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

void
delete_role (CHAR_DATA * ch, char *argument)
{
  int i = 1;
  ROLE_DATA *role;
  bool deleted = false;

  if (!isdigit (*argument))
    {
      send_to_char ("Syntax: role display <role number>\n", ch);
      return;
    }

  // Note: creates small mem leak, but I'm too lazy to bother free'ing right now.

  if (atoi (argument) == 1)
    {
      if (GET_TRUST (ch) == 4 && str_cmp (role_list->poster, ch->tname))
	{
	  send_to_char
	    ("Sorry, but you are only allowed to delete your own roles.\n",
	     ch);
	  return;
	}
      if (!role_list->next)
	role_list = NULL;
      else
	role_list = role_list->next;
      deleted = true;
    }
  else
    for (role = role_list, i = 1; role && role->next; role = role->next, i++)
      {
	if (i + 1 == atoi (argument))
	  {
	    if (GET_TRUST (ch) == 4
		&& str_cmp (role->next->poster, ch->tname))
	      {
		send_to_char
		  ("Sorry, but you are only allowed to delete your own roles.\n",
		   ch);
		return;
	      }
	    role->next = role->next->next;
	    deleted = true;
	    break;
	  }
      }

  if (!deleted)
    {
      send_to_char ("That role number was not found.\n", ch);
      return;
    }

  send_to_char
    ("The specified special role has been removed as an option from chargen.\n",
     ch);
  save_roles ();
}

void
display_role (CHAR_DATA * ch, char *argument)
{
  char output[MAX_STRING_LENGTH];
  int i = 1;
  ROLE_DATA *role;

  if (!isdigit (*argument))
    {
      send_to_char ("Syntax: role display <role number>\n", ch);
      return;
    }

  for (role = role_list; role; role = role->next)
    {
      if (i == atoi (argument))
	break;
      i++;
    }

  if (!role)
    {
      send_to_char ("That role number was not found.\n", ch);
      return;
    }

  sprintf (output, "\n#2Role Summary:#0  %s\n"
	   "#2Posting Admin:#0 %s\n"
	   "#2Posted On:#0     %s\n"
	   "#2Point Cost:#0    %d\n"
	   "\n"
	   "%s", role->summary, role->poster, role->date, role->cost,
	   role->body);

  page_string (ch->desc, output);
}

void
list_roles (CHAR_DATA * ch)
{
  char output[MAX_STRING_LENGTH];
  ROLE_DATA *role;
  int i = 1;

  *output = '\0';

  for (role = role_list; role; role = role->next)
    {
      sprintf (output + strlen (output), "%6d. %s\n", i, role->summary);
      i++;
    }

  if (!*output)
    {
      send_to_char ("No special roles are currently available in chargen.\n",
		    ch);
      return;
    }

  send_to_char
    ("\n#2The following special roles have been made available in chargen:\n#0",
     ch);
  page_string (ch->desc, output);
}

void
post_role (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  ROLE_DATA *role;
  time_t current_time;
  char *date;

  ch = d->character;

  if (!*ch->pc->msg)
    {
      send_to_char ("Role post aborted.\n", ch);
      return;
    }

  if (!role_list)
    {
      CREATE (role_list, ROLE_DATA, 1);
      role = role_list;
    }
  else
    for (role = role_list; role; role = role->next)
      {
	if (!role->next)
	  {
	    CREATE (role->next, ROLE_DATA, 1);
	    role = role->next;
	    break;
	  }
      }

  /* Get a date string from current time ( default = "" ) */
  date = (char *) alloc (256, 31);
  date[0] = '\0';
  current_time = time (0);
  if (asctime_r (localtime (&current_time), date) != NULL)
    {
      date[strlen (date) - 1] = '\0';
    }

  role->cost = ch->delay_info1;
  role->summary = str_dup (ch->delay_who);
  role->body = str_dup (ch->pc->msg);
  role->date = str_dup (date);
  role->poster = str_dup (ch->pc->account_name);
  role->timestamp = (int) time (0);

  ch->delay_who = NULL;
  ch->delay_info1 = 0;

  save_roles ();

  mem_free (date);
}

void
new_role (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cost;

  argument = one_argument (argument, buf);

  if (IS_NPC (ch))
    {
      send_to_char ("Please don't post roles while switched. Thanks!\n", ch);
      return;
    }

  if (!isdigit (*buf))
    {
      send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
      return;
    }
  cost = atoi (buf);

  if (cost < 1 || cost > 50)
    {
      send_to_char ("Permissible costs are from 1-50, inclusive.\n", ch);
      return;
    }

  if (!*argument)
    {
      send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
      return;
    }

  send_to_char
    ("\n#2Enter a detailed summary of the role you wish to post, to\n"
     "give prospective players a better idea as to what sort of RP\n"
     "will be required to successfully portray it.#0\n", ch);

  make_quiet (ch);
  ch->delay_info1 = cost;
  ch->delay_who = str_dup (argument);
  ch->pc->msg = (char *) alloc (sizeof (char), 1);
  *ch->pc->msg = '\0';
  ch->desc->str = &ch->pc->msg;
  ch->desc->max_str = MAX_STRING_LENGTH;
  ch->desc->proc = post_role;
}

void
update_role (CHAR_DATA * ch, char *argument)
{
  ROLE_DATA *role = NULL;
  char buf[MAX_STRING_LENGTH];
  int i = 0;

  argument = one_argument (argument, buf);

  if (!*buf || !isdigit (*buf))
    {
      send_to_char ("Please specify a role number to update.\n", ch);
      return;
    }

  if (atoi (buf) == 1)
    {
      role = role_list;
    }
  else
    for (role = role_list, i = 1; role; role = role->next, i++)
      {
	if (i == atoi (buf))
	  break;
      }

  if (!role)
    {
      send_to_char ("I could not find that role number to update.\n", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!*buf)
    {
      send_to_char ("Please specify which field in the role to update.\n",
		    ch);
      return;
    }
  else if (!str_cmp (buf, "contact"))
    {
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char
	    ("Please specify who the new point of contact should be.\n", ch);
	  return;
	}
      account *acct = new account (buf);
      if (!acct->is_registered ())
	{
	  delete acct;
	  send_to_char
	    ("That is not the name of a currently registered MUD account.\n",
	     ch);
	  return;
	}
      delete acct;
      mem_free (role->poster);
      role->poster = str_dup (CAP (buf));
      send_to_char ("The point of contact for that role has been updated.\n",
		    ch);
      save_roles ();
    }
  else if (!str_cmp (buf, "cost"))
    {
      argument = one_argument (argument, buf);
      if (!*buf || !isdigit (*buf))
	{
	  send_to_char ("Please specify what the new cost should be.\n", ch);
	  return;
	}
      role->cost = atoi (buf);
      send_to_char ("The cost for that role has been updated.\n", ch);
      save_roles ();
    }
}

void
do_role (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (IS_NPC (ch))
    {
      send_to_char ("Sorry, but this can't be done while switched.\n", ch);
      return;
    }

  if (!str_cmp (buf, "new"))
    new_role (ch, argument);
  else if (!str_cmp (buf, "list"))
    list_roles (ch);
  else if (!str_cmp (buf, "display"))
    display_role (ch, argument);
  else if (!str_cmp (buf, "delete"))
    delete_role (ch, argument);
  else if (!str_cmp (buf, "update"))
    update_role (ch, argument);
  else
    send_to_char
      ("Syntax: role (new|list|display|delete|update) <argument(s)>\n", ch);
}

void
save_roles (void)
{
  char text[MAX_STRING_LENGTH];
  char date_buf[MAX_STRING_LENGTH];
  char summary[MAX_STRING_LENGTH];
  ROLE_DATA *role;

  mysql_safe_query ("DELETE FROM special_roles");

  for (role = role_list; role; role = role->next)
    {
      *summary = '\0';
      *date_buf = '\0';
      *text = '\0';

      mysql_safe_query
	("INSERT INTO special_roles (summary, poster, date, cost, body, timestamp) VALUES ('%s', '%s', '%s', %d, '%s', %d)",
	 role->summary, role->poster, role->date, role->cost, role->body,
	 role->timestamp);
    }
}

void
reload_roles (void)
{
  ROLE_DATA *role = NULL;
  MYSQL_RES *result;
  MYSQL_ROW row;

  mysql_safe_query ("SELECT * FROM special_roles");
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      system_log ("No roles stored in database.", false);
      return;
    }

  while ((row = mysql_fetch_row (result)))
    {
      if (!role_list)
	{
	  CREATE (role_list, ROLE_DATA, 1);
	  role = role_list;
	}
      else
	{
	  CREATE (role->next, ROLE_DATA, 1);
	  role = role->next;
	}
      role->summary = str_dup (row[0]);
      role->poster = str_dup (row[1]);
      role->date = str_dup (row[2]);
      role->cost = atoi (row[3]);
      role->body = str_dup (row[4]);
      role->timestamp = atoi (row[5]);
      role->next = NULL;
    }

  mysql_free_result (result);
}
