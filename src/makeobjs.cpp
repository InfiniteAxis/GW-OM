/***********************************************************************
*                    Gundam Wing: Operation Meteor                     *
*----------------------------------------------------------------------*
* The recent additions and changes to this code (SWR 1.0), not         *
* including the original code itself is here by copyrighted.           *
* Under US federal and government regulations, any use of the          *
* additions or changes not part of the original code are subject       *
* to such fines as seeing fit by the government of the United          *
* United States of America. [Code Changes from SWR 1.0 to GW:OM        *
*----------------------------------------------------------------------*
* Additions to SWR (1.0) for Gundam Operation Meteor are               *
* copyrighted to - Justin Bedder (Cray/Trowa) 2001+ (c)                *
*----------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                       *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops            *
* ---------------------------------------------------------------------*
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael    *
* Chastain, Michael Quan, and Mitchell Tse.                            *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,      *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe. *
***********************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Make a fire.
 */
void make_fire( ROOM_INDEX_DATA *in_room, short timer )
{
	OBJ_DATA *fire;

	fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
	fire->timer = number_fuzzy( timer );
	obj_to_room( fire, in_room );
	return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap( int v0, int v1, int v2, int v3 )
{
	OBJ_DATA *trap;

	trap = create_object( get_obj_index( OBJ_VNUM_TRAP ), 0 );
	trap->timer = 0;
	trap->value[0] = v0;
	trap->value[1] = v1;
	trap->value[2] = v2;
	trap->value[3] = v3;
	return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps( OBJ_DATA *obj )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *scraps, *tmpobj;
	CHAR_DATA *ch = NULL;

	separate_obj( obj );
	scraps = create_object( get_obj_index( OBJ_VNUM_SCRAPS ), 0 );
	scraps->timer = number_range( 5, 15 );

	/*
	 * don't make scraps of scraps of scraps of ...
	 */
	if( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
	{
		STRFREE( scraps->short_descr );
		scraps->short_descr = STRALLOC( "some debris" );
		STRFREE( scraps->description );
		scraps->description = STRALLOC( "Bits of debris lie on the ground here." );
	}
	else
	{
		snprintf( buf, MAX_STRING_LENGTH, scraps->short_descr, obj->short_descr );
		STRFREE( scraps->short_descr );
		scraps->short_descr = STRALLOC( buf );
		snprintf( buf, MAX_STRING_LENGTH, scraps->description, obj->short_descr );
		STRFREE( scraps->description );
		scraps->description = STRALLOC( buf );
	}

	if( obj->carried_by )
	{
		act( AT_OBJECT, "$p falls to the ground in scraps!", obj->carried_by, obj, NULL, TO_CHAR );
		if( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
			&& ( tmpobj = get_eq_char( obj->carried_by, WEAR_DUAL_WIELD ) ) != NULL )
			tmpobj->wear_loc = WEAR_WIELD;

		obj_to_room( scraps, obj->carried_by->in_room );
	}
	else if( obj->in_room )
	{
		if( ( ch = obj->in_room->first_person ) != NULL )
		{
			act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_ROOM );
			act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_CHAR );
		}
		obj_to_room( scraps, obj->in_room );
	}
	if( ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_CORPSE_PC ) && obj->first_content )
	{
		if( ch && ch->in_room )
		{
			act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_ROOM );
			act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_CHAR );
		}
		if( obj->carried_by )
			empty_obj( obj, NULL, obj->carried_by->in_room );
		else if( obj->in_room )
			empty_obj( obj, NULL, obj->in_room );
		else if( obj->in_obj )
			empty_obj( obj, obj->in_obj, NULL );
	}
	extract_obj( obj );
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *voodoo;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	const char *name;

	if( IS_IMMORTAL( ch ) )
		return;

	if( IS_NPC( ch ) )
	{
		name = ch->short_descr;
		if( xIS_SET( ch->act, ACT_DROID ) )
			corpse = create_object( get_obj_index( OBJ_VNUM_DROID_CORPSE ), 0 );
		else
			corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_NPC ), 0 );
		corpse->timer = 6;
		if( ch->gold > 0 )
		{
			if( ch->in_room )
				ch->in_room->area->gold_looted += ch->gold;
			obj_to_obj( create_money( ch->gold ), corpse );
			ch->gold = 0;
		}

		/* Cannot use these!  They are used.
			corpse->value[0] = (int)ch->pIndexData->vnum;
			corpse->value[1] = (int)ch->max_hit;
		*/
		/*	Using corpse cost to cheat, since corpses not sellable */
		corpse->cost = ( -( int ) ch->pIndexData->vnum );
		corpse->value[2] = corpse->timer;
	}
	else
	{
		name = ch->name;
		corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
		corpse->timer = 40;
		corpse->value[2] = ( int ) ( corpse->timer / 8 );
		corpse->value[3] = 0;
		corpse->owner = str_dup( ch->name );
		corpse->killer = str_dup( killer->name );
		if( ch->gold > 0 )
		{
			if( ch->in_room )
				ch->in_room->area->gold_looted += ch->gold;
			obj_to_obj( create_money( ch->gold ), corpse );
			ch->gold = 0;
		}
	}

	if( !IS_NPC( ch ) )
	{
		voodoo = create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
		voodoo->value[5] = 1;
		snprintf( buf1, MAX_STRING_LENGTH, "%s piece", capitalize( name ) );
		STRFREE( voodoo->name );
		voodoo->name = STRALLOC( buf1 );

		snprintf( buf1, MAX_STRING_LENGTH, "A piece of %s", capitalize( name ) );
		STRFREE( voodoo->short_descr );
		voodoo->short_descr = STRALLOC( buf1 );

		snprintf( buf1, MAX_STRING_LENGTH, "A piece of %s is lying here.", capitalize( name ) );
		STRFREE( voodoo->description );
		voodoo->description = STRALLOC( buf1 );

		obj_to_room( voodoo, ch->in_room );
	}

	if( xIS_SET( ch->act, PLR_PKER ) )
		corpse->value[5] = 1;

	/*
	 * Added corpse name - make locate easier , other skills
	 */
	snprintf( buf, MAX_STRING_LENGTH, "corpse %s", name );
	STRFREE( corpse->name );
	corpse->name = STRALLOC( buf );

	snprintf( buf, MAX_STRING_LENGTH, corpse->short_descr, name );
	STRFREE( corpse->short_descr );
	corpse->short_descr = STRALLOC( buf );

	snprintf( buf, MAX_STRING_LENGTH, corpse->description, name );
	STRFREE( corpse->description );
	corpse->description = STRALLOC( buf );

	for( obj = ch->first_carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;
		obj_from_char( obj );

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			obj_to_room( obj, ch->in_room );
			save_artifacts( );
			snprintf( buf, MAX_STRING_LENGTH, "%s dropped to the floor.", obj->name );
			append_file( ch, ARTI_FILE, buf );
		}

		if( !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			if( IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
				extract_obj( obj );
			else
				obj_to_obj( obj, corpse );
		}
	}

	obj_to_room( corpse, ch->in_room );
	return;
}



void make_blood( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj = create_object( get_obj_index( OBJ_VNUM_BLOOD ), 0 );
	obj->timer = number_range( 2, 4 );
	obj->value[1] = number_range( 3, UMIN( 5, ch->top_level ) );
	obj_to_room( obj, ch->in_room );
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj = create_object( get_obj_index( OBJ_VNUM_BLOODSTAIN ), 0 );
	obj->timer = number_range( 1, 2 );
	obj_to_room( obj, ch->in_room );
}


/*
 * make some coinage
 */
OBJ_DATA *create_money( int amount )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	if( amount <= 0 )
	{
		bug( "Create_money: zero or negative money %d.", amount );
		amount = 1;
	}

	if( amount == 1 )
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
	}
	else
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
		snprintf( buf, MAX_STRING_LENGTH, obj->short_descr, amount );
		STRFREE( obj->short_descr );
		obj->short_descr = STRALLOC( buf );
		obj->value[0] = amount;
	}

	return obj;
}
