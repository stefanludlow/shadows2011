/*------------------------------------------------------------------------\
|  save.c : World Save Module                         www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#ifndef MACOSX
#include <malloc.h>
#endif

#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "protos.h"
#include "decl.h"

#define POSSESS_UNDEF   0
#define	POSSESS_CARRY	1
#define POSSESS_WEAR	2
#define POSSESS_ROOM	3

static int last_wc = POSSESS_UNDEF;
static int last_eqloc;
static int last_eqlev;
static int file_eof_found = 0;
static int save_stop_obj_processing = 0;

int load_char_objects;

extern int fgetc (FILE * fp);
extern int fread_number (FILE * fp);
extern char *fread_word (FILE * fp);

void write_char_data (CHAR_DATA * ch, FILE * fp);
void write_obj_data (OBJ_DATA * obj,
					 char *wc, int pos, int objstack, FILE * fp);
int load_player (char *name, CHAR_DATA * ch, int load_type);
void read_obj_list (int cur_wc,
					int cur_eqloc,
					int cur_eqlev,
					CHAR_DATA * ch,
					OBJ_DATA * holder, FILE * fp, int room_num);
OBJ_DATA *fread_obj (FILE * fp);
void read_aliases (CHAR_DATA * ch, FILE * fp);

char *
unspace (char *s)
{
	char *result;
	char *orig;

	if (!s)
	{
		((int *) -1)[0] = 1;
	}

	if (*s != ' ')
		return s;

	orig = s;

	result = (char *) alloc (strlen (s) + 1, 14);

	while (s && *s == ' ')
		s++;

	strcpy (result, s);

	mem_free (orig);

	return result;
}

void
fwrite_a_obj (OBJ_DATA * obj, FILE * fp)
{
	int affect_count = 0;
	int modifiers = 1;
	AFFECTED_TYPE *af = NULL;
	WOUND_DATA *wound;
	OBJECT_DAMAGE *damage;
	LODGED_OBJECT_INFO *lodged;
	OBJ_DATA *proto = NULL;

	proto = vtoo (obj->nVirtual);

	if (!proto)
		return;

	for (af = obj->xaffected; af; af = af->next)
	{

		if (af->a.spell.location)
		{
			affect_count++;
			continue;
		}
	}

	if (obj->clan_data)
		modifiers += 1;

	if (obj->omote_str)
		modifiers++;

	if (obj->loaded)
		modifiers += 1;

	for (wound = obj->wounds; wound; wound = wound->next)
	{
		modifiers++;
	}

	for (damage = obj->damage; damage; damage = damage->next)
	{
		modifiers++;
	}

	for (lodged = obj->lodged; lodged; lodged = lodged->next)
	{
		modifiers++;
	}

	if (obj->size)
		modifiers++;

	if (obj->count > 1)
		modifiers++;

	if (obj->obj_flags.extra_flags)
		modifiers++;

	if (obj->obj_flags.set_cost)
		modifiers++;

	if (obj->obj_flags.type_flag == ITEM_DRINKCON)
		modifiers++;

	if (obj->obj_timer)		/* obj_flags.timer */
		modifiers++;

	if (obj->clock && (obj->morphto >= 0) && obj->morphTime)
		modifiers += 3;

	if (obj->book_title && obj->title_skill)
		modifiers += 4;

	if (obj->var_color && str_cmp (obj->var_color, "none"))
		modifiers++;

	if (obj->item_wear)
		modifiers++;

	if (obj->obj_flags.type_flag != ITEM_DRINKCON)
	{

		if (proto->name && obj->name && *obj->name)
			if (strcmp (obj->name, proto->name))
				modifiers++;

		if (proto->short_description && obj->short_description
			&& *obj->short_description)
			if (strcmp (obj->short_description, proto->short_description))
				modifiers++;

		if (proto->description && obj->description && *obj->description)
			if (strcmp (obj->description, proto->description))
				modifiers++;

		if (proto->full_description && obj->full_description
			&& *obj->full_description)
			if (strcmp (obj->full_description, proto->full_description))
				modifiers++;

	}

	/* if there is no proto type key but there is an object key, save it */
	bool saveMkey = false;
	if (!(proto->desc_keys))
	{
		if (!(obj->desc_keys))
		{
			saveMkey=false; // no proto, no current setting
		}
		else
		{
			saveMkey = strlen(obj->desc_keys) > 1; // no proto, verify length on current setting
		}
	}
	else
	{
		if (!(obj->desc_keys))
		{
			saveMkey=false; // currently no mkey despite on on proto. Let it revert to proto.
		}
		else // both valid so strcmp possible. Save if different.
		{
			// if strcmp is not zero, there is a non-zero difference between them and 
			// thus the object's version should be saved
			saveMkey = ( strcmp(obj->desc_keys, proto->desc_keys) != 0 );
		}

	}
	if (saveMkey)
		modifiers++;

	if (obj->coldload_id)
		modifiers++;

	fprintf (fp, "Id       %d %d %d\n", obj->nVirtual, affect_count, modifiers);

	if (obj->coldload_id)
		fprintf (fp, "coldload       %d\n", obj->coldload_id);

	if (obj->obj_flags.type_flag != ITEM_DRINKCON)
	{

		if (proto->name && obj->name && *obj->name)
			if (strcmp (obj->name, proto->name))
				fprintf (fp, "name %s~\n", obj->name);

		if (proto->short_description && obj->short_description
			&& *obj->short_description)
			if (strcmp (obj->short_description, proto->short_description))
				fprintf (fp, "short %s~\n", obj->short_description);

		if (proto->description && obj->description && *obj->description)
			if (strcmp (obj->description, proto->description))
				fprintf (fp, "long %s~\n", obj->description);

		if (proto->full_description && obj->full_description
			&& *obj->full_description)
			if (strcmp (obj->full_description, proto->full_description))
				fprintf (fp, "full %s~\n", obj->full_description);

		if (saveMkey) // reuse value computed above
			fprintf (fp, "desc_keys %s~\n", obj->desc_keys);
	}


	if (obj->book_title && obj->title_skill)
	{
		fprintf (fp, "book_title     %s~\n", obj->book_title);
		fprintf (fp, "title_skill    %d\n", obj->title_skill);
		fprintf (fp, "title_language %d\n", obj->title_language);
		fprintf (fp, "title_script   %d\n", obj->title_script);
	}

	if (obj->loaded)
		fprintf (fp, "loaded %d\n", obj->loaded->nVirtual);

	if (obj->obj_flags.extra_flags)
		fprintf (fp, "extraflags %d\n", obj->obj_flags.extra_flags);

	if (obj->item_wear)
		fprintf (fp, "itemwear %d\n", obj->item_wear);

	if (obj->obj_flags.set_cost)
		fprintf (fp, "set_cost %d\n", obj->obj_flags.set_cost);

	fprintf (fp, "values %d %d %d %d %d %d\n", obj->o.od.value[0],
		obj->o.od.value[1], obj->o.od.value[2], obj->o.od.value[3],
		obj->o.od.value[4], obj->o.od.value[5]);

	if (obj->obj_flags.type_flag == ITEM_DRINKCON)
		fprintf (fp, "weight   %d\n", obj->obj_flags.weight);

	if (obj->size)
		fprintf (fp, "size     %d\n", obj->size);

	if (obj->count > 1)
		fprintf (fp, "count    %d\n", obj->count);

	if (obj->obj_timer)
		fprintf (fp, "timer    %d\n", obj->obj_timer);

	if (obj->clock && (obj->morphto >= 0) && obj->morphTime)
	{
		fprintf (fp, "clock	%d\n", obj->clock);
		fprintf (fp, "morphto	%d\n", obj->morphto);
		fprintf (fp, "morphTime	%d\n", obj->morphTime);
	}

	if (obj->var_color && str_cmp (obj->var_color, "none"))
		fprintf (fp, "VarColor  %s~\n", obj->var_color);

	if (obj->omote_str)
	{
		fprintf (fp, "OmoteStr   %s~\n", obj->omote_str);
	}

	if (obj->wounds)
	{
		for (wound = obj->wounds; wound; wound = wound->next)
		{
			/* AVERT A CRASH BUG WITH CORRUPT WOUND DATA */
			if (wound->healerskill < -1 || wound->healerskill > 200
				|| !isalpha (wound->location[0])
				|| !isalpha (wound->type[0])
				|| !isalpha (wound->severity[0]) || !isalpha (wound->name[0]))
			{

				continue;
			}
			fprintf (fp, "Wound      %s %s %s %s %d %d %d %d %d %d %d\n",
				wound->location, wound->type, wound->severity, wound->name,
				wound->damage, wound->bleeding, wound->poison,
				wound->infection, wound->healerskill, wound->lasthealed,
				wound->lastbled);
		}

	}

	if (obj->damage)
	{
		for (damage = obj->damage; damage; damage = damage->next)
		{
			object_damage__write_to_file (damage, fp);
		}
	}

	if (obj->lodged)
	{
		for (lodged = obj->lodged; lodged; lodged = lodged->next)
			fprintf (fp, "Lodged     %s %d\n", lodged->location, lodged->vnum);
	}

	if (obj->clan_data)
	{
		fprintf (fp, "Clan\n%s~\n%s~\n",
			obj->clan_data->name,
			obj->clan_data->rank);
	}


	for (af = obj->xaffected; af; af = af->next)
	{
		if (af->a.spell.location)
		{
			fprintf (fp, "Afflocmod %d %d %d %d %d %d %d\n",
				af->a.spell.location,
				af->a.spell.modifier,
				af->a.spell.duration,
				af->a.spell.bitvector,
				af->a.spell.t, af->a.spell.sn, af->type);
			continue;
		}
	}
}

