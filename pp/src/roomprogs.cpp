/*------------------------------------------------------------------------\
|  roomprogs.c : Room Scripting Module                www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "clan.h"
#include "group.h"

/* script program commands */
#define MAKE_STRING(msg) \  
(((std::ostringstream&) (std::ostringstream() << std::boolalpha << msg)).str())  
#define RP_ATECHO 	0
#define RP_GIVE		1
#define RP_TAKE		2
#define RP_TRANS	3
#define RP_LOAD		4
#define RP_FORCE	5
#define RP_LINK		6
#define RP_EXIT		7
#define RP_UNLOCK	8
#define RP_ATLOOK	9
#define RP_VSTR		10
#define RP_OSTR		11
#define RP_UNLINK	12
#define RP_UNEXIT	13
#define RP_PUT		14
#define RP_GET		15
#define RP_LOCK		16
#define RP_GETCASH	17
#define RP_GIVECASH	18
#define RP_LOADMOB	19
#define RP_EXMOB	20
#define RP_IF		21
#define RP_FI		22
#define RP_ELSE		23
#define RP_RFTOG	24
#define RP_PAIN		25
#define RP_VBR		26	/* Force line break to char */
#define RP_TRANSMOB	27
#define RP_ATREAD	28
#define RP_HALT		29
#define RP_PURGE	30
#define RP_LOAD_CLONE   31
#define RP_LOADOBJ	32
#define RP_STAYPUT	33	/* flag a mob stayput */
#define RP_ZONE_ECHO	34	/* echo to a zone (indoor and/or outdoor) */
#define RP_ATWRITE	35	/* leave a board message */
#define RP_SYSTEM	36	/* submit a system message */
#define RP_CLAN_ECHO	37	/* send a message to clan members */
#define RP_TRANS_GROUP	38	/* send a message to clan members */
#define RP_SET		39	/* send a message to clan members */
#define RP_CRIMINALIZE  40      /* criminalize a person or room */
#define RP_STRIP        41      /* takes a person's equipment and puts it all neatly in a bag */
#define RP_CLAN         42      /* adds people to a clan at a certain rank */
#define RP_TAKEMONEY 43 /* Take money from a player's inventory TBA: Take money from room too */
#define RP_DELAY 44 /* Delayed command just like scommand */
#define RP_TEACH 45 /* Teach character skill */
#define RP_DOITANYWAY 46
#define RP_DOOR 47
#define RP_MATH 48
#define RP_NOOP 49 /* Do nothing - i.e. for comments */
#define RP_TRANSOBJ 50
#define RP_OBR 51

extern std::multimap<int, room_prog> mob_prog_list;

struct room_prog_var
{
	std::string data;
	std::string name;
	room_prog_var *next_var;
};


void rxp (CHAR_DATA * ch, char *prg, char *, char *, char *);
char *next_line (char *old);
int doit(CHAR_DATA *ch, const char *func, char *arg, char *, char *, char *, room_prog_var *&);
void r_link (CHAR_DATA * ch, char *argument);
void r_unlink (CHAR_DATA * ch, char *argument);
void r_exit (CHAR_DATA * ch, char *argument);
void r_give (CHAR_DATA * ch, char *argument);
void r_put (CHAR_DATA * ch, char *argument);
void r_get (CHAR_DATA * ch, char *argument);
void r_take (CHAR_DATA * ch, char *argument);
void r_unexit (CHAR_DATA * ch, char *argument);
void r_atlook (CHAR_DATA * ch, char *argument);
void r_atecho (CHAR_DATA * ch, char *argument);
void r_atread (CHAR_DATA * ch, char *argument);
void r_loadmob (CHAR_DATA * ch, char *argument);
void r_exmob (CHAR_DATA * ch, char *argument);
void r_rftog (CHAR_DATA * ch, char *arg);
void r_force (CHAR_DATA * ch, char *argument);
void r_pain (CHAR_DATA * ch, char *argument);
void r_transmob (CHAR_DATA * ch, char *argument);
void r_painmess (CHAR_DATA * victim, int dam);
void r_purge (CHAR_DATA * ch, char *argument);
void r_load_clone (CHAR_DATA * ch, char *argument);
void r_load_obj (CHAR_DATA *ch, char *argument);
void r_stayput (CHAR_DATA *ch, char *argument);
void r_zone_echo (CHAR_DATA *ch, char *argument);
void r_atwrite (CHAR_DATA *ch, char *argument);
void r_system (CHAR_DATA *ch, char *argument);
void r_clan_echo (CHAR_DATA *ch, char *argument);
void r_trans_group (CHAR_DATA *ch, char *argument);
void r_set (CHAR_DATA *ch, char *argument, room_prog_var *&);
void r_criminalize (CHAR_DATA *ch, char *argument);
void r_strip (CHAR_DATA *ch, char *argument);
void r_clan (CHAR_DATA *ch, char *argument);
void r_takemoney(CHAR_DATA *ch, char *argument);
void r_delay (CHAR_DATA *ch, char *argument);
void r_teach (CHAR_DATA *ch, char *argument);
void r_doitanyway (CHAR_DATA *ch, char *argument, char *, char *, char *);
void r_door (CHAR_DATA *ch, char *argument);
void r_math (CHAR_DATA *ch, char *argument, room_prog_var *&);
void r_transobj (CHAR_DATA *ch, char *argument);


#define MAX_RPRG_NEST 30
bool ifin[MAX_RPRG_NEST];
int nNest = 1;
int random_number[10] = {0,0,0,0,0,0,0,0,0,0};
int count_number[10] = {0,0,0,0,0,0,0,0,0,0};

const char *rfuncs[] = {
	"atecho",
	"give",
	"take",
	"trans",
	"load",
	"force",
	"link",
	"exit",
	"unlock",
	"atlook",
	"vstr",
	"ostr",
	"unlink",
	"unexit",
	"put",
	"get",
	"lock",
	"getcash",
	"givecash",
	"loadmob",
	"exmob",
	"if",
	"fi",
	"else",
	"rftog",
	"pain",
	"vbr",
	"transmob",
	"atread",
	"halt",
	"purge",
	"load_clone",
	"loadobj",
	"stayput",
	"zone_echo",
	"atwrite",
	"system",
	"clan_echo",
	"trans_group",
	"set",
	"criminalize",
	"strip",
	"clan",
	"takemoney",
	"delay",
	"teach",
	"doitanyway",
	"door",
	"math",
	"noop",
	"transobj",
	"obr",
	"\n"
};

bool
is_variable_in_list (room_prog_var *& variable_list, std::string variable_name)
{
	room_prog_var * tvar;

	if (!variable_list)
		return false;

	for (tvar = variable_list; tvar; tvar = tvar->next_var)
	{
		if (!tvar->name.compare(variable_name))
			return true;
	}
	return false; 
}

std::string
get_variable_data (room_prog_var *& variable_list, std::string variable_name)
{
	room_prog_var *tvar;
	std::string return_string = "";

	if (!variable_list || variable_name.empty())
		return return_string;

	for (tvar = variable_list; tvar; tvar = tvar->next_var)
	{
		if (!tvar->name.compare(variable_name))
		{
			if (!tvar->data.empty())
				return tvar->data;
		}
	}

	return return_string;
}

bool
is_variable_number (room_prog_var *& variable_list, std::string variable_name)
{
	if (!is_variable_in_list(variable_list, variable_name))
		return false;

	if (is_number(get_variable_data(variable_list, variable_name).c_str()))
		return true;

	return false;
}

void
set_variable_data (room_prog_var *& variable_list, std::string variable_name, std::string variable_data)
{
	room_prog_var *tvar;

	if (!variable_list || variable_name.empty())
		return;

	for (tvar = variable_list; tvar; tvar = tvar->next_var)
	{
		if (!tvar->name.compare(variable_name))
			tvar->data = variable_data;
	}
}

void
add_variable_to_list (room_prog_var *& variable_list, std::string variable_name, std::string variable_data)
{
	room_prog_var *tvar;

	if (!variable_list)
	{	
		variable_list = new room_prog_var;
		variable_list->name = variable_name;
		variable_list->data = variable_data;
		variable_list->next_var = NULL;
		return;
	}

	if (!variable_list->next_var)
	{
		variable_list->next_var = new room_prog_var;
		variable_list->next_var->name = variable_name;
		variable_list->next_var->data = variable_data;
		variable_list->next_var->next_var = NULL;
		return;
	}

	for (tvar = variable_list->next_var; tvar->next_var; tvar = tvar->next_var)
		;

	tvar->next_var = new room_prog_var;
	tvar->next_var->name = variable_name;
	tvar->next_var->data = variable_data;
	tvar->next_var->next_var = NULL;
	return;
}

void
delete_all_vars (room_prog_var *& variable_list)
{
	if (!variable_list)
		return;

	if (variable_list->next_var)
	{
		delete_all_vars(variable_list->next_var);
	}

	delete variable_list;
	variable_list = NULL;
	return;
}

void
delete_var_from_list (room_prog_var *& variable_list, std::string variable_name)
{
	room_prog_var * tvar, * old_tvar = NULL;

	if (!variable_list || variable_name.empty())
		return;

	for (tvar = variable_list; tvar; tvar = tvar->next_var)
	{
		if (!variable_name.compare(tvar->name))
		{
			if (old_tvar)	// Not first in list
			{
				if (tvar->next_var)
				{
					old_tvar->next_var = tvar->next_var;
				}
				else
				{
					old_tvar->next_var = NULL;
				}
			}
			else
			{
				if (tvar->next_var)
				{
					variable_list = tvar->next_var;
				}
				else
				{
					variable_list = NULL;
				}
			}
			delete tvar;
			return;
		}
		old_tvar = tvar;
	}
}

int
r_isname (char *str, char *namelist)
{
	char *curname;
	char *curstr;

	if (!str)
		return 0;

	if (!namelist)
		return 0;

	curname = namelist;
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if ((!*curstr && !isalpha (*curname)) || !str_cmp (curstr, curname))
				return (1);

			if (!*curname)
				return (0);

			if (!*curstr || *curname == ' ')
				break;

			if (tolower (*curstr) != tolower (*curname))
				break;
		}

		/* skip to next name */

		for (; isalpha (*curname); curname++);
		if (!*curname)
			return (0);
		curname++;		/* first char of new name */
	}
}

// called by command_interpreter if there is a 
// program in the user's room.
int
r_program (CHAR_DATA * ch, char *argument)
{
	char cmd[MAX_STRING_LENGTH] = {'\0'};
	char arg[MAX_STRING_LENGTH] = {'\0'};
	argument = one_argument(argument, cmd);

	if (!cmd || !*cmd)
		return 0;

	for (struct room_prog *p = ch->room->prg; p; p = p->next)
	{
		if (!p->prog || !*p->prog)
			continue;

		if (r_isname (cmd, p->command))
		{
			if (!p->keys || !*p->keys)
			{
				rxp (ch, p->prog, cmd, "", argument);
				return (1);
			}
			if (arg[0] == '\0' && !argument[0] == '\0')
				argument = one_argument(argument, arg);
			if (!arg || r_isname (arg, p->keys))
			{
				rxp (ch, p->prog, cmd, arg, argument);
				return (1);
			}
		}
	}

	return (0);
}

int
m_prog (CHAR_DATA * ch, char * argument)
{
	std::string cmd, keys, strArgument = argument;
	strArgument = one_argument (strArgument, cmd);
	strArgument = one_argument (strArgument, keys);

	if (cmd.empty())
		return 0;

	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range;
	range = mob_prog_list.equal_range(ch->mob->nVirtual); 
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (!it->second.type)
			continue;
		if (!cmd.empty() && r_isname((char *) cmd.c_str(), it->second.command))
		{
			if (!it->second.keys || !*it->second.keys)
			{
				strArgument = keys + strArgument;
				rxp(ch, it->second.prog, (char *) cmd.c_str(), "", (char *) strArgument.c_str());
				return 1;
			}

			if (!keys.empty() && r_isname ((char *) keys.c_str(), it->second.keys))
			{
				rxp (ch, it->second.prog, (char *) cmd.c_str(), (char *) keys.c_str(), (char *) strArgument.c_str());
				return 1;
			}
		}
	}
	return 0;
}

int
m_prog (CHAR_DATA *ch, char *argument, room_prog prog)
{
	std::string cmd, keys, strArgument = argument;
	strArgument = one_argument (strArgument, cmd);
	strArgument = one_argument (strArgument, keys);

	if (cmd.empty())
		return 0;

	if (prog.type)
		return 0;

	if (!cmd.empty() && r_isname((char *) cmd.c_str(), prog.command))
	{
		if (!prog.keys || !*prog.keys)
		{
			strArgument = keys + strArgument;
			rxp (ch, prog.prog, (char *) cmd.c_str(), "", (char *) strArgument.c_str());
			return 1;
		}

		if (!keys.empty() && r_isname ((char *) keys.c_str(), prog.keys))
		{
			rxp (ch, prog.prog, (char *) cmd.c_str(), (char *) keys.c_str(), (char *) strArgument.c_str());
			return 1;
		}
	}
	return 0;
}

