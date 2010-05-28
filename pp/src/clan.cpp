/*------------------------------------------------------------------------\
|  clan.c : Clan Module                               www.middle-earth.us |
|  Copyright (C) 2005, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <cstring>

#include "server.h"
#include "net_link.h"
#include "protos.h"
#include "clan.h"
#include "utils.h"
#include "utility.h"

extern rpie::server engine;

NAME_SWITCH_DATA *clan_name_switch_list = NULL;
CLAN_DATA *clan_list = NULL;

/*
CHANGES:
14th March 2010 - Kite - Total redo of clanning for new forums.


15471	Battalion Officers		strongroom_eradan
15873	Crimson Arms of Burzumlub	crimson_arms
15297	Family of Fellowship Members	fellow_family
14156	Gondor Players			mt_citizens
15877	Gondorian Nobles			gondorian_nobles
15891	Guilds of the Outpost		outpost_guilds
14181	Hawk and Dove			hawk_dove_2
14957	Heren Carnatalion			heren_carnatalion
15897	Riverwatch				riverwatch
11723	The Battalions of Ithilien	ithilien_battalion
15296	The Battered Shield		battered_shield
15893	The Bone Face Tribe		boneface
15872	Thornhill Farm			thornhill_farm
15874	Villeins Assembly			villein_assembly
14214	Wardenry				z3_astirian_wardens
15905 Balchoth in Mordor 		balchoth_vanguard
15906 Haradrim in Mordor 		mordor_slavers
15907 Blackbloods in Mordor		vadok_kraun
15908 Caolafon				caolafon-citizen
cao_guild : 15911
cao_marsh : 15910

*/
void
clan_forum_add (CHAR_DATA * ch, char *clan, char *rank)
{
	char buf[AVG_STRING_LENGTH * 10];

	int nGroupId = -1;

	if (IS_NPC (ch))
	{
		return;
	}
	if (strcmp (clan, "caolafon-citizen") == STR_MATCH)
	{
		nGroupId = 15908;
	}
	else if (strcmp (clan, "vadok_kraun") == STR_MATCH)
	{
		nGroupId = 15907;
	}
	else if (strcmp (clan, "strongroom_eradan") == STR_MATCH)
	{
		nGroupId = 15471;
	}
	else if (strcmp (clan, "balchoth_vanguard") == STR_MATCH)
	{
		nGroupId = 15905;
	}
	else if (strcmp (clan, "crimson_arms") == STR_MATCH)
	{
		nGroupId = 15873;
	}
	else if (strcmp (clan, "fellow_family") == STR_MATCH)
	{
		nGroupId = 15297;
	}
	else if (strcmp (clan, "mt_citizens") == STR_MATCH)
	{
		nGroupId = 14156;
	}
	else if (strcmp (clan, "gondorian_nobles") == STR_MATCH)
	{
		nGroupId = 15877;
	}
	else if (strcmp (clan, "outpost_guilds") == STR_MATCH)
	{
		nGroupId = 15891;
	}
	else if (strcmp (clan, "hawk_dove_2") == STR_MATCH)
	{
		nGroupId = 14181;
	}
	else if (strcmp (clan, "heren_carnatalion") == STR_MATCH)
	{
		nGroupId = 14957;
	}
	else if (strcmp (clan, "riverwatch") == STR_MATCH)
	{
		nGroupId = 15897;
	}
	else if (strcmp (clan, "ithilien_battalion") == STR_MATCH)
	{
		nGroupId = 11723;
	}
	else if (strcmp (clan, "battered_shield") == STR_MATCH)
	{
		nGroupId = 15296;
	}
	else if (strcmp (clan, "boneface") == STR_MATCH)
	{
		nGroupId = 15893;
	}
	else if (strcmp (clan, "thornhill_farm") == STR_MATCH)
	{
		nGroupId = 15872;
	}
	else if (strcmp (clan, "villein_assembly") == STR_MATCH)
	{
		nGroupId = 15874;
	}
	else if (strcmp (clan, "z3_astirian_wardens") == STR_MATCH)
	{
		nGroupId = 14214;
	}
	else if (strcmp (clan, "outpost_citizens") == STR_MATCH)
	{
		nGroupId = 15864;
	}
	else if (strcmp (clan, "moria_dwellers") == STR_MATCH)
	{
		nGroupId = 15863;
	}
	else if (strcmp (clan, "mordor_slavers") == STR_MATCH)
	{
		nGroupId = 15906;
	}  
	else if (strcmp (clan, "cao_guild") == STR_MATCH)
	{
		nGroupId = 15911;
	}
	else if (strcmp (clan, "cao_marsh") == STR_MATCH)
	{
		nGroupId = 15910;
	}
	// Avoid giving away ImmPCs in the Forums
	if (nGroupId > 0 && !GET_TRUST (ch) && !IS_SET (ch->flags, FLAG_ISADMIN))
	{
		sprintf (buf,
			"INSERT INTO phpbb3.phpbb_user_group "
			"SELECT ( %d ), user_id, ( 0 ),( 0 ) "
			"FROM phpbb3.phpbb_users "
			"WHERE username = '%s';", nGroupId, ch->pc->account_name);
		mysql_safe_query (buf);
	}
}

void
clan_forum_remove (CHAR_DATA * ch, char *clan)
{
	char buf[AVG_STRING_LENGTH * 10];
	int nGroupId = -1;

	// Leave ImmPCs in their forums
	if (IS_NPC (ch) || GET_TRUST (ch) || IS_SET (ch->flags, FLAG_ISADMIN))
	{
		return;
	}

	if (strcmp (clan, "caolafon-citizen") == STR_MATCH)
	{
		nGroupId = 15908;
	}
	else if (strcmp (clan, "vadok_kraun") == STR_MATCH)
	{
		nGroupId = 15907;
	}
	else if (strcmp (clan, "strongroom_eradan") == STR_MATCH)
	{
		nGroupId = 15471;
	}
	else if (strcmp (clan, "balchoth_vanguard") == STR_MATCH)
	{
		nGroupId = 15905;
	}
	else if (strcmp (clan, "crimson_arms") == STR_MATCH)
	{
		nGroupId = 15873;
	}
	else if (strcmp (clan, "fellow_family") == STR_MATCH)
	{
		nGroupId = 15297;
	}
	else if (strcmp (clan, "mt_citizens") == STR_MATCH)
	{
		nGroupId = 14156;
	}
	else if (strcmp (clan, "gondorian_nobles") == STR_MATCH)
	{
		nGroupId = 15877;
	}
	else if (strcmp (clan, "outpost_guilds") == STR_MATCH)
	{
		nGroupId = 15891;
	}
	else if (strcmp (clan, "hawk_dove_2") == STR_MATCH)
	{
		nGroupId = 14181;
	}
	else if (strcmp (clan, "heren_carnatalion") == STR_MATCH)
	{
		nGroupId = 14957;
	}
	else if (strcmp (clan, "riverwatch") == STR_MATCH)
	{
		nGroupId = 15897;
	}
	else if (strcmp (clan, "ithilien_battalion") == STR_MATCH)
	{
		nGroupId = 11723;
	}
	else if (strcmp (clan, "battered_shield") == STR_MATCH)
	{
		nGroupId = 15296;
	}
	else if (strcmp (clan, "boneface") == STR_MATCH)
	{
		nGroupId = 15893;
	}
	else if (strcmp (clan, "thornhill_farm") == STR_MATCH)
	{
		nGroupId = 15872;
	}
	else if (strcmp (clan, "villein_assembly") == STR_MATCH)
	{
		nGroupId = 15874;
	}
	else if (strcmp (clan, "z3_astirian_wardens") == STR_MATCH)
	{
		nGroupId = 14214;
	}
	else if (strcmp (clan, "outpost_citizens") == STR_MATCH)
	{
		nGroupId = 15864;
	}
	else if (strcmp (clan, "moria_dwellers") == STR_MATCH)
	{
		nGroupId = 15863;
	}
	else if (strcmp (clan, "mordor_slavers") == STR_MATCH)
	{
		nGroupId = 15906;
	}  
	else if (strcmp (clan, "cao_guild") == STR_MATCH)
	{
		nGroupId = 15911;
	}
	else if (strcmp (clan, "cao_marsh") == STR_MATCH)
	{
		nGroupId = 15910;
	}
	else
	{
		return;
	}
	sprintf (buf,
		"DELETE FROM phpbb3.phpbb_user_group WHERE user_id= "
		 "(SELECT user_id FROM phpbb3.phpbb_users WHERE username = '%s')"
		 "AND group_id = %d';",
		 ch->pc->account_name, nGroupId);
	mysql_safe_query (buf);
}

void
clan_forum_remove_all (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH * 10] = "\0";
	if (!GET_TRUST (ch) && !IS_SET (ch->flags, FLAG_ISADMIN) && !IS_NPC (ch))
	{
		sprintf (buf,
			"DELETE FROM phpbb3.phpbb_user_group WHERE user_id= "
			"(SELECT user_id FROM phpbb3.phpbb_users WHERE username = '%s')"
			"AND group_id in (15910,15911,15906,15863,15864,14214,15874,15872,15893,15296,11723,15897,14957,14181,15891,15877,14156,15297,15873,15905,15471,15907,15908);", ch->pc->account_name);
		mysql_safe_query (buf);
	}
	return;
}

char *
display_clan_ranks (CHAR_DATA * ch, CHAR_DATA * observer)
{
	CLAN_DATA *clan = NULL;
	static char buf[MAX_STRING_LENGTH] = { '\0' };
	char *argument = '\0', *argument2 = '\0';
	char ranks_list[MAX_STRING_LENGTH] = { '\0' };
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	char clan_name2[MAX_STRING_LENGTH] = { '\0' };
	char name[MAX_STRING_LENGTH] = { '\0' };
	int flags = 0, flags2 = 0, clans = 0;
	bool first = true;

	argument = observer->clans;

	*ranks_list = '\0';

	sprintf (buf, "You recognize that %s carries the rank of", HSSH (ch));

	while (get_next_clan (&argument, clan_name, &flags))
	{
		argument2 = ch->clans;
		while (get_next_clan (&argument2, clan_name2, &flags2))
		{
			if (!str_cmp (clan_name, "osgi_citizens") ||
				!str_cmp (clan_name, "mordor_char") || !str_cmp (clan_name, "moria_dwellers") )
				continue;
			clan = get_clandef (clan_name);
			if (!clan)
				continue;
			if (!str_cmp (clan_name, clan_name2))
			{
				if (*ranks_list)
				{
					strcat (buf, ranks_list);
					if (!first)
						strcat (buf, ",");
					*ranks_list = '\0';
				}
				sprintf (name, "%s", clan->literal);
				if (!strn_cmp (clan->literal, "The ", 4))
					*name = tolower (*name);
				if (strn_cmp (clan->literal, "the ", 4))
					sprintf (name, "the %s", clan->literal);
				/*	        sprintf (ranks_list + strlen (ranks_list), " %s in %s",
				get_clan_rank_name (flags2), name); */
				sprintf (ranks_list + strlen (ranks_list), " %s in %s", get_clan_rank_name (ch, clan_name, flags2), name);
				first = false;
				clans++;
			}
		}
	}

	if (*ranks_list)
	{
		if (clans > 1)
			strcat (buf, " and");
		strcat (buf, ranks_list);
	}

	strcat (buf, ".");

	if (clans)
		return buf;
	else
		return "";
}


void
add_clandef (char *argument)
{
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];

	clan = new CLAN_DATA;

	argument = one_argument (argument, buf);
	clan->name = duplicateString (buf);

	argument = one_argument (argument, buf);
	clan->zone = atoi (buf);

	argument = one_argument (argument, buf);
	clan->literal = duplicateString (buf);

	argument = one_argument (argument, buf);
	clan->member_vnum = atoi (buf);

	argument = one_argument (argument, buf);
	clan->leader_vnum = atoi (buf);

	argument = one_argument (argument, buf);
	clan->omni_vnum = atoi (buf);

	clan->next = clan_list;
	clan_list = clan;
}

/* moved to protos.h - int is_brother (CHAR_DATA * ch, CHAR_DATA * tch)
{
int flags;
char *c1;
char clan_name[MAX_STRING_LENGTH];

for (c1 = ch->clans; get_next_clan (&c1, clan_name, &flags);)
{

if (get_clan (tch, clan_name, &flags))
return 1;
}

return 0;
}*/

