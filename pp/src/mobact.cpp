/*------------------------------------------------------------------------\
|  mobact.c : Mobile AI Routines                      www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

#define JAILBAG_DESC_PREPEND "Belongings for"

bool could_attack (const CHAR_DATA* ch, const CHAR_DATA* target)
{
  int i = 0;
  for (CHAR_DATA * tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room)
    {
      if ((tch->fighting == target) && (++i >= 4))
	{
	  return false;
        }
    }
  return true;
}

int
is_wanted_in_area (CHAR_DATA * ch)
{
  if (!ch || !ch->room)
    return 0;

  if (get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone))
    return 1;

  if (get_affect (ch, MAGIC_CRIM_HOODED + ch->room->zone))
    return 1;

  return 0;
}

int
enforcer (CHAR_DATA * ch, CHAR_DATA * crim, int will_act, int witness)
{
  OBJ_DATA *obj;

  /* Return values:

     -1    ch is unable to enforce anyone
     0    ch is unable to enforce crim
     1    ch will enforce crim

     will_act
     0    ch won't do anything about the criminal act
     1    ch will do something: stand or attack

     witness (can also mean guard just saw through disguise)
     0    enforcer didn't see the act
     1    enforcer saw the act, so MAY attack or react
   */

/*	int			db_on = 0; */
  char buf[MAX_STRING_LENGTH];

  if (ch == crim)
    return 0;

  if (ch->pc)
    return -1;

  if (!IS_SET (ch->act, ACT_ENFORCER))
    return -1;

  if (ch->fighting)
    return -1;

  if (ch->desc)
    return -1;

  if (is_room_affected (ch->room->affects, MAGIC_ROOM_CALM))
    return -1;

  if (IS_SUBDUEE (ch))
    return -1;

  if (!CAN_SEE (ch, crim))
    return 0;

  if (IS_SUBDUEE (crim))
    return 0;

  if (is_brother (ch, crim))
    return 0;

  if (get_affect (crim, MAGIC_CRIM_HOODED + ch->room->zone))
    ;
  else if (is_hooded (crim) && witness &&
	   get_affect (crim, MAGIC_CRIM_BASE + ch->room->zone))
    ;
  else if (!is_hooded (crim) &&
	   get_affect (crim, MAGIC_CRIM_BASE + ch->room->zone))
    ;
  else
    return 0;

  /* A bad guy.  Deal with him. */

  if (GET_POS (ch) == SIT || GET_POS (ch) == REST)
    {

      if (will_act)
	add_second_affect (SA_STAND, 4, ch, NULL, NULL, 0);

      return -1;
    }

  if (!will_act)
    return 1;

  if (!ch->fighting && !ch->ranged_enemy && !ch->delay)
    {
      do_say (ch, "Surrender, now, or pay the consequences!", 0);
    }

  if (!IS_SET (crim->act, ACT_PARIAH) &&
      !IS_SET (crim->act, ACT_AGGRESSIVE) &&
      real_skill (ch, SKILL_SUBDUE) &&
      zone_table[ch->mob->zone].jail_room &&
      (GET_POS (crim) == UNCON || GET_POS (crim) == SLEEP))
    {
      name_to_ident (crim, buf);
      do_subdue (ch, buf, 0);
      return 1;
    }

  else
    {
      // I do not understand the purpose of setting obj here...
      // seems to be do-nothing code until "set_fighting"
      for (obj = ch->equip; obj; obj = obj->next_content)
	if (obj->o.weapon.use_skill == SKILL_LONGBOW ||
	    obj->o.weapon.use_skill == SKILL_SHORTBOW ||
	    obj->o.weapon.use_skill == SKILL_CROSSBOW)
	  break;
      if (ch->right_hand
	  && (ch->right_hand->o.weapon.use_skill == SKILL_SHORTBOW
	      || ch->right_hand->o.weapon.use_skill == SKILL_LONGBOW
	      || ch->right_hand->o.weapon.use_skill == SKILL_CROSSBOW))
	{
	  obj = ch->right_hand;
	}
      if (ch->left_hand
	  && (ch->left_hand->o.weapon.use_skill == SKILL_SHORTBOW
	      || ch->left_hand->o.weapon.use_skill == SKILL_LONGBOW
	      || ch->left_hand->o.weapon.use_skill == SKILL_CROSSBOW))
	{
	  obj = ch->left_hand;
	}

      if (could_attack (ch, crim))
	{
	  set_fighting (ch, crim);
	}

      return 1;
    }

  return 0;
}

CHAR_DATA *
mob_remembers_whom (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  ROOM_DATA *room;
  struct memory_data *mem;

  room = ch->room;

  if (!IS_SET (ch->act, ACT_MEMORY))
    return 0;

  for (tch = room->people; tch; tch = tch->next_in_room)
    {

      if (IS_NPC (tch) || !CAN_SEE (ch, tch))
	continue;

      for (mem = ch->remembers; mem; mem = mem->next)
	if (!strcmp (mem->name, GET_NAME (tch)))
	  {

	    if (tch->fighting && IS_SET (tch->flags, FLAG_SUBDUING))
	      continue;

	    return tch;
	  }
    }

  return NULL;
}

int
mob_remembers (CHAR_DATA * ch)
{
  CHAR_DATA *tch;

  if ((tch = mob_remembers_whom (ch)))
    {
      if (could_attack (ch, tch))
	{
	  set_fighting (ch, tch);
	}
      else
	{
	  add_threat (ch, tch, 2);
	}
      return 1;
    }

  return 0;
}

