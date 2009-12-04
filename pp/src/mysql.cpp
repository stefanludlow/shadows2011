/*------------------------------------------------------------------------\
|  mysql.c : mySQL Interface Module                   www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <sstream>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <mysql/mysql.h>
#include <dirent.h>
#include <signal.h>

#include "server.h"

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"

extern RACE_TABLE_ENTRY *entry;
extern CHAR_DATA *loaded_list;
extern std::multimap<int, room_prog> mob_prog_list;
extern std::multimap<int, room_prog> obj_prog_list;

RACE_TABLE_ENTRY *race_table = NULL;

extern MYSQL *database;
MYSQL *database;

bool mysql_logging = true;
extern int booting;
extern int finished_booting;
extern rpie::server engine;

int last_vnpc_sale=0;

void
init_mysql (void)
{
  if (!(database = mysql_init (database)))
    {
      fprintf (stderr, "The library call 'mysql_init' failed "
	       "to initialize the MySQL handle for the following reason: %s\n",
	       mysql_error (database));
      exit (1);      
    }

  if (!(mysql_real_connect 
	(database, 
	 engine.get_config ("mysql_host").c_str (), 
	 engine.get_config ("mysql_user").c_str (), 
	 engine.get_config ("mysql_passwd").c_str (), 
	 0, 0, 0, 0)))
    {
      fprintf (stderr, "mysql_real_connect: %s\n", mysql_error (database));
      exit (1);
    }

  std::string engine_db = engine.get_config ("engine_db");
  if ((mysql_select_db (database, engine_db.c_str ())) != 0)
    {
      fprintf (stderr, "mysql_select_db: %s\n", mysql_error (database));
      exit (1);
    }

  system_log ("MySQL connection initialized.", false);
}

void
refresh_db_connection (void)
{
  mysql_close (database);

  init_mysql ();
}

// Route ALL mysql queries through this wrapper to ensure they are escaped
// properly, to thwart various SQL injection attacks.

int
mysql_safe_query (char *fmt, ...)
{
  va_list argp;
  int i = 0;
  double j = 0;
  char *s = 0, *out = 0, *p = 0;
  char safe[2*MAX_STRING_LENGTH];
  char query[2*MAX_STRING_LENGTH];

  *query = '\0';
  *safe = '\0';

  va_start (argp, fmt);

  for (p = fmt, out = query; *p != '\0'; p++)
    {
      if (*p != '%')
	{
	  *out++ = *p;
	  continue;
	}

      switch (*++p)
	{
	case 'c':
	  i = va_arg (argp, int);
	  out += sprintf (out, "%c", i);;
	  break;
	case 's':
	  s = va_arg (argp, char *);
	  if (!s)
	    {
	      out += sprintf (out, "");
	      break;
	    }
	  mysql_real_escape_string (database, safe, s, strlen (s));
	  out += sprintf (out, "%s", safe);
	  break;
	case 'd':
	  i = va_arg (argp, int);
	  out += sprintf (out, "%d", i);
	  break;
	case 'f':
	  j = va_arg (argp, double);
	  out += sprintf (out, "%f", j);
	  break;
	case '%':
	  out += sprintf (out, "%%");
	  break;
	}
    }

  *out = '\0';

  va_end (argp);

  int result = mysql_real_query (database, query, strlen (query));
//  if (mysql_errno(database))
//    {
//      fprintf (stderr, "The library call 'mysql_real_query' failed to run "
//	       "the query '%s' for the following reason: %s\n",
//	       query, mysql_error (database));
//    }
  return (result);
} 

/* int mysql_safe_query (char *queryFormat, ...)
{
	char *arg;
	std::string queryStr = queryFormat;
	size_t position = 0;

	va_start (arg, queryFormat);
	
	while ((position = queryStr.find('%', position)) != std::string::npos) {
		if (queryStr.length() > position) {
			std::ostringstream tempDataStream;
			std::string safeString;
			char *s = '\0';
			
			switch (queryStr[position+1]) {
				case 'c':
					tempDataStream << va_arg(arg,char);
					break;
				
				case 'd':
				case 'i':
					tempDataStream << va_arg(arg,int);
					break;
				
				case 's':
					s = va_arg(arg,char *);

					if (!s) {
						tempDataStream << "";
						break;
					}
					safeString = s;

					if (safeString.empty()) {
						tempDataStream << "";
						break;
					}
					s = new char[(safeString.length()*2)+1]; // Requires at worst 2 bytes per character plus terminating character
					mysql_real_escape_string(database, s, safeString.c_str(), (unsigned long)safeString.length());
					tempDataStream << s;
					delete [] s;
					s = '\0';
					break;
				
				case 'f':
					tempDataStream << va_arg(arg,double);
					break;
			}
			if (tempDataStream.str().empty()) {
				position++;
				continue;
			}
			queryStr.replace(position, 2, tempDataStream.str());
			position = position + tempDataStream.str().length();
		}
		else {
			break;
		}
	}

	va_end(arg);

	int result = mysql_real_query (database, queryStr.c_str(), (unsigned long)queryStr.length());
	return (result);
} */

void load_obj_progs (void)
{
	MYSQL_RES *result;
	MYSQL_ROW row;

	mysql_safe_query ("SELECT vnum, cmd, keywords, prog, type FROM obj_progs");

	if (!(result = mysql_store_result (database)))
		return;

	while (row = mysql_fetch_row(result))
	{
		room_prog prog;
		prog.command = row[1];
		if (!strcmp(row[2], " ")) // God knows why this is the case?
		{
			prog.keys = "";
		}
		else
		{
			prog.keys = row[2];
		}
		prog.prog = row[3];
		prog.type = atoi(row[4]);
		obj_prog_list.insert (std::pair<int, room_prog>(atoi(row[0]), prog));
	}
}

void save_obj_progs (void)
{
	if (!engine.in_build_mode())
		return;
	mysql_safe_query ("INSERT INTO obj_progs_backup SELECT *, NOW() FROM obj_progs");
	mysql_safe_query ("DELETE FROM obj_progs");
	for (std::multimap<int, room_prog>::iterator it = obj_prog_list.begin(); it != obj_prog_list.end(); it++)
	{
		mysql_safe_query ("INSERT INTO obj_progs (vnum, cmd, keywords, prog, type) VALUES('%d', '%s', '%s', '%s', '%d')", it->first, it->second.command, it->second.keys, it->second.prog, it->second.type);
	}
}

void load_mob_progs (void)
{
	MYSQL_RES *result;
	MYSQL_ROW row;

	mysql_safe_query ("SELECT vnum, cmd, keywords, prog, type FROM mob_progs");

	if (!(result = mysql_store_result (database)))
		return;

	while (row = mysql_fetch_row(result))
	{
		room_prog prog;
		prog.command = row[1];
		prog.keys = row[2];
		prog.prog = row[3];
		prog.type = atoi(row[4]);
		mob_prog_list.insert (std::pair<int, room_prog>(atoi(row[0]), prog));
	}
}

void save_mob_progs (void)
{
	if (!engine.in_build_mode())
		return;
	mysql_safe_query ("INSERT INTO mob_progs_backup SELECT * FROM mob_progs");
	mysql_safe_query ("DELETE FROM mob_progs");
	for (std::multimap<int, room_prog>::iterator it = mob_prog_list.begin(); it != mob_prog_list.end(); it++)
	{
		mysql_safe_query ("INSERT INTO mob_progs (vnum, cmd, keywords, prog, type) VALUES('%d', '%s', '%s', '%s', '%d')", it->first, it->second.command, it->second.keys, it->second.prog, it->second.type);
	}
}


// Loads the master race table containing all race defines at boot

void
load_race_table (void)
{
  RACE_TABLE_ENTRY *entry, *tmp_entry;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int i = 0;

  mysql_safe_query ("SELECT * FROM races ORDER BY id ASC");

  if (!(result = mysql_store_result (database)))
    return;

  while ((row = mysql_fetch_row (result)) && i <= MAX_NUM_RACES)
    {
      i++;

      CREATE (entry, RACE_TABLE_ENTRY, 1);
      entry->id = atoi (row[RACE_ID]);
      entry->name = str_dup (row[RACE_NAME]);
      entry->pc_race = atoi (row[RACE_PC]);
      entry->starting_locs = atoi (row[RACE_START_LOC]);
      entry->rpp_cost = atoi (row[RACE_RPP_COST]);
      entry->created_by = str_dup (row[RACE_CREATED_BY]);
      entry->last_modified = atoi (row[RACE_LAST_MODIFIED]);
      entry->race_size = atoi (row[RACE_SIZE]);
      entry->body_proto = atoi (row[RACE_BODY_PROTO]);
      entry->innate_abilities = atoi (row[RACE_AFFECTS]);
      entry->str_mod = atoi (row[RACE_STR_MOD]);
      entry->dex_mod = atoi (row[RACE_DEX_MOD]);
      entry->agi_mod = atoi (row[RACE_AGI_MOD]);
      entry->con_mod = atoi (row[RACE_CON_MOD]);
      entry->wil_mod = atoi (row[RACE_WIL_MOD]);
      entry->int_mod = atoi (row[RACE_INT_MOD]);
      entry->aur_mod = atoi (row[RACE_AUR_MOD]);
      entry->native_tongue = atoi (row[RACE_NATIVE_TONGUE]);
      entry->min_age = atoi (row[RACE_MIN_AGE]);
      entry->max_age = atoi (row[RACE_MAX_AGE]);
      entry->min_ht = atoi (row[RACE_MIN_HEIGHT]);
      entry->max_ht = atoi (row[RACE_MAX_HEIGHT]);
      entry->fem_ht_adj = atoi (row[RACE_FEM_HT_DIFF]);

      if (!race_table)
	race_table = entry;
      else
	for (tmp_entry = race_table; tmp_entry; tmp_entry = tmp_entry->next)
	  {
	    if (!tmp_entry->next)
	      {
		tmp_entry->next = entry;
		break;
	      }
	  }
    }
}

// Returns race database id, if any, for specified race name

int
lookup_race_id (const char *name)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  static int id = -1;

  if (!name || !*name)
    return -1;

  mysql_safe_query ("SELECT * FROM races WHERE name = '%s'", name);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result != NULL)
	mysql_free_result (result);
      return -1;
    }

  row = mysql_fetch_row (result);

  id = atoi (row[RACE_ID]);

  mysql_free_result (result);

  return id;
}

// Returns the specified value for the specified race id

char *
lookup_race_variable (int id, int which_var)
{
  RACE_TABLE_ENTRY *entry;
  MYSQL_RES *result;
  MYSQL_ROW row;
  static char value[MAX_STRING_LENGTH];
  int looked_up = 0;

  if (id < 0 || which_var < RACE_NAME || which_var > RACE_LAST_MODIFIED)
    {
      if (which_var == RACE_NAME)
	return "Unknown";
      else if (which_var == RACE_NATIVE_TONGUE)
	return "Westron";
      else
	return NULL;
    }

  // Pull from race table on bootup for faster loading of NPCs; thereafter, pull race
  // defines directly from updated versions in database.

  if (booting)
    {
      for (entry = race_table; entry; entry = entry->next)
	{
	  if (entry->id != id)
	    continue;
	  if (which_var == RACE_NAME)
	    return entry->name;
	  else if (which_var == RACE_SIZE)
	    looked_up = entry->race_size;
	  else if (which_var == RACE_BODY_PROTO)
	    looked_up = entry->body_proto;
	  else if (which_var == RACE_AFFECTS)
	    looked_up = entry->innate_abilities;
	  else if (which_var == RACE_NATIVE_TONGUE)
	    looked_up = entry->native_tongue;
	  else if (which_var == RACE_MIN_AGE)
	    looked_up = entry->min_age;
	  else if (which_var == RACE_MAX_AGE)
	    looked_up = entry->max_age;
	  else if (which_var == RACE_MIN_HEIGHT)
	    looked_up = entry->min_ht;
	  else if (which_var == RACE_MAX_HEIGHT)
	    looked_up = entry->max_ht;
	  else if (which_var == RACE_FEM_HT_DIFF)
	    looked_up = entry->fem_ht_adj;
	  if (looked_up)
	    {
	      sprintf (value, "%d", looked_up);
	      return value;
	    }
	  break;
	}
    }

  *value = '\0';

  mysql_safe_query ("SELECT * FROM races WHERE id = %d", id);
  if ((result = mysql_store_result (database)))
    {
      if (mysql_num_rows (result))
	{
	  if ((row = mysql_fetch_row (result)))
	    {
	      strcpy (value, row[which_var] != NULL ? row[which_var] : "");
	    }
	}
      else
	{
	  if (which_var == RACE_NAME)
	    {
	      strcpy (value, "Unknown");
	    }
	  else if (which_var == RACE_NATIVE_TONGUE)
	    {
	      strcpy (value, "Westron");
	    }
	}
      mysql_free_result (result);
    }
  else
    {
      std::ostringstream error_message;
      error_message << "Error: " << std::endl
		    << __FILE__ << ':'
		    << __func__ << '('
		    << __LINE__ << "):" << std::endl
		    << mysql_error (database);
      const char *error_cstr = (error_message.str ()).c_str();
      send_to_gods (error_cstr);
      system_log (error_cstr, true);
    }
  return value;
}

MYSQL_RES *
mysql_player_search (int search_type, char *string, int timeframe)
{
  char query[MAX_STRING_LENGTH];
  static MYSQL_RES *result;
  int ind = 0;
  std::string player_db = engine.get_config ("player_db");
  if (search_type == SEARCH_CLAN) 
    {
      sprintf (query, 
	       "SELECT account, name, sdesc, create_state,"
	       " TRIM(BOTH '\\'' FROM SUBSTRING_INDEX(LEFT(clans,LOCATE('%s',clans)-2),' ',-1)) AS rank"
	       " FROM %s.pfiles WHERE ", 
	       string, player_db.c_str ());
    }
  else
    {
      sprintf (query, 
	       "SELECT account, name, sdesc, create_state "
	       " FROM %s.pfiles WHERE ", 
	       player_db.c_str ());
    }

  if (timeframe)
    {
    /* all, day, week, month, fortnight */
      int timeframes[5] = { 0, 86400, 604800, 2592000, 1209600 };
      sprintf (query + strlen (query), "lastlogon > (UNIX_TIMESTAMP() - %d) AND ", 
	       timeframes[timeframe]);
    }

  if (search_type != SEARCH_LEVEL)
    {
      strcpy (query + strlen (query), "level = 0 AND ");
    }

  if (search_type == SEARCH_KEYWORD)
    sprintf (query + strlen (query), "keywords LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_SDESC)
    sprintf (query + strlen (query), "sdesc LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_LDESC)
    sprintf (query + strlen (query), "ldesc LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_FDESC)
    sprintf (query + strlen (query), "description LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_CLAN)
    sprintf (query + strlen (query), "clans LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_SKILL)
    sprintf (query + strlen (query), "skills LIKE '%%%s%%'", string);
  else if (search_type == SEARCH_ROOM)
    sprintf (query + strlen (query), "room = %s", string);
  else if (search_type == SEARCH_LEVEL)
    sprintf (query + strlen (query), "level = %s", string);
  else if (search_type == SEARCH_RACE)
    {
      if ((ind = lookup_race_id (string)) == -1)
	return NULL;
      sprintf (query + strlen (query), "race = %d", ind);
    }
  else
    return NULL;

  /* ORDER BY create_state, name */
  sprintf (query + strlen (query),
	   " ORDER BY ABS(FLOOR((create_state - 2.1)*2)), name ASC");

  // Again, use real_query() here rather than safe_query() because the latter is
  // impractical and this command is for staff use only.
  fprintf(stderr, "%s\n", query);

  mysql_real_query (database, query, strlen (query));

  result = mysql_store_result (database);

  return result;
}

// Returns the specified value for the specified spell id

char *
lookup_spell_variable (int id, int which_var)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  static char value[MAX_STRING_LENGTH];

  if (id < 1 || which_var < VAR_ID || which_var > VAR_LAST_MODIFIED)
    {
      return NULL;
    }

  *value = '\0';

  mysql_safe_query ("SELECT * FROM spells WHERE id = %d", id);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result != NULL)
	mysql_free_result (result);
      return NULL;
    }

  row = mysql_fetch_row (result);

  sprintf (value, "%s", row[which_var] != NULL ? row[which_var] : "");

  mysql_free_result (result);

  return value;
}

// Returns the spell ID from the database for the named spell; -1 if not found

int
lookup_spell_id (char *name)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  static int id = 0;

  if (!name || !*name)
    return -1;

  mysql_safe_query ("SELECT * FROM spells WHERE name LIKE '%s%%'", name);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result != NULL)
	mysql_free_result (result);
      return -1;
    }

  row = mysql_fetch_row (result);

  id = atoi (row[VAR_ID]);

  mysql_free_result (result);

  return id;
}

// Returns the spell name from the database for the given id; -1 if not found

char *
spell_name_lookup (int id)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  static char buf[MAX_STRING_LENGTH];

  if (id < 1)
    return NULL;

  mysql_safe_query ("SELECT * FROM spells WHERE id = %d", id);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    return NULL;

  row = mysql_fetch_row (result);

  sprintf (buf, "%s", row[1]);

  mysql_free_result (result);

  return buf;
}

void
system_log (const char *str, bool error)
{
  char buf[MAX_STRING_LENGTH];
  char sha_buf[MAX_STRING_LENGTH];
  int timestamp = 0;

  if (!str || !*str || !mysql_logging)
    return;

  sprintf (buf, "%s", str);

  if (buf[strlen (buf) - 1] == '\n')
    buf[strlen (buf) - 1] = '\0';

  timestamp = (int) time (0);

  sprintf (sha_buf, "%d %s", timestamp, buf);

  std::string log_db = engine.get_config ("player_log_db");
  int port = engine.get_port ();
  mysql_safe_query
    ("INSERT INTO %s.mud (name, timestamp, port, room, error, entry) "
     "VALUES ('System', %d, %d, -1, %d, '%s')", 
     log_db.c_str (),
     timestamp, port, (int) error, buf);
}

