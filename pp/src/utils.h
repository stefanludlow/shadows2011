//////////////////////////////////////////////////////////////////////////////
//
/// utils.h - Inline Utility Macros
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2005-2006 C. W. McHenry
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


#ifndef _rpie_utils_h_
#define _rpie_utils_h_

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = toupper(*(st)), st)
#define LOW(st)  (*(st) = tolower(*(st)), st)
#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) alloc ((number) * sizeof(type), 16)))\
		{ perror("CREATE: alloc failure"); abort(); } } while(0)
#define IS_SET(flag,bit)  ((flag) & (bit))
#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->affected_by, (skill)) )

#define IS_NIGHT ( !sun_light )

#define IS_LIGHT(room)  !is_dark (room)

#define TOGGLE_BIT(var,bit)  ((var) = (var) ^ (bit) )
#define TOGGLE(flag, bit) { if ( IS_SET (flag, bit) ) \
                               flag &= ~bit; \
                            else \
                               flag |= bit; \
			   }

#define CAN_SEE(sub, obj)	( ( (IS_LIGHT (obj->room) || \
	get_affect (sub, MAGIC_AFFECT_INFRAVISION) || IS_SET (sub->affected_by, AFF_INFRAVIS)) && \
	\
	(!get_affect (obj, MAGIC_AFFECT_INVISIBILITY) || \
	get_affect (sub, MAGIC_AFFECT_SEE_INVISIBLE)) && \
	\
	!((obj->room->sector_type == SECT_WOODS || \
	obj->room->sector_type == SECT_FOREST || \
	obj->room->sector_type == SECT_HILLS) && \
	(get_affect (obj, MAGIC_AFFECT_CONCEALMENT) && \
	!get_affect (sub, MAGIC_AFFECT_SENSE_LIFE))) && \
	\
	(!get_affect (obj, MAGIC_HIDDEN) || are_grouped (obj, sub)) && \
        \
	!is_blind (sub) && \
	!IS_SET (obj->flags, FLAG_WIZINVIS) && \
	(weather_info[obj->room->zone].state != HEAVY_SNOW || IS_SET (obj->room->room_flags, INDOORS)) ) || \
	\
	!IS_MORTAL (sub) )

#define HSHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "his" : "her") : "its") : "its")

#define HSSH(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "he" : "she") : "it") : "it")

#define HMHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "him" : "her") : "it") : "it")
#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch) (IS_SET((ch)->act, ACT_ISNPC))
#define IS_LINKDEAD(ch) (!(ch)->desc)

#define GET_TRUST(ch)	(get_trust (ch))
#define IS_IMPLEMENTOR(ch)	( GET_TRUST(ch) > 5 )
#define IS_GUIDE(ch)	(!IS_NPC(ch) ? (IS_MORTAL(ch) && !IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? ch->pc->is_guide : 0) : 0)
#define IS_NEWBIE(ch)	(!is_newbie(ch) ? 0 : 1)
#define IS_MORTAL(ch)	(!GET_TRUST(ch))
#define GET_POS(ch)     ((ch)->position)
#define GET_COND(ch, i) ((ch)->conditions[(i)])
#define GET_NAME(ch)    ((ch)->tname)
#define GET_NAMES(ch)    ((ch)->name)
#define GET_AGE(ch)     (age(ch).year)
#define GET_STR(ch)     ((ch)->tmp_str)
#define GET_DEX(ch)     ((ch)->tmp_dex)
#define GET_INT(ch)     ((ch)->tmp_intel)
#define GET_WIL(ch)		((ch)->tmp_wil)
#define GET_AUR(ch)		((ch)->tmp_aur)
#define GET_CON(ch)     ((ch)->tmp_con)
#define GET_AGI(ch)		((ch)->tmp_agi)
#define GET_AC(ch)      ((ch)->armor)
#define GET_HIT(ch)     ((ch)->hit)
#define GET_MAX_HIT(ch) ((ch)->max_hit)
#define GET_MOVE(ch)    ((ch)->move)
#define GET_MAX_MOVE(ch) ((ch)->max_move)
#define GET_CASH(ch)    ((ch)->cash)
#define GET_SEX(ch)     ((ch)->sex)
#define GET_SPEAKS(ch)  ((ch)->speaks)
#define GET_OFFENSE(ch) ((ch)->offense)
#define GET_DAMROLL(ch) ((ch)->mob->damroll)
#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)
#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)
#define GET_FLAG(ch,flag) (IS_SET ((ch)->flags, flag))

/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)	can_see_obj (sub, obj)

