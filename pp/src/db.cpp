/*------------------------------------------------------------------------\
|  db.c : Database Module                             www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mysql/mysql.h>
#include <signal.h>
#include <stdexcept>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "clan.h"		/* clan__assert_objs() */

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

extern rpie::server engine;

ROOM_DATA * globalRoomArray[110000] = {NULL};
OBJ_DATA * globalObjectArray[110000] = {NULL}; // Arrays to handle rooms/object prototypes so lookup is constant time - Case

int mob_start_stat = 12;	/* Different on Koldryn's port */


//ROOM_DATA *wld_tab[ZONE_SIZE]; // I've tried to remove reliance on this - Case
OBJ_DATA *obj_tab[OBJECT_ZONE_SIZE]; // Expanded for the 110k globalObjectArray. Top 10k objects are object categories - Case
CHAR_DATA *mob_tab[ZONE_SIZE];

ROLE_DATA *role_list = NULL;
OBJ_DATA *object_list = NULL;
//CHAR_DATA *character_list = NULL;
std::list<char_data*> character_list;
std::multimap<int, room_prog> mob_prog_list;
std::multimap<int, room_prog> obj_prog_list;
CHAR_DATA *full_mobile_list = NULL;
ROOM_DATA *full_room_list = NULL;
OBJ_DATA *full_object_list = NULL;
NEWBIE_HINT *hint_list = NULL;
HELP_DATA *help_list = NULL;
HELP_DATA *bhelp_list = NULL;
TEXT_DATA *text_list = NULL;
TEXT_DATA *document_list = NULL;
SUBCRAFT_HEAD_DATA *crafts = NULL;
SITE_INFO *banned_site;

int booting = 1;

/* do_hour messes up time, so we keep track of how many times do_hour
was used, so we can make time adjustments. */

int times_do_hour_called = 0;
int next_mob_coldload_id = 0;
int next_pc_coldload_id = 0;	/* get_next_pc_coldload_id () */
int next_obj_coldload_id = 0;

int count_max_online = 0;
char max_online_date [AVG_STRING_LENGTH] = "";
int MAX_ZONE = 100;
int OBJECT_MAX_ZONE = 110;
int second_affect_active = 0;
int check_objects;
int check_characters;
long mud_time;
char *mud_time_str = NULL;

int mp_dirty = 0;		/* 1 if mob programs need to be written out */
time_t next_hour_update;	/* Mud hour pulse (15 min) */
time_t next_minute_update;	/* RL minute (1 min) */

int world_version_in = 12;
int world_version_out = 12;
int pulse_violence = 8;
REGISTRY_DATA *registry[MAX_REGISTRY];
struct zone_data *zone_table;
int advance_hour_now = 0;
char BOOT[AVG_STRING_LENGTH];

FILE *fp_log;
FILE *imm_log;
FILE *guest_log;
FILE *sys_log;

struct time_info_data time_info;

/* local procedures */
void boot_zones (void);
void setup_dir (FILE * fl, ROOM_DATA * room, int dir, int type);
ROOM_DATA *allocate_room (int nVirtual);
char *file_to_string (char *name);
void reset_time (void);
void reload_hints (void);
int change;
bool memory_check = false;

/* external refs */
extern struct descriptor_data *descriptor_list;
extern struct msg_data *msg_list;
void boot_mess (void);
void load_messages (void);
void boot_social_messages (void);

// Enhanced vToR - Case '09
/* moved to protos.h -  ROOM_DATA * vtor(int characterRVirtual) {
static ROOM_DATA *room = NULL;

if (characterRVirtual < 100000) {
room = globalRoomArray[characterRVirtual];

if ((room != NULL) && room->nVirtual == characterRVirtual) {
return (room);
}
}

for (room = wld_tab[characterRVirtual % ZONE_SIZE]; room; room = room->hnext) {
if (room->nVirtual == characterRVirtual) {
globalRoomArray[characterRVirtual] = (room);
return (room);
}
}

return NULL;
}*/

//ROOM_DATA *
//vtor (int nVirtual)
//{
//  static ROOM_DATA *room = NULL;
//  static int shortcuts = 0;
//  static int longways = 0;
//  static int failures = 0;
//
//  /* Short cut...caller might want last used room */
//
//  if (room && room->nVirtual == nVirtual)
//    {
//      shortcuts++;
//      return room;
//    }
//
//  if (nVirtual < 0)
//    return NULL;
//
//  for (room = wld_tab[nVirtual % ZONE_SIZE]; room; room = room->hnext)
//    if (room->nVirtual == nVirtual)
//      {
//	longways++;
//	return (room);
//      }
//
//  failures++;
//
//  return NULL;
//}


//ROOM_DATA* loadRoom (int vnum)
//{
//	ROOM_DATA* room = 0;
//	for (room = wld_tab[vnum % ZONE_SIZE]; room; room = room->hnext) {
//		if (room->nVirtual == vnum) {
//			globalRoomArray[vnum] = room;
//			return room;
//		}
//	}
//	return NULL;
//}



void
add_room_to_hash (ROOM_DATA * add_room)
{
	int hash;
	ROOM_DATA *troom;
	static ROOM_DATA *last_room = NULL;

	add_room->lnext = NULL;

	if (booting && last_room)
		last_room->lnext = add_room;

	else if (!full_room_list)
		full_room_list = add_room;

	else
	{
		troom = full_room_list;

		while (troom->lnext)
			troom = troom->lnext;

		troom->lnext = add_room;
	}

	last_room = add_room;

	if (globalRoomArray[add_room->nVirtual] == NULL) {
		globalRoomArray[add_room->nVirtual] = add_room;
	}
	//hash = add_room->nVirtual % ZONE_SIZE;

	//add_room->hnext = wld_tab[hash];
	//wld_tab[hash] = add_room;
}

CHAR_DATA *
vtom (int nVirtual)
{
	CHAR_DATA *mob;

	if (nVirtual < 0)
		return NULL;

	for (mob = mob_tab[nVirtual % ZONE_SIZE]; mob; mob = mob->mob->hnext)
		if (mob->mob->nVirtual == nVirtual)
			return (mob);

	return NULL;
}

void
add_mob_to_hash (CHAR_DATA * add_mob)
{
	int hash;
	CHAR_DATA *tmob;
	static CHAR_DATA *last_mob = NULL;

	add_mob->mob->lnext = NULL;

	if (booting && last_mob)
		last_mob->mob->lnext = add_mob;

	else if (!full_mobile_list)
		full_mobile_list = add_mob;

	else
	{
		tmob = full_mobile_list;

		while (tmob->mob->lnext)
			tmob = tmob->mob->lnext;

		tmob->mob->lnext = add_mob;
	}

	last_mob = add_mob;

	hash = add_mob->mob->nVirtual % ZONE_SIZE;

	add_mob->mob->hnext = mob_tab[hash];
	mob_tab[hash] = add_mob;
}

// Inline version in room.h - Case
//OBJ_DATA *
//vtoo (int nVirtual)
//{
//	OBJ_DATA *obj;
//
//	if (nVirtual < 0)
//		return NULL;
//
//	for (obj = obj_tab[nVirtual % OBJECT_ZONE_SIZE]; obj; obj = obj->hnext)
//		if (obj->nVirtual == nVirtual)
//			return (obj);
//
//	return NULL;
//}

