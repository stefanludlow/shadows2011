//////////////////////////////////////////////////////////////////////////////
//
/// protos.h - Function Prototypes
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


#ifndef _rpie_protos_h_
#define _rpie_protos_h_

#include <string>
#include <stdio.h>

#ifndef MACOSX
#include <malloc.h>
#endif

#include <sys/time.h>
#include <mysql/mysql.h>

#include "clan.h"

#ifndef uint16
typedef unsigned short uint16;
#endif


#define STR_MATCH 0		// useful to make strcmp more readable

//TODO: Portability: Change all calls to strcasecmp, 
//      And check that it is installed else use str_cmp
#define str_cmp strcasecmp	// q.v. utility. c

typedef void *malloc_t;

/* EXTERNAL DECLARATIONS */
extern const int movement_loss[];
extern struct timeval time_now;
extern SITE_INFO *banned_site;
extern const int sunrise[];	// weather.c
extern const int sunset[];	// weather.c
extern const int seasonal_temp[7][12];	// weather.c
extern bool pending_reboot;
extern const struct encumberance_info enc_tab[];
extern OBJ_DATA *full_object_list;
extern CHAR_DATA *full_mobile_list;
extern HELP_DATA *help_list;
extern HELP_DATA *bhelp_list;
extern long mud_time;
extern char *mud_time_str;
extern int world_version_in;
extern int world_version_out;
extern const int restricted_skills[];

extern int pulse_violence;
extern int x1;
extern int sun_light;
extern int global_moon_light;
extern int moon_light[];
extern int desc_weather[];
extern int MAX_ZONE;
extern int new_auctions;
extern int sold_auctions;
extern int auction_bids;

extern int count_max_online;
extern int arena_matches;
extern int guest_conns;
extern int new_accounts;
extern char max_online_date[32];
extern int num_texts;
extern const int arena_rooms[];
extern const int te_pit_rooms[];
extern long starttime;
extern char s_buf[];		// staff.c
extern char b_buf[];		// staff.c
extern char fatbuf[];
extern char dambuf[];
extern const char *materials[];	// olc.c
extern const char *variable_races[];	// create_mobile.c
extern const char *standard_object_colors[];	// commerce.c
extern const char *fine_object_colors[];	// commerce.c
extern const char *drab_object_colors[];	// commerce.c
extern const char *gem_colors[];	// commerce.c
extern const char *fine_gem_colors[];	// commerce.c
extern const char *wound_locations[];	// olc.c
extern const char *attrs[];
extern const char *sizes[];	// commerce.c
extern const char *sizes_named[];	// commmerce.c
extern const char *frames[];
extern const char* const speeds[];
extern const char* const mount_speeds[];
extern const char* const mount_speeds_ing[];
extern bool memory_check;
extern const char *skills[];
extern const char *rs_name[];	// fight.c
extern const char *attack_names[];	// fight.c
extern const char *econ_flags[];	// commerce.c
extern const char *phase_flags[];
extern const char *item_default_flags[];
extern const char *craft_mobile_flags[];
extern const char *locations[];
extern const char *subcraft_flags[];
extern const char *exit_bits[];
extern const int earth_grid[];
extern const int wind_grid[];
extern const int fire_grid[];
extern const int water_grid[];
extern const int shadow_grid[];
extern TEXT_DATA *text_list;
extern TEXT_DATA *document_list;
extern const float move_speeds[];
extern const struct fatigue_data fatigue[];
extern const struct command_data commands[];
extern const struct fight_data fight_tab[];
extern const struct language_data language_tab[];
extern const struct econ_data default_econ_info[];
extern SUBCRAFT_HEAD_DATA *crafts;
extern REGISTRY_DATA *registry[];
extern SPELL_DATA spell_table[];
extern RACE_TABLE_ENTRY *race_table;
extern CHAR_DATA *assist_queue;
extern CHAR_DATA *character_list;
extern ROLE_DATA *role_list;
extern OBJ_DATA *object_list;
extern struct zone_data *zone_table;
extern struct use_table_data use_table[];	// handler.c
extern const int weapon_armor_table[6][6];
extern const char *item_types[];
extern const char *drinks[];	// olc.c
extern const char *holiday_short_names[];
extern const char *armor_types[];
extern const char *month_short_name[];
extern const char *month_lkup[];
extern const char *verbal_time[];
extern const char *fullness[];
extern const char *color_liquid[];
extern const char *weather_room[];
extern struct msg_data *msg_list;
extern struct spell_table_data spell_list[];
extern const struct body_info body_tab[NUM_BODIES][MAX_HITLOC];
extern const char* const dirs[];
extern const char* const relative_dirs[];
extern const int rev_dir[];
extern int season_time;
extern int port;
extern int times_do_hour_called;
extern int next_mob_coldload_id;
extern int next_pc_coldload_id;
extern int next_obj_coldload_id;
extern const char *weapon_theme[];
extern bool maintenance_lock;

extern struct room_direction_data *dir_options[];
extern MYSQL *database;
extern MYSQL mysql;
extern bool mysql_logging;
extern char *connected_types[];
extern char *where[];
extern const char *position_types[];
extern const char *sex_types[];
extern const char *action_bits[];
extern const char *affected_bits[];
extern const char *room_bits[];
extern const char *sector_types[];	// olc.c
extern const char *seasons[];
extern const char *wear_bits[];
extern const char *extra_bits[];
extern const char *forms[];
extern const char *techniques[];
extern const char *magnitudes[];
extern const char *deity_name[];	// olc.c
extern long top_of_world;
extern int top_of_zone_table;
extern int shutd;
extern char BOOT[];
extern CHAR_DATA *combat_list;
extern int advance_hour_now;
extern struct time_info_data time_info;
extern time_t next_hour_update;
extern time_t next_minute_update;
extern int mp_dirty;
extern int maxdesc;

void update_coverage_times(int admin_time, int admin_time_active, int admin_pc_time, bool admin_found, bool admin_found_absoloute);
void prepare_copyover (int cmd);

