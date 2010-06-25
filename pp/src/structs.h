//////////////////////////////////////////////////////////////////////////////
//
/// structs.h - Data Structures
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2004-2006 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
/// URL: http://www.middle-earth.us
//
/// May includes portions derived from Harshlands
/// Authors: Charles Rand (Rassilon)
/// URL: http://www.harshlands.net
//
/// May include portions derived under license from DikuMUD Gamma (0.0)
/// which are Copyright (C) 1990, 1991 DIKU
/// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
///          Tom Madson (noop@freja.diku.dk)
///          Katja Nyboe (katz@freja.diku.dk)
///          Michael Seifert (seifert@freja.diku.dk)
///          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _rpie_structs_h_
#define _rpie_structs_h_

#include <string>
#include <string.h>
#include <set>
#include <map>
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/time.h>

#include "constants.h"
#include "object_damage.h"

typedef struct poison_data POISON_DATA;
typedef class track_data TRACK_DATA;
typedef class threat_data THREAT_DATA;
typedef class attacker_data ATTACKER_DATA;
typedef class newbie_hint NEWBIE_HINT;
typedef class role_data ROLE_DATA;
typedef struct enchantment_data ENCHANTMENT_DATA;
typedef struct known_spell_data KNOWN_SPELL_DATA;
typedef struct spell_table_data SPELL_TABLE_DATA;
typedef class help_info HELP_INFO;
typedef class writing_data WRITING_DATA;
typedef class site_info SITE_INFO;
typedef class wound_data WOUND_DATA;
typedef class affected_type AFFECTED_TYPE;
typedef class alias_data ALIAS_DATA;
typedef class board_data BOARD_DATA;
typedef struct char_ability_data CHAR_ABILITY_DATA;
typedef class char_data CHAR_DATA;
typedef class combat_msg_data COMBAT_MSG_DATA;
typedef struct delayed_affect_data DELAYED_AFFECT_DATA;
typedef class dream_data DREAM_DATA;
typedef class extra_descr_data EXTRA_DESCR_DATA;
typedef class default_item_data DEFAULT_ITEM_DATA;
typedef class help_data HELP_DATA;
typedef struct memory_t MEMORY_T;
typedef class mudmail_data MUDMAIL_DATA;
typedef class mob_data MOB_DATA;
typedef class mobprog_data MOBPROG_DATA;
typedef class move_data MOVE_DATA;
typedef class name_switch_data NAME_SWITCH_DATA;
typedef class negotiation_data NEGOTIATION_DATA;
typedef struct obj_data OBJ_DATA;
typedef class pc_data PC_DATA;
typedef class phase_data PHASE_DATA;
typedef class qe_data QE_DATA;
typedef class reset_affect RESET_AFFECT;
typedef class reset_data RESET_DATA;
typedef struct reset_time_data RESET_TIME_DATA;
typedef class room_direction_data ROOM_DIRECTION_DATA;
typedef struct room_extra_data ROOM_EXTRA_DATA;
typedef class room_prog ROOM_PROG;
typedef class shop_data SHOP_DATA;
typedef class second_affect SECOND_AFFECT;
typedef class social_data SOCIAL_DATA;
typedef struct spell_data SPELL_DATA;
typedef class subcraft_head_data SUBCRAFT_HEAD_DATA;
typedef class text_data TEXT_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef class var_data VAR_DATA;
typedef class written_descr_data WRITTEN_DESCR_DATA;
typedef class lodged_object_info LODGED_OBJECT_INFO;
typedef class lodged_missile_info LODGED_MISSILE_INFO;
typedef struct body_info BODY_INFO;
typedef class sighted_data SIGHTED_DATA;
typedef class obj_clan_data OBJ_CLAN_DATA;

#ifndef __cplusplus
typedef char bool;
#endif
typedef char byte;
typedef unsigned long bitflag;
typedef signed char shortint;

#define TREAT_ALL		(1 << 0)
#define TREAT_SLASH		(1 << 1)
#define TREAT_BLUNT		(1 << 2)
#define TREAT_PUNCTURE		(1 << 3)
#define TREAT_BURN		(1 << 6)
#define TREAT_FROST		(1 << 7)
#define TREAT_BLEED		(1 << 8)
#define TREAT_INFECTION		(1 << 9) //for future expansion

#define SEARCH_KEYWORD		1
#define SEARCH_SDESC		2
#define SEARCH_LDESC		3
#define SEARCH_FDESC		4
#define SEARCH_RACE		5
#define SEARCH_CLAN		6
#define SEARCH_SKILL		7
#define SEARCH_ROOM		8
#define SEARCH_LEVEL		9
#define SEARCH_STAT		10

/* Second Affects */
#define SA_STAND		1
#define SA_GET_OBJ		2
#define SA_WEAR_OBJ		3
#define SA_CLOSE_DOOR		4
#define SA_WORLD_SWAP		5
#define SA_UNUSED_1		6
#define SA_UNUSED_2		7
#define SA_UNUSED_3		8
#define SA_UNUSED_4		9
#define SA_UNUSED_5		10
#define SA_UNUSED_6		11
#define SA_WALK			12
#define SA_RUN			13
#define SA_FOLLOW		14
#define SA_SWIM			15
#define SA_SHADOW		16
#define SA_FLOOD		17
#define SA_KNOCK_OUT		18
#define SA_ESCAPE		19
#define SA_MOVE			20
#define SA_RESCUE		21
#define SA_COMMAND 	 	22
#define SA_WARNED		23
#define SA_ALREADY_WARNED 24
#define SA_FLEEING_WARNED 25
#define SA_DOANYWAY	26
#define SA_GFOLLOW 27
#define SA_LEAD 28

#define VNUM_PENNY		40
#define VNUM_FARTHING		41
#define VNUM_MONEY		42	/* The answer to everything */
#define VNUM_JAILBAG		43
#define VNUM_TICKET		44
#define VNUM_HEAD		45
#define VNUM_CORPSE		46
#define VNUM_SPEAKER_TOKEN	175
#define VNUM_STATUE		195
#define VNUM_ORDER_TICKET	575

#define VNUM_TIRITH_REPOS		3425
#define VNUM_TIRITH_RELEASE_ROOM	1357
#define VNUM_TIRITH_CELL_1		3418
#define VNUM_TIRITH_CELL_2		3419
#define VNUM_TIRITH_CELL_3		3420

#define VNUM_OSGILIATH_RELEASE_ROOM	2169
#define VNUM_MORGUL_RELEASE_ROOM	5064
#define VNUM_TE_RELEASE_ROOM		66003

#define VNUM_WELLGROOMED_RIDING_HORSE	29
#define VNUM_WELLGROOMED_WAR_HORSE	31

#define HOLIDAY_METTARE		1
#define HOLIDAY_YESTARE		2
#define HOLIDAY_TUILERE		3
#define HOLIDAY_LOENDE		4
#define HOLIDAY_ENDERI		5
#define HOLIDAY_YAVIERE		6

#define SPRING	        0
#define SUMMER  	    1
#define AUTUMN		    2
#define WINTER	        3

#define BASE_SPELL		1000

#define BLEEDING_INTERVAL 1	/* 1 minute per "bleeding pulse". */
#define BASE_SPECIAL_HEALING 30	/* Increased healing rate, for special mobs/PCs. */
#define BASE_PC_ELF_HEALING 15      // -
#define BASE_PC_DWARF_HEALING 20    //  | November 21st, Power changes - Case
#define BASE_PC_ORCTROLL_HEALING 30 // -
#define BASE_PC_STANDARD_HEALING 40	/* Number of minutes per healing pulse for standard PCs. */

/* debug parameters */

#define DEBUG_FIGHT		1
#define DEBUG_MISC		2
#define DEBUG_SKILLS	4
#define DEBUG_SUMMARY	8
#define DEBUG_SUBDUE	16

/* registry */

#define REG_REGISTRY		0
#define REG_MAXBONUS		1
#define REG_SPELLS			2
#define REG_DURATION		3
#define REG_OV				4
#define REG_LV				5
#define REG_CAP				6
#define REG_SKILLS			7	/* Link to DURATION, OV, LV, CAP */
#define REG_MISC			8
#define REG_MISC_NAMES		9	/* Link to MISC */
#define REG_MISC_VALUES		10
#define REG_MAGIC_SPELLS	11
#define REG_MAGIC_SPELLS_D	12	/* Linked to MAGIC_SPELLS */
#define REG_MAX_RATES		13	/* Learning rates table skill vs formula */
#define REG_CRAFT_MAGIC		14

#define MISC_DELAY_OFFSET	0
#define MISC_MAX_CARRY_N	1
#define MISC_MAX_CARRY_W	2
#define MISC_MAX_MOVE		3
#define MISC_STEAL_DEFENSE	4

#define CRIM_FIGHTING	5
#define CRIM_STEALING	3

/* vehicle types */

#define VEHICLE_BOAT	1
#define VEHICLE_HITCH	2


/* generic find defines */

#define FIND_CHAR_ROOM    	(1<<0)
#define FIND_CHAR_WORLD    	(1<<1)
#define FIND_OBJ_INV      	(1<<2)
#define FIND_OBJ_ROOM     	(1<<3)
#define FIND_OBJ_WORLD   	(1<<4)
#define FIND_OBJ_EQUIP   	(1<<5)

/* mob/object hash */

#define GEN_MOB 	1
#define GEN_OBJ 	2
#define GEN_WLD 	3
#define MAX_HASH    1000	/* 100 vnums per hash */

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif


#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3



#define WEATHER_TEMPERATE	0
#define WEATHER_COOL		1
#define WEATHER_COLD		2
#define WEATHER_ARCTIC		3
#define WEATHER_WARM		4
#define WEATHER_HOT		5
#define WEATHER_DESERT		6

/* Weather-room descrition constants - see weather_room in constants.c */

#define		WR_DESCRIPTIONS		12	/* Doesn't include WR_NORMAL */

#define		WR_FOGGY		0
#define		WR_CLOUDY		1
#define		WR_RAINY		2
#define		WR_STORMY		3
#define		WR_SNOWY		4
#define		WR_BLIZARD		5
#define		WR_NIGHT		6
#define		WR_NIGHT_FOGGY		7
#define		WR_NIGHT_RAINY		8
#define		WR_NIGHT_STORMY		9
#define		WR_NIGHT_SNOWY		10
#define		WR_NIGHT_BLIZARD	11
#define		WR_NORMAL		12

/* For 'type_flag' */

#define ITEM_LIGHT          1
#define ITEM_SCROLL         2
#define ITEM_WAND           3
#define ITEM_STAFF          4
#define ITEM_WEAPON         5
#define ITEM_SHIELD 	    6
#define ITEM_MISSILE        7
#define ITEM_TREASURE       8
#define ITEM_ARMOR          9
#define ITEM_POTION    	    10
#define ITEM_WORN      	    11
#define ITEM_OTHER     	    12
#define ITEM_TRASH     	    13
#define ITEM_TRAP      	    14
#define ITEM_CONTAINER 	    15
#define ITEM_NOTE      	    16
#define ITEM_DRINKCON   	17
#define ITEM_KEY       	    18
#define ITEM_FOOD      	    19
#define ITEM_MONEY     	    20
#define ITEM_ORE       	    21
#define ITEM_BOARD     	    22
#define ITEM_FOUNTAIN  	    23
#define ITEM_GRAIN		    24
#define ITEM_PERFUME	    25
#define ITEM_POTTERY	    26
#define ITEM_SALT		    27
#define ITEM_ZONE		    28
#define ITEM_PLANT          29
#define ITEM_COMPONENT	    30
#define ITEM_HERB		    31
#define ITEM_SALVE		    32
#define ITEM_POISON		    33
#define ITEM_LOCKPICK	    34
#define ITEM_INST_WIND      35
#define ITEM_INST_PERCU     36
#define ITEM_INST_STRING 	37
#define ITEM_FUR        	38
#define ITEM_WOODCRAFT  	39
#define ITEM_SPICE		    40
#define ITEM_TOOL		    41
#define ITEM_USURY_NOTE		42
#define ITEM_BRIDLE			43
#define ITEM_TICKET			44
#define ITEM_HEAD			45
#define ITEM_DYE			46
#define ITEM_CLOTH			47
#define ITEM_INGOT			48
#define ITEM_TIMBER			49
#define ITEM_FLUID			50
#define ITEM_LIQUID_FUEL	51
#define ITEM_HEALER_KIT		52
#define ITEM_PARCHMENT		53
#define ITEM_BOOK		54
#define ITEM_WRITING_INST	55
#define ITEM_INK		56
#define ITEM_QUIVER		57
#define ITEM_SHEATH		58
#define ITEM_KEYRING		59
#define ITEM_BULLET		60
#define ITEM_NPC_OBJECT		61
#define ITEM_DWELLING		62
#define ITEM_UNUSED		63
#define ITEM_REPAIR_KIT		64
#define ITEM_TOSSABLE		65
#define ITEM_CARD		66
#define ITEM_MERCH_TICKET	67
#define ITEM_ROOM_RENTAL	68

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE           (1<<0)
#define ITEM_WEAR_FINGER    (1<<1)
#define ITEM_WEAR_NECK      (1<<2)
#define ITEM_WEAR_BODY      (1<<3)
#define ITEM_WEAR_HEAD      (1<<4)
#define ITEM_WEAR_LEGS      (1<<5)
#define ITEM_WEAR_FEET      (1<<6)
#define ITEM_WEAR_HANDS     (1<<7)
#define ITEM_WEAR_ARMS      (1<<8)
#define ITEM_WEAR_SHIELD    (1<<9)
#define ITEM_WEAR_ABOUT		(1<<10)
#define ITEM_WEAR_WAIST   	(1<<11)
#define ITEM_WEAR_WRIST    	(1<<12)
#define ITEM_WIELD         	(1<<13)
#define ITEM_HOLD         	(1<<14)
#define ITEM_THROW        	(1<<15)
#define ITEM_WEAR_UNUSED	( 1 << 16 )
#define ITEM_WEAR_SHEATH	( 1 << 17 )
#define ITEM_WEAR_BELT		( 1 << 18 )
#define ITEM_WEAR_BACK		( 1 << 19 )
#define ITEM_WEAR_BLINDFOLD	( 1 << 20 )
#define ITEM_WEAR_THROAT	( 1 << 21 )
#define ITEM_WEAR_EAR		( 1 << 22 )
#define ITEM_WEAR_SHOULDER	( 1 << 23 )
#define ITEM_WEAR_ANKLE		( 1 << 24 )
#define ITEM_WEAR_HAIR		( 1 << 25 )
#define ITEM_WEAR_FACE		( 1 << 26 )
#define ITEM_WEAR_ARMBAND	( 1 << 27 )	/* Shoulder Patches */


/* NOTE: UPDATE wear_bits in constants.c */

/* Bitvector for 'extra_flags' */