void
write_obj_data (OBJ_DATA * obj, char *wc, int pos, int objstack, FILE * fp)
{
	if (!obj)
		return;

	if (!vtoo (obj->nVirtual))
		return;

	if (obj->nVirtual == 95)	// Leantos.
		return;

	fprintf (fp, "%s %d %d\n", wc, pos, objstack);

	/* Save object */

	fwrite_a_obj (obj, fp);

	/* Save everything this object contains */

	if (obj->contains)
		write_obj_data (obj->contains, wc, pos, objstack + 1, fp);

	/* Save the next object in the list */

	if ((obj->location == -1 && obj->next_content && obj != obj->next_content))
		write_obj_data (obj->next_content, wc, pos, objstack, fp);

	if (obj->next_content && obj->next_content == obj)
		obj->next_content = NULL;

}

OBJ_DATA *
fread_obj (FILE * fp)
{
	int obj_vnum;
	int affect_count;
	int modifiers = 0;
	int i, page = 0;
	int old_money = 0;
	long nFilePosition = 0;
	char *p;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	WRITING_DATA *writing = NULL;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *taf;
	WOUND_DATA *wound = NULL;
	OBJECT_DAMAGE *damage = NULL;
	LODGED_OBJECT_INFO *lodged = NULL;
	OBJ_CLAN_DATA *tclan = NULL;


	if (strcmp (p = fread_word (fp), "Id"))
	{
		abort ();
	}

	obj_vnum = fread_number (fp);
	affect_count = fread_number (fp);

	if (obj_vnum == VNUM_MONEY)
	{
		obj_vnum = VNUM_PENNY;
		old_money = 1;
	}

	if (!(obj = load_object_full (obj_vnum,false)))
	{
		if (!(obj = load_object (666)))
		{
			if (!obj)
			{
				return NULL;
			}
		}
	}

	/* Remove prototype affects */

	while ((af = obj->xaffected))
	{
		obj->xaffected = af->next;
		mem_free (af);
	}

	fscanf (fp, "%d", &modifiers);

	for (; modifiers; modifiers--)
	{

		nFilePosition = ftell (fp);
		p = fread_word (fp);

		if (!strcmp (p, "name"))
		{
			fgetc (fp);
			obj->name = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "short"))
		{
			fgetc (fp);
			obj->short_description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "loaded"))
		{
			obj->loaded = vtoo (fread_number (fp));
			continue;
		}
		else if (!strcmp (p, "coldload"))
		{
			obj->coldload_id = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "extraflags"))
		{
			obj->obj_flags.extra_flags = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "itemwear"))
		{
			obj->item_wear = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "set_cost"))
		{
			obj->obj_flags.set_cost = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "long"))
		{
			fgetc (fp);
			obj->description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "full"))
		{
			fgetc (fp);
			obj->full_description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "values"))
		{
			obj->o.od.value[0] = fread_number (fp);
			obj->o.od.value[1] = fread_number (fp);
			obj->o.od.value[2] = fread_number (fp);
			obj->o.od.value[3] = fread_number (fp);
			obj->o.od.value[4] = fread_number (fp);
			obj->o.od.value[5] = fread_number (fp);
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK
				|| GET_ITEM_TYPE (obj) == ITEM_PARCHMENT)
				writing = obj->writing;
			continue;
		}
		else if (!strcmp (p, "weight"))
		{
			obj->obj_flags.weight = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "size"))
		{
			obj->size = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "count"))
		{
			obj->count = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "timer"))
		{
			obj->obj_timer = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "clock"))
		{
			obj->clock = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "morphTime"))
		{
			obj->morphTime = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "morphto"))
		{
			obj->morphto = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "VarColor"))
		{
			obj->var_color = unspace (fread_string (fp));
			continue;
		}
		else if (!strcmp (p, "book_title"))
		{
			obj->book_title = unspace (fread_string (fp));
			continue;
		}
		else if (!str_cmp (p, "title_skill"))
		{
			obj->title_skill = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "title_script"))
		{
			obj->title_script = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "title_language"))
		{
			obj->title_language = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "page:"))
		{
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			writing = obj->writing;
			page = fread_number (fp);
			for (i = 1; i; i++)
			{
				if (i == page)
					break;
				else
					writing = writing->next_page;
			}
			continue;
		}
		else if (!strcmp (p, "message:"))
		{
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->message = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "author:"))
		{
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->author = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "date:"))
		{
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->date = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "language:"))
		{
			writing->language = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "script:"))
		{
			writing->script = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "ink:"))
		{
			if (GET_ITEM_TYPE (obj) == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->ink = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "skill:"))
		{
			writing->skill = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "desc_keys"))
		{
			fgetc (fp);
			obj->desc_keys = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "OmoteStr"))
		{
			obj->omote_str = unspace (fread_string (fp));
			continue;
		}
		else if (!strcmp (p, "Wound"))
		{
			if (!obj->wounds)
			{
				CREATE (obj->wounds, WOUND_DATA, 1);
				wound = obj->wounds;
				obj->wounds->next = NULL;
			}
			else
			{
				CREATE (wound->next, WOUND_DATA, 1);
				wound = wound->next;
				wound->next = NULL;
			}

			wound->location = add_hash (fread_word (fp));
			wound->type = add_hash (fread_word (fp));
			wound->severity = add_hash (fread_word (fp));
			wound->name = add_hash (fread_word (fp));
			wound->damage = fread_number (fp);
			wound->bleeding = fread_number (fp);
			wound->poison = fread_number (fp);
			wound->infection = fread_number (fp);
			wound->healerskill = fread_number (fp);
			wound->lasthealed = fread_number (fp);
			wound->lastbled = fread_number (fp);
			continue;
		}

		else if (!strcmp (p, "Clan"))
		{
			CREATE (tclan, OBJ_CLAN_DATA, 1);
			obj->clan_data->next = NULL;
			obj->clan_data->name = add_hash (fread_string (fp));
			obj->clan_data->rank = add_hash (fread_string (fp));
			continue;
		}


		else if (!strcmp (p, "Damage"))
		{

			if ((damage = object_damage__read_from_file (fp)))
			{
				damage->next = obj->damage;
				obj->damage = damage;
			}
			continue;

		}
		else if (!strcmp (p, "Lodged"))
		{
			if (!obj->lodged)
			{
				CREATE (obj->lodged, LODGED_OBJECT_INFO, 1);
				lodged = obj->lodged;
				obj->lodged->next = NULL;
			}
			else
			{
				CREATE (lodged->next, LODGED_OBJECT_INFO, 1);
				lodged = lodged->next;
				lodged->next = NULL;
			}
			lodged->location = add_hash (fread_word (fp));
			lodged->vnum = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "Super_vnum"))
		{
			obj->super_vnum = fread_number(fp);
		}
		else if (!strcmp (p, "DONE"))
		{
			fseek (fp, nFilePosition, SEEK_SET);
			break;
		}
		else if (!strcmp (p, "Id"))
		{
			fseek (fp, nFilePosition, SEEK_SET);
			break;
		}

	}//for (; modifiers; modifiers--)

	if (old_money)
	{
		obj->count = obj->o.od.value[0];
		obj->o.od.value[0] = 0;
	}

	if (obj->nVirtual == VNUM_PENNY || obj->nVirtual == VNUM_FARTHING)
		name_money (obj);

	/* read_object probably created associated some affects with the
	new object.  Lets erase all those and put in saved affects */

	for (i = 0; obj && i < affect_count; i++)
	{

		if (strcmp (fread_word (fp), "Afflocmod"))
		{
			return (NULL);	/* Forfiet remainder of objects */
		}

		CREATE (af, AFFECTED_TYPE, 1);

		af->next = NULL;

		fscanf (fp, "%d %d %d %d %d %d %d\n",
			&af->a.spell.location,
			&af->a.spell.modifier,
			&af->a.spell.duration,
			&af->a.spell.bitvector,
			&af->a.spell.t, &af->a.spell.sn, &af->type);

		if (!obj->xaffected)
			obj->xaffected = af;
		else
		{
			taf = obj->xaffected;
			while (taf->next)
				taf = taf->next;
			taf->next = af;
		}
	}

	if (GET_ITEM_TYPE (obj) == ITEM_DRINKCON ||
		GET_ITEM_TYPE (obj) == ITEM_LIGHT ||
		GET_ITEM_TYPE (obj) == ITEM_FOUNTAIN)
		if (!(tobj = vtoo (obj->o.drinkcon.liquid)) ||
			GET_ITEM_TYPE (tobj) != ITEM_FLUID)
			obj->o.drinkcon.liquid = vtoo (obj->nVirtual)->o.drinkcon.liquid;

	if (!IS_SET (obj->obj_flags.extra_flags, ITEM_NEWSKILLS))
		update_weapon_skills (obj);

	/* Weapons and Armour should default to prototype values at loadtime */

	if (GET_ITEM_TYPE (obj) == ITEM_ARMOR || GET_ITEM_TYPE (obj) == ITEM_WEAPON || GET_ITEM_TYPE(obj) == ITEM_MISSILE)
	{
		for (int index = 0; index < 6; index++)
		{
			obj->o.od.value[index] = vtoo (obj->nVirtual)->o.od.value[index];
		}
	}

	/* Record what object previously was */

	if (obj->nVirtual == 666 && !obj->o.od.value[0])
		obj->o.od.value[0] = obj_vnum;

	return obj;
}

