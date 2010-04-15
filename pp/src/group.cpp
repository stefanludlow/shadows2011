#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "group.h"
#include "protos.h"

bool
is_with_group (CHAR_DATA * ch)
{
	CHAR_DATA *tch;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch)
			continue;

		if (tch->following == ch ||
			ch->following == tch ||
			(ch->following && tch->following == ch->following))
		{
			return 1;
		}
	}

	return 0;
}

void
do_follow (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *leader = NULL, *tch = NULL, *orig_leader = NULL;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Follow whom?\n", ch);
		return;
	}

	if (!(leader = get_char_room_vis (ch, buf)))
	{
		send_to_char ("There is nobody here by that name.\n", ch);
		return;
	}

	if (IS_MORTAL (ch) && leader != ch
		&& IS_SET (leader->plr_flags, GROUP_CLOSED))
	{
		send_to_char
			("That individual's group is currently closed to new followers.\n",
			ch);
		act ("$n just attempted to join your group.", true, ch, 0, leader,
			TO_VICT | _ACT_FORMAT);
		return;
	}

	if (leader != ch)
	{
		if (leader->following == ch)
		{
			send_to_char
				("You'll need to ask them to stop following you first.\n", ch);
			return;
		}
		orig_leader = leader;
		while (leader->following)
			leader = leader->following;
		if (IS_MORTAL (ch) && leader != ch
			&& IS_SET (leader->plr_flags, GROUP_CLOSED))
		{
			send_to_char
				("That individual's group is currently closed to new followers.\n",
				ch);
			act ("$n just attempted to join your group.", true, ch, 0, leader,
				TO_VICT | _ACT_FORMAT);
			return;
		}
		ch->following = leader;
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch->following == ch)
			{
				tch->following = leader;
				act ("You fall into stride with the group's new leader, $N.",
					false, tch, 0, leader, TO_CHAR | _ACT_FORMAT);
			}
		}
		if (orig_leader != leader)
		{
			if (!IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf,
				"You begin following #5%s's#0 group's current leader, $N.",
				char_short (orig_leader));
			else if (IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf,
				"You will secretly follow #5%s#0's group's current leader, $N.",
				char_short (orig_leader));
		}
		else
		{
			if (!IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf, "You begin following $N.");
			else if (IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf, "You will secretly follow $N.");
		}
		act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n falls into stride with you.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS))
			act (buf, false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);
		sprintf (buf, "$n falls into stride with $N.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS))
			act (buf, false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
		return;
	}

	if (leader == ch && ch->following && ch->following != ch)
	{
		sprintf (buf, "You will no longer follow $N.");
		act (buf, false, ch, 0, ch->following, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n is no longer following you.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS)
			&& ch->room == ch->following->room)
			act (buf, false, ch, 0, ch->following, TO_VICT | _ACT_FORMAT);
		sprintf (buf, "$n stops following $N.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS)
			&& ch->room == ch->following->room)
			act (buf, false, ch, 0, ch->following, TO_NOTVICT | _ACT_FORMAT);
		ch->following = 0;
		return;
	}

	if (leader == ch && (!ch->following || ch->following == ch))
	{
		send_to_char ("You aren't following anyone!\n", ch);
		return;
	}

}


std::string tactical_status (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	std::ostringstream status;
	int i = 0;
	CHAR_DATA *guardedChar = NULL;

	if (af = get_affect(ch, MAGIC_GUARD)) {
		guardedChar = (CHAR_DATA *) af->a.spell.t;
	}

	if ((af = get_affect (ch, AFFECT_GUARD_DIR)))
	{
		status << " #6(guarding: " << dirs[af->a.shadow.edge] << ")#0";
	}

	if (get_affect (ch, MAGIC_HIDDEN))
	{
		status << " #1(hidden)#0";
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room) {
		CHAR_DATA *tempChar = NULL;

		if ((af = get_affect (tch, MAGIC_GUARD)) && (tempChar = (CHAR_DATA *) af->a.spell.t) == ch) {
			i++;
		}

		if (guardedChar != NULL) {
			if (guardedChar == tch) {
				status << " #6(guarding: " << char_short(guardedChar) << ")#0";
			}
		}
	}

	if (i > 0)
	{
      if (i == 1)
		{
			status << " #2(guarded)#0";
		}
      else if (i > 1)
		{
			status << " #2(guarded x " << i << ")#0";
		}
	}

	i = 0;

	if (ch->fighting)
	{
		i++;
	}

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch->fighting)
		{
			continue;
		}
      if (tch->fighting == ch)
		{
			i++;
		}
	}

	if (i > 0)
   {
		if (i == 1)
		{
			status << " #1(engaged)#0";
		}
      else if (i > 1)
		{
			status << " #1(engaged x " << i << ")#0";
		}
	}

  return status.str();
}