#define MOB_IGNORES_DOORS -1
int
mob_wander (CHAR_DATA * ch)
{
  ROOM_DATA *room_exit;
  ROOM_DATA *room;
  int room_exit_zone;
  int room_exit_virt;
  int exit_tab[6];
  int zone;
  int num_exits = 0;
  int to_exit;
  int i;
  int door = MOB_IGNORES_DOORS;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];


  if (!IS_SET (ch->act, ACT_ENFORCER) && number (0, 5))
    return 0;
  else if (IS_SET (ch->act, ACT_ENFORCER) && number (0, 2))
    return 0;

  room = ch->room;
  zone = room->zone;

  if (zone == 76)
    zone = 10;

  if (IS_SET (ch->act, ACT_SENTINEL))
    return 0;

  if (IS_SET (ch->act, ACT_VEHICLE))
    return 0;

  if ((ch->following && ch->following->room == ch->room) || number (0, 9))
    return 0;

  if (GET_POS (ch) != POSITION_STANDING)
    return 0;

  if (ch->desc && ch->desc->original)
    return 0;

  if (IS_RIDEE (ch)
      && (IS_SET (ch->mount->act, ACT_SENTINEL) || !IS_NPC (ch->mount)))
    return 0;

  if (IS_HITCHEE (ch)
      && (IS_SET (ch->hitcher->act, ACT_SENTINEL) || !IS_NPC (ch->hitcher)
	  || ch->hitcher->desc))
    return 0;

  if (IS_SUBDUEE (ch))
    return 0;

  /* Decide here if we want to open a door */
  door = (ch->race > 29
	  || is_mounted (ch)) ? MOB_IGNORES_DOORS : (rand () % 6);

  for (i = 0; i <= LAST_DIR; i++)
    {

      if ((!EXIT (ch, i)) ||	/* IF Dir not an exit */
	  (EXIT (ch, i)->to_room == NOWHERE) ||	/* OR Exit to nowhere */
	  (IS_SET (EXIT (ch, i)->exit_info, EX_CLOSED) &&	/* OR closed doors AND */
	   ((i != door) ||	/* -- IF ch is ignoring door */
	    (EXIT (ch, i)->exit_info != (EX_ISDOOR | EX_CLOSED | EX_ISGATE))	/* -- OR they are complex */
	   )))
	{
	  continue;
	}

      room_exit = vtor (EXIT (ch, i)->to_room);

      if (!room_exit)
	{
	  continue;
	}

      room_exit_zone = room_exit->zone;

      if (room_exit_zone == 76)
	room_exit_zone = 10;

      if (!IS_SET (room_exit->room_flags, NO_MOB) &&
	  !(IS_MERCHANT (ch) &&
	    IS_SET (room_exit->room_flags, NO_MERCHANT)) &&
	  !(ch->mob->noaccess_flags & room_exit->room_flags) &&
	  (!ch->mob->access_flags
	   || ch->mob->access_flags & room_exit->room_flags)
	  && !(IS_SET (ch->act, ACT_STAY_ZONE) && zone != room_exit_zone))
	exit_tab[num_exits++] = i;
    }

  if (num_exits == 0)
    return 0;

  to_exit = number (1, num_exits) - 1;

  if (vtor (EXIT (ch, exit_tab[to_exit])->to_room)->nVirtual == ch->last_room)
    to_exit = (to_exit + 1) % num_exits;

  room_exit_virt = vtor (EXIT (ch, exit_tab[to_exit])->to_room)->nVirtual;

  ch->last_room = room_exit_virt;

  i = exit_tab[to_exit];
  if (IS_SET (ch->room->dir_option[i]->exit_info, EX_CLOSED))
    {
      one_argument (ch->room->dir_option[i]->keyword, buf2);
      sprintf (buf, "%s %s", buf2, dirs[i]);
      do_open (ch, buf, 0);
    }

  if (IS_SET (ch->affected_by, AFF_SNEAK) && ch->speed < 3)
    {
      ch->speed = 0;
      do_sneak (ch, NULL, exit_tab[to_exit]);
    }
  else
    do_move (ch, "", exit_tab[to_exit]);

  return 1;
}

void
common_guard (CHAR_DATA * ch)
{
  int dir;
  int knocked = 0;
  bool bleeding = false;
  bool nodded = false;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  ROOM_DATA *room;
  WOUND_DATA *wound;
  CHAR_DATA *tch;

  for (wound = ch->subdue->wounds; wound; wound = wound->next)
    if (wound->bleeding)
      bleeding = true;

  if (bleeding)
    {
      do_sheathe (ch, "", 0);
      do_wear (ch, "shield", 0);
      name_to_ident (ch->subdue, buf);
      char__do_bind (ch, buf, 0);
      return;
    }

  if (!is_wanted_in_area (ch->subdue) && !IS_SET (ch->act, ACT_JAILER))
    {
      do_release (ch, "", 0);
      return;
    }

  if (is_hooded (ch->subdue))
    {
      if (get_equip (ch->subdue, WEAR_ABOUT)
	  && IS_SET (get_equip (ch->subdue, WEAR_ABOUT)->obj_flags.
		     extra_flags, ITEM_MASK))
	{
	  one_argument (get_equip (ch->subdue, WEAR_ABOUT)->name, buf);
	  name_to_ident (ch->subdue, buf2);
	  sprintf (buf3, "%s %s", buf, buf2);
	  do_take (ch, buf3, 0);
	  do_junk (ch, buf, 0);
	  return;
	}
      if (get_equip (ch->subdue, WEAR_HEAD)
	  && IS_SET (get_equip (ch->subdue, WEAR_ABOUT)->obj_flags.
		     extra_flags, ITEM_MASK))
	{
	  one_argument (get_equip (ch->subdue, WEAR_ABOUT)->name, buf);
	  name_to_ident (ch->subdue, buf2);
	  sprintf (buf3, "%s %s", buf, buf2);
	  do_take (ch, buf3, 0);
	  do_junk (ch, buf, 0);
	  return;
	}
      if (get_equip (ch->subdue, WEAR_FACE)
	  && IS_SET (get_equip (ch->subdue, WEAR_ABOUT)->obj_flags.
		     extra_flags, ITEM_MASK))
	{
	  one_argument (get_equip (ch->subdue, WEAR_ABOUT)->name, buf);
	  name_to_ident (ch->subdue, buf2);
	  sprintf (buf3, "%s %s", buf, buf2);
	  do_take (ch, buf3, 0);
	  do_junk (ch, buf, 0);
	  return;
	}
    }

  room = ch->room;

  if (ch->in_room == zone_table[ch->mob->zone].jail_room_num)
    {

      if (!zone_table[ch->mob->zone].jailer ||
	  !vtom (zone_table[ch->mob->zone].jailer))
	{
	  printf ("Attempting to release to room.\n");
	  fflush (stdout);
	  do_release (ch, "", 0);
	  return;
	}

      strcpy (buf, "to ");
      one_argument (GET_NAMES (vtom (zone_table[ch->mob->zone].jailer)),
		    buf + 3);
      do_release (ch, buf, 0);

      if (ch->mob->spawnpoint)
	{
	  act ("$n returns to duty.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	  char_from_room (ch);
	  char_to_room (ch, ch->mob->spawnpoint);
	  act ("$n arrives.", true, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
	}

      return;
    }

  dir = track (ch, zone_table[ch->mob->zone].jail_room_num);

  if (dir == -1)
    {
      sprintf (buf, "Couldn't track guard to jail: in room %d, guard %d",
	       ch->in_room, ch->mob->nVirtual);
      system_log (buf, true);
      do_release (ch, "", 0);
      return;
    }

  if (!ch->room->dir_option[dir])
    {
      sprintf (buf, "NON-REV error, guard can't track: in room %d, guard %d",
	       ch->in_room, ch->mob->nVirtual);
      system_log (buf, true);
      do_release (ch, "", 0);
      return;
    }

  one_argument (ch->room->dir_option[dir]->keyword, buf2);

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
	{
	  sprintf (buf, "%s %s", buf2, dirs[dir]);
	  do_knock (ch, buf, 0);
	  knocked = 1;
	}
    }

  else if (IS_SET (ch->room->dir_option[dir]->exit_info, EX_CLOSED))
    {
      sprintf (buf, "%s %s", buf2, dirs[dir]);
      do_open (ch, buf, 0);
    }

  if (!knocked && !CAN_GO (ch, dir))
    {
      send_to_room ("Hmmm, I can't take you any further.\n", ch->in_room);
      do_release (ch, "", 0);
    }
  else
    do_move (ch, "", dir);
}

void
jailer_func (CHAR_DATA * ch)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  OBJ_DATA *bag = NULL;
  int cells[] = { 0, 0, 0 };
  char buf[MAX_STRING_LENGTH];
  char msg[MAX_STRING_LENGTH];
  time_t time_now;
  char *date;

  cells[0] = ch->cell_1;
  cells[1] = ch->cell_2;
  cells[2] = ch->cell_3;

  if (!IS_SUBDUER (ch))
    {				/* How can this happen?  Just release guy */
      do_release (ch, "", 0);
      return;
    }

  victim = ch->subdue;

  if (!is_wanted_in_area (victim))
    {
      do_release (ch, "", 0);
      return;
    }

  act ("$N brutally smashes you on the back of your head!",
       false, victim, 0, ch, TO_CHAR);

  GET_POS (victim) = SLEEP;

  act ("$n smashes $N on the head and removes $s possessions.",
       false, ch, 0, victim, TO_NOTVICT);

  bag = load_object (VNUM_JAILBAG);

  if (bag && (victim->right_hand || victim->left_hand || victim->equip))
    {
      sprintf (buf, "A prison bag, labeled '%s %s' sits here.",
	       JAILBAG_DESC_PREPEND, victim->short_descr);
      bag->description = str_dup (buf);

      sprintf (buf, "a bag labeled '%s %s'", JAILBAG_DESC_PREPEND,
	       victim->short_descr);
      bag->short_description = str_dup (buf);

      if (IS_NPC (victim))
	bag->obj_timer = 24 * 4;	/* one rl day */

      if (victim->right_hand)
	{
	  obj = victim->right_hand;
	  obj_from_char (&obj, 0);
	  if (bag)
	    obj_to_obj (obj, bag);
	}

      if (victim->left_hand)
	{
	  obj = victim->left_hand;
	  obj_from_char (&obj, 0);
	  if (bag)
	    obj_to_obj (obj, bag);
	}

      while (victim->equip)
	{
	  obj = victim->equip;
	  if (bag)
	    obj_to_obj (unequip_char (victim, obj->location), bag);
	}

      obj_to_room (bag, ch->in_room);
    }

  do_release (ch, "", 0);

  char_from_room (victim);
  char_to_room (victim, cells[number (0, 2)]);

  send_to_room ("The jailer tosses his prisoner into a cell.\n\r",
		ch->in_room);

  act ("The jailer tosses $n into the cell with you.",
       false, victim, 0, 0, TO_ROOM);

  time_now = time (0);
  date = (char *) asctime (localtime (&time_now));
  date[strlen (date) - 1] = '\0';
  sprintf (msg, "Imprisoned in cell VNUM %d until wanted time expires.\n",
	   victim->in_room);
  sprintf (buf, "Jailed in %s.", zone_table[ch->room->zone].name);
  add_message (1, victim->tname, -2, "Server", date, buf, "", msg, 0);

  sprintf (msg,
	   "Imprisoned in cell VNUM %d until wanted time expires.\n\nCheck %s playerfile notes for relevant charges.\n",
	   victim->in_room, HSHR (victim));
  sprintf (buf, "#1Jailed:#0 %s", victim->tname);
  add_message (1, "Prisoners", -5, "Server", date, buf, "", msg, 0);

}

