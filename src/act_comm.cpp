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
#include <time.h>
#include "mud.h"

/*
 *  Externals
 */
void send_obj_page_to_char( CHAR_DATA *ch, OBJ_INDEX_DATA *idx, char page );
void send_room_page_to_char( CHAR_DATA *ch, ROOM_INDEX_DATA *idx, char page );
void send_page_to_char( CHAR_DATA *ch, MOB_INDEX_DATA *idx, char page );
void send_control_page_to_char( CHAR_DATA *ch, char page );
bool check_social( CHAR_DATA *ch, const char *command, const char *argument );

#define MAX_CHANBACK 15

const char *chan_back_table[MAX_CHANNEL][MAX_CHANBACK];

/*
 * Local functions.
 */
void talk_channel( CHAR_DATA *ch, const char *argument, int channel, const char *verb );

const char *drunk_speech( const char *argument, CHAR_DATA *ch );
void drop_artifacts( CHAR_DATA *ch, OBJ_DATA *obj );  /* Scion */

void sound_to_room( ROOM_INDEX_DATA *room, const char *argument )
{
	CHAR_DATA *vic;

	if( room == NULL )
		return;

	for( vic = room->first_person; vic; vic = vic->next_in_room )
		if( !IS_NPC( vic ) && xIS_SET( vic->act, PLR_SOUND ) )
			send_to_char( argument, vic );

}


