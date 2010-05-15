/*------------------------------------------------------------------------\
|  arena.c : Arena Combat Processor                   www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

const int arena_rooms[6] = {
	5142,
	5119,
	5196,
	5197,
	5198,
	5199
};

#define PITBAG_DESC_PREPEND "Belongings for"
#define VNUM_PITBAG 66241

bool arena_stopped = false;
bool morgul_arena_fight = false;
bool morgul_arena_troll_check = false;
bool morgul_arena_warg_check = false;
int morgul_arena_second_echo = 0;
int morgul_arena_third_echo = 0;
int morgul_arena_cleanup_echo = 0;
int morgul_arena_last_echo = 0;
int humans = 0, orcs = 0, wargs = 0; //counts number actually in areana/pit

const int te_pit_rooms[4] = {
	66955,
	66956,
	66954,
	66953
};
bool te_pit_stopped = false;
bool te_pit_fight = false;
bool te_pit_troll_check = false;
bool te_pit_warg_check = false;
bool orcswin = false;
bool humanswin = false;
int te_pit_second_echo = 0;
int te_pit_third_echo = 0;
int te_pit_cleanup_echo = 0;
int te_pit_last_echo = 0;
int te_pit_orc_max_count = 5; //number of NPC Orcs to load for fight
int te_pit_human_max_count = 5; //number of NPC humans to load for fight

void
morgul_arena_wargs (void)
{
	char buf[MAX_STRING_LENGTH];
	int i, numwargs;
	CHAR_DATA *tch;

	if (morgul_arena_warg_check)
		return;

	if (!vtom (5995))
		return;

	numwargs = number (3, 6);
	sprintf (buf,
		"The crowd howls with glee as the gate shrieks open and a snarling, blood-crazed pack of Wargs bounds into the Arena! The gate is shut hurriedly behind them, slamming closed with a thunderous reverberation.");
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	send_to_room (buf, 5142);
	send_to_room (buf, 5119);
	send_to_room (buf, 5196);
	send_to_room (buf, 5197);
	send_to_room (buf, 5198);
	send_to_room (buf, 5199);

	for (i = 1; i <= numwargs; i++)
	{
		tch = load_mobile (5995);
		char_to_room (tch, 5142);
	}

	morgul_arena_warg_check = true;
	wargs = numwargs;
}

void
morgul_arena_troll (void)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	OBJ_DATA *obj;

	if (morgul_arena_troll_check)
		return;

	if (!(tch = load_mobile (5996)))
		return;

	sprintf (buf,
		"Suddenly, the iron-barred gate shrieks open, and #5%s#0 steps through and into the Arena. As it bellows an earthshaking roar, the gate is closed hurriedly behind it!",
		char_short (tch));
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	send_to_room (buf, 5142);
	send_to_room (buf, 5119);
	send_to_room (buf, 5196);
	send_to_room (buf, 5197);
	send_to_room (buf, 5198);
	send_to_room (buf, 5199);
	obj = load_object (5000);
	obj_to_char (obj, tch);
	do_wear (tch, "loincloth", 0);
	char_to_room (tch, 5142);
	morgul_arena_troll_check = true;
}

void
do_arena (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (morgul_arena_fight && !*buf)
	{
		send_to_char ("There is already a fight in progress.\n", ch);
		return;
	}
	else if (!morgul_arena_fight && !*buf)
	{
		morgul_arena_first ();
		send_to_char ("Arena battle initiated.\n", ch);
		return;
	}

	if (!str_cmp (buf, "halt"))
	{
		if (arena_stopped)
		{
			send_to_char
				("The arena is already halted. Type ARENA RESUME to cancel.\n",
				ch);
			return;
		}
		arena_stopped = true;
		send_to_char
			("The arena will not start again until an admin types ARENA RESUME.\n",
			ch);
		return;
	}

	if (!str_cmp (buf, "resume"))
	{
		if (!arena_stopped)
		{
			send_to_char
				("The arena has not been halted by an administrator.\n", ch);
			return;
		}
		arena_stopped = false;
		send_to_char ("The arena will now start again periodically as usual.\n",
			ch);
		return;
	}

	if (!str_cmp (buf, "troll"))
	{
		if (!morgul_arena_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		if (morgul_arena_troll_check)
		{
			send_to_char ("The troll has already been released!\n", ch);
			return;
		}
		morgul_arena_troll ();
		send_to_char ("The troll has been released.\n", ch);
		return;
	}

	if (!str_cmp (buf, "wargs"))
	{
		if (!morgul_arena_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		if (morgul_arena_warg_check)
		{
			send_to_char ("The Wargs have already been released!\n", ch);
			return;
		}
		morgul_arena_wargs ();
		send_to_char ("The Wargs have been released.\n", ch);
		return;
	}

	if (!str_cmp (buf, "stop"))
	{
		if (!morgul_arena_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		send_to_char ("Arena battle cancelled.\n", ch);
		morgul_arena_cleanup ();
		return;
	}

	send_to_char ("Eh? Check HELP ARENA for command syntax.\n", ch);
}

void
morgul_arena_last (void)
{
	char buf[MAX_STRING_LENGTH];

	morgul_arena_last_echo = 0;
	sprintf (buf,
		"Finally, after the Arena floor is scoured clean of battered corpses and dropped equipment, the Uruk-Hai exit via the gate, which is secured behind them.");
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	morgul_arena_fight = false;
	morgul_arena_troll_check = false;
	morgul_arena_warg_check = false;

}

void
morgul_arena_third (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!(tch = d->character) || d->connected != CON_PLYNG)
			continue;
		if (tch->deleted)
			continue;
		if (IS_NPC (tch))
			continue;
		if (!IS_SET (tch->flags, FLAG_GUEST))
			continue;
		if (tch->in_room == 5142 || tch->in_room == 5119 || tch->in_room == 5196
			|| tch->in_room == 5197 || tch->in_room == 5198
			|| tch->in_room == 5199)
			continue;
		act
			("\n#2The Witchking's Arena is no longer accepting new Guest entrants.#0",
			false, tch, 0, 0, TO_CHAR);
	}

	morgul_arena_third_echo = 0;
	sprintf (buf,
		"A deafening horn reverberates throughout the Arena, and the battle begins!");
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	send_to_room (buf, 5142);
	send_to_room (buf, 5119);
	send_to_room (buf, 5196);
	send_to_room (buf, 5197);
	send_to_room (buf, 5198);
	send_to_room (buf, 5199);
	morgul_arena ();
}

void
morgul_arena_second (void)
{
	char buf[MAX_STRING_LENGTH];

	morgul_arena_second_echo = 0;
	sprintf (buf,
		"Under the brutal guidance of a group of Uruk-Hai slavers, the soon-to-be combatants are freed from their bonds and equipped. They array themselves about the Arena in anticipation of the coming battle.");
	morgul_arena_third_echo = 61;
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	send_to_room (buf, 5142);
	send_to_room (buf, 5119);
	send_to_room (buf, 5196);
	send_to_room (buf, 5197);
	send_to_room (buf, 5198);
	send_to_room (buf, 5199);
}

void
morgul_arena_first (void)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	DESCRIPTOR_DATA *d;

	if (!vtor (5144) || !vtor (5201) || !vtor (5200) || !vtor (5142) ||
		!vtor (5119) || !vtor (5196) || !vtor (5197) || !vtor (5198) ||
		!vtor (5199))
		return;

	if (arena_stopped)
		return;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!(tch = d->character) || d->connected != CON_PLYNG)
			continue;
		if (tch->deleted)
			continue;
		if (IS_NPC (tch))
			continue;
		if (!IS_SET (tch->flags, FLAG_GUEST))
			continue;
		act
			("\n#2A battle is about to commence in the Witchking's Arena. To join, simply type ENTER ARENA; you will be returned here when the fight is concluded.#0",
			false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	sprintf (buf,
		"The jagged, iron-barred gate leading into the bowels of the Arena opens with the deafening shriek of tormented metal, and out processes a shackled and chained line of slaves, both Human and Orcish.\n");
	morgul_arena_second_echo = 31;
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
	send_to_room (buf, 5142);
	send_to_room (buf, 5119);
	send_to_room (buf, 5196);
	send_to_room (buf, 5197);
	send_to_room (buf, 5198);
	send_to_room (buf, 5199);
	vtor (5142)->dir_option[3]->exit_info |= EX_LOCKED;
}

void
equip_slave (CHAR_DATA * tmob)
{
	OBJ_DATA *tobj;
	int weapon;

	for (tobj = tmob->equip; tobj; tobj = tobj->next_content)
	{
		if (tobj == tmob->equip)
			tmob->equip = tmob->equip->next_content;
		else
			tmob->equip->next_content = tobj->next_content;
	}
	tmob->right_hand = NULL;
	tmob->left_hand = NULL;

	if (tmob->sex == SEX_MALE)
	{
		tobj = load_object (5000);
		equip_char (tmob, tobj, WEAR_LEGS);
	}
	else
	{
		tobj = load_object (5262);
		equip_char (tmob, tobj, WEAR_BODY);
	}
	weapon = number (1, 10);
	if (weapon == 1)
	{
		tobj = load_object (6005);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 2)
	{
		tobj = load_object (6006);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 3)
	{
		tobj = load_object (6007);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 4)
	{
		tobj = load_object (6009);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 5)
	{
		tobj = load_object (6011);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 6)
	{
		tobj = load_object (5002);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 7)
	{
		tobj = load_object (5501);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}
	else if (weapon == 8)
	{
		tobj = load_object (5045);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 9)
	{
		tobj = load_object (1082);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}
	else
	{
		tobj = load_object (6020);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}

}

void
morgul_arena (void)
{
	CHAR_DATA *tmob;
	int room, num_slaves;

	morgul_arena_fight = true;

	num_slaves = number (5, 25);

	for (; orcs <= num_slaves;)
	{
		tmob = NULL;
		if (!number (0, 3))
			tmob = load_mobile (5997);
		else
			tmob = load_mobile (5998);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		equip_slave (tmob);
		tmob->act |= ACT_AGGRESSIVE;
		room = number (1, 6);
		if (room == 1)
			char_to_room (tmob, 5142);
		else if (room == 2)
			char_to_room (tmob, 5199);
		else if (room == 3)
			char_to_room (tmob, 5119);
		else if (room == 4)
			char_to_room (tmob, 5197);
		else if (room == 5)
			char_to_room (tmob, 5198);
		else if (room == 6)
			char_to_room (tmob, 5196);
		else
			char_to_room (tmob, 5142);
		orcs++;
	}

	for (; humans <= num_slaves;)
	{
		tmob = NULL;
		tmob = load_mobile (5999);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		equip_slave (tmob);
		tmob->act |= ACT_AGGRESSIVE;
		room = number (1, 6);
		if (room == 1)
			char_to_room (tmob, 5142);
		else if (room == 2)
			char_to_room (tmob, 5199);
		else if (room == 3)
			char_to_room (tmob, 5119);
		else if (room == 4)
			char_to_room (tmob, 5197);
		else if (room == 5)
			char_to_room (tmob, 5198);
		else if (room == 6)
			char_to_room (tmob, 5196);
		else
			char_to_room (tmob, 5142);
		humans++;
	}
}

void
arena_engage_echo (CHAR_DATA * ch, CHAR_DATA * vict)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *tch, *ally = NULL;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->fighting == vict && vict->fighting == tch)
		{
			ally = tch;
			break;
		}
	}

	if (ally)
		sprintf (buf, "%s#0 engages #5%s#0, coming to #5%s#0's aid!",
		char_short (ch), char_short (vict), char_short (ally));
	else
		sprintf (buf, "%s#0 engages #5%s#0!", char_short (ch), char_short (vict));

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);
}

int
is_arena_clear (void)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;
	int i = 0;

	for (i = 0; i <= 5; i++)
	{
		room = vtor (arena_rooms[i]);
		if (!room)
			continue;
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (!IS_NPC (tch))
				return 0;
	}

	return 1;
}

void
morgul_arena_cleanup (void)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;
	OBJ_DATA *obj, *next_o;
	CHAR_DATA *tch, *next_c;
	WOUND_DATA *wound, *next_wound;
	int i;

	humans = 0;
	orcs = 0;

	morgul_arena_cleanup_echo = 0;
	room = vtor (5142);
	room->dir_option[3]->exit_info &= ~EX_LOCKED;
	sprintf (buf,
		"The surviving slaves are shackled once more and led back through the gate, as numerous Uruk-Hai flood the Arena floor to remove the corpses of the fallen and various pieces of equipment dropped during the battle.");
	for (i = 0; i <= 5; i++)
	{
		send_to_room (buf, arena_rooms[i]);
	}

	if (morgul_arena_troll_check)
	{
		sprintf (buf,
			"The troll bellows furiously as a team of large Uruk-Hai wrestle it into submission and shackle it. After a great deal of struggle they manage to bring it to heel, and lead it back through the gate.");
		for (i = 0; i <= 5; i++)
		{
			send_to_room ("\n", arena_rooms[i]);
			send_to_room (buf, arena_rooms[i]);
		}
		send_to_room (buf, 5144);
		send_to_room (buf, 5201);
		send_to_room (buf, 5200);
	}

	if (morgul_arena_warg_check)
	{
		sprintf (buf,
			"A group of Goblin handlers scurries out onto the Arena floor, deftly chaining the remaining Wargs and leading them back to the kennels.");
		for (i = 0; i <= 5; i++)
		{
			send_to_room ("\n", arena_rooms[i]);
			send_to_room (buf, arena_rooms[i]);
		}
		send_to_room (buf, 5144);
		send_to_room (buf, 5201);
		send_to_room (buf, 5200);
	}

	for (i = 0; i <= 5; i++)
	{
		room = vtor (arena_rooms[i]);
		for (obj = room->contents; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (!room->people)
			continue;
		for (tch = room->people; tch; tch = next_c)
		{
			next_c = tch->next_in_room;
			if (IS_NPC (tch))
				extract_char (tch);
			else if (!IS_NPC (tch) && IS_SET (tch->flags, FLAG_GUEST)
				&& tch->desc)
			{
				tch->damage = 0;
				for (wound = tch->wounds; wound; wound = next_wound)
				{
					next_wound = wound->next;
					wound_from_char (tch, wound);
				}
				create_guest_avatar (tch->desc, "recreate");
			}
		}
	}

	morgul_arena_last_echo = 31;
}


void
arena__death (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	if (ch->delay_ch)
	{
		sprintf (buf, "%s#0 has been vanquished by #5%s#0!",
			char_short (ch), char_short (ch->delay_ch));
		ch->delay_ch = NULL;
	}
	else if (!ch->delay_ch)
	{
		sprintf (buf, "%s#0 has been vanquished!", char_short (ch));
	}

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);
	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);

	if (ch->race == 0)
	{
		humans--;
	}
	else if (ch->race == 27 || ch->race == 119 || ch->race == 120 || ch->race == 121)
	{
		orcs--;
	}

	if (ch->race == 28 || ch->race == 86)
	{
		morgul_arena_troll_check = false;
	}

	if (ch->race == 89)
	{
		wargs--;
		if (wargs <= 0)
		{
			morgul_arena_warg_check = false;
		}
	}

	if (orcs <= 0 && !morgul_arena_cleanup_echo && morgul_arena_fight)
	{
		send_to_room ("\n", 5144);
		send_to_room ("\n", 5201);
		send_to_room ("\n", 5200);
		sprintf (buf,
			"The crowd roars in dismay as the Human slaves emerge victorious!\n");
		send_to_room (buf, 5144);
		send_to_room (buf, 5201);
		send_to_room (buf, 5200);
		send_to_room (buf, 5142);
		send_to_room (buf, 5119);
		send_to_room (buf, 5196);
		send_to_room (buf, 5197);
		send_to_room (buf, 5198);
		send_to_room (buf, 5199);
		morgul_arena_cleanup_echo = 21;
	}

	if (humans <= 0 && !morgul_arena_cleanup_echo && morgul_arena_fight)
	{
		send_to_room ("\n", 5144);
		send_to_room ("\n", 5201);
		send_to_room ("\n", 5200);
		sprintf (buf,
			"The crowd howls in glee as the Orcish slaves triumph!\n");
		send_to_room (buf, 5144);
		send_to_room (buf, 5201);
		send_to_room (buf, 5200);
		send_to_room (buf, 5142);
		send_to_room (buf, 5119);
		send_to_room (buf, 5196);
		send_to_room (buf, 5197);
		send_to_room (buf, 5198);
		send_to_room (buf, 5199);
		morgul_arena_cleanup_echo = 21;
	}

	if (ch->race == 28 && morgul_arena_troll_check)
	{
		morgul_arena_troll_check = false;
	}

}

void
arena_combat_message (CHAR_DATA * src, CHAR_DATA * tar, char *location,
					  int damage, int room)
{
	char area[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char impact[MAX_STRING_LENGTH];

	if (damage == 0)
		return;

	if (damage < 2)
		return;

	if (room == 5142)
		sprintf (area, "western");
	else if (room == 5199)
		sprintf (area, "northwestern");
	else if (room == 5198)
		sprintf (area, "northeastern");
	else if (room == 5197)
		sprintf (area, "eastern");
	else if (room == 5196)
		sprintf (area, "southeastern");
	else if (room == 5119)
		sprintf (area, "southwestern");

	if (damage > 0 && damage <= 3)
		sprintf (impact, "a glancing blow to");
	else if (damage > 3 && damage <= 6)
		sprintf (impact, "a light blow to");
	else if (damage > 6 && damage <= 10)
		sprintf (impact, "moderately hard on");
	else if (damage > 10 && damage <= 15)
		sprintf (impact, "a heavy blow to");
	else if (damage > 15 && damage <= 20)
		sprintf (impact, "a severe blow to");
	else if (damage > 20 && damage <= 25)
		sprintf (impact, "grievously hard on");
	else if (damage >= 25)
		sprintf (impact, "a devastating blow to");
	else
		return;

	sprintf (buf, "In the %s part of the Arena, #5%s#0", area,
		char_short (src));
	sprintf (buf + strlen (buf), " strikes #5%s#0 %s the %s.", char_short (tar),
		impact, expand_wound_loc (location));

	send_to_room (buf, 5144);
	send_to_room (buf, 5201);
	send_to_room (buf, 5200);

}


void
arena__update_delays (void)
{
	if (morgul_arena_last_echo > 1)
	{
		morgul_arena_last_echo--;
		if (morgul_arena_last_echo == 1)
			morgul_arena_last ();
	}

	if (morgul_arena_cleanup_echo > 1)
	{
		morgul_arena_cleanup_echo--;
		if (morgul_arena_cleanup_echo == 1)
			morgul_arena_cleanup ();
	}

	if (morgul_arena_second_echo > 1)
	{
		morgul_arena_second_echo--;
		if (morgul_arena_second_echo == 1)
			morgul_arena_second ();
	}

	if (morgul_arena_third_echo > 1)
	{
		morgul_arena_third_echo--;
		if (morgul_arena_third_echo == 1)
			morgul_arena_third ();
	}
}

void
arena__do_shout (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	int i = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };


	for (i = 0; i <= 5; i++)
	{
		for (tch = vtor (arena_rooms[i])->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (GET_SEX (ch) == SEX_MALE)
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a male voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a male voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
				{
					sprintf (buf,
						"You hear a male voice shout from the stands,");
				}
			}
			else if (GET_SEX (ch) == SEX_FEMALE)
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a female voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a female voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
					sprintf (buf,
					"You hear a female voice shout from the stands,");
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
					sprintf (buf, "You hear a voice shout from the stands,");
			}
			if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));
			if (!decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf + strlen (buf),
					" though you cannot decipher the words.");
				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
			}
			else if (tch->desc)
			{
				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				if (decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
						sprintf (buf, "   \"%s\"\n", buf4);
					else
						sprintf (buf, "   \"%s\"\n", buf4);
					send_to_char (buf, tch);
				}
			}

			if (GET_TRUST (tch) && !IS_NPC (tch)
				&& GET_FLAG (tch, FLAG_SEE_NAME))
				sprintf (buf3, " (%s)", GET_NAME (ch));
			else
				*buf3 = '\0';
			sprintf (buf2, argument);	/* Reset. */
		}
	}

}