void
rxp (CHAR_DATA *ch, char *prg, char *command, char *keyword, char *argument)
{
	// Let's use std::string
	std::string line, program = prg;
	std::stringstream streamProg;
	streamProg << program;

	// Initialize the logic table
	for (int i = 0; i < MAX_RPRG_NEST; i++)
		ifin[i] = 0;
	nNest = 1;

	// Initialize the dynamic variables
	room_prog_var * local_variables;
	local_variables = NULL;

	// Loop through the program
	do
	{
		// Spit out one line from the program

		std::getline(streamProg, line, '\n'); // std::getline is a global function (or 'algorithm') in the STL

		if (line.empty())
			break;

		// Replace Variables in the line
		while (line.find("&(") != std::string::npos)
		{
			std::string::size_type first_index = line.find_first_of("&");
			std::string::size_type index = first_index+2;
			std::string TempString = "";

			while (line[index] != ')' && index < line.length() )
			{
				TempString.push_back(line[index++]);
			}
			if (is_number(TempString.c_str()))
			{
				if (atoi(TempString.c_str()) == 0)
					line.replace(first_index, index-first_index+1, argument);
				else
				{
					std::string ArgumentList = argument, ThisArgument;
					for (int iterations = atoi(TempString.c_str()); iterations > 0; iterations--)
					{
						ArgumentList = one_argument(ArgumentList, ThisArgument);
					}
					if (ThisArgument.empty())
						line.erase(first_index, index-first_index+1);
					else
						line.replace(first_index, index-first_index+1, ThisArgument);
				}
			}
			else if (!strncmp(TempString.c_str(), "count", 5))
			{
				if (TempString.length() == 5)
					line.replace(first_index, index-first_index+1, ((std::ostringstream&) (std::ostringstream() << std::boolalpha << count_number[0])).str());
				else if (TempString.length() == 6)
				{
					if (is_number(((std::ostringstream&) (std::ostringstream() << std::boolalpha << TempString[5])).str().c_str()))
					{
						std::ostringstream conversion;
						conversion << TempString[5];
						line.replace(first_index, index-first_index+1, ((std::ostringstream&) (std::ostringstream() << std::boolalpha << count_number[atoi((conversion.str()).c_str())])).str());
					}
				}
				else
				{
					line.erase(first_index, index-first_index+1);
				}
			}
			else if (!strncmp(TempString.c_str(), "random", 6))
			{
				if (TempString.length() == 6)
					line.replace(first_index, index-first_index+1, ((std::ostringstream&) (std::ostringstream() << std::boolalpha << random_number[0])).str());
				else if (TempString.length() == 7)
				{
					if (is_number(((std::ostringstream&) (std::ostringstream() << std::boolalpha << TempString[6])).str().c_str()))
					{
						std::ostringstream conversion;
						conversion << TempString[6];
						line.replace(first_index, index-first_index+1, ((std::ostringstream&) (std::ostringstream() << std::boolalpha << random_number[atoi((conversion.str()).c_str())])).str());
					}
				}
				else
				{
					line.erase(first_index, index-first_index+1);
				}
			}
			else if (is_variable_in_list(local_variables, TempString))
			{
				line.replace(first_index, index-first_index+1, get_variable_data(local_variables, TempString));
			}
			else
			{
				line.erase(first_index, index-first_index+1);
			}
		}

		// Work out which rprog command to do
		std::string strCommand;
		line = one_argument(line, strCommand);

		char buf [MAX_STRING_LENGTH]; // doit isn't really set up to handle constant c_str so must convert to non-const
		sprintf(buf, "%s", line.c_str());
		if (!doit (ch, strCommand.c_str(), buf, command, keyword, argument, local_variables))
		{
			break;
		}
	}
	while (!line.empty());

	delete_all_vars(local_variables);
	return;
}

/* change the %'s to $'s and remove the trailing \n */
void
ref (char *str, char *return_string)
{
	*return_string = '\0';

	for (; *str; str++, return_string++)
	{
		if (*str == '%')
			*return_string = '$';
		else
			*return_string = *str;
	}

	*return_string = '\0';
}


bool
oexist (int nVirtual, OBJ_DATA * ptrContents, bool bNest)
{
	OBJ_DATA *obj = NULL;

	for (obj = ptrContents; obj; obj = obj->next_content)
	{
		if (obj->nVirtual == nVirtual)
		{
			return true;
		}
		if (bNest && obj->contains && oexist (nVirtual, obj->contains, bNest))
		{
			return true;
		}
	}
	return false;
}


/* Handles the if statement -- returns true if the if is false */
/* Yeah I know, it's backwards.....I'll change it when I have time */

