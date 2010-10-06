/*------------------------------------------------------------------------\
|  object_damage.c : Obejct Damage Class             www.middle-earth.us  | 
|  Copyright (C) 2005, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "object_damage.h"
#include "utility.h"

extern rpie::server engine;

std::map<e_material_type, int> material_armour;
std::map<e_material_type, std::string> material_names;

const char *unspecified_conditions[] = {
	"It has been completely destroyed, and is clearly unfit for use.\n",
	"It has been savaged, rendering the item nearly unusable.\n",
	"This item is in shambles, having sustained quite a bit of damage.\n",
	"It is in ill repair, and rather damaged in appearance.\n",
	"It seems to have seen a decent amount of usage.\n",
	"It appears to be nearly flawless in condition.\n"
};

const char *fabric_conditions[] = {
	"All that remains of this item is an unsalvageable mass of torn fabric.\n",
	"What few hale stitches remain between large tears are frayed and worn.\n",
	"It is torn, frayed, and altogether quite piteous in appearance.\n",
	"It bears a few noticeable tears and rips from hard usage.\n",
	"This item has seen some usage, looking frayed and somewhat worn.\n",
	"It is nearly flawless, frayed only very slightly in a few spots.\n"
};

const char *leather_conditions[] = {
	"This item has been utterly ravaged, and is entirely unsalvageable.\n",
	"What little undamaged material remains is dark with age and use.\n",
	"It has been cracked and worn in many places with time and abuse.\n",
	"It bears a number of small cracks and scars, looking quite used.\n",
	"It has seen some usage, appearing cracked and worn in a few spots.\n",
	"It is in excellent condition, bearing only a few blemishes and scars.\n"
};

const char *wood_conditions[] = {
	"All that remains of this item is a few jagged splinters of wood.\n",
	"This object has been cracked and splintered to the point of unusability.\n",
	"It has been cracked in a number of places, bearing numerous splinters.\n",
	"A few small but noticeable cracks have been made in this object.\n",
	"Splintered in a few places, this looks to have seen some usage.\n",
	"It is nearly flawless, only a small splinter or two out of place.\n"
};

const char *bone_conditions[] = {
	"This item has been cracked, chipped and flaked into oblivion.\n",
	"Nearly unusable, it has been cracked and chipped beyond recognition.\n",
	"It is cracked and chipped in a significant number of places.\n",
	"A few small, very faint cracks can be seen on this item.\n",
	"Nearly without flaw, only a small bit of wear is visible.\n",
};

short skill_to_damage_name_index (ushort n_skill);

void
initialize_materials (void)
{
	material_names[MATERIAL_TYPE_UNDEFINED] = "undefined";
	material_names[MATERIAL_BURLAP] = "burlap";
	material_names[MATERIAL_LINEN] = "linen";
	material_names[MATERIAL_WOOL] = "wool";
	material_names[MATERIAL_COTTON] = "cotton";
	material_names[MATERIAL_SILK] = "silk";
	material_names[MATERIAL_VELVET] = "velvet";
	material_names[MATERIAL_CLOTH] = "cloth";
	material_names[MATERIAL_THIN_LEATHER] = "thin leather";
	material_names[MATERIAL_PLAIN_LEATHER] = "plain leather";
	material_names[MATERIAL_THICK_LEATHER] = "thick leather";
	material_names[MATERIAL_COURBOULLI] = "courboulli";
	material_names[MATERIAL_LEATHER] = "leather";
	material_names[MATERIAL_COPPER] = "copper";
	material_names[MATERIAL_BRONZE] = "bronze";
	material_names[MATERIAL_LEAD] = "lead";
	material_names[MATERIAL_BRASS] = "brass";
	material_names[MATERIAL_TIN] = "tin";
	material_names[MATERIAL_MITHRIL] = "mithril";
	material_names[MATERIAL_IRON] = "iron";
	material_names[MATERIAL_STEEL] = "steel";
	material_names[MATERIAL_METAL] = "metal";
	material_names[MATERIAL_IVORY] = "ivory";
	material_names[MATERIAL_WOOD] = "wood";



	material_armour[MATERIAL_TYPE_UNDEFINED] = 0;
	material_armour[MATERIAL_BURLAP] = 0;
	material_armour[MATERIAL_LINEN] = 0;
	material_armour[MATERIAL_WOOL] = 0;
	material_armour[MATERIAL_COTTON] = 0;
	material_armour[MATERIAL_SILK] = 0;
	material_armour[MATERIAL_VELVET] = 0;
	material_armour[MATERIAL_CLOTH] = 0;
	material_armour[MATERIAL_THIN_LEATHER] = 0;
	material_armour[MATERIAL_PLAIN_LEATHER] = 0;
	material_armour[MATERIAL_THICK_LEATHER] = 1;
	material_armour[MATERIAL_COURBOULLI] = 1;
	material_armour[MATERIAL_LEATHER] = 0;
	material_armour[MATERIAL_COPPER] = 1;
	material_armour[MATERIAL_LEAD] = 1;
	material_armour[MATERIAL_BRASS] = 1;
	material_armour[MATERIAL_TIN] = 1;
	material_armour[MATERIAL_MITHRIL] = 5;
	material_armour[MATERIAL_BRONZE] = 2;
	material_armour[MATERIAL_IRON] = 2;
	material_armour[MATERIAL_STEEL] = 2;
	material_armour[MATERIAL_METAL] = 1;
	material_armour[MATERIAL_IVORY] = 0;
	material_armour[MATERIAL_WOOD] = 0;
}

// This is a debug command, designed to be run once on the BP to convert
// all existing objects to have a material of some sort.
void
do_materialhack (CHAR_DATA *ch, char * argument, int cmd)
{
	extern OBJ_DATA *obj_tab[OBJECT_ZONE_SIZE];
	OBJ_DATA *obj;
	for (int i = 0; i <= 999; i++)
	{
		for (obj = obj_tab[i]; obj; obj = obj->hnext)
		{
			for (std::map<e_material_type, std::string>::iterator it = material_names.begin(); it != material_names.end(); it++)
			{
				std::string names = obj->name;
				if (names.find(it->second) != std::string::npos)
				{
					obj->material = it->first;
					break;
				}

				if (is_tagged(obj->name))
				{
					switch GET_MATERIAL_TYPE(obj)
					{
					case (1 << 1): // TEXTILE
						if (isname ("burlap", obj->name))
							obj->material = MATERIAL_BURLAP;
						else if (isname ("linen", obj->name))
							obj->material = MATERIAL_LINEN;
						else if (isname ("wool", obj->name))
							obj->material = MATERIAL_WOOL;
						else if (isname ("cotton", obj->name))
							obj->material = MATERIAL_COTTON;
						else if (isname ("silk", obj->name))
							obj->material = MATERIAL_SILK;
						else if (isname ("velvet", obj->name))
							obj->material = MATERIAL_VELVET;
						else
							obj->material = MATERIAL_CLOTH;
						break;
					case (1 << 2): // LEATHER
						if (isname ("thin leather", obj->name))
							obj->material = MATERIAL_THIN_LEATHER;
						else if (isname ("plain leather", obj->name))
							obj->material = MATERIAL_PLAIN_LEATHER;
						else if (isname ("thick leather", obj->name))
							obj->material = MATERIAL_THICK_LEATHER;
						else if (isname ("courboulli", obj->name))
							obj->material = MATERIAL_COURBOULLI;
						else if (isname ("leather", obj->name))
							obj->material = MATERIAL_LEATHER;
						else if (isname ("hide", obj->name))
							obj->material = MATERIAL_LEATHER;
						else if (isname ("deerskin", obj->name))
							obj->material = MATERIAL_LEATHER;
						else if (isname ("deerhide", obj->name))
							obj->material = MATERIAL_LEATHER;
						else if (isname ("pelt", obj->name))
							obj->material = MATERIAL_LEATHER;
						else if (isname ("rawhide", obj->name))
							obj->material = MATERIAL_LEATHER;
						else
							obj->material = MATERIAL_LEATHER;
						break;
					case (1 << 3): // WOOD
						obj->material = MATERIAL_WOOD;
						break;
					case (1 << 4): // METAL
						if (isname ("copper", obj->name))
							obj->material = MATERIAL_COPPER;
						else if (isname ("bronze", obj->name))
							obj->material = MATERIAL_BRONZE;
						else if (isname ("iron", obj->name))
							obj->material = MATERIAL_IRON;
						else if (isname ("steel", obj->name))
							obj->material = MATERIAL_STEEL;
						else if (isname ("brass", obj->name))
							obj->material = MATERIAL_BRASS;
						else if (isname ("tin", obj->name))
							obj->material = MATERIAL_TIN;
						else if (isname ("lead", obj->name))
							obj->material = MATERIAL_LEAD;
						else if (isname ("metal", obj->name))
							obj->material = MATERIAL_METAL;
						else
							obj->material = MATERIAL_METAL;
						break;

					case (1 << 5): // STONE
					case (1 << 6): // GLASS
					case (1 << 11): // OTHER
						if (isname ("ivory", obj->name))
							obj->material = MATERIAL_IVORY;
						else if (isname ("stone", obj->name))
							obj->material = MATERIAL_STONE;
						else if (isname ("chert", obj->name))
							obj->material = MATERIAL_STONE;
						else if (isname ("flint", obj->name))
							obj->material = MATERIAL_STONE;
						else if (isname ("obsidian", obj->name))
							obj->material = MATERIAL_STONE;
						else if (isname ("ceramic", obj->name))
							obj->material = MATERIAL_CERAMIC;
						else if (isname ("brick", obj->name))
							obj->material = MATERIAL_CERAMIC;
						break;
					}
					break;
				}
			}
		}
	}
}

/*------------------------------------------------------------------------\
|  new()                                                                  |
|                                                                         |
|  Returns a pointer to the newly allocated instance of object_damage.    |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new ()
{
	OBJECT_DAMAGE *thisPtr = NULL;

	/* TODO: Remove this when we're ready to go live with damage */
		if (engine.in_play_mode())
		return NULL;

	
	thisPtr = new OBJECT_DAMAGE;
	thisPtr->next = NULL;
	thisPtr->source = (DAMAGE_TYPE) 0;
	thisPtr->material = (MATERIAL_TYPE) 0;
	thisPtr->severity = (DAMAGE_SEVERITY) 0;
	thisPtr->impact = 0;
	thisPtr->name = 0;
	thisPtr->lodged = 0;
	thisPtr->when = 0;


	return thisPtr;
}


