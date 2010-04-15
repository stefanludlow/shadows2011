/*------------------------------------------------------------------------\
|  limits.c : Gain Control Module                     www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "math.h"
#include "utility.h"

int
move_gain (CHAR_DATA * ch)
{
  int gain;
  int move_rate;
  int moves_gained;

  /* Move_rate is 100 moves per 5 mins  ( * 10000 for granularity ) */

  move_rate = 10000 * GET_CON (ch) * UPDATE_PULSE / (12 * 16);

  gain = move_rate;

  switch (GET_POS (ch))
    {
    case POSITION_SLEEPING:
      gain += gain >> 1;
      break;
    case POSITION_RESTING:
      gain += gain >> 2;
      break;
    case POSITION_SITTING:
      gain += gain >> 3;
      break;
    }

  if (!ch->hunger || !ch->thirst)
    gain >>= 2;

  if (ch->move_points < 0)
    ch->move_points = 0;

  ch->move_points += gain;

  moves_gained = ch->move_points / 10000;

  ch->move_points -= moves_gained * 10000;

  if (IS_SET (ch->room->room_flags, OOC) && !IS_SET (ch->flags, FLAG_GUEST))
    moves_gained = 0;

  return moves_gained;
}

int
sleep_needed_in_seconds (CHAR_DATA * ch)
{
  if (!ch->pc)
    return 0;

  return ch->pc->sleep_needed / 100000;
}

void
sleep_credit (CHAR_DATA * ch)
{
  /* We're aiming for straight credit for sleeping, which will
     be 5 seconds per update if UPDATE_PULSE is 5 * 4 (5 seconds) */

  if (!ch->pc || !ch->desc)
    return;

  ch->pc->sleep_needed -= (UPDATE_PULSE / 4) * 100000;

  if (ch->pc->sleep_needed < 0)
    ch->pc->sleep_needed = 0;
}

void
sleep_need (CHAR_DATA * ch)
{
  int need;

  if (!ch->pc)
    return;
  else				/* To enable sleep, get rid of this next return */
    return;

  /* 5 * 60 is RL seconds (5 RL minutes sleep need per mud day) */
  /* sleep needed is called (60 * 60 * 24) / (UPDATE_PULSE/4)
     times a mud day. */
  /* 10000 is stuck in for granularity */

  /* If sleep needed is too long, assume that staff has set it
     this way. */

  if (ch->pc->sleep_needed > 10 * 60 * 100000)
    return;

  need = 5 * 60 * 100000 * (UPDATE_PULSE / 4) / (60 * 60 * 24);

  ch->pc->sleep_needed += need;

  if (ch->pc->sleep_needed > 10 * 60 * 100000)
    ch->pc->sleep_needed = 10 * 60 * 100000;
}

void
check_linkdead (void)
{
  CHAR_DATA *tch;

  //for (tch = character_list; tch; tch = tch->next)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	tch = *tch_iterator;
      if (tch->deleted)
	continue;
      if (IS_NPC (tch) || tch->desc || !IS_MORTAL (tch) || !tch->room)
	continue;
      if (tch->pc->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time)
	{
	  do_quit (tch, "", 3);
	}
    }
}

void
check_idling (DESCRIPTOR_DATA * d)
{
  /* Unmark people who aren't really idle */

  if (d->idle && d->time_last_activity + PLAYER_IDLE_SECS > mud_time)
    {
      d->idle = 0;
      return;
    }

  if (d->original)
    return;

  if (!d->character
      && d->time_last_activity + DESCRIPTOR_DISCONNECT_SECS < mud_time
      && (d->connected <= CON_ACCOUNT_MENU
	  || d->connected == CON_PENDING_DISC))
    {
      close_socket (d);
      return;
    }

  /* Disconnect those people idle for too long */

  if (d->idle && d->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time)
    {

      if (d->character
	  && (IS_NPC (d->character) || !IS_MORTAL (d->character)))
	return;

      if (d->original)
	return;

      /* Idle PCs in the chargen process */

      if (d->character && !d->character->room)
	{
	  close_socket (d);
	}

      if (d->character && d->connected && d->character->room)
	do_quit (d->character, "", 3);

      return;
    }

  /* Warn people who are just getting to be idle */

  if (!d->idle && d->time_last_activity + PLAYER_IDLE_SECS < mud_time)
    {

      if (d->character)
	{
	  if (d->connected == CON_PLYNG)
	    SEND_TO_Q ("Your thoughts begin to drift. #2(Idle)#0\n\r", d);
	  else
	    SEND_TO_Q
	      ("\n\rYour attention is required to prevent disconnection from the server. #2(Idle)#0\n\r",
	       d);
	}

      d->idle = 1;
    }
}

