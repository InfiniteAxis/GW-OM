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


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>
#include "mud.h"
#include "mccp.h"
#include <dirent.h>
#include "sha256.h"

#define RESTORE_INTERVAL 10800

const char *const save_flag[] = { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
   "auction", "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
   "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
   "r28", "r29", "r30", "r31"
};

/*
 * Local functions.
 */
void save_banlist( void );
void fwrite_locker( CHAR_DATA *ch );
void delete_locker( CHAR_DATA *ch );

int get_color( const char *argument );    /* function proto */
void calc_season( void );
/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
bool is_wizvis( CHAR_DATA *ch, CHAR_DATA *victim );

/*
 * Semi-globals
 */
extern int port;
extern int control;

int get_saveflag( const char *name )
{
	size_t x;

	for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
		if( !str_cmp( name, save_flag[x] ) )
			return x;
	return -1;
}

HELP_DATA *get_help( CHAR_DATA *ch, const char *argument );

CMDF( do_wizhelp )
{
	CMDTYPE *cmd;
	int col, hash;
	int curr_lvl;
	bool nCode = false;
	bool nHelp = false;
	char ccolor[100];
	col = 0;
	set_pager_color( AT_WHITE, ch );

	for( curr_lvl = 1001; curr_lvl <= get_trust( ch ); curr_lvl++ )
	{
		send_to_pager( "\r\n\r\n", ch );
		pager_printf( ch, "&C[&cLevel &w%-4d&C]\r\n", curr_lvl );
		col = 0;
		for( hash = 0; hash < 126; hash++ )
		{
			for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
			{
				if( ( cmd->level == curr_lvl ) && cmd->level <= get_trust( ch ) && cmd->cshow == 0 )
				{
					nCode = false;
					nHelp = false;
					if( cmd->do_fun == skill_notfound )
						nCode = true;
					if( !get_help( ch, cmd->name ) )
						nHelp = true;
					if( nCode && nHelp )
						mudstrlcpy( ccolor, "&Y", MSL );
					else if( nCode )
						mudstrlcpy( ccolor, "&O", MSL );
					else if( nHelp )
						mudstrlcpy( ccolor, "&W", MSL );
					else
						mudstrlcpy( ccolor, "&c", MSL );
					pager_printf( ch, "%s%-15s", ccolor, cmd->name );
					if( ++col % 5 == 0 )
						send_to_pager( "&D\r\n", ch );
				}
			}
		}
	}
	if( col % 5 != 0 )
		send_to_pager( "&D\r\n", ch );
	return;
}


CMDF( do_restrict )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	short level, hash;
	CMDTYPE *cmd;
	bool found;

	found = false;

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Restrict which command?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg2 );
	if( arg2[0] == '\0' )
		level = get_trust( ch );
	else
		level = atoi( arg2 );

	level = UMAX( UMIN( get_trust( ch ), level ), 0 );

	hash = arg[0] % 126;
	for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	{
		if( !str_prefix( arg, cmd->name ) && cmd->level <= get_trust( ch ) )
		{
			found = true;
			break;
		}
	}

	if( found )
	{
		if( !str_prefix( arg2, "show" ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s show", cmd->name );
			do_cedit( ch, buf );
			/*    		ch_printf( ch, "%s is at level %d.\r\n", cmd->name, cmd->level );*/
			return;
		}
		cmd->level = level;
		ch_printf( ch, "You restrict %s to level %d\r\n", cmd->name, level );
		snprintf( buf, MAX_STRING_LENGTH, "%s restricting %s to level %d", ch->name, cmd->name, level );
		log_string( buf );
	}
	else
		send_to_char( "You may not restrict that command.\r\n", ch );

	return;
}

/*
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA *ch, char *name )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ret_char;
	static unsigned int number_of_hits;

	number_of_hits = 0;
	for( d = first_descriptor; d; d = d->next )
	{
		if( d->character && ( !str_prefix( name, d->character->name ) ) && IS_WAITING_FOR_AUTH( d->character ) )
		{
			if( ++number_of_hits > 1 )
			{
				ch_printf( ch, "%s does not uniquely identify a char.\r\n", name );
				return NULL;
			}
			ret_char = d->character;   /* return current char on exit */
		}
	}
	if( number_of_hits == 1 )
		return ret_char;
	else
	{
		send_to_char( "No one like that waiting for authorization.\r\n", ch );
		return NULL;
	}
}

CMDF( do_authorize )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\r\n", ch );
		send_to_char( "Pending authorizations:\r\n", ch );
		send_to_char( " Chosen Character Name\r\n", ch );
		send_to_char( "---------------------------------------------\r\n", ch );
		for( d = first_descriptor; d; d = d->next )
			if( ( victim = d->character ) != NULL && IS_WAITING_FOR_AUTH( victim ) )
				ch_printf( ch, " %s@%s new %s...\r\n", victim->name, victim->desc->host, race_table[victim->race].race_name );
		return;
	}

	victim = get_waiting_desc( ch, arg1 );
	if( victim == NULL )
		return;

	if( arg2[0] == '\0' || !str_cmp( arg2, "accept" ) || !str_cmp( arg2, "yes" ) )
	{
		victim->pcdata->auth_state = 3;
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_UNAUTHED );
		if( victim->pcdata->authed_by )
			STRFREE( victim->pcdata->authed_by );
		victim->pcdata->authed_by = QUICKLINK( ch->name );
		snprintf( buf, MAX_STRING_LENGTH, "%s authorized %s", ch->name, victim->name );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
		ch_printf( ch, "You have authorized %s.\r\n", victim->name );

		ch_printf( victim,
			"The MUD Administrators have accepted the name %s.\r\n"
			"You are now fully authorized to play the Gundam Wing: Operation Meteor.\r\n", victim->name );
		return;
	}
	else if( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
	{
		send_to_char( "You have been denied access.\r\n", victim );
		snprintf( buf, MAX_STRING_LENGTH, "%s denied authorization to %s", ch->name, victim->name );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
		ch_printf( ch, "You have denied %s.\r\n", victim->name );
		snprintf( buf, MAX_STRING_LENGTH, "%s add", victim->name );
		do_reserve( ch, buf );
		do_quit( victim, "" );
	}

	else if( !str_cmp( arg2, "name" ) || !str_cmp( arg2, "n" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s has denied %s's name", ch->name, victim->name );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
		ch_printf( victim,
			"The MUD Administrators have found the name %s "
			"to be unacceptable.\r\n" "Use 'name' to change it to something more apropriate.\r\n", victim->name );
		ch_printf( ch, "You requested %s change names.\r\n Type Help name\r\n", victim->name );
		victim->pcdata->auth_state = 2;
		return;
	}

	else
	{
		send_to_char( "Invalid argument.\r\n", ch );
		return;
	}
}

/*void do_bioauth( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	if( !ch )
		return;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( !arg || arg[0] == '\0' )
	{
		send_to_char( "Usage: bioauth <player> <yes|no> <reason>\r\n", ch );
		send_to_char( "\r\n  Reason being optional, only applicable if bio is denied.\r\n", ch );
		return;
	}

	victim = get_char_world( ch, arg );

	if( !victim || IS_NPC(victim) )
	{
		send_to_char( "No one like that is waiting to have their bio authed.\r\n", ch );
		return;
	}

	if( victim->pcdata->rprate != RATE_UNAUTHED )
	{
		send_to_char( "No one like that is waiting to have their bio authed.\r\n". ch );
		return;
	}

	if( !arg2 || arg2[0] == '\0' || !str_cmp( arg2, "yes" ) )
	{
		victim->pcdata->rprate = 0;
		if( victim->pcdata->bio_authed )
			STRFREE( victim->pcdata->bio_authed );
		victim->pcdata->bio_authed = STRALLOC( ch->name );
		send_to_char( "Ok.\r\n", ch );
		ch_printf( victim, "Your bio has been authorized by %s.\r\n", ch->name );
		return;
	}

	else if( !str_cmp( arg2, "no" ) )
	{
		send_to_char( "Ok.\r\n", ch );
		ch_printf( victim, "Your bio has been denied by %s. Reason: %s\r\n", ch->name,
			!argument ? "None." : argument );
		return;
	}

	do_bioauth( ch, "" );
	return;
}*/

CMDF( do_bamfin )
{
	if( !IS_NPC( ch ) )
	{
		argument = smash_tilde_static( argument );
		DISPOSE( ch->pcdata->bamfin );
		ch->pcdata->bamfin = str_dup( argument );
		send_to_char( "Ok.\r\n", ch );
	}
}

CMDF( do_bamfout )
{
	if( !IS_NPC( ch ) )
	{
		argument = smash_tilde_static( argument );
		DISPOSE( ch->pcdata->bamfout );
		ch->pcdata->bamfout = str_dup( argument );
		send_to_char( "Ok.\r\n", ch );
	}
}

CMDF( do_rank )
{
	if( IS_NPC( ch ) )
		return;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: rank <string>.\r\n", ch );
		send_to_char( "   or: rank none.\r\n", ch );
		return;
	}

	argument = smash_tilde_static( argument );
	STRFREE( ch->pcdata->rank );
	if( !str_cmp( argument, "none" ) )
		ch->pcdata->rank = STRALLOC( "" );
	else
		ch->pcdata->rank = STRALLOC( argument );
	send_to_char( "Ok.\r\n", ch );
}


CMDF( do_retire )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Retire whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( victim->top_level < LEVEL_LIAISON )
	{
		send_to_char( "The minimum level for retirement is Liaison.\r\n", ch );
		return;
	}

	if( IS_RETIRED( victim ) )
	{
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
		ch_printf( ch, "%s returns from retirement.\r\n", victim->name );
		ch_printf( victim, "%s brings you back from retirement.\r\n", ch->name );
	}
	else
	{
		SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
		ch_printf( ch, "%s is now a retired immortal.\r\n", victim->name );
		ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\r\n", ch->name );
	}
	return;
}

CMDF( do_pissoff )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Who deserves this hilarious fate?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't playing.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "I wouldn't even think of that if I were you...\r\n", ch );
		return;
	}

	set_char_color( AT_WHITE, ch );
	send_to_char( "You summon the demon Balzhur to wreak your wrath!\r\n", ch );
	send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\r\n", ch );
	set_char_color( AT_IMMORT, victim );
	send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\r\n", victim );
	snprintf( buf, MAX_STRING_LENGTH, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	SET_BIT( victim->pcdata->flags, PCFLAG_FBALZHUR );
	victim->top_level = 1;
	return;
}

CMDF( do_deny )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Deny whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	xSET_BIT( victim->act, PLR_DENY );
	send_to_char( "You are denied access!\r\n", victim );
	send_to_char( "OK.\r\n", ch );
	do_quit( victim, "" );

	return;
}



CMDF( do_disconnect )
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Disconnect whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim->desc == NULL )
	{
		act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( get_trust( ch ) <= get_trust( victim ) )
	{
		send_to_char( "They might not like that...\r\n", ch );
		return;
	}

	for( d = first_descriptor; d; d = d->next )
	{
		if( d == victim->desc )
		{
			close_socket( d, false );
			send_to_char( "Ok.\r\n", ch );
			return;
		}
	}

	bug( "Do_disconnect: *** desc not found ***.", 0 );
	send_to_char( "Descriptor not found!\r\n", ch );
	return;
}

/*
 * Force a level one player to quit.             Gorog
 */
CMDF( do_fquit )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Force whom to quit?\r\n", ch );
		return;
	}

	if( !( victim = get_char_world( ch, arg1 ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim->top_level != 1 )
	{
		send_to_char( "They are not level oneo!\r\n", ch );
		return;
	}

	send_to_char( "The MUD administrators force you to quit\r\n", victim );
	do_quit( victim, "" );
	send_to_char( "Ok.\r\n", ch );
	return;
}


CMDF( do_forceclose )
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int desc;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Usage: forceclose <descriptor#>\r\n", ch );
		return;
	}
	desc = atoi( arg );

	for( d = first_descriptor; d; d = d->next )
	{
		if( d->descriptor == desc )
		{
			if( d->character && get_trust( d->character ) >= get_trust( ch ) )
			{
				send_to_char( "They might not like that...\r\n", ch );
				return;
			}
			close_socket( d, false );
			send_to_char( "Ok.\r\n", ch );
			return;
		}
	}

	send_to_char( "Not found!\r\n", ch );
	return;
}

CMDF( do_twit )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: twit <character>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( ( victim->top_level >= ch->top_level ) && ( victim != ch ) )
	{
		send_to_char( "Your command backfires!\r\n", ch );
		send_to_char( "You are now considered a TWIT.\r\n", ch );
		SET_BIT( ch->pcdata->flags, PCFLAG_TWIT );
		return;
	}

	if( IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
	{
		send_to_char( "Someone beat you to it.\r\n", ch );
	}
	else
	{
		SET_BIT( victim->pcdata->flags, PCFLAG_TWIT );
		send_to_char( "Twit flag set.\r\n", ch );
		send_to_char( "You are now considered a TWIT.\r\n", victim );
		snprintf( buf, MAX_STRING_LENGTH, "&C%s &GH&ga&Gs &BB&be&Be&bn &PT&pw&rit&pe&Pd&R! &YG&OE&YT &RT&rH&RE&rM&z!&W!&z!", victim->name );
		info_chan( buf );
	}
	return;
}


CMDF( do_pardon )
{
	CHAR_DATA *victim;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: pardon <character>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
	{
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_TWIT );
		send_to_char( "Twit flag removed.\r\n", ch );
		send_to_char( "You are no longer a TWIT.\r\n", victim );
	}
	return;
}

void echo_to_all( short AT_COLOR, const char *argument, short tar )
{
	DESCRIPTOR_DATA *d;

	if( !argument || argument[0] == '\0' )
		return;

	for( d = first_descriptor; d; d = d->next )
	{
		/*
		 * Added showing echoes to players who are editing, so they won't
		 * miss out on important info like upcoming reboots. --Narn
		 */
		if( d->connected == CON_PLAYING || d->connected == CON_EDITING )
		{
			/*
			 * This one is kinda useless except for switched..
			 */
			if( tar == ECHOTAR_PC && IS_NPC( d->character ) )
				continue;
			else if( tar == ECHOTAR_IMM && !IS_IMMORTAL( d->character ) )
				continue;
			set_char_color( AT_COLOR, d->character );
			send_to_char( argument, d->character );
			send_to_char( "\r\n", d->character );
		}
	}
	return;
}

CMDF( do_echo )
{
	char arg[MAX_INPUT_LENGTH];
	short color;
	int target;
	const char *parg;

	if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
	{
		send_to_char( "You are noemoted and can not echo.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Echo what?\r\n", ch );
		return;
	}

	if( ( color = get_color( argument ) ) )
		argument = one_argument( argument, arg );
	parg = argument;
	argument = one_argument( argument, arg );
	if( !str_cmp( arg, "PC" ) || !str_cmp( arg, "player" ) )
		target = ECHOTAR_PC;
	else if( !str_cmp( arg, "imm" ) )
		target = ECHOTAR_IMM;
	else
	{
		target = ECHOTAR_ALL;
		argument = parg;
	}
	if( !color && ( color = get_color( argument ) ) )
		argument = one_argument( argument, arg );
	if( !color )
		color = AT_IMMORT;
	one_argument( argument, arg );
	if( !str_cmp( arg, "Merth" ) || !str_cmp( arg, "Durga" ) )
	{
		ch_printf( ch, "I don't think %s would like that!\r\n", arg );
		return;
	}
	echo_to_all( color, argument, target );
}

void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA *room, const char *argument )
{
	CHAR_DATA *vic;

	if( room == NULL )
		return;


	for( vic = room->first_person; vic; vic = vic->next_in_room )
	{
		set_char_color( AT_COLOR, vic );
		send_to_char( argument, vic );
		send_to_char( "\r\n", vic );
	}
}

CMDF( do_recho )
{
	char arg[MAX_INPUT_LENGTH];
	short color;

	if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
	{
		send_to_char( "You are noemoted and can not recho.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Recho what?\r\n", ch );
		return;
	}

	one_argument( argument, arg );
	if( !str_cmp( arg, "Thoric" )
		|| !str_cmp( arg, "Dominus" )
		|| !str_cmp( arg, "Circe" )
		|| !str_cmp( arg, "Haus" )
		|| !str_cmp( arg, "Narn" ) || !str_cmp( arg, "Scryn" ) || !str_cmp( arg, "Blodkai" ) || !str_cmp( arg, "Damian" ) )
	{
		ch_printf( ch, "I don't think %s would like that!\r\n", arg );
		return;
	}
	if( ( color = get_color( argument ) ) )
	{
		argument = one_argument( argument, arg );
		echo_to_room( color, ch->in_room, argument );
	}
	else
		echo_to_room( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, const char *arg )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if( is_number( arg ) )
		return get_room_index( atoi( arg ) );

	if( ( victim = get_char_world( ch, arg ) ) != NULL )
		return victim->in_room;

	if( ( obj = get_obj_world( ch, arg ) ) != NULL )
		return obj->in_room;

	return NULL;
}

/* This function shared by do_transfer and do_mptransfer
 *
 * Immortals bypass most restrictions on where to transfer victims.
 * NPCs cannot transfer victims who are:
 * 1. Not authorized yet.
 * 2. Outside of the level range for the target room's area.
 * 3. Being sent to private rooms.
 */
void transfer_char( CHAR_DATA *ch, CHAR_DATA *victim, ROOM_INDEX_DATA *location )
{
	if( !victim->in_room )
	{
		bug( "%s: victim in NULL room: %s", __FUNCTION__, victim->name );
		return;
	}

	if( IS_NPC( ch ) && room_is_private( victim, location ) )
	{
		progbug( "Mptransfer - Private room", ch );
		return;
	}

	if( !can_see( ch, victim ) )
		return;

	if( IS_NPC( ch ) && NOT_AUTHED( victim ) && location->area != victim->in_room->area )
	{
		char buf[MAX_STRING_LENGTH];

		snprintf( buf, MAX_STRING_LENGTH, "Mptransfer - unauthed char (%s)", victim->name );
		progbug( buf, ch );
		return;
	}

	/* If victim not in area's level range, do not transfer */
	if( IS_NPC( ch ) && !in_hard_range( victim, location->area )
		&& !xIS_SET( location->room_flags, ROOM_PROTOTYPE ) )
		return;

	if( victim->fighting )
		stop_fighting( victim, true );

	if( !IS_NPC( ch ) )
	{
		act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
		victim->retran = victim->in_room->vnum;
	}
	char_from_room( victim );
	char_to_room( victim, location );
	if( !IS_NPC( ch ) )
	{
		act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
		if( ch != victim )
			act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
		do_look( victim, "auto" );
		if( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && !in_hard_range( victim, location->area ) )
			act( AT_DANGER, "Warning:  this player's level is not within the area's level range.", ch, NULL, NULL, TO_CHAR );
	}
}

CMDF( do_transfer )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	set_char_color( AT_IMMORT, ch );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Transfer whom (and where)?\r\n", ch );
		return;
	}

	if( arg2[0] != '\0' )
	{
		if( !( location = find_location( ch, arg2 ) ) )
		{
			send_to_char( "That location does not exist.\r\n", ch );
			return;
		}
	}
	else
		location = ch->in_room;

	if( !str_cmp( arg1, "all" ) && get_trust( ch ) >= LEVEL_LIAISON )
	{
		for( d = first_descriptor; d; d = d->next )
		{
			if( d->connected == CON_PLAYING && d->character && d->character != ch && d->character->in_room )
				transfer_char( ch, d->character, location );
		}
		return;
	}

	if( !( victim = get_char_world( ch, arg1 ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	transfer_char( ch, victim, location );
}

CMDF( do_retran )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Retransfer whom?\r\n", ch );
		return;
	}
	if( !( victim = get_char_world( ch, arg ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	snprintf( buf, MAX_STRING_LENGTH, "'%s' %d", victim->name, victim->retran );
	do_transfer( ch, buf );
	return;
}

CMDF( do_regoto )
{
	char buf[MAX_STRING_LENGTH];

	snprintf( buf, MAX_STRING_LENGTH, "%d", ch->regoto );
	do_goto( ch, buf );
	return;
}

CMDF( do_at )
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	CHAR_DATA *wch;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "At where what?\r\n", ch );
		return;
	}

	if( ( location = find_location( ch, arg ) ) == NULL )
	{
		send_to_char( "No such location.\r\n", ch );
		return;
	}

	if( room_is_private( ch, location ) )
	{
		if( get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That room is private right now.\r\n", ch );
			return;
		}
		else
		{
			send_to_char( "Overriding private flag!\r\n", ch );
		}

	}

	original = ch->in_room;
	char_from_room( ch );
	char_to_room( ch, location );
	interpret( ch, argument );

	/*
	 * See if 'ch' still exists before continuing!
	 * Handles 'at XXXX quit' case.
	 */
	for( wch = first_char; wch; wch = wch->next )
	{
		if( wch == ch )
		{
			char_from_room( ch );
			char_to_room( ch, original );
			break;
		}
	}

	return;
}

CMDF( do_rat )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	int Start, End, vnum;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Syntax: rat <start> <end> <command>\r\n", ch );
		return;
	}

	Start = atoi( arg1 );
	End = atoi( arg2 );

	if( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUMS )
	{
		send_to_char( "Invalid range.\r\n", ch );
		return;
	}

	if( !str_cmp( argument, "quit" ) )
	{
		send_to_char( "I don't think so!\r\n", ch );
		return;
	}

	original = ch->in_room;
	for( vnum = Start; vnum <= End; vnum++ )
	{
		if( ( location = get_room_index( vnum ) ) == NULL )
			continue;
		char_from_room( ch );
		char_to_room( ch, location );
		interpret( ch, argument );
	}

	char_from_room( ch );
	char_to_room( ch, original );
	send_to_char( "Done.\r\n", ch );
	return;
}