void
add_obj_to_hash (OBJ_DATA * add_obj)
{
	int hash;
	OBJ_DATA *tobj;
	static OBJ_DATA *last_obj = NULL;

	add_obj->lnext = NULL;

	if (booting && last_obj)
		last_obj->lnext = add_obj;

	else if (!full_object_list)
		full_object_list = add_obj;

	else
	{
		tobj = full_object_list;

		while (tobj->lnext)
			tobj = tobj->lnext;

		tobj->lnext = add_obj;
	}

	last_obj = add_obj;

	if (globalObjectArray[add_obj->nVirtual] == NULL) {
		globalObjectArray[add_obj->nVirtual] = add_obj;

		hash = add_obj->nVirtual % OBJECT_ZONE_SIZE;
		add_obj->hnext = obj_tab[hash];
		obj_tab[hash] = add_obj;
	}
}

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void
boot_db (void)
{
	int i;
	char buf[MAX_STRING_LENGTH];

	/* get the name of the person who rebooted */

	mm ("in boot_db");

	mud_time = time (NULL);

	mob_start_stat = 16;

	system_log ("Beginning database initialization.", false);

	system_log ("Initialising registry.", false);
	setup_registry ();

	mm ("post setup_registry");

	system_log ("Loading dynamic registry.", false);
	load_dynamic_registry ();

	mm ("post dynamic_registry");

	system_log ("Reloading banned site list.", false);
	reload_sitebans ();

	system_log ("Loading special chargen roles.", false);
	reload_roles ();

	system_log ("Loading newbie hints.", false);
	reload_hints ();

	system_log ("Reading external text files.", false);
	add_text (&text_list, WIZLIST_FILE, "wizlist");
	add_text (&text_list, GREET_FILE, "greetings");
	add_text (&text_list, MAINTENANCE_FILE, "greetings.maintenance");
	add_text (&text_list, MENU1_FILE, "menu1");
	add_text (&text_list, QSTAT_FILE, "qstat_message");
	add_text (&text_list, QSTAT_HELP, "qstat_help");
	add_text (&text_list, ACCT_APP_FILE, "account_application");
	add_text (&text_list, ACCT_REF_FILE, "account_referral");
	add_text (&text_list, ACCT_EMAIL_FILE, "account_email");
	add_text (&text_list, ACCT_POLICIES_FILE, "account_policies");
	add_text (&text_list, THANK_YOU_FILE, "thankyou");
	add_text (&text_list, CMENU_FILE, "cmenu");
	add_text (&text_list, NEW_MENU, "new_menu");
	add_text (&text_list, NEW_CHAR_INTRO, "new_char_intro");
	add_text (&text_list, RACE_SELECT, "race_select");
	add_text (&text_list, ROLE_SELECT, "role_select");
	add_text (&text_list, SPECIAL_ROLE_SELECT, "special_role_select");
	add_text (&text_list, SEX_SELECT, "sex_select");
	add_text (&text_list, AGE_SELECT, "age_select");
	add_text (&text_list, PLDESC_FILE, "help_pldesc");
	add_text (&text_list, PSDESC_FILE, "help_psdesc");
	add_text (&text_list, PDESC_FILE, "help_pdesc");
	add_text (&text_list, NAME_FILE, "help_name");
	add_text (&text_list, PKEYWORDS_FILE, "help_pkeywords");
	add_text (&text_list, HEIGHT_FRAME, "height_frame");
	add_text (&text_list, LOCATION, "location");
	add_text (&text_list, COMMENT_HELP, "comment_help");
	add_text (&text_list, SKILL_SELECT, "skill_select");
	add_text (&text_list, PROFESSION_SELECT, "professions");

	mm ("post file_to_string of files");

	system_log ("Reading race table.", false);
	load_race_table ();

	system_log ("Loading zone table.", false);
	boot_zones ();

	system_log ("Initializing dynamic weather zones.", false);
	initialize_weather_zones ();

	mm ("post boot_zones");

	system_log ("Resetting the game time:", false);
	reset_time ();

	mm ("post reset_time");

	system_log ("Loading rooms.", false);
	load_rooms ();

	mm ("post load_rooms");

	system_log("Loading room affects.", false);
	boot_zones_raffects();
	
	mm ("post boot_zones_raffects");
	
	system_log ("Loading auto-generated dwellings.", false);
	load_dwelling_rooms ();

	system_log ("Generating hash table for mobile files.", false);
	boot_mobiles ();

	mm ("post boot_mobiles");

	system_log ("Generating hash table for object files.", false);
	boot_objects ();

	mm ("post boot_objects");

	system_log ("Loading craft information.", false);
	boot_crafts ();

	mm ("post boot_crafts");

	system_log ("Load mobile triggers (mobprogs).", false);
	boot_mobprogs ();

	mm ("post boot_mobprogs");

	system_log ("Loading social messages.", false);
	boot_social_messages ();

	mm ("post boot_social_messages");

	system_log ("Reading online record.", false);
	load_online_stats ();

	system_log ("Reloading persistent tracks.", false);
	load_tracks ();

	if (engine.in_play_mode ())
	{
		system_log ("Loading persistent mobiles...", false);
		load_stayput_mobiles ();
		system_log ("Stocking any new deliveries set since last reboot...",
			false);
		stock_new_deliveries ();
	}

	mm ("post load_save_rooms");

	load_mob_progs();

	mm ("post load mob progs");

	load_obj_progs();

	mm ("post load object progs");

	//free_mem (overhead_base);

	mm ("freed overhead_base");


	for (i = 0; i < MAX_ZONE; i++)
	{
		if (!zone_table[i].cmd)
			continue;

		if (!engine.in_build_mode ())
		{
			zone_table[i].flags &= ~Z_FROZEN;
		}

		if (strncmp (zone_table[i].name, "Empty Zone", 10))
		{
			sprintf (buf, "Performing boot-time reset of %s: %d.",
				zone_table[i].name, i);
			system_log (buf, false);
		}

		reset_zone (i);

		sprintf (buf, "loaded zone %d", i);
		mm (buf);
	}

	// load timestamp of last sale from vnpc database
	load_vnpc_timestamp();

	initialize_materials();
	initialize_location_map();

	//print_mem_stats (NULL);

	booting = 0;

	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		CHAR_DATA *ch = *tch_iterator;
		if (IS_NPC(ch) && ch->mob->cues)
		{
			typedef std::multimap<mob_cue,std::string>::const_iterator N;
			std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_reboot);
			for (N n = range.first; n != range.second; n++)
			{
				std::string cue = n->second;
				if (!cue.empty())
				{
					command_interpreter(ch, (char *) cue.c_str());
				}
			}
		}
	}

	system_log ("Boot db -- DONE.", false);
}

void
reload_hints (void)
{
	NEWBIE_HINT *hint, *thint;
	FILE *fp;
	char *string;
	char buf[MAX_STRING_LENGTH];

	hint_list = NULL;

	if (!(fp = fopen ("text/hints", "r")))
		return;

	fgets (buf, 256, fp);

	while (1)
	{
		string = fread_string (fp);
		if (!string || !*string)
			break;
		hint = new NEWBIE_HINT;
		hint->hint = string;
		hint->next = NULL;
		if (hint_list == NULL)
			hint_list = hint;
		else
		{
			thint = hint_list;
			while (thint->next) {
				thint = thint->next;
			}
			thint->next = hint;
		}
	}

	fclose (fp);
}

void
stock_new_deliveries (void)
{
	CHAR_DATA *tch;
	OBJ_DATA *obj;
	ROOM_DATA *room;
	int i = 0;

	if (!engine.in_play_mode ())
	{
		return;
	}

	//for (tch = character_list; tch; tch = tch->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted)
			continue;
		if (!IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER) || !tch->shop)
			continue;
		room = vtor (tch->shop->store_vnum);
		if (!room)
			continue;
		if (!room->psave_loaded)
			load_save_room (room);
		for (i = 0; i < MAX_TRADES_IN; i++)
		{
			if (!get_obj_in_list_num (tch->shop->delivery[i], room->contents))
			{
				obj = load_object (tch->shop->delivery[i]);
				obj_to_room (obj, room->nVirtual);
			}
		}
	}

	system_log ("New shopkeeper deliveries stocked.", false);
}

