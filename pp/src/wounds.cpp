/*------------------------------------------------------------------------\
|  wounds.c : Wounds Module                           www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sstream>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

#define WOUND_INFECTIONS 1	// Set to 1 to enable wound infection; 0 to disable.

int
general_damage (CHAR_DATA * ch, int amount)
{
	return wound_to_char (ch, "bloodloss", amount, 0, 0, 0, 0);
}

void
apply_con_penalties (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;

	if (IS_NPC (ch))
		return;

	if ((af = get_affect (ch, AFFECT_LOST_CON)))
	{
		af->a.spell.sn += 2;
		af->a.spell.duration += 16;
	}
	else
		magic_add_affect (ch, AFFECT_LOST_CON, 16, 0, 0, 0, 2);

	ch->con -= 2;
	ch->tmp_con -= 2;
}


// returns 1 if character died, else 0
int
wound_to_char (CHAR_DATA * ch, char *location, int impact, int type,
			   int bleeding, int poison, int infection)
{
	WOUND_DATA *wound;
	char *p;
	int curdamage = 0, stun = 0, difficulty_rating = 0;
	float limit = 0;
	char buf[MAX_STRING_LENGTH], name[MAX_STRING_LENGTH],
		severity[MAX_STRING_LENGTH];

	if (impact <= 0)
		return 0;

	if (ch->wounds)
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
			{
				stun += wound->damage;
				curdamage += (wound->damage / 2);
			}
			else
				curdamage += wound->damage;
		}

		curdamage += ch->damage;

		limit = ch->max_hit;

		if (impact > 0 && impact <= (limit * .02))
		{
			sprintf (severity, "small");
			difficulty_rating = 10;
		}
		else if (impact > (limit * .02) && impact <= (limit * .10))
		{
			sprintf (severity, "minor");
			difficulty_rating = 12;
		}
		else if (impact > (limit * .10) && impact <= (limit * .20))
		{
			sprintf (severity, "moderate");
			difficulty_rating = 15;
		}
		else if (impact > (limit * .20) && impact <= (limit * .30))
		{
			sprintf (severity, "severe");
			difficulty_rating = 16;
		}
		else if (impact > (limit * .30) && impact <= (limit * .40))
		{
			sprintf (severity, "grievous");
			difficulty_rating = 17;
		}
		else if (impact > (limit * .40) && impact <= (limit * .50))
		{
			sprintf (severity, "terrible");
			difficulty_rating = 18;
		}
		else if (impact > (limit * .50))
		{
			sprintf (severity, "mortal");
			difficulty_rating = 20;
		}

		if (type == 2 || type == 4)
			switch (number (1, 6))
		{
			case 1:
				sprintf (name, "slice");
				break;
			case 2:
				sprintf (name, "cut");
				break;
			case 3:
				sprintf (name, "gash");
				break;
			case 4:
				sprintf (name, "slash");
				break;
			case 5:
				sprintf (name, "nick");
				break;
			case 6:
				sprintf (name, "laceration");
				break;
		}

		else if (type == 0 || type == 1)
			switch (number (1, 5))
		{
			case 1:
				sprintf (name, "puncture");
				break;
			case 2:
				sprintf (name, "piercing");
				break;
			case 3:
				sprintf (name, "stab");
				break;
			case 4:
				sprintf (name, "hole");
				break;
			case 5:
				sprintf (name, "perforation");
				break;
		}

		else if (type == 3)
			switch (number (1, 5))
		{
			case 1:
				sprintf (name, "bruise");
				break;
			case 2:
				sprintf (name, "bruise");
				break;
			case 3:
				sprintf (name, "contusion");
				break;
			case 4:
				sprintf (name, "contusion");
				break;
			case 5:
				sprintf (name, "crush");
				break;
		}

		else if (type == 5)
			switch (number (1, 7))
		{
			case 1:
				sprintf (name, "frostburn");
				break;
			case 2:
				sprintf (name, "discoloration");
				break;
			case 3:
				sprintf (name, "frostnip");
				break;
			case 4:
				sprintf (name, "waxy-frostbite");
				break;
			case 5:
				sprintf (name, "white-frostbite");
				break;
			case 6:
				sprintf (name, "gray-frostbite");
				break;
			case 7:
				sprintf (name, "black-frostbite");
				break;
		}

		else if (type == 6)
			switch (number (1, 7))
		{
			case 1:
				sprintf (name, "burn");
				break;
			case 2:
				sprintf (name, "sear");
				break;
			case 3:
				sprintf (name, "singe");
				break;
			case 4:
				sprintf (name, "char");
				break;
			case 5:
				sprintf (name, "blistering");
				break;
			case 6:
				sprintf (name, "scorch");
				break;
			case 7:
				sprintf (name, "scald");
				break;
		}

		else if (type == 7)		// Natural attacks -- teeth.
			switch (number (1, 5))
		{
			case 1:
				sprintf (name, "bite");
				break;
			case 2:
				sprintf (name, "tooth-puncture");
				break;
			case 3:
				sprintf (name, "incision");
				break;
			case 4:
				sprintf (name, "notch");
				break;
			case 5:
				sprintf (name, "gore");
				break;
		}

		else if (type == 8)		// Natural attacks -- claws.
			switch (number (1, 5))
		{
			case 1:
				sprintf (name, "claw-gash");
				break;
			case 2:
				sprintf (name, "rent");
				break;
			case 3:
				sprintf (name, "tear");
				break;
			case 4:
				sprintf (name, "rip");
				break;
			case 5:
				sprintf (name, "gouge");
				break;
		}

		else if (type == 9)  // Natural attacks -- fist.
		{
			if (number(0, 3))
				type = 10;
			switch (number (1, 3))
			{
			case 1:
				sprintf (name, "bruise");
				break;
			case 2:
				sprintf (name, "abrasion");
				break;
			case 3:
				sprintf (name, (type == 10) ? ("mark") : ("contusion"));
				break;
			}
		}
		else if (type == 10) { // stun etc
			sprintf(name, "numbness");
		}
		else {
			sprintf(name, "ERROR");
		}

		if (str_cmp (location, "bloodloss"))
		{
			if (!ch->wounds)
			{
				ch->wounds = new WOUND_DATA;
				wound = ch->wounds;
				wound->next = NULL;
			}

			else
			{
				for (wound = ch->wounds; wound; wound = wound->next)
				{
					if (!wound->next)
					{
						wound->next = new WOUND_DATA;
						wound = wound->next;
						wound->next = NULL;
						break;
					}
				}
			}

			wound->location = duplicateString (location);
			wound->damage = impact;

			if (type == 2 || type == 4)
				wound->type = duplicateString ("slash");
			else if (type == 0 || type == 1)
				wound->type = duplicateString ("pierce");
			else if (type == 3)
				wound->type = duplicateString ("blunt");
			else if (type == 5)
				wound->type = duplicateString ("frost");
			else if (type == 6)
				wound->type = duplicateString ("fire");
			else if (type == 7)
				wound->type = duplicateString ("bite");
			else if (type == 8)
				wound->type = duplicateString ("claw");
			else if (type == 9)
				wound->type = duplicateString ("fist");
			else if (type == 10)
				wound->type = duplicateString("stun");
			else if (!str_cmp(location, "bloodloss"))
				wound->type = duplicateString("bloodloss");
			else
				wound->type = duplicateString("bloodloss");

			wound->name = duplicateString (name);
			wound->severity = duplicateString (severity);

			if (!str_cmp (severity, "severe") && !bleeding && str_cmp(wound->type, "stun"))
				wound->bleeding = number (2, 3);
			else if (!str_cmp (severity, "grievous") && !bleeding && str_cmp(wound->type, "stun"))
				wound->bleeding = number (3, 5);
			else if (!str_cmp (severity, "terrible") && !bleeding && str_cmp(wound->type, "stun"))
				wound->bleeding = number (5, 10);
			else if (!str_cmp (severity, "mortal") && !bleeding && str_cmp(wound->type, "stun"))
				wound->bleeding = number (10, 20);
			else if (bleeding)
				wound->bleeding = bleeding;
			if (IS_SET (ch->act, ACT_NOBLEED))
				wound->bleeding = 0;

			wound->poison = poison;
			wound->infection = infection;
			wound->healerskill = 0;
			wound->lasthealed = time (0);
			wound->lastbled = time (0);
			wound->next = NULL;

			if (IS_MORTAL (ch) && wound->bleeding)
			{
				sprintf (buf,
					"#1You grimace as you feel blood begin to flow from a %s %s on your %s.#0\n",
					wound->severity, wound->name,
					expand_wound_loc (wound->location));
				reformat_string (buf, &p);
				send_to_char ("\n", ch);
				send_to_char (p, ch);
				free_mem (p);
			}
		} // if bloodloss
		else
			ch->damage += impact;

		int old_damage = curdamage;
		if (type != 10)
			curdamage += impact;
		else
			curdamage += (impact / 2);

		if (IS_NPC (ch) && ch->mob->cues)
		{
			// hook: cue_on_health reflex
			// We only want ones that have the flag (<NN)
			typedef std::multimap<mob_cue,std::string>::const_iterator N;
			std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_health);
			if (range.first != range.second)
			{
				// Want to only execute the health cue the first time health
				// drops below NN
				int old_health = 100 - (int((old_damage * 100.0) / (ch->max_hit)));
				int new_health = 100 - (int((curdamage * 100.0) / (ch->max_hit)));

				for (N n = range.first; n != range.second; n++)
				{
					std::string cue = n->second;
					const char *r = cue.c_str();

					if (!cue.empty () && strncmp(r,"(<",2) == 0)
					{
						char *p;
						int threshold = strtol (r+2, &p, 0);
						if (new_health < threshold && old_health >= threshold)
						{
							char reflex[AVG_STRING_LENGTH] = "";
							strcpy (reflex, p+2);
							command_interpreter (ch, reflex);
						}
					}
				} // for N n
			} //if (range.first != range.second)
		} //if (IS_NPC (ch) && ch->mob->cues)


		if ((curdamage > ch->max_hit) && (IS_MORTAL (ch) || IS_NPC (ch)))
		{
			if (ch->room && !IS_SET (ch->flags, FLAG_COMPETE))
				die (ch);
			return 1;
		}

		if (ch->race == 64)
			return 0;

		if (((curdamage > ch->max_hit * .85) || (stun > (ch->max_hit * (0.5+GET_WIL(ch)*0.01)) && (wound->type && *(wound->type) && !strncmp(wound->type, "stun", 4)))) && GET_POS (ch) != POSITION_UNCONSCIOUS
			&& (IS_MORTAL (ch) || IS_NPC (ch)))
		{
			GET_POS (ch) = POSITION_UNCONSCIOUS;
			ch->knockedout = time (0);
			if (IS_SUBDUER (ch))
				release_prisoner (ch, NULL);
			do_drop (ch, "all", 0);
			send_to_char
				("\n#1Overcome at last, you slip into a deep unconsciousness. . .#0\n",
				ch);
			sprintf (buf, "$n has been rendered unconscious.");
			act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
			sprintf (buf, "%s is mortally wounded and unconscious! [%d]\n",
				ch->tname, ch->in_room);
			clear_moves (ch);
			clear_current_move (ch);
			if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST))
				send_to_gods (buf);
			apply_con_penalties (ch);
			if (ch->con <= 3)
			{
				if (ch->room && !IS_SET (ch->flags, FLAG_COMPETE))
					die (ch);
				return 1;
			}
		}

		return 0;
}

void
free_lodged (LODGED_OBJECT_INFO * lodged)
{
	if (!lodged)
		return;

	if (lodged->location && strlen (lodged->location) > 1)
		free_mem (lodged->location);

	free_mem (lodged);
}

void
free_wound (WOUND_DATA * wound)
{
	if (!wound)
		return;

	if (wound->location && *wound->location)
		free_mem (wound->location);

	if (wound->type && *wound->type)
		free_mem (wound->type);

	if (wound->name && *wound->name)
		free_mem (wound->name);

	if (wound->severity && *wound->severity)
		free_mem (wound->severity);

	free_mem (wound);
}

void
heal_all_wounds (CHAR_DATA * ch)
{
	while (ch->wounds)
		wound_from_char (ch, ch->wounds);

	while (ch->lodged)
		lodge_from_char (ch, ch->lodged);

	ch->damage = 0;

	GET_POS (ch) = POSITION_STANDING;

}

void
lodge_from_char (CHAR_DATA * ch, LODGED_OBJECT_INFO * lodged)
{
	LODGED_OBJECT_INFO *templodged;

	if (!ch || !lodged)
		return;

	if (ch->lodged == lodged)
		ch->lodged = ch->lodged->next;

	else
	{
		for (templodged = ch->lodged; templodged; templodged = templodged->next)
			if (templodged->next == lodged)
				templodged->next = templodged->next->next;
	}

	free_lodged (lodged);
}

void
lodge_from_obj (OBJ_DATA * obj, LODGED_OBJECT_INFO * lodged)
{
	LODGED_OBJECT_INFO *templodged;

	if (!obj || !lodged)
		return;

	if (obj->lodged == lodged)
		obj->lodged = obj->lodged->next;

	else
	{
		for (templodged = obj->lodged; templodged;
			templodged = templodged->next)
			if (templodged->next == lodged)
				templodged->next = templodged->next->next;
	}

	free_lodged (lodged);
}

void
wound_from_char (CHAR_DATA * ch, WOUND_DATA * wound)
{
	WOUND_DATA *tempwound;

	if (!ch || !wound)
		return;

	if (ch->wounds == wound)
		ch->wounds = ch->wounds->next;

	else
	{
		for (tempwound = ch->wounds; tempwound; tempwound = tempwound->next)
			if (tempwound->next == wound)
				tempwound->next = tempwound->next->next;
	}

	free_wound (wound);
}

void
wound_from_obj (OBJ_DATA * obj, WOUND_DATA * wound)
{
	WOUND_DATA *tempwound;

	if (!obj || !wound)
		return;

	if (obj->wounds == wound)
		obj->wounds = obj->wounds->next;

	else
	{
		for (tempwound = obj->wounds; tempwound; tempwound = tempwound->next)
			if (tempwound->next == wound)
				tempwound->next = tempwound->next->next;
	}

	free_wound (wound);
}

int
is_proper_kit (OBJ_DATA * kit, WOUND_DATA * wound)
{
	if (!kit->o.od.value[5] || IS_SET (kit->o.od.value[5], TREAT_ALL))
		return 1;

	if (!str_cmp (wound->name, "slice") || !str_cmp (wound->name, "cut")
		|| !str_cmp (wound->name, "gash") || !str_cmp (wound->name, "slash")
		|| !str_cmp (wound->name, "nick")
		|| !str_cmp (wound->name, "laceration"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_SLASH))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "claw-gash") || !str_cmp (wound->name, "rent")
		|| !str_cmp (wound->name, "tear") || !str_cmp (wound->name, "rip")
		|| !str_cmp (wound->name, "gouge"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_SLASH))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "puncture") || !str_cmp (wound->name, "piercing")
		|| !str_cmp (wound->name, "stab") || !str_cmp (wound->name, "hole")
		|| !str_cmp (wound->name, "perforation"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_PUNCTURE))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "bite")
		|| !str_cmp (wound->name, "tooth-puncture")
		|| !str_cmp (wound->name, "incision") || !str_cmp (wound->name, "gore")
		|| !str_cmp (wound->name, "notch"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_PUNCTURE))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "bruise") || !str_cmp (wound->name, "abrasion")
		|| !str_cmp (wound->name, "contusion")
		|| !str_cmp (wound->name, "crush"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_BLUNT))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "frostburn")
		|| !str_cmp (wound->name, "discoloration")
		|| !str_cmp (wound->name, "frostnip")
		|| !str_cmp (wound->name, "waxy-frostbite")
		|| !str_cmp (wound->name, "white-frostbite")
		|| !str_cmp (wound->name, "gray-frostbite")
		|| !str_cmp (wound->name, "black-frostbite"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_FROST))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->name, "burn") || !str_cmp (wound->name, "sear")
		|| !str_cmp (wound->name, "singe") || !str_cmp (wound->name, "char")
		|| !str_cmp (wound->name, "blistering")
		|| !str_cmp (wound->name, "scorch") || !str_cmp (wound->name, "scald"))
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_FROST))
			return 0;
		else
			return 1;
	}

	if (!str_cmp (wound->location, "bloodloss") )
	{
		if (!IS_SET (kit->o.od.value[5], TREAT_BLEED))
			return 0;
		else
			return 1;
	}
	return 0;
}

void
begin_treatment (CHAR_DATA * ch, CHAR_DATA * tch, char *location, int mode)
{
	WOUND_DATA *wound;
	OBJ_DATA *kit;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT)
		kit = ch->right_hand;
	else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT)
		kit = ch->left_hand;
	else
	{
		sprintf (buf,
			"Having discarded your healer's kit, you cease tending $N.");
		sprintf (buf,
			"Having discarded $s healer's kit, $n ceases $s ministrations.");
		act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
		act (buf2, false, ch, 0, tch, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (kit->o.od.value[0] <= 0)
	{
		send_to_char
			("That healer's kit no longer contains any useful materials.\n", ch);
		return;
	}

	if (kit->o.od.value[2] && kit->o.od.value[2] > ch->skills[SKILL_HEALING])
	{
		send_to_char
			("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if (tch->in_room != ch->in_room)
	{
		send_to_char ("You cannot treat someone who isn't here!\n", ch);
		return;
	}

	for (wound = tch->wounds; wound; wound = wound->next)
	{
		/* if you have >71 healing do not allow constant attempts to re-treat on expertly treated wounds*/
		int skill = ch->skills[SKILL_HEALING];
		skill = (skill > 71 ? 71 : skill);

		if (!str_cmp (wound->location, location)
			&& (wound->healerskill  < skill
			&& wound->healerskill != -1))
		{
			if (!str_cmp (wound->severity, "small")
				|| !str_cmp (wound->severity, "minor"))
			{
				sprintf (buf,
					"The %s %s is too minor to benefit from medical attention.",
					wound->severity, wound->name);
				act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				continue;
			}
			if (!is_proper_kit (kit, wound))
			{
				sprintf (buf, "#2%s#0 is of no assistance with the %s %s.",
					obj_short_desc (kit), wound->severity, wound->name);
				buf[2] = toupper (buf[2]);
				act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				continue;
			}
			if (mode)
			{
				sprintf (buf,
					"You turn your attention to the %s %s on your %s.",
					wound->severity, wound->name,
					expand_wound_loc (wound->location));
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				ch->delay_who = duplicateString (location);
				ch->delay_ch = tch;
				ch->delay_type = DEL_TREAT_WOUND;
				ch->delay = wound->damage - ch->skills[SKILL_HEALING] / 10;
				ch->delay = MAX (ch->delay, 2);
				return;
			}
			else
			{
				sprintf (buf,
					"You turn your attention to the %s %s on $N's %s.",
					wound->severity, wound->name,
					expand_wound_loc (wound->location));
				sprintf (buf2, "$n turns $s attention to the %s %s on your %s.",
					wound->severity, wound->name,
					expand_wound_loc (wound->location));
				act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
				ch->delay_who = duplicateString (location);
				ch->delay_ch = tch;
				ch->delay_type = DEL_TREAT_WOUND;
				ch->delay = wound->damage - ch->skills[SKILL_HEALING] / 10;
				ch->delay = MAX (ch->delay, 2);
				return;
			}
		}
	}

	sprintf (buf,
		"No other wounds on that area can benefit from your attention.");
	act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);

	return;
}