int
is_leader (CHAR_DATA * src, CHAR_DATA * tar)
{
	int c1_flags;
	int c2_flags;
	char *c1;
	char clan_name[MAX_STRING_LENGTH];

	for (c1 = src->clans; get_next_clan (&c1, clan_name, &c1_flags);)
	{

		if (get_clan (tar, clan_name, &c2_flags) &&
			!IS_SET (c2_flags, CLAN_LEADER) &&
			!IS_SET (c2_flags, CLAN_LEADER_OBJ) &&
			(c1_flags > c2_flags || IS_SET (c1_flags, CLAN_LEADER)))
			return 1;
	}

	if (!IS_NPC (src) && IS_NPC (tar))
	{
		if (tar->mob->owner && !str_cmp (src->tname, tar->mob->owner))
			return 1;
	}

	return 0;
}


void
add_clan_id_string (char *string, char *clan_name, char *clan_flags)
{
	NAME_SWITCH_DATA *nsp;
	char *argument;
	char buf[MAX_STRING_LENGTH];

	/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
		if (!str_cmp (nsp->old_name, clan_name))
			clan_name = nsp->new_name;

	argument = string;

	while (1)
	{

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if (!*buf)
			break;

		if (!str_cmp (buf, clan_name))
			return;
	}

	/* string doesn't have that clan, add it */

	if (string && *string)
		sprintf (buf, "%s '%s' %s", string, clan_flags, clan_name);
	else
		sprintf (buf, "'%s' %s", clan_flags, clan_name);

	free_mem (string);

	string = duplicateString (buf);
}

void
add_clan_id (CHAR_DATA * ch, char *clan_name, const char *clan_flags)
{
	NAME_SWITCH_DATA *nsp;
	char *argument;
	char buf[MAX_STRING_LENGTH];

	/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
		if (!str_cmp (nsp->old_name, clan_name))
			clan_name = nsp->new_name;

	argument = ch->clans;

	while (1)
	{

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if (!*buf)
			break;

		if (!str_cmp (buf, clan_name))
			return;
	}

	/* ch doesn't have that clan, add it */

	if (ch->clans && *ch->clans)
		sprintf (buf, "%s '%s' %s", ch->clans, clan_flags, clan_name);
	else
		sprintf (buf, "'%s' %s", clan_flags, clan_name);

	free_mem (ch->clans);

	ch->clans = duplicateString (buf);
}

int
is_clan_member (CHAR_DATA * ch, char *clan_name)
{
	int clan_flags;

	if (get_clan (ch, clan_name, &clan_flags))
		return 1;

	return 0;
}

/* the _player version of this routine is called to parse a clan name
a player might type in.  If clan is registered, then only the full
name is valid.  If it is not, the short name is appropriate.
*/

int
is_clan_member_player (CHAR_DATA * ch, char *clan_name)
{
	CLAN_DATA *clan;

	if (is_clan_member (ch, clan_name))
	{				/* Short name match */
		if (get_clandef (clan_name))
			return 0;		/* Supplied short name when long existed */
		return 1;
	}

	if (!(clan = get_clandef_long (clan_name)))
		return 0;

	return is_clan_member (ch, clan->name);
}

void
clan_object_equip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int clan_flags;
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{
		if (clan->member_vnum == obj->nVirtual ||
			clan->leader_vnum == obj->nVirtual)
			break;
	}

	if (!clan)
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if (clan->member_vnum == obj->nVirtual)
		clan_flags |= CLAN_MEMBER_OBJ;

	if (clan->leader_vnum == obj->nVirtual)
		clan_flags |= CLAN_LEADER_OBJ;

	add_clan (ch, clan->name, clan_flags);
}

void
clan_object_unequip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int clan_flags;
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{
		if (clan->member_vnum == obj->nVirtual ||
			clan->leader_vnum == obj->nVirtual)
			break;
	}

	if (!clan)
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if (clan->member_vnum == obj->nVirtual)
		clan_flags &= ~CLAN_MEMBER_OBJ;

	if (clan->leader_vnum == obj->nVirtual)
		clan_flags &= ~CLAN_LEADER_OBJ;

	if (clan_flags)
		add_clan (ch, clan->name, clan_flags);
}

