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


/* new imm commands */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

void bombplanet( SHIP_DATA *ship, const char *arg );
bool check_parse_name( const char *name );
void clear_special( CHAR_DATA *ch );
const char *sweapon_bit_name( int vector );

const char *sweapon_bit_name( int vector )
{
	static char buf[512];

	buf[0] = '\0';
	if( vector & CYBA_REGULAR )
		strcat( buf, " Normal" );
	if( vector & CYBA_HASBUBBLE )
		strcat( buf, " Bubble" );
	if( vector & CYBA_HASSMOG )
		strcat( buf, " Smog" );
	if( vector & CYBA_HASTORNADO )
		strcat( buf, " Tornado" );
	if( vector & CYBA_HASLIFESAP )
		strcat( buf, " LifeSap" );
	return ( buf[0] != '\0' ) ? buf + 1 : "None Besides Normal";
}

/* random room generation procedure */
ROOM_INDEX_DATA *get_random_room( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA *room;

	for( ;; )
	{
		room = get_room_index( number_range( 200, 12000 ) );
		if( room != NULL )
			if( !room_is_private( ch, room )
				&& !xIS_SET( room->room_flags, ROOM_PRIVATE )
				&& !xIS_SET( room->room_flags, ROOM_SOLITARY )
				&& !xIS_SET( room->room_flags, ROOM_SAFE ) && ( IS_NPC( ch ) || xIS_SET( ch->act, ACT_AGGRESSIVE ) ) )
				break;
	}

	return room;
}

CMDF( do_fakequit )
{


	set_char_color( AT_WHITE, ch );
	send_to_char( "You 'quit the game,' you little sneak.\r\n", ch );
	act( AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM );
	set_char_color( AT_GREY, ch );

	if( !xIS_SET( ch->act, PLR_WIZINVIS ) )
		xSET_BIT( ch->act, PLR_WIZINVIS );
}

CMDF( do_lagout )
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: Lagout <victim> <amount>\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They're not here.\r\n", ch );
		return;
	}

	if( ( x = atoi( argument ) ) <= 0 )
	{
		send_to_char( "That makes a LOT of sense.\r\n", ch );
		return;
	}

	if( x > 1000 )
	{
		send_to_char( "There's a limit to cruel and unusual punishment.\r\n", ch );
		return;
	}

	//  send_to_char("Laaaaaaaaaaag!!\r\n", victim);
	WAIT_STATE( victim, x );
	send_to_char( "Adding lag now...\r\n", ch );
	return;
}

CMDF( do_vload )
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;
	bool found = false;
	const char *name;

	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: load voodoo <player>\r\n", ch );
		return;
	}

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_prefix( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( wch->top_level > ch->top_level )
				continue;

			found = true;

			if( ( pObjIndex = get_obj_index( OBJ_VNUM_VOODOO ) ) == NULL )
			{
				send_to_char( "Cannot find the voodoo doll vnum.\r\n", ch );
				return;
			}
			obj = create_object( pObjIndex, 0 );
			name = wch->name;
			snprintf( buf, MAX_STRING_LENGTH, obj->short_descr, name );
			obj->short_descr = STRALLOC( buf );
			snprintf( buf, MAX_STRING_LENGTH, obj->description, name );
			obj->description = STRALLOC( buf );
			snprintf( buf, MAX_STRING_LENGTH, obj->name, name );
			obj->name = STRALLOC( buf );
			if( CAN_WEAR( obj, ITEM_TAKE ) )
				obj_to_char( obj, ch );
			else
				obj_to_room( obj, ch->in_room );
			act( AT_MAGIC, "$n has created $p!", ch, obj, NULL, TO_ROOM );
			send_to_char( "Ok.\r\n", ch );
			return;
		}
	}
	send_to_char( "No one of that name is playing.\r\n", ch );
	return;
}

/*
void do_charge( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	int chance;
	SHIP_DATA *ship;
	int chargetype;

	argument = one_argument( argument, arg1 );

		if ( arg1[0] == '\0' )
		{
			send_to_char( "Charge what?\r\n", ch );
			send_to_char( "First  Second   Third?\r\n", ch );
			return;
		}

	if ( !str_cmp( arg1, "first" ) )
	{
	   send_to_char( "Charging!!\r\n", ch );
	   chargetype = 1;
	}
	if ( !str_cmp( arg1, "second" ) )
	{
	   send_to_char( "Charging!!\r\n", ch );
	   chargetype = 2;
	}
	if ( !str_cmp( arg1, "third" ) )
	{
	   send_to_char( "Charging!!\r\n", ch );
	   chargetype = 3;
	}

	strcpy( arg, argument );

	switch( ch->substate )
	{
		default:
				if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
				{
					send_to_char("&RYou must be in the cockpit of a ship to do that!\r\n",ch);
					return;
				}
				if (ship->shipstate == SHIP_HYPERSPACE)
				{
				  send_to_char("&RYou can only do that in realspace!\r\n",ch);
				  return;
				}
			  if (ship->target0 == NULL )
			 {
				send_to_char("&RYou need to choose a target first.\r\n",ch);
				return;
			 }
			   if (ship->shipstate == SHIP_DISABLED)
				{
					send_to_char("&RThe ships drive is disabled. Unable to manuever.\r\n",ch);
					return;
				}
				if (ship->shipstate == SHIP_DOCKED)
				{
					send_to_char("&RYou can't do that until after you've launched!\r\n",ch);
					return;
				}
				if (ship->shipstate != SHIP_READY)
				{
					send_to_char("&RPlease wait until the ship has finished its current manouver.\r\n",ch);
					return;
				}

				if ( ship->energy <1 )
				{
				   send_to_char("&RTheres not enough fuel!\r\n",ch);
				   return;
				}

				if ( ship->ship_class == MOBILE_SUIT )
					chance = IS_NPC(ch) ? ch->top_level
					 : (int)  (ch->pcdata->learned[gsn_mobilesuits]) ;
				if ( ship->ship_class == TRANSPORT_SHIP )
					chance = IS_NPC(ch) ? ch->top_level
					 : (int)  (ch->pcdata->learned[gsn_midships]) ;
				if ( ship->ship_class == CAPITAL_SHIP )
					chance = IS_NPC(ch) ? ch->top_level
					 : (int) (ch->pcdata->learned[gsn_capitalships]);
				if ( number_percent( ) < chance )
			{
			   send_to_char( "&G\r\n", ch);
			   act( AT_PLAIN, "$n does  ...", ch,
				NULL, argument , TO_ROOM );
		   echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");
			   add_timer ( ch , TIMER_DO_FUN , 1 , do_charge , 1 );
			   ch->dest_buf = str_dup(arg);
			   return;
			}
			send_to_char("&RYou fail to work the controls properly.\r\n",ch);
*/
/*	        if ( ship->ship_class == MOBILE_SUIT )
					learn_from_failure( ch, gsn_mobilesuits );
				if ( ship->ship_class == TRANSPORT_SHIP )
					learn_from_failure( ch, gsn_midships );
				if ( ship->ship_class == CAPITAL_SHIP )
					learn_from_failure( ch, gsn_capitalships ); */
					/*    	   	return;

							case 1:
								if ( !ch->dest_buf )
								   return;
								strcpy(arg, ch->dest_buf);
								DISPOSE( ch->dest_buf);
								break;

							case SUB_TIMER_DO_ABORT:
								DISPOSE( ch->dest_buf );
								ch->substate = SUB_NONE;
								if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
									  return;
									send_to_char("&Raborted.\r\n", ch);
									echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "blah3");
								if (ship->shipstate != SHIP_DISABLED)
								   ship->shipstate = SHIP_READY;
								return;
						}

						ch->substate = SUB_NONE;

						if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
						{
						   return;
						}

						send_to_char( "&Gblah\r\n", ch);
						act( AT_PLAIN, "$n does  ...", ch,
							 NULL, argument , TO_ROOM );
						echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "blah2");
						if ( chargetype == 1 )
						 {
						   do_firstweapon(ch, "" );
						   return;
						 }
					  else if ( chargetype == 2 )
						 {
						   do_secondweapon(ch, "" );
						   return;
						 }
					  else
						 {
						   do_thirdweapon(ch, "" );
						   return;
						 }
					*/
					/*
						if ( ship->ship_class == MOBILE_SUIT )
							learn_from_success( ch, gsn_space );
						if ( ship->ship_class == TRANSPORT_SHIP )
							learn_from_success( ch, gsn_midships );
						if ( ship->ship_class == CAPITAL_SHIP )
							learn_from_success( ch, gsn_capitalships );
					*/
					// return;
					// }