CMDF( do_beep )
{
	CHAR_DATA *victim;
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	bool ch_comlink, victim_comlink;

	argument = one_argument( argument, arg );

	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) || xIS_SET( ch->act, PLR_NO_TELL ) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Beep who?\r\n", ch );
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
		send_to_char( "They don't seem to have a comlink!\r\n", ch );
		return;
	}

	if( NOT_AUTHED( ch ) && !NOT_AUTHED( victim ) && !IS_IMMORTAL( victim ) )
	{
		send_to_char( "They can't hear you because you are not authorized.\r\n", ch );
		return;
	}

	if( IS_SET( victim->pcdata->flags, PCFLAG_DND ) )
	{
		send_to_char( "Sorry, you can't send tells to a person in DND mode.", ch );
		return;
	}

	if( !IS_NPC( victim ) && ( victim->switched ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}

	else if( !IS_NPC( victim ) && ( !victim->desc ) )
	{
		send_to_char( "That player is link-dead.\r\n", ch );
		return;
	}

	if( IS_SET( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
	{
		act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->act, PLR_SILENCE ) ) )
	{
		send_to_char( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );
	}

	if( ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
		|| ( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
	{
		act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if( victim->desc /* make sure desc exists first  -Thoric */
		&& victim->desc->connected == CON_EDITING && get_trust( ch ) < LEVEL_LIAISON )
	{
		act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	ch_printf( ch, "&WYou beep %s: %s\r\n\a", victim->name, argument );
	send_to_char( "\a", victim );

	act( AT_WHITE, "$n beeps: '$t'", ch, argument, victim, TO_VICT );
	return;
}

/* I'll rewrite this later if its still needed.. -- Altrag */
const char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}

const char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
	const char *arg = argument;
	static char buf[MAX_INPUT_LENGTH * 2];
	char buf1[MAX_INPUT_LENGTH * 2];
	short drunk;
	char *txt;
	char *txt1;

	if( IS_NPC( ch ) || !ch->pcdata )
		return ( char * ) argument;

	drunk = ch->pcdata->condition[COND_DRUNK];

	if( drunk <= 0 )
		return ( char * ) argument;

	buf[0] = '\0';
	buf1[0] = '\0';

	if( !argument )
	{
		bug( "Drunk_speech: NULL argument", 0 );
		return "";
	}

	/*
	 * if ( *arg == '\0' )
	 * return (char *) argument;
	 */

	txt = buf;
	txt1 = buf1;

	while( *arg != '\0' )
	{
		if( toupper( *arg ) == 'S' )
		{
			if( number_percent( ) < ( drunk * 2 ) )   /* add 'h' after an 's' */
			{
				*txt++ = *arg;
				*txt++ = 'h';
			}
			else
				*txt++ = *arg;
		}
		else if( toupper( *arg ) == 'X' )
		{
			if( number_percent( ) < ( drunk * 2 / 2 ) )
			{
				*txt++ = 'c', *txt++ = 's', *txt++ = 'h';
			}
			else
				*txt++ = *arg;
		}
		else if( number_percent( ) < ( drunk * 2 / 5 ) ) /* slurred letters */
		{
			short slurn = number_range( 1, 2 );
			short currslur = 0;

			while( currslur < slurn )
				*txt++ = *arg, currslur++;
		}
		else
			*txt++ = *arg;

		arg++;
	};

	*txt = '\0';

	txt = buf;

	while( *txt != '\0' )    /* Let's mess with the string's caps */
	{
		if( number_percent( ) < ( 2 * drunk / 2.5 ) )
		{
			if( isupper( *txt ) )
				*txt1 = tolower( *txt );
			else if( islower( *txt ) )
				*txt1 = toupper( *txt );
			else
				*txt1 = *txt;
		}
		else
			*txt1 = *txt;

		txt1++, txt++;
	};

	*txt1 = '\0';
	txt1 = buf1;
	txt = buf;

	while( *txt1 != '\0' )   /* Let's make them stutter */
	{
		if( *txt1 == ' ' )    /* If there's a space, then there's gotta be a */
		{ /* along there somewhere soon */

			while( *txt1 == ' ' )  /* Don't stutter on spaces */
				*txt++ = *txt1++;

			if( ( number_percent( ) < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
			{
				short offset = number_range( 0, 2 );
				short pos = 0;

				while( *txt1 != '\0' && pos < offset )
					*txt++ = *txt1++, pos++;

				if( *txt1 == ' ' )  /* Make sure not to stutter a space after */
				{   /* the initial offset into the word */
					*txt++ = *txt1++;
					continue;
				}

				pos = 0;
				offset = number_range( 2, 4 );
				while( *txt1 != '\0' && pos < offset )
				{
					*txt++ = *txt1;
					pos++;
					if( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */
					{    /* A hyphen right before a space */
						txt1--;
						break;
					}
					*txt++ = '-';
				}
				if( *txt1 != '\0' )
					txt1++;
			}
		}
		else
			*txt++ = *txt1++;
	}

	*txt = '\0';

	return buf;
}

void update_chanback( const char *string, int bitv )
{
	int channel = -1;
	int x;

	if( !string )
	{
		bug( "%s: NULL string passed", __FUNCTION__ );
		return;
	}

	/*
	 * Figure out which channel value we need. Has to be
	 * an easier way. -- Halcyon
	 */
	for( x = 0; x < 32; x++ )
	{
		if( bitv >> x == 1 )
		{
			channel = x;
			break;
		}
	}

	if( channel < 0 )
	{
		bug( "%s: unknown bitvector value '%d' passed", __FUNCTION__, bitv );
		return;
	}

	/*
	 * Check to see if the chan's backlog has been initialized.
	 * If not, well... Initialize it? -- Halcyon
	 */
	if( !chan_back_table[channel][0] )
	{
		for( x = 0; x < MAX_CHANBACK; x++ )
			chan_back_table[channel][x] = STRALLOC( "" );
	}

	for( x = 0; x < MAX_CHANBACK; x++ )
	{
		if( chan_back_table[channel][x][0] == '\0' )
		{
			STRFREE( chan_back_table[channel][x] );
			chan_back_table[channel][x] = STRALLOC( string );
			return;
		}
	}

	/*
	 * If we didn't get a return by now, it's because the table's
	 * full, so let's do some SUPER crappy looping and cycle it all
	 * back! :D -- Halcyon
	 */

	for( x = 0; x < ( MAX_CHANBACK - 1 ); x++ )
	{
		STRFREE( chan_back_table[channel][x] );
		chan_back_table[channel][x] = STRALLOC( chan_back_table[channel][x + 1] );
	}

	STRFREE( chan_back_table[channel][( MAX_CHANBACK - 1 )] );
	chan_back_table[channel][( MAX_CHANBACK - 1 )] = STRALLOC( string );
	return;
}

void chanback( CHAR_DATA *ch, int bitv, const char *verb )
{
	char buf[MAX_STRING_LENGTH];
	int channel = -1;
	int x;

	for( x = 0; x < 32; x++ )
	{
		if( bitv >> x == 1 )
		{
			channel = x;
			break;
		}
	}

	if( channel < 0 )
	{
		bug( "%s: unknown bitvector value '%d' passed", __FUNCTION__, bitv );
		return;
	}

	if( !chan_back_table[channel][0] || chan_back_table[channel][0][0] == '\0' )
	{
		sprintf( buf, "&RNobody's used the '%s' channel!\r\n", verb );
		send_to_char( buf, ch );
		return;
	}
	else
	{
		sprintf( buf, "&WChanback for '%s' channel:\r\n", verb );
		send_to_char( buf, ch );
		for( x = 0; x < MAX_CHANBACK; x++ )
		{
			if( !chan_back_table[channel][x] || chan_back_table[channel][x][0] == '\0' )
			{
				if( x == 0 )
					bug( "%s: chanback for channel #%d initialized, but has no data!", __FUNCTION__, channel );
				return;
			}
			send_to_char( chan_back_table[channel][x], ch );
		}
		send_to_char( "\r\n", ch );
		return;
	}
}

void clear_chanback( int bitv )
{
	int x, channel;

	for( x = 0; x < 32; x++ )
	{
		if( bitv >> x == 1 )
		{
			channel = x;
			break;
		}
	}

	if( channel < 0 )
	{
		bug( "%s: unknown bitvector value '%d' passed", __FUNCTION__, bitv );
		return;
	}

	for( x = 0; x < MAX_CHANBACK; x++ )
	{
		if( chan_back_table[channel][x] )
			STRFREE( chan_back_table[channel][x] );
		chan_back_table[channel][x] = STRALLOC( "" );
	}
	return;
}


char *act_string( const char *format, CHAR_DATA *to, CHAR_DATA *ch, const void *arg1, const void *arg2 );

/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, const char *argument, int channel, const char *verb )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int position;
	CLAN_DATA *clan = NULL;

	/*   if( !has_rate( ch, RATE_CHANNELS ) )
	   {
		   send_to_char( "You don't rate channels! HELP RATE for more info.\r\n", ch );
		   return;
	   }*/

	bool ch_comlink = false;

	if( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL && channel != CHANNEL_IMMTALK && channel != CHANNEL_OOC && channel != CHANNEL_ASK && channel != CHANNEL_NEWBIE   /*&& channel != CHANNEL_AVTALK */
		&& channel != CHANNEL_SHIP && channel != CHANNEL_SYSTEM && channel != CHANNEL_SPACE
		&& channel != CHANNEL_RP && channel != CHANNEL_i104 && channel != CHANNEL_QUOTE )
	{
		OBJ_DATA *obj;

		if( IS_IMMORTAL( ch ) )
			ch_comlink = true;
		else
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

	}

	if( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) && channel == CHANNEL_CLAN )
	{
		send_to_char( "Mobs can't be in clans.\r\n", ch );
		return;
	}

	if( channel == CHANNEL_CLAN )
	{
		if( ch->pcdata->clan->mainclan )
			clan = ch->pcdata->clan->mainclan;
		else
			clan = ch->pcdata->clan;
	}

	if( IS_NPC( ch ) && channel == CHANNEL_ORDER )
	{
		send_to_char( "Mobs can't be in orders.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		if( ch->master )
			send_to_char( "I don't think so...\r\n", ch->master );
		return;
	}

	if( IS_IMMORTAL( ch ) && !str_cmp( argument, ":clear:" ) )
	{
		clear_chanback( channel );
		send_to_char( "Ok.\r\n", ch );
		return;
	}

	if( !argument || argument[0] == '\0' )
	{
		if( channel != CHANNEL_CLAN )
		{
			chanback( ch, channel, verb );
			return;
		}
		sprintf( buf, "%s what?\r\n", verb );
		buf[0] = UPPER( buf[0] );
		send_to_char( buf, ch );  /* where'd this line go? */
		return;
	}

	if( !IS_NPC( ch ) && ( ( ch->top_level < 8 && channel != CHANNEL_YELL )
		|| ( IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) ) ) )
		channel = CHANNEL_NEWBIE;

	REMOVE_BIT( ch->deaf, channel );

	switch( channel )
	{
	default:
		set_char_color( AT_GOSSIP, ch );
		ch_printf( ch, "&B[&CI&cC&B] &cYou say&B, '&C%s&B'\r\n", argument );
		sprintf( buf, "&B[&CI&cC&B] &c$n says&B, '&C$t&B'" );
		break;
	case CHANNEL_CLANTALK:
		set_char_color( AT_CLAN, ch );
		ch_printf( ch, "&Y|&RG&rT&RM&Y| &RYou send&r, &Y'&r%s&Y'\r\n", argument );
		sprintf( buf, "&Y|&RG&rT&RM&Y| &R$n sends&r, &Y'&r$t&Y'" );
		break;
	case CHANNEL_MUSIC:
		set_char_color( AT_GOSSIP, ch );
		ch_printf( ch, "&CYou sing, '%s'\r\n", argument );
		sprintf( buf, "$n sings, '$t'" );
		break;
	case CHANNEL_SHIP:
		set_char_color( AT_SHIP, ch );
		ch_printf( ch, "You tell the ship, '%s'\r\n", argument );
		sprintf( buf, "$n says over the ships com system, '$t'" );
		break;
	case CHANNEL_SYSTEM:
	case CHANNEL_SPACE:
		set_char_color( AT_GOSSIP, ch );
		ch_printf( ch, "You %s, '%s'\r\n", verb, argument );
		sprintf( buf, "$n %ss, '$t'", verb );
		break;
	case CHANNEL_YELL:
	case CHANNEL_SHOUT:
		set_char_color( AT_GOSSIP, ch );
		ch_printf( ch, "You %s, '%s'\r\n", verb, argument );
		sprintf( buf, "$n %ss, '$t'", verb );
		break;
	case CHANNEL_ASK:
		set_char_color( AT_OOC, ch );
		ch_printf( ch, "You %s, '%s'\r\n", verb, argument );
		sprintf( buf, "$n %ss, '$t'", verb );
		break;
	case CHANNEL_NEWBIE:
		set_char_color( AT_OOC, ch );
		ch_printf( ch, "(NEWBIE) %s: %s\r\n", ch->name, argument );
		sprintf( buf, "%s", "(NEWBIE) $n: $t" );
		break;
	case CHANNEL_OOC:
		set_char_color( AT_GREEN, ch );
		if( ch->top_level < LEVEL_STAFF )
			sprintf( buf, "&G(&B=&CO&cO&CC&B=&G) &C%s&B:&G $t", ch->name );
		else
			sprintf( buf, "&G(&B=&CI&cM&CM&R:&CO&cO&CC&B=&G) &C%s&B:&G $t", ch->name );
		position = ch->position;
		ch->position = POS_STANDING;
		act( AT_OOC, buf, ch, argument, NULL, TO_CHAR );
		ch->position = position;
		break;
	case CHANNEL_WARTALK:
		set_char_color( AT_WARTALK, ch );
		sprintf( buf, "&C(&c~&zGmote&c~&C) &c$n $t" );
		position = ch->position;
		ch->position = POS_STANDING;
		act( AT_WARTALK, buf, ch, argument, NULL, TO_CHAR );
		ch->position = position;
		break;
	case CHANNEL_PEEKAY:
		set_char_color( AT_RED, ch );
		ch_printf( ch, "&R(&r[&WPK&r]&R) &rYou say, '&R%s&r'\r\n", argument );
		sprintf( buf, "%s", "&R(&r[&WPK&r]&R) &r$n says, '&R$t&r'" );
		break;
	case CHANNEL_IMMTALK:
	case CHANNEL_RP:
	case CHANNEL_i104:
	case CHANNEL_GOCIAL:
	case CHANNEL_QUOTE:
		if( channel == CHANNEL_IMMTALK )
			sprintf( buf, "&R{&wIMM&R} &w$n&G: &Y$t" );
		else if( channel == CHANNEL_RP )
			sprintf( buf, "&C[&c>&RR&ro&Rl&re&PP&pl&Pa&py&c<&C] &R$n&p: &c$t" );
		else if( channel == CHANNEL_i104 )
			sprintf( buf, "&G[&YA&OT&YG&G] &Y$n&G: &O$t" );
		else if( channel == CHANNEL_QUOTE )
			sprintf( buf, "&G&g$n quotes, '$t&g'" );
		position = ch->position;
		ch->position = POS_STANDING;
		act( AT_IMMORT, buf, ch, argument, NULL, TO_CHAR );
		ch->position = position;
		break;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf2, "%s: %s (%s)", IS_NPC( ch ) ? ch->short_descr : ch->name, argument, verb );
		append_to_file( LOG_FILE, buf2 );
	}

	for( d = first_descriptor; d; d = d->next )
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->original ? d->original : d->character;
		vch = d->character;

		if( d->connected == CON_PLAYING && vch != ch && !IS_SET( och->deaf, channel ) )
		{
			const char *sbuf = argument;
			ch_comlink = false;

			if( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL && channel != CHANNEL_IMMTALK
				&& channel != CHANNEL_OOC && channel != CHANNEL_ASK && channel != CHANNEL_NEWBIE
				&& channel != CHANNEL_SHIP && channel != CHANNEL_SYSTEM && channel != CHANNEL_SPACE
				&& channel != CHANNEL_RP && channel != CHANNEL_QUOTE )
			{
				OBJ_DATA *obj;

				if( IS_IMMORTAL( och ) )
					ch_comlink = true;
				else
					for( obj = och->last_carrying; obj; obj = obj->prev_content )
					{
						if( obj->pIndexData->item_type == ITEM_COMLINK )
							ch_comlink = true;
					}

				if( !ch_comlink )
					continue;
			}

			if( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) )
				continue;
			if( channel == CHANNEL_RP && NOT_AUTHED( och ) )
				continue;
			if( channel == CHANNEL_WARTALK && NOT_AUTHED( och ) )
				continue;
			/*
			 * if ( channel == CHANNEL_AVTALK && !IS_HERO(och) )
			 * continue;
			 */

			if( xIS_SET( vch->in_room->room_flags, ROOM_SILENCE ) )
				continue;

			if( xIS_SET( vch->in_room->room_flags, ROOM_CYBER ) )
				continue;

			if( channel == CHANNEL_YELL || channel == CHANNEL_SHOUT )
			{
				if( ch->in_room != och->in_room )
					continue;
			}

			if( channel == CHANNEL_PEEKAY && !IS_NPC( och ) && !IS_IMMORTAL( och ) && !xIS_SET( och->act, PLR_PKER ) )
				continue;

			/*
			 * Check to see if target is ignoring the sender
			 */
			if( is_ignoring( vch, ch ) )
			{
				/*
				 * If the sender is an imm then they cannot be ignored
				 */
				if( !IS_IMMORTAL( ch ) || vch->top_level > ch->top_level )
				{
					/*
					 * Off to oblivion!
					 */
					continue;
				}
				else
					set_char_color( AT_PLAIN, vch );
			}

			if( channel == CHANNEL_CLAN || channel == CHANNEL_ORDER )
			{

				if( IS_NPC( vch ) )
					continue;

				if( !vch->pcdata->clan )
					continue;

				if( vch->pcdata->clan != clan && vch->pcdata->clan->mainclan != clan )
					continue;
			}

			if( channel == CHANNEL_SHIP || channel == CHANNEL_SPACE || channel == CHANNEL_SYSTEM )
			{
				SHIP_DATA *ship = ship_from_cockpit( ch->in_room->vnum );
				SHIP_DATA *target;

				if( !ship )
					continue;

				if( !vch->in_room )
					continue;

				if( channel == CHANNEL_SHIP )
					if( vch->in_room->vnum > ship->lastroom || vch->in_room->vnum < ship->firstroom )
						continue;

				target = ship_from_cockpit( vch->in_room->vnum );

				if( !target )
					continue;

				if( channel == CHANNEL_SYSTEM )
					if( target->starsystem != ship->starsystem )
						continue;
			}

			position = vch->position;
			if( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
				vch->position = POS_STANDING;
			sbuf = argument;
			MOBtrigger = false;
			if( channel == CHANNEL_IMMTALK || channel == CHANNEL_RP || channel == CHANNEL_i104 || channel == CHANNEL_QUOTE )
				act( AT_IMMORT, buf, ch, sbuf, vch, TO_VICT );
			else if( channel == CHANNEL_WARTALK )
				act( AT_WARTALK, buf, ch, sbuf, vch, TO_VICT );
			else if( channel == CHANNEL_OOC || channel == CHANNEL_NEWBIE || channel == CHANNEL_ASK )
				act( AT_OOC, buf, ch, sbuf, vch, TO_VICT );
			else if( channel == CHANNEL_SHIP )
				act( AT_SHIP, buf, ch, sbuf, vch, TO_VICT );
			else if( channel == CHANNEL_CLAN )
				act( AT_CLAN, buf, ch, sbuf, vch, TO_VICT );
			else if( channel == CHANNEL_PEEKAY )
				act( AT_BLOOD, buf, ch, sbuf, vch, TO_VICT );
			else
				act( AT_GOSSIP, buf, ch, sbuf, vch, TO_VICT );
			vch->position = position;
		}
	}
	{
		char cbuf[MAX_STRING_LENGTH];

		sprintf( cbuf, "%s", act_string( buf, ch, ch, argument, NULL ) );
		update_chanback( cbuf, channel );
	}
	return;
}