void
do_clan (CHAR_DATA * ch, char *argument, int cmd)
{
	int zone;
	int leader_obj_vnum;
	int member_obj_vnum;
	int new_flags;
	int clan_flags;
	int num_clans = 0;
	int i;
	NAME_SWITCH_DATA *name_switch;
	NAME_SWITCH_DATA *nsp;
	OBJ_DATA *obj;
	CLAN_DATA *clan;
	CLAN_DATA *delete_clan;
	CHAR_DATA *edit_mob = NULL;
	CHAR_DATA *tch;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];
	char oldname[MAX_STRING_LENGTH];
	char newname[MAX_STRING_LENGTH];
	char literal[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char *the_clans[2000];

	if (IS_NPC (ch))
	{
		send_to_char ("Sorry, but you can't do this while switched...\n", ch);
		return;
	}

	if (!*argument)
	{

		send_to_char ("Old clan name   ->  New clan name\n", ch);
		send_to_char ("=============       =============\n", ch);

		if (!clan_name_switch_list)
			send_to_char ("No clans set to be renamed.\n", ch);

		else
		{
			for (nsp = clan_name_switch_list; nsp; nsp = nsp->next)
			{
				sprintf (buf, "%-15s     %-15s\n", nsp->old_name,
					nsp->new_name);
				send_to_char (buf, ch);
			}
		}

		sprintf (buf,"\nClan Name        Full Clan Name\n");
		sprintf (buf + strlen(buf),"===============  =================================\n");

		for (clan = clan_list; clan; clan = clan->next)
		{
			sprintf (buf + strlen(buf), "%-15s  %s\n", clan->name, clan->literal);

			if (clan->zone)
			{
				sprintf (buf + strlen(buf), "                 Enforcement Zone %d\n\n",
					clan->zone);
			}

			if (clan->member_vnum)
			{
				obj = vtoo (clan->member_vnum);
				sprintf (buf + strlen(buf), "                 Member Object (%05d):  %s\n",
					clan->member_vnum,
					obj ? obj->short_description : "UNDEFINED");
			}

			if (clan->leader_vnum)
			{
				obj = vtoo (clan->leader_vnum);
				sprintf (buf + strlen(buf), "                 Leader Object (%05d):  %s\n",
					clan->leader_vnum,
					obj ? obj->short_description : "UNDEFINED");
			}
		}
		page_string (ch->desc, buf);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		send_to_char ("Syntax for modifying PCs:\n\n", ch);
		send_to_char ("   clan set <clan-name> [<clan-flags>]\n", ch);
		send_to_char ("   clan remove <clan-name>         (or all)\n", ch);
		send_to_char ("\nSyntax for modifying clan definitions:\n", ch);
		send_to_char ("   clan rename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan unrename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan register <name> <enforcement zone> <long name> "
			"[<leader obj>] [<member obj>]\n", ch);
		send_to_char ("   clan unregister <name>\n", ch);
		send_to_char ("   clan list\n", ch);
		send_to_char ("\nThe obj vnums are optional.  Specify zone 0 if no "
			"enforcement zone.\n\nExamples:\n", ch);
		send_to_char ("  > clan set osgilwatch member leader\n", ch);
		send_to_char ("  > clan remove osgilwatch\n", ch);
		send_to_char ("  > clan register osgilwatch 10 'Osgiliath Watch'\n", ch);
		send_to_char ("  > clan unregister osgilwatch\n", ch);
		send_to_char ("  > clan rename 10 osgilwatch\n", ch);
		send_to_char ("  > clan unrename 10 osgilwatch\n", ch);
		send_to_char ("  > clan list\n", ch);
		return;
	}

	if (!str_cmp (buf, "list"))
	{

		for (std::list<char_data*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++) {
			tch = *tch_iterator;

			if (tch->deleted)
				continue;

			p = tch->clans;

			if (!p || !*p)
				continue;

			while (get_next_clan (&p, clan_name, &clan_flags))
			{

				if (num_clans >= 1999)	/* Too many in list */
					break;

				for (i = 0; i < num_clans; i++)
					if (!str_cmp (clan_name, the_clans[i]))
						break;

				if (i >= num_clans)
					the_clans[num_clans++] = duplicateString (clan_name);
			}

		}

		*b_buf = '\0';

		for (i = 0; i < num_clans; i++)
		{

			sprintf (b_buf + strlen (b_buf), "%-15s", the_clans[i]);

			if (i % 5 == 4)
				strcat (b_buf, "\n");
			else
				strcat (b_buf, " ");
		}

		page_string (ch->desc, b_buf);

		return;
	}

	if (!str_cmp (buf, "set") ||
		!str_cmp (buf, "remove") || !str_cmp (buf, "delete"))
	{

		if (IS_NPC (ch))
		{
			send_to_char ("This command cannot be used while switched.\n", ch);
			return;
		}

		if ((!ch->pc->edit_mob && !ch->pc->edit_player) ||
			(!(edit_mob = vtom (ch->pc->edit_mob)) &&
			!(edit_mob = ch->pc->edit_player)))
		{
			send_to_char ("Start by using the MOBILE command.\n", ch);
			return;
		}
	}

	if (!str_cmp (buf, "set"))
	{

		argument = one_argument (argument, clan_name);

		if (!*clan_name)
		{
			send_to_char ("Expected a clan name after 'set'.\n", ch);
			return;
		}

		new_flags = clan_flags_to_value (argument);

		if (get_clan (edit_mob, clan_name, &clan_flags))
			remove_clan (edit_mob, clan_name);

		clan_flags = 0;

		while (1)
		{

			argument = one_argument (argument, buf);

			if (!*buf)
				break;

			if (!str_cmp (buf, "member"))
			{
				TOGGLE (clan_flags, CLAN_MEMBER);
			}			/* {}'s define     */
			else if (!str_cmp (buf, "leader"))	/* so no ; needed  */
			{
				TOGGLE (clan_flags, CLAN_LEADER);
			}
			else if (!str_cmp (buf, "recruit"))
			{
				TOGGLE (clan_flags, CLAN_RECRUIT);
			}
			else if (!str_cmp (buf, "private"))
			{
				TOGGLE (clan_flags, CLAN_PRIVATE);
			}
			else if (!str_cmp (buf, "corporal"))
			{
				TOGGLE (clan_flags, CLAN_CORPORAL);
			}
			else if (!str_cmp (buf, "sergeant"))
			{
				TOGGLE (clan_flags, CLAN_SERGEANT);
			}
			else if (!str_cmp (buf, "lieutenant"))
			{
				TOGGLE (clan_flags, CLAN_LIEUTENANT);
			}
			else if (!str_cmp (buf, "captain"))
			{
				TOGGLE (clan_flags, CLAN_CAPTAIN);
			}
			else if (!str_cmp (buf, "general"))
			{
				TOGGLE (clan_flags, CLAN_GENERAL);
			}
			else if (!str_cmp (buf, "commander"))
			{
				TOGGLE (clan_flags, CLAN_COMMANDER);
			}
			else if (!str_cmp (buf, "apprentice"))
			{
				TOGGLE (clan_flags, CLAN_APPRENTICE);
			}
			else if (!str_cmp (buf, "journeyman"))
			{
				TOGGLE (clan_flags, CLAN_JOURNEYMAN);
			}
			else if (!str_cmp (buf, "master"))
			{
				TOGGLE (clan_flags, CLAN_MASTER);
			}
			else
			{
				sprintf (literal, "Flag %s is unknown for clans.\n", buf);
				send_to_char (literal, ch);
			}
		}

		if (!clan_flags)
			clan_flags = CLAN_MEMBER;

		add_clan (edit_mob, clan_name, clan_flags);

		if (edit_mob->mob)
		{
			sprintf (buf, "%d mobile(s) in world redefined.\n",
				redefine_mobiles (edit_mob));
			send_to_char (buf, ch);
		}

		return;			/* Return from n/pc specific uses of clan */
	}

	else if (!str_cmp (buf, "remove") || !str_cmp (buf, "delete"))
	{

		argument = one_argument (argument, clan_name);

		if (!*clan_name)
		{
			send_to_char ("Expected a clan name to remove from n/pc.\n", ch);
			return;
		}

		if (!str_cmp (clan_name, "all"))
		{
			free_mem (ch->clans);
			ch->clans = duplicateString ("");
		}

		else if (!get_clan (edit_mob, clan_name, &clan_flags))
		{
			send_to_char ("N/PC doesn't have that clan.\n", ch);
			return;
		}

		remove_clan (edit_mob, clan_name);

		return;			/* Return from n/pc specific uses of clan */
	}

	else if (!str_cmp (buf, "unregister"))
	{

		argument = one_argument (argument, clan_name);

		if (!clan_list)
		{
			send_to_char ("There are no registered clans.\n", ch);
			return;
		}

		if (!str_cmp (clan_list->name, clan_name))
		{
			delete_clan = clan_list;
			clan_list = clan_list->next;
		}

		else
		{
			delete_clan = NULL;

			for (clan = clan_list; clan->next; clan = clan->next)
			{
				if (!str_cmp (clan->next->name, clan_name))
				{
					delete_clan = clan->next;
					clan->next = delete_clan->next;
					break;
				}
			}

			if (!delete_clan)
			{
				send_to_char ("No such registered clan name.\n", ch);
				return;
			}
		}
		clan__do_remove (delete_clan);

		free_mem (delete_clan->name);
		free_mem (delete_clan->literal);
		free_mem (delete_clan);
	}

	else if (!str_cmp (buf, "rename"))
	{

		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if (!*oldname || !*newname)
		{
			send_to_char ("rename <oldclanname> <newclanname>\n", ch);
			return;
		}

		name_switch = new name_switch_data;
		name_switch->old_name = duplicateString (oldname);
		name_switch->new_name = duplicateString (newname);

		if (!clan_name_switch_list)
			clan_name_switch_list = name_switch;
		else
		{
			for (nsp = clan_name_switch_list; nsp->next; nsp = nsp->next)
				;
			nsp->next = name_switch;
		}
	}

	else if (!str_cmp (buf, "unrename"))
	{
		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if (!*oldname || !*newname)
		{
			send_to_char ("clan unrename <oldclanname> <newclanname>\n", ch);
			send_to_char ("This command deletes a rename entry.\n", ch);
			return;
		}

		if (!str_cmp (clan_name_switch_list->old_name, oldname) &&
			!str_cmp (clan_name_switch_list->new_name, newname))
		{
			free_mem (clan_name_switch_list->old_name);
			free_mem (clan_name_switch_list->new_name);
			nsp = clan_name_switch_list;
			clan_name_switch_list = nsp->next;
			free_mem (nsp);
			send_to_char ("Rename entry deleted.\n", ch);
			write_dynamic_registry (ch);
			return;
		}

		for (nsp = clan_name_switch_list; nsp->next; nsp = nsp->next)
			if (!str_cmp (nsp->next->old_name, oldname) &&
				!str_cmp (nsp->next->new_name, newname))
				break;

		if (!nsp->next)
		{
			send_to_char ("Sorry, no such rename entry pair found.\n", ch);
			return;
		}

		name_switch = nsp->next;
		nsp->next = name_switch->next;

		free_mem (name_switch->old_name);
		free_mem (name_switch->new_name);
		free_mem (name_switch);

		send_to_char ("Rename entry deleted.\n", ch);
	}

	else if (!str_cmp (buf, "register"))
	{

		argument = one_argument (argument, name);

		if (get_clandef (name))
		{
			send_to_char ("That clan has already been registered.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if (!*name || !*buf || !isdigit (*buf))
		{
			send_to_char ("Type clan ? for syntax.\n", ch);
			return;
		}

		zone = atoi (buf);

		argument = one_argument (argument, literal);

		if (!*literal)
		{
			send_to_char ("Syntax error parsing literal, type clan ? for "
				"syntax.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if (!*buf)
		{
			leader_obj_vnum = 0;
		}
		else if (!isdigit (*buf))
		{
			send_to_char ("Syntax error.  Did you enclose the long name in "
				"quotes?  Type clan ?.\n", ch);
			return;
		}
		else
		{
			leader_obj_vnum = atoi (buf);
			if (leader_obj_vnum && !vtoo (leader_obj_vnum))
				send_to_char ("NOTE:  Leader object doesn't currently "
				"exist.\n", ch);
		}

		argument = one_argument (argument, buf);

		if (!*buf)
		{
			member_obj_vnum = 0;
		}
		else if (!isdigit (*buf))
		{
			send_to_char ("Syntax error parsing member object.  Type clan ? "
				"for syntax.\n", ch);
			return;
		}
		else
		{
			member_obj_vnum = atoi (buf);
			if (member_obj_vnum && !vtoo (member_obj_vnum))
				send_to_char ("NOTE:  Member object doesn't currently "
				"exist.\n", ch);
		}

		clan = new CLAN_DATA;

		clan->name = duplicateString (name);
		clan->zone = zone;
		clan->literal = duplicateString (literal);
		clan->member_vnum = member_obj_vnum;
		clan->leader_vnum = leader_obj_vnum;
		clan->omni_vnum = 0;

		clan__do_add (clan);

		clan->next = clan_list;
		clan_list = clan;
	}

	else
	{
		send_to_char ("What do you want to do again?  Type clan ? for help.\n",
			ch);
		return;
	}

	write_dynamic_registry (ch);
}

// Overloaded clan_flags_to_value for use in do_promote
int
clan_flags_to_value (char *flag_names, char *clan_name)
{
	int flags = 0;
	char buf[MAX_STRING_LENGTH];

	while (1)
	{
		flag_names = one_argument (flag_names, buf);

		if (!*buf)
			break;

		if (!str_cmp (buf, "member"))
			flags |= CLAN_MEMBER;
		else if (!str_cmp (buf, "leader"))
			flags |= CLAN_LEADER;
		else if (!str_cmp (buf, "memberobj"))
			flags |= CLAN_MEMBER_OBJ;
		else if (!str_cmp (buf, "leaderobj"))
			flags |= CLAN_LEADER_OBJ;

		else if (!str_cmp (buf, "member")
			|| ((!str_cmp (buf, "kalla'jundi") || !str_cmp (buf, "kalla'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "kalla") || !str_cmp (buf, "kalla al mara"))
			&& !str_cmp (clan_name, "abdul-matin")) )
			flags |= CLAN_MEMBER;

		else if (!str_cmp (buf, "recruit")
			|| ((!str_cmp (buf, "khafid'jundi") || !str_cmp (buf, "khafid'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "khafid") || !str_cmp (buf, "khafid al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| ((!str_cmp (buf, "snaga") || !str_cmp (buf, "snaga-uruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "initiate"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp (buf, "kaal")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "squire")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "push-khur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "roenucht")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "squire recruit")
			&& !str_cmp (clan_name, "astirian_villeins"))
			|| (!str_cmp (buf, "army recruit")
			&& !str_cmp (clan_name, "militaryrecruits"))
			|| (!str_cmp (buf, "liegeman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))) )
			flags |= CLAN_RECRUIT;

		else if (!str_cmp (buf, "private")
			|| ((!str_cmp (buf, "harba'jundi") || !str_cmp (buf, "harba'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "harba") || !str_cmp (buf, "harba al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| ((!str_cmp (buf, "acolyte"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp(buf, "ohtar")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "uruk") || !str_cmp (buf, "high-snaga"))
			&& !str_cmp (clan_name, "gothakra"))
			|| (!str_cmp (buf, "rukh-kaal")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "apprentice-seeker-knight")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "khur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "heggurach")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "squire of lord astirian")
			&& !str_cmp (clan_name, "astirian_villeins"))
			|| (!str_cmp (buf, "free spear") && !str_cmp (clan_name, "dalewatch"))
			|| (!str_cmp (buf, "footman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_PRIVATE;

		else if (!str_cmp (buf, "corporal")
			|| ((!str_cmp (buf, "sayf'jundi") || !str_cmp (buf, "sayf'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "sayf") || !str_cmp (buf, "sayf al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| (!str_cmp(buf, "roquen")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "dark-priest"))
			&& !str_cmp (clan_name, "shadow-cult"))
			||  ((!str_cmp (buf, "puruk") || !str_cmp (buf, "zuruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| (!str_cmp (buf, "rukh")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "seeker-knight")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gur")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "rhyfelwr")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "villein employer of lord astirian")
			&& !str_cmp (clan_name, "astirian_villeins"))
			|| (!str_cmp (buf, "armsman")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_CORPORAL;

		else if (!str_cmp (buf, "sergeant")
			|| ((!str_cmp (buf, "qasir'jundi") || !str_cmp (buf, "qasir'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "qasir") || !str_cmp (buf, "qasir al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| ((!str_cmp (buf, "zaak") || !str_cmp (buf, "high-puruk"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "dread-minister "))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| ((!str_cmp (buf, "mik") || !str_cmp (buf, "mith"))
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-lieutenant")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gurash")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "cloumaggen")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "warden of lord astirian") || !str_cmp (buf, "the yeoman of the wardenry"))
			&& !str_cmp (clan_name, "astirian_villeins"))
			flags |= CLAN_SERGEANT;

		else if (!str_cmp (buf, "lieutenant")
			|| ((!str_cmp (buf, "ashum'jundi") || !str_cmp (buf, "ashum'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "ashum") || !str_cmp (buf, "ashum al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| (!str_cmp(buf, "constable")
			&& !str_cmp(clan_name, "tirithguard"))
			|| ((!str_cmp (buf, "overlord"))
			&& !str_cmp (clan_name, "shadow-cult"))
			||  ((!str_cmp (buf, "puruk-zuul") || !str_cmp (buf, "ba'zaak"))
			&& !str_cmp (clan_name, "gothakra"))
			|| ((!str_cmp (buf, "amme") || !str_cmp (buf, "atto"))
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-captain")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "jomaa")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "Chief Warden of Lord Astirian")
			&& !str_cmp (clan_name, "astirian_villeins"))
			|| (!str_cmp (buf, "constable")
			&& (!str_cmp (clan_name, "eradan_battalion")
			|| !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_LIEUTENANT;

		else if (!str_cmp (buf, "captain")
			|| ((!str_cmp (buf, "badikh'jundi") || !str_cmp (buf, "badikh'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "badikh") || !str_cmp (buf, "badikh al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| (!str_cmp (buf, "barun")
			&& !str_cmp (clan_name, "com"))
			|| ((!str_cmp (buf, "barun-an-Nalo"))
			&& !str_cmp (clan_name, "shadow-cult"))
			|| (!str_cmp (buf, "knight-general")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "gurashul")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "clowgos")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "Retainer of Lord Astirian")
			&& !str_cmp (clan_name, "astirian_villeins"))
			|| (!str_cmp (buf, "lord")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion"))))
			flags |= CLAN_CAPTAIN;

		else if (!str_cmp (buf, "general")
			|| ((!str_cmp (buf, "dahab'jundi") || !str_cmp (buf, "dahab'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| (!str_cmp(buf, "marshall")
			&& !str_cmp(clan_name, "tirithguard"))
			||  (!str_cmp (buf, "nalo-barun")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-commander")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "rhi")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "bughrak")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "Senior Retainer of Lord Astirian")
			&& !str_cmp (clan_name, "astirian_villeins")) )
			flags |= CLAN_GENERAL;

		else if (!str_cmp (buf, "commander")
			|| ((!str_cmp (buf, "sha'jundi") || !str_cmp (buf, "sha'jundi al mara"))
			&& !str_cmp (clan_name, "silvermoon") || !str_cmp (clan_name, "silvermoon-cobra") || !str_cmp (clan_name, "silvermoon-field-cobra"))
			|| ((!str_cmp (buf, "sha") || !str_cmp (buf, "sha al mara"))
			&& !str_cmp (clan_name, "abdul-matin"))
			|| (!str_cmp (buf, "daur-phazan")
			&& !str_cmp (clan_name, "com"))
			|| (!str_cmp (buf, "knight-grand-cross")
			&& !str_cmp (clan_name, "seekers"))
			|| (!str_cmp (buf, "duumul-bughrak")
			&& !str_cmp (clan_name, "khagdu"))
			|| (!str_cmp (buf, "Jarl")
			&& !str_cmp (clan_name, "nebavla_tribe"))
			|| (!str_cmp (buf, "captain of great lord astirian")
			&& !str_cmp (clan_name, "astirian_villeins")) )
			flags |= CLAN_COMMANDER;

		else if (!str_cmp (buf, "leader")
			|| (!str_cmp (buf, "commander of west osgiliath and yeoman to lord astirian")
			&& !str_cmp (clan_name, "militaryrecruits"))
			|| (!str_cmp (buf, "lord eradan")
			&& (!str_cmp (clan_name, "eradan_battalion") || !str_cmp (clan_name, "ithilien_battalion")))
			|| (!str_cmp (buf, "great lord astirian")
			&& !str_cmp (clan_name, "astirian_villeins")) )
			flags |= CLAN_LEADER;

		else if (!str_cmp (buf, "apprentice") || (!str_cmp (buf, "apprentice-")) )
			flags |= CLAN_APPRENTICE;

		else if (!str_cmp (buf, "journeyman")
			|| (!str_cmp (buf, "yameg")
			&& !str_cmp (clan_name, "khagdu")) )
			flags |= CLAN_JOURNEYMAN;

		else if (!str_cmp (buf, "master")
			|| (!str_cmp (buf, "yameg-khur")
			&& !str_cmp (clan_name, "khagdu")) )
			flags |= CLAN_MASTER;
	}

	return flags;
}



int
clan_flags_to_value (char *flag_names)
{
	int flags = 0;
	char buf[MAX_STRING_LENGTH];

	while (1)
	{
		flag_names = one_argument (flag_names, buf);

		if (!*buf)
			break;

		if (!str_cmp (buf, "member"))
			flags |= CLAN_MEMBER;
		else if (!str_cmp (buf, "leader"))
			flags |= CLAN_LEADER;
		else if (!str_cmp (buf, "memberobj"))
			flags |= CLAN_MEMBER_OBJ;
		else if (!str_cmp (buf, "leaderobj"))
			flags |= CLAN_LEADER_OBJ;
		else if (!str_cmp (buf, "recruit"))
			flags |= CLAN_RECRUIT;
		else if (!str_cmp (buf, "private"))
			flags |= CLAN_PRIVATE;
		else if (!str_cmp (buf, "corporal"))
			flags |= CLAN_CORPORAL;
		else if (!str_cmp (buf, "sergeant"))
			flags |= CLAN_SERGEANT;
		else if (!str_cmp (buf, "lieutenant"))
			flags |= CLAN_LIEUTENANT;
		else if (!str_cmp (buf, "captain"))
			flags |= CLAN_CAPTAIN;
		else if (!str_cmp (buf, "general"))
			flags |= CLAN_GENERAL;
		else if (!str_cmp (buf, "commander"))
			flags |= CLAN_COMMANDER;
		else if (!str_cmp (buf, "apprentice"))
			flags |= CLAN_APPRENTICE;
		else if (!str_cmp (buf, "journeyman"))
			flags |= CLAN_JOURNEYMAN;
		else if (!str_cmp (buf, "master"))
			flags |= CLAN_MASTER;
	}

	return flags;
}

char *
value_to_clan_flags (int flags)
{
	*s_buf = '\0';

	if (IS_SET (flags, CLAN_LEADER))
		strcat (s_buf, "leader ");

	if (IS_SET (flags, CLAN_MEMBER))
		strcat (s_buf, "member ");

	if (IS_SET (flags, CLAN_RECRUIT))
		strcat (s_buf, "recruit ");

	if (IS_SET (flags, CLAN_PRIVATE))
		strcat (s_buf, "private ");

	if (IS_SET (flags, CLAN_CORPORAL))
		strcat (s_buf, "corporal ");

	if (IS_SET (flags, CLAN_SERGEANT))
		strcat (s_buf, "sergeant ");

	if (IS_SET (flags, CLAN_LIEUTENANT))
		strcat (s_buf, "lieutenant ");

	if (IS_SET (flags, CLAN_CAPTAIN))
		strcat (s_buf, "captain ");

	if (IS_SET (flags, CLAN_GENERAL))
		strcat (s_buf, "general");

	if (IS_SET (flags, CLAN_COMMANDER))
		strcat (s_buf, "commander ");

	if (IS_SET (flags, CLAN_APPRENTICE))
		strcat (s_buf, "apprentice ");

	if (IS_SET (flags, CLAN_JOURNEYMAN))
		strcat (s_buf, "journeyman ");

	if (IS_SET (flags, CLAN_MASTER))
		strcat (s_buf, "master ");

	if (IS_SET (flags, CLAN_LEADER_OBJ))
		strcat (s_buf, "leaderobj ");

	if (IS_SET (flags, CLAN_MEMBER_OBJ))
		strcat (s_buf, "memberobj ");

	if (*s_buf && s_buf[strlen (s_buf) - 1] == ' ')
		s_buf[strlen (s_buf) - 1] = '\0';

	return s_buf;
}

char *
remove_clan_from_string (char *string, char *old_clan_name)
{
	char *argument;
	static char buf[MAX_STRING_LENGTH];
	char clan_flags[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!*string)
		return NULL;

	argument = string;

	*buf = '\0';

	while (1)
	{
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (str_cmp (clan_name, old_clan_name))
			sprintf (buf + strlen (buf), "'%s' %s ", clan_flags, clan_name);
	}

	free_mem (string);
	string = NULL;

	if (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	return buf;
}

char *
add_clan_to_string (char *string, char *new_clan_name, int clan_flags)
{
	char *argument;
	char clan_name[MAX_STRING_LENGTH];
	char flag_names[MAX_STRING_LENGTH];
	static char buf[MAX_STRING_LENGTH];

	argument = string;

	/* Look to see if we're just changing flags */

	while (1)
	{

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (!str_cmp (clan_name, new_clan_name))
		{
			remove_clan_from_string (string, new_clan_name);
			break;
		}
	}

	/* Clan is new */

	if (string && *string)
		sprintf (buf, "'%s' %s %s",
		value_to_clan_flags (clan_flags), new_clan_name, string);
	else
		sprintf (buf, "'%s' %s", value_to_clan_flags (clan_flags), new_clan_name);

	while (buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (string && *string)
	{
		free_mem (string);
		string = NULL;
	}

	return buf;
}

void
remove_clan (CHAR_DATA * ch, char *old_clan_name)
{
	char *argument;
	char buf[MAX_STRING_LENGTH];
	char clan_flags[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!*ch->clans)
		return;

	argument = ch->clans;

	*buf = '\0';

	while (1)
	{
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (str_cmp (clan_name, old_clan_name))
			sprintf (buf + strlen (buf), "'%s' %s ", clan_flags, clan_name);
	}

	if (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (ch->clans && *ch->clans != '\0')
		free_mem (ch->clans);

	ch->clans = duplicateString (buf);
	clan_forum_remove (ch, old_clan_name);
}

void
add_clan (CHAR_DATA * ch, char *new_clan_name, int clan_flags)
{
	char *argument;
	char clan_name[MAX_STRING_LENGTH];
	char flag_names[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = ch->clans;

	/* Look to see if we're just changing flags */

	while (1)
	{

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if (!*clan_name)
			break;

		if (!str_cmp (clan_name, new_clan_name))
		{
			remove_clan (ch, new_clan_name);
			break;
		}
	}

	/* Clan is new */
	clan_forum_add (ch, new_clan_name, value_to_clan_flags (clan_flags));
	sprintf (buf, "'%s' %s %s",
		value_to_clan_flags (clan_flags), new_clan_name, ch->clans);

	while (buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	if (ch->clans && *ch->clans != '\0')
		free_mem (ch->clans);

	ch->clans = duplicateString (buf);
}

int
get_next_clan (char **p, char *clan_name, int *clan_flags)
{
	char flag_names[MAX_STRING_LENGTH];

	*p = one_argument (*p, flag_names);

	*clan_flags = clan_flags_to_value (flag_names);

	*p = one_argument (*p, clan_name);

	if (!*clan_name)
		return 0;

	return 1;
}

char *
get_shared_clan_rank (CHAR_DATA * ch, CHAR_DATA * observer)
{
	char *argument, *argument2;
	char clan_name[MAX_STRING_LENGTH], clan_name2[MAX_STRING_LENGTH];
	int flags = 0, flags2 = 0, highest_rank = 0;

	argument = observer->clans;

	while (get_next_clan (&argument, clan_name, &flags))
	{
		argument2 = ch->clans;
		while (get_next_clan (&argument2, clan_name2, &flags2))
		{
			if (!str_cmp (clan_name, "osgi_citizens") ||
				!str_cmp (clan_name, "mordor_char") ||
				!str_cmp (clan_name, "mt_citizens") ||
				!str_cmp (clan_name, "outpost_citizens"))
				continue;
			if (!str_cmp (clan_name, clan_name2))
			{
				if (flags2 > highest_rank)
					highest_rank = flags2;
			}
		}
	}

	flags2 = highest_rank;

	if (flags2 == CLAN_LEADER)
		return "Leadership";
	else if (flags2 == CLAN_RECRUIT)
		return "Recruit";
	else if (flags2 == CLAN_PRIVATE)
		return "Private";
	else if (flags2 == CLAN_CORPORAL)
		return "Corporal";
	else if (flags2 == CLAN_SERGEANT)
		return "Sergeant";
	else if (flags2 == CLAN_LIEUTENANT)
		return "Lieutenant";
	else if (flags2 == CLAN_CAPTAIN)
		return "Captain";
	else if (flags2 == CLAN_GENERAL)
		return "General";
	else if (flags2 == CLAN_COMMANDER)
		return "Commander";
	else if (flags2 == CLAN_APPRENTICE)
		return "Apprentice";
	else if (flags2 == CLAN_JOURNEYMAN)
		return "Journeyman";
	else if (flags2 == CLAN_MASTER)
		return "Master";
	else if (flags2 > 0)
		return "Membership";

	return NULL;
}

char *
get_clan_rank_name (int flags)
{
	if (flags == CLAN_LEADER)
		return "Leadership";
	else if (flags == CLAN_RECRUIT)
		return "Recruit";
	else if (flags == CLAN_PRIVATE)
		return "Private";
	else if (flags == CLAN_CORPORAL)
		return "Corporal";
	else if (flags == CLAN_SERGEANT)
		return "Sergeant";
	else if (flags == CLAN_LIEUTENANT)
		return "Lieutenant";
	else if (flags == CLAN_CAPTAIN)
		return "Captain";
	else if (flags == CLAN_GENERAL)
		return "General";
	else if (flags == CLAN_COMMANDER)
		return "Commander";
	else if (flags == CLAN_APPRENTICE)
		return "Apprentice";
	else if (flags == CLAN_JOURNEYMAN)
		return "Journeyman";
	else if (flags == CLAN_MASTER)
		return "Master";
	else if (flags > 0)
		return "Membership";

	return NULL;
}

char *get_clan_rank_name (CHAR_DATA *ch, char * clan, int flags)
{
	//Leader
	if (flags == CLAN_LEADER)
	{
		if (!str_cmp (clan, "militaryrecruits"))
		{
			return "Retainer to Great Lord Astirian";
		}
		if (!str_cmp (clan, "eradan_battalion") || !str_cmp (clan, "ithilien_battalion"))
		{
			return "Lord Eradan";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Great Lord Astirian";
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Qadim Al Mara";
			}
			else
			{
				return "Qadim";
			}
		}
		return "Leadership";
	}

	//Recruit
	else if (flags == CLAN_RECRUIT)
	{
		if (!str_cmp(clan,"z3_astirian_wardens"))
		{
			return "Squire-recruit";
		}
		if (!str_cmp (clan, "dalewatch"))
		{
			return "Dreng";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Fightah";
		}
		if (!str_cmp (clan, "ralan-salvage"))
		{
			return "Boot";
		}
		if (!str_cmp (clan, "gothakra"))
		{
			if (ch->race == lookup_race_id("Orc"))
			{
				return "Snaga Uruk";
			}
			else
			{
				return "Snaga";
			}
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Initiate";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Kaal";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Push-Khur";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Roenucht";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Khafid'Jundi Al Mara";
			}
			else
			{
				return "Khafid'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Khafid Al Mara";
			}
			else
			{
				return "Khafid";
			}
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Squire";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Squire-recruit of the Wardenry";
		}
		return "Recruit";
	}

	//Private
	else if (flags == CLAN_PRIVATE)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Squire";
		}
		if (!str_cmp (clan, "dalewatch"))
		{
			return "Spjot";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Respected Fightah";
		}
		if (!str_cmp (clan, "ralan-salvage"))
		{
			return "Veteran";
		}
		if (!str_cmp (clan, "gothakra"))
		{
			if (ch->race == lookup_race_id("Orc"))
			{
				return "Uruk";
			}
			else
			{
				return "High Snaga";
			}
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Acolyte";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Khur";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Heggurach";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Rukh-Kaal";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Apprentice Seeker-Knight";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Squire of the Wardenry";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Harba'Jundi Al Mara";
			}
			else
			{
				return "Harba'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Harba Al Mara";
			}
			else
			{
				return "Harba";
			}
		}
		return "Private";
	}

	//Corporal
	else if (flags == CLAN_CORPORAL)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Warden";
		}
		if (!str_cmp (clan, "dalewatch"))
		{
			return "Brimskr";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Fist";
		}
		if (!str_cmp (clan, "gothakra"))
		{
			if (ch->race == lookup_race_id("Orc"))
			{
				return "Zuruk";
			}
			else
			{
				return "Puruk";
			}
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Dark Priest";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Gur";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Rhyfelwr";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Rukh";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Seeker-Knight";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Villein Employer of Lord Astirian";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Sayf'Jundi Al Mara";
			}
			else
			{
				return "Sayf'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Sayf Al Mara";
			}
			else
			{
				return "Sayf";
			}
		}
		return "Corporal";
	}

	//Sergeant
	else if (flags == CLAN_SERGEANT)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Warden-Sergeant";
		}
		if (!str_cmp (clan, "dalewatch"))
		{
			return "Hundredman";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Respected Fist";
		}
		if (!str_cmp (clan, "ralan-salvage"))
		{
			for (OBJ_DATA *obj = ch->equip; obj; obj = obj->next_content)
			{
				if (obj->nVirtual == 66743)
					return "Quartermaster";
				if (obj->nVirtual == 66744)
					return "Master of Horse";
			}
			return "Sergeant";
		}
		if (!str_cmp (clan, "gothakra"))
		{
			if (ch->race == lookup_race_id("Orc"))
			{
				return "Zaak";
			}
			else
			{
				return "High Puruk";
			}
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Dread Minister";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Gurash";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Cloumaggen";
		}
		if (!str_cmp (clan, "com"))
		{
			if (HSSH(ch) == "her")
			{
				return "Mith";
			}
			else
			{
				return "Mik";
			}
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Knight-Lieutenant";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Warden of Lord Astirian";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Qasir'Jundi Al Mara";
			}
			else
			{
				return "Qasir'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Qasir Al Mara";
			}
			else
			{
				return "Qasir";
			}
		}
		return "Sergeant";
	}

	//lieutenant
	else if (flags == CLAN_LIEUTENANT)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Warden-Lieutenant";
		}
		if (!str_cmp(clan, "dalewatch"))
		{
			return "Heafodman";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Warleader";
		}
		if (!str_cmp (clan, "gothakra"))
		{
			if (ch->race == lookup_race_id("Orc"))
			{
				return "Ba'Zaak";
			}
			else
			{
				return "Puruk-Zuul";
			}
		}
		if (!str_cmp (clan, "com"))
		{
			if (HSSH(ch) == "her")
			{
				return "Amme";
			}
			else
			{
				return "Atto";
			}
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Overlord";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Jomaa";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Knight-Captain";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Senior Captain of the Wardenry";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Ashum'Jundi Al Mara";
			}
			else
			{
				return "Ashum'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Ashum Al Mara";
			}
			else
			{
				return "Ashum";
			}
		}
		return "Lieutenant";
	}

	//Captain
	else if (flags == CLAN_CAPTAIN)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Warden-Captain";
		}
		if (!str_cmp (clan, "bar_caged"))
		{
			return "Great Lord";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Barun";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Gurashul";
		}
		if (!str_cmp (clan, "shadow-cult"))
		{
			return "Barun An-Nalo";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Clowgos";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Knight-General";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Retainer of Lord Astirian";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Badikh'Jundi Al Mara";
			}
			else
			{
				return "Badikh'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Badikh Al Mara";
			}
			else
			{
				return "Badikh";
			}
		}
		return "Captain";
	}

	//General
	else if (flags == CLAN_GENERAL)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Senior Captain";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Nalo-Barun";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Bughrak";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Knight-Commander";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Rhi";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Senior Retainer of Lord Astirian";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Dahab'Jundi Al Mara";
			}
			else
			{
				return "Dahab'Jundi";
			}
		}
		return "General";
	}


	//Commander
	else if (flags == CLAN_COMMANDER)
	{
		if (!str_cmp (clan, "z3_astirian_wardens"))
		{
			return "Chief Warden";
		}
		if (!str_cmp(clan,"dalewatch"))
		{
			return "Ealdorman";
		}
		if (!str_cmp(clan, "ralan-salvage"))
		{
			return "Patron";
		}
		if (!str_cmp (clan, "com"))
		{
			return "Daur-Phazan";
		}
		if (!str_cmp (clan, "khagdu"))
		{
			return "Duumul-Bughrak";
		}
		if (!str_cmp (clan, "seekers"))
		{
			return "Knight-Grand-Cross";
		}
		if (!str_cmp (clan, "nebavla_tribe"))
		{
			return "Jarl";
		}
		if (!str_cmp (clan, "astirian_villeins"))
		{
			return "Chief Warden of Great Lord Astirian";
		}
		if (!str_cmp (clan, "silvermoon") || !str_cmp (clan, "silvermoon-cobra") || !str_cmp (clan, "silvermoon-field-cobra"))
		{
			if (HSSH(ch) == "her")
			{
				return "Sha'Jundi Al Mara";
			}
			else
			{
				return "Sha'Jundi";
			}
		}
		if (!str_cmp (clan, "abdul-matin"))
		{
			if (HSSH(ch) == "her")
			{
				return "Sha Al Mara";
			}
			else
			{
				return "Sha";
			}
		}
		return "Commander";
	}

	//Apprentice
	else if (flags == CLAN_APPRENTICE)
	{
		if (!str_cmp (clan, "mordor_char"))
			return "Kadar-Lai";
		return "Apprentice";
	}

	//Journeyman
	else if (flags == CLAN_JOURNEYMAN)
	{
		if (!str_cmp (clan, "khagdu"))
		{
			return "Yameg";
		}
		if (!str_cmp (clan, "mordor_char"))
			return "Khor";
		return "Journeyman";
	}

	//Master
	else if (flags == CLAN_MASTER)
	{
		if (!str_cmp (clan, "khagdu"))
		{
			return "Yameg-Khur";
		}
		if (!str_cmp (clan, "mordor_char"))
			return "Black Watchman";
		return "Master";
	}

	//Member
	else if (flags > 0)
	{
		if (!str_cmp (clan, "mordor_char"))
			return "Freeman";
		return "Membership";
	}

	return NULL;
}


int
get_clan_in_string (char *string, char *clan, int *clan_flags)
{
	int flags;
	char *argument;
	char clan_name[MAX_STRING_LENGTH];

	argument = string;

	while (get_next_clan (&argument, clan_name, &flags))
	{

		if (!str_cmp (clan_name, clan))
		{
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int
get_clan (CHAR_DATA * ch, const char *clan, int *clan_flags)
{
	int flags;
	char *argument;
	char clan_name[MAX_STRING_LENGTH];

	if (!ch->clans)
		return 0;

	argument = ch->clans;

	while (get_next_clan (&argument, clan_name, &flags))
	{

		if (!str_cmp (clan_name, clan))
		{
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int
get_clan_long (CHAR_DATA * ch, char *clan_name, int *clan_flags)
{
	CLAN_DATA *clan;

	if (!(clan = get_clandef_long (clan_name)))
		return 0;

	if (!get_clan (ch, clan->name, clan_flags))
		return 0;

	return 1;
}

int
get_clan_long_short (CHAR_DATA * ch, char *clan_name, int *clan_flags)
{
	if (get_clan_long (ch, clan_name, clan_flags))
		return 1;

	return get_clan (ch, clan_name, clan_flags);
}

int
get_next_leader (char **p, char *clan_name, int *clan_flags)
{
	char flag_names[MAX_STRING_LENGTH];

	while (1)
	{

		*p = one_argument (*p, flag_names);

		*clan_flags = clan_flags_to_value (flag_names);

		*p = one_argument (*p, clan_name);

		if (!*clan_name)
			return 0;

		if (IS_SET (*clan_flags, CLAN_LEADER) ||
			IS_SET (*clan_flags, CLAN_LEADER_OBJ) ||
			(*clan_flags >= CLAN_SERGEANT && *clan_flags <= CLAN_COMMANDER) ||
			IS_SET (*clan_flags, CLAN_MASTER))
			break;
	}

	return 1;
}

CLAN_DATA *
get_clandef (const char *clan_name)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
		if (!str_cmp (clan->name, clan_name))
			return clan;

	return NULL;
}

CLAN_DATA *
get_clandef_long (char *clan_long_name)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
		if (!str_cmp (clan->literal, clan_long_name))
			return clan;

	return NULL;
}

void
update_enforcement_array (CHAR_DATA * ch)
{
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH];

	p = ch->clans;

	while (get_next_clan (&p, clan_name, &flags))
	{
		if ((clan_def = get_clandef (clan_name)) && clan_def->zone)
		{
			ch->enforcement[clan_def->zone] = true;
		}
	}

	ch->enforcement[0] = true;
}

int
is_area_leader (CHAR_DATA * ch)
{
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH];

	p = ch->clans;

	while (get_next_leader (&p, clan_name, &flags))
	{

		if (!(clan_def = get_clandef (clan_name)))
			continue;

		if (clan_def->zone != 0 && ch->room->zone == clan_def->zone)
			return 1;
	}

	return 0;
}

void
do_assign (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char admin[MAX_STRING_LENGTH];
	char clan[MAX_STRING_LENGTH];

	if (!*argument)
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (str_cmp (buf, "add") && str_cmp (buf, "remove"))
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, admin);
	argument = one_argument (argument, buf2);

	sprintf (clan, "%s", buf2);

	if (!*admin)
	{
		send_to_char
			("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	if (islower (*admin))
		*admin = toupper (*admin);

	if (*clan && islower (*clan))
		*clan = toupper (*clan);

	if (!str_cmp (buf, "add"))
	{
		mysql_safe_query ("INSERT INTO clan_assignments VALUES ('%s', '%s')",
			clan, admin);
		send_to_char
			("The specified assignment has been added to the database.\n", ch);
		return;
	}
	else if (!str_cmp (buf, "remove"))
	{
		if (*clan)
			mysql_safe_query
			("DELETE FROM clan_assignments WHERE clan_name = '%s' AND imm_name = '%s'",
			clan, admin);
		else
			mysql_safe_query
			("DELETE FROM clan_assignments WHERE imm_name = '%s'", admin);
		if (*clan)
			send_to_char
			("The specified assignment has been removed from the database.\n",
			ch);
		else
			send_to_char
			("All clan assignments for the specified admin have been removed.\n",
			ch);
		return;
	}

	send_to_char ("Usage: assign (add | remove) <admin name> <clan name>\n",
		ch);
	return;
}

void
notify_captors (CHAR_DATA * ch)
{
	CLAN_DATA *clan_def = NULL;
	DESCRIPTOR_DATA *d;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	int clan_flags = 0;

	if (!ch->room)
		return;

	if (!is_in_cell (ch, ch->room->zone))
		return;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!d->character || d->connected != CON_PLYNG)
			continue;
		if (!d->character->room)
			continue;
		p = d->character->clans;
		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			if ((clan_def = get_clandef (clan_name)))
				strcpy (clan_name, clan_def->literal);
			if (!clan_def)
				continue;
			if (!clan_def->zone)
				continue;
			if (clan_def->zone == ch->room->zone)
			{
				send_to_char
					("\n#3A PC prisoner has logged into one of your cells and requires attention.#0\n",
					d->character);
				break;
			}
		}
	}
}

void
show_waiting_prisoners (CHAR_DATA * ch)
{
	CLAN_DATA *clan_def = NULL;
	DESCRIPTOR_DATA *d;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int clan_flags = 0, prisoners = 0;

	p = ch->clans;

	while (get_next_clan (&p, clan_name, &clan_flags))
	{
		if ((clan_def = get_clandef (clan_name)))
			strcpy (clan_name, clan_def->literal);
		if (!clan_def)
			continue;
		if (!clan_def->zone)
			continue;
		if (!zone_table[clan_def->zone].jailer)
			continue;
		for (d = descriptor_list; d; d = d->next)
		{
			if (!d->character || d->connected != CON_PLYNG)
				continue;
			if (!d->character->room)
				continue;
			if (is_area_enforcer (d->character))
				continue;
			if (is_in_cell (d->character, clan_def->zone))
				prisoners++;
		}
	}

	if (prisoners > 0)
	{
		sprintf (buf,
			"\n#3There %s currently %d PC prisoner%s awaiting your attention.\n",
			prisoners != 1 ? "are" : "is", prisoners,
			prisoners != 1 ? "s" : "");
		send_to_char (buf, ch);
	}
}


void
do_promote (CHAR_DATA * ch, char *argument, int cmd)
{
	int nLeaderRank = 0;
	int nLackeyRank = 0;
	CHAR_DATA *pLackey;
	CLAN_DATA *pClan;
	char strLackey[AVG_STRING_LENGTH] = "\0";
	char strClan[AVG_STRING_LENGTH] = "\0";
	char strRank[AVG_STRING_LENGTH] = "\0";
	char strOutput[AVG_STRING_LENGTH] = "\0";
	char strUsage[] = "Usage: promote <character> <clan> <rank>.\n\0";


	argument = one_argument (argument, strLackey);

	if (!*strLackey)
	{
		send_to_char (strUsage, ch);
		return;
	}

	if (!(pLackey = get_char_room_vis (ch, strLackey)))
	{
		send_to_char ("Nobody is here by that name.\n", ch);
		send_to_char (strUsage, ch);
		return;
	}

	if (pLackey == ch)
	{
		switch (number (1, 3))
		{
		case 1:
			send_to_char ("Your delusions of grandeur go largely unnoticed.\n",
				ch);
			break;
		case 2:
			send_to_char ("You give yourself a pat on the back.\n", ch);
			break;
		default:
			send_to_char
				("Unfortunately, promotions just don't work that way...\n", ch);
			break;
		}
		send_to_char (strUsage, ch);
		return;
	}

	argument = one_argument (argument, strClan);

	if (!*strClan)
	{
		send_to_char ("Which clan did you wish to make the promotion in?\n",
			ch);
		return;
	}

	if (!get_clan_long_short (ch, strClan, &nLeaderRank))
	{
		send_to_char ("You are not a member of such a clan.\n", ch);
		return;
	}

	if (!get_clan_long_short (pLackey, strClan, &nLackeyRank))
	{
		act ("$N is not a clan member.", false, ch, 0, pLackey, TO_CHAR);
		return;
	}

	/* Keep clan_name as the short name */
	if ((pClan = get_clandef_long (strClan)))
		strcpy (strClan, pClan->name);

	argument = one_argument (argument, strRank);

	if (!*strRank)
	{
		send_to_char (strUsage, ch);
		return;
	}

	//  if (!(nLackeyRank = clan_flags_to_value (strRank)))
	if (!(nLackeyRank = clan_flags_to_value (strRank, strClan)))
	{
		send_to_char ("I don't recognize the specified rank.\n", ch);
		return;
	}

	if (!IS_SET (nLeaderRank, CLAN_LEADER)
		&& (nLeaderRank < CLAN_CORPORAL || nLeaderRank > CLAN_COMMANDER)
		&& !IS_SET (nLeaderRank, CLAN_MASTER))
	{
		send_to_char ("You are not a officer or leader of that clan.\n", ch);
		return;
	}

	if (nLeaderRank >= CLAN_CORPORAL && nLeaderRank <= CLAN_COMMANDER
		&& (nLackeyRank < CLAN_RECRUIT || nLackeyRank > CLAN_COMMANDER) && !strcmp(strClan, "khagdu"))
	{
		send_to_char
			("You'll need to specify a military rank, e.g. Recruit, Private, etc.\n",
			ch);
		return;
	}
	else if (nLeaderRank >= CLAN_APPRENTICE && nLeaderRank <= CLAN_MASTER
		&& (nLackeyRank < CLAN_APPRENTICE || nLackeyRank > CLAN_MASTER))
	{
		send_to_char
			("You'll need to specify a guild rank, e.g. Apprentice, Journeyman, etc.\n",
			ch);
		return;
	}
	if (nLackeyRank >= nLeaderRank)
	{
		send_to_char ("You do not have the authority to make this promotion.\n",
			ch);
		return;
	}

	sprintf (strOutput, "You promote $N to the rank of %s.",
		get_clan_rank_name (pLackey, strClan, nLackeyRank));
	act (strOutput, false, ch, 0, pLackey, TO_CHAR | _ACT_FORMAT);

	sprintf (strOutput, "$n has promoted you to the rank of %s.",
		get_clan_rank_name (pLackey, strClan, nLackeyRank));
	act (strOutput, false, ch, 0, pLackey, TO_VICT | _ACT_FORMAT);

	act ("$n has promoted $N.", false, ch, 0, pLackey,
		TO_NOTVICT | _ACT_FORMAT);

	add_clan (pLackey, strClan, nLackeyRank);
}

bool
outranks (char *has_rank, char *compared_rank, char *clan)
{
	int rank, compare;
	rank = clan_flags_to_value(has_rank, clan);
	compare = clan_flags_to_value(compared_rank, clan);

	if (rank >= CLAN_RECRUIT && rank <= CLAN_COMMANDER)
	{
		if (rank == CLAN_RECRUIT)
			return false;

		if (compare == CLAN_MEMBER)
			return true;

		if (compare < CLAN_RECRUIT || compare > CLAN_COMMANDER)
			return false;

		if (rank > compare)
			return true;
	}

	if (rank == CLAN_LEADER)
	{
		if (compare == CLAN_MEMBER)
			return true;
		return false;
	}

	if (rank >= CLAN_APPRENTICE && rank <= CLAN_MASTER)
	{
		if (rank == CLAN_APPRENTICE)
			return false;

		if (compare == CLAN_MEMBER)
			return true;

		if (compare < CLAN_APPRENTICE || compare > CLAN_MASTER)
			return false;

		if (rank > compare)
			return true;
	}
	return false;

}

void
do_invite (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags;
	char *p;
	CHAR_DATA *tch;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Invite who?\n\r", ch);
		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n", ch);
		return;
	}

	if (tch == ch)
	{
		send_to_char ("You can't invite yourself.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		p = ch->clans;

		if (!get_next_leader (&p, clan_name, &clan_flags))
		{
			send_to_char ("You are not an initiated leader of any clan.\n", ch);
			return;
		}

		if ((clan = get_clandef (clan_name)))
		{
			send_to_char ("That clan could not be found in the database.\n",
				ch);
			return;
		}

		strcpy (buf, clan->name);
	}

	else
	{
		if (!get_clan_long_short (ch, buf, &clan_flags))
		{
			send_to_char ("You are not a member of such a clan.\n", ch);
			return;
		}

		if (!IS_SET (clan_flags, CLAN_LEADER)
			&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
			&& !IS_SET (clan_flags, CLAN_MASTER)
			&& !IS_SET (clan_flags, CLAN_JOURNEYMAN))
		{
			send_to_char ("You are not a leader of that clan.\n", ch);
			return;
		}

		/* Keep clan_name as the short name */

		if ((clan = get_clandef_long (buf)))
			strcpy (buf, clan->name);
	}

	if (get_clan (tch, buf, &clan_flags))
	{
		act ("$N is already a clan member.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (tch->delay || tch->fighting)
	{
		act ("$N is busy right now.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	if (!AWAKE (tch))
	{
		act ("$N isn't conscious right now.", false, ch, 0, tch, TO_CHAR);
		return;
	}

	clan = get_clandef (buf);

	tch->delay = IS_NPC (tch) && !tch->desc ? 3 : 120;
	tch->delay_type = DEL_INVITE;
	tch->delay_ch = ch;
	tch->delay_who = duplicateString (buf);

	sprintf (buf, "You invite $N to join %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, ch, 0, tch, TO_CHAR);

	sprintf (buf, "$N invites you to join %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, tch, 0, ch, TO_CHAR);

	act ("$N whispers something to $n about joining a clan.",
		false, tch, 0, ch, TO_NOTVICT | _ACT_FORMAT);

}

void
invite_accept (CHAR_DATA * ch, char *argument)
{
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_STRING_LENGTH];

	if (!ch->delay_who || !*ch->delay_who)
		return;

	ch->delay = 0;

	strcpy (clan_name, ch->delay_who);
	free_mem (ch->delay_who);
	ch->delay_who = NULL;

	if (!is_he_here (ch, ch->delay_ch, 1))
	{
		send_to_char ("Too late.\n", ch);
		return;
	}

	add_clan (ch, clan_name, CLAN_MEMBER);

	act ("$N accepts your invitation.", false, ch->delay_ch, 0, ch, TO_CHAR);

	clan = get_clandef (clan_name);

	sprintf (buf, "You have been initiated into %s.",
		clan ? clan->literal : clan_name);

	act (buf, false, ch, 0, 0, TO_CHAR);
}

void
do_recruit (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *pal;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_leader (ch, pal))
				break;
		}

		if (!pal)
		{
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		pal->following = ch;
		// ch->group->insert (pal);

		act ("$N nods to you to follow.", false, pal, 0, ch,
			TO_CHAR | _ACT_FORMAT);
		act ("$N motions to $S clanmates.", false, pal, 0, ch,
			TO_NOTVICT | _ACT_FORMAT);
		act ("$n falls in.", false, pal, 0, ch, TO_ROOM | _ACT_FORMAT);
		return;
	}

	if (!str_cmp (buf, "all"))
	{
		act ("$n motions to $s clanmates.", false, ch, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_leader (ch, pal))
			{
				pal->following = ch;
				// ch->group->insert (pal);
				act ("$N nods to you to follow.", false, pal, 0, ch,
					TO_CHAR | _ACT_FORMAT);
				act ("$n falls in.", false, pal, 0, ch, TO_ROOM | _ACT_FORMAT);
			}
		}

		if (!pal)
		{
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		return;
	}

	if (!(pal = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if (pal == ch)
	{
		send_to_char ("Not yourself.\n\r", ch);
		return;
	}

	if (!is_leader (ch, pal))
	{
		act ("You don't have the authority to recruit $N.", false, ch, 0, pal,
			TO_CHAR);
		return;
	}

	if (pal->following == ch)
	{
		act ("$N is already following you.", false, ch, 0, pal, TO_CHAR);
		return;
	}

	pal->following = ch;
	// ch->group->insert (pal);

	act ("$N motions to $S clanmates.", false, pal, 0, ch,
		TO_NOTVICT | _ACT_FORMAT);
	act ("$n falls in.", false, pal, 0, ch, TO_VICT | _ACT_FORMAT);
	act ("$N nods to you to follow.", false, pal, 0, ch, TO_CHAR);
}

void
do_disband (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *pal;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf || !str_cmp (buf, "all"))
	{

		act ("$n motions to $s clanmates.", false, ch, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		act ("You motion to your clanmates.", false, ch, 0, 0,
			TO_CHAR | _ACT_FORMAT);

		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{

			if (pal->following != ch || !IS_NPC (pal))
				continue;

			if (is_leader (ch, pal) && AWAKE (pal))
			{
				act ("$N motions to you to stop following.",
					false, pal, 0, ch, TO_CHAR | _ACT_FORMAT);
				act ("$n falls out of step.", false, pal, 0, ch,
					TO_ROOM | _ACT_FORMAT);
				pal->following = 0;
				// ch->group->erase (pal);
			}
		}

		return;
	}

	if (!(pal = get_char_room_vis (ch, buf)))
	{
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if (pal->following != ch)
	{
		act ("$N is not following you.", false, ch, 0, pal, TO_CHAR);
		return;
	}

	if (!is_leader (ch, pal))
	{
		act ("You can't give $N orders.", false, ch, 0, pal, TO_CHAR);
		return;
	}

	if (!IS_NPC (pal))
	{
		send_to_char ("This command does not work on PCs.\n", ch);
		return;
	}

	pal->following = 0;
	// ch->group->erase (pal);

	act ("You motion to $N.", false, ch, 0, pal, TO_CHAR | _ACT_FORMAT);
	act ("$N motions to $n.", false, pal, 0, ch, TO_NOTVICT | _ACT_FORMAT);
	act ("$N motions to you to stop following.", false, pal, 0, ch, TO_CHAR);
	act ("$n falls out of step.", false, pal, 0, ch, TO_ROOM | _ACT_FORMAT);
}

void
do_castout (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags = 0, clan_flags2 = 0;
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	bool found = false;
	bool load_tag = false;
	char *p;
	char clan_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		send_to_char ("Castout whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room_vis (ch, buf)))
	{
		found = true;
	}
	else if (!found && (victim = get_pc(buf)))
	{
		found = true;
	}
	else if (!found && (victim = load_pc(buf)))
	{
		found = true;
		load_tag = true;
	}
	else
	{
		sprintf (buf2,"There is no one with that name.\n\r");
	}

	if (!found)
	{
		send_to_char (buf2, ch);
		return;
	}


	argument = one_argument (argument, buf);

	p = ch->clans;

	if (!*buf)
	{
		if (!get_next_leader (&p, clan_name, &clan_flags))
		{
			send_to_char
				("You are not a leader in that clan.  You can't cast out anybody.\n",
				ch);
			if (load_tag)
				unload_pc(victim);
			return;
		}

		clan = get_clandef (clan_name);
		if (!clan)
		{
			send_to_char ("No such clan, I am afraid.\n", ch);
			if (load_tag)
				unload_pc(victim);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else if (get_clan_long_short (ch, buf, &clan_flags))
	{
		if (!IS_SET (clan_flags, CLAN_LEADER)
			&& (clan_flags < CLAN_SERGEANT || clan_flags > CLAN_COMMANDER)
			&& !IS_SET (clan_flags, CLAN_MASTER))
		{
			send_to_char
				("You are not a leader in that clan.  You cannot cast anyone out.\n",
				ch);
			if (load_tag)
				unload_pc(victim);
			return;
		}

		clan = get_clandef_long (buf);
		if (!clan)
		{
			send_to_char ("No such clan, I am afraid.\n", ch);
			if (load_tag)
				unload_pc(victim);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else
	{
		send_to_char ("You are not a member of such a clan.\n", ch);
		if (load_tag)
			unload_pc(victim);
		return;
	}

	if (!get_clan (victim, clan_name, &clan_flags2))
	{
		act ("$N is not part of your clan.\n", false, ch, 0, victim, TO_CHAR);
		if (load_tag)
			unload_pc(victim);
		return;
	}

	if (IS_SET (clan_flags, CLAN_LEADER) || clan_flags <= clan_flags2)
	{
		act ("You are not of sufficient rank to cast $N from the clan.\n",
			false, ch, 0, victim, TO_CHAR | _ACT_FORMAT);
		if (load_tag)
			unload_pc(victim);
		return;
	}

	remove_clan (victim, clan_name);

	sprintf (buf, "$N is no longer a part of %s.", clan->literal);
	act (buf, false, ch, 0, victim, TO_CHAR);
	sprintf (buf, "$n has stripped you of your clan membership in %s.",
		clan->literal);
	act (buf, false, ch, 0, victim, TO_VICT | _ACT_FORMAT);
	sprintf (buf, "$n has stripped $N of membership in %s.", clan->literal);
	act ("$n has stripped $N of clan membership.", false, ch, 0, victim,
		TO_NOTVICT | _ACT_FORMAT);

	if (load_tag)
		unload_pc(victim);
}


/* called by db.c:boot_objects */
void
clan__assert_member_objs ()
{
	char buf[AVG_STRING_LENGTH];
	CLAN_DATA *clan;
	OBJ_DATA *obj;
	int i = 0;

	for (clan = clan_list; clan; clan = clan->next)
	{

		if (clan->leader_vnum)
		{

			if (!(obj = vtoo (clan->leader_vnum)))
			{
				sprintf (buf,
					"Note:  Clan leader obj %d does not exist for %d.",
					clan->leader_vnum, i);
				system_log (buf, true);
			}
			else
			{
				obj->obj_flags.extra_flags |= ITEM_LEADER;
			}

		}

		if (clan->member_vnum)
		{

			if (!(obj = vtoo (clan->member_vnum)))
			{
				sprintf (buf,
					"Note:  Clan member obj %d does not exist for %d.",
					clan->member_vnum, i);
				system_log (buf, true);
			}
			else
			{
				obj->obj_flags.extra_flags |= ITEM_MEMBER;
			}

		}

		if (clan->omni_vnum)
		{

			if (!(obj = vtoo (clan->omni_vnum)))
			{
				sprintf (buf, "Note:  Clan omni obj %d does not exist for %d.",
					clan->omni_vnum, i);
				system_log (buf, true);
			}
			else
			{
				obj->obj_flags.extra_flags |= ITEM_OMNI;
			}

		}
	}

	return;
}

/*********************************************************************
*                                                                    *
*  clan->do_add                                                      *
*                                                                    *
*  Inserts this clan's definition into the clans database.           *
*  TODO: Update definition on duplicate clan->name ?                 *
*  TODO: Escape apostrophes in clan->name and clan->literal          *
*                                                                    *
*********************************************************************/
void
clan__do_add (CLAN_DATA * clan)
{
	char buf[AVG_STRING_LENGTH * 2] = "";

	sprintf (buf, "INSERT INTO shadows_pfiles.clans "
		"(name,long_name,zone,member_obj,leader_obj,omni_obj) "
		"VALUES ('%s','%s', %d, %d, %d, %d) ;",
		clan->name,
		clan->literal,
		clan->zone, clan->member_vnum, clan->leader_vnum, clan->omni_vnum);

	mysql_safe_query (buf);
}


/*********************************************************************
*                                                                    *
*  clan->do_update                                                   *
*                                                                    *
*  Updates this clan's definition in the clans database.             *
*  TODO: Update definition based on shadows_pfiles.clans.id          *
*  TODO: Escape apostrophes in clan->name and clan->literal          *
*                                                                    *
*********************************************************************/
void
clan__do_update (CLAN_DATA * clan)
{
	char buf[AVG_STRING_LENGTH * 2] = "";

	sprintf (buf,
		"UPDATE shadows_pfiles.clans "
		"SET long_name = '%s', zone = %d, member_obj = %d, leader_obj = %d, omni_obj = %d "
		"WHERE name = '%s' ;",
		clan->literal,
		clan->zone,
		clan->member_vnum, clan->leader_vnum, clan->omni_vnum, clan->name);

	mysql_safe_query (buf);
}


/*********************************************************************
*                                                                    *
*  clan->do_remove                                                   *
*                                                                    *
*  Deletes this clan's definition from the clans database.           *
*                                                                    *
*********************************************************************/
void
clan__do_remove (CLAN_DATA * clan)
{
	std::string player_db = engine.get_config ("player_db");
	mysql_safe_query
		("DELETE FROM %s.clans "
		"WHERE name = '%s' ;",
		player_db.c_str (), clan->name);
}

/*********************************************************************
*                                                                    *
*  clan->do_load                                                     *
*                                                                    *
*  Loads the database of clan definition into the game               *
*                                                                    *
*********************************************************************/
void
clan__do_load ()
{
	CLAN_DATA *clan = NULL, *last_clan = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[AVG_STRING_LENGTH] = "";

	std::string player_db = engine.get_config ("player_db");
	mysql_safe_query
		("SELECT name,long_name,zone,member_obj,leader_obj,omni_obj"
		" FROM %s.clans"
		" ORDER BY name ;",
		player_db.c_str ());

	if (!(result = mysql_store_result (database)))
	{
		sprintf (buf, "clan__do_load: %s", mysql_error (database));
		system_log (buf, true);
		return;
	}
	while ((row = mysql_fetch_row (result)))
	{
		clan = new CLAN_DATA;
		clan->name = duplicateString (row[0]);
		clan->zone = strtol (row[2], NULL, 0);
		clan->literal = duplicateString (row[1]);
		clan->member_vnum = strtol (row[3], NULL, 0);
		clan->leader_vnum = strtol (row[4], NULL, 0);
		clan->omni_vnum = strtol (row[5], NULL, 0);

		if (clan_list)
		{
			clan->next = NULL;
			last_clan->next = clan;
		}
		else
		{
			clan->next = clan_list;
			clan_list = clan;
		}
		last_clan = clan;
	}
	mysql_free_result (result);
	result = NULL;
}

void
clan__do_score (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH] = "";
	int clan_flags = 0;
	char clan_name[MAX_STRING_LENGTH] = { '\0' };
	char clan_alias[MAX_STRING_LENGTH] = { '\0' };
	CLAN_DATA *clan_def = NULL;
	char *p = '\0';

	if (ch->clans && *ch->clans)
	{

		send_to_char ("\nYou are affiliated with the following clans:\n\n", ch);

		p = ch->clans;

		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			strcpy (clan_alias, clan_name);
			if ((clan_def = get_clandef (clan_name)))
				strcpy (clan_name, clan_def->literal);

			sprintf (buf, "  %-40s ", clan_name);

			/*	    if (IS_SET (clan_flags, CLAN_LEADER))
			strcat (buf, "#2(Leader)#0  ");
			else if (IS_SET (clan_flags, CLAN_RECRUIT))
			strcat (buf, "#2(Recruit)#0  ");
			else if (IS_SET (clan_flags, CLAN_RECRUIT))
			strcat (buf, "#2(Recruit)#0  ");
			else if (IS_SET (clan_flags, CLAN_PRIVATE))
			strcat (buf, "#2(Private)#0  ");
			else if (IS_SET (clan_flags, CLAN_CORPORAL))
			strcat (buf, "#2(Corporal)#0  ");
			else if (IS_SET (clan_flags, CLAN_SERGEANT))
			strcat (buf, "#2(Sergeant)#0  ");
			else if (IS_SET (clan_flags, CLAN_LIEUTENANT))
			strcat (buf, "#2(Lieutenant)#0  ");
			else if (IS_SET (clan_flags, CLAN_CAPTAIN))
			strcat (buf, "#2(Captain)#0  ");
			else if (IS_SET (clan_flags, CLAN_GENERAL))
			strcat (buf, "#2(General)#0  ");
			else if (IS_SET (clan_flags, CLAN_COMMANDER))
			strcat (buf, "#2(Commander)#0  ");
			else if (IS_SET (clan_flags, CLAN_APPRENTICE))
			strcat (buf, "#2(Apprentice)#0  ");
			else if (IS_SET (clan_flags, CLAN_JOURNEYMAN))
			strcat (buf, "#2(Journeyman)#0  ");
			else if (IS_SET (clan_flags, CLAN_MASTER))
			strcat (buf, "#2(Master)#0  ");
			else
			strcat (buf, "#2(Member)#0  "); */

			sprintf (buf + strlen(buf), "#2(%s)#0  ", get_clan_rank_name (ch, clan_alias, clan_flags));

			strcat (buf, "\n");
			send_to_char (buf, ch);
		}
	}
}


/* caller: staff.c:write_dynamic_registry() 	deprecated */
void
clan__filedump (FILE * fp)
{
	CLAN_DATA *clan;

	for (clan = clan_list; clan; clan = clan->next)
	{

		fprintf (fp, "clandef %s %d \"%s\" %d %d %d\n",
			clan->name,
			clan->zone,
			clan->literal,
			clan->member_vnum, clan->leader_vnum, clan->omni_vnum);
	}
}

/* removes a clan from the object */
void
clan_rem_obj (OBJ_DATA *obj, OBJ_CLAN_DATA * targ)
{
	obj->clan_data = NULL;
	free_mem (targ->name);
	free_mem (targ->rank);
	free_mem (targ);

	return;
}

/** roster <clan name>
** Will list all clanmembers who have logged in during the
** last two weeks. Only the sdesc and rank is given.
**/
void
do_rollcall (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char clanname[MAX_STRING_LENGTH];
	int clan_flags;
	MYSQL_RES *result;
	MYSQL_ROW row;
	CLAN_DATA *tclan;
	int index;


	argument = one_argument (argument, clanname);


	if (!*clanname)
	{
		send_to_char ("Which clan do you want a roster for?\n", ch);
		return;
	}

	if (!get_clan_long_short (ch, clanname, &clan_flags))
	{
		send_to_char ("You are not a member of that clan.\n", ch);
		return;
	}

	if (!IS_SET (clan_flags, CLAN_LEADER)
		&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
		&& !IS_SET (clan_flags, CLAN_MASTER)
		&& !IS_SET (clan_flags, CLAN_JOURNEYMAN))
	{
		send_to_char ("You are not a leader of that clan.\n", ch);
		return;
	}

	/* Keep clan_name as the short name */
	if ((tclan = get_clandef_long (clanname)))
		strcpy (clanname, tclan->name);



	/* search last fornight */
	result = mysql_player_search (SEARCH_CLAN, clanname, 4);

	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);


		send_to_char ("No names were found.\n", ch);
		return;
	}

	sprintf (buf, "#6Clan members:  %d#0\n\n",
		(int) mysql_num_rows (result));

	index = 1;

	while ((row = mysql_fetch_row (result)))
	{
		// row[2] is sdesc
		// row[4] is rank
		sprintf (buf2, "%4d. %-13s %s\n", index, row[4], row[2]);

		if (strlen (buf) + strlen (buf2) >= MAX_STRING_LENGTH)
			break;
		else
			sprintf (buf + strlen (buf), "%s", buf2);

		index++;
	}

	page_string (ch->desc, buf);
	mysql_free_result (result);
	return;
}

/**************************************************************\
* Function: do_setPay                 				*
* Purpose:  This function allows a clan leader to set the pay  *
*           for his/her members.               			*
\**************************************************************/

void do_setPay (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags, index, tmp, job;
	char clanname[MAX_STRING_LENGTH];
	AFFECTED_TYPE* af;
	MYSQL_RES *result;
	MYSQL_ROW row;
	CLAN_DATA* tclan;
	Stringstack arg = argument;
	bool finished = false;

	if (IS_NPC (ch))
	{
		return;
	}

	if(!*argument || arg.pop() == "?")
	{
		std::string helpStr =   "#6Add Job:\n"
			" <name>\t\t\tName of the employee\n"
			" '<clan name>'\t\tClan the employee will be working for\n"
			" <pay amount>\t\tpay PC in cash by amount\n"
			" <pay-period-days>\t# of days between payday\n"
			"\n""Delete Job:\n"
			" <name>\t\t\tName of the employee\n"
			" '<clan name>'\t\tClan the employee was working for\n"
			" delete\n#0";
		send_to_char (helpStr.c_str(), ch);
		return;
	}

	// Get Player Name (Online Players only)
	bool isFound = false;
	CHAR_DATA *tch;

	for(std::list<CHAR_DATA*>::iterator tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted)
		{
			continue;
		}
		if (isname(arg.recall(0).c_str(), tch->tname))
		{
			isFound = true;
			break;
		}
	}

	if(!isFound || IS_NPC(tch))
	{
		send_to_char ("Player not found or not online.\n", ch);
		return;
	}
	else if (tch == ch)
	{
		send_to_char ("Giving yourself a job? Unthinkable.\n", ch);
		return;
	}

	// Get Clan Name
	if(arg.isFinished()) // Is this the end of the argument?
	{
		send_to_char("You must specify a clan name.\n", ch);
		return;
	}

	arg.popSpeech(); // Moves to the next word in the argument, allowing quote use
	std::strcpy (clanname, arg.recall(1).c_str());

	// Check Clan Powers
	if(!get_clan_long_short(ch, clanname, &clan_flags))
	{
		send_to_char ("You are not a member of that clan.\n", ch);
		return;
	}
	else if(!IS_SET(clan_flags, CLAN_LEADER)
		&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
		&& !IS_SET(clan_flags, CLAN_MASTER)
		&& !IS_SET(clan_flags, CLAN_JOURNEYMAN))
	{
		send_to_char ("You are not a leader of that clan.\n", ch);
		return;
	}

	if (!get_clan_long_short(tch, clanname, &clan_flags))
	{
		send_to_char("They are not a member of that clan.\n", ch);
		return;
	}

	//Keep clan_name as the short name
	if ((tclan = get_clandef_long (clanname)))
		strcpy (clanname, tclan->name);

	//Get Pay Master
	std::string player_db = engine.get_config ("player_db");
	//Some clans return a NULL paymaster. Search sets NULL to 0 if such is the case - Vader
	mysql_safe_query("SELECT IF(pay_master IS NULL, 0, pay_master) AS pay_master FROM %s.clans WHERE name='%s'", player_db.c_str(), clanname);
	result = mysql_store_result (database);
	if (!result || !mysql_num_rows (result) || !(row = mysql_fetch_row(result)) || !row[0])
	{
		if (result)
			mysql_free_result (result);
		send_to_char ("Pay Master is not defined.\n", ch);
		return;
	}

	int employer = atoi(row[0]);
	if (!vtom (employer))
	{
		send_to_char("Pay Master is not defined.\n", ch);
		mysql_free_result(result);
		return;
	}

	if (arg.isFinished())
	{
		send_to_char("Use #6delete#0 to remove a current\njob or enter the #6pay amount#0.\n", ch);
		mysql_free_result(result);
		return;
	}

	// Check for delete
	if(arg.pop() == "delete")
	{
		for (job = JOB_1; job <= JOB_3; job++)
		{
			af = get_affect(tch, job);
			if (af != NULL && af->a.job.employer == employer)
			{
				affect_remove(tch, af);
				send_to_char("Job removed.\n", ch);
				mysql_free_result(result);
				return;
			}
		}
		send_to_char("That job wasn't assigned.\n", ch);
		mysql_free_result(result);
		return;
	}

	//Get Cash amount
	int count = arg.toInt();
	if(!count || count < 0)
	{
		send_to_char("How much did you wish to pay them?\n", ch);
		mysql_free_result(result);
		return;
	}
	if (count > 500)
	{
		send_to_char("That cash amount is a bit high.\n", ch);
		mysql_free_result(result);
		return;
	}

	//Get Pay period
	if (arg.isFinished())
	{
		send_to_char("Specify the number of days until payday.\n", ch);
		mysql_free_result(result);
		return;
	}
	arg.pop();
	int days = arg.toInt();
	if(!days || days < 0)
	{
		send_to_char("Specify the number of days until payday.\n", ch);
		mysql_free_result(result);
		return;
	}

	int pay_date = time_info.month * 30 + time_info.day + time_info.year * 12 * 30 + days;

	//Add Job
	for (job = JOB_1; job <= JOB_3; job++)
	{
		af = get_affect(tch, job);
		if (af != NULL && af->a.job.employer == employer)
		{
			send_to_char("A job from this clan already exists.\nPlease delete this job first in order to change it.\n", ch);
			mysql_free_result(result);
			return;
		}
		else if (af == NULL)
			break;
	}
	if (af != NULL)
	{
		send_to_char("This player cannot have any more paydays.", ch);
		mysql_free_result(result);
		return;
	}

	job_add_affect(tch, job, days, pay_date, count, 0, 0, employer);
	send_to_char("Job successfully added.\n", ch);
	mysql_free_result(result);
}

/*********************************************************************\
* Function: do_checkPay                     				*
* Purpose:  This function allows a clan leader to check the      	*
*           various pay amount and pay shcedules for their      	*
*           members.                        				*
\*********************************************************************/

void do_checkPay (CHAR_DATA * ch, char *argument, int cmd)
{
	int clan_flags = 0, index = 1, job, payday, cash, count, tEmployer, ovnum;
	MYSQL_RES *result;
	MYSQL_RES *p_result;
	MYSQL_ROW row;
	MYSQL_ROW p_row;
	CLAN_DATA *tclan;
	CHAR_DATA *tch;
	OBJ_DATA *cost;
	AFFECTED_TYPE* af;
	char buf[MAX_STRING_LENGTH], clanname[MAX_STRING_LENGTH];
	std::ostringstream CheckPay;
	Stringstack arg;

	//NPCs cannot use this command.
	if (IS_NPC (ch))
	{
		return;
	}

	//checkPay takes only 1 argument. It's faster to use one_argument in such cases.
	argument = one_argument(argument, clanname);

	//Display soft help if either no argument or '?' is entered.
	if(!*clanname || clanname == "?")
	{
		send_to_char ("Usage: checkpay \"<clan name>\"\n", ch);
		return;
	}

	//Player must be a member and leader of the clan they are attempting to get job info for.
	if (!get_clan_long_short (ch, clanname, &clan_flags))
	{
		send_to_char ("You are not a member of that clan.\n", ch);
		return;
	}

	if(!IS_SET(clan_flags, CLAN_LEADER)
		&& (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER)
		&& !IS_SET(clan_flags, CLAN_MASTER)
		&& !IS_SET(clan_flags, CLAN_JOURNEYMAN))
	{
		send_to_char ("You are not a leader of that clan.\n", ch);
		return;
	}

	//Keep clan_name as the short name
	if ((tclan = get_clandef_long (clanname)))
		strcpy (clanname, tclan->name);

	//Return the paymaster vnum for the clan entered. If pay_master is NULL, return 0 instead.
	std::string player_db = engine.get_config ("player_db");
	mysql_safe_query("SELECT IF(pay_master IS NULL, 0, pay_master) AS pay_master FROM %s.clans WHERE name='%s'", player_db.c_str(), clanname);
	result = mysql_store_result (database);

	if (!result || !mysql_num_rows (result) || !(row = mysql_fetch_row(result)) || !row[0])
	{
		if (result)
			mysql_free_result (result);
		send_to_char ("Pay Master is not defined.\n", ch);
		return;
	}

	int employer = atoi(row[0]);
	if (!vtom (employer))
	{
		send_to_char("Pay Master is not defined.\n", ch);
		mysql_free_result(result);
		return;
	}


	/*In order to retrieve job information without using load_pc() to get CHAR_DATA, we must find the appropriate Affect string and pass it
	as an argument. To do this we use the following mysql query. The affects field contains job information, all crafts, skill blocks,
	pending notifies, etc. The query searches through this list to find a match to the employer vnum of the clan, then steps back between 24 to 30
	positions (this varies based on where the job is located in the affects field) before stepping forward to find a string that matches "Affect\t60"
	(Jobs always begin with this string. A few other affects do as well, this is why we must find the employer number first; this also makes sure we have the correct job)
	then will return "Affect\t60" to "\n" (indicating the end of the job string) as the payday.

	Limits: Level (NO IMMS), lastlogon (2 Weeks), strlen (In the rare occurance that the employer vnum is found elsewhere)*/

	mysql_safe_query(
		"SELECT name, rank, sdesc, create_state, SUBSTRING(affects, strpos, (LOCATE('\\n', affects, strpos) - strpos)) AS payday"
		" FROM ("
		"SELECT"
		" name, sdesc, create_state, affects,"
		" TRIM(BOTH '\\'' FROM SUBSTRING_INDEX(LEFT(clans,LOCATE('%s',clans)-2),' ',-1)) AS rank,"
		" LOCATE('Affect\t60', affects, LOCATE('%d', affects) - IF(LOCATE('%d', affects) < 30, LOCATE('%d', affects) - 1, 30)) AS strpos,"
		" LOCATE('%d', affects) AS strpos2 FROM %s.pfiles"
		" WHERE lastlogon > (UNIX_TIMESTAMP() - 1209600) AND level = 0 AND clans LIKE '%%%s%%'"
		") AS subqry WHERE strpos2 - strpos > 20 ORDER BY rank", clanname, employer, employer, employer, employer, player_db.c_str(), clanname);

	p_result = mysql_store_result(database);

	if (!p_result || !mysql_num_rows (p_result))
	{
		if (p_result)
			mysql_free_result (p_result);
		mysql_free_result(result);
		send_to_char ("No names were found.\n", ch);
		return;
	}

	// loop through all clan members search for job status.
	CheckPay << "#6Clan members with jobs:\n#0";
	while (p_row = mysql_fetch_row (p_result))
	{
		arg = p_row[4];                  		  	   //Pass the job string as a Stringstack to break it down into integer fields.
		arg.argReplace("Affect\t", "");         	   //Jobs are returned as "Affect\t600 30 854623 30 0 11087 0". Remove the beginning so we are left with "600 30 854623 30 0 11087 0"
		arg.popAll();                     		  	   //Popping the entire argument leaves fewer lines of code than popping one at a time. Possibly more efficient?
		job = atoi(arg.recall(0).c_str());          	   //The first argument is the job. 600 = job 1, 601 = job 2, 603 = job 3
		payday = atoi(arg.recall(1).c_str());       	   //Payday is the days between payday, not the date. This is the second argument of the job string.
		//Recall(2) is skipped. It's not needed in this function.
		cash = atoi(arg.recall(3).c_str());         	   //Cash is the total amount of cp that the employee gets.
		if (!cash)                     		  	   //If there is a cash value of 0, then the employee gets paid out of thin air and we take the next value.
		{
			count = atoi(arg.recall(4).c_str()); 	   //Count is the number of objects recieved from the job, not necessarily the total amount of cp (could be paid in 5's, not 1's)
			ovnum = atoi(arg.recall(6).c_str()); 	   //If cash amount is 0, then there will be an ovnum in the job
			//(ovnums can only be assigned in a job by staff and do_job makes sure count and ovnum are entered together)
			cost = vtoo(ovnum);				   //Get the object that matches the ovnum. Again, do_job makes sure this is legit.
			count *= cost->farthings + cost->silver * 4; //Multiply the number of objects by the object cost to determine to total amount the employee gets.
		}
		if (cash)
			count = cash;                  		   //If cash is true, make count equal cash.
		if (atoi(p_row[3]) == STATE_APPROVED)         	   //Only print out players jobs that are still alive.
		{
			sprintf (buf, "\n%d. [#6%s#0] #5%s#0: #3%dcp #0every #2%d#0 days",
				index++, p_row[1], p_row[2], count, payday);
			if (!cash)
				sprintf (buf + strlen(buf), "#1!#0"); //Jobs that get paid out of thin air will end in a red '!'. WE DON'T WANT THESE TYPE OF JOBS.
			else
				sprintf(buf + strlen(buf), ".");      //Regular jobs end in a period.
		}
		CheckPay << buf;
	}

	send_to_char(CheckPay.str().c_str(), ch);
	mysql_free_result (result);
	mysql_free_result (p_result);
	return;
}