void
arena__do_look (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	ROOM_DATA *troom = NULL;
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, arg1);
	if (!*arg1)
	{
		troom = vtor (5142);
		*buf = '\0';
		send_to_char
			("You gaze out over the blood-hued dirt of the Arena floor below, seeing...\n",
			ch);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the western part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (5199);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the northwestern part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (5198);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the northeastern part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (5197);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the eastern part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (5196);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the southeastern part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (5119);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the southwestern part of the Arena:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		if (!*buf)
			send_to_char
			("\n...nothing of interest. There is no fight currently in progress.\n",
			ch);
		else
		{
		}
		return;
	}
	else
	{
		tch = get_char_room (arg1, 5142);
		if (!tch)
			tch = get_char_room (arg1, 5199);
		if (!tch)
			tch = get_char_room (arg1, 5119);
		if (!tch)
			tch = get_char_room (arg1, 5197);
		if (!tch)
			tch = get_char_room (arg1, 5198);
		if (!tch)
			tch = get_char_room (arg1, 5196);
		if (!tch)
		{
			send_to_char ("You do not see that in the Arena.\n", ch);
			return;
		}

		show_char_to_char (tch, ch, (cmd == 2) ? 15 : 1);
		return;
	}
}


void
arena__do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int roll = number (1, 2);


	if (!morgul_arena_second_echo && !morgul_arena_third_echo)
	{
		send_to_char
			("The Witchking's Arena is not accepting new entrants at the moment.\n",
			ch);
		return;
	}
	if (ch->in_room == 5142 || ch->in_room == 5119 || ch->in_room == 5196 ||
		ch->in_room == 5197 || ch->in_room == 5198 || ch->in_room == 5199)
	{
		send_to_char ("You're already in the Arena!\n", ch);
		return;
	}

	if ((!vtom (5999) || !vtom (5997) || !vtom (5998)) ||
		(!vtor (5142) || !vtor (5119) || !vtor (5196) ||
		!vtor (5197) || !vtor (5198) || !vtor (5199)))
	{
		send_to_char ("The necessary prototypes cannot be found. Aborting.\n",
			ch);
		return;
	}

	act
		("$n suddenly vanishes in a subtle glimmer of light, spirited off to the Witchking's Arena!",
		true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	send_to_char
		("#2\nConverting you to a more proper form in preparation for the fight...\n",
		ch);

	if (roll == 1)
	{
		ch->race = 0;
		send_to_char ("#6\nYou have been placed on the Human team!#0\n", ch);
		humans++;
		ch->description = duplicateString (vtom (5999)->description);
	}
	else if (roll == 2)
	{
		if (!number (0, 9))
		{
			ch->race = 119;
			ch->description = duplicateString (vtom (5997)->description);
		}
		else
		{
			ch->race = 120;
			ch->description = duplicateString (vtom (5998)->description);
		}
		send_to_char ("#1\nYou have been placed on the Orcish team!#0\n", ch);
		orcs++;
	}
	if (IS_SET (ch->plr_flags, NEWBIE_HINTS))
	{
		ch->plr_flags &= ~NEWBIE_HINTS;
	}

	randomize_mobile (ch);

	if (ch->race == 119 || ch->race == 120)
	{
		add_clan (ch, "orcslaves", CLAN_MEMBER);
	}
	else
	{
		add_clan (ch, "humanslaves", CLAN_MEMBER);
	}
	send_to_char ("#2\nThe battle will begin in just a moment. Good luck!#0\n",
		ch);
	equip_slave (ch);
	ch->move = 100;
	ch->max_move = 100;
	ch->hit = 85;
	ch->max_hit = 85;
	ch->desc->original = NULL;
	char_from_room (ch);
	char_to_room (ch, 5142);
	sprintf (buf, "%s %s", ch->name, ch->tname);
	free_mem (ch->name);
	ch->name = duplicateString (buf);
	return;
}