int
adjust_wound (CHAR_DATA * ch, WOUND_DATA * wound, int amount)
{
	WOUND_DATA *twound;
	char buf[MAX_STRING_LENGTH];
	int curdamage = 0;

	if (!ch || !wound || !amount)
		return 0;

	wound->damage += amount;

	if (wound->damage <= 0)
	{
		wound_from_char (ch, wound);
		return 0;
	}

	sprintf (buf, "%s", downsized_wound (ch, wound));
	free_mem (wound->severity);
	wound->severity = NULL;
	wound->severity = duplicateString (buf);

	if (amount < 0)
		return 0;

	for (twound = ch->wounds; twound; twound = twound->next)
	{
		if (str_cmp(twound->type, "stun"))
			curdamage += twound->damage;
		else
			curdamage += (twound->damage / 2);
	}

	curdamage += ch->damage;

	if ((curdamage > ch->max_hit) && (IS_MORTAL (ch) || IS_NPC (ch)))
	{
		send_to_char
			("\n#1The last thing you perceive in this world is the cry of\n"
			"agony that passes from your lips, borne upon the wings of\n"
			"your last, labored breath -- may your soul find peace...#0\n\n",
			ch);
		if (ch->room && !IS_SET (ch->flags, FLAG_COMPETE))
			die (ch);
		return 1;
	}

	if ((curdamage > ch->max_hit * .85) && GET_POS (ch) != POSITION_UNCONSCIOUS
		&& (IS_MORTAL (ch) || IS_NPC (ch)))
	{
		GET_POS (ch) = POSITION_UNCONSCIOUS;
		ch->knockedout = time (0);
		if (IS_SUBDUER (ch))
			release_prisoner (ch, NULL);
		do_drop (ch, "all", 0);
		send_to_char
			("\n#1Finally overcome with the tremendous pain of your wounds,\n"
			"you collapse to the ground, unable to fight the inevitable\n"
			"any longer. Your vision fades away into blackness...#0\n\n", ch);
		sprintf (buf, "$n collapses, rendered unconscious by the pain.");
		act (buf, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		sprintf (buf, "%s is mortally wounded! [%d]\n", ch->tname, ch->in_room);
		clear_moves (ch);
		clear_current_move (ch);
		if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST))
			send_to_gods (buf);
	}

	return 0;
}