void
player_log (CHAR_DATA * ch, char *command, char *str)
{
  char buf[MAX_STRING_LENGTH];
  char sha_buf[MAX_STRING_LENGTH];
  int timestamp = 0;

  if (!str || !*str || !command || !*command || !ch || ch->deleted
      || !mysql_logging)
    return;

  sprintf (buf, "%s", str);

  if (buf[strlen (buf) - 1] == '\n')
    buf[strlen (buf) - 1] = '\0';

  timestamp = (int) time (0);

  sprintf (sha_buf, "%d %s %s", timestamp, command, buf);

  std::string log_db = engine.get_config ("player_log_db");
  int port = engine.get_port ();
  mysql_safe_query
    ("INSERT INTO %s.mud (name, account, switched_into, timestamp, port, room, guest, immortal, command, entry) "
     "VALUES ('%s', '%s', '%s', %d, %d, %d, %d, %d, '%s', '%s')",
     log_db.c_str (), ch->desc
     && ch->desc->original ? ch->desc->original->tname : ch->tname, ch->pc
     && ch->pc->account_name ? ch->pc->account_name : "", ch->desc
     && ch->desc->original ? ch->tname : "", timestamp, port, ch->in_room,
     IS_SET (ch->flags, FLAG_GUEST) ? 1 : 0, GET_TRUST (ch) > 0
     && !IS_NPC (ch) ? 1 : 0, command, buf);

  // feed to stdout for any on-server, realtime monitoring

  printf ("%s [%d]: %s %s\n", ch->tname, ch->in_room, command, str);

  fflush (stdout);
}

/*void system_log (const char *str, bool error)
{
	static std::stringstream log_dump;// = std::stringstream();
	static int logs_count = 0;
	std::string log_db = engine.get_config ("player_log_db");
	int port = engine.get_port ();
//	if (!str || !*str || !mysql_logging)
//		return;
	if(logs_count == 0)
	{
		log_dump.clear();
		log_dump<<"INSERT INTO "<<log_db.c_str()<<".mud (name, timestamp, port, room, error, entry) VALUES ";
	}
	log_dump<<"('System', "<<((int) time (0))<<", "<<port<<", "<<((int) error)<<", "<<str<<")";
	if(++logs_count < 100)
		log_dump<<", ";
	else
	{
		mysql_safe_query((char*)log_dump.str().c_str());
		log_dump.flush();
		log_dump.clear();
		log_dump.str(" ");
		log_dump.seekp(0);
		logs_count = 0;
	}
}

void player_log (CHAR_DATA * ch, char *command, char *str)
{
	static std::stringstream log_dump;// = std::stringstream();
	static int logs_count = 0;
	std::string log_db = engine.get_config ("player_log_db");
	int port = engine.get_port ();
	if (!str || !*str || !command || !*command || !ch || ch->deleted || !mysql_logging)
		return;
	if(logs_count == 0)
	{
		log_dump.clear();
		log_dump<<"INSERT INTO "<<log_db.c_str()<<".mud (name, account, switched_into, timestamp, port, room, guest, immortal, command, entry) VALUES ";
	}
	log_dump<<"('"<<((ch->desc && ch->desc->original) ? ch->desc->original->tname : ch->tname)
		<<"', '"<<((ch->pc && ch->pc->account_name) ? ch->pc->account_name : "")<<"', '"
		<<((ch->desc && ch->desc->original) ? ch->tname : "")<<"', "<<((int) time (0))<<", "
		<<port<<", "<<ch->in_room<<", "<<(IS_SET (ch->flags, FLAG_GUEST) ? 1 : 0)<<", "
		<<((GET_TRUST (ch) > 0 && !IS_NPC (ch)) ? 1 : 0)<<", '"<<command<<"', '"<<str<<"') ";
	if(++logs_count < 50)
		log_dump<<", ";
	else
	{
//		send_to_room((char*)log_dump.str().c_str(), 686);
		mysql_safe_query((char*)log_dump.str().c_str());
		log_dump.flush();
		log_dump.clear();
		log_dump.str(" ");
		log_dump.seekp(0);
		logs_count = 0;
	}
}*/

void
add_profession_skills (CHAR_DATA * ch, char *skill_list)
{
  int ind = 0;
  char buf[MAX_STRING_LENGTH];

  while (*skill_list)
    {
      skill_list = one_argument (skill_list, buf);
      if ((ind = index_lookup (skills, buf)) == -1)
	continue;
      ch->skills[ind] = 1;
      ch->pc->skills[ind] = 1;
    }
}

/*                                                                          *
 * function: get_profession_name                                            *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - fixed instance where mysql_res was not freed          *
 *                                                                          */
char *
get_profession_name (int prof_id)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];
  static char prof[MAX_STRING_LENGTH];
  int nProfessions = 0;

  sprintf (prof, "None");

  mysql_safe_query ("SELECT name FROM professions WHERE id = %d", prof_id);

  if ((result = mysql_store_result (database)) == NULL)
    {
      sprintf (buf, "Warning: get_profession_name(): %s",
	       mysql_error (database));
      system_log (buf, true);
    }
  else
    {
      nProfessions = mysql_num_rows (result);
      row = mysql_fetch_row (result);
      if (nProfessions > 0 && row != NULL)
	{
	  sprintf (prof, "%s", row[0]);
	}
      mysql_free_result (result);
      result = NULL;
    }

  return prof;
}


int
is_restricted_profession (CHAR_DATA * ch, char *skill_list)
{
  int ind = 0;
  char buf[MAX_STRING_LENGTH];

  while (*skill_list)
    {
      skill_list = one_argument (skill_list, buf);
      if ((ind = index_lookup (skills, buf)) == -1)
	continue;
      if ((is_restricted_skill (ch, ind)) && restricted_skills[ind] != -5)
	return 1;
    }

  return 0;
}

/*                                                                          *
 * function: do_professions                                                 *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - fixed instance where mysql_res was not freed          *
 *                  - modified 'delete' to show how many professions were   *
 *                    deleted, or none, if that were the case.              *
 *                                                                          */
void
do_professions (CHAR_DATA * ch, char *argument, int cmd)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char b_buf[MAX_STRING_LENGTH];
  char skill_list[MAX_STRING_LENGTH];
  char profession[MAX_STRING_LENGTH];
  int ind = 0, i = 1, skill_num = 0;
  int nProfessions = 0;

  argument = one_argument (argument, buf);

  if (!str_cmp (buf, "delete"))
    {
      if (!*argument)
	{
	  send_to_char
	    ("Please include the name of the profession you wish to delete.\n",
	     ch);
	  return;
	}

      argument = one_argument (argument, buf);

      mysql_safe_query ("DELETE FROM professions WHERE name = '%s'", buf);

      nProfessions = mysql_affected_rows (database);
      if (nProfessions)
	{
	  sprintf (buf2,
		   "%d profession%s matching the name '%s' %s deleted.\n",
		   nProfessions, (nProfessions == 1) ? "" : "s", buf,
		   (nProfessions == 1) ? "was" : "were");
	  send_to_char (buf2, ch);
	  return;
	}
      sprintf (buf2, "No professions matching the name '%s' were found.\n",
	       buf);
      send_to_char (buf2, ch);
      return;
    }

  if (!str_cmp (buf, "list"))
    {
      mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
      if ((result = mysql_store_result (database)) != NULL)
	{
	  if (mysql_num_rows (result) > 0)
	    {
	      sprintf (b_buf, "#2Currently Defined Professions:#0\n\n");
	      while ((row = mysql_fetch_row (result)))
		{
		  if (i < 10)
		    sprintf (b_buf + strlen (b_buf),
			     "   %d.  #2%15s#0: [%4d] %s\n", i, row[0],
			     atoi (row[3]), row[1]);
		  else
		    sprintf (b_buf + strlen (b_buf),
			     "   %d. #2%15s#0: [%4d] %s\n", i, row[0],
			     atoi (row[3]), row[1]);
		  i++;
		}
	      page_string (ch->desc, b_buf);
	    }
	  else
	    {
	      send_to_char
		("There are no professions currently in the database.\n", ch);
	    }
	  mysql_free_result (result);
	  result = NULL;
	  return;
	}
      else
	{
	  sprintf (buf, "Warning: do_professions(): %s",
		   mysql_error (database));
	  system_log (buf, true);
	}
      send_to_char ("An error occurred while trying to list professions.\n",
		    ch);
      return;
    }

  if (!str_cmp (buf, "add"))
    {

      if (!*argument)
	{
	  send_to_char ("Add which profession?\n", ch);
	  return;
	}

      argument = one_argument (argument, profession);

      if (!*argument)
	{
	  send_to_char ("Which skills should this profession start with?\n",
			ch);
	  return;
	}

      *skill_list = '\0';

      while (*argument)
	{
	  skill_num++;
	  if (skill_num > 6)
	    {
	      send_to_char
		("You can only define a set of 6 skills per profession.\n",
		 ch);
	      return;
	    }
	  argument = one_argument (argument, buf);
	  if ((ind = index_lookup (skills, buf)) == -1)
	    {
	      sprintf (buf2,
		       "I couldn't find the '%s' skill in our database. Aborted.\n",
		       buf);
	      send_to_char (buf2, ch);
	      return;
	    }
	  if (*skill_list)
	    sprintf (skill_list + strlen (skill_list), " %s", buf);
	  else
	    sprintf (skill_list + strlen (skill_list), "%s", buf);
	}

      mysql_safe_query
	("INSERT INTO professions (name, skill_list) VALUES ('%s', '%s')",
	 profession, skill_list);

      send_to_char ("Profession has been added to the database.\n", ch);
      return;
    }

  send_to_char ("Usage: profession (add | delete | list) (<arguments>)\n",
		ch);
}

void
set_hobbitmail_flags (int id, int flag)
{
  mysql_safe_query ("UPDATE hobbitmail SET flags = %d WHERE id = %d", flag,
		    id);
}

void
update_weapon_skills (OBJ_DATA * obj)
{
  if (GET_ITEM_TYPE (obj) != ITEM_WEAPON)
    return;
  if (IS_SET (obj->obj_flags.extra_flags, ITEM_NEWSKILLS))
    return;

  obj->o.od.value[3] += 1;

  if (obj->o.od.value[3] == SKILL_STAFF)
    obj->o.od.value[3] = SKILL_SHORTBOW;
  else if (obj->o.od.value[3] == SKILL_POLEARM)
    obj->o.od.value[3] = SKILL_LONGBOW;
  else if (obj->o.od.value[3] == SKILL_THROWN)
    obj->o.od.value[3] = SKILL_CROSSBOW;
  else if (obj->o.od.value[3] == SKILL_BLOWGUN)
    obj->o.od.value[3] = SKILL_STAFF;
  else if (obj->o.od.value[3] == SKILL_SLING)
    obj->o.od.value[3] = SKILL_STAFF;
  else if (obj->o.od.value[3] == SKILL_SHORTBOW)
    obj->o.od.value[3] = SKILL_POLEARM;

  obj->obj_flags.extra_flags |= ITEM_NEWSKILLS;
}


/*                                                                          *
 * function: reload_mob_resets                                              *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - fixed instance where mysql_res was not freed          *
 *                                                                          */
void
reload_mob_resets (void)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];

  mysql_safe_query ("SELECT * FROM mob_resets");
  if ((result = mysql_store_result (database)) != NULL)
    {
      while ((row = mysql_fetch_row (result)) != NULL)
	{
	  zone_table[atoi (row[0])].cmd[atoi (row[1])].enabled = 1;
	}
      mysql_free_result (result);
      result = NULL;
    }
  else
    {
      sprintf (buf, "Warning: reload_mob_resets(): %s",
	       mysql_error (database));
      system_log (buf, true);
    }
  return;
}

std::string
resolved_host (char *ip)
{
  std::string resolved_hostname;

  mysql_safe_query ("SELECT ip, hostname, timestamp "
		    "FROM resolved_hosts "
		    "WHERE ip = '%s'", ip);

  MYSQL_RES *result = 0;
  if ((result = mysql_store_result (database)) != NULL)
    {
      MYSQL_ROW row = 0;
      if ((row = mysql_fetch_row (result)) != NULL)
	{
	  int timestamp = int (strtol (row[2], 0, 10));
	  if ((time (0) - timestamp) >= (60 * 60 * 24 * 30))
	    {			// Update listing after it's a month old.
	      mysql_safe_query ("DELETE FROM resolved_hosts "
				"WHERE ip = '%s'", ip);
	    }
	  else
	    {
	      resolved_hostname = row[1];
	    }
	}
      mysql_free_result (result);
    }
  else
    {
      std::string mysql_error_message =  "Warning: resolved_host(): ";
      mysql_error_message += mysql_error (database);
      system_log (mysql_error_message.c_str (), true);
    }

  return resolved_hostname;
}

/*                                                                          *
 * function: reload_sitebans                                                *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                                                                          */
void
reload_sitebans ()
{
  SITE_INFO *site, *tmp_site;
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];

  mysql_safe_query ("SELECT * FROM banned_sites");
  if ((result = mysql_store_result (database)) == NULL)
    {
      sprintf (buf, "Warning: reload_sitebans(): %s", mysql_error (database));
      system_log (buf, true);
      return;
    }

  while ((row = mysql_fetch_row (result)))
    {
      CREATE (site, SITE_INFO, 1);
      site->name = str_dup (row[0]);
      site->banned_by = str_dup (row[1]);
      site->banned_on = atoi (row[2]);
      site->banned_until = atoi (row[3]);
	site->next = NULL; //bug killer right here!
      if (!banned_site)
	banned_site = site;
      else
	for (tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next)
	  {
	    if (!tmp_site->next)
	      {
		tmp_site->next = site;
		break;
	      }
	  }
    }

  mysql_free_result (result);
  result = NULL;
}

void
save_banned_sites ()
{
  SITE_INFO *site = NULL;
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  if (!engine.in_play_mode ())
    return;

  mysql_safe_query ("DELETE FROM banned_sites");

  for (site = banned_site; site; site = site->next)
    {
      mysql_safe_query
	("INSERT INTO banned_sites VALUES ('%s', '%s', %d, %d)", site->name,
	 site->banned_by, site->banned_on, site->banned_until);
    }

  if (!(fp = fopen ("online_stats", "w+")))
    {
      system_log (buf, true);
      return;
    }

  fprintf (fp, "%d\n%s~\n", count_max_online, max_online_date);
  fclose (fp);
}

OBJ_DATA *
read_saved_obj (MYSQL_ROW row)
{
  OBJ_DATA *obj;
  WOUND_DATA *wound;
  LODGED_OBJECT_INFO *lodged;
  AFFECTED_TYPE *af;
  char buf[MAX_STRING_LENGTH];
  char location[MAX_STRING_LENGTH], type[MAX_STRING_LENGTH];
  char severity[MAX_STRING_LENGTH], name[MAX_STRING_LENGTH];
  char *p;
  int damage = 0, bleeding = 0, poison = 0, vnum = 0;;
  int healerskill = 0, lasthealed = 0, lastbled = 0;

  obj = load_object (atoi (row[4]));
  if (!obj)
    return NULL;

  if (row[5] && strlen (row[5]) > 3 && str_cmp (row[5], "(null)"))
    {
      mem_free (obj->name);
      obj->name = str_dup (row[5]);
    }
  if (row[6] && strlen (row[6]) > 3 && str_cmp (row[6], "(null)"))
    {
      mem_free (obj->short_description);
      obj->short_description = str_dup (row[6]);
    }
  if (row[7] && strlen (row[7]) > 3 && str_cmp (row[7], "(null)"))
    {
      mem_free (obj->description);
      obj->description = str_dup (row[7]);
    }
  if (row[8] && strlen (row[8]) > 3 && str_cmp (row[8], "(null)"))
    {
      mem_free (obj->full_description);
      obj->full_description = str_dup (row[8]);
    }
  if (row[9] && strlen (row[9]) > 3 && str_cmp (row[9], "(null)"))
    {
      mem_free (obj->desc_keys);
      obj->desc_keys = str_dup (row[9]);
    }
  if (row[10] && strlen (row[10]) > 3 && str_cmp (row[10], "(null)"))
    {
      mem_free (obj->book_title);
      obj->book_title = str_dup (row[10]);
    }

  obj->title_skill = atoi (row[11]);
  obj->title_language = atoi (row[12]);
  obj->title_script = atoi (row[13]);
  obj->loaded = load_object (atoi (row[14]));
  obj->obj_flags.extra_flags = atoi (row[15]);
  obj->o.od.value[0] = atoi (row[16]);
  obj->o.od.value[2] = atoi (row[17]);
  obj->o.od.value[3] = atoi (row[18]);
  obj->o.od.value[4] = atoi (row[19]);
  obj->o.od.value[5] = atoi (row[20]);
  obj->o.od.value[6] = atoi (row[21]);
  obj->obj_flags.weight = atoi (row[22]);
  obj->size = atoi (row[23]);
  obj->count = atoi (row[24]);
  obj->obj_timer = atoi (row[25]);
  obj->clock = atoi (row[26]);
  obj->morphto = atoi (row[27]);
  obj->morphTime = atoi (row[28]);

  if (row[29] && strlen (row[29]) > 3 && str_cmp (row[29], "(null)"))
    {
      mem_free (obj->var_color);
      obj->var_color = str_dup (row[29]);
    }
  if (row[30] && strlen (row[30]) > 3 && str_cmp (row[30], "(null)"))
    {
      mem_free (obj->omote_str);
      obj->omote_str = str_dup (row[30]);
    }

  if (row[31] && strlen (row[31]) > 3 && str_cmp (row[31], "(null)"))
    {
      p = str_dup (row[31]);
      while (1)
	{
	  if (!*p)
	    break;
	  get_line (&p, buf);
	  if (!*buf || !str_cmp (buf, "(null)"))
	    break;
	  CREATE (wound, WOUND_DATA, 1);
	  sscanf (buf, "%s %s %s %s %d %d %d %d %d %d\n",
		  location, type, severity, name, &damage, &bleeding, &poison,
		  &healerskill, &lasthealed, &lastbled);
	  wound->location = str_dup (location);
	  wound->type = str_dup (type);
	  wound->severity = str_dup (severity);
	  wound->name = str_dup (name);
	  wound->damage = damage;
	  wound->bleeding = bleeding;
	  wound->poison = poison;
	  wound->healerskill = healerskill;
	  wound->lasthealed = lasthealed;
	  wound->lastbled = lastbled;
	  if (obj->wounds)
	    wound->next = obj->wounds;
	  else
	    wound->next = NULL;
	  obj->wounds = wound;
	}
      if (p)
	{
	  mem_free (p);
	  p = NULL;
	}
    }

  if (row[32] && strlen (row[32]) > 3 && str_cmp (row[32], "(null)"))
    {
      p = str_dup (row[32]);
      while (1)
	{
	  if (!*p)
	    break;
	  get_line (&p, buf);
	  if (!*buf || !str_cmp (buf, "(null)"))
	    break;
	  CREATE (lodged, LODGED_OBJECT_INFO, 1);
	  sscanf (buf, "%s %d\n", location, &vnum);
	  lodged->location = str_dup (location);
	  lodged->vnum = vnum;
	  if (obj->lodged)
	    lodged->next = obj->lodged;
	  else
	    lodged->next = NULL;
	  obj->lodged = lodged;
	}
    }

  if (row[33] && strlen (row[33]) > 3 && str_cmp (row[33], "(null)"))
    {
      p = str_dup (row[33]);
      while (1)
	{
	  if (!*p)
	    break;
	  get_line (&p, buf);
	  if (!*buf || !str_cmp (buf, "(null)"))
	    break;
	  CREATE (af, AFFECTED_TYPE, 1);
	  sscanf (buf, "%d %d %d %d %d %d %d\n",
		  &af->a.spell.location, &af->a.spell.modifier,
		  &af->a.spell.duration, &af->a.spell.bitvector,
		  &af->a.spell.t, &af->a.spell.sn, &af->type);
	  if (obj->xaffected)
	    af->next = obj->xaffected;
	  else
	    af->next = NULL;
	  obj->xaffected = af;
	}
    }

  obj->next_content = NULL;

  return obj;
}


