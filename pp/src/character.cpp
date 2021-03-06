/*------------------------------------------------------------------------\
|  character.cpp : Character Module                   www.middle-earth.us |
|  Copyright (C) 2008, Shadows of Isildur: Japheth and Case               |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <string>
#include <sstream>

#include "server.h"

#include "account.h"
#include "character.h"
#include "protos.h"
#include "decl.h"
#include "group.h"
#include "structs.h"
#include "utils.h"
#include "utility.h"

extern rpie::server engine;

CHAR_DATA *
new_char (int pc_type)
{
	CHAR_DATA *ch;

	ch = new char_data();

	if (pc_type)
	{
		ch->pc = new pc_data();
	}
	else
	{
		ch->mob = new mob_data();
	}
	return ch;
}

void
free_char (CHAR_DATA *&ch)
{
	delete ch;
	ch = NULL;
}

void
char_data::clear_char ()
{
	this->in_room = 0;
	this->room = NULL;
	this->deleted = 0;
	this->circle = 0;
	this->fight_mode = 0;
	this->debug_mode = 0;
	this->primary_delay = 0;
	this->secondary_delay = 0;
	this->coldload_id = 0;
	this->natural_delay = 0;
	this->body_type = 0;
	this->nat_attack_type = 0;
	this->flags = 0;
	this->move_points = 0;
	this->hit_points = 0;
	this->speaks = 0;
	this->alarm = 0;
	this->trigger_delay = 0;
	this->trigger_line = 0;
	this->trigger_id = 0;
	this->psionic_talents = 0;
	this->subdue = NULL;
	this->prog = NULL;
	this->vartab = NULL;
	this->shop = NULL;
	this->vehicle = NULL;
	this->str = 0;
	this->intel = 0;
	this->wil = 0;
	this->dex = 0;
	this->con = 0;
	this->aur = 0;
	this->agi = 0;
	this->tmp_str = 0;
	this->tmp_intel = 0;
	this->tmp_wil = 0;
	this->tmp_dex = 0;
	this->tmp_con = 0;
	this->tmp_aur = 0;
	this->tmp_agi = 0;
	for (int i = 0; i < MAX_SKILLS; i++) {
		this->skills[i] = 0;
	}
	this->hour_affects = NULL;
	this->equip = NULL;
	this->desc = NULL;
	this->next_in_room = NULL;
	this->next = NULL;
	this->next_fighting = NULL;
	this->next_assist = NULL;
	this->assist_pos = 0;
	this->following = NULL;
	this->pc = NULL;
	this->mob = NULL;
	this->moves = NULL;
	this->casting_arg = NULL;
	this->hit = 0;
	this->max_hit = 0;
	this->move = 0;
	this->max_move = 0;
	this->armor = 0;
	this->offense = 0;
	this->ppoints = 0;
	this->fighting = NULL;
	this->distance_to_target = 0;
	this->remembers = NULL;
	this->affected_by = 0;
	this->position = 8;
	this->default_pos = 8;
	this->act = 0;
	this->hmflags = 0;
	this->carry_weight = 0;
	this->carry_items = 0;
	this->delay_type = 0;
	this->delay = 0;
	this->delay_who = NULL;
	this->delay_who2 = NULL;
	this->delay_ch = NULL;
	this->delay_obj = NULL;
	this->delay_info1 = 0;
	this->delay_info2 = 0;
	this->delay_info3 = 0;
	this->was_in_room = 0;
	this->intoxication = 0;
	this->hunger = 0;
	this->thirst = 0;
	this->last_room = 0;
	this->attack_type = 0;
	this->name = NULL;
	this->tname = NULL;
	this->short_descr = NULL;
	this->long_descr = NULL;
	this->pmote_str = NULL;
	this->voice_str = NULL;
	this->description = NULL;
	this->sex = 0;
	this->deity = 0;
	this->race = 0;
	this->color = 0;
	this->speed = 0;
	this->age = 0;
	this->height = 0;
	this->frame = 0;
	this->time.birth = 0;
	this->time.logon = 0;
	this->time.played = 0;
	this->clans = NULL;
	this->mount = NULL;
	this->hitcher = NULL;
	this->hitchee = NULL;
	this->combat_log = NULL;
	this->wounds = NULL;
	this->damage = 0;
	this->lastregen = 0;
	this->defensive = 0;
	this->cell_1 = 0;
	this->cell_2 = 0;
	this->cell_3 = 0;
	this->laststuncheck = 0;
	this->knockedout = 0;
	this->writes = 0;
	this->stun = 0;
	this->curse = 0;
	this->aiming_at = NULL;
	this->targeted_by = NULL;
	this->aim = 0;
	this->lodged = NULL;
	this->mana = 0;
	this->max_mana = 0;
	this->harness = 0;
	this->max_harness = 0;
	this->preparing_id = 0;
	this->preparing_time = 0;
	this->enchantments = NULL;
	this->roundtime = 0;
	this->right_hand = NULL;
	this->left_hand = NULL;
	this->plr_flags = 0;
	this->ranged_enemy = NULL;
	this->enemy_direction = NULL;
	this->threats = NULL;
	this->attackers = NULL;
	this->whirling = 0;
	this->from_dir = 0;
	this->sighted = NULL;
	this->balance = 0;
	this->travel_str = NULL;
	for (int i = 0; i < MAX_LEARNED_SPELLS; i++) {
		this->spells[i][0] = 0;
		this->spells[i][1] = 0;
	}
	this->body_proto = 0;
	this->bmi = 0;
	this->size = 0;
	this->guardian_mode = 0;
	this->hire_storeroom = 0;
	this->hire_storeobj = 0;
	this->dmote_str = NULL;
	this->cover_from_dir = 0;
	this->morph_type = 0;
	this->clock = 0;
	this->morph_time = 0;
	this->morphto = 0;
	this->craft_index = 0;
	for (int i = 0; i < 100; i++) {
		this->enforcement[i] = 0;
	}
	this->bleeding_prompt = false;
	this->fight_percentage = 0;
	this->plan = NULL;
	this->goal = NULL;
	this->naughtyFlag = false;
	this->RPFlag = false;
	this->plotFlag = false;
	this->wrapLength = 80;
	this->petition_flags = 0;
}

char_data::char_data ()
{
	this->clear_char();
}

char_data::~char_data ()
{
	VAR_DATA *var;
	struct memory_data *mem;
	if (this->pc)
	{
		delete this->pc;
	}

	if (this->mob)
	{
		delete this->mob;
	}

	if (this->clans && *this->clans)
	{
		free_mem (this->clans);
		this->clans = NULL;
	}

	if (this->combat_log && *this->combat_log)
	{
		free_mem (this->combat_log);
		this->combat_log = NULL;
	}

	if (this->enemy_direction && *this->enemy_direction)
	{
		free_mem (this->enemy_direction);
		this->enemy_direction = NULL;
	}

	if (this->delay_who && !isdigit (*this->delay_who) && *this->delay_who)
	{
		free_mem (this->delay_who);
		this->delay_who = NULL;
	}

	if (this->delay_who2 && !isdigit (*this->delay_who2) && *this->delay_who2)
	{
		free_mem (this->delay_who2);
		this->delay_who2 = NULL;
	}

	if (this->voice_str && *this->voice_str)
	{
		free_mem (this->voice_str);
		this->voice_str = NULL;
	}

	if (this->travel_str && *this->travel_str)
	{
		free_mem (this->travel_str);
		this->travel_str = NULL;
	}

	if (this->pmote_str && *this->pmote_str)
	{
		free_mem (this->pmote_str);
		this->pmote_str = NULL;
	}

	if (this->plan)
	{
		delete this->plan;
		this->plan = 0;
	}
	if (this->goal)
	{
		delete this->goal;
		this->goal = 0;
	}

	while (this->vartab)
	{

		var = this->vartab;
		this->vartab = var->next;

		if (var->name && *var->name)
		{
			free_mem (var->name);
			var->name = NULL;
		}

		free_mem (var);
		var = NULL;
	}

	while (this->remembers)
	{
		mem = this->remembers;
		this->remembers = mem->next;
		free_mem (mem);
		mem = NULL;
	}

	while (this->wounds)
		wound_from_char (this, this->wounds);

	while (this->lodged)
		lodge_from_char (this, this->lodged);

	while (this->attackers)
		attacker_from_char (this, this->attackers);

	while (this->threats)
		threat_from_char (this, this->threats);

	while (this->hour_affects)
		affect_remove (this, this->hour_affects);

	clear_pmote(this);
	clear_dmote(this);

	if (this->mob)
	{
		CHAR_DATA* proto = vtom(this->mob->nVirtual);
		if (proto)
		{
			if (this->tname && *this->tname
				&& this->tname != proto->tname)
			{
				free_mem (this->tname);
				this->tname = NULL;
			}

			if (this->name && *this->name
				&& this->name != proto->name)
			{
				free_mem (this->name);
				this->name = NULL;
			}

			if (this->short_descr && *this->short_descr
				&& this->short_descr != proto->short_descr )
			{
				free_mem (this->short_descr);
				this->short_descr = NULL;
			}

			if (this->long_descr && *this->long_descr
				&& this->long_descr != proto->long_descr)
			{
				free_mem (this->long_descr);
				this->long_descr = NULL;
			}

			if (this->description && *this->description
				&& this->description != proto->description)
			{
				free_mem (this->description);
				this->description = NULL;
			}
		}
		else
		{
			fprintf (stderr, "Proto not defined for NPC %d?\n", this->mob->nVirtual);
		}
	}
	else
	{
		if (this->tname && *this->tname )
		{
			free_mem (this->tname);
			this->tname = NULL;
		}

		if (this->name && *this->name )
		{
			free_mem (this->name);
			this->name = NULL;
		}

		if (this->short_descr && *this->short_descr)
		{
			free_mem (this->short_descr);
			this->short_descr = NULL;
		}

		if (this->long_descr && *this->long_descr)
		{
			free_mem (this->long_descr);
			this->long_descr = NULL;
		}

		if (this->description && *this->description)
		{
			free_mem (this->description);
			this->description = NULL;
		}
	}
}

pc_data::pc_data()
{
	this->aliases = NULL;
	this->execute_alias = NULL;
	this->nanny_state = 0;
	this->role = 0;
	this->admin_loaded = false;
	this->creation_comment = NULL;
	this->imm_enter = NULL;
	this->imm_leave = NULL;
	this->site_lie = NULL;
	this->account_name = NULL;
	this->msg = NULL;
	this->edit_player = NULL;
	this->target_mob = NULL;
	this->dot_shorthand = NULL;
	this->dreams = NULL;
	this->dreamed = NULL;
	this->last_global_staff_msg = 0;
	this->staff_notes = 0;
	this->mortal_mode = 0;
	this->create_state = 0;
	this->edit_obj = 0;
	this->edit_mob = 0;
	this->load_count = 0;
	this->start_str = 0;
	this->start_dex = 0;
	this->start_con = 0;
	this->start_wil = 0;
	this->start_aur = 0;
	this->start_intel = 0;
	this->start_agi = 0;
	this->level = 0;
	this->boat_virtual = 0;
	this->mount_speed = 0;
	this->time_last_activity = 0;
	this->is_guide = 0;
	this->profession = 0;
	this->app_cost = 0;
	this->chargen_flags = 0;
	this->last_global_pc_msg = 0;
	this->sleep_needed = 0;
	this->auto_toll = 0;
	this->doc_type = 0;
	this->doc_index = -1;
	this->writing_on = NULL;
	this->special_role = NULL;
	for (int i = 0; i < MAX_SKILLS; i++) {
		this->skills[i] = 0;
	}
	this->owner = NULL;
	this->last_logon = 0;
	this->last_logoff = 0;
	this->last_disconnect = 0;
	this->last_connect = 0;
	this->last_died = 0;
	this->edit_craft = NULL;
}

pc_data::~pc_data()
{
	DREAM_DATA *dream;
	ALIAS_DATA *tmp_alias;
	ROLE_DATA *trole;

	while (this->aliases)
	{
		tmp_alias = this->aliases;
		this->aliases = this->aliases->next_alias;
		alias_free (tmp_alias);
	}
	while (this->dreams)
	{
		dream = this->dreams;

		this->dreams = this->dreams->next;

		if (dream->dream && *dream->dream)
		{
			free_mem (dream->dream);
			dream->dream = NULL;
		}

		free_mem (dream);
		dream = NULL;
	}
	while (this->dreamed)
	{
		dream = this->dreamed;
		this->dreamed = this->dreamed->next;
		if (dream->dream && *dream->dream)
		{
			free_mem (dream->dream);
			dream->dream = NULL;
		}
		free_mem (dream);
		dream = NULL;
	}

	if ((trole = this->special_role) != NULL)
	{
		if (trole->summary && *trole->summary)
			free_mem (trole->summary);
		if (trole->body && *trole->body)
			free_mem (trole->body);
		if (trole->date && *trole->date)
			free_mem (trole->date);
		if (trole->poster && *trole->poster)
			free_mem (trole->poster);
		free_mem (trole);
		this->special_role = NULL;
	}

	if (this->account_name && *this->account_name)
	{
		free_mem (this->account_name);
		this->account_name = NULL;
	}

	if (this->site_lie && *this->site_lie)
	{
		free_mem (this->site_lie);
		this->site_lie = NULL;
	}

	if (this->imm_leave && *this->imm_leave)
	{
		free_mem (this->imm_leave);
		this->imm_leave = NULL;
	}

	if (this->imm_enter && *this->imm_enter)
	{
		free_mem (this->imm_enter);
		this->imm_enter = NULL;
	}

	if (this->creation_comment && *this->creation_comment)
	{
		free_mem (this->creation_comment);
		this->creation_comment = NULL;
	}

	if (this->msg && *this->msg)
	{
		free_mem (this->msg);
		this->msg = NULL;
	}

	this->owner = NULL;
}

mob_data::mob_data()
{
	this->owner = NULL;
	this->hnext = NULL;
	this->lnext = NULL;
	this->resets = NULL;
	this->cues = NULL;
	this->skinned_vnum = 0;
	this->carcass_vnum = 0;
	this->damnodice = 0;
	this->damroll = 0;
	this->min_height = 0;
	this->max_height = 0;
	this->size = 0;
	this->nVirtual = 0;
	this->zone = 0;
	this->spawnpoint = 0;
	this->merch_seven = 0;
	this->vehicle_type = 0;
	this->helm_room = 0;
	this->access_flags = 0;
	this->noaccess_flags = 0;
	this->reset_zone = 0;
	this->reset_cmd = 0;
	this->currency_type = 0;
	this->jail = 0;
	this->resets = NULL;
}

mob_data::~mob_data()
{
	if (this->owner && *this->owner)
	{
		free_mem(this->owner);
	}
}

// This function is only intended to be called by redefine_mobiles()
void char_data::partial_deep_copy (CHAR_DATA *proto)
{
	if (this->name)
	{
		free_mem(this->name);
	}
	this->name = duplicateString(proto->name);

	if (this->short_descr)
	{
		free_mem(this->short_descr);
	}
	this->short_descr = duplicateString(proto->short_descr);

	if (this->long_descr)
	{
		free_mem(this->long_descr);
	}
	this->long_descr = duplicateString(proto->long_descr);

	if (this->description)
	{
		free_mem(this->description);
	}
	this->description = duplicateString(proto->description);

	if (this->clans)
	{
		free_mem(this->clans);
	}
	this->clans = duplicateString(proto->clans);

	this->act = proto->act;
	this->mob->damnodice = proto->mob->damnodice;
	this->mob->damsizedice = proto->mob->damsizedice;
	this->position = proto->position;
	this->default_pos = proto->default_pos;
	this->hmflags = proto->hmflags;

	this->str = proto->str;
	this->dex = proto->dex;
	this->intel = proto->intel;
	this->aur = proto->aur;
	this->con = proto->con;
	this->wil = proto->wil;
	this->agi = proto->agi;

	this->flags = proto->flags;
	this->shop = proto->shop;

	this->hit = proto->hit;
	this->max_hit = proto->max_hit;
	this->move = proto->move;
	this->max_move = proto->max_move;
	this->armor = proto->armor;
	this->offense = proto->offense;
	this->mob->damroll = proto->mob->damroll;
	this->ppoints = proto->ppoints;
	this->nat_attack_type = proto->nat_attack_type;

	this->sex = proto->sex;
	this->deity = proto->deity;

	this->circle = proto->circle;
	this->mob->skinned_vnum = proto->mob->skinned_vnum;
	this->mob->carcass_vnum = proto->mob->carcass_vnum;
	this->mob->merch_seven = proto->mob->merch_seven;
	this->mob->vehicle_type = proto->mob->vehicle_type;
	this->mob->helm_room = proto->mob->helm_room;
	this->natural_delay = proto->natural_delay;
	this->fight_mode = proto->fight_mode;
	this->race = proto->race;
	this->mob->access_flags = proto->mob->access_flags;
	this->speaks = proto->speaks;

	/*
	this->height				  = proto->height;
	this->frame				  = proto->frame;
	*/
	this->age = proto->age;

	for (int i = 0; i < MAX_SKILLS; i++)
		this->skills[i] = proto->skills[i];

	this->str = proto->str;
	this->dex = proto->dex;
	this->con = proto->con;
	this->wil = proto->wil;
	this->aur = proto->aur;
	this->intel = proto->intel;

	this->mob->currency_type = proto->mob->currency_type;
}

