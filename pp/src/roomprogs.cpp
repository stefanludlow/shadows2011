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
#include <vector>
#include <exception>
#include "server.h"

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "clan.h"
#include "group.h"


extern rpie::server engine;

/* script program commands */
#define MAKE_STRING(msg) (((std::ostringstream&) (std::ostringstream() << std::boolalpha << msg)).str())
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
#define RP_CLAN	37	/* send a message to clan members */
#define RP_TRANS_GROUP	38	/* send a message to clan members */
#define RP_SET		39
#define RP_CRIMINALIZE  40      /* criminalize a person or room */
#define RP_STRIP        41      /* takes a person's equipment and puts it all neatly in a bag */
#define RP_CLAN_ECHO         42      /* adds people to a clan at a certain rank */
#define RP_TAKEMONEY 43 /* Take money from a player's inventory TBA: Take money from room too */
#define RP_DELAY 44 /* Delayed command just like scommand */
#define RP_TEACH 45 /* Teach character skill */
#define RP_DOITANYWAY 46
#define RP_DOOR 47
#define RP_MATH 48
#define RP_NOOP 49 /* Do nothing - i.e. for comments */
#define RP_TRANSOBJ 50
#define RP_OBR 51
#define RP_TALLY 52
#define RP_OBJVNUM 53
#define RP_RANDOMOBJ 54
#define RP_INFO 55
#define RP_SETVAL 56
#define RP_CONCAT 57
#define RP_MFTOG 58
#define RP_NOOP2 59
#define RP_COPPER_TO_SHILLINGS 60
#define RP_MCOPY 61
#define RP_AFFECT 62

extern std::multimap<int, room_prog> mob_prog_list;
extern std::multimap<int, room_prog> obj_prog_list;

class room_prog_var {
public:
	std::string data;
	std::string name;
	room_prog_var *next_var;

	room_prog_var() {
		next_var = NULL;
	}
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
void r_mcopy (CHAR_DATA * ch, char *argument);
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
void r_tally (CHAR_DATA *ch, std::string argument, room_prog_var *&);
void r_objvnum (CHAR_DATA *ch, std::string argument, room_prog_var *&);
void r_randomobj (CHAR_DATA *ch, std::string argument, room_prog_var *&);
void r_info (CHAR_DATA *, std::string, room_prog_var *&);
void r_setval (CHAR_DATA *, std::string);
void r_concat (CHAR_DATA *, std::string, room_prog_var *&);
void r_mftog (CHAR_DATA *, std::string);
void r_coppertoshillings (std::string, room_prog_var *&);
void r_rmaffect (CHAR_DATA *, char *argument);

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
	"clan",
	"trans_group",
	"set",
	"criminalize",
	"strip",
	"clanecho",
	"takemoney",
	"delay",
	"teach",
	"doitanyway",
	"door",
	"math",
	"noop",
	"transobj",
	"obr",
	"tally",
	"objvnum",
	"randomobj",
	"info",
	"setval",
	"concat",
	"mftog",
	"//",
	"copper_to_shillings",
	"mcopy",
	"rmaffect",
	"\n"
};

/** 
 * rmaffect <room-num>, <affect string>, duration, intensity
 * will add the affect to the specidifed room lasting duration at itensity
 * room-num = -1 is the current room
 * duration = 1 hour by default
 * instensity = 1 by default
 *
 */
void
r_rmaffect (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80], arg4[80];
	char bufarg[MAX_STRING_LENGTH] = {'\0'};
	char err_buf[MAX_STRING_LENGTH];
	ROOM_DATA *troom;
	int room;
	int virt;
	int duration;
	int intensity;
	int nFlag;
	
	argument = one_argument(argument, arg1);
	
	if (!strcmp(arg1, "-1"))
		virt = ch->room->nVirtual;
	else
		virt = strtol(arg1, NULL, 10);
	
	if (!(troom = vtor (virt)))
	{
		send_to_char ("Error: rmaffect: No such room.\n", ch);
		return;
	}
	
	argument = one_argument(argument, arg2);
	nFlag = lookup_value(arg2, REG_AFFECT);
	
	if (nFlag == -1)
	{
		send_to_char ("Error: rmaffect: No such room-affect.\n", ch);
		return;
	}
	
	argument = one_argument(argument, arg3);
	
	if (!strcmp(arg3, "delete"))
	{
		remove_room_affect (troom, nFlag);
		return;
	}
	
	if (!arg3)
		duration = 1;
	else 
		duration = atoi(arg3);
	
	
	argument = one_argument(argument, arg4);
	if (!arg4)
		intensity = 1;
	else
		intensity = atoi(arg4);
	
	add_room_affect (&troom->affects, nFlag, duration, intensity);
	save_room_affects (troom->zone);	

}	
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
				strArgument = keys + " " + strArgument;
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
			strArgument = keys + " " + strArgument;
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