/*                                                                          *
 * function: load_mysql_save_rooms                                          *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - simplified the logic flow by nesting                  *
 *                                                                          */
void
load_mysql_save_rooms ()
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  int i = 0;
  char buf[MAX_STRING_LENGTH];

  mysql_safe_query ("SELECT * FROM saveroom_objs");

  if ((result = mysql_store_result (database)) == NULL)
    {
      sprintf (buf, "Warning: load_mysql_save_rooms(): %s",
	       mysql_error (database));
      system_log (buf, true);
      return;
    }

  while ((row = mysql_fetch_row (result)))
    {
      sprintf (buf, "Object #%d: %s [%s]", i, row[5], row[0]);
      i++;
      if (atoi (row[0]) == -1)
	continue;
      obj_to_room (read_saved_obj (row), atoi (row[0]));
      system_log (buf, false);
    }

  mysql_free_result (result);
}

/*                                                                          *
 * function: is_newbie                                                      *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - simplified the logic flow by nesting                  *
 *                                                                          */
int
is_newbie (const CHAR_DATA* ch)
{
  MYSQL_RES *result;
  char buf[MAX_STRING_LENGTH];
  int nPlayerFiles = 0;

  if (IS_NPC (ch))
    return 0;
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query
    ("SELECT name FROM %s.pfiles WHERE account = '%s' AND create_state > 1",
     player_db.c_str (), ch->pc->account_name);

  if ((result = mysql_store_result (database)) != NULL)
    {
      nPlayerFiles = mysql_num_rows (result);
      mysql_free_result (result);
      result = NULL;
      return ((nPlayerFiles > 0) ? 0 : 1);
    }
  else
    {
      sprintf (buf, "Warning: is_newbie(): %s", mysql_error (database));
      system_log (buf, true);
      return 0;			/* assume not a newbie ? */
    }
}

bool
is_newbie (const char* account_name)
{
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query ("SELECT COUNT(*) "
		    "FROM %s.pfiles "
		    "WHERE account = '%s' "
		    "AND create_state > 1",
		    player_db.c_str (), account_name);

  MYSQL_RES *result;
  bool is_brand_newbian = false;
  if ((result = mysql_store_result (database)) != NULL)
    {
      MYSQL_ROW row;
      if ((row = mysql_fetch_row (result)))
	{
	  if (strtol (row[0],0,10) == 0)
	    is_brand_newbian = true;
	}
      mysql_free_result (result);
    }
  else
    {
      std::ostringstream error_message;
      error_message << "Error: " << std::endl
		    << __FILE__ << ':'
		    << __func__ << '('
		    << __LINE__ << "):" << std::endl
		    << mysql_error (database);
      const char *error_cstr = (error_message.str ()).c_str();
      send_to_gods (error_cstr);
      system_log (error_cstr, true);
    }

  return is_brand_newbian;
}

int
is_yours (const char *name, const char *account_name)
{
  MYSQL_RES *result;
  int isYours = 0;

  if (!name || !*name || !account_name || !*account_name)
    return 0;

  mysql_safe_query
    ("SELECT * FROM reviews_in_progress WHERE char_name = '%s' AND reviewer = '%s' AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20",
     name, account_name);
  if ((result = mysql_store_result (database)) != NULL)
    {

      if (mysql_num_rows (result) > 0)
	{
	  isYours = 1;
	}
      mysql_free_result (result);
    }
  else if (result != NULL)
    mysql_free_result (result);

  return isYours;
}

int
is_being_reviewed (const char *name, const char *account_name)
{
  MYSQL_RES *result;
  int isBeingReviewed = 0;

  if (!name || !*name || !account_name || !*account_name)
    return 0;

  mysql_safe_query
    ("SELECT *"
     " FROM reviews_in_progress"
     " WHERE char_name = '%s'"
     " AND reviewer != '%s'"
     " AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20",
     name, account_name);
  if ((result = mysql_store_result (database)) != NULL)
    {

      if (mysql_num_rows (result) > 0)
	{
	  isBeingReviewed = 1;
	}
      mysql_free_result (result);
    }
  else if (result != NULL)
    mysql_free_result (result);

  return isBeingReviewed;
}

int
is_admin (const char *username)
{
  MYSQL_RES *result;

  if (!username || !*username)
    return 0;

  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query
    ("SELECT name"
     " FROM %s.pfiles"
     " WHERE level > 0"
     " AND account = '%s'",
     player_db.c_str (), username);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      return 0;
    }

  if (result)
    mysql_free_result (result);

  return 1;

}

/*                                                                          *
 * function: do_history                                                     *
 *                                                                          *
 * 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
 *                  - simplified the logic flow by nesting                  *
 *                                                                          */
void
do_history (CHAR_DATA * ch, char *argument, int cmd)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  char buf[MAX_STRING_LENGTH];
  char name[255];
  int nResponses = 0;

  if (!ch->delay || ch->delay_type != DEL_APP_APPROVE)
    {
      send_to_char ("You are not currently reviewing an application.\n", ch);
      return;
    }

  *name = '\0';

  strcpy (name, ch->delay_who);
  mem_free (ch->delay_who);
  ch->delay_who = NULL;
  ch->delay = 0;

  if (!*name)
    {
      send_to_char ("You are not currently reviewing an application.\n", ch);
      return;
    }

  mysql_safe_query
    ("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s' ORDER BY timestamp ASC",
     name);
  if ((result = mysql_store_result (database)) != NULL)
    {
      nResponses = mysql_num_rows (result);
      *b_buf = '\0';
      while ((row = mysql_fetch_row (result)) != NULL)
	{
	  sprintf (b_buf + strlen (b_buf),
		   "\n#6Reviewed By:#0 %s\n"
		   "#6Reviewed On:#0 %s\n\n"
		   "%s\n--", row[3], row[4], row[5]);
	}
      mysql_free_result (result);
      result = NULL;

      if (nResponses)
	{
	  strcat (b_buf, "\n");
	  page_string (ch->desc, b_buf);
	}
      else
	{
	  send_to_char
	    ("This application has not yet had any administrator responses.\n",
	     ch);
	}
    }
  else
    {
      sprintf (buf, "Warning: do_history(): %s", mysql_error (database));
      system_log (buf, true);
    }
  return;
}

void
do_writings (CHAR_DATA * ch, char *argument, int cmd)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i = 0;

  if (!*argument)
    {
      send_to_char ("How do you wish to access the writings database?\n", ch);
      return;
    }

  argument = one_argument (argument, buf);

  if (!str_cmp (buf, "by"))
    {
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char
	    ("For whom do you wish to search in the writings database?\n",
	     ch);
	  return;
	}
      argument = one_argument (argument, buf2);
      if (!str_cmp (buf2, "containing"))
	{
	  argument = one_argument (argument, buf2);
	  if (!*buf2)
	    {
	      send_to_char
		("Which strings did you wish to search for in these writings?\n",
		 ch);
	      return;
	    }
	  mysql_safe_query
	    ("SELECT db_key FROM player_writing WHERE author = '%s' AND writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC",
	     buf, buf2);
	}
      else
	mysql_safe_query
	  ("SELECT db_key FROM player_writing WHERE author = '%s' GROUP BY db_key ORDER BY db_key ASC",
	   buf);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  send_to_char
	    ("No writings were found in the database for that PC.\n", ch);
	  return;
	}

      *b_buf = '\0';
      sprintf (b_buf,
	       "\nThe database contains the following keys to writings authored by #6%s#0:\n\n",
	       CAP (buf));
      i = 0;

      while ((row = mysql_fetch_row (result)))
	{
	  sprintf (b_buf + strlen (b_buf), "   %-15d", atoi (row[0]));
	  i++;
	  if (!(i % 4))
	    strcat (b_buf, "\n");
	}

      if ((i % 4))
	strcat (b_buf, "\n");

      page_string (ch->desc, b_buf);

      mysql_free_result (result);
      result = NULL;
    }
  else if (!str_cmp (buf, "containing"))
    {
      argument = one_argument (argument, buf2);
      if (!*buf2)
	{
	  send_to_char
	    ("Which strings did you wish to search for in these writings?\n",
	     ch);
	  return;
	}
      mysql_safe_query
	("SELECT db_key FROM player_writing WHERE writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC",
	 buf2);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  send_to_char
	    ("No writings were found in the database matching that string.\n",
	     ch);
	  return;
	}

      *b_buf = '\0';
      sprintf (b_buf,
	       "\nThe database contains the following keys to writings matching that string:\n\n");
      i = 0;

      while ((row = mysql_fetch_row (result)))
	{
	  sprintf (b_buf + strlen (b_buf), "   %-15d", atoi (row[0]));
	  i++;
	  if (!(i % 4))
	    strcat (b_buf, "\n");
	}

      if ((i % 4))
	strcat (b_buf, "\n");

      page_string (ch->desc, b_buf);

      mysql_free_result (result);
      result = NULL;
    }
  else if (!str_cmp (buf, "display"))
    {
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char
	    ("Which database key did you wish to view the writing for?\n",
	     ch);
	  return;
	}
      if (!isdigit (*buf))
	{
	  send_to_char
	    ("You'll need to specify a numeric database key to view.\n", ch);
	  return;
	}

      mysql_safe_query
	("SELECT * FROM player_writing WHERE db_key = %d ORDER BY page ASC",
	 atoi (buf));
      result = mysql_store_result (database);

      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  send_to_char ("The database contains no writing under that key.\n",
			ch);
	  return;
	}

      *b_buf = '\0';

      while ((row = mysql_fetch_row (result)))
	{
	  sprintf (b_buf + strlen (b_buf), "\n#6Author:#0       %s\n"
		   "#6Written On:#0   %s\n"
		   "#6Written In:#0   %s and %s\n"
		   "#6Page Number:#0  %d\n"
		   "\n"
		   "%s", row[1], row[3], skills[atoi (row[5])],
		   skills[atoi (row[6])], atoi (row[2]), row[8]);
	}

      page_string (ch->desc, b_buf);

      mysql_free_result (result);
      result = NULL;
    }
  else if (!str_cmp (buf, "key"))
    {
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char ("Which object did you wish to key a writing to?\n",
			ch);
	  return;
	}
      if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)))
	{
	  send_to_char ("I don't see that object.\n", ch);
	  return;
	}
      if (GET_ITEM_TYPE (obj) != ITEM_BOOK
	  && GET_ITEM_TYPE (obj) != ITEM_PARCHMENT)
	{
	  send_to_char
	    ("You may only key writings to books or parchment objects.\n",
	     ch);
	  return;
	}
      argument = one_argument (argument, buf);
      if (!*buf)
	{
	  send_to_char
	    ("Which writing entry did you wish to clone to this object?\n",
	     ch);
	  return;
	}
      if (!isdigit (*buf))
	{
	  send_to_char
	    ("You'll need to specify a numeric database key to clone.\n", ch);
	  return;
	}
      mysql_safe_query
	("SELECT COUNT(*) FROM player_writing WHERE db_key = %d", atoi (buf));
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  send_to_char
	    ("The database contains no keys matching that number.\n", ch);
	  return;
	}
      row = mysql_fetch_row (result);
      if ((GET_ITEM_TYPE (obj) == ITEM_PARCHMENT && atoi (row[0]) > 1) ||
	  (GET_ITEM_TYPE (obj) == ITEM_BOOK
	   && atoi (row[0]) > obj->o.od.value[0]))
	{
	  send_to_char
	    ("That writing key contains too many pages for this object to hold.\n",
	     ch);
	  return;
	}
      if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
	obj->o.od.value[0] = atoi (buf);
      else
	obj->o.od.value[1] = atoi (buf);
      load_writing (obj);
      if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
	obj->o.od.value[0] = unused_writing_id ();
      else
	obj->o.od.value[1] = unused_writing_id ();
      save_writing (obj);
      send_to_char
	("Writing has been successfully cloned and keyed to the object.\n",
	 ch);
      mysql_free_result (result);
      result = NULL;
      return;
    }
  else
    send_to_char ("That isn't a recognized option for this command.\n", ch);
}

void
load_all_writing (void)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  WRITING_DATA *writing;
  OBJ_DATA *obj;
  bool loaded = false;
  int i;

  mysql_safe_query ("SELECT * FROM player_writing");

  result = mysql_store_result (database);
  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      return;
    }

  while ((row = mysql_fetch_row (result)))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (obj->deleted)
	    continue;

	  if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT
	      && obj->o.od.value[0] == atoi (row[0]))
	    {
	      if (!obj->writing)
		CREATE (obj->writing, WRITING_DATA, 1);
	      obj->writing->author = add_hash (row[1]);
	      obj->writing->date = add_hash (row[3]);
	      obj->writing->ink = add_hash (row[4]);
	      obj->writing->language = atoi (row[5]);
	      obj->writing->script = atoi (row[6]);
	      obj->writing->skill = atoi (row[7]);
	      obj->writing->message = add_hash (row[8]);
	      obj->writing_loaded = true;
	      break;
	    }
	  else if (GET_ITEM_TYPE (obj) == ITEM_BOOK
		   && obj->o.od.value[1] == atoi (row[0]))
	    {
	      if (!obj->writing)
		CREATE (obj->writing, WRITING_DATA, 1);
	      writing = obj->writing;
	      for (i = 1; i <= obj->o.od.value[0]; i++)
		{
		  if (!writing->next_page && i + 1 <= obj->o.od.value[0])
		    {
		      CREATE (writing->next_page, WRITING_DATA, 1);
		    }
		  if (i == atoi (row[2]))
		    {
		      writing->author = add_hash (row[1]);
		      writing->date = add_hash (row[3]);
		      writing->ink = add_hash (row[4]);
		      writing->language = atoi (row[5]);
		      writing->script = atoi (row[6]);
		      writing->skill = atoi (row[7]);
		      writing->message = add_hash (row[8]);
		      loaded = true;
		      break;
		    }
		  writing = writing->next_page;
		}
	      if (loaded)
		{
		  obj->writing_loaded = true;
		  loaded = false;
		  break;
		}
	    }
	}
    }

  if (result)
    mysql_free_result (result);
  result = NULL;
}