/* COMMAND PROTOTYPES */
void do_mark (CHAR_DATA* ch, char *argument, int cmd); // commerce.c
void do_csv (CHAR_DATA* ch, char *argument, int cmd); // staff.c
void retreat (CHAR_DATA* ch, int direction, CHAR_DATA* leader);
void do_retreat (CHAR_DATA * ch, char *argument, int cmd);
void do_doitanyway (CHAR_DATA *ch, char *argument, int cmd);
void do_accuse (CHAR_DATA * ch, char *argument, int cmd);	/* act.other.c        */
void do_party (CHAR_DATA * ch, char *argument, int cmd);	/* act.movement.c     */
void do_accept (CHAR_DATA * ch, char *argument, int cmd);	/* magic.c            */
void do_addcraft (CHAR_DATA * ch, char *argument, int cmd);	/* crafts.c           */
void do_affect (CHAR_DATA * ch, char *argument, int cmd);	/* staff.c            */
void do_aggro (CHAR_DATA * ch, char *argument, int cmd);
void do_aim (CHAR_DATA * ch, char *argument, int cmd);
void do_alert (CHAR_DATA * ch, char *argument, int cmd);
void do_alias (CHAR_DATA * ch, char *argument, int cmd);
void do_alog (CHAR_DATA * ch, char *argument, int cmd);
void do_apply (CHAR_DATA * ch, char *argument, int cmd);
void do_as (CHAR_DATA * ch, char *argument, int cmd);
void do_assist (CHAR_DATA * ch, char *argument, int cmd);
void do_ask (CHAR_DATA * ch, char *argument, int cmd);
void do_assign (CHAR_DATA * ch, char *argument, int cmd);
void do_at (CHAR_DATA * ch, char *argument, int cmd);
void do_auction (CHAR_DATA * ch, char *argument, int cmd);
void do_award (CHAR_DATA * ch, char *argument, int cmd);
void do_ban (CHAR_DATA * ch, char *argument, int cmd);
void do_becho (CHAR_DATA * ch, char *argument, int cmd);
void char__do_bind (CHAR_DATA * ch, char *argument, int cmd);	/* wounds.c           */
void do_blog (CHAR_DATA * ch, char *argument, int cmd);	/* staff.c            */
void do_bolt (CHAR_DATA * ch, char *argument, int cmd);
void do_bridle (CHAR_DATA * ch, char *argument, int cmd);
void do_brief (CHAR_DATA * ch, char *argument, int cmd);
void do_broadcast (CHAR_DATA * ch, char *argument, int cmd);
void do_buck (CHAR_DATA * ch, char *argument, int cmd);
void do_bug (CHAR_DATA * ch, char *argument, int cmd);
void do_camp (CHAR_DATA * ch, char *argument, int cmd);
void do_cast (CHAR_DATA * ch, char *argument, int cmd);
void do_castout (CHAR_DATA * ch, char *argument, int cmd);
void do_choke (CHAR_DATA * ch, char *argument, int cmd);
void do_classify (CHAR_DATA * ch, char *argument, int cmd);
void do_clockout (CHAR_DATA * ch, char *argument, int cmd);
void do_clockin (CHAR_DATA * ch, char *argument, int cmd);
void do_clog (CHAR_DATA * ch, char *argument, int cmd);
void do_close (CHAR_DATA * ch, char *argument, int cmd);
void do_command (CHAR_DATA * ch, char *argument, int cmd);
void do_commands (CHAR_DATA * ch, char *argument, int cmd);
void do_commence (CHAR_DATA * ch, char *argument, int cmd);
void do_compact (CHAR_DATA * ch, char *argument, int cmd);
void do_compare (CHAR_DATA * ch, char *argument, int cmd);
void do_compete (CHAR_DATA * ch, char *argument, int cmd);
void do_contents (CHAR_DATA * ch, char *argument, int cmd);
void do_cover (CHAR_DATA * ch, char *argument, int cmd);
void do_count (CHAR_DATA * ch, char *argument, int cmd);
void do_crafts (CHAR_DATA * ch, char *argument, int cmd);
void do_craftspc (CHAR_DATA * ch, char *argument, int cmd);
void do_credits (CHAR_DATA * ch, char *argument, int cmd);
void do_cset (CHAR_DATA * ch, char *argument, int cmd);
void do_day (CHAR_DATA * ch, char *argument, int cmd);
void do_debug (CHAR_DATA * ch, char *argument, int cmd);
void do_decline (CHAR_DATA * ch, char *argument, int cmd);
void do_deduct (CHAR_DATA * ch, char *argument, int cmd);
void do_diagnose (CHAR_DATA * ch, char *argument, int cmd);
void do_dip (CHAR_DATA * ch, char *argument, int cmd);
void do_disband (CHAR_DATA * ch, char *argument, int cmd);
void do_dismantle (CHAR_DATA * ch, char *argument, int cmd);
void do_dismount (CHAR_DATA * ch, char *argument, int cmd);
void do_document (CHAR_DATA * ch, char *argument, int cmd);
void do_down (CHAR_DATA * ch, char *argument, int cmd);
void do_dmote (CHAR_DATA * ch, char *argument, int cmd);
void do_drag (CHAR_DATA * ch, char *argument, int cmd);
void do_dreams (CHAR_DATA * ch, char *argument, int cmd);
void do_east (CHAR_DATA * ch, char *argument, int cmd);
void do_echo (CHAR_DATA * ch, char *argument, int cmd);
void do_map (CHAR_DATA * ch, char *argument, int cmd);
void do_edit (CHAR_DATA * ch, char *argument, int cmd);
void do_email (CHAR_DATA * ch, char *argument, int cmd);
void do_emote (CHAR_DATA * ch, char *argument, int cmd);
void do_enter (CHAR_DATA * ch, char *argument, int cmd);
void do_equipment (CHAR_DATA * ch, char *argument, int cmd);
void do_erase (CHAR_DATA * ch, char *argument, int cmd);
void do_escape (CHAR_DATA * ch, char *argument, int cmd);
void do_examine (CHAR_DATA * ch, char *argument, int cmd);
void do_exits (CHAR_DATA * ch, char *argument, int cmd);
void do_find (CHAR_DATA * ch, char *argument, int cmd);
void do_fire (CHAR_DATA * ch, char *argument, int cmd);
void do_fivenet (CHAR_DATA * ch, char *argument, int cmd);
void do_flee (CHAR_DATA * ch, char *argument, int cmd);
void do_flip (CHAR_DATA * ch, char *argument, int cmd);
void do_forage (CHAR_DATA * ch, char *argument, int cmd);
void do_force (CHAR_DATA * ch, char *argument, int cmd);
void do_freeze (CHAR_DATA * ch, char *argument, int cmd);
void do_gecho (CHAR_DATA * ch, char *argument, int cmd);
void do_givedream (CHAR_DATA * ch, char *argument, int cmd);
void do_goto (CHAR_DATA * ch, char *argument, int cmd);
void do_gstat (CHAR_DATA * ch, char *argument, int cmd);
void do_guard (CHAR_DATA * ch, char *argument, int cmd);
void do_heal (CHAR_DATA * ch, char *argument, int cmd);
void do_health (CHAR_DATA * ch, char *argument, int cmd);
void do_hedit (CHAR_DATA * ch, char *argument, int cmd);
void do_help (CHAR_DATA * ch, char *argument, int cmd);
void do_hex (CHAR_DATA * ch, char *argument, int cmd);
void do_hide (CHAR_DATA * ch, char *argument, int cmd);
void do_hire (CHAR_DATA * ch, char *argument, int cmd);
void do_hit (CHAR_DATA * ch, char *argument, int cmd);
void do_hitch (CHAR_DATA * ch, char *argument, int cmd);
void do_hood (CHAR_DATA * ch, char *argument, int cmd);
void do_hour (CHAR_DATA * ch, char *argument, int cmd);
void do_ic (CHAR_DATA * ch, char *argument, int cmd);
void do_ichat (CHAR_DATA * ch, char *argument, int cmd);
void do_idea (CHAR_DATA * ch, char *argument, int cmd);
void do_immcommands (CHAR_DATA * ch, char *argument, int cmd);
void do_immtell (CHAR_DATA * ch, char *argument, int cmd);
void do_inside (CHAR_DATA * ch, char *argument, int cmd);
void do_instruct (CHAR_DATA * ch, char *argument, int cmd);
void do_inventory (CHAR_DATA * ch, char *argument, int cmd);
void do_invis (CHAR_DATA * ch, char *argument, int cmd);
void do_invite (CHAR_DATA * ch, char *argument, int cmd);
void do_invoke (CHAR_DATA * ch, char *argument, int cmd);
void do_job (CHAR_DATA * ch, char *argument, int cmd);
void do_jerase (CHAR_DATA * ch, char *argument, int cmd);
void do_journal (CHAR_DATA * ch, char *argument, int cmd);
void do_jwrite (CHAR_DATA * ch, char *argument, int cmd);
void do_jread (CHAR_DATA * ch, char *argument, int cmd);
void do_nokill (CHAR_DATA * ch, char *argument, int cmd);
void do_kill (CHAR_DATA * ch, char *argument, int cmd);
void do_knock (CHAR_DATA * ch, char *argument, int cmd);
void do_last (CHAR_DATA * ch, char *argument, int cmd);
void do_leave (CHAR_DATA * ch, char *argument, int cmd);
void do_load (CHAR_DATA * ch, char *argument, int cmd);
void do_locate (CHAR_DATA * ch, char *argument, int cmd);
void do_lock (CHAR_DATA * ch, char *argument, int cmd);
void do_log (CHAR_DATA * ch, char *argument, int cmd);
void do_look (CHAR_DATA * ch, char *argument, int cmd);
void do_materials (CHAR_DATA * ch, char *argument, int cmd);
void do_mclone (CHAR_DATA * ch, char *argument, int cmd);
void do_mcopy (CHAR_DATA * ch, char *argument, int cmd);
void do_minit (CHAR_DATA * ch, char *argument, int cmd);
void do_mlist (CHAR_DATA *ch, char *argument, int cmd);  /* olc.cpp */
void do_mobile (CHAR_DATA * ch, char *argument, int cmd);
void do_mount (CHAR_DATA * ch, char *argument, int cmd);
void move (CHAR_DATA * ch, char *argument, int dir, int speed);
void do_move (CHAR_DATA * ch, char *argument, int cmd);
void do_mset (CHAR_DATA * ch, char *argument, int cmd);
void do_munused (CHAR_DATA * ch, char *argument, int cmd);
void do_mute (CHAR_DATA * ch, char *argument, int cmd);
void do_mysql (CHAR_DATA * ch, char *argument, int cmd);
void do_news (CHAR_DATA * ch, char *argument, int cmd);
void do_nod (CHAR_DATA * ch, char *argument, int cmd);
void do_north (CHAR_DATA * ch, char *argument, int cmd);
void do_notes (CHAR_DATA * ch, char *argument, int cmd);
void do_notify (CHAR_DATA * ch, char *argument, int cmd);
void do_nuke (CHAR_DATA * ch, char *argument, int cmd);
void do_object (CHAR_DATA * ch, char *argument, int cmd);
void do_oinit (CHAR_DATA * ch, char *argument, int cmd);
void do_olist (CHAR_DATA * ch, char *argument, int cmd);
void do_omote (CHAR_DATA * ch, char *argument, int cmd);
void do_ooc (CHAR_DATA * ch, char *argument, int cmd);
void do_open (CHAR_DATA * ch, char *argument, int cmd);
void do_openskill (CHAR_DATA * ch, char *argument, int cmd);
void do_order (CHAR_DATA * ch, char *argument, int cmd);
void do_oset (CHAR_DATA * ch, char *argument, int cmd);
void do_ounused (CHAR_DATA * ch, char *argument, int cmd);
void do_outfit (CHAR_DATA * ch, char *argument, int cmd);
void do_outside (CHAR_DATA * ch, char *argument, int cmd);
void do_ownership (CHAR_DATA *ch, char *argument, int cmd);
void do_palm (CHAR_DATA * ch, char *argument, int cmd);
void do_pay (CHAR_DATA * ch, char *argument, int cmd);
void do_payday (CHAR_DATA * ch, char *argument, int cmd);
void do_payroll (CHAR_DATA * ch, char *argument, int cmd);
void do_pardon (CHAR_DATA * ch, char *argument, int cmd);
void do_passwd (CHAR_DATA * ch, char *argument, int cmd);
void do_pecho (CHAR_DATA * ch, char *argument, int cmd);
void do_petition (CHAR_DATA * ch, char *argument, int cmd);
void do_pfile (CHAR_DATA * ch, char *argument, int cmd);
void do_pick (CHAR_DATA * ch, char *argument, int cmd);
void do_pitch (CHAR_DATA * ch, char *argument, int cmd);
void do_plan (CHAR_DATA * ch, char *argument, int cmd);
void do_plog (CHAR_DATA * ch, char *argument, int cmd);
void do_pmote (CHAR_DATA * ch, char *argument, int cmd);
void do_point (CHAR_DATA * ch, char *argument, int cmd);
void do_poison (CHAR_DATA * ch, char *argument, int cmd);
void do_prescience (CHAR_DATA * ch, char *argument, int cmd);
void do_prepare (CHAR_DATA * ch, char *argument, int cmd);
void do_print (CHAR_DATA * ch, char *argument, int cmd);
void do_professions (CHAR_DATA * ch, char *argument, int cmd);
void do_prog (CHAR_DATA * ch, char *argument, int cmd);
void do_promote (CHAR_DATA * ch, char *argument, int cmd);
void do_purge (CHAR_DATA * ch, char *argument, int cmd);
void do_quaff (CHAR_DATA * ch, char *argument, int cmd);
void do_quit (CHAR_DATA * ch, char *argument, int cmd);
void do_qscan (CHAR_DATA * ch, char *argument, int cmd);
void do_rappend (CHAR_DATA * ch, char *argument, int cmd);
void do_rblock (CHAR_DATA * ch, char *argument, int cmd);
void do_rcap (CHAR_DATA * ch, char *argument, int cmd);
void do_rclone (CHAR_DATA * ch, char *argument, int cmd);
void do_rcret (CHAR_DATA * ch, char *argument, int cmd);
void do_rddesc (CHAR_DATA * ch, char *argument, int cmd);
void do_rdesc (CHAR_DATA * ch, char *argument, int cmd);
void do_rdflag (CHAR_DATA * ch, char *argument, int cmd);
void do_rdoor (CHAR_DATA * ch, char *argument, int cmd);
void do_rgate (CHAR_DATA * ch, char *argument, int cmd);
void do_rdelete (CHAR_DATA * ch, char *argument, int cmd);
void do_reach (CHAR_DATA * ch, char *argument, int cmd);
void do_read (CHAR_DATA * ch, char *argument, int cmd);
void do_recruit (CHAR_DATA * ch, char *argument, int cmd);
void do_redesc (CHAR_DATA * ch, char *argument, int cmd);
void do_refresh (CHAR_DATA * ch, char *argument, int cmd);
void do_register (CHAR_DATA * ch, char *argument, int cmd);
void do_release (CHAR_DATA * ch, char *argument, int cmd);
void do_remcraft (CHAR_DATA * ch, char *argument, int cmd);
void do_replace (CHAR_DATA * ch, char *argument, int cmd);
void do_report (CHAR_DATA * ch, char *argument, int cmd);
void do_rescue (CHAR_DATA * ch, char *argument, int cmd);
void do_resets (CHAR_DATA * ch, char *argument, int cmd);
void do_rlist (CHAR_DATA * ch, char *argument, int cmd);
void do_history (CHAR_DATA * ch, char *argument, int cmd);
void do_rest (CHAR_DATA * ch, char *argument, int cmd);
void do_restore (CHAR_DATA * ch, char *argument, int cmd);
void do_return (CHAR_DATA * ch, char *argument, int cmd);
void do_review (CHAR_DATA * ch, char *argument, int cmd);
void do_rexit (CHAR_DATA * ch, char *argument, int cmd);
void do_rexitrm (CHAR_DATA * ch, char *argument, int cmd);
void do_rflags (CHAR_DATA * ch, char *argument, int cmd);
void do_rinit (CHAR_DATA * ch, char *argument, int cmd);
void do_rkey (CHAR_DATA * ch, char *argument, int cmd);
void do_rlink (CHAR_DATA * ch, char *argument, int cmd);
void do_rlinkrm (CHAR_DATA * ch, char *argument, int cmd);
void do_rmove (CHAR_DATA * ch, char *argument, int cmd);
void do_rname (CHAR_DATA * ch, char *argument, int cmd);
void do_role (CHAR_DATA * ch, char *argument, int cmd);
void do_roll (CHAR_DATA * ch, char *argument, int cmd);
void do_roster (CHAR_DATA * ch, char *argument, int cmd);
void do_rpadd (CHAR_DATA * ch, char *argument, int cmd);
void do_rpapp (CHAR_DATA * ch, char *argument, int cmd);
void do_rpcmd (CHAR_DATA * ch, char *argument, int cmd);
void do_rpdel (CHAR_DATA * ch, char *argument, int cmd);
void do_rpkey (CHAR_DATA * ch, char *argument, int cmd);
void do_rpprg (CHAR_DATA * ch, char *argument, int cmd);
void do_rpstat (CHAR_DATA * ch, char *argument, int cmd);
void do_rsector (CHAR_DATA * ch, char *argument, int cmd);
void do_rset (CHAR_DATA * ch, char *argument, int cmd);
void do_runused (CHAR_DATA * ch, char *argument, int cmd);
void do_rxchange (CHAR_DATA * ch, char *argument, int cmd);
void do_sasil (CHAR_DATA * ch, char *argument, int cmd);
void do_save (CHAR_DATA * ch, char *argument, int cmd);
void do_saverooms (CHAR_DATA * ch, char *argument, int cmd);
void do_say (CHAR_DATA * ch, char *argument, int cmd);
void do_scan (CHAR_DATA * ch, char *argument, int cmd);
void do_scommand (CHAR_DATA *ch, char *argument, int cmd);
void do_score (CHAR_DATA * ch, char *argument, int cmd);
void do_scribe (CHAR_DATA * ch, char *argument, int cmd);
void do_search (CHAR_DATA * ch, char *argument, int cmd);
void do_see (CHAR_DATA * ch, char *argument, int cmd);
void do_select_script (CHAR_DATA * ch, char *argument, int cmd);
void do_send (CHAR_DATA * ch, char *argument, int cmd);
void do_sense (CHAR_DATA * ch, char *argument, int cmd);
void do_set (CHAR_DATA * ch, char *argument, int cmd);
void do_shadow (CHAR_DATA * ch, char *argument, int cmd);
void do_shout (CHAR_DATA * ch, char *argument, int cmd);
void do_show (CHAR_DATA * ch, char *argument, int cmd);
void do_shutdown (CHAR_DATA * ch, char *argument, int cmd);
void do_sing (CHAR_DATA * ch, char *argument, int cmd);
void do_sit (CHAR_DATA * ch, char *argument, int cmd);
void do_skills (CHAR_DATA * ch, char *argument, int cmd);
void do_sleep (CHAR_DATA * ch, char *argument, int cmd);
void do_sneak (CHAR_DATA * ch, char *argument, int cmd);
void do_snoop (CHAR_DATA * ch, char *argument, int cmd);
void do_south (CHAR_DATA * ch, char *argument, int cmd);
void do_speak (CHAR_DATA * ch, char *argument, int cmd);
void do_spells (CHAR_DATA * ch, char *argument, int cmd);
void do_stable (CHAR_DATA * ch, char *argument, int cmd);
void do_stand (CHAR_DATA * ch, char *argument, int cmd);
void do_stat (CHAR_DATA * ch, char *argument, int cmd);
void do_stayput (CHAR_DATA * ch, char *argument, int cmd);
void do_steal (CHAR_DATA * ch, char *argument, int cmd);
void do_stop (CHAR_DATA * ch, char *argument, int cmd);
void do_strike (CHAR_DATA * ch, char *argument, int cmd);
void do_study (CHAR_DATA * ch, char *argument, int cmd);
void do_subdue (CHAR_DATA * ch, char *argument, int cmd);
void do_summon (CHAR_DATA * ch, char *argument, int cmd);
void do_surrender (CHAR_DATA * ch, char *argument, int cmd);
void do_swap (CHAR_DATA * ch, char *argument, int cmd);
void do_swim (CHAR_DATA * ch, char *argument, int cmd);
void do_switch (CHAR_DATA * ch, char *argument, int cmd);
void do_corpses (CHAR_DATA * ch, char *argument, int cmd);
void do_tables (CHAR_DATA * ch, char *argument, int cmd);
void do_tags (CHAR_DATA * ch, char *argument, int cmd);
void do_talk (CHAR_DATA * ch, char *argument, int cmd);
void do_tame (CHAR_DATA * ch, char *argument, int cmd);
void do_teach (CHAR_DATA * ch, char *argument, int cmd);
void do_tear (CHAR_DATA * ch, char *argument, int cmd);
void do_tell (CHAR_DATA * ch, char *argument, int cmd);
void do_te_pit (CHAR_DATA * ch, char *argument, int cmd);
void do_thaw (CHAR_DATA * ch, char *argument, int cmd);
void do_think (CHAR_DATA * ch, char *argument, int cmd);
void do_throw (CHAR_DATA * ch, char *argument, int cmd);
void do_time (CHAR_DATA * ch, char *argument, int cmd);
void do_timeconvert (CHAR_DATA * ch, char *argument, int cmd);
void do_title (CHAR_DATA * ch, char *argument, int cmd);
void do_toll (CHAR_DATA * ch, char *argument, int cmd);
void char__do_toss (CHAR_DATA * ch, char *argument, int cmd);
void char__do_cards (CHAR_DATA * ch, char *argument, int cmd);
void do_tally (CHAR_DATA * ch, char *argument, int cmd);	/* commerce.c */
void do_ticket (CHAR_DATA * ch, char *argument, int cmd); /* act.informative.cpp */
void do_track (CHAR_DATA * ch, char *argument, int cmd);
void do_transfer (CHAR_DATA * ch, char *argument, int cmd);
void do_travel (CHAR_DATA * ch, char *argument, int cmd);	/* act.comm.c */
void do_treat (CHAR_DATA * ch, char *argument, int cmd);
void do_typo (CHAR_DATA * ch, char *argument, int cmd);
void do_unban (CHAR_DATA * ch, char *argument, int cmd);
void do_unlock (CHAR_DATA * ch, char *argument, int cmd);
void do_unload (CHAR_DATA * ch, char *argument, int cmd);
void do_unhitch (CHAR_DATA * ch, char *argument, int cmd);
void do_up (CHAR_DATA * ch, char *argument, int cmd);
void do_users (CHAR_DATA * ch, char *argument, int cmd);
void do_vboards (CHAR_DATA * ch, char *argument, int cmd);
void do_vis (CHAR_DATA * ch, char *argument, int cmd);
void do_voice (CHAR_DATA * ch, char *argument, int cmd);
void do_wake (CHAR_DATA * ch, char *argument, int cmd);
void do_wanted (CHAR_DATA * ch, char *argument, int cmd);
void do_watch (CHAR_DATA * ch, char *arguemnt, int cmd);
void do_wclone (CHAR_DATA * ch, char *argument, int cmd);
void do_wearloc (CHAR_DATA * ch, char *argument, int cmd);
void do_weather (CHAR_DATA * ch, char *argument, int cmd);
void do_west (CHAR_DATA * ch, char *argument, int cmd);
void do_whap (CHAR_DATA * ch, char *argument, int cmd);
void do_where (CHAR_DATA * ch, char *argument, int cmd);
void do_whirl (CHAR_DATA * ch, char *argument, int cmd);
void do_whisper (CHAR_DATA * ch, char *argument, int cmd);
void do_who (CHAR_DATA * ch, char *argument, int cmd);
void do_wizlist (CHAR_DATA * ch, char *argument, int cmd);
void do_wizlock (CHAR_DATA * ch, char *argument, int cmd);
void do_wlog (CHAR_DATA * ch, char *argument, int cmd);
void do_wmotd(CHAR_DATA * ch, char *argument, int cmd);
void do_would (CHAR_DATA * ch, char *argument, int cmd);
void do_write (CHAR_DATA * ch, char *argument, int cmd);
void do_write_book (CHAR_DATA * ch, char *argument, int cmd);
void do_writings (CHAR_DATA * ch, char *argument, int cmd);
void do_wsave (CHAR_DATA * ch, char *argument, int cmd);
void do_zecho (CHAR_DATA * ch, char *argument, int cmd);
void do_zlife (CHAR_DATA * ch, char *argument, int cmd);
void do_zlock (CHAR_DATA * ch, char *argument, int cmd);
void do_zmode (CHAR_DATA * ch, char *argument, int cmd);
void do_zname (CHAR_DATA * ch, char *argument, int cmd);
void do_zsave (CHAR_DATA * ch, char *argument, int cmd);
void do_zset (CHAR_DATA * ch, char *argument, int cmd);


