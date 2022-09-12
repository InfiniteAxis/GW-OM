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
#include "bet.h"

/*double sqrt( double x );*/

/*
 * External functions
 */

 /*
  * Local functions.
  */
void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container );
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace );
bool has_artifact( CHAR_DATA *ch );
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, short wear_bit );
void write_corpses( CHAR_DATA *ch, const char *name );

/*
 * how resistant an object is to damage				-Thoric
 */
short get_obj_resistance( OBJ_DATA *obj )
{
	short resist;

	resist = number_fuzzy( MAX_ITEM_IMPACT );

	/*
	 * magical items are more resistant
	 */
	if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
		resist += number_fuzzy( 12 );
	/*
	 * blessed objects should have a little bonus
	 */
	if( IS_OBJ_STAT( obj, ITEM_BLESS ) )
		resist += number_fuzzy( 5 );
	/*
	 * lets make store inventory pretty tough
	 */
	if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
		resist += 20;

	/*
	 * okay... let's add some bonus/penalty for item level...
	 */
	resist += ( obj->level / 10 );

	/*
	 * and lasty... take armor or weapon's condition into consideration
	 */
	if( obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON )
		resist += ( obj->value[0] );

	return URANGE( 10, resist, 99 );
}

bool has_artifact( CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )   //Found the artifact
			return true;
	}
	return false;
}