CMDF( do_rstat )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	EXIT_DATA *pexit;
	int cnt;
	static const char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

	one_argument( argument, arg );

	if( get_trust( ch ) < LEVEL_STAFF )
	{
		AREA_DATA *pArea;

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to goto.\r\n", ch );
			return;
		}

		if( ch->in_room->vnum < pArea->low_vnum || ch->in_room->vnum > pArea->hi_vnum )
		{
			send_to_char( "You can only rstat within your assigned range.\r\n", ch );
			return;
		}

	}


	if( !str_cmp( arg, "exits" ) )
	{
		location = ch->in_room;

		ch_printf( ch, "Exits for room '%s.' vnum %d\r\n", location->name, location->vnum );

		for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
			ch_printf( ch,
				"%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\r\n",
				++cnt,
				dir_text[pexit->vdir],
				pexit->to_room ? pexit->to_room->vnum : 0,
				pexit->key,
				pexit->exit_info,
				pexit->keyword,
				pexit->description[0] != '\0'
				? pexit->description : "(none).\r\n",
				pexit->rexit ? pexit->rexit->vnum : 0, pexit->rvnum, pexit->distance );
		return;
	}
	location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
	if( !location )
	{
		send_to_char( "No such location.\r\n", ch );
		return;
	}

	if( ch->in_room != location && room_is_private( ch, location ) )
	{
		if( get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That room is private right now.\r\n", ch );
			return;
		}
		else
		{
			send_to_char( "Overriding private flag!\r\n", ch );
		}

	}

	ch_printf( ch, "Name: %s.\n\rArea: %s  Filename: %s.\r\n",
		location->name,
		location->area ? location->area->name : "None????", location->area ? location->area->filename : "None????" );

	ch_printf( ch,
		"Vnum: %d.  Sector: %d.  Light: %d.  TeleDelay: %d.  TeleVnum: %d  Tunnel: %d\n\rHome Owner: %s.\r\n",
		location->vnum,
		location->sector_type,
		location->light,
		location->tele_delay, location->tele_vnum, location->tunnel, location->owner );

	ch_printf( ch, "Room flags: %s\r\n", ext_flag_string( &location->room_flags, r_flags ) );
	ch_printf( ch, "Description:\r\n%s", location->description );

	if( location->first_extradesc )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_char( "Extra description keywords: '", ch );
		for( ed = location->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword, ch );
			if( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\r\n", ch );
	}

	send_to_char( "Characters:", ch );
	for( rch = location->first_person; rch; rch = rch->next_in_room )
	{
		if( can_see( ch, rch ) )
		{
			send_to_char( " ", ch );
			one_argument( rch->name, buf );
			send_to_char( buf, ch );
		}
	}

	send_to_char( ".\n\rObjects:   ", ch );
	for( obj = location->first_content; obj; obj = obj->next_content )
	{
		send_to_char( " ", ch );
		one_argument( obj->name, buf );
		send_to_char( buf, ch );
	}
	send_to_char( ".\r\n", ch );

	if( location->first_exit )
		send_to_char( "------------------- EXITS -------------------\r\n", ch );
	for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
		ch_printf( ch,
			"%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\r\n",
			++cnt,
			dir_text[pexit->vdir],
			pexit->to_room ? pexit->to_room->vnum : 0,
			pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
	return;
}

void ostat_plus( CHAR_DATA *ch, OBJ_DATA *obj )
{
	SKILLTYPE *sktmp;
	int dam;
	char buf[MSL];
	//   char *armortext = NULL;
	int x;

	/******
	 * A more informative ostat, so You actually know what those obj->value[x] mean
	 * without looking in the code for it. Combines parts of look, examine, the
	 * identification spell, and things that were never seen.. Probably overkill
	 * on most things, but I'm lazy and hate digging through code to see what
	 * value[x] means... -Druid
	 *
	 * Make sure this stays in synch with the copy in olcobj.c - Samson
	 ******/
	send_to_char( "Additional Object information:\r\n", ch );
	switch( obj->item_type )
	{
	default:
		send_to_char( "Sorry, No additional inforamtion avalible.\r\n", ch );
		break;
	case ITEM_LIGHT:
		send_to_char( "Value[2] - Hours left: ", ch );
		if( obj->value[2] >= 0 )
			ch_printf( ch, "%d\r\n", obj->value[2] );
		else
			send_to_char( "Infinite\r\n", ch );
		break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
		ch_printf( ch, "Value[0] - Spell Level: %d\r\n", obj->value[0] );
		for( x = 1; x <= 3; x++ )
		{
			if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
				ch_printf( ch, "Value[%d] - Spell (%d): %s\r\n", x, obj->value[x], sktmp->name );
			else
				ch_printf( ch, "Value[%d] - Spell: None\r\n", x );
		}
		if( obj->item_type == ITEM_PILL )
			ch_printf( ch, "Value[4] - Food Value: %d\r\n", obj->value[4] );
		break;
	case ITEM_SALVE:
	case ITEM_WAND:
	case ITEM_STAFF:
		ch_printf( ch, "Value[0] - Spell Level: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[1] - Max Charges: %d\r\n", obj->value[1] );
		ch_printf( ch, "Value[2] - Charges Remaining: %d\r\n", obj->value[2] );
		if( obj->item_type != ITEM_SALVE )
		{
			if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
				ch_printf( ch, "Value[3] - Spell (%d): %s\r\n", obj->value[3], sktmp->name );
			else
				ch_printf( ch, "Value[3] - Spell: None\r\n" );
			break;
		}
		ch_printf( ch, "Value[3] - Delay (beats): %d\r\n", obj->value[3] );
		for( x = 4; x <= 5; x++ )
		{
			if( obj->value[x] >= 0 && ( sktmp = get_skilltype( obj->value[x] ) ) != NULL )
				ch_printf( ch, "Value[%d] - Spell (%d): %s\r\n", x, obj->value[x], sktmp->name );
			else
				ch_printf( ch, "Value[%d] - Spell: None\r\n", x );
		}
		break;
	case ITEM_WEAPON:
		ch_printf( ch, "Value[0] - Base Condition: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[1] - Min. Damage: %d\r\n", obj->value[1] );
		ch_printf( ch, "Value[2] - Max Damage: %d\r\n", obj->value[2] );
		ch_printf( ch, "           Average Hit: %d\r\n", ( obj->value[1] + obj->value[2] ) / 2 );
		ch_printf( ch, "Value[3] - Damage Type (%d): %s\r\n", obj->value[3], attack_table[obj->value[3]] );
		if( obj->value[3] == WEAPON_BLASTER || obj->value[3] == WEAPON_BOWCASTER )
		{
			ch_printf( ch, "Value[4] - Current Energy: %d\r\n", obj->value[4] );
			ch_printf( ch, "Value[5] - Maximum Energy: %d\r\n", obj->value[5] );
		}
		ch_printf( ch, "Condition: %d\r\n", obj->value[0] );
		break;
	case ITEM_ARMOR:
		ch_printf( ch, "Value[0] - Current AC: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[1] - Base AC: %d\r\n", obj->value[1] );
		send_to_char( "Condition: ", ch );
		break;
		/*
		 * Bug Fix 7/9/00 -Druid
		 */
	case ITEM_FOOD:
		ch_printf( ch, "Value[0] - Food Value: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[1] - Condition (%d): ", obj->value[1] );
		if( obj->timer > 0 && obj->value[1] > 0 )
			dam = ( obj->timer * 10 ) / obj->value[1];
		else
			dam = 10;
		if( dam >= 10 )
			mudstrlcpy( buf, "It is fresh.", MAX_STRING_LENGTH );
		else if( dam == 9 )
			mudstrlcpy( buf, "It is nearly fresh.", MAX_STRING_LENGTH );
		else if( dam == 8 )
			mudstrlcpy( buf, "It is perfectly fine.", MAX_STRING_LENGTH );
		else if( dam == 7 )
			mudstrlcpy( buf, "It looks good.", MAX_STRING_LENGTH );
		else if( dam == 6 )
			mudstrlcpy( buf, "It looks ok.", MAX_STRING_LENGTH );
		else if( dam == 5 )
			mudstrlcpy( buf, "It is a little stale.", MAX_STRING_LENGTH );
		else if( dam == 4 )
			mudstrlcpy( buf, "It is a bit stale.", MAX_STRING_LENGTH );
		else if( dam == 3 )
			mudstrlcpy( buf, "It smells slightly off.", MAX_STRING_LENGTH );
		else if( dam == 2 )
			mudstrlcpy( buf, "It smells quite rank.", MAX_STRING_LENGTH );
		else if( dam == 1 )
			mudstrlcpy( buf, "It smells revolting!", MAX_STRING_LENGTH );
		else if( dam <= 0 )
			mudstrlcpy( buf, "It is crawling with maggots!", MAX_STRING_LENGTH );
		mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
		send_to_char( buf, ch );
		if( obj->value[3] != 0 )
		{
			ch_printf( ch, "Value[3] - Poisoned (%d): Yes\r\n", obj->value[3] );
			x = 2 * obj->value[0] * ( obj->value[3] > 0 ? obj->value[3] : 1 );
			ch_printf( ch, "Duration: %d\r\n", x );
		}
		if( obj->timer > 0 && obj->value[1] > 0 )
			dam = ( obj->timer * 10 ) / obj->value[1];
		else
			dam = 10;
		if( obj->value[3] <= 0 && ( ( dam < 4 && number_range( 0, dam + 1 ) == 0 ) ) )
		{
			send_to_char( "Poison: Yes\r\n", ch );
			x = 2 * obj->value[0] * ( obj->value[3] > 0 ? obj->value[3] : 1 );
			ch_printf( ch, "Duration: %d\r\n", x );
		}
		if( obj->value[4] )
			ch_printf( ch, "Value[4] - Timer: %d\r\n", obj->value[4] );
		break;
	case ITEM_DRINK_CON:
		ch_printf( ch, "Value[0] - Capacity: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[1] - Quantity Left (%d): ", obj->value[1] );
		if( obj->value[1] > obj->value[0] )
			send_to_char( "More than Full\r\n", ch );
		else if( obj->value[1] == obj->value[0] )
			send_to_char( "Full\r\n", ch );
		else if( obj->value[1] >= ( 3 * obj->value[0] / 4 ) )
			send_to_char( "Almost Full\r\n", ch );
		else if( obj->value[1] > ( obj->value[0] / 2 ) )
			send_to_char( "More than half full\r\n", ch );
		else if( obj->value[1] == ( obj->value[0] / 2 ) )
			send_to_char( "Half full\r\n", ch );
		else if( obj->value[1] >= ( obj->value[0] / 4 ) )
			send_to_char( "Less than half full\r\n", ch );
		else if( obj->value[1] >= 1 )
			send_to_char( "Almost Empty\r\n", ch );
		else
			send_to_char( "Empty\r\n", ch );
		break;
	case ITEM_HERB:
		ch_printf( ch, "Value[1] - Charges: %d\r\n", obj->value[1] );
		ch_printf( ch, "Value[2] - Herb #: Y%d\r\n", obj->value[2] );
		break;
	case ITEM_CONTAINER:
		ch_printf( ch, "Value[0] - Capacity (%d): ", obj->value[0] );
		ch_printf( ch, "%s\r\n",
			obj->value[0] < 76 ? "Small capacity" :
			obj->value[0] < 150 ? "Small to medium capacity" :
			obj->value[0] < 300 ? "Medium capacity" :
			obj->value[0] < 550 ? "Medium to large capacity" :
			obj->value[0] < 751 ? "Large capacity" : "Giant capacity" );
		ch_printf( ch, "Value[1] - Flags (%d): ", obj->value[1] );
		if( obj->value[1] <= 0 )
			send_to_char( " None\r\n", ch );
		else
		{
			if( IS_SET( obj->value[1], CONT_CLOSEABLE ) )
				send_to_char( " Closeable", ch );
			if( IS_SET( obj->value[1], CONT_PICKPROOF ) )
				send_to_char( " PickProof", ch );
			if( IS_SET( obj->value[1], CONT_CLOSED ) )
				send_to_char( " Closed", ch );
			if( IS_SET( obj->value[1], CONT_LOCKED ) )
				send_to_char( " Locked", ch );
			//         if( IS_SET( obj->value[1], CONT_EATKEY ) )
			//            send_to_char( " EatKey", ch );
			send_to_char( "\r\n", ch );
		}
		ch_printf( ch, "Value[2] - Key (%d): ", obj->value[2] );
		if( obj->value[2] <= 0 )
			send_to_char( "None\r\n", ch );
		else
		{
			OBJ_INDEX_DATA *key = get_obj_index( obj->value[2] );

			if( key )
				ch_printf( ch, "%s\r\n", key->short_descr );
			else
				send_to_char( "ERROR: Key does not exist!\r\n", ch );
		}
		ch_printf( ch, "Value[3] - Condition: %d\r\n", obj->value[3] );
		if( obj->timer )
			ch_printf( ch, "Object Timer, Time Left: %d\r\n", obj->timer );
		break;
	case ITEM_MONEY:
		ch_printf( ch, "Value[0] - # of Dollars: %d\r\n", obj->value[0] );
		break;
	case ITEM_KEY:
		ch_printf( ch, "Value[0] - Lock #: %d\r\n", obj->value[0] );
		ch_printf( ch, "Value[4] - Durability: %d\r\n", obj->value[4] );
		ch_printf( ch, "Value[5] - Container Lock Number: %d\r\n", obj->value[5] );
		break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	case ITEM_BUTTON:
		ch_printf( ch, "Value[0] - Flags (%d): ", obj->value[0] );
		if( IS_SET( obj->value[0], TRIG_UP ) )
			send_to_char( " UP", ch );
		if( IS_SET( obj->value[0], TRIG_UNLOCK ) )
			send_to_char( " Unlock", ch );
		if( IS_SET( obj->value[0], TRIG_LOCK ) )
			send_to_char( " Lock", ch );
		if( IS_SET( obj->value[0], TRIG_D_NORTH ) )
			send_to_char( " North", ch );
		if( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
			send_to_char( " South", ch );
		if( IS_SET( obj->value[0], TRIG_D_EAST ) )
			send_to_char( " East", ch );
		if( IS_SET( obj->value[0], TRIG_D_WEST ) )
			send_to_char( " West", ch );
		if( IS_SET( obj->value[0], TRIG_D_UP ) )
			send_to_char( " Up", ch );
		if( IS_SET( obj->value[0], TRIG_D_DOWN ) )
			send_to_char( " Down", ch );
		if( IS_SET( obj->value[0], TRIG_DOOR ) )
			send_to_char( " Door", ch );
		if( IS_SET( obj->value[0], TRIG_CONTAINER ) )
			send_to_char( " Container", ch );
		if( IS_SET( obj->value[0], TRIG_OPEN ) )
			send_to_char( " Open", ch );
		if( IS_SET( obj->value[0], TRIG_CLOSE ) )
			send_to_char( " Close", ch );
		if( IS_SET( obj->value[0], TRIG_PASSAGE ) )
			send_to_char( " Passage", ch );
		if( IS_SET( obj->value[0], TRIG_OLOAD ) )
			send_to_char( " Oload", ch );
		if( IS_SET( obj->value[0], TRIG_MLOAD ) )
			send_to_char( " Mload", ch );
		if( IS_SET( obj->value[0], TRIG_TELEPORT ) )
			send_to_char( " Teleport", ch );
		if( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
			send_to_char( " TeleportAll", ch );
		if( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
			send_to_char( " TeleportPlus", ch );
		if( IS_SET( obj->value[0], TRIG_DEATH ) )
			send_to_char( " Death", ch );
		if( IS_SET( obj->value[0], TRIG_CAST ) )
			send_to_char( " Cast", ch );
		if( IS_SET( obj->value[0], TRIG_FAKEBLADE ) )
			send_to_char( " FakeBlade", ch );
		if( IS_SET( obj->value[0], TRIG_RAND4 ) )
			send_to_char( " Rand4", ch );
		if( IS_SET( obj->value[0], TRIG_RAND6 ) )
			send_to_char( " Rand6", ch );
		if( IS_SET( obj->value[0], TRIG_TRAPDOOR ) )
			send_to_char( " Trapdoor", ch );
		if( IS_SET( obj->value[0], TRIG_ANOTHEROOM ) )
			send_to_char( " Anotheroom", ch );
		if( IS_SET( obj->value[0], TRIG_USEDIAL ) )
			send_to_char( " UseDial", ch );
		if( IS_SET( obj->value[0], TRIG_ABSOLUTEVNUM ) )
			send_to_char( " AbsoluteVnum", ch );
		if( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
			send_to_char( " ShowRoomDesc", ch );
		if( IS_SET( obj->value[0], TRIG_AUTORETURN ) )
			send_to_char( " AutoReturn", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Trigger Position: ", ch );
		if( obj->item_type != ITEM_BUTTON )
		{
			if( IS_SET( obj->value[0], TRIG_UP ) )
				send_to_char( "UP\r\n", ch );
			else
				send_to_char( "Down\r\n", ch );
		}
		else
		{
			if( IS_SET( obj->value[0], TRIG_UP ) )
				send_to_char( "IN\r\n", ch );
			else
				send_to_char( "Out\r\n", ch );
		}
		ch_printf( ch, "Automatically Reset Trigger?: %s\r\n", IS_SET( obj->value[0], TRIG_AUTORETURN ) ? "Yes" : "No" );
		if( HAS_PROG( obj->pIndexData, PULL_PROG ) || HAS_PROG( obj->pIndexData, PUSH_PROG ) )
			send_to_char( "Object Has: ", ch );
		if( HAS_PROG( obj->pIndexData, PULL_PROG ) && HAS_PROG( obj->pIndexData, PUSH_PROG ) )
			send_to_char( "Push and Pull Programs\r\n", ch );
		else if( HAS_PROG( obj->pIndexData, PULL_PROG ) )
			send_to_char( "Pull Program\r\n", ch );
		else if( HAS_PROG( obj->pIndexData, PUSH_PROG ) )
			send_to_char( "Push Program\r\n", ch );
		if( IS_SET( obj->value[0], TRIG_TELEPORT )
			|| IS_SET( obj->value[0], TRIG_TELEPORTALL ) || IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
		{
			ch_printf( ch, "Triggers: Teleport %s\r\n",
				IS_SET( obj->value[0], TRIG_TELEPORT ) ? "Character <actor>" :
				IS_SET( obj->value[0], TRIG_TELEPORTALL ) ? "All Characters in room" :
				"All Characters and Objects in room" );
			ch_printf( ch, "Value[1] - Teleport to Room: %d\r\n", obj->value[1] );
			ch_printf( ch, "Show Room Description on Teleport? %s\r\n",
				IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) ? "Yes" : "No" );
		}
		if( IS_SET( obj->value[0], TRIG_RAND4 ) || IS_SET( obj->value[0], TRIG_RAND6 ) )
			ch_printf( ch, "Triggers: Randomize Exits (%s)\r\n", IS_SET( obj->value[0], TRIG_RAND4 ) ? "3" : "5" );
		if( IS_SET( obj->value[0], TRIG_DOOR ) )
		{
			send_to_char( "Triggers: Door\r\n", ch );
			if( IS_SET( obj->value[0], TRIG_PASSAGE ) )
				send_to_char( "Triggers: Create Passage\r\n", ch );
			if( IS_SET( obj->value[0], TRIG_UNLOCK ) )
				send_to_char( "Triggers: Unlock Door\r\n", ch );
			if( IS_SET( obj->value[0], TRIG_LOCK ) )
				send_to_char( "Triggers: Lock Door\r\n", ch );
			if( IS_SET( obj->value[0], TRIG_OPEN ) )
				send_to_char( "Triggers: Open Door\r\n", ch );
			if( IS_SET( obj->value[0], TRIG_CLOSE ) )
				send_to_char( "Triggers: Close Door\r\n", ch );
			ch_printf( ch, "Value[1] - In Room: %d\r\n", obj->value[1] );
			ch_printf( ch, "To the: %s\r\n",
				IS_SET( obj->value[0], TRIG_D_NORTH ) ? "North" :
				IS_SET( obj->value[0], TRIG_D_SOUTH ) ? "South" :
				IS_SET( obj->value[0], TRIG_D_EAST ) ? "East" :
				IS_SET( obj->value[0], TRIG_D_WEST ) ? "West" :
				IS_SET( obj->value[0], TRIG_D_UP ) ? "UP" :
				IS_SET( obj->value[0], TRIG_D_DOWN ) ? "Down" : "Unknown" );
			if( IS_SET( obj->value[0], TRIG_PASSAGE ) )
				ch_printf( ch, "Value[2] - To Room: %d\r\n", obj->value[2] );
		}
		break;
	case ITEM_BLOOD:
		ch_printf( ch, "Value[1] - Amount Remaining: %d\r\n", obj->value[1] );
		if( obj->timer )
			ch_printf( ch, "Object Timer, Time Left: %d\r\n", obj->timer );
		break;
		/*
			case ITEM_PIECE:
				ch_printf( ch, "Value[0] - Obj Vnum for Other Half: %d", obj->value[0] );
				ch_printf( ch, "Value[1] - Obj Vnum for Combined Object: %d", obj->value[1] );
		*/
		break;
	}
}


CMDF( do_ostat )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	const char *pdesc;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Ostat what?\r\n", ch );
		return;
	}
	if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
		mudstrlcpy( arg, argument, MSL );

	if( ( obj = get_obj_world( ch, arg ) ) == NULL )
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
		return;
	}

	ch_printf( ch, "Name: %s.\r\n", obj->name );

	pdesc = get_extra_descr( arg, obj->first_extradesc );
	if( !pdesc )
		pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc );
	if( !pdesc )
		pdesc = get_extra_descr( obj->name, obj->first_extradesc );
	if( !pdesc )
		pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
	if( pdesc )
		send_to_char( pdesc, ch );


	ch_printf( ch, "Vnum: %d.  Type: %s.  Count: %d  Gcount: %d\r\n",
		obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count, obj->count );

	ch_printf( ch, "Serial#: %d  TopIdxSerial#: %d  TopSerial#: %d\r\n",
		obj->serial, obj->pIndexData->serial, cur_obj_serial );

	ch_printf( ch, "Short description: %s&w.\n\rLong description: %s&w\r\n", obj->short_descr, obj->description );

	if( obj->action_desc[0] != '\0' )
		ch_printf( ch, "Action description: %s.\r\n", obj->action_desc );

	ch_printf( ch, "Wear flags : %s\r\n", flag_string( obj->wear_flags, w_flags ) );
	ch_printf( ch, "Extra flags: %s\r\n", ext_flag_string( &obj->extra_flags, o_flags ) );

	ch_printf( ch, "Number: %d/%d.  Weight: %d/%d.  Layers: %d\r\n",
		1, get_obj_number( obj ), obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

	ch_printf( ch, "Cost: %d.  Rent: %d.  Timer: %d.  Level: %d.\r\n",
		obj->cost, obj->pIndexData->rent, obj->timer, obj->level );

	ch_printf( ch,
		"In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\r\n",
		obj->in_room == NULL ? 0 : obj->in_room->vnum,
		obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
		obj->carried_by == NULL ? "(none)" : obj->carried_by->name, obj->wear_loc );

	ch_printf( ch, "Index Values : %d %d %d %d %d %d.\r\n",
		obj->pIndexData->value[0], obj->pIndexData->value[1],
		obj->pIndexData->value[2], obj->pIndexData->value[3], obj->pIndexData->value[4], obj->pIndexData->value[5] );
	ch_printf( ch, "Object Values: %d %d %d %d %d %d.\r\n",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

	if( obj->pIndexData->first_extradesc )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_char( "Primary description keywords:   '", ch );
		for( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword, ch );
			if( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\r\n", ch );
	}
	if( obj->first_extradesc )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_char( "Secondary description keywords: '", ch );
		for( ed = obj->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword, ch );
			if( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\r\n", ch );
	}

	/*
	 * Rather useful and cool function provided by Druid to decipher those values
	 */
	send_to_char( "\r\n", ch );
	ostat_plus( ch, obj );
	send_to_char( "\r\n", ch );

	for( paf = obj->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );
	//  ch_printf( ch, "Affects %s by %d. (extra)\r\n",
	//      affect_loc_name( paf->location ), paf->modifier );

	for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );
	//  ch_printf( ch, "Affects %s by %d.\r\n",
	//      affect_loc_name( paf->location ), paf->modifier );

	return;
}



CMDF( do_mstat )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	SKILLTYPE *skill;

	set_char_color( AT_PLAIN, ch );

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Mstat whom?\r\n", ch );
		return;
	}
	if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
		mudstrlcpy( arg, argument, MSL );

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
	{
		set_char_color( AT_IMMORT, ch );
		send_to_char( "Their godly glow prevents you from getting a good look.\r\n", ch );
		return;
	}

	if( !IS_SUPREME( ch ) && !str_cmp( victim->name, "Sedle" ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	ch_printf( ch, "Name: %s     Organization: %s\r\n",
		victim->name, ( IS_NPC( victim ) || !victim->pcdata->clan ) ? "(none)" : victim->pcdata->clan->name );
	/*
		if ( IS_NPC( victim ) )
		{
		  ch_printf( ch, "Gang: %s\r\n", victim->gang ? victim->gang : "None" );
		}
		else if ( !IS_NPC(victim) && victim->pcdata->inivictim )
		{
		  ch_printf( ch, "Initiation Target: %s\r\n", victim->pcdata->inivictim ? victim->pcdata->inivictim : "None" );
		}
	*/
	if( get_trust( ch ) >= LEVEL_LIAISON && !IS_NPC( victim ) && victim->desc )
		ch_printf( ch, "User: %s@%s   Descriptor: %d   Trust: %d   AuthedBy: %s\r\n",
			victim->desc->user, victim->desc->host, victim->desc->descriptor,
			victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
	if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
		ch_printf( ch, "Helled until %24.24s by %s.\r\n", ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

	ch_printf( ch, "Vnum: %d   Sex: %s   Room: %d   Count: %d  Killed: %d  LP: %d\r\n",
		IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
		victim->sex == SEX_MALE ? "male" :
		victim->sex == SEX_FEMALE ? "female" : "neutral",
		victim->in_room == NULL ? 0 : victim->in_room->vnum,
		IS_NPC( victim ) ? victim->pIndexData->count : 1,
		IS_NPC( victim ) ? victim->pIndexData->killed
		: victim->pcdata->mdeaths + victim->pcdata->pdeaths, !IS_NPC( victim ) ? victim->pcdata->lp : 0 );
	ch_printf( ch, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d\r\n",
		get_curr_str( victim ),
		get_curr_int( victim ),
		get_curr_wis( victim ),
		get_curr_dex( victim ),
		get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ) );
	ch_printf( ch, "Hps: %d/%d  Move: %d/%d\r\n",
		victim->hit, victim->max_hit, victim->move, victim->max_move );
	if( !IS_NPC( victim ) )
	{
		int ability;

		for( ability = 0; ability < MAX_ABILITY; ability++ )
			ch_printf( ch, "%-15s   Level: %-3d   Max: %-3d   Exp: %-10ld   Next: %-10ld\r\n",
				ability_name[ability], victim->skill_level[ability], max_level( victim, ability ),
				victim->experience[ability], exp_level( victim->skill_level[ability] + 1 ) );
	}
	ch_printf( ch,
		"Top Level: %d     Race: %d  Align: %d  AC: %d  Gold: %d",
		victim->top_level, victim->race, victim->alignment, GET_AC( victim ), victim->gold );

	if( !IS_NPC( victim ) )
		ch_printf( ch, "  Bank: %d\r\n", victim->pcdata->bank );

	if( victim->race >= 0 )
	{
		if( IS_NPC( victim ) && victim->race < MAX_NPC_RACE )
			ch_printf( ch, "Race: %s\r\n", npc_race[victim->race] );
		if( !IS_NPC( victim ) && victim->race < MAX_RACE )
			ch_printf( ch, "Race: %s\r\n", pc_race[victim->race] );
	}
	ch_printf( ch, "Hitroll: %d   Damroll: %d   Position: %s (%d)   Wimpy: %d \r\n",
		GET_HITROLL( victim ), GET_DAMROLL( victim ), npc_position[victim->position], victim->position, victim->wimpy );
	ch_printf( ch, "Fighting: %s    Master: %s    Leader: %s\r\n",
		victim->fighting ? victim->fighting->who->name : "(none)",
		victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );
	if( !IS_NPC( victim ) )
		ch_printf( ch,
			"Thirst: %d   Full: %d   BLust %d    Drunk: %d     Glory: %d/%d\r\n",
			victim->pcdata->condition[COND_THIRST],
			victim->pcdata->condition[COND_FULL],
			victim->pcdata->condition[COND_BLOODTHIRST],
			victim->pcdata->condition[COND_DRUNK], victim->pcdata->quest_curr, victim->pcdata->quest_accum );
	else
		ch_printf( ch, "Mob hitdie : %dd%d+%d    Mob damdie : %dd%d+%3d    Index damdie : %dd%d+%3d\r\n",
			victim->pIndexData->hitnodice,
			victim->pIndexData->hitsizedice,
			victim->pIndexData->hitplus,
			victim->barenumdie,
			victim->baresizedie,
			victim->damplus,
			victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus );
	ch_printf( ch, "MentalState: %d   EmotionalState: %d\r\n", victim->mental_state, victim->emotional_state );
	ch_printf( ch, "Saving throws: %d %d %d %d %d.\r\n",
		victim->saving_poison_death,
		victim->saving_wand, victim->saving_para_petri, victim->saving_breath, victim->saving_spell_staff );
	ch_printf( ch, "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\r\n",
		victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ), victim->numattacks );
	ch_printf( ch, "Years: %d   Seconds Played: %d   Timer: %d\r\n",
		get_age( victim ), ( int ) victim->played, victim->timer );
	if( IS_NPC( victim ) )
	{
		ch_printf( ch, "Act flags: %s\r\n", ext_flag_string( &victim->act, act_flags ) );
		ch_printf( ch, "VIP flags: %s\r\n", flag_string( victim->vip_flags, planet_flags ) );
	}
	else
	{
		ch_printf( ch, "Player flags: %s\r\n", ext_flag_string( &victim->act, plr_flags ) );
		ch_printf( ch, "Pcflags: %s\r\n", flag_string( victim->pcdata->flags, pc_flags ) );
		if( ch->top_level == 1007 )
		{
			ch_printf( ch, "X-Buster: %s\r\n", !victim->pcdata->cybaflags ? "None" :
				flag_string( victim->pcdata->cybaflags, cyba_flags ) );
			ch_printf( ch, "Charge Level: %d  Special Weapon: %s\r\n", victim->pcdata->chargelevel,
				special_weapons[victim->pcdata->specialweapon] );
		}

		ch_printf( ch, "Wanted flags: %s\r\n", flag_string( victim->pcdata->wanted_flags, planet_flags ) );
	}
	ch_printf( ch, "Affected by: %s\r\n", affect_bit_name( victim->affected_by ) );
	if( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0' )
		ch_printf( ch, "Bestowments: %s\r\n", victim->pcdata->bestowments );
	ch_printf( ch, "Short description: %s\n\rLong  description: %s",
		victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\r\n" );
	if( IS_NPC( victim ) && ( victim->spec_fun || victim->spec_2 ) )
		ch_printf( ch, "Mobile has spec fun: %s %s\r\n",
			victim->spec_funname, victim->spec_2 ? victim->spec_funname2 : "" );
	ch_printf( ch, "Body Parts : %s\r\n", flag_string( victim->xflags, part_flags ) );
	ch_printf( ch, "Resistant  : %s\r\n", flag_string( victim->resistant, ris_flags ) );
	ch_printf( ch, "Immune     : %s\r\n", flag_string( victim->immune, ris_flags ) );
	ch_printf( ch, "Susceptible: %s\r\n", flag_string( victim->susceptible, ris_flags ) );
	ch_printf( ch, "Attacks    : %s\r\n", flag_string( victim->attacks, attack_flags ) );
	ch_printf( ch, "Defenses   : %s\r\n", flag_string( victim->defenses, defense_flags ) );
	for( paf = victim->first_affect; paf; paf = paf->next )
		if( ( skill = get_skilltype( paf->type ) ) != NULL )
			ch_printf( ch,
				"%s: '%s' modifies %s by %d for %d rounds with bits %s.\r\n",
				skill_tname[skill->type],
				skill->name,
				affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( paf->bitvector ) );
	return;
}



CMDF( do_mfind )
{
	/*  extern int top_mob_index; */
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	/*  int vnum; */
	int hash;
	int nMatch;
	bool fAll;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Mfind whom?\r\n", ch );
		return;
	}

	fAll = !str_cmp( arg, "all" );
	nMatch = 0;
	set_pager_color( AT_PLAIN, ch );

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	 /*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
		 {
		 if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		 {
			 if ( fAll || is_name( arg, pMobIndex->player_name ) )
			 {
			 nMatch++;
			 snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s\r\n",
				 pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
			 send_to_char( buf, ch );
			 }
		 }
		 }
		  */

		  /*
		   * This goes through all the hash entry points (1024), and is therefore
		   * much faster, though you won't get your vnums in order... oh well. :)
		   *
		   * Tests show that Furey's method will usually loop 32,000 times, calling
		   * get_mob_index()... which loops itself, an average of 1-2 times...
		   * So theoretically, the above routine may loop well over 40,000 times,
		   * and my routine bellow will loop for as many index_mobiles are on
		   * your mud... likely under 3000 times.
		   * -Thoric
		   */
	for( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
			if( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
			{
				nMatch++;
				pager_printf( ch, "[%5d] %s\r\n", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
			}

	if( nMatch )
		pager_printf( ch, "Number of matches: %d\n", nMatch );
	else
		send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

	return;
}



CMDF( do_ofind )
{
	/*  extern int top_obj_index; */
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	/*  int vnum; */
	int hash;
	int nMatch;
	bool fAll;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Ofind what?\r\n", ch );
		return;
	}

	set_pager_color( AT_PLAIN, ch );
	fAll = !str_cmp( arg, "all" );
	nMatch = 0;
	/*  nLoop	= 0; */

	   /*
		* Yeah, so iterating over all vnum's takes 10,000 loops.
		* Get_obj_index is fast, and I don't feel like threading another link.
		* Do you?
		* -- Furey
		for ( vnum = 0; nMatch < top_obj_index; vnum++ )
		{
		nLoop++;
		if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
		{
		if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
		{
		nMatch++;
		snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s\r\n",
		pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		send_to_char( buf, ch );
		}
		}
		}
		*/

		/*
		 * This goes through all the hash entry points (1024), and is therefore
		 * much faster, though you won't get your vnums in order... oh well. :)
		 *
		 * Tests show that Furey's method will usually loop 32,000 times, calling
		 * get_obj_index()... which loops itself, an average of 2-3 times...
		 * So theoretically, the above routine may loop well over 50,000 times,
		 * and my routine bellow will loop for as many index_objects are on
		 * your mud... likely under 3000 times.
		 * -Thoric
		 */
	for( hash = 0; hash < MAX_KEY_HASH; hash++ )
		for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
			if( fAll || nifty_is_name( arg, pObjIndex->name ) )
			{
				nMatch++;
				pager_printf( ch, "[%5d] %s\r\n", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
			}

	if( nMatch )
		pager_printf( ch, "Number of matches: %d\n", nMatch );
	else
		send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

	return;
}



CMDF( do_mwhere )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool found;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Mwhere whom?\r\n", ch );
		return;
	}

	set_pager_color( AT_PLAIN, ch );
	found = false;
	for( victim = first_char; victim; victim = victim->next )
	{
		if( IS_NPC( victim ) && victim->in_room && nifty_is_name( arg, victim->name ) )
		{
			found = true;
			pager_printf( ch, "[%5d] %-28s [%5d] %s\r\n",
				victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name );
		}
	}

	if( !found )
		act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

	return;
}


CMDF( do_bodybag )
{
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bodybag whom?\r\n", ch );
      return;
   }

   /*
    * check to see if vict is playing? 
    */
   snprintf( buf2, MAX_STRING_LENGTH, "the corpse of %s", arg );
   found = false;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && !str_cmp( buf2, obj->short_descr ) )
      {
         found = TRUE;
         ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\r\n",
                    obj->pIndexData->vnum, obj->short_descr, obj->in_room->vnum, obj->in_room->name );
         obj_from_room( obj );
         obj = obj_to_char( obj, ch );
         obj->timer = -1;
         save_char_obj( ch );
      }
   }

   if( !found )
      ch_printf( ch, " You couldn't find any %s\r\n", buf2 );
}


