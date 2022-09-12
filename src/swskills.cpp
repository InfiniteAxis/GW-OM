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


#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void add_reinforcements( CHAR_DATA *ch );
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
int xp_compute( CHAR_DATA *ch, CHAR_DATA *victim );
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );
int ris_save( CHAR_DATA *ch, int chance, int ris );
CHAR_DATA *get_char_room_mp( CHAR_DATA *ch, const char *argument );

extern int top_affect;

CMDF( do_makeblade )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;// , charge;
	bool checktool, checkdura, checkbatt, checkoven;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf2;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:

		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makeblade <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkdura = false;
		checkbatt = false;
		checkoven = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_DURASTEEL )
				checkdura = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;

			if( obj->item_type == ITEM_OVEN )
				checkoven = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a vibro-blade.\r\n", ch );
			return;
		}

		if( !checkdura )
		{
			send_to_char( "&RYou need something to make it out of.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a power source for your blade.\r\n", ch );
			return;
		}

		if( !checkoven )
		{
			send_to_char( "&RYou need a small furnace to heat the metal.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblade] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of crafting a vibroblade.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
				NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 25, do_makeblade, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makeblade );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblade] );
	vnum = 10422;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n", ch );
		return;
	}

	checktool = false;
	checkdura = false;
	checkbatt = false;
	checkoven = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_OVEN )
			checkoven = true;
		if( obj->item_type == ITEM_DURASTEEL && checkdura == false )
		{
			checkdura = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			//charge = UMAX( 5, obj->value[0] );
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblade] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) )
	{
		send_to_char( "&RYou activate your newly created vibroblade.\r\n", ch );
		send_to_char( "&RIt hums softly for a few seconds then begins to shake violently.\r\n", ch );
		send_to_char( "&RIt finally shatters breaking apart into a dozen pieces.\r\n", ch );
		learn_from_failure( ch, gsn_makeblade );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_WEAPON;
	SET_BIT( obj->wear_flags, ITEM_WIELD );
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = 3;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was left here." );
	obj->description = STRALLOC( buf );
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type = -1;
	paf->duration = -1;
	paf->location = APPLY_BACKSTAB;
	paf->modifier = level / 3;
	paf->bitvector = 0;
	paf->next = NULL;
	LINK( paf, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	CREATE( paf2, AFFECT_DATA, 1 );
	paf2->type = -1;
	paf2->duration = -1;
	paf2->location = APPLY_HITROLL;
	paf2->modifier = -2;
	paf2->bitvector = 0;
	paf2->next = NULL;
	LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	obj->value[0] = INIT_WEAPON_CONDITION;
	obj->value[1] = ( int ) ( level / 20 + 10 );  /* min dmg  */
	obj->value[2] = ( int ) ( level / 10 + 20 );  /* max dmg */
	obj->value[3] = WEAPON_VIBRO_BLADE;
	obj->value[4] = 0;
	obj->value[5] = 0;
	obj->cost = obj->value[2] * 10;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created blade.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes crafting a vibro-blade.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 200,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}

	learn_from_success( ch, gsn_makeblade );
}

CMDF( do_makeblaster )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;
	bool checktool, checkdura, checkbatt, checkoven, checkcond, checkcirc, checkammo;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum, power, scope, ammo;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf2;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makeblaster <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkdura = false;
		checkbatt = false;
		checkoven = false;
		checkcond = false;
		checkcirc = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_DURASTEEL )
				checkdura = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_OVEN )
				checkoven = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
			if( obj->item_type == ITEM_SUPERCONDUCTOR )
				checkcond = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a blaster.\r\n", ch );
			return;
		}

		if( !checkdura )
		{
			send_to_char( "&RYou need something to make it out of.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a power source for your blaster.\r\n", ch );
			return;
		}

		if( !checkoven )
		{
			send_to_char( "&RYou need a small furnace to heat the plastics.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit board to control the firing mechanism.\r\n", ch );
			return;
		}

		if( !checkcond )
		{
			send_to_char( "&RYou still need a small superconductor.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblaster] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of making a blaster.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
				NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 25, do_makeblaster, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makeblaster );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblaster] );
	vnum = 10420;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checkammo = false;
	checktool = false;
	checkdura = false;
	checkbatt = false;
	checkoven = false;
	checkcond = false;
	checkcirc = false;
	power = 0;
	scope = 0;
	ammo = 0;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_OVEN )
			checkoven = true;
		if( obj->item_type == ITEM_DURAPLAST && checkdura == false )
		{
			checkdura = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_AMMO && checkammo == false )
		{
			ammo = obj->value[0];
			checkammo = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
		if( obj->item_type == ITEM_LENS && scope == 0 )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			scope++;
		}
		if( obj->item_type == ITEM_SUPERCONDUCTOR && power < 2 )
		{
			power++;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcond = true;
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcirc = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeblaster] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )
		|| ( !checkcond ) || ( !checkcirc ) )
	{
		send_to_char( "&RYou hold up your new blaster and aim at a leftover piece of plastic.\r\n", ch );
		send_to_char( "&RYou slowly squeeze the trigger hoping for the best...\r\n", ch );
		send_to_char( "&RYour blaster backfires destroying your weapon and burning your hand.\r\n", ch );
		learn_from_failure( ch, gsn_makeblaster );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_WEAPON;
	SET_BIT( obj->wear_flags, ITEM_WIELD );
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = 2 + level / 10;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was carelessly misplaced here." );
	obj->description = STRALLOC( buf );
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type = -1;
	paf->duration = -1;
	paf->location = APPLY_HITROLL;
	paf->modifier = URANGE( 0, 1 + scope, level / 30 );
	paf->bitvector = 0;
	paf->next = NULL;
	LINK( paf, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	CREATE( paf2, AFFECT_DATA, 1 );
	paf2->type = -1;
	paf2->duration = -1;
	paf2->location = APPLY_DAMROLL;
	paf2->modifier = URANGE( 0, power, level / 30 );
	paf2->bitvector = 0;
	paf2->next = NULL;
	LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	obj->value[0] = INIT_WEAPON_CONDITION;   /* condition  */
	obj->value[1] = ( int ) ( level / 10 + 15 );  /* min dmg  */
	obj->value[2] = ( int ) ( level / 5 + 25 );   /* max dmg  */
	obj->value[3] = WEAPON_BLASTER;
	obj->value[4] = ammo;
	obj->value[5] = 2000;
	obj->cost = obj->value[2] * 50;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created blaster.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making $s new blaster.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 50,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makeblaster );
}

CMDF( do_makelightsaber )
{
}


CMDF( do_makespice )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance;
	OBJ_DATA *obj;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RFrom what?\r\n&w", ch );
			return;
		}

		if( !xIS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
		{
			send_to_char( "&RYou need to be in a refinery to create drugs from spice.\r\n", ch );
			return;
		}

		if( ms_find_obj( ch ) )
			return;

		if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			send_to_char( "&RYou do not have that item.\r\n&w", ch );
			return;
		}

		if( obj->item_type != ITEM_RAWSPICE )
		{
			send_to_char( "&RYou can't make a drug out of that\r\n&w", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_spice_refining] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of refining spice into a drug.\r\n", ch );
			act( AT_PLAIN, "$n begins working on something.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_makespice, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out what to do with the stuff.\r\n", ch );
		learn_from_failure( ch, gsn_spice_refining );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are distracted and are unable to finish your work.\r\n&w", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You seem to have lost your spice!\r\n", ch );
		return;
	}
	if( obj->item_type != ITEM_RAWSPICE )
	{
		send_to_char( "&RYou get your tools mixed up and can't finish your work.\r\n&w", ch );
		return;
	}

	obj->value[1] = URANGE( 10, obj->value[1], ( IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->pcdata->learned[gsn_spice_refining] ) ) + 10 );
	strcpy( buf, obj->name );
	STRFREE( obj->name );
	strcat( buf, " drug spice" );
	strcat( buf, ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, "a drug made from " );
	strcat( buf, obj->short_descr );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	strcat( buf, " was foolishly left lying around here." );
	STRFREE( obj->description );
	obj->description = STRALLOC( buf );
	obj->item_type = ITEM_SPICE;

	send_to_char( "&GYou finish your work.\r\n", ch );
	act( AT_PLAIN, "$n finishes $s work.", ch, NULL, argument, TO_ROOM );

	obj->cost += obj->value[1] * 10;
	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 50,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}

	learn_from_success( ch, gsn_spice_refining );

}

CMDF( do_makegrenade )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance, strength, weight;
	bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makegrenade <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkdrink = false;
		checkbatt = false;
		checkchem = false;
		checkcirc = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
				checkdrink = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
			if( obj->item_type == ITEM_CHEMICAL )
				checkchem = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a grenade.\r\n", ch );
			return;
		}

		if( !checkdrink )
		{
			send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a small battery for the timer.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit for the timer.\r\n", ch );
			return;
		}

		if( !checkchem )
		{
			send_to_char( "&RSome explosive chemicals would come in handy!\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makegrenade] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of making a grenade.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
				NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 25, do_makegrenade, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makegrenade );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makegrenade] );
	vnum = 10425;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checkdrink = false;
	checkbatt = false;
	checkchem = false;
	checkcirc = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_DRINK_CON && checkdrink == false && obj->value[1] == 0 )
		{
			checkdrink = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
		if( obj->item_type == ITEM_CHEMICAL )
		{
			strength = URANGE( 10, obj->value[0], level * 5 );
			weight = obj->weight;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkchem = true;
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcirc = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makegrenade] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem )
		|| ( !checkcirc ) )
	{
		send_to_char
		( "&RJust as you are about to finish your work,\n\ryour newly created grenade explodes in your hands...doh!\r\n",
			ch );
		learn_from_failure( ch, gsn_makegrenade );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_GRENADE;
	SET_BIT( obj->wear_flags, ITEM_HOLD );
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = weight;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was carelessly misplaced here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = strength / 2;
	obj->value[1] = strength;
	obj->cost = obj->value[1] * 5;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created grenade.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making $s new grenade.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 50,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makegrenade );
}

CMDF( do_makelandmine )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance, strength, weight;
	bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makelandmine <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkdrink = false;
		checkbatt = false;
		checkchem = false;
		checkcirc = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
				checkdrink = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
			if( obj->item_type == ITEM_CHEMICAL )
				checkchem = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a landmine.\r\n", ch );
			return;
		}

		if( !checkdrink )
		{
			send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a small battery for the detonator.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit for the detonator.\r\n", ch );
			return;
		}

		if( !checkchem )
		{
			send_to_char( "&RSome explosive chemicals would come in handy!\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelandmine] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of making a landmine.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
				NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 25, do_makelandmine, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makelandmine );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelandmine] );
	vnum = 10427;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checkdrink = false;
	checkbatt = false;
	checkchem = false;
	checkcirc = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_DRINK_CON && checkdrink == false && obj->value[1] == 0 )
		{
			checkdrink = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
		if( obj->item_type == ITEM_CHEMICAL )
		{
			strength = URANGE( 10, obj->value[0], level * 5 );
			weight = obj->weight;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkchem = true;
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcirc = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelandmine] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem )
		|| ( !checkcirc ) )
	{
		send_to_char
		( "&RJust as you are about to finish your work,\n\ryour newly created landmine explodes in your hands...doh!\r\n",
			ch );
		learn_from_failure( ch, gsn_makelandmine );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_LANDMINE;
	SET_BIT( obj->wear_flags, ITEM_HOLD );
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = weight;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was carelessly misplaced here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = strength / 2;
	obj->value[1] = strength;
	obj->cost = obj->value[1] * 5;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created landmine.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making $s new landmine.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 50,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makelandmine );
}
CMDF( do_makelight )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance, strength;
	bool checktool, checkbatt, checkchem, checkcirc, checklens;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makeflashlight <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkbatt = false;
		checkchem = false;
		checkcirc = false;
		checklens = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
			if( obj->item_type == ITEM_CHEMICAL )
				checkchem = true;
			if( obj->item_type == ITEM_LENS )
				checklens = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a light.\r\n", ch );
			return;
		}

		if( !checklens )
		{
			send_to_char( "&RYou need a lens to make a light.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a battery for the light to work.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit.\r\n", ch );
			return;
		}

		if( !checkchem )
		{
			send_to_char( "&RSome chemicals to light would come in handy!\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelight] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of making a light.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_makelight, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makelight );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelight] );
	vnum = 10428;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checklens = false;
	checkbatt = false;
	checkchem = false;
	checkcirc = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			strength = obj->value[0];
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
		if( obj->item_type == ITEM_CHEMICAL )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkchem = true;
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcirc = true;
		}
		if( obj->item_type == ITEM_LENS && checklens == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checklens = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makelight] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checklens ) || ( !checkbatt ) || ( !checkchem )
		|| ( !checkcirc ) )
	{
		send_to_char
		( "&RJust as you are about to finish your work,\n\ryour newly created light explodes in your hands...doh!\r\n",
			ch );
		learn_from_failure( ch, gsn_makelight );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_LIGHT;
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = 3;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was carelessly misplaced here." );
	obj->description = STRALLOC( buf );
	obj->value[2] = strength;
	obj->cost = obj->value[2];

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created light.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making $s new light.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 100,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makelight );
}