int
fread_look_obj_header (int *wc, int *eqloc, int *eqlev, FILE * fp)
{
	char *ptr;

	if (last_wc != POSSESS_UNDEF)
	{
		*wc = last_wc;
		*eqloc = last_eqloc;
		*eqlev = last_eqlev;
		return (1);
	}

	if (file_eof_found || save_stop_obj_processing)
		return (0);

	ptr = fread_word (fp);

	if (*ptr == '\0')		/* Probably the end of file mark */
		return (0);

	else if (!strcmp (ptr, "CARRY"))
		*wc = POSSESS_CARRY;

	else if (!strcmp (ptr, "WEAR"))
		*wc = POSSESS_WEAR;

	else if (!strcmp (ptr, "ROOM"))
		*wc = POSSESS_ROOM;

	else if (!strcmp (ptr, "DONE"))
	{
		save_stop_obj_processing = 1;
		return (0);
	}

	else if (!strcmp (ptr, "END"))
	{
		file_eof_found = 1;
		return (0);
	}

	else
		return (0);

	*eqloc = fread_number (fp);
	*eqlev = fread_number (fp);

	last_wc = *wc;
	last_eqloc = *eqloc;
	last_eqlev = *eqlev;

	return (1);
}

void
read_obj_list (int cur_wc, int cur_eqloc, int cur_eqlev, CHAR_DATA * ch,
			   OBJ_DATA * holder, FILE * fp, int room_num)
{
	int eqloc = 0;
	int eqlev = 0;
	int wc = 0;
	OBJ_DATA *obj = NULL;

	while (1)
	{

		if (!fread_look_obj_header (&wc, &eqloc, &eqlev, fp))
			return;

		if (cur_eqlev > eqlev)
			return;

		if ((cur_wc != wc || cur_eqloc != eqloc) && eqloc != -1
			&& eqloc != WEAR_PRIM && eqloc != WEAR_SEC && eqloc != WEAR_BOTH)
		{
			return;
		}

		if ((cur_eqlev < eqlev))
			read_obj_list (wc, eqloc, eqlev, ch, obj, fp, room_num);
		else
		{
			last_wc = POSSESS_UNDEF;

			obj = fread_obj (fp);

			if (!obj)
				return;
			else if (holder)
			{
				obj_to_obj (obj, holder);
			}
			else if (wc == POSSESS_ROOM)
			{
				if (!vtor (room_num))
					printf ("Room %d does not exist for object %d.\n",
					room_num, obj->nVirtual);
				else
					obj_to_room (obj, room_num);
			}
			else if (cur_eqloc == -1)
			{
				obj->in_room = NOWHERE;
				obj_to_char (obj, ch);
				if (eqloc == WEAR_PRIM || eqloc == WEAR_SEC
					|| eqloc == WEAR_BOTH)
					equip_char (ch, obj, eqloc);
			}
			else
			{
				obj->carried_by = ch;
				obj->in_room = NOWHERE;
				equip_char (ch, obj, eqloc);
			}

		}			/* cur_eqlev < eqlev */
	}				/* while (1) */
}