/* New owhere by Altrag, 03/14/96 */
CMDF( do_owhere )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	bool found;
	int icnt = 0;

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Owhere what?\r\n", ch );
		return;
	}
	argument = one_argument( argument, arg2 );

	set_pager_color( AT_PLAIN, ch );
	/*    if ( arg2[0] != '\0' && !str_prefix(arg2, "nesthunt") )
		{
			OBJ_DATA *tobj;
			int ncnt = 0;

			if( !( obj = get_obj_world( ch, arg ) ) )
			{
				send_to_char( "Nest hunt for what object?\r\n", ch );
				return;
			}

			for( obj = first_object; obj; obj = obj->next )
			{
				tobj = obj;
				if( !nifty_is_name( arg, obj->pIndexData->name ) )
					continue;
				for( ; tobj->in_obj; tobj = obj->in_obj )
				{
					pager_printf( ch, "[%5d] %-28s&w in object [%5d] %s%s&w\r\n", tobj->pIndexData->vnum,
						tobj->short_descr, tobj->in_obj->pIndexData->vnum, tobj->in_obj->short_descr );
					ncnt++;
				}

				snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s&w in", tobj->pIndexData->vnum, tobj->short_descr );
				if( obj->carried_by )
					pager_printf( ch, "%s invent [%5d] %s\r\n", buf, IS_NPC(obj->carried_by) ?
					obj->carried_by->pIndexData->vnum : 0, PERS( obj->carried_by, ch ) );
				else if( obj->in_room )
					pager_printf( ch, "%s room [%5d] %s\r\n", buf, obj->in_room->vnum,
					obj->in_room->name );
				else
				{
					bug( "%s: Nesthunt reports object at end of nest neither in room nor in inventory: %d",
						__FUNCTION__, tobj->pIndexData->vnum );
					pager_printf( ch, "%s ERROR, inform Halcyon immediately.\r\n", buf );
				}
				ncnt++;
				if( ncnt > 1 )
					pager_printf( ch, "Nested %d objects deep.\r\n", ncnt );
				icnt++;
			}
			pager_printf( ch, "Found %d matches.\r\n", icnt );
			return;
		}*/

	found = false;
	for( obj = first_object; obj; obj = obj->next )
	{
		if( !nifty_is_name( arg, obj->name ) )
			continue;
		found = true;

		snprintf( buf, MAX_STRING_LENGTH, "&w(%3d) [%5d] %-28s&w in ", ++icnt, obj->pIndexData->vnum, obj_short( obj ) );
		if( obj->carried_by )
			snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "invent [%5d] %s\r\n",
				( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
		else if( obj->in_room )
			snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "room   [%5d] %s\r\n", obj->in_room->vnum, obj->in_room->name );
		else if( obj->in_obj )
			snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "object [%5d] %s\r\n", obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
		else
		{
			bug( "do_owhere: object doesnt have location!", 0 );
			strcat( buf, "nowhere??\r\n" );
		}
		send_to_pager( buf, ch );
	}

	if( !found )
		act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	else
		pager_printf( ch, "%d matches.\r\n", icnt );

	return;
}

CMDF( do_reboo )
{
	send_to_char( "If you want to REBOOT, spell it out.\r\n", ch );
	return;
}

CMDF( do_reboot )
{
	char buf[MAX_STRING_LENGTH];
	extern bool mud_down;
	CHAR_DATA *vch;

	if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
	{
		send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\r\n", ch );
		return;
	}

	if( auction->item )
		do_auction( ch, "stop" );

	snprintf( buf, MAX_STRING_LENGTH, "Reboot by %s.", ch->name );
	do_echo( ch, buf );

	if( !str_cmp( argument, "and sort skill table" ) )
	{
		sort_skill_table( );
		save_skill_table( );
	}

	/*
	 * Save all characters before booting.
	 */
	if( str_cmp( argument, "nosave" ) )
		for( vch = first_char; vch; vch = vch->next )
			if( !IS_NPC( vch ) )
				save_char_obj( vch );

	mud_down = true;
	return;
}



CMDF( do_shutdow )
{
	send_to_char( "If you want to SHUTDOWN, spell it out.\r\n", ch );
	return;
}



CMDF( do_shutdown )
{
	char buf[MAX_STRING_LENGTH];
	extern bool mud_down;
	CHAR_DATA *vch;

	if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
	{
		send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\r\n", ch );
		return;
	}

	if( auction->item )
		do_auction( ch, "stop" );

	snprintf( buf, MAX_STRING_LENGTH, "Shutdown by %s.", ch->name );
	append_file( ch, SHUTDOWN_FILE, buf );
	strcat( buf, "\r\n" );
	do_echo( ch, buf );

	/*
	 * Save all characters before booting.
	 */
	if( str_cmp( argument, "nosave" ) )
		for( vch = first_char; vch; vch = vch->next )
			if( !IS_NPC( vch ) )
				save_char_obj( vch );
	mud_down = true;
	return;
}