void
check_idlers ()
{
  DESCRIPTOR_DATA *d, *d_next;

  for (d = descriptor_list; d; d = d_next)
    {
      d_next = d->next;
      check_idling (d);
    }
}

/* Update both PC's & NPC's and objects*/
void
point_update (void)
{
  int cycle_count;
  int roll, i;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *ch;
  CHAR_DATA *next_ch = NULL;
//  ROOM_DATA *room;
  AFFECTED_TYPE *af;
  WOUND_DATA *wound, *next_wound;
  struct time_info_data healing_time;
  struct time_info_data bled_time;
  struct time_info_data playing_time;

  //static int reduceIntox = 0;

  cycle_count = 0;

  mud_time = time (NULL);
  mud_time_str = (char *) asctime (localtime (&mud_time));
  *(mud_time_str + strlen (mud_time_str) - 1) = '\0';

  for (i = 0; i <= 99; i++)
    zone_table[i].player_in_zone = 0;

  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	ch = *tch_iterator;

      if (!ch)
	continue;

      if (ch->deleted)
	continue;

      if (!ch->room)
	continue;

      if (!IS_NPC (ch) && ch->room)
	zone_table[ch->room->zone].player_in_zone++;

    //  room = ch->room;

      //*ch->short_descr = tolower (*ch->short_descr);

//boats take damage - Disabled by Grommit - no forseeable usage
/*      if (IS_SET (ch->act, ACT_VEHICLE))
	{
	  if (room->sector_type == SECT_REEF)
	    {

	      //for (tch = character_list; tch; tch = tch->next)
		  for (std::list<char_data*>::iterator tch_iterator_two = character_list.begin(); tch_iterator_two != character_list.end(); tch_iterator_two++)
		{
		tch = *tch_iterator_two;

		  if (tch->deleted)
		    continue;

		  if (tch->vehicle == ch)
		    send_to_char
		      ("The boat shudders and you hear the sides of the "
		       "ship scrape against\n\r" "something.\n\r", tch);
		}

	      send_to_char ("Ouch!  You're scraping against the reef!\n\r",
			    ch);

	      if (weaken (ch, 10, 0, "boat in REEF"))	/* 10 hits for scraping the reef */
//		continue;
//	    }
//	}

//Sleep - disabled by  Grommit, to be used later */
/*  */    if (GET_POS (ch) >= SLEEP)
	{

	  /*if (ch->pc)
	    {

	      if ((af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)))
		{

		  if (GET_POS (ch) == STAND)
		    {
		      send_to_char ("You fall down.\n", ch);
		      act ("$n falls down.", false, ch, 0, 0, TO_ROOM);
		    }

		  else if (GET_POS (ch) == SIT)
		    {
		      send_to_char ("You collapse.\n", ch);
		      act ("$n collapses.", false, ch, 0, 0, TO_ROOM);
		    }

		  GET_POS (ch) = REST;
		}

	      if ((GET_POS (ch) == REST || GET_POS (ch) == SIT) &&
		  sleep_needed_in_seconds (ch) > 7 * 60 &&
		  IS_MORTAL (ch) && !number (0, 8) &&
		  !get_second_affect (ch, SA_STAND, NULL))
		{
		  act ("$n falls alseep.", false, ch, 0, 0, TO_ROOM);
		  GET_POS (ch) = SLEEP;
		}

	      if (sleep_needed_in_seconds (ch) == 300 && IS_MORTAL (ch))
		{
		  send_to_char ("You are feeling drowsy.\n", ch);
		  // Adding this is 1 sec.  If we don't do this, the
		     drowsy message appears every 5 seconds. //
		  ch->pc->sleep_needed += 100000;
		}

	      else if (sleep_needed_in_seconds (ch) == 350 && IS_MORTAL (ch))
		{
		  send_to_char ("You are feeling sleepy.\n", ch);
		  ch->pc->sleep_needed += 100000;
		}

	      else if (sleep_needed_in_seconds (ch) == 400 && IS_MORTAL (ch))
		{
		  send_to_char ("You are about to fall asleep.\n", ch);
		  ch->pc->sleep_needed += 100000;
		}

	      else if (GET_POS (ch) >= REST && GET_POS (ch) != FIGHT &&
		       sleep_needed_in_seconds (ch) > 6 * 60 &&
		       number (0, 25) == 25 &&
		       IS_MORTAL (ch) && ch->desc && ch->desc->original == ch)
		{
		  if (number (0, 1))
		    act ("You stifle a yawn.", false, ch, 0, 0, TO_CHAR);
		  else
		    {

		      if (get_affect (ch, MAGIC_HIDDEN) && would_reveal (ch))
			{
			  remove_affect_type (ch, MAGIC_HIDDEN);
			  act ("$n reveals $mself.", true, ch, 0, 0, TO_ROOM);
			}

		      act ("You yawn.", false, ch, 0, 0, TO_CHAR);
		      act ("$n yawns.", false, ch, 0, 0, TO_ROOM);
		    }
		}

	      if (GET_POS (ch) == SLEEP)
		sleep_credit (ch);
	      else if (IS_MORTAL (ch))
		sleep_need (ch);
	    }*/

	  if (GET_POS (ch) == POSITION_SLEEPING && ch->desc &&
	      ch->pc && ch->pc->dreams && !number (0, 5))
	    dream (ch);

	  
	}
/*end sleep code removed by grommit */

	if (GET_MOVE (ch) < GET_MAX_MOVE (ch))
	    GET_MOVE (ch) = MIN (GET_MOVE (ch) + move_gain (ch), GET_MAX_MOVE (ch));

// Healing from bloodloss
      int old_damage = ch->damage;
      int new_damage = 0;
      if (ch->damage)
		{
			// Power changes to healing - Case
			int base = BASE_PC_STANDARD_HEALING;

			if (ch->race >= 16 && ch->race <= 19 || ch->race == 93) {
				base = BASE_PC_ELF_HEALING;
			}
			else if (ch->race == 23) {
				base = BASE_PC_DWARF_HEALING;
			}
			else if (ch->race == 24 || ch->race == 25 || ch->race == 28 || ch->race == 86 ||ch->race == 29) {
				base = BASE_PC_ORCTROLL_HEALING;
			}
			// End Power changes to healing

			healing_time = real_time_passed (time (0) - ch->lastregen, 0);
			if (healing_time.minute >= (base - ch->con / 6))
			{
				roll = dice (1, 100);
				if (GET_POS (ch) == POSITION_SLEEPING)
					roll -= 20;
				if (GET_POS (ch) == POSITION_RESTING)
					roll -= 10;
				if (GET_POS (ch) == POSITION_SITTING)
					roll -= 5;
				if (roll <= (ch->con * 4))
				{
					if (roll % 5 == 0)
						ch->damage -= 2;
					else
						ch->damage -= 1;
				} 
				ch->lastregen = time (0);
			} // end if time to cure some bloodloss
	  } // end if damaged

// Stun recovery
      if (GET_POS (ch) == POSITION_STUNNED)
		{
			  if ((time (0) - ch->laststuncheck) >= number (15, 20))
				{
					ch->laststuncheck = time (0);
					GET_POS (ch) = REST;
					send_to_char ("You shake your head vigorously, recovering from your stun.\n", ch);
					sprintf (buf,
						"$n shakes $s head vigorously, seeming to recover.");
			      act (buf, false, ch, 0, 0, TO_ROOM);
					if (IS_NPC (ch))
						do_stand (ch, "", 0);
			  } // end if time to update stun
	  } // end if stunned

//regain consciousness
    if (GET_POS (ch) == POSITION_UNCONSCIOUS)
	{
	  int total_damage = get_damage_total(ch);
	  healing_time = real_time_passed (time (0) - ch->knockedout, 0);
	  if ((healing_time.minute >= 5 && ((ch->max_hit * 0.85) > total_damage)) || healing_time.minute > 10)
	    {
	      GET_POS (ch) = REST;
	      send_to_char ("Groaning groggily, you regain consciousness.\n",
			    ch);
	      sprintf (buf, "Groaning groggily, $n regains consciousness.");
	      act (buf, false, ch, 0, 0, TO_ROOM);
	      if (IS_NPC (ch))
			do_stand (ch, "", 0);
	  } // end if time to come to senses
	} // end if unconscious

//intoxication
  //    if (!ch)
//	continue;

 //     if (reduceIntox == 3)
//	{
//	  if (ch->intoxication > 0)
//	    if (!--ch->intoxication)
//	      ;
//	  /*send_to_char ("You are sober.\n", ch); */
//	  reduceIntox = 0;

//	}
  //    else
//	reduceIntox++;

//Application RPP cost
    if (!IS_NPC (ch) && ch->pc->app_cost && ch->desc)
	{
	  playing_time = real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
	  if (playing_time.hour >= 10 && ch->desc->acct)
	    {
//restoring the deduction of rpp for races -- Huan
	      ch->desc->acct->pay_application_cost (ch->pc->app_cost);
	      ch->pc->app_cost = 0;
	      save_char (ch, true);
	    }
	}

//Remove New Player Flag
    if (!IS_NPC (ch) && IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
	{
	  playing_time =
	    real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
	  if (playing_time.hour > 12)
	    {
	      ch->plr_flags &= ~NEW_PLAYER_TAG;
	      act
		("You've been playing for over 12 hours, now; the #2(new player)#0 tag on your long description has been removed. Once again, welcome - have fun, and best of luck in your travels!\n",
		 false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
	    }
	}

// Bleeding wounds and healing check
     
	// skip if they have no wounds
	if (ch->wounds)
	{
		for (wound = ch->wounds; wound; wound = next_wound)
		{
			next_wound = wound->next;
			int wound_damage = wound->damage;
			if (strcmp(wound->type, "stun"))
			{
				old_damage += (wound_damage / 2);
				new_damage += (wound_damage / 2);
			}
			else
			{
				old_damage += wound_damage;
				new_damage += wound_damage;
			}
			healing_time = real_time_passed (time (0) - wound->lasthealed, 0);
			bled_time = real_time_passed (time (0) - wound->lastbled, 0);

			/* do not bleed if binding wounds */
			if (IS_MORTAL (ch) && bled_time.minute >= BLEEDING_INTERVAL && wound->bleeding && ch->delay_type!=DEL_BIND_WOUNDS)
			{
						
				wound->lastbled = time (0);
				if (wound->bleeding > 0 && wound->bleeding <= 3)
				{
					sprintf (buf,
					"#1Blood continues to seep from a %s %s on your %s.#0",
										 wound->severity,
										 wound->name,
										 expand_wound_loc (wound->location));
									
					sprintf (buf2,
					"Blood continues to seep from a %s %s on #5%s#0's %s.",
										 wound->severity,
										 wound->name,
										 char_short (ch),
					expand_wound_loc (wound->location));
				}
				else if (wound->bleeding > 3 && wound->bleeding <= 6)
				{
					sprintf (buf, "#1Blood flows from a %s %s on your %s.#0",
									wound->severity,
									wound->name,
									expand_wound_loc (wound->location));
									
					sprintf (buf2, "Blood flows from a %s %s on #5%s#0's %s.",
									wound->severity,
									wound->name,
									char_short (ch),
					expand_wound_loc (wound->location));
				}
				else if (wound->bleeding > 6 && wound->bleeding <= 9)
				{
					sprintf (buf,
					"#1Blood flows heavily from a %s %s on your %s!#0",
										 wound->severity,
										 wound->name,
					expand_wound_loc (wound->location));
					sprintf (buf2,
					"Blood flows heavily from a %s %s on #5%s#0's %s!",
										 wound->severity,
										 wound->name,
										 char_short (ch),
					expand_wound_loc (wound->location));
				}
				else if (wound->bleeding > 9)
				{
					sprintf (buf,
						"#1Blood gushes from a %s %s on your %s!#0",
							wound->severity,
							wound->name,
							expand_wound_loc (wound->location));
									
					sprintf (buf2,
						"Blood gushes from a %s %s on #5%s#0's %s!",
							wound->severity,
							wound->name,
							char_short (ch),
							expand_wound_loc (wound->location));
				}
				act (buf, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
				act (buf2, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
        
				if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
    				act ("#6To stop the bleeding before it's too late, type BIND.#0", false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
    				
				// continue out of further updates for this char if died of bleeding
				if (general_damage (ch, wound->bleeding))
					continue;

			} // end bleeding update
      		
			// commence healing update
			// Power changes to healing - Case
			int base = BASE_PC_STANDARD_HEALING;

			if (ch->race >= 16 && ch->race <= 19 || ch->race == 93) {
				base = BASE_PC_ELF_HEALING;
			}
			else if (ch->race == 23) {
				base = BASE_PC_DWARF_HEALING;
			}
			else if (ch->race == 24 || ch->race == 25 || ch->race == 28 || ch->race == 28 || ch->race == 29) {
				base = BASE_PC_ORCTROLL_HEALING;
			}
			// End Power changes to healing

			if (healing_time.minute >= (base + (wound->damage / 3) 
				      - (GET_CON (ch) / 7) 
				      - (wound->healerskill / 20)))
			{
				wound->lasthealed = time (0);
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					if (strcmp(wound->type, "stun"))
						new_damage -= wound_damage;
					else
						new_damage -= (wound_damage / 2);

					// returns false if wound still exists
					if (!natural_healing_check (ch, wound))
					{
						if (strcmp(wound->type, "stun"))
							new_damage += wound->damage;
						else
							new_damage += (wound->damage / 2);
					}
				} //no OOC room
			} // if can be healed
		} // end wound for loop

	  // post loop cleanup
      new_damage += ch->damage; // we add this later, since bloodloss can add
      // NPCs that healed trigger on_health (>NN)
      if (IS_NPC(ch) && new_damage < old_damage && ch->mob->cues)
	{
	  // hook: cue_on_health reflex
	  // We only want ones that have the flag (>NN)
	  typedef std::multimap<mob_cue,std::string>::const_iterator N;
	  std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_health);
	  if (range.first != range.second) 
	    {
	      // Want to only execute the health cue the first time health 
	      // drops below NN
	      int old_health = 100 - (int((old_damage * 100.0) / (ch->max_hit)));
	      int new_health = 100 - (int((new_damage * 100.0) / (ch->max_hit)));
	      
	      for (N n = range.first; n != range.second; n++)
		{
		  std::string cue = n->second;
		  const char *r = cue.c_str();
		  
		  if (!cue.empty () && strncmp(r,"(>",2) == 0)
		    {
		      char *p;
		      int threshold = strtol (r+2, &p, 0);
		      if (new_health > threshold && old_health <= threshold)
			  {
			    char reflex[AVG_STRING_LENGTH] = "";
			    strcpy (reflex, p+2);
			    command_interpreter (ch, reflex);
			  }
		    }
		  }  // for loop
	  } // range.first != range.second
	 } // end is NPC
   } // end if wounded
	 
   if ((af = get_affect (ch, MAGIC_CRAFT_DELAY)))
	{
	  if (time (0) >= af->a.spell.modifier)
	    {
	      send_to_char
		("#6OOC: Your craft delay timer has expired. You may resume crafting delayed items.#0\n",
		 ch);
	      remove_affect_type (ch, MAGIC_CRAFT_DELAY);
	    }
	}

      if (ch->damage < 0)
	ch->damage = 0;

      if (ch->room && sun_light && IS_SET (ch->affected_by, AFF_SUNLIGHT_PET)
	  && !is_overcast (ch->room))
	{
	  ch->plr_flags |= FLAG_PETRIFIED;
	  die (ch);
	}

	}				/* for */
}

void
hourly_update (void)
{
  int current_time;
  int hours;
  int nomsg = 0;
  CHAR_DATA *ch, *next_ch;
  OBJ_DATA *obj;
  OBJ_DATA *objj;
  OBJ_DATA *next_thing2;
  ROOM_DATA *room;
  ENCHANTMENT_DATA *ench;
  NEGOTIATION_DATA *neg;
  NEGOTIATION_DATA *new_list = NULL;
  NEGOTIATION_DATA *tmp_neg;
  char your_buf[MAX_STRING_LENGTH];
  char room_buf[MAX_STRING_LENGTH];
  char room_msg_buf[MAX_STRING_LENGTH];

int admin_time = 0, admin_time_absoloute = 0, admin_pc_time = 0;
bool admin_found = false, admin_found_absoloute = false;

  current_time = time (NULL);

  //for (ch = character_list; ch; ch = next_ch)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	ch = *tch_iterator;

      //next_ch = ch->next;

      if (ch->deleted || !ch->room)
	continue;

if (GET_TRUST(ch) && ch->desc)
{
   if (!ch->desc->idle)
      admin_time_absoloute++;
   admin_time++;
   admin_found = true;
   admin_found_absoloute = true;
}
else if (!IS_NPC(ch) && ch->pc->level == 0 && IS_SET (ch->flags, FLAG_ISADMIN))
{
   admin_pc_time++;
   admin_found_absoloute = true;
}

      for (ench = ch->enchantments; ench; ench = ench->next)
	{
	  ench->current_hours--;
	  if (ench->current_hours <= 0)
	    remove_enchantment (ch, ench);
	}
      
      if (IS_NPC (ch))
	{
	  // Mighty Morphing
	  if ((ch->morph_time) && ch->morph_time < current_time)
	    {
	      morph_mob (ch);
	    }

	  // Delete Old Barters
	  if (IS_SET (ch->flags, FLAG_KEEPER) && ch->shop)
	    {
	      neg = ch->shop->negotiations;
	      new_list = NULL;
	      
	      while (neg)
		{
		  tmp_neg = neg->next;

		  if (neg->time_when_forgotten <= current_time)
		    {
		      mem_free (neg);
		    }
		  else
		    {
		      neg->next = new_list;
		      new_list = neg;
		    }
		  neg = tmp_neg;
		}
	      ch->shop->negotiations = new_list;
	    }

	  // hook: cue_on_hour reflex
	  if (ch->mob->cues) 
	    {
	      typedef std::multimap<mob_cue,std::string>::const_iterator N;
	      std::pair<N,N> range = ch->mob->cues->equal_range (cue_on_hour);
	      for (N n = range.first; n != range.second; n++)
		{
		  std::string cue = n->second;
		  if (!cue.empty ())
		    {
		      strcpy (your_buf, cue.c_str ());
		      command_interpreter (ch, your_buf);
		    }
		}
	    }
	}
      else 
	{
	  if (!IS_SET (ch->room->room_flags, OOC))
	    hunger_thirst_process (ch);
	}

      trigger (ch, "", TRIG_HOUR);
    }

  for (obj = object_list; obj; obj = obj->next)
    {

      if (obj->deleted)
	continue;

      /* Check for morphto first, or object can be extracted accidently before call */
      if ((obj->morphTime) && obj->morphTime < current_time)
	{
	  morph_obj (obj);
	  continue;
	}

      /* Mob jailbags need to disappear after a time */

      if (obj->nVirtual == VNUM_JAILBAG &&
	  obj->obj_timer && --obj->obj_timer == 0)
	extract_obj (obj);

      else if (IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER)
	       && --obj->obj_timer <= 0)
	{

	  room = (obj->in_room == NOWHERE ? NULL : vtor (obj->in_room));

	  if (obj->carried_by)
	    act ("$p decays in your hands.",
		 false, obj->carried_by, obj, 0, TO_CHAR);

	  else if (room && room->people)
	    {
	      act ("$p gradually decays away.",
		   true, room->people, obj, 0, TO_ROOM);
	      act ("$p gradually decays away.",
		   true, room->people, obj, 0, TO_CHAR);
	    }

	  for (objj = obj->contains; objj; objj = next_thing2)
	    {

	      next_thing2 = objj->next_content;	/* Next in inventory */

	      if (obj->nVirtual == VNUM_CORPSE)
		{
		  objj->obj_flags.extra_flags |= ITEM_TIMER;
		  objj->obj_timer = 12;	/* Stuff from corpses lasts 1 */
		  /* 3 RL hours, unless handled by a PC. */
		}

	      obj_from_obj (&objj, 0);

	      if (obj->in_obj)
		obj_to_obj (objj, obj->in_obj);
	      else if (obj->carried_by)
		obj_to_room (objj, obj->carried_by->in_room);
	      else if (obj->in_room != NOWHERE)
		obj_to_room (objj, obj->in_room);
	      else
		extract_obj (objj);
	    }

	  extract_obj (obj);
	}

      else if (GET_ITEM_TYPE (obj) == ITEM_LIGHT &&
	       obj->o.light.on && obj->o.light.hours)
	{

	  obj->o.light.hours--;

	  hours = obj->o.light.hours;

	  if (!(ch = obj->carried_by))
	    ch = obj->equiped_by;

	  switch (hours)
	    {
	    case 0:
	      strcpy (your_buf, "Your $o burns out.");
	      strcpy (room_buf, "$n's $o burns out.");
	      strcpy (room_msg_buf, "$p burns out.");
	      break;

	    case 1:
	      strcpy (your_buf, "Your $o is just a dim flicker now.");
	      strcpy (room_buf, "$n's $o is just a dim flicker now.");
	      strcpy (room_msg_buf, "$p is just a dim flicker now.");
	      break;

	    case 2:
	      strcpy (your_buf, "Your $o begins to burn low.");
	      strcpy (room_buf, "$n's $o begins to burn low.");
	      strcpy (room_msg_buf, "$p begins to burn low.");
	      break;

	    case 10:
	      strcpy (your_buf, "Your $o sputters.");
	      strcpy (room_buf, "$n's $o sputters.");
	      strcpy (room_msg_buf, "$p sputters.");
	      break;

	    default:
	      nomsg = 1;
	      break;
	    }

	  if (hours == 0 ||
	      (!is_name_in_list ("candle", obj->name) && !nomsg))
	    {

	      if (ch)
		{
		  act (your_buf, false, ch, obj, 0, TO_CHAR);
		  act (room_buf, false, ch, obj, 0, TO_ROOM);
		}

	      if (obj->in_room &&
		  (room = vtor (obj->in_room)) && room->people)
		{
		  act (room_msg_buf, false, room->people, obj, 0, TO_ROOM);
		  act (room_msg_buf, false, room->people, obj, 0, TO_CHAR);
		}
	    }

	  if (obj->o.light.hours > 0)
	    continue;

	  if (is_name_in_list ("candle", obj->name))
	    extract_obj (obj);
	}
    }
}