int
has_weapon (CHAR_DATA * ch)
{
  OBJ_DATA *obj;

  if ((obj = get_equip (ch, WEAR_BOTH)))
    {
      if (GET_ITEM_TYPE (obj) == ITEM_WEAPON)
	if (obj->o.od.value[1] * obj->o.od.value[2] > 3)
	  return 1;
    }

  if ((obj = get_equip (ch, WEAR_PRIM)))
    {
      if (GET_ITEM_TYPE (obj) == ITEM_WEAPON)
	if (obj->o.od.value[1] * obj->o.od.value[2] > 3)
	  return 1;
    }

  if ((obj = get_equip (ch, WEAR_SEC)))
    {
      if (GET_ITEM_TYPE (obj) == ITEM_WEAPON)
	if (obj->o.od.value[1] * obj->o.od.value[2] > 3)
	  return 1;
    }

  return 0;
}

int
enforcer_weapon_check (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  int weapon_count = 0;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if (!IS_SET (ch->act, ACT_ENFORCER) || !is_area_enforcer (ch)
      || !IS_SET (ch->room->room_flags, LAWFUL))
    return 0;

  if (number (0, 1))
    {

      weapon_count = 0;

      for (tch = ch->room->people; tch; tch = tch->next_in_room)
	if (!IS_NPC (tch) && has_weapon (tch)
	    && !get_affect (tch, MAGIC_WARNED) && !is_area_enforcer (tch))
	  weapon_count++;

      if (weapon_count)
	{
	  weapon_count = number (1, weapon_count);

	  for (tch = ch->room->people;; tch = tch->next_in_room)
	    {

	      if (!IS_NPC (tch) && has_weapon (tch)
		  && !get_affect (tch, MAGIC_WARNED)
		  && !is_area_enforcer (tch))
		weapon_count--;

	      if (weapon_count <= 0)
		break;
	    }

	  /* Prevent a guard for warning him for a bit */

	  magic_add_affect (tch, MAGIC_WARNED, 90, 0, 0, 0, 0);

	  name_to_ident (tch, buf);

	  if (ch->room->zone == 5 || ch->room->zone == 6)
	    sprintf (buf2,
		     "%s (with a menacing scowl) Don't let me see that weapon out again, maggot.",
		     buf);
	  else
	    sprintf (buf2,
		     "%s (frowning) You'll need to stow that weapon here, citizen.",
		     buf);

	  do_tell (ch, buf2, 0);

	  return 1;
	}
    }

  return 0;
}

int
enforcer_hood_check (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];
  int hood_count;

  if (!IS_SET (ch->act, ACT_ENFORCER) || !is_area_enforcer (ch))
    return 0;

  if (number (0, 1))
    {

      hood_count = 0;

      for (tch = ch->room->people; tch; tch = tch->next_in_room)
	if (is_hooded (tch) && !get_affect (tch, MAGIC_STARED)
	    && !is_area_enforcer (tch))
	  hood_count++;

      if (hood_count)
	{
	  hood_count = number (1, hood_count);

	  for (tch = ch->room->people;; tch = tch->next_in_room)
	    {

	      if (is_hooded (tch) && !get_affect (tch, MAGIC_STARED)
		  && !is_area_enforcer (tch))
		hood_count--;

	      if (hood_count <= 0)
		break;
	    }

	  /* Prevent a guard for studying him for a second */

	  magic_add_affect (tch, MAGIC_STARED, 10, 0, 0, 0, 0);

	  name_to_ident (tch, buf);

	  do_study (ch, buf, 0);

	  return 1;
	}
    }

  return 0;
}

int
mob_weather_reaction (CHAR_DATA * ch)
{
  AFFECTED_TYPE *af;

  if (!get_equip (ch, WEAR_ABOUT))
    return 0;

  if (!IS_SET (get_equip (ch, WEAR_ABOUT)->obj_flags.extra_flags, ITEM_MASK))
    return 0;

  af = get_affect (ch, MAGIC_RAISED_HOOD);

  if (ch->room->sector_type == SECT_INSIDE
      || weather_info[ch->room->zone].state <= CHANCE_RAIN)
    {
      if (af && is_hooded (ch))
	{
	  affect_remove (ch, af);
	  do_hood (ch, "", 0);
	  return 1;
	}
      else
	return 0;
    }

  if (get_affect (ch, MAGIC_RAISED_HOOD))
    return 0;

  if (!is_hooded (ch) && ch->room->sector_type != SECT_INSIDE
      && weather_info[ch->room->zone].state > CHANCE_RAIN)
    {
      if (!af)
	{
	  CREATE (af, AFFECTED_TYPE, 1);
	  af->type = MAGIC_RAISED_HOOD;
	  affect_to_char (ch, af);
	}
      do_hood (ch, "", 0);
      return 1;
    }

  return 0;
}

int
miscellaneous_routine (CHAR_DATA * ch)
{
  if (enforcer_weapon_check (ch))
    return 1;

  if (enforcer_hood_check (ch))
    return 1;

  return 0;
}

int
combat_routine (CHAR_DATA * ch)
{
  return 0;
}