CMDF( do_makejewelry )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;
	bool checktool, checkoven, checkmetal;
	OBJ_DATA *obj;
	OBJ_DATA *metal;
	int value, cost;

	argument = one_argument( argument, arg );
	mudstrlcpy( arg2, argument, MAX_INPUT_LENGTH );

	if( !str_cmp( arg, "body" )
		|| !str_cmp( arg, "head" )
		|| !str_cmp( arg, "legs" )
		|| !str_cmp( arg, "arms" )
		|| !str_cmp( arg, "about" )
		|| !str_cmp( arg, "eyes" )
		|| !str_cmp( arg, "waist" ) || !str_cmp( arg, "hold" ) || !str_cmp( arg, "feet" ) || !str_cmp( arg, "hands" ) )
	{
		send_to_char( "&RYou cannot make jewelry for that body part.\r\n&w", ch );
		send_to_char( "&RTry MAKEARMOR.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "shield" ) )
	{
		send_to_char( "&RYou cannot make jewelry worn as a shield.\r\n&w", ch );
		send_to_char( "&RTry MAKESHIELD.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "relic" ) )
	{
		send_to_char( "Try a different wearloc!\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "wield" ) )
	{
		send_to_char( "&RAre you going to fight with your jewelry?\r\n&w", ch );
		send_to_char( "&RTry MAKEBLADE...\r\n&w", ch );
		return;
	}

	switch( ch->substate )
	{
	default:

		if( arg2[0] == '\0' )
		{
			send_to_char( "&RUsage: Makejewelry <wearloc> <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkoven = false;
		checkmetal = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_OVEN )
				checkoven = true;
			if( obj->item_type == ITEM_RARE_METAL )
				checkmetal = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need a toolkit.\r\n", ch );
			return;
		}

		if( !checkoven )
		{
			send_to_char( "&RYou need an oven.\r\n", ch );
			return;
		}

		if( !checkmetal )
		{
			send_to_char( "&RYou need some precious metal.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makejewelry] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of creating some jewelry.\r\n", ch );
			act( AT_PLAIN, "$n takes $s toolkit and some metal and begins to work.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 15, do_makejewelry, 1 );
			ch->dest_buf = str_dup( arg );
			ch->dest_buf_2 = str_dup( arg2 );
			return;
		}
		send_to_char( "&RYou can't figure out what to do.\r\n", ch );
		learn_from_failure( ch, gsn_makejewelry );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		if( !ch->dest_buf_2 )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		mudstrlcpy( arg2, ( const char * ) ch->dest_buf_2, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf_2 );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		DISPOSE( ch->dest_buf_2 );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makejewelry] );

	checkmetal = false;
	checkoven = false;
	checktool = false;
	value = 0;
	cost = 0;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_OVEN )
			checkoven = true;
		if( obj->item_type == ITEM_RARE_METAL && checkmetal == false )
		{
			checkmetal = true;
			separate_obj( obj );
			obj_from_char( obj );
			metal = obj;
		}
		if( obj->item_type == ITEM_CRYSTAL )
		{
			cost += obj->cost;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makejewelry] );

	if( number_percent( ) > chance * 2 || ( !checkoven ) || ( !checktool ) || ( !checkmetal ) )
	{
		send_to_char( "&RYou hold up your newly created jewelry.\r\n", ch );
		send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\r\n", ch );
		send_to_char( "&Rpiece of junk you've ever seen. You quickly hide your mistake...\r\n", ch );
		learn_from_failure( ch, gsn_makejewelry );
		return;
	}

	obj = metal;

	obj->item_type = ITEM_ARMOR;
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	value = get_wflag( arg );
	if( value < 0 || value > 31 )
		SET_BIT( obj->wear_flags, ITEM_WEAR_NECK );
	else
		SET_BIT( obj->wear_flags, 1 << value );
	obj->level = level;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg2 ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg2 );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was dropped here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = obj->value[1];
	obj->cost *= 10;
	obj->cost += cost;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created jewelry.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes sewing some new jewelry.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 100,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makejewelry );

}

CMDF( do_makearmor )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;
	bool checksew, checkfab;
	OBJ_DATA *obj;
	OBJ_DATA *material;
	int value;

	argument = one_argument( argument, arg );
	mudstrlcpy( arg2, argument, MAX_INPUT_LENGTH );

	if( !str_cmp( arg, "eyes" )
		|| !str_cmp( arg, "ears" ) || !str_cmp( arg, "finger" ) || !str_cmp( arg, "neck" ) || !str_cmp( arg, "wrist" ) )
	{
		send_to_char( "&RYou cannot make clothing for that body part.\r\n&w", ch );
		send_to_char( "&RTry MAKEJEWELRY.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "shield" ) )
	{
		send_to_char( "&RYou cannot make clothing worn as a shield.\r\n&w", ch );
		send_to_char( "&RTry MAKESHIELD.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "relic" ) )
	{
		send_to_char( "Try a different wearloc!\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "wield" ) )
	{
		send_to_char( "&RAre you going to fight with your clothing?\r\n&w", ch );
		send_to_char( "&RTry MAKEBLADE...\r\n&w", ch );
		return;
	}

	switch( ch->substate )
	{
	default:

		if( arg2[0] == '\0' )
		{
			send_to_char( "&RUsage: Makearmor <wearloc> <name>\r\n&w", ch );
			return;
		}

		checksew = false;
		checkfab = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_FABRIC )
				checkfab = true;
			if( obj->item_type == ITEM_THREAD )
				checksew = true;
		}

		if( !checkfab )
		{
			send_to_char( "&RYou need some sort of fabric or material.\r\n", ch );
			return;
		}

		if( !checksew )
		{
			send_to_char( "&RYou need a needle and some thread.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makearmor] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of creating some armor.\r\n", ch );
			act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 15, do_makearmor, 1 );
			ch->dest_buf = str_dup( arg );
			ch->dest_buf_2 = str_dup( arg2 );
			return;
		}
		send_to_char( "&RYou can't figure out what to do.\r\n", ch );
		learn_from_failure( ch, gsn_makearmor );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		if( !ch->dest_buf_2 )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		mudstrlcpy( arg2, ( const char * ) ch->dest_buf_2, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf_2 );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		DISPOSE( ch->dest_buf_2 );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makearmor] );

	checksew = false;
	checkfab = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_THREAD )
			checksew = true;
		if( obj->item_type == ITEM_FABRIC && checkfab == false )
		{
			checkfab = true;
			separate_obj( obj );
			obj_from_char( obj );
			material = obj;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makearmor] );

	if( number_percent( ) > chance * 2 || ( !checkfab ) || ( !checksew ) )
	{
		send_to_char( "&RYou hold up your newly created armor.\r\n", ch );
		send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\r\n", ch );
		send_to_char( "&Rgarment you've ever seen. You quickly hide your mistake...\r\n", ch );
		learn_from_failure( ch, gsn_makearmor );
		return;
	}

	obj = material;

	obj->item_type = ITEM_ARMOR;
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	value = get_wflag( arg );
	if( value < 0 || value > 31 )
		SET_BIT( obj->wear_flags, ITEM_WEAR_BODY );
	else
		SET_BIT( obj->wear_flags, 1 << value );
	obj->level = level;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg2 ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg2 );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was dropped here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = obj->value[1];
	obj->cost *= 10;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created garment.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes sewing some new armor.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 100,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makearmor );
}


CMDF( do_makecomlink )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance;
	bool checktool, checkgem, checkbatt, checkcirc;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:

		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makecomlink <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkgem = false;
		checkbatt = false;
		checkcirc = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_CRYSTAL )
				checkgem = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a comlink.\r\n", ch );
			return;
		}

		if( !checkgem )
		{
			send_to_char( "&RYou need a small crystal.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a power source for your comlink.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecomlink] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of making a comlink.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_makecomlink, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makecomlink );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	vnum = 10430;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checkgem = false;
	checkbatt = false;
	checkcirc = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_CRYSTAL && checkgem == false )
		{
			checkgem = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			checkcirc = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecomlink] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkcirc ) || ( !checkbatt ) || ( !checkgem ) )
	{
		send_to_char( "&RYou hold up your newly created comlink....\r\n", ch );
		send_to_char( "&Rand it falls apart in your hands.\r\n", ch );
		learn_from_failure( ch, gsn_makecomlink );
		return;
	}

	obj = create_object( pObjIndex, ch->top_level );

	obj->item_type = ITEM_COMLINK;
	SET_BIT( obj->wear_flags, ITEM_HOLD );
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->weight = 3;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was left here." );
	obj->description = STRALLOC( buf );
	obj->cost = 50;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created comlink.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes crafting a comlink.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 100,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makecomlink );

}

CMDF( do_makeshield )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance;
	bool checktool, checkbatt, checkcond, checkcirc, checkgems;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum, level, charge, gemtype;

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makeshield <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkbatt = false;
		checkcond = false;
		checkcirc = false;
		checkgems = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a workshop.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_CRYSTAL )
				checkgems = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;
			if( obj->item_type == ITEM_CIRCUIT )
				checkcirc = true;
			if( obj->item_type == ITEM_SUPERCONDUCTOR )
				checkcond = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make an energy shield.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a power source for your energy shield.\r\n", ch );
			return;
		}

		if( !checkcirc )
		{
			send_to_char( "&RYou need a small circuit board.\r\n", ch );
			return;
		}

		if( !checkcond )
		{
			send_to_char( "&RYou still need a small superconductor for your energy shield.\r\n", ch );
			return;
		}

		if( !checkgems )
		{
			send_to_char( "&RYou need a small crystal.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeshield] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of crafting an energy shield.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 20, do_makeshield, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makeshield );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeshield] );
	vnum = 10429;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checkbatt = false;
	checkcond = false;
	checkcirc = false;
	checkgems = false;
	charge = 0;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;

		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			charge = UMIN( obj->value[1], 10 );
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
		if( obj->item_type == ITEM_SUPERCONDUCTOR && checkcond == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcond = true;
		}
		if( obj->item_type == ITEM_CIRCUIT && checkcirc == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkcirc = true;
		}
		if( obj->item_type == ITEM_CRYSTAL && checkgems == false )
		{
			gemtype = obj->value[0];
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkgems = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makeshield] );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkbatt )
		|| ( !checkgems ) || ( !checkcond ) || ( !checkcirc ) )

	{
		send_to_char( "&RYou hold up your new energy shield and press the switch hoping for the best.\r\n", ch );
		send_to_char( "&RInstead of a field of energy being created, smoke starts pouring from the device.\r\n", ch );
		send_to_char( "&RYou drop the hot device and watch as it melts on away on the floor.\r\n", ch );
		learn_from_failure( ch, gsn_makeshield );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_ARMOR;
	SET_BIT( obj->wear_flags, ITEM_WIELD );
	SET_BIT( obj->wear_flags, ITEM_WEAR_SHIELD );
	obj->level = level;
	obj->weight = 2;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was carelessly misplaced here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = ( int ) ( level / 10 + gemtype * 2 ); /* condition */
	obj->value[1] = ( int ) ( level / 10 + gemtype * 2 ); /* armor */
	obj->value[4] = charge;
	obj->value[5] = charge;
	obj->cost = obj->value[2] * 100;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created energy shield.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making $s new energy shield.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 50,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makeshield );

}

#define IS_WEAR_LOC( obj, value )   IS_SET( obj->wear_flags, value )