OBJ_DATA *
get_equipped (CHAR_DATA * ch, int location)
{
	OBJ_DATA *obj;

	for (obj = ch->equip; obj; obj = obj->next_content)
		if (obj->location == location)
			return obj;

	return NULL;
}

void
write_obj_suppliment (CHAR_DATA * ch, FILE * fp)
{
	int i;

	if (ch->right_hand)
	{
		write_obj_data (ch->right_hand, "CARRY", ch->right_hand->location, 0,
			fp);
	}

	if (ch->left_hand)
	{
		write_obj_data (ch->left_hand, "CARRY", ch->left_hand->location, 0, fp);
	}

	for (i = 0; i < MAX_WEAR; i++)
	{
		if (i == WEAR_PRIM || i == WEAR_BOTH || i == WEAR_SEC
			|| i == WEAR_SHIELD)
			continue;
		write_obj_data (get_equipped (ch, i), "WEAR", i, 0, fp);
	}

	fprintf (fp, "END\n");
}

void
read_obj_suppliment (CHAR_DATA * ch, FILE * fp)
{
	int i;

	file_eof_found = 0;
	save_stop_obj_processing = 0;
	last_wc = POSSESS_UNDEF;
	load_char_objects = 1;	/* Global variable */

	ch->right_hand = NULL;
	ch->left_hand = NULL;
	ch->equip = NULL;

	read_obj_list (POSSESS_CARRY, -1, 0, ch, NULL, fp, 0);

	for (i = 0; i < MAX_WEAR; i++)
	{
		if (i == WEAR_PRIM || i == WEAR_SEC || i == WEAR_BOTH)
			continue;
		read_obj_list (POSSESS_WEAR, i, 0, ch, NULL, fp, 0);
	}

	load_char_objects = 0;
}

#define		TYPE_STRING			1
#define		TYPE_INT			2
#define		TYPE_SKILL			3
#define		TYPE_PVARIC_SPELL		4
#define		TYPE_END			5
#define		TYPE_AFFECT			6
#define		TYPE_SHORTINT			7
#define		TYPE_DREAM			8
#define		TYPE_OBSOLETE			9
#define		TYPE_LONG			10
#define		TYPE_SUBCRAFT			11
#define		TYPE_ALIAS			12
#define		TYPE_WOUND			13
#define		TYPE_STORED_PC			14
#define		TYPE_LODGED			15
#define		TYPE_SPELL			16
#define		TYPE_ENCHANTMENT		17
#define		TYPE_VOICE			18
#define		TYPE_HOODED			19
#define		TYPE_NANNY			21
#define		TYPE_APPCOST			22
#define		TYPE_ROLE			23
#define		TYPE_ROLE_SUMMARY		24
#define		TYPE_ROLE_BODY			25
#define		TYPE_ROLE_DATE			26
#define		TYPE_ROLE_POSTER		27
#define		TYPE_ROLE_COST			28
#define		TYPE_MAIL_MESSAGE		29
#define		TYPE_IP				30
#define		TYPE_AFFECTEDBY			31
#define		TYPE_NEWSLETTER			32
#define		TYPE_OWNER			33
#define		TYPE_STABLED			34
#define		TYPE_HAS_INV			35
#define		TYPE_SPAWN			36
#define		TYPE_MOUNT			37
#define		TYPE_RESET			38
#define		TYPE_RESETZ			39
#define		TYPE_DONE			40