/*
//  act.comm.c
*/
int decipher_speaking (CHAR_DATA * ch, int skillnum, int skill);
char *accent_desc (CHAR_DATA * ch, int skill);
void personalize_emote (CHAR_DATA *ch, char *emote);
bool evaluate_emote_string (CHAR_DATA *ch, std::string * first_person, std::string third_person, std::string argument);




/*
// act.informative.c
*/

void list_char_to_char (CHAR_DATA * list, CHAR_DATA * ch, int mode);
void do_evaluate (CHAR_DATA *ch, char *argument, int cmd);
void post_motd (DESCRIPTOR_DATA * d);
void read_motd(DESCRIPTOR_DATA * d); //nanny.cpp
/*
//  arena.c Funtions --
*/
void morgul_arena_wargs (void);	/* arena.c */
void morgul_arena_troll (void);	/* arena.c */
void do_arena (CHAR_DATA * ch, char *argument, int cmd);	/* arena.c */
void morgul_arena_last (void);	/* arena.c */
void morgul_arena_third (void);	/* arena.c */
void morgul_arena_second (void);	/* arena.c */
void morgul_arena_first (void);	/* arena.c */
void equip_slave (CHAR_DATA * mob);	/* arena.c */
void morgul_arena (void);	/* arena.c */
void arena_engage_echo (CHAR_DATA * ch, CHAR_DATA * vict);
int is_arena_clear (void);
void morgul_arena_cleanup (void);	/* arena.c */
void arena__death (CHAR_DATA * ch);
void arena_combat_message (CHAR_DATA * src, CHAR_DATA * tar, char *location,
			   int damage, int room);