void
reval (CHAR_DATA * ch, char *arg)
{
	int i, dsiz, dir, tsiz, nFlag = 0, nStat = 0;
	int nArg1 = 0, nArg2 = 0, nArg3 = 0;
	long virt = 0, who;
	char tmp[80], tmp2[80], *dbuf, rbuf[80], sarg[80];
	CHAR_DATA *tmp_ch, *tch1, *tch2;
	OBJ_DATA *obj;
	bool check;
	ROOM_DATA *troom = NULL;

	*rbuf = '\0';
	strcpy (sarg, arg);
	while (*arg != '(' && *arg != '\0')
		arg++;
	arg++;
	i = 0;
	while (*arg != ')' && *arg != '\0')
	{
		tmp[i] = *arg++;
		i++;
	}
	tmp[i++] = '\0';
	tsiz = strlen (tmp);
	strcpy (tmp2, tmp);
	if ((dbuf = strchr (tmp, ',')))
	{
		dsiz = strlen (dbuf);
		dbuf++;
		for (; isspace (*dbuf); dbuf++);
		strncat (rbuf, tmp2, (tsiz - dsiz));
	}

	// Check to see if mudhour compares with specified logic
	// Syntax: if (hour=x)
	//         if (hour>x)
	//         if (hour<x)
	//	     if (hour!x)
	//	     if (hour#x) - note, this is hour mod x, but % char not allowed in mud input.
	if (!strncmp(sarg, "(hour", 5))
	{
		int iTest = strtol(sarg+6, 0, 0);
		bool pass = false;
		switch (sarg[5])
		{
		case '=':
			pass = (iTest == time_info.hour);
			break;
		case '>':
			pass = (iTest < time_info.hour);
			break;
		case '<':
			pass = (iTest > time_info.hour);
			break;
		case '!':
			pass = (iTest != time_info.hour);
			break;
		case '#':
			pass = !(time_info.hour % iTest);
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}
	else if (!strncmp(sarg, "(day", 4))
	{
		int iTest = strtol(sarg+5, 0, 0);
		bool pass = false;
		switch (sarg[4])
		{
		case '=':
			pass = (iTest == (time_info.day+1));
			break;
		case '>':
			pass = (iTest < (time_info.day +1));
			break;
		case '<':
			pass = (iTest > (time_info.day + 1));
			break;
		case '!':
			pass = (iTest != (time_info.day+1));
			break;
		case '#':
			pass = !((time_info.day+1) % iTest);
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}
	// Note - first month is Midwinter
	else if (!strncmp(sarg, "(month", 6))
	{
		int iTest = strtol(sarg+7, 0, 0);
		bool pass = false;
		switch (sarg[6])
		{
		case '=':
			pass = (iTest == (time_info.month+1));
			break;
		case '>':
			pass = (iTest < (time_info.month+1));
			break;
		case '<':
			pass = (iTest > (time_info.month+1));
			break;
		case '!':
			pass = (iTest != (time_info.month+1));
			break;
		case '#':
			pass = !((time_info.month+1) % iTest);
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}
	else if (!strncmp(sarg, "(year", 5))
	{
		int iTest = strtol(sarg+6, 0, 0);
		bool pass = false;
		switch (sarg[5])
		{
		case '=':
			pass = (iTest == time_info.year);
			break;
		case '>':
			pass = (iTest < time_info.year);
			break;
		case '<':
			pass = (iTest > time_info.year);
			break;
		case '!':
			pass = (iTest != time_info.year);
			break;
		case '#':
			pass = !(time_info.year % iTest);
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}
	// First season (1) is Spring
	else if (!strncmp(sarg, "(season", 7))
	{
		int iTest = strtol(sarg+8, 0, 0);
		bool pass = false;
		switch (sarg[7])
		{
		case '=':
			pass = (iTest == (time_info.season+1));
			break;
		case '>':
			pass = (iTest < (time_info.season+1));
			break;
		case '<':
			pass = (iTest > (time_info.season+1));
			break;
		case '!':
			pass = (iTest != (time_info.hour+1));
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}
	else if (!strncmp(sarg, "clanrank", 8))
	{
		if (!is_clan_member(ch, rbuf))
		{
			ifin[nNest] = 1;
			return;
		}

		int flags = 0;
		get_clan (ch, rbuf, &flags);

		if (flags != clan_flags_to_value(dbuf) || flags != clan_flags_to_value(dbuf, rbuf))
		{
			if (!outranks(get_clan_rank_name(flags), dbuf, rbuf))
			{
				ifin[nNest] = 1;
				return;
			}
		}

		return;
	}

	else if (!strncmp(sarg, "fighting", 8))
	{
		if (!ch->fighting)
			ifin[nNest] = 1;
		return;
	}

	else if (!strncmp (sarg, "door", 4))
	{
		int direction;
		switch (rbuf[0])
		{
		case 'n':
			direction = 0;
			break;
		case 'e':
			direction = 1;
			break;
		case 's':
			direction = 2;
			break;
		case 'w':
			direction = 3;
			break;
		case 'u':
			direction = 4;
			break;
		case 'd':
			direction = 5;
			break;
		default:
			ifin[nNest] = 1;
			return;
		}

		if (!strcmp(dbuf, "locked") && !IS_SET(EXIT(ch, direction)->exit_info, EX_LOCKED))
			ifin[nNest] = 1;
		else if (!strcmp(dbuf, "unlocked") && IS_SET(EXIT(ch, direction)->exit_info, EX_LOCKED))
			ifin[nNest] = 1;
		else if (!strcmp(dbuf, "open") && IS_SET(EXIT(ch, direction)->exit_info, EX_CLOSED))
			ifin[nNest] = 1;
		else if (!strcmp(dbuf, "closed") && !IS_SET(EXIT(ch, direction)->exit_info, EX_CLOSED))
			ifin[nNest] = 1;
		return;
	}

	else if (!strncmp (sarg, "sector", 6))
	{
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
			"CrowsNest",
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

		if (!strncmp(rbuf, "-1", 2))
		{
			if (ch->room->sector_type != parse_argument (sector_types, dbuf))
			{
				ifin[nNest] = 1;
				return;
			}
			return;
		}
		else
			if (!vtor(atoi(rbuf)))
			{
				ifin[nNest] = 1;
				return;
			}

			if ((vtor(atoi(rbuf)))->sector_type != parse_argument(sector_types, dbuf))
			{
				ifin[nNest] = 1;
				return;
			}
			return;


	}

	/* Check to see if you can take specified money from character */
	/* Usage: if can_take_money(amount, currency) */
	else if (!strncmp (sarg, "can_take_money", 14))
	{
		int currency = 0;
		if (!strncmp (dbuf, "gondorian", 9))
		{
			currency = 0;
		}
		else if (!strncmp (dbuf, "orkish", 6) || !strncmp(dbuf, "yrkish", 6) || !strncmp(dbuf, "orcish", 6))
		{
			currency = 1;
		}
		else if (!strncmp (dbuf, "numenorean", 10))
		{
			currency = 2;
		}
		else
		{
			ifin[nNest] = 1;
			return;
		}

		if (!can_subtract_money(ch, atoi(rbuf), currency))
		{
			ifin[nNest] = 1;
		}
	}
	/* Check to see if a mob exists in a given room */
	/* Usage: if mexist(mobvnum,roomvnum)           */

	else if (!strncmp (sarg, "mexist", 6))
	{
		if (!strcmp(dbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = atol (dbuf);

		if (!strcmp(rbuf, "-1"))
		{
			if (IS_NPC(ch))
				who = ch->mob->nVirtual;
			else
			{
				ifin[nNest] = 1;
				return;
			}
		}
		else
			who = atol (rbuf);

		for (tmp_ch = vtor (virt)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room)
		{
			if (IS_NPC (tmp_ch) && tmp_ch->mob->nVirtual == who)
				break;
		}
		if (!tmp_ch)
		{
			ifin[nNest] = 1;
			return;
		}
	}

	/* Check to see if a obj exists in a given room */
	/* Usage: if oexist(objvnum,roomvnum)           */

	else if (!strncmp (sarg, "oexist_nested", 13))
	{
		if (!strcmp(dbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = atol (dbuf);
		who = atol (rbuf);
		obj = NULL;

		if (!oexist (who, vtor (virt)->contents, true))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}

	/* Check to see if a obj exists in a given room */
	/* Usage: if oexist(objvnum,roomvnum)           */

	else if (!strncmp (sarg, "oexist", 6))
	{
		if (!strcmp(dbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = atol (dbuf);
		who = atol (rbuf);
		obj = NULL;

		if (!oexist (who, vtor (virt)->contents, false))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}

	/* Check to see if a flag is set on a given room */
	/* Usage: if flag(room-flag,roomvnum)            */

	else if (!strncmp (sarg, "flag", 4) || !strncmp (sarg, "rflag", 5))
	{

		if ((nFlag = index_lookup (room_bits, rbuf)) == -1)
		{
			send_to_char ("Error: if flag: No such room-flag.\n", ch);
			ifin[nNest] = 1;
			return;
		}
		if (!strcmp(dbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = strtol (dbuf, NULL, 10);
		if (!(troom = vtor (virt)))
		{
			send_to_char ("Error: if flag: No such room.\n", ch);
			ifin[nNest] = 1;
			return;
		}
		if (!IS_SET (troom->room_flags, (1 << nFlag)))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}

	else if (!strncmp (sarg, "(count", 6))
	{
		bool digit_yes = false;
		int which_count = 0;
		if (isdigit(sarg[6]))
		{
			char conversion [8];
			sprintf(conversion, "%c", sarg[6]);
			which_count = atoi(conversion);
			digit_yes = true;
		}

		bool pass = false;
		int test_number;
		if (digit_yes)
			test_number = strtol(sarg+8,0,0);
		else
			test_number = strtol(sarg+7,0,0);

		switch (digit_yes ? sarg[7] : sarg[6])
		{
		case '=':
			pass = (count_number[which_count] == test_number);
			break;
		case '>':
			pass = (count_number[which_count] > test_number);
			break;
		case '<':
			pass = (count_number[which_count] < test_number);
			break;
		case '!':
			pass = (count_number[which_count] != test_number);
			break;
		}
		if (!pass)
			ifin[nNest] = 1;
		return;
	}

	/* test against a random number */
	else if (!strncmp (sarg, "(random", 7))
	{
		bool digit_yes = false;
		int which_random_number = 0;
		if (isdigit(sarg[7]))
		{
			char conversion[8];
			sprintf(conversion, "%c", sarg[7]);
			which_random_number = atoi(conversion);
			digit_yes = true;
		}

		int test_number;
		if (digit_yes)
			test_number = strtol (sarg+9,0,0);
		else
			test_number = strtol (sarg+8,0,0);
		bool pass = false;
		switch (digit_yes ? sarg[8] : sarg[7])
		{
		case '=':
			pass = (random_number[which_random_number] == test_number);
			break;

		case '<':
			pass = (random_number[which_random_number] < test_number);
			break;

		case '>':
			pass = (random_number[which_random_number] > test_number);
			break;

		case '!':
			pass = (random_number[which_random_number] != test_number);
			break;
		}
		if (!pass)
		{
			ifin[nNest] = 1;
		}
		return;
	}

	/* Checks if the initiator is a NPC. If so, returns true. No arguments */

	else if (!strncmp (sarg, "npc", 3))
	{
		if(!IS_NPC(ch))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}

	/* Checks to see if initiator is wanted in a certain zone */
	/* usage: if wanted(zone, time)                           */
	/* will return true if wanted time is equal to or greater than "time". Current zone is -1. */

	else if (!strncmp (sarg, "wanted", 6))
	{ 
		int zone = atol(rbuf);
		int test = atol(dbuf);

		if(zone == -1)
			zone = ch->room->zone;

		if (!get_affect (ch, MAGIC_CRIM_BASE + zone))
		{
			ifin[nNest] = 1;
			return;
		}
		else

			if (!((get_affect(ch, MAGIC_CRIM_BASE + zone)->a.spell.duration) >= test))
			{
				ifin[nNest] = 1;
				return;
			}
			return;
	}

	/* Check to see if mob/player has clanning (use shortname) */
	/* Usage: if clan(mobvnum,clanname)                        */
	/* Only checks in current room. To denote player use -1   */

	else if (!strncmp (sarg, "clan", 4))
	{
		who = strtol (rbuf, NULL, 10);
		if (who == -1)
		{
			tmp_ch = ch;
		}
		else
		{
			for (tmp_ch = vtor (ch->in_room)->people; tmp_ch;
				tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch->mob && tmp_ch->mob->nVirtual == who)
					break;
			}
			if (!tmp_ch)
			{
				ifin[nNest] = 1;
				return;
			}
		}
		if (!get_clan (tmp_ch, dbuf, &nFlag))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}
	/* Check to see if mob/player has race */
	/* Usage: if race(mobvnum,racename)                        */
	/* Only checks in current room. To denote player use -1   */

	else if (!strncmp (sarg, "race", 4))
	{
		who = strtol (rbuf, NULL, 10);
		if (who == -1)
		{
			tmp_ch = ch;
		}
		else
		{
			for (tmp_ch = vtor (ch->in_room)->people; tmp_ch;
				tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch->mob && tmp_ch->mob->nVirtual == who)
					break;
			}
			if (!tmp_ch)
			{
				ifin[nNest] = 1;
				return;
			}
		}
		if ((tmp_ch->race != atoi(dbuf)) && (tmp_ch->race != lookup_race_id (dbuf)))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}

	else if (!strncmp (sarg, "statcheck", 9))
  	{
		nStat = number(1,strtol (dbuf, NULL, 10));
		if (!str_cmp (rbuf, "str") && GET_STR (ch) >= nStat)
                {
                        return;
                }
                else if (!str_cmp (rbuf, "dex") && GET_DEX (ch) >= nStat)
                {
                        return;
                }
                else if (!str_cmp (rbuf, "con") && GET_CON (ch) >= nStat)
                {
                        return;
                }
                else if (!str_cmp (rbuf, "int") && GET_INT (ch) >= nStat)
                {
                        return;
                }
                else if (!str_cmp (rbuf, "wil") && GET_WIL (ch) >= nStat)
                {
                        return;
                }
                else if (!str_cmp (rbuf, "aur") && GET_AUR (ch) >= nStat)
                {
                        return;
                }
		else if (!str_cmp (rbuf, "agi") && GET_AGI (ch) >= nStat)
		{
			return;
		}
		ifin[nNest] = 1;
		return;
	}

	/* Check to see if player meets minimum ability score */
	/* Usage: if stat(ability,minimum)                    */

	else if (!strncmp (sarg, "stat", 4))
	{
		nStat = strtol (dbuf, NULL, 10);
		if (!str_cmp (rbuf, "str") && GET_STR (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "dex") && GET_DEX (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "con") && GET_CON (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "int") && GET_INT (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "wil") && GET_WIL (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "aur") && GET_AUR (ch) >= nStat)
		{
			return;
		}
		else if (!str_cmp (rbuf, "agi") && GET_AGI (ch) >= nStat)
		{
			return;
		}
		ifin[nNest] = 1;
		return;
	}

	/* Check to see if player meets minimum skill score */
	/* Usage: if skillcheck(skillname,XdY)                  */

	else if (!strncmp (sarg, "skillcheck", 10))
	{
		nArg2 = strtol (dbuf, NULL, 10);
		dbuf = strchr (dbuf, 'd');
		if (dbuf && dbuf++)
		{
			nArg3 = strtol (dbuf, NULL, 10);

			if (((nArg1 = index_lookup (skills, rbuf)) != -1)
				&& (skill_level (ch, nArg1, 0) >= dice (nArg2, nArg3)))
			{
				return;
			}
		}
		ifin[nNest] = 1;
		return;
	}



	/* Check to see if player meets minimum skill score */
	/* Usage: if skill(skillname,minimum)                  */

	else if (!strncmp (sarg, "skill", 5))
	{
		nArg2 = strtol (dbuf, NULL, 10);
		if (((nArg1 = index_lookup (skills, rbuf)) != -1)
			&& (skill_level (ch, nArg1, 0) >= nArg2))
		{
			return;
		}
		ifin[nNest] = 1;
		return;
	}

	/* Check to see if mob/player has object (also checks eq) */
	/* Usage: if haso(mobvnum,objvnum)                        */
	/* Only checks in current room. To denote player use -1   */

	else if (!strncmp (sarg, "haso_nested", 11))
	{
		who = atol (rbuf);
		if (who == -1)
		{
			tmp_ch = ch;
		}
		else
		{
			for (tmp_ch = vtor (ch->in_room)->people; tmp_ch;
				tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch->mob && tmp_ch->mob->nVirtual == who)
					break;
			}
			if (!tmp_ch)
			{
				ifin[nNest] = 1;
				return;
			}
		}

		obj = get_obj_in_list_num (atol (dbuf), tmp_ch->right_hand);
		if (!obj)
			obj = get_obj_in_list_num (atol (dbuf), tmp_ch->left_hand);
		if (!obj)
		{
			if ((check = get_obj_in_equip_num (tmp_ch, atol (dbuf)) == false))
			{
				ifin[nNest] = 1;
				return;
			}
		}
	}

	else if (!strncmp (sarg, "haso", 4))
	{
		who = atol (rbuf);
		if (who == -1)
		{
			tmp_ch = ch;
		}
		else
		{
			for (tmp_ch = vtor (ch->in_room)->people; tmp_ch;
				tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch->mob && tmp_ch->mob->nVirtual == who)
					break;
			}
			if (!tmp_ch)
			{
				ifin[nNest] = 1;
				return;
			}
		}

		obj = get_obj_in_list_num (atol (dbuf), tmp_ch->right_hand);
		if (!obj)
			obj = get_obj_in_list_num (atol (dbuf), tmp_ch->left_hand);
		if (!obj)
		{
			if ((check = get_obj_in_equip_num (tmp_ch, atol (dbuf)) == false))
			{
				ifin[nNest] = 1;
				return;
			}
		}
	}

	/* Check to see if mob/player can see mob/player */
	/* Usage: if cansee(seer,seen)                        */
	/* Only checks in current room. To denote player use -1   */
	/* General check for vision: if cansee(-1,-1) */

	else if (!strncmp (sarg, "cansee", 6))
	{
		who = atol (rbuf);
		if (who == -1)
		{
			tch1 = ch;
		}
		else
		{
			for (tch1 = vtor (ch->in_room)->people; tch1;
				tch1 = tch1->next_in_room)
			{
				if (tch1->mob && tch1->mob->nVirtual == who)
					break;
			}
			if (!tch1)
			{
				ifin[nNest] = 1;
				return;
			}
		}
		who = atol (dbuf);
		if (who == -1)
		{
			tch2 = ch;
		}
		else
		{
			for (tch2 = vtor (ch->in_room)->people; tch2;
				tch2 = tch2->next_in_room)
			{
				if (tch2->mob && tch2->mob->nVirtual == who)
					break;
			}
			if (!tch2)
			{
				ifin[nNest] = 1;
				return;
			}
		}
		if (!CAN_SEE (tch1, tch2))
		{
			ifin[nNest] = 1;
			return;
		}
		return;
	}



	/* Checks to see if a link exist in a given room and direction */
	/* Usage: if link(roomvnum, dir)                               */

	else if (!strncmp (sarg, "link", 4))
	{
		if (!strcmp(rbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = atol (rbuf);
		switch (*dbuf)
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
			system_log ("Unknown direction in reval::link", true);
			ifin[nNest] = 1;
			return;
		}

		if (!(troom = vtor (virt)))
		{
			system_log ("ERROR: tar room not found in reval::link", true);
			ifin[nNest] = 1;
			return;
		}

		if (!troom->dir_option[dir])
		{
			ifin[nNest] = 1;
			return;
		}

	}
	else
	{
		if (sarg[0] != '(')
		{
			ifin[nNest] = 1;
			return;
		}
		std::stringstream streamBuf;
		std::string whole, rhs, lhs;
		char delim_char = '\0';

		whole.assign(sarg);

		// make sure this string has a close paren

		if (whole.find(')') == std::string::npos)
		{
			ifin[nNest] = 1;
			return;
		}

		whole.erase(whole.begin());

		// work out which sort of comparison we're doing
		if (whole.find('=') != std::string::npos)
		{
			delim_char = '=';
		}
		else if (whole.find('!') != std::string::npos)
		{
			delim_char = '!';
		}
		else if (whole.find('<') != std::string::npos)
		{
			delim_char = '<';
		}
		else if (whole.find('>') != std::string::npos)
		{
			delim_char = '>';
		}

		if (!delim_char)
		{
			ifin[nNest] = 1;
			return;
		}

		streamBuf << whole;
		std::getline(streamBuf, lhs, delim_char);
		std::getline(streamBuf, rhs, ')');

		if (lhs.empty() || rhs.empty())
		{
			ifin[nNest] = 1;
			return;
		}

		bool invert_logic = false;
		switch (delim_char)
		{
		case '!':
			invert_logic = true;
		case '=':
			if (is_number(lhs.c_str()) && is_number(rhs.c_str()))
			{
				if (atoi(lhs.c_str()) == atoi(rhs.c_str()))
				{
					if (invert_logic)
						ifin[nNest] = 1;
					return;
				}
				else
				{
					if (!invert_logic)
						ifin[nNest] = 1;
					return;
				}
			}
			else
			{
				if (!lhs.compare(rhs))
				{
					if (invert_logic)
						ifin[nNest] = 1;
					return;
				}
				else
				{
					if (!invert_logic)
						ifin[nNest] = 1;
					return;
				}
			}
			break;
		case '<':
			invert_logic = true;
		case '>':
			if (is_number(lhs.c_str()) && is_number(rhs.c_str()))
			{
				if (atoi(lhs.c_str()) > atoi(rhs.c_str()))
				{
					if (invert_logic)
						ifin[nNest] = 1;
					return;
				}
				else
				{
					if (!invert_logic)
						ifin[nNest] = 1;
					return;
				}
			}
			else
			{
				ifin[nNest] = 1;
				return;
			}
			break;
		}
	}
}

int
doit (CHAR_DATA *ch, const char *func, char *arg, char *command, char *keyword, char *argument, room_prog_var *& variable_list)
{
	int i;
	char tmp[MAX_STRING_LENGTH];

	for (i = 0; (*rfuncs[i] != '\n'); i++)
		if (!strcmp (rfuncs[i], func))
			break;

	switch (i)
	{
	case RP_NOOP: // Comment
		return 1;
	case RP_MATH:
		if (!ifin[nNest])
			r_math(ch, arg, variable_list);
		return 1;
	case RP_ATECHO:
		if (!ifin[nNest])
			r_atecho (ch, arg);
		
		return 1;
	case RP_LOADOBJ:
		if (!ifin[nNest])
			r_load_obj (ch, arg);
		
		return 1;
	case RP_TAKEMONEY:
		if (!ifin[nNest])
			r_takemoney(ch, arg);
		
		return 1;
	case RP_EXIT:
		if (!ifin[nNest])
			r_exit (ch, arg);
		
		return 1;
	case RP_LINK:
		if (!ifin[nNest])
			r_link (ch, arg);
		
		return 1;
	case RP_ATLOOK:
		if (!ifin[nNest])
			r_atlook (ch, arg);
		
		return 1;
	case RP_TRANS:
		if (!ifin[nNest])
		{
			if (!vtor (atoi (arg)))
			{
				
				return 1;
			}
			if (ch->mount)
			{
				char_from_room (ch->mount);
				char_to_room (ch->mount, vtor (atoi (arg))->nVirtual);
			}
			char_from_room (ch);
			char_to_room (ch, vtor (atoi (arg))->nVirtual);
		}
		
		return 1;
	case RP_TRANSMOB:
		if (!ifin[nNest])
		{
			r_transmob (ch, arg);
		}
		
		return 1;
	case RP_VSTR:
		if (!ifin[nNest])
		{
			ref (arg, tmp);
			act (tmp, false, ch, 0, 0, TO_CHAR | _ACT_FORMAT);
		}
		
		return 1;
	case RP_VBR:
		if (!ifin[nNest])
			send_to_char ("\n", ch);
		
		return 1;
	case RP_OBR:
		if (!ifin[nNest])
			send_to_not_char ("\n", ch);

		return 1;
	case RP_OSTR:
		if (!ifin[nNest])
		{
			ref (arg, tmp);
			act (tmp, false, ch, 0, 0, TO_ROOM | _ACT_FORMAT);
		}
		
		return 1;
	case RP_TEACH:
		if (!ifin[nNest])
			r_teach (ch, arg);
		
		return 1;
	case RP_UNLINK:
		if (!ifin[nNest])
			r_unlink (ch, arg);
		
		return 1;
	case RP_UNEXIT:
		if (!ifin[nNest])
			r_unexit (ch, arg);
		
		return 1;
	case RP_GIVE:
		if (!ifin[nNest])
			r_give (ch, arg);
		
		return 1;
	case RP_TAKE:
		if (!ifin[nNest])
			r_take (ch, arg);
		
		return 1;
	case RP_PUT:
		if (!ifin[nNest])
			r_put (ch, arg);
		
		return 1;
	case RP_GET:
		if (!ifin[nNest])
			r_get (ch, arg);
		
		return 1;
	case RP_GETCASH:
	case RP_GIVECASH:
		
		return 1;
	case RP_STAYPUT:
		if (!ifin[nNest])
			r_stayput (ch, arg);
		
		return 1;
	case RP_ZONE_ECHO:
		if (!ifin[nNest])
			r_zone_echo (ch, arg);
		
		return 1;
	case RP_ATWRITE:
		if (!ifin[nNest])
			r_atwrite (ch, arg);
		
		return 1;
	case RP_SYSTEM:
		if (!ifin[nNest])
			r_system (ch, arg);
		
		return 1;
	case RP_CLAN_ECHO:
		if (!ifin[nNest])
			r_clan_echo (ch, arg);
		
		return 1;
	case RP_TRANS_GROUP:
		if (!ifin[nNest])
			r_trans_group (ch, arg);
		
		return 1;
	case RP_SET:
		if (!ifin[nNest])
			r_set (ch, arg, variable_list);
		
		return 1;
	case RP_LOADMOB:
		if (!ifin[nNest])
			r_loadmob (ch, arg);
		
		return 1;
	case RP_LOAD_CLONE:
		if (!ifin[nNest])
			r_load_clone (ch, arg);
		
		return 1;
	case RP_EXMOB:
		if (!ifin[nNest])
			r_exmob (ch, arg);
		
		return 1;
	case RP_FORCE:
		if (!ifin[nNest])
			r_force (ch, arg);
		
		return 1;
	case RP_IF:
		if (!ifin[nNest])
			reval (ch, arg);
		ifin[nNest + 1] = ifin[nNest];
		nNest++;
		
		return 1;
	case RP_FI:
		--nNest;
		ifin[nNest] = ifin[nNest - 1];
		
		return 1;
	case RP_ELSE:
		nNest--;
		if (!ifin[nNest - 1])
		{
			ifin[nNest] = !ifin[nNest];
		}
		ifin[nNest + 1] = ifin[nNest];
		nNest++;
		
		return 1;
	case RP_RFTOG:
		if (!ifin[nNest])
			r_rftog (ch, arg);
		
		return 1;
	case RP_PAIN:
		if (!ifin[nNest])
			r_pain (ch, arg);
		
		return 1;
	case RP_ATREAD:
		if (!ifin[nNest])
			r_atread (ch, arg);
		
		return 1;
	case RP_PURGE:
		if (!ifin[nNest])
			r_purge (ch, arg);
		
		return 1;
	case RP_CRIMINALIZE:
		if (!ifin[nNest])
			r_criminalize (ch, arg);
		
		return 1;
	case RP_STRIP:
		if (!ifin[nNest])
			r_strip (ch, arg);
		
		return 1;
	case RP_DOITANYWAY:
		if (!ifin[nNest])
			r_doitanyway(ch, arg, command, keyword, argument);
		
		return 1;
	case RP_CLAN:
		if (!ifin[nNest])
			r_clan (ch, arg);
		
		return 1;
	case RP_DELAY:
		if (!ifin[nNest])
			r_delay (ch, arg);
		
		return 1;
	case RP_HALT:
		if (!ifin[nNest])
		{
			
			return 0;
		}
		else
		{
			
			return 1;
		}
	case RP_DOOR:
		if (!ifin[nNest])
			r_door (ch, arg);
		
		return 1;
	case RP_TRANSOBJ:
		if (!ifin[nNest])
			r_transobj(ch, arg);

		return 1;
	default:
		system_log ("ERROR: unknown command in program", true);
		
		return 0;
	}
}

// mplist
// mplist Vnum [num]
// mplist zZone

void
do_mpstat (CHAR_DATA *ch, char *argument, int cmd)
{
	std::string strArgument = argument, ThisArgument, output = "";
	strArgument = one_argument(strArgument, ThisArgument);
	int lastVnum = -1, counter = 1;

	if (ThisArgument.empty())
	{
		for (std::multimap<int, room_prog>::iterator it = mob_prog_list.begin(); it != mob_prog_list.end(); it++)
		{
			if (lastVnum == -1 || lastVnum != it->first)
			{
				output += "\nMobile #2" + MAKE_STRING(it->first) + "#0:\n";
				counter = 1;
			}
			lastVnum = it->first;
			output += "[#B" + MAKE_STRING(counter) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
		}
		page_string (ch->desc, output.c_str());
		return;
	}

	if (ThisArgument[0] == '*')
	{
		ThisArgument.erase(0, 1);
		if (!is_number(ThisArgument.c_str()))
		{
			send_to_char ("Correct usage: #6mplist#0, #6mplist <vnum>#0, or #6mplist *<zone>#0.\n", ch);
			return;
		}
		int zone = atoi(ThisArgument.c_str());
		output += "Zone \"#F" + MAKE_STRING(zone_table[zone].name) + "#0\" [Zone " + ThisArgument + "]\n";
		for (std::multimap<int, room_prog>::iterator it = mob_prog_list.begin(); it != mob_prog_list.end(); it++)
		{
			if ((it->first/1000) != zone)
				continue;
			if (lastVnum == -1 || lastVnum != it->first)
				output += "\nMobile #2" + MAKE_STRING(it->first) + "#0:\n";
			lastVnum = it->first;
			output += "[#B" + MAKE_STRING(counter) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
		}
		page_string (ch->desc, output.c_str());
		return;
	}

	if (!is_number(ThisArgument.c_str()))
	{
		send_to_char ("Correct usage: #6mplist#0, #6mplist <vnum> [number]#0, or #6mplist *<zone>#0.\n", ch);
		return;
	}
	int vnum = atoi(ThisArgument.c_str());
	strArgument = one_argument(strArgument, ThisArgument);
	bool SingleProg = false;
	if (!ThisArgument.empty() && is_number(ThisArgument.c_str()))
	{
		SingleProg = true;
	}
	std::pair<std::multimap<int, room_prog>::iterator,std::multimap<int, room_prog>::iterator> pair = mob_prog_list.equal_range(vnum);
	output += "Mobile #2" + MAKE_STRING(vnum);
	if (SingleProg)
		output += "#0 Program Number #6" + ThisArgument + "#0:\n\n";
	else
		output += "#0:\n";
	counter = 1;
	for (std::multimap<int, room_prog>::iterator it = pair.first; it != pair.second; ++it)
	{
		if (!SingleProg)
			output += "[#B" + MAKE_STRING(counter) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
		else
		{
			if (counter == atoi(ThisArgument.c_str()))
			{
				output += it->second.prog;
			}
		}
		counter++;
	}
	page_string (ch->desc, output.c_str());
	return;
}

void
do_mpadd (CHAR_DATA *ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum;

	strArgument = one_argument (strArgument, vnum);
	if (vnum.empty() || atoi(vnum.c_str()) < 1 || atoi(vnum.c_str()) > 99999)
	{
		send_to_char ("You must specify a valid vnum of the mob to initialize a new mprog for.\n", ch);
		return;
	}

	room_prog prog;
	prog.prog = "";
	prog.keys = "";
	prog.command = "";
	prog.type = 1;

	mob_prog_list.insert(std::pair<int, room_prog>(atoi(vnum.c_str()), prog));
	int prog_num = 0;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(atoi(vnum.c_str()));
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		prog_num++;
	}
	send_to_char("Inserted blank mob prog for mobile vnum [#2", ch);
	send_to_char(vnum.c_str(), ch);
	send_to_char("#0] at position #6", ch);
	send_to_char(((std::ostringstream&) (std::ostringstream() << std::boolalpha << prog_num)).str().c_str(), ch);
	send_to_char("#0.\n", ch);
	return;
}

void
do_mpdel (CHAR_DATA *ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPDEL <vnum> <prognum>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			delete &it->second;
			mob_prog_list.erase(it);
			std::string output;
			output = "Erasing mob prog number #6" + prognum + "#0 from mobile vnum [#2" + vnum + "#0].\n";
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_mpcmd (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPCMD <vnum> <prognum> <cmd>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	if (strArgument.empty())
	{
		send_to_char("What command would you like to install?\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			it->second.command = add_hash((char *) strArgument.c_str());
			std::string output;
			output = "Installed command(s) [#6" + strArgument + "#0] for mob prog number #6" + prognum + "#0 for mobile vnum [#2" + vnum + "#0].\n";
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_mpkey (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPKEY <vnum> <prognum> <key>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	if (strArgument.empty())
	{
		send_to_char("What keyword would you like to install?\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			it->second.keys = add_hash((char *) strArgument.c_str());
			std::string output;
			output = "Installed keyword(s) [#6" + strArgument + "#0] for mob prog number #6" + prognum + "#0 for mobile vnum [#2" + vnum + "#0].\n";
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_mptype (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPTYPE <vnum> <prognum> <key>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	if (strArgument.empty())
	{
		send_to_char("What type are you setting this program to (0 or 1)?\n", ch);
		return;
	}

	if (strArgument[0] != '1' && strArgument[0] != '0')
	{
		send_to_char("Type is either 0 (like a room program) or 1 (only for the mob itself).\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			if (strArgument[0] == '1')
			{
				it->second.type = 1;
				send_to_char("Set type to 1.\n", ch);
			}
			else
			{
				it->second.type = 0;
				send_to_char("Set type to 0.\n", ch);
			}
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}


void
do_mpprg (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPPRG <vnum> <prognum>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			make_quiet (ch);
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r", ch);
			ch->desc->str = &it->second.prog;
			it->second.prog = 0;
			ch->desc->max_str = MAX_STRING_LENGTH;
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_mpapp (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is MPAPP <vnum> <prognum>.\n", ch);
		return;
	}

	int ivnum = atoi(vnum.c_str()), iprognum = atoi(prognum.c_str());
	if (ivnum < 1 || ivnum > 99999)
	{
		send_to_char("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if (iprognum < 1)
	{
		send_to_char("Prognum must be a positive number.\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			make_quiet (ch);
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r", ch);
			ch->desc->str = &it->second.prog;
			ch->desc->max_str = MAX_STRING_LENGTH;
			return;
		}
		count++;
	}

	std::string output;
	output = "Mobile number [#2" + vnum + "#0] does not have a mob program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_rpadd (CHAR_DATA * ch, char *argument, int cmd)
{
	struct room_prog *t, *old, *tmp;

	CREATE (t, struct room_prog, 1);

	t->next = NULL;
	t->command = NULL;
	t->keys = NULL;
	t->prog = NULL;

	int count = 0;
	if (!vtor (ch->in_room)->prg)
	{
		vtor (ch->in_room)->prg = t;
		count = 1;
	}
	else
	{
		old = vtor (ch->in_room)->prg;
		tmp = old;
		count++;
		while (tmp)
		{
			old = tmp;
			tmp = tmp->next;
			count++;
		}
		old->next = t;
	}
	sprintf(argument, "New Program [#2%d#0] initialized.\n\r", count);
	send_to_char(argument, ch);
}

void
do_rpdel (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 1, j;
	struct room_prog *p = 0, *tmp;

	j = atoi (argument);

	for (tmp = vtor (ch->in_room)->prg; tmp; tmp = tmp->next)
	{
		if (i == j)
		{
			if (!p)
				vtor (ch->in_room)->prg = tmp->next;
			else
				p->next = tmp->next;
			mem_free (tmp);
			send_to_char ("Done.\n\r", ch);
			return;
		}
		p = tmp;
		i++;
	}
	send_to_char
		("No such program, can you count past ten with your shoes on?\n\r", ch);
	return;
}

void
do_rpcmd (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[80], arg2[80];
	int i, j;
	struct room_prog *t;

	half_chop (argument, arg1, arg2);

	if (!isdigit (arg1[0]))
	{
		send_to_char ("Specify a program number to edit.\r\n", ch);
		return;
	}
	if (!arg2[0])
	{
		send_to_char ("Specify command(s) to install.\n\r", ch);
		return;
	}
	i = atoi (arg1);
	for (j = 1, t = vtor (ch->in_room)->prg; t; j++, t = t->next)
	{
		if (i == j)
		{
			t->command = add_hash (arg2);
			send_to_char ("Command installed.\n\r", ch);
			return;
		}
	}
	send_to_char ("That program does not exist.\n\r", ch);
	return;
}

void
do_rpkey (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[80], arg2[80];
	int i, j;
	struct room_prog *t;

	half_chop (argument, arg1, arg2);

	if (!isdigit (arg1[0]))
	{
		send_to_char ("Specify a program number to edit.\r\n", ch);
		return;
	}
	i = atoi (arg1);
	for (j = 1, t = vtor (ch->in_room)->prg; t; j++, t = t->next)
	{
		if (i == j)
		{
			t->keys = add_hash (arg2);
			send_to_char ("Keywords installed.\n\r", ch);
			return;
		}
	}
	send_to_char ("That program does not exist.\n\r", ch);
	return;
}

void
do_rpprg (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[80];
	int i, j;
	struct room_prog *t;

	one_argument (argument, arg1);

	if (!isdigit (arg1[0]))
	{
		send_to_char ("Specify a program number to edit.\r\n", ch);
		return;
	}
	i = atoi (arg1);
	for (j = 1, t = vtor (ch->in_room)->prg; t; j++, t = t->next)
	{
		if (i == j)
		{
			make_quiet (ch);
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",
				ch);
			ch->desc->str = &t->prog;
			t->prog = 0;
			ch->desc->max_str = MAX_STRING_LENGTH;
			return;
		}
	}
	send_to_char ("That program does not exist.\n\r", ch);
	return;
}

void
do_rpapp (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[80];
	int i, j;
	struct room_prog *t;

	one_argument (argument, arg1);

	if (!isdigit (arg1[0]))
	{
		send_to_char ("Specify a program number to edit.\r\n", ch);
		return;
	}
	i = atoi (arg1);
	for (j = 1, t = vtor (ch->in_room)->prg; t; j++, t = t->next)
	{
		if (i == j)
		{
			make_quiet (ch);
			send_to_char
				("Append to program now, Terminate entry with an '@'\n\r", ch);
			ch->desc->str = &t->prog;
			ch->desc->max_str = MAX_STRING_LENGTH;
			return;
		}
	}
	send_to_char ("That program does not exist.\n\r", ch);
	return;
}

void
do_rpstat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	int i = 1;
	struct room_prog *r;

	*buf = '\0';

	if (!vtor (ch->in_room)->prg)
	{
		send_to_char ("No program for this room.\n\r", ch);
		return;
	}
	if (atoi(argument) > 0)
	{
		for (r = vtor (ch->in_room)->prg; r; r = r->next, i++)
		{
			if (i == atoi(argument))
			{
				sprintf (buf + strlen (buf), "Program Number[%d]\n\r", i);
				sprintf (buf + strlen (buf), "Command words[%s]\n\r", r->command);
				sprintf (buf + strlen (buf), "Argument Keywords[%s]\n\r", r->keys);
				sprintf (buf + strlen (buf), "Program -\n\r%s\n\r", r->prog);
				break;
			}
		}
	}
	else if (!strncmp(argument, "list", 4))
	{
		for (r = vtor (ch->in_room)->prg; r; r = r->next, i++)
		{
			sprintf (buf + strlen(buf), "[#B%d#0]: Commands [#6%s#0] Keys [#6%s#0]\n", i, r->command, r->keys);
		}
	}
	else
	{
		for (r = vtor (ch->in_room)->prg; r; r = r->next, i++)
		{
			sprintf (buf + strlen (buf), "Program Number[%d]\n\r", i);
			sprintf (buf + strlen (buf), "Command words[%s]\n\r", r->command);
			sprintf (buf + strlen (buf), "Argument Keywords[%s]\n\r", r->keys);
			sprintf (buf + strlen (buf), "Program -\n\r%s\n\r", r->prog);
		}
	}
	page_string (ch->desc, buf);
}

void
r_link (CHAR_DATA * ch, char *argument)
{
	char buf1[80], buf2[80], buf3[80];
	int dir;
	int location, location2;
	ROOM_DATA *source_room;
	ROOM_DATA *target_room;

	arg_splitter (3, argument, buf1, buf2, buf3);

	if (!*buf1 || !*buf2 || !*buf3)
	{
		system_log ("ERROR: Missing args in r_link", true);
		return;
	}

	switch (*buf2)
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
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		system_log ("ERROR: Invalid direction in r_link", true);
		return;
	}

	if (!strcmp(buf1, "-1"))
		location = ch->room->nVirtual;
	else
		location = atol (buf1);

	if (!strcmp(buf3, "-1"))
		location2 = ch->room->nVirtual;
	else
		location2 = atol (buf3);

	if (!(target_room = vtor (location2)))
	{
		system_log ("ERROR: tar room not found in r_link", true);
		return;
	}

	if (!(source_room = vtor (location)))
	{
		system_log ("ERROR: cha room not found in r_link", true);
		return;
	}

	if (source_room->dir_option[dir])
		vtor (source_room->dir_option[dir]->to_room)->dir_option[rev_dir[dir]] =
		0;

	CREATE (source_room->dir_option[dir], struct room_direction_data, 1);
	source_room->dir_option[dir]->general_description = 0;
	source_room->dir_option[dir]->keyword = 0;
	source_room->dir_option[dir]->exit_info = 0;
	source_room->dir_option[dir]->key = -1;
	source_room->dir_option[dir]->to_room = target_room->nVirtual;

	CREATE (target_room->dir_option[rev_dir[dir]], struct room_direction_data,
		1);
	target_room->dir_option[rev_dir[dir]]->general_description = 0;
	target_room->dir_option[rev_dir[dir]]->keyword = 0;
	target_room->dir_option[rev_dir[dir]]->exit_info = 0;
	target_room->dir_option[rev_dir[dir]]->key = -1;
	target_room->dir_option[rev_dir[dir]]->to_room = source_room->nVirtual;
}

void
r_exit (CHAR_DATA * ch, char *argument)
{
	char buf1[256], buf2[256], buf3[80];
	int dir;
	int location, location2;
	ROOM_DATA *source_room;
	ROOM_DATA *target_room;

	arg_splitter (3, argument, buf1, buf2, buf3);

	if (!*buf1 || !*buf2 || !*buf3)
	{
		system_log ("ERROR: Missing args in r_link", true);
		return;
	}

	switch (*buf2)
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
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		system_log ("ERROR: Invalid direction in r_link", true);
		return;
	}

	if (!strcmp(buf1, "-1"))
		location = ch->room->nVirtual;
	else
		location = atol (buf1);

	if (!strcmp(buf3, "-1"))
		location2 = ch->room->nVirtual;
	else
		location2 = atol (buf3);

	if (!(target_room = vtor (location2)))
	{
		system_log ("ERROR: tar room not found in r_link", true);
		return;
	}

	if (!(source_room = vtor (location)))
	{
		system_log ("ERROR: cha room not found in r_link", true);
		return;
	}

	if (!source_room->dir_option[dir])
		CREATE (source_room->dir_option[dir], struct room_direction_data, 1);

	source_room->dir_option[dir]->general_description = 0;
	source_room->dir_option[dir]->keyword = 0;
	source_room->dir_option[dir]->exit_info = 0;
	source_room->dir_option[dir]->key = -1;
	source_room->dir_option[dir]->to_room = target_room->nVirtual;

}

void
r_atlook (CHAR_DATA * ch, char *argument)
{
	char loc_str[MAX_INPUT_LENGTH];
	int loc_nr, original_loc;
	CHAR_DATA *target_mob;
	ROOM_DATA *troom;

	strcpy (loc_str, argument);

	if (!strcmp(loc_str, "-1"))
		loc_nr = ch->room->nVirtual;
	else
		loc_nr = atoi (loc_str);

	if (!(troom = vtor (loc_nr)))
	{
		system_log ("ERROR: Room not found in r_atlook", true);
		return;
	}

	original_loc = ch->in_room;
	char_from_room (ch);
	char_to_room (ch, loc_nr);
	do_look (ch, "", 0);
	/* check if the guy's still there */
	for (target_mob = troom->people; target_mob;
		target_mob = target_mob->next_in_room)
		if (ch == target_mob)
		{
			char_from_room (ch);
			char_to_room (ch, original_loc);
		}
}

void
r_set (CHAR_DATA * ch, char *argument, room_prog_var *& variable_list)
{
	char var[AVG_STRING_LENGTH];

	argument = one_argument (argument, var);
	if (!strncmp (var, "random", 6))
	{
		int which_random = 0;
		if (isdigit(var[6]))
		{
			char conversion[8];
			sprintf(conversion, "%c", var[6]);
			which_random = atoi(conversion);
		}
		char * p;
		int rolls = strtol (argument, &p, 0);
		int die = strtol ((*p)?(p+1):(p), 0, 0);

		if ((rolls > 0 && rolls <= 100)
			&& (die > 0 && die <= 1000)
			&& (*p == ' ' || *p == 'd'))
		{
			random_number[which_random] = dice (rolls, die);
		}      
	}
	else if (!strncmp(var, "count", 5))
	{
		bool object = false;
		int which_count = 0, vnum = 0, room_vnum = 0;
		if (isdigit(var[5]))
		{
			char conversion [8];
			sprintf (conversion, "%c", var[5]);
			which_count = atoi(conversion);
		}

		argument = one_argument(argument, var);
		if (!strcmp(var, "obj"))
			object = true;
		else if (strcmp(var, "mob"))
			return;

		argument = one_argument(argument, var);
		vnum = atoi(var);
		if (!vnum)
			return;

		argument = one_argument(argument, var);
		if (!strcmp(var, "-1"))
			room_vnum = ch->room->nVirtual;
		else
			room_vnum = atoi(var);
		if (!room_vnum)
			return;

		int count = 0;
		if (object)
		{
			for (OBJ_DATA *obj = vtor(room_vnum)->contents; obj; obj = obj->next_content)
			{
				if (obj->nVirtual == vnum)
				{
					if (obj->count > 1)
						count += obj->count;
					else
						count++;
				}
			}
		}
		else
		{
			for (CHAR_DATA *tch = vtor(room_vnum)->people; tch; tch = tch->next_in_room)
			{
				if (IS_NPC(tch) && tch->mob->nVirtual == vnum)
					count++;
				else if (vnum == -1 && !IS_NPC(tch))
					count++;
			}
		}
		count_number[which_count] = count;
	}
	else if (!strncmp(var, "var", 3))
	{
		std::string str_argument, str_name;
		argument = one_argument(argument, var);
		str_argument = argument;
		str_name = var;

		if (str_name.empty() || str_argument.empty())
			return;

		if (is_variable_in_list(variable_list, str_name))
			set_variable_data(variable_list, str_name, str_argument);
		else
		{
			add_variable_to_list(variable_list, str_name, str_argument);
		}
	}
}


// void
// r_atecho (CHAR_DATA * ch, char *argument)
// {
//  char loc_str[MAX_INPUT_LENGTH], buf[4096];
//
//  half_chop (argument, loc_str, buf);
//
//  if (!isdigit (*loc_str))
//    {
//      system_log ("ERROR: atecho location not a digit", true);
//      return;
//    }
//
//  if (!vtor (atoi (loc_str)))
//    {
//      system_log ("ERROR: Room not found in r_atecho", true);
//      return;
//    }
//
//  strcat (buf, "\n\r");
//  send_to_room (buf, vtor (atoi (loc_str))->nVirtual);
// }


void 
r_atecho(CHAR_DATA *ch, char *argument) 
{ 
	char   loc_str[MAX_INPUT_LENGTH] = {'\0'}; 
	char   loc_str1[MAX_INPUT_LENGTH] = {'\0'}; 
	char    *ploc_str; 
	char   *ploc_str1; 
	char   buf[MAX_INPUT_LENGTH] = {'\0'}; 
	char   test_dat[2] = "-"; 
	char   mt1[MAX_INPUT_LENGTH] = "              "; 
	int    room_span = 0; 
	int    first_room = 0; 
	int   last_room = 0; 

	half_chop(argument, loc_str, buf); 
	ploc_str = loc_str; 
	ploc_str1 = loc_str1; 
	strcat(buf,"\n\r"); 

	// buf is ready to go.  it's the echo that gets sent out to the rooms. 

	while(1) 
	{ 
		if (!strncmp(ploc_str, test_dat, 1)) // if it's a '-' set the room_span flag 
			room_span = true; 
		for (; ispunct(*ploc_str); ploc_str++); // bypass any punctuation 
		for (; isdigit(*ploc_str1 = *ploc_str); ploc_str++, ploc_str1++); // read room # into loc_str1 

		if ( !isdigit(*loc_str1) ) { 
			return; 
		} 

		if ( !vtor (strtol(loc_str1, NULL, 10)) ) { 
			system_log("ERROR: Room not found in r_atecho", true); 
			//   return; 
		} 
		else { 
			strcat(buf,"\n\r"); 
			send_to_room (buf, vtor (strtol(loc_str1, NULL, 10))->nVirtual); 
		} 


		if(room_span) { // if room_span is set, the last room echoed to was the end of the span 
			// go echo to the rooms in between now. 
			last_room = strtol(loc_str1, NULL, 10); // set the last room as an integer 

			// iterate through the span of rooms 
			while(first_room + 1 < last_room) { 
				if ( !vtor (first_room + 1) ) { // does the room exist? 
					first_room++; // increment even if the room doesn't exist 
				} 
				else { 
					send_to_room(buf, vtor (first_room + 1)->nVirtual); 
					first_room++; 
				} 
			} 
			room_span = 0;    // reset the trigger 
		} 

		first_room = strtol(loc_str1, NULL, 10); // set first_room as the last room echoed to 
		strcpy(loc_str1, mt1); // overwrite loc_str1 
		ploc_str1 = loc_str1; // reset the pointer 
	} 
	return; 
} 


void
r_unlink (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int dir;
	int old_rnum, location;
	ROOM_DATA *troom;

	half_chop (argument, arg1, arg2);

	if (!strcmp(arg2, "-1"))
		location = ch->room->nVirtual;
	else
		location = atoi (arg2);

	switch (*arg1)
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
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		system_log ("ERROR: Invalid direction in r_unlink", true);
		return;
	}

	if (!(troom = vtor (location)))
	{
		system_log ("ERROR: cha room not found in r_unlink", true);
		return;
	}

	if (troom->dir_option[dir])
	{
		old_rnum = troom->dir_option[dir]->to_room;
	}
	else
	{
		sprintf (buf, "ERROR: Unknown exit in r_unlink [%d]: %s",
			troom->nVirtual, argument);
		system_log (buf, true);
		return;
	}

	troom->dir_option[dir] = 0;
	vtor (old_rnum)->dir_option[rev_dir[dir]] = 0;
}

void
r_unexit (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80];
	int dir;
	int location;
	ROOM_DATA *troom;

	half_chop (argument, arg1, arg2);

	if (!strcmp(arg2, "-1"))
		location = ch->room->nVirtual;
	else
		location = atoi (arg2);

	switch (*arg1)
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
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		system_log ("ERROR: Invalid direction in r_unexit", true);
		return;
	}

	if (!(troom = vtor (location)))
	{
		system_log ("ERROR: cha room not found in r_unexit", true);
		return;
	}

	troom->dir_option[dir] = 0;
}

void
r_give (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	char buf [AVG_STRING_LENGTH];


	argument = one_argument(argument, buf);
	obj = load_object (atoi (buf));

	if (is_number(argument))
		obj->count = atoi(argument);
	if (obj)
		obj_to_char (obj, ch);
	else
		system_log ("ERROR: Object does not exist in r_give", true);
}

void
r_take (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	char buf [AVG_STRING_LENGTH];

	argument = one_argument(argument, buf);

	obj = get_obj_in_list_num (atol (buf), ch->right_hand);
	if (!obj)
		obj = get_obj_in_list_num (atol (buf), ch->left_hand);

	argument = one_argument(argument, buf);
	if (obj)
	{
		if (atoi(buf) > 0)
			obj_from_char (&obj, atoi(buf));
		else
		{
			obj_from_char (&obj, 0);
			extract_obj (obj);
		}
	}
	else
		system_log ("ERROR: Object not found in r_take", true);
}

void
r_put (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80];

	half_chop (argument, arg1, arg2);

	if (!vtor (atoi (arg2)) || !vtoo (atoi (arg1)))
		system_log ("ERROR: Object does not exist in r_put", true);
	else
		obj_to_room (load_object (atoi (arg1)), vtor (atoi (arg2))->nVirtual);
}

// load_clone(<holder>,<obj_keyword>,<recipient>)
// holder: -1 (prog-user) or vnum of mob
// obj_keyword: matches the keys of an object held by holder
// recipient: -1 (room-floor) or vnum of mob
void
r_load_clone (CHAR_DATA * ch, char *argument)
{
	size_t len = strlen (argument);
	char *arg1 = new char [len];
	char *arg2 = new char [len];
	char *arg3 = new char [len];
	OBJ_DATA *obj = 0;
	OBJ_DATA *clone = 0;

	arg_splitter (3, argument, arg1, arg2, arg3);
	int holder_vnum = strtol (arg1, 0, 10);
	int recipient_vnum = strtol (arg3, 0, 10);

	CHAR_DATA* holder = (holder_vnum <= 0) ? ch : 0 ;
	CHAR_DATA* recipient = 0;

	CHAR_DATA* i = ch->room->people;
	for (; i; i = i->next_in_room)
	{      
		if (i->deleted)
			continue;

		if (!IS_NPC (i))
			continue;

		if (i->mob->nVirtual == holder_vnum)
		{
			holder = i;
		}
		if (i->mob->nVirtual == recipient_vnum)
		{
			recipient = i;
		}

		if (holder && recipient_vnum == -1)
			break;
	}


	if ((obj = get_obj_in_dark (holder, arg2, holder->right_hand))
		||(obj = get_obj_in_dark (holder, arg2, holder->left_hand)))
	{
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE))
			clone = load_colored_object (obj->nVirtual, obj->var_color);
		else
			clone = load_object (obj->nVirtual);

		clone->o.od.value[0] = obj->o.od.value[0];
		clone->o.od.value[1] = obj->o.od.value[1];
		clone->o.od.value[2] = obj->o.od.value[2];
		clone->o.od.value[3] = obj->o.od.value[3];
		clone->o.od.value[4] = obj->o.od.value[4];
		clone->o.od.value[5] = obj->o.od.value[5];

		clone->size = obj->size;

		if (recipient_vnum == -1)
			obj_to_room (clone, holder->room->nVirtual);
		else if (recipient)
			obj_to_char (clone, recipient);
		else 
			extract_obj (clone);
	}

	delete [] arg1;
	delete [] arg2;
	delete [] arg3;
}

void
r_get (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80];
	OBJ_DATA *obj;
	int virt;

	arg_splitter (3,argument, arg1, arg2, arg3);

	if (!strcmp(arg2, "-1"))
		virt = ch->room->nVirtual;
	else
		virt = atoi (arg2);

	if (!vtor (virt))
	{
		system_log ("ERROR: Object-room not found in r_get", true);
		return;
	}

	obj = get_obj_in_list_num (atoi (arg1), vtor (virt)->contents);
	if (obj)
	{
		if (is_number(arg3))
			obj_from_room (&obj, atoi(arg3));
		else
		{
			obj_from_room (&obj, 0);
			extract_obj (obj);
		}
	}
	else
		system_log ("ERROR: Object not found in r_get", true);
}

void
r_lock (CHAR_DATA * ch, char *argument)
{
	long virt;
	int dir;
	char arg1[80], arg2[80];

	half_chop (argument, arg1, arg2);

	virt = atol (arg2);

	switch (*arg1)
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
		dir = -1;
		break;
	}

	if (dir == -1)
	{
		system_log ("ERROR: Invalid direction in r_unexit", true);
		return;
	}
}

void
r_loadmob (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80];
	int mob_vnum, room_vnum;

	half_chop (argument, arg1, arg2);

	if (!strcmp(arg1, "-1"))
	{
		if (IS_NPC(ch))
			mob_vnum = ch->mob->nVirtual;
		else
			return;
	}
	else
		mob_vnum = atoi(arg1);

	if (!strcmp(arg2, "-1"))
		room_vnum = ch->room->nVirtual;
	else
		room_vnum = atoi(arg2);

	if (!vtom (mob_vnum) || !vtor (room_vnum))
	{
		system_log ("ERROR: Mobile does not exist in r_loadmob", true);
		return;
	}

	char_to_room (load_mobile (mob_vnum), room_vnum);
}

void
r_exmob (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *ptrMob = NULL, *tmp_ch = NULL;
	char arg1[80], arg2[80];
	int virt;
	long nMobVnum = 0;

	half_chop (argument, arg1, arg2);

	if (!strcmp(arg2, "-1"))
		virt = ch->room->nVirtual;
	else
		virt = atol (arg2);

	if (!vtor (virt))
	{
		system_log ("ERROR: Mobile-room does not exist in r_exmob", true);
		return;
	}

	nMobVnum = strtol (arg1, NULL, 10);
	if (nMobVnum > 0 && nMobVnum < 100000)
	{
		for (tmp_ch = vtor (virt)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room)
		{
			if (tmp_ch->mob && tmp_ch->mob->nVirtual == nMobVnum)
			{
				ptrMob = tmp_ch;
			}
		}
		if (!ptrMob)
		{
			system_log ("ERROR: Mobile does not exist in r_exmob", true);
			return;
		}
	}
	else if (!(ptrMob = get_char_room (arg1, virt)))
	{
		system_log ("ERROR: Mobile does not exist in r_exmob", true);
		return;
	}

	extract_char (ptrMob);
}

void
r_rftog (CHAR_DATA * ch, char *arg)
{
	int flag;
	char buf[80], rbuf[80];
	ROOM_DATA *troom;

	*buf = *rbuf = '\0';

	half_chop (arg, buf, rbuf);

	if (!strcmp(rbuf, "-1"))
		troom = vtor (ch->room->nVirtual);
	else
		troom = vtor(atoi(rbuf));

	if (!troom)
	{
		system_log ("ERROR: Unknown room in r_rftog.", true);
		return;
	}

	flag = parse_argument (room_bits, buf);

	if (!IS_SET (troom->room_flags, (1 << flag)))
		troom->room_flags |= (1 << flag);
	else
		troom->room_flags &= ~(1 << flag);
}

void
r_force (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[256];
	CHAR_DATA *tmp_ch;
	int room, mob;
	char buf[1024];
	bool old_ifin [MAX_RPRG_NEST];
	for (int index = 0; index < MAX_RPRG_NEST; index++)
	{
		old_ifin[index] = ifin[index];
	}

	arg_splitter (3, argument, arg1, arg2, arg3);
	*s_buf = '\0';
	if (!strncmp(arg1, "all", 3))
		mob = 0;
	else
		mob = atoi (arg1);

	if (mob == -1)
	{
		command_interpreter (ch, arg3);
		for (int index = 0; index < MAX_RPRG_NEST; index++)
		{
			ifin[index] = old_ifin[index];
		}
		return;
	}

	if (!strcmp(arg2, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg2);

	if (!vtor (room))
	{
		system_log ("ERROR: unknown room in r_force.", true);
		return;
	}

	for (tmp_ch = vtor (room)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
	{
		if (tmp_ch->mob && tmp_ch->mob->nVirtual == mob)
		{
			sprintf (buf, arg3, GET_NAME (ch));
			command_interpreter (tmp_ch, buf);
		}
		else if (mob == 0)
		{
			sprintf (buf, arg3, GET_NAME(ch));
			command_interpreter (tmp_ch, buf);
		}
	}

	for (int index = 0; index < MAX_RPRG_NEST; index++)
	{
		ifin[index] = old_ifin[index];
	}



}

void
r_transmob (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[256];
	CHAR_DATA *ptrMob = NULL, *tmp_ch;
	int nOriginRoom, nTargetRoom, nMobVnum;

	arg_splitter (3, argument, arg1, arg2, arg3);

	nMobVnum = atoi (arg1);
	if (!strcmp(arg2, "-1"))
		nOriginRoom = ch->room->nVirtual;
	else
		nOriginRoom = atoi (arg2);

	if (!strcmp(arg3, "-1"))
		nTargetRoom = ch->room->nVirtual;
	else
		nTargetRoom = atoi (arg3);

	if (!vtor (nOriginRoom))
	{
		system_log ("ERROR: unknown origin room in r_transmob.", true);
		nOriginRoom = ch->room->nVirtual;
	}

	if (!vtor (nTargetRoom))
	{
		system_log ("ERROR: unknown desination room in r_transmob.", true);
		return;
	}

	if (nMobVnum == -1)
	{
		ptrMob = ch;
	}
	else
	{
		for (tmp_ch = vtor (nOriginRoom)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room)
		{
			if (tmp_ch->mob && tmp_ch->mob->nVirtual == nMobVnum)
			{
				ptrMob = tmp_ch;
			}
		}
	}

	if (ptrMob->mount)
	{
		char_from_room (ptrMob->mount);
		char_to_room (ptrMob->mount, vtor (nTargetRoom)->nVirtual);
	}
	char_from_room (ptrMob);
	char_to_room (ptrMob, vtor (nTargetRoom)->nVirtual);
	return;
}


/*
* r_trans_group
*
* trans_group <mob> <from> <to>
* 
* if <mob> is -1 assumes program user, and <from> is ignored
* 
*/
void
r_trans_group (CHAR_DATA * ch, char * argument)
{
	char subject_arg [80];
	char origin [80];
	char destination [256];
	int subject_mnum, origin_rnum, destination_rnum;

	// Parse the arguments into parts (could probably just use strtol)
	arg_splitter (3, argument, subject_arg, origin, destination);

	subject_mnum = strtol (subject_arg, 0, 10);
	if (!strcmp(origin, "-1"))
		origin_rnum = ch->room->nVirtual;
	else
		origin_rnum = strtol (origin, 0, 10);

	if (!strcmp(destination, "-1"))
		destination_rnum = ch->room->nVirtual;
	else
		destination_rnum = strtol (destination, 0, 10);

	// If origin_rnum doesn't exist, default to user's room
	if (!vtor (origin_rnum))
	{
		system_log ("ERROR: unknown origin room in r_transmob.", true);
		origin_rnum = ch->room->nVirtual;
	}

	// If the destination room doesn't exist, not need to continue
	if (!vtor (destination_rnum))
	{
		system_log ("ERROR: unknown desination room in r_transmob.", true);
		return;
	}


	CHAR_DATA *subject = 0;
	if (subject_mnum == -1)
	{
		subject = ch;
	}
	else
	{
		for (CHAR_DATA *tmp_ch = vtor (origin_rnum)->people; tmp_ch;
			tmp_ch = tmp_ch->next_in_room)
		{
			if (tmp_ch->mob && tmp_ch->mob->nVirtual == subject_mnum)
			{
				subject = tmp_ch;
			}
		}
	}
	/// loop here

	// if subdued we transfer the captors group
	if (IS_SUBDUEE (subject))
		subject = subject->subdue;

	if (IS_RIDEE (subject))
		subject = subject->mount;

	// CHAR_DATA * leader = (subject->following) ? subject->following : subject;
	CHAR_DATA * leader = subject;

	int queue = 0;
	CHAR_DATA * transfer_queue [128]; // can trans up to 128 people
	CHAR_DATA * tmp_ch = subject->room->people;
	for (; tmp_ch && queue < 124; tmp_ch = tmp_ch->next_in_room)
	{
		if (tmp_ch->following == leader || tmp_ch == leader)
		{
			if (tmp_ch->mount) 
			{
				transfer_queue[queue++] = tmp_ch->mount;
			}
			if (IS_SUBDUER(tmp_ch))
			{
				transfer_queue[queue++] = tmp_ch->subdue;
			}
			if (IS_HITCHER(tmp_ch))
			{
				transfer_queue[queue++] = tmp_ch->hitchee;
			}
			transfer_queue[queue++] = tmp_ch;
		}
	}  

	// move them
	do 
	{
		char_from_room (transfer_queue[--queue]);
		char_to_room (transfer_queue[queue], destination_rnum);
		do_look (transfer_queue[queue],"",0);
	}
	while (queue);
	return;
}


void
r_stayput (CHAR_DATA * ch, char *argument) {}

void
r_zone_echo (CHAR_DATA * ch, char *argument)
{
	char *sector_message;
	int zone = strtol (argument,&sector_message,10);

	char sector [AVG_STRING_LENGTH];
	argument = one_argument (sector_message, sector);

	if (strcmp (sector, "all") == 0)
	{
		for (ROOM_DATA *room = full_room_list; room; room = room->lnext)
			if (room->people && room->zone == zone)
				send_to_room (argument, room->nVirtual);
	}
	else if (strcmp (sector, "outside") == 0)
	{
		for (ROOM_DATA *room = full_room_list; room; room = room->lnext)
			if (room->people && room->zone == zone)
			{
				switch (room->sector_type)
				{
				case SECT_INSIDE:
				case SECT_UNDERWATER:
				case SECT_PIT:
					break;
				default:		
					send_to_room (argument, room->nVirtual);
					break;
				}
			}            
	}
	else 
	{
		int sector_flag = parse_argument (sector_types, sector);
		if (sector_flag >= 0)
		{
			for (ROOM_DATA *room = full_room_list; room; room = room->lnext)
				if (room->people && room->zone == zone 
					&& room->sector_type == sector_flag)
					send_to_room (argument, room->nVirtual);
		}
	}
}

void
r_atwrite (CHAR_DATA * ch, char *argument) {}

void
r_system (CHAR_DATA * ch, char *argument)
{
	char buf [AVG_STRING_LENGTH];
	char *token = index (argument, '@');

	if (token) 
	{
		token[0] = 0;
		sprintf (buf, "#6%s#5%s#0#6%s#0\n", argument, ch->tname, token+1);
	}
	else
	{
		sprintf (buf, "#5%s#0: #6%s#0\n", ch->tname, argument);
	}
	send_to_gods (buf);
}

void
r_clan_echo (CHAR_DATA * ch, char *argument) 
{
	CHAR_DATA *tch;
	char buf [MAX_STRING_LENGTH];
	argument = one_argument(argument, buf);
	if (is_number(buf))
	{
		if (!vtor(atoi(buf)))
			return;
		char * formatted;
		ROOM_DATA *room = vtor(atoi(buf));
		argument = one_argument(argument, buf);
		reformat_string(argument, &formatted);
		for (tch = room->people; tch; tch = tch->next_in_room)
		{
			if (is_clan_member(tch, buf))
			{
				send_to_char(formatted, tch);
				send_to_char("\n", tch);
			}
		}
		mem_free(formatted);
	}
	else if (!strncmp(buf, "room", 4))
	{
		char * formatted;
		argument = one_argument(argument, buf);
		reformat_string(argument, &formatted);
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (is_clan_member(tch, buf))
			{
				send_to_char(formatted, tch);
				send_to_char("\n", tch);
			}
		}
		mem_free(formatted);
	}
	else if (!strncmp(buf, "zone", 4))
	{
		DESCRIPTOR_DATA *d;
		bool outside = false, inside = false;
		char * formatted;
		if (!strncmp(buf, "zone_inside", 11))
			inside = true;
		if (!strncmp(buf, "zone_outside", 12))
			outside = true;
		argument = one_argument(argument, buf);
		reformat_string(argument, &formatted);
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->connected || !d->character || !d->character->room)
				continue;
			if (d->character->room->zone != ch->room->zone)
				continue;
			if (IS_SET (d->character->act, PLR_QUIET) || !AWAKE (d->character) || !is_clan_member(d->character, buf))
				continue;
			if ((outside && !IS_OUTSIDE(d->character)) || (inside && IS_OUTSIDE(d->character)))
				continue;

			send_to_char (formatted, d->character);
			send_to_char("\n", d->character);
		}
		mem_free(formatted);
	}
	else if (!strncmp(buf, "all", 3))
	{
		DESCRIPTOR_DATA *d;
		bool outside = false, inside = false;
		char * formatted;
		if (!strncmp(buf, "all_inside", 10))
			inside = true;
		if (!strncmp(buf, "all_outside", 11))
			outside = true;
		argument = one_argument(argument, buf);
		reformat_string(argument, &formatted);
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->connected || !d->character || !d->character->room)
				continue;
			if (IS_SET (d->character->act, PLR_QUIET) || !AWAKE (d->character) || !is_clan_member(d->character, buf))
				continue;
			if ((outside && !IS_OUTSIDE(d->character)) || (inside && IS_OUTSIDE(d->character)))
				continue;

			send_to_char(formatted, d->character);
			send_to_char("\n", d->character);
		}
		mem_free(formatted);
	}
	return;
}

void
r_pain (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80], arg4[80], arg5[80];
	int high, low, dam, type;
	int room;
	CHAR_DATA *victim;

	arg_splitter (5, argument, arg1, arg2, arg3, arg4, arg5);

	if (!strcmp(arg1, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg1);

	if (!vtor (room))
	{
		system_log ("ERROR: unknown room in r_pain.", true);
		return;
	}
	low = atoi (arg2);
	high = atoi (arg3);
	if ((type = index_lookup (damage_type, arg5)) < 0)
	{
		type = 3;
	}

	if (!strncmp (arg4, "all", 3))
	{

		for (victim = vtor (room)->people; victim;
			victim = victim->next_in_room)
		{

			if ((dam = number (low, high)))
			{
				wound_to_char (victim, figure_location (victim, number (0, 10)),
					dam, type, 0, 0, 0);
			}

		}
	}
	else
	{

		if ((dam = number (low, high)))
		{
			wound_to_char (ch, figure_location (ch, number (0, 10)), dam, type,
				0, 0, 0);
		}

	}
}


/* atread <room> <board> <message> */
void
r_atread (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80], buf[80];
	int room, nMsgNum;
	OBJ_DATA *ptrBoard = NULL;
	ROOM_DATA *ptrRoom = NULL;
	//      CHAR_DATA *victim = NULL;

	arg_splitter (3, argument, arg1, arg2, arg3);

	if (!strcmp(arg1, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg1);

	if (!(ptrRoom = vtor (room)))
	{
		system_log ("ERROR: unknown room in r_atread.", true);
		return;
	}
	else
	{
		if (!(ptrBoard = get_obj_in_list (arg2, ptrRoom->contents))
			|| (GET_ITEM_TYPE (ptrBoard) != ITEM_BOARD))
		{
			send_to_char ("What board?\n", ch);
			return;
		}

		if (!isdigit (*arg3) && *arg3 != '-')
		{
			send_to_char ("Which message on that board?\n", ch);
			return;
		}
		nMsgNum = atoi (arg3);
		one_argument (ptrBoard->name, buf);
		display_mysql_board_message (ch, buf, nMsgNum, 1);
	}
	return;
}

void
r_purge (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80];
	int room;
	OBJ_DATA *object = NULL;
	OBJ_DATA *next_object = NULL;
	ROOM_DATA *ptrRoom = NULL;
	//      CHAR_DATA *victim = NULL;

	arg_splitter (3, argument, arg1, arg2, arg3);

	if (!strcmp(arg1, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg1);

	if (!(ptrRoom = vtor (room)))
	{
		system_log ("ERROR: unknown room in r_purge.", true);
		return;
	}
	else
	{
		for (object = ptrRoom->contents; object; object = next_object)
		{
			next_object = object->next_content;
			if (GET_ITEM_TYPE (object) == ITEM_DWELLING
				&& object->o.od.value[0] >= 100000)
			{
				continue;
			}
			extract_obj (object);
		}
	}

	return;
}

// strip <room>
// room = vnum of a room -- where the bag will be dropped off. -1 is character's room.

void
r_strip (CHAR_DATA *ch, char *argument)
{
	int drop_room = atoi(argument);
	OBJ_DATA *obj;
	OBJ_DATA *bag = NULL;
	char buf[MAX_STRING_LENGTH];

	if (drop_room == -1)
		drop_room = ch->room->nVirtual;

	if (!( vtor(drop_room)))
	{
		system_log("ERROR: Room does not exist in r_strip", true);
	}
	else
	{
		bag = load_object (VNUM_JAILBAG);

		if (bag && (ch->right_hand || ch->left_hand || ch->equip))
		{
			sprintf (buf, "A bag belonging to %s sits here.", ch->short_descr);
			bag->description = str_dup (buf);

			sprintf (buf, "a bag labeled '%s'", ch->short_descr);
			bag->short_description = str_dup (buf);

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

			obj_to_room (bag, drop_room);
		}
	}
}

// clan <clan short name> <rank>

void
r_clan (CHAR_DATA *ch, char *argument)
{
	size_t len = strlen (argument);
	char *arg1 = new char [len];
	char *arg2 = new char [len];
	char *arg3 = new char [len];
	arg_splitter (3, argument, arg1, arg2, arg3);

	int flags;

	if((!strncmp (arg3, "remove", 6)) || (!strncmp (arg2, "remove", 6)))
	{
		remove_clan(ch, arg1);
	}
	else
	{

		if (!strncmp (arg2, "leader", 6))
			flags = CLAN_LEADER;
		else if (!strncmp (arg2, "memberobj", 9))
			flags = CLAN_MEMBER_OBJ;
		else if (!strncmp (arg2, "leaderobj", 9))
			flags = CLAN_LEADER_OBJ;
		else if (!strncmp (arg2, "recruit", 7))
			flags = CLAN_RECRUIT;
		else if (!strncmp (arg2, "private", 7))
			flags = CLAN_PRIVATE;
		else if (!strncmp (arg2, "corporal", 8))
			flags = CLAN_CORPORAL;
		else if (!strncmp (arg2, "sergeant", 8))
			flags = CLAN_SERGEANT;
		else if (!strncmp (arg2, "lieutenant", 10))
			flags = CLAN_LIEUTENANT;
		else if (!strncmp (arg2, "captain", 7))
			flags = CLAN_CAPTAIN;
		else if (!strncmp (arg2, "general", 7))
			flags = CLAN_GENERAL;
		else if (!strncmp (arg2, "commander", 9))
			flags = CLAN_COMMANDER;
		else if (!strncmp (arg2, "apprentice", 10))
			flags = CLAN_APPRENTICE;
		else if (!strncmp (arg2, "journeyman", 10))
			flags = CLAN_JOURNEYMAN;
		else if (!strncmp (arg2, "master", 6))
			flags = CLAN_MASTER;
		else 
			flags = CLAN_MEMBER;

		add_clan(ch, arg1, flags);
	}

	delete [] arg1;
	delete [] arg2;
	delete [] arg3;
}


// criminalize <target> <zone> <hours>
// target = -1, trigger initiator, or 'all' for all initiator's room.
// zone = game zone
// hours = positive integer

void
r_criminalize (CHAR_DATA *ch, char *argument)
{
	size_t len = strlen (argument);
	char *arg1 = new char [len];
	char *arg2 = new char [len];
	char *arg3 = new char [len];
	arg_splitter (3, argument, arg1, arg2, arg3);
	int zone = atoi(arg2);
	int time = atoi(arg3);

	CHAR_DATA* i = ch->room->people;

	if (time <= 0)
		time = 0;

	if (zone == -1)
		zone = ch->room->zone;
	else if ((zone <= 0) || (zone > 100))
		zone = 0;

	if (!strncmp (arg1, "all", 3))
	{
		for (; i; i = i->next_in_room)
		{
			add_criminal_time (i, zone, time);
		}
	}
	else
	{
		add_criminal_time (ch, zone, time);
	}

	delete [] arg1;
	delete [] arg2;
	delete [] arg3;
}


// Loadobj <room> <number> <object>
// room = -1 (trigger-puller's room) or vnum
// number = positive integer
// object = vnum

void
r_load_obj (CHAR_DATA *ch, char *argument)
{
	size_t len = strlen(argument);
	char *arg1 = new char [(int) len];
	char *arg2 = new char [(int) len];
	char *arg3 = new char [(int) len];
	bool exit = false;
	OBJ_DATA *obj = NULL;
	int rvnum = 0, ovnum = 0, count = 0;
	arg_splitter (3, argument, arg1, arg2, arg3);
	rvnum = atoi(arg1);
	count = atoi(arg2);
	ovnum = atoi(arg3);

	if ( rvnum == -1)
	{
		rvnum = ch->room->nVirtual;
	}

	if (!( vtor(rvnum) ))
	{
		system_log("ERROR: Room does not exist in r_load_obj", true);
		exit = true;
	}
	if (count < 1)
	{
		system_log("ERROR: Negative count specified in r_load_obj", true);
		exit = true;
	}
	if (count == 1)
	{
		sprintf(arg2, "%d %d", ovnum, rvnum);
		r_put(ch, arg2);
		exit = true;
	}
	if (!exit && !(obj = load_object (ovnum)))
	{
		system_log("ERROR: Item does not exist in r_load_obj", true);
		exit = true;
	}

	if ( !exit)
	{
		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			obj_to_room (obj, rvnum);
			for (int i = 1; i < count; i++)
			{
				obj = load_object(ovnum);
				obj_to_room (obj, rvnum);
			}
		}
		else
		{
			obj->count = count;
			obj_to_room (obj, rvnum);
		}
	}


	delete [] arg1;
	delete [] arg2;
	delete [] arg3;

}

void r_delay (CHAR_DATA *ch, char *argument)
{
	do_scommand(ch, argument, 1);
}

void r_takemoney ( CHAR_DATA *ch, char * argument)
{
	std::string ArgumentList(argument);
	std::string ThisArgument("");
	int TargetVnum = -1, Count = 0, Currency = 0;

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	/*	if (!ThisArgument.empty())
	{
	if (is_number(ThisArgument.c_str()) && ThisArgument.c_str() == "-1") // Room target disabled, must be -1 (player) for now.
	{
	TargetVnum = atoi(ThisArgument.c_str());
	}
	else
	{
	send_to_gods("Error: Non supported target argument in r_takemoney");
	return;
	}
	}
	else
	{
	send_to_gods("Error: Missing target argument in r_takemoney");
	return;
	} */ // Currently unneccisary as the taking money from room functionality is not in place

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (!ThisArgument.empty())
	{
		if (is_number(ThisArgument.c_str()) && atoi(ThisArgument.c_str()) > 0)
		{
			Count = atoi(ThisArgument.c_str());
		}
		else
		{
			send_to_gods("Error: Invalid amount argument in r_takemoney");
			return;
		}
	}
	else
	{
		send_to_gods("Error: Missing amount argument in r_takemoney");
		return;
	}

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (!ThisArgument.empty())
	{
		if (ThisArgument.find("gondorian") != std::string::npos)
			Currency = 0;
		else if (ThisArgument.find("numenorean") != std::string::npos)
			Currency = 2;
		else if (ThisArgument.find("orkish") != std::string::npos || ThisArgument.find("yrkish") != std::string::npos || ThisArgument.find("orcish") != std::string::npos)
			Currency = 1;
		else
		{
			send_to_gods("Error: Invalid currency type in r_takemoney");
			return;
		}
	}
	else
	{
		send_to_gods("Error: Missing currency argument in r_takemoney");
		return;
	}

	if (TargetVnum == -1)
	{
		subtract_money(ch, (-1*Count), Currency);
		return;
	}
}

void r_door (CHAR_DATA *ch, char *argument)
{
	std::string ThisArgument, ArgumentList;
	int room, direction;
	ArgumentList.assign(argument);

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (ThisArgument.find("-1") != std::string::npos)
		room = ch->room->nVirtual;
	else
		room = atoi(ThisArgument.c_str());

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	switch (ThisArgument[0])
	{
	case 'n':
		direction = 0;
		break;
	case 'e':
		direction = 1;
		break;
	case 's':
		direction = 2;
		break;
	case 'w':
		direction = 3;
		break;
	case 'u':
		direction = 4;
		break;
	case 'd':
		direction = 5;
		break;
	default:
		return;
	}

	if (!vtor(room) || !vtor(room)->dir_option[direction])
		return;

	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (ThisArgument.find("open") != std::string::npos)
	{
		vtor(room)->dir_option[direction]->exit_info &= ~EX_CLOSED;
		vtor(room)->dir_option[direction]->exit_info &= ~EX_LOCKED;
		if (vtor(vtor(room)->dir_option[direction]->to_room) && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]] && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->to_room == room)
		{
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info &= ~EX_CLOSED;
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info &= ~EX_LOCKED;
		}
	}
	else if (ThisArgument.find("close") != std::string::npos)
	{
		vtor(room)->dir_option[direction]->exit_info |= EX_CLOSED;
		vtor(room)->dir_option[direction]->exit_info &= ~EX_LOCKED;
		if (vtor(vtor(room)->dir_option[direction]->to_room) && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]] && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->to_room == room)
		{
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info |= EX_CLOSED;
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info &= ~EX_LOCKED;
		}
	}
	else if (ThisArgument.find("unlock") != std::string::npos)
	{
		vtor(room)->dir_option[direction]->exit_info |= EX_CLOSED;
		vtor(room)->dir_option[direction]->exit_info &= ~EX_LOCKED;
		if (vtor(vtor(room)->dir_option[direction]->to_room) && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]] && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->to_room == room)
		{
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info |= EX_CLOSED;
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info &= ~EX_LOCKED;
		}
	}
	else if (ThisArgument.find("lock") != std::string::npos)
	{
		vtor(room)->dir_option[direction]->exit_info |= EX_CLOSED;
		vtor(room)->dir_option[direction]->exit_info |= EX_LOCKED;
		if (vtor(vtor(room)->dir_option[direction]->to_room) && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]] && vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->to_room == room)
		{
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info |= EX_CLOSED;
			vtor(vtor(room)->dir_option[direction]->to_room)->dir_option[rev_dir[direction]]->exit_info |= EX_LOCKED;
		}
	}
	return;
}