struct key_data
{
	char key[AVG_STRING_LENGTH];
	int key_type;
	void *ptr;
};


int
save_char (CHAR_DATA * ch, int save_objs)
{
	if (IS_NPC (ch) || IS_SET (ch->flags, FLAG_GUEST) || !ch->tname
		|| !*ch->tname)
	{
		return 1;
	}

	save_char_mysql (ch);

	if (save_objs)
		save_char_objs (ch, ch->tname);

	if (ch->room)
		save_attached_mobiles (ch, 0);

	return 0;
}

void
load_online_stats ()
{
	FILE *fp;

	if (!(fp = fopen ("online_stats", "r")))
	{
		system_log ("Error opening online_stats!", true);
		return;
	}
	fscanf (fp, "%d\n", &count_max_online);
	char *tmpdate = fread_string (fp);
	strcpy (max_online_date, tmpdate);
	mem_free (tmpdate); // char* 
	fclose (fp);
}

void
load_leantos ()
{
	FILE *fp;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	int toroom = 0, inroom = 0;
	struct room_prog *t, *old, *tmp;

	if (!(fp = fopen ("../regions/leantos", "r")))
		return;

	while (!feof (fp))
	{
		fscanf (fp, "leanto from room %d to room %d\n", &inroom, &toroom);
		obj = load_object (95);
		obj_to_room (obj, inroom);
		sprintf (buf,
			"vstr You enter the lean-to.\nostr %%n enters the lean-to.\natecho %d %%n enters the lean-to.\ntrans %d\natlook %d\n",
			toroom, toroom, toroom);
		CREATE (t, struct room_prog, 1);
		t->next = NULL;
		t->command = add_hash ("enter");
		t->keys = add_hash ("lean-to leanto");
		t->prog = add_hash (buf);
		if (!vtor (inroom)->prg)
		{
			vtor (inroom)->prg = t;
		}
		else
		{
			old = vtor (inroom)->prg;
			tmp = old;
			while (tmp)
			{
				old = tmp;
				tmp = tmp->next;
			}
			old->next = t;
		}
	}

	fclose (fp);
}

void
load_mobiles ()
{
	CHAR_DATA *mob;
	WOUND_DATA *wound, *tmpwound;
	FILE *fp, *op;
	char *key;
	char name[MAX_STRING_LENGTH], location[MAX_STRING_LENGTH];
	char severity[MAX_STRING_LENGTH], type[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH], line[MAX_STRING_LENGTH];
	int i, num, to_room = 0, sn;

	mob = NULL;

	for (i = 0; i <= 99; i++)
	{
		sprintf (buf, "save/mobiles/mobs.%d", i);
		if (!(fp = fopen (buf, "r+")))
			continue;
		for (; !feof (fp);)
		{
			key = fread_word (fp);
			if (!str_cmp (key, "END"))
			{
				if (mob)
				{
					mob->act |= ACT_STAYPUT;
					char_to_room (mob, to_room);
					if (!mob->room)
					{
						extract_char (mob);
					}
					mob = NULL;
				}
			}
			else if (!str_cmp (key, "Keywords"))
			{
				mob->name = unspace (fread_string (fp));
			}
			else if (!str_cmp (key, "Owner"))
			{
				mob->mob->owner = unspace (fread_string (fp));
			}
			else if (!str_cmp (key, "SDesc"))
			{
				mob->short_descr = unspace (fread_string (fp));
			}
			else if (!str_cmp (key, "Skill"))
			{
				sn = index_lookup (skills, (key = fread_word (fp)));
				if (sn == -1)
					sn = index_lookup (skills, key);
				if (sn == -1)
					continue;
				if (sn > MAX_SKILLS)
					continue;
				else
					mob->skills[sn] = fread_number (fp);
			}
			else if (!str_cmp (key, "LDesc"))
			{
				mob->long_descr = unspace (fread_string (fp));
			}
			else if (!str_cmp (key, "DatabaseNumber"))
			{
				num = fread_number (fp);
				mob = new_char (0);
				mob = load_mobile (num);
			}
			else if (!str_cmp (key, "ColdloadID"))
			{
				num = fread_number (fp);
				mob->coldload_id = num;
			}
			else if (!str_cmp (key, "CurrentRoom"))
			{
				num = fread_number (fp);
				to_room = num;
			}
			else if (!str_cmp (key, "SpawnLocation"))
			{
				num = fread_number (fp);
				mob->mob->spawnpoint = num;
			}

			else if (!str_cmp (key, "Wound"))
			{
				CREATE (wound, WOUND_DATA, 1);
				wound->next = NULL;
				sscanf (line,
					"Wound              %s %s %s %s %d %d %d %d %d %d %d\n",
					location, type, severity, name, &wound->damage,
					&wound->bleeding, &wound->poison, &wound->infection,
					&wound->healerskill, &wound->lasthealed,
					&wound->lastbled);
				wound->location = add_hash (unspace (fread_word (fp)));
				wound->type = add_hash (unspace (fread_word (fp)));
				wound->severity = add_hash (unspace (fread_word (fp)));
				wound->name = add_hash (unspace (fread_word (fp)));
				wound->damage = fread_number (fp);
				wound->bleeding = fread_number (fp);
				wound->poison = fread_number (fp);
				wound->infection = fread_number (fp);
				wound->healerskill = fread_number (fp);
				wound->lasthealed = fread_number (fp);
				wound->lastbled = fread_number (fp);
				if (!mob->wounds)
					mob->wounds = wound;
				else
				{
					tmpwound = mob->wounds;
					while (tmpwound->next)
						tmpwound = tmpwound->next;
					tmpwound->next = wound;
				}
			}

			else if (!str_cmp (key, "Morph_time"))
			{
				num = fread_number (fp);
				mob->morph_time = num;
			}

			else if (!str_cmp (key, "HasInventory"))
			{
				op = NULL;
				*buf = '\0';
				sprintf (buf, "save/mobiles/inventory/%d", mob->coldload_id);
				if (!(op = fopen (buf, "r+")))
					continue;
				read_obj_suppliment (mob, op);
				fclose (op);
			}
			else if (!str_cmp (key, "IsHooded"))
			{
				mob->affected_by |= AFF_HOODED;
			}
			else if (!str_cmp (key, "ENDFILE"))
				break;
			else
				continue;
		}
		fclose (fp);
	}
}

