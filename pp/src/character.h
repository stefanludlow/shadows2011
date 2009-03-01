#ifndef _rpie_character_h
#define _rpie_character_h

#include <map>
#include <string>

#include "constants.h"
#include "structs.h"

class pc_data {
public:
	ALIAS_DATA *aliases;
	ALIAS_DATA *execute_alias;
	
	bitflag nanny_state;
	bitflag role;

	bool admin_loaded;

	char *creation_comment;
	char *imm_enter;		///< Immortals only
	char *imm_leave;
	char *site_lie;		/* Lie about connecting site */
	char *account_name;
	char *msg;			/* SUBMIT/APPROVAL system */

	CHAR_DATA *edit_player;
	CHAR_DATA *target_mob;	/* Used by 'mob' and 'resets' */
	CHAR_DATA *dot_shorthand;	/* '.' indicates last referenced n/pc */

	DREAM_DATA *dreams;
	DREAM_DATA *dreamed;

	// Values for staff functions
	int last_global_staff_msg;
	int staff_notes;
	int mortal_mode;		/* Immortals can act mortal */
	int create_state;		/* Approval system */
	int edit_obj;
	int edit_mob;
	int load_count;		/* See load_pc */

	// Stats
	int start_str;
	int start_dex;
	int start_con;
	int start_wil;
	int start_aur;
	int start_intel;
	int start_agi;

	// Miscellaneous variables
	int level;
	int boat_virtual;
	int mount_speed;
	int time_last_activity;
	int is_guide;
	int profession;
	int app_cost;
	int chargen_flags;
	int last_global_pc_msg;
	int sleep_needed;
	int auto_toll;		/* Amount willing to pay if following */
	int doc_type;			/* Not saved.  Current doc */
	int doc_index;		/* Not saved.  type relative index */

	OBJ_DATA *writing_on;

	ROLE_DATA *special_role;

	shortint skills[MAX_SKILLS];
	struct descriptor_data *owner;
	
	time_t last_logon;
	time_t last_logoff;
	time_t last_disconnect;
	time_t last_connect;
	time_t last_died;
	
	SUBCRAFT_HEAD_DATA *edit_craft;

	pc_data();
	~pc_data();
};

class mob_data {
public:
	char *owner;

	CHAR_DATA *hnext;
	CHAR_DATA *lnext;

	// Corpses
	int skinned_vnum;		/* What mob skins into */
	int carcass_vnum;		/* What mob leaves for a carcass */
	
	// Attributes
	int damnodice;
	int damsizedice;
	int damroll;
	int min_height;		// min height for proto define
	int max_height;		// min height for proto define
	int size;			// race size for proto define

	// Miscellaneous values
	int nVirtual;
	int zone;
	int spawnpoint;
	int merch_seven;		/* Merchant set up for 7 economic rules like Regi wanted - punt in time */
	int vehicle_type;		/* Mobile value: boat, etc */
	int helm_room;		/* If boat, defines helm */
	int access_flags;		/* Flags; mob room access */
	int noaccess_flags;		/* Flags; mob room noaccess */
	int reset_zone;		/* mobs only */
	int reset_cmd;		/* mobs only */
	int currency_type;
	int jail;			/* What jail the enforcer brings criminals to */
		
	RESET_DATA *resets;

	// Mob Triggers
	std::multimap<mob_cue,std::string> *cues;
	mob_data();
	~mob_data();
};

class char_data {
private:
	unsigned int wrapLength;

public:
	AFFECTED_TYPE *hour_affects;
	
	ATTACKER_DATA *attackers;

	bitflag act;
	bitflag hmflags;
	bitflag plr_flags;
	bitflag petition_flags; /* grommit - used to subscribe to zones for sphere-based petitions*/

	bool enforcement [100]; // keeps track of zones this char is enforcer in
	bool bleeding_prompt;
	bool naughtyFlag;
	bool RPFlag;
	bool plotFlag;

	char *delay_who;
	char *delay_who2;
	char *casting_arg;		/* Contains delayed spell */
	char *name;
	char *tname;
	char *short_descr;
	char *long_descr;
	char *pmote_str;
	char *voice_str;
	char *description;
	char *clans;
	char *enemy_direction;
	char *combat_log;
	char *travel_str;
	char *dmote_str;

	CHAR_DATA *subdue;		/* Subduer or subduee */
	CHAR_DATA *vehicle;		/* Char that is the vehicle */
	CHAR_DATA *next_in_room;
	CHAR_DATA *next;
	CHAR_DATA *next_fighting;
	CHAR_DATA *next_assist;
	CHAR_DATA *following;
	CHAR_DATA *fighting;
	CHAR_DATA *delay_ch;
	CHAR_DATA *mount;		/* Rider/Ridee */
	CHAR_DATA *hitcher;
	CHAR_DATA *hitchee;
	CHAR_DATA *aiming_at;
	CHAR_DATA *targeted_by;
	CHAR_DATA *ranged_enemy;