CMDF( do_sharpen )
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	bool checkwhet, infact;
	OBJ_DATA *obj;
	OBJ_DATA *whetstone;

	argument = one_argument( argument, arg );

	if( get_timerptr( ch, TIMER_DO_FUN ) == NULL )
	{
		if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			send_to_char( "Sharpen what?\r\n", ch );
			return;
		}

		if( IS_WEAR_LOC( obj, ITEM_WEAR_EARS ) || IS_WEAR_LOC( obj, ITEM_WEAR_FINGER )
			|| IS_WEAR_LOC( obj, ITEM_WEAR_NECK ) || IS_WEAR_LOC( obj, ITEM_WEAR_WRIST )
			|| IS_WEAR_LOC( obj, ITEM_WEAR_ANKLE ) || IS_WEAR_LOC( obj, ITEM_WEAR_RELIC ) )
		{
			send_to_char( "&RJewelry is much too delicate to work with like that!\r\n", ch );
			return;
		}

		else if( obj->item_type == ITEM_LIGHT )
		{
			send_to_char( "&RI think sharpening a light is likely to only make it more dull.\r\n", ch );
			return;
		}

		else if( IS_WEAR_LOC( obj, ITEM_HOLD ) )
		{
			send_to_char( "&RSomething tells you that probably won't accomplish much.\r\n", ch );
			return;
		}

		else if( !IS_WEAR_LOC( obj, ITEM_WIELD ) && !IS_WEAR_LOC( obj, ITEM_DUAL_WIELD ) )
		{
			send_to_char( "&RSharpening armor? Are you hoping someone will bump into you and bleed to death?\r\n", ch );
			send_to_char( "&RTry REINFORCE.\r\n", ch );
			return;
		}

		if( obj->value[3] != WEAPON_VIBRO_AXE && obj->value[3] != WEAPON_VIBRO_BLADE
			&& obj->value[3] != WEAPON_LIGHTSABER && obj->value[3] != WEAPON_CLAW && obj->value[3] != WEAPON_FORCE_PIKE )
		{
			send_to_char( "&RYou can't sharpen that which is neither edged nor pointed.\r\n", ch );
			return;
		}

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			send_to_char( "&RMeddling with an artifact would just be wrong.\r\n", ch );
			return;
		}

		if( xIS_SET( obj->extra_flags, ITEM_AUGMENTED ) )
		{
			send_to_char( "&RIt's already as sharp as it will get!\r\n", ch );
			return;
		}
	}

	switch( ch->substate )
	{
	default:
		checkwhet = false;

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_WHETSTONE )
				checkwhet = true;
		}

		if( !checkwhet )
		{
			send_to_char( "&RYou'll need something to sharpen it with.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_sharpen] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin to sharpen your weapon carefully.\r\n", ch );
			act( AT_PLAIN, "$n begins to sharpen $s weapon carefully.", ch, NULL, NULL, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 15, do_sharpen, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou nearly cut yourself trying to settle your weapon on the stone.\r\n", ch );
		learn_from_failure( ch, gsn_sharpen );
		return;

	case 1:
		if( !ch->dest_buf )
		{
			send_to_char( "&RSomething went wrong. Report to Axis.\r\n", ch );
			return;
		}
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou're interrupted before you can finish sharpening properly.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "&RYour weapon seems to have disappeared...\r\n", ch );
		return;
	}

	checkwhet = false;

	for( whetstone = ch->last_carrying; whetstone; whetstone = whetstone->prev_content )
	{
		if( whetstone->item_type == ITEM_WHETSTONE )
		{
			checkwhet = true;
			separate_obj( whetstone );
			obj_from_char( whetstone );
			break;
		}
	}

	if( !checkwhet )
	{
		send_to_char( "&RYour whetstone has apparently vanished...\r\n", ch );
		return;
	}

	infact = false;

	if( xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		infact = true;

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_sharpen] );
	if( number_percent( ) > chance * 2 || IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
	{
		send_to_char( "&RAs you attempt to sharpen your weapon, your hands slips,\r\n", ch );
		send_to_char( "&Rand you only succeed in damaging it!\r\n", ch );
		separate_obj( obj );
		damage_obj( obj );
		if( --whetstone->value[0] <= 0 )
		{
			send_to_char( "&YUpon inspecting your whetstone, you find a little too much\r\n", ch );
			send_to_char( "&Ymaterial in the grit, and promptly discard it.\r\n", ch );
			extract_obj( whetstone );
		}
		else
			whetstone = obj_to_char( whetstone, ch );
		learn_from_failure( ch, gsn_sharpen );
		return;
	}

	separate_obj( obj );
	obj_from_char( obj );
	xSET_BIT( obj->extra_flags, ITEM_AUGMENTED );

	obj->value[1] = ( int ) ( obj->value[1] * ( 100 + URANGE( 1, ( IS_NPC( ch ) ? 5
		: ( int ) ( ( ( ch->pcdata->learned[gsn_sharpen] * ( infact ? 2 : 1 ) ) * 100 ) / 1000 ) ), 20 ) ) / 100 );
	obj->value[2] = ( int ) ( obj->value[2] * ( 100 + URANGE( 1, ( IS_NPC( ch ) ? 5
		: ( int ) ( ( ( ch->pcdata->learned[gsn_sharpen] * ( infact ? 2 : 1 ) ) * 100 ) / 1000 ) ), 20 ) ) / 100 );
	obj->cost = 0;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your strenuous work, pull a hair from your head,\r\n", ch );
	send_to_char( "&Gand watch it split in two overtop your fearsome weapon.\r\n", ch );
	act( AT_PLAIN, "$n finishes sharpening $s weapon.", ch, NULL, NULL, TO_ROOM );
	if( --whetstone->value[0] <= 0 )
	{
		send_to_char( "&YUpon inspecting your whetstone, you find a little too much\r\n", ch );
		send_to_char( "&Ymaterial in the grit, and promptly discard it.\r\n", ch );
		extract_obj( whetstone );
	}
	else
		whetstone = obj_to_char( whetstone, ch );
	learn_from_success( ch, gsn_sharpen );
	return;
}

#undef IS_WEAR_LOC

CMDF( do_makecontainer )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;
	bool checksew, checkfab;
	OBJ_DATA *obj;
	OBJ_DATA *material;
	int value;

	argument = one_argument( argument, arg );
	mudstrlcpy( arg2, argument, MAX_INPUT_LENGTH );

	if( !str_cmp( arg, "eyes" )
		|| !str_cmp( arg, "ears" )
		|| !str_cmp( arg, "finger" ) || !str_cmp( arg, "neck" ) || !str_cmp( arg, "relic" ) || !str_cmp( arg, "wrist" ) )
	{
		send_to_char( "&RYou cannot make a container for that body part.\r\n&w", ch );
		send_to_char( "&RTry MAKEJEWELRY.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "feet" ) || !str_cmp( arg, "hands" ) )
	{
		send_to_char( "&RYou cannot make a container for that body part.\r\n&w", ch );
		send_to_char( "&RTry MAKEARMOR.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "relic" ) )
	{
		send_to_char( "Try a different wearloc!\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "shield" ) )
	{
		send_to_char( "&RYou cannot make a container a shield.\r\n&w", ch );
		send_to_char( "&RTry MAKESHIELD.\r\n&w", ch );
		return;
	}
	if( !str_cmp( arg, "wield" ) )
	{
		send_to_char( "&RAre you going to fight with a container?\r\n&w", ch );
		send_to_char( "&RTry MAKEBLADE...\r\n&w", ch );
		return;
	}

	switch( ch->substate )
	{
	default:

		if( arg2[0] == '\0' )
		{
			send_to_char( "&RUsage: Makecontainer <wearloc> <name>\r\n&w", ch );
			return;
		}

		checksew = false;
		checkfab = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_FABRIC )
				checkfab = true;
			if( obj->item_type == ITEM_THREAD )
				checksew = true;
		}

		if( !checkfab )
		{
			send_to_char( "&RYou need some sort of fabric or material.\r\n", ch );
			return;
		}

		if( !checksew )
		{
			send_to_char( "&RYou need a needle and some thread.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecontainer] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of creating a bag.\r\n", ch );
			act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_makecontainer, 1 );
			ch->dest_buf = str_dup( arg );
			ch->dest_buf_2 = str_dup( arg2 );
			return;
		}
		send_to_char( "&RYou can't figure out what to do.\r\n", ch );
		learn_from_failure( ch, gsn_makecontainer );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		if( !ch->dest_buf_2 )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		mudstrlcpy( arg2, ( const char * ) ch->dest_buf_2, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf_2 );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		DISPOSE( ch->dest_buf_2 );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecontainer] );

	checksew = false;
	checkfab = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_THREAD )
			checksew = true;
		if( obj->item_type == ITEM_FABRIC && checkfab == false )
		{
			checkfab = true;
			separate_obj( obj );
			obj_from_char( obj );
			material = obj;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecontainer] );

	if( number_percent( ) > chance * 2 || ( !checkfab ) || ( !checksew ) )
	{
		send_to_char( "&RYou hold up your newly created container.\r\n", ch );
		send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\r\n", ch );
		send_to_char( "&Rcontainer you've ever seen. You quickly hide your mistake...\r\n", ch );
		learn_from_failure( ch, gsn_makecontainer );
		return;
	}

	obj = material;

	obj->item_type = ITEM_CONTAINER;
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	value = get_wflag( arg );
	if( value < 0 || value > 31 )
		SET_BIT( obj->wear_flags, ITEM_HOLD );
	else
		SET_BIT( obj->wear_flags, 1 << value );
	obj->level = level;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg2 ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg2 );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was dropped here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = level;
	obj->value[1] = 0;
	obj->value[2] = 0;
	obj->value[3] = 10;
	obj->cost *= 10;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created container.&w\r\n", ch );
	act( AT_PLAIN, "$n finishes sewing a new container.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( obj->cost * 500,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makecontainer );
}

CMDF( do_makemissile )
{
	/*
	 * don't think we really need this
	 */
	send_to_char( "&RSorry, this skill isn't finished yet :(\r\n", ch );
}

CMDF( do_gemcutting )
{
	send_to_char( "&RSorry, this skill isn't finished yet :(\r\n", ch );
}

CMDF( do_reinforcements )
{
	char arg[MAX_INPUT_LENGTH];
	int chance, credits;

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "&RNot while in the Arena!\r\n", ch );
		return;
	}

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( ch->backup_wait )
		{
			send_to_char( "&RYour reinforcements are already on the way.\r\n", ch );
			return;
		}

		if( !ch->pcdata->clan )
		{
			send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\r\n", ch );
			return;
		}

		if( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 10 )
		{
			ch_printf( ch, "&RYou dont have enough money to send for reinforcements.\r\n" );
			return;
		}

		chance = ( int ) ( ch->pcdata->learned[gsn_reinforcements] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin making the call for reinforcements.\r\n", ch );
			act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 1, do_reinforcements, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou call for reinforcements but nobody answers.\r\n", ch );
		learn_from_failure( ch, gsn_reinforcements );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted before you can finish your call.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	send_to_char( "&GYour reinforcements are on the way.\r\n", ch );
	credits = ch->skill_level[LEADERSHIP_ABILITY] * 10;
	ch_printf( ch, "It cost you %d dollars.\r\n", credits );
	ch->gold -= UMIN( credits, ch->gold );

	learn_from_success( ch, gsn_reinforcements );
	ch->backup_mob = MOB_VNUM_MERC;

	ch->backup_wait = number_range( 1, 2 );

}