/***********************/
/******** PIT **********/
/***********************/
void

// This is where the wargs come in.

te_pit_wargs (void)
{
	char buf[MAX_STRING_LENGTH];
	int i, numwargs;
	CHAR_DATA *tch;

	if (te_pit_warg_check)
		return;

	if (!vtom (5995))
		return;

	numwargs = number (1, 2);
	sprintf (buf,
		"A wailing howl reverberates throughout the earthen pit, and after a momentary shiver by many of the spectators, a matching howl of bloodthirsty glee issues forth to greet the wargs that bound in to the pit.");
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	send_to_room (buf, 66955);
	send_to_room (buf, 66956);
	send_to_room (buf, 66954);
	send_to_room (buf, 66953);

	for (i = 1; i <= numwargs; i++)
	{
		tch = load_mobile (5995);
		char_to_room (tch, 66953);
	}

	te_pit_warg_check = true;
	wargs = numwargs;
}

// This is where the troll comes in.

void
te_pit_troll (void)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	OBJ_DATA *obj;

	if (te_pit_troll_check)
		return;

	if (!(tch = load_mobile (5996)))
		return;

	sprintf (buf,
		"The wooden stands of the Tur Edendor Battle Pit shake beneath you, the gates being bashed open by #5%s#0, the beast issuing a guttural snarl as it thunders in to the fray.",
		char_short (tch));
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	send_to_room (buf, 66955);
	send_to_room (buf, 66956);
	send_to_room (buf, 66954);
	send_to_room (buf, 66953);
	obj = load_object (5000);
	obj_to_char (obj, tch);
	do_wear (tch, "loincloth", 0);
	char_to_room (tch, 66953);
	te_pit_troll_check = true;
}