bool can_loot( CHAR_DATA *ch, OBJ_DATA *obj )
{
	CHAR_DATA *owner, *wch, *killer;

	if( IS_IMMORTAL( ch ) )
		return true;

	if( !obj->owner || obj->owner == NULL )
		return true;

	owner = NULL;
	for( wch = first_char; wch != NULL; wch = wch->next )
		if( !str_cmp( wch->name, obj->owner ) )
			owner = wch;

	killer = NULL;
	if( obj->killer || obj->killer != NULL )
		for( wch = first_char; wch != NULL; wch = wch->next )
			if( !str_cmp( wch->name, obj->killer ) )
				killer = wch;

	if( owner == NULL )
		return true;

	if( !str_cmp( ch->name, owner->name ) )
		return true;

	if( killer != NULL )
		if( !str_cmp( ch->name, killer->name ) )
			return true;

	if( !IS_NPC( owner ) && IS_SET( owner->pcdata->flags, PCFLAG_CANLOOT ) )
		return true;

	if( is_same_group( ch, owner ) )
		return true;

	return false;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
	char buf[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	int weight;

	if( !CAN_WEAR( obj, ITEM_TAKE ) && ( ch->top_level < sysdata.level_getobjnotake ) )
	{
		send_to_char( "You can't take that.\r\n", ch );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
	{
		send_to_char( "A godly force prevents you from getting close to it.\r\n", ch );
		return;
	}

	if( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
		act( AT_PLAIN, "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
		weight = obj->weight;
	else
		weight = get_obj_weight( obj );

	/* Money weight shouldn't count */
	if( obj->item_type != ITEM_MONEY )
	{
		if( obj->in_obj )
		{
			OBJ_DATA *tobj = obj->in_obj;
			int inobj = 1;
			bool checkweight = false;

			/* need to make it check weight if its in a magic container */
			if( tobj->item_type == ITEM_CONTAINER && IS_OBJ_STAT( tobj, ITEM_MAGIC ) )
				checkweight = true;

			while( tobj->in_obj )
			{
				tobj = tobj->in_obj;
				inobj++;

				/* need to make it check weight if its in a magic container */
				if( tobj->item_type == ITEM_CONTAINER && IS_OBJ_STAT( tobj, ITEM_MAGIC ) )
					checkweight = true;
			}

			/* need to check weight if not carried by ch or in a magic container. */
			if( !tobj->carried_by || tobj->carried_by != ch || checkweight )
			{
				if( ( ch->carry_weight + weight ) > can_carry_w( ch ) )
				{
					act( AT_PLAIN, "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
					return;
				}
			}
		}
		else if( ( ch->carry_weight + weight ) > can_carry_w( ch ) )
		{
			act( AT_PLAIN, "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
			return;
		}
	}

	if( !xIS_SET( ch->act, PLR_PKER ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
	{
		send_to_char( "If you want to use artifacts, go PK!\r\n", ch );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) && IS_NPC( ch ) )
	{
		send_to_char( "You can't pick it up.\r\n", ch );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) && has_artifact( ch ) )
	{
		send_to_char( "Drop the artifact you have before picking up another.\r\n", ch );
		return;
	}
	if( container )
	{
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ?
			"&wYou get $p&w from beneath $P." : "&wYou get $p&w from $P", ch, obj, container, TO_CHAR );
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ?
			"&w$n gets $p&w from beneath $P." : "&w$n gets $p&w from $P", ch, obj, container, TO_ROOM );
		obj_from_obj( obj );
	}
	else
	{
		act( AT_ACTION, "&wYou get $p&w.", ch, obj, container, TO_CHAR );
		act( AT_ACTION, "&w$n gets $p&w.", ch, obj, container, TO_ROOM );

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			save_artifacts( );
			sprintf( buf, "Picked up %s.", obj->name );
			append_file( ch, ARTI_FILE, buf );
		}

		obj_from_room( obj );
	}

	/*
	 * Clan storeroom checks
	 */
	if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) && ( !container || container->carried_by == NULL ) )
		for( clan = first_clan; clan; clan = clan->next )
			if( clan->storeroom == ch->in_room->vnum )
				save_clan_storeroom( ch, clan );

	if( obj->item_type != ITEM_CONTAINER )
		check_for_trap( ch, obj, TRAP_GET );
	if( char_died( ch ) )
		return;

	if( obj->item_type == ITEM_MONEY )
	{
		ch->gold += obj->value[0] * obj->count;
		extract_obj( obj );
	}
	else
	{
		obj = obj_to_char( obj, ch );
	}

	if( char_died( ch ) || obj_extracted( obj ) )
		return;
	oprog_get_trigger( ch, obj );
	return;
}


CMDF( do_get )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *container;
	short number;
	bool found;

	if( xIS_SET( ch->in_room->room_flags, ROOM_PLR_HOME ) && ch->plr_home != ch->in_room && !IS_IMMORTAL( ch ) )
	{
		send_to_char( "Don't take things from peoples houses!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	if( is_number( arg1 ) )
	{
		number = atoi( arg1 );
		if( number < 1 )
		{
			send_to_char( "That was easy...\r\n", ch );
			return;
		}
		if( ( ch->carry_number + number ) > can_carry_n( ch ) )
		{
			send_to_char( "You can't carry that many.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg1 );
	}
	else
		number = 0;
	argument = one_argument( argument, arg2 );
	/*
	 * munch optional words
	 */
	if( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	/*
	 * Get type.
	 */
	if( arg1[0] == '\0' )
	{
		send_to_char( "Get what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( arg2[0] == '\0' )
	{
		if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			/*
			 * 'get obj'
			 */
			obj = get_obj_list( ch, arg1, ch->in_room->first_content );
			if( !obj )
			{
				act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
				return;
			}
			separate_obj( obj );

			get_obj( ch, obj, NULL );
			if( char_died( ch ) )
				return;
			if( IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );

		}
		else
		{
			short cnt = 0;
			bool fAll;
			char *chk;

			if( xIS_SET( ch->in_room->room_flags, ROOM_DONATION ) )
			{
				send_to_char( "The gods frown upon such a display of greed!\r\n", ch );
				return;
			}
			if( !str_cmp( arg1, "all" ) )
				fAll = true;
			else
				fAll = false;
			if( number > 1 )
				chk = arg1;
			else
				chk = &arg1[4];
			/*
			 * 'get all' or 'get all.obj'
			 */
			found = false;
			for( obj = ch->in_room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if( ( fAll || nifty_is_name( chk, obj->name ) ) && can_see_obj( ch, obj ) )
				{
					found = true;
					if( number && ( cnt + obj->count ) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					get_obj( ch, obj, NULL );
					if( char_died( ch )
						|| ch->carry_number >= can_carry_n( ch )
						|| ch->carry_weight >= can_carry_w( ch ) || ( number && cnt >= number ) )
					{
						if( IS_SET( sysdata.save_flags, SV_GET ) && !char_died( ch ) )
							save_char_obj( ch );
						return;

					}
				}
			}

			if( !found )
			{
				if( fAll )
					send_to_char( "I see nothing here.\r\n", ch );
				else
					act( AT_PLAIN, "I see no $T here.", ch, NULL, chk, TO_CHAR );
			}
			else if( IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
			save_artifacts( );
		}
	}
	else
	{
		/*
		 * 'get ... container'
		 */
		if( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}

		if( ( container = get_obj_here( ch, arg2 ) ) == NULL )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
			return;
		}

		switch( container->item_type )
		{
		default:
			if( !IS_OBJ_STAT( container, ITEM_COVERING ) )
			{
				send_to_char( "That's not a container.\r\n", ch );
				return;
			}
			if( ch->carry_weight + container->weight > can_carry_w( ch ) )
			{
				send_to_char( "It's too heavy for you to lift.\r\n", ch );
				return;
			}
			break;

		case ITEM_CONTAINER:
		case ITEM_WHOLDER:
		case ITEM_DROID_CORPSE:
			//  case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
			break;

		case ITEM_CORPSE_PC:
		{

			if( !can_loot( ch, container ) )
			{
				send_to_char( "You can't do that.\r\n", ch );
				return;
			}
		}
		}


		if( container->item_type == ITEM_CONTAINER )
		{
			if( !IS_OBJ_STAT( container, ITEM_COVERING ) && IS_SET( container->value[1], CONT_CLOSED ) )
			{
				act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
				return;
			}
		}

		if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			/*
			 * 'get obj container'
			 */
			obj = get_obj_list( ch, arg1, container->first_content );
			if( !obj )
			{
				act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
					"I see nothing like that beneath the $T." : "I see nothing like that in the $T.", ch, NULL, container->short_descr, TO_CHAR );
				return;
			}
			separate_obj( obj );
			get_obj( ch, obj, container );

			if( container->item_type == ITEM_CORPSE_PC )
				write_corpses( NULL, container->short_descr + 14 );

			check_for_trap( ch, container, TRAP_GET );
			if( char_died( ch ) )
				return;
			if( IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
		}
		else
		{
			int cnt = 0;
			bool fAll;
			char *chk;

			/*
			 * 'get all container' or 'get all.obj container'
			 */
			if( IS_OBJ_STAT( container, ITEM_DONATION ) )
			{
				send_to_char( "The gods frown upon such an act of greed!\r\n", ch );
				return;
			}
			if( !str_cmp( arg1, "all" ) )
				fAll = true;
			else
				fAll = false;
			if( number > 1 )
				chk = arg1;
			else
				chk = &arg1[4];
			found = false;
			for( obj = container->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if( ( fAll || nifty_is_name( chk, obj->name ) ) && can_see_obj( ch, obj ) )
				{
					found = true;
					if( number && ( cnt + obj->count ) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					get_obj( ch, obj, container );
					if( char_died( ch )
						|| ch->carry_number >= can_carry_n( ch )
						|| ch->carry_weight >= can_carry_w( ch ) || ( number && cnt >= number ) )
					{
						if( container->item_type == ITEM_CORPSE_PC )
							write_corpses( NULL, container->short_descr + 14 );
						if( found && IS_SET( sysdata.save_flags, SV_GET ) );
						save_char_obj( ch );
						return;
					}
				}
			}

			if( !found )
			{
				if( fAll )
					act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
						"I see nothing beneath the $T." : "I see nothing in the $T.", ch, NULL, container->short_descr, TO_CHAR );
				else
					act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
						"I see nothing like that beneath the $T." :
						"I see nothing like that in the $T.", ch, NULL, container->short_descr, TO_CHAR );
			}
			else
				check_for_trap( ch, container, TRAP_GET );
			if( char_died( ch ) )
				return;
			if( found && IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
		}
	}
	return;
}

void oprog_put_trigger( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *tobj );

CMDF( do_put )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *container;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *sheath;
	CLAN_DATA *clan;
	short count;
	int number;
	bool save_char = false;

	argument = one_argument( argument, arg1 );
	if( is_number( arg1 ) )
	{
		number = atoi( arg1 );
		if( number < 1 )
		{
			send_to_char( "That was easy...\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg1 );
	}
	else
		number = 0;
	argument = one_argument( argument, arg2 );
	/*
	 * munch optional words
	 */
	if( ( !str_cmp( arg2, "into" ) || !str_cmp( arg2, "inside" ) || !str_cmp( arg2, "in" ) ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Put what in what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
		return;
	}

	if( !container->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
		save_char = true;

	if( IS_OBJ_STAT( container, ITEM_COVERING ) )
	{
		if( ch->carry_weight + container->weight > can_carry_w( ch ) )
		{
			send_to_char( "It's too heavy for you to lift.\r\n", ch );
			return;
		}
	}
	else
	{
		if( container->item_type != ITEM_CONTAINER && container->item_type != ITEM_WHOLDER )
		{
			send_to_char( "That's not a container.\r\n", ch );
			return;
		}

		if( container->item_type == ITEM_CONTAINER )
		{
			if( IS_SET( container->value[1], CONT_CLOSED ) )
			{
				act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
				return;
			}
		}
	}

	if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
		/*
		 * 'put obj container'
		 */
		if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\r\n", ch );
			return;
		}

		if( obj == container )
		{
			send_to_char( "You can't fold it into itself.\r\n", ch );
			return;
		}

		if( !can_drop_obj( ch, obj ) )
		{
			send_to_char( "You can't let go of it.\r\n", ch );
			return;
		}

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			send_to_char( "You can't put an artifact in a container!\r\n", ch );
			return;
		}

		if( container->item_type == ITEM_WHOLDER && ( container->value[1] != obj->value[3] ) )
		{
			send_to_char( "You can only put a weapon made for the sheath in the sheath.\r\n", ch );
			return;
		}
		if( container->item_type == ITEM_WHOLDER
			&& ( ( container->value[2] != obj->pIndexData->vnum ) && container->value[2] != 0 ) )
		{
			send_to_char( "Your sheath was made for only one weapon and the weapon you have is not it.\r\n", ch );
			return;
		}

		if( ( IS_OBJ_STAT( container, ITEM_COVERING )
			&& ( get_obj_weight( obj ) / obj->count )
			> ( ( get_obj_weight( container ) / container->count ) - container->weight ) ) )
		{
			send_to_char( "It won't fit under there.\r\n", ch );
			return;
		}

		if( ( get_obj_weight( obj ) / obj->count ) + ( get_obj_weight( container ) / container->count ) > container->value[0] )
		{
			send_to_char( "It won't fit.\r\n", ch );
			return;
		}

		if( container->item_type == ITEM_WHOLDER )
		{
			count = 0;
			if( container->first_content )
			{
				for( sheath = container->first_content; sheath; sheath = sheath->next_content )
					count++;
			}

			if( count == 1 )
			{
				send_to_char( "Sheath's only fit one weapon, remove the one in it first.\r\n", ch );
				return;
			}
		}

		separate_obj( obj );
		separate_obj( container );
		obj_from_char( obj );
		obj = obj_to_obj( obj, container );
		check_for_trap( ch, container, TRAP_PUT );
		if( char_died( ch ) )
			return;
		count = obj->count;
		obj->count = 1;
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
			? "&w$n hides $p&w beneath $P&w." : "&w$n puts $p&w in $P&w.", ch, obj, container, TO_ROOM );
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
			? "&wYou hide $p&w beneath $P&w." : "&wYou put $p&w in $P&w.", ch, obj, container, TO_CHAR );
		obj->count = count;

		oprog_put_trigger( ch, container, obj );

		if( save_char )
			save_char_obj( ch );
		save_artifacts( );
		/*
		 * Clan storeroom check
		 */
		if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) && container->carried_by == NULL )
			for( clan = first_clan; clan; clan = clan->next )
				if( clan->storeroom == ch->in_room->vnum )
					save_clan_storeroom( ch, clan );
	}
	else
	{
		bool found = false;
		int cnt = 0;
		bool fAll;
		char *chk;

		if( container->item_type == ITEM_WHOLDER )
		{
			send_to_char( "You cannot put all your items in a sheath, only one weapon.\r\n", ch );
			return;
		}

		if( !str_cmp( arg1, "all" ) )
			fAll = true;
		else
			fAll = false;
		if( number > 1 )
			chk = arg1;
		else
			chk = &arg1[4];

		separate_obj( container );
		/*
		 * 'put all container' or 'put all.obj container'
		 */
		for( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;

			if( ( fAll || nifty_is_name( chk, obj->name ) )
				&& can_see_obj( ch, obj )
				&& obj->wear_loc == WEAR_NONE
				&& obj != container
				&& can_drop_obj( ch, obj ) && get_obj_weight( obj ) + get_obj_weight( container ) <= container->value[0] )
			{
				if( number && ( cnt + obj->count ) > number )
					split_obj( obj, number - cnt );
				cnt += obj->count;

				if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
					continue;

				obj_from_char( obj );
				act( AT_ACTION, "&w$n puts $p&w in $P&w.", ch, obj, container, TO_ROOM );
				act( AT_ACTION, "&wYou put $p&w in $P&w.", ch, obj, container, TO_CHAR );
				obj = obj_to_obj( obj, container );
				found = true;

				check_for_trap( ch, container, TRAP_PUT );
				oprog_put_trigger( ch, container, obj );
				if( char_died( ch ) )
					return;
				if( number && cnt >= number )
					break;
			}
		}

		/*
		 * Don't bother to save anything if nothing was dropped   -Thoric
		 */
		if( !found )
		{
			if( fAll )
				act( AT_PLAIN, "You are not carrying anything.", ch, NULL, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "You are not carrying any $T.", ch, NULL, chk, TO_CHAR );
			return;
		}

		if( save_char )
			save_char_obj( ch );
		if( container->item_type == ITEM_CORPSE_PC )
		{
			write_corpses( NULL, container->short_descr + 14 );
		}

		save_artifacts( );
		/*
		 * Clan storeroom check
		 */
		if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) && container->carried_by == NULL )
			for( clan = first_clan; clan; clan = clan->next )
				if( clan->storeroom == ch->in_room->vnum )
					save_clan_storeroom( ch, clan );
	}

	return;
}