void
delayed_treatment (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	WOUND_DATA *wound, *next_wound;
	OBJ_DATA *kit;
	int mode = 0;
	int roll = 0;
	int treat_effect = 0;
	char *location;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	tch = ch->delay_ch;

	if (tch == ch)
		mode = 1;

	location = ch->delay_who;

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT)
		kit = ch->right_hand;
	else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT)
		kit = ch->left_hand;
	else
	{
		sprintf (buf,
			"Having discarded your healer's kit, you cease tending $N.");
		sprintf (buf,
			"Having discarded $s healer's kit, $n ceases $s ministrations.");
		act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
		act (buf2, false, ch, 0, tch, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (kit->o.od.value[0] <= 0)
	{
		send_to_char
			("That healer's kit no longer contains any useful materials.\n", ch);
		return;
	}

	if (kit->o.od.value[2] && kit->o.od.value[2] > ch->skills[SKILL_HEALING])
	{
		send_to_char
			("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if (ch->right_hand && ch->left_hand)
	{
		send_to_char ("You'll need one hand free to treat someone.\n", ch);
		return;
	}

	if (tch->in_room != ch->in_room)
	{
		send_to_char ("You cannot treat someone who isn't here!\n", ch);
		return;
	}

	/**
	A roll is made against the skill level of the PC to determine how good or bad they are.

	A second skill_use check is then made to give the PC a chance to increase thier skill, although the result has no bearing on the code below.
	**/
	int healerSkillLevel = skill_level(ch, SKILL_HEALING, 0);
	roll = number (1, MAX(90, healerSkillLevel - 10));
	if (roll <= skill_level (ch, SKILL_HEALING, 0) - 15)
	{
		treat_effect =  3;
	}
	else if (roll <= skill_level (ch, SKILL_HEALING, 0) - 0 || (healerSkillLevel > 70))
	{
		treat_effect =  2;
	}
	else if ((roll <= skill_level (ch, SKILL_HEALING, 0) + 10) || (healerSkillLevel > 50))
	{
		treat_effect =  1;
	}
	else
	{
		treat_effect = 0;
	}

	skill_use (ch, SKILL_HEALING, 0);

	for (wound = tch->wounds; wound; wound = next_wound)
	{
		next_wound = wound->next;
		/* if you have >71 healing do not allow constant attempts to re-treat on expertly treated wounds*/
		int skill = ch->skills[SKILL_HEALING];
		skill = (skill > 71 ? 71 : skill);
		if (!str_cmp (wound->location, location)
			&& (wound->healerskill < skill
			&& wound->healerskill != -1)
			&&
			((str_cmp (wound->severity, "minor")
			&& str_cmp (wound->severity, "small")) || wound->infection))
		{
			/***
			Really good healers can make any kit work better, and can heal more points than a normal healer.
			**/
			if (treat_effect == 3)
			{
				if (mode)
				{
					//wound->healerskill = ch->skills[SKILL_HEALING];
					//if (kit->o.od.value[1])
					//	wound->healerskill += kit->o.od.value[1]*2;
					/** To show that this is an expertly treated wound **/
					wound->healerskill = 71;
					if (wound->infection)
						wound->infection = -1;
					if (kit->o.od.value[3])
					{
						sprintf (buf,
							"You treat the wound expertly, making it look considerably better.");
						adjust_wound (tch, wound, kit->o.od.value[3] * -2);

					}
					else {
						sprintf (buf,
							"You treat and dress the wound expertly.");
						adjust_wound (tch, wound, -3);
					}
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					//wound->healerskill = ch->skills[SKILL_HEALING];
					//if (kit->o.od.value[1])
					//	wound->healerskill += kit->o.od.value[1]*2;
					/** To show that this is an expertly treated wound **/
					wound->healerskill = 71;
					if (wound->infection)
						wound->infection = -1;
					if (kit->o.od.value[3])
					{
						sprintf (buf,
							"You treat the wound expertly, making it look considerably better.");
						sprintf (buf2,
							"$n treats the wound expertly, making it look considerably better.");
						adjust_wound (tch, wound, kit->o.od.value[3] * -2);
					}
					else
					{
						sprintf (buf,
							"You treat and dress the wound expertly.");
						sprintf (buf2,
							"$n treats and dresses the wound expertly.");
						adjust_wound (tch, wound, -2);
						/** To show that this is an expertly treated wound **/
						wound->healerskill = 71;
					}
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
				}
				wound->lasthealed = time (0);
			}

			/***
			Normal healers can use a kit and heal points back to the wound, but not as effectively as better healers.
			**/
			else if (treat_effect == 2)
			{
				if (mode)
				{
					//wound->healerskill = ch->skills[SKILL_HEALING];
					//if (kit->o.od.value[1])
					//	wound->healerskill += kit->o.od.value[1];
					/** To show that this is an adroitly treated wound **/
					wound->healerskill = 51;
					if (wound->infection)
						wound->infection = -1;
					if (kit->o.od.value[3])
					{
						sprintf (buf,
							"You treat the wound adroitly, making it look better.");
						adjust_wound (tch, wound, kit->o.od.value[3] * -1);
					}
					else
					{
						sprintf (buf,
							"You treat and dress the wound skillfully.");
						/** To show that this is an adroitly treated wound **/
						wound->healerskill = 51;
						adjust_wound (tch, wound, -1);
					}
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					//wound->healerskill = ch->skills[SKILL_HEALING];
					//if (kit->o.od.value[1])
					//	wound->healerskill += kit->o.od.value[1];
					/** To show that this is an adroitly treated wound **/
					wound->healerskill = 51;
					if (wound->infection)
						wound->infection = -1;
					if (kit->o.od.value[3])
					{
						sprintf (buf,
							"You treat the wound adroitly, making it look better.");
						sprintf (buf2,
							"$n treats the wound adroitly, making it look better.");
						adjust_wound (tch, wound, kit->o.od.value[3] * -1);
					}
					else
					{
						/** To show that this is an adroitly treated wound **/
						wound->healerskill = 51;
						sprintf (buf,
							"You treat and dress the wound skillfully.");
						sprintf (buf2,
							"$n treats and dresses the wound skillfully.");
						adjust_wound (tch, wound, -1);
					}
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
				}
				wound->lasthealed = time (0);
			}
			/**
			Poor healers try hard, but they can't do much more than stop the bleeding and sort of make the wound look better. At least they don't cause infections.
			Decently treated wounds cannot be made worse at this point - Case
			**/
			else if (treat_effect == 1 || (wound->healerskill > 40))
			{
				if (mode)
				{
					sprintf (buf,
						"Your ministrations to the wound seem somewhat ineffectual.");
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					wound->healerskill = -1;
				}
				else
				{
					sprintf (buf,
						"Your ministrations to the wound seem somewhat ineffectual.");
					sprintf (buf2,
						"$n's ministrations to your wound seem somewhat ineffectual.");
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					wound->healerskill = -1;
				}
				wound->lasthealed = time (0);
			}
			/***
			Bad healers don't know how to use the kits and cause damage instead of healing it, and preventing better healers from fixing it until the wound needs treatment again. Additionally, they have a chance to cause infections
			**/
			else
			{
				if (mode)
				{
					sprintf (buf,
						"Your attentions seem to make the wound worse.");
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					wound->healerskill = -1;
				}
				else
				{
					sprintf (buf,
						"Your attentions seem to make the wound worse.");
					sprintf (buf2,
						"$n's attentions to your wound seems to make the wound worse.");
					act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
					act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
					wound->healerskill = -1;
				}
				//Make the wound worse, but not enough to kill the player
				if ((ch->damage + 3) >= ch->max_hit)
					adjust_wound (tch, wound, 2);
				else if ((ch->damage + 2) >= ch->max_hit)
					adjust_wound (tch, wound, 1);

				//50% chance to cause infections but elves don't get infected
				if (!wound->infection && dice(1, 10) <= 5 && !(tch->race >= 16 && tch->race <= 19 || tch->race == 93))
					wound->infection = WOUND_INFECTIONS;

				wound->lasthealed = time (0);
			}

			wound->bleeding = 0;
			wound->bindskill = 0;
			break;
		}
	}

	if (kit)
		kit->o.od.value[0] -= 1;

	if (kit->o.od.value[0] <= 0)
	{
		send_to_char
			("Having used the last of the materials in your kit, you quickly discard it.\n",
			ch);
		if (kit->count > 1)
		{
			kit->o.od.value[0] = vtoo (kit->nVirtual)->o.od.value[0];
			kit->count -= 1;
		}
		else
			extract_obj (kit);
		return;
	}

	begin_treatment (ch, tch, location, mode);
}

void
npc_treatment (CHAR_DATA * ch, CHAR_DATA * mob, char *argument)
{
	WOUND_DATA *wound;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int damage = 0;
	float cost = 0;

	if (!mob || !IS_SET (mob->act, ACT_PHYSICIAN))
	{
		send_to_char ("I don't see a physician here.\n", ch);
		return;
	}

	if (!*argument)
	{
		send_to_char ("Which wounds did you wish to get treated?\n", ch);
		return;
	}

	if (mob->delay)
	{
		act ("$n appears to be busy.", true, ch, 0, mob, TO_CHAR | _ACT_FORMAT);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("How did you wish to be treated?\n", ch);
		return;
	}

	name_to_ident (ch, buf2);
	if (!ch->wounds)
	{
		sprintf (buf, "whisper %s I don't see any wounds on you to treat!",
			buf2);
		command_interpreter (mob, buf);
		return;
	}

	if (!str_cmp (buf, "value"))
	{
		if (!*argument)
		{
			send_to_char
				("For which wounds did you wish to get an appraisal?\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		if (!strn_cmp (buf, "all_wounds", strlen (buf)))
		{
			for (wound = ch->wounds; wound; wound = wound->next)
			{
				if (wound->healerskill)
					continue;
				if (ch->wounds->next)
				{
					cost += wound->damage * 0.85;
					cost += wound->bleeding * 0.85;
				}
				else
				{
					cost += wound->damage * 0.95;
					cost += wound->bleeding * 0.95;
				}
			}
			if (mob->shop)
				cost *= mob->shop->markup;
		}
		else
		{
			for (wound = ch->wounds; wound; wound = wound->next)
			{
				if (wound->healerskill)
					continue;
				if (!strncmp (wound->location, buf, strlen (wound->location)))
				{
					cost += wound->damage * 1.15;
					cost += wound->bleeding * 1.15;
					if (mob->shop)
						cost *= mob->shop->markup;
					break;
				}
			}
			if (!wound)
			{
				sprintf (buf, "whisper %s I don't see a wound there to treat.",
					buf2);
				command_interpreter (mob, buf);
				return;
			}
			else if (wound->healerskill)
			{
				sprintf (buf, "whisper %s That wound has already been treated.",
					buf2);
				command_interpreter (mob, buf);
				return;
			}
		}

		if (cost < 1 && cost > 0)
			cost = 1;

		if (cost < 1)
		{
			sprintf (buf,
				"whisper %s All your wounds have been taken care of - there's nothing I can do.",
				buf2);
			command_interpreter (mob, buf);
			return;
		}

		sprintf (buf,
			"whisper %s I'll get you taken care of for a total of %d coppers.",
			buf2, (int) cost);
		command_interpreter (mob, buf);
		return;
	}

	if (!strn_cmp (buf, "all_wounds", strlen (buf)))
	{
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (wound->healerskill)
				continue;
			if (ch->wounds->next)
			{
				cost += wound->damage * 0.85;
				cost += wound->bleeding * 0.85;
			}
			else
			{
				cost += wound->damage * 0.95;
				cost += wound->bleeding * 0.95;
			}
		}
		if (mob->shop)
			cost *= mob->shop->markup;
	}
	else
	{
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (!strncmp (wound->location, buf, strlen (wound->location)))
			{
				if (wound->healerskill)
					continue;
				cost += wound->damage * 1.15;
				cost += wound->bleeding * 1.15;
				if (mob->shop)
					cost *= mob->shop->markup;
				break;
			}
		}
		if (!wound)
		{
			sprintf (buf, "whisper %s I don't see a wound there to treat.",
				buf2);
			command_interpreter (mob, buf);
			return;
		}
		else if (wound->healerskill)
		{
			sprintf (buf, "whisper %s That wound has already been treated.",
				buf2);
			command_interpreter (mob, buf);
			return;
		}
	}

	if (cost < 1 && cost > 0)
		cost = 1;

	if (cost < 1)
	{
		sprintf (buf,
			"whisper %s All your wounds have been taken care of - there's nothing I can do.",
			buf2);
		command_interpreter (mob, buf);
		return;
	}

	if (!is_brother (ch, mob))
	{

		if (!can_subtract_money (ch, (int) cost, mob->mob->currency_type))
		{
			sprintf (buf, "%s You seem to be a little short on coin.", buf2);
			do_whisper (mob, buf, 83);
			return;
		}

		subtract_money (ch, (int) cost, mob->mob->currency_type);
		if (mob->shop && mob->shop->store_vnum)
			money_to_storeroom (mob, (int) cost);

		send_to_char ("\n", ch);
	}
	else
	{
		sprintf (buf, "whisper %s There is no cost to you for this treatment.",
			buf2);
		command_interpreter (mob, buf);
	}

	act ("$N promptly tends to your wounds.", true, ch, 0, mob,
		TO_CHAR | _ACT_FORMAT);
	act ("$N promptly tends to $n's wounds.", true, ch, 0, mob,
		TO_ROOM | _ACT_FORMAT);

	if (wound)
	{
		if (skill_use (mob, SKILL_HEALING, 0))
		{
			wound->healerskill = mob->skills[SKILL_HEALING];
			if (wound->infection)
				wound->infection = -1;
			damage += wound->damage;
		}
		else
			wound->healerskill = -1;
		wound->bleeding = 0;
		wound->bindskill = 0;
	}
	else
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (wound->healerskill)
				continue;
			if (skill_use (mob, SKILL_HEALING, 0))
			{
				wound->healerskill = mob->skills[SKILL_HEALING];
				if (wound->infection)
					wound->infection = -1;
			}
			else
				wound->healerskill = -1;
			wound->bleeding = 0;
			wound->bindskill = 0;
			damage += wound->damage;
		}
}

void
do_treat (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	WOUND_DATA *wound;
	OBJ_DATA *kit;
	int mode = 0, location_match = 0, treatable = 0;
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];

	one_argument (argument, arg);

	if (!(tch = get_char_room_vis (ch, arg)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (!IS_NPC (tch))
				continue;
			if (!IS_SET (tch->act, ACT_PHYSICIAN))
				continue;
			npc_treatment (ch, tch, argument);
			return;
		}
		send_to_char ("You don't see them here.\n", ch);
		return;
	}

	argument = one_argument (argument, arg);

	if (!ch->skills[SKILL_HEALING])
	{
		send_to_char
			("You'd likely only make matters worse. Find a physician!\n", ch);
		return;
	}

	if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT)
		kit = ch->right_hand;
	else if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT)
		kit = ch->left_hand;
	else
	{
		send_to_char
			("You need to have some sort of remedy handy to treat someone.\n", ch);
		return;
	}

	if (kit->o.od.value[0] <= 0)
	{
		send_to_char ("That remedy no longer contains any useful materials.\n",
			ch);
		return;
	}

	if (kit->o.od.value[2] && kit->o.od.value[2] > ch->skills[SKILL_HEALING])
	{
		send_to_char
			("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if (ch->right_hand && ch->left_hand)
	{
		send_to_char ("You'll need one hand free to treat someone.\n", ch);
		return;
	}

	if (tch == ch)
		mode = 1;

	one_argument (argument, arg);

	if (!*argument)
	{
		send_to_char ("Which location on the body are you treating?\n", ch);
		return;
	}

	for (wound = tch->wounds; wound; wound = wound->next)
	{
		if (!str_cmp (wound->location, arg))
			location_match += 1;
	}

	if (!location_match)
	{
		if (mode)
			send_to_char ("You aren't wounded in that area.\n", ch);
		else
			send_to_char ("Your patient is not wounded in that area.\n", ch);
		return;
	}

	/* allow retreatment by a better healer */
	/* sets skill to 71 if high treat roll (Exp Treat), 51 if medium (Treat), -1 if low, -1 and damage if horrid */
	for (wound = tch->wounds; wound; wound = wound->next)
	{
		/* if you have >71 healing do not allow constant attempts to re-treat on expertly treated wounds*/
		int skill = ch->skills[SKILL_HEALING];
		skill = (skill > 71 ? 71 : skill);
		if (wound->healerskill < skill && wound->healerskill != -1)
			treatable += 1;
	}

	if (!treatable)
	{
		if (mode)
			send_to_char ("You have already been treated by a healer.\n", ch);
		else
			send_to_char ("From the looks of it, your patient has already been treated.\n",	   ch);
		return;
	}

	if (mode)
	{
		sprintf (buf,
			"#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\nYou begin to work at treating the wounds on your %s...",
			expand_wound_loc (arg));
		sprintf (buf2,
			"#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\n$n begins to work at treating the wounds on $n's %s...",
			expand_wound_loc (arg));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	else /* is mode : healing others */
	{
		sprintf (buf,
			"#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\nYou begin to work at treating the wounds on $N's %s...",
			expand_wound_loc (arg));
		sprintf (buf2,
			"#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\n$n begins to work at treating the wounds on your %s...",
			expand_wound_loc (arg));
		sprintf (buf3,
			"$n begins to work at treating the wounds on $N's %s...",
			expand_wound_loc (arg));
		act (buf, false, ch, 0, tch, TO_CHAR | _ACT_FORMAT);
		act (buf2, false, ch, 0, tch, TO_VICT | _ACT_FORMAT);
		act (buf3, false, ch, 0, tch, TO_NOTVICT | _ACT_FORMAT);
	} /* end mode */

	begin_treatment (ch, tch, arg, mode);

	return;
}

void
do_health (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	show_wounds (ch, 1);

	sprintf (buf, "\nOverall Health:    %s\n"
		"Remaining Stamina: %s\n", wound_total (ch,true), fatigue_bar (ch));

	if (ch->max_mana && ch->mana)
		sprintf (buf + strlen (buf), "Remaining Mana:    %s\n", mana_bar (ch));

	send_to_char (buf, ch);
}

char *
expand_wound_loc (const char *location)
{
	static char buf[30];

	if (!str_cmp (location, "reye"))
		sprintf (buf, "right eye");

	else if (!str_cmp (location, "leye"))
		sprintf (buf, "left eye");

	else if (!str_cmp (location, "rshoulder"))
		sprintf (buf, "right shoulder");

	else if (!str_cmp (location, "lshoulder"))
		sprintf (buf, "left shoulder");

	else if (!str_cmp (location, "rupperarm"))
		sprintf (buf, "right upper arm");

	else if (!str_cmp (location, "lupperarm"))
		sprintf (buf, "left upper arm");

	else if (!str_cmp (location, "relbow"))
		sprintf (buf, "right elbow");

	else if (!str_cmp (location, "lelbow"))
		sprintf (buf, "left elbow");

	else if (!str_cmp (location, "rforearm"))
		sprintf (buf, "right forearm");

	else if (!str_cmp (location, "lforearm"))
		sprintf (buf, "left forearm");

	else if (!str_cmp (location, "rhand"))
		sprintf (buf, "right hand");

	else if (!str_cmp (location, "lhand"))
		sprintf (buf, "left hand");

	else if (!str_cmp (location, "rhip"))
		sprintf (buf, "right hip");

	else if (!str_cmp (location, "lhip"))
		sprintf (buf, "left hip");

	else if (!str_cmp (location, "rthigh"))
		sprintf (buf, "right thigh");

	else if (!str_cmp (location, "lthigh"))
		sprintf (buf, "left thigh");

	else if (!str_cmp (location, "rknee"))
		sprintf (buf, "right knee");

	else if (!str_cmp (location, "lknee"))
		sprintf (buf, "left knee");

	else if (!str_cmp (location, "rcalf"))
		sprintf (buf, "right calf");

	else if (!str_cmp (location, "lcalf"))
		sprintf (buf, "left calf");

	else if (!str_cmp (location, "rfoot"))
		sprintf (buf, "right foot");

	else if (!str_cmp (location, "lfoot"))
		sprintf (buf, "left foot");

	else if (!str_cmp (location, "rforeleg"))
		sprintf (buf, "right foreleg");

	else if (!str_cmp (location, "rhindleg"))
		sprintf (buf, "right hindleg");

	else if (!str_cmp (location, "lforeleg"))
		sprintf (buf, "left foreleg");

	else if (!str_cmp (location, "lhindleg"))
		sprintf (buf, "left hindleg");

	else if (!str_cmp (location, "rforepaw"))
		sprintf (buf, "right forepaw");

	else if (!str_cmp (location, "lforepaw"))
		sprintf (buf, "left forepaw");

	else if (!str_cmp (location, "rhindpaw"))
		sprintf (buf, "right hindpaw");

	else if (!str_cmp (location, "lhindpaw"))
		sprintf (buf, "left hindpaw");

	else if (!str_cmp (location, "rforehoof"))
		sprintf (buf, "right forehoof");

	else if (!str_cmp (location, "lforehoof"))
		sprintf (buf, "left forehoof");

	else if (!str_cmp (location, "rhindhoof"))
		sprintf (buf, "right hindhoof");

	else if (!str_cmp (location, "lhindhoof"))
		sprintf (buf, "left hindhoof");

	else if (!str_cmp (location, "rforefoot"))
		sprintf (buf, "right forefoot");

	else if (!str_cmp (location, "lforefoot"))
		sprintf (buf, "left forefoot");

	else if (!str_cmp (location, "rhindfoot"))
		sprintf (buf, "right hindfoot");

	else if (!str_cmp (location, "lhindfoot"))
		sprintf (buf, "left hindfoot");

	else if (!str_cmp (location, "rleg"))
		sprintf (buf, "right leg");

	else if (!str_cmp (location, "lleg"))
		sprintf (buf, "left leg");

	else if (!str_cmp (location, "rwing"))
		sprintf (buf, "right wing");

	else if (!str_cmp (location, "lwing"))
		sprintf (buf, "left wing");

	else if (!str_cmp (location, "stinger"))
		sprintf (buf, "barbed stinger");

	else
		sprintf (buf, location);

	return buf;

}

/************************
*  mode = 0  Shows other's wounds
*  mode = 1  Shows self wounds
**************************/
char *
show_wounds (CHAR_DATA * ch, int mode)
{
	WOUND_DATA *wound;
	LODGED_OBJECT_INFO *lodged;
	char buf4[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	static char buf[MAX_STRING_LENGTH];
	int curdamage = 0, stun_damage = 0;

	*buf = '\0';
	*buf2 = '\0';
	*buf4 = '\0';

	if (ch->wounds && mode == 0)
	{
		*buf = '\0';
		if (is_hooded (ch))
			sprintf (buf2, "It has ");
		else
			sprintf (buf2, "%s has ", char_short (ch));
		*buf2 = toupper (*buf2);
		strcat (buf, buf2);
	}

	if (mode == 1)
		send_to_char ("\n", ch);

	if (mode == 1 && get_affect (ch, AFFECT_LOST_CON))
	{
		send_to_char
			("You feel considerably less hale from the beating you took.\n", ch);
	}
	else if (mode == 1 && !ch->wounds && !ch->damage && !ch->lodged)
	{
		send_to_char ("You are in excellent condition.\n", ch);
		return NULL;
	}

	*buf3 = '\0';
	if (mode == 1 && ch->wounds)
	{
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
				stun_damage += wound->damage;
		}

		if (stun_damage > (ch->max_hit * (0.5 + (GET_WIL(ch) * 0.01))))
			send_to_char ("You are shaked, dazed and incredibly groggy.\n", ch);
		else if (stun_damage > ((ch->max_hit * (0.5 + (GET_WIL(ch) * 0.01)) * 0.66)))
			send_to_char ("You are getting quite dazed and groggy.\n", ch);
		else if (stun_damage > ((ch->max_hit * (0.5 + (GET_WIL(ch) * 0.01)) * 0.33)))
			send_to_char ("You are a little dazed.\n", ch);
		else if (stun_damage > 0)
			send_to_char ("You have taken a superficial beating.\n", ch);
	}
	*buf3 = '\0';
	if (mode == 1 && ch->damage)
	{
		for (wound = ch->wounds; wound; wound = wound->next)
		{
			if (strcmp(wound->type, "stun"))
				curdamage += wound->damage;
			else
			{
				curdamage += (wound->damage /2);
			}
		}
		curdamage += ch->damage;
		if (curdamage > 0 && curdamage <= ch->max_hit * .25)
			sprintf (buf3, "You feel slightly faint.\n");
		else if (curdamage > ch->max_hit * .25 && curdamage < ch->max_hit * .80)
			sprintf (buf3, "You feel considerably weakened.\n");
		else if (curdamage > ch->max_hit * .80 && curdamage < ch->max_hit * .95)
			sprintf (buf3, "You feel as if you can barely remain standing.\n");
		else if (curdamage > ch->max_hit * .95)
			sprintf (buf3, "You feel yourself growing cold and faint...\n");
		if (mode == 1 && get_affect (ch, AFFECT_LOST_CON))
			send_to_char ("\n", ch);
		send_to_char (buf3, ch);
	}

	if (ch->wounds && mode == 1)
		sprintf (buf, "You have ");

	for (wound = ch->wounds; wound; wound = wound->next)
	{

		if (!wound->next && wound != ch->wounds)
			sprintf (buf4, "and a ");
		else
			sprintf (buf4, "a ");

		if (mode == 0)
		{
			if (wound->bleeding > 0 && wound->bleeding <= 3)
				strcat (buf4, "#1lightly bleeding#0 ");
			else if (wound->bleeding > 3 && wound->bleeding <= 6)
				strcat (buf4, "#1moderately bleeding#0 ");
			else if (wound->bleeding > 6 && wound->bleeding <= 9)
				strcat (buf4, "#1heavily bleeding#0 ");
			else if (wound->bleeding > 9)
				strcat (buf4, "#1gushing#0 ");
			else if (wound->bindskill > 1)
			{
				if (wound->bindskill >= 70)
				{
					strcat (buf4, "neatly bound ");
				}
				else if (wound->bindskill >= 30)
				{
					strcat (buf4, "bound ");
				}
				else
				{
					strcat (buf4, "poorly bound ");
				}
			}
		}

		strcat (buf, buf4);
		if (mode == 0 && wound->bindskill > 1)
		{
			sprintf (buf4, "wound on the %s",
				expand_wound_loc (wound->location));
		}
		else if (strncmp(wound->name, "numbness", 7))  //mode is not 0 or bindskill <=1
		{
			sprintf (buf4, "%s %s on the %s", wound->severity, wound->name,
				expand_wound_loc (wound->location));
		}
		else {
			sprintf (buf4, "%s numbness in the %s", wound->severity, expand_wound_loc (wound->location));
		}

		if (mode == 1 && wound->infection)
			strcat (buf4, " #3(infected)#0");
		if (mode == 1 && wound->bleeding)
		{
			if (wound->bleeding > 0 && wound->bleeding <= 3)
				strcat (buf4, " #1(bleeding: light)#0");
			else if (wound->bleeding > 3 && wound->bleeding <= 6)
				strcat (buf4, " #1(bleeding: moderate)#0");
			else if (wound->bleeding > 6 && wound->bleeding <= 9)
				strcat (buf4, " #1(bleeding: heavy)#0");
			else if (wound->bleeding > 9)
				strcat (buf4, " #1(bleeding: mortal)#0");
		}
		else if (mode == 1 && wound->bindskill > 0)
		{
			strcat (buf4, " #4(bound)#0");
		}
		if (mode == 1 && wound->healerskill > 0)
			strcat (buf4, " #6(treated)#0");
		if (mode == 1 && wound->healerskill == -1)
			strcat (buf4, " #4(tended)#0");
		strcat (buf, buf4);
		if (!wound->next)
			strcat (buf, ".#0");
		else
			strcat (buf, ", ");
		*buf4 = '\0';
	}

	if (ch->lodged && mode == 0)
	{
		sprintf (buf2, "%s has ", HSSH (ch));
		*buf2 = toupper (*buf2);
		if (ch->wounds)
			strcat (buf, "\n\n");
		strcat (buf, buf2);
	}

	else if (ch->lodged && mode == 1)
	{
		if (ch->wounds)
			strcat (buf, "\n\n");
		strcat (buf, "You have ");
	}

	for (lodged = ch->lodged; lodged; lodged = lodged->next)
	{
		if (lodged->next)
			sprintf (buf2, "#2%s#0 lodged in the %s, ",
			vtoo (lodged->vnum)->short_description,
			expand_wound_loc (lodged->location));
		else if (!lodged->next && lodged != ch->lodged)
			sprintf (buf2, "and #2%s#0 lodged in the %s.",
			vtoo (lodged->vnum)->short_description,
			expand_wound_loc (lodged->location));
		else if (!lodged->next && lodged == ch->lodged)
			sprintf (buf2, "#2%s#0 lodged in the %s.",
			vtoo (lodged->vnum)->short_description,
			expand_wound_loc (lodged->location));
		strcat (buf, buf2);
	}

	if (mode == 1 && *buf)
	{
		send_to_char ("\n", ch);
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		return NULL;
	}
	else if (mode == 1)
		return NULL;

	return buf;
}

void
do_diagnose (CHAR_DATA * ch, char *argument, int cmd)
{

	unsigned int i;
	int damage = 0, totdam = 0, stun = 0, loaded_char = 0;
	int poisoned = 0, bleeding = 0, treated = 0, infected = 0, tended =
		0, bound = 0;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char arg[AVG_STRING_LENGTH]={'\0'};
	WOUND_DATA *wound;
	CHAR_DATA *tch = NULL, *xch = NULL;
	AFFECTED_TYPE *af, *next_affect = NULL;

	// get the target to diagnose

	if (argument && *argument)
	{
		one_argument (argument, arg);
	}
	else if (!IS_MORTAL(ch))
	{
		sprintf (arg, "%s", "self");
	}


	//do they exist and are they visible?
	if (!IS_MORTAL (ch))
	{
		if (!(tch = get_char_room_vis (ch, arg)))
		{
			if (!(tch = get_char (arg)))
			{
				if (!(tch = load_pc (arg)))
				{
					send_to_char ("No such being.\n", ch);
					return;
				}
				else
					loaded_char = 1;
			}
		}
	}
	else
	{
		if (!*arg)
		{
			if (ch->fighting)
			{
				tch = ch->fighting;
			}
			else
			{
				tch = get_char_room_vis (ch, "self");
			}
		}
		else
		{
			tch = get_char_room_vis (ch, arg);
		}

		if (!tch)
		{
			send_to_char ("You don't see them.\n", ch);
			return;
		}
	}

	if (!IS_MORTAL(ch) || (ch->skills[SKILL_HEALING] && !(ch->fighting)))
	{

		//Immortals/healers see different amounts of information
		//Healers who are fighting, don't get the extra info

		// header info

		send_to_char ("\n", ch);
		if (IS_MORTAL(ch))
		{
			sprintf (buf, "%s's wounds:", char_short(tch));
		}
		else
		{
			sprintf (buf, "%s's wounds:", tch->tname);
		}

		*buf = toupper (*buf);
		send_to_char (buf, ch);
		*buf2 = '\0';
		for (i = 0; i <= strlen (buf); i++)
		{
			if (i == strlen (buf))
				strcat (buf2, "\n");
			else
				strcat (buf2, "-");
		}
		send_to_char ("\n", ch);
		send_to_char (buf2, ch);

		//Begin wound listing
		for (wound = tch->wounds; wound; wound = wound->next)
		{
			if (wound == tch->wounds)
				send_to_char ("\n", ch);
			damage = wound->damage;
			if (strcmp(wound->type, "stun"))
			{
				totdam += damage;
			}
			else
			{
				totdam += (damage / 2);
				stun += damage;
			}

			sprintf (buf3, "A %s %s on the %s.",
				wound->severity,
				wound->name,
				expand_wound_loc (wound->location));

			pad_buffer (buf3, 45);

			//Immortals get to see points of damage, healers don't

			if (!IS_MORTAL(ch))
			{
				sprintf (buf2, "-> #1Points: %d.#0", damage);
			}
			else
			{
				sprintf (buf2, " -> ");
			}

			if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
				strcat (buf2, " #6[stun]#0 ");

			if (wound->infection)
			{
				strcat (buf2, " #3(I)#0 ");
				infected = 1;
			}

			if (wound->poison)
			{
				strcat (buf2, " #2(P)#0 ");
				poisoned = 1;
			}

			if (wound->healerskill > 0)
			{
				if (wound->healerskill < 30)
					sprintf (buf2 + strlen(buf2), " #6(Poorly Tr)#0 ");
				else if (wound->healerskill >= 30 && wound->healerskill < 70)
					sprintf (buf2 + strlen(buf2), " #6(Tr)#0 ");
				else if (wound->healerskill >= 70)
					sprintf (buf2 + strlen(buf2), " #6(Expertly Tr)#0 ");

				treated = 1;
			}


			if (wound->healerskill < 0 )
			{
				strcat (buf2, " #4(Tended)#0 ");
				tended = 1;
			}

			if (wound->bindskill > 0)
			{
				if (wound->bindskill < 30)
					sprintf (buf2 + strlen(buf2), " #4(Poorly Bo)#0 ");
				else if (wound->bindskill >= 30 && wound->bindskill < 70)
					sprintf (buf2 + strlen(buf2), " #4(Bo)#0 ");
				else if (wound->bindskill >= 70)
					sprintf (buf2 + strlen(buf2), " #4(Expertly Bo)#0 ");

				bound = 1;
			}

			if (wound->bindskill < 0)
			{
				sprintf (buf2 + strlen(buf2), " #4(Very Poorly Bo)#0 ");
			}

			// only immortals get to see points of damage
			if (!IS_MORTAL(ch) && (wound->bleeding))
			{
				sprintf (buf, " #1(Bl:%d)#0 ", wound->bleeding);
				bleeding = 1;
				strcat (buf2, buf);
			}
			else if (wound->bleeding)
			{
				sprintf (buf, " #1(Bl)#0 ");
				bleeding = 1;
				strcat (buf2, buf);
			}

			if (!IS_MORTAL(ch))
			{
				// display minutes and seconds since last heal
				int time_diff = time(0) - wound->lasthealed;
				sprintf(buf," #6[%dm%ds]#0 ",time_diff/60,time_diff%60);
				strcat(buf2, buf);
			}

			strcat (buf2, "\n");
			strcat (buf3, buf2);
			send_to_char (buf3, ch);
		}
		*buf3 = '\0';
		//extra bit of explanation when needed
		if (infected)
			strcat (buf3, "#3( I = Infected )#0 ");
		if (poisoned)
			strcat (buf3, "#2( P = Poisoned ) #0 ");
		if (bleeding)
			strcat (buf3, "#1( Bl = Bleeding )#0 ");
		if (bound)
			strcat (buf3, "#4( Bo = Bound )#0 ");
		if (treated)
			strcat (buf3, "#6( Tr = Treated )#0 ");
		if (tended)
			strcat (buf3, "#4( Te = Tended ) #0 ");

		if (infected || poisoned || bleeding || treated)
		{
			send_to_char ("\n", ch);
			send_to_char (buf3, ch);
			send_to_char ("\n", ch);
		}
		for (af = tch->hour_affects; af; af = next_affect)
		{
			if (af->type == AFFECT_LOST_CON)
			{
				sprintf (buf,
					"They have lost %d percent of their CON and will regain it in %d in-game hours.\n",
					(double)(af->a.spell.sn * 100.0/ch->con),
					af->a.spell.duration);
				send_to_char (buf, ch);
				continue;
			}
		}

		// Only immortals can see damage points

		if (!IS_MORTAL(ch))
		{
			if (tch->damage)
			{
				sprintf (buf3, "\n#5Bloodloss Points: %d.#0\n", tch->damage);
				send_to_char (buf3, ch);
			}

			if (!tch->wounds && tch->damage <= 0)
			{
				sprintf (buf3, "%s does not currently seem to be wounded.\n",
					char_short (tch));
				*buf3 = toupper (*buf3);
				send_to_char ("\n", ch);
				send_to_char (buf3, ch);
			}
			else
			{
				totdam += tch->damage;
				int max_stun = tch->max_hit * (0.5 + 0.01 * GET_WIL(tch));
				sprintf (buf3, "\n#1Total Injury Points: %d [%d stun]#0\n", totdam, stun);
				send_to_char (buf3, ch);
				sprintf (buf3, "\n#1Total Injury Point Limit: %d [%d stun]#0\n",
					tch->max_hit, max_stun);
				send_to_char (buf3, ch);
			}
		}

		if (loaded_char)
			unload_pc (tch);
	}

	//Non-healer mortals and healers who are fighting see this information
	else
	{				// Japheth's Diagnose Additions, originally conceived by Zapata

		if (!*arg || (tch != get_char_room_vis (ch, arg)))
		{
			if (ch->fighting)
			{
				tch = ch->fighting;
			}
			else
			{
				tch = get_char_room_vis (ch, "self");
			}
		}

		sprintf (buf2, "%s#0", char_short (tch));
		*buf2 = toupper (*buf2);
		sprintf (buf, "#5%s", buf2);
		send_to_char (buf, ch);
		buf2[0] = '\0';
		if (are_grouped (tch, ch))
		{
			send_to_char (" #6(grouped)#0", ch);
		}

		if (IS_SUBDUEE (tch))
		{
			if (tch->subdue == ch)
			{
				send_to_char (" #1(subdued by you)#0", ch);
			}
			else
			{
				send_to_char (" #1(subdued)#0", ch);
			}
		}
		send_to_char ("\n\n", ch);
		buf[0] = '\0';

		if (tch->damage > 0 && tch->damage <= tch->max_hit * .25)
			sprintf (buf, "%s face looks slightly pale.\n", HSHR (tch));
		else if (tch->damage > tch->max_hit * .25
			&& tch->damage < tch->max_hit * .50)
			sprintf (buf, "%s face looks rather pallid.\n", HSHR (tch));
		else if (tch->damage > tch->max_hit * .50
			&& tch->damage < tch->max_hit * .75)
			sprintf (buf, "%s face looks quite ashen.\n", HSHR (tch));
		else if (tch->damage > tch->max_hit * .75)
			sprintf (buf, "%s face looks deathly pale.\n", HSHR (tch));
		*buf = toupper (*buf);
		send_to_char (buf, ch);

		buf[0] = '\0';
		totdam = 0;
		for (wound = tch->wounds; wound; wound = wound->next)
		{
			totdam += wound->damage;
		}
		totdam += ch->damage;

		if (totdam <= tch->max_hit * .1667 && totdam != 0)
			sprintf (buf, "%s appears to be slightly the worse for wear.\n\n",
			HSSH (tch));
		else if (totdam <= tch->max_hit * .3333
			&& totdam > tch->max_hit * .1667)
			sprintf (buf, "%s appears injured.\n\n", HSSH (tch));
		else if (totdam <= tch->max_hit * .6667
			&& totdam > tch->max_hit * .3333)
			sprintf (buf, "%s appears moderately injured.\n\n", HSSH (tch));
		else if (totdam <= tch->max_hit * .8335
			&& totdam > tch->max_hit * .6667)
			sprintf (buf, "%s appears seriously injured.\n\n", HSSH (tch));
		else if (totdam >= tch->max_hit * .8335)
			sprintf (buf, "%s appears close to death.\n\n", HSSH (tch));

		if (totdam)
		{
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		if (tch->wounds || tch->lodged)
		{
			sprintf (buf, "%s", show_wounds (tch, 0));
			/****** Japheth strip of minor and small for fighting **********/
			if (ch->fighting)
			{
				sprintf(buf, strip_small_minor(buf, ch));
				reformat_string (buf, &p);
				send_to_char (p, ch);
				free_mem (p);
				p = NULL;
			}
			else
			{
				reformat_string (buf, &p);
				send_to_char (p, ch);
				free_mem (p);
				p = NULL;
			}
			/*********** end Japheth strip of minor and small ***************/

			send_to_char ("\n", ch);
		}
		else
		{
			sprintf (buf, "%s appears to be in excellent condition.\n",
				HSSH (tch));
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		if (tch->fighting)
		{
			buf[0] = '\0';	// Final String
			buf2[0] = '\0';	// Previous String
			send_to_char ("\nCurrently fighting: #1", ch);
			if (tch->fighting == ch || ch->fighting == tch)
			{
				sprintf (buf, "You");
			}
			for (xch = ch->room->people; xch; xch = xch->next_in_room)
			{
				if ((xch->fighting == tch || tch->fighting == xch) && xch != ch)
				{
					if (buf2[0])
					{		// 2nd else if already called before
						sprintf (buf + strlen (buf), ", %s", buf2);
						sprintf (buf2, "%s", char_short (xch));
					}
					else if (!buf[0])
					{		// Hasn't been written to yet
						sprintf (buf, "%s", char_short (xch));
						*buf = toupper (*buf);
					}
					else
					{		// Already a first word, but no buf2, make one
						sprintf (buf2, "%s", char_short (xch));
					}
				}//xch->fighting == tch
			} //for (xch = ch->room->people

			if (buf2[0])
			{
				sprintf (buf + strlen (buf), " and %s.#0", buf2);
			}
			else
			{
				sprintf (buf + strlen (buf), ".#0");
			}
			send_to_char (buf, ch);
		} //if (tch->fighting
		return;

	}//end Jap and Zap additions
}

char *
wound_total (CHAR_DATA * ch, bool showStun)
{
	WOUND_DATA *wound;
	static char buf[75];
	int damage = 0, limit = 0, stun = 0;

	limit = ch->max_hit;
	ch->bleeding_prompt = false;
	for (wound = ch->wounds; wound; wound = wound->next)
	{
		if (strcmp(wound->type, "stun"))
			damage += wound->damage;
		else
		{
			stun += wound->damage;
			damage += (wound->damage / 2);
		}

		if (wound->bleeding)
		{
			ch->bleeding_prompt = true;
		}
	}


	damage += ch->damage;

	if (damage <= 0)
		sprintf (buf, "#1**#3**#2**#0");

	else if (damage && damage <= limit * .1667)
		sprintf (buf, "#1**#3**#2*#0 ");

	else if (damage > limit * .1667 && damage <= limit * .3333)
		sprintf (buf, "#1**#3**#0  ");

	else if (damage > limit * .3333 && damage < limit * .6667)
		sprintf (buf, "#1**#3*#0   ");

	else if (damage > limit * .6667 && damage < limit * .8335)
		sprintf (buf, "#1**#0    ");

	else if (damage >= limit * .8335)
		sprintf (buf, "#1*#0     ");

	if (showStun && (ch->fighting || stun))
	{
		if (!stun)
			strcat(buf, " [stun: #1**#3**#2**#0]");
		else if (stun >= (limit * (0.5 + (GET_WIL(ch) * 0.01))) * 0.8335)
			strcat(buf, " [stun: #1*#0]");
		else if (stun >= (limit * (0.5 + (GET_WIL(ch) * 0.01))) * 0.6666)
			strcat(buf, " [stun: #1**#0]");
		else if (stun >= (limit * (0.5 + (GET_WIL(ch) * 0.01))) * 0.3333)
			strcat(buf, " [stun: #1**#3*#0]");
		else if (stun >= (limit * (0.5 + (GET_WIL(ch) * 0.01))) * 0.1775)
			strcat(buf, " [stun: #1**#3**#0]");
		else
			strcat(buf, " [stun: #1**#3**#2*#0]");
	}

	return buf;
}

char *
figure_location (CHAR_DATA * tar, int location)
{
	int locroll = number (1, 100);
	static char loc[50];

	// Location 0: Body area.
	// Location 1: Leg area.
	// Location 2: Arm area.
	// Location 3: Head area.
	// Location 4: Neck area.

	// Need to design a prototype for the ents and huorns!

	if (location > 4 || location < 0)
		location = 0;

	if (IS_NPC (tar) && IS_SET (tar->act, ACT_VEHICLE))
	{
		sprintf (loc, "side");
	}

	switch (tar->body_proto)
	{
	case PROTO_SERPENTINE:
		if (location == 0 || location == 3)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "back");
			else if (locroll > 50)
				sprintf (loc, "underbelly");
		}
		else if (location == 1 || location == 2)
			sprintf (loc, "tail");
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	case PROTO_WINGED_NOTAIL:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "thorax");
			else if (locroll > 40 && locroll <= 80)
				sprintf (loc, "abdomen");
			else if (locroll > 80)
			{
				if (!number (0, 1))
					sprintf (loc, "rhip");
				else
					sprintf (loc, "lhip");
			}
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "rleg");
			else if (locroll > 40 && locroll <= 80)
				sprintf (loc, "lleg");
			else if (locroll > 80 && locroll <= 90)
				sprintf (loc, "rfoot");
			else if (locroll > 90)
				sprintf (loc, "lfoot");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "rwing");
			else if (locroll > 50)
				sprintf (loc, "lwing");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "skull");
			else if (locroll > 40 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 60)
				sprintf (loc, "leye");
			else if (locroll > 60)
				sprintf (loc, "face");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	case PROTO_WINGED_TAIL:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "thorax");
			else if (locroll > 40 && locroll <= 80)
				sprintf (loc, "abdomen");
			else if (locroll > 80)
			{
				if (!number (0, 1))
					sprintf (loc, "rhip");
				else
					sprintf (loc, "lhip");
			}
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 25)
				sprintf (loc, "rleg");
			else if (locroll > 25 && locroll <= 50)
				sprintf (loc, "lleg");
			else if (locroll > 50 && locroll <= 80)
				sprintf (loc, "tail");
			else if (locroll > 80 && locroll <= 90)
				sprintf (loc, "rfoot");
			else if (locroll > 90)
				sprintf (loc, "lfoot");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "rwing");
			else if (locroll > 50)
				sprintf (loc, "lwing");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "skull");
			else if (locroll > 40 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 60)
				sprintf (loc, "leye");
			else if (locroll > 60)
				sprintf (loc, "muzzle");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	case PROTO_FOURLEGGED_FEET:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "thorax");
			else if (locroll > 50)
				sprintf (loc, "abdomen");
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "groin");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rhindleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lhindleg");
			else if (locroll > 60 && locroll <= 80)
				sprintf (loc, "tail");
			else if (locroll > 80 && locroll <= 90)
				sprintf (loc, "rhindfoot");
			else if (locroll > 90)
				sprintf (loc, "lhindfoot");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "thorax");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rforeleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lforeleg");
			else if (locroll > 60 && locroll <= 70)
				sprintf (loc, "neck");
			else if (locroll > 70 && locroll <= 85)
				sprintf (loc, "rforefoot");
			else if (locroll > 85)
				sprintf (loc, "lforefoot");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "skull");
			else if (locroll > 40 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 60)
				sprintf (loc, "leye");
			else if (locroll > 60)
				sprintf (loc, "muzzle");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	case PROTO_FOURLEGGED_PAWS:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "thorax");
			else if (locroll > 50)
				sprintf (loc, "abdomen");
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "groin");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rhindleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lhindleg");
			else if (locroll > 60 && locroll <= 80)
				sprintf (loc, "tail");
			else if (locroll > 80 && locroll <= 90)
				sprintf (loc, "rhindpaw");
			else if (locroll > 90)
				sprintf (loc, "lhindpaw");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "thorax");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rforeleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lforeleg");
			else if (locroll > 60 && locroll <= 70)
				sprintf (loc, "neck");
			else if (locroll > 70 && locroll <= 85)
				sprintf (loc, "rforepaw");
			else if (locroll > 85)
				sprintf (loc, "lforepaw");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "skull");
			else if (locroll > 40 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 60)
				sprintf (loc, "leye");
			else if (locroll > 60)
				sprintf (loc, "muzzle");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	case PROTO_FOURLEGGED_HOOVES:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 50)
				sprintf (loc, "thorax");
			else if (locroll > 50)
				sprintf (loc, "abdomen");
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "groin");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rhindleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lhindleg");
			else if (locroll > 60 && locroll <= 80)
				sprintf (loc, "tail");
			else if (locroll > 80 && locroll <= 90)
				sprintf (loc, "rhindhoof");
			else if (locroll > 90)
				sprintf (loc, "lhindhoof");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 5)
				sprintf (loc, "thorax");
			else if (locroll > 5 && locroll <= 20)
				sprintf (loc, "abdomen");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "rforeleg");
			else if (locroll > 40 && locroll <= 60)
				sprintf (loc, "lforeleg");
			else if (locroll > 60 && locroll <= 70)
				sprintf (loc, "neck");
			else if (locroll > 70 && locroll <= 85)
				sprintf (loc, "rforehoof");
			else if (locroll > 85)
				sprintf (loc, "lforehoof");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "skull");
			else if (locroll > 40 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 60)
				sprintf (loc, "leye");
			else if (locroll > 60)
				sprintf (loc, "muzzle");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	default:
	case PROTO_HUMANOID:
		if (location == 0)
		{
			if (locroll >= 1 && locroll <= 40)
				sprintf (loc, "thorax");
			else if (locroll > 40 && locroll <= 80)
				sprintf (loc, "abdomen");
			else if (locroll > 80)
			{
				if (!number (0, 1))
					sprintf (loc, "rhip");
				else
					sprintf (loc, "lhip");
			}
		}
		else if (location == 1)
		{
			if (locroll >= 1 && locroll <= 3)
				sprintf (loc, "groin");
			else if (locroll > 3 && locroll <= 20)
				sprintf (loc, "rthigh");
			else if (locroll > 20 && locroll <= 40)
				sprintf (loc, "lthigh");
			else if (locroll > 40 && locroll <= 45)
				sprintf (loc, "rknee");
			else if (locroll > 45 && locroll <= 50)
				sprintf (loc, "lknee");
			else if (locroll > 50 && locroll <= 70)
				sprintf (loc, "rcalf");
			else if (locroll > 70 && locroll <= 90)
				sprintf (loc, "lcalf");
			else if (locroll > 90 && locroll <= 95)
				sprintf (loc, "rfoot");
			else if (locroll > 95)
				sprintf (loc, "lfoot");
		}
		else if (location == 2)
		{
			if (locroll >= 1 && locroll <= 10)
				sprintf (loc, "rshoulder");
			else if (locroll > 10 && locroll <= 20)
				sprintf (loc, "lshoulder");
			else if (locroll > 20 && locroll <= 35)
				sprintf (loc, "rupperarm");
			else if (locroll > 35 && locroll <= 50)
				sprintf (loc, "lupperarm");
			else if (locroll > 50 && locroll <= 55)
				sprintf (loc, "relbow");
			else if (locroll > 55 && locroll <= 60)
				sprintf (loc, "lelbow");
			else if (locroll > 60 && locroll <= 75)
				sprintf (loc, "rforearm");
			else if (locroll > 75 && locroll <= 90)
				sprintf (loc, "lforearm");
			else if (locroll > 90 && locroll <= 95)
				sprintf (loc, "rhand");
			else if (locroll > 95)
				sprintf (loc, "lhand");
		}
		else if (location == 3)
		{
			if (locroll >= 1 && locroll <= 45)
				sprintf (loc, "skull");
			else if (locroll > 45 && locroll <= 50)
				sprintf (loc, "reye");
			else if (locroll > 50 && locroll <= 55)
				sprintf (loc, "leye");
			else if (locroll > 55)
				sprintf (loc, "face");
		}
		else if (location == 4)
		{
			sprintf (loc, "neck");
		}
		break;
	}

	return loc;
}

