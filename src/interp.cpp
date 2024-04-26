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
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 * Externals
 */

void subtract_times( struct timeval *etime, struct timeval *stime );
bool check_social( CHAR_DATA *ch, const char *command, const char *argument );
bool check_alias( CHAR_DATA *ch, const char *command, const char *argument );
char last_command[MAX_STRING_LENGTH];


/*
 * Log-all switch.
 */
bool fLogAll = false;


CMDTYPE *command_hash[126]; /* hash table for cmd_table */
SOCIALTYPE *social_index[27];   /* hash table for socials   */

						  /*
 * Character not in position for command?
 */
bool check_pos( CHAR_DATA *ch, short position )
{
	if( ch->position < position )
	{
		switch( ch->position )
		{
		case POS_DEAD:
			send_to_char( "A little difficult to do when you are DEAD...\r\n", ch );
			break;

		case POS_MORTAL:
		case POS_INCAP:
			send_to_char( "You are hurt far too bad for that.\r\n", ch );
			break;

		case POS_STUNNED:
			send_to_char( "You are too stunned to do that.\r\n", ch );
			break;

		case POS_SLEEPING:
			send_to_char( "In your dreams, or what?\r\n", ch );
			break;

		case POS_RESTING:
			send_to_char( "Nah... You feel too relaxed...\r\n", ch );
			break;

		case POS_SITTING:
			send_to_char( "You can't do that sitting down.\r\n", ch );
			break;

		case POS_FIGHTING:
			send_to_char( "No way!  You are still fighting!\r\n", ch );
			break;

		}
		return false;
	}
	return true;
}