void
add_attacker (CHAR_DATA * victim, CHAR_DATA * threat)
{
  ATTACKER_DATA *tmp_att, *tmp_next;
  CHAR_DATA *tch;
  int i = 0;

  if (!IS_NPC (victim))
    return;

  if (!victim->attackers)
    {
      CREATE (victim->attackers, ATTACKER_DATA, 1);
      victim->attackers->attacker = threat;
      victim->attackers->next = NULL;
    }
  else
    {
      CREATE (tmp_att, ATTACKER_DATA, 1);
      tmp_att->next = victim->attackers;
      victim->attackers = tmp_att;
    }

  for (tmp_att = victim->attackers, i = 1; tmp_att; tmp_att = tmp_next, i++)
    {
      tmp_next = tmp_att->next;
      tch = tmp_att->attacker;
      if (i > 4 || !tch)
	remove_attacker (victim, tmp_att->attacker);
    }
}

void
add_threat (CHAR_DATA * victim, CHAR_DATA * threat, int amount)
{
  THREAT_DATA *tmp;

  if (!IS_NPC (victim))
    return;

  if (!victim->threats)
    {
      CREATE (victim->threats, THREAT_DATA, 1);
      victim->threats->source = threat;
      victim->threats->level = amount;
      victim->threats->next = NULL;
    }
  else
    {
      CREATE (tmp, THREAT_DATA, 1);
      tmp->next = victim->threats;
      victim->threats = tmp;
    }

  add_attacker (victim, threat);
}

int
npc_charge (CHAR_DATA * ch, CHAR_DATA * archer)
{
  int dir = 0, num = 0, i = 0;
  ROOM_DATA *room;
  char buf[MAX_STRING_LENGTH];

  if (IS_SET (ch->act, ACT_AGGRESSIVE))
    ch->flags |= FLAG_KILL;

  if (ch->in_room == archer->in_room)
    {
      ch->speed = 0;
      name_to_ident (archer, buf);
      if (could_attack (ch, archer))
	{
	  set_fighting (ch, archer);
	}
      else
	{
	  add_threat (ch, archer, 7);
	}
      return 1;
    }

  ch->speed = 4;

  dir = track (ch, archer->in_room);

  if (dir == -1)
    return -1;

  if (!EXIT (ch, dir))
    return 0;

  room = vtor (ch->room->dir_option[dir]->to_room);
  num = 1;

  while (room->nVirtual != archer->in_room)
    {
      if (!room->dir_option[dir])
	return -1;
      room = vtor (room->dir_option[dir]->to_room);
      num++;
    }

  for (i = 1; i <= num; i++)
    do_move (ch, "", dir);

  add_attacker (ch, archer);
  add_threat (ch, archer, 7);

  return 1;
}

int
morale_broken (CHAR_DATA * ch)
{
  WOUND_DATA *wound;
  float damage = 0, limit = 0;
  bool morale_broken = false, morale_held = true;

  if (ch->in_room == 5142 || ch->in_room == 5119 || ch->in_room == 5196 ||
      ch->in_room == 5197 || ch->in_room == 5198 || ch->in_room == 5199)
    return 0;			/* Fight to the death in the Arena. */

  if (ch->in_room == 66953 || ch->in_room == 66954 || ch->in_room == 66955 || ch->in_room == 66956)
    return 0;			/* Fight to the death in the Pit. */

  if (IS_SET (ch->act, ACT_WIMPY) || IS_SET (ch->act, ACT_PREY))
    return 1;

  for (wound = ch->wounds; wound; wound = wound->next)
    {
      if (wound)
	damage += wound->damage;
    }

  damage += ch->damage;


  limit = ch->max_hit * .25;

  if (damage < limit && IS_SET (ch->plr_flags, MORALE_BROKEN))
    ch->plr_flags &= ~MORALE_BROKEN;

  if (IS_SET (ch->plr_flags, MORALE_BROKEN))
    return 1;

  if (IS_SET (ch->plr_flags, MORALE_HELD))
    return 0;

  if (IS_SET (ch->act, ACT_ENFORCER) && IS_SET (ch->act, ACT_SENTINEL))
    {				/* Mob flagged as both re-evaluates at 15%. */
      if (damage > (limit = ch->max_hit * .85))
	{
	  if (number (1, 25) > ch->wil)
	    {
	      morale_broken = true;
	    }
	}
      else
	morale_held = true;
    }

  if (IS_SET (ch->act, ACT_ENFORCER) || IS_SET (ch->act, ACT_SENTINEL))
    {				/* Mob flagged as one re-evaluates at 20%. */
      if (damage > (limit = ch->max_hit * .8))
	{
	  if (number (1, 25) > ch->wil)
	    {
	      morale_broken = true;
	    }
	}
      else
	morale_held = true;
    }

  if (IS_SET (ch->act, ACT_AGGRESSIVE))
    {				/* Aggro mobs cop out at 25%. */
      if (damage > (limit = ch->max_hit * .75))
	{
	  if (number (1, 25) > ch->wil)
	    {
	      morale_broken = true;
	    }
	}
      else
	morale_held = true;
    }

  if (damage > (limit = ch->max_hit * .60))
    {				/* All others reconsider at 40%. */
      if (number (1, 25) > ch->wil)
	{
	  morale_broken = true;
	}
      else
	morale_held = true;
    }

  if (morale_broken)
    {
      if (IS_SET (ch->act, ACT_MOUNT) && ch->mount)
	{

	  if (skill_use (ch->mount, SKILL_RIDE, 15))
	    morale_held = true;

	  /* * * * *  Japheth's change  * * * * * *

	     if (ch->race == lookup_race_variable (lookup_race_id ("warhorse"), RACE_NAME) && skill_use (ch->mount, SKILL_RIDE, -20) )
	     morale_held = true;
	     else if (ch->race == lookup_race_variable (lookup_race_id ("warg"), RACE_NAME) && skill_use (ch->mount, SKILL_RIDE, -10) ) 
	     morale_held = true;
	     else if (skill_use (ch->mount, SKILL_RIDE, 20) )
	     morale_held = true;

	     if (morale_held && IS_NPC (ch->mount)) {
	     return morale_broken (ch->mount);
	     }

	     * * * * * * * * * * * * * * * * * * * * */

	}
      else
	{
	  ch->plr_flags |= MORALE_BROKEN;
	  return 1;
	}
    }

  if (morale_held)
    {
      ch->plr_flags |= MORALE_HELD;
      return 0;
    }

  return 0;
}

void
threat_from_char (CHAR_DATA * ch, THREAT_DATA * att)
{
  THREAT_DATA *tempatt;

  if (!ch || !att)
    return;

  if (ch->threats == att)
    ch->threats = ch->threats->next;

  else
    {
      for (tempatt = ch->threats; tempatt; tempatt = tempatt->next)
	if (tempatt->next == att)
	  tempatt->next = tempatt->next->next;
    }

  mem_free (att);
}

void
remove_threat (CHAR_DATA * victim, CHAR_DATA * threat)
{
  THREAT_DATA *tmp, *targ_threat = NULL;

  if (victim->threats && victim->threats->source == threat)
    {
      targ_threat = victim->threats;
      victim->threats = victim->threats->next;
    }
  else if (victim->threats)
    {
      for (tmp = victim->threats; tmp; tmp = tmp->next)
	{
	  if (tmp->next && tmp->next->source && tmp->next->source == threat)
	    {
	      targ_threat = tmp->next;
	      tmp->next = tmp->next->next;
	    }
	}
    }

  if (targ_threat)
    mem_free (targ_threat);
}