void
reset_time (void)
{
	char buf[MAX_STRING_LENGTH];

	long beginning_of_time = GAME_SECONDS_BEGINNING;

	int qz[] = { 6, 6, 6, 6, 5, 5, 4, 4, 5, 6, 6, 7 };
	int moonrise, sunrise, moonset, t1, t2, t3;

	int i = 0;

	struct time_info_data mud_time_passed (time_t t2, time_t t1);

	next_hour_update = time (0) + ((time (0) - beginning_of_time) % 900);
	next_minute_update = time (0);

	time_info = mud_time_passed (time (0), beginning_of_time);

	sunrise = qz[(int) time_info.month];	/* sunrise is easy */
	if (time_info.day <= 15)
		t1 = 15 - time_info.day;
	else
		t1 = time_info.day - 15;
	/* t1 is 0 at new moon when moon is up for day length and 15 at
	full moon when moon is up for night length */
	t3 = sunrise << 1;		/* because it doesn't work otherwise! :( */
	t2 = ((t3 + 1) * (15 - t1)) + ((23 - t3) * t1);
	/* t2 = time between moonrise and moonset * 15 */
	moonrise = (345 + time_info.day * 24 - t2) / 30;
	moonset = (345 + time_info.day * 24 + t2) / 30;
	if (moonrise >= 24)
	{
		moonrise -= 24;
		moonset -= 24;
	}				/* moonset is allowed to be >= 24 if the moon is out at midnight */

	for (i = 0; i <= 99; i++)
	{
		if (time_info.hour == sunrise)
			weather_info[i].sunlight = SUN_RISE;
		else if (time_info.hour == (23 - t3))
			weather_info[i].sunlight = SUN_SET;
		else if (time_info.hour > sunrise && time_info.hour < (23 - t3))
			weather_info[i].sunlight = SUN_LIGHT;
		else if (time_info.hour >= moonrise && time_info.hour < moonset)
			weather_info[i].sunlight = MOON_RISE;
		else if (time_info.hour < (moonset % 24) && moonset >= 24)
			weather_info[i].sunlight = MOON_RISE;
		else
			weather_info[i].sunlight = SUN_DARK;
	}

	sprintf (buf, "   Current Gametime: %dH %dD %dM %dY.",
		time_info.hour, time_info.day, time_info.month, time_info.year);
	system_log (buf, false);

	if (time_info.month == 0 || time_info.month == 1 || time_info.month == 11)
		time_info.season = WINTER;
	else if (time_info.month < 11 && time_info.month > 7)
		time_info.season = AUTUMN;
	else if (time_info.month < 8 && time_info.month > 4)
		time_info.season = SUMMER;
	else
		time_info.season = SPRING;

	for (i = 0; i <= 99; i++)
	{
		if (Weather::weather_unification (i))
			continue;

		weather_info[i].trend = number (0, 15);
		weather_info[i].clouds = number (0, 3);
		if (time_info.season == SUMMER)
			weather_info[i].clouds = number (0, 1);
		if (zone_table[i].weather_type == WEATHER_DESERT)
			weather_info[i].clouds = 0;
		weather_info[i].fog = 0;
		if (weather_info[i].clouds > 0 && zone_table[i].weather_type != WEATHER_DESERT)
			weather_info[i].state = number (0, 1);
		else if (zone_table[i].weather_type == WEATHER_DESERT)
			weather_info[i].state = NO_RAIN;
		weather_info[i].temperature =
			seasonal_temp[zone_table[i].weather_type][time_info.month];
		weather_info[i].wind_speed = number (0, 2);
		if (time_info.hour >= sunrise
			&& time_info.hour < sunset[time_info.month])
		{
			sun_light = 1;
			weather_info[i].temperature += 15;
		}
		else
			weather_info[i].temperature -= 15;
	}

	time_info.holiday = 0;
}

void
create_room_zero (void)
{
	ROOM_DATA *room;

	room = allocate_room (0);
	room->zone = 0;
	room->name = duplicateString ("Heaven");
	room->description = duplicateString ("You are in heaven.\n");

	if (str_cmp (zone_table[0].name, "Empty Zone"))
		return;

	zone_table[0].name = duplicateString ("Heaven");
	zone_table[0].top = 0;
	zone_table[0].lifespan = 0;
	zone_table[0].reset_mode = 0;
	zone_table[0].flags = 0;
	zone_table[0].jailer = 0;
	zone_table[0].jail_room_num = 0;
	zone_table[0].weather_type = 0;

	zone_table[0].flags |= Z_FROZEN;
}

