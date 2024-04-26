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

#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3

#define TRACK_THROUGH_DOORS

extern int top_room;

bool mob_snipe( CHAR_DATA *ch, CHAR_DATA *victim );
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

struct bfs_queue_struct
{
	ROOM_INDEX_DATA *room;
	char dir;
	struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = NULL, *queue_tail = NULL, *room_queue = NULL;

/* Utility macros */
#define MARK(room)		xSET_BIT( (room)->room_flags, BFS_MARK )
#define UNMARK(room)		xREMOVE_BIT( (room)->room_flags, BFS_MARK )
#define IS_MARKED(room)		xIS_SET( (room)->room_flags, BFS_MARK )

ROOM_INDEX_DATA *toroom( ROOM_INDEX_DATA *room, short door )
{
	return ( get_exit( room, door )->to_room );
}

bool valid_edge( ROOM_INDEX_DATA *room, short door )
{
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *to_room;

	pexit = get_exit( room, door );
	if( pexit && ( to_room = pexit->to_room ) != NULL
#ifndef TRACK_THROUGH_DOORS
		&& !IS_SET( pexit->exit_info, EX_CLOSED )
#endif
		&& !IS_MARKED( to_room ) )
		return true;
	else
		return false;
}

void bfs_enqueue( ROOM_INDEX_DATA *room, char dir )
{
	struct bfs_queue_struct *curr;

	CREATE( curr, struct bfs_queue_struct, 1 );
	curr->room = room;
	curr->dir = dir;
	curr->next = NULL;

	if( queue_tail )
	{
		queue_tail->next = curr;
		queue_tail = curr;
	}
	else
		queue_head = queue_tail = curr;
}


void bfs_dequeue( void )
{
	struct bfs_queue_struct *curr;

	curr = queue_head;

	if( !( queue_head = queue_head->next ) )
		queue_tail = NULL;
	DISPOSE( curr );
}


void bfs_clear_queue( void )
{
	while( queue_head )
		bfs_dequeue( );
}

void room_enqueue( ROOM_INDEX_DATA *room )
{
	struct bfs_queue_struct *curr;

	CREATE( curr, struct bfs_queue_struct, 1 );
	curr->room = room;
	curr->next = room_queue;

	room_queue = curr;
}

void clean_room_queue( void )
{
	struct bfs_queue_struct *curr, *curr_next;

	for( curr = room_queue; curr; curr = curr_next )
	{
		//UNMARK( curr->room );
		xREMOVE_BIT( curr->room->room_flags, BFS_MARK );
		curr_next = curr->next;
		DISPOSE( curr );
	}
	room_queue = NULL;
}


int find_first_step( ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *target, int maxdist )
{
	int curr_dir, count;

	if( !src || !target )
	{
		bug( "Illegal value passed to find_first_step (track.c)", 0 );
		return BFS_ERROR;
	}

	if( src == target )
		return BFS_ALREADY_THERE;

	if( src->area != target->area )
		return BFS_NO_PATH;

	room_enqueue( src );
	MARK( src );

	/*
	 * first, enqueue the first steps, saving which direction we're going.
	 */
	for( curr_dir = 0; curr_dir < 10; curr_dir++ )
	{
		if( valid_edge( src, curr_dir ) )
		{
			MARK( toroom( src, curr_dir ) );
			room_enqueue( toroom( src, curr_dir ) );
			bfs_enqueue( toroom( src, curr_dir ), curr_dir );
		}
	}

	count = 0;
	while( queue_head )
	{
		if( ++count > maxdist )
		{
			bfs_clear_queue( );
			clean_room_queue( );
			return BFS_NO_PATH;
		}
		if( queue_head->room == target )
		{
			curr_dir = queue_head->dir;
			bfs_clear_queue( );
			clean_room_queue( );
			return curr_dir;
		}
		else
		{
			for( curr_dir = 0; curr_dir < 10; curr_dir++ )
			{
				if( valid_edge( queue_head->room, curr_dir ) )
				{
					MARK( toroom( queue_head->room, curr_dir ) );
					room_enqueue( toroom( queue_head->room, curr_dir ) );
					bfs_enqueue( toroom( queue_head->room, curr_dir ), queue_head->dir );
				}
			}
			bfs_dequeue( );
		}
	}
	clean_room_queue( );

	return BFS_NO_PATH;
}


CMDF( do_track )
{
	CHAR_DATA *vict;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int dir, maxdist;

	if( !IS_NPC( ch ) && !ch->pcdata->learned[gsn_track] )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ch->in_room && !IS_IMMORTAL( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_TAG ) )
	{
		send_to_char( "Not while in the tag arena!\r\n", ch );
		return;
	}

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Whom are you trying to track?\r\n", ch );

		if( IS_AFFECTED( ch, AFF_STALK ) && ( !ch->hunting ) )
		{
			REMOVE_BIT( ch->affected_by, AFF_STALK );
		}