CMDF( do_ignore )
{
	char arg[MAX_INPUT_LENGTH];
	IGNORE_DATA *temp, *next;
	char fname[1024];
	struct stat fst;
	CHAR_DATA *victim;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg );

	snprintf( fname, sizeof(fname), "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );

	victim = NULL;

	/*
	 * If no arguements, then list players currently ignored
	 */
	if( arg[0] == '\0' )
	{
		set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "\r\n----------------------------------------\r\n" );
		set_char_color( AT_DGREEN, ch );
		ch_printf( ch, "You are currently ignoring:\r\n" );
		set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "----------------------------------------\r\n" );
		set_char_color( AT_PLAIN, ch );

		if( !ch->pcdata->first_ignored )
		{
			ch_printf( ch, "\t    no one\r\n" );
			return;
		}

		for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
		{
			ch_printf( ch, "\t  - %s\r\n", temp->name );
		}

		return;
	}
	/*
	 * Clear players ignored if given arg "none"
	 */
	else if( !strcmp( arg, "none" ) )
	{
		for( temp = ch->pcdata->first_ignored; temp; temp = next )
		{
			next = temp->next;
			UNLINK( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
			STRFREE( temp->name );
			DISPOSE( temp );
		}

		set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "You now ignore no one.\r\n" );

		return;
	}
	/*
	 * Prevent someone from ignoring themself...
	 */
	else if( !strcmp( arg, "self" ) || nifty_is_name( arg, ch->name ) )
	{
		set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "Did you type something?\r\n" );
		return;
	}
	else
	{
		int i;

		/*
		 * get the name of the char who last sent tell to ch
		 */
		if( !strcmp( arg, "reply" ) )
		{
			if( !ch->reply )
			{
				set_char_color( AT_PLAIN, ch );
				ch_printf( ch, "They're not here.\r\n" );
				return;
			}
			else
			{
				strcpy( arg, ch->reply->name );
			}
		}

		/*
		 * Loop through the linked list of ignored players
		 */
		 /*
		  * keep track of how many are being ignored
		  */
		for( temp = ch->pcdata->first_ignored, i = 0; temp; temp = temp->next, i++ )
		{
			/*
			 * If the argument matches a name in list remove it
			 */
			if( !strcmp( temp->name, capitalize( arg ) ) )
			{
				UNLINK( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
				set_char_color( AT_PLAIN, ch );
				ch_printf( ch, "You no longer ignore %s.\r\n", temp->name );
				STRFREE( temp->name );
				DISPOSE( temp );
				return;
			}
		}

		/*
		 * if there wasn't a match check to see if the name
		 */
		 /*
		  * is valid. This if-statement may seem like overkill
		  */
		  /*
		   * but it is intended to prevent people from doing the
		   */
		   /*
			* spam and log thing while still allowing ya to
			*/
			/*
			 * ignore new chars without pfiles yet...
			 */
		if( stat( fname, &fst ) == -1 &&
			( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) || strcmp( capitalize( arg ), victim->name ) != 0 ) )
		{
			set_char_color( AT_PLAIN, ch );
			ch_printf( ch, "No player exists by that name.\r\n" );
			return;
		}

		if( victim )
		{
			strcpy( capitalize( arg ), victim->name );
		}

		if( !check_parse_name( capitalize( arg ) ) )
		{
			send_to_char( "No player exists by that name.\r\n", ch );
			return;
		}

		/*
		 * If its valid and the list size limit has not been reached create a node and at it to the list
		 */
		if( i < sysdata.maxign )
		{
			IGNORE_DATA *newIg;
			CREATE( newIg, IGNORE_DATA, 1 );
			newIg->name = STRALLOC( capitalize( arg ) );
			newIg->next = NULL;
			newIg->prev = NULL;
			LINK( newIg, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
			set_char_color( AT_PLAIN, ch );
			ch_printf( ch, "You now ignore %s.\r\n", newIg->name );
			return;
		}
		else
		{
			set_char_color( AT_PLAIN, ch );
			ch_printf( ch, "You may only ignore %d players.\r\n", sysdata.maxign );
			return;
		}
	}
}

/*
 * This function simply checks to see if ch is ignoring ign_ch.
 * Last Modified: October 10, 1997
 * - Fireblade
 */
bool is_ignoring( CHAR_DATA *ch, CHAR_DATA *ign_ch )
{
	IGNORE_DATA *temp;

	if( !ch )    /* Paranoid bug check, you never know. */
	{
		bug( "is_ignoring: NULL CH!", 0 );
		return false;
	}

	if( !ign_ch )    /* Bail out, webwho can access this and ign_ch will be NULL */
		return false;

	if( IS_NPC( ch ) || IS_NPC( ign_ch ) )
		return false;

	for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
	{
		if( nifty_is_name( temp->name, ign_ch->name ) )
			return true;
	}
	return false;
}

CMDF( do_dnd )
{
	if( !IS_NPC( ch ) && ch->pcdata )
		if( IS_SET( ch->pcdata->flags, PCFLAG_DND ) )
		{
			REMOVE_BIT( ch->pcdata->flags, PCFLAG_DND );
			send_to_char( "Your 'do not disturb' flag is now off.\r\n", ch );
		}
		else
		{
			SET_BIT( ch->pcdata->flags, PCFLAG_DND );
			send_to_char( "Your 'do not disturb' flag is now on.\r\n", ch );
		}
	else
		send_to_char( "huh?\r\n", ch );
}

/*
 *  "Clones" immortal command
 *  Author: Cronel (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */


CMDF( do_multicheck )
{
	DESCRIPTOR_DATA *dsrc, *ddst, *dsrc_next, *ddst_next;
	DESCRIPTOR_DATA *dlistf, *dlistl;
	short clone_count;

	set_pager_color( AT_PLAIN, ch );
	pager_printf( ch, "\r\n %-12.12s | %-12.12s | %-s\r\n", "characters", "user", "host" );
	pager_printf( ch, "--------------+--------------+---------------------------------------------\r\n" );

	dlistf = dlistl = NULL;

	for( dsrc = first_descriptor; dsrc; dsrc = dsrc_next )
	{
		if( ( dsrc->character && !can_see( ch, dsrc->character ) ) || !dsrc->user || !dsrc->host )
		{
			dsrc_next = dsrc->next;
			continue;
		}

		pager_printf( ch, " %-12.12s |",
			dsrc->original ? dsrc->original->name : ( dsrc->character ? dsrc->character->name : "(No name)" ) );
		clone_count = 1;

		for( ddst = first_descriptor; ddst; ddst = ddst_next )
		{
			ddst_next = ddst->next;

			if( dsrc == ddst )
				continue;
			if( ( ddst->character && !can_see( ch, dsrc->character ) ) || !ddst->user || !ddst->host )
				continue;

			if( !str_cmp( dsrc->user, ddst->user ) && !str_cmp( dsrc->host, ddst->host ) )
			{
				UNLINK( ddst, first_descriptor, last_descriptor, next, prev );
				LINK( ddst, dlistf, dlistl, next, prev );
				pager_printf( ch, "              |\r\n %-12.12s |",
					ddst->original ? ddst->original->name :
					( ddst->character ? ddst->character->name : "(No name)" ) );
				clone_count++;
			}
		}

		pager_printf( ch, " %-12.12s | %s (%d clone%s)\r\n", dsrc->user, dsrc->host, clone_count, clone_count > 1 ? "s" : "" );

		dsrc_next = dsrc->next;

		UNLINK( dsrc, first_descriptor, last_descriptor, next, prev );
		LINK( dsrc, dlistf, dlistl, next, prev );
	}


	for( dsrc = dlistf; dsrc; dsrc = dsrc_next )
	{
		dsrc_next = dsrc->next;
		UNLINK( dsrc, dlistf, dlistl, next, prev );
		LINK( dsrc, first_descriptor, last_descriptor, next, prev );
	}
}


#define NUM_DAYS 31
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define MAP_HEIGHT 8
/* Should be the string length and number of the constants below.*/