void
load_rooms (void)
{
	FILE *fl;
	int zon, flag = 0, tmp, sdir;
	int virtual_nr;
	char *temp, chk[50], errbuf[80], wfile[80];
	struct extra_descr_data *new_descr;
	struct written_descr_data *w_desc;
	struct room_prog *r_prog;
	struct secret *r_secret;
	struct stat fstatus;
	ROOM_DATA *room;
	AFFECTED_TYPE *af;
	int i;

	for (i = 0; i < ZONE_SIZE; i++)
	{
		//wld_tab[i] = NULL;
		mob_tab[i] = NULL;
	}

	for (i = 0; i < OBJECT_ZONE_SIZE; i++) {
		obj_tab[i] = NULL;
	}

	for (zon = 0; zon < MAX_ZONE; zon++)
	{

		sprintf (wfile, "%s/rooms.%d", REGIONS, zon);

		if (stat (wfile, &fstatus))
		{
			sprintf (errbuf, "Zone %d rooms did not load. Aborting.", zon);
			system_log (errbuf, true);
			abort ();
		}

		if (fstatus.st_size == 0)
		{
			sprintf (s_buf, "ERROR: Corrupt %d.wld :: aborting.", zon);
			system_log (s_buf, true);
			abort ();
		}

		if ((fl = fopen (wfile, "r")) == NULL)
		{
			sprintf (errbuf, "Could not load rooms.%d. Aborting.", zon);
			system_log (errbuf, true);
			abort ();
		}

		do
		{
			fscanf (fl, " #%d\n", &virtual_nr);
			temp = fread_string (fl);
			if (!temp)
				continue;		/* KILLER CDR */

			if ((flag = (*temp != '$')))
			{
				room = allocate_room (virtual_nr);
				room->zone = zon;
				room->name = temp;
				room->description = fread_string (fl);
				fscanf (fl, "%d", &tmp);

				fscanf (fl, " %d ", &tmp);
				room->room_flags = tmp;

				/* The STORAGE bit is set when loading in shop keepers */

				room->room_flags &= ~(STORAGE | PC_ENTERED);
				room->room_flags |= SAVE;

				fscanf (fl, " %d ", &tmp);
				room->sector_type = tmp;

				fscanf (fl, "%d\n", &tmp);
				room->deity = tmp;

				room->contents = 0;
				room->people = 0;
				room->capacity = 0;
				room->light = 0;

				for (tmp = 0; tmp <= LAST_DIR; tmp++)
					room->dir_option[tmp] = 0;

				room->ex_description = 0;
				room->wdesc = 0;
				room->prg = 0;
				for (tmp = 0; tmp <= LAST_DIR; tmp++)
					room->secrets[tmp] = 0;

				for (;;)
				{
					fscanf (fl, " %s \n", chk);

					if (*chk == 'D')	/* direction field */
						setup_dir (fl, room, atoi (chk + 1), 0);

					else if (*chk == 'H')	/* Secret (hidden) */
						setup_dir (fl, room, atoi (chk + 1), 1);

					else if (*chk == 'T')	/* Trapped door */
						setup_dir (fl, room, atoi (chk + 1), 2);

					else if (*chk == 'B')	/* Trapped hidden door */
						setup_dir (fl, room, atoi (chk + 1), 3);

					else if (*chk == 'Q')
					{		/* Secret search desc */
						r_secret = new secret;
						sdir = atoi (chk + 1);
						fscanf (fl, "%d\n", &tmp);
						r_secret->diff = tmp;
						r_secret->stext = fread_string (fl);
						room->secrets[sdir] = r_secret;
					}

					else if (*chk == 'U')
						room->noInvLimit = 1;

					else if (*chk == 'E')	/* extra description field */
					{
						struct extra_descr_data *tmp_extra;

						new_descr = new extra_descr_data;
						new_descr->next = NULL;
						new_descr->keyword = fread_string (fl);
						new_descr->description = fread_string (fl);

						if (!room->ex_description)
							room->ex_description = new_descr;
						else
						{
							tmp_extra = room->ex_description;

							while (tmp_extra->next)
								tmp_extra = tmp_extra->next;

							tmp_extra->next = new_descr;
						}
					}

					else if (*chk == 'W')
					{
						w_desc = new written_descr_data;
						fscanf (fl, "%d\n", &tmp);
						w_desc->language = tmp;
						w_desc->description = fread_string (fl);
						room->wdesc = w_desc;

					}
					else if (*chk == 'P')
					{
						struct room_prog *tmp_prg;
						r_prog = new room_prog;
						r_prog->next = NULL;
						r_prog->command = fread_string (fl);
						r_prog->keys = fread_string (fl);
						r_prog->prog = fread_string (fl);

						/* Make sure that the room program is stored at
						end of the list.  This way when the room is
						saved, the rprogs get saved in the same order
						- Rassilon
						*/

						if (!room->prg)
							room->prg = r_prog;
						else
						{
							tmp_prg = room->prg;

							while (tmp_prg->next)
								tmp_prg = tmp_prg->next;

							tmp_prg->next = r_prog;
						}
					}

					else if (*chk == 'A')
					{		/* Additional descriptions */

						room->extra = new ROOM_EXTRA_DATA;

						for (i = 0; i < WR_DESCRIPTIONS; i++)
						{
							room->extra->weather_desc[i] = fread_string (fl);
							if (!strlen (room->extra->weather_desc[i]))
								room->extra->weather_desc[i] = NULL;
						}

						for (i = 0; i <= 5; i++)
						{
							room->extra->alas[i] = fread_string (fl);
							if (!strlen (room->extra->alas[i]))
								room->extra->alas[i] = NULL;
						}
					}//else if (*chk == 'A')

					else if (*chk == 'F')
					{ //room aFfects
						af = new AFFECTED_TYPE;
						fscanf (fl, "%d %d %d\n",
								&af->type, &af->a.room.duration,
								&af->a.room.intensity);						
						if (room->affects)
							af->next = room->affects;
						else
							af->next = NULL;
						room->affects = af;
					}
					
					else if (*chk == 'C')
					{	/* Capacity of room */
						fscanf (fl, "%d\n", &tmp);
						room->capacity = tmp;

					}
					else if (*chk == 'S')	/* end of current room */
						break;
				}
			}
		}
		while (flag);
		fclose (fl);
	}

	for (i = 0; i < MAX_ZONE; i++)
		if (zone_table[i].jail_room_num)
			zone_table[i].jail_room = vtor (zone_table[i].jail_room_num);

	if (!vtor (0))
		create_room_zero ();
}

#define MAX_PREALLOC_ROOMS		14000

ROOM_DATA *
get_room (void) {
	ROOM_DATA *room;
	room = new ROOM_DATA;
	room->lnext = NULL;
	room->hnext = NULL;
	room->affects = NULL;
	room->tracks = NULL;
	room->contents = NULL;
	room->description = NULL;
	room->people = NULL;
	room->prg = NULL;
	room->extra = NULL;

	return room;
}

ROOM_DATA *
allocate_room (int nVirtual)
{
	ROOM_DATA *new_room;
	char buf[MAX_STRING_LENGTH];

	new_room = get_room ();

	new_room->nVirtual = nVirtual;
	new_room->psave_loaded = 0;

#define CHECK_DOUBLE_DEFS 0
#ifdef CHECK_DOUBLE_DEFS

	if (vtor (nVirtual))
	{
		sprintf (buf, "Room %d multiply defined!!", nVirtual);
		system_log (buf, true);
	}
	else
#endif
		add_room_to_hash (new_room);
	if (new_room->nVirtual < 110000) {
		globalRoomArray[new_room->nVirtual] = (new_room);
	}

	return new_room;
}

void
setup_dir (FILE * fl, ROOM_DATA * room, int dir, int type)
{
	int tmp2;

	room->dir_option[dir] = new room_direction_data;
	room->dir_option[dir]->general_description = fread_string (fl);
	room->dir_option[dir]->keyword = fread_string (fl);

	fscanf (fl, " %d ", &tmp2);
	if (tmp2 == 3)
		room->dir_option[dir]->exit_info = EX_ISGATE;
	else if (tmp2 == 1)
		room->dir_option[dir]->exit_info = EX_ISDOOR;
	else if (tmp2 == 2)
		room->dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
	else
		room->dir_option[dir]->exit_info = 0;

	fscanf (fl, " %d ", &room->dir_option[dir]->key);

	if (world_version_in >= 1)
		fscanf (fl, " %d ", &room->dir_option[dir]->pick_penalty);

	fscanf (fl, " %d", &room->dir_option[dir]->to_room);

	switch (type)
	{
	case 1:
		room->dir_option[dir]->exit_info |= EX_SECRET;
		break;
	case 2:
		room->dir_option[dir]->exit_info |= EX_TRAP;
		break;
	case 3:
		room->dir_option[dir]->exit_info |= (EX_SECRET | EX_TRAP);
		break;
	}
}

/* load the zone table and command tables */