void
load_writing (OBJ_DATA * obj)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  WRITING_DATA *writing;
  int id, i;

  if (obj->writing_loaded)
   {
    return;
	}

  if (GET_ITEM_TYPE (obj) != ITEM_BOOK && GET_ITEM_TYPE (obj) != ITEM_PARCHMENT)
   {
    return;
	}

  obj->writing_loaded = true;

  if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
   {
    id = (obj)->o.od.value[1];
	}
  else
   {
    id = (obj)->o.od.value[0];
	}

  mysql_safe_query
    ("SELECT * FROM player_writing WHERE db_key = %d ORDER BY page ASC",
     id);

  result = mysql_store_result (database);
  if (!result || !mysql_num_rows (result))
    {
      if (result)
		{
			mysql_free_result (result);
	   }
      return;
    }

  if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
    {
      row = mysql_fetch_row (result);
      if (!row)
		{
			return;
		}
      if (!obj->writing)
		{
			CREATE (obj->writing, WRITING_DATA, 1);
				writing = obj->writing;
				writing->author = add_hash (row[1]);
				writing->date = add_hash (row[3]);
				writing->ink = add_hash (row[4]);
				writing->language = atoi (row[5]);
				writing->script = atoi (row[6]);
				writing->skill = atoi (row[7]);
				writing->message = add_hash (row[8]);
		}
      if (result)
		{
			mysql_free_result (result);
		}
      return;
    }
  else if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
    {
	      if (!obj->writing)
			 {
            CREATE (obj->writing, WRITING_DATA, 1);
			 }
		  /* iterate among all known pages, listed in ascending order */
          writing = obj->writing;
          row = mysql_fetch_row(result);
          /* iterate pages */
          for (i = 1; i <= obj->o.od.value[0]; i++)
            {
                        /* if the next page should exist (for the last iteration next_page
                           should be left null */
              if (!writing->next_page && ((i + 1) <= obj->o.od.value[0]))
                  {
                          CREATE (writing->next_page, WRITING_DATA, 1);
                  }

                  /* if the data specify info on this page, add in information */
              if (row && (i == atoi(row[2])))
                  {
                        //std::ostringstream oss;
                        //oss << "Loading written page on book " << id << " for page " << i << "\n";
                        //  send_to_gods (oss.str().c_str());

                  writing->author = add_hash (row[1]);
				  writing->date = add_hash (row[3]);
                  writing->ink = add_hash (row[4]);
                  writing->language = atoi (row[5]);
                  writing->script = atoi (row[6]);
                  writing->skill = atoi (row[7]);
                  writing->message = add_hash (row[8]);
                  // load the next row to be considered on next loop
                  row = mysql_fetch_row(result);
                  }
                  /* if the data do not exist, mark pages blank as in load_object_full */
                  else
                  {
                        //  std::ostringstream oss;
                        //  oss << "Adding blank page on book " << id << " for page " << i << "\n";
                        //  send_to_gods (oss.str().c_str());

                        writing->message = add_hash ("blank");
                        writing->author = add_hash ("blank");
                        writing->date = add_hash ("blank");
                        writing->ink = add_hash ("blank");
                        writing->language = 0;
						writing->script = 0;
                        writing->skill = 0;
                        writing->torn = false;
                  }
              writing = writing->next_page;
          } // for loop iterating pages
  } // if book

  if (result)
   {
    mysql_free_result (result);
	}
  result = NULL;
}




void
save_writing (OBJ_DATA * obj)
{
  WRITING_DATA *writing;
  int i = 1;

  if (!obj->writing)
   {
    return;
	}

  if (GET_ITEM_TYPE (obj) != ITEM_PARCHMENT && GET_ITEM_TYPE (obj) != ITEM_BOOK)
	{
    return;
	}

  if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
    {
      writing = obj->writing;
      if (!writing || !writing->date || !str_cmp (writing->date, "blank")
			|| !writing->language || !writing->script)
	   {
			return;
		}
			
      if (!obj->o.od.value[0])
		{
			obj->o.od.value[0] = unused_writing_id ();
		}
		
      mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d",
			obj->o.od.value[0]);
      mysql_safe_query
	("INSERT INTO player_writing VALUES (%d, '%s', 1, '%s', '%s', %d, "
	 "%d, %d, '%s', %d)", obj->o.od.value[0], writing->author,
	 writing->date, writing->ink, writing->language, writing->script,
	 writing->skill, writing->message, (int) time (0));
    }

  if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
    {
      if (!obj->o.od.value[1])
		{
			obj->o.od.value[1] = unused_writing_id ();
		}
		
      mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d", obj->o.od.value[1]);
      
		for (writing = obj->writing; writing != NULL && i <= obj->o.od.value[0];
	   writing = writing->next_page, i++)
	{
	  if (!writing )
		{
	    continue;
		}
	  mysql_safe_query
	    ("INSERT INTO player_writing VALUES (%d, '%s', %d, '%s', '%s', %d, "
	     "%d, %d, '%s', %d)", obj->o.od.value[1], writing->author, i,
	     writing->date, writing->ink, writing->language, writing->script,
	     writing->skill, writing->message, (int) time (0));
	  if (!writing->next_page)
		{
	    break;
		}
	}
    }

}

int
unused_writing_id (void)
{
  MYSQL_RES *result;
  bool again = true;
  int id;

  do
    {
      id = rand ();
      mysql_safe_query ("SELECT db_key FROM player_writing WHERE db_key = %d",
			id);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  return id;
	}
      mysql_free_result (result);
      result = NULL;
    }
  while (again);

  return id;
}

void
save_dreams (CHAR_DATA * ch)
{
  DREAM_DATA *dream;

  if (!ch || !ch->tname)
    return;
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query ("DELETE FROM %s.dreams WHERE name = '%s'",
		    player_db.c_str (), ch->tname);

  if (ch->pc->dreams)
    {
      for (dream = ch->pc->dreams; dream; dream = dream->next)
	{
	  mysql_safe_query ("INSERT INTO %s.dreams VALUES('%s', %d, '%s')",
			    player_db.c_str (), ch->tname, 0, dream->dream);
	}
    }

  if (ch->pc->dreamed)
    {
      for (dream = ch->pc->dreamed; dream; dream = dream->next)
	{
	  mysql_safe_query ("INSERT INTO %s.dreams VALUES('%s', %d, '%s')",
			    player_db.c_str (), ch->tname, 1, dream->dream);
	}
    }
}

void
load_dreams (CHAR_DATA * ch)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  DREAM_DATA *dream, *dream_list = NULL;

  if (!ch || !ch->tname)
    return;
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query ("SELECT * FROM %s.dreams WHERE name = '%s'",
		    player_db.c_str (), ch->tname);
  result = mysql_store_result (database);
  if (result)
    {
      while ((row = mysql_fetch_row (result)))
	{
	  CREATE (dream, DREAM_DATA, 1);
	  dream->dream = str_dup (row[2]);
	  dream->next = NULL;
	  
	  if (atoi (row[1]) > 0)
	    {			// Already dreamed.
	      if (ch->pc->dreamed)
		dream_list = ch->pc->dreamed;
	      else
		ch->pc->dreamed = dream;
	    }
	  else
	    {
	      if (ch->pc->dreams)
		dream_list = ch->pc->dreams;
	      else
		ch->pc->dreams = dream;
	    }
	  
	  if (dream_list)
	    {
	      while (dream_list->next != NULL)
		dream_list = dream_list->next;
	      dream_list->next = dream;
	    }
	  
	  dream_list = NULL;
	}

      mysql_free_result (result);
    }
  else
    {
      fprintf (stderr, "load_dreams: The function call 'mysql_safe_query' "
	       "failed to return a valid result\nfor the following reason: "
	       "\n%s\n", mysql_error (database));
      exit (1);
    }
}

void
perform_pfile_update (CHAR_DATA * ch)
{
  int i = 0;

  if (!ch->pc)
    return;

  if (!ch->skills[SKILL_SWIMMING])
    {
      open_skill (ch, SKILL_SWIMMING);
      starting_skill_boost (ch, SKILL_SWIMMING);
    }

  if (ch->skills[SKILL_SUBDUE])
    ch->skills[SKILL_SUBDUE] = 0;

  if (!ch->pc->level)
    {
      for (i = 1; i <= LAST_SKILL; i++)
	{
	  if (ch->skills[i] > calc_lookup (ch, REG_CAP, i))
	    ch->skills[i] = calc_lookup (ch, REG_CAP, i);
	}

      update_crafts (ch);
    }

  if ((ch->race >= 16 && ch->race <= 19) || ch->race == 93)
    {				// Elves have superb distance vision and hearing.
      ch->skills[SKILL_SCAN] = 120;
      ch->skills[SKILL_LISTEN] = 120;
    }

  for (i = 1; i <= LAST_SKILL; i++)
    ch->pc->skills[i] = ch->skills[i];

  ch->tmp_str = ch->str;
  ch->tmp_dex = ch->dex;
  ch->tmp_agi = ch->agi;
  ch->tmp_con = ch->con;
  ch->tmp_aur = ch->aur;
  ch->tmp_intel = ch->intel;
}

void
process_queued_review (MYSQL_ROW row)
{
  CHAR_DATA *tch, *tmp_ch;
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char email[255], subject[255];
  long int time_elapsed = 0;
  bool accepted = false;

  if (!row)
    return;

  if (!(tch = load_pc (row[0])))
    {
      return;
    }

  //for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	tmp_ch = *tch_iterator;
      if (tmp_ch->deleted)
	continue;
      if (tmp_ch->pc && tmp_ch->pc->edit_player
	  && !str_cmp (tmp_ch->pc->edit_player->tname, tch->tname))
	{
	  send_to_char
	    ("The PC in your editor has been closed for an application response.\n",
	     tmp_ch);
	  tmp_ch->pc->edit_player = NULL;
	}
    }

  while (tch->pc->load_count > 1)
    unload_pc (tch);

  if (tch->pc->create_state != 1)
    {
      mysql_safe_query ("DELETE FROM queued_reviews WHERE char_name = '%s'",
			row[0]);
      unload_pc (tch);
      return;
    }

  accepted = atoi (row[7]);
  *buf = '\0';
  *subject = '\0';

  // Acceptance; process the new PC for entry into the game

  if (accepted)
    {
      setup_new_character (tch);

      mysql_safe_query
	("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");

      mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d",
			tch->pc->profession);

      sprintf (buf, "Greetings,\n"
	       "\n"
	       "   Thank you for your interest in %s! This is an automated\n"
	       "system notification sent to inform you that your application for a character\n"
	       "named %s has been ACCEPTED by the reviewer, and you that may\n"
	       "enter the game at your earliest convenience. We'll see you there!\n"
	       "\n"
	       "%s left the following comments regarding your application:\n"
	       "\n%s", MUD_NAME, tch->tname, row[1], row[8]);
      sprintf (subject, "#2Accepted:#0 %s: %s", tch->pc->account_name, tch->tname);
      sprintf (buf, "%s\n", row[8]);
      add_message (1, tch->tname, -2, row[1], NULL, "Application Acceptance",
		   "", buf, 0);
    }
  else
    {
      tch->pc->create_state = STATE_REJECTED;

      mysql_safe_query
	("UPDATE newsletter_stats SET declined_apps=declined_apps+1");

      sprintf (buf,
	       "\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
	       "\n%s", row[1], row[8]);
      if (buf[strlen (buf) - 1] != '\n')
	strcat (buf, "\n");
      tch->pc->msg = add_hash (buf);

      sprintf (buf, "Greetings,\n"
	       "\n"
	       "   Thank you for your interest in %s! This is an\n"
	       "automated system notification to inform you that your application for\n"
	       "a character named %s was deemed inappropriate by the reviewer, and\n"
	       "therefore was declined. However, don't despair! This is a relatively\n"
	       "common occurrence, and nothing to worry about. Your application has\n"
	       "been saved on our server, and you may make the necessary changes simply\n"
	       "by entering the game as that character. You will be dropped back\n"
	       "into the character generation engine, where you may make corrections.\n"
	       "\n"
	       "%s left the following comments regarding your application:\n"
	       "\n%s", MUD_NAME, tch->tname, row[1], row[8]);
      sprintf (subject, "#1Declined:#0 %s: %s", tch->pc->account_name, tch->tname);
    }

  account *acct = new account (tch->pc->account_name);
  if (acct->is_registered () && *buf)
    {
      sprintf (email, "%s <%s>", MUD_NAME, MUD_EMAIL);
      send_email (acct, NULL, email, "Re: Your Character Application",
		  buf);
      if (accepted && tch->pc->special_role)
	{
	  sprintf (email, "%s Player <%s>", MUD_NAME, acct->email.c_str ());
	  delete acct;
	  acct = new account (tch->pc->special_role->poster);
	  if (acct->is_registered ())
	    {
	      sprintf (buf, "Hello,\n\n"
		       "This email is being sent to inform you that a special role you placed\n"
		       "in chargen has been applied for and accepted. The details are attached.\n"
		       "\n"
		       "Please contact this player at your earliest convenience to arrange a time\n"
		       "to set up and integrate their new character. To do so, simply click REPLY;\n"
		       "their email has been listed in this message as the FROM address.\n\n"
		       "Character Name: %s\n"
		       "Role Description: %s\n"
		       "Role Post Date: %s\n"
		       "\n"
		       "%s\n\n", tch->tname, tch->pc->special_role->summary,
		       tch->pc->special_role->date,
		       tch->pc->special_role->body);
	      sprintf (buf2, "New PC: %s", tch->pc->special_role->summary);
	      send_email (acct, APP_EMAIL, email, buf2, buf);
	    }
	}
    }

  if (*subject)
    {
      sprintf (buf, "%s\n", row[8]);
      add_message (1, "Applications", -5, row[1], NULL, subject, "", buf, 0);
    }

  time_elapsed = time (0) - tch->time.birth;
  mysql_safe_query ("INSERT INTO application_wait_times (wait_time) "
                    "VALUES (%d)",
		    (int) time_elapsed);

  mysql_safe_query ("DELETE FROM queued_reviews WHERE char_name = '%s'",
		    tch->tname);

  if (tch->name && *tch->name && str_cmp (row[2], tch->name))
    {
      mem_free (tch->name);
      tch->name = str_dup (row[2]);
    }

  if (tch->short_descr && *tch->short_descr
      && str_cmp (row[3], tch->short_descr))
    {
      mem_free (tch->short_descr);
      tch->short_descr = str_dup (row[3]);
    }

  if (tch->long_descr && *tch->long_descr
      && str_cmp (row[4], tch->long_descr))
    {
      mem_free (tch->long_descr);
      tch->long_descr = str_dup (row[4]);
    }

  if (tch->description && *tch->description
      && str_cmp (row[5], tch->description))
    {
      mem_free (tch->description);
      tch->description = str_dup (row[5]);
      reformat_string (tch->description, &tch->description);
    }

  if (tch->pc && tch->pc->creation_comment && *tch->pc->creation_comment
      && str_cmp (row[6], tch->pc->creation_comment))
    {
      mem_free (tch->pc->creation_comment);
      tch->pc->creation_comment = str_dup (row[6]);
    }
  delete acct;
  unload_pc (tch);
}

void
process_reviews (void)
{
  MYSQL_RES *result = NULL;
  MYSQL_ROW row;

  if (!engine.in_play_mode ())
    return;

  mysql_safe_query ("SELECT * FROM queued_reviews");
  if ((result = mysql_store_result (database)) != NULL)
    {

      while ((row = mysql_fetch_row (result)))
	{
	  process_queued_review (row);
	}

      mysql_free_result (result);
    }

}

// Return number of possible starting locations for PC of given race

int
num_starting_locs (int race)
{
  int flags = 0, start_loc = 0;

  if (lookup_race_variable (race, RACE_START_LOC))
    {
      flags = atoi (lookup_race_variable (race, RACE_START_LOC));
      if (IS_SET (flags, RACE_HOME_GONDOR))
	start_loc++;
      if (IS_SET (flags, RACE_HOME_ANGOST))
	start_loc++;
	  if (IS_SET (flags, RACE_HOME_HARAD))
	    start_loc++;
	  if (IS_SET (flags, RACE_HOME_MORIA))
		  start_loc++;
      return start_loc;
    }

  return 1;
}

// Update an N/PC's racial affects/abilities

void
apply_race_affects (CHAR_DATA * tch)
{
  int affects = 0;

  if (lookup_race_variable (tch->race, RACE_AFFECTS))
    {
      affects = atoi (lookup_race_variable (tch->race, RACE_AFFECTS));
      if (IS_SET (affects, INNATE_INFRA))
	tch->affected_by |= AFF_INFRAVIS;
      else
	tch->affected_by &= ~AFF_INFRAVIS;
      if (IS_SET (affects, INNATE_FLYING))
	tch->act |= ACT_FLYING;
      else
	tch->affected_by &= ~ACT_FLYING;
      if (IS_SET (affects, INNATE_WAT_BREATH))
	tch->affected_by |= AFF_BREATHE_WATER;
      else
	tch->affected_by &= ~AFF_BREATHE_WATER;
      if (IS_SET (affects, INNATE_NOBLEED))
	tch->act |= ACT_NOBLEED;
      else
	tch->act &= ~ACT_NOBLEED;
      if (IS_SET (affects, INNATE_SUN_PEN))
	tch->affected_by |= AFF_SUNLIGHT_PEN;
      else
	tch->affected_by &= ~AFF_SUNLIGHT_PEN;
      if (IS_SET (affects, INNATE_SUN_PET))
	tch->affected_by |= AFF_SUNLIGHT_PET;
      else
	tch->affected_by &= ~AFF_SUNLIGHT_PET;
    }
}

// To update PCs who haven't been loaded since their race config was changed

void
refresh_race_configuration (CHAR_DATA * tch)
{
  if (!tch->pc)
    return;

  tch->str = tch->pc->start_str;
  tch->con = tch->pc->start_con;
  tch->dex = tch->pc->start_dex;
  tch->intel = tch->pc->start_intel;
  tch->wil = tch->pc->start_wil;
  tch->aur = tch->pc->start_aur;
  tch->con = tch->pc->start_con;
  tch->agi = tch->pc->start_agi;

  apply_race_affects (tch);

  if (lookup_race_variable (tch->race, RACE_STR_MOD))
    tch->str += atoi (lookup_race_variable (tch->race, RACE_STR_MOD));
  if (lookup_race_variable (tch->race, RACE_CON_MOD))
    tch->con += atoi (lookup_race_variable (tch->race, RACE_CON_MOD));
  if (lookup_race_variable (tch->race, RACE_DEX_MOD))
    tch->dex += atoi (lookup_race_variable (tch->race, RACE_DEX_MOD));
  if (lookup_race_variable (tch->race, RACE_AGI_MOD))
    tch->agi += atoi (lookup_race_variable (tch->race, RACE_AGI_MOD));
  if (lookup_race_variable (tch->race, RACE_INT_MOD))
    tch->intel += atoi (lookup_race_variable (tch->race, RACE_INT_MOD));
  if (lookup_race_variable (tch->race, RACE_WIL_MOD))
    tch->wil += atoi (lookup_race_variable (tch->race, RACE_WIL_MOD));
  if (lookup_race_variable (tch->race, RACE_AUR_MOD))
    tch->aur += atoi (lookup_race_variable (tch->race, RACE_AUR_MOD));

  if ((tch->height = calculate_race_height (tch)) == -1)
    tch->height = calculate_size_height (tch);

//      if ( lookup
}

