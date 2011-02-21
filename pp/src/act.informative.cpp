/*------------------------------------------------------------------------\
|  act.informative.c : Informational Module           www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"

#include "clan.h"
#include "group.h"
#include "utility.h"

extern rpie::server engine;
extern const char *skills[];
//added fro REPAIR
extern const char *unspecified_conditions[];
extern const char *fabric_conditions[];
extern const char *leather_conditions[];
extern const char *wood_conditions[];
extern const char *bone_conditions[];
extern const char *damage_severity[]; 
//end add for repair

BOARD_DATA *full_board_list = NULL;

#define LAST_STABLE_TICKET 1000

const char *month_name[12] = {
	"month of Narvinye",
	"month of Nenime",
	"month of Sulime",
	"month of Viresse",
	"month of Lotesse",
	"month of Narie",
	"month of Cermie",
	"month of Urime",
	"month of Yavannie",
	"month of Narquelie",
	"month of Hisime",
	"month of Ringare"
};

int loc_order[MAX_WEAR] = {
	WEAR_LIGHT,
	WEAR_SHIELD,
	WEAR_PRIM,
	WEAR_SEC,
	WEAR_BOTH,
	WEAR_UNUSED_1,
	WEAR_CARRY_R,
	WEAR_CARRY_L,

	WEAR_HEAD,
	WEAR_HAIR,
	WEAR_EAR,
	WEAR_BLINDFOLD,
	WEAR_FACE,
	WEAR_THROAT,
	WEAR_NECK_1,
	WEAR_NECK_2,

	WEAR_ABOUT,
	WEAR_BODY,
	WEAR_BACK,
	WEAR_SHOULDER_R,
	WEAR_SHOULDER_L,
	WEAR_ARMBAND_R,
	WEAR_ARMBAND_L,
	WEAR_ARMS,
	WEAR_WRIST_R,
	WEAR_WRIST_L,
	WEAR_HANDS,
	WEAR_FINGER_R,
	WEAR_FINGER_L,

	WEAR_WAIST,
	WEAR_BELT_1,
	WEAR_BELT_2,

	WEAR_LEGS,
	WEAR_ANKLE_R,
	WEAR_ANKLE_L,
	WEAR_FEET
};

static char *strTimeWord[] = {
	"twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
	"nine", "ten", "eleven",
	"twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
	"nine", "ten", "eleven",
	"twelve"
};

const char *fog_states[] = {
	"no fog",
	"thin fog",
	"thick fog",
	"\n"
};

const char *weather_states[] = {
	"no rain",
	"chance of rain",
	"light rain",
	"steady rain",
	"heavy rain",
	"light snow",
	"steady snow",
	"heavy snow",
	"\n"
};

const char *weather_clouds[] = {
	"clear sky",
	"light clouds",
	"heavy clouds",
	"overcast",
	"\n"
};


const char *wind_speeds[] = {
	"calm",
	"breeze",
	"windy",
	"gale",
	"stormy",
	"\n"
};

const char *wind_directions[] = {
	"westerly",
	"north westerly",
	"northerly",
	"north easterly",
	"easterly",
	"south easterly",
	"southerly",
	"south westerly",
	"\n"
};

const char *special_effects[] = {
	"no effect",
	"volcanic smoke",
	"foul stench",
	"low mist",
	"\n"
};


const char *holiday_names[] = {
	"(null)",
	"the feastday of mettare",
	"the feastday of yestare",
	"the feastday of tuilere",
	"the feastday of loende",
	"the feastday of enderi",
	"the feastday of yaviere",
	"\n"
};

const char *holiday_short_names[] = {
	"(null)",
	"mettare",
	"yestare",
	"tuilere",
	"loende",
	"enderi",
	"yaviere",
	"\n"
};

void
do_credits (CHAR_DATA * ch, char *argument, int cmd)
{
	do_help (ch, "diku_license", 0);
	return;
}

void
target_sighted (CHAR_DATA * ch, CHAR_DATA * target)
{
	SIGHTED_DATA *sighted = NULL;

	if (!ch || !target)
		return;

	if (!ch->sighted)
	{
		ch->sighted = new SIGHTED_DATA;
		ch->sighted->next = NULL;
		ch->sighted->target = target;
		return;
	}
	for (sighted = ch->sighted; sighted; sighted = sighted->next)
	{
		if (sighted->target == target)
			return;
		if (!sighted->next)
		{
			sighted->next = new SIGHTED_DATA;
			sighted->next->next = NULL;
			sighted->next->target = target;
			return;
		}
	}
}


void
do_point (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char distance[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *target = NULL, *tch = NULL;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *room = NULL;
	int dir = 0, range = 1;

	if (!*argument)
	{
		send_to_char ("Usage: point <direction> <target>\n", ch);
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (!strn_cmp ("north", arg1, strlen (arg1)))
		dir = 0;
	else if (!strn_cmp ("east", arg1, strlen (arg1)))
		dir = 1;
	else if (!strn_cmp ("south", arg1, strlen (arg1)))
		dir = 2;
	else if (!strn_cmp ("west", arg1, strlen (arg1)))
		dir = 3;
	else if (!strn_cmp ("up", arg1, strlen (arg1)))
		dir = 4;
	else if (!strn_cmp ("down", arg1, strlen (arg1)))
		dir = 5;
	else
	{
		send_to_char ("Usage: point <direction> <target>\n", ch);
		return;
	}

	if (!EXIT (ch, dir))
	{
		send_to_char ("There isn't an exit in that direction.\n", ch);
		return;
	}

	room = vtor (EXIT (ch, dir)->to_room);
	exit = EXIT (ch, dir);

	if (exit
		&& IS_SET (exit->exit_info, EX_ISDOOR)
		&& IS_SET (exit->exit_info, EX_CLOSED)
		&& !IS_SET (exit->exit_info, EX_ISGATE))
	{
		send_to_char ("Your view is blocked.\n", ch);
		return;
	}

	if (!(target = get_char_room_vis2 (ch, room->nVirtual, arg2))
		|| !has_been_sighted (ch, target))
	{
		exit = room->dir_option[dir];
		if (!exit)
		{
			send_to_char ("You don't see them within range.\n", ch);
			return;
		}
		if (exit
			&& IS_SET (exit->exit_info, EX_ISDOOR)
			&& IS_SET (exit->exit_info, EX_CLOSED)
			&& !IS_SET (exit->exit_info, EX_ISGATE))
		{
			send_to_char ("Your view is blocked.\n", ch);
			return;
		}
		if (room->dir_option[dir])
			room = vtor (room->dir_option[dir]->to_room);
		else
			room = NULL;
		if (is_sunlight_restricted (ch))
			return;
		if (!(target = get_char_room_vis2 (ch, room->nVirtual, arg2))
			|| !has_been_sighted (ch, target))
		{
			exit = room->dir_option[dir];
			if (!exit)
			{
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}
			if (exit
				&& IS_SET (exit->exit_info, EX_ISDOOR)
				&& IS_SET (exit->exit_info, EX_CLOSED)
				&& !IS_SET (exit->exit_info, EX_ISGATE))
			{
				send_to_char ("Your view is blocked.\n", ch);
				return;
			}
			if (room->dir_option[dir])
				room = vtor (room->dir_option[dir]->to_room);
			else
				room = NULL;
			if (!(target = get_char_room_vis2 (ch, room->nVirtual, arg2))
				|| !has_been_sighted (ch, target))
			{
				exit = room->dir_option[dir];
				if (!exit)
				{
					send_to_char ("You don't see them within range.\n", ch);
					return;
				}
				if (exit
					&& IS_SET (exit->exit_info, EX_ISDOOR)
					&& IS_SET (exit->exit_info, EX_CLOSED)
					&& !IS_SET (exit->exit_info, EX_ISGATE))
				{
					send_to_char ("Your view is blocked.\n", ch);
					return;
				}
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}
			else
				range = 3;
		}
		else
			range = 2;
	}
	else
		range = 1;

	if (!target || !CAN_SEE (ch, target) || !has_been_sighted (ch, target))
	{
		send_to_char ("You don't see them within range.\n", ch);
		return;
	}

	if (range == 2)
		sprintf (distance, "far ");
	else if (range == 3)
		sprintf (distance, "very far ");
	else
		*distance = '\0';

	sprintf (distance + strlen (distance), "to the %s", dirs[dir]);

	sprintf (buf, "You point at #5%s#0, %s.", char_short (target), distance);
	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "%s#0 points at #5%s#0, %s.", char_short (ch),
		char_short (target), distance);
	*buf = toupper (*buf);

	sprintf (buffer, "#5%s", buf);
	sprintf (buf, "%s", buffer);
	act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!CAN_SEE (tch, ch))
			continue;
		target_sighted (tch, target);
	}
}

void
do_title (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char echo[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *obj = NULL;
	double skill = 0;

	if (!*argument)
	{
		send_to_char ("What did you wish to title?\n", ch);
		return;
	}

	if (!strstr (argument, "\"") && !strstr (argument, "\'"))
	{
		send_to_char
			("You must enclose the book's desired title in quotation marks.\n",
			ch);
		return;
	}

	if (!ch->writes)
	{
		send_to_char
			("In which script would you like to write? (See the SCRIBE command.)\n",
			ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		send_to_char ("You need to be holding the book you wish to title.\n",
			ch);
		return;
	}

	if (obj->book_title && IS_MORTAL (ch))
	{
		send_to_char ("This work has already been titled.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) != ITEM_BOOK)
	{
		send_to_char ("This command only works with books.\n", ch);
		return;
	}

	skill =
		(ch->skills[ch->writes] * 0.70) + (ch->skills[ch->speaks] * 0.30);

	skill = (int) skill;
	skill = MIN (95, (int) skill);

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What did you wish to title this work?\n", ch);
		return;
	}

	if (strlen (buf) > 55)
	{
		send_to_char ("There is a 55-character limit on book titles.\n", ch);
		return;
	}

	obj->book_title = duplicateString (buf);
	obj->title_skill = (int) skill;
	obj->title_script = ch->writes;
	obj->title_language = ch->speaks;

	sprintf (echo, "You have entitled #2%s#0 '%s'.", obj->short_description,
		buf);
	act (echo, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

}

char *
parse_room_description (CHAR_DATA * ch, char *desc)
{
	return desc;
}

void
do_timeconvert (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int hour = 0, day = 0, month = 0, year = 0;
	time_t temp_time = 0;
	struct tm real_date;
	struct time_info_data game_date;
	char suf[5] = { '\0' };

	if (!ch->desc || !ch->desc->acct)
	{
		send_to_char ("Only PCs can use this command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	hour = atoi (buf);

	if (hour > 23 || hour < 0)
	{
		send_to_char ("You must specify an hour between 0 and 23.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	day = atoi (buf);

	if (day > 31 || day < 1)
	{
		send_to_char ("You must specify a day between 1 and 31.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	month = atoi (buf);

	if (month > 12 || month < 1)
	{
		send_to_char ("You must specify a month between 1 and 12.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	year = atoi (buf);

	if (year < 2600 && year > 2460)
	{

		if (day > 30 || day < 1)
		{
			send_to_char ("You must specify a day between 1 and 30.\n", ch);
			return;
		}

#ifndef MACOSX
		temp_time =
			GAME_SECONDS_BEGINNING + timezone +
			(int)(ch->desc->acct->timezone * 60.0 * 60.0);
		temp_time +=
			((((year - GAME_BASE_YEAR) * GAME_SECONDS_PER_YEAR) +
			((month - 1) * GAME_SECONDS_PER_MONTH) +
			((day - 1) * GAME_SECONDS_PER_DAY) +
			(hour * GAME_SECONDS_PER_HOUR)) / PULSES_PER_SEC);
#else
		(int) temp_time =
			GAME_SECONDS_BEGINNING +
			(int) (ch->desc->acct->timezone * 60.0 * 60.0);
		(int) temp_time +=
			((((year - GAME_BASE_YEAR) * GAME_SECONDS_PER_YEAR) +
			((month - 1) * GAME_SECONDS_PER_MONTH) +
			((day - 1) * GAME_SECONDS_PER_DAY) +
			(hour * GAME_SECONDS_PER_HOUR)) / PULSES_PER_SEC);
#endif

		strftime (buf, 255,
			"In your timezone, the specified in-game time will fall at or near %I:%M %P, %A %B %e %Y.",
			localtime (&temp_time));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	}
	else if (year < 2100 && year > 2002)
	{

		real_date.tm_hour = hour;
		real_date.tm_mday = day;
		real_date.tm_mon = month - 1;
		real_date.tm_year = year - 1900;
		real_date.tm_sec = 0;
		real_date.tm_min = 0;

#ifndef MACOSX
		temp_time =
			((mktime (&real_date))
			- ((int)(ch->desc->acct->timezone * 60.0 * 60.0)))
			- timezone;
#else
		(int) temp_time =
			(((int) mktime (&real_date))
			- ((int) (ch->desc->acct->timezone * 60.0 * 60.0)))
			- (int) timezone;
#endif
		game_date = mud_time_passed (temp_time, GAME_SECONDS_BEGINNING);

		day = game_date.day + 1;

		if (day == 1)
			strcpy (suf, "st");
		else if (day == 2)
			strcpy (suf, "nd");
		else if (day == 3)
			strcpy (suf, "rd");
		else if (day < 20)
			strcpy (suf, "th");
		else if ((day % 10) == 1)
			strcpy (suf, "st");
		else if ((day % 10) == 2)
			strcpy (suf, "nd");
		else if ((day % 10) == 3)
			strcpy (suf, "rd");
		else
			strcpy (suf, "th");

		sprintf (buf,
			"In your timezone, the specified time will fall at or near %d:00 %s on the %d%s day of the %s in the year %d of the Steward's Reckoning.",
			(game_date.hour ==
			0) ? 12 : ((game_date.hour >
			12) ? game_date.hour - 11 : game_date.hour),
			(game_date.hour <= 12) ? "am" : "pm", game_date.day + 1, suf,
			month_name[(int) game_date.month], game_date.year);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

		return;

	}
	else
	{
		send_to_char ("You must specify a year between 2460 and 2600.\n", ch);
	}

}

char *frame_built[] = {
	"fragily-built",
	"scantly-built",
	"lightly-built",
	"typically-built",
	"heavily-built",
	"massively-built",
	"gigantically-built",
	"\n"
};


char *verbal_intox[] =
{ "sober", "tipsy", "slightly drunk", "drunk", "intoxicated",
"plastered"
};

char *verbal_hunger[] =
{ "starving", "hungry", "feeling slightly hungry", "feeling peckish",
"quite full",
"absolutely stuffed"
};

char *verbal_thirst[] =
{ "dying of thirst", "quite parched", "feeling thirsty",
"feeling slightly thirsty",
"nicely quenched", "completely sated"
};

void post_message (DESCRIPTOR_DATA * d);

int
armor_total (CHAR_DATA * ch)
{
	OBJ_DATA *armor = NULL;
	int total = 0;

	for (armor = ch->equip; armor; armor = armor->next_content)
		if (GET_ITEM_TYPE (armor) == ITEM_ARMOR)
			total += armor->o.od.value[0];

	return total;
}

char *
tilde_eliminator (char *string)
{
	char *p = '\0';

	while ((p = strchr (string, '~')))
		*p = '-';

	return duplicateString (string);
}

char *
carry_desc (OBJ_DATA * obj)
{
	if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH)
		return "wielding";

	else if (obj->location == WEAR_SHIELD)
		return "gripping";

	else
		return "carrying";
}

char *
mana_bar (CHAR_DATA * ch)
{
	static char buf[25] = { '\0' };
	float calc = 0;
	float move = 0;

	if (ch->mana >= ch->max_mana)
		sprintf (buf, "#C******#0");

	if ((move = ch->mana) >= (calc = ch->max_mana * .6667)
		&& ch->mana < ch->max_mana)
		sprintf (buf, "#C*****#0 ");

	if ((move = ch->mana) >= (calc = ch->max_mana * .5)
		&& (move = ch->mana) < (calc = ch->max_mana * .6667))
		sprintf (buf, "#C****#0  ");

	if ((move = ch->mana) >= (calc = ch->max_mana * .3333)
		&& (move = ch->mana) < (calc = ch->max_mana * .5))
		sprintf (buf, "#C***#0   ");

	if ((move = ch->mana) >= (calc = ch->max_mana * .1667)
		&& (move = ch->mana) < (calc = ch->max_mana * .3333))
		sprintf (buf, "#C**#0    ");

	if ((move = ch->mana) >= (calc = ch->max_mana * .0001)
		&& (move = ch->mana) < (calc = ch->max_mana * .1667))
		sprintf (buf, "#C*#0     ");

	if (ch->mana == 0)
		sprintf (buf, "       ");

	return buf;
}

char *
fatigue_bar (CHAR_DATA * ch)
{
	static char buf[25] = { '\0' };
	float calc = 0;
	float move = 0;

	if (ch->move >= ch->max_move)
		sprintf (buf, "#1||#3||#2||#0");

	if ((move = ch->move) >= (calc = ch->max_move * .6667)
		&& ch->move < ch->max_move)
		sprintf (buf, "#1||#3||#2|#0 ");

	if ((move = ch->move) >= (calc = ch->max_move * .5)
		&& (move = ch->move) < (calc = ch->max_move * .6667))
		sprintf (buf, "#1||#3||#0  ");

	if ((move = ch->move) >= (calc = ch->max_move * .3333)
		&& (move = ch->move) < (calc = ch->max_move * .5))
		sprintf (buf, "#1||#3|#0   ");

	if ((move = ch->move) >= (calc = ch->max_move * .1667)
		&& (move = ch->move) < (calc = ch->max_move * .3333))
		sprintf (buf, "#1||#0    ");

	if ((move = ch->move) >= (calc = ch->max_move * .0001)
		&& (move = ch->move) < (calc = ch->max_move * .1667))
		sprintf (buf, "#1|#0     ");

	if (ch->move == 0)
		sprintf (buf, "       ");


	return buf;
}

char *
breath_bar (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;
	static char buf[25] = { '\0' };
	int current = 0, max = 0;
	float calc = 0;
	float move = 0;

	if (!(af = get_affect (ch, AFFECT_HOLDING_BREATH)))
		return NULL;

	current = af->a.spell.duration;
	max = af->a.spell.sn;

	if (current >= max)
		sprintf (buf, "#6******#0");

	else if ((move = current) >= (calc = max * .6667) && current < max)
		sprintf (buf, "#6*****#0 ");

	else if ((move = current) >= (calc = max * .5)
		&& (move = current) < (calc = max * .6667))
		sprintf (buf, "#6****#0  ");

	else if ((move = current) >= (calc = max * .3333)
		&& (move = current) < (calc = max * .5))
		sprintf (buf, "#6***#0   ");

	else if ((move = current) >= (calc = max * .1667)
		&& (move = current) < (calc = max * .3333))
		sprintf (buf, "#6**#0    ");

	else if ((move = current) >= (calc = max * .0001)
		&& (move = current) < (calc = max * .1667))
		sprintf (buf, "#6*#0     ");

	else
		sprintf (buf, "      ");


	return buf;
}

/* Procedures related to 'look' */

void
argument_split_2 (char *argument, char *first_arg, char *second_arg)
{
	int look_at = 0, found = 0, begin = 0;
	found = begin = 0;

	if (!argument)
	{
		return;
	}

	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++);

	/* Find length of first word */
	for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

		/* Make all letters lower case, AND copy them to first_arg */
		*(first_arg + look_at) = tolower (*(argument + begin + look_at));
	*(first_arg + look_at) = '\0';
	begin += look_at;

	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++);

	/* Find length of second word */
	for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

		/* Make all letters lower case, AND copy them to second_arg */
		*(second_arg + look_at) = tolower (*(argument + begin + look_at));
	*(second_arg + look_at) = '\0';
	begin += look_at;
}

char *
find_ex_description (char *word, EXTRA_DESCR_DATA * list)
{
	EXTRA_DESCR_DATA *i = NULL;

	for (i = list; i; i = i->next)
		if (isname (word, i->keyword))
			return (i->description);

	return NULL;
}

/*
Old compare command is deprecated. It is no longer used on SOI
but will be left commented here in case it is wanted at a
later date - Valarauka

void
do_compare (CHAR_DATA * ch, char *argument, int cmd)
{
CHAR_DATA *tch = NULL, *tch2 = NULL;
int ch_height_rating = 0, tch_height_rating = 0, tch2_height_rating = 0;
char buf[250] = { '\0' }, arg1[MAX_STRING_LENGTH] =
{
'\0'}, target2[MAX_STRING_LENGTH] =
{
'\0'};

*target2 = '\0';
*arg1 = '\0';
*buf = '\0';

argument = one_argument (argument, arg1);

ch_height_rating = 8;
if (ch->height < 84)
ch_height_rating = 7;
if (ch->height < 77)
ch_height_rating = 6;
if (ch->height < 71)
ch_height_rating = 5;
if (ch->height < 66)
ch_height_rating = 4;
if (ch->height < 61)
ch_height_rating = 3;
if (ch->height < 54)
ch_height_rating = 2;
if (ch->height < 36)
ch_height_rating = 1;

if ((tch = get_char_room_vis (ch, arg1)))
{
tch_height_rating = 8;
if (tch->height < 84)
tch_height_rating = 7;
if (tch->height < 77)
tch_height_rating = 6;
if (tch->height < 71)
tch_height_rating = 5;
if (tch->height < 66)
tch_height_rating = 4;
if (tch->height < 61)
tch_height_rating = 3;
if (tch->height < 54)
tch_height_rating = 2;
if (tch->height < 36)
tch_height_rating = 1;
}
else
{
send_to_char ("Compare who?\n", ch);
return;
}

one_argument (argument, target2);

if (*target2)
{
if ((tch2 = get_char_room_vis (ch, target2)))
{
tch2_height_rating = 8;
if (tch2->height < 84)
tch2_height_rating = 7;
if (tch2->height < 77)
tch2_height_rating = 6;
if (tch2->height < 71)
tch2_height_rating = 5;
if (tch2->height < 66)
tch2_height_rating = 4;
if (tch2->height < 61)
tch2_height_rating = 3;
if (tch2->height < 54)
tch2_height_rating = 2;
if (tch2->height < 36)
tch2_height_rating = 1;
}
else
{
send_to_char ("I don't see the second target.\n", ch);
return;
}
}

if (tch2 == ch)
{
send_to_char ("Compare yourself to yourself? Eh?\n", ch);
return;
}

if (!tch2 && tch == ch)
{
send_to_char ("Compare yourself to yourself? Eh?\n", ch);
return;
}

if (tch == tch2)
{
send_to_char ("Comparing something to itself? Brilliant!\n", ch);
return;
}

if (!tch2)
{
if (tch_height_rating >= ch_height_rating)
{
if (tch_height_rating - ch_height_rating == 7)
sprintf (buf,
"#5%s#0 is %s, and is nearly three times your height!",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 6)
sprintf (buf, "#5%s#0 is %s, and is nearly twice your height!",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 5)
sprintf (buf, "#5%s#0 is %s, and soars above you.",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 4)
sprintf (buf, "#5%s#0 is %s, and towers above you.",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 3)
sprintf (buf, "#5%s#0 is %s, and is quite a bit taller than you.",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 2)
sprintf (buf,
"#5%s#0 is %s, and is considerably taller than you.",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating == 1)
sprintf (buf, "#5%s#0 is %s, and is a bit taller than you.",
char_short (tch), frame_built[tch->frame]);
if (tch_height_rating - ch_height_rating <= 0)
{
if (tch->height > ch->height)
sprintf (buf,
"#5%s#0 is %s, and is only slightly taller than you.",
char_short (tch), frame_built[tch->frame]);
else if (tch->height < ch->height)
sprintf (buf, "#5%s#0 is %s, and slightly shorter than you.",
char_short (tch), frame_built[tch->frame]);
else
sprintf (buf, "#5%s#0 is %s, and roughly the same height.",
char_short (tch), frame_built[tch->frame]);
}
}
else if (tch_height_rating < ch_height_rating)
{
if (ch_height_rating - tch_height_rating == 7)
sprintf (buf,
"#5%s#0 is %s, and is about a third of your height.",
char_short (tch), frame_built[tch->frame]);
if (ch_height_rating - tch_height_rating == 6)
sprintf (buf, "#5%s#0 is %s, and is roughly half your height.",
char_short (tch), frame_built[tch->frame]);
if (ch_height_rating - tch_height_rating == 5)
sprintf (buf, "#5%s#0 is %s, and you soar over #5%s#0.",
char_short (tch), frame_built[tch->frame],
!is_hooded (tch) ? HMHR (tch) : "it");
if (ch_height_rating - tch_height_rating == 4)
sprintf (buf, "#5%s#0 is %s, and you tower over #5%s#0.",
char_short (tch), frame_built[tch->frame],
!is_hooded (tch) ? HMHR (tch) : "it");
if (ch_height_rating - tch_height_rating == 3)
sprintf (buf,
"#5%s#0 is %s, and you are quite a bit taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
!is_hooded (tch) ? HMHR (tch) : "it");
if (ch_height_rating - tch_height_rating == 2)
sprintf (buf,
"#5%s#0 is %s, and you are considerably taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
!is_hooded (tch) ? HMHR (tch) : "it");
if (ch_height_rating - tch_height_rating == 1)
sprintf (buf,
"#5%s#0 is %s, and you are a bit taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
!is_hooded (tch) ? HMHR (tch) : "it");
}
}

else
{
if (tch_height_rating >= tch2_height_rating)
{
if (tch_height_rating - tch2_height_rating == 7)
sprintf (buf,
"#5%s#0 is %s, and is nearly three times #5%s#0's height!",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 6)
sprintf (buf,
"#5%s#0 is %s, and is nearly twice #5%s#0's height!",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 5)
sprintf (buf, "#5%s#0 is %s, and soars above #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 4)
sprintf (buf, "#5%s#0 is %s, and towers above #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 3)
sprintf (buf,
"#5%s#0 is %s, and is quite a bit taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 2)
sprintf (buf,
"#5%s#0 is %s, and is considerably taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating == 1)
sprintf (buf, "#5%s#0 is %s, and is a bit taller than #5%s#0.\n",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch_height_rating - tch2_height_rating <= 0)
{
if (tch->height > tch2->height)
sprintf (buf,
"#5%s#0 is %s, and is only slightly taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
else if (tch->height < tch2->height)
sprintf (buf,
"#5%s#0 is %s, and slightly shorter than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
else
sprintf (buf,
"#5%s#0 is %s, and roughly the same height as #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
}
}
if (tch_height_rating < tch2_height_rating)
{
if (tch2_height_rating - tch_height_rating == 7)
sprintf (buf,
"#5%s#0 is %s, and is about a third of #5%s#0's height.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch2_height_rating - tch_height_rating == 6)
sprintf (buf,
"#5%s#0 is %s, and is roughly half #5%s#0's height.",
char_short (tch), frame_built[tch->frame],
char_short (tch2));
if (tch2_height_rating - tch_height_rating == 5)
sprintf (buf, "#5%s#0 is %s, and %s soars over #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
if (tch2_height_rating - tch_height_rating == 4)
sprintf (buf, "#5%s#0 is %s, and %s towers over #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
if (tch2_height_rating - tch_height_rating == 3)
sprintf (buf,
"#5%s#0 is %s, and %s is quite a bit taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
if (tch2_height_rating - tch_height_rating == 2)
sprintf (buf,
"#5%s#0 is %s, and %s is considerably taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
if (tch2_height_rating - tch_height_rating == 1)
sprintf (buf, "#5%s#0 is %s, and %s is a bit taller than #5%s#0.",
char_short (tch), frame_built[tch->frame],
char_short (tch2), !is_hooded (tch) ? HMHR (tch) : "it");
}
}

act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

}*/

/* New version of the compare command to compare objects

This function takes two object references as arguments
and compares attributes such as weight and cost to give
an in character comparison string. - Valarauka

*/
void
do_compare(CHAR_DATA * ch, char *argument, int cmd)
{
	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	char		arg2 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj1 = NULL;
	OBJ_DATA	*obj2 = NULL;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };

	/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( GET_POS (ch) < POSITION_SLEEPING ) {
		send_to_char ("You are unconscious!\n", ch);
		return;
	}

	if ( GET_POS (ch) == POSITION_SLEEPING ) {
		send_to_char ("You are asleep.\n", ch);
		return;
	}

	if ( is_blind (ch) ) {
		send_to_char ("You are blind!\n", ch);
		return;
	}

	/*** Make sure enough arguments are spcified***/

	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		send_to_char ("Compare what?\n", ch);
		return;
	}

	argument = one_argument (argument, arg2);

	if ( !*arg2 ) {
		send_to_char ("Compare it with what?\n", ch);
		return;
	}


	/*** Find the objects being compared ***/

	if ( !(obj1 = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->equip)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->room->contents))) {

			send_to_char ("You don't see that.\n", ch);
			return;
	}

	if ( !(obj2 = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->equip)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->room->contents))) {

			send_to_char ("You don't see that.\n", ch);
			return;
	}

	/*** Compared objects must be of the same type ***/

	if(GET_ITEM_TYPE(obj1)!=GET_ITEM_TYPE(obj2))
	{
		send_to_char("You can only compare similar objects.",ch);
		return;
	}

	/*** Cannot compare something to itself ***/
	if(obj1==obj2)
	{
		send_to_char("Compare it with itself?",ch);
		return;
	}

	/*** Start Comparison Proper ***/

	sprintf (buffer, "\nYou compare #2%s#0 with #2%s#0.",
		obj1->short_description,
		obj2->short_description);

	act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	send_to_char("\n",ch);

	/*** Compare weights ***/
	if((obj1->obj_flags.weight + obj1->contained_wt) >
		(obj2->obj_flags.weight + obj2->contained_wt))
	{
		sprintf (buffer, "\nAfter a quick appraisal, you guess that #2%s#0 is heavier than #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}
	else if((obj2->obj_flags.weight + obj2->contained_wt) >
		(obj1->obj_flags.weight + obj1->contained_wt))
	{
		sprintf (buffer, "\nAfter a quick appraisal, you guess that #2%s#0 is lighter than #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}
	else
	{
		sprintf (buffer, "\nAfter a quick appraisal, you guess that #2%s#0 weighs about the same as #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}

	act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	send_to_char ("\n", ch);

	/*** Compare values ***/

	if((obj1->farthings + obj1->silver *4) >
		(obj2->farthings + obj2->silver *4))
	{
		sprintf (buffer, "\nYou estimate that #2%s#0 looks to be worth more than #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}
	else if((obj2->farthings + obj2->silver *4) >
		(obj1->farthings + obj1->silver *4))
	{
		sprintf (buffer, "\nYou estimate that #2%s#0 looks to be worth less than #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}
	else
	{
		sprintf (buffer, "\nYou estimate that #2%s#0 looks to be worth about the same as #2%s#0\n. ",
			obj1->short_description,
			obj2->short_description);
	}

	act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	send_to_char ("\n", ch);

	/*** If food item compare decay timers***/
	if(GET_ITEM_TYPE (obj1) == ITEM_FOOD)
	{
		if ((obj1->morphTime)&&(obj2->morphTime))
		{
			int time1, time2;

			time1 = obj1->morphTime - time (0);
			time2 = obj2->morphTime - time (0);

			if(time1>time2)
			{
				sprintf (buffer, "\nAs you look over #2%s#0 you notice that it appears fresher than #2%s#0\n. ",
					obj1->short_description,
					obj2->short_description);
			}
			else if(time2>time1)
			{
				sprintf (buffer, "\nAs you look over #2%s#0 you notice that it appears less fresh than #2%s#0\n. ",
					obj1->short_description,
					obj2->short_description);
			}
			else
			{
				sprintf (buffer, "\nAs you look over #2%s#0 you notice that it appears about as fresh as #2%s#0\n. ",
					obj1->short_description,
					obj2->short_description);
			}

			act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buffer = '\0';
			send_to_char ("\n", ch);
		}
	}

	/***** Compare time left in light objects ***/
	if ( GET_ITEM_TYPE (obj1) == ITEM_LIGHT )
	{
		if ( obj1->o.light.hours > obj2->o.light.hours )
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will provide light for longer than #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}
		else if(obj2->o.light.hours > obj1->o.light.hours)
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will provide light for less time than #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}
		else
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will provide light for about as long as #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}

		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);

	}

	/**** Compare capacity of containers ****/

	if ((GET_ITEM_TYPE (obj1) == ITEM_CONTAINER)||
		( GET_ITEM_TYPE (obj1) == ITEM_DRINKCON ))
	{
		if(obj1->o.container.capacity > obj2->o.container.capacity)
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will hold more than #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}
		else if(obj2->o.container.capacity > obj1->o.container.capacity)
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will hold less than #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}
		else
		{
			sprintf (buffer, "\nFrom the look of it #2%s#0 will hold about the same as #2%s#0\n. ",
				obj1->short_description,
				obj2->short_description);
		}

		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);
	}


	return;
}

char *
writing_adj (int skill)
{
	if (skill <= 10)
		return "crudely";
	if (skill <= 20)
		return "poorly";
	if (skill <= 30)
		return "functionally";
	if (skill <= 40)
		return "with skill";
	if (skill <= 50)
		return "with great skill";
	if (skill <= 60)
		return "artfully";
	if (skill <= 70)
		return "beautifully";
	return "flawlessly";
}

int
decipher_script (CHAR_DATA * ch, int script, int language, int skill)
{
	double check = 0;

	if (!real_skill (ch, script) || !real_skill (ch, language))
		return 0;

	if (skill > 0 && skill <= 15)
		check = 70;
	else if (skill > 15 && skill < 30)
		check = 50;
	else if (skill >= 30 && skill < 50)
		check = 30;
	else if (skill >= 50 && skill < 70)
		check = 20;
	else if (skill >= 70)
		check = 10;

	skill_use (ch, script, 0);
	skill_use (ch, language, 0);

	if (((ch->skills[script] * .70) + (ch->skills[language] * .30) ) >= check)
		return 1;
	else
		return 0;
}

void
reading_check (CHAR_DATA * ch, OBJ_DATA * obj, WRITING_DATA * writing,
			   int page)
{
	static char output[MAX_STRING_LENGTH] = { '\0' };

	if (!writing || !writing->message)
	{
		send_to_char
			("There seems to be a problem with this object. Please report it to a staff member.\n",
			ch);
		return;
	}

	if (!ch->skills[writing->script]
	&& strcasecmp (writing->author, ch->tname) != STR_MATCH)
	{
		sprintf (output,
			"This document is written in a script entirely unfamiliar to you.");
		act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!ch->skills[writing->language] && real_skill (ch, writing->script)
		&& strcasecmp (writing->author, ch->tname) != STR_MATCH)
	{
		sprintf (output,
			"Although you recognise the script as %s, the mode and language in which this document is written is unknown to you.",
			skills[writing->script]);
		act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!decipher_script
		(ch, writing->script, writing->language, writing->skill)
		&& (strcasecmp (writing->author, ch->tname) != STR_MATCH
		|| !real_skill (ch, writing->script)))
	{
		sprintf (output,
			"You find that you can make neither heads nor tails of this document.");
		act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!page)
	{
		sprintf (output,
			"#2On %s#0, %s sigils of the %s mode, scribed %s in %s, bear a message in the %s tongue:",
			obj->short_description, skills[writing->script],skills[writing->language],
			writing_adj (writing->skill), writing->ink,
			skills[writing->language]);
	}
	else
	{
		sprintf (output,
			"On #2page %d#0, %s sigils of the %s mode, scribed %s in %s, bear a message in the %s tongue:",
			page - 1, skills[writing->script],skills[writing->language],
			writing_adj (writing->skill), writing->ink,
			skills[writing->language]);
	}

	act (output, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	send_to_char ("\n", ch);

	if (strcasecmp (writing->author, ch->tname) != STR_MATCH)
	{
		skill_use (ch, writing->script, 0);
		if (!number (0, 1))
			skill_use (ch, writing->language, 0);
	}

	sprintf (output, "%s", writing->message);
	page_string (ch->desc, output);
}

char *
article (const char *string)
{
	if (strcasecmp (string, "something") == STR_MATCH)
		return "";

	if (*string == 'a' || *string == 'e' || *string == 'i' ||
		*string == 'o' || *string == 'u')
		return "an ";
	else
		return "a ";
}//Add for REPAIR
/************************************************************
 * mode == 0 - general look, no tables or corpses
 * mode == 1 - a container object that has contents
 * mode == 2
 * mode == 3
 * mode == 4 
 * mode == 5 - general look, all objects
 * mode == 6 - invisible items
 * mode == 7 - a container itself, tables will show illumination
 * mode == 15 - examine, shows extra information
 *
 ***********************************************************/
void
show_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, int mode)
{
	int found = 0;
	int crafts_found = 0;
	int first_seen;
	unsigned int i = 0;
	WRITING_DATA *writing = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj2 = NULL, *tobj = NULL;
	WOUND_DATA *wound = NULL;
	LODGED_OBJECT_INFO *lodged = NULL;
	AFFECTED_TYPE *af = NULL;
	CLAN_DATA *tclan = NULL;
	char *p = '\0';
	char output[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	int count;
	int item_cond; //add for REPAIR


	*buffer = '\0';

	if ((mode == 0) && obj->description)
	{

		if (IS_TABLE (obj))
			return;

		if (obj->nVirtual == VNUM_CORPSE)
			return;

		strcat (buffer, "#2");

		if (obj->omote_str)
		{
			sprintf (buf, "%s %s", obj->short_description, obj->omote_str);
			if (obj->count > 1)
			{
				sprintf (buf + strlen (buf), " (x%d)", obj->count);
			}
			if (buf[0] == '#')
			{
				buf[2] = toupper (buf[2]);
			}
			else
			{
				buf[0] = toupper (buf[0]);
			}
			strcat (buffer, buf);
		}
		else
		{
			strcat (buffer, obj_desc (obj));
		}

		if (get_obj_affect (obj, MAGIC_HIDDEN))
			strcat (buffer, " (hidden from view)");
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC))
			strcat (buffer, " #6(vNPC)#0");

		if (GET_ITEM_TYPE (obj) == ITEM_LIGHT &&
			obj->o.light.hours && obj->o.light.on)
			strcat (buffer, " #1(lit)#0");

		strcat (buffer, "#0");
		reformat_string (buffer, &p);
		sprintf (buffer, "%s", p);
		free_mem (p); // char*
		buffer[strlen (buffer) - 1] = '\0';
	}

	if (mode == 7 && obj->description)
	{
		strcpy (buffer, "#2");
		strcat (buffer, obj_desc (obj));
		strcat (buffer, "#0");
		if (IS_TABLE (obj))
		{
			for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
				if (obj2->obj_flags.type_flag == ITEM_LIGHT && obj2->o.light.hours && obj2->o.light.on)
				{
					strcat (buffer, " #1(Illuminated)#0");
					continue;
				}
		}

		reformat_string (buffer, &p);
		sprintf (buffer, "%s", p);
		free_mem (p); //char*
		buffer[strlen (buffer) - 1] = '\0';
	}

	else if (obj->short_description && ((mode == 1) ||
		(mode == 2) || (mode == 3)
		|| (mode == 4)))
	{

		strcpy (buffer, "#2");
		strcat (buffer, obj_short_desc (obj));
		strcat (buffer, "#0");

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_DESTROYED))
			strcat (buffer, " #1(destroyed)#0");

		if (GET_TRUST (ch) && (GET_ITEM_TYPE (obj) == ITEM_MONEY))
			sprintf (buffer + strlen (buffer), " (%d)", obj->count);
	}

	else if (mode == 5 || mode == 15)
	{

		if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
		{
			if (!obj->writing_loaded)
				load_writing (obj);
			if (obj->writing && obj->writing->message)
			{
				if (!IS_MORTAL (ch))
				{
					sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
						obj->writing->author, obj->writing->date);
					send_to_char (buf, ch);
				}
				if (obj->carried_by && obj->carried_by != ch && IS_MORTAL (ch))
				{
					send_to_char
						("You aren't quite close enough to make out the words.\n",
						ch);
					return;
				}
				reading_check (ch, obj, obj->writing, 0);
				return;
			}
			else
			{
				sprintf (buf, "%s#0 seems to be blank.",
					obj->short_description);
				sprintf (buffer, "#2%s\n", CAP (buf));
				return;
			}
		}

		else if (obj->obj_flags.type_flag == ITEM_BOOK)
		{
			if (!obj->writing_loaded)
				load_writing (obj);
			if (!obj->open)
			{
				if (obj->full_description && *obj->full_description)
					strcat (buffer, obj->full_description);
				else
					sprintf (buffer, "   It is #2%s#0.\n", OBJS (obj, ch));
				sprintf (buffer + strlen (buffer),
					"\n   #6This book seems to have %d pages remaining.#0\n",
					obj->o.od.value[0]);
				if (obj->book_title)
				{
					if (decipher_script
						(ch, obj->title_script, obj->title_language,
						obj->title_skill))
						sprintf (buf, "   #6The book has been entitled '%s.'#0\n",
						obj->book_title);
					else
						sprintf (buf,
						"   #6You cannot quite decipher what seems to be this book's title.#0\n");
					reformat_string (buf, &p);
					strcat (buffer, "\n");
					strcat (buffer, p);
					free_mem (p); //char*
				}
				page_string (ch->desc, buffer);
				return;
			}
			if (obj->carried_by && obj->carried_by != ch && IS_MORTAL (ch))
			{
				send_to_char
					("You aren't quite close enough to make out the words.\n",
					ch);
				return;
			}

			if (!obj->writing || !obj->o.od.value[0])
			{
				send_to_char ("All its pages have been torn out.\n", ch);
				return;
			}

			for (i = 2, writing = obj->writing; i <= obj->open; i++)
			{
				if (!writing->next_page)
					break;
				writing = writing->next_page;
			}

			if (writing && writing->message)
			{
				if (strcasecmp (writing->message, "blank") != STR_MATCH)
				{
					if (!IS_MORTAL (ch))
					{
						sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
							writing->author, writing->date);
						send_to_char (buf, ch);
					}
					reading_check (ch, NULL, writing, i);
					return;
				}
				else
				{
					sprintf (buf, "This page seems to be blank.");
					sprintf (buffer, "%s\n", CAP (buf));
					send_to_char (buffer, ch);
					return;
				}
			}
			else
			{
				sprintf (buf, "This page seems to be blank.");
				sprintf (buffer, "%s\n", CAP (buf));
				send_to_char (buffer, ch);
				return;
			}
		}

		else
		{
			if (obj->full_description && *obj->full_description)
			{
				if (obj->count > 1)
				{
					sprintf (buffer, "   It is #2%s#0. (x%d)\n", OBJS (obj, ch), obj->count);

				}
				else
				{
					sprintf (buffer, "   It is #2%s#0.\n", OBJS (obj, ch));
				}
				sprintf (buffer + strlen (buffer), "%s", obj->full_description);
			}
			else
			{
				if (obj->count > 1)
				{
					sprintf (buffer, "   It is #2%s#0. (x%d)\n", OBJS (obj, ch), obj->count);
				}
				else
				{
					sprintf (buffer, "   It is #2%s#0.\n", OBJS (obj, ch));
				}
			}

			if (mode == 15)
				sprintf (buffer + strlen (buffer), "%s",
				object__examine_damage (obj, ch)); //change for REPAIR

			if (IS_WEARABLE (obj) && obj->size && mode == 15)
			{
				sprintf (buffer + strlen (buffer),
					"\n   #6This garment would fit individuals wearing size %s.#0",
					sizes_named[obj->size]);
			}

			/* Show information about how many spaces at a table
			are taken or available */
			if (IS_TABLE (obj))
			{
				//get the number of people currently sat at the table
				count = 0;
				for (tch = ch->room->people; tch; tch = tch->next_in_room)
				{
					if ((af = get_affect (tch, MAGIC_SIT_TABLE)) &&
						af->a.table.obj == obj)
						count++;
				}

				//append approrpiate message
				if(obj->o.container.table_max_sitting == 0)
				{
					sprintf (buffer + strlen (buffer), "\n   #6There is plenty of space here.#0");
				}
				else if (obj->o.container.table_max_sitting == 1)
				{
					if(count==1)
					{
						sprintf (buffer + strlen (buffer), "\n   #6The single space here is taken.#0");
					}
					else
					{
						sprintf (buffer + strlen (buffer), "\n   #6The one space here is available.#0");
					}
				}
				else
				{
					sprintf (buffer + strlen (buffer),
						"\n   #6%d of the %d places here %s taken.#0",
						count,obj->o.container.table_max_sitting,count==1 ? "is" : "are");
				}
			}

			if (GET_ITEM_TYPE (obj) == ITEM_TOSSABLE && mode == 15
				&& obj->desc_keys && strlen (obj->desc_keys))
			{

				strcat (buffer,
					"\n   #6Its sides are marked as follows:#0\n      #6");

				for (i = 0; i < strlen (obj->desc_keys); i++)
				{
					if (obj->desc_keys[i] == ' ')
					{
						strcat (buffer, ", ");
					}
					else
					{
						buffer[strlen (buffer) + 1] = '\0';
						buffer[strlen (buffer)] = obj->desc_keys[i];
					}
				}

				strcat (buffer, "#0\n");

			}

			if (GET_ITEM_TYPE (obj) == ITEM_WEAPON && mode == 15)
			{
				sprintf (buffer + strlen (buffer),
					"\n   #6You'd guess it to be a %s weapon.#0\n",
					skills[obj->o.weapon.use_skill]);

				if (IS_SET (obj->obj_flags.extra_flags, ITEM_THROWING)
					&& mode == 15)
					sprintf (buffer + strlen (buffer),
					"\n   #6This weapon appears ideally suited for throwing.#0\n");
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_DWELLING && mode == 15)
			{
				sprintf (buffer + strlen (buffer),
					"\n   #6You'd guess this dwelling could shelter %d individual%s.#0\n",
					obj->o.od.value[1],
					obj->o.od.value[1] != 1 ? "s" : "");
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_HEALER_KIT && mode == 15)
			{
				if (ch->skills[SKILL_HEALING]
				&& ch->skills[SKILL_HEALING] >= obj->o.od.value[2])
				{
					sprintf (buffer + strlen (buffer),
						"\n   #6Used to Treat:#0 ");
					if (IS_SET (obj->o.od.value[5], TREAT_ALL)
						|| !obj->o.od.value[5])
						sprintf (buffer + strlen (buffer), " Any");
					else
					{
						if (IS_SET (obj->o.od.value[5], TREAT_SLASH))
							sprintf (buffer + strlen (buffer), " Slashes");
						if (IS_SET (obj->o.od.value[5], TREAT_PUNCTURE))
							sprintf (buffer + strlen (buffer), " Punctures");
						if (IS_SET (obj->o.od.value[5], TREAT_BLUNT))
							sprintf (buffer + strlen (buffer), " Contusions");
						if (IS_SET (obj->o.od.value[5], TREAT_BURN))
							sprintf (buffer + strlen (buffer), " Burns");
						if (IS_SET (obj->o.od.value[5], TREAT_FROST))
							sprintf (buffer + strlen (buffer), " Frostbite");
						if (IS_SET (obj->o.od.value[5], TREAT_BLEED))
							sprintf (buffer + strlen (buffer), " Bleeding");
					}
					sprintf (buffer + strlen (buffer),
						"\n   #6Uses Remaining:#0 %d\n",
						obj->o.od.value[0]);
				}
				else
				{
					sprintf (buffer + strlen (buffer),
						"\n   #6Your training is not yet sufficient for this remedy.#0\n");
				}
			}
			else if (GET_ITEM_TYPE (obj) == ITEM_REPAIR_KIT && mode == 15)
			{
				if (obj->o.od.value[3] < 0 && obj->o.od.value[5] < 0)
				{
					sprintf (buffer + strlen (buffer),
						" #1Error, This kit cannot repair anything!#0");
				}
				else if (obj->o.od.value[3]
				&& (!ch->skills[(obj->o.od.value[3])]
				|| ch->skills[(obj->o.od.value[3])] <
					obj->o.od.value[2]))
				{
					sprintf (buffer + strlen (buffer),
						"\n   #6Your training is not yet sufficient to use these materials.#0\n");
				}
				else
				{
					sprintf (buffer + strlen (buffer),
						"\n   #6Used to Repair:#0 ");
					if (obj->o.od.value[5] == 0)
					{
						sprintf (buffer + strlen (buffer),
							" #6Any item#0 made with ");
					}
					else
					{
						sprintf (buffer + strlen (buffer),
								 " #6%s #0items made with ",
							item_types[(obj->o.od.value[5])]);
					}
					if (obj->o.od.value[3] == 0)
					{
						sprintf (buffer + strlen (buffer), "#6Any skill#0.");
					}
					else
					{
						sprintf (buffer + strlen (buffer), "the #6%s skill#0.",
							skills[(obj->o.od.value[3])]);
					}
					
					if (obj->o.od.value[4] > 0)
					{
						sprintf (buffer + strlen (buffer),
								 "\n   #6Maximum Damage that can be repaired:#0  %s\n",damage_severity[obj->o.od.value[4]]);
					}
					
					if (obj->o.od.value[0] < 0)
					{
						sprintf (buffer + strlen (buffer),
								 "\n   #6Uses Remaining:#0  Unlimited\n");
					}
					else 
					{
					sprintf (buffer + strlen (buffer),
						"\n   #6Uses Remaining:#0  %d\n",
						obj->o.od.value[0]);
				}
					
					
					
				}
			}

			if (obj->clan_data)
			{
				if (is_clan_member(ch, obj->clan_data->name))
				{
					tclan = get_clandef(obj->clan_data->name);
					sprintf (buf, "   You see the mark of %s.\n",
						tclan->literal);

					reformat_string (buf, &p);
					strcat (buffer, "\n");
					strcat (buffer, p);
					free_mem (p); //char*
					p = 0;
				}
			}

			*buf = '\0';

			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			{
				if (!(af = get_affect (ch, i)))
					continue;
				if (!af->a.craft || !af->a.craft->subcraft)
					continue;
				if (!craft_uses (af->a.craft->subcraft, obj->nVirtual))
					continue;
				if (crafts_found)
					sprintf (buf + strlen (buf), ", ");
				sprintf (buf + strlen (buf), "'%s %s'",
					af->a.craft->subcraft->command,
					af->a.craft->subcraft->subcraft_name);
				crafts_found += 1;
			}

			SUBCRAFT_HEAD_DATA *tcraft;
			if (!IS_MORTAL (ch))
			{
				for (tcraft = crafts; tcraft; tcraft = tcraft->next)
				{
					if (!craft_uses (tcraft, obj->nVirtual))
						continue;
					if (crafts_found)
						sprintf (buf + strlen (buf), ", ");
					sprintf (buf + strlen (buf), "'%s %s'",
						tcraft->command,
						tcraft->subcraft_name);
					crafts_found += 1;
				}
			}


			if (crafts_found && mode == 15)
			{
				sprintf (output,
					"   You realize that you could make use of this item in the following craft%s: %s.",
					crafts_found != 1 ? "s" : "", buf);
				reformat_string (output, &p);
				sprintf (buffer + strlen (buffer), "\n%s", p);
				free_mem (p); // char*;
				p = 0;
			}

			if (obj->wounds)
			{
				sprintf (buf, "   It has ");
				for (wound = obj->wounds; wound; wound = wound->next)
				{
					if (!wound->next && wound != obj->wounds)
						sprintf (buf2, "and a %s %s on the %s", wound->severity,
						wound->name, expand_wound_loc (wound->location));
					else
						sprintf (buf2, "a %s %s on the %s", wound->severity,
						wound->name, expand_wound_loc (wound->location));
					strcat (buf, buf2);
					if (!wound->next)
						strcat (buf, ".");
					else
						strcat (buf, ", ");
				}
				reformat_string (buf, &p);
				strcat (buffer, "\n");
				strcat (buffer, p);
				free_mem (p); //char*
				p = 0;
			}

			
			if (obj->lodged)
			{
				sprintf (buf, "   It has ");
				for (lodged = obj->lodged; lodged; lodged = lodged->next)
				{
					tobj = load_object (lodged->vnum);
					if (!tobj)
						continue;
					if (!lodged->next && lodged != obj->lodged)
						sprintf (buf2, "and #2%s#0 lodged in the %s",
						tobj->short_description,
						expand_wound_loc (lodged->location));
					else
						sprintf (buf2, "#2%s#0 lodged in the %s",
						tobj->short_description,
						expand_wound_loc (lodged->location));
					strcat (buf, buf2);
					if (!lodged->next)
						strcat (buf, ".");
					else
						strcat (buf, ", ");
					extract_obj (tobj);
				}
				reformat_string (buf, &p);
				if (obj->wounds)
					strcat (buffer, "\n");
				strcat (buffer, p);
				free_mem (p); //char*
				p = NULL;
			}

			first_seen = 1;

			if (IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
			{
				for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
				{

					if (CAN_SEE_OBJ (ch, obj2))
					{

						if (first_seen)
							sprintf (buffer + strlen (buffer),
							"\nOn #2%s#0 you see:\n", OBJS (obj, ch));

						first_seen = 0;


						sprintf (buffer + strlen (buffer), "   %s",

							OBJS (obj2, ch));

						if (GET_ITEM_TYPE (obj2) == ITEM_LIGHT &&
							obj2->o.light.hours && obj2->o.light.on)
							strcat (buffer + strlen (buffer), " #1(lit)#0\n");
						else
							strcat (buffer + strlen (buffer), "\n");


					}
				}

				for (tch = ch->room->people; tch; tch = tch->next_in_room)
				{
				}
			}
			
			if (obj->nVirtual != VNUM_CORPSE)
			{			 //changed for REPAIR
			if (obj->item_wear == 100 )
				item_cond = 6;
			else if (obj->item_wear < 100 && obj->item_wear >= 90)
				item_cond = 5;
			else if (obj->item_wear < 90 && obj->item_wear >= 70)
				item_cond = 4;
			else if (obj->item_wear < 70 && obj->item_wear >= 50)
				item_cond = 3;	
			else if (obj->item_wear < 50 && obj->item_wear >= 20)
				item_cond = 2;
			else if (obj->item_wear < 20 && obj->item_wear >= 0)
				item_cond = 1;
			else if (obj->item_wear < 0)
				item_cond = 0;
//actual displays commented out until bugs can be fixed	
			//if (item_cond == 6)
			//{
				sprintf (buffer + strlen(buffer), "\n   It appears to be in flawless condition.\n");
			//}
/**
			else 
			{	
				
				switch GET_MATERIAL_TYPE(obj)
				{
				case (1 << 1): // TEXTILE
					sprintf (buffer + strlen(buffer), "\n   %s\n", fabric_conditions[item_cond]);
					break;
					
				case (1 << 2): // LEATHER
					sprintf (buffer + strlen(buffer), "\n   %s\n", leather_conditions[item_cond]);
					break;
					
				case (1 << 3): // WOOD
					sprintf (buffer + strlen(buffer), "\n   %s\n", wood_conditions[item_cond]);
					break;
					
				case (1 << 4): // METAL
					sprintf (buffer + strlen(buffer), "\n   %s\n", unspecified_conditions[item_cond]);
					break;
					
				case (1 << 5): // STONE
				case (1 << 6): // GLASS
				case (1 << 11): // OTHER
					sprintf (buffer + strlen(buffer), "\n   %s\n", unspecified_conditions[item_cond]);
					break;
					
				default:
					sprintf (buffer + strlen(buffer), "\n   %s\n", unspecified_conditions[item_cond]);
					break;
				}
				
				
			}
*******/			
	
				
			}
		}
	}

	else if (mode == 6)
	{

		strcpy (buffer, "   ");

		if (CAN_SEE_OBJ (ch, obj) && IS_OBJ_STAT (obj, ITEM_INVISIBLE))
			strcat (buffer, "(invis) ");

		sprintf (buffer + strlen (buffer), "#2%s#0", obj_short_desc (obj));

		if (GET_TRUST (ch) && (GET_ITEM_TYPE (obj) == ITEM_MONEY))
			sprintf (buffer + strlen (buffer), " (%d)", obj->count);
	}

	if (mode != 3)
	{
		found = false;
		if (IS_OBJ_STAT (obj, ITEM_INVISIBLE))
		{
			strcat (buffer, "(invisible)");
			found = true;
		}
	}

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_LIGHT)
	{
		if (obj->o.light.hours && obj->o.light.on)
			sprintf (buffer + strlen (buffer), " #1(lit)#0");
		else if (obj->o.light.hours && !obj->o.light.on)
			sprintf (buffer + strlen (buffer), " #1(unlit)#0");
		else
			sprintf (buffer + strlen (buffer), " #1(spent)#0");
	}

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title
		&& decipher_script (ch, obj->title_script, obj->title_language,
		obj->title_skill))
		sprintf (buffer, "#2\"%s\"#0", obj->book_title);
	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->open)
		sprintf (buffer + strlen (buffer), " #1(open)#0");
	else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && !obj->open)
		sprintf (buffer + strlen (buffer), " #1(closed)#0");

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_WEAPON
		&& (obj->contains || obj->loaded))
		sprintf (buffer + strlen (buffer), " #6(loaded)#0");

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_SHEATH && obj->contains)
		sprintf (buffer + strlen (buffer), " bearing %s#2%s#0",
		article (OBJN (obj->contains, ch)), OBJN (obj->contains, ch));

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_QUIVER && obj->contains)
		sprintf (buffer + strlen (buffer), " bearing #2%ss#0",
		OBJN (obj->contains, ch));

	if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
		obj->o.od.value[0] == obj->o.od.value[1])
		sprintf (buffer + strlen (buffer), " #1(full)#0");
	else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
		obj->o.od.value[0] > obj->o.od.value[1] / 2)
		sprintf (buffer + strlen (buffer), " #1(mostly full)#0");
	else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
		obj->o.od.value[0] == obj->o.od.value[1] / 2)
		sprintf (buffer + strlen (buffer), " #1(half full)#0");
	else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
		obj->o.od.value[0] < obj->o.od.value[1] / 2
		&& obj->o.od.value[0] > 0)
		sprintf (buffer + strlen (buffer), " #1(mostly empty)#0");
	else if (mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
		obj->o.od.value[0] <= 0)
		sprintf (buffer + strlen (buffer), " #1(empty)#0");

	if (buffer[strlen (buffer) - 1] != '\n')
		strcat (buffer, "\n");

	page_string (ch->desc, buffer);

}

void
list_obj_to_char (OBJ_DATA * list, CHAR_DATA * ch, int mode, int show)
{
	OBJ_DATA *i = NULL;
	int found = 0, j = 0;
	int looked_for_tables = 0;
	int looked_for_corpses = 0;
	OBJ_DATA *obj = NULL;
	bool clump = false;

	found = false;

	if (!list)
	{
		if (show)
			send_to_char ("Nothing.\n", ch);
		return;
	}

	for (i = list; i; i = i->next_content)
	{
		j++;
	}

	if (j >= 25 && show != 4 && !list->in_obj)
		clump = true;

	for (i = list; i; i = i->next_content)
	{
		if (CAN_SEE_OBJ (ch, i))
		{

			if (!mode && !looked_for_tables && IS_TABLE (i))
			{

				for (obj = i; obj; obj = obj->next_content)
					if (IS_TABLE (obj))
						looked_for_tables++;

				//show first four tables otherwise group them
				if (looked_for_tables < 5)
				{
					for (obj = i; obj; obj = obj->next_content)
						if (IS_TABLE (obj))
							show_obj_to_char (obj, ch, 7);
				}
				else
				{
					send_to_char ("#6There are several furnishings here.#0\n",
						ch);
				}
			}

			if (!mode && !looked_for_corpses && i->nVirtual == VNUM_CORPSE)
			{

				for (obj = i; obj; obj = obj->next_content)
					if (obj->nVirtual == VNUM_CORPSE)
						looked_for_corpses++;

				if (looked_for_corpses == 1)
					show_obj_to_char (i, ch, 7);
				else if (looked_for_corpses == 2)
					send_to_char ("#6There are a couple corpses here.#0\n", ch);
				else if (looked_for_corpses < 10)
					send_to_char ("#6There are several corpses here.#0\n", ch);
				else
					send_to_char ("#6There are a number of corpses strewn about the area.#0\n", ch);
			}


			if (clump)
				continue;

			show_obj_to_char (i, ch, mode);

			found = true;
		}
	}

	if (clump)
	{
		if (j >= 25 && j < 35)
			send_to_char ("#2The area is strewn with a number of objects.#0\n",
			ch);
		else if (j >= 35 && j < 45)
			send_to_char
			("#2The area is strewn with a sizeable number of objects.#0\n", ch);
		else if (j >= 45 && j < 55)
			send_to_char
			("#2The area is strewn with a large number of objects.#0\n", ch);
		else if (j >= 55 && j < 65)
			send_to_char
			("#2The area is strewn with a great number of objects.#0\n", ch);
		else
			send_to_char
			("#2The area is strewn with a staggering number of objects.#0\n",
			ch);
	}
	else if (!found && show && show != 4)
		send_to_char ("Nothing.\n", ch);
}

void
list_char_to_char (CHAR_DATA * list, CHAR_DATA * ch, int mode)
{
	CHAR_DATA *i = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int count = 0, j = 0;


	for (i = list; i; i = i->next_in_room)
	{
		if (i == ch)
			continue;
		j++;
	}

	if (j < 25 || mode == 4 || ch->room->nVirtual == AMPITHEATRE)
	{
		for (i = list; i; i = i->next_in_room)
		{
			if (ch != i && ch->vehicle != i)
			{
				if (ch->room->nVirtual == AMPITHEATRE && IS_MORTAL (i))
				{
					count++;
					continue;
				}
				show_char_to_char (i, ch, mode);
			}
		}
	}
	else
	{
		if (j >= 25 && j < 35)
			send_to_char ("#5The area is filled by a crowd of individuals.\n#0",
			ch);
		if (j >= 35 && j < 45)
			send_to_char
			("#5The area is filled by a decently-sized crowd of individuals.\n#0",
			ch);
		else if (j >= 45 && j < 55)
			send_to_char
			("#5The area is filled by a sizeable crowd of individuals.\n#0",
			ch);
		else if (j >= 55 && j < 65)
			send_to_char
			("#5The area is filled by a large crowd of individuals.\n#0", ch);
		else if (j > 65)
			send_to_char
			("#5The area is filled by an immense crowd of individuals.\n#0",
			ch);
	}

	if (count)
	{
		sprintf (buf,
			"#5There %s %d other %s assembled here for the meeting.#0\n",
			count != 1 ? "are" : "is", count,
			count != 1 ? "people" : "person");
		send_to_char (buf, ch);
	}
}

void
show_contents (CHAR_DATA * ch, char *argument, int cmd)
{
	if (cmd == 1)
	{
		if (!ch->room->contents)
		{
			send_to_char ("\n   None.\n", ch);
			return;
		}

		if (ch->room->contents)
		{
			send_to_char ("\n", ch);
			list_obj_to_char (ch->room->contents, ch, 0, 4);
		}
		return;
	}
	else if (cmd == 2)
	{
		if ((!ch->room->people || !ch->room->people->next_in_room))
		{
			send_to_char ("\n   None.\n", ch);
			return;
		}

		if (ch->room->people
			&& (ch->room->people->next_in_room || ch->room->people != ch))
		{
			send_to_char ("\n", ch);
			list_char_to_char (ch->room->people, ch, 4);
		}
		return;
	}
}

void
do_contents (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!ch->room->contents
		&& (!ch->room->people || !ch->room->people->next_in_room))
	{
		send_to_char ("   None.\n", ch);
		return;
	}

	if (ch->room->contents)
	{
		send_to_char ("\n", ch);
		list_obj_to_char (ch->room->contents, ch, 0, 4);
	}

	if (ch->room->people
		&& (ch->room->people->next_in_room || ch->room->people != ch))
	{
		send_to_char ("\n", ch);
		list_char_to_char (ch->room->people, ch, 4);
	}
}

int
enter_exit_msg (CHAR_DATA * ch, char *buffer)
{
	QE_DATA *qe = NULL;
	char *addon = '\0';
	bool isLeaving = false;
	char *e_dirs[] = {
		"to the north",
		"to the east",
		"to the south",
		"to the west",
		"up",
		"down",
		"to the outside",
		"to the inside"
	};
	char *a_dirs[] = {
		"the south",
		"the west",
		"the north",
		"the east",
		"below",
		"above",
		"the outside",
		"the inside"
	};

	extern QE_DATA *quarter_event_list;

	for (qe = quarter_event_list; qe; qe = qe->next)
		if (qe->ch == ch)
			break;

	if (!qe)
		return 0;

	addon = buffer + strlen (buffer);

	if (GET_FLAG (ch, FLAG_LEAVING))
	{
		isLeaving = true;
	}
	else if (GET_FLAG (ch, FLAG_ENTERING))
	{
		isLeaving = false;
	}
	else
		return 0;


	char* charShort = duplicateString(char_short(ch));

	sprintf (addon, "#3%s is %s %s%s%s.#0",
		CAP (charShort),
		(isLeaving ? "leaving" : "arriving from"),
		(isLeaving ? e_dirs[qe->dir] : a_dirs[qe->dir]),
		((qe->travel_str == NULL && ch->travel_str) ? ", " : ""),
		((qe->travel_str ? qe->
travel_str : (ch->travel_str ? ch->travel_str : ""))));
	addon[2] = toupper (addon[2]);

	free_mem(charShort);

	return 1;
}

char *
height_phrase (CHAR_DATA * ch)
{
	static char phrase[MAX_STRING_LENGTH];

	sprintf (phrase, "gigantic");

	if (ch->height < 96)
		sprintf (phrase, "towering");
	if (ch->height < 78)
		sprintf (phrase, "very tall");
	if (ch->height < 75)
		sprintf (phrase, "tall");
	if (ch->height < 71)
		sprintf (phrase, "%s", frame_built[ch->frame]);
	if (ch->height < 60)
		sprintf (phrase, "short");
	if (ch->height < 48)
		sprintf (phrase, "very short");
	if (ch->height < 36)
		sprintf (phrase, "extremely short");
	if (ch->height < 24)
		sprintf (phrase, "tiny");

	return phrase;
}

// A or An needs to be based on the phrase, not the height
char *
char_short (CHAR_DATA * ch)
{
	OBJ_DATA *obj = NULL;
	static char buf[MAX_STRING_LENGTH] = { '\0' };
	char phrase[MAX_STRING_LENGTH] = { '\0' };

	if (!ch)
		return NULL;

	if (!ch->short_descr && !ch->pc && !ch->mob)
	{
		system_log
			("AVOIDED CRASH BUG:  char_short: (!ch->short_descr && !ch->pc && !ch->mob)",
			true);
		return NULL;
	}

	sprintf (phrase, "%s", height_phrase (ch));

	if ((obj = get_equip (ch, WEAR_NECK_1)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
	{
		sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
			phrase, obj->desc_keys);
		return buf;
	}

	if ((obj = get_equip (ch, WEAR_NECK_2)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
	{
		sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
			phrase, obj->desc_keys);
		return buf;
	}

	if ((obj = get_equip (ch, WEAR_ABOUT)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
	{
		sprintf (buf, "a%s %s, %s person", (isvowel (phrase[0]) ? "n" : ""),
			phrase, obj->desc_keys);
		return buf;
	}

	if (((obj = get_equip (ch, WEAR_HEAD))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
		|| ((obj = get_equip (ch, WEAR_FACE))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{

		if (obj->obj_flags.type_flag == ITEM_WORN)
			sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
		else
			sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
		return buf;

	}

	if ((obj = get_equip (ch, WEAR_FACE))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
	{

		if (obj->obj_flags.type_flag == ITEM_WORN)
			sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
		else
			sprintf (buf, "the %s, %s person", phrase, obj->desc_keys);
		return buf;

	}

	if (!ch->short_descr)
		return "a strangely amorphous person";
	return ch->short_descr;
}

char *
char_long (CHAR_DATA * ch, int show_tname)
{
	OBJ_DATA *obj = NULL;
	static char buf[MAX_STRING_LENGTH] = { '\0' };
	char phrase[20] = { '\0' };
	char tname[MAX_STRING_LENGTH] = { '\0' };

	sprintf (phrase, "%s", height_phrase (ch));

	if (show_tname && is_hooded (ch))
		sprintf (tname, "(%s) ", ch->tname);
	else
		*tname = '\0';

	if ((obj = get_equip (ch, WEAR_NECK_1))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (ch->affected_by, AFF_HOODED))
	{
		if (ch->pmote_str)
		{
			if (*(ch->pmote_str) == '\'')
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
			else
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
		}
		else
		{
			sprintf (buf, "A %s, %s person %sis here.", phrase,
				obj->desc_keys, tname);
			return buf;
		}
	}
	if ((obj = get_equip (ch, WEAR_NECK_1))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (ch->affected_by, AFF_HOODED))
	{
		if (ch->pmote_str)
		{
			if (*(ch->pmote_str) == '\'')
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
			else
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
		}
		else
		{
			sprintf (buf, "A %s, %s person %sis here.", phrase,
				obj->desc_keys, tname);
			return buf;
		}
	}
	if ((obj = get_equip (ch, WEAR_ABOUT))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (ch->affected_by, AFF_HOODED))
	{
		if (ch->pmote_str)
		{
			if (*(ch->pmote_str) == '\'')
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
			else
			{
				sprintf (buf, "A %s, %s person%s %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				return buf;
			}
		}
		else
		{
			sprintf (buf, "A %s, %s person %sis here.", phrase,
				obj->desc_keys, tname);
			return buf;
		}
	}

	if (((obj = get_equip (ch, WEAR_HEAD))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
		|| ((obj = get_equip (ch, WEAR_FACE))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{

		if (obj->obj_flags.type_flag == ITEM_WORN)
			if (ch->pmote_str)
			{
				if (*(ch->pmote_str) == '\'')
					sprintf (buf, "A %s, %s %sperson%s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				else
					sprintf (buf, "A %s, %s %sperson %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
			}
			else
				sprintf (buf, "A %s, %s %sperson is here.", phrase,
				obj->desc_keys, tname);
		else
		{
			if (ch->pmote_str)
			{
				if (*(ch->pmote_str) == '\'')
					sprintf (buf, "A %s, %s %sperson%s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
				else
					sprintf (buf, "A %s, %s %sperson %s", phrase,
					obj->desc_keys, tname, ch->pmote_str);
			}
			else
				sprintf (buf, "A %s, %s %sperson is here.", phrase,
				obj->desc_keys, tname);
		}
		return buf;
	}

	if (ch->pmote_str)
	{
		if (*(ch->pmote_str) == '\'')
			sprintf (buf, "%s%s", ch->short_descr, ch->pmote_str);
		else
			sprintf (buf, "%s %s", ch->short_descr, ch->pmote_str);
		buf[0] = (('a' <= buf[0]) && (buf[0] <= 'z'))
			? (char) (buf[0] - 32) : buf[0];
		buf[0] = toupper (buf[0]);
		return buf;
	}
	else
		return ch->long_descr;
}


char *
left_right_grip (CHAR_DATA * ch)
{
	bool right = false, left = false;

	if (get_equip (ch, WEAR_PRIM))
		right = true;
	if (get_equip (ch, WEAR_SEC))
		left = true;
	if (get_equip (ch, WEAR_SHIELD))
		left = true;
	if (get_equip (ch, WEAR_BOTH))
	{
		right = true;
		left = true;
	}
	if (get_equip (ch, WEAR_CARRY_L))
		left = true;
	if (get_equip (ch, WEAR_CARRY_R))
		right = true;

	if ((!right && left) || (!right && !left))
		return "right";
	if (right && !left)
		return "left";
	else
		system_log ("Error in left_right_grip, act.informative.c", true);

	return NULL;
}

char *
worn_third_loc (CHAR_DATA * ch, OBJ_DATA * obj)
{
	static char buf[MAX_STRING_LENGTH] = { '\0' };

	if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
		sprintf (buf, "around %s neck", HSHR (ch));
	else if (obj->location == WEAR_BODY)
		sprintf (buf, "on %s body", HSHR (ch));
	else if (obj->location == WEAR_HEAD)
		sprintf (buf, "on %s head", HSHR (ch));
	else if (obj->location == WEAR_ARMS)
		sprintf (buf, "on %s arms", HSHR (ch));
	else if (obj->location == WEAR_ABOUT)
		sprintf (buf, "about %s body", HSHR (ch));
	else if (obj->location == WEAR_WAIST)
		sprintf (buf, "around %s waist", HSHR (ch));
	else if (obj->location == WEAR_WRIST_L)
		sprintf (buf, "on %s left wrist", HSHR (ch));
	else if (obj->location == WEAR_WRIST_R)
		sprintf (buf, "on %s right wrist", HSHR (ch));
	else if (obj->location == WEAR_HAIR)
		sprintf (buf, "in %s hair", HSHR (ch));
	else if (obj->location == WEAR_FACE)
		sprintf (buf, "on %s face", HSHR (ch));
	else if (obj->location == WEAR_ANKLE_L)
		sprintf (buf, "on %s left ankle", HSHR (ch));
	else if (obj->location == WEAR_ANKLE_R)
		sprintf (buf, "on %s right ankle", HSHR (ch));
	else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
		sprintf (buf, "on %s belt", HSHR (ch));
	else if (obj->location == WEAR_BACK)
		sprintf (buf, "on %s back", HSHR (ch));
	else if (obj->location == WEAR_THROAT)
		sprintf (buf, "around %s throat", HSHR (ch));
	else if (obj->location == WEAR_BLINDFOLD)
		sprintf (buf, "as a blindfold");
	else if (obj->location == WEAR_EAR)
		sprintf (buf, "on %s ear", HSHR (ch));
	else if (obj->location == WEAR_SHOULDER_R)
		sprintf (buf, "over %s right shoulder", HSHR (ch));
	else if (obj->location == WEAR_SHOULDER_L)
		sprintf (buf, "over %s left shoulder", HSHR (ch));
	else if (obj->location == WEAR_FEET)
		sprintf (buf, "on %s feet", HSHR (ch));
	else if (obj->location == WEAR_FINGER_R)
		sprintf (buf, "on %s right ring finger", HSHR (ch));
	else if (obj->location == WEAR_FINGER_L)
		sprintf (buf, "on %s left ring finger", HSHR (ch));
	else if (obj->location == WEAR_ARMBAND_R)
		sprintf (buf, "around %s upper right arm.", HSHR (ch));
	else if (obj->location == WEAR_ARMBAND_L)
		sprintf (buf, "around %s upper left arm.", HSHR (ch));
	else if (obj->location == WEAR_LEGS)
		sprintf (buf, "on %s legs", HSHR (ch));
	else if (obj->location == WEAR_HANDS)
		sprintf (buf, "on %s hands", HSHR (ch));
	else
		sprintf (buf, "#1in an uknown location#0");

	return buf;
}

void
show_char_to_char (CHAR_DATA * i, CHAR_DATA * ch, int mode)
{
	int percent = 0;
	int curdamage = 0;
	int location = 0;
	OBJ_DATA *eq = NULL;
	OBJ_DATA *blindfold = NULL;
	OBJ_DATA *tobj = NULL;
	AFFECTED_TYPE *af = NULL;
	ROOM_DATA *troom = NULL;
	WOUND_DATA *wound = NULL;
	ENCHANTMENT_DATA *enchantment = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	char *dir_names[] = {
		"to the north", "to the east", "to the south",
		"to the west", "above", "below"
	};
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	bool found = false;

	if (mode == 4)
		mode = 0;

	if (!mode)
	{

		if (IS_SUBDUEE (i))
		{
			if (i->subdue == ch)
				act ("You have #5$N#0 in tow.", false, ch, 0, i, TO_CHAR);
			return;
		}

		if (!CAN_SEE (ch, i))
		{
			if (!GET_FLAG (i, FLAG_WIZINVIS) &&
				get_affect (ch, MAGIC_AFFECT_SENSE_LIFE))
				send_to_char ("\nYou sense a hidden life form in the room.\n\n",
				ch);
			return;
		}

		if ((blindfold = get_equip (i, WEAR_BLINDFOLD)) ||
			!char_long (i, 0) || GET_POS (i) != i->default_pos ||
			IS_SWIMMING (i))
		{
			if (char_short (i))
			{
				strcpy (buffer, "#5");
				strcat (buffer, char_short (i));
				strcat (buffer, "#0");
				buffer[2] = toupper (buffer[2]);

			}
			else
				strcpy (buffer, "#5A nameless one#0");

			if (IS_SWIMMING (i))
			{
				if ( IS_SET(i->act, ACT_FLYING ))
					strcat(buffer, " is here, flying.");
				else
					strcat (buffer, " is here, swimming.");
			}
			else
				switch (GET_POS (i))
			{
				case POSITION_STUNNED:
					if (blindfold)
						strcat (buffer, " is here, stunned and blindfolded.");
					else
						strcat (buffer, " is here, stunned.");
					break;
				case POSITION_UNCONSCIOUS:
					if (blindfold)
						strcat (buffer,
						" is lying here, unconscious and blindfolded.");
					else
						strcat (buffer, " is lying here, unconscious.");
					break;
				case POSITION_MORTALLYW:
					if (blindfold)
						strcat (buffer,
						" is lying here, mortally wounded and blindfolded.");
					else
						strcat (buffer, " is lying here, mortally wounded.");
					break;
				case POSITION_DEAD:
					strcat (buffer, " is lying here, dead.");
					break;
				case POSITION_STANDING:
					if (blindfold)
						strcat (buffer, " is standing here, blindfolded.");
					else
						strcat (buffer, " is standing here.");
					break;
				case POSITION_SITTING:
					if (blindfold)
						strcat (buffer, " is sitting here, blindfolded");
					if (i->pmote_str)
					{
						sprintf (buffer, "#5");
						strcat (buffer, char_long (i, GET_TRUST (ch)));
						strcat (buffer, "#0");
						break;
					}
					else
						strcat (buffer, " is sitting here");

					if ((af = get_affect (i, MAGIC_SIT_TABLE)) &&
						is_obj_in_list (af->a.table.obj, i->room->contents))
					{
						tobj = af->a.table.obj;
						sprintf (buffer + strlen (buffer), " at #2%s#0.",
							OBJS (tobj, i));
					}
					else
						strcat (buffer, ".");
					break;
				case POSITION_RESTING:
					if (blindfold)
						strcat (buffer, " is resting here, blindfolded");
					else
					{
						if (i->pmote_str)
						{
							sprintf (buffer, "#5");
							strcat (buffer, char_long (i, GET_TRUST (ch)));
							strcat (buffer, "#0");
							break;
						}
						else
							strcat (buffer, " is resting here");
					}
					if ((af = get_affect (i, MAGIC_SIT_TABLE)) &&
						is_obj_in_list (af->a.table.obj, i->room->contents))
					{
						tobj = af->a.table.obj;
						sprintf (buffer + strlen (buffer), " at #2%s#0.",
							OBJS (tobj, i));
					}
					else
						strcat (buffer, ".");
					break;
				case POSITION_SLEEPING:
					if (blindfold)
						strcat (buffer, " is sleeping here, blindfolded");
					else
						strcat (buffer, " is sleeping here");

					if ((af = get_affect (i, MAGIC_SIT_TABLE)) &&
						is_obj_in_list ((OBJ_DATA *) af->a.spell.t,
						i->room->contents))
					{
						tobj = (OBJ_DATA *) af->a.spell.t;
						sprintf (buffer + strlen (buffer), " at #2%s#0.",
							OBJS (tobj, i));
					}
					else
						strcat (buffer, ".");

					break;
				case POSITION_FIGHTING:
					if (i->fighting)
					{

						if (IS_SET (i->flags, FLAG_SUBDUING))
						{
							if (blindfold)
								strcat (buffer,
								" is here, #1wrestling blindfolded#0 against ");
							else
								strcat (buffer, " is here, #1wrestling#0 ");
						}
						else
						{
							if (blindfold)
								strcat (buffer,
								" is here, #1fighting blindfolded#0 against ");
							else
								strcat (buffer, " is here, #1fighting#0 ");
						}

						if (i->fighting == ch)
							strcat (buffer, " you!");
						else
						{
							if (i->in_room == i->fighting->in_room)
							{
								strcat (buffer, char_short (i->fighting));
								strcat (buffer, ".");
							}
							else
								strcat (buffer, "someone who has already left.");
						}
					}
					else		/* NIL fighting pointer */
						strcat (buffer, " is here struggling with thin air.");
					break;
				default:
					strcat (buffer, " is floating here.");
					break;
			}

			if (GET_TRUST(ch)) {
				std::stringstream health;health << " #0<" << wound_total(i,true) << "> ";
				strcat (buffer, health.str().c_str());
			}

			if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
				strcat (buffer, " #2(new player)#0");

			if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
				strcat (buffer, " #B(new player guide)#0");

			if (i->desc && i->desc->idle)
				strcat (buffer, " #1(idle)#0");

			if ((GET_TRUST (i) || get_affect (ch, MAGIC_AFFECT_SENSE_LIFE))
				&& get_affect (i, MAGIC_HIDDEN))
				strcat (buffer, " #1(hidden)#0");

			if (are_grouped (i, ch))
				strcat (buffer, " #6(grouped)#0");

			if (GET_FLAG (i, FLAG_WIZINVIS))
				strcat (buffer, " #C(wizinvis)#0");

			if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
				strcat (buffer, " #1(invisible)#0");

			if (get_affect (i, MAGIC_AFFECT_CONCEALMENT) &&
				(i == ch || GET_TRUST (i) ||
				get_affect (ch, MAGIC_AFFECT_SENSE_LIFE)))
				strcat (buffer, " #1(blend)#0");

			if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
				strcat (buffer, " #1(editing)#0");

			if (!IS_NPC (i) && !i->desc && !i->pc->admin_loaded)
				strcat (buffer, " #1(link dead)#0");

			if (!IS_MORTAL (ch) && i->pc && !i->desc && i->pc->admin_loaded)
				strcat (buffer, " #3(loaded)#0");

			if (i->desc && i->desc->original && !IS_MORTAL (ch))
				strcat (buffer, " #2(animated)#0");
			strcat (buffer, "#0\n");
			reformat_string (buffer, &p);
			send_to_char (p, ch);
			free_mem (p); // char*
		}

		else
		{			/* npc with long */
			if (IS_SUBDUER (i))
			{
				if (IS_RIDER (i))
					sprintf (buffer,
					"#5%s#0, mounted on #5%s#0, has #5%s#0 in tow.\n",
					char_short (i), char_short (i->mount),
					i->subdue == ch ? "you" : char_short (i->subdue));
				else
					sprintf (buffer, "#5%s#0 has #5%s#0 in tow.\n",
					char_short (i),
					i->subdue == ch ? "you" : char_short (i->subdue));
				buffer[2] = toupper (buffer[2]);
				send_to_char (buffer, ch);
			}

			else if (IS_RIDER (i) && !IS_SUBDUER (i))
			{

				sprintf (buffer, "%s#0 %s %s#5%s#0.",
					char_short (i), AWAKE (i) ?
					"sits atop" : "is asleep upon", AWAKE (i->mount) ?
					"" : "a sleeping mount, ", char_short (i->mount));

				*buffer = toupper (*buffer);
				sprintf (buf2, "#5%s", buffer);
				sprintf (buffer, "%s", buf2);
				act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

			else if (IS_RIDEE (i))
			{

				if (ch == i->mount)
				{
					sprintf (buffer, "You sit upon %s#5%s#0.\n", AWAKE (i) ?
						"" : "a sleeping mount, ", char_short (i));

					send_to_char (buffer, ch);
				}
			}

			else if (IS_HITCHEE (i))
			{

				if (i == ch)
					sprintf (buffer, "#5You#0 ride upon #5%s#0.\n",
					char_short (i));
				else
					sprintf (buffer, "#5%s#0 is here, hitched to #5%s#0.\n",
					char_short (i),
					i->hitcher == ch ? "you" : char_short (i->hitcher));

				buffer[2] = toupper (buffer[2]);
				send_to_char (buffer, ch);
			}

			else
			{
				*buffer = '\0';


				if ((GET_FLAG (i, FLAG_ENTERING) ||
					GET_FLAG (i, FLAG_LEAVING)) && enter_exit_msg (i, buffer))
				{
					if (GET_TRUST(ch)) {std::stringstream health;
					health << " #0<" << wound_total(i,true) << "> ";
					strcat (buffer, health.str().c_str());
					}

					if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
						strcat (buffer, " #2(new player)#0");

					if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
						strcat (buffer, " #B(new player guide)#0");

					if (i->desc && !IS_NPC (i) && i->desc->idle)
						strcat (buffer, "#1(idle)#0 ");

					if (GET_FLAG (i, FLAG_WIZINVIS))
						strcat (buffer, " #C(wizinvis)#0");

					if (get_affect (i, MAGIC_HIDDEN))
						strcat (buffer, " #1(hidden)#0");

					if (are_grouped (i, ch))
						strcat (buffer, " #6(grouped)#0");

					if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
						strcat (buffer, " #1(invisible)#0");

					if (get_affect (i, MAGIC_AFFECT_CONCEALMENT))
						strcat (buffer, " #1(blend)#0");

					if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
						strcat (buffer, " #1(editing)#0");

					if (i->desc && i->desc->original && !IS_MORTAL (ch))
						strcat (buffer, " #2(animated)#0");

					if (!IS_NPC (i) && !i->desc && !i->pc->admin_loaded)
						strcat (buffer, " #1(link dead)#0");

					if (!IS_MORTAL (ch) && !IS_NPC (i) && !i->desc
						&& i->pc->admin_loaded)
						strcat (buffer, " #3(loaded)#0");

					strcat (buffer, "#0\n");
					reformat_string (buffer, &p);
					send_to_char (p, ch);
					free_mem (p); // char*
				}
				else if ((af = get_affect (i, AFFECT_SHADOW)) &&
					af->a.shadow.edge != -1)
				{
					sprintf (buf, "%s", char_short (i));
					*buf = toupper (*buf);
					sprintf (buffer + strlen (buffer),
						"#5%s is here, #0standing %s.", buf,
						dir_names[af->a.shadow.edge]);
					act (buffer, false, ch, 0, i, TO_CHAR | _ACT_FORMAT);
				}
				else if ((af = get_affect (i, AFFECT_GUARD_DIR)) &&
					af->a.shadow.edge != -1)
				{
					sprintf (buf, "%s", char_short (i));
					*buf = toupper (*buf);
					sprintf (buffer + strlen (buffer),
						"#5%s#0 is here, guarding the %s exit.", buf,
						dirs[af->a.shadow.edge]);
					act (buffer, false, ch, 0, i, TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					sprintf (buffer, "#5");
					strcat (buffer, char_long (i, GET_TRUST (ch)));

					if (GET_TRUST(ch)) {std::stringstream health;
					health << " #0<" << wound_total(i,true) << "> ";
					strcat (buffer, health.str().c_str());
					}

					if (!IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG))
						strcat (buffer, " #2(new player)#0");

					if (!IS_NPC (i) && IS_GUIDE(i) && IS_SET (i->flags, FLAG_GUEST))
						strcat (buffer, " #B(new player guide)#0");

					if (get_affect (i, MAGIC_HIDDEN))
						strcat (buffer, " #1(hidden)#0");

					if (are_grouped (i, ch))
						strcat (buffer, " #6(grouped)#0");

					if (i->desc && i->desc->idle)
						strcat (buffer, " #1(idle)#0");

					if (GET_FLAG (i, FLAG_WIZINVIS))
						strcat (buffer, " #C(wizinvis)#0");

					if (get_affect (i, MAGIC_AFFECT_INVISIBILITY))
						strcat (buffer, " #1(invisible)#0");

					if (get_affect (i, MAGIC_AFFECT_CONCEALMENT))
						strcat (buffer, " #1(blend)#0");

					if (IS_SET (i->act, PLR_QUIET) && !IS_NPC (i))
						strcat (buffer, " #1(editing)#0");

					if (i->desc && i->desc->original && !IS_MORTAL (ch))
						strcat (buffer, " #2(animated)#0");

					if (!IS_NPC (i) && !i->desc && !i->pc->admin_loaded)
						strcat (buffer, " #1(link dead)#0");

					if (!IS_MORTAL (ch) && !IS_NPC (i) && !i->desc
						&& i->pc->admin_loaded)
						strcat (buffer, " #3(loaded)#0");

					strcat (buffer, "#0\n");
					reformat_string (buffer, &p);
					send_to_char (p, ch);
					free_mem (p); //char*
				}
			}
		}

		if ((af = get_affect (i, MAGIC_TOLL)) &&
			af->a.toll.room_num == i->in_room)
		{
			sprintf (buffer, "$n #Dis collecting tolls from people leaving "
				"%s.#0", dirs[af->a.toll.dir]);
			act (buffer, true, i, 0, ch, TO_VICT);
		}

	}
	else if (mode == 1 || mode == 15)
	{

		if (i->description)
		{

			if (((eq = get_equip (i, WEAR_HEAD))
				&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
				|| ((eq = get_equip (i, WEAR_FACE))
				&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
				|| ((eq = get_equip (i, WEAR_NECK_1))
				&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
				&& IS_SET (i->affected_by, AFF_HOODED))
				|| ((eq = get_equip (i, WEAR_NECK_2))
				&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
				&& IS_SET (i->affected_by, AFF_HOODED))
				|| ((eq = get_equip (i, WEAR_ABOUT))
				&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
				&& IS_SET (i->affected_by, AFF_HOODED)))
			{
				send_to_char ("This person's features are not visible.\n", ch);
			}
			else
			{
				send_to_char (i->description, ch);
			}
		}
		else
		{
			act ("You see nothing special about $m.", false, i, 0, ch, TO_VICT);
		}

		/* Show name (first keyword) if mobile is owned by the character examining the mobile*/
		if (mode == 15)
		{
			if(IS_NPC(i))
			{
				if(i->mob->owner)
				{
					if(!strcmp(i->mob->owner, ch->tname))
					{
						sprintf (buffer, "\nYou recognise $n to be called "
							"%s.", GET_NAME (i));
						act (buffer, false, i, 0, ch, TO_VICT);
					}
				}
			}
		}

		/* show dmote */
		/* Description is formated to 65 char, so we need to use reformat_desc instead of _ACT_FORMAT*/
		if (i->dmote_str)
		{
			reformat_desc (i->dmote_str, &i->dmote_str);
			sprintf(buffer, "\n%s", i->dmote_str);
			send_to_char (buffer, ch);
		}

		/* Show a character to another */

		if (GET_MAX_HIT (i) > 0)
			percent = (100 * GET_HIT (i)) / GET_MAX_HIT (i);
		else
			percent = -1;		/* How could MAX_HIT be < 1?? */

		strcpy (buffer, char_short (i));
		*buffer = toupper (*buffer);

		if (IS_SET (i->act, ACT_VEHICLE))
		{
			if (percent >= 100)
				strcat (buffer, " is in pristine condition.\n");
			else if (percent >= 90)
				strcat (buffer, " is slightly damaged.\n");
			else if (percent >= 70)
				strcat (buffer, " is damaged, but still functional.\n");
			else if (percent >= 50)
				strcat (buffer, " is badly damaged.\n");
			else if (percent >= 25)
				strcat (buffer, " is barely functional.\n");
			else if (percent >= 10)
				strcat (buffer, " has sustained a great deal of damage!\n");
			else if (percent >= 0)
				strcat (buffer, " is about ready to collapse inward!\n");
			send_to_char ("\n", ch);
			send_to_char (buffer, ch);
		}

		*buf2 = '\0';
		*buf3 = '\0';
		strcpy (buf2, display_clan_ranks (i, ch));
		if (mode == 15 && i != ch && *buf2 && !is_hooded (i))
		{
			send_to_char ("\n", ch);
			*buf2 = toupper (*buf2);
			reformat_string (buf2, &p);
			send_to_char (p, ch);
			free_mem (p); //char*
			p = NULL;
		}

		/*
		if ( (mode == 1 || mode == 15) && !IS_NPC(i) ) {
		send_to_char ("\n", ch);
		sprintf (buffer, "%s is approximately %d inches in height, and appears to be of %s build.", HSSH(i), i->height, frames[i->frame]);
		buffer[0] = toupper(buffer[0]);
		reformat_string (buffer, &p);
		send_to_char (p, ch);
		free_mem (p); // char*
		p = NULL;
		}
		*/

		if (mode == 15 && i->damage && !IS_SET (i->act, ACT_VEHICLE)
			&& !is_hooded (i))
		{
			curdamage = i->damage;
			if (curdamage > 0 && curdamage <= i->max_hit * .25)
				sprintf (buffer, "%s face looks slightly pale.\n",
				char_short (i));
			else if (curdamage > i->max_hit * .25
				&& curdamage < i->max_hit * .50)
				sprintf (buffer, "%s face looks rather pallid.\n",
				char_short (i));
			else if (curdamage > i->max_hit * .50
				&& curdamage < i->max_hit * .75)
				sprintf (buffer, "%s face looks quite ashen.\n", char_short (i));
			else if (curdamage > i->max_hit * .75)
				sprintf (buffer, "%s face looks deathly pale.\n", char_short (i));
			*buffer = toupper (*buffer);
			send_to_char ("\n", ch);
			send_to_char (buffer, ch);
		}

		if (mode == 1)
		{
			curdamage = 0;
			for (wound = i->wounds; wound; wound = wound->next)
				curdamage += wound->damage;
			curdamage += i->damage;

			if (curdamage <= 0)
				sprintf (buffer, "%s appears to be in excellent condition.\n",
				char_short (i));
			else if (curdamage <= i->max_hit * .1667)
				sprintf (buffer,
				"%s appears to be slightly the worse for wear.\n",
				char_short (i));
			else if (curdamage > i->max_hit * .1667
				&& curdamage <= i->max_hit * .3333)
				sprintf (buffer, "%s appears injured.\n", char_short (i));
			else if (curdamage > i->max_hit * .3333
				&& curdamage <= i->max_hit * .6667)
				sprintf (buffer, "%s appears moderately injured.\n",
				char_short (i));
			else if (curdamage > i->max_hit * .6667
				&& curdamage <= i->max_hit * .8335)
				sprintf (buffer, "%s appears severely injured.\n",
				char_short (i));
			else if (curdamage >= i->max_hit * .8335)
				sprintf (buffer, "%s appears close to death.\n", char_short (i));
			send_to_char ("\n", ch);
			*buffer = toupper (*buffer);
			reformat_string (buffer, &p);
			send_to_char (p, ch);
			free_mem (p); // char*
			p = NULL;
		}

		if (mode == 15 && !IS_SET (i->act, ACT_VEHICLE)
			&& (i->wounds || i->lodged))
		{
			sprintf (buf2, "%s", show_wounds (i, 0));
			if (ch->fighting || i->fighting)
				sprintf (buf2, "%s", strip_small_minor(buf2, ch));

			send_to_char ("\n", ch);
			strcat (buf3, buf2);
			act (buf3, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buf3 = '\0';
		}

		if (mode == 15 && (!i->damage || is_hooded (i))
			&& !IS_SET (i->act, ACT_VEHICLE) && !i->wounds && !i->lodged)
		{
			send_to_char ("\n", ch);
			sprintf (buf2, "%s appears to be in excellent condition.",
				char_short (i));
			*buf2 = toupper (*buf2);
			reformat_string (buf2, &p);
			send_to_char (p, ch);
			free_mem (p); //char*
			p = NULL;
		}

		if (i->mob &&
			i->mob->vehicle_type == VEHICLE_HITCH &&
			(troom = vtor (i->mob->nVirtual)) &&
			(troom->people || troom->contents) && i->room != troom)
		{
			sprintf (buf, "\nOn board, you see:\n");
			send_to_char (buf, ch);
			if (troom->people)
				list_char_to_char (troom->people, ch, 0);
			if (troom->contents)
				list_obj_to_char (troom->contents, ch, 0, true);
		}

		for (enchantment = i->enchantments; enchantment;
			enchantment = enchantment->next)
		{
			if (*show_enchantment (enchantment))
				sprintf (buf3 + strlen (buf3), "%s",
				show_enchantment (enchantment));
		}

		if (*buf3)
		{
			act (buf3, false, ch, 0, 0, TO_CHAR);
		}

		*buf3 = '\0';
		if (i == ch)
		{
			send_to_char ("\n", i);
			do_equipment (i, "", 0);
		}
		else
		{
			if (i->equip || i->right_hand || i->left_hand)
				send_to_char ("\n", ch);
			if (i->right_hand)
			{
				if (!IS_SET (i->act, ACT_MOUNT))
				{
					if (i->right_hand->location == WEAR_PRIM
						|| i->right_hand->location == WEAR_SEC)
						sprintf (buf, "<wielded in right hand>  ");
					else if (i->right_hand->location == WEAR_BOTH)
						sprintf (buf, "<wielded in both hands>  ");
					else if (i->right_hand->location == WEAR_SHIELD)
						sprintf (buf, "<gripped in right hand>  ");
					else
						sprintf (buf, "<carried in right hand>  ");
				}
				else
					sprintf (buf, "<carried on back>        ");
				send_to_char (buf, ch);
				show_obj_to_char (i->right_hand, ch, 1);
			}
			if (i->left_hand)
			{
				if (!IS_SET (i->act, ACT_MOUNT))
				{
					if (i->left_hand->location == WEAR_PRIM
						|| i->left_hand->location == WEAR_SEC)
						sprintf (buf, "<wielded in left hand>   ");
					else if (i->left_hand->location == WEAR_BOTH)
						sprintf (buf, "<wielded in both hands>  ");
					else if (i->left_hand->location == WEAR_SHIELD)
						sprintf (buf, "<gripped in left hand>   ");
					else
						sprintf (buf, "<carried in left hand>   ");
				}
				else
					sprintf (buf, "<carried on back>        ");

				send_to_char (buf, ch);
				show_obj_to_char (i->left_hand, ch, 1);
			}

			if (i->equip && (i->left_hand || i->right_hand))
				send_to_char ("\n", ch);

			for (location = 0; location < MAX_WEAR; location++)
			{

				if (!(eq = get_equip (i, loc_order[location])))
					continue;

				if (eq == i->right_hand || eq == i->left_hand)
					continue;

				send_to_char (where[loc_order[location]], ch);

				if (location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq))
					show_obj_to_char (eq, ch, 1);
				else
					send_to_char ("#2something#0\n", ch);

				found = true;
			}
		}
	}

	else if (mode == 3)
	{
		if (CAN_SEE (ch, i))
		{
			send_to_char ("   ", ch);
			act ("$N", false, ch, 0, i, TO_CHAR);
		}
	}
}

void
do_search (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!real_skill (ch, SKILL_SEARCH))
	{
		send_to_char ("You'll need to learn some search skills first.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
		dir = -1;
	else
	{
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
		default:
			send_to_char ("That's not a valid direction.\n", ch);
			return;
		}
	}

	act ("$n carefully searches the area.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
	act ("You search carefully...", false, ch, 0, 0, TO_CHAR);

	ch->delay_type = DEL_SEARCH;
	ch->delay = 3 + number (0, 2);
	ch->delay_info1 = dir;
}

void
delayed_search (CHAR_DATA * ch)
{
	int dir = 0;
	int skill_tried = 0;
	int somebody_found = 0;
	int search_quality = 0;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	dir = ch->delay_info1;

	if (dir == -1)
	{

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			// You will never find yourself playing SOI
			if (tch == ch)
				continue;

			// Don't reveal people you can already see
			if (!get_affect (tch, MAGIC_HIDDEN))
				continue;

			// Don't reveal group members
			if (are_grouped (ch, tch))
				continue;

			// Don't reveal Admins
			if (GET_FLAG (tch, FLAG_WIZINVIS))
				continue;

			// Don't reveal when night blind
			if (!IS_LIGHT (ch->room)
				&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
				continue;

			// Don't reveal elves?
			if ((ch->room->sector_type == SECT_WOODS ||
				ch->room->sector_type == SECT_FOREST ||
				ch->room->sector_type == SECT_HILLS) &&
				get_affect (tch, MAGIC_AFFECT_CONCEALMENT))
				continue;

			// Don't reveal the invisible
			if (get_affect (tch, MAGIC_AFFECT_INVISIBILITY) &&
				!get_affect (ch, MAGIC_AFFECT_SEE_INVISIBLE))
				continue;

			//
			if (!skill_tried)
			{
				skill_use (ch, SKILL_SEARCH, 0);
				skill_tried = 1;
			}

			search_quality = skill_level (ch, SKILL_SEARCH, 0) -
				skill_level (tch, SKILL_HIDE, 0) / 5;

			if (search_quality <= 0)
				continue;

			if (search_quality < number (0, 100))
			{
				if (!number (0, 10))
					act ("$n looks right at you, but still doesn't see you.",
					true, ch, 0, tch, TO_VICT);
				continue;
			}

			act ("You are exposed by $N!", false, tch, 0, ch, TO_CHAR);
			act ("You expose $N!", false, ch, 0, tch, TO_CHAR);
			act ("$n exposes $N.", false, ch, 0, tch, TO_NOTVICT);

			if (enter_exit_msg (ch, buf))
				act (buf, false, tch, 0, 0, TO_ROOM);

			remove_affect_type (tch, MAGIC_HIDDEN);

			somebody_found = 1;
		}

		if (!somebody_found)
		{
			send_to_char ("You didn't find anybody hiding.\n", ch);
			act ("$n finishes searching.", true, ch, 0, 0, TO_ROOM);
		}

		for (obj = ch->room->contents; obj; obj = obj->next_content)
		{

			if (!(af = get_obj_affect (obj, MAGIC_HIDDEN)))
				continue;

			if (!IS_LIGHT (ch->room)
				&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
				continue;

			if (af->a.hidden.coldload_id == ch->coldload_id)
				continue;

			if (!skill_tried)
			{
				skill_use (ch, SKILL_SEARCH, 0);
				skill_tried = 1;
			}

			if (skill_level (ch, SKILL_SEARCH, 0) <= number (1, 100))
				continue;

			if (af->a.hidden.hidden_value
				&& number (1, skill_level (ch, SKILL_SEARCH, 0)) < number (1,
				af->
				a.
				hidden.
				hidden_value))
				continue;

			remove_obj_affect (obj, MAGIC_HIDDEN);

			act ("You reveal $p.", false, ch, obj, 0, TO_CHAR);
			act ("$n reveals $p.", false, ch, obj, 0, TO_ROOM);
		}

		return;
	}

	if (!ch->room->secrets[dir] ||
		ch->skills[SKILL_SEARCH] < ch->room->secrets[dir]->diff ||
		!skill_use (ch, SKILL_SEARCH, 5 * ch->room->secrets[dir]->diff))
	{
		send_to_char ("You didn't find anything.\n", ch);
		return;
	}

	send_to_char (ch->room->secrets[dir]->stext, ch);
}

/**
 * Adds a racial and POW realted descriptive sentence for rooms with Shadow or Iluavatar affect
 * Can be used for other room affect as needed
 *
 */

void
affected_desciption(ROOM_DATA * room, CHAR_DATA * ch)
{
	int shadow_level = 0;
	int iluvatar_level = 0;
	bool elfness = false;
	bool orcness = false;
	int pow_level = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	AFFECTED_TYPE *room_shadow;
	AFFECTED_TYPE *room_iluvatar;
	
	room_shadow = is_room_affected(room->affects, MAGIC_ROOM_SHADOW);
	room_iluvatar = is_room_affected(room->affects, MAGIC_ROOM_ILUVATAR);
	
	if (ch->race >= 16 && ch->race <= 19 || ch->race == 93)
		elfness = true;
	
	if (ch->race == lookup_race_id("Orc"))
		orcness = true;
	
	if(room_shadow)
		shadow_level = room_shadow->a.room.intensity;
	if (room_iluvatar)
		iluvatar_level = room_iluvatar->a.room.intensity;
	
	switch (shadow_level)
	{
		case 1:
			if (elfness)
				act ("  A faint sense of taint lingers in the air.",
				 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  A comfortable gloom has begun to spread here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  The air is heavy and there is an unsettling gloom that lingers here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The air is heavy and thick here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;

		case 2:
			if (elfness)
				act ("  Here a winding darkness surfaces that remains invisible to the eye, but clear to the senses; an impending potential.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  The shadows begin to grow thick here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  Shadows cling where they shouldn't, creating an oozing sense of gloom.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  A heaviness seems to weigh upon the region.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 3:
			if (elfness)
				act ("  An unsettling darkness falls upon the region like a dark and oily film coating everything.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  Shadows ooze from the crevices and cracks, gloomy and dark.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  An eerie darkness seems to have infiltrated this place, creeping out to shade more than is natural.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The area seems strangely gloomy.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 4:
			if (elfness)
				act ("  Dripping with taint, the sense of foul shadow growing infests, winding around to infect this area and all that reside here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  Like a blanket of dark fog the shadows rise in this area.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  The shadows grow and move like a living thing, dark and twisted.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  Creeping over the region, an unsettling shadow seems to spread.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 5:
			if (elfness)
				act ("  Writhing and undulating like a living creature, the darkness encroaches on everything here, destroying and twisting what it can.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  The darkness rises from the ground, seeking to obscure and extinguish light.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  Like a foul fog, the shadows rise to wrap tendrils of darkness around everything here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  An eerie shadow grows, dimming vision and obscuring light.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 6:
			if (elfness)
				act ("  An overwhelming sense of unnatural darkness pervades the region and seems to drain life and light like a foul siphon leaving nothing untainted.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  A heavy and pervading sense of foul darkness has completely overtaken this region.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  A sense of taint accompanies the overwhelming shadow that has completely over here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The strange darkness that surrounds this area is most unsettling.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
		
		default:
			break;
	}
	
	switch (iluvatar_level)
	{
		case 1:
			if (elfness)
				act ("  A faint sense of well being lingers in places.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  An uncomfortable light has begun to spread here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  The air is light and there is a sense of well being that lingers here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The air is light and fresh here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 2:
			if (elfness)
				act ("  Here a breath of fresh air remains invisible to the eye, but clear to the senses; an impending potential.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  The light begin to grow brighter here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  Well being clings to everything, creating an bright sense of life.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  A feeling of health seems to brightens the region.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 3:
			if (elfness)
				act ("  A feeling of warmth falls upon the region like the hand of a loving caretaker.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  Brightness ooze from the crevices and cracks, fresh and light.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  A soft warmth seems to have infiltrated this place, reaching out to brighten more than is natural.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The area seems strangely warm and comforting.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 4:
			if (elfness)
				act ("  Dripping with light, the sense of well being grows, winding around to protect this area and all that reside here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  Like a blanket of warm water, the feeling of health rise in this area.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  The warmth grows and moves like a living thing, bright and caring.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  Spreading over the region, a comforting warmth seems to spread.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 5:
			if (elfness)
				act ("  The faint sound of songs can be heard, growing and undulating like a living creature, protecting what it can.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  The warmth rises from the ground, seeking to brighten what light already exists.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  Like a bird, the sound of song rise to wrap tendrils of warmth around everything here.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  An eerie yet pleasant song grows, bringing cheer to your heart.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		case 6:
			if (elfness)
				act ("  An overwhelming sense of protection and comfort pervades the region like a song unheard since the First Age.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (orcness)
				act ("  A rich sense of protection has completely overtaken this region.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else if (GET_AUR(ch) > 7)
				act ("  A sense of warmth accompanies a song that is felt more than it is heard.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			else
				act ("  The comfort that surrounds this area is very protective.",
					 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			break;
			
		default:
			break;
	}
	
	return;	
}
//////////////////////////////////////////////////////////////////////////////
// room__get_description ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Obtain the current room description
//
/// \param[in]  room  The room to examine (i.e. "this->get_description").
/// \return     The current room description
//
/// By default, return the room's description. If there are extra descriptions
/// defined, then pick the closest case and return that instead. If you have
/// an extra description for the current weather & time scenario, return that.
/// If you don't have a match for the current weather, but you do have a
/// description for night, and it is night time, return that description.
/// Otherwise fall back on the default room description.
//
//////////////////////////////////////////////////////////////////////////////
char *
room__get_description (ROOM_DATA * room)
{
	char * description = room->description;
	ROOM_EXTRA_DATA * room_extra = room->extra;
	if (room_extra)
	{
		int weather_room = desc_weather[room->zone];
		char * extra_description = 0;
		if ((weather_room != WR_NORMAL
			&& (extra_description = room_extra->weather_desc[weather_room]))
			|| (!sun_light
			&& (extra_description = room_extra->weather_desc[WR_NIGHT])))
		{
			description = extra_description;
		}
	}
	return description;
}


void
do_look (CHAR_DATA * ch, char *argument, int cmd)
{
	int temp = 0;
	int dir = 0;
	char *ptr = '\0';
	OBJ_DATA *obj = NULL;
	CHAR_DATA *tch = NULL;
	ROOM_DATA *troom = NULL;
	ROOM_DIRECTION_DATA *exit = NULL;
	AFFECTED_TYPE *af = NULL;
	bool contents = false;
	char bitbuf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int nRoomVnum = 0, nZone = 1, original_loc = 0;
	bool change = false, again = true, abrt = false;

	char * blizzard_description =
		"\n   A howling blanket of wind-driven snow almost obscures your vision.\n";

	const char *e_dirs[] =
	{ "the north", "the east", "the south", "the west", "above", "below" };


	if (GET_POS (ch) < POSITION_SLEEPING)
	{
		send_to_char ("You are unconscious!\n", ch);
		return;
	}

	if (GET_POS (ch) == POSITION_SLEEPING)
	{
		send_to_char ("You are asleep.\n", ch);
		return;
	}

	if (is_blind (ch))
	{
		send_to_char ("You are blind!\n", ch);
		return;
	}

	if (!ch->room)
		ch->room = vtor (ch->in_room);

	
	argument = one_argument (argument, arg1);

	/** Arena **/
	if (strcasecmp (arg1, "arena") == STR_MATCH &&
		(ch->in_room == 5200 || ch->in_room == 5201 || ch->in_room == 5144))
	{
		arena__do_look (ch, argument, cmd);
		return;
	}

	/** Pit **/
	if (strcasecmp (arg1, "pit") == STR_MATCH &&
		(ch->in_room == 66950 || ch->in_room == 66951 || ch->in_room == 66952))
	{
		te_pit_do_look (ch, argument, cmd);
		return;
	}

	/** Window **/
	if ((strcasecmp (arg1, "window") == STR_MATCH && ch->in_room == OOC_LOUNGE)
		|| (!strn_cmp (arg1, "window", 6)
		&& strcasecmp (ch->room->name, PREGAME_ROOM_NAME) == STR_MATCH))
	{

		while (again)
		{

			nZone = number (1, 15);
			nRoomVnum = (nZone * 1000) + number (1, 999);

			if (!(troom = vtor (nRoomVnum)))
				continue;

			if (IS_SET (troom->room_flags, INDOORS))
				continue;

			if (troom->sector_type == SECT_INSIDE)
				continue;

			if (IS_SET (troom->room_flags, STORAGE))
				continue;

			if (strlen (troom->description) < 256)
				continue;

			if (!strncmp (troom->description, "No Description Set", 17))
				continue;

			if (!strncmp (troom->description, "   No Description Set", 20))
				continue;

			abrt = false;
			for (tch = troom->people; tch; tch = tch->next_in_room)
				if (!IS_NPC (tch))
					abrt = true;

			if (!abrt)
			{
				again = false;
			}
		}

		if (!IS_SET (ch->affected_by, AFF_INFRAVIS))
		{
			ch->affected_by |= AFF_INFRAVIS;
			change = true;
		}
		original_loc = ch->in_room;
		act
			("$n presses $s face to a window, gazing down at Arda's brilliant blue, far below.",
			true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		act
			("You press your face to a window, gazing down at Arda's brilliant blue, potent optical magics allowing you to catch a closer, lifelike glimpse of:",
			false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		char_from_room (ch);
		char_to_room (ch, nRoomVnum);
		send_to_char ("\n", ch);
		do_look (ch, "", 0);
		char_from_room (ch);
		char_to_room (ch, original_loc);
		if (IS_MORTAL (ch))
			ch->roundtime = 15;
		if (change)
			ch->affected_by &= ~AFF_INFRAVIS;
		return;
	}

	/* LOOK IN A CERTAIN DIRECTION */

	if (*arg1 && strcasecmp (arg1, "in") != STR_MATCH
		&& (dir = index_lookup (dirs, arg1)) != -1)
	{

		if (!(exit = EXIT (ch, dir)))
		{
			send_to_char ("There is no exit that way.\n", ch);
			return;
		}

		if (exit->general_description && *exit->general_description)
			send_to_char (exit->general_description, ch);

		if ((af = get_affect (ch, AFFECT_SHADOW))
			&& af->a.shadow.edge == dir
			&& IS_SET (exit->exit_info, EX_ISDOOR)
			&& IS_SET (exit->exit_info, EX_CLOSED)
			&& !IS_SET (exit->exit_info, EX_ISGATE))
		{
			send_to_char
				("Your field of view through that exit is obstructed.\n", ch);
		}

		else if ((af = get_affect (ch, AFFECT_SHADOW)) &&
			af->a.shadow.edge == dir)
		{
			send_to_char ("You are close enough to see what is in the next "
				"room:\n\n", ch);

			temp = ch->in_room;

			char_from_room (ch);
			char_to_room (ch, exit->to_room);

			list_obj_to_char (ch->room->contents, ch, 0, false);
			list_char_to_char (ch->room->people, ch, 0);

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
				if (CAN_SEE (ch, tch))
					target_sighted (ch, tch);

			char_from_room (ch);
			char_to_room (ch, temp);
		}

		else
		{

			if (!(troom = vtor (exit->to_room)))
			{
				send_to_char ("The way in that direction is blocked.\n", ch);
				return;
			}

			temp = 0;

			for (tch = troom->people; tch; tch = tch->next_in_room)
				if ((af = get_affect (tch, AFFECT_SHADOW)) &&
					af->a.shadow.edge == rev_dir[dir] && could_see (ch, tch))
				{
					act ("You see $N.", false, ch, 0, tch, TO_CHAR);
					temp = 1;
				}
		}
		return;
	}

	if (is_dark (ch->room) && IS_MORTAL (ch) &&
		!get_affect (ch, MAGIC_AFFECT_INFRAVISION) &&
		!IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("It is pitch black.\n", ch);
		return;
	}

	/* LOOK INSIDE ANOTHER OBJECT */

	if (strcasecmp (arg1, "in") == STR_MATCH)
	{

		argument = one_argument (argument, arg1);

		if (!*arg1)
		{
			send_to_char ("Look in what?\n", ch);
			return;
		}

		if (!(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
			!(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
		{
			send_to_char ("You don't see that here.\n", ch);
			return;
		}

		if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON)
		{

			if (obj->o.drinkcon.volume <= 0)
				act ("$o is empty.", false, ch, obj, 0, TO_CHAR);

			else
			{
				if (obj->o.drinkcon.capacity)
					temp = obj->o.drinkcon.volume * 3 / obj->o.drinkcon.capacity;
				else
					temp = 1;

				sprintf (buf, "$p is %sfull of %s.",
					fullness[temp],
					vnum_to_liquid_name (obj->o.drinkcon.liquid));
				act (buf, false, ch, obj, 0, TO_CHAR);
			}

			return;
		}

		else if (GET_ITEM_TYPE (obj) == ITEM_LIGHT)
		{
			if (obj->o.light.hours)
			{
				sprintf (buf,
					"$p looks like it will last about %d more hour%s.",
					obj->o.light.hours,
					(obj->o.light.hours > 1) ? "s" : "");
				act (buf, false, ch, obj, 0, TO_CHAR);
			}
			else if (obj->o.light.hours == 0)
			{
				act ("$o is spent.", false, ch, obj, 0, TO_CHAR);
			}
			else
			{
				send_to_char
					("You can't tell how much longer that will last..\n", ch);
			}
		}

		else if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
			GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
			GET_ITEM_TYPE (obj) == ITEM_SHEATH ||
			(GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
			obj->o.weapon.use_skill == SKILL_SLING) ||
			GET_ITEM_TYPE (obj) == ITEM_KEYRING)
		{

			if (IS_SET (obj->o.container.flags, CONT_CLOSED))
			{
				send_to_char ("It is closed.\n", ch);
				return;
			}

			send_to_char (fname (obj->name), ch);

			if (obj->in_room != NOWHERE)
				send_to_char (" (here) : \n", ch);
			else if (obj->carried_by)
				send_to_char (" (carried) : \n", ch);
			else
				send_to_char (" (used) : \n", ch);

			list_obj_to_char (obj->contains, ch, 1, true);
		}

		else
			send_to_char ("That is not a container.\n", ch);
	}

	/* LOOK OVERBOAD */
	else if (strcasecmp (arg1, "overboard") == STR_MATCH
		|| strcasecmp (arg1, "over") == STR_MATCH)
	{

		if (!ch->vehicle)
		{
			send_to_char ("You're not on a boat?\n", ch);
			return;
		}

		if (!IS_OUTSIDE (ch))
		{
			send_to_char ("You can do that on deck, but not from here.\n", ch);
			return;
		}

		if (is_dark (ch->room) && IS_MORTAL (ch))
			send_to_char ("It is dark overboard.\n", ch);
		else
		{
			sprintf (buf, "#6%s#0\n", ch->vehicle->room->name);
			send_to_char (buf, ch);
			
			if (weather_info[ch->vehicle->room->zone].state == HEAVY_SNOW
				&& !IS_SET (ch->vehicle->room->room_flags, INDOORS)
				&& IS_MORTAL (ch))
			{
					//added one line for HEAVY_SNOW appearance
				send_to_char (room__get_description (ch->vehicle->room), ch);
				send_to_char (blizzard_description, ch);
				affected_desciption(ch->vehicle->room, ch);
			}
			else
			{
				send_to_char (room__get_description (ch->vehicle->room), ch);
				affected_desciption(ch->vehicle->room, ch);
			}
			 
		}

		if (ch->vehicle && IS_OUTSIDE (ch))
		{
			troom = ch->vehicle->room;
			if (troom->contents || troom->people->next_in_room)
			{
				if (troom->contents)
					list_obj_to_char (troom->contents, ch, 0, false);
				if (troom->people->next_in_room)
					list_char_to_char (troom->people, ch, 0);
			}
		}
	}

	/* LOOK AT SOMETHING */
	else if (*arg1)
	{

		if (strcasecmp (arg1, "at") == STR_MATCH)
			argument = one_argument (argument, arg1);

		if (!*arg1)
		{
			send_to_char ("Look at what?\n", ch);
			return;
		}

		if ((tch = get_char_room_vis (ch, arg1)))
		{

			argument = one_argument (argument, arg1);

			if (*arg1
				&& ((obj = get_obj_in_list_vis (tch, arg1, tch->right_hand))
				|| (obj = get_obj_in_list_vis (tch, arg1, tch->left_hand))))
			{
				act ("$N is carrying $p.", false, ch, obj, tch, TO_CHAR);
				send_to_char ("\n", ch);
				if (cmd == 2)
					show_obj_to_char (obj, ch, 15);
				else
					show_obj_to_char (obj, ch, 5);
				return;
			}

			if (*arg1 && (obj = get_obj_in_list_vis (tch, arg1, tch->equip)))
			{
				act ("$N is wearing $p.", false, ch, obj, tch, TO_CHAR);
				send_to_char ("\n", ch);
				if (cmd == 2)
					show_obj_to_char (obj, ch, 15);
				else
					show_obj_to_char (obj, ch, 5);
				return;
			}

			if (*arg1)
			{
				act ("$N doesn't have that.", false, ch, 0, tch, TO_CHAR);
				return;
			}

			if (cmd == 2)
				show_char_to_char (tch, ch, 15);
			else
				show_char_to_char (tch, ch, 1);

			return;
		}

		if (!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)))
			obj = get_obj_in_list_vis (ch, arg1, ch->room->contents);

		/** BOARDS **/
		if (obj && obj->obj_flags.type_flag == ITEM_BOARD)
		{

			//if (obj->clan_data
			//&& !(is_clan_member(ch, obj->clan_data->name)))
			//{
			//show_obj_to_char (obj, ch, 15);
			//send_to_char ("\nYou can not read these reports, but you can write one.", ch);
			//return;
			//}

			//else
			//{
			one_argument (obj->name, buf);
			retrieve_mysql_board_listing (ch, buf);
			return;
			//}
		}

		/* Extra room description */

		if ((ptr = find_ex_description (arg1, ch->room->ex_description)) &&
			*ptr)
		{
			page_string (ch->desc, ptr);
			return;
		}
//added comment for REAPIR
		/* look at a single object in the room or in your hand */
		if (obj)
		{
			if (cmd == 2)
				show_obj_to_char (obj, ch, 15);
			else
				show_obj_to_char (obj, ch, 5);
			return;
		}

		if (*arg1)
		{
			if (!strn_cmp (arg1, "furnishings", strlen (arg1)) ||
				!strn_cmp (arg1, "furniture", strlen (arg1)) ||
				!strn_cmp (arg1, "tables", strlen (arg1)))
			{
				do_tables (ch, "", 0);
				return;
			}

			if (!strn_cmp (arg1, "corpses", strlen (arg1)) ||
				!strn_cmp (arg1, "bodies", strlen (arg1)) ||
				!strn_cmp (arg1, "dead", strlen (arg1)))
			{
				do_corpses (ch, "", 0);
				return;
			}

			if (!strn_cmp (arg1, "objects", strlen (arg1)))
			{
				show_contents (ch, "", 1);
				return;
			}

			if (!strn_cmp (arg1, "crowd", strlen (arg1)) ||
				!strn_cmp (arg1, "individuals", strlen (arg1)))
			{
				show_contents (ch, "", 2);
				return;
			}
		}

		send_to_char ("You do not see that here.\n", ch);

		return;
	}

	else
	{				/* General look */

		
		
		if (IS_MORTAL (ch))
		{

			sprintf (buf, "#6%s#0", ch->room->name);
			send_to_char (buf, ch);
		}
		else
		{
			sprintbit (ch->room->room_flags, room_bits, bitbuf);

			sprintf (buf, "#6%s#0 #2[%d: %s%s#6%s#2]#0",
				ch->room->name,
				ch->room->nVirtual,
				bitbuf, ch->room->noInvLimit ? "NoInvLimit " : "", sector_types[ch->room->sector_type]);

			send_to_char (buf, ch);

			if (ch->room->prg)
				send_to_char (" [Prog]", ch);

			if (ch->room->deity && ch->room->sector_type != SECT_LEANTO)
			{
				/* Lean-tos use the deity int to record the number of people camping there. */
				sprintf (buf, " Temple of %s\n", deity_name[ch->room->deity]);
				send_to_char (buf, ch);
			}

			if (ch->room->sector_type == SECT_LEANTO)
			{
				sprintf (buf, "\nNumber of Occupants: %d.",
					(ch->room->deity - 15));
				if (!IS_MORTAL (ch))
					send_to_char (buf, ch);
			}
		}

		/* Display path vehicle can take */

		if (ch->vehicle &&
			is_he_somewhere (ch->vehicle) &&
			ch->vehicle->mob->helm_room == ch->in_room)
		{

			strcpy (buf, " May sail: [#4");

			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!EXIT (ch->vehicle, dir) ||
					!(troom = vtor (EXIT (ch->vehicle, dir)->to_room)))
					continue;

				if (troom->sector_type == SECT_DOCK ||
					troom->sector_type == SECT_REEF ||
					troom->sector_type == SECT_OCEAN ||
					is_room_affected (troom->affects, MAGIC_ROOM_FLOOD))
					add_char (buf, toupper (*dirs[dir]));
			}

			strcat (buf, "#0]");
			send_to_char (buf, ch);
		}

		/**  if it is snowing, they can't see the exits **/
		if (((weather_info[ch->room->zone].state != HEAVY_SNOW
			|| get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			|| IS_SET (ch->affected_by, AFF_INFRAVIS))
			|| IS_SET (ch->room->room_flags, INDOORS))
			|| !IS_MORTAL (ch))
		
		{
			sprintf (buf, "#6Exits:#0 ");

			for (dir = 0; dir <= LAST_DIR; dir++)
			{
				if (!EXIT (ch, dir))
					continue;
				if (ch->room->secrets[dir]
				&& IS_SET (EXIT (ch, dir)->exit_info, EX_CLOSED)
					&& IS_MORTAL (ch))
					continue;
				if (EXIT (ch, dir)->to_room
					&& (troom = vtor (EXIT (ch, dir)->to_room))
					&& IS_SET (troom->room_flags, FALL))
					sprintf (buf + strlen (buf), "#1%s#0 ", dirs[dir]);
				else if (EXIT (ch, dir)->to_room
					&& (troom = vtor (EXIT (ch, dir)->to_room))
					&& (troom->sector_type == SECT_RIVER
					|| troom->sector_type == SECT_LAKE
					|| troom->sector_type == SECT_OCEAN
					|| troom->sector_type == SECT_REEF
					|| troom->sector_type == SECT_UNDERWATER))
					sprintf (buf + strlen (buf), "#4%s#0 ", dirs[dir]);
				else if (EXIT (ch, dir)->to_room && (troom = vtor (EXIT(ch, dir)->to_room)) && (ch->room->sector_type == SECT_ROAD || ch->room->sector_type == SECT_TRAIL || IS_SET(ch->room->room_flags, ROAD)) && (troom->sector_type == SECT_ROAD || troom->sector_type == SECT_TRAIL || IS_SET(troom->room_flags, ROAD)))
					sprintf (buf + strlen (buf), "#3%s#0 ", dirs[dir]);
				else
					sprintf (buf + strlen (buf), "#2%s#0 ", dirs[dir]);
				if (!EXIT (ch, dir)->exit_info
					|| (dir > 5
					&& (obj = find_dwelling_obj (ch->room->nVirtual))
					&& !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE)))
					continue;
				sprintf (buf + strlen (buf), "(%s %s) ",
						 IS_SET (EXIT (ch, dir)->exit_info, EX_CLOSED) 
						 ? "closed" : "open", EXIT (ch, dir)->keyword);
			}

			send_to_char ("\n", ch);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		else {
			send_to_char ("\n", ch);
		}

		if (is_dark(ch->room) && IS_SET (ch->affected_by, AFF_INFRAVIS)) {
			send_to_char("#6It is dark here.#0\n\n", ch);
		}
		else {
			send_to_char ("\n", ch);
		}

		
		if (weather_info[ch->room->zone].state == HEAVY_SNOW
			&& !IS_SET (ch->room->room_flags, INDOORS)
			&& IS_MORTAL (ch)
			&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
		{
				//added one line for HEAY_SNOW chagnes
			send_to_char (room__get_description (ch->room), ch);
			send_to_char (blizzard_description, ch);
			affected_desciption(ch->room, ch);
		}
		else
		{
			send_to_char (room__get_description (ch->room), ch);
			affected_desciption(ch->room, ch);
		}


		/*** Every time a room is looked at, check for weather objects that need be loaded.
		To avoid uneccessary memory use this will not be done on the Builder Port. ***/
		if(!(engine.in_build_mode ()))
		{
			if(IS_OUTSIDE (ch))
			{
				load_weather_obj(ch->room);
			}
		}

		for (obj = ch->room->contents; obj; obj = obj->next_content)
			if (CAN_SEE_OBJ (ch, obj))
				contents = true;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
			if (CAN_SEE (ch, tch) && ch != tch)
				contents = true;

		if (contents)
			send_to_char ("\n", ch);

		if (!IS_SET (ch->room->room_flags, INDOORS)
			&& ch->room->sector_type != SECT_UNDERWATER)
		{
			*buf = '\0';
			if (!contents)
				sprintf (buf, "\n");
			if (IS_SET (ch->room->room_flags, STIFLING_FOG))
				sprintf (buf + strlen (buf),
				"#6A stifling fog blankets the area, minimizing visibility.#0\n");
			else if (weather_info[ch->room->zone].state == LIGHT_RAIN)
				sprintf (buf + strlen (buf),
				"#6A light, cool rain is falling here.#0\n");
			else if (weather_info[ch->room->zone].state == STEADY_RAIN)
				sprintf (buf + strlen (buf),
				"#6A copious amount of rain is falling steadily here.#0\n");
			else if (weather_info[ch->room->zone].state == HEAVY_RAIN)
				sprintf (buf + strlen (buf),
				"#6Rain falls in heavy sheets, inundating the area with water.#0\n");
			else if (weather_info[ch->room->zone].state == LIGHT_SNOW)
				sprintf (buf + strlen (buf),
				"#6A light dusting of snow is falling in the area.#0\n");
			else if (weather_info[ch->room->zone].state == STEADY_SNOW)
				sprintf (buf + strlen (buf),
				"#6Snow is falling steadily here, beginning to blanket the area.#0\n");
			else if (weather_info[ch->room->zone].fog == THIN_FOG)
				sprintf (buf + strlen (buf),
				"#6Thin, silvery tendrils of fog permeate the area.#0\n");
			else if (weather_info[ch->room->zone].fog == THICK_FOG)
				sprintf (buf + strlen (buf),
				"#6The area is cloaked in a thick, muted veil of dense white fog.#0\n");
			else if (weather_info[ch->room->zone].state == CHANCE_RAIN)
				sprintf (buf + strlen (buf),
				"#6The air here carries the subtle aroma of impending rain.\n#0");
			/*** Special effects set by IMMs***/
			if (weather_info[ch->room->zone].special_effect != NO_EFFECT)
			{
				//show effect specific message
				if (weather_info[ch->room->zone].special_effect == VOLCANIC_SMOKE)
				{
					sprintf (buf + strlen (buf),
						"#6A cloud of thick, dust filled volcanic smoke drifts through the air.\n#0");
				}
				else if (weather_info[ch->room->zone].special_effect == FOUL_STENCH)
				{
					sprintf (buf + strlen (buf),
						"#6A foul stench permeates the area.\n#0");
				}
				else if (weather_info[ch->room->zone].special_effect == LOW_MIST)
				{
					sprintf (buf + strlen (buf),
						"#6A low, eerie mist sits heavily upon the land.\n#0");
				}
			}
			if (contents)
				sprintf (buf + strlen (buf), "\n");
			if (strlen (buf) > 2)
				send_to_char (buf, ch);
		}

		list_obj_to_char (ch->room->contents, ch, 0, false);

		list_char_to_char (ch->room->people, ch, 0);

		*buf = '\0';

		for (af = ch->room->affects; af; af = af->next)
		{
			if (af->type == MAGIC_ROOM_FIGHT_NOISE)
				sprintf (buf + strlen (buf),
				"#1The clamor of an armed conflict emanates from %s!#0\n",
				e_dirs[af->a.room.duration]);
		}

		if (*buf)
		{
			send_to_char ("\n", ch);
			send_to_char (buf, ch);
		}
	}
}

int
read_pc_message (CHAR_DATA * ch, char *name, char *argument)
{
	CHAR_DATA *who = NULL;
	MESSAGE_DATA *message = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	argument = one_argument (argument, buf);

	if (!atoi (buf))
	{
		send_to_char ("Which message?\n", ch);
		unload_pc (who);
		return 1;
	}

	if (!(message = load_message (name, 7, atoi (buf))))
	{
		send_to_char ("No such message.\n", ch);
		unload_pc (who);
		return 1;
	}

	sprintf (b_buf, "#6Date:#0    %s\n"
		"#6Author:#0  %s\n"
		"#6Subject:#0 %s\n\n%s", message->date, message->poster,
		message->subject, message->message);

	send_to_char ("\n", ch);
	page_string (ch->desc, b_buf);

	if (ch == who && !message->flags)
		mark_as_read (ch, atoi (buf));

	free_mem(message);
	unload_pc (who);

	return 1;
}

int
read_virtual_message (CHAR_DATA * ch, char *name, char *argument)
{
	MESSAGE_DATA *message = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*name = toupper (*name);

	argument = one_argument (argument, buf);

	if (!atoi (buf))
	{
		send_to_char ("Which message?\n", ch);
		return 1;
	}

	if (!(message = load_message (name, 5, atoi (buf))))
	{
		send_to_char ("No such virtual board message.\n", ch);
		return 1;
	}

	sprintf (b_buf, "#6Date:#0     %s\n"
		"#6Author:#0   %s\n"
		"#6Subject:#0  %s\n"
		"#6Context:#0  http://www.middle-earth.us/index.php?display=staffportal&context=%ld&db=all##%ld\n\n%s",
		message->date, message->poster, message->subject,
		message->nTimestamp, message->nTimestamp, message->message);

	send_to_char ("\n", ch);
	page_string (ch->desc, b_buf);

	free_mem(message);

	return 1;
}

void
do_read (CHAR_DATA * ch, char *argument, int cmd)
{
	int msg_num = 0;
	OBJ_DATA *board_obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		if (!*buf)
		{
			send_to_char ("Which message?\n", ch);
			return;
		}

		if (!
			((board_obj = get_obj_in_dark (ch, buf, ch->right_hand))
			&& (GET_ITEM_TYPE (board_obj) == ITEM_BOOK
			|| GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT
			|| GET_ITEM_TYPE (board_obj) == ITEM_BOARD))
			&& !((board_obj = get_obj_in_dark (ch, buf, ch->left_hand))
			&& (GET_ITEM_TYPE (board_obj) == ITEM_BOOK
			|| GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT
			|| GET_ITEM_TYPE (board_obj) == ITEM_BOARD))
			&& !((board_obj = get_obj_in_dark (ch, buf, ch->equip))
			&& (GET_ITEM_TYPE (board_obj) == ITEM_BOOK
			|| GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT
			|| GET_ITEM_TYPE (board_obj) == ITEM_BOARD))
			&& !((board_obj = get_obj_in_list_vis (ch, buf, ch->room->contents))
			&& (GET_ITEM_TYPE (board_obj) == ITEM_BOOK
			|| GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT
			|| GET_ITEM_TYPE (board_obj) == ITEM_BOARD)))
		{
			if (IS_MORTAL (ch) || !read_pc_message (ch, buf, argument))
			{
				if (IS_MORTAL (ch) || !read_virtual_message (ch, buf, argument))
				{
					send_to_char ("You can't see that board.\n", ch);
				}
			}
			return;
		}

		if (GET_ITEM_TYPE (board_obj) == ITEM_BOOK
			|| GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT)
		{
			do_look (ch, board_obj->name, 0);
			return;
		}

		argument = one_argument (argument, buf);

		if (!isdigit (*buf))
		{
			send_to_char ("Which message on that board?\n", ch);
			return;
		}
	}

	else
	{
		for (board_obj = ch->room->contents;
			board_obj; board_obj = board_obj->next_content)
		{

			if (!CAN_SEE_OBJ (ch, board_obj))
				continue;

			if (board_obj->obj_flags.type_flag == ITEM_BOARD)
				break;
		}

		if (!board_obj)
		{
			send_to_char ("You do not see a board here.\n", ch);
			return;
		}
	}

	msg_num = atoi (buf);

	one_argument (board_obj->name, buf);

	if (board_obj->clan_data &&
		(!is_clan_member(ch, board_obj->clan_data->name)))

	{
		send_to_char ("You are not authorized to read these reports", ch);
		return;
	}

	display_mysql_board_message (ch, buf, msg_num, 0);
}

void
do_examine (CHAR_DATA * ch, char *argument, int cmd)
{
	do_look (ch, argument, 2);
}

void
do_exits (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	ROOM_DIRECTION_DATA *exit = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char strExit[AVG_STRING_LENGTH] = { '\0' };

	char *exits[] = {
		"North",
		"East ",
		"South",
		"West ",
		"Up   ",
		"Down ",
		"Outside",
		"Inside"
	};

	if (is_dark (ch->room) &&
		IS_MORTAL (ch) &&
		!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("You can't see a thing!\n", ch);
		return;
	}


	*s_buf = '\0';

	for (dir = 0; dir <= LAST_DIR; dir++)
	{

		exit = EXIT (ch, dir);

		if (!exit || exit->to_room == NOWHERE)
			continue;

		if (!vtor (exit->to_room))
		{
			sprintf (buf, "Room %d lead to nonexistant room %d (do_exits)\n",
				ch->in_room, exit->to_room);
			continue;
		}

		strExit[0] = '\0';

		if (IS_SET (exit->exit_info, EX_ISDOOR)
			|| IS_SET (exit->exit_info, EX_ISGATE))
		{

			if (IS_SET (exit->exit_info, EX_CLOSED))
			{

				if (!ch->room->secrets[dir])
				{
					sprintf (strExit + strlen (strExit), "%s -  %s (Closed)",
						exits[dir], exit->keyword);
				}
				else if (!IS_MORTAL (ch) && ch->room->secrets[dir])
				{
					sprintf (strExit + strlen (strExit),
						"%s -  %s (Closed; Secret)", exits[dir],
						exit->keyword);
				}

			}
			else if (is_dark (vtor (exit->to_room)) &&
				IS_MORTAL (ch) &&
				!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
				sprintf (strExit + strlen (strExit),
				"%s - %s (Open) - Too dark to tell", exits[dir],
				exit->keyword);

			else
				sprintf (strExit + strlen (strExit),
				"%s - %s (Open) - %s",
				exits[dir], exit->keyword, vtor (exit->to_room)->name);
		}

		else
		{

			if (is_dark (vtor (exit->to_room)) &&
				IS_MORTAL (ch) &&
				!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
				sprintf (strExit + strlen (strExit), "%s - Too dark to tell",
				exits[dir]);

			else
				sprintf (strExit + strlen (strExit),
				"%s - %s", exits[dir], vtor (exit->to_room)->name);
		}

		if (!IS_MORTAL (ch))
		{
			sprintf (strExit + strlen (strExit), " [%d]",
				vtor (exit->to_room)->nVirtual);
		}

		if (strExit && strExit[0] != '\0')
		{
			sprintf (s_buf + strlen (s_buf), "%s\n", strExit);
		}
	}

	

	/** add a line if it is snowing hard **/
	if (weather_info[ch->room->zone].state == HEAVY_SNOW
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
		&& !IS_SET (ch->room->room_flags, INDOORS) && IS_MORTAL (ch))
	{
		send_to_char("The blowing snow makes it difficult to see anything!\n", ch);
		
	}
	send_to_char ("Obvious exits:\n", ch);

	if (*s_buf)
		send_to_char (s_buf, ch);
	else
		send_to_char ("None.\n", ch);
}

int
get_stat_range (int score)
{
	if (score <= 4)
		return 0;
	if (score >= 5 && score <= 6)
		return 1;
	if (score >= 7 && score <= 8)
		return 2;
	if (score >= 9 && score <= 12)
		return 3;
	if (score >= 13 && score <= 14)
		return 4;
	if (score >= 15 && score <= 17)
		return 5;
	if (score >= 18 && score <= 19)
		return 6;
	if (score >= 20 && score <= 21)
		return 7;
	if (score >= 22 && score <= 23)
		return 8;
	if (score >= 24)
		return 9;
	return 0;
}


int
get_comestible_range (int num)
{
	if (num <= 0)
		return 0;
	if (num >= 1 && num <= 5)
		return 1;
	if (num >= 6 && num <= 10)
		return 2;
	if (num >= 11 && num <= 15)
		return 3;
	if (num >= 16 && num <= 19)
		return 4;
	if (num >= 20)
		return 5;
	return 0;
}


void
hunger_thirst_process (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char update_buf[MAX_STRING_LENGTH] = { '\0' };

	if (IS_NPC (ch))
		return;

	*update_buf = '\0';

	if (get_comestible_range (ch->hunger) !=
		get_comestible_range (ch->hunger - 1))
	{
		sprintf (update_buf + strlen (update_buf), "%s",
			verbal_hunger[get_comestible_range (ch->hunger - 1)]);
	}

	if (get_comestible_range (ch->thirst) !=
		get_comestible_range (ch->thirst - 1))
	{
		if (*update_buf)
			sprintf (update_buf + strlen (update_buf), ", and ");
		sprintf (update_buf + strlen (update_buf), "%s",
			verbal_thirst[get_comestible_range (ch->thirst - 1)]);
	}

	if (*update_buf)
	{
		sprintf (buf, "#6You are %s.#0\n", update_buf);
		send_to_char (buf, ch);
	}

	if (ch->hunger > 0)
		ch->hunger--;
	if (ch->thirst > 0)
		ch->thirst--;

	if (ch->thirst < -1)
		ch->thirst = 1;
	if (ch->hunger < -1)
		ch->hunger = 1;
}

char *
suffix (int number)
{
	if (number == 1)
		return "st";
	else if (number == 2)
		return "nd";
	else if (number == 3)
		return "rd";
	else if (number < 20)
		return "th";
	else if ((number % 10) == 1)
		return "st";
	else if ((number % 10) == 2)
		return "nd";
	else if ((number % 10) == 3)
		return "rd";
	else
		return "th";
}

void
do_score (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	int weight = 0;
	int time_diff = 0, days_remaining = 0, hours_remaining = 0;
	struct time_info_data playing_time;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *af_table = NULL;
	CHAR_DATA *rch = NULL;
	OBJ_DATA *vobj = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	bool first = true;
	struct time_info_data birth_date;
	static char *verbal_stats[] =
	{ "horrible", "bad", "poor", "avg", "good", "great", "peak", "superhuman", "legendary", "epic" };
	//static char *power_verbal_stats = {"void", "flicker", "spark", "flame", "fire", "flare", "brilliant", "blinding", "consuming"}; Reference - Case
	//static char *dark_power_verbal_stats = {"void", "flicker", "trace", "shadow", "darkness"}; // Perhaps for evil dudes? - Case

	birth_date = time_info;

	birth_date.minute -= age (ch).minute;
	if (birth_date.minute < 0)
	{
		birth_date.hour -= 1;
		birth_date.minute += 60;
	}
	birth_date.hour -= age (ch).hour;
	if (birth_date.hour < 0)
	{
		birth_date.day -= 1;
		birth_date.hour += 24;
	}
	birth_date.day -= age (ch).day;
	if (birth_date.day <= 0)
	{
		birth_date.month -= 1;
		birth_date.day += 30;
	}
	birth_date.month -= age (ch).month;
	if (birth_date.month < 0)
	{
		birth_date.year -= 1;
		birth_date.month += 12;
	}
	birth_date.year -= age (ch).year;

	send_to_char ("\n", ch);
	if (!IS_SET (ch->flags, FLAG_GUEST))
		sprintf (buf, "#2%s, a %d year old %s %s:#0\n",
		GET_NAME (ch), GET_AGE (ch), lookup_race_variable (ch->race,
		RACE_NAME),
		sex_types[ch->sex]);
	else
		sprintf (buf, "#2Guest Login:#0 %s\n\n", ch->tname);

	send_to_char (buf, ch);

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		if (IS_MORTAL (ch))
			sprintf (buf,
			"Str[#2%s#0] Dex[#2%s#0] Con[#2%s#0] Int[#2%s#0] Wil[#2%s#0] Agi[#2%s#0]\n",
			verbal_stats[get_stat_range (GET_STR (ch))],
			verbal_stats[get_stat_range (GET_DEX (ch))],
			verbal_stats[get_stat_range (GET_CON (ch))],
			verbal_stats[get_stat_range (GET_INT (ch))],
			verbal_stats[get_stat_range (GET_WIL (ch))],
			verbal_stats[get_stat_range (GET_AGI (ch))]);
		else
			sprintf (buf,
			"Str[#2%d#0] Dex[#2%d#0] Con[#2%d#0] Int[#2%d#0] Wil[#2%d#0] Aura/Power[#2%d#0] Agi[#2%d#0]\n",
			GET_STR (ch), GET_DEX (ch), GET_CON (ch), GET_INT (ch),
			GET_WIL (ch), GET_AUR (ch), GET_AGI (ch));

		send_to_char ("\n", ch);
		send_to_char (buf, ch);
	}

	// November 22nd 2009, Segment to display power - Case
	if (!IS_SET (ch->flags, FLAG_GUEST)) {
		int pcAur = GET_AUR(ch);

		if (pcAur < 4) {
			send_to_char("Your soul #1flickers#0 like a candle, raw and untapped.", ch);
		}
		else if (isElf(ch)) { // If elf - Case
			if (pcAur < 25) {
				send_to_char("#6Your soul burns outward from your body brilliantly.#0", ch);
			}
			else if (pcAur < 32) {
				send_to_char("#6Blinding power burns outwards from your body.#0", ch);
			}
			else {
				send_to_char("#6Your soul burns as consumingly as Anor.#0", ch);
			}
		}

		else if (pcAur < 7) {
			send_to_char("Your soul #1sparks#0 like embers flying free of a fire.", ch);
		}
		else if (pcAur < 11) {
			send_to_char("Your soul burns with #1flames#0.", ch);
		}
		else if (pcAur < 16) {
			send_to_char("Your soul burns with potent #9fire#0.", ch);
		}
		else if (pcAur < 19) {
			send_to_char("Power #9flares#0 within your soul.", ch);
		}
		else if (pcAur < 32) {
			send_to_char("#9Potency roars through your soul.#0", ch);
		}
		else {
			send_to_char("#9Your soul burns as consumingly as Anor.#0", ch);
		}
		send_to_char("\n", ch);
	}// End of Power messages - Case

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		sprintf (buf, "You have been incarnated as #5%s#0.\n\n",
			char_short (ch));
		send_to_char (buf, ch);
		sprintf (buf, "You are a #2%s#0 #2%s#0.\n", sex_types[ch->sex],
			lookup_race_variable (ch->race, RACE_NAME));
		send_to_char (buf, ch);
	}

	int acct_rpp = 0;
	if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST) && ch->desc
		&& ch->desc->acct && (acct_rpp = ch->desc->acct->get_rpp ()) > 0
		&& IS_SET (ch->desc->acct->flags, ACCOUNT_RPPDISPLAY))
	{
		sprintf (buf, "Your account has been awarded #2%d#0 roleplay points.\n",
			acct_rpp);
		send_to_char (buf, ch);
	}

	/* Add support for listing, hunger, thirst, and intox. */
	sprintf (buf, "You are #2%s#0, and #2%s#0.\n",
		ch->hunger >=
		0 ? verbal_hunger[get_comestible_range (ch->hunger)] : "full",
		ch->thirst >=
		0 ? verbal_thirst[get_comestible_range (ch->thirst)] : "quenched");
	send_to_char (buf, ch);

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		sprintf (buf, "You are #2%d#0 inches tall, and weigh #2%d#0 pounds.\n",
			ch->height, get_weight (ch) / 100);
		send_to_char (buf, ch);

		sprintf (buf,
			"You are of #2%s#0 build for one of your people, and wear size #2%s#0 (%s).\n",
			frames[ch->frame], sizes_named[get_size (ch)],
			sizes[get_size (ch)]);
		send_to_char (buf, ch);
	}

	if (!IS_SET (ch->flags, FLAG_PACIFIST))
		sprintf (buf, "When in combat, your mode is #2%s#0.\n",
		fight_tab[ch->fight_mode].name);
	else
		sprintf (buf, "You are currently in #2Pacifist#0 mode.\n");

	send_to_char (buf, ch);

	if (GET_FLAG (ch, FLAG_AUTOFLEE))
		send_to_char ("If in combat, you will #2FLEE#0.\n", ch);

	sprintf (buf, "You are currently speaking #2%s#0.\n", skills[ch->speaks]);
	send_to_char (buf, ch);

	if (ch->writes)
	{
		sprintf (buf, "You are currently set to write in #2%s#0.\n",
			skills[ch->writes]);
		send_to_char (buf, ch);
	}

	for (i = 0; i <= 3; i++)
		if (GET_STR (ch) * enc_tab[i].str_mult_wt >= IS_CARRYING_W (ch))
			break;

	sprintf (buf, "You are currently #2%s#0.\n", enc_tab[i].encumbered_status);
	send_to_char (buf, ch);

	weight = IS_CARRYING_W (ch);

	if (weight < 1000)
		weight = (weight + 50) / 100 * 100;
	else if (weight < 10000)
		weight = (weight + 500) / 1000 * 1000;
	else
		weight = (weight + 1250) / 2500 * 2500;

	if (IS_CARRYING_W (ch) == 0)
		send_to_char ("You are carrying nothing.\n", ch);
	else
	{
		sprintf (buf, "You are carrying about #2%d#0 pounds.\n", weight / 100);
		send_to_char (buf, ch);
	}

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		if (birth_date.year > 0)
			sprintf (buf,
			"You were born on the #2%d%s#0 day of #2%s#0, #2%d#0.\n",
			birth_date.day, suffix (birth_date.day),
			month_short_name[birth_date.month], birth_date.year);
		else
			sprintf (buf,
			"You were born on the #2%d%s#0 day of #2%s#0, many millenia past.\n",
			birth_date.day, suffix (birth_date.day),
			month_short_name[birth_date.month]);
		send_to_char (buf, ch);
		playing_time =
			real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
		sprintf (buf,
			"You have been playing for #2%d#0 days and #2%d#0 hours.\n",
			playing_time.day, playing_time.hour);
		send_to_char (buf, ch);
	}

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		if (ch->fight_percentage <= 20)
		{
			sprintf (buf, "You will fight with #1%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
		else if (ch->fight_percentage <= 40)
		{
			sprintf (buf, "You will fight with #3%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
		else if (ch->fight_percentage <= 60)
		{
			sprintf (buf, "You will fight with #4%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
		else if (ch->fight_percentage <= 80)
		{
			sprintf (buf, "You will fight with #6%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
		else if (ch->fight_percentage <= 99)
		{
			sprintf (buf, "You will fight with #5%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
		else if (ch->fight_percentage == 100)
		{
			sprintf (buf, "You will fight with #F%d#0 percent of your effort.\n", ch->fight_percentage);
			send_to_char (buf, ch);
		}
	}


	switch (GET_POS (ch))
	{
	case POSITION_DEAD:
		send_to_char ("You are DEAD!\n", ch);
		break;
	case POSITION_MORTALLYW:
		send_to_char ("You are mortally wounded, and should seek help!\n", ch);
		break;
	case POSITION_UNCONSCIOUS:
		send_to_char ("You are unconscious.\n", ch);
		break;
	case POSITION_STUNNED:
		send_to_char ("You are stunned! You can't move!\n", ch);
		break;
	case POSITION_SLEEPING:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			send_to_char ("You are sleeping.\n", ch);
		else
		{
			sprintf (buf, "You are sleeping at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			send_to_char (buf, ch);
		}
		break;
	case POSITION_RESTING:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			send_to_char ("You are resting.\n", ch);
		else if (af_table && af_table->a.table.obj)
		{
			sprintf (buf, "You are resting at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			send_to_char (buf, ch);
		}
		break;
	case POSITION_SITTING:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			send_to_char ("You are sitting.\n", ch);
		else
		{
			sprintf (buf, "You are sitting at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			send_to_char (buf, ch);
		}
		break;
	case POSITION_FIGHTING:
		if (ch->fighting && ch->fighting->short_descr)
		{
			send_to_char ("\n", ch);
			act ("You are fighting $N.", false, ch, 0, ch->fighting, TO_CHAR);

			for (rch = ch->room->people; rch; rch = rch->next_in_room)
			{
				if (rch->fighting == ch && rch != ch->fighting)
					act ("You are also being attacked by $N.", false, ch, 0,
					rch, TO_CHAR);
			}

			send_to_char ("\n", ch);
		}
		else
		{
			/* Was send_to_char 'You are fighting thin air' */
			stop_fighting (ch);
		}
		break;
	case POSITION_STANDING:
		send_to_char ("You are standing.\n", ch);
		break;
	default:
		send_to_char ("You are floating.\n", ch);
		break;
	}

	for (rch = ch->room->people; rch; rch = rch->next_in_room)
	{
		if ((af = get_affect (rch, MAGIC_GUARD)) && !af->a.spell.modifier && af->a.spell.t == (long int) ch)
		{
			if (first)
				send_to_char ("\n", ch);
			act ("You are being guarded by $N.", false, ch, 0, rch, TO_CHAR);
			first = false;
		}
		if ((af = get_affect (ch, MAGIC_GUARD)) && !af->a.spell.modifier && af->a.spell.t == (long int) rch)
		{
			if (first)
				send_to_char ("\n", ch);
			act ("You are guarding $N.", false, ch, 0, rch, TO_CHAR);
			first = false;
		}
	}

	if (first == false)
		send_to_char ("\n", ch);

	if (!IS_NPC (ch) && ch->pc->create_state == STATE_DIED)
		send_to_char ("You are DEAD.\n", ch);

	if (get_affect (ch, AFFECT_GUARD_DIR))
	{
		sprintf (buf, "You are currently guarding the %s exit.\n",
			dirs[get_affect (ch, AFFECT_GUARD_DIR)->a.shadow.edge]);
		send_to_char (buf, ch);
	}

	sprintf (buf, "You #2%s#0 when you travel.\n", speeds[ch->speed]);
	send_to_char (buf, ch);

	if (get_affect(ch,MAGIC_HIDDEN))
	{
		send_to_char ("You are currently #6hidden#0.\n", ch);
	}

	if (IS_SET (ch->plr_flags, GROUP_CLOSED))
		send_to_char ("Your group is currently #2closed#0 to new members.\n", ch);
	else
		send_to_char ("Your group is currently #2open#0 to new members.\n", ch);

	if ((af = get_affect (ch, MAGIC_CRAFT_DELAY))
		&& time (0) < af->a.spell.modifier)
	{
		time_diff = af->a.spell.modifier - time (0);
		days_remaining = time_diff / (60 * 60 * 24);
		time_diff %= 60 * 60 * 24;
		hours_remaining = time_diff / (60 * 60);
		if (!days_remaining && !hours_remaining)
		{
			sprintf (buf,
				"Your craft timer has #2less than 1#0 RL hour remaining.\n");
		}
		else
		{
			sprintf (buf, "Your craft timer has approximately");
			if (days_remaining)
				sprintf (buf + strlen (buf), " #2%d#0 RL day%s ", days_remaining,
				days_remaining > 1 ? "s" : "");
			if (hours_remaining && days_remaining)
				sprintf (buf + strlen (buf), "and");
			if (hours_remaining)
				sprintf (buf + strlen (buf), " #2%d#0 RL hour%s ",
				hours_remaining, hours_remaining > 1 ? "s" : "");
			sprintf (buf + strlen (buf), "remaining.\n");
		}
		send_to_char (buf, ch);
	}

	if (!IS_MORTAL (ch))
	{
		if (!ch->hour_affects)
			send_to_char ("No spells affect you.\n", ch);
		else
		{
			for (af = ch->hour_affects; af; af = af->next)
			{


				if (af->type == AFFECT_SHADOW)
				{

					if (!af->a.shadow.shadow && af->a.shadow.edge != -1)
						sprintf (buf, "   Standing");

					else if (!is_he_somewhere (af->a.shadow.shadow))
						continue;

					else if (IS_NPC (af->a.shadow.shadow))
						sprintf (buf, "   Shadowing %s (%d)",
						af->a.shadow.shadow->short_descr,
						af->a.shadow.shadow->mob->nVirtual);
					else
						sprintf (buf, "   Shadowing PC %s",
						GET_NAME (af->a.shadow.shadow));

					if (af->a.shadow.edge != -1)
						sprintf (buf + strlen (buf), " on %s edge.",
						dirs[af->a.shadow.edge]);

					strcat (buf, "\n");

					send_to_char (buf, ch);

					continue;
				}

				else if (af->type == MAGIC_SIT_TABLE)
				{
					sprintf (buf, "   Sitting at table affect.\n");
					send_to_char (buf, ch);
					continue;
				}

				if (af->type >= JOB_1 && af->type <= JOB_3)
				{

					i =
						time_info.year * 12 * 30 + time_info.month * 30 +
						time_info.day;
					i = af->a.job.pay_date - i;

					vobj = vtoo (af->a.job.object_vnum);

					sprintf (buf,
						"   Job %d:  %d of %d days until payday\n",
						af->type - JOB_1 + 1,
						i,
						af->a.job.days);

					sprintf (buf + strlen(buf),
						"             %d x %s",
						af->a.job.count,
						!vobj ? "UNDEFINED" : vobj->short_description
						);

					if (af->a.job.employer)
					{
						sprintf (buf + strlen(buf),
							"\n             Employer: %s \n",
							vtom(af->a.job.employer)->short_descr);
					}
					else
					{
						sprintf (buf+ strlen(buf),"\n");
					}

					send_to_char (buf, ch);
					continue;
				} //if (af->type >= JOB_1


				if (af->type >= MAGIC_CRIM_BASE &&
					af->type <= MAGIC_CRIM_BASE + 100)
				{
					sprintf (buf, "   Wanted in zone %d for %d hours.\n",
						af->type - MAGIC_CRIM_BASE, af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if (af->type >= MAGIC_CRIM_HOODED &&
					af->type < MAGIC_CRIM_HOODED + 100)
				{
					sprintf (buf, "   Hooded criminal charge in zone %d for "
						"%d RL seconds.\n",
						af->type - MAGIC_CRIM_HOODED,
						af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if (af->type == MAGIC_STARED)
				{
					sprintf (buf, "%d   Studied by an enforcer.  Won't be "
						"studied for %d RL seconds.\n",
						af->type, af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
					continue;

				if (af->type == MAGIC_CRAFT_DELAY || MAGIC_CRAFT_BRANCH_STOP)
					continue;

				if (af->type == MAGIC_HIDDEN) /// \todo Check this as dead code
				{
					sprintf (buf, "%d   Hidden.\n", af->type);
					send_to_char (buf, ch);
					continue;
				}

				if (af->type == MAGIC_SNEAK)
				{
					sprintf (buf, "%d   Currently trying to sneak.\n",
						af->type);
					send_to_char (buf, ch);
					continue;
				}
				if (af->type == SA_WARNED)
				{
					send_to_char("You have been warned to surrender to an enforcer.\n", ch);
					continue;
				}
				if (af->type == SA_ALREADY_WARNED)
				{
					send_to_char("You will receive no warning from enforcers.\n", ch);
					continue;
				}

				p = lookup_string (af->type, REG_SPELLS);

				if (!p)
					p = lookup_string (af->type, REG_MAGIC_SPELLS);

				sprintf (buf, "   %s affects you by %d for %d hours.\n",
					p == NULL ? "Unknown" : p,
					af->a.spell.modifier, af->a.spell.duration);
				send_to_char (buf, ch);
			}
		}
	}

	if (get_equip (ch, WEAR_BLINDFOLD))
		send_to_char ("YOU ARE BLINDFOLDED!\n", ch);
	else if (is_blind (ch))
		send_to_char ("YOU ARE BLIND!\n", ch);

	if (IS_SET (ch->affected_by, AFF_HOODED))
		send_to_char ("You are currently cloaked and hooded.\n", ch);

	if (IS_SET (ch->affected_by, AFF_LEADER_COMMAND))
		send_to_char ("You are allowed to use special leadership commands.\n", ch);


	if (ch->voice_str)
	{
		send_to_char ("\nYour current voice string: (#2", ch);
		send_to_char (ch->voice_str, ch);
		send_to_char ("#0)\n", ch);
	}

	if (ch->travel_str)
	{
		if (!ch->voice_str)
			send_to_char ("\n", ch);
		send_to_char ("Your current travel string: (#2", ch);
		send_to_char (ch->travel_str, ch);
		send_to_char ("#0)\n", ch);
	}

	if (ch->pmote_str)
	{
		if (!ch->voice_str && !ch->travel_str)
			send_to_char ("\n", ch);
		send_to_char ("Your current pmote: (#2", ch);
		send_to_char (ch->pmote_str, ch);
		send_to_char ("#0)\n", ch);
	}

	if (ch->dmote_str)
	{
		if (!ch->voice_str && !ch->travel_str && !ch->pmote_str)
			send_to_char ("\n", ch);
		send_to_char ("Your current dmote:\n", ch);
		send_to_char (ch->dmote_str, ch);
		send_to_char ("\n", ch);
	}

	/* after dmote, before clan dump, list subscribed petition zones for imms */
	if (GET_TRUST(ch))
	{
		send_to_char("You are subscribed to petitions from:#5",ch);
		for (int i=0; i<SPHERE_COUNT; i++)
		{
			if (IS_SET(ch->petition_flags,(1<<i)))
			{
				send_to_char(" ",ch);
				send_to_char(spheres[i].name,ch);
			}
		}
		send_to_char("#0",ch);
	}



	clan__do_score (ch);
}

#define skill_lev(val) val >= 70 ? " Master " : val >= 50 ? " Adroit " : val >= 30 ? "Familiar" : " Novice "

void
do_skills (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	int j = 2;
	int loaded_char = 0;
	CHAR_DATA *who = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (IS_MORTAL (ch) || !*buf)
		who = ch;
	else if (!(who = get_char_room_vis (ch, buf)))
	{
		if (!(who = load_pc (buf)))
		{
			send_to_char ("No body here by that name.\n", ch);
			return;
		}
		else
			loaded_char = 1;
	}

	*buf = '\0';

	if (IS_MORTAL (ch))
	{

		sprintf (buf, "\n     #2Offense:#0 %s ", skill_lev (who->offense));

		for (i = 1; i <= LAST_SKILL; i++)
		{

			if (!real_skill (who, i))
				continue;

			sprintf (buf2, "#2%12.12s:#0 %s ",
				skills[i], skill_lev (who->skills[i]));

			while (strlen (buf2) < 18)
				strcat (buf2, " ");

			strcat (buf, buf2);

			if (!(j % 3))
				strcat (buf, "\n");
			j++;
		}
	}

	else
	{
		sprintf (buf, " #2Offense#0[%03d/%03d] ", who->offense, 50);

		for (i = 1; i <= LAST_SKILL; i++)
		{
			if (real_skill (who, i))
			{
				sprintf (buf + strlen (buf),
					"#2%8.8s#0[%03d/%03d] ", skills[i], who->skills[i],
					calc_lookup (who, REG_CAP, i));
				if (!(j % 4))
					strcat (buf, "\n");
				j++;
			}
		}
	}

	send_to_char (buf, ch);

	if (buf[strlen (buf) - 1] != '\n')
		send_to_char ("\n", ch);

	if (loaded_char)
		unload_pc (who);
}


char *
time_string (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char suf[4] = { '\0' };
	int day = 0;
	int minutes = 0;
	int high_sun = 0;
	int nCharAstronomySkill = 0;
	char day_buf[AVG_STRING_LENGTH];
	char phrase[MAX_STRING_LENGTH];
	static char time_str[MAX_STRING_LENGTH] = { '\0' };

	const char *season_string[AVG_STRING_LENGTH] = {
		"deep winter",
		"late winter",
		"early spring",
		"mid-spring",
		"late spring",
		"early summer",
		"high summer",
		"late summer",
		"early autumn",
		"mid-autumn",
		"late autumn",
		"early winter"
	};

	minutes = 4 * (15 * 60 - (next_hour_update - time (0))) / 60;

	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	sprintf (phrase, "[report error: %d]", time_info.hour);

	if (time_info.hour == sunset[time_info.month] - 2)
		sprintf (phrase, "late afternoon");
	else if (time_info.hour == sunset[time_info.month] - 1)
		sprintf (phrase, "dusk");
	else if (time_info.hour == sunset[time_info.month])
		sprintf (phrase, "twilight");
	else if (time_info.hour == 23)
		sprintf (phrase, "before midnight");
	else if (time_info.hour == 0)
		sprintf (phrase, "midnight");
	else if (time_info.hour == 1)
		sprintf (phrase, "after midnight");
	else if (time_info.hour == sunrise[time_info.month] - 1)
		sprintf (phrase, "before dawn");
	else if (time_info.hour == sunrise[time_info.month])
		sprintf (phrase, "dawn");
	else if (time_info.hour == sunrise[time_info.month] + 1)
		sprintf (phrase, "early morning");
	else if (time_info.hour == high_sun - 1)
		sprintf (phrase, "late morning");
	else if (time_info.hour == high_sun)
		sprintf (phrase, "high sun");
	else if (time_info.hour == high_sun + 1)
		sprintf (phrase, "early afternoon");

	else if (time_info.hour > high_sun
		&& time_info.hour < sunset[time_info.month] - 2)
		sprintf (phrase, "afternoon");
	else if (time_info.hour > sunrise[time_info.month] + 1
		&& time_info.hour < high_sun - 1)
		sprintf (phrase, "morning");
	else if (time_info.hour >= sunset[time_info.month] && time_info.hour < 21)
		sprintf (phrase, "evening");
	else if (time_info.hour >= sunset[time_info.month] && time_info.hour < 23)
		sprintf (phrase, "night time");
	else if (time_info.hour > high_sun + 1
		&& time_info.hour < sunset[time_info.hour] - 2)
		sprintf (phrase, "afternoon");
	else if (time_info.hour > 1
		&& time_info.hour < sunrise[time_info.month] - 1)
		sprintf (phrase, "late at night");

	// Astronomy skill gives knowledge of more precise time
	if (ch && ch->skills[SKILL_ASTRONOMY])
	{

		nCharAstronomySkill = ch->skills[SKILL_ASTRONOMY];
		if (!IS_OUTSIDE (ch) || (IS_NIGHT && !moon_light[ch->room->zone]))
		{
			nCharAstronomySkill -= 40;
		}
		else
		{
			nCharAstronomySkill -= (10 * weather_info[ch->room->zone].clouds);
			nCharAstronomySkill -= (2 * weather_info[ch->room->zone].state);
		}

		if (nCharAstronomySkill >= 70)
		{
			if (minutes < 7)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (minutes < 23)
			{
				sprintf (phrase + strlen (phrase), ", quarter-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (minutes < 37)
			{
				sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (minutes < 52)
			{
				sprintf (phrase + strlen (phrase), ", quarter-to %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

		else if (nCharAstronomySkill >= 50)
		{
			if (minutes < 15)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (minutes < 45)
			{
				sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

		else if (nCharAstronomySkill >= 30)
		{
			if (minutes < 30)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

	}

	sprintf (buf, "It is %s ", phrase);

	if (ch && !IS_MORTAL (ch))
	{
		sprintf (buf + strlen (buf), "[%d:%s%d %s] ",
			((time_info.hour % 12 == 0) ? 12 : ((time_info.hour) % 12)),
			minutes >= 10 ? "" : "0", minutes,
			((time_info.hour >= 12) ? "pm" : "am"));
	}

	day = time_info.day + 1;	/* day in [1..35] */

	if (day == 1)
		strcpy (suf, "st");
	else if (day == 2)
		strcpy (suf, "nd");
	else if (day == 3)
		strcpy (suf, "rd");
	else if (day < 20)
		strcpy (suf, "th");
	else if ((day % 10) == 1)
		strcpy (suf, "st");
	else if ((day % 10) == 2)
		strcpy (suf, "nd");
	else if ((day % 10) == 3)
		strcpy (suf, "rd");
	else
		strcpy (suf, "th");

	sprintf (day_buf, "%d%s", day, suf);

	/* Special output for holidays */

	if (time_info.holiday == 0 &&
		!(time_info.month == 1 && day == 12) &&
		!(time_info.month == 4 && day == 10) &&
		!(time_info.month == 7 && day == 11) &&
		!(time_info.month == 10 && day == 12))
		sprintf (buf + strlen (buf), "on the %s day of the %s,", day_buf,
		month_name[(int) time_info.month]);
	else
	{
		if (time_info.holiday > 0)
		{
			sprintf (buf + strlen (buf), "on %s,",
				holiday_names[time_info.holiday]);
		}
		else if (time_info.month == 1 && day == 12)
			sprintf (buf + strlen (buf), "on Erukyerme, The Prayer to Eru,");
		else if (time_info.month == 4 && day == 10)
			sprintf (buf + strlen (buf), "on Lairemerende, The Greenfest,");
		else if (time_info.month == 7 && day == 11)
			sprintf (buf + strlen (buf), "on Eruhantale, Thanksgiving to Eru,");
		else if (time_info.month == 10 && day == 12)
			sprintf (buf + strlen (buf), "on Airilaitale, The Hallowmas,");
	}

	sprintf (buf + strlen (buf),
		" %s in the year %d of the Steward's Reckoning.\n",
		season_string[(int) time_info.month], time_info.year);

	sprintf (time_str, "%s", buf);

	return time_str;
}

void
do_time (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *p;
	int day = time_info.day + 1, d_day = 0, moon_q = 0, moon_r = 0, moon_s = 0;
	static char *strRelativeTime[] = {
		"morning", "morning", "morning", "morning", "afternoon", "afternoon",
		"evening", "night"
	};

	sprintf (buf, "\n#6%s#0", time_string (ch));

	if (ch->skills[SKILL_ASTRONOMY])
	{

		// Sunrise and set

		if (time_info.hour < sunrise[time_info.month])
		{

			sprintf (buf + strlen (buf),
				" #6Anor shall rise around %s o'clock this morning and will rest again around %s o'clock today.#0 ",
				strTimeWord[sunrise[time_info.month]],
				strTimeWord[sunset[time_info.month]]);

		}
		else if (time_info.hour > sunset[time_info.month])
		{

			sprintf (buf + strlen (buf),
				" #6Anor shall rise around %s o'clock this morning and will rest again around %s o'clock tomorrow.#0 ",
				strTimeWord[sunrise[time_info.month]],
				strTimeWord[sunset[time_info.month]]);

		}
		else
		{
			sprintf (buf + strlen (buf),
				" #6Anor shall rest around %s o'clock today and will rise again around %s o'clock tomorrow morning.#0 ",
				strTimeWord[sunset[time_info.month]],
				strTimeWord[sunrise[time_info.month]]);
		}


		// Moon rise, set and phase

		d_day = (time_info.day + 15) % 30;
		moon_q = d_day * 24 / 30;
		moon_r = (24 + (moon_q - 6)) % 24;
		moon_s = (24 + (moon_q - 17)) % 24;


		if (moon_r < moon_s)
		{

			if (time_info.hour < moon_r)
			{
				sprintf (buf + strlen (buf),
					" #6(1) Ithil shall rise around %s o'clock this %s and will rest again around %s o'clock this %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);
			}
			else if (time_info.hour > moon_s)
			{
				sprintf (buf + strlen (buf),
					" #6(2) Ithil shall rise around %s o'clock tomorrow %s and will rest again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);

			}
			else
			{
				sprintf (buf + strlen (buf),
					" #6(3) Ithil shall rest around %s o'clock this %s and will rise again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);

			}

		}

		else
		{
			if (time_info.hour < moon_s)
			{
				sprintf (buf + strlen (buf),
					" #6(4) Ithil shall rest around %s o'clock this %s and will rise again around %s o'clock this %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);
			}
			else if (time_info.hour > moon_r)
			{
				sprintf (buf + strlen (buf),
					" #6(5) Ithil shall rest around %s o'clock tomorrow %s and will rise again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);

			}
			else
			{
				sprintf (buf + strlen (buf),
					" #6(6) Ithil shall rise around %s o'clock this %s and will rest again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);

			}
		}


		// Feastday info

		if (time_info.holiday == 0 &&
			!(time_info.month == 1 && day == 12) &&
			!(time_info.month == 4 && day == 10) &&
			!(time_info.month == 7 && day == 11) &&
			!(time_info.month == 10 && day == 12))
		{
			strcat (buf, "\n#6The next holiday will be ");
			if (time_info.month < 1
				|| (time_info.month == 1 && time_info.day <= 12))
			{
				sprintf (buf + strlen (buf), "Erukyerme in %d days.#0",
					((1 - time_info.month) * 30) + (12 - time_info.day));
			}
			else if (time_info.month < 3)
			{
				sprintf (buf + strlen (buf),
					"The Feastday of Tuilere in %d days.#0",
					((2 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 4
				|| (time_info.month == 4 && time_info.day <= 10))
			{
				sprintf (buf + strlen (buf), "The Greenfest in %d days.#0",
					((4 - time_info.month) * 30) + (10 - time_info.day));
			}
			else if (time_info.month < 6)
			{
				// todo: add enderi in leapyears
				sprintf (buf + strlen (buf),
					"The Feastday of Loende in %d days.#0",
					((5 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 7
				|| (time_info.month == 7 && time_info.day <= 11))
			{
				sprintf (buf + strlen (buf), "Eruhantale in %d days.#0",
					((7 - time_info.month) * 30) + (11 - time_info.day));
			}
			else if (time_info.month < 9)
			{
				sprintf (buf + strlen (buf),
					"The Feastday of Yaviere in %d days.#0",
					((8 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 10
				|| (time_info.month == 10 && time_info.day <= 12))
			{
				sprintf (buf + strlen (buf), "The Hallowmas in %d days.#0",
					((10 - time_info.month) * 30) + (12 - time_info.day));
			}
			else
			{
				sprintf (buf + strlen (buf),
					"The Feastdays of Mettare & Yestare in %d days.#0",
					((12 - time_info.month) * 30) + (30 - time_info.day));
			}
		}
	}
	reformat_string (buf, &p);

	send_to_char ("\n", ch);
	send_to_char (p, ch);

	free_mem (p); //char*
	p = NULL;

	if (GET_TRUST (ch))
	{
		send_to_char ("\nThe following variables apply:\n", ch);

		if (IS_LIGHT (ch->room))
			send_to_char ("   IS_LIGHT is true.\n", ch);
		else
			send_to_char ("   IS_LIGHT is false.\n", ch);

		if (IS_OUTSIDE (ch))
			send_to_char ("   OUTSIDE is true.\n", ch);
		else
			send_to_char ("   OUTSIDE is false.\n", ch);

		if (IS_NIGHT)
			send_to_char ("   IS_NIGHT is true.\n", ch);
		else
			send_to_char ("   IS_NIGHT is false.\n", ch);

		send_to_char ((global_moon_light) ? "   Global Moon is true.\n" :
			"   Global Moon is false.\n", ch);
		send_to_char ((moon_light[ch->room->zone]) ? "   Zone Moon is true.\n" :
			"   Zone Moon is false.\n", ch);

		sprintf (buf, "   Light count in room:  %d\n", ch->room->light);
		send_to_char (buf, ch);
	}
}

void
do_weather (CHAR_DATA * ch, char *argument, int cmd)
{
	int sunrise[] = { 6, 6, 6, 6, 5, 5, 4, 4, 5, 6, 6, 7 };
	int sunset[] = { 18, 18, 19, 19, 20, 21, 22, 22, 21, 20, 19, 18 };
	int ind = 0;
	char w_phrase[AVG_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char imm_buf[MAX_STRING_LENGTH];
	int wind_case = 0;
	int temp_case = 0;
	char wind[AVG_STRING_LENGTH] = { '\0' };
	int high_sun = 0;
	AFFECTED_TYPE *room_af = NULL;
	extern AFFECTED_TYPE *world_affects;
	int i = 0;


	argument = one_argument (argument, buf);

	/*** Start set weather ***/
	if(!strcmp("set", buf))
	{
		argument = one_argument (argument, buf);

		if(!strcmp("?", buf))
		{
			send_to_char("The following weather states are available:\n\n", ch);

			/* Cycle through each struct and show options */
			sprintf (buf,"     Fog States\n");
			for (i = 0; *fog_states[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					fog_states[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Precipitation States\n");
			for (i = 0; *weather_states[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					weather_states[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Cloud Covers\n");
			for (i = 0; *weather_clouds[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					weather_clouds[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Wind Speeds\n");
			for (i = 0; *wind_speeds[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					wind_speeds[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Wind Directions\n");
			for (i = 0; *wind_directions[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					wind_directions[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Special Effects\n");
			for (i = 0; *special_effects[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					special_effects[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			page_string(ch->desc, buf);

			return;
		}

		if (!IS_MORTAL (ch) && GET_TRUST (ch) > 3 && *buf)
		{
			if ((ind = index_lookup (weather_states, buf)) != -1)
			{
				sprintf (buf, "You have changed the precipitation state to #6%s#0.\n",
					weather_states[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].state = ind;
				return;
			}
			else if ((ind = index_lookup (weather_clouds, buf)) != -1)
			{
				sprintf (buf, "You have changed cloud state to #6%s#0.\n",
					weather_clouds[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].clouds = ind;
				return;
			}
			else if ((ind = index_lookup (wind_speeds, buf)) != -1)
			{
				sprintf (buf, "You have changed wind speed to #6%s#0.\n",
					wind_speeds[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].wind_speed = ind;
				return;
			}
			else if ((ind = index_lookup (fog_states, buf)) != -1)
			{
				sprintf (buf, "You have changed the fog level to #6%s#0.\n",
					fog_states[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].fog = ind;
				return;
			}
			else if((ind = index_lookup (wind_directions, buf)) != -1)
			{
				sprintf (buf, "You have changed the wind direction to #6%s#0.\n",
					wind_directions[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].wind_dir = ind;
				return;
			}
			else if((ind = index_lookup (special_effects, buf)) != -1)
			{
				sprintf (buf, "You have changed the special effect to #6%s#0.\n",
					special_effects[ind]);
				send_to_char (buf, ch);
				weather_info[ch->room->zone].special_effect = ind;
				return;
			}
			send_to_char ("That is not a recognized weather state.\n", ch);
			return;
		}
	}
	/*** End set weather***/

	int weather_echo_state = 0; // 0 = outdoors, 1 = indoors, 2 = deep indoors
	if (!IS_OUTSIDE(ch))
	{
		for (int exit = 0; exit < 6; exit++)
		{
			if (EXIT(ch, exit) && (EXIT(ch, exit)->to_room != -1))
			{
				ROOM_DATA * room = vtor(EXIT(ch, exit)->to_room);
				if (!room)
					continue;

				if (!IS_SET(room->room_flags, INDOORS) && room->sector_type != SECT_INSIDE && room->sector_type != SECT_UNDERWATER)
				{
					weather_echo_state = 1;
					break;
				}
			}
		}
		if (!weather_echo_state)
			weather_echo_state = 2;
	}

	if (GET_TRUST(ch))
		weather_echo_state = 0;

	if (weather_info[ch->room->zone].temperature < 120)
	{
		wind_case = 0;
		temp_case = 1;
	}
	if (weather_info[ch->room->zone].temperature < 110)
	{
		temp_case = 2;
		wind_case = 1;
	}
	if (weather_info[ch->room->zone].temperature < 100)
	{
		wind_case = 2;
		temp_case = 3;
	}
	if (weather_info[ch->room->zone].temperature < 94)
	{
		wind_case = 3;
		temp_case = 3;
	}
	if (weather_info[ch->room->zone].temperature < 90)
		temp_case = 4;
	if (weather_info[ch->room->zone].temperature < 80)
	{
		wind_case = 4;
		temp_case = 5;
	}
	if (weather_info[ch->room->zone].temperature < 75)
		temp_case = 5;
	if (weather_info[ch->room->zone].temperature < 65)
	{
		wind_case = 5;
		temp_case = 6;
	}
	if (weather_info[ch->room->zone].temperature < 55)
	{
		temp_case = 7;
		wind_case = 6;
	}
	if (weather_info[ch->room->zone].temperature < 47)
	{
		wind_case = 7;
		temp_case = 8;
	}
	if (weather_info[ch->room->zone].temperature < 38)
		temp_case = 9;
	if (weather_info[ch->room->zone].temperature < 33)
	{
		wind_case = 8;
		temp_case = 10;
	}
	if (weather_info[ch->room->zone].temperature < 21)
	{
		wind_case = 9;
		temp_case = 11;
	}
	if (weather_info[ch->room->zone].temperature < 11)
		temp_case = 12;
	if (weather_info[ch->room->zone].temperature < 1)
	{
		wind_case = 10;
		temp_case = 13;
	}
	if (weather_info[ch->room->zone].temperature < -10)
		temp_case = 14;

	*buf = '\0';
	*buf2 = '\0';

	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	if (time_info.hour >= sunrise[time_info.month]
	&& time_info.hour <= high_sun - 2)
		sprintf (w_phrase, "morning");
	else if (time_info.hour > high_sun - 2
		&& time_info.hour <= high_sun + 1)
		sprintf (w_phrase, "day");
	else if (time_info.hour > high_sun + 1
		&& time_info.hour < sunset[time_info.month])
		sprintf (w_phrase, "afternoon");
	else if (time_info.hour >= sunset[time_info.month]
	&& time_info.hour < 21)
		sprintf (w_phrase, "evening");
	else
		sprintf (w_phrase, "night");

	if (time_info.season == SPRING)
		sprintf (buf2, "spring ");
	if (time_info.season == SUMMER)
		sprintf (buf2, "summer ");
	if (time_info.season == AUTUMN)
		sprintf (buf2, "autumn ");
	if (time_info.season == WINTER)
		sprintf (buf2, "winter ");
	if (weather_echo_state != 0)
		buf2[0] = '\0';

	*imm_buf = '\0';

	if (!IS_MORTAL (ch))
		sprintf (imm_buf, " [%d F]",
		weather_info[ch->room->zone].temperature);

	switch (temp_case)
	{
	case 0:
		sprintf (buf, "It is a dangerously searing %s%s%s.\n", buf2,
			w_phrase, imm_buf);
		break;
	case 1:
		sprintf (buf, "It is a painfully blazing %s%s%s.\n", buf2,
			w_phrase, imm_buf);
		break;
	case 2:
		sprintf (buf, "It is a blistering %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 3:
		sprintf (buf, "It is a sweltering %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 4:
		sprintf (buf, "It is a hot %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	case 5:
		sprintf (buf, "It is a temperate %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 6:
		sprintf (buf, "It is a cool %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	case 7:
		sprintf (buf, "It is a chill %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	case 8:
		sprintf (buf, "It is a cold %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	case 9:
		sprintf (buf, "It is a very cold %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 10:
		sprintf (buf, "It is a frigid %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	case 11:
		sprintf (buf, "It is a freezing %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 12:
		sprintf (buf, "It is a numbingly frigid %s%s%s.\n", buf2, w_phrase,
			imm_buf);
		break;
	case 13:
		sprintf (buf, "It is a painfully freezing %s%s%s.\n", buf2,
			w_phrase, imm_buf);
		break;
	case 14:
		sprintf (buf, "It is a dangerously freezing %s%s%s.\n", buf2,
			w_phrase, imm_buf);
		break;

	default:
		sprintf (buf, "It is a cool %s%s%s.\n", buf2, w_phrase, imm_buf);
		break;
	}

	send_to_char (buf, ch);

	*buf = '\0';
	*buf2 = '\0';

	if (weather_info[ch->room->zone].state >= LIGHT_RAIN)
	{
		if (weather_info[ch->room->zone].wind_speed == STORMY)
			sprintf (w_phrase, "rolling");
		else if (weather_info[ch->room->zone].wind_speed > BREEZE)
			sprintf (w_phrase, "flowing");
		else
			sprintf (w_phrase, "brooding");

		switch (weather_info[ch->room->zone].clouds)
		{
		case LIGHT_CLOUDS:
			sprintf (buf2, "a scattering of grey clouds.\n");
			break;
		case HEAVY_CLOUDS:
			sprintf (buf2, "dark, %s clouds.\n", w_phrase);
			break;
		case OVERCAST:
			sprintf (buf2, "a thick veil of %s storm clouds.\n", w_phrase);
			break;
		}

		if (weather_info[ch->room->zone].fog == THICK_FOG)
			sprintf (buf2, " the fog-shrouded sky.\n");

		switch (weather_info[ch->room->zone].state)
		{
		case LIGHT_RAIN:
			sprintf (buf, "A light drizzle is falling from %s", buf2);
			break;
		case STEADY_RAIN:
			sprintf (buf, "A steady rain is falling from %s", buf2);
			break;
		case HEAVY_RAIN:
			sprintf (buf, "A shower of rain is pouring from %s", buf2);
			break;
		case LIGHT_SNOW:
			sprintf (buf, "A light snow is falling from %s", buf2);
			break;
		case STEADY_SNOW:
			sprintf (buf, "Snow is falling from %s", buf2);
			break;
		case HEAVY_SNOW:
			sprintf (buf,
				"Blinding snow swarms down from an obscured white sky.\n");
			break;
		}
		if (!weather_echo_state)
			send_to_char (buf, ch);
	}

	*buf = '\0';
	*buf2 = '\0';

	if (weather_info[ch->room->zone].wind_speed)
	{
		if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
			wind_case++;

		switch (wind_case)
		{

		case 0:
			sprintf (buf, "searing");
			break;
		case 1:
			sprintf (buf, "scorching");
			break;
		case 2:
			sprintf (buf, "sweltering");
			break;
		case 3:
			sprintf (buf, "hot");
			break;
		case 4:
			sprintf (buf, "warm");
			break;
		case 5:
			sprintf (buf, "cool");
			break;
		case 6:
			sprintf (buf, "chill");
			break;
		case 7:
			sprintf (buf, "cold");
			break;
		case 8:
			sprintf (buf, "frigid");
			break;
		case 9:
			sprintf (buf, "freezing");
			break;
		case 10:
			sprintf (buf, "arctic");
			break;

		default:
			sprintf (buf, "cool");
			break;
		}

		if (weather_echo_state == 1)
			sprintf (wind, "%s", buf);
		else if (weather_info[ch->room->zone].wind_dir == NORTH_WIND)
			sprintf (wind, "%s northerly", buf);
		else if(weather_info[ch->room->zone].wind_dir == NORTHEAST_WIND)
			sprintf (wind, "%s north easterly", buf);
		else if(weather_info[ch->room->zone].wind_dir == EAST_WIND)
			sprintf (wind, "%s easterly", buf);
		else if(weather_info[ch->room->zone].wind_dir == SOUTHEAST_WIND)
			sprintf (wind, "%s south easterly", buf);
		else if(weather_info[ch->room->zone].wind_dir == SOUTH_WIND)
			sprintf (wind, "%s southerly", buf);
		else if(weather_info[ch->room->zone].wind_dir == SOUTHWEST_WIND)
			sprintf (wind, "%s south westerly", buf);
		else if(weather_info[ch->room->zone].wind_dir == NORTHWEST_WIND)
			sprintf (wind, "%s north westerly", buf);
		else
			sprintf (wind, "%s westerly", buf);
	}

	*buf = '\0';
	*buf2 = '\0';

	if (weather_echo_state == 1 || weather_info[ch->room->zone].state >= LIGHT_RAIN
		|| !weather_info[ch->room->zone].clouds
		|| weather_info[ch->room->zone].fog == THICK_FOG)
	{
		switch (weather_info[ch->room->zone].wind_speed)
		{
		case CALM:
			sprintf (buf, "The air is calm and quiet");
			break;
		case BREEZE:
			sprintf (buf, "There is a %s breeze", wind);
			break;
		case WINDY:
			sprintf (buf, "There is a %s wind", wind);
			break;
		case GALE:
			sprintf (buf, "A %s gale is blowing", wind);
			break;
		case STORMY:
			sprintf (buf, "A %s wind whips and churns in a stormy fury",
				wind);
			break;
		}
		if (weather_echo_state == 1)
			sprintf (buf+strlen(buf), " outside");
		sprintf(buf+strlen(buf), ".\n");
		if (weather_echo_state < 2)
			send_to_char (buf, ch);
	}

	*buf = '\0';
	*buf2 = '\0';

	if (weather_info[ch->room->zone].state < LIGHT_RAIN
		&& weather_info[ch->room->zone].clouds
		&& weather_info[ch->room->zone].fog < THICK_FOG)
	{
		if (weather_info[ch->room->zone].state == NO_RAIN)
			sprintf (w_phrase, "rain");
		else
			sprintf (w_phrase, "white");

		switch (weather_info[ch->room->zone].clouds)
		{
		case LIGHT_CLOUDS:
			sprintf (buf2, "Wispy %s clouds", w_phrase);
			break;
		case HEAVY_CLOUDS:
			sprintf (buf2, "Heavy %s clouds", w_phrase);
			break;
		case OVERCAST:
			sprintf (buf2, "A veil of thick %s clouds", w_phrase);
			break;
		}

		switch (weather_info[ch->room->zone].wind_speed)
		{
		case CALM:
			sprintf (buf, "%s hang motionless in the sky.\n", buf2);
			break;
		case BREEZE:
			sprintf (buf, "%s waft overhead upon a %s breeze.\n", buf2,
				wind);
			break;
		case WINDY:
			sprintf (buf, "%s waft overhead upon the %s winds.\n", buf2,
				wind);
			break;
		case GALE:
			sprintf (buf, "%s rush overhead upon a %s gale.\n", buf2, wind);
			break;
		case STORMY:
			sprintf (buf,
				"%s churn violently in the sky upon the %s winds.\n",
				buf2, wind);
			break;
		}
		if (!weather_echo_state)
			send_to_char (buf, ch);
	}

	*buf = '\0';
	*buf2 = '\0';
	if (!weather_echo_state && weather_info[ch->room->zone].fog
		&& !(weather_info[ch->room->zone].state >= LIGHT_RAIN
		&& weather_info[ch->room->zone].fog == THICK_FOG))
	{
		if (weather_info[ch->room->zone].fog == THIN_FOG)
			send_to_char ("A patchy fog floats in the air.\n", ch);
		else
			send_to_char ("A thick fog lies heavy upon the land.\n", ch);
	}

	if (!weather_echo_state && (room_af = is_room_affected (world_affects, MAGIC_WORLD_CLOUDS)) &&
		(IS_OUTSIDE (ch) || !IS_MORTAL (ch)))
	{
		send_to_char
			("Looming black clouds cover the sky, blotting out the sun.\n",
			ch);
	}

	/*** Special effects set by IMMs***/
	if (weather_info[ch->room->zone].special_effect != NO_EFFECT)
	{
		//show effect specific message
		if (weather_info[ch->room->zone].special_effect == VOLCANIC_SMOKE)
		{
			send_to_char
				("A cloud of thick, dust filled volcanic smoke drifts through the air.\n",
				ch);
		}
		else if (weather_info[ch->room->zone].special_effect == FOUL_STENCH)
		{
			send_to_char
				("A foul stench permeates the area.\n",
				ch);
		}
		else if (!weather_echo_state && weather_info[ch->room->zone].special_effect == LOW_MIST)
		{
			send_to_char
				("A low, eerie mist sits heavily upon the land.\n",
				ch);
		}
	}

	if (!weather_echo_state && moon_light[ch->room->zone] >= 1)
	{
		if (!sun_light)
			send_to_char
			("A full and gleaming Ithil limns the area in ghostly argent radiance.\n",
			ch);
		else
			send_to_char
			("Ithil's ethereal silhouette is barely visible in the daylight.\n",
			ch);
	}

	if (weather_echo_state == 2)
		send_to_char("You can glean no further details as you are too far from the outside world.\n", ch);
}

HELP_DATA *
is_help (CHAR_DATA * ch, HELP_DATA * list, char *topic)
{
	HELP_DATA *element = NULL;

	for (element = list; element; element = element->next)
	{

		if (!element->master_element)
			continue;

		if (strcasecmp (element->keyword, topic) == STR_MATCH)
			return element->master_element;
	}

	return NULL;
}

void
post_help (DESCRIPTOR_DATA * d)
{
	char date[32] = "";
	time_t current_time = 0;

	mysql_safe_query
		("DELETE FROM helpfiles WHERE category = \'%s\' AND name = \'%s\'",
		d->character->delay_who, d->character->delay_who2);

	current_time = time (0);
	ctime_r (&current_time, date);
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
	{
		send_to_char ("No help entry written.\n", d->character);
		d->pending_message = NULL;
		d->character->delay_who = NULL;
		d->character->delay_who2 = NULL;
		d->character->delay_info1 = 0;
		return;
	}

	*d->character->delay_who = toupper (*d->character->delay_who);
	*d->character->delay_who2 = toupper (*d->character->delay_who2);

	mysql_safe_query
		("INSERT INTO helpfiles VALUES (\'%s\', \'%s\', \'\n%s\n\', \'(null)\', \'%d\', \'%s\', \'%s\')",
		d->character->delay_who2, d->character->delay_who,
		d->pending_message->message, d->character->delay_info1, date,
		d->character->tname);

	d->pending_message = NULL;
	d->character->delay_who = NULL;
	d->character->delay_who2 = NULL;
	d->character->delay_info1 = 0;
}

void
do_hedit (CHAR_DATA * ch, char *argument, int cmd)
{
	char topic[MAX_STRING_LENGTH] = { '\0' };
	char subject[MAX_STRING_LENGTH] = { '\0' };
	char level[MAX_STRING_LENGTH] = { '\0' };
	int lvl = 0;

	argument = one_argument (argument, topic);
	argument = one_argument (argument, subject);
	argument = one_argument (argument, level);

	if (IS_NPC (ch))
	{
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	if (!*topic || !*subject || !*level)
	{
		send_to_char
			("You must specify the topic, subject, and required level for the help entry.\n",
			ch);
		return;
	}

	if (!isdigit (*level))
	{
		send_to_char
			("You must specify a number for the entry's required access level.\n",
			ch);
		return;
	}

	lvl = atoi (level);

	ch->delay_who = duplicateString (topic);
	ch->delay_who2 = duplicateString (subject);
	ch->delay_info1 = lvl;

	send_to_char ("Enter the text of this database entry:\n", ch);

	make_quiet (ch);

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_help;
}

void
do_vboards (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	char buf[MAX_STRING_LENGTH];
	int colnum = 1;

	*b_buf = '\0';

	mysql_safe_query
		("SELECT board_name FROM virtual_boards GROUP BY board_name ORDER BY board_name ASC");
	result = mysql_store_result (database);

	while ((row = mysql_fetch_row (result)))
	{
		if (colnum == 1)
			sprintf (b_buf + strlen (b_buf), "%-29s", row[0]);
		else if (colnum == 2)
			sprintf (b_buf + strlen (b_buf), "%-29s", row[0]);
		else if (colnum == 3)
		{
			sprintf (b_buf + strlen (b_buf), "%s\n", row[0]);
			colnum = 1;
			continue;
		}
		colnum++;
	}

	sprintf (buf, "#6                      %s Virtual Boards#0\n", MUD_NAME);
	send_to_char (buf, ch);

	if (!*b_buf)
		send_to_char ("None.\n", ch);
	else
		page_string (ch->desc, b_buf);

	if (colnum != 1)
		send_to_char ("\n", ch);

}

void
log_missing_helpfile (CHAR_DATA * ch, char *string)
{
	char msg[MAX_STRING_LENGTH] = { '\0' };
	char subj[MAX_STRING_LENGTH] = { '\0' };
	MYSQL_RES *result = NULL;

	mysql_safe_query
		("SELECT * FROM unneeded_helpfiles WHERE name = '%s' AND (datestamp > UNIX_TIMESTAMP()-60*60*24*30*6)",
		string);
	result = mysql_store_result (database);

	if (result && mysql_num_rows (result) >= 1)
	{
		mysql_free_result (result);
		return;
	}

	mysql_safe_query ("DELETE FROM unneeded_helpfiles WHERE name = \'%s\'",
		string);

	sprintf (subj, "#1Not Found:#0 %s", string);
	sprintf (msg, "%s: help %s\n", ch->tname, string);

	add_message (1, "Helpfiles", -5, ch->tname, 0, subj, "", msg, 0);

	mysql_safe_query ("INSERT INTO unneeded_helpfiles VALUES (\'%s\', %d)",
		string, (int) time (0));
}


int
number_of_helpfiles_available (int player_level)
{
	// Build a query to get the number of helpfiles for that level
	int number_of_helpfiles = 0;
	std::ostringstream query_stream;
	query_stream << "SELECT COUNT(*) AS num_helpfiles "
		<< "FROM helpfiles "
		<< "WHERE required_level <= "
		<< player_level;

	std::string query = query_stream.str ();
	if ((mysql_safe_query ((char *)query.c_str())) == 0)
	{
		MYSQL_RES *result = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			MYSQL_ROW row = 0;
			if ((row = mysql_fetch_row (result)) != 0)
			{
				number_of_helpfiles = strtol (row[0],0,10);
			}
			mysql_free_result (result);
		}
		else
		{
			std::string error_message =
				"number_of_helpfiles_available: "
				"Couldn't get a helpfile count because: ";
			error_message += mysql_error (database);

			std::cerr << error_message << std::endl;
			system_log (error_message.c_str (), true);
		}
	}
	else
	{
		std::string error_message =
			"number_of_helpfiles_available: "
			"'mysql_safe_query' failed for the following reason: ";
		error_message += mysql_error (database);

		std::cerr << error_message << std::endl;
		system_log (error_message.c_str (), true);
	}

	return number_of_helpfiles;
}

std::string
output_categories_available (int player_level)
{
	std::string category_list;

	// Query a list of categories from the helpfiles table
	std::ostringstream query_stream;
	query_stream << "SELECT category "
		<< "FROM helpfiles "
		<< "WHERE required_level <= "
		<< player_level << ' '
		<< "GROUP BY category ASC";

	std::string query = query_stream.str ();
	if ((mysql_safe_query ((char *)query.c_str ())) == 0)
	{
		// Store a formatted table of categories
		MYSQL_RES *result = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			char category[512];
			int category_counter = 0;
			MYSQL_ROW row = 0;
			while ((row = mysql_fetch_row (result)))
			{
				sprintf (category, "#6%-18.18s#0 ", row[0]);
				category_list += category;
				if (!(category_counter % 4))
				{
					category_list += '\n';
				}
				category_counter++;
			}
			if ((category_counter % 4) != 1)
			{
				category_list += '\n';
			}
			mysql_free_result (result);
		}
		else
		{
			std::string error_message =
				"output_categories_available: "
				"Couldn't create a category list because: ";
			error_message += mysql_error (database);
			std::cerr << error_message << std::endl;
			system_log (error_message.c_str (), true);
		}
	}
	else
	{
		std::string error_message =
			"output_categories_available: "
			"'mysql_safe_query' failed to query shadows.helpfiles because: ";
		error_message += mysql_error (database);

		std::cerr << error_message << std::endl;
		system_log (error_message.c_str (), true);
	}

	return category_list;
}

void
do_help (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char example[MAX_STRING_LENGTH] = { '\0' };
	char original[MAX_STRING_LENGTH] = { '\0' };
	bool header_needed = false, category_list = false, soundex = false;
	int j = 1;
	int player_level = 0;

	result = NULL;

	*arg1 = '\0';
	*arg2 = '\0';
	*b_buf = '\0';
	*example = '\0';

	if (strstr (argument, ":"))
	{
		send_to_char ("Please see #6HELP HELP#0 for proper syntax and usage.\n",
			ch);
		return;
	}

	sprintf (original, "%s", argument);

	argument = one_argument (argument, arg1);

	if (*argument)
		argument = one_argument (argument, arg2);

	// Mortals and NPCs get Level 0 Help
	char level_condition[16];
	if (IS_MORTAL (ch) || IS_NPC (ch))
	{
		player_level = 0;
		sprintf (level_condition, "= 0");
	}
	else
	{
		player_level = ch->pc->level;
		sprintf (level_condition, "<= %d", ch->pc->level);
	}

	if (!*arg1)
	{
		int help_available = number_of_helpfiles_available (player_level);
		std::string category_list = output_categories_available (player_level);

		// Query one helpfile name at random
		sprintf (buf,
			"SELECT name "
			"FROM helpfiles "
			"WHERE required_level %s "
			"ORDER BY RAND() DESC "
			"LIMIT 1", level_condition);

		if ((mysql_safe_query (buf)) !=0)
		{
			std::cerr << "The library call 'mysql_safe_query' failed to "
				<< "query shadows.helpfiles for the following reason: "
				<< mysql_error (database) << std::endl;
			return;
		}

		// Store the helpfile name
		*buf = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			if ((row = mysql_fetch_row (result)))
			{
				sprintf (buf, "Helpfile of the Moment: #6%s#0", row[0]);
			}
			mysql_free_result (result);
		}
		else
		{
			sprintf (buf, "do_help: "
				"Couldn't create a category list because: %s\n",
				mysql_error (database));
			std::cerr << buf;
			system_log (buf, true);
		}

		// Spit out a generic syntax message
		sprintf (b_buf,
			"\n                      #6%s Help Database#0\n\n"
			"There are currently #6%d#0 helpfiles accessible to you"
			" in our database.\n\n"
			"Helpfile Categories:\n\n%s\n%s\n\n"
			"To see a full list of helpfiles within these topics,"
			" use \'#6help on <topic name>#0\'.\n\n"
			"For a detailed description of HELP searching syntax,"
			" please see \'#6help help#0\'.\n\n"
			"Our helpfiles on the Web:"
			" #6http://www.middle-earth.us/index.php?display=help#0.\n",
			MUD_NAME,
			help_available,
			category_list.empty () ? "#6None#0\n" : category_list.c_str (),
			buf);

		page_string (ch->desc, b_buf);
		return;
	}

	if (strcasecmp (arg1, "named") == STR_MATCH && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
		arg2, ch->pc ? ch->pc->level : 0);
	else if (strcasecmp (arg1, "containing") == STR_MATCH && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE (entry LIKE '%%%s%%' OR related_entries LIKE '%%%s%%') AND required_level <= %d ORDER BY category,name ASC",
		arg2, arg2, ch->pc ? ch->pc->level : 0);
	else if (strcasecmp (arg1, "on") == STR_MATCH && *arg2)
	{
		category_list = true;
		mysql_safe_query
			("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
			arg2, ch->pc ? ch->pc->level : 0);
	}
	else if (*arg1 && !*arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name LIKE '%s%%' AND required_level <= %d ORDER BY category,name ASC",
		arg1, ch->pc ? ch->pc->level : 0);
	else if (*arg1 && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name = '%s' AND (category = '%s' AND required_level <= %d) ORDER BY category,name ASC",
		arg2, arg1, ch->pc ? ch->pc->level : 0);

	result = mysql_store_result (database);

	if (!result)
	{
		send_to_char
			("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
			ch);
		log_missing_helpfile (ch, original);
		return;
	}

	if (mysql_num_rows (result) == 0)
	{
		result = NULL;
		mysql_safe_query
			("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
			arg1, ch->pc ? ch->pc->level : 0);
		result = mysql_store_result (database);
		if (!result || mysql_num_rows (result) == 0)
		{
			mysql_safe_query
				("SELECT * FROM helpfiles WHERE SOUNDEX(category) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
				arg1, ch->pc ? ch->pc->level : 0);
			result = mysql_store_result (database);
			if (!result || mysql_num_rows (result) == 0)
			{
				mysql_safe_query
					("SELECT * FROM helpfiles WHERE SOUNDEX(name) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
					arg1, ch->pc ? ch->pc->level : 0);
				result = mysql_store_result (database);
				if (!result || mysql_num_rows (result) == 0)
				{
					mysql_safe_query
						("SELECT * FROM helpfiles WHERE related_entries LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
						arg1, ch->pc ? ch->pc->level : 0);
					result = mysql_store_result (database);
					if (!result || mysql_num_rows (result) == 0)
					{
						mysql_safe_query
							("SELECT * FROM helpfiles WHERE entry LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
							arg1, ch->pc ? ch->pc->level : 0);
						result = mysql_store_result (database);
						if (result || mysql_num_rows (result) > 0)
							category_list = true;
					}
				}
				else
					soundex = true;
			}
			else
				category_list = true;
		}
		else
			category_list = true;
	}
	if (mysql_num_rows (result) == 1)
	{
		row = mysql_fetch_row (result);
		if (!row || GET_TRUST (ch) < atoi (row[4]))
		{
			if (!mysql_num_rows (result))
			{
				if (result != NULL)
					mysql_free_result (result);
				send_to_char
					("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
					ch);
				mysql_free_result (result);
				log_missing_helpfile (ch, original);
				return;
			}
		}
		if (mysql_num_rows (result) == 1)
		{
			sprintf (b_buf, "\n#6%s: %s#0\n", row[1], row[0]);
			if (*row[2] != '\n')
				strcat (b_buf, "\n");
			sprintf (b_buf + strlen (b_buf), "%s", row[2]);
			if (*row[3] && strcasecmp (row[3], "(null)") != STR_MATCH)
				sprintf (b_buf + strlen (b_buf), "#6See Also:#0 %s\n\n", row[3]);
			sprintf (b_buf + strlen (b_buf), "#6Last Updated:#0 %s, by %s\n",
				row[5], CAP (row[6]));
		}
	}

	if (mysql_num_rows (result) > 1)
	{
		header_needed = true;
		while ((row = mysql_fetch_row (result)))
		{
			if (GET_TRUST (ch) < atoi (row[4]))
				continue;
			if (header_needed)
			{
				if (!soundex)
					sprintf (b_buf,
					"\nYour query matched the following helpfiles:\n\n   ");
				else
					sprintf (b_buf,
					"\nThe following entries most closely matched your spelling:\n\n   ");
				header_needed = false;
			}
			if (!category_list)
				sprintf (arg2, "%13s: %s", row[1], row[0]);
			else
				sprintf (arg2, "%s", row[0]);
			if (!category_list)
			{
				if (j == 1)
				{
					sprintf (arg1, "%3d. #6%s#0", j, arg2);
					sprintf (example, "%s", row[0]);
					if (!strchr (example, ' '))
						sprintf (example,
						"\nTo pull up the first entry listed, type \'#6help %s %s#0\'.\n",
						LOW (row[1]), LOW (row[0]));
					else
						sprintf (example,
						"\nTo pull up the first entry listed, type \'#6help %s \"%s\"#0\'.\n",
						LOW (row[1]), row[0]);
				}
				else
					sprintf (arg1, "\n   %3d. #6%s#0", j, arg2);
			}
			else
				sprintf (arg1, "#6%-22.22s#0   ", arg2);
			if (!(j % 3) && category_list)
				strcat (arg1, "\n   ");
			j++;
			sprintf (b_buf + strlen (b_buf), "%s", arg1);
		}
		if ((((j - 1) % 3) && !header_needed) || !category_list)
			strcat (b_buf, "\n");
		if (*example)
			sprintf (b_buf + strlen (b_buf), "%s", example);
	}

	mysql_free_result (result);
	result = NULL;

	if (*b_buf)
		page_string (ch->desc, b_buf);
	else
	{
		send_to_char
			("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n",
			ch);
		log_missing_helpfile (ch, original);
		return;
	}
}

void
do_users (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d = NULL;
	CHAR_DATA *tch = NULL;
	char colon_char = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char line[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };

	strcpy (buf, "\n#6Users Currently Connected:\n"
		"#6--------------------------\n\n#0");

	if (maintenance_lock)
		sprintf (buf + strlen (buf),
		"#2We are currently closed for maintenance.\n\n#0");

	if (pending_reboot)
		sprintf (buf + strlen (buf),
		"#2There is currently a reboot pending.\n\n#0");

	send_to_char (buf, ch);

	for (d = descriptor_list; d; d = d->next)
	{

		colon_char = ':';

		*buf2 = '\0';
		*buf3 = '\0';
		*buf4 = '\0';

		if (d->character && !d->character->desc)
			continue;

		if (d->character && d->original)
			tch = d->original;
		else
			tch = d->character;

		if (tch && tch->pc->level)
			colon_char = '*';

		if (tch && d->acct)
		{
			sprintf (line, "%s  %c ", tch->pc->account_name, colon_char);
			sprintf (line + strlen (line), " #2[%s]#0 ", tch->tname);
			sprintf (line + strlen (line), "#3[%s]#0 ", d->strClientHostname);
			sprintf (line + strlen (line), "#5[%d]#0", tch->in_room);
			if (tch->pc->create_state == STATE_DIED)
				sprintf (line + strlen (line), " #1(Dead)#0");
			if (d->idle)
				sprintf (line + strlen (line), " #4(Idle)#0");
			if (IS_SET (tch->act, PLR_QUIET))
				sprintf (line + strlen (line), " #5(Edit)#0");
			if (IS_SET (tch->plr_flags, NEW_PLAYER_TAG))
				sprintf (line + strlen (line), " #2(New)#0");
			sprintf (line + strlen (line), "\n");
		}

		else
			continue;

		if (strlen (output) + strlen (line) > MAX_STRING_LENGTH)
			break;

		sprintf (output + strlen (output), "%s", line);
	}

	page_string (ch->desc, output);
}

char *
worn_first_loc (OBJ_DATA * obj)
{
	if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
		return "around your neck";
	else if (obj->location == WEAR_BODY)
		return "on your body";
	else if (obj->location == WEAR_HEAD)
		return "on your head";
	else if (obj->location == WEAR_ARMS)
		return "on your arms";
	else if (obj->location == WEAR_ABOUT)
		return "about your body";
	else if (obj->location == WEAR_WAIST)
		return "around your waist";
	else if (obj->location == WEAR_WRIST_L)
		return "on your left wrist";
	else if (obj->location == WEAR_WRIST_R)
		return "on your right wrist";
	else if (obj->location == WEAR_ANKLE_L)
		return "on your left ankle";
	else if (obj->location == WEAR_ANKLE_R)
		return "on your right ankle";
	else if (obj->location == WEAR_HAIR)
		return "in your hair";
	else if (obj->location == WEAR_FACE)
		return "on your face";
	else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
		return "on your belt";
	else if (obj->location == WEAR_BACK)
		return "on your back";
	else if (obj->location == WEAR_THROAT)
		return "around your throat";
	else if (obj->location == WEAR_BLINDFOLD)
		return "as a blindfold";
	else if (obj->location == WEAR_EAR)
		return "on your ear";
	else if (obj->location == WEAR_SHOULDER_R)
		return "over your right shoulder";
	else if (obj->location == WEAR_SHOULDER_L)
		return "over your left shoulder";
	else if (obj->location == WEAR_FEET)
		return "on your feet";
	else if (obj->location == WEAR_FINGER_R)
		return "on your right ring finger";
	else if (obj->location == WEAR_FINGER_L)
		return "on your left ring finger";
	else if (obj->location == WEAR_ARMBAND_R)
		return "around your upper right arm";
	else if (obj->location == WEAR_ARMBAND_L)
		return "around your upper left arm";
	else if (obj->location == WEAR_LEGS)
		return "on your legs";
	else if (obj->location == WEAR_HANDS)
		return "on your hands";

	else
		return "#1in an unknown location#0";
}

void
do_equipment (CHAR_DATA * ch, char *argument, int cmd)
{
	int location = 0;
	int found = false;
	OBJ_DATA *eq = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';
	if (ch->right_hand)
	{
		if (ch->right_hand->location == WEAR_PRIM
			|| ch->right_hand->location == WEAR_SEC)
			sprintf (buf, "<wielded in right hand>  ");
		else if (ch->right_hand->location == WEAR_BOTH)
			sprintf (buf, "<wielded in both hands>  ");
		else if (ch->right_hand->location == WEAR_SHIELD)
			sprintf (buf, "<gripped in right hand>  ");
		else
			sprintf (buf, "<carried in right hand>  ");

		send_to_char (buf, ch);
		show_obj_to_char (ch->right_hand, ch, 1);
	}
	if (ch->left_hand)
	{
		if (ch->left_hand->location == WEAR_PRIM
			|| ch->left_hand->location == WEAR_SEC)
			sprintf (buf, "<wielded in left hand>   ");
		else if (ch->left_hand->location == WEAR_BOTH)
			sprintf (buf, "<wielded in both hands>  ");
		else if (ch->left_hand->location == WEAR_SHIELD)
			sprintf (buf, "<gripped in left hand>   ");
		else
			sprintf (buf, "<carried in left hand>   ");

		send_to_char (buf, ch);
		show_obj_to_char (ch->left_hand, ch, 1);
	}

	if (ch->left_hand || ch->right_hand)
		send_to_char ("\n", ch);

	for (location = 0; location < MAX_WEAR; location++)
	{

		if (!(eq = get_equip (ch, loc_order[location])))
			continue;

		if (eq == ch->right_hand || eq == ch->left_hand)
			continue;

		send_to_char (where[loc_order[location]], ch);

		if (location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq))
			show_obj_to_char (eq, ch, 1);
		else
			send_to_char ("#2something#0\n", ch);

		found = true;
	}

	if (!found)
		send_to_char ("You are naked.\n", ch);

}

void
do_news (CHAR_DATA * ch, char *argument, int cmd)
{

	std::string msg_line;
	std::string output;

	std::ifstream fin( "MOTD" );

	if( !fin )
	{
		system_log ("The MOTD could not be found", true);
		send_to_char("The MOTD could not be found", ch);
		return;
	}

	while( getline(fin, msg_line) )
	{
		output.append(msg_line);
	}

	fin.close();

	if (!output.empty())
	{
		send_to_char (output.c_str(), ch);
		send_to_char ("\n", ch);
	}

}

void
do_wizlist (CHAR_DATA * ch, char *argument, int cmd)
{
	page_string (ch->desc, get_text_buffer (ch, text_list, "wizlist"));
}

int
show_where_char (const CHAR_DATA * ch, int indent)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!ch || !ch->room || ch->in_room == NOWHERE || !ch->short_descr
		|| !ch->tname)
		return 0;

	memset (buf, ' ', indent * 3);
	buf[indent * 3] = '\0';

	sprintf (buf + strlen (buf), "%s",
		IS_NPC (ch) ? ch->short_descr : ch->tname);

	if (IS_NPC (ch))
		sprintf (buf + strlen (buf), " (%d)", ch->mob->nVirtual);

	if (ch->in_room == -1)
		strcat (buf, " NOWHERE\n");
	else
		sprintf (buf + strlen (buf), " in room %d\n", ch->in_room);

	if (strlen (b_buf) > B_BUF_SIZE - 500)
		return 0;

	strcat (b_buf, buf);

	return 1;
}

int
show_where_obj (OBJ_DATA * obj, int indent)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!obj)
		return 0;

	memset (buf, ' ', indent * 3);
	buf[indent * 3] = '\0';

	sprintf (buf + strlen (buf), "%s (%d)",
		obj->short_description, obj->nVirtual);

	if (obj->in_room != -1)
		sprintf (buf + strlen (buf), " in room %d\n", obj->in_room);
	else if (obj->in_obj)
		sprintf (buf + strlen (buf), " in obj:\n");
	else if (obj->carried_by && obj->location == -1
		&& obj->carried_by->room != NULL)
		sprintf (buf + strlen (buf), " carried by:\n");
	else if (obj->equiped_by && obj->equiped_by->room != NULL)
		sprintf (buf + strlen (buf), " equipped by:\n");
	else
		sprintf (buf + strlen (buf), " ; not sure\n");

	strcat (b_buf, buf);

	if (strlen (b_buf) > B_BUF_SIZE - 500)
		return 0;

	if (obj->in_obj)
		show_where_obj (obj->in_obj, indent + 1);
	else if (obj->equiped_by)
		show_where_char (obj->equiped_by, indent + 1);
	else if (obj->carried_by)
		show_where_char (obj->carried_by, indent + 1);

	return 1;
}

void
do_find (CHAR_DATA * ch, char *argument, int cmd)
{
	int nots = 0;
	int musts = 0;
	int zone = -1;
	int nVirtual = -1;
	int type = -1;
	int i;
	OBJ_DATA *obj = NULL;
	char *not_list[AVG_STRING_LENGTH] = { '\0' };
	char *must_list[AVG_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("find +keyword     'keyword' must exist on obj\n", ch);
		send_to_char ("     -keyword     'keyword' must not exist on obj.\n",
			ch);
		send_to_char ("     zone         Only consider objects in this zone.\n",
			ch);
		send_to_char ("     vnum         Just find the specific object vnum.\n",
			ch);
		send_to_char ("\nExamples:\n\n", ch);
		send_to_char ("   > find +bag +leather 10\n", ch);
		send_to_char ("   > find 10001\n", ch);
		send_to_char ("   > find +sword -rusty 12\n", ch);
		return;
	}

	while (*buf)
	{

		if (*buf == '+')
			must_list[musts++] = duplicateString (buf + 1);

		else if (*buf == '-')
			not_list[nots++] = duplicateString (buf + 1);

		else if (isdigit (*buf))
		{
			if (atoi (buf) < 100)
				zone = atoi (buf);
			else
				nVirtual = atoi (buf);
		}

		else if ((type = index_lookup (item_types, buf)) != -1)
			;

		else
		{
			send_to_char ("Unknown keyword to where obj.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
	}

	*b_buf = '\0';

	for (obj = object_list; obj; obj = obj->next)
	{

		if (obj->deleted)
			continue;

		for (i = 0; i < musts; i++)
			if (!isname (must_list[i], obj->name))
				break;

		if (i != musts)		/* Got out of loop w/o all musts being there */
			continue;

		for (i = 0; i < nots; i++)
			if (isname (must_list[i], obj->name))
				break;

		if (i != nots)		/* Got out of loop w/o all nots not being thr */
			continue;

		if (nVirtual != -1 && obj->nVirtual != nVirtual)
			continue;

		if (type != -1 && obj->obj_flags.type_flag != type)
			continue;

		/* Zone is a little tricky to determine */

		if (!show_where_obj (obj, 0))
			break;
	}

	page_string (ch->desc, b_buf);
}

void
do_locate (CHAR_DATA * ch, char *argument, int cmd)
{
	int num_clans = 0;
	int i;
	int musts = 0;
	int nots = 0;
	bitflag act_bits = 0;
	int position = -1;
	int ind = 0;
	int nVirtual = 0;
	int zone = -1;
	int pc_only = 0;
	CHAR_DATA *mob = NULL;
	char *arg = '\0';
	char clan_names[10][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char not_list[50][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char must_list[50][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char acts[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("locate [+/-mob name]\n"
			"       [zone]\n"
			"       [virtual]\n"
			"       act \"<action-bits>\"\n"
			"       [clan name]\n"
			"       clan <clan name>\n" "       [position]\n", ch);
		return;
	}

	while (*buf)
	{

		if (*buf == '+')
			strcpy (must_list[musts++], buf + 1);

		else if (*buf == '-')
			strcpy (not_list[nots++], buf + 1);

		else if ((position = index_lookup (position_types, buf)) != -1)
			;

		else if (isdigit (*buf))
		{
			ind = atoi (buf);
			if (ind > 100)
				nVirtual = ind;
			else
				zone = ind;
		}

		else if (strcasecmp (buf, "act") == STR_MATCH)
		{

			argument = one_argument (argument, acts);

			arg = one_argument (acts, buf);

			while (*buf)
			{

				if ((ind = index_lookup (action_bits, buf)) == -1)
				{
					send_to_char ("No such action-bit:  ", ch);
					send_to_char (buf, ch);
					send_to_char ("\n", ch);
					return;
				}

				act_bits |= (1 << ind);

				arg = one_argument (arg, buf);
			}
		}

		else if (strcasecmp (buf, "clan") == STR_MATCH)
		{

			argument = one_argument (argument, buf);

			if ( !str_cmp (buf, "pc") ){
				argument = one_argument (argument, buf);
				pc_only++;
			}

			if (num_clans >= 9)
			{
				send_to_char ("Hey, 10 clans is enough!\n", ch);
				return;
			}

			strcpy (clan_names[num_clans++], buf);
		}

		else if (isalpha (*buf))
		{

			if (num_clans >= 9)
			{
				send_to_char ("Hey, 10 clans is enough!\n", ch);
				return;
			}

			strcpy (clan_names[num_clans++], buf);
		}

		else
		{
			send_to_char ("Unknown keyword:  ", ch);
			send_to_char (buf, ch);
			send_to_char ("\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
	}

	*b_buf = '\0';

	//for (mob = character_list; mob; mob = mob->next)
	for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;

		if (mob->deleted)
			continue;

		/* Check act bits against mob's act bits.  The act bits
		specified on the command line must all be in mob->act */

		if (IS_NPC(mob) && pc_only)
			continue;

		if (zone != -1 && mob->room->zone != zone)
			continue;

		if (nVirtual && (!mob->mob || mob->mob->nVirtual != nVirtual))
			continue;

		if (position != -1 && GET_POS (mob) != position)
			continue;

		if (act_bits && (act_bits & mob->act) != act_bits)
			continue;

		for (i = 0; i < num_clans; i++)
			if (is_clan_member (mob, clan_names[i]))
				break;

		if (num_clans && i >= num_clans)	/* Couldn't find a clan member */
			continue;

		for (i = 0; i < musts; i++)
			if (!isname (must_list[i], mob->name))
				break;

		if (i != musts)		/* Got out of loop w/o all musts being there */
			continue;

		for (i = 0; i < nots; i++)
			if (isname (not_list[i], mob->name))
				break;

		if (i != nots)		/* Got out of loop w/o all nots not being thr */
			continue;

		show_where_char (mob, 0);
	}

	page_string (ch->desc, b_buf);
}

void do_where (CHAR_DATA * ch, char *argument, int cmd) {
	Stringstack whereArg = argument;

	int RPP = -1; // Will be less than 0 if this is where aura
	bool checkPower = false;
	bool checkClan = false; // Wait until my clan update - Case
	std::string whichClan = "";

	std::multimap<int, std::string> whereMap;
	std::multimap<int, std::string>::iterator it;
	std::stringstream outputWhere;


	if (IS_MORTAL(ch)) {
		send_to_char ("What are you looking for?\n", ch);
		return;
	}

	// Check for 'aura'
	if (whereArg.pop() == "power") {
		checkPower = true;
	}

	for (descriptor_data *d = descriptor_list; d; d = d->next) {
		if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)) {
			if (!checkPower) {
				RPP = d->acct->get_rpp();
			}
			std::pair<int, std::string> reportPair = d->character->reportWhere(checkClan, RPP, whichClan);

			if (d->original) {
				std::stringstream animationStream;
				animationStream << " #0(#5" << d->original->tname
					<< "#0: #2" << d->original->room->nVirtual << "#0)";
				reportPair.second += animationStream.str();
			}
			whereMap.insert(reportPair);
		}
	}

	outputWhere << std::endl << std::endl;

	// Inorder traversal, with the key being the room number, puts it into room order
	for (it = whereMap.begin(); it != whereMap.end(); it++) {
		// Hibou - Truncating WHERE output, as requested by Kite
		if (it->second.size() > 80) {
			it->second = it->second.substr (0, it->second.find_last_of (" ", 80));
		}
		outputWhere << it->second << std::endl;
	}

	// Append the remainder of the where information
	outputWhere << std::endl << "#0Color key: #5Admin,#0 #6Admin PC,#0 #3Guide,#0 #2New PC,#0 #1In Melee,#0 #4In Water#0"
		<< std::endl << std::endl << "#0Flag key:" << std::endl
		<< "e - Editing, i - Idle, h - Hidden, s - Sleeping" << std::endl
		<< "L - Link Dead, U - Unconscious, X - Near Death" << std::endl
		<< "#1!#0 - Watch For Bad Behaviour, #2R#0 - Watch To Potentially Award" << std::endl
		<< "#3P#0 - Involved In A Plot" << std::endl;

	page_string (ch->desc, outputWhere.str().c_str());
}

//Will allow sphere counts in who to count by zones instead of clanning. - Vader
int kingdom_from_zone (CHAR_DATA *ch)
{
	int zone = ch->in_room / 1000;
	if (IS_SET(ch->flags, FLAG_GUEST))
		return 0;
	else if (zone == 4 || zone == 12 || zone == 51 || zone == 76)
		return 6;
	else if (zone == 23 || zone == 5 || zone == 6 || zone == 66 || zone == 9 || zone == 24)
		return 5;
	else if (zone == 41 || zone == 40)
		return 4;
	else if (zone == 73 || zone == 80 || zone == 81 || zone == 82)
		return 3;
	else if (zone == 45)
		return 2;
	else if (zone == 1  || zone == 2  || zone == 3  ||
		zone == 8  || zone == 10 || zone == 11 ||
		zone == 13 || zone == 14 || zone == 15 || zone == 18 ||
		zone == 19 || zone == 21 || zone == 22 || zone == 38 ||
		zone == 54 || zone == 70 || zone == 71 ||
		zone == 72 || zone == 74 || zone == 75 ||
		zone == 77 || zone == 78 || zone == 79 || zone == 96)
		return 1; 
	else return -1;
}

//This helper function counts how many players are in the room.
//Anytime a new room is reference with vtor, run this command. - Vader
// Err, no Vader, if it mattered so much, put it in vtor. This command ignores hidden people now - Case

void roomCount(ROOM_DATA *rd)
{
	if (rd == NULL) {
		return;
	}
	rd->occupants = 0;
	for (CHAR_DATA *tch = rd->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC(tch))
		{
			if (!IS_LINKDEAD(tch) && IS_MORTAL(tch) && !get_affect(tch, MAGIC_HIDDEN))
				rd->occupants++;
		}
		else if (tch->desc && tch->desc->original && !get_affect(tch, MAGIC_HIDDEN))
			rd->occupants++;
	}
}

std::string gatheringPlace(int room_num, std::string name)
{
	return gatheringPlaceCore(room_num, name, true);
}

std::string gatheringPlaceCore(int room_num, std::string name, bool colorise)
{
	std::stringstream placestream;

	ROOM_DATA* rd = vtor(room_num);
	roomCount(rd);

	int num_occu = rd->occupants;

	if (num_occu > 0)
	{
		placestream << "In " << name << ", there " << (num_occu == 1 ? "is " : "are ");

		/* color codes are ignored entirely when colorise is false, for web export */
		if (colorise)
			placestream << "#2";

		placestream << num_occu;

		/* color codes are ignored entirely when colorise is false, for web export */
		if (colorise)
			placestream << "#0";

		placestream <<  (num_occu == 1 ? " player." : " players.") << std::endl;
	}

	return placestream.str();

}

void do_who (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d = NULL;

	int mortals = 0, immortals = 0, guests = 0, clanCount = 0, clansphere = 0;

	std::stringstream whoStream;

	std::stringstream availableAdminsStream;
	availableAdminsStream << std::endl << "#2Available Staff#0:" << std::endl;
	bool availableAdmins = false;

	int sphere = kingdom_from_zone(ch); // 0 = Guest, 1 = Gondor, 2 = Northmen, 3 = Fahad Jafari, 4 = Orcs,  5 = Mordor, 6 = In Ithilien

	for (d = descriptor_list; d; d = d->next)
	{
		if (d->character && d->character->pc && d->connected == CON_PLYNG)
		{
			clansphere = kingdom_from_zone(d->character);

			if (clansphere)
				mortals++;
			else
				guests++;

			if((clansphere == sphere) && (!(d->character->pc->level > 0)))
				clanCount++;

			if(d->character->pc->level > 0)
			{
				immortals++;
				if(IS_SET( d->character->flags, FLAG_AVAILABLE ))
				{
					availableAdminsStream << d->character->tname << std::endl;
					availableAdmins = true;
				}

			}
		}
	}

	if (mortals < 1) {
		whoStream << std::endl << "There aren't any beings within Middle-earth. " << std::endl;
	}
	else if (mortals == 1) {
		whoStream << std::endl << "There is a single presence within Middle-earth. " << std::endl;
	}
	else {
		whoStream << std::endl << "There are #2" << mortals << "#0 presences within Middle-earth. " << std::endl;
	}

	//     if (immortals == 1) {
	//             if (mortals == 1) {
	//                     whoStream << " and it is an administrator." << std::endl;
	//             }
	//             else {
	//                     whoStream << ", of which #21#0 is an administrator." << std::endl;
	//             }
	//     }
	//     else if (immortals > 1) {
	//             whoStream << ", of which #2" << immortals << "#0 are administrators." << std::endl;
	//     }
	//     else {
	//             whoStream << "." << std::endl;
	//     }

	switch (sphere) {
		case 1:
			whoStream << "In the Kingdom of Gondor, there " << (clanCount == 1 ? "is#2 " : "are#2 ") << clanCount << " #0player";
			break;
		case 2:
			whoStream << "Around Caolafon, there " << (clanCount == 1 ? "is#2 " : "are#2 ") << clanCount << " #0player";
			break;
			//case 3:
			//	whoStream << "In Fahad Jafari, there " << (clanCount == 1 ? "is#2 " : "are#2 ") << clanCount << " #0player";
			//	break;
			//case 4:
			//	whoStream << "In the Mines Of Moria, there " << (clanCount == 1 ? "is#2 " : "are#2 ") << clanCount << " #0player";
			//	break;
		case 5:
			whoStream << "Currently in the Mordorian Spire, there " << (clanCount == 1 ? "is#2 " : "are#2 ") << clanCount << " #0player";
			break;
		case 6:
			whoStream << "The lands of Ithilien may hold any number of players."<< std::endl;
			break;
		default:
			break;
	}


	if (sphere > 0 && clanCount != 1 && sphere != 6) {
		whoStream << "s." << std::endl;
	}
	else if (sphere > 0 && sphere != 6) {
		whoStream << "." << std::endl;
	}

	if (sphere == -1)
		whoStream << "Outside of the scope of Arda [#6" << ch->in_room / 1000 << "#0] " << std::endl;

	if (sphere == 1)
	{

		whoStream << gatheringPlace(3271, "the Wardog Commons");
		whoStream << gatheringPlace(1108, "the Battered Shield");
		whoStream << gatheringPlace(1111, "the Fox and Lily");
		whoStream << gatheringPlace(3831, "the Copper Tankard Tavern");
		whoStream << gatheringPlace(54454, "the White Hart Tavern");

		//Show battalion information to only those who can access it - Vader
		if (is_clan_member(ch, "ithilien_battalion"))
		{
			whoStream << gatheringPlace(51162, "the Keep's Courtyard");
			whoStream << gatheringPlace(51169, "the Keep's Dining Hall");
			whoStream << gatheringPlace(51170, "the Keep's Kitchen");
			whoStream << gatheringPlace(54430, "the Mess Hall");
		}
		if (is_clan_member(ch, "hawk_and_dove"))
		{
			whoStream << gatheringPlace(8299, "the Hawk and Dove Barn");
		}
		if (is_clan_member(ch, "mt_lower_class"))
		{
			whoStream << gatheringPlace(21170, "the Commons of the Wayside Tavern");
		}
		if (is_clan_member(ch, "z3_astirian_wardens"))
		{
			whoStream << gatheringPlace(8805, "the Pelennor Bastion Commons");
		}
	}

	else if (sphere == 2)
	{
		whoStream << gatheringPlace(45001, "The Boar's Head Inn");
		whoStream << gatheringPlace(45280, "An Expansive Marshaling Yard");
		whoStream << gatheringPlace(45000, "Intersection at the Boar's Head Tavern");
	}
	else if (sphere == 3)
	{
		whoStream << gatheringPlace(80129, "the Drifting Lily Inn");
		whoStream << gatheringPlace(80239, "the Drowning Corsair");
	}
	else if (sphere == 5)
	{
		whoStream << gatheringPlace(24217, "The Sunken Arena");
		whoStream << gatheringPlace(24212, "The Swill Hole");
		whoStream << gatheringPlace(24119, "The Lodge");
	}


	if (guests != 1) {
		whoStream << "Additionally, there are #2" << guests << "#0 guests visiting our out of character Guest Lounge." << std::endl;
	}
	else {
		whoStream << "Additionally, there is #21#0 guest visiting our out of character Guest Lounge." << std::endl;
	}

	whoStream << std::endl << "Our record is #2" << count_max_online << "#0 players, last seen on #2" << max_online_date << "#0." << std::endl;
	whoStream << availableAdminsStream.str();

	if (!availableAdmins) {
		whoStream << "None." << std::endl;
	}

	send_to_char (whoStream.str().c_str(), ch);
}

void
do_scan (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;

	argument = one_argument (argument, buf);

	if (!real_skill (ch, SKILL_SCAN) || ch->skills[SKILL_SCAN] <= 0)
	{
		send_to_char ("You cannot focus well enough.\n", ch);
		return;
	}

	if (is_sunlight_restricted (ch))
		return;

	if (IS_SET (ch->room->room_flags, STIFLING_FOG)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		send_to_char ("The thick fog in the area prevents any such attempt.\n",
			ch);
		return;
	}

	
	if (IS_MORTAL (ch)
		&& weather_info[ch->room->zone].state == HEAVY_SNOW
		&& !IS_SET (ch->room->room_flags, INDOORS)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		act ("The onslaught of blowing snow severely inhibits your scanning.", false, ch, 0, 0, TO_CHAR);
	}


	if (!*buf)
	{				/* Quick one-room deep scan */

		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if ((exit = EXIT (ch, dir)) &&
				vtor (exit->to_room) &&
				(!(IS_SET (exit->exit_info, EX_ISDOOR) &&
				IS_SET (exit->exit_info, EX_CLOSED)) ||
				(IS_SET (exit->exit_info, EX_CLOSED) &&
				IS_SET (exit->exit_info, EX_ISGATE))))
			{
				break;
			}
		}

		if (dir == LAST_DIR + 1)
		{
			send_to_char ("There is nowhere to scan.\n", ch);
			return;
		}

		/* cmd = 0 for normal scan */
		/* cmd = 1 for scan without echo */
		if (cmd == 0)
		{
			if (IS_OUTSIDE (ch))
			{
				act ("$n scans the horizon in all directions.",
					true, ch, 0, 0, TO_ROOM);
				act ("You focus on the horizon . . .", false, ch, 0, 0, TO_CHAR);
			}
			else
			{
				act ("$n peers into the distance in all directions.",
					true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
				act ("You peer into the distance . . .", false, ch, 0, 0, TO_CHAR);
			}
		}

		if (cmd == 0)
		{
			ch->delay = 2;
			ch->delay_info3 = 0;
		}
		else
		{
			ch->delay = 1; //quiet scan is faster
			ch->delay_info3 = 1; //penalty for quiet scan
		}

		ch->delay_type = DEL_QUICK_SCAN;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;	/* Indicates something was seen */

		return;
	} //if (!*buf)

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
		ch->delay = 1; //quiet scan is faster
		ch->delay_info3 = 1; //penalty for quiet scan
		ch->delay_type = DEL_QUICK_SCAN;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;	/* Indicates something was seen */
		return;
	default:
		send_to_char ("Which direction would you like to scan?\n", ch);
		return;
	}

	exit = EXIT (ch, dir);

	if (!exit || !(next_room = vtor (exit->to_room)))
	{
		send_to_char ("There's nothing to scan that way.\n", ch);
		return;
	}

	if (IS_OUTSIDE (ch))
	{
		if (dir != UP && dir != DOWN)
		{
			sprintf (buf, "$n scans the %s horizon.", relative_dirs[dir]);
			act (buf, true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
			sprintf (buf, "You focus on the %s horizon . . .",
				relative_dirs[dir]);
			act (buf, false, ch, 0, 0, TO_CHAR);
		}
		else
		{
			act ("$n scans the horizon.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
			act ("You focus on the horizon . . .", false, ch, 0, 0, TO_CHAR);
		}
	}
	else
	{
		act ("$n peers into the distance.", true, ch, 0, 0, TO_ROOM | _ACT_SEARCH);
		act ("You peer into the distance . . .", false, ch, 0, 0, TO_CHAR);
	}

	ch->delay_type = DEL_SCAN;
	ch->delay = 2;
	ch->delay_info1 = dir;
}


bool
char__room_scan (CHAR_DATA * ch, ROOM_DATA * room, const char *ptrStrDir,
				 int nSkillLevel, bool bSaw)
{
	CHAR_DATA *tch = NULL;
	OBJ_DATA *tobj = NULL;
	char buf[AVG_STRING_LENGTH] = "";
	char buf2[AVG_STRING_LENGTH] = "";

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		*buf2 = '\0';

		if (!could_see (ch, tch))
			continue;

		if (tch == ch)
			continue;

		if (nSkillLevel < number (1, SKILL_CEILING)
			&& !has_been_sighted (ch, tch))
			continue;

		if (!bSaw)
		{
			bSaw = true;
			sprintf (buf, "%s%syou see:\n",
				ptrStrDir,
				IS_LIGHT (room) ? " " : ", #4where it is dark#0, ");
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		sprintf (buf, "#5%s#0%s\n", char_long (tch, 0), buf2);
		send_to_char (buf, ch);

		target_sighted (ch, tch);
	}

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{

		if (!could_see_obj (ch, tobj))
			continue;

		if (nSkillLevel < number (1, SKILL_CEILING))
			continue;

		if (!bSaw)
		{
			bSaw = true;
			sprintf (buf, "%s%syou see:\n",
				ptrStrDir,
				IS_LIGHT (room) ? " " : ", #4where it is dark#0, ");
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		show_obj_to_char (tobj, ch, 7);

		ch->delay_info2 = 1;
	}

	if (!bSaw)
	{
		sprintf (buf, "%s%syou see nothing.\n", ptrStrDir,
			IS_LIGHT (room) ? " " : ", #4where it is dark#0, ");
		*buf = toupper (*buf);
		send_to_char (buf, ch);
	}

	return bSaw;
}

ROOM_DATA *
get_diagonal_scan_room (CHAR_DATA * ch, int nDirOne, int nDirTwo)
{
	ROOM_DIRECTION_DATA *ptrTmpExit = NULL;
	ROOM_DATA *ptrTmpRoom = NULL;

	if ((ptrTmpExit = EXIT (ch, nDirOne))
		&& ((!IS_SET (ptrTmpExit->exit_info, EX_ISDOOR))
		&& (ptrTmpRoom = vtor (ptrTmpExit->to_room))
		&& (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
		&& (ptrTmpRoom->sector_type != SECT_INSIDE)
		&& (ptrTmpRoom->sector_type != SECT_CITY)
		&& (ptrTmpRoom->sector_type != SECT_UNDERWATER)
		&& !(IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
		&& (ptrTmpExit = ptrTmpRoom->dir_option[nDirTwo])
		&& (!IS_SET (ptrTmpExit->exit_info, EX_ISDOOR))
		&& (ptrTmpRoom = vtor (ptrTmpExit->to_room))
		&& (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
		&& (ptrTmpRoom->sector_type != SECT_INSIDE)
		&& (ptrTmpRoom->sector_type != SECT_CITY)
		&& (ptrTmpRoom->sector_type != SECT_UNDERWATER)
		&& !(IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))))
	{

		return ptrTmpRoom;

	}

	return NULL;
}


/*
*	diagonal scan - quick method
*
*	called if we just checked dir x from an outdoor room
*
*	1. start with a 100% penalty
*	2. if the room in dir x is outdoor -34%
*	3. if the room in dir x+1 is outdoor -34%
*	4. if the there's a joining room that is outdoor scan & delay
*      5. resume normal scan
*
*	A---B	If we are at room 'N' and just scanned north to 'A'
*	|    Y  and are about to scan east to 'X' then we have a
*	|    |	chance of seeing 'B' east of 'A' as well as 'Y' north
*	N----X	of 'X' in many cases 'Y' and 'B' will be the same.
*
*/
const char *diag_dirs[] = {
	"northeast",
	"southeast",
	"southwest",
	"northwest",
	"\n"
};
void
delayed_quick_scan_diag (CHAR_DATA * ch)
{
	int nLastDir = 0;
	int nNextDir = 0;
	int nPenalty = 100;
	int nSkillLevel = 0;
	bool bSaw = false;

	ROOM_DATA *ptrLastDirScanRoom = NULL;
	ROOM_DATA *ptrNextDirScanRoom = NULL;

	nLastDir = ch->delay_info1;
	nNextDir = (nLastDir + 1) % 4;

	ch->delay_type = DEL_QUICK_SCAN;
	ch->delay_info1 = nLastDir + 1;
	ch->delay = 1;

	/* LastDirScanRoom = Room B in the Diagram */

	if ((ptrLastDirScanRoom = get_diagonal_scan_room (ch, nLastDir, nNextDir)))
	{

		nPenalty -= 34;

	}

	/* NextDirScanRoom = Room Y in the Diagram */

	if ((ptrNextDirScanRoom = get_diagonal_scan_room (ch, nNextDir, nLastDir)))
	{

		if (ptrNextDirScanRoom == ptrLastDirScanRoom)
		{
			nPenalty -= 34;
			ptrNextDirScanRoom = NULL;
		}

	}

	if (weather_info[ch->room->zone].fog == THIN_FOG)
		nPenalty += 5;
	else if (weather_info[ch->room->zone].fog == THICK_FOG)
		nPenalty += 15;

	if (weather_info[ch->room->zone].state == STEADY_RAIN)
		nPenalty += 5;
	else if (weather_info[ch->room->zone].state == HEAVY_RAIN)
		nPenalty += 15;
	else if (weather_info[ch->room->zone].state == STEADY_SNOW)
		nPenalty += 20;
	else if (weather_info[ch->room->zone].state == HEAVY_SNOW)
		nPenalty += 35;

	nSkillLevel = skill_level (ch, SKILL_SCAN, nPenalty);

	if (ptrLastDirScanRoom)
	{
		bSaw =
			char__room_scan (ch, ptrLastDirScanRoom, diag_dirs[nLastDir],
			nSkillLevel, bSaw);
	}
	if (ptrNextDirScanRoom)
	{
		bSaw =
			char__room_scan (ch, ptrNextDirScanRoom, diag_dirs[nLastDir],
			nSkillLevel, bSaw);
	}

	if (bSaw)
	{
		ch->delay_info2 = 1;	/* We saw something */
	}
}

void
delayed_quick_scan (CHAR_DATA * ch)
{
	int dir = 0;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;
	int nPenalty = 0;
	int nSkillLevel = 0;

	dir = ch->delay_info1;

	if (!dir)
	{
		if (ch->delay_info3 == 0)
			skill_use (ch, SKILL_SCAN, 0);
		else
			skill_use (ch, SKILL_SCAN, 25); //quick and quiet scan
	}

	for (; dir <= LAST_DIR;)
	{

		if (!(exit = EXIT (ch, dir))
			|| (IS_SET (exit->exit_info, EX_ISDOOR)
			&& IS_SET (exit->exit_info, EX_CLOSED))
			|| !(next_room = vtor (exit->to_room))
			|| (IS_SET (next_room->room_flags, STIFLING_FOG)
			&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS))))
		{

			if (dir < 4)
			{
				ch->delay_type = DEL_QUICK_SCAN_DIAG;
				ch->delay_info1 = dir;
				ch->delay = 1;
			}
			return;

		}
		else
		{

			break;

		}

	}

	if (dir > LAST_DIR)
	{
		return;
	}

	if (weather_info[ch->room->zone].fog == THIN_FOG)
		nPenalty += 5;
	else if (weather_info[ch->room->zone].fog == THICK_FOG)
		nPenalty += 15;

	if (weather_info[ch->room->zone].state == STEADY_RAIN)
		nPenalty += 5;
	else if (weather_info[ch->room->zone].state == HEAVY_RAIN)
		nPenalty += 15;
	else if (weather_info[ch->room->zone].state == STEADY_SNOW)
		nPenalty += 20;
	else if (weather_info[ch->room->zone].state == HEAVY_SNOW)
		nPenalty += 35;

	nSkillLevel = skill_level (ch, SKILL_SCAN, nPenalty);

	ch->delay_info2 =
		char__room_scan (ch, next_room, dirs[dir], nSkillLevel, 0);

	if (dir < 4)
	{
		ch->delay_type = DEL_QUICK_SCAN_DIAG;
		ch->delay_info1 = dir;
		ch->delay = 1;
	}
	else if (dir <= LAST_DIR)
	{
		dir++;
		ch->delay_type = DEL_QUICK_SCAN;
		ch->delay_info1 = dir;
		ch->delay = 1;
	}
	else
	{
		return;
	}

}

void
delayed_scan (CHAR_DATA * ch)
{
	int dir = 0;
	int blank = 0;
	int seen = 0;
	int something_said = 0;
	int penalty = 0;
	CHAR_DATA *tch = NULL;
	ROOM_DIRECTION_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;
	OBJ_DATA *tobj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	/*
	4.  Scan into darkness.  As suggested by a player (Terisa), scanning
	into a darkened room should at least have a chance of revealing
	minimal information (i.e., "You think you see something.").  This
	should only affect rooms adjacent to the room being scanned from,
	and should, if possible, only work if the room being scanned is
	adjacent to a lighted room.  (Vague impressions seen in the dim
	light.)
	Change the message given for "very far away" room to 'something'
	instead of 'someone'.
	*/

	dir = ch->delay_info1;

	if (!real_skill (ch, SKILL_SCAN) || ch->skills[SKILL_SCAN] <= 0)
	{
		send_to_char ("You can see nothing there.\n", ch);
		return;
	}

	exit = EXIT (ch, dir);

	if (!exit || !(next_room = vtor (exit->to_room)))
	{
		send_to_char ("There's nothing to scan that way.\n", ch);
		return;
	}

	if (IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED) &&
		!IS_SET (exit->exit_info, EX_ISGATE))
	{
		if (!IS_LIGHT (ch->room))
			send_to_char ("You see nothing.\n", ch);
		else
			send_to_char ("Your view is blocked.\n", ch);

		return;
	}

	skill_use (ch, SKILL_SCAN, 0);

	seen = 0;
	blank = 1;

	if (weather_info[ch->room->zone].fog == THIN_FOG)
		penalty += 5;
	if (weather_info[ch->room->zone].fog == THICK_FOG)
		penalty += 15;
	if (weather_info[ch->room->zone].state == STEADY_RAIN)
		penalty += 5;
	if (weather_info[ch->room->zone].state == HEAVY_RAIN)
		penalty += 15;
	if (weather_info[ch->room->zone].state == STEADY_SNOW)
		penalty += 20;
	if (weather_info[ch->room->zone].state == HEAVY_SNOW)
		penalty += 35;

	if (!next_room->psave_loaded)
		load_save_room (next_room);

	for (tch = next_room->people; tch; tch = tch->next_in_room)
	{

		*buf2 = '\0';

		if (!could_see (ch, tch))
			continue;

		if ((IS_SET (next_room->room_flags, STIFLING_FOG) && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS))))
			continue;

		if (ch->skills[SKILL_SCAN] - penalty < number (1, 100)
			&& !has_been_sighted (ch, tch))
			continue;

		if (!seen)
		{
			seen = 1;
			blank = 0;
			if (IS_LIGHT (next_room))
				send_to_char ("Nearby you see:\n", ch);
			else
				send_to_char ("Nearby, #4where it is dark#0, you see:\n", ch);
		}

		sprintf (buf, "#5%s#0%s\n", char_long (tch, 0), buf2);
		send_to_char (buf, ch);

		something_said = 1;

		target_sighted (ch, tch);
	}

	for (tobj = next_room->contents; tobj; tobj = tobj->next_content)
	{

		if (!could_see_obj (ch, tobj))
			continue;

		if ((IS_SET (next_room->room_flags, STIFLING_FOG) && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS))))
			continue;

		if (ch->skills[SKILL_SCAN] - penalty < number (1, 100))
			continue;

		if (!seen)
		{
			seen = 1;
			blank = 0;
			if (IS_LIGHT (next_room))
				send_to_char ("Nearby you see:\n", ch);
			else
				send_to_char ("Nearby, #4where it is dark#0, you see:\n", ch);
		}

		show_obj_to_char (tobj, ch, 7);

		something_said = 1;
	}

	exit = next_room->dir_option[dir];

	if (!exit || !(next_room = vtor (exit->to_room)) ||
		(IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED)))
	{

		if (!something_said)
			send_to_char ("You detect nothing.\n", ch);

		return;
	}

	if (!blank)
	{
		blank = 1;
		send_to_char ("\n", ch);
	}

	seen = 0;

	if (!next_room->psave_loaded)
		load_save_room (next_room);

	penalty += 5;

	for (tch = next_room->people; tch; tch = tch->next_in_room)
	{

		*buf2 = '\0';

		if (!could_see (ch, tch))
			continue;

		if (IS_SET (next_room->room_flags, STIFLING_FOG) && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)))
			continue;

		if (ch->skills[SKILL_SCAN] - penalty < number (1, 100)
			&& !has_been_sighted (ch, tch))
			continue;

		if (!seen)
		{
			seen = 1;
			blank = 0;
			if (IS_LIGHT (next_room))
				send_to_char ("Far away you see:\n", ch);
			else
				send_to_char ("Far away, #4where it is dark#0, you see:\n", ch);
		}

		sprintf (buf, "   #5%s#0%s\n", char_short (tch), buf2);
		send_to_char (buf, ch);

		something_said = 1;

		target_sighted (ch, tch);
	}

	for (tobj = next_room->contents; tobj; tobj = tobj->next_content)
	{

		if (!could_see_obj (ch, tobj))
			continue;

		if (IS_SET (next_room->room_flags, STIFLING_FOG) && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)))
			continue;

		if (tobj->obj_flags.weight < 10000 && !CAN_WEAR (tobj, ITEM_TAKE))
			continue;

		if (!seen)
		{
			seen = 1;
			blank = 0;
			if (IS_LIGHT (next_room))
				send_to_char ("Far away you see:\n", ch);
			else
				send_to_char ("Far away, #4where it is dark#0, you see:\n", ch);
		}

		sprintf (buf, "   you see #2%s#0.\n", tobj->short_description);
		send_to_char (buf, ch);

		something_said = 1;
	}

	exit = next_room->dir_option[dir];

	if (!exit || !(next_room = vtor (exit->to_room)) ||
		(IS_SET (exit->exit_info, EX_ISDOOR) &&
		IS_SET (exit->exit_info, EX_CLOSED)))
	{

		if (!something_said)
			send_to_char ("You detect nothing.\n", ch);

		return;
	}

	if (!blank)
	{
		blank = 1;
		send_to_char ("\n", ch);
	}

	penalty += 10;

	for (tch = next_room->people; tch; tch = tch->next_in_room)
	{

		*buf2 = '\0';

		if (!could_see (ch, tch))
			continue;

		if (IS_SET (next_room->room_flags, STIFLING_FOG) && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)))
			continue;

		if (ch->skills[SKILL_SCAN] - penalty < number (1, 100)
			&& !has_been_sighted (ch, tch))
			continue;

		if (!seen)
		{
			seen = 1;
			blank = 0;
			if (IS_LIGHT (next_room))
				send_to_char ("Very far away you see:\n", ch);
			else
				send_to_char ("Very far away, #4where it is dark#0, you see:\n",
				ch);
		}

		sprintf (buf, "   #5%s#0%s\n", char_short (tch), buf2);
		send_to_char (buf, ch);
		something_said = 1;

		target_sighted (ch, tch);
	}

	if (!something_said)
		send_to_char ("You detect nothing.\n", ch);
}

void
do_scan_old (CHAR_DATA * ch, char *argument, int cmd)
{
	int roll, dir, count = 0;
	CHAR_DATA *tmp_ch = NULL;
	ROOM_DATA tmp_room;
	char *dname[] = {
		"north",
		"east",
		"south",
		"west",
		"up",
		"down"
	};

	for (; isspace (*argument); argument++);

	switch (*argument)
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
	default:
		send_to_char ("Which direction is that??\n", ch);
		return;
	}
	if (!EXIT (ch, dir) || !vtor (EXIT (ch, dir)->to_room))
	{
		send_to_char ("Nothing to see there.\n", ch);
		return;
	}
	if (IS_SET (EXIT (ch, dir)->exit_info, EX_ISDOOR) &&
		IS_SET (EXIT (ch, dir)->exit_info, EX_CLOSED))
	{
		sprintf (s_buf, "A %s blocks your view.\n", EXIT (ch, dir)->keyword);
		send_to_char (s_buf, ch);
		return;
	}

	act ("$n scans the horizon.\n", true, ch, 0, 0, TO_ROOM);
	sprintf (s_buf, "You scan %s and see:\n[Near]\n", dname[dir]);
	send_to_char (s_buf, ch);
	s_buf[0] = '\0';

	if (!vtor (EXIT (ch, dir)->to_room)->people ||
		!skill_use (ch, SKILL_SCAN, 0))
		sprintf (s_buf + strlen (s_buf), "Nothing detected.\n");
	else
		list_char_to_char (vtor (EXIT (ch, dir)->to_room)->people, ch, 0);

	tmp_room = *(vtor (EXIT (ch, dir)->to_room));
	if (!tmp_room.dir_option[dir]
	|| (IS_SET (tmp_room.dir_option[dir]->exit_info, EX_ISDOOR)
		&& IS_SET (tmp_room.dir_option[dir]->exit_info, EX_CLOSED)))
	{
		send_to_char (s_buf, ch);
		return;
	}
	strcat (s_buf, "[Far]\n");
	roll = number (1, 100);
	if ((roll > ch->skills[SKILL_SCAN])
		|| !vtor (tmp_room.dir_option[dir]->to_room)->people)
		strcat (s_buf, "Nothing detected.\n");
	else
	{
		for (tmp_ch = vtor (tmp_room.dir_option[dir]->to_room)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room)
			count++;
		if (count > 1)
			sprintf (s_buf + strlen (s_buf), "%d life forms.\n", count);
		else
			strcat (s_buf, "1 life form.\n");
	}

	tmp_room = *(vtor (tmp_room.dir_option[dir]->to_room));
	if (!tmp_room.dir_option[dir]
	|| (IS_SET (tmp_room.dir_option[dir]->exit_info, EX_ISDOOR)
		&& IS_SET (tmp_room.dir_option[dir]->exit_info, EX_CLOSED)))
	{
		send_to_char (s_buf, ch);
		return;
	}
	strcat (s_buf, "[Very Far]\n");
	if ((roll > ch->skills[SKILL_SCAN])
		|| !vtor (tmp_room.dir_option[dir]->to_room)->people)
		strcat (s_buf, "Nothing detected.\n");
	else if (vtor (tmp_room.dir_option[dir]->to_room)->people)
		strcat (s_buf, "You detect movement.\n");
	else
		strcat (s_buf, "Nothing detected.\n");

	send_to_char (s_buf, ch);
}

void
do_qscan (CHAR_DATA * ch, char *argument, int cmd)
{
	do_scan (ch, "", 1);
	return;
}

void
do_count (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *obj = NULL;

	argument = one_argument (argument, buf);

	if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& IS_MORTAL (ch) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("It's too dark to count coins.\n", ch);
		return;
	}

	if (!*buf)
	{
		sprintf (buf, "You begin searching through your belongings, taking a tally of your coin.\n");
		send_to_char (buf, ch);
	}
	else
	{
		if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
		{
			send_to_char ("I don't see that group of coins.\n", ch);
			return;
		}

		if (GET_ITEM_TYPE (obj) != ITEM_MONEY)
		{
			send_to_char ("That isn't a group of coins.\n", ch);
			return;
		}

		sprintf (buf, "After a moment of sorting, you determine that there are %d"
			" coins in the pile.", obj->count);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}
	ch->delay = 10;
	ch->delay_type = DEL_COUNT_COIN;
}

char* coin_sdesc( const OBJ_DATA * coin )
{
	return coin_sdesc( coin->nVirtual, coin->count ? true : false );
}

char* coin_sdesc( const int currnum, const int seqnum, const bool plural )
{
	if( currnum == CURRENCY_TIRITH )
		return coin_sdesc( 1544  - seqnum, plural );
	if( currnum == CURRENCY_MORGUL )
		return coin_sdesc( 5030  + seqnum, plural );
	if( currnum == CURRENCY_EDEN )
		return coin_sdesc( 6900  + seqnum, plural );
	if( currnum == CURRENCY_HARAD )
		return coin_sdesc( 80010 + seqnum, plural );
	if( currnum == CURRENCY_NORTHMAN )
		return coin_sdesc( 42131 + seqnum, plural );
	return "BAH";
}

char* coin_sdesc( const int objnum, const bool plural )
{
	std::string coinDesc;
	switch (objnum)
	{
	case 1538:
		coinDesc = "glittering mithril coin";
		break;
	case 1539:
		coinDesc = "thick, hexagonal gold coin";
		break;
	case 1540:
		coinDesc = "thin, ridged silver coin";
		break;
	case 1541:
		coinDesc = "large, rounded bronze coin";
		break;
	case 1542:
		coinDesc = "semicircular copper coin";
		break;
	case 1543:
		coinDesc = "thin, slightly fluted gold coin";
		break;
	case 1544:
		coinDesc = "heavy, oblong silver coin";
		break;
	case 5030:
		if (plural)
			return "crudely hewn tokens of dark granite";
		else
			return "crudely hewn token of dark granite";
	case 5031:
		if (plural)
			return "razor-sharp flakes of obsidian";
		else
			return "razor-sharp flake of obsidian";
	case 5032:
		coinDesc = "rectangular token of dusky brass";
		break;
	case 5033:
		coinDesc = "weighty pentagonal bronze coin";
		break;
	case 5034:
		if (plural)
			return "hexagonal tokens of blackened steel";
		else
			return "hexagonal token of blackened steel";
	case 5035:
		if (plural)
			return "octagonal coins of smoky silver";
		else
			"octagonal coin of smoky silver";
	case 42131:
		coinDesc = "small, rounded bronze coin";
		break;
	case 42132:
		coinDesc = "heavy, stamped bronze coin";
		break;
	case 42133:
		coinDesc = "hexagonal, stamped silver coin";
		break;
	case 42134:
		coinDesc = "large, ornately detailed silver coin";
		break;
	case 66900:
		coinDesc = "small iron disc";
		break;
	case 66901:
		coinDesc = "dull copper coin";
		break;
	case 66902:
		coinDesc = "small, circular silver coin";
		break;
	case 66903:
		coinDesc = "brazen silver coin";
		break;
	case 66904:
		coinDesc = "ornate silver coin";
		break;
	case 66905:
		coinDesc = "heavy, gleaming, gold coin";
		break;
	case 80010:
		coinDesc = "small copper coin";
		break;
	case 80011:
		coinDesc = "large bronze coin";
		break;
	case 80012:
		coinDesc = "thin silver coin";
		break;
	case 80013:
		coinDesc = "heavy silver coin";
		break;
	case 80014:
		coinDesc = "small gold coin";
		break;
	case 80015:
		coinDesc = "shiny, thick round gold coin";
		break;
	default:
		coinDesc = "coin";
		break;
	}

	if (plural)
		coinDesc += 's';

	return (char*)(coinDesc.c_str());
}
char get_currency_symbol(const int objnum)
{
	switch(objnum)
	{
	case NORTHMAN_ONUM_FARTHING:
		return NORTHMAN_PRINTC_FARTHING;
	case NORTHMAN_ONUM_PENNY:
		return NORTHMAN_PRINTC_PENNY;
	case NORTHMAN_ONUM_SHILLING:
		return NORTHMAN_PRINTC_SHILLING;
	case  NORTHMAN_ONUM_POUND:
		return NORTHMAN_PRINTC_POUND;
	default:
		return 0;
	}
}

char get_currency_symbol(const int currnum, const int denomnum)
{
	if(currnum == CURRENCY_NORTHMAN)
		return get_currency_symbol( 42131 + denomnum );
	return 0;
}

int get_currency_value(const int objnum)
{
	switch(objnum)
	{
	case NORTHMAN_ONUM_FARTHING:
		return NORTHMAN_VAL_FARTHING;
	case NORTHMAN_ONUM_PENNY:
		return NORTHMAN_VAL_PENNY;
	case NORTHMAN_ONUM_SHILLING:
		return NORTHMAN_VAL_SHILLING;
	case  NORTHMAN_ONUM_POUND:
		return NORTHMAN_VAL_POUND;
	default:
		return 0;
	}
}

int get_currency_value(const OBJ_DATA* obj)
{ return (int) obj->farthings; }

//shorthand define, for readability, please don't move this to a const file, as that
//will horribly obfuscate the code.  Thankies
#define CURR_SYM get_currency_symbol(obj->nVirtual)
void currency_print_line(const OBJ_DATA* obj, int* money, char* buffer, const char* wear_loc)
{
	sprintf(buffer + strlen( buffer ), "   #2%d %s#0 (#2%s#0): ", obj->count, coin_sdesc( obj ), wear_loc);
	money[CURR_SYM ? 1 : 0] += get_currency_value(obj) * obj->count;  //Badass line, check that shit twice, then gasp :D
	if( CURR_SYM ) //checks if this is northman, note, this will need updating if more currencies are ever added
		sprintf(buffer + strlen( buffer ), "%d%c\n", obj->count, CURR_SYM);
	else
		sprintf(buffer + strlen( buffer ), "%d coppers\n", obj->count * get_currency_value(obj), CURR_SYM);
}
#undef CURR_SYM

void delayed_count_coin (CHAR_DATA * ch)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA* hand_obj_ary[2] = { ch->right_hand, ch->left_hand };
	char* hand_str_ary[2] = { "right hand", "left hand" };
	OBJ_DATA *obj = NULL, *tobj = NULL;
	int location = 0, money[2] = {0,0}, nmoney[4]; //money[] holds the count of various types of money the char has, other in slot 0 northman in slot 1
	char currency_symbol = '\0';
	std::string amount;

	for( int i = 0; i < 2; i++ )
	{
		if( hand_obj_ary[i] )
		{
			if( GET_ITEM_TYPE( hand_obj_ary[i] ) == ITEM_MONEY )
				currency_print_line( hand_obj_ary[i], money, buf2, hand_str_ary[i] );
			if( GET_ITEM_TYPE( hand_obj_ary[i] ) == ITEM_CONTAINER)
			{
				if (IS_SET (hand_obj_ary[i]->o.od.value[1], CONT_CLOSED))
				{
					std::stringstream ss;
					ss << "#2" <<  (char)toupper(hand_obj_ary[i]->short_description[0]) << (char*)&(hand_obj_ary[i]->short_description[1]) << "#0" << " is closed.\n";
					send_to_char(ss.str().c_str(),ch);
				}
				else
				{

					for( tobj = hand_obj_ary[i]->contains; tobj; tobj = tobj->next_content )
					{
						if( GET_ITEM_TYPE( tobj ) == ITEM_MONEY )
						{
							currency_print_line( tobj, money, buf2, obj_short_desc( tobj->in_obj ) );
						}
					}
				} // is closed
			} // is container
		} // item exists in this hand
	} // iterate both hands

	for( location = 0; location < MAX_WEAR; location++ )
	{
		if( obj = get_equip(ch, location) )
		{
			//			if( GET_ITEM_TYPE( obj ) == ITEM_MONEY )		//Technically unnecessary, as money cannot be worn, included for completeness
			//				currency_print_line( obj, money, buf2, locations[location] );
			if( GET_ITEM_TYPE( obj ) == ITEM_CONTAINER )
			{
				if (IS_SET (obj->o.od.value[1], CONT_CLOSED))
				{
					std::stringstream ss;
					ss << "#2" <<  (char)toupper(obj->short_description[0]) << (char*)&(obj->short_description[1]) << "#0" << " is closed.\n";
					send_to_char(ss.str().c_str(),ch);
				}
				else
				{
					for( tobj = obj->contains; tobj; tobj = tobj->next_content )
					{
						if( GET_ITEM_TYPE( tobj ) == ITEM_MONEY )
						{
							currency_print_line( tobj, money, buf2, obj_short_desc( tobj->in_obj ) );
						}
					}
				}
			} // container
		} // object exists at location
	}// for loop through locations

	if(money[1])
	{
		nmoney[0] = money[1] / NORTHMAN_VAL_POUND;
		money[1] %= NORTHMAN_VAL_POUND;
		nmoney[1] = money[1] / NORTHMAN_VAL_SHILLING;
		money[1] %= NORTHMAN_VAL_SHILLING;
		nmoney[2] = money[1] / NORTHMAN_VAL_PENNY;
		money[1] %= NORTHMAN_VAL_PENNY;
		nmoney[3] = money[1] / NORTHMAN_VAL_FARTHING;
		money[1] = 1;
		for( int i = 0; i < 4; i++ )
			if( nmoney[i] )
			{
				if( !amount.empty() )
					amount += " ";
				amount += MAKE_STRING( nmoney[i] ) + get_currency_symbol( CURRENCY_NORTHMAN, 3-i );
			}

	}

	if (money[0])
		if(money[1])
			sprintf (buf, "By your count, you have %s in coin, and also %d coppers:", amount.c_str(), money[0]);
		else
			sprintf( buf, "By your count, you have %d coppers' worth in coin:", money[0] );
	else
		if(money[1])
			sprintf (buf, "By your count, you have %s in coin:", amount.c_str());
		else
			send_to_char ("You don't seem to have any coin.\n", ch);
	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	send_to_char ("\n", ch);
	send_to_char (buf2, ch);
}

void
stop_counting (CHAR_DATA * ch)
{
	send_to_char ("You bore of counting coins.\n", ch);

	ch->delay = 0;

	ch->delay_obj = NULL;
}

void
tracking_system_response (CHAR_DATA * ch, MESSAGE_DATA * message)
{
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char from[MAX_STRING_LENGTH] = { '\0' };
	char subject[AVG_STRING_LENGTH] = { '\0' };
	char type[AVG_STRING_LENGTH] = { '\0' };

	if (!message)
		return;

	if (!ch->desc->pending_message->message || !*ch->desc->pending_message->message)
		return;

	account acct (message->poster);
	if (!acct.is_registered ())
	{
		if (!(tch = load_pc (message->poster)))
			return;
		else
			acct = account (tch->pc->account_name);
	}

	if (!acct.is_registered ())
	{
		if (tch != NULL)
			unload_pc (tch);
		return;
	}

	if (strcasecmp (ch->delay_who, "Bugs") == STR_MATCH)
		sprintf (type, "Bug");
	else if (strcasecmp (ch->delay_who, "Typos") == STR_MATCH)
		sprintf (type, "Typo");
	else if (strcasecmp (ch->delay_who, "Ideas") == STR_MATCH)
		sprintf (type, "Idea");
	else if (strcasecmp (ch->delay_who, "Submissions") == STR_MATCH)
		sprintf (type, "Writing Submission");
	else if (strcasecmp (ch->delay_who, "Petitions") == STR_MATCH)
		sprintf (type, "Petition");
	else
		sprintf (type, "Issue");

	sprintf (buf, "%s"
		"\n\n-- Original Report, Filed %s --\n\n"
		"%s", ch->desc->pending_message->message, message->date,
		message->message);

	sprintf (from, "%s <%s>", ch->tname, ch->desc->acct->email.c_str ());

	if (strcasecmp (type, "Writing Submission") == STR_MATCH)
	{
		sprintf (subject, "Re: Your Website Submission");
		send_email (&acct, REPORT_EMAIL, from, subject, buf);
	}
	else if (strcasecmp (type, "Petition") == STR_MATCH)
	{
		sprintf (subject, "Re: Your Logged Petition");
		send_email (&acct, PET_EMAIL, from, subject, buf);
	}
	else
	{
		sprintf (subject, "Re: Your %s Report", type);
		send_email (&acct, REPORT_EMAIL, from, subject, buf);
	}

	unload_pc (tch);
}

void
post_track_response (DESCRIPTOR_DATA * d)
{
	MESSAGE_DATA *message = NULL;
	MYSQL_RES *result = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';
	
	if (!strncasecmp(d->character->delay_who, "Typos",5))
		mysql_safe_query
		("UPDATE newsletter_stats SET resolved_typos=resolved_typos+1");
	else if (strcasecmp (d->character->delay_who, "Bugs") == STR_MATCH)
		mysql_safe_query
		("UPDATE newsletter_stats SET resolved_bugs=resolved_bugs+1");

	message =
		load_message (d->character->delay_who, 5, d->character->delay_info1);
	if (!d->pending_message->message)
	{
		send_to_char ("No email sent.\n", d->character);
	}
	else
		send_to_char ("The report poster has been sent an email notification.\n",
		d->character);
	tracking_system_response (d->character, message);

	mysql_safe_query
		("DELETE FROM virtual_boards WHERE post_number = \'%d\' AND board_name = \'%s\'",
		d->character->delay_info1, d->character->delay_who);

	if (mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = \'%s\'",
		d->character->delay_who))
		return;
	result = mysql_store_result (database);
	if (mysql_num_rows (result) == 0)
	{
		sprintf (buf, "vboards/%s", d->character->delay_who);
		unlink (buf);
	}

	d->character->delay_who = NULL;
	d->character->delay_info1 = 0;

	mysql_free_result (result);
	result = NULL;

	free_mem(message);
}

int
erase_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
	FILE *fp = NULL;
	unsigned int i = 0;
	CHAR_DATA *who = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (strlen (name) > 15)
		return 0;

	for (i = 0; i <= strlen (name); i++)
		if (!isalpha (*name))
			return 0;

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	if (who->pc->level > ch->pc->level)
	{
		unload_pc(who);
		return 0;
	}

	unload_pc (who);

	if (!isdigit (*argument))
		return 0;

	if (atoi (argument) > 255)
		return 0;

	sprintf (buf, "player_boards/%s.%06d", name, atoi (argument));
	if (!(fp = fopen (buf, "r")))
	{
		return 0;
	}
	else
		fclose (fp);

	sprintf (buf, "rm player_boards/%s.%06d", name, atoi (argument));
	system (buf);

	return 1;
}

int
erase_journal (CHAR_DATA * ch, char *argument)
{
	FILE *fp = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!isdigit (*argument))
	{
		send_to_char ("The specified journal entry must be a number.\n", ch);
		return 1;
	}

	if (atoi (argument) > 255)
	{
		send_to_char ("Enter a number between 1 and 255 to erase.\n", ch);
		return 1;
	}

	sprintf (buf, "player_journals/%s.%06d", ch->tname, atoi (argument));
	if (!(fp = fopen (buf, "r")))
	{
		send_to_char ("That journal entry could not be found.\n", ch);
		return 1;
	}
	else
		fclose (fp);

	sprintf (buf, "rm player_journals/%s.%06d", ch->tname, atoi (argument));
	system (buf);

	send_to_char ("The specified journal entry has been erased.\n", ch);

	return 1;
}

void
do_jerase (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if (!*buf)
	{
		send_to_char ("Which message do you wish to erase?\n", ch);
		return;
	}

	if (!erase_mysql_board_post (ch, ch->tname, 3, buf))
	{
		send_to_char ("I couldn't find that journal entry.\n", ch);
		return;
	}

	send_to_char ("Journal entry erased successfully.\n", ch);
}

void
do_erase (CHAR_DATA * ch, char *argument, int cmd)
{
	int msg_no = 0;
	MESSAGE_DATA *message = NULL;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Which message do you wish to erase?\n", ch);
		return;
	}

	if (IS_NPC (ch))
	{
		send_to_char ("You can't do this while switched.\n", ch);
		return;
	}

	if (!isdigit (*buf))
	{
		if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
		{
			if (IS_MORTAL (ch)
				|| !erase_mysql_board_post (ch, buf, 2, argument))
			{
				if (IS_MORTAL (ch)
					|| !erase_mysql_board_post (ch, buf, 1, argument))
				{
					if (IS_MORTAL (ch))
						send_to_char ("You do not see that board here.\n", ch);
					else
						send_to_char
						("Either the specified board is not here, or the specified message does not exist.\n",
						ch);
					return;
				}
				else
				{
					send_to_char ("The specified message has been erased.\n",
						ch);
					return;
				}
			}
			else
			{
				send_to_char ("The specified message has been erased.\n", ch);
				return;
			}
		}

		if (obj->obj_flags.type_flag != ITEM_BOARD)
		{
			act ("$p isn't a board.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		one_argument (obj->name, buf2);
		argument = one_argument (argument, buf);
	}

	else
	{
		if (!(obj = get_obj_in_list_vis (ch, "board", ch->room->contents)) &&
			!(obj = get_obj_in_list_vis (ch, "board", ch->left_hand)) &&
			!(obj = get_obj_in_list_vis (ch, "board", ch->right_hand)))
		{
			send_to_char ("You do not see that board here.\n", ch);
			return;
		}

		if (obj->obj_flags.type_flag != ITEM_BOARD)
		{
			act ("$p, the first board here, isn't a real board.",
				false, ch, obj, 0, TO_CHAR);
			return;
		}

		one_argument (obj->name, buf2);
	}

	if (!isdigit (*buf))
	{
		send_to_char ("Expected a message number to erase.\n", ch);
		return;
	}

	msg_no = atoi (buf);

	if (!(message = load_message (buf2, 6, msg_no)))
	{
		send_to_char ("That message doesn't exist.\n", ch);
		return;
	}

	if (!GET_TRUST (ch)
		&& strcasecmp (GET_NAME (ch), message->poster) != STR_MATCH)
	{
		send_to_char ("You can only erase your own messages.\n", ch);
		free_mem(message);
		return;
	}

	if (erase_mysql_board_post (ch, buf2, 0, buf))
		send_to_char ("The specified message has been erased.\n", ch);
	else
		send_to_char ("There was a problem erasing that message.\n", ch);

	free_mem(message);
}

int
write_virtual_board (CHAR_DATA * ch, char *name, char *argument)
{
	if (strlen (name) > 15)
		return 0;

	if (!isalpha (*name))
		return 0;

	*name = toupper (*name);

	if (!*argument)
	{
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem (ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -2;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);
	ch->desc->pending_message->flags = MF_READ;

	make_quiet (ch);

	send_to_char ("Enter your note, terminate with an '@'\n\n", ch);


	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_virtual_board;

	return 1;
}

void
post_player_message (DESCRIPTOR_DATA * d)
{
	char date[AVG_STRING_LENGTH] = "";
	time_t current_time = 0;

	current_time = time (0);

	ctime_r (&current_time, date);

	/* asctime adds a \n to the end of the date string - remove it */

	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
		send_to_char ("No message posted.\n", d->character);
	else
		add_message (1,		/* New message */
		d->pending_message->poster,	/* name */
		d->pending_message->nVirtual,	/* virtual */
		GET_NAME (d->character),	/* poster */
		date,		/* date */
		d->pending_message->subject,	/* subject */
		d->pending_message->info,	/* info */
		d->pending_message->message,	/* message */
		d->pending_message->flags);

	free_mem(d->pending_message);

	d->pending_message = NULL;
}

int
write_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
	unsigned int i = 0;
	CHAR_DATA *who = NULL;

	if (strlen (name) > 15)
		return 0;

	for (i = 0; i <= strlen (name); i++)
		if (!isalpha (*name))
			return 0;

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	unload_pc (who);

	if (!*argument)
	{
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -1;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);
	ch->desc->pending_message->flags = 0;

	make_quiet (ch);

	send_to_char ("Enter your note, terminate with an '@'\n\n", ch);

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_player_board;

	return 1;
}
/// \todo merge with post_player_message
void
post_journal (DESCRIPTOR_DATA * d)
{
	char date[AVG_STRING_LENGTH] = "";
	time_t current_time = 0;

	current_time = time (0);

	ctime_r (&current_time, date);

	/* asctime adds a \n to the end of the date string - remove it */

	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
		send_to_char ("No journal entry added.\n", d->character);
	else
		add_message (5,		/* New message */
		d->pending_message->poster,	/* name */
		-3,		/* virtual */
		GET_NAME (d->character),	/* poster */
		date,		/* date */
		d->pending_message->subject,	/* subject */
		d->pending_message->info,	/* info */
		d->pending_message->message,	/* message */
		d->pending_message->flags);

	free_mem(d->pending_message);

	d->pending_message = NULL;
}

int
write_journal (CHAR_DATA * ch, char *argument)
{

	if (!*argument)
	{
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (ch->tname);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -1;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);

	make_quiet (ch);

	send_to_char ("Type your journal entry; terminate with an '@'\n\n", ch);

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_journal;

	return 1;
}

void
do_write (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char title[MAX_STRING_LENGTH] = { '\0' };

	if (IS_NPC (ch))
	{
		send_to_char ("Write is only available to PCs.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("What did you want to write on?\n", ch);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		if (IS_MORTAL (ch) || !write_pc_board (ch, buf, argument))
		{
			if (IS_MORTAL (ch) || !write_virtual_board (ch, buf, argument))
				send_to_char ("You can't find that.\n", ch);
		}
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT ||
		obj->obj_flags.type_flag == ITEM_BOOK)
	{
		do_scribe (ch, buf, 0);
		return;
	}

	if (!*argument)
	{
		send_to_char ("What would you like to write about?\n", ch);
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_BOARD)
	{
		act ("You can't write on $p.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	while (*argument == ' ')
		argument++;

	/* Get the name of the board */

	one_argument (obj->name, buf);

	strcpy (title, argument);

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (buf);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = 0;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (title);

	ch->desc->pending_message->flags = 0;

	send_to_char
		("\n#6Enter your message below. To terminate, use the '@' character. Please ensure\n"
		"you use proper linebreaks, and that your writing follows the acceptable posting\n"
		"policies for our in-game boards as outlined in HELP POSTING_POLICIES.#0\n\n"
		"1-------10--------20--------30--------40--------50--------60--------70--------80\n",
		ch);

	make_quiet (ch);

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_message;
}

void
do_jwrite (CHAR_DATA * ch, char *argument, int cmd)
{

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if (IS_NPC (ch))
	{
		send_to_char ("Journals are only available to PCs.\n", ch);
		return;
	}

	if (!*argument)
	{
		send_to_char ("What did you wish your subject to be?\n", ch);
		return;
	}

	if (!write_journal (ch, argument))
	{
		send_to_char ("There seems to be a problem with your journal.\n", ch);
		return;
	}
}

int
read_journal_message (CHAR_DATA * ch, CHAR_DATA * reader, char *argument)
{
	MESSAGE_DATA *message = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };

	sprintf (name, "%s", ch->tname);

	if (!atoi (argument))
	{
		if (!reader)
			send_to_char ("Which entry?\n", ch);
		else
			send_to_char ("Which entry?\n", reader);
		return 1;
	}

	if (!(message = load_message (name, 8, atoi (argument))))
	{
		if (!reader)
			send_to_char ("No such journal entry.\n", ch);
		else
			send_to_char ("No such journal entry.\n", reader);
		return 1;
	}

	sprintf (b_buf, "#6Date:#0    %s\n"
		"#6Subject:#0 %s\n\n%s", message->date, message->subject,
		message->message);

	if (!reader)
	{
		send_to_char ("\n", ch);
		page_string (ch->desc, b_buf);
	}
	else
	{
		send_to_char ("\n", reader);
		page_string (reader->desc, b_buf);
	}

	free_mem(message);

	return 1;
}

void
do_jread (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };

	bool loaded_char = false;
	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if (IS_NPC (ch))
	{
		send_to_char ("Journals are only available to PCs.\n", ch);
		return;
	}

	if (!IS_MORTAL (ch))
	{
		if (!*argument)
			who = ch;
		else if (*argument && isdigit (*argument))
			who = ch;
		else
		{
			argument = one_argument (argument, name);
			*name = toupper (*name);
			if (!(who = load_pc (name)))
			{
				send_to_char ("No such PC, I'm afraid.\n", ch);
				return;
			}
			loaded_char = true;
		}
		if (!read_journal_message (who, ch, argument))
		{
			send_to_char ("There seems to be a problem with the journal.\n",
				ch);
			if (loaded_char)
				unload_pc(who);
			return;
		}
	}
	else
	{
		if (!read_journal_message (ch, NULL, argument))
		{
			send_to_char ("There seems to be a problem with the journal.\n",
				ch);
			if (loaded_char)
				unload_pc(who);
			return;
		}
	}
	if (loaded_char)
		unload_pc(who);
}

void
post_message (DESCRIPTOR_DATA * d)
{
	if (!d->pending_message->message) {
		send_to_char ("No message posted.\n", d->character);
		free_mem(d->pending_message);
		d->pending_message = NULL;
	}
	else {
		post_to_mysql_board (d);
	}
}

void
add_board (int level, char *name, char *title)
{
	BOARD_DATA *board = NULL;
	BOARD_DATA *board_entry = NULL;

	/* Make sure this board doesn't already exist */

	if (board_lookup (name))
		return;

	board_entry = new BOARD_DATA;
	board_entry->next = NULL;
	board_entry->level = level;
	board_entry->name = duplicateString (name);
	board_entry->title = duplicateString (title);
	board_entry->next_virtual = 1;

	/* Add board_entry to end of full_board_list */

	if (!full_board_list)
		full_board_list = board_entry;

	else
	{
		board = full_board_list;

		while (board->next)
			board = board->next;

		board->next = board_entry;
	}
}

BOARD_DATA *
board_lookup (const char *name)
{
	BOARD_DATA *board = NULL;

	for (board = full_board_list; board; board = board->next)
	{
		if (strcasecmp (board->name, name) == STR_MATCH)
			return board;
	}

	return NULL;
}

void
show_unread_messages (CHAR_DATA * ch)
{
	int header = 1;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[AVG_STRING_LENGTH] = "";
	char query[MAX_STRING_LENGTH] = { '\0' };
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;

	mysql_safe_query
		("SELECT * FROM player_notes WHERE name = '%s' AND flags = 0 ORDER BY post_number DESC",
		ch->tname);
	result = mysql_store_result (database);

	*b_buf = '\0';

	while ((row = mysql_fetch_row (result)))
	{
		if (header)
		{
			sprintf (b_buf, "\nUnread messages on your private board:\n\n");
			header = false;
		}
		if (strlen (row[2]) > 44)
		{
			sprintf (query, "%s", row[2]);
			query[41] = '.';
			query[42] = '.';
			query[43] = '.';
			query[44] = '\0';
		}
		else
			sprintf (query, "%s", row[2]);
		sprintf (b_buf + strlen (b_buf), " #6%3d#0 - %16s %-10.10s: %s\n",
			atoi (row[1]), row[4], row[3], query);
	}

	if (!header)			/* Meaning, we have something to print */
		page_string (ch->desc, b_buf);

	mysql_free_result (result);
	result = NULL;

	mysql_safe_query
		("SELECT board_name, MIN(post_number) post_from, MAX(post_number) post_to, author, subject FROM virtual_boards WHERE timestamp >= %d GROUP BY board_name ORDER BY board_name ASC",
		(int) ch->pc->last_logon);
	result = mysql_store_result (database);
	header = true;

	while ((row = mysql_fetch_row (result)))
	{
		if (header)
		{
			sprintf (b_buf, "\nWelcome back! Since you last logged in:\n\n");
			send_to_char (b_buf, ch);
			header = false;
		}
		if (strcmp (row[1], row[2]) == 0)
		{
			if (strcmp (row[0], "Applications") == 0)
			{
				if (strstr (row[4], "Accepted"))
				{
					sprintf (buf2, "#6%s", row[4] + 14);
				}
				else
				{
					sprintf (buf2, "#1%s", row[4] + 14);
				}
			}
			else if (strcmp (row[0], "Crashes") == 0)
			{
				sprintf (buf2, "#6%s", row[3]);
			}
			else if (strcmp (row[0], "Helpfiles") == 0)
			{
				sprintf (buf2, "#6%s", row[4] + 15);
			}
			else if (strcmp (row[0], "Petitions") == 0)
			{
				sprintf (buf2, "#6%s", row[3]);
			}
			else if (strcmp (row[0], "Prisoners") == 0)
			{
				sprintf (buf2, "#6%s", row[4] + 12);
			}
			else if (strcmp (row[0], "Submissions") == 0)
			{
				row[4][strlen (row[4] - 2)] = '\0';
				sprintf (buf2, "#6%s", row[4] + 2);
			}
			else
			{
				sprintf (buf2, "#6%s", row[4]);
			}
			if (strlen (buf2) > 28)
			{
				strcpy (buf2 + 25, "...");
			}
			sprintf (buf, "   - '#2%s#0' was posted to (msg %s: %s#0).\n",
				row[0], row[1], buf2);
		}
		else
		{
			sprintf (buf, "   - '#2%s#0' was posted to (msgs %s to %s).\n",
				row[0], row[1], row[2]);
		}
		send_to_char (buf, ch);
	}

	mysql_free_result (result);
}

void
add_message (int new_message, const char *name, int nVirtual, const char *poster,
			 char *date, char *subject, char *info, char *message, long flags)
{
	int named = 0, day, i = 0;
	MESSAGE_DATA *msg = NULL;
	BOARD_DATA *board = NULL;
	CHAR_DATA *ch = NULL;
	DIR *dir = NULL;
	FILE *fp = NULL;
	bool found = false;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char msg_file_name[MAX_STRING_LENGTH] = { '\0' };
	char date_buf[MAX_STRING_LENGTH] = { '\0'};
	char *suf = '\0';
	time_t current_time = 0;

	if (!new_message)
	{
		system_log ("Add_mesage() called with new_message = 0", true);
		abort ();
	}

	if (!date)
	{
		/// \todo see who is responsible for deleteing the mem alloced by ctime
		current_time = time (0);
		date = ctime (&current_time);
		if (strlen (date) > 1)
			date[strlen (date) - 1] = '\0';
	}

	/* 1 means put a new message to a board (nVirtual == -1 to pc board */
	/* 2 means update a pc board message */

	if (new_message != 5 && new_message != 2 && nVirtual != -5 && nVirtual != -2
		&& nVirtual != -1 && !(board = board_lookup (name)))
	{
		printf ("No board for message; board:  '%s'.\n", name);
		return;
	}

	msg = new MESSAGE_DATA;

	if (nVirtual == -1)
	{

		if (!(ch = load_pc (name)))
		{
			system_log ("No such character by name in add_message()!", true);
			return;
		}

		nVirtual = ++ch->pc->staff_notes;
		named = 1;

		unload_pc (ch);
	}

	else if (nVirtual == -2)
	{
		new_message = 4;
	}

	else if (nVirtual == -5)
	{
		new_message = 3;
	}

	else if (nVirtual == -3)
	{
		if (!(dir = opendir (JOURNAL_DIR)))
		{
			perror ("opendir");
			printf ("Unable to open journal directory.");
			return;
		}
		for (i = 1; i <= 5000; i++)
		{
			found = false;
			sprintf (buf, "player_journals/%s.%06d", name, i);
			if (!(fp = fopen (buf, "r")))
				break;
			else
				fclose (fp);
		}
	}
	else if (new_message == 2 || new_message == 3 || new_message == 4
		|| new_message == 5)
		;
	else if (!nVirtual)
		nVirtual = board->next_virtual++;
	else if (nVirtual >= board->next_virtual)
		board->next_virtual = nVirtual + 1;
	else
		system_log
		("Virtual requested less than board's next virtual, add_message()",
		true);

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

	if (new_message != 3 && new_message != 4 && new_message != 5)
		msg->nVirtual = nVirtual;
	else
		msg->nVirtual = i;
	msg->flags = flags;
	msg->poster = duplicateString (poster);
	msg->date = duplicateString (date);
	msg->subject = tilde_eliminator (subject);
	msg->info = duplicateString (info);
	msg->message = tilde_eliminator (message);
	msg->icdate = duplicateString (date_buf);

	if (named || new_message == 2)
		sprintf (msg_file_name, PLAYER_BOARD_DIR "/%s.%06d", name, nVirtual);
	else if (new_message != 2 && new_message != 3 && new_message != 4
		&& new_message != 5 && ch)
	{
		ch->desc->pending_message = msg;
		post_to_mysql_board (ch->desc);
		return;
	}
	else if (new_message == 5)
	{
		sprintf (msg_file_name, JOURNAL_DIR "/%s.%06d", name, i);
	}
	else if (new_message == 3)
	{
		add_message_to_mysql_vboard (name, poster, msg);
		return;
	}
	else if (new_message == 4)
	{
		add_message_to_mysql_player_notes (name, poster, msg);
		return;
	}

	if (!new_message)
	{
		free_mem(msg);
		return;
	}

	system_log ("Reached end of add_message()", true);

	free_mem(msg);
}


char *
read_a_line (FILE * fp)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	fgets (buf, MAX_STRING_LENGTH, fp);

	if (*buf)
		buf[strlen (buf) - 1] = '\0';

	return duplicateString (buf);
}

MESSAGE_DATA *
load_message (char *msg_name, int pc_message, int msg_number)
{
	MESSAGE_DATA *message = NULL;
	FILE *fp_message = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' }, date_buf[MAX_STRING_LENGTH] =
	{
		'\0'}, *suf = '\0';
		int day = 0;
		bool tilde = false;

		*date_buf = '\0';

		if (pc_message == 1)
			sprintf (buf, PLAYER_BOARD_DIR "/%s.%06d", msg_name, msg_number);
		else if (pc_message == 2)
			sprintf (buf, VIRTUAL_BOARD_DIR "/%s.%06d", msg_name, msg_number);
		else if (pc_message == 3)
			sprintf (buf, JOURNAL_DIR "/%s.%06d", msg_name, msg_number);
		else
			sprintf (buf, BOARD_DIR "/%s.%06d", msg_name, msg_number);

		if (pc_message == 5)
		{
			message = load_mysql_message (msg_name, 1, msg_number);
			return message;
		}
		else if (pc_message == 6)
		{
			message = load_mysql_message (msg_name, 0, msg_number);
			return message;
		}
		else if (pc_message == 7)
		{
			message = load_mysql_message (msg_name, 2, msg_number);
			return message;
		}
		else if (pc_message == 8)
		{
			message = load_mysql_message (msg_name, 3, msg_number);
			return message;
		}

		if (!(fp_message = fopen (buf, "r+")))
		{
			printf ("Couldn't open %s\n", buf);
			return NULL;
		}

		message = new MESSAGE_DATA;

		message->nVirtual = msg_number;
		message->poster = read_a_line (fp_message);
		message->date = read_a_line (fp_message);
		message->subject = read_a_line (fp_message);
		message->info = read_a_line (fp_message);
		message->message = fread_string (fp_message);
		fscanf (fp_message, "%ld", &message->flags);

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

		sprintf (date_buf, "%s", read_a_line (fp_message));
		sprintf (date_buf, "%s", read_a_line (fp_message));

		if (!*date_buf)
		{
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
						holiday_short_names[time_info.holiday],
						time_info.year);
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
		}

		if (isalpha (*date_buf))
			*date_buf = toupper (*date_buf);

		if (*date_buf == '~')
			tilde = true;

		while (tilde)
		{
			sprintf (date_buf, "%s", read_a_line (fp_message));
			if (*date_buf != '~')
				tilde = false;
		}

		message->icdate = duplicateString (date_buf);

		fclose (fp_message);

		return message;
}

void
do_notes (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	bool messages = false;
	char name[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, name);

	if (!*name)
	{
		send_to_char
			("Which PC or virtual board did you wish to get a listing for?\n",
			ch);
		return;
	}

	*name = toupper (*name);

	if (!(who = load_pc (name)))
	{
		messages = get_mysql_board_listing (ch, 1, name);
		if (!messages)
		{
			send_to_char ("No such PC or vboard.\n", ch);
			return;
		}
		else
			return;
	}

	unload_pc (who);


	if (ch->pc && ch->pc->level < 3)
	  {
	    send_to_char("You are not permitted to read player notes.\n",ch);
	  }
	else
	  {
	    messages = get_mysql_board_listing (ch, 2, name);

	    if (!messages)
	      send_to_char ("That player does not have any notes.\n", ch);
	  }
}

void
do_journal (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	int messages = 0;
	bool loaded_char = false;

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if (!IS_MORTAL (ch))
	{
		argument = one_argument (argument, name);
		if (!*name)
			who = ch;
		else
		{
			*name = toupper (*name);
			if (!(who = load_pc (name)))
			{
				send_to_char ("No such PC, I'm afraid.\n", ch);
				return;
			}
			loaded_char = true;
		}
	}
	else
		who = ch;

	messages = get_mysql_board_listing (ch, 3, who->tname);
	if (!messages)
		send_to_char ("No journal entries found.\n", ch);

	if (who && who != ch && loaded_char)
		unload_pc (who);
}

void
do_notify (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	bool notify = false;
	CLAN_DATA *clan;

	if (IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC))
	{
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		send_to_char ("Guests cannot use the notify command.\n", ch);
		return;
	}

	if (!GET_TRUST (ch))
		send_to_char
		("#6Note:  This command is here solely to facilitate RP, and is therefore strictly OOC.#0\n",
		ch);

	while (*argument == ' ')
		argument++;

	strcpy (buf, argument);

	while (*buf && *(buf + strlen (buf) - 1) == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (!*buf)
	{

		if ((af = get_affect (ch, MAGIC_CLAN_NOTIFY)))
		{

			if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			{
				send_to_char ("\nThey're no longer online.\n", ch);
				affect_remove (ch, af);
				return;
			}

			send_to_char ("\nClan member has been notified.\n", ch);

			sprintf (buf, "#3[%s is online.]#0\n", char_short (ch));
			buf[3] = toupper (buf[3]);

			send_to_char (buf, (CHAR_DATA *) af->a.spell.t);

			if (!IS_SET (((CHAR_DATA *) af->a.spell.t)->plr_flags, MUTE_BEEPS))
				send_to_char ("\a", (CHAR_DATA *) af->a.spell.t);

			affect_remove (ch, af);

			return;
		}

		if (!(af = get_affect (ch, MAGIC_NOTIFY)))
		{
			send_to_char ("\nNobody has notified you that they were online.\n",
				ch);
			return;
		}

		if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
		{
			send_to_char ("\nThey're no longer online.\n", ch);
			affect_remove (ch, af);
			return;
		}

		send_to_char ("#3[Notifyee is online.]#0\n",
			(CHAR_DATA *) af->a.spell.t);
		send_to_char ("Requesting party notified.\n", ch);

		affect_remove (ch, af);

		return;
	}

	if (is_clan_member_player (ch, buf))
	{

		send_to_char
			("\nAll clan members currently online have been notified.\n", ch);
		sprintf (buf2, "#3[%s (%s) is online.  Use NOTIFY to reply in kind.]#0",
			char_short (ch),
			(((clan =
			get_clandef_long (buf)) != NULL) ? clan->literal : buf));
		buf2[3] = toupper (buf2[3]);

		//for (tch = character_list; tch; tch = tch->next)
		for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;

			if (ch == tch || tch->deleted || IS_NPC (tch))
				continue;

			if (!is_clan_member_player (tch, buf))
				continue;

			act (buf2, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);

			magic_add_affect (tch, MAGIC_CLAN_NOTIFY, 2, 0, 0, 0, 0);

			get_affect (tch, MAGIC_CLAN_NOTIFY)->a.spell.t = (long int) ch;
		}

		return;
	}

	if (!(tch = get_pc (buf)))
	{
		if (!(tch = load_pc (buf)))
		{
			mysql_safe_query
				("SELECT name FROM %s.pfiles WHERE keywords LIKE \'%%%s%%\'",
				(engine.get_config ("player_db")).c_str (), buf);
			result = mysql_store_result (database);
			if (!result || mysql_num_rows (result) <= 0)
			{
				mysql_safe_query
					("SELECT name FROM %s.pfiles WHERE sdesc LIKE \'%%%s%%\'",
					(engine.get_config ("player_db")).c_str (), buf);
				result = mysql_store_result (database);
			}
			if (result && mysql_num_rows (result) > 0
				&& mysql_num_rows (result) <= 50)
			{
				while ((row = mysql_fetch_row (result)))
				{
					if ((tch = get_pc (row[0])))
					{
						sprintf (buf,
							"\a#3[%s is online.  Use NOTIFY to reply in kind.]#0\n",
							char_short (ch));
						buf[4] = toupper (buf[4]);
						send_to_char (buf, tch);
						if (!IS_SET (tch->plr_flags, MUTE_BEEPS))
							send_to_char ("\a", tch);
						if ((af = get_affect (tch, MAGIC_NOTIFY)))
						{
							af->a.spell.t = (long int) ch;
							af->a.spell.duration = 2;
							if (result)
								mysql_free_result (result);
							return;
						}
						magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);
						get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (long int) ch;
						notify = true;
					}
				}
				if (result)
					mysql_free_result (result);
				if (!IS_MORTAL (ch))
				{
					if (notify)
						send_to_char ("\nParty was found online and notified.\n",
						ch);
					else
						send_to_char
						("\nNo PC online has such a description or alias.  Did you spell it fully and correctly?\n",
						ch);
				}
				else
				{
					send_to_char
						("\nIf this individual is online they have been notified.\n",
						ch);
				}
				return;
			}
			if (result && mysql_num_rows (result) > 50)
			{
				send_to_char
					("\nToo many matches were found. Please try elaborating on the description.\n",
					ch);
				mysql_free_result (result);
				return;
			}

			if (!IS_MORTAL (ch))
				send_to_char
				("\nNo PC has such a name.  Did you spell it fully and correctly?\n",
				ch);
			else
				send_to_char
				("\nIf this individual is online they have been notified.\n",
				ch);
			return;
		}

		unload_pc (tch);
		send_to_char
			("\nIf this individual is online they have been notified.\n", ch);
		return;
	}

	send_to_char ("\nIf this individual is online they have been notified.\n",
		ch);

	sprintf (buf, "#3[%s is online.  Use NOTIFY to reply in kind.]#0\n",
		char_short (ch));
	buf[3] = toupper (buf[3]);

	send_to_char (buf, tch);

	if (!IS_SET (tch->plr_flags, MUTE_BEEPS))
		send_to_char ("\a", tch);

	if ((af = get_affect (tch, MAGIC_NOTIFY)))
	{
		af->a.spell.t = (long int) ch;
		af->a.spell.duration = 2;
		return;
	}

	magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);

	get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (long int) ch;
}

void
post_writing (DESCRIPTOR_DATA * d)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *quill = NULL;
	CHAR_DATA *ch = NULL;
	OBJ_DATA *ink = NULL;
	WRITING_DATA *writing = NULL;
	unsigned int i = 0;
	time_t current_time = 0;
	char date[AVG_STRING_LENGTH] = "";
	char message[MAX_STRING_LENGTH] = "";
	float mod = 0;

	if (!d->pending_message || !d->pending_message->message || !*d->pending_message->message)
		return;

	ch = d->character;

	if (!(obj = ch->pc->writing_on))
	{
		send_to_char ("That object is no longer there!\n", ch);
		return;
	}

	current_time = time (0);
	ctime_r (&current_time, date);
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	sprintf (message, "%s", d->pending_message->message);

	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_INK)
	{
		ink = ch->right_hand;
		if (ink->o.od.value[1] <= 0)
		{
			send_to_char
				("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n",
				ch);
			extract_obj (ink);
		}
	}

	if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_INK)
	{
		ink = ch->left_hand;
		if (ink->o.od.value[1] <= 0)
		{
			send_to_char
				("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n",
				ch);
			extract_obj (ink);
		}
	}

	if ((quill = ch->right_hand))
	{
		if (GET_ITEM_TYPE (quill) == ITEM_WRITING_INST)
		{
			if (quill->ink_color)
				quill->ink_color = NULL;
		}
		else
			quill = NULL;
	}

	if (!quill && ch->left_hand)
	{
		quill = ch->left_hand;
		if (GET_ITEM_TYPE (quill) == ITEM_WRITING_INST)
		{
			if (quill->ink_color)
				quill->ink_color = NULL;
		}
		else
			quill = NULL;
	}

	mod =
		(skill_level(ch, ch->writes, 0) * 0.70) +
		(skill_level(ch, ch->speaks, 0) * 0.30);


	mod = (float) MIN (95, (int) mod);

	if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
	{
		obj->writing = new WRITING_DATA;
		obj->writing->next_page = NULL;
		obj->writing->ink = duplicateString (ch->delay_who);
		obj->writing->author = duplicateString (ch->tname);
		obj->writing->date = duplicateString (date);
		obj->writing->language = ch->speaks;
		obj->writing->script = ch->writes;
		obj->writing->message = tilde_eliminator (message);
		obj->writing->skill = (int) mod;
	}

	if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
	{
		for (i = 2, writing = obj->writing; i <= obj->open; i++)
		{
			if (obj->writing->next_page)
			{
				writing = writing->next_page;
			}
		}
		writing->ink = duplicateString (ch->delay_who);
		writing->author = duplicateString (ch->tname);
		writing->date = duplicateString (date);
		writing->language = ch->speaks;
		writing->script = ch->writes;
		writing->message = tilde_eliminator (message);
		writing->skill = (int) mod;
	}

	ch->pc->writing_on = NULL;
	ch->delay_who = NULL;
	free_mem(d->pending_message);
	d->pending_message = NULL;

	skill_use (ch, ch->writes, 0);
	if (!number (0, 1))
		skill_use (ch, ch->speaks, 0);

	save_writing (obj);
}

void
do_flip (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		send_to_char ("What did you want to flip?\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) != ITEM_BOOK)
	{
		send_to_char ("That isn't a book, unfortunately.\n", ch);
		return;
	}

	if (!obj->open)
	{
		send_to_char ("You'll need to open it, first.\n", ch);
		return;
	}

	if (!obj->o.od.value[0])
	{
		send_to_char ("It doesn't have any pages left to flip.\n", ch);
		return;
	}

	if (*argument && !isdigit (*argument))
	{
		send_to_char ("Which page would you like to flip to?\n", ch);
		return;
	}

	unsigned int page_arg=0;
	if (!*argument)
	{
		page_arg = obj->open + 1;
	}
	else
	{
		page_arg = strtol (argument, NULL, 10);
	}

	if (page_arg == obj->open)
	{
		send_to_char ("It's already open to that page.\n", ch);
		return;
	}

	if (page_arg > obj->o.od.value[0])
	{
		sprintf (buf, "There are only %d pages in this book.\n",
			obj->o.od.value[0]);
		send_to_char (buf, ch);
		return;
	}

	if (page_arg > obj->open + 1)
		sprintf (buf,
		"You leaf carefully through #2%s#0 until you arrive at page %d.",
		obj->short_description, page_arg);
	else
		sprintf (buf, "You turn #2%s's#0 page.", obj->short_description);

	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	if (page_arg > obj->open + 1)
		sprintf (buf,
		"%s#0 leafs carefully through #2%s#0 until %s arrives at the desired page.",
		char_short (ch), obj->short_description, HSSH (ch));
	else
		sprintf (buf, "%s#0 turns #2%s's#0 page.", char_short (ch),
		obj->short_description);
	sprintf (buffer, "#5%s", CAP (buf));
	act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
	obj->open = page_arg;
}

void
do_tear (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *parchment = NULL;
	WRITING_DATA *writing = NULL;
	WRITING_DATA *page = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	unsigned int i = 0;

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		send_to_char ("What did you wish to tear?\n", ch);
		return;
	}

	if (ch->right_hand && ch->left_hand)
	{
		send_to_char ("You must have one hand free.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) != ITEM_PARCHMENT
		&& GET_ITEM_TYPE (obj) != ITEM_BOOK)
	{
		send_to_char ("I'm afraid that can't be torn.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
	{
		if (!obj->writing_loaded)
			load_writing (obj);
		if (!obj->open)
		{
			send_to_char ("Open it to the page you wish to tear out, first.\n",
				ch);
			return;
		}
		if (!obj->writing || !obj->o.od.value[0])
		{
			send_to_char ("It doesn't have any pages left!\n", ch);
			return;
		}
		if (!(parchment = load_object (61)))
		{
			send_to_char
				("The parchment prototype (VNUM 61) appears to be missing. Please inform staff.\n",
				ch);
			return;
		}
		for (i = 1, writing = obj->writing; i <= obj->open; i++)
		{
			if (i == 1 && i == obj->open)
			{
				sprintf (buf, "You carefully tear page %d from #2%s#0.", i,
					obj->short_description);
				page = writing;
				if (!obj->writing->next_page)
				{
					obj->writing = NULL;
				}
				else
					obj->writing = writing->next_page;
				break;
			}
			else if (i + 1 == obj->open && obj->open > 1)
			{
				sprintf (buf, "You carefully tear page %d from #2%s#0.", i + 1,
					obj->short_description);
				page = writing->next_page;
				if (!writing->next_page->next_page)
				{
					writing->next_page = NULL;
					obj->open--;
				}
				else
				{
					writing->next_page = writing->next_page->next_page;
				}
				break;
			}
			writing = writing->next_page;
		}

		act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "%s#0 carefully tears a page from #2%s#0.",
			char_short (ch), obj->short_description);
		sprintf (buffer, "#5%s", CAP (buf));
		act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

		if (*page->message && strcasecmp (page->message, "blank") != STR_MATCH)
		{
			parchment->writing = new WRITING_DATA;
			parchment->writing->next_page = NULL;
			parchment->writing->ink = duplicateString (page->ink);
			parchment->writing->author = duplicateString (page->author);
			parchment->writing->date = duplicateString (page->date);
			parchment->writing->language = page->language;
			parchment->writing->script = page->script;
			parchment->writing->message = duplicateString (page->message);
			parchment->writing->skill = page->skill;;
		}

		obj->o.od.value[0]--;
		obj_to_char (parchment, ch);
		save_writing (obj);
		save_writing (parchment);
		return;
	}

	sprintf (buf,
		"You rend #2%s#0 into small pieces, which you then meticulously discard.",
		obj->short_description);
	act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
	sprintf (buf,
		"%s rends #2%s#0 into small pieces, which %s then meticulously discards.",
		char_short (ch), obj->short_description, HSSH (ch));
	sprintf (buffer, "#5%s", CAP (buf));
	act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);

	extract_obj (obj);
}

void
do_dip (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *quill = NULL, *ink = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!
		(ch->left_hand
		&& ch->left_hand->obj_flags.type_flag == ITEM_WRITING_INST)
		&& !(ch->right_hand
		&& ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST))
	{
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (ch->right_hand
			&& GET_ITEM_TYPE (ch->right_hand) == ITEM_WRITING_INST)
			quill = ch->right_hand;
		else
			quill = ch->left_hand;

		if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_INK)
			ink = ch->right_hand;
		else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_INK)
			ink = ch->left_hand;
	}
	else
	{
		quill = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if (!quill)
			quill = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if (quill && GET_ITEM_TYPE (quill) != ITEM_WRITING_INST)
		{
			send_to_char
				("You must specify first the writing implement, and then the ink source.\n",
				ch);
			return;
		}
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			send_to_char
				("You must specify both a writing implement and an ink source.\n",
				ch);
			return;
		}
		ink = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if (!ink)
			ink = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if (!ink)
			ink = get_obj_in_list_vis (ch, buf, ch->room->contents);
		if (ink && GET_ITEM_TYPE (ink) != ITEM_INK)
		{
			send_to_char
				("You must specify first the writing implement, and then the ink source.\n",
				ch);
			return;
		}
	}

	if (!ink)
	{
		send_to_char
			("You need to have an ink source in one hand, or in the room.\n", ch);
		return;
	}

	if (!quill)
	{
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if (ink->o.od.value[0] <= 0)
	{
		send_to_char ("Your ink source seems to be empty.\n", ch);
		return;
	}

	sprintf (buf,
		"You dip #2%s#0 carefully into #2%s#0, liberally coating its tip.",
		quill->short_description, ink->short_description);
	if (ink->var_color)
		quill->ink_color = ink->var_color;
	else
		quill->ink_color = ink->ink_color;

	act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	ink->o.od.value[0]--;
}

void
do_scribe (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *quill = NULL;
	WRITING_DATA *writing = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	unsigned int i = 0;
	int script = 0;

	if (IS_NPC (ch))
	{
		send_to_char ("This is only available to PCs.\n", ch);
		return;
	}

	for (i = SKILL_SCRIPT_SARATI; i <= SKILL_SCRIPT_ANGERTHAS_EREBOR; i++)
		if (ch->skills[i])
			script = 1;

	if (!script)
	{
		send_to_char ("You have no knowledge of any written scripts.\n", ch);
		return;
	}

	if (!ch->writes)
	{
		send_to_char
			("In which script would you like to write? (See the SCRIBE command.)\n",
			ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		send_to_char ("You can't find that!\n", ch);
		return;
	}

	if (!ch->left_hand && !ch->right_hand)
	{
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WRITING_INST)
		quill = ch->right_hand;
	else if (ch->left_hand
		&& GET_ITEM_TYPE (ch->left_hand) == ITEM_WRITING_INST)
		quill = ch->left_hand;
	else
	{
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if (!quill->ink_color)
	{
		send_to_char ("The writing instrument must be dipped in ink, first.\n",
			ch);
		return;
	}

	ch->delay_who = quill->ink_color;

	if (obj->obj_flags.type_flag != ITEM_PARCHMENT &&
		obj->obj_flags.type_flag != ITEM_BOOK)
	{
		act ("That cannot be written on.", false, ch, obj, 0, TO_CHAR);
		return;
	}

	if (!obj->writing_loaded)
		load_writing (obj);

	if (GET_ITEM_TYPE (obj) == ITEM_PARCHMENT && obj->writing)
	{
		send_to_char ("That has already been written on.\n", ch);
		return;
	}

	else if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
	{
		if (!obj->open)
		{
			send_to_char ("You need to open it, first.\n", ch);
			return;
		}
		if (!obj->writing || !obj->o.od.value[0])
		{
			send_to_char ("It doesn't have any pages left!\n", ch);
			return;
		}
		for (i = 2, writing = obj->writing; i <= obj->open; i++)
		{
			if (obj->writing->next_page)
			{
				writing = writing->next_page;
			}
		}
		if (strcasecmp (writing->message, "blank") != STR_MATCH)
		{
			send_to_char ("That has already been written on.\n", ch);
			return;
		}
	}

	sprintf (buffer, "#5%s#0 begins writing on #2%s#0.", char_short (ch),
		obj->short_description);
	buffer[3] = toupper (buffer[3]);
	act (buffer, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	send_to_char
		("Scribe your message; terminate with an '@'. Please keep its length plausible\nfor the size of the writing object, since we have opted against coded limits.\n",
		ch);
	sprintf (buf,
		"1-------10--------20--------30--------40--------50--------60--------70\n");
	send_to_char (buf, ch);

	make_quiet (ch);

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_writing;
	ch->pc->writing_on = obj;
}

void
do_ticket (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[AVG_STRING_LENGTH];
	char buf2[AVG_STRING_LENGTH];
	char f_tick[AVG_STRING_LENGTH];
	char l_tick[AVG_STRING_LENGTH];
	int first_tick;
	int last_tick;
	int tick_num = 1;
	int roomnum = 0;

	argument = one_argument(argument, buf);

	if (!*buf)
	{
		send_to_char ("ticket read <number>\n", ch);
		send_to_char ("ticket browse <first number> <last number>\n", ch);
		send_to_char ("ticket delete <ticket number>\n", ch);
		send_to_char ("ticket search <value> owner | mount | room\n", ch);
		send_to_char ("To search by owner's name, mount's name or ostler's room number.\n", ch);
		return;
	}

	if (!strcmp(buf, "browse"))
	{
		argument = one_argument (argument, f_tick);
		if (!*f_tick)
		{
			send_to_char ("Starting with which ticket?\n", ch);
			return;
		}

		if (!isdigit (*f_tick))
		{
			send_to_char ("You must specify the first ticket number\n", ch);
			return;
		}


		argument = one_argument (argument, l_tick);
		if (!*l_tick)
		{
			send_to_char ("ending at which ticket?\n", ch);
			return;
		}

		if (!isdigit (*l_tick))
		{
			send_to_char ("You must specify the last ticket number.\n", ch);
			return;
		}

		//No more than 1000 tickets to be checked at a time, instead of the whole 10 million (9,999,999). May need to adjust if there really are more tickets
		first_tick = atoi(f_tick);
		last_tick = MIN(1000, atoi(l_tick));

		if (first_tick >= last_tick)
		{
			send_to_char ("The last number must be larger than the first number, and cannot exceed 1000.\n", ch);
			return;
		}

		//browse_ticket(ch, first_tick, last_tick);
		for (tick_num = first_tick; tick_num <= last_tick; tick_num ++)
		{
			read_ticket(ch, tick_num, 0);
		};

	} //if browse

	if (!strcmp(buf, "read"))
	{
		argument = one_argument (argument, buf2);
		if (!*buf2)
		{
			send_to_char("Which ticket did you wish to read?\n", ch);
			return;
		}

		if (isdigit (*buf2))
		{
			tick_num = atoi (buf2);
		}
		else
		{
			send_to_char("You must use the ticket number.\n", ch);
			return;
		}

		read_ticket(ch, tick_num, 1);
	}//read


	if (!strcmp(buf, "delete"))
	{
		argument = one_argument (argument, buf2);
		if (!*buf2)
		{
			send_to_char("Which ticket did you wish to delete?\n", ch);
			return;
		}

		if (isdigit (*buf2))
		{
			tick_num = atoi (buf2);
		}
		else
		{
			send_to_char("You must use the ticket number.\n", ch);
			return;
		}

		delete_ticket(ch, tick_num);
	}//delete

	if (!strcmp(buf, "search"))
	{
		argument = one_argument (argument, buf2);
		
		if (!strncmp(argument, "owner", 5)) //case 1
		{
			if (!*buf2)
			{
				send_to_char("Whose tickets did you wish to read?\n", ch);
				return;
			}
			
			sprintf(buf, "#2Mounts owned by#0 %s\n\n", CAP(buf2));
			send_to_char (buf, ch);
			search_ticket(ch, buf2, 1);
		}//owner
		
		if (!strncmp(argument, "mount", 5)) //case 2
		{
				//argument = one_argument (argument, buf2);
			if (!*buf2)
			{
				send_to_char("What is the name of the mount you wish to find?\n", ch);
				return;
			}
			
			sprintf(buf, "#2Mount named#0 %s\n\n", (buf2));
			send_to_char (buf, ch);
			search_ticket(ch, CAP(buf2), 2);
		}//mount name
		
		if (!strncmp(argument, "room", 4)) //case 3
		{
				//argument = one_argument (argument, buf2);
			if (!*buf2)
			{
				send_to_char("What is the room number of the stable you wish to search?\n", ch);
				return;
			}
			if (!is_number(buf2))
			{
				send_to_char("What is the room number of the stable you wish to search?\n", ch);
				return;
			}
			
			roomnum = atoi(buf2);
			sprintf(buf, "#2Mounts in stable named#0 %s\n\n", vtor(roomnum)->name);
			send_to_char (buf, ch);
			search_ticket(ch, buf2, 3);
		}//mount name
	} //end search			
	
	
	return;
}

void
read_ticket (CHAR_DATA * ch, int tick_num, int cmd)
{

	int nVirtual;
	int loaded;
	CHAR_DATA *mob;
	FILE *fp;
	char buf[AVG_STRING_LENGTH];
	char buf2[AVG_STRING_LENGTH] = {'\0'};
	char hookup[AVG_STRING_LENGTH];
	char name[AVG_STRING_LENGTH];


	sprintf (name, TICKET_DIR "/%07d", tick_num);

	if (!(fp = fopen (name, "r")))
	{
		sprintf(buf, "Ticket %d not found or mount has been unstabled.\n", tick_num);
		send_to_char(buf, ch);
		return;
	}

	sprintf(buf2, "\nTicket number: %d\n", tick_num);

	while (fgets (buf, 256, fp))
	{
		//skip lines with blank space at start or blank lines
		if (*buf == ' ' || *buf == '\n')
			fgets (buf, 255, fp);

		//Look for the MOB
		if (sscanf (buf, "%d %s", &nVirtual, hookup) != 2)
		{
			fclose (fp);
			system_log ("The ticket system is broken, read_ticket() - mob", true);
			sprintf(buf2 + strlen(buf2), "Bad file format-mob\n");
			send_to_char(buf2, ch);
			return;
		}

		mob = load_a_saved_mobile (nVirtual, fp, true);

		if (mob)
		{
			loaded = 1;
			sprintf(buf2 + strlen(buf2), "Vnum: %d \nNamed: %s \nClans: %s \nOwner: %s \nStabled at: %s (%d) \n", mob->mob->nVirtual, mob->name, mob->clans, mob->mob->owner, vtor(mob->in_room)->name, mob->in_room);

			if ((mob->equip) && (cmd == 1))
			{
				show_char_to_char (mob, ch, 1); //shows description and gear
			}
			
			if (loaded)
			save_mobile (mob, fp, "HITCH", 1);	/* Extracts the mobile */
		}
		else
		{
			sprintf(buf2 + strlen(buf2), "Mount not found - Ticket data has been removed\n");
		}


		fclose (fp);
		page_string (ch->desc, buf2);
		return;
	}

	fclose (fp);
	page_string (ch->desc, buf2);
	return;
}


void
delete_ticket (CHAR_DATA * ch, int tick_num)
{

	FILE *fp;
	char buf[AVG_STRING_LENGTH];
	char buf2[AVG_STRING_LENGTH] = {'\0'};
	char name[AVG_STRING_LENGTH];


	sprintf (name, TICKET_DIR "/%07d", tick_num);

	if (!(fp = fopen (name, "r")))
	{
		return;
	}

	sprintf(buf2, "\nTicket number %d deleted\n", tick_num);

	sprintf(buf, "rm %s", name);
	system (buf);

	fclose (fp);

	send_to_char(buf2, ch);
	return;
}

void
search_ticket (CHAR_DATA * ch, char * chkvalue, int searchtype)
{
	
	int nVirtual;
	int index;
	CHAR_DATA *mob;
	char name[AVG_STRING_LENGTH];
	char hookup[AVG_STRING_LENGTH];
	FILE *fp;
	char buf[AVG_STRING_LENGTH] = {'\0'};
	char buf2[MAX_STRING_LENGTH] = {'\0'};
	int roomchk;
	int loaded = 0;  
	
	for (index = 1; index < LAST_STABLE_TICKET; index ++)
  	{
  		sprintf (name, TICKET_DIR "/%07d", index);
  		
  		if (!(fp = fopen (name, "r")))
		{
			continue;
		}
    	while (fgets (buf, 256, fp))
		{
				//skip lines with blank space at start or blank lines
			if (*buf == ' ' || *buf == '\n')
				fgets (buf, 255, fp);
			
			if (sscanf (buf, "%d %s", &nVirtual, hookup) != 2)
			{
				continue;
			}	
			else
			{
				mob = load_a_saved_mobile (nVirtual, fp, true);
				if (mob)
				{
					loaded = 1;
					switch (searchtype)
					{
						case 1: //by owner
							if(mob->mob->owner && strcmp (mob->mob->owner, CAP(chkvalue)) == 0)
							{
							sprintf(buf2 + strlen(buf2), "#2Ticket: %d#0\nVnum: %d \nNamed: %s \nClans: %s nOwner: %s \nStabled at: %s (%d) \n\n",
										index,
										mob->mob->nVirtual,
										mob->name,
										mob->clans,
										mob->mob->owner,
										vtor(mob->in_room)->name,
										mob->in_room);		
							}
							
							break;
							
						case 2: //mount name
							if (isname(chkvalue, mob->name))											
							{
								sprintf(buf2 + strlen(buf2), "#2Ticket: %d#0\nVnum: %d \nNamed: %s \nClans: %s \nOwner: %s \nStabled at: %s (%d) \n\n",
										index,
										mob->mob->nVirtual,
										mob->name,
										mob->clans,
										mob->mob->owner,
										vtor(mob->in_room)->name,
										mob->in_room);		
							}
							break;
							
						case 3: //stable
							roomchk = atoi(chkvalue);
							
							if (!roomchk)
								break;
							
							if(roomchk == mob->in_room)
							{
								sprintf(buf2 + strlen(buf2), "#2Ticket: %d#0\nVnum: %d \nNamed: %s \nClans: %s \nOwner: %s \nStabled at: %s (%d) \n\n",
										index,
										mob->mob->nVirtual,
										mob->name,
										mob->clans,
										mob->mob->owner,
										vtor(mob->in_room)->name,
										mob->in_room);		
							}
							break;
							
						default:
							break;
					}//switch
					if (loaded)
						save_mobile (mob, fp, "HITCH", 1);	/* Extracts the mobile */
				} //if (mob)
			}
		}//while    
		fclose (fp);
  	} //for
	
 	page_string (ch->desc, buf2); 
	
	
	return;
}


/***
do_evaluate

This command provides detailed information about a held object
including weight, cost and any skill affects, as well as other
object specific information such as time left for light objects
and liquid left in drinks containers.

- Valarauka

***/
void
do_evaluate (CHAR_DATA *ch, char *argument, int cmd)
{


	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj = NULL;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };

	/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( GET_POS (ch) < POSITION_SLEEPING ) {
		send_to_char ("You are unconscious!\n", ch);
		return;
	}

	if ( GET_POS (ch) == POSITION_SLEEPING ) {
		send_to_char ("You are asleep.\n", ch);
		return;
	}

	if ( is_blind (ch) ) {
		send_to_char ("You are blind!\n", ch);
		return;
	}

	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		send_to_char ("Evaluate what?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->room->contents))) {

			send_to_char ("You don't have that.\n", ch);
			return;
	}

	/*** Describe the object ***/
	if (obj) {
		snprintf (buffer, MAX_STRING_LENGTH,  "\nIt is #2%s#0", obj->short_description);
		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
	}
	else{
		send_to_char ("You cannot evaluate something unless you have it.\n", ch);
		return;
	}

	send_to_char ("\n", ch);

	show_evaluate_information(ch, obj);

	return;
}

void
show_evaluate_information (CHAR_DATA *ch, OBJ_DATA	*obj)
{
	int 		variance = 0;
	int 		guess_weight = 0;
	int 		guess_value = 0;
	int			temp = 0;
	int			i = 0;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;
	AFFECTED_TYPE *af;

	/*** WEAR LOCATIONS FOR WEARABLE ITEMS ***/

	if (obj->obj_flags.wear_flags)
	{
		temp = 0;
		for (i = 0; (*wear_bits[i] != '\n'); i++)
		{
			if ((IS_SET (obj->obj_flags.wear_flags, (1 << i)))
				&& (strcmp (wear_bits[i],"Take"))
				&& (strcmp (wear_bits[i],"Unused"))) //dont want to show Take and Unused
			{
				//if found something to write and not already written inital string, write it
				if(temp == 0)
				{
					sprintf (buffer, "\nYou recognise that you could wear this item in the following locations:\n");
					temp = 1;
				}

				sprintf (buffer + strlen (buffer), "  #6%s#0\n", wear_bits[i]);
			}
		}

		send_to_char (buffer, ch);
		send_to_char ("\n", ch);
	}

	/*** END WEAR LOCATIONS ***/

	/**** Guess at the Weight based on scan skill **/


	if (IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE)){

		/*
		Rather than give the exact weight to the player, a random factor
		is introduced to replicate a rough estimate. The scan
		skill is used to determine how far from the actual weight the value given
		may deviate so that those with a high scan skill will be fairly accurate,
		where as those with a low scan skill might be quite far from the actual weight.
		The heavier an item, the harder it will be to get a very accurate estimate.

		In addition, a skill check must then be passed to allow the player to see the
		estimated weight.
		*/

		/*** Determine how far estimate may vary ***/
		if(ch->skills[SKILL_SCAN]>70)
		{
			variance = number (95, 105);
		}
		else if(ch->skills[SKILL_SCAN]>60)
		{
			variance = number (92, 108);
		}
		else if(ch->skills[SKILL_SCAN]>50)
		{
			variance = number (90, 110);
		}
		else if(ch->skills[SKILL_SCAN]>40)
		{
			variance = number (87, 113);
		}
		else
		{
			variance = number (85, 115);
		}

		/*** Calculate estimate ***/
		guess_weight = (int)((obj->obj_flags.weight + obj->contained_wt)/variance);

		if (skill_use(ch, SKILL_SCAN, 0)){
			if (guess_weight <= 1){
				snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item weighs less than a pound.");
			}
			else{  /** weighs more than a pound **/
				snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item weighs about %d pounds.",guess_weight);
			}
		}
		else{ /** failed skill check **/
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can't even begin to guess how much this weighs.");
		}
	}
	else{ /** no way to check weight - message for non-takeable objects **/
		snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can't even begin to guess how much this weighs.");
	}

	act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	send_to_char ("\n", ch);

	/*** end weight guess ***/

	/*** Guess at value based on barter skill ***/

	/*
	Rather than give the exact cost to the player, a random factor
	is introduced to replicate a rough estimate of the cost. The barter
	skill is used to determine how far from the actual cost the value given
	may deviate so that those with a high barter skill will be fairly accurate,
	where as those with a low barter skill might be quite far from the actual cost.
	The more expensive an item, the harder it will be to get a very accurate estimate.

	In addition, a skill check must then be passed to allow the player to see the cost.
	*/

	/*** Determine how far estimation may vary ***/
	if(ch->skills[SKILL_BARTER]>70)
	{
		variance = number (95, 105);
	}
	else if(ch->skills[SKILL_BARTER]>60)
	{
		variance = number (92, 108);
	}
	else if(ch->skills[SKILL_BARTER]>50)
	{
		variance = number (90, 110);
	}
	else if(ch->skills[SKILL_BARTER]>40)
	{
		variance = number (87, 113);
	}
	else
	{
		variance = number (85, 115);
	}

	/*** Calculate estimate ***/
	guess_value = (int)(((obj->farthings + obj->silver *4)*100)/variance);

	if (skill_use(ch, SKILL_BARTER, 0)){/*** Passed Skill Check ***/
		if (guess_value <= 1){
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item is worth less than 1 copper.");
		}
		else{
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou would guess that this item is worth around %d coppers.", guess_value);
		}
	}
	else{/*** Failed Skill Check ***/
		snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can't even begin to guess the value of this item.");
	}

	act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	send_to_char ("\n", ch);

	/*** end value guess ***/


	/***** Pslim lantern code ***/
	if ( GET_ITEM_TYPE (obj) == ITEM_LIGHT ) {
		if ( obj->o.light.hours <= 0 ){
			snprintf (buffer, MAX_STRING_LENGTH,  "\nThe $o is empty.");
		}
		else {
			temp = obj->o.light.hours;
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou think the $o will last another %d hours.", temp);
		}
		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);
	}
	/***** end Pslim lantern code *****/

	/**** SHOW CAPACITY OF CONTAINER ****/

	if (GET_ITEM_TYPE (obj) == ITEM_CONTAINER)
	{

		sprintf (buffer, "\nYou estimate that it would hold around %d.%.2d lbs",
			obj->o.container.capacity / 100,
			obj->o.container.capacity % 100);
		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);
	}

	if ((GET_ITEM_TYPE(obj) == ITEM_DRINKCON))
	{
		if (obj->o.drinkcon.capacity > 150)
			sprintf(buffer, "\nYou estimate that it would hold around %d gallons of fluid.", obj->o.drinkcon.capacity / 50);
		else
			sprintf(buffer, "\nYou estimate that it would hold around %d fluid ounces.", obj->o.drinkcon.capacity * 12 / 5);
		act(buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);
	}

	/*** END CAPACITY CODE ***/


	/**** LOOK INSIDE A DRINK CONTATINER ***/

	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON ) {
		if ( obj->o.drinkcon.volume <= 0 ){
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is empty.",
				obj->short_description);
		}
		else {
			if ( obj->o.drinkcon.capacity ) {
				temp = (obj->o.drinkcon.volume * 3) / obj->o.drinkcon.capacity;
			}
			else{
				temp = 1;
			}
			temp_arg = vnum_to_liquid_name (obj->o.drinkcon.liquid);
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is %sfull of %s.",
				obj->short_description, fullness [temp], temp_arg);

		}
		act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		send_to_char ("\n", ch);

	}
	/*** END DRINK CONTAINER ***/


	/**** LOOK INSIDE A NON-DRINK CONTAINER ***/

	if ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
		GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
		GET_ITEM_TYPE (obj) == ITEM_SHEATH ||

		(GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
		obj->o.weapon.use_skill == SKILL_SLING) ||

		GET_ITEM_TYPE (obj) == ITEM_KEYRING ) {

			if ( IS_SET (obj->o.container.flags, CONT_CLOSED) ) {
				send_to_char ("\nIt is closed.", ch);
				return;
			}

			send_to_char ("\nContents :\n", ch);

			list_obj_to_char (obj->contains, ch, 1, true);
	}
	/* If not a container, no need to show any message here */

	/*** END NON_DRINK CONTAINER ***/

	/*** SHOW TIME TO DECAY FOR FOOD ***/

	if(GET_ITEM_TYPE (obj) == ITEM_FOOD)
	{
		if (obj->morphTime)
		{
			int delta, days, hours, minutes;

			delta = obj->morphTime - time (0);

			days = delta / 86400;
			delta -= days * 86400;

			hours = delta / 3600;
			delta -= hours * 3600;

			minutes = delta / 60;

			//write appropriate message for length of time left till decay
			if(days > 1)
			{
				sprintf (buffer,
					"\nYou notice that %s appears fresh with no sign of decay.", obj->short_description);
			}
			else if(hours > 12)
			{
				sprintf (buffer,
					"\nYou notice that %s still appears fresh, though it is slowly beginning to lose its original unsullied appearance.",
					obj->short_description);
			}
			else if(hours > 1)
			{
				sprintf (buffer,
					"\nWhile still in good condition, %s no longer appears fresh.", obj->short_description);
			}
			else
			{
				sprintf (buffer,
					"\nYou notice that %s has already begun to decay in places and it will not be too long before it is completely rotten.",
					obj->short_description);
			}

			act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buffer = '\0';
			send_to_char ("\n", ch);
		}
	}

	/*** END FOOD DECAY TIME ***/

	/*** DETAIL ANY SKILLS AFFECTED BY THE OBJECT ***/

	for (af = obj->xaffected; af; af = af->next)
	{
		if (af->type == MAGIC_HIDDEN)
			continue;
		if (af->a.spell.location >= 10000)
		{
			if(af->a.spell.modifier < 0)
			{
				sprintf (buffer, "\nYou judge that this item would hinder your %s skill.",
					skills[af->a.spell.location - 10000]);
			}
			else
			{
				sprintf (buffer, "\nYou judge that this item would improve your %s skill.",
					skills[af->a.spell.location - 10000]);
			}

			act (buffer, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buffer = '\0';
		}
	}

	send_to_char ("\n", ch);

	/*** END SKILL AFFECTS ***/

	return;
}


void do_origins (CHAR_DATA *ch, char *argument, int cmd)
{
	char output[MAX_STRING_LENGTH];
	OBJ_DATA * obj = 0;

	/* check hands then equip list then finally room */
	obj = get_obj_in_list_vis(ch,argument,ch->left_hand);
	if (!obj)
		obj = get_obj_in_list_vis(ch,argument,ch->right_hand);
	if (!obj)
		obj = get_obj_in_list_vis(ch,argument,ch->equip);
	if (!obj)
		obj = get_obj_in_list_vis (ch, argument, ch->room->contents);

	/* complain if it doesn't exist */
	if (!obj)
	{
		send_to_char("I don't see what you're referring to.\n",ch);
		return;
	}

	/* build the initial output */
	sprintf(output, "   It is #2%s#0.\n\n", OBJS (obj, ch));

	/* add the crafts list */
	char* originlist = origins_list(ch,obj);
	sprintf(output + strlen(output), originlist);
	free_mem(originlist);
	send_to_char(output,ch);
}


/* returns line formatted edition of crafts you can use something in */
/* this must be free_mem'd to prevent leaks */
char * origins_list(CHAR_DATA * ch, OBJ_DATA * obj)
{
	char* p = 0;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char output[MAX_STRING_LENGTH];
	int crafts_found = 0;
	AFFECTED_TYPE *af=0;

	/* if you're an imm, search all crafts regardless of those on your avvie */
	SUBCRAFT_HEAD_DATA *tcraft;
	if (!IS_MORTAL (ch))
	{
		for (tcraft = crafts; tcraft; tcraft = tcraft->next)
		{
			if (!craft_produces (tcraft, obj->nVirtual))
				continue;
			if (crafts_found)
				sprintf (buf + strlen (buf), ", ");
			sprintf (buf + strlen (buf), "'%s %s'",tcraft->command,tcraft->subcraft_name);
			crafts_found += 1;
		}
	}
	else /* you're a mortal .. only list from those you know */
	{
		/* search all crafts you have and look for the indicated object */
		for (int i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				continue;
			/* this shouldn't happen since it's in the craft range of affect nums, but might as well check */
			if (!af->a.craft || !af->a.craft->subcraft)
				continue;
			/* if the craft does not produce this item, ignore that craft */
			if (!craft_produces (af->a.craft->subcraft, obj->nVirtual))
				continue;

			/* format prettily */
			if (crafts_found)
				sprintf (buf + strlen (buf), ", ");

			/* print the command and name */
			sprintf (buf + strlen (buf), "'%s %s'",af->a.craft->subcraft->command,af->a.craft->subcraft->subcraft_name);
			crafts_found += 1;
		}
	}

	/* now whether mortal or not, check for any crafts found and stored in buf */
	if (crafts_found)
	{
		sprintf (output,
			"\n   You realize that you could make this item with the following craft%s: %s.\n",
			crafts_found != 1 ? "s" : "", buf);

		/* output reformats the long line into breaks for columns */
		reformat_string (output, &p);
	}

	/* return the duplicateString'd version */
	return p;
}