CMDF( do_snoop )
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Snoop whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( !IS_SUPREME( ch ) && !str_cmp( victim->name, "Sedle" ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}


	if( !victim->desc )
	{
		send_to_char( "No descriptor to snoop.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Cancelling all snoops.\r\n", ch );
		for( d = first_descriptor; d; d = d->next )
			if( d->snoop_by == ch->desc )
				d->snoop_by = NULL;
		return;
	}

	if( victim->desc->snoop_by )
	{
		send_to_char( "Busy already.\r\n", ch );
		return;
	}

	/*
	 * Minimum snoop level... a secret mset value
	 * makes the snooper think that the victim is already being snooped
	 */
	if( get_trust( victim ) >= get_trust( ch ) || ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
	{
		send_to_char( "Busy already.\r\n", ch );
		return;
	}

	if( ch->desc )
	{
		for( d = ch->desc->snoop_by; d; d = d->snoop_by )
			if( d->character == victim || d->original == victim )
			{
				send_to_char( "No snoop loops.\r\n", ch );
				return;
			}
	}

	/*  Snoop notification for higher imms, if desired, uncomment this
		if ( get_trust(victim) > LEVEL_LIAISON && get_trust(ch) < LEVEL_OWNER )
		  write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\r\n", 0 );
	*/
	victim->desc->snoop_by = ch->desc;
	send_to_char( "Ok.\r\n", ch );
	return;
}



CMDF( do_switch )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Switch into whom?\r\n", ch );
		return;
	}

	if( !ch->desc )
		return;

	if( ch->desc->original )
	{
		send_to_char( "You are already switched.\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( victim->desc )
	{
		send_to_char( "Character in use.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
	{
		send_to_char( "You cannot switch into a player!\r\n", ch );
		return;
	}

	ch->desc->character = victim;
	ch->desc->original = ch;
	victim->desc = ch->desc;
	ch->desc = NULL;
	ch->switched = victim;
	send_to_char( "Ok.\r\n", victim );
	return;
}



CMDF( do_return )
{
	if( !ch->desc )
		return;

	if( !ch->desc->original )
	{
		send_to_char( "You aren't switched.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->act, ACT_POLYMORPHED ) )
	{
		send_to_char( "Use revert to return from a polymorphed mob.\r\n", ch );
		return;
	}

	send_to_char( "You return to your original body.\r\n", ch );
	if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
	{
		affect_strip( ch, gsn_possess );
		REMOVE_BIT( ch->affected_by, AFF_POSSESS );
	}
	/*    if ( IS_NPC( ch->desc->character ) )
		  REMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
	ch->desc->character = ch->desc->original;
	ch->desc->original = NULL;
	ch->desc->character->desc = ch->desc;
	ch->desc->character->switched = NULL;
	ch->desc = NULL;
	return;
}



CMDF( do_minvoke )
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	int vnum;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: minvoke <vnum>.\r\n", ch );
		return;
	}

	if( !is_number( arg ) )
	{
		char arg2[MAX_INPUT_LENGTH];
		int hash, cnt;
		int count = number_argument( arg, arg2 );

		vnum = -1;
		for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
			for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
				if( nifty_is_name( arg2, pMobIndex->player_name ) && ++cnt == count )
				{
					vnum = pMobIndex->vnum;
					break;
				}
		if( vnum == -1 )
		{
			send_to_char( "No such mobile exists.\r\n", ch );
			return;
		}
	}
	else
		vnum = atoi( arg );

	if( get_trust( ch ) < LEVEL_LIAISON )
	{
		AREA_DATA *pArea;

		if( IS_NPC( ch ) )
		{
			send_to_char( "Huh?\r\n", ch );
			return;
		}

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to invoke this mobile.\r\n", ch );
			return;
		}
		if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
		{
			send_to_char( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}

	if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
	{
		send_to_char( "No mobile has that vnum.\r\n", ch );
		return;
	}

	victim = create_mobile( pMobIndex );
	char_to_room( victim, ch->in_room );
	act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
	send_to_char( "Ok.\r\n", ch );
	return;
}



CMDF( do_oinvoke )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int vnum;
	int level;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: oinvoke <vnum> <level>.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		level = get_trust( ch );
	}
	else
	{
		if( !is_number( arg2 ) )
		{
			send_to_char( "Syntax: oinvoke <vnum> <level>.\r\n", ch );
			return;
		}
		level = atoi( arg2 );
		if( level < 0 || level > get_trust( ch ) )
		{
			send_to_char( "Limited to your trust level.\r\n", ch );
			return;
		}
	}

	if( !is_number( arg1 ) )
	{
		char arg[MAX_INPUT_LENGTH];
		int hash, cnt;
		int count = number_argument( arg1, arg );

		vnum = -1;
		for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
			for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
				if( nifty_is_name( arg, pObjIndex->name ) && ++cnt == count )
				{
					vnum = pObjIndex->vnum;
					break;
				}
		if( vnum == -1 )
		{
			send_to_char( "No such object exists.\r\n", ch );
			return;
		}
	}
	else
		vnum = atoi( arg1 );

	if( get_trust( ch ) < LEVEL_LIAISON )
	{
		AREA_DATA *pArea;

		if( IS_NPC( ch ) )
		{
			send_to_char( "Huh?\r\n", ch );
			return;
		}

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to invoke this object.\r\n", ch );
			return;
		}
		if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
		{
			send_to_char( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char( "No object has that vnum.\r\n", ch );
		return;
	}

	/* Commented out by Narn, it seems outdated
		if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
		&&	 pObjIndex->count > 5 )
		{
		send_to_char( "That object is at its limit.\r\n", ch );
		return;
		}
	*/

	obj = create_object( pObjIndex, level );
	if( CAN_WEAR( obj, ITEM_TAKE ) )
	{
		if( !IS_SUPREME( ch ) && IS_OBJ_STAT( obj, ITEM_NOINVOKE ) )
		{
			send_to_char( "Don't even try, Putz.\r\n", ch );
			snprintf( buf, MAX_STRING_LENGTH, "%s just tried to oinvoke %s[%d].", ch->name, obj->short_descr, obj->pIndexData->vnum );
			log_string( buf );
			extract_obj( obj );
			return;
		}
		obj = obj_to_char( obj, ch );
	}
	else
	{
		if( !IS_SUPREME( ch ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			send_to_char( "Don't even try, Putz.\r\n", ch );
			snprintf( buf, MAX_STRING_LENGTH, "%s just tried to oinvoke %s[%d].", ch->name, obj->short_descr, obj->pIndexData->vnum );
			log_string( buf );
			extract_obj( obj );
			return;
		}
		obj = obj_to_room( obj, ch->in_room );
		act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
	}
	send_to_char( "Ok.\r\n", ch );
	return;
}



CMDF( do_purge )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		/*
		 * 'purge'
		 */
		CHAR_DATA *vnext;
		OBJ_DATA *obj_next;

		for( victim = ch->in_room->first_person; victim; victim = vnext )
		{
			vnext = victim->next_in_room;
			if( IS_NPC( victim ) && victim != ch && !xIS_SET( victim->act, ACT_POLYMORPHED ) )
				extract_char( victim, true );
		}

		for( obj = ch->in_room->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if( obj->item_type == ITEM_SPACECRAFT )
				continue;
			extract_obj( obj );
		}

		act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM );
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	victim = NULL;
	obj = NULL;

	/*
	 * fixed to get things in room first -- i.e., purge portal (obj),
	 * * no more purging mobs with that keyword in another room first
	 * * -- Tri
	 */
	if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
		if( ( victim = get_char_world( ch, arg ) ) == NULL && ( obj = get_obj_world( ch, arg ) ) == NULL )    /* no get_obj_room */
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}

	/* Single object purge in room for high level purge - Scryn 8/12*/
	if( obj )
	{
		separate_obj( obj );
		act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM );
		act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
		extract_obj( obj );
		return;
	}


	if( !IS_NPC( victim ) )
	{
		send_to_char( "Not on PC's.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You cannot purge yourself!\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, ACT_POLYMORPHED ) )
	{
		send_to_char( "You cannot purge a polymorphed player.\r\n", ch );
		return;
	}
	act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
	extract_char( victim, true );
	return;
}


CMDF( do_low_purge )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Purge what?\r\n", ch );
		return;
	}

	victim = NULL;
	obj = NULL;
	if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
		send_to_char( "You can't find that here.\r\n", ch );
		return;
	}

	if( obj )
	{
		separate_obj( obj );
		act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
		act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
		extract_obj( obj );
		return;
	}

	if( !IS_NPC( victim ) )
	{
		send_to_char( "Not on PC's.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You cannot purge yourself!\r\n", ch );
		return;
	}

	act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
	extract_char( victim, true );
	return;
}


CMDF( do_balzhur )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	AREA_DATA *pArea;
	int sn;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Who is deserving of such a fate?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't playing.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "I wouldn't even think of that if I were you...\r\n", ch );
		return;
	}

	set_char_color( AT_WHITE, ch );
	send_to_char( "You summon the demon Balzhur to wreak your wrath!\r\n", ch );
	send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\r\n", ch );
	set_char_color( AT_IMMORT, victim );
	send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\r\n", victim );
	snprintf( buf, MAX_STRING_LENGTH, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	victim->top_level = 1;
	victim->trust = 0;
	{
		int ability;

		for( ability = 0; ability < MAX_ABILITY; ability++ )
		{
			victim->experience[ability] = 1;
			victim->skill_level[ability] = 1;
		}
	}
	victim->max_hit = 500;
	victim->max_move = 1000;
	for( sn = 0; sn < top_sn; sn++ )
		victim->pcdata->learned[sn] = 0;
	victim->hit = victim->max_hit;
	victim->move = victim->max_move;


	snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, capitalize( victim->name ) );

	if( !remove( buf ) )
		send_to_char( "Player's immortal data destroyed.\r\n", ch );
	else if( errno != ENOENT )
	{
		ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\r\n", errno, strerror( errno ) );
		snprintf( buf2, MAX_STRING_LENGTH, "%s balzhuring %s", ch->name, buf );
		perror( buf2 );
	}
	snprintf( buf2, MAX_STRING_LENGTH, "%s.are", capitalize( arg ) );
	for( pArea = first_build; pArea; pArea = pArea->next )
		if( !strcmp( pArea->filename, buf2 ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s%s", BUILD_DIR, buf2 );
			if( IS_SET( pArea->status, AREA_LOADED ) )
				fold_area( pArea, buf, false );
			close_area( pArea );
			snprintf( buf2, MAX_STRING_LENGTH, "%s.bak", buf );
			set_char_color( AT_RED, ch );  /* Log message changes colors */
			if( !rename( buf, buf2 ) )
				send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
			else if( errno != ENOENT )
			{
				ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\r\n", errno, strerror( errno ) );
				snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
				perror( buf2 );
			}
		}


	make_wizlist( );
	do_help( victim, "M_BALZHUR_" );
	set_char_color( AT_WHITE, victim );
	send_to_char( "You awake after a long period of time...\r\n", victim );
	while( victim->first_carrying )
		extract_obj( victim->first_carrying );
	return;
}

CMDF( do_advance )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level, ability;
	int iLevel, iAbility;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) )
	{
		send_to_char( "Syntax: advance <char> <ability> <level>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	ability = -1;
	for( iAbility = 0; iAbility < MAX_ABILITY; iAbility++ )
	{
		if( !str_prefix( arg3, ability_name[iAbility] ) )
		{
			ability = iAbility;
			break;
		}
	}

	if( ability == -1 )
	{
		send_to_char( "No Such Ability.\r\n", ch );
		do_advance( ch, "" );
		return;
	}


	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	/*
	 * You can demote yourself but not someone else at your own trust. -- Narn
	 */
	if( get_trust( ch ) <= get_trust( victim ) && ch != victim )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( ( level = atoi( arg2 ) ) < 1 || level > 1000 )
	{
		send_to_char( "Level must be 1 to 1000.\r\n", ch );
		return;
	}

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if( level <= victim->skill_level[ability] )
	{
		send_to_char( "Lowering a player's level!\r\n", ch );
		set_char_color( AT_IMMORT, victim );
		send_to_char( "Cursed and forsaken! The gods have lowered your level.\r\n", victim );
		victim->experience[ability] = 0;
		victim->skill_level[ability] = 1;
		if( ability == COMBAT_ABILITY )
			victim->max_hit = 500;
	}
	else
	{
		send_to_char( "Raising a player's level!\r\n", ch );
		send_to_char( "The gods feel fit to raise your level!\r\n", victim );
		snprintf( log_buf, MAX_STRING_LENGTH, "%s advanced %s in %s for %s levels. From level %d.", ch->name, victim->name, arg3, arg2, victim->skill_level[ability] );
		log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
	}

	for( iLevel = victim->skill_level[ability]; iLevel < level; iLevel++ )
	{
		victim->experience[ability] = exp_level( iLevel + 1 );
		gain_exp( victim, 0, ability );
	}
	return;
}

/* bookmark */
CMDF( do_immortalize )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: immortalize <char>\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( victim->top_level != LEVEL_AVATAR )
	{
		send_to_char( "This player is not worthy of immortality yet.\r\n", ch );
		return;
	}

	send_to_char( "Immortalizing a player...\r\n", ch );
	set_char_color( AT_IMMORT, victim );
	act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...", ch, NULL, NULL, TO_ROOM );
	set_char_color( AT_WHITE, victim );
	send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
	set_char_color( AT_LBLUE, victim );

	//    do_help(victim, "M_GODLVL1_" );
	set_char_color( AT_WHITE, victim );
	send_to_char( "You awake... all your possessions are gone.\r\n", victim );
	while( victim->first_carrying )
		extract_obj( victim->first_carrying );

	victim->top_level = LEVEL_STAFF;

	/*    advance_level( victim );  */

	victim->trust = 0;
	return;
}



CMDF( do_trust )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
	{
		send_to_char( "Syntax: trust <char> <level>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( ( level = atoi( arg2 ) ) < 1001 || level > 1007 )
	{
		send_to_char( "Level must be 0 (reset) or 1001 to 1007.\r\n", ch );
		return;
	}

	if( level > get_trust( ch ) )
	{
		send_to_char( "Limited to your own trust.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	victim->trust = level;
	send_to_char( "Ok.\r\n", ch );
	return;
}



CMDF( do_restore )
{
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Restore whom?\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "all" ) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if( !ch->pcdata )
			return;

		if( get_trust( ch ) < LEVEL_BUILDER )
		{
			if( IS_NPC( ch ) )
			{
				send_to_char( "You can't do that.\r\n", ch );
				return;
			}
			else
			{
				/*
				 * Check if the player did a restore all within the last 18 hours.
				 */
				if( current_time - last_restore_all_time < RESTORE_INTERVAL )
				{
					send_to_char( "Sorry, you can't do a restore all yet.\r\n", ch );
					do_restoretime( ch, "" );
					return;
				}
			}
		}
		last_restore_all_time = current_time;
		ch->pcdata->restore_time = current_time;
		save_char_obj( ch );
		send_to_char( "Ok.\r\n", ch );
		for( vch = first_char; vch; vch = vch_next )
		{
			vch_next = vch->next;

			/*		if( !has_rate( vch, RATE_RESTORE_ALL ) )
						continue;*/

			if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
			{
				vch->hit = vch->max_hit;
				vch->move = vch->max_move;
				vch->pcdata->condition[COND_FULL] = 100;
				vch->pcdata->condition[COND_THIRST] = 100;
				vch->mental_state = 0;
				if( IS_SET( vch->pcdata->cybaflags, CYBA_HASGUN ) )
				{
					vch->pcdata->xenergy = vch->pcdata->xenergymax;
				}

				update_pos( vch );
				act( AT_IMMORT,
					"&W$n &Yg&Oi&Yv&Oe&Ys &Cy&co&Cu &Rs&ro&Rm&re &BG&bu&zn&Wd&Ba&Wn&zi&bu&Bm &GE&gl&cix&ge&Gr&Y!&O!&Y!", ch,
					NULL, vch, TO_VICT );
			}
		}
	}
	else
	{

		CHAR_DATA *victim;

		if( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if( get_trust( ch ) < LEVEL_LIAISON && victim != ch
			&& !( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}

		victim->hit = victim->max_hit;
		victim->move = victim->max_move;
		victim->mental_state = 0;
		if( !IS_NPC( victim ) && IS_SET( victim->pcdata->cybaflags, CYBA_HASGUN ) )
		{
			victim->pcdata->xenergy = victim->pcdata->xenergymax;
		}

		if( victim->pcdata )
		{
			victim->pcdata->condition[COND_BLOODTHIRST] = ( 10 + victim->top_level );
			victim->pcdata->condition[COND_FULL] = 100;
			victim->pcdata->condition[COND_THIRST] = 100;
		}
		update_pos( victim );
		if( ch != victim )
			act( AT_IMMORT, "&W$n &Yg&Oi&Yv&Oe&Ys &Cy&co&Cu &Rs&ro&Rm&re &BG&bu&zn&Wd&Ba&Wn&zi&bu&Bm &GE&gl&cix&ge&Gr&Y!&O!&Y!",
				ch, NULL, victim, TO_VICT );
		send_to_char( "Ok.\r\n", ch );
		return;
	}
}

CMDF( do_restoretime )
{
	long int time_passed;
	int hour, minute;

	if( !last_restore_all_time )
		ch_printf( ch, "There has been no restore all since reboot\r\n" );
	else
	{
		time_passed = current_time - last_restore_all_time;
		hour = ( int ) ( time_passed / 3600 );
		minute = ( int ) ( ( time_passed - ( hour * 3600 ) ) / 60 );
		ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
	}

	if( !ch->pcdata )
		return;

	if( !ch->pcdata->restore_time )
	{
		send_to_char( "You have never done a restore all.\r\n", ch );
		return;
	}

	time_passed = current_time - ch->pcdata->restore_time;
	hour = ( int ) ( time_passed / 3600 );
	minute = ( int ) ( ( time_passed - ( hour * 3600 ) ) / 60 );
	ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
	return;
}

CMDF( do_freeze )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Freeze whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_FREEZE ) )
	{
		xREMOVE_BIT( victim->act, PLR_FREEZE );
		send_to_char( "You can play again.\r\n", victim );
		send_to_char( "FREEZE removed.\r\n", ch );
	}
	else
	{
		xSET_BIT( victim->act, PLR_FREEZE );
		send_to_char( "You can't do ANYthing!\r\n", victim );
		send_to_char( "FREEZE set.\r\n", ch );
	}

	save_char_obj( victim );

	return;
}



CMDF( do_log )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Log whom?\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "all" ) )
	{
		if( fLogAll )
		{
			fLogAll = false;
			send_to_char( "Log ALL off.\r\n", ch );
		}
		else
		{
			fLogAll = true;
			send_to_char( "Log ALL on.\r\n", ch );
		}
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	if( xIS_SET( victim->act, PLR_LOG ) )
	{
		xREMOVE_BIT( victim->act, PLR_LOG );
		send_to_char( "LOG removed.\r\n", ch );
	}
	else
	{
		xSET_BIT( victim->act, PLR_LOG );
		send_to_char( "LOG set.\r\n", ch );
	}

	return;
}


CMDF( do_litterbug )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Set litterbug flag on whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_LITTERBUG ) )
	{
		xREMOVE_BIT( victim->act, PLR_LITTERBUG );
		send_to_char( "You can drop items again.\r\n", victim );
		send_to_char( "LITTERBUG removed.\r\n", ch );
	}
	else
	{
		xSET_BIT( victim->act, PLR_LITTERBUG );
		send_to_char( "You a strange force prevents you from dropping any more items!\r\n", victim );
		send_to_char( "LITTERBUG set.\r\n", ch );
	}

	return;
}


CMDF( do_noemote )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Noemote whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
	{
		xREMOVE_BIT( victim->act, PLR_NO_EMOTE );
		send_to_char( "You can emote again.\r\n", victim );
		send_to_char( "NO_EMOTE removed.\r\n", ch );
	}
	else
	{
		xSET_BIT( victim->act, PLR_NO_EMOTE );
		send_to_char( "You can't emote!\r\n", victim );
		send_to_char( "NO_EMOTE set.\r\n", ch );
	}

	return;
}

CMDF( do_noooc )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Noooc whom?", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}
	if( IS_SET( victim->pcdata->flags, PCFLAG_NO_OOC ) )
	{
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_NO_OOC );
		send_to_char( "You can use OOC again.\r\n", victim );
		send_to_char( "NO_OOC removed.\r\n", ch );
	}
	else
	{
		SET_BIT( victim->pcdata->flags, PCFLAG_NO_OOC );
		send_to_char( "You can't use OOC!\r\n", victim );
		send_to_char( "NO_OOC set.\r\n", ch );
	}

	return;
}



CMDF( do_notell )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Notell whom?", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_NO_TELL ) )
	{
		xREMOVE_BIT( victim->act, PLR_NO_TELL );
		send_to_char( "You can tell again.\r\n", victim );
		send_to_char( "NO_TELL removed.\r\n", ch );
	}
	else
	{
		xSET_BIT( victim->act, PLR_NO_TELL );
		send_to_char( "You can't tell!\r\n", victim );
		send_to_char( "NO_TELL set.\r\n", ch );
	}

	return;
}


CMDF( do_notitle )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Notitle whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
	{
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
		send_to_char( "You can set your own title again.\r\n", victim );
		send_to_char( "NOTITLE removed.\r\n", ch );
	}
	else
	{
		SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
		snprintf( buf, MAX_STRING_LENGTH, "%s", victim->name );
		set_title( victim, buf );
		send_to_char( "You can't set your own title!\r\n", victim );
		send_to_char( "NOTITLE set.\r\n", ch );
	}

	return;
}

CMDF( do_silence )
{
	char arg[MAX_INPUT_LENGTH];
	//    char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	//    short time;

	one_argument( argument, arg );
	//    one_argument( argument, arg2 );

	one_argument( argument, arg3 );

	if( arg[0] == '\0' )
	{
		send_to_char( "Silence whom <time?> <minutes/hour>?", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_SILENCE ) )
	{
		send_to_char( "Player already silenced, use unsilence to remove.\r\n", ch );
		return;
	}
	else
	{
		xSET_BIT( victim->act, PLR_SILENCE );
		send_to_char( "You can't use channels!\r\n", victim );
		send_to_char( "SILENCE set.\r\n", ch );
	}
	return;
}