CHAR_DATA *
load_char_mysql (const char *name)
{
  int ind = 0, lev = 0, damage = 0, bleeding = 0, i = 0;
  int poison = 0, infection = 0, healerskill = 0, lasthealed = 0;
  int lastbled = 0, bindskill = 0, lastbound = 0;
  char wound_name[100], severity[100], type[100], location[100];
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  MYSQL_RES *result;
  MYSQL_ROW row;
  CHAR_DATA *ch = NULL;
  WOUND_DATA *wound, *tmpwound;
  LODGED_OBJECT_INFO *lodged, *tmplodged;
  SUBCRAFT_HEAD_DATA *craft;
  AFFECTED_TYPE *af;

  if (!name || !*name)
    return NULL;
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query ("SELECT * FROM %s.pfiles WHERE name = '%s'",
		    player_db.c_str (), name);
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result != NULL)
	mysql_free_result (result);
      return NULL;
    }

  row = mysql_fetch_row (result);

  if (!row)
    {
      return NULL;
    }

  ch = new_char (1);
  //clear_char (ch);

  ch->tname = str_dup (row[0]);
  ch->name = str_dup (row[1]);
  ch->pc->account_name = str_dup (row[2]);
  ch->short_descr = str_dup (row[3]);
  //correction for capital short_descr's;
  if (isupper(ch->short_descr[0]))
	  ch->short_descr[0] = tolower(ch->short_descr[0]);

  ch->long_descr = str_dup (row[4]);
  ch->description = str_dup (row[5]);
  ch->pc->msg = str_dup (row[6]);
  ch->pc->creation_comment = str_dup (row[7]);
  ch->pc->create_state = atoi (row[8]);
  ch->pc->nanny_state = atoi (row[9]);
  ch->pc->role = atoi (row[10]);

  if (strn_cmp (row[13], "~", 1) && strn_cmp (row[13], " ", 1))
    {
      if (!ch->pc->special_role)
	CREATE (ch->pc->special_role, ROLE_DATA, 1);
      ch->pc->special_role->summary = str_dup (row[11]);
      ch->pc->special_role->body = str_dup (row[12]);
      ch->pc->special_role->date = str_dup (row[13]);
      ch->pc->special_role->poster = str_dup (row[14]);
      ch->pc->special_role->cost = atoi (row[15]);
      ch->pc->special_role->id = atoi(row[96]);
    }

  ch->pc->app_cost = atoi (row[16]);
  ch->pc->level = atoi (row[17]);
  ch->sex = atoi (row[18]);
  ch->deity = atoi (row[19]);
  ch->race = atoi (row[20]);
  ch->in_room = atoi (row[21]);

  ch->str = atoi (row[22]);
  ch->intel = atoi (row[23]);
  ch->wil = atoi (row[24]);
  ch->con = atoi (row[25]);
  ch->dex = atoi (row[26]);
  ch->aur = atoi (row[27]);
  ch->agi = atoi (row[28]);
  ch->pc->start_str = atoi (row[29]);
  ch->pc->start_intel = atoi (row[30]);
  ch->pc->start_wil = atoi (row[31]);
  ch->pc->start_con = atoi (row[32]);
  ch->pc->start_dex = atoi (row[33]);
  ch->pc->start_aur = atoi (row[34]);
  ch->pc->start_agi = atoi (row[35]);
  ch->tmp_str = ch->str;
  ch->tmp_intel = ch->intel;
  ch->tmp_wil = ch->wil;
  ch->tmp_con = ch->con;
  ch->tmp_dex = ch->dex;
  ch->tmp_aur = ch->aur;
  ch->tmp_agi = ch->agi;

  ch->time.played = atoi (row[36]);
  ch->time.birth = atoi (row[37]);
  ch->time.logon = time (0);

  /*      Time? row[38]           */

  ch->offense = atoi (row[39]);
  ch->hit = atoi (row[40]);
  ch->max_hit = atoi (row[41]);
  ch->nat_attack_type = atoi (row[42]);
  ch->move = atoi (row[43]);
  ch->max_move = atoi (row[44]);
  ch->circle = atoi (row[45]);
  ch->ppoints = atoi (row[46]);
  ch->fight_mode = atoi (row[47]);
  ch->color = atoi (row[48]);
  ch->speaks = atoi (row[49]);
  ch->flags = atoi (row[50]);
  ch->plr_flags = atoi (row[51]);
  ch->pc->boat_virtual = atoi (row[52]);
  ch->speed = atoi (row[53]);
  ch->pc->mount_speed = atoi (row[54]);
  ch->pc->sleep_needed = atoi (row[55]);
  ch->pc->auto_toll = atoi (row[56]);
  ch->coldload_id = atoi (row[57]);
  ch->affected_by = atoi (row[58]);

  if (str_cmp (row[59], "~") && str_cmp (row[59], " "))
    {
      while (1)
	{
	  if (!*row[59])
	    break;
	  get_line (&row[59], buf);
	  if (!*buf || !str_cmp (buf, "~") || !str_cmp (buf, " "))
	    break;
	  sscanf (buf, "%s ", buf2);
	  if (!*buf2)
	    break;

	  if (!str_cmp (buf2, "Subcraft"))
	    {
	      sscanf (buf, "Subcraft '%s'", buf2);
	      buf2[strlen (buf2) - 1] = '\0';
	      for (craft = crafts;
		   craft && str_cmp (craft->subcraft_name, buf2);
		   craft = craft->next)
		;
	      if (!craft)
		continue;
	      for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		if (!get_affect (ch, i))
		  break;
	      if (i > CRAFT_LAST)
		continue;
	      magic_add_affect (ch, i, -1, 0, 0, 0, 0);
	      af = get_affect (ch, i);
	      af->a.craft =
		(struct affect_craft_type *)
		alloc (sizeof (struct affect_craft_type), 23);
	      af->a.craft->subcraft = craft;
	    }
	  else if (!str_cmp (buf2, "Affect"))
	    {
	      af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

	      sscanf (buf, "Affect %d %d %d %d %d %d %d\n",
		      &af->type,
		      &af->a.spell.duration,
		      &af->a.spell.modifier,
		      &af->a.spell.location,
		      &af->a.spell.bitvector,
		      &af->a.spell.sn, &af->a.spell.t);
	      if (af->type == 3416)
		{
		  mem_free (af);
		}
	      else
		{
		  af->next = NULL;
		  if (af->a.spell.location <= APPLY_CON)
		    affect_to_char (ch, af);
		  else if (af->a.spell.location >= FIRST_APPLY_SKILL &&
			   af->a.spell.location <= LAST_APPLY_SKILL)
		    affect_to_char (ch, af);
		  else
		    {
		      af->next = ch->hour_affects;
		      ch->hour_affects = af;
		    }
		}
	    }
	}
    }

  ch->age = atoi (row[60]);
  ch->intoxication = atoi (row[61]);
  ch->hunger = atoi (row[62]);
  ch->thirst = atoi (row[63]);
  ch->height = atoi (row[64]);
  ch->frame = atoi (row[65]);
  ch->damage = atoi (row[66]);
  ch->lastregen = atoi (row[67]);
  ch->last_room = atoi (row[68]);
  ch->mana = atoi (row[69]);
  ch->max_mana = atoi (row[70]);
  ch->pc->last_logon = atoi (row[71]);
  ch->pc->last_logoff = atoi (row[72]);
  ch->pc->last_disconnect = atoi (row[73]);
  ch->pc->last_connect = atoi (row[74]);
  ch->pc->last_died = atoi (row[75]);

  if (atoi (row[76]) > 0)
    ch->affected_by |= AFF_HOODED;

  if (strlen (row[77]) > 1)
    ch->pc->imm_enter = str_dup (row[77]);
  if (strlen (row[78]) > 1)
    ch->pc->imm_leave = str_dup (row[78]);
  if (strlen (row[79]) > 1)
    ch->pc->site_lie = str_dup (row[79]);
  if (strlen (row[80]) > 1 && str_cmp (row[80], "(null)"))
    ch->voice_str = str_dup (row[80]);

  if (str_cmp (row[81], "~") && str_cmp (row[81], " "))
    {
      while (1)
	{
	  row[81] = one_argument (row[81], buf);
	  row[81] = one_argument (row[81], buf2);
	  if (!*buf2)
	    break;
	  add_clan_id (ch, buf2, buf);
	}
    }

  if (str_cmp (row[82], "~") && str_cmp (row[82], " "))
    {
      while (1)
	{
	  if (!row[82] || !*row[82])
	    break;
	  get_line (&row[82], buf);
	  if (!*buf || !str_cmp (buf, "~") || !str_cmp (buf, " "))
	    break;
	  sscanf (buf, "%s %d\n", buf2, &lev);
	  if (!*buf2)
	    continue;
	  if ((ind = index_lookup (skills, buf2)) == -1)
	    continue;
	  ch->skills[ind] = lev;
	  ch->pc->skills[ind] = ch->skills[ind];
	}
    }

  if (str_cmp (row[83], "~") && str_cmp (row[83], " "))
    {
      while (1)
	{
	  if (!row[83] || !*row[83])
	    break;
	  get_line (&row[83], buf);
	  if (!*buf)
	    break;

	  sscanf (buf, "%s %s %s %s %d %d %d %d %d %d %d %d %d", location,
		  type, severity, wound_name, &damage, &bleeding, &poison,
		  &infection, &healerskill, &lasthealed, &lastbled,
		  &bindskill, &lastbound);

	  CREATE (wound, WOUND_DATA, 1);
	  wound->next = NULL;

	  wound->location = str_dup (location);
	  wound->type = str_dup (type);
	  wound->severity = str_dup (severity);
	  wound->name = str_dup (wound_name);
	  wound->damage = damage;
	  wound->bleeding = bleeding;
	  wound->poison = poison;
	  wound->infection = infection;
	  wound->healerskill = healerskill;
	  wound->lasthealed = lasthealed;
	  wound->lastbled = lastbled;
	  wound->bindskill = bindskill;
	  wound->lastbound = lastbound;

	  if (!ch->wounds)
	    ch->wounds = wound;
	  else
	    {
	      tmpwound = ch->wounds;
	      while (tmpwound->next)
		tmpwound = tmpwound->next;
	      tmpwound->next = wound;
	    }
	}
    }

  if (str_cmp (row[84], "~") && str_cmp (row[84], " "))
    {
      while (1)
	{
	  if (!row[84] || !*row[84])
	    break;
	  get_line (&row[84], buf);
	  if (!*buf)
	    break;

	  sscanf (buf, "%s %d", location, &i);

	  if (!*location)
	    continue;

	  CREATE (lodged, LODGED_OBJECT_INFO, 36);
	  lodged->next = NULL;

	  lodged->location = str_dup (location);
	  lodged->vnum = i;

	  if (!ch->lodged)
	    ch->lodged = lodged;
	  else
	    {
	      tmplodged = ch->lodged;
	      while (tmplodged->next)
		tmplodged = tmplodged->next;
	      tmplodged->next = lodged;
	    }
	}
    }

  ch->writes = atoi (row[85]);

  ch->pc->profession = atoi (row[86]);

  ch->was_in_room = atoi (row[87]);

  if (row[88] && strlen (row[88]) > 1 && str_cmp (row[88], "(null)"))
    ch->travel_str = str_dup (row[88]);

  ch->bmi = atoi (row[90]);

  ch->guardian_mode = atoi (row[91]);

  ch->hire_storeroom = atoi (row[92]);
  ch->hire_storeobj = atoi (row[93]);

  if (row[94] && *row[94])
    {
      ch->plan = new std::string(row[94]);
    }

  if (row[95] && *row[95])
    {
      ch->goal = new std::string(row[95]);
    }
	
	// Code added by Vermonkey: 081021
	// extracts three flags from row 97
	// room for 5 more flags!
	{
		int temp =  atoi (row[97]);
		if((temp >> 0) % 2)
			ch->toggleNaughtyFlag();
		if((temp >> 1) % 2)
			ch->toggleRPFlag();
		if((temp >> 2) % 2)
			ch->togglePlotFlag();
	}
	
	// Persistant dmotes, wooo!
	if (row[98] && strlen (row[98]) > 1 && str_cmp (row[98], "(null)"))
		ch->dmote_str = str_dup (row[98]);
	
	// Load our UID
	ch->pc->char_num = atoi( row[101] );

	// Determines the number of chars to wrap the screen to
	ch->setWrapLength( atoi( row[102] ) );
	// END Vermonkey
	
	//Grommit - petition flags
	ch->petition_flags = ( row[103] == 0 ? 0 : ( atoi ( row[103] ) ));

  fix_offense (ch);

  load_dreams (ch);

  if (!ch->coldload_id)
    ch->coldload_id = get_next_coldload_id (1);

  if (ch->race == 28)
    {
      ch->max_hit = 200 + (GET_CON (ch) * CONSTITUTION_MULTIPLIER) + (MIN(GET_AUR(ch),25));
      ch->armor = 3;
    }
  else
    {
      ch->max_hit = 50 + (GET_CON (ch) * CONSTITUTION_MULTIPLIER) + (MIN(GET_AUR(ch),25)) ;
      ch->armor = 0;
    }

  if (!ch->max_mana
      && (ch->skills[SKILL_BLACK_WISE] || ch->skills[SKILL_WHITE_WISE]
	  || ch->skills[SKILL_GREY_WISE]))
    {
      ch->max_mana = 100;
      ch->mana = ch->max_mana;
    }

  ch->pc->is_guide = is_guide (ch->pc->account_name);
  
  int old = ch->pc->skills[1];
  
  //load_skills_mysql(ch, false);
  
  if (ch->pc->skills[1] != old){
	send_to_char("FAIL", ch);
	}

  if (ch->speaks == SKILL_HEALING)
    ch->speaks = SKILL_SPEAK_WESTRON;

  mysql_free_result (result);
/*
	if ( lookup_race_variable (ch->race, RACE_LAST_MODIFIED) &&
	     ch->pc->last_logon <= atoi(lookup_race_variable(ch->race, RACE_LAST_MODIFIED)) )
		refresh_race_configuration(ch);
*/
  if (lookup_race_variable (ch->race, RACE_BODY_PROTO) != NULL)
    ch->body_proto = atoi (lookup_race_variable (ch->race, RACE_BODY_PROTO));

  if (is_admin (ch->pc->account_name))
    ch->flags |= FLAG_ISADMIN;
  else
    ch->flags &= ~FLAG_ISADMIN;

  ch->fight_percentage = 100;

  return ch;
}