void
attacker_from_char (CHAR_DATA * ch, ATTACKER_DATA * att)
{
  ATTACKER_DATA *tempatt;

  if (!ch || !att)
    return;

  if (ch->attackers == att)
    ch->attackers = ch->attackers->next;

  else
    {
      for (tempatt = ch->attackers; tempatt; tempatt = tempatt->next)
	if (tempatt->next == att)
	  tempatt->next = tempatt->next->next;
    }

  mem_free (att);
}


void
remove_attacker (CHAR_DATA * victim, CHAR_DATA * threat)
{
  ATTACKER_DATA *tmp, *targ_att = NULL;

  if (victim->attackers && victim->attackers->attacker == threat)
    {
      targ_att = victim->attackers;
      victim->attackers = victim->attackers->next;
    }
  else if (victim->attackers)
    {
      for (tmp = victim->attackers; tmp; tmp = tmp->next)
	{
	  if (tmp->next && tmp->next->attacker
	      && tmp->next->attacker == threat)
	    {
	      targ_att = tmp->next;
	      tmp->next = tmp->next->next;
	    }
	}
    }

  if (targ_att)
    mem_free (targ_att);
}


/*
stack trace:
main::run_the_game::game_loop::command_interpreter::do_fire::npc_archery_retaliation
*/
void
npc_evasion (CHAR_DATA * ch, int dir)
{
  int roll = 0;

  do_stand (ch, "", 0);

  // Try to control our mounts!
  if (IS_SET (ch->act, ACT_MOUNT) 
      && ch->mount 
      && skill_use (ch->mount, SKILL_RIDE, 15))
    {
      return;
    }

  // grouped enforcers stand ground - often
  //else if (IS_SET (ch->act, ACT_ENFORCER))
    //{
      if (ch->following)
	{
//	  if (number (1,25) <= ch->wil)
//	    {
	      return;
//	    }
	}
  //    else if (is_group_leader (ch))
//	{
//	  if (number (1, 25) <= ch->wil + 4)
//	    {
//	      return;
//	    }
//	}
//    }

  roll = number (1, 3);

  do_set (ch, "run", 0);

  if (dir == -1)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else if (dir == 0)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else if (dir == 1)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else if (dir == 2)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else if (dir == 3)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else if (dir == 4)
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, DOWN))
	    do_down (ch, "", 0);
	}
    }
  else
    {
      if (roll == 1)
	{
	  if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	}
      else if (roll == 2)
	{
	  if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	}
      else if (roll == 3)
	{
	  if (CAN_GO (ch, SOUTH))
	    do_south (ch, "", 0);
	  else if (CAN_GO (ch, NORTH))
	    do_north (ch, "", 0);
	  else if (CAN_GO (ch, EAST))
	    do_east (ch, "", 0);
	  else if (CAN_GO (ch, WEST))
	    do_west (ch, "", 0);
	  else if (CAN_GO (ch, UP))
	    do_up (ch, "", 0);
	}
    }

  do_set (ch, "walk", 0);
}

OBJ_DATA *
get_bow (CHAR_DATA * ch)
{
  OBJ_DATA *obj;

  if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WEAPON)
    {
      if (ch->right_hand->o.weapon.use_skill == SKILL_LONGBOW ||
	  ch->right_hand->o.weapon.use_skill == SKILL_SHORTBOW ||
	  ch->right_hand->o.weapon.use_skill == SKILL_CROSSBOW)
	return ch->right_hand;
    }

  if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_WEAPON)
    {
      if (ch->left_hand->o.weapon.use_skill == SKILL_LONGBOW ||
	  ch->left_hand->o.weapon.use_skill == SKILL_SHORTBOW ||
	  ch->left_hand->o.weapon.use_skill == SKILL_CROSSBOW)
	return ch->left_hand;
    }

  for (obj = ch->equip; obj; obj = obj->next_content)
    {
      if (GET_ITEM_TYPE (obj) != ITEM_WEAPON)
	continue;
      if (obj->o.weapon.use_skill == SKILL_LONGBOW ||
	  obj->o.weapon.use_skill == SKILL_SHORTBOW ||
	  obj->o.weapon.use_skill == SKILL_CROSSBOW)
	return obj;
    }

  if (IS_NPC (ch))
    do_set (ch, "walk", 0);

  return NULL;
}

int
is_bow (OBJ_DATA * bow)
{
  if (bow->o.weapon.use_skill != SKILL_LONGBOW
      && bow->o.weapon.use_skill != SKILL_SHORTBOW
      && bow->o.weapon.use_skill != SKILL_CROSSBOW)
    return 0;

  return 1;
}

void
ready_melee_weapons (CHAR_DATA * ch)
{
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  unready_bow (ch);

  /* already ready? */
  if (get_equip (ch, WEAR_PRIM) || get_equip (ch, WEAR_BOTH))
    {
      return;
    }

  if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WEAPON
      && !is_bow (ch->right_hand))
    {
      one_argument (ch->right_hand->name, buf);
      do_wield (ch, buf, 0);
    }
  if (ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_WEAPON
      && !is_bow (ch->left_hand))
    {
      one_argument (ch->left_hand->name, buf);
      do_wield (ch, buf, 0);
    }
  if (ch->right_hand && GET_ITEM_TYPE (ch->right_hand) != ITEM_WEAPON)
    {
      if (GET_ITEM_TYPE (ch->right_hand) == ITEM_KEY)
	{
	  do_switch (ch, "", 0);
	}
      else
	{
	  one_argument (ch->right_hand->name, buf);
	  do_drop (ch, buf, 0);
	}
    }

  for (obj = ch->equip; obj; obj = obj->next_content)
    {
      if (GET_ITEM_TYPE (obj) == ITEM_SHEATH)
	{
	  if (obj->contains && GET_ITEM_TYPE (obj->contains) == ITEM_WEAPON
	      && !is_bow (obj->contains))
	    {
	      one_argument (obj->contains->name, buf);
	      do_draw (ch, buf, 0);
	      break;
	    }
	}
    }

  do_remove (ch, "shield", 0);

  for (obj = ch->equip; obj; obj = obj->next_content)
    {
      if (GET_ITEM_TYPE (obj) == ITEM_SHEATH)
	{
	  if (obj->contains && GET_ITEM_TYPE (obj->contains) == ITEM_WEAPON
	      && !is_bow (obj->contains))
	    {
	      one_argument (obj->contains->name, buf);
	      do_draw (ch, buf, 0);
	      break;
	    }
	}
    }
}

void
unready_weapons (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];

  if (!IS_NPC (ch) || ch->desc)
    return;

  if (ch->right_hand && (GET_ITEM_TYPE (ch->right_hand) == ITEM_WEAPON ||
			 GET_ITEM_TYPE (ch->right_hand) == ITEM_SHIELD))
    {
      one_argument (ch->right_hand->name, buf);
      do_sheathe (ch, buf, 0);
      do_wear (ch, buf, 0);
    }

  if (ch->left_hand && (GET_ITEM_TYPE (ch->left_hand) == ITEM_WEAPON ||
			GET_ITEM_TYPE (ch->left_hand) == ITEM_SHIELD))
    {
      one_argument (ch->left_hand->name, buf);
      do_sheathe (ch, buf, 0);
      do_wear (ch, buf, 0);
    }
}

