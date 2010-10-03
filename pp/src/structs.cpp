#include "structs.h"

default_item_data::default_item_data() {
	flags = 0;
	memset(items, 0, MAX_DEFAULT_ITEMS * sizeof(int));
	item_counts = 0;
	phase = NULL;
	key_index = 0;
	color = NULL;
}

default_item_data::default_item_data(const default_item_data &RHS) {
	flags = RHS.flags;
	memcpy(items, RHS.items, MAX_DEFAULT_ITEMS * sizeof(int));
	item_counts = RHS.item_counts;

	if (RHS.phase != NULL) {
		phase = new phase_data();
		phase = RHS.phase;
	}
	else {
		phase = NULL;
	}
	key_index = RHS.key_index;
	color = duplicateString(RHS.color);
}

default_item_data& default_item_data::operator= (const default_item_data &RHS) {
	if (this != &RHS) {
		flags = RHS.flags;
		memcpy(items, RHS.items, MAX_DEFAULT_ITEMS * sizeof(int));
		item_counts = RHS.item_counts;

		if (phase != NULL) {
			delete phase;
			phase = NULL;
		}

		if (RHS.phase != NULL) {
			phase = new phase_data(*(RHS.phase));
		}
		else {
			phase = NULL;
		}
		key_index = RHS.key_index;

		free_mem(color);
		color = duplicateString(RHS.color);
	}
	return *this;
}

default_item_data::~default_item_data() {
	free_mem(color);

	if (phase != NULL) {
		delete phase;
		phase = NULL;
	}
}

phase_data::phase_data() {
	first = NULL;
	second = NULL;
	third = NULL;
	self = NULL;
	second_failure = NULL;
	third_failure = NULL;
	failure = NULL;
	flags = 0;
	phase_seconds = 0;
	skill = 0;
	attr = -1;
	dice = 0;
	sides = 0;
	targets = 0;
	duration = 0;
	power = 0;
	hit_cost = 0;
	move_cost = 0;
	spell_type = 0;
	open_skill = 0;
	req_skill = 0;
	req_skill_value = 0;
	attribute = -1;
	tool = NULL;
	load_mob = 0;
	nMobFlags = 0;
	group_mess = NULL;
	fail_group_mess = NULL;
	next = NULL;
}

phase_data::phase_data(const phase_data &RHS) {
	first = duplicateString(RHS.first);
	second = duplicateString(RHS.second);
	third = duplicateString(RHS.third);
	self = duplicateString(RHS.self);
	second_failure = duplicateString(RHS.second_failure);
	third_failure = duplicateString(RHS.third_failure);
	failure = duplicateString(RHS.failure);
	flags = RHS.flags;
	phase_seconds = RHS.phase_seconds;
	skill = RHS.skill;
	attr = RHS.attr;
	dice = RHS.dice;
	sides = RHS.sides;
	targets = RHS.targets;
	duration = RHS.duration;
	power = RHS.power;
	hit_cost = RHS.hit_cost;
	move_cost = RHS.move_cost;
	spell_type = RHS.spell_type;
	open_skill = RHS.open_skill;
	req_skill = RHS.req_skill;
	req_skill_value = RHS.req_skill_value;
	attribute = RHS.attribute;

	if (RHS.tool != NULL) {
		tool = new default_item_data(*(RHS.tool));
	}
	else {
		tool = new default_item_data;
	}
	load_mob = RHS.load_mob;
	nMobFlags = RHS.nMobFlags;
	group_mess = duplicateString(RHS.group_mess);
	fail_group_mess = duplicateString(RHS.fail_group_mess);
	next = RHS.next;
	
	
}

phase_data& phase_data::operator= (const phase_data &RHS) {
	if (this != &RHS) {
		free_mem(first);
		first = duplicateString(RHS.first);

		free_mem(second);
		second = duplicateString(RHS.second);

		free_mem(third);
		third = duplicateString(RHS.third);

		free_mem(self);
		self = duplicateString(RHS.self);

		free_mem(second_failure);
		second_failure = duplicateString(RHS.second_failure);

		free_mem(third_failure);
		third_failure = duplicateString(RHS.third_failure);

		free_mem(failure);
		failure = duplicateString(RHS.failure);

		flags = RHS.flags;
		phase_seconds = RHS.phase_seconds;
		skill = RHS.skill;
		attr = RHS.attr;
		dice = RHS.dice;
		sides = RHS.sides;
		targets = RHS.targets;
		duration = RHS.duration;
		power = RHS.power;
		hit_cost = RHS.hit_cost;
		move_cost = RHS.move_cost;
		spell_type = RHS.spell_type;
		open_skill = RHS.open_skill;
		req_skill = RHS.req_skill;
		req_skill_value = RHS.req_skill_value;
		attribute = RHS.attribute;

		if (tool != NULL) {
			delete tool;
			tool = NULL;
		}
		tool = new default_item_data(*(RHS.tool));

		load_mob = RHS.load_mob;
		nMobFlags = RHS.nMobFlags;

		free_mem(group_mess);
		group_mess = duplicateString(RHS.group_mess);

		free_mem(fail_group_mess);
		fail_group_mess = duplicateString(RHS.fail_group_mess);
		next = RHS.next;
	}
	return *this;
}

phase_data::~phase_data() {
	free_mem(first);
	free_mem(second);
	free_mem(third);
	free_mem(self);
	free_mem(second_failure);
	free_mem(third_failure);
	free_mem(failure);
	free_mem(group_mess);
	free_mem(fail_group_mess);

	if (tool != NULL) {
		delete tool;
		tool = NULL;
	}
}