void
save_char_mysql (CHAR_DATA * ch)
{
  AFFECTED_TYPE *af;
  WOUND_DATA *wound;
  LODGED_OBJECT_INFO *lodged_obj;
  MYSQL_RES *result;
  char buf[MAX_STRING_LENGTH];
  char wounds[MAX_STRING_LENGTH], lodged[MAX_STRING_LENGTH];
  char skills_buf[MAX_STRING_LENGTH], affects[2*MAX_STRING_LENGTH]; // double size needed for huge crafts list
  int i = 0, hooded = 0;
  
  if (IS_NPC (ch) || IS_SET (ch->flags, FLAG_GUEST))
    return;

	/* adjust last_logoff to now for offline healing tracking in event of crash, but ONLY if PC is online. */
  if (ch->desc)
	ch->pc->last_logoff = time(0);

  *wounds = '\0';
  *skills_buf = '\0';
  *lodged = '\0';
  *affects = '\0';

  if (ch->wounds)
    {
      *buf = '\0';
      for (wound = ch->wounds; wound; wound = wound->next)
	sprintf (buf + strlen (buf),
		 "%s %s %s %s %d %d %d %d %d %d %d %d %d\n", wound->location,
		 wound->type, wound->severity, wound->name, wound->damage,
		 wound->bleeding, wound->poison, wound->infection,
		 wound->healerskill, wound->lasthealed, wound->lastbled,
		 wound->bindskill, wound->lastbound);
      sprintf (wounds, "%s", buf);
    }

  *buf = '\0';
  for (i = 0; i <= LAST_SKILL; i++)
    {
      if (real_skill (ch, i))
	sprintf (buf + strlen (buf), "%-15s   %d\n", skills[i],
		 real_skill (ch, i));
    }
  sprintf (skills_buf, "%s", buf);

  *buf = '\0';
  for (af = ch->hour_affects; af; af = af->next)
    {
      if ((af->type < MAGIC_CLAN_MEMBER_BASE ||
	   af->type > MAGIC_CLAN_OMNI_BASE + MAX_CLANS) &&
	  (af->type < CRAFT_FIRST ||
	   af->type > CRAFT_LAST) &&
	  af->type != MAGIC_CLAN_NOTIFY &&
	  af->type != MAGIC_NOTIFY &&
	  af->type != MAGIC_WATCH1 &&
	  af->type != MAGIC_WATCH2 &&
	  af->type != MAGIC_WATCH3 &&
	  af->type != MAGIC_GUARD &&
	  af->type != AFFECT_SHADOW &&
	  (af->type > CRAFT_LAST || af->type < CRAFT_FIRST))
	sprintf (buf + strlen (buf), "Affect	%d %d %d %d %d %d %d\n",
		 af->type, af->a.spell.duration, af->a.spell.modifier,
		 af->a.spell.location, af->a.spell.bitvector,
		 af->a.spell.sn, af->a.spell.t);
      else if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
	sprintf (buf + strlen (buf), "Subcraft     '%s'\n",
		 af->a.craft->subcraft->subcraft_name);
    }
  sprintf (affects, "%s", buf);

  *buf = '\0';
  for (lodged_obj = ch->lodged; lodged_obj; lodged_obj = lodged_obj->next)
    sprintf (buf + strlen (buf), "%s %d\n", lodged_obj->location,
	     lodged_obj->vnum);

  sprintf (lodged, "%s", buf);
  std::string player_db = engine.get_config ("player_db");
  mysql_safe_query ("SELECT name FROM %s.pfiles WHERE name = '%s'",
		    player_db.c_str (), ch->tname);
  result = mysql_store_result (database);

  if (result && mysql_num_rows (result) >= 1)
    {				// Update an existing PC record.
      mysql_free_result (result);
      mysql_safe_query
	("UPDATE %s.pfiles SET keywords = '%s', account = '%s', sdesc = '%s', ldesc = '%s', description = '%s', msg = '%s', create_comment = '%s', create_state = %d, "
	 "nanny_state = %d, role = %d, role_summary = '%s', role_body = '%s', role_date = '%s', role_poster = '%s', role_cost = %d, app_cost = %d, level = %d, sex = %d, deity = %d, "
	 "race = %d, room = %d, str = %d, intel = %d, wil = %d, con = %d, dex = %d, aur = %d, agi = %d, start_str = %d, start_intel = %d, start_wil = %d, start_con = %d, start_dex = %d, start_aur = %d, "
	 "start_agi = %d, played = %d, birth = %d, time = %d, offense = %d, hit = %d, maxhit = %d, nat_attack_type = %d, move = %d, maxmove = %d, circle = %d, ppoints = %d, fightmode = %d, color = %d, "
	 "speaks = %d, flags = %d, plrflags = %d, boatvnum = %d, speed = %d, mountspeed = %d, sleepneeded = %d, autotoll = %d, coldload = %d, affectedby = %d, "
	 "affects = '%s', age = %d, intoxication = %d, hunger = %d, thirst = %d, height = %d, frame = %d, damage = %d, lastregen = %d, lastroom = %d, harness = %d, maxharness = %d, "
	 "lastlogon = %d, lastlogoff = %d, lastdis = %d, lastconnect = %d, lastdied = %d, hooded = %d, immenter = '%s', immleave = '%s', sitelie = '%s', voicestr = '%s', clans = '%s', skills = '%s', "
	 "wounds = '%s', lodged = '%s', writes = %d, profession = %d, was_in_room = %d, travelstr = '%s', bmi = %d, guardian_mode = %d, hire_storeroom = %d, hire_storeobj = %d, plan = '%s', goal = '%s', "
	 "role_id = %d, player_flags = %d, dmotestr = '%s', wraplen = %d, petition_flags = %d WHERE name = '%s'",
	 player_db.c_str (), ch->name, ch->pc->account_name, ch->short_descr,
	 ch->long_descr, ch->description, ch->pc->msg,
	 ch->pc->creation_comment, ch->pc->create_state, ch->pc->nanny_state,
	 0, ch->pc->special_role ? ch->pc->special_role->summary : "~",
	 ch->pc->special_role ? ch->pc->special_role->body : "~",
	 ch->pc->special_role ? ch->pc->special_role->date : "~",
	 ch->pc->special_role ? ch->pc->special_role->poster : "~",
	 ch->pc->special_role ? ch->pc->special_role->cost : 0,
	 ch->pc->app_cost, ch->pc->level, ch->sex, ch->deity, ch->race,
	 ch->in_room, ch->str, ch->intel, ch->wil, ch->con, ch->dex, ch->aur,
	 ch->agi, ch->pc->start_str, ch->pc->start_intel, ch->pc->start_wil,
	 ch->pc->start_con, ch->pc->start_dex, ch->pc->start_aur,
	 ch->pc->start_agi,
	 (int) (ch->time.played + time (0) - ch->time.logon),
	 (int) ch->time.birth, (int) time (0), ch->offense, ch->hit,
	 ch->max_hit, ch->nat_attack_type, ch->move, ch->max_move, ch->circle,
	 ch->ppoints, ch->fight_mode, ch->color, ch->speaks, (int) ch->flags,
	 (int) ch->plr_flags, ch->pc->boat_virtual, ch->speed,
	 ch->pc->mount_speed, (int) ch->pc->sleep_needed, ch->pc->auto_toll,
	 (int) ch->coldload_id, (int) ch->affected_by, affects, ch->age,
	 ch->intoxication, ch->hunger, ch->thirst, ch->height, ch->frame,
	 ch->damage, (int) ch->lastregen, ch->last_room, ch->mana,
	 ch->max_mana, (int) ch->pc->last_logon, (int) ch->pc->last_logoff,    /* use current time as last logoff so if there is a crash, wounds heal properly not spam heal */
	 (int) ch->pc->last_disconnect, (int) ch->pc->last_connect,
	 (int) ch->pc->last_died, hooded, ch->pc->imm_enter,
	 ch->pc->imm_leave, ch->pc->site_lie, ch->voice_str, ch->clans,
	 skills_buf, wounds, lodged, ch->writes, ch->pc->profession,
	 ch->was_in_room, ch->travel_str, ch->bmi,
	 ch->guardian_mode, ch->hire_storeroom, ch->hire_storeobj, 
	 (ch->plan && !ch->plan->empty()) ? ch->plan->c_str() : "",
	 (ch->goal && !ch->goal->empty()) ? ch->goal->c_str() : "",
	 ch->pc->special_role ? ch->pc->special_role->id : 0,     
	(int)((ch->getNaughtyFlag() << 0) + (ch->getRPFlag() << 1) + (ch->getPlotFlag() << 2)), //bool compression of player_flags
	 ch->dmote_str, ch->getWrapLength(), (int)ch->petition_flags, ch->tname);
      save_dreams (ch);
    }
  else
    {				// New PC record.
      if (result)
	mysql_free_result (result);
      mysql_safe_query ("DELETE FROM %s.pfiles WHERE name = '%s'",
			player_db.c_str (), ch->tname);
      mysql_safe_query
	("INSERT INTO %s.pfiles (name, keywords, account, sdesc, ldesc, description, msg, create_comment, create_state, "
	 "nanny_state, role, role_summary, role_body, role_date, role_poster, role_cost, app_cost, level, sex, deity, "
	 "race, room, str, intel, wil, con, dex, aur, agi, start_str, start_intel, start_wil, start_con, start_dex, start_aur, "
	 "start_agi, played, birth, time, offense, hit, maxhit, nat_attack_type, move, maxmove, circle, ppoints, fightmode, color, "
	 "speaks, flags, plrflags, boatvnum, speed, mountspeed, sleepneeded, autotoll, coldload, affectedby, "
	 "affects, age, intoxication, hunger, thirst, height, frame, damage, lastregen, lastroom, harness, maxharness, "
	 "lastlogon, lastlogoff, lastdis, lastconnect, lastdied, hooded, immenter, immleave, sitelie, voicestr, clans, skills, "
	 "wounds, lodged, writes, profession, was_in_room, travelstr, bmi, hire_storeroom, hire_storeobj, plan, goal, role_id, "
	 "player_flags, dmotestr, wraplen) VALUES "
	 "('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, "
	 "%d, %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, "
	 "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
	 "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
	 "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
	 "'%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
	 "%d, %d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', "
	 "'%s', '%s', %d, %d, %d, '%s', %d, %d, %d, '%s', '%s', %d, %d, '%s', %d)", 
	 player_db.c_str (),
	 ch->tname, ch->name, ch->pc->account_name, ch->short_descr,
	 ch->long_descr, ch->description, ch->pc->msg,
	 ch->pc->creation_comment, ch->pc->create_state, ch->pc->nanny_state,
	 0, ch->pc->special_role ? ch->pc->special_role->summary : "~",
	 ch->pc->special_role ? ch->pc->special_role->body : "~",
	 ch->pc->special_role ? ch->pc->special_role->date : "~",
	 ch->pc->special_role ? ch->pc->special_role->poster : "~",
	 ch->pc->special_role ? ch->pc->special_role->cost : 0,
	 ch->pc->app_cost, ch->pc->level, ch->sex, ch->deity, ch->race,
	 ch->in_room, ch->str, ch->intel, ch->wil, ch->con, ch->dex, ch->aur,
	 ch->agi, ch->pc->start_str, ch->pc->start_intel, ch->pc->start_wil,
	 ch->pc->start_con, ch->pc->start_dex, ch->pc->start_aur,
	 ch->pc->start_agi,
	 (int) (ch->time.played + time (0) - ch->time.logon),
	 (int) ch->time.birth, (int) time (0), ch->offense, ch->hit,
	 ch->max_hit, ch->nat_attack_type, ch->move, ch->max_move, ch->circle,
	 ch->ppoints, ch->fight_mode, ch->color, ch->speaks, (int) ch->flags,
	 (int) ch->plr_flags, ch->pc->boat_virtual, ch->speed,
	 ch->pc->mount_speed, (int) ch->pc->sleep_needed, ch->pc->auto_toll,
	 (int) ch->coldload_id, (int) ch->affected_by, affects, ch->age,
	 ch->intoxication, ch->hunger, ch->thirst, ch->height, ch->frame,
	 ch->damage, (int) ch->lastregen, ch->last_room, ch->mana,
	 ch->max_mana, (int) ch->pc->last_logon, (int) ch->pc->last_logoff,
	 (int) ch->pc->last_disconnect, (int) ch->pc->last_connect,
	 (int) ch->pc->last_died, hooded, ch->pc->imm_enter,
	 ch->pc->imm_leave, ch->pc->site_lie, ch->voice_str, ch->clans,
	 skills_buf, wounds, lodged, ch->writes, ch->pc->profession,
	 ch->was_in_room, ch->travel_str, ch->bmi,
	 ch->hire_storeroom, ch->hire_storeobj,
	 (ch->plan && !ch->plan->empty()) ? ch->plan->c_str() : "",
	 (ch->goal && !ch->goal->empty()) ? ch->goal->c_str() : "",
   	 ch->pc->special_role ? ch->pc->special_role->id : 0,
	(int)((ch->getNaughtyFlag() << 0) + (ch->getRPFlag() << 1) + (ch->getPlotFlag() << 2)), //bool compression of player_flags
	ch->dmote_str, ch->getWrapLength()
	);
    }
	
//	save_skills_mysql(ch, false);
}

int
check_account_flags (char *host)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  int flag = 0;

  mysql_safe_query
    ("SELECT account_flags FROM forum_users WHERE user_last_ip = '%s'", host);
  result = mysql_store_result (database);

  while ((row = mysql_fetch_row (result)))
    flag = atoi (row[0]);

  mysql_free_result (result);

  return flag;
}

char *
reference_ip (char *guest_name, char *host)
{
  static char gname[MAX_STRING_LENGTH];
  MYSQL_RES *result;
  MYSQL_ROW row;

  if (strstr (host, "middle-earth.us"))
    return guest_name;

  mysql_safe_query
    ("SELECT username FROM forum_users WHERE user_last_ip = '%s'", host);
  result = mysql_store_result (database);

  while ((row = mysql_fetch_row (result)))
    {
      sprintf (gname, "%s", row[0]);
      mysql_free_result (result);
      return gname;
    }

  mysql_free_result (result);

  return guest_name;
}

void
load_stayput_mobiles ()
{
  FILE *fp;
  MYSQL_RES *result;
  MYSQL_ROW row;
  CHAR_DATA *mob;
  char query[MAX_STRING_LENGTH];

  if (!engine.in_play_mode ())
    return;

  mysql_safe_query ("SELECT * FROM stayput_mobiles");
  result = mysql_store_result (database);

  system_log ("Loading stayput mobiles...", false);

  while ((row = mysql_fetch_row (result)))
    {
      sprintf (query, "save/mobiles/%s", row[2]);
      fp = fopen (query, "r");
      if (!fp)
	continue;
      mob = load_a_saved_mobile (atoi (row[0]), fp, false);
      if (mob)
	char_to_room (mob, atoi (row[1]));
      mob = NULL;
      fclose (fp);
    }

		CHAR_DATA *temp_mob;
        for (ROOM_DATA *temp_room = full_room_list; temp_room; temp_room = temp_room->lnext)
                {
                        for (CHAR_DATA *temp_mob = temp_room->people; temp_mob; temp_mob = temp_mob->next_in_room)
                        {
                                bool mob_found = false;
                                //for (CHAR_DATA *temp_mob_from_list = character_list; temp_mob_from_list; temp_mob_from_list = temp_mob_from_list->next)
								for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
                                {
                                        if (temp_mob == *tch_iterator)
                                                mob_found = true;
                                }
                                if (!mob_found)
                                {
                                        //temp_mob->next = character_list;
                                        //character_list = temp_mob;
										if (temp_mob)
											character_list.push_front(temp_mob);
                                }
                        }
                }


  mysql_free_result (result);
}

void
save_reboot_mobiles ()
{
  CHAR_DATA *mob;
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  if (!finished_booting)
    return;

  //for (mob = character_list; mob; mob = mob->next)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	mob = *tch_iterator;
      if (mob->deleted)
	continue;
      if (!IS_NPC (mob) && mob->desc && !mob->desc->original)
	continue;
      if (IS_RIDEE (mob) && !IS_NPC (mob->mount))
	continue;
      if (IS_HITCHEE (mob) && !IS_NPC (mob->hitcher))
	continue;
      if (mob->deleted)
	continue;
      if (!mob->room)
	continue;
      sprintf (buf, "save/reboot/%d", mob->coldload_id);
      fp = fopen (buf, "w");
      save_mobile (mob, fp, "STAYPUT", 0);
      fclose (fp);

      mysql_safe_query ("DELETE FROM reboot_mobiles WHERE coldload_id = %d",
			mob->coldload_id);
      mysql_safe_query ("INSERT INTO stayput_mobiles VALUES (%d, %d, %d, %d)", mob->mob->nVirtual, mob->in_room, mob->coldload_id, mob->morph_time);
    }
}

void
save_stayput_mobiles ()
{
  CHAR_DATA *mob;
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  if (!finished_booting)
    return;

  //for (mob = character_list; mob; mob = mob->next)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	mob = *tch_iterator;
      if (mob->deleted)
	continue;
      if (!IS_SET (mob->act, ACT_STAYPUT))
	continue;
      if (!IS_NPC (mob) && mob->desc && !mob->desc->original)
	continue;
      if (IS_HITCHEE (mob))
	continue;
      if (IS_RIDEE (mob))
	continue;
      sprintf (buf, "save/mobiles/%d", mob->coldload_id);
      fp = fopen (buf, "w");
      save_mobile (mob, fp, "STAYPUT", 0);
      fclose (fp);

     	mysql_safe_query ("DELETE FROM stayput_mobiles WHERE coldload_id = %d", mob->coldload_id);
     	mysql_safe_query ("INSERT INTO stayput_mobiles VALUES (%d, %d, %d, %d)", mob->mob->nVirtual, mob->in_room, mob->coldload_id, mob->morph_time);
    }
}

void
insert_newsletter_into_website (int timestamp, char *newsletter)
{
  mysql_safe_query ("INSERT INTO newsletters VALUES (%d, '%s')", timestamp,
		    newsletter);
}

void
update_website_statistics (void)
{
  FILE *fp;
  DESCRIPTOR_DATA *d;
  ROOM_DATA *room;
  CHAR_DATA *mob;
  OBJ_DATA *obj;
  int craft_tot = 0, objects = 0, mobs = 0, rooms = 0, players = 0;

  if (engine.in_play_mode ())
    {
      for (d = descriptor_list; d; d = d->next)
	{
	  if (!d->character || IS_NPC (d->character))
	    continue;
	  if (!IS_MORTAL (d->character))
	    continue;
	  if (d->connected != CON_PLYNG)
	    continue;
	  players++;
	}
      if ((fp = fopen ("mrtg/player_stats", "w")))
	{
	  fprintf (fp, "#%d", players);
	  fclose (fp);
	}
      system ("env LANG=C /usr/bin/mrtg /home/shadows/shadows/pp/lib/mrtg/mrtg.cfg &");
    }

  for (obj = full_object_list; obj; obj = obj->lnext)
    if (!obj->deleted)
      objects++;

  for (mob = full_mobile_list; mob; mob = mob->mob->lnext)
    if (!mob->deleted)
      mobs++;

  for (room = full_room_list; room; room = room->lnext)
    rooms++;

  mysql_safe_query ("DELETE FROM building_totals");

  mysql_safe_query ("INSERT INTO building_totals VALUES (%d, %d, %d, %d)",
		    rooms, objects, mobs, craft_tot);

}

void
load_tracks (void)
{
  ROOM_DATA *room;
  TRACK_DATA *track, *tmp_track;
  MYSQL_RES *result;
  MYSQL_ROW row;

  mysql_safe_query ("SELECT * FROM tracks");
  result = mysql_store_result (database);

  if (!result)
    return;

  while ((row = mysql_fetch_row (result)))
    {
      if (!(room = vtor (atoi (row[0]))))
	continue;
      CREATE (track, TRACK_DATA, 1);
      track->next = NULL;
      if (!room->tracks)
	room->tracks = track;
      else
	for (tmp_track = room->tracks; tmp_track; tmp_track = tmp_track->next)
	  {
	    if (!tmp_track->next)
	      {
		tmp_track->next = track;
		break;
	      }
	  }
      track->race = atoi (row[1]);
      track->from_dir = atoi (row[2]);
      track->to_dir = atoi (row[3]);
      track->hours_passed = atoi (row[4]);
      track->speed = atoi (row[5]);
      track->flags = atoi (row[6]);
    }

  mysql_free_result (result);
  result = NULL;
}