/*
void r_setvar (char *argument, DynamicVariableList *Variables)
{
std::string ArgumentList(argument), ThisArgument;

ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.find("declare")  != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty() || Variables.GetNamedVariable(ThisArgument) != NULL)
{
system_log("Error: Already declared variable in setvar.", true);
return;
}

DynamicVariable NewVar(ThisArgument);
Variables.push_back(NewVar);
}
else if (ThisArgument.find("add")  != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}
std::string VarName(ThisArgument);
if (Variables.GetNamedVariable(VarName) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}

ArgumentList = one_argument(ArgumentList, ThisArgument);
if (is_number(ThisArgument.c_str()) && is_number((Variables.GetNamedVariable(VarName))->GetVariableContents()))
{
std::ostringstream conversion << atoi(Variables.GetNamedVariable(VarName)->GetVariableContent().c_str())+atoi(ThisArgument.c_str());
Variables.GetNamedVariable(VarName)->SetVariableContents(converstion.str());
}
else
{
Variables.GetNamedVariable(VarName)->SetVariableContents(Variables.GetNamedVariable(VarName)->GetVariableContents() + ThisArgument);
}
}
else if (ThisArgument.find("assign") != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}
std::string VarName(ThisArgument);
if (Variables.GetNamedVariable(VarName) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}

ArgumentList = one_argument(ArgumentList, ThisArgument);
Variables.GetNamedVariable(VarName)->SetVariableContent(ThisArgument);
}
else if (ThisArgument.find("reset") != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}
if (Variables.GetNamedVariable(ThisArgument) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}
Variables.GetNamedVariable(ThisArgument)->SetVariableContent("");
}
else if (ThisArgument.find("divide") != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}
std::string VarName(ThisArgument);
if (Variables.GetNamedVariable(VarName) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}

ArgumentList = one_argument(ArgumentList, ThisArgument);
if (!is_number(ThisArgument.c_str()))
{
system_log("Error: Non numeric value given to divide in setvar", true);
return;
}
std::ostringstream conversion << (atoi(Variables.GetNamedVariable(VarName)->GetVariableContent())/atoi(ThisArgument.c_str()));
Variables.GetNamedVariable(VarName)->SetVariableContent(conversion.str());
}
else if (ThisArgument.find("multiply") != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}
std::string VarName(ThisArgument);
if (Variables.GetNamedVariable(VarName) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}

ArgumentList = one_argument(ArgumentList, ThisArgument);
if (!is_number(ThisArgument.c_str()))
{
system_log("Error: Non numeric value given to divide in setvar", true);
return;
}
std::ostringstream conversion << (atoi(Variables.GetNamedVariable(VarName)->GetVariableContent()) * atoi(ThisArgument.c_str()));
Variables.GetNamedVariable(VarName)->SetVariableContent(conversion.str());
}
else if (ThisArgument.find("addrandom") != std::string::npos)
{
ArgumentList = one_argument(ArgumentList, ThisArgument);
if (ThisArgument.empty())
{
system_log("Error: No argument after 'setvar arg'", true);
return;
}

if (Variables.GetNamedVariable(VarName) == NULL)
{
system_log("Error: Invalid variable in setvar", true);
return;
}
std::ostringstream conversion << random_number;
Variables.GetNamedVariable(VarName)->SetVariableContent(conversion.str());
}
else
{
system_log("Unknown argument type for Setvar.", true);
return;
}
}
*/