void
save_player_rooms ()
{
	FILE *fp;
	ROOM_DATA *room;
	char buf[MAX_STRING_LENGTH];

	for (room = full_room_list; room; room = room->lnext)
	{
		if (room->psave_loaded && room->contents)
		{
			sprintf (buf, "save/rooms/%d", room->nVirtual);
			fp = fopen (buf, "w");
			if (!fp)
				continue;
			write_obj_data (room->contents, "ROOM", 0, 0, fp);
			fprintf (fp, "DONE\n");
			fclose (fp);
		}
		else if (room->psave_loaded && !room->contents)
		{
			sprintf (buf, "save/rooms/%d", room->nVirtual);
			unlink (buf);
		}
	}
}

void
do_saverooms (CHAR_DATA * ch, char *argument, int cmd)
{
	save_player_rooms ();
	send_to_char ("Ok.\n\r", ch);
}

void
load_save_rooms ()
{
	int room_num;
	int line_no = 1;
	int rooms = 0;
	int room_obj_count;
	OBJ_DATA *obj;
	FILE *rp;
	char buf[MAX_STRING_LENGTH];

	//load_mysql_save_rooms();
	//return;

	if (!(rp = fopen ("save_rooms", "r")))
	{
		perror ("save_rooms");
		return;
	}

	file_eof_found = 0;
	save_stop_obj_processing = 0;
	last_wc = POSSESS_UNDEF;

	while (1)
	{
		if (!fgets (buf, 81, rp))
		{
			perror ("save_rooms");
			fclose (rp);
			return;
		}

		if (!str_cmp (buf, "\n"))
			continue;

		line_no++;

		if (*buf == '#')
		{

			rooms++;

			sscanf (buf, "#%d", &room_num);

			save_stop_obj_processing = 0;

			read_obj_list (POSSESS_ROOM, 0, 0, NULL, NULL, rp, room_num);

			if (vtor (room_num))
			{

				for (room_obj_count = 0, obj = vtor (room_num)->contents;
					obj; obj = obj->next_content)
					room_obj_count++;
			}
		}

		else if (!str_cmp (buf, "END\n"))
		{
			sprintf (buf, "Save rooms restored:  %d", rooms);
			break;
		}

		else
			printf ("Unexpected line: %s\n", buf);
	}

	fclose (rp);
}

void
load_save_room (ROOM_DATA * room)
{
	FILE *rp;
	char buf[MAX_STRING_LENGTH];

	if (!room || room->psave_loaded)
		return;

	room->psave_loaded = 1;

	sprintf (buf, "save/rooms/%d", room->nVirtual);

	if (!(rp = fopen (buf, "r")))
	{
		return;
	}

	file_eof_found = 0;
	save_stop_obj_processing = 0;
	last_wc = POSSESS_UNDEF;

	read_obj_list (POSSESS_ROOM, 0, 0, NULL, NULL, rp, room->nVirtual);

	fclose (rp);
}

CHAR_DATA *
load_saved_mobiles (CHAR_DATA * ch, char *name)
{
	int nVirtual;
	CHAR_DATA *mob;
	CHAR_DATA *last_mob;
	CHAR_DATA *return_mob = NULL;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char hookup[MAX_STRING_LENGTH];

	if (!(fp = fopen (name, "r")))
	{
		return NULL;
	}

	last_mob = ch;

	while (fgets (buf, 256, fp))
	{

		if (*buf == ' ' || *buf == '\n')
			fgets (buf, 255, fp);

		if (sscanf (buf, "%d %s", &nVirtual, hookup) != 2)
		{
			fclose (fp);
			return return_mob;
		}

		mob = load_a_saved_mobile (nVirtual, fp, true);

		if (!return_mob)
			return_mob = mob;

		char_to_room (mob, mob->in_room);

		if (!last_mob)
			;

		else if (!str_cmp (hookup, "HITCH"))
			hitch_char (last_mob, mob);

		else if (!str_cmp (hookup, "RIDE"))
		{
			last_mob->mount = mob;
			mob->mount = last_mob;
		}

		last_mob = mob;
	}

	fclose (fp);

	return return_mob;
}