CMDF( do_postguard )
{
	char arg[MAX_INPUT_LENGTH];
	int chance, credits;

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "&RNot while in the Arena!\r\n", ch );
		return;
	}

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

	switch( ch->substate )
	{
	default:
		if( ch->backup_wait )
		{
			send_to_char( "&RYou already have backup coming.\r\n", ch );
			return;
		}

		if( !ch->pcdata->clan )
		{
			send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\r\n", ch );
			return;
		}

		if( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 10 )
		{
			ch_printf( ch, "&RYou dont have enough money.\r\n", ch );
			return;
		}

		chance = ( int ) ( ch->pcdata->learned[gsn_postguard] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin making the call for reinforcements.\r\n", ch );
			act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 1, do_postguard, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou call for a guard but nobody answers.\r\n", ch );
		learn_from_failure( ch, gsn_postguard );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted before you can finish your call.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	send_to_char( "&GYour guard is on the way.\r\n", ch );

	credits = ch->skill_level[LEADERSHIP_ABILITY] * 10;
	ch_printf( ch, "It cost you %d dollars.\r\n", credits );
	ch->gold -= UMIN( credits, ch->gold );

	learn_from_success( ch, gsn_postguard );
	ch->backup_mob = MOB_VNUM_BOUNCER;

	ch->backup_wait = 1;

}

void add_reinforcements( CHAR_DATA *ch )
{
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *blaster;
	OBJ_INDEX_DATA *pObjIndex;

	if( ( pMobIndex = get_mob_index( ch->backup_mob ) ) == NULL )
		return;

	if( ch->backup_mob == MOB_VNUM_MERC )
	{
		CHAR_DATA *mob[3];
		int mob_cnt;

		send_to_char( "Your reinforcements have arrived.\r\n", ch );
		for( mob_cnt = 0; mob_cnt < 3; mob_cnt++ )
		{
			int ability;
			mob[mob_cnt] = create_mobile( pMobIndex );
			char_to_room( mob[mob_cnt], ch->in_room );
			act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[mob_cnt], TO_ROOM );
			mob[mob_cnt]->top_level = ch->skill_level[LEADERSHIP_ABILITY] / 3;
			for( ability = 0; ability < MAX_ABILITY; ability++ )
				mob[mob_cnt]->skill_level[ability] = mob[mob_cnt]->top_level;
			mob[mob_cnt]->hit = mob[mob_cnt]->top_level * 15;
			mob[mob_cnt]->max_hit = mob[mob_cnt]->hit;
			mob[mob_cnt]->armor = 100 - mob[mob_cnt]->top_level * 2.5;
			mob[mob_cnt]->damroll = mob[mob_cnt]->top_level / 5;
			mob[mob_cnt]->hitroll = mob[mob_cnt]->top_level / 5;
			if( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
			{
				blaster = create_object( pObjIndex, mob[mob_cnt]->top_level );
				obj_to_char( blaster, mob[mob_cnt] );
				equip_char( mob[mob_cnt], blaster, WEAR_WIELD );
			}
			if( mob[mob_cnt]->master )
				stop_follower( mob[mob_cnt] );
			add_follower( mob[mob_cnt], ch );
			SET_BIT( mob[mob_cnt]->affected_by, AFF_CHARM );
			do_setblaster( mob[mob_cnt], "full" );
		}
	}
	else
	{
		CHAR_DATA *mob;
		int ability;

		mob = create_mobile( pMobIndex );
		char_to_room( mob, ch->in_room );
		if( ch->pcdata && ch->pcdata->clan )
		{
			char tmpbuf[MAX_STRING_LENGTH];

			STRFREE( mob->name );
			mob->name = STRALLOC( ch->pcdata->clan->name );
			sprintf( tmpbuf, "(%s) %s", ch->pcdata->clan->name, mob->long_descr );
			STRFREE( mob->long_descr );
			mob->long_descr = STRALLOC( tmpbuf );
		}
		act( AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM );
		send_to_char( "Your guard has arrived.\r\n", ch );
		mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
		for( ability = 0; ability < MAX_ABILITY; ability++ )
			mob->skill_level[ability] = mob->top_level;
		mob->hit = mob->top_level * 15;
		mob->max_hit = mob->hit;
		mob->armor = 100 - mob->top_level * 2.5;
		mob->damroll = mob->top_level / 5;
		mob->hitroll = mob->top_level / 5;
		if( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
		{
			blaster = create_object( pObjIndex, mob->top_level );
			obj_to_char( blaster, mob );
			equip_char( mob, blaster, WEAR_WIELD );
		}

		/*
		 * for making this more accurate in the future
		 */

		if( mob->mob_clan )
			STRFREE( mob->mob_clan );
		if( ch->pcdata && ch->pcdata->clan )
			mob->mob_clan = STRALLOC( ch->pcdata->clan->name );
	}
}

/*
void do_torture( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, dam;
	bool fail;

	if ( !IS_NPC(ch)
	&&  ch->pcdata->learned[gsn_torture] <= 0  )
	{
	send_to_char(
		"Your mind races as you realize you have no idea how to do that.\r\n", ch );
	return;
	}

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
	send_to_char( "You can't do that right now.\r\n", ch );
	return;
	}

	one_argument( argument, arg );

	if ( ch->mount )
	{
	send_to_char( "You can't get close enough while mounted.\r\n", ch );
	return;
	}

	 if ( !IS_NPC(ch) && !IS_NPC(victim) )
   {

	 if ( !xIS_SET(ch->act, PLR_PKER) )
	{
		send_to_char( "You can't do that. You are peaceful.\r\n", ch );
		return;
	}

	if ( !xIS_SET(victim->act, PLR_PKER) )
	{
		send_to_char( "You can't do that. They are peaceful.\r\n", ch );
		return;
	}
 }

	if ( arg[0] == '\0' )
	{
	send_to_char( "Torture whom?\r\n", ch );
	return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	send_to_char( "They aren't here.\r\n", ch );
	return;
	}

	if ( victim == ch )
	{
	send_to_char( "Are you masacistic or what...\r\n", ch );
	return;
	}

	if ( !IS_AWAKE(victim) )
	{
	send_to_char( "You need to wake them first.\r\n", ch );
	return;
	}

	if ( is_safe( ch, victim ) )
	  return;

	if ( victim->fighting )
	{
	send_to_char( "You can't torture someone whos in combat.\r\n", ch );
	return;
	}
	if ( !IS_NPC(victim) && !xIS_SET(victim->act, PLR_PKER) )
	{
		send_to_char( "You can't do that. They are peaceful.\r\n", ch );
		return;
	}

	ch->alignment = ch->alignment -= 100;
	ch->alignment = URANGE( -1000, ch->alignment, 1000 );

	WAIT_STATE( ch, skill_table[gsn_torture]->beats );

	fail = false;
	chance = ris_save( victim, ch->skill_level[HUNTING_ABILITY], RIS_PARALYSIS );
	if ( chance == 1000 )
	  fail = true;
	else
	  fail = saves_para_petri( chance, victim );

	if ( !IS_NPC(ch) && !IS_NPC(victim) )
	  chance = sysdata.stun_plr_vs_plr;
	else
	  chance = sysdata.stun_regular;
	if ( !fail
	&& (  IS_NPC(ch)
	|| (number_percent( ) + chance) < ch->pcdata->learned[gsn_torture] ) )
	{
	learn_from_success( ch, gsn_torture );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, PULSE_VIOLENCE );
	act( AT_SKILL, "$N slowly tortures you. The pain is excruciating.", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You torture $N, leaving $M screaming in pain.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tortures $N, leaving $M screaming in agony!", ch, NULL, victim, TO_NOTVICT );

		dam = dice( ch->skill_level[HUNTING_ABILITY]/10 , 4 );
		dam = URANGE( 0, victim->max_hit-10, dam );
		victim->hit -= dam;
		victim->max_hit -= dam;

		ch_printf( victim, "You lose %d permanent hit points." ,dam);
		ch_printf( ch, "They lose %d permanent hit points." , dam);

	}
	else
	if ( !IS_NPC(victim) && !xIS_SET(victim->act, PLR_PKER) )
	{
		send_to_char( "You can't do that. They are peaceful.\r\n", ch );
		return;
	}
	{
	act( AT_SKILL, "$N tries to cut off your finger!", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You mess up big time.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tries to painfully torture $N.", ch, NULL, victim, TO_NOTVICT );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
		global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}
	return;

}
*/
CMDF( do_disguise )
{
	int chance;

	if( IS_NPC( ch ) )
		return;

	/*	if( !has_rate( ch, RATE_TITLE ) )
		{
			send_to_char( "You don't rate disguises! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
	{
		send_to_char( "You try but the fates forsake you.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Change your title to what?\r\n", ch );
		return;
	}

	chance = ( int ) ( ch->pcdata->learned[gsn_disguise] );

	if( number_percent( ) > chance )
	{
		send_to_char( "You try to disguise yourself but fail.\r\n", ch );
		return;
	}

	char title[50];
	mudstrlcpy( title, argument, 50 );

	smash_tilde( title );
	set_title( ch, title );
	learn_from_success( ch, gsn_disguise );
	send_to_char( "Ok.\r\n", ch );
}

CMDF( do_mine )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	bool shovel;
	short move;

	if( ch->pcdata->learned[gsn_mine] <= 0 )
	{
		ch_printf( ch, "You have no idea how to do that.\r\n" );
		return;
	}

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "And what will you mine the room with?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	shovel = false;
	for( obj = ch->first_carrying; obj; obj = obj->next_content )
		if( obj->item_type == ITEM_SHOVEL )
		{
			shovel = true;
			break;
		}

	obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
	if( !obj )
	{
		send_to_char( "You don't see on here.\r\n", ch );
		return;
	}

	separate_obj( obj );
	if( obj->item_type != ITEM_LANDMINE )
	{
		act( AT_PLAIN, "That's not a landmine!", ch, obj, 0, TO_CHAR );
		return;
	}

	if( !CAN_WEAR( obj, ITEM_TAKE ) )
	{
		act( AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR );
		return;
	}

	switch( ch->in_room->sector_type )
	{
	case SECT_CITY:
	case SECT_INSIDE:
		send_to_char( "The floor is too hard to dig through.\r\n", ch );
		return;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
		send_to_char( "You cannot bury a mine in the water.\r\n", ch );
		return;
	case SECT_AIR:
		send_to_char( "What?  Bury a mine in the air?!\r\n", ch );
		return;
	}

	if( obj->weight > ( UMAX( 5, ( can_carry_w( ch ) / 10 ) ) ) && !shovel )
	{
		send_to_char( "You'd need a shovel to bury something that big.\r\n", ch );
		return;
	}

	move = ( obj->weight * 50 * ( shovel ? 1 : 5 ) ) / UMAX( 1, can_carry_w( ch ) );
	move = URANGE( 2, move, 1000 );
	if( move > ch->move )
	{
		send_to_char( "You don't have the energy to bury something of that size.\r\n", ch );
		return;
	}
	ch->move -= move;

	xSET_BIT( obj->extra_flags, ITEM_BURRIED );
	WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );

	STRFREE( obj->armed_by );
	obj->armed_by = STRALLOC( ch->name );

	ch_printf( ch, "You arm and bury %s.\r\n", obj->short_descr );
	act( AT_PLAIN, "$n arms and buries $p.", ch, obj, NULL, TO_ROOM );

	learn_from_success( ch, gsn_mine );

	return;
}

CMDF( do_first_aid )
{
	OBJ_DATA *medpac;
	CHAR_DATA *victim;
	int heal;
	char buf[MAX_STRING_LENGTH];

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You can't do that while fighting!\r\n", ch );
		return;
	}

	medpac = get_eq_char( ch, WEAR_HOLD );
	if( !medpac || medpac->item_type != ITEM_MEDPAC )
	{
		send_to_char( "You need to be holding a medpac.\r\n", ch );
		return;
	}

	if( medpac->value[0] <= 0 )
	{
		send_to_char( "Your medpac seems to be empty.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
		victim = ch;
	else
		victim = get_char_room( ch, argument );

	if( !victim )
	{
		ch_printf( ch, "I don't see any %s here...\r\n", argument );
		return;
	}

	heal = number_range( 1, 150 );

	if( heal > ch->pcdata->learned[gsn_first_aid] * 2 )
	{
		ch_printf( ch, "You fail in your attempt at first aid.\r\n" );
		learn_from_failure( ch, gsn_first_aid );
		return;
	}

	if( victim == ch )
	{
		ch_printf( ch, "You tend to your wounds.\r\n" );
		sprintf( buf, "$n uses %s to help heal $s wounds.", medpac->short_descr );
		act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
	}
	else
	{
		sprintf( buf, "You tend to $N's wounds." );
		act( AT_ACTION, buf, ch, NULL, victim, TO_CHAR );
		sprintf( buf, "$n uses %s to help heal $N's wounds.", medpac->short_descr );
		act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
		sprintf( buf, "$n uses %s to help heal your wounds.", medpac->short_descr );
		act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
	}

	--medpac->value[0];
	victim->hit += URANGE( 0, heal, victim->max_hit - victim->hit );

	learn_from_success( ch, gsn_first_aid );
}

CMDF( do_snipe )
{
	OBJ_DATA *wield;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	short dir, dist;
	short max_dist = 3;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in_room;
	ROOM_INDEX_DATA *to_room;
	CHAR_DATA *victim;
	int chance;
	char buf[MAX_STRING_LENGTH];
	bool pfound = false;

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You'll have to do that elswhere.\r\n", ch );
		return;
	}

	if( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
	{
		send_to_char( "You can't do that while wielding two weapons.", ch );
		return;
	}

	wield = get_eq_char( ch, WEAR_WIELD );
	if( !wield || wield->item_type != ITEM_WEAPON || wield->value[3] != WEAPON_BLASTER )
	{
		send_to_char( "You don't seem to be holding a blaster", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( ( dir = get_door( arg ) ) == -1 || arg2[0] == '\0' )
	{
		send_to_char( "Usage: snipe <dir> <target>\r\n", ch );
		return;
	}

	if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	{
		send_to_char( "Are you expecting to fire through a wall!?\r\n", ch );
		return;
	}

	if( IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
		send_to_char( "Are you expecting to fire through a door!?\r\n", ch );
		return;
	}

	was_in_room = ch->in_room;

	for( dist = 0; dist <= max_dist; dist++ )
	{
		if( IS_SET( pexit->exit_info, EX_CLOSED ) )
			break;

		if( !pexit->to_room )
			break;

		to_room = NULL;
		if( pexit->distance > 1 )
			to_room = generate_exit( ch->in_room, &pexit );

		if( to_room == NULL )
			to_room = pexit->to_room;

		char_from_room( ch );
		char_to_room( ch, to_room );


		if( IS_NPC( ch ) && ( victim = get_char_room_mp( ch, arg2 ) ) != NULL )
		{
			pfound = true;
			break;
		}
		else if( !IS_NPC( ch ) && ( victim = get_char_room( ch, arg2 ) ) != NULL )
		{
			pfound = true;
			break;
		}


		if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
			break;

	}

	char_from_room( ch );
	char_to_room( ch, was_in_room );

	if( !pfound )
	{
		ch_printf( ch, "You don't see that person to the %s!\r\n", dir_name[dir] );
		char_from_room( ch );
		char_to_room( ch, was_in_room );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Shoot yourself ... really?\r\n", ch );
		return;
	}

	if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You can't shoot them there.\r\n", ch );
		return;
	}

	if( is_safe( ch, victim ) )
		return;

	if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
	{
		act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You do the best you can!\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
	{
		send_to_char( "You feel too nice to do that!\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && !xIS_SET( victim->act, PLR_PKER ) )
	{
		send_to_char( "You can't do that. They are peaceful.\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? 100 : ( int ) ( ch->pcdata->learned[gsn_snipe] );

	switch( dir )
	{
	case 0:
	case 1:
		dir += 2;
		break;
	case 2:
	case 3:
		dir -= 2;
		break;
	case 4:
	case 7:
		dir += 1;
		break;
	case 5:
	case 8:
		dir -= 1;
		break;
	case 6:
		dir += 3;
		break;
	case 9:
		dir -= 3;
		break;
	}

	char_from_room( ch );
	char_to_room( ch, victim->in_room );

	if( number_percent( ) < chance )
	{
		char_from_room( ch );
		char_to_room( ch, was_in_room );
		sprintf( buf, "$n fires a shot to the %s.", dir_name[get_door( arg )] );
		act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

		char_from_room( ch );
		char_to_room( ch, victim->in_room );

		sprintf( buf, "A shot fires at you from the %s.", dir_name[dir] );
		act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
		act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );
		sprintf( buf, "A shot fires at $N from the %s.", dir_name[dir] );
		act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );

		one_hit( ch, victim, TYPE_UNDEFINED );

		if( char_died( ch ) )
			return;

		stop_fighting( ch, true );

		learn_from_success( ch, gsn_snipe );
	}
	else
	{
		char_from_room( ch );
		char_to_room( ch, was_in_room );
		sprintf( buf, "$n fires a shot to the %s.", dir_name[get_door( arg )] );
		act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

		char_from_room( ch );
		char_to_room( ch, victim->in_room );

		act( AT_ACTION, "You fire at $N but don't even come close.", ch, NULL, victim, TO_CHAR );
		sprintf( buf, "A shot fired from the %s barely misses you.", dir_name[dir] );
		act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
		learn_from_failure( ch, gsn_snipe );
	}

	char_from_room( ch );
	char_to_room( ch, was_in_room );

	if( IS_NPC( ch ) )
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	else
	{
		if( number_percent( ) < ch->pcdata->learned[gsn_fifth_attack] )
			WAIT_STATE( ch, 1 * PULSE_PER_SECOND );
		else if( number_percent( ) < ch->pcdata->learned[gsn_fourth_attack] )
			WAIT_STATE( ch, 2 * PULSE_PER_SECOND );
		else if( number_percent( ) < ch->pcdata->learned[gsn_third_attack] )
			WAIT_STATE( ch, 3 * PULSE_PER_SECOND );
		else if( number_percent( ) < ch->pcdata->learned[gsn_second_attack] )
			WAIT_STATE( ch, 4 * PULSE_PER_SECOND );
		else
			WAIT_STATE( ch, 5 * PULSE_PER_SECOND );
	}
	if( IS_NPC( victim ) && !char_died( victim ) )
	{
		if( xIS_SET( victim->act, ACT_SENTINEL ) )
		{
			victim->was_sentinel = victim->in_room;
			xREMOVE_BIT( victim->act, ACT_SENTINEL );
		}

		start_hating( victim, ch );
		start_hunting( victim, ch );

	}

}

/* syntax throw <obj> [direction] [target] */

CMDF( do_throw )
{
	OBJ_DATA *obj;
	OBJ_DATA *tmpobj;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	short dir;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in_room;
	ROOM_INDEX_DATA *to_room;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];


	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	was_in_room = ch->in_room;

	if( arg[0] == '\0' )
	{
		send_to_char( "Usage: throw <object> [direction] [target]\r\n", ch );
		return;
	}

	obj = get_eq_char( ch, WEAR_MISSILE_WIELD );
	if( !obj || !nifty_is_name( arg, obj->name ) )
		obj = get_eq_char( ch, WEAR_HOLD );
	if( !obj || !nifty_is_name( arg, obj->name ) )
		obj = get_eq_char( ch, WEAR_WIELD );
	if( !obj || !nifty_is_name( arg, obj->name ) )
		obj = get_eq_char( ch, WEAR_DUAL_WIELD );
	if( !obj || !nifty_is_name( arg, obj->name ) )
		if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
			obj = get_eq_char( ch, WEAR_HOLD );
	if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
		obj = get_eq_char( ch, WEAR_WIELD );
	if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
		obj = get_eq_char( ch, WEAR_DUAL_WIELD );
	if( !obj || !nifty_is_name_prefix( arg, obj->name ) )
	{
		ch_printf( ch, "You don't seem to be holding or wielding %s.\r\n", arg );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
	{
		act( AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		victim = who_fighting( ch );
		if( char_died( victim ) )
			return;
		act( AT_ACTION, "You throw $p at $N.", ch, obj, victim, TO_CHAR );
		act( AT_ACTION, "$n throws $p at $N.", ch, obj, victim, TO_NOTVICT );
		act( AT_ACTION, "$n throw $p at you.", ch, obj, victim, TO_VICT );
	}

	else if( arg2[0] == '\0' )
	{
		sprintf( buf, "$n throws %s at the floor.", obj->short_descr );
		act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
		ch_printf( ch, "You throw %s at the floor.\r\n", obj->short_descr );

		victim = NULL;
	}
	else if( ( dir = get_door( arg2 ) ) != -1 )
	{
		if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
		{
			send_to_char( "Are you expecting to throw it through a wall!?\r\n", ch );
			return;
		}


		if( IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			send_to_char( "Are you expecting to throw it  through a door!?\r\n", ch );
			return;
		}


		switch( dir )
		{
		case 0:
		case 1:
			dir += 2;
			break;
		case 2:
		case 3:
			dir -= 2;
			break;
		case 4:
		case 7:
			dir += 1;
			break;
		case 5:
		case 8:
			dir -= 1;
			break;
		case 6:
			dir += 3;
			break;
		case 9:
			dir -= 3;
			break;
		}

		to_room = NULL;
		if( pexit->distance > 1 )
			to_room = generate_exit( ch->in_room, &pexit );

		if( to_room == NULL )
			to_room = pexit->to_room;


		char_from_room( ch );
		char_to_room( ch, to_room );

		victim = get_char_room( ch, arg3 );

		if( victim )
		{
			if( is_safe( ch, victim ) )
				return;

			if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
			{
				act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
				return;
			}

			if( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
			{
				send_to_char( "You feel too nice to do that!\r\n", ch );
				return;
			}

			char_from_room( ch );
			char_to_room( ch, was_in_room );


			if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You'll have to do that elswhere.\r\n", ch );
				return;
			}

			to_room = NULL;
			if( pexit->distance > 1 )
				to_room = generate_exit( ch->in_room, &pexit );

			if( to_room == NULL )
				to_room = pexit->to_room;


			char_from_room( ch );
			char_to_room( ch, to_room );

			sprintf( buf, "Someone throws %s at you from the %s.", obj->short_descr, dir_name[dir] );
			act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
			act( AT_ACTION, "You throw %p at $N.", ch, obj, victim, TO_CHAR );
			char_from_room( ch );
			char_to_room( ch, was_in_room );
			sprintf( buf, "$n throws %s to the %s.", obj->short_descr, dir_name[get_dir( arg2 )] );
			act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, to_room );
			sprintf( buf, "%s is thrown at $N from the %s.", obj->short_descr, dir_name[dir] );
			act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
		}
		else
		{
			ch_printf( ch, "You throw %s %s.\r\n", obj->short_descr, dir_name[get_dir( arg2 )] );
			char_from_room( ch );
			char_to_room( ch, was_in_room );
			sprintf( buf, "$n throws %s to the %s.", obj->short_descr, dir_name[get_dir( arg2 )] );
			act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, to_room );
			sprintf( buf, "%s is thrown from the %s.", obj->short_descr, dir_name[dir] );
			act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
		}
	}

	else if( ( victim = get_char_room( ch, arg2 ) ) != NULL )
	{
		if( is_safe( ch, victim ) )
			return;

		if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
		{
			act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
			return;
		}

		if( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
		{
			send_to_char( "You feel too nice to do that!\r\n", ch );
			return;
		}

	}
	else
	{
		ch_printf( ch, "They don't seem to be here!\r\n" );
		return;
	}


	if( obj == get_eq_char( ch, WEAR_WIELD ) && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
		tmpobj->wear_loc = WEAR_WIELD;

	unequip_char( ch, obj );
	separate_obj( obj );
	obj_from_char( obj );
	obj = obj_to_room( obj, ch->in_room );

	if( obj->item_type != ITEM_GRENADE )
		damage_obj( obj );

	/* NOT NEEDED UNLESS REFERING TO OBJECT AGAIN

	   if( obj_extracted(obj) )
		  return;
	*/
	if( ch->in_room != was_in_room )
	{
		char_from_room( ch );
		char_to_room( ch, was_in_room );
	}

	if( !victim || char_died( victim ) )
		learn_from_failure( ch, gsn_throw );
	else
	{

		WAIT_STATE( ch, skill_table[gsn_throw]->beats );
		if( IS_NPC( ch ) || number_percent( ) < ch->pcdata->learned[gsn_throw] )
		{
			learn_from_success( ch, gsn_throw );
			global_retcode =
				damage( ch, victim, number_range( obj->weight * 2, ( obj->weight * 2 + ch->perm_str ) ), TYPE_HIT );
		}
		else
		{
			learn_from_failure( ch, gsn_throw );
			global_retcode = damage( ch, victim, 0, TYPE_HIT );
		}

		if( IS_NPC( victim ) && !char_died( victim ) )
		{
			if( xIS_SET( victim->act, ACT_SENTINEL ) )
			{
				victim->was_sentinel = victim->in_room;
				xREMOVE_BIT( victim->act, ACT_SENTINEL );
			}

			start_hating( victim, ch );
			start_hunting( victim, ch );

		}

	}

	return;

}

CMDF( do_beg )
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int percent, xp;
	int amount;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg1 );

	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Beg fo money from whom?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "They're a little busy right now.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position <= POS_SLEEPING )
	{
		send_to_char( "You might want to wake them first...\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) )
	{
		send_to_char( "You beg them for money.\r\n", ch );
		act( AT_ACTION, "$n begs you to give $s some change.\r\n", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n begs $N for change.\r\n", ch, NULL, victim, TO_NOTVICT );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_beg]->beats );
	percent = number_percent( ) + ch->skill_level[SMUGGLING_ABILITY] + victim->top_level;

	if( percent > ch->pcdata->learned[gsn_beg] )
	{
		/*
		 * Failure.
		 */
		send_to_char( "You beg them for money but don't get any!\r\n", ch );
		act( AT_ACTION, "$n is really getting on your nerves with all this begging!\r\n", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n begs $N for money.\r\n", ch, NULL, victim, TO_NOTVICT );

		if( victim->alignment < 0 && victim->top_level >= ch->top_level + 5 )
		{
			sprintf( buf, "%s is an annoying beggar and needs to be taught a lesson!", ch->name );
			do_yell( victim, buf );
			global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		}

		learn_from_failure( ch, gsn_beg );

		return;
	}


	act( AT_ACTION, "$n begs $N for money.\r\n", ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "$n begs you for money!\r\n", ch, NULL, victim, TO_VICT );

	amount = UMIN( victim->gold, number_range( 1, 10 ) );
	if( amount <= 0 )
	{
		do_look( victim, ch->name );
		do_say( victim, "Sorry I have nothing to spare.\r\n" );
		learn_from_failure( ch, gsn_beg );
		return;
	}

	ch->gold += amount;
	victim->gold -= amount;
	ch_printf( ch, "%s gives you %d dollars.\r\n", victim->short_descr, amount );
	learn_from_success( ch, gsn_beg );
	xp =
		UMIN( amount * 10,
			( exp_level( ch->skill_level[SMUGGLING_ABILITY] + 1 ) - exp_level( ch->skill_level[SMUGGLING_ABILITY] ) ) );
	xp = UMIN( xp, xp_compute( ch, victim ) );
	gain_exp( ch, xp, SMUGGLING_ABILITY );
	ch_printf( ch, "&WYou gain %ld smuggling experience points!\r\n", xp );
	act( AT_ACTION, "$N gives $n some money.\r\n", ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "You give $n some money.\r\n", ch, NULL, victim, TO_VICT );

	return;

}

CMDF( do_pickshiplock )
{
	do_pick( ch, argument );
}

CMDF( do_hijack )
{
	int chance;
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *p, *p_prev, *victim;
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *toroom;
	int value;
	int rand;

	if( IS_NPC( ch ) )
		return;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	if( ship->lastdoc != ship->location )
	{
		send_to_char( "&rYou don't seem to be docked right now.\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "The ship is not docked right now.\r\n", ch );
		return;
	}

	if( ship->stype == 7 && !xIS_SET( ch->act, PLR_PKER ) )
	{
		send_to_char( "You so wish, don't you? Ahaha Nice try.\r\n", ch );
		return;
	}

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "&CSyntax&c: &zHijack &c<&zLaunch Code&c>\r\n", ch );
		return;
	}

	value = is_number( arg ) ? atoi( arg ) : -1;

	if( atoi( arg ) < -1 && value == -1 )
		value = atoi( arg );

	if( ( value < 0 || value >= 100 ) )
	{
		ch_printf( ch, "&RA Launch code is between 0 and 99!.\r\n" );
		return;
	}

	if( atoi( arg ) != ship->code )
	{
		send_to_char( "&RWrong launch code!\r\n", ch );
		if( ( toroom = get_room_index( ship->location ) ) != NULL )
		{
			ship->hatchopen = false;
			act( AT_PLAIN, "$n exits the ship.", ch, NULL, argument, TO_ROOM );
			act( AT_PLAIN, "You are forced out of the suit.", ch, NULL, argument, TO_CHAR );
			char_from_room( ch );
			char_to_room( ch, toroom );
			act( AT_PLAIN, "$n steps out of a ship.", ch, NULL, argument, TO_ROOM );
			do_look( ch, "auto" );
			rand = number_range( 10, 99 );
			ship->code = rand;
			return;
		}

	}
	if( check_pilot( ch, ship ) )
	{
		send_to_char( "&RWhat would be the point of that!\r\n", ch );
		return;
	}

	if( ship->type == MOB_SHIP && get_trust( ch ) < 102 )
	{
		send_to_char( "&RThis ship isn't pilotable by mortals at this point in time...\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "The ships drive is disabled .\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_hijack] );
	if( number_percent( ) > chance )
	{
		send_to_char( "You fail to figure out the correct launch code.\r\n", ch );
		learn_from_failure( ch, gsn_hijack );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( ship->ship_class == CAPITAL_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_capitalships] );
	if( number_percent( ) < chance )
	{

		if( ship->hatchopen )
		{
			ship->hatchopen = false;
			sprintf( buf, "The hatch on %s closes.", ship->name );
			echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
			echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch slides shut." );
		}
		set_char_color( AT_GREEN, ch );
		send_to_char( "Launch code succeeded!.\r\n", ch );
		act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch, NULL, argument, TO_ROOM );
		echo_to_ship( AT_YELLOW, ship, "The ship hums as it lifts off the ground." );
		sprintf( buf, "%s begins to launch.", ship->name );
		echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
		rand = number_range( 10, 99 );
		ship->code = rand;
		ship->shipstate = SHIP_LAUNCH;
		ship->currspeed = ship->realspeed;
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_success( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_success( ch, gsn_midships );
		if( ship->ship_class == CAPITAL_SHIP )
			learn_from_success( ch, gsn_capitalships );

		learn_from_success( ch, gsn_hijack );
		//                   sprintf( buf, "%s has been hijacked!", ship->name );
		//             echo_to_all( AT_RED , buf, 0 );
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			ship->pksuit = 1;
		}
		else if( !str_cmp( "Holosuit", ship->owner ) )
		{
			ship->pksuit = 1;
		}
		else
		{
			ship->pksuit = 0;
		}

		for( p = last_char; p; p = p_prev )

		{
			p_prev = p->prev;  /* TRI */
			if( !IS_NPC( p ) && get_trust( p ) >= LEVEL_LIAISON )
			{
				sprintf( buf2, "%s", ship->name );
				ch_printf( p, "&R[&PA&pL&PA&pR&PM&R] &P%s has been hijacked by &P%s&p!\r\n", buf2, ch->name );
			}
		}

		//            if ( ship->alarm == 0 )
		//                return;
		if( !str_cmp( "Public", ship->owner ) )
			return;
		for( victim = first_char; victim; victim = victim->next )
		{
			bool victim_comlink;
			OBJ_DATA *obj;

			if( !check_pilot( victim, ship ) )
				continue;

			victim_comlink = false;
			if( IS_IMMORTAL( victim ) )
				victim_comlink = true;
			for( obj = victim->last_carrying; obj; obj = obj->prev_content )
			{
				if( obj->pIndexData->item_type == ITEM_COMLINK )
					victim_comlink = true;
			}
			if( !victim_comlink )
				continue;

			if( !IS_NPC( victim ) && victim->switched )
				continue;

			if( !IS_AWAKE( victim ) || xIS_SET( victim->in_room->room_flags, ROOM_SILENCE ) )
				continue;

			ch_printf( victim, "&R[&PA&pL&PA&pR&PM&R] &P%s &phas been hijacked by &P%s&p!\r\n", ship->name, ch->name );
		}



		return;
	}



	set_char_color( AT_RED, ch );
	send_to_char( "You fail to work the controls properly!\r\n", ch );
	if( ship->ship_class == MOBILE_SUIT )
		learn_from_failure( ch, gsn_mobilesuits );
	if( ship->ship_class == TRANSPORT_SHIP )
		learn_from_failure( ch, gsn_midships );
	if( ship->ship_class == CAPITAL_SHIP )
		learn_from_failure( ch, gsn_capitalships );
	return;

}

CMDF( do_add_patrol )
{
}

CMDF( do_special_forces )
{
}

CMDF( do_elite_guard )
{

}

CMDF( do_jail )
{
	CHAR_DATA *victim = NULL;
	CLAN_DATA *clan = NULL;
	ROOM_INDEX_DATA *jail = NULL;

	if( IS_NPC( ch ) )
		return;

	if( !ch->pcdata || ( clan = ch->pcdata->clan ) == NULL )
	{
		send_to_char( "Only members of organizations can jail their enemies.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "&RNot while in the Arena!\r\n", ch );
		return;
	}

	jail = get_room_index( clan->jail );
	if( !jail && clan->mainclan )
		jail = get_room_index( clan->mainclan->jail );

	if( !jail )
	{
		send_to_char( "Your orginization does not have a suitable prison.\r\n", ch );
		return;
	}
	/*
		if ( jail->area && ch->in_room->area
		&& jail->area != ch->in_room->area &&
		( !jail->area->planet || jail->area->planet != ch->in_room->area->planet ) )
		{
			 send_to_char( "Your orginizations prison is to far away.\r\n", ch );
		 return;
		}
	*/
	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Jail who?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "That would be a waste of time.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position >= POS_SLEEPING )
	{
		send_to_char( "You will have to stun them first.\r\n", ch );
		return;
	}

	send_to_char( "You have them escorted off to jail.\r\n", ch );
	act( AT_ACTION, "You have a strange feeling that you've been moved.\r\n", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n has $N escorted away.\r\n", ch, NULL, victim, TO_NOTVICT );

	char_from_room( victim );
	char_to_room( victim, jail );

	act( AT_ACTION, "The door opens briefly as $n is shoved into the room.\r\n", victim, NULL, NULL, TO_ROOM );

	learn_from_success( ch, gsn_jail );

	return;
}

CMDF( do_smalltalk )
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL;
	PLANET_DATA *planet = NULL;
	CLAN_DATA *clan = NULL;
	int percent;

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "What would be the point of that.\r\n", ch );
	}

	argument = one_argument( argument, arg1 );

	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Create smalltalk with whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "They're a little busy right now.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, ACT_SENTINEL ) || ( victim->position != POS_STANDING ) )
	{
		send_to_char( "They're not willing to listen.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_char( "Try using that on someone who isn't being charmed!", ch );
		return;
	}

	if( ch->master )
		stop_follower( ch );

	if( !IS_NPC( victim ) || victim->vip_flags == 0 )
	{
		send_to_char( "Diplomacy would be wasted on them.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position <= POS_SLEEPING )
	{
		send_to_char( "You might want to wake them first...\r\n", ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_smalltalk]->beats );

	if( ( percent = number_percent( ) - ch->skill_level[DIPLOMACY_ABILITY] + victim->top_level )
	> ch->pcdata->learned[gsn_smalltalk] )
	{
		/*
		 * Failure.
		 */
		send_to_char( "You attempt to make smalltalk with them.. but are ignored.\r\n", ch );
		act( AT_ACTION, "$n is really getting on your nerves with all this chatter!\r\n", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n asks $N about the weather but is ignored.\r\n", ch, NULL, victim, TO_NOTVICT );

		if( victim->alignment < -500 && victim->top_level >= ch->top_level + 5 )
		{
			sprintf( buf, "SHUT UP %s!", ch->name );
			do_yell( victim, buf );
			global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		}

		return;
	}

	send_to_char( "You strike up a short conversation with them.\r\n", ch );
	act( AT_ACTION, "$n smiles at you and says, 'hello'.\r\n", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n chats briefly with $N.\r\n", ch, NULL, victim, TO_NOTVICT );

	if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
		return;

	if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
		clan = ch->pcdata->clan;

	planet = ch->in_room->area->planet;

	if( clan != planet->governed_by )
		return;

	planet->pop_support += 0.2;
	send_to_char( "Popular support for your organization increases slightly.\r\n", ch );

	gain_exp( ch, victim->top_level * 40, DIPLOMACY_ABILITY );
	ch_printf( ch, "You gain %d diplomacy experience.\r\n", victim->top_level * 10 );

	learn_from_success( ch, gsn_smalltalk );

	if( planet->pop_support > 200 )
		planet->pop_support = 200;
}

CMDF( do_propeganda )
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	PLANET_DATA *planet;
	CLAN_DATA *clan;
	int percent;

	if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area )
	{
		send_to_char( "What would be the point of that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );

	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Spread propeganda to who?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "They're a little busy right now.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, ACT_SENTINEL ) || ( victim->position != POS_STANDING ) )
	{
		send_to_char( "They're not willing to listen.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_char( "Try using that on someone who isn't being charmed!", ch );
		return;
	}

	if( ch->master )
		stop_follower( ch );

	if( victim->vip_flags == 0 )
	{
		send_to_char( "Diplomacy would be wasted on them.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position <= POS_SLEEPING )
	{
		send_to_char( "You might want to wake them first...\r\n", ch );
		return;
	}

	if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
		clan = ch->pcdata->clan;

	planet = ch->in_room->area->planet;

	sprintf( buf, ", and the evils of %s", planet->governed_by ? planet->governed_by->name : "their current leaders" );
	ch_printf( ch, "You speak to them about the benifits of the %s%s.\r\n", ch->pcdata->clan->name,
		planet->governed_by == clan ? "" : buf );
	act( AT_ACTION, "$n speaks about his organization.\r\n", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n tells $N about their organization.\r\n", ch, NULL, victim, TO_NOTVICT );

	WAIT_STATE( ch, skill_table[gsn_propeganda]->beats );

	percent = number_percent( ) - ( get_curr_cha( ch ) - 14 ) - ( ch->skill_level[DIPLOMACY_ABILITY] - victim->top_level );

	if( percent > ch->pcdata->learned[gsn_propeganda] )
	{

		if( planet->governed_by != clan )
		{
			sprintf( buf, "%s is a traitor!", ch->name );
			do_yell( victim, buf );
			global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
			return;
		}

	}

	if( planet->governed_by == clan )
	{
		planet->pop_support += 1;
		send_to_char( "Popular support for your organization increases.\r\n", ch );
	}
	else
	{
		planet->pop_support -= .25;
		send_to_char( "Popular support for the current government decreases.\r\n", ch );
	}

	gain_exp( ch, victim->top_level * 100, DIPLOMACY_ABILITY );
	ch_printf( ch, "You gain %d diplomacy experience.\r\n", victim->top_level * 100 );

	learn_from_success( ch, gsn_propeganda );

	if( planet->pop_support > 200 )
		planet->pop_support = 200;
	if( planet->pop_support < -100 )
		planet->pop_support = -100;

}

CMDF( do_bribe )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	PLANET_DATA *planet;
	CLAN_DATA *clan;
	int percent, amount;

	if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
	{
		send_to_char( "What would be the point of that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );

	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Bribe who how much?\r\n", ch );
		return;
	}

	amount = atoi( argument );

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, ACT_SENTINEL ) || ( victim->position != POS_STANDING ) )
	{
		send_to_char( "They're not willing to listen.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_char( "Try using that on someone who isn't being charmed!", ch );
		return;
	}

	if( ch->master )
		stop_follower( ch );

	if( amount <= 0 )
	{
		send_to_char( "A little bit more money would be a good plan.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "They're a little busy right now.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position <= POS_SLEEPING )
	{
		send_to_char( "You might want to wake them first...\r\n", ch );
		return;
	}

	if( victim->vip_flags == 0 )
	{
		send_to_char( "Diplomacy would be wasted on them.\r\n", ch );
		return;
	}

	if( ch->gold < amount )
	{
		send_to_char( "You don't have that much money.\r\n", ch );
		return;
	}

	ch->gold -= amount;
	//    victim->gold += amount;

	ch_printf( ch, "You give them a small gift on behalf of %s.\r\n", ch->pcdata->clan->name );
	act( AT_ACTION, "$n offers you a small bribe.\r\n", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n gives $N some money.\r\n", ch, NULL, victim, TO_NOTVICT );

	if( !IS_NPC( victim ) )
		return;

	WAIT_STATE( ch, skill_table[gsn_bribe]->beats );

	if( ( percent = number_percent( ) - amount + victim->top_level ) > ch->pcdata->learned[gsn_bribe] )
		return;

	if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
		clan = ch->pcdata->clan;

	planet = ch->in_room->area->planet;


	if( clan == planet->governed_by )
	{
		planet->pop_support += URANGE( 0.1, amount / 200000, 5 );
		send_to_char( "Popular support for your organization increases slightly.\r\n", ch );

		amount =
			UMIN( amount,
				( exp_level( ch->skill_level[DIPLOMACY_ABILITY] + 1 ) - exp_level( ch->skill_level[DIPLOMACY_ABILITY] ) ) );

		gain_exp( ch, amount, DIPLOMACY_ABILITY );
		ch_printf( ch, "You gain %d diplomacy experience.\r\n", amount );

		learn_from_success( ch, gsn_bribe );
	}

	if( planet->pop_support > 200 )
		planet->pop_support = 200;
}

CMDF( do_seduce )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Seduce whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}


	if( IS_AFFECTED( victim, AFF_CHARM ) && victim->master )
	{
		act( AT_PLAIN, "But he'd rather follow $N!", ch, NULL, victim->master, TO_CHAR );
		return;
	}

	if( circle_follow( victim, ch ) )
	{
		send_to_char( "Following in loops is not allowed... sorry.\r\n", ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_seduce]->beats );

	if( ( victim->top_level - get_curr_cha( ch ) > ch->pcdata->learned[gsn_seduce] ) || IS_SET( victim->immune, RIS_CHARM ) )
	{
		send_to_char( "You failed.\r\n", ch );
		sprintf( buf, "%s failed to seduce you.", ch->name );
		send_to_char( buf, victim );
		global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		return;
	}


	if( victim->master )
		stop_follower( victim );

	learn_from_success( ch, gsn_seduce );

	add_follower( victim, ch );
	return;

}



CMDF( do_mass_propeganda )
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	PLANET_DATA *planet;
	CLAN_DATA *clan;
	int percent;

	if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
	{
		send_to_char( "What would be the point of that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );

	if( ch->mount )
	{
		send_to_char( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Spread propeganda to who?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "This isn't a good place to do that.\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "Interesting combat technique.\r\n", ch );
		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "They're a little busy right now.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, ACT_SENTINEL ) || ( victim->position != POS_STANDING ) )
	{
		send_to_char( "They're not willing to listen.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( victim, AFF_CHARM ) )
	{
		send_to_char( "Try using that on someone who isn't being charmed!", ch );
		return;
	}


	if( victim->vip_flags == 0 )
	{
		send_to_char( "Diplomacy would be wasted on them.\r\n", ch );
		return;
	}

	if( ch->position <= POS_SLEEPING )
	{
		send_to_char( "In your dreams or what?\r\n", ch );
		return;
	}

	if( victim->position <= POS_SLEEPING )
	{
		send_to_char( "You might want to wake them first...\r\n", ch );
		return;
	}

	if( ( clan = ch->pcdata->clan->mainclan ) == NULL )
		clan = ch->pcdata->clan;

	planet = ch->in_room->area->planet;

	sprintf( buf, ", and the evils of %s", planet->governed_by ? planet->governed_by->name : "their current leaders" );
	ch_printf( ch, "You speak to them about the benifits of the %s%s.\r\n", ch->pcdata->clan->name,
		planet->governed_by == clan ? "" : buf );
	act( AT_ACTION, "$n speaks about his organization.\r\n", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n tells $N about their organization.\r\n", ch, NULL, victim, TO_NOTVICT );

	WAIT_STATE( ch, skill_table[gsn_masspropeganda]->beats );

	percent = number_percent( ) - ( get_curr_cha( ch ) - 14 ) - ( ch->skill_level[DIPLOMACY_ABILITY] - victim->top_level );

	if( percent > ch->pcdata->learned[gsn_masspropeganda] )
	{

		if( planet->governed_by != clan )
		{
			sprintf( buf, "%s is a traitor!", ch->name );
			do_yell( victim, buf );
			global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		}

	}

	if( planet->governed_by == clan )
	{
		planet->pop_support += 1.5;
		send_to_char( "Popular support for your organization increases.\r\n", ch );
	}
	else
	{
		planet->pop_support -= .375;
		send_to_char( "Popular support for the current government decreases.\r\n", ch );
	}

	gain_exp( ch, victim->top_level * 200, DIPLOMACY_ABILITY );
	ch_printf( ch, "You gain %d diplomacy experience.\r\n", victim->top_level * 200 );

	learn_from_success( ch, gsn_masspropeganda );

	if( planet->pop_support > 200 )
		planet->pop_support = 200;
	if( planet->pop_support < -100 )
		planet->pop_support = -100;

}


CMDF( do_gather_intelligence )
{
	CHAR_DATA *victim;

	char buf[MAX_STRING_LENGTH];

	int percent, chance;

	PLANET_DATA *planet;


	buf[0] = '\0';

	if( argument[0] == '\0' )

	{

		send_to_char( "You must input a name.\r\n", ch );

		return;

	}

	strcat( buf, "0." );

	strcat( buf, argument );


	WAIT_STATE( ch, skill_table[gsn_gather_intelligence]->beats );


	if( ( ( victim = get_char_world( ch, buf ) ) == NULL ) )

	{

		send_to_char( "You fail to gather information on that individual.\r\n", ch );

		return;

	}



	if( IS_NPC( victim ) )

	{
		send_to_char( "This person has not made much of a name for himself!\r\n", ch );
		return;
	}

	percent = number_percent( ) * 2;

	if( IS_NPC( ch ) || percent < ch->pcdata->learned[gsn_gather_intelligence] )
	{

		if( ch == victim )
		{
			send_to_char( "I am sure you know enough about yourself right now", ch );
			return;
		}

		learn_from_success( ch, gsn_gather_intelligence );

		chance = number_percent( );

		if( chance < 25 )
		{
			if( ( planet = victim->in_room->area->planet ) == NULL )
			{
				sprintf( buf, "Information has been recieved that %s is travelling.", victim->name );
				send_to_char( buf, ch );
				return;
			}
			else
			{
				sprintf( buf, "Information has been recieved that %s is on %s.", victim->name, planet->name );
				send_to_char( buf, ch );
				return;
			}
			return;
		}
		if( chance < 30 )
		{
			if( victim->pcdata->clan )
			{
				sprintf( buf, "%s seems to be involved with %s.", victim->name, victim->pcdata->clan->name );
				send_to_char( buf, ch );
				return;
			}
			else
			{
				sprintf( buf, "%s does not seem to be involved with any organization.", victim->name );
				send_to_char( buf, ch );
				return;
			}
			return;
		}

		if( chance < 40 )
		{
			if( victim->hit < ( ( victim->max_hit ) / 4 ) )
			{
				sprintf( buf, "Hospital records show that %s has had a very serious injury and has not fully recovered.",
					victim->name );
				send_to_char( buf, ch );
				return;
			}
			if( victim->hit < ( ( victim->max_hit ) / 2 ) )
			{
				sprintf( buf, "Hospital records show that %s has had a serious injury and has begun to recover.", victim->name );
				send_to_char( buf, ch );
				return;
			}
			if( victim->hit < ( ( victim->max_hit ) ) )
			{
				sprintf( buf, "Hospital records show that %s has had a minor injury recently.", victim->name );
				send_to_char( buf, ch );
				return;
			}
			if( victim->hit == victim->max_hit )
			{
				sprintf( buf, "There has been no recently medical history for %s", victim->name );
				send_to_char( buf, ch );
				return;
			}
			return;
		}

		if( chance < 50 )
		{
			switch( victim->main_ability )
			{
			case 0:
				sprintf( buf, "%s appears to have centered training on combat.", victim->name );
				break;
			case 1:
				sprintf( buf, "%s appears to have centered training on piloting ships.", victim->name );
				break;
			case 2:
				sprintf( buf, "%s appears to have centered training on engineering.", victim->name );
				break;
			case 3:
				sprintf( buf, "%s appears to have centered training on bounty hunting.", victim->name );
				break;
			case 4:
				sprintf( buf, "%s appears to have centered training on smuggling.", victim->name );
				break;
			case 5:
				sprintf( buf, "%s appears to have centered training on diplomacy.", victim->name );
				break;
			case 6:
				sprintf( buf, "%s appears to have centered training on leadership.", victim->name );
				break;
			case 7:
				sprintf( buf, "%s appears to have centered attention on studying the force.", victim->name );
				break;
			case 8:
				sprintf( buf, "%s has not centered training on anything, but seems to mix smuggling with piloting abilities.",
					victim->name );
				break;
			default:
				break;
			}
			send_to_char( buf, ch );
			return;
		}

		send_to_char( "You fail to gather information on that individual.", ch );
		return;
	}
	else
	{
		send_to_char( "You fail to gather information on that individual.", ch );
		return;
	}

}

CMDF( do_hack )
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	bool ch_comlink, victim_comlink;
	int schance;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 25 )
	{
		send_to_char( "&RYou need atleast 25 LP to use this skill!\r\n", ch );
		return;
	}


	argument = one_argument( argument, arg );

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	if( ch->in_room == get_room_index( 6 ) )
	{
		send_to_char( "&RYou just wait out your punishment!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Hack whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL
		|| ( IS_NPC( victim ) && victim->in_room != ch->in_room )
		|| ( !NOT_AUTHED( ch ) && NOT_AUTHED( victim ) && !IS_IMMORTAL( ch ) ) || ( !can_see( ch, victim ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	ch_comlink = false;
	victim_comlink = false;

	if( IS_IMMORTAL( ch ) )
	{
		ch_comlink = true;
		victim_comlink = true;
	}

	if( IS_IMMORTAL( victim ) )
		victim_comlink = true;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI to do that!\r\n", ch );
		return;
	}

	for( obj = victim->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			victim_comlink = true;
	}

	if( !victim_comlink )
	{
		send_to_char( "They don't seem to have a CDI!\r\n", ch );
		return;
	}

	if( ( victim->top_level - ch->top_level > 300 ) || IS_IMMORTAL( victim ) )
	{
		ch_printf( ch, "&R%s has to high of a security on their CDI to hack!\r\n", victim->name );
		ch_printf( victim, "&RCDI ALERT&r:&R %s just tried to hack you!\r\n", ch->name );
		return;
	}

	location = victim->in_room;
	if( !location )
		return;

	schance = ch->pcdata->learned[gsn_hack] - ( get_curr_lck( victim ) - 12 );
	if( number_percent( ) > schance )
		send_to_char( "&cYour CDI makes a soft humming noise...\r\n", victim );
	ch_printf( ch, "&cYou hack and locate %s from their CDI Information...\r\n", victim->name );
	original = ch->in_room;
	char_from_room( ch );
	char_to_room( ch, location );
	do_look( ch, "auto" );
	char_from_room( ch );
	char_to_room( ch, original );
	learn_from_limiter( ch, gsn_hack );
	return;
}

CMDF( do_phase )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	bool ch_comlink, victim_comlink;
	char arg[MAX_STRING_LENGTH];

	if( !IS_NPC( ch ) && ch->pcdata->lp < 50 )
	{
		send_to_char( "&RYou need atleast 50 LP to use this skill!\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	if( ch->in_room == get_room_index( 6 ) )
	{
		send_to_char( "&RYou just wait out your punishment!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Phase to whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL
		|| ( IS_NPC( victim ) && victim->in_room != ch->in_room )
		|| ( !NOT_AUTHED( ch ) && NOT_AUTHED( victim ) && !IS_IMMORTAL( ch ) ) || ( !can_see( ch, victim ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim->in_room == get_room_index( 6 ) )
	{
		send_to_char( "&RThey're helled right now. Do you really want to join them?\r\n", ch );
		return;
	}

	ch_comlink = false;
	victim_comlink = false;

	if( IS_IMMORTAL( ch ) )
	{
		ch_comlink = true;
		victim_comlink = true;
	}

	if( IS_IMMORTAL( victim ) )
		victim_comlink = true;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI to do that!\r\n", ch );
		return;
	}

	for( obj = victim->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			victim_comlink = true;
	}

	if( !victim_comlink )
	{
		send_to_char( "They don't seem to have a CDI!\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_ARENA )
		|| xIS_SET( victim->in_room->room_flags, ROOM_ARENA )
		|| xIS_SET( ch->in_room->room_flags, ROOM_HOTEL )
		|| xIS_SET( victim->in_room->room_flags, ROOM_HOTEL )
		|| xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT )
		|| xIS_SET( victim->in_room->room_flags, ROOM_SPACECRAFT ) )
	{
		send_to_char( "You failed!\r\n", ch );
		return;
	}

	if( ( victim->top_level - ch->top_level > 300 ) || IS_IMMORTAL( victim ) )
	{
		ch_printf( ch, "You can't seem to phase to %s!\r\n", victim->name );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_phase]->beats );

	if( !IS_NPC( ch ) && number_percent( ) > ch->pcdata->learned[gsn_phase] )
	{
		send_to_char( "You failed.\r\n", ch );
		learn_from_failure( ch, gsn_phase );
		return;
	}

	act( AT_YELLOW, "$n slowly phases out of sight.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You enter a program in your CDI and slowly phase to your victim.\r\n", ch );
	learn_from_limiter( ch, gsn_phase );
	room = victim->in_room;
	if( !room )
	{
		send_to_char( "Your victim seems to be stuck in the middle of nowhere... No sense leaving you\r\nthere, too.\r\n",
			ch );
		return;
	}
	char_from_room( ch );
	char_to_room( ch, room );
	act( AT_YELLOW, "$n slowly phases into sight.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	return;
}

CMDF( do_makecamera )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int level, chance;
	bool checktool, checkdura, checkbatt, checkoven;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;

	strcpy( arg, argument );

	if( !IS_NPC( ch ) && ch->pcdata->lp < 3 )
	{
		send_to_char( "&RYou need atleast 3 LP to use this skill!\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:

		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makecamera <name>\r\n&w", ch );
			return;
		}

		checktool = false;
		checkdura = false;
		checkbatt = false;
		checkoven = false;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_TOOLKIT )
				checktool = true;
			if( obj->item_type == ITEM_DURASTEEL )
				checkdura = true;
			if( obj->item_type == ITEM_BATTERY )
				checkbatt = true;

			if( obj->item_type == ITEM_OVEN )
				checkoven = true;
		}

		if( !checktool )
		{
			send_to_char( "&RYou need toolkit to make a Camera.\r\n", ch );
			return;
		}

		if( !checkdura )
		{
			send_to_char( "&RYou need something to make it out of.\r\n", ch );
			return;
		}

		if( !checkbatt )
		{
			send_to_char( "&RYou need a power source for your camera.\r\n", ch );
			return;
		}

		if( !checkoven )
		{
			send_to_char( "&RYou need a small furnace to heat the metal.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecamera] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the long process of creating a camera.\r\n", ch );
			act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
				NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 25, do_makecamera, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makecamera );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makecamera] );
	vnum = 10431;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char
		( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n",
			ch );
		return;
	}

	checktool = false;
	checkdura = false;
	checkbatt = false;
	checkoven = false;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->item_type == ITEM_TOOLKIT )
			checktool = true;
		if( obj->item_type == ITEM_OVEN )
			checkoven = true;
		if( obj->item_type == ITEM_DURASTEEL && checkdura == false )
		{
			checkdura = true;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}
		if( obj->item_type == ITEM_BATTERY && checkbatt == false )
		{
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
			checkbatt = true;
		}
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( 60 );

	if( number_percent( ) > chance * 2 || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) )
	{
		send_to_char( "&RYou activate your newly created camera.\r\n", ch );
		send_to_char( "&RIt focuses its lens...\r\n", ch );
		send_to_char( "&RIt suddenly shatters all over the floor.\r\n", ch );
		learn_from_failure( ch, gsn_makecamera );
		return;
	}

	obj = create_object( pObjIndex, level );

	obj->item_type = ITEM_REMOTE;
	SET_BIT( obj->wear_flags, ITEM_TAKE );
	obj->level = level;
	obj->weight = 3;
	STRFREE( obj->name );
	sprintf( buf, "%s %s", remand( arg ), ch->name );
	obj->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	strcat( buf, " was left here." );
	obj->description = STRALLOC( buf );
	obj->value[0] = 0;
	obj->value[1] = 0;
	obj->value[2] = 0;
	obj->value[3] = 0;
	obj->value[4] = 0;
	obj->value[5] = 0;
	obj->cost = 100;

	obj = obj_to_char( obj, ch );

	send_to_char( "&GYou finish your work and hold up your newly created camera!&w\r\n", ch );
	act( AT_PLAIN, "$n finishes creating a camera.", ch, NULL, argument, TO_ROOM );

	learn_from_limiter( ch, gsn_makecamera );
	return;
}

CMDF( do_lore )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if( !IS_NPC( ch ) && ch->pcdata->lp < 20 )
	{
		send_to_char( "&RYou need atleast 20 LP to use this skill!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Lore what?\r\n", ch );
		return;
	}
	if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
		strcpy( arg, argument );
	/*
		if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
		{
		send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
		return;
		}
	*/

	if( ms_find_obj( ch ) )
		return;

	if( ( obj = find_obj( ch, argument, true ) ) == NULL )
		return;

	ch_printf( ch, "\r\n&YName&O:&B %s       &YType&O:&B %s\r\n", obj->name, item_type_name( obj ) );

	send_to_char( "&O----------------------------------------------------\r\n", ch );

	ch_printf( ch, "&YShort description&O:&B %s\r\n&YLong description&O:&B %s\r\n", obj->short_descr, obj->description );

	ch_printf( ch, "&YWearloc(s)&O:&B %s\r\n", flag_string( obj->wear_flags, w_flags ) );

	ch_printf( ch, "&YCost&O:&B %s                &YWeight&O: &B%d&O/&B%d\r\n",
		num_punct( obj->cost ), obj->weight, get_obj_weight( obj ) );

	if( obj->item_type == ITEM_WEAPON )
	{
		ch_printf( ch, "&YAverage Dam&O:&P %-8d", obj->value[2] );
		if( obj->value[3] == WEAPON_BLASTER
			|| obj->value[3] == WEAPON_BOWCASTER )
			ch_printf( ch, "&YCharges&O: &P%d&O/&P%d", obj->value[4], obj->value[5] );
		send_to_char( "\r\n", ch );
	}

	if( obj->item_type == ITEM_ARMOR )
	{
		ch_printf( ch, "&YArmor Conditioner&O:&B %d&O/&B%d\r\n", obj->value[0], obj->value[1] );
	}

	for( paf = obj->first_affect; paf; paf = paf->next )
		ch_printf( ch, "&YAffect&O:&B %s &Oby&B %d\r\n", affect_loc_name( paf->location ), paf->modifier );

	for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		ch_printf( ch, "&YAffect&O:&B %s &Oby &B%d\r\n", affect_loc_name( paf->location ), paf->modifier );

	learn_from_limiter( ch, gsn_lore );
	return;
}


CMDF( do_sabotage )
{
	char arg[MAX_INPUT_LENGTH];
	int chance, change;
	SHIP_DATA *ship;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 18 )
	{
		send_to_char( "&RYou need atleast 18 LP to use this skill!\r\n", ch );
		return;
	}


	strcpy( arg, argument );

	switch( ch->substate )
	{
	default:
		if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in the engine room of a ship to do that!\r\n", ch );
			return;
		}

		if( str_cmp( argument, "thrusters" ) && str_cmp( argument, "torso" ) &&
			str_cmp( argument, "leftarm" ) && str_cmp( argument, "rightarm" ) && str_cmp( argument, "navigational" ) )
		{
			send_to_char( "&RYou can only sabotage one of the following:\r\n", ch );
			send_to_char( "&rTry: Thrusters, Torso, Leftarm, Rightarm, or Navigational.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_sabotage] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin your 'work'.\r\n", ch );
			act( AT_PLAIN, "$n begins 'working' on the ship's $T.", ch, NULL, argument, TO_ROOM );
			if( !str_cmp( arg, "hull" ) )
				add_timer( ch, TIMER_DO_FUN, 15, do_sabotage, 1 );
			else
				add_timer( ch, TIMER_DO_FUN, 15, do_sabotage, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou fail to figure out where to start.\r\n", ch );
		learn_from_failure( ch, gsn_sabotage );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
			return;
		send_to_char( "&RYou are distracted and fail to finish your 'work'.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
	{
		return;
	}


	if( !str_cmp( arg, "Torso" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_sabotage] ),
				( int ) ( ch->pcdata->learned[gsn_sabotage] ) ), ( ship->armor ) );
		ship->armor -= change;
		ch_printf( ch, "&GSabotage complete.. Torso armor damaged &g%d &Gpoints.\r\n", change );
	}

	if( !str_cmp( arg, "thrusters" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_sabotage] ),
				( int ) ( ch->pcdata->learned[gsn_sabotage] ) ), ( ship->armorlegs ) );
		ship->armorlegs -= change;
		ch_printf( ch, "&GSabotage complete.. Legs armor damaged &g%d &Gpoints.\r\n", change );
	}

	if( !str_cmp( arg, "leftarm" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_sabotage] ),
				( int ) ( ch->pcdata->learned[gsn_sabotage] ) ), ( ship->armorlarm ) );
		ship->armorlarm -= change;
		ch_printf( ch, "&GSabotage complete.. Left arm damaged by &g%d &Gpoints.\r\n", change );
	}

	if( !str_cmp( arg, "rightarm" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_sabotage] ),
				( int ) ( ch->pcdata->learned[gsn_sabotage] ) ), ( ship->armorrarm ) );
		ship->armorrarm -= change;
		ch_printf( ch, "&GSabotage complete.. Right arm damaged &g%d &Gpoints.\r\n", change );
	}

	if( !str_cmp( arg, "navigational" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_sabotage] ),
				( int ) ( ch->pcdata->learned[gsn_sabotage] ) ), ( ship->armorhead ) );
		ship->armorhead -= change;
		ch_printf( ch, "&GSabotage complete.. Navigation Systems in head damaged &g%d &Gpoints.\r\n", change );
	}

	act( AT_PLAIN, "$n finishes the work.", ch, NULL, argument, TO_ROOM );

	learn_from_limiter( ch, gsn_sabotage );
	return;
}


CMDF( do_codebreak )
{
	SHIP_DATA *ship;
	int chance;
	OBJ_DATA *obj;
	bool ch_comlink;
	int percent;
	percent = number_range( 1, 5 );

	if( IS_NPC( ch ) )
		return;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 45 )
	{
		send_to_char( "&RYou need atleast 45 LP to use this skill!\r\n", ch );
		return;
	}


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	ch_comlink = false;

	if( IS_IMMORTAL( ch ) )
	{
		ch_comlink = true;
	}

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI to do that!\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:


		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_codebreak] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin trying to decipher.\r\n", ch );
			act( AT_PLAIN, "$n takes $s CDI and tries to decipher the launch code.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_codebreak, 1 );
			return;
		}
		send_to_char( "&RYou can't figure out what to do.\r\n", ch );
		learn_from_failure( ch, gsn_codebreak );
		return;

	case 1:
		break;

	case SUB_TIMER_DO_ABORT:
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to decipher the code.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( percent == 1 )
	{
		ch_printf( ch, "&GCode narrowed down to&g, &G%d&g,&G %d&g,&G %d&g,&G %d&g,&G or %d&g.\r\n",
			ship->code - 1, ship->code - 2, ship->code, ship->code + 1, ship->code + 2 );
	}
	else if( percent == 2 )
	{
		ch_printf( ch, "&GCode narrowed down to&g, &G%d&g,&G %d&g,&G %d&g,&G %d&g,&G or %d&g.\r\n",
			ship->code - 1, ship->code, ship->code + 1, ship->code + 2, ship->code + 3 );
	}
	else if( percent == 3 )
	{
		ch_printf( ch, "&GCode narrowed down to&g, &G%d&g,&G %d&g,&G %d&g,&G %d&g,&G or %d&g.\r\n",
			ship->code, ship->code + 1, ship->code + 2, ship->code + 3, ship->code + 4 );
	}
	else if( percent == 4 )
	{
		send_to_char( "&RYou fail to decipher the code!\r\n", ch );
		return;
	}
	else if( percent == 5 )
	{
		send_to_char( "&RYou fail to decipher the code!\r\n", ch );
		return;
	}

	learn_from_limiter( ch, gsn_codebreak );
}