CMDF( do_unsilence )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Unsilence whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_SILENCE ) )
	{
		xREMOVE_BIT( victim->act, PLR_SILENCE );
		send_to_char( "You can use channels again.\r\n", victim );
		send_to_char( "SILENCE removed.\r\n", ch );
	}
	else
	{
		send_to_char( "That player is not silenced.\r\n", ch );
	}

	return;
}




CMDF( do_peace )
{
	CHAR_DATA *rch;

	act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
	for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
	{
		if( rch->fighting )
		{
			stop_fighting( rch, true );
			do_sit( rch, "" );
		}

		/*
		 * Added by Narn, Nov 28/95
		 */
		stop_hating( rch );
		stop_hunting( rch );
		stop_fearing( rch );
	}

	send_to_char( "Ok.\r\n", ch );
	return;
}



BAN_DATA *first_ban;
BAN_DATA *last_ban;

void save_banlist( void )
{
	BAN_DATA *pban;
	FILE *fp;

	if( !( fp = FileOpen( SYSTEM_DIR BAN_LIST, "w" ) ) )
	{
		bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
		perror( BAN_LIST );
		return;
	}
	for( pban = first_ban; pban; pban = pban->next )
		fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );
	fprintf( fp, "-1\n" );
	FileClose( fp );
	return;
}



CMDF( do_ban )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	BAN_DATA *pban;
	int bnum;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg );
	if( !str_cmp( arg, "rr.com" ) )
	{
		send_to_char( "&RNo ya don't!\r\n", ch );
		return;
	}
	set_pager_color( AT_PLAIN, ch );
	if( arg[0] == '\0' )
	{
		send_to_pager( "Syntax: ban <site address>\n\rSyntax: ban <ban number> <level <lev>|newban|mortal|total>\r\n", ch );
		send_to_pager( "Banned sites:\r\n", ch );
		send_to_pager( "[ #] (Lv) Time                     Site\r\n", ch );
		send_to_pager( "---- ---- ------------------------ ---------------\r\n", ch );
		for( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
			pager_printf( ch, "[%2d] (%2d) %-24s %s\r\n", bnum, pban->level, pban->ban_time, pban->name );
		return;
	}

	/*
	 * People are gonna need .# instead of just # to ban by just last
	 * number in the site ip.                               -- Altrag
	 */
	if( is_number( arg ) )
	{
		for( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
			if( bnum == atoi( arg ) )
				break;
		if( !pban )
		{
			do_ban( ch, "" );
			return;
		}
		argument = one_argument( argument, arg );
		if( arg[0] == '\0' )
		{
			do_ban( ch, "help" );
			return;
		}
		if( !str_cmp( arg, "level" ) )
		{
			argument = one_argument( argument, arg );
			if( arg[0] == '\0' || !is_number( arg ) )
			{
				do_ban( ch, "help" );
				return;
			}
			if( atoi( arg ) < 1 || atoi( arg ) > LEVEL_OWNER )
			{
				ch_printf( ch, "Level range: 1 - %d.\r\n", LEVEL_OWNER );
				return;
			}
			pban->level = atoi( arg );
			send_to_char( "Ban level set.\r\n", ch );
		}
		else if( !str_cmp( arg, "newban" ) )
		{
			pban->level = 1;
			send_to_char( "New characters banned.\r\n", ch );
		}
		else if( !str_cmp( arg, "mortal" ) )
		{
			pban->level = LEVEL_AVATAR;
			send_to_char( "All mortals banned.\r\n", ch );
		}
		else if( !str_cmp( arg, "total" ) )
		{
			pban->level = LEVEL_OWNER;
			send_to_char( "Everyone banned.\r\n", ch );
		}
		else
		{
			do_ban( ch, "help" );
			return;
		}
		save_banlist( );
		return;
	}

	if( !str_cmp( arg, "help" ) )
	{
		send_to_char( "Syntax: ban <site address>\r\n", ch );
		send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|" "total>\r\n", ch );
		return;
	}

	for( pban = first_ban; pban; pban = pban->next )
	{
		if( !str_cmp( arg, pban->name ) )
		{
			send_to_char( "That site is already banned!\r\n", ch );
			return;
		}
	}

	CREATE( pban, BAN_DATA, 1 );
	LINK( pban, first_ban, last_ban, next, prev );
	pban->name = str_dup( arg );
	pban->level = LEVEL_AVATAR;
	snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
	pban->ban_time = str_dup( buf );
	save_banlist( );
	send_to_char( "Ban created.  Mortals banned from site.\r\n", ch );
	return;
}


CMDF( do_allow )
{
	char arg[MAX_INPUT_LENGTH];
	BAN_DATA *pban;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Remove which site from the ban list?\r\n", ch );
		return;
	}

	for( pban = first_ban; pban; pban = pban->next )
	{
		if( !str_cmp( arg, pban->name ) )
		{
			UNLINK( pban, first_ban, last_ban, next, prev );
			DISPOSE( pban->ban_time );
			DISPOSE( pban->name );
			DISPOSE( pban );
			save_banlist( );
			send_to_char( "Site no longer banned.\r\n", ch );
			return;
		}
	}

	send_to_char( "Site is not banned.\r\n", ch );
	return;
}



CMDF( do_wizlock )
{
	extern bool wizlock;
	wizlock = !wizlock;

	if( wizlock )
		send_to_char( "Game wizlocked.\r\n", ch );
	else
		send_to_char( "Game un-wizlocked.\r\n", ch );

	return;
}


CMDF( do_noresolve )
{
	sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

	if( sysdata.NO_NAME_RESOLVING )
		send_to_char( "Name resolving disabled.\r\n", ch );
	else
		send_to_char( "Name resolving enabled.\r\n", ch );

	return;
}


CMDF( do_users )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int count;
	const char *st;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	count = 0;
	buf[0] = '\0';

	send_to_char( "\r\n", ch );
	set_pager_color( AT_PLAIN, ch );
	snprintf( buf, MAX_STRING_LENGTH, "\r\n&B&W[&zNum    Con-State  Idle&W] &wPlayer         Host\r\n" );
	strcat( buf, "&B&W--------------------------------------------------------------------------\r\n" );
	send_to_pager( buf, ch );

	for( d = first_descriptor; d; d = d->next )
	{

		switch( d->connected )
		{
		case CON_PLAYING:
			st = "    PLAYING    ";
			break;
		case CON_GET_NAME:
			st = "   Get Name    ";
			break;
		case CON_AGREEMENT:
			st = "   AGREEMENT   ";
			break;
		case CON_GET_OLD_PASSWORD:
			st = "Get Old Passwd ";
			break;
		case CON_CONFIRM_NEW_NAME:
			st = " Confirm Name  ";
			break;
		case CON_GET_NEW_PASSWORD:
			st = "Get New Passwd ";
			break;
		case CON_CONFIRM_NEW_PASSWORD:
			st = "Confirm Passwd ";
			break;
		case CON_GET_NEW_RACE:
			st = "  Get New Race ";
			break;
		case CON_GET_NEW_SEX:
			st = "  Get New Gender  ";
			break;
		case CON_GET_NEW_CLASS:
			st = " Get New Class ";
			break;
		case CON_GET_HAIR:
			st = " Get New Hair  ";
			break;
		case CON_GET_EYE:
			st = " Get New Eye   ";
			break;
		case CON_GET_BUILD:
			st = " Get New Build ";
			break;
		case CON_EDITING:
			st = "    Editing    ";
			break;
		case CON_PRESS_ENTER:
			st = "  Press Enter  ";
			break;
		case CON_GET_HIGHLIGHT:
			st = " Get Highlight ";
			break;
		case CON_GET_HERO:
			st = " Get Hero      ";
			break;
		case CON_ROLL_STATS:
			st = " Rolling Stats ";
			break;
		case CON_STATS_OK:
			st = "  Stats Okay   ";
			break;
			//              case CON_DEFAULT_CHOICE:     st = " Choosing Cust ";    break;
			//              case CON_GEN_GROUPS:         st = " Customization ";    break;
			//              case CON_PICK_WEAPON:        st = " Picking Weapon";    break;
		case CON_READ_IMOTD:
			st = " Reading IMOTD ";
			break;
		case CON_GET_WANT_RIPANSI:
			st = "  ANSI CHOICE  ";
			break;
		case CON_READ_MOTD:
			st = "  Reading MOTD ";
			break;
		case CON_NOTE_EXPIRE:
			st = "  Writing Note ";
			break;
		case CON_NOTE_TO:
			st = "  Writing Note ";
			break;
		case CON_NOTE_TEXT:
			st = "  Writing Note ";
			break;
		case CON_NOTE_SUBJECT:
			st = "  Writing Note ";
			break;
		case CON_NOTE_FINISH:
			st = "  Writing Note ";
			break;
		default:
			st = "   !UNKNOWN!   ";
			break;
		}

		if( arg[0] == '\0' )
		{
			if( get_trust( ch ) >= LEVEL_OWNER || ( d->character && can_see( ch, d->character ) ) )
			{
				count++;
				snprintf( buf, MAX_STRING_LENGTH,
					"&B[&c%3d &C%s &c%3d&B] &Y%-12s &R%s",
					d->descriptor,
					st,
					( ( d->idle / 240 ) % 1000 ),
					d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host );
				strcat( buf, "\r\n" );
				send_to_pager( buf, ch );
			}
		}
		else
		{
			if( ( get_trust( ch ) >= LEVEL_OWNER
				|| ( d->character && can_see( ch, d->character ) ) )
				&& ( !str_prefix( arg, d->host ) || ( d->character && !str_prefix( arg, d->character->name ) ) ) )
			{
				count++;
				pager_printf( ch,
					" %3d| %2d|%4d|%6d| %-12s@%-16s ",
					d->descriptor,
					d->connected,
					d->idle / 4,
					d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host );
				buf[0] = '\0';
				if( get_trust( ch ) >= LEVEL_LIAISON )
					snprintf( buf, MAX_STRING_LENGTH, "| %s", d->user );
				strcat( buf, "\r\n" );
				send_to_pager( buf, ch );
			}
		}
	}
	pager_printf( ch, "%d user%s.\r\n", count, count == 1 ? "" : "s" );
	return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
CMDF( do_force )
{
	char arg[MAX_INPUT_LENGTH];
	bool mobsonly;
	argument = one_argument( argument, arg );

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Force whom to do what?\r\n", ch );
		return;
	}

	mobsonly = get_trust( ch ) < sysdata.level_forcepc;

	if( !str_cmp( arg, "all" ) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if( mobsonly )
		{
			send_to_char( "Force whom to do what?\r\n", ch );
			return;
		}

		for( vch = first_char; vch; vch = vch_next )
		{
			vch_next = vch->next;

			if( ch->desc && ch->desc->connected == CON_PLAYING )
			{
				if( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
				{
					//      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
					interpret( vch, argument );
				}
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		if( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if( victim == ch )
		{
			send_to_char( "Aye aye, right away!\r\n", ch );
			return;
		}

		if( ( get_trust( victim ) >= get_trust( ch ) ) || ( mobsonly && !IS_NPC( victim ) ) )
		{
			send_to_char( "Do it yourself!\r\n", ch );
			return;
		}

		//    act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
		interpret( victim, argument );
	}

	send_to_char( "Ok.\r\n", ch );
	return;
}


CMDF( do_invis )
{
	char arg[MAX_INPUT_LENGTH];
	short level;

	/*
	 * if ( IS_NPC(ch))
	 * return;
	 */

	argument = one_argument( argument, arg );
	if( arg[0] != '\0' )
	{
		if( !is_number( arg ) )
		{
			send_to_char( "Usage: invis | invis <level>\r\n", ch );
			return;
		}
		level = atoi( arg );
		if( level < 2 || level > get_trust( ch ) )
		{
			send_to_char( "Invalid level.\r\n", ch );
			return;
		}

		if( !IS_NPC( ch ) )
		{
			ch->pcdata->wizinvis = level;
			ch_printf( ch, "Wizinvis level set to %d.\r\n", level );
		}

		if( IS_NPC( ch ) )
		{
			ch->mobinvis = level;
			ch_printf( ch, "Mobinvis level set to %d.\r\n", level );
		}
		return;
	}

	if( !IS_NPC( ch ) )
	{
		if( ch->pcdata->wizinvis < 2 )
			ch->pcdata->wizinvis = ch->top_level;
	}

	if( IS_NPC( ch ) )
	{
		if( ch->mobinvis < 2 )
			ch->mobinvis = ch->top_level;
		return;
	}

	if( ch->pcdata->wizinvis < 2 )
		ch->pcdata->wizinvis = ch->top_level;

	if( xIS_SET( ch->act, PLR_WIZINVIS ) )
	{
		xREMOVE_BIT( ch->act, PLR_WIZINVIS );
		act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly fade back into existence.\r\n", ch );
	}
	else
	{
		xSET_BIT( ch->act, PLR_WIZINVIS );
		act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly vanish into thin air.\r\n", ch );
	}

	return;
}


CMDF( do_holylight )
{
	if( IS_NPC( ch ) )
		return;

	if( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
	{
		xREMOVE_BIT( ch->act, PLR_HOLYLIGHT );
		send_to_char( "Holy light mode off.\r\n", ch );
	}
	else
	{
		xSET_BIT( ch->act, PLR_HOLYLIGHT );
		send_to_char( "Holy light mode on.\r\n", ch );
	}

	return;
}

bool check_area_conflict( AREA_DATA *area, int low_range, int hi_range )
{
	if( low_range < area->low_vnum && area->low_vnum < hi_range )
		return true;

	if( low_range < area->hi_vnum && area->hi_vnum < hi_range )
		return true;

	if( ( low_range >= area->low_vnum ) && ( low_range <= area->hi_vnum ) )
		return true;

	if( ( hi_range <= area->hi_vnum ) && ( hi_range >= area->low_vnum ) )
		return true;

	return false;
}

/* Runs the entire list, easier to call in places that have to check them all */
bool check_area_conflicts( int lo, int hi )
{
	AREA_DATA *area;

	for( area = first_area; area; area = area->next )
		if( check_area_conflict( area, lo, hi ) )
			return true;
	return false;
}

void process_sorting( AREA_DATA *tarea, bool isproto );

CMDF( do_vassign )
{
	char arg1[MIL], arg2[MIL], arg3[MIL];
	int lo = -1, hi = -1;
	CHAR_DATA *victim, *mob;
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	AREA_DATA *tarea;
	char filename[256];

	set_char_color( AT_IMMORT, ch );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	lo = atoi( arg2 );
	hi = atoi( arg3 );

	if( arg1[0] == '\0' || lo < 0 || hi < 0 )
	{
		ch_printf( ch, "Syntax: vassign <who> <low> <high>\r\n" );
		return;
	}

	if( !( victim = get_char_world( ch, arg1 ) ) )
	{
		ch_printf( ch, "They don't seem to be around.\r\n" );
		return;
	}

	if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_STAFF )
	{
		ch_printf( ch, "They wouldn't know what to do with a vnum range.\r\n" );
		return;
	}

	if( lo == 0 && hi == 0 )
	{
		if( victim->pcdata->area )
			SET_BIT( victim->pcdata->area->status, AREA_DELETED );
		victim->pcdata->area = NULL;
		victim->pcdata->low_vnum = 0;
		victim->pcdata->hi_vnum = 0;
		ch_printf( victim, "%s has removed your vnum range.\r\n", ch->name );
		save_char_obj( victim );
		return;
	}

	if( victim->pcdata->area && lo != 0 )
	{
		ch_printf( ch, "You cannot assign them a range, they already have one!\r\n" );
		return;
	}

	if( lo == 0 && hi != 0 )
	{
		ch_printf( ch, "Unacceptable vnum range, low vnum cannot be 0 when hi vnum is not.\r\n" );
		return;
	}

	if( lo > hi )
	{
		ch_printf( ch, "Unacceptable vnum range, low vnum must be smaller than high vnum.\r\n" );
		return;
	}

	if( check_area_conflicts( lo, hi ) )
	{
		ch_printf( ch, "That vnum range conflicts with another area. Check the zones or vnums command.\r\n" );
		return;
	}

	victim->pcdata->low_vnum = lo;
	victim->pcdata->hi_vnum = hi;
	assign_area( victim );
	ch_printf( ch, "Done.\r\n" );
	ch_printf( victim, "%s has assigned you the vnum range %d - %d.\r\n", ch->name, lo, hi );
	assign_area( victim );  /* Put back by Thoric on 02/07/96 */
	if( !victim->pcdata->area )
	{
		bug( "%s: assign_area failed", __FUNCTION__ );
		return;
	}

	tarea = victim->pcdata->area;

	/*
	 * Initialize first and last rooms in range
	 */
	if( !( room = make_room( lo, tarea ) ) )
	{
		bug( "%s: make_room failed to initialize first room.", __FUNCTION__ );
		return;
	}

	if( !( room = make_room( hi, tarea ) ) )
	{
		bug( "%s: make_room failed to initialize last room.", __FUNCTION__ );
		return;
	}

	/*
	 * Initialize first mob in range
	 */
	if( !( pMobIndex = make_mobile( lo, 0, "first mob" ) ) )
	{
		bug( "%s: make_mobile failed to initialize first mob.", __FUNCTION__ );
		return;
	}
	mob = create_mobile( pMobIndex );
	char_to_room( mob, room );

	/*
	 * Initialize last mob in range
	 */
	if( !( pMobIndex = make_mobile( hi, 0, "last mob" ) ) )
	{
		bug( "%s: make_mobile failed to initialize last mob.", __FUNCTION__ );
		return;
	}
	mob = create_mobile( pMobIndex );
	char_to_room( mob, room );

	/*
	 * Initialize first obj in range
	 */
	if( !( pObjIndex = make_object( lo, 0, "first obj" ) ) )
	{
		bug( "%s: make_object failed to initialize first obj.", __FUNCTION__ );
		return;
	}
	obj = create_object( pObjIndex, 0 );
	obj_to_room( obj, room );

	/*
	 * Initialize last obj in range
	 */
	if( !( pObjIndex = make_object( hi, 0, "last obj" ) ) )
	{
		bug( "%s: make_object failed to initialize last obj.", __FUNCTION__ );
		return;
	}
	obj = create_object( pObjIndex, 0 );
	obj_to_room( obj, room );

	/*
	 * Save character and newly created zone
	 */
	save_char_obj( victim );

	if( !IS_SET( tarea->status, AREA_DELETED ) )
	{
		SET_BIT( tarea->flags, AFLAG_PROTOTYPE );
		tarea->installed = false;
		SET_BIT( tarea->status, AREA_LOADED );
		snprintf( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
		fold_area( tarea, filename, false );
		process_sorting( tarea, true );
	}

	set_char_color( AT_IMMORT, ch );
	ch_printf( ch, "Vnum range set for %s and initialized.\r\n", victim->name );
}

CMDF( do_cmdtable )
{
	int hash, cnt;
	CMDTYPE *cmd;

	set_pager_color( AT_PLAIN, ch );
	send_to_pager( "Commands and Number of Uses This Run\r\n", ch );

	for( cnt = hash = 0; hash < 126; hash++ )
		for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
		{
			if( ( ++cnt ) % 4 )
				pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
			else
				pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->userec.num_uses );
		}
	return;
}

bool check_parse_name( const char *name );

/*
 * Load up a player file
 */
CMDF( do_loadup )
{
	char fname[1024];
	char name[256];
	struct stat fst;
	DESCRIPTOR_DATA *d;
	int old_room_vnum;
	char buf[MAX_STRING_LENGTH];

	one_argument( argument, name );
	if( name[0] == '\0' )
	{
		send_to_char( "Usage: loadup <playername>\r\n", ch );
		return;
	}

	name[0] = UPPER( name[0] );

	snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
	if( check_parse_name( name ) && lstat( fname, &fst ) != -1 )
	{
		CREATE( d, DESCRIPTOR_DATA, 1 );
		d->next = NULL;
		d->prev = NULL;
		d->connected = CON_GET_NAME;
		d->outsize = 2000;
		CREATE( d->outbuf, char, d->outsize );

		load_char_obj( d, name, false, false );
		add_char( d->character );
		old_room_vnum = d->character->in_room->vnum;
		char_to_room( d->character, ch->in_room );
		if( get_trust( d->character ) >= get_trust( ch ) )
		{
			do_say( d->character, "Do *NOT* disturb me again!" );
			send_to_char( "I think you'd better leave that player alone!\r\n", ch );
			d->character->desc = NULL;
			char_to_room( d->character, get_room_index( old_room_vnum ) );
			do_quit( d->character, "" );
			return;
		}
		d->character->desc = NULL;
		d->character->retran = old_room_vnum;
		d->character = NULL;
		DISPOSE( d->outbuf );
		DISPOSE( d );
		ch_printf( ch, "Player %s loaded from room %d.\r\n", capitalize( name ), old_room_vnum );
		snprintf( buf, MAX_STRING_LENGTH, "%s appears from nowhere, eyes glazed over.\r\n", capitalize( name ) );
		act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
		send_to_char( "Done.\r\n", ch );
		return;
	}
	/*
	 * else no player file
	 */
	send_to_char( "No such player.\r\n", ch );
	return;
}

CMDF( do_fixchar )
{
	char name[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, name );
	if( name[0] == '\0' )
	{
		send_to_char( "Usage: fixchar <playername>\r\n", ch );
		return;
	}
	victim = get_char_room( ch, name );
	if( !victim )
	{
		send_to_char( "They're not here.\r\n", ch );
		return;
	}
	fix_char( victim );
	/*  victim->armor	= 100;
		victim->mod_str	= 0;
		victim->mod_dex	= 0;
		victim->mod_wis	= 0;
		victim->mod_int	= 0;
		victim->mod_con	= 0;
		victim->mod_cha	= 0;
		victim->mod_lck	= 0;
		victim->damroll	= 0;
		victim->hitroll	= 0;
		victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
		victim->saving_spell_staff = 0; */
	send_to_char( "Done.\r\n", ch );
}

CMDF( do_newbieset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: newbieset <char>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( ( victim->top_level < 1 ) || ( victim->top_level > 5 ) )
	{
		send_to_char( "Level of victim must be 1 to 5.\r\n", ch );
		return;
	}
	obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 1 );
	obj_to_char( obj, victim );

	obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ), 1 );
	obj_to_char( obj, victim );

	/*
	 * Added by Brittany, on Nov. 24, 1996. The object is the adventurer's
	 * guide to the realms of despair, part of academy.are.
	 */
	{
		OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333 );
		if( obj_ind != NULL )
		{
			obj = create_object( obj_ind, 1 );
			obj_to_char( obj, victim );
		}
	}

	/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
	   called Spectral Gate.  Brittany */

	{

		OBJ_INDEX_DATA *obj_ind = get_obj_index( 123 );
		if( obj_ind != NULL )
		{
			obj = create_object( obj_ind, 1 );
			obj_to_char( obj, victim );
		}
	}

	act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT );
	ch_printf( ch, "You have re-equipped %s.\r\n", victim->name );
	return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names( const char *inp, char *out )
{
	char buf[MAX_INPUT_LENGTH], *pbuf = buf;
	int len;

	*out = '\0';
	while( inp && *inp )
	{
		inp = one_argument( inp, buf );
		if( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
		{
			if( *out )
				strcat( out, " " );
			strcat( out, buf );
		}
	}
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names( const char *inp, char *out )
{
	char buf[MAX_INPUT_LENGTH], *pbuf = buf;
	int len;

	*out = '\0';
	while( inp && *inp )
	{
		inp = one_argument( inp, buf );
		if( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
		{
			if( *out )
				strcat( out, " " );
			strcat( out, buf );
		}
	}
}

CMDF( do_bestowarea )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int arg_len;

	argument = one_argument( argument, arg );

	if( get_trust( ch ) < LEVEL_BUILDER )
	{
		send_to_char( "Sorry...\r\n", ch );
		return;
	}

	if( !*arg )
	{
		send_to_char( "Syntax:\r\n"
			"bestowarea <victim> <filename>.are\r\n"
			"bestowarea <victim> none             removes bestowed areas\r\n"
			"bestowarea <victim> list             lists bestowed areas\r\n"
			"bestowarea <victim>                  lists bestowed areas\r\n", ch );
		return;
	}

	if( !( victim = get_char_world( ch, arg ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't give special abilities to a mob!\r\n", ch );
		return;
	}

	if( get_trust( victim ) < LEVEL_STAFF )
	{
		send_to_char( "They aren't an immortal.\r\n", ch );
		return;
	}

	if( !victim->pcdata->bestowments )
		victim->pcdata->bestowments = str_dup( "" );

	if( !*argument || !str_cmp( argument, "list" ) )
	{
		extract_area_names( victim->pcdata->bestowments, buf );
		ch_printf( ch, "Bestowed areas: %s\r\n", buf );
		return;
	}

	if( !str_cmp( argument, "none" ) )
	{
		remove_area_names( victim->pcdata->bestowments, buf );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	arg_len = strlen( argument );
	if( arg_len < 5
		|| argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
		|| argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e' )
	{
		send_to_char( "You can only bestow an area name\r\n", ch );
		send_to_char( "E.G. bestow joe sam.are\r\n", ch );
		return;
	}

	snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, argument );
	DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup( buf );
	ch_printf( victim, "%s has bestowed on you the area: %s\r\n", ch->name, argument );
	send_to_char( "Done.\r\n", ch );
}

CMDF( do_bestow )
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg_buf[MAX_STRING_LENGTH];
	char tmparg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CMDTYPE *cmd;
	bool fComm = false;

	set_char_color( AT_IMMORT, ch );

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Bestow whom with what?\r\n", ch );
		return;
	}
	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't give special abilities to a mob!\r\n", ch );
		return;
	}
	if( victim == ch || get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You aren't powerful enough...\r\n", ch );
		return;
	}

	if( !victim->pcdata->bestowments )
		victim->pcdata->bestowments = str_dup( "" );

	if( argument[0] == '\0' || !str_cmp( argument, "show list" ) )
	{
		ch_printf( ch, "Current bestowed commands on %s: %s.\r\n", victim->name, victim->pcdata->bestowments );
		return;
	}

	if( !str_cmp( argument, "none" ) )
	{
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( "" );
		ch_printf( ch, "Bestowments removed from %s.\r\n", victim->name );
		ch_printf( victim, "%s has removed your bestowed commands.\r\n", ch->name );
		return;
	}

	arg_buf[0] = '\0';

	argument = one_argument( argument, arg );

	while( arg[0] != '\0' )
	{
		const char *cmd_buf;
		char cmd_tmp[MAX_INPUT_LENGTH];
		bool cFound = false;

		if( !( cmd = find_command( arg ) ) )
		{
			ch_printf( ch, "No such command as %s!\r\n", arg );
			argument = one_argument( argument, arg );
			continue;
		}
		else if( cmd->level > get_trust( ch ) )
		{
			ch_printf( ch, "You can't bestow the %s command!\r\n", arg );
			argument = one_argument( argument, arg );
			continue;
		}

		cmd_buf = victim->pcdata->bestowments;
		cmd_buf = one_argument( cmd_buf, cmd_tmp );
		while( cmd_tmp[0] != '\0' )
		{
			if( !str_cmp( cmd_tmp, arg ) )
			{
				cFound = true;
				break;
			}

			cmd_buf = one_argument( cmd_buf, cmd_tmp );
		}

		if( cFound == true )
		{
			argument = one_argument( argument, arg );
			continue;
		}

		snprintf( tmparg, MAX_INPUT_LENGTH, "%s ", arg );
		strncat( arg_buf, tmparg, MAX_STRING_LENGTH );
		argument = one_argument( argument, arg );
		fComm = true;
	}
	if( !fComm )
	{
		send_to_char( "Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\r\n", ch );
		return;
	}

	if( arg_buf[strlen( arg_buf ) - 1] == ' ' )
		arg_buf[strlen( arg_buf ) - 1] = '\0';

	snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg_buf );
	DISPOSE( victim->pcdata->bestowments );
	smash_tilde( buf );
	victim->pcdata->bestowments = str_dup( buf );
	set_char_color( AT_IMMORT, victim );
	ch_printf( victim, "%s has bestowed on you the command(s): %s\r\n", ch->name, arg_buf );
	send_to_char( "Done.\r\n", ch );
	return;
}

CMDF( do_pcrename )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char newname[MAX_STRING_LENGTH];
	char oldname[MAX_STRING_LENGTH];
	char backname[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg1 );
	one_argument( argument, arg2 );
	smash_tilde( arg2 );


	if( IS_NPC( ch ) )
		return;

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: rename <victim> <new name>\r\n", ch );
		return;
	}

	if( !check_parse_name( arg2 ) )
	{
		send_to_char( "Illegal name.\r\n", ch );
		return;
	}
	/*
	 * Just a security precaution so you don't rename someone you don't mean
	 * * too --Shaddai
	 */
	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That person is not in the room.\r\n", ch );
		return;
	}
	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't rename NPC's.\r\n", ch );
		return;
	}

	if( get_trust( ch ) < get_trust( victim ) )
	{
		send_to_char( "I don't think they would like that!\r\n", ch );
		return;
	}
	snprintf( newname, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( arg2[0] ), capitalize( arg2 ) );
	snprintf( oldname, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( victim->name[0] ), capitalize( victim->name ) );
	snprintf( backname, MAX_STRING_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( victim->name[0] ), capitalize( victim->name ) );
	if( access( newname, F_OK ) == 0 )
	{
		send_to_char( "That name already exists.\r\n", ch );
		return;
	}

	/*
	 * Have to remove the old god entry in the directories
	 */
	if( IS_IMMORTAL( victim ) )
	{
		char godname[MAX_STRING_LENGTH];
		snprintf( godname, MAX_STRING_LENGTH, "%s%s", GOD_DIR, capitalize( victim->name ) );
		remove( godname );
	}

	/*
	 * Remember to change the names of the areas
	 */
	if( victim->pcdata->area )
	{
		char filename[MAX_STRING_LENGTH];
		char newfilename[MAX_STRING_LENGTH];

		snprintf( filename, MAX_STRING_LENGTH, "%s%s.are", BUILD_DIR, victim->name );
		snprintf( newfilename, MAX_STRING_LENGTH, "%s%s.are", BUILD_DIR, capitalize( arg2 ) );
		rename( filename, newfilename );
		snprintf( filename, MAX_STRING_LENGTH, "%s%s.are.bak", BUILD_DIR, victim->name );
		snprintf( newfilename, MAX_STRING_LENGTH, "%s%s.are.bak", BUILD_DIR, capitalize( arg2 ) );
		rename( filename, newfilename );
	}

	STRFREE( victim->name );
	victim->name = STRALLOC( capitalize( arg2 ) );

	remove( backname );
	if( remove( oldname ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "Error: Couldn't delete file %s in do_rename.", oldname );
		send_to_char( "Couldn't delete the old file!\r\n", ch );
		log_string( oldname );
	}
	/*
	 * Time to save to force the affects to take place
	 */
	save_char_obj( victim );

	/*
	 * Now lets update the wizlist
	 */
	if( IS_IMMORTAL( victim ) )
		make_wizlist( );
	send_to_char( "Character was renamed.\r\n", ch );
	return;
}