int
remove_room_affect (ROOM_DATA * room, int type)
{
  AFFECTED_TYPE *af;
  AFFECTED_TYPE *free_af;

  if (!room->affects)
    return 0;

  if (room->affects->type == type)
    {
      free_af = room->affects;
      room->affects = free_af->next;
      mem_free (free_af);
      return 1;
    }

  for (af = room->affects; af->next; af = af->next)
    if (af->next->type == type)
      {
	free_af = af->next;
	af->next = free_af->next;
	mem_free (free_af);
	return 1;
      }

  return 0;
}

void
room_affect_wearoff (ROOM_DATA * room, int type)
{
  if (!remove_room_affect (room, type))
    return;

  switch (type)
    {
    case MAGIC_ROOM_CALM:
      if (room)
	send_to_room
	  ("Slowly, the sense of peace dissipates, and things return to normal.\n",
	   room->nVirtual);
      else
	send_to_all ("The sense of peace everwhere fades.\n\r");
      break;

    case MAGIC_ROOM_LIGHT:
      if (room)
	send_to_room ("The unnatural light emanations fade.\n\r",
		      room->nVirtual);
      else
	send_to_all
	  ("The unnatural light emanations fade from the land.\n\r");
      break;

    case MAGIC_ROOM_DARK:
      if (room)
	send_to_room ("The unnatural darkness fades away.\n\r",
		      room->nVirtual);
      else
	send_to_all ("The unnatural darkness fades from the land.\n\r");
      break;

    case MAGIC_WORLD_SOLAR_FLARE:
      if (room)
	send_to_room ("The localized solar flare has ended.\n\r",
		      room->nVirtual);
      else
	send_outside ("The ball of flame in the sky slowly dies out.\n\r");
      break;
    }
}

