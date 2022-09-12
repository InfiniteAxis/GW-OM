/***************************************************************************
* Automated Freeze Tag Code                                                *
* Markanth : dlmud@dlmud.com                                               *
* Devil's Lament : dlmud.com port 3778                                     *
* Web Page : http://www.dlmud.com                                          *
*                                                                          *
* Provides automated freeze tag games in an area.                          *
* Code orginally done by Nebseni of Clandestine MUD                        *
* <clandestine.mudnet.net:9476>.                                           *
*                                                                          *
* All I ask in return is that you give me credit on your mud somewhere     *
* or email me if you use it.                                               *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

TAG_DATA tag_game;

CMDF( do_red )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) )
		return;

	if( tag_game.status == TAG_OFF )
	{
		send_to_char( "There is no tag game playing.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  red <message>\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) && !IS_SET( ch->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) )
	{
		send_to_char( "You must be a freeze tag player to use this channel.\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->tag_flags, TAG_BLUE ) )
	{
		sprintf( buf, "&z[&RR&rE&RD&z] &R%s&r:&z %s\r\n", ch->name, argument );
		send_to_char( buf, ch );
	}
	else
	{
		sprintf( buf, "&z[&RR&rE&RD&z] &R%s&r:&z %s\r\n", ch->name, argument );
	}

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		if( d->connected == CON_PLAYING && d->character && !IS_NPC( d->character )
			&& IS_SET( d->character->pcdata->tag_flags, TAG_RED ) )
		{
			send_to_char( buf, d->character );
		}
	}
}

CMDF( do_blue )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) )
		return;

	if( tag_game.status == TAG_OFF )
	{
		send_to_char( "There is no tag game playing.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  blue <message>\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) && !IS_SET( ch->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) )
	{
		send_to_char( "You must be a freeze tag player to use this channel.\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->tag_flags, TAG_RED ) )
	{
		sprintf( buf, "&z[&BB&bL&BU&bE&z]&B %s&b:&z %s\r\n", ch->name, argument );
		send_to_char( buf, ch );
	}
	else
	{
		sprintf( buf, "&z[&BB&bL&BU&bE&z] &B%s&b:&z %s\r\n", ch->name, argument );
	}

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		if( d->connected == CON_PLAYING && d->character && !IS_NPC( d->character )
			&& IS_SET( d->character->pcdata->tag_flags, TAG_BLUE ) )
		{
			send_to_char( buf, d->character );
		}
	}
}

void end_tag( void )
{
	DESCRIPTOR_DATA *d;

	tag_game.status = TAG_OFF;
	tag_game.timer = -1;
	tag_game.next = number_range( 30, 50 );
	tag_game.playing = 0;

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		if( d->connected != CON_PLAYING || !d->character || IS_NPC( d->character ) )
			continue;

		if( IS_SET( d->character->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) )
		{
			char_from_room( d->character );
			char_to_room( d->character, get_room_index( d->character->retran ) );
			do_look( d->character, "auto" );
			send_to_char( "Freeze tag has been stopped!\r\n", d->character );
		}
		d->character->pcdata->tag_flags = 0;
	}
}

void start_tag( void )
{
	DESCRIPTOR_DATA *d;
	ROOM_INDEX_DATA *loc;
	int count = 0;
	char buf[MAX_INPUT_LENGTH];

	tag_game.status = TAG_ISPLAY;
	tag_game.timer = 5 * tag_game.playing;
	for( d = first_descriptor; d != NULL; d = d->next )
	{
		if( d->connected != CON_PLAYING || !d->character || IS_NPC( d->character ) )
			continue;

		if( IS_SET( d->character->pcdata->tag_flags, TAG_WAITING ) )
		{
			count++;
			loc = get_room_index( number_range( ROOM_FTAG_MIN_VNUM, ROOM_FTAG_MAX_VNUM ) );
			REMOVE_BIT( d->character->pcdata->tag_flags, TAG_FROZEN );
			REMOVE_BIT( d->character->pcdata->tag_flags, TAG_WAITING );
			SET_BIT( d->character->pcdata->tag_flags, TAG_PLAYING );
			char_from_room( d->character );
			char_to_room( d->character, loc );
			do_look( d->character, "auto" );
		}
	}
	sprintf( buf, "Freeze Tag has started! %d people playing.", count );
	tag_channel( NULL, buf );
}

bool fRed = false;

void check_team_frozen( CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( IS_SET( ch->pcdata->tag_flags, TAG_BLUE ) )
	{
		for( d = first_descriptor; d != NULL; d = d->next )
		{
			if( d->connected == CON_PLAYING
				&& d->character && !IS_NPC( d->character )
				&& IS_SET( d->character->pcdata->tag_flags, TAG_PLAYING )
				&& IS_SET( d->character->pcdata->tag_flags, TAG_BLUE )
				&& !IS_SET( d->character->pcdata->tag_flags, TAG_FROZEN ) )
			{
				return;
			}
		}
		tag_channel( NULL, "The RED team has won FREEZE TAG!!!" );
		end_tag( );
		return;
	}

	if( IS_SET( ch->pcdata->tag_flags, TAG_RED ) )
	{
		for( d = first_descriptor; d != NULL; d = d->next )
		{
			if( d->connected == CON_PLAYING
				&& d->character && !IS_NPC( d->character )
				&& IS_SET( d->character->pcdata->tag_flags, TAG_PLAYING )
				&& IS_SET( d->character->pcdata->tag_flags, TAG_RED )
				&& !IS_SET( d->character->pcdata->tag_flags, TAG_FROZEN ) )
			{
				return;
			}
		}
		tag_channel( NULL, "The BLUE team has won FREEZE TAG!!!" );
		end_tag( );
		return;
	}
}

CMDF( do_ftag )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *tch;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "\r\n&BSyntax&c:  &zftag join\r\n", ch );
		send_to_char( "         &zftag info\r\n", ch );
		send_to_char( "         &zftag who\r\n", ch );
		send_to_char( "         &zftag start\r\n", ch );
		send_to_char( "&cTo communicate use the &z'&Bblue&z'&c and &z'&Rred&z'&c channels.\r\n", ch );
		send_to_char( "&cTo tag someone once the game has started use the &z'&ctag&z'&c comand.\r\n", ch );
		if( IS_IMMORTAL( ch ) )
		{
			send_to_char( "\r\n&BFor Immortals&c:\r\n", ch );
			send_to_char( "&BSyntax&c:  &zftag reset\r\n", ch );
			send_to_char( "         &zftag next\r\n", ch );
			send_to_char( "         &zftag red &B<&cplayer&B>\r\n", ch );
			send_to_char( "         &zftag blue &B<&cplayer&B>\r\n", ch );
		}
		return;
	}

	if( !str_cmp( arg1, "join" ) )
	{
		ROOM_INDEX_DATA *loc;

		if( tag_game.status != TAG_ISWAIT )
		{
			send_to_char( "There is no tag game to join.\r\n", ch );
			return;
		}

		if( ch->in_room == get_room_index( 6 ) )
		{
			send_to_char( "&RAhahaha, Yeah Right!\r\n", ch );
			return;
		}

		if( IS_SET( ch->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) )
		{
			send_to_char( "Your already playing.\r\n", ch );
			return;
		}

		if( ( loc = get_room_index( ROOM_FTAG_WAIT_ROOM ) ) == NULL )
		{
			send_to_char( "The freeze tag arena hasn't been finished yet.\r\n", ch );
			return;
		}
		/*
			while ( victim->first_affect )
			affect_remove( victim, victim->first_affect );
			victim->affected_by	= race_table[victim->race].affected;
		*/
		send_to_char( "You join freeze tag.\r\n", ch );
		do_visible( ch, "" );
		ch->retran = ch->in_room->vnum;
		char_from_room( ch );
		char_to_room( ch, loc );
		tag_game.playing += 1;
		do_look( ch, "auto" );
		if( ( fRed = !fRed ) )
		{
			SET_BIT( ch->pcdata->tag_flags, TAG_WAITING );
			REMOVE_BIT( ch->pcdata->tag_flags, TAG_FROZEN );
			SET_BIT( ch->pcdata->tag_flags, TAG_RED );
			REMOVE_BIT( ch->pcdata->tag_flags, TAG_BLUE );
			send_to_char( "&zYou are on the &RR&rE&RD &zteam!\r\n", ch );
			sprintf( buf, "&R%s&z is on the &RR&rE&RD&z team!", ch->name );
			tag_channel( NULL, buf );
		}
		else
		{
			SET_BIT( ch->pcdata->tag_flags, TAG_WAITING );
			REMOVE_BIT( ch->pcdata->tag_flags, TAG_FROZEN );
			SET_BIT( ch->pcdata->tag_flags, TAG_BLUE );
			REMOVE_BIT( ch->pcdata->tag_flags, TAG_RED );
			send_to_char( "&zYou are on the &BB&bL&BU&bE &zteam!\r\n", ch );
			sprintf( buf, "&B%s &zis on the &BB&bL&BU&bE &zteam!", ch->name );
			tag_channel( NULL, buf );
		}
		SET_BIT( ch->pcdata->tag_flags, TAG_WAITING );
		return;
	}

	if( !str_cmp( arg1, "info" ) )
	{
		if( tag_game.status == TAG_OFF )
		{
			sprintf( buf, "The next game of freeze tag will start in %d minute%s.\r\n",
				tag_game.next, tag_game.next == 1 ? "" : "s" );
			send_to_char( buf, ch );
		}
		else
			send_to_char( "A tag game is currently playing.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "who" ) )
	{
		if( tag_game.status == TAG_OFF )
		{
			send_to_char( "There isn't a tag game going.\r\n", ch );
			return;
		}

		if( tag_game.playing == 0 )
		{
			send_to_char( "No one has joined yet.\r\n", ch );
			return;
		}

		//send_to_char(buf, ch);

		send_to_char( "\r\n   &PN&pa&Pm&pe           &RT&re&ba&Bm     &GS&gt&ca&Ct&Gu&gs\r\n", ch );
		send_to_char
		( "&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=\r\n",
			ch );

		for( tch = first_char; tch; tch = tch->next )

			if( !IS_NPC( tch ) && tch->in_room && xIS_SET( tch->in_room->room_flags, ROOM_TAG ) )
			{
				sprintf( buf, "   &P%-12s   %-4s     %s\r\n",
					tch->name,
					IS_SET( tch->pcdata->tag_flags, TAG_RED ) ? "&RR&rE&RD " : "&BB&bL&BU&bE",
					IS_SET( tch->pcdata->tag_flags, TAG_FROZEN ) ? "&CF&cr&Co&cz&Ce" : "&YM&Oo&Yv&Oi&Yn&Og" );
				send_to_char( buf, ch );
			}
		ch_printf( ch, "\r\n&RP&reople &RP&rlaying&p: &P%d\r\n", tag_game.playing );
		send_to_char
		( "&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=&W+&z=\r\n",
			ch );
		return;
	}

	if( !str_cmp( arg1, "start" ) )
	{
		if( tag_game.status != TAG_OFF )
		{
			send_to_char( "A game has already started.\r\n", ch );
			return;
		}

		if( !IS_IMMORTAL( ch ) && ch->gold < 50000 )
		{
			send_to_char( "Only Immortals can start a tag game.\r\n", ch );
			return;
		}

		tag_channel( NULL, "&YA Freeze Tag Game has started&O! &YType &O'&Rftag join&O' &Yto play&O." );
		tag_game.status = TAG_ISWAIT;
		tag_game.timer = 3;
		tag_game.playing = 0;
		tag_game.next = -1;
		if( !IS_IMMORTAL( ch ) )
			ch->gold -= 50000;
		return;
	}

	if( !IS_IMMORTAL( ch ) )
	{
		do_ftag( ch, "" );
		return;
	}

	if( !str_cmp( arg1, "next" ) )
	{
		tag_game.next = atoi( argument );
		sprintf( buf, "Next freeze tag game will start in %d ticks.\r\n", tag_game.next );
		send_to_char( buf, ch );
		return;
	}
	if( !str_cmp( arg1, "reset" ) )
	{
		end_tag( );
		send_to_char( "All players reset.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg2 );
	if( arg2[0] == '\0' || ( str_cmp( arg1, "red" ) && str_cmp( arg1, "blue" ) ) )
	{
		send_to_char( "Syntax:  ftag red <player>\r\n", ch );
		send_to_char( "         ftag blue <player>\r\n", ch );
		return;
	}

	if( tag_game.status == TAG_ISPLAY )
	{
		send_to_char( "The tag game has already started.\r\n", ch );
		return;
	}
	if( ( victim = get_char_world( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "They can't play.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "red" ) )
	{
		SET_BIT( victim->pcdata->tag_flags, TAG_WAITING );
		REMOVE_BIT( victim->pcdata->tag_flags, TAG_FROZEN );
		SET_BIT( victim->pcdata->tag_flags, TAG_RED );
		REMOVE_BIT( victim->pcdata->tag_flags, TAG_BLUE );
		act( AT_PLAIN, "&zYou are on the &RR&rE&RD&z team!", ch, NULL, victim, TO_VICT );
		act( AT_PLAIN, "&R$N &zis on the &RR&rE&RD&z team!", ch, NULL, victim, TO_NOTVICT );
		act( AT_PLAIN, "&R$N &zis on the &RR&rE&RD&z team!", ch, NULL, victim, TO_CHAR );
	}
	else if( !str_cmp( arg1, "blue" ) )
	{
		SET_BIT( victim->pcdata->tag_flags, TAG_WAITING );
		REMOVE_BIT( victim->pcdata->tag_flags, TAG_FROZEN );
		SET_BIT( victim->pcdata->tag_flags, TAG_BLUE );
		REMOVE_BIT( victim->pcdata->tag_flags, TAG_RED );
		act( AT_PLAIN, "&zYou are on the &BB&bL&BU&bE&z team!", ch, NULL, victim, TO_VICT );
		act( AT_PLAIN, "&B$N&z is on the &BB&bL&BU&bE&z team!", ch, NULL, victim, TO_NOTVICT );
		act( AT_PLAIN, "&B$N&z is on the &BB&bL&BU&bE&z team!", ch, NULL, victim, TO_CHAR );
	}

	return;
}

CMDF( do_tag )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if( IS_NPC( ch ) )
		return;

	if( tag_game.status == TAG_OFF )
	{
		send_to_char( "There is no tag game playing.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( !IS_SET( ch->pcdata->tag_flags, TAG_PLAYING ) )
	{
		send_to_char( "You're not playing freeze tag.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Tag whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You tag yourself.  How amusing.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't tag them.\r\n", ch );
		return;
	}

	if( !IS_SET( victim->pcdata->tag_flags, TAG_PLAYING ) )
	{
		send_to_char( "They're not playing freeze tag.\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->tag_flags, TAG_FROZEN ) )
	{
		send_to_char( "You can't tag, you're frozen!\r\n", ch );
		return;
	}

	act( AT_PLAIN, "&Y$n&O tags you.", ch, NULL, victim, TO_VICT );
	act( AT_PLAIN, "&Y$n&O tags &Y$N&O.", ch, NULL, victim, TO_NOTVICT );
	act( AT_PLAIN, "&YYou&O tag&Y $N&O.", ch, NULL, victim, TO_CHAR );

	if( ( IS_SET( ch->pcdata->tag_flags, TAG_RED ) &&
		IS_SET( victim->pcdata->tag_flags, TAG_RED ) )
		|| ( IS_SET( ch->pcdata->tag_flags, TAG_BLUE ) && IS_SET( victim->pcdata->tag_flags, TAG_BLUE ) ) )
	{
		if( IS_SET( victim->pcdata->tag_flags, TAG_FROZEN ) )
		{
			REMOVE_BIT( victim->pcdata->tag_flags, TAG_FROZEN );
			act( AT_PLAIN, "&GYou are no longer frozen!", ch, NULL, victim, TO_VICT );
			act( AT_PLAIN, "&G$N is no longer frozen!", ch, NULL, victim, TO_NOTVICT );
			act( AT_PLAIN, "&G$N is no longer frozen!", ch, NULL, victim, TO_CHAR );
		}
		else
		{
			act( AT_PLAIN, "&C$N is not frozen!", ch, NULL, victim, TO_CHAR );
		}
	}
	else
	{
		if( IS_SET( victim->pcdata->tag_flags, TAG_FROZEN ) )
		{
			act( AT_PLAIN, "&Y$N &Ois already frozen!", ch, NULL, victim, TO_CHAR );
		}
		else
		{
			SET_BIT( victim->pcdata->tag_flags, TAG_FROZEN );
			act( AT_PLAIN, "&RYou are frozen!", ch, NULL, victim, TO_VICT );
			act( AT_PLAIN, "&R$N is frozen!", ch, NULL, victim, TO_NOTVICT );
			act( AT_PLAIN, "&R$N is frozen!", ch, NULL, victim, TO_CHAR );
			victim->pcdata->beenfroze += 1;
			ch->pcdata->hasfroze += 1;
			adjust_hiscore( "beenfroze", ch, victim->pcdata->beenfroze );
			adjust_hiscore( "hasfroze", ch, ch->pcdata->hasfroze );
			check_team_frozen( victim );
		}
	}

	return;
}

void auto_tag( void )
{
	DESCRIPTOR_DATA *d;
	int count = 0;

	for( d = first_descriptor; d != NULL; d = d->next )
		if( d->connected == CON_PLAYING )
			count++;

	if( count <= 2 )
	{
		end_tag( );
		return;
	}

	tag_channel( NULL, "&YA Freeze Tag Game has started&O! &YType &O'&Rftag join&O' &Yto play&O." );
	tag_game.status = TAG_ISWAIT;
	tag_game.timer = 3;
	tag_game.playing = 0;
	tag_game.next = -1;
}

void tag_update( void )
{
	char buf[MAX_STRING_LENGTH];
	/*
		if ( tag_game.next > 0 && tag_game.status == TAG_OFF )
		{
			if ( --tag_game.next == 0 )
				auto_tag (  );
		}
	*/
	if( tag_game.status == TAG_ISWAIT )
	{

		tag_game.timer--;

		if( tag_game.timer > 0 )
		{
			sprintf( buf, "&O%d &Yminute%s left to join freeze tag&O.", tag_game.timer, tag_game.timer == 1 ? "" : "s" );
			tag_channel( NULL, buf );
		}
		else
		{
			if( tag_game.playing < 3 )
			{
				end_tag( );
				sprintf( buf, "&YNot enough people have joined&O,&Y maybe next time&O!" );
				tag_channel( NULL, buf );
				return;
			}
			else
			{
				start_tag( );
			}
		}
	}
	else if( tag_game.status == TAG_ISPLAY )
	{
		if( tag_game.playing == 0 )
		{
			end_tag( );
			sprintf( buf, "No one left in freeze tag, next game in %d minutes.", tag_game.next );
			tag_channel( NULL, buf );
			return;
		}
		switch( tag_game.timer )
		{
		case 0:
			end_tag( );
			sprintf( buf, "Time has run out for freeze tag, next game will start in %d minutes.", tag_game.next );
			tag_channel( NULL, buf );
			return;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 10:
		case 15:
			sprintf( buf, "%d minute%s remaining in freeze tag.", tag_game.timer, tag_game.timer > 1 ? "s" : "" );
			tag_channel( NULL, buf );
		default:
			tag_game.timer--;
			break;
		}
	}
}


bool is_tagging( CHAR_DATA *ch )
{
	if( !ch || IS_NPC( ch ) )
		return false;

	if( IS_SET( ch->pcdata->tag_flags, TAG_PLAYING | TAG_WAITING ) && tag_game.status != TAG_OFF )
		return true;

	return false;
}

void tag_channel( CHAR_DATA *ch, const char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *original;
	char buf[MAX_INPUT_LENGTH];

	sprintf( buf, "&B|&CFr&cee&Cze &zT&Wa&zg&B| %s", argument ); /* last %s to reset color */

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */
		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_FREEZE )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}