struct tm *update_time( struct tm *old_time )
{
	time_t time;

	time = mktime( old_time );
	return localtime( &time );
}

//FILE *popen( const char *command, const char *type );
//int pclose( FILE *stream );
char *fgetf( char *s, int n, register FILE *iop );

CMDF( do_pipe )
{
	char buf[5000];
	FILE *fp;

	fp = popen( argument, "r" );

	fgetf( buf, 5000, fp );

	send_to_char( buf, ch );

	pclose( fp );

	return;
}

char *fgetf( char *s, int n, register FILE *iop )
{
	register int c;
	register char *cs;

	c = '\0';
	cs = s;
	while( --n > 0 && ( c = getc( iop ) ) != EOF )
		if( ( *cs++ = c ) == '\0' )
			break;
	*cs = '\0';
	return ( ( c == EOF && cs == s ) ? NULL : s );
}


CMDF( do_set_boot_time )
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	bool check;

	check = false;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\r\n", ch );
		send_to_char( "        setboot manual {0/1}\r\n", ch );
		send_to_char( "        setboot default\r\n", ch );
		ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\r\n", reboot_time, set_boot_time->manual );
		return;
	}

	if( !str_cmp( arg, "time" ) )
	{
		struct tm *now_time;

		argument = one_argument( argument, arg );
		argument = one_argument( argument, arg1 );
		if( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
		{
			send_to_char( "You must input a value for hour and minute.\r\n", ch );
			return;
		}
		now_time = localtime( &current_time );

		if( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
		{
			send_to_char( "Valid range for hour is 0 to 23.\r\n", ch );
			return;
		}

		if( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
		{
			send_to_char( "Valid range for minute is 0 to 59.\r\n", ch );
			return;
		}

		argument = one_argument( argument, arg );
		if( *arg != '\0' && is_number( arg ) )
		{
			if( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
			{
				send_to_char( "Valid range for day is 1 to 31.\r\n", ch );
				return;
			}
			argument = one_argument( argument, arg );
			if( *arg != '\0' && is_number( arg ) )
			{
				if( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
				{
					send_to_char( "Valid range for month is 1 to 12.\r\n", ch );
					return;
				}
				now_time->tm_mon--;
				argument = one_argument( argument, arg );
				if( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 || now_time->tm_year > 199 )
				{
					send_to_char( "Valid range for year is 1900 to 2099.\r\n", ch );
					return;
				}
			}
		}
		now_time->tm_sec = 0;
		if( mktime( now_time ) < current_time )
		{
			send_to_char( "You can't set a time previous to today!\r\n", ch );
			return;
		}
		if( set_boot_time->manual == 0 )
			set_boot_time->manual = 1;
		new_boot_time = update_time( now_time );
		new_boot_struct = *new_boot_time;
		new_boot_time = &new_boot_struct;
		reboot_check( mktime( new_boot_time ) );
		get_reboot_string( );

		ch_printf( ch, "Boot time set to %s\r\n", reboot_time );
		check = true;
	}
	else if( !str_cmp( arg, "manual" ) )
	{
		argument = one_argument( argument, arg1 );
		if( arg1[0] == '\0' )
		{
			send_to_char( "Please enter a value for manual boot on/off\r\n", ch );
			return;
		}

		if( !is_number( arg1 ) )
		{
			send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
			return;
		}

		if( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
		{
			send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
			return;
		}

		set_boot_time->manual = atoi( arg1 );
		ch_printf( ch, "Manual bit set to %s\r\n", arg1 );
		check = true;
		get_reboot_string( );
		return;
	}

	else if( !str_cmp( arg, "default" ) )
	{
		set_boot_time->manual = 0;
		/*
		 * Reinitialize new_boot_time
		 */
		new_boot_time = localtime( &current_time );
		new_boot_time->tm_mday += 1;
		if( new_boot_time->tm_hour > 12 )
			new_boot_time->tm_mday += 1;
		new_boot_time->tm_hour = 6;
		new_boot_time->tm_min = 0;
		new_boot_time->tm_sec = 0;
		new_boot_time = update_time( new_boot_time );

		sysdata.DENY_NEW_PLAYERS = false;

		send_to_char( "Reboot time set back to normal.\r\n", ch );
		check = true;
	}

	if( !check )
	{
		send_to_char( "Invalid argument for setboot.\r\n", ch );
		return;
	}

	else
	{
		get_reboot_string( );
		new_boot_time_t = mktime( new_boot_time );
	}
}

CMDF( do_form_password )
{
	char *pwcheck;

	set_char_color( AT_IMMORT, ch );

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: formpass <password>\r\n", ch );
		return;
	}

	/*
	 * This is arbitrary to discourage weak passwords
	 */
	if( strlen( argument ) < 5 )
	{
		send_to_char( "Usage: formpass <password>\r\n", ch );
		send_to_char( "New password must be at least 5 characters in length.\r\n", ch );
		return;
	}

	if( argument[0] == '!' )
	{
		send_to_char( "Usage: formpass <password>\r\n", ch );
		send_to_char( "New password cannot begin with the '!' character.\r\n", ch );
		return;
	}

	pwcheck = sha256_crypt( argument );
	ch_printf( ch, "%s results in the encrypted string: %s\r\n", argument, pwcheck );
	return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
CMDF( do_destro )
{
	set_char_color( AT_RED, ch );
	send_to_char( "If you want to destroy a character, spell it out!\r\n", ch );
	return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA *pArea )
{
	extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
	extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	CHAR_DATA *ech;
	CHAR_DATA *ech_next;
	OBJ_DATA *eobj;
	OBJ_DATA *eobj_next;
	int icnt;
	ROOM_INDEX_DATA *rid;
	ROOM_INDEX_DATA *rid_next;
	OBJ_INDEX_DATA *oid;
	OBJ_INDEX_DATA *oid_next;
	MOB_INDEX_DATA *mid;
	MOB_INDEX_DATA *mid_next;
	EXTRA_DESCR_DATA *eed;
	EXTRA_DESCR_DATA *eed_next;
	EXIT_DATA *exit;
	EXIT_DATA *exit_next;
	MPROG_ACT_LIST *mpact;
	MPROG_ACT_LIST *mpact_next;
	MPROG_DATA *mprog;
	MPROG_DATA *mprog_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for( ech = first_char; ech; ech = ech_next )
	{
		ech_next = ech->next;

		if( ech->fighting )
			stop_fighting( ech, true );
		if( IS_NPC( ech ) )
		{
			/*
			 * if mob is in area, or part of area.
			 */
			if( URANGE( pArea->low_vnum, ech->pIndexData->vnum, pArea->hi_vnum ) == ech->pIndexData->vnum || ( ech->in_room && ech->in_room->area == pArea ) )
				extract_char( ech, true );
			continue;
		}
		if( ech->in_room && ech->in_room->area == pArea )
			do_recall( ech, "" );
	}
	for( eobj = first_object; eobj; eobj = eobj_next )
	{
		eobj_next = eobj->next;
		/*
		 * if obj is in area, or part of area.
		 */
		if( URANGE( pArea->low_vnum, eobj->pIndexData->vnum, pArea->hi_vnum ) == eobj->pIndexData->vnum || ( eobj->in_room && eobj->in_room->area == pArea ) )
			extract_obj( eobj );
	}
	for( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
	{
		for( rid = room_index_hash[icnt]; rid; rid = rid_next )
		{
			rid_next = rid->next;

			for( exit = rid->first_exit; exit; exit = exit_next )
			{
				exit_next = exit->next;
				if( rid->area == pArea || exit->to_room->area == pArea )
				{
					STRFREE( exit->keyword );
					STRFREE( exit->description );
					UNLINK( exit, rid->first_exit, rid->last_exit, next, prev );
					DISPOSE( exit );
				}
			}
			if( rid->area != pArea )
				continue;
			STRFREE( rid->name );
			STRFREE( rid->description );
			if( rid->first_person )
			{
				bug( "close_area: room with people #%d", rid->vnum );
				for( ech = rid->first_person; ech; ech = ech_next )
				{
					ech_next = ech->next_in_room;
					if( ech->fighting )
						stop_fighting( ech, true );
					if( IS_NPC( ech ) )
						extract_char( ech, true );
					else
						do_recall( ech, "" );
				}
			}
			if( rid->first_content )
			{
				bug( "close_area: room with contents #%d", rid->vnum );
				for( eobj = rid->first_content; eobj; eobj = eobj_next )
				{
					eobj_next = eobj->next_content;
					extract_obj( eobj );
				}
			}
			for( eed = rid->first_extradesc; eed; eed = eed_next )
			{
				eed_next = eed->next;
				STRFREE( eed->keyword );
				STRFREE( eed->description );
				DISPOSE( eed );
			}
			for( mpact = rid->mpact; mpact; mpact = mpact_next )
			{
				mpact_next = mpact->next;
				DISPOSE( mpact->buf );
				DISPOSE( mpact );
			}
			for( mprog = rid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE( mprog->arglist );
				STRFREE( mprog->comlist );
				DISPOSE( mprog );
			}
			if( rid == room_index_hash[icnt] )
				room_index_hash[icnt] = rid->next;
			else
			{
				ROOM_INDEX_DATA *trid;

				for( trid = room_index_hash[icnt]; trid; trid = trid->next )
					if( trid->next == rid )
						break;
				if( !trid )
					bug( "Close_area: rid not in hash list %d", rid->vnum );
				else
					trid->next = rid->next;
			}
			DISPOSE( rid );
		}

		for( mid = mob_index_hash[icnt]; mid; mid = mid_next )
		{
			mid_next = mid->next;

			if( mid->vnum < pArea->low_vnum || mid->vnum > pArea->hi_vnum )
				continue;

			STRFREE( mid->player_name );
			STRFREE( mid->short_descr );
			STRFREE( mid->long_descr );
			STRFREE( mid->description );
			if( mid->pShop )
			{
				UNLINK( mid->pShop, first_shop, last_shop, next, prev );
				DISPOSE( mid->pShop );
			}
			if( mid->rShop )
			{
				UNLINK( mid->rShop, first_repair, last_repair, next, prev );
				DISPOSE( mid->rShop );
			}
			for( mprog = mid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE( mprog->arglist );
				STRFREE( mprog->comlist );
				DISPOSE( mprog );
			}
			if( mid == mob_index_hash[icnt] )
				mob_index_hash[icnt] = mid->next;
			else
			{
				MOB_INDEX_DATA *tmid;

				for( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
					if( tmid->next == mid )
						break;
				if( !tmid )
					bug( "Close_area: mid not in hash list %s", mid->vnum );
				else
					tmid->next = mid->next;
			}
			DISPOSE( mid );
		}

		for( oid = obj_index_hash[icnt]; oid; oid = oid_next )
		{
			oid_next = oid->next;

			if( oid->vnum < pArea->low_vnum || oid->vnum > pArea->hi_vnum )
				continue;

			STRFREE( oid->name );
			STRFREE( oid->short_descr );
			STRFREE( oid->description );
			STRFREE( oid->action_desc );

			for( eed = oid->first_extradesc; eed; eed = eed_next )
			{
				eed_next = eed->next;
				STRFREE( eed->keyword );
				STRFREE( eed->description );
				DISPOSE( eed );
			}
			for( paf = oid->first_affect; paf; paf = paf_next )
			{
				paf_next = paf->next;
				DISPOSE( paf );
			}
			for( mprog = oid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE( mprog->arglist );
				STRFREE( mprog->comlist );
				DISPOSE( mprog );
			}
			if( oid == obj_index_hash[icnt] )
				obj_index_hash[icnt] = oid->next;
			else
			{
				OBJ_INDEX_DATA *toid;

				for( toid = obj_index_hash[icnt]; toid; toid = toid->next )
					if( toid->next == oid )
						break;
				if( !toid )
					bug( "Close_area: oid not in hash list %s", oid->vnum );
				else
					toid->next = oid->next;
			}
			DISPOSE( oid );
		}
	}
	DISPOSE( pArea->name );
	DISPOSE( pArea->filename );
	STRFREE( pArea->author );

	if( IS_SET( pArea->flags, AFLAG_PROTOTYPE ) )
	{
		UNLINK( pArea, first_build, last_build, next, prev );
		UNLINK( pArea, first_bsort, last_bsort, next_sort, prev_sort );
	}
	else
	{
		UNLINK( pArea, first_area, last_area, next, prev );
		UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
	}

	DISPOSE( pArea );
}

CMDF( do_pldestroy )
{
	CHAR_DATA *victim;
	char *name;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	struct stat fst;

	one_argument( argument, arg );
	/*
		 if ( ch->skill_level[DIPLOMACY_ABILITY] != 311 )
		{
	//     send_to_char( "Haha, can't use it :P\r\n", ch );
		 snprintf( buf, MAX_STRING_LENGTH, "FUCKOVER - %s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
		 append_to_file( BUG_FILE, buf );
		 return;
		}
	*/


	if( arg[0] == '\0' )
	{
		send_to_char( "Destroy what player file?\r\n", ch );
		return;
	}

	/*
	 * Set the file points.
	 */
	name = capitalize( arg );
	snprintf( buf, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
	snprintf( buf2, MAX_STRING_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), name );

	/*
	 * This check makes sure the name is valid and that the file is there, else there
	 * is no need to go on. -Orion
	 */
	if( !check_parse_name( name ) || lstat( buf, &fst ) == -1 )
	{
		ch_printf( ch, "No player exists by the name %s.\r\n", name );
		return;
	}

	for( victim = first_char; victim; victim = victim->next )
	{
		if( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
			break;
	}

	if( !victim )
	{
		DESCRIPTOR_DATA *d;

		/*
		 * Make sure they aren't halfway logged in.
		 */
		for( d = first_descriptor; d; d = d->next )
			if( ( victim = d->character ) && !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
				break;
		if( d )
			close_socket( d, true );
	}
	else
	{
		int x, y;

		if( victim->pcdata->clan )
		{
			CLAN_DATA *clan = victim->pcdata->clan;

			if( !str_cmp( victim->name, clan->leader ) )
			{
				STRFREE( clan->leader );
				clan->leader = STRALLOC( "" );
			}
			if( !str_cmp( victim->name, clan->number1 ) )
			{
				STRFREE( clan->number1 );
				clan->number1 = STRALLOC( "" );
			}
			if( !str_cmp( victim->name, clan->number2 ) )
			{
				STRFREE( clan->number2 );
				clan->number2 = STRALLOC( "" );
			}

			--clan->members;
			save_clan( clan );

		}

		quitting_char = victim;
		save_char_obj( victim );
		saving_char = NULL;
		extract_char( victim, true );
		for( x = 0; x < MAX_WEAR; x++ )
		{
			for( y = 0; y < MAX_LAYERS; y++ )
				save_equipment[x][y] = NULL;
		}
	}

	if( !rename( buf, buf2 ) )
	{
		AREA_DATA *pArea;

		set_char_color( AT_RED, ch );
		send_to_char( "Player destroyed.  Pfile saved in backup directory.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, capitalize( arg ) );
		if( !remove( buf ) )
			send_to_char( "Player's immortal data destroyed.\r\n", ch );
		else if( errno != ENOENT )
		{
			ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\r\n", errno, strerror( errno ) );
			snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
			perror( buf2 );
		}

		/*
		 * Nuke lockers so cheaters don't remake and pilfer the goods -- Halcyon
		 */

		snprintf( buf, 256, "../storage/%s", capitalize( arg ) );

		if( !remove( buf ) )
			send_to_char( "Player's storage locker destroyed.\r\n", ch );
		else if( errno != ENOENT )
		{
			ch_printf( ch, "Unknown error #%d - %s (storage locker). Report to Halcyon.\r\n", errno, strerror( errno ) );
			snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
			perror( buf2 );
		}

		snprintf( buf2, MAX_STRING_LENGTH, "%s.are", capitalize( arg ) );
		for( pArea = first_build; pArea; pArea = pArea->next )
		{
			if( !strcmp( pArea->filename, buf2 ) )
			{
				snprintf( buf, MAX_STRING_LENGTH, "%s%s", BUILD_DIR, buf2 );
				if( IS_SET( pArea->status, AREA_LOADED ) )
				{
					fold_area( pArea, buf, false );
					close_area( pArea );
				}
				snprintf( buf2, MAX_STRING_LENGTH, "%s.bak", buf );
				set_char_color( AT_RED, ch );   /* Log message changes colors */
				if( !rename( buf, buf2 ) )
					send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
				else if( errno != ENOENT )
				{
					ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\r\n", errno, strerror( errno ) );
					snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
					perror( buf2 );
				}
			}
		}
	}
	else if( errno == ENOENT )
	{
		set_char_color( AT_PLAIN, ch );
		send_to_char( "Player does not exist.\r\n", ch );
	}
	else
	{
		set_char_color( AT_WHITE, ch );
		ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\r\n", errno, strerror( errno ) );
		snprintf( buf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, arg );
		perror( buf );
	}
	return;
}

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];  /* db.c */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example:

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand( CHAR_DATA *ch )
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];

	if( !IS_NPC( ch ) )
		return ch->name;

	one_argument( ch->name, name );  /* copy the first word into name */

	if( !name[0] )   /* weird mob .. no keywords */
	{
		mudstrlcpy( outbuf, "", MSL ); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	/*
	 * ->people changed to ->first_person -- TRI
	 */
	for( rch = ch->in_room->first_person; rch && ( rch != ch ); rch = rch->next_in_room )
		if( is_name( name, rch->name ) )
			count++;


	snprintf( outbuf, MAX_INPUT_LENGTH, "%d.%s", count, name );
	return outbuf;
}


CMDF( do_for )
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = false, fMortals = false, fMobs = false, fEverywhere = false, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_prev;   /* p_next to p_prev -- TRI */
	int i;

	argument = one_argument( argument, range );

	if( !range[0] || !argument[0] )  /* invalid usage? */
	{
		do_help( ch, "for" );
		return;
	}

	if( !str_prefix( "quit", argument ) )
	{
		send_to_char( "Are you trying to crash the MUD or something?\r\n", ch );
		return;
	}


	if( !str_cmp( range, "all" ) )
	{
		fMortals = true;
		fGods = true;
	}
	else if( !str_cmp( range, "gods" ) )
		fGods = true;
	else if( !str_cmp( range, "mortals" ) )
		fMortals = true;
	else if( !str_cmp( range, "mobs" ) )
		fMobs = true;
	else if( !str_cmp( range, "everywhere" ) )
		fEverywhere = true;
	else
		do_help( ch, "for" ); /* show syntax */

	 /*
	  * do not allow # to make it easier
	  */
	if( fEverywhere && strchr( argument, '#' ) )
	{
		send_to_char( "Cannot use FOR EVERYWHERE with the # thingie.\r\n", ch );
		return;
	}

	if( strchr( argument, '#' ) )    /* replace # ? */
	{
		/*
		 * char_list - last_char, p_next - gch_prev -- TRI
		 */
		for( p = last_char; p; p = p_prev )
		{
			p_prev = p->prev;  /* TRI */
			/*
			 * p_next = p->next;
			 *//*
			 * In case someone DOES try to AT MOBS SLAY #
			 */
			found = false;

			if( !( p->in_room ) || room_is_private( p, p->in_room ) || ( p == ch ) )
				continue;

			if( IS_NPC( p ) && fMobs )
				found = true;
			else if( !IS_NPC( p ) && get_trust( p ) >= LEVEL_STAFF && fGods )
				found = true;
			else if( !IS_NPC( p ) && get_trust( p ) < LEVEL_STAFF && fMortals )
				found = true;

			/*
			 * It looks ugly to me.. but it works :)
			 */
			if( found )    /* p is 'appropriate' */
			{
				const char *pSource = argument;   /* head of buffer to be parsed */
				char *pDest = buf;  /* parse into this */

				while( *pSource )
				{
					if( *pSource == '#' )    /* Replace # with name of target */
					{
						const char *namebuf = name_expand( p );

						if( namebuf ) /* in case there is no mob name ?? */
							while( *namebuf )  /* copy name over */
								*( pDest++ ) = *( namebuf++ );

						pSource++;
					}
					else
						*( pDest++ ) = *( pSource++ );
				}   /* while */
				*pDest = '\0';  /* Terminate */

				/*
				 * Execute
				 */
				old_room = ch->in_room;
				char_from_room( ch );
				char_to_room( ch, p->in_room );
				interpret( ch, buf );
				char_from_room( ch );
				char_to_room( ch, old_room );

			}  /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for( i = 0; i < MAX_KEY_HASH; i++ )   /* run through all the buckets */
			for( room = room_index_hash[i]; room; room = room->next )
			{
				found = false;

				/*
				 * Anyone in here at all?
				 */
				if( fEverywhere )   /* Everywhere executes always */
					found = true;
				else if( !room->first_person )  /* Skip it if room is empty */
					continue;
				/*
				 * ->people changed to first_person -- TRI
				 */

				 /*
				  * Check if there is anyone here of the requried type
				  */
				  /*
				   * Stop as soon as a match is found or there are no more ppl in room
				   */
				   /*
					* ->people to ->first_person -- TRI
					*/
				for( p = room->first_person; p && !found; p = p->next_in_room )
				{

					if( p == ch )    /* do not execute on oneself */
						continue;

					if( IS_NPC( p ) && fMobs )
						found = true;
					else if( !IS_NPC( p ) && ( get_trust( p ) >= LEVEL_STAFF ) && fGods )
						found = true;
					else if( !IS_NPC( p ) && ( get_trust( p ) <= LEVEL_STAFF ) && fMortals )
						found = true;
				}   /* for everyone inside the room */

				if( found && !room_is_private( p, room ) )  /* Any of the required type here AND room not private? */
				{
					/*
					 * This may be ineffective. Consider moving character out of old_room
					 * once at beginning of command then moving back at the end.
					 * This however, is more safe?
					 */

					old_room = ch->in_room;
					char_from_room( ch );
					char_to_room( ch, room );
					interpret( ch, argument );
					char_from_room( ch );
					char_to_room( ch, old_room );
				}   /* if found */
			}  /* for every room in a bucket */
	}    /* if strchr */
}   /* do_for */

void save_sysdata( SYSTEM_DATA sys );

CMDF( do_cset )
{
	char arg[MAX_STRING_LENGTH];
	short level;
	int value = -1;

	set_char_color( AT_IMMORT, ch );

	if( argument[0] == '\0' )
	{
		ch_printf( ch, "Mail:\r\n  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\r\n",
			sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
		ch_printf( ch, "  Take all mail: %d.\r\n", sysdata.take_others_mail );
		ch_printf( ch, "Channels:\r\n  Muse: %d. Think: %d. Log: %d. Build: %d.\r\n",
			sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
		ch_printf( ch, "Building:\r\n  Prototype modification: %d.  Player msetting: %d.\r\n",
			sysdata.level_modify_proto, sysdata.level_mset_player );
		ch_printf( ch, "Guilds:\r\n  Overseer: %s.  Advisor: %s.\r\n", sysdata.guild_overseer, sysdata.guild_advisor );
		ch_printf( ch, "Other:\r\n  Force on players: %d.  ", sysdata.level_forcepc );
		ch_printf( ch, "Private room override: %d.\r\n", sysdata.level_override_private );
		ch_printf( ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
		ch_printf( ch, "Penalty to stun plr vs. plr: %d.\r\n", sysdata.stun_plr_vs_plr );
		ch_printf( ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
		ch_printf( ch, "Percent damage plr vs. mob: %d.\r\n", sysdata.dam_plr_vs_mob );
		ch_printf( ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
		ch_printf( ch, "Percent damage mob vs. mob: %d.\r\n", sysdata.dam_mob_vs_mob );
		ch_printf( ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake );
		ch_printf( ch, "Autosave frequency (minutes): %d.\r\n", sysdata.save_frequency );
		ch_printf( ch, "  Save flags: %s\r\n", flag_string( sysdata.save_flags, save_flag ) );
		ch_printf( ch, "  Lottery Hour Countdown: %d    Winning Number: %d\r\n", sysdata.lotterytimer, sysdata.lotterynum );
		ch_printf( ch, "  Lottery Week: %d    Jackpot: %d\r\n", sysdata.lotteryweek, sysdata.jackpot );
		send_to_char( "\n\rTime Stuff:\r\n", ch );
		ch_printf( ch, "  Hours/day: %d  Days/week: %d  Days/month: %d   Months/year: %d  Days/year: %d\n",
			sysdata.hoursperday, sysdata.daysperweek, sysdata.dayspermonth, sysdata.monthsperyear,
			sysdata.daysperyear );
		ch_printf( ch, "  Sunrise: %d  DayBegin: %d   Noon: %d  Sunset: %d   NightBegin: %d   Midnight: %d\r\n",
			sysdata.hoursunrise, sysdata.hourdaybegin, sysdata.hournoon, sysdata.hoursunset, sysdata.hournightbegin,
			sysdata.hourmidnight );
		ch_printf( ch, "Storage Hour: %d.  Leaves Loaded: %d.", sysdata.storagetimer, sysdata.leafcount );
		ch_printf( ch, "\r\n&wPfile autocleanup status: &W%s  &wDays before purging newbies: &W%d\r\n",
			sysdata.CLEANPFILES ? "On" : "Off", sysdata.newbie_purge );
		ch_printf( ch, "&wDays before purging regular players: &W%d\r\n", sysdata.regular_purge );
		return;
	}

	argument = one_argument( argument, arg );

	if( !str_cmp( arg, "help" ) )
	{
		do_help( ch, "controls" );
		return;
	}

	if( !str_cmp( arg, "pfiles" ) )
	{

		sysdata.CLEANPFILES = !sysdata.CLEANPFILES;

		if( sysdata.CLEANPFILES )
			send_to_char( "Pfile autocleanup enabled.\r\n", ch );
		else
			send_to_char( "Pfile autocleanup disabled.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "save" ) )
	{
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "saveflag" ) )
	{
		int x = get_saveflag( argument );

		if( x == -1 )
			send_to_char( "Not a save flag.\r\n", ch );
		else
		{
			TOGGLE_BIT( sysdata.save_flags, 1 << x );
			send_to_char( "Ok.\r\n", ch );
		}
		return;
	}

	if( !str_prefix( arg, "guild_overseer" ) )
	{
		STRFREE( sysdata.guild_overseer );
		sysdata.guild_overseer = str_dup( argument );
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_prefix( arg, "guild_advisor" ) )
	{
		STRFREE( sysdata.guild_advisor );
		sysdata.guild_advisor = str_dup( argument );
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	level = ( short ) atoi( argument );

	if( !str_prefix( arg, "savefrequency" ) )
	{
		sysdata.save_frequency = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "stun" ) )
	{
		sysdata.stun_regular = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "stun_pvp" ) )
	{
		sysdata.stun_plr_vs_plr = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "dam_pvp" ) )
	{
		sysdata.dam_plr_vs_plr = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "get_notake" ) )
	{
		sysdata.level_getobjnotake = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "dam_pvm" ) )
	{
		sysdata.dam_plr_vs_mob = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "dam_mvp" ) )
	{
		sysdata.dam_mob_vs_plr = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "dam_mvm" ) )
	{
		sysdata.dam_mob_vs_mob = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	value = atoi( argument );

	if( !str_cmp( arg, "hoursperday" ) )
	{
		sysdata.hoursperday = value;
		ch_printf( ch, "Hours per day set to %d.\r\n", value );
		update_calendar( );
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "lotterytimer" ) )
	{
		sysdata.lotterytimer = value;
		ch_printf( ch, "Lottery Time set to %d.\r\n", value );
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "daysperweek" ) )
	{
		sysdata.daysperweek = value;
		ch_printf( ch, "Days per week set to %d.\r\n", value );
		update_calendar( );
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "dayspermonth" ) )
	{
		sysdata.dayspermonth = value;
		ch_printf( ch, "Days per month set to %d.\r\n", value );
		update_calendar( );
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "maxplayers" ) )
	{
		sysdata.alltimemax = value;
		ch_printf( ch, "All time max set to %d.\r\n", value );
		save_sysdata( sysdata );
		return;
	}

	if( !str_cmp( arg, "monthsperyear" ) )
	{
		sysdata.monthsperyear = value;
		ch_printf( ch, "Months per year set to %d.\r\n", value );
		update_calendar( );
		save_sysdata( sysdata );
		return;
	}

	if( level < 0 || level > MAX_LEVEL )
	{
		send_to_char( "Invalid value for new control.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "read_all" ) )
	{
		sysdata.read_all_mail = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "read_free" ) )
	{
		sysdata.read_mail_free = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "write_free" ) )
	{
		sysdata.write_mail_free = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "take_all" ) )
	{
		sysdata.take_others_mail = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "muse" ) )
	{
		sysdata.muse_level = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "think" ) )
	{
		sysdata.think_level = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "log" ) )
	{
		sysdata.log_level = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "build" ) )
	{
		sysdata.build_level = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "proto_modify" ) )
	{
		sysdata.level_modify_proto = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "override_private" ) )
	{
		sysdata.level_override_private = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "forcepc" ) )
	{
		sysdata.level_forcepc = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "mset_player" ) )
	{
		sysdata.level_mset_player = level;
		send_to_char( "Ok.\r\n", ch );
		return;
	}
	else
	{
		send_to_char( "Invalid argument.\r\n", ch );
		return;
	}
}