		return;
	}

	WAIT_STATE( ch, skill_table[gsn_track]->beats );

	if( !( vict = get_char_world( ch, arg ) ) || ( !can_see( ch, vict ) ) )
	{
		send_to_char( "You can't find a trail of anyone like that.\r\n", ch );

		if( IS_AFFECTED( ch, AFF_STALK ) )
		{
			REMOVE_BIT( ch->affected_by, AFF_STALK );
		}

		return;
	}

	if( IS_AFFECTED( ch, AFF_STALK ) )
	{
		start_hunting( ch, vict );
	}

	maxdist = 100 + ch->top_level * 30;

	if( !IS_NPC( ch ) )
		maxdist = ( maxdist * ch->pcdata->learned[gsn_track] ) / 100;

	dir = find_first_step( ch->in_room, vict->in_room, maxdist );
	switch( dir )
	{
	case BFS_ERROR:
		send_to_char( "Hmm... something seems to be wrong.\r\n", ch );
		break;
	case BFS_ALREADY_THERE:
		send_to_char( "You're already in the same room!\r\n", ch );
		if( IS_AFFECTED( ch, AFF_STALK ) )
		{
			REMOVE_BIT( ch->affected_by, AFF_STALK );
			stop_hunting( ch );
		}
		break;
	case BFS_NO_PATH:
		snprintf( buf, MAX_STRING_LENGTH, "You can't sense a trail from here.\r\n" );
		send_to_char( buf, ch );
		learn_from_failure( ch, gsn_track );

		if( IS_AFFECTED( ch, AFF_STALK ) )
		{
			REMOVE_BIT( ch->affected_by, AFF_STALK );
			stop_hunting( ch );
		}

		break;
	default:
		ch_printf( ch, "You sense a trail %s from here...\r\n", dir_name[dir] );
		learn_from_success( ch, gsn_track );

		if( IS_AFFECTED( ch, AFF_STALK ) )
		{
			learn_from_success( ch, gsn_stalk );
		}

		break;
	}
}


void found_prey( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_STRING_LENGTH];
	char victname[MAX_STRING_LENGTH];



	if( victim == NULL )
	{
		bug( "Found_prey: null victim", 0 );
		return;
	}

	if( ch == NULL )
	{
		bug( "Found_prey: null ch", 0 );
		return;
	}

	if( victim->in_room == NULL )
	{
		bug( "Found_prey: null victim->in_room", 0 );
		return;
	}

	mudstrlcpy( victname, IS_NPC( victim ) ? victim->short_descr : victim->name, MAX_INPUT_LENGTH );

	if( !can_see( ch, victim ) )
	{
		if( number_percent( ) < 90 )
			return;
		switch( number_bits( 2 ) )
		{
		case 0:
			snprintf( buf, MAX_STRING_LENGTH, "Don't make me find you, %s!", victname );
			do_say( ch, buf );
			break;
		case 1:
			act( AT_ACTION, "$n sniffs around the room for $N.", ch, NULL, victim, TO_NOTVICT );
			act( AT_ACTION, "You sniff around the room for $N.", ch, NULL, victim, TO_CHAR );
			act( AT_ACTION, "$n sniffs around the room for you.", ch, NULL, victim, TO_VICT );
			snprintf( buf, MAX_STRING_LENGTH, "I can smell your blood!" );
			do_say( ch, buf );
			break;
		case 2:
			snprintf( buf, MAX_STRING_LENGTH, "I'm going to tear %s apart!", victname );
			do_say( ch, buf );
			break;
		case 3:
			do_say( ch, "Just wait until I find you..." );
			break;
		}
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		if( number_percent( ) < 90 )
			return;
		switch( number_bits( 2 ) )
		{
		case 0:
			do_say( ch, "C'mon out, you coward!" );
			snprintf( buf, MAX_STRING_LENGTH, "%s is a bloody coward!", victname );
			do_say( ch, buf );
			break;
		case 1:
			snprintf( buf, MAX_STRING_LENGTH, "Let's take this outside, %s", victname );
			do_say( ch, buf );
			break;
		case 2:
			snprintf( buf, MAX_STRING_LENGTH, "%s is a yellow-bellied wimp!", victname );
			do_say( ch, buf );
			break;
		case 3:
			act( AT_ACTION, "$n takes a few swipes at $N.", ch, NULL, victim, TO_NOTVICT );
			act( AT_ACTION, "You try to take a few swipes $N.", ch, NULL, victim, TO_CHAR );
			act( AT_ACTION, "$n takes a few swipes at you.", ch, NULL, victim, TO_VICT );
			break;
		}
		return;
	}

	switch( number_bits( 2 ) )
	{
	case 0:
		snprintf( buf, MAX_STRING_LENGTH, "Your blood is mine, %s!", victname );
		do_say( ch, buf );
		break;
	case 1:
		snprintf( buf, MAX_STRING_LENGTH, "Alas, we meet again, %s!", victname );
		do_say( ch, buf );
		break;
	case 2:
		snprintf( buf, MAX_STRING_LENGTH, "What do you want on your tombstone, %s?", victname );
		do_say( ch, buf );
		break;
	case 3:
		act( AT_ACTION, "$n lunges at $N from out of nowhere!", ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "You lunge at $N catching $M off guard!", ch, NULL, victim, TO_CHAR );
		act( AT_ACTION, "$n lunges at you from out of nowhere!", ch, NULL, victim, TO_VICT );
	}
	stop_hunting( ch );
	set_fighting( ch, victim );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
}