void
do_group (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL, *top_leader = NULL;
	std::ostringstream buf;
	char arg[MAX_STRING_LENGTH];
	bool found = false;

	argument = one_argument (argument, arg);

	if (*arg)
	{
		if (is_abbrev (arg, "open"))
		{
	  		if (!IS_SET (ch->plr_flags, GROUP_CLOSED))
			{
				send_to_char ("Your group is already open!\n", ch);
				return;
			}
			ch->plr_flags &= ~GROUP_CLOSED;
			send_to_char ("You will now allow people to follow you.\n", ch);
			return;
		}
		
		else if (is_abbrev (arg, "close"))
		{
			if (IS_SET (ch->plr_flags, GROUP_CLOSED))
			{
				send_to_char ("Your group is already closed!\n", ch);
				return;
			}
			ch->plr_flags |= GROUP_CLOSED;
			send_to_char ("You will no longer allow people to follow you.\n", ch);
			return;
		}
		
		else if (is_abbrev (arg, "retreat"))
		{
			char direction_arg[AVG_STRING_LENGTH] = "";
			int direction = 0;
			argument = one_argument (argument, direction_arg);
		  
			if (!*direction_arg || (direction = index_lookup (dirs, direction_arg)) == -1 )
			{
				send_to_char ("Order a retreat in which direction?\n", ch);
			}
			else
			{
				CHAR_DATA* tch;
				bool ordered = false;
				for (tch = ch->room->people; tch; tch = tch->next_in_room)
				{
					if (tch->following == ch)
					{
						ordered = true;
						retreat (tch, direction, ch);
					}
				}
				
				if (ordered)
				{
					retreat (ch, direction, ch);
				}
				else
				{
					retreat (ch, direction, 0);
				}
			}
			return;
		}
	}

	if (!(top_leader = ch->following))
	{
		top_leader = ch;
	}

	if (!top_leader)
	{
		send_to_char ("You aren't in a group.\n", ch);
		return;
	}

	buf << "#5" << char_short (top_leader) << "#0 [" << wound_total (top_leader, false) << "]" << tactical_status (top_leader) << ", leading: " << std::endl << std::endl;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following != top_leader)
		{
			continue;
		}
		
		if (found != false)
		{
			buf << "," << std::endl;
		}
		
		buf << "   #5" << char_short (tch) << "#0 [" << wound_total (tch, false) << "]" << tactical_status (tch);
		found = true;
	}

	buf << "." << std::endl;

	if (!found)
	{
		send_to_char ("You aren't in a group.\n", ch);
		return;
	}

	send_to_char (buf.str().c_str(), ch);
}

void
followers_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time)
{
  CHAR_DATA *tch;
  ROOM_DATA *room_exit;

  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    {
      if (tch == ch || GET_FLAG (tch, FLAG_LEAVING))
	continue;

      if (tch->following != ch)
	{
	  if (!IS_RIDEE (tch) || tch->mount->following != ch)
	    continue;
	}

      if (IS_HITCHEE (tch))
	continue;

      /* Check if this mob tch is allow to go to target room */

      if (IS_NPC (tch) && CAN_GO (tch, dir) && !isguarded (ch->room, dir) &&
	  (room_exit = vtor (EXIT (tch, dir)->to_room)))
	{

	  if (IS_MERCHANT (tch) &&
	      IS_SET (room_exit->room_flags, NO_MERCHANT))
	    continue;

	  if (tch->mob->access_flags &&
	      !(tch->mob->access_flags & room_exit->room_flags))
	    continue;

	  if (IS_SET (tch->act, ACT_STAY_ZONE) &&
	      tch->room->zone != room_exit->zone)
	    continue;
	}

      if (GET_POS (tch) == SIT)
	{
	  act ("You can't follow $N while sitting.",
	       false, tch, 0, ch, TO_CHAR);
	  return;
	}

      else if (GET_POS (tch) == REST)
	{
	  act ("You can't follow $N while resting.",
	       false, tch, 0, ch, TO_CHAR);
	  return;
	}
	
	if(tch->delay_type == DEL_HIDE)
  {
	act ("You can't follow $N while looking for a place to hide.",
	       false, tch, 0, ch, TO_CHAR);
	return;
  }	

      else if (GET_POS (tch) < FIGHT)
	return;

      if (get_affect (tch, MAGIC_HIDDEN) && real_skill (tch, SKILL_SNEAK))
	{
	  if (odds_sqrt (skill_level (tch, SKILL_SNEAK, 0)) >= number (1, 100)
	      || !would_reveal (tch))
	    {
	      magic_add_affect (tch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
	    }
	  else
	    {
	      remove_affect_type (tch, MAGIC_HIDDEN);
	      act ("$n attempts to be stealthy.", true, tch, 0, 0, TO_ROOM);
	    }
	}

      move (tch, "", dir, leave_time + arrive_time);
    }
}

void
follower_catchup (CHAR_DATA * ch)
{
  CHAR_DATA *tch;
  QE_DATA *qe;

  if (!ch->room)
    return;

  for (tch = ch->room->people; tch; tch = tch->next_in_room)
    if (ch->following == tch)
      break;

  if (!tch || !GET_FLAG (tch, FLAG_LEAVING) || !CAN_SEE (tch, ch)
      || IS_SWIMMING (tch))
    return;

  for (qe = quarter_event_list; qe->ch != tch; qe = qe->next)
    ;

  if (!qe)
    return;

  if (ch->aiming_at)
    {
      send_to_char ("You lose your aim as you move.\n", ch);
      ch->aiming_at->targeted_by = NULL;
      ch->aiming_at = NULL;
      ch->aim = 0;
    }

  if (get_affect (ch, MAGIC_HIDDEN) && real_skill (ch, SKILL_SNEAK))
    {
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
    }

  move (ch, "", qe->dir, qe->event_time + qe->arrive_time);
}


// called when fleeing or when getting extracted
void
stop_followers (CHAR_DATA * ch)
{
  CHAR_DATA *tch;

  //for (tch = character_list; tch; tch = tch->next)
  for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
    {
	tch = *tch_iterator;
      if (tch->deleted)
	  continue;

      if (tch->following == ch)
	tch->following = 0;
    }
  //  if (ch->group)
  //    {
  //    ch->group->clear ();
  //  }
}

int num_followers (CHAR_DATA * ch)
{
	CHAR_DATA		*top_leader = NULL;
	CHAR_DATA		*tch = NULL;
	int group_count = 0;  
	
	if (!(top_leader = ch->following))
    top_leader = ch;
    
	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
    {
      if (tch->following != top_leader)
	continue;
      if (!CAN_SEE (ch, tch))
	continue;
     	group_count = group_count + 1;

    }

	return (group_count);

}