const char *star_map[] = {
   "                                               C. C.                  g*",
   "    O:       R*        G*    G.  W* W. W.          C. C.    Y* Y. Y.    ",
   "  O*.                c.          W.W.     W.            C.       Y..Y.  ",
   "O.O. O.              c.  G..G.           W:      B*                   Y.",
   "     O.    c.     c.                     W. W.                  r*    Y.",
   "     O.c.     c.      G.             P..     W.        p.      Y.   Y:  ",
   "        c.                    G*    P.  P.           p.  p:     Y.   Y. ",
   "                 b*             P.: P*                 p.p:             "
};

/****************** CONSTELLATIONS and STARS *****************************
  Cygnus     Mars        Orion      Dragon       Cassiopeia          Venus
		   Ursa Ninor                           Mercurius     Pluto
			   Uranus              Leo                Crown       Raptor
*************************************************************************/
const char *sun_map[] = {
   "\\\\||//",
   "--++--",
   "//||\\\\"
};
const char *moon_map[] = {
   " ### ",
   "#####",
   " ### "
};

void look_sky( CHAR_DATA *ch )
{
	static char buf[MAX_STRING_LENGTH];
	static char buf2[4];
	int starpos, sunpos, moonpos, moonphase, i, linenum;

	send_to_char( "\n\rYou gaze up towards the heavens and see:\r\n", ch );

	sunpos = ( MAP_WIDTH * ( 24 - time_info.hour ) / 24 );
	moonpos = ( sunpos + time_info.day * MAP_WIDTH / NUM_DAYS ) % MAP_WIDTH;
	if( ( moonphase = ( ( ( ( MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) + ( MAP_WIDTH / 16 ) ) * 8 ) / MAP_WIDTH ) > 4 )
		moonphase -= 8;
	starpos = ( sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS ) % MAP_WIDTH;
	/*
	 * The left end of the star_map will be straight overhead at midnight during
	 * month 0
	 */

	for( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
	{
		if( ( time_info.hour >= 6 && time_info.hour <= 18 ) && ( linenum < 3 || linenum >= 6 ) )
			continue;
		snprintf( buf, MAX_STRING_LENGTH, " " );


		/*
		 * for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
		 */
		for( i = 1; i <= MAP_WIDTH; i++ )
		{
			/*
			 * plot moon on top of anything else...unless new moon & no eclipse
			 */
			if( ( time_info.hour >= 6 && time_info.hour <= 18 )    /* daytime? */
				&& ( moonpos >= MAP_WIDTH / 4 - 2 ) && ( moonpos <= 3 * MAP_WIDTH / 4 + 2 )    /* in sky? */
				&& ( i >= moonpos - 2 ) && ( i <= moonpos + 2 )    /* is this pixel near moon? */
				&& ( ( sunpos == moonpos && time_info.hour == 12 ) || moonphase != 0 ) /*no eclipse */
				&& ( moon_map[linenum - 3][i + 2 - moonpos] == '@' ) )
			{
				if( ( moonphase < 0 && i - 2 - moonpos >= moonphase ) || ( moonphase > 0 && i + 2 - moonpos <= moonphase ) )
					strcat( buf, "&W@" );
				else
					strcat( buf, " " );
			}
			else if( ( linenum >= 3 ) && ( linenum < 6 ) &&    /* nighttime */
				( moonpos >= MAP_WIDTH / 4 - 2 ) && ( moonpos <= 3 * MAP_WIDTH / 4 + 2 )  /* in sky? */
				&& ( i >= moonpos - 2 ) && ( i <= moonpos + 2 )   /* is this pixel near moon? */
				&& ( moon_map[linenum - 3][i + 2 - moonpos] == '@' ) )
			{
				if( ( moonphase < 0 && i - 2 - moonpos >= moonphase ) || ( moonphase > 0 && i + 2 - moonpos <= moonphase ) )
					strcat( buf, "&W@" );
				else
					strcat( buf, " " );
			}
			else   /* plot sun or stars */
			{
				if( time_info.hour >= 6 && time_info.hour <= 18 )   /* daytime */
				{
					if( i >= sunpos - 2 && i <= sunpos + 2 )
					{
						snprintf( buf2, MAX_STRING_LENGTH, "&Y%c", sun_map[linenum - 3][i + 2 - sunpos] );
						strcat( buf, buf2 );
					}
					else
						strcat( buf, " " );
				}
				else
				{
					switch( star_map[linenum][( MAP_WIDTH + i - starpos ) % MAP_WIDTH] )
					{
					default:
						strcat( buf, " " );
						break;
					case ':':
						strcat( buf, ":" );
						break;
					case '.':
						strcat( buf, "." );
						break;
					case '*':
						strcat( buf, "*" );
						break;
					case 'G':
						strcat( buf, "&G " );
						break;
					case 'g':
						strcat( buf, "&g " );
						break;
					case 'R':
						strcat( buf, "&R " );
						break;
					case 'r':
						strcat( buf, "&r " );
						break;
					case 'C':
						strcat( buf, "&C " );
						break;
					case 'O':
						strcat( buf, "&O " );
						break;
					case 'B':
						strcat( buf, "&B " );
						break;
					case 'P':
						strcat( buf, "&P " );
						break;
					case 'W':
						strcat( buf, "&W " );
						break;
					case 'b':
						strcat( buf, "&b " );
						break;
					case 'p':
						strcat( buf, "&p " );
						break;
					case 'Y':
						strcat( buf, "&Y " );
						break;
					case 'c':
						strcat( buf, "&c " );
						break;
					}
				}
			}
		}
		strcat( buf, "\r\n" );
		pager_printf( ch, buf );
	}
}

CMDF( do_wizslap )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *pRoomIndex;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "WizSlap whom?\r\n", ch );
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

	if( victim->top_level >= ch->top_level )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}

	for( ;; )
	{
		pRoomIndex = get_room_index( number_range( 200, 13000 ) );
		if( pRoomIndex )
			if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
				&& !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && !xIS_SET( pRoomIndex->room_flags, ROOM_SPACECRAFT )
				//  &&   !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT)
				&& !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
				&& !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && in_hard_range( ch, pRoomIndex->area ) )
				break;
	}

	act( AT_PLAIN, "$n slugs you hard, sending you flying!", ch, NULL, victim, TO_VICT );
	act( AT_PLAIN, "$n slugs $N hard, sending $M flying!", ch, NULL, victim, TO_NOTVICT );
	act( AT_PLAIN, "You slug $N , sending $M flying!", ch, NULL, victim, TO_CHAR );
	char_from_room( victim );
	char_to_room( victim, pRoomIndex );
	act( AT_PLAIN, "$n lands face first in the ground!", victim, NULL, NULL, TO_ROOM );
	/*
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("weaken");
		af.level     = 105;
		af.duration  = 5;
		af.location  = APPLY_STR;
		af.modifier  = -1 * (105 / 5);
		af.bitvector = AFF_WEAKEN;
		affect_to_char( victim, &af );
	*/
	//    send_to_char( "You feel your strength slip away.\r\n", victim );
	do_look( victim, "auto" );
	return;
}


#define ALL_SHIP -1
CMDF( do_freeships )
{
	//    char arg1[MAX_INPUT_LENGTH];
	//    char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	//    int count = 0;
	int ship_stype;


	if( argument[0] == '\0' )
	{
		send_to_char( "&zSyntax: allships <Leo/Aries/Taurus/Serpent/Clan/Midship/Gundam>.\r\n", ch );
		return;
	}
	else if( !str_cmp( argument, "leo" ) )
		ship_stype = 1;
	else if( !str_cmp( argument, "aries" ) )
		ship_stype = 2;
	else if( !str_cmp( argument, "taurus" ) )
		ship_stype = 3;
	else if( !str_cmp( argument, "serpent" ) )
		ship_stype = 4;
	else if( !str_cmp( argument, "clan" ) )
		ship_stype = 5;
	else if( !str_cmp( argument, "midship" ) )
		ship_stype = 6;
	else if( !str_cmp( argument, "gundam" ) )
		ship_stype = 7;
	else
	{
		send_to_char( "No such type.\r\n", ch );
		return;
	}

	send_to_char( "\r\n&WShip                     Owner/Owned           Where?\r\n", ch );

	for( ship = first_ship; ship; ship = ship->next )
	{

		if( ship_stype != ALL_SHIP && ship->stype != ship_stype )
			continue;

		ch_printf( ch, "&G%-26s ", ship->name );

		if( str_cmp( ship->owner, "" ) )
		{
			ch_printf( ch, "&O%-12s  ", ship->owner );
		}
		else
		{
			ch_printf( ch, "&Y%-12ld  ", get_ship_value( ship ) );
		}

		if( ship->in_room )
		{
			ch_printf( ch, "&B%s", ship->in_room->name );
		}
		else
		{
			ch_printf( ch, "&BIn Space" );
		}

		//       if (ship->stype == ship_stype )
		//       {
		ch_printf( ch, "\r\n" );
		continue;
		//       return;
		//       }
		send_to_char( "\r\n", ch );
		return;
	}
	return;
}

