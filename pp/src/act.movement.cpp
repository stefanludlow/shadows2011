/*------------------------------------------------------------------------\
|  act.movement.c : Movement Module                   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <stdexcept>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

extern double round (double x);	// from math.h


int nPartyRoom = 0;
char nPartyDir = -1;
CLAN_DATA *ptrPartyClan = NULL;
QE_DATA *quarter_event_list = NULL;

const int movement_loss[] = {
	1,				/* Inside */
	1,				/* City */
	2,				/* Road */
	3,				/* Trail */
	4,				/* Field */
	5,				/* Woods */
	6,				/* Forest */
	7,				/* Hills */
	8,				/* Mountain */
	8,				/* Swamp */
	1,				/* Dock */
	1,				/* Crowsnest */
	4,				/* Pasture */
	5,				/* Heath */
	3,				/* Pit */
	1,				/* Leanto */
	5,				/* Lake */
	15,				/* River */
	9,				/* Ocean */
	7,				/* Reef */
	10				/* Underwater */
};

const char* const relative_dirs[] = {
	"northern",
	"eastern",
	"southern",
	"western",
	"above",
	"below",
};

const char* const dirs[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"outside",
	"inside",
	"\n"
};

const int rev_dir[] = {
	2,
	3,
	0,
	1,
	5,
	4,
	7,
	6
};

const char* const speeds[] = {
	"walk",			/* 1.00 */
	"trudge",			/* 2.50 */
	"pace",			/* 1.60 */
	"jog",			/* 0.66 */
	"run",			/* 0.50 */
	"sprint",			/* 0.33 */
	"immwalk",			/* 0    */
	"swim",			/* 1.60 */
	"\n"
};

const char* const mount_speeds[] = {
	"walk",
	"trot",
	"canter",
	"gallop",
	"run",
	"race",
	"immwalk",
	"swim",
	"\n"
};

const char* const mount_speeds_ing[] = {
	"walking",
	"trotting",
	"cantering",
	"galloping",
	"running",
	"racing",
	"immwalking",
	"swimming",
	"\n"
};

const float move_speeds[] = { 1.00, 2.50, 1.60, 0.66, 0.50, 0.33, 1.60 };

/* NOTICE:  Set the define ENCUMBERANCE_ENTRIES in structs.h if
more entries are added to encumberance_info.  */

const struct encumberance_info enc_tab[] = {
	{300, 0, 0, 0, 0, 1.00, "unencumbered"},
	{500, 5, 1, 0, 0, 1.00, "lightly encumbered"},
	{800, 10, 2, 5, 1, 0.95, "encumbered"},
	{1300, 15, 3, 10, 2, 0.90, "heavily encumbered"},
	{2000, 25, 4, 15, 2, 0.80, "critically encumbered"},
	{9999900, 25, 4, 15, 2, 0.70, "immobile"},
};

const struct fatigue_data fatigue[] = {
	{5, 0.75, "Completely Exhausted"},
	{15, 0.80, "Exhausted"},
	{25, 0.85, "Extremely Tired"},
	{35, 0.90, "Tired"},
	{45, 0.95, "Somewhat Tired"},
	{55, 1.00, "Winded"},
	{75, 1.00, "Somewhat Winded"},
	{90, 1.00, "Relatively Fresh"},
	{999, 1.00, "Completely Rested"}
};

