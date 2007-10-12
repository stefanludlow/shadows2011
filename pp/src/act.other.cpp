/*------------------------------------------------------------------------\
|  act.other.c : Miscellaneous Module                 www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

extern rpie::server engine;

SECOND_AFFECT *second_affect_list = NULL;
extern int keeper_has_money (CHAR_DATA * keeper, int cost);
extern void keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money);
extern void subtract_keeper_money (CHAR_DATA * keeper, int cost);

const char *direct[] =
    { "north", "east", "south", "west", "up", "down" };


int
is_in_cell (CHAR_DATA * ch, int zone)
{
  CHAR_DATA *jailer;

  if (!zone_table[zone].jailer)
    return 0;

  jailer = vtom (zone_table[zone].jailer);

  if (!jailer)
    return 0;

  if (ch->in_room == jailer->cell_1)
    return 1;
  else if (ch->in_room == jailer->cell_2)
    return 1;
  else if (ch->in_room == jailer->cell_3)
    return 1;

  return 0;
}


void
do_commence (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch, *tch_next;
  DESCRIPTOR_DATA *td;
  int from_room = 0, to_room = 0;
  bool morgul = false;
  char buf[MAX_STRING_LENGTH];

  if (!IS_SET (ch->plr_flags, NEWBIE))
    {
      send_to_char ("It appears that you've already begun play...\n", ch);
      return;
    }

  if (IS_SET (ch->flags, FLAG_GUEST))
    {
      send_to_char ("Sorry, but guests are not allowed into the gameworld.\n",
		    ch);
      return;
    }

  act
    ("$n decides to take the plunge, venturing off into the world for the very first time!",
     true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  sprintf (buf, "\n#6Welcome to %s!#0\n\n", MUD_NAME);
  send_to_char (buf, ch);

  if (get_queue_position (ch) != -1)
    update_assist_queue (ch, true);

  from_room = ch->in_room;

  if (IS_SET (ch->plr_flags, START_MORDOR))
    {
      char_from_room (ch);
      char_to_room (ch, MINAS_MORGUL_START_LOC);
      // char_to_room (ch, EDEN_START_LOC);
      ch->was_in_room = 0;
      add_clan (ch, "mordor_char", CLAN_MEMBER);
      if (!ch->skills[SKILL_SPEAK_BLACK_SPEECH]
	  || ch->skills[SKILL_SPEAK_BLACK_SPEECH] < 30)
	{
	  ch->skills[SKILL_SPEAK_BLACK_SPEECH] = 30;
	  ch->pc->skills[SKILL_SPEAK_BLACK_SPEECH] = 30;
	}
      morgul = true;
    }
  else
    {
      char_from_room (ch);
      char_to_room (ch, MINAS_TIRITH_START_LOC);
      ch->was_in_room = 0;
      add_clan (ch, "mt_citizens", CLAN_MEMBER);
      if (!ch->skills[SKILL_SPEAK_WESTRON]
	  || ch->skills[SKILL_SPEAK_WESTRON] < 30)
	{
	  ch->skills[SKILL_SPEAK_WESTRON] = 30;
	  ch->pc->skills[SKILL_SPEAK_WESTRON] = 30;
	}
    }

  ch->plr_flags &= ~NEWBIE;
  ch->time.played = 0;
  do_save (ch, "", 0);

  act ("$n has entered Middle-earth for the very first time!", true, ch, 0, 0,
       TO_ROOM | _ACT_FORMAT);
  sprintf (buf,
	   "#3[%s has entered Middle-earth for the first time at the %s.]#0",
	   char_short (ch), (morgul) ? "Tur Edendor settlement" : "Wizard's Cafe");

  for (td = descriptor_list; td; td = td->next)
    {
      if (!td->character || td->connected != CON_PLYNG)
	continue;
      if (!is_brother (ch, td->character))
	continue;
      if (IS_SET (td->character->plr_flags, MENTOR))
	{
	  act (buf, true, ch, 0, td->character, TO_VICT | _ACT_FORMAT);
	}
    }

  // Delete auto-genned individual debriefing area, if applicable.

  if (from_room >= 100000 && vtor (from_room) &&
      !str_cmp (vtor (from_room)->name, PREGAME_ROOM_NAME))
    {

      for (tch = (vtor (from_room))->people; tch; tch = tch_next)
	{
	  tch_next = tch->next_in_room;
	  if (!IS_NPC (tch))
	    {
	      char_from_room (tch);
	      if ((to_room = tch->was_in_room) && vtor (to_room))
		char_to_room (tch, to_room);
	      else
		char_to_room (tch, OOC_LOUNGE);
	    }
	}

      delete_contiguous_rblock (vtor (from_room), -1, -1);
    }

  do_look (ch, NULL, 0);
}

void
do_ic (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];

  if (!IS_SET (ch->room->room_flags, OOC))
    {
      send_to_char ("This isn't an OOC area. Find your way out, IC.\n", ch);
      return;
    }

  if (IS_SET (ch->flags, FLAG_GUEST))
    {
      send_to_char ("Sorry, but guests are not allowed into the gameworld.\n",
		    ch);
      return;
    }

  if (!ch->was_in_room)
    {
      send_to_char ("Hmm... I'm not quite sure where to drop you. You'd"
		    " best petition for assistance. Sorry!\n", ch);
      return;
    }

  if (ch->was_in_room == -1)
    {
      send_to_char ("You'll need to begin play via the COMMENCE command.\n",
		    ch);
      return;
    }

  sprintf (buf, "Weary of the OOC chatter, #5%s#0 returns to Middle-earth.",
	   char_short (ch));
  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  send_to_char ("Weary of the OOC chatter, you return to Middle-earth.\n\n",
		ch);

  char_from_room (ch);
  char_to_room (ch, ch->was_in_room);
  ch->was_in_room = 0;

  sprintf (buf, "%s#0 enters the area from the OOC lounge.)",
	   char_short (ch));
  *buf = toupper (*buf);
  sprintf (buffer, "(#5%s", buf);
  sprintf (buf, "%s", buffer);

  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  do_look (ch, "", 0);

}

void
delayed_ooc (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];

  send_to_char ("You are now exiting to the OOC lounge...\n", ch);
  send_to_char
    ("\n#1Remember, the use of this lounge is a privilege -- do not abuse it!\n"
     "To return to the in-character parts of the grid, use the IC command.#0\n\n",
     ch);

  sprintf (buf, "%s#0 has exited to the OOC lounge.)", char_short (ch));
  *buf = toupper (*buf);
  sprintf (buffer, "(#5%s", buf);
  sprintf (buf, "%s", buffer);

  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  ch->was_in_room = ch->room->nVirtual;
  char_from_room (ch);
  char_to_room (ch, OOC_LOUNGE);

  sprintf (buf, "%s#0 has entered the OOC lounge.", char_short (ch));
  *buf = toupper (*buf);
  sprintf (buffer, "#5%s", buf);
  sprintf (buf, "%s", buffer);

  act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

  do_look (ch, "", 0);

  ch->delay = 0;
}

void
do_quit (CHAR_DATA * ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  int dwelling = 0, outside = 0;
  bool block = false;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *tch;
  DESCRIPTOR_DATA *d;

  if (ch->desc && ch->desc->original)
    {
      send_to_char ("Not while you are switched.  RETURN first.\n\r", ch);
      return;
    }

  if (is_switched (ch))
    return;

  if (IS_NPC (ch))
    {
      send_to_char ("NPC's cannot quit.\n\r", ch);
      return;
    }

  if (ch->in_room == 5142 || ch->in_room == 5119 || ch->in_room == 5196 ||
      ch->in_room == 5197 || ch->in_room == 5198 || ch->in_room == 5199)
    {
      if (IS_MORTAL (ch))
	{
	  send_to_char ("You can't quit out in the Arena!\n", ch);
	  return;
	}
    }


  
if (ch->in_room == 66955 || ch->in_room == 66956 || ch->in_room == 66954 || ch->in_room == 66953)
    {
      if (IS_MORTAL (ch))
	{
	  send_to_char ("You can't quit out in the Pit!\n", ch);
	  return;
	}
    }
    
  argument = one_argument (argument, arg);

  if (!IS_SET (ch->flags, FLAG_GUEST) && IS_MORTAL (ch)
      && !IS_SET (ch->room->room_flags, SAFE_Q) && ch->desc && cmd != 3)
    {
      send_to_char
	("You may not quit in this area. Most inns, taverns, and so forth are\n"
	 "designated as safe-quit areas; if you are in the wilderness, you may\n"
	 "also use the CAMP command to create a safe-quit room. #1If you simply\n"
	 "drop link without quitting, your character will remain here, and may\n"
	 "be harmed or injured while you are away. We make no guarantees!#0\n",
	 ch);
      return;
    }

  if (GET_POS (ch) == POSITION_FIGHTING && !IS_SET (ch->flags, FLAG_GUEST))
    {
      send_to_char ("You can't quit during combat!\n", ch);
      return;
    }

  if (IS_SUBDUEE (ch))
    {
      act ("$N won't let you leave!", false, ch, 0, ch->subdue, TO_CHAR);
      return;
    }

  if (IS_MORTAL (ch) &&
      IS_SET (ch->room->room_flags, LAWFUL) &&
      get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone) && cmd != 3)
    {
      send_to_char ("You may not exit the game here.  While a criminal in "
		    "this area you may not quit\nin lawful places.\n", ch);
      return;
    }

  if (IS_HITCHER (ch) && !cmd)
    {
      send_to_char
	("You cannot quit out with a hitched mount; try CAMPING or the STABLE.\n",
	 ch);
      return;
    }

  if (IS_SUBDUER (ch))
    do_release (ch, "", 0);

  if (!IS_NPC (ch) && ch->pc->edit_player)
    {
      unload_pc (ch->pc->edit_player);
      ch->pc->edit_player = NULL;
    }

  remove_affect_type (ch, MAGIC_SIT_TABLE);

  act ("Goodbye, friend.. Come back soon!", false, ch, 0, 0, TO_CHAR);
  act ("$n leaves the area.", true, ch, 0, 0, TO_ROOM);

  sprintf (s_buf, "%s has left the game.\n", GET_NAME (ch));

  if (!ch->pc->admin_loaded)
    send_to_gods (s_buf);

  d = ch->desc;

  sprintf (s_buf, "%s is quitting.  Saving character.", GET_NAME (ch));
  system_log (s_buf, false);

  if (ch->pc)			/* Not sure why we wouldn't have a pc, but being careful */
    ch->pc->last_logoff = time (0);

  if (ch->in_room >= 100000 && vtor (ch->in_room)
      && (vtor (ch->in_room))->dir_option[OUTSIDE] != NULL)
    {
      for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
	  if (tch != ch && IS_MORTAL (tch))
	    block = true;
	}
      dwelling = ch->in_room;
      outside = ch->was_in_room;
      if (ch->room->dir_option[OUTSIDE]
	  && !IS_SET (ch->room->dir_option[OUTSIDE]->exit_info, EX_CLOSED))
	block = true;
      if (!block)
	{
	  for (obj = (vtor (outside))->contents; obj; obj = obj->next_content)
	    {
	      if (obj->o.od.value[0] == dwelling
		  && GET_ITEM_TYPE (obj) == ITEM_DWELLING
		  && IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC_DWELLING))
		{
		  obj->obj_flags.extra_flags |= ITEM_VNPC;
		  sprintf (buf,
			   "#2%s#0 grows still as its occupants settle in.",
			   obj_short_desc (obj));
		  buf[2] = toupper (buf[2]);
		  send_to_room (buf, obj->in_room);
		  break;
		}
	    }
	}
    }

  extract_char (ch);

  ch->desc = NULL;

  if (!d)
    return;

  d->character = NULL;

  if (!d->acct || str_cmp(d->acct->name.c_str (), "Guest") == 0)
    {
      close_socket (d);
      return;
    }

  d->connected = CON_ACCOUNT_MENU;
  nanny (d, "");
}

void
do_save (CHAR_DATA * ch, char *argument, int cmd)
{
  char *p;
  char buf[MAX_STRING_LENGTH];

  if (GET_TRUST (ch))
    {
      p = one_argument (argument, buf);

      if (*buf)
	{
	  save_document (ch, argument);
	  return;
	}
    }

  if (IS_NPC (ch))
    return;

  sprintf (buf, "Saving %s.\n\r", GET_NAME (ch));
  send_to_char (buf, ch);
  save_char (ch, true);
}

void
do_sneak (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int dir;

  if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
      send_to_char ("You cannot do this in an OOC area.\n", ch);
      return;
    }

  if (!real_skill (ch, SKILL_SNEAK))
    {
      send_to_char ("You just aren't stealthy enough to try.\n", ch);
      return;
    }

  if (IS_SUBDUER (ch))
    {
      send_to_char ("You can't sneak while you have someone in tow.\n", ch);
      return;
    }

  if (IS_ENCUMBERED (ch))
    {
      send_to_char ("You are too encumbered to sneak.\n\r", ch);
      return;
    }

  if (IS_SWIMMING (ch))
    {
      send_to_char ("You can't do that while swimming!\n", ch);
      return;
    }

  if (ch->speed == SPEED_JOG ||
      ch->speed == SPEED_RUN || ch->speed == SPEED_SPRINT)
    {
      sprintf (buf, "You can't sneak and %s at the same time.\n",
	       speeds[ch->speed]);
      send_to_char (buf, ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!*buf)
    {

      if (IS_NPC (ch) && IS_SET (ch->affected_by, AFF_SNEAK))
	{
	  if (cmd == 0)
	    sprintf (buf, "north");
	  if (cmd == 1)
	    sprintf (buf, "east");
	  if (cmd == 2)
	    sprintf (buf, "south");
	  if (cmd == 3)
	    sprintf (buf, "west");
	  if (cmd == 4)
	    sprintf (buf, "up");
	  if (cmd == 5)
	    sprintf (buf, "down");
	}
      else
	{
	  send_to_char ("Sneak in what direction?\n\r", ch);
	  return;
	}

    }

  if ((dir = index_lookup (dirs, buf)) == -1)
    {
      send_to_char ("Sneak where?\n\r", ch);
      return;
    }

  if (!CAN_GO (ch, dir))
    {
      if (ch->room->extra && ch->room->extra->alas[dir])
	send_to_char (ch->room->extra->alas[dir], ch);
      else
	send_to_char ("There is no exit that way.\n", ch);
      return;
    }

  // Heads up to the player sneaking into a no-hide room, 
  // as long as they can see inside
  ROOM_DATA * dest = vtor (ch->room->dir_option[dir]->to_room);
  if (dest && IS_SET (dest->room_flags, NOHIDE) && *argument != '!'
      && (IS_LIGHT (dest) 
	  || get_affect (ch, MAGIC_AFFECT_INFRAVISION)
	  || IS_SET (ch->affected_by, AFF_INFRAVIS)))
    {
      char message [AVG_STRING_LENGTH] = "";
      sprintf (message, "   As you quietly approach the area ahead, "
	       "you notice that there \nare no hiding places available.  "
	       "If you wish to sneak out of this \narea into the one "
	       "ahead, then: #6 sneak %s !#0\n", buf); 
      send_to_char (message, ch);
      return;
    }


  skill_use (ch, SKILL_SNEAK, 0);

  if (odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) >= number (1, 100)
      || !would_reveal (ch))
    {
      magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
    }
  else
    {
      remove_affect_type (ch, MAGIC_HIDDEN);
      act ("$n attempts to be stealthy.", true, ch, 0, 0, TO_ROOM);
    }

  do_move (ch, "", dir);
}