CHAR_DATA *
load_a_saved_mobile (int nVirtual, FILE * fp, bool stable)
{
	int i;
	int n = 0;
	int sn;
	int last_key = 0;
	int num_keys;
	int stabled = 0, checks = 0;
	struct time_info_data healing_time;
	char *p, *p2;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	AFFECTED_TYPE *af;
	CHAR_DATA *mob, *tch;
	WOUND_DATA *wound, *tmpwound;
	LODGED_OBJECT_INFO *lodged, *tmplodged;

	struct key_data key_table[] = {
		{"Room", TYPE_INT, NULL},
		{"SDesc", TYPE_STRING, NULL},
		{"LDesc", TYPE_STRING, NULL},
		{"Keys", TYPE_STRING, NULL},
		{"FightMode", TYPE_INT, NULL},
		{"Clans", TYPE_STRING, NULL},
		{"ColdloadId", TYPE_INT, NULL},
		{"Flags", TYPE_INT, NULL},
		{"Moves", TYPE_INT, NULL},
		{"Hits", TYPE_INT, NULL},
		{"Speaks", TYPE_INT, NULL},
		{"AffectedBy", TYPE_INT, NULL},
		{"Position", TYPE_INT, NULL},
		{"Act", TYPE_INT, NULL},
		{"Cond0", TYPE_INT, NULL},
		{"Cond1", TYPE_INT, NULL},
		{"Cond2", TYPE_INT, NULL},
		{"Sex", TYPE_INT, NULL},
		{"Height", TYPE_INT, NULL},
		{"Frame", TYPE_INT, NULL},
		{"AccessFlags", TYPE_INT, NULL},
		{"Morph_time", TYPE_INT, NULL},
		{"SpawnLocation", TYPE_SPAWN, NULL},
		{"Rider", TYPE_MOUNT, NULL},
		{"Mount", TYPE_MOUNT, NULL},
		{"ResetCmd", TYPE_RESET, NULL},
		{"ResetZone", TYPE_RESETZ, NULL},
		{"Skill", TYPE_SKILL, NULL},
		{"Owner", TYPE_OWNER, NULL},
		{"Stabled", TYPE_STABLED, NULL},
		{"Affect", TYPE_AFFECT, NULL},
		{"Wound", TYPE_WOUND, NULL},
		{"Lodged", TYPE_LODGED, NULL},
		{"HasInventory", TYPE_HAS_INV, NULL},
		{"DONE", TYPE_DONE, NULL},
		{"End", TYPE_END, NULL},
		{"\0", TYPE_INT, NULL}
	};

	if (!(mob = load_mobile (nVirtual)))
	{
		return NULL;
	}

	key_table[n++].ptr = &mob->in_room;
	key_table[n++].ptr = &mob->short_descr;
	key_table[n++].ptr = &mob->long_descr;
	key_table[n++].ptr = &mob->name;
	key_table[n++].ptr = &mob->fight_mode;
	key_table[n++].ptr = &mob->clans;
	key_table[n++].ptr = &mob->coldload_id;
	key_table[n++].ptr = &mob->flags;
	key_table[n++].ptr = &mob->move;
	key_table[n++].ptr = &mob->hit;
	key_table[n++].ptr = &mob->speaks;
	key_table[n++].ptr = &mob->affected_by;
	key_table[n++].ptr = &mob->position;
	key_table[n++].ptr = &mob->act;
	key_table[n++].ptr = &mob->intoxication;
	key_table[n++].ptr = &mob->hunger;
	key_table[n++].ptr = &mob->thirst;
	key_table[n++].ptr = &mob->sex;
	key_table[n++].ptr = &mob->height;
	key_table[n++].ptr = &mob->frame;
	key_table[n++].ptr = &mob->mob->access_flags;
	key_table[n++].ptr = &mob->morph_time;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;

	for (num_keys = 0; *key_table[num_keys].key;)
		num_keys++;

	for (; !feof (fp);)
	{

		if (!(p = fread_word (fp)))
		{
			free_char (mob);
			return NULL;
		}

		for (i = last_key;
			i < last_key + num_keys &&
			str_cmp (key_table[i % num_keys].key, p);)
			i++;

		i = i % num_keys;

		if (str_cmp (key_table[i].key, p))
		{
			continue;
		}

		if (key_table[i].key_type == TYPE_SKILL ||
			key_table[i].key_type == TYPE_AFFECT ||
			key_table[i].key_type == TYPE_WOUND ||
			key_table[i].key_type == TYPE_LODGED ||
			key_table[i].key_type == TYPE_OWNER ||
			key_table[i].key_type == TYPE_STABLED ||
			key_table[i].key_type == TYPE_HAS_INV ||
			key_table[i].key_type == TYPE_SPAWN ||
			key_table[i].key_type == TYPE_RESET ||
			key_table[i].key_type == TYPE_RESETZ ||
			key_table[i].key_type == TYPE_MOUNT)
			last_key = i;
		else
			last_key = i + 1;

		if (key_table[i].key_type == TYPE_INT)
			*(int *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_SHORTINT)
			*(shortint *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_STRING)
			*(char **) key_table[i].ptr = unspace (fread_string (fp));

		else if (key_table[i].key_type == TYPE_LONG)
			*(long *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_OBSOLETE)
			sn = fread_number (fp);

		else if (key_table[i].key_type == TYPE_SPAWN)
			mob->mob->spawnpoint = fread_number (fp);

		else if (key_table[i].key_type == TYPE_RESET)
			mob->mob->reset_cmd = fread_number (fp);

		else if (key_table[i].key_type == TYPE_RESETZ)
			mob->mob->reset_zone = fread_number (fp);

		else if (key_table[i].key_type == TYPE_STABLED)
		{
			stabled = fread_number (fp);
			if (!stable)
				continue;
			healing_time = real_time_passed (time (0) - stabled, 0);
			checks = (healing_time.day * 12);
			checks += (healing_time.hour / 2);
			for (wound = mob->wounds; wound; wound = tmpwound)
			{
				tmpwound = wound->next;
				for (i = 0; i < checks; i++)
					natural_healing_check (mob, wound);
			}
		}
		else if (key_table[i].key_type == TYPE_HAS_INV)
			read_obj_suppliment (mob, fp);

		else if (key_table[i].key_type == TYPE_SKILL)
		{

			sn = index_lookup (skills, (p = fread_word (fp)));

			if (sn == -1)
				sn = index_lookup (skills, p);

			if (sn == -1)
			{
				return NULL;
			}

			if (sn > MAX_SKILLS)
				printf ("Skill Num # %d (learned %d) out of range.\n",
				sn, fread_number (fp));
			else
				mob->skills[sn] = fread_number (fp);
		}

		else if (key_table[i].key_type == TYPE_OWNER)
		{
			mob->mob->owner = unspace (fread_string (fp));
		}

		else if (key_table[i].key_type == TYPE_AFFECT)
		{

			af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

			fscanf (fp, "%d %d %d %d %d %d %d\n",
				&af->type,
				&af->a.spell.duration,
				&af->a.spell.modifier,
				&af->a.spell.location,
				&af->a.spell.bitvector, &af->a.spell.sn, &af->a.spell.t);
			af->next = NULL;

			affect_to_char (mob, af);
		}

		else if (key_table[i].key_type == TYPE_MOUNT)
		{
			if (!(tch = get_char_id (fread_number (fp))))
				continue;
			if (mob->mount || tch->mount)
				continue;
			tch->mount = mob;
			mob->mount = tch;
		}

		else if (key_table[i].key_type == TYPE_WOUND)
		{
			CREATE (wound, WOUND_DATA, 1);
			wound->next = NULL;

			wound->location = add_hash (fread_word (fp));
			wound->type = add_hash (fread_word (fp));
			wound->severity = add_hash (fread_word (fp));
			wound->name = add_hash (fread_word (fp));
			wound->damage = fread_number (fp);
			wound->bleeding = fread_number (fp);
			wound->poison = fread_number (fp);
			wound->infection = fread_number (fp);
			wound->healerskill = fread_number (fp);
			wound->lasthealed = fread_number (fp);
			wound->lastbled = fread_number (fp);

			if (!mob->wounds)
				mob->wounds = wound;
			else
			{
				tmpwound = mob->wounds;
				while (tmpwound->next)
					tmpwound = tmpwound->next;
				tmpwound->next = wound;
			}
		}

		else if (key_table[i].key_type == TYPE_LODGED)
		{
			lodged =
				(LODGED_OBJECT_INFO *) alloc (sizeof (LODGED_OBJECT_INFO), 36);
			lodged->next = NULL;

			lodged->location = add_hash (fread_word (fp));
			lodged->vnum = fread_number (fp);

			if (!mob->lodged)
				mob->lodged = lodged;
			else
			{
				tmplodged = mob->lodged;
				while (tmplodged->next)
					tmplodged = tmplodged->next;
				tmplodged->next = lodged;
			}

		}

		else if (key_table[i].key_type == TYPE_DONE)
		{

			mob->flags &= ~(FLAG_ENTERING | FLAG_LEAVING);

			fix_offense (mob);

			if (get_affect (mob, MAGIC_AFFECT_SLEEP))
				GET_POS (mob) = SLEEP;

			mob->time.logon = time (0);

			return mob;
		}

		else if (key_table[i].key_type == TYPE_END)
		{
			p = mob->clans;
			p2 = p;
			mob->clans = str_dup ("");

			while (1)
			{

				p = one_argument (p, buf);	/* flags     */
				p = one_argument (p, buf2);	/* clan name */

				if (!*buf2)
					break;

				add_clan_id (mob, buf2, buf);
			}

			mem_free (p2);

			return mob;
		}
	}

	free_char (mob);

	return NULL;
}