/*------------------------------------------------------------------------\
|  new_init()                                                             |
|                                                                         |
|  Returns a pointer to the newly allocated and initialized instance of   |
|  object damage.
	Severity	Impact
	miniscule:	 1
	small:		 2-3
	minor:		 4-5
	moderate:	 6-10
	large:		 11-20
	deep:		 21-30
	massive:	 31-40
	terrible:	 41 + 
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new_init (DAMAGE_TYPE source, ushort impact,
						 MATERIAL_TYPE material, int lodged)
{
	OBJECT_DAMAGE *thisPtr = NULL;

	/* Exemption List (sketchy at best) */
	if ((source == DAMAGE_FIST)
		|| ((source == DAMAGE_FREEZE) && (material <= MATERIAL_LEATHER))
		|| ((source == DAMAGE_WATER) && (material <= MATERIAL_LEATHER))
		|| ((source == DAMAGE_BLUNT) && (material <= MATERIAL_LEATHER)))
	{

		return NULL;

	}

	if (!(thisPtr = object_damage__new ()))
	{
		return NULL;
	}

	thisPtr->source = source;
	thisPtr->material = material;
	thisPtr->severity = (DAMAGE_SEVERITY) ((impact > 8) ? 
						(MIN (8, ((impact - 1) / 10) + 4)) :
						((impact / 2) + 1));
	thisPtr->impact = (source == DAMAGE_BLOOD) ? 0 : impact;
	thisPtr->name = number (0, 3); //used to reference third index of damage_name[12][5][4]
	thisPtr->lodged = lodged;
	thisPtr->when = time (0);

	return thisPtr;
}