extern char lastplayercmd[MAX_INPUT_LENGTH * 2];

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, const char *argument )
{
	char command[MAX_INPUT_LENGTH];
	char logline[MAX_INPUT_LENGTH];
	char logname[MAX_INPUT_LENGTH];
	TIMER *timer = NULL;
	CMDTYPE *cmd = NULL;
	int trust;
	int loglvl;
	bool found;
	struct timeval time_used;
	long tmptime;
	int messege;
	messege = number_range( 0, 7 );


	if( !ch )
	{
		bug( "interpret: null ch!", 0 );
		return;
	}

	found = false;
	if( ch->substate == SUB_REPEATCMD )
	{
		DO_FUN *fun;

		if( ( fun = ch->last_cmd ) == NULL )
		{
			ch->substate = SUB_NONE;
			bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
			return;
		}
		else
		{
			int x;

			/*
			 * yes... we lose out on the hashing speediness here...
			 * but the only REPEATCMDS are wizcommands (currently)
			 */
			for( x = 0; x < 126; x++ )
			{
				for( cmd = command_hash[x]; cmd; cmd = cmd->next )
					if( cmd->do_fun == fun )
					{
						found = true;
						break;
					}
				if( found )
					break;
			}
			if( !found )
			{
				cmd = NULL;
				bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
				return;
			}
			snprintf( logline, MAX_INPUT_LENGTH, "(%s) %s", cmd->name, argument );
		}
	}

	if( !cmd )
	{
		/*
		 * Changed the order of these ifchecks to prevent crashing.
		 */
		if( !argument || !strcmp( argument, "" ) )
		{
			bug( "interpret: null argument!", 0 );
			return;
		}

		/*
		 * Strip leading spaces.
		 */
		while( isspace( *argument ) )
			argument++;
		if( argument[0] == '\0' )
			return;

		timer = get_timerptr( ch, TIMER_DO_FUN );

		/*
		 * REMOVE_BIT( ch->affected_by, AFF_HIDE );
		 */

		 /*
		  * Implement freeze command.
		  */
		if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_FREEZE ) )
		{
			send_to_char( "You're totally frozen!\r\n", ch );
			return;
		}

		/*
		 * Grab the command word.
		 * Special parsing so ' can be a command,
		 *   also no spaces needed after punctuation.
		 */
		mudstrlcpy( logline, argument, MAX_INPUT_LENGTH );
		if( !isalpha( argument[0] ) && !isdigit( argument[0] ) )
		{
			command[0] = argument[0];
			command[1] = '\0';
			argument++;
			while( isspace( *argument ) )
				argument++;
		}
		else
			argument = one_argument( argument, command );

		/*
		 * Look for command in command table.
		 * Check for council powers and/or bestowments
		 */
		trust = get_trust( ch );
		for( cmd = command_hash[LOWER( command[0] ) % 126]; cmd; cmd = cmd->next )
			if( !str_prefix( command, cmd->name )
				&& ( cmd->level <= trust
					|| ( !IS_NPC( ch ) && ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0'
						&& is_name( cmd->name, ch->pcdata->bestowments ) && cmd->level <= ( trust + 5 ) ) ) )
			{
				found = true;
				break;
			}

		/*
		 * Turn off afk bit when any command performed.
		 */
		if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AFK ) && ( str_cmp( command, "AFK" ) ) )
		{
			xREMOVE_BIT( ch->act, PLR_AFK );
			act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
			if( ch->pcdata->tells >= 1 )
				ch_printf( ch, "\n&GYou have &g%d&G messeges waiting on your CDI.\r&GType &g'&Gcheck&g'&G, to read them.\r\n",
					ch->pcdata->tells );
		}
	}

	/*
	 * Log and snoop.
	 */
	snprintf( lastplayercmd, MAX_INPUT_LENGTH * 2, "** %s: %s", ch->name, logline );

	if( found && cmd->log == LOG_NEVER )
		mudstrlcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX", MAX_INPUT_LENGTH );

	loglvl = found ? cmd->log : LOG_NORMAL;

	if( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LOG ) )
		|| fLogAll || loglvl == LOG_BUILD || loglvl == LOG_HIGH || loglvl == LOG_ALWAYS )
	{
		/*
		 * Added by Narn to show who is switched into a mob that executes
		 * a logged command.  Check for descriptor in case force is used.
		 */
		if( ch->desc && ch->desc->original )
			snprintf( log_buf, MAX_STRING_LENGTH, "Log %s (%s): %s", ch->name, ch->desc->original->name, logline );
		else
			snprintf( log_buf, MAX_STRING_LENGTH, "Log %s: %s", ch->name, logline );

		/*
		 * Make it so a 'log all' will send most output to the log
		 * file only, and not spam the log channel to death -Thoric
		 */
		if( fLogAll && loglvl == LOG_NORMAL && ( IS_NPC( ch ) || !xIS_SET( ch->act, PLR_LOG ) ) )
			loglvl = LOG_ALL;

		/*
		 * This is handled in get_trust already
		 */
		 /*	if ( ch->desc && ch->desc->original )
			   log_string_plus( log_buf, loglvl,
				 ch->desc->original->level );
			 else*/
		log_string_plus( log_buf, loglvl, get_trust( ch ) );
	}

	if( ch->desc && ch->desc->snoop_by )
	{
		snprintf( logname, MAX_INPUT_LENGTH, "%s", ch->name );
		write_to_buffer( ch->desc->snoop_by, logname, 0 );
		write_to_buffer( ch->desc->snoop_by, "% ", 2 );
		write_to_buffer( ch->desc->snoop_by, logline, 0 );
		write_to_buffer( ch->desc->snoop_by, "\r\n", 2 );
	}

	if( timer )
	{
		int tempsub;

		tempsub = ch->substate;
		ch->substate = SUB_TIMER_DO_ABORT;
		( timer->do_fun ) ( ch, "" );
		if( char_died( ch ) )
			return;
		if( ch->substate != SUB_TIMER_CANT_ABORT )
		{
			ch->substate = tempsub;
			extract_timer( ch, timer );
		}
		else
		{
			ch->substate = tempsub;
			return;
		}
	}

	/*
	 * Look for command in skill and socials table.
	 */
	if( !found )
	{
		if( !check_skill( ch, command, argument )
			&& !check_alias( ch, command, argument ) && !check_social( ch, command, argument ) )
		{
			EXIT_DATA *pexit;

			/*
			 * check for an auto-matic exit command
			 */
			if( ( pexit = find_door( ch, command, true ) ) != NULL && IS_SET( pexit->exit_info, EX_xAUTO ) )
			{
				if( IS_SET( pexit->exit_info, EX_CLOSED )|| IS_SET( pexit->exit_info, EX_NOPASSDOOR ) )
				{
					if( !IS_SET( pexit->exit_info, EX_SECRET ) )
						act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
					else
						send_to_char( "You cannot do that here.\r\n", ch );
					return;
				}
				if( check_pos( ch, POS_STANDING ) )
					move_char( ch, pexit, 0 );
				return;
			}
			if( messege == 1 )
			{
				send_to_char( "Type much?\r\n", ch );
			}
			else if( messege == 2 )
			{
				send_to_char( "If you're going to type with one hand, do it right!\r\n", ch );
			}
			else if( messege == 3 )
			{
				send_to_char( "Maybe you missed a letter..?\r\n", ch );
			}
			else if( messege == 4 )
			{
				send_to_char( "Computer: $2000   Mouse: $10   Moniter: $600   Working Keyboard: Priceless.\r\n", ch );
			}
			else if( messege == 5 )
			{
				send_to_char( "Hhuh? <-- That shares a similarity with what you just typed.\r\n", ch );
			}
			else if( messege == 6 )
			{
				send_to_char( "Bad Typo! BAD!\r\n", ch );
			}
			else if( messege == 7 )
			{
				send_to_char( "If you are reading this... you entered the wrong command =/\r\n", ch );
			}
			else
			{
				send_to_char( "Huh?\r\n", ch );
			}
		}
		return;
	}

	/*
	 * Character not in position for command?
	 */
	if( !check_pos( ch, cmd->position ) )
		return;

	/*
	 * Berserk check for flee.. maybe add drunk to this?.. but too much
	 * hardcoding is annoying.. -- Altrag
	 */
	if( !str_cmp( cmd->name, "flee" ) && IS_AFFECTED( ch, AFF_BERSERK ) )
	{
		send_to_char( "You aren't thinking very clearly..\r\n", ch );
		return;
	}

	/*
	 * Dispatch the command.
	 */
	ch->prev_cmd = ch->last_cmd; /* haus, for automapping */
	ch->last_cmd = cmd->do_fun;
	start_timer( &time_used );
	( *cmd->do_fun ) ( ch, argument );
	end_timer( &time_used );
	/*
	 * Update the record of how many times this command has been used (haus)
	 */
	update_userec( &time_used, &cmd->userec );
	tmptime = UMIN( time_used.tv_sec, 19 ) * 1000000 + time_used.tv_usec;

	/*
	 * laggy command notice: command took longer than 1.5 seconds
	 */
	if( tmptime > 1500000 )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "[*****] LAG: %s: %s %s (R:%d S:%d.%06d)", ch->name,
			cmd->name, ( cmd->log == LOG_NEVER ? "XXX" : argument ),
			ch->in_room ? ch->in_room->vnum : 0, ( int ) ( time_used.tv_sec ), ( int ) ( time_used.tv_usec ) );
		log_string_plus( log_buf, LOG_NORMAL, get_trust( ch ) );
	}
	mudstrlcpy( lastplayercmd, "No commands pending", MSL );
	tail_chain( );
}