	ENCHANTMENT_DATA *enchantments;
	
	// Attributes
	int str;
	int intel;
	int wil;
	int dex;
	int con;
	int aur;
	int agi;
	int tmp_str;
	int tmp_intel;
	int tmp_wil;
	int tmp_dex;
	int tmp_con;
	int tmp_aur;
	int tmp_agi;

	// Body values
	int balance;
	int bmi;
	int body_proto;
	int body_type;
	int height;
	int frame;	/* Determines hit locations */
	int size;

	// Physical values
	int hunger;
	int thirst;
	int sex;
	int age;
	int race;
	int move_points;		/* Not saved; move remainder */
	int hit_points;		/* Not saved; hit remainder */
	int hit;
	int max_hit;
	int move;
	int max_move;
	int armor;
	int carry_weight;
	int carry_items;
	int lastregen;
	
	// Combat variables
	int fight_percentage;
	int primary_delay;		/* ticks to next hit */
	int secondary_delay;		/* ticks to next hit */
	int fight_mode;		/* Frantic..defensive */
	int nat_attack_type;		/* Bite, claw, slime, etc */
	int natural_delay;		/* Added delay for natural */
	int damage;
	int distance_to_target;
	int attack_type;
	int speed;
	int defensive;
	int laststuncheck;
	int knockedout;
	int stun;
	int aim;
	int whirling;

	// Magic variables
	int spells[MAX_LEARNED_SPELLS][2];
	int circle;			/* Rank within church */ // Should be bloody removed, why it's still here is beyond me - Case
	int mana;
	int max_mana;
	int harness;
	int max_harness;
	int curse;

	// Economic variables
	int hire_storeroom;
	int hire_storeobj;

	// Skills variables
	int speaks;			/* Currently spoken language */
	int psionic_talents;
	int skills[MAX_SKILLS];
	int offense;

	// Morph/Craft Values
	int morph_type;
	int morph_time;
	int morphto;
	int craft_index; //remembers which item was used in craft key-pair

	// Miscellaneous variables
	int deleted;
	int clock;
	int cover_from_dir;
	int in_room;
	int debug_mode;
	int coldload_id;		/* Uniq id of mob */
	int flags;			/* FLAG_ stuff */
	int alarm;			/* Not saved. */
	int trigger_delay;		/* Not saved. */
	int trigger_line;		/* Used with trigger_delay */
	int trigger_id;		/* Used with trigger_delay */
	int ppoints; // What? - Case
	int position;
	int default_pos;
	int delay_type;
	int delay;
	int delay_info1;
	int delay_info2;
	int delay_info3;
	int was_in_room;
	int intoxication;
	int last_room;
	int deity;
	int color;
	int cell_1, cell_2, cell_3;	/* NPC jailer cell VNUMS */
	int writes;
	int preparing_id;
	int preparing_time;
	int assist_pos;
	int roundtime;
	int from_dir;
	// int nightmare;		/* Nightmare sequence # */ // May get away with commenting this out - Case
	// int poison_type;		/* Bite causes poison */  // Should be removed when it's possible - Case
	
	LODGED_OBJECT_INFO *lodged;

	long affected_by;

	MOB_DATA *mob;

	MOBPROG_DATA *prog;
	
	MOVE_DATA *moves;

	OBJ_DATA *equip;
	OBJ_DATA *delay_obj;
	OBJ_DATA *right_hand;
	OBJ_DATA *left_hand;

	PC_DATA *pc;

	// POISON_DATA *venom;

	ROOM_DATA *room;

	SHOP_DATA *shop;

	SIGHTED_DATA *sighted;

	std::string *plan;
	std::string *goal;

	struct var_data *vartab;	/* Mob program variable table */
	struct descriptor_data *desc;
	struct memory_data *remembers;
	struct time_data time;

	THREAT_DATA *threats;

	unsigned short int guardian_mode;

	WOUND_DATA *wounds;
	// std::set<CHAR_DATA*> *group; // complement to ch->following

	void clear_char();
	char_data();
	~char_data();
	
	std::pair<int, std::string> reportWhere(bool checkClan, int RPP, std::string whichClan);
	
	// Character Flags For reportWhere()
	bool getNaughtyFlag();
	bool getRPFlag();
	bool getPlotFlag();
	void toggleNaughtyFlag();
	void toggleRPFlag();
	void togglePlotFlag();

	// Utility Method For do_who - These are currently Unused - Vader
	bool* checkClansForWho();
	int majorSphere();	

	// Methods to establish if account this character is on is Level 5, and whether to treat them like one
	bool isLevelFivePC();
	char * getLevelFiveName();
	bool hasMortalBody();

	// Lexing methods
	unsigned int getWrapLength();
	void setWrapLength(unsigned int length);
	
};
#endif