void to_channel( const char *argument, int channel, const char *verb, short level )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	if( !first_descriptor || argument[0] == '\0' )
		return;

	sprintf( buf, "%s: %s\r\n", verb, argument );

	for( d = first_descriptor; d; d = d->next )
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->original ? d->original : d->character;
		vch = d->character;

		if( !och || !vch )
			continue;
		if( !IS_IMMORTAL( vch )
			|| ( get_trust( vch ) < sysdata.build_level && channel == CHANNEL_BUILD )
			|| ( get_trust( vch ) < sysdata.log_level && ( channel == CHANNEL_LOG || channel == CHANNEL_COMM ) ) )
			continue;

		if( d->connected == CON_PLAYING && !IS_SET( och->deaf, channel ) && get_trust( vch ) >= level )
		{
			set_char_color( AT_LOG, vch );
			send_to_char( buf, vch );
		}
	}

	return;
}

CMDF( do_chat )
{
	talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_CHAT, "ic" );
	return;
}

CMDF( do_shiptalk )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_SHIP, "shiptalk" );
	return;
}

CMDF( do_systemtalk )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_SYSTEM, "systemtalk" );
	return;
}

CMDF( do_spacetalk )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_SPACE, "spacetalk" );
	return;
}

CMDF( do_ooc )
{
	if( !IS_NPC( ch ) )
	{
		if( IS_SET( ch->pcdata->flags, PCFLAG_NO_OOC ) )
		{
			send_to_char( "Sorry, you can't use OOC.\r\n", ch );
			return;
		}
	}

	talk_channel( ch, argument, CHANNEL_OOC, "ooc" );
	return;
}

CMDF( do_pktalk )
{
	if( NOT_AUTHED( ch ) || IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) && !xIS_SET( ch->act, PLR_PKER ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	talk_channel( ch, argument, CHANNEL_PEEKAY, "pktalk" );
	return;
}

CMDF( do_clantalk )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	talk_channel( ch, argument, CHANNEL_CLAN, "clantalk" );
	return;
}

CMDF( do_newbiechat )
{
	if( ch->top_level > 6 && get_trust( ch ) < LEVEL_STAFF )
	{
		send_to_char( "Aren't you a little old for the newbie channel?\r\n", ch );
		return;
	}

	talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
	WAIT_STATE( ch, 12 );
	return;
}

CMDF( do_ot )
{
	do_ordertalk( ch, argument );
}

CMDF( do_ordertalk )
{
	send_to_char( "Huh?\r\n", ch );
	return;
}

CMDF( do_guildtalk )
{
	send_to_char( "Huh?\r\n", ch );
	return;
}