void char_data::deep_copy (CHAR_DATA *copy_from)
{
	// Lazy way of getting everything non-dynamic across. One advantage of this approach is that people who add members to char_data don't have to add them here unless they use dynamic memory.
	mob_data *tmob = this->mob;
	pc_data *tpc = this->pc;
	memcpy (this, copy_from, sizeof(CHAR_DATA));
	this->mob = tmob;
	this->pc = tpc;

	if (copy_from->delay_who)
	{
		this->delay_who = duplicateString(copy_from->delay_who);
	}

	if (copy_from->delay_who2)
	{
		this->delay_who2 = duplicateString(copy_from->delay_who2);
	}

	if (copy_from->casting_arg)
	{
		this->casting_arg = duplicateString(copy_from->casting_arg);
	}

	if (copy_from->name)
	{
		this->name = duplicateString(copy_from->name);
	}

	if (copy_from->tname)
	{
		this->tname = duplicateString(copy_from->tname);
	}

	if (copy_from->short_descr)
	{
		this->short_descr = duplicateString(copy_from->short_descr);
	}

	if (copy_from->long_descr)
	{
		this->long_descr = duplicateString(copy_from->long_descr);
	}

	if (copy_from->pmote_str)
	{
		this->pmote_str = duplicateString(copy_from->pmote_str);
	}

	if (copy_from->voice_str)
	{
		this->voice_str = duplicateString(copy_from->voice_str);
	}

	if (copy_from->description)
	{
		this->description = duplicateString(copy_from->description);
	}

	if (copy_from->clans)
	{
		this->clans = duplicateString(copy_from->clans);
	}

	if (copy_from->enemy_direction)
	{
		this->enemy_direction = duplicateString(copy_from->enemy_direction);
	}

	if (copy_from->combat_log)
	{
		this->combat_log = duplicateString(copy_from->combat_log);
	}

	if (copy_from->travel_str)
	{
		this->travel_str = duplicateString(copy_from->travel_str);
	}

	if (copy_from->dmote_str)
	{
		this->dmote_str = duplicateString(copy_from->dmote_str);
	}

	if (copy_from->plan)
	{
		this->plan = new std::string (*copy_from->plan);
	}

	if (copy_from->goal)
	{
		this->goal = new std::string (*copy_from->goal);
	}

	if (copy_from->pc && this->pc)
	{
		this->pc->deep_copy(copy_from->pc);
	}

	if (copy_from->mob && this->mob)
	{
		this->mob->deep_copy(copy_from->mob);
	}
}