#define ITEM_DESTROYED      ( 1 << 0 )
#define ITEM_PITCHABLE      ( 1 << 1 )	/* Dwelling that needs pitching before use. */
#define ITEM_INVISIBLE      ( 1 << 2 )
#define ITEM_MAGIC          ( 1 << 3 )
#define ITEM_NODROP         ( 1 << 4 )
#define ITEM_BLESS          ( 1 << 5 )
#define ITEM_GET_AFFECT	    ( 1 << 6 )
#define ITEM_DROP_AFFECT    ( 1 << 7 )
#define ITEM_MULTI_AFFECT   ( 1 << 8 )
#define ITEM_WEAR_AFFECT    ( 1 << 9 )
#define ITEM_WIELD_AFFECT   ( 1 << 10 )
#define ITEM_HIT_AFFECT	    ( 1 << 11 )
#define ITEM_OK	      		( 1 << 12 )
#define ITEM_COMBINABLE	    ( 1 << 13 )
#define ITEM_LEADER	        ( 1 << 14 )
#define ITEM_MEMBER	        ( 1 << 15 )
#define ITEM_OMNI	        ( 1 << 16 )
#define ITEM_ILLEGAL	    ( 1 << 17 )
#define ITEM_RESTRICTED	    ( 1 << 18 )
#define ITEM_MASK	        ( 1 << 19 )
#define ITEM_MOUNT			( 1 << 20 )
#define ITEM_TABLE			( 1 << 21 )
#define ITEM_STACK			( 1 << 22 )	/* Item stack with same vnum objs */
#define ITEM_VNPC_DWELLING		( 1 << 23 )	/* Becomes vNPC dwelling after PC logout */
#define ITEM_LOADS			( 1 << 24 )	/* Item is loaded by a reset */
#define ITEM_VARIABLE		( 1 << 25 )
#define ITEM_TIMER		( 1 << 26 )	/* Will decay */
#define ITEM_PC_SOLD		( 1 << 27 )	/* Sold to shopkeep by PC. */
#define ITEM_THROWING		( 1 << 28 )	/* Weapon is suitable for throwing */
#define ITEM_NEWSKILLS		( 1 << 29 )	/* Doesn't need to be converted */
#define ITEM_PITCHED		( 1 << 30 )	/* Whether tent has been pitched or not */
#define ITEM_VNPC		( 1 << 31 )	/* Item exists but isn't visible to players */

/* Some different kind of liquids */

#define LIQ_WATER	    0
#define LIQ_ALE         1
#define LIQ_BEER        2
#define LIQ_CIDER	    3
#define LIQ_MEAD	    4
#define LIQ_WINE	    5
#define LIQ_BRANDY	    6
#define LIQ_SOUP	    7
#define LIQ_MILK	    8
#define LIQ_TEA		    9
#define LIQ_SALTWATER	10

/* for containers  - value[1] */

#define CONT_CLOSEABLE  ( 1 << 0 )
#define CONT_PICKPROOF  ( 1 << 1 )
#define CONT_CLOSED     ( 1 << 2 )
#define CONT_LOCKED     ( 1 << 3 )
#define CONT_TRAP       ( 1 << 4 )
#define CONT_BEHEADED	( 1 << 5 )

/* harsh land object flags */

#define HO_CLONE	(1<<0)
#define HO_LIMIT   	(1<<1)

#define OBJ_NOTIMER	-7000000
#define NOWHERE    	-1

/* Bitvector for obj 'tmp_flags' */

#define SA_DROPPED	( 1 << 0 )

/* Bitvector For 'room_flags' */

#define DARK       	( 1 << 0 )
#define RUINS      	( 1 << 1 )
#define NO_MOB     	( 1 << 2 )
#define INDOORS    	( 1 << 3 )
#define LAWFUL     	( 1 << 4 )
#define NO_MAGIC   	( 1 << 5 )
#define TUNNEL     	( 1 << 6 )
#define CAVE       	( 1 << 7 )
#define SAFE_Q     	( 1 << 8 )
#define DEEP	   	( 1 << 9 )
#define FALL		( 1 << 10 )
#define NO_MOUNT	( 1 << 11 )
#define	VEHICLE		( 1 << 12 )
#define STIFLING_FOG	( 1 << 13 )
#define NO_MERCHANT	( 1 << 14 )
#define CLIMB		( 1 << 15 )
#define SAVE		( 1 << 16 )
#define LAB		( 1 << 17 )
#define ROAD		( 1 << 18 )
#define WEALTHY		( 1 << 19 )
#define POOR		( 1 << 20 )
#define SCUM		( 1 << 21 )
#define TEMPORARY	( 1 << 22 )
#define ARENA		( 1 << 23 )
#define DOCK		( 1 << 24 )
#define WILD		( 1 << 25 )
#define LIGHT		( 1 << 26 )
#define NOHIDE		( 1 << 27 )
#define STORAGE		( 1 << 28 )
#define PC_ENTERED	( 1 << 29 )
#define ROOM_OK		( 1 << 30 )
#define OOC		( 1 << 31 )

/* For 'dir_option' */

#define NORTH		0
#define EAST		1
#define SOUTH		2
#define WEST		3
#define UP		4
#define DOWN		5
#define OUTSIDE		6
#define INSIDE		7

#define LAST_DIR	INSIDE

/* exit_info */

#define EX_ISDOOR       ( 1 << 0 )
#define EX_CLOSED	    ( 1 << 1 )
#define EX_LOCKED	    ( 1 << 2 )
#define EX_RSCLOSED	    ( 1 << 3 )
#define EX_RSLOCKED	    ( 1 << 4 )
#define EX_PICKPROOF	( 1 << 5 )
#define EX_SECRET   	( 1 << 6 )
#define EX_TRAP	    	( 1 << 7 )
#define EX_TOLL			( 1 << 8 )
#define EX_ISGATE       ( 1 << 9 )

/* For 'Sector types' */

#define SECT_INSIDE	    0
#define SECT_CITY      	    1
#define SECT_ROAD      	    2
#define SECT_TRAIL	    3
#define SECT_FIELD	    4
#define SECT_WOODS	    5
#define SECT_FOREST	    6
#define SECT_HILLS	    7
#define SECT_MOUNTAIN  	    8
#define SECT_SWAMP     	    9
#define SECT_DOCK	    10
#define SECT_CAVE	    11
#define SECT_PASTURE	    12
#define SECT_HEATH	    13
#define SECT_PIT	    14
#define SECT_LEANTO	    15
#define SECT_LAKE	    16
#define SECT_RIVER	    17
#define SECT_OCEAN	    18
#define SECT_REEF	    19
#define SECT_UNDERWATER	    20

/* ---- For new herb stuff ---- */

/* Herb sectors */

#define HERB_NUMSECTORS		9

#define HERB_FIELD		0
#define HERB_PASTURE		1
#define HERB_WOODS		2
#define HERB_FOREST		3
#define HERB_MOUNTAINS		4
#define HERB_SWAMP		5
#define HERB_HEATH		6
#define HERB_HILLS		7
#define HERB_WATERNOSWIM	8

#define HERB_RARITIES		5

#define MAX_HERBS_PER_ROOM	3

#define HERB_RESET_DURATION	24

/* For 'equip' */

#define WEAR_LIGHT		0
#define WEAR_FINGER_R		1
#define WEAR_FINGER_L		2
#define WEAR_NECK_1		3
#define WEAR_NECK_2		4
#define WEAR_BODY		5
#define WEAR_HEAD		6
#define WEAR_LEGS		7
#define WEAR_FEET		8
#define WEAR_HANDS		9
#define WEAR_ARMS		10
#define WEAR_SHIELD		11
#define WEAR_ABOUT		12
#define WEAR_WAIST		13
#define WEAR_WRIST_R		14
#define WEAR_WRIST_L		15
#define WEAR_PRIM		16
#define WEAR_SEC		17
#define WEAR_BOTH		18
#define WEAR_UNUSED_1		19
#define WEAR_BELT_1		20
#define WEAR_BELT_2		21
#define WEAR_BACK		22
#define WEAR_BLINDFOLD		23
#define WEAR_THROAT		24
#define WEAR_EAR		25
#define WEAR_SHOULDER_R		26
#define WEAR_SHOULDER_L		27
#define WEAR_ANKLE_R		28
#define WEAR_ANKLE_L		29
#define WEAR_HAIR		30
#define WEAR_FACE		31
#define WEAR_CARRY_R		32
#define WEAR_CARRY_L		33
#define WEAR_ARMBAND_R		34
#define WEAR_ARMBAND_L		35

#define MAX_WEAR        	36

#define MAX_SKILLS		150
#define MAX_SPELLS      	100
#define MAX_AFFECT		25


/* conditions */

#define DRUNK			0
#define FULL			1
#define THIRST			2

/* Bitvector for 'affected_by' */

#define AFF_UNDEF1              ( 1 << 0 )
#define AFF_INVISIBLE	        ( 1 << 1 )
#define AFF_INFRAVIS        	( 1 << 2 )
#define AFF_DETECT_INVISIBLE 	( 1 << 3 )
#define AFF_DETECT_MAGIC     	( 1 << 4 )
#define AFF_SENSE_LIFE       	( 1 << 5 )
/* define AFF_HOLD???		( 1 << 6 ) ????? */
#define AFF_SANCTUARY		( 1 << 7 )	/* 7. */
#define AFF_GROUP            	( 1 << 8 )
#define AFF_CURSE            	( 1 << 9 )
#define AFF_FLAMING          	( 1 << 10 )
#define AFF_POISON           	( 1 << 11 )
#define AFF_SCAN		( 1 << 12 )
/* define AFF_PARALYSIS???	( 1 << 13 ) ????? */
/* define AFF_???		( 1 << 14 ) ????? */
/* define AFF_???		( 1 << 15 ) ????? */
/* define AFF_SLEEP???		( 1 << 16 ) ????? */
/* define AFF_DODGE???		( 1 << 17 ) ????? */
#define AFF_SNEAK	        ( 1 << 18 )	/* 18. */
#define AFF_HIDE		( 1 << 19 )	/* 19. */
/* define AFF_FEAR???		( 1 << 20 ) ????? */
#define AFF_FOLLOW           	( 1 << 21 )	/* 21. MAY CONFLICT WITH CHARM ? */
#define AFF_HOODED              ( 1 << 22 )	/* 22. */
/* define AFF_CHARM???		( 1 << 23 ) ????? */
#define AFF_SUNLIGHT_PEN	( 1 << 23 )
#define AFF_SUNLIGHT_PET	( 1 << 24 )
#define AFF_BREATHE_WATER	( 1 << 25 )

/* modifiers to char's abilities */

#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT		    3
#define APPLY_CHA		    4
#define APPLY_AUR		    5
#define APPLY_WIL           6
#define APPLY_CON           7
#define APPLY_SEX           8
#define APPLY_AGE           9
#define APPLY_CHAR_WEIGHT	10
#define APPLY_CHAR_HEIGHT	11
#define APPLY_DEFENSE		12	/* Free - APPLY_DEFENSE not used */
#define APPLY_HIT		    13
#define APPLY_MOVE		    14
#define APPLY_CASH		    15
#define APPLY_AC		    16
#define APPLY_ARMOR		    16
#define APPLY_OFFENSE		17	/* Free - APPLY_OFFENSE not used */
#define APPLY_DAMROLL		18
#define APPLY_SAVING_PARA	19
#define APPLY_SAVING_ROD	20
#define APPLY_SAVING_PETRI	21
#define APPLY_SAVING_BREATH	22
#define APPLY_SAVING_SPELL	23
#define APPLY_AGI			24

/* Above 100, don't reapply upon restore of character. */

#define APPLY_BRAWLING		123
#define APPLY_CLUB		    124
#define APPLY_SPEAR		    125
#define APPLY_SWORD		    126
#define APPLY_DAGGER		127
#define APPLY_AXE		    128
#define APPLY_WHIP		    129
#define APPLY_POLEARM		130
#define APPLY_DUAL		    131
#define APPLY_BLOCK		    132
#define APPLY_PARRY		    133
#define APPLY_SUBDUE		134
#define APPLY_DISARM		135
#define APPLY_SNEAK		    136
#define APPLY_HIDE 		    137
#define APPLY_STEAL		    138
#define APPLY_PICK		    139
#define APPLY_SEARCH		140
#define APPLY_LISTEN		141
#define APPLY_FORAGE		142
#define APPLY_RITUAL		143
#define APPLY_SCAN		    144
#define APPLY_BACKSTAB 		145
#define APPLY_BARTER		146
#define APPLY_RIDE		    147
#define APPLY_CLIMB		    148
#define APPLY_PEEP		    149	/* Obsoleted */
#define APPLY_HUNT		    150
#define APPLY_SKIN		    151
#define APPLY_SAIL		    152
#define APPLY_POISONING		153
#define APPLY_ALCHEMY		154
#define APPLY_HERBALISM		155

#define FIRST_APPLY_SKILL	APPLY_BRAWLING
#define LAST_APPLY_SKILL 	APPLY_HERBALISM

/* NOTE:  Change affect_modify in handler.c if new APPLY's are added */

/* sex */

#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */

#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_UNCONSCIOUS 2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8

/* for mobile actions: specials.act */

#define ACT_MEMORY	    ( 1 << 0 )
#define ACT_SENTINEL    ( 1 << 1 )
#define ACT_SCAVENGER   ( 1 << 2 )
#define ACT_NOCOMMAND   ( 1 << 2 )
#define ACT_ISNPC       ( 1 << 3 )
#define ACT_NOVNPC	    ( 1 << 4 )	/* Shopkeep doesn't have vNPC buyers */
#define ACT_AGGRESSIVE 	( 1 << 5 )
#define ACT_STAY_ZONE  	( 1 << 6 )
#define ACT_WIMPY     	( 1 << 7 )
#define ACT_PURSUE		( 1 << 8 )
#define ACT_SOLDIER     ( 1 << 9 )  /* Thief converted to Soldier - Case */
#define ACT_NOORDER	    ( 1 << 10 )
#define ACT_NOBUY	    ( 1 << 11 )
#define ACT_ENFORCER	( 1 << 12 )
#define ACT_STEALTHY	( 1 << 13 )
#define ACT_VEHICLE	    ( 1 << 14 )
#define ACT_STOP	    ( 1 << 15 )	/* Lines up with PLR_STOP */
#define ACT_CRIMINAL	( 1 << 16 )
#define ACT_PARIAH	    ( 1 << 17 )
#define ACT_MOUNT		( 1 << 18 )
#define ACT_DONTUSE     ( 1 << 19 )	/* Was AUTOFLEE, use FLAG_AUTOFLEE */
#define ACT_PCOWNED		( 1 << 20 )
#define ACT_WILDLIFE	( 1 << 21 )	/* Wildlife doesn't attack wildlife */
#define ACT_STAYPUT		( 1 << 22 )	/* Saves between reboots */
#define ACT_PASSIVE		( 1 << 23 )	/* Won't assist clan members */
#define ACT_AUCTIONEER 	( 1 << 24 ) /* Mobile is an NPC auctioneer - auctions.cpp */
#define ACT_ECONZONE	( 1 << 25 )	/* NPC uses econ zone discount/markups */
#define ACT_JAILER	( 1 << 26 )	/* New jailer flag for NPCs */
#define ACT_NOBIND	( 1 << 27 )
#define ACT_NOBLEED	( 1 << 28 )
#define ACT_FLYING	( 1 << 29 )
#define ACT_PHYSICIAN	( 1 << 30 )
#define ACT_PREY	( 1 << 31 )	/* Animals may only be ambushed or hit with ranged weapons */

/* For players : specials.act */

#define PLR_QUIET	    ( 1 << 19 )	/* WAS ( 1 << 4 ) - conflicts with ACT_NOVNPC */
#define PLR_STOP	    ( 1 << 15 )

/* harsh land specific mobile flags */

#define HM_CLONE	(1<<0)
#define HM_KEEPER  	(1<<1)

/* Target classes: */

#define TAR_AREA            0
#define TAR_CHAR_OFFENSIVE  1
#define TAR_CHAR_DEFENSIVE  2
#define TAR_CHAR_SELF       3
#define TAR_OBJ             4
#define TAR_OBJ_INV         5
#define TAR_CHAR_SECRET     6
#define TAR_IGNORE          7

/* Begin magic defines */

#define MAX_LEARNED_SPELLS		100

#define CASTER_NONE			0
#define CASTER_WHITE			1
#define CASTER_GREY			2
#define CASTER_BLACK			3

#define SPHERE_EARTH			0
#define SPHERE_AIR			1
#define SPHERE_FIRE			2
#define SPHERE_WATER			3
#define SPHERE_LIGHT			4
#define SPHERE_SHADOW			5