void
save_tracks (void)
{
  ROOM_DATA *room;
  TRACK_DATA *track;

  return; /// \todo Find out why this is turned off

  if (!engine.in_play_mode ())
    return;

  mysql_safe_query ("DELETE FROM tracks");

  for (room = full_room_list; room; room = room->lnext)
    {
      if (!room->tracks)
	continue;
      for (track = room->tracks; track; track = track->next)
	{
	  if (!track)
	    continue;
	  if (!IS_SET (track->flags, PC_TRACK))
	    continue;
	  mysql_safe_query
	    ("INSERT INTO tracks VALUES(%d, %d, %d, %d, %d, %d, %d)",
	     room->nVirtual, track->race, track->from_dir, track->to_dir,
	     track->hours_passed, track->speed, (int) track->flags);
	}
    }
}

void
save_hobbitmail_message (account * acct, MUDMAIL_DATA * message)
{
  if (!acct || acct->name.empty () || !message)
    return;

  mysql_safe_query
    ("INSERT INTO hobbitmail (account, flags, from_line, from_account, sent_date, subject, message, timestamp, to_line)"
     " VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', UNIX_TIMESTAMP(), '%s' )",
     acct->name.c_str (), (int) message->flags, message->from,
     message->from_account, message->date, message->subject, message->message,
     message->target);
}


void
mark_as_read (CHAR_DATA * ch, int number)
{
  mysql_safe_query
    ("UPDATE player_notes SET flags = 1 WHERE name = '%s' AND post_number = %d",
     ch->tname, number);
}

void
display_mysql_board_message (CHAR_DATA * ch, char *board_name, int msg_num,
			     bool bHideHeader)
{
  char query[MAX_STRING_LENGTH];
  MYSQL_RES *result;
  MYSQL_ROW row;

  if (msg_num < 1)
    {
      mysql_safe_query
	("SELECT count(post_number) FROM boards WHERE board_name = '%s'",
	 board_name);
      if ((result = mysql_store_result (database)) != NULL)
	{
	  row = mysql_fetch_row (result);
	  msg_num = number (1, atoi (row[0]));
	  mysql_free_result (result);
	  mysql_safe_query
	    ("SELECT * FROM boards WHERE board_name = '%s' LIMIT %d, 1",
	     board_name, msg_num - 1);
	}
      else
	{
	  return;
	}
    }
  else
    {
      mysql_safe_query
	("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d",
	 board_name, msg_num);
    }
  result = mysql_store_result (database);

  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      send_to_char ("That message does not seem to exist in the database.\n",
		    ch);
      return;
    }

  row = mysql_fetch_row (result);

  *b_buf = '\0';
  *query = '\0';

  if (!bHideHeader)
    {
      if (*row[4])
	sprintf (query, " (%s)", row[4]);
      sprintf (b_buf + strlen (b_buf), "\n#6Date:#0    %s%s\n", row[5],
	       !IS_MORTAL (ch) && *query ? query : "");
      if (!IS_MORTAL (ch))
	{
	  sprintf (b_buf + strlen (b_buf), "#6Author:#0  %s\n", row[3]);
	}
      sprintf (b_buf + strlen (b_buf), "#6Subject:#0 %s\n\n", row[2]);
    }
  sprintf (b_buf + strlen (b_buf), "%s", row[6]);
  page_string (ch->desc, b_buf);

  mysql_free_result (result);
}

void
retrieve_mysql_board_listing (CHAR_DATA * ch, char *board_name)
{
  mysql_safe_query ("SELECT CONCAT("
		    "'#6', "
		    "LPAD(post_number,5,' '), "
		    "':#0 ', "
		    "LPAD(ic_date, 24, ' '), "
		    "' - ', "
		    "SUBSTRING(subject,1,50)"
		    ") AS formatted "
		    "FROM boards "
		    "WHERE board_name = '%s' "
		    "ORDER BY post_number DESC "
		    "LIMIT 220",
		    board_name);

  MYSQL_RES* result = mysql_store_result (database);

  if (result)
    {
      if (mysql_num_rows (result))
	{
	  std::string listing;
	  std::string subject;
	  MYSQL_ROW row;

	  while ((row = mysql_fetch_row (result)))
	    {
	      subject = row[0];
	      if (subject.length () > 82)
		{
		  subject.erase (79, std::string::npos);
		  subject += "...";		  
		}

	      listing += subject + "\n";
	    }
	  if (listing.length () > MAX_STRING_LENGTH)
	    {
	      listing.erase (MAX_STRING_LENGTH - 10, std::string::npos);
	    }
	  page_string (ch->desc, listing.c_str ());
	}
      else
	{
	  send_to_char ("No messages found for this listing.\n", ch);
	}
      mysql_free_result (result);
    }
  else
    {
      std::ostringstream error_message;
      error_message << "Error: " << std::endl
		    << __FILE__ << ':'
		    << __func__ << '('
		    << __LINE__ << "):" << std::endl
		    << mysql_error (database);
      send_to_gods ((error_message.str ()).c_str());
    }
}

void
post_to_mysql_board (DESCRIPTOR_DATA * d)
{
  char date_buf[MAX_STRING_LENGTH];
  char *date, *suf;
  int free_slot = 0, day;
  MYSQL_RES *result;
  MYSQL_ROW row;
  time_t current_time;

  if (!*d->pending_message->message)
    {
      send_to_char ("No message posted.\n", d->character);
      unload_message (d->pending_message);
      d->pending_message = NULL;
      return;
    }

  current_time = time (0);
  date = asctime (localtime (&current_time));
  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  *date_buf = '\0';
  day = time_info.day + 1;
  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  if (time_info.holiday == 0 &&
      !(time_info.month == 1 && day == 12) &&
      !(time_info.month == 4 && day == 10) &&
      !(time_info.month == 7 && day == 11) &&
      !(time_info.month == 10 && day == 12))
    sprintf (date_buf, "%d%s %s, %d SR", day, suf,
	     month_short_name[time_info.month], time_info.year);
  else
    {
      if (time_info.holiday > 0)
	{
	  sprintf (date_buf, "%s, %d SR",
		   holiday_short_names[time_info.holiday], time_info.year);
	}
      else if (time_info.month == 1 && day == 12)
	sprintf (date_buf, "Erukyerme, %d SR", time_info.year);
      else if (time_info.month == 4 && day == 10)
	sprintf (date_buf, "Lairemerende, %d SR", time_info.year);
      else if (time_info.month == 7 && day == 11)
	sprintf (date_buf, "Eruhantale, %d SR", time_info.year);
      else if (time_info.month == 10 && day == 12)
	sprintf (date_buf, "Airilaitale, %d SR", time_info.year);
    }

  if (isalpha (*date_buf))
    *date_buf = toupper (*date_buf);

  mysql_safe_query
    ("SELECT * FROM boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
     d->pending_message->poster);
  result = mysql_store_result (database);

  if (!result)
    {
      send_to_char
	("There seems to be a problem with the database listing.\n",
	 d->character);
      return;
    }

  free_slot = 1;

  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[1]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', %d)",
     d->pending_message->poster, free_slot, d->pending_message->subject,
     d->character->tname, date, date_buf, d->pending_message->message,
     (int) time (0));

  if (result)
    mysql_free_result (result);

  unload_message (d->pending_message);
  d->pending_message = NULL;
  mem_free (date);
}

/* This is mainly a "test" command - not really functional for any one speciifc purpose! */

void
do_mysql (CHAR_DATA * ch, char *argument, int cmd)
{
  DESCRIPTOR_DATA *d;
  SUBCRAFT_HEAD_DATA *craft;
  ROOM_DATA *room;
  CHAR_DATA *tch;
  OBJ_DATA *obj;
  TRACK_DATA *track;
  WOUND_DATA *wound, *next_wound;
  char buf[MAX_STRING_LENGTH];
  char severity[MAX_STRING_LENGTH];
  int purse = 0, i = 0, time_passed = 0, num_wounds = 0, points = 0;
  struct time_info_data passed;

  if (!IS_IMPLEMENTOR (ch))
    {
      send_to_char ("Eh?\n", ch);
      return;
    }

  if (!*argument)
    {
      send_to_char ("What query?\n", ch);
      return;
    }

  if (!str_cmp (argument, "loop"))
    {
      while (1)
	{
	}
    }

  if (!strn_cmp (argument, "logging", 7))
    {
      argument = one_argument (argument, buf);
      if (!str_cmp (argument, "off"))
	{
	  mysql_logging = false;
	  send_to_char ("MySQL logging is disabled.\n", ch);
	  return;
	}
      mysql_logging = true;
      send_to_char ("MySQL logging is enabled.\n", ch);
      return;
    }

  if (!strn_cmp (argument, "loaded_list", 11))
  {
	  for (tch = loaded_list; tch; tch = tch->next)
	  {
		  sprintf(buf, "%s\n", tch->tname);
		  send_to_char(buf, ch);
	  }
  }

  if (!strn_cmp(argument, "pc_list", 7))
  {
	  std::string output;
	  for (std::list<char_data*>::iterator it = character_list.begin(); it != character_list.end(); it++)
	  {
		  tch = *it;
		  if (IS_NPC(tch))
			  continue;
		  else
			  sprintf(buf, "[PC] #5%s#0 (%s) (%d)\n", char_short(tch), tch->tname, tch->coldload_id);
		  output += buf;
	  }
	  page_string(ch->desc, (char *) output.c_str());
  }

  if (!strn_cmp (argument, "character_list", 14))
  {
	  std::string output;
	  for (std::list<char_data*>::iterator it = character_list.begin(); it != character_list.end(); it++)
	  {
		  tch = *it;
		  if (IS_NPC(tch))
			  sprintf(buf, "[%d] #5%s#0 (%d)\n", tch->mob->nVirtual, char_short(tch), tch->coldload_id);
		  else
			  sprintf(buf, "[PC] #5%s#0 (%s) (%d)\n", char_short(tch), tch->tname, tch->coldload_id);
		  output += buf;
	  }
	  page_string(ch->desc, (char *) output.c_str());
  }

  if (!str_cmp (argument, "keepers"))
    {
      for (tch = full_mobile_list; tch; tch = tch->mob->lnext)
	{
	  if (!IS_SET (tch->flags, FLAG_KEEPER))
	    continue;
	  tch->shop->buy_flags = 0;
	  tch->shop->nobuy_flags = 0;
	}
      send_to_char ("Done.\n", ch);
      return;
    }

	if (!str_cmp (argument, "listfix"))
	{
		CHAR_DATA *temp_mob;
        for (ROOM_DATA *temp_room = full_room_list; temp_room; temp_room = temp_room->lnext)
                {
                        for (CHAR_DATA *temp_mob = temp_room->people; temp_mob; temp_mob = temp_mob->next_in_room)
                        {
                                bool mob_found = false;
                                //for (CHAR_DATA *temp_mob_from_list = character_list; temp_mob_from_list; temp_mob_from_list = temp_mob_from_list->next)
								for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
                                {
                                        if (temp_mob == *tch_iterator)
                                                mob_found = true;
                                }
                                if (!mob_found)
                                {
                                        //temp_mob->next = character_list;
                                        //character_list = temp_mob;
										if (temp_mob)
											character_list.push_front(temp_mob);
                                }
                        }
                }
	}

  if (!str_cmp (argument, "host"))
    {
      sprintf (buf, "Host: %s\nServer: %s\n Status: %s\n",
	       mysql_get_host_info (database),
	       mysql_get_server_info (database), mysql_stat (database));
      send_to_char (buf, ch);
      return;
    }

  if (!str_cmp (argument, "fixtent"))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (GET_ITEM_TYPE (obj) == ITEM_DWELLING
	      && obj->o.od.value[0] >= 100000)
	    {
	      if (!(room = vtor (obj->o.od.value[0])))
		continue;
	      if (!room->dir_option[OUTSIDE])
		{
		  CREATE (room->dir_option[OUTSIDE],
			  struct room_direction_data, 1);
		  room->dir_option[OUTSIDE]->general_description =
		    str_dup ("");
		  room->dir_option[OUTSIDE]->keyword = str_dup ("entryway");
		  room->dir_option[OUTSIDE]->exit_info |= (1 << 0);
		  room->dir_option[OUTSIDE]->key = obj->o.od.value[3];
		  room->dir_option[OUTSIDE]->to_room = obj->in_room;
		  room->dir_option[OUTSIDE]->pick_penalty =
		    obj->o.od.value[4];
		}
	    }
	}
      send_to_char ("Done.\n", ch);
      return;
    }

  if (!str_cmp (argument, "refresh"))
    {
      refresh_db_connection ();
      send_to_char ("Connection refresh triggered.\n", ch);
      return;
    }

  if (!str_cmp (argument, "test"))
    {
      sprintf (buf, "File name %s, line number %ld", __FILE__,
	       (long) __LINE__);
      send_to_char (buf, ch);
      d = NULL;
      SEND_TO_Q ("testing", d);
      return;
    }

  if (!strn_cmp (argument, "testcraft", 9))
    {
      argument = one_argument (argument, buf);
      argument = one_argument (argument, buf);

      if (!(tch = load_pc (buf)))
	{
	  send_to_char ("No such PC.\n", ch);
	  return;
	}

      if (!*argument)
	{
	  send_to_char ("Which craft did you wish to test?\n", ch);
	  unload_pc (tch);
	  return;
	}

      for (craft = crafts; craft; craft = craft->next)
	{
	  if (!str_cmp (craft->subcraft_name, argument))
	    break;
	}

      if (!craft)
	{
	  send_to_char ("No such craft.\n", ch);
	  unload_pc (tch);
	  return;
	}

      sprintf (buf,
	       "meets_skill_requirements: %d\nhas_required_crafting_skills: %d\n",
	       meets_skill_requirements (tch, craft),
	       has_required_crafting_skills (tch, craft));

      send_to_char (buf, ch);

      unload_pc (tch);

      return;
    }

  if (!str_cmp (argument, "website"))
    {
      update_website_statistics ();
      return;
    }

  if (!str_cmp (argument, "stayputs"))
    {
      load_stayput_mobiles ();
      send_to_char ("Stayput mobiles reloaded from database.\n", ch);
      return;
    }

  if (!strn_cmp (argument, "wounds", 6))
    {
      argument = one_argument (argument, buf);
      if (*argument)
	{
	  if (!str_cmp (argument, "delete"))
	    {
	      mysql_safe_query ("DELETE FROM wound_data");
	      send_to_char ("Data deleted.\n", ch);
	      return;
	    }
	  num_wounds = atoi (argument);
	  while (num_wounds > 0)
	    {
	      wound_to_char (ch, figure_location (ch, 0), number (1, 60),
			     number (0, 9), 0, 0, 0);
	      num_wounds--;
	    }
	}
      
      extern rpie::server engine;
      engine.disable_timer_abort ();

      for (wound = ch->wounds; wound; wound = next_wound)
	{
	  next_wound = wound->next;
	  sprintf (buf, "%s %s on the %s: ", wound->severity, wound->name,
		   expand_wound_loc (wound->location));
	  *buf = toupper (*buf);
	  time_passed = 0;
	  sprintf (severity, "%s", wound->severity);
	  points = wound->damage;
	  while (wound->damage > 0 && !wound->infection)
	    {
	      natural_healing_check (ch, wound);
	      time_passed += (BASE_PC_STANDARD_HEALING - ch->con / 6) * 60;
	    }
	  passed = real_time_passed (time_passed, 0);
	  mysql_safe_query
	    ("INSERT INTO wound_data VALUES ('%s', %d, %d, %d, %d)", severity,
	     points, time_passed, wound->infection, ch->con);
	  wound_from_char (ch, wound);
	}

      engine.enable_timer_abort ();
    }

  if (!str_cmp (argument, "crash"))
    {
      tch = NULL;
      if (*tch->pc->account_name)
	tch->pc->account_name = '\0';
    }

  if (!str_cmp (argument, "sell"))
    {
      //for (tch = character_list; tch; tch = tch->next)
	  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
	tch = *tch_iterator;
	  if (tch->deleted)
	    continue;
	  if (!IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER))
	    continue;
	  purse = number (100, 450);
	  while (purse > 0)
	    purse -= vnpc_customer (tch, purse);
	}
      send_to_char ("VNPC customers have been triggered.\n", ch);
      return;
    }

  if (!str_cmp (argument, "saverooms2"))
    {
      load_save_room (ch->room);
      send_to_char ("Room loaded.\n", ch);
      return;
    }

  if (!str_cmp (argument, "tracks"))
    {
      for (room = full_room_list; room; room = room->lnext)
	{
	  for (track = room->tracks; track; track = track->next)
	    i++;
	}
      sprintf (buf, "There are #2%d#0 tracks currently on the grid.\n", i);
      send_to_char (buf, ch);
    }

  if (!str_cmp (argument, "locks"))
    {
      for (room = full_room_list; room; room = room->lnext)
	{
	  for (i = 0; i <= LAST_DIR; i++)
	    {
	      if (room->dir_option[i]
		  && (IS_SET (room->dir_option[i]->exit_info, EX_ISDOOR) ||
		  	IS_SET (room->dir_option[i]->exit_info, EX_ISGATE))
		  && IS_SET (room->dir_option[i]->exit_info, EX_LOCKED)
		  && room->dir_option[i]->pick_penalty < 30)
		room->dir_option[i]->pick_penalty = 30;
	    }
	}
    }

  if (!str_cmp (argument, "moon"))
    {
      global_moon_light = 1;
    }

  if (!strn_cmp (argument, "death_email", 11))
    {
      argument = one_argument (argument, buf);
      if (!*argument)
	death_email (ch);
      else
	death_email (load_pc (argument));
    }

  if (!str_cmp (argument, "saverooms"))
    save_player_rooms ();

  if (!str_cmp (argument, "pc-sold"))
    {
      //for (tch = character_list; tch; tch = tch->next)
	  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
	tch = *tch_iterator;
	  if (tch->deleted)
	    continue;

	  if (!IS_SET (tch->flags, FLAG_KEEPER) || !tch->shop)
	    continue;

	  if (!(room = vtor (tch->shop->store_vnum)))
	    continue;

	  if (!room->contents)
	    continue;

	  for (obj = room->contents; obj; obj = obj->next_content)
	    {
	      if (GET_ITEM_TYPE (obj) == ITEM_MONEY)
		continue;
	      if (!keeper_makes (tch, obj->nVirtual)
		  && !IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
		obj->obj_flags.extra_flags |= ITEM_PC_SOLD;
	    }
	}

      send_to_char ("PC-Sold flags have been reset on all objects.\n", ch);
    }

  if (!str_cmp (argument, "writing"))
    {
      for (obj = object_list; obj; obj = obj->next)
	if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT
	    || GET_ITEM_TYPE (obj) == ITEM_BOOK)
	  save_writing (obj);
    }

  if (!str_cmp (argument, "blank_ids"))
    {
      for (obj = object_list; obj; obj = obj->next)
	{
	  if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
	    obj->o.od.value[0] = 0;
	  else if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
	    obj->o.od.value[1] = 0;
	}
    }
}