void hunt_victim( CHAR_DATA *ch )
{
	bool found;
	CHAR_DATA *tmp;
	short ret;

	if( !ch || !ch->hunting || !ch->hunting->who )
		return;

	/*
	 * make sure the char still exists
	 */
	for( found = false, tmp = first_char; tmp && !found; tmp = tmp->next )
	{
		if( ch->hunting->who == tmp )
			found = true;
	}

	if( !found )
	{
		do_say( ch, "Damn!  My prey is gone!!" );
		stop_hunting( ch );
		return;
	}

	if( ch->in_room == ch->hunting->who->in_room )
	{
		if( ch->fighting )
			return;
		found_prey( ch, ch->hunting->who );
		return;
	}

	/*
	 * hunting with snipe
	 */
	{
		OBJ_DATA *wield;

		wield = get_eq_char( ch, WEAR_WIELD );
		if( wield != NULL && wield->value[3] == WEAPON_BLASTER )
		{
			if( mob_snipe( ch, ch->hunting->who ) == true )
				return;
		}
		else if( !xIS_SET( ch->act, ACT_DROID ) )
			do_hide( ch, "" );
	}

	ret = find_first_step( ch->in_room, ch->hunting->who->in_room, 5000 );
	if( ret < 0 )
	{
		do_say( ch, "Damn!  Lost my prey!" );
		stop_hunting( ch );
		return;
	}
	else
	{
		EXIT_DATA *pexit;
		if( ( pexit = get_exit( ch->in_room, ret ) ) == NULL )
			return;
		if( xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
			return;
		move_char( ch, pexit, false );
		if( !ch->hunting )
		{
			if( !ch->in_room )
			{
				char buf[MAX_STRING_LENGTH];
				snprintf( buf, MAX_STRING_LENGTH, "Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.",
					ch->pIndexData->vnum, ch->name );
				bug( buf, 0 );
				char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
				return;
			}
			do_say( ch, "Damn!  Lost my prey!" );
			return;
		}
		if( ch->in_room == ch->hunting->who->in_room )
			found_prey( ch, ch->hunting->who );
		return;
	}
}

bool mob_snipe( CHAR_DATA *ch, CHAR_DATA *victim )
{
	short dir, dist;
	short max_dist = 3;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in_room;
	ROOM_INDEX_DATA *to_room;
	char buf[MAX_STRING_LENGTH];
	bool pfound = false;

	if( !ch->in_room || !victim->in_room )
		return false;

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		return false;

	for( dir = 0; dir <= 10; dir++ )
	{
		if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
			continue;

		if( IS_SET( pexit->exit_info, EX_CLOSED ) )
			continue;

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


			if( ch->in_room == victim->in_room )
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
			char_from_room( ch );
			char_to_room( ch, was_in_room );
			continue;
		}

		if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
			return false;

		if( is_safe( ch, victim ) )
			return false;

		if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
			return false;

		if( ch->position == POS_FIGHTING )
			return false;

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

		snprintf( buf, MAX_STRING_LENGTH, "A blaster shot fires at you from the %s.", dir_name[dir] );
		act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
		act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );
		snprintf( buf, MAX_STRING_LENGTH, "A blaster shot fires at $N from the %s.", dir_name[dir] );
		act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );

		one_hit( ch, victim, TYPE_UNDEFINED );

		if( char_died( ch ) )
			return true;

		stop_fighting( ch, true );

		if( victim && !char_died( victim ) && victim->hit < 0 )
		{
			stop_hunting( ch );
			stop_hating( ch );
		}

		char_from_room( ch );
		char_to_room( ch, was_in_room );

		return true;
	}

	return false;
}

CMDF( do_stalk )
{

	if( IS_AFFECTED( ch, AFF_STALK ) )
	{
		send_to_char( "You are already stalking someone.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && number_percent( ) > ch->pcdata->learned[gsn_stalk] )
	{
		send_to_char( "You fail!\r\n", ch );
		learn_from_failure( ch, gsn_stalk );
		return;
	}
	SET_BIT( ch->affected_by, AFF_STALK );
	do_track( ch, argument );
	return;
}