void r_teach (CHAR_DATA *ch, char * argument)
{
	std::string ArgumentList = argument, ThisArgument;
	int index = 0;
	ArgumentList = one_argument (ArgumentList, ThisArgument);
	if ((index = index_lookup(skills, ThisArgument.c_str())) == -1)
	{
		return;
	}
	if (real_skill(ch, index))
	{
		return;
	}
	open_skill(ch, index);
}

void
r_doitanyway (CHAR_DATA *ch, char *argument, char *command, char *keyword, char *player_args)
{
	if (!strncmp(argument, "-1", 2))
	{
		char buf [MAX_STRING_LENGTH];
		sprintf(buf, "%s %s %s", command, keyword, player_args);
		do_doitanyway(ch, buf, 1);
		return;
	}
	do_doitanyway(ch, argument, 1);
	return;
}

// transobj keyword/vnum source destination [count]
void
r_transobj (CHAR_DATA *ch, char *argument)
{
	std::string strArgument = argument, strKeyword, strSource, strDestination, strCount;
	int iCount = 0, iVnum = 0, iSource = 0, iDestination = 0;

	strArgument = one_argument(strArgument, strKeyword);

	if (strKeyword.empty())
		return;

	if (atoi(strKeyword.c_str()) != 0)
		iVnum = atoi(strKeyword.c_str());

	strArgument = one_argument(strArgument, strSource);
	if (strSource.empty() || atoi(strSource.c_str()) == 0)
		return;

	if (!strSource.compare("-1"))
		iSource = ch->in_room;
	else
		iSource = atoi(strSource.c_str());

	strArgument = one_argument(strArgument, strDestination);
	if (strDestination.empty() || atoi(strDestination.c_str()) == 0)
		return;

	if (!strDestination.compare("-1"))
		iDestination = ch->in_room;
	else
		iDestination = atoi(strDestination.c_str());

	strArgument = one_argument(strArgument, strCount);
	if (!strCount.empty() && atoi(strCount.c_str()) != 0)
		iCount = atoi(strCount.c_str());

	if (iDestination == iSource)
		return;

	if (!(vtor(iDestination)) || !(vtor(iSource)))
		return;

	OBJ_DATA *obj = NULL;
	if (iVnum)
		obj = get_obj_in_list_num(iVnum, vtor(iSource)->contents);
	else
		obj = get_obj_in_list((char *) strKeyword.c_str(), vtor(iSource)->contents);

	if (!obj)
		return;

	if (iCount < 0 && obj->count > (-1 * iCount))
		iCount = obj->count + iCount;

	obj_from_room(&obj, iCount);
	obj_to_room(obj, iDestination);
}