CMDF( do_drop )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	bool found;
	CLAN_DATA *clan;
	int number;

	argument = one_argument( argument, arg );
	if( is_number( arg ) )
	{
		number = atoi( arg );
		if( number < 1 )
		{
			send_to_char( "That was easy...\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg );
	}
	else
		number = 0;

	if( arg[0] == '\0' )
	{
		send_to_char( "Drop what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( xIS_SET( ch->in_room->room_flags, ROOM_NODROP ) || ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LITTERBUG ) ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "A magical force stops you!\r\n", ch );
		set_char_color( AT_TELL, ch );
		send_to_char( "Someone tells you, 'No littering here!'\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_PLR_HOME ) && ch->plr_home != ch->in_room && !IS_IMMORTAL( ch ) )
	{
		send_to_char( "Don't make a mess out of other peoples houses!\r\n", ch );
		return;
	}

	if( number > 0 )
	{
		/*
		 * 'drop NNNN coins'
		 */

		if( !str_cmp( arg, "dollars" ) || !str_cmp( arg, "dollar" ) )
		{
			if( ch->gold < number )
			{
				send_to_char( "You haven't got that much money.\r\n", ch );
				return;
			}

			ch->gold -= number;

			for( obj = ch->in_room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;

				switch( obj->pIndexData->vnum )
				{
				case OBJ_VNUM_MONEY_ONE:
					number += 1;
					extract_obj( obj );
					break;

				case OBJ_VNUM_MONEY_SOME:
					number += obj->value[0];
					extract_obj( obj );
					break;
				}
			}

			act( AT_ACTION, "&w$n drops some money.", ch, NULL, NULL, TO_ROOM );
			obj_to_room( create_money( number ), ch->in_room );
			send_to_char( "OK.\r\n", ch );
			if( IS_SET( sysdata.save_flags, SV_DROP ) )
				save_char_obj( ch );
			return;
		}
	}

	if( number <= 1 && str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
	{
		/*
		 * 'drop obj'
		 */
		if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			send_to_char( "You do not have that item.\r\n", ch );
			return;
		}

		if( !can_drop_obj( ch, obj ) )
		{
			send_to_char( "You can't let go of it.\r\n", ch );
			return;
		}

		separate_obj( obj );

		if( ( xIS_SET( ch->in_room->room_flags, ROOM_PLR_HOME ) || xIS_SET( ch->in_room->room_flags, ROOM_EMPTY_HOME ) ) )
		{
			if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
			{
				send_to_char( "You can't drop that in here!\r\n", ch );
				return;
			}
		}

		act( AT_ACTION, "&w$n &wdrops $p&w.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "&wYou drop $p&w.", ch, obj, NULL, TO_CHAR );

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			save_artifacts( );
			sprintf( buf, "Dropped %s.", obj->name );
			append_file( ch, ARTI_FILE, buf );
		}

		obj_from_char( obj );
		obj = obj_to_room( obj, ch->in_room );
		oprog_drop_trigger( ch, obj );    /* mudprogs */

		if( char_died( ch ) || obj_extracted( obj ) )
			return;

		if( obj->item_type == ITEM_CORPSE_PC )
			write_corpses( NULL, obj->short_descr + 14 );

		/*
		 * Clan storeroom saving
		 */
		if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
			for( clan = first_clan; clan; clan = clan->next )
				if( clan->storeroom == ch->in_room->vnum )
					save_clan_storeroom( ch, clan );
	}
	else
	{
		int cnt = 0;
		char *chk;
		bool fAll;

		if( !str_cmp( arg, "all" ) )
			fAll = true;
		else
			fAll = false;
		if( number > 1 )
			chk = arg;
		else
			chk = &arg[4];
		/*
		 * 'drop all' or 'drop all.obj'
		 */
		if( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
		{
			send_to_char( "You can't seem to do that here...\r\n", ch );
			return;
		}
		found = false;
		for( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;

			if( ( xIS_SET( ch->in_room->room_flags, ROOM_PLR_HOME ) || xIS_SET( ch->in_room->room_flags, ROOM_EMPTY_HOME ) ) )
			{
				if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
				{
					send_to_char( "You can't drop that in here!\r\n", ch );
					continue;
				}
			}


			if( ( fAll || nifty_is_name( chk, obj->name ) )
				&& can_see_obj( ch, obj ) && obj->wear_loc == WEAR_NONE && can_drop_obj( ch, obj ) )
			{
				found = true;
				if( HAS_PROG( obj->pIndexData, DROP_PROG ) && obj->count > 1 )
				{
					++cnt;
					separate_obj( obj );
					obj_from_char( obj );
					if( !obj_next )
						obj_next = ch->first_carrying;
				}
				else
				{
					if( number && ( cnt + obj->count ) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					obj_from_char( obj );
				}
				act( AT_ACTION, "&w$n &wdrops $p&w.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou drop $p&w.", ch, obj, NULL, TO_CHAR );
				obj = obj_to_room( obj, ch->in_room );
				oprog_drop_trigger( ch, obj );  /* mudprogs */

				if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
				{
					save_artifacts( );
					sprintf( buf, "Dropped %s.", obj->name );
					append_file( ch, ARTI_FILE, buf );
				}

				if( char_died( ch ) )
					return;
				if( number && cnt >= number )
					break;
			}
		}

		if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
			for( clan = first_clan; clan; clan = clan->next )
				if( clan->storeroom == ch->in_room->vnum )
					save_clan_storeroom( ch, clan );

		if( !found )
		{
			if( fAll )
				act( AT_PLAIN, "You are not carrying anything.", ch, NULL, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "You are not carrying any $T.", ch, NULL, chk, TO_CHAR );
		}
	}
	if( IS_SET( sysdata.save_flags, SV_DROP ) )
		save_char_obj( ch );  /* duping protector */
	return;
}



CMDF( do_give )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Give what to whom?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

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
			send_to_char( "Give what to whom?\r\n", ch );
			return;
		}

		if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if( ch->gold < amount )
		{
			send_to_char( "Very generous of you, but you haven't got that much money.\r\n", ch );
			return;
		}

		ch->gold -= amount;
		victim->gold += amount;
		mudstrlcpy( buf, "$n gives you ", MAX_STRING_LENGTH );
		mudstrlcat( buf, arg1, MAX_STRING_LENGTH );
		mudstrlcat( buf, ( amount > 1 ) ? " dollars." : " dollar.", MAX_STRING_LENGTH );

		act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "&w$n gives $N some money.", ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "&wYou give $N some money.", ch, NULL, victim, TO_CHAR );
		send_to_char( "OK.\r\n", ch );
		mprog_bribe_trigger( victim, ch, amount );
		if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
			save_char_obj( ch );
		save_artifacts( );
		if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
			save_char_obj( victim );
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

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
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

	if( !can_see_obj( victim, obj ) )
	{
		act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
	{
		act( AT_PLAIN, "You cannot give that to $N!", ch, NULL, victim, TO_CHAR );
		return;
	}

	separate_obj( obj );
	obj_from_char( obj );
	act( AT_ACTION, "&w$n gives $p&w to $N.", ch, obj, victim, TO_NOTVICT );
	act( AT_ACTION, "&w$n gives you $p&w.", ch, obj, victim, TO_VICT );
	act( AT_ACTION, "&wYou give $p&w to $N.", ch, obj, victim, TO_CHAR );
	obj = obj_to_char( obj, victim );

	mprog_give_trigger( victim, ch, obj );
	if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
		save_char_obj( ch );
	save_artifacts( );
	if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
		save_char_obj( victim );

	return;
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj( OBJ_DATA *obj )
{
	CHAR_DATA *ch;
	CHAR_DATA *victim;
	obj_ret objcode;

	ch = obj->carried_by;
	objcode = rNONE;

	separate_obj( obj );

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
		return objcode;

	if( ( victim = who_fighting( ch ) ) != NULL )
	{
		if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_PPKER ) )
		{
			if( !IS_NPC( victim ) || ( IS_NPC( victim ) && victim->pIndexData->vnum != 2030 ) )
				return objcode;
		}
	}
	else if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_PPKER ) )
		return objcode;

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		return objcode;

	if( ch )
		act( AT_OBJECT, "($p&w gets damaged)", ch, obj, NULL, TO_CHAR );
	else if( obj->in_room && ( ch = obj->in_room->first_person ) != NULL )
	{
		act( AT_OBJECT, "($p&w gets damaged)", ch, obj, NULL, TO_ROOM );
		act( AT_OBJECT, "($p&w gets damaged)", ch, obj, NULL, TO_CHAR );
		ch = NULL;
	}

	oprog_damage_trigger( ch, obj );
	if( obj_extracted( obj ) )
		return global_objcode;

	switch( obj->item_type )
	{
	default:
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
		break;
	case ITEM_CONTAINER:
	case ITEM_WHOLDER:
		if( --obj->value[3] <= 0 )
		{
			make_scraps( obj );
			objcode = rOBJ_SCRAPPED;
		}
		break;
	case ITEM_ARMOR:
		if( ch && obj->value[0] >= 1 )
			ch->armor += apply_ac( obj, obj->wear_loc );
		if( --obj->value[0] <= 0 )
		{
			make_scraps( obj );
			objcode = rOBJ_SCRAPPED;
		}
		else if( ch && obj->value[0] >= 1 )
			ch->armor -= apply_ac( obj, obj->wear_loc );
		break;
	case ITEM_WEAPON:
		if( --obj->value[0] <= 0 )
		{
			make_scraps( obj );
			objcode = rOBJ_SCRAPPED;
		}
		break;
	}
	if( ch != NULL )
		save_char_obj( ch );  /* Stop scrap duping - Samson 1-2-00 */

	return objcode;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
	OBJ_DATA *obj, *tmpobj;

	if( ( obj = get_eq_char( ch, iWear ) ) == NULL )
		return true;

	if( !fReplace && ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
		act( AT_PLAIN, "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
		return false;
	}

	if( !fReplace )
		return false;

	if( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
	{
		act( AT_PLAIN, "You can't remove $p&w.", ch, obj, NULL, TO_CHAR );
		return false;
	}

	if( obj == get_eq_char( ch, WEAR_WIELD ) && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
		tmpobj->wear_loc = WEAR_WIELD;

	unequip_char( ch, obj );

	act( AT_ACTION, "&w$n stops using $p&w.", ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "&wYou stop using $p&w.", ch, obj, NULL, TO_CHAR );
	oprog_remove_trigger( ch, obj );
	return true;
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual( CHAR_DATA *ch )
{
	if( IS_NPC( ch ) )
		return true;
	if( ch->pcdata->learned[gsn_dual_wield] )
		return true;

	return false;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual( CHAR_DATA *ch )
{
	if( !could_dual( ch ) )
		return false;

	if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
	{
		send_to_char( "You are already wielding two weapons!\r\n", ch );
		return false;
	}
	if( get_eq_char( ch, WEAR_HOLD ) )
	{
		send_to_char( "You cannot dual wield while holding something!\r\n", ch );
		return false;
	}
	return true;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer( CHAR_DATA *ch, OBJ_DATA *obj, short wear_loc )
{
	OBJ_DATA *otmp;
	short bitlayers = 0;
	short objlayers = obj->pIndexData->layers;

	for( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
		if( otmp->wear_loc == wear_loc )
		{
			if( !otmp->pIndexData->layers )
				return false;
			else
				bitlayers |= otmp->pIndexData->layers;
		}
	if( ( bitlayers && !objlayers ) || bitlayers > objlayers )
		return false;
	if( !bitlayers || ( ( bitlayers & ~objlayers ) == bitlayers ) )
		return true;
	return false;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 * Restructured a bit to allow for specifying body location	-Thoric
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, short wear_bit )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *tmpobj;
	short bit, tmp;

	separate_obj( obj );

	if( wear_bit > -1 )
	{
		bit = wear_bit;
		if( !CAN_WEAR( obj, 1 << bit ) )
		{
			if( fReplace )
			{
				switch( 1 << bit )
				{
				case ITEM_HOLD:
					send_to_char( "You cannot hold that.\r\n", ch );
					break;
				case ITEM_WIELD:
					send_to_char( "You cannot wield that.\r\n", ch );
					break;
				default:
					sprintf( buf, "You cannot wear that on your %s.\r\n", w_flags[bit] );
					send_to_char( buf, ch );
				}
			}
			return;
		}
	}
	else
	{
		for( bit = -1, tmp = 1; tmp < 31; tmp++ )
		{
			if( CAN_WEAR( obj, 1 << tmp ) )
			{
				bit = tmp;
				break;
			}
		}
	}

	/*
	 * currently cannot have a light in non-light position
	 */
	if( obj->item_type == ITEM_LIGHT )
	{
		if( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
			return;
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n holds $p&w as a light.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou hold $p&w as your light.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_LIGHT );
		oprog_wear_trigger( ch, obj );
		return;
	}

	if( bit == -1 )
	{
		if( fReplace )
			send_to_char( "You can't wear, wield, or hold that.\r\n", ch );
		return;
	}

	switch( 1 << bit )
	{
	default:
		bug( "wear_obj: uknown/unused item_wear bit %d", bit );
		if( fReplace )
			send_to_char( "You can't wear, wield, or hold that.\r\n", ch );
		return;

	case ITEM_WEAR_FINGER:
		if( get_eq_char( ch, WEAR_FINGER_L )
			&& get_eq_char( ch, WEAR_FINGER_R )
			&& !remove_obj( ch, WEAR_FINGER_L, fReplace ) && !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
			return;

		if( !get_eq_char( ch, WEAR_FINGER_L ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n slips $s left finger into $p&w.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou slip your left finger into $p&w.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_FINGER_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if( !get_eq_char( ch, WEAR_FINGER_R ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n slips $s right finger into $p&w.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou slip your right finger into $p&w.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_FINGER_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		bug( "Wear_obj: no free finger.", 0 );
		send_to_char( "You already wear something on both fingers.\r\n", ch );
		return;

	case ITEM_WEAR_ANKLE:
		if( get_eq_char( ch, WEAR_ANKLE_L )
			&& get_eq_char( ch, WEAR_ANKLE_R )
			&& !remove_obj( ch, WEAR_ANKLE_L, fReplace ) && !remove_obj( ch, WEAR_ANKLE_R, fReplace ) )
			return;

		if( !get_eq_char( ch, WEAR_ANKLE_L ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n slips $p&w over $s left ankle.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou slip $p&w over your left ankle.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_ANKLE_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if( !get_eq_char( ch, WEAR_ANKLE_R ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n slips $p&w over $s right ankle.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou slip $p&w over your right ankle.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_ANKLE_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		bug( "Wear_obj: no free ankle.", 0 );
		send_to_char( "You already have things on both ankles.\r\n", ch );
		return;


	case ITEM_WEAR_NECK:
		if( get_eq_char( ch, WEAR_NECK_1 ) != NULL
			&& get_eq_char( ch, WEAR_NECK_2 ) != NULL
			&& !remove_obj( ch, WEAR_NECK_1, fReplace ) && !remove_obj( ch, WEAR_NECK_2, fReplace ) )
			return;

		if( !get_eq_char( ch, WEAR_NECK_1 ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n wears $p&w around $s neck.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou wear $p&w around your neck.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_NECK_1 );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if( !get_eq_char( ch, WEAR_NECK_2 ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n wears $p&w around $s neck.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou wear $p&w around your neck.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_NECK_2 );
			oprog_wear_trigger( ch, obj );
			return;
		}

		bug( "Wear_obj: no free neck.", 0 );
		send_to_char( "You already wear two neck items.\r\n", ch );
		return;

	case ITEM_WEAR_BODY:
		/*
		 * if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
		 * return;
		 */
		 /*
				 if ( !can_layer( ch, obj, WEAR_BODY ) )
				 {
				 send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
				 return;
				 }
		 */
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n fits $p&w on $s body.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou fit $p&w on your body.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_BODY );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_HEAD:
		if( !remove_obj( ch, WEAR_HEAD, fReplace ) )
			return;
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n dons $p&w upon $s head.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou don $p&w upon your head.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_HEAD );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_EYES:
		if( !remove_obj( ch, WEAR_EYES, fReplace ) )
			return;
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n places $p&w on $s eyes.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou place $p&w on your eyes.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_EYES );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_EARS:
		if( !remove_obj( ch, WEAR_EARS, fReplace ) )
			return;
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w on $s ears.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w on your ears.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_EARS );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_LEGS:
		if( !can_layer( ch, obj, WEAR_LEGS ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n slips into $p&w.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou slip into $p&w.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_LEGS );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_FEET:
		if( !can_layer( ch, obj, WEAR_FEET ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w on $s feet.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w on your feet.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_FEET );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_HANDS:
		/*
				if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
				  return;
		*/
		if( !can_layer( ch, obj, WEAR_HANDS ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w on $s hands.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w on your hands.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_HANDS );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_ARMS:
		/*
				if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
				  return;
		*/
		if( !can_layer( ch, obj, WEAR_ARMS ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w on $s arms.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w on your arms.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_ARMS );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_ABOUT:
		/*
		 * if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
		 * return;
		 */
		if( !can_layer( ch, obj, WEAR_ABOUT ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w about $s body.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w about your body.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_ABOUT );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_RELIC:
		if( !can_layer( ch, obj, WEAR_RELIC ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n uses $p&w as $s relic.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou use $p&w as your relic.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_RELIC );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_WAIST:
		/*
				if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
				  return;
		*/
		if( !can_layer( ch, obj, WEAR_WAIST ) )
		{
			send_to_char( "It won't fit overtop of what you're already wearing.\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wears $p&w about $s waist.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wear $p&w about your waist.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_WAIST );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_WRIST:
		if( get_eq_char( ch, WEAR_WRIST_L )
			&& get_eq_char( ch, WEAR_WRIST_R )
			&& !remove_obj( ch, WEAR_WRIST_L, fReplace ) && !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
			return;

		if( !get_eq_char( ch, WEAR_WRIST_L ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n fits $p&w around $s left wrist.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou fit $p&w around your left wrist.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_WRIST_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if( !get_eq_char( ch, WEAR_WRIST_R ) )
		{
			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if( !obj->action_desc || obj->action_desc[0] == '\0' )
				{
					act( AT_ACTION, "&w$n fits $p&w around $s right wrist.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "&wYou fit $p&w around your right wrist.", ch, obj, NULL, TO_CHAR );
				}
				else
					actiondesc( ch, obj, NULL );
			}
			equip_char( ch, obj, WEAR_WRIST_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		bug( "Wear_obj: no free wrist.", 0 );
		send_to_char( "You already wear two wrist items.\r\n", ch );
		return;

	case ITEM_WEAR_SHIELD:
		if( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
			return;
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n uses $p&w as an energy shield.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou use $p&w as an energy shield.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_SHIELD );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WIELD:
		if( ( tmpobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL && !could_dual( ch ) )
		{
			send_to_char( "You're already wielding something.\r\n", ch );
			return;
		}

		if( tmpobj )
		{
			if( can_dual( ch ) )
			{
				if( get_obj_weight( obj ) + get_obj_weight( tmpobj ) > str_app[get_curr_str( ch )].wield )
				{
					send_to_char( "It is too heavy for you to wield.\r\n", ch );
					return;
				}
				if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					if( !obj->action_desc || obj->action_desc[0] == '\0' )
					{
						act( AT_ACTION, "&w$n dual-wields $p&w.", ch, obj, NULL, TO_ROOM );
						act( AT_ACTION, "&wYou dual-wield $p&w.", ch, obj, NULL, TO_CHAR );
					}
					else
						actiondesc( ch, obj, NULL );
				}
				equip_char( ch, obj, WEAR_DUAL_WIELD );
				oprog_wear_trigger( ch, obj );
			}
			return;
		}

		if( get_obj_weight( obj ) > str_app[get_curr_str( ch )].wield )
		{
			send_to_char( "It is too heavy for you to wield.\r\n", ch );
			return;
		}

		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n wields $p&w.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou wield $p&w.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_WIELD );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_HOLD:
		if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
		{
			send_to_char( "You cannot hold something AND two weapons!\r\n", ch );
			return;
		}
		if( !remove_obj( ch, WEAR_HOLD, fReplace ) )
			return;
		if( obj->item_type == ITEM_DEVICE
			|| obj->item_type == ITEM_GRENADE
			|| obj->item_type == ITEM_FOOD
			|| obj->item_type == ITEM_PILL
			|| obj->item_type == ITEM_POTION
			|| obj->item_type == ITEM_DRINK_CON
			|| obj->item_type == ITEM_SALVE
			|| obj->item_type == ITEM_KEY || !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "&w$n holds $p&w in $s hands.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "&wYou hold $p&w in your hands.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_HOLD );
		oprog_wear_trigger( ch, obj );
		return;

	case ITEM_WEAR_FLOATING:
		if( !can_layer( ch, obj, WEAR_FLOATING ) )
		{
			send_to_char( "You already have an object floating by you!\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n turns $p&w on. $p rises up and floats behind $n.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou turn $p&w on, as it rises up and floats behind you.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_FLOATING );
		oprog_wear_trigger( ch, obj );
		return;
	case ITEM_WEAR_BACK:
		if( !can_layer( ch, obj, WEAR_BACK ) )
		{
			send_to_char( "You already have something on your back!\r\n", ch );
			return;
		}
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			if( !obj->action_desc || obj->action_desc[0] == '\0' )
			{
				act( AT_ACTION, "&w$n throws $p&w onto $s back.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "&wYou throw $p&w onto your back.", ch, obj, NULL, TO_CHAR );
			}
			else
				actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_FLOATING );
		oprog_wear_trigger( ch, obj );
		return;

	}
}


CMDF( do_wear )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	short wear_bit;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if( ( !str_cmp( arg2, "on" ) || !str_cmp( arg2, "upon" ) || !str_cmp( arg2, "around" ) ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Wear, wield, or hold what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( !str_cmp( arg1, "all" ) )
	{
		OBJ_DATA *obj_next;

		for( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
				wear_obj( ch, obj, false, -1 );
		}
		return;
	}
	else
	{
		if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\r\n", ch );
			return;
		}
		if( arg2[0] != '\0' )
			wear_bit = get_wflag( arg2 );
		else
			wear_bit = -1;
		wear_obj( ch, obj, true, wear_bit );
	}

	return;
}



CMDF( do_remove )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *obj_next;


	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Remove what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		send_to_char( "You are not using that item.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "all" ) ) /* SB Remove all */
	{
		for( obj = ch->first_carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			if( obj->wear_loc != WEAR_NONE && can_see_obj( ch, obj ) )
				remove_obj( ch, obj->wear_loc, true );
		}
		return;
	}

	if( ( obj = get_obj_wear( ch, arg ) ) == NULL )
	{
		send_to_char( "You are not using that item.\r\n", ch );
		return;
	}
	if( ( obj_next = get_eq_char( ch, obj->wear_loc ) ) != obj )
	{
		act( AT_PLAIN, "You must remove $p&w first.", ch, obj_next, NULL, TO_CHAR );
		return;
	}

	remove_obj( ch, obj->wear_loc, true );
	return;
}


CMDF( do_bury )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	bool shovel;
	short move;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "What do you wish to bury?\r\n", ch );
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
		send_to_char( "You can't find it.\r\n", ch );
		return;
	}

	separate_obj( obj );
	if( !CAN_WEAR( obj, ITEM_TAKE ) )
	{
		act( AT_PLAIN, "You cannot bury $p&w.", ch, obj, 0, TO_CHAR );
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
		send_to_char( "You cannot bury something here.\r\n", ch );
		return;
	case SECT_AIR:
		send_to_char( "What?  In the air?!\r\n", ch );
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

	act( AT_ACTION, "&wYou solemnly bury $p&w...", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "&w$n&w solemnly buries $p&w...", ch, obj, NULL, TO_ROOM );
	xSET_BIT( obj->extra_flags, ITEM_BURRIED );
	WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );
	return;
}

CMDF( do_sacrifice )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
	{
		act( AT_ACTION, "&w$n offers $mself to $s deity, who graciously declines.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "Your deity appreciates your offer and may accept it later.", ch );
		return;
	}

	if( ( !str_cmp( arg, "all" ) ) || ( !str_cmp( arg, "ALL" ) ) )
	{
		OBJ_DATA *obj_next;

		if( !ch->in_room->first_content )
		{
			send_to_char( "There's nothing in the room to sacrifice.\r\n", ch );
			return;
		}

		for( obj = ch->in_room->first_content; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;

			if( ( obj->item_type == ITEM_CORPSE_PC )
				|| ( obj->item_type == ITEM_FURNITURE ) || ( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) ) )
				continue;

			//  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)

			extract_obj( obj );
		}

		send_to_char( "You sacrifice everything in the room.\r\n", ch );
		act( AT_PLAIN, "$n sacrifices everything in the room.", ch, obj, NULL, TO_ROOM );
		return;
	}


	if( ms_find_obj( ch ) )
		return;

	obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
	if( !obj )
	{
		send_to_char( "You can't find it.\r\n", ch );
		return;
	}

	separate_obj( obj );
	if( !CAN_WEAR( obj, ITEM_TAKE ) || ( obj->item_type == ITEM_CORPSE_PC ) || ( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) ) )
	{
		act( AT_PLAIN, "$p&w is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
		return;
	}

	oprog_sac_trigger( ch, obj );
	if( obj_extracted( obj ) )
		return;
	if( cur_obj == obj->serial )
		global_objcode = rOBJ_SACCED;
	extract_obj( obj );
	return;
}

CMDF( do_brandish )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	OBJ_DATA *staff;
	ch_ret retcode;
	int sn;

	if( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
	{
		send_to_char( "You hold nothing in your hand.\r\n", ch );
		return;
	}

	if( staff->item_type != ITEM_STAFF )
	{
		send_to_char( "You can brandish only with a staff.\r\n", ch );
		return;
	}

	if( ( sn = staff->value[3] ) < 0 || sn >= top_sn || skill_table[sn]->spell_fun == NULL )
	{
		bug( "Do_brandish: bad sn %d.", sn );
		return;
	}

	WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

	if( staff->value[2] > 0 )
	{
		if( !oprog_use_trigger( ch, staff, NULL, NULL, NULL ) )
		{
			act( AT_MAGIC, "$n brandishes $p&w.", ch, staff, NULL, TO_ROOM );
			act( AT_MAGIC, "You brandish $p&w.", ch, staff, NULL, TO_CHAR );
		}
		for( vch = ch->in_room->first_person; vch; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) && vch->pcdata->wizinvis >= LEVEL_STAFF )
				continue;
			else
				switch( skill_table[sn]->target )
				{
				default:
					bug( "Do_brandish: bad target for sn %d.", sn );
					return;

				case TAR_IGNORE:
					if( vch != ch )
						continue;
					break;

				case TAR_CHAR_OFFENSIVE:
					if( IS_NPC( ch ) ? IS_NPC( vch ) : !IS_NPC( vch ) )
						continue;
					break;

				case TAR_CHAR_DEFENSIVE:
					if( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
						continue;
					break;

				case TAR_CHAR_SELF:
					if( vch != ch )
						continue;
					break;
				}

			retcode = obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
			if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
			{
				bug( "do_brandish: char died", 0 );
				return;
			}
		}
	}

	if( --staff->value[2] <= 0 )
	{
		act( AT_MAGIC, "$p&w blazes bright and vanishes from $n's hands!", ch, staff, NULL, TO_ROOM );
		act( AT_MAGIC, "$p&w blazes bright and is gone!", ch, staff, NULL, TO_CHAR );
		if( staff->serial == cur_obj )
			global_objcode = rOBJ_USED;
		extract_obj( staff );
	}

	return;
}



CMDF( do_zap )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wand;
	OBJ_DATA *obj;
	ch_ret retcode;

	one_argument( argument, arg );
	if( arg[0] == '\0' && !ch->fighting )
	{
		send_to_char( "Zap whom or what?\r\n", ch );
		return;
	}

	if( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
	{
		send_to_char( "You hold nothing in your hand.\r\n", ch );
		return;
	}

	if( wand->item_type != ITEM_WAND )
	{
		send_to_char( "You can zap only with a wand.\r\n", ch );
		return;
	}

	obj = NULL;
	if( arg[0] == '\0' )
	{
		if( ch->fighting )
		{
			victim = who_fighting( ch );
		}
		else
		{
			send_to_char( "Zap whom or what?\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
		{
			send_to_char( "You can't find it.\r\n", ch );
			return;
		}
	}

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

	if( wand->value[2] > 0 )
	{
		if( victim )
		{
			if( !oprog_use_trigger( ch, wand, victim, NULL, NULL ) )
			{
				act( AT_MAGIC, "$n aims $p&w at $N.", ch, wand, victim, TO_ROOM );
				act( AT_MAGIC, "You aim $p&w at $N.", ch, wand, victim, TO_CHAR );
			}
		}
		else
		{
			if( !oprog_use_trigger( ch, wand, NULL, obj, NULL ) )
			{
				act( AT_MAGIC, "$n aims $p&w at $P&w.", ch, wand, obj, TO_ROOM );
				act( AT_MAGIC, "You aim $p&w at $P&w.", ch, wand, obj, TO_CHAR );
			}
		}

		retcode = obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
		if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
		{
			bug( "do_zap: char died", 0 );
			return;
		}
	}

	if( --wand->value[2] <= 0 )
	{
		act( AT_MAGIC, "$p&w explodes into fragments.", ch, wand, NULL, TO_ROOM );
		act( AT_MAGIC, "$p&w explodes into fragments.", ch, wand, NULL, TO_CHAR );
		if( wand->serial == cur_obj )
			global_objcode = rOBJ_USED;
		extract_obj( wand );
	}

	return;
}

/*
 * Save items in a clan storage room			-Scryn & Thoric
 */
void save_clan_storeroom( CHAR_DATA *ch, CLAN_DATA *clan )
{
	FILE *fp;
	char filename[256];
	short templvl;
	OBJ_DATA *contents;

	if( !clan )
	{
		bug( "save_clan_storeroom: Null clan pointer!", 0 );
		return;
	}

	if( !ch )
	{
		bug( "save_clan_storeroom: Null ch pointer!", 0 );
		return;
	}

	sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_clan_storeroom: FileOpen", 0 );
		perror( filename );
	}
	else
	{
		templvl = ch->top_level;
		ch->top_level = LEVEL_HERO;   /* make sure EQ doesn't get lost */
		contents = ch->in_room->last_content;
		if( contents )
			fwrite_obj( ch, contents, fp, 0, OS_CARRY, false );
		fprintf( fp, "#END\n" );
		ch->top_level = templvl;
		FileClose( fp );
		return;
	}
	return;
}


/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall( OBJ_DATA *obj, bool through )
{
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *to_room;
	static int fall_count;
	char buf[MAX_STRING_LENGTH];
	static bool is_falling;  /* Stop loops from the call to obj_to_room()  -- Altrag */

	if( !obj->in_room || is_falling )
		return;

	if( fall_count > 30 )
	{
		bug( "object falling in loop more than 30 times", 0 );
		extract_obj( obj );
		fall_count = 0;
		return;
	}

	if( xIS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && CAN_GO( obj, DIR_DOWN ) && !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
	{

		pexit = get_exit( obj->in_room, DIR_DOWN );
		to_room = pexit->to_room;

		if( through )
			fall_count++;
		else
			fall_count = 0;

		if( obj->in_room == to_room )
		{
			sprintf( buf, "Object falling into same room, room %d", to_room->vnum );
			bug( buf, 0 );
			extract_obj( obj );
			return;
		}

		if( obj->in_room->first_person )
		{
			act( AT_PLAIN, "$p&w falls far below...", obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p&w falls far below...", obj->in_room->first_person, obj, NULL, TO_CHAR );
		}
		obj_from_room( obj );
		is_falling = true;
		obj = obj_to_room( obj, to_room );
		is_falling = false;

		if( obj->in_room->first_person )
		{
			act( AT_PLAIN, "$p&w falls from above...", obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p&w falls from above...", obj->in_room->first_person, obj, NULL, TO_CHAR );
		}

		if( !xIS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && through )
		{
			/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
			*/ int dam = fall_count * obj->weight / 2;
			/*
			 * Damage players
			 */
			if( obj->in_room->first_person && number_percent( ) > 15 )
			{
				CHAR_DATA *rch;
				CHAR_DATA *vch = NULL;
				int chcnt = 0;

				for( rch = obj->in_room->first_person; rch; rch = rch->next_in_room, chcnt++ )
					if( number_range( 0, chcnt ) == 0 )
						vch = rch;
				act( AT_WHITE, "$p&w falls on $n!", vch, obj, NULL, TO_ROOM );
				act( AT_WHITE, "$p&w falls on you!", vch, obj, NULL, TO_CHAR );
				damage( vch, vch, dam * vch->top_level, TYPE_UNDEFINED );
			}
			/*
			 * Damage objects
			 */
			switch( obj->item_type )
			{
			case ITEM_WEAPON:
			case ITEM_ARMOR:
				if( ( obj->value[0] - dam ) <= 0 )
				{
					if( obj->in_room->first_person )
					{
						act( AT_PLAIN, "$p&w is destroyed by the fall!", obj->in_room->first_person, obj, NULL, TO_ROOM );
						act( AT_PLAIN, "$p&w is destroyed by the fall!", obj->in_room->first_person, obj, NULL, TO_CHAR );
					}
					make_scraps( obj );
				}
				else
					obj->value[0] -= dam;
				break;
			default:
				if( ( dam * 15 ) > get_obj_resistance( obj ) )
				{
					if( obj->in_room->first_person )
					{
						act( AT_PLAIN, "$p&w is destroyed by the fall!", obj->in_room->first_person, obj, NULL, TO_ROOM );
						act( AT_PLAIN, "$p&w is destroyed by the fall!", obj->in_room->first_person, obj, NULL, TO_CHAR );
					}
					make_scraps( obj );
				}
				break;
			}
		}
		obj_fall( obj, true );
	}
	return;
}

CMDF( do_sheath )
{
	OBJ_DATA *sheath;
	OBJ_DATA *weapon = NULL;
	char buf[MAX_STRING_LENGTH];
	const char *type;
	int found = 0;
	int rev = 0;
	int count = 0;

	if( !str_cmp( argument, "go reverse" ) )
		rev = 1;

	if( !str_cmp( argument, "go reverse again" ) )
		rev = 2;

	if( ms_find_obj( ch ) )
		return;

	if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
	{
		weapon = get_eq_char( ch, WEAR_DUAL_WIELD );
	}
	else
	{
		if( get_eq_char( ch, WEAR_WIELD ) )
		{
			weapon = get_eq_char( ch, WEAR_WIELD );
		}
	}
	if( !weapon )
	{
		send_to_char( "You need to be wielding a weapon to put it away.\r\n", ch );
		return;
	}

	switch( weapon->value[3] )
	{
	default:
		type = "(Its broken! Tell Axis!)";
		break;
	case 1:
	case 2:
	case 3:
	case 5:
	case 7:
	case 8:
	case 11:
	case 12:
		type = "sheath";
		break;

	case 4:
	case 6:
	case 9:
		type = "holster";
		break;
	}


	if( IS_OBJ_STAT( weapon, ITEM_NOREMOVE ) )
	{
		act( AT_PLAIN, "You can't remove $p&w.", ch, weapon, NULL, TO_CHAR );
		return;
	}
	/*
	 * Total of 3 possible sheath locations, will loop through three times
	 * to find a good one if it has to
	 */
	for( sheath = ch->first_carrying; sheath; sheath = sheath->next_content )
	{
		if( sheath->item_type == ITEM_WHOLDER && ( sheath->wear_loc == WEAR_WAIST || sheath->wear_loc == WEAR_BACK ) )
		{
			found = 1;
			if( rev != count )
			{
				count++;
				continue;
			}
			if( sheath->value[1] == weapon->value[3] && get_obj_weight( weapon ) <= sheath->value[0] && !sheath->first_content )
			{
				if( sheath->value[2] != 0 && sheath->value[2] == weapon->pIndexData->vnum )
					break;
				else
				{
					if( sheath->value[2] == 0 )
						break;
					else
						continue;
				}

				break;
			}
		}
	}
	if( found == 0 )
	{
		sprintf( buf, "You need to be wearing a %s, before you can %s your weapon.\r\n", type, type );
		send_to_char( buf, ch );
		return;
	}
	if( !sheath )
	{
		sprintf( buf, "%s&w won't fit in that.\r\n", weapon->short_descr );
		send_to_char( buf, ch );
		return;
	}
	if( ( weapon->pIndexData->vnum != sheath->value[2] ) && sheath->value[2] != 0 )
	{
		if( rev == 0 )
		{
			do_sheath( ch, "go reverse" );
			return;
		}
		else if( rev == 1 )
		{
			do_sheath( ch, "go reverse again" );
			return;
		}
		else
		{
			sprintf( buf, "Your %s was made for a particular weapon.\r\n", type );
			send_to_char( buf, ch );
			return;
		}
	}

	sprintf( buf, "'%s'", weapon->name );
	do_remove( ch, buf );
	sprintf( buf, "'%s' '%s'", weapon->name, sheath->name );
	do_put( ch, buf );

	if( !sheath->first_content )
	{
		sprintf( buf, "'%s'", weapon->name );
		do_wear( ch, buf );
	}
	return;
}

CMDF( do_draw )
{
	OBJ_DATA *sheath;
	OBJ_DATA *weapon = NULL;
	OBJ_DATA *tmpobj;
	char buf[MAX_STRING_LENGTH];
	int found = 0;

	if( ms_find_obj( ch ) )
		return;

	for( sheath = ch->first_carrying; sheath; sheath = sheath->next_content )
	{
		if( sheath->item_type == ITEM_WHOLDER && ( sheath->wear_loc == WEAR_WAIST || sheath->wear_loc == WEAR_BACK ) )
		{
			found = 1;
			if( sheath->first_content )
			{
				weapon = sheath->first_content;
				break;
			}
		}
	}

	if( found == 0 )
	{
		send_to_char( "You aren't wearing a weapon holder.", ch );
		return;
	}
	if( !weapon )
	{
		//       sprintf(buf, "Your %s is empty.\r\n", sheath->name );
		//       send_to_char( buf, ch );
		send_to_char( "Your weapon holder is empty.\r\n", ch );
		return;
	}
	/*
	   switch( weapon->value[3] )
	   {
			default:    type = "(Its broken! Tell Cray!)";  break;
			case 1:
			case 2:
			case 3:
			case 5:
			case 7:
			case 8:
			case 11:
			case 12:
			type = "sheath"; break;

			case 4:
			case 6:
			case 9:
			type = "holster"; break;
	   }
	*/
	/*
	 * Quick checks to see if it is wieldable before removing, will safe some
	 * text spam on the player plus problems
	 */
	if( !could_dual( ch ) )
	{
		if( !remove_obj( ch, WEAR_MISSILE_WIELD, 2 ) )
			return;
		if( !remove_obj( ch, WEAR_WIELD, 2 ) )
			return;
		tmpobj = NULL;
		if( get_obj_weight( weapon ) > str_app[get_curr_str( ch )].wield )
		{
			send_to_char( "It is too heavy for you to wield.\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( tmpobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL
			&& ( get_eq_char( ch, WEAR_MISSILE_WIELD ) || get_eq_char( ch, WEAR_DUAL_WIELD ) ) )
		{
			send_to_char( "You're already wielding two weapons.\r\n", ch );
			return;
		}
		if( tmpobj )
		{
			if( get_obj_weight( weapon ) + get_obj_weight( tmpobj ) > str_app[get_curr_str( ch )].wield )
			{
				send_to_char( "It is too heavy for you to wield.\r\n", ch );
				return;
			}
		}
		else
		{
			if( get_obj_weight( weapon ) > str_app[get_curr_str( ch )].wield )
			{
				send_to_char( "It is too heavy for you to wield.\r\n", ch );
				return;
			}
		}
	}

	sprintf( buf, "'%s' '%s'", weapon->name, sheath->name );
	do_get( ch, buf );
	sprintf( buf, "'%s'", weapon->name );
	do_wear( ch, buf );
	if( weapon->wear_loc != WEAR_WIELD && weapon->wear_loc != WEAR_MISSILE_WIELD && weapon->wear_loc != WEAR_DUAL_WIELD )
	{
		sprintf( buf, "'%s' '%s'", weapon->name, sheath->name );
		do_put( ch, buf );
	}
	/*
		if (argument[0] != '\0')
		{
		   CHAR_DATA *victim;

		   if ( ( victim = get_char_room( ch, argument ) ) == NULL )
		   {
		  send_to_char( "They aren't here.\r\n", ch );
		  return;
		   }

		   if ( victim == ch )
		   {
		  send_to_char( "You hit yourself.  Ouch!\r\n", ch );
		  return;
		   }

		   if ( is_safe( ch, victim ) )
		  return;

		   if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
		   {
			  act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
		  return;
		   }

		   if ( ch->position == POS_FIGHTING )
		   {
		  send_to_char( "You do the best you can!\r\n", ch );
		  return;
		   }

	//       check_attacker( ch, victim );
	//       multi_hit( ch, victim, gsn_unsheath );
		   return;
		}
		if (ch->fighting && ch->fighting->who != ch)
		{
	//       check_attacker( ch, ch->fighting->who );
	//       multi_hit( ch, ch->fighting->who, gsn_unsheath );
		}
		return;
	*/
}