void
unready_bow (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *bow;

  bow = get_equip (ch, WEAR_BOTH);

  if (!bow)
    bow = get_bow (ch);

  if (!bow)
    return;

  if (GET_ITEM_TYPE (bow) != ITEM_WEAPON)
    return;

  if (!is_bow (bow))
    return;

  do_unload (ch, "", 0);

  one_argument (bow->name, buf);

  do_wear (ch, buf, 0);
}

void
ready_bow (CHAR_DATA * ch)
{
  OBJ_DATA *bow;


/** if we don't have a bow in  our hand(s), then look for one in our inventory. If we don't have one, give up. If we do, then use it. Make certain it is loaded once we have it**/
  if (!((bow = get_equip (ch, WEAR_BOTH))
  		|| (bow = get_equip (ch, WEAR_PRIM))))
    {
      bow = get_bow (ch);

      if (!bow)
				return;

      do_remove (ch, bow->name, 0);
      do_wield (ch, bow->name, 0);
    }

  if (!bow->loaded)
    do_load (ch, "", 0);
}

int
has_ammunition (CHAR_DATA * ch)
{
  OBJ_DATA *bow, *quiver, *ammo;
  int i;

  bow = get_bow (ch);

  if (!bow)
    return 0;

  for (i = 0; i < MAX_WEAR; i++)
    {
      if (!(quiver = get_equip (ch, i)))
	continue;
      if (GET_ITEM_TYPE (quiver) != ITEM_QUIVER)
	continue;
      for (ammo = quiver->contains; ammo; ammo = ammo->next_content)
	{
	  if (bow->o.weapon.use_skill == SKILL_LONGBOW
	      || bow->o.weapon.use_skill == SKILL_SHORTBOW)
	    {
	      if (!strn_cmp (ammo->name, "arrow", 5))
		{
		  return 1;
		}
	    }
	  if (bow->o.weapon.use_skill == SKILL_CROSSBOW)
	    {
	      if (!strn_cmp (ammo->name, "bolt", 4))
		{
		  return 1;
		}
	    }
	}
    }

  return 0;
}

int
is_archer (CHAR_DATA * ch)
{
  OBJ_DATA *bow;

  bow = get_bow (ch);

  if (!bow)
    return 0;

  if (!has_ammunition (ch))
    return 0;

  if (!ch->skills[bow->o.weapon.use_skill])
    return 0;

  if (bow->o.od.value[1] == 0)
    return 3;			/* Longbow; range of 3. */

  if (bow->o.od.value[1] == 1 || bow->o.od.value[1] == 2)
    return 2;			/* Shortbow and crossbow. */

  else
    return 1;			/* Sling, eventually. */
}

CHAR_DATA *
acquire_archer_target (CHAR_DATA * ch, int i)
{
  CHAR_DATA *tch;
  ROOM_DATA *troom;
  int j = 0, range = 0;

  if (!is_archer (ch))
    return NULL;

  troom = ch->room;

  if (!troom->dir_option[i])
    return NULL;

  if (!(troom = vtor (troom->dir_option[i]->to_room)))
    return NULL;

  if ((range = is_archer (ch)))
    {
      for (j = 1; j <= range; j++)
	{
	  if (IS_SET (troom->room_flags, INDOORS))
	    continue;
	  if (IS_SET (troom->room_flags, STIFLING_FOG))
	    return NULL;
	  for (tch = troom->people; tch; tch = tch->next_in_room)
	    {
	      if (ch->room == tch->room && CAN_SEE (tch, ch)
		  && CAN_SEE (ch, tch))
		continue;
	      if (would_attack (ch, tch) || would_attack (tch, ch))
		{
		  if (!CAN_SEE (ch, tch))
		    continue;
		  if (!skill_use (ch, SKILL_SCAN, 0))
		    continue;
		  if (IS_SUBDUEE (tch)
		      && (!IS_SET (ch->act, ACT_AGGRESSIVE)
			  || is_brother (tch->subdue, ch)))
		    continue;
		  if (tch->fighting)
		    {
		      if (is_brother (ch, tch->fighting)
			  || (!would_attack (ch, tch->fighting)
			      && !would_attack (tch->fighting, ch)))
			continue;
		    }
		  return tch;
		}
	    }
	  if (!troom->dir_option[i])
	    break;
	  troom = vtor (troom->dir_option[i]->to_room);
	}
    }
  else
    return NULL;

  return NULL;
}

CHAR_DATA *
acquire_distant_target (CHAR_DATA * ch, int i)
{
  CHAR_DATA *tch;
  ROOM_DATA *troom;
  int j = 0, penalty = 0;

  troom = ch->room;

  if (IS_SET (troom->room_flags, STIFLING_FOG))
    return NULL;

  if (!troom->dir_option[i])
    return NULL;
  if (IS_SET (troom->dir_option[i]->exit_info, EX_CLOSED))
    return NULL;

  troom = vtor (troom->dir_option[i]->to_room);
  if (!troom || IS_SET (troom->room_flags, NO_MOB))
    return NULL;
  if ((ch->mob->noaccess_flags & troom->room_flags))
    return NULL;

  for (j = 1; j <= 3; j++)
    {
      if (IS_SET (troom->room_flags, STIFLING_FOG))
	return NULL;
      for (tch = troom->people; tch; tch = tch->next_in_room)
	{
	  if (j == 2)
	    penalty = 10;
	  else if (j == 3)
	    penalty = 15;
	  if (would_attack (ch, tch))
	    {
	      if (!CAN_SEE (ch, tch))
		continue;
	      if (!skill_use (ch, SKILL_SCAN, penalty))
		continue;
	      if (IS_SUBDUEE (tch)
		  && (!IS_SET (ch->act, ACT_AGGRESSIVE)
		      || is_brother (tch->subdue, ch)))
		continue;
	      return tch;
	    }
	}
      if (!troom->dir_option[i])
	break;
      if (IS_SET (troom->dir_option[i]->exit_info, EX_CLOSED))
	break;
      troom = vtor (troom->dir_option[i]->to_room);
      if (!troom || IS_SET (troom->room_flags, NO_MOB))
	return NULL;
      if ((ch->mob->noaccess_flags & troom->room_flags))
	return NULL;
    }

  return NULL;
}