CMDF( do_slip )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( !IS_NPC( ch ) && ch->pcdata->lp < 25 )
	{
		send_to_char( "&RYou need atleast 25 LP to use this skill!\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Slip what into whom's pocket?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	WAIT_STATE( ch, skill_table[gsn_slip]->beats );

	if( !IS_NPC( ch ) && number_percent( ) > ch->pcdata->learned[gsn_slip] )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( is_number( arg1 ) )
	{
		/*
		 * 'give NNNN coins victim'
		 */
		int amount;

		amount = atoi( arg1 );
		if( amount <= 0 || ( str_cmp( arg2, "dollars" ) && str_cmp( arg2, "dollar" ) ) )
		{
			send_to_char( "Sorry, you can't do that.\r\n", ch );
			return;
		}

		argument = one_argument( argument, arg2 );
		if( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
			argument = one_argument( argument, arg2 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Slip what into whom's Pocket?\r\n", ch );
			return;
		}

		if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if( ch->gold < amount )
		{
			send_to_char( "You don't have that much to give!\r\n", ch );
			return;
		}

		ch->gold -= amount;
		victim->gold += amount;
		strcpy( buf, "$n gives you " );
		strcat( buf, arg1 );
		strcat( buf, ( amount > 1 ) ? " dollars." : " dollar." );

		act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "You slip some money into $N's pocket.", ch, NULL, victim, TO_CHAR );
		send_to_char( "OK.\r\n", ch );
		mprog_bribe_trigger( victim, ch, amount );
		if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
			save_char_obj( ch );
		if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
			save_char_obj( victim );
		learn_from_limiter( ch, gsn_slip );
		return;
	}

	if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	}

	if( obj->wear_loc != WEAR_NONE )
	{
		send_to_char( "You must remove it first.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( !can_drop_obj( ch, obj ) )
	{
		send_to_char( "You can't let go of it.\r\n", ch );
		return;
	}

	if( victim->carry_number + ( get_obj_number( obj ) / obj->count ) > can_carry_n( victim ) )
	{
		act( AT_PLAIN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( victim->carry_weight + ( get_obj_weight( obj ) / obj->count ) > can_carry_w( victim ) )
	{
		act( AT_PLAIN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
	{
		act( AT_PLAIN, "You cannot slip that into $N's inventory!", ch, NULL, victim, TO_CHAR );
		return;
	}

	separate_obj( obj );
	obj_from_char( obj );
	act( AT_ACTION, "You carefully slip $p&w into $N's inventory.", ch, obj, victim, TO_CHAR );
	learn_from_limiter( ch, gsn_slip );
	obj = obj_to_char( obj, victim );

	mprog_give_trigger( victim, ch, obj );
	if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
		save_char_obj( ch );
	if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
		save_char_obj( victim );
	return;
}

CMDF( do_fry )
{
	SHIP_DATA *ship;
	int chance;
	OBJ_DATA *obj;
	bool ch_comlink;

	if( IS_NPC( ch ) )
		return;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 49 )
	{
		send_to_char( "&RYou need atleast 49 LP to use this skill!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	ch_comlink = false;

	if( IS_IMMORTAL( ch ) )
	{
		ch_comlink = true;
	}

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI to do that!\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:


		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_fry] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin trying to fry the launch circuits.\r\n", ch );
			act( AT_PLAIN, "$n takes $s CDI and tries fry the launch circuits.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_fry, 1 );
			return;
		}
		send_to_char( "&RYou can't figure out what to do.\r\n", ch );
		learn_from_failure( ch, gsn_fry );
		return;

	case 1:
		break;

	case SUB_TIMER_DO_ABORT:
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to fry the launch circuits.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ship->autoboom == 1 )
	{
		send_to_char( "You only now just figured out the circuits have already been fried...\r\n", ch );
		return;
	}

	ch_printf( ch, "Circuits fried! Lets just hope %s goes boom!.\r\n", ship->name );
	ship->autoboom = 1;
	learn_from_limiter( ch, gsn_fry );
	return;
}

CMDF( do_meddle )
{
	DESCRIPTOR_DATA *d;
	bool checkdata;
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int chance;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 10 )
	{
		send_to_char( "&RYou need atleast 10 LP to use this skill!\r\n", ch );
		return;
	}

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );
	checkdata = false;
	switch( ch->substate )
	{
	default:

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_COMLINK )
				checkdata = true;
		}

		if( ( checkdata == false ) )
		{
			send_to_char( "You need a CDI to hack into the bank files.\r\n", ch );
			return;
		}
		if( !xIS_SET( ch->in_room->room_flags, ROOM_BANK ) )
		{
			send_to_char( "You must be in a bank.\r\n", ch );
			return;
		}
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_meddle] );
		if( number_percent( ) < chance )
		{

			send_to_char( "&GYou start trying to hack into the bank files.\r\n", ch );
			sprintf( buf, "$n takes $s CDI and begins doing something." );
			act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_meddle, 1 );
			return;

		}
		send_to_char( "&RYou are unsuccessful at hacking into the bank system.\r\n", ch );
		learn_from_failure( ch, gsn_meddle );
		return;

	case 1:
		break;
	case SUB_TIMER_DO_ABORT:
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interrupted and fail to hack into the bank system\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_meddle] );

	if( number_percent( ) > chance * 2 )
	{
		ch_printf( ch,
			"&W=====================&BW&borld &BB&banking &BS&bystem&W=====================\r\n"
			"\r\n&YAuthorized Users only. All others subject to penalty of law.\r\n\r\n"
			"\r\n&zLogin&W:&w %d\r\n"
			"&zPassword&W:&w **************\r\n"
			"\r\n\r\n"
			"&RIncorrect password! Location being traced!!!\r\n"
			"&W==============================================================\r\n\r\n\n&RDisconnecting..",
			number_range( 10000, 99999 ) );
		learn_from_failure( ch, gsn_meddle );
		return;
	}

	ch_printf( ch,
		"&W=====================&BW&borld &BB&banking &BS&bystem&W=====================\r\n"
		"\r\n&YAuthorized Users only. All others subject to penalty of law.\r\n\r\n"
		"\r\n&zLogin&W:&w %d\r\n"
		"&zPassword&W:&w **************\r\n"
		"\r\n\r\n"
		"&BLogin and Password authorized, scanning data.\r\n"
		"&W==============================================================\r\n\r\n", number_range( 10000, 99999 ) );

	for( d = first_descriptor; d; d = d->next )
	{
		if( !d->character )
			continue;
		if( d->connected != CON_PLAYING )
			continue;
		if( IS_IMMORTAL( d->character ) )
			continue;
		ch_printf( ch, "&z%-15s             &Y%-9s\r\n", d->character->name, num_punct( d->character->pcdata->bank ) );
	}
	ch_printf( ch, "&W==============================================================\r\n" );

	learn_from_limiter( ch, gsn_meddle );
	return;
}