void update_calendar( void )
{
	sysdata.daysperyear = sysdata.dayspermonth * sysdata.monthsperyear;
	sysdata.hoursunrise = sysdata.hoursperday / 4;
	sysdata.hourdaybegin = sysdata.hoursunrise + 1;
	sysdata.hournoon = sysdata.hoursperday / 2;
	sysdata.hoursunset = ( ( sysdata.hoursperday / 4 ) * 3 );
	sysdata.hournightbegin = sysdata.hoursunset + 1;
	sysdata.hourmidnight = sysdata.hoursperday;
	calc_season( );
	return;
}

void get_reboot_string( void )
{
   snprintf( reboot_time, 50, "%s", asctime( new_boot_time ) );
}


CMDF( do_orange )
{
	send_to_char( "Function under construction.\r\n", ch );
	return;
}

CMDF( do_mrange )
{
	send_to_char( "Function under construction.\r\n", ch );
	return;
}

CMDF( do_hell )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	short time;
	bool h_d = false;
	struct tm *tms;

	argument = one_argument( argument, arg );
	if( !*arg )
	{
		send_to_char( "Hell who, and for how long?\r\n", ch );
		return;
	}
	if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	if( IS_IMMORTAL( victim ) )
	{
		send_to_char( "There is no point in helling an immortal.\r\n", ch );
		return;
	}
	if( victim->pcdata->release_date != 0 )
	{
		ch_printf( ch, "They are already in hell until %24.24s, by %s.\r\n",
			ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
		return;
	}
	argument = one_argument( argument, arg );
	if( !*arg || !is_number( arg ) )
	{
		send_to_char( "Hell them for how long?\r\n", ch );
		return;
	}
	time = atoi( arg );
	if( time <= 0 )
	{
		send_to_char( "You cannot hell for zero or negative time.\r\n", ch );
		return;
	}
	argument = one_argument( argument, arg );
	if( !*arg || !str_prefix( arg, "hours" ) )
		h_d = true;
	else if( str_prefix( arg, "days" ) )
	{
		send_to_char( "Is that value in hours or days?\r\n", ch );
		return;
	}
	else if( time > 30 )
	{
		send_to_char( "You may not hell a person for more than 30 days at a time.\r\n", ch );
		return;
	}
	tms = localtime( &current_time );
	if( h_d )
		tms->tm_hour += time;
	else
		tms->tm_mday += time;
	victim->pcdata->release_date = mktime( tms );
	victim->pcdata->helled_by = STRALLOC( ch->name );
	ch_printf( ch, "%s will be released from hell at %24.24s.\r\n", victim->name, ctime( &victim->pcdata->release_date ) );
	act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
	char_from_room( victim );
	char_to_room( victim, get_room_index( 6 ) );
	act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );
	do_look( victim, "auto" );
	ch_printf( victim, "The immortals are not pleased with your actions.\r\n"
		"You shall remain in hell for %d %s%s.\r\n", time, ( h_d ? "hour" : "day" ), ( time == 1 ? "" : "s" ) );
	save_char_obj( victim ); /* used to save ch, fixed by Thoric 09/17/96 */
	return;
}

CMDF( do_unhell )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;

	argument = one_argument( argument, arg );
	if( !*arg )
	{
		send_to_char( "Unhell whom..?\r\n", ch );
		return;
	}
	location = ch->in_room;
	ch->in_room = get_room_index( 6 );
	victim = get_char_room( ch, arg );
	ch->in_room = location;  /* The case of unhell self, etc. */
	if( !victim || IS_NPC( victim ) || victim->in_room->vnum != 6 )
	{
		send_to_char( "No one like that is in hell.\r\n", ch );
		return;
	}
	location = get_room_index( wherehome( victim ) );
	if( !location )
		location = ch->in_room;
	MOBtrigger = false;
	act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
	char_from_room( victim );
	char_to_room( victim, location );
	send_to_char( "The gods have smiled on you and released you from hell early!\r\n", victim );
	do_look( victim, "auto" );
	send_to_char( "They have been released.\r\n", ch );

	if( victim->pcdata->helled_by )
	{
		if( str_cmp( ch->name, victim->pcdata->helled_by ) )
			ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\r\n",
				victim->pcdata->helled_by );
		STRFREE( victim->pcdata->helled_by );
		victim->pcdata->helled_by = NULL;
	}

	MOBtrigger = false;
	act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
	victim->pcdata->release_date = 0;
	save_char_obj( victim );
	return;
}