/*------------------------------------------------------------------------\
|  delete()                                                               |
|                                                                         |
|  Frees the memory of this object damage instance and returns the next.  |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__delete (OBJECT_DAMAGE * thisPtr)
{
	OBJECT_DAMAGE *next = NULL;

	if (thisPtr)
	{
		next = thisPtr->next;
		free_mem (thisPtr);
	}

	return next;
}


/*------------------------------------------------------------------------\
|  get_sdesc()                                                            |
|                                                                         |
|  Returns a short description of the damage instance.                    |
\------------------------------------------------------------------------*/
/**
 broad_damage_type: cloth=0, leather=1, metals=2, ivoery/wood =3, ceramic/stone=4
 str_desc: a small puncture 
 **/
char *
object_damage__get_sdesc (OBJECT_DAMAGE * thisPtr)
{
	ushort n_sdesc_length = 0;
	char *str_sdesc = NULL;
	extern const char *damage_severity[DAMAGE_SEVERITY_MAX + 1];
	extern const char *damage_name[12][5][4];

	if (thisPtr->source == DAMAGE_PERMANENT)
		return NULL;

	int broad_damage_type;
	broad_damage_type = object__get_material(thisPtr);

	n_sdesc_length = strlen (damage_severity[thisPtr->severity])
		+
		strlen (damage_name[thisPtr->source]
	[(thisPtr->material < MATERIAL_IRON) ? 0 : 1][thisPtr->name]) + 5;
	
	str_sdesc = new char[n_sdesc_length];
	sprintf (str_sdesc, "%s %s %s",
		(isvowel (damage_severity[thisPtr->severity][0])) ? "an" : "a",
		damage_severity[thisPtr->severity],
		damage_name[thisPtr->
		source][broad_damage_type][thisPtr->name]);
	return str_sdesc;
}