void
do_te_pit (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (te_pit_fight && !*buf)
	{
		send_to_char ("There is already a fight in progress.\n", ch);
		return;
	}
	else if (!te_pit_fight && !*buf)
	{
		te_pit_first ();
		send_to_char ("Pit battle initiated.\n", ch);
		return;
	}

	if (!str_cmp (buf, "halt"))
	{
		if (te_pit_stopped)
		{
			send_to_char
				("The pit is already halted. Type pit RESUME to cancel.\n",
				ch);
			return;
		}
		te_pit_stopped = true;
		send_to_char
			("The pit will not start again until an admin types pit RESUME.\n",
			ch);
		return;
	}

	if (!str_cmp (buf, "resume"))
	{
		if (!te_pit_stopped)
		{
			send_to_char
				("The pit has not been halted by an administrator.\n", ch);
			return;
		}
		te_pit_stopped = false;
		send_to_char ("The pit will now start again periodically as usual.\n",
			ch);
		return;
	}

	if (!str_cmp (buf, "troll"))
	{
		if (!te_pit_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		if (te_pit_troll_check)
		{
			send_to_char ("The troll has already been released!\n", ch);
			return;
		}
		te_pit_troll ();
		send_to_char ("The troll has been released.\n", ch);
		return;
	}

	if (!str_cmp (buf, "wargs"))
	{
		if (!te_pit_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		if (te_pit_warg_check)
		{
			send_to_char ("The Wargs have already been released!\n", ch);
			return;
		}
		te_pit_wargs ();
		send_to_char ("The Wargs have been released.\n", ch);
		return;
	}

	if (!str_cmp (buf, "stop"))
	{
		if (!te_pit_fight)
		{
			send_to_char ("There is currently no fight in progress!\n", ch);
			return;
		}
		send_to_char ("Pit battle cancelled.\n", ch);
		te_pit_cleanup ();
		return;
	}

	send_to_char ("Eh? Check HELP te_pit for command syntax.\n", ch);
}

void
te_pit_last (void)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	ROOM_DATA *room, *room_chk;
	OBJ_DATA *obj, *tobj, *next_obj = NULL;
	CHAR_DATA *tch, *next_ch, *keeper;

	te_pit_last_echo = 0;
	sprintf (buf,
		"The dour human slaves finish dragging corpses and broken weaponry out of the bloodied dirt pit, waving away the carrion birds that had already started to descend with a chorus hopeful cawing. Already, goblin bet-takers begin to swarm the stands offering odds and predictions for the next fight, and a few spectators make their way to the betting booth to collect their winnings or place new bets.");
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	te_pit_fight = false;
	te_pit_troll_check = false;
	te_pit_warg_check = false;

	//remove offduty shopkeepers and replace with ticket redeemer shopkeep
	room_chk = vtor (66951);

	for (tch = room_chk->people; tch; tch = next_ch)
	{
		next_ch = tch->next_in_room;
		if (IS_NPC(tch))
		{
			if (tch->mob->nVirtual == 66950)
				extract_char (tch);
			if (tch->mob->nVirtual == 66952)
				extract_char (tch);
		}
	}

	keeper = load_mobile (66951);
	char_to_room (keeper, 66951);

	/**
	66236  an in-progress token
	66237  a warg-won token
	66238  a orc-won token
	66239  a human-won token

	*/
	if (orcswin)
	{
		room = vtor (66951);
		for (obj = room->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (obj->nVirtual == 66236 || obj->nVirtual == 66237 || obj->nVirtual == 66238 || obj->nVirtual == 66239 )
				extract_obj (obj);
		}


		//load orc win result token
		tobj = load_object (66238);
		obj_to_room(tobj, 66951);
	}
	else if (humanswin)
	{
		room = vtor (66951);
		for (obj = room->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (obj->nVirtual == 66236 || obj->nVirtual == 66237 || obj->nVirtual == 66238 || obj->nVirtual == 66239)
				extract_obj (obj);
		}

		//load human win result token
		tobj = load_object (66239);
		obj_to_room(tobj, 66951);
	}
	else //i.e. a troll or warg wins
	{
		room = vtor (66951);
		for (obj = room->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (obj->nVirtual == 66236 || obj->nVirtual == 66237 || obj->nVirtual == 66238 || obj->nVirtual == 66239 )
				extract_obj (obj);
		}

		//load warg-win result token
		tobj = load_object (66237);
		obj_to_room(tobj, 66951);
		sprintf (buf2,
			"The betters mumble and grumble that with the loss of both orcs and humans, all their tokens are worthless.");
		send_to_room (buf2, 66950);
		send_to_room (buf2, 66952);
		send_to_room (buf2, 66951);
	}
}

void
te_pit_third (void)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!(tch = d->character) || d->connected != CON_PLYNG)
			continue;
		if (tch->deleted)
			continue;
		if (IS_NPC (tch))
			continue;
		if (tch->in_room == 66957) //where PCs can enter he pit
			continue;
		if (!IS_SET (tch->flags, FLAG_GUEST))
			continue;

		if ( tch->in_room == 66955 || tch->in_room == 66956 || tch->in_room == 66954 || tch->in_room == 66953 || tch->in_room == 66957)
			continue;
		act
			("\n#2The Tur Edendor Fighting Pit is no longer accepting new entrants.#0",
			false, tch, 0, 0, TO_CHAR);

	}

	te_pit_third_echo = 0;
	sprintf (buf,
		"A warbling, mournful blast comes from a twisted, battered horn strapped to the northern side of the Battle Pit, a chubby goblin huffing and puffing ferociously to announce the start of the battle.");
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	send_to_room (buf, 66955);
	send_to_room (buf, 66956);
	send_to_room (buf, 66954);
	send_to_room (buf, 66953);
	send_to_room (buf, 66957);

	te_pit ();
}

