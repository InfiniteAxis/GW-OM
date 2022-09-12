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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define PREP_START  42   /* vnum of first prep room */
#define PREP_END    43   /* vnum of last prep room */
#define ARENA_START number_range( 29, 41)    /* vnum of first real arena room*/
#define ARENA_END   41   /* vnum of last real arena room*/
#define HALL_FAME_FILE  SYSTEM_DIR "halloffame.lst"
#define ARENA_MAXBET 100
struct hall_of_fame_element
{
	char name[MAX_INPUT_LENGTH + 1];
	time_t date;
	int award;
	struct  hall_of_fame_element *next;
};

/*void sportschan(char *)*/
void start_arena( );
void show_jack_pot( );
void do_game( );
int num_in_arena( );
void find_game_winner( );
void do_end_game( );
void start_game( );
void silent_end( );
void write_fame_list( void );
void write_one_fame_node( FILE *fp, struct hall_of_fame_element *node );
void load_hall_of_fame( void );
void find_bet_winners( CHAR_DATA *winner );
void reset_bets( );

struct hall_of_fame_element *fame_list = NULL;

int ppl_challenged = 0;
int ppl_in_arena = 0;
int in_start_arena = 0;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int time_to_start;
int time_left_in_game;
int arena_pot;
int bet_pot;
int barena = 0;

extern int parsebet( const int currentbet, char *s );
extern int advatoi( char *s );

CMDF( do_bet )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	int newbet;

	argument = one_argument( argument, arg );
	one_argument( argument, buf1 );

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobs cant bet on the arena.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "Arena players can not make bets.", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Usage: bet <player> <amt>\r\n", ch );
		return;
	}
	else if( !in_start_arena && !ppl_challenged )
	{
		send_to_char( "Sorry the arena is closed, wait until it opens up to bet.\r\n", ch );
		return;
	}
	else if( ppl_in_arena )
	{
		send_to_char( "Sorry Arena has already started, no more bets.\r\n", ch );
		return;
	}
	else if( !( ch->betted_on = get_char_world( ch, arg ) ) )
		send_to_char( "No such person exists in the galaxy.", ch );
	else if( ch->betted_on == ch )
		send_to_char( "That doesn't make much sense, does it?\r\n", ch );
	else if( ch->in_room && !( xIS_SET( ch->betted_on->in_room->room_flags, ROOM_ARENA ) ) )
		send_to_char( "Sorry that person is not in the arena.\r\n", ch );
	else
	{
		if( GET_BET_AMT( ch ) > 0 )
		{
			send_to_char( "Sorry you have already bet.\r\n", ch );
			return;
		}
		GET_BETTED_ON( ch ) = ch->betted_on;
		newbet = parsebet( bet_pot, buf1 );
		if( newbet == 0 )
		{
			send_to_char( "Bet some gold why dont you!\r\n", ch );
			return;
		}
		if( newbet > ch->gold )
		{
			send_to_char( "You don't have that much money!\r\n", ch );
			return;
		}
		if( newbet > ARENA_MAXBET )
		{
			send_to_char( "Sorry the house will not accept that much.\r\n", ch );
			return;
		}

		ch->gold -= newbet;
		arena_pot += ( newbet / 2 );
		bet_pot += ( newbet / 2 );
		GET_BET_AMT( ch ) = newbet;
		sprintf( buf, "You place %d dollars on %s.\r\n", newbet, ch->betted_on->name );
		send_to_char( buf, ch );
		sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r %s has placed %d dollars on %s.", ch->name,
			newbet, ch->betted_on->name );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	}
}