subcraft_head_data::subcraft_head_data() {
	craft_name = NULL;
	subcraft_name = NULL;
	command = NULL;
	failure = NULL;
	failobjs = NULL;
	failmobs = NULL;
	help = NULL;
	clans = NULL;
	phases = NULL;

	memset(obj_items, 0, (MAX_ITEMS_PER_SUBCRAFT * sizeof(DEFAULT_ITEM_DATA*)));
	memset(fails, 0, (MAX_ITEMS_PER_SUBCRAFT * sizeof(DEFAULT_ITEM_DATA*)));
	memset(sectors, 0, SECTORSMAX * sizeof(int));
	memset(seasons, 0, SEASONSMAX * sizeof(int));
	memset(opening, 0, OPENINGMAX * sizeof(int));
	memset(race, 0, RACEMAX * sizeof(int));
	memset(weather, 0, WEATHERMAX * sizeof(int));

	subcraft_flags = 0;
	failmob = 0;
	delay = 0;
	faildelay = 0;
	key_index = 0;
	key_first = 0;
	key_end = 0;
	followers = 0;

	next = NULL;
}

subcraft_head_data::subcraft_head_data(const subcraft_head_data &RHS) {
	craft_name = duplicateString(RHS.craft_name);
	subcraft_name = duplicateString(RHS.subcraft_name);
	command = duplicateString(RHS.command);
	failure = duplicateString(RHS.failure);
	failobjs = duplicateString(RHS.failobjs);
	failmobs = duplicateString(RHS.failmobs);
	help = duplicateString(RHS.help);
	clans = duplicateString(RHS.clans);

	if (RHS.phases != NULL) {
		phases = new phase_data(*(RHS.phases));
	}
	else {
		phases = new phase_data;
	}

	for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++) {
		if (RHS.obj_items[i] != NULL) {
			obj_items[i] = new default_item_data(*(RHS.obj_items[i]));
		}
		else {
			obj_items[i] = NULL;
		}

		if (RHS.fails[i] != NULL) {
			fails[i] = new default_item_data(*(RHS.fails[i]));
		}
		else {
			fails[i] = NULL;
		}
	}

	subcraft_flags = RHS.subcraft_flags;

	memcpy(sectors, RHS.sectors, SECTORSMAX * sizeof(int));
	memcpy(seasons, RHS.seasons, SEASONSMAX * sizeof(int));
	memcpy(opening, RHS.opening, OPENINGMAX * sizeof(int));
	memcpy(race, RHS.race, RACEMAX * sizeof(int));
	memcpy(weather, RHS.weather, WEATHERMAX * sizeof(int));

	failmob = RHS.failmob;
	delay = RHS.delay;
	faildelay = RHS.faildelay;
	key_index = RHS.key_index;
	key_first = RHS.key_first;
	key_end = RHS.key_end;
	followers = RHS.followers;

	// This is part of a global list so deep copying this creates huge problems - Case
	next = NULL;
}

subcraft_head_data& subcraft_head_data::operator= (const subcraft_head_data &RHS) {
	if (this != &RHS) {
		free_mem(craft_name);
		craft_name = duplicateString(RHS.craft_name);

		free_mem(subcraft_name);
		subcraft_name = duplicateString(RHS.subcraft_name);

		free_mem(command);
		command = duplicateString(RHS.command);

		free_mem(failure);
		failure = duplicateString(RHS.failure);

		free_mem(failobjs);
		failobjs = duplicateString(RHS.failobjs);

		free_mem(failmobs);
		failmobs = duplicateString(RHS.failmobs);

		free_mem(help);
		help = duplicateString(RHS.help);

		free_mem(clans);
		clans = duplicateString(RHS.clans);

		if (phases != NULL) {
			delete phases;
			phases = NULL;
		}

		if (RHS.phases != NULL) {
			phases = new phase_data(*(RHS.phases));
		}
		else {
			phases = new phase_data;
		}

		for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++) {
			if (obj_items[i] != NULL) {
				delete obj_items[i];
				obj_items[i] = NULL;
			}

			if (fails[i] != NULL) {
				delete fails[i];
				fails[i] = NULL;
			}

			if (RHS.obj_items[i] != NULL) {
				obj_items[i] = new default_item_data(*(RHS.obj_items[i]));
			}
			else {
				obj_items[i] = NULL;
			}

			if (RHS.fails[i] != NULL) {
				fails[i] = new default_item_data(*(RHS.fails[i]));
			}
			else {
				fails[i] = NULL;
			}
		}

		subcraft_flags = RHS.subcraft_flags;

		memcpy(sectors, RHS.sectors, SECTORSMAX * sizeof(int));
		memcpy(seasons, RHS.seasons, SEASONSMAX * sizeof(int));
		memcpy(opening, RHS.opening, OPENINGMAX * sizeof(int));
		memcpy(weather, RHS.weather, WEATHERMAX * sizeof(int));
		memcpy(race, RHS.race, RACEMAX * sizeof(int));
		

		failmob = RHS.failmob;
		delay = RHS.delay;
		faildelay = RHS.faildelay;
		key_index = RHS.key_index;
		key_first = RHS.key_first;
		key_end = RHS.key_end;
		followers = RHS.followers;

		// This is part of a global list so deep copying this creates huge problems - Case
		next = NULL;
	}
	return *this;
}

subcraft_head_data::~subcraft_head_data() {
	free_mem(craft_name);
	free_mem(subcraft_name);
	free_mem(command);
	free_mem(failure);
	free_mem(failobjs);
	free_mem(failmobs);
	free_mem(help);
	free_mem(clans);

	for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++) {
		if (obj_items[i] != NULL) {
			delete obj_items[i];
			obj_items[i] = NULL;
		}

		if (fails[i] != NULL) {
			delete fails[i];
			fails[i] = NULL;
		}
	}

	if (phases != NULL) {
		delete phases;
		phases = NULL;
	}
}