void pc_data::deep_copy (pc_data *copy_from)
{
	memcpy(this, copy_from, sizeof(pc_data));

	if (copy_from->creation_comment)
	{
		this->creation_comment = duplicateString(copy_from->creation_comment);
	}

	if (copy_from->imm_enter)
	{
		this->imm_enter = duplicateString(copy_from->imm_enter);
	}

	if (copy_from->imm_leave)
	{
		this->imm_leave = duplicateString(copy_from->imm_leave);
	}

	if (copy_from->site_lie)
	{
		this->site_lie = duplicateString(copy_from->site_lie);
	}

	if (copy_from->account_name)
	{
		this->account_name = duplicateString(copy_from->account_name);
	}

	if (copy_from->msg)
	{
		this->msg = duplicateString(copy_from->msg);
	}
}

void mob_data::deep_copy (mob_data *copy_from)
{
	memcpy(this, copy_from, sizeof(mob_data));

	if (copy_from->owner)
	{
		this->owner = duplicateString(copy_from->owner);
	}
}

bool char_data::getNaughtyFlag() {
	return naughtyFlag;
}

bool char_data::getRPFlag() {
	return RPFlag;
}
bool char_data::getPlotFlag() {
	return plotFlag;
}

void char_data::toggleNaughtyFlag() {
	if (naughtyFlag) {
		naughtyFlag = false;
	}
	else {
		naughtyFlag = true;
	}
}