void
te_pit_second (void)
{
	char buf[MAX_STRING_LENGTH];

	te_pit_second_echo = 0;
	sprintf (buf,
		"Shouted and assaulted from all sides by burly orcs bearing the whip-patch of the Slavers Guild, the contestants arrange themselves in two ragged lines facing off one another in the bloodied dirt of the fighting pit.");
	te_pit_third_echo = 61;
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	send_to_room (buf, 66955);
	send_to_room (buf, 66956);
	send_to_room (buf, 66954);
	send_to_room (buf, 66953);
	send_to_room (buf, 66957);
}

void
te_pit_first (void)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch, *keeper, *next_ch;


	if (!vtor (66950) || !vtor (66952) || !vtor (66951) || !vtor (66955) || !vtor (66956) || !vtor (66954) || !vtor (66953) || !vtor (66957))
		return;

	if (te_pit_stopped)
		return;

	//remove the off-duty keeper and replace with ticket seller
	room = vtor (66951);

	for (keeper = room->people; keeper; keeper = next_ch)
	{
		next_ch = keeper->next_in_room;
		if (IS_NPC(keeper))
		{
			if (keeper->mob->nVirtual == 66951)
				extract_char (keeper);
			if (keeper->mob->nVirtual == 66952)
				extract_char (keeper);
		}
	}

	keeper = load_mobile (66950);
	char_to_room (keeper, 66951);

	for (d = descriptor_list; d; d = d->next)
	{
		if (!(tch = d->character) || d->connected != CON_PLYNG)
			continue;
		if (tch->deleted)
			continue;
		if (IS_NPC (tch))
			continue;
		if (IS_SET (tch->flags, FLAG_GUEST))
			continue;

		if (tch->in_room == 66957) //where PC wait for entry
		{
			act
				("\n#2A battle is about to commence in the Tur Edendor Fighting Pit. To join, simply type ENTER PIT; your gear will be stored in another room, and you will be sent there if your survive the fight.#0",
				false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
	} //end for d

	sprintf (buf,
		"The scarred, notched, and weapon-impaled wooden gates slowly swing open, out marching a collection of ragged humans and orcs looking freshly plucked from the streets of Tur Edendor. The cracks of their masters' whips behind them is all but drowned out in the shouts and screams from the crowd that accompany their entrance.\n");
	te_pit_second_echo = 31;
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
	send_to_room (buf, 66955);
	send_to_room (buf, 66956);
	send_to_room (buf, 66954);
	send_to_room (buf, 66953);
	send_to_room (buf, 66957);

	//load spare weapons in a big pile in assorted rooms
	obj_to_room(load_object (1082), 66953);
	obj_to_room(load_object (1082), 66954);
	obj_to_room(load_object (5002), 66953);
	obj_to_room(load_object (5002), 66955);
	obj_to_room(load_object (5045), 66953);
	obj_to_room(load_object (5419), 66956);
	obj_to_room(load_object (5501), 66953);
	obj_to_room(load_object (6005), 66953);
	obj_to_room(load_object (6005), 66956);
	obj_to_room(load_object (6006), 66953);
	obj_to_room(load_object (6006), 66955);
	obj_to_room(load_object (6007), 66953);
	obj_to_room(load_object (6009), 66953);
	obj_to_room(load_object (6011), 66953);
	obj_to_room(load_object (6016), 66953);
	obj_to_room(load_object (6020), 66953);
	obj_to_room(load_object (6020), 66954);
	obj_to_room(load_object (6806), 66954);
	obj_to_room(load_object (6807), 66955);
	obj_to_room(load_object (6810), 66954);
	obj_to_room(load_object (6815), 66953);
}

void
te_equip_slave (CHAR_DATA * tmob)
{
	OBJ_DATA *tobj;
	int weapon;

	for (tobj = tmob->equip; tobj; tobj = tobj->next_content)
	{
		if (tobj == tmob->equip)
			tmob->equip = tmob->equip->next_content;
		else
			tmob->equip->next_content = tobj->next_content;
	}
	tmob->right_hand = NULL;
	tmob->left_hand = NULL;

	if (!IS_NPC(tmob))
	{
		tobj = load_object (6800);
		equip_char (tmob, tobj, WEAR_BODY);
		tobj = load_object (6803);
		equip_char (tmob, tobj, WEAR_LEGS);
		tobj = load_object (6801);
		equip_char (tmob, tobj, WEAR_ARMS);
		tobj = load_object (6805);
		equip_char (tmob, tobj, WEAR_HEAD);
	}
	else //NPCs
	{

		if (tmob->sex == SEX_MALE)
		{
			tobj = load_object (5000);
			equip_char (tmob, tobj, WEAR_LEGS);
		}
		else
		{
			tobj = load_object (5262);
			equip_char (tmob, tobj, WEAR_BODY);
		}
	}

	weapon = number (1, 10);
	if (weapon == 1)
	{
		tobj = load_object (6005);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 2)
	{
		tobj = load_object (6006);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 3)
	{
		tobj = load_object (6007);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 4)
	{
		tobj = load_object (6009);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 5)
	{
		tobj = load_object (6011);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 6)
	{
		tobj = load_object (5002);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 7)
	{
		tobj = load_object (5501);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}
	else if (weapon == 8)
	{
		tobj = load_object (5045);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_PRIM);
	}
	else if (weapon == 9)
	{
		tobj = load_object (1082);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}
	else
	{
		tobj = load_object (6020);
		obj_to_char (tobj, tmob);
		equip_char (tmob, tobj, WEAR_BOTH);
	}

}