CMDTYPE *find_command( const char *command )
{
	CMDTYPE *cmd;
	int hash;

	hash = LOWER( command[0] ) % 126;

	for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
		if( !str_prefix( command, cmd->name ) )
			return cmd;

	return NULL;
}

SOCIALTYPE *find_social( const char *command )
{
	SOCIALTYPE *social;
	int hash;

	char c = LOWER( command[0] );

	if( c < 'a' || c > 'z' )
		hash = 0;
	else
		hash = ( c - 'a' ) + 1;

	for( social = social_index[hash]; social; social = social->next )
		if( !str_prefix( command, social->name ) )
			return social;

	return NULL;
}

bool check_social( CHAR_DATA *ch, const char *command, const char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim, *victim_next;
	SOCIALTYPE *social;
	CHAR_DATA *removed[128]; /* What are the chances of more than 128? */
	ROOM_INDEX_DATA *room;
	int i = 0, k = 0;

	if( ( social = find_social( command ) ) == NULL )
		return false;

	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
	{
		send_to_char( "You are anti-social!\r\n", ch );
		return true;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still; you are DEAD.\r\n", ch );
		return true;

	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return true;

	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return true;

	case POS_SLEEPING:
		/*
		 * I just know this is the path to a 12" 'if' statement.  :(
		 * But two players asked for it already!  -- Furey
		 */
		if( !str_cmp( social->name, "snore" ) )
			break;
		send_to_char( "In your dreams, or what?\r\n", ch );
		return true;

	}

	/*
	 * Search room for chars ignoring social sender and
	 */
	 /*
	  * remove them from the room until social has been
	  */
	  /*
	   * completed
	   */
	room = ch->in_room;
	for( victim = ch->in_room->first_person; victim; victim = victim_next )
	{
		if( i == 127 )
			break;
		victim_next = victim->next_in_room;
		if( is_ignoring( victim, ch ) )
		{
			if( !IS_IMMORTAL( ch ) || get_trust( victim ) > get_trust( ch ) )
			{
				removed[i] = victim;
				i++;
				UNLINK( victim, room->first_person, room->last_person, next_in_room, prev_in_room );
			}
			else
			{
				set_char_color( AT_WHITE, victim );
				ch_printf( victim, "You attempt to ignore %s, but are unable to do so.\r\n", ch->name );
			}
		}
	}


	one_argument( argument, arg );
	victim = NULL;
	if( arg[0] == '\0' )
	{
		act( AT_DGREY, social->others_no_arg, ch, NULL, victim, TO_ROOM );
		act( AT_DGREY, social->char_no_arg, ch, NULL, victim, TO_CHAR );
	}
	else if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		/*
		 * If they aren't in the room, they may be in the list of
		 */
		 /*
		  * people ignoring...
		  */
		if( i != 0 )
		{
			for( k = 0, victim = removed[0]; k < i; k++, victim = removed[k] )
			{
				if( nifty_is_name( victim->name, arg ) || nifty_is_name_prefix( arg, victim->name ) )
				{
					set_char_color( AT_WHITE, ch );
					ch_printf( ch, "%s is ignoring you.\r\n", victim->name );
					break;
				}
			}
		}

		if( !victim )
			send_to_char( "They aren't here.\r\n", ch );
	}
	else if( victim == ch )
	{
		act( AT_DGREY, social->others_auto, ch, NULL, victim, TO_ROOM );
		act( AT_DGREY, social->char_auto, ch, NULL, victim, TO_CHAR );

	}
	else
	{
		act( AT_DGREY, social->others_found, ch, NULL, victim, TO_NOTVICT );
		act( AT_DGREY, social->char_found, ch, NULL, victim, TO_CHAR );
		act( AT_DGREY, social->vict_found, ch, NULL, victim, TO_VICT );

		if( !IS_NPC( ch ) && IS_NPC( victim )
			&& !IS_AFFECTED( victim, AFF_CHARM ) && IS_AWAKE( victim ) && !HAS_PROG( victim->pIndexData, ACT_PROG ) )
		{
			switch( number_bits( 4 ) )
			{
			case 0:
				if( !xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) || IS_EVIL( ch ) )
					multi_hit( victim, ch, TYPE_UNDEFINED );
				else if( IS_NEUTRAL( ch ) )
				{
					act( AT_ACTION, "$n slaps $N.", victim, NULL, ch, TO_NOTVICT );
					act( AT_ACTION, "You slap $N.", victim, NULL, ch, TO_CHAR );
					act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT );
				}
				else
				{
					act( AT_ACTION, "$n acts like $N doesn't even exist.", victim, NULL, ch, TO_NOTVICT );
					act( AT_ACTION, "You just ignore $N.", victim, NULL, ch, TO_CHAR );
					act( AT_ACTION, "$n appears to be ignoring you.", victim, NULL, ch, TO_VICT );
				}
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				act( AT_DGREY, social->others_found, victim, NULL, ch, TO_NOTVICT );
				act( AT_DGREY, social->char_found, victim, NULL, ch, TO_CHAR );
				act( AT_DGREY, social->vict_found, victim, NULL, ch, TO_VICT );
				break;

			case 9:
			case 10:
			case 11:
			case 12:
				act( AT_ACTION, "$n slaps $N.", victim, NULL, ch, TO_NOTVICT );
				act( AT_ACTION, "You slap $N.", victim, NULL, ch, TO_CHAR );
				act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT );
				break;
			}
		}

	}

	/*
	 * Replace the chars in the ignoring list to the room
	 * note that the ordering of the players in the room
	 * might change
	 */
	if( i != 0 )
	{
		for( k = 0, victim = removed[0]; k < i; k++, victim = removed[k] )
		{
			LINK( victim, room->first_person, room->last_person, next_in_room, prev_in_room );
		}
	}

	return true;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( const char *arg )
{
	if( *arg == '\0' )
		return false;

	for( ; *arg != '\0'; arg++ )
	{
		if( !isdigit( *arg ) )
			return false;
	}

	return true;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( const char *argument, char *arg )
{
	const char *pdot;
	int number;

	for( pdot = argument; *pdot != '\0'; pdot++ )
	{
		if( *pdot == '.' )
		{
			char *numPortion = ( char * ) malloc( pdot - argument + 1 );
			memcpy( numPortion, argument, pdot - argument );
			numPortion[pdot - argument] = '\0';

			number = atoi( numPortion );

			free( numPortion );

			mudstrlcpy( arg, pdot + 1, MAX_INPUT_LENGTH );
			return number;
		}
	}

	mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );
	return 1;
}