/* Defines for wound-type manifestation of spell damage */

#define MAGIC_DAMAGE_SLASH			0
#define MAGIC_DAMAGE_BLUNT			1
#define MAGIC_DAMAGE_PUNCTURE			2
#define MAGIC_DAMAGE_BURN			3
#define MAGIC_DAMAGE_FROST			4

/* Defines for save/defensive methods against spells */

#define SAVE_NONE			0
#define SAVE_EVADE			1
#define SAVE_MAGIC_RESIST		2
#define SAVE_AGI			3
#define SAVE_DEX			4
#define SAVE_INTEL			5
#define SAVE_WIL			6
#define SAVE_AUR			7
#define SAVE_CON			8
#define SAVE_STR			9

/* Defines for what a successful save will result in */

#define SAVE_FOR_NO_EFFECT		0
#define SAVE_FOR_HALF_EFFECT		1
#define SAVE_FOR_THIRD_EFFECT		2
#define SAVE_FOR_QUARTER_EFFECT		3

/* Target types used in spell definitions */

#define TARG_SELF			0
#define TARG_OBJ			1
#define TARG_OTHER			2
#define TARG_OTHER_HOSTILE		3
#define TARG_SELF_OTHER			4
#define TARG_SELF_OTHER_HOSTILE		5
#define TARG_GROUP			6
#define TARG_GROUP_HOSTILE		7
#define TARG_ROOM			8
#define TARG_ROOM_HOSTILE		9

/* Target types trackable via CAST */

#define TARGET_SELF             0
#define TARGET_OTHER            1
#define TARGET_OBJ              2
#define TARGET_ROOM             3
#define TARGET_OTHER_INVENTORY  4
#define TARGET_EXIT		5
#define TARGET_REMOTE_ROOM	6
#define TARGET_REMOTE_OTHER	7

/* See MAGIC_AFFECT_X for various specific effects */

/* End magic defines */

#define JOB_1					600
#define JOB_2					601
#define JOB_3					602

/* Affect for toggling listen/mute */
#define MUTE_EAVESDROP				620

/* Affect for counting the number of times a room is herbed */
#define HERBED_COUNT				621

#define CRAFT_FIRST_OLD				650	/* Crafts can be assigned in the */
#define CRAFT_LAST_OLD				899	/*  range CRAFT_FIRST..CRAFT_LAST */

#define AFFECT_SHADOW			900	/* Affected n/pc shadows another n/pc */
#define AFFECT_GUARD_DIR		950	/* Affected n/pc shadows another n/pc */

#define AFFECT_LOST_CON			980

#define AFFECT_HOLDING_BREATH		990	/* Swimming underwater! */

#define MAGIC_CRIM_BASE		    1000	/* Criminal tags reservd from 1000..1100 */
#define MAGIC_CRIM_RESERVED	    1100	/* 1000..1100 are reserved */

#define MAGIC_CLAN_MEMBER_BASE		1200	/* Clan tags reserved from 1200..1299 */
#define MAGIC_CLAN_LEADER_BASE		1300	/* Clan tags reserved from 1300..1399 */
#define MAGIC_CLAN_OMNI_BASE		1400	/* Clan tags reserved from 1400..1499 */

#define MAGIC_SKILL_GAIN_STOP		1500	/* Skill use spam 1500..1500+MAX_SKILLS */
#define MAGIC_SPELL_GAIN_STOP		1800
#define MAGIC_CRAFT_BRANCH_STOP		2000
#define MAGIC_CRAFT_DELAY		2010

#define MAGIC_HIDDEN			2011
#define MAGIC_SNEAK			2012
#define MAGIC_NOTIFY			2013
#define MAGIC_CLAN_NOTIFY		2014
#define MAGIC_TOLL			2015
#define MAGIC_TOLL_PAID			2016
#define MAGIC_DRAGGER			2017
#define MAGIC_GUARD			2018
#define MAGIC_WATCH1			2019
#define MAGIC_WATCH2			2020
#define MAGIC_WATCH3			2021
#define MAGIC_STAFF_SUMMON		2022
#define AFFECT_GROUP_RETREAT		2023

#define AFFECT_COVER_NORTH		2100	//protect from north
#define AFFECT_COVER_EAST		2101
#define AFFECT_COVER_SOUTH		2102
#define AFFECT_COVER_WEST		2103
#define AFFECT_COVER_UP			2104
#define AFFECT_COVER_DOWN		2105

#define MAGIC_CRIM_HOODED		2600	/* Hooded criminal observed in zone .. */

#define MAGIC_STARED			2700	/* Don't stare again until this expires */

#define MAGIC_SKILL_MOD_FIRST	3000	/* Reserve 200 for skill mod affects */
#define MAGIC_SKILL_MOD_LAST	3200

#define MAGIC_SENT			3250	/* Used with the emote system. */

#define MAGIC_SMELL_FIRST		3300
#define MAGIC_AKLASH_ODOR		3300
#define MAGIC_ROSE_SCENT		3301
#define MAGIC_JASMINE_SCENT		3302
#define MAGIC_SEWER_STENCH		3303
#define MAGIC_SOAP_AROMA		3304
#define MAGIC_CINNAMON_SCENT		3305
#define MAGIC_LEORTEVALD_STENCH		3306
#define MAGIC_YULPRIS_ODOR		3307
#define MAGIC_FRESH_BREAD		3308
#define MAGIC_MOWN_HAY			3309
#define MAGIC_FRESH_LINEN		3310
#define MAGIC_INCENSE_SMOKE		3311
#define MAGIC_WOOD_SMOKE		3312
#define MAGIC_SMELL_LAST		3399

#define MAGIC_AFFECT_FIRST			3400

#define MAGIC_AFFECT_DMG			3400
#define MAGIC_AFFECT_HEAL			3401
#define MAGIC_AFFECT_PROJECTILE_IMMUNITY	3402
#define MAGIC_AFFECT_INFRAVISION		3403
#define MAGIC_AFFECT_CONCEALMENT		3404
#define MAGIC_AFFECT_INVISIBILITY		3405
#define MAGIC_AFFECT_SEE_INVISIBLE		3406
#define MAGIC_AFFECT_SENSE_LIFE			3407
#define MAGIC_AFFECT_TONGUES			3408
#define MAGIC_AFFECT_LEVITATE			3409
#define MAGIC_AFFECT_SLOW			3410
#define MAGIC_AFFECT_SPEED			3411
#define MAGIC_AFFECT_SLEEP			3412
#define MAGIC_AFFECT_PARALYSIS			3413
#define MAGIC_AFFECT_FEAR			3414
#define MAGIC_AFFECT_REGENERATION		3415
#define MAGIC_AFFECT_CURSE			3416
#define MAGIC_AFFECT_DIZZINESS			3417
#define MAGIC_AFFECT_FURY			3418
#define MAGIC_AFFECT_INVULNERABILITY		3419
#define MAGIC_AFFECT_ARMOR			3420
#define MAGIC_AFFECT_BLESS			3421

#define MAGIC_AFFECT_LAST			4999

#define MAGIC_ROOM_CALM			5000
#define MAGIC_ROOM_LIGHT		5001
#define MAGIC_ROOM_DARK			5002
#define MAGIC_ROOM_DEBUG		5003
#define MAGIC_ROOM_FLOOD		5004
#define MAGIC_WORLD_CLOUDS		5005	/* Blocks the sun */
#define MAGIC_WORLD_SOLAR_FLARE	5006	/* Creates an artificial sun */
#define MAGIC_WORLD_MOON		5006	/* Moonlight in all rooms */

#define MAGIC_BUY_ITEM			5400

#define MAGIC_ROOM_FIGHT_NOISE		5500

#define MAGIC_PETITION_MESSAGE		5600

#define MAGIC_STABLING_PAID		5700
#define MAGIC_STABLING_LAST		5999

#define MAGIC_FLAG_NOGAIN		6000

#define MAGIC_WARNED			6500

#define MAGIC_RAISED_HOOD		6600

#define MAGIC_SIT_TABLE			6700	/* PC acquires this affect when at a table */

#define MAGIC_FIRST_POISON		7000
#define POISON_LETHARGY			7001
#define MAGIC_LAST_POISON		7001

#define CRAFT_FIRST			8000	/* Crafts can be assigned in the */
#define CRAFT_LAST			18000	/*  range CRAFT_FIRST..CRAFT_LAST */

#define MAGIC_FIRST_SOMA                20000	/* SOMATIC EFFECTS TBA */
#define SOMA_MUSCULAR_CRAMP             MAGIC_FIRST_SOMA + 1	/* pain echo */
#define SOMA_MUSCULAR_TWITCHING         MAGIC_FIRST_SOMA + 2
#define SOMA_MUSCULAR_TREMOR            MAGIC_FIRST_SOMA + 3
#define SOMA_MUSCULAR_PARALYSIS         MAGIC_FIRST_SOMA + 4
#define SOMA_DIGESTIVE_ULCER            MAGIC_FIRST_SOMA + 5	/* pain echo */
#define SOMA_DIGESTIVE_VOMITING         MAGIC_FIRST_SOMA + 6
#define SOMA_DIGESTIVE_BLEEDING         MAGIC_FIRST_SOMA + 7	/* vomiting blood */
#define SOMA_EYE_BLINDNESS              MAGIC_FIRST_SOMA + 8
#define SOMA_EYE_BLURRED                MAGIC_FIRST_SOMA + 9
#define SOMA_EYE_DOUBLE                 MAGIC_FIRST_SOMA + 10
#define SOMA_EYE_DILATION               MAGIC_FIRST_SOMA + 11	/* light sensitivity */
#define SOMA_EYE_CONTRACTION            MAGIC_FIRST_SOMA + 12	/* tunnel vision? */
#define SOMA_EYE_LACRIMATION            MAGIC_FIRST_SOMA + 13	/* watery eyes */
#define SOMA_EYE_PTOSIS                 MAGIC_FIRST_SOMA + 14	/* drooping lids */
#define SOMA_EAR_TINNITUS               MAGIC_FIRST_SOMA + 15	/* noise echo */
#define SOMA_EAR_DEAFNESS               MAGIC_FIRST_SOMA + 16
#define SOMA_EAR_EQUILLIBRIUM           MAGIC_FIRST_SOMA + 17	/* dizziness */
#define SOMA_NOSE_ANOSMIA               MAGIC_FIRST_SOMA + 18	/* ignore aroma effects */
#define SOMA_NOSE_RHINITIS              MAGIC_FIRST_SOMA + 19	/* itchy/runny nose */
#define SOMA_MOUTH_SALIVATION           MAGIC_FIRST_SOMA + 20	/* heh */
#define SOMA_MOUTH_TOOTHACHE            MAGIC_FIRST_SOMA + 21	/* pain echo / looseness */
#define SOMA_MOUTH_DRYNESS              MAGIC_FIRST_SOMA + 22	/* echo */
#define SOMA_MOUTH_HALITOSIS            MAGIC_FIRST_SOMA + 23
#define SOMA_CHEST_DIFFICULTY           MAGIC_FIRST_SOMA + 24	/* pain echo */
#define SOMA_CHEST_WHEEZING             MAGIC_FIRST_SOMA + 25
#define SOMA_CHEST_RAPIDBREATH          MAGIC_FIRST_SOMA + 26
#define SOMA_CHEST_SLOWBREATH           MAGIC_FIRST_SOMA + 27
#define SOMA_CHEST_FLUID                MAGIC_FIRST_SOMA + 28
#define SOMA_CHEST_PALPITATIONS         MAGIC_FIRST_SOMA + 29
#define SOMA_CHEST_COUGHING             MAGIC_FIRST_SOMA + 30
#define SOMA_CHEST_PNEUMONIA            MAGIC_FIRST_SOMA + 31
#define SOMA_NERVES_PSYCHOSIS           MAGIC_FIRST_SOMA + 32
#define SOMA_NERVES_DELIRIUM            MAGIC_FIRST_SOMA + 33
#define SOMA_NERVES_COMA                MAGIC_FIRST_SOMA + 34	/* depression & drowsiness too */
#define SOMA_NERVES_CONVULSIONS         MAGIC_FIRST_SOMA + 35
#define SOMA_NERVES_HEADACHE            MAGIC_FIRST_SOMA + 36
#define SOMA_NERVES_CONFUSION           MAGIC_FIRST_SOMA + 37	/* misdirection? ;) */
#define SOMA_NERVES_PARETHESIAS         MAGIC_FIRST_SOMA + 38	/* am i on fire? ;) */
#define SOMA_NERVES_ATAXIA              MAGIC_FIRST_SOMA + 39	/* --dex */
#define SOMA_NERVES_EQUILLIBRIUM        MAGIC_FIRST_SOMA + 40
#define SOMA_SKIN_CYANOSIS              MAGIC_FIRST_SOMA + 41
#define SOMA_SKIN_DRYNESS               MAGIC_FIRST_SOMA + 42
#define SOMA_SKIN_CORROSION             MAGIC_FIRST_SOMA + 43
#define SOMA_SKIN_JAUNDICE              MAGIC_FIRST_SOMA + 44
#define SOMA_SKIN_REDNESS               MAGIC_FIRST_SOMA + 45
#define SOMA_SKIN_RASH                  MAGIC_FIRST_SOMA + 46
#define SOMA_SKIN_HAIRLOSS              MAGIC_FIRST_SOMA + 47
#define SOMA_SKIN_EDEMA                 MAGIC_FIRST_SOMA + 48
#define SOMA_SKIN_BURNS                 MAGIC_FIRST_SOMA + 49
#define SOMA_SKIN_PALLOR                MAGIC_FIRST_SOMA + 50
#define SOMA_SKIN_SWEATING              MAGIC_FIRST_SOMA + 51
#define SOMA_GENERAL_WEIGHTLOSS         MAGIC_FIRST_SOMA + 52
#define SOMA_GENERAL_LETHARGY           MAGIC_FIRST_SOMA + 53
#define SOMA_GENERAL_APPETITELOSS       MAGIC_FIRST_SOMA + 54
#define SOMA_GENERAL_PRESSUREDROP       MAGIC_FIRST_SOMA + 55
#define SOMA_GENERAL_PRESSURERISE       MAGIC_FIRST_SOMA + 56
#define SOMA_GENERAL_FASTPULSE          MAGIC_FIRST_SOMA + 57
#define SOMA_GENERAL_SLOWPULSE          MAGIC_FIRST_SOMA + 58
#define SOMA_GENERAL_HYPERTHERMIA       MAGIC_FIRST_SOMA + 59
#define SOMA_GENERAL_HYPOTHERMIA        MAGIC_FIRST_SOMA + 60
#define SOMA_BREAK_ARM_L			    MAGIC_FIRST_SOMA + 61
#define SOMA_BREAK_ARM_R			    MAGIC_FIRST_SOMA + 62
#define SOMA_BREAK_LEG_L			    MAGIC_FIRST_SOMA + 63
#define SOMA_BREAK_LEG_R			    MAGIC_FIRST_SOMA + 64
#define SOMA_BREAK_HAND_R				MAGIC_FIRST_SOMA + 65
#define SOMA_BREAK_HAND_L				MAGIC_FIRST_SOMA + 66
#define SOMA_BREAK_FOOT_R				MAGIC_FIRST_SOMA + 67
#define SOMA_BREAK_FOOT_L				MAGIC_FIRST_SOMA + 68
#define SOMA_BREAK_COLLARBONE			MAGIC_FIRST_SOMA + 69
#define SOMA_BREAK_RIB					MAGIC_FIRST_SOMA + 70
#define SOMA_BREAK_HIP_L				MAGIC_FIRST_SOMA + 71
#define SOMA_BREAK_HIP_R				MAGIC_FIRST_SOMA + 72
#define SOMA_BREAK_SKULL				MAGIC_FIRST_SOMA + 73
#define SOMA_BREAK_SHOULDER_L			MAGIC_FIRST_SOMA + 74
#define SOMA_BREAK_SHOULDER_R			MAGIC_FIRST_SOMA + 75
#define SOMA_BREAK_NECK					MAGIC_FIRST_SOMA + 76
#define SOMA_BREAK_BACK					MAGIC_FIRST_SOMA + 77
#define SOMA_BREAK_JAW					MAGIC_FIRST_SOMA + 78
#define SOMA_BREAK_NOSE					MAGIC_FIRST_SOMA + 79
#define SOMA_BREAK_EYE_L				MAGIC_FIRST_SOMA + 80
#define SOMA_BREAK_EYE_R				MAGIC_FIRST_SOMA + 81
#define SOMA_SPRAIN_ARM_L			    MAGIC_FIRST_SOMA + 82
#define SOMA_SPRAIN_ARM_R			    MAGIC_FIRST_SOMA + 83
#define SOMA_SPRAIN_LEG_L			    MAGIC_FIRST_SOMA + 84
#define SOMA_SPRAIN_LEG_R			    MAGIC_FIRST_SOMA + 85
#define SOMA_SPRAIN_HAND_R				MAGIC_FIRST_SOMA + 86
#define SOMA_SPRAIN_HAND_L				MAGIC_FIRST_SOMA + 87
#define SOMA_SPRAIN_FOOT_R				MAGIC_FIRST_SOMA + 88
#define SOMA_SPRAIN_FOOT_L				MAGIC_FIRST_SOMA + 89
#define SOMA_SPRAIN_COLLARBONE			MAGIC_FIRST_SOMA + 90
#define SOMA_SPRAIN_RIB					MAGIC_FIRST_SOMA + 91
#define SOMA_SPRAIN_HIP_L				MAGIC_FIRST_SOMA + 92
#define SOMA_SPRAIN_HIP_R				MAGIC_FIRST_SOMA + 93
#define SOMA_SPRAIN_SHOULDER_L			MAGIC_FIRST_SOMA + 94
#define SOMA_SPRAIN_SHOULDER_R			MAGIC_FIRST_SOMA + 95
#define SOMA_SPRAIN_NECK				MAGIC_FIRST_SOMA + 96
#define SOMA_SPRAIN_BACK				MAGIC_FIRST_SOMA + 97
#define SOMA_SPRAIN_JAW					MAGIC_FIRST_SOMA + 98
#define SOMA_BLACK_EYE_L				MAGIC_FIRST_SOMA + 99
#define SOMA_BLACK_EYE_R				MAGIC_FIRST_SOMA + 100
#define SOMA_SEVERED_ARM_L			    MAGIC_FIRST_SOMA + 101
#define SOMA_SEVERED_ARM_R			    MAGIC_FIRST_SOMA + 102
#define SOMA_SEVERED_LEG_L			    MAGIC_FIRST_SOMA + 103
#define SOMA_SEVERED_LEG_R			    MAGIC_FIRST_SOMA + 104
#define SOMA_SEVERED_HAND_R				MAGIC_FIRST_SOMA + 105
#define SOMA_SEVERED_HAND_L				MAGIC_FIRST_SOMA + 106
#define SOMA_SEVERED_FOOT_R				MAGIC_FIRST_SOMA + 107
#define SOMA_SEVERED_FOOT_L				MAGIC_FIRST_SOMA + 108
#define MAGIC_LAST_SOMA                 MAGIC_FIRST_SOMA + 108