// Supported Operations:
// math var add x
// math var subtract x
// math var invert x
// math var times x
// math var divide x
// math var power x
// math var abs x
// math var root x
// math var max x y z ... n
// math var min x y z ... n
// math var max_non_zero x y z ... n
// math var min_non_zero x y z ... n
void
r_math (CHAR_DATA *ch, char *arg, room_prog_var *& variable_list)
{
	std::string variable_name, operation, argument, buffer;
	argument = arg;

	argument = one_argument(argument, variable_name);
	argument = one_argument(argument, operation);
	argument = one_argument(argument, buffer);

	if (variable_name.empty() || operation.empty())
		return;

	if (is_variable_in_list(variable_list, variable_name))
	{
		std::string variable = get_variable_data(variable_list, variable_name);
		if (variable.empty() || !is_number(variable.c_str()))
			return;

		int new_value = 0;
		if (!operation.compare("invert"))
		{
			new_value = atoi(variable.c_str()) * -1;
		}
		else if (!operation.compare("abs"))
		{
			new_value = abs(atoi(variable.c_str()));
		}
		else
		{

			if (buffer.empty() || !is_number(buffer.c_str()))
				return;

			if (!operation.compare("add"))
			{
				new_value = atoi(variable.c_str()) + atoi(buffer.c_str());
			}
			else if (!operation.compare("sub"))
			{
				new_value = atoi(variable.c_str()) - atoi(buffer.c_str());
			}
			else if (!operation.compare("multiply"))
			{
				new_value = atoi(variable.c_str()) * atoi(buffer.c_str());
			}
			else if (!operation.compare("divide"))
			{
				new_value = atoi(variable.c_str()) / atoi(buffer.c_str());
			}
			else if (!operation.compare("power"))
			{
				new_value = pow(atoi(variable.c_str()), atoi(buffer.c_str()));
			}
			else if (!operation.compare("root"))
			{
				new_value = pow(atoi(variable.c_str()), 1 / atoi(buffer.c_str()));
			}
		}
		std::stringstream conversion;
		conversion << new_value;
		variable = conversion.str();
		set_variable_data(variable_list, variable_name, variable);
		return;
	}
	else
		return;
}