CMDF( do_music )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	talk_channel( ch, argument, CHANNEL_MUSIC, "sing" );
	return;
}

CMDF( do_ask )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_ASK, "ask" );
	return;
}

CMDF( do_answer )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_ASK, "answer" );
	return;
}

CMDF( do_shout )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_SHOUT, "shout" );
	WAIT_STATE( ch, 12 );
	return;
}



CMDF( do_yell )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_YELL, "yell" );
	return;
}



CMDF( do_immtalk )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
	return;
}


CMDF( do_i103 )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_RP, "rp" );
	return;
}

CMDF( do_i104 )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_NO_OOC ) )
	{
		send_to_char( "Sorry, you can't use an OOC Channel.\r\n", ch );
		return;
	}

	talk_channel( ch, argument, CHANNEL_i104, "atg" );
	return;
}

CMDF( do_i105 )
{
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	talk_channel( ch, argument, CHANNEL_QUOTE, "quote" );
	return;
}


/*void do_avtalk( CHAR_DATA *ch, char *argument )
{
	if (NOT_AUTHED(ch))
	{
	  send_to_char("Huh?\r\n", ch);
	  return;
	}
	talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_AVTALK, "avtalk" );
	return;
}*/


CMDF( do_say )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *vch;
	EXT_BV actflags;
	int length;

	if( argument[0] == '\0' )
	{
		send_to_char( "Say what?\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	actflags = ch->act;
	if( IS_NPC( ch ) )
		xREMOVE_BIT( ch->act, ACT_SECRETIVE );
	for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		//      char *sbuf = argument;

		if( vch == ch )
			continue;

		/*
		 * Check to see if character is ignoring speaker
		 */
		if( is_ignoring( vch, ch ) )
		{
			/*
			 * continue unless speaker is an immortal
			 */
			if( !IS_IMMORTAL( ch ) || vch->top_level > ch->top_level )
				continue;
			else
			{
				set_char_color( AT_PLAIN, vch );
				ch_printf( vch, "You attempt to ignore %s, but are unable to do so.\r\n", ch->name );
			}
		}

	}
	/*    MOBtrigger = false;
		act( AT_SAY, "&C$n says '&c$T&C'", ch, NULL, argument, TO_ROOM );*/
	ch->act = actflags;
	MOBtrigger = false;
	length = strlen( argument );

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_CYBER ) )
	{
		sprintf( buf, "&R%s inputs '&r%s&R' to the screen.", ch->pcdata->avatar, argument );
		act( AT_SAY, buf, ch, NULL, NULL, TO_ROOM );
		sprintf( buf, "&RYou input '&r%s&R' to the screen.", argument );
		act( AT_SAY, buf, ch, NULL, NULL, TO_CHAR );
		return;
	}

	/*
		for (d = first_descriptor; d != NULL; d = d->next)
		{
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if (!can_see(ch,wch))
			continue;

		if (!str_cmp(arg1,wch->name) && !found)
		{
			if (IS_NPC(wch))
			continue;

			found = true;

			send_to_char( "&RYou hear.\r\n",ch);

				ch_printf( wch, "&R%s says, '&r%s&R'.\r\n", ch->name, argument );
		}
		}
	*/

	if( argument[length - 1] == '!' )
	{
		if( argument[length - 2] != '!' )
		{
			act( AT_SAY, "&C$n exclaims '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
			act( AT_SAY, "&CYou exclaim '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
		}
		else
		{
			act( AT_SAY, "&C$n screams '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
			act( AT_SAY, "&CYou scream '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
		}
	}
	else if( argument[length - 1] == '?' )
	{
		if( argument[length - 2] == '!' )
		{
			act( AT_SAY, "&C$n boggles '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
			act( AT_SAY, "&CYou boggle '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
		}
		else if( argument[length - 2] != '?' )
		{
			act( AT_SAY, "&C$n asks '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
			act( AT_SAY, "&CYou ask '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
		}
		else
		{
			act( AT_SAY, "&C$n demands '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
			act( AT_SAY, "&CYou demand '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
		}
	}
	else if( argument[length - 1] == '.' && argument[length - 2] == '.' && argument[length - 3] == '.' )
	{
		act( AT_SAY, "&C$n mutters '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou mutter '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == ')' && ( argument[length - 2] == '=' || argument[length - 2] == ':' ) )
	{
		act( AT_SAY, "&C$n chuckles '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou chuckle '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == '^' && ( argument[length - 2] == '=' || argument[length - 2] == '^' ) )
	{
		act( AT_SAY, "&C$n beams '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou beam '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == '(' && ( argument[length - 2] == '=' || argument[length - 2] == ':' ) )
	{
		act( AT_SAY, "&C$n sulks '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou sulk '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == 'P' && ( argument[length - 2] == '=' || argument[length - 2] == ':' ) )
	{
		act( AT_SAY, "&C$n smirks '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou smirk '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == ')' && argument[length - 2] == ';' )
	{
		act( AT_SAY, "&C$n leers '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou leer '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else if( argument[length - 1] == 'O' && ( argument[length - 2] == '=' || argument[length - 2] == ':' ) )
	{
		act( AT_SAY, "&C$n hum '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou hum '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}
	else
	{
		act( AT_SAY, "&C$n says '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );
		act( AT_SAY, "&CYou say '&c$T&C'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "&C%s:&c %s", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
		append_to_file( LOG_FILE, buf );
	}
	mprog_speech_trigger( argument, ch );
	if( char_died( ch ) )
		return;
	oprog_speech_trigger( argument, ch );
	if( char_died( ch ) )
		return;
	rprog_speech_trigger( argument, ch );
	return;
}

CMDF( do_replay )
{
	if( IS_NPC( ch ) )
	{
		send_to_char( "You can't replay.\r\n", ch );
		return;
	}

	if( ch->pcdata->tells == 0 )
	{
		send_to_char( "You have no waiting tells.\r\n", ch );
		return;
	}

	send_to_char( ch->pcdata->tellbuf, ch );
	DISPOSE( ch->pcdata->tellbuf );
	ch->pcdata->tellbuf = str_dup( "" );
	ch->pcdata->tells = 0;
}

CMDF( do_tell )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	CHAR_DATA *switched_victim;
	bool ch_comlink;
	bool victim_comlink;
	OBJ_DATA *obj;

	switched_victim = NULL;

	if( IS_SET( ch->deaf, CHANNEL_TELLS ) && !IS_IMMORTAL( ch ) )
	{
		act( AT_PLAIN, "You have tells turned off... try chan +tells first", ch, NULL, NULL, TO_CHAR );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_CYBER ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) || xIS_SET( ch->act, PLR_NO_TELL ) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Tell whom what?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL
		|| ( IS_NPC( victim ) && victim->in_room != ch->in_room )
		|| ( !NOT_AUTHED( ch ) && NOT_AUTHED( victim ) && !IS_IMMORTAL( ch ) ) || ( !can_see( ch, victim ) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( ch == victim )
	{
		send_to_char( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if( victim->in_room == ch->in_room )
	{
		send_to_char( "It would be safer to whisper to that person.\r\n", ch );
		return;
	}

	if( IS_SET( victim->pcdata->flags, PCFLAG_DND ) )
	{
		send_to_char( "Sorry, you can't send tells to a person in DND mode.", ch );
		return;
	}

	if( victim->in_room != ch->in_room && !IS_IMMORTAL( ch ) )
	{
		ch_comlink = false;
		victim_comlink = false;

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

		if( IS_IMMORTAL( victim ) )
			victim_comlink = true;

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

	}

	if( NOT_AUTHED( ch ) && !NOT_AUTHED( victim ) && !IS_IMMORTAL( victim ) )
	{
		send_to_char( "They can't hear you because you are not authorized.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && ( victim->switched )
		&& ( get_trust( ch ) > LEVEL_AVATAR )
		&& !xIS_SET( victim->switched->act, ACT_POLYMORPHED ) && !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}

	else if( !IS_NPC( victim ) && ( victim->switched )
		&& ( xIS_SET( victim->switched->act, ACT_POLYMORPHED ) || IS_AFFECTED( victim->switched, AFF_POSSESS ) ) )
		switched_victim = victim->switched;

	else if( !IS_NPC( victim ) && ( !victim->desc ) )
	{
		//      send_to_char( "That player is link-dead.\r\n", ch );

		send_to_char( "That player is link-dead, but will recieve your messege when they get back.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );

		return;
	}

	if( !IS_NPC( victim ) && victim->position == POS_FIGHTING )
	{
		send_to_char( "That player is fighting, but will recieve your text messege upon return.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );
		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->act, PLR_AFK ) ) )
	{
		//      send_to_char( "That player is AFK.\r\n", ch );


		send_to_char( "That player is AFK, but will get your tell when they return.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );

		return;
	}

	if( IS_SET( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
	{
		act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}


	if( !IS_NPC( victim ) && ( xIS_SET( victim->act, PLR_SILENCE ) || xIS_SET( victim->act, PLR_NO_TELL ) ) )
	{
		send_to_char( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->in_room->room_flags, ROOM_CYBER ) ) )
	{
		send_to_char( "They can't hear you right now.\r\n", ch );
		return;
	}

	if( ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
		|| ( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
	{
		act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if( victim->desc /* make sure desc exists first  -Thoric */
		&& victim->desc->connected == CON_EDITING && get_trust( ch ) < LEVEL_LIAISON )
	{
		act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * Check to see if target of tell is ignoring the sender
	 */
	if( is_ignoring( victim, ch ) )
	{
		/*
		 * If the sender is an imm then they cannot be ignored
		 */
		if( !IS_IMMORTAL( ch ) || victim->top_level > ch->top_level )
		{
			/*
			 * Drop the command into oblivion, why tell the other guy you're ignoring them?
			 */
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
		else
		{
			set_char_color( AT_PLAIN, victim );
			ch_printf( victim, "You attempt to ignore %s, but are unable to do so.\r\n", ch->name );
		}
	}

	if( switched_victim )
		victim = switched_victim;

	act( AT_TELL, "You tell $N '$t'", ch, drunk_speech( argument, ch ), victim, TO_CHAR );
	position = victim->position;
	victim->position = POS_STANDING;
	act( AT_TELL, "$n tells you '$t'", ch, drunk_speech( argument, ch ), victim, TO_VICT );
	victim->position = position;
	victim->reply = ch;
	if( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s: %s (tell to) %s.",
			IS_NPC( ch ) ? ch->short_descr : ch->name, argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
		append_to_file( LOG_FILE, buf );
	}
	mprog_speech_trigger( argument, ch );
	return;
}

CMDF( do_reply )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int position;


	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_CYBER ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( ( victim = ch->reply ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->act, PLR_NO_TELL ) ) )
	{
		send_to_char( "Your message didn't get through.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && ( victim->switched ) && can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}
	else if( !IS_NPC( victim ) && ( !victim->desc ) )
	{
		//      send_to_char( "That player is link-dead.\r\n", ch );

		send_to_char( "That player is link-dead. But will recieve your text messege upon return.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );

		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->act, PLR_AFK ) ) )
	{
		//      send_to_char( "That player is AFK.\r\n", ch );

		send_to_char( "That player is AFK, but will get your tell when they return.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );

		return;
	}

	if( victim->position == POS_FIGHTING )
	{
		send_to_char( "That player is fighting, but will recieve your text messege upon return.\r\n", ch );
		sprintf( buf, "%s&B%s tells you '%s'\n", victim->pcdata->tellbuf ? victim->pcdata->tellbuf : "", ch->name, argument );
		victim->pcdata->tells++;
		victim->pcdata->tellbuf = str_dup( buf );
		return;
	}

	if( IS_SET( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
	{
		act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
		|| ( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
	{
		act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->in_room->room_flags, ROOM_CYBER ) ) )
	{
		send_to_char( "They can't hear you right now.\r\n", ch );
		return;
	}


	/*
	 * Check to see if the receiver is ignoring the sender
	 */
	if( is_ignoring( victim, ch ) )
	{
		/*
		 * If the sender is an imm they cannot be ignored
		 */
		if( !IS_IMMORTAL( ch ) || victim->top_level > ch->top_level )
		{
			/*
			 * Drop into oblivion - let annoyances eat static
			 */
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
		else
		{
			set_char_color( AT_PLAIN, victim );
			ch_printf( victim, "You attempt to ignore %s, but are unable to do so.\r\n", ch->name );
		}
	}

	act( AT_TELL, "You tell $N '$t'", ch, drunk_speech( argument, ch ), victim, TO_CHAR );
	position = victim->position;
	victim->position = POS_STANDING;
	act( AT_TELL, "$n tells you '$t'", ch, drunk_speech( argument, ch ), victim, TO_VICT );
	victim->position = position;
	victim->reply = ch;
	if( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s: %s (reply to) %s.",
			IS_NPC( ch ) ? ch->short_descr : ch->name, argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
		append_to_file( LOG_FILE, buf );
	}
	return;
}

CMDF( do_emote )
{
	char buf[MAX_STRING_LENGTH];
	const char *plast;
	EXT_BV act_Flags;

	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
	{
		send_to_char( "You can't show your emotions.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Emote what?\r\n", ch );
		return;
	}

	act_Flags = ch->act;
	if( IS_NPC( ch ) )
		xREMOVE_BIT( ch->act, ACT_SECRETIVE );
	for( plast = argument; *plast != '\0'; plast++ )
		;

	mudstrlcpy( buf, argument, MAX_STRING_LENGTH );
	if( isalpha( plast[-1] ) )
		mudstrlcat( buf, ".", MAX_STRING_LENGTH );

	MOBtrigger = false;
	act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
	MOBtrigger = false;
	act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );
	ch->act = act_Flags;
	if( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s %s (emote)", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
		append_to_file( LOG_FILE, buf );
	}
	if( !IS_NPC( ch ) )
	{
		sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
		append_to_file( BUG_FILE, buf );
	}
	return;
}


CMDF( do_bug )
{
	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "&CWe encourage players to report bugs! Please use this command to report bugs.\r\n", ch );
		send_to_char( "&CSyntax&B: &cbug &B<&Wdescription of bug&B>&D\r\n", ch );
		return;
	}
	append_file( ch, BUG_FILE, argument );
	send_to_char( "Ok.  Thanks.\r\n", ch );
	return;
}


CMDF( do_ide )
{
	send_to_char( "If you want to send an idea, type 'idea <message>'.\r\n", ch );
	send_to_char( "If you want to identify an object and have the identify spell,\r\n", ch );
	send_to_char( "Type 'cast identify <object>'.\r\n", ch );
	return;
}

/*
void do_idea( CHAR_DATA *ch, char *argument )
{
	append_file( ch, IDEA_FILE, argument );
	send_to_char( "Ok.  Thanks.\r\n", ch );
	return;
}
*/


CMDF( do_typo )
{
	set_char_color( AT_PLAIN, ch );
	if( argument[0] == '\0' )
	{
		send_to_char( "\n\rUsage:  'typo <message>'  (your location is automatically recorded)\r\n", ch );
		if( get_trust( ch ) >= LEVEL_LIAISON )
			send_to_char( "Usage:  'typo list' or 'typo clear now'\r\n", ch );
		return;
	}
	if( !str_cmp( argument, "clear now" ) && get_trust( ch ) >= LEVEL_BUILDER )
	{
		FILE *fp = FileOpen( TYPO_FILE, "w" );
		if( fp )
			FileClose( fp );
		send_to_char( "Typo file cleared.\r\n", ch );
		return;
	}
	if( !str_cmp( argument, "list" ) )
	{
		send_to_char( "\r\n VNUM \r\n.......\r\n", ch );
		show_file( ch, TYPO_FILE );
	}
	else
	{
		append_file( ch, TYPO_FILE, argument );
		send_to_char( "Thanks, your typo notice has been recorded.\r\n", ch );
	}
	return;
}



CMDF( do_rent )
{
	set_char_color( AT_WHITE, ch );
	send_to_char( "There is no rent here.  Just save and quit.\r\n", ch );
	return;
}



CMDF( do_qui )
{
	set_char_color( AT_RED, ch );
	send_to_char( "If you want to QUIT, you have to spell it out.\r\n", ch );
	return;
}

#define MAX_LOGOFF 3

struct logoff_type
{
	char *text;
	//    char *    by;
};


/*
 * The Quotes - insert yours in, and increase MAX_LOGOFF */
 /*
 const struct logoff_type logoff_table [MAX_LOGOFF] =
 {
	 { "Tell Your Local Imm To Make More Quotes" },
	 { "&GBuy Buy, have a nice time in the real world!\r\n&CLesson&c: &BA Synonym is a word you use if you can't spell the other one.&w" },
	 { "Tell Your Local Imm To Make More Quotes" }
 };

 void do_logoff( CHAR_DATA *ch )
 {
	 char buf[MAX_STRING_LENGTH];
	 int number;

	 number = number_range( 1, MAX_LOGOFF);

	 sprintf ( buf, "\r\n%s\r\n",
		logoff_table[number].text);
	 send_to_char ( buf, ch );
	 return;
 }
 */

void do_logoff( CHAR_DATA *ch )
{
	const char *t;

	switch( number_range( 1, 11 ) )
	{
	default:
	case 1:
		t = "&GBuy Buy, have a nice time in the real world!\r\n&CLesson&c: &BA Synonym is a word you use if you can't spell the other one.&w\r\n";
		break;
	case 2:
		t = "&CThought to Ponder&c:\r\n&BIs Santa so jolly because he knows where all the bad girls live?\r\n";
		break;
	case 3:
		t = "&CFor hopeless people In-Love&c:\r\n&BI've learned that you cannot make someone love you. All you can\n\rdo is stalk them and hope they panic and give in.\r\n";
		break;
	case 4:
		t = "&CTo help females understand Males&c:\r\n&BMales have a brain and a penis, unfortunately we only\n\rhave enough blood to use one at a time!\r\n";
		break;
	case 5:
		t = "&CInsult For Savage&c:\r\n&BOver one million sperm and YOU were the fastest!?\r\n";
		break;
	case 6:
		t = "&GMy girlfriend broke up with me and sent me pictures of her\n\rand her new boyfriend in bed together.\r\n&CSolution&c:&B I sent them to her Dad!\r\n";
		break;
	case 7:
		t = "&GWhy is it that bullets ricochet off of Superman's chest,\n\rbut he ducks when the gun is thrown at him?\r\n";
		break;
	case 8:
		t = "&GWho's cruel idea was it to put the letter 'S' in the word 'Lisp'!?\r\n";
		break;
	case 9:
		t = "&GThe Chico, California, City Council enacted a ban on nuclear weapons, setting\n\ra $500 fine for anyone detonating one within city limits.\r\n";
		break;
	case 10:
		t = "&GError: Keyboard not attached. Press F1 to continue.\r\n";
		break;
	case 11:
		t = "&GYour mouse has moved. Please wait while Windows restarts for\n\rthe change to take effect.\r\n";
		break;
	}
	send_to_char( "\r\n&P===========================================\r\n", ch );
	send_to_char( t, ch );
	send_to_char( "&P===========================================&w\r\n", ch );
	send_to_char( "&RCome back &O-&Y=&zSoon!&Y=&O-&w\r\n", ch );
	return;
}

CMDF( do_quit )
{
	/*
	 * OBJ_DATA *obj;
	 *//*
	 * Unused
	 */
	char buf[MAX_STRING_LENGTH];
	int x, y;
	int level;
	CHAR_DATA *vch;

	if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_POLYMORPHED ) )
	{
		send_to_char( "You can't quit while polymorphed.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) )
		return;

	if( ch->position == POS_FIGHTING )
	{
		set_char_color( AT_RED, ch );
		send_to_char( "No way! You are fighting.\r\n", ch );
		return;
	}

	if( ch->position < POS_STUNNED )
	{
		set_char_color( AT_BLOOD, ch );
		send_to_char( "You're not DEAD yet.\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) )
	{
		send_to_char( "You can't do that right now.\r\n", ch );
		return;
	}

	if( get_timer( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL( ch ) )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			set_char_color( AT_RED, ch );
			send_to_char( "You need to cool down before you can leave!\r\n", ch );
			return;
		}
	}

	if( auction_list != NULL && ( auction_list->high_bidder == ch || auction_list->owner == ch ) )
	{
		send_to_char( "You still have a stake in the auction!\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) && ch->in_room && !xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) && !NOT_AUTHED( ch ) )
	{
		send_to_char( "You can't quit here, you should search for a hotel.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( ch, AFF_STALK ) || ch->hunting )
	{
		if( ch->hunting )
		{
			stop_hunting( ch );
		}
		affect_strip( ch, gsn_track );
		REMOVE_BIT( ch->affected_by, AFF_STALK );
	}

	/*
	 * Free note that might be there somehow
	 */
	if( ch->pcdata->in_progress )
		free_note( ch->pcdata->in_progress );


	drop_artifacts( ch, ch->last_carrying );
	save_artifacts( );
	save_char_obj( ch );
	save_home( ch );
	do_logoff( ch );

	if( !IS_IMMORTAL( ch ) )
	{
		for( vch = first_char; vch; vch = vch->next )
		{
			if( IS_NPC( vch ) || vch == ch )
				continue;

			if( ch->pcdata->exit && ch->pcdata->exit[0] != '\0' )
			{
				sprintf( buf, "&C(&zGW&c:&zOM&C)&w %s", ch->pcdata->exit );
				if( !IS_SET( vch->deaf, CHANNEL_INFO ) )
					act( AT_GOSSIP, buf, ch, NULL, vch, TO_VICT );
			}
			else
			{
				sprintf( buf,
					"&C(&zGW&c:&zOM&C)&w &B%s &GD&ge&cc&Ci&cd&ge&Gd &YT&Oo &RR&re&pjo&ri&Rn &CT&ch&Ce &zR&Wea&zl &PW&po&Pr&pl&Pd&z.",
					ch->name );
				if( !IS_SET( vch->deaf, CHANNEL_INFO ) )
					act( AT_GOSSIP, buf, ch, NULL, vch, TO_VICT );
			}
		}
	}

	act( AT_BYE, "$n has reentered the real world.", ch, NULL, NULL, TO_ROOM );
	sprintf( log_buf, "%s has quit. (Room %d)", ch->name, ch->in_room->vnum );
	quitting_char = ch;
	save_char_obj( ch );
	save_home( ch );

	level = get_trust( ch );
	/*
	 * After extract_char the ch is no longer valid!
	 */
	extract_char( ch, true );
	for( x = 0; x < MAX_WEAR; x++ )
		for( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;

	/*
	 * don't show who's logging off to leaving player
	 */
	 /*
		 to_channel( log_buf, CHANNEL_MONITOR, "Monitor", level );
	 */
	log_string_plus( log_buf, LOG_COMM, level );
	return;
}

void drop_artifacts( CHAR_DATA *ch, OBJ_DATA *obj )
{
	/*
	 * Expecting a ch->last_carrying or an obj->last_content
	 * ( We go BACKWARDS through the lists!)
	 */

	if( !obj )
		return;

	while( obj )
	{
		if( obj->last_content )
			drop_artifacts( ch, obj->last_content );

		if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		{
			OBJ_DATA *tobj;

			tobj = obj;
			obj = obj->prev_content;

			if( tobj->in_obj )
				obj_from_obj( tobj );
			if( tobj->carried_by )
				obj_from_char( tobj );

			obj_to_room( tobj, ch->in_room );
			act( AT_MAGIC, "$p slips out onto the ground!", ch, tobj, NULL, TO_CHAR );
			act( AT_MAGIC, "$p slips out of $n's hands, onto the ground!", ch, tobj, NULL, TO_ROOM );
		}
		else
			obj = obj->prev_content;
	}
}

void send_rip_screen( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num = 0;
	char BUFF[MAX_STRING_LENGTH * 2];

	if( ( rpfile = FileOpen( RIPSCREEN_FILE, "r" ) ) != NULL )
	{
		while( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
			num++;
		FileClose( rpfile );
		BUFF[num] = 0;
		write_to_buffer( ch->desc, BUFF, num );
	}
}

void send_rip_title( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num = 0;
	char BUFF[MAX_STRING_LENGTH * 2];

	if( ( rpfile = FileOpen( RIPTITLE_FILE, "r" ) ) != NULL )
	{
		while( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
			num++;
		FileClose( rpfile );
		BUFF[num] = 0;
		write_to_buffer( ch->desc, BUFF, num );
	}
}

void send_ansi_title( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num = 0;
	char BUFF[MAX_STRING_LENGTH * 2];

	if( ( rpfile = FileOpen( ANSITITLE_FILE, "r" ) ) != NULL )
	{
		while( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
			num++;
		FileClose( rpfile );
		BUFF[num] = 0;
		write_to_buffer( ch->desc, BUFF, num );
	}
}

void send_ascii_title( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num = 0;
	char BUFF[MAX_STRING_LENGTH];

	if( ( rpfile = FileOpen( ASCTITLE_FILE, "r" ) ) != NULL )
	{
		while( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
			num++;
		FileClose( rpfile );
		BUFF[num] = 0;
		write_to_buffer( ch->desc, BUFF, num );
	}
}


CMDF( do_rip )
{
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Rip ON or OFF?\r\n", ch );
		return;
	}
	if( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
	{
		send_rip_screen( ch );
		xSET_BIT( ch->act, PLR_ANSI );
		return;
	}

	if( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
	{
		send_to_char( "!|*\n\rRIP now off...\r\n", ch );
		return;
	}
}

CMDF( do_ansi )
{
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "ANSI ON or OFF?\r\n", ch );
		return;
	}
	if( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
	{
		xSET_BIT( ch->act, PLR_ANSI );
		set_char_color( AT_WHITE + AT_BLINK, ch );
		send_to_char( "ANSI ON!!!\r\n", ch );
		return;
	}

	if( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
	{
		xREMOVE_BIT( ch->act, PLR_ANSI );
		send_to_char( "Okay... ANSI support is now off\r\n", ch );
		return;
	}
}

CMDF( do_sound )
{
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "SOUND ON or OFF?\r\n", ch );
		return;
	}
	if( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
	{
		xSET_BIT( ch->act, PLR_SOUND );
		set_char_color( AT_WHITE + AT_BLINK, ch );
		send_to_char( "SOUND ON!!!\r\n", ch );
		send_to_char( "!!SOUND(hopeknow)", ch );
		return;
	}

	if( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
	{
		xREMOVE_BIT( ch->act, PLR_SOUND );
		send_to_char( "Okay... SOUND support is now off\r\n", ch );
		return;
	}
}


CMDF( do_save )
{
	if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_POLYMORPHED ) )
	{
		send_to_char( "You can't save while polymorphed.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) )
		return;
	/*
		if ( !xIS_SET( ch->affected_by, race_table[ch->race].affected ) )
		xSET_BIT( ch->affected_by, race_table[ch->race].affected );
		if ( !xIS_SET( ch->resistant, race_table[ch->race].resist ) )
		xSET_BIT( ch->resistant, race_table[ch->race].resist );
		if ( !xIS_SET( ch->susceptible, race_table[ch->race].suscept ) )
		xSET_BIT( ch->susceptible, race_table[ch->race].suscept );
	*/
	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "You can't save until after you've graduated from the acadamey.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ch->top_level < 1001 && ch->pcdata->bank > 1000000000 )
	{
		ch->pcdata->bank = 100000000;
	}
	//    update_aris(ch);   
	save_char_obj( ch );
	save_home( ch );
	saving_char = NULL;
	send_to_char( "Saved.\r\n", ch );
	return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *tmp;

	for( tmp = victim; tmp; tmp = tmp->master )
		if( tmp == ch )
			return true;
	return false;
}


CMDF( do_follow )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Follow whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master )
	{
		act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
		return;
	}

	if( victim == ch )
	{
		if( !ch->master )
		{
			send_to_char( "You already follow yourself.\r\n", ch );
			return;
		}
		stop_follower( ch );
		return;
	}

	if( circle_follow( ch, victim ) )
	{
		send_to_char( "Following in loops is not allowed... sorry.\r\n", ch );
		return;
	}

	if( ch->master )
		stop_follower( ch );

	add_follower( ch, victim );
	return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
	if( ch->master )
	{
		bug( "Add_follower: non-null master.", 0 );
		return;
	}

	ch->master = master;
	ch->leader = NULL;

	if( can_see( master, ch ) )
		act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

	act( AT_ACTION, "You now follow $N.", ch, NULL, master, TO_CHAR );

	return;
}



void stop_follower( CHAR_DATA *ch )
{
	if( !ch->master )
	{
		bug( "Stop_follower: null master.", 0 );
		return;
	}

	if( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		REMOVE_BIT( ch->affected_by, AFF_CHARM );
		affect_strip( ch, gsn_charm_person );
	}

	if( can_see( ch->master, ch ) )
		if( !( !IS_NPC( ch->master ) && IS_IMMORTAL( ch ) && !IS_IMMORTAL( ch->master ) ) )
			act( AT_ACTION, "$n stops following you.", ch, NULL, ch->master, TO_VICT );
	act( AT_ACTION, "You stop following $N.", ch, NULL, ch->master, TO_CHAR );

	ch->master = NULL;
	ch->leader = NULL;
	return;
}



void die_follower( CHAR_DATA *ch )
{
	CHAR_DATA *fch;

	if( ch->master )
		stop_follower( ch );

	ch->leader = NULL;

	for( fch = first_char; fch; fch = fch->next )
	{
		if( fch->master == ch )
			stop_follower( fch );
		if( fch->leader == ch )
			fch->leader = fch;
	}
	return;
}



CMDF( do_order )
{
	char arg[MAX_INPUT_LENGTH];
	char argbuf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	mudstrlcpy( argbuf, argument, MAX_INPUT_LENGTH );
	argument = one_argument( argument, arg );

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Order whom to do what?\r\n", ch );
		return;
	}

	if( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You feel like taking, not giving, orders.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "all" ) )
	{
		fAll = true;
		victim = NULL;
	}
	else
	{
		fAll = false;
		if( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if( victim == ch )
		{
			send_to_char( "Aye aye, right away!\r\n", ch );
			return;
		}

		if( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
		{
			send_to_char( "Do it yourself!\r\n", ch );
			return;
		}
	}

	found = false;
	for( och = ch->in_room->first_person; och; och = och_next )
	{
		och_next = och->next_in_room;

		if( IS_AFFECTED( och, AFF_CHARM ) && och->master == ch && ( fAll || och == victim ) )
		{
			found = true;
			act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
			interpret( och, argument );
		}
	}

	if( found )
	{
		sprintf( log_buf, "%s: order %s.", ch->name, argbuf );
		log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
		send_to_char( "Ok.\r\n", ch );
		WAIT_STATE( ch, 12 );
	}
	else
		send_to_char( "You have no followers here.\r\n", ch );
	return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

CMDF( do_group )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		CHAR_DATA *gch;
		CHAR_DATA *leader;

		leader = ch->leader ? ch->leader : ch;
		set_char_color( AT_GREEN, ch );
		ch_printf( ch, "%s's group:\r\n", PERS( leader, ch ) );

		/* Changed so that no info revealed on possess */
		for( gch = first_char; gch; gch = gch->next )
		{
			if( is_same_group( gch, ch ) )
			{
				set_char_color( AT_DGREEN, ch );
				if( IS_AFFECTED( gch, AFF_POSSESS ) )
					ch_printf( ch,
						"[%2d %s] %-16s %4s/%4s hp %4s/%4s mv %5s xp\r\n",
						gch->top_level,
						IS_NPC( gch ) ? "Mob" : race_table[gch->race].race_name,
						capitalize( PERS( gch, ch ) ), "????", "????", "????", "????", "?????" );

				else
					ch_printf( ch,
						"[%2d %s] %-16s %4d/%4d hp %4d/%4d mv\r\n",
						gch->top_level,
						IS_NPC( gch ) ? "Mob" : race_table[gch->race].race_name,
						capitalize( PERS( gch, ch ) ), gch->hit, gch->max_hit, gch->move, gch->max_move );
			}
		}
		return;
	}

	if( !strcmp( arg, "disband" ) )
	{
		CHAR_DATA *gch;
		int count = 0;

		if( ch->leader || ch->master )
		{
			send_to_char( "You cannot disband a group if you're following someone.\r\n", ch );
			return;
		}

		for( gch = first_char; gch; gch = gch->next )
		{
			if( is_same_group( ch, gch ) && ( ch != gch ) )
			{
				gch->leader = NULL;
				gch->master = NULL;
				count++;
				send_to_char( "Your group is disbanded.\r\n", gch );
			}
		}

		if( count == 0 )
			send_to_char( "You have no group members to disband.\r\n", ch );
		else
			send_to_char( "You disband your group.\r\n", ch );

		return;
	}

	/*    if ( !strcmp( arg, "all" ) )
		{
		CHAR_DATA *rch;
		int count = 0;

			for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
		{
			   if ( ch != rch
			   &&   !IS_NPC( rch )
		   &&   rch->master == ch
		   &&   !ch->master
		   &&   !ch->leader
			   &&   !is_same_group( rch, ch )
			  )
		   {
			rch->leader = ch;
			count++;
		   }
		}

		if ( count == 0 )
		  send_to_char( "You have no eligible group members.\r\n", ch );
		else
		{
			   act( AT_ACTION, "$n groups $s followers.", ch, NULL, victim, TO_ROOM );
		   send_to_char( "You group your followers.\r\n", ch );
		}
		return;
		}
	*/
	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( ch->master || ( ch->leader && ch->leader != ch ) )
	{
		send_to_char( "But you are following someone else!\r\n", ch );
		return;
	}

	if( victim->master != ch && ch != victim )
	{
		act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( victim->skill_level[COMBAT_ABILITY] - ch->skill_level[COMBAT_ABILITY] > 300 )
	{
		send_to_char( "They are to high of a level for your group.\r\n", ch );
		return;
	}

	if( victim->skill_level[COMBAT_ABILITY] - ch->skill_level[COMBAT_ABILITY] < -300 )
	{
		send_to_char( "They are to low of a level for your group.\r\n", ch );
		return;
	}

	if( is_same_group( victim, ch ) && ch != victim )
	{
		victim->leader = NULL;
		act( AT_ACTION, "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "$n removes you from $s group.", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( !IS_NPC( victim ) && IS_IMMORTAL( victim ) )
	{
		send_to_char( "Sorry, can't group with Immortals.\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( victim ) && IS_IMMORTAL( ch ) )
	{
		send_to_char( "&RYou can't group with mortals!\r\n", ch );
		return;
	}

	victim->leader = ch;
	act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR );
	return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
CMDF( do_split )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount;
	int share;
	int extra;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Split how much?\r\n", ch );
		return;
	}

	amount = atoi( arg );

	if( amount < 0 )
	{
		send_to_char( "Your group wouldn't like that.\r\n", ch );
		return;
	}

	if( amount == 0 )
	{
		send_to_char( "You hand out zero dollars, but no one notices.\r\n", ch );
		return;
	}

	if( ch->gold < amount )
	{
		send_to_char( "You don't have that many dollars.\r\n", ch );
		return;
	}

	members = 0;
	for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		if( is_same_group( gch, ch ) )
			members++;
	}


	if( ( xIS_SET( ch->act, PLR_AUTOGOLD ) ) && ( members < 2 ) )
		return;

	if( members < 2 )
	{
		send_to_char( "Just keep it all.\r\n", ch );
		return;
	}

	share = amount / members;
	extra = amount % members;

	if( share == 0 )
	{
		send_to_char( "Don't even bother, cheapskate.\r\n", ch );
		return;
	}

	ch->gold -= amount;
	ch->gold += share + extra;

	set_char_color( AT_GOLD, ch );
	ch_printf( ch, "You split %d dollars.  Your share is %d dollars.\r\n", amount, share + extra );

	sprintf( buf, "$n splits %d dollars.  Your share is %d dollars.", amount, share );

	for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		if( gch != ch && is_same_group( gch, ch ) )
		{
			act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
			gch->gold += share;
		}
	}
	return;
}



CMDF( do_gtell )
{
	CHAR_DATA *gch;

	if( argument[0] == '\0' )
	{
		send_to_char( "Tell your group what?\r\n", ch );
		return;
	}

	if( xIS_SET( ch->act, PLR_NO_TELL ) )
	{
		send_to_char( "Your message didn't get through!\r\n", ch );
		return;
	}

	/*
	 * Note use of send_to_char, so gtell works on sleepers.
	 */
	 /*    sprintf( buf, "%s tells the group '%s'.\r\n", ch->name, argument );*/
	for( gch = first_char; gch; gch = gch->next )
	{
		if( is_same_group( gch, ch ) )
		{
			set_char_color( AT_GTELL, gch );
			ch_printf( gch, "%s tells the group '%s'.\r\n", ch->name, argument );
		}
	}

	return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
	if( ach->leader )
		ach = ach->leader;
	if( bch->leader )
		bch = bch->leader;
	return ach == bch;
}

void info_chan( const char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;

	sprintf( buf, "&C(&zGW&c:&zOM&C)&w %s", argument );  /* last %s to reset color */

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */
		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_INFO )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}

void rank_chan( const char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;

	sprintf( buf, "&Y[&RR&ra&pn&Pk&pi&rn&Rg&Y]&w %s", argument );    /* last %s to reset color */

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */
		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_RANK )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}

void talk_arena( const char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;

	sprintf( buf, "&P&p-&P=&RA&rr&Re&rn&Ra&P=&p- %s", argument );    /* last %s to reset color */

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */
		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_ARENA )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction( const char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;
	
	sprintf( buf, "&CAuction&c:&C %s", argument );

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */

		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_AUCTION )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}

CMDF( do_wartalk )
{
	char buf[MAX_STRING_LENGTH];
	const char *plast;

	if( NOT_AUTHED( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) )
		return;

	if( IS_SET( ch->pcdata->flags, PCFLAG_NO_OOC ) )
	{
		send_to_char( "Sorry, you can't use OOC.\r\n", ch );
		return;
	}

	for( plast = argument; *plast != '\0'; plast++ )
		;

	mudstrlcpy( buf, argument, MAX_STRING_LENGTH );
	if( isalpha( plast[-1] ) )
		mudstrlcat( buf, ".", MAX_STRING_LENGTH );

	talk_channel( ch, buf, CHANNEL_WARTALK, "Gmote" );
	return;
}

/*DESCRIPTOR_DATA *d;

if ( argument[0] == '\0' )
{
send_to_char( "Global emote what?\r\n", ch );
return;
}

for ( d = last_descriptor; d; d = d->next )
{
if ( d->connected == CON_PLAYING )
 printf ( "&z[&BGl&bob&Bal &CEm&co&Cte&z]&W: ch->character" );
 printf ( "\r\n d->character" );
}
}
*/

CMDF( do_whisper )
{
	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg1 );
	strcpy( arg2, argument );

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still; you are DEAD.\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}


	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They are not in here.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_CYBER ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( ch == victim )
	{
		send_to_char( "You whisper to yourself....\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && ( xIS_SET( victim->in_room->room_flags, ROOM_CYBER ) ) )
	{
		send_to_char( "They can't hear you right now.\r\n", ch );
		return;
	}

	/*
	 * Check to see if target of tell is ignoring the sender
	 */
	if( is_ignoring( victim, ch ) )
	{
		/*
		 * If the sender is an imm then they cannot be ignored
		 */
		if( !IS_IMMORTAL( ch ) || get_trust( victim ) > get_trust( ch ) )
		{
			return;
		}
		else
		{
			set_char_color( AT_PLAIN, victim );
			ch_printf( victim, "You attempt to ignore %s, but are unable to do so.\r\n", ch->name );
		}
	}

	sprintf( buf, "&WYou whisper, '&z%s&W'\r\n", arg2 );
	send_to_char( buf, ch );
	sprintf( buf, "&W%s whispers, '&z%s&W'\r\n", ch->name, arg2 );
	send_to_char( buf, victim );
	act( AT_SOCIAL, "$n whispers something to $N...", ch, NULL, victim, TO_NOTVICT );
}