#define TYPE_UNDEFINED		    -1

#define TYPE_SUFFERING		    200	/* KILLER CDR: Eliminate this line somehow */

#define SKILL_CEILING		80	// Top limit that skill checks are made against.

/* skills */

#define SKILL_DEFENSE		-3	/* Special cased; a pseudo skill */
#define SKILL_OFFENSE		-2	/* Special cased; a pseudo skill */
#define SKILL_BRAWLING		1
#define SKILL_LIGHT_EDGE	2
#define SKILL_MEDIUM_EDGE	3
#define SKILL_HEAVY_EDGE	4
#define SKILL_LIGHT_BLUNT	5
#define SKILL_MEDIUM_BLUNT	6
#define SKILL_HEAVY_BLUNT	7
#define SKILL_LIGHT_PIERCE	8
#define SKILL_MEDIUM_PIERCE	9
#define SKILL_HEAVY_PIERCE	10
#define SKILL_STAFF		11
#define SKILL_POLEARM		12
#define SKILL_THROWN		13
#define SKILL_BLOWGUN		14
#define SKILL_SLING		15
#define SKILL_SHORTBOW		16
#define SKILL_LONGBOW		17
#define SKILL_CROSSBOW		18
#define SKILL_DUAL          	19
#define SKILL_BLOCK         	20
#define SKILL_PARRY         	21
#define SKILL_SUBDUE        	22
#define SKILL_DISARM        	23 //not used
#define SKILL_SNEAK         	24
#define SKILL_HIDE          	25
#define SKILL_STEAL         	26
#define SKILL_PICK          	27
#define SKILL_SEARCH        	28
#define SKILL_LISTEN        	29
#define SKILL_FORAGE        	30
#define SKILL_RITUAL        	31
#define SKILL_SCAN          	32
#define SKILL_BACKSTAB      	33
#define SKILL_BARTER       	34
#define SKILL_RIDE		35
#define SKILL_CLIMB		36
#define SKILL_SWIMMING		37
#define SKILL_HUNT		38
#define SKILL_SKIN		39
#define SKILL_SAIL		40
#define SKILL_POISONING		41
#define SKILL_ALCHEMY		42
#define SKILL_HERBALISM		43
#define SKILL_CLAIRVOYANCE	44
#define SKILL_DANGER_SENSE	45
#define SKILL_EMPATHIC_HEAL	46
#define SKILL_HEX		47
#define SKILL_MENTAL_BOLT	48
#define SKILL_PRESCIENCE	49
#define SKILL_SENSITIVITY	50
#define SKILL_TELEPATHY		51
#define SKILL_SEAFARING		52
#define SKILL_DODGE		53
#define SKILL_TAME		54
#define SKILL_BREAK		55
#define SKILL_METALCRAFT	56
#define SKILL_WOODCRAFT		57
#define SKILL_TEXTILECRAFT	58
#define SKILL_COOKERY		59
#define SKILL_BAKING		60
#define SKILL_HIDEWORKING	61
#define SKILL_STONECRAFT	62
#define SKILL_CANDLERY		63
#define SKILL_BREWING		64
#define SKILL_DISTILLING	65
#define SKILL_LITERACY		66
#define SKILL_DYECRAFT		67
#define SKILL_APOTHECARY	68
#define SKILL_GLASSWORK		69
#define SKILL_GEMCRAFT		70
#define SKILL_MILLING		71
#define SKILL_MINING		72
#define SKILL_PERFUMERY		73
#define SKILL_POTTERY		74
#define SKILL_TRACKING		75
#define SKILL_FARMING		76
#define SKILL_HEALING		77
#define SKILL_SPEAK_ATLIDUK     78
#define SKILL_SPEAK_ADUNAIC     79
#define SKILL_SPEAK_HARADAIC    80
#define SKILL_SPEAK_WESTRON     81
#define SKILL_SPEAK_DUNAEL      82
#define SKILL_SPEAK_LABBA       83
#define SKILL_SPEAK_NORLIDUK    84
#define SKILL_SPEAK_ROHIRRIC    85
#define SKILL_SPEAK_TALATHIC	86
#define SKILL_SPEAK_UMITIC		87
#define SKILL_SPEAK_NAHAIDUK		88
#define SKILL_SPEAK_PUKAEL		89
#define SKILL_SPEAK_SINDARIN		90
#define SKILL_SPEAK_QUENYA		91
#define SKILL_SPEAK_SILVAN		92
#define SKILL_SPEAK_KHUZDUL	    	93
#define SKILL_SPEAK_ORKISH		94
#define SKILL_SPEAK_BLACK_SPEECH	95
#define SKILL_SCRIPT_SARATI		96
#define SKILL_SCRIPT_TENGWAR		97
#define SKILL_SCRIPT_BELERIAND_TENGWAR	98
#define SKILL_SCRIPT_CERTHAS_DAERON	99
#define SKILL_SCRIPT_ANGERTHAS_DAERON	100
#define SKILL_SCRIPT_QUENYAN_TENGWAR	101
#define SKILL_SCRIPT_ANGERTHAS_MORIA	102
#define SKILL_SCRIPT_GONDORIAN_TENGWAR	103
#define SKILL_SCRIPT_ARNORIAN_TENGWAR	104
#define SKILL_SCRIPT_NUMENIAN_TENGWAR	105
#define SKILL_SCRIPT_NORTHERN_TENGWAR	106
#define SKILL_SCRIPT_ANGERTHAS_EREBOR	107
#define SKILL_BLACK_WISE		108
#define SKILL_GREY_WISE			109
#define SKILL_WHITE_WISE		110
#define SKILL_RUNECASTING		111
#define SKILL_GAMBLING			112
#define SKILL_BONECARVING		113
#define SKILL_GARDENING			114
#define SKILL_SLEIGHT			115
#define SKILL_ASTRONOMY			116
#define LAST_SKILL			SKILL_ASTRONOMY

#define PSIONIC_TALENTS			8

/* How much light is in the land ? */

#define SUN_DARK		0
#define SUN_RISE		1
#define SUN_LIGHT		2
#define SUN_SET			3
#define MOON_RISE		4
#define MOON_SET		5

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY   	1
#define SKY_RAINING  	2
#define SKY_LIGHTNING	3
#define SKY_STORMY		4
#define SKY_FOGGY 		5

#define MAX_OBJ_SAVE	15

/* Delay types */

#define DEL_PICK		1
#define DEL_SEARCH		2
#define DEL_RITUAL		3
#define DEL_BACKSTAB		4
#define	DEL_SPELL		5
#define DEL_WORSHIP		6
#define DEL_FORAGE_SEEK		7
#define DEL_JOIN_FAITH		8
#define DEL_APP_APPROVE		9
#define DEL_CAST		10
/* Fyren start */
/* ??#define DEL_SKIN		11 */
#define DEL_SKIN_1		4002
#define DEL_SKIN_2		4003
#define DEL_SKIN_3		4004
/* Fyren end */
#define DEL_COUNT_COIN		12
#define DEL_IDENTIFY		13
#define DEL_GATHER		14
#define DEL_COMBINE		15
#define DEL_WHAP		16
#define DEL_GET_ALL		17
#define DEL_AWAKEN		18
#define DEL_EMPATHIC_HEAL	19
#define DEL_MENTAL_BOLT		20
#define DEL_ALERT		21
#define DEL_INVITE		22
#define DEL_CAMP1		23
#define DEL_CAMP2		24
#define DEL_CAMP3		25
#define DEL_CAMP4		26
#define DEL_COVER		27
#define DEL_TAKE		28
#define DEL_PUTCHAR		29
#define DEL_STARE		30
#define DEL_HIDE		31
#define DEL_SCAN		32
#define DEL_QUICK_SCAN		33
#define DEL_HIDE_OBJ		34
#define DEL_PICK_OBJ		35
#define DEL_COUNT_FARTHING	36
#define DEL_RUMMAGE		37
#define DEL_QUAFF		38
#define DEL_BIND_WOUNDS		39
#define DEL_TREAT_WOUND		40
#define DEL_LOAD_WEAPON		41
#define DEL_OOC			42
#define DEL_WEAVESIGHT		43
#define DEL_TRACK		44
#define DEL_FORAGE	        45
#define DEL_PITCH		46
#define DEL_PURCHASE_ITEM	47
#define DEL_WATER_REMOVE	48
#define DEL_ORDER_ITEM		49
#define DEL_QUICK_SCAN_DIAG	50
#define DEL_PLACE_AUCTION	51
#define DEL_PLACE_BID		52
#define DEL_PLACE_BUYOUT	53
#define DEL_CANCEL_AUCTION	54

/* Zone flags */

#define Z_FROZEN    1
#define Z_LOCKED    2

/* Projectile-related defines */

#define NUM_BODIES              1
#define MAX_HITLOC              7

struct body_info
{
	char part[AVG_STRING_LENGTH];
	int damage_mult;
	int damage_div;
	int percent;
	int wear_loc1;
	int wear_loc2;
};

#define RESULT_CS       1
#define RESULT_MS       2
#define RESULT_MF       3
#define RESULT_CF       4

#define GLANCING_HIT    1
#define HIT             2
#define CRITICAL_HIT    3
#define MISS            4
#define CRITICAL_MISS   5
#define SHIELD_BLOCK    7

/* Magic-related defines */

#define MAGNITUDE_SUBTLE        1
#define MAGNITUDE_GENTLE        2
#define MAGNITUDE_SOFT          3
#define MAGNITUDE_STOLID        4
#define MAGNITUDE_STAUNCH       5
#define MAGNITUDE_VIGOROUS      6
#define MAGNITUDE_VIBRANT       7
#define MAGNITUDE_POTENT        8
#define MAGNITUDE_POWERFUL      9
#define MAGNITUDE_DIRE          10

#define TECHNIQUE_CREATION              1
#define TECHNIQUE_DESTRUCTION           2
#define TECHNIQUE_TRANSFORMATION        3
#define TECHNIQUE_PERCEPTION            4
#define TECHNIQUE_CONTROL               5

#define FORM_ANIMAL             1
#define FORM_PLANT              2
#define FORM_IMAGE              3
#define FORM_LIGHT              4
#define FORM_DARKNESS           5
#define FORM_POWER              6
#define FORM_MIND               7
#define FORM_SPIRIT             8
#define FORM_AIR                9
#define FORM_EARTH              10
#define FORM_FIRE               11
#define FORM_WATER              12

extern void replaceString(char *&destination, const char *source);
extern char* duplicateString(const char *source);
extern int free_mem(char *&ptr);
extern int free_mem(void *ptr);

class forage_data {
public:
	long virt;
	int sector;
	struct forage_data *next;

	forage_data() {
		virt = 0;
		sector = 0;
		next = NULL;
	}
};

class extra_descr_data {
public:
	char *keyword;
	char *description;
	struct extra_descr_data *next;

	extra_descr_data() {
		keyword = NULL;
		description = NULL;
		next = NULL;
	}

	//~extra_descr_data() {
	//	free_mem(keyword);
	//	free_mem(description);
	//}
};

struct time_data {
	time_t birth;			/* This represents the characters age                */
	time_t logon;			/* Time of the last logon (used to calculate played) */
	long played;			/* This is the total accumulated time played in secs */
};

class writing_data {
public:
	char *message;
	char *author;
	char *date;
	char *ink;
	int language;
	int script;
	bool torn;
	int skill;
	WRITING_DATA *next_page;

	writing_data() {
		message = NULL;
		author = NULL;
		date = NULL;
		ink = NULL;
		language = -1;
		script = -1;
		torn = false;
		skill = -1;
		next_page = NULL;
	}

	//~writing_data() {
	//	free_mem(message);
	//	free_mem(author);
	//	free_mem(date);
	//	free_mem(ink);
	//}
};

class written_descr_data {	/* for descriptions written in languages */
public:
	byte language;
	char *description;

	written_descr_data() {
		language = '\0';
		description = NULL;
	}

	//~written_descr_data() {
	//	free_mem(description);
	//}
};

class obj_flag_data {
public:
	byte type_flag;
	bitflag wear_flags;
	int extra_flags;
	int weight;
	int set_cost; // set by an npc-shopkeeper or imm
	long bitvector;

	obj_flag_data() {
		type_flag = '\0';
		wear_flags = 0;
	}
};

struct armor_data
{
	int armor_value;
	int armor_type;
	int v2;
	int v3;
	int v4;

};