char *one_argument( char *argument, char *arg_first )
{
	return ( char * ) one_argument( ( const char * ) argument, arg_first );
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes. No longer mangles case either. That used to be annoying.
 */
const char *one_argument( const char *argument, char *arg_first )
{
	char cEnd;
	int count;

	count = 0;

	while( isspace( *argument ) )
		argument++;

	cEnd = ' ';
	if( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while( *argument != '\0' || ++count >= 255 )
	{
		if( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = ( *argument );
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while( isspace( *argument ) )
		argument++;

	return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 * No longer mangles case either. That used to be annoying.
 */
const char *one_argument2( const char *argument, char *arg_first )
{
	char cEnd;
	short count;

	count = 0;

	if( !argument || argument[0] == '\0' )
	{
		arg_first[0] = '\0';
		return argument;
	}

	while( isspace( *argument ) )
		argument++;

	cEnd = ' ';
	if( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while( *argument != '\0' || ++count >= 255 )
	{
		if( *argument == cEnd || *argument == '-' )
		{
			argument++;
			break;
		}
		*arg_first = ( *argument );
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while( isspace( *argument ) )
		argument++;

	return argument;
}

CMDF( do_timecmd )
{
	struct timeval stime;
	struct timeval etime;
	static bool timing;
	extern CHAR_DATA *timechar;
	char arg[MAX_INPUT_LENGTH];

	send_to_char( "Timing\r\n", ch );
	if( timing )
		return;
	one_argument( argument, arg );
	if( !*arg )
	{
		send_to_char( "No command to time.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "update" ) )
	{
		if( timechar )
			send_to_char( "Another person is already timing updates.\r\n", ch );
		else
		{
			timechar = ch;
			send_to_char( "Setting up to record next update loop.\r\n", ch );
		}
		return;
	}
	set_char_color( AT_PLAIN, ch );
	send_to_char( "Starting timer.\r\n", ch );
	timing = true;
	gettimeofday( &stime, NULL );
	interpret( ch, argument );
	gettimeofday( &etime, NULL );
	timing = false;
	set_char_color( AT_PLAIN, ch );
	send_to_char( "Timing complete.\r\n", ch );
	subtract_times( &etime, &stime );
	ch_printf( ch, "Timing took %d.%06d seconds.\r\n", etime.tv_sec, etime.tv_usec );
	return;
}

void start_timer( struct timeval *stime )
{
	if( !stime )
	{
		bug( "Start_timer: NULL stime.", 0 );
		return;
	}
	gettimeofday( stime, NULL );
	return;
}

time_t end_timer( struct timeval *stime )
{
	struct timeval etime;

	/*
	 * Mark etime before checking stime, so that we get a better reading..
	 */
	gettimeofday( &etime, NULL );
	if( !stime || ( !stime->tv_sec && !stime->tv_usec ) )
	{
		bug( "End_timer: bad stime.", 0 );
		return 0;
	}
	subtract_times( &etime, stime );
	/*
	 * stime becomes time used
	 */
	*stime = etime;
	return ( etime.tv_sec * 1000000 ) + etime.tv_usec;
}

void send_timer( struct timerset *vtime, CHAR_DATA *ch )
{
	struct timeval ntime;
	int carry;

	if( vtime->num_uses == 0 )
		return;
	ntime.tv_sec = vtime->total_time.tv_sec / vtime->num_uses;
	carry = ( vtime->total_time.tv_sec % vtime->num_uses ) * 1000000;
	ntime.tv_usec = ( vtime->total_time.tv_usec + carry ) / vtime->num_uses;
	ch_printf( ch, "Has been used %d times this boot.\r\n", vtime->num_uses );
	ch_printf( ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
		"\r\n", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
		ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec );
	return;
}

void update_userec( struct timeval *time_used, struct timerset *userec )
{
	userec->num_uses++;
	if( !timerisset( &userec->min_time ) || timercmp( time_used, &userec->min_time, < ) )
	{
		userec->min_time.tv_sec = time_used->tv_sec;
		userec->min_time.tv_usec = time_used->tv_usec;
	}
	if( !timerisset( &userec->max_time ) || timercmp( time_used, &userec->max_time, > ) )
	{
		userec->max_time.tv_sec = time_used->tv_sec;
		userec->max_time.tv_usec = time_used->tv_usec;
	}
	userec->total_time.tv_sec += time_used->tv_sec;
	userec->total_time.tv_usec += time_used->tv_usec;
	while( userec->total_time.tv_usec >= 1000000 )
	{
		userec->total_time.tv_sec++;
		userec->total_time.tv_usec -= 1000000;
	}
	return;
}

RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;
MNAME_DATA *first_mname;
MNAME_DATA *last_mname;
FNAME_DATA *first_fname;
FNAME_DATA *last_fname;

void save_reserved( void );
void sort_reserved( RESERVE_DATA *pRes );

/* Modified to require an "add" or "remove" argument in addition to name -
Samson 10-18-98 */
CMDF( do_reserve )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	RESERVE_DATA *res;
	FNAME_DATA *fname;
	FNAME_DATA *rndm = NULL;
	int count;

	set_char_color( AT_PLAIN, ch );

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( arg[0] == '\0' )
	{
		int wid = 0;

		send_to_char( "To add a name: reserve <name> add\n\rTo remove a name: reserve <name> remove\r\n", ch );
		send_to_char( "\r\n-- Reserved Names --\r\n", ch );
		for( res = first_reserved; res; res = res->next )
		{
			ch_printf( ch, "%c%-17s ", ( *res->name == '*' ? '*' : ' ' ), ( *res->name == '*' ? res->name + 1 : res->name ) );
			if( ++wid % 4 == 0 )
				send_to_char( "\r\n", ch );
		}
		if( wid % 4 != 0 )
			send_to_char( "\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "test" ) )
	{
		count = 0;
		for( fname = first_fname; fname; fname = fname->next )
		{
			if( number_range( 0, count ) == 0 )
				rndm = fname;
			count++;
		}

		ch_printf( ch, "Random Name: &R|&w%s&R|\r\n", rndm->name );
		return;
	}

	if( !str_cmp( arg2, "remove" ) )
	{
		for( res = first_reserved; res; res = res->next )
		{
			if( !str_cmp( arg, res->name ) )
			{
				UNLINK( res, first_reserved, last_reserved, next, prev );
				DISPOSE( res->name );
				DISPOSE( res );
				save_reserved( );
				send_to_char( "Name no longer reserved.\r\n", ch );
				return;
			}
		}
		ch_printf( ch, "The name %s isn't on the reserved list.\r\n", arg );
		return;
	}

	if( !str_cmp( arg2, "add" ) )
	{
		for( res = first_reserved; res; res = res->next )
		{
			if( !str_cmp( arg, res->name ) )
			{
				ch_printf( ch, "The name %s has already been reserved.\r\n", arg );
				return;
			}
		}

		CREATE( res, RESERVE_DATA, 1 );
		res->name = str_dup( arg );
		sort_reserved( res );
		save_reserved( );
		send_to_char( "Name reserved.\r\n", ch );
		return;
	}
	send_to_char( "Invalid argument.\r\n", ch );
}

/*
void do_gocial( CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  SOCIALTYPE *social;
  char command[MAX_INPUT_LENGTH];
	int counter;
	int count;
	char buf2[MAX_STRING_LENGTH];



	argument = one_argument( argument , command );

	if ( command[0] == '\0' )
	{
	   send_to_char( "Gocial what?\r\n", ch );
	   return;
	}

	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if (!IS_NPC(ch) && ( xIS_SET(ch->act, PLR_SILENCE) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

  if ( (social=find_social(command)) == NULL)
	{
	  send_to_char ("Pick a correct social to use.\r\n", ch);
	  return;
	}

	if ( IS_SET( ch->deaf, CHANNEL_GOCIAL )
	&& !IS_IMMORTAL( ch ) )
	{
	  act( AT_PLAIN, "You have Gocial turned off... try chan +gocial first.", ch, NULL, NULL,
		TO_CHAR );
	  return;
	}

  switch (ch->position)
	{
	case POS_DEAD:
	  send_to_char ("Lie still; you are DEAD.\r\n", ch);
	  return;
	case POS_INCAP:
	case POS_MORTAL:
	  send_to_char ("You are hurt far too bad for that.\r\n", ch);
	  return;
	case POS_STUNNED:
	  send_to_char ("You are too stunned to do that.\r\n", ch);
	  return;
	case POS_SLEEPING:
	  if (!str_cmp (social->name, "snore"))
	break;
	  send_to_char ("In your dreams, or what?\r\n", ch);
	  return;
	}

  one_argument (argument, arg);
  victim = NULL;

  if (arg[0] == '\0')
	{
   sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->others_no_arg);
	for (d = first_descriptor; d != NULL; d = d->next)
	{
		CHAR_DATA *vch;
		vch = d->original ? d->original : d->character;
		if (d->connected == CON_PLAYING &&
		d->character != ch &&
		!IS_SET(vch->deaf, CHANNEL_GOCIAL) )
		{
		act(AT_SOCIAL, buf,ch,NULL,vch,TO_VICT );
		}
	}

   sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_no_arg);
   act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
   return;
	}

  if ( ( (victim = get_char_world (ch, arg)) == NULL)
  || (!can_see(ch, victim) ) )
	{
	  send_to_char ("That person isn't logged on!\r\n", ch);
	  return;
	}

  if (IS_NPC(victim))
	{
	  send_to_char ("That person isn't logged on!\r\n", ch);
	  return;
	}

  if (victim == ch)
	{
	  sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->others_auto);
		for (d = first_descriptor; d != NULL; d = d->next)
		{
			CHAR_DATA *vch;
			vch = d->original ? d->original : d->character;
			if (d->connected == CON_PLAYING &&
				d->character != ch &&
				!IS_SET(vch->deaf, CHANNEL_GOCIAL) )
			{
				act(AT_SOCIAL, buf,ch,NULL,vch,TO_VICT );
			}
		}

	  sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_auto);
	  act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
	  return;
	}
  else
	{
	  sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->others_found);
	for (counter = 0; buf[counter+1] != '\0'; counter++)
	{
		if (buf[counter] == '$' && buf[counter + 1] == 'N')
		{
		strcpy(buf2,buf);
		buf2[counter] = '\0';
		strcat(buf2,victim->name);
		for (count = 0; buf[count] != '\0'; count++)
		{
			buf[count] = buf[count+counter+2];
		}
		strcat(buf2,buf);
		strcpy(buf,buf2);

		}
		else if (buf[counter] == '$' && buf[counter + 1] == 'E')
		{
		switch (victim->sex)
		{
		default:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"it");
			for (count = 0; buf[count] != '\0'; count ++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		case 1:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"it");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		case 2:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"it");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		}
		}
		else if (buf[counter] == '$' && buf[counter + 1] == 'M')
		{
		buf[counter] = '%';
		buf[counter + 1] = 's';
		switch (victim->sex)
		{
		default:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"it");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		case 1:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"him");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		case 2:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"her");
			for (count = 0; buf[count] != '\0'; count++);
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		}
		}
		else if (buf[counter] == '$' && buf[counter + 1] == 'S')
		{
		switch (victim->sex)
		{
		default:
		strcpy(buf2,buf);
		buf2[counter] = '\0';
		strcat(buf2,"its");
		for (count = 0;buf[count] != '\0'; count++)
		{
			buf[count] = buf[count+counter+2];
		}
		strcat(buf2,buf);
		strcpy(buf,buf2);
		break;
		case 1:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"his");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		case 2:
			strcpy(buf2,buf);
			buf2[counter] = '\0';
			strcat(buf2,"hers");
			for (count = 0; buf[count] != '\0'; count++)
			{
			buf[count] = buf[count+counter+2];
			}
			strcat(buf2,buf);
			strcpy(buf,buf2);
			break;
		}
		}
	 }

		for (d = first_descriptor; d != NULL; d = d->next)
		{
			CHAR_DATA *vch;
			vch = d->original ? d->original : d->character;
			if (d->connected == CON_PLAYING &&
				d->character != ch &&
				d->character != victim &&
				!IS_SET(vch->deaf, CHANNEL_GOCIAL) )
			{
				act(AT_BLUE, buf,ch,NULL,vch,TO_VICT );
			}
		}

	  sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_found);
	  act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
	  sprintf (buf, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->vict_found);
	  act (AT_SOCIAL, buf, ch, NULL, victim, TO_VICT);
	  return;
	}
}
*/

void stage_update( CHAR_DATA *ch, CHAR_DATA *victim, int stage )
{
	if( IS_NPC( ch ) || IS_NPC( victim ) )
		return;

	if( stage == 0 )
	{
		if( ch->sex == SEX_MALE )
		{
			act( AT_SOCIAL, "You feel yourself harden.", ch, NULL, NULL, TO_CHAR );
			act( AT_SOCIAL, "You feel $n harden.", ch, NULL, victim, TO_VICT );
			return;
		}
		else if( ch->sex == SEX_FEMALE )
		{
			act( AT_SOCIAL, "You feel moist.", ch, NULL, NULL, TO_CHAR );
			act( AT_SOCIAL, "You feel $n dampen.", ch, NULL, victim, TO_VICT );
			return;
		}
	}
	else if( stage == 2 )
	{
		if( ch->sex == SEX_MALE )
		{
			act( AT_SOCIAL, "You clench your teeth as you cum in $M.", ch, NULL, victim, TO_CHAR );
			act( AT_SOCIAL, "$n clenches $s teeth as $e cums in you.", ch, NULL, victim, TO_VICT );
			act( AT_SOCIAL, "$n clenches $s teeth as $e cums in $N.", ch, NULL, victim, TO_NOTVICT );
			ch->pcdata->stage[0] = 0;
			ch->pcdata->stage[1] = 0;
			/*	    ch->pcdata->genes[8] += 1;
					victim->pcdata->genes[8] += 1;
					do_autosave( ch, "" );
					do_autosave( victim, "" );
			*/ if( ch->pcdata->stage[0] <= 250 )
	ch->pcdata->stage[0] = 0;
			else
	victim->pcdata->stage[0] -= 250;
			victim->pcdata->stage[1] = 0;
			/*	    if ( victim->sex == SEX_FEMALE &&
					!IS_EXTRA( victim, EXTRA_PREGNANT ) && number_percent( ) <= 8 )
					make_preg( victim, ch );
			*/ return;
		}
		else if( ch->sex == SEX_FEMALE )
		{
			act( AT_SOCIAL, "You wimper as you cum.", ch, NULL, victim, TO_CHAR );
			act( AT_SOCIAL, "$n wimpers as $e cums.", ch, NULL, victim, TO_ROOM );
			if( victim->pcdata->stage[2] < 1 || victim->pcdata->stage[2] >= 250 )
			{
				ch->pcdata->stage[2] = 0;
				if( ch->pcdata->stage[0] >= 200 )
				{
					ch->pcdata->stage[0] -= 100;
				}
			}
			else
				ch->pcdata->stage[2] = 200;
			return;
		}
	}
	return;
}


/******************************************************
	  Desolation of the Dragon MUD II - Alias Code
	  (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com
 ******************************************************/

ALIAS_DATA *find_alias( CHAR_DATA *ch, const char *argument )
{
	ALIAS_DATA *pal;
	char buf[MAX_INPUT_LENGTH];

	if( !ch || !ch->pcdata )
		return ( NULL );

	one_argument( argument, buf );

	for( pal = ch->pcdata->first_alias; pal; pal = pal->next )
		if( !str_prefix( buf, pal->name ) )
			return ( pal );

	return ( NULL );
}

CMDF( do_alias )
{
	ALIAS_DATA *pal = NULL;
	char arg[MAX_INPUT_LENGTH];
	const char *p;

	if( IS_NPC( ch ) )
		return;

	for( p = argument; *p != '\0'; p++ )
	{
		if( *p == '~' )
		{
			send_to_char( "Don't use the ~ character!\r\n", ch );
			return;
		}
	}

	argument = one_argument( argument, arg );

	if( !*arg )
	{
		if( !ch->pcdata->first_alias )
		{
			send_to_char( "No aliases set.\r\n", ch );
			return;
		}
		pager_printf( ch, "\r\n&G%-20s Set To\r\n&B================================\r\n", "Alias" );
		for( pal = ch->pcdata->first_alias; pal; pal = pal->next )
			pager_printf( ch, "&C%-20s &c%s\r\n", pal->name, pal->cmd );
		return;
	}

	if( !*argument )
	{
		if( ( pal = find_alias( ch, arg ) ) != NULL )
		{
			DISPOSE( pal->name );
			DISPOSE( pal->cmd );
			UNLINK( pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
			DISPOSE( pal );
			send_to_char( "Alias Deleted.\r\n", ch );
		}
		else
			send_to_char( "No such Alias.\r\n", ch );
		return;
	}

	if( ( pal = find_alias( ch, arg ) ) == NULL )
	{
		CREATE( pal, ALIAS_DATA, 1 );
		pal->name = str_dup( arg );
		pal->cmd = str_dup( argument );
		LINK( pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
		send_to_char( "Alias created.\r\n", ch );
	}
	else
	{
		if( pal->cmd );
		DISPOSE( pal->cmd );
		pal->cmd = str_dup( argument );
		send_to_char( "Alias changed.\r\n", ch );
	}
}

void free_aliases( CHAR_DATA *ch )
{
	ALIAS_DATA *pal, *next_pal;

	if( !ch || !ch->pcdata )
		return;

	for( pal = ch->pcdata->first_alias; pal; pal = next_pal )
	{
		next_pal = pal->next;

		DISPOSE( pal->name );
		DISPOSE( pal->cmd );
		DISPOSE( pal );
	}
}

bool check_alias( CHAR_DATA *ch, const char *command, const char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	ALIAS_DATA *alias;

	if( ( alias = find_alias( ch, command ) ) == NULL )
		return false;

	if( !alias->cmd || !*alias->cmd )
		return false;

	snprintf( arg, MAX_INPUT_LENGTH, "%s", alias->cmd );

	if( ch->pcdata->cmd_recurse == -1 || ++ch->pcdata->cmd_recurse > 50 )
	{
		if( ch->pcdata->cmd_recurse != -1 )
		{
			send_to_char( "Unable to further process command, recurses too much.\r\n", ch );
			ch->pcdata->cmd_recurse = -1;
		}
		return false;
	}

	if( argument && *argument != '\0' )
	{
		strcat( arg, " " );
		strcat( arg, argument );
	}

	interpret( ch, arg );
	return true;
}