void char_data::toggleRPFlag() {
	if (RPFlag) {
		RPFlag = false;
	}
	else {
		RPFlag = true;
	}
}

void char_data::togglePlotFlag() {
	if (plotFlag) {
		plotFlag = false;
	}
	else {
		plotFlag = true;
	}
}

std::pair<int, std::string> char_data::reportWhere(bool checkClan, int RPP, std::string whichClan) {

	bool underwater = (room->sector_type == SECT_RIVER
		|| room->sector_type == SECT_LAKE
		|| room->sector_type == SECT_OCEAN
		|| room->sector_type == SECT_REEF
		|| room->sector_type == SECT_UNDERWATER);

	int characterNameColour = 0;

	if ((pc && pc->level > 0 && !fighting)) {
		characterNameColour = 5;
	}
	else if (fighting) {
		characterNameColour = 1;
	}
	else if (underwater) {
		characterNameColour = 4;
	}
	else if (IS_SET (flags, FLAG_ISADMIN)) {
		characterNameColour = 6;
	}
	else if (IS_SET (plr_flags, NEW_PLAYER_TAG)) {
		characterNameColour = 2;
	}
	else if (IS_GUIDE (this)) { //If the person is a guide
		characterNameColour = 3;
	}
	else {
		characterNameColour = 0;
	}

	char characterState = '_';

	switch (GET_POS (this)) {
		case POSITION_DEAD:
		case POSITION_MORTALLYW:
			characterState = 'X';
			break;
		case POSITION_UNCONSCIOUS:
		case POSITION_STUNNED:
			characterState = 'U';
			break;
		case POSITION_SLEEPING:
			characterState = 's';
			break;
		default:
			if (!IS_NPC (this) && !desc && !pc->admin_loaded) {
				characterState = 'L';
			}
			else if (IS_SET (act, PLR_QUIET) && !IS_NPC (this)) {
				characterState = 'e';
			}
			else if (get_affect (this, MAGIC_HIDDEN)) {
				characterState = 'h';
			}
			else if (desc && desc->idle) {
				characterState = 'i';
			}
	}

	std::stringstream reportStream;

	reportStream << "#" << characterNameColour;

	int blankLineSize = 15;  // 15 characters alloted to display the name, down from 17
	if (tname)
	  {
	    blankLineSize -= strlen(tname);
	  }

	if (blankLineSize < 1) {
	  std::string tempString = tname;  //never true on null string, as it would still be 15
		reportStream << tempString.substr(0,15);
	}
	else {
	  reportStream << ((tname) ? tname : "(null)");
		for (size_t i = 0; i < blankLineSize; i++) {
			reportStream << " ";
		}
	}

	if (RPP < 0) {
		if (aur < 10) {
			reportStream << "#3[ " << aur << "]";
		}
		else {
			reportStream << "#3[" << aur << "]";
		}
	}
	else if (RPP < 10) {
		reportStream << "#3[ " << RPP << "]";
	}
	else {
		reportStream << "#3[" << RPP << "]";
	}

	reportStream << " #0" << characterState;
	reportStream << (getNaughtyFlag() ? "#1!" : "#0_");	// Flag to watch player for bad behaviour
	reportStream << (getRPFlag() ? "#2R" : "#0_");	// Flag to watch player for possible award
	reportStream << (getPlotFlag() ? "#3P" : "#0_") << " ";	// Flag to indicate player is in a plot

	char roomBuffer[MAX_STRING_LENGTH];

	// [rnum and char name]
	sprintf (roomBuffer, "#2[%6d]#6 %s#0", room->nVirtual, room->name);
	reportStream << roomBuffer;

	return std::make_pair(room->nVirtual, reportStream.str());
}