struct weapon_data
{
	int handedness;
	int dice;
	int sides;
	int use_skill;
	int hit_type;
	int delay;
	int basedamage;
	int attackclass;
	int defenseclass;
	int range;
};

struct light_data
{
	int capacity;
	int hours;
	int liquid;
	int on;
	int v4;
	int v5;
};

struct drink_con_data
{
	int capacity;
	int volume;
	int liquid;
	int spell_1;
	int spell_2;
	int spell_3;
};

struct fountain_data
{
	int capacity;
	int volume;
	int liquid;
	int spell_1;
	int spell_2;
	int spell_3;
};

struct container_data
{
	int capacity;
	int flags;
	int key;
	int pick_penalty;
	int v4;
	int table_max_sitting;
};

struct clan_container_data
{
	int capacity;
	int flags;
	int key;
	int pick_penalty;
	int v4;
	int v5;
};
struct cloak_data
{
	int v0;
	int v1;
	int v2;
	int v3;
	int v4;
};

struct ticket_data
{
	int ticket_num;
	int keeper_vnum;
	int stable_date;
	int v3;
	int v4;
	int v5;
};

struct perfume_data
{
	int type;
	int duration;
	int aroma_strength;
	int doses;
	int v5;
	int v6;
};

struct food_data
{
	int food_value;
	int spell1;
	int spell2;
	int spell3;
	int spell4;
	int bites;
};

struct poison_obj_data
{
	int poison1;
	int poison2;
	int poison3;
	int poison4;
	int poison5;
	int doses;
};

struct fluid_data
{
	int alcohol;
	int water;
	int food;
	int v3;
	int v4;
	int v5;
};

class lodged_object_info {
public:
	char *location;
	int vnum;
	LODGED_OBJECT_INFO *next;

	lodged_object_info() {
		location = NULL;
		vnum = 0;
		next = NULL;
	}

	//~lodged_object_info() {
	//	free_mem(location);
	//}
};

class lodged_missile_info {
public:
	int vnum;
	LODGED_MISSILE_INFO *next;

	lodged_missile_info() {
		vnum = 0;
		next = NULL;
	}
};

class wound_data {
public:
	char *location;
	char *type;
	char *name;
	char *severity;
	int damage;
	int bleeding;
	int poison;
	int infection;
	int healerskill;
	int bindskill;
	int lasthealed;
	int lastbled;
	int lastbound;
	WOUND_DATA *next;

	wound_data() {
		location = NULL;
		type = NULL;
		name = NULL;
		severity = NULL;
		damage = 0;
		bleeding = 0;
		poison = 0;
		infection = 0;
		healerskill = 0;
		bindskill = 0;
		lasthealed = 0;
		lastbled = 0;
		lastbound = 0;
		next = NULL;
	}

	//~wound_data() {
	//	free_mem(location);
	//	free_mem(type);
	//	free_mem(name);
	//	free_mem(severity);
	//}
};

struct repair_data
{
	unsigned short int nCraftMinutes;
	unsigned short int nUses;
	unsigned char nMendBonus;
	unsigned char nMinSkillNeeded;
	unsigned char nRequiresClan;
	unsigned char bAnyoneMayUse;
	unsigned int flgDamageTypes;
	unsigned int flgMaterialSkills;
	unsigned char arrItemType[4];
	unsigned int nUnused;
};

struct default_obj_data
{
	int value[6];
};

union obj_info
{
	struct weapon_data weapon;
	struct drink_con_data drinkcon;
	struct container_data container;
	struct default_obj_data od;
	struct light_data light;
	struct armor_data armor;
	struct cloak_data cloak;
	struct ticket_data ticket;
	struct perfume_data perfume;
	struct poison_obj_data poison;
	struct food_data food;
	struct fluid_data fluid;
	struct fountain_data fountain;
	struct repair_data repair;
	struct clan_container_data locker;
};

class obj_clan_data {
public:
	char *name;
	char *rank;
	OBJ_CLAN_DATA *next;

	obj_clan_data() {
		name = NULL;
		rank = NULL;
		next = NULL;
	}

	//~obj_clan_data() {
	//	free_mem(name);
	//	free_mem(rank);
	//}
};
/* ======================== Structure for object ========================= */
struct obj_data
{
	int deleted;
	int nVirtual;
	int zone;
	int in_room;
	int instances;		/* Proto-only field; keeps track of loaded instances */
	int order_time;		/* Proto-only field to track time required to order item */
	struct obj_flag_data obj_flags;
	union obj_info o;
	AFFECTED_TYPE *xaffected;
	char *name;
	char *description;
	char *short_description;
	char *full_description;
	char *omote_str;
	EXTRA_DESCR_DATA *ex_description;
	WRITTEN_DESCR_DATA *wdesc;
	CHAR_DATA *carried_by;
	CHAR_DATA *equiped_by;
	OBJ_DATA *in_obj;
	OBJ_DATA *contains;
	OBJ_DATA *next_content;
	OBJ_DATA *next;
	OBJ_DATA *hnext;
	OBJ_DATA *lnext;
	int clock;
	int morphTime;
	int morphto;
	int location;
	int contained_wt;
	int activation;		/* Affect applied when picked up */
	int quality;
	int econ_flags;		/* Flag means used enhanced prices */
	int size;
	int count;			/* How many this obj represents */
	int obj_timer;
	float farthings;		/* Partial value in farthings    */
	float silver;			/* Partial value in silver coins */
	WOUND_DATA *wounds;		/* For corpses */
	int item_wear;		/* Percentile; 100%, brand new */
	WRITING_DATA *writing;
	char *ink_color;
	unsigned int open;		// 0 closed, 1+ open/page number
	OBJ_DATA *loaded;
	LODGED_OBJECT_INFO *lodged;
	char *desc_keys;
	char *var_color;
	char *book_title;
	int title_skill;
	int title_language;
	int title_script;
	int material;
	int tmp_flags;
	bool writing_loaded;
	int coldload_id;
	OBJECT_DAMAGE *damage;
	char *indoor_desc;
	int sold_at;
	int sold_by;
	OBJ_CLAN_DATA *clan_data;
	int super_vnum;           // Vnum of parent class for object inheritance, e.g. a red brick fireplace inherits from 'fireplace' object.

	void deep_copy (OBJ_DATA *copy_from);
	void partial_deep_copy (OBJ_DATA *copy_from);
};


class room_direction_data {
public:
	char *general_description;
	char *keyword;
	int exit_info;
	int key;
	int pick_penalty;
	int to_room;

	room_direction_data() {
		general_description = NULL;
		keyword = NULL;
		exit_info = 0;
		key = 0;
		pick_penalty = 0;
		to_room = 0;
	}

	//~room_direction_data() {
	//	free_mem(general_description);
	//	free_mem(keyword);
	//}
};

#include "room.h"

enum mob_cue {
	cue_none,
	cue_notes,		// Freeform notes
	cue_flags,		// Flags (unimplemented)
	cue_memory,		// Var = Value
	cue_on_reboot,

	// Scheduled Cues
	cue_on_hour,		// (hr/sun/moon) reflex
	cue_on_time,		// (min/%) reflex

	// Personal Cues
	cue_on_health,	// (hp%) reflex
	cue_on_moves,		// (mp%) reflex

	// Socials
	cue_on_command,	// (command key (with rvnum)) reflex
	cue_on_receive,	// (vnum) reflex
	cue_on_hear,		// (phrase) reflex
	cue_on_nod,		// (nodder) reflex

	// Crime Cues
	cue_on_theft,
	cue_on_witness,	// (pick,steal,assault)

	// Combat Cues
	cue_on_engage,
	cue_on_flee,
	cue_on_scan,		// (specific enemy) reflex
	cue_on_hit		// reflex

};

#include "character.h"

class prog_vars {
public:
	char *name;
	int value;
	struct prog_vars *next;

	prog_vars() {
		name = NULL;
		value = 0;
		next = NULL;
	}

	//~prog_vars() {
	//	free_mem(name);
	//}
};

class room_prog {
public:
	char *command;		/* List of commands to trigger this program */
	char *keys;			/* List of valid arguments, NULL always executes */
	char *prog;			/* The program itself */
	struct prog_vars *vars;
	struct room_prog *next;	/* next program for this room */
	int type;			/* Only used for mprogs. 1 is personal, 0 is whole-room */

	room_prog() {
		command = NULL;
		keys = NULL;
		prog = NULL;
		vars = NULL;
		next = NULL;
		type = 0;
	}

	//~room_prog() {
	//	free_mem(command);
	//	free_mem(keys);
	//	free_mem(prog);
	//}
};

class secret {
public:
	int diff;			/* difficulty (search skill abil) */
	char *stext;

	secret() {
		diff = 0;
		stext = NULL;
	}

	//~secret() {
	//	free_mem(stext);
	//}
};

#define MAX_DELIVERIES			200
#define MAX_TRADES_IN			100 // DO NOT CHANGE THIS WITHOUT CHANGING THE FREAD FOR IT TO BE DYNAMIC. CURRENTLY DOES 10 ONLY - CASE

class negotiation_data {
public:
	int ch_coldload_id;
	int obj_vnum;
	int time_when_forgotten;
	int price_delta;
	int transactions;
	int true_if_buying;
	struct negotiation_data *next;

	negotiation_data() {
		ch_coldload_id = 0;
		obj_vnum = 0;
		time_when_forgotten = 0;
		price_delta = 0;
		transactions = 0;
		true_if_buying = 0;
		next = NULL;
	}
};

class shop_data {
public:
	float markup;			/* Objects sold are multiplied by this  */
	float discount;		/* Objects bought are muliplied by this */
	int shop_vnum;		/* Rvnum of shop                                                */
	int store_vnum;		/* Rvnum of inventory                                   */
	char *no_such_item1;		/* Message if keeper hasn't got an item */
	char *no_such_item2;		/* Message if player hasn't got an item */
	char *missing_cash1;		/* Message if keeper hasn't got cash    */
	char *missing_cash2;		/* Message if player hasn't got cash    */
	char *do_not_buy;		/* If keeper dosn't buy such things.    */
	char *message_buy;		/* Message when player buys item                */
	char *message_sell;		/* Message when player sells item               */
	int delivery[MAX_DELIVERIES];	/* Merchant replaces these      */
	int trades_in[MAX_TRADES_IN];	/* item_type that can buy       */
	int econ_flags1;		/* Bits which enhance price             */
	int econ_flags2;
	int econ_flags3;
	int econ_flags4;
	int econ_flags5;
	int econ_flags6;
	int econ_flags7;
	float econ_markup1;		/* Sell markup for flagged items        */
	float econ_markup2;
	float econ_markup3;
	float econ_markup4;
	float econ_markup5;
	float econ_markup6;
	float econ_markup7;
	float econ_discount1;		/* Buy markup for flagged items         */
	float econ_discount2;
	float econ_discount3;
	float econ_discount4;
	float econ_discount5;
	float econ_discount6;
	float econ_discount7;
	int buy_flags;		/* Any econ flags set here are traded */
	int nobuy_flags;		/* Any econ flags set here aren't traded */
	int materials;		/* Item materials the shop trades in - MATERIALS in structs.h */
	NEGOTIATION_DATA *negotiations;	/* Haggling information                         */
	int opening_hour;
	int closing_hour;
	int exit;

	shop_data() {
		markup = 0;
		discount = 0;
		buy_flags = 0;
		nobuy_flags = 0;
		materials = 0;
		opening_hour = 0;
		closing_hour = 0;
		exit = 0;
		econ_flags1 = 0;
		econ_flags2 = 0;
		econ_flags3 = 0;
		econ_flags4 = 0;
		econ_flags5 = 0;
		econ_flags6 = 0;
		econ_flags7 = 0;
		econ_markup1 = 0;
		econ_markup2 = 0;
		econ_markup3 = 0;
		econ_markup4 = 0;
		econ_markup5 = 0;
		econ_markup6 = 0;
		econ_markup7 = 0;
		econ_discount1 = 0;
		econ_discount2 = 0;
		econ_discount3 = 0;
		econ_discount4 = 0;
		econ_discount5 = 0;
		econ_discount6 = 0;
		econ_discount7 = 0;
		store_vnum = 0;
		shop_vnum = 0;
		no_such_item1 = NULL;
		no_such_item2 = NULL;
		missing_cash1 = NULL;
		missing_cash2 = NULL;
		do_not_buy = NULL;
		message_buy = NULL;
		message_sell = NULL;

		memset(delivery, 0, (MAX_DELIVERIES * sizeof(int)));
		memset(trades_in, 0, (MAX_TRADES_IN * sizeof(int)));

		negotiations = NULL;
	}

	//~shop_data() {
	//	free_mem(no_such_item1);
	//	free_mem(no_such_item2);
	//	free_mem(missing_cash1);
	//	free_mem(missing_cash2);
	//	free_mem(do_not_buy);
	//	free_mem(message_buy);
	//	free_mem(message_sell);
	//}
};

struct room_extra_data
{
	char *alas[6];
	char *weather_desc[WR_DESCRIPTIONS];
};

#define PC_TRACK	( 1 << 0 )
#define BLOODY_TRACK	( 1 << 1 )
#define FLEE_TRACK	( 1 << 2 )

class track_data {
public:
	shortint race;
	shortint from_dir;
	shortint to_dir;
	shortint hours_passed;
	shortint speed;
	bitflag flags;
	TRACK_DATA *next;

	track_data() {
		race = 0;
		from_dir = 0;
		to_dir = 0;
		hours_passed = 0;
		speed = 0;
		flags = 0;
		next = NULL;
	}
};

struct reset_time_data
{
	int month;
	int day;
	int minute;
	int hour;
	int second;
	int flags;
};

struct time_info_data
{
	int hour;
	int day;
	int month;
	int year;
	int season;
	int minute;
	int holiday;
};

class memory_data {
public:
	char *name;
	struct memory_data *next;

	memory_data() {
		name = NULL;
		next = NULL;
	}

	//~memory_data() {
	//	free_mem(name);
	//}
};

struct char_ability_data
{
	int str;
	int intel;
	int wil;
	int dex;
	int con;
	int aur;
	int agi;
};

class newbie_hint {
public:
	char *hint;
	NEWBIE_HINT *next;

	newbie_hint() {
		hint = NULL;
		next = NULL;
	}

	//~newbie_hint() {
	//	free_mem(hint);
	//}
};

class role_data {
public:
	char *summary;
	char *body;
	char *poster;
	char *date;
	int cost;
	int timestamp;
	int id;
	ROLE_DATA *next;

	role_data() {
		summary = NULL;
		body = NULL;
		poster = NULL;
		date = NULL;
		cost = 0;
		timestamp = 0;
		id = 0;
		next = NULL;
	}

	//~role_data() {
	//	free_mem(summary);
	//	free_mem(body);
	//	free_mem(poster);
	//	free_mem(date);
	//}
};

struct known_spell_data
{
	char *name;
	int rating;
	struct known_spell_data *next;
};

struct affect_spell_type
{
	int duration;			/* For how long its effects will last           */
	int modifier;			/* This is added to apropriate ability          */
	int location;			/* Tells which ability to change(APPLY_XXX)     */
	int bitvector;		/* Tells which bits to set (AFF_XXX)            */
	long t;			/* Extra information                        */
	int sn;			/* Acquired by spell number                                     */
	int technique;
	int form;
	int magnitude;
	int discipline;
	int mana_cost;
};

struct enchantment_data
{
	char *name;
	int original_hours;
	int current_hours;
	int power_source;
	int caster_skill;
	ENCHANTMENT_DATA *next;
};

struct affect_job_type
{
	int days;
	int pay_date;
	int cash;
	int count;
	int object_vnum;
	int employer;
};

struct affect_table_type
{
	int uu1;
	int uu2;
	int uu3;
	int uu4;
	OBJ_DATA *obj;
	int uu6;
};