int
evaluate_threats (CHAR_DATA * ch)
{
  THREAT_DATA *tmp;
  ATTACKER_DATA *tmp_att;
  WOUND_DATA *wound;
  CHAR_DATA *tch;
  int i = 0;
  int dir = 0;
  float damage = 0;
  bool safe = true;
  bool bleeding = false;

  if (ch->fighting)
    return 0;

  for (tmp = ch->threats; tmp; tmp = tmp->next)
    {
      if (tmp->level <= 0 || !tmp->source)
	{
	  remove_threat (ch, tmp->source);
	  return 1;
	}

      dir = track (ch, tmp->source->in_room);
      if (morale_broken (ch) && CAN_SEE (tmp->source, ch))
	{
	  if (GET_POS (ch) != POSITION_STANDING)
	    do_stand (ch, "", 0);
	  npc_evasion (ch, dir);

	  if (tmp->source->in_room != ch->in_room)
				{
				if (ch->delay <= 0 && IS_SET (ch->affected_by, AFF_HIDE)
						&& !get_affect (ch, MAGIC_HIDDEN))
					{
					do_hide (ch, "", 0);
					return 1;
					}

	      tmp->level -= 1;
	    }

			return 1;
			}
		} // for (tmp = ch->threats

  if (ch->delay <= 0 && IS_SET (ch->affected_by, AFF_HIDE)
      && !get_affect (ch, MAGIC_HIDDEN))
    {
      do_hide (ch, "", 0);
      return 1;
    }

  for (tmp_att = ch->attackers; tmp_att; tmp_att = tmp_att->next)
    {

      if (tmp_att
	  && tmp_att->attacker
	  && tmp_att->attacker->in_room
	  && ch->in_room == tmp_att->attacker->in_room
	  && CAN_SEE (ch, tmp_att->attacker))
	{
	  if (!GET_FLAG (tmp_att->attacker, FLAG_SUBDUEE)
	      && GET_POS (tmp_att->attacker) != POSITION_UNCONSCIOUS)
	    {
				safe = false;
				}

			if (morale_broken (ch))
				{
				if (GET_POS (ch) != POSITION_STANDING)
					{
					do_stand (ch, "", 0);
					}
	      npc_evasion (ch, tmp_att->attacker->from_dir);

	      if (tmp_att->attacker->in_room != ch->in_room)
		{
					if (ch->delay <= 0 && IS_SET (ch->affected_by, AFF_HIDE)
							&& !get_affect (ch, MAGIC_HIDDEN))
						{
		      do_hide (ch, "", 0);
		      return 1;
		    }
		}
	      return 1;

	      /* FIXME 
	         for ( i = 0; i <= 6; i++ ) {
	         if ( CAN_GO (ch, i) ) {
	         do_move (ch, "", i);
	         return 1;
	         }
	         } */
	    	}//if (morale_broken (ch))
			}//if (tmp_att
			
      for (i = 0; i <= LAST_DIR; i++)
	{
	  if (EXIT (ch, i) && CAN_SEE (ch, tmp_att->attacker)
	      && EXIT (ch, i)->to_room == tmp_att->attacker->in_room)
	    safe = false;
	}
    }

  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
      if (would_attack (ch, tch) || would_attack (tch, ch))
	safe = false;
    }

  for (wound = ch->wounds; wound; wound = wound->next)
    {
      if (wound->bleeding)
	bleeding = true;
      damage += wound->damage;
    }

  damage += ch->damage;

  if (safe && bleeding && !IS_SET (ch->act, ACT_NOBIND))
    {
      char__do_bind (ch, "", 0);
      return 1;
    }

  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
      if (!is_brother (ch, tch))
	continue;
		
      for (wound = tch->wounds; wound; wound = wound->next)
			if (wound->bleeding)
				bleeding = true;

      if (bleeding && !AWAKE (tch))
	{
	  char__do_bind (ch, tch->tname, 0);
	  return 1;
	}
    }

  return 0;
}

int
survival_routine (CHAR_DATA * ch)
{
  if (evaluate_threats (ch))
    return 1;
  else
    return 0;
}

int
predatory_mobile (CHAR_DATA * ch)
{
  ROOM_DATA *troom;
  TRACK_DATA *track;

  if (!ch->room->tracks)
    return 0;

  if (is_dark (ch->room) && IS_MORTAL (ch) &&
      !get_affect (ch, MAGIC_AFFECT_INFRAVISION) &&
      !IS_SET (ch->affected_by, AFF_INFRAVIS))
    {
      return 0;
    }

  for (track = ch->room->tracks; track; track = track->next)
    {
      if (track->hours_passed > 0)
	continue;
      if (!skill_use (ch, SKILL_TRACKING, 0))
	continue;
      if (!ch->room->dir_option[track->to_dir])
	continue;
      if (!(troom = vtor (ch->room->dir_option[track->to_dir]->to_room)))
	continue;
      if ((ch->mob->noaccess_flags & troom->room_flags))
	return 0;
      if (IS_SET (troom->room_flags, NO_MOB))
	continue;
      if (track->to_dir == ch->from_dir)
	continue;
      if (track->race >= 0 && track->race <= 29)
	{
	  ch->speed = MIN (5, track->speed + 2);
	  do_move (ch, "", track->to_dir);
	  return 1;
	}
    }

  return 0;
}


int
target_acquisition (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  OBJ_DATA *bow;
  char buf[MAX_STRING_LENGTH];
  int i = 0;

  if (ch->fighting)
    return 1;

  if (ch->aiming_at)
    return 1;

  i = 1;
  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
      if (morale_broken (ch))
	break;
      if (!CAN_SEE (ch, tch))
	continue;
      if (IS_SUBDUEE (tch))
	continue;
      if (would_attack (ch, tch))
	{
	  if (IS_SET (ch->act, ACT_AGGRESSIVE) || AWAKE (tch))
	    {
	      add_threat (ch, tch, 3);
	      if (could_attack (ch, tch) && number (0,i++))
		{
		  if (IS_SET (ch->act, ACT_AGGRESSIVE))
		    ch->flags |= FLAG_KILL;
		  set_fighting (ch, tch);
		  return 1;
		}
	    }
	}
      if (tch->fighting && !tch->fighting->deleted && is_brother (ch, tch)
	  && IS_SET (ch->act, ACT_ENFORCER)
	  && !is_brother (ch, tch->fighting)
	  && could_attack (ch, tch->fighting)
	  && number (0, 1))
	{
	  if (IS_SET (ch->act, ACT_AGGRESSIVE) || AWAKE (tch))
	    {
	      add_threat (ch, tch->fighting, 3);
	      if (could_attack (ch, tch->fighting) && number (0,i++))
		{
		  if (IS_SET (ch->act, ACT_AGGRESSIVE))
		    ch->flags |= FLAG_KILL;
		  set_fighting (ch, tch->fighting);
		  return 1;
		}
	    }
	}
    }

  i = 0;
  if (is_archer (ch))
    {
      for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
	  if (would_attack (ch, tch) || would_attack (tch, ch))
	    {
	      if (CAN_SEE (ch, tch))
		{
		  if (IS_SUBDUEE (tch)
		      && (!IS_SET (ch->act, ACT_AGGRESSIVE)
			  || is_brother (tch->subdue, ch)))
		    continue;
		  if (is_area_enforcer (ch) && is_wanted_in_area (tch))	/* Enforcers should rush in with melee weapons to subdue */
		    continue;	/* criminals, not shoot them full of arrows. */
		  if (tch->fighting)
		    {
		      if (is_brother (ch, tch)
			  || is_brother (ch, tch->fighting))
			continue;
		    }
		  break;
		}
	    }
	}
      if (!tch)
	{
	  for (i = 0; i <= LAST_DIR; i++)
	    if ((tch = acquire_archer_target (ch, i)))
	      break;
	  if (is_area_enforcer (ch) && is_wanted_in_area (tch))
	    tch = NULL;
	}
      if (tch)
	{

	  // hook: cue_on_scan reflex
	  typedef std::multimap<mob_cue,std::string>::const_iterator N;
	  std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_scan);
	  for (N n = range.first; n != range.second; n++)
	    {
	      std::string cue = n->second;
	      if (!cue.empty ())
		{
		  strcpy (buf, cue.c_str ());
		  command_interpreter (ch, buf);
		}
	    }

	  // Resume Default behavior
	  bow = get_bow (ch);
	  if (bow && bow->loaded)
	    {
	      *buf = '\0';
	      if (tch->in_room != ch->in_room)
		{
		  if (!IS_NPC (tch))
		    sprintf (buf, "%s %s", dirs[i], tch->tname);
		  else
		    sprintf (buf, "%s %s", dirs[i], tch->name);
		}
	      else
		sprintf (buf, "%s", tch->tname);

	      ready_bow (ch);
	      do_aim (ch, buf, 0);
	      return 1;
	    }
	  else if (bow && !bow->loaded)
	    {
	      // hook: cue_on_scan reflex
	      if (ch->mob->cues)
		{
		  typedef std::multimap<mob_cue,std::string>::const_iterator N;
		  std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_scan);
		  for (N n = range.first; n != range.second; n++)
		    {
		      std::string cue = n->second;
		      if (!cue.empty ())
			{
			  strcpy (buf, cue.c_str ());
			  command_interpreter (ch, buf);
			}
		    }
		}
	      ready_bow (ch);
	      return 1;
	    }
	}
    }
  if ((IS_SET (ch->act, ACT_AGGRESSIVE) || IS_SET (ch->act, ACT_ENFORCER))
      && !IS_SET (ch->act, ACT_SENTINEL) && !morale_broken (ch))
    {
      for (i = 0; i <= LAST_DIR; i++)
	{
	  if ((tch = acquire_distant_target (ch, i)))
	    {
	      sprintf (buf, "%d", tch->in_room);
	      ch->speed = tch->speed + 2;
	      ch->speed = MIN (ch->speed, 4);
	      do_track (ch, buf, 0);
	      return 1;
	    }
	}
    }
  if (IS_SET (ch->act, ACT_AGGRESSIVE) && !IS_SET (ch->act, ACT_SENTINEL)
      && real_skill (ch, SKILL_TRACKING) && !morale_broken (ch))
    {
      if (predatory_mobile (ch))
	return 1;
    }

  return 0;
}