void
te_pit (void)
{
	CHAR_DATA *tmob, *keeper, *tch, *next_ch = NULL;
	int room;
	OBJ_DATA *obj, *tobj, *next_obj = NULL;
	ROOM_DATA *room_chk;

	te_pit_fight = true;

	room_chk = vtor(66951);
	//remove old result tokens then repalce with in progress token
	for (obj = room_chk->contents; obj; obj = next_obj)
	{
		if(obj)
		{
			next_obj = obj->next_content;
			if (obj->nVirtual == 66236 || obj->nVirtual == 66237 || obj->nVirtual == 66238 || obj->nVirtual == 66239 )
				extract_obj (obj);
		}
	}
	tobj = load_object (66236);
	obj_to_room(tobj, 66951);

	//remove active shopkeepers and replace with off-duty shopkeep
	room_chk = vtor (66951);

	for (tch = room_chk->people; tch; tch = next_ch)
	{
		next_ch = tch->next_in_room;
		if (IS_NPC(tch))
		{
			if (tch->mob->nVirtual == 66950)
				extract_char (tch);
			if (tch->mob->nVirtual == 66951)
				extract_char (tch);
		}
	}

	keeper = load_mobile (66952);
	char_to_room (keeper, 66951);

	for (; orcs <= te_pit_orc_max_count;)
	{
		tmob = NULL;
		tmob = load_mobile (5998);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		te_equip_slave (tmob);
		tmob->act |= ACT_AGGRESSIVE;
		room = number (1, 4);
		if (room == 1)
			char_to_room (tmob, 66953);
		else if (room == 2)
			char_to_room (tmob, 66955);
		else if (room == 3)
			char_to_room (tmob, 66956);
		else if (room == 4)
			char_to_room (tmob, 66954);
		else
			char_to_room (tmob, 66953);
		orcs++;
	}

	for (; humans <= te_pit_human_max_count;)
	{
		tmob = NULL;
		tmob = load_mobile (5999);
		sprintf (tmob->long_descr, "%s is here.", char_short (tmob));
		*tmob->long_descr = toupper (*tmob->long_descr);
		te_equip_slave (tmob);
		tmob->act |= ACT_AGGRESSIVE;
		room = number (1, 4);
		if (room == 1)
			char_to_room (tmob, 66953);
		else if (room == 2)
			char_to_room (tmob, 66955);
		else if (room == 3)
			char_to_room (tmob, 66956);
		else if (room == 4)
			char_to_room (tmob, 66954);
		else
			char_to_room (tmob, 66953);
		humans++;
	}


}

void
te_pit_engage_echo (CHAR_DATA * ch, CHAR_DATA * vict)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *tch, *ally = NULL;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->fighting == vict && vict->fighting == tch)
		{
			ally = tch;
			break;
		}
	}

	if (ally)
		sprintf (buf, "%s#0 engages #5%s#0, coming to #5%s#0's aid!",
		char_short (ch), char_short (vict), char_short (ally));
	else
		sprintf (buf, "%s#0 engages #5%s#0!", char_short (ch), char_short (vict));

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);

	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);
}

int
is_te_pit_clear (void)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;
	int i = 0;

	for (i = 0; i <= 3; i++)
	{
		room = vtor (te_pit_rooms[i]);
		if (!room)
			continue;
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (!IS_NPC (tch))
				return 0;
	}

	return 1;
}