/* Vnum search command by Swordbearer */
CMDF( do_vsearch )
{
	char arg[MAX_INPUT_LENGTH];
	bool found = false;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int obj_counter = 1;
	int argi;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax:  vsearch <vnum>.\r\n", ch );
		return;
	}

	set_pager_color( AT_PLAIN, ch );
	argi = atoi( arg );
	if( argi < 0 && argi > 32767 )
	{
		send_to_char( "Vnum out of range.\r\n", ch );
		return;
	}
	for( obj = first_object; obj != NULL; obj = obj->next )
	{
		if( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
			continue;

		found = true;
		for( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

		if( in_obj->carried_by != NULL )
			pager_printf( ch, "[%2d] Level %d %s carried by %s.\r\n",
				obj_counter, obj->level, obj_short( obj ), PERS( in_obj->carried_by, ch ) );
		else
			pager_printf( ch, "[%2d] [%-5d] %s in %s.\r\n", obj_counter,
				( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
				obj_short( obj ), ( in_obj->in_room == NULL ) ? "somewhere" : in_obj->in_room->name );

		obj_counter++;
	}

	if( !found )
		send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

	return;
}

/*
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96
 */
CMDF( do_sober )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on mobs.\r\n", ch );
		return;
	}

	if( victim->pcdata )
		victim->pcdata->condition[COND_DRUNK] = 0;
	send_to_char( "Ok.\r\n", ch );
	send_to_char( "You feel sober again.\r\n", victim );
	return;
}

/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE *social )
{
	DISPOSE( social->name );
	DISPOSE( social->char_no_arg );
	DISPOSE( social->others_no_arg );
	DISPOSE( social->char_found );
	DISPOSE( social->others_found );
	DISPOSE( social->vict_found );
	DISPOSE( social->char_auto );
	DISPOSE( social->others_auto );
	DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE *social )
{
	SOCIALTYPE *tmp, *tmp_next;
	int hash;

	if( !social )
	{
		bug( "Unlink_social: NULL social", 0 );
		return;
	}

	if( social->name[0] < 'a' || social->name[0] > 'z' )
		hash = 0;
	else
		hash = ( social->name[0] - 'a' ) + 1;

	if( social == ( tmp = social_index[hash] ) )
	{
		social_index[hash] = tmp->next;
		return;
	}
	for( ; tmp; tmp = tmp_next )
	{
		tmp_next = tmp->next;
		if( social == tmp_next )
		{
			tmp->next = tmp_next->next;
			return;
		}
	}
}
/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE *social )
{
	int hash, x;
	SOCIALTYPE *tmp, *prev;

	if( !social )
	{
		bug( "Add_social: NULL social", 0 );
		return;
	}

	if( !social->name )
	{
		bug( "Add_social: NULL social->name", 0 );
		return;
	}

	if( !social->char_no_arg )
	{
		bug( "Add_social: NULL social->char_no_arg", 0 );
		return;
	}

	/*
	 * make sure the name is all lowercase
	 */
	for( x = 0; social->name[x] != '\0'; x++ )
		( ( char * ) social->name )[x] = LOWER( social->name[x] );

	if( social->name[0] < 'a' || social->name[0] > 'z' )
		hash = 0;
	else
		hash = ( social->name[0] - 'a' ) + 1;

	if( ( prev = tmp = social_index[hash] ) == NULL )
	{
		social->next = social_index[hash];
		social_index[hash] = social;
		return;
	}

	for( ; tmp; tmp = tmp->next )
	{
		if( ( x = strcmp( social->name, tmp->name ) ) == 0 )
		{
			bug( "Add_social: trying to add duplicate name to bucket %d", hash );
			free_social( social );
			return;
		}
		else if( x < 0 )
		{
			if( tmp == social_index[hash] )
			{
				social->next = social_index[hash];
				social_index[hash] = social;
				return;
			}
			prev->next = social;
			social->next = tmp;
			return;
		}
		prev = tmp;
	}

	/*
	 * add to end
	 */
	prev->next = social;
	social->next = NULL;
	return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
CMDF( do_sedit )
{
	SOCIALTYPE *social;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char snoarg[MAX_INPUT_LENGTH + 5];

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_SOCIAL, ch );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: sedit <social> [field]\r\n", ch );
		send_to_char( "Syntax: sedit <social> create\r\n", ch );
		if( get_trust( ch ) > LEVEL_LIAISON )
			send_to_char( "Syntax: sedit <social> delete\r\n", ch );
		if( get_trust( ch ) > LEVEL_LIAISON )
			send_to_char( "Syntax: sedit <save>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "save" ) )
	{
		save_socials( );
		send_to_char( "Saved.\r\n", ch );
		return;
	}

	social = find_social( arg1 );

	if( !str_cmp( arg2, "create" ) )
	{
		if( social )
		{
			send_to_char( "That social already exists!\r\n", ch );
			return;
		}
		CREATE( social, SOCIALTYPE, 1 );
		social->name = str_dup( arg1 );
		snprintf(snoarg, MAX_INPUT_LENGTH + 5, "You %s.", arg1);
		social->char_no_arg = str_dup(snoarg);
		add_social(social);
		send_to_char("Social added.\r\n", ch);
		return;
	}

	if( !social )
	{
		send_to_char( "Social not found.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
	{
		ch_printf( ch, "Social: %s\r\n\n\rCNoArg: %s\r\n", social->name, social->char_no_arg );
		ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\r\n",
			social->others_no_arg ? social->others_no_arg : "(not set)",
			social->char_found ? social->char_found : "(not set)",
			social->others_found ? social->others_found : "(not set)" );
		ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\r\n",
			social->vict_found ? social->vict_found : "(not set)",
			social->char_auto ? social->char_auto : "(not set)",
			social->others_auto ? social->others_auto : "(not set)" );
		return;
	}

	if( get_trust( ch ) > LEVEL_LIAISON && !str_cmp( arg2, "delete" ) )
	{
		unlink_social( social );
		free_social( social );
		send_to_char( "Deleted.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "cnoarg" ) )
	{
		if( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
		{
			send_to_char( "You cannot clear this field.  It must have a message.\r\n", ch );
			return;
		}
		if( social->char_no_arg )
			DISPOSE( social->char_no_arg );
		social->char_no_arg = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "onoarg" ) )
	{
		if( social->others_no_arg )
			DISPOSE( social->others_no_arg );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->others_no_arg = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "cfound" ) )
	{
		if( social->char_found )
			DISPOSE( social->char_found );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->char_found = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "ofound" ) )
	{
		if( social->others_found )
			DISPOSE( social->others_found );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->others_found = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "vfound" ) )
	{
		if( social->vict_found )
			DISPOSE( social->vict_found );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->vict_found = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "cauto" ) )
	{
		if( social->char_auto )
			DISPOSE( social->char_auto );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->char_auto = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "oauto" ) )
	{
		if( social->others_auto )
			DISPOSE( social->others_auto );
		if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
			social->others_auto = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( get_trust( ch ) > LEVEL_LIAISON && !str_cmp( arg2, "name" ) )
	{
		bool relocate;
		SOCIALTYPE *checksocial;

		one_argument( argument, arg1 );
		if( arg1[0] == '\0' )
		{
			send_to_char( "Cannot clear name field!\r\n", ch );
			return;
		}
		if( ( checksocial = find_social( arg1 ) ) != NULL )
		{
			ch_printf( ch, "There is already a social named %s.\r\n", arg1 );
			return;
		}
		if( arg1[0] != social->name[0] )
		{
			unlink_social( social );
			relocate = true;
		}
		else
			relocate = false;
		if( social->name )
			DISPOSE( social->name );
		social->name = str_dup( arg1 );
		if( relocate )
			add_social( social );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	 * display usage message
	 */
	do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE *command )
{
	DISPOSE( command->name );
	DISPOSE( command->fun_name );
	DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE *command )
{
	CMDTYPE *tmp, *tmp_next;
	int hash;

	if( !command )
	{
		bug( "Unlink_command NULL command", 0 );
		return;
	}

	hash = command->name[0] % 126;

	if( command == ( tmp = command_hash[hash] ) )
	{
		command_hash[hash] = tmp->next;
		return;
	}
	for( ; tmp; tmp = tmp_next )
	{
		tmp_next = tmp->next;
		if( command == tmp_next )
		{
			tmp->next = tmp_next->next;
			return;
		}
	}
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE *command )
{
	int hash, x;
	CMDTYPE *tmp, *prev;

	if( !command )
	{
		bug( "Add_command: NULL command", 0 );
		return;
	}

	if( !command->name )
	{
		bug( "Add_command: NULL command->name", 0 );
		return;
	}

	if( !command->do_fun )
	{
		bug( "Add_command: NULL command->do_fun", 0 );
		return;
	}

	/*
	 * make sure the name is all lowercase
	 */
	for( x = 0; command->name[x] != '\0'; x++ )
		( ( char * ) command->name )[x] = LOWER( command->name[x] );

	hash = command->name[0] % 126;

	if( ( prev = tmp = command_hash[hash] ) == NULL )
	{
		command->next = command_hash[hash];
		command_hash[hash] = command;
		return;
	}

	/*
	 * add to the END of the list
	 */
	for( ; tmp; tmp = tmp->next )
		if( !tmp->next )
		{
			tmp->next = command;
			command->next = NULL;
		}
	return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
CMDF( do_cedit )
{
	CMDTYPE *command;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_IMMORT, ch );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: cedit save\r\n", ch );
		if( get_trust( ch ) > LEVEL_BUILDER )
		{
			send_to_char( "Syntax: cedit <command> create [code]\r\n", ch );
			send_to_char( "Syntax: cedit <command> delete\r\n", ch );
			send_to_char( "Syntax: cedit <command> show\r\n", ch );
			send_to_char( "Syntax: cedit <command> [field]\r\n", ch );
			send_to_char( "\n\rField being one of:\r\n", ch );
			send_to_char( "  level position log code\r\n", ch );
		}
		return;
	}

	if( get_trust( ch ) > LEVEL_LIAISON && !str_cmp( arg1, "save" ) )
	{
		save_commands( );
		send_to_char( "Saved.\r\n", ch );
		return;
	}

	command = find_command( arg1 );

	if( get_trust( ch ) > LEVEL_BUILDER && !str_cmp( arg2, "create" ) )
	{
		if( command )
		{
			send_to_char( "That command already exists!\r\n", ch );
			return;
		}
		CREATE( command, CMDTYPE, 1 );
		command->name = str_dup( arg1 );
		command->level = get_trust( ch );
		if( *argument )
			one_argument( argument, arg2 );
		else
			snprintf( arg2, MAX_INPUT_LENGTH+3, "do_%s", arg1 );
		command->do_fun = skill_function( arg2 );
		command->fun_name = str_dup( arg2 );
		add_command( command );
		send_to_char( "Command added.\r\n", ch );
		if( command->do_fun == skill_notfound )
			ch_printf( ch, "Code %s not found.  Set to no code.\r\n", arg2 );
		return;
	}

	if( !command )
	{
		send_to_char( "Command not found.\r\n", ch );
		return;
	}
	else if( command->level > get_trust( ch ) )
	{
		send_to_char( "You cannot touch this command.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
	{
		ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\r\n",
			command->name, command->level, command->position, command->log, command->fun_name );
		if( command->userec.num_uses )
			send_timer( &command->userec, ch );
		return;
	}

	if( get_trust( ch ) <= LEVEL_BUILDER )
	{
		do_cedit( ch, "" );
		return;
	}

	if( !str_cmp( arg2, "delete" ) )
	{
		unlink_command( command );
		free_command( command );
		send_to_char( "Deleted.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "code" ) )
	{
		DO_FUN *fun = skill_function( argument );

		if( fun == skill_notfound )
		{
			send_to_char( "Code not found.\r\n", ch );
			return;
		}
		command->do_fun = fun;
		DISPOSE( command->fun_name );
		command->fun_name = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "level" ) )
	{
		int level = atoi( argument );

		if( level < 0 || level > get_trust( ch ) )
		{
			send_to_char( "Level out of range.\r\n", ch );
			return;
		}
		command->level = level;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "hidden" ) )
	{
		int cshow = atoi( argument );

		if( ( cshow < 0 || cshow > 1 ) )
		{
			send_to_char( "hidden should be 0(shown) or 1(not shown).\r\n", ch );
			return;
		}
		command->cshow = cshow;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "log" ) )
	{
		int log = atoi( argument );

		if( log < 0 || log > LOG_COMM )
		{
			send_to_char( "Log out of range.\r\n", ch );
			return;
		}
		command->log = log;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "position" ) )
	{
		int position = atoi( argument );

		if( position < 0 || position > POS_DRAG )
		{
			send_to_char( "Position out of range.\r\n", ch );
			return;
		}
		command->position = position;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		bool relocate;
		CMDTYPE *checkcmd;

		one_argument( argument, arg1 );
		if( arg1[0] == '\0' )
		{
			send_to_char( "Cannot clear name field!\r\n", ch );
			return;
		}
		if( ( checkcmd = find_command( arg1 ) ) != NULL )
		{
			ch_printf( ch, "THere is already a command named %s.\r\n", arg1 );
			return;
		}
		if( arg1[0] != command->name[0] )
		{
			unlink_command( command );
			relocate = true;
		}
		else
			relocate = false;
		if( command->name )
			DISPOSE( command->name );
		command->name = str_dup( arg1 );
		if( relocate )
			add_command( command );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	 * display usage message
	 */
	do_cedit( ch, "" );
}

void save_reserved( void )
{
	RESERVE_DATA *res;
	FILE *fp;

	if( !( fp = FileOpen( SYSTEM_DIR RESERVED_LIST, "w" ) ) )
	{
		bug( "Save_reserved: cannot open " RESERVED_LIST, 0 );
		perror( RESERVED_LIST );
		return;
	}
	for( res = first_reserved; res; res = res->next )
		fprintf( fp, "%s~\n", res->name );
	fprintf( fp, "$~\n" );
	FileClose( fp );
	return;
}

CMDF( do_delete )
{

	char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg );

	if( IS_NPC( ch ) )
		return;

	if( IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		send_to_char( "Shouldn't delete over that!\r\n", ch );
		return;
	}

	if( ch->in_room == get_room_index( 6 ) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: Delete <Password>\r\n", ch );
		return;
	}

	if( strcmp( sha256_crypt( arg ), ch->pcdata->pwd ) )
	{
		send_to_char( "Wrong password.\r\n", ch );
		return;
	}
	else
	{
		do_pldestroy( ch, ch->name );
		snprintf( buf, MAX_STRING_LENGTH, "%s has deleted their character.", ch->name );
		log_string( buf );
	}
	return;
}

CMDF( do_saveall )
{
	CHAR_DATA *vch;
	CLAN_DATA *c;
	SHIP_DATA *ship;


	send_to_char( "&CSaving Characters and in-progress areas...&D\r\n", ch );
	for( vch = first_char; vch; vch = vch->next )
	{
		if( IS_NPC( vch ) )
			continue;
		interpret( vch, "save" );
		if( IS_IMMORTAL( vch ) && ( vch->pcdata->area ) )
			interpret( vch, "savearea" );
	}
	send_to_char( "&CSaving commands...&D\r\n", ch );
	save_commands( );
	send_to_char( "&CSaving system data...&D\r\n", ch );
	save_sysdata( sysdata );
	send_to_char( "&CSaving skills...&D\r\n", ch );
	save_skill_table( );
	send_to_char( "&CSaving herbs...&D\r\n", ch );
	save_herb_table( );
	send_to_char( "&CSaving socials...&D\r\n", ch );
	save_socials( );
	send_to_char( "&CSaving areas...&D\r\n", ch );
	hidefoldmessage = true;
	AREA_DATA *tarea;
	for( tarea = first_area; tarea; tarea = tarea->next )
		fold_area( tarea, tarea->filename, false );
	hidefoldmessage = false;
	send_to_char( "&CSaving Suits/Ships...&D\r\n", ch );
	for( ship = first_ship; ship; ship = ship->next )
		save_ship( ship );
	send_to_char( "&CSaving clans...&D\r\n", ch );
	for( c = first_clan; c; c = c->next )
		save_clan( c );
	send_to_char( "&CSaving boards...&D\r\n", ch );
	save_notes( );
	send_to_char( "&CSaving artifacts...&D\r\n", ch );
	save_artifacts( );
	send_to_char( "\r\n&CSaveall completed succesfully.&D\r\n", ch );

}

CMDF( do_makeimm )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int value;
	CHAR_DATA *victim;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Hahaha, you can't use it :P\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: makeimm <char> <level>.\r\n", ch );
		send_to_char( "Level must be 1001 - 1006.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	value = atoi( arg2 );

	if( value > 1006 || value < 1001 )
	{
		do_makeimm( ch, "" );
		return;
	}

	STRFREE( victim->pcdata->rank );
	victim->pcdata->rank = STRALLOC( "" );

	send_to_char( "Immortalizing a player...\r\n", ch );
	ch_printf( victim, "You are now Immortal level %d!\r\n", value );
	victim->top_level = value;
	return;
}

CMDF( do_checkskills )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int sn;
	int col;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "&zSyntax: checkskills <player>.\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "No one like that online.\r\n", ch );
		return;
	}

	col = 0;

	if( !IS_NPC( victim ) )
	{
		set_char_color( AT_MAGIC, ch );
		for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
		{
			if( skill_table[sn]->name == NULL )
				break;
			if( victim->pcdata->learned[sn] == 0 )
				continue;

			snprintf( buf, MAX_STRING_LENGTH, "%20s %3d%% ", skill_table[sn]->name, victim->pcdata->learned[sn] );
			send_to_char( buf, ch );

			if( ++col % 3 == 0 )
				send_to_char( "\r\n", ch );
		}
	}
	return;
}


CMDF( do_appset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int value;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	value = is_number( arg2 ) ? atoi( arg2 ) : -1;

	if( atoi( arg2 ) < -1 && value == -1 )
		value = atoi( arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "\r\n&BSyntax&c:&C Setapp &c<&Bappearance&c> &c<&Bvalue&c>\r\n", ch );
		send_to_char( "&BAppearance&c:&C hair, highlights, build, eye.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "hair" ) )
	{
		if( value < 0 )
			value = atoi( arg2 );
		if( !IS_NPC( ch ) && ( value < 0 || value >= 23 ) )
		{
			ch_printf( ch, "Hair number range is 0 to 23.\n" );
			ch_printf( ch, "Help hair for number ranges.\n" );
			return;
		}
		if( IS_NPC( ch ) )
		{
			ch_printf( ch, "Huh?\n" );
			return;
		}
		ch->pcdata->hair = value;
		send_to_char( "Done.", ch );
		return;
	}

	if( !str_cmp( arg1, "highlights" ) )
	{

		if( value < 0 )
			value = atoi( arg2 );
		if( !IS_NPC( ch ) && ( value < 0 || value >= 23 ) )
		{
			ch_printf( ch, "\n\rHighlight number range is 0 to 23.\n" );
			ch_printf( ch, "Help highlight for number ranges.\n" );
			return;
		}
		if( IS_NPC( ch ) )
		{
			ch_printf( ch, "Huh?\n" );
			return;
		}
		ch->pcdata->highlight = value;
		send_to_char( "Done.", ch );
		return;
	}

	if( !str_cmp( arg1, "eye" ) )
	{
		if( value < 0 )
			value = atoi( arg2 );
		if( !IS_NPC( ch ) && ( value < 0 || value >= 16 ) )
		{
			ch_printf( ch, "\n\rEye colour range is 0 to 15.\n" );
			ch_printf( ch, "Help eye for number ranges.\n" );
			return;
		}
		if( IS_NPC( ch ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		ch->pcdata->eye = value;
		send_to_char( "Done.", ch );
		return;
	}

	if( !str_cmp( arg1, "build" ) )
	{
		if( value < 0 )
			value = atoi( arg2 );
		if( !IS_NPC( ch ) && ( value < 0 || value >= 8 ) )
		{
			ch_printf( ch, "\n\rBuild size range is 0 to 8.\n" );
			ch_printf( ch, "Help bodysize for number ranges.\n" );
			return;
		}
		if( IS_NPC( ch ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		ch->pcdata->build = value;
		send_to_char( "Done.", ch );
		return;
	}
	do_appset( ch, "" );
	return;
}

CMDF( do_setlevels )
{
	int iAbility, sn;

	for( iAbility = 0; iAbility < MAX_ABILITY; iAbility++ )
	{
		ch->skill_level[iAbility] = 1000;
		ch->experience[iAbility] = exp_level( 1000 );
	}

	for( sn = 0; sn < top_sn; sn++ )
	{
		if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
			continue;
		if( skill_table[sn]->name && ( ch->skill_level[skill_table[sn]->guild] >= skill_table[sn]->min_level ) )
			ch->pcdata->learned[sn] = 100;
	}
	ch_printf( ch, "&[plain]All levels and skills have been set for you.&D\r\n" );
	return;
}