void
mobile_routines (int pulse)
{
  int zone, dir = 0, jailer = 0;
  CHAR_DATA *ch, *tch;
  ROOM_DATA *room;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  for (ch = character_list; ch; ch = ch->next)
    {

      if (ch->deleted)
	continue;

      if (IS_SET (ch->flags, FLAG_BINDING))
	continue;

      if (ch->desc)
	continue;

      if (!ch->room)
	continue;

      if (ch->alarm > 0)
	{
	  ch->alarm -= 2;
	  if (ch->alarm <= 0 && !trigger (ch, "", TRIG_ALARM))
	    continue;
	}

      if (ch->trigger_delay)
	continue;

      if (GET_FLAG (ch, FLAG_LEAVING))
	continue;

      /* close the door of the room we are coming from */
      if (GET_FLAG (ch, FLAG_ENTERING))
	{

	  if (EXIT (ch, ch->from_dir) &&
	      (EXIT (ch, ch->from_dir)->exit_info == EX_ISDOOR ||
	      EXIT (ch, ch->from_dir)->exit_info == EX_ISGATE) &&
	      !ch->following && ch->race <= 29)
	    {

	      one_argument (EXIT (ch, ch->from_dir)->keyword, buf2);
	      sprintf (buf, "%s %s", buf2, dirs[ch->from_dir]);
	      do_close (ch, buf, 0);

	    }

	  continue;
	}

      if (ch->mount && (GET_FLAG (ch->mount, FLAG_ENTERING)
			|| GET_FLAG (ch->mount, FLAG_LEAVING)))
	continue;

      if (GET_FLAG (ch, FLAG_FLEE))
	continue;

      if (ch->delay)
	continue;

      if (ch->aiming_at)
	continue;

      if (IS_SUBDUEE (ch))
	continue;

      room = ch->room;
      zone = room->zone;

      if (!IS_NPC (ch) || IS_FROZEN (zone) || !AWAKE (ch))
	continue;

      if (!trigger (ch, "", TRIG_MOBACT))
	continue;

      for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
	  if (enforcer (ch, tch, 1, 0) > 0)
	    continue;
	}

      if (IS_SET (ch->act, ACT_JAILER) && IS_SUBDUER (ch))
	{
	  for (dir = 0; dir <= LAST_DIR; dir++)
	    {
	      if (ch->room->dir_option[dir])
		{
		  if (ch->room->dir_option[dir]->to_room == ch->cell_1
		      || ch->room->dir_option[dir]->to_room == ch->cell_2
		      || ch->room->dir_option[dir]->to_room == ch->cell_3)
		    {
		      jailer = 1;
		    }
		  if (jailer)
		    jailer_func (ch);
		}
	    }
	}

      if (IS_SUBDUER (ch))
	{
	  common_guard (ch);
	  continue;
	}

      if (survival_routine (ch))
	continue;
      if (target_acquisition (ch))
	continue;
      if (miscellaneous_routine (ch))
	continue;

      if ((rand () % 40 == 0) && mob_weather_reaction (ch))
	continue;

      if (!((pulse + 1) % PULSE_SMART_MOBS) && mob_wander (ch))
	continue;

    }				/* for */

}

int
is_threat (CHAR_DATA * ch, CHAR_DATA * tch)
{
  THREAT_DATA *tmp;
  ATTACKER_DATA *tmp_att;

  for (tmp = ch->threats; tmp; tmp = tmp->next)
    if (tmp->source == tch)
      return 1;

  for (tmp_att = ch->attackers; tmp_att; tmp_att = tmp_att->next)
    if (tmp_att->attacker == tch)
      return 1;

  return 0;
}

int
would_attack (CHAR_DATA * ch, CHAR_DATA * tch)
{
  if (ch == tch)
    return 0;

  if (IS_SET (tch->act, ACT_VEHICLE))
    return 0;

  if (IS_NPC (ch) &&
      IS_NPC (tch) &&
      IS_SET (ch->act, ACT_WILDLIFE) && IS_SET (tch->act, ACT_WILDLIFE))
    return 0;

  /* Wrestling */

  if (tch->fighting && GET_FLAG (tch, FLAG_SUBDUING))
    return 0;

  if (IS_SUBDUEE (tch))
    return 0;

  if (IS_SET (ch->act, ACT_ENFORCER))
    {
      if (!is_area_enforcer (ch))
	;
      else if (IS_SUBDUEE (tch))
	return 0;
      else if (!is_hooded (tch)
	       && get_affect (tch, MAGIC_CRIM_BASE + ch->room->zone))
	return 1;
      else if (is_hooded (tch)
	       && get_affect (tch, MAGIC_CRIM_HOODED + ch->room->zone))
	return 1;
      else if (ch->mob->zone == 1)
	{
	  if (get_affect (tch, MAGIC_CRIM_HOODED + 3))
	    return 1;
	  if (!is_hooded (tch) && get_affect (tch, MAGIC_CRIM_BASE + 3))
	    return 1;
	}
    }

  if (CAN_SEE (ch, tch) && IS_SET (ch->act, ACT_AGGRESSIVE) &&
      (!IS_SET (ch->act, ACT_WIMPY) || !AWAKE (tch)) && !is_brother (ch, tch))
    return 1;

  if (is_threat (ch, tch))
    return 1;

  return 0;
}

void
do_would (CHAR_DATA * ch, char *argument, int cmd)
{
  CHAR_DATA *tch;
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, buf);

  if (!*buf || *buf == '?')
    {
      send_to_char ("Would determines if a mob would attack you.\n", ch);
      return;
    }

  if ((tch = get_char_room (buf, ch->in_room)))
    {
      if (would_attack (tch, ch))
	act ("$n would attack $N.", false, tch, 0, ch, TO_VICT);
      if (would_attack (ch, tch))
	act ("You would attack $N.", false, ch, 0, tch, TO_CHAR);
    }
  else
    send_to_char ("Couldn't find that mob.\n", ch);
}

void
activate_resets (CHAR_DATA * ch)
{
}