#define IS_OBJ_VIS(sub, obj)										\
	( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) || 	\
	     get_affect (sub, MAGIC_AFFECT_SEE_INVISIBLE) ) &&					\
		 !is_blind (sub))                                           \
         || obj->location == WEAR_BLINDFOLD                         \
	     || !IS_MORTAL(sub))

#define GET_MATERIAL_TYPE(obj) (determine_material(obj))

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))
#define IS_WEARABLE(obj) ((obj)->obj_flags.wear_flags &            \
				(ITEM_WEAR_BODY | ITEM_WEAR_LEGS | ITEM_WEAR_ARMS))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)
#define OBJ_MASS(obj) obj_mass(obj)

#define CAN_CARRY_W(ch) calc_lookup (ch, REG_MISC, MISC_MAX_CARRY_W)
#define CAN_CARRY_N(ch) (IS_MOUNT (ch) ? 0 : calc_lookup (ch, REG_MISC, MISC_MAX_CARRY_N))

#define IS_CARRYING_W(ch) carrying(ch)
#define IS_CARRYING_N(ch) ((ch)->carry_items)

#define IS_ENCUMBERED(ch) (GET_STR (ch) * enc_tab [1].str_mult_wt < IS_CARRYING_W (ch))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_MERCHANT(mob) (IS_SET((mob)->hmflags,HM_KEEPER))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)  (CAN_SEE((vict), (ch)) ? \
	char_short((ch)) : "someone")

/* #define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")
*/

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	obj_short_desc (obj) : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define IS_OUTSIDE(ch) (!IS_SET((ch)->room->room_flags,INDOORS) && \
			ch->room->sector_type != SECT_INSIDE && ch->room->sector_type != SECT_UNDERWATER)

#define EXIT(ch, door)  ((ch->room) ? (ch)->room->dir_option[door] : NULL)

#define CAN_GO(ch, door) (EXIT(ch,door)  &&  (EXIT(ch,door)->to_room != NOWHERE) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_FLEE_SOMEWHERE(ch) (CAN_GO(ch, NORTH) || CAN_GO(ch, SOUTH) || \
								CAN_GO(ch, EAST) || CAN_GO(ch, WEST))

#define SWIM_ONLY(room) ((room)->sector_type == SECT_OCEAN || \
					 	(room)->sector_type == SECT_REEF ||  \
						(room)->sector_type == SECT_RIVER || \
						(room)->sector_type == SECT_LAKE || \
						(room)->sector_type == SECT_UNDERWATER || \
						is_room_affected (room->affects, MAGIC_ROOM_FLOOD) )

#define IS_SWIMMING(ch) SWIM_ONLY((ch)->room)

#define IS_DROWNING(ch) (IS_SWIMMING(ch) && IS_MORTAL(ch)) && \
				(get_affect (ch, AFFECT_HOLDING_BREATH) && \
				get_affect (ch, AFFECT_HOLDING_BREATH)->a.spell.duration <= 0)

#define IS_FROZEN(zone) (IS_SET(zone_table[(zone)].flags,Z_FROZEN))

#define IS_SUBDUEE(ch) (is_he_here (ch, (ch)->subdue, 0) && \
                        GET_FLAG (ch, FLAG_SUBDUEE))
#define IS_SUBDUER(ch) (is_he_here (ch, (ch)->subdue, 0) && \
                        GET_FLAG (ch, FLAG_SUBDUER))

#define IS_MOUNT(ch) (IS_SET (ch->act, ACT_MOUNT))

#define IS_RIDER(ch) (is_he_here (ch, (ch)->mount, 0) && \
                      !IS_SET (ch->act, ACT_MOUNT))
#define IS_RIDEE(ch) (is_he_here (ch, (ch)->mount, 0) && \
                      IS_SET (ch->act, ACT_MOUNT))

#define IS_HITCHER(ch) (is_he_here (ch, (ch)->hitchee, 0) &&	\
						ch->hitchee->hitcher == ch)
#define IS_HITCHEE(ch) (is_he_here (ch, (ch)->hitcher, 0) &&	\
						ch->hitcher->hitchee == ch)

#define IS_TABLE(obj) (GET_ITEM_TYPE (obj) == ITEM_CONTAINER && \
                       IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))


#define SEND_TO_Q(messg, desc)  write_to_q ((messg), (desc) ? &(desc)->output : NULL)

#ifdef NOVELL
#define sigmask(m) ((unsigned long) 1 << ((m) - 1 ))
#endif

#endif // _rpie_utils_h_