void arena__update_delays (void);
void arena__do_shout (CHAR_DATA * ch, char *argument, int cmd);	/* arena.c */
void arena__do_look (CHAR_DATA * ch, char *argument, int cmd);
void arena__do_enter (CHAR_DATA * ch, char *argument, int cmd);

void te_pit_wargs (void);
void te_pit_troll (void);
void te_pit_last (void);
void te_pit_third (void);
void te_pit_second (void);
void te_pit_first (void);
void te_equip_slave (CHAR_DATA * tmob);
void te_pit (void);
void te_pit_engage_echo (CHAR_DATA * ch, CHAR_DATA * vict);
int is_te_pit_clear (void);
void te_pit_cleanup (void);
void te_pit_death (CHAR_DATA * ch);
void te_pit_combat_message (CHAR_DATA * src, CHAR_DATA * tar, char *location, int damage, int room);
void te_pit_update_delays (void);
void te_pit_do_shout (CHAR_DATA * ch, char *argument, int cmd);
void te_pit_do_look (CHAR_DATA * ch, char *argument, int cmd);
void te_pit_do_enter (CHAR_DATA * ch, char *argument, int cmd);


/* 
//  objects.c Functions --
//  I moved them all together for a reason. 
//  --jw 8^)
*/
void do_mend (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_rend (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void object__drench (CHAR_DATA * ch, OBJ_DATA * _obj, bool isChEquip);	/* objects.c */
char *object__examine_damage (OBJ_DATA * thisPtr);	/* objects.c */
OBJECT_DAMAGE *object__add_damage (OBJ_DATA * thisPtr, DAMAGE_TYPE source, unsigned int impact);	/* objects.c */
void do_grip (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void refresh_colors (CHAR_DATA * keeper);	/* objects.c */
int vnpc_customer (CHAR_DATA * keeper, int purse);	/* objects.c */
void do_switch_item (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void clear_omote (OBJ_DATA * obj);	/* objects.c */
int can_obj_to_container (OBJ_DATA * obj, OBJ_DATA * container, char **msg, int count);	/* objects.c */
int can_obj_to_inv (OBJ_DATA * obj, CHAR_DATA * ch, int *error, int count);	/* objects.c */
int just_a_number (char *buf);	/* objects.c */
void do_junk (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_get (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_take (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_take (CHAR_DATA * ch);	/* objects.c */
void get_break_delay (CHAR_DATA * ch);	/* objects.c */
void delayed_get (CHAR_DATA * ch);	/* objects.c */
void delayed_remove (CHAR_DATA * ch);	/* objects.c */
void do_drop (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_putchar (CHAR_DATA * ch);	/* objects.c */
void do_put (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_give (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
OBJ_DATA *get_equip (CHAR_DATA * ch, int location);	/* objects.c */
void do_drink (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_eat (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_fill (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_pour (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_wear (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_wield (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_remove (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
int can_handle (OBJ_DATA * obj, CHAR_DATA * ch);	/* objects.c */
void do_sheathe (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_draw (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_skin (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_skin_new1 (CHAR_DATA * ch);	/* objects.c */
void delayed_skin_new2 (CHAR_DATA * ch);	/* objects.c */
void delayed_skin_new3 (CHAR_DATA * ch);	/* objects.c */
void do_rummage (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_rummage (CHAR_DATA * ch);	/* objects.c */
void do_gather (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_gather (CHAR_DATA * ch);	/* objects.c */
void do_identify (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void delayed_identify (CHAR_DATA * ch);	/* objects.c */
void do_list (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_preview (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
int keeper_makes (CHAR_DATA * keeper, int ovnum);	/* objects.c */
void money_to_storeroom (CHAR_DATA * keeper, int amount);	/* objects.c */
void subtract_keeper_money (CHAR_DATA * keeper, int cost);	/* objects.c */
void do_buy (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
int keeper_uses_currency_type (int currency_type, OBJ_DATA * obj);	/* objects.c */
int keeper_has_money (CHAR_DATA * keeper, int cost);	/* objects.c */
void do_sell (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_value (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_exchange (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_barter (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_empty (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_blindfold (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_behead (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void light (CHAR_DATA * ch, OBJ_DATA * obj, int on, int on_off_msg);	/* objects.c */
void do_light (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_smell (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void do_receipts (CHAR_DATA * ch, char *argument, int cmd);	/* objects.c */
void show_evaluate_information (CHAR_DATA *ch, OBJ_DATA * obj);	/* objects.c */
/* end objects.c */


void fwrite_a_obj (OBJ_DATA * obj, FILE * fp);
int get_user_seconds ();
struct time_info_data mud_time_passed (time_t t2, time_t t1);
int fread_number (FILE * fp);
void update_crafts_file ();
void list_all_crafts (CHAR_DATA * ch);
void display_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
int craft_uses (SUBCRAFT_HEAD_DATA * craft, int vnum);
void craft_clan (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_delay (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_delete (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_failobjs (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_failmobs (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_failure (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_key (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_key_product (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_opening (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_race (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_seasons (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_sectors (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_setup (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_weather (CHAR_DATA * ch, char *argument, char *subcmd);
OBJ_DATA * get_key_end_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index);
OBJ_DATA * get_key_start_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index);
void craft_group (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_group_pain (CHAR_DATA * ch, int pain_cost);
void craft_group_move (CHAR_DATA * ch, int move_cost, char *message);
void craftstat (CHAR_DATA * ch, char *argument);
void clan_rem_obj (OBJ_DATA *obj, OBJ_CLAN_DATA * targ);

char *breath_bar (CHAR_DATA * ch);
char *fatigue_bar (CHAR_DATA * ch);
char *mana_bar (CHAR_DATA * ch);
int get_token (char **s, char *token);
std::string resolved_host (char *ip);
void mysql_secure_query (MYSQL * conn, char *query, int length);
void retrieve_mysql_board_listing (CHAR_DATA * ch, char *board_name);
void display_mysql_board_message (CHAR_DATA * ch, char *board_name,
				  int msg_num, bool bHideHeader);
int get_comestible_range (int num);
void show_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, int mode);
int has_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
void pad_buffer (char *buf, int pad_stop);
void remove_threat (CHAR_DATA * victim, CHAR_DATA * threat);
void delayed_pitch (CHAR_DATA * ch);
void delayed_quaff (CHAR_DATA * ch);
void delayed_treatment (CHAR_DATA * ch);
int meets_skill_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
int natural_healing_check (CHAR_DATA * ch, WOUND_DATA * wound);
void remove_enchantment (CHAR_DATA * ch, ENCHANTMENT_DATA * ench);
void morph_obj (OBJ_DATA * obj);
void morph_mob (CHAR_DATA * ch);
int can_learn (CHAR_DATA * ch);
int mob_weather_reaction (CHAR_DATA * ch);
int would_attack (CHAR_DATA * ch, CHAR_DATA * tch);
void ready_melee_weapons (CHAR_DATA * ch);
int get_stat_range (int score);
void reformat_say_string (char *source, char **target, CHAR_DATA * to);
void reset_itimer ();
void alarm_handler ();
void init_alarm_handler ();
void temproom (CHAR_DATA * ch, int type);
char *char_short (CHAR_DATA * c);
char *char_names (CHAR_DATA * ch);
char *char_long (CHAR_DATA * c, int show_tname);
void combat_round (void);
void write_obj_data_mysql (OBJ_DATA * obj, char *wc, int pos, int objstack);
void scribe (int new_message, int nVirtual, char *author, char *date,
	     char *message, long flags);
void wear_off_message (CHAR_DATA * ch, ENCHANTMENT_DATA * ench);
char *show_enchantment (ENCHANTMENT_DATA * enchantment);
int general_damage (CHAR_DATA * ch, int amount);
void initialize_weather_zones (void);
int wound_to_char (CHAR_DATA * ch, char *location, int damage, int type,
		   int bleeding, int poison, int infection);
char *downsized_wound (CHAR_DATA * ch, WOUND_DATA * wound);
void heal_all_wounds (CHAR_DATA * ch);
void lodge_from_char (CHAR_DATA * ch, LODGED_OBJECT_INFO * lodged);
void lodge_from_obj (OBJ_DATA * obj, LODGED_OBJECT_INFO * lodged);
void wound_from_char (CHAR_DATA * ch, WOUND_DATA * wound);
void wound_from_obj (OBJ_DATA * obj, WOUND_DATA * wound);
char *expand_wound_loc (char *location);
char *show_wounds (CHAR_DATA * ch, int mode);
int determine_material (OBJ_DATA * obj);
char *figure_location (CHAR_DATA * tar, int location);
char *wound_total (CHAR_DATA * ch);
void make_quiet (CHAR_DATA * ch);
void close_socket (struct descriptor_data *d);
bool isvowel (char c);
//int str_cmp(char *arg1, char *arg2);
int strn_cmp (const char *arg1, const char *arg2, int n);
void send_to_gods (const char *messg);
void send_to_guides (char *message);
void send_to_imms (char *messg);
void send_to_guardians (char *messg, unsigned short int flag);
int is_yours (const char *name, const char *account);
int is_being_reviewed (const char *name, const char *account);
int is_admin (const char *username);
int is_guide (const char *username);
int is_newbie (const CHAR_DATA* ch);
bool is_newbie (const char* account_name);
void send_outside (char *message);
int calculate_race_height (CHAR_DATA * tch);
int calculate_size_height (CHAR_DATA * tch);
void send_outside_zone (char *message, int zone);
unsigned int dice (unsigned int number, unsigned int size);
struct time_info_data age (CHAR_DATA * ch);
void reload_roles (void);
int trades_in (CHAR_DATA * keeper, OBJ_DATA * obj);
int keeper_has_item (CHAR_DATA * keeper, int ovnum);
void redeem_order (CHAR_DATA * ch, OBJ_DATA * ticket, CHAR_DATA * keeper);
float calculate_sale_price (OBJ_DATA * obj, CHAR_DATA * keeper,
			    CHAR_DATA * ch, int quantity, bool round_result,
			    bool sell);
void get_damage (CHAR_DATA * ch, CHAR_DATA * victim, int *dam, int attack_num,
		 int *location);
void affect_modify (CHAR_DATA * ch, int type, int loc, int mod, int bitv,
		    int add, int sn);
int get_damage_total (CHAR_DATA * ch);
void affect_to_char (CHAR_DATA * ch, struct affected_type *af);
void affect_remove (CHAR_DATA * ch, struct affected_type *af);
void affect_join (CHAR_DATA * ch, struct affected_type *af, bool avg_dur,
		  bool avg_mod);
OBJ_DATA *create_money (int amount);
int isname (const char *str, char *namelist);
int isnamec (char *str, char *namelist);
char *fname (char *namelist);
char *fname_hyphen (char *namelist);
void obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch);
void obj_from_char (OBJ_DATA ** obj, int count);
void equip_char (CHAR_DATA * ch, OBJ_DATA * obj, int pos);
OBJ_DATA *unequip_char (CHAR_DATA * ch, int pos);
OBJ_DATA *get_carried_item (CHAR_DATA * ch, int item_type);
OBJ_DATA *get_obj_in_list (char *name, OBJ_DATA * list);
OBJ_DATA *get_obj_in_list_num (int num, OBJ_DATA * list);
OBJ_DATA *get_obj (char *name);
void obj_to_room (OBJ_DATA * obj, int room);
void obj_from_room (OBJ_DATA ** obj, int count);
void obj_to_obj (OBJ_DATA * obj, OBJ_DATA * container);
void obj_from_obj (OBJ_DATA ** obj, int count);
void object_list_new_owner (OBJ_DATA * list, CHAR_DATA * ch);
void extract_obj (OBJ_DATA * obj);
void starting_skill_boost (CHAR_DATA * ch, int skill);
void setup_new_character (CHAR_DATA * tch);
void npc_ranged_response (CHAR_DATA * npc, CHAR_DATA * retal_ch);
void npc_ranged_retaliation (CHAR_DATA * target, CHAR_DATA * ch);
int has_been_sighted (CHAR_DATA * ch, CHAR_DATA * target);
CHAR_DATA *is_switched (CHAR_DATA * ch);
CHAR_DATA *get_mob_vnum (int nVirtual);
CHAR_DATA *get_char_id (int coldload_id);
OBJ_DATA *get_obj_id (int coldload_id);
OBJ_DATA *get_obj_in_list_id (int coldload_id, OBJ_DATA * list);
CHAR_DATA *get_char_room (char *name, int room);
CHAR_DATA *get_char (char *name);
CHAR_DATA *get_char_nomask (char *name);
void char_from_room (CHAR_DATA * ch);
void char_to_room (CHAR_DATA * ch, int room);
CHAR_DATA *get_char_room_vis (CHAR_DATA * ch, const char *name);
CHAR_DATA *get_char_room_vis2 (CHAR_DATA * ch, int vnum, char *name);
CHAR_DATA *get_char_vis (CHAR_DATA * ch, char *name);
void soma_ten_second_affect (CHAR_DATA * ch, AFFECTED_TYPE * af);
void soma_rl_minute_affect (CHAR_DATA * ch, AFFECTED_TYPE * af);
void read_spell (PHASE_DATA * phase, char *argument);
OBJ_DATA *get_obj_in_list_vis (CHAR_DATA * ch, const char *name, OBJ_DATA * list);
OBJ_DATA *get_obj_vis (CHAR_DATA * ch, char *name);
void extract_char (CHAR_DATA * ch);
int generic_find (char *arg, int bitvector, CHAR_DATA * ch,
		  CHAR_DATA ** tar_ch, OBJ_DATA ** tar_obj);
char *swap_xmote_target (CHAR_DATA * ch, char *argument, int cmd);
void clear_pmote (CHAR_DATA * ch);
void clear_voice (CHAR_DATA * ch);
int drowned (CHAR_DATA * ch);
void clear_travel (CHAR_DATA * ch);	/* act.comm.c */
void clear_dmote (CHAR_DATA * ch);

void boot_crafts (void);
void insert_string_variables (OBJ_DATA * new_obj, OBJ_DATA * proto,
			      char *string);
void boot_db (void);
void delayed_remove (CHAR_DATA * ch);
void reg_read_crafts (FILE * fp_reg, char *buf);
void boot_mobiles (void);
void boot_mobprogs (void);
void boot_objects (void);

void copyover_recovery (void);
int is_direction (char *argument);
void vote_notifications (void);
void update_website_statistics (void);
void update_crafts_database (void);

int mysql_safe_query (char *fmt, ...);
void load_race_table (void);
char *expand_wound_loc (char *location);
int number (int from, int to);
void hunger_thirst_process (CHAR_DATA * ch);
void add_shop (int vnum);
void save_shop (FILE * fs, int vnum);
void reset_zone (int zone);
int hit_gain (CHAR_DATA * ch, int poisons_only);
int move_gain (CHAR_DATA * ch);
void sprinttype (int type, const char *names[], char *result);
void list_all_crafts (CHAR_DATA * ch);
void display_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
int craft_uses (SUBCRAFT_HEAD_DATA * craft, int vnum);
void sprintbit (long vektor, const char *names[], char *result);
int rnum_to_znum (int rnum, int zonesize);
void hit_char (CHAR_DATA * ch, CHAR_DATA * victim, int strike_parm);
void stop_fighting (CHAR_DATA * ch);
void renum_zone_table (void);
void update_room_tracks (void);
int is_tagged (char *name_str);
void update_char_objects (CHAR_DATA * ch);
int is_number (const char *str);
int do_simple_move (CHAR_DATA * ch, int dir, int following, int falling,
		    int speed);
void raw_kill (CHAR_DATA * ch);
OBJ_DATA *get_object_in_equip_vis (CHAR_DATA * ch, char *arg,
				   OBJ_DATA * equipment[], int *j);
void gain_condition (CHAR_DATA * ch, int condition, int value);
void npc_evasion (CHAR_DATA * ch, int dir);
void refresh_map (void);
void page_string (struct descriptor_data *d, const char *str);
int save_rooms (CHAR_DATA * ch, int zone);
char *strgdup (char *source);
void arg_splitter (int argc, char *fmt, ...);
int parse_argument (const char *commands[], char *string);
void stop_follower (CHAR_DATA * ch);

void perform_pfile_update (CHAR_DATA * ch);
int is_abbrev (const char *arg1, const char *arg2);
int is_abbrevc (const char *arg1, const char *arg2);
void free_obj (OBJ_DATA * obj);
int search_block (char *arg, char **list, bool exact);
int r_program (CHAR_DATA * ch, char *argument);
void add_memory (CHAR_DATA * add, CHAR_DATA * mob);
bool get_obj_in_equip_num (CHAR_DATA * ch, long vnum);
void spitstat (CHAR_DATA * ch, struct descriptor_data *recipient);
void save_char_objs (CHAR_DATA * ch, char *name);
void criminalize (CHAR_DATA * ch, CHAR_DATA * vict, int zone,
		  int penalty_time);
int skill_level (CHAR_DATA * ch, int skill, int diff_mod);
int skill_use (CHAR_DATA * ch, int skill, int diff_mod);
void add_mob_to_hash (CHAR_DATA * add_mob);
void add_obj_to_hash (OBJ_DATA * add_obj);
OBJ_DATA *vtoo (int nVirtual);
CHAR_DATA *vtom (int nVirtual);
AFFECTED_TYPE *is_crafting (CHAR_DATA * ch);
void randomize_mobile (CHAR_DATA * mob);
CHAR_DATA *load_mobile (int nVirtual);
OBJ_DATA *load_object (int nVirtual);
void soma_stat (CHAR_DATA * ch, AFFECTED_TYPE * af);
void soma_ten_second_affect (CHAR_DATA * ch, AFFECTED_TYPE * af);
void soma_rl_minute_affect (CHAR_DATA * ch, AFFECTED_TYPE * af);
OBJ_DATA *load_colored_object (int nVirtual, char *string);
OBJ_DATA *fread_object (int vnum, int nZone, FILE * fp);
OBJ_DATA *fread_obj (FILE * fp);
void death_email (CHAR_DATA * ch);
int index_lookup (const char* const* index, const char* lookup);
int check_climb (CHAR_DATA * ch);
int _filbuf ();
int ungetc ();
int spell_lookup (char *spell_name);
void read_spell (PHASE_DATA * phase, char *argument);
void archive_log (int log_type);
void create_menu_options (struct descriptor_data *d);
MYSQL_RES *mysql_player_search (int search_type, char *string, int timeframe);
CHAR_DATA *load_char_mysql (const char *name);
int save_char (CHAR_DATA * ch, int save_objs);
void save_char_mysql (CHAR_DATA * ch);
int mem_free (malloc_t ptr);
AFFECTED_TYPE *is_room_affected (AFFECTED_TYPE * af, int type);
void room_update (void);
void apply_race_affects (CHAR_DATA * tch);
void refresh_race_configuration (CHAR_DATA * tch);
void learn_circles_spells (CHAR_DATA * target, CHAR_DATA * ch);
int lookup_value (char *string, int reg_index);
AFFECTED_TYPE *get_affect (const CHAR_DATA * ch, int spell);
int swimming_check (CHAR_DATA * ch);
char *lookup_string (int value, int reg_index);

void set_hobbitmail_flags (int id, int flags);
void setup_registry (void);
void write_dynamic_registry (CHAR_DATA * ch);
void free_registry (void);
struct time_info_data real_time_passed (time_t t2, time_t t1);
int calc_lookup (CHAR_DATA * ch, int index, int entry);
int damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int attacktype);
void mobile_routines (int pulse);
int is_enchanted (CHAR_DATA * ch, char *enchantment_name);
int strike (CHAR_DATA * src, CHAR_DATA * tar, int attack_num);
int is_threat (CHAR_DATA * ch, CHAR_DATA * tch);
void add_threat (CHAR_DATA * victim, CHAR_DATA * threat, int amount);
void show_char_to_char (CHAR_DATA * i, CHAR_DATA * ch, int mode);
void combine_money_inv (OBJ_DATA * source, CHAR_DATA * ch);
void combine_money_obj (OBJ_DATA * source, OBJ_DATA * container,
			CHAR_DATA * ch);
void show_waiting_prisoners (CHAR_DATA * ch);
void notify_captors (CHAR_DATA * ch);
void notify_guardians (CHAR_DATA * ch, CHAR_DATA * tch, int cmd);
int is_in_cell (CHAR_DATA * ch, int zone);
void ready_melee_weapon (CHAR_DATA * ch);
void unready_bow (CHAR_DATA * ch);
void name_money (OBJ_DATA * obj);
void create_guest_avatar (struct descriptor_data *d, char *argument);
void stop_counting (CHAR_DATA * ch);
char *encrypt_buf (const char *buf);
void free_lodged (LODGED_OBJECT_INFO * lodged);
void free_wound (WOUND_DATA * wound);
void free_char (CHAR_DATA * ch);
OBJ_DATA *find_dwelling_obj (int dwelling_room);
void add_room_affect (AFFECTED_TYPE ** af, int type, int duration);
void cleanup_the_dead (int mode);
int calculate_missile_result (CHAR_DATA * ch, int ch_skill, int att_modifier,
			      CHAR_DATA * target, int def_modifier,
			      OBJ_DATA * weapon, OBJ_DATA * missile,
			      AFFECTED_TYPE * spell, int *location,
			      float *damage);
int projectile_shield_block (CHAR_DATA * ch, int result);
int save_dwelling_rooms ();
void load_dwelling_rooms ();
char *time_string (CHAR_DATA * ch);
int num_starting_locs (int race);
void insert_newsletter_into_website (int timestamp, char *newsletter);
void update_website ();
void check_maintenance ();
void unban_site (SITE_INFO * site);
void disconnect_banned_hosts ();
void ban_host (char *host, char *banned_by, int length);
void list_validate (char *name);
int unused_writing_id (void);
void act_black_curse (CHAR_DATA * ch);
void print_mem_stats (CHAR_DATA * ch);
void init_memory ();
char *get_mem (int size);
char *add_hash (const char *string);
char *str_dup (const char *string);
void process_reviews (void);
PHASE_DATA *new_phase (void);
CHAR_DATA *new_char (int pc_type);
OBJ_DATA *new_object ();
int attempt_disarm (CHAR_DATA * ch, CHAR_DATA * victim);
void forget (CHAR_DATA * ch, CHAR_DATA * foe);
void sort_int_array (int *array, int entries);
malloc_t alloc (int bytes, int dtype);
int redefine_mobiles (CHAR_DATA * proto);
AFFECTED_TYPE *get_obj_affect (OBJ_DATA * obj, int type);
AFFECTED_TYPE *get_obj_affect_location (OBJ_DATA * obj, int location);
void remove_obj_affect (OBJ_DATA * obj, int type);
void remove_obj_affect_location (OBJ_DATA * obj, int location);
void affect_to_obj (OBJ_DATA * obj, AFFECTED_TYPE * af);
void process_queued_reviews (void);
void process_reviews (void);
char *colorize (const char *source, char *target, struct descriptor_data *d);
int trigger (CHAR_DATA * ch, char *argument, int trigger);
void trigger_say (CHAR_DATA * ch, CHAR_DATA * mob, char *prog, char *arg);
void hourly_update (void);
void traslate_it (int num);
OBJ_DATA *get_obj_in_dark (CHAR_DATA * ch, char *name, OBJ_DATA * list);
char getall (char *name, char *newname);
int magic_add_affect (CHAR_DATA * ch, int type, int duration,
		      int modifier, int location, int bitvector, int sn);
void magic_affect (CHAR_DATA * ch, int magic);
int magic_add_obj_affect (OBJ_DATA * obj, int type, int duration,
			  int modifier, int location, int bitvector, int sn);
void define_variable (CHAR_DATA * mob, MOBPROG_DATA * program,
		      char *argument);
VAR_DATA *setvar (CHAR_DATA * mob, char *var_name, int value, int type);
VAR_DATA *getvar (CHAR_DATA * mob, char *var_name);
int mp_eval_eq (CHAR_DATA * mob, char **equation);
FILE *open_and_rename (CHAR_DATA * ch, char *name, int zone);
int weaken (CHAR_DATA * victim, uint16 hp_penalty, uint16 mp_penalty,
	    char *log_msg);
void save_writing (OBJ_DATA * obj);
void load_writing (OBJ_DATA * obj);
void load_all_writing (void);
void save_banned_sites ();
void save_roles ();
void save_stayput_mobiles ();
void load_stayput_mobiles ();
void save_reboot_mobiles ();
void load_reboot_mobiles ();
void lodge_missile (CHAR_DATA * target, OBJ_DATA * ammo,
		    char *strike_location);
void stock_new_deliveries ();
void save_tracks ();
void load_tracks ();
void load_leantos ();
void load_online_stats ();
void nullify_affects (CHAR_DATA * ch);
void reapply_affects (CHAR_DATA * ch);
int real_skill (CHAR_DATA * ch, int skill);
int is_in_room (CHAR_DATA * ch, CHAR_DATA * target);
void ve_process (struct descriptor_data *c, char *buf);
void ve_setup_screen (struct descriptor_data *c);
void command_interpreter (CHAR_DATA * ch, char *argument);
void argument_interpreter (char *argument, char *first_arg, char *second_arg);
int fill_word (char *argument);
void half_chop (char *string, char *arg1, char *arg2);
void nanny (struct descriptor_data *d, char *arg);
int social (CHAR_DATA * ch, char *argument);
void checkpointing (int);
void shutdown_request (int);
void logsig (int);
void hupsig (int);
void sigsegv (int);
void sigchld (int);
void sigabrt (int);
int create_entry (char *name);
void zone_update (void);
void clear_char (CHAR_DATA * ch);
void clear_object (OBJ_DATA * obj);
void free_char (CHAR_DATA * ch);
char *read_string (char *string);
char *fread_string (FILE * fl);
char *fread_word (FILE * fl);
void load_boards (void);
int add_registry (int reg_index, int value, const char *string);
void write_board_list (void);
BOARD_DATA *board_lookup (const char *name);
void add_message (int new_message, const char *name, int nVirtual, const char *poster,
		  char *date, char *subject, char *info, char *message,
		  long flags);
void update_assist_queue (CHAR_DATA * ch, bool remove);
int get_queue_position (CHAR_DATA * ch);
void add_char (char *buf, char c);
void add_board (int level, char *name, char *title);
void send_to_all (char *message);
void send_to_all_unf (char *message);
void send_to_char (const char *message, const CHAR_DATA * ch);
void send_to_not_char (const char *message, const CHAR_DATA *ch);
void send_to_room (char *message, int room_num);
void send_to_room_unf (char *message, int room_num);

void mark_as_read (CHAR_DATA * ch, int number);
int adjust_wound (CHAR_DATA * ch, WOUND_DATA * wound, int amount);
struct message_data *load_mysql_message (char *msg_name, int board_type,
					 int msg_number);
int erase_mysql_board_post (CHAR_DATA * ch, char *name, int board_type,
			    char *argument);
int get_mysql_board_listing (CHAR_DATA * ch, int board_type, char *name);
void add_message_to_mysql_vboard (const char *name, const char *poster,
				  struct message_data *message);
void add_message_to_mysql_player_notes (const char *name, const char *poster,
					struct message_data *message);

char *lookup_race_variable (int id, int which_var);
int lookup_race_id (const char *name);

char *lookup_spell_variable (int id, int which_var);
int lookup_spell_id (char *name);

char *spell_fade_echo (int id, int affect_type);
char *spell_activity_echo (int id, int affect_type);
void set_spell_proficiency (CHAR_DATA * ch, int id, int set_to);
void remove_spell (CHAR_DATA * ch, int id);
int knows_spell (CHAR_DATA * ch, int id);
int caster_type (CHAR_DATA * ch);
void const_to_non_const_cstr (const char * string, char * edit_string);
void const_to_non_const_cstr (const char * string, char * edit_string, int n);
void act (char *str, int hide_invisible, CHAR_DATA * ch, OBJ_DATA * obj,
	  void *vict_obj, int type);
CHAR_DATA *try_load_char (char *name);
void dream (CHAR_DATA * ch);
void awaken_break_delay (CHAR_DATA * ch);
int is_he_here (CHAR_DATA * ch, CHAR_DATA * he, int check);
int is_he_somewhere (CHAR_DATA * he);
void post_greater (struct descriptor_data *d);
void post_dream (struct descriptor_data *d);
void post_lesser (struct descriptor_data *d);
void post_prescience (struct descriptor_data *d);
void post_mset (struct descriptor_data *d);
void post_track_response (struct descriptor_data *d);
void post_oset (struct descriptor_data *d);
void post_to_mysql_virtual_board (struct descriptor_data *d);
void post_to_mysql_player_board (struct descriptor_data *d);
void post_to_mysql_journal (struct descriptor_data *d);
void post_to_mysql_board (struct descriptor_data *d);
void post_message (struct descriptor_data *d);
void nanny_choose_pc (struct descriptor_data *d, char *argument);
void sense_activity (CHAR_DATA * user, int talent);

void check_idlers ();
void check_linkdead ();
void delayed_trigger_activity ();
void delayed_ooc (CHAR_DATA * ch);
void delayed_track (CHAR_DATA * ch);
void delayed_load (CHAR_DATA * ch);
void delayed_alert (CHAR_DATA * ch);
void delayed_awaken (CHAR_DATA * ch);
void delayed_bind (CHAR_DATA * ch);
void delayed_bolt (CHAR_DATA * ch);
void delayed_camp1 (CHAR_DATA * ch);
void delayed_camp2 (CHAR_DATA * ch);
void delayed_camp3 (CHAR_DATA * ch);
void delayed_camp4 (CHAR_DATA * ch);
void delayed_cast (CHAR_DATA * ch);
void delayed_count_coin (CHAR_DATA * ch);
void delayed_cover (CHAR_DATA * ch);
void delayed_forage (CHAR_DATA * ch);
void delayed_forage_seek (CHAR_DATA * ch);
void delayed_heal (CHAR_DATA * ch);
void delayed_hide (CHAR_DATA * ch);
void delayed_hide_obj (CHAR_DATA * ch);
void delayed_join_faith (CHAR_DATA * ch);
void delayed_quick_scan (CHAR_DATA * ch);
void delayed_quick_scan_diag (CHAR_DATA * ch);
void delayed_pick (CHAR_DATA * ch);
void delayed_pick_obj (CHAR_DATA * ch);
void delayed_scan (CHAR_DATA * ch);
void delayed_search (CHAR_DATA * ch);
void delayed_skin (CHAR_DATA * ch);
void delayed_study (CHAR_DATA * ch);
void delayed_whap (CHAR_DATA * ch);
void delayed_worship (CHAR_DATA * ch);

int track (CHAR_DATA * ch, int to_room);
int release_prisoner (CHAR_DATA * ch, CHAR_DATA * target);
void name_to_ident (CHAR_DATA * ch, char *buf);
void figure_damage (CHAR_DATA * src, CHAR_DATA * tar,
		    OBJ_DATA * attack_weapon, int off_result,
		    int *damage, int *location);
void combat_results (CHAR_DATA * src, CHAR_DATA * tar,
		     OBJ_DATA * attack_weapon, OBJ_DATA * defense_weapon,
		     OBJ_DATA * broken_eq, int damage, char *location,
		     int off_result, int def_result, int attack_num, char *fd,
		     int off_success, int def_success);
void fix_offense (CHAR_DATA * ch);
void add_second_affect (int type, int seconds, CHAR_DATA * ch,
			OBJ_DATA * obj, const char *info, int info2);
void second_affect_update (void);
void hour_affect_update (void);
void remove_cover (CHAR_DATA *ch, int type);
int under_cover (CHAR_DATA *ch);

int find_door (CHAR_DATA * ch, char *type, char *dir);
char *get_profession_name (int prof_id);
void add_profession_skills (CHAR_DATA * ch, char *skill_list);
int has_required_crafting_skills (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
int is_opening_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
OBJ_DATA *has_key (CHAR_DATA * ch, OBJ_DATA * obj, int key);
int is_brother (CHAR_DATA * ch, CHAR_DATA * tch);
void refresh_zone (void);
int is_leader (CHAR_DATA * src, CHAR_DATA * tar);
void invite_accept (CHAR_DATA * ch, char *argument);
void tashal_prisoner_release (CHAR_DATA * ch);
malloc_t get_perm (int size);
int flee_attempt (CHAR_DATA * ch);
SECOND_AFFECT *get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj);
void remove_second_affect (SECOND_AFFECT * sa);
void map_next_step (CHAR_DATA * ch);
void open_skill (CHAR_DATA * ch, int skill);
int get_trust (CHAR_DATA * ch);
int real_trust (CHAR_DATA * ch);
int is_obj_here (CHAR_DATA * ch, OBJ_DATA * obj, int check);
void jailer_func (CHAR_DATA * ch);
void coronan_arena_check (CHAR_DATA * victim);
void reformat_string (char *source, char **target);
CHAR_DATA *get_pc (char *buf);
CHAR_DATA *load_pc (const char *buf);
void unload_pc (CHAR_DATA * ch);
void pc_to_game (CHAR_DATA * ch);
void create_menu_options (struct descriptor_data *d);
void create_menu_actions (struct descriptor_data *d, char *arg);
void attribute_priorities (struct descriptor_data *d, char *arg);
void sex_selection (struct descriptor_data *d, char *arg);
void race_selection (struct descriptor_data *d, char *arg);
void skill_selection (struct descriptor_data *d, char *argument);
void skill_display (struct descriptor_data *d);
char *read_a_line (FILE * fp);
struct message_data *load_message (char *msg_name, int pc_message,
				   int msg_number);
void unload_message (struct message_data *message);
OBJ_DATA *get_item_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item,
			PHASE_DATA * phase);
void reformat_desc (char *s, char **t);
void reformat_string (char *s, char **t);
void process_quarter_events (void);
void initiate_move (CHAR_DATA * ch);
void clear_moves (CHAR_DATA * ch);
void save_mud_time (time_t game_time);
void initialize_weather_zones (void);
int combat_roll (int ability);
int clear_current_move (CHAR_DATA * ch);
int spell_chill_touch (CHAR_DATA * ch, CHAR_DATA * victim, int sn);
void clear_char (CHAR_DATA * ch);
void weight_change_object (OBJ_DATA * obj, int weight);
int is_blind (CHAR_DATA * ch);
int wakeup (CHAR_DATA * ch);
void remove_object_affect (OBJ_DATA * obj, AFFECTED_TYPE * af);
int eval_att_eq (CHAR_DATA * ch, char **equation);
void knock_out (CHAR_DATA * ch, int seconds);
CHAR_DATA *being_dragged (CHAR_DATA * ch);
void reg_read_crafts (FILE * fp_reg, char *buf);
void update_crafts (CHAR_DATA * ch);
int get_size (CHAR_DATA * ch);
int get_weight (CHAR_DATA * ch);
struct descriptor_data *is_pc_attached (char *buf);
int is_mounted (CHAR_DATA * ch);
void rl_minute_affect_update (void);
void clear_watch (CHAR_DATA * ch);
void show_unread_messages (CHAR_DATA * ch);
char *file_to_string (char *name);
void check_memory ();
OBJ_DATA *get_bow (CHAR_DATA * ch);
int is_incantation (char *argument);
void magic_incantation (CHAR_DATA * ch, char *argument);
int is_obj_in_list (OBJ_DATA * obj, OBJ_DATA * list);
int is_restricted_skill (CHAR_DATA * ch, int skill);
int is_restricted_profession (CHAR_DATA * ch, char *skill_list);
void release_nonplaying_pc (CHAR_DATA * ch);
void release_pc (CHAR_DATA * ch);
void hitches_follow (CHAR_DATA * ch, int dir, int leave_time,
		     int arrive_time);
void dump_rider (CHAR_DATA * rider, int forced);
CHAR_DATA *load_saved_mobiles (CHAR_DATA * ch, char *name);
void save_mobile (CHAR_DATA * mob, FILE * fp, char *save_reason, int extract);
void save_attached_mobiles (CHAR_DATA * ch, int extract);
CHAR_DATA *load_a_saved_mobile (int nVirtual, FILE * fp, bool stable);
int hitch_char (CHAR_DATA * ch, CHAR_DATA * hitch);
void load_rooms (void);
void job_add_affect (CHAR_DATA * ch, int type, int days, int pay_date,
		     int cash, int count, int object_vnum, int employer);
void remove_object_affect (OBJ_DATA * attack_weapon, AFFECTED_TYPE * af);
AFFECTED_TYPE *get_obj_affect_type (OBJ_DATA * obj, int type);
CHAR_DATA *is_guarded (CHAR_DATA * victim, CHAR_DATA * criminal);
void sa_rescue (SECOND_AFFECT * sa);
int decipher_speakign (CHAR_DATA * ch, int skillnum, int skill);
int rescue_attempt (CHAR_DATA * ch, CHAR_DATA * friendPtr);
int is_archer (CHAR_DATA * ch);
int npc_charge (CHAR_DATA * ch, CHAR_DATA * archer);
int morale_broken (CHAR_DATA * ch);
int survival_routine (CHAR_DATA * ch);
void attacker_from_char (CHAR_DATA * ch, ATTACKER_DATA * att);
void threat_from_char (CHAR_DATA * ch, THREAT_DATA * att);
void remove_attacker (CHAR_DATA * ch, CHAR_DATA * threat);
void guard_check (CHAR_DATA * victim);
OBJ_DATA *is_at_table (CHAR_DATA * ch, OBJ_DATA * table);
void print_bit_map (void);
int whisper_it (CHAR_DATA * ch, int skill, char *source, char *target);
int sleep_needed_in_seconds (CHAR_DATA * ch);
void sleep_credit (CHAR_DATA * ch);
void outfit_new_char (CHAR_DATA *ch, ROLE_DATA *role);
void sleep_need (CHAR_DATA * ch);
int would_reveal (CHAR_DATA * ch);
void shadowers_shadow (CHAR_DATA * ch, int to_room, int move_dir);
int could_see (CHAR_DATA * ch, CHAR_DATA * target);
void craft_command (CHAR_DATA * ch, char *command_args,
		    AFFECTED_TYPE * craft_affect);
AFFECTED_TYPE *is_craft_command (CHAR_DATA * ch, char *argument);
void activate_phase (CHAR_DATA * ch, AFFECTED_TYPE * af);
char *obj_short_desc (OBJ_DATA * obj);
char *obj_desc (OBJ_DATA * obj);
OBJ_DATA *split_obj (OBJ_DATA * obj, int count);
void ten_second_update (void);
char *get_line (char **buf, char *ret_buf);
ALIAS_DATA *is_alias (CHAR_DATA * ch, char *argument);
void alias_free (ALIAS_DATA * alias);
void update_weapon_skills (OBJ_DATA * obj);
void missing_item_msg (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item,
		       char *header);

void web_reply (struct descriptor_data *d, char *reply, char *message);
void web_verify_logon (struct descriptor_data *d, char *argument);
void web_process (struct descriptor_data *d, char *argument);
void web_send_craft (struct descriptor_data *d, char *argument);
void web_send_craft_list (struct descriptor_data *d);
void web_process_input (struct descriptor_data *d);

int check_password (const char *pass, const char *encrypted);
char *generate_password (int argc, char **argv);
void web_send_room (struct descriptor_data *d, int room_num);
int can_move (CHAR_DATA * ch);
void magic_add_delayed_affect (CHAR_DATA * victim, int sn, int delay,
			       int duration, int power);
void rl_minute_delayed_affects (void);
int enforcer (CHAR_DATA * ch, CHAR_DATA * crim, int will_act, int witness);
void offline_healing (CHAR_DATA * ch, int since);
int is_hooded (CHAR_DATA * ch);
void add_criminal_time (CHAR_DATA * ch, int zone, int penalty_time);
void remove_affect_type (CHAR_DATA * ch, int type);
int could_see_obj (CHAR_DATA * ch, OBJ_DATA * obj);
OBJ_DATA *get_obj_in_list_vis_not_money (CHAR_DATA * ch, char *name,
					 OBJ_DATA * list);
void init_mysql (void);
void refresh_db_connection (void);
void reload_sitebans (void);
void reload_mob_resets (void);
void target_sighted (CHAR_DATA * ch, CHAR_DATA * target);
void system_log (const char *str, bool error);
void player_log (CHAR_DATA * ch, char *command, char *str);
void weather_and_time (int mode);
void autosave (void);
void autosave_stayputs (void);
void update_delays (void);
void newbie_hints (void);
void set_fighting (struct char_data *ch, struct char_data *vict);
void die (struct char_data *ch);
void break_delay (struct char_data *ch);
int _parse_name (char *arg, char *name);
void equip_newbie (CHAR_DATA * ch);
int load_char_objs (struct char_data *ch, char *name);
void read_obj_suppliment (struct char_data *ch, FILE * fp);
void write_obj_suppliment (struct char_data *ch, FILE * fp);
void unstable (CHAR_DATA * ch, OBJ_DATA * ticket, CHAR_DATA * keeper);
int apply_affect (CHAR_DATA * ch, int sn, int duration, int power);
void craft_prepare_message (CHAR_DATA * ch, char *message, CHAR_DATA * n,
			    CHAR_DATA * N, CHAR_DATA * T, char *phase_msg,
			    OBJ_DATA * tool, OBJ_DATA ** obj_list);
void spell_defenses (CHAR_DATA * defender, CHAR_DATA * target);
int odds_sqrt (int percent);
void activate_resets (CHAR_DATA * ch);
char *type_to_spell_name (int type);
int can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj);
void reset_insert (CHAR_DATA * ch, RESET_DATA * reset);
int is_true_brother (CHAR_DATA * ch, CHAR_DATA * tch);
void m (void);
void mm (char *msg);
void stop_tolls (CHAR_DATA * ch);
int is_human (CHAR_DATA * ch);
int is_toll_paid (CHAR_DATA * ch, int dir);
void write_stats (void);
int is_same_zone (int zone1, int zone2);
TEXT_DATA *add_text (TEXT_DATA ** list, char *filename, char *document_name);
void edit_string (struct descriptor_data *d, std::string argument);
void save_document (CHAR_DATA * ch, char *document_name);
HELP_DATA *load_help_file (FILE * fp);
void load_help (void);
void load_bhelp (void);
HELP_DATA *is_help (CHAR_DATA * ch, HELP_DATA * list, char *topic);
void write_help (char *filename, HELP_DATA * list);
int doc_parse (CHAR_DATA * ch, char *argument, char **start, int *length,
	       int *start_line, int *doc_type);
char *get_text_buffer (CHAR_DATA * ch, TEXT_DATA * list, char *text_name);
TEXT_DATA *find_text (CHAR_DATA * ch, TEXT_DATA * list, char *buf);
void load_documents (void);
void write_text (CHAR_DATA * ch, TEXT_DATA * text);
HELP_DATA *add_help_topics (CHAR_DATA * ch, HELP_DATA ** list,
			    char *argument);
void delete_help_topics (CHAR_DATA * ch, HELP_DATA ** list, char *argument);
TEXT_DATA *add_document (CHAR_DATA * ch, TEXT_DATA ** list, char *argument);
void delete_document (CHAR_DATA * ch, TEXT_DATA ** list, char *argument);
int get_next_coldload_id (int for_a_pc);
void load_dynamic_registry (void);
int can_subtract_money (CHAR_DATA * ch, int farthings_to_subtract,
			int currency_type);
void subtract_money (CHAR_DATA * ch, int farthings_to_subtract,
		     int currency_type);
void keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money);
int redefine_objects (OBJ_DATA * proto);
void money_from_char_to_room (CHAR_DATA * ch, int vnum);
float econ_markup (CHAR_DATA * keeper, OBJ_DATA * obj);
float econ_discount (CHAR_DATA * keeper, OBJ_DATA * obj);
void add_combat_log (CHAR_DATA * ch, char *msg);
int zone_to_econ_zone (int zone);
int get_bite_value (OBJ_DATA * obj);
void make_height (CHAR_DATA * mob);
void make_frame (CHAR_DATA * mob);
int is_name_in_list (char *name, char *list);
char *vnum_to_liquid_name (int vnum);
int obj_mass (OBJ_DATA * obj);
int carrying (CHAR_DATA * ch);
char *tilde_eliminator (char *string);
char *reference_ip (char *guest_name, char *host);
int check_account_flags (char *host);
char *strip_small_minor(char * wounds, CHAR_DATA * ch);
int room_avail(ROOM_DATA *troom, OBJ_DATA *tobj, CHAR_DATA *tch);
int force_enter (CHAR_DATA *tch, ROOM_DATA *troom);


void read_ticket (CHAR_DATA * ch, int tick_num);
void search_ticket (CHAR_DATA * ch, char * chkvalue, int searchtype);
void delete_ticket (CHAR_DATA * ch, int tick_num);

void load_weather_obj(ROOM_DATA *troom);
int weather_object_exists(OBJ_DATA * list, int vnum);

/* Magical effect functions for spellcasting */

void creation_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type);
void creation_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void creation_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void creation_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void creation_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void creation_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void creation_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void creation_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type);
void creation_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type);
void creation_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void creation_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void creation_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void destruction_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			       void *target, int target_type);
void destruction_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void destruction_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void destruction_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void destruction_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void destruction_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void destruction_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void destruction_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			       void *target, int target_type);
void destruction_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type);
void destruction_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void destruction_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void destruction_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void transformation_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				  void *target, int target_type);
void transformation_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void transformation_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void transformation_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void transformation_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				    void *target, int target_type);
void transformation_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void transformation_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				void *target, int target_type);
void transformation_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				  void *target, int target_type);
void transformation_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			       void *target, int target_type);
void transformation_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void transformation_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				void *target, int target_type);
void transformation_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				 void *target, int target_type);
void perception_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void perception_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void perception_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void perception_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void perception_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
				void *target, int target_type);
void perception_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void perception_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type);
void perception_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			      void *target, int target_type);
void perception_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void perception_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void perception_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			    void *target, int target_type);
void perception_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void control_animal_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void control_plant_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void control_image_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void control_light_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void control_darkness_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			     void *target, int target_type);
void control_power_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void control_mind_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type);
void control_spirit_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell,
			   void *target, int target_type);
void control_air_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			int target_type);
void control_earth_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);
void control_fire_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			 int target_type);
void control_water_spell (CHAR_DATA * ch, AFFECTED_TYPE * spell, void *target,
			  int target_type);

#endif // _rpie_protos_h_