struct affect_smell_type
{
	int duration;
	int aroma_strength;
	int uu3;
	int uu4;
	int uu5;
	int uu6;
};

struct affect_paralyze
{
	int duration;
	int minutes_until_paralyzed;
	int uu3;
	int uu4;
	int uu5;
	int sn;
};

struct affect_shadow_type
{
	CHAR_DATA *shadow;		/* target begin shadowed              */
	int edge;			/* -1, center.  0-5 edge by direction */
};

struct affect_hidden_type
{
	int duration;
	int hidden_value;
	int coldload_id;
	int uu4;
	int uu5;
	int uu6;
};

struct affect_toll_type
{
	int duration;			/* Generally forever. */
	int dir;
	int room_num;			/* for verification */
	int charge;			/* Cost for others to pass toll crossing */
	int uu5;
	int uu6;
};

struct affect_room_type
{
	int duration;
	int uu2;
	int uu3;
	int uu4;
	int uu5;
	int uu6;
};

struct affect_craft_type
{
	int timer;
	int skill_check;
	PHASE_DATA *phase;
	SUBCRAFT_HEAD_DATA *subcraft;
	CHAR_DATA *target_ch;
	OBJ_DATA *target_obj;
};

struct affect_listen_type
{				/* For muting and later, directed listening */
	int duration;			/* Always on if it exists */
	int on;			/* nonzero is on */
};

struct affect_herbed_type
{				/* For counting the number of herbs found in rooms */
	int duration;
	int timesHerbed;
};

/* Agent FX - let the suffering begin (soon)
enum AGENT_FORM {
AGENT_NONE=0,
AGENT_FUME,
AGENT_POWDER,
AGENT_SOLID,
AGENT_SALVE,
AGENT_LIQUID,
AGENT_SPRAY
};
enum AGENT_METHOD {
AGENT_NONE=0,
AGENT_INJECTED,
AGENT_INHALED,
AGENT_INGESTED,
AGENT_INWOUND,
AGENT_TOUCHED
};
****************************************/

/*
SOMATIC RESPONSES by Sighentist

Basically the idea here, is that the magnitude of the effect is
enveloped over time. So after a delay, the effect begins to grow
in strength until it reaches a peak. After cresting the peak the
strength weakens to a fraction of the peak strength. It remains
fixed there until the effect begins to wear off completely.

Notes:
If latency == duration the affected is a carrier.

*/
struct affect_soma_type
{				/* SOMA EFFECTS TBA */
	int duration;			/* rl hours */
	unsigned short int latency;	/* rl hours of delay  */

	unsigned short int minute;	/* timer  */
	unsigned short int max_power;	/* type-dependant value */
	unsigned short int lvl_power;	/* fraction of max_power */
	unsigned short int atm_power;	/* the current power */

	unsigned short int attack;	/* minutes to amp to max_power */
	unsigned short int decay;	/* minutes to decay to lvl_power */
	unsigned short int sustain;	/* minutes to lvl_power drops */
	unsigned short int release;	/* minutes to end of effect */
};

union affected_union
{
	struct affect_spell_type spell;
	struct affect_job_type job;
	struct affect_table_type table;
	struct affect_shadow_type shadow;
	struct affect_paralyze paralyze;
	struct affect_smell_type smell;
	struct affect_hidden_type hidden;
	struct affect_room_type room;
	struct affect_toll_type toll;
	struct affect_craft_type *craft;
	struct affect_listen_type listening;
	struct affect_herbed_type herbed;
	struct affect_soma_type soma;
};

class affected_type {
public:
	int type;
	union affected_union a;
	AFFECTED_TYPE *next;

	affected_type() {
		memset(this, 0, sizeof(affected_type));
	}
};

//#ifdef HAHA
//struct affected_type
//{
//	int type;			/* The type of spell that caused this           */
//	int duration;			/* For how long its effects will last           */
//	int modifier;			/* This is added to apropriate ability          */
//	int location;			/* Tells which ability to change(APPLY_XXX)     */
//	int bitvector;		/* Tells which bits to set (AFF_XXX)            */
//	int t;			/* Extra information                        */
//	int sn;			/* Acquired by spell number                                     */
//	AFFECTED_TYPE *next;
//};
//#endif

class second_affect {
public:
	int type;
	int seconds;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	char *info;
	int info2;
	SECOND_AFFECT *next;

	second_affect() {
		type = 0;
		seconds = 0;
		ch = NULL;
		obj = NULL;
		info = NULL;
		info2 = 0;
		next = NULL;
	}

	//~second_affect() {
	//	free_mem(info);
	//}

	bool operator== (second_affect &rhs)
	{
		if (this->type != rhs.type || this->seconds != rhs.seconds || this->ch != rhs.ch || this->obj != rhs.obj || this->info2 != rhs.info2 || this->info != rhs.info) {
			return false;
		}
		return true;
	}
};

class var_data {
public:
	char *name;
	int value;
	int type;
	struct var_data *next;

	var_data() {
		name = NULL;
		value = 0;
		type = 0;
		next = NULL;
	}

	//~var_data() {
	//	free_mem(name);
	//}
};

#define MPF_BROKEN		(1 << 0)

class mobprog_data {
public:
	char *trigger_name;
	char *prog;
	char *line;
	int busy;
	int flags;
	int mob_virtual;
	struct mobprog_data *next;
	struct mobprog_data *next_full_prog;

	mobprog_data() {
		trigger_name = NULL;
		prog = NULL;
		line = NULL;
		busy = 0;
		flags = 0;
		mob_virtual = 0;
		next = NULL;
		next_full_prog = NULL;
	}

	//~mobprog_data() {
	//	free_mem(trigger_name);
	//	free_mem(prog);
	//	free_mem(line);
	//}
};

class dream_data {
public:
	char *dream;
	DREAM_DATA *next;

	dream_data() {
		dream = NULL;
		next = NULL;
	}

	//~dream_data() {
	//	free_mem(dream);
	//}
};

class site_info {
public:
	char *name;
	char *banned_by;
	int banned_on;
	int banned_until;
	SITE_INFO *next;

	site_info() {
		name = NULL;
		banned_by = NULL;
		banned_on = 0;
		banned_until = 0;
		next = NULL;
	}

	//~site_info() {
	//	free_mem(name);
	//	free_mem(banned_by);
	//}
};

#define CHARGEN_INT_FIRST		( 1 << 0 )
#define CHARGEN_INT_SECOND		( 1 << 1 )
#define CHARGEN_DEX_1ST2ND		( 1 << 2 )

/* These should match the order of the rows in the spells */
/* database, since they're read in ascending order. */

#define VAR_ID				0
#define VAR_NAME			1
#define VAR_COST			2
#define VAR_SPHERE			3
#define VAR_CIRCLE			4
#define VAR_CH_ECHO			5
#define VAR_SELF_ECHO			6
#define VAR_VICT_ECHO			7
#define VAR_ROOM_ECHO			8
#define VAR_SPELL_DESC			9
#define VAR_TARGET_TYPE			10
#define VAR_SAVE_TYPE			11
#define VAR_SAVE_FOR			12
#define VAR_DAMAGE_FORM			13
#define VAR_AFFECT1			14
#define VAR_DICE1			15
#define VAR_ACT1			16
#define VAR_FADE1			17
#define VAR_AFFECT2			18
#define VAR_DICE2			19
#define VAR_ACT2			20
#define VAR_FADE2			21
#define VAR_AFFECT3			22
#define VAR_DICE3			23
#define VAR_ACT3			24
#define VAR_FADE3			25
#define VAR_AFFECT4			26
#define VAR_DICE4			27
#define VAR_ACT4			28
#define VAR_FADE4			29
#define VAR_AFFECT5			30
#define VAR_DICE5			31
#define VAR_ACT5			32
#define VAR_FADE5			33
#define VAR_AUTHOR			34
#define VAR_LAST_MODIFIED		35

#include <string>

class threat_data {
public:
	CHAR_DATA *source;
	int level;
	THREAT_DATA *next;

	threat_data() {
		source = NULL;
		level = 0;
		next = NULL;
	}
};

class attacker_data {
public:
	CHAR_DATA *attacker;
	ATTACKER_DATA *next;

	attacker_data() {
		attacker = NULL;
		next = NULL;
	}
};

struct sighted_data
{
	CHAR_DATA *target;
	SIGHTED_DATA *next;
};

/* ======================================================================== */

#include "weather.h"

struct constant_data
{
	char constant_name[AVG_STRING_LENGTH];
	char description[AVG_STRING_LENGTH];
	void **index;
};

#define args( list )	list

typedef int SPELL_FUN args ((CHAR_DATA * ch, CHAR_DATA * victim, int sn));
typedef int SPELL_PURE args ((CHAR_DATA * ch, CHAR_DATA * victim));

struct spell_data
{
	char *name;			/* Name of skill              */
	SPELL_FUN *spell_fun;		/* Spell pointer (for spells) */
	int target;			/* Legal targets              */
	int piety_cost;		/* Minimum mana used          */
	int delay;			/* Waiting time BEFORE use     */
	char *msg_off;		/* Wear off message           */
};

struct spell_table_data
{
	char *name;
	SPELL_PURE *spell_fun;
	int discipline;
	int energy_source;
	long cost;
	int target;
};

#define STR_ONE_LINE		2001
#define STR_MULTI_LINE		2000

#define MAX_NATURAL_SKILLS	50

/* Max number of race defines in table */

#define MAX_NUM_RACES		500

typedef struct race_data RACE_TABLE_ENTRY;

class race_data {
public:
	int id;
	char *name;
	bool pc_race;
	int starting_locs;
	int rpp_cost;
	char *created_by;
	int last_modified;
	int race_size;
	int body_proto;
	int innate_abilities;
	int str_mod;
	int con_mod;
	int dex_mod;
	int agi_mod;
	int int_mod;
	int wil_mod;
	int aur_mod;
	int native_tongue;
	int min_age;
	int max_age;
	int min_ht;
	int max_ht;
	int fem_ht_adj;
	RACE_TABLE_ENTRY *next;

	race_data() {
		name = NULL;
		next = NULL;
	}

	//~race_data() {
	//	free_mem(name);
	//}
};

/* Body prototypes for wound location definitions */

#define PROTO_HUMANOID			0
#define PROTO_FOURLEGGED_PAWS		1
#define PROTO_FOURLEGGED_HOOVES		2
#define PROTO_FOURLEGGED_FEET		3
#define PROTO_WINGED_TAIL		4
#define PROTO_WINGED_NOTAIL		5
#define PROTO_SERPENTINE		6

/* Racial define innate abilities */

#define INNATE_INFRA			( 1 << 0 )
#define INNATE_FLYING			( 1 << 1 )
#define INNATE_WAT_BREATH		( 1 << 2 )
#define INNATE_NOBLEED			( 1 << 3 )
#define INNATE_SUN_PEN			( 1 << 4 )
#define INNATE_SUN_PET			( 1 << 5 )

/* Possible starting locations for race defines */

#define RACE_HOME_GONDOR		( 1 << 0 ) /* 1 */
#define RACE_HOME_MORDOR    ( 1 << 1 )     /* 2 */
#define RACE_HOME_HARAD     ( 1 << 2 )     /* 4 */
#define RACE_HOME_CAOLAFON	( 1 << 3 )     /* 8 */
#define RACE_HOME_MORDOR_ORC ( 1 << 4)     /* 16 - orcs */
#define RACE_HOME_BALCHOTH   ( 1 << 5)     /* 32 */

/* These should match the order of the rows in the races */
/* database, since they're read in ascending order. */

#define RACE_NAME		0
#define RACE_ID			1
#define RACE_RPP_COST		2
#define RACE_DESC		3
#define RACE_START_LOC		4
#define RACE_PC			5
#define RACE_AFFECTS		6
#define RACE_BODY_PROTO		7
#define RACE_SIZE		8
#define RACE_STR_MOD		9
#define RACE_CON_MOD		10
#define RACE_DEX_MOD		11
#define RACE_AGI_MOD		12
#define RACE_INT_MOD		13
#define RACE_WIL_MOD		14
#define RACE_AUR_MOD		15
#define RACE_MIN_AGE		16
#define RACE_MAX_AGE		17
#define RACE_MIN_HEIGHT		18
#define RACE_MAX_HEIGHT		19
#define RACE_FEM_HT_DIFF	20
#define RACE_NATIVE_TONGUE	21
#define RACE_SKILL_MODS		22
#define RACE_CREATED_BY		23
#define RACE_LAST_MODIFIED	24
#define RACE_MAX_HIT		25
#define RACE_MAX_MOVE		26
#define RACE_ARMOR		27

struct religion_data
{
	char *tree[30];
};

#define MAX_REGISTRY        50

typedef struct registry_data REGISTRY_DATA;

class registry_data {
public:
	int value;
	char *string;
	REGISTRY_DATA *next;

	registry_data() {
		value = 0;
		string = NULL;
		next = NULL;
	}

	//~registry_data() {
	//	free_mem(string);
	//}
};

struct fight_data
{
	char name[AVG_STRING_LENGTH];
	float offense_modifier;
	float defense_modifier;
	int delay;
};

struct language_data
{
	char name[AVG_STRING_LENGTH];
};

struct poison_data
{
	int poison_type;
	int duration_die_1;
	int duration_die_2;
	int effect_die_1;
	int effect_die_2;
	POISON_DATA *next;
};

class encumberance_info {
public:
	int str_mult_wt;		/* if wt <= str * str_mult, then element applies */
	int defense_pct;
	int delay;
	int offense_pct;
	int move;
	float penalty;
	char *encumbered_status;

	//~encumberance_info() {
	//	free_mem(encumbered_status);
	//}
};

#define NUM_BUCKETS 1024
#define ENCUMBERANCE_ENTRIES	6

/* The FLAG_ bits are saved with the player character */

#define FLAG_KEEPER			( 1 << 0 )
#define FLAG_COMPACT		( 1 << 1 )	/* Player in compact mode */
#define FLAG_BRIEF			( 1 << 2 )	/* Player in brief mode */
#define FLAG_WIZINVIS		( 1 << 3 )
#define FLAG_SUBDUEE		( 1 << 4 )
#define FLAG_SUBDUER		( 1 << 5 )
#define FLAG_SUBDUING		( 1 << 6 )
#define FLAG_ANON			( 1 << 7 )
#define FLAG_COMPETE		( 1 << 8 )
#define FLAG_LEADER_1		( 1 << 9 )	/* Clan 1 leader */
#define FLAG_LEADER_2		( 1 << 10 )	/* Clan 2 leader */
#define FLAG_DEAD			( 1 << 11 )	/* Player has been killed */
#define FLAG_KILL			( 1 << 12 )	/* Player intends to kill */
#define FLAG_FLEE			( 1 << 13 )	/* Player wants to flee combat */
#define FLAG_BINDING		( 1 << 14 )	/* NPC is curently tending wounds */
#define FLAG_SEE_NAME		( 1 << 15 )	/* Show mortal name in says */
#define FLAG_AUTOFLEE		( 1 << 16 )	/* Flee automatically in combat */
#define FLAG_ENTERING		( 1 << 17 )
#define FLAG_LEAVING		( 1 << 18 )
#define FLAG_INHIBITTED		( 1 << 19 )	/* Mob event blocking on program */
#define FLAG_NOPROMPT		( 1 << 20 )	/* Make prompt disappear */
#define FLAG_WILLSKIN       ( 1 << 21 )	/* Everyone can skin corpse */
#define FLAG_ALIASING		( 1 << 22 )	/* Executing an alias */
#define FLAG_OSTLER			( 1 << 23 )	/* Stablemaster */
#define FLAG_TELEPATH 		( 1 << 24 )	/* Hears PC thoughts */
#define FLAG_PACIFIST		( 1 << 25 )	/* Character won't fight back */
#define FLAG_WIZNET		( 1 << 26 )	/* Immortal wiznet toggle */
#define FLAG_HARNESS		( 1 << 27 )	/* Display harness in prompt */
#define FLAG_VARIABLE		( 1 << 28 )	/* Randomized mob prototype */
#define FLAG_ISADMIN		( 1 << 29 )	/* Is an admin's mortal PC */
#define FLAG_AVAILABLE		( 1 << 30 )	/* Available for petitions */
#define FLAG_GUEST		( 1 << 31 )	/* Guest login */

