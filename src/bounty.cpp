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
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"


BOUNTY_DATA *first_bounty;
BOUNTY_DATA *last_bounty;
BOUNTY_DATA *first_disintigration;
BOUNTY_DATA *last_disintigration;


void disintigration( CHAR_DATA *ch, CHAR_DATA *victim, long amount );
void nodisintigration( CHAR_DATA *ch, CHAR_DATA *victim, long amount );
int xp_compute( CHAR_DATA *ch, CHAR_DATA *victim );
void check_head( CHAR_DATA *ch, OBJ_DATA *obj );

void save_disintigrations( )
{
	BOUNTY_DATA *tbounty;
	FILE *fpout;
	char filename[256];

	sprintf( filename, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST );
	fpout = FileOpen( filename, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open disintigration.lst for writing!\r\n", 0 );
		return;
	}
	for( tbounty = first_disintigration; tbounty; tbounty = tbounty->next )
	{
		fprintf( fpout, "%s\n", tbounty->target );
		fprintf( fpout, "%ld\n", tbounty->amount );
		fprintf( fpout, "%s\n", tbounty->poster );
	}
	fprintf( fpout, "$\n" );
	FileClose( fpout );

}


bool is_disintigration( CHAR_DATA *victim )
{
	BOUNTY_DATA *bounty;

	for( bounty = first_disintigration; bounty; bounty = bounty->next )
		if( !str_cmp( victim->name, bounty->target ) )
			return true;
	return false;
}

BOUNTY_DATA *get_disintigration( const char *target )
{
	BOUNTY_DATA *bounty;

	for( bounty = first_disintigration; bounty; bounty = bounty->next )
		if( !str_cmp( target, bounty->target ) )
			return bounty;
	return NULL;
}

void load_bounties( )
{
	FILE *fpList;
	const char *target, *poster;
	char bountylist[256];
	BOUNTY_DATA *bounty;
	long int amount;

	first_disintigration = NULL;
	last_disintigration = NULL;

	log_string( "Loading disintigrations..." );

	sprintf( bountylist, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST );
	if( ( fpList = FileOpen( bountylist, "r" ) ) == NULL )
	{
		perror( bountylist );
		exit( 8 );
	}

	for( ;; )
	{
		target = feof( fpList ) ? "$" : fread_word( fpList );
		if( target[0] == '$' )
			break;
		CREATE( bounty, BOUNTY_DATA, 1 );
		LINK( bounty, first_disintigration, last_disintigration, next, prev );
		bounty->target = STRALLOC( target );
		amount = fread_number( fpList );
		bounty->amount = amount;
		poster = feof( fpList ) ? "$" : fread_word( fpList );

		if( poster[0] == '$' )
			break;

		bounty->poster = STRALLOC( poster );


	}
	FileClose( fpList );
	log_string( " Done bounties " );

	return;
}

CMDF( do_bounties )
{
	BOUNTY_DATA *bounty;
	int count = 0;

	set_char_color( AT_WHITE, ch );

	send_to_char( "\r\n&z|&WBounty&z|                 &z|&WAmount&z|            &z|&WPoster&z|\r\n", ch );
	for( bounty = first_disintigration; bounty; bounty = bounty->next )
	{
		set_char_color( AT_RED, ch );
		ch_printf( ch, "%-26s %-20ld %-20s\r\n", bounty->target, bounty->amount, bounty->poster );
		count++;
	}

	if( !count )
	{
		set_char_color( AT_GREY, ch );
		send_to_char( "There are no bounties set at this time.\r\n", ch );
		return;
	}
}

void disintigration( CHAR_DATA *ch, CHAR_DATA *victim, long amount )
{
	BOUNTY_DATA *bounty;
	bool found;
	char buf[MAX_STRING_LENGTH];

	found = false;

	for( bounty = first_disintigration; bounty; bounty = bounty->next )
	{
		if( !str_cmp( bounty->target, victim->name ) )
		{
			found = true;
			break;
		}
	}

	if( !found )
	{
		CREATE( bounty, BOUNTY_DATA, 1 );
		LINK( bounty, first_disintigration, last_disintigration, next, prev );

		bounty->target = STRALLOC( victim->name );
		bounty->amount = 0;
		bounty->poster = STRALLOC( ch->name );
	}

	bounty->amount = bounty->amount + amount;
	save_disintigrations( );

	sprintf( buf, "%s has added %ld dollars to the bounty on %s.", ch->name, amount, victim->name );
	echo_to_all( AT_RED, buf, 0 );

}