MESSAGE_DATA *
load_mysql_message (char *msg_name, int board_type, int msg_number)
{
  MESSAGE_DATA *message;
  MYSQL_RES *result;
  MYSQL_ROW row;

  result = NULL;

  if (board_type == 0)
    {				// In-game board system.
      mysql_safe_query
	("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d",
	 msg_name, msg_number);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  return NULL;
	}
      if ((row = mysql_fetch_row (result)))
	{
	  message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
	  message->nVirtual = atoi (row[1]);
	  message->poster = add_hash (row[3]);
	  message->date = add_hash (row[4]);
	  message->icdate = add_hash (row[5]);
	  message->subject = add_hash (row[2]);
	  message->message = add_hash (row[6]);
	  mysql_free_result (result);
	  return message;
	}
    }
  else if (board_type == 1)
    {				// Virtual boards.
      mysql_safe_query
	("SELECT * FROM virtual_boards WHERE board_name = '%s' AND post_number = %d",
	 msg_name, msg_number);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  return NULL;
	}
      if ((row = mysql_fetch_row (result)))
	{
	  message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
	  message->nVirtual = msg_number;
	  message->poster = add_hash (row[3]);
	  message->date = add_hash (row[4]);
	  message->subject = add_hash (row[2]);
	  message->message = add_hash (row[5]);
	  message->nTimestamp = strtol (row[6], NULL, 10);
	  mysql_free_result (result);
	  return message;
	}
    }
  else if (board_type == 2)
    {				// Player notes.
      mysql_safe_query
	("SELECT * FROM player_notes WHERE name = '%s' AND post_number = %d",
	 msg_name, msg_number);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  return NULL;
	}
      if ((row = mysql_fetch_row (result)))
	{
	  message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
	  message->nVirtual = msg_number;
	  message->poster = add_hash (row[3]);
	  message->date = add_hash (row[4]);
	  message->subject = add_hash (row[2]);
	  message->message = add_hash (row[5]);
	  message->flags = atoi (row[6]);
	  mysql_free_result (result);
	  return message;
	}
    }
  else if (board_type == 3)
    {				// Player journal entries.
      mysql_safe_query
	("SELECT * FROM player_journals WHERE name = '%s' AND post_number = %d",
	 msg_name, msg_number);
      result = mysql_store_result (database);
      if (!result || !mysql_num_rows (result))
	{
	  if (result)
	    mysql_free_result (result);
	  return NULL;
	}
      if ((row = mysql_fetch_row (result)))
	{
	  message = (MESSAGE_DATA *) alloc (sizeof (MESSAGE_DATA), 1);
	  message->nVirtual = msg_number;
	  message->poster = add_hash (row[3]);
	  message->date = add_hash (row[4]);
	  message->subject = add_hash (row[2]);
	  message->message = add_hash (row[5]);
	  mysql_free_result (result);
	  return message;
	}
    }

  return NULL;
}

int
erase_mysql_board_post (CHAR_DATA * ch, char *name, int board_type,
			char *argument)
{
  MESSAGE_DATA *message;

  if (strlen (name) > 75)
    return 0;

  name[0] = toupper (name[0]);

  if (!isdigit (*argument))
    return 0;

  if (atoi (argument) < 1)
    return 0;

  if (board_type == 0)
    {
      if (!(message = load_message (name, 6, atoi (argument))))
	return 0;
      unload_message (message);
      mysql_safe_query
	("DELETE FROM boards WHERE post_number = %d AND board_name = '%s'",
	 atoi (argument), name);
    }
  else if (board_type == 1)
    {
      if (!(message = load_message (name, 5, atoi (argument))))
	return 0;
      if ((!str_cmp (name, "Bugs") || !strncasecmp(name, "Typos",5)
	   || !str_cmp (name, "Ideas") || !strncasecmp (name, "Petitions",9)
	   || !str_cmp (name, "Submissions")) && GET_TRUST (ch))
	{
	  send_to_char
	    ("Please enter what you did in response to this report:\n", ch);
	  CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	  ch->desc->pending_message->message = NULL;
	  ch->desc->pending_message->poster = add_hash (message->poster);
	  ch->desc->str = &ch->desc->pending_message->message;
	  ch->desc->max_str = MAX_STRING_LENGTH;
	  ch->desc->proc = post_track_response;
	  ch->delay_who = add_hash (name);
	  ch->delay_info1 = atoi (argument);
	  make_quiet (ch);
	  unload_message (message);
	  return 1;
	}
      if (!str_cmp (name, "Prescience") && GET_TRUST (ch))
	{
	  if (!(ch->delay_ch = load_pc (message->poster)))
	    {
	      send_to_char
		("I couldn't find the PC that left the prescience request.\n",
		 ch);
	      return 0;
	    }
	  send_to_char
	    ("Enter the dream you'd like to give in response to this prescience request:\n",
	     ch);
	  ch->desc->str = &ch->delay_who;
	  ch->desc->max_str = STR_MULTI_LINE;
	  ch->desc->proc = post_dream;
	  make_quiet (ch);
	  unload_message (message);
	}
      mysql_safe_query
	("DELETE FROM virtual_boards WHERE post_number = %d AND board_name = '%s'",
	 atoi (argument), name);
      unload_message (message);
    }
  if (board_type == 2)
    {
      if (!(message = load_message (name, 7, atoi (argument))))
	return 0;
      unload_message (message);
      mysql_safe_query
	("DELETE FROM player_notes WHERE post_number = %d AND name = '%s'",
	 atoi (argument), name);
    }
  if (board_type == 3)
    {
      if (!(message = load_message (name, 8, atoi (argument))))
	return 0;
      unload_message (message);
      mysql_safe_query
	("DELETE FROM player_journals WHERE post_number = %d AND name = '%s'",
	 atoi (argument), name);
    }

  return 1;
}

int
get_mysql_board_listing (CHAR_DATA * ch, int board_type, char *name)
{
  char query[MAX_STRING_LENGTH] = "";
  MYSQL_RES *result;
  MYSQL_ROW row;

  result = NULL;

  if (board_type == 1)
    {
      mysql_safe_query
	("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC",
	 name);
    }
  else if (board_type == 2)
    {
      std::string player_db = engine.get_config ("player_db");
      mysql_safe_query
	("SELECT CONCAT('\\'',GROUP_CONCAT(aa.name SEPARATOR '\\', \\''),'\\'') AS characters "
	 "FROM %s.pfiles aa, %s.pfiles bb "
	 "WHERE bb.name = '%s' and aa.account = bb.account "
	 "GROUP BY aa.account;", 
	 player_db.c_str (), player_db.c_str (), name);
      if ((result = mysql_store_result (database)) == NULL)
	{
	  return 0;
	}
      if (mysql_num_rows (result) && (row = mysql_fetch_row (result)) != NULL)
	{
	  sprintf (query, "SELECT name,post_number,subject,author,"
		   " FROM_UNIXTIME(timestamp,'%%%%b %%%%d %%%%Y' ) AS date "
		   "FROM player_notes "
		   "WHERE name IN ( %s ) "
		   "ORDER BY timestamp DESC, post_number DESC", row[0]);
	}
      mysql_free_result (result);
      if (!*query)
	{
	  return 0;
	}
      mysql_safe_query (query);
    }
  else if (board_type == 3)
    {
      mysql_safe_query
	("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC",
	 name);
    }

  result = mysql_store_result (database);
  if (!result || !mysql_num_rows (result))
    {
      if (result)
	mysql_free_result (result);
      return 0;
    }

  *b_buf = '\0';
  while ((row = mysql_fetch_row (result)))
    {
      if (strlen (row[2]) > 34)
	{
	  sprintf (query, "%s", row[2]);
	  query[31] = '.';
	  query[32] = '.';
	  query[33] = '.';
	  query[34] = '\0';
	}
      else
	sprintf (query, "%s", row[2]);
      if (board_type == 2)
	{
	  sprintf (b_buf + strlen (b_buf),
		   ((!strcmp (name, row[0])) ?
		    " #6%-10.10s %3d - %11s %-10.10s: %s#0\n" :
		    " %-10.10s #6%3d#0 - %11s %-10.10s: %s\n"), row[0],
		   atoi (row[1]), row[4], row[3], query);
	}
      else
	{
	  sprintf (b_buf + strlen (b_buf), " #6%3d#0 - %16s %-10.10s: %s\n",
		   atoi (row[1]), row[4], row[3], query);
	}
      if (strlen (b_buf) > B_BUF_SIZE - AVG_STRING_LENGTH)
	break;
    }

  mysql_free_result (result);

  if (!*b_buf)
    return 0;

  page_string (ch->desc, b_buf);
  return 1;
}

void
post_to_mysql_virtual_board (DESCRIPTOR_DATA * d)
{
  char *date;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int free_slot = 0;
  time_t current_time;

  if (!*d->pending_message->message)
    {
      send_to_char ("No message posted.\n", d->character);
      unload_message (d->pending_message);
      d->pending_message = NULL;
      return;
    }

  current_time = time (0);
  date = asctime (localtime (&current_time));
  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  mysql_safe_query
    ("SELECT post_number FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
     d->pending_message->poster);
  result = mysql_store_result (database);

  if (!result)
    {
      send_to_char
	("There seems to be a problem with the database listing.\n",
	 d->character);
      return;
    }

  free_slot = 1;
  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[0]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
     d->pending_message->poster, free_slot, d->pending_message->subject,
     d->character->tname, date, d->pending_message->message, (int) time (0));

  if (result)
    mysql_free_result (result);

  unload_message (d->pending_message);
  d->pending_message = NULL;
  mem_free (date);
}

void
post_to_mysql_journal (DESCRIPTOR_DATA * d)
{
  char *date;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int free_slot = 0;
  time_t current_time;

  if (!*d->pending_message->message)
    {
      send_to_char ("No message posted.\n", d->character);
      unload_message (d->pending_message);
      d->pending_message = NULL;
      return;
    }

  current_time = time (0);
  date = asctime (localtime (&current_time));
  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  mysql_safe_query
    ("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
     d->character->tname);
  result = mysql_store_result (database);

  if (!result)
    {
      send_to_char
	("There seems to be a problem with the database listing.\n",
	 d->character);
      return;
    }

  free_slot = 1;
  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[1]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO player_journals VALUES ('%s', %d, '%s', '%s', '%s', '%s')",
     d->pending_message->poster, free_slot, d->pending_message->subject,
     d->character->tname, date, d->pending_message->message);

  if (result)
    mysql_free_result (result);

  unload_message (d->pending_message);
  d->pending_message = NULL;
  mem_free (date);
}

void
post_to_mysql_player_board (DESCRIPTOR_DATA * d)
{
  char *date;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int free_slot = 0;
  time_t current_time;

  if (!*d->pending_message->message)
    {
      send_to_char ("No message posted.\n", d->character);
      unload_message (d->pending_message);
      d->pending_message = NULL;
      return;
    }

  current_time = time (0);
  date = asctime (localtime (&current_time));
  if (strlen (date) > 1)
    date[strlen (date) - 1] = '\0';

  mysql_safe_query
    ("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
     d->pending_message->poster);
  result = mysql_store_result (database);

  if (!result)
    {
      send_to_char
	("There seems to be a problem with the database listing.\n",
	 d->character);
      return;
    }

  free_slot = 1;
  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[1]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO player_notes VALUES ('%s', '%d', '%s', '%s', '%s', '%s', 0, %d)",
     d->pending_message->poster, free_slot, d->pending_message->subject,
     d->character->tname, date, d->pending_message->message, (int) time (0));

  if (result)
    mysql_free_result (result);

  unload_message (d->pending_message);
  d->pending_message = NULL;
  mem_free (date);
}

void
add_message_to_mysql_player_notes (const char *name, const char *poster,
				   MESSAGE_DATA * message)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  int free_slot = 0;

  mysql_safe_query
    ("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
     name);
  result = mysql_store_result (database);
  if (!result)
    {
      return;
    }

  free_slot = 1;
  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[1]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO player_notes VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d, %d)",
     name, free_slot, message->subject, poster, message->date,
     message->message, (int) message->flags, (int) time (0));

  if (result)
    mysql_free_result (result);
}

void
add_message_to_mysql_vboard (const char *name, const char *poster, MESSAGE_DATA * message)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  int free_slot = 0;

  mysql_safe_query
    ("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
     name);
  result = mysql_store_result (database);

  if (!result)
    {
      return;
    }

  free_slot = 1;
  if ((row = mysql_fetch_row (result)))
    {
      free_slot = atoi (row[1]);
      free_slot++;
    }

  mysql_safe_query
    ("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
     name, free_slot, message->subject, poster, message->date,
     message->message, (int) time (0));

  if (result)
    mysql_free_result (result);
}

void update_coverage_times(int admin_time, int admin_time_active, int admin_pc_time, bool admin_found, bool admin_found_absoloute)
{
int no_admin_time = 0, no_admin_time_absoloute = 0;
MYSQL_RES *result;
MYSQL_ROW row;

mysql_safe_query("SELECT * FROM admin_coverage");
result = mysql_store_result(database);
int week_num = mysql_num_rows(result) - 1; 

if (!admin_found)
  no_admin_time = 1;
if (!admin_found_absoloute)
  no_admin_time_absoloute = 1;

mysql_safe_query("UPDATE admin_coverage SET coverage_time=coverage_time+%d, coverage_time_absoloute=coverage_time_absoloute+%d, admin_pc_time=admin_pc_time+%d, no_admin_time=no_admin_time+%d, no_admin_time_absoloute=no_admin_time_absoloute+%d WHERE week_num=%d", admin_time, admin_time_active, admin_pc_time, no_admin_time, no_admin_time_absoloute, week_num); 
}


void load_vnpc_timestamp()
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  
  mysql_safe_query
    ("SELECT * FROM shopdata LIMIT 1");

  result = mysql_store_result (database);

  if (!result)
    {
		/* if no result, set to now */
		last_vnpc_sale = time(0);
		return;
    }

  /* get one row */
  if ((row = mysql_fetch_row (result)))
    {
		last_vnpc_sale = atoi(row[0]);
    }
 
  if (result)
    mysql_free_result (result);
}

void save_vnpc_timestamp()
{
	mysql_safe_query("UPDATE shopdata SET last_vnpc_timestamp=%d",last_vnpc_sale);
}

int getCharNum( CHAR_DATA * ch, bool isMob )
{
	if( !isMob )
	{
		mysql_safe_query("SELECT char_num FROM %s.pfiles WHERE name='%s'", engine.get_config("player_db").c_str(), ch->tname);
		MYSQL_RES *result = mysql_store_result( database );
		if (!result || !mysql_num_rows( result ))
		{
			if (result != NULL)
				mysql_free_result( result );
			return 0;
		}
		MYSQL_ROW row = mysql_fetch_row( result );
		mysql_free_result( result );
		return atoi(row[0]);
	}
	else
		return ch->mob->nVirtual;
}

void load_skills_mysql(CHAR_DATA * ch, bool isMob)
{
	int c_num = getCharNum(ch, isMob);
	std::string player_db = engine.get_config ("player_db");
	MYSQL_RES* result;
	MYSQL_ROW row;
	if( !ch->name )
		return;
	mysql_safe_query( "SELECT skill_num, skill_value FROM %s.skill_values WHERE char_num=%d and is_mob=%d", player_db.c_str(), c_num, isMob);
	result = mysql_store_result( database );
	if (!result || !mysql_num_rows( result ))
	{
		if (result != NULL)
			mysql_free_result( result );
		return;
	}
	for(row = mysql_fetch_row( result ); row; row = mysql_fetch_row( result ))
	{
		ch->skills[atoi(row[1])] = atoi(row[2]);
		if(!isMob)
			ch->pc->skills[atoi(row[1])] = ch->skills[atoi(row[1])];
		row = mysql_fetch_row (result);
	}
	mysql_free_result( result );
}

void save_skills_mysql( CHAR_DATA * ch, bool isMob )
{
	std::string player_db = engine.get_config ("player_db");
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int c_num = getCharNum(ch, isMob), curr_skill = 0;

	for( int i = 0; i < LAST_SKILL; i++ )
		if( ch->skills[i] )
		{
			mysql_safe_query("SELECT skill_value FROM %s.skill_values WHERE char_num=%d AND skill_num=%d AND is_mob=%d", player_db.c_str(), c_num, i, isMob);
			result = mysql_store_result( database );
			if( mysql_num_rows( result ) != 0 )
			{
				row = mysql_fetch_row( result );
				curr_skill = atoi( row[0] );
				if( curr_skill != ch->skills[i] )
					mysql_safe_query("UPDATE %s.skill_values SET skill_value=%d WHERE char_num=%d AND skill_num=%d AND is_mob=%d", player_db.c_str(), ch->skills[i], c_num, i, isMob);
			}
			else
				mysql_safe_query("INSERT INTO %s.skill_values SET char_num=%d, skill_num=%d, skill_value=%d, is_mob=%d", player_db.c_str(), c_num, i, ch->skills[i], isMob);
			mysql_free_result( result );
		}
}