void
do_hood (CHAR_DATA * ch, char *argument, int cmd)
{
  OBJ_DATA *obj;

  if (!(obj = get_equip (ch, WEAR_NECK_1))
      || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
    {

      if (!(obj = get_equip (ch, WEAR_NECK_2))
	  || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{

	  if (!(obj = get_equip (ch, WEAR_ABOUT))
	      || (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	    {
	      send_to_char ("You are not wearing a hooded item.\n\r", ch);
	      return;
	    }
	}
    }

  if (!IS_SET (ch->affected_by, AFF_HOODED))
    {

      act ("You raise $p's hood, obscuring your face.", false, ch, obj, 0,
	   TO_CHAR | _ACT_FORMAT);

      act ("$n raises $p's hood, concealing $s face.",
	   false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

      ch->affected_by |= AFF_HOODED;

      return;
    }

  ch->affected_by &= ~AFF_HOODED;

  act ("You lower $p's hood, revealing your features.", false, ch, obj, 0,
       TO_CHAR | _ACT_FORMAT);
  act ("$n lowers $p's hood, revealing $s features.", false, ch, obj, 0,
       TO_ROOM | _ACT_FORMAT);

  return;
}

void
do_hide (CHAR_DATA * ch, char *argument, int cmd)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
      send_to_char ("You cannot do this in an OOC area.\n", ch);
      return;
    }

  if (!real_skill (ch, SKILL_HIDE))
    {
      send_to_char ("You lack the skill to hide.\n", ch);
      return;
    }

  if (IS_SET (ch->room->room_flags, NOHIDE))
    {
      send_to_char ("This room offers no hiding spots.\n", ch);
      return;
    }

  if (ch->aiming_at)
    {
      send_to_char ("You loose your aim as you move to conceal yourself.\n", ch);
      ch->aiming_at->targeted_by = NULL;
      ch->aiming_at = NULL;
      ch->aim = 0;
      return;
    }
    
  argument = one_argument (argument, buf);

  if (*buf)
    {

      if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
	  !(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
	  send_to_char ("You don't have that.\n", ch);
	  return;
	}

      if (!get_obj_in_list_vis (ch, buf, ch->right_hand) &&
	  !get_obj_in_list_vis (ch, buf, ch->left_hand))
	{
	  act ("It is too dark to hide it.", false, ch, 0, 0, TO_CHAR);
	  return;
	}

      act ("You begin looking for a hiding spot for $p.",
	   false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);

      ch->delay_type = DEL_HIDE_OBJ;
      ch->delay_info1 = (long int) obj;
      ch->delay = 5;

      return;
    }

  if (IS_ENCUMBERED (ch))
    {
      send_to_char ("You are too encumbered to hide.\n", ch);
      return;
    }

  send_to_char ("You start trying to conceal yourself.\n", ch);

  ch->delay_type = DEL_HIDE;
  ch->delay = 5;
}

void
delayed_hide (CHAR_DATA * ch)
{
  int mod = 0;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj = NULL;

  ch->delay_type = 0;

  send_to_char ("You settle down in what looks like a good spot.\n", ch);

  switch (ch->room->sector_type)
    {
    case SECT_INSIDE:
      mod = 0;
      break;
    case SECT_CITY:
      mod = 0;
      break;
    case SECT_ROAD:
      mod = 10;
      break;
    case SECT_TRAIL:
      mod = 10;
      break;
    case SECT_FIELD:
      mod = 20;
      break;
    case SECT_WOODS:
      mod = -10;
      break;
    case SECT_FOREST:
      mod = -20;
      break;
    case SECT_HILLS:
      mod = -20;
      break;
    case SECT_MOUNTAIN:
      mod = -20;
      break;
    case SECT_SWAMP:
      mod = -10;
      break;
    case SECT_PASTURE:
      mod = 20;
      break;
    case SECT_HEATH:
      mod = 20;
      break;
    }

  if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_LIGHT)
    obj = ch->right_hand;

  if (obj)
    {
      if (obj->o.light.hours > 0 && obj->o.light.on)
	{

	  act ("You put out $p so you won't be detected.",
	       false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	  act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	  obj->o.light.on = 0;
	}
      obj = NULL;
    }

  if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_LIGHT)
    obj = ch->left_hand;

  if (obj)
    {
      if (obj->o.light.hours > 0 && obj->o.light.on)
	{

	  act ("You put out $p so you won't be detected.",
	       false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	  act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	  obj->o.light.on = 0;
	}
      obj = NULL;
    }

  for (obj = ch->equip; obj; obj = obj->next_content)
    {
      if (obj->obj_flags.type_flag != ITEM_LIGHT)
	continue;

      if (obj->o.light.hours > 0 && obj->o.light.on)
	{

	  act ("You put out $p so you won't be detected.",
	       false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	  act ("$n put out $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	  obj->o.light.on = 0;
	}
    }

  if (get_affect (ch, MAGIC_HIDDEN))
    {
      room_light (ch->room);
      return;
    }

  if (skill_use (ch, SKILL_HIDE, mod))
    {
      magic_add_affect (ch, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
      sprintf (buf, "[%s hides]", ch->tname);
      act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
    }

  room_light (ch->room);
}

void
delayed_hide_obj (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  AFFECTED_TYPE *af;

  obj = (OBJ_DATA *) ch->delay_info1;

  if (obj != ch->right_hand && obj != ch->left_hand)
    {
      send_to_char ("You don't have whatever you were hiding anymore.\n", ch);
      return;
    }

  obj_from_char (&obj, 0);

  remove_obj_affect (obj, MAGIC_HIDDEN);	/* Probably doesn't exist */

  af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

  af->type = MAGIC_HIDDEN;
  af->a.hidden.duration = -1;
  af->a.hidden.hidden_value = ch->skills[SKILL_HIDE];
  af->a.hidden.coldload_id = ch->coldload_id;

  act ("You hide $p.", false, ch, obj, 0, TO_CHAR);

  af->next = obj->xaffected;
  obj->xaffected = af;
  obj_to_room (obj, ch->in_room);

  act ("$n hides $p.", false, ch, obj, 0, TO_ROOM);
}

/**********************************************************************
 * CASE 1:
 * Usage: palm <item>
 *        will get item from room. 
 *             Uses Sleight
 *
 * CASE 2:
 * Usage: palm <item> into <container> 
 *        will put item into container in the room, including tables
 *              Uses Sleight
 *
 * CASE 3:
 * Usage: palm <item> into <targetPC> <contaienr>
 *        will put item into container worn by PC (target or self)
 *             Uses Steal if PC is target
 *             Uses Sleight if PC is self
 *
 * CASE 4:
 * Usage: palm <item> from <container>
 *        takes item from container worn by PC
 *             Uses Sleight
 *
 * CASE 5:
 * Usage: palm <item> from <targetPC> <container>
 *        takes item from container worn by PC
 *             Uses Sleight if PC is self
 *
 * Note: Must use STEAL command if taking from a container worn
 * by targetPC.
 *
 **********************************************************************/

void
do_palm (CHAR_DATA * ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char contbuf[MAX_STRING_LENGTH];
  char msgbuf[MAX_STRING_LENGTH];
  char objtarget[MAX_STRING_LENGTH];
  CHAR_DATA *tch;
  OBJ_DATA *cobj = NULL;  //container
  OBJ_DATA *tobj = NULL;  //item being palmed
  AFFECTED_TYPE *af;
  int modifier = 0;
  bool into = false;
  bool from = false;
  bool targchk = false;  //is there a target PC?
  bool contchk = false;  //is there a container?

  if (IS_SWIMMING (ch))
    {
      send_to_char ("You can't do that while swimming!\n", ch);
      return;
    }

  if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
      send_to_char ("You cannot do this in an OOC area.\n", ch);
      return;
    }

  if (!real_skill (ch, SKILL_STEAL) && !real_skill (ch, SKILL_SLEIGHT))
    {
      send_to_char ("You lack the skill to palm objects.\n\r", ch);
      return;
    }


 argument = one_argument (argument, objtarget);

  if (!*objtarget)
    {
      send_to_char ("What did you wish to palm?\n", ch);
      return;
    }
    
	if (ch->right_hand && ch->left_hand)
				{
					send_to_char
						("One of your hands needs to be free before attempting to palm something.\n",
						 ch);
					return;
				}

  argument = one_argument (argument, buf); //into or from

  if (!*buf)
    {				
/****** CASE 1: palm <item> (from room) *******/
			if (!(tobj = get_obj_in_list_vis (ch, objtarget, ch->room->contents)))
				{
					send_to_char ("You don't see that here.\n", ch);
					return;
				}

			if (tobj->obj_flags.weight / 100 > 3)
				{
					send_to_char ("That's too heavy for you to pick up very stealthily.\n", ch);
						return;
				}

			if (!IS_SET (tobj->obj_flags.wear_flags, ITEM_TAKE))
	{
	  send_to_char ("That cannot be picked up.\n", ch);
	  return;
	}
			
			obj_from_room (&tobj, 0);
			clear_omote (tobj);
			act ("You carefully attempt to palm $p.", false, ch, tobj, 0, 
			TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf, "#3[Guardian: %s%s]#0 Tries to palm %s in %d.",
			GET_NAME (ch),
			IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
			tobj->short_description,
			ch->in_room);
			send_to_guardians (msgbuf, 0xFF);

			if (!skill_use (ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
				{
					act ("$n attempts to surreptitiously take $p.", false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
				}
			
			obj_to_char (tobj, ch);
			return;
		} //CASE 1:


/****** CASE 2, 3, 4 & 5: 
	palm <item> [from | into] [target?] <container>
*****/		
      if (!str_cmp (buf, "into"))
	{
				//container/target name
					argument = one_argument (argument, contbuf); 
	  into = true;
	}
      else if (!str_cmp (buf, "from"))
	{
				//container/target name
					argument = one_argument (argument, contbuf); 
	  from = true;
	}
      else
	{
					if (!from & !into)
						{
							send_to_char ("Do you wish to palm INTO or FROM a container?\n", ch);
							return;
						}
				}

		if (!(tch = get_char_room_vis (ch, contbuf))) //contbuf is not PC, so it may be a container CASE 2 or 4
			{
				if (!(cobj = get_obj_in_list (contbuf, ch->equip)) && !(cobj = get_obj_in_list_vis (ch, contbuf, ch->room->contents)))
					{
						send_to_char ("You don't see that here.\n", ch);
						return;
					}
				
				if (cobj == get_obj_in_list (contbuf, ch->equip))
					{
						tch =  get_char_room_vis (ch, "self");
						targchk = true; //self is the target PC CASE 3 & 5
						contchk = true;
					}
				
				if (cobj == get_obj_in_list_vis (ch, contbuf, ch->room->contents))
					{
						targchk = false;
						contchk = true;
					}
					
				if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
					{
						send_to_char ("You can only palm items into or from containers.\n", ch);
						return;
					}
				
			}
		else
			{
				targchk = true; //there is a target PC CASE 3 & 5				
			}

/*** CASE 2: palm <item> into <container> ***/
// item is tobj
// cobj is container
		if (into && !targchk && contchk)
			{
				if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
		       && !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
					{
						send_to_char ("What did you wish to palm into it?\n", ch);
						return;
					}
				
				if (tobj->obj_flags.weight / 100 > 3)
					{
						send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
						return;
					}
					
				//Treat tables as a special container
	      if (!IS_SET (cobj->obj_flags.extra_flags, ITEM_TABLE))
					{
						act ("You carefully slide $p onto $P.", false, ch, tobj,
								 cobj, TO_CHAR | _ACT_FORMAT);
						
						/* Alert the staff of the theft */
						sprintf (msgbuf,
							 "#3[Guardian: %s%s]#0 Tries to slip %s onto %s in %d.",
							 GET_NAME (ch),
							 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
							 tobj->short_description,
							 cobj->short_description,
							 ch->in_room);
						send_to_guardians (msgbuf, 0xFF);
			
						if (!skill_use
								(ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
							act ("$n attempts to surreptitiously place $p atop $P.",
						 false, ch, tobj, cobj, TO_ROOM | _ACT_FORMAT);
					}	
				else
					{
						act ("You carefully slide $p into $P.", false, ch, tobj,
								 cobj, TO_CHAR | _ACT_FORMAT);
						
						/* Alert the staff of the theft */
						sprintf (msgbuf,
							 "#3[Guardian: %s%s]#0 Tries to slip %s into %s in %d.",
							 GET_NAME (ch),
							 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
							 tobj->short_description,
							 cobj->short_description,
							 ch->in_room);
						send_to_guardians (msgbuf, 0xFF);
			
						if (!skill_use
								(ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
							act ("$n attempts to surreptitiously place $p into $P.",
						 false, ch, tobj, cobj, TO_ROOM | _ACT_FORMAT);
					}
				
				//transfer any special effects
				for (af = tobj->xaffected; af; af = af->next)
					affect_modify (ch, af->type, af->a.spell.location,
			       af->a.spell.modifier, tobj->obj_flags.bitvector,
			       false, 0);
	      
	      if (ch->right_hand == tobj)
					ch->right_hand = NULL;
	      else if (ch->left_hand == tobj)
					ch->left_hand = NULL;
	      obj_to_obj (tobj, cobj);
				return;		
			}//CASE 2

/*** CASE 3: palm <item> into <targetPC> <container> ***/
// item is tobj
// tch is targetPC 
// cobj is container
 		if (into && targchk)
 			{
 				argument = one_argument (argument, contbuf);
 				
 				if (!contchk)
 					{
						if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
							{
								send_to_char ("You don't see that container.\n", ch);
								return;
							}
						
						if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
							{
								send_to_char ("You can only palm items into containers.\n", ch);
								return;
							}
					}
						
				if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
		       && !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
					{
						send_to_char ("What did you wish to palm into it?\n", ch);
						return;
					}
				
				if (tobj->obj_flags.weight / 100 > 3)
					{
						send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
						return;
					}
					
 				if (tch == ch)
 					{
 						act ("You carefully slide $p into $P.", false, ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);
		   
 						 /* Alert the staff of the theft */
						sprintf (msgbuf,
							 "#3[Guardian: %s%s]#0 Tries to secretly place %s into %s in %d.",
							 GET_NAME (ch),
							 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
							 tobj->short_description,
							 cobj->short_description,
							 ch->in_room);
						send_to_guardians (msgbuf, 0xFF);
		
						
						if (!skill_use (ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
							act ("$n attempts to surreptitiously manipulate $p.",
								false, ch, tobj, 0, TO_ROOM | _ACT_FORMAT);
								
						
 					}
 				else
 					{
 						modifier = tch->skills[SKILL_SCAN] / 5;
						modifier += tobj->obj_flags.weight / 100;
						modifier += 15;
						
						sprintf (msgbuf, "You carefully slide $p into #5%s#0's $P.",
		   char_short (tch));
	  				act (msgbuf, false, ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);

						/* Alert the staff of the theft */
						sprintf (msgbuf, "#3[Guardian: %s%s]#0 Plants %s on %s in %d.",
		   GET_NAME (ch),
		   IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
		   tobj->short_description,
		   (!IS_NPC (tch)) ? GET_NAME (tch) : (tch->short_descr),
		   ch->in_room);
						send_to_guardians (msgbuf, 0xFF);

	  if (!skill_use (ch, SKILL_STEAL, modifier))
	    {
								sprintf (msgbuf,
		       "$n approaches you and surreptitiously slips $p into #2%s#0!",
									 obj_short_desc (cobj));
								act (msgbuf, false, ch, tobj, tch, TO_VICT | _ACT_FORMAT);
								
								sprintf (msgbuf,
		       "$n approaches $N and surreptitiously slips $p into #2%s#0.",
									 obj_short_desc (cobj));
								act (msgbuf, false, ch, tobj, tch, TO_NOTVICT | _ACT_FORMAT);
							}
 					}
 					
 				for (af = tobj->xaffected; af; af = af->next)
								affect_modify (ch, af->type, af->a.spell.location,
									af->a.spell.modifier, tobj->obj_flags.bitvector,
								 false, 0);
						
				if (ch->right_hand == tobj)
					ch->right_hand = NULL;
				else if (ch->left_hand == tobj)
					ch->left_hand = NULL;
				obj_to_obj (tobj, cobj);	
				return; 
 			} //CASE 3
 
/*** CASE 4: palm <item> from <container> ***/
// item is tobj
// cobj is container	
		if (from && !targchk && contchk)
			{ 					
				if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
					{
						send_to_char ("You don't see such an item in that container.\n",
							ch);
						return;
					}
				
				if (tobj->obj_flags.weight / 100 > 3)
					{
						send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
						return;
					}
				
				if (ch->right_hand && ch->left_hand)
					{
						send_to_char
							("One of your hands needs to be free before attempting to palm something.\n",
							 ch);
						return;
					}
				
				act ("You carefully flick $p from $P into your hand.", false,
		   ch, tobj, cobj, TO_CHAR | _ACT_FORMAT);
	      
	      /* Alert the staff of the theft */
	      sprintf (msgbuf,
		       "#3[Guardian: %s%s]#0 Tries to secretly draw %s in %d.",
		       GET_NAME (ch),
		       IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
		       tobj->short_description,
		       ch->in_room);
	      send_to_guardians (msgbuf, 0xFF);

	      if (!skill_use (ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
		act ("$n attempts to handle $p surreptitiously.", false, ch,
		     tobj, 0, TO_ROOM | _ACT_FORMAT);
	      
	      
	      obj_from_obj (&tobj, 0); //rmoves tobj from wherever it is
	      obj_to_char (tobj, ch);
	     	return; 
		
			}// CASE 4
	    
	    
/*** CASE 5: palm <item> from <targetPC> <container> ***/
// item is tobj
// tch is targetPC
// cobj is container

	if (from && targchk)
			{
				argument = one_argument (argument, contbuf);
				
				if (!contchk)
					{
						if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
							{
								send_to_char
									("What did you wish to palm from?\n", ch);
								return;
							}
							
						if (GET_ITEM_TYPE (cobj) != ITEM_CONTAINER)
							{
								send_to_char ("You can only palm items from containers.\n", ch);
								return;
							}
					}
					
				if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
					{
						send_to_char ("You don't see such an item in that container.\n",
							ch);
						return;
					}
	    
	    	if (tobj->obj_flags.weight / 100 > 3)
					{
						send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
						return;
					}
					
				if (ch->right_hand && ch->left_hand)
					{
						send_to_char
							("One of your hands needs to be free before attempting to palm something.\n",
							 ch);
						return;
					}
				
				if (tch == ch)
 					{
 						act ("You carefully attempt to palm $p from $P.", false, ch,
		   tobj, cobj, TO_CHAR | _ACT_FORMAT);

						/* Alert the staff of the theft */
						sprintf (msgbuf,
							 "#3[Guardian: %s%s]#0 Tries to palm %s from %s in %d.",
							 GET_NAME (ch),
							 IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
							 tobj->short_description,
							 cobj->short_description,
							 ch->in_room);
						send_to_guardians (msgbuf, 0xFF);
		
						if (!skill_use (ch, SKILL_SLEIGHT, tobj->obj_flags.weight / 100))
							act ("$n gets $p from $P.", false, ch, tobj, cobj, 		     TO_ROOM | _ACT_FORMAT);
							
						obj_from_obj (&tobj, 0); //removes tobj from wherever it is
						obj_to_char (tobj, ch);
 					} //tch = ch
 				else
 					{
 						//must use the steal command. Besides, you can't see the item in the other guys container
 						send_to_char ("You can't see into that container.\n",
							ch);
						return;
 					}
 				return;	
			} // CASE 5
			
	return;
}//end function

void
do_steal (CHAR_DATA * ch, char *argument, int cmd)
{
  char target[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char command[MAX_INPUT_LENGTH];
  char obj_name[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  OBJ_DATA *obj = NULL;
  OBJ_DATA *tobj = NULL;
  int i, j, obj_num, amount = 0, modifier = 0, count = 0;

  sprintf (command, "%s", argument);

  if (IS_SWIMMING (ch))
    {
      send_to_char ("You can't do that while swimming!\n", ch);
      return;
    }

  if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
      send_to_char ("You cannot do this in an OOC area.\n", ch);
      return;
    }

  if (!real_skill (ch, SKILL_STEAL))
    {
      send_to_char ("You lack the skill to steal.\n\r", ch);
      return;
    }

  if (ch->right_hand || ch->left_hand)
    {
      send_to_char
	("Both your hands need to be free before attempting to steal something.\n",
	 ch);
      return;
    }

  argument = one_argument (argument, target);

  if (!*target)
    {
      send_to_char ("From whom did you wish to steal?\n", ch);
      return;
    }

  if (*target)
    {
      if (!(victim = get_char_room_vis (ch, target)))
	{
	  send_to_char ("Steal from whom?", ch);
	  return;
	}

      if (victim == ch)
	{
	  send_to_char ("You can't steal from yourself!\n\r", ch);
	  return;
	}

      if (victim->fighting)
	{
	  act ("$N's moving around too much for you to attempt a grab.",
	       false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
	  return;
	}

      if (!IS_NPC (victim) && !IS_MORTAL (victim) && !victim->pc->mortal_mode)
	{
	  send_to_char
	    ("The immortal cowers as you approach.  You just don't have the heart!\n\r",
	     ch);
	  return;
	}

      argument = one_argument (argument, obj_name);
      if (*obj_name && *obj_name != '!')
	{

	  if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
	      && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!')
	    {
	      sprintf (buf,
		       "You are in a lawful area. If you're caught stealing, "
		       "you may be killed or imprisoned. To confirm, "
		       "type \'#6steal %s !#0\', without the quotes.",
		       command);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      return;
	    }

	  if ((tobj = get_equip (victim, WEAR_BELT_1)) &&
	      CAN_SEE_OBJ (ch, tobj) && isname (obj_name, tobj->name))
	    obj = tobj;

	  else if ((tobj = get_equip (victim, WEAR_BELT_2)) &&
		   CAN_SEE_OBJ (ch, tobj) && isname (obj_name, tobj->name))
	    obj = tobj;

	  if (!obj)
	    {
	      send_to_char ("You don't see that on their belt.\n", ch);
	      return;
	    }

	  modifier = victim->skills[SKILL_SCAN] / 5;
	  modifier += obj->obj_flags.weight / 100;
	  modifier += 15;

	  /* Alert the staff of the theft */
	  notify_guardians (ch, victim, 6);

	  if (skill_use (ch, SKILL_STEAL, modifier))
	    {
	      act
		("You approach $N, deftly slipping $p from $S belt and moving off before you can be noticed.",
		 false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
	      unequip_char (victim, obj->location);
	      obj_to_char (obj, ch);
	      return;
	    }
	  else
	    {
	      if (skill_use (victim, SKILL_SCAN, 0))
		{
		  act
		    ("You approach $N cautiously, but at the last moment, before you can make the grab, $E glances down and notices your attempt!",
		     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		  act
		    ("Suddenly, you glance down, and notice $m attempting to lift $p from your belt!",
		     false, ch, obj, victim, TO_VICT | _ACT_FORMAT);
		  send_to_char ("\n", ch);
		  criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
		  return;
		}
	      else
		{
		  act
		    ("You approach $N cautiously, but at the last moment $E turns away, and your attempt is stymied. Thankfully, however, you have not been noticed.",
		     false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
		  return;
		}
	    }
	}
      else
	{

	  if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
	      && IS_SET (ch->room->room_flags, LAWFUL) && *obj_name != '!')
	    {
	      sprintf (buf,
		       "You are in a lawful area. If you're caught stealing, "
		       "you may be killed or imprisoned. To confirm, "
		       "type \'#6steal %s !#0\', without the quotes.",
		       command);
	      act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	      return;
	    }

	  for (obj = victim->equip; obj; obj = obj->next_content)
	    if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER
		&& !CAN_WEAR (obj, ITEM_WEAR_FEET)
		&& !CAN_WEAR (obj, ITEM_WEAR_WRIST)
		&& !CAN_WEAR (obj, ITEM_WEAR_ANKLE))
	      break;

	  if (!obj)
	    {
	      send_to_char
		("They don't seem to be wearing any containers you can pilfer from.\n",
		 ch);
	      return;
	    }

	  i = 0;

	  for (tobj = obj->contains; tobj; tobj = tobj->next_content)
	    i++;

	  obj_num = number (0, i);

	  j = 0;

	  for (tobj = obj->contains; tobj; tobj = tobj->next_content, j++)
	    if (j == obj_num)
	      break;

	  if (IS_SET (obj->o.container.flags, CONT_CLOSED))
	    modifier += 15;

	  modifier = victim->skills[SKILL_SCAN] / 10;

	  if (tobj)
	    modifier += tobj->obj_flags.weight;

	  /* Alert the staff of the theft */
	  notify_guardians (ch, victim, 6);

	  if (skill_use (ch, SKILL_STEAL, modifier))
	    {
	      if (tobj)
		{
		  act
		    ("You approach $N cautiously, slipping a hand inside $p. A moment later you move stealthily away, having successfully lifted something!",
		     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		  if (tobj->count >= 6)
		    {
		      amount = number (1, 6);
		    }
		  else
		    {
		      count = tobj->count;
		      count = MAX (1, tobj->count);
		      amount = number (1, count);
		    }
		  obj_from_obj (&tobj, amount);
		  obj_to_char (tobj, ch);
		  /*
		     if ( IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
		     obj_activate (ch, obj);
		   */
		  /*
		     tobj->count -= amount;
		     new_obj = load_object(tobj->nVirtual);
		     new_obj->count = amount;
		     obj_to_char (new_obj, ch);
		     if ( tobj->count <= 0 ) {
		     tobj->count = 1;
		     extract_obj(tobj);
		     }
		     }
		     else { 
		     count = tobj->count;
		     count = MAX (1, tobj->count);
		     amount = number(1,count);
		     tobj->count -= amount;
		     new_obj = load_object(tobj->nVirtual);
		     new_obj->count = amount;
		     obj_to_char (new_obj, ch);
		     if ( tobj->count <= 0 ) {
		     tobj->count = 1;
		     extract_obj (tobj);
		     }
		     }
		   */
		  return;
		}
	      else
		{
		  act
		    ("You approach $N cautiously, slipping your hand stealthily into $p. A moment later you withdraw, having been unable to find anything to lift, though your attempt has gone unnoticed.",
		     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		  return;
		}
	    }
	  else
	    {
	      if (skill_use (victim, SKILL_SCAN, 0))
		{
		  act
		    ("You approach $N cautiously, moving to slip your hand into $p. At the last moment, $E glances down, noticing your attempt!",
		     false, ch, obj, victim, TO_CHAR | _ACT_FORMAT);
		  act
		    ("You glance down, your gaze having been attracted by a hint of movement; you notice $n's hand moving toward $p, in an attempt to pilfer from you!",
		     false, ch, obj, victim, TO_VICT | _ACT_FORMAT);
		  send_to_char ("\n", ch);
		  criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
		  return;
		}
	      else
		{
		  act
		    ("You approach $N cautiously, but at the last moment $E turns away, fate having allowed $M to evade your attempt. Thankfully, it would seem that you were unnoticed.",
		     false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
		  return;
		}
	    }
	}
    }
}

void
post_idea (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  char msg[MAX_STRING_LENGTH];
  char *date;
  char *date2;
  time_t account_time;

  account_time = d->acct->created_on;
  date = (char *) asctime (localtime (&account_time));

  account_time = time (0);
  date2 = (char *) asctime (localtime (&account_time));

  date2[strlen (date2) - 1] = '\0';

  ch = d->character;
  if (!*d->pending_message->message)
    {
      send_to_char ("No suggestion report entered.\n", ch);
      mem_free (date);
      mem_free (date2);
      return;
    }

  sprintf (msg, "From: %s [%d]\n", ch->tname, ch->in_room);
  sprintf (msg + strlen (msg), "\n");
  sprintf (msg + strlen (msg), "%s", ch->desc->pending_message->message);

  add_message (1, "Ideas",
	       -5,
	       ch->desc->acct->name.c_str (),
	       date2, "Player Suggestion", "", msg, 0);

  send_to_char ("Thank you! Your suggestion has been recorded.\n\r", ch);

  unload_message (d->pending_message);

  mem_free (date);
  mem_free (date2);
}

void
do_idea (CHAR_DATA * ch, char *argument, int cmd)
{

  send_to_char
    ("Please submit your ideas for consideration on our web forum, located at:\n\n"
     "   #6http://www.middle-earth.us/forums/#0\n", ch);
  return;

}


const char *
get_room_desc_tag (CHAR_DATA * ch, ROOM_DATA * room)
{
  if (IS_MORTAL (ch)
      && weather_info[room->zone].state == HEAVY_SNOW
      && !IS_SET (room->room_flags, INDOORS))
    {

      return NULL;

    }
  else if (!room->extra
	   || desc_weather[room->zone] == WR_NORMAL
	   || !room->extra->weather_desc[desc_weather[room->zone]])
    {

      if (room->extra && room->extra->weather_desc[WR_NIGHT] && !sun_light)
	{

	  return weather_room[WR_NIGHT];
	}
      else
	{

	  return NULL;

	}

    }
  else
    {
      return weather_room[desc_weather[room->zone]];
    }
}

void
post_typo (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  char buf2[AVG_STRING_LENGTH];
  char msg[MAX_STRING_LENGTH];
  char *date;
  char *date2;
  const char *wr;
  time_t account_time;

  account_time = d->acct->created_on;
  date = (char *) asctime (localtime (&account_time));

  account_time = time (0);
  date2 = (char *) asctime (localtime (&account_time));

  date2[strlen (date2) - 1] = '\0';

  ch = d->character;
  if (!*d->pending_message->message)
    {
      send_to_char ("No typo report posted.\n", ch);
      mem_free (date);
      mem_free (date2);
      return;
    }

  if (ch->room->zone <= 99)
    sprintf (buf2, "%s", zone_table[ch->room->zone].name);
  else
    sprintf (buf2, "Auto-Generated Zone");

  wr = get_room_desc_tag (ch, ch->room);
  sprintf (msg, "From: %s [%d%s%s]\n", ch->tname, ch->in_room,
	   wr ? " - rset " : "", wr ? wr : "");
  sprintf (msg + strlen (msg), "\n");
  sprintf (msg + strlen (msg), "%s", ch->desc->pending_message->message);

  add_message (1, "Typos",
	       -5, ch->desc->acct->name.c_str (), date2, buf2, "", msg, 0);

  send_to_char
    ("Thank you! Your typo report has been entered into our tracking system.\n\r",
     ch);

  unload_message (d->pending_message);

  mem_free (date);
  mem_free (date2);
}

void
do_typo (CHAR_DATA * ch, char *argument, int cmd)
{

  if (IS_NPC (ch))
    {
      send_to_char ("Mobs can't submit bug reports.\n\r", ch);
      return;
    }

  CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
  send_to_char
    ("Enter a typo report to be submitted to the admins. Terminate\n"
     "the editor with an '@' symbol. Please note that your post\n"
     "will be stamped with all pertinent contact information; no\n"
     "need to include that in the body of your message. Thanks for\n"
     "doing your part to help improve our world!\n", ch);

  make_quiet (ch);

  CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

  ch->desc->str = &ch->desc->pending_message->message;
  ch->desc->max_str = MAX_STRING_LENGTH;

  ch->desc->proc = post_typo;
}

void
post_bug (DESCRIPTOR_DATA * d)
{
  CHAR_DATA *ch;
  char buf2[MAX_STRING_LENGTH];
  char msg[MAX_STRING_LENGTH];
  char *date;
  char *date2;
  time_t account_time;

  account_time = d->acct->created_on;
  date = (char *) asctime (localtime (&account_time));

  account_time = time (0);
  date2 = (char *) asctime (localtime (&account_time));

  date2[strlen (date2) - 1] = '\0';

  ch = d->character;
  if (!*d->pending_message->message)
    {
      send_to_char ("No bug report posted.\n", ch);
      mem_free (date);
      mem_free (date2);
      return;
    }

  sprintf (buf2, "Bug Report");

  sprintf (msg, "From: %s [%d]\n", ch->tname, ch->in_room);
  sprintf (msg + strlen (msg), "\n");
  sprintf (msg + strlen (msg), "%s", ch->desc->pending_message->message);

  add_message (1, "Bugs",
	       -5, ch->desc->acct->name.c_str (), date2, buf2, "", msg, 0);

  send_to_char
    ("Thank you! Your bug report has been entered into our system.\n\r", ch);

  unload_message (d->pending_message);

  mem_free (date);
  mem_free (date2);
}

void
do_bug (CHAR_DATA * ch, char *argument, int cmd)
{

  send_to_char
    ("This command has been disabled; we have found in past experience that bug\n"
     "reports are best posted on our web forum, in case they aren't actually bugs.\n"
     "However, if you feel this is an urgent issue, please email the staff.\n",
     ch);
  return;
}

void
do_brief (CHAR_DATA * ch, char *argument, int cmd)
{
  send_to_char ("Sorry, but this command has been disabled.\n", ch);
}

void
do_compact (CHAR_DATA * ch, char *argument, int cmd)
{
  const char *message[2] =
    {
      "You are now in the uncompacted mode.\n"
      "You are now in compact mode.\n",
    };

  // toggle FLAG_COMPACT and test if it is ON or OFF.
  // deliver the appropriate message to the player
  send_to_char (message[((ch->flags ^= FLAG_COMPACT) & FLAG_COMPACT)], ch);
}

void
sa_stand (SECOND_AFFECT * sa)
{
  if (!is_he_somewhere (sa->ch))
    return;

  if (GET_POS (sa->ch) == FIGHT || GET_POS (sa->ch) == STAND)
    return;

  do_stand (sa->ch, "", 0);
}

void
sa_get (SECOND_AFFECT * sa)
{
  char buf[MAX_STRING_LENGTH];

  if (!is_he_somewhere (sa->ch))
    return;

  sa->obj->tmp_flags &= ~SA_DROPPED;

  if (sa->obj == sa->ch->right_hand || sa->obj == sa->ch->left_hand)
    return;

  sprintf (buf, "get .c %d", sa->obj->coldload_id);

  command_interpreter (sa->ch, buf);
}

void
sa_wear (SECOND_AFFECT * sa)
{
  int num;
  char buf[MAX_STRING_LENGTH];

  if (!is_he_somewhere (sa->ch))
    return;

  if (!(num = is_obj_in_list (sa->obj, sa->ch->right_hand)) &&
      !(num = is_obj_in_list (sa->obj, sa->ch->left_hand)))
    {
      if (!IS_SET (sa->ch->flags, FLAG_COMPETE) ||
	  !is_obj_in_list (sa->obj, sa->ch->room->contents))
	return;

      extract_obj (sa->obj);

      return;
    }

  if (CAN_WEAR (sa->obj, ITEM_WIELD))
    sprintf (buf, "wield .c %d", sa->obj->coldload_id);
  else
    sprintf (buf, "wear .c %d", sa->obj->coldload_id);

  if (!CAN_WEAR (sa->obj, ITEM_WEAR_SHIELD)
      && GET_ITEM_TYPE (sa->obj) != ITEM_SHIELD)
    command_interpreter (sa->ch, buf);
}

void
sa_close_door (SECOND_AFFECT * sa)
{
  char buf[MAX_STRING_LENGTH];

  sprintf (buf, "close %s", sa->info);
  command_interpreter (sa->ch, buf);

  sprintf (buf, "lock %s", sa->info);
  command_interpreter (sa->ch, buf);
}

void
sa_knock_out (SECOND_AFFECT * sa)
{
  if (!is_he_somewhere (sa->ch))
    return;

  if (GET_POS (sa->ch) != SLEEP)
    return;

  if (get_affect (sa->ch, MAGIC_AFFECT_SLEEP))
    return;

  GET_POS (sa->ch) = REST;

  send_to_char ("You regain consciousness, flat on the ground.", sa->ch);
  act ("$n regains consciousness.", false, sa->ch, 0, 0, TO_ROOM);
}

void
sa_move (SECOND_AFFECT * sa)
{
  if (!is_he_somewhere (sa->ch))
    return;

  if (GET_POS (sa->ch) < FIGHT)
    return;

  do_move (sa->ch, "", sa->info2);
}

void
sa_command (SECOND_AFFECT * sa)
{
	command_interpreter(sa->ch, sa->info);
}

void
add_second_affect (int type, int seconds, CHAR_DATA * ch, OBJ_DATA * obj,
		  const char *info, int info2)
{
  SECOND_AFFECT *sa;

  CREATE (sa, SECOND_AFFECT, 1);

  sa->type = type;
  sa->seconds = seconds;
  sa->ch = ch;
  sa->obj = obj;
  sa->info2 = info2;

  if (info)
    sa->info = str_dup (info);
  else
    sa->info = NULL;

  sa->next = second_affect_list;

  second_affect_list = sa;
}

SECOND_AFFECT *
get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj)
{
  SECOND_AFFECT *sa;

  for (sa = second_affect_list; sa; sa = sa->next)
    if ((!ch || sa->ch == ch) && sa->type == type)
      if (!obj || obj == sa->obj)
	return sa;

  return NULL;
}

void
remove_second_affect (SECOND_AFFECT * sa)
{
  SECOND_AFFECT *sa_list;

  if (sa == second_affect_list)
    {

      second_affect_list = sa->next;

      if (sa->info)
	mem_free (sa->info);

      mem_free (sa);
      return;
    }

  for (sa_list = second_affect_list; sa_list; sa_list = sa_list->next)
    if (sa_list->next == sa)
      sa_list->next = sa->next;

  if (sa->info)
    mem_free (sa->info);

  mem_free (sa);
}

void
second_affect_update (void)
{
  SECOND_AFFECT *sa;
  SECOND_AFFECT *sa_t;
  SECOND_AFFECT *next_sa;
  extern int second_affect_active;

  for (sa = second_affect_list; sa; sa = next_sa)
    {

      next_sa = sa->next;

      if (--(sa->seconds) > 0)
	continue;

      if (sa == second_affect_list)
	second_affect_list = sa->next;
      else
	{
	  for (sa_t = second_affect_list; sa_t->next; sa_t = sa_t->next)
	    if (sa_t->next == sa)
	      {
		sa_t->next = sa->next;
		break;
	      }
	}

      second_affect_active = 1;

      switch (sa->type)
	{
	case SA_STAND:
	  sa_stand (sa);
	  break;
	case SA_GET_OBJ:
	  sa_get (sa);
	  break;
	case SA_WEAR_OBJ:
	  sa_wear (sa);
	  break;
	case SA_CLOSE_DOOR:
	  sa_close_door (sa);
	  break;
	case SA_WORLD_SWAP:
	  break;
	case SA_KNOCK_OUT:
	  sa_knock_out (sa);
	  break;
	case SA_MOVE:
	  sa_move (sa);
	  break;
	case SA_ESCAPE:
	  break;
	case SA_RESCUE:
	  sa_rescue (sa);
	  break;
        case SA_COMMAND:
          sa_command (sa);
	  break;
	}

      second_affect_active = 0;

      if (sa->info)
	{
	  mem_free (sa->info);
	  sa->info = NULL;
	}

      mem_free (sa);
    }
}

void
do_scommand (CHAR_DATA * ch, char * argument, int cmd)
{
	if (!IS_NPC(ch) && !GET_TRUST(ch) && !cmd)
	{
		send_to_char("You are not permitted to use this command.\n\r", ch);
		return;
	}
	std::string ArgumentList = argument, ThisArgument;
	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (ArgumentList.empty() || ThisArgument.empty())
	{
		send_to_char("Correct syntax: #6scommand delay command#0.\n\r", ch);
		return;
	}
	if (!(is_number(ThisArgument.c_str())))
	{
		send_to_char("Delay must be a number.\n\r", ch);
		return;
	}
	add_second_affect(SA_COMMAND, atoi(ThisArgument.c_str()), ch, NULL, ArgumentList.c_str(), 0);
	return;
}

void
prisoner_release (CHAR_DATA * ch, int zone)
{
  CHAR_DATA *tch;
  OBJ_DATA *obj, *bag;
  OBJ_DATA *tobj, *next_obj;
  OBJ_DATA *tobj2, *next_obj2;
  OBJ_DATA *tobj3, *next_obj3;
  ROOM_DATA *room;
  int dir = 0, jail_vnum = 0;
  bool jailed = false;
  char buf[MAX_STRING_LENGTH];
  char msg[MAX_STRING_LENGTH];
  char *date;
  time_t time_now;

  /* Make sure prisoners are in their cell */

  for (dir = 0; dir <= LAST_DIR; dir++)
    {
      if (!ch->room->dir_option[dir])
	continue;
      if (!(room = vtor (ch->room->dir_option[dir]->to_room)))
	continue;
      for (tch = room->people; tch; tch = tch->next_in_room)
	{
	  if (IS_SET (tch->act, ACT_JAILER))
	    {
	      jailed = true;
	      jail_vnum = tch->in_room;
	      break;
	    }
	}
    }

  if (!jailed)
    return;

  send_to_room ("The jailer flings open the cell door.\n\r", ch->in_room);
  act ("The jailer brutally knocks $n unconscious.", false, ch, 0, 0,
       TO_ROOM);
  act ("The jailer brutally knocks you unconscious.", false, ch, 0, 0,
       TO_CHAR);

  GET_POS (ch) = SLEEP;

  act ("The jailer removes $n and closes the door before you can react.",
       false, ch, 0, 0, TO_ROOM);

  char_from_room (ch);

  sprintf (buf, "A prison bag, labeled \'Belongings for %s\' sits here.",
	   ch->short_descr);

  bag = NULL;

  for (obj = object_list; obj; obj = obj->next)
    {
      if (obj->deleted)
	continue;
      if (GET_ITEM_TYPE (obj) != ITEM_CONTAINER)
	continue;
      if (!str_cmp (obj->description, buf))
	{
	  bag = load_object (1345);
	  for (tobj = obj->contains; tobj; tobj = next_obj)
	    {
	      next_obj = tobj->next_content;
	      if (IS_SET (tobj->obj_flags.extra_flags, ITEM_ILLEGAL))
		continue;
	      for (tobj2 = tobj->contains; tobj2; tobj2 = next_obj2)
		{
		  next_obj2 = tobj2->next_content;
		  if (IS_SET (tobj2->obj_flags.extra_flags, ITEM_ILLEGAL))
		    extract_obj (tobj2);
		  if (GET_ITEM_TYPE (tobj2) == ITEM_WEAPON)
		    extract_obj (tobj2);
		  for (tobj3 = tobj2->contains; tobj3; tobj3 = next_obj3)
		    {
		      next_obj3 = tobj3->next_content;
		      if (IS_SET (tobj3->obj_flags.extra_flags, ITEM_ILLEGAL))
			extract_obj (tobj3);
		      if (GET_ITEM_TYPE (tobj3) == ITEM_WEAPON)
			extract_obj (tobj3);
		    }
		}
	      obj_from_obj (&tobj, 0);
	      obj_to_obj (tobj, bag);
	    }
	  extract_obj (obj);
	  obj_to_char (bag, ch);
	}
    }

  switch (zone)
    {
    case 1:
    case 3:
      char_to_room (ch, VNUM_TIRITH_RELEASE_ROOM);
      break;
    case 5:
      char_to_room (ch, VNUM_MORGUL_RELEASE_ROOM);
      break;
    case 66:
      char_to_room (ch, VNUM_TE_RELEASE_ROOM);
      break;
    default:
      char_to_room (ch, jail_vnum);
      break;
    }

  time_now = time (0);
  date = (char *) asctime (localtime (&time_now));
  date[strlen (date) - 1] = '\0';

  if (bag)
    sprintf (msg, "Released from prison in %s with belongings intact.\n",
	     zone_table[ch->room->zone].name);
  else
    sprintf (msg, "Released from prison in %s. Belongings were not found!\n",
	     zone_table[ch->room->zone].name);

  sprintf (buf, "#2Released:#0 %s", ch->tname);

  add_message (1, ch->tname, -2, "Server", date, "Released.", "", msg, 0);
  add_message (1, "Prisoners", -5, "Server", date, buf, "", msg, 0);

}

void
rl_minute_affect_update (void)
{
  CHAR_DATA *ch;
  AFFECTED_TYPE *af;
  AFFECTED_TYPE *next_af;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  next_minute_update += 60;	/* This is a RL minute */

  for (ch = character_list; ch; ch = ch->next)
    {

      if (ch->deleted)
	continue;

      if (!ch->room)
	continue;

      if (IS_SET (ch->room->room_flags, OOC))
	continue;

      if (ch->mana < ch->max_mana && !IS_SET (ch->room->room_flags, OOC))
	{
	  ch->mana += (ch->aur / 3) + number (2, 6);
	}

      for (af = ch->hour_affects; af; af = next_af)
	{

	  next_af = af->next;

	  if (af->type == MAGIC_SIT_TABLE)
	    continue;

			/*** NOTE:  Make sure these are excluded in hour_affect_update ***/

	  if (af->type >= MAGIC_SPELL_GAIN_STOP
	      && af->type <= MAGIC_CRAFT_BRANCH_STOP)
	    {
	      if (--af->a.spell.duration <= 0)
		affect_remove (ch, af);
	    }

	  if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
	    {
	      if (--af->a.spell.duration <= 0)
		{
		  *buf = '\0';
		  sprintf (buf, "%s",
			   (af->a.spell.sn >=
			    0) ? spell_fade_echo (af->a.spell.sn,
						  af->type) : "");
		  if (!*buf)
		    {
		      sprintf (buf, "'%s' %d",
			       lookup_spell_variable (af->a.spell.sn,
						      VAR_NAME), af->type);
		      sprintf (buf2,
			       "There seems to be a problem with one of the fade echoes on the %s spell. Please report this to the staff. Thank you.",
			       buf);
		      // act (buf2, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
                      send_to_gods (buf2);
                      system_log (buf2, true);
		    }
		  else
		    act (buf, true, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		  if (af->type == MAGIC_AFFECT_SLEEP)
		    {
		      affect_remove (ch, af);
		      do_wake (ch, "", 0);
		      if (IS_NPC (ch))
			do_stand (ch, "", 0);
		    }
		  else
		    affect_remove (ch, af);
		}
	    }

	  if (af->type == MAGIC_PETITION_MESSAGE)
	    {
	      if (--af->a.spell.duration <= 0)
		affect_remove (ch, af);
	    }

	  if (af->type >= MAGIC_SKILL_GAIN_STOP &&
	      af->type <= MAGIC_SKILL_GAIN_STOP + LAST_SKILL)
	    {
	      if (--af->a.spell.duration <= 0)
		affect_remove (ch, af);
	    }

	  if (af->type >= MAGIC_FIRST_SOMA && af->type <= MAGIC_LAST_SOMA)
	    {
	      soma_rl_minute_affect (ch, af);
	    }
	}
    }
}

void
character_stink (CHAR_DATA * ch, AFFECTED_TYPE * ch_stink)
{
  int aroma_strength;
  AFFECTED_TYPE *room_stink;

  aroma_strength = ch_stink->a.smell.aroma_strength - 500;

  if (aroma_strength <= 0)
    return;

  room_stink = is_room_affected (ch->room->affects, ch_stink->type);

  if (room_stink && room_stink->a.smell.aroma_strength > aroma_strength)
    return;

  if (room_stink && room_stink->a.smell.duration == -1)
    return;

  if (!room_stink)
    {
      room_stink = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);
      room_stink->type = ch_stink->type;
      room_stink->next = ch->room->affects;

      ch->room->affects = room_stink;
    }

  /* Increase duration, but not above aroma_strength */

  if (room_stink->a.smell.duration < aroma_strength)
    {
      room_stink->a.smell.duration += aroma_strength / 100;
      if (room_stink->a.smell.duration > aroma_strength)
	room_stink->a.smell.duration = aroma_strength;
    }

  /* Same with aroma_strength */

  if (room_stink->a.smell.aroma_strength < aroma_strength)
    {
      room_stink->a.smell.aroma_strength += aroma_strength / 100;
      if (room_stink->a.smell.aroma_strength > aroma_strength)
	room_stink->a.smell.aroma_strength = aroma_strength;
    }
}

void
ten_second_update (void)
{
  CHAR_DATA *ch, *ch_next;
  AFFECTED_TYPE *af;
  AFFECTED_TYPE *next_af;
  AFFECTED_TYPE *room_stink;

  for (ch = character_list; ch; ch = ch_next)
    {

      ch_next = ch->next;

      if (ch->deleted)
	continue;

      if (!ch->room)
	continue;

      if (IS_SET (ch->room->room_flags, OOC))
	continue;

      if ((ch->room->sector_type == SECT_LAKE
	   || ch->room->sector_type == SECT_RIVER
	   || ch->room->sector_type == SECT_OCEAN
	   || ch->room->sector_type == SECT_REEF
	   || ch->room->sector_type == SECT_UNDERWATER) && !number (0, 1))
	{
	  if (swimming_check (ch))
	    continue;		/* PC drowned. */
	}

      for (af = ch->hour_affects; af; af = next_af)
	{

	  next_af = af->next;

	  if (af->type == MAGIC_SIT_TABLE)
	    continue;

			/***** Exclusion must be made in hour_affect_update!!! ******/

	  if (af->type >= MAGIC_SMELL_FIRST && af->type <= MAGIC_SMELL_LAST)
	    {

	      room_stink = is_room_affected (ch->room->affects, af->type);

	      if (af->a.smell.duration != -1 &&
		  (!room_stink ||
		   room_stink->a.smell.aroma_strength - 500 <
		   af->a.smell.aroma_strength))
		{

		  af->a.smell.duration -= 10;
		  af->a.smell.aroma_strength -= 10;

		  if (af->a.smell.duration <= 0 ||
		      af->a.smell.aroma_strength <= 0)
		    {
		      affect_remove (ch, af);
		      continue;
		    }
		}

	      if (af->a.smell.duration > 500 &&
		  af->a.smell.aroma_strength > 500)
		character_stink (ch, af);
	    }

	  else if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
	    {

	      if (IS_NPC (ch))
		continue;

	      if (!af->a.craft->timer)
		continue;

	      af->a.craft->timer -= 10;

	      if (af->a.craft->timer < 0)
		af->a.craft->timer = 0;

	      if (!af->a.craft->timer)
		activate_phase (ch, af);
	    }

	  else if (af->type >= MAGIC_FIRST_SOMA &&
		   af->type <= MAGIC_LAST_SOMA)
	    {
	      soma_ten_second_affect (ch, af);
	    }

	  else if (af->type >= MAGIC_CRIM_HOODED &&
		   af->type < MAGIC_CRIM_HOODED + 100)
	    {

	      af->a.spell.duration -= 10;

	      /* If ch is fighting, keep the temp flag on him so
	         we can criminalize him after the fight with the
	         guard is over. */

	      if (ch->fighting && af->a.spell.duration < 1)
		af->a.spell.duration = 1;

	      if (af->a.spell.duration <= 0)
		affect_remove (ch, af);
	    }

	  else if (af->type == MAGIC_STARED || af->type == MAGIC_WARNED)
	    {
	      af->a.spell.duration -= 10;

	      if (af->a.spell.duration <= 0)
		affect_remove (ch, af);
	    }
	}

      if (ch->deleted)
	continue;

      if (ch->mob && ch->mob->resets)
	activate_resets (ch);
    }
}

void
payday (CHAR_DATA * ch, CHAR_DATA * employer, AFFECTED_TYPE * af)
{
  int t;
  int i;
  OBJ_DATA *tobj;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;

  if (time_info.holiday >= 1)
    {
      send_to_char ("Check again after the feastday has ended.\n", ch);
      return;
    }

  t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

  af->a.job.pay_date = t + af->a.job.days;

  if (af->a.job.cash)
    {
    if (employer)
		{	
		if (keeper_has_money(employer, af->a.job.cash))
			{
			keeper_money_to_char(employer, ch, af->a.job.cash);
			subtract_keeper_money (employer, af->a.job.cash);
			sprintf (buf,
	   			"INSERT INTO server_logs.payroll "
	   			"(time, shopkeep, who, customer, amount, "
	   			"room, gametime, port) "
	   			"VALUES (NOW(), %d, '%s','%s', %d, %d,'%d-%d-%d %d:00',%d)",
	   			employer->mob->nVirtual, GET_NAME (ch), char_short (ch),
	   			af->a.job.cash, ch->in_room, time_info.year,
				 time_info.month + 1, time_info.day + 1, time_info.hour, engine.get_port ());
  			mysql_safe_query (buf);
			
			sprintf (buf, "$N pays you for all your hard work.");
	  		act (buf, true, ch, 0, employer, TO_CHAR | _ACT_FORMAT);
	  		act ("$N pays $n some coins.", false, ch, 0, employer, 	       TO_NOTVICT | _ACT_FORMAT);
	  		} //employer/keeper has the coin
	  	else
	  		{
	  		sprintf (buf, "$N whispers to you that they do not have the funds available. Try again later.");
	  		act (buf, true, ch, 0, employer, TO_CHAR | _ACT_FORMAT);
	  		act ("$N whispers somemthing to $n.", false, ch, 0, employer,  TO_NOTVICT | _ACT_FORMAT);
	  		} //employer Does NOT have the coin
		}//there is an employer
	else
		{
      obj = load_object (1542);
      obj->count = af->a.job.cash;
      obj_to_char (obj, ch);
	  sprintf (buf, "You are paid %d coppers.\n", af->a.job.cash);
	  	send_to_char (buf, ch);
		} // there is not an employer
    } //paid in cash

  if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
    {

      for (i = af->a.job.count; i; i--)
	obj_to_char (load_object (af->a.job.object_vnum), ch);

      if (employer)
	{
	  sprintf (buf, "$N pays you %d x $p.", af->a.job.count);
	  act (buf, false, ch, tobj, employer, TO_CHAR | _ACT_FORMAT);
	  act ("$N pays $n with $o.", true, ch, tobj, employer,
	       TO_NOTVICT | _ACT_FORMAT);
	}
      else
	{
	  sprintf (buf, "You are paid %d x $p.", af->a.job.count);
	  act (buf, false, ch, tobj, 0, TO_CHAR | _ACT_FORMAT);
	  act ("$n is paid with $o.", true, ch, tobj, 0,
	       TO_ROOM | _ACT_FORMAT);
	}
    }

  sprintf (buf, "\nYour next payday is in %d days.\n", af->a.job.days);
  send_to_char (buf, ch);
}

void
do_payday (CHAR_DATA * ch, char *argument, int cmd)
{
  int i;
  int t;
  bool isEmployed = false;
  CHAR_DATA *employer = NULL;
  OBJ_DATA *tobj = NULL;
  AFFECTED_TYPE *af;
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];

  if (time_info.holiday >= 1)
    {
      send_to_char ("Check again after the feastday has ended.\n", ch);
      return;
    }

  t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

  for (i = JOB_1; i <= JOB_3; i++)
    {
		employer = 0;

      if (!(af = get_affect (ch, i)))
	continue;

      isEmployed = true;

      if (af->a.job.employer)
	employer = vtom (af->a.job.employer);

      /* PC action to get paid if it is time */

      if (t >= af->a.job.pay_date)
	{
	  if (!af->a.job.employer)
	    {
	      payday (ch, NULL, af);
	      continue;
	    }
	  for (tch = ch->room->people; tch; tch = tch->next_in_room)
	    {
	      if (IS_NPC (tch) && tch->mob->nVirtual == af->a.job.employer)
		{
		  payday (ch, tch, af);
		  continue;
		}
	    }
	}

      /* Either its not time to be paid, or employer is not around */

      if ((af->a.job.employer) && (employer))
	{
	  sprintf (buf, "%s#0 will pay you ", employer->short_descr);
	  *buf = toupper (*buf);
	  sprintf (buffer, "#5%s", buf);
	  sprintf (buf, "%s", buffer);
	}
      else
	strcpy (buf, "You get paid ");

      if (af->a.job.cash)
	{
	  sprintf (buf + strlen (buf), "%d coppers", af->a.job.cash);
	  if (af->a.job.count && vtoo (af->a.job.object_vnum))
	    strcat (buf, " and ");
	}

      if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
	{
	  if (af->a.job.count == 1)
	    strcat (buf, "$p");
	  else
	    sprintf (buf + strlen (buf), "%d x $p", af->a.job.count);
	}

      sprintf (buf + strlen (buf), ", every %d day%s.  Next #3payday#0 in %d "
	       "day%s.",
	       af->a.job.days, af->a.job.days == 1 ? "" : "s",
	       af->a.job.pay_date - t,
	       af->a.job.pay_date - t == 1 ? "" : "s");

      act (buf, true, ch, tobj, employer, TO_CHAR | _ACT_FORMAT);
    }

  if (!isEmployed)
    {
      send_to_char ("You do not appear to have been setup with a payday.\n",
		    ch);
    }
}

void
hour_affect_update (void)
{
  int t;
  CHAR_DATA *ch;
  AFFECTED_TYPE *af;
  AFFECTED_TYPE *next_af;

  t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

  for (ch = character_list; ch; ch = ch->next)
    {

      if (ch->deleted || !ch->room)
	continue;

      if (ch->room && IS_SET (ch->room->room_flags, OOC))
	continue;

      for (af = ch->hour_affects; af; af = next_af)
	{

	  next_af = af->next;

	  if (af->type == MAGIC_SIT_TABLE)
	    continue;

	  if (af->type >= JOB_1 && af->type <= JOB_3)
	    {

	      if (t < af->a.job.pay_date)
		continue;

	      if (af->a.job.employer) /// \todo Check as dead code
		continue;

	      /* payday (ch, NULL, af); */
	      continue;
	    }

	  if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
	    continue;

	  if (af->type >= MAGIC_SMELL_FIRST && af->type <= MAGIC_SMELL_LAST)
	    continue;

	  if (af->type >= MAGIC_SKILL_GAIN_STOP &&
	      af->type <= MAGIC_SKILL_GAIN_STOP + LAST_SKILL)
	    continue;

	  if (af->type >= MAGIC_FLAG_NOGAIN &&
	      af->type <= MAGIC_FLAG_NOGAIN + LAST_SKILL)
	    continue;

	  if (af->type >= MAGIC_CRIM_HOODED &&
	      af->type < MAGIC_CRIM_HOODED + 100)
	    continue;

	  if (af->type == MAGIC_STARED)
	    continue;

	  if (af->type == MAGIC_RAISED_HOOD)
	    continue;

	  if (af->type == MAGIC_CRAFT_DELAY
	      || af->type == MAGIC_CRAFT_BRANCH_STOP)
	    continue;

	  if (af->type == MAGIC_PETITION_MESSAGE)
	    continue;

	  if (af->type == AFFECT_HOLDING_BREATH)
	    continue;

	  if (af->type == MAGIC_GUARD ||
	      af->type == AFFECT_SHADOW ||
	      (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST))
	    continue;

	  if (af->a.spell.duration > 0)
	    af->a.spell.duration--;

	  if (af->a.spell.duration)
	    continue;

	  else if (af->type >= MAGIC_CRIM_BASE + 1
		   && af->type <= MAGIC_CRIM_BASE + 99)
	    prisoner_release (ch, af->type - MAGIC_CRIM_BASE);

	  else if (af->type == AFFECT_LOST_CON)
	    {
	      send_to_char ("You finally feel at your best once more.\n", ch);
	      ch->con += af->a.spell.sn;
	      ch->tmp_con += af->a.spell.sn;
	    }

				/******************************************/
	  /*   SPELL WEAR-OFF MESSAGE               */
				/******************************************/

	  else if (af->type == MAGIC_AFFECT_TONGUES)
	    ch->speaks = af->a.spell.modifier;	/* Revert to former language */

	  affect_remove (ch, af);
	}
    }
}

void
do_initiate (CHAR_DATA * ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *target_name;
  CHAR_DATA *target;
  int to_circle;
  int circle_levs[] = { 0, 36, 42, 48, 52, 57, 60, 65, 70, 100 };

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (!ch->deity)
    {
      send_to_char ("Be careful you don't attract the wrath of a god.\n\r",
		    ch);
      return;
    }

  if (atoi (arg1))
    {
      to_circle = atoi (arg1);
      target_name = arg2;
    }
  else if (atoi (arg2))
    {
      to_circle = atoi (arg2);
      target_name = arg1;
    }
  else
    {
      send_to_char ("You must follow the exact time honored rituals when "
		    "initiating a brother\n\ror sister.\n\r", ch);
      return;
    }

  if (to_circle < 1 || to_circle > 9)
    {
      send_to_char ("$g doesn't recognize that circle.\n\r", ch);
      return;
    }

  if (!(target = get_char_room_vis (ch, target_name)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (to_circle >= ch->circle)
    {
      send_to_char ("You cannot initiate at or above your circle.\n\r", ch);
      return;
    }

  if (target->deity != ch->deity)
    {
      act ("Your lesson is lost on $N.", false, ch, 0, target, TO_CHAR);
      return;
    }

  if (target->circle + 1 < to_circle)
    {
      act ("$N is not ready for that circle.\n\r",
	   false, ch, 0, target, TO_CHAR);
      return;
    }
  else if (target->circle + 1 > to_circle)
    {
      act ("$N is beyond those teachings.", false, ch, 0, target, TO_CHAR);
      return;
    }

  if (target->skills[SKILL_RITUAL] < circle_levs[to_circle])
    {
      act ("$N cannot be initiated to that circle.",
	   false, ch, 0, target, TO_CHAR);
      return;
    }

  target->circle++;

  sprintf (buf, "You quietly instruct $N in the secrets "
	   "of the %d%s circle of $G.", target->circle,
	   target->circle == 1 ? "st" :
	   target->circle == 2 ? "nd" : target->circle == 3 ? "rd" : "th");
  act (buf, false, ch, 0, target, TO_CHAR);

  sprintf (buf, "$N quietly instructs you in the secrets "
	   "of the %d%s circle of $g.", target->circle,
	   target->circle == 1 ? "st" :
	   target->circle == 2 ? "nd" : target->circle == 3 ? "rd" : "th");
  act (buf, false, target, 0, ch, TO_CHAR);
}

void
open_skill (CHAR_DATA * ch, int skill)
{
  if (IS_NPC (ch))
    ch->skills[skill] += calc_lookup (ch, REG_OV, skill);
  else
    {
      ch->pc->skills[skill] = calc_lookup (ch, REG_OV, skill);
      ch->skills[skill] += ch->pc->skills[skill];
    }
}

#define MIN_PREREQ	15

int
prereq_skill (CHAR_DATA * ch, CHAR_DATA * victim, int skill,
	      int prereq1, int prereq2)
{
  char buf[MAX_STRING_LENGTH];

  if (prereq1 && victim->skills[prereq1] < MIN_PREREQ)
    {

      sprintf (buf, "$N cannot learn '%s' until $S learns '%s' sufficiently.",
	       skills[skill], skills[prereq1]);
      act (buf, true, ch, 0, victim, TO_CHAR);

      sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
	       "'%s' sufficiently.", skills[skill], skills[prereq1]);
      act (buf, true, victim, 0, ch, TO_CHAR);

      return 0;
    }

  else if (prereq2 && victim->skills[prereq2] < MIN_PREREQ)
    {

      sprintf (buf, "$N cannot learn '%s' until $E learns '%s' sufficiently.",
	       skills[skill], skills[prereq2]);
      act (buf, true, ch, 0, victim, TO_CHAR);

      sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
	       "'%s' sufficiently.", skills[skill], skills[prereq2]);
      act (buf, true, victim, 0, ch, TO_CHAR);

      return 0;
    }

  return 1;
}

int
meets_craft_teaching_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
  PHASE_DATA *phase;

  for (phase = craft->phases; phase; phase = phase->next)
    {
      if (phase->skill)
	if (!real_skill (ch, phase->skill)
	    || ch->skills[phase->skill] <
	    (int) ((phase->dice * phase->sides) * .75))
	  return 0;
    }

  return 1;
}

int
meets_craft_learning_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
  PHASE_DATA *phase;

  for (phase = craft->phases; phase; phase = phase->next)
    {
      if (phase->skill)
	if (!real_skill (ch, phase->skill)
	    || ch->skills[phase->skill] <
	    (int) ((phase->dice * phase->sides) * .33))
	  return 0;
    }

  return 1;
}

void
do_teach (CHAR_DATA * ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int i;
  CHAR_DATA *victim;
  SUBCRAFT_HEAD_DATA *craft;
  AFFECTED_TYPE *af;
  int sn = -1;

  if (IS_SET (ch->room->room_flags, OOC))
    {
      send_to_char ("This is not allowed in OOC areas.\n", ch);
      return;
    }

  half_chop (argument, arg1, arg2);

  if (!*arg1)
    {
      send_to_char ("Teach what?\n\r", ch);
      return;
    }

  if (!*arg2)
    {
      send_to_char ("Teach what to who?\n\r", ch);
      return;
    }

  if (!(victim = get_char_room_vis (ch, arg2)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if ((i = index_lookup (skills, arg1)) == -1)
    {
      for (craft = crafts; craft; craft = craft->next)
	{
	  if (ch->desc->original && !str_cmp (craft->subcraft_name, arg1))
	    break;
	  if (!str_cmp (craft->subcraft_name, arg1) && has_craft (ch, craft))
	    break;
	}

      if (craft)
	{
	  if (!ch->desc->original
	      && !meets_craft_teaching_requirements (ch, craft))
	    {
	      send_to_char
		("You aren't proficient enough to teach that craft yet.\n",
		 ch);
	      return;
	    }
	  if (!meets_craft_learning_requirements (victim, craft))
	    {
	      send_to_char
		("The intricacies of this craft seem to be beyond your pupil at this time.\n",
		 ch);
	      return;
	    }
	  if (has_craft (victim, craft))
	    {
	      send_to_char ("They already know that craft.\n", ch);
	      return;
	    }
	  for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	    if (!get_affect (victim, i))
	      break;
	  magic_add_affect (victim, i, -1, 0, 0, 0, 0);
	  af = get_affect (victim, i);
	  af->a.craft =
	    (struct affect_craft_type *)
	    alloc (sizeof (struct affect_craft_type), 23);
	  af->a.craft->subcraft = craft;
	  sprintf (buf, "You teach $N '%s %s'.", craft->command,
		   craft->subcraft_name);
	  act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
	  sprintf (buf, "$n has taught you '%s %s'.", craft->command,
		   craft->subcraft_name);
	  act (buf, false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
	  sprintf (buf, "$n teaches $N something.");
	  act (buf, false, ch, 0, victim, TO_NOTVICT | _ACT_FORMAT);
	  return;
	}
    }

  if ((i = index_lookup (skills, arg1)) == -1 &&
      (sn = spell_lookup (arg1)) == -1)
    {
      send_to_char ("No such skill, spell or craft.\n\r", ch);
      return;
    }

//Spells 

  if (sn != -1)
    {
      send_to_char ("No teaching spells at the moment.\n\r", ch);
      return;
    }

//Regular Skills
  if (!real_skill (ch, i))
    {
      send_to_char ("You don't know that skill!\n\r", ch);
      return;
    }

  if (ch->skills[i] < 50)
    {
      send_to_char ("You don't yet know that skill well enough.\n\r", ch);
      return;
    }

  if (real_skill (victim, i))
    {
      send_to_char ("They already know that.\n\r", ch);
      return;
    }

  if (!trigger (ch, argument, TRIG_TEACH))
    return;

  switch (i)
    {
    case SKILL_LIGHT_EDGE:
    case SKILL_MEDIUM_EDGE:
    case SKILL_HEAVY_EDGE:
    case SKILL_LIGHT_BLUNT:
    case SKILL_MEDIUM_BLUNT:
    case SKILL_HEAVY_BLUNT:
    case SKILL_LIGHT_PIERCE:
    case SKILL_MEDIUM_PIERCE:
    case SKILL_HEAVY_PIERCE:
    case SKILL_SHORTBOW:
    case SKILL_LONGBOW:
    case SKILL_CROSSBOW:
    case SKILL_THROWN:
    case SKILL_STAFF:
    case SKILL_POLEARM:
      if ((victim->skills[SKILL_LIGHT_EDGE]
	   || victim->skills[SKILL_MEDIUM_EDGE]
	   || victim->skills[SKILL_HEAVY_EDGE]
	   || victim->skills[SKILL_LIGHT_BLUNT]
	   || victim->skills[SKILL_MEDIUM_BLUNT]
	   || victim->skills[SKILL_HEAVY_BLUNT]
	   || victim->skills[SKILL_LIGHT_PIERCE]
	   || victim->skills[SKILL_MEDIUM_PIERCE]
	   || victim->skills[SKILL_HEAVY_PIERCE]
	   || victim->skills[SKILL_LONGBOW] || victim->skills[SKILL_CROSSBOW]
	   || victim->skills[SKILL_SHORTBOW] || victim->skills[SKILL_THROWN]
	   || victim->skills[SKILL_STAFF] || victim->skills[SKILL_POLEARM])
	  && victim->offense < MIN_PREREQ)
	{
	  sprintf (buf, "$N cannot learn %s until $E learns offense better.",
		   skills[i]);
	  act (buf, true, ch, 0, victim, TO_CHAR | _ACT_FORMAT);

	  sprintf (buf, "$N tries to teach you '%s', but cannot until "
		   "you learn offense better.", skills[i]);
	  act (buf, true, victim, 0, ch, TO_CHAR | _ACT_FORMAT);
	  return;
	}
      break;
   

//Psionics
    case SKILL_MENTAL_BOLT:
    case SKILL_PRESCIENCE:
    case SKILL_CLAIRVOYANCE:
    case SKILL_EMPATHIC_HEAL:
    case SKILL_DANGER_SENSE:
    case SKILL_TELEPATHY:
    case SKILL_HEX:
    case SKILL_SENSITIVITY:
      send_to_char ("Psionics cannot be taught.\n", ch);
      return;

//Dependant skills
    case SKILL_HERBALISM:
      if (!prereq_skill (ch, victim, SKILL_HERBALISM, SKILL_FORAGE, 0))
      	{
					send_to_char ("They don't know enought about Foraging.\n\r", ch);
					return;
				}
      break;

    case SKILL_ALCHEMY:
      if (!prereq_skill (ch, victim, SKILL_ALCHEMY, SKILL_HERBALISM, 0))
				{
					send_to_char ("They don't know enought about herbalism.\n\r", ch);
					return;
				}
      break;

    case SKILL_SNEAK:
      if (!prereq_skill (ch, victim, SKILL_SNEAK, SKILL_HIDE, 0))
				{
					send_to_char ("They don't know enought about hiding.\n\r", ch);
					return;
				}
      break;

    case SKILL_DUAL:
      if (victim->offense < 26)
	{
	  send_to_char ("Their offense is too low for them to "
			"learn dual wield.\n\r", ch);
	  return;
	}
      break;

    case SKILL_STEAL:
      if (!prereq_skill (ch, victim, SKILL_STEAL, SKILL_SNEAK, SKILL_HIDE))
				{
					send_to_char ("They don't know enought about several things.\n\r", ch);
					return;
				} /// \todo Check if the above condition is always true. 
      break;

    case SKILL_RITUAL:
    	{
      send_to_char ("Ritual can only be learned through conversion.\n\r", ch);
      	return;
      }
      break;

//Skills that are not supported at this time
		case SKILL_DISARM:
			return;  //not used
      break;
      
		case SKILL_POISONING:
      if (!prereq_skill (ch, victim, SKILL_POISONING,
			 SKILL_BACKSTAB, SKILL_ALCHEMY))
				{
					send_to_char ("They don't know enought about several things.\n\r", ch);
					return;
				}
      break;

    case SKILL_BACKSTAB:
      if (!prereq_skill (ch, victim, SKILL_BACKSTAB,
			 SKILL_LIGHT_PIERCE, SKILL_HIDE))
				{
					send_to_char ("They don't know enought about several things.\n\r", ch);
					return;
				}
      break;
      
//The following skills will be taught without any checks or restrictions
//Normal skills
    case SKILL_PARRY:
    case SKILL_BLOCK:
    case SKILL_PICK:
    case SKILL_SEARCH:
    case SKILL_SCAN:
    case SKILL_CLIMB:
    case SKILL_FORAGE:
    case SKILL_SKIN:
    case SKILL_HIDE:
    case SKILL_LISTEN:
      break;
      
//Scripts
    case SKILL_SCRIPT_BELERIAND_TENGWAR:
    case SKILL_SCRIPT_CERTHAS_DAERON:
    case SKILL_SCRIPT_ANGERTHAS_DAERON:
    case SKILL_SCRIPT_QUENYAN_TENGWAR:
    case SKILL_SCRIPT_ANGERTHAS_MORIA:
    case SKILL_SCRIPT_GONDORIAN_TENGWAR:
    case SKILL_SCRIPT_ARNORIAN_TENGWAR:
    case SKILL_SCRIPT_NUMENIAN_TENGWAR:
    case SKILL_SCRIPT_NORTHERN_TENGWAR:
    case SKILL_SCRIPT_ANGERTHAS_EREBOR:
			break;

//Langauges     
    case SKILL_SPEAK_ATLIDUK:
    case SKILL_SPEAK_ADUNAIC:
    case SKILL_SPEAK_HARADAIC:
    case SKILL_SPEAK_WESTRON:
    case SKILL_SPEAK_DUNAEL:
    case SKILL_SPEAK_LABBA:
    case SKILL_SPEAK_NORLIDUK:
    case SKILL_SPEAK_ROHIRRIC:
    case SKILL_SPEAK_TALATHIC:
    case SKILL_SPEAK_UMITIC:
    case SKILL_SPEAK_NAHAIDUK:
    case SKILL_SPEAK_PUKAEL:
    case SKILL_SPEAK_SINDARIN:
    case SKILL_SPEAK_QUENYA:
    case SKILL_SPEAK_SILVAN:
    case SKILL_SPEAK_KHUZDUL:
    case SKILL_SPEAK_ORKISH:
    case SKILL_SPEAK_BLACK_SPEECH:
      break;
    }

	open_skill (victim, i);	
  sprintf (buf, "$N teaches you '%s'.", skills[i]);
  act (buf, true, victim, 0, ch, TO_CHAR);
 

  if (IS_MORTAL (victim))
    update_crafts (victim);

  send_to_char ("Done.\n\r", ch);
}

void
add_memory (CHAR_DATA * add, CHAR_DATA * mob)
{
  struct memory_data *memory;
  char name[MAX_STRING_LENGTH];

  if (IS_NPC (add))
    one_argument (GET_NAMES (add), name);
  else
    strcpy (name, GET_NAME (add));

  for (memory = mob->remembers; memory; memory = memory->next)
    if (!strcmp (memory->name, name))
      return;

  CREATE (memory, struct memory_data, 1);

  memory->name = add_hash (name);
  memory->next = mob->remembers;

  mob->remembers = memory;
}

void
forget (CHAR_DATA * ch, CHAR_DATA * foe)
{
  struct memory_data *mem;
  struct memory_data *tmem;

  if (!ch->remembers)
    return;

  if (!strcmp (GET_NAME (foe), ch->remembers->name))
    {
      mem = ch->remembers;
      ch->remembers = ch->remembers->next;
      mem_free (mem);
      return;
    }

  for (mem = ch->remembers; mem->next; mem = mem->next)
    {
      if (!strcmp (GET_NAME (foe), mem->next->name))
	{
	  tmem = mem->next;
	  mem->next = tmem->next;
	  mem_free (tmem);
	  return;
	}
    }
}

void
do_forage (CHAR_DATA * ch, char *argument, int cmd)
{
  int sector_type;

  if (!real_skill (ch, SKILL_FORAGE))
    {
      send_to_char ("You don't have any idea how to forage!\n\r", ch);
      return;
    }

  if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
      && !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
      send_to_char ("It's too dark to forage.\n\r", ch);
      return;
    }

  if (is_sunlight_restricted (ch))
    return;

  sector_type = vtor (ch->in_room)->sector_type;

  if (sector_type != SECT_WOODS &&
      sector_type != SECT_FOREST &&
      sector_type != SECT_FIELD &&
      sector_type != SECT_PASTURE &&
      sector_type != SECT_HEATH && sector_type != SECT_HILLS)
    {
      send_to_char ("This place is devoid of anything edible.\n\r", ch);
      return;
    }

  send_to_char ("You begin rummaging for something edible.\n\r", ch);
  act ("$n begins examining the flora.", true, ch, 0, 0, TO_ROOM);

  ch->delay_type = DEL_FORAGE;
  ch->delay = 25;
}

#define NUM_FORAGEABLES 17
const int forageables[NUM_FORAGEABLES] = {
  161,
  97002,
  92072,
  97132,
  97202,
  97238,
  97372,
  97405,
  97554,
  97594,
  97739,
  97742,
  97759,
  97784,
  97853,
  98043,
  98098
};

void
delayed_forage (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  int seasonal_penalty = 0, temp = 0, base_temp = 75, range = 20;

  temp = weather_info[ch->room->zone].temperature;
  seasonal_penalty =
    (temp > base_temp) ? (temp - base_temp) : (base_temp - temp);
  seasonal_penalty =
    (seasonal_penalty > range) ? (seasonal_penalty - range) : 0;

  if (skill_use (ch, SKILL_FORAGE, seasonal_penalty))
    {

      obj = NULL;
      obj = load_object (forageables[number (0, NUM_FORAGEABLES - 1)]);
      if (!obj)
	obj = load_object (161);
      if (obj)
	{
	  act ("You pick $p.", false, ch, obj, 0, TO_CHAR);
	  act ("$n finds $p hidden in the flora.", true, ch, obj, 0,
	       TO_ROOM | _ACT_FORMAT);
	  obj_to_char (obj, ch);
	  if (obj->in_room != NOWHERE)
	    act ("$n leaves $p on the the ground.", true, ch, obj, 0,
		 TO_ROOM | _ACT_FORMAT);
	}
      else
	{
	  send_to_char ("You deserved something, but the MUD is bare.\n\r",
			ch);
	  return;
	}



    }
  else
    {
      send_to_char ("Your efforts to find food are of no avail.\n\r", ch);
      act ("$n stops searching the flora.", true, ch, 0, 0, TO_ROOM);
    }
}

int
has_a_key (CHAR_DATA * mob)
{
  if (mob->right_hand && GET_ITEM_TYPE (mob->right_hand) == ITEM_KEY)
    return 1;
  if (mob->left_hand && GET_ITEM_TYPE (mob->left_hand) == ITEM_KEY)
    return 1;

  return 0;
}

void
do_knock (CHAR_DATA * ch, char *argument, int cmd)
{
  int door;
  int target_room;
  int trigger_info;
  int key;
  char dir[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char key_name[MAX_STRING_LENGTH];
  CHAR_DATA *tch;
  ROOM_DATA *room;

  argument = one_argument (argument, buf);
  argument = one_argument (argument, dir);

  if ((door = find_door (ch, buf, dir)) == -1)
    return;

  if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
  		&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
    {
      send_to_char ("That's not a door.\n\r", ch);
      return;
    }

  else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
    {
      send_to_char ("It's already open!\n\r", ch);
      return;
    }

  target_room = EXIT (ch, door)->to_room;

  if (!vtor (target_room))
    {
      send_to_char ("Actually, that door doesn't lead anywhere.\n\r", ch);
      return;
    }

  sprintf (buf, "You hear tapping from the %s.\n\r", dirs[rev_dir[door]]);
  send_to_room (buf, target_room);

  act ("You rap on the door.", false, ch, 0, 0, TO_CHAR);
  act ("$n raps on the door.", false, ch, 0, 0, TO_ROOM);

  /* The trigger command is activated for mobs in the room that ch
     occupies.  So, we have to switch to the target_room to get the
     trigger to happen in that room.
   */

  room = vtor (target_room);

  ch->room = room;

  trigger_info = trigger (ch, argument, TRIG_KNOCK);

  ch->room = vtor (ch->in_room);

  /* An unfortunate drag:  If the trigger was activated, that doesn't
     necessarily mean some mob shouldn't automatically open the door.
     But, the way this is implemented, no mob will unlock the door
     if the trigger was activated.
   */
  if (!trigger_info)
    return;

  key = room->dir_option[rev_dir[door]]->key;

  for (tch = room->people; tch; tch = tch->next_in_room)
    {

      if (!IS_NPC (tch) || !AWAKE (tch) || GET_POS (tch) == FIGHT || !CAN_SEE (tch, tch))	/* Too dark if he can't see self. */
	continue;

      if (!has_a_key (tch))
	continue;

      if (tch->desc)
	continue;

      if (!is_brother (ch, tch) &&
	  !(IS_SET (ch->act, ACT_ENFORCER) &&
	    IS_SET (tch->act, ACT_ENFORCER)))
	continue;

      if (!IS_SET (room->dir_option[rev_dir[door]]->exit_info, EX_LOCKED)
	  || has_key (tch, NULL, key))
	{
	  one_argument (room->dir_option[rev_dir[door]]->keyword, key_name);
	  sprintf (buf, "unlock %s %s", key_name, dirs[rev_dir[door]]);
	  command_interpreter (tch, buf);
	  sprintf (buf, "open %s %s", key_name, dirs[rev_dir[door]]);
	  command_interpreter (tch, buf);
	  sprintf (buf, "%s %s", key_name, dirs[rev_dir[door]]);
	  add_second_affect (SA_CLOSE_DOOR, IS_NPC (ch) ? 5 : 10,
			     tch, NULL, buf, 0);
	  return;
	}
    }
}

void
do_accuse (CHAR_DATA * ch, char *argument, int cmd)
{
  int hours = -1;
  CHAR_DATA *victim;
  AFFECTED_TYPE *af;
  char buf[MAX_STRING_LENGTH];
  char tmpbuf[MAX_STRING_LENGTH];
  CHAR_DATA *tmp = (CHAR_DATA *) NULL;

  /*   accuse { pc | mob } [hours]                                 */

  if (IS_MORTAL (ch) && !is_area_leader (ch))
    {
      send_to_char ("You cannot accuse anyone here.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(victim = get_char_room_vis (ch, buf)))
    {

      victim = get_char (buf);

      if (!victim)
	{

	  /* Let's imms accuse no matter what */
	  if (!IS_MORTAL (ch))
	    {
	      tmp = load_pc (buf);
	      if (tmp == (CHAR_DATA *) NULL)
		{
		  send_to_char ("No PC with that name.\n", ch);
		  return;
		}
	      victim = tmp;
	    }
	  else
	    {
	      send_to_char ("Nobody is here by that name.\n", ch);
	      return;
	    }
	}

      if (IS_NPC (victim))
	{
	  send_to_char ("Who?\n", ch);
	  return;
	}
    }

  if (tmp == (CHAR_DATA *) NULL)	/* Only f the char is logged in */
    /* unfortunately, is_area_leader() requires the player to
       be logged in.  That means an imm can set an area leader
       as wanted.  WHile not perfect, its not unreasonable all
       in all. */
    if (is_area_leader (victim))
      {
	act ("$N is a leader, you can't accuse $M.",
	     false, ch, 0, victim, TO_CHAR);
	if (tmp != (CHAR_DATA *) NULL)
	  unload_pc (tmp);
	return;
      }

  argument = one_argument (argument, buf);

  if (*buf)
    {
      if (!atoi (buf))
	{
	  send_to_char ("What number of hours do you intend to use?\n", ch);
	  if (tmp != (CHAR_DATA *) NULL)
	    unload_pc (tmp);
	  return;
	}

      hours = atoi (buf);
    }

  if ((af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)))
    {

      if (hours == -1 && af->a.spell.duration == -1)
	{
	  sprintf (tmpbuf, "%s is already permanently wanted.",
		   victim->short_descr);
	  send_to_char (tmpbuf, ch);
	  if (tmp != (CHAR_DATA *) NULL)
	    unload_pc (tmp);
	  return;
	}

      if (hours == -1)
	{
	  sprintf (tmpbuf, "%s is already permanently wanted.",
		   victim->short_descr);
	  send_to_char (tmpbuf, ch);
	}

      af->a.spell.duration = hours;
    }
  else 
    {
      magic_add_affect (victim, MAGIC_CRIM_BASE + ch->room->zone,
			hours, 0, 0, 0, 0);
      if (ch->room->zone == 1)
	{
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 3,
			    hours, 0, 0, 0, 0);
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 11,
			    hours, 0, 0, 0, 0);
	}
      else if (ch->room->zone == 3)
	{
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 1,
			    hours, 0, 0, 0, 0);
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 11,
			    hours, 0, 0, 0, 0);
	}
      else if (ch->room->zone == 11)
	{
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 3,
			    hours, 0, 0, 0, 0);
	  magic_add_affect (victim, MAGIC_CRIM_BASE + 1,
			    hours, 0, 0, 0, 0);
	}
    }
  if (tmp != (CHAR_DATA *) NULL)
    unload_pc (tmp);
  send_to_char ("Ok.\n", ch);
}

void
do_pardon (CHAR_DATA * ch, char *argument, int cmd)
{
  DESCRIPTOR_DATA *td;
  CHAR_DATA *victim;
  AFFECTED_TYPE *af;
  char buf[MAX_STRING_LENGTH];
  char tmpbuf[MAX_STRING_LENGTH];
  CHAR_DATA *tmp = (CHAR_DATA *) NULL;

  if (IS_MORTAL (ch) && !is_area_leader (ch))
    {
      send_to_char ("You cannot pardon anyone here.\n\r", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!(victim = get_char_room_vis (ch, buf)))
    {

      victim = get_char_vis (ch, buf);

      if (!victim)
	{
	  if (!IS_MORTAL (ch))
	    {
	      tmp = load_pc (buf);
	      if (tmp == (CHAR_DATA *) NULL)
		{
		  send_to_char ("No PC with that name.\n", ch);
		  return;
		}
	      victim = tmp;
	    }
	  else
	    {

	      send_to_char ("Nobody is here by that name.\n\r", ch);
	      return;
	    }
	}

      if (IS_NPC (victim))
	{
	  send_to_char ("Who?\n\r", ch);
	  return;
	}
    }

  if (!(af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)))
    {

      if (IS_SET (victim->act, ACT_PARIAH))
	{
	  sprintf (tmpbuf, "%s is a pariah, and cannot be pardoned.",
		   victim->short_descr);
	  send_to_char (tmpbuf, ch);

	  if (tmp != (CHAR_DATA *) NULL)
	    unload_pc (tmp);
	  return;
	}

      sprintf (tmpbuf, "%s isn't wanted for anything.", victim->short_descr);

      send_to_char (tmpbuf, ch);

      if (tmp != (CHAR_DATA *) NULL)
	unload_pc (tmp);
      return;
    }

  if (IS_NPC (victim))
    {
      if (af->type >= MAGIC_CRIM_BASE + 1 && af->type <= MAGIC_CRIM_BASE + 99)
	prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
    }
  else
    {
      for (td = descriptor_list; td; td = td->next)
	{
	  if (td->connected != CON_PLYNG)
	    continue;
	  if (!td->character)
	    continue;
	  if (td->character == victim)
	    {
	      if (af->type >= MAGIC_CRIM_BASE + 1
		  && af->type <= MAGIC_CRIM_BASE + 99)
		prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
	      break;
	    }
	}
    }

  affect_remove (victim, af);

  if (tmp != (CHAR_DATA *) NULL)
    unload_pc (tmp);
  send_to_char ("Ok.\n\r", ch);
}

/* nod_log 

room = guard room
seeker_sdescs = (hooded) descs
seeker_name
timestamp
ig_time

store 1 ig month
*/


void
do_nod (CHAR_DATA * ch, char *argument, int cmd)
{
  int opened_a_door = 0;
  int dir;
  char buf[MAX_STRING_LENGTH];
  char key_name[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument (argument, buf);

  if (ch->room->nVirtual == AMPITHEATRE && IS_MORTAL (ch))
    {
      if (!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
	  !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand))
	{
	  send_to_char
	    ("You decide against making a commotion. PETITION to request to speak.\n",
	     ch);
	  return;
	}
    }

  if (!*buf)
    {
      act ("You nod.", false, ch, 0, 0, TO_CHAR);
      act ("$n nods.", false, ch, 0, 0, TO_ROOM);
      return;
    }

  if (!(victim = get_char_room_vis (ch, buf)))
    {
      send_to_char ("You don't see that person.\n\r", ch);
      return;
    }

  if (IS_NPC (victim) &&
      AWAKE (victim) &&
      !victim->fighting &&
      CAN_SEE (victim, ch) &&
      is_brother (ch, victim) && has_a_key (victim) && !victim->desc)
    {
      if ((get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone) 
	   || get_affect (ch, MAGIC_CRIM_HOODED + ch->room->zone)) 
	  && is_area_enforcer(victim)) 
	{
	  if (is_hooded(ch))
	    {
	      do_say (victim, "(sternly) Show yer face.", 0);
	    }
	  else 
	    {
	      do_alert (victim, "", 0);
	      do_say (victim, "(sourly) You ain't gettin' away so easy, you lout.", 0);
	    }
	  return;
	}
      argument = one_argument (argument, buf);
      
      if (!*buf)
	{
	  for (dir = 0; dir <= LAST_DIR; dir++)
	    {

	      if (!EXIT (ch, dir))
		continue;

	      if (IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED)
		  && !has_key (victim, NULL, EXIT (ch, dir)->key))
		continue;

	      one_argument (EXIT (victim, dir)->keyword, key_name);
	      sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
	      command_interpreter (victim, buf);
	      sprintf (buf, "open %s %s", key_name, dirs[dir]);
	      command_interpreter (victim, buf);
	      sprintf (buf, "%s %s", key_name, dirs[dir]);
	      add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

	      opened_a_door = 1;
	    }
	}
      else
	{
	  dir = is_direction (buf);
	  if (dir == -1 || !EXIT (ch, dir))
	    {
	      send_to_char ("There is no exit in that direction.\n", ch);
	      return;
	    }
	  if (IS_SET (EXIT (ch, dir)->exit_info, EX_LOCKED)
	      && !has_key (victim, NULL, EXIT (ch, dir)->key))
	    ;
	  else
	    {
	      one_argument (EXIT (victim, dir)->keyword, key_name);
	      sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
	      command_interpreter (victim, buf);
	      sprintf (buf, "open %s %s", key_name, dirs[dir]);
	      command_interpreter (victim, buf);
	      sprintf (buf, "%s %s", key_name, dirs[dir]);
	      add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

	      opened_a_door = 1;
	    }
	}

      if (opened_a_door)
	return;
    }

  sprintf (buf, "You nod to #5%s#0.", char_short (victim));
  act (buf, false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);

  act ("$n nods to you.", true, ch, 0, victim, TO_VICT | _ACT_FORMAT);
  act ("$n nods to $N.", true, ch, 0, victim, TO_NOTVICT | _ACT_FORMAT);
}

void
do_camp (CHAR_DATA * ch, char *argument, int cmd)
{
  int sector_type, block = 0;
  struct room_prog *p;

  if (is_switched (ch))
    return;
  if (ch->desc && ch->desc->original)
    {
      send_to_char ("Not while switched. Use RETURN first.\n", ch);
      return;
    }

  for (p = ch->room->prg; p; p = p->next)
    if (!str_cmp (p->keys, "lean-to leanto")
	&& !str_cmp (p->command, "enter"))
      block++;

  if (block)
    {
      send_to_char ("Someone else appears to have already made camp here.\n",
		    ch);
      return;
    }

  if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, SAFE_Q))
    {
      delayed_camp4 (ch);
      return;
    }

  sector_type = ch->room->sector_type;

  ///\TODO Check the necessity of this code. I believe it is a relic
  /// from a former implementation  of camp which added room progs to 
  /// the room.
  if (engine.in_build_mode ())
    {
      send_to_char ("Camping is not allowed on the build server.\n", ch);
      return;
    }


  if (sector_type != SECT_WOODS && sector_type != SECT_FOREST &&
      sector_type != SECT_FIELD && sector_type != SECT_HILLS)
    {
      send_to_char ("You can only camp in the woods, forest, a "
		    "field or the hills.\n\r", ch);
      return;
    }

/*
	send_to_char ("You'll need to pitch a tent or find suitable shelter.\n", ch);
	return;
*/

  send_to_char ("You search for a suitable location to build a lean-to.\n\r",
		ch);
  act ("$n begins looking around in the brush.", true, ch, 0, 0, TO_ROOM);

  ch->delay_type = DEL_CAMP1;
  ch->delay = 30;
}

void
delayed_camp1 (CHAR_DATA * ch)
{
  send_to_char
    ("Finding a safe location behind a tree, you begin constructing\n"
     "a frame out of fallen branches, found nearby.\n", ch);
  act ("$n starts constructing a frame using stripped\ntree branches.", false,
       ch, 0, 0, TO_ROOM);

  ch->delay_type = DEL_CAMP2;
  ch->delay = 30;
}

void
delayed_camp2 (CHAR_DATA * ch)
{
  send_to_char
    ("You begin sewing leafy branches to the frame with long pieces\n"
     "of saw grass.\n", ch);

  act ("$n begins sewing leafy branches with grass\nto the lean-to's frame.",
       false, ch, 0, 0, TO_ROOM);

  ch->delay_type = DEL_CAMP3;
  ch->delay = 30;
}

void
delayed_camp3 (CHAR_DATA * ch)
{
  ROOM_DATA *room = ch->room;

  send_to_char
    ("Finally finished, you enter your lean-to, carefully adjusting\n"
     "its covering of leaves and bark along the way.\n", ch);

  act ("Finished, $n enters $s lean-to.", true, ch, 0, 0, TO_ROOM);

  room->room_flags |= SAFE_Q;
  do_quit (ch, "", 1);
  room->room_flags &= ~SAFE_Q;
  /// \todo Can we camp without modifying the SAFE_Q room flag?
}

void
delayed_camp4 (CHAR_DATA * ch)
{
  do_quit (ch, "", 0);
}

void
knock_out (CHAR_DATA * ch, int seconds)
{
  SECOND_AFFECT *sa;

  if (GET_POS (ch) > SLEEP)
    {
      send_to_char ("You stagger to the ground, losing consciousness!\n\r",
		    ch);
      act ("$n staggers to the ground, losing consciousness.", false, ch, 0,
	   0, TO_ROOM);
      GET_POS (ch) = SLEEP;
    }

  if ((sa = get_second_affect (ch, SA_KNOCK_OUT, NULL)))
    {
      if (sa->seconds < seconds)
	sa->seconds = seconds;
      return;
    }

  add_second_affect (SA_KNOCK_OUT, seconds, ch, NULL, NULL, 0);
}

void
do_tables (CHAR_DATA * ch, char *argument, int cmd)
{
  OBJ_DATA *obj;
  CHAR_DATA *tmp;
  AFFECTED_TYPE *af_table = NULL;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int table_count = 0;

  send_to_char ("\n", ch);

  for (obj = ch->room->contents; obj; obj = obj->next_content)
    {

      if (GET_ITEM_TYPE (obj) != ITEM_CONTAINER ||
	  !IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
	continue;

      table_count++;

      sprintf (buf, "#6%s#0\n", obj_desc (obj));

      for (tmp = ch->room->people; tmp; tmp = tmp->next_in_room)
	{
	  af_table = get_affect (tmp, MAGIC_SIT_TABLE);
	  if (af_table && is_at_table (tmp, obj) && tmp != ch)
	    {
	      sprintf (buf2, "    #5%s#0 is seated here.\n",
		       CAP (char_short (tmp)));
	      strcat (buf, buf2);
	    }
	}

      send_to_char (buf, ch);
    }

  if (!table_count)
    send_to_char ("   None.\n", ch);

  return;
}

void
do_corpses (CHAR_DATA * ch, char *argument, int cmd)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH] = "";
  int corpse_count = 0;

  send_to_char ("\n", ch);

  for (obj = ch->room->contents; obj; obj = obj->next_content)
    {

      if ((obj->nVirtual != VNUM_CORPSE)
	  || !CAN_SEE_OBJ (ch, obj))
	continue;

      corpse_count++;

      sprintf (buf + strlen(buf), "#2%s#0\n", obj_desc (obj));
    }

  if (!corpse_count)
    send_to_char ("   You don't see any corpses.\n", ch);
  else
    page_string (ch->desc, buf);
    
  return;
}

/***********************
*			cover <direction>
* take cover from the < direction > east behind whatever is available,
* pmote dependant on terrain
*
*			cover  <direction> < obj>
* take cover from <direction> using <obj> as part of the pmote
*
* gives value to a flag that is used by do_aim

************************/
void 
do_cover (CHAR_DATA *ch, char *argument, int cmd)
{
  OBJ_DATA *obj = NULL;
  char buf[MAX_STRING_LENGTH] = { '\0' };
  char buf2[MAX_STRING_LENGTH] = { '\0' };
	char * dir_word = NULL;
  char * arg2 = NULL;
  char key[MAX_STRING_LENGTH] = { '\0' };
  int dir = -1;
  int key_e = 0;
  int index = 0;


  if (IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch))
    {
      send_to_char ("You cannot do this in an OOC area.\n", ch);
      return;
    }

  if (IS_SET (ch->room->room_flags, NOHIDE))
    {
      send_to_char ("This room offers no hiding spots.\n", ch);
      return;
    }

	if (!*argument)
    {
      send_to_char ("Usage: cover <direction>  or \ncover <direction><object>\n", ch);
      return;
    }


  argument = one_argument (argument, buf);
	
	for (index = 0; index < 6; index ++)
		{
		if (!strn_cmp (direct[index], buf, strlen (buf)))
			dir = index;
		}
  
  if (dir == -1)
    {
      send_to_char ("Usage: cover <direction>  or \ncover <direction><object>\n", ch);
      return;
    }

	switch (dir)
		{
		case 0:
			dir_word = str_dup("north");
		  break;
		case 1:
			dir_word = str_dup("east");
		  break;
		case 2:	
			dir_word = str_dup( "south");
		  break;
		case 3:		
			dir_word = str_dup("west");
		  break;
		case 4:	
			dir_word = str_dup("up");
		  break;
		case 5:	
			dir_word = str_dup( "down");
		  break;
		}	
/** taking cover behind an object **/
		arg2 = add_hash(argument);
		
		if (*arg2)
			{
			while (isdigit (*arg2))
				{
				key[key_e++] = *(arg2++);
				}

			if (*arg2 == '.')
				{
				key[key_e++] = *(arg2++);
				}

			while (isalpha (*arg2) || *arg2 == '-')
				{
				key[key_e++] = *(arg2++);
				}

			key[key_e] = '\0';
			key_e = 0;

			if (!(obj = get_obj_in_list_vis (ch, key, ch->room->contents)))
				{
				sprintf (buf, "I don't see %s here.\n", obj_short_desc (obj) );
				send_to_char (buf, ch);
				return;
				}

			sprintf (buf2, "%s", obj_short_desc (obj));
			}
	    

	    
	if (*buf2)
		{
		sprintf (buf, "You start trying to take cover from the %s behind #2%s#0.\n", dir_word, buf2);
		act ("$n starts trying to take cover.", false, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);
		ch->delay_info1 = dir;
		ch->delay_info2 = obj->nVirtual;
		}
	else
		{
		sprintf (buf, "You start trying to take cover from the %s.\n", dir_word);
		act ("$n starts trying to take cover.", false, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);
  	ch->delay_info1 = dir;
  	ch->delay_info2 = 0;
  	}
  	
		send_to_char (buf, ch);
		ch->delay_type = DEL_COVER;
		ch->delay = 2;  //half the delay of HIDE
}


void
delayed_cover (CHAR_DATA * ch)
{
  int mod = 0;
  int check = 0;
  int aff_type = 0;
  char *direct = NULL;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *tobj = NULL;

  ch->delay_type = 0;

	switch (ch->delay_info1)
		{
		case 0:
			direct = str_dup("north");
			aff_type = AFFECT_COVER_NORTH;
		  break;
		case 1:
			direct = str_dup("east");
			aff_type = AFFECT_COVER_EAST;
		  break;
		case 2:	
			direct = str_dup( "south");
			aff_type = AFFECT_COVER_SOUTH;
		  break;
		case 3:		
			direct = str_dup("west");
			aff_type = AFFECT_COVER_WEST;
		  break;
		case 4:	
			direct = str_dup("up");
			aff_type = AFFECT_COVER_UP;
		  break;
		case 5:	
			direct = str_dup( "down");
			aff_type = AFFECT_COVER_DOWN;
		  break;
		}	

		if (ch->delay_info2 == 0)
				{
				sprintf (buf, "You take cover from the %s.\n", direct);
				send_to_char (buf, ch);
		
				sprintf (buf, "has taken cover from the %s.", direct);
  	    GET_POS (ch) = SIT;
				ch->pmote_str = add_hash(buf);
				}
		else
				{
				tobj = vtoo(ch->delay_info2);
				sprintf (buf, "You take cover from the %s, hiding behind #2%s#0.\n",
											 direct, obj_short_desc (tobj));
				send_to_char (buf, ch);
			
				sprintf (buf, "has taken cover from the %s by hiding behind %s.",
											 direct, obj_short_desc (tobj));
				ch->pmote_str = add_hash(buf);
        GET_POS (ch) = SIT;
				}			
				
  switch (ch->room->sector_type) //how much cover in the terrain
    {
    case SECT_INSIDE:  //tables, chairs
      mod = 0;
      break;
    case SECT_CITY:		//walls, alleys
      mod = 10;
      break;
    case SECT_ROAD:		//wide open for travel
      mod = -30;
      break;
    case SECT_TRAIL:	//open for travel
      mod = -20;
      break;
    case SECT_FIELD:		//open for farming
      mod = -5;
      break;
    case SECT_WOODS:		// some trees
      mod = 10;
      break;
    case SECT_FOREST:		//big trees
      mod = 20;
      break;
    case SECT_HILLS:		//ravines, boulders
      mod = 25;
      break;
    case SECT_MOUNTAIN:	//ravines, big boulders
      mod = 30;
      break;
    case SECT_SWAMP:		//trees
      mod = 10;				
      break;
    case SECT_PASTURE:	//ditches, fences
      mod = 5;
      break;
    case SECT_HEATH:	//ditches, small boulders
      mod = 10;
      break;
    }

		if (IS_SET (ch->room->room_flags, RUINS))
			mod += 30;
			
		if (IS_ENCUMBERED (ch))
			mod -= 5;
			
		if (get_equip (ch, WEAR_SHIELD))
			mod += 5;
			
			
		//50% chance to find cover inside, 80% in mountains
		check = number (1, 100);		
  	check = check + mod;

/** If the character is -taking cover-, they can only be under 1 cover at a time. **/
		if (check > 50)
    	{
    	remove_cover(ch, 0);  
      magic_add_affect (ch, aff_type, -1, 0, 0, 0, 0);
      ch->cover_from_dir = ch->delay_info1;
      sprintf (buf, "[%s covered from %s]", ch->tname, direct);
      act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
  		}
  	return;
}

/* Type 0 is remove all covers
*  type 1-6 removes cover from that direction
*/
void
remove_cover (CHAR_DATA *ch, int type)
{
		int index = 0;
		
		
		if (type == 0)
			{
			for(index = AFFECT_COVER_NORTH;
					index <= AFFECT_COVER_DOWN;
					index ++)
					{
					remove_affect_type (ch, index);
					//cover from this direction is no cover at all
					ch->cover_from_dir = -1; 
					}
			return;
			}
	
		else if ((type >= AFFECT_COVER_NORTH) &&
						 (type <= AFFECT_COVER_DOWN))
			{
			remove_affect_type (ch, type);
			//cover from this direction is no cover at all
			ch->cover_from_dir = -1;
			return;
			}		
			
	return;
}

int
under_cover (CHAR_DATA *ch)
{
		int index = 0;
		
		
		for(index = AFFECT_COVER_NORTH;
				index <= AFFECT_COVER_DOWN;
				index ++)
					{
					if (get_affect (ch, index))
						return (1);
					}

	return (0);
}

// Command Ownership, for transfering ownership of mobs 
// Syntax: OWNERSHIP TRANSFER <mob> <character> or OWNERSHIP SET <mob> <character>

void do_ownership (CHAR_DATA *ch, char *argument, int command)
{
	CHAR_DATA *property, *target;
	std::string ArgumentList = argument;
	std::string ThisArgument;
	std::string Output;
	bool transfer = true;
	
	ArgumentList = one_argument(ArgumentList, ThisArgument);
	
	if ((ThisArgument.find("?", 0) != std::string::npos) || ThisArgument.empty())
	{
		send_to_char("Syntax is:\n", ch);
		send_to_char("ownership transfer <mob> <character>\n", ch);
		send_to_char("(Staff level)\nownership set <mob> <character> \n", ch);
		return;
	}
	
	if (ThisArgument.find("set", 0) != std::string::npos)
	{
		transfer = false;
		
		if (!GET_TRUST(ch))
			{
				send_to_char ("You can only transfer ownership.\n", ch);
				return;
			}
	}
	
	ArgumentList = one_argument(ArgumentList, ThisArgument);
	
	if (ThisArgument.empty())
	{
		send_to_char ("Which individual do you wish to transfer ownership of?\n", ch);
		return;
	}
	
	property = get_char_room_vis (ch, ThisArgument.c_str());
	
	if (!property)
	{
		if (GET_TRUST(ch))
			{
				property = get_char ((char*)ThisArgument.c_str());
			}
			
		if (!property)
			{
		send_to_char ("Cannot find mobile with keyword \"#2", ch);
		send_to_char (ThisArgument.c_str(), ch);
		send_to_char ("#0\".\n", ch);
		return;		
	}
	}
	
	if (!IS_NPC(property))
	{
		send_to_char ("You have no authority to deliniate the ownership of a PC.\n", ch);
		return;
	}
	
	if (IS_NPC(property) && !property->mob->owner  && !GET_TRUST(ch))
	{
		send_to_char ("You have no authority to deliniate the ownership of this individual.\n", ch);
		return;
	}
	
	if (property->mob->owner && strcmp(property->mob->owner, ch->tname) && !GET_TRUST(ch))
	{
		send_to_char ("You have no authority to deliniate the ownership of this individual.\n", ch);
		return;
	}
	
	ArgumentList = one_argument(ArgumentList, ThisArgument);
	
		
	if (ThisArgument.empty())
	{
		send_to_char ("Transfer the ownership to whom?\n", ch);
		return;
	}
		
	target = get_char_room_vis(ch, ThisArgument.c_str());
	
		if (!target)
		{
			if (GET_TRUST(ch))
			{
				target = get_char ((char*)ThisArgument.c_str());
			}
			
			send_to_char ("You do not see a person with the keyword \"#2", ch);
			send_to_char (ThisArgument.c_str(), ch);
			send_to_char ("#0\" to transfer #5", ch);
			send_to_char (char_short(property), ch);
			send_to_char ("#0 to.\n", ch);
			return;
		}
	
	if (!transfer && GET_TRUST(ch))
		{
		
			ThisArgument[0] = toupper(ThisArgument[0]);
			property->mob->owner = str_dup (ThisArgument.c_str());
			send_to_char ("Setting ownership of #5", ch);
			send_to_char (char_short(property), ch);
			send_to_char ("#0 to \"#2", ch);
			send_to_char (ThisArgument.c_str(), ch);
			send_to_char ("#0\".", ch);
			return;
			
		}
		
	else
		{
					
		ArgumentList = one_argument(ArgumentList, ThisArgument);
		
		if (IS_NPC(target) && (ThisArgument.find('!') == std::string::npos))
		{
			send_to_char ("You are proposing to transfer ownership of #5", ch);
			send_to_char (char_short(property), ch);
			send_to_char ("#0 to #5", ch);
			send_to_char (char_short(target), ch);
			send_to_char ("#0, who is an NPC. Please confirm by typing #6OWNERSHIP TRANSFER <property> <target> !#0\n", ch);
			return;
		}
		
		
		Output.assign(target->tname);
		Output[0] = toupper(Output[0]);
		property->mob->owner = str_dup (Output.c_str());
		send_to_char ("You transfer ownership of #5", ch);
		send_to_char (char_short(property), ch);
		send_to_char ("#0 to #5", ch);
		send_to_char (char_short(target), ch);
		send_to_char ("#0.\n", ch);
		Output.assign("#5");
		Output.append(char_short(ch));
		Output.append("#0 transfers ownership of #5");
		Output.append(char_short(property));
		Output.append("#0 to you.\n");
		Output[2] = toupper(Output[2]);
		send_to_char (Output.c_str(), target);
		return;
	}
	return;
}


	