char *
downsized_wound (CHAR_DATA * ch, WOUND_DATA * wound)
{
	static char buf[MAX_STRING_LENGTH];
	int limit = 0;

	if (!ch || !wound)
		return NULL;

	*buf = '\0';

	limit = ch->max_hit;
	if (wound->damage <= (limit * .02))
		sprintf (buf, "small");
	else if (wound->damage > (limit * .02) && wound->damage <= (limit * .10))
		sprintf (buf, "minor");
	else if (wound->damage > (limit * .10) && wound->damage <= (limit * .20))
		sprintf (buf, "moderate");
	else if (wound->damage > (limit * .20) && wound->damage <= (limit * .30))
		sprintf (buf, "severe");
	else if (wound->damage > (limit * .30) && wound->damage <= (limit * .40))
		sprintf (buf, "grievous");
	else if (wound->damage > (limit * .40) && wound->damage <= (limit * .50))
		sprintf (buf, "terrible");
	else if (wound->damage > (limit * .50))
		sprintf (buf, "mortal");

	return buf;
}

int
natural_healing_check (CHAR_DATA * ch, WOUND_DATA * wound)
{
	char buf[MAX_STRING_LENGTH];
	char woundbuf[MAX_STRING_LENGTH];
	int roll = 0, needed = 0, hr = 0, upper = 0;

	if (!str_cmp (wound->severity, "small")
		|| !str_cmp (wound->severity, "minor"))
		hr = 6;
	else if (!str_cmp (wound->severity, "moderate"))
		hr = 5;
	else if (!str_cmp (wound->severity, "severe"))
		hr = 4;
	else if (!str_cmp (wound->severity, "grievous"))
		hr = 3;
	else if (!str_cmp (wound->severity, "terrible"))
		hr = 2;
	else
		hr = 1;

	wound->lasthealed = time (0);

	sprintf (buf, "Now attempting to heal a %s on the %s...\n",
		wound->name, expand_wound_loc (wound->location));

	if (wound->infection == -1)
	{
		sprintf (buf,
			"You feel the raging infection in a %s %s on your %s gradually subside.",
			wound->severity, wound->name,
			expand_wound_loc (wound->location));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		wound->infection = 0;
	}

	if (wound->healerskill == -1)
	{
		sprintf (buf,
			"You feel as if another treatment may benefit a %s %s on your %s.",
			wound->severity, wound->name,
			expand_wound_loc (wound->location));
		act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		wound->healerskill = 0;
	}

	if (wound->infection > 0)
	{
		sprintf (buf, "Wound is infected -- bypassing...\n");
		return 0;
	}
	if (wound->bindskill > 0
		&& str_cmp (wound->severity, "small") != STR_MATCH
		&& str_cmp (wound->severity, "minor") != STR_MATCH)
	{

		if (hr * ch->con + wound->bindskill < number (1, 80) && !(ch->race >= 16 && ch->race <= 19 || ch->race == 93)) // Elves don't get infected - Case
		{
			sprintf (buf,
				"The stinging sensation in the %s %s on your %s intensifies.",
				wound->severity, wound->name,
				expand_wound_loc (wound->location));
			act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
			wound->infection = WOUND_INFECTIONS;
			sprintf (buf, "Bound Wound Infecting.\n");
		}
		return 0;
	}

	needed = hr * ch->con;
	needed = MIN (needed, 95);	// the higher needed, the better chance of healing
	// Changes to healing thanks to Power - Case
	if (ch->race >= 16 && ch->race <= 19 || ch->race == 93) { // Elves heal faster
		needed += 25;
	}
	else if (ch->race == 23) { // Dwarves
		needed += 15;
	}
	else if (ch->race == 24 || ch->race == 25 || ch->race == 28 || ch->race == 29 || (ch->race >= 119 && ch->race <= 121)) { // Snaga, Orcs, Trolls, Uruk-Hai
		needed += 10;
	}
	else if (ch->race == 1 || ch->race == 3 || ch->race == 101) { // Dunedain and Black Numenoreans
		needed += 12;
	}
	// End Changes to healing thanks to Power

	roll = number (1, 120);
	if (!((ch->race >= 16 && ch->race <= 19) || ch->race == 93 ||
		ch->race == 24 || ch->race == 25 || ch->race == 28 || ch->race == 29
		|| (ch->race >= 119 && ch->race <= 121))) { // Elves, Snaga, Trolls, Orcs, Uruk-Hai heal continuously - Case
			switch (GET_POS (ch))
			{
			case POSITION_SLEEPING:
				roll -= 20;
				break;
			case POSITION_RESTING:
				roll -= 10;
				break;
			case POSITION_SITTING:
				roll -= 10;
				break;
			case POSITION_STANDING:
				roll += 25;
				break;
			case POSITION_FIGHTING:
				roll += 50;
				break;
			}
	}
	if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
		roll -= 25; // Stun wounds heal much faster
	if (wound->healerskill && wound->healerskill != -1)
		roll -= wound->healerskill / 3;
	roll = MAX (roll, 1);		// lower roll the better chance of healing

	sprintf (buf, "Roll: %d.\n", roll);
	sprintf (buf, "Roll Needed: %d ( %d CON x HR %d ).\n", needed, ch->con, hr);

	*woundbuf = '\0';

	if (roll <= needed)
	{
		if (roll % 5 == 0)
		{
			sprintf (buf, "Critical healing success.\n");
			if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
			{
				wound->damage -= number (2, (GET_CON(ch) / 4) + (GET_WIL(ch) / 4));
				if (wound->healerskill > 0)
					wound->damage -= (wound->healerskill / 15);
			}
			else
			{
				if (ch->con / 3 < 3)
					upper = 3;
				else
					upper = number (2, ch->con / 3);
				wound->damage -= number (1, upper);
				if (wound->healerskill >= 0)
					wound->damage -= (wound->healerskill / 25);
			}
		}
		else
		{
			sprintf (buf, "Healing success.\n");
			if (wound->type && *(wound->type) && !strn_cmp(wound->type, "stun", 4))
			{
				wound->damage -= (GET_CON(ch) / 5) + (GET_WIL(ch) / 5);
				if (wound->healerskill > 0)
					wound->damage -= wound->healerskill / 15;
			}
			else
			{
				if (ch->con / 5 < 3)
					upper = 2;
				else
					upper = number (2, ch->con / 5);
				wound->damage -= number (1, upper);
				if (wound->healerskill >= 0)
					wound->damage -= (wound->healerskill / 25);
			}
		}
		if (wound->healerskill == -1)
			wound->healerskill = 0;
		if (wound->damage > 0)
		{
			sprintf (buf, "%s", downsized_wound (ch, wound));
			free_mem (wound->severity);
			wound->severity = NULL;
			wound->severity = duplicateString (buf);
		}
	}
	else if (roll > needed && WOUND_INFECTIONS)
	{
		if (roll % 5 == 0 && wound->healerskill <= 0 && !number (0, 7) &&
			str_cmp (wound->severity, "minor")
			&& str_cmp (wound->severity, "small")
			&& !(ch->race >= 16 && ch->race <= 19 || ch->race == 93))
		{
			wound->infection = WOUND_INFECTIONS;
			sprintf (buf, "Critical failure! Wound infected...\n");
		}
	}

	if (wound->damage <= 0)
	{
		wound_from_char (ch, wound);
		return 1;
	}

	return 0;
}