void
te_pit_cleanup (void)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_DATA *room;
	OBJ_DATA *obj, *next_o;
	CHAR_DATA *tch, *next_c;
	int i;

	humans = 0;
	orcs = 0;

	te_pit_cleanup_echo = 0;
	room = vtor (66957);

	sprintf (buf,
		"The survivors of the battle are herded up by the Slavers before the gates, released from the still fresh scene of blood and carnage. The lucky few professional gladiators are escorted off in another direction.  Following them, a group of dour humans rush in and go to work, dragging away both corpses and injured.");
	for (i = 0; i <= 3; i++)
	{
		send_to_room (buf, te_pit_rooms[i]);
	}

	if (te_pit_troll_check)
	{
		sprintf (buf,
			"The troll bellows furiously as a team of burly Slavers wrestle it into submission and shackle it. After a great deal of struggle they manage to bring it to heel, and lead it back through the gate.");
		for (i = 0; i <= 3; i++)
		{
			send_to_room ("\n", te_pit_rooms[i]);
			send_to_room (buf, te_pit_rooms[i]);
		}
		send_to_room (buf, 66950);
		send_to_room (buf, 66952);
		send_to_room (buf, 66951);
	}

	if (te_pit_warg_check)
	{
		sprintf (buf,
			"A group of Goblin handlers scurries out onto the te_pit floor, deftly chaining the remaining Wargs and leading them back to the kennels.");
		for (i = 0; i <= 3; i++)
		{
			send_to_room ("\n", te_pit_rooms[i]);
			send_to_room (buf, te_pit_rooms[i]);
		}
		send_to_room (buf, 66950);
		send_to_room (buf, 66952);
		send_to_room (buf, 66951);
	}

	for (i = 0; i <= 3; i++)
	{
		room = vtor (te_pit_rooms[i]);
		for (obj = room->contents; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
		if (!room->people)
			continue;
		for (tch = room->people; tch; tch = next_c)
		{
			next_c = tch->next_in_room;
			if (IS_NPC (tch))
				extract_char (tch);
			else
			{
				//strip all gear
				if (tch->right_hand)
				{
					obj = tch->right_hand;
					extract_obj (obj);
				}

				if (tch->left_hand)
				{
					obj = tch->left_hand;
					extract_obj (obj);
				}

				while (tch->equip)
				{
					obj = tch->equip;
					extract_obj (unequip_char (tch, obj->location));
				}
				//move nekkid player to locker room
				char_from_room (tch);
				char_to_room (tch, 66960);
				act
					("\n#2As the other survivors of the battle are herded up by the Slavers before the gates, you are escorted back to the locker room, and your gladiator gear is taken from you. You are told that you will find the prize for surving the battle in your bag.#0",
					false, tch, 0, 0, TO_CHAR | _ACT_FORMAT);
			}

		}
	}

	te_pit_last_echo = 31;
}


void
te_pit_death (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	if (ch->delay_ch)
	{
		sprintf (buf, "%s#0 falls to the ground, whether dead or merely critically injured by #5%s#0's hand, it matters not.",
			char_short (ch), char_short (ch->delay_ch));
		ch->delay_ch = NULL;
	}
	else if (!ch->delay_ch)
	{
		sprintf (buf, "%s#0 falls to the ground, whether dead or merely critically injured it matters not.", char_short (ch));
	}

	*buf = toupper (*buf);
	sprintf (buf2, "#5%s", buf);
	sprintf (buf, "%s", buf2);
	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);



	if (ch->race == 0 || ch->race == 1 || ch->race == 101 || ch->race == 3|| ch->race == 4 || ch->race == 6 || ch->race == 9)
	{
		humans--;
	}
	else if (ch->race == 27 || ch->race == 119 || ch->race == 120 || ch->race == 121)
	{
		orcs--;
	}

	if (ch->race == 28 || ch->race == 86)
	{
		te_pit_troll_check = false;
	}

	if (ch->race == 89)
	{
		wargs--;
		if (wargs <= 0)
		{
			te_pit_warg_check = false;
		}
	}

	if (orcs <= 0 && !te_pit_cleanup_echo && te_pit_fight)
	{
		send_to_room ("\n", 66950);
		send_to_room ("\n", 66952);
		send_to_room ("\n", 66951);
		sprintf (buf,
			"The human segments of the crowd shout in victory as they emerge victorious!\n");
		send_to_room (buf, 66950);
		send_to_room (buf, 66952);
		send_to_room (buf, 66951);
		send_to_room (buf, 66955);
		send_to_room (buf, 66956);
		send_to_room (buf, 66954);
		send_to_room (buf, 66953);
		send_to_room (buf, 66960);
		te_pit_cleanup_echo = 21;

		orcswin = false;
		humanswin = true;
	}

	if (humans <= 0 && !te_pit_cleanup_echo && te_pit_fight)
	{
		send_to_room ("\n", 66950);
		send_to_room ("\n", 66952);
		send_to_room ("\n", 66951);
		sprintf (buf,
			"The orcs, goblins and half-breeds in the stall gibber and cackle as the final human collapses beneath their kins' blades down in the dirt pit!\n");
		send_to_room (buf, 66950);
		send_to_room (buf, 66952);
		send_to_room (buf, 66951);
		send_to_room (buf, 66955);
		send_to_room (buf, 66956);
		send_to_room (buf, 66954);
		send_to_room (buf, 66953);
		send_to_room (buf, 66960);
		te_pit_cleanup_echo = 21;

		orcswin = true;
		humanswin = false;
	}

	if (ch->race == 28 && te_pit_troll_check)
	{
		te_pit_troll_check = false;
	}

}

void
te_pit_combat_message (CHAR_DATA * src, CHAR_DATA * tar, char *location,
					   int damage, int room)
{
	char area[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char impact[MAX_STRING_LENGTH];

	if (damage == 0)
		return;

	if (damage < 2)
		return;

	if (room == 66953)
		sprintf (area, "northwestern");
	else if (room == 66954)
		sprintf (area, "northeastern");
	else if (room == 66956)
		sprintf (area, "southeastern");
	else if (room == 66955)
		sprintf (area, "southwestern");

	// Don't want -too- much. Display heavy-end mods is enough.

	if (damage > 10 && damage <= 15)
		sprintf (impact, "a heavy blow to");
	else if (damage > 15 && damage <= 20)
		sprintf (impact, "a severe blow to");
	else if (damage > 20 && damage <= 25)
		sprintf (impact, "grievously hard on");
	else if (damage > 25 )
		sprintf (impact, "a devastating blow to");
	else return;

	sprintf (buf, "In the %s part of the pit, #5%s#0", area,
		char_short (src));
	sprintf (buf + strlen (buf), " strikes #5%s#0 %s the %s.", char_short (tar),
		impact, expand_wound_loc (location));

	send_to_room (buf, 66950);
	send_to_room (buf, 66952);
	send_to_room (buf, 66951);

}


void
te_pit_update_delays (void)
{
	if (te_pit_last_echo > 1)
	{
		te_pit_last_echo--;
		if (te_pit_last_echo == 1)
			te_pit_last ();
	}

	if (te_pit_cleanup_echo > 1)
	{
		te_pit_cleanup_echo--;
		if (te_pit_cleanup_echo == 1)
			te_pit_cleanup ();
	}

	if (te_pit_second_echo > 1)
	{
		te_pit_second_echo--;
		if (te_pit_second_echo == 1)
			te_pit_second ();
	}

	if (te_pit_third_echo > 1)
	{
		te_pit_third_echo--;
		if (te_pit_third_echo == 1)
			te_pit_third ();
	}
}

void
te_pit_do_shout (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	int i = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };


	for (i = 0; i <= 3; i++)
	{
		for (tch = vtor (te_pit_rooms[i])->people; tch; tch = tch->next_in_room)
		{
			if (tch == ch)
				continue;
			if (GET_SEX (ch) == SEX_MALE)
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a male voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a male voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
				{
					sprintf (buf,
						"You hear a male voice shout from the stands,");
				}
			}
			else if (GET_SEX (ch) == SEX_FEMALE)
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a female voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a female voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
					sprintf (buf,
					"You hear a female voice shout from the stands,");
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (!tch->skills[ch->speaks])
						sprintf (buf,
						"You hear a voice shout in an unknown tongue from the stands,");
					else
						sprintf (buf,
						"You hear a voice shout in %s from the stands,",
						skills[ch->speaks]);
				}
				else
					sprintf (buf, "You hear a voice shout from the stands,");
			}
			if (tch->skills[ch->speaks] >= 50 && ch->skills[ch->speaks] < 50)
				sprintf (buf + strlen (buf), " %s,",
				accent_desc (ch, ch->skills[ch->speaks]));
			if (!decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
			{
				sprintf (buf + strlen (buf),
					" though you cannot decipher the words.");
				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
			}
			else if (tch->desc)
			{
				act (buf, false, tch, 0, ch, TO_CHAR | _ACT_FORMAT);
				if (decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]))
				{
					if (!IS_SET (ch->room->room_flags, OOC))
						sprintf (buf, "   \"%s\"\n", buf4);
					else
						sprintf (buf, "   \"%s\"\n", buf4);
					send_to_char (buf, tch);
				}
			}

			if (GET_TRUST (tch) && !IS_NPC (tch)
				&& GET_FLAG (tch, FLAG_SEE_NAME))
				sprintf (buf3, " (%s)", GET_NAME (ch));
			else
				*buf3 = '\0';
			sprintf (buf2, argument);    /* Reset. */
		}
	}

}