void
boot_zones (void)
{
	FILE *fl;
	int zon;
	int cmd_no;
	int tmp;
	int jail_room_num;
	char c;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char zfile[MAX_STRING_LENGTH];
	char rafile[MAX_STRING_LENGTH];
	struct stat fstatus;
	RESET_AFFECT *ra;

	zone_table = new zone_data[MAX_ZONE];

	for (zon = 0; zon < MAX_ZONE; zon++)
	{

		sprintf (zfile, "%s/resets.%d", REGIONS, zon);

		if (stat (zfile, &fstatus))
		{
			sprintf (buf, "Zone %d resets did not load.", zon);
			system_log (buf, true);
			continue;
		}

		if ((fl = fopen (zfile, "r")) == NULL)
		{
			system_log (zfile, true);
			perror ("boot_zone");
			break;
		}

		
		fscanf (fl, " #%*d\n");
		fread_string (fl);

		fgets (buf, 80, fl);	/* Zone number header stuff */
		fgets (buf, 80, fl);

		for (cmd_no = 1;;)
		{

			fscanf (fl, " ");
			fscanf (fl, "%c", &c);

			if (c == 'S')
				break;

			if (c != '*')
				cmd_no++;

			fgets (buf, 80, fl);
		}

		zone_table[zon].cmd = new reset_com[cmd_no];

		zone_table[zon].weather_type = 0;

		rewind (fl);

		cmd_no = 0;

		fscanf (fl, " #%*d\nLead: ");
		zone_table[zon].lead = fread_string (fl);
		zone_table[zon].name = fread_string (fl);

		jail_room_num = 0;

		fgets (buf, 80, fl);
		fgets (buf, 80, fl);

		/* Note:  At this point, no rooms have been read in yet */

		sscanf (buf, " %d %d %d %lu %d %d",
			&zone_table[zon].top,
			&zone_table[zon].lifespan,
			&zone_table[zon].reset_mode,
			&zone_table[zon].flags,
			&zone_table[zon].jailer, &zone_table[zon].jail_room_num);

		zone_table[zon].flags |= Z_FROZEN;

		/* read the command table */

		cmd_no = 0;

		for (;;)
		{

			fscanf (fl, " ");	/* skip blanks */
			fscanf (fl, "%c", &zone_table[zon].cmd[cmd_no].command);

			if (zone_table[zon].cmd[cmd_no].command == 'C')
			{

				fgets (buf, 80, fl);

				while (*buf && isspace (buf[strlen (buf) - 1]))
					buf[strlen (buf) - 1] = '\0';

				zone_table[zon].cmd[cmd_no].arg1 = (long int) duplicateString (buf);

				cmd_no++;

				continue;
			}

			if (zone_table[zon].cmd[cmd_no].command == 'R')
			{

				fscanf (fl, "%d %d %d %d %d %d %d %d %d %d",
					&zone_table[zon].cmd[cmd_no].arg1,
					&tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp);

				fgets (buf, 80, fl);

				cmd_no++;

				continue;
			}

			if (zone_table[zon].cmd[cmd_no].command == 'm')
			{

				fscanf (fl, "%d", &zone_table[zon].cmd[cmd_no].arg1);

				if (zone_table[zon].cmd[cmd_no].arg1 == RESET_REPLY)
				{
					fgets (buf, MAX_STRING_LENGTH - 100, fl);
					buf[strlen (buf) - 1] = '\0';	/* Remove new line */

					p = buf;
					while (isspace (*p))
						p++;

					zone_table[zon].cmd[cmd_no].arg2 = (long int) duplicateString (p);
				}

				else
				{
					system_log ("UNKNOWN m type RESET, ignored.", true);
					fgets (buf, 80, fl);
				}

				cmd_no++;

				continue;
			}

			if (zone_table[zon].cmd[cmd_no].command == 'A')
			{

				ra = new RESET_AFFECT;

				fscanf (fl, "%d %d %d %d %d %d %d",
					&ra->type,
					&ra->duration,
					&ra->modifier,
					&ra->location, &ra->bitvector, &ra->sn, &ra->t);

 // putting ra into an in will most certainly create
 //				a migration problem.  A migrator should create a
 //				new element in zone_table for ra, and not use arg1 

				zone_table[zon].cmd[cmd_no].arg1 = (long int) ra;

				fgets (buf, 80, fl);

				cmd_no++;

				continue;
			}

			if (zone_table[zon].cmd[cmd_no].command == 'S')
				break;

			if (zone_table[zon].cmd[cmd_no].command == '*')
			{
				fgets (buf, 80, fl);	/* skip command */
				continue;
			}

			fscanf (fl, " %d %d %d",
				&tmp,
				&zone_table[zon].cmd[cmd_no].arg1,
				&zone_table[zon].cmd[cmd_no].arg2);

			if (zone_table[zon].cmd[cmd_no].command == 'G' &&
				zone_table[zon].cmd[cmd_no].arg1 == VNUM_MONEY)
				zone_table[zon].cmd[cmd_no].arg1 = VNUM_PENNY;

			zone_table[zon].cmd[cmd_no].if_flag = tmp;

			if (zone_table[zon].cmd[cmd_no].command == 'M' ||
				zone_table[zon].cmd[cmd_no].command == 'O' ||
				zone_table[zon].cmd[cmd_no].command == 'E' ||
				zone_table[zon].cmd[cmd_no].command == 'P' ||
				zone_table[zon].cmd[cmd_no].command == 'a' ||
				zone_table[zon].cmd[cmd_no].command == 'D')
				fscanf (fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

			if (zone_table[zon].cmd[cmd_no].command == 'M')
			{
				zone_table[zon].cmd[cmd_no].enabled = 1;
				fscanf (fl, " %d", &zone_table[zon].cmd[cmd_no].arg4);
			}

			fgets (buf, 80, fl);	/* read comment */

			cmd_no++;
		}
		
		fclose (fl);
	}
		//boot_zones_raffects();

}

/*************************************************************************
*  stuff related to the save/load player system								  *
*********************************************************************** */

/* Load a char, true if loaded, false if not */

int
load_char_objs (CHAR_DATA * ch, char *name)
{
	FILE *pf;
	char fbuf[265];

	if (!name)
	{
		system_log ("BUG: name NULL in load_char_objs: db.c", true);
		return 0;
	}
	else if (!*name)
	{
		system_log ("BUG: name empty in load_char_objs: db.c\n", true);
		return 0;
	}

	sprintf (fbuf, "save/objs/%c/%s", tolower (*name), name);
	if (!(pf = fopen (fbuf, "r")))
	{
		equip_newbie (ch);
		return 0;
	}

	read_obj_suppliment (ch, pf);

	fclose (pf);

	return 1;
}

void
autosave (void)
{
	int save_count = 0;
	CHAR_DATA *t;

	//for (t = character_list; t; t = t->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		t = *tch_iterator;

		if (t->deleted || IS_NPC (t))
			continue;

		if (t->desc && t->desc->connected == CON_PLYNG && (t->in_room!=NOWHERE))
		{
			save_char (t, true);
			save_count++;
		}
	}
}


/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */
inline bool
is_valid_string_char (char c)
{
	return ((c >= 32 && c <= 126)
		|| (c >= 9 && c <= 13));
	/// \todo Allow valid extended characters (c >= 0xC0 && c <= 0xFC)
}

char *
fread_string (FILE * fp)
{
	char c;
	char string_space[MAX_STRING_LENGTH];
	char *plast;

	plast = string_space;

	while (isspace ((c = getc (fp))))
	{
		*plast++ = c;
		if (c != '\t' && c != ' ')
			plast = string_space;
	}

	if ((*plast++ = c) == '~')
		return "";

	for (;;)
	{
		switch (*plast = getc (fp))
		{
		default:
			if (is_valid_string_char(*plast))
				plast++;
			break;

		case EOF:
			*plast++ = '\0';
			system_log ("Fread_string() error.", true);
			exit (1);
			break;

		case '~':
			*plast = '\0';
			return duplicateString (string_space);
		}
	}
	return NULL;
}

char *
read_string (char *string)
{
	char buf[MAX_STRING_LENGTH];

	*buf = '\0';

	while (isspace (*string))
	{
		*string++;
	}

	if (*string == '~')
		return "";

	for (;;)
	{
		switch (*string)
		{
		default:
			*string++;
			sprintf (buf + strlen (buf), "%c", *string);
			break;
		case '~':
			return duplicateString (buf);
		}
	}
}