/*------------------------------------------------------------------------\
|  write_to_file()                                                        |
|                                                                         |
|  Export a string that describes this damage instance.                   |
\------------------------------------------------------------------------*/
int
object_damage__write_to_file (OBJECT_DAMAGE * thisPtr, FILE * fp)
{
	return fprintf (fp, "Damage     %d %d %d %d %d %d %d\n",
		thisPtr->source, thisPtr->material,
		thisPtr->severity, thisPtr->impact,
		thisPtr->name, thisPtr->lodged, thisPtr->when);
}


/*------------------------------------------------------------------------\
|  read_from_file()                                                       |
|                                                                         |
|  Import a string that describes a damage instance.                      |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__read_from_file (FILE * fp)
{
	OBJECT_DAMAGE *thisPtr = NULL;

	if (!(thisPtr = object_damage__new ()))
	{
		return NULL;
	}

	/* TODO: Make this an fscanf statement? */
	thisPtr->source = (DAMAGE_TYPE) fread_number (fp);
	thisPtr->material = (MATERIAL_TYPE) fread_number (fp);
	thisPtr->severity = (DAMAGE_SEVERITY) fread_number (fp);
	thisPtr->impact = fread_number (fp);
	thisPtr->name = fread_number (fp);
	thisPtr->lodged = fread_number (fp);
	thisPtr->when = fread_number (fp);

	return thisPtr;
}


/*------------------------------------------------------------------------\
|                                                                         |
|  STRING STORAGE                                                         |
|                                                                         |
\------------------------------------------------------------------------*/

const char *damage_type[DAMAGE_TYPE_MAX + 1] = {
	"stab",
	"pierce",
	"chop",
	"blunt",
	"slash",
	"freeze",			/*  5 */
	"burn",
	"tooth",
	"claw",
	"fist",
	"blood",			/* 10 */
	"water",
	"lightning",
	"permanent",			/* 13 */
	"repair",
	"\n"
};


const char *material_type[MATERIAL_TYPE_MAX + 1] = {
	"undefined",
	"burlap",
	"wool",
	"linen",
	"cotton",
	"silk",
	"leather",
	"steel",
	"iron",
	"wood",
	"ivory",
	"\n"
};


const char *damage_severity[DAMAGE_SEVERITY_MAX + 1] = {
	"unnoticeable",
	"miniscule",
	"small",
	"minor",
	"moderate",
	"large",
	"deep",
	"massive",
	"terrible",
	"\n"
};



short
skill_to_damage_name_index (ushort n_skill)
{
	switch (n_skill)
	{

	case SKILL_TEXTILECRAFT:
		return 0;
	case SKILL_HIDEWORKING:
		return 1;
	case SKILL_METALCRAFT:
		return 2;
	case SKILL_WOODCRAFT:
		return 3;
	case SKILL_STONECRAFT:
		return 4;
	case SKILL_DYECRAFT:
		return -1;
	case SKILL_GLASSWORK:
		return -1;
	case SKILL_GEMCRAFT:
		return -1;
	case SKILL_POTTERY:
		return -1;
	default:
		return -1;

	}
}