void
room_update (void)
{
  /* Expire affects on rooms */
/*
	for ( room = full_room_list; room; room = room->lnext ) {
		for ( room_affect = room->affects;
			  room_affect;
			  room_affect = next_room_affect ) {

			next_room_affect = room_affect->next;

			if ( room_affect->type >= MAGIC_SMELL_FIRST &&
				 room_affect->type <= MAGIC_SMELL_LAST )
				continue;

			if ( room_affect->type == MAGIC_ROOM_FIGHT_NOISE )
				continue;

			if ( room_affect->a.room.duration > 0 )
				room_affect->a.room.duration--;

			if ( !room_affect->a.room.duration )
				room_affect_wearoff (room, room_affect->type);
		}
	}
*/
}


/* returns the situation-affected skill level */
int
skill_level (CHAR_DATA * ch, int skill, int diff_mod)
{
  int skill_lev;
  OBJ_DATA *obj;
  AFFECTED_TYPE *af;

  if (!(ch->skills[skill]))
	  return 0;

  skill_lev = ch->skills[skill];

  skill_lev -= diff_mod;

  if (ch->stun > 0)
    skill_lev -= ch->stun * 2;

  if ((af = get_affect (ch, MAGIC_AFFECT_CURSE)))
    skill_lev -= af->a.spell.modifier;

  if ((ch->race == 119 || ch->race == 120 || ch->race == 121 || ch->race == 25 || ch->race == 29  ) && sun_light
      && ch->room->sector_type != SECT_INSIDE && !IS_SET (ch->room->room_flags, INDOORS))
    {
      if (weather_info[ch->room->zone].clouds == CLEAR_SKY)
	skill_lev -= 40;
      else if (weather_info[ch->room->zone].clouds == LIGHT_CLOUDS)
	skill_lev -= 30;
      else if (weather_info[ch->room->zone].clouds == HEAVY_CLOUDS)
	skill_lev -= 15;
      else if (weather_info[ch->room->zone].clouds == OVERCAST)
	skill_lev -= 5;
    }

  for (obj = ch->equip; obj; obj = obj->next_content)
    {
      /* Why skip weapons?
         if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON || GET_ITEM_TYPE (obj) == ITEM_SHIELD )
         continue;
       */
      for (af = obj->xaffected; af; af = af->next)
	{
	  if (af->a.spell.location - 10000 == skill)
	    skill_lev += af->a.spell.modifier;
	}
    }

  if ((obj = ch->right_hand))
    {
      for (af = obj->xaffected; af; af = af->next)
	{
	  if (af->a.spell.location - 10000 == skill)
	    skill_lev += af->a.spell.modifier;
	}
    }

  if ((obj = ch->left_hand))
    {
      for (af = obj->xaffected; af; af = af->next)
	{
	  if (af->a.spell.location - 10000 == skill)
	    skill_lev += af->a.spell.modifier;
	}
    }

  skill_lev = MAX (2, skill_lev);

  return skill_lev;
}