//Revision for PCs with multiple major clan-clanning - Vermonkey 090226
//Currently Unused - Vader
bool *char_data::checkClansForWho()
{
	bool *ret = new bool(6);
	ret[0] = IS_SET(flags,FLAG_GUEST);
	ret[1] = ret[2] = ret[3] = ret[4] = ret[5] = false;
	if( clans )
	{
		Stringstack clansArgument = clans;
		while(!(clansArgument.isFinished()))
		{
			clansArgument.pop(); // Pop Clan Flags
			clansArgument.pop(); // Pop Clan Name
			if( clansArgument == "mt_citizens" )
				ret[1] = true;
			else if( clansArgument == "outpost_citizens" )
				ret[2] = true;
			else if( clansArgument == "fahad_jafari" )
				ret[3] = true;
			else if( clansArgument == "moria_orks" )
				ret[4] = true;
			else if( clansArgument == "mordor_char" )
				ret[5] = true;
		}
	}
	return ret;
}

//Currently Unused - Vader
int char_data::majorSphere()
{
	if( IS_SET(flags,FLAG_GUEST) )
		return 0;
	if( clans )
	{
		Stringstack clansArgument = clans;
		while(!(clansArgument.isFinished()))
		{
			clansArgument.pop(); // Pop Clan Flags
			clansArgument.pop(); // Pop Clan Name
			if( clansArgument == "mt_citizens" )
				return 1;
			else if( clansArgument == "outpost_citizens" )
				return 2;
			else if( clansArgument == "fahad_jafari" )
				return 3;
			else if( clansArgument == "moria_orks" )
				return 4;
			else if( clansArgument == "mordor_char" )
				return 5;
		}
	}
	return -1;
}