const char *damage_name[12][5][4] = {

	{				/*  DAMAGE_STAB  */
		{"puncture", "hole", "perforation", "piercing"}, /* vs cloth */
		{"puncture", "hole", "perforation", "piercing"}, /* vs hide */
		{"puncture", "gouge", "perforation", "rupture"}, /* vs metal */
		{"hole", "split", "nick", "split"}, /* vs wood */
		{"chip", "scratch", "scrape", "nick"} /* vs stone */
	},
	{				/*  DAMAGE_PIERCE  */
		{"puncture", "hole", "perforation", "piercing"}, /* vs cloth */
		{"puncture", "hole", "perforation", "piercing"}, /* vs hide */
		{"puncture", "gouge", "perforation", "rupture"}, /* vs metal */
		{"hole", "split", "nick", "split"}, /* vs wood */
		{"chip", "scratch", "scrape", "nick"} /* vs stone */
	},
	{				/*  DAMAGE_CHOP  */
		{"slice", "cut", "slash", "gash"}, /* vs cloth */
		{"slice", "cut", "slash", "gash"}, /* vs hide */
		{"nick", "chip", "rivet", "gash"}, /* vs metal */
		{"split", "nick", "gouge", "hack"}, /* vs wood */
		{"chip", "nick", "scrape", "crack"} /* vs stone */
	},
	{				/*  DAMAGE_BLUNT  */
		{"tear", "rip", "rent", "tatter"}, /* vs cloth */
		{"tear", "split", "rent", "tatter"}, /* vs hide */
		{"gouge", "rivet", "nick", "dent"}, /* vs metal */
		{"dent", "splinter", "crack", "split"}, /* vs wood */
		{"chip", "nick", "crack", "dent"} /* vs stone */
	},
	{				/*  DAMAGE_SLASH  */
		{"slice", "cut", "slash", "gash"}, /* vs cloth */
		{"slice", "cut", "slash", "gash"}, /* vs hide */
		{"nick", "chip", "rivet", "gash"}, /* vs metal */
		{"split", "nick", "gouge", "hack"}, /* vs wood */
		{"chip", "nick", "scrape", "crack"} /* vs stone */
	},
	{				/*  DAMAGE_FREEZE  */
		{"stain", "tarnish", "tear", "tarnish"}, /* vs cloth */
		{"crack", "tarnish", "tarnish", "tear"}, /* vs hide */
		{"crack", "split", "dent", "tarnish"}, /* vs metal */
		{"warp", "split", "warp", "splint"}, /* vs wood */
		{"crack", "chip", "crack", "chip"} /* vs stone */
	},
	{				/*  DAMAGE_BURN  */
		{"scorching", "charring", "blackening", "hole"}, /* vs cloth */
		{"scorching", "charring", "blackening", "hole"}, /* vs hide */
		{"scorching", "charring", "blackening", "tarnish"}, /* vs metal */
		{"charring", "blackening", "scorching", "cindering"}, /* vs wood */
		{"blackening", "scorching", "blackening", "scorching"} /* vs stone */
	},
	{				/*  DAMAGE_TOOTH  */
		{"tear", "rip", "rent", "tatter"}, /* vs cloth */
		{"tear", "split", "rent", "tatter"}, /* vs hide */
		{"gouge", "rivet", "nick", "dent"}, /* vs metal */
		{"dent", "splinter", "crack", "split"}, /* vs wood */
		{"chip", "nick", "crack", "dent"} /* vs stone */
	},
	{				/*  DAMAGE_CLAW  */
		{"tear", "rip", "rent", "tatter"}, /* vs cloth */
		{"tear", "split", "rent", "tatter"}, /* vs hide */
		{"gouge", "rivet", "nick", "dent"}, /* vs metal */
		{"dent", "splinter", "crack", "split"}, /* vs wood */
		{"chip", "nick", "crack", "dent"} /* vs stone */
	},
	{				/*  DAMAGE_FIST  */
		{"dent", "deformation", "dimple", "impression"},	/* vs metal */
		{"dent", "deformation", "dimple", "impression"},		/* vs wood */
		{"dent", "deformation", "dimple", "impression"},		/* vs cloth */
		{"dent", "deformation", "dimple", "impression"},		/* vs hide */
		{"dent", "deformation", "dimple", "impression"}		/* vs stone */
	},
	{				/*  DAMAGE_BLOOD  */
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},	/* vs cloth */
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},	/* vs hide */
		{"bloodstain", "bloodstain", "blood-splatter", "tarnish"},	/* vs metal */
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},		/* vs wood */
		{"bloodstain", "bloodstain", "blood-splatter", "stain"}		/* vs stone */
	},
	{				/*  DAMAGE_WATER  */
		{"stain", "discoloration", "blemish", "splotch"},	/* vs cloth */
		{"stain", "discoloration", "blemish", "splotch"},	/* vs hide */
		{"tarnish", "corrosion", "flaking", "deterioration"},	/* vs metal */
		{"stain", "discoloration", "blemish", "splotch"},		/* vs wood */
		{"stain", "discoloration", "blemish", "splotch"}		/* vs stone */
	}
};