int
skill_use (CHAR_DATA * ch, int skill, int diff_mod)
{
  double roll;
  int lv;
  int cap;
  int min, max;

  if (!real_skill (ch, skill))
    return 0;

  lv = calc_lookup (ch, REG_LV, skill);
  cap = calc_lookup (ch, REG_CAP, skill);
  int skill_level_val = skill_level (ch, skill, diff_mod);

  roll = number (1, MAX(100, skill_level_val));

  if (!AWAKE (ch))
    return 0;

  if (roll <= skill_level_val)
    return 1;

  if (IS_NPC (ch))
    {
      if (ch->skills[skill] < cap && lv >= number (1, MAX(100, skill_level_val)))
	ch->skills[skill]++;
    }

  else if (ch->pc->skills[skill] < cap && lv >= number (1, MAX(100, skill_level_val)))
    {

      if (!ch->desc || ch->desc->idle)	/* No skill gain idle/discon */
	return 0;

      if (IS_SET (ch->room->room_flags, OOC))	/* No skill gain in OOC areas. */
	return 0;

      if (!get_affect (ch, MAGIC_SKILL_GAIN_STOP + skill)
	  && !get_affect (ch, MAGIC_FLAG_NOGAIN + skill))
	{

	  ch->skills[skill]++;
	  ch->pc->skills[skill]++;

	  min = 40;

	  max = 40 + ch->skills[skill] + number (1, 60);
	  max = MIN (180, max);

	  magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill,
			    number (min, max), 0, 0, 0, 0);

	  return 2;
	}
    }

  return 0;
}