void
save_mobile (CHAR_DATA * mob, FILE * fp, char *save_reason, int extract)
{
	AFFECTED_TYPE *af;
	int i = 0;
	WOUND_DATA *wound;
	LODGED_OBJECT_INFO *lodged;

	if (!IS_NPC (mob))
		return;

	fprintf (fp, "%d %s\n", mob->mob->nVirtual, save_reason);

	fprintf (fp, "Room             %d\n", mob->in_room);
	fprintf (fp, "SDesc            %s~\n", mob->short_descr);
	fprintf (fp, "LDesc            %s~\n", mob->long_descr);
	fprintf (fp, "Keys             %s~\n", mob->name);
	fprintf (fp, "Fightmode        %d\n", mob->fight_mode);
	fprintf (fp, "Clans            %s~\n", mob->clans);
	fprintf (fp, "ColdloadId       %d\n", mob->coldload_id);
	fprintf (fp, "Flags            %d\n", mob->flags);
	fprintf (fp, "Moves            %d\n", mob->move);
	fprintf (fp, "Hits             %d\n", mob->hit);
	fprintf (fp, "Speaks           %d\n", mob->speaks);
	fprintf (fp, "AffectedBy       %ld\n", mob->affected_by);
	fprintf (fp, "Position         %d\n", mob->position);
	fprintf (fp, "Act              %ld\n", mob->act);
	fprintf (fp, "Cond0            %d\n", mob->intoxication);
	fprintf (fp, "Cond1            %d\n", mob->hunger);
	fprintf (fp, "Cond2            %d\n", mob->thirst);
	fprintf (fp, "Sex              %d\n", mob->sex);
	fprintf (fp, "Height           %d\n", mob->height);
	fprintf (fp, "Frame            %d\n", mob->frame);
	fprintf (fp, "AccessFlags      %d\n", mob->mob->access_flags);
	fprintf (fp, "Morph_time       %d\n", mob->morph_time);
	fprintf (fp, "SpawnLocation    %d\n", mob->mob->spawnpoint);
	fprintf (fp, "ResetCmd         %d\n", mob->mob->reset_cmd);
	fprintf (fp, "ResetZone        %d\n", mob->mob->reset_zone);

	if (IS_RIDEE (mob) && IS_NPC (mob->mount))
		fprintf (fp, "Rider            %d\n", mob->mount->coldload_id);

	if (IS_RIDER (mob) && IS_NPC (mob->mount))
		fprintf (fp, "Mount            %d\n", mob->mount->coldload_id);

	for (i = 1; i <= LAST_SKILL; i++)
	{
		if (mob->skills[i])
			fprintf (fp, "Skill %s %d\n", skills[i], mob->skills[i]);
	}

	if (mob->mob->owner)
		fprintf (fp, "Owner            %s~\n", mob->mob->owner);

	for (af = mob->hour_affects; af; af = af->next)
		if ((af->type < MAGIC_CLAN_MEMBER_BASE ||
			af->type > MAGIC_CLAN_OMNI_BASE + MAX_CLANS) &&
			af->type != MAGIC_CLAN_NOTIFY &&
			af->type != MAGIC_NOTIFY &&
			af->type != MAGIC_WATCH1 &&
			af->type != MAGIC_WATCH2 &&
			af->type != MAGIC_WATCH3 && af->type != MAGIC_GUARD)
			fprintf (fp, "Affect       %d %d %d %d %d %d %ld\n",
			af->type,
			af->a.spell.duration,
			af->a.spell.modifier,
			af->a.spell.location,
			af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);

	for (wound = mob->wounds; wound; wound = wound->next)
		fprintf (fp, "Wound        %s %s %s %s %d %d %d %d %d %d %d\n",
		wound->location, wound->type, wound->severity, wound->name,
		wound->damage, wound->bleeding, wound->poison,
		wound->infection, wound->healerskill, wound->lasthealed,
		wound->lastbled);

	for (lodged = mob->lodged; lodged; lodged = lodged->next)
		fprintf (fp, "Lodged        %s %d\n", lodged->location, lodged->vnum);

	if (mob->equip || mob->right_hand || mob->left_hand)
	{
		fprintf (fp, "HasInventory\n");
		write_obj_suppliment (mob, fp);
	}

	fprintf (fp, "Done\n");

	if (IS_HITCHER (mob))
		save_mobile (mob->hitchee, fp, "HITCH", extract);
	else
		fprintf (fp, "End\n");

	if (extract)
		extract_char (mob);
}

void
save_attached_mobiles (CHAR_DATA * ch, int extract)
{
	FILE *fp;
	char save_name[MAX_STRING_LENGTH];

	sprintf (save_name, "save/player/%c/%s.a", tolower (*ch->tname), ch->tname);

	if (!(fp = fopen (save_name, "w")))
	{
		return;
	}

	if (!IS_HITCHER (ch) && !IS_RIDER (ch))
		fprintf (fp, "end\n");

	if (IS_HITCHER (ch))
		save_mobile (ch->hitchee, fp, "HITCH", extract);

	if (IS_RIDER (ch))
		save_mobile (ch->mount, fp, "RIDE", extract);

	fclose (fp);
}