/* plr_flags */

#define NEWBIE_HINTS		( 1 << 0 )	/* Toggle the hint system on or off */
#define NEWBIE			( 1 << 1 )	/* Has not yet commenced */
#define MORALE_BROKEN		( 1 << 2 )
#define MORALE_HELD		( 1 << 3 )
#define FLAG_PETRIFIED		( 1 << 4 )
#define NEW_PLAYER_TAG		( 1 << 5 )	/* Displays (new player) in their ldescs */
#define MENTOR			( 1 << 6 )	/* PC Mentor flag */
#define NOPETITION		( 1 << 7 )	/* No Petition */
#define PRIVATE			( 1 << 8 )	/* Non-Guide-reviewable app */
#define START_GONDOR            ( 1 << 9 )	/* Human, chose to start in Gondor */
#define START_MORDOR_ORC            ( 1 << 10 )	/* Human, chose to start in Mordor */
#define NO_PLAYERPORT		( 1 << 11 )	/* Admins w/out admin access to 4500 */
#define MUTE_BEEPS		( 1 << 12 )	/* Doesn't receive NOTIFY beeps */
#define COMBAT_FILTER		( 1 << 13 )	/* Only receives local combat messages */
#define GROUP_CLOSED		( 1 << 14 )	/* Not accepting any other followers */
#define QUIET_SCAN		( 1 << 15 ) /* quick and quiet scan when entering rooms */
#define NO_BUILDERPORT          ( 1 << 16 )     /* Admins w/o access to 4501 */
#define START_CAOLAFON             ( 1 << 17 ) /* Players starting on human/elf/dwarf side of Caolafon */
#define START_HARAD             ( 1 << 18 ) /* Mordor Haradrim */
#define START_MORDOR_ORC_ORC			( 1 << 19 ) /* Start in Mordor orcs */
#define IS_CRAFTER              ( 1 << 20)  /* required to use cset */
#define START_BALCHOTH				( 1 << 21) /* Start in the Balchoth Vanguard */
#define REBOOT_ACCESS          ( 1 << 22) /* Allows non L5's to use reboot */


/* char_data.guardian_flags - controls notification of PC initiated attacks */
#define GUARDIAN_PC		( 1 << 0 )	/* 01 */
#define GUARDIAN_NPC_HUMANOIDS	( 1 << 1 )	/* 02 */
#define GUARDIAN_NPC_WILDLIFE 	( 1 << 2 )	/* 04 */
#define GUARDIAN_NPC_SHOPKEEPS	( 1 << 3 )	/* 08 */
#define GUARDIAN_NPC_SENTINELS	( 1 << 4 )	/* 16 */
#define GUARDIAN_NPC_KEYHOLDER	( 1 << 5 )	/* 32 */
#define GUARDIAN_NPC_ENFORCERS	( 1 << 6 )	/* 64 */

#define STATE_NAME		( 1 << 0 )	/* Enter name */
#define STATE_GENDER		( 1 << 1 )	/* Choose gender */
#define STATE_RACE		( 1 << 2 )	/* Choose race */
#define STATE_AGE		( 1 << 3 )	/* Input age */
#define STATE_ATTRIBUTES	( 1 << 4 )	/* Distribute attributes */
#define STATE_SDESC		( 1 << 5 )	/* Enter short desc */
#define STATE_LDESC		( 1 << 6 )	/* Enter long desc */
#define STATE_FDESC		( 1 << 7 )	/* Enter full desc */
#define STATE_KEYWORDS		( 1 << 8 )	/* Enter keywords */
#define STATE_FRAME		( 1 << 9 )	/* Choose frame */
#define STATE_SKILLS		( 1 << 10 )	/* Skill selection */
#define STATE_COMMENT		( 1 << 11 )	/* Creation comment */
#define STATE_ROLES		( 1 << 12 )	/* Hardcoded roles/advantages */
#define STATE_SPECIAL_ROLES	( 1 << 13 )	/* Admin-posted special roles */
#define STATE_PRIVACY		( 1 << 14 )	/* Flag app private? */
#define STATE_LOCATION		( 1 << 15 )	/* Humans choose start loc */
#define STATE_PROFESSION	( 1 << 16 )	/* Choosing profession */

/* Hardcoded starting roles/options */

#define EXTRA_COIN		( 1 << 0 )	/* Starts with surplus coin; 1 point */
#define APPRENTICE		( 1 << 1 )	/* Starts as low-ranking member of established org; 1 point */
#define STARTING_ARMOR		( 1 << 2 )	/* Starts with a full set of leather armor; 2 points */
#define SKILL_BONUS		( 1 << 3 )	/* Small bonus to starting skills; 2 points */
#define EXTRA_SKILL		( 1 << 4 )	/* Extra starting skill; 3 points */
#define MAXED_STAT		( 1 << 5 )	/* Starts with one guaranteed 18; 3 points */
#define JOURNEYMAN		( 1 << 6 )	/* As apprentice, but higher-level; 4 points */
#define FELLOW			( 1 << 7 )	/* Starts as a fully-fledged Fellow; 5 points */
#define LESSER_NOBILITY		( 1 << 8 )	/* Starts in a minor noble/merchant family; 6 points */
#define APPRENTICE_MAGE		( 1 << 9 )	/* Self-explanatory; 7 points */

#define TRIG_DONT_USE	0
#define TRIG_SAY		1
#define TRIG_ENTER		2
#define TRIG_EXIT		3
#define TRIG_HIT		4
#define TRIG_MOBACT		5
#define TRIG_ALARM		6
#define TRIG_HOUR		7
#define TRIG_DAY		8
#define TRIG_TEACH		9
#define TRIG_WHISPER	10
#define TRIG_PRISONER	11
#define TRIG_KNOCK		12

#define GAME_BASE_YEAR		2433
#define GAME_SECONDS_BEGINNING  830928041	/* Subtr 10800 to ++gametime 12hr */
#define GAME_SECONDS_PER_YEAR	31104000
#define GAME_SECONDS_PER_MONTH	2592000
#define GAME_SECONDS_PER_DAY	86400
#define GAME_SECONDS_PER_HOUR	3600

#define MODE_COMMAND		(1 << 0)
#define MODE_DONE_EDITING	(1 << 1)
#define MODE_VISEDIT		(1 << 2)
#define MODE_VISHELP		(1 << 3)
#define MODE_VISMAP			(1 << 4)

#define HOME				"\033[H"
#define CLEAR				"\033[2J"

#define DEAD		0
#define MORT		1
#define UNCON		2
#define STUN		3
#define SLEEP		4
#define REST		5
#define SIT		6
#define FIGHT		7
#define STAND		8

#define C_LV1		( 1 << 0 )	/* Immortal level 1 */
#define C_LV2		( 1 << 1 )	/* Immortal level 2 */
#define C_LV3		( 1 << 2 )	/* Immortal level 3 */
#define C_LV4		( 1 << 3 )	/* Immortal level 4 */
#define C_LV5		( 1 << 4 )	/* Immortal level 5 */
#define	C_DEL		( 1 << 5 )	/* Will not break a delay */
#define C_SUB		( 1 << 6 )	/* Commands legal while subdued */
#define C_HID		( 1 << 7 )	/* Commands that keep character hidden */
#define C_DOA		( 1 << 8 )	/* Commands allowed when dead */
#define C_BLD		( 1 << 9 )	/* Commands allowed when blind */
#define C_WLK		( 1 << 10 )	/* Commands NOT allowed while moving */
#define C_XLS		( 1 << 11 )	/* Don't list command */
#define C_MNT		( 1 << 12 )	/* Commands NOT allowed while mounted */
#define C_PAR		( 1 << 13 )	/* Things you CAN do paralyzed */
#define C_GDE		( 1 << 14 )	/* Guide-only command */
#define C_SPL		( 1 << 15 )	/* Commands legal while preparing spell */
#define C_NLG		( 1 << 16 )	/* Command is not logged */
#define C_NWT		( 1 << 17 )	/* Command doesn't show up in SNOOP or WATCH */
#define C_IMP		( 1 << 18 )	/* IMPLEMENTOR ONLY */

class command_data {
public:
	char *command;
	void (*proc) (CHAR_DATA * ch, char *argument, int cmd);
	int min_position;
	int flags;

	// Initialised in an array
	//command_data() {
	//	command = NULL;
	//	min_position = 0;
	//	flags = 0;
	//}

	//~command_data() {
	//	free_mem(command);
	//}
};

class social_data {
public:
	char *social_command;
	int hide;
	int min_victim_position;	/* Position of victim */

	/* No argument was supplied */

	char *char_no_arg;
	char *others_no_arg;

	/* An argument was there, and a victim was found */

	char *char_found;		/* if NULL, read no further, ignore args */
	char *others_found;
	char *vict_found;

	/* An argument was there, but no victim was found */

	char *not_found;

	/* The victim turned out to be the character */

	char *char_auto;
	char *others_auto;

	social_data() {
		social_command = NULL;
		hide = 0;
		min_victim_position = 0;
		char_no_arg = NULL;
		others_no_arg = NULL;
		char_found = NULL;
		others_found = NULL;
		vict_found = NULL;
		not_found = NULL;
		char_auto = NULL;
		others_auto = NULL;
	}

	//~social_data() {
	//	free_mem(social_command);
	//	free_mem(char_no_arg);
	//	free_mem(others_no_arg);
	//	free_mem(char_found);
	//	free_mem(others_found);
	//	free_mem(vict_found);
	//	free_mem(not_found);
	//	free_mem(char_auto);
	//	free_mem(others_auto);
	//}
};

/* data files used by the game system */

#define DFLT_DIR               	"lib"	/* default data directory     */
#define NEWS_FILE       	"news"	/* for the 'news' command     */
#define QSTAT_FILE	 	"text/chargen/stat_message"
#define QSTAT_HELP	  	"text/stat_help"
#define GREET_FILE	  	"text/greetings"
#define MAINTENANCE_FILE	"text/greetings.maintenance"
#define MENU1_FILE	  	"text/menu1"
#define ACCT_APP_FILE		"text/account_application"
#define ACCT_REF_FILE		"text/account_referral"
#define ACCT_EMAIL_FILE		"text/account_email"
#define ACCT_POLICIES_FILE	"text/account_policies"
#define THANK_YOU_FILE		"text/thankyou"
#define CMENU_FILE	  	"text/cmenu"
#define NEW_MENU		"text/new_menu"
#define PDESC_FILE		"text/chargen/new_desc"
#define NAME_FILE		"text/chargen/new_name"
#define PKEYWORDS_FILE	"text/chargen/new_keyword"
#define PLDESC_FILE		"text/chargen/new_ldesc"
#define PSDESC_FILE		"text/chargen/new_sdesc"
#define PLAYER_NEW		"text/player_new"
#define PLAYER_FILE     "save/players"	/* the player database        */
#define SOCMESS_FILE    "text/actions"	/* messgs for social acts     */
#define HELP_FILE		"text/help_table"	/* for HELP <keywrd>          */
#define BHELP_FILE		"text/bhelp_table"	/* for BHELP <keywrd>         */
#define HELP_PAGE_FILE  "text/help"	/* for HELP <CR>              */
#define BHELP_PAGE_FILE "text/bhelp"	/* for BHELP <CR>             */
#define INFO_FILE       "text/info"	/* for INFO                   */
#define WIZLIST_FILE    "text/wizlist"	/* for WIZLIST                */
#define NEW_CHAR_INTRO	"text/new_char_intro"	/* ground rules                       */
#define RACE_SELECT		"text/chargen/race_select"	/* Race choice question           */
#define AGE_SELECT		"text/chargen/age_select"	/* Age choice question        */
#define SEX_SELECT		"text/chargen/sex_select"	/* Sex choice question        */
#define ROLE_SELECT		"text/chargen/role_select"	/* Explanation of roles */
#define SPECIAL_ROLE_SELECT	"text/chargen/special_role_select"	/* Explanation of special roles */
#define HEIGHT_FRAME	"text/chargen/height_frame"
#define COMMENT_HELP	"text/chargen/comment_help"
#define LOCATION	"text/chargen/location"
#define SKILL_SELECT	"text/chargen/skills_select"
#define PROFESSION_SELECT	"text/chargen/professions"
#define SAVE_DIR		"save"
#define HELP_DIR	"../lib/help/"
#define REGIONS			"../regions"
#define REGISTRY_FILE	REGIONS "/registry"
#define CRAFTS_FILE		REGIONS "/crafts"
#define BOARD_DIR		"boards"
#define JOURNAL_DIR		"player_journals"
#define BOARD_ARCHIVE	"archive"
#define PLAYER_BOARD_DIR "player_boards"
#define VIRTUAL_BOARD_DIR	"vboards"
#define TICKET_DIR		"tickets"
#define AUCTION_DIR		"auctions"
#define COLDLOAD_IDS	"coldload_ids"
#define DYNAMIC_REGISTRY REGIONS "/dynamic_registry"
#define STAYPUT_FILE	"stayput"

#define ZONE_SIZE	1000
#define OBJECT_ZONE_SIZE 1000

/* structure for the reset commands */

class reset_affect {
public:
	char *craft_name;
	int type;
	int duration;
	int modifier;
	int location;
	int bitvector;
	int t;
	int sn;

	reset_affect() {
		craft_name = NULL;
		type = 0;
		duration = 0;
		modifier = 0;
		location = 0;
		bitvector = 0;
		t = 0;
		sn = 0;
	}

	//~reset_affect() {
	//	free_mem(craft_name);
	//}
};

struct reset_com
{
	char command;			/* current command                      */
	bool if_flag;			/* if true: exe only if preceding exe'd */
	int arg1;			/* (Can be ptr to reset_affect data)    */
	int arg2;			/* Arguments to the command             */
	int arg3;			/*                                      */
	int arg4;			/*                                      */
	int arg5;			/*                                      */
	int enabled;			/* true if this reset should be used    */

	/*
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*  'C': Craft (an affect) *
	*  'A': Affect            *
	*  'R': Room #            *
	*  'r': Room Affect       *
	*  'm': Mob reset (timed, reply variety)
	*/
};

/* zone definition structure. for the 'zone-table'   */

#define MAX_MSGS_PER_BOARD		5000
#define MAX_LANG_PER_BOARD		  10

class board_data {
public:
	char *name;
	char *title;
	int level;
	int next_virtual;
	int msg_nums[MAX_MSGS_PER_BOARD];
	char *msg_titles[MAX_MSGS_PER_BOARD];
	int language[MAX_LANG_PER_BOARD];
	BOARD_DATA *next;

	board_data() {
		name = NULL;
		title = NULL;
		level = 0;
		next_virtual = 0;
		next = NULL;
	}

	//~board_data() {
	//	free_mem(name);
	//	free_mem(title);
	//}
};

#define MF_READ		( 1 << 0 )
#define MF_ANON		( 1 << 1 )
#define MF_PRIVATE	( 1 << 2 )
#define MF_URGENT	( 1 << 3 )
#define MF_DREAM	( 1 << 4 )
#define MF_REPLIED	( 1 << 5 )

class mudmail_data {
public:
	long flags;
	char *from;
	char *from_account;
	char *date;
	char *subject;
	char *message;
	MUDMAIL_DATA *next_message;
	char *target;