OBJ_DATA *
new_object ()
{
	OBJ_DATA *obj = new OBJ_DATA;

	obj->wdesc = NULL;
	obj->wounds = NULL;
	obj->writing = NULL;
	obj->writing_loaded = false;
	obj->xaffected = NULL;
	obj->zone = -1;
	obj->material = 0;
	obj->morphTime = 0;
	obj->morphto = 0;
	obj->name = NULL;
	obj->next = NULL;
	obj->next_content = NULL;
	obj->nVirtual = 0;
	obj->obj_timer = 0;
	obj->omote_str = NULL;
	obj->open = 0;
	obj->order_time = 0;
	obj->quality = 0;
	obj->short_description = NULL;
	obj->silver = 0;
	obj->size = 0;
	obj->sold_at = 0;
	obj->sold_by = 0;
	obj->super_vnum = 0;
	obj->title_language = 0;
	obj->title_script = 0;
	obj->title_skill = -1;
	obj->tmp_flags = 0;
	obj->var_color = NULL;
	obj->deleted = 0;
	obj->desc_keys = NULL;
	obj->description = NULL;
	obj->econ_flags = 0;
	obj->equiped_by = NULL;
	obj->ex_description = NULL;
	obj->farthings = 0;
	obj->full_description = NULL;
	obj->in_obj = NULL;
	obj->in_room = 0;
	obj->indoor_desc = NULL;
	obj->ink_color = NULL;
	obj->instances = 0;
	obj->item_wear = 100;
	obj->loaded = NULL;
	obj->location = 0;
	obj->lodged = NULL;
	obj->activation = 0;
	obj->book_title = NULL;
	obj->carried_by = NULL;
	obj->clan_data = NULL;
	obj->clock = 0;
	obj->coldload_id = 0;
	obj->contained_wt = 0;
	obj->contains = NULL;
	obj->count = 0;
	obj->damage = NULL;
	obj->hnext = NULL;
	obj->lnext = NULL;

	return obj;
}


/* release memory allocated for a char struct */
/*
void
free_char (CHAR_DATA * ch)
{
struct memory_data *mem;
ROLE_DATA *role;
DREAM_DATA *dream;
PC_DATA *pc;
MOB_DATA *mob;
VAR_DATA *var;
ALIAS_DATA *tmp_alias;

memory_check = true;

if (ch->pc)
{
while (ch->pc->aliases)
{
tmp_alias = ch->pc->aliases;
ch->pc->aliases = ch->pc->aliases->next_alias;
alias_free (tmp_alias);
}
}

clear_pmote (ch);
clear_dmote (ch);

while (ch->hour_affects)
affect_remove (ch, ch->hour_affects);

while (ch->pc && ch->pc->dreams)
{

dream = ch->pc->dreams;

ch->pc->dreams = ch->pc->dreams->next;

if (dream->dream && *dream->dream)
{
free_mem (dream->dream);
dream->dream = NULL;
}

free_mem (dream);
dream = NULL;
}

while (ch->pc && ch->pc->dreamed)
{
dream = ch->pc->dreamed;
ch->pc->dreamed = ch->pc->dreamed->next;
if (dream->dream && *dream->dream)
{
free_mem (dream->dream);
dream->dream = NULL;
}
free_mem (dream);
dream = NULL;
}

while (ch->remembers)
{
mem = ch->remembers;
ch->remembers = mem->next;
free_mem (mem);
mem = NULL;
}

while (ch->vartab)
{

var = ch->vartab;
ch->vartab = var->next;

if (var->name && *var->name)
{
free_mem (var->name);
var->name = NULL;
}

free_mem (var);
var = NULL;
}

if (ch->pc)
{

if ((role = ch->pc->special_role) != NULL)
{
if (role->summary && *role->summary)
free_mem (role->summary);
if (role->body && *role->body)
free_mem (role->body);
if (role->date && *role->date)
free_mem (role->date);
if (role->poster && *role->poster)
free_mem (role->poster);
free_mem (role);
ch->pc->special_role = NULL;
}

if (ch->pc->account_name && *ch->pc->account_name)
{
free_mem (ch->pc->account_name);
ch->pc->account_name = NULL;
}

if (ch->pc->site_lie && *ch->pc->site_lie)
{
free_mem (ch->pc->site_lie);
ch->pc->site_lie = NULL;
}

if (ch->pc->imm_leave && *ch->pc->imm_leave)
{
free_mem (ch->pc->imm_leave);
ch->pc->imm_leave = NULL;
}

if (ch->pc->imm_enter && *ch->pc->imm_enter)
{
free_mem (ch->pc->imm_enter);
ch->pc->imm_enter = NULL;
}

if (ch->pc->creation_comment && *ch->pc->creation_comment)
{
free_mem (ch->pc->creation_comment);
ch->pc->creation_comment = NULL;
}

if (ch->pc->msg && *ch->pc->msg)
{
free_mem (ch->pc->msg);
ch->pc->msg = NULL;
}

ch->pc->owner = NULL;
}

if (ch->voice_str && *ch->voice_str)
{
free_mem (ch->voice_str);
ch->voice_str = NULL;
}

if (ch->travel_str && *ch->travel_str)
{
free_mem (ch->travel_str);
ch->travel_str = NULL;
}

if (ch->pmote_str && *ch->pmote_str)
{
free_mem (ch->pmote_str);
ch->pmote_str = NULL;
}

if (ch->plan)
{
delete ch->plan;
ch->plan = 0;
}
if (ch->goal)
{
delete ch->goal;
ch->goal = 0;
}

// for the typical mob proto (loaded at boot time)
// these pointers are in perm memory and do not get
// deleted. however newly minit'd mobs share memory
// that is not protected, so freeing here will corrupt
// the proto if it is not saved.
if (!IS_NPC(ch))
{
if (ch->tname && *ch->tname)
{
free_mem (ch->tname);
ch->tname = NULL;
}

if (ch->name && *ch->name)
{
free_mem (ch->name);
ch->name = NULL;
}

if (ch->short_descr && *ch->short_descr)
{
free_mem (ch->short_descr);
ch->short_descr = NULL;
}

if (ch->long_descr && *ch->long_descr)
{
free_mem (ch->long_descr);
ch->long_descr = NULL;
}

if (ch->description && *ch->description)
{
free_mem (ch->description);
ch->description = NULL;
}
}
else
{
CHAR_DATA* proto = vtom(ch->mob->nVirtual);
if (proto)
{
if (ch->tname && *ch->tname
&& ch->tname != proto->tname)
{
free_mem (ch->tname);
ch->tname = NULL;
}

if (ch->name && *ch->name
&& ch->name != proto->name)
{
free_mem (ch->name);
ch->name = NULL;
}

if (ch->short_descr && *ch->short_descr
&& ch->short_descr != proto->short_descr )
{
free_mem (ch->short_descr);
ch->short_descr = NULL;
}

if (ch->long_descr && *ch->long_descr
&& ch->long_descr != proto->long_descr)
{
free_mem (ch->long_descr);
ch->long_descr = NULL;
}

if (ch->description && *ch->description
&& ch->description != proto->description)
{
free_mem (ch->description);
ch->description = NULL;
}
}
else
{
fprintf (stderr, "Proto not defined for NPC %d?\n", ch->mob->nVirtual);
}
}

if (ch->clans && *ch->clans)
{
free_mem (ch->clans);
ch->clans = NULL;
}

if (ch->combat_log && *ch->combat_log)
{
free_mem (ch->combat_log);
ch->combat_log = NULL;
}

if (ch->enemy_direction && *ch->enemy_direction)
{
free_mem (ch->enemy_direction);
ch->enemy_direction = NULL;
}

if (ch->delay_who && !isdigit (*ch->delay_who) && *ch->delay_who)
{
free_mem (ch->delay_who);
ch->delay_who = NULL;
}

if (ch->delay_who2 && !isdigit (*ch->delay_who2) && *ch->delay_who2)
{
free_mem (ch->delay_who2);
ch->delay_who2 = NULL;
}

while (ch->wounds)
wound_from_char (ch, ch->wounds);

while (ch->lodged)
lodge_from_char (ch, ch->lodged);

while (ch->attackers)
attacker_from_char (ch, ch->attackers);

while (ch->threats)
threat_from_char (ch, ch->threats);

if (ch->pc)
free_mem (ch->pc);

if (ch->mob)
free_mem (ch->mob);

free_mem (ch);

ch = NULL;

memory_check = false;
} */