CMDF( do_arena )
{
	char buf[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobs cant play in the arena.\r\n", ch );
		return;
	}

	if( !in_start_arena )
	{
		send_to_char( "The killing fields are closed right now.\r\n", ch );
		return;
	}

	if( ch->top_level < lo_lim )
	{
		sprintf( buf, "Sorry but you must be at least level %d to enter this arena.\r\n", lo_lim );
		send_to_char( buf, ch );
		return;
	}

	if( ch->top_level > hi_lim )
	{
		send_to_char( "This arena is for lower level characters.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "You are in the arena already\r\n", ch );
		return;
	}
	else
	{
		act( AT_RED, "$n has been whisked away to the killing fields.", ch, NULL, NULL, TO_ROOM );
		ch->retran = ch->in_room->vnum;
		char_from_room( ch );
		char_to_room( ch, get_room_index( PREP_START ) );
		act( AT_WHITE, "$n is dropped from the sky.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You have been taken to the killing fields\r\n", ch );
		do_look( ch, "auto" );
		sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r %s has joined the blood bath.", ch->name );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
		send_to_char( buf, ch );
		ch->hit = ch->max_hit;
		ch->move = ch->max_move;
		return;
	}
}

CMDF( do_chaos )
{
	char lolimit[MAX_INPUT_LENGTH];
	char hilimit[MAX_INPUT_LENGTH], start_delay[MAX_INPUT_LENGTH];
	char length[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	char purse[MAX_INPUT_LENGTH];
	/*Usage: chaos lo hi start_delay cost/lev length*/

	argument = one_argument( argument, lolimit );
	lo_lim = atoi( lolimit );
	argument = one_argument( argument, hilimit );
	hi_lim = atoi( hilimit );
	argument = one_argument( argument, start_delay );
	start_time = atoi( start_delay );
	argument = one_argument( argument, length );
	game_length = atoi( length );
	one_argument( argument, purse );
	arena_pot = atoi( purse );

	sprintf( buf, "LowLim %d HiLim %d Delay %d Length %d\r\n", lo_lim,
		hi_lim, start_time, game_length );
	send_to_char( buf, ch );

	if( hi_lim >= LEVEL_BUILDER + 1 )
	{
		send_to_char( "Please choose a hi_lim under the Imps level\r\n", ch );
		return;
	}

	if( !*lolimit || !*hilimit || !*start_delay || !*length )
	{
		send_to_char( "Syntax: chaos <Low Level> <High Level> <Delay Time> <Length of War> [Prize Money if Any]", ch );
		return;
	}

	if( lo_lim >= hi_lim )
	{
		send_to_char( "Ya that just might be smart.\r\n", ch );
		return;
	}

	if( ( lo_lim || hi_lim || game_length ) < 0 )
	{
		send_to_char( "I like positive numbers thank you.\r\n", ch );
		return;
	}

	if( start_time <= 0 )
	{
		send_to_char( "Lets at least give them a chance to enter!\r\n", ch );
		return;
	}

	ppl_in_arena = 0;
	in_start_arena = 1;
	time_to_start = start_time;
	time_left_in_game = 0;
	bet_pot = 0;
	barena = 1;
	start_arena( );

}


void start_arena( )
{
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];

	if( !( ppl_challenged ) )
	{
		if( time_to_start == 0 )
		{
			in_start_arena = 0;
			show_jack_pot( );
			ppl_in_arena = 1;    /* start the blood shed */
			time_left_in_game = game_length;
			start_game( );
		}
		else
		{
			if( time_to_start > 1 )
			{
				sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r The Killing Fields are open to levels &R%d &Wthru &R%d!", lo_lim, hi_lim );
				echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
				sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r %d &Whours to start!", time_to_start );
				echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
			}
			else
			{
				sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r The Killing Fields are open to levels &R%d &Wthru &R%d!", lo_lim, hi_lim );
				echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
				sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r 1 &Whour to start!" );
				echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
			}
			sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r Type &Rarena &rto enter." );
			echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
			sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r Place your bets!!!" );
			echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
			/* echo_to_all(AT_WHITE, buf1, ECHOTAR_ALL); */
			time_to_start--;
		}
	}
	else
		if( !( ppl_in_arena ) )
		{
			if( time_to_start == 0 )
			{
				ppl_challenged = 0;
				show_jack_pot( );
				ppl_in_arena = 1;    /* start the blood shed */
				time_left_in_game = 5;
				start_game( );
			}
			else
			{
				if( time_to_start > 1 )
				{
					sprintf( buf1, "The dual will start in %d hours. Place your bets!",
						time_to_start );
				}
				else
				{
					sprintf( buf1, "The dual will start in 1 hour. Place your bets!" );
				}
				echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
				time_to_start--;
			}
		}
}

void start_game( )
{
	CHAR_DATA *i;
	DESCRIPTOR_DATA *d;

	for( d = first_descriptor; d; d = d->next )
		if( !d->connected )
		{
			i = d->character;
			if( i == NULL )
				continue;

			if( i->in_room && xIS_SET( i->in_room->room_flags, ROOM_ARENA ) )
			{
				send_to_char( "\r\nThe floor falls out from below, dropping you in the arena.\r\n", i );
				char_from_room( i );
				char_to_room( i, get_room_index( ARENA_START ) );
				do_look( i, "auto" );
			}
		}
	do_game( );
}

void do_game( )
{
	char buf[MAX_INPUT_LENGTH];

	if( num_in_arena( ) == 1 )
	{
		ppl_in_arena = 0;
		ppl_challenged = 0;
		find_game_winner( );
	}
	else if( time_left_in_game == 0 )
	{
		do_end_game( );
	}
	else if( num_in_arena( ) == 0 )
	{
		ppl_in_arena = 0;
		ppl_challenged = 0;
		silent_end( );
	}
	else if( time_left_in_game % 5 )
	{
		sprintf( buf, "With %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena( ) );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	}
	else if( time_left_in_game == 1 )
	{
		sprintf( buf, "With 1 hour left in the game there are %d players left.", num_in_arena( ) );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	}
	else if( time_left_in_game <= 4 )
	{
		sprintf( buf, "With %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena( ) );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	}
	time_left_in_game--;
}

void find_game_winner( )
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	CHAR_DATA *i;
	DESCRIPTOR_DATA *d;
	struct hall_of_fame_element *fame_node;

	for( d = first_descriptor; d; d = d->next )
	{
		i = d->original ? d->original : d->character;

		if( i == NULL )
			continue;

		if( i->in_room && xIS_SET( i->in_room->room_flags, ROOM_ARENA )
			&& !IS_IMMORTAL( i ) )
		{
			char_from_room( i );
			char_to_room( i, get_room_index( i->retran ) );
			do_look( i, "auto" );
			act( AT_YELLOW, "$n falls from the sky.", i, NULL, NULL, TO_ROOM );
			stop_fighting( i, true );
			if( i->hit > 1 )
			{
				if( time_left_in_game == 1 )
				{
					sprintf( buf, "After 1 hour of battle %s is declared the winner", i->name );
					echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
				}
				else
				{
					sprintf( buf, "After %d hours of battle %s is declared the winner",
						game_length - time_left_in_game, i->name );
					echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
				}
				i->gold += arena_pot / 2;
				sprintf( buf, "You have been awarded %d dollars for winning the arena\r\n",
					( arena_pot / 2 ) );
				send_to_char( buf, i );
				sprintf( buf2, "%s awarded %d dollars for winning arena", i->name,
					( arena_pot / 2 ) );
				bug( buf2, 0 );
				CREATE( fame_node, struct hall_of_fame_element, 1 );
				strncpy( fame_node->name, i->name, MAX_INPUT_LENGTH );
				fame_node->name[MAX_INPUT_LENGTH] = '\0';
				fame_node->date = time( 0 );
				fame_node->award = ( arena_pot / 2 );
				fame_node->next = fame_list;
				fame_list = fame_node;
				write_fame_list( );
				find_bet_winners( i );
				ppl_in_arena = 0;
				reset_bets( );
				ppl_challenged = 0;
			}
			i->hit = i->max_hit;
			i->move = i->max_move;
			i->challenged = NULL;
		}
	}
}

void show_jack_pot( )
{
	char buf[MAX_INPUT_LENGTH];
	/*  char buf1[MAX_INPUT_LENGTH]; */

	sprintf( buf, "\r\nLets get ready to RUMBLE!!!!!!!!\r\n" );
	sprintf( buf, "The jack pot for this arena is %d dollars\r\n", arena_pot );
	sprintf( buf, "%d dollars have been bet on this arena.\r\n", bet_pot );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
}

void silent_end( )
{
	char buf[MAX_INPUT_LENGTH];
	ppl_in_arena = 0;
	ppl_challenged = 0;
	in_start_arena = 0;
	start_time = 0;
	game_length = 0;
	time_to_start = 0;
	time_left_in_game = 0;
	arena_pot = 0;
	bet_pot = 0;
	sprintf( buf, "It looks like no one was brave enough to enter the Arena." );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	reset_bets( );
}

void do_end_game( )
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *i;
	DESCRIPTOR_DATA *d;

	for( d = first_descriptor; d; d = d->next )
		if( !d->connected )
		{
			i = d->character;

			if( i == NULL )
				continue;

			if( i->in_room && xIS_SET( i->in_room->room_flags, ROOM_ARENA ) )
			{
				i->hit = i->max_hit;
				i->move = i->max_move;
				i->challenged = NULL;
				stop_fighting( i, true );
				char_from_room( i );
				char_to_room( i, get_room_index( i->retran ) );
				do_look( i, "auto" );
				act( AT_TELL, "$n falls from the sky.", i, NULL, NULL, TO_ROOM );
			}
		}
	sprintf( buf, "After %d hours of battle the Match is a draw.", game_length );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	time_left_in_game = 0;
	ppl_in_arena = 0;
	ppl_challenged = 0;
	reset_bets( );
}

int num_in_arena( )
{
	CHAR_DATA *i;
	DESCRIPTOR_DATA *d;
	int num = 0;

	for( d = first_descriptor; d; d = d->next )
	{
		i = d->original ? d->original : d->character;
		if( i == NULL )
			continue;

		if( i->in_room && xIS_SET( i->in_room->room_flags, ROOM_ARENA ) )
		{
			if( !IS_IMMORTAL( i )
				&& i->hit > 1 )
				num++;
		}
	}
	return num;
}

/*void sportschan(char *argument)
{
  char buf1[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *i;

  sprintf(buf1, "&R[Arena] &W%s\r\n", argument);

  for (i = first_descriptor; i; i = i->next)
  {
	if (!i->connected && i->character)
	{
	   send_to_char(buf1, i->character);
	}
  }
}*/

CMDF( do_awho )
{
	CHAR_DATA *tch;
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int num = num_in_arena( );

	if( num == 0 )
	{
		send_to_char( "There is noone in the arena right now.\r\n", ch );
		return;
	}

	sprintf( buf, "&W  Players in the Arena\r\n" );
	sprintf( buf, "%s-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-", buf );
	sprintf( buf, "%s&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-\r\n", buf );
	sprintf( buf, "%sGame Length = &R%-3d   &WTime To Start &R%-3d\r\n", buf, game_length, time_to_start );
	sprintf( buf, "%s&WLevel Limits &R%d &Wto &R%d\r\n", buf, lo_lim, hi_lim );
	sprintf( buf, "%s         &WJackpot = &R%d\r\n", buf, arena_pot );
	sprintf( buf, "%s&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B", buf );
	sprintf( buf, "%s-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B\r\n", buf );
	send_to_char( buf, ch );

	for( tch = first_char; tch; tch = tch->next )
		if( tch->in_room && xIS_SET( tch->in_room->room_flags, ROOM_ARENA )
			&& ( tch->top_level < LEVEL_STAFF ) )
		{
			sprintf( buf2, "&W%s\r\n", tch->name );
			send_to_char( buf2, ch );
		}
	return;
}

CMDF( do_ahall )
{
	char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
	char format2[MAX_INPUT_LENGTH];
	struct hall_of_fame_element *fame_node;

	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	if( !fame_list )
	{
		send_to_char( "No-one is in the Hall of Fame.\r\n", ch );
		return;
	}

	sprintf( buf2, "&B|---------------------------------------|\r\n" );
	strcat( buf2, "| &WPast Winners of The Rise in Power Arena&B  |\r\n" );
	strcat( buf2, "|---------------------------------------|\r\r\n\n" );

	send_to_char( buf2, ch );
	strcpy( format, "%-25.25s  %-10.10s  %-16.16s\r\n" );
	sprintf( buf, format,
		"&RName",
		"&RDate",
		"&RAward Amt" );
	send_to_char( buf, ch );
	sprintf( buf, format,
		"&B---------------------------------",
		"&B---------------------------------",
		"&B---------------------------------" );

	send_to_char( buf, ch );
	strcpy( format2, "&W%-25.25s  &R%-10.10s  &Y%-16d\r\n" );
	for( fame_node = fame_list; fame_node; fame_node = fame_node->next )
	{
		if( fame_node->date )
		{
			timestr = asctime( localtime( &( fame_node->date ) ) );
			*( timestr + 10 ) = 0;
			strcpy( site, timestr );
		}
		else
			strcpy( site, "Unknown" );
		sprintf( buf, format2, fame_node->name, site, fame_node->award );
		send_to_char( buf, ch );
	}
	return;
}

void load_hall_of_fame( void )
{
	FILE *fl;
	int date, award;
	char name[MAX_INPUT_LENGTH + 1];
	struct hall_of_fame_element *next_node;

	fame_list = 0;

	if( !( fl = FileOpen( HALL_FAME_FILE, "r" ) ) )
	{
		perror( "Unable to open hall of fame file" );
		return;
	}
	while( fscanf( fl, "%s %d %d", name, &date, &award ) == 3 )
	{
		CREATE( next_node, struct hall_of_fame_element, 1 );
		strncpy( next_node->name, name, MAX_INPUT_LENGTH );
		next_node->date = date;
		next_node->award = award;
		next_node->next = fame_list;
		fame_list = next_node;
	}

	FileClose( fl );
	return;
}

void write_fame_list( void )
{
	FILE *fl;

	if( !( fl = FileOpen( HALL_FAME_FILE, "w" ) ) )
	{
		bug( "Error writing _hall_of_fame_list", 0 );
		return;
	}
	write_one_fame_node( fl, fame_list );/* recursively write from end to start */
	FileClose( fl );

	return;
}

void write_one_fame_node( FILE *fp, struct hall_of_fame_element *node )
{
	if( node )
	{
		write_one_fame_node( fp, node->next );
		fprintf( fp, "%s %ld %d\n", node->name, ( long ) node->date, node->award );
	}
}

void find_bet_winners( CHAR_DATA *winner )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;

	char buf1[MAX_INPUT_LENGTH];

	for( d = first_descriptor; d; d = d->next )
		if( !d->connected )
		{
			wch = d->original ? d->original : d->character;

			if( wch == NULL )
				continue;

			if( ( !IS_NPC( wch ) ) && ( GET_BET_AMT( wch ) > 0 ) && ( GET_BETTED_ON( wch ) == winner ) )
			{
				sprintf( buf1, "You have won %d dollars on your bet.\r\n", ( GET_BET_AMT( wch ) ) * 2 );
				send_to_char( buf1, wch );
				wch->gold += GET_BET_AMT( wch ) * 2;
				GET_BETTED_ON( wch ) = NULL;
				GET_BET_AMT( wch ) = 0;
			}
		}
}

CMDF( do_challenge )
{
	CHAR_DATA *victim;
	char buf[MAX_INPUT_LENGTH];

	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "&WThat character is not of these realms!\r\n", ch );
		return;
	}

	if( IS_IMMORTAL( ch ) || IS_IMMORTAL( victim ) )
	{
		send_to_char( "Sorry, Immortal's are not allowed to participate in the arena.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "&WYou cannot challenge mobiles!\r\n", ch );
		return;
	}

	if( victim->name == ch->name )
	{
		send_to_char( "&WYou cannot challenge yourself!", ch );
		return;
	}

	if( victim->top_level < 5 )
	{
		send_to_char( "&WThat character is too young.\r\n", ch );
		return;
	}

	if( ( !( ch->top_level - 15 < victim->top_level ) ) || ( !( ch->top_level + 15 > victim->top_level ) ) )
	{
		send_to_char( "&WThat character is out of your level range.\r\n", ch );
		return;
	}

	if( get_timer( victim, TIMER_PKILLED ) > 0 )
	{
		send_to_char( "&WThat player has died within the last 5 minutes and cannot be challenged!\r\n", ch );
		return;
	}

	if( victim->top_level < 5 )
	{
		send_to_char( "You are too young to die.\r\n", ch );
		return;
	}

	if( get_timer( ch, TIMER_PKILLED ) > 0 )
	{
		send_to_char( "&WYou have died within the last 5 minutes and cannot challenge anyone.\r\n", ch );
		return;
	}

	if( num_in_arena( ) > 0 )
	{
		send_to_char( "&WSomeone is already in the arena!\r\n", ch );
		return;
	}
	sprintf( buf, "&R%s &Whas challenged you to a dual!\r\n", ch->name );
	send_to_char( buf, victim );
	send_to_char( "&WPlease either agree to, or decline the challenge.\r\n\r\n", victim );
	sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&r %s has challenged %s to a dual!!\r\n", ch->name, victim->name );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	victim->challenged = ch;
}

CMDF( do_agree )
{
	char buf[MAX_INPUT_LENGTH];

	if( num_in_arena( ) > 0 )
	{
		send_to_char( "Please wait until the current arena is closed before you agree.\r\n", ch );
		return;
	}

	if( !( ch->challenged ) )
	{
		send_to_char( "You have not been challenged!\r\n", ch );
		return;
	}
	else
	{
		CHAR_DATA *dch;
		dch = ch->challenged;

		if( !dch || !( dch->in_room ) || !( dch->name ) || ( ch->name[0] == '\0' ) )
			return;

		if( dch->in_room == ch->in_room )
		{
			send_to_char( "You must be in a different room as your challenger.\r\n", ch );
		}

		if( dch->in_room == ch->in_room )
		{
			send_to_char( "You must be in a different room as your challenger.\r\n", ch );
			return;
		}

		sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&G %s &CH&cA&CS&G accepted %s's challenge! &C(&zYAY&W! &B^^&R_&B^^&C)\r\n", ch->name, dch->name );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
		ch->challenged = NULL;
		ch->retran = ch->in_room->vnum;
		char_from_room( ch );
		char_to_room( ch, get_room_index( PREP_END ) );
		do_look( ch, "auto" );
		dch->retran = dch->in_room->vnum;
		char_from_room( dch );
		char_to_room( dch, get_room_index( PREP_START ) );
		do_look( dch, "auto" );
		ppl_in_arena = 0;
		ppl_challenged = 1;
		time_to_start = 3;
		time_left_in_game = 0;
		arena_pot = 0;
		bet_pot = 0;
		start_arena( );
		return;
	}
}

CMDF( do_decline )
{
	char buf[MAX_INPUT_LENGTH];

	if( ch->challenged )
	{
		sprintf( buf, "&R[&r|&PA&pr&re&pn&Pa&r|&R]&G %s did &CN&cO&CT&G accept %s's challenge! &C(&zCoward&W! &B^^&R_&B^^&C)\r\n", ch->name, ch->challenged->name );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
		ch->challenged = NULL;
		return;
	}
	else
	{
		send_to_char( "You have not been challenged!\r\n", ch );
		return;
	}
}

/*
 * Reset bets for those that did not win.
 * Added by Ulysses, rewritten by Darrik Vequir.
 */
void reset_bets( )
{
	CHAR_DATA *ch;

	for( ch = first_char; ch; ch = ch->next )
	{
		if( ch == NULL )
			continue;

		if( !IS_NPC( ch ) )
		{
			GET_BETTED_ON( ch ) = NULL;
			GET_BET_AMT( ch ) = 0;
		}
	}
}