int
o_prog (CHAR_DATA *ch, char *argument, room_prog prog)
{
	std::string cmd, keys, strArgument = argument;
	strArgument = one_argument (strArgument, cmd);
	strArgument = one_argument (strArgument, keys);

	if (cmd.empty())
		return 0;

	if (!cmd.empty() && r_isname((char *) cmd.c_str(), prog.command))
	{
		if (!prog.keys || !*prog.keys)
		{
			strArgument = keys + " " + strArgument;
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
	// set null or empty argument to 'NOARG'
	if (!argument || !*argument)
		argument="NOARG";

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

		while (line.find("\\n") != std::string::npos)
		{
			line.replace(line.find("\\n"), 2, "\n");
		}

		while (line.find("\\t") != std::string::npos)
		{
			line.replace(line.find("\\t"), 2, "\t");
		}

		// Work out which rprog command to do
		std::string strCommand;
		line = one_argument(line, strCommand);

		char buf [MAX_STRING_LENGTH]; // doit isn't really set up to handle constant c_str so must convert to non-const
		sprintf(buf, "%s", line.c_str());
		int returnval = 0;
		if ((returnval = doit (ch, strCommand.c_str(), buf, command, keyword, argument, local_variables))!=1)
		{
			if ((engine.in_build_mode () || engine.in_test_mode()) && returnval == -1)
			{
				std::ostringstream oss;
				oss << "#1Error in prog line:#0 " << strCommand << " " << buf << std::endl;
				send_to_char((char*)oss.str().c_str(),ch);
			}
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
reval (CHAR_DATA * ch, char *arg, room_prog_var *& variable_list)
{
	int dir, nFlag = 0, nStat = 0;
	int nArg1 = 0, nArg2 = 0, nArg3 = 0;
	long virt = 0, who;
	char tmp2[80], *dbuf, rbuf[80], sarg[80];
	CHAR_DATA *tmp_ch, *tch1, *tch2;
	OBJ_DATA *obj;
	bool check;
	ROOM_DATA *troom = NULL;

	*rbuf = '\0';
	strncpy (sarg, arg, 79);

	while (*arg != '(' && *arg != '\0') {
		arg++;
	}

	if (*arg != '\0') {
		arg++;
	}

	std::string temp = arg;
	size_t tempFind = temp.find_first_of(")");
	if (tempFind != std::string::npos) {
		temp = temp.substr(0, tempFind);
	}
	else {
		std::stringstream errorWhat;
		errorWhat << ch->tname << " has triggered an rprog with an if statement missing a closing bracket.";
		send_to_gods(errorWhat.str().c_str());
		ifin[nNest] = 1;
		return;
	}

	size_t sizeTemp = temp.length();

	if (sizeTemp > 79) {
		ifin[nNest] = 1;
		return;
	}

	strncpy (tmp2, temp.c_str(), 79);

	tempFind = temp.find_first_of(",");

	if (tempFind != std::string::npos) {
		size_t dSize = temp.length() - tempFind;
		//dbuf++;
		dbuf = &(temp[tempFind + 1]);
		while (isspace (*dbuf)) {
			dbuf++;
		}
		strncat (rbuf, tmp2, (temp.length() - dSize));
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

	else if (!strncmp(sarg, "admin", 5))
	{
		if (!GET_TRUST(ch))
			ifin[nNest] = 1;
		return;
	}

	else if (!strncmp(sarg, "fighting", 8))
	{
		if (!ch->fighting)
			ifin[nNest] = 1;
		return;
	}

	else if (!strncmp (sarg, "delayed", 7))
	{
		if (!get_second_affect (ch, SA_COMMAND, NULL))
		{
			ifin[nNest] = 1;
		}
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

		if (!EXIT(ch, direction))
			ifin[nNest] = 1;
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
		else if (!strncmp (dbuf, "harad", 5))
		{
			currency = 3;
		}
		else if (!strncmp (dbuf, "northman", 8) || !strncmp(dbuf, "shillings", 9))
		{
			currency = 4;
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

	/* Check to see if an action flag is set */
	/*  if mflag(flag,target) */
	else if (!strncmp (sarg, "mflag", 5))
	{
		CHAR_DATA *tch;
		if (is_number(dbuf))
		{
			if (atoi(dbuf) == -1 && IS_NPC(ch))
			{
				tch = ch;
			}
			else
			{
				for (tch = ch->room->people; tch; tch = tch->next_in_room)
				{
					if (!IS_NPC(tch))
						continue;

					if (tch->mob->nVirtual == atoi(dbuf))
						break;
				}
			}
		}
		else
		{
			tch = get_char_room (dbuf, ch->in_room);
		}
		if (!tch)
		{
			ifin[nNest] = 1;
			return;
		}
		if (!IS_NPC(tch))
		{
			ifin[nNest] = 1;
			return;
		}
		if ((nFlag = index_lookup (action_bits, rbuf)) == -1)
		{
			ifin[nNest] = 1;
			return;
		}
		if (!IS_SET (tch->act, (1 << nFlag)))
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

	/* Check to see if an affect exists in the given room at a certain level*/
	/* Usage: if raffect(room-affect,roomvnum,level)            */
	/* if raffect (Shadow, -1, 4) */
	else if (!strncmp (sarg, "raffect", 7))
	{
		
		char * room_str;
		char * check_str;
		int check_level;
		room_str = strtok (dbuf," ,");
		check_str = strtok (NULL, " ,");
		sprintf(dbuf, "%s", room_str);
		
		if (check_str)
			check_level = atoi(check_str);
		else 
			check_level = -1;
		
		
		nFlag = lookup_value(rbuf, REG_AFFECT);
				 
		if (nFlag == -1)
		{
			send_to_char ("Error: if raffect: No such room-affect.\n", ch);
			ifin[nNest] = 1;
			return;
		}
		
		if (!strcmp(dbuf, "-1"))
			virt = ch->room->nVirtual;
		else
			virt = strtol (dbuf, NULL, 10);
		
		if (!(troom = vtor (virt)))
		{
			send_to_char ("Error: if raffect: No such room.\n", ch);
			ifin[nNest] = 1;
			return;
		}
		
		
		if (!is_room_affected (troom->affects, nFlag))
		{
			ifin[nNest] = 1;
			return;
		
		}
	//yes it is reveresed, but progs are stupid in this respect
		if ((troom->affects->a.room.intensity == check_level))
		{
			ifin[nNest] = 0;
			return;
		}	
		
		if (check_level > 0)
		{
			ifin[nNest] = 1;
			return;
		}
		
		return;
	}
	
	/* Check to see if person is in a group. */
	/* Usage: if in_group()   					  */

	else if (!strncmp (sarg, "in_group", 8))
	{
		if (!is_with_group (ch))
		{
			ifin[nNest] = 1;
		}
		return;
	}

	/* Check to see if mob/player are in same group as the second mob */
	/* Usage: if is_grouped_with(first,second)                        */
	/* Only checks in current room. To denote player use -1   */
		
	else if (!strncmp (sarg, "is_grouped_with", 6))
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
		if (!are_grouped(tch1, tch2))
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
			nArg1 = index_lookup (skills, rbuf);
			skill_use(ch,nArg1,0); // skill use for chance to improve
			if ((nArg1 != -1)
				&& ((unsigned int) skill_level (ch, nArg1, 0) >= dice (nArg2, nArg3)))
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
			return; // no chance to improve since it was a minimum test not a check
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

		ifin[nNest] = 1;
		if (tmp_ch->right_hand)
		{
			if (tmp_ch->right_hand->nVirtual == atol(dbuf))
				ifin[nNest] = 0;
			else if (GET_ITEM_TYPE(tmp_ch->right_hand) == ITEM_CONTAINER || GET_ITEM_TYPE(tmp_ch->right_hand) == ITEM_SHEATH || GET_ITEM_TYPE(tmp_ch->right_hand) == ITEM_QUIVER || GET_ITEM_TYPE(tmp_ch->right_hand) == ITEM_KEYRING)
			{
				for (obj = tmp_ch->right_hand->contains; obj; obj = obj->next_content)
				{
					if (obj->nVirtual == atol(dbuf))
					{
						ifin[nNest] = 0;
						break;
					}
					if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER || GET_ITEM_TYPE(obj) == ITEM_SHEATH || GET_ITEM_TYPE(obj) == ITEM_QUIVER || GET_ITEM_TYPE(obj) == ITEM_KEYRING)
					{
						for (OBJ_DATA *tobj = obj->contains; tobj; tobj = tobj->next_content)
						{
							if (tobj->nVirtual == atol(dbuf))
								ifin[nNest] = 0;
						}
					}
				}
			}
		}
		if (tmp_ch->left_hand)
		{
			if (tmp_ch->left_hand->nVirtual == atol(dbuf))
				ifin[nNest] = 0;
			else if (GET_ITEM_TYPE(tmp_ch->left_hand) == ITEM_CONTAINER || GET_ITEM_TYPE(tmp_ch->left_hand) == ITEM_SHEATH || GET_ITEM_TYPE(tmp_ch->left_hand) == ITEM_QUIVER || GET_ITEM_TYPE(tmp_ch->left_hand) == ITEM_KEYRING)
			{
				for (obj = tmp_ch->left_hand->contains; obj; obj = obj->next_content)
				{
					if (obj->nVirtual == atol(dbuf))
					{
						ifin[nNest] = 0;
						break;
					}
					if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER || GET_ITEM_TYPE(obj) == ITEM_SHEATH || GET_ITEM_TYPE(obj) == ITEM_QUIVER || GET_ITEM_TYPE(obj) == ITEM_KEYRING)
					{
						for (OBJ_DATA *tobj = obj->contains; tobj; tobj = tobj->next_content)
						{
							if (tobj->nVirtual == atol(dbuf))
								ifin[nNest] = 0;
						}
					}
				}
			}
		}
		if (tmp_ch->equip)
		{
			for (OBJ_DATA *equipped = tmp_ch->equip; equipped; equipped = equipped->next_content)
			{
				if (equipped->nVirtual == atol(dbuf))
					ifin[nNest] = 0;
				else if (GET_ITEM_TYPE(equipped) == ITEM_CONTAINER || GET_ITEM_TYPE(equipped) == ITEM_SHEATH || GET_ITEM_TYPE(equipped) == ITEM_QUIVER || GET_ITEM_TYPE(equipped) == ITEM_KEYRING)
				{
					for (obj = equipped->contains; obj; obj = obj->next_content)
					{
						if (obj->nVirtual == atol(dbuf))
						{
							ifin[nNest] = 0;
							break;
						}
						if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER || GET_ITEM_TYPE(obj) == ITEM_SHEATH || GET_ITEM_TYPE(obj) == ITEM_QUIVER || GET_ITEM_TYPE(obj) == ITEM_KEYRING)
						{
							for (OBJ_DATA *tobj = obj->contains; tobj; tobj = tobj->next_content)
							{
								if (tobj->nVirtual == atol(dbuf))
									ifin[nNest] = 0;
							}
						}
					}
				}
			}
		}

		return;
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
	else if (!strncmp (sarg, "defined", 7))
	{
		if (!is_variable_in_list(variable_list, temp.c_str()))
		{
			ifin[nNest] = 1;
			return;
		}
	}

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
			if (is_number(lhs.c_str()) && is_number(rhs.c_str()))
			{
				if (atoi(lhs.c_str()) >= atoi(rhs.c_str()))
				{
					ifin[nNest] = 1;
					return;
				}
				return;
			}
			else
			{
				ifin[nNest] = 1;
				return;
			}
		case '>':
			if (is_number(lhs.c_str()) && is_number(rhs.c_str()))
			{
				if (atoi(lhs.c_str()) <= atoi(rhs.c_str()))
				{
					ifin[nNest] = 1;
					return;
				}
				return;
			}
			else
			{
				ifin[nNest] = 1;
				return;
			}
		}
	}
}

int
doit (CHAR_DATA *ch, const char *func, char *arg, char *command, char *keyword, char *argument, room_prog_var *& variable_list)
{
	int i;
	char tmp[MAX_STRING_LENGTH];

	for (i = 0; (*rfuncs[i] != '\n'); i++)
		if (!strn_cmp (rfuncs[i], func, strlen(func)))
			break;

	switch (i)
	{
	case RP_NOOP: // Comment
		return 1;
	case RP_NOOP2: // <-- Comment in that style
		return 1;
	case RP_MATH:
		if (!ifin[nNest])
			r_math(ch, arg, variable_list);
		return 1;
	case RP_SETVAL:
		if (!ifin[nNest])
			r_setval(ch, arg);
		return 1;
	case RP_INFO:
		if (!ifin[nNest])
			r_info(ch, arg, variable_list);
		return 1;
	case RP_MFTOG:
		if (!ifin[nNest])
			r_mftog(ch, arg);
		return 1;
	case RP_CONCAT:
		if (!ifin[nNest])
			r_concat(ch, arg, variable_list);
		return 1;
	case RP_ATECHO:
		if (!ifin[nNest])
			r_atecho (ch, arg);

		return 1;
	case RP_OBJVNUM:
		if (!ifin[nNest])
			r_objvnum(ch, arg, variable_list);

		return 1;
	case RP_RANDOMOBJ:
		if (!ifin[nNest])
			r_randomobj(ch, arg, variable_list);

		return 1;
	case RP_LOADOBJ:
		if (!ifin[nNest])
			r_load_obj (ch, arg);

		return 1;
	case RP_TAKEMONEY:
		if (!ifin[nNest])
			r_takemoney(ch, arg);

		return 1;
	case RP_COPPER_TO_SHILLINGS:
		if (!ifin[nNest])
			r_coppertoshillings(arg, variable_list);
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

	case RP_MCOPY:
		if (!ifin[nNest])
			r_mcopy (ch,arg);

		return 1;

	case RP_AFFECT:
		if (!ifin[nNest])
			r_rmaffect (ch, arg);
			
			return 1;
	case RP_FORCE:
		if (!ifin[nNest])
			r_force (ch, arg);

		return 1;
	case RP_IF:
		if (!ifin[nNest])
			reval (ch, arg, variable_list);
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
	case RP_TALLY:
		if (!ifin[nNest])
			r_tally (ch, arg, variable_list);
		return 1;
	case RP_TRANSOBJ:
		if (!ifin[nNest])
			r_transobj(ch, arg);

		return 1;
	default:
		//system_log ("ERROR: unknown command in program", true);

		return -1;
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
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
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
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
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
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + MAKE_STRING(it->second.type) + "#0]\n";
		else
		{
			if (counter++ == atoi(ThisArgument.c_str()))
			{
				output += it->second.prog;
			}
		}
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
			it->second.command = duplicateString((char *) strArgument.c_str());
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
			it->second.keys = duplicateString((char *) strArgument.c_str());
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

/* Procedure called upon completion of editing a mob prog */
/* the vnum of the mob is stored in delay_info1 */
/* and the serial number of the prog is stored in delay_info2 */
void post_mpprg (DESCRIPTOR_DATA * d)
{
	CHAR_DATA* ch = d->character;
	int vnum = ch->delay_info1;
	int prog_number = ch->delay_info2;

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = mob_prog_list.equal_range(vnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == prog_number)
		{
			/* overwrite the program with the new string */
			free_mem(it->second.prog);
			it->second.prog = duplicateString(d->descStr);
			break;
		}
		count++;
	}

	free_mem(d->descStr);
	d->descStr = NULL;
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

			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",	ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = NULL;
			ch->desc->proc = (CALL_PROC*) post_mpprg;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->delay_info1 = ivnum; // store the vnum of the mob
			ch->delay_info2 = iprognum; // store the serial number of the prog
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
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",	ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = duplicateString(it->second.prog); // clone existing program as starting str
			ch->desc->proc = (CALL_PROC*) post_mpprg;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->delay_info1 = ivnum; // store the vnum of the mob
			ch->delay_info2 = iprognum; // store the serial number of the prog
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
do_opstat (CHAR_DATA *ch, char *argument, int cmd)
{
	std::vector<std::string> OprogTypes;
	OprogTypes.push_back("held");
	OprogTypes.push_back("worn");
	OprogTypes.push_back("personal");
	OprogTypes.push_back("room");
	OprogTypes.push_back("all");
	std::string strArgument = argument, ThisArgument, output = "";
	strArgument = one_argument(strArgument, ThisArgument);
	int lastVnum = -1, counter = 1;

	if (ThisArgument.empty())
	{
		for (std::multimap<int, room_prog>::iterator it = obj_prog_list.begin(); it != obj_prog_list.end(); it++)
		{
			if (lastVnum == -1 || lastVnum != it->first)
			{
				output += "\nObject #2" + MAKE_STRING(it->first) + "#0:\n";
				counter = 1;
			}
			lastVnum = it->first;
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + OprogTypes[it->second.type-1] + "#0]\n";
		}
		page_string (ch->desc, output.c_str());
		return;
	}

	if (ThisArgument[0] == '*')
	{
		ThisArgument.erase(0, 1);
		if (!is_number(ThisArgument.c_str()))
		{
			send_to_char ("Correct usage: #6oplist#0, #6oplist <vnum>#0, or #6oplist *<zone>#0.\n", ch);
			return;
		}
		int zone = atoi(ThisArgument.c_str());
		output += "Zone \"#F" + MAKE_STRING(zone_table[zone].name) + "#0\" [Zone " + ThisArgument + "]\n";
		for (std::multimap<int, room_prog>::iterator it = obj_prog_list.begin(); it != obj_prog_list.end(); it++)
		{
			if ((it->first/1000) != zone)
				continue;
			if (lastVnum == -1 || lastVnum != it->first)
				output += "\nObject #2" + MAKE_STRING(it->first) + "#0:\n";
			lastVnum = it->first;
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + OprogTypes[it->second.type-1] + "#0]\n";
		}
		page_string (ch->desc, output.c_str());
		return;
	}

	if (!is_number(ThisArgument.c_str()))
	{
		send_to_char ("Correct usage: #6oplist#0, #6oplist <vnum> [number]#0, or #6oplist *<zone>#0.\n", ch);
		return;
	}
	int vnum = atoi(ThisArgument.c_str());
	strArgument = one_argument(strArgument, ThisArgument);
	bool SingleProg = false;
	if (!ThisArgument.empty() && is_number(ThisArgument.c_str()))
	{
		SingleProg = true;
	}
	std::pair<std::multimap<int, room_prog>::iterator,std::multimap<int, room_prog>::iterator> pair = obj_prog_list.equal_range(vnum);
	output += "Object #2" + MAKE_STRING(vnum);
	if (SingleProg)
		output += "#0 Program Number #6" + ThisArgument + "#0:\n\n";
	else
		output += "#0:\n";
	counter = 1;
	for (std::multimap<int, room_prog>::iterator it = pair.first; it != pair.second; ++it)
	{
		if (!SingleProg)
			output += "[#B" + MAKE_STRING(counter++) + "#0]: Commands [#6" + it->second.command + "#0] Keys [#6" + it->second.keys + "#0] Type [#1" + OprogTypes[it->second.type-1] + "#0]\n";
		else
		{
			if (counter++ == atoi(ThisArgument.c_str()))
			{
				output += (it->second.prog == NULL ? "(null)\n" : it->second.prog);
			}
		}
	}
	page_string (ch->desc, output.c_str());
	return;
}

void
do_opadd (CHAR_DATA *ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum;

	strArgument = one_argument (strArgument, vnum);
	if (vnum.empty() || atoi(vnum.c_str()) < 1 || atoi(vnum.c_str()) > 99999)
	{
		send_to_char ("You must specify a valid vnum of the object to initialize a new oprog for.\n", ch);
		return;
	}

	room_prog prog;
	prog.prog = "";
	prog.keys = "";
	prog.command = "";
	prog.type = 5;

	obj_prog_list.insert(std::pair<int, room_prog>(atoi(vnum.c_str()), prog));
	int prog_num = 0;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(atoi(vnum.c_str()));
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		prog_num++;
	}
	send_to_char("Inserted blank object prog for Object vnum [#2", ch);
	send_to_char(vnum.c_str(), ch);
	send_to_char("#0] at position #6", ch);
	send_to_char(((std::ostringstream&) (std::ostringstream() << std::boolalpha << prog_num)).str().c_str(), ch);
	send_to_char("#0.\n", ch);
	return;
}

void
do_opdel (CHAR_DATA *ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPDEL <vnum> <prognum>.\n", ch);
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
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			obj_prog_list.erase(it);
			std::string output;
			output = "Erasing object prog number #6" + prognum + "#0 from Object vnum [#2" + vnum + "#0].\n";
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_opcmd (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPCMD <vnum> <prognum> <cmd>.\n", ch);
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
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			it->second.command = duplicateString((char *) strArgument.c_str());
			std::string output;
			output = "Installed command(s) [#6" + strArgument + "#0] for object prog number #6" + prognum + "#0 for Object vnum [#2" + vnum + "#0].\n";
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_opkey (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPKEY <vnum> <prognum> <key>.\n", ch);
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

	/*if (strArgument.empty())
	{
	send_to_char("What keyword would you like to install?\n", ch);
	return;
	}*/

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			it->second.keys = duplicateString((char *) strArgument.c_str());
			std::string output;
			if (strArgument.empty())
			{
				it->second.keys = '\0';
				output = "Keyword removed.\n";
			}
			else
			{
				output = "Installed keyword(s) [#6" + strArgument + "#0] for object prog number #6" + prognum + "#0 for Object vnum [#2" + vnum + "#0].\n";
			}
			send_to_char(output.c_str(), ch);
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

// Held, room, personal, worn, all
void
do_optype (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPTYPE <vnum> <prognum> <key>.\n", ch);
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
		send_to_char("What type are you setting this program to (held, worn, personal, room, all)?\n", ch);
		return;
	}

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			if (strArgument.find("held") != std::string::npos)
			{
				it->second.type = 1;
				send_to_char("Set type to held.\n", ch);
			}
			else if (strArgument.find("worn") != std::string::npos)
			{
				it->second.type = 2;
				send_to_char("Set type to worn.\n", ch);
			}
			else if (strArgument.find("personal") != std::string::npos)
			{
				it->second.type = 3;
				send_to_char("Set type to personal.\n", ch);
			}
			else if (strArgument.find("room") != std::string::npos)
			{
				it->second.type = 4;
				send_to_char("Set type to room.\n", ch);
			}
			else if (strArgument.find("all") != std::string::npos)
			{
				it->second.type = 5;
				send_to_char("Set type to all.\n", ch);
			}
			else
			{
				send_to_char("Type is one of: held, worn, personal, room or all.\n", ch);
				return;
			}
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}


/* Procedure called upon completion of editing an object prog */
/* the vnum of the object is stored in delay_info1 */
/* and the serial number of the prog is stored in delay_info2 */
void post_opprg (DESCRIPTOR_DATA * d)
{
	CHAR_DATA* ch = d->character;
	int vnum = ch->delay_info1;
	int prog_number = ch->delay_info2;

	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(vnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == prog_number)
		{
			/* overwrite the program with the new string */
			free_mem(it->second.prog);
			it->second.prog = duplicateString(d->descStr);
			break;
		}
		count++;
	}

	free_mem(d->descStr);
	d->descStr = NULL;
}

void
do_opprg (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPPRG <vnum> <prognum>.\n", ch);
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

	/* this is nearly unnecessary, but it does ensure that the program has been properly opadded first */
	int count = 1;
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",	ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = NULL;
			ch->desc->proc = (CALL_PROC*) post_opprg;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->delay_info1 = ivnum; // store the vnum of the item
			ch->delay_info2 = iprognum; // store the serial number of the prog
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_opapp (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string strArgument = argument, vnum, prognum;
	strArgument = one_argument(strArgument, vnum);
	strArgument = one_argument(strArgument, prognum);

	if (vnum.empty() || prognum.empty())
	{
		send_to_char("Format is OPAPP <vnum> <prognum>.\n", ch);
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
	std::pair<std::multimap<int, room_prog>::iterator, std::multimap<int, room_prog>::iterator> range = obj_prog_list.equal_range(ivnum);
	for (std::multimap<int, room_prog>::iterator it = range.first; it != range.second; ++it)
	{
		if (count == iprognum)
		{
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",	ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = duplicateString(it->second.prog); // clone existing program as starting str
			ch->desc->proc = (CALL_PROC*) post_opprg;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->delay_info1 = ivnum; // store the vnum of the item
			ch->delay_info2 = iprognum; // store the serial number of the prog
			return;
		}
		count++;
	}

	std::string output;
	output = "Object number [#2" + vnum + "#0] does not have a object program with id #6" + prognum + "#0.\n";
	send_to_char(output.c_str(), ch);
	return;
}

void
do_rpadd (CHAR_DATA * ch, char *argument, int cmd)
{
	struct room_prog *t, *old, *tmp;

	t = new room_prog;

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
			free_mem (tmp);
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
			t->command = duplicateString (arg2);
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
			t->keys = duplicateString (arg2);
			send_to_char ("Keywords installed.\n\r", ch);
			return;
		}
	}
	send_to_char ("That program does not exist.\n\r", ch);
	return;
}


/* Procedure called upon completion of editing a room prog */
/* the serial number of the prog is stored in delay_info1 */
void post_rpprg (DESCRIPTOR_DATA * d)
{
	CHAR_DATA* ch = d->character;
	int prog_number = ch->delay_info1;
	ROOM_DATA* r = vtor(ch->in_room);
	struct room_prog* prg = NULL;

	if (!r)
		return;

	prg = r->prg;

	/* Select the N-th prog */
	for (int i=1; prg!=NULL; prg=prg->next, i++)
	{
		if (i==prog_number)
		{
			/* overwrite the program with the new string */
			free_mem(prg->prog);
			prg->prog = duplicateString(d->descStr);
			break;
		}
	}

	free_mem(d->descStr);
	d->descStr = NULL;
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
			send_to_char ("Enter program now, Terminate entry with an '@'\n\r",	ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = NULL;
			ch->desc->proc = (CALL_PROC*) post_rpprg;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->delay_info1 = i; // store which program to edit for the callback
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
			send_to_char("Append to program now, Terminate entry with an '@'\n\r", ch);
			make_quiet (ch);
			free_mem(ch->desc->descStr);
			ch->desc->descStr = duplicateString(t->prog); // clone existing program as starting str
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->desc->proc = (CALL_PROC*) post_rpprg;
			ch->delay_info1 = i; // store which program to edit for the callback
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
	char err_buf[MAX_STRING_LENGTH];
	int dir;
	int location, location2;
	ROOM_DATA *source_room;
	ROOM_DATA *target_room;

	arg_splitter (3, argument, buf1, buf2, buf3);

	if (!*buf1 || !*buf2 || !*buf3)
	{
		sprintf(err_buf, "ERROR: Missing args in r_link in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
		sprintf(err_buf, "ERROR: Invalid direction in r_link in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
		sprintf(err_buf, "ERROR: target room not found in r_link in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (!(source_room = vtor (location)))
	{
		sprintf(err_buf, "ERROR: cha room not found in r_link in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (source_room->dir_option[dir])
		vtor (source_room->dir_option[dir]->to_room)->dir_option[rev_dir[dir]] =
		0;

	source_room->dir_option[dir] = new room_direction_data;
	source_room->dir_option[dir]->general_description = 0;
	source_room->dir_option[dir]->keyword = 0;
	source_room->dir_option[dir]->exit_info = 0;
	source_room->dir_option[dir]->key = -1;
	source_room->dir_option[dir]->to_room = target_room->nVirtual;

	target_room->dir_option[rev_dir[dir]] = new room_direction_data;
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
	char err_buf[MAX_STRING_LENGTH];
	int dir;
	int location, location2;
	ROOM_DATA *source_room;
	ROOM_DATA *target_room;

	arg_splitter (3, argument, buf1, buf2, buf3);

	if (!*buf1 || !*buf2 || !*buf3)
	{
		sprintf(err_buf, "ERROR: Missing args in r_exit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
		sprintf(err_buf, "ERROR: Invalid direction in r_exit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
		sprintf(err_buf, "ERROR: tar room not found in r_exit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (!(source_room = vtor (location)))
	{
		sprintf(err_buf, "ERROR: cha room not found in r_exit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (!source_room->dir_option[dir])
		source_room->dir_option[dir] = new room_direction_data;

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
	char err_buf[MAX_STRING_LENGTH];
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
		sprintf(err_buf, "ERROR: Room not found in r_atlook in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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

		if (!vtor(room_vnum))
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

void
r_tally (CHAR_DATA *ch, std::string argument, room_prog_var *&variable_list)
{
	std::string buf;
	CHAR_DATA *tch = NULL;
	ROOM_DATA *room = NULL;
	argument = one_argument(argument, buf);
	if (buf.find("-1") != std::string::npos) // -1 means trigger puller
	{
		tch = ch;
		room = ch->room;
		argument = one_argument(argument, buf); // Ignore next input
	}
	else
	{
		std::string strroom;
		argument = one_argument(argument, strroom);
		if (strroom.find("-1") != std::string::npos)
		{
			room = ch->room;
		}
		else if (is_number((char *) strroom.c_str()))
		{
			if (!vtor(atoi((char *) strroom.c_str())))
				return;

			room = vtor(atoi((char *) strroom.c_str()));
		}

		if (is_number((char *) buf.c_str()))
		{
			for (CHAR_DATA *xch = room->people; xch; xch = xch->next_in_room)
			{
				if (!IS_NPC(xch))
					continue;

				if (xch->mob->nVirtual == atoi((char *) buf.c_str()))
				{
					tch = xch;
					break;
				}
			}
		}
		else
		{
			tch = get_char_room_vis2 (ch, room->nVirtual, (char *) buf.c_str());
		}
		if (!tch)
			return;
	}

	float total = 0.0;
	OBJ_DATA *obj;
	char buffer [MAX_STRING_LENGTH] = {'\0'};
	if ((obj = tch->right_hand))
		total += tally (obj, buffer, 1);
	if ((obj = tch->left_hand))
		total += tally (obj, buffer, 1);
	for (int location = 0; location < MAX_WEAR; location++)
	{
		if (!(obj = get_equip (tch, location)))
			continue;
		total += tally (obj, buffer, 1);
	}

	std::string strTally = MAKE_STRING(total);
	argument = one_argument (argument, buf);   // Get variable name
	if (buf.empty())
		return;

	if (is_variable_in_list(variable_list, buf))
		set_variable_data(variable_list, buf, strTally);
	else
		add_variable_to_list(variable_list, buf, strTally);


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
	char err_buf[MAX_STRING_LENGTH];
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

	std::string bufchange = buf;
	while (bufchange.find("$n") != std::string::npos)
	{
		bufchange.replace(bufchange.find("$n"), 2, char_short(ch));
	}

	sprintf (buf, "%s", bufchange.c_str());

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

		if ( !vtor (strtol(loc_str1, NULL, 10)) ) 
		{
			sprintf(err_buf, "ERROR: Room %d not found in r_atecho in room %d.", (strtol(loc_str1, NULL, 10)), ch->room->nVirtual);
			system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
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
		sprintf(err_buf, "ERROR: Invalid direction in r_unlink in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (!(troom = vtor (location)))
	{
		sprintf(err_buf, "ERROR: cha room not found in r_unlink in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
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
		sprintf(err_buf, "ERROR: Invalid direction in r_unexit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (!(troom = vtor (location)))
	{
		sprintf(err_buf, "ERROR: cha room not found in r_unexit in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	troom->dir_option[dir] = 0;
}

void
r_give (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	char buf [AVG_STRING_LENGTH];
	char err_buf[MAX_STRING_LENGTH];

	argument = one_argument(argument, buf);
	obj = load_object (atoi (buf));

	if (is_number(argument))
		obj->count = atoi(argument);
	if (obj)
		obj_to_char (obj, ch);
	else
	{
		sprintf(err_buf, "ERROR: Object does not exist in r_give in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
	}
}

void
r_take (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	char buf [AVG_STRING_LENGTH];
	char err_buf[MAX_STRING_LENGTH];

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
	{
		sprintf(err_buf, "ERROR: Object not found in r_take in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
	}
}

void
r_put (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80];
	char err_buf[MAX_STRING_LENGTH];

	half_chop (argument, arg1, arg2);

	if (!vtor (atoi (arg2)) || !vtoo (atoi (arg1)))
	{
		sprintf(err_buf, "ERROR: Object does not exist in r_put in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
	}
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
	char err_buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int virt;

	arg_splitter (3,argument, arg1, arg2, arg3);

	if (!strcmp(arg2, "-1"))
		virt = ch->room->nVirtual;
	else
		virt = atoi (arg2);

	if (!vtor (virt))
	{
		sprintf(err_buf, "ERROR: Object-room not found in r_get in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
	
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
	{
		sprintf(err_buf, "ERROR: Object not found in r_get in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		
		return;
	}
}

void
r_lock (CHAR_DATA * ch, char *argument)
{
	long virt;
	int dir;
	char arg1[80], arg2[80];
	char err_buf[MAX_STRING_LENGTH];


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
		sprintf(err_buf, "ERROR: Invalid direction in r_lock in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}
}

void
r_loadmob (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80];
	char err_buf[MAX_STRING_LENGTH];
	int mob_vnum, room_vnum;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

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
		sprintf(err_buf, "ERROR: Mobile does not exist in r_loadmob in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	CHAR_DATA * loaded_mob = load_mobile (mob_vnum);
	char_to_room (loaded_mob, room_vnum);
	if (!strn_cmp(argument, "stayput", 7))
	{
		loaded_mob->act |= ACT_STAYPUT;
		save_stayput_mobiles ();
	}
}

void
r_mcopy (CHAR_DATA * ch, char *argument)
{
	do_mcopy(ch, argument, 0);
}


void
r_exmob (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *ptrMob = NULL, *tmp_ch = NULL;
	char arg1[80], arg2[80];
	char err_buf[MAX_STRING_LENGTH];
	int virt;
	long nMobVnum = 0;

	half_chop (argument, arg1, arg2);

	if (!strcmp(arg2, "-1"))
		virt = ch->room->nVirtual;
	else
		virt = atol (arg2);

	if (!vtor (virt))
	{
		sprintf(err_buf, "ERROR: Mobile-room does not exist in r_exmob in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
			sprintf(err_buf, "ERROR: Mobile does not exist in r_exmob in room %d.", ch->room->nVirtual);
			system_log (err_buf, true);
			return;
		}
	}
	else if (!(ptrMob = get_char_room (arg1, virt)))
	{
		sprintf(err_buf, "ERROR: Mobile does not exist in r_exmob in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	extract_char (ptrMob);
}

void
r_rftog (CHAR_DATA * ch, char *arg)
{
	int flag;
	char buf[80], rbuf[80];
	char err_buf[MAX_STRING_LENGTH];
	ROOM_DATA *troom;

	*buf = *rbuf = '\0';

	half_chop (arg, buf, rbuf);

	if (!strcmp(rbuf, "-1"))
		troom = vtor (ch->room->nVirtual);
	else
		troom = vtor(atoi(rbuf));

	if (!troom)
	{
		sprintf(err_buf, "ERROR: Unknown room in r_rftog in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
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
	else if (!strncmp(arg1, "group", 5))
		mob = -2;
	else if (!strncmp(arg1, "-1", 2))
		mob = -1;
	else if (is_number(arg1))
		mob = atoi (arg1);
	else mob = -3;

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
		sprintf(err_buf, "ERROR: unknown room in r_force in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}

	if (mob == -3)
	{
		tmp_ch = get_char_room_vis2(ch, room, arg1);
		if (!tmp_ch)
			return;

		command_interpreter (tmp_ch, arg3);
	}
	else
	{

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
			else if (mob == -2 && are_grouped(ch, tmp_ch))
			{
				sprintf (buf, arg3, GET_NAME(ch));
				command_interpreter (tmp_ch, buf);
			}
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
	char err_buf[MAX_STRING_LENGTH];
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
		sprintf(err_buf, "ERROR: unknown origin room in r_transmob in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		nOriginRoom = ch->room->nVirtual;
	}

	if (!vtor (nTargetRoom))
	{
		sprintf(err_buf, "ERROR: unknown desination room in r_transmob in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
	int subject_mnum, origin_rnum, destination_rnum;

	// Parse the arguments into parts (could probably just use strtol)
	arg_splitter (3, argument, subject_arg, origin, destination);

	subject_mnum = strtol (subject_arg, 0, 10);
	if (strtol(origin, 0, 10) == 0) {
		origin_rnum = ch->room->nVirtual;
	}
	else {
		origin_rnum = strtol (origin, 0, 10);
	}

	if (strtol(destination, 0, 10) == 0) {
		destination_rnum = ch->room->nVirtual;
	}
	else {
		destination_rnum = strtol (destination, 0, 10);
	}

	// If origin_rnum doesn't exist, default to user's room
	if (!vtor (origin_rnum))
	{
		sprintf(err_buf, "ERROR: unknown origin room in r_trans_group in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		origin_rnum = ch->room->nVirtual;
	}

	// If the destination room doesn't exist, not need to continue
	if (!vtor (destination_rnum))
	{
		sprintf(err_buf,"ERROR: unknown desination room in r_trans_group in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	if (subject == NULL) {
		return;
	}
	/// loop here

	// if subdued we transfer the captor's group
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
r_atwrite (CHAR_DATA * ch, char *argument)
{
	std::string strArgument = argument, board, title;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char * date;
	char date_buf[MAX_STRING_LENGTH];
	char * suf;
	int day;

	time_t time_now = time(0);
	date = (char *) asctime (localtime (&time_now));
	date[strlen(date) - 1] = '\0';

	while (strArgument.find('@') != std::string::npos)
	{
		strArgument.replace(strArgument.find('@'), 1, ch->tname);
	}
	while (strArgument.find("$n") != std::string::npos)
	{
		strArgument.replace(strArgument.find("$n"), 2, char_short(ch));
	}
	while (strArgument.find("$r") != std::string::npos)
	{
		strArgument.replace(strArgument.find("$r"), 2, ch->room->name);
	}
	while (strArgument.find("\\n") != std::string::npos)
	{
		strArgument.replace(strArgument.find("\\n"), 2, "\n");
	}

	strArgument = one_argument (strArgument, board);
	strArgument = one_argument (strArgument, title);

	if (strArgument.empty() || board.empty() || title.empty())
	{
		return;
	}

	title[0] = toupper(title[0]);

	if (board[0] == '*') // PC Board
	{
		board.erase(0, 1);
		board[0] = toupper(board[0]);
		add_message (1, board.c_str(), -2, "Server", date,  (char *) title.c_str(), "", (char *) strArgument.c_str(), 0);
	}
	else if (board[0] == '!')   // Virtual board
	{
		board.erase(0, 1);
		add_message (1, board.c_str(), -5, "Server", date, (char *) title.c_str(), "", (char *) strArgument.c_str(), 0);
	}
	else						// Non-virtual board (i.e. board object)
	{
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
			sprintf (date_buf, "%d%s %s, %d SR", day, suf, month_short_name[time_info.month], time_info.year);
		else
		{
			if (time_info.holiday > 0)
			{
				sprintf (date_buf, "%s, %d SR", holiday_short_names[time_info.holiday], time_info.year);
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

		mysql_safe_query ("SELECT * FROM boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1", (char *) board.c_str());
		result = mysql_store_result(database);
		int free_slot = 1;
		if ((row = mysql_fetch_row (result)))
		{
			free_slot = atoi(row[1]);
			free_slot++;
		}

		mysql_safe_query ("INSERT INTO boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', %d)", (char *) board.c_str(), free_slot, (char *) title.c_str(), "System", date, date_buf, (char *) strArgument.c_str(), (int) time(0));

		if (result)
			mysql_free_result (result);
	}
	return;
}

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
		std::string strReplacement = argument;
		if (strReplacement.find("$n") != std::string::npos)
			strReplacement.replace(strReplacement.find("$n"), 2,  char_short(ch));
		argument = (char *) strReplacement.c_str();
		reformat_string(argument, &formatted);
		for (tch = room->people; tch; tch = tch->next_in_room)
		{
			if (is_clan_member(tch, buf))
			{
				send_to_char(formatted, tch);
				send_to_char("", tch);
			}
		}
		free_mem(formatted);
	}
	else if (!strncmp(buf, "room", 4))
	{
		char * formatted;
		argument = one_argument(argument, buf);
		std::string strReplacement = argument;
		if (strReplacement.find("$n") != std::string::npos)
			strReplacement.replace(strReplacement.find("$n"), 2,  char_short(ch));
		argument = (char *) strReplacement.c_str();
		reformat_string(argument, &formatted);
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (is_clan_member(tch, buf))
			{
				send_to_char(formatted, tch);
				send_to_char("", tch);
			}
		}
		free_mem(formatted);
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
		std::string strReplacement = argument;
		if (strReplacement.find("$n") != std::string::npos)
			strReplacement.replace(strReplacement.find("$n"), 2,  char_short(ch));
		argument = (char *) strReplacement.c_str();
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
			send_to_char ("", d->character);
		}
		free_mem(formatted);
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
		std::string strReplacement = argument;
		if (strReplacement.find("$n") != std::string::npos)
			strReplacement.replace(strReplacement.find("$n"), 2,  char_short(ch));
		argument = (char *) strReplacement.c_str();
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

		}
		free_mem(formatted);
	}
	return;
}

void
r_pain (CHAR_DATA * ch, char *argument)
{
	char arg1[80], arg2[80], arg3[80], arg4[80], arg5[80];
	char err_buf[MAX_STRING_LENGTH];
	int high, low, dam, type;
	int room;
	CHAR_DATA *victim;
	CHAR_DATA* tempnext=NULL;

	arg_splitter (5, argument, arg1, arg2, arg3, arg4, arg5);

	if (!strcmp(arg1, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg1);

	if (!vtor (room))
	{
		sprintf(err_buf,"ERROR: unknown room in r_pain in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;
	}
	low = atoi (arg2);
	high = atoi (arg3);
	if ((type = index_lookup (damage_type, arg5)) < 0)
	{
		type = 3;
	}
	if (!strncmp (arg4, "allbutme", 8))
	{
		for (victim = vtor (room)->people; victim;
			victim = tempnext)
		{
			/* save early so pointer isn't invalidated by death of char */
			tempnext = victim->next_in_room;

			if (victim == ch)
				continue;


			if ((dam = number (low, high)))
			{
				wound_to_char (victim, figure_location (victim, number (0, 10)),
					dam, type, 0, 0, 0);
			}

		}
	}
	else if (!strncmp(arg4, "all", 3))
	{
		for (victim = vtor(room)->people; victim; victim = tempnext)
		{

			/* save early so pointer isn't invalidated by death of char */
			tempnext = victim->next_in_room;

			if ((dam = number (low, high)))
			{
				wound_to_char (victim, figure_location (victim, number (0, 10)),dam, type, 0, 0, 0);
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
	char err_buf[MAX_STRING_LENGTH];
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
		sprintf(err_buf,"ERROR: unknown room in r_atread in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
	int room;
	OBJ_DATA *object = NULL;
	OBJ_DATA *next_object = NULL;
	ROOM_DATA *ptrRoom = NULL;

	arg_splitter (3, argument, arg1, arg2, arg3);

	if (!strcmp(arg1, "-1"))
		room = ch->room->nVirtual;
	else
		room = atoi (arg1);

	if (!(ptrRoom = vtor (room)))
	{
		sprintf(err_buf,"ERROR: unknown room in r_purge in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
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
	char err_buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *bag = NULL;
	char buf[MAX_STRING_LENGTH];

	if (drop_room == -1)
		drop_room = ch->room->nVirtual;

	if (!( vtor(drop_room)))
	{
		sprintf(err_buf,"ERROR: Room does not exist in r_strip in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		return;

	}
	else
	{
		bag = load_object (VNUM_JAILBAG);

		if (bag && (ch->right_hand || ch->left_hand || ch->equip))
		{
			sprintf (buf, "A bag belonging to %s sits here.", ch->short_descr);
			bag->description = duplicateString (buf);

			sprintf (buf, "a bag labeled '%s'", ch->short_descr);
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
	char err_buf[MAX_STRING_LENGTH];
	bool exit = false;
	OBJ_DATA *obj = NULL;
	int rvnum = 0, ovnum = 0, count = 0;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	rvnum = atoi(arg1);
	count = atoi(arg2);
	ovnum = atoi(arg3);
	argument = one_argument(argument, arg1);
	std::string buf = arg1;

	if ( rvnum == -1)
	{
		rvnum = ch->room->nVirtual;
	}

	if (!( vtor(rvnum) ))
	{
		sprintf(err_buf,"ERROR: Room does not exist in r_load_obj in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		exit = true;
	}
	if (count < 1)
	{
		sprintf(err_buf,"ERROR: Negative count specified in r_load_obj in room %d.", ch->room->nVirtual);
		system_log (err_buf, true);
		exit = true;
	}

	std::string color;
	if (!buf.empty())
	{
		int index = 0;
		if ((index = index_lookup (standard_object_colors, arg1)) != -1)
			color = standard_object_colors[index];
		else if ((index = index_lookup (fine_object_colors, arg1)) != -1)
			color = fine_object_colors[index];
		else if ((index = index_lookup (drab_object_colors, arg1)) != -1)
			color = drab_object_colors[index];
		else if ((index = index_lookup (fine_gem_colors, arg1)) != -1)
			color = fine_gem_colors[index];
		else if ((index = index_lookup (gem_colors, arg1)) != -1)
			color = gem_colors[index];
	}

	if (!color.empty())
	{
		if (!exit && !(obj = load_colored_object (ovnum, (char *) color.c_str())))
		{
			sprintf(err_buf,"ERROR: item does not exist in in r_load_obj in room %d.", ch->room->nVirtual);
			system_log (err_buf, true);
			exit = true;
		}
	}
	else
	{
		if (!exit && !(obj = load_object (ovnum)))
		{
			sprintf(err_buf,"ERROR: item does not exist in in r_load_obj in room %d.", ch->room->nVirtual);
			system_log (err_buf, true);
			exit = true;
		}
	}

	if ( !exit)
	{
		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE))
		{
			obj_to_room (obj, rvnum);
			for (int i = 1; i < count; i++)
			{
				if (!color.empty())
					obj = load_colored_object(ovnum, (char *) color.c_str());
				else
				{
					obj = load_object(ovnum);
				}
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
	char err_buf[MAX_STRING_LENGTH];

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
			sprintf(err_buf,"ERROR: Invalid amount argument in r_takemoney in room %d.", ch->room->nVirtual);
			system_log (err_buf, true);
			return;
		}
	}
	else
	{
		sprintf(err_buf,"ERROR: Missing amount argument in r_takemoney in room %d.", ch->room->nVirtual);
		send_to_gods(err_buf);
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
		else if (ThisArgument.find("harad") != std::string::npos)
			Currency = 3;
		else if (ThisArgument.find("northman") != std::string::npos || ThisArgument.find("shillings") != std::string::npos)
			Currency = 4;
		else
		{
			sprintf(err_buf,"ERROR: Invalid currency type in r_takemoney in room %d.", ch->room->nVirtual);
			send_to_gods(err_buf);
			return;
		}
	}
	else
	{
		sprintf(err_buf,"ERROR: Missing currency argument in r_takemoney in room %d.", ch->room->nVirtual);
		send_to_gods(err_buf);
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

void r_teach (CHAR_DATA *ch, char * argument)
{
	std::string ArgumentList = argument, ThisArgument;
	int index = 0;
	int i;
	SUBCRAFT_HEAD_DATA *craft;
	AFFECTED_TYPE *af;
	ArgumentList = one_argument (ArgumentList, ThisArgument);
	
	index = index_lookup(skills, ThisArgument.c_str());
	
	if (index > 0) //it is a skill
	{
		if (real_skill(ch, index)) // they know it already
	{
		return;
	}
		else
	{
			open_skill(ch, index); //they don't know, so learn it
		return;
	}
}

	else if (index == -1) //it's not a skill so we continue and test for crafts

	{	
					
		for (craft = crafts; craft; craft = craft->next)
		{
			if (!str_cmp (craft->subcraft_name, ThisArgument.c_str()))
				break;
			
		}	
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!get_affect (ch, i))
				break;
		}
		
		if (i > CRAFT_LAST)
		{
			return; //they have too many crafts already, so they can't learn this one
		}
		
		if (!str_cmp (craft->subcraft_name, ThisArgument.c_str()))
		{
			magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			af = get_affect (ch, i);
			af->a.craft = new affect_craft_type;
			af->a.craft->phase = NULL;
			af->a.craft->subcraft = NULL;
			af->a.craft->target_ch = NULL;
			af->a.craft->target_obj = NULL;
			af->a.craft->skill_check = 0;
			af->a.craft->timer = 0;
			
			af->a.craft->subcraft = craft;
			return;
		}
	}
	else 
	{
		return;
	}
	
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

// objvnum <room> <keyword or number> <variable>
void
r_objvnum (CHAR_DATA *ch, std::string argument, room_prog_var *& variable_list)
{
	OBJ_DATA *obj;
	ROOM_DATA *room;
	std::string buf;

	// Room first. -1 means actor's room.
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;
	if (buf.find("-1") != std::string::npos)
	{
		room = ch->room;
	}
	else
	{
		if (!atoi(buf.c_str()))
			return;

		room = vtor(atoi(buf.c_str()));
		if (!room)
			return;
	}

	// Work out if it's a keyword or a number. Numbers mean nth object in room, keyword means first keyword named object.
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;
	if (buf.find(".") != std::string::npos || !atoi(buf.c_str()))
	{
		obj = get_obj_in_list ((char *) buf.c_str(), room->contents);
	}
	else
	{
		if (atoi(buf.c_str()) < 0)
			return;
		int j = atoi(buf.c_str());
		obj = room->contents;
		for (int i = 1; i != j; i++, obj = obj->next_content)
			;
	}
	if (!obj)
		return;

	// Work out what variable to save to.
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;

	if (is_variable_in_list(variable_list, buf))
		set_variable_data(variable_list, buf, MAKE_STRING(obj->nVirtual));
	else
		add_variable_to_list(variable_list, buf, MAKE_STRING(obj->nVirtual));

	return;
}

// randomobj <room> <variable>
void
r_randomobj (CHAR_DATA *ch, std::string argument, room_prog_var *& variable_list)
{
	OBJ_DATA *obj;
	ROOM_DATA *room;
	std::string buf;

	// Room first. -1 means actor's room.
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;
	if (buf.find("-1") != std::string::npos)
	{
		room = ch->room;
	}
	else
	{
		if (!atoi(buf.c_str()))
			return;

		room = vtor(atoi(buf.c_str()));
		if (!room)
			return;
	}

	// Get a random object in the room

	int count = 0;
	for (obj = room->contents; obj; obj = obj->next_content)
	{
		count++;
	}

	// Grab a random number in the list to use
	int rnumber = number(1, count);

	// Find the nth object
	for (count = 1, obj = room->contents; obj; count++, obj = obj->next_content)
	{
		if (count == rnumber)
			break;
	}

	if (!obj)
		return;

	// Work out what variable to save to.
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;

	if (is_variable_in_list(variable_list, buf))
		set_variable_data(variable_list, buf, MAKE_STRING(obj->nVirtual));
	else
		add_variable_to_list(variable_list, buf, MAKE_STRING(obj->nVirtual));

	return;
}

void
r_mftog (CHAR_DATA *ch, std::string argument)
{
	std::string buf;
	CHAR_DATA *tch;
	argument = one_argument (argument, buf);
	if (!buf.compare("-1"))
	{
		tch = ch;
	}
	else if (is_number(buf.c_str()))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (!IS_NPC(tch))
				continue;

			if (tch->mob->nVirtual == atoi(buf.c_str()))
				break;
		}
	}
	else
	{
		tch = get_char_room ((char *) buf.c_str(), ch->in_room);
	}
	if (!tch)
		return;

	if (!IS_NPC(tch))
		return;

	argument = one_argument(argument, buf);
	int flag;
	if ((flag = index_lookup(action_bits, (char *) buf.c_str())) == -1)
	{
		return;
	}

	if (!IS_SET(tch->act, (1 << flag)))
		tch->act |= (1 << flag);
	else
		tch->act &= ~(1 << flag);

	return;
}

char * room__get_description (ROOM_DATA * room);
char *exits[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"outside",
	"inside"
};

bool
prog_object_properties (OBJ_DATA *obj, std::string argument, std::string& output)
{
	std::string buf;
	argument = one_argument(argument, buf);
	if (buf.empty())
		return false;

	if (buf.find("name") != std::string::npos)
	{
		std::string objname = obj->name;
		one_argument (objname, output);
	}
	if (buf.find("keywords") != std::string::npos)
	{
		output = obj->name;
	}
	else if (buf.find("vnum") != std::string::npos)
	{
		output = MAKE_STRING(obj->nVirtual);
	}
	else if (buf.find("sdesc") != std::string::npos)
	{
		output = obj_short_desc(obj);
	}
	else if (buf.find("ldesc") != std::string::npos)
	{
		output = obj->description;
	}
	else if (buf.find("desc") != std::string::npos)
	{
		output = obj->full_description;
	}
	else if (buf.find("type") != std::string::npos)
	{
		output = item_types[(int) obj->obj_flags.type_flag];
	}
	else if (buf.find("material") != std::string::npos)
	{
		extern std::map<int, std::string> material_names;
		std::map<int, std::string>::iterator it;
		it = material_names.find(obj->material);
		if (it != material_names.end())
			output = it->second;
	}
	else if (buf.find("weight") != std::string::npos)
	{
		output = MAKE_STRING(obj_mass(obj));
	}
	else if (buf.find("cost") != std::string::npos)
	{
		output = MAKE_STRING((obj->obj_flags.set_cost ? obj->obj_flags.set_cost/100 : obj->farthings));
	}
	else if (buf.find("quality") != std::string::npos)
	{
		output = MAKE_STRING(obj->quality);
	}
	else if (buf.find("count") != std::string::npos)
	{
		if (obj->count > 1)
			output = MAKE_STRING(obj->count);
		else
			output = "1";
	}
	else if (buf.find("color") != std::string::npos)
	{
		if (obj->var_color)
			output = obj->var_color;
	}
	else if (buf.find("ink") != std::string::npos)
	{
		if (obj->ink_color)
			output = obj->ink_color;
	}
	else if (buf.find("mkey") != std::string::npos)
	{
		if (obj->desc_keys)
			output = obj->desc_keys;
	}
	else if (buf.find("months") != std::string::npos)
	{
		int morphtime = obj->morphTime - time(0);
		morphtime /= 3600;
		output = MAKE_STRING(morphtime / (24*30));
	}
	else if (buf.find("days") != std::string::npos)
	{
		int delta, days;

		delta = obj->morphTime - time (0);
		days = delta / 86400;
		days %= 30;
		output = MAKE_STRING(days);
	}
	else if (buf.find("hours") != std::string::npos)
	{
		int delta, days, hours;
		delta = obj->morphTime - time (0);
		days = delta / 86400;
		delta -= days * 86400;
		hours = delta / 3600;
		output = MAKE_STRING(hours);
	}
	else if (buf.find("minutes") != std::string::npos)
	{
		int delta, days, hours, minutes;
		delta = obj->morphTime - time (0);
		days = delta / 86400;
		delta -= days * 86400;
		hours = delta / 3600;
		delta -= hours * 3600;
		minutes = delta / 60;
		output = MAKE_STRING(minutes);
	}
	else if (buf.find("oval") != std::string::npos)
	{
		switch (buf[4])
		{
		case '0':
			output = MAKE_STRING(obj->o.od.value[0]);
			break;
		case '1':
			output = MAKE_STRING(obj->o.od.value[1]);
			break;
		case '2':
			output = MAKE_STRING(obj->o.od.value[2]);
			break;
		case '3':
			output = MAKE_STRING(obj->o.od.value[3]);
			break;
		case '4':
			output = MAKE_STRING(obj->o.od.value[4]);
			break;
		case '5':
			output = MAKE_STRING(obj->o.od.value[5]);
			break;
		}
	}

	if (output.empty())
		return false;

	return true;
}

void
prog_object_properties (OBJ_DATA *obj, std::string argument)     // Overloaded versions sets rather than reads
{
	std::string buf;
	argument = one_argument(argument, buf);
	if (buf.empty())
		return;

	if (buf.find("name") != std::string::npos)
	{
		if (MAKE_STRING(obj->name).find(argument) != std::string::npos)
		{
			std::string buf2 = obj->name;
			buf2.erase(buf2.find(argument), argument.length());
			buf2 = argument + buf2;
			free_mem(obj->name);
			obj->name = duplicateString(buf2.c_str());
		}
		else
		{
			std::string buf2 = obj->name;
			free_mem(obj->name);
			obj->name = duplicateString((argument + buf2).c_str());
		}
	}
	else if (buf.find("sdesc") != std::string::npos)
	{
		free_mem(obj->short_description);
		obj->short_description = duplicateString(argument.c_str());
	}
	else if (buf.find("ldesc") != std::string::npos)
	{
		free_mem(obj->description);
		obj->description = duplicateString(argument.c_str());
	}
	else if (buf.find("desc") != std::string::npos)
	{
		if(argument.substr(0, 8).compare("reformat") == 0)
		{
				reformat_desc (obj->full_description, &obj->full_description);
		}
		else
		{
			free_mem(obj->full_description);
			obj->full_description = duplicateString(argument.c_str());
		}
		
	}
	else if (buf.find("material") != std::string::npos)
	{
		extern std::map<int, std::string> material_names;
		for (std::map<int, std::string>::iterator it = material_names.begin(); it != material_names.end(); it++)
		{
			if (it->second.find(argument) != std::string::npos)
			{
				obj->material = it->first;
				break;
			}
		}
	}
	else if (buf.find("weight") != std::string::npos)
	{
		obj->obj_flags.weight = atoi(argument.c_str());
	}
	else if (buf.find("cost") != std::string::npos)
	{
		obj->obj_flags.set_cost = atol(argument.c_str()) * 100;
	}
	else if (buf.find("quality") != std::string::npos)
	{
		obj->quality = atoi(argument.c_str());
	}
	else if (buf.find("count") != std::string::npos)
	{
		obj->count = atoi(argument.c_str());
	}
	else if (buf.find("mkey") != std::string::npos)
	{
		free_mem(obj->desc_keys);
		obj->desc_keys = duplicateString(argument.c_str());
	}
	else if (buf.find("timer") != std::string::npos)
	{
		std::string months, days, hours, minutes;
		argument = one_argument(argument, months);
		argument = one_argument(argument, days);
		argument = one_argument(argument, hours);
		argument = one_argument(argument, minutes);
		if (months.empty() || days.empty() || hours.empty())
			return;
		int clock = 0;
		clock = atoi(months.c_str()) * 24 * 30 + atoi(days.c_str()) * 24 + atoi(hours.c_str());
		clock *= 3600;
		if (!minutes.empty())
			clock += (atoi(minutes.c_str())*60);
		obj->morphTime = time (0) + clock;
	}
	else if (buf.find("oval") != std::string::npos)
	{
		switch (buf[4])
		{
		case '0':
			obj->o.od.value[0] = atoi(argument.c_str());
			break;
		case '1':
			obj->o.od.value[1] = atoi(argument.c_str());
			break;
		case '2':
			obj->o.od.value[2] = atoi(argument.c_str());
			break;
		case '3':
			obj->o.od.value[3] = atoi(argument.c_str());
			break;
		case '4':
			obj->o.od.value[4] = atoi(argument.c_str());
			break;
		case '5':
			obj->o.od.value[5] = atoi(argument.c_str());
			break;
		}
	}
	else if (buf.find("morphto") != std::string::npos )
	{
		obj->morphto = atoi(argument.c_str());
	}
	else if (buf.find("clock") != std::string::npos )
	{
		int month, day, hour;

		if (!isdigit (argument[0]))
			return;

		month = atoi (argument.c_str());
		argument = one_argument(argument, buf);

		if (!isdigit (argument[0]))
			return;

		day = atoi (argument.c_str());

		argument = one_argument (argument, buf);

		if (!isdigit (argument[0]))
			return;

		hour = atoi (argument.c_str());

		obj->clock = ((month * 30 * 24) + (day * 24) + hour);
	}

	return;
}

// info <variable> obj/char/room target property
void
r_info (CHAR_DATA *ch, std::string argument, room_prog_var *& variable_list)
{
	std::string buf, variable;
	std::string value; // What we're setting the variable to be
	argument = one_argument (argument, buf);

	if (buf.empty())
		return;
	else
		variable = buf;

	argument = one_argument (argument, buf);
	if (buf.find("obj") != std::string::npos)
	{
		OBJ_DATA *obj = NULL;
		// Ascertain what object they want to target. With this command it is done using a vnum
		argument = one_argument (argument, buf);
		if (buf.empty())
			return;

		if (atoi(buf.c_str()) < 1)
			return;

		obj = vtoo(atoi(buf.c_str()));

		if (!obj)
			return;

		// what property?
		argument = one_argument (argument, buf);
		if (buf.empty())
			return;

		if (buf.find("vnum") != std::string::npos)
		{
			value = MAKE_STRING(obj->nVirtual);
		}
		if (buf.find("name") != std::string::npos)
		{
			std::string objname = obj->name;
			one_argument(objname, value);
		}
		else if (buf.find("sdesc") != std::string::npos)
		{
			value = obj_short_desc(obj);
		}
		else if (buf.find("ldesc") != std::string::npos)
		{
			value = obj->description;
		}
		else if (buf.find("desc") != std::string::npos)
		{
			value = obj->full_description;
		}
		else if (buf.find("weight") != std::string::npos)
		{
			value = MAKE_STRING(obj_mass(obj));
		}
		else if (buf.find("cost") != std::string::npos)
		{
			value = MAKE_STRING(obj->farthings);
		}
		else if (buf.find("quality") != std::string::npos)
		{
			value = MAKE_STRING(obj->quality);
		}
		else if (buf.find("color") != std::string::npos)
		{
			if (obj->var_color)
				value = obj->var_color;
		}
		else if (buf.find("ink") != std::string::npos)
		{
			if (obj->ink_color)
				value = obj->ink_color;
		}
		else if (buf.find("mkey") != std::string::npos)
		{
			if (obj->desc_keys)
				value = obj->desc_keys;
		}
		else if (buf.find("oval") != std::string::npos)
		{
			switch (buf[4])
			{
			case '0':
				value = MAKE_STRING(obj->o.od.value[0]);
				break;
			case '1':
				value = MAKE_STRING(obj->o.od.value[1]);
				break;
			case '2':
				value = MAKE_STRING(obj->o.od.value[2]);
				break;
			case '3':
				value = MAKE_STRING(obj->o.od.value[3]);
				break;
			case '4':
				value = MAKE_STRING(obj->o.od.value[4]);
				break;
			case '5':
				value = MAKE_STRING(obj->o.od.value[5]);
				break;
			}
		}
	}
	else if (buf.find("room") != std::string::npos)
	{
		OBJ_DATA *obj = NULL;
		ROOM_DATA *room = NULL;
		CHAR_DATA *tch = NULL;
		argument = one_argument(argument, buf);

		if (buf.empty())
			return;

		// Argument is a room vnum
		if (buf.find("-1") != std::string::npos)
		{
			room = ch->room;
		}
		else if (atoi(buf.c_str()) > 0)
		{
			room = vtor(atoi(buf.c_str()));
		}
		else
			return;

		if (!room)
			return;

		// What about the room do they want to know?
		argument = one_argument (argument, buf);
		if (buf.empty())
			return;

		if (buf.find("vnum") != std::string::npos)
		{
			value = MAKE_STRING(room->nVirtual);
		}
		if (buf.find("name") != std::string::npos)
		{
			value = room->name;
		}
		else if (buf.find("desc") != std::string::npos)
		{
			value = room__get_description(room);  // This function handles "weather room" descriptions so that case is covered
		}
		else if (buf.find("sector") != std::string::npos)
		{
			value = sector_types[room->sector_type];
		}
		else if (buf.find("exit") != std::string::npos)    // Get the vnum of the room that exit has requested
		{
			argument = one_argument(argument, buf);
			if (buf.empty())
				return;

			int exit_requested;
			if ((exit_requested = index_lookup (exits, buf.c_str())) != -1)
			{
				if (!(room->dir_option[exit_requested]))
					return;

				value = MAKE_STRING(room->dir_option[exit_requested]->to_room);
			}
			else
			{
				return;
			}
		}
		else if (buf.find("char") != std::string::npos)    // Another option is that you want a character in that room
		{
			argument = one_argument (argument, buf);
			if (buf.empty())
				return;

			if (buf.find("-1") != std::string::npos)        // Actor
			{
				tch = ch;
			}
			else if (atoi(buf.c_str()) > 0)            // Have a vnum
			{
				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					if (IS_NPC(tch) && tch->mob->nVirtual == atoi(buf.c_str()))
						break;
				}
			}
			else				// Assumedly a keyword
			{
				tch = get_char_room ((char *) buf.c_str(), room->nVirtual);
			}

			if (!tch)
				return;

			argument = one_argument (argument, buf);

			//    We now know who we're targetting, work out what property we want
			if (buf.find("vnum") != std::string::npos)
			{
				if (!IS_NPC(tch))
					value = "-1";
				else
					value = MAKE_STRING(tch->mob->nVirtual);
			}
			else if (buf.find("race") != std::string::npos)
			{
				value = lookup_race_variable (tch->race, RACE_NAME);
			}
			else if (buf.find("name") != std::string::npos)
			{
				if (tch->tname != '\0')
				{
					value = tch->tname;
				}
				else
				{
					std::string tchname = tch->name;
					one_argument(tchname, value);
				}
			}
			else if (buf.find("sdesc") != std::string::npos)
			{
				value = char_short(tch);
			}
			else if (buf.find("ldesc") != std::string::npos)
			{
				value = tch->long_descr;
			}
			else if (buf.find("desc") != std::string::npos)
			{
				value = tch->description;
			}
			else if (buf.find("height") != std::string::npos)
			{
				value = MAKE_STRING(tch->height);
			}
			else if (buf.find("weight") != std::string::npos)
			{
				value = MAKE_STRING(get_weight(tch) / 100);
			}
			else if (buf.find("str") != std::string::npos)
			{
				value = MAKE_STRING(GET_STR(tch));
			}
			else if (buf.find("dex") != std::string::npos)
			{
				value = MAKE_STRING(GET_DEX(tch));
			}
			else if (buf.find("agi") != std::string::npos)
			{
				value = MAKE_STRING(GET_AGI(tch));
			}
			else if (buf.find("wil") != std::string::npos)
			{
				value = MAKE_STRING(GET_WIL(tch));
			}
			else if (buf.find("int") != std::string::npos)
			{
				value = MAKE_STRING(GET_INT(tch));
			}
			else if (buf.find("aur") != std::string::npos)
			{
				value = MAKE_STRING(GET_AUR(tch));
			}
			else if (buf.find("con") != std::string::npos)
			{
				value = MAKE_STRING(GET_CON(tch));
			}
			else if (buf.find("skill") != std::string::npos)	// skill <skillname>
			{
				argument = one_argument(argument, buf);
				if (buf.empty())
					return;
				int index;
				if ((index = index_lookup(skills, buf.c_str())) != -1)
				{
					value = MAKE_STRING(tch->skills[index]);
				}
				else
					return;
			}
			else if (buf.find("sex") != std::string::npos)
			{
				switch (tch->sex)
				{
				case 0:
					value = "genderless";
					break;
				case 1:
					value = "male";
					break;
				case 2:
					value = "female";
					break;
				}
			}
			else if (buf.find("inv") != std::string::npos || buf.find("worn") != std::string::npos)
			{
				if (buf.find("inv") != std::string::npos)	// Target object by keyword, or vnum, in rhand, lhand and equip
				{
					argument = one_argument (argument, buf);
					if (buf.empty())
						return;

					if (is_number(buf.c_str()) && atoi(buf.c_str()) > 0)    // Obj vnum
					{
						if (tch->right_hand && tch->right_hand->nVirtual == atoi(buf.c_str()))
							obj = tch->right_hand;
						else if (tch->left_hand && tch->left_hand->nVirtual == atoi(buf.c_str()))
							obj = tch->left_hand;
						else if (tch->equip)
						{
							for (obj = tch->equip; obj; obj = obj->next_content)
							{
								if (obj->nVirtual == atoi(buf.c_str()))
									break;
							}
						}
					}
					else				// Obj keyword
					{
						if (tch->right_hand && get_obj_in_list ((char *) buf.c_str(), tch->right_hand))
							obj = tch->right_hand;
						else if (tch->left_hand && get_obj_in_list ((char *) buf.c_str(), tch->left_hand))
							obj = tch->left_hand;
						else if (tch->equip)
							obj = get_obj_in_list ((char *) buf.c_str(), tch->equip);
					}

					if (!obj)
						return;

					if (!prog_object_properties (obj, argument, value))
						return;
				}
				else 		// Target object by worn location (worn <location>)
				{
					// There is actually no good way to target a specific location in the existing code, as all "wordy" location names
					// are general - i.e. although there is a right shoulder and left shoulder, you can only target shoulder and it will
					// choose for you. In this case that's inappropriate, and I don't want the builder to need to know any numbers.
					// Hence the mapWearByName container was created to store this information.

					extern std::map<std::string, int> mapWearByName;
					std::map<std::string, int>::iterator it;
					argument = one_argument(argument, buf);
					if (buf.empty())
						return;
					if (buf.find("rhand") != std::string::npos)
					{
						obj = tch->right_hand;
					}
					else if (buf.find("lhand") != std::string::npos)
					{
						obj = tch->left_hand;
					}
					else
					{
						it = mapWearByName.find(buf);
						if (it != mapWearByName.end())
							obj = get_equip(tch, it->second);
					}
					if (!obj)
						return;

					if (!prog_object_properties (obj, argument, value))
						return;
				}
			}
		}
		else if (buf.find("obj") != std::string::npos)
		{
			argument = one_argument(argument, buf);
			if (buf.empty())
				return;

			if (is_number(buf.c_str()) && atoi(buf.c_str()) > 0)
			{
				obj = get_obj_in_list_num(atoi(buf.c_str()), room->contents);
			}
			else
			{
				obj = get_obj_in_list((char *) buf.c_str(), room->contents);
			}

			if (!obj)
				return;

			if (!prog_object_properties (obj, argument, value))
				return;
		}
	}
	else if (buf.find("group_members") != std::string::npos)
	{
		value = MAKE_STRING (num_followers (ch) + 1);
	}

	if (value.empty())
		return;

	if (is_variable_in_list(variable_list, variable))
		set_variable_data(variable_list, variable, value);
	else
		add_variable_to_list(variable_list, variable, value);

	return;
}

// setval obj/char/room target property <value>
void
r_setval (CHAR_DATA *ch, std::string argument)
{
	std::string buf;
	std::string value; // What we're setting the variable to be

	argument = one_argument (argument, buf);

	if (buf.find("room") != std::string::npos)
	{
		OBJ_DATA *obj = NULL;
		ROOM_DATA *room = NULL;
		CHAR_DATA *tch = NULL;
		argument = one_argument(argument, buf);

		if (buf.empty())
			return;

		// Argument is a room vnum
		if (buf.find("-1") != std::string::npos)
		{
			room = ch->room;
		}
		else if (atoi(buf.c_str()) > 0)
		{
			room = vtor(atoi(buf.c_str()));
		}
		else
			return;

		if (!room)
			return;

		// What about the room do they want to know?
		argument = one_argument (argument, buf);
		if (buf.empty())
			return;

		if (buf.find("name") != std::string::npos)
		{
			free_mem(room->name);
			room->name = duplicateString(argument.c_str());
		}
		else if (buf.find("desc") != std::string::npos)
		{
			if(argument.substr(0, 8).compare("reformat") == 0)
			{
				reformat_desc (room->description, &room->description);
			}
			else
			{
				free_mem(room->description);
				room->description = duplicateString(argument.c_str());
			}
		}
		else if (buf.find("sector") != std::string::npos)
		{
			int ind = 0;
			if ((ind = index_lookup (sector_types, argument.c_str())) != -1)
				room->sector_type = ind;
		}
		else if (buf.find("char") != std::string::npos)    // Another option is that you want a character in that room
		{
			argument = one_argument (argument, buf);
			if (buf.empty())
				return;

			if (buf.find("-1") != std::string::npos)        // Actor
			{
				tch = ch;
			}
			else if (atoi(buf.c_str()) > 0)            // Have a vnum
			{
				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					if (IS_NPC(tch) && tch->mob->nVirtual == atoi(buf.c_str()))
						break;
				}
			}
			else				// Assumedly a keyword
			{
				tch = get_char_room ((char *) buf.c_str(), room->nVirtual);
			}

			if (!tch)
				return;

			argument = one_argument (argument, buf);

			//    We now know who we're targetting, work out what property we want
			if (buf.find("name") != std::string::npos)
			{
				free_mem(tch->tname);
				tch->tname = duplicateString (argument.c_str());
			}
			else if (buf.find("sdesc") != std::string::npos)
			{
				free_mem(tch->short_descr);
				tch->short_descr = duplicateString (argument.c_str());
			}
			else if (buf.find("ldesc") != std::string::npos)
			{
					free_mem(tch->long_descr);
					tch->long_descr = duplicateString (argument.c_str());			
			}
			else if (buf.find("desc") != std::string::npos)
			{
				if(argument.substr(0, 8).compare("reformat") == 0)
				{
					reformat_desc (tch->description, &tch->description);
				}
				else
				{
					free_mem(tch->description);
					tch->description = duplicateString (argument.c_str());	
				}
			}
			else if (buf.find("height") != std::string::npos)
			{
				tch->height = atoi(argument.c_str());
			}
			else if (buf.find("bmi") != std::string::npos)
			{
				tch->bmi = atoi(argument.c_str());
			}
			else if (buf.find("str") != std::string::npos)
			{
				GET_STR(tch) = atoi(argument.c_str());
			}
			else if (buf.find("dex") != std::string::npos)
			{
				GET_DEX(tch) = atoi(argument.c_str());
			}
			else if (buf.find("agi") != std::string::npos)
			{
				GET_AGI(tch) = atoi(argument.c_str());
			}
			else if (buf.find("wil") != std::string::npos)
			{
				GET_WIL(tch) = atoi(argument.c_str());
			}
			else if (buf.find("int") != std::string::npos)
			{
				GET_INT(tch) = atoi(argument.c_str());
			}
			else if (buf.find("aur") != std::string::npos)
			{
				GET_AUR(tch) = atoi(argument.c_str());
			}
			else if (buf.find("con") != std::string::npos)
			{
				GET_CON(tch) = atoi(argument.c_str());
			}
			else if (buf.find("skill") != std::string::npos)	// skill <skillname>
			{
				argument = one_argument(argument, buf);
				if (buf.empty())
					return;
				int index;
				if ((index = index_lookup(skills, buf.c_str())) != -1)
				{
					tch->skills[index] = atoi(argument.c_str());
				}
				else
					return;
			}
			else if (buf.find("sex") != std::string::npos)
			{
				if (argument.find("genderless") != std::string::npos)
					tch->sex = 0;
				else if (argument.find("male") != std::string::npos)
					tch->sex = 1;
				else if (argument.find("female") != std::string::npos)
					tch->sex = 2;
			}
			else if (buf.find("inv") != std::string::npos || buf.find("worn") != std::string::npos)
			{
				if (buf.find("inv") != std::string::npos)	// Target object by keyword, or vnum, in rhand, lhand and equip
				{
					argument = one_argument (argument, buf);
					if (buf.empty())
						return;

					if (atoi(buf.c_str()) > 0)    // Obj vnum
					{
						if (tch->right_hand && tch->right_hand->nVirtual == atoi(buf.c_str()))
							obj = tch->right_hand;
						else if (tch->left_hand && tch->left_hand->nVirtual == atoi(buf.c_str()))
							obj = tch->left_hand;
						else if (tch->equip)
						{
							for (obj = tch->equip; obj; obj = obj->next_content)
							{
								if (obj->nVirtual == atoi(buf.c_str()))
									break;
							}
						}
					}
					else				// Obj keyword
					{
						if (tch->right_hand && get_obj_in_list ((char *) buf.c_str(), tch->right_hand))
							obj = tch->right_hand;
						else if (tch->left_hand && get_obj_in_list ((char *) buf.c_str(), tch->left_hand))
							obj = tch->left_hand;
						else if (tch->equip)
							obj = get_obj_in_list ((char *) buf.c_str(), tch->equip);
					}

					if (!obj)
						return;

					prog_object_properties (obj, argument);
				}
				else 		// Target object by worn location (worn <location>)
				{
					// There is actually no good way to target a specific location in the existing code, as all "wordy" location names
					// are general - i.e. although there is a right shoulder and left shoulder, you can only target shoulder and it will
					// choose for you. In this case that's inappropriate, and I don't want the builder to need to know any numbers.
					// Hence the mapWearByName container was created to store this information.

					extern std::map<std::string, int> mapWearByName;
					std::map<std::string, int>::iterator it;
					argument = one_argument(argument, buf);
					if (buf.empty())
						return;
					it = mapWearByName.find(buf);
					if (it != mapWearByName.end())
						obj = get_equip(tch, it->second);

					if (!obj)
						return;

					prog_object_properties (obj, argument);
				}
			}
		}
		else if (buf.find("obj") != std::string::npos)
		{
			argument = one_argument(argument, buf);
			if (buf.empty())
				return;

			if (atoi(buf.c_str()) > 0)
			{
				obj = get_obj_in_list_num(atoi(buf.c_str()), room->contents);
			}
			else
			{
				obj = get_obj_in_list((char *) buf.c_str(), room->contents);
			}

			if (!obj)
				return;

			prog_object_properties (obj, argument);
		}
	}
	return;
}

std::string alternate_money_description (int money);

void
r_coppertoshillings (std::string argument, room_prog_var *& variable_list)
{
	std::string variable;
	argument = one_argument(argument, variable);
	if (variable.empty() || argument.empty() || !is_number(argument))
		return;

	std::string value = alternate_money_description(atoi(argument.c_str()));
	if (is_variable_in_list(variable_list, variable))
		set_variable_data(variable_list, variable, value);
	else
		add_variable_to_list(variable_list, variable, value);
}

void
r_concat (CHAR_DATA *ch, std::string argument, room_prog_var *& variable_list)
{
	std::string variable;
	argument = one_argument(argument, variable);

	if (variable.empty() || argument.empty())
		return;

	if (is_variable_in_list(variable_list, variable))
		set_variable_data(variable_list, variable, get_variable_data(variable_list, variable) + argument);
	else
		add_variable_to_list(variable_list, variable, get_variable_data(variable_list, variable) + argument);
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

		double new_value = 0;
		if (!operation.compare("invert"))
		{
			new_value = atof(variable.c_str()) * -1;
		}
		else if (!operation.compare("abs"))
		{
			new_value = (double) abs(atoi(variable.c_str()));
		}
		else if (!operation.compare("int"))
		{
			new_value = (double) (atoi(variable.c_str()));
		}
		else if (!operation.compare("round"))
		{
			if (buffer.empty() || !is_number(buffer.c_str()))
			{
				new_value = (double) (floor(atof(variable.c_str())+0.5));
			}
			else
			{
				new_value = RoundDouble(atof(variable.c_str()), atoi(buffer.c_str()));
			}
		}
		else
		{

			if (buffer.empty() || !is_number(buffer.c_str()))
				return;

			if (!operation.compare("add"))
			{
				new_value = atof(variable.c_str()) + atof(buffer.c_str());
			}
			else if (!operation.compare("sub"))
			{
				new_value = atof(variable.c_str()) - atof(buffer.c_str());
			}
			else if (!operation.compare("multiply"))
			{
				new_value = atof(variable.c_str()) * atof(buffer.c_str());
			}
			else if (!operation.compare("divide"))
			{
				if (atof(buffer.c_str()) != 0)
					new_value = atof(variable.c_str()) / atof(buffer.c_str());
			}
			else if (!operation.compare("power"))
			{
				new_value = pow(atof(variable.c_str()), atof(buffer.c_str()));
			}
			else if (!operation.compare("sqrt"))
			{
				new_value = sqrt(atof(variable.c_str()));
			}
			else if (!operation.compare("mod"))
			{
				new_value = (double) (atoi(variable.c_str()) % atoi(buffer.c_str()));
			}
		}
		set_variable_data(variable_list, variable_name, MAKE_STRING(new_value));
		return;
	}
	else
		return;
}