void
offline_healing (CHAR_DATA * ch, int since)
{
	WOUND_DATA *wound, *next_wound;
	time_t healing_time = 0;
	int checks = 0, i = 0, roll = 0;

	healing_time = time (0) - since;

	checks += (healing_time / ((BASE_PC_STANDARD_HEALING - ch->con / 6) * 60));	// BASE_PC is in minutes, not seconds.

	/* wound reduction */
	for (wound = ch->wounds; wound; wound = next_wound)
	{
		next_wound = wound->next;

		for (i = 0; i < checks; i++)
		{
			if (natural_healing_check (ch, wound) == 1)
				break;
		}

	}

	/* bloodloss reduction */
	for (i = 0; i < checks; i++)
	{
		if (ch->damage)
		{
			roll = dice (1, 100);
			if (roll <= (ch->con * 6))
			{
				if (roll % 5 == 0)
					ch->damage -= 2;
				else
					ch->damage -= 1;
				ch->lastregen = time (0);
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////
//
//      char__do_bind
//
//////////////////////////////////////////////////////////////////////

void
char__do_bind (CHAR_DATA * thisPtr, char *argument, int cmd)
{
	int time = 0;
	int heal_adj = 1;
	short nHasClothProp = 0;
	CHAR_DATA *pTargetActor = NULL;
	WOUND_DATA *pWound = NULL;
	OBJ_DATA *pClothProp = NULL;
	OBJ_DATA *pClothPropTwo = NULL;
	char strTargetKeyword[AVG_STRING_LENGTH] = "\0";

	// Check for ACT_NOBIND Flag (usually on animals)
	//
	// TODO: Determine if the NoBindFlag is redundant based on
	// race body type. Is there any reason this can't just check 'humanoid'?

	if (IS_SET (thisPtr->act, ACT_NOBIND))
	{
		send_to_char ("You can't bind!\n", thisPtr);
		return;
	}


	// Check for one empty hand

	if ((thisPtr->right_hand && (thisPtr->right_hand->obj_flags.type_flag != ITEM_HEALER_KIT && strstr(thisPtr->right_hand->name, "TEXTILE") == NULL)) && (thisPtr->left_hand && (thisPtr->left_hand->obj_flags.type_flag != ITEM_HEALER_KIT && strstr(thisPtr->left_hand->name, "TEXTILE") == NULL)))
	{
		send_to_char ("You'll need a free hand or to have only healing kits or cloth items held.\n", thisPtr);
		return;
	}

	// Check for binding cloth in either hand

	if (thisPtr->right_hand && (strstr (thisPtr->right_hand->name, "TEXTILE") != NULL || thisPtr->right_hand->obj_flags.type_flag == ITEM_HEALER_KIT))
	{
		pClothProp = thisPtr->right_hand;
		if (GET_ITEM_TYPE (pClothProp) == ITEM_HEALER_KIT && thisPtr->skills[SKILL_HEALING])
		{
			if (IS_SET (pClothProp->o.od.value[5], TREAT_BLEED))
				nHasClothProp = pClothProp->o.od.value[1];
			else
				nHasClothProp = 1;
		}
		else
			nHasClothProp = 1;
	}
	if (thisPtr->left_hand && (strstr (thisPtr->left_hand->name, "TEXTILE") != NULL || thisPtr->left_hand->obj_flags.type_flag == ITEM_HEALER_KIT))
	{
		pClothPropTwo = thisPtr->left_hand;
		if (GET_ITEM_TYPE (pClothPropTwo) == ITEM_HEALER_KIT && thisPtr->skills[SKILL_HEALING])
		{
			if (IS_SET (pClothPropTwo->o.od.value[5], TREAT_BLEED))
			{
				if (pClothPropTwo->o.od.value[5] > nHasClothProp)
				{
					nHasClothProp = pClothPropTwo->o.od.value[5];
					if (pClothProp)
						pClothProp = pClothPropTwo;
				}
			}
			else
			{
				if (!nHasClothProp)
				{
					nHasClothProp = 1;
					if (pClothProp)
						pClothProp = pClothPropTwo;
				}
			}
		}
		else
		{
			if (!nHasClothProp)
			{
				nHasClothProp = 1;
				if (pClothProp)
					pClothProp = pClothPropTwo;
			}
		}
	}

	// Check for target N/PC keyword. If none, treat self.

	argument = one_argument (argument, strTargetKeyword);
	if (!*strTargetKeyword)
	{
		pTargetActor = thisPtr;
	}
	else if (!(pTargetActor = get_char_room_vis (thisPtr, strTargetKeyword)))
	{
		send_to_char ("You do not see them here.\n\r", thisPtr);
		return;
	}

	if (GET_POS(pTargetActor) == POSITION_FIGHTING)
	{
		send_to_char ("You cannot bind someone's bleeding wounds while they're still fighting!\n\r", thisPtr);
		return;
	}

	// Look through all wounds on the target and note the bleeders.

	for (pWound = pTargetActor->wounds; pWound; pWound = pWound->next)
	{
		if (pWound->bleeding)
		{
			time += pWound->bleeding;
		}
	}


	// If there weren't any bleeders notify and bail

	if (!time)
	{
		if (pTargetActor != thisPtr)
		{
			act ("$N doesn't seem to need your assistance.", false, thisPtr, 0,
				pTargetActor, TO_CHAR | _ACT_FORMAT);
		}
		else
		{
			act ("You have no wounds in need of binding.", false, thisPtr, 0, 0,
				TO_CHAR | _ACT_FORMAT);
		}
		return;
	}

	// Show everyone the binding has begun

	if (IS_NPC(thisPtr) && IS_NPC(pTargetActor)) //NPCs carry around virtual bandages
		nHasClothProp = 1; // NPCs cannot bind PCs unless they follow PC rules

	if (pTargetActor != thisPtr)
	{
		if (nHasClothProp)
		{
			act ("You crouch beside $N, carefully beginning to bind $S wounds.", false, thisPtr, 0, pTargetActor, TO_CHAR | _ACT_FORMAT);
			act ("$n crouches beside $N, carefully beginning to bind $S wounds.", false, thisPtr, 0, pTargetActor, TO_NOTVICT | _ACT_FORMAT);
			act ("$n crouches beside you, carefully beginning to bind your wounds.", false, thisPtr, 0, pTargetActor, TO_VICT | _ACT_FORMAT);
		}
		else  //applying pressure on wound
		{
			act ("You crouch beside $N, trying to stop $S's bleeding.", false, thisPtr, 0, pTargetActor, TO_CHAR | _ACT_FORMAT);
			act ("$n crouches beside $N, trying to stop $S's bleeding.", false, thisPtr, 0, pTargetActor, TO_NOTVICT | _ACT_FORMAT);
			act ("$n crouches beside you, trying to stop your bleeding.", false, thisPtr, 0, pTargetActor, TO_VICT | _ACT_FORMAT);
		}
	}
	else // bind yourself
	{
		if (nHasClothProp)
		{
			act ("You slowly begin administering aid to your wounds, attempting to stem the bleeding.",false, thisPtr, 0, 0, TO_CHAR | _ACT_FORMAT);
			act ("$n slowly begins administering aid to $s wounds, attempting to stem the bleeding.", false, thisPtr, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
		else  //applying pressure on wound
		{
			act("You apply pressure to your wounds, attempting to stem the bleeding.",false, thisPtr, 0, 0, TO_CHAR | _ACT_FORMAT);
			act("$n apply pressure to $s wounds, attempting to stem the bleeding.",false, thisPtr, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
	}

	thisPtr->flags |= FLAG_BINDING;

	// Prepare delay callback
	if (thisPtr->skills[SKILL_HEALING] || thisPtr->skills[SKILL_EMPATHIC_HEAL])
		heal_adj =  (number (1, 2));
	else
		heal_adj =  (number (2,3));

	thisPtr->delay_type = DEL_BIND_WOUNDS;
	thisPtr->delay_ch = pTargetActor;

	thisPtr->delay = time * heal_adj;
	thisPtr->delay -= nHasClothProp; //faster if you're using a bandage
	if (thisPtr->delay < 1) { // Let's avoid some really long delays - Case
		thisPtr->delay = 1;
	}
}

void
delayed_bind (CHAR_DATA * thisPtr)
{
	bool bIsTargetActorBound = 0;
	short nHasClothProp = 0;
	CHAR_DATA *pTargetActor = NULL;
	WOUND_DATA *pWound = NULL;
	OBJ_DATA *pClothProp = NULL;
	OBJ_DATA *pClothPropTwo = NULL;


	thisPtr->flags &= ~FLAG_BINDING;

	// If we reach this point without a target, bail

	if (!(pTargetActor = thisPtr->delay_ch) || (thisPtr != pTargetActor&& !is_he_here (thisPtr, pTargetActor, true)))
	{
		send_to_char ("Your patient is no longer here!\n", thisPtr);
		thisPtr->delay_ch = NULL;
		thisPtr->delay_type = 0;
		thisPtr->delay = 0;
		return;
	}

	// Japh's addition

	if (thisPtr->right_hand && (strstr (thisPtr->right_hand->name, "TEXTILE") != NULL || thisPtr->right_hand->obj_flags.type_flag == ITEM_HEALER_KIT))
	{
		pClothProp = thisPtr->right_hand;
		if (GET_ITEM_TYPE (pClothProp) == ITEM_HEALER_KIT && thisPtr->skills[SKILL_HEALING])
		{
			if (IS_SET (pClothProp->o.od.value[5], TREAT_BLEED))
				nHasClothProp = pClothProp->o.od.value[1];
			else
				nHasClothProp = 1;
		} /* if not a healing kit but still some other textile, value = 1 */
		else
			nHasClothProp = 1;
	} /* nothing on right hand */

	if (thisPtr->left_hand && (strstr (thisPtr->left_hand->name, "TEXTILE") != NULL || thisPtr->left_hand->obj_flags.type_flag == ITEM_HEALER_KIT))
	{
		pClothPropTwo = thisPtr->left_hand;
		if (GET_ITEM_TYPE (pClothPropTwo) == ITEM_HEALER_KIT && thisPtr->skills[SKILL_HEALING])
		{
			if (IS_SET (pClothPropTwo->o.od.value[5], TREAT_BLEED))
			{
				if (pClothPropTwo->o.od.value[5] > nHasClothProp)
				{
					nHasClothProp = pClothPropTwo->o.od.value[5];
					if (pClothProp)
						pClothProp = pClothPropTwo;
				}
			}
			else /* if you have a healer kit but it's not for bleeding...*/
			{
				if (!nHasClothProp) /* and didn't have one in your right hand either */
				{
					nHasClothProp = 1; /* then you're at value one */
					if (pClothProp)
						pClothProp = pClothPropTwo; /* and use the left hand cloth instead */
				}
			}
		} /* left hand item is not a heal kit */
		else
		{
			if (!nHasClothProp) /* if you don't already have something in right, use the left one */
			{
				nHasClothProp = 1;
				if (pClothProp)
					pClothProp = pClothPropTwo;
			}
		}
	} // Japh's Addition

	if (IS_NPC(thisPtr) && IS_NPC(pTargetActor)) //NPCs carry around virtual bandages
		nHasClothProp = 1;

	// Go through the wounds and bind the bleeders if we have a BINDING of some sort

	if (nHasClothProp)
	{
		for (pWound = pTargetActor->wounds; pWound; pWound = pWound->next)
		{
			if (pWound->bleeding)
			{
				bIsTargetActorBound = 1;
				pWound->bleeding = 0;
				pWound->lastbound = time (0);

				// base binding quality is the better of empathy and healing a pc has
				pWound->bindskill =
					MAX (thisPtr->skills[SKILL_HEALING],
					thisPtr->skills[SKILL_EMPATHIC_HEAL]);

				// bonus for having both helpful skills
				pWound->bindskill += (thisPtr->skills[SKILL_HEALING] &&
					thisPtr->skills[SKILL_EMPATHIC_HEAL])
					? (((100 - pWound->bindskill) *
					MIN (thisPtr->skills[SKILL_HEALING],
					thisPtr->skills[SKILL_EMPATHIC_HEAL])) /100) : 0;

				// bonus for having cloth
				pWound->bindskill += nHasClothProp * 2;
			} // wound bleeding
		} //wounds iteration
	} // does not have a cloth prop, thus just holding it down


	// Destroy the plain Cloth, if any. Deduct a use from Remedy type objects

	if (pClothProp && GET_ITEM_TYPE (pClothProp) == ITEM_HEALER_KIT)
	{
		pClothProp->o.od.value[0] -= 1;

		if (pClothProp->o.od.value[0] <= 0)
		{
			send_to_char ("You realized you just used the last binding agent and resolve to get more.\n", thisPtr);
			/* if you used up an item and it is part of a stack, reset its oval 0 and decrement count */
			if (pClothProp->count > 1)
			{
				pClothProp->o.od.value[0] = vtoo (pClothProp->nVirtual)->o.od.value[0];
				pClothProp->count -= 1;
			}
			else // otherwise junk it
			{
				extract_obj (pClothProp);
			}
		} /* kit utilised but not used up */
	} /* not a kit */
	else if (pClothProp && nHasClothProp == 1)
	{
		if (pClothProp->count > 1)
			pClothProp->count--;
		else
			extract_obj (pClothProp);
	}

	// Show the actors that binding occured

	// if binding someone else
	if (pTargetActor != thisPtr)
	{
		if (nHasClothProp != 0)
		{
			act ("You finish your ministrations; $N's wounds are bound.", false, thisPtr, 0, pTargetActor, TO_CHAR | _ACT_FORMAT);
			act ("$n finishes $s ministrations; $N wounds are bound.", false, thisPtr, 0, pTargetActor, TO_NOTVICT | _ACT_FORMAT);
			act ("$n finishes $s ministrations; your wounds are bound.", false, thisPtr, 0, pTargetActor, TO_VICT | _ACT_FORMAT);
		}
		else
		{
			act ("You continue to apply pressure to $N's wounds.", false, thisPtr, 0, pTargetActor, TO_CHAR | _ACT_FORMAT);
			act ("$n continues to apply pressure to $N wounds.", false,thisPtr, 0, pTargetActor, TO_NOTVICT | _ACT_FORMAT);
			act ("$n continues to apply pressure to your wounds.", false,thisPtr, 0, pTargetActor, TO_VICT | _ACT_FORMAT);
		}
	}
	else /* binding self */
	{
		if (nHasClothProp != 0)
		{
			act ("You finish binding your wounds, and have managed to stem the bleeding.",false, thisPtr, 0, 0, TO_CHAR | _ACT_FORMAT);
			act	("$n finishes binding $s wounds, and has managed to stem the bleeding.", false, thisPtr, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			act ("You continue applying pressure to your wounds.", false, thisPtr, 0, 0, TO_CHAR | _ACT_FORMAT);
			act	("$n continues to apply pressure to $s wounds.", false, thisPtr, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
	} // end binding self


	/* if this bind was performed without a bandage, reset bind flag and continue to bind */
	if (nHasClothProp == 0)
	{
		if (!IS_SET (thisPtr->flags, FLAG_BINDING))
			thisPtr->flags |= FLAG_BINDING;

		thisPtr->delay_type = DEL_BIND_WOUNDS;
		thisPtr->delay_ch = pTargetActor;
		thisPtr->delay = 5; //5 second time to continue to echo
	}
	else // Clear delay for bind with cloth or kit
	{
		thisPtr->delay_ch = NULL;
		thisPtr->delay_type = 0;
		thisPtr->delay = 0;
	}
}


char *
strip_small_minor(char * wounds, CHAR_DATA * ch)
{
	std::string woundstr, temp_string;
	int jdex, kdex, temp_dex, rdex;


	woundstr = wounds;

	if ((woundstr.find("moderate", 0) == std::string::npos) &&
		(woundstr.find("severe", 0) == std::string::npos) &&
		(woundstr.find("grievous", 0) == std::string::npos) &&
		(woundstr.find("terrible", 0) == std::string::npos) &&
		(woundstr.find("mortal", 0) == std::string::npos))
	{
		kdex = woundstr.find(" has ");
		if (kdex != std::string::npos)
		{
			woundstr.erase(kdex);
			woundstr.insert(kdex, " has no apparent wounds.");
			return ((char *)woundstr.c_str());
		}
	}

	while ((woundstr.find("small", 0) != std::string::npos) || (woundstr.find("minor", 0) != std::string::npos))
	{
		if (woundstr.find("small", 0) == std::string::npos)
			jdex = woundstr.find("minor", 0);
		else
			jdex = woundstr.find("small", 0);

		if (woundstr[jdex - 2] == 'a')
			temp_dex = jdex;	//begining of substring to be removed
		else //There may be something between the 'a' and the 'small/minor' like 'lightly bleeding' or 'gushing'
		{
			for (rdex = jdex; rdex > 0; rdex --)
			{
				if (woundstr.substr(rdex, 3) == " a ")
				{
					jdex = rdex + 3;
					break;
				}
			}
			temp_dex = jdex;	//begining of substring to be removed
		}

		while (woundstr[jdex] != ',' && woundstr[jdex] != '.')
		{
			// Copy the sub-string into a temporary string and advance index
			temp_string.push_back(woundstr[jdex]);
			jdex++;
		} // while (woundstr[jdex] != ','

		// deleting a string at the begining or in the middle
		if (woundstr[jdex] == ',')
		{
			// We delete the string we don't want, including the preceding "a " and trailing space
			temp_string.push_back(woundstr[jdex]);
			temp_string = " a " + temp_string;
			woundstr.erase(temp_dex - 2, temp_string.length());
		}

		//Deleting the last segment in the string.
		else if (woundstr[jdex] == '.') // we are removing a string that is the end
		{
			// We delete the string we don't want, including the preceding "a "
			temp_string = " a " + temp_string;
			woundstr.erase(temp_dex - 2, temp_string.length());


			kdex = woundstr.rfind(",");
			if (kdex != std::string::npos)
			{
				woundstr.erase(kdex);
				woundstr.insert(woundstr.length(), ".");
			}

			kdex = woundstr.rfind(",");
			if (kdex != std::string::npos)
			{
				woundstr.replace(kdex, 2, ", and ");
			}

		}
		//Deal with two special cases
		kdex = woundstr.find(" has and ");
		if (kdex != std::string::npos)
		{
			woundstr.erase(kdex+4, 4);
		}

		kdex = woundstr.find(".");
		if (kdex == std::string::npos)
		{
			woundstr.insert(woundstr.length(), ".");
		}

		temp_string.clear();
	}

	return ((char *)woundstr.c_str());
}

void
do_wound (CHAR_DATA *ch, char * arg, int cmd)
{
	std::string argument = arg, buffer;
	argument = one_argument(argument, buffer);

	if (buffer.empty())
	{
		send_to_char("Who do you wish to wound?\n", ch);
		return;
	}

	if (buffer[0] == '?')
	{
		std::string help_mess;
		help_mess = "Syntax: wound <char> <damage> <type> [options]\n\n";
		help_mess = help_mess + "damage - points or XdY format\n";
		help_mess = help_mess + "type - stab, chop, blunt, slash, frost, fire, bite, claw, fist, or stun\n\n";
		
		help_mess = help_mess + "Options:\n";
		help_mess = help_mess + "location <location> - to specify a location for the damage.\n\n";
		help_mess = help_mess + "Level <value> - allows you to take armor, and other adjustments to damage into account. level 1 is a normal hit, up to level 4 for a double critical hit.\n";
		help_mess = help_mess + "Normal damage will ignore armor and can be at any location you specify. If you use the level option, you can must use head, neck, body, arms, or leg for locations\n\n";
		help_mess = help_mess + "bleeding <value> - to specify points of  bleeding with the wound\n\n";
		help_mess = help_mess + "lodge <vnum> - lodge an object in the wound\n\n";
		help_mess = help_mess + "infected - makes the wound infected\n\n";
		help_mess = help_mess + "echo  - will echo the result of this command to the player\n\n";
	
		help_mess = help_mess + "Examples:\n";
		help_mess = help_mess + "#3wound traithe 4 stab#0 - will give Traithe 4 points of damage in a random location\n\n";
		help_mess = help_mess + "#3wound traithe 2d6 stab#0 - will give Traithe 2d6 of damage in random location\n\n";
		help_mess = help_mess + "#3wound traithe 4 stab location nose#0 - will give Traithe 4 points of damage to his nose\n\n";
		help_mess = help_mess + "#3wound traithe 4 stab location body level 4 echo#0 -  Will give Traithe 4 points of damage, adjusted to account for armor on his body, level 4 (double critcal) damage bonus, location bonus and brutality bonus for an accurate wound, and tell him he received a massive wound.\n";
		
		send_to_char (help_mess.c_str(), ch);
		return;
	}

	CHAR_DATA * target = '\0';
	if (!(target = get_char_room_vis(ch, buffer.c_str())))
	{
		send_to_char("You do not see anyone with the keyword [#2", ch);
		send_to_char(buffer.c_str(), ch);
		send_to_char("#0] to wound.\n", ch);
		return;
	}

	argument = one_argument(argument, buffer);
	if (buffer.empty())
	{
		send_to_char("How much damage do you wish to do?\n", ch);
		return;
	}

	int damage = 0;
	if (is_number(buffer.c_str()))
	{
		damage = atoi(buffer.c_str());
	}
	else
	{
		if (buffer.find('d') == std::string::npos)
		{
			send_to_char("Damage is either a number, or xdy / xdy+i. (error 1)\n", ch);
			return;
		}
		std::stringstream streambuf;
		streambuf << buffer;
		std::getline(streambuf, buffer, 'd');

		if (!is_number(buffer.c_str()))
		{
			send_to_char("Damage is either a number, or xdy / xdy+i. (error 2)\n", ch);
			return;
		}

		int number = atoi(buffer.c_str());
		int sides = 0;
		int mod = 0;
		// Work out whether or not it's xdy or xdy+i
		//buffer = streambuf.str();
		std::getline(streambuf, buffer, '\0');
		bool subtract_dam = false;
		if (buffer.find('+') == std::string::npos && buffer.find('-') == std::string::npos)
		{
			if (!is_number(buffer.c_str()))
			{
				send_to_char("Damage is either a number, or xdy / xdy+i. (error 3)\n", ch);
				return;
			}

			sides = atoi(buffer.c_str());
		}
		else
		{
			std::stringstream secondstream;
			secondstream << buffer;
			if (buffer.find('+') == std::string::npos)
			{
				subtract_dam = true;
				std::getline(secondstream, buffer, '-');
			}
			else
				std::getline(secondstream, buffer, '+');

			if (!is_number(buffer.c_str()))
			{
				send_to_char("Damage is either a number, or xdy / xdy+i. (error 4)\n", ch);
				return;
			}

			sides = atoi(buffer.c_str());
			//buffer = streambuf.str();
			std::getline(secondstream, buffer, '\0');

			if (!is_number(buffer.c_str()))
			{
				send_to_char("Damage is either a number, or xdy / xdy+i. (error 5)\n", ch);
				return;
			}

			// mod = atoi(buffer.c_str());
			mod = atoi(buffer.c_str()) * (subtract_dam) ? (-1) : (1);
		}

		damage = dice(number, sides) + mod;
	}

	if (damage < 0)
		damage = 0;

	argument = one_argument(argument, buffer);
	if (buffer.empty())
	{
		send_to_char("What type of damage are you intending to do?\n", ch);
		return;
	}

	int type = -1;
	std::string verbose_type = buffer;
	if (!buffer.compare("pierce"))
		type = 0;
	else if (!buffer.compare("stab"))
		type = 1;
	else if (!buffer.compare("chop"))
		type = 2;
	else if (!buffer.compare("blunt"))
		type = 3;
	else if (!buffer.compare("slash"))
		type = 4;
	else if (!buffer.compare("frost"))
		type = 5;
	else if (!buffer.compare("fire"))
		type = 6;
	else if (!buffer.compare("bite"))
		type = 7;
	else if (!buffer.compare("claw"))
		type = 8;
	else if (!buffer.compare("fist"))
		type = 9;
	else if (!buffer.compare("stun"))
		type = 10;

	if (type == -1)
	{
		send_to_char("Invalid damage type.\n", ch);
		return;
	}

	std::string location;
	int bleeding = 0, lodged = 0;
	bool infected = false, echo = false;
	int hit_type = 0;

	argument = one_argument(argument, buffer);
	if (!buffer.empty())
	{
		for ( ; ;)
		{
			if (!buffer.compare("location"))
			{
				argument = one_argument(argument, location);
				if (location.empty())
				{
					send_to_char("Which location?\n", ch);
					return;
				}
			}
			else if (!buffer.compare("bleeding"))
			{
				argument = one_argument(argument, buffer);
				if (!is_number(buffer.c_str()) || buffer.empty())
				{
					send_to_char("How much bleeding?\n", ch);
					return;
				}

				bleeding = atoi(buffer.c_str());
			}
			else if (!buffer.compare("lodged"))
			{
				argument = one_argument(argument, buffer);
				if (!is_number(buffer.c_str()) || buffer.empty())
				{
					send_to_char("Lodge what?\n", ch);
					return;
				}
				if (!(vtoo(atoi(buffer.c_str()))))
				{
					send_to_char("Object does not exist.\n", ch);
					return;
				}

				lodged = atoi(buffer.c_str());
			}
			else if (!buffer.compare("infected"))
			{
				infected = true;
			}
			else if (!buffer.compare("echo"))
			{
				echo = true;
			}
			else if (!buffer.compare("level"))
			{
				argument = one_argument(argument, buffer);
				if (!is_number(buffer.c_str()) || buffer.empty())
				{
					send_to_char("If you are want to include armor, you must specify type of hit\n", ch);
					send_to_char("1 - hit, 2 - hard hit, 3 - critical hit, 4 - double critical hit.\n", ch);
					return;
				}
				
				hit_type = atoi(buffer.c_str());
			}
			else
			{
				break;
			}
			argument = one_argument(argument, buffer);
		}
	}

	if (location.empty())
	{
		int location_number = -1;
		int roll = number(1, 100);

		while (roll > 0)
			roll = roll - body_tab[0][++location_number].percent;

		location = figure_location(target, location_number);
	}

		//adjust damage by armor worn - does not damage the armor!!
	if (hit_type > 0)
	{
		int wear_loc1;
		int wear_loc2;
		int dam_mult = 1;
		int dam_div = 1;
		OBJ_DATA *eq;
		char locbuf[AVG_STRING_LENGTH] = "\0";		
				
			//body_type = 0 is the only one supported
			//body_type = target->body_type;
		
		if (!str_cmp (location.c_str(), "body"))
			{
				wear_loc1 = body_tab[0][0].wear_loc1;
				wear_loc2 = body_tab[0][0].wear_loc2;
				dam_mult = body_tab[0][0].damage_mult;
				dam_div = body_tab[0][0].damage_div;
				
			}
		else if (!str_cmp (location.c_str(), "leg"))
			{
				wear_loc1 = body_tab[0][1].wear_loc1;
				wear_loc2 = body_tab[0][1].wear_loc2;
				dam_mult = body_tab[0][1].damage_mult;
				dam_div = body_tab[0][1].damage_div;
			}
		else if (!str_cmp (location.c_str(), "arms"))
			{
				wear_loc1 = body_tab[0][2].wear_loc1;
				wear_loc2 = body_tab[0][2].wear_loc2;
				dam_mult = body_tab[0][2].damage_mult;
				dam_div = body_tab[0][2].damage_div;
			}
		else if (!str_cmp (location.c_str(), "head"))

			{
				wear_loc1 = body_tab[0][3].wear_loc1;
				wear_loc2 = body_tab[0][3].wear_loc2;
				dam_mult = body_tab[0][3].damage_mult;
				dam_div = body_tab[0][3].damage_div;
			}
		else if (!str_cmp (location.c_str(), "neck"))

			{
				wear_loc1 = body_tab[0][4].wear_loc1;
				wear_loc2 = body_tab[0][4].wear_loc2;
				dam_mult = body_tab[0][4].damage_mult;
				dam_div = body_tab[0][4].damage_div;
			}
				

		
		/* Subtract the armor protection at the hit location */
			//first armor bit
		eq = get_equip (target, wear_loc1);
		
		if (eq && eq->obj_flags.type_flag == ITEM_ARMOR)
			damage = damage - (eq->item_wear/100) * eq->o.armor.armor_value;
		else if (target->armor) /* Mobs will have marmor, which is natural armor */
			damage -= (target->armor);
		
			//second armor bit
		eq = get_equip (target, wear_loc2);
		
		if (eq && eq->obj_flags.type_flag == ITEM_ARMOR)
			damage = damage - (eq->item_wear/100) * eq->o.armor.armor_value;
		
		/* Multiply by hit location multiplier */
		
		damage *= (dam_mult * 1.0) / (dam_div * 1.0);
		
		/* Multiply in critical strike bonus */
		int off_result;
		off_result = hit_type + 4; //mapping hit_type to RESULT_HIT
		if (off_result == RESULT_HIT)
			damage *= 1;
		else if (off_result == RESULT_HIT1)
			damage *= 1.3;
		else if (off_result == RESULT_HIT2)
			damage *= 1.5;
		else if (off_result == RESULT_HIT3)
		{
			damage += 2;
			damage *= 1.7;
		}
		else if (off_result == RESULT_HIT4)
		{
			damage += 3;
			damage *= 2;
		}

		
		if (get_affect (target, MAGIC_AFFECT_CURSE)
			&& !get_affect (target, MAGIC_AFFECT_BLESS))
			damage += damage / 4 + 1;
		
		else if (get_affect (target, MAGIC_AFFECT_BLESS) &&
				 !get_affect (target, MAGIC_AFFECT_CURSE))
			damage = damage * 3 / 4;
		
		damage *= COMBAT_BRUTALITY;
		
		damage = (int) damage;
		
		
	}

	
	std::string output;
	std::ostringstream conversion;
	conversion << damage;
	output = "Giving a";
	if (infected)
		output = output + "n infected";
	output = output + " [#6" + verbose_type + "#0] wound of damage [#1" + conversion.str() + "#0] on the #6" + expand_wound_loc((char *) location.c_str()) + "#0 to #5" + char_short(target) + "#0";
	if (bleeding)
	{
		std::stringstream secondconv;
		secondconv << bleeding;
		output = output + " #1bleeding for " + secondconv.str() + "#0";
	}
	if (lodged)
		output = output + " with #2" + obj_short_desc(vtoo(lodged)) + "#0 lodged";
	if (echo)
		output = output + " (echoed to the player)";

	output = output + ".\n";

	send_to_char (output.c_str(), ch);
	if (lodged)
		lodge_missile (target, vtoo(lodged), (char *) location.c_str());
	wound_to_char (target, (char *) location.c_str(), damage, type, bleeding, 0, (int) infected);
	return;
}