void
free_obj (OBJ_DATA * obj)
{
	AFFECTED_TYPE *af;
	OBJ_DATA *tobj;
	WRITING_DATA *writing;

	if (obj->nVirtual == 0)
		return;
	else
		tobj = vtoo (obj->nVirtual);

	/* Make sure these arn't duplicate fields of the prototype */

	if (!tobj || tobj->name != obj->name)
		free_mem (obj->name);

	if (!tobj || tobj->short_description != obj->short_description)
		free_mem (obj->short_description);

	if (!tobj || tobj->description != obj->description)
		free_mem (obj->description);

	if (!tobj || tobj->full_description != obj->full_description)
		free_mem (obj->full_description);

	if (!tobj || tobj->var_color != obj->var_color)
		free_mem (obj->var_color);

	if (!tobj || tobj->desc_keys != obj->desc_keys)
		free_mem (obj->desc_keys);

	clear_omote (obj);

	obj->short_description = NULL;
	obj->description = NULL;
	obj->full_description = NULL;
	obj->var_color = NULL;
	obj->name = NULL;
	obj->desc_keys = NULL;

	while ((af = obj->xaffected))
	{
		obj->xaffected = af->next;
		free_mem (af);
	}

	while (obj->wounds)
		wound_from_obj (obj, obj->wounds);

	while (obj->lodged)
		lodge_from_obj (obj, obj->lodged);

	while (obj->writing)
	{
		writing = obj->writing;
		obj->writing = writing->next_page;
		if (writing->message)
			free_mem (writing->message);
		if (writing->author)
			free_mem (writing->author);
		if (writing->date)
			free_mem (writing->date);
		if (writing->ink)
			free_mem (writing->ink);
		free_mem (writing);
	}

	memset (obj, 0, sizeof (OBJ_DATA));

	free_mem (obj);

}

/* read contents of a text file, and place in buf */

char *
file_to_string (char *name)
{
	FILE *fl;
	char tmp[MAX_STRING_LENGTH];	/* max size on the string */
	char *string;
	int num_chars;

	if (!(fl = fopen (name, "r")))
	{
		sprintf (tmp, "file_to_string(%s)", name);
		perror (tmp);
		string = duplicateString('\0');
		return (string);
	}

	num_chars = fread (tmp, 1, MAX_STRING_LENGTH - 1, fl);
	tmp[num_chars] = '\0';
	std::string tempString = tmp;
	tempString += "\r\0";
	string = duplicateString(tempString.c_str());

	fclose (fl);

	return (string);
}

/*
void
clear_char (CHAR_DATA * ch)
{
PC_DATA *pc;
MOB_DATA *mob;
int i = 0;

if ((mob = ch->mob))
memset (mob, 0, sizeof (MOB_DATA));

if ((pc = ch->pc))
{
pc->dreams = 0;
pc->dreamed = 0;
pc->create_state = 0;
pc->mortal_mode = 0;
pc->edit_obj = 0;
pc->edit_mob = 0;
pc->level = 0;
pc->boat_virtual = 0;
pc->staff_notes = 0;
pc->mount_speed = 0;
pc->edit_player = 0;
pc->target_mob = 0;
pc->msg = 0;
pc->creation_comment = 0;
pc->imm_enter = 0;
pc->imm_leave = 0;
pc->site_lie = 0;
pc->start_str = 0;
pc->start_dex = 0;
pc->start_con = 0;
pc->start_wil = 0;
pc->start_aur = 0;
pc->start_intel = 0;
pc->start_agi = 0;
pc->load_count = 0;
pc->chargen_flags = 0;
pc->last_global_pc_msg = 0;
pc->last_global_staff_msg = 0;
pc->sleep_needed = 0;
pc->auto_toll = 0;
pc->doc_type = 0;
pc->doc_index = 0;
pc->owner = 0;
for (int i = 0; i < MAX_SKILLS; ++i)
{
pc->skills[i] = 0;
}
pc->dot_shorthand = 0;
pc->aliases = 0;
pc->execute_alias = 0;
pc->last_logon = 0;
pc->last_logoff = 0;
pc->last_disconnect = 0;
pc->last_connect = 0;
pc->last_died = 0;
pc->account_name = 0;
pc->writing_on = 0;
pc->edit_craft = 0;
pc->app_cost = 0;
pc->nanny_state = 0;
pc->role = 0;
pc->special_role = 0;
pc->admin_loaded = 0;
pc->time_last_activity = 0;
pc->is_guide = 0;
pc->profession = 0;
}
memset (ch, 0, sizeof (CHAR_DATA));

ch->pc = pc;
ch->mob = mob;

ch->damage = 0;

ch->room = NULL;
ch->in_room = NOWHERE;
ch->was_in_room = NOWHERE;
ch->position = POSITION_STANDING;
ch->default_pos = POSITION_STANDING;
ch->wounds = NULL;
ch->body_proto = 0;
ch->lodged = NULL;
ch->mount = NULL;
ch->following = 0;
ch->fighting = NULL;
ch->subdue = NULL;
ch->vehicle = NULL;
ch->shop = NULL;
ch->hour_affects = NULL;
ch->equip = NULL;
ch->desc = NULL;
ch->hitcher = NULL;
ch->hitchee = NULL;
ch->aiming_at = NULL;
ch->targeted_by = NULL;

ch->next = NULL;
ch->next_in_room = NULL;
ch->next_fighting = NULL;
ch->next_assist = NULL;
ch->assist_pos = 0;

if (ch->pc)
{
ch->pc->is_guide = 0;
ch->pc->admin_loaded = false;
ch->pc->profession = 0;
}

// initialize and clear ch's known spell array

for (i = 0; i < MAX_LEARNED_SPELLS; i++)
{
ch->spells[i][0] = 0;
ch->spells[i][1] = 0;
}
} */


void
clear_object (OBJ_DATA * obj)
{
	memset (obj, 0, sizeof (OBJ_DATA));

	obj->silver = 0;
	obj->farthings = 0;
	obj->nVirtual = -1;
	obj->in_room = NOWHERE;
	obj->wounds = NULL;
	obj->lodged = NULL;
	obj->description = NULL;
	obj->short_description = NULL;
	obj->full_description = NULL;
	obj->equiped_by = NULL;
	obj->carried_by = NULL;
}

void
save_char_objs (CHAR_DATA * ch, char *name)
{
	FILE *of;
	char fbuf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	sprintf (fbuf, "save/objs/%c/%s", tolower (*name), name);

	if (!IS_NPC (ch) && ch->pc->create_state == STATE_DIED)
	{
		sprintf (buf2, "mv %s %s.died", fbuf, fbuf);
		system (buf2);
	}

	if (!(of = fopen (fbuf, "w")))
	{
		sprintf (buf2, "ERROR: Opening obj save file. (%s)", ch->tname);
		system_log (buf2, true);
		return;
	}

	write_obj_suppliment (ch, of);

	fclose (of);
}


