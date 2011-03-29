/*------------------------------------------------------------------------\
|  olc.c : OLC Module                                 www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <mysql/mysql.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "decl.h"
#include "utils.h"
#include "utility.h"


#define ROOM_MAX		100000

#define s(a) send_to_char (a "\n", ch);

extern rpie::server engine;
extern const char *weather_states[];
extern const char *damage_severity[];
extern int free_mem(char *&ptr);
extern int free_mem(void *ptr);

const char *apply_types[] = {
	"None",
	"Strength",
	"Dexterity",
	"Intelligence",
	"Charisma",
	"Aura",
	"Will",
	"Constitution",
	"Sex",
	"Age",
	"Weight",
	"Height",
	"Defense",
	"Hitpoints",
	"Movement",
	"Cash",
	"Armor",
	"Offense",
	"Damagebonus",
	"Save_Par",
	"Save_Rod",
	"Save_Pet",
	"Save_Bre",
	"Save_Spe",
	"\n"
};

const char *deity_name[] = {
	"None",
	"Manwe Sulimo",
	"Varda Elentari",
	"Ulmo",
	"Yavanna Kementari",
	"Aule the Smith",
	"Nienna",
	"Orome",
	"Este the Gentle",
	"Mandos",
	"Vaire the Weaver",
	"Lorien",
	"Vana the Ever-young",
	"Tulkas Astaldo",
	"Nessa",
	"\n"
};

const char *wound_locations[] = {
	"skull",
	"reye",
	"leye",
	"face",
	"neck",
	"rshoulder",
	"lshoulder",
	"rupperarm",
	"lupperarm",
	"relbow",
	"lelbow",
	"rforearm",
	"lforearm",
	"rhand",
	"lhand",
	"thorax",
	"abdomen",
	"hip",
	"groin",
	"rthigh",
	"lthigh",
	"rknee",
	"lknee",
	"rcalf",
	"lcalf",
	"rfoot",
	"lfoot",
	"rforeleg",
	"lforeleg",
	"rhindleg",
	"lhindleg",
	"rforefoot",
	"lforefoot",
	"rhindfoot",
	"lhindfoot",
	"rforepaw",
	"lforepaw",
	"rhindpaw",
	"lhindpaw",
	"rforehoof",
	"lforehoof",
	"rhindhoof",
	"lhindhoof",
	"stinger",
	"muzzle",
	"rleg",
	"lleg",
	"rwing",
	"lwing",
	"tail",
	"\n"
};

const char *materials[] = {
	"None",
	"Textile",
	"Leather",
	"Wood",
	"Metal",
	"Stone",
	"Glass",
	"Parchment",
	"Liquid",
	"Vegetation",
	"Ceramic",
	"Other",
	"Meat",
	"\n"
};

const char *drinks[] = {
	"water",
	"ale",
	"beer",
	"cider",
	"mead",
	"wine",
	"brandy",
	"soup",
	"milk",
	"tea",
	"saltwater",
	"dark ale",
	"amber ale",
	"pale ale",
	"dark beer",
	"white wine",
	"red wine",
	"herbal tea",
	"black tea",
	"chamomile tea",
	"tonic",
	"blood",
	"apple juice",
	"peach nectar",
	"pear juice",
	"carrot juice",
	"hard cider",
	"rye whiskey",
	"red ale",
	"stout ale",
	"stout beer",
	"oil",
	"wax",
	"\n"
};

const char *frames[] = {
	"fragile",
	"scant",
	"light",
	"medium",
	"heavy",
	"massive",
	"monstrous",
	"mammoth",
	"gigantic",
	"gargantuan",
	"colossal",
	"\n"
};


const char *wear_bits[] = {
	"Take",
	"Finger",
	"Neck",
	"Body",
	"Head",
	"Legs",
	"Feet",
	"Hands",
	"Arms",
	"Wshield",
	"About",
	"Waist",
	"Wrist",
	"Wield",
	"Unused",
	"Unused",
	"Unused",
	"Sheath",
	"Belt",
	"Back",
	"Blindfold",
	"Throat",
	"Ears",
	"Shoulder",
	"Ankle",
	"Hair",
	"Face",
	"Armband",
	"\n"
};

const char *extra_bits[] = {
	"Destroyed",
	"Pitchable",
	"Invisible",
	"Magic",
	"Nodrop",
	"Bless",
	"Get-affect",
	"Drop-affect",
	"Multi-affect",
	"Wear-affect",
	"Wield-affect",
	"Hit-affect",
	"Ok",
	"Combinable",
	"item-leader",
	"item-member",
	"item-omni",
	"Illegal",
	"Restricted",
	"Mask",
	"Mount",
	"Table",
	"Stack",
	"vNPCDwelling",
	"Loads",
	"Variable",
	"Timer",
	"PC-Sold",
	"Thrown",
	"NewSkills",
	"Pitched",
	"IsVNPC",
	"\n"
};

const char *room_bits[] = {
	"Dark",
	"Ruins",
	"NoMob",
	"Indoors",
	"Lawful",
	"NoMagic",
	"Tunnel",
	"Cave",
	"SafeQuit",
	"Deep",
	"Fall",
	"NoMount",
	"Vehicle",
	"Fog",
	"NoMerchant",
	"Climb",
	"Psave",
	"Lab",
	"Road",
	"Wealthy",
	"Poor",
	"Scum",
	"Temporary",
	"Arena",
	"Dock",
	"Wild",
	"Light",
	"NoHide",
	"Storage",
	"PcEntered",
	"Ok",
	"OOC",
	"\n"
};

const char *sector_types[] = {
	"Inside",
	"City",
	"Road",
	"Trail",
	"Field",
	"Woods",
	"Forest",
	"Hills",
	"Mountains",
	"Swamp",
	"Dock",
	"Cave",
	"Pasture",
	"Heath",
	"Pit",
	"Leanto",
	"Lake",
	"River",
	"Ocean",
	"Reef",
	"Underwater",
	"\n"
};

const char *item_types[] = {
	"Undefined",
	"Light",
	"Scroll",
	"Wand",
	"Staff",
	"Weapon",
	"Shield",
	"Missile",
	"Treasure",
	"Armor",
	"Potion",
	"Worn",
	"Other",
	"Trash",
	"Trap",
	"Container",
	"Note",
	"Liquid_container",
	"Key",
	"Food",
	"Money",
	"Ore",
	"Board",
	"Fountain",
	"Grain",
	"Perfume",
	"Pottery",
	"Salt",
	"Zone",
	"Plant",
	"Component",
	"Herb",
	"Salve",
	"Poison",
	"Lockpick",
	"Wind_inst",
	"Percu_inst",
	"String_inst",
	"Fur",
	"Woodcraft",
	"Spice",
	"Tool",
	"Usury_note",
	"Bridle",
	"Ticket",
	"Skull",
	"Dye",
	"Cloth",
	"Ingot",
	"Timber",
	"Fluid",
	"Liquid_Fuel",
	"Remedy",
	"Parchment",
	"Book",
	"Writing_inst",
	"Ink",
	"Quiver",
	"Sheath",
	"Keyring",
	"Bullet",
	"NPC_Object",
	"Dwelling",
	"Unused",
	"Repair",
	"Tossable",
	"DO_NOT_USE",
	"MerchTicket",
	"RoomRental",
	"\n"
};

const struct constant_data constant_info[] = {
	{"item-types", "OSET                   ", (void **) item_types},
	{"econ-flags", "OSET/MSET flag         ", (void **) econ_flags},
	{"materials", "OSET NAME <list>       ", (void **) materials},
	{"wear-bits", "OSET flag              ", (void **) wear_bits},
	{"extra-bits", "OSET flag              ", (void **) extra_bits},
	{"apply-types", "OSET affect            ", (void **) apply_types},
	{"drinks", "OSET oval2 <#>         ", (void **) drinks},
	{"position-types", "MSET                   ", (void **) position_types},
	{"sex-types", "MSET                   ", (void **) sex_types},
	{"action-bits", "MSET flag              ", (void **) action_bits},
	{"affected-bits", "MSET flag              ", (void **) affected_bits},
	{"skills", "MSET                   ", (void **) skills},
	{"speeds", "MSET speed             ", (void **) speeds},
	{"room-bits", "RFLAGS flag              ", (void **) room_bits},
	{"exit-bits", "RDFLAGS dir flag		  ", (void **) exit_bits},
	{"sector-types", "RSECTOR                  ", (void **) sector_types},
	{"weather-room", "WEATHER                  ", (void **) weather_room},
	//      { "rfuncs",             "Room Programs            ", (void **) rfuncs },
	{"colors", "Random Object Colors     ", (void **) standard_object_colors},
	{"drabcolors", "Random Object Colors     ", (void **) drab_object_colors},
	{"finecolors", "Random Object Colors     ", (void **) fine_object_colors},
	{"gemcolors", "Random Object Colors     ", (void **) gem_colors},
	{"finegemcolors", "Random Object Colors     ", (void **) fine_gem_colors},
	{"woundlocs", "Wound Locations          ", (void **) wound_locations},
	{"variable-races", "Variable Races           ", (void **) variable_races},
	{"damage-types", "REND Damage Types        ", (void **) damage_type},
	{"weather-states", "Weather states           ", (void **) weather_states},
	{"damage-severity", "Damage severity          ", (void **) damage_severity},
	{"", "", NULL}
};

void
do_wearloc (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	char loc[MAX_STRING_LENGTH];
	int wear_loc, zone = 0, count = 0;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Which wear location did you wish to search for?\n", ch);
		return;
	}
	else
		sprintf (loc, "%s", buf);

	if ((wear_loc = index_lookup (wear_bits, loc)) == -1)
	{
		send_to_char ("That isn't a valid wear location.\n", ch);
		return;
	}

	*buf = '\0';

	argument = one_argument (argument, buf);

	if (*buf && isdigit (*buf))
		zone = atoi (buf);
	else
		zone = -1;

	*b_buf = '\0';

	sprintf (b_buf, "Vnum       Name(s)           Short Desc"
		"               Clones\tLocation\n\n");

	for (obj = full_object_list; obj; obj = obj->lnext)
	{
		if (!obj)
			continue;
		if (zone != -1 && obj->zone != zone)
			continue;
		if (!IS_SET (obj->obj_flags.wear_flags, 1 << wear_loc))
			continue;
		count++;
		sprintf (b_buf + strlen (b_buf),
			"%.5d %-16.16s %-28.28s %.3d\t(R) %05d\n", obj->nVirtual,
			obj->name, obj->short_description, 999, obj->in_room);
	}

	page_string (ch->desc, b_buf);

}

/* some extra arg functions....probably should be moved */

void
make_quiet (CHAR_DATA * ch)
{
	ch->act |= PLR_QUIET;
}

void
do_zmode (CHAR_DATA * ch, char *argument, int cmd)
{
	int mode, zone;
	char buf[80];

	for (; isspace (*argument); argument++);
	if (!*argument)
	{
		send_to_char ("You must supply a mode.\n", ch);
		return;
	}
	if (!isdigit (*argument))
	{
		send_to_char ("Argument must be a number.\n", ch);
		return;
	}
	zone = ch->room->zone;
	mode = atoi (argument);

	zone_table[zone].reset_mode = mode;

	sprintf (buf, "Zone %d reset mode changed to %d.\n", zone, mode);
	send_to_char (buf, ch);
}

void
do_zlife (CHAR_DATA * ch, char *argument, int cmd)
{
	int life, zone;
	char buf[80];

	for (; isspace (*argument); argument++);
	if (!*argument)
	{
		send_to_char ("You must supply a lifespan.\n", ch);
		return;
	}
	if (!isdigit (*argument))
	{
		send_to_char ("Argument must be a number.\n", ch);
		return;
	}

	zone = vtor (ch->in_room)->zone;

	life = atoi (argument);

	zone_table[zone].lifespan = life;

	sprintf (buf, "Zone %d lifespan changed to %d minutes.\n", zone, life);
	send_to_char (buf, ch);
}

void
do_freeze (CHAR_DATA * ch, char *argument, int cmd)
{
	int num;
	char buf[80];

	for (; isspace (*argument); argument++);

	if (!isdigit (*argument))
	{
		send_to_char ("That is not a valid zone number.\n", ch);
		return;
	}

	num = atoi (argument);

	if (num == 101)
	{
		for (num = 0; num < MAX_ZONE; num++)
			zone_table[num].flags |= Z_FROZEN;
		return;
	}

	if (num < 0 || num > 99)
	{
		send_to_char ("The zone number must be between 0 and 99.\n", ch);
		return;
	}
	if (IS_SET (zone_table[num].flags, Z_FROZEN))
	{
		sprintf (buf, "Zone %d is already frozen.\n", num);
		send_to_char (buf, ch);
		return;
	}
	else
	{
		zone_table[num].flags |= Z_FROZEN;
		sprintf (buf,
			"Zone %d is now frozen, type 'thaw %d' to re-enable mobile activity.\n",
			num, num);
		send_to_char (buf, ch);
	}
}

void
do_thaw (CHAR_DATA * ch, char *argument, int cmd)
{
	int num;
	int unfrozen = 0;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	num = atoi (buf);

	if (!str_cmp (buf, "all"))
	{
		for (num = 0; num < MAX_ZONE; num++)
		{

			if (IS_SET (zone_table[num].flags, Z_FROZEN))
				unfrozen++;

			zone_table[num].flags &= ~Z_FROZEN;
		}

		sprintf (buf, "%d zone(s) unfrozen.\n", unfrozen);
		send_to_char (buf, ch);
		return;
	}

	if (*buf != '0' && !num)
	{
		send_to_char ("Expected 0..99 or 'all'\n", ch);
		return;
	}

	if (num < 0 || num > MAX_ZONE)
	{
		send_to_char ("The zone number must be between 0 and MAX_ZONE.\n", ch);
		return;
	}

	if (!IS_SET (zone_table[num].flags, Z_FROZEN))
		send_to_char ("That zone isn't frozen.\n", ch);

	zone_table[num].flags &= ~Z_FROZEN;
}

void
fwrite_mobile (CHAR_DATA * tmob, FILE * fp)
{
	int k, i;

	fprintf (fp, "#%d\n", tmob->mob->nVirtual);
	fprintf (fp, "%s~\n%s~\n", tmob->name, tmob->short_descr);
	fprintf (fp, "%s~\n", tmob->long_descr);

	fprintf (fp, "%s~\n%lu %ld %d %d %d 5d1+%d %dd%d+%d " "%d %d %d %d %d %d %d %d\n", tmob->description, tmob->act, tmob->affected_by, tmob->offense, tmob->race,	/* Was defense */
		tmob->armor, tmob->max_hit, tmob->mob->damnodice, tmob->mob->damsizedice, tmob->mob->damroll, (int) tmob->time.birth,	/* Was clan_2 */
		tmob->position, tmob->default_pos, tmob->sex, tmob->mob->merch_seven,	/* Was clan_1 */
		tmob->shop ? tmob->shop->materials : 0,
		tmob->mob->vehicle_type, tmob->shop ? tmob->shop->buy_flags : 0);

	if (world_version_out <= 12)
	{
		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", tmob->mob->skinned_vnum, tmob->circle, tmob->cell_1, tmob->mob->carcass_vnum, tmob->cell_2,	/* Formerly tmob->defense_bonus */
			tmob->ppoints,
			tmob->natural_delay,
			tmob->mob->helm_room,
			tmob->body_type,
			0,
			tmob->nat_attack_type,
			tmob->mob->access_flags,
			tmob->height,
			tmob->frame, tmob->mob->noaccess_flags, tmob->cell_3);
	}
	else
	{
		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", tmob->mob->skinned_vnum, tmob->circle, tmob->cell_1, tmob->mob->carcass_vnum, tmob->cell_2,	/* Formerly tmob->defense_bonus */
			tmob->ppoints,
			tmob->natural_delay,
			tmob->mob->helm_room,
			tmob->body_type,
			tmob->nat_attack_type,
			tmob->mob->access_flags,
			tmob->height,
			tmob->frame, tmob->mob->noaccess_flags, tmob->cell_3);
	}

	if (world_version_out < 6)
		fprintf (fp, "%d %d %d %d %d %d %d\n",
		tmob->str,
		tmob->intel,
		tmob->wil, tmob->aur, tmob->dex, tmob->con, tmob->speaks);
	else
		fprintf (fp, "%d %d %d %d %d %d %d %d\n",
		tmob->str,
		tmob->intel,
		tmob->wil,
		tmob->aur, tmob->dex, tmob->con, tmob->speaks, tmob->agi);

	if (IS_SET (tmob->flags, FLAG_KEEPER) && !tmob->shop)
		tmob->flags &= ~FLAG_KEEPER;

	fprintf (fp, "%d %d\n", tmob->flags, tmob->mob->currency_type);

	if (IS_SET (tmob->flags, FLAG_KEEPER))
	{

		fprintf (fp, "%d %d %f %f %f %f %d\n",
			tmob->shop->shop_vnum,
			tmob->shop->store_vnum,
			tmob->shop->markup,
			tmob->shop->discount,
			tmob->shop->econ_markup1,
			tmob->shop->econ_discount1, tmob->shop->econ_flags1);

		if (tmob->mob->merch_seven > 0)
		{
			fprintf (fp,
				"%f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %d\n",
				tmob->shop->econ_markup2, tmob->shop->econ_discount2,
				tmob->shop->econ_flags2, tmob->shop->econ_markup3,
				tmob->shop->econ_discount3, tmob->shop->econ_flags3,
				tmob->shop->econ_markup4, tmob->shop->econ_discount4,
				tmob->shop->econ_flags4, tmob->shop->econ_markup5,
				tmob->shop->econ_discount5, tmob->shop->econ_flags5,
				tmob->shop->econ_markup6, tmob->shop->econ_discount6,
				tmob->shop->econ_flags6, tmob->shop->econ_markup7,
				tmob->shop->econ_discount7, tmob->shop->econ_flags7,
				tmob->shop->nobuy_flags);
		}
		else if (world_version_in >= 8)
		{
			fprintf (fp, "%f %f %d %f %f %d %d\n",
				tmob->shop->econ_markup2,
				tmob->shop->econ_discount2,
				tmob->shop->econ_flags2,
				tmob->shop->econ_markup3,
				tmob->shop->econ_discount3,
				tmob->shop->econ_flags3, tmob->shop->nobuy_flags);
		}

		for (i = 0; i <= MAX_DELIVERIES; i++)
		{
			if (!tmob->shop->delivery[i])
				break;
			fprintf (fp, "%d ", tmob->shop->delivery[i]);
		}
		fprintf (fp, "-1\n");

		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
			tmob->shop->trades_in[0],
			tmob->shop->trades_in[1],
			tmob->shop->trades_in[2],
			tmob->shop->trades_in[3],
			tmob->shop->trades_in[4],
			tmob->shop->trades_in[5],
			tmob->shop->trades_in[6],
			tmob->shop->trades_in[7],
			tmob->shop->trades_in[8], tmob->shop->trades_in[9]);
	}

	for (k = 0; k < (world_version_out >= 12 ? MAX_SKILLS : 60) / 10; k++)
		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
		tmob->skills[k * 10],
		tmob->skills[k * 10 + 1],
		tmob->skills[k * 10 + 2],
		tmob->skills[k * 10 + 3],
		tmob->skills[k * 10 + 4],
		tmob->skills[k * 10 + 5],
		tmob->skills[k * 10 + 6],
		tmob->skills[k * 10 + 7],
		tmob->skills[k * 10 + 8], tmob->skills[k * 10 + 9]);

	if (world_version_out >= 10)
		fprintf (fp, "%s~\n", tmob->clans);

	/*** for morphing mobs ***/
	fprintf (fp, "%d %d %d\n", tmob->clock, tmob->morphto, tmob->morph_type);
	/*************************/
}

/*
create table shadows_world.item_type (
id INT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
tag VARCHAR(255) NULL UNIQUE,
constant VARCHAR(255) NULL UNIQUE,
name VARCHAR(255) NULL,
sdesc VARCHAR(255) NULL,
ldesc VARCHAR(255) NULL,
fdesc VARCHAR(255) NULL,
PRIMARY KEY(id)
);

create table bp_vobjects (
vnum INT UNSIGNED NULL UNIQUE,
zone SMALLINT UNSIGNED NOT NULL DEFAULT 0,
type INT UNSIGNED NOT NULL DEFAULT 0,
material INT NOT NULL DEFAULT 0,
size SMALLINT NOT NULL DEFAULT -1,
weight INT NOT NULL DEFAULT 0,
quality INT NOT NULL DEFAULT 0,
item_wear INT NOT NULL DEFAULT 0,
silver FLOAT NOT NULL DEFAULT 0.0,
farthings FLOAT NOT NULL DEFAULT 0.0,
activation INT NOT NULL DEFAULT 0,
count INT NOT NULL DEFAULT 1,
clock INT NOT NULL DEFAULT 0,
morphto INT NOT NULL DEFAULT 0,
oval0 INT NOT NULL DEFAULT 0,
oval1 INT NOT NULL DEFAULT 0,
oval2 INT NOT NULL DEFAULT 0,
oval3 INT NOT NULL DEFAULT 0,
oval4 INT NOT NULL DEFAULT 0,
oval5 INT NOT NULL DEFAULT 0,
extra_flags SET('destroyed', 'pitchable', 'invisible', 'magic', 'nodrop', 'bless', 'get_affect', 'drop_affect', 'multi_affect', 'wear_affect', 'wield_affect', 'hit_affect', 'ok', 'combinable', 'leader', 'member', 'omni', 'illegal', 'restricted', 'mask', 'mount', 'table', 'stack', 'vnpc_dwelling', 'loads', 'variable', 'timer', 'pc_sold', 'throwing', 'newskills', 'pitched', 'vnpc') NULL,
wear_flags SET('take', 'finger', 'neck', 'body', 'head', 'legs', 'feet', 'hands', 'arms', 'shield', 'about', 'waist', 'wrist', 'wield', 'hold', 'throw', 'unused', 'sheath', 'belt', 'back', 'blindfold', 'throat', 'ear', 'shoulder', 'ankle', 'hair', 'face', 'armband') NULL,
econ_flags SET('tirith','osgiliath','gondor','morgul','magical','rare','valuable','foreign','junk','illegal','wild','poor','fine','haradaic','orkish','practice','used','numenorean','dunlending','elvish','dwarvish','hobbit','admin','fellowship','cooked','meat','pelargir','generic') NULL,
name VARCHAR(255) NULL,
sdesc VARCHAR(255) NULL,
ldesc VARCHAR(255) NULL,
fdesc TEXT NULL,
desc_keys VARCHAR(255) NULL,
ex_description TEXT NULL,
xaffected TEXT NULL,
PRIMARY KEY (vnum),
FOREIGN KEY (type) REFERENCES shadows_world.item_type (type)
);
*/
void
mysql_save_vobject (OBJ_DATA * tobj, int port)
{

	mysql_safe_query ("INSERT INTO shadows_world.bp_vobjects ( "
		" vnum, zone, "
		" name, sdesc, ldesc, fdesc, "
		" type, extra_flags, wear_flags, "
		" oval0, oval1, oval2, oval3, "
		" weight, silver, "
		" oval4 "
		") "
		"VALUES ( "
		" %d, %d, "
		" '%s','%s','%s','%s', "
		" %d, %d, %d, "
		" %d, %d, %d, %d, "
		" %d, %f, "
		" %d "
		") ;",
		tobj->nVirtual,
		tobj->nVirtual / 1000,
		tobj->name,
		tobj->short_description,
		tobj->description,
		tobj->full_description,
		tobj->obj_flags.type_flag,
		tobj->obj_flags.extra_flags & ~ITEM_LOADS,
		tobj->obj_flags.wear_flags,
		tobj->o.od.value[0],
		tobj->o.od.value[1],
		tobj->o.od.value[2],
		tobj->o.od.value[3],
		tobj->obj_flags.weight,
		tobj->silver, tobj->o.od.value[4]);
	//send_to_gods ( mysql_error ( database ) );
}

void
fwrite_object (OBJ_DATA * tobj, FILE * fp)
{
	AFFECTED_TYPE *af;
	/*
	if (engine.in_build_mode ())
	{
	mysql_save_vobject (tobj, port);
	}
	*/
	fprintf (fp, "#%d\n", tobj->nVirtual);
	fprintf (fp, "%s~\n%s~\n%s~\n%s~\n%d %d %ld\n%d %d %d %d\n%d %f\n%d ",
		tobj->name,
		tobj->short_description,
		tobj->description,
		tobj->full_description,
		tobj->obj_flags.type_flag,
		tobj->obj_flags.extra_flags & ~ITEM_LOADS,
		tobj->obj_flags.wear_flags,
		tobj->o.od.value[0],
		tobj->o.od.value[1],
		tobj->o.od.value[2],
		tobj->o.od.value[3],
		tobj->obj_flags.weight, tobj->silver, tobj->o.od.value[4]);

	if (tobj->obj_flags.type_flag == ITEM_INK)
		fprintf (fp, "\n%s~\n", tobj->ink_color);

	else if (tobj->obj_flags.type_flag == ITEM_WORN &&
		IS_SET (tobj->obj_flags.extra_flags, ITEM_MASK))
	{
		if (!tobj->desc_keys)
			fprintf (fp, "\n~\n");
		else
			fprintf (fp, "\n%s~\n", tobj->desc_keys);
	}

	else if (tobj->obj_flags.type_flag == ITEM_ARMOR &&
		IS_SET (tobj->obj_flags.extra_flags, ITEM_MASK))
	{
		if (!tobj->desc_keys)
			fprintf (fp, "\n~\n");
		else
			fprintf (fp, "\n%s~\n", tobj->desc_keys);
	}

	else if (tobj->obj_flags.type_flag == ITEM_TOSSABLE)
	{
		if (!tobj->desc_keys)
			fprintf (fp, "\n~\n");
		else
			fprintf (fp, "\n%s~\n", tobj->desc_keys);
	}

	else
	{
		if ((tobj->obj_flags.type_flag == ITEM_WORN) ||
			(tobj->obj_flags.type_flag == ITEM_ARMOR))
			tobj->o.od.value[5] = 0;

		fprintf (fp, "%d ", tobj->o.od.value[5]);
	}

	if (world_version_in == 8)
		fprintf (fp, "%d %d %d %d\n",
		tobj->activation, tobj->quality, tobj->econ_flags, tobj->size);
	else
		fprintf (fp, "%d %d %d %d %d\n",
		tobj->activation,
		tobj->quality, tobj->econ_flags, tobj->size, tobj->count);

	if (world_version_out >= 11)
	{
		fprintf (fp, "%f %d %d %d %d 0 0\n",
			tobj->farthings, tobj->clock, tobj->morphto, tobj->item_wear,
			tobj->material);
	}

	if (tobj->clan_data)
	{
		fprintf (fp, "C\n%s~\n%s~\n",
			tobj->clan_data->name, tobj->clan_data->rank);
	}

	if (tobj->super_vnum > 0) {
		fprintf(fp, "S\n%d\n", tobj->super_vnum); // Super vnum storage - Case
	}											  // Would prefer something better than 'S', but when in Rome.

	if (tobj->wdesc)
		fprintf (fp, "W\n%d\n%s~\n",
		tobj->wdesc->language, tobj->wdesc->description);

	for (af = tobj->xaffected; af; af = af->next)
		fprintf (fp, "A\n%d %d\n", af->a.spell.location, af->a.spell.modifier);
}

void
fwrite_room (ROOM_DATA * troom, FILE * fp)
{
	EXTRA_DESCR_DATA *exptr;
	struct room_prog *rp;
	AFFECTED_TYPE *af;
	int j;

	if (!troom->description)
		troom->description = duplicateString ("No Description Set\n");

	/*
	char query_format[MAX_STRING_LENGTH] = "";
	sprintf (query_format,
	"INSERT INTO shadows_worldfile.bp_rooms "
	"(rid, zone, terrain, flags, name) "
	"VALUES ('%d', '%d', '%d', '%lu', '%%s') "
	"ON DUPLICATE KEY UPDATE "
	"zone = '%d', terrain = '%d', flags = '%lu', name = '%%s'",
	troom->nVirtual,troom->zone,troom->sector_type + 1,
	(((unsigned long)troom->room_flags)),
	troom->zone,troom->sector_type + 1,
	(((unsigned long)troom->room_flags)));


	mysql_safe_query (query_format,
	troom->name,
	troom->name);
	*/


	fprintf (fp, "#%d\n%s~\n%s~\n",
		troom->nVirtual, troom->name, troom->description);
	fprintf (fp, "%d %d %d\n",
		troom->zone, troom->room_flags, troom->sector_type);

	if (world_version_out >= 1)
		fprintf (fp, "%d\n", troom->deity);

	if (troom->extra)
	{

		fprintf (fp, "A\n");

		for (j = 0; j < WR_DESCRIPTIONS; j++)
			fprintf (fp, "%s~\n", troom->extra->weather_desc[j] ?
			troom->extra->weather_desc[j] : "");

		for (j = 0; j <= 5; j++)
			fprintf (fp, "%s~\n", troom->extra->alas[j] ?
			troom->extra->alas[j] : "");
	}

	for (j = 0; j <= 5; j++)
	{

		if (!troom->dir_option[j])
			continue;

		if (IS_SET (troom->dir_option[j]->exit_info, EX_SECRET))
			if (IS_SET (troom->dir_option[j]->exit_info, EX_TRAP))
				fprintf (fp, "B%d\n", j);
			else
				fprintf (fp, "H%d\n", j);

		else if (IS_SET (troom->dir_option[j]->exit_info, EX_TRAP))
			fprintf (fp, "T%d\n", j);
		else
			fprintf (fp, "D%d\n", j);

		if (troom->dir_option[j]->general_description)
			fprintf (fp, "%s~\n", troom->dir_option[j]->general_description);
		else
			fprintf (fp, "~\n");

		if (troom->dir_option[j]->keyword)
			fprintf (fp, "%s~\n", troom->dir_option[j]->keyword);
		else
			fprintf (fp, "~\n");

		if (IS_SET (troom->dir_option[j]->exit_info, EX_ISGATE))
			fprintf (fp, "3");
		else if (IS_SET (troom->dir_option[j]->exit_info, EX_PICKPROOF))
			fprintf (fp, "2");
		else if (IS_SET (troom->dir_option[j]->exit_info, EX_LOCKED))
			fprintf (fp, "1");
		else if (IS_SET (troom->dir_option[j]->exit_info, EX_ISDOOR))
			fprintf (fp, "1");
		else
			fprintf (fp, "0");

		fprintf (fp, " %d ", troom->dir_option[j]->key);

		if (world_version_out >= 1)
			fprintf (fp, " %d ", troom->dir_option[j]->pick_penalty);

		if (troom->dir_option[j]->to_room == NOWHERE)
			fprintf (fp, "-1\n");
		else
			fprintf (fp, "%d\n", troom->dir_option[j]->to_room);
	}

	for (j = 0; j <= 5; j++)
	{
		if (troom->secrets[j])
		{
			fprintf (fp, "Q%d\n%d\n", j, troom->secrets[j]->diff);
			fprintf (fp, "%s~\n", troom->secrets[j]->stext);
		}
	}

	if (troom->noInvLimit)
		fprintf (fp, "U\n");

	for (exptr = troom->ex_description; exptr; exptr = exptr->next)
		if (exptr->description)
			fprintf (fp, "E\n%s~\n%s~\n", exptr->keyword, exptr->description);

	if (troom->wdesc)
		fprintf (fp, "W\n%d\n%s~\n",
		troom->wdesc->language, troom->wdesc->description);

	if (troom->prg)
	{
		for (rp = troom->prg; rp; rp = rp->next)
		{
			fprintf (fp, "P\n");
			fprintf (fp, "%s~\n", rp->command);
			fprintf (fp, "%s~\n", rp->keys);
			fprintf (fp, "%s~\n", rp->prog);
		}
	}

	if (troom->capacity)
	{
		fprintf (fp, "C\n");
		fprintf (fp, "%d\n", troom->capacity);
	}

		//r for affects, since it is written to a separate file for resets
	if (troom->affects)
	{
		for (af = troom->affects; af; af = af->next)
		{
			fprintf(fp, "r\n");
			fprintf (fp, "%d %d %d\n", 
					 af->type,
					 af->a.room.duration,
					 af->a.room.intensity);
		}
	}
 
	fprintf (fp, "S\n");
}

void
save_affect_reset (FILE * fp, CHAR_DATA * tmp_mob, AFFECTED_TYPE * af)
{
	CHAR_DATA *proto;

	if (af->type == MAGIC_DRAGGER || af->type == MAGIC_WATCH1 ||
		af->type == MAGIC_WATCH2 || af->type == MAGIC_WATCH3 ||
		af->type == MAGIC_NOTIFY || af->type == MAGIC_GUARD ||
		af->type == MAGIC_SIT_TABLE || af->type == AFFECT_SHADOW ||
		af->type == MAGIC_CLAN_NOTIFY)
		return;

	if (af->type == MAGIC_ROOM_FIGHT_NOISE)
		return;

	if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
	{
		fprintf (fp, "C '%s'\n", af->a.craft->subcraft->subcraft_name);
		return;
	}

	proto = vtom (tmp_mob->mob->nVirtual);

	if (af->type == MAGIC_AFFECT_INFRAVISION &&
		IS_SET (tmp_mob->affected_by, AFF_INFRAVIS))
		return;

	if (af->type == MAGIC_AFFECT_INVISIBILITY &&
		IS_SET (tmp_mob->affected_by, AFF_INVISIBLE))
		return;

	if (af->type == MAGIC_HIDDEN && IS_SET (tmp_mob->affected_by, AFF_HIDE))
		return;

	if ((af->type == MAGIC_ROOM_SHADOW) || (af->type == MAGIC_ROOM_ILUVATAR))
	{
		fprintf(fp, "r %d %d %d\n", 
					 af->type,
					 af->a.room.duration,
					 af->a.room.intensity);
	}
	
	fprintf (fp, "A %d %d %d %d %d %d %d\n",
		af->type,
		af->a.spell.duration,
		af->a.spell.modifier,
		af->a.spell.location,
		af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);
}

void
fwrite_resets (ROOM_DATA * troom, FILE * fp)
{
	CHAR_DATA *tmp_mob;
	OBJ_DATA *to;
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	RESET_DATA *reset;
	int j;
	int w;

	/* Write room header information if we need to write room affects */

	if (troom->affects)
		fprintf (fp, "R %d \n", troom->nVirtual);

	for (af = troom->affects; af; af = af->next)
	{
		if (af->type == MAGIC_ROOM_FIGHT_NOISE)
			continue;
		
		if ((af->type == MAGIC_ROOM_SHADOW) || (af->type == MAGIC_ROOM_ILUVATAR))
		{
			fprintf(fp, "r %d %d %d 0 0 0 0\n", 
					af->type,
					af->a.room.duration,
					af->a.room.intensity);
		}
		else
		{
			fprintf (fp, "r %d %d %d %d %d %d %d \n",
			af->type,
			af->a.spell.duration,
			af->a.spell.modifier,
			af->a.spell.location,
			af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);
	}
	}

	for (j = 0; j <= LAST_DIR; j++)
	{

		if (!troom->dir_option[j])
			continue;

		if (IS_SET (troom->dir_option[j]->exit_info, EX_PICKPROOF))
			fprintf (fp, "D 0 %d %d 2\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->exit_info, EX_LOCKED))
			fprintf (fp, "D 0 %d %d 2\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->exit_info, EX_ISDOOR))
			fprintf (fp, "D 0 %d %d 1\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->exit_info, EX_ISGATE))
			fprintf (fp, "D 0 %d %d 3\n", troom->nVirtual, j);
	}

	if (troom->people)
	{
		for (tmp_mob = troom->people; tmp_mob; tmp_mob = tmp_mob->next_in_room)
		{

			if (!IS_NPC (tmp_mob))
				continue;

			if (IS_SET (tmp_mob->act, ACT_STAYPUT)
				|| IS_SET (tmp_mob->act, ACT_MOUNT))
				continue;

			fprintf (fp, "M 0 %d 0 %d %d\n", tmp_mob->mob->nVirtual,
				troom->nVirtual,
				tmp_mob->mount ? tmp_mob->mount->mob->nVirtual : 0);

			if (tmp_mob->right_hand)
			{
				to = tmp_mob->right_hand;
				if (to->nVirtual == VNUM_PENNY || to->nVirtual == VNUM_FARTHING)
					fprintf (fp, "G 1 %d %d\n", to->nVirtual, to->count);
				else
					fprintf (fp, "G 1 %d 0\n", to->nVirtual);
				for (to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->nVirtual,
					to->in_obj->nVirtual);
			}

			if (tmp_mob->left_hand)
			{
				to = tmp_mob->left_hand;
				if (to->nVirtual == VNUM_PENNY || to->nVirtual == VNUM_FARTHING)
					fprintf (fp, "G 1 %d %d\n", to->nVirtual, to->count);
				else
					fprintf (fp, "G 1 %d 0\n", to->nVirtual);
				for (to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->nVirtual,
					to->in_obj->nVirtual);
			}

			for (w = 0; w < MAX_WEAR; w++)
			{
				if (!get_equip (tmp_mob, w))
					continue;

				obj = get_equip (tmp_mob, w);
				fprintf (fp, "E 1 %d 0 %d\n", obj->nVirtual, w);
				if (obj->contains
					&& (GET_ITEM_TYPE (obj) == ITEM_CONTAINER
					|| GET_ITEM_TYPE (obj) == ITEM_KEYRING
					|| GET_ITEM_TYPE (obj) == ITEM_SHEATH
					|| GET_ITEM_TYPE (obj) == ITEM_QUIVER))
					fprintf (fp, "s 1 %d %d\n", obj->contains->nVirtual,
					obj->contains->count);
			}

			for (af = tmp_mob->hour_affects; af; af = af->next)
				save_affect_reset (fp, tmp_mob, af);

			for (reset = tmp_mob->mob->resets; reset; reset = reset->next)
				if (reset->type == RESET_REPLY)
					fprintf (fp, "m %d %s\n", reset->type, reset->command);
				else
					system_log ("ATTEMPT TO SAVE UNKNOWN KIND OF RESET", true);

			if (IS_RIDER (tmp_mob))
			{
				for (w = 0; w < MAX_WEAR; w++)
				{
					if (!get_equip (tmp_mob->mount, w))
						continue;

					obj = get_equip (tmp_mob->mount, w);
					fprintf (fp, "a 1 %d 0 %d\n", obj->nVirtual, w);
					if (obj->contains
						&& (GET_ITEM_TYPE (obj) == ITEM_CONTAINER
						|| GET_ITEM_TYPE (obj) == ITEM_KEYRING
						|| GET_ITEM_TYPE (obj) == ITEM_SHEATH
						|| GET_ITEM_TYPE (obj) == ITEM_QUIVER))
						fprintf (fp, "s 1 %d %d\n", obj->contains->nVirtual,
						obj->contains->count);
				}
			}
		}
	}
}

void
do_zsave (CHAR_DATA * ch, char *arg, int cmd)
{
	int num, stat, i;

	if (engine.in_play_mode () && !engine.in_test_mode())
	{
		send_to_char ("This command cannot be used on the player port.\n", ch);
		return;
	}

	if (!str_cmp (arg, "dwellings"))
	{
		save_dwelling_rooms ();
		send_to_char ("Dwelling rooms saved.\n", ch);
		return;
	}

	if (!str_cmp (arg, "all"))
	{
	  //save objects only for zones 100-109
	  for (i = 100; i<110;i++)
	    {
	      save_objs(ch,i);
	    }
		for (i = 0; i <=99; i++)
		{
			if ((cmd != 226) && (IS_SET (zone_table[i].flags, Z_LOCKED)))
				continue;
			save_rooms (ch, i);
		}
		if (!engine.in_build_mode ())
		{
			i = 99;
			if ((cmd != 226) && (IS_SET (zone_table[i].flags, Z_LOCKED)))
				;
			else
				save_rooms (ch, i);
		}

		if (engine.in_build_mode ())
		{
			system ("../scripts/backup");
		}

		save_mob_progs();
		save_obj_progs();
		update_crafts_file ();

		send_to_char ("All in-game zones have been saved.\n", ch);
		if (engine.in_build_mode ())
		{
			std::ostringstream oss;
			oss << "cd " << engine.get_base_path() << "/regions; git commit -a -m \"Changes committed by ";
			if (!ch || !ch->tname)
				oss << "System";
			else
				oss << ch->tname;
			oss << " via zsave all\"";
			system(oss.str().c_str());
			//send_to_char(oss.str().c_str(),ch);
		}
		return;
	}

	for (; isspace (*arg); arg++);

	if (!isdigit (*arg))
	{
		send_to_char ("That is not a valid zone number.\n", ch);
		return;
	}

	num = atoi (arg);

	if (num > 109 || num < 0)
	{
		send_to_char ("You must specify a zone between 0 and 109.\n", ch);
		return;
	}

	if (num == 99 && engine.in_build_mode ())
	{
		send_to_char
			("Sorry, but this zone can't be saved on the builder port.\n", ch);
		return;
	}

	if ((cmd != 226) && (IS_SET (zone_table[num].flags, Z_LOCKED)))
	{
		send_to_char ("That zone is locked. Please try zsaving later.\n", ch);
		return;
	}

	if (num < 0 || num > 109)
	{
		send_to_char ("The zone must be between 0 and 109.\n", ch);
		return;
	}
	save_mob_progs();
	save_obj_progs();
	stat = save_rooms (ch, num);
	if (!stat)
	{
		send_to_char ("Saved.\n", ch);
		if (engine.in_build_mode ())
		{
			std::ostringstream oss;
			oss << "cd " << engine.get_base_path() << "/regions; git add *."
				<< num << " ; git commit -m \"Changes committed by ";
			if (!ch || !ch->tname)
				oss << "System";
			else
				oss << ch->tname;
			oss << " via zsave " << num << "\"";
			system(oss.str().c_str());
			//send_to_char(oss.str().c_str(),ch);
		}
	}
	else
	{
		send_to_char
			("Zone save failed - you should #2SHUTDOWN DIE#0 as soon as possible.\n",
			ch);
	}

	return;

}

FILE *
open_and_rename (CHAR_DATA * ch, char *name, int zone)
{
	char buf[MAX_STRING_LENGTH];
	FILE *fp;

	sprintf (buf, "%s/%s.%d", REGIONS, name, zone);

	if ((fp = fopen (buf, "w")) == NULL)
		return NULL;

	return fp;
}



int save_objs (CHAR_DATA * ch, int zone)
{
  OBJ_DATA *tobj;
  int tmp;
  FILE *fo;

  if (!(fo = open_and_rename (ch, "objs", zone)))
		return 1;

  for (tobj = full_object_list; tobj; tobj = tobj->lnext)
	if (tobj->zone == zone && tobj->nVirtual != -1 && tobj->nVirtual != 42)
		if (!tobj->deleted || get_obj_in_list_num (tobj->nVirtual, object_list))
		{
			/* Remove clan information from extra flags before
			   saving to disk...put it back afterwords */

			tmp = tobj->obj_flags.extra_flags;
			tobj->obj_flags.extra_flags &= ~(ITEM_LEADER | ITEM_MEMBER | ITEM_OMNI);
			fwrite_object (tobj, fo);
			tobj->obj_flags.extra_flags = tmp;
		}

  fprintf (fo, "$~\n");
  fclose(fo);
}

int
save_rooms (CHAR_DATA * ch, int zone)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *tmob;
	OBJ_DATA *tobj;
	ROOM_DATA *troom;
	FILE *magic;
	FILE *fz;
	FILE *fm;
	FILE *fr;
	int elemental = 0, znum = 0;
	int shadow = 0, holy = 0, psychic = 0;
	int room_good;
	int n;
	int tmp;
	int empty_rooms = 0;
	static int total_empty_rooms = 0;

	sprintf (buf, "Saving rooms in zone %d.", zone);
	system_log (buf, false);


	//save objects only if above zone 99
	if (zone>99)
	  return save_objs(ch, zone);

	
	if (!(fr = open_and_rename (ch, "rooms", zone)))
		return 1;

	if (!(fz = open_and_rename (ch, "resets", zone)))
		return 1;

	if (!(fm = open_and_rename (ch, "mobs", zone)))
		return 1;
	


	
	fprintf (fz, "#%d\nLead: %s~\n%s~\n%d %d %d %ld %d %d\n",
		zone,
		zone_table[zone].lead,
		zone_table[zone].name,
		zone_table[zone].top,
		zone_table[zone].lifespan,
		zone_table[zone].reset_mode,
		zone_table[zone].flags,
		zone_table[zone].jailer,
		zone_table[zone].jail_room ?
		zone_table[zone].jail_room->nVirtual : 0);

	*buf = '\0';

	if ((magic = fopen ("../regions/zone_magic_grid", "r")))
	{
		fscanf (magic, "Zone %d: %d %d %d %d\n", &znum, &elemental, &psychic,
			&holy, &shadow);
		if (znum != zone)
			sprintf (buf, "Zone %d: %d %d %d %d\n", znum, elemental, psychic,
			holy, shadow);
		fclose (magic);
	}

	if (zone_table[zone].earth_mod || zone_table[zone].wind_mod
		|| zone_table[zone].fire_mod || zone_table[zone].water_mod
		|| zone_table[zone].shadow_mod)
		sprintf (buf + strlen (buf), "Zone %d: %d %d %d %d %d\n", zone,
		zone_table[zone].earth_mod, zone_table[zone].wind_mod,
		zone_table[zone].fire_mod, zone_table[zone].water_mod,
		zone_table[zone].shadow_mod);

	if ((magic = fopen ("../regions/zone_magic_grid", "w+")))
	{
		fprintf (magic, "%s", buf);
		fclose (magic);
	}

	for (troom = full_room_list; troom; troom = troom->lnext)
		if (troom->zone == zone)
		{

			room_good = 0;

			for (n = 0; n <= LAST_DIR; n++)
				if (troom->dir_option[n] && troom->dir_option[n]->to_room > 0)
					room_good = 1;

			if (troom->contents || troom->people)
				room_good = 1;

			if (strncmp (troom->description, "No Description Set", 18))
				room_good = 1;

			if (room_good)
			{
				fwrite_room (troom, fr);
				fwrite_resets (troom, fz);
			}
			else
			{
				empty_rooms++;
				total_empty_rooms++;
			}
		}

		if (empty_rooms)
		{
			sprintf (buf, "%d empty rooms were not saved for zone %d.",
				empty_rooms, zone);
			system_log (buf, false);

			strcat (buf, "\n");
			send_to_char (buf, ch);
		}

		for (tmob = full_mobile_list; tmob; tmob = tmob->mob->lnext)
			if (tmob->mob->zone == zone && !tmob->deleted
				&& !IS_SET (tmob->act, ACT_STAYPUT))
				fwrite_mobile (tmob, fm);
	 

		
				fprintf (fr, "$~\n");
				fprintf (fm, "$~\n");
				fprintf (fz, "S\n");

				fclose (fr);
				fclose (fz);
				fclose (fm);
				
				//objects pieced out - July 2010 Grommit
				save_objs(ch,zone);

				return 0;
}

void
do_rclone (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *source_room;
	ROOM_DATA *target_room;
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	argument = one_argument (argument, buf1);

	if (!str_cmp (buf1, "block"))
	{
		sprintf (buf1, "Done; start room is at %d.\n",
			clone_contiguous_rblock (ch->room, -1));
		send_to_char (buf1, ch);
		return;
	}

	if (!atoi (buf1))
	{
		send_to_char ("You must supply a virtual room number.\n", ch);
		return;
	}

	argument = one_argument (argument, buf2);

	if (!atoi (buf2))
		target_room = ch->room;

	else
	{
		if (!(target_room = vtor (atoi (buf2))))
		{
			send_to_char ("No such target room.\n", ch);
			return;
		}
	}

	if (!(source_room = vtor (atoi (buf1))))
	{
		send_to_char ("No such source room.\n", ch);
		return;
	}

	clone_room (source_room, target_room, false);

	send_to_char ("Done.\n", ch);
}

void
do_rdoor (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[256], buf2[256];
	int dir;
	ROOM_DATA *room;

	half_chop (argument, buf1, buf2);

	switch (*buf1)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	room = vtor (ch->in_room);

	if (!room->dir_option[dir])
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	room->dir_option[dir]->exit_info |= EX_ISDOOR;

	if (!*buf2)
		strcpy (buf2, "door");

	room->dir_option[dir]->keyword = duplicateString (buf2);

	send_to_char ("Done.\n", ch);
}

void
do_rgate (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[256], buf2[256];
	int dir;
	ROOM_DATA *room;

	half_chop (argument, buf1, buf2);

	switch (*buf1)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	room = vtor (ch->in_room);

	if (!room->dir_option[dir])
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	room->dir_option[dir]->exit_info |= EX_ISGATE;

	if (!*buf2)
		strcpy (buf2, "gate");

	room->dir_option[dir]->keyword = duplicateString (buf2);

	send_to_char ("Done.\n", ch);
}

/*                                                                          *
* funtion: do_review                    < e.g.> review <name>              *
*                                                                          *
* 09/17/2004 [JWW] - Fixed an instances where mysql result was not freed   *
*                                                                          */
void
do_review (CHAR_DATA * ch, char *argument, int cmd)
{
	int silent_review = 0;
	CHAR_DATA *review_ch;
	MYSQL_RES *result;
	char buf[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];

	argument = one_argument (argument, name);

	if (IS_NPC (ch))
	{
		send_to_char ("You must RETURN before reviewing any applications.\n",
			ch);
		return;
	}

	if (engine.in_build_mode ())
	{
		send_to_char ("Applications may only be reviewed on the player port.\n",
			ch);
		return;
	}

	if (!isalpha (*name))
	{
		send_to_char ("Illegal name.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf == '!')
		silent_review = 1;

	if (!(review_ch = load_pc (name)))
	{
		send_to_char ("There is no pfile associated with that name, "
			"sorry.\n", ch);
		return;
	}

	if (IS_MORTAL (ch)
		&& (!is_newbie (review_ch) || IS_SET (review_ch->plr_flags, PRIVATE)))
	{
		send_to_char
			("There is no playerfile associated with that name, sorry.\n", ch);
		unload_pc(review_ch);
		return;
	}

	if (review_ch->pc->create_state != 1)
	{
		sprintf (buf,
			"It appears that this application has already been reviewed and responded to.\n");
		send_to_char (buf, ch);
		unload_pc (review_ch);
		return;
	}

	if (is_being_reviewed (review_ch->tname, ch->desc->acct->name.c_str ()))
	{
		sprintf (buf,
			"That application is currently checked out - try again later.\n");
		send_to_char (buf, ch);
		unload_pc (review_ch);
		return;
	}

	spitstat (review_ch, ch->desc);

	ch->delay_type = DEL_APP_APPROVE;
	ch->delay = 3 * 60;
	ch->delay_who = duplicateString (name);

	mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s'",
		review_ch->tname);

	if ((result = mysql_store_result (database)) != NULL)
	{
		if (mysql_num_rows (result) >= 1)
		{
			send_to_char
				("\n#6Please type HISTORY to review previous responses to this application.#0\n",
				ch);
		}
		mysql_free_result (result);
		result = NULL;
	}
	send_to_char ("\n#2Please ACCEPT or DECLINE this application.#0\n", ch);

	if (!is_yours (review_ch->tname, ch->desc->acct->name.c_str ()))
	{
		mysql_safe_query
			("INSERT INTO reviews_in_progress VALUES ('%s', '%s', UNIX_TIMESTAMP())",
			review_ch->tname, ch->desc->acct->name.c_str());
	}

	unload_pc (review_ch);
}

void
show_reg_by_name (CHAR_DATA * ch, char *reg_name)
{
	char buf[MAX_INPUT_LENGTH];
	int index;
	int count = 0;
	REGISTRY_DATA *reg;

	if (!*reg_name)
		strcpy (buf, "registry");
	else
		strcpy (buf, reg_name);

	send_to_char ("\n", ch);

	if ((index = lookup_value (buf, REG_REGISTRY)) == -1)
	{
		send_to_char ("No such registry.\n", ch);
		return;
	}

	reg = registry[index];

	for (reg = registry[index]; reg; reg = reg->next)
	{
		if (index == REG_MISC)
			sprintf (buf, "  %20s          %s\n",
			lookup_string (reg->value, REG_MISC_NAMES), reg->string);

		else if (index == REG_CAP || index == REG_OV || index == REG_LV)
			sprintf (buf, "  %20s          %s\n",
			lookup_string (reg->value, REG_SKILLS), reg->string);

		else
			sprintf (buf, "  %-15s%s", reg->string, ++count % 4 ? "" : "\n");

		send_to_char (buf, ch);
	}

	send_to_char ("\n", ch);
}

void
olist_show (OBJ_DATA * obj, int type, int header)
{
	char buf[MAX_STRING_LENGTH];
	char wear_loc[MAX_STRING_LENGTH];
	char armor_type[MAX_STRING_LENGTH];

	switch (type)
	{

	case ITEM_WEAPON:

		if (header)
			sprintf (b_buf + strlen (b_buf),
			"Virt#  Dam   Weight  Value Delay Qty 1st-key           Short Description\n");

		one_argument (obj->name, buf);

		sprintf (b_buf + strlen (b_buf),
			"%.5d%s%2dd%-2d %3d.%.2d  %7.2f  %3d  %4d  %-13.13s  %-23.23s#0\n",
			obj->nVirtual, *obj->full_description ? " " : "*",
			obj->o.weapon.dice, obj->o.weapon.sides,
			obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
			obj->farthings, obj->o.weapon.delay, obj->quality,
			buf, obj->short_description);

		break;

	case ITEM_ARMOR:

		if (header)
			sprintf (b_buf + strlen (b_buf),
			"Virt# AC Weight  Value Type   Wear-Flgs 1st-key        Short Description\n");

		one_argument (obj->name, buf);

		switch (obj->o.armor.armor_type)
		{
		case 0:
			strcpy (armor_type, "Quilt ");
			break;
		case 1:
			strcpy (armor_type, "Leath ");
			break;
		case 2:
			strcpy (armor_type, "Ring  ");
			break;
		case 3:
			strcpy (armor_type, "Scale ");
			break;
		case 4:
			strcpy (armor_type, "Mail  ");
			break;
		case 5:
			strcpy (armor_type, "Plate ");
			break;
		default:
			strcpy (armor_type, "????? ");
			break;
		}

		*wear_loc = '\0';

		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FINGER))
			strcat (wear_loc, "FING ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_NECK))
			strcat (wear_loc, "NECK ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BODY))
			strcat (wear_loc, "BODY ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
			strcat (wear_loc, "HEAD ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_LEGS))
			strcat (wear_loc, "LEGS ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FEET))
			strcat (wear_loc, "FEET ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HANDS))
			strcat (wear_loc, "HAND ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMS))
			strcat (wear_loc, "ARMS ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHIELD))
			strcat (wear_loc, "SHLD ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT))
			strcat (wear_loc, "ABOT ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WAIST))
			strcat (wear_loc, "WSTE ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST))
			strcat (wear_loc, "WRST ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ANKLE))
			strcat (wear_loc, "ANKL ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HAIR))
			strcat (wear_loc, "HAIR ");
		if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FACE))
			strcat (wear_loc, "FACE ");

		if (strlen (wear_loc) > 10)
		{
			wear_loc[14] = '+';
			wear_loc[15] = '\0';
		}

		else
			while (strlen (wear_loc) < 10)
				strcat (wear_loc, " ");

		sprintf (b_buf + strlen (b_buf),
			"%.5d%s%2d %3d.%.2d  %8.2f %s %s%-13.13s  %-21.21s#0\n",
			obj->nVirtual, *obj->full_description ? " " : "*",
			obj->o.armor.armor_value,
			obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
			obj->farthings, armor_type, wear_loc, buf,
			obj->short_description);

		break;

	case ITEM_DRINKCON:
	case ITEM_FOUNTAIN:
		if (header)
			sprintf (b_buf + strlen (b_buf),
			"Virt#     Weight ##drnk  Cap Contents                   Short Description\n");

		sprintf (b_buf + strlen (b_buf),
			"%.5d%s %5d.%02d %5d %5d %-25s    %s#0\n",
			obj->nVirtual,
			(obj->full_description && *obj->full_description) ? " " : "*",
			obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
			obj->o.drinkcon.volume,
			obj->o.drinkcon.capacity,
			!obj->o.drinkcon.volume ? "(empty)" :
			vtoo (obj->o.drinkcon.liquid) ?
			vtoo (obj->o.drinkcon.liquid)->short_description : "not set",
			obj->short_description);

		break;

	case ITEM_FOOD:
		if (header)
			sprintf (b_buf + strlen (b_buf),
			"Virt # Foodval bites spel1 spel2 spel3 spel4 Short Description\n");

		sprintf (b_buf + strlen (b_buf),
			"%.5d%s %5d %5d %5d %5d %5d %5d   %s#0\n",
			obj->nVirtual,
			(obj->full_description && *obj->full_description) ? " " : "*",
			obj->o.food.food_value,
			obj->o.food.bites,
			obj->o.food.spell1,
			obj->o.food.spell2,
			obj->o.food.spell3,
			obj->o.food.spell4, obj->short_description);
		break;
	case -1:
	default:
		sprintf (b_buf + strlen (b_buf),
			"%.5d%s %7.2f  %-25.25s  %-25.25s#0",
			obj->nVirtual, (obj->full_description
			&& *obj->full_description) ? " " : "*",
			obj->farthings, obj->name, obj->short_description);
		OBJ_DATA* filling = 0;
		if (obj->obj_flags.type_flag == ITEM_DRINKCON && obj->o.od.value[1]
		&& obj->o.od.value[2] && (filling = vtoo (obj->o.od.value[2])))
		{
			one_argument (filling->name, buf);
			sprintf (b_buf + strlen (b_buf), " #6(%.8s)#0", buf);
		}
		strcat (b_buf, "\n");
		break;
	}
}

void

mlist_show (std::string *output_string, CHAR_DATA *mob, bool header)

{

	std::string line;
	std::ostringstream conversion;



	if (header)

	{

		output_string->clear();

		output_string->append("#2+-------+-------------------------------+-------------------------------+--------+#0\n");

		output_string->append("#2|#0 Vnum  #2|#0 Keywords                      #2|#0 Short Description             #2|#0 Loaded #2|#0\n");

		output_string->append("#2+-------+-------------------------------+-------------------------------+--------+#0\n");

	}



	line.assign("#2| ");
	conversion << mob->mob->nVirtual;


	for (int i = 0, j = (5 - conversion.str().length()); i < j; i++)

	{

		line.append("0");

	}


	line.append(conversion.str());

	line.append(" #2|#0 ");

	line.append(mob->name, MIN((int) strlen(mob->name), 29));

	for (int i = 0, j = (30 - MIN((int) strlen(mob->name), 29)); i < j; i++)

	{

		line.append(" ");

	}

	line.append("#2|#0 ");

	line.append(mob->short_descr, MIN((int) strlen(mob->short_descr), 29));

	for (int i = 0, j = (30 - MIN((int) strlen(mob->short_descr), 29)); i < j; i++)

	{

		line.append(" ");

	}

	line.append("#2|#0 ");

	int loads = 0;

	//for (CHAR_DATA *ch = character_list; ch; ch = ch->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)

	{
		CHAR_DATA *ch = *tch_iterator;

		if (ch->deleted || !IS_NPC(ch))

			continue;



		if (ch->mob->nVirtual == mob->mob->nVirtual)

			loads++;

	}
	std::ostringstream conversion2;
	conversion2 << loads;

	line.append(conversion2.str());

	for (int i = 0, j = (7-conversion2.str().length()); i < j; i++)

	{

		line.append(" ");

	}

	line.append("#2|#0\n");
	while (line.find_first_of('\0') != std::string::npos)
	{
		line.erase(line.find_first_of('\0'), 1);
	}



	output_string->append(line);

	return;

}



void

do_mlist (CHAR_DATA *ch, char *input_argument, int cmd)

{

	std::string buf, key1, key2, key3, clan, argument;

	int race = -1, zone = -1, clan_flags;

	bool yes_key1 = false, yes_key2 = false, yes_key3 = false, inclusive = false;

	CHAR_DATA * mob;

	argument.assign(input_argument);



	argument = one_argument (argument, buf);



	if (buf.empty())

	{

		send_to_char ("Selection Parameters:\n\n", ch);

		send_to_char ("    +/-<mobile keyword>     Include/exclude mobile keyword.\n", ch);

		send_to_char ("    <zone>                  Mobiles from specified zone only.\n", ch);

		send_to_char ("    '<race>'                Only mobiles of specified race.\n", ch);

		send_to_char ("    *clan                   Only mobiles of specified clan.\n", ch);

		send_to_char ("\n\nExample:    mlist +stocky -pale 'Fallohide Hobbit' *bakers_fellowship\n", ch);

		send_to_char ("\t...would list all stocky, non-pale Fallohide Hobbits who were in the Fellowship of Bakers.\n", ch);

		return;

	}



	while (!(buf.empty()))

	{

		inclusive = true;

		race = lookup_race_id (buf.c_str());

		if (buf.length() > 1 && isalpha(buf[0]) && race != -1)

		{

			argument = one_argument (argument, buf);

			continue;

		}

		if (isdigit(buf[0]))

		{

			if ((zone = atoi(buf.c_str())) >= MAX_ZONE)

			{

				send_to_char("Zone not in range 0...99\n", ch);

				return;

			}



			argument = one_argument (argument, buf);

			continue;

		}



		switch (buf[0])

		{

		case '-':

			inclusive = 0;

		case '+':

			if (!buf[1])

			{

				send_to_char ("Expected a keyword after (#2+/-#0).\n", ch);

				return;

			}

			if (key1.empty())

			{

				yes_key1 = inclusive;

				buf.erase(0, 1);

				key1 = buf;

			}

			else if (key2.empty())

			{

				yes_key2 = inclusive;

				buf.erase(0, 1);

				key2 = buf;

			}

			else if (!key3.empty())

			{

				send_to_char ("Sorry, at most three keywords.\n", ch);

				return;

			}

			else

			{

				yes_key3 = inclusive;

				buf.erase(0, 1);

				key3 = buf;

			}

			break;

		case '*':

			if (!buf[1])

			{

				send_to_char ("Expected a keyword after (#2*#0).\n", ch);

				return;

			}



			buf.erase(0, 1);

			clan = buf;

			break;

		}



		argument = one_argument (argument, buf);

	}



	std::string out_string;

	int count = 0;

	bool header = true;



	for (mob = full_mobile_list; mob; mob = mob->mob->lnext)

	{

		if (zone != -1 && mob->mob->zone != zone)

			continue;



		if (race != -1 && mob->race != race)

			continue;



		if (!clan.empty() && !get_clan (mob, clan.c_str(), &clan_flags))

			continue;



		if (!key1.empty())

		{

			if (yes_key1 && !isname(key1.c_str(), mob->name))

				continue;

			else if (!yes_key1 && isname(key1.c_str(), mob->name))

				continue;

		}



		if (!key2.empty())

		{

			if (yes_key2 && !isname(key2.c_str(), mob->name))

				continue;

			else if (!yes_key2 && isname(key2.c_str(), mob->name))

				continue;

		}



		if (!key3.empty())

		{

			if (yes_key3 && !isname(key3.c_str(), mob->name))

				continue;

			else if (!yes_key3 && isname(key3.c_str(), mob->name))

				continue;

		}



		count++;



		if (count < 200)

			mlist_show (&out_string, mob, header);



		header = false;



	}

	if (count == 0)
	{
		send_to_char("No results found.", ch);
		return;
	}



	if (count > 200)

	{

		buf.assign("You have selected #6");
		std::ostringstream conversion;
		conversion << count;

		buf.append(conversion.str());

		buf.append("#0 mobiles, which is too many to display.\n");

		send_to_char(buf.c_str(), ch);

		return;

	}



	buf.assign("Searching:");



	if (!clan.empty())

	{

		CLAN_DATA *clan_def = NULL;

		if ((clan_def = get_clandef (clan.c_str())))
		{
			buf.append(" Clan [#6");

			buf.append(clan_def->literal);

			buf.append("#0]");
		}
		else
		{
			buf.append(" Clan [#6");
			buf.append(clan);
			buf.append("#1 (Unregistered)#0]");
		}



	}



	if (zone != -1)

	{


		buf.append(" Zone [#6");
		std::ostringstream conversion;
		conversion << zone;

		buf.append(conversion.str());

		buf.append("#0]");

	}



	if (race != -1)

	{

		buf.append(" Race [#6");

		buf.append(lookup_race_variable(race, RACE_NAME));

		buf.append("#0]");

	}



	if (!key1.empty())

	{



		if (key2.empty())

		{

			buf.append(" Keyword [");

		}

		else

		{

			buf.append(" Keywords [");

		}
		if (yes_key1)
			buf.append("#2");
		else
			buf.append("#1");



		buf.append(key1);
		buf.append("#0");



		if (!key2.empty())

		{

			buf.append(" ");
			if (yes_key2)
				buf.append("#2");
			else
				buf.append("#1");


			buf.append(key2);
			buf.append("#0");

		}



		if (!key3.empty())

		{

			buf.append(" ");
			if (yes_key3)
				buf.append("#2");
			else
				buf.append("#1");


			buf.append(key3);

		}



		buf.append ("#0]");

	}
	buf.append("\n");



	send_to_char(buf.c_str(), ch);



	out_string.append("#2+-------+-------------------------------+-------------------------------+--------+#0\n");

	page_string (ch->desc, out_string.c_str());





}

void
do_olist (CHAR_DATA * ch, char *argument, int cmd)
{
	int header = 1;
	int type = -1;
	int inclusive;
	int zone = -1;
	int yes_key1 = 0;
	int yes_key2 = 0;
	int yes_key3 = 0;
	int count = 0;
	OBJ_DATA *obj;
	char key1[MAX_STRING_LENGTH] = { '\0' };
	char key2[MAX_STRING_LENGTH] = { '\0' };
	char key3[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Selection Parameters:\n\n", ch);
		send_to_char
			("   +/-<object keyword>       Include/exclude object keyword.\n",
			ch);
		send_to_char ("   <zone>                    Objects from zone only.\n",
			ch);
		send_to_char
			("   <item-type>               Include items of item-type.\n", ch);
		send_to_char ("\nExample:   olist +sword -rusty weapon 10\n", ch);
		send_to_char
			("will only get non-rusty swords of type weapon from zone 10.\n", ch);
		return;
	}

	while (*buf)
	{

		inclusive = 1;

		if (strlen (buf) > 1 && isalpha (*buf) &&
			(type = index_lookup (item_types, buf)) != -1)
		{
			argument = one_argument (argument, buf);
			continue;
		}

		if (isdigit (*buf))
		{

			if ((zone = atoi (buf)) >= OBJECT_MAX_ZONE)
			{
				send_to_char ("Zone not in range 0..110\n", ch);
				return;
			}

			argument = one_argument (argument, buf);
			continue;
		}

		switch (*buf)
		{

		case '-':
			inclusive = 0;
		case '+':

			if (!buf[1])
			{
				send_to_char ("Expected keyname after 'k'.\n", ch);
				return;
			}

			if (!*key1)
			{
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			}
			else if (!*key2)
			{
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			}
			else if (*key3)
			{
				send_to_char ("Sorry, at most three keywords.\n", ch);
				return;
			}
			else
			{
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;

		case 'z':

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || atoi (buf) >= OBJECT_MAX_ZONE)
			{
				send_to_char ("Expected valid zone after 'z'.\n", ch);
				return;
			}

			zone = atoi (buf);

			break;
		}

		argument = one_argument (argument, buf);
	}

	*b_buf = '\0';

	for (obj = full_object_list; obj; obj = obj->lnext)
	{

		if (zone != -1 && obj->zone != zone)
			continue;

		if (type != -1 && obj->obj_flags.type_flag != type)
			continue;

		if (*key1)
		{
			if (yes_key1 && !isname (key1, obj->name))
				continue;
			else if (!yes_key1 && isname (key1, obj->name))
				continue;
		}

		if (*key2)
		{
			if (yes_key2 && !isname (key2, obj->name))
				continue;
			else if (!yes_key2 && isname (key2, obj->name))
				continue;
		}

		if (*key3)
		{
			if (yes_key3 && !isname (key3, obj->name))
				continue;
			else if (!yes_key3 && isname (key3, obj->name))
				continue;
		}

		count++;

		if (count < 200)
			olist_show (obj, type, header);

		header = 0;
	}

	if (count > 200)
	{
		sprintf (buf, "You have selected %d objects (too many to print).\n",
			count);
		send_to_char (buf, ch);
		return;
	}

	page_string (ch->desc, b_buf);
}

/*                                                                          *
* funtion: do_show                      < e.g.> show (k|a|l|v|m|o|q|r|u|z) *
*                                                                          *
* 09/17/2004 [JWW] - asserted that MYSQL_RES objects were reset to NULL    *
*                  - changed two asctime calls to asctime_r                *
*                  - added an include statement for the asctime_r calls    *
*                                                                          */
void
do_show (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d;
	char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH],
		buf3[MAX_STRING_LENGTH], buf4[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];
	std::ostringstream stream;
	char *date;
	int mtop = 0, count, count2, otop = 0, n, i;
	int count_total_rooms, docs = 0;
	bool found = false;
	OBJ_DATA *k;
	CHAR_DATA *j;
	ROOM_DATA *troom;
	SUBCRAFT_HEAD_DATA *craft;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int mat_only = 0;
	int keepers_only = 0;
	int got_line = 0;
	int ok_only = 0, craft_tot = 0;
	int big_counts_only = 0;
	int undesc_only = 0;
	int stayput = 0, chars = 0, accounts = 0;
	int search_type = 0;
	char *obj_name;
	char *mob_name;
	char *zone_str;
	int nSearchZone = -1;
	char buf[MAX_STRING_LENGTH];
	time_t current_time;
	int timeframe = 0;  // 0 - all, 1 = day, 2 - week, 3 - month

	arg_splitter (4, argument, buf1, buf2, buf3, buf4);

	if (IS_MORTAL (ch) && !IS_NPC (ch) && !ch->pc->is_guide)
	{
		send_to_char ("Eh?\n", ch);
		return;
	}

	if (ch->pc->is_guide && IS_MORTAL (ch))
	{
		if (*buf1 != 'l' && *buf1 != 'L')
		{
			send_to_char
				("Type SHOW L to get a list of applications in the queue.\n", ch);
			return;
		}
	}

	if  (GET_TRUST (ch) < 2)
	{
		if (*buf1 != 'a' &&
			*buf1 != 'i' &&
			*buf1 != 'o' &&
			*buf1 != 'q' &&
			*buf1 != 'r' &&
			*buf1 != 'u' &&
			*buf1 != 'x' &&
			*buf1 != 'z')

		{
			s ("   a           per zone stats");
			s ("   i           object inheritance");
			s ("   o           objects");
			s ("   q           objects with ok flag");
			s ("   r           rooms");
			s ("   u           undescribed rooms");
			s ("   x           objects without material flags");
			s ("   z           zones");
			return;
		}
	}

	if  (GET_TRUST (ch) < 3)
	{
		if (*buf1 != 'a' &&
			*buf1 != 'i' &&
			*buf1 != 'k' &&
			*buf1 != 'o' &&
			*buf1 != 'm' &&
			*buf1 != 'q' &&
			*buf1 != 'r' &&
			*buf1 != 'u' &&
			*buf1 != 'x' &&
			*buf1 != 'z')

		{
			s ("   a           per zone stats");
			s ("   i           object inheritance");
			s ("   k           shopkeepers");
			s ("   o           objects");
			s ("   m           mobiles");
			s ("   q           objects with ok flag");
			s ("   r           rooms");
			s ("   u           undescribed rooms");
			s ("   x           objects without material flags");
			s ("   z           zones");
			return;
		}
	}

	if  (GET_TRUST (ch) < 5)
	{
		if (*buf1 != 'a' &&
			*buf1 != 'c' &&
			*buf1 != 'i' &&
			*buf1 != 'k' &&
			*buf1 != 'l' &&
			*buf1 != 'm' &&
			*buf1 != 'o' &&
			*buf1 != 'p' &&
			*buf1 != 'q' &&
			*buf1 != 'r' &&
			*buf1 != 'u' &&
			*buf1 != 'x' &&
			*buf1 != 'z')

		{
			s ("   a           per zone stats");
			s ("   c           characters matching search");
			s ("   i           object inheritance");
			s ("   k           shopkeepers");
			s ("   l           applications");
			s ("   o           objects");
			s ("   m           mobiles");
			s ("   p           PCs with specified points");
			s ("   q           objects with ok flag");
			s ("   r           rooms");
			s ("   u           undescribed rooms");
			s ("   x           objects without material flags");
			s ("   z           zones");
			return;
		}
	}

	if (!*buf1)
	{
		s ("   a           per zone stats");
		s ("   c           characters matching search");
		s ("   i           object inheritance");
		s ("   k           shopkeepers");
		s ("   l           applications");
		s ("   m           mobiles");
		s ("   o           objects");
		s ("   p           PCs with specified points");
		s ("   q           objects with ok flag");
		s ("   r           rooms");
		s ("   u           undescribed rooms");
		s ("   v           registry."); //level 5
		s ("   x           objects without material flags");
		s ("   z           zones");
	}

	std::string player_db = engine.get_config ("player_db");
	switch (*buf1)
	{

	case 'v':			/* registry entry */
		show_reg_by_name (ch, buf2);
		break;

	case 'p':
		if (!*buf2)
		{
			send_to_char ("What did you wish the point criterion to be?\n", ch);
			return;
		}
		if (!isdigit (*buf2))
		{
			send_to_char ("Please specify a number of points.\n", ch);
			return;
		}
		*buf = '\0';
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->character && IS_MORTAL (d->character)
				&& (d->connected == CON_PLYNG)
				&& (d->character->in_room != NOWHERE) && d->acct
				&& d->acct->get_rpp () <= atoi (buf2)
				&& ((time (0) - d->acct->get_last_rpp_date ()) >= 60 * 60 * 24 * 60))
			{
				found = true;
				sprintf (output, "%-20s - %s [%d]\n",
					d->character->tname,
					vtor (d->character->in_room)->name,
					vtor (d->character->in_room)->nVirtual);
				if (strlen (output) + strlen (buf) >= MAX_STRING_LENGTH)
				{
					send_to_char
						("Too many matches found - please use a more selective criterion!\n",
						ch);
					return;
				}
				sprintf (buf + strlen (buf), "%s", output);
			}
		}

		if (found)
		{
			sprintf (output,
				"#2The following PCs online currently have %d roleplay points or less and\n"
				"have not been awarded or deducted within the last 60 days:#0\n\n%s",
				atoi (buf2), buf);
		}
		else
		{
			sprintf (output,
				"#1No PCs currently online have %d roleplay points or less who have not\n"
				"been awarded or deducted within the last 60 days.#0\n",
				atoi (buf2));
		}

		page_string (ch->desc, output);

		break;

	case 'l':			/* Pending applications */

		current_time = time (0);
		date = new char[256];
		date[0] = '\0';
		if (asctime_r (localtime (&current_time), date) != NULL)
		{
			date[strlen (date) - 1] = '\0';
		}
		sprintf (buf, "\nApplication Queue, as of #2%s#0:\n\n", date);
		free_mem (date);
		send_to_char (buf, ch);

		mysql_safe_query
			("SELECT name, "
			"HOUR(TIMEDIFF(NOW(), FROM_UNIXTIME(birth))) AS birth,"
			"account, "
			"plrflags, "
			"sdesc "
			"FROM %s.pfiles "
			"WHERE create_state = 1 "
			"ORDER BY birth DESC",
			player_db.c_str ());
		result = mysql_store_result (database);

		if (result)
		{
			if (mysql_num_rows (result))
			{
				sprintf (buf, "  #2%-15s#0  #2%-15s#0  #2%-8s#0  #2#0\n",
					IS_MORTAL (ch) ? "" : "Account", "Character",
					"In Queue");
				send_to_char (buf, ch);
			}
			while ((row = mysql_fetch_row (result)))
			{
				bool is_guide_rev = IS_MORTAL (ch);
				bool is_newb_app = is_newbie (row[2]);
				bool is_private_app = (strtol(row[3],0,10) & PRIVATE);

				if (is_guide_rev && (!is_newb_app || is_private_app))
					continue;

				got_line++;
				sprintf (buf,
					"  %-15s  %-15s  ",
					is_guide_rev ? "" : row[2],
					row[0]);
				current_time = strtol (row[1], NULL, 10);

				if (current_time >= 48)
					sprintf (buf + strlen (buf), "#1%3ldd %2ldh#0",
					current_time / 24, current_time % 24);
				else if (current_time >= 24)
					sprintf (buf + strlen (buf), "#3%3ldd %2ldh#0",
					current_time / 24, current_time % 24);
				else
					sprintf (buf + strlen (buf), "#2   %c %2dh#0",
					(!current_time) ? '<' : ' ',
					(int) MAX (1, (int)current_time));

				if (!IS_NPC (ch))
				{
					if (!is_guide_rev)
					{
						if (is_newb_app)
							sprintf (buf + strlen (buf), " #6(New)#0");

						if (is_private_app)
							sprintf (buf + strlen (buf), " #1(Private)#0");
					}
					// combine the two following sql calls into one
					if (is_yours (row[0], ch->pc->account_name))
						sprintf (buf + strlen (buf), " #3(Yours)#0");
					else if (is_being_reviewed (row[0], ch->pc->account_name))
						sprintf (buf + strlen (buf), " #2(Checked Out)#0");

				}
				strcat (buf, "\n");
				send_to_char (buf, ch);
			}

			mysql_free_result (result);
			result = NULL;
		}

		if (!got_line)
			send_to_char ("  None\n", ch);

		break;

	case 'k':
		keepers_only = 1;

	case 'm':			/* Mobile */

		mob_name = buf3;
		zone_str = NULL;

		if (*buf2 && !isdigit (*buf2))
			mob_name = duplicateString (buf2);
		else if (*buf2)
			zone_str = buf2;
		else
		{
			s("  You must specify a zone, keyword or both.");
			s("  'show m 3' to show mobiles in zone 3");
			s("  'show m zombie' to show all mobiles with zombie as a keyword");
			s("  'show m 3 zombie' to show all zombies in zone 3");
			return;
			}

		sprintf (tmp, "Vnum  K    Name(s)                            Short desc\n\n");
		for (j = full_mobile_list; j; j = j->mob->lnext)
		{

			if (!IS_NPC (j))
				continue;

			if (keepers_only && !j->shop)
				continue;

			if (zone_str && j->mob->zone != atoi (zone_str))
				continue;

			if (!*j->name)
				continue;

			if ((*mob_name && strstr (j->name, mob_name)) || !*mob_name)
			{
				if (IS_SET (j->hmflags, HM_KEEPER))
					sprintf (tmp + strlen (tmp),
					"%.5d * %-30.30s  %-28.28s\n",
					j->mob->nVirtual, j->name, j->short_descr);
				else
				{
					sprintf (tmp + strlen (tmp),
						"%.5d   %-30.30s  %-28.28s\n",
						j->mob->nVirtual, j->name, j->short_descr);
				}
				if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
				{
					send_to_char
					("Too many matches found - please use a more selective criterion!\n",
					 ch);
					return;
				}
			}
		}
		page_string (ch->desc, tmp);
		break;

	case '!':
		big_counts_only = 1;

	case 'q':			/* Object only with ok flag */
		ok_only = 1;

	case 'x':			/* Object without material flags */
		mat_only = 1;

	case 'o':			/* object */

		obj_name = duplicateString (buf3);
		zone_str = NULL;

		if (!isdigit (*buf2))
			obj_name = buf2;
		else
			zone_str = buf2;

		sprintf (tmp, "Vnum       Name(s)           Short desc"
			"               \tLocation\n\n");
		for (k = full_object_list; k; k = k->lnext)
		{

			if (zone_str && k->zone != atoi (zone_str))
				continue;

			if (ok_only && !IS_SET (k->obj_flags.extra_flags, ITEM_OK))
				continue;

			if (mat_only)
			{
				if (is_tagged (k->name))
					continue;
			}

			if (big_counts_only && k->count < 100)
				continue;

			if ((*obj_name && strstr (k->name, obj_name)) || !*obj_name)
			{
				if (!*k->name)
					continue;
				if (k->in_obj)
					sprintf (tmp + strlen (tmp),
					"%.5d  %-21.21s  %-28.28s#0  \t(I)%.5d\n",
					k->nVirtual, k->name, k->short_description,
					k->in_obj->nVirtual);

				else if (k->carried_by)
					sprintf (tmp + strlen (tmp),
					"%.5d  %-21.21s  %-28.28s#0  \t(C)%.5d\n",
					k->nVirtual, k->name, k->short_description,
					k->carried_by->mob ? k->carried_by->mob->nVirtual : 0);

				else if (k->equiped_by)
					sprintf (tmp + strlen (tmp),
					"%.5d  %-21.21s  %-28.28s#0  \t(E)%.5d\n",
					k->nVirtual, k->name, k->short_description, 
					k->equiped_by->mob ? k->equiped_by->mob->nVirtual : 0);

				else
					sprintf (tmp + strlen (tmp),
					"%.5d  %-21.21s  %-28.28s#0  \t(R) %05d\n",
					k->nVirtual, k->name, k->short_description, 
					k->in_room);
				
				if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
				{
					send_to_char
					("Too many matches found - please use a more selective criterion!\n",
					 ch);
					return;
				}
			}
		}
		page_string (ch->desc, tmp);
		break;

		case 'i':			/* object inheritance - show i 100008*/
			
			if (!isdigit (*buf2))
			{
				send_to_char ("Please specify a vnum.\n", ch);
				return;
			}
			
			sprintf (tmp, "Vnum       Name(s)           Short desc\n\n");
			for (k = full_object_list; k; k = k->lnext)
			{
				if (k->super_vnum == atoi(buf2))
					sprintf (tmp + strlen (tmp),
							 "%.5d  %-20.20s  %-32.32s#0\n",
							 k->nVirtual, k->name, k->short_description);
				
				if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
				{
					send_to_char
					("Too many matches found - please use a more selective criterion!\n",
					 ch);
					return;
				}
				
			}
			page_string (ch->desc, tmp);
			break;
			
	case 'c':
		if (!str_cmp (buf2, "keyword"))
			search_type = SEARCH_KEYWORD;
		else if (!str_cmp (buf2, "sdesc"))
			search_type = SEARCH_SDESC;
		else if (!str_cmp (buf2, "ldesc"))
			search_type = SEARCH_LDESC;
		else if (!str_cmp (buf2, "fdesc"))
			search_type = SEARCH_FDESC;
		else if (!str_cmp (buf2, "clan"))
			search_type = SEARCH_CLAN;
		else if (!str_cmp (buf2, "skill"))
			search_type = SEARCH_SKILL;
		else if (!str_cmp (buf2, "room"))
			search_type = SEARCH_ROOM;
		else if (!str_cmp (buf2, "race"))
			search_type = SEARCH_RACE;
		else if (!str_cmp (buf2, "stat"))
			search_type = SEARCH_STAT;
		else if (!str_cmp (buf2, "level") && GET_TRUST (ch) >= 5)
			search_type = SEARCH_LEVEL;
		else
		{
			if (GET_TRUST (ch) < 5)
				send_to_char
				("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, or room.\n",
				ch);
			else
				send_to_char
				("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, room, or level.\n",
				ch);
			return;
		}

		if (*buf4) {
			if (strcmp(buf4,"today") == 0) {
				timeframe = 1;
			}
			if (strcmp(buf4,"day") == 0) {
				timeframe = 1;
			}
			else if (strcmp(buf4,"week") == 0) {
				timeframe = 2;
			}
			else if (strcmp(buf4,"fortnight") == 0) {
				timeframe = 4;
			}
			else if (strcmp(buf4,"month") == 0) {
				timeframe = 3;
			}
		}
		result = mysql_player_search (search_type, buf3, timeframe);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			send_to_char ("No playerfiles matching your search were found.\n",
				ch);
			return;
		}
		sprintf (buf, "#6Playerfiles Matching Search: %d#0\n"
			"(#2approved#0, #1dead#0, #5suspended#0, #6pending#0)\n\n",
			(int) mysql_num_rows (result));
		i = 1;

		while ((row = mysql_fetch_row (result)))
		{
			// row[0] = account
			// row[1] = name
			// row[2] = sdsc
			// row[3] = create_state
			// opt row[4] = clan_rank

			const int color_state [5] = { 6, 6, 2, 5, 1 };
			int create_state = strtol (row[3], 0, 10);

			// safeguard wonky create_state values
			if (create_state < 0 || create_state > 4)
				create_state = 0;

			sprintf (buf2, "%4d. %-12s #%d%-10s",
				i, row[0], color_state[create_state], row[1]);
			if (search_type == SEARCH_CLAN)
			{
				sprintf (buf2 + strlen(buf2), " #0%-10s#%d",
					row[4], color_state[create_state]);
			}
			sprintf (buf2 + strlen(buf2), " %s#0\n", row[2]);
			if (strlen (buf) + strlen (buf2) >= MAX_STRING_LENGTH)
				break;
			else
				sprintf (buf + strlen (buf), "%s", buf2);
			i++;
			
			if (strlen (buf) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
			{
				send_to_char
				("Too many matches found - please use a more selective criterion!\n",
				 ch);
				return;
			}
		}
		page_string (ch->desc, buf);
		mysql_free_result (result);
		return;

	case 'u':			/* Undescribed room only */

		if (isdigit (*buf2))
			nSearchZone = atoi (buf2);

		if (nSearchZone < 0 || nSearchZone > MAX_ZONE)
		{
			nSearchZone = -1;
			stream << "\n#6Searching All Zones for Undescribed Rooms and Objects#0\n";
		}
		else
		{
			stream << "\n#6Searching Zone ##" << nSearchZone << ", " << zone_table[nSearchZone].name << ", for Undescribed Rooms and Objects#0\n";
		}

		stream <<  "\n#6Rnum     Room Name#0\n\n";
		for (troom = full_room_list; troom; troom = troom->lnext)
		{

			if (nSearchZone >= 0 && troom->zone != nSearchZone)
				continue;

			if (strncmp (troom->description, "No Description Set", 18))
				continue;

			if (IS_SET (troom->room_flags, STORAGE))
				continue;

			stream <<  "[" << troom->nVirtual << "]  " << troom->name << "\n";
		}

		stream <<  "\n\n#6Vnum     Object Name(s)   Short desc#0\n\n";
		for (k = full_object_list; k; k = k->lnext)
		{

			if (nSearchZone >= 0 && k->zone != nSearchZone)
				continue;

			if (k->full_description && k->full_description[0])
			{
				continue;
			}

			stream << "[" << k->nVirtual << "] " <<  ((k->name) ? k->name : "#1Unnamed Object!#0") << ((k->short_description) ? k->short_description : "#1No Short Desc!#0" ) << "\n";
		}

		page_string (ch->desc, stream.str().c_str());
		return;
	case 'r':			/* Room */

		if (buf1[1] == '*')
			ok_only = 1;

		*tmp = 0;
		for (troom = full_room_list; troom; troom = troom->lnext)
		{

			if (*buf3 && atoi (buf3) > troom->nVirtual)
				continue;

			if (*buf4 && atoi (buf4) < troom->nVirtual)
				continue;

			if (!ok_only && troom->zone != atoi (buf2))
				continue;

			if (undesc_only && !IS_SET (troom->room_flags, STORAGE) &&
				strncmp (troom->description, "No Description Set", 18))
				continue;

			if (ok_only && !IS_SET (troom->room_flags, ROOM_OK))
				continue;

			if (IS_SET (troom->room_flags, STORAGE))
				strcat (tmp, "S");
			else
				strcat (tmp, " ");

			if (troom->prg)
				strcat (tmp, "P");
			else
				strcat (tmp, " ");

			sprintf (tmp + strlen (tmp), "[%5d] %s", troom->nVirtual,
				troom->name);

			for (n = 0; n <= LAST_DIR; n++)
			{
				if (troom->dir_option[n] && troom->dir_option[n]->to_room != -1)
				{
					switch (n)
					{
					case 0:
						sprintf (tmp + strlen (tmp), "  N [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 1:
						sprintf (tmp + strlen (tmp), "  E [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 2:
						sprintf (tmp + strlen (tmp), "  S [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 3:
						sprintf (tmp + strlen (tmp), "  W [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 4:
						sprintf (tmp + strlen (tmp), "  U [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 5:
						sprintf (tmp + strlen (tmp), "  D [%5d]",
							troom->dir_option[n]->to_room);
						break;
					}
				}
			}
			sprintf (tmp + strlen (tmp), "\n");
			if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
			{
				send_to_char
				("Too many matches found - please use a more selective criterion!\n",
				 ch);
				return;
			}
		}
		page_string (ch->desc, tmp);
		return;
	case 'z':			/* By zone */
		for (i = 0; i < MAX_ZONE; i += 5)
		{
			sprintf (tmp, "##%2d %-11.11s ##%2d %-11.11s ##%2d %-11.11s ##%2d "
				"%-11.11s ##%2d %-11.11s\n", i, zone_table[i].name,
				i + 1, zone_table[i + 1].name, i + 2,
				zone_table[i + 2].name, i + 3, zone_table[i + 3].name,
				i + 4, zone_table[i + 4].name);
			send_to_char (tmp, ch);
		}
		return;
	case 'a':
		sprintf (tmp,
			"    Name                     Rooms     Plyrs     Mobs     Objs\n\n");
		for (i = 0; i < MAX_ZONE; i++)
		{
			if (strcmp (zone_table[i].name, "Empty Zone"))
			{
				sprintf (tmp + strlen (tmp), "%2d - %-16.16s", i,
					zone_table[i].name);
				count = 0;
				n = 0;
				for (troom = full_room_list; troom; troom = troom->lnext)
				{
					if (troom->zone == i)
					{
						count++;
						if (!
							(strncmp
							(troom->description, "No Description Set", 18)))
							n++;
					}
				}
				sprintf (tmp + strlen (tmp), "%10d/%-4d", count, n);
				count = 0;
				count2 = 0;
				//for (j = character_list; j; j = j->next)
				for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
				{
					j = *tch_iterator;

					if (!IS_NPC (j))
						continue;

					if (j->in_room / 1000 == i)
						count++;
				}
				//for (j = character_list; j; j = j->next)
				for (std::list<char_data*>::iterator tch_iterator_two = character_list.begin(); tch_iterator_two != character_list.end(); tch_iterator_two++)
				{
					j = *tch_iterator_two;
					if (!IS_NPC (j) && j->room && j->room->zone == i)
						count2++;
				}
				sprintf (tmp + strlen (tmp), "%6d%10d", count2, count);
				count = 0;

				for (k = object_list; k; k = k->next)
					if (!k->deleted && k->zone == i)
						count++;

				sprintf (tmp + strlen (tmp), "%10d\n", count);
			}
		}
		page_string (ch->desc, tmp);
		return;

	case 's':
		n = 0;
		otop = 0;
		mtop = 0;

		for (count_total_rooms = 0, troom = full_room_list;
			troom; troom = troom->lnext, count_total_rooms++)
			if (!(strncmp (troom->description, "No Description Set", 18)) ||
				!strncmp (troom->description, "   No Description Set", 21))
				n++;

		for (k = full_object_list; k; k = k->lnext)
			if (!k->deleted)
				otop++;

		for (j = full_mobile_list; j; j = j->mob->lnext)
			if (!j->deleted && IS_NPC (j))
				mtop++;

		//for (j = character_list; j; j = j->next)
		for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			j = *tch_iterator;
			if (j->deleted)
				continue;
			if (IS_NPC (j) && IS_SET (j->act, ACT_STAYPUT))
				stayput++;
		}

		for (craft = crafts; craft; craft = craft->next)
			craft_tot++;

		mysql_safe_query ("SELECT count(name) from %s.pfiles", player_db.c_str ());
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				chars = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		mysql_safe_query ("SELECT count(username) from forum_users");
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				accounts = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		mysql_safe_query ("SELECT count(*) from player_writing");
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				docs = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		sprintf (tmp, "\n   Described Rooms:  %-5d\n"
			"   Total Rooms:      %-5d\n"
			"   Total Mobiles:    %-5d\n"
			"   Total Objects:    %-5d\n"
			"   Total Crafts:     %-5d\n\n"
			"   Total Characters: %-5d\n"
			"   Total Accounts:   %-5d\n"
			"   Player Writings:  %-5d\n\n"
			"   Stayput Mobiles:  %-5d\n",
			count_total_rooms - n, count_total_rooms, mtop, otop,
			craft_tot, chars, accounts, docs, stayput);
		send_to_char (tmp, ch);
		return;
	default:
		send_to_char ("Not a valid show option.\n", ch);
		return;
	}
}

void
do_zname (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	while (isspace (*argument))
		argument++;

	sprintf (buf, "zset %d name %s", ch->room->zone, argument);

	command_interpreter (ch, buf);
}

void
do_rlink (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int dir;
	int cha_rnum;
	ROOM_DATA *troom;

	argument = one_argument (argument, buf1);
	argument = one_argument (argument, buf2);

	if (!*buf2)
	{
		send_to_char ("Syntax:  rlink <direction> <room-number>\n", ch);
		return;
	}

	switch (*buf1)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;

	if (!(troom = vtor (atoi (buf2))))
	{
		send_to_char ("No room exists with that number.\n", ch);
		return;
	}

	if (vtor (ch->in_room)->dir_option[dir])
		vtor (vtor (ch->in_room)->dir_option[dir]->to_room)->
		dir_option[rev_dir[dir]] = 0;

	vtor (cha_rnum)->dir_option[dir] = new room_direction_data;
	vtor (cha_rnum)->dir_option[dir]->general_description = 0;
	vtor (cha_rnum)->dir_option[dir]->keyword = 0;
	vtor (cha_rnum)->dir_option[dir]->exit_info = 0;
	vtor (cha_rnum)->dir_option[dir]->key = -1;
	vtor (cha_rnum)->dir_option[dir]->to_room = troom->nVirtual;

	troom->dir_option[rev_dir[dir]] = new room_direction_data;
	troom->dir_option[rev_dir[dir]]->general_description = 0;
	troom->dir_option[rev_dir[dir]]->keyword = 0;
	troom->dir_option[rev_dir[dir]]->exit_info = 0;
	troom->dir_option[rev_dir[dir]]->key = -1;
	troom->dir_option[rev_dir[dir]]->to_room = cha_rnum;

	send_to_char ("Done.\n", ch);
}

void
post_rcret (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	EXTRA_DESCR_DATA *redesc;

	ch = d->character;
	ROOM_DATA* r = vtor(ch->in_room);
	
	if(!r->secrets[ch->delay_info1])
	{
		send_to_char("There was an error saving the redesc. Please report this to a coder.\n", ch);
	}
	else
	{
		r->secrets[ch->delay_info1]->stext = duplicateString(d->descStr);
		send_to_char("Rcret installed.\n", ch);
	}

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
}

void
do_rcret (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[80], buf2[80];
	int dir, dif;
	struct secret *r_secret;

	half_chop (argument, buf1, buf2);
	if (!*buf2)
	{
		send_to_char ("Usage:  rcret <dir> <[difficulty | 'remove']>\n", ch);
		return;
	}
	dif = atoi (buf2);

	switch (*buf1)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	if (vtor (ch->in_room)->secrets[dir])
	{
		if (!strcmp(buf2, "remove"))
		{
			send_to_char("Secret exit removed.\n", ch);
			vtor (ch->in_room)->secrets[dir] = 0;
			return;
		}
		send_to_char ("The old secret description was: \n\n", ch);
		send_to_char (vtor (ch->in_room)->secrets[dir]->stext, ch);
		r_secret = vtor (ch->in_room)->secrets[dir];
	}
	else
	{
		r_secret = new secret;
	}
	r_secret->diff = dif;
	vtor (ch->in_room)->secrets[dir] = r_secret;

	make_quiet (ch);
	send_to_char ("Enter a new secret description.  Terminate with an '@'\n",
		ch);
	free_mem(ch->desc->descStr);
	r_secret->stext = 0;
	ch->desc->max_str = 2000;
	ch->delay_info1 = dir;
	ch->desc->proc = post_rcret;

	send_to_char ("Done.\n", ch);
}

void
do_rexit (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int dir;
	int cha_rnum;
	ROOM_DATA *troom;

	half_chop (argument, buf1, buf2);

	if (!strcmp (buf2, ""))
	{
		send_to_char ("No room specified...aborting...\n", ch);
		return;
	}

	switch (*buf1)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}
	cha_rnum = ch->in_room;

	if (!(troom = vtor (atoi (buf2))))
	{
		send_to_char ("No room exists with that number.\n", ch);
		return;
	}

	if (!vtor (ch->in_room)->dir_option[dir])
		vtor (cha_rnum)->dir_option[dir] = new room_direction_data;

	vtor (cha_rnum)->dir_option[dir]->general_description = 0;
	vtor (cha_rnum)->dir_option[dir]->keyword = 0;
	vtor (cha_rnum)->dir_option[dir]->exit_info = 0;
	vtor (cha_rnum)->dir_option[dir]->key = -1;
	vtor (cha_rnum)->dir_option[dir]->to_room = troom->nVirtual;
	vtor (cha_rnum)->dir_option[dir]->pick_penalty = 0;

	send_to_char ("Done.\n", ch);
}

void
do_rname (CHAR_DATA * ch, char *argument, int cmd)
{

	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& GET_TRUST (ch) < 1)
	{
		send_to_char ("You do not have approval for leadership commands", ch);
		return;
	}
	
	for (; isspace (*argument); argument++);	/* Get rid of whitespaces */

	vtor (ch->in_room)->name = duplicateString (argument);

	send_to_char ("Done.\n", ch);
}

void
do_rinit (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = -1;
	int virt_nr;
	int i, j;
	ROOM_DATA *troom;
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, arg1);

	if (!*arg1)
	{
		send_to_char
			("You must supply a new room number, or 'n' to use the next available slot.\n",
			ch);
		return;
	}

	virt_nr = atoi (arg1);

	if (!isdigit (*arg1) && *arg1 != 'n')
	{
		send_to_char
			("The argument must be a digit, or 'n' to use the next available slot.\n",
			ch);
		return;
	}
	else if (vtor (virt_nr) && *arg1 != 'n')
	{
		send_to_char ("That room number already exists.\n", ch);
		return;
	}
	else if (*arg1 == 'n')
	{
		for (i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++)
			if (!vtor (i))
				break;
		virt_nr = i;
	}

	if (virt_nr > ROOM_MAX)
	{
		sprintf (buf, "Room numbers cannot exceed %d, You need to recompile "
			"and increase ROOM_MAX", ROOM_MAX);
		send_to_char (buf, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf)
		dir = index_lookup (dirs, buf);

	troom = allocate_room (virt_nr);

	troom->nVirtual = virt_nr;
	troom->contents = NULL;
	troom->people = NULL;
	troom->light = 0;		/* Zero light sources */
	troom->zone = virt_nr / ZONE_SIZE;
	troom->name = duplicateString ("New Room");
	troom->description = duplicateString ("No Description Set.\n");
	troom->ex_description = NULL;
	troom->wdesc = NULL;
	troom->extra = NULL;
	for (j = 0; j < LAST_DIR; j++)
		troom->dir_option[j] = 0;

	if (dir != -1)
	{
		sprintf (buf, "%s %d", dirs[dir], troom->nVirtual);
		do_rlink (ch, buf, 0);
	}
	else
	{
		sprintf (buf, "Room %d has been initialized.\n", virt_nr);
		send_to_char (buf, ch);
	}
}

void
do_rxchange (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH], *result;
	char *tail, *end;
	int i = 0, j = 0, k = 0;

	half_chop (argument, arg1, arg2);

	if (!(tail = strstr (ch->room->description, arg1)))
	{
		send_to_char ("Old string not found.\n", ch);
		return;
	}
	i = strlen (ch->room->description);
	j = strlen (tail);
	end = (tail += strlen (arg1));
	k = strlen (end);
	if (!arg2[0])
	{
		result = new char[i + j + k];
		strncpy (result, ch->room->description, (i - j));
		strcat (result, arg2);
		strcat (result, end + 1);
	}
	else
	{
		result = new char[i + j + k + 1];
		strncpy (result, ch->room->description, (i - j));
		strcat (result, arg2);
		strcat (result, end);
	}

	free_mem (ch->room->description);
	ch->room->description = NULL;
	ch->room->description = duplicateString (result);
}

void
post_rdesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	ROOM_DATA *room;

	ch = d->character;
	room = vtor (ch->delay_info1);
	ch->delay_info1 = 0;

	if (!d->pending_message->message)
	{
		send_to_char ("No room description posted.\n", ch);
		return;
	}

	room->description = duplicateString (d->pending_message->message);
	free_mem(d->pending_message);
	d->pending_message = NULL;
}

void
do_rdesc (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;

	room = ch->room;

	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& GET_TRUST (ch) < 1)
	{
		send_to_char ("You do not have approval for leadership commands", ch);
		return;
	}
	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "reformat"))
	{
		reformat_desc (ch->room->description, &ch->room->description);
		send_to_char (ch->room->description, ch);
		return;
	}

	if (*buf)
	{
		send_to_char ("The only valid parameter to rdesc is 'reformat'.\n", ch);
		return;
	}

	if (room->description && !IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
	{
		send_to_char ("The old description was: \n", ch);
		send_to_char (room->description, ch);
	}

	act ("$n begins editing a room description.", false, ch, 0, 0, TO_ROOM);

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->delay_info1 = room->nVirtual;

	if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
	{
		ve_setup_screen (ch->desc);
	}
	else
	{
		send_to_char
			("Please enter the new description; terminate with an '@'\n\n", ch);
		send_to_char
			("1-------10--------20--------30--------40--------50--------60---65\n",
			ch);
		make_quiet (ch);
	}

	//TOGGLE (room->room_flags, ROOM_OK);
	ch->desc->proc = post_rdesc;
}

void
do_rappend (CHAR_DATA * ch, char *argument, int cmd)
{
	char *new_string;

	new_string = new char[strlen (vtor (ch->in_room)->description) + 1];
	strcpy (new_string, vtor (ch->in_room)->description);
	vtor (ch->in_room)->description = new_string;

	if (vtor (ch->in_room)->description)
	{
		send_to_char ("The old description was: \n", ch);
		send_to_char (vtor (ch->in_room)->description, ch);
	}

	send_to_char ("\nEnter a your additions.  Terminate with an '@'\n", ch);
	send_to_char
		("1-------10--------20--------30--------40--------50--------60---65\n",
		ch);
	make_quiet (ch);
	free_mem(ch->desc->descStr);
	ch->desc->descStr = duplicateString(vtor (ch->in_room)->description);
	ch->desc->max_str = 2000;
}

void
post_redesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	EXTRA_DESCR_DATA *redesc;

	ch = d->character;
	ROOM_DATA* r = vtor(ch->in_room);
	
	for (redesc = r->ex_description; redesc; redesc = redesc->next)
	{
		if (!strn_cmp (redesc->keyword, ch->delay_who, strlen (ch->delay_who)))
		{
			break;
		}
	}
	
	if(!redesc)
	{
		send_to_char("There was an error saving the redesc. Please report this to a coder.\n", ch);
	}
	else
	{
		r->ex_description->description = duplicateString(d->descStr);
		send_to_char("Redesc installed.\n", ch);
	}

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
}

void
do_redesc (CHAR_DATA * ch, char *argument, int cmd)
{
	EXTRA_DESCR_DATA *tmp;
	EXTRA_DESCR_DATA *newdesc;
	char buf[MAX_STRING_LENGTH];

	if (!*argument)
	{
		send_to_char ("No argument specified\n", ch);
		return;
	}

	argument = one_argument(argument, buf);

	if (!str_cmp (argument, "reformat"))
	{
		for (tmp = vtor (ch->in_room)->ex_description; tmp; tmp = tmp->next)
		{
			if (!strn_cmp (tmp->keyword, buf, strlen (buf)))
			{
				reformat_desc (tmp->description, &tmp->description);
				send_to_char (tmp->description, ch);
				return;
			}
		}
		return;
	}

	if (!str_cmp (argument, "delete"))
	{
		if ((vtor (ch->in_room)->ex_description)
			&& (!strn_cmp (vtor (ch->in_room)->ex_description->keyword, buf, strlen (buf))))
		{
			send_to_char("Description deleted\n", ch);
			vtor (ch->in_room)->ex_description =
				vtor (ch->in_room)->ex_description->next;
			return;
		}

		for (tmp = vtor (ch->in_room)->ex_description; tmp; tmp = tmp->next)
		{
			if ((tmp->next)
				&& (!strn_cmp (tmp->next->keyword, buf, strlen (buf))))
			{
				send_to_char("Description deleted\n", ch);
				tmp->next = tmp->next->next;
			}
		}
		return;
	}


	for (tmp = vtor (ch->in_room)->ex_description; tmp; tmp = tmp->next)
	{
		if (!strn_cmp (tmp->keyword, buf, strlen (buf)))
		{
			break;
		}
	}

	if (!tmp)
	{
		newdesc = new extra_descr_data;
		newdesc->next = vtor (ch->in_room)->ex_description;
		newdesc->keyword = duplicateString (buf);
		vtor (ch->in_room)->ex_description = newdesc;
	}
	else
	{
		send_to_char ("The old description was: \n\n", ch);
		send_to_char (vtor (ch->in_room)->ex_description->description, ch);
		newdesc = tmp;
	}

	free_mem(ch->desc->descStr);

	if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
		ve_setup_screen (ch->desc);
	else
	{
		send_to_char ("Enter a new description.  Terminate with an '@'\n", ch);
		send_to_char
			("1-------10--------20--------30--------40--------50--------60---65\n",
			ch);
		make_quiet (ch);
		newdesc->description = 0;
		ch->desc->max_str = 2000;
		ch->desc->proc = post_redesc;
		ch->delay_who = duplicateString (buf);
	}
}

void
do_rlinkrm (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[256];
	int dir;
	int cha_rnum, old_rnum;

	one_argument (argument, buf);

	switch (*buf)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;
	if (vtor (ch->in_room)->dir_option[dir])
	{
		old_rnum = vtor (ch->in_room)->dir_option[dir]->to_room;
	}
	else
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	vtor (cha_rnum)->dir_option[dir] = 0;
	vtor (old_rnum)->dir_option[rev_dir[dir]] = 0;
	send_to_char ("Done.\n", ch);
}

void
do_rexitrm (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[256];
	int dir;
	int cha_rnum;

	one_argument (argument, buf);

	switch (*buf)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;

	vtor (cha_rnum)->dir_option[dir] = 0;
	send_to_char ("Done.\n", ch);
}

void
do_wsave (CHAR_DATA * ch, char *argument, int cmd)
{
	send_to_char ("This command has been disabled. Please use 'zsave all'.\n",
		ch);
	return;
}

void
do_rsector (CHAR_DATA * ch, char *argument, int cmd)
{
	int flag, no = 1, i;
	char buf[256], buf2[512], buf3[512];


	one_argument (argument, buf);
	if (!strcmp (buf, ""))
	{
		sprintf (buf3, "Current sector type: %s\n",
			sector_types[vtor (ch->in_room)->sector_type]);
		send_to_char (buf3, ch);
		return;
	}

	if (buf[0] == '?')
	{
		sprintf (buf2, "The following sector types are available:\n\t\t\t");
		for (i = 0; *sector_types[i] != '\n'; i++)
		{
			sprintf (buf2 + strlen (buf2), "%-10s ", sector_types[i]);
			if (!(no % 4))
				(strcat (buf2, "\n\t\t\t"));
			no++;
		}
		strcat (buf2, "\n");
		send_to_char (buf2, ch);
		return;
	}


	flag = parse_argument (sector_types, buf);

	ch->room->sector_type = flag;

	send_to_char ("Done.\n", ch);
}


void
do_rflags (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *room;
	int flag;
	int no = 1;
	int i;
	char buf[256];
	char buf2[512];
	char buf3[512];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		sprintbit (ch->room->room_flags, room_bits, buf2);
		sprintf (buf3, "Current room flags: %s", buf2);
		if (ch->room->noInvLimit)
			strcat (buf3, "NoInvLimit");
		strcat (buf3, "\n");
		send_to_char (buf3, ch);
		return;
	}

	//Hibou -- Adding for unlimited buying
	if (!str_cmp (buf, "NoInvLimit"))
	{
		if (ch->room->noInvLimit)
			ch->room->noInvLimit = 0;
		else
			ch->room->noInvLimit = 1;
		send_to_char ("Unlimited buying toggled.\n", ch);
		return;
	}

	if (!str_cmp (buf, "mudwide") && IS_IMPLEMENTOR (ch))
	{
		argument = one_argument (argument, buf2);
		if (!*buf2)
		{
			send_to_char ("Which room flag did you wish to toggle, MUD-wide?\n",
				ch);
			return;
		}
		if ((flag = index_lookup (room_bits, buf2)) == -1)
		{
			send_to_char ("No such room flag or deity.\n", ch);
			return;
		}
		argument = one_argument (argument, buf2);
		if (str_cmp (buf2, "on") && str_cmp (buf2, "off"))
		{
			send_to_char ("Did you wish to toggle that flag ON or OFF?\n", ch);
			return;
		}
		for (room = full_room_list; room; room = room->lnext)
		{
			if (IS_SET (room->room_flags, (1 << flag))
				&& !str_cmp (buf2, "off"))
				room->room_flags &= ~(1 << flag);
			else if (!IS_SET (room->room_flags, (1 << flag))
				&& !str_cmp (buf2, "on"))
				room->room_flags |= (1 << flag);
		}
		send_to_char ("Done.\n", ch);
		return;
	}

	if (buf[0] == '?')
	{
		sprintf (buf2, "The following room flags are available:\n\t\t\t");
		for (i = 0; *room_bits[i] != '\n'; i++)
		{
			sprintf (buf2 + strlen (buf2), "%-10s ", room_bits[i]);
			if (!(no % 4))
				(strcat (buf2, "\n\t\t\t"));
			no++;
		}
		strcat (buf2, "\n");
		send_to_char (buf2, ch);
		send_to_char ("\nUse 'NoInvLimit' to allow a shopkeeper unlimited buying.", ch);
		send_to_char ("\nYou may also specify a church name too.\n", ch);
		return;
	}

	if ((i = index_lookup (deity_name, buf)) != -1)
	{
		ch->room->deity = i;
		send_to_char ("Deity set.\n", ch);
		return;
	}

	if ((flag = index_lookup (room_bits, buf)) == -1)
	{
		send_to_char ("No such room flag or deity.\n", ch);
		return;
	}

	if ((((1 << flag) == OOC) || ((1 << flag) == ROOM_OK)) && GET_TRUST (ch) < 5)
	{
		send_to_char ("Only a level 5 or above can set the OOC or OK bit.\n", ch);
		return;
	}

	if (((1 << flag) == TEMPORARY) || ((1 << flag) == SAVE))
	{
		send_to_char ("This flag cannot be set manually.\n", ch);
		return;
	}

	if (!IS_SET (ch->room->room_flags, (1 << flag)))
		ch->room->room_flags |= (1 << flag);
	else
		ch->room->room_flags &= ~(1 << flag);

	send_to_char ("Flag (re)set.\n", ch);
}

void
do_rddesc (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[256];

	one_argument (argument, buf);

	switch (*buf)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;
	default:
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	if (!vtor (ch->in_room)->dir_option[dir])
	{
		send_to_char ("No room exists in that direction.\n", ch);
		return;
	}

	if (vtor (ch->in_room)->dir_option[dir]->general_description)
	{
		send_to_char ("The old description was: \n", ch);
		send_to_char (vtor (ch->in_room)->dir_option[dir]->general_description,
			ch);
	}

	send_to_char ("\nEnter a new description.  Terminate with an '@'\n", ch);
	send_to_char
		("1-------10--------20--------30--------40--------50--------60---65\n",
		ch);
	make_quiet (ch);
	free_mem(ch->desc->descStr);
	ch->desc->descStr = duplicateString(vtor (ch->in_room)->dir_option[dir]->general_description);
	vtor (ch->in_room)->dir_option[dir]->general_description = 0;
	ch->desc->max_str = 2000;
}

void
do_rkey (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int dir;
	ROOM_DATA *room;

	argument = one_argument (argument, buf);

	switch (*buf)
	{
	case 'n':
		dir = 0;
		break;
	case 'e':
		dir = 1;
		break;
	case 's':
		dir = 2;
		break;
	case 'w':
		dir = 3;
		break;
	case 'u':
		dir = 4;
		break;
	case 'd':
		dir = 5;
		break;
	case 'o':
		dir = 6;
		break;
	case 'i':
		dir = 7;
		break;

	default:
		send_to_char ("What direction is that?\n", ch);
		return;
	}

	room = vtor (ch->in_room);

	if (!room->dir_option[dir])
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		send_to_char ("Syntax:  rkey <dir> <key-vnum> [pick-penalty]\n", ch);
		return;
	}

	room->dir_option[dir]->key = atoi (buf);

	*buf = '\0';

	argument = one_argument (argument, buf);

	if (isdigit (*buf))
		room->dir_option[dir]->pick_penalty = atoi (buf);

	send_to_char ("Done.\n", ch);
}

void
do_rdflag (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	int ind;
	ROOM_DIRECTION_DATA *exit;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || (dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Use a direction:  North, South, East, or West.\n", ch);
		return;
	}

	if (!(exit = ch->room->dir_option[dir]))
	{
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What flag would you like to set?  (see tags "
			"exit-bits)\n", ch);
		return;
	}

	if ((ind = index_lookup (exit_bits, buf)) == -1)
	{
		send_to_char ("Unknown door/gate flag (see tags exit-bits)\n", ch);
		return;
	}

	TOGGLE (exit->exit_info, 1 << ind);

	if (IS_SET (exit->exit_info, 1 << ind))
		sprintf (buf, "Flag %s set.\n", exit_bits[ind]);
	else
		sprintf (buf, "Flag %s removed.\n", exit_bits[ind]);

	send_to_char (buf, ch);
}

void
do_minit (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[80];
	char buf[MAX_STRING_LENGTH];
	int vnum;
	CHAR_DATA *newmob;
	int i;

	extern int mob_start_stat;

	if (IS_NPC (ch))
	{
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	one_argument (argument, arg);
	if (!arg)
	{
		send_to_char ("You must supply a vnum.\n", ch);
		return;
	}
	if (!isdigit (*arg) && *arg != 'n')
	{
		send_to_char
			("The vnum must be a number, or 'n' to use the next available slot.\n",
			ch);
		return;
	}
	if (*arg == 'n')
	{
		for (i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++)
			if (!vtom (i) && i != 0)
				break;
		vnum = i;
	}
	else
		vnum = atol (arg);
	if ((vnum < 0) || (vnum > 99999))
	{
		send_to_char ("Vnum must be between 0 and 99999.\n", ch);
		return;
	}

	if (vtom (vnum))
	{
		send_to_char ("That vnum is already in use.\n", ch);
		return;
	}

	newmob = new_char (0);	/* MOB */

	//clear_char (newmob);

	newmob->mob->nVirtual = vnum;
	newmob->mob->zone = vnum / ZONE_SIZE;

	ch->pc->edit_mob = vnum;

	newmob->act = 0;
	newmob->act |= ACT_ISNPC;

	newmob->name = duplicateString ("mob new");
	newmob->short_descr = duplicateString ("a new mobile");

	newmob->speaks = SKILL_SPEAK_WESTRON;
	newmob->skills[SKILL_SPEAK_WESTRON] = 100;
	newmob->max_hit = 80;
	newmob->hit = 80;
	newmob->max_move = 80;
	newmob->move = 80;
	newmob->armor = 0;
	newmob->offense = 0;
	newmob->mob->damroll = 0;

	newmob->str = mob_start_stat;
	newmob->dex = mob_start_stat;
	newmob->intel = mob_start_stat;
	newmob->wil = mob_start_stat;
	newmob->aur = 1; //changed for POWER
	newmob->con = mob_start_stat;
	newmob->agi = mob_start_stat;
	newmob->tmp_str = mob_start_stat;
	newmob->tmp_dex = mob_start_stat;
	newmob->tmp_intel = mob_start_stat;
	newmob->tmp_wil = mob_start_stat;
	newmob->tmp_aur = 1;//changed for POWER
	newmob->tmp_con = mob_start_stat;
	newmob->tmp_agi = mob_start_stat;
	newmob->mob->damnodice = 1;
	newmob->mob->damsizedice = 2;
	newmob->intoxication = 0;
	newmob->hunger = -1;
	newmob->thirst = -1;
	newmob->equip = NULL;

	open_skill (newmob, SKILL_PARRY);
	open_skill (newmob, SKILL_BLOCK);
	open_skill (newmob, SKILL_DODGE);
	open_skill (newmob, SKILL_BRAWLING);

	fix_offense (newmob);

	newmob->natural_delay = 24;
	newmob->fight_mode = 2;
	newmob->speed = SPEED_WALK;

	newmob->clans = duplicateString ("");	/* Null clan list */

	add_mob_to_hash (newmob);

	act ("$n creates a new mobile.\n", true, ch, 0, 0, TO_ROOM);
	sprintf (buf, "Mobile %d has been initialized.\n", vnum);
	send_to_char (buf, ch);
}

void
do_oinit (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int type;
	int num = 1;
	int i;
	OBJ_DATA *newobj;

	arg_splitter (2, argument, arg, arg2);
	if (!arg)
	{
		send_to_char ("You must supply a vnum.\n", ch);
		return;
	}

	if (arg[0] == '?')
	{
		sprintf (buf, "The following item types are available:\n");
		for (type = 0; *item_types[type] != '\n'; type++)
		{
			sprintf (buf + strlen (buf), "%-20s", item_types[type]);
			if (!(num % 4))
				strcat (buf, "\n");
			num++;
		}
		strcat (buf, "\n");
		send_to_char (buf, ch);
		return;
	}

	if (!isdigit (*arg) && *arg != 'n')
	{
		send_to_char
			("The vnum must be a number, or 'n' to use the next available slot.\n",
			ch);
		return;
	}
	if (*arg == 'n')
	{
		for (i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++)
			if (!vtoo (i) && i != 0)
				break;
		vnum = i;
	}
	else
		vnum = atol (arg);
	if ((vnum < 0) || (vnum > 109999))
	{
		send_to_char ("Vnum must be between 0 and 109000.\n", ch);
		return;
	}

	if (vnum == 0)
	{
		send_to_char ("Object 0 must NEVER be used.\n", ch);
		return;
	}

	if (vnum == VNUM_MONEY)
	{
		send_to_char ("That vnum is reserved for old silver use.\n", ch);
		return;
	}

	if (vnum == VNUM_PENNY)
	{
		send_to_char ("That vnum is reserved for pennies.\n", ch);
		return;
	}

	if (vnum == VNUM_FARTHING)
	{
		send_to_char ("That vnum is reserved for farthings.\n", ch);
		return;
	}

	if (vnum == VNUM_TICKET || vnum == VNUM_ORDER_TICKET)
	{
		send_to_char ("That vnum is reserved for ticket use.\n", ch);
		return;
	}

	if (vtoo (vnum))
	{
		send_to_char ("That vnum is already in use.\n", ch);
		return;
	}

	type = parse_argument (item_types, arg2);
	if (type == -1)
	{
		send_to_char ("Not a valid type.\n", ch);
		return;
	}

	newobj = new_object ();

	clear_object (newobj);

	newobj->nVirtual = vnum;
	newobj->zone = vnum / OBJECT_ZONE_SIZE;

	add_obj_to_hash (newobj);

	ch->pc->edit_obj = vnum;

	newobj->full_description = 0;
	newobj->contains = 0;
	newobj->in_room = NOWHERE;
	newobj->in_obj = 0;
	newobj->next_content = 0;
	newobj->carried_by = 0;
	newobj->full_description = duplicateString ("");
	newobj->obj_flags.type_flag = index_lookup (item_types, arg2);

	switch (type)
	{
	case ITEM_FLUID:
		newobj->name = duplicateString ("water");
		newobj->short_description = duplicateString ("filled with water");
		newobj->description = duplicateString ("");
		newobj->obj_flags.weight = 100;
		break;

	case ITEM_LIQUID_FUEL:
		newobj->name = duplicateString ("oil");
		newobj->short_description = duplicateString ("filled with oil");
		newobj->description = duplicateString ("");
		newobj->obj_flags.weight = 100;
		break;

	case ITEM_DWELLING:
		newobj->name = duplicateString ("shelter");
		newobj->short_description = duplicateString ("a hastily-constructed shelter");
		newobj->description =
			duplicateString ("a very crudely and hastily constructed shelter");
		newobj->o.od.value[2] = 1;
		newobj->o.od.value[3] = -1;
		newobj->obj_flags.weight = 1500;
		break;

	case ITEM_LIGHT:
		newobj->name = duplicateString ("lantern");
		newobj->short_description = duplicateString ("a brass lantern");
		newobj->description = duplicateString ("an old dented lantern");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[2] = 20;
		newobj->obj_flags.weight = 500;
		break;

	case ITEM_WEAPON:
		newobj->name = duplicateString ("sword");
		newobj->short_description = duplicateString ("a long sword");
		newobj->description = duplicateString ("a gleaming long sword");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.wear_flags |= ITEM_WIELD;
		newobj->o.od.value[1] = 2;
		newobj->o.od.value[2] = 4;
		newobj->o.od.value[3] = 3;
		newobj->obj_flags.weight = 700;
		break;

	case ITEM_ARMOR:
		newobj->name = duplicateString ("jacket");
		newobj->short_description = duplicateString ("a leather jacket");
		newobj->description = duplicateString ("a heavy leather jacket");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.wear_flags |= ITEM_WEAR_BODY;
		newobj->o.od.value[0] = 2;
		newobj->obj_flags.weight = 700;
		break;

	case ITEM_KEY:
		newobj->name = duplicateString ("key");
		newobj->short_description = duplicateString ("a key");
		newobj->description = duplicateString ("a small silver key");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 10;
		break;

	case ITEM_DRINKCON:
		newobj->name = duplicateString ("skin");
		newobj->short_description = duplicateString ("a water skin");
		newobj->description = duplicateString ("a leaky skin lies here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 10;
		newobj->o.od.value[1] = 0;
		newobj->o.od.value[2] = 0;
		newobj->o.od.value[3] = 0;
		newobj->obj_flags.weight = 200;
		break;

	case ITEM_FOOD:
		newobj->name = duplicateString ("bread");
		newobj->short_description = duplicateString ("a loaf of bread");
		newobj->description = duplicateString ("a loaf of bread");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 6;
		newobj->o.od.value[3] = 0;
		newobj->obj_flags.weight = 300;
		break;

	case ITEM_CONTAINER:
		newobj->name = duplicateString ("bag");
		newobj->short_description = duplicateString ("a rather large bag");
		newobj->description = duplicateString ("a large brown bag lies here");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 100;
		newobj->o.od.value[1] = 0;
		newobj->o.od.value[2] = -1;
		newobj->obj_flags.weight = 300;
		break;

	case ITEM_BOARD:
		newobj->name = duplicateString ("board");
		newobj->short_description = duplicateString ("a board");
		newobj->description = duplicateString ("a bulletin board is standing here.");
		newobj->o.od.value[2] = 1;
		newobj->obj_flags.weight = 100;
		break;

	case ITEM_USURY_NOTE:
		newobj->name = duplicateString ("note usury");
		newobj->short_description = duplicateString ("a usury note");
		newobj->description = duplicateString ("A usury note lies on the floor.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 1;
		newobj->o.od.value[1] = 1;
		newobj->o.od.value[2] = 20;
		newobj->o.od.value[3] = 40;
		newobj->obj_flags.weight = 100;
		break;

	case ITEM_BULLET:
		newobj->name = duplicateString ("bullet sphere small crude iron");
		newobj->short_description = duplicateString ("a crude iron sling bullet");
		newobj->description =
			duplicateString
			("A small sphere of crudely-wrought iron has been left here.");
		newobj->obj_flags.extra_flags |= ITEM_STACK;
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.weight = 10;
		newobj->o.od.value[0] = 1;
		newobj->o.od.value[1] = 4;
		newobj->quality = 30;
		break;

	case ITEM_MISSILE:
		newobj->name = duplicateString ("arrow iron-tipped ash-shafted flight");
		newobj->short_description =
			duplicateString ("an iron-tipped, ash-shafted flight arrow");
		newobj->description = duplicateString ("An arrow has been left here.");
		newobj->obj_flags.extra_flags |= ITEM_STACK;
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.weight = 10;
		newobj->o.od.value[0] = 1;
		newobj->o.od.value[1] = 6;
		newobj->quality = 30;
		break;

	case ITEM_SHEATH:
		newobj->name = duplicateString ("sheath brown leather");
		newobj->short_description = duplicateString ("a brown leather sheath");
		newobj->description =
			duplicateString ("A brown leather sheath has been left here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.wear_flags |= ITEM_WEAR_BELT;
		newobj->o.od.value[0] = 800;
		newobj->quality = 30;
		newobj->obj_flags.weight = 200;
		break;

	case ITEM_KEYRING:
		newobj->name = duplicateString ("keyring sturdy iron");
		newobj->short_description = duplicateString ("a sturdy iron keyring");
		newobj->description =
			duplicateString ("A sturdy iron keyring lies here, forgotten.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.wear_flags |= ITEM_WEAR_BELT;
		newobj->o.od.value[0] = 10;
		newobj->quality = 30;
		newobj->obj_flags.weight = 50;
		break;

	case ITEM_QUIVER:
		newobj->name = duplicateString ("quiver brown leather");
		newobj->short_description = duplicateString ("a brown leather quiver");
		newobj->description = duplicateString ("A brown leather quiver lies here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->obj_flags.wear_flags |= ITEM_WEAR_SHOULDER;
		newobj->o.od.value[0] = 25;
		newobj->quality = 30;
		newobj->obj_flags.weight = 200;
		break;

	case ITEM_INK:
		newobj->name = duplicateString ("ink black pot ceramic small");
		newobj->short_description =
			duplicateString ("a small ceramic pot of black ink");
		newobj->description =
			duplicateString ("A small ceramic pot has been left here.");
		newobj->ink_color = duplicateString ("black ink");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 10;
		newobj->o.od.value[1] = 10;
		newobj->obj_flags.weight = 200;
		break;

	case ITEM_BOOK:
		newobj->name = duplicateString ("tome leather-bound small");
		newobj->short_description = duplicateString ("a small, leather-bound tome");
		newobj->description = duplicateString ("A small tome has been left here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 25;
		newobj->obj_flags.weight = 500;
		break;

	case ITEM_PARCHMENT:
		newobj->name = duplicateString ("paper parchment sheet");
		newobj->short_description = duplicateString ("a sheet of parchment");
		newobj->description =
			duplicateString ("A sheet of parchment has been discarded here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 10;
		break;

	case ITEM_WRITING_INST:
		newobj->name = duplicateString ("quill writing");
		newobj->short_description = duplicateString ("a writing quill");
		newobj->description =
			duplicateString ("A writing quill has been discarded here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 25;
		break;

	case ITEM_HEALER_KIT:
		newobj->name = duplicateString ("kit healer");
		newobj->short_description = duplicateString ("a healer's kit");
		newobj->description = duplicateString ("A healer's kit lies on the floor.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 500;
		break;

	case ITEM_REPAIR_KIT:
		newobj->name = duplicateString ("kit repair mending");
		newobj->short_description = duplicateString ("a repair kit");
		newobj->description = duplicateString ("A repair kit lies on the floor.");
		newobj->obj_flags.wear_flags |= (ITEM_TAKE | ITEM_WEAR_BELT);
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 500;
		break;

	default:
		act ("Generic object created.", false, ch, 0, 0, TO_CHAR);
		newobj->name = duplicateString ("object generic");
		newobj->short_description = duplicateString ("a generic object");
		newobj->description =
			duplicateString ("Some careless admin has left a generic object here.");
		newobj->obj_flags.wear_flags |= ITEM_TAKE;
		newobj->o.od.value[0] = 0;
		newobj->obj_flags.weight = 100;
		return;
		break;
	}

	if (IS_SET (newobj->obj_flags.extra_flags, ITEM_MAGIC))
		newobj->obj_flags.extra_flags &= ~ITEM_MAGIC;
	if (IS_SET (newobj->obj_flags.extra_flags, ITEM_INVISIBLE))
		newobj->obj_flags.extra_flags &= ~ITEM_INVISIBLE;

	if (GET_TRUST (ch) < 5)
		newobj->obj_flags.extra_flags |= index_lookup (extra_bits, "ok");

	// do not auto-load virtual category objects 
	if (vnum<100000)
	  newobj = load_object (vnum);

	obj_to_char (newobj, ch);

	act ("$n does a little creatin'.\n", true, ch, 0, 0, TO_ROOM);
	send_to_char ("Done.\n", ch);
}

void
do_rblock (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *troom;
	char arg1[80], arg2[80], arg3[80];
	int zone, width, height, bfree, used = 0;

	*s_buf = '\0';
	arg_splitter (3, argument, arg1, arg2, arg3);

	zone = atoi (arg1);
	width = atoi (arg2);
	height = atoi (arg3);

	for (troom = full_room_list; troom; troom = troom->lnext)
	{
		if (troom->zone == zone)
			used++;
	}
	bfree = (ZONE_SIZE - used);
	sprintf (s_buf, "bfree:%d\n", bfree);
	send_to_char (s_buf, ch);
	if (bfree < (width * height))
	{
		send_to_char ("Not enough free rooms for block.\n", ch);
		return;
	}
}

void
do_runused (CHAR_DATA * ch, char *argument, int cmd)
{
	int unused[ZONE_SIZE];
	int zone;
	int line_entry;
	int i;
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;

	argument = one_argument (argument, buf);

	if (!*buf)
		zone = vtor (ch->in_room)->zone;

	else if (!just_a_number (buf) || atoi (buf) < 0 || atoi (buf) >= MAX_ZONE)
	{
		send_to_char ("Syntax:  runused [zone #]\n", ch);
		return;
	}

	else
		zone = atoi (buf);

	for (i = 0; i < ZONE_SIZE; i++)
		unused[i] = 0;

	for (room = full_room_list; room; room = room->lnext)
		if (room->zone == zone)
			unused[room->nVirtual % ZONE_SIZE] = 1;

	sprintf (buf, "  ");
	for (i = 0, line_entry = -2; i < ZONE_SIZE; i++)
	{

		if (unused[i])
			continue;

		line_entry = (line_entry + 1) % 12;

		if (line_entry == 11)
			strcat (buf, "\n  ");

		sprintf (buf + strlen (buf), "%-4d ", i + ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

void
do_munused (CHAR_DATA * ch, char *argument, int cmd)
{
	int unused[ZONE_SIZE];
	int zone;
	int line_entry;
	int i;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *mob;

	argument = one_argument (argument, buf);

	if (!*buf)
		zone = vtor (ch->in_room)->zone;

	else if (!just_a_number (buf) || atoi (buf) < 0 || atoi (buf) >= MAX_ZONE)
	{
		send_to_char ("Syntax:  munused [zone #]\n", ch);
		return;
	}

	else
		zone = atoi (buf);

	for (i = 0; i < ZONE_SIZE; i++)
		unused[i] = 0;

	for (mob = full_mobile_list; mob; mob = mob->mob->lnext)
		if (mob->mob->nVirtual / ZONE_SIZE == zone)
			unused[mob->mob->nVirtual % ZONE_SIZE] = 1;

	sprintf (buf, "  ");
	for (i = 0, line_entry = -2; i < ZONE_SIZE; i++)
	{

		if (unused[i])
			continue;

		line_entry = (line_entry + 1) % 12;

		if (line_entry == 11)
			strcat (buf, "\n  ");

		sprintf (buf + strlen (buf), "%-4d ", i + ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

void
do_ounused (CHAR_DATA * ch, char *argument, int cmd)
{
	int unused[OBJECT_ZONE_SIZE];
	int zone;
	int line_entry;
	int i;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	argument = one_argument (argument, buf);

	if (!*buf)
		zone = vtor (ch->in_room)->zone;

	else if (!just_a_number (buf) || atoi (buf) < 0 || atoi (buf) >= OBJECT_MAX_ZONE)
	{
		send_to_char ("Syntax:  ounused [zone #]\n", ch);
		return;
	}

	else
		zone = atoi (buf);

	for (i = 0; i < OBJECT_ZONE_SIZE; i++)
		unused[i] = 0;

	for (obj = full_object_list; obj; obj = obj->lnext)
		if (obj->zone == zone)
			unused[obj->nVirtual % OBJECT_ZONE_SIZE] = 1;

	sprintf (buf, "  ");
	for (i = 0, line_entry = -2; i < OBJECT_ZONE_SIZE; i++)
	{

		if (unused[i])
			continue;

		line_entry = (line_entry + 1) % 12;

		if (line_entry == 11)
			strcat (buf, "\n  ");

		sprintf (buf + strlen (buf), "%-4d ", i + OBJECT_ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

int
index_lookup (const char* const* index, const char* const lookup)
{
	unsigned int i;

	for (i = 0; *index[i] != '\n'; i++)
		if (!strn_cmp (index[i], lookup, strlen (lookup)))
			return i;

	return -1;
}

void
do_tags (CHAR_DATA * ch, char *argument, int cmd)
{
	char tag_name[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int name_no;
	int i;

	//  extern const struct constant_data constant_info[];

	argument = one_argument (argument, tag_name);

	if (!*tag_name)
	{
		for (name_no = 0; *constant_info[name_no].constant_name; name_no++)
		{
			sprintf (buf, "\t%-20s    %s\n",
				constant_info[name_no].constant_name,
				constant_info[name_no].description);
			send_to_char (buf, ch);
		}
		return;
	}

	for (name_no = 0; *constant_info[name_no].constant_name; name_no++)
		if (!strn_cmp (constant_info[name_no].constant_name, tag_name,
			strlen (tag_name)))
			break;

	if (!str_cmp (tag_name, "drinks"))
	{
		act
			("Type 'show o fluid' and set OVAL2 on the drink object to the desired fluid's VNUM.",
			false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!*constant_info[name_no].constant_name)
	{
		send_to_char ("No such tag name, type 'tags' to get a "
			"listing.\n", ch);
		return;
	}

	strcpy (buf, "   ");

	for (i = 0; *(char *) constant_info[name_no].index[i] != '\n'; i++)
	{
		sprintf (buf + strlen (buf), "%-23s ",
			(char *) constant_info[name_no].index[i]);
		if (!((i + 1) % 3))
			strcat (buf, "\n   ");
	}

	if (!((i + 1) % 3) || ((i + 1) % 3) == 2)
		strcat (buf, "\n");

	send_to_char (buf, ch);
}

void
do_object (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *edit_object;

	if (IS_NPC (ch))
	{
		send_to_char ("Only PCs can use this command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (ch->pc->edit_obj && vtoo (ch->pc->edit_obj))
		{
			one_argument (vtoo (ch->pc->edit_obj)->name, buf2);
			sprintf (buf, "object %d", ch->pc->edit_obj);
			do_stat (ch, buf, 0);
		}
		else
			send_to_char ("You're not editing an object.\n", ch);
		return;
	}

	if (just_a_number (buf) && vtoo (atoi (buf)))
		edit_object = vtoo (atoi (buf));

	else if ((edit_object = get_obj_in_list_vis (ch, buf, ch->right_hand)) ||
		(edit_object = get_obj_in_list_vis (ch, buf, ch->left_hand)) ||
		(edit_object = get_obj_in_list_vis (ch, buf, ch->room->contents)))
		;
	else
	{
		send_to_char ("Couldn't find that object.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}

	if (edit_object->nVirtual == VNUM_MONEY ||
		edit_object->nVirtual == VNUM_PENNY ||
		edit_object->nVirtual == VNUM_FARTHING)
	{
		send_to_char ("Nobody is allowed to edit coins.  Sorry.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}

	if (edit_object->nVirtual == VNUM_TICKET
		|| edit_object->nVirtual == VNUM_ORDER_TICKET)
	{
		send_to_char ("Nobody is allowed to edit tickets.  Sorry.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}


	if ((edit_object->nVirtual == VNUM_HEAD)
		|| (edit_object->nVirtual == VNUM_CORPSE))
	{
		send_to_char ("You cannot make changes to this item.\n", ch);
		return;
	}

	ch->pc->edit_obj = edit_object->nVirtual;

	while (*argument == ' ')
		argument++;

	if (*argument)
		do_oset (ch, argument, 0);

	sprintf (buf, "Editing object %s\n", edit_object->name);
	send_to_char (buf, ch);
}

int
redefine_objects (OBJ_DATA * proto)
{
	int change_count = 0;
	OBJ_DATA *obj;

	for (obj = object_list; obj; obj = obj->next)
	{

		if (obj->deleted || obj->nVirtual != proto->nVirtual)
			continue;

		if (GET_ITEM_TYPE (obj) == ITEM_DWELLING)
			continue;

		change_count++;

		obj->partial_deep_copy(proto);
	}

	return change_count;
}

int
redefine_mobiles (CHAR_DATA * proto)
{
	int i;
	int change_count = 0;
	CHAR_DATA *mob;

	//for (mob = character_list; mob; mob = mob->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;

		if (mob->deleted ||
			!IS_NPC (mob) || mob->mob->nVirtual != proto->mob->nVirtual)
			continue;

		change_count++;

		mob->partial_deep_copy(proto);
	}

	return change_count;
}

void
give_oset_help (CHAR_DATA * ch)
{
	page_string (ch->desc, "\n"
		"oset [obj_vnum]\n"
		"     name           \"keywords\"\n"
		"     sdesc          \"a short description\"\n"
		"     ldesc          \"the long description\"\n"
		"     desc           \"the look at/examine description\"\n"
		"     indoor-desc    indoor description for tents and dwellings\n"
		"\n"
		"     delete\n"
		"     'item-types'                        See:  tags item-types\n"
		"     'wear-bits'                         See:  tags wear-bits\n"
		"     'apply-types'  <#>                  Add as next affect-see tags\n"
		"     'extra-bits'                        See:  tags extra-bits\n"
		"     slot <0..5>    <magic spell>        See:  show v magic\n"
		"     [econ] <econ-flags>                 See:  tags econ-flags\n"
		"\n"
		"     activation     <spell-affect>       See:  show v magic\n"
		"        (use in conjunction with get-affect, etc - see extra-bits)\n"
		"\n"
		"     weight         #.#>\n"
		"     cost           #p ##f                set penny/farthing value"
		"     size           <sz-name>            small, large, etc\n"
		"     quality        #                    Odds of breaking\n"
		"     oval           #[#[#[#]]]           All ovalues\n"
		"     oval1...oval6  #                    Individual oval\n"
		"     dam            ##d#                  Natural damage attack\n"
		"     clock          month day hour	     How long until it morphs\n"
		"     morphto        objnum               What it morphs to\n"
		"\n"
		"NOTE:  object affects (apply-types) are not copied from prototype.\n");
}

void
post_odesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	OBJ_DATA *obj;

	ch = d->character;
	obj = vtoo (ch->delay_info1);
	ch->delay_info1 = 0;

	if (!d->pending_message->message)
	{
		send_to_char ("No object description posted.\n", ch);
		return;
	}

	if (ch->delay_info2)
		obj->indoor_desc = duplicateString (d->pending_message->message);
	else
		obj->full_description = duplicateString (d->pending_message->message);

	ch->delay_info2 = 0;

	d->pending_message = NULL;
}

void
do_oset (CHAR_DATA * ch, char *argument, int cmd)
{
	char *tmp_arg;
	OBJ_DATA *edit_obj;
	int ind;
	int i;
	int weight_int;
	int weight_dec;
	int bonus;
	int parms;
	int sides;
	int dice;
	int mend_type;
	int magic_spell;
	int new_cflags;
	int full_description = 0, indoor_description = 0;
	AFFECTED_TYPE *af;
	char subcmd[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];
	char clan_rank[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
	{
		send_to_char ("Not an NPC command.\n", ch);
		return;
	}

	argument = one_argument (argument, subcmd);

	if (!*subcmd || *subcmd == '?')
	{
		give_oset_help (ch);
		return;
	}

	if (atoi (subcmd) != 0 && (edit_obj = vtoo (atoi (subcmd))))
	{
		ch->pc->edit_obj = edit_obj->nVirtual;
		argument = one_argument (argument, subcmd);
	}

	else if (atoi (subcmd))
	{
		sprintf (buf, "No such object vnum %s\n", subcmd);
		send_to_char (buf, ch);
		send_to_char ("Not editing an object now.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}

	else if (!(edit_obj = vtoo (ch->pc->edit_obj)))
	{
		send_to_char ("Start by using the OBJECT command.\n", ch);
		return;
	}

	while (*subcmd)
	{

		if (GET_TRUST (ch) < 5)
			edit_obj->obj_flags.extra_flags |= ITEM_OK;

		if ((ind = index_lookup (item_types, subcmd)) != -1)
		{
			edit_obj->obj_flags.type_flag = ind;
			sprintf (buf, "Type %s set.\n", item_types[ind]);
			send_to_char (buf, ch);
		}

		else if ((ind = index_lookup (wear_bits, subcmd)) != -1)
		{
			if (ind == index_lookup (wear_bits, "Hold"))
			{
				send_to_char
					("The HOLD wear bit has been deprecated and is no longer used.\n",
					ch);
				return;
			}
			if (ind == index_lookup (wear_bits, "Wshield"))
			{
				send_to_char
					("The WSHIELD wear bit has been deprecated and is no longer used.\n",
					ch);
				return;
			}
			if (IS_SET (edit_obj->obj_flags.wear_flags, 1 << ind))
				edit_obj->obj_flags.wear_flags &= ~(1 << ind);
			else
				edit_obj->obj_flags.wear_flags |= (1 << ind);
		}

		else if (!str_cmp (subcmd, "timer"))
		{
			argument = one_argument (argument, buf);
			if (!*buf)
			{
				send_to_char
					("What did you want to set the object's timer to?\n", ch);
				return;
			}
			if (!isdigit (*buf))
			{
				send_to_char ("Expected a numeric value for the new timer.\n",
					ch);
				return;
			}
			edit_obj->obj_timer = atoi (buf);
			send_to_char ("The object's timer has been set.\n", ch);
			if (!edit_obj->obj_timer
				&& IS_SET (edit_obj->obj_flags.extra_flags, ITEM_TIMER))
				edit_obj->obj_flags.extra_flags &= ~ITEM_TIMER;
			if (edit_obj->obj_timer
				&& !IS_SET (edit_obj->obj_flags.extra_flags, ITEM_TIMER))
				edit_obj->obj_flags.extra_flags |= ITEM_TIMER;
			return;
		}

		else if ((ind = index_lookup (extra_bits, subcmd)) != -1)
		{

			if (((1 << ind) == ITEM_VNPC) || ((1 << ind) == ITEM_PITCHED))
			{
				send_to_char ("This flag cannot be set manually.\n", ch);
				return;
			}

			if (((edit_obj->obj_flags.type_flag != ITEM_ARMOR) &&
				(edit_obj->obj_flags.type_flag != ITEM_WORN) &&
				(!str_cmp (subcmd, "mask"))) ||
				((IS_SET (edit_obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT)) &&
				(IS_SET (edit_obj->obj_flags.wear_flags, ITEM_WEAR_HEAD)) &&
				(IS_SET (edit_obj->obj_flags.wear_flags, ITEM_WEAR_FACE)) &&
				(!str_cmp (subcmd, "mask"))))
				send_to_char
				("A mask must be worn HEAD, ABOUT or FACE and must be ARMOR or WORN.\n",
				ch);
			else if ((!IS_SET (edit_obj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
				&&
				(!IS_SET (edit_obj->obj_flags.wear_flags, ITEM_WEAR_FACE))
				&& (edit_obj->obj_flags.type_flag == ITEM_ARMOR)
				&& (!str_cmp (subcmd, "mask")))
				send_to_char
				("HEAD or FACE armor can be a mask, but not ABOUT armor.\n",
				ch);
			else if (IS_SET (edit_obj->obj_flags.extra_flags, 1 << ind))
				edit_obj->obj_flags.extra_flags &= ~(1 << ind);
			else
				edit_obj->obj_flags.extra_flags |= (1 << ind);
		}

		else if (!strn_cmp (subcmd, "affects", strlen (subcmd)))
		{
			argument = one_argument (argument, buf);

			if ((ind = index_lookup (skills, buf)) == -1)
			{
				send_to_char ("No such skill.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			if (*buf != '0' && !atoi (buf))
			{
				send_to_char ("Expected a skill value.\n", ch);
				break;
			}

			if (get_obj_affect_location (edit_obj, ind))
				remove_obj_affect_location (edit_obj, ind);

			if (atoi (buf))
			{
				send_to_char ("Object skill affect set.\n", ch);
				af = new AFFECTED_TYPE;

				af->type = 0;
				af->a.spell.duration = -1;
				af->a.spell.bitvector = 0;
				af->a.spell.sn = 0;
				af->a.spell.location = ind + 10000;
				af->a.spell.modifier = atoi (buf);
				af->next = NULL;

				affect_to_obj (edit_obj, af);
			}
			else
			{
				send_to_char ("Object skill affect removed, if applicable.\n",
					ch);
				remove_obj_affect_location (edit_obj, ind + 10000);
			}
		}

		else if (GET_TRUST (ch) >= 4 && !str_cmp (subcmd, "delete"))
		{

			if (edit_obj->deleted)
			{
				edit_obj->deleted = 0;
				send_to_char ("This object is no longer marked for deletion."
					"\n", ch);
				break;
			}

			if (get_obj_in_list_num (edit_obj->nVirtual, object_list))
			{
				send_to_char ("Clear this object from the world first.\n", ch);
				return;
			}

			send_to_char ("WARNING:  This object is cleared from the world.  "
				"However, the prototype\n"
				"          cannot be removed until the zone is saved,"
				"and the mud is rebooted.\n", ch);
			edit_obj->deleted = 1;
		}

		else if (!strn_cmp (subcmd, "material", 8))
		{
			extern std::map<e_material_type, std::string> material_names;
			bool found_material = false;
			for (std::map<e_material_type, std::string>::iterator it = material_names.begin(); it != material_names.end(); it++)
			{
				if (!it->second.compare(argument))
				{
					found_material = true;
					edit_obj->material = (int) it->first;
				}
			}
			if (!found_material)
			{
				send_to_char("That is not a valid material.\n", ch);
				return;
			}
			else
			{
				send_to_char("Material set.\n", ch);
				return;
			}
		}

		else if (!str_cmp (subcmd, "treats"))
		{
			if (GET_ITEM_TYPE (edit_obj) != ITEM_HEALER_KIT)
			{
				send_to_char ("That item is not a healer's kit.\n", ch);
				return;
			}
			if (!str_cmp (argument, "all"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_ALL))
				{
					edit_obj->o.od.value[5] = 0;
					send_to_char
						("This healer's kit will now treat all types of wounds.\n",
						ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_ALL;
				send_to_char
					("This healer's kit will no longer treat all types of wounds.\n",
					ch);
				return;
			}
			else if (!str_cmp (argument, "blunt"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_BLUNT))
				{
					edit_obj->o.od.value[5] |= TREAT_BLUNT;
					send_to_char
						("This healer's kit will now treat blunt wounds.\n", ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_BLUNT;
				send_to_char
					("This healer's kit will no longer treat blunt wounds.\n",
					ch);
				return;
			}
			else if (!str_cmp (argument, "slash"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_SLASH))
				{
					edit_obj->o.od.value[5] |= TREAT_SLASH;
					send_to_char
						("This healer's kit will now treat slash wounds.\n", ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_SLASH;
				send_to_char
					("This healer's kit will no longer treat slash wounds.\n",
					ch);
				return;
			}
			else if (!str_cmp (argument, "puncture"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_PUNCTURE))
				{
					edit_obj->o.od.value[5] |= TREAT_PUNCTURE;
					send_to_char
						("This healer's kit will now treat puncture wounds.\n",
						ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_PUNCTURE;
				send_to_char
					("This healer's kit will no longer treat puncture wounds.\n",
					ch);
				return;
			}
			else if (!str_cmp (argument, "burn"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_BURN))
				{
					edit_obj->o.od.value[5] |= TREAT_BURN;
					send_to_char
						("This healer's kit will now treat burn wounds.\n", ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_BURN;
				send_to_char
					("This healer's kit will no longer treat burn wounds.\n", ch);
				return;
			}
			else if (!str_cmp (argument, "frost"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_FROST))
				{
					edit_obj->o.od.value[5] |= TREAT_FROST;
					send_to_char
						("This healer's kit will now treat frost wounds.\n", ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_FROST;
				send_to_char
					("This healer's kit will no longer treat frost wounds.\n",
					ch);
				return;
			}

			else if (!str_cmp (argument, "bleed"))
			{
				if (!IS_SET (edit_obj->o.od.value[5], TREAT_BLEED))
				{
					edit_obj->o.od.value[5] |= TREAT_BLEED;
					send_to_char
						("This healer's kit will now treat bleeding wounds.\n", ch);
					return;
				}
				edit_obj->o.od.value[5] &= ~TREAT_BLEED;
				send_to_char
					("This healer's kit will no longer treat bleeding wounds.\n",
					ch);
				return;
			}
			else
				send_to_char
				("Please specify all, blunt, puncture, slash, burn, frost, or bleed.\n",
				ch);
			return;
		}

		else if (!str_cmp (subcmd, "mends"))
		{
			if (GET_ITEM_TYPE (edit_obj) != ITEM_REPAIR_KIT)
			{
				send_to_char ("That item is not a repair kit.\n", ch);
				return;
			}
			if (!str_cmp (argument, "noskill"))
			{
				edit_obj->o.od.value[3] = 0;
				send_to_char ("This repair kit requires no skill to use.\n", ch);
				return;
			}
			else if (!str_cmp (argument, "none"))
			{
				edit_obj->o.od.value[3] = -1;
				edit_obj->o.od.value[5] = -1;
				send_to_char
					("This repair kit will no longer mend any items.\n", ch);
				return;
			}
			else if (!str_cmp (argument, "all"))
			{
				edit_obj->o.od.value[5] = 0;
				send_to_char
				("This repair kit will repair all types of item.\n", ch);
				return;
			}
			else if ((mend_type = parse_argument (item_types, argument)) > 0)
			{

				switch (mend_type)
				{
				case -1:
					break;
				case ITEM_WEAPON:
				case ITEM_SHIELD:
				case ITEM_ARMOR:
				case ITEM_WORN:
				case ITEM_CONTAINER:
				case ITEM_QUIVER:
				case ITEM_SHEATH:
				case ITEM_DWELLING:
					if ((edit_obj->o.od.value[5] =
						(edit_obj->o.od.value[5] ==
						mend_type) ? -1 : mend_type) > 0)
					{
						sprintf (buf,
							"This repair kit will now mend items of type '%s'.\n",
							argument);
						send_to_char (buf, ch);
					}
					else if (edit_obj->o.od.value[3] < 0)
					{
						edit_obj->o.od.value[5] = -1;
						send_to_char
							("This repair kit will no longer mend any type of objects.\n",
							ch);
					}
					else
					{
						edit_obj->o.od.value[5] = 0;
						sprintf (buf,
							"This repair kit will no longer only mend items of type '%s'.\n",
							argument);
						send_to_char (buf, ch);
					}
					return;
				default:
					sprintf (buf,
						"You may not create kits for mending items of type '%s'.\n",
						argument);
					send_to_char (buf, ch);
					return;
				}
			}
			else if ((mend_type = index_lookup (skills, argument)) > 0)
			{
				switch (mend_type)
				{
				case -1:
					break;
				case SKILL_METALCRAFT:
				case SKILL_WOODCRAFT:
				case SKILL_TEXTILECRAFT:
				case SKILL_HIDEWORKING:
				case SKILL_STONECRAFT:
				case SKILL_DYECRAFT:
				case SKILL_GLASSWORK:
				case SKILL_GEMCRAFT:
				case SKILL_POTTERY:
					if ((edit_obj->o.od.value[3] =
						(edit_obj->o.od.value[3] ==
						mend_type) ? -1 : mend_type) > 0)
					{
						sprintf (buf,
							"This repair kit will now mend items created with the skill '%s'.\n",
							argument);
						send_to_char (buf, ch);
					}
					else if (edit_obj->o.od.value[5] < 0)
					{
						edit_obj->o.od.value[3] = -1;
						send_to_char
							("This repair kit will no longer mend any type of objects.\n",
							ch);
					}
					else
					{
						edit_obj->o.od.value[3] = 0;
						sprintf (buf,
							"This repair kit will no longer only mend items created with the skill '%s'.\n",
							argument);
						send_to_char (buf, ch);
					}
					return;
				default:
					sprintf (buf,
						"You may not use the skill '%s' for repairs.\n",
						argument);
					send_to_char (buf, ch);
					return;
				}
			}
				//ADDED FOR REPAIR
			else if ((mend_type = index_lookup (damage_severity, argument)) > 0)
			{
				if (mend_type >= 8)
				{
					edit_obj->o.od.value[4] = 8;
					send_to_char
					("This repair kit will mend any level of damage.\n",
					 ch);
				}
				else
				{
					edit_obj->o.od.value[4] = mend_type;
					sprintf (buf,
							 "This repair kit will mend only items of '%s' damage or less.\n",
							 damage_severity[mend_type]);
					send_to_char (buf, ch);
				}
				return;
			}
				
			send_to_char
				("\n"
				 "oset mends <item type>   - mending items of that type\n"
				 "                           (see tags item-types)\n" 
				 "oset mends all           - will repair all types of items\n"
				 "oset mends none          - will mend no items\n"
				 "\n"
				 "oset mends <skill name>  - skill needed to use this kit\n"
				 "oset mends noskill       - requires no skill to use\n"
				 "\n"
				 "oset mends <damage type> - items with this level of damage or less\n"
				 "                           (see tags damage-severity)\n" 
				 "\n",
				ch);
			return;
		}

		else if (!str_cmp (subcmd, "color"))
		{
			argument = one_argument (argument, buf);

			if (edit_obj->obj_flags.type_flag != ITEM_INK)
			{
				send_to_char ("This object is not an ink!\n", ch);
				return;
			}
			else
				edit_obj->ink_color = duplicateString (buf);
			if ((i = redefine_objects (edit_obj)) > 0)
			{
				sprintf (buf, "%d objects in world redefined.\n", i);
				send_to_char (buf, ch);
			}
			return;
		}

		else if (!str_cmp (subcmd, "weight"))
		{

			argument = one_argument (argument, buf);

			i = sscanf (buf, "%d.%d", &weight_int, &weight_dec);

			if (i == 0)
			{
				send_to_char ("Expected weight value.\n", ch);
				break;
			}

			if (i == 2 && (weight_dec < 0 || weight_dec >= 99))
			{
				send_to_char ("Decimal portion of weight not in 0..99\n", ch);
				break;
			}

			if (i == 1)
				edit_obj->obj_flags.weight = weight_int * 100;
			else
				edit_obj->obj_flags.weight = weight_int * 100 + weight_dec;
		}

		else if (!str_cmp (subcmd, "cost"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a cost value.\n", ch);
				return;
			}

			if (atoi (buf) < 0)
			{
				send_to_char ("Must be a number greater than -1.\n", ch);
				break;
			}

			sscanf (buf, "%f", &edit_obj->farthings);
			edit_obj->silver = 0;

		}

		else if (!str_cmp (subcmd, "size"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a size 1 for each 5 inches.\n", ch);
				break;
			}

			edit_obj->size = atoi (buf);
		}

		else if (!str_cmp (subcmd, "quality"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected quality value.\n", ch);
				break;
			}

			edit_obj->quality = atoi (buf);
		}

		else if (!str_cmp (subcmd, "name"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("What, no name?\n", ch);
				break;
			}

			if (!is_tagged (buf))
			{
				send_to_char
					("You need to include a materials tag! See TAGS MATERIALS.\n",
					ch);
				return;
			}

			edit_obj->name = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "desc") ||
			!str_cmp (subcmd, "descr") || !str_cmp (subcmd, "description"))
		{
			one_argument (argument, buf);
			if (!str_cmp (buf, "reformat"))
			{
				argument = one_argument (argument, buf);
				reformat_desc (edit_obj->full_description,
					&edit_obj->full_description);
				send_to_char (edit_obj->full_description, ch);
			}
			else
				full_description = 1;
		}

		else if (!str_cmp (subcmd, "sdesc"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("What, no short description?\n", ch);
				break;
			}

			edit_obj->short_description = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "ldesc"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("What, no long description?\n", ch);
				break;
			}

			edit_obj->description = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "type"))
		{
			if (GET_ITEM_TYPE (edit_obj) != ITEM_WEAPON)
			{
				send_to_char ("This is only for weapons.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ((ind = index_lookup (skills, buf)) == -1)
			{
				send_to_char
					("Unknown weapon type. It must be the name of the skill that you\nwish the weapon to use, e.g. \"light-edge\".\n",
					ch);
				return;
			}
			else
				edit_obj->o.od.value[3] = ind;
		}

		else if (!str_cmp (subcmd, "oval") || !str_cmp (subcmd, "oval0"))
		{
			if (GET_ITEM_TYPE (edit_obj) == ITEM_PARCHMENT
				|| GET_ITEM_TYPE (edit_obj) == ITEM_DWELLING)
			{
				send_to_char ("This oval cannot be edited.\n", ch);
				return;
			}
			for (i = 0; i <= LAST_DIR; i++)
			{
				tmp_arg = one_argument (argument, buf);
				int new_value = strtol (buf,0,10);

				if (*buf == '0' || new_value)
				{
					argument = tmp_arg;
					if (!i && GET_ITEM_TYPE (edit_obj) == ITEM_NPC_OBJECT)
					{
						if (new_value && !vtom(new_value)) {
							send_to_char ("You must create that mobile first.\n", ch);
							break;
						}
					}
					edit_obj->o.od.value[i] = new_value;
				}
				else if (*buf != '-')
					break;
			}
		}

		else if (!str_cmp (subcmd, "oval1"))
		{
			if (GET_ITEM_TYPE (edit_obj) == ITEM_BOOK
				|| GET_ITEM_TYPE (edit_obj) == ITEM_KEY)
			{
				send_to_char ("This oval cannot be edited.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[1] = atoi (buf);
			else
			{
				send_to_char ("Expected oval1 value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "oval2"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[2] = atoi (buf);
			else
			{
				send_to_char ("Expected oval2 value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "oval3"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[3] = atoi (buf);
			else
			{
				send_to_char ("Expected oval3 value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "oval4"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[4] = atoi (buf);
			else
			{
				send_to_char ("Expected oval4 value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "oval5"))
		{
			argument = one_argument (argument, buf);

			if (GET_ITEM_TYPE (edit_obj) == ITEM_DWELLING
				&& IS_SET (edit_obj->obj_flags.extra_flags, ITEM_PITCHABLE))
			{
				if (!vtor (atoi (buf)))
				{
					send_to_char ("That room doesn't exist.\n", ch);
					return;
				}
				for (i = 0; i <= LAST_DIR; i++)
				{
					if (vtor (atoi (buf))->dir_option[i])
					{
						send_to_char
							("Pitchable dwellings may only be linked to one-room floorplans.\n",
							ch);
						return;
					}
				}
			}

			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[5] = atoi (buf);
			else
			{
				send_to_char ("Expected oval5 value.\n", ch);
				break;
			}

		}

		// Implementation of object categories. Requires code to save builders from themselves - Case
		else if (!str_cmp (subcmd, "inherits")) {
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf)) {
				OBJ_DATA* proto = vtoo(edit_obj->nVirtual);
				int inheritedVNum = atoi(buf);

				// Any object could be a category, but I'm keeping it simple
				// and just using any vnum 100k and over to mark categories - Case
				if (inheritedVNum > 99999 && vtoo(inheritedVNum) != NULL) {
					if (inheritedVNum == edit_obj->nVirtual) {
						send_to_char("This object cannot inherit from that category because it would form an infinite loop.\n", ch);
						return;
					}

					std::set<int> chainedInts;
					chainedInts.insert(edit_obj->nVirtual);
					chainedInts.insert(inheritedVNum);
					OBJ_DATA * loopCheck = vtoo(inheritedVNum);

					while(loopCheck != NULL && loopCheck->super_vnum > 0) {
						if (chainedInts.count(loopCheck->super_vnum) > 0) {
							send_to_char("This object cannot inherit from that category because it would form an infinite loop.\n", ch);
							return;
						}
						chainedInts.insert(loopCheck->super_vnum);
						loopCheck = vtoo(loopCheck->super_vnum);
					}
					proto->super_vnum = atoi(buf);
				}
				else if (inheritedVNum == 0) {
					proto->super_vnum = 0;
				}
				else {
					send_to_char("That object does not exist or is not a category\n", ch);
					return;
				}
			}
			else {
				send_to_char ("Expected the virtual number of a category object.\n", ch);
				return;
			}
		}

		/*
		else if ( !str_cmp (subcmd, "mdesc") ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
		send_to_char ("You must enter a short style description.\n", ch);
		break;
		}

		if ( edit_obj->obj_flags.extra_flags != ITEM_MASK ) {
		send_to_char ("You must set the mask flag before entering mask data.\n",ch);
		break;
		}

		if ( edit_obj->obj_flags.type_flag == ITEM_ARMOR )
		edit_obj->o.armor.helm_short = duplicateString (buf);

		if ( edit_obj->obj_flags.type_flag == ITEM_WORN )
		edit_obj->o.cloak.cloak_short = duplicateString (buf);
		} */

		else if (!str_cmp (subcmd, "mkeys"))
		{

			argument = one_argument (argument, buf);

			if (edit_obj->obj_flags.type_flag != ITEM_TOSSABLE
				&& !IS_SET (edit_obj->obj_flags.extra_flags, ITEM_MASK))
			{
				send_to_char
					("Use <oset mask> to set the mask flag before entering keywords.\nOr change the type to TOSSABLE\n",
					ch);
				break;
			}

			if (!*buf)
			{
				send_to_char ("You must enter a keyword list.\n", ch);
				break;
			}

			if (edit_obj->obj_flags.type_flag == ITEM_ARMOR
				|| edit_obj->obj_flags.type_flag == ITEM_WORN
				|| edit_obj->obj_flags.type_flag == ITEM_TOSSABLE)
			{
				send_to_char
					("Remember, the first keyword will be used in the wearer's short description.\n",
					ch);
				edit_obj->desc_keys = duplicateString (buf);
			}

		}

		else if (!str_cmp (subcmd, "clock"))
		{

			int month, day, hour;

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of months.\n", ch);
				break;
			}
			month = atoi (buf);

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of days.\n", ch);
				break;
			}
			day = atoi (buf);

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of hours.\n", ch);
				break;
			}
			hour = atoi (buf);

			edit_obj->clock = ((month * 30 * 24) + (day * 24) + hour);
		}
		else if (!str_cmp (subcmd, "morphto"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected an object number.\n", ch);
				break;
			}

			edit_obj->morphto = atoi (buf);
		}


		else if (!str_cmp (subcmd, "dam"))
		{

			bonus = 0;

			argument = one_argument (argument, buf);

			parms = sscanf (buf, "%d%[dD]%d%d", &dice, subcmd, &sides, &bonus);

			if (parms < 3)
			{
				send_to_char ("The dam parameter format is #d#+/-#.\n", ch);
				break;
			}

			edit_obj->o.od.value[1] = dice;
			edit_obj->o.od.value[2] = sides;

			if (bonus)
				send_to_char ("Dam bonuses not supported yet.\n", ch);
		}

		else if ((ind = index_lookup (apply_types, subcmd)) != -1)
		{

			argument = one_argument (argument, buf);

			if (*buf != '0' && !atoi (buf))
			{
				send_to_char ("Expected an affect value.\n", ch);
				break;
			}

			if (get_obj_affect_location (edit_obj, ind))
				remove_obj_affect_location (edit_obj, ind);
			else
			{

				af = new AFFECTED_TYPE;

				af->type = 0;
				af->a.spell.duration = -1;
				af->a.spell.bitvector = 0;
				af->a.spell.sn = 0;
				af->a.spell.location = ind;
				af->a.spell.modifier = atoi (buf);
				af->next = NULL;

				affect_to_obj (edit_obj, af);
			}
		}

		else if (!str_cmp (subcmd, "activation"))
		{

			argument = one_argument (argument, buf);

			magic_spell = lookup_value (buf, REG_MAGIC_SPELLS);

			if (magic_spell == -1)
			{
				send_to_char ("Unknown magic spell.\n", ch);
				break;
			}

			edit_obj->activation = magic_spell;
		}

		else if (!str_cmp (subcmd, "slot"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || (i = atoi (buf)) >= 6)
			{
				send_to_char ("Expected slot # after 'slot'.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			magic_spell = lookup_value (buf, REG_MAGIC_SPELLS);

			if (magic_spell == -1)
			{
				send_to_char ("Unknown magic spell.\n", ch);
				break;
			}

			edit_obj->o.od.value[i] = magic_spell;
		}

		else if (!str_cmp (subcmd, "econ"))
		{
			argument = one_argument (argument, buf);
			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("I could not find that econ flag.\n", ch);
				return;
			}
			TOGGLE (edit_obj->econ_flags, 1 << ind);
		}



		/** clanning for objects **/
		/**
		clanadd <clan name> [<clan rank>]
		**/
		else if (!str_cmp (subcmd, "clanadd"))
		{
			argument = one_argument (argument, clan_name);

			if (!*clan_name)
			{
				send_to_char ("Expected a clan name\n", ch);
				return;
			}

			argument = one_argument (argument, clan_rank);
			if (!*clan_rank)
				sprintf(clan_rank, "member");

			new_cflags = clan_flags_to_value (clan_rank);

			if (new_cflags == 0)
				sprintf(clan_rank, "member");

			if (edit_obj->clan_data) //removes old clan
				clan_rem_obj (edit_obj, edit_obj->clan_data);

			if (!edit_obj->clan_data)
			{
				edit_obj->clan_data = new OBJ_CLAN_DATA;
				edit_obj->clan_data->name = duplicateString (clan_name);
				edit_obj->clan_data->rank = duplicateString (clan_rank);
				edit_obj->clan_data->next = NULL;
			}
			send_to_char("Clan/rank have been added.\n", ch);
		}//if clannadd


		/**
		clanremove
		**/
		else if (!str_cmp (subcmd, "clanremove"))
		{
			clan_rem_obj (edit_obj, edit_obj->clan_data);
		}

		else
		{
			sprintf (buf, "Unknown keyword: %s\n", subcmd);
			send_to_char (buf, ch);
			break;
		}

		argument = one_argument (argument, subcmd);
	}

	if ((i = redefine_objects (edit_obj)) > 0)
	{
		sprintf (buf, "%d objects in world redefined.\n", i);
		send_to_char (buf, ch);
	}

	if (full_description)
	{
		free_mem(ch->desc->descStr);
		free_mem(ch->desc->pending_message);
		ch->desc->pending_message = new MESSAGE_DATA;
		ch->desc->descStr = ch->desc->pending_message->message;
		ch->desc->proc = post_odesc;
		ch->delay_info1 = edit_obj->nVirtual;
		if (indoor_description)
			ch->delay_info2 = 1;

		if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
			ve_setup_screen (ch->desc);
		else
		{
			send_to_char ("\nOld description:\n\n", ch);
			if (!indoor_description)
				send_to_char (edit_obj->full_description, ch);
			else
				send_to_char (edit_obj->indoor_desc, ch);
			ch->desc->max_str = STR_MULTI_LINE;
			send_to_char
				("Please enter the new description; terminate with an '@'\n", ch);
			send_to_char
				("1-------10--------20--------30--------40--------50--------60---65\n",
				ch);
			make_quiet (ch);
		}

		return;
	}
}

void
post_oset (DESCRIPTOR_DATA * d)
{
	OBJ_DATA *edit_obj;

	d->proc = NULL;

	if (!d->character)
		return;

	if (!(edit_obj = vtoo (d->character->pc->edit_obj)))
	{
		send_to_char ("Sorry, that object disappeared!\n", d->character);
		return;
	}

	redefine_objects (edit_obj);
}

void
do_mobile (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *edit_mobile;

	if (IS_NPC (ch))
	{
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if (ch->pc->edit_player)
		{
			unload_pc (ch->pc->edit_player);
			ch->pc->edit_player = NULL;
		}

		ch->pc->edit_mob = 0;
	}

	if (!*buf)
	{
		if (ch->pc->edit_mob)
		{
			sprintf (buf, "m %d", ch->pc->edit_mob);
			do_stat (ch, buf, 0);
		}
		else if (ch->pc->edit_player)
		{
			sprintf (buf, "c %s", GET_NAME (ch->pc->edit_player));
			do_stat (ch, buf, 0);
		}
		else
			send_to_char ("You're not editing a mobile.\n", ch);
		return;
	}

	if (!(edit_mobile = load_pc (buf)))
	{

		if (IS_MORTAL (ch) || (!(edit_mobile = vtom (atoi (buf))) &&
			!(edit_mobile = get_char_room_vis (ch, buf))))
		{
			send_to_char ("Couldn't find that mobile.\n", ch);
			ch->pc->edit_mob = 0;
			return;
		}

		if (edit_mobile &&
			edit_mobile->mob &&
			edit_mobile != vtom (edit_mobile->mob->nVirtual))
			ch->pc->target_mob = edit_mobile;
		else
			ch->pc->target_mob = NULL;

		/* If PC, then only a parital name was specified, do a load_pc */

		if (!IS_NPC (edit_mobile))
		{
			if (!(edit_mobile = load_pc (GET_NAME (edit_mobile))))
			{
				send_to_char ("Unable to load online-PC.\n", ch);
				ch->pc->edit_mob = 0;
				return;
			}
		}
	}

	if (!IS_NPC (edit_mobile))
	{

		if (!str_cmp (ch->pc->account_name, edit_mobile->pc->account_name)
			&& !engine.in_test_mode () && edit_mobile != ch)
		{
			send_to_char
				("Sorry, but you'll need to get another staff member to edit your character.\n"
				"Editing of one's own PCs is not permitted, for a variety of reasons.\n",
				ch);
			unload_pc(edit_mobile);
			return;
		}

		if (IS_MORTAL (ch) && edit_mobile->pc->create_state != STATE_SUBMITTED)
		{
			send_to_char
				("You may only open PCs in the application queue to edit.\n", ch);
			unload_pc(edit_mobile);
			return;
		}

		if (GET_TRUST(ch) < 3)
		{
			send_to_char
				("You are not authorized to work with mobiles.\n", ch);
			unload_pc(edit_mobile);
			return;
		}

		ch->pc->edit_mob = 0;
		ch->pc->edit_player = edit_mobile;

		sprintf (buf, "Editing PLAYER %s\n", GET_NAME (edit_mobile));
		send_to_char (buf, ch);
		return;
	}

	if (IS_SET (edit_mobile->act, ACT_STAYPUT))
	{
		act
			("Stayput mobs should NOT be edited. If you need to make a change, edit the prototype on the BP, zsave, swap, purge any stayput instances here and reload them from the database.",
			false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	ch->pc->edit_mob = edit_mobile->mob->nVirtual;

	sprintf (buf, "Editing mobile %s\n", GET_NAMES (edit_mobile));
	send_to_char (buf, ch);
}

void
add_replace_mobprog (CHAR_DATA * ch, CHAR_DATA * mob, char *trigger_name)
{
	MOBPROG_DATA *prog;
	MOBPROG_DATA *last_prog = NULL;

	mp_dirty = 1;

	for (prog = mob->prog; prog; prog = prog->next)
	{

		last_prog = prog;

		if (!str_cmp (prog->trigger_name, trigger_name))
			break;
	}

	if (!prog)
	{
		prog = new MOBPROG_DATA;

		if (last_prog)
			last_prog->next = prog;
		else
			mob->prog = prog;

		prog->trigger_name = duplicateString (trigger_name);
		prog->next = NULL;

	}
	else
		free_mem (prog->prog);

	prog->prog = NULL;
	prog->busy = 0;

	send_to_char ("Enter your mob program.  Terminate with @.\n", ch);

	make_quiet (ch);

	free_mem(ch->desc->descStr);
	ch->desc->descStr = duplicateString(prog->prog);
	ch->desc->proc = post_mset;

	if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
		ve_setup_screen (ch->desc);
	else
		ch->desc->max_str = STR_MULTI_LINE;
}

void
give_mset_help (CHAR_DATA * ch)
{
	page_string (ch->desc,
		"\n"
		"mset [mob-vnum]                          Optionally specify mob vnum\n"
		"     name           \"keywords\"\n"
		"     sdesc          \"a short name\"\n"
		"     ldesc          \"The long desc\"\n"
		"     desc           Enter a full description\n"
		"\n"
		"         parms that take a simple numberic value:\n"
		"\n"
		"     armor | ac, moves, state (-1..4)\n"
		"     piety, natural, str, dex, int, wil, con, aur, bite,\n"
		"     height, frame, room (pc's only), sleep\n"
		"\n"
		"     access         <room flags>\n"
		"     noaccess       <room flags>\n"
		"     conds          <drunk #> <full #> <thirst #>   U must use all three\n"
		"     circle         <0..9>\n"
		"     dam            <##d##>                Natural damage attack\n"
		"     [fightmode]    <frantic | aggressive | normal | cautious | defensive>\n"
		"     helm           <room-vnum>          Set boat/hitch enter room\n"
		"       boat, hitch                         Specify vehicle type\n"
		"     hp | hits      <hit-points>\n"
		"     skinobj        <obj-vnum>           What corpse skins into\n"
		"     carcobj        <obj-vnum>           What carcass is left after skinning\n"
		"     clock          month day hour	     How long until it morphs\n"
		"     morphto        objnum               What it morphs to\n"
		"     merch_seven    <0,1>                Whether or not merch has seven econs\n"
		"     attack         <attack-type>        Natural attack type: claw, etc\n"
		"     allskills | noskills                Gives/removes every skills\n"
		"     delete                              Delete a character.\n"
		"\n"
		"     'position-types'                    Type: tags position-types\n"
		"     'deity'                             Type: tags deity\n"
		"     'races'                             Type: tags races\n"
		"     'skills'       <percent>            Type: tags skills\n"
		"     'action-bits'                       Type: tags action-bits\n"
		"     'affected-bits'                     Type: tags affected-bits\n"
		"     dpos 'position-types'               Default position\n"
		"     trudge, paced, walk, jog, run, sprint\n"
		"\n"
		"     keeper                              Associate shop with mob\n"
		"     ostler                              Mob will act as stablemaster\n"
		"     markup <number>                     To sell multiplier\n"
		"     discount <number>                   To buy multiplier\n"
		"     econ_markup[1-7] <number>           To sell econ flagged multiplier\n"
		"     econ_discount[1-7] <number>         To buy econ flagged multiplier\n"
		"     nobuy <econ flag>                   Won't buy flagged item\n"
		"     econ[1-7] <econ flag>               econ_markup/discount flags\n"
		"     shop                                Shop room number\n"
		"     store                               Store room number\n"
		"     trades \"'item-type' ... \"           Types of items traded\n"
		"     materials \"<material>\"              Materials of items traded\n"
		"     delivery \"<ovnum> ...\"              Objects replaced after buy\n"
		"\n");
}

void
post_mdesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	CHAR_DATA *mob;

	ch = d->character;
	if (ch->delay_info1)
		mob = vtom (ch->delay_info1);
	else
		mob = load_pc (ch->delay_who);
	
	ch->delay_info1 = 0;
	
	if (ch->delay_who)
		free_mem (ch->delay_who);

	if (!mob)
	{
		send_to_char ("NULL mob pointer... aborting...\n", ch);
		return;
	}

	if (!d->pending_message->message)
	{
		send_to_char ("No mobile description posted.\n", ch);
		if (!IS_NPC(mob))
			unload_pc(mob);
		return;
	}

	mob->description = duplicateString (d->pending_message->message);
	d->pending_message = NULL;
	if (!IS_NPC(mob))
		unload_pc(mob);
}

void
update_crafts_file (void)
{
	SUBCRAFT_HEAD_DATA *craft;
	DEFAULT_ITEM_DATA *items;
	DEFAULT_ITEM_DATA *fitems;
	PHASE_DATA *phase;
	FILE *fp;
	int i, j;
	char flag[MAX_STRING_LENGTH];
	bool openchk = false;
	bool opening = false;
	bool sectors = false;
	bool racial = false;
	bool seasonchk = false;
	bool weatherchk =  false;
	char opens_for[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int clanned = 0;
	int race = 0;
	int craft_tot = 0;

	if (engine.in_play_mode ())
		return;

	mysql_safe_query ("DELETE FROM crafts");

	for (craft = crafts; craft; craft = craft->next)
	{
		craft_tot++;

		if (IS_SET (craft->subcraft_flags, SCF_OBSCURE))
			continue;

		/** if race **/
		for (i = 0; i <= 24; i++)
		{
			if (craft->race[i] > 0)
			{
				race = 1;
			}
		}

		/** if clan **/
		if (craft->clans && strlen (craft->clans) > 3)
		{
			clanned = 1;
		}

		/** if opens for skill **/
		*opens_for = '\0';
		for (i = 0; i <= 24; i++)
		{
			if (craft->opening[i] > 0)
			{
				opening = 1;
				if (!*opens_for)
					sprintf (opens_for,
					"%s",
					skills[craft->opening[i]]);
				else
					sprintf (opens_for + strlen (opens_for),
					", %s",
					skills[craft->opening[i]]);
			}
		}

		/** uses skill in phases **/
		*buf = '\0';
		for (phase = craft->phases; phase; phase = phase->next)
		{
			if (phase->skill)
			{
				if (!*buf)
					sprintf (buf,
					"%s",
					skills[phase->skill]);
				else
					sprintf (buf + strlen (buf),
					" %s",
					skills[phase->skill]);
			}
		}

		/** add header info to database **/
		mysql_safe_query
			("INSERT INTO crafts VALUES ('%s', '%s', '%s', %d, '%s', %d, %d, '%s')",
			craft->craft_name, craft->subcraft_name, craft->command, opening,
			buf, clanned, race, *opens_for ? opens_for : "none");
		opening = 0;
		race = 0;
		clanned = 0;
	}

	/** add all information to text file **/
	if (!(fp = fopen (CRAFTS_FILE, "w+")))
	{
		system_log ("The crafts file could not be opened for writing.", true);
		return;
	}

	fprintf (fp, "[CRAFTS]\n");

	for (craft = crafts; craft; craft = craft->next)
	{
		fprintf (fp, "craft %s subcraft %s command %s\n",
			craft->craft_name,
			craft->subcraft_name,
			craft->command);

		if (craft->failure)
			fprintf (fp, "  fail: %s\n", craft->failure);

		if (craft->failobjs)
			fprintf (fp, "  failobjs: %s\n", craft->failobjs);

		if (craft->failmobs)
			fprintf (fp, "  failmobs: %s\n", craft->failmobs);

		if (IS_SET (craft->subcraft_flags, SCF_OBSCURE))
			fprintf (fp, "     flags: %s\n", "hidden");

		if (craft->delay)
			fprintf (fp, "  ic_delay: %d\n", craft->delay);

		if (craft->faildelay)
			fprintf (fp, "fail_delay: %d\n", craft->faildelay);

		if (craft->key_first > 0)
			fprintf (fp, "start_key: %d\n", craft->key_first);

		if (craft->key_end > 0)
			fprintf (fp, "  end_key: %d\n", craft->key_end);

		if (craft->followers > 0)
			fprintf (fp, "followers: %d\n", craft->followers);

		/** is opening craft for some skill(s) **/
		for (i = 0; i <= 24; i++)
		{
			if (craft->opening[i] > 0)
				openchk = true;
		}

		if (openchk)
		{
			fprintf (fp, " opening:");
			for (i = 0; i <= 24; i++)
			{
				if (craft->opening[i])
					fprintf (fp, " %d", craft->opening[i]);
			}
			fprintf (fp, "\n");
		}

		/** requires race **/
		for (i = 0; i <= 24; i++)
		{
			if (craft->race[i] > 0)
				racial = true;
		}

		if (racial)
		{
			fprintf (fp, " race:");

			for (i = 0; i <= 24; i++)
			{
				if (craft->race[i] > 0)
					fprintf (fp, " %d", craft->race[i]);
			}

			fprintf (fp, "\n");
		}

		/** requires clan **/
		if (craft->clans && strlen (craft->clans) > 1)
			fprintf (fp, " clans: %s\n", craft->clans);

		/** requires sector **/
		for (i = 0; i <= 24; i++)
		{
			if (craft->sectors[i] > 0)
				sectors = true;
		}

		if (sectors)
		{
			fprintf (fp, "sectors:");

			for (i = 0; i <= 24; i++)
			{
				if (craft->sectors[i])
					fprintf (fp, " %s", sector_types[craft->sectors[i] - 1]);
			}

			fprintf (fp, "\n");
		}

		/** requires season **/
		for (i = 0; i <= 5; i++)
		{
			if (craft->seasons[i] > 0)
				seasonchk = true;
		}

		if (seasonchk)
		{
			fprintf (fp, "seasons:");

			for (i = 0; i <= 5; i++)
			{
				if (craft->seasons[i])
					fprintf (fp, " %s", seasons[craft->seasons[i] - 1]);
			}

			fprintf (fp, "\n");
		}

		/** requires weather **/
		for (i = 0; i <= 8; i++)
		{
			if (craft->weather[i] > 0)
				weatherchk = true;
		}

		if (weatherchk)
		{
			fprintf (fp, "weather:");

			for (i = 0; i <= 8; i++)
			{
				if (craft->weather[i])
					fprintf (fp, " '%s'", weather_states[craft->weather[i] - 1]);
			}

			fprintf (fp, "\n");
		}


		/***** phases *********/
		for (phase = craft->phases; phase; phase = phase->next)
		{
			fprintf (fp, "  phase\n");

			if (phase->failure)
				fprintf (fp, "1stfail:  %s\n", phase->failure);

			if (phase->second_failure)
				fprintf (fp, "2ndfail:  %s\n", phase->second_failure);

			if (phase->third_failure)
				fprintf (fp, "3rdfail:  %s\n", phase->third_failure);

			if (phase->fail_group_mess)
				fprintf (fp, "groupfail:  %s\n", phase->fail_group_mess);

			if (phase->first)
				fprintf (fp, "    1st:  %s\n", phase->first);

			if (phase->second)
				fprintf (fp, "    2nd:  %s\n", phase->second);

			if (phase->third)
				fprintf (fp, "    3rd:  %s\n", phase->third);

			if (phase->group_mess)
				fprintf (fp, " group:  %s\n", phase->group_mess);

			if (phase->phase_seconds)
				fprintf (fp, "      t:  %d\n", phase->phase_seconds);

			if (phase->load_mob)
				fprintf (fp, "    mob:  %d", phase->load_mob);

			if (phase->nMobFlags)
			{
				if (IS_SET (phase->nMobFlags, 1 << 0))
					fprintf (fp, " set-owner");
			}

			fprintf (fp, "\n");

			if (phase->skill)
				fprintf (fp,
				"  skill:  %s vs %dd%d\n",
				skills[phase->skill],
				phase->dice,
				phase->sides);

			if ( phase->attribute > -1 )
				fprintf (fp,
				"   attr:  %s vs %dd%d\n",
				attrs[phase->attribute],
				phase->dice,
				phase->sides);

			if (phase->flags)
			{
				fprintf (fp, "      f: ");

				if (IS_SET (phase->flags, 1 << 0))
					fprintf (fp, " cannot-leave-room");
				if (IS_SET (phase->flags, 1 << 1))
					fprintf (fp, " open_on_self");
				if (IS_SET (phase->flags, 1 << 2))
					fprintf (fp, " require_on_self");
				if (IS_SET (phase->flags, 1 << 3))
					fprintf (fp, " require_greater");

				fprintf (fp, "\n");
			}

			if (phase->move_cost)
				fprintf (fp, "   cost:  moves %d\n", phase->move_cost);

			if (phase->hit_cost)
				fprintf (fp, "   cost:  hits %d\n", phase->hit_cost);

			if (craft->obj_items)
			{
				for (i = 1; craft->obj_items[i]; i++)
				{
					items = craft->obj_items[i];
					if (items->phase != phase)
						continue;

					if (items->items && items->items[0])
					{
						if (IS_SET (items->flags, SUBCRAFT_HELD))
							sprintf (flag, "held");
						else if (IS_SET (items->flags, SUBCRAFT_WIELDED))
							sprintf (flag, "wielded");
						else if (IS_SET (items->flags, SUBCRAFT_USED))
							sprintf (flag, "used");
						else if (IS_SET (items->flags, SUBCRAFT_PRODUCED))
							sprintf (flag, "produced");
						else if (IS_SET (items->flags, SUBCRAFT_WORN))
							sprintf (flag, "worn");
						else if (IS_SET (items->flags, SUBCRAFT_GIVE))
							sprintf (flag, "give");
						else
							sprintf (flag, "in-room");

						fprintf (fp, "      %d:  (%s", i, flag);

						if (items->item_counts > 1)
							fprintf (fp, " x%d", items->item_counts);

						for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
						{
							if (items->items[j]
							&& items->items[j] != items->item_counts)
							{
								fprintf (fp, " %d", items->items[j]);
							}
						}

						fprintf (fp, ")\n");
					} //if (items->items && items->items[0])
				} //for (i = 1; craft->items[i]; i++)
			}//	if (craft->items)

			if (craft->fails)
			{
				for (i = 1; craft->fails[i]; i++)
				{
					fitems = craft->fails[i];

					if (fitems->phase != phase)
						continue;

					if (fitems->items && fitems->items[0])
					{
						fprintf (fp, " Fail %d:", i);

						if (fitems->item_counts > 1)
							fprintf (fp, " x%d", fitems->item_counts);

						for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
						{
							if (fitems->items[j]
							&& fitems->items[j] != fitems->item_counts)
							{
								fprintf (fp, " %d",
									fitems->items[j]);
							}
						}

						fprintf (fp, "\n");
					}//if (fitems->items && fitems->items[0])
				}//for (i = 1; craft->fails[i]; i++)
			}//if (craft->fails)
		}// for (phase = craft->phases; phase; phase = phase->next)
		fprintf (fp, "end\n");
	}//for (craft = crafts; craft; craft = craft->next)


	fprintf (fp, "[end-crafts]\n");
	fclose (fp);
}

void
do_cset (CHAR_DATA * ch, char *argument, int cmd)
{
	char subcmd[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int phasenum, i, j, dice, sides;
	int objnum = 0;
	int fobjnum = 0;
	int mobnum = 0;
	SUBCRAFT_HEAD_DATA *craft;
	PHASE_DATA *phase;
	DEFAULT_ITEM_DATA *items;
	DEFAULT_ITEM_DATA *fitems;


	if (IS_NPC (ch))
	{
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	if (!IS_SET(ch->plr_flags, IS_CRAFTER))
	{
		send_to_char ("You must be an authorised crafter to use this command.\n",ch);
		return;
	}

	if (!ch->pc->edit_craft)
	{
		send_to_char
			("You must first open a craft to edit via the CRAFT command.\n", ch);
		return;
	}

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		do_crafts (ch, "", 0);
		return;
	}

	/*** Craft set-up ****/
	argument = one_argument (argument, subcmd);

	if ((!str_cmp (subcmd, "craft"))||
		(!str_cmp (subcmd, "subcraft")) ||
		(!str_cmp (subcmd, "command")) ||
		(!str_cmp (subcmd, "hidden")) ||
		(!str_cmp (subcmd, "obscure")))
	{
		craft_setup (ch, argument, subcmd);
		return;
	}


	else if (!str_cmp (subcmd, "sectors"))
	{
		craft_sectors(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "seasons"))
	{
		craft_seasons(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "weather"))
	{
		craft_weather(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "opening"))
	{
		craft_opening(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "race"))
	{
		craft_race(ch, argument, subcmd);
		return;
	}

	else if (!strn_cmp (subcmd, "clan", 4))
	{
		craft_clan(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "delete"))
	{
		craft_delete(ch, argument, subcmd);
		return;
	}

	else if (!strn_cmp (subcmd, "delay", 5))
	{
		craft_delay(ch, argument, subcmd);
		return;
	}

	/***** Default Failures ***********/
	else if (!str_cmp (subcmd, "failure"))
	{
		craft_failure(ch, argument, subcmd);
		return;
	}


	else if (!strn_cmp (subcmd, "failobjs", 6))
	{
		craft_failobjs(ch, argument, subcmd);
		return;
	}


	else if (!strn_cmp (subcmd, "failmobs", 7))
	{
		craft_failmobs(ch, argument, subcmd);
		return;
	}


	/*** key item **/
	else if (!strn_cmp (subcmd, "key_start", 9))
	{
		craft_key(ch, argument, subcmd);
		return;
	}

	/*** key-production list **/
	else if (!strn_cmp (subcmd, "key_end", 7))
	{
		craft_key_product(ch, argument, subcmd);
		return;
	}

	/*** groups **/
	else if (!strn_cmp (subcmd, "followers", 9))
	{
		craft_group(ch, argument, subcmd);
		return;
	}

	/************/
	/*  PHASES  */
	/************/
	else if (!strn_cmp (subcmd, "phase", 5) || !strn_cmp (subcmd, "phsae", 5))
	{
		if (!isdigit (subcmd[5]))
		{
			send_to_char
				("Please specify a phase number, e.g. 'cset phase2'.\n", ch);
			return;
		}
		sprintf (buf, "%c", subcmd[5]);
		if (isdigit (subcmd[6]))
			sprintf (buf + strlen (buf), "%c", subcmd[6]);
		phasenum = atoi (buf);

		if (!craft->phases)
		{
			craft->phases = new PHASE_DATA;
			phase = craft->phases;
		}
		else
			for (phase = craft->phases, i = 1; phase; phase = phase->next, i++)
			{
				if (!phase->next && phasenum == i + 1)
				{
					phase->next = new PHASE_DATA;
				}
				else if (i == phasenum)
					break;
				else if (!phase->next && i++ < phasenum)
				{
					sprintf (output,
						"\nIf you want to add a new phase, please use the\nnext available slot; in this case, %d.\n",
						i);
					send_to_char (output, ch);
					return;
				}
			}
			argument = one_argument (argument, subcmd);
			if (!*subcmd)
			{
				send_to_char ("What would you like to edit in this phase?\n", ch);
				return;
			}

			/** Phase Echos **/
			if (!str_cmp (subcmd, "1st"))
			{
				argument = one_argument (argument, buf);
				phase->first = duplicateString (buf);
				sprintf (output,
					"The first person echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			else if (!str_cmp (subcmd, "2nd"))
			{
				argument = one_argument (argument, buf);
				phase->second = duplicateString (buf);
				sprintf (output,
					"The second person echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			else if (!str_cmp (subcmd, "3rd"))
			{
				argument = one_argument (argument, buf);
				phase->third = duplicateString (buf);
				sprintf (output,
					"The third person echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			else if (!str_cmp (subcmd, "failure-1st"))
			{
				argument = one_argument (argument, buf);
				phase->failure = duplicateString (buf);
				sprintf (output,
					"The first-person failure echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			else if (!str_cmp (subcmd, "failure-2nd"))
			{
				argument = one_argument (argument, buf);
				phase->second_failure = duplicateString (buf);
				sprintf (output,
					"The second-person failure echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			else if (!str_cmp (subcmd, "failure-3rd"))
			{
				argument = one_argument (argument, buf);
				phase->third_failure = duplicateString (buf);
				sprintf (output,
					"The third-person failure echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}

			else if (!str_cmp (subcmd, "group"))
			{
				argument = one_argument (argument, buf);
				phase->group_mess = duplicateString (buf);
				sprintf (output,
					"The group echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}

			else if (!str_cmp (subcmd, "failure-group"))
			{
				argument = one_argument (argument, buf);
				phase->fail_group_mess = duplicateString (buf);
				sprintf (output,
					"The group echo for phase %d has been modified.\n",
					phasenum);
				send_to_char (output, ch);
			}
			/** Phase Flags **/
			else if (!str_cmp (subcmd, "flag"))
			{
				argument = one_argument (argument, buf);
				if ((j = index_lookup (phase_flags, buf)) == -1)
				{
					sprintf (output, "The phase flag \'%s\' does not exist.\n",
						buf);
					send_to_char (output, ch);
					return;
				}
				else
				{
					if (IS_SET (phase->flags, 1 << j))
					{
						phase->flags &= ~(1 << j);
						sprintf (output,
							"The \'%s\' phase flag has been removed.\n", buf);
						send_to_char (output, ch);
					}
					else
					{
						phase->flags |= (1 << j);
						sprintf (output, "The \"%s\" phase flag has been added.\n",
							buf);
						send_to_char (output, ch);
					}
				}
			}
			/** Phase Mobs **/
			else if (!strn_cmp (subcmd, "mob", 7))
			{
				argument = one_argument (argument, buf);
				if (!*buf)
				{
					send_to_char
						("Which mob did you want to load on success in this phase?\n",
						ch);
					return;
				}
				if (!isdigit (*buf))
				{
					send_to_char ("You'll need to specify a mob's VNUM.\n", ch);
					return;
				}
				if (atoi (buf) == 0)
					send_to_char ("Loaded mobile removed from the specified phase.\n",
					ch);
				else if (!vtom (atoi (buf)))
				{
					send_to_char
						("The mob you specified doesn't exist in the database.\n",
						ch);
					return;
				}
				else
					send_to_char
					("The specified mobile will now load on success in that phase.\n",
					ch);
				phase->load_mob = atoi (buf);
				argument = one_argument (argument, buf);
				if (*buf && strcmp (buf, "set-owner") == 0)
				{
					phase->nMobFlags |= CRAFT_MOB_SETOWNER;
				}

			}

			/** Phase Delete **/
			else if (!str_cmp (subcmd, "delete"))
			{
				if (phasenum == 1)
				{
					send_to_char
						("If you want to delete the craft, use 'cset delete'.\n", ch);
					return;
				}
				else
					for (phase = craft->phases, i = 1; phase;
						phase = phase->next, i++)
						if (phasenum == i + 1)
							phase->next = phase->next->next;
				sprintf (output, "Phase %d has been deleted.\n", phasenum);
				send_to_char (output, ch);
			}

			/** Phase Timer **/
			else if (!str_cmp (subcmd, "time"))
			{
				argument = one_argument (argument, buf);
				if (!isdigit (*buf))
				{
					send_to_char
						("The specified value for 'time' must be an integer.\n", ch);
					return;
				}
				if (atoi (buf) < 1)
				{
					send_to_char
						("The specified value for 'time' must be greater than one.\n",
						ch);
					return;
				}
				phase->phase_seconds = atoi (buf);
				sprintf (output,
					"The time delay for phase %d has been set to %d seconds.\n",
					phasenum, atoi (buf));
				send_to_char (output, ch);
			}

			/** Phase Skill set and check **/
			else if (!str_cmp (subcmd, "skill"))
			{
				if (phase->attribute > -1)
				{
					send_to_char ("Remove the attribute check if you wish to check against a skill\n", ch);
					return;
				}

				argument = one_argument (argument, buf);
				if (isdigit (*buf) || !(*buf))
				{
					send_to_char
						("The specified 'skill' value must be a skill name.\n", ch);
					return;
				}
				if (!str_cmp (buf, "none"))
				{
					phase->skill = 0;
					send_to_char ("The phase's skill check has been removed.\n",
						ch);
					return;
				}
				if ((i = index_lookup (skills, buf)) == -1)
				{
					sprintf (output,
						"I could not find the '%s' skill in our registry.\n",
						buf);
					send_to_char (output, ch);
					return;
				}
				phase->skill = i;
				for (size_t iter = 0; iter <= strlen (buf); iter++)
					buf[iter] = toupper (buf[iter]);
				sprintf (output, "Phase %d will now check against the %s skill.\n",
					phasenum, buf);
				send_to_char (output, ch);
			}

			else if (!str_cmp (subcmd, "check"))
			{
				argument = one_argument (argument, buf);
				if (!isdigit (*buf))
				{
					send_to_char
						("The specified 'check' value must be in the format XdY, e.g. 2d35.\n",
						ch);
					return;
				}
				if (!phase->skill)
				{
					send_to_char
						("You must first specify a skill to check against for this phase.\n",
						ch);
					return;
				}
				sscanf (buf, "%dd%d", &dice, &sides);
				if (dice < 1 || sides < 0)
				{
					send_to_char
						("Both values specified must be greater than zero.\n", ch);
					return;
				}
				phase->dice = dice;
				phase->sides = sides;
				sprintf (output,
					"The phase's skill check has been changed to %dd%d.\n",
					dice, sides);
				send_to_char (output, ch);
			}

			/** Phase Attribute set and check **/
			else if ( !str_cmp (subcmd, "attribute") )
			{
				if (phase->skill > 0)
				{
					send_to_char ("Remove the skill check if you wish to check an attribute.\n", ch);
					return;
				}

				argument = one_argument (argument, buf);
				if (isdigit (*buf) || !(*buf))
			 {
				 send_to_char ("The specified 'attribute' value must be an attribute name.\n", ch);
				 return;
				}
				if ( !str_cmp (buf, "none") ) {
					phase->attribute = -1;
					send_to_char ("The phase's attribute test has been removed.\n", ch);
					return;
				}
				if ( (i = index_lookup (attrs, buf)) == -1 ) {
					snprintf (output, MAX_STRING_LENGTH,  "I could not find the '%s' attribute in our files.\n", buf);
					send_to_char (output, ch);
					return;
				}
				phase->attribute = index_lookup (attrs, buf);


				for ( i = 0; i <= strlen(buf); i++ )
					buf[i] = toupper(buf[i]);
				snprintf (output, MAX_STRING_LENGTH,  "Phase %d will now test  against the %s attribute.\n", phasenum, buf);
				send_to_char (output, ch);
			}

			else if ( !str_cmp (subcmd, "checkattribute") ) {
				argument = one_argument (argument, buf);
				if ( !isdigit (*buf) ) {
					send_to_char ("The specified 'check' value must be in the format XdY, e.g. 2d35.\n", ch);
					return;
				}
				if ( phase->attribute == -1 ) {
					send_to_char ("You must first specify an attribute to test against for this phase.\n", ch);
					return;
				}
				sscanf (buf, "%dd%d", &dice, &sides);
				if ( dice < 1 || sides < 0 ) {
					send_to_char ("Both values specified must be greater than zero.\n", ch);
					return;
				}
				phase->dice = dice;
				phase->sides = sides;
				snprintf (output, MAX_STRING_LENGTH,  "The phase's attribute check has been changed to %dd%d.\n", dice, sides);
				send_to_char (output, ch);
			}

			/** Phase move cost **/
			else if (!str_cmp (subcmd, "movecost"))
			{
				argument = one_argument (argument, buf);
				if (!isdigit (*buf))
				{
					send_to_char ("Expected a numeric value.\n", ch);
					return;
				}
				else
					phase->move_cost = atoi (buf);
				send_to_char ("The phase's movement cost has been modified.\n", ch);
			}

			/** Phase pain cost **/
			else if (!str_cmp (subcmd, "paincost"))
			{
				argument = one_argument (argument, buf);
				if (!isdigit (*buf))
				{
					send_to_char ("Expected a numeric value.\n", ch);
					return;
				}
				else
					phase->hit_cost = atoi (buf);
				send_to_char ("The phase's pain cost has been modified.\n", ch);
			}
			/** Phase Failure objects **/
			else if (!strn_cmp (subcmd, "fobj", 4))
			{
				if (!isdigit (subcmd[4]))
				{
					send_to_char
						("A failure object number must be specified, e.g. fobj6.\n", ch);
					return;
				}

				sprintf (buf, "%c", subcmd[4]);

				if (isdigit (subcmd[5]))
					sprintf (buf + strlen (buf), "%c", subcmd[5]);

				fobjnum = atoi (buf);

				if (fobjnum < 1)
				{
					send_to_char
						("The specified failure object set must be greater than 1, e.g. fobj6.\n", ch);
					return;
				}

				argument = one_argument (argument, buf);

				if (!*buf)
				{
					send_to_char ("What failure object did you wish to set?\n", ch);
					return;
				}

				if (!craft->fails[0]) {
					craft->fails[0] = new DEFAULT_ITEM_DATA;
					craft->fails[0]->phase = NULL;
				}

				for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
				{
					fitems = craft->fails[i];

					if (i == fobjnum)
						break;

					if (!craft->fails[i + 1])
					{
						if (i + 1 != fobjnum)
						{
							sprintf (output,
								"Please use the next available slot to add a new\n           failure object set; in this case, fobj%d.\n",
								i + 1);
							send_to_char (output, ch);
							return;
						}

						else {
							craft->fails[i + 1] = new DEFAULT_ITEM_DATA;
							craft->fails[i + 1]->phase = NULL;
						}
					}
				}//for (i = 0

				memset (fitems->items, 0, sizeof(int)*MAX_DEFAULT_ITEMS);
				fitems->item_counts = 0;

				if (*buf == 'X' || *buf == 'x')
				{
					*buf2 = '\0';
					for (size_t j = 1; j <= strlen (buf); j++)
						sprintf (buf2 + strlen (buf2), "%c", buf[j]);

					fitems->item_counts = atoi (buf2);
				}

				else if (isdigit (*buf))
				{
					for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
					{
						if (!fitems->items[j])
						{
							if (!vtoo (atoi (buf)) && atoi (buf) != 0)
							{
								sprintf (output,
									"Sorry, but object VNUM %d could not be loaded             for inclusion.\n",
									atoi (buf));
								send_to_char (output, ch);
								break;
							}

							if (atoi (buf) == 0)
							{
								sprintf (output,
									"Failure Object set %d has been removed.\n",
									objnum);
								send_to_char (output, ch);

								for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
									fitems->items[j] = 0;

								break;
							}

							sprintf (output,
								"#2%s#0 has been added to fail object set %d.\n",
								vtoo (atoi (buf))->short_description, fobjnum);

							output[2] = toupper (output[2]);
							send_to_char (output, ch);
							fitems->items[j] = atoi (buf);
							break;
						}// if (!fitems->items[j])
					}//for (j = 0;
				}//else if (isdigit

				while ((argument = one_argument (argument, buf)))
				{
					if (!isdigit (*buf))
						break;

					for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
					{
						if (!fitems->items[j])
						{
							if (!vtoo (atoi (buf)) && atoi (buf) != 0)
							{
								sprintf (output,
									"Sorry, but failure object VNUM %d could not be loaded for inclusion.\n",
									atoi (buf));

								send_to_char (output, ch);
								break;
							}

							if (atoi (buf) == 0)
							{
								sprintf (output,
									"Failure Object set %d has been removed.\n",
									fobjnum);

								send_to_char (output, ch);
								fitems->items[j] = 0;
								break;
							}

							sprintf (output,
								"#2%s#0 has been added to fail object set %d.\n",
								vtoo (atoi (buf))->short_description, fobjnum);
							output[2] = toupper (output[2]);
							send_to_char (output, ch);
							fitems->items[j] = atoi (buf);
							break;
						}//if (!fitems
					}//for (j = 0;
				}//while ((argument

				fitems->phase = phase;
			}//else if (!strn_cmp (subcmd, "fobj", 4))

			/** Phase Objects (normal)**/
			else if (!strn_cmp (subcmd, "object", 6))
			{
				if (!isdigit (subcmd[6]))
				{
					send_to_char
						("An object number must be specified, e.g. object6.\n", ch);
					return;
				}
				sprintf (buf, "%c", subcmd[6]);

				if (isdigit (subcmd[7]))
					sprintf (buf + strlen (buf), "%c", subcmd[7]);

				objnum = atoi (buf);

				if (objnum < 1)
				{
					send_to_char
						("The specified object set must be greater than 1, e.g. object6.\n",
						ch);
					return;
				}

				argument = one_argument (argument, buf);
				if (!*buf)
				{
					send_to_char ("What object did you wish to set?\n", ch);
					return;
				}

				if (!craft->obj_items[0]) {
					craft->obj_items[0] = new DEFAULT_ITEM_DATA;
					craft->obj_items[0]->phase = NULL;
				}

				for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
				{
					items = craft->obj_items[i];
					if (i == objnum)
						break;
					if (!craft->obj_items[i + 1])
					{
						if (i + 1 != objnum)
						{
							sprintf (output,
								"Please use the next available slot to add a new\nobject set; in this case, object%d.\n",
								i + 1);
							send_to_char (output, ch);
							return;
						}
						else {
							craft->obj_items[i + 1] = new DEFAULT_ITEM_DATA;
							craft->obj_items[i + 1]->phase = NULL;
						}
					}
				}//for (i = 0;
				memset (items->items, 0, sizeof(int)*MAX_DEFAULT_ITEMS);
				items->item_counts = 0;
				if (*buf == 'X' || *buf == 'x')
				{
					*buf2 = '\0';
					for (size_t j = 1; j <= strlen (buf); j++)
						sprintf (buf2 + strlen (buf2), "%c", buf[j]);

					items->item_counts = atoi (buf2);
				}

				else if (isdigit (*buf))
				{
					for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
					{
						if (!items->items[j])
						{
							if (!vtoo (atoi (buf)) && atoi (buf) != 0)
							{
								sprintf (output,
									"Sorry, but object VNUM %d could not be loaded for inclusion.\n",
									atoi (buf));
								send_to_char (output, ch);
								break;
							}

							if (atoi (buf) == 0)
							{
								sprintf (output,
									"Object set %d has been removed.\n",
									objnum);
								send_to_char (output, ch);

								for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
									items->items[j] = 0;

								break;
							}
							sprintf (output,
								"#2%s#0 has been added to object set %d.\n",
								vtoo (atoi (buf))->short_description, objnum);
							output[2] = toupper (output[2]);
							send_to_char (output, ch);
							items->items[j] = atoi (buf);
							break;
						}//if (!items->items[j])
					}//for (j = 0;
				}//else if (isdigit (*buf))

				while ((argument = one_argument (argument, buf)))
				{
					if (!isdigit (*buf))
						break;
					for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
					{
						if (!items->items[j])
						{
							if (!vtoo (atoi (buf)) && atoi (buf) != 0)
							{
								sprintf (output,
									"Sorry, but object VNUM %d could not be loaded for inclusion.\n",
									atoi (buf));
								send_to_char (output, ch);
								break;
							}

							if (atoi (buf) == 0)
							{
								sprintf (output,
									"Object set %d has been removed.\n",
									objnum);
								send_to_char (output, ch);
								items->items[j] = 0;
								break;
							}

							sprintf (output,
								"#2%s#0 has been added to object set %d.\n",
								vtoo (atoi (buf))->short_description, objnum);
							output[2] = toupper (output[2]);
							send_to_char (output, ch);
							items->items[j] = atoi (buf);
							break;
						}
					}//for (j = 0
				}//while ((argument

				if (!isdigit (*buf) && *buf)
				{
					if ((j = index_lookup (item_default_flags, buf)) == -1)
					{
						sprintf (output, "The item flag '%s' does not exist.\n",
							buf);
						send_to_char (output, ch);
						return;
					}
					else
					{
						items->flags = 0;
						items->flags |= (1 << j);
						sprintf (output, "The item's '%s' flag has been set.\n",
							item_default_flags[j]);
						send_to_char (output, ch);
					}
				}//if (!isdigit

				items->phase = phase;
			}
			/** Phase error message **/
			else
			{
				send_to_char ("I'm afraid that isn't a recognized phase command.\n", ch);
				return;
			}
	}

	/** cset error message **/
	else
	{
		send_to_char ("I'm afraid that isn't a recognized cset command.\n", ch);
		return;
	}
}

void
mset_account_flag (const CHAR_DATA* ch, const char *account_name, const char *subcmd)
{
	const char* message[14] =
	{
		"There was a problem accessing that player's account!\n",
		"Error: Unknown account flag.\n",
		"The petition ban on that PC's account has been removed.\n",
		"A petition ban has been placed on that PC's account.\n",
		"That PC's account is no longer exempt from any applicable sitebans.\n",
		"That PC's account is now exempt from any applicable sitebans.\n",
		"The guest ban on that PC's account has been removed.\n",
		"A guest ban has been placed on that PC's account.\n",
		"The psionics ban on that PC's account has been removed.\n",
		"A psionics ban has been placed on that PC's account.\n",
		"The retirement ban on that PC's account has been removed.\n",
		"A retirement ban has been placed on that PC's account.\n",
		"That player will no longer be able to share IPs with others.\n",
		"The IP sharing restrictions have been lifted on that account.\n"
	};

	enum
	{
		no_player,
		unknown_flag,
		nopetition_removed,
		nopetition_set,
		noban_removed,
		noban_set,
		noguest_removed,
		noguest_set,
		nopsi_removed,
		nopsi_set,
		noretire_removed,
		noretire_set,
		ipsharing_removed,
		ipsharing_set
	}
	response = no_player;

	account acct (account_name);
	if (acct.is_registered ())
	{
		response = unknown_flag;
		if (!strcasecmp (subcmd, "nopetition"))
		{
			response = nopetition_removed;
			if (acct.toggle_petition_ban ())
			{
				response = nopetition_set;
			}
		}

		else if (!strcasecmp (subcmd, "noban"))
		{
			response = noban_removed;
			if (acct.toggle_ban_pass ())
			{
				response = noban_set;
			}
		}

		else if (!strcasecmp (subcmd, "noguest"))
		{
			response = noguest_removed;
			if (acct.toggle_guest_ban ())
			{
				response = noguest_set;
			}
		}

		else if (!strcasecmp (subcmd, "nopsi"))
		{
			response = nopsi_removed;
			if (acct.toggle_psionics_ban ())
			{
				response = nopsi_set;
			}
		}

		else if (!strcasecmp (subcmd, "noretire"))
		{
			response = noretire_removed;
			if (acct.toggle_retirement_ban ())
			{
				response = noretire_set;
			}
		}

		else if (!str_cmp (subcmd, "ipsharingok"))
		{
			response = ipsharing_removed;
			if (acct.toggle_ip_sharing ())
			{
				response = ipsharing_set;
			}
		}
	}
	send_to_char (message[response], ch);
}

void
mset_cue (CHAR_DATA * builder, MOB_DATA *mob, const char *cue, const char *reflex)
{
	const char * cues [] =
	{
		"none", "notes", "flags", "memory", "on_reboot",
		"on_hour", "on_time",
		"on_health", "on_moves",
		"on_command", "on_receive", "on_hear", "on_nod",
		"on_theft", "on_witness",
		"on_engage", "on_flee", "on_scan", "on_hit",
		"\n"
	};

	int index =  index_lookup (cues, cue);
	if (index < 0)
	{
		send_to_char("Options available are: notes, on_hour, on_health, on_scan\n", builder);
		send_to_char("To remove all cues of one type, use mset <cue type> clear", builder);
		return;
	}
	if (index >= 0)
	{
		mob_cue c = mob_cue (index);
		typedef std::multimap<mob_cue,std::string>::const_iterator N;
		std::pair<N,N> range = mob->cues->equal_range (c);
		std::string world_db = engine.get_config ("world_db");

		if (strcmp (reflex, "delete") == 0
			|| strcmp (reflex, "remove") == 0
			|| strcmp (reflex, "clear") == 0)
		{
			for (N n = range.first; n != range.second; n++)
			{
				std::string cue_string = n->second;
				if (!cue_string.empty ())
				{
					cue_string = std::string (cue) + std::string (" ")
						+ cue_string + " #1deleted#0\n";
					send_to_char (cue_string.c_str (), builder);
				}
			}
			if (engine.in_build_mode ())
			{
				mysql_safe_query
					( "DELETE"
					" FROM %s.cues"
					" WHERE mid = %d"
					" AND cue = %d ",
					world_db.c_str (), mob->nVirtual, index);
			}
			mob->cues->erase (c);
		}
		else if (*reflex)
		{
			std::string reflex_string (reflex);

			if (engine.in_build_mode ())
			{
				mysql_safe_query
					( "INSERT"
					" INTO %s.cues (mid, cue, reflex)"
					" VALUES ('%d', '%d', '%s')",
					world_db.c_str (),
					mob->nVirtual, index, reflex);
			}
			mob->cues->insert (std::make_pair (c, reflex_string));
			reflex_string = std::string (cue) + std::string (" ")
				+ reflex_string + " #6installed#0\n";
			send_to_char (reflex_string.c_str (), builder);
		}// if (*reflex)
		else // list
		{
			for (N n = range.first; n != range.second; n++)
			{
				std::string cue_string = n->second;
				if (!cue_string.empty ())
				{
					cue_string = std::string (cue) + std::string (" ")
						+ cue_string + "\n";
					send_to_char (cue_string.c_str (), builder);
				}
			}
		}
	}
}

void
do_mset (CHAR_DATA * ch, char *argument, int cmd)
{
	char subcmd[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char trigger_name[MAX_INPUT_LENGTH] = { '\0' };
	char type[MAX_STRING_LENGTH];
	CHAR_DATA *edit_mob;
	CHAR_DATA *tch;
	WOUND_DATA *wound = NULL;
	WOUND_DATA *tempwound;
	POISON_DATA *poison;
	AFFECTED_TYPE *af;
	int i;
	int j;
	int delta;
	int ind;
	int full_description = 0;
	int dice;
	int sides;
	int bonus;
	int parms;
	int loads = 0;
	int magic_spell;
	int error = 0;
	int damage, typenum = 0;
	char c;
	char *p;
	bool builder_only = false;
	extern const char *mobprog_triggers[];


	if (IS_NPC (ch))
	{
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	argument = one_argument (argument, subcmd);

	if (IS_MORTAL (ch) && str_cmp (subcmd, "sdesc") && str_cmp (subcmd, "ldesc")
		&& str_cmp (subcmd, "name") && str_cmp (subcmd, "desc"))
	{
		send_to_char
			("Uses: MSET NAME, MSET SDESC, MSET LDESC, MSET DESC or MSET DESC REFORMAT.\n",
			ch);
		return;
	}

	if (!*subcmd || *subcmd == '?')
	{
		give_mset_help (ch);
		return;
	}

	if (isdigit (*subcmd) && (edit_mob = vtom (atoi (subcmd))))
	{
		ch->pc->edit_mob = edit_mob->mob->nVirtual;
		argument = one_argument (argument, subcmd);
	}

	else if (isdigit (*subcmd))
	{
		sprintf (buf, "No such mobile vnum %s\n", subcmd);
		send_to_char (buf, ch);
		send_to_char ("Not editing a mobile now.\n", ch);
		ch->pc->edit_mob = 0;
		return;
	}

	else if (!(edit_mob = vtom (ch->pc->edit_mob)) &&
		!(edit_mob = ch->pc->edit_player))
	{
		send_to_char ("Start by using the MOBILE command.\n", ch);
		return;
	}

	if (ch->pc->edit_player)
		edit_mob = ch->pc->edit_player;

	while (*subcmd)
	{

		if (IS_SET (edit_mob->flags, FLAG_VARIABLE)
			&& str_cmp (subcmd, "variable"))
		{
			send_to_char
				("Don't mset this mob, unless you're removing the variable flag.\n",
				ch);
			return;
		}

		else if (!str_cmp (subcmd, "naughty")) {
			edit_mob->toggleNaughtyFlag();
		}
		else if (!str_cmp (subcmd, "RP")) {
			edit_mob->toggleRPFlag();
		}
		else if (!str_cmp (subcmd, "plot")) {
			edit_mob->togglePlotFlag();
		}

		else if (!strncmp(subcmd,"crafter",7)) {
			if (GET_TRUST(ch) < 5)
			{
				send_to_char ("You must be level 5 to toggle the IsCrafter flag.\n", ch);
				return;
			}

			if (!IS_SET (edit_mob->plr_flags, IS_CRAFTER))
			{
				edit_mob->plr_flags |= IS_CRAFTER;
				send_to_char
					("This admin is now authorised to use cset.\n", ch);
				return;
			}

			edit_mob->plr_flags &= ~IS_CRAFTER;
			send_to_char
				("This admin is no longer authorised to use cset.\n",ch);
			return;
		}
/**
		else if (!str_cmp (subcmd, "spell")) {
			argument = one_argument (argument, buf);
			if (!*buf)
			{
				send_to_char
					("Which spell did you wish to add, edit, or remove?\n", ch);
				return;
			}
			if ((ind = lookup_spell_id (buf)) == -1)
			{
				send_to_char ("That spell was not found in the database.\n",
					ch);
				return;
			}
			if (knows_spell (ch, ind))
			{
				argument = one_argument (argument, buf);
				if (!*buf || atoi (buf) > 100 || atoi (buf) < 0)
				{
					send_to_char
						("Expected a spell proficiency between 0 and 100.\n", ch);
					return;
				}
				if (atoi (buf) > 0)
					set_spell_proficiency (edit_mob, ind, atoi (buf));
				else
					remove_spell (edit_mob, ind);
			}
			else
			{
				argument = one_argument (argument, buf);
				if (!*buf || atoi (buf) > 100 || atoi (buf) < 1)
				{
					send_to_char
						("Expected a spell proficiency between 1 and 100.\n", ch);
					return;
				}
				set_spell_proficiency (edit_mob, ind, atoi (buf));
			}
		}
**/
		else if (!str_cmp (subcmd, "currency"))
		{
			if (!IS_NPC (edit_mob))
			{
				send_to_char ("Currency type may only be set on mobiles.\n",
					ch);
				return;
			}
			argument = one_argument (argument, buf);
			if (!*buf)
			{
				send_to_char
					("Which currency type should they deal in? (black, copper, engren, harad, northman)\n",
					ch);
				return;
			}
			*buf = tolower (*buf);
			if (!str_cmp (buf, "yrcish") || !str_cmp (buf, "black"))
			{
				edit_mob->mob->currency_type = CURRENCY_MORGUL;
				send_to_char
					("That mobile will now deal in Orcish currency (Blacks).\n",
					ch);
			}
			else if (!str_cmp (buf, "gondorian") || !str_cmp (buf, "copper"))
			{
				edit_mob->mob->currency_type = CURRENCY_TIRITH;
				send_to_char
					("That mobile will now deal in Gondorian currency (Coppers).\n",
					ch);
			}

			else if (!str_cmp (buf, "numenorean") || !str_cmp (buf, "engren"))
			{
				edit_mob->mob->currency_type = CURRENCY_EDEN;
				send_to_char
					("That mobile will now deal in Numenorean currency (Engrens).\n",
					ch);
			}

			else if (!strn_cmp (buf, "harad", 5))
			{
				edit_mob->mob->currency_type = CURRENCY_HARAD;
				send_to_char ("That mobile will now deal in Haradaic currency.\n", ch);
			}

			else if (!strn_cmp (buf, "northman", 8))
			{
				edit_mob->mob->currency_type = CURRENCY_NORTHMAN;
				send_to_char ("That mobile will now deal in Northman currency (shillings).\n", ch);
			}

			else
			{
				send_to_char
					("Specify Orcish, Gondorian, Haradaic, Numenorean, or Northman currency.\n", ch);
				return;
			}
		}

		else if (!str_cmp (subcmd, "trigger") || !str_cmp (subcmd, "cue"))
		{
			if (!IS_NPC (edit_mob))
			{
				send_to_char ("Cues may only be set on mobiles.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			mset_cue (ch, edit_mob->mob, buf, argument);
			return;
		}


		else if (!str_cmp (subcmd, "level"))
		{

			if (GET_TRUST (ch) < 5)
			{
				send_to_char ("You cannot assign levels.\n", ch);
				break;
			}

			if (IS_NPC (ch))
			{
				send_to_char ("Change levels on PCs only.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (!str_cmp (buf, "builder-only"))
			{
				builder_only = true;
				argument = one_argument (argument, buf);
			}

			if (!isdigit (*buf) || atoi (buf) > 5)
			{
				send_to_char ("Expected level 0..5\n", ch);
				return;
			}
			edit_mob->pc->level = atoi (buf);
			save_char (edit_mob, true);
			send_to_char ("Player level set.\n", ch);
			return;
		}

		else if (!str_cmp (subcmd, "account"))
		{
			if (GET_TRUST (ch) < 5)
			{
				send_to_char ("This option is only for level 5 admins.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if (!*buf)
			{
				send_to_char
					("Which account did you wish to move this PC to?\n", ch);
				return;
			}
			if (IS_NPC (edit_mob))
			{
				send_to_char ("This command is for PCs only.\n", ch);
				return;
			}
			if (edit_mob->pc->account_name && *edit_mob->pc->account_name)
				free_mem (edit_mob->pc->account_name);
			buf[0] = toupper (buf[0]);
			edit_mob->pc->account_name = duplicateString (buf);
			send_to_char ("Account set.\n", ch);
			save_char (edit_mob, false);
			return;
		}

		else if (!str_cmp (subcmd, "nogain"))
		{
			if (GET_TRUST (ch) < 4)
			{
				send_to_char
					("You must be at least a level four admin to use this flag.\n",
					ch);
				return;
			}
			argument = one_argument (argument, buf);
			if (!*buf)
			{
				send_to_char ("Toggle the nogain flag in which skill?\n", ch);
				return;
			}
			ind = index_lookup (skills, buf);
			if (ind == -1)
			{
				send_to_char ("Unknown skill.\n", ch);
				return;
			}
			af = get_affect (edit_mob, MAGIC_FLAG_NOGAIN + ind);
			if (!af)
			{
				af = new AFFECTED_TYPE;
				af->next = NULL;
				af->type = MAGIC_FLAG_NOGAIN + ind;
				affect_to_char (edit_mob, af);
				send_to_char
					("Skill advancement has been halted in the specified skill.\n",
					ch);
				if (!IS_NPC (edit_mob))
				{
					send_to_char
						("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n",
						ch);
					sprintf (buf, "%s Advancement Halted: %s.", edit_mob->tname,
						skills[ind]);
					do_write (ch, buf, 0);
				}
			}
			else
			{
				affect_remove (edit_mob, af);
				send_to_char
					("The character may now resume advancement in the specified skill.\n",
					ch);
				if (!IS_NPC (edit_mob))
				{
					send_to_char
						("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n",
						ch);
					sprintf (buf, "%s Advancement Resumed: %s.",
						edit_mob->tname, skills[ind]);
					do_write (ch, buf, 0);
				}
			}
			return;
		}

		else if (!str_cmp (subcmd, "sleep"))
		{

			if (!edit_mob->pc)
			{
				send_to_char ("The sleep keyword can only be used against "
					"PCs.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Sleep is in seconds and less than 600 (10 "
					"minutes).\n", ch);
				break;
			}

			if (atoi (buf) > 600)
				edit_mob->pc->sleep_needed = atoi (buf);
			else
				edit_mob->pc->sleep_needed = 100000 * atoi (buf);

			if (sleep_needed_in_seconds (edit_mob) > 600 || atoi (buf) > 600)
			{
				sprintf (buf, "Sleep needed set to %d minutes, %d seconds.\n",
					sleep_needed_in_seconds (edit_mob) / 60,
					sleep_needed_in_seconds (edit_mob) % 60);
				send_to_char (buf, ch);
			}
		}

		else if (!str_cmp (subcmd, "wil"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->wil - atoi (buf);
				edit_mob->wil = atoi (buf);
				GET_WIL (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for wil.\n", ch);
				break;
			}
		}

		else if (!strcasecmp (subcmd, "nopetition")
			|| !strcasecmp (subcmd, "noban")
			|| !strcasecmp (subcmd, "noguest")
			|| !strcasecmp (subcmd, "nopsi")
			|| !strcasecmp (subcmd, "noretire")
			|| !strcasecmp (subcmd, "ipsharingok"))
		{
			if (!IS_NPC (edit_mob) && edit_mob->pc && edit_mob->pc->account_name)
			{

				mset_account_flag (ch, edit_mob->pc->account_name, subcmd);
			}
			else
			{
				send_to_char ("I couldn't access that character's account.\n", ch);
			}
			return;
		}

		else if (!str_cmp (subcmd, "noplayerport") && GET_TRUST (ch) >= 5)
		{
			if (!IS_SET (edit_mob->plr_flags, NO_PLAYERPORT))
			{
				edit_mob->plr_flags |= NO_PLAYERPORT;
				send_to_char
					("That admin is no longer allowed player port access.\n", ch);
				return;
			}

			edit_mob->plr_flags &= ~NO_PLAYERPORT;
			send_to_char
				("That admin will now be allowed to log in to the player port.\n",
				ch);
			return;
		}

		else if (!str_cmp (subcmd, "state"))
		{

			if (IS_NPC (edit_mob))
			{
				send_to_char ("Mobiles don't have a create state.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (!atoi (buf) && !isdigit (*buf))
			{
				send_to_char ("Expected a creation state value -1..4.\n", ch);
				break;
			}

			if (atoi (buf) > 4 || atoi (buf) < -1)
			{
				send_to_char ("Expected a creation state value -1..4.\n", ch);
				break;
			}

			edit_mob->pc->create_state = atoi (buf);

			if (edit_mob->pc->create_state != STATE_DIED)
				edit_mob->flags &= ~FLAG_DEAD;
			else
				edit_mob->flags |= FLAG_DEAD;

			if (edit_mob->pc->create_state != 1)
			{
				strcpy (subcmd, edit_mob->tname);
				*subcmd = toupper (*subcmd);
			}
			else
			{
				strcpy (subcmd, edit_mob->tname);
				*subcmd = toupper (*subcmd);
			}
		}

		else if (!str_cmp (subcmd, "leader1"))
		{
			TOGGLE (edit_mob->flags, FLAG_LEADER_1);
		}

		else if (!str_cmp (subcmd, "leader2"))
		{
			TOGGLE (edit_mob->flags, FLAG_LEADER_2);
		}

		else if (!str_cmp (subcmd, "willskin"))
		{
			TOGGLE (edit_mob->flags, FLAG_WILLSKIN);
		}
		/** morphing mobs **/
		else if (!str_cmp (subcmd, "morphto"))
		{
			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected a mob Vnum.\n", ch);
				break;
			}

			edit_mob->morphto = atoi (buf);
		}

		else if (!str_cmp (subcmd, "clock"))
		{

			int month, day, hour;

			argument = one_argument (argument, buf);
			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of months.\n", ch);
				break;
			}
			month = atoi (buf);

			argument = one_argument (argument, buf);
			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of days.\n", ch);
				break;
			}
			day = atoi (buf);

			argument = one_argument (argument, buf);
			if (!isdigit (*buf))
			{
				send_to_char ("Expected a number of hours.\n", ch);
				break;
			}
			hour = atoi (buf);

			edit_mob->clock = ((month * 30 * 24) + (day * 24) + hour);
		}

		else if (!str_cmp (subcmd, "morphtype"))
		{
			int mtype;

			argument = one_argument (argument, buf);
			if (!isdigit (*buf))
			{
				send_to_char ("Expected 1 for physical morph, 2 for skill morph.\n", ch);
				break;
			}
			mtype = atoi (buf);
			if (mtype > 2 || mtype < 0)
				mtype = 1;
			edit_mob->morph_type = mtype;

		}

		else if (!str_cmp (subcmd, "openinghour"))
		{
			argument = one_argument (argument, buf);
			if (!*buf || !isdigit (*buf) || atoi (buf) > 24 || atoi (buf) < 1)
			{
				send_to_char ("Expected a value from 1 to 24.\n", ch);
				return;
			}
			if (!IS_SET (edit_mob->flags, FLAG_KEEPER))
			{
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else
				edit_mob->shop->opening_hour = atoi (buf);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if (!str_cmp (subcmd, "closinghour"))
		{
			argument = one_argument (argument, buf);
			if (!*buf || !isdigit (*buf) || atoi (buf) > 24 || atoi (buf) < 1)
			{
				send_to_char ("Expected a value from 1 to 24.\n", ch);
				return;
			}
			if (!IS_SET (edit_mob->flags, FLAG_KEEPER))
			{
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else
				edit_mob->shop->closing_hour = atoi (buf);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if (!str_cmp (subcmd, "shop_exit"))
		{
			argument = one_argument (argument, buf);
			if (!*buf || !isdigit (*buf) || atoi (buf) > 5 || atoi (buf) < 0)
			{
				send_to_char ("Expected a value from 0 to 5.\n", ch);
				send_to_char ("North: 0, East: 1, South: 2,\n", ch);
				send_to_char ("West: 3, Up: 4, Down: 5\n", ch);
				return;
			}
			if (!IS_SET (edit_mob->flags, FLAG_KEEPER))
			{
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else
				edit_mob->shop->exit = atoi (buf);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if (!str_cmp (subcmd, "keeper"))
		{
			if (IS_SET (edit_mob->flags, FLAG_KEEPER))
			{
				edit_mob->flags &= ~FLAG_KEEPER;
				send_to_char ("Note:  Keeper flag removed.  Shop data will "
					"be deleted when the zone\n"
					"       is saved.\n", ch);
			}

			else
			{
				edit_mob->flags |= FLAG_KEEPER;

				if (!edit_mob->shop) {
					edit_mob->shop = new SHOP_DATA;
					edit_mob->shop->negotiations = NULL;
				}

				edit_mob->shop->discount = 1.0;
				edit_mob->shop->markup = 1.0;
			}
		}

		else if (!str_cmp (subcmd, "ostler"))
		{
			if (GET_FLAG (edit_mob, FLAG_OSTLER))
				edit_mob->flags &= ~FLAG_OSTLER;
			else if (!GET_FLAG (edit_mob, FLAG_KEEPER))
				send_to_char ("Assign KEEPER flag before OSTLER.\n", ch);
			else
				edit_mob->flags |= FLAG_OSTLER;
		}

		else if (!str_cmp (subcmd, "markup"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a markup."
					"\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->markup);
			else
			{
				send_to_char ("Expected a markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "discount"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->discount);
			else
			{
				send_to_char ("Expected a discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount1") ||
			!str_cmp (subcmd, "econ_discount1") ||
			!str_cmp (subcmd, "discount1"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount1);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup1") ||
			!str_cmp (subcmd, "econ_markup1") ||
			!str_cmp (subcmd, "markup1"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup1);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount2") ||
			!str_cmp (subcmd, "econ_discount2") ||
			!str_cmp (subcmd, "discount2"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount2);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup2") ||
			!str_cmp (subcmd, "econ_markup2") ||
			!str_cmp (subcmd, "markup2"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup2);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount3") ||
			!str_cmp (subcmd, "econ_discount3") ||
			!str_cmp (subcmd, "discount3"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount3);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup3") ||
			!str_cmp (subcmd, "econ_markup3") ||
			!str_cmp (subcmd, "markup3"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup3);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount4") ||
			!str_cmp (subcmd, "econ_discount4") ||
			!str_cmp (subcmd, "discount4"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount4);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup4") ||
			!str_cmp (subcmd, "econ_markup4") ||
			!str_cmp (subcmd, "markup4"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup4);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount5") ||
			!str_cmp (subcmd, "econ_discount5") ||
			!str_cmp (subcmd, "discount5"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount5);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup5") ||
			!str_cmp (subcmd, "econ_markup5") ||
			!str_cmp (subcmd, "markup5"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup5);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount6") ||
			!str_cmp (subcmd, "econ_discount6") ||
			!str_cmp (subcmd, "discount6"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount6);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup6") ||
			!str_cmp (subcmd, "econ_markup6") ||
			!str_cmp (subcmd, "markup6"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup6);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-discount7") ||
			!str_cmp (subcmd, "econ_discount7") ||
			!str_cmp (subcmd, "discount7"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_discount7);
			else
			{
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ-markup7") ||
			!str_cmp (subcmd, "econ_markup7") ||
			!str_cmp (subcmd, "markup7"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a "
					"economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '.' || isdigit (*buf))
				sscanf (buf, "%f", &edit_mob->shop->econ_markup7);
			else
			{
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "econ"))
		{
			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) != -1)
			{

				if (!edit_mob->shop)
				{
					send_to_char ("Assign the KEEPER flag before setting econ "
						"flags.\n", ch);
					break;
				}

				TOGGLE (edit_mob->shop->econ_flags1, 1 << ind);

				if (!str_cmp (econ_flags[ind], "nobarter"))
					send_to_char
					("Nobarter set as an econ flag.  Did you mean to "
					"set it as a nobuy flag?\nAs in 'mset nobuy "
					"nobarter'?\n", ch);
			}
			else
				send_to_char ("I couldn't locate that econ flag.\n", ch);
		}

		else if (!str_cmp (subcmd, "nobuy"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->nobuy_flags, 1 << ind);
		}


		else if (!str_cmp (subcmd, "buy"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->buy_flags, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ1"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ1 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags1, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ2"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ2 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags2, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ3"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ3 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags3, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ4"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ4 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags4, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ5"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ5 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags5, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ6"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ6 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags6, 1 << ind);
		}

		else if (!str_cmp (subcmd, "econ7"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting econ "
					"flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				send_to_char ("No such econ7 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags7, 1 << ind);
		}


		else if (!str_cmp (subcmd, "shop"))
		{

			argument = one_argument (argument, buf);

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a shop."
					"\n", ch);
				break;
			}

			if (*buf != '-' && !isdigit (*buf))
			{
				send_to_char ("Expected a room vnum or -1 (any room) after "
					" the 'shop' keyword.\n", ch);
				break;
			}

			if (atoi (buf) > 0 && !vtor (atoi (buf)))
			{
				send_to_char ("The shop room vnum specified doesn't exist.\n",
					ch);
				break;
			}

			edit_mob->shop->shop_vnum = atoi (buf);
		}

		else if (!str_cmp (subcmd, "store"))
		{

			argument = one_argument (argument, buf);

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before setting a store."
					"\n", ch);
				break;
			}

			if (*buf != '-' && !isdigit (*buf))
			{
				send_to_char ("Expected a room vnum or -1 (none) after "
					" the 'store' keyword.\n", ch);
				break;
			}

			if (atoi (buf) > 0 && !vtor (atoi (buf)))
			{
				send_to_char ("The store room vnum specified doesn't exist.\n",
					ch);
				break;
			}

			edit_mob->shop->store_vnum = atoi (buf);
		}

		else if (!str_cmp (subcmd, "delivery"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("Assign the KEEPER flag before using delivery."
					"\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);

			do
			{
				if (!isdigit (*buf2))
				{
					error = 1;
					sprintf (buf, "%d is not a delivery object vnum.\n",
						atoi (buf2));
					send_to_char (buf, ch);
					break;
				}

				if (!vtoo ((j = atoi (buf2))))
				{
					error = 1;
					sprintf (buf, "%d is not defined.\n", atoi (buf2));
					send_to_char (buf, ch);
					break;
				}

				for (ind = -1, i = MAX_DELIVERIES - 1; i >= 0; i--)
				{

					if (edit_mob->shop->delivery[i] == -1)
						edit_mob->shop->delivery[i] = 0;

					if (!edit_mob->shop->delivery[i])
						ind = i;

					else if (edit_mob->shop->delivery[i] == j)
					{
						ind = -2;
						break;
					}
				}

				if (ind == -2)
					edit_mob->shop->delivery[i] = 0;
				else if (i == -1 && ind == -1)
					send_to_char ("Delivery table is full, sorry.\n", ch);
				else if (i == -1 && ind != -1)
					edit_mob->shop->delivery[ind] = j;

				p = one_argument (p, buf2);

			}
			while (*buf2);
		}

		else if (!str_cmp (subcmd, "newbie"))
		{
			if (!IS_SET (edit_mob->plr_flags, NEWBIE))
			{
				edit_mob->plr_flags |= NEWBIE;
				send_to_char ("Newbie bit enabled.\n", ch);
			}
			else
			{
				edit_mob->plr_flags &= ~NEWBIE;
				send_to_char ("Newbie bit removed.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "allskills"))
		{

			for (ind = 1; ind <= LAST_SKILL; ind++)
			{
				open_skill (edit_mob, ind);
				if (IS_NPC (edit_mob))
					edit_mob->skills[ind] = 100;
				else
				{
					edit_mob->pc->skills[ind] = 100;
					edit_mob->skills[ind] = 100;
				}
			}
		}

		else if (!str_cmp (subcmd, "noskills"))
		{
			for (ind = 1; ind <= LAST_SKILL; ind++)
			{
				if (IS_NPC (edit_mob))
					edit_mob->skills[ind] = 0;
				else
				{
					if (ind == SKILL_SPEAK_WESTRON)
					{
						send_to_char ("NOTE:  Leaving Westron set.\n", ch);
						continue;
					}

					i = edit_mob->pc->skills[ind];
					edit_mob->pc->skills[ind] = 0;
					edit_mob->skills[ind] -= i;
				}
			}
		}

		else if (!str_cmp (subcmd, "speaks") || !str_cmp (subcmd, "speak"))
		{

			argument = one_argument (argument, subcmd);

			if ((ind = index_lookup (skills, subcmd)) == -1)
			{
				send_to_char ("Unknown language.\n", ch);
				break;
			}

			if (!edit_mob->skills[ind])
				send_to_char ("NOTE:  Mob doesn't know that language.\n", ch);

			edit_mob->speaks = ind;
		}

		else if (!str_cmp (subcmd, "material")
			|| !str_cmp (subcmd, "materials"))
		{
			if (!edit_mob->shop)
			{
				send_to_char ("That mob isn't a shopkeeper.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);

			if (!*buf2)
			{
				send_to_char ("Which materials did you wish to set?\n", ch);
				return;
			}

			do
			{
				if ((ind = index_lookup (materials, buf2)) == -1)
				{
					sprintf (buf, "Unknown material: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				if (IS_SET (edit_mob->shop->materials, 1 << ind))
					edit_mob->shop->materials &= ~(1 << ind);
				else
					edit_mob->shop->materials |= (1 << ind);

				p = one_argument (p, buf2);

			}
			while (*buf2);

			if (error)
				break;
		}

		else if (!str_cmp (subcmd, "trades") || !str_cmp (subcmd, "tradesin"))
		{

			if (!edit_mob->shop)
			{
				send_to_char ("That mob isn't a shopkeeper.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);

			do
			{
				if ((ind = index_lookup (item_types, buf2)) == -1)
				{
					sprintf (buf, "Unknown item-type: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				for (j = -1, i = MAX_TRADES_IN - 1; i >= 0; i--)
				{

					if (!edit_mob->shop->trades_in[i])
						j = i;

					else if (edit_mob->shop->trades_in[i] == ind)
					{
						edit_mob->shop->trades_in[i] = 0;
						break;
					}
				}

				if (j == -1 && i == -1)
				{
					sprintf (buf, "Trades table full, sorry: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				else if (j != -1 && i == -1)
					edit_mob->shop->trades_in[j] = ind;

				p = one_argument (p, buf2);

			}
			while (*buf2);

			if (error)
				break;
		}


		else if (!str_cmp (subcmd, "access"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);

			do
			{
				if ((ind = index_lookup (room_bits, buf2)) == -1)
				{
					sprintf (buf, "Unknown room-flag: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				if (IS_SET (edit_mob->mob->access_flags, 1 << ind))
					edit_mob->mob->access_flags &= ~(1 << ind);
				else
					edit_mob->mob->access_flags |= (1 << ind);

				p = one_argument (p, buf2);

			}
			while (*buf2);

			if (error)
				break;
		}

		else if (!str_cmp (subcmd, "noaccess"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);

			do
			{
				if ((ind = index_lookup (room_bits, buf2)) == -1)
				{
					sprintf (buf, "Unknown room-flag: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				if (IS_SET (edit_mob->mob->noaccess_flags, 1 << ind))
					edit_mob->mob->noaccess_flags &= ~(1 << ind);
				else
					edit_mob->mob->noaccess_flags |= (1 << ind);

				p = one_argument (p, buf2);

			}
			while (*buf2);

			if (error)
				break;
		}

		else if (!str_cmp (subcmd, "jailer"))
		{
			if (IS_SET (edit_mob->act, ACT_JAILER))
			{
				edit_mob->act &= ~ACT_JAILER;
				send_to_char ("Creature is no longer flagged as a jailer.\n",
					ch);
			}
			else
			{
				edit_mob->act |= ACT_JAILER;
				send_to_char
					("Creature is now flagged as a jailer. Be SURE to set\n"
					"the cell1, cell2, and cell3 values.\n", ch);
			}
		}
		else if (!str_cmp (subcmd, "jail"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || (atoi (buf) && atoi (buf) >= 0))
			{

				sprintf (buf2, "Old jail VNUM: %d.\nNew jail VNUM: %d.\n",
					edit_mob->mob->jail, atoi (buf));
				edit_mob->mob->jail = atoi (buf);
				send_to_char (buf2, ch);
			}

			else if (atoi (buf) && atoi (buf) < 0)
			{
				send_to_char ("Expected a value of at least 0.\n", ch);
				return;
			}

			else
			{
				send_to_char
					("Expected the integer value of the target room's VNUM.\n",
					ch);
				return;
			}

		}

		else if (!str_cmp (subcmd, "cell1") || !str_cmp (subcmd, "cell2")
			|| !str_cmp (subcmd, "cell3"))
		{
			argument = one_argument (argument, buf);

			if ((*buf == '0' || (atoi (buf) && atoi (buf) >= 0))
				&& !str_cmp (subcmd, "cell1"))
			{
				sprintf (buf2, "Old cell1 VNUM: %d.\nNew cell1 VNUM: %d.\n",
					edit_mob->cell_1, atoi (buf));
				edit_mob->cell_1 = atoi (buf);
				send_to_char (buf2, ch);
			}

			else if ((*buf == '0' || (atoi (buf) && atoi (buf) >= 0))
				&& !str_cmp (subcmd, "cell2"))
			{
				sprintf (buf2, "Old cell2 VNUM: %d.\nNew cell2 VNUM: %d.\n",
					edit_mob->cell_2, atoi (buf));
				edit_mob->cell_2 = atoi (buf);
				send_to_char (buf2, ch);
			}

			else if ((*buf == '0' || (atoi (buf) && atoi (buf) >= 0))
				&& !str_cmp (subcmd, "cell3"))
			{
				sprintf (buf2, "Old cell3 VNUM: %d.\nNew cell3 VNUM: %d.\n",
					edit_mob->cell_3, atoi (buf));
				edit_mob->cell_3 = atoi (buf);
				send_to_char (buf2, ch);
			}

			else if (atoi (buf) && atoi (buf) < 0)
			{
				send_to_char ("Expected a value of at least 0.\n", ch);
				return;
			}

			else
			{
				send_to_char
					("Expected the integer value of the target room's VNUM.\n",
					ch);
				return;
			}
		}

		else if (!str_cmp (subcmd, "hits") || !str_cmp (subcmd, "hp"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->hit = atoi (buf);
				if (IS_NPC (edit_mob))
					edit_mob->max_hit = edit_mob->hit;
			}
			else
			{
				send_to_char ("Expected hitpoints value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "desc") ||
			!str_cmp (subcmd, "descr") || !str_cmp (subcmd, "description"))
		{
			one_argument (argument, buf);
			if (!str_cmp (buf, "reformat"))
			{
				argument = one_argument (argument, buf);
				reformat_desc (edit_mob->description, &edit_mob->description);
				send_to_char (edit_mob->description, ch);
			}
			else
				full_description = 1;
		}

		else if (!str_cmp (subcmd, "intoxication"))
		{

			argument = one_argument (argument, buf);

			if (!just_a_number (buf))
			{
				send_to_char ("Expected ... intoxication <num>\n", ch);
				break;
			}

			edit_mob->intoxication = atoi (buf);
		}

		else if (!str_cmp (subcmd, "hunger"))
		{

			argument = one_argument (argument, buf);

			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected ... hunger <num>\n", ch);
				break;
			}

			edit_mob->hunger = atoi (buf);
		}

		else if (!str_cmp (subcmd, "thirst"))
		{

			argument = one_argument (argument, buf);

			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected ... thirst <num>\n", ch);
				break;
			}

			edit_mob->thirst = atoi (buf);
		}

		else if (!str_cmp (subcmd, "conds"))
		{
			for (i = 0; i < 3; i++)
			{
				argument = one_argument (argument, buf);

				if (!isdigit (*buf) && !atoi (buf))
				{
					send_to_char ("You should enter 3 numbers after "
						"conds.\n", ch);
					send_to_char ("Note:  Use mset intoxication, hunger, "
						"thirst instead.\n", ch);
					break;
				}

				if (i == 0)
					edit_mob->intoxication = atoi (buf);
				else if (i == 1)
					edit_mob->hunger = atoi (buf);
				else
					edit_mob->thirst = atoi (buf);
			}

			if (!isdigit (*buf))
				break;
		}

		else if (!str_cmp (subcmd, "room"))
		{

			if (IS_NPC (edit_mob))
			{
				send_to_char
					("This only works on PCs.  For an NPC, switch into the target mob, then\nuse GOTO.\n",
					ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || !vtor (atoi (buf)))
			{
				send_to_char ("No such room.\n", ch);
				break;
			}

			if (edit_mob->room)
			{
				char_from_room (edit_mob);
				char_to_room (edit_mob, atoi (buf));
			}
			else
				edit_mob->in_room = atoi (buf);
		}

		else if (!str_cmp (subcmd, "hitch"))
		{
			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			if (edit_mob->mob->vehicle_type == VEHICLE_HITCH)
			{
				edit_mob->mob->vehicle_type = 0;
				send_to_char ("Hitch flag removed.\n", ch);
			}
			else
			{
				edit_mob->mob->vehicle_type = VEHICLE_HITCH;
				edit_mob->act |= ACT_VEHICLE;
				edit_mob->act |= ACT_MOUNT;

				sprintf (buf, "Note:  PCs entering hitch goto room %d.\n",
					edit_mob->mob->nVirtual);
				send_to_char (buf, ch);
			}
		}

		else if (!str_cmp (subcmd, "nobleed"))
		{
			if (IS_SET (edit_mob->act, ACT_NOBLEED))
			{
				send_to_char ("This mobile will now bleed from heavy wounds.\n",
					ch);
				edit_mob->act &= ~ACT_NOBLEED;
			}
			else
			{
				edit_mob->act |= ACT_NOBLEED;
				send_to_char
					("This mobile will no longer bleed from heavy wounds.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "prey"))
		{
			if (IS_SET (edit_mob->act, ACT_PREY))
			{
				send_to_char
					("This mobile will no longer flee from any approaching creature.\n",
					ch);
				edit_mob->act &= ~ACT_PREY;
			}
			else
			{
				send_to_char
					("This mobile will now flee from any approaching creature.\n",
					ch);
				edit_mob->act |= ACT_PREY;
			}
		}
		else if (!str_cmp (subcmd, "flying"))
		{
			if (IS_SET (edit_mob->act, ACT_FLYING))
			{
				send_to_char ("This mobile will no longer fly.\n", ch);
				edit_mob->act &= ~ACT_FLYING;
			}
			else
			{
				edit_mob->act |= ACT_FLYING;
				send_to_char ("This mobile will now fly.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "physician"))
		{
			if (IS_SET (edit_mob->act, ACT_PHYSICIAN))
			{
				send_to_char ("This mobile will no longer treat wounds.\n", ch);
				edit_mob->act &= ~ACT_PHYSICIAN;
			}
			else
			{
				edit_mob->act |= ACT_PHYSICIAN;
				send_to_char ("This mobile will now treat wounds.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "repairman"))
		{
			if (IS_SET (edit_mob->act, ACT_REPAIR))
			{
				send_to_char ("This mobile will no longer repair items.\n", ch);
				edit_mob->act &= ~ACT_REPAIR;
			}
			else
			{
				edit_mob->act |= ACT_REPAIR;
				send_to_char ("This mobile will now repair items.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "nobind"))
		{
			if (IS_SET (edit_mob->act, ACT_NOBIND))
			{
				send_to_char ("This mobile will now bind its own wounds.\n",
					ch);
				edit_mob->act &= ~ACT_NOBIND;
			}
			else
			{
				edit_mob->act |= ACT_NOBIND;
				send_to_char
					("This mobile will no longer bind its own wounds.\n", ch);
			}
		}

		else if (!str_cmp (subcmd, "boat"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			edit_mob->mob->vehicle_type = VEHICLE_BOAT;
			edit_mob->act |= ACT_VEHICLE;

			sprintf (buf, "Note:  PCs entering boat goto room %d.\n",
				edit_mob->mob->nVirtual);
			send_to_char (buf, ch);
		}

		else if (!str_cmp (subcmd, "helm"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->mob->helm_room = atoi (buf);
				if (edit_mob->mob->helm_room != 0
					&& !vtor (edit_mob->mob->helm_room))
					send_to_char ("NOTE:  No room exists for helm.\n", ch);
			}
			else
			{
				send_to_char ("Expected a room vnum after helm.\n", ch);
				break;
			}

			if (!edit_mob->mob->vehicle_type)
				send_to_char ("NOTE:  Remember to set a vehicle type - BOAT or "
				"HITCH.\n", ch);
		}

		else if (!str_cmp (subcmd, "moves"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->move = atoi (buf);
			}
			else
			{
				send_to_char ("Expected moves value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "damage"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) >= 0)
			{
				edit_mob->damage = atoi (buf);
				break;
			}

			else
				send_to_char ("Expected a positive value.\n", ch);

		}

		else if (!str_cmp (subcmd, "maxmoves"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->max_move = atoi (buf);
			}
			else
			{
				send_to_char ("Expected moves value.\n", ch);
				break;
			}
		}

		else if (GET_TRUST (ch) >= 4 && !str_cmp (subcmd, "delete"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("NPC only setting:  delete\n", ch);
				break;
			}

			if (edit_mob->deleted)
			{
				edit_mob->deleted = 0;
				send_to_char ("This mobile is no longer marked for deletion."
					"\n", ch);
				break;
			}
			
			if (IS_NPC(edit_mob))
			{
				loads = 0;
			//for (tch = character_list; tch; tch = tch->next)
			for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				tch = *tch_iterator;

				if (tch->deleted)
					continue;

				if (!IS_NPC (tch))
					continue;

				if (tch->mob->nVirtual == edit_mob->mob->nVirtual)
							//break;
					loads++;
			}
			}

				//if (tch)
			if(loads > 0)
			{
				send_to_char ("Clear this mobile from the world first.\n", ch);
				return;
			}

			send_to_char ("WARNING:  This mobile is cleared from the world.  "
				"However, the prototype\n"
				"          cannot be removed until the zone is saved,"
				"and the mud is rebooted.\n"
				"          Use the DELETE option again to undo "
				"deletion.\n", ch);

			edit_mob->deleted = 1;
		}

		else if (!str_cmp (subcmd, "attack"))
		{

			argument = one_argument (argument, buf2);

			if ((ind = index_lookup (attack_names, buf2)) == -1)
			{
				send_to_char ("No such attack method.\n", ch);
				break;
			}

			edit_mob->nat_attack_type = ind;
		}

		else if (!str_cmp (subcmd, "natural"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_mob->natural_delay = atoi (buf);
			else
			{
				send_to_char ("Expected natural delay value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "str"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) < 1)
			{
				send_to_char ("Expected a positive value.\n", ch);
				return;
			}

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->str - atoi (buf);
				edit_mob->str = atoi (buf);
				GET_STR (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for str.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "dex"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) < 1)
			{
				send_to_char ("Expected a positive value.\n", ch);
				return;
			}

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->dex - atoi (buf);
				edit_mob->dex = atoi (buf);
				GET_DEX (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for dex.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "con"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) < 1)
			{
				send_to_char ("Expected a positive value.\n", ch);
				return;
			}

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->con - atoi (buf);
				edit_mob->con = atoi (buf);
				GET_CON (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for con.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "int"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) < 1)
			{
				send_to_char ("Expected a positive value.\n", ch);
				return;
			}

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->intel - atoi (buf);
				edit_mob->intel = atoi (buf);
				GET_INT (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for int.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "pow"))
		{
			argument = one_argument (argument, buf);

			if (GET_TRUST(ch) >= 5) {
				if (atoi (buf) < 1)
				{
					send_to_char ("Expected a positive value.\n", ch);
					return;
				}

				if (*buf == '0' || atoi (buf))
				{
					delta = edit_mob->aur - atoi (buf);
					edit_mob->aur = atoi (buf);
					GET_AUR (edit_mob) -= delta;
				}
				else
				{
					send_to_char ("Expected a value for aur.\n", ch);
					break;
				}
			}
			else {
				send_to_char("Your admin level is not high enough to edit power.\n", ch);
				return;
			}
		}

		else if (!str_cmp (subcmd, "agi"))
		{
			argument = one_argument (argument, buf);

			if (atoi (buf) < 1)
			{
				send_to_char ("Expected a positive value.\n", ch);
				return;
			}

			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->agi - atoi (buf);
				edit_mob->agi = atoi (buf);
				GET_AGI (edit_mob) -= delta;
			}
			else
			{
				send_to_char ("Expected a value for agi.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "dam"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("NPC only setting:  dam\n", ch);
				break;
			}

			bonus = 0;

			argument = one_argument (argument, buf);

			parms = sscanf (buf, "%d%[dD]%d%d", &dice, &c, &sides, &bonus);

			if (parms < 3)
			{
				send_to_char ("The dam parameter format is #d#+/-#.\n", ch);
				break;
			}

			edit_mob->mob->damnodice = dice;
			edit_mob->mob->damsizedice = sides;
			edit_mob->mob->damroll = bonus;
		}

		else if (!str_cmp (subcmd, "armor") || !str_cmp (subcmd, "ac"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_mob->armor = atoi (buf);
			else
			{
				send_to_char ("Expected armor/ac value.\n", ch);
				break;
			}
		}


		else if (!str_cmp (subcmd, "fightmode"))
		{

			argument = one_argument (argument, buf);

			if (!str_cmp (buf, "frantic"))
				edit_mob->fight_mode = 0;
			else if (!str_cmp (buf, "aggressive"))
				edit_mob->fight_mode = 1;
			else if (!str_cmp (buf, "normal"))
				edit_mob->fight_mode = 2;
			else if (!str_cmp (buf, "cautious"))
				edit_mob->fight_mode = 3;
			else if (!str_cmp (buf, "defensive"))
				edit_mob->fight_mode = 4;
			else
			{
				send_to_char ("Unknown fight mode.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "circle"))
		{
			argument = one_argument (argument, buf);

			if (isdigit (*buf) && atoi (buf) <= 9)
				edit_mob->circle = atoi (buf);
			else
			{
				send_to_char ("Expected circle 0..9.\n", ch);
				break;
			}
		}

		else if (index_lookup (wound_locations, subcmd) != -1
			&& !full_description)
		{

			if (IS_NPC (edit_mob))
			{
				send_to_char ("This command may only be used on PCs.\n", ch);
				return;
			}

			if (!IS_IMPLEMENTOR (ch))
			{
				send_to_char
					("Sorry, but you should not be using this command.\n", ch);
				return;
			}

			for (i = 0; wound_locations[i]; i++)
				if (!strn_cmp (subcmd, wound_locations[i], strlen (subcmd)))
					sprintf (subcmd, wound_locations[i]);

			if ((sscanf (argument, "%d %s", &damage, type)) != 2 && damage != 0)
			{
				send_to_char
					("You must specify both the damage value and wound type.\n",
					ch);
				break;
			}

			if (damage != 0)
			{
				if (!str_cmp (type, "slash"))
					typenum = 2;
				else if (!str_cmp (type, "pierce"))
					typenum = 1;
				else if (!str_cmp (type, "blunt"))
					typenum = 3;
				else if (!str_cmp (type, "frost"))
					typenum = 5;
				else if (!str_cmp (type, "burn"))
					typenum = 6;
				else if (!str_cmp (type, "bite"))
					typenum = 7;
				else if (!str_cmp (type, "claw"))
					typenum = 8;
				else if (!str_cmp (type, "fist"))
					typenum = 9;
				else
				{
					send_to_char
						("Invalid wound type. Must be set to one of the following:\n",
						ch);
					send_to_char
						("slash, pierce, blunt, frost, burn, bite, claw, or fist.\n",
						ch);
					break;
				}
			}

			if ((damage <= 100 && damage > 0) && (typenum > 0 && typenum <= 9))
			{
				wound_to_char (edit_mob, duplicateString (subcmd), damage, typenum, 0,
					0, 0);
				break;
			}

			else if (damage == 0)
			{

				for (tempwound = edit_mob->wounds;
					tempwound; tempwound = tempwound->next)
				{
					if (wound != NULL)
						continue;
					if (!str_cmp (tempwound->location, subcmd))
						wound = tempwound;
				}

				wound_from_char (edit_mob, wound);
				break;

			}

			else
			{
				if (damage > 50 || damage < 0)
					send_to_char ("Expected a damage value from 0 to 100.\n", ch);
				if (typenum > 9 || typenum < 1)
					send_to_char ("Expected a wound type vaue from 1 to 9.\n",
					ch);
				break;
			}
		}


		else if (!str_cmp (subcmd, "piety"))
		{
			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
				edit_mob->ppoints = atoi (buf);
			else
			{
				send_to_char ("Expected piety value.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "skinobj"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->mob->skinned_vnum = atoi (buf);
				if (!vtoo (atoi (buf)))
					send_to_char ("NOTE:  No such object exists.\n", ch);
			}
			else
			{
				send_to_char ("Expected vnum of skinned item.\n", ch);
				break;
			}
		}


		else if (!str_cmp (subcmd, "carcobj") || !str_cmp (subcmd, "carcass"))
		{

			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf == '0' || atoi (buf))
			{
				edit_mob->mob->carcass_vnum = atoi (buf);
				if (!vtoo (atoi (buf)))
					send_to_char ("NOTE:  No such object exists.\n", ch);
			}
			else
			{
				send_to_char ("Expected vnum of carcass item.\n", ch);
				break;
			}
		}

		else if (!str_cmp (subcmd, "merch_seven"))
		{
			if (!IS_NPC (edit_mob))
			{
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if (*buf != '\0')
				edit_mob->mob->merch_seven = atoi (buf);
			else
			{
				send_to_char ("Expected a zero or one for merch_seven\n", ch);
				break;
			}
		}

		else if ((ind = index_lookup (position_types, subcmd)) != -1)
		{

			edit_mob->position = ind;

			if (edit_mob->default_pos == 0)
				edit_mob->default_pos = ind;
		}
		else if ((ind = index_lookup (sex_types, subcmd)) != -1)
			edit_mob->sex = ind;

		else if (!str_cmp (subcmd, "deity"))
		{

			argument = one_argument (argument, buf);

			if ((ind = index_lookup (deity_name, buf)) != -1)
				edit_mob->deity = ind;

		}

		else if (!str_cmp (subcmd, "variable"))
		{
			if (GET_TRUST (ch) < 4)
			{
				send_to_char
					("You must be a level four admin to set this bit.\n", ch);
				return;
			}
			if (!IS_SET (edit_mob->flags, FLAG_VARIABLE))
			{
				edit_mob->flags |= FLAG_VARIABLE;
				send_to_char ("This mob will now randomize at load-up.\n\n",
					ch);
				send_to_char
					("#1Be sure to reboot the builder port before loading this mob up for the first time!#0\n",
					ch);
				return;
			}
			else
			{
				edit_mob->flags &= ~FLAG_VARIABLE;
				send_to_char ("This mob will no longer randomize at load-up.\n",
					ch);
				return;
			}
		}

		else if (!str_cmp (subcmd, "isadmin"))
		{
			if (GET_TRUST (ch) < 5)
			{
				send_to_char ("You'll need a level 5 admin to set this bit.\n",
					ch);
				return;
			}
			if (IS_NPC (edit_mob) || GET_TRUST (edit_mob))
			{
				send_to_char
					("This bit is meant only to be set on mortal PCs.\n", ch);
				return;
			}
			if (IS_SET (edit_mob->flags, FLAG_ISADMIN))
				edit_mob->flags &= ~FLAG_ISADMIN;
			else
				edit_mob->flags |= FLAG_ISADMIN;
			send_to_char ("IsAdminPC flag toggled on character.\n", ch);
			return;
		}

		else if (!str_cmp (subcmd, "race"))
		{

			argument = one_argument (argument, buf);

			if ((ind = lookup_race_id (buf)) == -1)
				send_to_char ("That race isn't in the database, I'm afraid.\n",
				ch);
			else
				edit_mob->race = ind;

			argument = one_argument (argument, buf);

			if (!str_cmp (buf, "defaults") && ind <= LAST_PC_RACE)
			{
				randomize_mobile (edit_mob);
				send_to_char
					("Stats and skills re-initialized to race defaults.\n", ch);
				return;
			}

			refresh_race_configuration (edit_mob);
		}

		else if (!str_cmp (subcmd, "frame"))
		{

			argument = one_argument (argument, buf);

			ind = index_lookup (frames, buf);

			if ((ind == -1))
			{
				send_to_char ("Expected a frame: scant, light, ... \n", ch);
				break;
			}

			edit_mob->frame = ind;
		}

		else if (!str_cmp (subcmd, "height"))
		{

			argument = one_argument (argument, buf);

			if (!isdigit (*buf))
			{
				send_to_char ("Expected height.\n", ch);
				break;
			}

			edit_mob->height = atoi (buf);
		}

		else if ((ind = index_lookup (skills, subcmd)) != -1)
		{
			argument = one_argument (argument, buf);
			if (isdigit (*buf))
			{
				if (atoi (buf) < 0)
				{
					send_to_char ("Please specify a positive value.\n",
						ch);
					return;
				}
				if (ind == SKILL_CLAIRVOYANCE || ind == SKILL_DANGER_SENSE
					|| ind == SKILL_EMPATHIC_HEAL || ind == SKILL_HEX
					|| ind == SKILL_MENTAL_BOLT || ind == SKILL_PRESCIENCE
					|| ind == SKILL_SENSITIVITY || ind == SKILL_TELEPATHY)
				{
					if (GET_TRUST(ch) < 5)
					{
						send_to_char
							("Psionics may only be rolled randomly at character creation.\n",
							ch);
						return;
					}
				}
				open_skill (edit_mob, ind);
				if (IS_NPC (edit_mob))
				{
					edit_mob->skills[ind] = atoi (buf);
				}
				else
				{
					edit_mob->pc->skills[ind] = atoi (buf);
					edit_mob->skills[ind] = atoi (buf);
				}
			}
			else
			{
				send_to_char
					("Expected a value after the skill, from 0 to 100.\n", ch);
				break;
			}

			fix_offense (edit_mob);
		}

		else if ((ind = index_lookup (frames, subcmd) != -1))
			edit_mob->frame = ind;

		else if (!str_cmp (subcmd, "autoflee"))
		{			/* Need {}'s for macro */
			TOGGLE (edit_mob->flags, FLAG_AUTOFLEE);
		}

		else if ((ind = index_lookup (action_bits, subcmd)) != -1)
		{
			if (!strcmp (subcmd, "stayput"))
			{
				send_to_char
					("See HELP STAYPUT for usage of this functionality.\n", ch);
				return;
			}
			if (IS_SET (edit_mob->act, 1 << ind))
			{
				edit_mob->act &= ~(1 << ind);
			}
			else
			{
				edit_mob->act |= (1 << ind);
			}
		}

		else if ((ind = index_lookup (affected_bits, subcmd)) != -1)
		{
			if(GET_TRUST(ch) > 3)
			{
			if (IS_SET (edit_mob->affected_by, 1 << ind))
				{
					send_to_char ("Affect has been removed.\n", ch);
				edit_mob->affected_by &= ~(1 << ind);
				}
			else
				{
					send_to_char ("Affect has been added.\n", ch);
				edit_mob->affected_by |= (1 << ind);
		}
			}
			else
			{
				send_to_char ("You need to be level 4 or higher to set Affects.\n", ch);
				return;

			}

		}

		else if ((ind = index_lookup (speeds, subcmd)) != -1)
		{
			edit_mob->speed = ind;
			send_to_char ("Note:  Speed isn't saving on mobs yet.\n", ch);
		}

		else if (!str_cmp (subcmd, "dpos"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("Expected mob position after dpos.\n", ch);
				break;
			}

			if ((ind = index_lookup (position_types, buf)) == -1)
			{
				send_to_char ("Invalid position.\n", ch);
				break;
			}

			edit_mob->default_pos = ind;
		}

		else if (!str_cmp (subcmd, "name"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("Expected a name keywords after name.\n", ch);
				break;
			}

			if (!IS_NPC (edit_mob) && !isname (GET_NAME (edit_mob), buf))
			{
				send_to_char ("MAKE SURE THE PLAYERS REAL NAME IS INCLUDED!!!\n"
					"You don't realize how many problems you are trying to create!\n",
					ch);
				break;
			}

			edit_mob->name = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "sdesc"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("Short description expected after sdesc.\n", ch);
				break;
			}

			edit_mob->short_descr = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "ldesc"))
		{

			argument = one_argument (argument, buf);

			if (!*buf)
			{
				send_to_char ("Long description expected after sdesc.\n", ch);
				break;
			}

			edit_mob->long_descr = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "prog"))
		{

			argument = one_argument (argument, buf);

			if (!*buf || (ind = index_lookup (mobprog_triggers, buf)) == -1)
			{
				send_to_char ("That trigger is not known by the mud.\n", ch);
				break;
			}

			strcpy (trigger_name, buf);
		}

		else if (!str_cmp (subcmd, "archive"))
		{

			if (GET_TRUST (ch) < 5)
			{
				send_to_char ("Only level 5 can archive pfiles.\n", ch);
				return;
			}

			if (IS_NPC (edit_mob))
			{
				send_to_char ("You can't archive a mobile.", ch);
				return;
			}

			if (edit_mob->pc->load_count != 1)
			{
				send_to_char
					("Pfile is being accessed by more than just you.\n", ch);
				send_to_char
					("Player is online or someone else has him/her mob'ed.\n",
					ch);
				return;
			}

			argument = one_argument (argument, buf);

			if (str_cmp (buf, GET_NAME (edit_mob)))
			{
				send_to_char ("You are trying to archive a pfile.\n", ch);
				send_to_char
					("If you were trying to archive the pfile for 'Marvin', you would type:\n\n   mset archive marvin\n",
					ch);
				return;
			}

			unload_pc (edit_mob);
			edit_mob = NULL;
			ch->pc->edit_player = NULL;

			*buf = toupper (*buf);

			sprintf (subcmd, "save/player/archive/%s", buf);
			sprintf (buf2, "save/player/%c/%s", tolower (*buf), buf);

			if ((error = rename (buf2, subcmd)))
			{
				perror ("archive");
				sprintf (buf, "Failed, system error %d.\n", error);
				send_to_char (buf, ch);
				return;
			}

			send_to_char ("Pfile archived.\n", ch);

			return;
		}

		else
		{
			sprintf (buf, "Unknown keyword: %s\n", subcmd);
			send_to_char (buf, ch);
			break;
		}

		argument = one_argument (argument, subcmd);
	}

	if (!IS_NPC (edit_mob))
		send_to_char ("Player modified.\n", ch);

	else if ((i = redefine_mobiles (edit_mob)) > 0)
	{
		sprintf (buf, "%d mobile(s) in world redefined.\n", i);
		send_to_char (buf, ch);
	}

	if (*trigger_name && full_description)
	{
		send_to_char ("You can't create a description and a mob program at the "
			"same time.\nIgnoring mob program.\n", ch);
		*trigger_name = '\0';
	}

	if (*trigger_name)
		add_replace_mobprog (ch, edit_mob, trigger_name);

	if (full_description)
	{
		if (!IS_NPC (edit_mob))
			send_to_char ("\nEnter a full description.  End with @.\n", ch);
		send_to_char
			("1-------10--------20--------30--------40--------50--------60---65\n",
			ch);
		free_mem(ch->desc->descStr);
		free_mem(ch->desc->pending_message);
		ch->desc->pending_message = new MESSAGE_DATA;
		ch->desc->descStr = ch->desc->pending_message->message;
		ch->desc->proc = post_mdesc;
		if (IS_NPC (edit_mob))
			ch->delay_info1 = edit_mob->mob->nVirtual;
		else
			ch->delay_who = duplicateString (edit_mob->tname);
		if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
			ve_setup_screen (ch->desc);
		else
		{
			send_to_char ("\nOld description:\n\n", ch);
			send_to_char (edit_mob->description, ch);
			ch->desc->max_str = STR_MULTI_LINE;
			make_quiet (ch);
		}

		return;
	}
}

void
post_mset (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *edit_mob;

	d->proc = NULL;

	if (d->character->pc->edit_player)
		edit_mob = d->character->pc->edit_player;
	else
		edit_mob = vtom (d->character->pc->edit_mob);

	if (!edit_mob)
		send_to_char
		("DANGER:  Could not find PC or MOBILE!  Please reboot mud.\n",
		d->character);
	else if (IS_NPC (edit_mob))
		redefine_mobiles (edit_mob);
}

void
consider_wearing (CHAR_DATA * ch, OBJ_DATA * obj)
{
	char buf[MAX_STRING_LENGTH];

	obj_from_room (&obj, 0);

	one_argument (obj->name, buf);

	equip_char (ch, obj, obj->obj_flags.wear_flags);
}

void
do_outfit (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char name[MAX_STRING_LENGTH];
	CHAR_DATA *mob;
	OBJ_DATA *next_obj;
	OBJ_DATA *tobj;

	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& GET_TRUST (ch) < 2)
	{
		send_to_char ("You do not have approval for leadership commands", ch);
		return;
	}
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Outfit whom?\n", ch);
		return;
	}

	if (!(mob = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Mobile not available for outfitting.\n", ch);
		return;
	}

	if (mob == ch)
		send_to_char ("Outfit yourself?  Ok.\n", ch);

	for (tobj = mob->room->contents; tobj; tobj = next_obj)
	{

		next_obj = tobj->next_content;

		one_argument (tobj->name, name);

		sprintf (buf, "get %s", name);
		command_interpreter (mob, buf);

		if (GET_ITEM_TYPE (tobj) == ITEM_WEAPON)
			sprintf (buf, "wield %s", name);
		else
			sprintf (buf, "wear %s", name);

		command_interpreter (mob, buf);
	}
}

void
purge_zone (int zone, int *obj_count, int *mob_count, int *stray_count)
{
	CHAR_DATA *ch;
	CHAR_DATA *next_ch;
	OBJ_DATA *obj;
	OBJ_DATA *next_obj;
	ROOM_DATA *room;
	int extracted = 0;
	int mob_no_resets = 0;
	int char_count = 0;
	int mob_resets_enabled = 0;
	int mob_resets_in_zone = 0;
	int i;

	*obj_count = 0;
	*mob_count = 0;
	*stray_count = 0;

	//for (ch = character_list; ch; ch = next_ch)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		//next_ch = ch->next;

		if (ch->deleted || !IS_NPC (ch))
			continue;

		char_count++;

		if (ch->mob->reset_zone == 0 && ch->mob->reset_cmd == 0)
		{
			extract_char (ch);
			mob_no_resets++;
			continue;
		}

		if (ch->mob->reset_zone != zone)
			continue;

		if (zone_table[zone].cmd[ch->mob->reset_cmd].arg3 / 1000 !=
			ch->room->zone)
			(*stray_count)++;

		i = ch->mob->reset_cmd;

		(*mob_count)++;

		extract_char (ch);

		extracted++;
	}

	for (i = 0; zone_table[zone].cmd[i].command != 'S'; i++)
	{
		if (zone_table[zone].cmd[i].command == 'M')
		{
			mob_resets_in_zone++;
			if (zone_table[zone].cmd[i].enabled)
				mob_resets_enabled++;
		}
	}

	printf ("Extracted %d, counted %d\n", extracted, char_count);
	printf ("Mobs in zone without resets: %d\n", mob_no_resets);
	printf ("Mob resets in zone: %d\n", mob_resets_in_zone);
	printf ("Mob resets enabled: %d\n", mob_resets_enabled);
	fflush (stdout);

	for (room = full_room_list; room; room = room->lnext)
	{

		if (room->zone != zone || IS_SET (room->room_flags, SAVE))
			continue;

		for (obj = room->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			extract_obj (obj);
			(*obj_count)++;
		}
	}

	cleanup_the_dead (0);
}

void
do_refresh (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int obj_count;
	int mob_count;
	int stray_count;
	int obj_total = 0;
	int mob_total = 0;
	int stray_total = 0;
	int not_frozen = 0;
	int zone;
	int i;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("You may refresh all zones with:\n\n   refresh all\n\n"
			"or, you may refresh a single zone with:\n\n"
			"   refresh <zone#>\n", ch);
		return;
	}

	if (!str_cmp (buf, "all"))
	{
		for (i = 0; i < MAX_ZONE; i++)
		{

			if (!zone_table[i].cmd)
				continue;

			if (!IS_SET (zone_table[i].flags, Z_FROZEN))
				not_frozen = 1;

			purge_zone (i, &obj_count, &mob_count, &stray_count);
			reset_zone (i);

			obj_total += obj_count;
			mob_total += mob_count;
			stray_total += stray_count;
		}

		if (not_frozen)
			send_to_char ("NOTE:  At least one refresh zone was not frozen.\n",
			ch);

		sprintf (buf, "\nSummary:\n"
			"   Mobiles destroyed:  %d\n"
			"     of those, strays: %d\n"
			"   Objects in rooms:   %d (mobile possessions not "
			"included)\n", mob_total, stray_total, obj_total);
		send_to_char (buf, ch);

		return;
	}

	if (!just_a_number (buf) || atoi (buf) < 0 || atoi (buf) >= MAX_ZONE)
	{
		send_to_char ("Expected a zone 0..99 or all as parameter to refresh.\n",
			ch);
		return;
	}

	zone = atoi (buf);

	if (!zone_table[zone].cmd)
	{
		send_to_char ("There were no resets loaded for that zone.\n", ch);
		return;
	}

	purge_zone (zone, &obj_total, &mob_total, &stray_total);
	reset_zone (zone);

	if (!IS_SET (zone_table[zone].flags, Z_FROZEN))
		send_to_char ("NOTE:  That zone is not frozen.\n", ch);

	sprintf (buf, "\n\nSummary:\n"
		"   Mobiles destroyed:  %d\n"
		"     of those, strays: %d\n"
		"   Objects in rooms:   %d (mobile possessions not "
		"included)\n", mob_total, stray_total, obj_total);
	send_to_char (buf, ch);
}

void
replace_object (int src, int tar, CHAR_DATA * ch)
{
	OBJ_DATA *src_obj;
	OBJ_DATA *tar_obj;
	OBJ_DATA *obj;
	int zone;
	int cmd_no;
	int replace_count = 0;
	int affected_zone[110];	/* Wanna use MAX_ZONE, but there
							a bug on the photobooks computer,
							forcing me to use 100 */
	char cmd;
	char buf[MAX_STRING_LENGTH];

	if (!(src_obj = vtoo (src)))
	{
		sprintf (buf, "Object #%d does not exist.\n", src);
		send_to_char (buf, ch);
		return;
	}

	if (!(tar_obj = vtoo (tar)))
	{
		sprintf (buf, "Object #%d does not exist.\n", tar);
		send_to_char (buf, ch);
		return;
	}

	for (zone = 0; zone < OBJECT_MAX_ZONE; zone++)
	{

		affected_zone[zone] = 0;

		for (cmd_no = 0;; cmd_no++)
		{

			cmd = zone_table[zone].cmd[cmd_no].command;

			if (cmd == 'S')
				break;

			if ((cmd == 'M' || cmd == 'O' || cmd == 'P' ||
				cmd == 'G' || cmd == 'E') &&
				zone_table[zone].cmd[cmd_no].arg1 == src)
			{
				zone_table[zone].cmd[cmd_no].arg1 = tar;
				replace_count++;
				affected_zone[zone] = 1;
			}
		}
	}

	sprintf (buf, "%d object resets replaced.\n", replace_count);
	send_to_char (buf, ch);

	replace_count = 0;

	for (obj = object_list; obj; obj = obj->next)
	{
		if (obj->nVirtual == src)
		{
			obj->nVirtual = tar;
			replace_count++;
		}
	}

	sprintf (buf, "%d objects in world replaced.\n", replace_count);
	send_to_char (buf, ch);

	(void) redefine_objects (tar_obj);

	send_to_char ("Based on resets, the following zones were affected:\n"
		"(WARNING:  There could be more:)\n", ch);
	*buf = '\0';

	for (zone = 0; zone < OBJECT_MAX_ZONE; zone++)
	{
		if (affected_zone[zone])
			sprintf (buf + strlen (buf), "   %2d", zone);
	}

	if (*buf == '\0')
		send_to_char ("   None.\n", ch);
	else
	{
		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}
}

void
replace_mobile (int src, int tar, CHAR_DATA * ch)
{
	CHAR_DATA *src_mob;
	CHAR_DATA *tar_mob;
	CHAR_DATA *mob;
	int zone;
	int cmd_no;
	int replace_count = 0;
	int affected_zone[100];	/* Wanna use MAX_ZONE, but there
							a bug on the photobooks computer,
							forcing me to use 100 */
	char cmd;
	char buf[MAX_STRING_LENGTH];

	if (!(src_mob = vtom (src)))
	{
		sprintf (buf, "Mobile #%d does not exist.\n", src);
		send_to_char (buf, ch);
		return;
	}

	if (!(tar_mob = vtom (tar)))
	{
		sprintf (buf, "Mobile #%d does not exist.\n", tar);
		send_to_char (buf, ch);
		return;
	}

	for (zone = 0; zone < MAX_ZONE; zone++)
	{

		affected_zone[zone] = 0;

		for (cmd_no = 0;; cmd_no++)
		{

			cmd = zone_table[zone].cmd[cmd_no].command;

			if (cmd == 'S')
				break;

			if (cmd == 'M' && zone_table[zone].cmd[cmd_no].arg1 == src)
			{
				zone_table[zone].cmd[cmd_no].arg1 = tar;
				replace_count++;
				affected_zone[zone] = 1;
			}
		}
	}

	sprintf (buf, "%d mobile resets replaced.\n", replace_count);
	send_to_char (buf, ch);

	replace_count = 0;

	//for (mob = character_list; mob; mob = mob->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;
		if (mob->deleted)
			continue;
		if (IS_NPC (mob) && mob->mob->nVirtual == src)
		{
			mob->mob->nVirtual = tar;
			replace_count++;
		}
	}

	sprintf (buf, "%d mobiles in world replaced.\n", replace_count);
	send_to_char (buf, ch);

	(void) redefine_mobiles (tar_mob);

	send_to_char ("Based on resets, the following zones were affected:\n"
		"(WARNING:  There could be more:)\n", ch);
	*buf = '\0';

	for (zone = 0; zone < MAX_ZONE; zone++)
	{
		if (affected_zone[zone])
			sprintf (buf + strlen (buf), "   %2d", zone);
	}

	if (*buf == '\0')
		send_to_char ("   None.\n", ch);
	else
	{
		sprintf (buf + strlen (buf), "\n");
		send_to_char (buf, ch);
	}
}

void
do_replace (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int src;
	int tar;
	int replace_obj = 0;
	char *syntax = "Syntax:\n   REPLACE OBJ <vnum> WITH <vnum>\n";

	/* NOTE:  The replace procedure is imperfect.  Reset "counts" are
	not updated when replaced.
	*/

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "object") || !str_cmp (buf, "obj"))
		replace_obj = 1;
	else if (!str_cmp (buf, "char") || !str_cmp (buf, "mob") ||
		!str_cmp (buf, "mobile"));
	else
	{
		send_to_char (syntax, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		send_to_char (syntax, ch);
		return;
	}

	src = atoi (buf);

	argument = one_argument (argument, buf);

	if (str_cmp (buf, "with"))
	{
		send_to_char (syntax, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		send_to_char (syntax, ch);
		return;
	}

	tar = atoi (buf);

	if (replace_obj)
		replace_object (src, tar, ch);
	else
		replace_mobile (src, tar, ch);
}

#define TOK_WORD		0
#define	TOK_NEWLINE		1
#define TOK_PARAGRAPH		2
#define TOK_END			3
#define TOK_SENTENCE		4

void
advance_spaces (char **s)
{
	while (**s && (**s == ' ' || **s == '\t' || **s == '\r'))
		(*s)++;
}

int
get_token (char **s, char *token)
{
	static int start_sentence = 0;

	if (start_sentence)
	{
		start_sentence = 0;
		return TOK_SENTENCE;
	}

	*token = '\0';

	if (**s == '\n')
		(*s)++;

	if (!**s)
		return TOK_END;

	if (**s == '\n')
		return TOK_NEWLINE;

	if (**s == ' ')
	{
		advance_spaces (s);
		return TOK_PARAGRAPH;
	}

	while (**s && **s != ' ' && **s != '\t' && **s != '\n')
	{
		if (**s == '\\' && (*s)[1] == '\'')
			(*s)++;
		*token = **s;
		start_sentence = (**s == '.');
		token++;
		(*s)++;
	}

	*token = '\0';

	advance_spaces (s);

	return TOK_WORD;
}

void
reformat_desc (char *source, char **target)
{
	int token_value = 0;
	int first_line = 1;
	int line_len;
	char *s;
	char *r;
	char token[MAX_STRING_LENGTH];
	char result[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	if (source == NULL)
		return;

	if (strn_cmp (source, "   ", 3))
	{
		sprintf (buf, "   %s", source);
		replaceString(source, buf);
	}

	s = source;
	r = result;
	*result = '\0';

	line_len = 0;

	while (token_value != TOK_END)
	{

		token_value = get_token (&s, token);

		if (token_value == TOK_PARAGRAPH)
		{

			if (first_line)
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if (token_value == TOK_SENTENCE)
		{
			line_len += 1;
			continue;
		}

		if (token_value == TOK_NEWLINE)
		{
			if (line_len != 0)
				strcat (result, "\n");	/* Catch up */
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if (token_value == TOK_WORD)
		{
			if (line_len + strlen (token) > 65)
			{
				strcat (result, "\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");

			line_len += strlen (token) + 1;
		}
	}

	if (result[strlen (result) - 1] != '\n')
		strcat (result, "\n");

	*target = duplicateString (result);
}

// Case '08, for do_look. Extends length of the lines from 80 to 130.
void reformat_string (char *source, char **target)
{
	int token_value = 0;
	int first_line = 1;
	int line_len;
	char *s;
	char *r;
	char token[MAX_STRING_LENGTH];
	char result[MAX_STRING_LENGTH];

	s = source;
	r = result;
	*result = '\0';

	line_len = 0;

	while (token_value != TOK_END)
	{

		token_value = get_token (&s, token);

		if (token_value == TOK_PARAGRAPH)
		{

			if (first_line)
				first_line = 0;
			else
				strcat (result, "\r\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if (token_value == TOK_SENTENCE)
		{
			line_len += 1;
			continue;
		}

		if (token_value == TOK_NEWLINE)
		{
			if (line_len != 0)
				strcat (result, "\r\n");
			strcat (result, "\r\n");
			line_len = 0;
			continue;
		}

		if (token_value == TOK_WORD)
		{
			if ((line_len + strlen (token) > 79)
				&& !(*token == '#' && strlen (token) == 2))
			{
				strcat (result, "\r\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	if (result[strlen (result) - 1] != '\n')
		strcat (result, "\r\n");

	*target = duplicateString (result);
}

int
free_exit (struct room_direction_data *exit)
{
	if (!exit)
		return -1;

	if (exit->general_description && *exit->general_description)
		free_mem (exit->general_description);

	if (exit->keyword && *exit->keyword)
		free_mem (exit->keyword);

	free_mem (exit);

	return 1;
}

int
free_room (ROOM_DATA * room)
{
	int dir;
	ROOM_DATA *troom;

	while (room->affects)
		remove_room_affect (room, room->affects->type);

	while (room->people)
		extract_char (room->people);

	while (room->contents)
		extract_obj (room->contents);

	for (troom = full_room_list; troom; troom = troom->lnext)
	{
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if (troom->dir_option[dir]
			&& troom->dir_option[dir]->to_room == room->nVirtual)
			{
				free_exit (troom->dir_option[dir]);
				troom->dir_option[dir] = NULL;
			}
		}
	}

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (room->dir_option[dir])
		{
			free_exit (room->dir_option[dir]);
			room->dir_option[dir] = NULL;
		}
	}

	if (room->name && *room->name)
	{
		free_mem (room->name);
		room->name = NULL;
	}

	if (room->description && *room->description)
	{
		free_mem (room->description);
		room->description = NULL;
	}

	return 1;
}

int
rdelete (ROOM_DATA * room)
{
	char buf[100];
	int success = 0;
	CHAR_DATA *tch;
	ROOM_DATA *troom;

	if (!room)
		return -1;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			return -1;
	}

	if (room == full_room_list)
		full_room_list = full_room_list->lnext;
	else
		for (troom = full_room_list; troom && troom->lnext; troom = troom->lnext)
		{
			if (troom->lnext == room)
				troom->lnext = troom->lnext->lnext;
		}

		success = free_room (room);

		sprintf (buf, "save/rooms/%d", room->nVirtual);
		unlink (buf);

		room->nVirtual = -room->nVirtual;
		room->zone = -1;

		if (success == -1)
			return 0;

		return 1;
}

void
do_rdelete (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	int room_num;
	static int room_del_warn = 0;

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		send_to_char ("Expected a room virtual number.\n", ch);
		return;
	}

	room_num = atoi (buf);

	if (room_num == 0 || !vtor (room_num))
	{
		send_to_char ("No such room.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!rdelete (vtor (room_num)))
	{
		send_to_char ("ROOM NOT DELETED!\n", ch);
		return;
	}
	else
	{
		sprintf (buf, "Room %d deleted.\n", room_num);
		send_to_char (buf, ch);

		if (!room_del_warn)
		{
			send_to_char
				("WARNING:  It would be unwise to thaw the mud at this point.\n",
				ch);
			send_to_char ("          Save and reboot before a thawing.\n", ch);
			room_del_warn = 1;
		}
	}
}

void
do_rmove (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *room;
	ROOM_DATA *troom;
	CHAR_DATA *tch;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH];
	int dir;
	int target_room_num;

	//extern ROOM_DATA *wld_tab[];

	argument = one_argument (argument, buf);

	if (!isdigit (*buf) || !atoi (buf) || !(room = vtor (atoi (buf))))
	{
		send_to_char ("Invalid source room.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!isdigit (*buf) || !atoi (buf))
	{
		send_to_char ("Invalid target room.\n", ch);
		return;
	}

	target_room_num = atoi (buf);

	if (vtor (target_room_num))
	{
		send_to_char ("Target room already exists.\n", ch);
		return;
	}
	// Commented out by Case, trying to use vtor for all lookups
	//if (vtor(room->nVirtual) == room) { // If wld_tab nVirtual == room then
	//	//wld_tab[room->nVirtual % ZONE_SIZE] = room->hnext;
	//}
	//else
	//{
	//	printf ("%d %d\n", (long int) wld_tab[room->nVirtual % ZONE_SIZE],
	//		room->nVirtual % ZONE_SIZE);
	//	for (troom = wld_tab[room->nVirtual % ZONE_SIZE]; troom;
	//		troom = troom->hnext)
	//	{

	//		printf ("%d v %d\n", troom->hnext->nVirtual, room->nVirtual);

	//		fflush (stdout);

	//		if (troom->hnext->nVirtual == room->nVirtual)
	//			break;
	//	}

	//	troom->hnext = room->hnext;

	//	room->hnext = wld_tab[target_room_num % ZONE_SIZE];
	//	wld_tab[target_room_num % ZONE_SIZE] = room;
	//}

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		tch->in_room = target_room_num;

		if (IS_NPC (tch))
			tch->mob->zone = target_room_num / ZONE_SIZE;
	}

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		tobj->in_room = target_room_num;
		tobj->zone = target_room_num / OBJECT_ZONE_SIZE;
	}

	/* Relink rooms */

	for (troom = full_room_list; troom; troom = troom->lnext)
	{
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if (troom->dir_option[dir] &&
				troom->dir_option[dir]->to_room == room->nVirtual)
				troom->dir_option[dir]->to_room = target_room_num;
		}
	}

	/* I'm not bothering with resets, so that may cause some problems */

	room->nVirtual = target_room_num;
	room->zone = target_room_num / ZONE_SIZE;

	send_to_char ("Done.\n", ch);
}

void
do_zset (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char subcmd[MAX_STRING_LENGTH];
	int zone;

	argument = one_argument (argument, buf);

	if (!isdigit (*buf) || atoi (buf) > 99)
	{
		send_to_char ("Expected a zone number 0..99\n", ch);
		return;
	}

	zone = atoi (buf);

	argument = one_argument (argument, subcmd);

	if (!*subcmd)
	{

		sprintf (buf, "Zone:    [%2d]    %s\n", zone, zone_table[zone].name ?
			zone_table[zone].name : "Unnamed zone");
		send_to_char (buf, ch);

		sprintf (buf, "Project Lead: %s\n", zone_table[zone].lead);
		send_to_char (buf, ch);

		sprintf (buf, "Jailer:  [%5d] %s\n", zone_table[zone].jailer,
			vtom (zone_table[zone].jailer) ?
			vtom (zone_table[zone].jailer)->short_descr :
		"Undefined vnum");
		send_to_char (buf, ch);

		sprintf (buf, "Jail:    [%5d] %s\n", zone_table[zone].jail_room_num,
			zone_table[zone].jail_room ?
			zone_table[zone].jail_room->name : "Undefined room");
		send_to_char (buf, ch);

		return;
	}

	while (*subcmd)
	{

		argument = one_argument (argument, buf);

		if (!str_cmp (subcmd, "earth"))
		{
			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected a value between -1000 and 1000.\n", ch);
				return;
			}
			zone_table[zone].earth_mod = atoi (buf);
			send_to_char ("Zone-wide earth modifier applied.\n", ch);
			return;
		}

		if (!str_cmp (subcmd, "wind"))
		{
			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected a value between -1000 and 1000.\n", ch);
				return;
			}
			zone_table[zone].wind_mod = atoi (buf);
			send_to_char ("Zone-wide psychic modifier applied.\n", ch);
			return;
		}

		if (!str_cmp (subcmd, "fire"))
		{
			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected a value between -1000 and 1000.\n", ch);
				return;
			}
			zone_table[zone].fire_mod = atoi (buf);
			send_to_char ("Zone-wide fire modifier applied.\n", ch);
			return;
		}

		if (!str_cmp (subcmd, "water"))
		{
			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected a value between -1000 and 1000.\n", ch);
				return;
			}
			zone_table[zone].water_mod = atoi (buf);
			send_to_char ("Zone-wide water modifier applied.\n", ch);
			return;
		}

		if (!str_cmp (subcmd, "shadow"))
		{
			if (!just_a_number (buf) && *buf != '-')
			{
				send_to_char ("Expected a value between -1000 and 1000.\n", ch);
				return;
			}
			zone_table[zone].shadow_mod = atoi (buf);
			send_to_char ("Zone-wide shadow modifier applied.\n", ch);
			return;
		}

		if (!str_cmp (subcmd, "jailer"))
		{

			if (!isdigit (*buf) || !vtom (atoi (buf)))
			{
				send_to_char ("That mobile could not be found.\n", ch);
				return;
			}

			zone_table[zone].jailer = atoi (buf);
		}

		else if (!str_cmp (subcmd, "jail"))
		{

			if (!isdigit (*buf) || !vtor (atoi (buf)))
			{
				send_to_char ("That room could not be found.\n", ch);
				return;
			}

			zone_table[zone].jail_room_num = atoi (buf);
			zone_table[zone].jail_room = vtor (atoi (buf));
		}

		else if (!str_cmp (subcmd, "name"))
		{

			if (!*buf)
			{
				send_to_char ("Pick a name for your zone.\n", ch);
				return;
			}

			zone_table[zone].name = duplicateString (buf);
		}

		else if (!str_cmp (subcmd, "lead"))
		{
			if (GET_TRUST (ch) < 5)
			{
				send_to_char ("You cannot assign project leads.\n", ch);
				return;
			}
			else
			{
				zone_table[zone].lead = duplicateString (CAP (buf));
				send_to_char ("Project lead changed.\n", ch);
				return;
			}
		}
		argument = one_argument (argument, subcmd);
	}
}

CHAR_DATA *
mcopy (CHAR_DATA * mob)
{
	int container_container = 0;
	CHAR_DATA *new_mob;
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	OBJ_DATA *new_obj;
	OBJ_DATA *new_obj2;

	new_mob = load_mobile (mob->mob->nVirtual);

	new_mob->act = mob->act;

	for (obj = mob->equip; obj; obj = obj->next_content)
	{

		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;

		if (new_obj->nVirtual == VNUM_PENNY ||
			new_obj->nVirtual == VNUM_FARTHING)
			name_money (new_obj);

		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{

			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;

			if (new_obj2->nVirtual == VNUM_PENNY ||
				new_obj2->nVirtual == VNUM_FARTHING)
				name_money (new_obj2);

			obj_to_obj (new_obj2, new_obj);

			if (obj2->contains)
				container_container = 1;
		}

		equip_char (new_mob, new_obj, obj->location);
	}

	if (mob->right_hand)
	{
		obj = mob->right_hand;
		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;
		if (new_obj->nVirtual == VNUM_PENNY ||
			new_obj->nVirtual == VNUM_FARTHING)
			name_money (new_obj);
		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{
			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;
			if (new_obj2->nVirtual == VNUM_PENNY ||
				new_obj2->nVirtual == VNUM_FARTHING)
				name_money (new_obj2);
			obj_to_obj (new_obj2, new_obj);
			if (obj2->contains)
				container_container = 1;
		}
		obj_to_char (new_obj, new_mob);
	}

	if (mob->left_hand)
	{
		obj = mob->left_hand;
		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;
		if (new_obj->nVirtual == VNUM_PENNY ||
			new_obj->nVirtual == VNUM_FARTHING)
			name_money (new_obj);
		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{
			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;
			if (new_obj2->nVirtual == VNUM_PENNY ||
				new_obj2->nVirtual == VNUM_FARTHING)
				name_money (new_obj2);
			obj_to_obj (new_obj2, new_obj);
			if (obj2->contains)
				container_container = 1;
		}
		obj_to_char (new_obj, new_mob);
	}

	return new_mob;
}

void
do_mcopy (CHAR_DATA * ch, char *argument, int cmd)
{
	int room_num;
	CHAR_DATA *mob;
	CHAR_DATA *new_mob;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("mcopy [rvnum] [mob #.]mobname\n", ch);
		return;
	}

	room_num = ch->in_room;

	if (isdigit (buf[strlen (buf) - 1]))
	{

		room_num = atoi (buf);

		if (room_num <= 0 || room_num > 99999)
		{
			send_to_char ("Illegal room number.\n", ch);
			return;
		}

		if (!vtor (room_num))
		{
			send_to_char ("No such room.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
	}

	if (!(mob = get_char_room (buf, room_num)))
	{
		send_to_char ("No such mobile.\n", ch);
		return;
	}

	if (!IS_NPC (mob))
	{
		send_to_char ("You specified a PC.\n", ch);
		return;
	}

	new_mob = mcopy (mob);

	char_to_room (new_mob, ch->in_room);

	act ("$N copied and outfitted.", false, ch, 0, new_mob, TO_CHAR);
}

void
post_alas (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;

	ch = d->character;
	
	ch->room->extra->alas[ch->delay_info1] = duplicateString(d->descStr);
	
	send_to_char("Alas description set.\n", ch);

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
	ch->desc->proc = NULL;
}

void
post_weather_desc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;

	ch = d->character;
	
	ch->room->extra->weather_desc[ch->delay_info1] = duplicateString(d->descStr);
	
	send_to_char("Description set.\n", ch);

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
	ch->desc->proc = NULL;
}

void
do_rset (CHAR_DATA * ch, char *argument, int cmd)
{
	int ind;
	char buf[MAX_STRING_LENGTH];
	char str[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("Type tags weather-room for possible descriptions.\n",
			ch);
		send_to_char ("\n\nSyntax:\n     rset <weather-room> [reformat]\n", ch);
		send_to_char ("     rset alas <direction>\n", ch);
		return;
	}

	if (!str_cmp (buf, "alas"))
	{

		argument = one_argument (argument, buf);

		if ((ind = index_lookup (dirs, buf)) == -1)
		{
			send_to_char ("Expected north, south, east, west, up, or down.\n",
				ch);
			return;
		}

		if (!ch->room->extra)
			ch->room->extra = new ROOM_EXTRA_DATA;

		if (ch->room->extra->alas[ind] &&
			!IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
		{
			send_to_char ("The alas description was:\n", ch);
			send_to_char (ch->room->extra->alas[ind], ch);
		}

		act ("$n begins editing an alas description.", false, ch, 0, 0,
			TO_ROOM);

		free_mem(ch->desc->descStr);
		ch->desc->descStr = duplicateString(ch->room->extra->alas[ind]);

		if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
		{
			ve_setup_screen (ch->desc);
		}
		else
		{
			make_quiet (ch);
			send_to_char ("\nEnter a new description.  Terminate with an '@'\n",
				ch);
			ch->room->extra->alas[ind] = 0;
			ch->desc->max_str = 2000;
			ch->delay_info1 = ind;
			ch->desc->proc = post_alas;
		}

		return;
	}

	if ((ind = index_lookup (weather_room, buf)) == -1)
	{
		send_to_char ("No such weather-room description.\n", ch);
		return;
	}

	if (ind == WR_NORMAL)
	{
		do_rdesc (ch, argument, 0);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "reformat"))
	{

		if (!ch->room->extra || !ch->room->extra->weather_desc[ind])
		{
			sprintf (buf, "No current %s description.\n", weather_room[ind]);
			send_to_char (buf, ch);
			return;
		}

		reformat_desc (ch->room->extra->weather_desc[ind],
			&ch->room->extra->weather_desc[ind]);
		send_to_char (ch->room->extra->weather_desc[ind], ch);

		return;
	}

	if (!ch->room->extra)
	{
		ch->room->extra = new ROOM_EXTRA_DATA;
		printf ("Creating extra room data.\n");
		fflush (stdout);
	}

	if (ch->room->extra->weather_desc[ind] &&
		!IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
	{
		send_to_char ("The old description was: \n", ch);
		send_to_char (ch->room->extra->weather_desc[ind], ch);
	}

	act ("$n begins editing a room description.", false, ch, 0, 0, TO_ROOM);

	free_mem(ch->desc->descStr);
	ch->desc->descStr = duplicateString(ch->room->extra->weather_desc[ind]);

	if (IS_SET (ch->desc->edit_mode, MODE_VISEDIT))
	{
		ve_setup_screen (ch->desc);
	}
	else
	{
		send_to_char ("\nEnter a new description.  Terminate with an '@'\n",
			ch);
		send_to_char
			("1-------10--------20--------30--------40--------50--------60---65\n",
			ch);
		make_quiet (ch);
		ch->room->extra->weather_desc[ind] = 0;
		ch->desc->max_str = 2000;
		ch->delay_info1 = ind;
		ch->desc->proc = post_weather_desc;
	}
}

void
do_job (CHAR_DATA * ch, char *argument, int cmd)
{
	int job;
	int days = 0;
	int pay_date;
	int count = 0;
	int ovnum = 0;
	int employer = 0;
	int cash = 0;
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		s ("Start by using the mob command on a PC.");
		s ("");
		s ("job [1, 2, or 3]");
		s ("    pay <amount>              pay PC in cash by amount");
		s ("    days <pay-period-days>    # days between payday");
		s ("    employer <mob-vnum>       A mob pays PC instead of autopay");
		s ("    objects <count> <ovnum>   Pay PC a number of objects for pay");
		s ("    delete                    Delete the job");
		s ("");
		return;
	}

	if (IS_NPC(ch))
	{
		send_to_char ("This command is for PC's only.\n", ch);
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		send_to_char ("Start by using the MOB command on a PC.\n", ch);
		return;
	}

	if (atoi (buf) < 1 || atoi (buf) > 3)
	{
		send_to_char ("Specify job 1, 2 or 3.\n", ch);
		return;
	}

	job = atoi (buf) + JOB_1 - 1;

	while (*argument)
	{

		argument = one_argument (argument, buf);

		if (!str_cmp (buf, "delete"))
		{
			if ((af = get_affect (edit_mob, job)))
			{
				affect_remove (edit_mob, af);
				send_to_char ("Job removed.\n", ch);
				return;
			}
			else
				send_to_char ("That job wasn't assigned.\n", ch);
		}

		if (!str_cmp (buf, "pay") || !str_cmp (buf, "money") ||
			!str_cmp (buf, "cash"))
		{

			argument = one_argument (argument, buf);
			cash = atoi (buf);

			if (cash < 0)
				cash = 0;

			if (cash > 500)
				send_to_char ("That cash amount is a bit high.\n", ch);

			if (cash > 10000)
			{
				send_to_char ("Cash > 10000.  Disallowed.\n", ch);
				return;
			}
		}

		else if (!str_cmp (buf, "days"))
		{

			argument = one_argument (argument, buf);

			days = atoi (buf);

			if (days < 0)
				days = 0;
		}

		else if (!str_cmp (buf, "employer"))
		{

			argument = one_argument (argument, buf);

			employer = atoi (buf);

			if (!vtom (employer))
			{
				send_to_char ("Employer mob is not defined.\n", ch);
				return;
			}
		}

		else if (!str_cmp (buf, "objects"))
		{

			argument = one_argument (argument, buf);

			count = atoi (buf);

			if (count < 1 || count > 50)
			{
				send_to_char ("Count should be between 1 and 50.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			ovnum = atoi (buf);

			if (!vtoo (ovnum))
			{
				send_to_char ("Specified object vnum doesn't exist.\n", ch);
				return;
			}
		}
	}

	if (days <= 0)
	{
		send_to_char
			("Don't forget to specify the number of days until payday.\n", ch);
		return;
	}

	if (!count && !cash)
	{
		send_to_char ("Make the payment in cash or some number of objects.\n",
			ch);
		return;
	}

	pay_date = time_info.month * 30 + time_info.day +
		time_info.year * 12 * 30 + days;

	if ((af = get_affect (edit_mob, job)))
		affect_remove (edit_mob, af);

	job_add_affect (edit_mob, job, days, pay_date, cash, count, ovnum,
		employer);

	send_to_char ("Ok.\n", ch);
}

void
do_mclone (CHAR_DATA * ch, char *argument, int cmd)
{
	int oldVNum, newVNum, i;
	CHAR_DATA *newmob;
	CHAR_DATA *source;
	std::stringstream result;
	MOB_DATA *m;
	MOB_DATA *sm;
	char buf[8];
	char buf2[8];

	if (!ch->pc)
	{
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	argument = one_argument (argument, buf2);

	if (!*buf || !*buf2 || !atoi (buf) || !atoi (buf2))
	{
		send_to_char ("mclone <original vnum> <new vnum>   - copy a mobile prototype.\n", ch);
		return;
	}

	oldVNum = atoi (buf);
	newVNum = atoi (buf2);

	if (!vtom (oldVNum))
	{
		send_to_char ("No such prototype exists.", ch);
		return;
	}

	if (vtom (newVNum))
	{
		send_to_char ("A mob prototype already exists with your target vnum.\n", ch);
		return;
	}

	if (newVNum < 1 || newVNum > 99999)
	{
		send_to_char ("VNums must be between 1 and 99999.\n", ch);
		return;
	}

	newmob = new_char (0); /* Creates mob. */
	source = vtom (oldVNum);
	m = newmob->mob;
	sm = source->mob;

	m->nVirtual = newVNum;
	m->zone = newVNum / ZONE_SIZE;
	m->lnext = NULL;
	m->hnext = NULL;

	for (i = 0; i <= 100; i++)
	{
		newmob->enforcement[i] = source->enforcement[i];
	}
	newmob->act = source->act;

	newmob->name = duplicateString (source->name);
	newmob->tname = duplicateString (source->tname);
	newmob->short_descr = duplicateString (source->short_descr);
	newmob->long_descr = duplicateString (source->long_descr);
	newmob->description = duplicateString (source->description);
	newmob->clans = duplicateString (source->clans);
	newmob->travel_str = duplicateString (source->travel_str);

	newmob->str = source->str;
	newmob->intel = source->intel;
	newmob->wil = source->wil;
	newmob->dex = source->dex;
	newmob->con = source->con;
	newmob->aur = source->aur;
	newmob->agi = source->agi;
	newmob->tmp_str = source->tmp_str;
	newmob->tmp_intel = source->tmp_intel;
	newmob->tmp_wil = source->tmp_wil;
	newmob->tmp_dex = source->tmp_dex;
	newmob->tmp_con = source->tmp_con;
	newmob->tmp_aur = source->tmp_aur;
	newmob->tmp_agi = source->tmp_agi;

	newmob->balance = source->balance;
	newmob->bmi = source->bmi;
	newmob->body_proto = source->body_proto;
	newmob->body_type = source->body_type;
	newmob->height = source->height;
	newmob->frame = source->frame;
	newmob->size = source->size;

	newmob->sex = source->sex;
	newmob->age = source->age;
	newmob->race = source->race;
	newmob->hit = source->hit;
	newmob->max_hit = source->max_hit;
	newmob->move = source->move;
	newmob->max_move = source->max_move;
	newmob->armor = source->armor;
	newmob->carry_weight = source->carry_weight;
	newmob->carry_items = source->carry_items;
	newmob->lastregen = source->lastregen;

	newmob->fight_percentage = source->fight_percentage;
	newmob->primary_delay = source->primary_delay;
	newmob->secondary_delay = source->secondary_delay;
	newmob->fight_mode = source->fight_mode;
	newmob->nat_attack_type = source->nat_attack_type;
	newmob->natural_delay = source->natural_delay;
	newmob->speed = source->speed;
	newmob->defensive = source->defensive;

	if (source->spells)
	{
		for (i = 0; i <= MAX_LEARNED_SPELLS; i++)
		{
			newmob->spells[i][0] = source->spells[i][0];
			newmob->spells[i][1] = source->spells[i][1];
			newmob->spells[i][2] = source->spells[i][2];
			newmob->spells[i][3] = source->spells[i][3];
		}
	}

	newmob->mana = source->mana;
	newmob->max_mana = source->max_mana;
	newmob->harness = source->harness;
	newmob->max_harness = source->max_harness;
	newmob->curse = source->curse;

	newmob->hire_storeroom = source->hire_storeroom;
	newmob->hire_storeobj = source->hire_storeobj;

	newmob->speaks = source->speaks;
	newmob->psionic_talents = source->psionic_talents;
	for (i = 0; i <= MAX_SKILLS; i++)
	{
		newmob->skills[i] = source->skills[i];
	}
	newmob->offense = source->offense;

	newmob->morph_type = source->morph_type;
	newmob->morph_time = source->morph_time;
	newmob->morphto = source->morphto;

	newmob->coldload_id = source->coldload_id;
	newmob->flags = source->flags;
	newmob->deity = source->deity;
	newmob->color = source->color;
	newmob->writes = source->writes;

	m->owner = duplicateString (sm->owner);

	m->skinned_vnum = sm->skinned_vnum;
	m->carcass_vnum = sm->carcass_vnum;

	m->damnodice = sm->damnodice;
	m->damsizedice = sm->damsizedice;
	m->damroll = sm->damroll;
	m->min_height = sm->min_height;
	m->max_height = sm->max_height;
	m->size = sm->size;

	m->merch_seven = sm->merch_seven;
	m->helm_room = sm->helm_room;
	m->access_flags = sm->access_flags;
	m->noaccess_flags = sm->noaccess_flags;
	m->reset_zone = sm->reset_zone;
	m->reset_cmd = sm->reset_cmd;
	m->currency_type = sm->currency_type;
	m->jail = sm->jail;

	if (sm->resets)
	{
		m->resets->type = sm->resets->type;
		m->resets->command = duplicateString (sm->resets->command);
		m->resets->planned = sm->resets->planned;
	}

	if (source->moves)
	{
		newmob->moves->dir = source->moves->dir;
		newmob->moves->flags = source->moves->flags;
		newmob->moves->desired_time = source->moves->desired_time;
		newmob->moves->next = NULL;
		newmob->moves->travel_str = duplicateString (source->moves->travel_str);
	}

	if (source->shop)
	{
		newmob->shop = new SHOP_DATA;
		newmob->shop->negotiations = NULL;
		newmob->shop->markup = source->shop->markup;
		newmob->shop->discount = source->shop->discount;
		if (source->shop->shop_vnum)
			newmob->shop->shop_vnum = source->shop->shop_vnum;
		if (source->shop->store_vnum)
			newmob->shop->store_vnum = source->shop->store_vnum;

		if (source->shop->no_such_item1)
			newmob->shop->no_such_item1 = duplicateString(source->shop->no_such_item1);
		if (source->shop->no_such_item2)
			newmob->shop->no_such_item2 = duplicateString(source->shop->no_such_item2);
		if (source->shop->missing_cash1)
			newmob->shop->missing_cash1 = duplicateString(source->shop->missing_cash1);
		if (source->shop->missing_cash2)
			newmob->shop->missing_cash2 = duplicateString(source->shop->missing_cash2);
		if (source->shop->do_not_buy)
			newmob->shop->do_not_buy = duplicateString(source->shop->do_not_buy);
		if (source->shop->message_buy)
			newmob->shop->message_buy = duplicateString(source->shop->message_buy);
		if (source->shop->message_sell)
			newmob->shop->message_sell = duplicateString(source->shop->message_sell);


		if (source->shop->delivery)
		{
			for (i = 0; i <= MAX_DELIVERIES; i++)
			{
				newmob->shop->delivery[i] = source->shop->delivery[i];
			}
		}

		if (source->shop->trades_in)
		{
			for (i = 0; i <= MAX_TRADES_IN; i++)
			{
				newmob->shop->trades_in[i] = source->shop->trades_in[i];
			}
		}

		newmob->shop->econ_flags1 = source->shop->econ_flags1;
		newmob->shop->econ_flags2 = source->shop->econ_flags2;
		newmob->shop->econ_flags3 = source->shop->econ_flags3;
		newmob->shop->econ_flags4 = source->shop->econ_flags4;
		newmob->shop->econ_flags5 = source->shop->econ_flags5;
		newmob->shop->econ_flags6 = source->shop->econ_flags6;
		newmob->shop->econ_flags7 = source->shop->econ_flags7;
		newmob->shop->econ_markup1 = source->shop->econ_markup1;
		newmob->shop->econ_markup2 = source->shop->econ_markup2;
		newmob->shop->econ_markup3 = source->shop->econ_markup3;
		newmob->shop->econ_markup4 = source->shop->econ_markup4;
		newmob->shop->econ_markup5 = source->shop->econ_markup5;
		newmob->shop->econ_markup6 = source->shop->econ_markup6;
		newmob->shop->econ_markup7 = source->shop->econ_markup7;
		newmob->shop->econ_discount1 = source->shop->econ_discount1;
		newmob->shop->econ_discount2 = source->shop->econ_discount2;
		newmob->shop->econ_discount3 = source->shop->econ_discount3;
		newmob->shop->econ_discount4 = source->shop->econ_discount4;
		newmob->shop->econ_discount5 = source->shop->econ_discount5;
		newmob->shop->econ_discount6 = source->shop->econ_discount6;
		newmob->shop->econ_discount7 = source->shop->econ_discount7;
		newmob->shop->buy_flags = source->shop->buy_flags;
		newmob->shop->nobuy_flags = source->shop->nobuy_flags;
		if (source->shop->materials)
		{
			newmob->shop->materials = source->shop->materials;
		}

		if (newmob->shop->negotiations)
		{
			newmob->shop->negotiations->obj_vnum = source->shop->negotiations->obj_vnum;
			newmob->shop->negotiations->time_when_forgotten = source->shop->negotiations->time_when_forgotten;
			newmob->shop->negotiations->price_delta = source->shop->negotiations->price_delta;
			newmob->shop->negotiations->transactions = source->shop->negotiations->transactions;
			newmob->shop->negotiations->true_if_buying = source->shop->negotiations->true_if_buying;
			newmob->shop->negotiations->next = NULL;
		}

		newmob->shop->opening_hour = source->shop->opening_hour;
		newmob->shop->closing_hour = source->shop->closing_hour;
		newmob->shop->exit = source->shop->exit;
	}

	newmob->hunger = -1;
	newmob->thirst = -1;

	add_mob_to_hash (newmob);

	result << "Copied mobile #C" << sm->nVirtual << "#0 ('#2" << source->short_descr << "#0') to #C" << m->nVirtual << "#0." << std::endl;
	send_to_char (result.str().c_str(), ch);

}

void
do_oclone (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA * newObj = new_object ( );
	OBJ_DATA * source;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int oldVNum;
	int newVNum;
	std::stringstream result;

	if (!ch->pc)
	{
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	argument = one_argument (argument, buf2);

	if (!*buf || !*buf2 || !atoi (buf) || !atoi (buf2))
	{
		send_to_char ("oclone <original vnum> <new vnum>   - copy an object prototype.\n", ch);
		return;
	}

	oldVNum = atoi (buf);
	newVNum = atoi (buf2);

	if (!vtoo (oldVNum))
	{
		send_to_char ("No such prototype exists.\n", ch);
		return;
	}

	if (vtoo (newVNum))
	{
		send_to_char ("A prototype already exists with your target vnum.\n", ch);
		return;
	}

	if (newVNum < 0 || newVNum >= 110000)
	{
		send_to_char ("VNums must be between 1 and 110000.\n", ch);
		return;
	}

	clear_object (newObj);
	source = vtoo (oldVNum);

	newObj->nVirtual = newVNum;
	newObj->zone = newVNum / OBJECT_ZONE_SIZE;

	newObj->clock = source->clock;
	newObj->morphTime = source->morphTime;
	newObj->morphto = source->morphto;
	newObj->activation = source->activation;
	newObj->quality = source->quality;
	newObj->econ_flags = source->econ_flags;
	newObj->size = source->size;
	newObj->count = source->count;
	newObj->obj_timer = source->obj_timer;
	newObj->farthings = source->farthings;
	newObj->silver = source->silver;
	newObj->item_wear = source->item_wear;
	newObj->open = source->open;
	newObj->title_skill = source->title_skill;
	newObj->title_language = source->title_language;
	newObj->title_script = source->title_script;
	newObj->material = source->material;
	newObj->tmp_flags = source->tmp_flags;
	newObj->coldload_id = source->coldload_id;
	newObj->sold_at = source->sold_at;
	newObj->sold_by = source->sold_by;

	newObj->obj_flags = source->obj_flags;
	newObj->o.od.value[0] = source->o.od.value[0];
	newObj->o.od.value[1] = source->o.od.value[1];
	newObj->o.od.value[2] = source->o.od.value[2];
	newObj->o.od.value[3] = source->o.od.value[3];
	newObj->o.od.value[4] = source->o.od.value[4];
	newObj->o.od.value[5] = source->o.od.value[5];
	newObj->o.od.value[6] = source->o.od.value[6];

	newObj->instances = 0;
	newObj->next = NULL;
	newObj->lnext = NULL;
	newObj->hnext = NULL;
	newObj->contains = NULL;
	newObj->carried_by = NULL;
	newObj->equiped_by = NULL;

	newObj->name = duplicateString (source->name);
	newObj->description = duplicateString (source->description);
	newObj->short_description = duplicateString (source->short_description);
	newObj->full_description = duplicateString (source->full_description);
	newObj->omote_str = duplicateString (source->omote_str);
	newObj->ink_color = duplicateString (source->ink_color);
	newObj->desc_keys = duplicateString (source->desc_keys);
	newObj->var_color = duplicateString (source->var_color);
	newObj->book_title = duplicateString (source->book_title);
	newObj->indoor_desc = duplicateString (source->indoor_desc);

	if (source->ex_description)
	{
		newObj->ex_description->keyword = duplicateString (source->ex_description->keyword);
		newObj->ex_description->description = duplicateString (source->ex_description->description);
	}

	if (source->wdesc)
	{
		newObj->wdesc->language = source->wdesc->language;
		newObj->wdesc->description = duplicateString (source->wdesc->description);
	}

	if (source->wounds)
	{
		newObj->wounds->damage = source->wounds->damage;
		newObj->wounds->bleeding = source->wounds->bleeding;
		newObj->wounds->poison = source->wounds->poison;
		newObj->wounds->infection = source->wounds->infection;
		newObj->wounds->healerskill = source->wounds->healerskill;
		newObj->wounds->bindskill = source->wounds->bindskill;
		newObj->wounds->lasthealed = source->wounds->lasthealed;
		newObj->wounds->lastbled = source->wounds->lastbled;
		newObj->wounds->lastbound = source->wounds->lastbound;
		newObj->wounds->location = duplicateString (source->wounds->location);
		newObj->wounds->type = duplicateString (source->wounds->type);
		newObj->wounds->name = duplicateString (source->wounds->name);
		newObj->wounds->severity = duplicateString (source->wounds->severity);
	}

	if (source->writing)
	{
		newObj->writing->language = source->writing->language;
		newObj->writing->script = source->writing->script;
		newObj->writing->skill = source->writing->skill;
		newObj->writing->torn = source->writing->torn;
		newObj->writing->message = duplicateString (source->writing->message);
		newObj->writing->author = duplicateString (source->writing->author);
		newObj->writing->date = duplicateString (source->writing->date);
		newObj->writing->ink = duplicateString (source->writing->ink);
	}

	if (source->lodged)
	{
		newObj->lodged->vnum = source->lodged->vnum;
		newObj->lodged->location = duplicateString (source->lodged->location);
	}

	if (source->clan_data)
	{
		newObj->clan_data->name = duplicateString (source->clan_data->name);
		newObj->clan_data->rank = duplicateString (source->clan_data->rank);
	}

	add_obj_to_hash (newObj);
	ch->pc->edit_obj = newVNum;

	result << "Copied object #C" << source->nVirtual << "#0 ('#2" << source->short_description << "#0') to #C" << newObj->nVirtual << "#0." << std::endl;
	send_to_char (result.str().c_str(), ch);

	return;
}


void
list_mob_resets (CHAR_DATA * ch, CHAR_DATA * mob)
{
	int reset_num = 0;
	RESET_DATA *reset;
	char buf[MAX_STRING_LENGTH];
	char month[AVG_STRING_LENGTH];
	char day[AVG_STRING_LENGTH];
	char hour[AVG_STRING_LENGTH];

	if (!mob || !mob->mob)
	{
		send_to_char ("No mob or not an NPC.\n", ch);
		return;
	}

	if (!mob->mob->resets)
	{
		act ("$N has no resets.", false, ch, 0, mob, TO_CHAR);
		return;
	}

	for (reset = mob->mob->resets; reset; reset = reset->next)
	{

		reset_num++;

		if (reset->when.month == -1)
			strcpy (month, "**");
		else
			sprintf (month, "%02d", reset->when.month);

		if (reset->when.day == -1)
			strcpy (day, "**");
		else
			sprintf (day, "%02d", reset->when.day);

		if (reset->when.hour == -1)
			strcpy (hour, "**");
		else
			sprintf (hour, "%02d", reset->when.hour);

		sprintf (buf, "%2d) %s:%s:%s:%02d.%02d %s\n",
			reset_num,
			month,
			day,
			hour, reset->when.minute, reset->when.second, reset->command);
		send_to_char (buf, ch);
	}
}

void
reset_insert (CHAR_DATA * ch, RESET_DATA * reset)
{
	RESET_DATA *treset;
	RESET_DATA *last_reset = NULL;

	for (treset = ch->mob->resets;
		treset; last_reset = treset, treset = treset->next)
	{

		if (treset->when.month > reset->when.month)
			break;

		if (treset->when.month < reset->when.month)
			continue;

		if (treset->when.day > reset->when.day)
			break;

		if (treset->when.day < reset->when.day)
			continue;

		if (treset->when.hour > reset->when.hour)
			break;

		if (treset->when.hour < reset->when.hour)
			continue;

		if (treset->when.minute > reset->when.minute)
			break;

		if (treset->when.minute < reset->when.minute)
			continue;
	}

	if (!last_reset)
	{
		reset->next = ch->mob->resets;
		ch->mob->resets = reset;
		return;
	}

	reset->next = last_reset->next;
	last_reset->next = reset;
}

int
reset_delete (CHAR_DATA * mob, int reset_num)
{
	RESET_DATA *reset;
	RESET_DATA *free_reset;
	int i = 2;

	if (!mob->mob->resets)
		return 0;

	if (reset_num == 1)
	{
		free_reset = mob->mob->resets;
		mob->mob->resets = free_reset->next;
		free_mem (free_reset);
		return 1;
	}

	for (reset = mob->mob->resets; reset && i < reset_num; reset = reset->next)
		i++;

	if (!reset || !reset->next)
		return 0;

	free_reset = reset->next;
	reset->next = free_reset->next;

	free_mem (free_reset);

	return 1;
}

void
do_resets (CHAR_DATA * ch, char *argument, int cmd)
{
	int month;
	int day;
	int hour;
	int minute = 0;
	int second;
	int time_args;
	int i1, i2, i3, i4;
	RESET_DATA *reset;
	char buf[MAX_STRING_LENGTH];

	if (!ch->pc)
	{
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	if (!ch->pc->target_mob || !is_he_somewhere (ch->pc->target_mob))
	{
		send_to_char ("Use the 'mob' command to set target mobile receiving "
			"the reset first.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		list_mob_resets (ch, ch->pc->target_mob);
		return;
	}

	if (!strcmp (buf, "?") || !str_cmp (buf, "help"))
	{
		send_to_char ("reset reply 'keywords' text\n", ch);
		send_to_char ("reset delete #\n", ch);
		send_to_char ("resets\n", ch);
		return;
	}

	if (!strcmp (buf, "delete"))
	{
		argument = one_argument (argument, buf);

		if (!atoi (buf))
		{
			send_to_char ("Delete which numbered reset?\n", ch);
			return;
		}

		if (reset_delete (ch->pc->target_mob, atoi (buf)))
			send_to_char ("Ok.\n", ch);
		else
			send_to_char ("No such reset.\n", ch);

		return;
	}

	if (!strcmp (buf, "reply"))
	{

		reset = new RESET_DATA;

		reset->type = RESET_REPLY;

		reset->command = duplicateString (argument);

		reset->when.month = -1;
		reset->when.day = -1;
		reset->when.hour = -1;
		reset->when.minute = -1;
		reset->when.second = -1;

		reset_insert (ch->pc->target_mob, reset);

		return;
	}

	time_args = sscanf (buf, "%d:%d:%d:%d", &i1, &i2, &i3, &i4);

	if (!time_args)
	{
		send_to_char ("Expected a time argument first:\n   MM:DD:HH:MM\n"
			"   DD:HH:MM\n   HH:MM  or\n   MM\n", ch);
		return;
	}

	month = -1;
	day = -1;
	hour = -1;
	second = 0;

	switch (time_args)
	{

	case 1:
		minute = i1;
		break;

	case 2:
		hour = i1;
		minute = i2;
		break;

	case 3:
		day = i1;
		hour = i2;
		minute = i3;
		break;

	case 4:
		month = i1;
		day = i2;
		hour = i3;
		minute = i4;
		break;
	}

	if (month != -1 && month > 12)
	{
		send_to_char ("Months are 1 through 12.\n", ch);
		return;
	}

	if (day != -1 && day > 30)
	{
		send_to_char ("There are only 30 days per month.\n", ch);
		return;
	}

	if (hour != -1 && hour > 23)
	{
		send_to_char ("Hours are 0 through 23.\n", ch);
		return;
	}

	if (minute > 59)
	{
		send_to_char ("Minutes are 0 though 59.\n", ch);
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		send_to_char ("What reset would you like to put on the mob?\n", ch);
		return;
	}

	reset = new RESET_DATA;

	reset->command = duplicateString (argument);

	reset->type = RESET_TIMED;

	reset->when.month = month;
	reset->when.day = day;
	reset->when.hour = hour;
	reset->when.minute = minute;
	reset->when.second = second;

	reset_insert (ch->pc->target_mob, reset);

	strcpy (buf, "Reset added to ");

	name_to_ident (ch->pc->target_mob, buf + strlen (buf));

	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	if (ch->pc->target_mob->in_room != ch->in_room)
	{
		sprintf (buf, "NOTE:  Target mob is in room %d, not in yours.\n",
			ch->pc->target_mob->in_room);
		send_to_char (buf, ch);
	}
}

void
do_document (CHAR_DATA * ch, char *argument, int cmd)
{
	int doc_type;
	int doc_num = 0;
	int length;
	int line_start;
	int add_doc = 0;
	int delete_doc = 0;
	char *start;
	char *p;
	char buf[MAX_STRING_LENGTH];

	/* text, document, help, bhelp */

	if (!ch->pc)
	{
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	p = one_argument (argument, buf);

	if (!str_cmp (buf, "add"))
	{
		add_doc = 1;
		argument = p;
	}
	else if (!str_cmp (buf, "delete"))
	{
		delete_doc = 1;
		argument = p;
	}

	if (add_doc || delete_doc)
	{

		doc_type = -1;

		argument = one_argument (p, buf);

		if (!str_cmp (buf, "help"))
			doc_type = EDIT_TYPE_HELP;

		else if (!str_cmp (buf, "bhelp"))
			doc_type = EDIT_TYPE_BHELP;

		else if (!str_cmp (buf, "text"))
		{
			send_to_char ("You can't add or delete TEXT documents.\n", ch);
			return;
		}

		else if (!str_cmp (buf, "document") || !str_cmp (buf, "doc"))
			doc_type = EDIT_TYPE_DOCUMENT;

		else
		{
			send_to_char ("Unknown document type.\n", ch);
			return;
		}

		if (add_doc)
		{

			if (doc_type == EDIT_TYPE_HELP)
				doc_num = (long int) add_help_topics (ch, &help_list, argument);

			else if (doc_type == EDIT_TYPE_BHELP)
				doc_num = (long int) add_help_topics (ch, &bhelp_list, argument);

			else if (doc_type == EDIT_TYPE_DOCUMENT)
				doc_num = (long int) add_document (ch, &document_list, argument);

			ch->pc->doc_type = doc_type;
			ch->pc->doc_index = doc_num;

			send_to_char ("Created.\n", ch);

			return;
		}

		else
		{

			if (doc_type == EDIT_TYPE_HELP)
				delete_help_topics (ch, &help_list, argument);

			else if (doc_type == EDIT_TYPE_BHELP)
				delete_help_topics (ch, &bhelp_list, argument);

			else if (doc_type == EDIT_TYPE_DOCUMENT)
				delete_document (ch, &document_list, argument);

			ch->pc->doc_index = -1;

			send_to_char ("Deleted.\n", ch);

			return;
		}
	}


	/* doc_parse sets pc->doc_type and pc->doc_index */

	if (-1 == (doc_num = doc_parse (ch, argument, &start, &length,
		&line_start, &doc_type)))
		return;
}

void
do_instruct (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	int ind;
	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("See help.\n", ch);
		return;
	}

	if (!(mob = get_char_room_vis (ch, buf)))
	{
		send_to_char ("No such person here.\n", ch);
		return;
	}

	if (GET_TRUST (ch))
		;
	else if (!IS_NPC (mob) ||
		!IS_SET (mob->act, ACT_STAYPUT) || !is_leader (ch, mob))
	{
		act ("You can't set flags on $M.", false, ch, 0, mob, TO_CHAR);
		return;
	}

	if (!IS_NPC (mob))
	{
		send_to_char ("Mobs only.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (!GET_TRUST (ch))
		{
			if (IS_SET (mob->act, ACT_SENTINEL))
				send_to_char ("  Sentinel:    Yes.  The mob will not "
				"wander.\n", ch);
			else
				send_to_char ("  Sentinel:    No.   The mob will "
				"wander.\n", ch);

			if (IS_SET (mob->act, ACT_AGGRESSIVE))
				send_to_char ("  Aggressive:  Yes.  The mob will attack "
				"non-allies.\n", ch);
			else
				send_to_char ("  Aggressive:  No.   The mob will not pick "
				"fights.\n", ch);

			if (IS_SET (mob->act, ACT_WIMPY))
				send_to_char ("  Wimpy:       Yes.  The mob will flee at the "
				"first sign of a fight.\n", ch);
			else
				send_to_char ("  Wimpy:       No.   The mob will stand and "
				"fight until the death.\n", ch);

			if (IS_SET (mob->act, ACT_PURSUE))
				send_to_char ("  Pursue:      Yes.  The mob will follow "
				"fleeing opponents.\n", ch);
			else
				send_to_char ("  Pursue:      No.   The mob will not follow "
				"fleeing opponents.\n", ch);

			if (IS_SET (mob->act, ACT_PASSIVE))
				send_to_char
				("  Passive:     Yes.  The mob will assist a clan brother in a fight.\n",
				ch);
			else
				send_to_char
				("  Passive:     No.   The mob will not assist a clan brother in a fight.\n",
				ch);
		}

		else
		{
			for (i = 0; *action_bits[i] != '\n'; i++)
				if (IS_SET (mob->act, 1 << i))
					sprintf (buf + strlen (buf), "%s ", action_bits[i]);

			send_to_char (buf, ch);
		}

		if (IS_SET (mob->act, ACT_SENTINEL))
			return;

		if (!mob->mob->access_flags)
		{
			act ("$N will wander anywhere.", false, ch, 0, mob, TO_CHAR);
			return;
		}

		send_to_char ("\nMob wanders into rooms flagged as:\n\n   ", ch);

		for (i = 0; *room_bits[i] != '\n'; i++)
		{
			if (IS_SET (mob->mob->access_flags, 1 << i))
			{
				send_to_char (room_bits[i], ch);
				send_to_char (" ", ch);
			}
		}

		send_to_char ("\n", ch);

		return;
	}

	while (*buf)
	{

		if (GET_TRUST (ch))
		{
			if ((ind = index_lookup (room_bits, buf)) != -1)
			{
				TOGGLE (mob->mob->access_flags, 1 << ind);
			}

			else if ((ind = index_lookup (action_bits, buf)) != -1)
			{
				TOGGLE (mob->act, 1 << ind);
			}

			else
			{
				send_to_char ("No such 'action-bits' or 'room-bits': ", ch);
				send_to_char (buf, ch);
				send_to_char ("\n", ch);
				return;
			}
		}

		else
		{
			if ((ind = index_lookup (action_bits, buf)) != -1 &&
				(1 << ind == ACT_AGGRESSIVE ||
				1 << ind == ACT_PURSUE ||
				1 << ind == ACT_SENTINEL ||
				1 << ind == ACT_WIMPY || 1 << ind == ACT_PASSIVE))
			{
				TOGGLE (mob->act, 1 << ind);
			}

			else if ((ind = index_lookup (room_bits, buf)) != -1)
			{
				TOGGLE (mob->mob->access_flags, 1 << ind);
			}

			else
			{
				send_to_char ("Expected a standing order of:\n\n"
					"     Aggressive, Passive, Pursue, Sentinel, or Wimpy\n\n"		  
					"or a access flag (rooms the mob will wonder into):\n\n"
					"     Dark, Ruins, Indoors, Lawful, Tunnel, Cave, Deep, "
					"Fall, Temple,\n"
					"     Climb, Lab, Road, Wealthy, Poor, Scum, Market, "
					"Arena, Dock, Wild,\n" "     or Light\n", ch);
				return;
			}
		}

		argument = one_argument (argument, buf);
	}
}

void
rlist_show (ROOM_DATA * troom, int sector, int header)
{
	char bitbuf[MAX_STRING_LENGTH] = { '\0' };

	sprintbit (troom->room_flags, room_bits, bitbuf);
	sprintf (b_buf + strlen (b_buf), "#6%s#0 #2[%d: %s#6%s#2]#0",
		troom->name,
		troom->nVirtual,
		bitbuf,
		sector_types[troom->sector_type]);

	if (troom->prg)
		sprintf (b_buf + strlen (b_buf), " [Prog]");

	strcat (b_buf, "\n");

	return;
}

/******
* rlist +/-<keyword> z<zone number> <sector type> /$<roomflag>
*******/
void
do_rlist (CHAR_DATA * ch, char *argument, int cmd)
{
	int header = 1;
	int sector = -1;
	int inclusive;
	int zone = -1;
	int yes_key1 = 0;
	int yes_key2 = 0;
	int yes_key3 = 0;
	int flag_key1 = -2;
	int flag_key2 = -2;
	int flag_key3 = -2;
	int count = 0;
	int yes_flag1 = 0;
	int yes_flag2 = 0;
	int yes_flag3 = 0;
	int inc_flags;
	ROOM_DATA *troom;
	char key1[MAX_STRING_LENGTH] = { '\0' };
	char key2[MAX_STRING_LENGTH] = { '\0' };
	char key3[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Selection Parameters:\n\n", ch);
		send_to_char
			("   +/-<room keyword>       Include/exclude room keyword.\n",
			ch);
		send_to_char ("   z <zone>                Rooms from zone only.\n",
			ch);
		send_to_char
			("   $<flag>                 Include rooms with rflag.\n", ch);
		send_to_char
			("   /<flag>                 Exclude rooms with rflag.\n", ch);
		send_to_char
			("   s  <sector>             Include rooms with sector-type.\n", ch);

		send_to_char ("\nExample:   rlist +bedroom -poor $scum s city z 10\n", ch);
		send_to_char
			("will only get bedrooms of non-poor rooms, that are scum, in the city of zone 10.\n", ch);
		return;
	}


	while (*buf)
	{

		inclusive = 1;
		inc_flags = 1;
		sector = index_lookup (sector_types, buf);

		if (strlen (buf) > 1
			&& isalpha (*buf)
			&& (sector != -1))
		{
			argument = one_argument (argument, buf);
			continue;
		}

		if (isdigit (*buf))
		{

			if ((zone = atoi (buf)) >= MAX_ZONE)
			{
				send_to_char ("Zone not in range 0..99\n", ch);
				return;
			}

			argument = one_argument (argument, buf);
			continue;
		}

		switch (*buf)
		{

		case '-':
			inclusive = 0;

		case '+':

			if (!buf[1])
			{
				send_to_char ("Expected keyname after '+/-'.\n", ch);
				return;
			}

			if (!*key1)
			{
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			}
			else if (!*key2)
			{
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			}
			else if (*key3)
			{
				send_to_char ("Sorry, at most three keywords.\n", ch);
				return;
			}
			else
			{
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;

		case 'z':

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || atoi (buf) >= MAX_ZONE)
			{
				send_to_char ("Expected valid zone after 'z'.\n", ch);
				return;
			}

			zone = atoi (buf);

			break;

		case '/':
			inc_flags = 0;

		case '$':
			//argument = one_argument (argument, buf);

			if (!buf[1])
			{
				send_to_char ("Expected flag name after '/ or $'.\n", ch);
				return;
			}

			if (flag_key1 == -2)
			{
				yes_flag1 = inc_flags;
				flag_key1 = index_lookup (room_bits, buf + 1);
			}
			else if (flag_key2 == -2)
			{
				yes_flag2 = inc_flags;
				flag_key2 = index_lookup (room_bits, buf + 1);
			}
			else if (flag_key3 != -2)
			{
				send_to_char ("Sorry, at most three flags.\n", ch);
				return;
			}
			else
			{
				yes_flag3 = inc_flags;
				flag_key3 = index_lookup (room_bits, buf + 1);
			}

			break;
		}

		argument = one_argument (argument, buf);
	} // while (*buf)

	*b_buf = '\0';

	for (troom = full_room_list; troom; troom = troom->lnext)
	{

		if (zone != -1 && troom->zone != zone)
			continue;

		if (sector != -1 && troom->sector_type != sector)
			continue;

		if (flag_key1 != -2)
		{
			if (yes_flag1 && !(IS_SET (troom->room_flags, (1 << flag_key1))))
				continue;
			else if (!yes_flag1 && (IS_SET (troom->room_flags, (1 << flag_key1))))
				continue;
		}

		if (flag_key2 != -2)
		{
			if (yes_flag2 && !(IS_SET (troom->room_flags, (1 << flag_key2))))
				continue;
			else if (!yes_flag2 && (IS_SET (troom->room_flags, (1 << flag_key2))))
				continue;
		}

		if (flag_key3 != -2)
		{
			if (yes_flag3 && !(IS_SET (troom->room_flags, (1 << flag_key3))))
				continue;
			else if (!yes_flag3 && (IS_SET (troom->room_flags, (1 << flag_key3))))
				continue;
		}


		if (*key1)
		{
			if (yes_key1 && !isname (key1, troom->name))
				continue;
			else if (!yes_key1 && isname (key1, troom->name))
				continue;
		}

		if (*key2)
		{
			if (yes_key2 && !isname (key2, troom->name))
				continue;
			else if (!yes_key2 && isname (key2, troom->name))
				continue;
		}

		if (*key3)
		{
			if (yes_key3 && !isname (key3, troom->name))
				continue;
			else if (!yes_key3 && isname (key3, troom->name))
				continue;
		}

		count++;

		if (count < 200)
			rlist_show (troom, sector, header); //prints the first 200 rooms to b_buf

		header = 0;
	}

	if (count > 200)
	{
		sprintf (buf,
			"You have selected %d rooms (too many to print).\n",
			count);
		send_to_char (buf, ch);
		return;
	}

	page_string (ch->desc, b_buf);
}

void
do_rcap (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_DATA *room;

	room = vtor (ch->in_room);
	argument = one_argument (argument, buf);


	if (!isdigit (*buf))
	{
		send_to_char ("Syntax:  rcap [capacity]\n", ch);
		return;
	}

	room->capacity = atoi (buf);

	*buf = '\0';

	sprintf(buf, "Room capacity is now %d individuals.", room->capacity);

	send_to_char (buf, ch);
}

void
do_name (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	std::string strArgument = argument, ThisArg;

	strArgument = one_argument (strArgument, ThisArg);
	if (ThisArg.empty())
	{
		send_to_char("Name who?\n", ch);
		return;
	}
	if (!ThisArg.compare("self"))
	{
		tch = ch;
	}
	else
	{
		tch = get_char_room_vis(ch, (char *) ThisArg.c_str());
	}

	if (!tch)
	{
		send_to_char("You do not see that person to rename.\n", ch);
		return;
	}

	if (tch != ch && !IS_NPC(tch) && IS_MORTAL(ch))
	{
		act("Mortals may only change their own keywords (via #6self#0), or the keywords of NPCs they own.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (IS_MORTAL(ch) && IS_NPC(tch) && (!tch->mob->owner || strn_cmp(tch->mob->owner, ch->tname, strlen(tch->mob->owner))))
	{
		act("Mortals may only change their own keywords (via #6self#0), or the keywords of NPCs they own.", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	strArgument = one_argument (strArgument, ThisArg);
	if (ThisArg.empty())
	{
		send_to_char ("Do you wish to add, delete, or view names?\n", ch);
		return;
	}
	if (!ThisArg.compare("add"))
	{
		strArgument = one_argument (strArgument, ThisArg);
		if (ThisArg.empty())
		{
			send_to_char("What keyword would you like to add?\n", ch);
			return;
		}
		std::string keywords = tch->name;
		if (keywords.find(ThisArg) != std::string::npos)
		{
			send_to_char("That is already a keyword.\n", ch);
			return;
		}
		if (keywords.length() > 120)
		{
			send_to_char("You cannot have more than 120 characters of keywords, please remove some first.\n", ch);
			return;
		}
		keywords += " " + ThisArg;
		free_mem(tch->name);
		tch->name = duplicateString ((char *) keywords.c_str());
		if (!IS_NPC(tch))
			save_char(tch, true);
		else
			save_stayput_mobiles();
		std::string sshort = (tch == ch) ? "you" : char_short(tch);
		std::string output = "Keyword: #2" + ThisArg + "#0 installed for #5" + sshort + "#0.\n";
		send_to_char((char *) output.c_str(), ch);
		return;
	}
	else if (!ThisArg.compare("delete"))
	{
		strArgument = one_argument (strArgument, ThisArg);
		if (ThisArg.empty())
		{
			send_to_char ("What keyword would you like to delete?\n", ch);
			return;
		}
		std::string name = tch->tname;
		name[0] = tolower(name[0]);
		if (name.find(ThisArg) != std::string::npos)
		{
			send_to_char("You cannot remove the name from the keywords.\n", ch);
			return;
		}
		std::string keywords = tch->name;
		if (keywords.find(MAKE_STRING(" ") + ThisArg) == std::string::npos && keywords.find(ThisArg + " ") == std::string::npos)
		{
			send_to_char("That isn't a keyword.\n", ch);
			return;
		}
		std::string sdesc = tch->short_descr;
		if (sdesc.find(ThisArg) != std::string::npos)
		{
			send_to_char("You cannot remove keywords that are a part of the description.\n", ch);
			return;
		}
		std::string NewKeywords;
		while (!keywords.empty())
		{
			keywords = one_argument(keywords, sdesc);
			if (sdesc.compare(ThisArg))
			{
				if (name.find(sdesc) != std::string::npos)
					sdesc[0] = toupper(sdesc[0]);
				if (NewKeywords.empty())
					NewKeywords += sdesc;
				else
					NewKeywords += " " + sdesc;
			}
		}
		free_mem(tch->name);
		tch->name = duplicateString((char *) NewKeywords.c_str());
		if (!IS_NPC(tch))
			save_char(tch, true);
		else
			save_stayput_mobiles();
		std::string sshort = (tch == ch) ? "you" : char_short(tch);
		std::string output = "Keyword: #2" + ThisArg + "#0 removed for #5" + sshort + "#0.\n";
		send_to_char(output.c_str(), ch);
		return;

	}
	else if (!ThisArg.compare("view") || !ThisArg.compare("list"))
	{
		std::string output = MAKE_STRING(((tch == ch) ? "#5You#0 have" : (MAKE_STRING(char_short(tch)) + "has"))) + " the following keywords: " + MAKE_STRING(tch->name) + ".\n";
		send_to_char(output.c_str(), ch);
		return;
	}
	else
	{
		send_to_char("Do you wish to add, delete, or view names?\n", ch);
		return;
	}
}

int
save_room_affects (int zone)
{
	char buf[MAX_INPUT_LENGTH];
	char rafile[MAX_STRING_LENGTH];
	ROOM_DATA *troom;
	FILE *fr;
	int room_good;
	int n;
	int tmp;
	int empty_rooms = 0;
	static int total_empty_rooms = 0;
	
	sprintf (buf, "Saving rooms affects in zone %d.", zone);
	system_log (buf, false);
	
	sprintf (buf, "%s/raffects/resets.%d", SAVE_DIR, zone);	
	
	
	if ((fr = fopen (buf, "w+")) == NULL)
		return NULL;
		
		
	for (troom = full_room_list; troom; troom = troom->lnext)
		if ((troom->zone == zone) && (troom->affects))
		{
			fwrite_room_affects (troom, fr);
		}
	
	fprintf (fr, "$~\n");
	fclose (fr);
	
	
	return 0;
}

void
fwrite_room_affects (ROOM_DATA * troom, FILE * fp)
{
	
	AFFECTED_TYPE *af;
	
	if (troom->affects)
	{
		fprintf (fp, "#%d\n", troom->nVirtual );
		for (af = troom->affects; af; af = af->next)
		{
			fprintf (fp, "r %d %d %d\n", 
					 af->type,
					 af->a.room.duration,
					 af->a.room.intensity);
		}
	}
	
	fprintf (fp, "S\n");
}