void
te_pit_do_look (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	ROOM_DATA *troom = NULL;
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, arg1);

	if (!*arg1)
	{
		troom = vtor (66953);
		*buf = '\0';
		send_to_char
			("You gaze out over the blood-hued dirt of the pit below, seeing...\n",
			ch);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the northwestern part of the pit:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (66954);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the northeastern part of the pit:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (66956);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the southeastern part of the pit:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}
		troom = vtor (66955);
		if (troom && troom->people)
		{
			sprintf (buf, "#6\nIn the southwestern part of the pit:#0\n");
			send_to_char (buf, ch);
			list_char_to_char (troom->people, ch, 0);
		}



		if (!*buf)
			send_to_char
			("\n...nothing of interest. There is no fight currently in progress.\n",
			ch);
		else
		{
			sprintf (buf, "\n#3 A sign hangs on the bloody wall showing marks in two columns.#0\n #1Fang:#0 %d #6Tree:#0 %d\n", orcs, humans);
			send_to_char (buf, ch);
		}
		return;
	}
	else
	{
		tch = get_char_room (arg1, 66953);
		if (!tch)
			tch = get_char_room (arg1, 66955);
		if (!tch)
			tch = get_char_room (arg1, 66954);
		if (!tch)
			tch = get_char_room (arg1, 66956);
		if (!tch)
		{
			send_to_char ("You do not see that in the pit.\n", ch);
			return;
		}

		show_char_to_char (tch, ch, (cmd == 2) ? 15 : 1);
		return;
	}
}


void
te_pit_do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *bag;
	OBJ_DATA *obj;

	if (!te_pit_second_echo && !te_pit_third_echo)
	{
		send_to_char
			("The Tur Edendor Battle Pit is not accepting new entrants at the moment.\n",
			ch);
		return;
	}


	if (ch->in_room == 66955 || ch->in_room == 66956 || ch->in_room == 66954 || ch->in_room == 66953)
	{
		send_to_char ("You're already in the pit!\n", ch);
		return;
	}


	if ((!vtom (5999) || !vtom (5997) || !vtom (5998)) || (!vtor (66955) || !vtor (66956) || !vtor (66954) || !vtor (66953)))
	{
		send_to_char ("The necessary prototypes cannot be found. Aborting.\n",
			ch);
		return;
	}


	// number of entrants checked and clans assigned
	if (ch->race == 0 ||
		ch->race == 1 ||
		ch->race == 101 ||
		ch->race == 3 ||
		ch->race == 4 ||
		ch->race == 6 ||
		ch->race == 9)
	{
		if (te_pit_human_max_count > 0)
		{
			send_to_char ("#6\nYou have been placed on the Human team!#0\n", ch);
			add_clan (ch, "humanslaves", CLAN_MEMBER);
			te_pit_human_max_count--;
			humans++;
		}
		else
		{
			send_to_char ("#6\nThere is not enough room on the Human team!#0\n", ch);
			return;
		}
	}
	else if (ch->race == 27 || ch->race == 119 || ch->race == 120 || ch->race == 121)
	{
		if (te_pit_orc_max_count > 0)
		{
			add_clan (ch, "orcslaves", CLAN_MEMBER);
			send_to_char ("#1\nYou have been placed on the Orcish team!#0\n", ch);
			te_pit_orc_max_count--;
			orcs++;
		}
		else
		{
			send_to_char ("#6\nThere is not enough room on the Orcish  team!#0\n", ch);
			return;
		}
	}
	act
		("$n joins the crowds waiting to enter through the battered gate.",
		true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	send_to_char
		("#2\nConverting you to a more proper dress in preparation for the fight. Be sure to pick up your gear if you survive.\n",
		ch);

	/**** pit bags **/
	bag = load_object (VNUM_PITBAG);

	if (bag && (ch->right_hand || ch->left_hand || ch->equip))
	{
		sprintf (buf, "A gladiator's bag, labeled '%s %s' sits here.",
			PITBAG_DESC_PREPEND, ch->short_descr);
		bag->description = duplicateString (buf);

		sprintf (buf, "a bag labeled '%s %s'", PITBAG_DESC_PREPEND,
			ch->short_descr);
		bag->short_description = duplicateString (buf);


		if (ch->right_hand)
		{
			obj = ch->right_hand;
			obj_from_char (&obj, 0);
			if (bag)
				obj_to_obj (obj, bag);
		}

		if (ch->left_hand)
		{
			obj = ch->left_hand;
			obj_from_char (&obj, 0);
			if (bag)
				obj_to_obj (obj, bag);
		}

		while (ch->equip)
		{
			obj = ch->equip;
			if (bag)
				obj_to_obj (unequip_char (ch, obj->location), bag);
		}
		obj = load_object (66262);
		obj_to_obj (obj, bag); //winners token is added.
		obj_to_room (bag, 66960);
	}


	send_to_char ("#2\nThe battle will begin in just a moment. Good luck!#0\n",
		ch);

	te_equip_slave (ch);
	char_from_room (ch);
	char_to_room (ch, 66953);
	sprintf (buf, "%s %s", ch->name, ch->tname);
	free_mem (ch->name);
	ch->name = duplicateString (buf);
	return;
}