int
drowned (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH];
	float damage = 0.0;
	int roll = 0;

	if (!IS_MORTAL (ch)
		|| IS_SET (ch->affected_by, AFF_BREATHE_WATER)
		|| number (0, 9)
		|| (IS_SET (ch->act, ACT_FLYING) && !(ch->room->sector_type == SECT_UNDERWATER))
		)
	{
		return 0;
	}

	roll = number (1, 4);

	if (roll == 1)
		act
		("Searing needles of pain lance through your rapidly-dimming field of vision as your lungs burn from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 2)
		act
		("Your vision blurs as your lungs scream for air, your heart pounding painfully in your chest.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 3)
		act
		("A numbing, chilly dullness settles in upon your mind as it slowly dies from the lack of air.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	else if (roll == 4)
		act
		("Your limbs and chest burn in agony, and you instinctively open your mouth to gasp for air, only to choke on icy water as it fills your lungs.",
		false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

	act ("Thrashing sluggishly, $n struggles in vain against the water.", false,
		ch, 0, 0, TO_ROOM | _ACT_FORMAT);

	/* Saps both fatigue AND bloodloss points - drowning is bad news; saps fatigue */
	/* faster, so it immobilizes the player, while sapping general damage more slowly */
	/* to repesent how agonizing a death drowning can be. */

	damage = number (15, 35);
	damage /= 100;
	damage = (int) (ch->max_move * damage);

	if (ch->move < (int) damage)
		ch->move = 0;
	else
		ch->move -= (int) damage;

	damage = number (2, 15);
	damage /= 100;
	damage = (int) (ch->max_hit * damage);

	if (damage > 0.0 && general_damage (ch, (int) damage))
	{
		sprintf (buf, "Drowned: %s Room: %d", ch->tname, ch->in_room);
		system_log (buf, true);
		return 1;
	}

	return 0;
}

int
swimming_check (CHAR_DATA * ch)
{
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *troom = NULL;
	float encumb_percent = 0.0, damage = 0.0;
	bool encumbered = false, exhausted = false;

	if (ch->room->sector_type != SECT_RIVER
		&& ch->room->sector_type != SECT_LAKE
		&& ch->room->sector_type != SECT_OCEAN
		&& ch->room->sector_type != SECT_REEF
		&& ch->room->sector_type != SECT_UNDERWATER)
		return 0;

	if (!IS_MORTAL (ch))
		return 0;

	/* IS_ENCUMBERED() won't work, since we want to check if the PC is 'lightly encumbered' */
	/* or above, not fully encumbered or worse. */

	encumbered = (GET_STR (ch) * enc_tab[0].str_mult_wt < IS_CARRYING_W (ch));

	exhausted = (ch->move <= 10);

	encumb_percent =
		1.0 - (float) (CAN_CARRY_W (ch) -
		IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);
	encumb_percent *= 100;

	if (!encumb_percent)
		encumb_percent = 1;

	encumb_percent = (int) (encumb_percent * 4.5);

	if (!encumbered && !exhausted)
		return 0;

	if (skill_use (ch, SKILL_SWIMMING, (int) encumb_percent))
	{
		if (ch->room->sector_type != SECT_UNDERWATER && encumbered)
			send_to_char
			("You manage to tread water, staying afloat despite your encumberance. . .\n",
			ch);
		return 0;
	}

	exit = EXIT (ch, DOWN);
	if (exit)
	{
		troom = vtor (exit->to_room);
		if (troom && troom->sector_type != SECT_UNDERWATER)
			troom = NULL;
	}

	if (!encumbered && !exhausted)
	{				/* Gradual fatigue sap for swimmers failing their initial skill check. */
		damage = number (3, 15);
		damage /= 100;
		damage = (int) (ch->max_move * damage);
		if (ch->move < (int) damage)
		{
			damage -= ch->move;
			ch->move = 0;
			if (damage > 0.0 && general_damage (ch, (int) damage))
				return 1;
		}
		else
			ch->move -= (int) damage;
	}
	else if (ch->room->sector_type != SECT_UNDERWATER)
	{
		if (number (1, 100) > encumb_percent && (!troom || !exhausted))
		{
			send_to_char
				("You splutter and choke, sinking briefly beneath the surface. . .\n",
				ch);
			act
				("Spluttering and choking, $n sinks briefly beneath the water's surface.",
				false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);

			/* Saved check for encumbered, non-exhausted swimmer (or an exhausted swimmer who cannot */
			/* sink any further) inflicts more fatigue damage, eventually inflicting bloodloss damage */
			/* when they're totally exhausted and have no more fatigue to sap. */

			damage = number (3, 25);
			damage /= 100;
			damage = (int) (ch->max_move * damage);
			if (ch->move < (int) damage)
			{
				damage -= ch->move;
				ch->move = 0;
				if (damage > 0.0 && general_damage (ch, (int) damage))
					return 1;
			}
			else
				ch->move -= (int) damage;
		}
		else if (troom)
		{

			/* Failed check for encumbered swimmer, or exhaustion, sends them further down. */

			if (encumbered)
			{
				send_to_char
					("Struggling against the weight of your equipment, you sink below the surface. . .\n",
					ch);
				act
					("Struggling, $n sinks from sight beneath the water's surface.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else if (exhausted)
			{
				send_to_char
					("Utterly exhausted, you sink below the surface. . .\n", ch);
				act
					("Looking utterly exhausted, $n sinks from sight beneath the water's surface.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->nVirtual);
			do_look (ch, "", 0);
		}
	}
	else
	{
		if (number (1, 100) > encumb_percent && (!troom || !exhausted))
		{
			;
			/* Handle underwater asphyxiation in a different function; */
			/* only check for a failed swim skillcheck to see if they */
			/* sink any further due to encumberance. */
		}
		else if (troom)
		{
			if (encumbered)
			{
				send_to_char
					("You continue to sink helplessly beneath the weight of your gear. . .\n",
					ch);
				act
					("Struggling helplessly, $n continues to sink further downward.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else if (exhausted)
			{
				send_to_char
					("Too exhausted to fight it, you sink even further into the water. . .\n",
					ch);
				act
					("Struggling weakly, looking exhausted, $n sinks further downward.",
					false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->nVirtual);
			do_look (ch, "", 0);
		}
	}

	return 0;
}

void
do_outside (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, OUTSIDE);
}

void
do_inside (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, INSIDE);
}

void
track_from_room (ROOM_DATA * room, TRACK_DATA * track)
{
	TRACK_DATA *temptrack;

	if (!room || !track)
		return;

	if (room->tracks == track)
	{
		room->tracks = room->tracks->next;
	}
	else
	{
		for (temptrack = room->tracks; temptrack; temptrack = temptrack->next)
		{
			if (temptrack->next == track)
			{
				temptrack->next = temptrack->next->next;
			}
		}
	}

	free_mem (track); // TRACK_DATA*
}

void
track_expiration (ROOM_DATA * room, TRACK_DATA * track)
{
	int limit;

	if (room->sector_type == SECT_INSIDE || room->sector_type == SECT_CITY ||
		room->sector_type == SECT_ROAD)
		limit = 4;
	else if (room->sector_type == SECT_TRAIL)
		limit = 8;
	else if (room->sector_type == SECT_MOUNTAIN
		|| room->sector_type == SECT_SWAMP)
		limit = 12;
	else
		limit = 16;

	if (track->hours_passed > limit)
	{
		track_from_room (room, track);
	}
}

void
update_room_tracks (void)
{
	ROOM_DATA *room;
	TRACK_DATA *track, *next_track;

	for (room = full_room_list; room; room = room->lnext)
	{
		for (track = room->tracks; track; track = next_track)
		{
			next_track = track->next;
			track->hours_passed++;
			if (weather_info[room->zone].state == LIGHT_RAIN)
				track->hours_passed++;
			else if (weather_info[room->zone].state == STEADY_RAIN)
				track->hours_passed += 2;
			else if (weather_info[room->zone].state == HEAVY_RAIN)
				track->hours_passed += 3;
			else if (weather_info[room->zone].state == LIGHT_SNOW)
				track->hours_passed += 2;
			else if (weather_info[room->zone].state == STEADY_SNOW)
				track->hours_passed += 4;
			else if (weather_info[room->zone].state == HEAVY_SNOW)
				track->hours_passed += 6;
			track_expiration (room, track);
		}
	}
}

#define TRACK_LIMIT_PER_ROOM		15

void
leave_tracks (CHAR_DATA * ch, int to_dir, int from_dir)
{
	TRACK_DATA *track;
	WOUND_DATA *wound;
	int bleeding = 0, i = 0;

	if (!ch || !ch->room)
		return;

	if (!IS_MORTAL (ch))
		return;

	if (IS_SET (ch->act, ACT_FLYING))
		return;

	if (ch->room->sector_type == SECT_ROAD
		|| ch->room->sector_type == SECT_CITY)
		return;

	if (ch->race == 33
		|| ch->race == 35
		|| ch->race == 42
		|| ch->race == 66
		|| ch->race == 68
		|| ch->race == 74 || ch->race == 75 || ch->race == 81 || ch->race == 85)
	{
		/* Flying Creatures */
		return;
	}

	for (wound = ch->wounds; wound; wound = wound->next)
	{
		bleeding += wound->bleeding;
	}

	if (bleeding <= 2
		&& (ch->race == 31
		|| ch->race == 41
		|| ch->race == 43
		|| ch->race == 44
		|| ch->race == 46
		|| ch->race == 48
		|| ch->race == 49
		|| ch->race == 50
		|| ch->race == 52
		|| ch->race == 53 || ch->race == 54 || ch->race == 60))
	{
		/* Critters too small to track, unless bleeding. */
		return;
	}


	track = new TRACK_DATA;
	track->next = NULL;
	track->race = ch->race;
	track->to_dir = to_dir;
	track->from_dir = from_dir;
	track->speed = ch->speed;
	track->flags = 0;

	if (bleeding > 2)
		track->flags |= BLOODY_TRACK;
	if (!IS_NPC (ch))
		track->flags |= PC_TRACK;

	if (ch->room->tracks)
		track->next = ch->room->tracks;

	ch->room->tracks = track;

	for (track = ch->room->tracks; track; track = track->next)
	{
		i++;
		if (i > TRACK_LIMIT_PER_ROOM)
		{
			while (track->next)
				track_from_room (ch->room, track->next);
			break;
		}
	}
}

void
clear_pmote (CHAR_DATA * ch)
{

	if (ch->pmote_str)
	{
		free_mem (ch->pmote_str); // char*
		ch->pmote_str = (char *) NULL;
	}
}

int
check_climb (CHAR_DATA * ch)
{
	if (get_affect (ch, MAGIC_AFFECT_LEVITATE))
		return 1;

	if (IS_SET (ch->act, ACT_FLYING))
		return 1;

	if (!IS_MORTAL (ch))
		return 1;

	/* don't fall through solid ground or through doorways */
	if (!CAN_GO (ch, DOWN) ||
		IS_SET (EXIT (ch, DOWN)->exit_info, EX_ISDOOR) ||
		IS_SET (EXIT (ch, DOWN)->exit_info, EX_ISGATE))
		return 1;

	if (IS_SET (ch->room->room_flags, FALL))
	{
		send_to_char ("You plummet down!\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	if (!IS_SET (ch->room->room_flags, CLIMB))
		return 1;


	if (IS_ENCUMBERED (ch))
	{
		send_to_char ("\nYou are too encumbered to maintain your "
			"balance!  YOU FALL!\n\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	if (!skill_use (ch, SKILL_CLIMB, 0))
	{
		send_to_char ("\nYou lose your footing and FALL!\n\n", ch);
		act ("$n FALLS!", false, ch, 0, 0, TO_ROOM);
		return 0;
	}

	return 1;
}

void
do_swim (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Swim in which direction?\n", ch);
		return;
	}

	if (IS_RIDER (ch))
	{
		send_to_char ("Dismount first.\n", ch);
		return;
	}
	else if (IS_RIDEE (ch))
	{
		send_to_char ("Your rider must dismount first.\n", ch);
		return;
	}

	do_move (ch, "swim", dir);
}

int
clear_current_move (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	QE_DATA *qe;
	QE_DATA *tqe;

	/*
	printf ("Clear current move called for %s\n", ch->name);
	*/

	if (GET_FLAG (ch, FLAG_ENTERING))
		return 0;

	for (qe = quarter_event_list; qe; qe = qe->next)
		if (qe->ch == ch)
			break;

	if (!qe)
		return 0;

	if (qe == quarter_event_list)
		quarter_event_list = qe->next;
	else
	{

		for (tqe = quarter_event_list; tqe->next != qe; tqe = tqe->next)
			;

		tqe->next = qe->next;
	}

	ch->flags &= ~FLAG_LEAVING;
	if (qe->travel_str)
		free_mem (qe->travel_str);
	free_mem (qe); // QE_DATA*

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch || tch->deleted)
			continue;

		if (tch->following == ch && CAN_SEE (tch, ch))
		{
			/*
			printf ("Clearing all moves for %s\n", tch->name);
			*/
			clear_moves (tch);
			clear_current_move (tch);
		}
	}

	return 1;
}

void
stop_mount_train (CHAR_DATA * stopper)
{
	CHAR_DATA *ch;
	CHAR_DATA *leader;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;

	ch = stopper;
	leader = stopper;

	while (IS_HITCHEE (ch))
	{
		ch = ch->hitcher;
		leader = ch;
	}

	if (!ch)
	{
		printf ("Hey, ch is NULL!\n");
		return;
	}

	while (ch)
	{

		while (ch->moves)
		{
			move = ch->moves;
			ch->moves = move->next;
			if (move->travel_str)
				free_mem (move->travel_str);
			free_mem (move); //MOVE_DATA*
		}

		if ((af = get_affect (ch, MAGIC_DRAGGER)))
			affect_remove (ch, af);

		if (IS_HITCHER (ch))
			ch = ch->hitchee;
		else
			ch = NULL;
	}

	if (stopper == leader)
		send_to_char ("You prevent your hitches from moving.\n", leader);
	else
		act ("You prevent the hitches led by $N from moving.",
		true, stopper, 0, leader, TO_CHAR);

	act ("$N prevents your hitches from moving.",
		true, leader, 0, stopper, TO_CHAR);
	act ("The hitches led by $n are stopped by $N.",
		false, leader, 0, stopper, TO_NOTVICT);
}

void
clear_moves (CHAR_DATA * ch)
{
	MOVE_DATA *move = NULL;
	AFFECTED_TYPE *af;

	while (ch->moves)
	{
		move = ch->moves;
		ch->moves = move->next;
		if (move->travel_str)
			free_mem (move->travel_str);
		free_mem (move); // MOVE_DATA*
	}

	if (move && (IS_HITCHER (ch) || IS_HITCHEE (ch)))
	{
		stop_mount_train (ch);
		return;
	}

	ch->flags &= ~(FLAG_ENTERING | FLAG_LEAVING);

	if ((af = get_affect (ch, MAGIC_DRAGGER)))
		affect_remove (ch, af);
}

void
add_to_qe_list (QE_DATA * qe)
{
	QE_DATA *tqe;

	if (!quarter_event_list || quarter_event_list->event_time > qe->event_time)
	{
		qe->next = quarter_event_list;
		quarter_event_list = qe;
		return;
	}

	for (tqe = quarter_event_list; tqe->next; tqe = tqe->next)
		if (tqe->next->event_time > qe->event_time)
			break;

	qe->next = tqe->next;
	tqe->next = qe;
}

int
is_moving_toward (CHAR_DATA * target, int dir, CHAR_DATA * archer)
{
	int line;

	if (!target || !archer)
		return 0;

	line = track (target, archer->in_room);

	if (line == dir)
		return 1;

	return 0;
}

int
is_moving_away (CHAR_DATA * target, int dir, CHAR_DATA * archer)
{
	ROOM_DATA *troom;
	int line = 0, i = 0, range = 0;

	if (!target || !archer)
		return 0;

	troom = vtor (target->room->dir_option[dir]->to_room);

	line = track (archer, troom->nVirtual);

	if (line == dir)
	{
		range = is_archer (archer);
		if (!range)
			return 0;
		troom = archer->room;
		if (!troom->dir_option[dir])
			return 0;
		troom = vtor (troom->dir_option[dir]->to_room);
		if (troom->nVirtual == target->room->dir_option[dir]->to_room)
			return 1;
		for (i = 1; i <= range; i++)
		{
			if (troom->nVirtual == target->room->dir_option[dir]->to_room)
				return 1;
			if (!troom->dir_option[dir])
				break;
			troom = vtor (troom->dir_option[dir]->to_room);
		}
	}

	return 0;
}

void
enter_room (QE_DATA * qe)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	float encumb_percent = 0.0;
	int from_dir = 0;
	int rooms_fallen = 0;
	int temp;
	int roomnum;
	int echo = 0;
	int died = 0, i = 0, sensed = 0;
	int dir;
	int observed = 0;
	ROOM_DATA *next_room;
	ROOM_DATA *prevroom;
	LODGED_OBJECT_INFO *lodged;
	ROOM_DIRECTION_DATA *room_exit;
	ROOM_DIRECTION_DATA *exit;
	CHAR_DATA *ch;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *tmp_ch;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *drag_af;
	char buf[MAX_STRING_LENGTH];
	char *formbuf;

	char travel_str[MAX_STRING_LENGTH];
	const char *direction[] =
	{ "south", "west", "north", "east", "down", "up" };
	const char *direction2[] =
	{ "north", "east", "south", "west", "up", "down" };
	const char *rev_d[] = {
		"the south", "the west", "the north", "the east", "below", "above",
		"inside", "outside"
	};
	const char *dir_names[] = {
		"to the north", "to the east", "to the south",
		"to the west", "above", "below", "outside", "inside"
	};
	bool toward = false, away = false;

	ch = qe->ch;

	if (!ch)
	{
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*
		return;
	}

	if (!is_he_there (ch, qe->from_room))
	{

		if (is_he_somewhere (ch))
			clear_moves (ch);
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*

		return;
	}

	if (!is_toll_paid (ch, qe->dir))
	{
		sprintf (buf, "You must pay a toll before crossing %s.", dirs[qe->dir]);
		act (buf, false, ch, 0, 0, TO_CHAR);
		act ("$n is stopped by $N.",
			true, ch, 0, toll_collector (ch->room, qe->dir), TO_NOTVICT);
		act ("You stop $N for toll.",
			false, toll_collector (ch->room, qe->dir), 0, ch, TO_CHAR);
		clear_moves (ch);
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*
		return;
	}

	if (IS_SWIMMING (ch) && IS_SET (ch->flags, FLAG_LEAVING)
		&& ch->room->sector_type == SECT_UNDERWATER && IS_ENCUMBERED (ch)
		&& IS_MORTAL (ch))
	{
		encumb_percent =
			1.0 - (float) (CAN_CARRY_W (ch) -
			IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);
		encumb_percent *= 100;
		encumb_percent = (int) (encumb_percent * 4.5);

		if (IS_DROWNING (ch))
			encumb_percent = (int) (encumb_percent * 2.5);

		if (!skill_use (ch, SKILL_SWIMMING, (int) encumb_percent))
		{
			if (IS_ENCUMBERED (ch))
				send_to_char
				("The sheer weight of your equipment prevents you from making progress.\n",
				ch);
			else
				send_to_char
				("Your leaden limbs refuse to propel your exhausted body in that direction!\n",
				ch);
			clear_moves (ch);
			if (qe->travel_str)
				free_mem (qe->travel_str);
			free_mem (qe); // QE_DATA*
			return;
		}
	}

	remove_affect_type (ch, MAGIC_TOLL_PAID);

	if ((af = get_affect (ch, AFFECT_SHADOW)))
		af->a.shadow.edge = -1;

	travel_str[0] = '\0';
	if (qe && qe->travel_str)
	{
		strcpy (travel_str, qe->travel_str);
	}
	else if (ch->travel_str != NULL)
	{
		sprintf (travel_str, ", %s", ch->travel_str);
	}

	if (IS_SET (qe->flags, MF_ARRIVAL))
	{

		ch->flags &= ~FLAG_ENTERING;

		if (IS_SET (ch->flags, FLAG_LEAVING))
			printf ("Leaving still set!!\n");

		if ((af = get_affect (ch, MAGIC_DRAGGER)) &&
			is_he_here (ch, (CHAR_DATA *) af->a.spell.t, 0) && number (0, 1))
			do_wake (ch, GET_NAME ((CHAR_DATA *) af->a.spell.t), 1);

		if (af)
			affect_remove (ch, af);

		remove_affect_type (ch, MAGIC_SNEAK);

		if (ch->moves)
			initiate_move (ch);

		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*
		qe = NULL;

		return;
	}

	if (IS_SET (qe->flags, MF_TOEDGE))
	{

		if (!(af = get_affect (ch, AFFECT_SHADOW)))
		{
			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
		}

		af->a.shadow.edge = qe->dir;

		ch->flags &= ~FLAG_LEAVING;

		if (!af->a.shadow.shadow)
			;

		else if (!is_he_somewhere (af->a.shadow.shadow))
			send_to_char ("You can no longer see who you are shadowing.\n", ch);

		else if (!could_see (ch, af->a.shadow.shadow))
		{
			sprintf (buf, "You lose sight of #5%s#0.",
				char_short (af->a.shadow.shadow));
			send_to_char (buf, ch);
		}

		if (ch->moves)
		{
			if (qe->travel_str)
				free_mem (qe->travel_str);
			free_mem (qe); // QE_DATA*
			initiate_move (ch);
		}
		else
		{
			sprintf (buf, "$n stops just %s.", dir_names[qe->dir]);
			act (buf, false, ch, 0, 0, TO_ROOM);
			if (qe->travel_str)
				free_mem (qe->travel_str);
			free_mem (qe); // QE_DATA*
		}

		return;
	}

	room_exit = EXIT (ch, qe->dir);

	if (!room_exit)
	{

		sprintf (buf, "There doesn't appear to be an exit %s.\n",
			dirs[qe->dir]);
		send_to_char (buf, ch);

		clear_moves (ch);
		ch->flags &= ~FLAG_LEAVING;
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*

		return;
	}

	if (!IS_SET (qe->flags, MF_PASSDOOR) &&
		IS_SET (room_exit->exit_info, EX_CLOSED))
	{

		if (room_exit->keyword && strlen (room_exit->keyword))
		{
			sprintf (buf, "The %s seems to be closed.\n",
				fname (room_exit->keyword));
			send_to_char (buf, ch);
		}

		else
			send_to_char ("It seems to be closed.\n", ch);

		sprintf (buf, "$n stops at the closed %s.", fname (room_exit->keyword));
		act (buf, true, ch, 0, 0, TO_ROOM);

		clear_moves (ch);
		ch->flags &= ~FLAG_LEAVING;
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*

		return;
	}

	ch->flags &= ~FLAG_LEAVING;

	if (ch->targeted_by)
	{
		if (!(toward = is_moving_toward (ch, qe->dir, ch->targeted_by))
			&& !(away = is_moving_away (ch, qe->dir, ch->targeted_by)))
		{
			sprintf (buf,
				"You lose sight of your quarry as they move %sward.\n",
				direction2[qe->dir]);
			send_to_char (buf, ch->targeted_by);
			ch->targeted_by->aiming_at = NULL;
			ch->targeted_by->aim = 0;
			ch->targeted_by = NULL;
			if (ch->right_hand
				&& ch->right_hand->o.weapon.use_skill == SKILL_CROSSBOW)
			{
				ch->right_hand->location = WEAR_PRIM;
			}
			if (ch->left_hand
				&& ch->left_hand->o.weapon.use_skill == SKILL_CROSSBOW)
			{
				ch->left_hand->location = WEAR_PRIM;
			}

		}
		else if (toward)
		{

			sprintf (buf,
				"You carefully hold your aim as your quarry moves closer...\n");
			send_to_char (buf, ch->targeted_by);

		}
		else if (away)
		{

			if (ch->targeted_by->delay_who
				&& strlen (ch->targeted_by->delay_who) > 1)
				free_mem (ch->targeted_by->delay_who);
			ch->targeted_by->delay_who = duplicateString (dirs[qe->dir]);

			sprintf (buf,
				"You carefully hold your aim as your quarry moves away...\n");
			send_to_char (buf, ch->targeted_by);

		}
		else
		{
			sprintf (buf, "You lose sight of your quarry.\n");
			send_to_char (buf, ch->targeted_by);
			ch->targeted_by->aiming_at = NULL;
			ch->targeted_by->aim = 0;
			ch->targeted_by = NULL;
			if (ch->right_hand
				&& ch->right_hand->o.weapon.use_skill == SKILL_CROSSBOW)
			{
				ch->right_hand->location = WEAR_PRIM;
			}
			if (ch->left_hand
				&& ch->left_hand->o.weapon.use_skill == SKILL_CROSSBOW)
			{
				ch->left_hand->location = WEAR_PRIM;
			}
		}
	}

	if (ch->lodged && GET_ITEM_TYPE (vtoo (ch->lodged->vnum)) == ITEM_WEAPON
		&& !number (0, 2))
	{
		lodged = ch->lodged;
		obj = load_object (lodged->vnum);
		if (obj)
		{
			sprintf (buf,
				"#2%s#0 becomes dislodged with your movement, and falls to the ground.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			sprintf (buf,
				"#2%s#0 becomes dislodged as #5%s#0 moves, and falls to the ground.",
				obj_short_desc (obj), char_short (ch));
			buf[2] = toupper (buf[2]);
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			obj_to_room (obj, ch->room->nVirtual);
			lodge_from_char (ch, lodged);
		}
	}
	for (lodged = ch->lodged; lodged; lodged = lodged->next)
	{
		if (!lodged->next)
			break;
		if (GET_ITEM_TYPE (vtoo (lodged->next->vnum)) != ITEM_WEAPON)
			continue;
		if (!number (0, 2))
			continue;
		obj = load_object (lodged->next->vnum);
		if (!obj)
			continue;
		sprintf (buf,
			"#2%s#0 becomes dislodged with your movement, and falls to the ground.",
			obj_short_desc (obj));
		buf[2] = toupper (buf[2]);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf,
			"#2%s#0 becomes dislodged as #5%s#0 moves, and falls to the ground.",
			obj_short_desc (obj), char_short (ch));
		buf[2] = toupper (buf[2]);
		act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		obj_to_room (obj, ch->room->nVirtual);
		lodge_from_char (ch, lodged->next);
	}

	if ((drag_af = get_affect (ch, MAGIC_DRAGGER)) &&
		is_he_here (ch, (CHAR_DATA *) drag_af->a.spell.t, 1) &&
		GET_POS ((CHAR_DATA *) drag_af->a.spell.t) <= SLEEP)
	{
		char_from_room ((CHAR_DATA *) drag_af->a.spell.t);
		char_to_room ((CHAR_DATA *) drag_af->a.spell.t, room_exit->to_room);
	}
	else
		drag_af = NULL;

	if (IS_SUBDUER (ch))
	{
		char_from_room (ch->subdue);
		char_to_room (ch->subdue, room_exit->to_room);
	}

	if (IS_MOUNT (ch) && ch->mount && ch->mount->subdue)
	{
		char_from_room (ch->mount->subdue);
		char_to_room (ch->mount->subdue, room_exit->to_room);
		do_look (ch->mount->subdue, "", 0);
	}

	temp = GET_POS (ch);
	GET_POS (ch) = SLEEP;		/* Hack to make him not hear messages */

	shadowers_shadow (ch, room_exit->to_room, qe->dir);

	GET_POS (ch) = temp;

	if (is_mounted (ch))
	{
		char_from_room (ch->mount);
		char_to_room (ch->mount, room_exit->to_room);
	}

	roomnum = ch->in_room;
	prevroom = vtor (roomnum);

	//check for newly open wounds first,
	//so there is a chance to leave bloody tracks
	// Sneakers who make separate successful SNEAK checks leave no tracks.
	int start_bleed = 0;
	int tot_damage = 0;
	int sneakBleeding = 0;
	int sneakRoll = number(1, 100);
	for (WOUND_DATA *wound = ch->wounds; wound; wound = wound->next)
	{
			//bound wounds will re-open when moving faster than trudge
		if (!(ch->speed == 1) && (wound->bindskill > 1))
		{
			//chance for wound to re-open
			start_bleed = number(10, 100); 						
			if (start_bleed > wound->bindskill) 
			{
				wound->bleeding = number (1, 3);	
			}
			if (IS_MORTAL (ch) && wound->bleeding)
			{
				sprintf (buf,
				 "#1You wince as blood begins to flow again from a %s %s on your %s.#0\n",
				wound->severity, wound->name,
				expand_wound_loc (wound->location));
				send_to_char (buf, ch);
			}
		}
					
		sneakBleeding += wound->bleeding;
		tot_damage += wound->damage;
	}
	
	for (WOUND_DATA *wound = ch->wounds; wound; wound = wound->next)
	{
			//stuns do not get worse
		if (!strn_cmp(wound->type, "stun", 4))
		{
			continue;
		}
			//each wound will get worse, if moving faster than a walk
			//but only once you are already badly wounded
		else if (!str_cmp (wound->severity, "small")
			|| !str_cmp (wound->severity, "minor"))
		{
			continue;
		}

		else
		{
				//They take damage at current speed, then changed speed automatically to avoid future damage.
				/** trudge, pace, walk, jog -->> 1, 2, 0, 3 **/
			
//2 stars or less, and you -will- take damage moving faster than trudge (1)
			if (((ch->speed > 1) || (ch->speed == 0)) && (tot_damage > (.6667 * ch->max_hit)))
			{
			sprintf (buf,
						 "#1The pain from a %s %s on your %s gets much worse, so you slow down.#0\n",
					 wound->severity, wound->name,
					 expand_wound_loc (wound->location));
			send_to_char (buf, ch);
					//Make the wound worse, but not enough to kill the player 1-3 points
				if ((ch->damage + 4) >= ch->max_hit)
					adjust_wound (ch, wound, number(1, 3));
				else if ((ch->damage + 3) >= ch->max_hit)
					adjust_wound (ch, wound, number(1, 2));	
				else if ((ch->damage + 2) >= ch->max_hit)
					adjust_wound (ch, wound, 1);				  
				ch->speed = 1;
			}

//3 stars or less, and you -will- take damage moving faster than pace (2)
			else if (((ch->speed > 2)|| (ch->speed == 0)) && (tot_damage > (.333 * ch->max_hit)))
			{
				sprintf (buf,
						 "#1The pain from a %s %s on your %s gets much worse, so you slow down.#0\n",
						 wound->severity, wound->name,
						 expand_wound_loc (wound->location));
				send_to_char (buf, ch);
				//Make the wound worse, but not enough to kill the player 1-3 points
				if ((ch->damage + 4) >= ch->max_hit)
					adjust_wound (ch, wound, number(1, 3));
				else if ((ch->damage + 3) >= ch->max_hit)
					adjust_wound (ch, wound, number(1, 2));	
				else if ((ch->damage + 2) >= ch->max_hit)
					adjust_wound (ch, wound, 1);	
				ch->speed = 2;
			}
//four stars you -may- take some damage if move faster than walk (0)
			else if ((ch->speed > 2) && (tot_damage > (.1667 * ch->max_hit)))
			{
			sprintf (buf,
						 "#1The pain from a %s %s on your %s gets a little worse, so you slow down.#0\n",
					 wound->severity, wound->name,
					 expand_wound_loc (wound->location));
			send_to_char (buf, ch);
					//Make the wound worse, but not enough to kill the player 0-2 points
				if ((ch->damage + 3) >= ch->max_hit)
					adjust_wound (ch, wound, number(0, 2));	
				else if ((ch->damage + 2) >= ch->max_hit)
					adjust_wound (ch, wound, number(0,1));
				ch->speed = 0;
			}
			
		}
	}

	if (!IS_SET (qe->flags, MF_SNEAK) || (ch->skills[SKILL_SNEAK] < sneakRoll || sneakBleeding > 2))
	{
		leave_tracks (ch, qe->dir, ch->from_dir);
	}

	char_from_room (ch);

	if ((prevroom->sector_type == SECT_LEANTO
		|| prevroom->sector_type == SECT_PIT) && prevroom->deity <= 15)
	{
		while (prevroom->contents)
		{
			obj = prevroom->contents;
			obj_from_room (&obj, 0);
			obj_to_room (obj, room_exit->to_room);
			if (echo == 0)
				send_to_char ("You take your belongings with you as you leave.\n",
				ch);
			echo = 1;
		}
	}

	char_to_room (ch, room_exit->to_room);

	if (qe->dir == 0)
		from_dir = 2;
	else if (qe->dir == 1)
		from_dir = 3;
	else if (qe->dir == 2)
		from_dir = 0;
	else if (qe->dir == 3)
		from_dir = 1;
	else if (qe->dir == 4)
		from_dir = 5;
	else
		from_dir = 4;

	ch->from_dir = from_dir;

	if (!IS_NPC (ch) && !IS_SET (ch->act, ACT_VEHICLE))
		weaken (ch, 0, qe->move_cost, NULL);
	else
		weaken (ch, 0, qe->move_cost / 5, NULL);

	do_look (ch, "", 0);

	/* quick scan when they enter a room */
	if (IS_SET (ch->plr_flags, QUIET_SCAN))
		do_scan (ch, "", 1);

	if (IS_MORTAL (ch) && ch->speed != SPEED_IMMORTAL)
	{
		if ((prevroom->sector_type != SECT_LAKE
			&& prevroom->sector_type != SECT_RIVER
			&& prevroom->sector_type != SECT_OCEAN
			&& prevroom->sector_type != SECT_REEF
			&& prevroom->sector_type != SECT_UNDERWATER)
			&& SWIM_ONLY (ch->room))
		{
			send_to_char ("\n", ch);
			act ("You enter the water, and begin attempting to swim. . .",
				false, ch, 0, 0, TO_CHAR);
			object__drench (ch, ch->equip, true);
		}
		else if (prevroom->sector_type == SECT_UNDERWATER
			&& ch->room->sector_type == SECT_UNDERWATER)
		{
			send_to_char ("\n", ch);
			act ("You continue your swim beneath the water's surface. . .",
				false, ch, 0, 0, TO_CHAR);
		}
		else
			if ((prevroom->sector_type == SECT_LAKE
				|| prevroom->sector_type == SECT_RIVER
				|| prevroom->sector_type == SECT_OCEAN
				|| prevroom->sector_type == SECT_REEF
				|| prevroom->sector_type == SECT_UNDERWATER)
				&& (ch->room->sector_type != SECT_LAKE
				&& ch->room->sector_type != SECT_RIVER
				&& ch->room->sector_type != SECT_OCEAN
				&& ch->room->sector_type != SECT_REEF
				&& ch->room->sector_type != SECT_UNDERWATER))
			{
				send_to_char ("\n", ch);
				act ("You climb from the water, dripping.", false, ch, 0, 0,
					TO_CHAR);
			}
	}

	/*
	if ( !IS_SET (ch->act, ACT_PREY) && !IS_NPC (ch) ) {
	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
	if ( !IS_NPC (tch) || tch->desc )
	continue;
	if ( !IS_SET (tch->act, ACT_PREY) )
	continue;
	if ( get_affect (tch, MAGIC_HIDDEN) )
	continue;
	if ( !CAN_SEE (tch, ch) || (!CAN_SEE (ch, tch) && CAN_SEE (tch, ch)) )
	continue;
	evade_attacker (tch, -1);
	add_threat (tch, ch, 7);
	}
	}
	*/
	while (ch->sighted)
		ch->sighted = ch->sighted->next;	/* Remove list of target sightings. */

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (!(exit = EXIT (ch, dir)) || !(next_room = vtor (exit->to_room)))
			continue;
		sensed = 0;
		for (tch = next_room->people; tch; tch = tch->next_in_room)
		{
			if (real_skill (tch, SKILL_DANGER_SENSE) && !is_brother (ch, tch)
				&& IS_MORTAL (tch) && IS_SET (ch->act, ACT_AGGRESSIVE))
			{
				if (skill_use (tch, SKILL_DANGER_SENSE, 20))
				{
					sprintf (buf,
						"The hairs on the back of your neck prickle as you glance %sward.\n",
						direction[dir]);
					send_to_char (buf, tch);
				}
			}
			if (skill_use (ch, SKILL_DANGER_SENSE, 15)
				&& IS_SET (tch->act, ACT_AGGRESSIVE) && !is_brother (tch, ch)
				&& !sensed)
			{
				sprintf (buf,
					"A sense of foreboding fills you as you glance %sward.\n",
					direction2[dir]);
				send_to_char (buf, ch);
				sensed++;
			}
		}
	}

	if (is_mounted (ch))
	{
		do_look (ch->mount, "", 0);
		if (IS_SET (ch->plr_flags, QUIET_SCAN))
			do_scan (ch, "", 1);
	}

	if (IS_SUBDUER (ch))
	{
		do_look (ch->subdue, "", 0);
		if (IS_SET (ch->plr_flags, QUIET_SCAN))
			do_scan (ch, "", 1);
	}

	qe->flags |= MF_ARRIVAL;
	ch->flags |= FLAG_ENTERING;
	qe->event_time = qe->arrive_time;
	qe->from_room = ch->room;

	if (qe->event_time <= 0)
	{
		sprintf (buf, "$n arrives from %s.", rev_d[qe->dir]);
		act (buf, true, ch, 0, 0, TO_ROOM);
		enter_room (qe);
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (is_area_enforcer (tch) && enforcer (tch, ch, 1, 0) > 0)
			{
				continue;
			}
		}
	}
	else
	{
		if (is_mounted (ch) && IS_RIDEE (ch))
		{

			/* ch->mount is the rider */

			sprintf (buf, "$n %ss $N in from %s.",
				mount_speeds[ch->mount->pc ? ch->mount->pc->mount_speed :
				ch->speed], rev_d[qe->dir]);
			act (buf, false, ch->mount, 0, ch, TO_NOTVICT);
		}

		else
		{
			// if sneaking and the room is nohide, or they fail the skillcheck remove the sneak affect
			if (IS_SET (qe->flags, MF_SNEAK)
				&& (IS_SET (ch->room->room_flags, NOHIDE)
				|| (would_reveal (ch)
				&& odds_sqrt (skill_level (ch, SKILL_SNEAK, 0)) <
				number (1, 100))))
			{

				observed = 0;
				for (tmp_ch = ch->room->people; tmp_ch;
					tmp_ch = tmp_ch->next_in_room)
				{
					if (tmp_ch != ch && CAN_SEE (ch, tmp_ch)
						&& !are_grouped (tmp_ch, ch))
					{
						observed = 1;
						break;
					}
				}
				if (observed)
					send_to_char ("You are observed.\n", ch);
				remove_affect_type (ch, MAGIC_HIDDEN);
				remove_affect_type (ch, MAGIC_SNEAK);
				qe->flags &= ~MF_SNEAK;
			}

			if (!IS_SET (qe->flags, MF_SNEAK))
			{

				if (!IS_SET (ch->act, ACT_FLYING))
					sprintf (buf, "$n is entering from %s%s.", rev_d[qe->dir],
					(drag_af ? ", dragging $N" : travel_str));
				else
					sprintf (buf, "$n flies in from %s%s.", rev_d[qe->dir],
					drag_af ? ", dragging $N" : "");
				act (buf, true, ch, 0, drag_af ?
					(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT);
			}
			else
			{
				sprintf (buf, "[%s sneaking in from %s.]", ch->tname,
					rev_d[qe->dir]);
				act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
				sprintf (buf, "#5%s#0 sneaks into the area.", char_short (ch));
				buf[2] = toupper (buf[2]);
				act (buf, true, ch, 0, 0, TO_NOTVICT | TO_GROUP);
			}
		}

		add_to_qe_list (qe);
	}

	if (IS_SUBDUER (ch))
		act ("$n has $N in tow.", true, ch, 0, ch->subdue, TO_NOTVICT);

	if (!check_climb (ch))
	{

		clear_moves (ch);
		clear_current_move (ch);

		while (IS_SET (ch->room->room_flags, CLIMB) ||
			IS_SET (ch->room->room_flags, FALL))
		{

			room_exit = EXIT (ch, DOWN);

			/* Don't automatically fall through doors */
			if (!room_exit
				|| IS_SET (room_exit->exit_info, EX_ISDOOR)
				|| IS_SET (room_exit->exit_info, EX_ISGATE))
				break;

			rooms_fallen++;

			act ("$n plummets down!", true, ch, 0, 0, TO_ROOM);
			send_to_char ("\n\nYou plummet down!\n\n", ch);

			char_from_room (ch);
			char_to_room (ch, room_exit->to_room);

			do_look (ch, "", 0);
			//no qscan while falling
		}

		if (SWIM_ONLY (ch->room))
		{
			send_to_char ("You land with a splash!\n", ch);
			act ("$n lands with a splash!", true, ch, 0, 0, TO_ROOM);
			object__drench (ch, ch->equip, true);
		}
		else
		{
			send_to_char ("You land with a thud.\n", ch);
			act ("$n lands with a thud!", false, ch, 0, 0, TO_ROOM);
		}

		for (i = 1; i <= number (1, 5) * rooms_fallen; i++)
		{
			died +=
				wound_to_char (ch, figure_location (ch, 1),
				rooms_fallen * number (1, 5), 3, 0, 0, 0);
			if (died)
				break;
		}

		if (!died && !SWIM_ONLY (ch->room))
			knock_out (ch, 15);

		if (drag_af)
		{
			victim = (CHAR_DATA *) drag_af->a.spell.t;

			if (victim && victim->in_room)
			{
				char_from_room (victim);
				char_to_room (victim, room_exit->to_room);
			}

			victim = NULL;
		}
	}

	else if (ch->following)
		follower_catchup (ch);

	if (ch->aiming_at)
	{
		send_to_char ("You lose your aim as you move.\n", ch);
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
		if (ch->right_hand
			&& ch->right_hand->o.weapon.use_skill == SKILL_CROSSBOW)
		{
			ch->right_hand->location = WEAR_PRIM;
		}
		if (ch->left_hand
			&& ch->left_hand->o.weapon.use_skill == SKILL_CROSSBOW)
		{
			ch->left_hand->location = WEAR_PRIM;
		}
	}
}

void
process_quarter_events (void)
{
	QE_DATA *qe;

	for (qe = quarter_event_list; qe; qe = qe->next)
		qe->event_time--;

	while (quarter_event_list && quarter_event_list->event_time <= 0)
	{

		qe = quarter_event_list;

		quarter_event_list = qe->next;

		enter_room (qe);
	}
}

void
exit_room (CHAR_DATA * ch, int dir, int flags, int leave_time,
		   int arrive_time, int speed_name, int needed_movement,
		   char *travel_str)
{
	CHAR_DATA *mount;
	CHAR_DATA *rider;
	QE_DATA *qe;
	char buf[AVG_STRING_LENGTH];
	char *speed_type_names[] = {
		"walking", "wandering", "slowly walking", "jogging",
		"running", "sprinting"
	};
	char buf1[AVG_STRING_LENGTH];
	/*
	char		*mount_noun_speeds [] = {
	"trot", "trudge", "slowly trot", "quickly trot",
	"gallop", "quickly gallop" };
	*/

	ch->flags |= FLAG_LEAVING;

	qe = new QE_DATA;
	qe->next = NULL;
	qe->ch = ch;
	qe->dir = dir;
	qe->speed_type = speed_name;
	qe->flags = flags;
	qe->from_room = ch->room;
	qe->event_time = leave_time;
	qe->arrive_time = arrive_time;
	qe->move_cost = needed_movement;
	qe->travel_str = travel_str;

	if (ch->speed == SPEED_IMMORTAL)
	{
		sprintf (buf1, "$n leaves in a flash %s.", dirs[dir]);
		if (dir != OUTSIDE && dir != INSIDE)
			sprintf (buf, "You blast %sward.\n", dirs[dir]);
		else
			sprintf (buf, "You blast %s.\n", dirs[dir]);
		send_to_char (buf, ch);
		act (buf1, true, ch, 0, 0, TO_ROOM);
	}
	else
	{

		if (IS_SUBDUER (ch))
		{
			sprintf (buf, "$n starts %s %s, dragging $N along.",
				speed_type_names[ch->speed], dirs[dir]);
			act (buf, true, ch, 0, ch->subdue, TO_NOTVICT);
		}

		if (is_mounted (ch) && IS_RIDEE (ch))
		{

			mount = ch;
			rider = ch->mount;

			int speed;

			if (rider->pc)
			{
				speed = rider->pc->mount_speed;
			}
			else
			{
				speed = mount->speed;
			}

			sprintf (buf, "You make $N %s %s.",
				mount_speeds[speed], dirs[dir]);
			act (buf, false, rider, 0, mount, TO_CHAR);

			sprintf (buf, "$n starts %s $N %s.",
				mount_speeds_ing[speed], dirs[dir]);
			act (buf, false, rider, 0, mount, TO_NOTVICT);
			hitches_follow (ch->mount, dir, leave_time, arrive_time);
			followers_follow (ch->mount, dir, leave_time, arrive_time);
		}

		if (IS_SET (flags, MF_SNEAK))
		{
			sprintf (buf, "[%s starts sneaking %s.]", ch->tname, dirs[dir]);
			act (buf, true, ch, 0, 0, TO_NOTVICT | TO_IMMS);
			if (dir != OUTSIDE && dir != INSIDE)
				sprintf (buf, "#5%s#0 begins sneaking %sward.", char_short (ch),
				dirs[dir]);
			else
				sprintf (buf, "#5%s#0 begins sneaking %s.", char_short (ch),
				dirs[dir]);
			buf[2] = toupper (buf[2]);
			act (buf, true, ch, 0, 0, TO_NOTVICT | TO_GROUP);
		}

	}

	if (qe->event_time <= 0)
	{
		enter_room (qe);
		return;
	}

	add_to_qe_list (qe);

	hitches_follow (ch, dir, leave_time, arrive_time);
	followers_follow (ch, dir, leave_time, arrive_time);
}

int
calc_movement_charge (CHAR_DATA * ch, int dir, int wanted_time, int flags,
					  int *speed, int *speed_name, float *walk_time)
{
	char buf[MAX_STRING_LENGTH];
	int needed_movement = 0;
	float cost_modifier = 1.00;
	float encumb_percent = 1.00;
	int true_speed = 0;
	float t;
	AFFECTED_TYPE *dragger;
	ROOM_DATA *target_room;
	ROOM_DIRECTION_DATA *room_exit;

	room_exit = EXIT (ch, dir);

	if (!room_exit || !(target_room = vtor (room_exit->to_room)))
		return 0;

	if (dir == UP &&
		IS_SET (target_room->room_flags, FALL) &&
		!get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
		return 0;

	if (dir == UP &&
		IS_SET (target_room->room_flags, CLIMB) &&
		IS_ENCUMBERED (ch) &&
		!get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
		return 0;

	/* TOEDGE means were in the room and just want to get to the edge. */

	if (IS_SET (flags, MF_TOEDGE))
		needed_movement = movement_loss[ch->room->sector_type] / 2;

	/* TONEXT_EDGE means we're about to walk into the room and cross it
	completely to the other edge. */

	else if (IS_SET (flags, MF_TONEXT_EDGE))
		needed_movement = movement_loss[target_room->sector_type];

	/* Otherwise, we're just walking from midpoint to midpoint */

	else
		needed_movement = (movement_loss[ch->room->sector_type] +
		movement_loss[target_room->sector_type]) / 2;

	if ((dragger = get_affect (ch, MAGIC_DRAGGER)))
		cost_modifier *= 1.50;
	else if (IS_SUBDUER (ch))
		cost_modifier *= 1.50;

	encumb_percent =
		1.0 - (float) (CAN_CARRY_W (ch) -
		IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);

	if (encumb_percent > 0.95)
	{
		printf ("Huh?  %f\n",
			(float) (CAN_CARRY_W (ch) - IS_CARRYING_W (ch)) /
			(float) CAN_CARRY_W (ch));
		printf ("Very encumbered pc %s in room %d\n", ch->name, ch->in_room);
		sprintf (buf, "You are carrying too much to move.\n");
		send_to_char (buf, ch);
		clear_moves (ch);
		return -1;
	}

	/* 4 pulses per second.  5 seconds/room for a 13 agi N/PC */
	//speed of a group is the speed of the slowest person
	if (is_with_group (ch))
	{
	true_speed = speed_group(ch);
	}
	else
	{
		true_speed = ch->speed;
	}

	*walk_time = 0;

	*walk_time = 2 * 13.0 * 5.0 / (GET_AGI (ch) ? GET_AGI (ch) : 13);

	if (!wanted_time)
		wanted_time = (int) round (*walk_time * move_speeds[true_speed]);

	if (*walk_time == 0)
		*walk_time = 9.0;

	if (get_affect (ch, MAGIC_AFFECT_SLOW))
		*walk_time *= 1.5;

	if (get_affect (ch, MAGIC_AFFECT_SPEED))
		*walk_time *= 0.5;

	if ((SWIM_ONLY (ch->room) || SWIM_ONLY (target_room))
		&& !IS_SET (ch->act, ACT_FLYING))
	{
		*walk_time *= (1.45 - ((float) (ch->skills[SKILL_SWIMMING] / 100)));
		cost_modifier *= (1.45 - ((float) (ch->skills[SKILL_SWIMMING] / 100)));
		skill_use (ch, SKILL_SWIMMING, 0);
	}

	t = wanted_time / *walk_time;

	if (t < 2.50 && encumb_percent > 0.80)
		t = 2.50;
	else if (t < 1.60 && encumb_percent > 0.70)
		t = 1.60;
	else if (t < 1.00 && encumb_percent > 0.50)
		t = 1.00;
	else if (t < 0.66 && encumb_percent > 0.40)
		t = 0.66;
	else if (t < 0.50 && encumb_percent > 0.30)
		t = 0.50;
	else if (t < 0.33)
		t = 0.33;

	if (t > 1.60001)
		*speed_name = SPEED_CRAWL;
	else if (t > 1.00001)
		*speed_name = SPEED_PACED;
	else if (t > 0.66001)
		*speed_name = SPEED_WALK;
	else if (t > 0.50001)
		*speed_name = SPEED_JOG;
	else if (t > 0.33001)
		*speed_name = SPEED_RUN;
	else
		*speed_name = SPEED_SPRINT;

	if (IS_SWIMMING (ch) && !IS_SET (ch->act, ACT_FLYING))
	{
		t = 1.60;
		*speed_name = SPEED_SWIM;
	}

	if (encumb_percent > .30 && encumb_percent <= .40)
		encumb_percent *= 1.25;
	else if (encumb_percent > .40 && encumb_percent <= .50)
		encumb_percent *= 2.0;
	else if (encumb_percent > .50 && encumb_percent <= .60)
		encumb_percent *= 3.75;
	else if (encumb_percent > .60 && encumb_percent <= .75)
		encumb_percent *= 6.0;
	else if (encumb_percent > .75)
		encumb_percent *= 11.25;


	cost_modifier = cost_modifier / t;
	cost_modifier = (int) cost_modifier *(1.0 + encumb_percent);
	*speed = (int) round (*walk_time * t);

	if (GET_TRUST (ch) && ch->speed == SPEED_IMMORTAL)
	{
		*speed_name = SPEED_IMMORTAL;
		needed_movement = 0;
		*speed = 0;
	}

	if (IS_RIDER (ch))
		needed_movement = 0;

	if (IS_SET (ch->act, ACT_MOUNT))
		needed_movement = (int) round (((float) needed_movement) * 0.33);

	if (*speed < 0)
		*speed = 1;

	return ((int) round (((float) needed_movement) * cost_modifier));
}

void
initiate_move (CHAR_DATA * ch)
{
	int dir;
	int flags;
	int needed_movement;
	int exit_speed;		/* Actually, time */
	int enter_speed;		/* Actually, time */
	float walk_time;
	int speed;
	int wanted_time;
	int speed_name;
	bool can_move;
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	ROOM_DATA *target_room;
	ROOM_DIRECTION_DATA *room_exit;
	char buf[MAX_STRING_LENGTH], suffix[25];
	char *move_names[] = {
		"walking", "wandering", "slowly walking",
		"jogging", "running", "sprinting", "blasting",
		"swimming"
	};
	char *move_names2[] = {
		"walk", "wander", "pace",
		"jog", "run", "sprint", "blast",
		"swim"
	};
	char buf1[MAX_STRING_LENGTH];
	char travel_str[MAX_STRING_LENGTH] = "";


	if (!ch->moves)
	{
		printf ("Nothing to initiate!\n");
		return;
	}

	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		move_names[ch->speed] = "traveling";
		move_names2[ch->speed] = "travel";
	}

	if (ch->race == 33 || ch->race == 35 || ch->race == 66 ||
		ch->race == 68 || ch->race == 71 || ch->race == 74 ||
		ch->race == 75 || ch->race == 78 || ch->race == 81)
	{
		move_names[ch->speed] = "flying";
		move_names2[ch->speed] = "flie";	/* ("flies") */
	}
	else if (ch->race == 56 || ch->race == 58 || ch->race == 65)
	{
		move_names[ch->speed] = "slithering";
		move_names2[ch->speed] = "slither";
	}
	else if (ch->race == 42 || ch->race == 70 || ch->race == 79)
	{
		move_names[ch->speed] = "swimming";
		move_names2[ch->speed] = "swim";
	}

	if (IS_HITCHEE (ch))
	{

		if (!GET_FLAG (ch->hitcher, FLAG_LEAVING))
		{

			if (IS_RIDEE (ch))
				act ("Your mount is hitched to $N.",
				true, ch->mount, 0, ch->hitcher, TO_CHAR);

			act ("You can't move while hitched to $N.",
				true, ch, 0, ch->hitcher, TO_CHAR);

			clear_moves (ch);
			return;
		}
	}

	if (ch->fighting)
	{

		dir = ch->moves->dir;

		/* A "PURSUING" mob will follow from a fight after one second */

		/* Non-sentinel aggro/enforcer mobs now follow by default, if morale check is made. */

		if ((IS_SET (ch->act, ACT_PURSUE)
			|| (!morale_broken (ch)
			&& (IS_SET (ch->act, ACT_AGGRESSIVE)
			|| (IS_SET (ch->act, ACT_ENFORCER)
			&& !IS_SET (ch->act, ACT_SENTINEL))))) 
			&& ch->following
			&& is_he_here (ch, ch->following, true))
		{

			stop_fighting (ch);

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
				if (tch->fighting == ch && tch != ch && GET_HIT (tch) > 0)
				{
					set_fighting (ch, tch);
					return;
				}

				add_second_affect (SA_MOVE, 1, ch, NULL, NULL, dir);
		}

		return;
	}

	if (GET_POS (ch) != STAND)
	{
		send_to_char ("You'll need to stand in order to move.\n", ch);
		clear_moves (ch);
		return;
	}

	move = ch->moves;
	ch->moves = move->next;

	dir = move->dir;
	flags = move->flags;
	wanted_time = move->desired_time;
	if (move->travel_str)
	{
		sprintf (travel_str, ", %s", move->travel_str);
		free_mem (move->travel_str);
	}

	free_mem (move); // MOVE_DATA*

	room_exit = EXIT (ch, dir);

	if (!room_exit || !(target_room = vtor (room_exit->to_room)) ||
		(IS_SET (room_exit->exit_info, EX_CLOSED) && ch->room->secrets[dir]
	&& IS_MORTAL (ch)))
	{

		if (ch->room->extra && ch->room->extra->alas[dir])
			send_to_char (ch->room->extra->alas[dir], ch);
		else
			send_to_char ("Alas, you cannot go that way...\n", ch);

		clear_moves (ch);
		return;
	}

	if (dir == UP
		&& !get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& !IS_SET (ch->act, ACT_FLYING))
	{

		if (IS_SET (target_room->room_flags, FALL))
		{

			send_to_char ("Too steep.  You can't climb up.\n", ch);
			clear_moves (ch);
			return;
		}

		if (IS_SET (target_room->room_flags, CLIMB))
		{

			if (IS_ENCUMBERED (ch))
			{
				send_to_char ("You're too encumbered to climb up.\n", ch);
				clear_moves (ch);
				return;
			}
			else if ((ch->left_hand && ch->right_hand)
				|| get_equip (ch, WEAR_BOTH))
			{
				send_to_char ("You'll need to free your hands to climb up.\n",
					ch);
				clear_moves (ch);
				return;
			}
		}
	}
	//if (!(target_room->capacity == 0))
	//{
	//	if (!room_avail(target_room, NULL, ch)
	//		&& !(GET_TRUST(ch))
	//		&& !force_enter(ch, target_room))
	//	{
	//		send_to_char("There isn't enough room for you.", ch);
	//		clear_moves (ch);
	//		return;
	//	}
	//}
	// Case

	if (IS_SET (ch->act, ACT_MOUNT) &&
		IS_SET (target_room->room_flags, NO_MOUNT))
	{
		act ("You can't go there.", true, ch, 0, 0, TO_CHAR);

		if (IS_RIDEE (ch))
			act ("$N can't go there.", true, ch->mount, 0, ch, TO_CHAR);

		if (IS_HITCHEE (ch))
			act ("$N can't go there.", true, ch->hitcher, 0, ch, TO_CHAR);

		clear_moves (ch);
		return;
	}

	if (any_are_set(ch->act, ACT_VEHICLE)
		&& (target_room->sector_type >= SECT_PIT
		|| target_room->sector_type == SECT_SWAMP
		|| target_room->sector_type == SECT_MOUNTAIN
		|| target_room->sector_type == SECT_FOREST))
	{
		can_move = false;

		for (OBJ_DATA *tobj = target_room->contents; tobj; tobj = tobj->next_content)
		{
			if (tobj->nVirtual == 91686 || tobj->nVirtual == 91687)
			{
				can_move = true;
				break;
			}
		}

		if (!can_move)
		{
			act ("You can't go there.", true, ch, 0, 0, TO_CHAR);

			if (IS_RIDEE(ch))
				act ("$N can't go there.", true, ch->mount, 0, ch, TO_CHAR);

			if (IS_HITCHEE (ch))
			{
				act ("$N can't go there.", true, ch->hitcher, 0, ch, TO_CHAR);
				clear_moves (ch->hitcher);
			}

			clear_moves (ch);
			return;
		}
	}
		//mounts cannot travel faster than a trot or walk in certain terrain, without a trails
	if ((target_room->sector_type == SECT_SWAMP
		|| target_room->sector_type == SECT_MOUNTAIN
		|| target_room->sector_type == SECT_FOREST) && IS_SET (ch->act, ACT_MOUNT))
	{
		can_move = false;
		
		for (OBJ_DATA *tobj = target_room->contents; tobj; tobj = tobj->next_content)
		{
			if (tobj->nVirtual == 91686 || tobj->nVirtual == 91687)
			{
				can_move = true;
				break;
			}
		}
		
		int mnt_spd;
		if ((ch->mount) && (ch->mount->pc))
			mnt_spd = ch->mount->pc->mount_speed;
		else 
			mnt_spd = ch->speed;
		
		if ((!can_move) && (mnt_spd > 0))
		{
			act ("You can't go there at your current speed.", true, ch, 0, 0, TO_CHAR);
			
			if (IS_RIDEE(ch))
				act ("$N can't go there at their current speed.", true, ch->mount, 0, ch, TO_CHAR);
			
			if (IS_HITCHEE (ch))
			{
				act ("$N can't go there at their current speed.", true, ch->hitcher, 0, ch, TO_CHAR);
				clear_moves (ch->hitcher);
			}
			
			clear_moves (ch);
			return;
		}
	}
	
	if (isguarded (ch->room, dir) && (IS_MORTAL (ch) || IS_NPC (ch)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (!CAN_SEE (tch, ch))
				continue;
			if (IS_SET (ch->act, ACT_FLYING) && !IS_SET (tch->act, ACT_FLYING))
				continue;
			if ((af = get_affect (tch, AFFECT_GUARD_DIR)))
			{
				if (af->a.shadow.edge == dir)
				{
					if (get_affect (tch, MAGIC_HIDDEN))
					{
						remove_affect_type (tch, MAGIC_HIDDEN);
						act
							("$N emerges from hiding and moves to block your egress in that direction.",
							true, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
						act
							("$n attempts to move past you, but you emerge from hiding and intercept $m.",
							true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					else
					{
						act ("$N moves to block your egress in that direction.",
							true, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
						act
							("$n attempts to move past you, but you intercept $m.",
							true, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					}
					clear_moves (ch);
					return;
				}
			}
		}
	}

	if (IS_MORTAL (ch) && (!IS_SET (flags, MF_SWIM) &&
		SWIM_ONLY (vtor (room_exit->to_room)) &&
		!IS_SWIMMING (ch)) && !IS_SET (ch->act, ACT_FLYING))
	{
		send_to_char ("You'll have to swim for it.\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_SET (flags, MF_SWIM) &&
		!IS_SWIMMING (ch) && !SWIM_ONLY (vtor (room_exit->to_room)))
	{
		send_to_char ("You can't swim there.\n", ch);
		clear_moves (ch);
		return;
	}

	if (IS_SET (flags, MF_SWIM) || SWIM_ONLY (target_room))
	{
		if (IS_RIDER (ch))
		{
			send_to_char ("Dismount first.\n", ch);
			return;
		}
		else if (IS_RIDEE (ch))
		{
			send_to_char ("Your rider must dismount first.\n", ch);
			return;
		}
		move_names[ch->speed] = "swimming";
		move_names2[ch->speed] = "swim";
	}

	needed_movement =
		calc_movement_charge (ch, dir, wanted_time, flags, &speed, &speed_name,
		&walk_time);

	// Move failed for some reason, i.e. too much encumbrance

	if (needed_movement == -1)
	{
		clear_moves (ch);
		return;
	}

	if (GET_MOVE (ch) < needed_movement)
	{
		send_to_char ("You are too exhausted.\n", ch);
		clear_moves (ch);
		return;
	}

	if (speed == -1)
	{
		if (IS_SUBDUER (ch))
		{
			sprintf (buf, "You start %s %s, dragging $N.",
				move_names[speed_name], dirs[dir]);
			act (buf, true, ch, 0, ch->subdue, TO_CHAR);
			sprintf (buf, "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, true, ch->subdue, 0, ch, TO_CHAR);
		}
		if (IS_MOUNT (ch) && ch->mount && ch->mount->subdue)
		{
			sprintf (buf, "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, true, ch->subdue, 0, ch, TO_CHAR);
			act (buf, true, ch->mount->subdue, 0, ch->mount, TO_CHAR);
		}
	}

	if (IS_SET (flags, MF_TOEDGE))
	{
		exit_speed = speed;
		enter_speed = 0;
	}

	else if (IS_SET (flags, MF_TONEXT_EDGE))
	{
		exit_speed = 1;
		enter_speed = speed;
	}

	else
	{
		exit_speed = (speed + 1) / 2;
		enter_speed = speed / 2;
	}

	if (get_affect (ch, AFFECT_GUARD_DIR))
		remove_affect_type (ch, AFFECT_GUARD_DIR);

	if (IS_SET (flags, MF_SNEAK))
	{
		if (dir != OUTSIDE && dir != INSIDE)
			sprintf (buf, "You cautiously begin sneaking %sward.\n", dirs[dir]);
		else
			sprintf (buf, "You cautiously begin sneaking %s.\n", dirs[dir]);
		send_to_char (buf, ch);
		exit_speed += 4;
		if (ch->speed > 2)
			ch->speed = 2;
	}


	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		move_names[ch->speed] = duplicateString ("travelling");
		travel_str[0] = '\0';	/* override travel strings in vehicles */
	}
	else if ((strlen (travel_str) == 0) && (ch->travel_str != NULL))
	{
		sprintf (travel_str, ", %s", ch->travel_str);
	}

	if (dir == OUTSIDE || dir == INSIDE)
		*suffix = '\0';
	else
		sprintf (suffix, "ward");

	int true_speed;
	if (is_with_group (ch))
	{
		true_speed = speed_group(ch);
	}
	else
	{
		true_speed = ch->speed;
	}

	
	if ((ch->room->sector_type == SECT_INSIDE && !IS_SET (flags, MF_SNEAK)
		&& ch->speed != SPEED_IMMORTAL) || (!IS_MORTAL (ch)
		&& ch->speed != SPEED_IMMORTAL
		&& !IS_RIDER(ch)									
		&& !IS_SET (flags, MF_SNEAK)))
	{
		sprintf (buf, "You begin %s %s%s%s.\n", move_names[true_speed],
			dirs[dir], suffix, travel_str);
		sprintf (buf1, "$n begins %s %s%s%s.", move_names[true_speed], dirs[dir],
			suffix, travel_str);
		send_to_char (buf, ch);
		act (buf1, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}

	if (ch->room->sector_type != SECT_INSIDE && IS_MORTAL (ch)
		&& !IS_SET (flags, MF_SNEAK) && !IS_SUBDUER (ch) && !is_mounted (ch)
		&& !IS_RIDEE (ch))
	{

		switch (weather_info[ch->room->zone].state)
		{
		case LIGHT_RAIN:
			exit_speed += 2;
			needed_movement = (int) round (((float) needed_movement) * 1.2);
			sprintf (buf, "You %s %s%s through the light rain%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1, "$n %ss %s%s through the light rain%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		case STEADY_RAIN:
			exit_speed += 4;
			needed_movement = (int) round (((float) needed_movement) * 1.5);
			sprintf (buf, "You %s %s%s through the rain%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1, "$n %ss %s%s through the rain%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		case HEAVY_RAIN:
			exit_speed += 6;
			needed_movement *= 2;
			sprintf (buf,
				"You %s %s%s, struggling through the lashing rain%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1,
				"$n %ss %s%s, struggling through the lashing rain%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		case LIGHT_SNOW:
			needed_movement = (int) round (((float) needed_movement) * 1.5);
			sprintf (buf, "You %s %s%s through the light snowfall%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1, "$n %ss %s%s through the light snowfall%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		case STEADY_SNOW:
			exit_speed += 8;
			needed_movement *= 2;
			sprintf (buf, "You %s %s%s through the steadily falling snow%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1, "$n %ss %s%s through the steadily falling snow%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		case HEAVY_SNOW:
			exit_speed += 12;
			needed_movement *= 4;
			sprintf (buf,
				"You %s %s%s, struggling through the shrieking snow%s.\n",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			sprintf (buf1,
				"$n %ss %s%s, struggling through the shrieking snow%s.",
				move_names2[true_speed], dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		default:
			sprintf (buf, "You begin %s %s%s%s.\n", move_names[true_speed],
				dirs[dir], suffix, travel_str);
			sprintf (buf1, "$n begins %s %s%s%s.", move_names[true_speed],
				dirs[dir], suffix, travel_str);
			send_to_char (buf, ch);
			act (buf1, true, ch, 0, 0, TO_ROOM);
			break;
		}
	}

	if (IS_SET (ch->act, ACT_VEHICLE))
	{
		sprintf (buf, "%s", char_short (ch));
		*buf = toupper (*buf);
		sprintf (buf1, "#5%s#0 begins %s %s%s.\n", buf, move_names[true_speed],
			dirs[dir], suffix);
		send_to_room (buf1, ch->mob->nVirtual);
		needed_movement = 0;
	}

	exit_room (ch, dir, flags, exit_speed, enter_speed, speed_name,
		needed_movement,
		strlen (travel_str) ? duplicateString (travel_str) : NULL);
}

int
isguarded (ROOM_DATA * room, int dir)
{
	if (!room)
		return 0;
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;

	if (!room->people)
		return 0;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		if ((af = get_affect (tch, AFFECT_GUARD_DIR)))
			if (af->a.shadow.edge == dir)
				return 1;
	}

	return 0;
}

void
move (CHAR_DATA * ch, char *argument, int dir, int speed)
{
	MOVE_DATA *move;
	MOVE_DATA *tmove;
	QE_DATA *qe;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = "";
	int i = 0, j = 0;
	char *tmp = NULL;

	if (get_affect (ch, MAGIC_TOLL))
		stop_tolls (ch);

	
		argument = one_argument (argument, buf);
	

	move = new MOVE_DATA;
	move->next = NULL;
	move->dir = dir;
	move->desired_time = speed;
	move->travel_str = tmp;

	if (get_affect (ch, MAGIC_SNEAK))
		move->flags = MF_SNEAK;
	else
	{
		move->flags = 0;
		if (!ch->mob || !IS_SET (ch->affected_by, AFF_HIDE))
			remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if (!str_cmp (buf, "swim"))
		move->flags |= MF_SWIM;

	if (!IS_MORTAL (ch) && !str_cmp (buf, "!"))
		move->flags |= MF_PASSDOOR;

	if (!str_cmp (buf, "stand"))	/* Stand */
		move->flags = MF_TOEDGE;

	if (!ch->moves)
	{

		for (qe = quarter_event_list; qe && qe->ch != ch; qe = qe->next)
			;

		if (qe && qe->dir == rev_dir[dir] && GET_FLAG (ch, FLAG_LEAVING))
		{

			send_to_char ("You turn around.\n", ch);
			act ("$n changes directions and returns.", true, ch, 0, 0,
				TO_ROOM | _ACT_FORMAT);

			qe->dir = dir;
			qe->event_time = qe->arrive_time;

			qe->flags |= MF_ARRIVAL;
			ch->flags &= ~FLAG_LEAVING;
			ch->flags |= FLAG_ENTERING;

			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (!IS_NPC (tch))
					continue;
				if (tch->following != ch)
					continue;
				for (qe = quarter_event_list; qe && qe->ch && qe->ch != tch;
					qe = qe->next)
					;

				send_to_char ("You turn around.\n", tch);
				act ("$n changes directions and returns.", true, tch, 0, 0,
					TO_ROOM | _ACT_FORMAT);

				if (qe)
				{
					qe->dir = dir;
					qe->event_time = qe->arrive_time;

					qe->flags |= MF_ARRIVAL;
					tch->flags &= ~FLAG_LEAVING;
					tch->flags |= FLAG_ENTERING;
				}
			}

			return;
		}

		ch->moves = move;

		if (!GET_FLAG (ch, FLAG_LEAVING) && !GET_FLAG (ch, FLAG_ENTERING))
			initiate_move (ch);

		return;
	}

	for (tmove = ch->moves; tmove->next;)
		tmove = tmove->next;

	tmove->next = move;
}

void
do_move (CHAR_DATA * ch, char *argument, int dir)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[AVG_STRING_LENGTH];
	char command[AVG_STRING_LENGTH];

	if (!can_move (ch))
		return;

	if ( !IS_NPC(ch) )
	{
		for (af = ch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
				&& af->a.craft->timer)
			{
				send_to_char ("You'll need to stop crafting first.\n", ch);
				return;
			}
		}
	}

	/* If you are trying to move, bag the pmote */
	clear_pmote (ch);

	if ((af = get_affect(ch, MAGIC_GUARD)) && af->a.spell.modifier == 1)
		affect_remove(ch, af);

	if (get_second_affect (ch, SA_WARNED, NULL))
	{
		add_second_affect(SA_FLEEING_WARNED, 3, ch, 0, 0, 0);
		criminalize(ch, NULL, ch->room->zone, CRIME_FLEE);
		for (CHAR_DATA * shoutchar = ch->room->people; shoutchar; shoutchar = shoutchar->next_in_room)
		{
			if (!IS_SET(shoutchar->act, ACT_ENFORCER))
				continue;
			if (ch == shoutchar)
				continue;
			if (shoutchar->pc)
				continue;
			if (shoutchar->desc)
				continue;

			if (shoutchar->race == lookup_race_id("Black Numenorean"))
			{
				do_shout (shoutchar, "Halt now or you will not leave this place alive!", 0);
			}
			else if (isOrkin(shoutchar))
			{
				do_shout (shoutchar, "Oi! Youse not gunna get far! After da little bitch!", 0);
			}
			else
			{
				do_shout (shoutchar, "The fugitive is fleeing! Give chase!", 0);
			}
			break;
		}
		send_to_room("\n", ch->room->nVirtual);
		remove_second_affect(get_second_affect(ch, SA_WARNED, NULL));
		add_second_affect(SA_ALREADY_WARNED, 3600, ch, NULL, NULL, -1);

	}

	if (get_affect (ch, MAGIC_TOLL))
		stop_tolls (ch);

	if (dir == UP)
		sprintf (command, "up");
	else if (dir == DOWN)
		sprintf (command, "down");
	else if (dir == EAST)
		sprintf (command, "east");
	else if (dir == WEST)
		sprintf (command, "west");
	else if (dir == NORTH)
		sprintf (command, "north");
	else
		sprintf (command, "south");

	if (ch->room->dir_option[dir]
	&& vtor (ch->room->dir_option[dir]->to_room)
		&& IS_SET (vtor (ch->room->dir_option[dir]->to_room)->room_flags, FALL)
		&& !IS_SET (ch->act, ACT_FLYING) && *argument != '!')
	{
		sprintf (buf,
			"#6Moving in that direction will quite likely result in a rather nasty fall. If you're sure about this, type \'%s !\' to confirm.#0",
			command);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!is_mounted (ch) && GET_POS (ch) == POSITION_FIGHTING)
	{

		/* Make sure nobody is trying to fight us before
		declining move request */

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch == ch)
				continue;

			if (tch->fighting == ch && GET_POS (tch) >= POSITION_FIGHTING)
			{
				send_to_char ("You can't get away - try to FLEE instead!\n",
					ch);
				return;
			}
		}

	}

	if (IS_RIDER (ch))
	{
			//poorly trained mount 
		if (real_skill(ch->mount, SKILL_RIDE) < 33)
		{
				//untrained rider - no riding at all
				//they should not even be mounted
			if (real_skill (ch, SKILL_RIDE) == 0)
			{
				sprintf (buf,
						 "You are not sure how you mounted, but you can't make your mount move.");
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				dump_rider (ch,true);
				return;	
			}
				//trained rider - 
				//first fail means a chance to get bucked off
				//first success - can move easily
				//second failure means they do get dumped
				//success on second check means reistance from mount but they can move
			else if (!skill_use (ch, SKILL_RIDE, 0))
			{
				if (number(1, 100) > 50)
				{
					dump_rider (ch,false); //they get a skill check here
					return;	
				}
				sprintf (buf,
						 "Your mount fails to heed your signals at first, but eventually you force them in the direction  you want.");
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}
			
		}
			//moderatly trained mount
		else if ((real_skill(ch->mount, SKILL_RIDE) >= 33)
				 && (real_skill(ch->mount, SKILL_RIDE) < 50))
		{
			//untrained rider - can walk only, and only in safe terrain
			if (real_skill (ch, SKILL_RIDE) == 0)
			{
							
				if ((ch->pc->mount_speed > 0)  
					|| (ch->room->sector_type == SECT_WOODS)
					|| (ch->room->sector_type == SECT_FOREST)
					|| (ch->room->sector_type == SECT_HILLS)
					|| (ch->room->sector_type == SECT_MOUNTAIN)
					|| (ch->room->sector_type == SECT_SWAMP)
					|| (ch->room->sector_type == SECT_HEATH))
				{
					dump_rider (ch,true);
					return;
				}

			}
				//trained rider - skill learning for move -faster- than walk or difficult terrain
			else if ((ch->pc->mount_speed > 1)
					 || (ch->room->sector_type == SECT_WOODS)
					 || (ch->room->sector_type == SECT_FOREST)
					 || (ch->room->sector_type == SECT_HILLS)
					 || (ch->room->sector_type == SECT_MOUNTAIN)
					 || (ch->room->sector_type == SECT_SWAMP)
					 || (ch->room->sector_type == SECT_HEATH))
			{
				skill_use (ch, SKILL_RIDE, 0);
			}
			
			
		}
		
			//well trained mount - no skill learning for trained riders
		else if (real_skill(ch->mount, SKILL_RIDE) >= 50)
		{
				//untrained rider - can walk only - any terrain
			if (real_skill (ch, SKILL_RIDE) == 0)
			{
				if (ch->pc->mount_speed > 0)
				{
					dump_rider (ch,true);
					return;
				}
			}
				
			
		}
		
		ch = ch->mount;
	}
	move (ch, argument, dir, 0);


}

void
hitches_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time)
{
	MOVE_DATA *m;

	if (!IS_HITCHER (ch) ||
		GET_FLAG (ch->hitchee, FLAG_ENTERING) ||
		GET_FLAG (ch->hitchee, FLAG_LEAVING))
		return;

	while (ch->moves)
	{
		m = ch->moves;
		ch->moves = m->next;
		if (m->travel_str)
			free_mem (m->travel_str);
		free_mem (m); // MOVE_DATA*
	}

	clear_current_move (ch->hitchee);

	move (ch->hitchee, "", dir, leave_time + arrive_time);
}


void
do_east (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, EAST);
}

void
do_west (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, WEST);
}

void
do_north (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, NORTH);
}

void
do_south (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTH);
}

void
do_up (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, UP);
}

void
do_down (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, DOWN);
}

int
find_door (CHAR_DATA * ch, char *type, char *dir)
{
	char buf[MAX_STRING_LENGTH];
	int door;
	char *dirs[] = {
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"\n"
	};

	if (*dir)			/* a direction was specified */
	{
		if ((door = search_block (dir, dirs, false)) == -1)	/* Partial Match */
		{
			send_to_char ("That's not a direction.\n", ch);
			return (-1);
		}

		if (EXIT (ch, door))
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
				if (isname (type, EXIT (ch, door)->keyword))
					return (door);
				else
				{
					sprintf (buf, "I see no %s there.\n", type);
					send_to_char (buf, ch);
					return (-1);
				}
			else
				return (door);
		else
		{
			send_to_char ("There is nothing to open or close there.\n", ch);
			return (-1);
		}
	}
	else				/* try to locate the keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
					if (isname (type, EXIT (ch, door)->keyword))
						return (door);

		sprintf (buf, "I see no %s here.\n", type);
		send_to_char (buf, ch);
		return (-1);
	}
}


void
do_open (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char buffer[MAX_STRING_LENGTH];
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument_interpreter (argument, type, dir);

	if (!*type)
	{
		send_to_char ("Open what?\n", ch);
	}

	if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
	{

		/* this is an object */

		if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
		{
			if (obj->open)
			{
				send_to_char ("It's already opened.\n", ch);
				return;
			}
			if (!*dir || atoi (dir) <= 1)
			{
				/* load on demand */
				if (obj->o.od.value[0] && !obj->writing)
				{
					load_writing(obj);
				}

				if (!obj->writing || !obj->o.od.value[0])
				{
					sprintf (buf,
						"You open #2%s#0, and notice that it has no pages.\n",
						obj->short_description);
					send_to_char (buf, ch);
					sprintf (buf, "%s#0 opens #2%s#0.", char_short (ch),
						obj->short_description);
					sprintf (buffer, "#5%s", CAP (buf));
					act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
					obj->open = 1;
					return;
				}
				sprintf (buf, "You open #2%s#0 to the first page.\n",
					obj->short_description);
				send_to_char (buf, ch);
				sprintf (buf, "%s#0 opens #2%s#0 to the first page.",
					char_short (ch), obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = 1;
				return;
			}
			else if (atoi (dir) > obj->o.od.value[0])
			{
				send_to_char ("There aren't that many pages in the book.\n",
					ch);
				return;
			}
			else
			{
				sprintf (buf, "You open #2%s#0 to page %d.\n",
					obj->short_description, atoi (dir));
				send_to_char (buf, ch);
				sprintf (buf, "%s#0 opens #2%s#0.", char_short (ch),
					obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = atoi (dir);
				return;
			}
		}
		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
			{
				send_to_char ("That cannot be opened or closed.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("That's already open.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's locked.\n", ch);
				return;
			}
			obj->o.od.value[2] &= ~CONT_CLOSED;
			act ("You open $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n opens $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0 && vtor (obj->o.od.value[0])
				&& vtor (obj->o.od.value[0])->dir_option[OUTSIDE])
			{
				send_to_room ("The entryway is opened from the other side.",
					obj->o.od.value[0]);
				vtor (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info &=
					~EX_CLOSED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("But it's already open!\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
			send_to_char ("You can't do that.\n", ch);
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("It seems to be locked.\n", ch);
		else
		{
			obj->o.container.flags &= ~CONT_CLOSED;
			act ("You open $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n opens $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* perhaps it is a door */

		/* TODO: find_object_dwelling causes a crash if a guest is logged in ??? */

		if (door > 5 && (obj = find_dwelling_obj (ch->room->nVirtual)))
		{

			if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
			{
				send_to_char ("This entryway cannot be opened or closed.\n",
					ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("The entrance is already open.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's locked.\n", ch);
				return;
			}
			obj->o.od.value[2] &= ~CONT_CLOSED;
			sprintf (buf, "#2%s#0 is opened from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's impossible, I'm afraid.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("It's already open!\n", ch);
		else if (IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It seems to be locked.\n", ch);
		else
		{
			EXIT (ch, door)->exit_info &= ~EX_CLOSED;
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				sprintf (buf, "You open the %s $T.", relative_dirs[door]);
				act (buf, false, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s $T.", relative_dirs[door]);
				act (buf, false, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You open the %s door.", relative_dirs[door]);
				act ("You open the door.", false, ch, 0, 0,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s door.", relative_dirs[door]);
				act ("$n opens the door.", false, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
			}
			/* now for opening the OTHER side of the door! */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info &= ~EX_CLOSED;
						if (back->keyword && strlen (back->keyword))
						{
							sprintf (buf,
								"The %s %s %s opened from the other side.\n",
								relative_dirs[rev_dir[door]], back->keyword,
								(back->keyword[strlen (back->keyword) - 1] ==
								's') ? "are" : "is");
							/* was  "The %s is opened from the other side.\n",
							fname(back->keyword)); */
							send_to_room (buf, EXIT (ch, door)->to_room);
						}
						else
						{
							sprintf (buf,
								"The %s door is opened from the other side.",
								relative_dirs[rev_dir[door]]);
							send_to_room (buf, EXIT (ch, door)->to_room);
						}
					}
		}
	}
}


void
do_close (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	if (GET_POS (ch) == POSITION_FIGHTING)
	{
		send_to_char ("No way! You are fighting for your life!\n\r", ch);
		return;
	}

	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Close what?\n", ch);
	else
		if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
		{

			/* this is an object */

			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
			{
				if (!obj->open)
				{
					send_to_char ("That isn't currently open.\n", ch);
					return;
				}
				sprintf (buf, "You close #2%s#0.\n", obj->short_description);
				send_to_char (buf, ch);
				sprintf (buf, "%s#0 closes #2%s#0.", char_short (ch),
					obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				act (buffer, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = 0;
				return;
			}
			if (obj->obj_flags.type_flag == ITEM_DWELLING)
			{
				if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
				{
					send_to_char ("That cannot be opened or closed.\n", ch);
					return;
				}
				else if (IS_SET (obj->o.od.value[2], CONT_CLOSED))
				{
					send_to_char ("That's already closed.\n", ch);
					return;
				}
				obj->o.od.value[2] |= CONT_CLOSED;
				act ("You close $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n closes $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
				if (obj->o.od.value[0] > 0 && vtor (obj->o.od.value[0])) {
					send_to_room ("The entryway is closed from the other side.",
						obj->o.od.value[0]);
					vtor (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info |=
						EX_CLOSED;
				}
				return;
			}
			else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
				send_to_char ("That's not a container.\n", ch);
			else if (IS_SET (obj->o.container.flags, CONT_CLOSED))
				send_to_char ("But it's already closed!\n", ch);
			else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
				send_to_char ("That's impossible.\n", ch);
			else
			{
				obj->o.container.flags |= CONT_CLOSED;
				act ("You close $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
				act ("$n closes $p.", false, ch, obj, 0, TO_ROOM);
			}
		}
		else if ((door = find_door (ch, type, dir)) >= 0)
		{

			/* Or a door */

			if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
				&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
				send_to_char ("That's absurd.\n", ch);
			else if (IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
				send_to_char ("It's already closed!\n", ch);
			else
			{
				if (door > 5 && (obj = find_dwelling_obj (ch->room->nVirtual)))
				{
					if (!IS_SET (obj->o.od.value[2], CONT_CLOSEABLE))
					{
						send_to_char ("This entryway cannot be opened or closed.\n",
							ch);
						return;
					}
					else if (IS_SET (obj->o.od.value[2], CONT_CLOSED))
					{
						send_to_char ("The entrance is already closed.\n", ch);
						return;
					}
					else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
					{
						send_to_char ("I'm afraid that's locked.\n", ch);
						return;
					}
					obj->o.od.value[2] |= CONT_CLOSED;
					sprintf (buf, "#2%s#0 is closed from the other side.",
						obj_short_desc (obj));
					buf[2] = toupper (buf[2]);
					send_to_room (buf, obj->in_room);
				}
				EXIT (ch, door)->exit_info |= EX_CLOSED;
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
				{
					sprintf (buf, "You close the %s $T.", relative_dirs[door]);
					act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s $T.", relative_dirs[door]);
					act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
						TO_ROOM | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "You close the %s door.", relative_dirs[door]);
					act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s door.", relative_dirs[door]);
					act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
				}
				/* now for closing the other side, too */
				if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
					if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
						if (back->to_room == ch->in_room)
						{
							back->exit_info |= EX_CLOSED;
							if (back->keyword && strlen (back->keyword))
							{
								sprintf (buf,
									"The %s %s closes quietly.\n",
									relative_dirs[rev_dir[door]], back->keyword);
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								sprintf (buf,
									"The %s door closes quietly.\n",
									relative_dirs[rev_dir[door]]);
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}
			}
		}
}


OBJ_DATA *
has_key (CHAR_DATA * ch, OBJ_DATA * obj, int key)
{
	OBJ_DATA *tobj;

	if (ch->right_hand && ch->right_hand->nVirtual == key)
	{
		tobj = ch->right_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->left_hand && ch->left_hand->nVirtual == key)
	{
		tobj = ch->left_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_KEYRING)
	{
		for (tobj = ch->right_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_KEYRING)
	{
		for (tobj = ch->left_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	return 0;
}


void
do_lock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Lock what?\n", ch);
	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (obj->o.od.value[3] <= 0)
			{
				send_to_char ("That cannot be locked.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("You'll need to close it first.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's already locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] |= CONT_LOCKED;
			act ("You lock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n locks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0) {
				send_to_room ("The entryway is unlocked from the other side.",
					obj->o.od.value[0]);
				vtor (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info |=
					EX_LOCKED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("Maybe you should close it first...\n", ch);
		else if (obj->o.container.key <= 0)
			send_to_char ("That thing can't be locked.\n", ch);
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			send_to_char ("You don't seem to have the proper key.\n", ch);
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("It is locked already.\n", ch);
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags |= CONT_LOCKED;
			act ("You lock $p.", false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n locks $p.", false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* a door, perhaps */

		if ((obj = find_dwelling_obj (ch->room->nVirtual)))
		{
			if (!IS_SET (obj->o.od.value[2], CONT_CLOSED))
			{
				send_to_char ("You'll need to close the entrance first.\n", ch);
				return;
			}
			else if (IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that's already locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] |= CONT_LOCKED;
			sprintf (buf, "#2%s#0 is locked from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's absurd.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("You have to close it first, I'm afraid.\n", ch);
		else if (EXIT (ch, door)->key < 0)
			send_to_char ("There does not seem to be any keyholes.\n", ch);
		else if (!has_key (ch, NULL, EXIT (ch, door)->key))
			send_to_char ("You don't have the proper key.\n", ch);
		else if (IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It's already locked!\n", ch);
		else
		{
			EXIT (ch, door)->exit_info |= EX_LOCKED;
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				//dwellings don't have directions for doors
				if (ch->in_room < 100000)
					sprintf (buf, "You lock the %s $T.", relative_dirs[door]);
				else
					sprintf (buf, "You lock the $T.");

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);

				if (ch->in_room < 100000)
					sprintf (buf, "$n locks the %s $T.", relative_dirs[door]);
				else
					sprintf (buf, "$n locks the $T.");

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You lock the %s door.", relative_dirs[door]);
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);

				if (ch->in_room < 100000)
					sprintf (buf, "$n locks the %s door.", relative_dirs[door]);
				else
					sprintf (buf, "$n locks the door.");


				act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			}
			/* now for locking the other side, too */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info |= EX_LOCKED;
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s %s is locked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								else
									sprintf (buf,
									"The %s is locked from the other side.\n",
									back->keyword);

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s door is locked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								else
									sprintf (buf,
									"The door is locked from the other side.\n");
								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}

					}
		}
	}
}


void
do_unlock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		send_to_char ("Unlock what?\n", ch);

	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag == ITEM_DWELLING)
		{
			if (obj->o.od.value[3] <= 0)
			{
				send_to_char ("That cannot be unlocked.\n", ch);
				return;
			}
			else if (!IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that isn't locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] &= ~CONT_LOCKED;
			sprintf(buf, "You open $p with %s.", key->short_description);
			act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "You open $p with %s.", key->short_description);
			act (buf, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
			if (obj->o.od.value[0] > 0) {
				send_to_room ("The entryway is unlocked from the other side.",
					obj->o.od.value[0]);
				vtor (obj->o.od.value[0])->dir_option[OUTSIDE]->exit_info &=
					~EX_LOCKED;
			}
			return;
		}
		else if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char ("That's not a container.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			send_to_char ("It isn't closed.\n", ch);
		else if (obj->o.container.key <= 0)
			send_to_char ("Odd - you can't seem to find a keyhole.\n", ch);
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			send_to_char ("You don't have the proper key.\n", ch);
		else if (!IS_SET (obj->o.container.flags, CONT_LOCKED))
			send_to_char ("Oh.. it wasn't locked, after all.\n", ch);
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags &= ~CONT_LOCKED;
			sprintf(buf, "You unlock $p with %s.", key->short_description);
			act (buf, false, ch, obj, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n unlocks $p with %s.", key->short_description);
			act (buf, false, ch, obj, 0, TO_ROOM | _ACT_FORMAT);
		}

	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* it is a door */

		if ((obj = find_dwelling_obj (ch->room->nVirtual)))
		{
			if (!IS_SET (obj->o.od.value[2], CONT_LOCKED))
			{
				send_to_char ("I'm afraid that isn't locked.\n", ch);
				return;
			}
			else if (!(key = has_key (ch, obj, obj->o.od.value[3])))
			{
				send_to_char ("You don't seem to have the proper key.\n", ch);
				return;
			}
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.od.value[2] &= ~CONT_LOCKED;
			sprintf (buf, "#2%s#0 is unlocked from the other side.",
				obj_short_desc (obj));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, obj->in_room);
		}

		if (!IS_SET (EXIT (ch, door)->exit_info, EX_ISDOOR)
			&& !IS_SET (EXIT (ch, door)->exit_info, EX_ISGATE))
			send_to_char ("That's absurd.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED))
			send_to_char ("Heck.. it ain't even closed!\n", ch);
		else if (EXIT (ch, door)->key < 0)
			send_to_char ("You can't seem to spot any keyholes.\n", ch);
		else if (!(key = has_key(ch, NULL, EXIT(ch, door)->key)))
			send_to_char ("You do not have the proper key for that.\n", ch);
		else if (!IS_SET (EXIT (ch, door)->exit_info, EX_LOCKED))
			send_to_char ("It's already unlocked, it seems.\n", ch);
		else
		{
			EXIT (ch, door)->exit_info &= ~EX_LOCKED;
			if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
			{
				//dwellings don't have directions for door
				if (ch->in_room < 100000) {
					sprintf (buf, "You unlock the %s $T with %s.", relative_dirs[door], key->short_description);
				}
				else {
					sprintf (buf, "You unlock the $T with %s.", key->short_description);
				}

				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s $T with %s.", relative_dirs[door], key->short_description);
				act (buf, 0, ch, 0, EXIT (ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				//dwellings don't have directions for doors
				if (ch->in_room < 100000)
					sprintf (buf, "You unlock the %s door with %s.", relative_dirs[door], key->short_description);
				else
					sprintf (buf, "You unlock the door with %s.", key->short_description);

				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s door with %s.", relative_dirs[door], key->short_description);
				act (buf, false, ch, 0, 0, TO_ROOM);
			}
			/* now for unlocking the other side, too */
			if ((other_room = EXIT (ch, door)->to_room) != NOWHERE)
				if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->exit_info &= ~EX_LOCKED;
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								//dwellings don't have directions for doors
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s %s is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								else
									sprintf (buf,
									"The %s is unlocked from the other side.\n",
									back->keyword);

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
							else
							{
								//dwellings don't have directions for doors
								if (ch->in_room < 100000)
									sprintf (buf,
									"The %s door is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								else
									sprintf (buf,
									"The door is unlocked from the other side.\n");

								send_to_room (buf, EXIT (ch, door)->to_room);
							}
						}

					}
		}
	}
}

void do_pick (CHAR_DATA * ch, char *argument, int cmd) {
	int dir;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *locked_obj = NULL;

	if (!real_skill (ch, SKILL_PICK)) {
		send_to_char ("You don't know how to pick locks.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf) {
		send_to_char ("What would you like to pick?\n", ch);
		return;
	}

	if (!(ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_LOCKPICK) &&
		!(ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_LOCKPICK)) {
			send_to_char ("You must be holding a lockpick.\n", ch);
			return;
	}

	if (!(locked_obj = get_obj_in_list_vis (ch, buf, ch->room->contents))) {
		locked_obj = get_obj_in_dark (ch, buf, ch->right_hand);
	}

	if (locked_obj) {

		if (locked_obj->obj_flags.type_flag != ITEM_CONTAINER &&
			GET_ITEM_TYPE (locked_obj) != ITEM_DWELLING) {
				act ("You can't pick $p.", false, ch, locked_obj, 0, TO_CHAR);
				return;
		}

		if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER) {
			if (locked_obj->o.od.value[2] <= 0) {
				act ("$p looks unpickable.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.container.flags, CONT_CLOSED)) {
				act ("$p is already open.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.container.flags, CONT_LOCKED)) {
				act ("As you start, you discover $p is already unlocked.",
					false, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}
		else {
			if (locked_obj->o.od.value[3] <= 0) {
				act ("$p looks unpickable.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.od.value[1], CONT_CLOSED)) {
				act ("$p is already open.", false, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if (!IS_SET (locked_obj->o.od.value[1], CONT_LOCKED)) {
				act ("As you start, you discover $p is already unlocked.",
					false, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}

		sprintf (buf, "#3[Guardian: %s%s]#0 Tries to pick the lock of %s in %d.",
			GET_NAME (ch), IS_SET (ch->plr_flags,
			NEW_PLAYER_TAG) ? " (new)" : "",
			locked_obj->short_description, ch->in_room);

		send_to_guardians (buf, 0xFF);

		act ("You begin picking the lock of $p.", false, ch, locked_obj, 0, TO_CHAR);
		act ("$n uses $s tools on $p.", true, ch, locked_obj, NULL, TO_ROOM);

		ch->delay_type = DEL_PICK_OBJ;
		ch->delay = 25 - ch->skills[SKILL_PICK] / 10;
		ch->delay_info1 = (long int) locked_obj;

		return;
	}

	switch (*buf) {
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
			send_to_char ("You may pick north, south, east, west, up, or "
				"down.\n", ch);
			return;
	}

	if (!EXIT (ch, dir)) {
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	if (!IS_SET (EXIT (ch, dir)->exit_info, EX_ISDOOR)
		&& !IS_SET (EXIT (ch, dir)->exit_info, EX_ISGATE)) {
			send_to_char ("No door in that direction.\n", ch);
			return;
	}

	sprintf (buf, "#3[Guardian: %s%s]#0 Tries to pick the lock of %s in %d.",
		GET_NAME (ch),
		IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
		EXIT (ch, dir)->keyword, ch->in_room);

	send_to_guardians (buf, 0xFF);

	act ("You try to pick the $T.", false, ch, 0, EXIT (ch, dir)->keyword, TO_CHAR);

	if (number (1, 100) > ch->skills[SKILL_PICK]) {
		act ("$n uses $s tools on the $T.", true, ch, 0, EXIT (ch, dir)->keyword, TO_ROOM);

		unsigned int luckyBreak = number(1,100);

		if (IS_SET (ch->room->room_flags, LAWFUL)) {
			if (is_guarded (NULL, ch)) {
				if (luckyBreak > (unsigned int) ((ch->aur)/5)) {
					criminalize (ch, NULL, ch->room->zone, CRIME_PICKLOCK);
				}
			}
			else {
				if (luckyBreak > (unsigned int) (ch->aur)) {
					criminalize (ch, NULL, ch->room->zone, CRIME_PICKLOCK);
				}
			}
		}
		else {
			if (is_guarded (NULL, ch)) {
				if (luckyBreak > (unsigned int) ((ch->aur/3))) {
					criminalize (ch, NULL, ch->room->zone, CRIME_PICKLOCK);
				}
			}
		}
	}

	ch->delay_type = DEL_PICK;
	ch->delay = 25 - ch->skills[SKILL_PICK] / 10;
	ch->delay_info1 = dir;
	ch->delay_info2 = ch->in_room;
}

void
delayed_pick_obj (CHAR_DATA * ch)
{
	int roll;
	OBJ_DATA *tobj;
	OBJ_DATA *locked_obj = NULL;
	OBJ_DATA *obj = NULL;

	locked_obj = (OBJ_DATA *) ch->delay_info1;

	if (!is_obj_here (ch, locked_obj, 1))
	{

		if (ch->right_hand && ch->right_hand == locked_obj)
			obj = ch->right_hand;

		if (ch->left_hand && ch->left_hand == locked_obj)
			obj = ch->left_hand;

		if (obj == locked_obj)
		{
			send_to_char ("You stop picking.\n", ch);
			return;
		}
	}

	if (!IS_SET (locked_obj->o.container.flags, CONT_CLOSED) ||
		!IS_SET (locked_obj->o.container.flags, CONT_LOCKED))
	{
		send_to_char ("You stop picking.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER)
	{
		if (ch->skills[SKILL_PICK] > locked_obj->o.container.pick_penalty)
			skill_use (ch, SKILL_PICK, locked_obj->o.container.pick_penalty);

		if ((roll =
			number (1,
			100)) >
			ch->skills[SKILL_PICK] - locked_obj->o.container.pick_penalty)
		{
			if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)))
			{
				act ("You fail miserably, snapping your pick in the process!",
					false, ch, 0, 0, TO_CHAR);
				act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", false, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", true, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}
	else if (GET_ITEM_TYPE (locked_obj) == ITEM_DWELLING)
	{
		if (ch->skills[SKILL_PICK] > locked_obj->o.od.value[4])
			skill_use (ch, SKILL_PICK, locked_obj->o.od.value[4]);

		if ((roll =
			number (1,
			100)) > ch->skills[SKILL_PICK] - locked_obj->o.od.value[4])
		{
			if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)))
			{
				act ("You fail miserably, snapping your pick in the process!",
					false, ch, 0, 0, TO_CHAR);
				act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0,
					TO_ROOM | _ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", false, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", true, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}

	act ("You have successfully picked the lock.", true, ch, 0, 0, TO_CHAR);
	act ("$n has picked the lock of $p.", true, ch, locked_obj, 0,
		TO_ROOM | _ACT_FORMAT);

	if (GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER)
		locked_obj->o.container.flags &= ~CONT_LOCKED;
	else
		locked_obj->o.od.value[2] &= ~CONT_LOCKED;
}

void
delayed_pick (CHAR_DATA * ch) {
	ROOM_DATA *troom;
	OBJ_DATA *tobj;
	int dir;
	int roll;

	dir = ch->delay_info1;

	if (ch->delay_info2 != ch->in_room || !EXIT (ch, dir)) {
		return;
	}

	if (IS_SET (EXIT (ch, dir)->exit_info, EX_PICKPROOF)) {
		send_to_char ("You failed.\n", ch);
		return;
	}

	/* Don't let PC get any practice out of this lock if it is beyond
	his/her ability. */

	if (ch->skills[SKILL_PICK] > EXIT (ch, dir)->pick_penalty) {
		skill_use (ch, SKILL_PICK, EXIT (ch, dir)->pick_penalty);
	}

	if ((roll = number (1, 100)) > ch->skills[SKILL_PICK] - EXIT (ch, dir)->pick_penalty) {
		if (!(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK))) {
			act ("You fail miserably, snapping your pick in the process!", false, ch, 0, 0, TO_CHAR);
			act ("$n mumbles as $s lockpick snaps.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			extract_obj (tobj);
			return;
		}
		send_to_char ("You failed.\n", ch);
		return;
	}

	// blah = ~(blah & pos) | (blah & ~pos)
	// toggles the EX_LOCKED flag
	// -Vermonkey
	ch->room->dir_option[dir]->exit_info = ~(ch->room->dir_option[dir]->exit_info & EX_LOCKED) | (ch->room->dir_option[dir]->exit_info & ~EX_LOCKED);

	if ((troom = vtor (ch->room->dir_option[dir]->to_room))){
		troom->dir_option[rev_dir[dir]]->exit_info = ~(troom->dir_option[rev_dir[dir]]->exit_info & EX_LOCKED) | (troom->dir_option[rev_dir[dir]]->exit_info & ~EX_LOCKED);
	}

	send_to_char ("You successfully picked the lock.\n", ch);
}

void
enter_vehicle (CHAR_DATA * ch, CHAR_DATA * ent_mob)
{
	if (!ent_mob->mob)
	{
		send_to_char ("You can't.\n", ch);
		return;
	}

	for (CHAR_DATA *tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		AFFECTED_TYPE *af;
		if (!(af = get_affect(tch, MAGIC_GUARD)) || af->a.spell.modifier || (CHAR_DATA *) af->a.spell.t != ent_mob)
			continue;
		act ("You cannot enter that as $N is guarding it.", true, ch, 0, tch, TO_CHAR);
		return;
	}

	if (!vtor (ent_mob->mob->nVirtual))
	{
		send_to_char ("A note on the entrance says, 'broken'\n", ch);
		system_log ("Attempt to use a broken boat or hitch, enter_vehicle()",
			true);
		return;
	}

	if (ch == ent_mob)
		return;  //you can't enter yourself

	if (!room_avail(vtor (ent_mob->mob->nVirtual), NULL, ch))
	{
		send_to_char("There is not enough room.\n", ch);
		return;
	}

	if (IS_SET (ent_mob->act, ACT_MOUNT) && IS_SET(ent_mob->act, ACT_VEHICLE)) {
		act ("You climb into $N.", false, ch, 0, ent_mob, TO_CHAR);
		act ("$n climbs into $N.", true, ch, 0, ent_mob, TO_NOTVICT);
	}
	else {
		act ("You enter $N.", false, ch, 0, ent_mob, TO_CHAR);
		act ("$n enters $N.", true, ch, 0, ent_mob, TO_NOTVICT);
	}

	char_from_room (ch);
	remove_affect_type (ch, MAGIC_SNEAK);
	remove_affect_type (ch, MAGIC_HIDDEN);

	char_to_room (ch, ent_mob->mob->nVirtual);

	ch->vehicle = ent_mob;
	ch->coldload_id = ent_mob->coldload_id;

	if (!IS_NPC (ch))
		ch->pc->boat_virtual = ent_mob->mob->nVirtual;

	act ("$n has boarded.", true, ch, 0, 0, TO_ROOM);

	do_look (ch, "", 15);
}


void
do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *room;
	OBJ_DATA *obj;
	char buf[MAX_INPUT_LENGTH];
	char tmp[MAX_STRING_LENGTH];
	int door;
	CHAR_DATA *ent_mob, *tch;
	int occupants = 0;

	one_argument (argument, buf);

	if (!IS_NPC (ch) && IS_SET (ch->flags, FLAG_GUEST) && *buf)
	{
		*buf = tolower (*buf);
		if (!str_cmp (buf, "arena"))
		{
			arena__do_enter (ch, argument, cmd);
			return;
		}
	}


	if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST) && *buf && ch->in_room == 66957)
	{
		*buf = tolower (*buf);
		if (!str_cmp (buf, "pit"))
		{
			te_pit_do_enter (ch, argument, cmd);
			return;
		}
	}

	/** Entering a vehicle***/
	if (*buf && (ent_mob = get_char_room_vis (ch, buf)) &&
		IS_SET (ent_mob->act, ACT_VEHICLE))
	{
		enter_vehicle (ch, ent_mob);
		return;
	}

	/** Enter a dwelling **/
	if (*buf && (obj = get_obj_in_list_vis (ch, buf, ch->room->contents))
		&& (GET_ITEM_TYPE (obj) == ITEM_DWELLING))
	{
		argument = one_argument (argument, buf);
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHABLE)
			&& !IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED))
		{
			send_to_char ("You'll need to pitch it first.\n", ch);
			return;
		}
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			AFFECTED_TYPE *af;
			if (!(af = get_affect(tch, MAGIC_GUARD)) || !af->a.spell.modifier || (OBJ_DATA *) af->a.spell.t != obj)
				continue;
			act ("You cannot enter $p as $N is guarding it.", true, ch, obj, tch, TO_CHAR | _ACT_FORMAT);
			return;
		}
		if (IS_SET (obj->o.od.value[2], CONT_CLOSED)
			&& !(!IS_MORTAL (ch) && *argument == '!'))
		{
			send_to_char ("You'll need to open it before entering.\n", ch);
			return;
		}
		if (obj->o.od.value[5] < 99000 || obj->o.od.value[5] > 99999)
		{
			send_to_char
				("This dwelling object doesn't have a master floorplan set!\n",
				ch);
			return;
		}
		room = generate_dwelling_room (obj);
		if (!room)
		{
			send_to_char
				("There is an error with that dwelling object. Please report.\n",
				ch);
			return;
		}
		for (tch = room->people; tch; tch = tch->next_in_room)
			occupants++;

		if (obj->o.od.value[1] > 0 && occupants >= obj->o.od.value[1])
		{
			send_to_char ("There are already too many people inside.\n", ch);
			return;
		}
		sprintf (buf, "You enter #2%s#0.", obj_short_desc (obj));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n enters #2%s#0.", obj_short_desc (obj));
		act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		obj->o.od.value[0] = room->nVirtual;
		ch->was_in_room = ch->in_room;
		char_from_room (ch);
		char_to_room (ch, room->nVirtual);
		send_to_char ("\n", ch);
		do_look (ch, "", 0);
		sprintf (buf, "$n enters from the outside.");
		act (buf, true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		if (IS_MORTAL (ch))
			room->occupants++;
		return;
	}

	if (*buf)			/* an argument was supplied, search for door keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->keyword && strlen (EXIT (ch, door)->keyword))
					if (!str_cmp (EXIT (ch, door)->keyword, buf))
					{
						do_move (ch, "", door);
						return;
					}
					sprintf (tmp, "There is no %s here.\n", buf);
					send_to_char (tmp, ch);
	}
	else if (IS_SET (vtor (ch->in_room)->room_flags, INDOORS))
		send_to_char ("You are already indoors.\n", ch);
	else
	{
		/* try to locate an entrance */
		for (door = 0; door <= LAST_DIR; door++)
			if (EXIT (ch, door))
				if (EXIT (ch, door)->to_room != NOWHERE)
					if (!IS_SET (EXIT (ch, door)->exit_info, EX_CLOSED) &&
						IS_SET (vtor (EXIT (ch, door)->to_room)->room_flags, INDOORS))
					{
						do_move (ch, "", door);
						return;
					}
					send_to_char ("You can't seem to find anything to enter.\n", ch);
	}
}


void
leave_vehicle (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *vehicle;
	ROOM_DATA *troom;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (ch->vehicle)
	{

		vehicle = ch->vehicle;

		if (vehicle->mob->nVirtual != ch->in_room)
		{
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}

		act ("$n disembarks.", true, ch, 0, 0, TO_ROOM);

		troom = vtor (vehicle->in_room);

		if (troom->sector_type == SECT_REEF ||
			troom->sector_type == SECT_OCEAN ||
			is_room_affected (troom->affects, MAGIC_ROOM_FLOOD))
		{
			if (*buf != '!')
			{
				act ("$N is at sea.", false, ch, 0, vehicle, TO_CHAR);
				send_to_char ("If you jump ship into the water, type:\n"
					"   #3leave !#0\n", ch);
				return;
			}
		}

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		if (!IS_NPC (ch))
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", true, ch, 0, vehicle, TO_ROOM);

		do_look (ch, "", 15);
		return;
	}
	else if (IS_SET (ch->room->room_flags, VEHICLE))
	{
		if (!(vehicle = get_mob_vnum (ch->room->nVirtual)))
		{
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}
		act ("$n disembarks.", true, ch, 0, 0, TO_ROOM);

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		if (!IS_NPC (ch))
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", true, ch, 0, vehicle, TO_ROOM);

		do_look (ch, "", 15);
		return;
	}

	send_to_char ("You cannot exit the craft from here.\n", ch);
}

void
do_leave (CHAR_DATA * ch, char *argument, int cmd)
{

	if (ch->vehicle || IS_SET (ch->room->room_flags, VEHICLE))
	{
		leave_vehicle (ch, argument);
		return;
	}

	do_outside (ch, argument, 0);
}

void
do_stand (CHAR_DATA * ch, char *argument, int cmd)
{
	int already_standing = 0;
	int dir;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH] = "";
	std::string first_person, third_person;

	if (!can_move (ch))
	{
		send_to_char ("You can't move.\n", ch);
		return;
	}

	if(ch->delay_type == DEL_HIDE)
	{
		send_to_char ("You cannot move while looking for a place to hide.\n", ch);
		return;
	}

	if (*argument != '(')
		argument = one_argument (argument, buf);

	if (*buf)
	{
		if ((dir = index_lookup (dirs, buf)) == -1)
		{
			send_to_char ("Stand where?\n", ch);
			return;
		}

		if (!(af = get_affect (ch, AFFECT_SHADOW)))
		{

			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
			af->a.shadow.edge = -1;	/* Not on an edge */
		}

		move (ch, "stand", dir, 0);
		return;
	}

	if (get_second_affect (ch, SA_STAND, NULL))
		return;

	if (GET_POS (ch) == STAND)
		already_standing = 1;

	switch (GET_POS (ch))
	{
	case STAND:
		act ("You are already standing.", false, ch, 0, 0, TO_CHAR);
		return;
		break;

	case SIT:
		if ((af = get_affect (ch, AFFECT_COVER_NORTH)) ||
			(af = get_affect (ch, AFFECT_COVER_EAST)) ||
			(af = get_affect (ch, AFFECT_COVER_SOUTH)) ||
			(af = get_affect (ch, AFFECT_COVER_WEST)) ||
			(af = get_affect (ch, AFFECT_COVER_UP)) ||
			(af = get_affect (ch, AFFECT_COVER_DOWN)))
		{
			clear_pmote(ch);
			first_person.assign("You rise up from cover");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 rises out of cover");
			remove_cover(ch, 0);

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS (ch) = STAND;
			else
				return;

			break;
		}

		else if ((af = get_affect (ch, MAGIC_SIT_TABLE)) &&
			is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents))
		{
			first_person.assign("You get up from #2");
			first_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			first_person.append("#0");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 gets up from #2");
			third_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			third_person.append("#0");

		}
		else
		{
			first_person.assign("You stand up");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 clambers on #5");
			third_person.append(HSHR(ch));
			third_person.append("#0 feet");
		}
		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			GET_POS (ch) = STAND;
		else
			return;

		break;

	case REST:

		first_person.assign("You stop resting, and stand up");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 stops resting, and clambers on #5");
		third_person.append(HSHR(ch));
		third_person.append("#0 feet");

		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			GET_POS (ch) = POSITION_STANDING;
		else
			return;

		break;

	case SLEEP:
		do_wake (ch, argument, 0);
		break;

	case FIGHT:
		act ("You are standing and fighting.", false, ch, 0, 0, TO_CHAR);
		break;

	default:
		act ("You stop floating around, and put your feet on the ground.",
			false, ch, 0, 0, TO_CHAR);
		act ("$n stops floating around, and puts $s feet on the ground.",
			true, ch, 0, 0, TO_ROOM);
		break;
	}

	if (!already_standing && GET_POS (ch) == STAND)
		follower_catchup (ch);

	if (GET_POS (ch) != SIT && GET_POS (ch) != SLEEP &&
		(af = get_affect (ch, MAGIC_SIT_TABLE)))
		affect_remove (ch, af);
}


void
do_sit (CHAR_DATA * ch, char *argument, int cmd)
{
	int count;
	CHAR_DATA *tch;
	SECOND_AFFECT *sa;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]="";
	std::string first_person, third_person;

	if (*argument != '(')
		argument = one_argument (argument, buf);

	if ((sa = get_second_affect (ch, SA_STAND, NULL)))
		remove_second_affect (sa);

	if (*buf && !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		send_to_char ("You don't see that to sit at.\n", ch);
		return;
	}

	if (obj)
	{

		if (!IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
		{
			act ("You cannot sit at $p.", false, ch, obj, 0, TO_CHAR);
			return;
		}

		if (GET_POS (ch) != STAND)
		{
			act ("You must be standing before you can sit at $p.",
				false, ch, obj, 0, TO_CHAR);
			return;
		}

		count = 0;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch == ch)
				continue;

			if ((af = get_affect (tch, MAGIC_SIT_TABLE)) &&
				af->a.table.obj == obj)
				count++;
		}

		if (obj->o.container.table_max_sitting != 0 &&
			count >= obj->o.container.table_max_sitting)
		{
			act ("There is no available space at $p.", false, ch, obj, 0,
				TO_CHAR);
			return;
		}

		first_person.assign("You sit at #2");
		first_person.append(obj_short_desc(obj));
		first_person.append("#0");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 sits at #2");
		third_person.append(obj_short_desc(obj));
		third_person.append("#0");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
		{
			magic_add_affect (ch, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0);

			af = get_affect (ch, MAGIC_SIT_TABLE);

			af->a.table.obj = obj;

			GET_POS (ch) = SIT;
		}

		return;
	}

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
		{
			first_person.assign("You sit down");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 sits down");

			if (evaluate_emote_string (ch, &first_person, third_person, argument))
			{
				GET_POS (ch) = POSITION_SITTING;
			}
			else
				return;
		}
		break;
	case POSITION_SITTING:
		{
			send_to_char ("You're sitting already.\n", ch);
		}
		break;
	case POSITION_RESTING:
		{
			first_person.assign("You stop resting, and sit up");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 stops resting");
			if (evaluate_emote_string (ch, &first_person, third_person, argument))
				GET_POS (ch) = POSITION_SITTING;
			else
				return;
		}
		break;
	case POSITION_SLEEPING:
		{
			act ("You have to wake up first.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_FIGHTING:
		act ("YOU'RE FIGHTING!  YOU'LL GET SLAUGHTERED!!!",
			false, ch, 0, 0, TO_CHAR);
		act ("You sit down in combat.", false, ch, 0, 0, TO_CHAR);
		act ("$n sits down in combat.", true, ch, 0, 0, TO_ROOM);
		GET_POS (ch) = SIT;
		break;
	default:
		{
			act ("You stop floating around, and sit down.", false, ch, 0, 0,
				TO_CHAR);
			act ("$n stops floating around, and sits down.", true, ch, 0, 0,
				TO_ROOM);
			GET_POS (ch) = POSITION_SITTING;
		}
		break;
	}
	clear_pmote(ch);
}


void
do_rest (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;

	if (!can_move (ch))
	{
		send_to_char ("You can't move.\n", ch);
		return;
	}

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
		{
			first_person.assign("You sit down and rest your tired bones");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 sits down and rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS(ch) = POSITION_RESTING;
			else
				return;
		}
		break;
	case POSITION_SITTING:
		{
			first_person.assign("You rest your tired bones");
			third_person.assign("#5");
			third_person.append(char_short(ch));
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				GET_POS(ch) = POSITION_RESTING;
			else
				return;
		}
		break;
	case POSITION_RESTING:
		{
			act ("You are already resting.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_SLEEPING:
		{
			act ("You have to wake up first.", false, ch, 0, 0, TO_CHAR);
		}
		break;
	case POSITION_FIGHTING:
		{
			act ("Rest while fighting? are you MAD?", false, ch, 0, 0, TO_CHAR);
		}
		break;
	default:
		{
			act ("You stop floating around, and stop to rest your tired bones.",
				false, ch, 0, 0, TO_CHAR);
			act ("$n stops floating around, and rests.", false, ch, 0, 0,
				TO_ROOM);
			GET_POS (ch) = POSITION_SITTING;
		}
		break;
	}

	clear_pmote(ch);
}


void
do_sleep (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;
	clear_pmote (ch);

	switch (GET_POS (ch))
	{
	case POSITION_STANDING:
	case POSITION_SITTING:
	case POSITION_RESTING:

		first_person.assign("You fall asleep");
		third_person.assign("#5");
		third_person.append(char_short(ch));
		third_person[2] = toupper (third_person[2]);

		if (IS_SET (ch->act, ACT_MOUNT))
		{
			third_person.append("#0 lowers #5");
			third_person.append(HSHR(ch));
			third_person.append("#0 head and falls asleep");
		}
		else
			third_person.append("#0 falls asleep");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
			GET_POS (ch) = POSITION_SLEEPING;
		else
			return;

		break;

	case POSITION_SLEEPING:
		send_to_char ("You are already sound asleep.\n", ch);
		break;

	case POSITION_FIGHTING:
		send_to_char ("Sleep while fighting? are you MAD?\n", ch);
		break;

	default:
		act ("You stop floating around, and lie down to sleep.",
			false, ch, 0, 0, TO_CHAR);
		act ("$n stops floating around, and lie down to sleep.",
			true, ch, 0, 0, TO_ROOM);
		GET_POS (ch) = POSITION_SLEEPING;
		break;
	}
}

int
wakeup (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;

	if (GET_POS (ch) != SLEEP)
		return 0;

	if (get_affect (ch, MAGIC_AFFECT_SLEEP))
		return 0;

	send_to_char ("Your sleep is disturbed.\n", ch);

	if ((af = get_affect (ch, MAGIC_SIT_TABLE)) &&
		is_obj_in_list (af->a.table.obj, ch->room->contents))
		GET_POS (ch) = SIT;
	else
		GET_POS (ch) = REST;

	return 1;
}

void
do_wake (CHAR_DATA * ch, char *argument, int not_noisy)
{
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if (!(tch = get_char_room_vis (ch, buf)))
		{
			if (not_noisy)
				send_to_char ("They aren't here.\n", ch);
			return;
		}
	}

	/* Awaken someone else */

	if (*buf && tch != ch)
	{

		if (GET_POS (ch) == POSITION_SLEEPING)
		{
			send_to_char ("You must be awake yourself to do that.\n", ch);
			return;
		}

		if (GET_POS (tch) != POSITION_SLEEPING)
		{
			act ("$E isn't asleep.", true, ch, 0, tch, TO_CHAR);
			return;
		}

		if (get_affect (tch, MAGIC_AFFECT_SLEEP))
		{
			if (not_noisy)
				act ("$E doesn't respond.", false, ch, 0, tch, TO_CHAR);
			return;
		}

		if (get_second_affect (tch, SA_KNOCK_OUT, NULL))
		{
			if (not_noisy)
				act ("$E can't be roused.", false, ch, 0, tch, TO_CHAR);
			return;
		}

		act ("You wake $M up.", false, ch, 0, tch, TO_CHAR);
		GET_POS (tch) = POSITION_RESTING;

		act ("You are awakened by $n.", false, ch, 0, tch, TO_VICT);

		if ((tch = being_dragged (ch)))
		{
			act ("You awaken to discover $N was dragging you!",
				false, ch, 0, tch, TO_CHAR);
			if ((af = get_affect (tch, MAGIC_DRAGGER)))
				affect_remove (tch, af);
		}

		return;
	}

	/* Awaken yourself */

	if (get_affect (ch, MAGIC_AFFECT_SLEEP))
	{
		send_to_char ("You can't wake up!\n", ch);
		return;
	}

	if (GET_POS (ch) > POSITION_SLEEPING)
		send_to_char ("You are already awake...\n", ch);

	else if (GET_POS (ch) == POSITION_UNCONSCIOUS)
		send_to_char ("You're out cold, I'm afraid.\n", ch);

	else if (get_second_affect (ch, SA_KNOCK_OUT, NULL))
		send_to_char ("Your body is still recovering from trauma.", ch);

	else
	{
		GET_POS (ch) = REST;

		if ((tch = being_dragged (ch)))
		{
			if (GET_POS (ch) == REST)
			{
				act ("You awake to discover $N is dragging you!",
					false, ch, 0, tch, TO_CHAR);
			}
			else
			{
				act ("You awaken to discover $N was dragging you!",
					false, ch, 0, tch, TO_CHAR);
				if ((af = get_affect (tch, MAGIC_DRAGGER)))
					affect_remove (tch, af);
			}
		}
		else
		{
			if (GET_POS (ch) == REST)
				act ("You open your eyes.", false, ch, 0, 0, TO_CHAR);
			else
				act ("You awaken and stand.", false, ch, 0, 0, TO_CHAR);
		}

		act ("$n awakens.", true, ch, 0, 0, TO_ROOM);
	}
}



void
do_sail (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	int to_sector;
	int we_moved = 0;
	char direction[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;

	if (!ch->vehicle)
	{
		send_to_char ("You're not on a sailing vessel.\n", ch);
		return;
	}

	if (!real_skill (ch, SKILL_SAIL))
	{
		send_to_char ("Seek knowledge of sailing first.\n", ch);
		return;
	}

	if (ch->in_room != ch->vehicle->mob->helm_room)
	{
		send_to_char ("Navigation is done from the helm only.\n", ch);
		return;
	}

	argument = one_argument (argument, direction);

	if ((dir = index_lookup (dirs, direction)) == -1)
	{
		send_to_char ("Sail where?\n", ch);
		return;
	}

	if (!CAN_GO (ch->vehicle, dir))
	{
		send_to_char ("You can't sail that way.\n", ch);
		return;
	}

	to_sector = vtor (EXIT (ch->vehicle, dir)->to_room)->sector_type;

	if (is_room_affected (vtor (EXIT (ch->vehicle, dir)->to_room)->affects,
		MAGIC_ROOM_FLOOD))
	{
		send_to_char ("The flooded land there is too shallow to sail.\n", ch);
		return;
	}

	if (to_sector != SECT_REEF &&
		to_sector != SECT_DOCK && to_sector != SECT_OCEAN)
	{
		send_to_char ("Land lies that way.\n", ch);
		return;
	}

	if (skill_use (ch, SKILL_SAIL, 0))
	{
		do_move (ch->vehicle, "", dir);
		we_moved = 1;
	}

	else if (number (0, 4) == 4)
	{
		dir = (dir + 1) % 4;

		if (!CAN_GO (ch->vehicle, dir))
		{
			send_to_char ("The wind stalls.\n", ch);
			return;
		}

		to_sector = vtor (EXIT (ch->vehicle, dir)->to_room)->sector_type;

		if (to_sector != SECT_REEF &&
			to_sector != SECT_DOCK && to_sector != SECT_OCEAN)
		{
			send_to_char ("The wind isn't cooperating.\n", ch);
			return;
		}

		do_move (ch->vehicle, "", dir);

		sprintf (buf, "The wind and currents force you off "
			"course to the %s.\n", dirs[dir]);
		we_moved = 1;
	}

	else
		send_to_char ("The wind stalls.\n", ch);

	if (we_moved)
		//for (tch = character_list; tch; tch = tch->next)
		for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;

			if (tch->deleted)
				continue;

			if (tch->vehicle == ch->vehicle &&
				(IS_OUTSIDE (tch)
				|| tch->vehicle->mob->helm_room == tch->in_room))
				act ("$N sails . . .", false, tch, 0, tch->vehicle, TO_CHAR);
		}
}

/*	track finds the shortest path between a character and a room somewhere
in the mud.
*/

int search_sequence = 0;
int high_water = 0;
int rooms_scanned = 0;
/*
int
track_room (ROOM_DATA * from_room, int to_room)
{
int room_set_1[5000];		// These hardly need to be more than
int room_set_2[5000];		// about 200 elements...5000 tb safe
int *rooms;
int *torooms;
int *tmp;
int targets;
int options;
int i;
int dir;
ROOM_DATA *room;
ROOM_DATA *opt_room;
ROOM_DATA *target_room;

if (!from_room)
return -1;

if (!(target_room = vtor (to_room)))
return -1;

search_sequence++;

rooms = room_set_1;
torooms = room_set_2;

options = 1;

rooms[0] = to_room;

high_water = 0;

while (options)
{

targets = 0;

for (i = 0; i < options; i++)
{

opt_room = vtor (rooms[i]);

for (dir = 0; dir <= LAST_DIR; dir++)
{

if (!opt_room->dir_option[dir] ||
opt_room->dir_option[dir]->to_room == NOWHERE)
continue;

room = vtor (opt_room->dir_option[dir]->to_room);

if (!room)
{
continue;
}

if (!room->dir_option[rev_dir[dir]] ||
room->dir_option[rev_dir[dir]]->to_room !=
opt_room->nVirtual)
{
// printf ("Room %d only connects '%s' with room %d.\n",
// opt_room->nVirtual, dirs [dir], room->nVirtual);
continue;
}

if (room->search_sequence == search_sequence)
continue;

rooms_scanned++;

room->search_sequence = search_sequence;

if (room->nVirtual == from_room->nVirtual)
return rev_dir[dir];

torooms[targets++] = room->nVirtual;

if (targets > high_water)
high_water = targets;
}
}

tmp = rooms;
rooms = torooms;
torooms = tmp;		// Important - must point at other structure.

options = targets;
}

return -1;
}
*/
int
track (CHAR_DATA * ch, int to_room)
{
	int room_set_1[5000];		/* These hardly need to be more than */
	int room_set_2[5000];		/*  about 200 elements...5000 tb safe */
	int *rooms;
	int *torooms;
	int *tmp;
	int targets;
	int options;
	int i;
	int dir;
	int count = 0;
	char buf[MAX_STRING_LENGTH];	/* KILLER CDR */
	ROOM_DATA *room;
	ROOM_DATA *opt_room;
	ROOM_DATA *target_room;

	if (!(target_room = vtor (to_room)))
		return -1;

	if (to_room == ch->in_room)
		return -1;

	search_sequence++;

	rooms = room_set_1;
	torooms = room_set_2;

	options = 1;

	rooms[0] = to_room;

	high_water = 0;

	while (options)
	{

		targets = 0;

		for (i = 0; i < options; i++)
		{

			opt_room = vtor (rooms[i]);

			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!opt_room->dir_option[dir] ||
					opt_room->dir_option[dir]->to_room == NOWHERE)
					continue;

				room = vtor (opt_room->dir_option[dir]->to_room);

				if (!room)
				{
					sprintf (buf, "Huh?  Room %d -> %d (bad)\n",
						opt_room->nVirtual,
						opt_room->dir_option[dir]->to_room);
					send_to_char (buf, ch);
					continue;
				}

				if (!room->dir_option[rev_dir[dir]] ||
					room->dir_option[rev_dir[dir]]->to_room !=
					opt_room->nVirtual)
				{
					/*                      printf ("Room %d only connects '%s' with room %d.\n",
					opt_room->nVirtual, dirs [dir], room->nVirtual);
					*/ continue;
				}

				if (room->search_sequence == search_sequence)
					continue;

				rooms_scanned++;

				room->search_sequence = search_sequence;

				if (GET_TRUST (ch))
				{
					if (!strncmp (room->description, "No Description Set", 18))
					{
						if (count % 12 == 11)
							send_to_char ("\n", ch);
						count++;

						sprintf (buf, "%5d ", room->nVirtual);
						send_to_char (buf, ch);
					}
				}

				if (room->nVirtual == ch->in_room)
					return rev_dir[dir];

				torooms[targets++] = room->nVirtual;

				if (targets > high_water)
					high_water = targets;
			}
		}

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targets;
	}

	if (GET_TRUST (ch))
	{
		sprintf (buf, "\nTotal rooms: %d\n", count);
		send_to_char (buf, ch);
	}

	return -1;
}

char *
crude_name (int race)
{
	char *ptrBodyProto = NULL;
	if ((ptrBodyProto = lookup_race_variable (race, RACE_BODY_PROTO)) != NULL)
	{
		switch (strtol (ptrBodyProto, NULL, 10))
		{
		case PROTO_HUMANOID:
			return "bipedal creature";
		case PROTO_FOURLEGGED_PAWS:
		case PROTO_FOURLEGGED_FEET:
			return "four-legged creature";
		case PROTO_FOURLEGGED_HOOVES:
			return "four-legged, cloven-hoofed creature";
		case PROTO_SERPENTINE:
		case PROTO_WINGED_TAIL:
		case PROTO_WINGED_NOTAIL:
		default:
			;
		}
	}
	return "unknown creature";
}

char *
specific_name (int race)
{
	std::string buf;
	char *ptrBodyProto = NULL;
	char *ptrSizeProto = NULL;
	if ((ptrSizeProto = lookup_race_variable (race, RACE_SIZE)) != NULL)
	{
		switch (strtol (ptrSizeProto, NULL, 10))
		{
		case -3:
			buf.append("miniscule ");
			break;
		case -2:
			buf.append("tiny ");
			break;
		case -1:
			buf.append("small ");
			break;
		case 0:
			buf.append("medium-sized ");
			break;
		case 1:
			buf.append("large ");
			break;
		case 2:
			buf.append("enormous ");
			break;
		case 3:
			buf.append("colossal ");
			break;
		default:
			break;
		}
	}
	if ((ptrBodyProto = lookup_race_variable (race, RACE_BODY_PROTO)) != NULL)
	{
		switch (strtol (ptrBodyProto, NULL, 10))
		{
		case PROTO_WINGED_TAIL:
			buf.append("tailed, bipedal creature");
			break;
		case PROTO_WINGED_NOTAIL:
		case PROTO_HUMANOID:
			buf.append("bipedal creature");
			break;
		case PROTO_FOURLEGGED_PAWS:
		case PROTO_FOURLEGGED_FEET:
			buf.append("four-legged creature");
			break;
		case PROTO_FOURLEGGED_HOOVES:
			buf.append("four-legged, cloven-hoofed creature");
			break;
		case PROTO_SERPENTINE:
			buf.append("serpentine creature");
			break;
		default:
			buf.append("unknown creature");
		}
	}

	if (buf.empty())
		return "unknown creature";

	return (char *) buf.c_str();

}

/*
char *
specific_name (int race)
{
static char buf[MAX_STRING_LENGTH];

///\TODO: Make racially non-specific
switch (race)
{
case 0:   // commoner
case 1:   // dunadan
case 3:   // bn
case 4:   // corsair
case 5:   // dunlending
case 6:   // gondorian human
case 9:   // northman
case 13:  // woodman?
case 16:  // noldo
case 17:  // sinda
case 18:  // vanya
case 19:  // avar
case 25:  // orc
case 26:  // half-orc
case 93:  // silvan
case 94:  // renegade
case 96:  // easterling
case 99:  // other half-orc?
case 101: // arnorian
case 102:   // gondorian human
case 105: // generic undead
return "medium-sized humanoid";
case 24:  // snaga
case 14:  // wose
case 20:  // harfoot
case 21:  // stoor
case 22:  // fallohide
case 23:  // dwarf
return "small humanoid";
case 29:  // uruk-hai
case 104:  // variag
return "large humanoid";
case 27:  // half troll
case 28:  // troll
case 86:  // olog-hai
return "towering humanoid";
default:
strcpy (buf, lookup_race_variable (race, RACE_NAME));
if (!*buf || strcmp (buf, "(null)") == 0)
{
return "unknown creature";
}
*buf = tolower (*buf);
return buf;
}

}
*/
char *
track_age (int hours_passed)
{
	if (hours_passed <= 1)
		return "within the hour";
	else if (hours_passed <= 4)
		return "recently";
	else if (hours_passed <= 6)
		return "within hours";
	else if (hours_passed <= 12)
		return "within a half-day";
	else if (hours_passed <= 24)
		return "within a day";
	else if (hours_passed <= 48)
		return "within a couple days";
	else if (hours_passed <= 72)
		return "within a few days";
	else
		return "within days";
}

char *
speed_adj (int speed)
{
	if (speed == 0)
		return "a brisk walk";
	else if (speed == 1)
		return "a faltering stagger";
	else if (speed == 2)
		return "a deliberate pace";
	else if (speed == 3)
		return "a swift jog";
	else if (speed == 4)
		return "a loping run";
	else
		return "a haphazard sprint";
}

void
delayed_track (CHAR_DATA * ch)
{
	TRACK_DATA *track;
	bool found = false;
	int needed;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];

	skill_use (ch, SKILL_TRACKING, 0);
	*output = '\0';

	for (track = ch->room->tracks; track; track = track->next)
	{
		*buf = '\0';
		needed = ch->skills[SKILL_TRACKING];
		needed -= track->hours_passed / 4;
		if (IS_SET (track->flags, BLOODY_TRACK))
			needed += number (10, 25);
		needed += number (5, 10);
		needed = MAX (needed, 5);
		if (number (1, 100) > needed)
			continue;
		if (!found)
			send_to_char ("\n", ch);
		found = true;
		if (needed < 30)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 are here, leading from %s to %s.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race), dirs[track->from_dir],
				dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 are here, coming from the %s and then doubling back.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race), dirs[track->from_dir]);
		}
		else if (needed < 50)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 were laid here %s, leading from %s to %s.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race),
				track_age (track->hours_passed),
				dirs[track->from_dir], dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2The %stracks of a %s#0 were laid here %s, coming from the %s and then doubling back.",
				IS_SET (track->flags,
				BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
				crude_name (track->race),
				track_age (track->hours_passed),
				dirs[track->from_dir]);
		}
		else if (needed < 90)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.",
				specific_name (track->race), IS_SET (track->flags,
				BLOODY_TRACK) ?
				", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir],
				dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.",
				specific_name (track->race), IS_SET (track->flags,
				BLOODY_TRACK) ?
				", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir]);
		}
		else
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.",
				lookup_race_variable (track->race, RACE_NAME),
				IS_SET (track->flags,
				BLOODY_TRACK) ? ", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir],
				dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
				"#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.",
				lookup_race_variable (track->race, RACE_NAME),
				IS_SET (track->flags,
				BLOODY_TRACK) ? ", #1pooled with blood#0, " : "",
				track_age (track->hours_passed),
				speed_adj (track->speed), dirs[track->from_dir]);
		}
		*buf = toupper (*buf);
		sprintf (output + strlen (output), "%s ", buf);
	}

	if (!found)
	{
		send_to_char ("You were unable to locate any tracks in the area.\n",
			ch);
		return;
	}
	else
	{
		reformat_string (output, &p);
		page_string (ch->desc, p);
		free_mem (p); //char*
	}
}

void
skill_tracking (CHAR_DATA * ch, char *argument, int cmd)
{
	TRACK_DATA *track;
	char buf[MAX_STRING_LENGTH];

	if (ch->room->sector_type == SECT_ROAD
		|| ch->room->sector_type == SECT_CITY)
	{
		send_to_char ("Tracking in such an area is all but impossible.\n", ch);
		return;
	}

	if (!IS_MORTAL (ch))
	{
		if (!ch->room->tracks)
		{
			send_to_char ("There are no tracks here.\n", ch);
			return;
		}
		sprintf (buf, "The following tracks are here:\n\n");
		for (track = ch->room->tracks; track; track = track->next)
		{
			if (track->from_dir)
				sprintf (buf + strlen (buf),
				"%s tracks, from the %s, heading %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->from_dir], dirs[track->to_dir],
				speeds[track->speed], track->hours_passed);
			else
				sprintf (buf + strlen (buf),
				"%s tracks heading to the %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->to_dir], speeds[track->speed],
				track->hours_passed);
			if (IS_SET (track->flags, PC_TRACK))
				strcat (buf, " #2(PC)#0");
			if (IS_SET (track->flags, BLOODY_TRACK))
				strcat (buf, " #1(bloody)#0");
			if (IS_SET (track->flags, FLEE_TRACK))
				strcat (buf, " #6(fled)#0");
			strcat (buf, "\n");
		}
		send_to_char (buf, ch);
		return;
	}

	if (!real_skill (ch, SKILL_TRACKING))
	{
		send_to_char ("You don't know how to track!\n", ch);
		return;
	}

	if (is_dark (ch->room) && IS_MORTAL (ch)
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		send_to_char ("You can't see well enough to pick up any tracks.\n", ch);
		return;
	}

	ch->delay = 8;
	ch->delay_type = DEL_TRACK;
	send_to_char ("You survey the area carefully, searching for tracks...\n",
		ch);
	act ("$n surveys the area slowly, searching for something.", true, ch, 0, 0,
		TO_ROOM | _ACT_FORMAT);
}

void
do_track (CHAR_DATA * ch, char *argument, int cmd)
{
	int to_room;
	int dir;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	bool nodded = false;

	// If an npc was commanded by a player, disallow the track to succeed.
	if (cmd > 0 && IS_NPC (ch) && ((GET_TRUST (ch)) >= (cmd - 1)))
		return;

	if (!IS_NPC (ch))
	{
		if (IS_MORTAL (ch) || !*argument)
		{
			skill_tracking (ch, argument, cmd);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if (isdigit (*buf))
		to_room = atoi (buf);
	else if ((tch = get_char_vis (ch, buf)))
		to_room = tch->in_room;
	else
	{
		send_to_char ("You can't locate them.\n", ch);
		return;
	}

	if (ch->in_room == to_room)
		return;

	dir = track (ch, to_room);

	if (dir == -1)
	{
		send_to_char ("Unknown direction.\n", ch);
		return;
	}

	if (GET_TRUST (ch))
	{
		sprintf (buf, "[High: %d Scan: %d] Move %s\n",
			high_water, rooms_scanned, dirs[dir]);
		send_to_char (buf, ch);
	}

	high_water = 0;
	rooms_scanned = 0;

	one_argument (ch->room->dir_option[dir]->keyword, buf2);
	sprintf (buf, "%s %s", buf2, dirs[dir]);

	if (IS_SET (ch->room->dir_option[dir]->exit_info, EX_LOCKED))
	{
		if (ch->room->dir_option[dir]->key)
		{
			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (get_obj_in_list_num
					(ch->room->dir_option[dir]->key, tch->right_hand)
					|| get_obj_in_list_num (ch->room->dir_option[dir]->key,
					tch->left_hand))
				{
					name_to_ident (tch, buf2);
					sprintf (buf, "%s", buf2);
					do_nod (ch, buf, 0);
					nodded = true;
					break;
				}
			}
		}
		if (!nodded)
			do_knock (ch, buf, 0);
	}
	else if (IS_SET (ch->room->dir_option[dir]->exit_info, EX_CLOSED))
	{
		do_open (ch, buf, 0);
	}

	do_move (ch, "", dir);
}

void
do_drag (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];

	argument = one_argument (argument, name);

	if (!(victim = get_char_room_vis (ch, name)))
	{
		if (!(obj = get_obj_in_list_vis (ch, name, ch->room->contents)))
		{
			send_to_char ("Drag who or what?\n", ch);
			return;
		}
	}

	if (victim)
	{
		if (ch == victim)
		{
			send_to_char ("Drag yourself?\n", ch);
			return;
		}

		if (IS_SWIMMING (ch))
		{
			send_to_char ("You can barely tred water yourself!\n", ch);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		send_to_char ("Drag where?\n", ch);
		return;
	}

	if (victim && AWAKE (victim))
	{
		act ("$N is conscious, you can't drag $M.",
			false, ch, 0, victim, TO_CHAR);
		return;
	}

	if (victim && number (1, 4) == 1)
	{
		do_wake (ch, name, 1);
		if (GET_POS (victim) > SLEEP)
			return;
	}

	if (victim && IS_SUBDUER (ch))
	{
		send_to_char
			("You can't drag anything while you have someone in tow.\n", ch);
		return;
	}

	if (obj && !IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
	{
		act ("$p is firmly attached.  You can't drag it.",
			false, ch, obj, 0, TO_CHAR);
		return;
	}

	magic_add_affect (ch, MAGIC_DRAGGER, -1, 0, 0, 0, 0);

	if ((af = get_affect (ch, MAGIC_DRAGGER)))
		af->a.spell.t = obj ? (long int) obj : (long int) victim;

	do_move (ch, "", dir);
}

int
is_mounted (CHAR_DATA * ch)
{
	/* Is a mount mounted, or has a PC mounted something */

	if (!ch->mount || !is_he_here (ch, ch->mount, false))
		return 0;

	if (ch != ch->mount->mount)
	{
		ch->mount = NULL;
		return 0;
	}

	return 1;
}

int
can_mount (CHAR_DATA * ch, CHAR_DATA * mount)
{
	if (CAN_SEE (ch, mount) &&
		IS_SET (mount->act, ACT_MOUNT) &&
		!IS_SET (ch->act, ACT_MOUNT) &&
		GET_POS (mount) == STAND && !is_mounted (mount))
		return 1;

	return 0;
}

int
hitch_char (CHAR_DATA * ch, CHAR_DATA * hitch)
{
	if (IS_HITCHER (ch))
		return 0;

	if (!IS_SET (hitch->act, ACT_MOUNT))
		return 0;

	if (IS_HITCHEE (hitch))
		return 0;

	ch->hitchee = hitch;
	hitch->hitcher = ch;

	return 1;
}

void
unhitch_char (CHAR_DATA * ch, CHAR_DATA * hitch)
{
	if (ch != hitch->hitcher || ch->hitchee != hitch)
		return;

	ch->hitchee = NULL;
	hitch->hitcher = NULL;
}

void
do_hitch (CHAR_DATA * ch, char *argument, int cmd)
{
	int is_vehicle = 0;
	CHAR_DATA *hitch;
	CHAR_DATA *hitcher;
	OBJ_DATA *bridle;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!(hitch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf && IS_HITCHER (ch))
	{
		act ("You currently have $N hitched.",
			true, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if (hitch->mob && hitch->mob->vehicle_type == VEHICLE_HITCH)
		is_vehicle = 1;

	if (!IS_SET (hitch->act, ACT_MOUNT))
	{
		act ("$N can't be hitched.", true, ch, 0, hitch, TO_CHAR);
		return;
	}

	if (IS_HITCHEE (hitch))
	{
		act ("$N is already hitched to $3.",
			true, ch, (OBJ_DATA *) hitch->hitcher, hitch, TO_CHAR);
		return;
	}

	if (IS_RIDEE (hitch))
	{
		act ("$N is riding that animal.", true, ch, 0, hitch->mount, TO_CHAR);
		return;
	}

	if (!is_vehicle)
	{
		if (!(bridle = get_equip (hitch, WEAR_NECK_1)))
		{
			act ("$N doesn't have a bridle to grab.",
				true, ch, 0, hitch, TO_CHAR);
			return;
		}

		if (GET_ITEM_TYPE (bridle) != ITEM_BRIDLE)
		{
			act ("$N is wearing $o, which is not a bridle.",
				true, ch, bridle, hitch, TO_CHAR);
			return;
		}
	}

	if (!*buf && is_vehicle)
	{
		send_to_char ("You may only hitch a vehicle to an animal or another "
			"vehicle.\n", ch);
		return;
	}

	if (!*buf)
	{

		hitch_char (ch, hitch);

		act ("You grab the reins of $N.", true, ch, 0, hitch, TO_CHAR);
		act ("$n grabs the reins of $N.", false, ch, 0, hitch, TO_NOTVICT);
		act ("$N grabs your reins.", true, hitch, 0, ch, TO_CHAR);

		return;
	}

	if (!str_cmp (buf, "to"))
		argument = one_argument (argument, buf);

	if (!(hitcher = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_SET (hitcher->act, ACT_MOUNT))
	{
		act ("$N can't be hitched.", true, ch, 0, hitcher, TO_CHAR);
		return;
	}

	if (IS_HITCHER (hitcher))
	{
		act ("$N has an animal hitched already.",
			true, ch, 0, hitcher, TO_CHAR);
		return;
	}

	hitch_char (hitcher, hitch);

	act ("$n hitches $N to $3.",
		false, ch, (OBJ_DATA *) hitcher, hitch, TO_NOTVICT);
	act ("$N hitches $3 to you.",
		false, ch, (OBJ_DATA *) hitch, hitcher, TO_VICT);
	act ("You hitch $3 to $N.", true, ch, (OBJ_DATA *) hitch, hitcher, TO_CHAR);
	act ("$N hitches you to $3.",
		true, hitch, (OBJ_DATA *) hitcher, ch, TO_CHAR);
}

void
do_unhitch (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *hitch;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		if (!is_he_here (ch, ch->hitchee, 0))
		{
			send_to_char ("Unhitch what?\n", ch);
			return;
		}

		hitch = ch->hitchee;
	}

	else if (!(hitch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_HITCHEE (hitch))
	{
		act ("$N isn't currently hitched.", true, ch, 0, hitch, TO_CHAR);
		return;
	}

	/* Mount is hitched, but not by us.  Check for non-mount hitch */

	if (hitch->hitcher != ch && !IS_SET (hitch->hitcher->act, ACT_MOUNT))
	{
		act ("$N is holding that animal's reigns.",
			true, ch, 0, hitch->hitcher, TO_CHAR);
		return;
	}

	unhitch_char (hitch->hitcher, hitch);

	act ("$n unhitches $N.", true, ch, 0, hitch, TO_NOTVICT);
	act ("You unhitch $N.", true, ch, 0, hitch, TO_CHAR);
	act ("$N unhitches you.", true, hitch, 0, ch, TO_CHAR);
}

void
do_mount (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mount;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (mount = ch->room->people; mount; mount = mount->next_in_room)
			if (can_mount (ch, mount))
				break;

		if (!mount)
		{
			send_to_char ("There is nothing here you can mount.\n", ch);
			return;
		}
	}

	else if (!(mount = get_char_room_vis (ch, buf)))
	{
		send_to_char ("There is no such mount here.\n", ch);
		return;
	}

	if (!can_mount (ch, mount))
	{
		act ("You can't mount $N.", false, ch, 0, mount, TO_CHAR);
		return;
	}

		//poorly trained mount - untrained rider - no mounting at all
	if ((real_skill (ch, SKILL_RIDE) == 0) && (real_skill(mount, SKILL_RIDE) < 33))
	{
		act ("$N backs away just as you attempt to mount it.",
			false, ch, 0, mount, TO_CHAR);
		act ("You back away as $N approaches you.",
			false, mount, 0, ch, TO_CHAR);
		act ("$N backs away when $n attempts to mount it.",
			false, ch, 0, mount, TO_NOTVICT);
		return;
	}

		//poorly trained mount - trained rider - fail skill means no mounting
	if (!skill_use (ch, SKILL_RIDE, 0) && (real_skill(mount, SKILL_RIDE) < 33))
	{
		act ("You manage to straddle $N, but it quickly throws you off.",
			false, ch, 0, mount, TO_CHAR);
		act ("$n is thrown to the ground after attempting to mount $N.",
			false, ch, 0, mount, TO_NOTVICT);
		act ("You throw $N to the ground after $E attempts to mount you.",
			false, mount, 0, ch, TO_CHAR);
		return;
	}

	
	
	act ("$n perches $mself on top of $N.", false, ch, 0, mount, TO_NOTVICT);
	act ("You mount $N.", false, ch, 0, mount, TO_CHAR);
	act ("$N mounts you.", false, mount, 0, ch, TO_CHAR);

	mount->mount = ch;
	ch->mount = mount;
	
	if (get_affect (mount, MAGIC_GUARD))
		remove_affect_type (mount, MAGIC_GUARD);

	unhitch_char (ch, mount);
}

void
do_tame (CHAR_DATA * ch, char *argument, int cmd)
{
	send_to_char ("This command is under construction.\n", ch);
}

void
do_dismount (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!is_mounted (ch))
	{
		ch->mount = NULL;
		send_to_char ("You're not on a mount.\n", ch);
		return;
	}

	act ("$n dismounts from $N.", false, ch, 0, ch->mount, TO_NOTVICT);
	act ("You dismount from $N.", false, ch, 0, ch->mount, TO_CHAR);
	act ("$N gets off of you.", false, ch->mount, 0, ch, TO_CHAR);

	hitch_char (ch, ch->mount);

	ch->mount->mount = NULL;
	ch->mount = NULL;
}


void
do_bridle (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mount;
	OBJ_DATA *obj;
	OBJ_DATA *bridle;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (IS_HITCHER (ch))
	{
		act ("Unhitch $N first.", true, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if (!*buf)
	{
		send_to_char ("What do you want to bridle?\n", ch);
		return;
	}

	if (!(mount = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if (!IS_SET (mount->act, ACT_MOUNT))
	{
		act ("$N is not a mount; it can't be bridled.",
			false, ch, 0, mount, TO_CHAR);
		return;
	}

	if (!(bridle = ch->right_hand))
	{
		send_to_char ("You need to hold a bridle in your right hand.\n", ch);
		return;
	}

	if (GET_ITEM_TYPE (bridle) != ITEM_BRIDLE)
	{
		act ("$o is not a bridle.", false, ch, bridle, 0, TO_CHAR);
		return;
	}

	if ((obj = get_equip (mount, WEAR_NECK_1)))
	{
		act ("$N already has $o around its neck.",
			false, ch, obj, mount, TO_CHAR);
		return;
	}

		//bridling an untrained mount
	if (!real_skill (mount, SKILL_RIDE))
	{

		if (ch->skills[SKILL_RIDE] < 30)
		{
			skill_use (ch, SKILL_RIDE, 0); //they learn from it
			act ("$N shies away as attempt to slip $o onto $M.",
				false, ch, bridle, mount, TO_CHAR);
			return;
		}

		if (!skill_use (ch, SKILL_RIDE, 0))
		{
			act ("$N kicks you as you attempt to bridle $M!",
				false, ch, 0, mount, TO_CHAR);
			act ("You kick $N when $E tries to bridle you.",
				false, mount, 0, ch, TO_CHAR);
			act ("$N kicks $n as $E attempts to bridle $M.",
				false, ch, 0, mount, TO_NOTVICT);
			wound_to_char (ch, figure_location (ch, 0), dice (2, 12), 3, 0, 0,
				0);

			return;
		}
	}
		//anyone can try to bridle a poorly trained mount but they may take damage
	else if (mount->skills[SKILL_RIDE] < 33)
	{
			//does mount resist for random reason?
		if (number (1, 100) > mount->skills[SKILL_RIDE] * 3)
		{
			//poorly skilled or unskilled PC will take some damage
			if (ch->skills[SKILL_RIDE] < 30)  
			{
				act ("$N kicks you as you attempt to bridle $M!",
					 false, ch, 0, mount, TO_CHAR);
				act ("You kick $N when $E tries to bridle you.",
					 false, mount, 0, ch, TO_CHAR);
				act ("$N kicks $n as $E attempts to bridle $M.",
					 false, ch, 0, mount, TO_NOTVICT);
				wound_to_char (ch, figure_location (ch, 0), dice (2, 6), 3, 0, 0,
							   0);
				skill_use (ch, SKILL_RIDE, 0);//chance to learn
				return;
			}
			else
			{
			act ("$N shies away as attempt to slip $p onto $M.", false, ch,
				bridle, mount, TO_CHAR);
			skill_use (ch, SKILL_RIDE, 0);//chance to learn
			return;
		}
	}
	}

	act ("You slip $p onto $N.", false, ch, bridle, mount, TO_CHAR);
	act ("$N slips $p over your neck.", false, mount, bridle, ch, TO_CHAR);
	act ("$n slips $p over $N's neck.", false, ch, bridle, mount, TO_NOTVICT);

	obj_from_char (&bridle, 0);

	equip_char (mount, bridle, WEAR_NECK_1);

	hitch_char (ch, mount);
}
void
do_unbridle (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mount;
	OBJ_DATA *obj;
	OBJ_DATA *bridle;
	char buf[MAX_STRING_LENGTH];
	
	argument = one_argument (argument, buf);
	
	if (!IS_HITCHER (ch))
	{
		send_to_char ("You must be hitched to the mount you want to unbridle.\n", ch);
		return;
	}
	
	if (!*buf)
	{
		send_to_char ("What mount do you want to unbridle?\n", ch);
		return;
	}
	
	if (!(mount = get_char_room_vis (ch, buf)))
	{
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}
	
	bridle = get_equip (mount, WEAR_NECK_1);
	if (!bridle)
	{
		act ("$N doesn't have anything around its neck.",
			 false, ch, bridle, mount, TO_CHAR);
		return;
	}
	
		//unbridling an untrained mount
	if (!real_skill (mount, SKILL_RIDE))
	{
		
		if (ch->skills[SKILL_RIDE] < 30)
		{
			skill_use (ch, SKILL_RIDE, 0); //they learn from it
			act ("$N shies away as attempt to remove the bridle.",
				 false, ch, 0, mount, TO_CHAR);
			return;
		}
		
		if (!skill_use (ch, SKILL_RIDE, 0))
		{
			act ("$N kicks you as you attempt to unbridle $M!",
				 false, ch, 0, mount, TO_CHAR);
			act ("You kick $N when $E tries to unbridle you.",
				 false, mount, 0, ch, TO_CHAR);
			act ("$N kicks $n as $E attempts to unbridle $M.",
				 false, ch, 0, mount, TO_NOTVICT);
			wound_to_char (ch, figure_location (ch, 0), dice (2, 12), 3, 0, 0,
						   0);
			
			return;
		}
	}
		//anyone can try to unbridle a poorly trained mount
	else if (mount->skills[SKILL_RIDE] < 33)
	{
			//does mount resist for random reason?
		if (number (1, 100) > mount->skills[SKILL_RIDE] * 3)
		{
				//poorly skilled or unskilled PC will take some damage
			if (ch->skills[SKILL_RIDE] < 30)  
			{
				act ("$N kicks you as you attempt to unbridle $M!",
					 false, ch, 0, mount, TO_CHAR);
				act ("You kick $N when $E tries to unbridle you.",
					 false, mount, 0, ch, TO_CHAR);
				act ("$N kicks $n as $E attempts to unbridle $M.",
					 false, ch, 0, mount, TO_NOTVICT);
				wound_to_char (ch, figure_location (ch, 0), dice (2, 6), 3, 0, 0,
							   0);
				skill_use (ch, SKILL_RIDE, 0);//chance to learn
				return;
			}
			else
			{
				act ("$N shies away as attempt to slip $p off $M.", false, ch,
					 bridle, mount, TO_CHAR);
				skill_use (ch, SKILL_RIDE, 0);//chance to learn
				return;
			}
		}
	}
	
	act ("You slip $p off of $N.", false, ch, bridle, mount, TO_CHAR);
	act ("$N slips $p off of your neck.", false, mount, bridle, ch, TO_CHAR);
	act ("$n slips $p off of $N's neck.", false, ch, bridle, mount, TO_NOTVICT);
	
	unequip_char (mount, bridle->location);
	obj_to_char (bridle, ch);
	
	unhitch_char (ch, mount);
}

void
dump_rider (CHAR_DATA * rider, int forced)
{


	if (!IS_RIDER (rider))
		return;

	if (forced || !skill_use (rider, SKILL_RIDE, 0))
	{
		act ("You throw $N off your back.  HEHE HAHA, Walk man, walk!",
			false, rider->mount, 0, rider, TO_CHAR);
		act ("$N bucks and throws you off!",
			false, rider, 0, rider->mount, TO_CHAR);
		act ("$N bucks $n off its back.",
			true, rider, 0, rider->mount, TO_NOTVICT);

		rider->mount->speed = 0;
		rider->mount->mount = NULL;
		rider->mount = NULL;

		if (wound_to_char (rider, figure_location (rider, 1), 1, 3, 0, 0, 0))
			return;


		GET_POS (rider) = SIT;
		add_second_affect (SA_STAND, 5, rider, NULL, NULL, 0);

		return;
	}

	act ("You buck as you pretend to be REAL scared.",
		false, rider->mount, 0, rider, TO_CHAR);
	act ("$N bucks and tries to throw you off!", false, rider, 0, rider->mount, TO_CHAR);
	act ("$N bucks and tries to throw $n off its back.", true, rider, 0, rider->mount, TO_NOTVICT);
}

void
do_buck (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	if (!IS_SET (ch->act, ACT_MOUNT))
	{
		send_to_char ("This is a mount only command.\n", ch);
		return;
	}

	if (!IS_RIDEE (ch))
	{
		send_to_char ("You're not riding anybody.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf == '!')
		dump_rider (ch->mount, true);
	else
		dump_rider (ch->mount, false);
}

void
do_shadow (CHAR_DATA * ch, char *argument, int cmd)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (!(af = get_affect (ch, AFFECT_SHADOW)) || !af->a.shadow.shadow)
		{
			send_to_char ("You aren't shadowing anybody.\n", ch);
			return;
		}

		send_to_char ("Ok.\n", ch);

		af->a.shadow.shadow = NULL;

		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("They aren't here to shadow.\n", ch);
		return;
	}

	if (tch == ch)
	{
		if ((af = get_affect (ch, AFFECT_SHADOW)))
		{
			send_to_char ("Ok, you stop shadowing.\n", ch);
			af->a.shadow.shadow = NULL;
			return;
		}

		send_to_char ("Ok.\n", ch);
		return;
	}

	magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

	af = get_affect (ch, AFFECT_SHADOW);

	af->a.shadow.shadow = tch;
	af->a.shadow.edge = -1;	/* Not on an edge */

	act ("You will attempt to shadow $N.", false, ch, 0, tch, TO_CHAR);
}

void
shadowers_shadow (CHAR_DATA * ch, int to_room, int move_dir)
{
	int dir;
	CHAR_DATA *tch;
	CHAR_DATA *next_in_room;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	ROOM_DIRECTION_DATA *exit;
	ROOM_DATA *room;

	/* ch is leaving in direction dir.  Pick up people in same room and
	have them move to the edge.  Pick up people in surrounding rooms on
	edges and have them move to edge of ch's current room too. */

	/* Handle current room shadowers first */

	for (tch = ch->room->people; tch; tch = next_in_room)
	{

		next_in_room = tch->next_in_room;

		if (ch == tch)
			continue;

		if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
			continue;

		if (tch->moves)
			continue;

		if (!(af = get_affect (tch, AFFECT_SHADOW)))
			continue;

		if (af->a.shadow.shadow != ch)
			continue;

		if (!CAN_SEE (tch, ch))
			continue;

		move = new MOVE_DATA;

		move->dir = move_dir;
		move->flags = MF_TOEDGE;
		move->desired_time = 0;
		move->next = NULL;
		move->travel_str = NULL;

		tch->moves = move;

		initiate_move (tch);
	}

	/* Handle people who are on the edge of this room.  Those people
	will head to the edge of this room joining the room that ch is
	about to enter. */

	for (dir = 0; dir <= LAST_DIR; dir++)
	{

		if (!(exit = EXIT (ch, dir)))
			continue;

		if (exit->to_room == to_room)	/* The shadowee is returning */
			continue;

		if (!(room = vtor (exit->to_room)))
			continue;

		for (tch = room->people; tch; tch = next_in_room)
		{

			next_in_room = tch->next_in_room;

			if (ch == tch)
				continue;		/* Hopefully not possible */

			if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
				continue;

			if (tch->moves)
				continue;

			if (!(af = get_affect (tch, AFFECT_SHADOW)))
				continue;

			if (af->a.shadow.shadow != ch)
				continue;

			if (af->a.shadow.edge != rev_dir[dir])
				continue;

			if (!could_see (tch, ch))	/* Make sure shadowee is visable */
				continue;

			/* Make N/PC enter room of ch as ch leaves */

			move = new MOVE_DATA;

			move->dir = af->a.shadow.edge;
			move->flags = MF_TONEXT_EDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves = move;

			/* Make N/PC move to edge joining room ch just entered */

			move = new MOVE_DATA;

			move->dir = move_dir;
			move->flags = MF_TOEDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves->next = move;

			initiate_move (tch);
		}
	}
}

int
room_avail(ROOM_DATA *troom, OBJ_DATA *tobj, CHAR_DATA *tch)
{
	int count = 0;
	int obj_wt = 0;
	int tot_wt = 0;
	float wt_count = 0.0;
	CHAR_DATA *temp_char = NULL;
	OBJ_DATA *temp_obj = NULL;

	if (troom->capacity == 0)
		return(1);

	if (tobj)
		obj_wt = obj_mass(tobj)/100;

	if (tch && (IS_NPC (tch) || IS_MORTAL (tch)))
	{
		if (IS_NPC(tch))
			tot_wt = carrying(tch)/100 + 200;
		else
			tot_wt = carrying(tch)/100 + get_weight(tch)/100;
	}

	for (temp_obj = troom->contents; temp_obj; temp_obj = temp_obj->next_content)
	{
		obj_wt = obj_wt + (obj_mass(temp_obj)/100);
	}

	for (temp_char = troom->people; temp_char; temp_char = temp_char->next_in_room)
	{
		if (IS_NPC (temp_char) || IS_MORTAL (temp_char))
		{
			if (IS_NPC(temp_char))
				tot_wt = tot_wt + carrying(temp_char)/100 + 200;
			else
				tot_wt = tot_wt + carrying(temp_char)/100 + get_weight(temp_char)/100;
			count++;
		}
	}

	wt_count = (tot_wt + obj_wt);

	troom->occupants = (int)(wt_count); //weight in pounds

	if (wt_count >= (troom->capacity * 200))
	{
		return (0); //there is no room
	}
	else
	{
		return (1); //there is room
	}

	return(0);

}


int
force_enter (CHAR_DATA *tch, ROOM_DATA *troom)
{
	int mod;
	int attr_str;
	int attr_agi;
	int num_roll;
	int check_roll;
	float occup;
	float capac;

	attr_str = GET_STR (tch);
	attr_agi = GET_AGI (tch);

	occup = troom->occupants;
	capac = troom->capacity;

	weaken (tch, 0, 50, "Forcing into a room");

	if (capac > 0)
	{
		if (occup > (capac) && (occup <= (1.25 * capac)))
			mod = 1;
		else if (occup > (1.25 * capac) && (occup <= (1.50 * capac)))
			mod = 2;
		else if (occup > (1.50 * capac) && (occup <= (1.75 * capac)))
			mod = 3;
		else if (occup > (1.75 * capac) && (occup <= (2.00 * capac)))
			mod = 4;
		else if (occup > (2.00 * capac))
			mod = 5;

		num_roll = 12 + (mod * 3);
		check_roll = (number (1, num_roll));

		if ( check_roll < attr_str)
		{
			send_to_char ("You force your way inside.\n", tch);
			return (1); //they forced their way in
		}
		else if ((number (1, num_roll) < attr_agi))
		{
			send_to_char ("You manage to squeeze your way in.\n", tch);
			return (1); //they squeezed their way in
		}
		else
			return (0); //they couldn't get in
	}

	return (1); //they had no trouble getting in
}