#undef ALL_SHIP

CMDF( do_upgradesuit )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ch->in_room != get_room_index( 4129 ) )
	{
		send_to_char( "&RThere isn't anyone here to upgrade your suit!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
	{
		send_to_char( "\n\rSyntax: upgradesuit <suit> <upgrade> <value>\r\n", ch );
		send_to_char( "Upgrade: firstweapon, secondweapon, thirdweapon, mod, alloy.\r\n", ch );
		return;
	}

	ship = get_ship( arg1 );
	if( !ship )
	{
		send_to_char( "No such ship.\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "&RThat ship has already started to launch", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RUpgrade your own suit!\r\n", ch );
		return;
	}

	if( !str_cmp( ship->owner, "Public" ) )
	{
		send_to_char( "&RUpgrade your own suit!\r\n", ch );
		return;
	}


	if( !str_cmp( arg2, "firstweapon" ) )
	{
		if( !is_number( argument ) )
		{
			send_to_char( "The value of the upgrade is a number!\r\n", ch );
			return;
		}
		if( atoi( argument ) < 0 || atoi( argument ) > 9 )
		{
			send_to_char( "&RNumber must be 0 - 9!\r\n", ch );
			return;
		}
		if( ch->gold < 100000 )
		{
			send_to_char( "&RYou need 100,000 dollars to upgrade your weapon!\r\n", ch );
			return;
		}
		ch->gold -= 100000;
		ship->firstweapon = URANGE( 0, atoi( argument ), 9 );
		send_to_char( "\r\n&YYou select your weapon and hand the worker 100,000 dollars.\r\n", ch );
		send_to_char
		( "&GA large crane moves over to your suit. A storm of sparks appear as your\nnew weapon is grafted onto your suit.\r\n",
			ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "secondweapon" ) )
	{
		if( !is_number( argument ) )
		{
			send_to_char( "The value of the upgrade is a number!\r\n", ch );
			return;
		}
		if( atoi( argument ) < 0 || atoi( argument ) > 9 )
		{
			send_to_char( "&RNumber must be 0 - 9!\r\n", ch );
			return;
		}
		if( ch->gold < 100000 )
		{
			send_to_char( "&RYou need 100,000 dollars to upgrade your weapon!\r\n", ch );
			return;
		}
		ch->gold -= 100000;
		ship->secondweapon = URANGE( 0, atoi( argument ), 9 );
		send_to_char( "\r\n&YYou select your weapon and hand the worker 100,000 dollars.\r\n", ch );
		send_to_char
		( "&GA large crane moves over to your suit. A storm of sparks appear as your\nnew weapon is grafted onto your suit.\r\n",
			ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "thirdweapon" ) )
	{
		if( !is_number( argument ) )
		{
			send_to_char( "The value of the upgrade is a number!\r\n", ch );
			return;
		}
		if( atoi( argument ) < 0 || atoi( argument ) > 9 )
		{
			send_to_char( "&RNumber must be 0 - 9!\r\n", ch );
			return;
		}
		if( ch->gold < 100000 )
		{
			send_to_char( "&RYou need 100,000 dollars to upgrade your weapon!\r\n", ch );
			return;
		}
		ch->gold -= 100000;
		ship->thirdweapon = URANGE( 0, atoi( argument ), 9 );
		send_to_char( "\r\n&YYou select your weapon and hand the worker 100,000 dollars.\r\n", ch );
		send_to_char
		( "&GA large crane moves over to your suit. A storm of sparks appear as your\nnew weapon is grafted onto your suit.\r\n",
			ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "mod" ) )
	{
		if( ship->ship_class != MOBILE_SUIT )
		{
			send_to_char( "&RThat isn't a Mobile Suit!\r\n", ch );
			return;
		}

		if( ship->offon == 1 )
		{
			send_to_char( "Take your suit out of transform mode first!\r\n", ch );
			return;
		}

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Syntax: setship <ship> mod <number>\r\n", ch );
			send_to_char( "Mods : 0 - None        1 - Zero System \r\n", ch );
			send_to_char( "       &R2 - Cloaking    &w3 - Transform\r\n", ch );
			send_to_char( "&RRed&w is inactive at the moment.\r\n", ch );
			return;
		}

		if( !is_number( argument ) )
		{
			send_to_char( "The value of the upgrade is a number!\r\n", ch );
			return;
		}

		if( atoi( argument ) < 0 || atoi( argument ) > 4 )
		{
			send_to_char( "&RNumber must be 0 - 4!\r\n", ch );
			return;
		}
		if( atoi( argument ) == 0 )
		{

			if( ship->mod == 3 )
			{
				ship->mod = 0;
				if( ship->offon == 1 )
				{
					ship->offon = 0;
					ship->realspeed = ship->realspeed / 2;
					ship->hyperspeed = ship->hyperspeed / 2;
				}
			}

			ship->mod = 0;
			send_to_char( "Done.\r\n", ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 1 )
		{
			if( ch->gold < 2000000 )
			{
				send_to_char( "&RYou need 2,000,000 dollars to purchase the Zero System!\r\n", ch );
				return;
			}

			if( ship->mod == 3 )
			{
				ship->mod = 0;
				if( ship->offon == 1 )
				{
					ship->offon = 0;
					ship->realspeed = ship->realspeed / 2;
					ship->hyperspeed = ship->hyperspeed / 2;
				}
			}

			ch->gold -= 2000000;
			ship->mod = 1;
			send_to_char( "&YYou give 2,000,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GA Dock worker jumps into the cockpit and goes to work installing a module, while another worker configures the suit core for the Zero System.\r\n",
				ch );
			save_ship( ship );
			return;
		}

		/*
		 * if (atoi(argument) == 2)
		 * {
		 * if ( ch->gold < 1500000 )
		 * {
		 * send_to_char( "&RYou need 1,500,000 dollars to purchase a Cloaking System!\r\n", ch );
		 * return;
		 * }
		 * ch->gold -= 1500000;
		 * ship->alloy = 2;
		 * send_to_char( "&YYou give 1,500,000 dollars to the worker.\r\n", ch );
		 * send_to_char( "&GA Dock worker quickly attaches a module to your suits core. He then jumps into the cock pit, flipping a switch, and your suit dissapears, then reappears.!\r\n", ch );
		 * save_ship( ship );
		 * return;
		 * }
		 */

		if( atoi( argument ) == 3 )
		{
			if( ch->gold < 1750000 )
			{
				send_to_char( "&RYou need 1,750,000 dollars to install a transform module!\r\n", ch );
				return;
			}

			if( ship->mod == 3 )
			{
				ship->mod = 0;
				if( ship->offon == 1 )
				{
					ship->offon = 0;
					ship->realspeed = ship->realspeed / 2;
					ship->hyperspeed = ship->hyperspeed / 2;
				}
			}

			ch->gold -= 1750000;
			ship->mod = 3;
			send_to_char( "&YYou give 1,750,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GYour suits arms and legs retract and fold in, wing pieces enveloping from behind it as it tests its new Transform Module!\r\n",
				ch );
			save_ship( ship );
			return;
		}

	}

	if( !str_cmp( arg2, "alloy" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Syntax: Upgrade <suit> alloy <number>\r\n", ch );
			send_to_char( "Alloy: 1 - Iron        2 - Steel\r\n", ch );
			send_to_char( "       3 - Titanium    4 - Neo-Titanium\r\n", ch );
			send_to_char( "       5 - Gundanium   6 - Z-Gundanium\r\n", ch );
			send_to_char( "Setting to 0 makes armor Basic.\r\n", ch );
			return;
		}

		if( !is_number( argument ) )
		{
			send_to_char( "The value of the upgrade is a number!\r\n", ch );
			return;
		}

		if( atoi( argument ) < 0 || atoi( argument ) > 6 )
		{
			send_to_char( "&RNumber must be 0 - 6!\r\n", ch );
			return;
		}

		if( atoi( argument ) == 0 )
		{
			ship->alloy = 0;
			send_to_char( "Done.\r\n", ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 1 )
		{
			if( ch->gold < 100000 )
			{
				send_to_char( "&RYou need 100,000 dollars to upgrade to Iron Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 100000;
			ship->alloy = 1;
			send_to_char( "&YYou give 100,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Iron Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 2 )
		{
			if( ch->gold < 250000 )
			{
				send_to_char( "&RYou need 250,000 dollars to upgrade to Steel Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 250000;
			ship->alloy = 2;
			send_to_char( "&YYou give 250,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Steel Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 3 )
		{
			if( ch->gold < 500000 )
			{
				send_to_char( "&RYou need 500,000 dollars to upgrade to Titanium Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 500000;
			ship->alloy = 3;
			send_to_char( "&YYou give 500,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Titanium Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 4 )
		{
			if( ch->gold < 900000 )
			{
				send_to_char( "&RYou need 900,000 dollars to upgrade to Neo-Titanium Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 900000;
			ship->alloy = 4;
			send_to_char( "&YYou give 900,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Neo-Titanium Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 5 )
		{
			if( ch->gold < 2000000 )
			{
				send_to_char( "&RYou need 2,000,000 dollars to upgrade to Gundanium Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 2000000;
			ship->alloy = 5;
			send_to_char( "&YYou give 500,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Gundanium Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

		if( atoi( argument ) == 6 )
		{
			if( ch->gold < 4500000 )
			{
				send_to_char( "&RYou need 4,500,000 dollars to upgrade to Z-Gundanium Alloy!\r\n", ch );
				return;
			}
			ch->gold -= 4500000;
			ship->alloy = 6;
			send_to_char( "&YYou give 500,000 dollars to the worker.\r\n", ch );
			send_to_char
			( "&GSeveral works quickly go to work, unbolting your old armor casing, and just as fast apply your new Z-Gundanium Alloy!\r\n",
				ch );
			save_ship( ship );
			return;
		}

	}
	return;
}

CMDF( do_award )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( arg1[0] == '\0' )
	{
		send_to_char( "\n\rSyntax: Award <dollars/bank/fp> <amount> <Person/all>\r\n", ch );
		return;
	}

	//    if (arg2[0] != '\0' )
	//        value = atoi(arg2);

	value = is_number( arg2 ) ? atoi( arg2 ) : -1;

	if( atoi( arg2 ) < -1 && value == -1 )
		value = atoi( arg2 );


	if( !str_prefix( arg1, "dollars" ) )
	{

		if( value < -3000000 || value > 3000000 )
		{
			send_to_char( "You can only award between 3000000 and 3000000 dollars.\r\n", ch );
			return;
		}

		if( !str_cmp( arg3, "all" ) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			if( !ch->pcdata )
				return;

			save_char_obj( ch );
			send_to_char( "Ok.\r\n", ch );
			for( vch = first_char; vch; vch = vch_next )
			{
				vch_next = vch->next;

				if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
				{
					if( value >= 0 )
					{
						vch->gold += value;
						ch_printf( vch, "&Y%s&O has awarded you &Y%d&O dollars!\r\n", ch->name, value );
					}
					else if( value <= 0 )
					{
						vch->gold += value;
						ch_printf( vch, "&Y%s&O has penalized you &Y%d&O dollars!\r\n", ch->name, value );
					}
				}
			}
		}
		else
		{

			CHAR_DATA *victim;

			if( ( victim = get_char_world( ch, arg3 ) ) == NULL )
			{
				send_to_char( "They aren't here.\r\n", ch );
				return;
			}

			send_to_char( "Ok.\r\n", ch );

			if( value >= 0 )
			{
				victim->gold += value;
				ch_printf( victim, "&Y%s&O has awarded you &Y%d&O dollars!\r\n", ch->name, value );
			}
			else if( value <= 0 )
			{
				victim->gold += value;
				ch_printf( victim, "&Y%s&O has penalized you &Y%d&O dollars!\r\n", ch->name, value );
			}
			return;
		}
		return;
	}

	if( !str_prefix( arg1, "fp" ) )
	{

		if( value < -999 || value > 999 )
		{
			send_to_char( "You can only award between -999 and 999 fp.\r\n", ch );
			return;
		}

		if( !str_cmp( arg3, "all" ) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			if( !ch->pcdata )
				return;

			save_char_obj( ch );
			send_to_char( "Ok.\r\n", ch );
			for( vch = first_char; vch; vch = vch_next )
			{
				vch_next = vch->next;

				if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
				{
					if( value >= 0 )
					{
						vch->questpoints += value;
						ch_printf( vch, "&Y%s&O has awarded you &Y%d&O Favor Points!\r\n", ch->name, value );
					}
					else if( value <= 0 )
					{
						vch->questpoints += value;
						ch_printf( vch, "&Y%s&O has penalized you &Y%d&O Favor Points!\r\n", ch->name, value );
					}
				}
			}
		}
		else
		{

			CHAR_DATA *victim;

			if( ( victim = get_char_world( ch, arg3 ) ) == NULL )
			{
				send_to_char( "They aren't here.\r\n", ch );
				return;
			}

			send_to_char( "Ok.\r\n", ch );

			if( value >= 0 )
			{
				victim->questpoints += value;
				ch_printf( victim, "&Y%s&O has awarded you &Y%d&O Favor Points!\r\n", ch->name, value );
			}
			else if( value <= 0 )
			{
				victim->questpoints += value;
				ch_printf( victim, "&Y%s&O has penalized you &Y%d&O Favor Points!\r\n", ch->name, value );
			}
			return;
		}
		return;
	}

	if( !str_prefix( arg1, "bank" ) )
	{

		if( value < -3000000 || value > 3000000 )
		{
			send_to_char( "You can only award between -3000000 and 3000000 banked money.\r\n", ch );
			return;
		}

		if( !str_cmp( arg3, "all" ) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			if( !ch->pcdata )
				return;

			save_char_obj( ch );
			send_to_char( "Ok.\r\n", ch );
			for( vch = first_char; vch; vch = vch_next )
			{
				vch_next = vch->next;

				if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
				{
					if( value >= 0 )
					{
						vch->pcdata->bank += value;
						ch_printf( vch, "&Y%s&O has awarded you &Y%d&O Banked Money!\r\n", ch->name, value );
					}
					else if( value <= 0 )
					{
						vch->pcdata->bank += value;
						ch_printf( vch, "&Y%s&O has penalized you &Y%d&O Banked Money!\r\n", ch->name, value );
					}
				}
			}
		}
		else
		{

			CHAR_DATA *victim;

			if( ( victim = get_char_world( ch, arg3 ) ) == NULL )
			{
				send_to_char( "They aren't here.\r\n", ch );
				return;
			}

			send_to_char( "Ok.\r\n", ch );

			if( value >= 0 )
			{
				victim->pcdata->bank += value;
				ch_printf( victim, "&Y%s&O has awarded you &Y%d&O Banked Money!\r\n", ch->name, value );
			}
			else if( value <= 0 )
			{
				victim->pcdata->bank += value;
				ch_printf( victim, "&Y%s&O has penalized you &Y%d&O Banked Money!\r\n", ch->name, value );
			}
			return;
		}
		return;
	}

	else
	{
		do_award( ch, "" );
		return;
	}

}

CMDF( do_createvoodoo )
{

	char name[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *doll;

	//    if (!IS_NPC(ch) )
	//       return;

	corpse = get_eq_char( ch, WEAR_HOLD );
	if( !corpse || ( corpse->item_type != ITEM_FOOD && corpse->value[5] != 1 ) )
	{
		send_to_char( "You are not holding a body part.\r\n", ch );
		return;
	}

	one_argument( corpse->name, name );
	doll = create_object( get_obj_index( OBJ_VNUM_VOODOO ), 0 );
	snprintf( buf, MAX_STRING_LENGTH, doll->short_descr, name );
	STRFREE( doll->short_descr );
	doll->short_descr = STRALLOC( buf );
	snprintf( buf, MAX_STRING_LENGTH, doll->description, name );
	snprintf( buf, MAX_STRING_LENGTH, doll->name, name );
	STRFREE( doll->name );
	doll->name = STRALLOC( buf );
	act( AT_MAGIC, "$p morphs into a voodoo doll.", ch, corpse, NULL, TO_CHAR );
	obj_from_char( corpse );
	extract_obj( corpse );
	obj_to_char( doll, ch );
	equip_char( ch, doll, WEAR_HOLD );
	act( AT_MAGIC, "$n has created $p!", ch, doll, NULL, TO_ROOM );
	learn_from_success( ch, gsn_createvoodoo );
	return;
}


CMDF( do_camset )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int code, location;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if( !IS_NPC( ch ) && ch->pcdata->lp < 5 )
	{
		send_to_char( "&RYou need atleast 5 LP to use this skill!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "\r\n&BCamset Set &b<&BCam Name&b> <&BCode&b>&B.\r\n", ch );
		send_to_char( "&BCamset connect &b<&BCode&b>&B.\r\n", ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_camset]->beats );

	if( !IS_NPC( ch ) && number_percent( ) > ch->pcdata->learned[gsn_camset] )
	{
		send_to_char( "You failed.\r\n", ch );
		learn_from_failure( ch, gsn_camset );
		return;
	}

	if( !str_prefix( arg, "set" ) )
	{

		obj = get_eq_char( ch, WEAR_HOLD );

		if( !obj || obj->item_type != ITEM_REMOTE )
		{
			send_to_char( "You aren't holding that item.\r\n", ch );
			return;
		}

		if( obj->item_type != ITEM_REMOTE )
		{
			send_to_char( "That isn't a camera.\r\n", ch );
			return;
		}

		if( arg3[0] == '\0' )
		{
			send_to_char( "Which code?\r\n", ch );
			return;
		}

		code = atoi( arg3 );
		if( code <= 0 )
		{
			send_to_char( "Your code needs to be a number higher than 0.", ch );
			return;
		}
		obj->value[0] = code;
		send_to_char( "Camera code added, and ready for use.\r\n", ch );
		return;
	}

	code = atoi( arg2 );
	location = ch->in_room->vnum;
	if( !str_prefix( arg, "connect" ) )
	{
		for( obj = first_object; obj != NULL; obj = obj->next )
		{
			if( obj->item_type == ITEM_REMOTE )
			{
				if( obj->value[0] == code )
				{
					if( obj->value[0] == 0 )
					{
						send_to_char( "Do you think a code would be that easy?\r\n", ch );
						return;
					}


					if( obj->in_room == NULL )
					{
						send_to_char( "You see nothing.\r\n", ch );
						return;
					}

					char_from_room( ch );
					char_to_room( ch, obj->in_room );
					do_look( ch, "auto" );
					char_from_room( ch );
					char_to_room( ch, get_room_index( location ) );
					learn_from_limiter( ch, gsn_camset );
					return;
				}
			}
		}
		send_to_char( "No image can be seen through this channel..\r\n", ch );
		return;
	}
	do_camset( ch, "" );
	return;
}

CMDF( do_bomb )
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	SHIP_DATA *ship;
	int vx, vy, vz;
	char buf[MAX_STRING_LENGTH];
	int destination;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	ROOM_INDEX_DATA *original;
	ROOM_INDEX_DATA *toroom;
	int percent;
	percent = number_range( 0, 100 );

	strcpy( arg, argument );

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
		send_to_char( "&RYou need to be in the pilot seat!\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't land platforms\r\n", ch );
		return;
	}

	if( ship->ship_class == CAPITAL_SHIP )
	{
		send_to_char( "&RCapital ships are to big to land. You'll have to take a shuttle.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. Unable to land.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RThe ship is already docked!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_READY )
	{
		send_to_char( "&RPlease wait until the ship has finished its current manouver.\r\n", ch );
		return;
	}

	if( ship->bombs <= 0 )
	{
		send_to_char( "&RPerhaps you should purchase a bomb to drop first?\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RThere's thing to bomb around here!", ch );
		return;
	}

	if( ship->energy < ( 25 + 25 * ship->ship_class ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		ch_printf( ch, "\n\rYour Coordinates: %.0f %.0f %.0f\r\n", ship->vx, ship->vy, ship->vz );
		return;
	}

	if( str_prefix( argument, ship->starsystem->location1a ) &&
		str_prefix( argument, ship->starsystem->location2a ) &&
		str_prefix( argument, ship->starsystem->location3a ) &&
		str_prefix( argument, ship->starsystem->location1b ) &&
		str_prefix( argument, ship->starsystem->location2b ) &&
		str_prefix( argument, ship->starsystem->location3b ) &&
		str_prefix( argument, ship->starsystem->location1c ) &&
		str_prefix( argument, ship->starsystem->location2c ) && str_prefix( argument, ship->starsystem->location3c ) )

		if( !str_prefix( argument, ship->starsystem->location3a ) ||
			!str_prefix( argument, ship->starsystem->location3b ) || !str_prefix( argument, ship->starsystem->location3c ) )
		{
			vx = ship->starsystem->p3x;
			vy = ship->starsystem->p3y;
			vz = ship->starsystem->p3z;
		}
	if( !str_prefix( argument, ship->starsystem->location2a ) ||
		!str_prefix( argument, ship->starsystem->location2b ) || !str_prefix( argument, ship->starsystem->location2c ) )
	{
		vx = ship->starsystem->p2x;
		vy = ship->starsystem->p2y;
		vz = ship->starsystem->p2z;
	}
	if( !str_prefix( argument, ship->starsystem->location1a ) ||
		!str_prefix( argument, ship->starsystem->location1b ) || !str_prefix( argument, ship->starsystem->location1c ) )
	{
		vx = ship->starsystem->p1x;
		vy = ship->starsystem->p1y;
		vz = ship->starsystem->p1z;
	}
	if( ( vx > ship->vx + 200 ) || ( vx < ship->vx - 200 ) ||
		( vy > ship->vy + 200 ) || ( vy < ship->vy - 200 ) || ( vz > ship->vz + 200 ) || ( vz < ship->vz - 200 ) )
	{
		send_to_char( "&R That platform is too far away! You'll have to fly a litlle closer.\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_bomb] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_bomb] );
	if( number_percent( ) < chance )
	{
		set_char_color( AT_GREEN, ch );
		send_to_char( "\n\rBombing procedures initiated.\r\n", ch );
		act( AT_PLAIN, "$n begins the bombing sequence.", ch, NULL, argument, TO_ROOM );
		ship->dest = STRALLOC( arg );
		ship->currspeed = 0;

		if( !str_prefix( arg, ship->starsystem->location3a ) )
			destination = ship->starsystem->doc3a;
		if( !str_prefix( arg, ship->starsystem->location3b ) )
			destination = ship->starsystem->doc3b;
		if( !str_prefix( arg, ship->starsystem->location3c ) )
			destination = ship->starsystem->doc3c;
		if( !str_prefix( arg, ship->starsystem->location2a ) )
			destination = ship->starsystem->doc2a;
		if( !str_prefix( arg, ship->starsystem->location2b ) )
			destination = ship->starsystem->doc2b;
		if( !str_prefix( arg, ship->starsystem->location2c ) )
			destination = ship->starsystem->doc2c;
		if( !str_prefix( arg, ship->starsystem->location1a ) )
			destination = ship->starsystem->doc1a;
		if( !str_prefix( arg, ship->starsystem->location1b ) )
			destination = ship->starsystem->doc1b;
		if( !str_prefix( arg, ship->starsystem->location1c ) )
			destination = ship->starsystem->doc1c;

		if( !ship_to_room( ship, destination ) )
		{
			echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Could not complete aproach. Landing aborted." );
			echo_to_ship( AT_YELLOW, ship, "The ship pulls back up out of its bombing sequence." );
			if( ship->shipstate != SHIP_DISABLED )
				ship->shipstate = SHIP_READY;
			return;
		}

		echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Bombing sequence activated." );
		snprintf( buf, MAX_STRING_LENGTH, "%s begins to drop bombs.", ship->name );
		echo_to_system( AT_YELLOW, ship, buf, NULL );
		ship->bombs -= 1;
		learn_from_success( ch, gsn_bomb );

		for( vch = first_char; vch; vch = vch_next )
		{
			vch_next = vch->next;
			if( !vch->in_room )
				continue;

			if( ch == vch )
				continue;

			toroom = get_room_index( destination );
			original = ch->in_room;
			char_from_room( ch );
			char_to_room( ch, toroom );
			//        do_look(ch, "auto" );

			if( vch->in_room->area == ch->in_room->area )
			{
				if( !IS_NPC( vch ) )
				{
					int dam;
					if( percent >= 41 )
					{
						send_to_char( "&PThe ground shudders while being bombed from above!\r\n", vch );
						dam = number_range( vch->hit, vch->max_hit ) / 4;
						damage( vch, vch, dam, TYPE_UNDEFINED );
						ch_printf( vch, "&RYou take %d damage from falling rubble!\r\n", dam );
					}
					else if( percent <= 40 )
					{
						send_to_char( "&GYou jump just in time, escaping falling rubble.\r\n", vch );
					}
				}
			}
			char_from_room( ch );
			char_to_room( ch, original );
		}

		save_ship( ship );
		return;
	}
	send_to_char( "You fail to work the controls properly.\r\n", ch );
	learn_from_failure( ch, gsn_bomb );
	return;
}

/*
void bombplanet( SHIP_DATA *ship, char *arg )
{
	SHIP_DATA *target;
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH];
	int destination;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	ROOM_INDEX_DATA *toroom;

	if ( !str_prefix(arg,ship->starsystem->location3a) )
	   destination = ship->starsystem->doc3a;
	if ( !str_prefix(arg,ship->starsystem->location3b) )
	   destination = ship->starsystem->doc3b;
	if ( !str_prefix(arg,ship->starsystem->location3c) )
	   destination = ship->starsystem->doc3c;
	if ( !str_prefix(arg,ship->starsystem->location2a) )
	   destination = ship->starsystem->doc2a;
	if ( !str_prefix(arg,ship->starsystem->location2b) )
	   destination = ship->starsystem->doc2b;
	if ( !str_prefix(arg,ship->starsystem->location2c) )
	   destination = ship->starsystem->doc2c;
	if ( !str_prefix(arg,ship->starsystem->location1a) )
	   destination = ship->starsystem->doc1a;
	if ( !str_prefix(arg,ship->starsystem->location1b) )
	   destination = ship->starsystem->doc1b;
	if ( !str_prefix(arg,ship->starsystem->location1c) )
	   destination = ship->starsystem->doc1c;

	if ( !ship_to_room( ship , destination ) )
	{
	   echo_to_room( AT_YELLOW , get_room_index(ship->pilotseat), "Could
not complete aproach. Landing aborted.");
	   echo_to_ship( AT_YELLOW , ship , "The ship pulls back up out of
its landing sequence.");
	   if (ship->shipstate != SHIP_DISABLED)
		   ship->shipstate = SHIP_READY;
	   return;
	}

	echo_to_room( AT_YELLOW , get_room_index(ship->pilotseat), "Bo
mbing sequence activated.");
	snprintf( buf, MAX_STRING_LENGTH, "%s begins to drop bombs." , ship->name  );
	echo_to_system( AT_YELLOW, ship, buf , NULL );

	for ( vch = first_char; vch; vch = vch_next )
	{
	vch_next	= vch->next;
	if ( !vch->in_room )
		continue;

		toroom = get_room_index( destination );
		original = ch->in_room;
		char_from_room( ch );
		char_to_room( ch, toroom );
		do_look(ch, "auto" );

	if ( vch->in_room->area == ch->in_room->area )
	{
		set_char_color( AT_MAGIC, vch );
		send_to_char( "The earth trembles and shivers.\r\n", vch );
	}
		char_from_room( ch );
		char_to_room( ch, original );
	}


	save_ship(ship);
	return;
}

*/

CMDF( do_seize )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Seize what from whom?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( victim = get_char_world( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "That's pointless.\r\n", ch );
		return;
	}

	if( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
	{
		if( victim->position <= POS_SLEEPING )
		{
			if( ( obj = get_obj_wear( victim, arg1 ) ) != NULL )
			{
				unequip_char( victim, obj );
			}
		}

		send_to_char( "You can't seem to find it.\r\n", ch );
		return;
	}

	obj_from_char( obj );
	obj_to_char( obj, ch );
	send_to_char( "Got it!\r\n", ch );
	ch_printf( victim, "%s has seized %s&w from you!\r\n", ch->name, obj->short_descr );
	return;

	send_to_char( "You can't find it.\r\n", ch );
	return;
}

/********************************
Trap Values:
V0 - Type

1 - Gas
2 - Bear
3 - Acid
4 - Energy Net
5 - PDart
6 - BackLash

********************************/

CMDF( do_trap )
{
	char arg1[MAX_INPUT_LENGTH];
	OBJ_DATA *ptrap;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA af;
	int dam = 0;

	argument = one_argument( argument, arg1 );

	if( IS_NPC( ch ) )
		return;

	if( !IS_NPC( ch ) && ch->pcdata->lp < 30 )
	{
		send_to_char( "&RYou need atleast 30 LP to use this skill!\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: trap <type>\r\n\r\n", ch );
		send_to_char( " Type: Gas, Bear, Acid, Energy Net\r\n", ch );
		send_to_char( " Type: Pdart, Lash\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "gas" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 30 )
		{
			send_to_char( "&RYou need atleast 30 LP to use this skill!\r\n", ch );
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_gas]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_gas] + ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			send_to_char( "&RYou failed! Your trap is set off into your face!\r\n", ch );

			if( !IS_AFFECTED( ch, AFF_SLEEP ) )
			{
				af.type = gsn_sleep;
				af.duration = ch->top_level / 50;
				af.location = APPLY_NONE;
				af.modifier = 0;
				af.bitvector = AFF_SLEEP;
				affect_join( ch, &af );
			}
			send_to_char( "You feel very sleepy ..... zzzzzz.\r\n", ch );
			act( AT_BLUE, "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
			ch->position = POS_SLEEPING;
			learn_from_failure( ch, gsn_gas );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 1;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou lay down a small trip wire, ready to catch a victim.\r\n", ch );
		act( AT_PLAIN, "$n snaps a small trip wire into place.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_gas );
		learn_from_limiter( ch, gsn_trap );
		return;

	}

	if( !str_cmp( arg1, "bear" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 38 )
		{
			send_to_char( "&RYou need atleast 38 LP to use this skill!\r\n", ch );
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_bear]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_bear] + ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			send_to_char( "&RYou failed! Your foot gets stuck in the trap!\r\n", ch );
			act( AT_PLAIN, "$n messed up, stepping in $s own trap!", ch, NULL, argument, TO_ROOM );
			dam = number_range( ch->top_level, ch->top_level * 10 );
			damage( ch, ch, dam, TYPE_UNDEFINED );
			ch_printf( ch, "&RYou take &P%d&R damage from the bear trap!\r\n", dam );
			learn_from_failure( ch, gsn_bear );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 2;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou place a small sharp trap on the ground.\r\n", ch );
		act( AT_PLAIN, "$n places a small sharp trap on the ground.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_bear );
		learn_from_limiter( ch, gsn_trap );
		return;

	}

	if( !str_cmp( arg1, "acid" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 32 )
		{
			send_to_char( "&RYou need atleast 32 LP to use this skill!\r\n", ch );
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_acid]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_acid] + ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			if( !IS_AFFECTED( ch, AFF_BLIND ) )
			{
				af.type = gsn_blindness;
				af.location = APPLY_HITROLL;
				af.modifier = -6;
				af.duration = ch->top_level / 20;
				af.bitvector = AFF_BLIND;
				affect_to_char( ch, &af );
				act( AT_SKILL, "&RYou messed up, acid gets poured into your eyes!\r\n&BYou can't see a thing!", ch, NULL, NULL,
					TO_CHAR );
			}
			act( AT_BLUE, "$n just messed up, being blinded by acid!", ch, NULL, NULL, TO_ROOM );
			learn_from_failure( ch, gsn_acid );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 3;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou place a small trip wire on the ground.\r\n", ch );
		act( AT_PLAIN, "$n places a small trip wire on the ground.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_acid );
		learn_from_limiter( ch, gsn_trap );
		return;

	}

	if( !str_cmp( arg1, "energy net" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 37 )
		{
			send_to_char( "&RYou need atleast 37 LP to use this skill!\r\n", ch );
			return;
		}


		WAIT_STATE( ch, skill_table[gsn_energynet]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_energynet]
			+ ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			if( !IS_AFFECTED( ch, AFF_PARALYSIS ) )
			{
				af.type = gsn_stun;
				af.location = APPLY_AC;
				af.modifier = 150;
				af.duration = ch->top_level / 20;
				af.bitvector = AFF_PARALYSIS;
				affect_to_char( ch, &af );
				act( AT_SKILL, "&RYou mess up, springing your own trap! You're caught in your own energy net!", ch, NULL, NULL,
					TO_CHAR );
				update_pos( ch );
			}
			act( AT_BLUE, "$n messed up! An energy net encases itself over $n!", ch, NULL, NULL, TO_ROOM );
			learn_from_failure( ch, gsn_energynet );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 4;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou place a small trip wire on the ground.\r\n", ch );
		act( AT_PLAIN, "$n places a small trip wire on the ground.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_energynet );
		learn_from_limiter( ch, gsn_trap );
		return;

	}

	if( !str_cmp( arg1, "pdart" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 33 )
		{
			send_to_char( "&RYou need atleast 33 LP to use this skill!\r\n", ch );
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_pdart]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_pdart] + ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			if( !IS_AFFECTED( ch, AFF_POISON ) )
			{
				af.type = gsn_poison;
				af.location = APPLY_HITROLL;
				af.modifier = -6;
				af.duration = ch->top_level / 20;
				af.bitvector = AFF_POISON;
				affect_to_char( ch, &af );
				act( AT_SKILL, "&RYou messed up! OW the tipped dart dropped into your leg!", ch, NULL, NULL, TO_CHAR );
			}
			act( AT_BLUE, "$n messed up! $m lets out a yelp as $m gets stuck by the poison tipped dart!", ch, NULL, NULL,
				TO_ROOM );
			learn_from_failure( ch, gsn_poison );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 5;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou place a small trip wire on the ground.\r\n", ch );
		act( AT_PLAIN, "$n places a small trip wire on the ground.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_pdart );
		learn_from_limiter( ch, gsn_trap );
		return;

	}

	if( !str_cmp( arg1, "backlash" ) )
	{
		if( !IS_NPC( ch ) && ch->pcdata->lp < 39 )
		{
			send_to_char( "&RYou need atleast 39 LP to use this skill!\r\n", ch );
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_backlash]->beats );

		if( !IS_NPC( ch ) && number_percent( ) > ( ( ch->pcdata->learned[gsn_backlash]
			+ ch->pcdata->learned[gsn_trap] ) / 2 ) )
		{
			send_to_char( "&RYou failed! Your backlash springs at you, hitting you in the face!\r\n", ch );
			act( AT_PLAIN, "$n messed up, $s trap springing back into $s face!", ch, NULL, argument, TO_ROOM );
			dam = number_range( ch->top_level * 2, ch->top_level * 15 );
			damage( ch, ch, dam, TYPE_UNDEFINED );
			ch_printf( ch, "&RYou take &P%d&R damage from the backlash trap!\r\n", dam );
			learn_from_failure( ch, gsn_backlash );
			return;
		}

		ptrap = create_object( get_obj_index( OBJ_VNUM_PTRAP ), 0 );
		ptrap->value[0] = 6;
		ptrap->value[1] = ch->top_level;
		snprintf( buf, MAX_STRING_LENGTH, "%s", ch->name );
		STRFREE( ptrap->short_descr );
		ptrap->short_descr = STRALLOC( buf );
		ptrap = obj_to_room( ptrap, ch->in_room );
		send_to_char( "&GYou place a small sharp trap on the ground.\r\n", ch );
		act( AT_PLAIN, "$n places a small sharp trap on the ground.", ch, NULL, argument, TO_ROOM );
		learn_from_limiter( ch, gsn_backlash );
		learn_from_limiter( ch, gsn_trap );
		return;
	}
	do_trap( ch, "" );
	return;
}


CMDF( do_createdrug )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	OBJ_DATA *drug1;
	OBJ_DATA *drug2;
	OBJ_DATA *drug3;
	OBJ_DATA *cdrug;
	int dtype;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "You need 3 things to create a drug!\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		send_to_char( "You need 2 more things to create a drug!\r\n", ch );
		return;
	}


	if( arg3[0] == '\0' )
	{
		send_to_char( "You need 1 more thing to create a drug!\r\n", ch );
		return;
	}

	if( ( drug1 = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You need to have 3 things to create a drug with!\r\n", ch );
		return;
	}

	if( ( drug2 = get_obj_carry( ch, arg2 ) ) == NULL )
	{
		send_to_char( "You need to have 3 things to create a drug with!\r\n", ch );
		return;
	}

	if( ( drug3 = get_obj_carry( ch, arg3 ) ) == NULL )
	{
		send_to_char( "You need to have 3 things to create a drug with!\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && number_percent( ) > ch->pcdata->learned[gsn_createdrug] )
	{
		send_to_char( "&RYou failed! Your drug burns to a crisp!\r\n", ch );
		extract_obj( drug1 );
		extract_obj( drug2 );
		extract_obj( drug3 );
		return;
	}

	dtype = ( drug1->value[3] + drug2->value[3] + drug3->value[3] );


	if( dtype == 0 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG1 ), 0 );
	if( dtype == 1 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG1 ), 0 );
	if( dtype == 2 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG2 ), 0 );
	if( dtype == 3 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG3 ), 0 );
	if( dtype == 4 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG4 ), 0 );
	if( dtype == 5 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG5 ), 0 );
	if( dtype == 6 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG6 ), 0 );
	if( dtype == 7 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG7 ), 0 );
	if( dtype == 8 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG8 ), 0 );
	if( dtype == 9 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG9 ), 0 );
	if( dtype == 10 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG10 ), 0 );
	if( dtype == 11 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG11 ), 0 );
	if( dtype == 12 )
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG12 ), 0 );
	else
		cdrug = create_object( get_obj_index( OBJ_VNUM_DRUG7 ), 0 );


	cdrug->value[3] = dtype;
	cdrug->cost = ( dtype * 100 );
	obj_to_char( cdrug, ch );
	send_to_char( "You created a drug! Yay!\r\n", ch );
	extract_obj( drug1 );
	extract_obj( drug2 );
	extract_obj( drug3 );
	learn_from_success( ch, gsn_createdrug );
	return;

}

CMDF( do_rmaffect )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Rmaffect <chara>\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That person isn't here.\r\n", ch );
		return;
	}


	{
		send_to_char( "All affects cleared.\r\n", ch );

		if( str_cmp( victim->name, ch->name ) )
			send_to_char( "Your affects have been stripped away!\r\n", victim );

		while( victim->first_affect )
			affect_remove( victim, victim->first_affect );

		//        victim->affected_by = NULL;
		return;
	}

}

CMDF( do_artifacts )
{
	OBJ_DATA *obj;

	send_to_char( "\r\n&BArtifact                    Where\r\n&G==========================================\r\n", ch );

	for( obj = first_object; obj; obj = obj->next )
	{
		if( !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
			continue;

		ch_printf( ch, "&C%s&c   ", obj->short_descr );
		if( obj->carried_by )
			ch_printf( ch, "     carried by %-15s\r\n", PERS( obj->carried_by, ch ) );
		else if( obj->in_room )
			ch_printf( ch, "     laying in %s\r\n", obj->in_room->name );
		else if( obj->in_obj )
			ch_printf( ch, " object [%5d] %s\r\n", obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
		else
		{
			bug( "do_artifacts: artifact isn't anywhere. Transfering to limbo!", 0 );
			obj_to_room( obj, get_room_index( ROOM_VNUM_LIMBO ) );
			send_to_char( "nowhere??\r\n", ch );
		}

	}

	return;
}

CMDF( do_newspaper )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if( IS_NPC( ch ) )
		return;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Whack whom with your newspaper?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) )
		ch_printf( victim, "&wYou just got smacked on the nose by %s's newspaper!\r\n", ch->name );
	ch_printf( ch, "&wYou thwap %s on the nose with your newspaper!\r\n", victim->name );
	if( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
	{
		af.type = gsn_stun;
		af.location = APPLY_AC;
		af.modifier = 5;
		af.duration = 7;
		af.bitvector = AFF_PARALYSIS;
		affect_to_char( victim, &af );
		update_pos( victim );
	}
	act( AT_SOCIAL, "$n falls to the ground after getting smacked on the nose by $N's newspaper.", victim, NULL, victim,
		TO_NOTVICT );
	victim->hit = -100;
	return;
}