CMDF( do_addbounty )
{
	char arg[MAX_STRING_LENGTH];
	long int amount;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int count;
	int egain;

	count = 0;
	egain = 0;

	if( !str_cmp( argument, "gain" ) )
	{
		if( ch->in_room != get_room_index( 2065 ) )
		{
			send_to_char( "&RYou need to be at the bounty office!\r\n", ch );
			return;
		}

		/*		if( !has_rate( ch, RATE_LEVEL ) )
				{
					send_to_char( "You don't rate leveling! HELP RATE for more info.\r\n", ch );
					return;
				}*/

		for( obj = ch->first_carrying; obj; obj = obj->next_content )
		{
			if( obj->pIndexData->vnum == 2031 )    // Zoot
			{
				gain_exp( ch, 1000, HUNTING_ABILITY );
				separate_obj( obj );
				extract_obj( obj );
				egain += 1000;
				count++;
			}

			if( obj->pIndexData->vnum == 2032 )    // Adar
			{
				gain_exp( ch, 6000, HUNTING_ABILITY );
				separate_obj( obj );
				extract_obj( obj );
				egain += 6000;
				count++;
			}

			if( obj->pIndexData->vnum == 2033 )    // Kolak
			{
				gain_exp( ch, 10000, HUNTING_ABILITY );
				separate_obj( obj );
				extract_obj( obj );
				egain += 10000;
				count++;
			}

			if( obj->pIndexData->vnum == 2034 )    // Zeinel
			{
				gain_exp( ch, 20000, HUNTING_ABILITY );
				separate_obj( obj );
				extract_obj( obj );
				egain += 20000;
				count++;
			}

			if( obj->pIndexData->vnum == 2035 )    // SNIT
			{
				gain_exp( ch, 30000, HUNTING_ABILITY );
				separate_obj( obj );
				extract_obj( obj );
				egain += 30000;
				count++;
			}

		}
		if( count == 0 )
		{
			send_to_char( "You have nothing to turn in!\r\n", ch );
			return;
		}
		else
		{
			ch_printf( ch, "&YYou turn in &O%d &Yheads, for &O%s&Y exp.\r\n", count, num_punct( egain ) );
			return;
		}
		return;
	}

	if( !argument || argument[0] == '\0' )
	{
		do_bounties( ch, argument );
		return;
	}

	argument = one_argument( argument, arg );

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: Addbounty <target> <amount>\r\n", ch );
		return;
	}

	if( ch == victim )
	{
		send_to_char( "Bountying yourself? ahaha...\r\n", ch );
		return;
	}

	if( !xIS_SET( ch->act, PLR_PKER ) )
	{
		send_to_char( "&RIf you want to set Bounty's, go PK!!\r\n", ch );
		return;
	}

	/*    if ( !ch->in_room || ch->in_room->vnum != 6604 )
		{
			send_to_char( "You will have to go to the Hunters Guild on Tatooine to add a new bounty.", ch );
			return;
		}
	 */
	if( argument[0] == '\0' )
		amount = 0;
	else
		amount = atoi( argument );

	if( amount < 50000 )
	{
		send_to_char( "A bounty should be at least 50,000 dollars.\r\n", ch );
		return;
	}

	if( !( victim = get_char_world( ch, arg ) ) )
	{
		send_to_char( "They don't appear to be here .. wait til they log in.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can only set bounties on other players .. not mobs!\r\n", ch );
		return;
	}

	if( !xIS_SET( victim->act, PLR_PKER ) )
	{
		send_to_char( "&RIf you want to add bounty's, add them on PK players!\r\n", ch );
		return;
	}

	if( amount <= 0 )
	{
		send_to_char( "Nice try! How about 1 or more dollars instead...\r\n", ch );
		return;
	}

	if( ch->gold < amount )
	{
		send_to_char( "You don't have that many dollars!\r\n", ch );
		return;
	}

	ch->gold = ch->gold - amount;

	disintigration( ch, victim, amount );
}

void remove_disintigration( BOUNTY_DATA *bounty )
{
	UNLINK( bounty, first_disintigration, last_disintigration, next, prev );
	STRFREE( bounty->target );
	STRFREE( bounty->poster );
	DISPOSE( bounty );

	save_disintigrations( );
}

void claim_disintigration( CHAR_DATA *ch, CHAR_DATA *victim )
{
	BOUNTY_DATA *bounty;
	long int exp;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( victim ) )
		return;

	bounty = get_disintigration( victim->name );

	if( ch == victim )
	{
		if( bounty != NULL )
			remove_disintigration( bounty );
		return;
	}


	if( bounty == NULL )
	{
		if( victim == ch )
		{
			send_to_char( "Trying to remove your bounty by a bug CHEATER??", ch );
			return;
		}
		if( xIS_SET( victim->act, PLR_KILLER ) && !IS_NPC( ch ) )
		{
			if( ch->wfm_timer == 0 )
			{
				exp =
					URANGE( 1, xp_compute( ch, victim ),
						( exp_level( ch->skill_level[HUNTING_ABILITY] + 1 ) - exp_level( ch->skill_level[HUNTING_ABILITY] ) ) );
				gain_exp( ch, exp, HUNTING_ABILITY );
				set_char_color( AT_BLOOD, ch );
				ch_printf( ch, "You receive %ld hunting experience for executing a wanted killer.\r\n", exp );
				ch->wfm_timer = 60;
			}
			else
				ch_printf( ch, "&RYou cannot gain anymore bounty hunting experience for %d minutes.\r\n", ch->wfm_timer );
		}
		else if( !IS_NPC( ch ) )
		{
			xSET_BIT( ch->act, PLR_KILLER );
			ch_printf( ch, "You are now wanted for the murder of %s.\r\n", victim->name );
		}
		sprintf( buf, "%s is Dead!", victim->name );
		echo_to_all( AT_RED, buf, 0 );
		return;

	}

	ch->gold += bounty->amount;

	exp =
		URANGE( 1, bounty->amount + xp_compute( ch, victim ),
			( exp_level( ch->skill_level[HUNTING_ABILITY] + 1 ) - exp_level( ch->skill_level[HUNTING_ABILITY] ) ) );
	gain_exp( ch, exp, HUNTING_ABILITY );

	set_char_color( AT_BLOOD, ch );
	ch_printf( ch, "You receive %ld experience and %ld dollars,\r\n from the bounty on %s\r\n", exp, bounty->amount,
		bounty->target );

	sprintf( buf, "%s has claimed the bounty on %s!", ch->name, victim->name );
	echo_to_all( AT_RED, buf, 0 );
	sprintf( buf, "%s is Dead!", victim->name );
	echo_to_all( AT_RED, buf, 0 );

	if( !xIS_SET( victim->act, PLR_KILLER ) )
		xSET_BIT( ch->act, PLR_KILLER );
	remove_disintigration( bounty );
}