bool char_data::isLevelFivePC() {
	if (pc && pc->level == 5) {
		return true;
	}

	if (desc && desc->acct) {
		MYSQL_RES *result;
		MYSQL_ROW row;
		std::string player_db = engine.get_config ("player_db");

		mysql_safe_query ("SELECT level FROM %s.pfiles WHERE account = '%s'", player_db.c_str (), desc->acct->name.c_str());
		result = mysql_store_result (database);

		if(!result || !mysql_num_rows(result)) {
			if( result != NULL) {
				mysql_free_result(result);
			}
			return false;
		}

		row = mysql_fetch_row (result);

		do {
			if(atoi(row[0]) == 5) {
				mysql_free_result(result);
				return true;
			}
			row = mysql_fetch_row (result);
		}while(row);
		mysql_free_result(result);
	}
	return false;
}

char * char_data::getLevelFiveName() {
	if (pc && pc->level == 5) {
		return tname;
	}

	if (desc && desc->acct) {
		MYSQL_RES *result;
		MYSQL_ROW row;
		std::string player_db = engine.get_config ("player_db");

		mysql_safe_query ("SELECT level,name FROM %s.pfiles WHERE account = '%s'", player_db.c_str (), desc->acct);
		result = mysql_store_result (database);

		if (!result || !mysql_num_rows(result)) {
			if (result != NULL) {
				mysql_free_result( result );
			}
			return NULL;
		}

		row = mysql_fetch_row (result);

		do {
			if(atoi(row[0]) == 5) {
				return row[1];
			}
			row = mysql_fetch_row (result);
		}while(row);
	}
	return NULL;
}

bool char_data::hasMortalBody() {
	if (!pc) {
		return true;
	}
	return (pc->level == 0 ? true : false);
}

unsigned int char_data::getWrapLength() {
	return 80;
}

void char_data::setWrapLength(unsigned int length) {
	wrapLength = length;
}

int char_data::get_char_num()
{
	if( mob )
		return mob->nVirtual;
	return pc->char_num;
}