	mudmail_data() {
		flags = 0;
		from = NULL;
		from_account = NULL;
		date = NULL;
		subject = NULL;
		message = NULL;
		next_message = NULL;
		target = NULL;
	}

	//~mudmail_data() {
	//	free_mem(from);
	//	free_mem(from_account);
	//	free_mem(date);
	//	free_mem(subject);
	//	free_mem(message);
	//	free_mem(target);
	//}
};


#define TO_ROOM		( 1 << 0 )
#define TO_VICT		( 1 << 1 )
#define TO_NOTVICT	( 1 << 2 )
#define TO_CHAR		( 1 << 3 )
#define _ACT_FORMAT	( 1 << 4 )
#define TO_IMMS		( 1 << 5 )
#define _ACT_COMBAT	( 1 << 6 )
#define TO_GROUP	( 1 << 7 )
#define _ACT_SEARCH	( 1 << 8 )

struct fatigue_data
{
	int percent;
	float penalty;
	char name[AVG_STRING_LENGTH];
};

struct use_table_data
{
	int delay;
};

/* NOTE:  If adding a new COMBAT_, update cs_name in fight.c */

#define SUC_CF				0
#define SUC_MF				1
#define SUC_MS				2
#define SUC_CS				3

/* NOTE:  If adding a new RESULT_, update rs_name in fight.c */

#define RESULT_NONE			0
#define RESULT_ADV			1
#define RESULT_BLOCK			2
#define RESULT_PARRY			3
#define RESULT_FUMBLE			4
#define RESULT_HIT			5
#define RESULT_HIT1			6
#define RESULT_HIT2			7
#define RESULT_HIT3			8
#define RESULT_HIT4			9
#define RESULT_STUMBLE			10
#define RESULT_NEAR_FUMBLE		11
#define RESULT_NEAR_STUMBLE		12
#define RESULT_DEAD			13
#define RESULT_ANY			14
#define RESULT_WEAPON_BREAK 		15
#define RESULT_SHIELD_BREAK 		16
#define RESULT_KO			17
#define RESULT_JUST_KO			18

class combat_msg_data {
public:
	int off_result;
	int def_result;
	int table;
	char *def_msg;
	char *off_msg;
	char *other_msg;
	COMBAT_MSG_DATA *next;

	combat_msg_data() {
		off_result = 0;
		def_result = 0;
		table = 0;
		def_msg = NULL;
		off_msg = NULL;
		other_msg = NULL;
		next = NULL;
	}

	//~combat_msg_data() {
	//	free_mem(def_msg);
	//	free_mem(off_msg);
	//	free_mem(other_msg);
	//}
};


#define STATE_REJECTED		-1
#define STATE_APPLYING		0
#define STATE_SUBMITTED		1
#define STATE_APPROVED		2
#define STATE_SUSPENDED		3
#define STATE_DIED			4

class move_data {
public:
	int dir;
	int flags;
	int desired_time;
	MOVE_DATA *next;
	char *travel_str;

	move_data() {
		dir = 0;
		flags = 0;
		desired_time = 0;
		next = NULL;
		travel_str = NULL;
	}

	//~move_data() {
	//	free_mem(travel_str);
	//}
};

class qe_data {
public:
	/* Quarter second events data structure */
	CHAR_DATA *ch;
	int dir;
	int speed_type;
	int flags;
	ROOM_DATA *from_room;
	int event_time;
	int arrive_time;
	int move_cost;
	QE_DATA *next;
	char *travel_str;

	qe_data() {
		ch = NULL;
		dir = 0;
		speed_type = 0;
		flags = 0;
		from_room = NULL;
		event_time = 0;
		arrive_time = 0;
		move_cost = 0;
		next = NULL;
		travel_str = NULL;
	}

	//~qe_data() {
	//	free_mem(travel_str);
	//}
};

#define MF_WALK			( 1 << 0 )
#define MF_RUN			( 1 << 1 )
#define MF_SWIM			( 1 << 2 )
#define MF_PASSDOOR		( 1 << 3 )
#define MF_ARRIVAL		( 1 << 4 )
#define MF_TOEDGE		( 1 << 5 )
#define MF_TONEXT_EDGE	( 1 << 6 )
#define MF_SNEAK		( 1 << 7 )

#define SPEED_WALK		0
#define SPEED_CRAWL		1
#define SPEED_PACED		2
#define SPEED_JOG		3
#define SPEED_RUN		4
#define SPEED_SPRINT	5
#define SPEED_IMMORTAL	6
#define SPEED_SWIM		7

typedef int PP ();
typedef int PP_ch (CHAR_DATA * ch, int success, int psn);
typedef int PP_ch_victim (CHAR_DATA * ch, CHAR_DATA * victim, int success,
						  int psn);
typedef int PP_ch_obj (CHAR_DATA * ch, OBJ_DATA * obj, int success, int psn);

#define FRAME_FEATHER		0
#define FRAME_SCANT		1
#define FRAME_LIGHT		2
#define FRAME_MEDIUM		3
#define FRAME_HEAVY		4
#define FRAME_MASSIVE		5
#define FRAME_SUPER_MASSIVE	6

#define SIZE_UNDEFINED		0
#define SIZE_XXS		1	/* Smaller than PC sized mobs */
#define SIZE_XS			2
#define SIZE_S			3
#define SIZE_M			4
#define SIZE_L			5
#define SIZE_XL			6
#define SIZE_XXL		7	/* Larger than PC sized mobs */

struct mob_race_data
{
	char race_name[AVG_STRING_LENGTH];
	int male_ht_dice;
	int male_ht_sides;
	int male_ht_constant;
	int male_fr_dice;
	int male_fr_sides;
	int male_fr_constant;
	int female_ht_dice;
	int female_ht_sides;
	int female_ht_constant;
	int female_fr_dice;
	int female_fr_sides;
	int female_fr_constant;
	int can_subdue;
};

struct cbt
{
	char bt[100];
};

struct ibt
{
	int i[25];
};

#define CRIME_KILL			1	/* 5 hours, see criminalize() */
#define CRIME_STEAL			2	/* 3 hours */
#define CRIME_PICKLOCK		3	/* 1 hour */
#define CRIME_BACKSTAB		4	/* 5 hours */
#define CRIME_SUBDUE		5	/* 1 hour */
#define CRIME_BRAWL         6   
#define CRIME_RESIST_ARREST 7
#define CRIME_FLEE          8

struct memory_t
{
	int dtype;
	int entry;
	int bytes;
	int time_allocated;
};

#define PHASE_CANNOT_LEAVE_ROOM ( 1 << 0 )
#define PHASE_OPEN_ON_SELF		( 1 << 1 )
#define PHASE_REQUIRE_ON_SELF   ( 1 << 2 )
#define PHASE_REQUIRE_GREATER	( 1 << 3 )

/* These flags apply to objects within a phase */

#define SUBCRAFT_IN_ROOM		( 1 << 0 )
#define SUBCRAFT_GIVE			( 1 << 1 )	// Give to Crafter's hands on production
#define SUBCRAFT_HELD			( 1 << 2 )
#define SUBCRAFT_WIELDED		( 1 << 3 )
#define SUBCRAFT_USED			( 1 << 4 )
#define SUBCRAFT_PRODUCED		( 1 << 5 )
#define SUBCRAFT_WORN			( 1 << 6 )

/* Craft Mobile flags */

#define CRAFT_MOB_SETOWNER		( 1 << 0 )

/* Subcraft flags */

#define SCF_TARGET_OBJ			( 1 << 0 )	/* Target object */
#define SCF_TARGET_CHAR			( 1 << 1 )	/* Target character */
#define SCF_DEFENSIVE			( 1 << 2 )	/* Defensive - default */
#define SCF_OFFENSIVE			( 1 << 3 )	/* Offensive spell */
#define SCF_AREA				( 1 << 4 )	/* Area spell, all objs or chars */
#define SCF_ROOM				( 1 << 5 )	/* Target room */
#define SCF_TARGET_SELF			( 1 << 6 )	/* Target self */
#define SCF_OBSCURE    			( 1 << 7 )	/* Hide from Website/Palantir */

#define MAX_DEFAULT_ITEMS		20

class default_item_data {
public:
	int flags;			/* See SUBCRAFT_ flags */
	int items[MAX_DEFAULT_ITEMS];	/* Up to 20 default items */
	short item_counts;		/* instances of items */
	PHASE_DATA *phase;		/* Phase mentioned */
	int key_index;		/* index for key items */
	char *color;

	default_item_data();
	default_item_data(const default_item_data &RHS);

	default_item_data& operator= (const default_item_data &RHS);

	~default_item_data();
};

class phase_data {
public:
	char *first;			/* First person message */
	char *second;			/* Second person message */
	char *third;			/* Third person message */
	char *self;			/* If targets self, use this as first */
	char *second_failure;		/* Failure message to 2nd person */
	char *third_failure;		/* Failure message to 3rd persons */
	char *failure;		/* Failure message for phase */
	int flags;			/* PHase flags */
	int phase_seconds;		/* Time in seconds of phase */
	int skill;			/* Only used for skill checks */
	int attr; 		/* Used for attribute check (like skill) */
	int dice;			/* dice v skill */
	int sides;			/* sides v skill (diceDsides v skill) */
	int targets;			/* Spell target flags */
	int duration;			/* Hours on spell */
	int power;			/* Power of spell.  eg -3 to +3 str */
	int hit_cost;			/* Hit cost of phase */
	int move_cost;		/* Move cost of phase */
	int spell_type;		/* Spell number */
	int open_skill;		/* Skill to open (crafter or target) */
	int req_skill;		/* Required skill */
	int req_skill_value;		/* Value req_skill must be > or < */
	int attribute;		/* Used for attribute check (like skill) */
	DEFAULT_ITEM_DATA *tool;	/* Usable tools */
	int load_mob;			/* Mob VNUM */
	int nMobFlags;		/* Mob Flags (Stayput, Set Owner) */
	char *group_mess;		/* message to groups followers */
	char *fail_group_mess; /* Failure message for groups */
	PHASE_DATA *next;

	phase_data();
	phase_data(const phase_data &RHS);

	phase_data& operator= (const phase_data &RHS);

	~phase_data();
};

#define MAX_ITEMS_PER_SUBCRAFT	150
#define SECTORSMAX 25
#define SEASONSMAX 7
#define OPENINGMAX 25
#define RACEMAX 25
#define WEATHERMAX 9 

class subcraft_head_data {
public:
	char *craft_name;
	char *subcraft_name;
	char *command;
	char *failure;		/* Failure message for subcraft */
	char *failobjs;		/* String of failure object numbers */
	char *failmobs;		/* String of failure object numbers */
	char *help;
	char *clans;
	PHASE_DATA *phases;
	DEFAULT_ITEM_DATA *obj_items[MAX_ITEMS_PER_SUBCRAFT];
	int subcraft_flags;
	int sectors[SECTORSMAX];
	int seasons[SEASONSMAX];
	int opening[OPENINGMAX];
	int race[RACEMAX];
	int weather[WEATHERMAX];
	int failmob;			/* VNUM of mob it loads up on failure */
	int delay;
	int faildelay;
	int key_index;
	int key_first;			/* which one of the items lists is the key*/
	int key_end;	/* which production item list will be indexed for the key*/
	DEFAULT_ITEM_DATA *fails[MAX_ITEMS_PER_SUBCRAFT];
	int followers; /* number of people following the caller if the craft */
	SUBCRAFT_HEAD_DATA *next;

	subcraft_head_data();
	subcraft_head_data(const subcraft_head_data &RHS);

	subcraft_head_data& operator= (const subcraft_head_data &RHS);

	~subcraft_head_data();
};

#define SCF_CANNOT_LEAVE_ROOM		( 1 << 0 )	/* SCF = Subcraft flag */

class alias_data {
public:
	char *command;
	char *line;
	ALIAS_DATA *next_line;
	ALIAS_DATA *next_alias;

	alias_data() {
		command = NULL;
		line = NULL;
		next_line = NULL;
		next_alias = NULL;
	}

	//~alias_data() {
	//	free_mem(command);
	//	free_mem(line);
	//}
};

#define RESET_TIMED		1
#define RESET_REPLY		2

class reset_data {
public:
	int type;
	char *command;
	int planned;			/* Gets set a minute before reset activates */
	RESET_TIME_DATA when;
	RESET_DATA *next;

	reset_data() {
		type = 0;
		command = NULL;
		planned = 0;
		when.day = 0;
		when.flags = 0;
		when.hour = 0;
		when.minute = 0;
		when.month = 0;
		when.second = 0;
		next = NULL;
	}

	//~reset_data() {
	//	free_mem(command);
	//}
};

class text_data {
public:
	char *filename;
	char *name;
	char *text;
	TEXT_DATA *next;

	text_data() {
		filename = NULL;
		name = NULL;
		text = NULL;
		next = NULL;
	}

	//~text_data() {
	//	free_mem(filename);
	//	free_mem(name);
	//	free_mem(text);
	//}
};

#define EDIT_TYPE_DOCUMENT	0
#define EDIT_TYPE_HELP		1
#define EDIT_TYPE_BHELP		2
#define EDIT_TYPE_CRAFT		3
#define EDIT_TYPE_TEXT		4

class help_info {
public:
	char *keyword;
	char *master_list;
	int required_level;
	char *info;
	HELP_INFO *master_element;
	HELP_INFO *next;

	help_info() {
		keyword = NULL;
		master_list = NULL;
		required_level = 0;
		info = NULL;
		master_element = NULL;
		next = NULL;
	}

	//~help_info() {
	//	free_mem(keyword);
	//	free_mem(master_list);
	//	free_mem(info);
	//}
};

class help_data {
public:
	char *keyword;
	char *keywords;
	char *help_info;		/* Will be null if main_element used */
	HELP_DATA *master_element;	/* Contains actual help_info */
	HELP_DATA *next;

	help_data() {
		keyword = NULL;
		keywords = NULL;
		help_info = NULL;
		master_element = NULL;
		next = NULL;
	}

	//~help_data() {
	//	free_mem(keyword);
	//	free_mem(keywords);
	//	free_mem(help_info);
	//}
};

class name_switch_data {
public:
	char *old_name;
	char *new_name;
	NAME_SWITCH_DATA *next;

	name_switch_data() {
		old_name = NULL;
		new_name = NULL;
		next = NULL;
	}

	//~name_switch_data() {
	//	free_mem(old_name);
	//	free_mem(new_name);
	//}
};

#define ECON_ZONES 13
struct econ_data
{
	char flag_name[AVG_STRING_LENGTH];
	struct
	{
		float markup;
		float discount;
	} obj_econ_info[ECON_ZONES];
};

class zone_data {
public:

	char *name;			/* name of this zone                  */
	char *lead;			/* Name of the project lead             */
	int lifespan;			/* how long between resets (minutes)  */
	int age;			/* current age of this zone (minutes) */
	int top;
	int frozen;
	unsigned long flags;
	int reset_mode;		/* conditions for reset (see below)   */
	struct reset_com *cmd;	/* command table for reset                  */
	int jailer;
	int jail_room_num;
	ROOM_DATA *jail_room;
	int earth_mod;
	int wind_mod;
	int fire_mod;
	int water_mod;
	int shadow_mod;
	int player_in_zone;
	int weather_type;

	zone_data() {
		name = NULL;
		lead = NULL;
		lifespan = 0;
		age = 0;
		top = 0;
		frozen = 0;
		flags = 0;
		reset_mode = 0;
		cmd = NULL;
		jailer = 0;
		jail_room_num = 0;
		jail_room = NULL;
		earth_mod = 0;
		wind_mod = 0;
		fire_mod = 0;
		water_mod = 0;
		shadow_mod = 0;
		player_in_zone = 0;
		weather_type = 0;
	}

	//~zone_data() {
	//	free_mem(name);
	//	free_mem(lead);
	//}
};

#endif //_rpie_structs_h