/*
* Read a number from a file.
*/
int
fread_number (FILE * fp)
{
	char c;
	long int number;
	bool sign;

	do
	{
		c = getc (fp);
	}
	while (isspace (c));

	number = 0;

	sign = false;
	if (c == '+')
	{
		c = getc (fp);
	}
	else if (c == '-')
	{
		sign = true;
		c = getc (fp);
	}

	if (!isdigit (c))
	{
		system_log ("Fread_number(): bad format.", true);
		return 0;
		//abort ();
	}

	while (isdigit (c))
	{
		number = number * 10 + c - '0';
		c = getc (fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number (fp);
	else if (c != ' ')
		ungetc (c, fp);

	return number;
}


/*
* Read one word (into static buffer).
*/
char *
fread_word (FILE * fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
	{
		cEnd = getc (fp);
	}
	while (isspace (cEnd));

	if (cEnd == '\'' || cEnd == '"')
	{
		pword = word;
	}
	else
	{
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = getc (fp);
		if (cEnd == ' ' ? isspace (*pword) : *pword == cEnd)
		{
			if (cEnd == ' ')
				ungetc (*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	system_log ("Fread_word(): word too long.", true);
	return NULL;
	//abort ();
}

void
boot_mobiles ()
{
	char strFilename[AVG_STRING_LENGTH] = "";
	char buf[AVG_STRING_LENGTH] = "";
	int nVirtual = 0;
	int nZone = 0;
	FILE *fp = NULL;
	extern CHAR_DATA *fread_mobile (int vnum, const int *nZone, FILE * fp);

	for (nZone = 0; nZone < MAX_ZONE; nZone++)
	{

		sprintf (strFilename, "%s/mobs.%d", REGIONS, nZone);

		if ((fp = fopen (strFilename, "r")) == NULL)
			continue;

		while (1)
		{
			if (!fgets (buf, 81, fp))
			{
				system_log ("Error reading mob file:", true);
				system_log (buf, true);
				perror ("Reading mobfile");
				abort ();
			}

			if (*buf == '#')
			{
				sscanf (buf, "#%d", &nVirtual);
				fread_mobile (nVirtual, &nZone, fp);
			}
			else if (*buf == '$')
				break;
		}

		fclose (fp);
	}
}

void
create_penny_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_PENNY) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_PENNY;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("pennies penny coins money cash silver");

	obj->short_description = "";
	obj->description = "";
	obj->full_description = "";

	obj->obj_flags.type_flag = ITEM_MONEY;
	obj->obj_flags.wear_flags = ITEM_TAKE;
	obj->obj_flags.extra_flags = ITEM_STACK;
	obj->obj_flags.weight = 1;

	obj->farthings = 0;
	obj->silver = 1;
	obj->count = 1;

	obj->in_room = NOWHERE;
}

void
create_farthing_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_FARTHING) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_FARTHING;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("farthings coins money cash brass");

	obj->short_description = "";
	obj->description = "";
	obj->full_description = "";

	obj->obj_flags.type_flag = ITEM_MONEY;
	obj->obj_flags.wear_flags = ITEM_TAKE;
	obj->obj_flags.extra_flags = ITEM_STACK;
	obj->obj_flags.weight = 1;

	obj->farthings = 1;
	obj->silver = 0;
	obj->count = 1;

	obj->in_room = NOWHERE;
}

void
create_money_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_MONEY) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_MONEY;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("coins coin silver money cash penny");

	obj->obj_flags.type_flag = ITEM_MONEY;
	obj->obj_flags.wear_flags = ITEM_TAKE;
	obj->obj_flags.weight = 1;

	obj->farthings = 0;
	obj->silver = 1;

	obj->in_room = NOWHERE;
}

void
create_ticket_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_TICKET) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_TICKET;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("ticket small number paper");
	obj->short_description = duplicateString ("a small ostler's ticket");
	obj->description = duplicateString ("A small paper ticket with a number "
		"is here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1;
	obj->obj_flags.type_flag = ITEM_TICKET;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
}

void
create_order_ticket_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_ORDER_TICKET) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_ORDER_TICKET;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("ticket small paper merchandise");
	obj->short_description = duplicateString ("a small merchandise ticket");
	obj->description =
		duplicateString ("A small merchandise ticket has been carelessly left here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1;
	obj->obj_flags.type_flag = ITEM_MERCH_TICKET;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
}

void
create_head_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_HEAD) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_HEAD;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("head");
	obj->short_description = duplicateString ("a head");
	obj->description = duplicateString ("A head is here.");
	obj->full_description = "";

	obj->obj_flags.weight = 10;
	obj->obj_flags.type_flag = ITEM_HEAD;
	obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;

	obj->in_room = NOWHERE;
}

void
create_corpse_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_CORPSE) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_CORPSE;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("corpse");
	obj->short_description = duplicateString ("a corpse");
	obj->description = duplicateString ("A corpse is here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1000;
	obj->o.container.capacity = 0;	/* No keeping things on a corpse */
	obj->obj_flags.type_flag = ITEM_CONTAINER;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
}

void
create_statue_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_STATUE) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_STATUE;

	add_obj_to_hash (obj);

	obj->name = duplicateString ("statue");
	obj->short_description = duplicateString ("a remarkably lifelike statue");
	obj->description = duplicateString ("A remarkably lifelike statue looms here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1000;
	obj->o.container.capacity = 0;	/* No keeping things on a corpse */
	obj->obj_flags.type_flag = ITEM_CONTAINER;

	obj->in_room = NOWHERE;
}

void
boot_objects ()
{
	char buf[MAX_STRING_LENGTH];
	int nVirtual;
	int zone;
	FILE *fp;

	for (zone = 0; zone < OBJECT_MAX_ZONE; zone++)
	{

		sprintf (buf, "%s/objs.%d", REGIONS, zone);

		if ((fp = fopen (buf, "r")) == NULL)
			continue;

		while (1)
		{
			if (!fgets (buf, 81, fp))
			{
				system_log ("Error reading obj file:", true);
				perror ("Reading objfile");
				abort ();
			}

			if (*buf == '#')
			{
				sscanf (buf, "#%d", &nVirtual);
				fread_object (nVirtual, zone, fp);
			}
			else if (*buf == '$')
				break;
		}

		fclose (fp);
	}

	clan__assert_member_objs ();

	create_ticket_proto ();
	create_order_ticket_proto ();
	create_head_proto ();
	create_corpse_proto ();
	create_statue_proto ();
}

struct hash_data
{
	int len;
	char *string;
	struct hash_data *next;
};

void
boot_zones_raffects (void)
{
	FILE *fl;
	int zon;
	bool flag;
	int room_num;
	char c;
	char zfile[MAX_STRING_LENGTH];
	struct stat fstatus;
	char buf[MAX_STRING_LENGTH];
	AFFECTED_TYPE *af;
	ROOM_DATA * troom;
	
	
	for (zon = 0; zon < MAX_ZONE; zon++)
	{
		
		sprintf (zfile, "%s/raffects/resets.%d", SAVE_DIR, zon);
		
		if (!(fl = fopen (zfile, "r")))
		{
			continue;
		}
		
		fscanf (fl, "%c", &c);
		
		if (c == '$')
		{
			fclose (fl);
			continue;
		}
		
		for (;;)
		{
			if (c == '#')
			{
				fscanf (fl, "%d\n", &room_num); //room number				
				if (!(troom = vtor (room_num))) //if the room doesn't exist
				{
					flag = false;
					sprintf (buf, "Room %d does not exist.", room_num);
					system_log (buf, true);
				}
				else {
					flag = true;
				}

			}
			
			fscanf (fl, "%c", &c);
			
			if (c == 'r' && flag)
			{
				
				af = new AFFECTED_TYPE;
				
				fscanf (fl, "%d %d %d",
						&af->type,
						&af->a.room.duration,
						&af->a.room.intensity);
				if (flag)
					add_room_affect (&troom->affects, af->type, af->a.room.duration, af->a.room.intensity);
				
			}
			
			if (c == 'S')
			{
				flag = true;
				continue;
			}
			
			if (c == '$')
			{
				fclose (fl);
				break;
			}
			
		}
		
		
	}
	
	return;
	
}

