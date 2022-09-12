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
#include <unistd.h>
#include "mud.h"
#include "sha256.h"


void look_sky( CHAR_DATA *ch );
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );
bool is_wizvis( CHAR_DATA *ch, CHAR_DATA *victim );

const char *const where_name[] = {
   "&z<&WUsed as Light&z>&B     ",
   "&z<&WWorn on Finger&z>&B    ",
   "&z<&WWorn on Finger&z>&B    ",
   "&z<&WWorn around Neck&z>&B  ",
   "&z<&WWorn around Neck&z>&B  ",
   "&z<&WWorn on Body&z>&B      ",
   "&z<&WWorn on Head&z>&B      ",
   "&z<&WWorn on Legs&z>&B      ",
   "&z<&WWorn on Feet&z>&B      ",
   "&z<&WWorn on Hands&z>&B     ",
   "&z<&WWorn on Arms&z>&B      ",
   "&z<&WEnergy Shield&z>&B     ",
   "&z<&WWorn About Body&z>&B   ",
   "&z<&WWorn about Waist&z>&B  ",
   "&z<&WWorn around Wrist&z>&B ",
   "&z<&WWorn around Wrist&z>&B ",
   "&z<&WWielded&z>&B           ",
   "&z<&WHeld&z>&B              ",
   "&z<&WDual Wielded&z>&B      ",
   "&z<&WWorn on Ears&z>&B      ",
   "&z<&WWorn on Eyes&z>&B      ",
   "&z<&WMissile Wielded&z>&B   ",
   "&z<&WFloating&z>&B          ",
   "&z<&WWorn around Ankle&z>&B ",
   "&z<&WWorn around Ankle&z>&B ",
   "&z<&WWorn on Back&z>&B      ",
   "&z<&WRelic&z>&B             ",
};


/*
 * Local functions.
 */
void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch );
void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch );
void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch );
void show_ships_to_char( SHIP_DATA *ship, CHAR_DATA *ch );
bool check_blind( CHAR_DATA *ch );
void show_condition( CHAR_DATA *ch, CHAR_DATA *victim );
short str_similarity( const char *astr, const char *bstr );
short str_prefix_level( const char *astr, const char *bstr );
const char *get_weapontype( short wtype );
CMDF( similar_help_files );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if( IS_OBJ_STAT( obj, ITEM_INVIS ) )
		mudstrlcat( buf, "&w(&zInvis&w) &D", MSL );
	if( ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) || IS_IMMORTAL( ch ) ) && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
		mudstrlcat( buf, "&w(&BBlue Aura&w) &D", MSL );
	if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
		mudstrlcat( buf, "&w(&YGlowing&w) &D", MSL );
	if( IS_OBJ_STAT( obj, ITEM_HUM ) )
		mudstrlcat( buf, "&w(&gHumming&w) &D", MSL );
	if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
		mudstrlcat( buf, "&w(&zHidden&w) &D", MSL );
	if( IS_OBJ_STAT( obj, ITEM_BURRIED ) )
		mudstrlcat( buf, "&w(&OBuried&w) &D", MSL );
	if( IS_IMMORTAL( ch ) && IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		mudstrlcat( buf, "&w(&GPROTO&w) &D", MSL );
	if( ( IS_AFFECTED( ch, AFF_DETECTTRAPS ) || xIS_SET( ch->act, PLR_HOLYLIGHT ) ) && is_trapped( obj ) )
		mudstrlcat( buf, "&w(&RTrap&w) &D", MSL );


	if( fShort )
	{
		if( obj->short_descr )
			mudstrlcat( buf, obj->short_descr, MSL );
	}
	else
	{
		if( obj->description )
			mudstrlcat( buf, obj->description, MSL );
	}
	return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
const char *halucinated_object( int ms, bool fShort )
{
	int sms = URANGE( 1, ( ms + 10 ) / 5, 20 );

	if( fShort )
		switch( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
		{
		case 1:
			return "a sword";
		case 2:
			return "a stick";
		case 3:
			return "something shiny";
		case 4:
			return "something";
		case 5:
			return "something interesting";
		case 6:
			return "something colorful";
		case 7:
			return "something that looks cool";
		case 8:
			return "a nifty thing";
		case 9:
			return "a cloak of flowing colors";
		case 10:
			return "a mystical flaming sword";
		case 11:
			return "a swarm of insects";
		case 12:
			return "a deathbane";
		case 13:
			return "a figment of your imagination";
		case 14:
			return "your gravestone";
		case 15:
			return "the long lost boots of Ranger Thoric";
		case 16:
			return "a glowing tome of arcane knowledge";
		case 17:
			return "a long sought secret";
		case 18:
			return "the meaning of it all";
		case 19:
			return "the answer";
		case 20:
			return "the key to life, the universe and everything";
		}
	switch( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
	{
	case 1:
		return "A nice looking sword catches your eye.";
	case 2:
		return "The ground is covered in small sticks.";
	case 3:
		return "Something shiny catches your eye.";
	case 4:
		return "Something catches your attention.";
	case 5:
		return "Something interesting catches your eye.";
	case 6:
		return "Something colorful flows by.";
	case 7:
		return "Something that looks cool calls out to you.";
	case 8:
		return "A nifty thing of great importance stands here.";
	case 9:
		return "A cloak of flowing colors asks you to wear it.";
	case 10:
		return "A mystical flaming sword awaits your grasp.";
	case 11:
		return "A swarm of insects buzzes in your face!";
	case 12:
		return "The extremely rare Deathbane lies at your feet.";
	case 13:
		return "A figment of your imagination is at your command.";
	case 14:
		return "You notice a gravestone here... upon closer examination, it reads your name.";
	case 15:
		return "The long lost boots of Ranger Thoric lie off to the side.";
	case 16:
		return "A glowing tome of arcane knowledge hovers in the air before you.";
	case 17:
		return "A long sought secret of all mankind is now clear to you.";
	case 18:
		return "The meaning of it all, so simple, so clear... of course!";
	case 19:
		return "The answer.  One.  It's always been One.";
	case 20:
		return "The key to life, the universe and everything awaits your hand.";
	}
	return "Whoa!!!";
}

char *num_punct( int foo )
{
	int index_new, rest, x;
	unsigned int nindex;
	char buf[16];
	static char buf_new[16];

	snprintf( buf, 16, "%d", foo );
	rest = strlen( buf ) % 3;

	for( nindex = index_new = 0; nindex < strlen( buf ); nindex++, index_new++ )
	{
		x = nindex - rest;
		if( nindex != 0 && ( x % 3 ) == 0 )
		{
			buf_new[index_new] = ',';
			index_new++;
			buf_new[index_new] = buf[nindex];
		}
		else
			buf_new[index_new] = buf[nindex];
	}
	buf_new[index_new] = '\0';
	return buf_new;
}

char *num_punct( long foo )
{
	int index_new, rest, x;
	unsigned int nindex;
	char buf[16];
	static char buf_new[16];

	snprintf( buf, 16, "%ld", foo );
	rest = strlen( buf ) % 3;

	for( nindex = index_new = 0; nindex < strlen( buf ); nindex++, index_new++ )
	{
		x = nindex - rest;
		if( nindex != 0 && ( x % 3 ) == 0 )
		{
			buf_new[index_new] = ',';
			index_new++;
			buf_new[index_new] = buf[nindex];
		}
		else
			buf_new[index_new] = buf[nindex];
	}
	buf_new[index_new] = '\0';
	return buf_new;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
	char **prgpstrShow;
	int *prgnShow;
	int *pitShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count, offcount, tmp, ms, cnt;
	bool fCombine;

	if( !ch->desc )
		return;

	/*
	 * if there's no list... then don't do all this crap!  -Thoric
	 */
	if( !list )
	{
		if( fShowNothing )
		{
			if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
				send_to_char( "     ", ch );
			send_to_char( "Nothing.\r\n", ch );
		}
		return;
	}

	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		send_to_char( "Nothing.\r\n", ch );
		return;
	}

	/*
	 * Alloc space for output lines.
	 */
	count = 0;
	for( obj = list; obj; obj = obj->next_content )
		count++;

	ms = ( ch->mental_state ? ch->mental_state : 1 )
		* ( IS_NPC( ch ) ? 1 : ( ch->pcdata->condition[COND_DRUNK] ? ( ch->pcdata->condition[COND_DRUNK] / 12 ) : 1 ) );

	/*
	 * If not mentally stable...
	 */
	if( abs( ms ) > 40 )
	{
		offcount = URANGE( -( count ), ( count * ms ) / 100, count * 2 );
		if( offcount < 0 )
			offcount += number_range( 0, abs( offcount ) );
		else if( offcount > 0 )
			offcount -= number_range( 0, offcount );
	}
	else
		offcount = 0;

	if( count + offcount <= 0 )
	{
		if( fShowNothing )
		{
			if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
				send_to_char( "     ", ch );
			send_to_char( "Nothing.\r\n", ch );
		}
		return;
	}

	CREATE( prgpstrShow, char *, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	CREATE( prgnShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	CREATE( pitShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	nShow = 0;
	tmp = ( offcount > 0 ) ? offcount : 0;
	cnt = 0;

	/*
	 * Format the list of objects.
	 */
	for( obj = list; obj; obj = obj->next_content )
	{
		if( offcount < 0 && ++cnt >( count + offcount ) )
			break;
		if( tmp > 0 && number_bits( 1 ) == 0 )
		{
			prgpstrShow[nShow] = str_dup( halucinated_object( ms, fShort ) );
			prgnShow[nShow] = 1;
			pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
			--tmp;
		}
		if( obj->wear_loc == WEAR_NONE
			&& can_see_obj( ch, obj ) && ( obj->item_type != ITEM_TRAP || IS_AFFECTED( ch, AFF_DETECTTRAPS ) ) )
		{
			pstrShow = format_obj_to_char( obj, ch, fShort );
			fCombine = false;

			if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
			{
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop backwords.
				 */
				for( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if( !strcmp( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow] += obj->count;
						fCombine = true;
						break;
					}
				}
			}

			pitShow[nShow] = obj->item_type;
			/*
			 * Couldn't combine, or didn't want to.
			 */
			if( !fCombine )
			{
				prgpstrShow[nShow] = str_dup( pstrShow );
				prgnShow[nShow] = obj->count;
				nShow++;
			}
		}
	}
	if( tmp > 0 )
	{
		int x;
		for( x = 0; x < tmp; x++ )
		{
			prgpstrShow[nShow] = str_dup( halucinated_object( ms, fShort ) );
			prgnShow[nShow] = 1;
			pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
		}
	}

	/*
	 * Output the formatted list.       -Color support by Thoric
	 */
	for( iShow = 0; iShow < nShow; iShow++ )
	{
		switch( pitShow[iShow] )
		{
		default:
			set_char_color( AT_OBJECT, ch );
			break;
		case ITEM_BLOOD:
			set_char_color( AT_BLOOD, ch );
			break;
		case ITEM_MONEY:
		case ITEM_TREASURE:
			set_char_color( AT_YELLOW, ch );
			break;
		case ITEM_FOOD:
			set_char_color( AT_HUNGRY, ch );
			break;
		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			set_char_color( AT_THIRSTY, ch );
			break;
		case ITEM_FIRE:
			set_char_color( AT_FIRE, ch );
			break;
		case ITEM_SCROLL:
		case ITEM_WAND:
		case ITEM_STAFF:
			set_char_color( AT_MAGIC, ch );
			break;
		}
		if( fShowNothing )
			send_to_char( "     ", ch );
		send_to_char( prgpstrShow[iShow], ch );
		/*	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) ) */
		{
			if( prgnShow[iShow] != 1 )
				ch_printf( ch, " (%d)", prgnShow[iShow] );
		}

		send_to_char( "\r\n", ch );
		DISPOSE( prgpstrShow[iShow] );
	}

	if( fShowNothing && nShow == 0 )
	{
		if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
			send_to_char( "     ", ch );
		send_to_char( "Nothing.\r\n", ch );
	}

	/*
	 * Clean up.
	 */
	DISPOSE( prgpstrShow );
	DISPOSE( prgnShow );
	DISPOSE( pitShow );
	return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];

	if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
	{
		if( IS_GOOD( victim ) )
		{
			set_char_color( AT_WHITE, ch );
			ch_printf( ch, "%s is encased by a strange purple aura.\r\n",
				IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
		}
		else if( IS_EVIL( victim ) )
		{
			set_char_color( AT_WHITE, ch );
			ch_printf( ch, "%s shimmers beneath an aura of dark energy.\r\n",
				IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
		}
		else
		{
			set_char_color( AT_WHITE, ch );
			ch_printf( ch, "%s is shrouded in flowing shadow and light.\r\n",
				IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
		}
	}
	if( !IS_NPC( victim ) && victim->pcdata->chargelevel == CHARGE_ONE )
	{
		ch_printf( ch, "%s's buster has a &Ggreen&w energy.\r\n", victim->name );
	}
	if( !IS_NPC( victim ) && victim->pcdata->chargelevel == CHARGE_TWO )
	{
		ch_printf( ch, "%s is surrounded by a &Clight blue&w aura.\r\n", victim->name );
	}
	if( !IS_NPC( victim ) && victim->pcdata->chargelevel == CHARGE_THREE )
	{
		ch_printf( ch, "%s is enveloped in a &ppurple&w aura.\r\n", victim->name );
	}
	if( !IS_NPC( victim ) && victim->pcdata->chargelevel == CHARGE_FOUR )
	{
		ch_printf( ch, "%s is engulfed in a &Rred&w blazing aura.\r\n", victim->name );
	}
	if( !IS_NPC( victim ) && IS_SET( victim->pcdata->cybaflags, CYBA_ISCHARGING ) )
	{
		ch_printf( ch, "%s is gathering energy.\r\n", victim->name );
	}
	if( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
	{
		set_char_color( AT_FIRE, ch );
		ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\r\n",
			IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
	}
	if( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
	{
		set_char_color( AT_BLUE, ch );
		ch_printf( ch, "%s is surrounded by cascading torrents of energy.\r\n",
			IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
	}
	/*Scryn 8/13*/
	if( IS_AFFECTED( victim, AFF_ICESHIELD ) )
	{
		set_char_color( AT_LBLUE, ch );
		ch_printf( ch, "%s is ensphered by shards of glistening ice.\r\n",
			IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
	}
	if( IS_AFFECTED( victim, AFF_CHARM ) )
	{
		set_char_color( AT_MAGIC, ch );
		ch_printf( ch, "%s looks ahead free of expression.\r\n",
			IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
	}
	if( !IS_NPC( victim ) && !victim->desc && victim->switched && IS_AFFECTED( victim->switched, AFF_POSSESS ) )
	{
		set_char_color( AT_MAGIC, ch );
		mudstrlcpy( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
		mudstrlcat( buf, " appears to be in a deep trance...\r\n", MAX_STRING_LENGTH );
	}
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char message[MAX_STRING_LENGTH];

	buf[0] = '\0';

	if( IS_NPC( victim ) )
		strcat( buf, " " );

	if( !IS_NPC( victim ) && !victim->desc )
	{
		if( !victim->switched )
			mudstrlcat( buf, "&P[(Link Dead)] ", MAX_STRING_LENGTH );
		else if( !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
			mudstrlcat( buf, "(Switched) ", MAX_STRING_LENGTH );
	}
	if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
		mudstrlcat( buf, "&w[&WAFK&w] &D", MSL );

	if( ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_WIZINVIS ) )
		|| ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_MOBINVIS ) ) )
	{
		if( !IS_NPC( victim ) )
			snprintf( buf1, MSL, "&P(&WInvis %d&P) ", victim->pcdata->wizinvis );
		else
			snprintf( buf1, MSL, "&P(&WMobinvis %d&P) ", victim->mobinvis );
		mudstrlcat( buf, buf1, MSL );
	}
	if( IS_AFFECTED( victim, AFF_INVISIBLE ) )
		mudstrlcat( buf, "&w(&zInvis&w)&D ", MSL );
	if( IS_AFFECTED( victim, AFF_HIDE ) )
		mudstrlcat( buf, "&w(&gHide&w)&D ", MSL );
	//    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
	if( !IS_NPC( victim ) && IS_SET( victim->pcdata->tag_flags, TAG_RED ) )
		mudstrlcat( buf, "&z[&RR&rE&RD&z] ", MAX_STRING_LENGTH );
	if( !IS_NPC( victim ) && IS_SET( victim->pcdata->tag_flags, TAG_BLUE ) )
		mudstrlcat( buf, "&z[&BB&bL&BU&bE&z] ", MAX_STRING_LENGTH );
	if( !IS_NPC( victim ) && IS_SET( victim->pcdata->tag_flags, TAG_FROZEN )
		&& IS_SET( victim->pcdata->tag_flags, TAG_PLAYING ) )
		mudstrlcat( buf, "&R>&CF&cr&Co&cz&Ce&R< ", MAX_STRING_LENGTH );
	if( IS_EVIL( victim ) && IS_AFFECTED( ch, AFF_DETECT_EVIL ) )
		mudstrlcat( buf, "&w(&RRed Aura&w)&D ", MSL );
	if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_LITTERBUG ) )
		mudstrlcat( buf, "&w(&YLITTERBUG&w)&D ", MSL );
	if( !IS_NPC( victim ) && IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
		mudstrlcat( buf, "&R(&PT&pW&PI&pT&R)", MAX_STRING_LENGTH );
	if( IS_NPC( victim ) && IS_IMMORTAL( ch ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		mudstrlcat( buf, "&G(&BPROTO&G)&D ", MSL );
	if( victim->desc && victim->desc->connected == CON_EDITING )
		mudstrlcat( buf, "&w(&zWriting&w)&D ", MSL );

	set_char_color( AT_PERSON, ch );
	if( victim->position == victim->defposition && victim->long_descr[0] != '\0' )
	{
		mudstrlcat( buf, victim->long_descr, MAX_STRING_LENGTH );
		send_to_char( buf, ch );
		show_visible_affects_to_char( victim, ch );
		return;
	}

	if( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_CYBER ) )
	{
		if( victim->pcdata->avatar && victim->pcdata->avatar[0] != '\0' )
		{
			sprintf( buf1, "%s&w is here.\n", victim->pcdata->avatar );
			mudstrlcat( buf, buf1, MAX_STRING_LENGTH );
			send_to_char( buf, ch );
			return;
		}
		else
		{
			sprintf( buf1, "A Nameless Avatar is here..\n" );
			mudstrlcat( buf, buf1, MAX_STRING_LENGTH );
			send_to_char( buf, ch );
			return;
		}
	}

	/*
	 * strcat( buf, PERS( victim, ch ) );       old system of titles
	 * *    removed to prevent prepending of name to title     -Kuran
	 * *
	 * *    But added back bellow so that you can see mobs too :P   -Durga
	 */

	if( !IS_NPC( victim ) && victim->in_room && xIS_SET( victim->in_room->room_flags, ROOM_TAG ) )
		mudstrlcat( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
	else if( !IS_NPC( victim ) && !xIS_SET( ch->act, PLR_BRIEF ) )
		mudstrlcat( buf, victim->pcdata->title, MAX_STRING_LENGTH );
	else
		mudstrlcat( buf, PERS( victim, ch ), MAX_STRING_LENGTH );

	switch( victim->position )
	{
	case POS_DEAD:
		mudstrlcat( buf, " &wis DEAD!!", MAX_STRING_LENGTH );
		break;
	case POS_MORTAL:
		mudstrlcat( buf, " &wis mortally wounded.", MAX_STRING_LENGTH );
		break;
	case POS_INCAP:
		mudstrlcat( buf, " &wis incapacitated.", MAX_STRING_LENGTH );
		break;
	case POS_STUNNED:
		mudstrlcat( buf, " &wis lying here stunned.", MAX_STRING_LENGTH );
		break;
	case POS_SLEEPING:
		if( victim->on != NULL )
		{
			if( IS_SET( victim->on->value[2], SLEEP_AT ) )
			{
				sprintf( message, " &wis sleeping at %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else if( IS_SET( victim->on->value[2], SLEEP_ON ) )
			{
				sprintf( message, " &wis sleeping on %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else
			{
				sprintf( message, " &wis sleeping in %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
		}
		else
		{
			if( ch->position == POS_SITTING || ch->position == POS_RESTING )
				mudstrlcat( buf, " &wis sleeping nearby.", MAX_STRING_LENGTH );
			else
				mudstrlcat( buf, " &wis deep in slumber here.", MAX_STRING_LENGTH );
		}
		break;
	case POS_RESTING:

		if( victim->on != NULL )
		{
			if( IS_SET( victim->on->value[2], REST_AT ) )
			{
				sprintf( message, " &wis resting at %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else if( IS_SET( victim->on->value[2], REST_ON ) )
			{
				sprintf( message, " &wis resting on %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else
			{
				sprintf( message, " &wis resting in %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
		}
		else
		{
			if( ch->position == POS_RESTING )
				mudstrlcat( buf, " &wis sprawled out alongside you.", MAX_STRING_LENGTH );
			else if( ch->position == POS_MOUNTED )
				mudstrlcat( buf, " &wis sprawled out at the foot of your mount.", MAX_STRING_LENGTH );
			else
				mudstrlcat( buf, " &wis sprawled out here.", MAX_STRING_LENGTH );
		}
		break;
	case POS_SITTING:
		if( victim->on != NULL )
		{
			if( IS_SET( victim->on->value[2], SIT_AT ) )
			{
				sprintf( message, " &wis sitting at %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else if( IS_SET( victim->on->value[2], SIT_ON ) )
			{
				sprintf( message, " &wis sitting on %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else
			{
				sprintf( message, " &wis sitting in %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
		}
		else
			mudstrlcat( buf, " &wis sitting here.", MAX_STRING_LENGTH );
		break;
	case POS_STANDING:
		if( victim->on != NULL )
		{
			if( IS_SET( victim->on->value[2], STAND_AT ) )
			{
				sprintf( message, " &wis standing at %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else if( IS_SET( victim->on->value[2], STAND_ON ) )
			{
				sprintf( message, " &wis standing on %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
			else
			{
				sprintf( message, " &wis standing in %s.", victim->on->short_descr );
				mudstrlcat( buf, message, MAX_STRING_LENGTH );
			}
		}
		else if( IS_IMMORTAL( victim ) )
			mudstrlcat( buf, " &wis here before you.", MAX_STRING_LENGTH );
		else
			if( ( victim->in_room->sector_type == SECT_UNDERWATER )
				&& !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
				mudstrlcat( buf, " &wis drowning here.", MAX_STRING_LENGTH );
			else if( victim->in_room->sector_type == SECT_UNDERWATER )
				mudstrlcat( buf, " &wis here in the water.", MAX_STRING_LENGTH );
			else
				if( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
					&& !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
					mudstrlcat( buf, " &wis drowning here.", MAX_STRING_LENGTH );
				else if( victim->in_room->sector_type == SECT_OCEANFLOOR )
					mudstrlcat( buf, " &wis standing here in the water.", MAX_STRING_LENGTH );
				else if( IS_AFFECTED( victim, AFF_FLOATING ) || IS_AFFECTED( victim, AFF_FLYING ) )
					mudstrlcat( buf, " &wis hovering here.", MAX_STRING_LENGTH );
				else
					mudstrlcat( buf, " &wis standing here.", MAX_STRING_LENGTH );
		break;
	case POS_SHOVE:
		mudstrlcat( buf, " &wis being shoved around.", MAX_STRING_LENGTH );
		break;
	case POS_DRAG:
		mudstrlcat( buf, " &wis being dragged around.", MAX_STRING_LENGTH );
		break;
	case POS_MOUNTED:
		mudstrlcat( buf, " &wis here, upon ", MAX_STRING_LENGTH );
		if( !victim->mount )
			mudstrlcat( buf, "&wthin air???", MAX_STRING_LENGTH );
		else if( victim->mount == ch )
			mudstrlcat( buf, "&wyour back.", MAX_STRING_LENGTH );
		else if( victim->in_room == victim->mount->in_room )
		{
			mudstrlcat( buf, PERS( victim->mount, ch ), MAX_STRING_LENGTH );
			mudstrlcat( buf, ".", MAX_STRING_LENGTH );
		}
		else
			mudstrlcat( buf, "someone who left??", MAX_STRING_LENGTH );
		break;
	case POS_FIGHTING:
		mudstrlcat( buf, " &wis here, fighting ", MAX_STRING_LENGTH );
		if( !victim->fighting )
			mudstrlcat( buf, "&wthin air???", MAX_STRING_LENGTH );
		else if( who_fighting( victim ) == ch )
			mudstrlcat( buf, "YOU!", MAX_STRING_LENGTH );
		else if( victim->in_room == victim->fighting->who->in_room )
		{
			mudstrlcat( buf, PERS( victim->fighting->who, ch ), MAX_STRING_LENGTH );
			mudstrlcat( buf, ".", MAX_STRING_LENGTH );
		}
		else
			mudstrlcat( buf, "&wsomeone who left??", MAX_STRING_LENGTH );
		break;
	}

	mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
	buf[0] = UPPER( buf[0] );
	send_to_char( buf, ch );
	show_visible_affects_to_char( victim, ch );
	return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	int iWear;
	bool found;

	if( can_see( victim, ch ) )
	{
		act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
	}

	if( victim->description[0] != '\0' )
	{
		send_to_char( victim->description, ch );
	}
	else
	{
		act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	}

	show_condition( ch, victim );

	found = false;
	for( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if( ( obj = get_eq_char( victim, iWear ) ) != NULL && can_see_obj( ch, obj ) )
		{
			if( !found )
			{
				send_to_char( "\r\n", ch );
				act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
				found = true;
			}
			send_to_char( where_name[iWear], ch );
			send_to_char( format_obj_to_char( obj, ch, true ), ch );
			send_to_char( "\r\n", ch );

		}
	}

	/*
	 * Crash fix here by Thoric
	 */
	if( IS_NPC( ch ) || victim == ch )
		return;

	if( IS_IMMORTAL( victim ) && ( victim->top_level > ch->top_level ) )
		return;

	if( number_percent( ) < ch->pcdata->learned[gsn_peek] )
	{
		send_to_char( "\n\rYou peek at the inventory:\r\n", ch );

		show_list_to_char( victim->first_carrying, ch, true, true );
		learn_from_success( ch, gsn_peek );
	}
	else if( ch->pcdata->learned[gsn_peek] )
		learn_from_failure( ch, gsn_peek );

	return;
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
	CHAR_DATA *rch;

	for( rch = list; rch; rch = rch->next_in_room )
	{
		if( rch == ch )
			continue;

		if( can_see( ch, rch ) )
		{
			show_char_to_char_0( rch, ch );
		}
		else if( room_is_dark( ch->in_room ) && IS_AFFECTED( rch, AFF_INFRARED ) )
		{
			set_char_color( AT_BLOOD, ch );
			send_to_char( "The red form of a living creature is here.\r\n", ch );
		}
	}

	return;
}

void show_ships_to_char( SHIP_DATA *ship, CHAR_DATA *ch )
{
	SHIP_DATA *rship;
	SHIP_DATA *nship = NULL;

	for( rship = ship; rship; rship = nship )
	{
		ch_printf( ch, "&[ship]%-35s     ", rship->name );
		if( ( nship = rship->next_in_room ) != NULL )
		{
			ch_printf( ch, "%-35s", nship->name );
			nship = nship->next_in_room;
		}
		ch_printf( ch, "\r\n&D" );
	}

	return;
}



bool check_blind( CHAR_DATA *ch )
{
	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
		return true;

	if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
		return true;

	if( IS_AFFECTED( ch, AFF_BLIND ) )
	{
		send_to_char( "You can't see a thing!\r\n", ch );
		return false;
	}

	return true;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( const char *arg )
{
	int door;

	if( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
		door = 0;
	else if( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
		door = 1;
	else if( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
		door = 2;
	else if( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
		door = 3;
	else if( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
		door = 4;
	else if( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
		door = 5;
	else if( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
		door = 6;
	else if( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
		door = 7;
	else if( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
		door = 8;
	else if( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
		door = 9;
	else
		door = -1;
	return door;
}

CMDF( do_look )
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	const char *kwd;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *original;
	const char *pdesc;
	bool doexaprog;
	short door;
	int number, cnt;

	if( !ch->desc )
		return;

	if( ch->position < POS_SLEEPING )
	{
		send_to_char( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if( ch->position == POS_SLEEPING )
	{
		send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if( !check_blind( ch ) )
		return;

	if( !IS_NPC( ch )
		&& !xIS_SET( ch->act, PLR_HOLYLIGHT ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && room_is_dark( ch->in_room ) )
	{
		set_char_color( AT_DGREY, ch );
		send_to_char( "It is pitch black ... \r\n", ch );
		show_char_to_char( ch->in_room->first_person, ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

	if( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
	{
		SHIP_DATA *ship;
		send_to_char( "\r\n", ch );
		/*
		 * 'look' or 'look auto'
		 */
		set_char_color( AT_LBLUE, ch );
		send_to_char( ch->in_room->name, ch );
		send_to_char( " ", ch );

		if( !ch->desc->original )
		{
			if( ( get_trust( ch ) >= LEVEL_STAFF ) )
			{
				if( xIS_SET( ch->act, PLR_ROOMVNUM ) )
				{
					set_char_color( AT_BLUE, ch );   /* Added 10/17 by Kuran of */
					send_to_char( "&B{", ch );   /* SWReality */
					ch_printf( ch, "%d", ch->in_room->vnum );
					send_to_char( "} &D", ch );
				}
				if( IS_SET( ch->pcdata->flags, PCFLAG_ROOM ) )
				{
					set_char_color( AT_CYAN, ch );
					send_to_char( "&c[", ch );
					send_to_char( ext_flag_string( &ch->in_room->room_flags, r_flags ), ch );
					send_to_char( "]&D", ch );
				}
			}
		}

		send_to_char( "\r\n", ch );
		send_to_char( "&c=======================================================================\r\n", ch );
		set_char_color( AT_DGREY, ch );

		if( arg1[0] == '\0' || ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_BRIEF ) ) )
			send_to_char( ch->in_room->description, ch );

		if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOEXIT ) )
			do_exits( ch, "" );

		if( IS_AFFECTED( ch, AFF_STALK ) && ch->hunting && !IS_NPC( ch ) )
		{
			do_track( ch, ch->hunting->name );
		}

		show_ships_to_char( ch->in_room->first_ship, ch );
		show_list_to_char( ch->in_room->first_content, ch, false, false );
		show_char_to_char( ch->in_room->first_person, ch );

		if( str_cmp( arg1, "auto" ) )
			if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) != NULL )
			{
				set_char_color( AT_WHITE, ch );
				ch_printf( ch, "\n\rYou look at the monitors and see:\r\n" );

				if( ship->location == ship->lastdoc )
				{
					ROOM_INDEX_DATA *to_room;

					if( ( to_room = get_room_index( ship->location ) ) != NULL )
					{
						ch_printf( ch, "\r\n" );
						original = ch->in_room;
						char_from_room( ch );
						char_to_room( ch, to_room );
						do_glance( ch, "" );
						char_from_room( ch );
						char_to_room( ch, original );
					}
				}
				else
					ch_printf( ch, "Nothing but endless space...\r\n" );

			}

		return;
	}

	if( !str_cmp( arg1, "sky" ) || !str_cmp( arg1, "stars" ) )
	{
		if( !IS_OUTSIDE( ch ) )
		{
			send_to_char( "You can't see the sky indoors.\r\n", ch );
			return;
		}
		else
		{
			look_sky( ch );
			return;
		}
	}

	if( !str_cmp( arg1, "under" ) )
	{
		int count;

		/*
		 * 'look under'
		 */
		if( arg2[0] == '\0' )
		{
			send_to_char( "Look beneath what?\r\n", ch );
			return;
		}

		if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not see that here.\r\n", ch );
			return;
		}
		if( ch->carry_weight + obj->weight > can_carry_w( ch ) )
		{
			send_to_char( "It's too heavy for you to look under.\r\n", ch );
			return;
		}
		count = obj->count;
		obj->count = 1;
		act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
		act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
		obj->count = count;
		if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
			show_list_to_char( obj->first_content, ch, true, true );
		else
			send_to_char( "Nothing.\r\n", ch );
		if( doexaprog )
			oprog_examine_trigger( ch, obj );
		return;
	}

	if( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
	{
		int count;

		/*
		 * 'look in'
		 */
		if( arg2[0] == '\0' )
		{
			send_to_char( "Look in what?\r\n", ch );
			return;
		}

		if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not see that here.\r\n", ch );
			return;
		}

		switch( obj->item_type )
		{
		default:
			send_to_char( "That is not a container.\r\n", ch );
			break;

		case ITEM_DRINK_CON:
			if( obj->value[1] <= 0 )
			{
				send_to_char( "It is empty.\r\n", ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				break;
			}

			ch_printf( ch, "It's %s full of a %s liquid.\r\n",
				obj->value[1] < obj->value[0] / 4
				? "less than" :
				obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color );

			if( doexaprog )
				oprog_examine_trigger( ch, obj );
			break;

		case ITEM_PORTAL:
			for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
			{
				if( pexit->vdir == DIR_PORTAL && IS_SET( pexit->exit_info, EX_PORTAL ) )
				{
					if( room_is_private( ch, pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
					{
						set_char_color( AT_WHITE, ch );
						send_to_char( "That room is private buster!\r\n", ch );
						return;
					}
					original = ch->in_room;
					char_from_room( ch );
					char_to_room( ch, pexit->to_room );
					do_look( ch, "auto" );
					char_from_room( ch );
					char_to_room( ch, original );
					return;
				}
			}
			send_to_char( "You see a swirling chaos...\r\n", ch );
			break;
		case ITEM_WHOLDER:
			ch_printf( ch, "%s&w is for %s type weapons, and holds a weight of %d.\r\n",
				obj->short_descr, get_weapontype( obj->value[1] ), obj->value[0] );
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_DROID_CORPSE:
			if( IS_SET( obj->value[1], CONT_CLOSED ) && obj->item_type != ITEM_WHOLDER )
			{
				send_to_char( "It is closed.\r\n", ch );
				break;
			}

			count = obj->count;
			obj->count = 1;
			act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
			obj->count = count;
			show_list_to_char( obj->first_content, ch, true, true );
			if( doexaprog )
				oprog_examine_trigger( ch, obj );
			break;
		}
		return;
	}

	if( ( pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc ) ) != NULL )
	{
		send_to_char( pdesc, ch );
		return;
	}

	door = get_door( arg1 );
	if( ( pexit = find_door( ch, arg1, true ) ) != NULL )
	{
		if( pexit->keyword )
		{
			if( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_SET( pexit->exit_info, EX_WINDOW ) )
			{
				if( IS_SET( pexit->exit_info, EX_SECRET ) && door != -1 )
					send_to_char( "Nothing special there.\r\n", ch );
				else
					act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
				return;
			}
			if( IS_SET( pexit->exit_info, EX_BASHED ) )
				act( AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );
		}

		if( pexit->description && pexit->description[0] != '\0' )
			send_to_char( pexit->description, ch );
		else
			send_to_char( "Nothing special there.\r\n", ch );

		/*
		 * Ability to look into the next room         -Thoric
		 */
		if( pexit->to_room
			&& ( IS_AFFECTED( ch, AFF_SCRYING )
				|| IS_SET( pexit->exit_info, EX_xLOOK ) || get_trust( ch ) >= LEVEL_STAFF ) )
		{
			if( !IS_SET( pexit->exit_info, EX_xLOOK ) && get_trust( ch ) < LEVEL_STAFF )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You attempt to scry...\r\n", ch );
				/*
				 * Change by Narn, Sept 96 to allow characters who don't have the
				 * scry spell to benefit from objects that are affected by scry.
				 */
				if( !IS_NPC( ch ) )
				{
					int percent = ch->pcdata->learned[skill_lookup( "scry" )];
					if( !percent )
						percent = 99;

					if( number_percent( ) > percent )
					{
						send_to_char( "You fail.\r\n", ch );
						return;
					}
				}
			}
			if( room_is_private( ch, pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
			{
				set_char_color( AT_WHITE, ch );
				send_to_char( "That room is private buster!\r\n", ch );
				return;
			}
			original = ch->in_room;
			if( pexit->distance > 1 )
			{
				ROOM_INDEX_DATA *to_room;
				if( ( to_room = generate_exit( ch->in_room, &pexit ) ) != NULL )
				{
					char_from_room( ch );
					char_to_room( ch, to_room );
				}
				else
				{
					char_from_room( ch );
					char_to_room( ch, pexit->to_room );
				}
			}
			else
			{
				char_from_room( ch );
				char_to_room( ch, pexit->to_room );
			}
			do_look( ch, "auto" );
			char_from_room( ch );
			char_to_room( ch, original );
		}
		return;
	}
	else if( door != -1 )
	{
		send_to_char( "Nothing special there.\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) != NULL )
	{
		show_char_to_char_1( victim, ch );
		return;
	}


	/*
	 * finally fixed the annoying look 2.obj desc bug   -Thoric
	 */
	number = number_argument( arg1, arg );
	for( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		/*
		 * cronel hiscore
		 */
		kwd = is_hiscore_obj( obj );
		if( kwd )
		{
			show_hiscore( kwd, ch );
			return;
		}


		if( can_see_obj( ch, obj ) )
		{
			if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}

			if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}

			if( nifty_is_name_prefix( arg, obj->name ) )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
				if( !pdesc )
					pdesc = get_extra_descr( obj->name, obj->first_extradesc );
				if( !pdesc )
					send_to_char( "You see nothing special.\r\n", ch );
				else
					send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}
		}
	}

	for( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
	{

		/*
		 * cronel hiscore
		 */
		kwd = is_hiscore_obj( obj );
		if( kwd )
		{
			show_hiscore( kwd, ch );
			return;
		}


		if( can_see_obj( ch, obj ) )
		{
			if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}

			if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}
			if( nifty_is_name_prefix( arg, obj->name ) )
			{
				if( ( cnt += obj->count ) < number )
					continue;
				pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
				if( !pdesc )
					pdesc = get_extra_descr( obj->name, obj->first_extradesc );
				if( !pdesc )
					send_to_char( "You see nothing special.\r\n", ch );
				else
					send_to_char( pdesc, ch );
				if( doexaprog )
					oprog_examine_trigger( ch, obj );
				return;
			}
		}
	}

	send_to_char( "You do not see that here.\r\n", ch );
	return;
}

const char *get_weapontype( short wtype )
{
	switch( wtype )
	{
	default:
		return "Unknown";
	case 0:
		return "Hit";
	case 1:
		return "Slice";
	case 2:
		return "Stab";
	case 3:
		return "Slash";
	case 4:
		return "Whip";
	case 5:
		return "Claw";
	case 6:
		return "Blast";
	case 7:
		return "Pound";
	case 8:
		return "Crush";
	case 9:
		return "Shot";
	case 10:
		return "Bite";
	case 11:
		return "Pierce";
	case 12:
		return "Suction";

		/*
			  case 13: return "Swing";
			  case 14: return "Rip";
			  case 15: return "Knife";
			  case 16: return "Cleave";
			  case 17: return "Fist";
			  case 18: return "Immolation";
			  case 19: return "Freeze";
			  case 20: return "Acid";
			  case 21: return "Electrocution";
			  case 22: return "Blessing";
			  case 23: return "Curse";
		*/
	}
}

void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_STRING_LENGTH];
	int percent;

	if( victim->max_hit > 0 )
		percent = ( int ) ( ( 100.0 * ( double ) ( victim->hit ) ) / ( double ) ( victim->max_hit ) );
	else
		percent = -1;


	strcpy( buf, PERS( victim, ch ) );

	if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_DROID ) )
	{

		if( percent >= 100 )
			strcat( buf, " is in perfect condition.\r\n" );
		else if( percent >= 90 )
			strcat( buf, " is slightly scratched.\r\n" );
		else if( percent >= 80 )
			strcat( buf, " has a few scrapes.\r\n" );
		else if( percent >= 70 )
			strcat( buf, " has some dents.\r\n" );
		else if( percent >= 60 )
			strcat( buf, " has a couple holes in its plating.\r\n" );
		else if( percent >= 50 )
			strcat( buf, " has a many broken pieces.\r\n" );
		else if( percent >= 40 )
			strcat( buf, " has many exposed circuits.\r\n" );
		else if( percent >= 30 )
			strcat( buf, " is leaking oil.\r\n" );
		else if( percent >= 20 )
			strcat( buf, " has smoke coming out of it.\r\n" );
		else if( percent >= 10 )
			strcat( buf, " is almost completely broken.\r\n" );
		else
			strcat( buf, " is about to EXPLODE.\r\n" );

	}
	else
	{

		if( percent >= 100 )
			strcat( buf, ": &B[&C==========&B]&w\r\n" );
		else if( percent >= 90 )
			strcat( buf, ": &B[&C=========&c-&B]&w\r\n" );
		else if( percent >= 80 )
			strcat( buf, ": &B[&C========&c--&B]&w\r\n" );
		else if( percent >= 70 )
			strcat( buf, ": &B[&C=======&c---&B]&w\r\n" );
		else if( percent >= 60 )
			strcat( buf, ": &B[&C======&c----&B]&w\r\n" );
		else if( percent >= 50 )
			strcat( buf, ": &B[&C=====&c-----&B]&w\r\n" );
		else if( percent >= 40 )
			strcat( buf, ": &B[&C====&c------&B]&w\r\n" );
		else if( percent >= 30 )
			strcat( buf, ": &B[&C===&c-------&B]&w\r\n" );
		else if( percent >= 20 )
			strcat( buf, ": &B[&C==&c--------&B]&w\r\n" );
		else if( percent >= 10 )
			strcat( buf, ": &B[&C=&c---------&B]&w\r\n" );
		else
			strcat( buf, ": &B[&r--&RALMOST&r--&B]&w\r\n" );

	}
	buf[0] = UPPER( buf[0] );
	send_to_char( buf, ch );
	return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
CMDF( do_glance )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	EXT_BV save_act;

	xCLEAR_BITS( save_act );

	if( !ch->desc )
		return;

	if( ch->position < POS_SLEEPING )
	{
		send_to_char( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if( ch->position == POS_SLEEPING )
	{
		send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if( !check_blind( ch ) )
		return;

	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		save_act = ch->act;
		xSET_BIT( ch->act, PLR_BRIEF );
		do_look( ch, "auto" );
		ch->act = save_act;
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They're not here.", ch );
		return;
	}
	else
	{
		if( can_see( victim, ch ) )
		{
			act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
			act( AT_ACTION, "$n glances at $N.", ch, NULL, victim, TO_NOTVICT );
		}

		show_condition( ch, victim );
		return;
	}

	return;
}


CMDF( do_examine )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	//    BOARD_DATA *board;
	short dam;

	if( !argument )
	{
		bug( "do_examine: null argument.", 0 );
		return;
	}

	if( !ch )
	{
		bug( "do_examine: null ch.", 0 );
		return;
	}

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Examine what?\r\n", ch );
		return;
	}

	sprintf( buf, "%s noprog", arg );
	do_look( ch, buf );

	/*
	 * Support for looking at boards, checking equipment conditions,
	 * and support for trigger positions by Thoric
	 */
	if( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		/*
			if ( (board = get_board( obj )) != NULL )
			{
			   if ( board->num_posts )
				 ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\r\n", board->num_posts );
			   else
				 send_to_char( "There aren't any notes posted here.\r\n", ch );
			}
		*/
		switch( obj->item_type )
		{
		default:
			break;

		case ITEM_ARMOR:
			if( obj->value[1] == 0 )
				obj->value[1] = obj->value[0];
			if( obj->value[1] == 0 )
				obj->value[1] = 1;
			dam = ( short ) ( ( obj->value[0] * 10 ) / obj->value[1] );
			strcpy( buf, "As you look more closely, you notice that it is " );
			if( dam >= 10 )
				strcat( buf, "in superb condition." );
			else if( dam == 9 )
				strcat( buf, "in very good condition." );
			else if( dam == 8 )
				strcat( buf, "in good shape." );
			else if( dam == 7 )
				strcat( buf, "showing a bit of wear." );
			else if( dam == 6 )
				strcat( buf, "a little run down." );
			else if( dam == 5 )
				strcat( buf, "in need of repair." );
			else if( dam == 4 )
				strcat( buf, "in great need of repair." );
			else if( dam == 3 )
				strcat( buf, "in dire need of repair." );
			else if( dam == 2 )
				strcat( buf, "very badly worn." );
			else if( dam == 1 )
				strcat( buf, "practically worthless." );
			else if( dam <= 0 )
				strcat( buf, "broken." );
			strcat( buf, "\r\n" );
			send_to_char( buf, ch );
			break;

		case ITEM_WEAPON:
			dam = INIT_WEAPON_CONDITION - obj->value[0];
			strcpy( buf, "As you look more closely, you notice that it is " );
			if( dam <= 0 )
				strcat( buf, "in superb condition." );
			else if( dam == 1 )
				strcat( buf, "in excellent condition." );
			else if( dam == 2 )
				strcat( buf, "in very good condition." );
			else if( dam == 3 )
				strcat( buf, "in good shape." );
			else if( dam == 4 )
				strcat( buf, "showing a bit of wear." );
			else if( dam == 5 )
				strcat( buf, "a little run down." );
			else if( dam == 6 )
				strcat( buf, "in need of repair." );
			else if( dam == 7 )
				strcat( buf, "in great need of repair." );
			else if( dam == 8 )
				strcat( buf, "in dire need of repair." );
			else if( dam == 9 )
				strcat( buf, "very badly worn." );
			else if( dam == 10 )
				strcat( buf, "practically worthless." );
			else if( dam == 11 )
				strcat( buf, "almost broken." );
			else if( dam == 12 )
				strcat( buf, "broken." );
			strcat( buf, "\r\n" );
			send_to_char( buf, ch );
			if( obj->value[3] == WEAPON_BLASTER )
				ch_printf( ch, "It has %d shots remaining.\r\n", obj->value[4] );
			break;

		case ITEM_FOOD:
			if( obj->timer > 0 && obj->value[1] > 0 )
				dam = ( obj->timer * 10 ) / obj->value[1];
			else
				dam = 10;
			strcpy( buf, "As you examine it carefully you notice that it " );
			if( dam >= 10 )
				strcat( buf, "is fresh." );
			else if( dam == 9 )
				strcat( buf, "is nearly fresh." );
			else if( dam == 8 )
				strcat( buf, "is perfectly fine." );
			else if( dam == 7 )
				strcat( buf, "looks good." );
			else if( dam == 6 )
				strcat( buf, "looks ok." );
			else if( dam == 5 )
				strcat( buf, "is a little stale." );
			else if( dam == 4 )
				strcat( buf, "is a bit stale." );
			else if( dam == 3 )
				strcat( buf, "smells slightly off." );
			else if( dam == 2 )
				strcat( buf, "smells quite rank." );
			else if( dam == 1 )
				strcat( buf, "smells revolting." );
			else if( dam <= 0 )
				strcat( buf, "is crawling with maggots." );
			strcat( buf, "\r\n" );
			send_to_char( buf, ch );
			break;

		case ITEM_SWITCH:
		case ITEM_LEVER:
		case ITEM_PULLCHAIN:
			if( IS_SET( obj->value[0], TRIG_UP ) )
				send_to_char( "You notice that it is in the up position.\r\n", ch );
			else
				send_to_char( "You notice that it is in the down position.\r\n", ch );
			break;
		case ITEM_BUTTON:
			if( IS_SET( obj->value[0], TRIG_UP ) )
				send_to_char( "You notice that it is depressed.\r\n", ch );
			else
				send_to_char( "You notice that it is not depressed.\r\n", ch );
			break;

			/* Not needed due to check in do_look already
				case ITEM_PORTAL:
					sprintf( buf, "in %s noprog", arg );
					do_look( ch, buf );
					break;
			*/

		case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
		{
			short timerfrac = obj->timer;
			if( obj->item_type == ITEM_CORPSE_PC )
				timerfrac = ( int ) obj->timer / 8 + 1;

			switch( timerfrac )
			{
			default:
				send_to_char( "This corpse has recently been slain.\r\n", ch );
				break;
			case 4:
				send_to_char( "This corpse was slain a little while ago.\r\n", ch );
				break;
			case 3:
				send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\r\n", ch );
				break;
			case 2:
				send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\r\n", ch );
				break;
			case 1:
			case 0:
				send_to_char( "Little more than bones, there isn't much left of this corpse.\r\n", ch );
				break;
			}
		}
		if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
			break;
		send_to_char( "When you look inside, you see:\r\n", ch );
		sprintf( buf, "in %s noprog", arg );
		do_look( ch, buf );
		break;

		case ITEM_DROID_CORPSE:
		{
			short timerfrac = obj->timer;

			switch( timerfrac )
			{
			default:
				send_to_char( "These remains are still smoking.\r\n", ch );
				break;
			case 4:
				send_to_char( "The parts of this droid have cooled down completely.\r\n", ch );
				break;
			case 3:
				send_to_char( "The broken droid components are beginning to rust.\r\n", ch );
				break;
			case 2:
				send_to_char( "The pieces are completely covered in rust.\r\n", ch );
				break;
			case 1:
			case 0:
				send_to_char( "All that remains of it is a pile of crumbling rust.\r\n", ch );
				break;
			}
		}

		case ITEM_CONTAINER:
			if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
				break;

		case ITEM_DRINK_CON:
		case ITEM_WHOLDER:
			send_to_char( "When you look inside, you see:\r\n", ch );
			sprintf( buf, "in %s noprog", arg );
			do_look( ch, buf );
		}
		if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
		{
			sprintf( buf, "under %s noprog", arg );
			do_look( ch, buf );
		}
		oprog_examine_trigger( ch, obj );
		if( char_died( ch ) || obj_extracted( obj ) )
			return;

		check_for_trap( ch, obj, TRAP_EXAMINE );
	}
	return;
}


CMDF( do_exits )
{
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;

	set_char_color( AT_EXITS, ch );
	buf[0] = '\0';
	fAuto = !str_cmp( argument, "auto" );

	if( !check_blind( ch ) )
		return;


	mudstrlcpy( buf, fAuto ? "Exits:" : "&C[&cExits&C]\r\n", MAX_STRING_LENGTH );

	found = false;
	for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
		if( pexit->to_room && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
		{
			found = true;
			if( !fAuto )
			{

				if( IS_SET( pexit->exit_info, EX_CLOSED ) )
				{
					sprintf( buf + strlen( buf ), "&W%-5s &W- (closed)\r\n", capitalize( dir_name[pexit->vdir] ) );
				}
				else if( IS_SET( pexit->exit_info, EX_WINDOW ) )
				{
					sprintf( buf + strlen( buf ), "&W%-5s &W- (window)\r\n", capitalize( dir_name[pexit->vdir] ) );
				}
				else if( IS_SET( pexit->exit_info, EX_xAUTO ) )
				{
					sprintf( buf + strlen( buf ), "&W%-5s &W-&W %s\r\n",
						capitalize( pexit->keyword ),
						room_is_dark( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
				}
				else
					sprintf( buf + strlen( buf ), "&W%-5s &W- &W%s\r\n",
						capitalize( dir_name[pexit->vdir] ),
						room_is_dark( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
			}
			else
			{
				sprintf( buf + strlen( buf ), " %s", capitalize( dir_name[pexit->vdir] ) );

			}
		}
	}

	if( !found )
		mudstrlcat( buf, fAuto ? " none.\r\n" : "None.\r\n", MAX_STRING_LENGTH );
	else if( fAuto )
		mudstrlcat( buf, ".\r\n", MAX_STRING_LENGTH );
	send_to_char( buf, ch );
	return;
}

const char *const day_name[] = {
   "Sunday", "Monday", "Tuesday", "Wednesday",
   "Thursday", "Friday", "Saturday"
};

const char *const month_name[] = {
   "March", "April", "May", "June", "July",
   "August", "September", "October", "November", "December",
   "January", "February"
};

CMDF( do_time )
{
	extern char str_boot_time[];
	extern char reboot_time[];
	const char *suf;
	int day;

	day = time_info.day + 1;

	if( day > 4 && day < 20 )
		suf = "th";
	else if( day % 10 == 1 )
		suf = "st";
	else if( day % 10 == 2 )
		suf = "nd";
	else if( day % 10 == 3 )
		suf = "rd";
	else
		suf = "th";

	set_char_color( AT_YELLOW, ch );
	ch_printf( ch,
		"\r\n&GIt is %d&g%s&G, &g%s&G, the &g%d%s&G of &g%s&G.\r\n"
		"&BIt is the season of %s.\r\n"
		"\r\n&YThe system time (E.S.T.):&O %s\r",
		( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
		time_info.hour >= 12 ? "pm" : "am",
		day_name[day % 7],
		day, suf, month_name[time_info.month], season_name[time_info.season], ( char * ) ctime( &current_time ) );

	if( IS_IMMORTAL( ch ) )
	{
		ch_printf( ch, "&YThe mud started up at:&O    %s\r" "&YNext Reboot is set for:&O   %s\r", str_boot_time, reboot_time );
		return;
	}
	return;
}



CMDF( do_weather )
{
	static const char *const sky_look[4] = {
	   "cloudless",
	   "cloudy",
	   "rainy",
	   "lit by flashes of lightning"
	};

	if( !IS_OUTSIDE( ch ) )
	{
		send_to_char( "You can't see the sky from here.\r\n", ch );
		return;
	}

	set_char_color( AT_BLUE, ch );
	ch_printf( ch, "The sky is %s and %s.\r\n",
		sky_look[weather_info.sky],
		weather_info.change >= 0 ? "a warm southerly breeze blows" : "a cold northern gust blows" );
	return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA *ch, const char *argument )
{
	char argall[MAX_INPUT_LENGTH];
	char argone[MAX_INPUT_LENGTH];
	char argnew[MAX_INPUT_LENGTH];
	HELP_DATA *pHelp;
	int lev;

	if( argument[0] == '\0' )
		argument = "summary";

	if( isdigit( argument[0] ) && !is_number( argument ) )
	{
		lev = number_argument( argument, argnew );
		argument = argnew;
	}
	else
		lev = -2;
	/*
	 * Tricky argument handling so 'help a b' doesn't match a.
	 */
	argall[0] = '\0';
	while( argument[0] != '\0' )
	{
		argument = one_argument( argument, argone );
		if( argall[0] != '\0' )
			strcat( argall, " " );
		strcat( argall, argone );
	}

	for( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		if( pHelp->level > get_trust( ch ) )
			continue;
		if( lev != -2 && pHelp->level != lev )
			continue;

		if( is_name( argall, pHelp->keyword ) )
			return pHelp;
	}

	return NULL;
}


/*
 * Now this is cleaner
 */

 //  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles
 //  Snippet by Senir.
short str_similarity( const char *astr, const char *bstr )
{
	short matches = 0;

	if( !astr || !bstr )
		return matches;

	for( ; *astr; astr++ )
	{
		if( LOWER( *astr ) == LOWER( *bstr ) )
			matches++;

		if( ++bstr == '\0' )
			return matches;
	}

	return matches;
}

//  Ranks by number of matches until there's a nonmatching character between two words.
//  Coded for the Similar Helpfiles Snippet by Senir.
short str_prefix_level( const char *astr, const char *bstr )
{
	short matches = 0;

	if( !astr || !bstr )
		return matches;

	for( ; *astr; astr++ )
	{
		if( LOWER( *astr ) == LOWER( *bstr ) )
			matches++;
		else
			return matches;

		if( ++bstr == '\0' )
			return matches;
	}

	return matches;
}

// Main function of Similar Helpfiles Snippet by Senir. It loops through all of the
// helpfiles, using the string matching function defined to find the closest matching
// helpfiles to the argument. It then checks for singles. Then, if matching helpfiles
// are found at all, it loops through and prints out the closest matching helpfiles.
// If its a single(there's only one), it opens the helpfile.
CMDF( similar_help_files )
{
	HELP_DATA *pHelp = NULL;
	char buf[MAX_STRING_LENGTH];
	const char *extension;
	short lvl = 0;
	bool single = false;


	send_to_pager_color( "&GOther Files that **may** help&g:\r\n", ch );

	for( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		buf[0] = '\0';
		extension = pHelp->keyword;

		if( pHelp->level > get_trust( ch ) )
			continue;

		while( extension[0] != '\0' )
		{
			extension = one_argument( extension, buf );

			if( str_similarity( argument, buf ) > lvl )
			{
				lvl = str_similarity( argument, buf );
				single = true;
			}
			else if( str_similarity( argument, buf ) == lvl && lvl > 0 )
			{
				single = false;
			}
		}
	}

	if( lvl == 0 )
	{
		send_to_pager_color( "&GNo similar help files&g.\r\n", ch );
		return;
	}

	for( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		buf[0] = '\0';
		extension = pHelp->keyword;

		while( extension[0] != '\0' )
		{
			extension = one_argument( extension, buf );

			if( str_similarity( argument, buf ) >= lvl && pHelp->level <= get_trust( ch ) )
			{
				if( single )
				{
					send_to_pager_color( "&GOpening only similar helpfile&g.\r\n", ch );
					do_help( ch, buf );
					return;
				}

				ch_printf( ch, "&C-  &c%s\r\n", pHelp->keyword );
				break;

			}

		}
	}
	return;
}


CMDF( do_help )
{
	HELP_DATA *pHelp;
	static char const *donotshow = "IMOTD MOTD AMOTD NMOTD M_ADVHERO_ M_BALZHUR_ M_GODLVL1_ M_GODLVL2_ \
                                  M_GODLVL3_ M_GODLVL4_ M_GODLVL5_ M_GODLVL6_ M_GODLVL7_ M_GODLVL8_ M_GODLVL9_ \
                                   HITBYABOAT";

	set_pager_color( AT_HELP, ch );
	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "&YSyntax: Help <keyword>&D\r\n", ch );
		send_to_char( "&WCommon keywords:\n\rRules\n\rChannelusage\n\rPk\n\rBank\n\rTrain\n\rNewbie\r\n", ch );
		return;
	}

	if( ( pHelp = get_help( ch, argument ) ) == NULL )
	{
		ch_printf( ch, "&YNo help file found for &O%s&Y.\r\n", argument );
		similar_help_files( ch, argument );
		return;

	}

	if( strstr( donotshow, pHelp->keyword ) == NULL )
	{
		if( IS_IMMORTAL( ch ) )
			pager_printf( ch, "&[helps](%4d) %s", pHelp->level, pHelp->keyword );
		else
			send_to_pager( pHelp->keyword, ch );
		send_to_pager( "\r\n&[divider]=========================================================&D\r\n", ch );
	}

	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SOUND ) )
		send_to_pager( "!!SOUND(help)", ch );

	/*
	 * Strip leading '.' to allow initial blanks.
	 */
	if( pHelp->text[0] == '.' )
		pager_printf( ch, "&[helps]%s", pHelp->text + 1 );
	else
		pager_printf( ch, "&[helps]%s", pHelp->text );
	return;
}


/*
 * Help editor							-Thoric
 */
CMDF( do_hedit )
{
	HELP_DATA *pHelp;

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor.\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;
	case SUB_HELP_EDIT:
		if( ( pHelp = ( HELP_DATA * ) ch->dest_buf ) == NULL )
		{
			bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
			stop_editing( ch );
			return;
		}
		STRFREE( pHelp->text );
		pHelp->text = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
	if( ( pHelp = get_help( ch, argument ) ) == NULL ) /* new help */
	{
		HELP_DATA *tHelp;
		char argnew[MAX_INPUT_LENGTH];
		int lev;
		bool new_help = true;

		for( tHelp = first_help; tHelp; tHelp = tHelp->next )
			if( !str_cmp( argument, tHelp->keyword ) )
			{
				pHelp = tHelp;
				new_help = false;
				break;
			}
		if( new_help )
		{
			if( isdigit( argument[0] ) )
			{
				lev = number_argument( argument, argnew );
				argument = argnew;
			}
			else
				lev = get_trust( ch );
			CREATE( pHelp, HELP_DATA, 1 );
			pHelp->keyword = STRALLOC( strupper( argument ) );
			pHelp->text = STRALLOC( "" );
			pHelp->level = lev;
			add_help( pHelp );
		}
	}
	ch->substate = SUB_HELP_EDIT;
	ch->dest_buf = pHelp;
	start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
const char *help_fix( const char *text )
{
	char *fixed;

	if( !text )
		return "";

	fixed = strip_cr( text );

	if( fixed[0] == ' ' )
		fixed[0] = '.';

	return fixed;
}

CMDF( do_hset )
{
	HELP_DATA *pHelp;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = smash_tilde_copy( argument );
	argument = one_argument( argument, arg1 );
	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: hset <field> [value] [help page]\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Field being one of:\r\n", ch );
		send_to_char( "  level keyword remove save\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "save" ) )
	{
		FILE *fpout;

		log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_LIAISON );

		rename( "help.are", "help.are.bak" );
		if( ( fpout = FileOpen( "help.are", "w" ) ) == NULL )
		{
			bug( "hset save: FileOpen", 0 );
			perror( "help.are" );
			return;
		}

		fprintf( fpout, "#HELPS\n\n" );
		for( pHelp = first_help; pHelp; pHelp = pHelp->next )
			fprintf( fpout, "%d %s~\n%s~\n\n", pHelp->level, pHelp->keyword, help_fix( pHelp->text ) );

		fprintf( fpout, "0 $~\n\n\n#$\n" );
		FileClose( fpout );
		send_to_char( "Saved.\r\n", ch );
		return;
	}
	if( str_cmp( arg1, "remove" ) )
		argument = one_argument( argument, arg2 );

	if( ( pHelp = get_help( ch, argument ) ) == NULL )
	{
		send_to_char( "Cannot find help on that subject.\r\n", ch );
		return;
	}
	if( !str_cmp( arg1, "remove" ) )
	{
		UNLINK( pHelp, first_help, last_help, next, prev );
		STRFREE( pHelp->text );
		STRFREE( pHelp->keyword );
		DISPOSE( pHelp );
		send_to_char( "Removed.\r\n", ch );
		return;
	}
	if( !str_cmp( arg1, "level" ) )
	{
		int lev;

		if( !is_number( arg2 ) )
		{
			send_to_char( "Level field must be numeric.\r\n", ch );
			return;
		}

		lev = atoi( arg2 );
		if( lev < -1 || lev > get_trust( ch ) )
		{
			send_to_char( "You can't set the level to that.\r\n", ch );
			return;
		}
		pHelp->level = lev;
		send_to_char( "Done.\r\n", ch );
		return;
	}
	if( !str_cmp( arg1, "keyword" ) )
	{
		STRFREE( pHelp->keyword );
		pHelp->keyword = STRALLOC( strupper( arg2 ) );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	do_hset( ch, "" );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
CMDF( do_hlist )
{
	int min, max, minlimit, maxlimit, cnt;
	char arg[MAX_INPUT_LENGTH];
	HELP_DATA *help;

	maxlimit = get_trust( ch );
	minlimit = maxlimit >= LEVEL_LIAISON ? -1 : 0;
	argument = one_argument( argument, arg );
	if( arg[0] != '\0' )
	{
		min = URANGE( minlimit, atoi( arg ), maxlimit );
		if( argument[0] != '\0' )
			max = URANGE( min, atoi( argument ), maxlimit );
		else
			max = maxlimit;
	}
	else
	{
		min = minlimit;
		max = maxlimit;
	}
	set_pager_color( AT_GREEN, ch );
	pager_printf( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );
	for( cnt = 0, help = first_help; help; help = help->next )
		if( help->level >= min && help->level <= max )
		{
			pager_printf( ch, "  %3d %s\r\n", help->level, help->keyword );
			++cnt;
		}
	if( cnt )
		pager_printf( ch, "\r\n%d pages found.\r\n", cnt );
	else
		send_to_char( "None found.\r\n", ch );
}


/*
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 */
CMDF( do_who )
{
	char buf[MAX_STRING_LENGTH];
	char clan_name[MAX_INPUT_LENGTH];
	char invis_str[MAX_INPUT_LENGTH];
	char char_name[MAX_INPUT_LENGTH];
	char extra_title[MAX_STRING_LENGTH];
	char pker[MAX_STRING_LENGTH];
	char lev[MAX_STRING_LENGTH];
	char dnd[MAX_STRING_LENGTH];
	char race_text[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int iRace;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	int vMatch;
	CHAR_DATA *wint;
	char tempbuf[20];
	char tempbuf2[20];
	bool rgfRace[MAX_RACE];
	bool fRaceRestrict;
	bool fImmortalOnly;
	bool fShowHomepage;
	bool fClanMatch; /* SB who clan */
	CLAN_DATA *pClan;
	FILE *whoout;
	bool ch_comlink = false;
	OBJ_DATA *obj;
	int wlevel;
	/*
	 * #define WT_IMM    0;
	 * #define WT_MORTAL 1;
	 */

	WHO_DATA *cur_who = NULL;
	WHO_DATA *next_who = NULL;
	WHO_DATA *first_mortal = NULL;
	WHO_DATA *first_newbie = NULL;
	WHO_DATA *first_imm = NULL;

	/*
	 * Set default arguments.
	 */
	iLevelLower = 0;
	iLevelUpper = MAX_LEVEL;
	fRaceRestrict = false;
	fImmortalOnly = false;
	fShowHomepage = false;
	fClanMatch = false;  /* SB who clan  */
	for( iRace = 0; iRace < MAX_RACE; iRace++ )
		rgfRace[iRace] = false;

	if( ch && IS_NPC( ch ) )
		return;

	if( ch )
	{
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

	/*
	 * Parse arguments.
	 */
	nNumber = 0;
	for( ;; )
	{
		char arg[MAX_STRING_LENGTH];

		argument = one_argument( argument, arg );
		if( arg[0] == '\0' )
			break;


		if( is_number( arg ) )
		{
			switch( ++nNumber )
			{
			case 1:
				iLevelLower = atoi( arg );
				break;
			case 2:
				iLevelUpper = atoi( arg );
				break;
			default:
				send_to_char( "Only two level numbers allowed.\r\n", ch );
				return;
			}
		}
		else
		{
			if( strlen( arg ) < 3 )
			{
				send_to_char( "Be a little more specific please.\r\n", ch );
				return;
			}

			/*
			 * Look for classes to turn on.
			 */

			if( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
				fImmortalOnly = true;
			else if( !str_cmp( arg, "www" ) )
				fShowHomepage = true;
			else /* SB who clan (order), guild */ if( ( pClan = get_clan( arg ) ) )
				fClanMatch = true;
			else
			{
				for( iRace = 0; iRace < MAX_RACE; iRace++ )
				{
					if( !str_cmp( arg, race_table[iRace].race_name ) )
					{
						rgfRace[iRace] = true;
						break;
					}
				}
				if( iRace != MAX_RACE )
					fRaceRestrict = true;

				if( iRace == MAX_RACE && fClanMatch == false )
				{
					send_to_char( "That's not a race, or organization.\r\n", ch );
					return;
				}
			}
		}
	}

	/*
	 * Now find matching chars.
	 */
	wint = 0;
	vMatch = 0;
	nMatch = 0;
	buf[0] = '\0';
	if( ch )
		set_pager_color( AT_GREEN, ch );
	else
	{
		if( fShowHomepage )
			whoout = FileOpen( WEBWHO_FILE, "w" );
		else
			whoout = FileOpen( WHO_FILE, "w" );
	}

	for( wint = first_char; wint; wint = wint->next )
	{
		if( !IS_NPC( wint ) && is_wizvis( ch, wint ) )
			vMatch++;
	}

	/*
	 * start from last to first to get it in the proper order
	 */
	 /*
	  * Thunder -- Beginning for-loop for sorted who
	  */

	for( wlevel = 1; wlevel <= 2001; wlevel++ )
	{

		for( d = last_descriptor; d; d = d->prev )
		{
			CHAR_DATA *wch;
			char const *race;

			if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
				continue;

			if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
				|| ( !can_see( ch, d->character ) && IS_IMMORTAL( d->character ) ) || d->original )
				continue;


			wch = d->original ? d->original : d->character;

			if( wch->skill_level[COMBAT_ABILITY] + wch->skill_level[HUNTING_ABILITY] != wlevel )
				continue;

			if( wch->top_level < iLevelLower
				|| wch->top_level > iLevelUpper
				|| ( fImmortalOnly && wch->top_level < LEVEL_STAFF )
				|| ( fRaceRestrict && !rgfRace[wch->race] ) || ( fClanMatch && ( pClan != wch->pcdata->clan ) ) /* SB */ )
				continue;

			nMatch++;

			//       if( wch->top_level != wlevel )
			//          continue;

			if( fShowHomepage && wch->pcdata->homepage && wch->pcdata->homepage[0] != '\0' )
				sprintf( char_name, "<A HREF=\"%s\">%s</A>", show_tilde( wch->pcdata->homepage ), wch->name );
			else
				mudstrlcpy( char_name, "", MAX_INPUT_LENGTH );


			if( wch->sex == SEX_MALE )
			{
				sprintf( race_text, "&B%s", race_table[wch->race].race_name );

				if( strlen( race_text ) > 5 )
				{
					strncpy( tempbuf, race_text, 5 );
					tempbuf[5] = '\0';
				}
				race = tempbuf;
			}
			else if( wch->sex == SEX_FEMALE )
			{
				sprintf( race_text, "&P%s", race_table[wch->race].race_name );

				if( strlen( race_text ) > 5 )
				{
					strncpy( tempbuf, race_text, 5 );
					tempbuf[5] = '\0';
				}

				race = tempbuf;
			}
			else if( wch->sex == SEX_NONBINARY )
			{
				sprintf( race_text, "&W%s", race_table[wch->race].race_name );

				if( strlen( race_text ) > 5 )
				{
					strncpy( tempbuf, race_text, 5 );
					tempbuf[5] = '\0';
				}
				race = tempbuf;
			}

			switch( wch->top_level )
			{
			default:
				break;
			case 2000:
				race = "The Ghost in the Machine";
				break;
			case MAX_LEVEL - 0:
				race = "Owner";
				break;
			case MAX_LEVEL - 1:
				race = "Admin";
				break;
			case MAX_LEVEL - 2:
				race = "Shipwright";
				break;
			case MAX_LEVEL - 3:
				race = "Builder";
				break;
			case MAX_LEVEL - 4:
				race = "Enforcer";
				break;
			case MAX_LEVEL - 5:
				race = "Liaison";
				break;
			case MAX_LEVEL - 6:
				race = "Staff";
				break;
			}

			if( ch && !nifty_is_name( wch->name, wch->pcdata->title ) && ch->top_level > wch->top_level )
				snprintf( extra_title, MAX_INPUT_LENGTH, " [%s]", wch->name );
			else
				mudstrlcpy( extra_title, "", MAX_INPUT_LENGTH );

			if( xIS_SET( wch->act, PLR_PPKER ) )
				sprintf( pker, " &r|&PP&pP&PK&r|" );
			else if( xIS_SET( wch->act, PLR_PKER ) )
				sprintf( pker, "  &p|&rPK&p|&W" );
			else
				mudstrlcpy( pker, "      &W", MSL );

			if( IS_SET( wch->pcdata->flags, PCFLAG_DND ) )
				sprintf( dnd, "&B[&CD&cN&CD&B]" );
			else
				mudstrlcpy( dnd, "", MSL );

			if( IS_RETIRED( wch ) )
			{
				race = "Retired";
			}
			else if( IS_GUEST( wch ) )
				race = "Guest";
			else if( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
				race = wch->pcdata->rank;

			sprintf( tempbuf2, "  " );
			if( wch->pcdata->clan )
			{
				sprintf( tempbuf2, "%s %s ^z", wch->pcdata->clan->cone, wch->pcdata->clan->ctwo );
			}
			mudstrlcpy( clan_name, tempbuf2, MSL );

			if( xIS_SET( wch->act, PLR_WIZINVIS ) )
				sprintf( invis_str, "(%d)", wch->pcdata->wizinvis );
			else
				invis_str[0] = '\0';

			if( IS_IMMORTAL( wch ) )
			{
				sprintf( lev, "%d", wch->top_level );
			}
			else
			{
				if( IS_SET( wch->pcdata->cybaflags, CYBA_NOLEVEL ) )
				{
					sprintf( lev, "&b    " );
				}
				else
				{
					sprintf( lev, "%d", wch->skill_level[COMBAT_ABILITY] + wch->skill_level[HUNTING_ABILITY] );
				}
			}

			snprintf( buf, MSL+1, "&C[&c%4s %s&C]&W %s^z%s%s%s %s%s%s %s%s%s %s^x\r\n",
				lev,
				race,
				clan_name,
				pker,
				invis_str,
				IS_SET( wch->pcdata->flags, PCFLAG_TWIT ) ? "&R(&PT&pW&PI&pT&R) " : "",
				IS_SET( wch->pcdata->tag_flags, TAG_RED ) ? "&z[&RR&rE&RD&z] " : "",
				IS_SET( wch->pcdata->tag_flags, TAG_BLUE ) ? "&z[&BB&bL&BU&bE&z] " : "",
				char_name,
				IS_NPC( wch ) ? "" : xIS_SET( wch->act, PLR_AFK ) ? wch->pcdata->afkmess : wch->pcdata->title,
				extra_title, xIS_SET( wch->act, PLR_KILLER ) ? "&P[&RW&rF&RM&P] &W" : "&W", dnd );
			/*
			 * This is where the old code would display the found player to the ch.
			 * What we do instead is put the found data into a linked list
			 */

			 /*
			  * First make the structure.
			  */
			CREATE( cur_who, WHO_DATA, 1 );
			cur_who->text = str_dup( buf );
			if( IS_IMMORTAL( wch ) )
				cur_who->type = WT_IMM;
			else if( get_trust( wch ) <= 7 )
				cur_who->type = WT_NEWBIE;
			else
				cur_who->type = WT_MORTAL;

			/*
			 * Then put it into the appropriate list.
			 */
			switch( cur_who->type )
			{
			case WT_MORTAL:
				cur_who->next = first_mortal;
				first_mortal = cur_who;
				break;
			case WT_IMM:
				cur_who->next = first_imm;
				first_imm = cur_who;
				break;
			case WT_NEWBIE:
				cur_who->next = first_newbie;
				first_newbie = cur_who;
				break;

			}

		}

	}
	/* Ok, now we have three separate linked lists and what remains is to
	* display the information and clean up.
	*/

	/* Deadly list removed for swr ... now only 2 lists */

	if( ch )
		send_to_char( "\r\n&pAccessing CDI Information&P...\r\n&P...\r\n&P..&pFound!\r\n", ch );
	else
		fprintf( whoout, "%s", "\r\n&pAccessing CDI Information&P...\r\n&P...\r\n&P..&pFound!\r\n" );

	if( first_imm )
	{
		if( !ch )
			fprintf( whoout, "%s",
				"\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &c Staff  &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n" );
		else
			send_to_pager
			( "\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &c Staff  &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n",
				ch );
	}

	for( cur_who = first_imm; cur_who; cur_who = next_who )
	{
		if( !ch )
			fprintf( whoout, "%s", cur_who->text );
		else
			send_to_pager( cur_who->text, ch );
		next_who = cur_who->next;
		DISPOSE( cur_who->text );
		DISPOSE( cur_who );
	}

	if( first_mortal )
	{
		if( !ch )
			fprintf( whoout, "%s",
				"\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &cPlayers &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n" );
		else
			send_to_pager
			( "\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &cPlayers &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n",
				ch );
	}

	for( cur_who = first_mortal; cur_who; cur_who = next_who )
	{

		if( !ch )
			fprintf( whoout, "%s", cur_who->text );
		else
			send_to_pager( cur_who->text, ch );
		next_who = cur_who->next;
		DISPOSE( cur_who->text );
		DISPOSE( cur_who );
	}

	if( first_newbie )
	{
		if( !ch )
			fprintf( whoout, "%s",
				"\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &cNewbies &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n" );
		else
			send_to_pager
			( "\r\n&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&C[ &cNewbies &C]&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-&G=&g-\r\n",
				ch );
	}

	for( cur_who = first_newbie; cur_who; cur_who = next_who )
	{
		if( !ch )
			fprintf( whoout, "%s", cur_who->text );
		else
			send_to_pager( cur_who->text, ch );
		next_who = cur_who->next;
		DISPOSE( cur_who->text );
		DISPOSE( cur_who );
	}

	if( !ch )
	{
		fprintf( whoout, "&YVisible Player%s: %d/%d   Max since reboot: %d   Max Ever: %d\r\n",
			vMatch == 1 ? "" : "s", nMatch, vMatch, sysdata.maxplayers, sysdata.alltimemax );
		fprintf( whoout, "%s", "\r\n&pLogging off&P..\r\n&P...\r\n&P..&pDisconnected." );
		FileClose( whoout );
		return;
	}

	set_char_color( AT_YELLOW, ch );
	ch_printf( ch, "Visible Player%s: %d/%d   Max since reboot: %d   Max Ever: %d\r\n", vMatch == 1 ? "" : "s", nMatch,
		vMatch, sysdata.maxplayers, sysdata.alltimemax );
	send_to_char( "\r\n&pLogging off&P..\r\n&P...\r\n&P..&pDisconnected.", ch );
	return;
}

CMDF( do_compare )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	const char *msg;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if( arg1[0] == '\0' )
	{
		send_to_char( "Compare what to what?\r\n", ch );
		return;
	}

	if( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		for( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
		{
			if( obj2->wear_loc != WEAR_NONE
				&& can_see_obj( ch, obj2 )
				&& obj1->item_type == obj2->item_type && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) != 0 )
				break;
		}

		if( !obj2 )
		{
			send_to_char( "You aren't wearing anything comparable.\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\r\n", ch );
			return;
		}
	}

	msg = NULL;
	value1 = 0;
	value2 = 0;

	if( obj1 == obj2 )
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else if( obj1->item_type != obj2->item_type )
	{
		msg = "You can't compare $p and $P.";
	}
	else
	{
		switch( obj1->item_type )
		{
		default:
			msg = "You can't compare $p and $P.";
			break;

		case ITEM_ARMOR:
			value1 = obj1->value[0];
			value2 = obj2->value[0];
			break;

		case ITEM_WEAPON:
			value1 = obj1->value[1] + obj1->value[2];
			value2 = obj2->value[1] + obj2->value[2];
			break;
		}
	}

	if( !msg )
	{
		if( value1 == value2 )
			msg = "$p and $P look about the same.";
		else if( value1 > value2 )
			msg = "$p looks better than $P.";
		else
			msg = "$p looks worse than $P.";
	}

	act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
	return;
}



CMDF( do_where )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;

	if( get_trust( ch ) < LEVEL_STAFF )
	{
		send_to_char( "If only life were really that simple...\r\n", ch );
		return;
	}

	one_argument( argument, arg );

	set_pager_color( AT_PERSON, ch );
	if( arg[0] == '\0' )
	{
		if( get_trust( ch ) >= LEVEL_STAFF )
			send_to_pager( "Players logged in:\r\n", ch );
		else
			pager_printf( ch, "Players near you in %s:\r\n", ch->in_room->area->name );
		found = false;
		for( d = first_descriptor; d; d = d->next )
			if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
				&& ( victim = d->character ) != NULL
				&& !IS_NPC( victim )
				&& victim->in_room
				&& ( victim->in_room->area == ch->in_room->area || get_trust( ch ) >= LEVEL_STAFF )
				&& can_see( ch, victim ) )
			{
				found = true;
				pager_printf( ch, "&W%-15s &w%-25s    &W%-5d\r\n", victim->name, victim->in_room->name, victim->in_room->vnum );
			}
		if( !found )
			send_to_char( "None\r\n", ch );
	}
	else
	{
		found = false;
		for( victim = first_char; victim; victim = victim->next )
			if( victim->in_room
				&& victim->in_room->area == ch->in_room->area
				&& !IS_AFFECTED( victim, AFF_HIDE )
				&& !IS_AFFECTED( victim, AFF_SNEAK ) && can_see( ch, victim ) && is_name( arg, victim->name ) )
			{
				found = true;
				pager_printf( ch, "%-28s %s\r\n", PERS( victim, ch ), victim->in_room->name );
				break;
			}
		if( !found )
			act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	}

	return;
}



/*
void do_consider( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int diff;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	send_to_char( "Consider killing whom?\r\n", ch );
	return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	send_to_char( "They're not here.\r\n", ch );
	return;
	}

	diff = ( victim->top_level - ch->top_level ) * 10 ;

	diff += (int) (victim->max_hit - ch->max_hit) / 10;

	 if ( diff <= -200) msg = "$N looks like a feather!";
	else if ( diff <= -150) msg = "Hey! Where'd $N go?";
	else if ( diff <= -100) msg = "Easy as pie!";
	else if ( diff <=  -50) msg = "$N is a wimp.";
	else if ( diff <=    0) msg = "$N looks weaker than you.";
	else if ( diff <=   50) msg = "$N looks about as strong as you.";
	else if ( diff <=  100) msg = "It would take a bit of luck...";
	else if ( diff <=  150) msg = "It would take a lot of luck, and a really big blaster!";
	else if ( diff <=  200) msg = "Why don't you just attack a gundam with a stick?";
	else                    msg = "$N is built like an Gundam!";
	act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

	return;
}
*/

CMDF( do_consider )
{
	/*
	 * New Consider code by Ackbar counts in a little bit more than
	 * just their hp compared to yours :P Messages from godwars so leave
	 * this in they deserve some credit :) - Ackbar
	 */
	int diff;
	int con_hp;
	const char *msg;
	int overall;
	CHAR_DATA *victim;
	/*
	 * It counts your difference's in combat, ac, and hp and then uses
	 * * a simple enough formula to determine who's most likely to win :).
	 */

	if( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char( "They are not here.\r\n", ch );
		return;
	}

	act( AT_WHITE, "$n examines $N closely looking for any weaknesses.", ch, NULL, victim, TO_NOTVICT );
	act( AT_WHITE, "You examine $N closely looking for any weaknesses.", ch, NULL, victim, TO_CHAR );
	act( AT_WHITE, "$n examines you closely looking for weaknesses.", ch, NULL, victim, TO_VICT );

	overall = 0;
	con_hp = victim->hit;


	diff = ch->hit * 100 / con_hp;
	if( diff <= 10 )
	{
		msg = "$E is currently FAR healthier than you are.";
		overall = overall - 3;
	}
	else if( diff <= 50 )
	{
		msg = "$E is currently much healthier than you are.";
		overall = overall - 2;
	}
	else if( diff <= 75 )
	{
		msg = "$E is currently slightly healthier than you are.";
		overall = overall - 1;
	}
	else if( diff <= 125 )
	{
		msg = "$E is currently about as healthy as you are.";
	}
	else if( diff <= 200 )
	{
		msg = "You are currently slightly healthier than $M.";
		overall = overall + 1;
	}
	else if( diff <= 500 )
	{
		msg = "You are currently much healthier than $M.";
		overall = overall + 2;
	}
	else
	{
		msg = "You are currently FAR healthier than $M.";
		overall = overall + 3;
	}
	act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

	diff = victim->armor - ch->armor;
	if( diff <= -100 )
	{
		msg = "$E is FAR better armoured than you.";
		overall = overall - 3;
	}
	else if( diff <= -50 )
	{
		msg = "$E looks much better armoured than you.";
		overall = overall - 2;
	}
	else if( diff <= -25 )
	{
		msg = "$E looks better armoured than you.";
		overall = overall - 1;
	}
	else if( diff <= 25 )
	{
		msg = "$E seems about as well armoured as you.";
	}
	else if( diff <= 50 )
	{
		msg = "You are better armoured than $M.";
		overall = overall + 1;
	}
	else if( diff <= 100 )
	{
		msg = "You are much better armoured than $M.";
		overall = overall + 2;
	}
	else
	{
		msg = "You are FAR better armoured than $M.";
		overall = overall + 3;
	}
	act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

	diff = victim->top_level - ch->top_level + GET_HITROLL( victim ) - GET_HITROLL( ch );
	if( diff <= -35 )
	{
		msg = "You are FAR more skilled than $M.";
		overall = overall + 3;
	}
	else if( diff <= -15 )
	{
		msg = "$E is not as skilled as you are.";
		overall = overall + 2;
	}
	else if( diff <= -5 )
	{
		msg = "$E doesn't seem quite as skilled as you.";
		overall = overall + 1;
	}
	else if( diff <= 5 )
	{
		msg = "You are about as skilled as $M.";
	}
	else if( diff <= 15 )
	{
		msg = "$E is slightly more skilled than you are.";
		overall = overall - 1;
	}
	else if( diff <= 35 )
	{
		msg = "$E seems more skilled than you are.";
		overall = overall - 2;
	}
	else
	{
		msg = "$E is FAR more skilled than you.";
		overall = overall - 3;
	}
	act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

	diff = victim->top_level - ch->top_level + GET_DAMROLL( victim ) - GET_DAMROLL( ch );
	if( diff <= -35 )
	{
		msg = "You are FAR more powerful than $M.";
		overall = overall + 3;
	}
	else if( diff <= -15 )
	{
		msg = "$E is not as powerful as you are.";
		overall = overall + 2;
	}
	else if( diff <= -5 )
	{
		msg = "$E doesn't seem quite as powerful as you.";
		overall = overall + 1;
	}
	else if( diff <= 5 )
	{
		msg = "You are about as powerful as $M.";
	}
	else if( diff <= 15 )
	{
		msg = "$E is slightly more powerful than you are.";
		overall = overall - 1;
	}
	else if( diff <= 35 )
	{
		msg = "$E seems more powerful than you are.";
		overall = overall - 2;
	}
	else
	{
		msg = "$E is FAR more powerful than you.";
		overall = overall - 3;
	}
	act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

	diff = overall;
	if( diff <= -11 )
	{
		msg = "Conclusion: $E would kill you in seconds.";
	}
	else if( diff <= -7 )
	{
		msg = "Conclusion: You would need a lot of luck to beat $M.";
	}
	else if( diff <= -3 )
	{
		msg = "Conclusion: You would need some luck to beat $N.";
	}
	else if( diff <= 2 )
	{
		msg = "Conclusion: It would be a very close fight.";
	}
	else if( diff <= 6 )
	{
		msg = "Conclusion: You shouldn't have a lot of trouble defeating $M.";
	}
	else if( diff <= 10 )
	{
		msg = "Conclusion: $N is no match for you.  You can easily beat $M.";
	}
	else
	{
		msg = "Conclusion: $E wouldn't last more than a few seconds against you.";
	}
	act( AT_WHITE, msg, ch, NULL, victim, TO_CHAR );

	return;

}

/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

CMDF( do_practice )
{
	char buf[MAX_STRING_LENGTH];
	char starrating[MAX_STRING_LENGTH];
	int sn;

	if( IS_NPC( ch ) )
		return;

	if( argument[0] == '\0' )
	{
		int col;
		short lasttype, cnt;

		col = cnt = 0;
		lasttype = SKILL_SPELL;
		set_pager_color( AT_MAGIC, ch );
		for( sn = 0; sn < top_sn; sn++ )
		{
			if( !skill_table[sn]->name )
				break;

			if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
				continue;

			if( strcmp( skill_table[sn]->name, "reserved" ) == 0 && ( IS_IMMORTAL( ch ) ) )
			{
				if( col % 3 != 0 )
					send_to_pager( "\r\n", ch );
				send_to_pager( "&G--------------------------------&g[&cSpells&g]&G---------------------------------\r\n", ch );
				col = 0;
			}
			if( skill_table[sn]->type != lasttype )
			{
				if( !cnt )
					send_to_pager( "                                &Y(&Onone&Y)\r\n", ch );
				else if( col % 3 != 0 )
					send_to_pager( "\r\n", ch );
				pager_printf( ch,
					"&G----------------------------------&g[&c%ss&g]&G-----------------------------------\r\n",
					skill_tname[skill_table[sn]->type] );
				col = cnt = 0;
			}
			lasttype = skill_table[sn]->type;

			if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
				continue;

			if( ch->pcdata->learned[sn] <= 0 && ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
				continue;

			if( ch->pcdata->learned[sn] == 0 && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
				continue;

			if( ch->pcdata->learned[sn] == 0 )
			{
				sprintf( starrating, "&B-----" );
			}
			else if( ch->pcdata->learned[sn] <= 20 )
			{
				sprintf( starrating, "&c*    " );
			}
			else if( ch->pcdata->learned[sn] <= 40 )
			{
				sprintf( starrating, "&c**   " );
			}
			else if( ch->pcdata->learned[sn] <= 60 )
			{
				sprintf( starrating, "&c***  " );
			}
			else if( ch->pcdata->learned[sn] <= 80 )
			{
				sprintf( starrating, "&c**** " );
			}
			else if( ch->pcdata->learned[sn] <= 99 )
			{
				sprintf( starrating, "&c*****" );
			}
			else if( ch->pcdata->learned[sn] >= 100 )
			{
				sprintf( starrating, "&C<>=<>" );
			}


			//         if ( skill_table[sn]->guild == 0 )
			//         {
			++cnt;
			pager_printf( ch, "&z%18s&W: &w%5s", skill_table[sn]->name, starrating );
			if( ++col % 3 == 0 )
				send_to_pager( "\r\n", ch );
			//         }
			/*
			 * else if ( skill_table[sn]->guild == 1 )
			 * {
			 * ++cnt;
			 * pager_printf( ch, "&z%18s&W: &w%-5s",
			 * skill_table[sn]->name, starrating );
			 * if ( ++col % 3 == 0 )
			 * send_to_pager( "\r\n", ch );
			 * }
			 */
		}

		if( col % 3 != 0 )
			send_to_pager( "\r\n", ch );

	}
	else
	{
		CHAR_DATA *mob;
		int adept;
		bool can_prac = true;

		if( !IS_AWAKE( ch ) )
		{
			send_to_char( "In your dreams, or what?\r\n", ch );
			return;
		}

		for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
		{
			if( IS_NPC( mob ) && xIS_SET( mob->act, ACT_PRACTICE ) )
				break;
		}

		if( !mob )
		{
			send_to_char( "You can't do that here.\r\n", ch );
			return;
		}


		sn = skill_lookup( argument );

		if( sn == -1 )
		{
			act( AT_TELL, "$n tells you 'I've never heard of that one...'", mob, NULL, ch, TO_VICT );
			return;
		}

		if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
		{
			act( AT_TELL, "$n tells you 'I cannot teach you that...'", mob, NULL, ch, TO_VICT );
			return;
		}


		if( skill_table[sn]->guild != ESPIONAGE_ABILITY )
		{

			if( can_prac && !IS_NPC( ch ) && ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
			{
				act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'", mob, NULL, ch, TO_VICT );
				return;
			}
		}
		if( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
		{
			act( AT_TELL, "$n tells you 'I do not know how to teach that.'", mob, NULL, ch, TO_VICT );
			return;
		}

		/*
		 * Skill requires a special teacher
		 */
		if( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
		{
			sprintf( buf, "%d", mob->pIndexData->vnum );
			if( !is_name( buf, skill_table[sn]->teachers ) )
			{
				act( AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT );
				return;
			}
		}
		else
		{
			act( AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT );
			return;
		}

		adept = 20;

		if( ch->gold < skill_table[sn]->min_level * 20 )
		{
			sprintf( buf, "$n tells you, 'I charge %d dollars to teach that. You don't have enough.'",
				skill_table[sn]->min_level * 20 );
			act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
			act( AT_TELL, "$n tells you 'You don't have enough money.'", mob, NULL, ch, TO_VICT );
			return;
		}

		if( ch->pcdata->learned[sn] >= adept )
		{
			sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'", skill_table[sn]->name );
			act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
			act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'", mob, NULL, ch, TO_VICT );
		}
		else
		{
			ch->gold -= skill_table[sn]->min_level * 20;
			ch->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
			act( AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name, TO_CHAR );
			act( AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name, TO_ROOM );
			if( ch->pcdata->learned[sn] >= adept )
			{
				ch->pcdata->learned[sn] = adept;
				act( AT_TELL, "$n tells you. 'You'll have to practice it on your own now...'", mob, NULL, ch, TO_VICT );
			}
		}
	}
	return;
}

CMDF( do_teach )
{
	char buf[MAX_STRING_LENGTH];
	int sn;
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg );

	if( argument[0] == '\0' )
	{
		send_to_char( "Teach who, what?\r\n", ch );
		return;
	}
	else
	{
		CHAR_DATA *victim;
		int adept;

		if( !IS_AWAKE( ch ) )
		{
			send_to_char( "In your dreams, or what?\r\n", ch );
			return;
		}

		if( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			send_to_char( "They don't seem to be here...\r\n", ch );
			return;
		}

		if( IS_NPC( victim ) )
		{
			send_to_char( "You can't teach that to them!\r\n", ch );
			return;
		}

		sn = skill_lookup( argument );

		if( sn == -1 )
		{
			act( AT_TELL, "You have no idea what that is.", victim, NULL, ch, TO_VICT );
			return;
		}

		if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
		{
			act( AT_TELL, "Thats just not going to happen.", victim, NULL, ch, TO_VICT );
			return;
		}

		if( victim->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
		{
			act( AT_TELL, "$n isn't ready to learn that yet.", victim, NULL, ch, TO_VICT );
			return;
		}

		if( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
		{
			act( AT_TELL, "You are unable to teach that skill.", victim, NULL, ch, TO_VICT );
			return;
		}

		adept = 20;

		if( victim->pcdata->learned[sn] >= adept )
		{
			act( AT_TELL, "$n must practice that on their own.", victim, NULL, ch, TO_VICT );
			return;
		}
		if( ch->pcdata->learned[sn] < 100 )
		{
			act( AT_TELL, "You must perfect that yourself before teaching others.", victim, NULL, ch, TO_VICT );
			return;
		}
		else
		{
			victim->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
			sprintf( buf, "You teach %s $T.", victim->name );
			act( AT_ACTION, buf, ch, NULL, skill_table[sn]->name, TO_CHAR );
			sprintf( buf, "%s teaches you $T.", ch->name );
			act( AT_ACTION, buf, victim, NULL, skill_table[sn]->name, TO_CHAR );
		}
	}
	return;
}


CMDF( do_wimpy )
{
	char arg[MAX_INPUT_LENGTH];
	int wimpy;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
		wimpy = ( int ) ch->max_hit / 5;
	else
		wimpy = atoi( arg );

	if( wimpy < 0 )
	{
		send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
		return;
	}

	if( wimpy > ch->max_hit )
	{
		send_to_char( "Such cowardice ill becomes you.\r\n", ch );
		return;
	}

	ch->wimpy = wimpy;
	ch_printf( ch, "Wimpy set to %d hit points.\r\n", wimpy );
	return;
}



CMDF( do_password )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char cEnd;

	if( IS_NPC( ch ) )
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;
	while( isspace( *argument ) )
		argument++;

	cEnd = ' ';
	if( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while( *argument != '\0' )
	{
		if( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	pArg = arg2;
	while( isspace( *argument ) )
		argument++;

	cEnd = ' ';
	if( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while( *argument != '\0' )
	{
		if( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: password <old> <new>.\r\n", ch );
		return;
	}

	if( strcmp( sha256_crypt( arg1 ), ch->pcdata->pwd ) )
	{
		WAIT_STATE( ch, ( PULSE_PER_SECOND * 10 ) );
		send_to_char( "Wrong password.  Wait 10 seconds.\r\n", ch );
		return;
	}

	if( strlen( arg2 ) < 5 )
	{
		send_to_char( "New password must be at least five characters long.\r\n", ch );
		return;
	}

	pwdnew = sha256_crypt( arg2 );   /* SHA-256 Encryption */

	DISPOSE( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
		save_char_obj( ch );
	send_to_char( "Ok.\r\n", ch );
	return;
}



CMDF( do_socials )
{
	int iHash;
	int col = 0;
	SOCIALTYPE *social;

	set_pager_color( AT_PLAIN, ch );
	for( iHash = 0; iHash < 27; iHash++ )
		for( social = social_index[iHash]; social; social = social->next )
		{
			pager_printf( ch, "%-12s", social->name );
			if( ++col % 6 == 0 )
				send_to_pager( "\r\n", ch );
		}

	if( col % 6 != 0 )
		send_to_pager( "\r\n", ch );
	return;
}

CMDF( do_commands )
{
	int col;
	bool found;
	bool nCode = false;
	bool nHelp = false;
	char ccolor[100];
	int hash;
	CMDTYPE *command;

	col = 0;
	set_pager_color( AT_PLAIN, ch );
	if( argument[0] == '\0' )
	{
		for( hash = 0; hash < 126; hash++ )
			for( command = command_hash[hash]; command; command = command->next )
				if( command->level < LEVEL_HERO
					&& command->level <= get_trust( ch )
					&& ( command->name[0] != 'm' || command->name[1] != 'p' ) && command->cshow == 0 )
				{
					nCode = false;
					nHelp = false;
					if( command->do_fun == skill_notfound )
						nCode = true;
					if( !get_help( ch, command->name ) )
						nHelp = true;
					if( nCode && nHelp )
						mudstrlcpy( ccolor, "&b", MSL );
					else if( nCode )
						mudstrlcpy( ccolor, "&z", MSL );
					else if( nHelp )
						mudstrlcpy( ccolor, "&B", MSL );
					else
						mudstrlcpy( ccolor, "&w", MSL );
					pager_printf( ch, "%s%-12s", !IS_IMMORTAL( ch ) ? "&w" : ccolor, command->name );
					if( ++col % 6 == 0 )
						send_to_pager( "\r\n", ch );
				}
		if( col % 6 != 0 )
			send_to_pager( "\r\n", ch );
	}
	else
	{
		found = false;
		for( hash = 0; hash < 126; hash++ )
			for( command = command_hash[hash]; command; command = command->next )
				if( command->level < LEVEL_HERO
					&& command->level <= get_trust( ch )
					&& !str_prefix( argument, command->name ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
				{
					nCode = false;
					nHelp = false;
					if( command->do_fun == skill_notfound )
						nCode = true;
					if( !get_help( ch, command->name ) )
						nHelp = true;
					if( nCode && nHelp )
						mudstrlcpy( ccolor, "&b", MSL );
					else if( nCode )
						mudstrlcpy( ccolor, "&z", MSL );
					else if( nHelp )
						mudstrlcpy( ccolor, "&B", MSL );
					else
						mudstrlcpy( ccolor, "&w", MSL );
					pager_printf( ch, "%s%-12s", !IS_IMMORTAL( ch ) ? "&w" : ccolor, command->name );
					found = true;
					if( ++col % 6 == 0 )
						send_to_pager( "\r\n", ch );
				}

		if( col % 6 != 0 )
			send_to_pager( "\r\n", ch );

		if( !found )
			ch_printf( ch, "No command found under %s.\r\n", argument );
	}
	return;
}

CMDF( do_channels )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
		{
			send_to_char( "You are silenced.\r\n", ch );
			return;
		}

		ch_printf( ch, "\r\n&c%s&b\r\n", color_align( "OOC Channels", 70, ALIGN_CENTER ) );
		ch_printf( ch, "&C%s\r\n", color_align( "========================================", 70, ALIGN_CENTER ) );

		mudstrlcpy( buf, ( !IS_SET( ch->deaf, CHANNEL_OOC ) ? "&G+&BOOC" : "&R-&Booc" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_GOCIAL ) ? "     &G+&BGOCIAL" : "     &R-&Bgocial" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_i104 ) ? "     &G+&BATG" : "     &R-&Batg" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_QUOTE ) ? "     &G+&BQUOTE" : "     &R-&Bquote" ), MSL );
		if( get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_AUCTION ) ? "     &G+&BAUCTION" : "     &R-&Bauction" ), MSL );

		ch_printf( ch, "%s\r\n", color_align( buf, 70, ALIGN_CENTER ) );

		ch_printf( ch, "\r\n&c%s&b\r\n", color_align( "IC Channels", 70, ALIGN_CENTER ) );
		ch_printf( ch, "&C%s\r\n", color_align( "========================================", 70, ALIGN_CENTER ) );

		mudstrlcpy( buf, ( !IS_SET( ch->deaf, CHANNEL_CHAT ) ? "&G+&BCHAT" : "&R-&Bchat" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_TELLS ) ? "     &G+&BTELLS" : "     &R-&Btells" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_WARTALK ) ? "     &G+&BGMOTE" : "     &R-&Bgmote" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_ASK ) ? "     &G+&BASK" : "     &R-&Bask" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_SHOUT ) ? "     &G+&BSHOUT" : "     &R-&Bshout" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_YELL ) ? "     &G+&BYELL" : "     &R-&Byell" ), MSL );

		if( !IS_NPC( ch ) && ch->pcdata->clan )
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_CLAN ) ? "     &G+&BGTM" : "     &R-&Bgtm" ), MSL );

		ch_printf( ch, "%s\r\n", color_align( buf, 70, ALIGN_CENTER ) );

		ch_printf( ch, "\r\n&c%s&b\r\n", color_align( "Misc Channels", 70, ALIGN_CENTER ) );
		ch_printf( ch, "&C%s\r\n", color_align( "========================================", 70, ALIGN_CENTER ) );

		mudstrlcpy( buf, ( !IS_SET( ch->deaf, CHANNEL_ARENA ) ? "&G+&BARENA" : "&R-&Barena" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_INFO ) ? "     &G+&BINFO" : "     &R-&Binfo" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_RANK ) ? "     &G+&BRANK" : "     &R-&Brank" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_FREEZE ) ? "     &G+&BFREEZE" : "     &R-&Bfreeze" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_MUSIC ) ? "     &G+&BMUSIC" : "     &R-&Bmusic" ), MSL );
		mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_NEWBIE ) ? "     &G+&BNEWBIE" : "     &R-&Bnewbie" ), MSL );

		ch_printf( ch, "%s\r\n", color_align( buf, 70, ALIGN_CENTER ) );


		if( IS_IMMORTAL( ch ) )
		{
			ch_printf( ch, "\r\n&c%s&b\r\n", color_align( "Imm Channels", 70, ALIGN_CENTER ) );
			ch_printf( ch, "&C%s\r\n", color_align( "========================================", 70, ALIGN_CENTER ) );

			mudstrlcpy( buf, ( !IS_SET( ch->deaf, CHANNEL_MONITOR ) ? "&G+&BMONITOR" : "&R-&Bmonitor" ), MSL );
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_IMMTALK ) ? "     &G+&BIMMTALK" : "     &R-&Bimmtalk" ), MSL );
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_LOG ) ? "     &G+&BLOG" : "     &R-&Blog" ), MSL );
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_BUILD ) ? "     &G+&BBUILD" : "     &R-&Bbuild" ), MSL );
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_COMM ) ? "     &G+&BCOMM" : "     &R-&Bcomm" ), MSL );
			mudstrlcat( buf, ( !IS_SET( ch->deaf, CHANNEL_BUG ) ? "     &G+&BBUG" : "     &R-&Bbug" ), MSL );

			ch_printf( ch, "%s\r\n", color_align( buf, 70, ALIGN_CENTER ) );
		}

		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			ch_printf( ch, "\r\n&c%s&b\r\n", color_align( "PK Channels", 70, ALIGN_CENTER ) );
			ch_printf( ch, "&C%s\r\n", color_align( "========================================", 70, ALIGN_CENTER ) );
			mudstrlcpy( buf, ( !IS_SET( ch->deaf, CHANNEL_PEEKAY ) ? "&G+&BPKTALK" : "&R-&Bpktalk" ), MSL );
			ch_printf( ch, "%s\r\n", color_align( buf, 70, ALIGN_CENTER ) );
		}
	}
	else
	{
		bool fClear;
		bool ClearAll;
		int bit;

		bit = 0;
		ClearAll = false;

		if( arg[0] == '+' )
			fClear = true;
		else if( arg[0] == '-' )
			fClear = false;
		else
		{
			send_to_char( "&BChannels &R-&Bchannel or &G+&Bchannel?\r\n", ch );
			return;
		}

		if( !str_cmp( arg + 1, "auction" ) )
			bit = CHANNEL_AUCTION;
		else if( !str_cmp( arg + 1, "chat" ) )
			bit = CHANNEL_CHAT;
		else if( !str_cmp( arg + 1, "ooc" ) )
			bit = CHANNEL_OOC;
		else if( !str_cmp( arg + 1, "gtm" ) )
			bit = CHANNEL_CLAN;
		else if( !str_cmp( arg + 1, "rank" ) )
			bit = CHANNEL_RANK;
		else if( !str_cmp( arg + 1, "info" ) )
			bit = CHANNEL_INFO;
		else if( !str_cmp( arg + 1, "tells" ) )
			bit = CHANNEL_TELLS;
		else if( !str_cmp( arg + 1, "immtalk" ) )
			bit = CHANNEL_IMMTALK;
		else if( !str_cmp( arg + 1, "log" ) )
			bit = CHANNEL_LOG;
		else if( !str_cmp( arg + 1, "bug" ) )
			bit = CHANNEL_BUG;
		else if( !str_cmp( arg + 1, "build" ) )
			bit = CHANNEL_BUILD;
		else if( !str_cmp( arg + 1, "monitor" ) )
			bit = CHANNEL_MONITOR;
		else if( !str_cmp( arg + 1, "newbie" ) )
			bit = CHANNEL_NEWBIE;
		else if( !str_cmp( arg + 1, "music" ) )
			bit = CHANNEL_MUSIC;
		else if( !str_cmp( arg + 1, "ask" ) )
			bit = CHANNEL_ASK;
		else if( !str_cmp( arg + 1, "shout" ) )
			bit = CHANNEL_SHOUT;
		else if( !str_cmp( arg + 1, "yell" ) )
			bit = CHANNEL_YELL;
		else if( !str_cmp( arg + 1, "comm" ) )
			bit = CHANNEL_COMM;
		else if( !str_cmp( arg + 1, "order" ) )
			bit = CHANNEL_ORDER;
		else if( !str_cmp( arg + 1, "gmote" ) )
			bit = CHANNEL_WARTALK;
		else if( !str_cmp( arg + 1, "atg" ) )
			bit = CHANNEL_i104;
		else if( !str_cmp( arg + 1, "quote" ) )
			bit = CHANNEL_QUOTE;
		else if( !str_cmp( arg + 1, "gocial" ) )
			bit = CHANNEL_GOCIAL;
		else if( !str_cmp( arg + 1, "arena" ) )
			bit = CHANNEL_ARENA;
		else if( !str_cmp( arg + 1, "freeze" ) )
			bit = CHANNEL_FREEZE;
		else if( !str_cmp( arg + 1, "pktalk" ) )
			bit = CHANNEL_PEEKAY;
		else if( !str_cmp( arg + 1, "all" ) )
			ClearAll = true;
		else
		{
			send_to_char( "Set or clear which channel?\r\n", ch );
			return;
		}

		if( ( fClear ) && ( ClearAll ) )
		{
			REMOVE_BIT( ch->deaf, CHANNEL_AUCTION );
			REMOVE_BIT( ch->deaf, CHANNEL_CHAT );
			REMOVE_BIT( ch->deaf, CHANNEL_INFO );
			REMOVE_BIT( ch->deaf, CHANNEL_MUSIC );
			REMOVE_BIT( ch->deaf, CHANNEL_ASK );
			REMOVE_BIT( ch->deaf, CHANNEL_SHOUT );
			REMOVE_BIT( ch->deaf, CHANNEL_YELL );
			REMOVE_BIT( ch->deaf, CHANNEL_OOC );
			REMOVE_BIT( ch->deaf, CHANNEL_QUOTE );
			REMOVE_BIT( ch->deaf, CHANNEL_WARTALK );
			REMOVE_BIT( ch->deaf, CHANNEL_NEWBIE );
			REMOVE_BIT( ch->deaf, CHANNEL_GOCIAL );
			REMOVE_BIT( ch->deaf, CHANNEL_i104 );
			REMOVE_BIT( ch->deaf, CHANNEL_ARENA );
			REMOVE_BIT( ch->deaf, CHANNEL_FREEZE );
			REMOVE_BIT( ch->deaf, CHANNEL_RANK );
			REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
			if( xIS_SET( ch->act, PLR_PKER ) )
				REMOVE_BIT( ch->deaf, CHANNEL_PEEKAY );
			if( !IS_NPC( ch ) && ch->pcdata->clan )
				REMOVE_BIT( ch->deaf, CHANNEL_CLAN );

			if( ch->top_level >= sysdata.log_level )
			{
				REMOVE_BIT( ch->deaf, CHANNEL_COMM );
				REMOVE_BIT( ch->deaf, CHANNEL_LOG );
				REMOVE_BIT( ch->deaf, CHANNEL_MONITOR );
				REMOVE_BIT( ch->deaf, CHANNEL_IMMTALK );
				REMOVE_BIT( ch->deaf, CHANNEL_BUILD );
				REMOVE_BIT( ch->deaf, CHANNEL_BUG );
			}

		}
		else if( ( !fClear ) && ( ClearAll ) )
		{
			SET_BIT( ch->deaf, CHANNEL_AUCTION );
			SET_BIT( ch->deaf, CHANNEL_CHAT );
			SET_BIT( ch->deaf, CHANNEL_INFO );
			SET_BIT( ch->deaf, CHANNEL_MUSIC );
			SET_BIT( ch->deaf, CHANNEL_ASK );
			SET_BIT( ch->deaf, CHANNEL_SHOUT );
			SET_BIT( ch->deaf, CHANNEL_YELL );
			SET_BIT( ch->deaf, CHANNEL_WARTALK );
			SET_BIT( ch->deaf, CHANNEL_GOCIAL );
			SET_BIT( ch->deaf, CHANNEL_ARENA );
			SET_BIT( ch->deaf, CHANNEL_FREEZE );
			SET_BIT( ch->deaf, CHANNEL_i104 );
			SET_BIT( ch->deaf, CHANNEL_OOC );
			SET_BIT( ch->deaf, CHANNEL_RANK );
			SET_BIT( ch->deaf, CHANNEL_QUOTE );
			SET_BIT( ch->deaf, CHANNEL_TELLS );
			SET_BIT( ch->deaf, CHANNEL_CLAN );
			SET_BIT( ch->deaf, CHANNEL_NEWBIE );
			if( xIS_SET( ch->act, PLR_PKER ) )
				SET_BIT( ch->deaf, CHANNEL_PEEKAY );

			if( ch->top_level >= sysdata.log_level )
			{
				SET_BIT( ch->deaf, CHANNEL_COMM );
				SET_BIT( ch->deaf, CHANNEL_LOG );
				SET_BIT( ch->deaf, CHANNEL_MONITOR );
				SET_BIT( ch->deaf, CHANNEL_IMMTALK );
				SET_BIT( ch->deaf, CHANNEL_BUILD );
				SET_BIT( ch->deaf, CHANNEL_BUG );
			}
		}
		else if( fClear )
			REMOVE_BIT( ch->deaf, bit );
		else
			SET_BIT( ch->deaf, bit );

		send_to_char( "Ok.\r\n", ch );
	}

	return;
}


/*
 * display WIZLIST file						-Thoric
 */
CMDF( do_wizlist )
{
	set_pager_color( AT_IMMORT, ch );
	show_file( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 */
CMDF( do_config )
{
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
		return;

	one_argument( argument, arg );

	set_char_color( AT_WHITE, ch );
	if( arg[0] == '\0' )
	{
		send_to_char( "[ Keyword  ] Option\r\n", ch );
		/*
			send_to_char(  xIS_SET(ch->act, PLR_FLEE)
				? "[+FLEE     ] You flee if you get attacked.\r\n"
				: "[-flee     ] You fight back if you get attacked.\r\n"
				, ch );
		*/
		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_NORECALL )
			? "[+NORECALL ] You fight to the death, link-dead or not.\r\n"
			: "[-norecall ] You try to recall if fighting link-dead.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_AUTOEXIT )
			? "[+AUTOEXIT ] You automatically see exits.\r\n"
			: "[-autoexit ] You don't automatically see exits.\r\n", ch );

		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_CANLOOT )
			? "[+LOOTABLE ] Your corpse is lootable by others.\r\n"
			: "[-lootable ] Your corpse isn't lootable by others.\r\n", ch );


		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_STUN )
			? "[+STUN     ] You stun players instead of kill on finishing blow.\r\n"
			: "[-stun     ] You kill players instead of stun on finishing blow.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_AUTOLOOT )
			? "[+AUTOLOOT ] You automatically loot corpses.\r\n"
			: "[-autoloot ] You don't automatically loot corpses.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_AUTOSAC )
			? "[+AUTOSAC  ] You automatically sacrifice corpses.\r\n"
			: "[-autosac  ] You don't automatically sacrifice corpses.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_AUTOGOLD )
			? "[+AUTOCRED ] You automatically split dollars from kills in groups.\r\n"
			: "[-autocred ] You don't automatically split dollars from kills in groups.\r\n", ch );

		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_AUTOWHO )
			? "[+AUTOWHO  ] You auto 'Who' at login.\r\n"
			: "[-autowho  ] You don't automatically who at login.\r\n", ch );

		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_GAG )
			? "[+GAG      ] You see only necessary battle text.\r\n"
			: "[-gag      ] You see full battle text.\r\n", ch );

		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON )
			? "[+PAGER    ] Long output is page-paused.\r\n"
			: "[-pager    ] Long output scrolls to the end.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_BLANK )
			? "[+BLANK    ] You have a blank line before your prompt.\r\n"
			: "[-blank    ] You have no blank line before your prompt.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_BRIEF )
			? "[+BRIEF    ] You see brief descriptions.\r\n" : "[-brief    ] You see long descriptions.\r\n", ch );
		/*
			send_to_char(  xIS_SET(ch->act, PLR_COMBINE)
				? "[+COMBINE  ] You see object lists in combined format.\r\n"
				: "[-combine  ] You see object lists in single format.\r\n"
				, ch );
		*/
		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO )
			? "[+NOINTRO  ] You don't see the ascii intro screen on login.\r\n"
			: "[-nointro  ] You see the ascii intro screen on login.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_PROMPT )
			? "[+PROMPT   ] You have a prompt.\r\n" : "[-prompt   ] You don't have a prompt.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_TELNET_GA )
			? "[+TELNETGA ] You receive a telnet GA sequence.\r\n"
			: "[-telnetga ] You don't receive a telnet GA sequence.\r\n", ch );
		/*
			send_to_char(  xIS_SET(ch->act, PLR_ANSI)
				? "[+ANSI     ] You receive ANSI color sequences.\r\n"
				: "[-ansi     ] You don't receive receive ANSI colors.\r\n"
				, ch );

			send_to_char(  xIS_SET(ch->act, PLR_SOUND)
				? "[+SOUND     ] You have MSP support.\r\n"
				: "[-sound     ] You don't have MSP support.\r\n"
				, ch );


			send_to_char(  xIS_SET(ch->act, PLR_SHOVEDRAG)
				  ? "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\r\n"
				  : "[-shovedrag] You'd rather not be shoved or dragged around.\r\n"
				  , ch );
		*/
		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON )
			? "[+NOSUMMON ] You do not allow other players to summon you.\r\n"
			: "[-nosummon ] You allow other players to summon you.\r\n", ch );

		send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_AUTOSCAN )
			? "[+AUTOSCAN ] You automatically scan in the direction you're heading.\r\n"
			: "[-autoscan ] You don't scan automatically in the direction you're headed.\r\n", ch );

		if( IS_IMMORTAL( ch ) )
			send_to_char( xIS_SET( ch->act, PLR_ROOMVNUM )
				? "[+VNUM     ] You can see the VNUM of a room.\r\n"
				: "[-vnum     ] You do not see the VNUM of a room.\r\n", ch );
		/*
			if ( IS_IMMORTAL( ch ) )
			  send_to_char(  xIS_SET(ch->act, PLR_AUTOMAP)
				  ? "[+MAP      ] You can see the MAP of a room.\r\n"
				  : "[-map      ] You do not see the MAP of a room.\r\n"
				  , ch );
		*/
		if( IS_IMMORTAL( ch ) )   /* Added 10/16 by Kuran of SWR */
			send_to_char( IS_SET( ch->pcdata->flags, PCFLAG_ROOM )
				? "[+ROOMFLAGS] You will see room flags.\r\n" : "[-roomflags] You will not see room flags.\r\n", ch );

		send_to_char( xIS_SET( ch->act, PLR_SILENCE ) ? "[+SILENCE  ] You are silenced.\r\n" : "", ch );

		send_to_char( !xIS_SET( ch->act, PLR_NO_EMOTE ) ? "" : "[-emote    ] You can't emote.\r\n", ch );

		send_to_char( !xIS_SET( ch->act, PLR_NO_TELL ) ? "" : "[-tell     ] You can't use 'tell'.\r\n", ch );

		send_to_char( !xIS_SET( ch->act, PLR_LITTERBUG )
			? "" : "[-litter  ] A convicted litterbug. You cannot drop anything.\r\n", ch );
	}
	else
	{
		bool fSet;
		int bit = 0;

		if( arg[0] == '+' )
			fSet = true;
		else if( arg[0] == '-' )
			fSet = false;
		else
		{
			send_to_char( "Config -option or +option?\r\n", ch );
			return;
		}

		if( !str_prefix( arg + 1, "autoexit" ) )
			bit = PLR_AUTOEXIT;
		else if( !str_prefix( arg + 1, "autoloot" ) )
			bit = PLR_AUTOLOOT;
		else if( !str_prefix( arg + 1, "autosac" ) )
			bit = PLR_AUTOSAC;
		else if( !str_prefix( arg + 1, "autocred" ) )
			bit = PLR_AUTOGOLD;
		else if( !str_prefix( arg + 1, "blank" ) )
			bit = PLR_BLANK;
		else if( !str_prefix( arg + 1, "brief" ) )
			bit = PLR_BRIEF;
		//  else if ( !str_prefix( arg+1, "combine"  ) ) bit = PLR_COMBINE;
		else if( !str_prefix( arg + 1, "prompt" ) )
			bit = PLR_PROMPT;
		else if( !str_prefix( arg + 1, "telnetga" ) )
			bit = PLR_TELNET_GA;
		//  else if ( !str_prefix( arg+1, "ansi"     ) ) bit = PLR_ANSI;
		//  else if ( !str_prefix( arg+1, "sound"      ) ) bit = PLR_SOUND;
		//  else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE;
		else if( !str_prefix( arg + 1, "nice" ) )
			bit = PLR_NICE;
		//  else if ( !str_prefix( arg+1, "shovedrag") ) bit = PLR_SHOVEDRAG;
		else if( IS_IMMORTAL( ch ) && !str_prefix( arg + 1, "vnum" ) )
			bit = PLR_ROOMVNUM;
		else if( IS_IMMORTAL( ch ) && !str_prefix( arg + 1, "map" ) )
			bit = PLR_AUTOMAP; /* maps */

		if( bit )
		{

			if( fSet )
				xSET_BIT( ch->act, bit );
			else
				xREMOVE_BIT( ch->act, bit );
			send_to_char( "Ok.\r\n", ch );
			return;
		}
		else
		{
			if( !str_prefix( arg + 1, "norecall" ) )
				bit = PCFLAG_NORECALL;
			else if( !str_prefix( arg + 1, "nointro" ) )
				bit = PCFLAG_NOINTRO;
			else if( !str_prefix( arg + 1, "nosummon" ) )
				bit = PCFLAG_NOSUMMON;
			else if( !str_prefix( arg + 1, "lootable" ) )
				bit = PCFLAG_CANLOOT;
			else if( !str_prefix( arg + 1, "stun" ) )
				bit = PCFLAG_STUN;
			else if( !str_prefix( arg + 1, "autoscan" ) )
				bit = PCFLAG_AUTOSCAN;
			else if( !str_prefix( arg + 1, "autowho" ) )
				bit = PCFLAG_AUTOWHO;
			else if( !str_prefix( arg + 1, "gag" ) )
				bit = PCFLAG_GAG;
			else if( !str_prefix( arg + 1, "pager" ) )
				bit = PCFLAG_PAGERON;
			else if( !str_prefix( arg + 1, "roomflags" ) && ( IS_IMMORTAL( ch ) ) )
				bit = PCFLAG_ROOM;
			else
			{
				send_to_char( "Config which option?\r\n", ch );
				return;
			}

			if( fSet )
				SET_BIT( ch->pcdata->flags, bit );
			else
				REMOVE_BIT( ch->pcdata->flags, bit );

			send_to_char( "Ok.\r\n", ch );
			return;
		}
	}

	return;
}


CMDF( do_credits )
{
	do_help( ch, "credits" );
}


extern int top_area;

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;

	iAreaHalf = (top_area + 1) / 2;
	pArea1    = first_area;
	pArea2    = first_area;
	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	{
	ch_printf( ch, "%-39s%-39s\r\n",
		pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
		pArea2 = pArea2->next;
	}

	return;
}
*/

/*
 * New do_areas with soft/hard level ranges
 */

CMDF( do_areas )
{
	AREA_DATA *pArea;

	set_pager_color( AT_PLAIN, ch );
	send_to_pager( "\r\n   Author    |             Area                     | Recommended |  Enforced\r\n", ch );
	send_to_pager( "-------------+--------------------------------------+-------------+-----------\r\n", ch );

	for( pArea = first_area; pArea; pArea = pArea->next )
		pager_printf( ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \r\n",
			pArea->author, pArea->name, pArea->low_soft_range,
			pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
	return;
}

CMDF( do_afk )
{
	char buf[MAX_STRING_LENGTH], msg[MSL];

	if( IS_NPC( ch ) )
		return;

	if( get_timer( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL( ch ) )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			set_char_color( AT_RED, ch );
			send_to_char( "Wait until you cool down to go AFK!\r\n", ch );
			return;
		}
	}

	if( xIS_SET( ch->act, PLR_AFK ) )
	{
		xREMOVE_BIT( ch->act, PLR_AFK );
		send_to_char( "You are no longer afk.\r\n", ch );
		act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
		if( ch->pcdata->tells >= 1 )
		{
			ch_printf( ch, "\n&GYou have &g%d&G messages waiting on your CDI.\r&GType &g'&Gcheck&g'&G, to read them.\r\n",
				ch->pcdata->tells );
		}
		return;
	}
	else
	{
		xSET_BIT( ch->act, PLR_AFK );
		send_to_char( "You are now afk.\r\n", ch );
		act( AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_ROOM );
		//        return;
	}

	if( ch->pcdata->afkmess )
		DISPOSE( ch->pcdata->afkmess );

	if( argument[0] == '\0' )
	{
		send_to_char( "Normal AFK message set.\r\n", ch );
		sprintf( buf, "&W[&zA&WF&zK&W] %s points at the pretty AFK Tag.", ch->name );
		ch->pcdata->afkmess = str_dup( buf );
		return;
	}

	if( !str_cmp( argument, "brb" ) )
	{
		send_to_char( "BRB Afk message set.\r\n", ch );
		sprintf( buf, "&W[&zA&WF&zK&W] %s will BrB!", ch->name );
		ch->pcdata->afkmess = str_dup( buf );
		return;
	}

	if( !str_cmp( argument, "boo" ) )
	{
		send_to_char( "Boo message set.\r\n", ch );
		sprintf( buf, "&W[&zA&WF&zK&W] %s is currently busy with %s boo!", ch->name,
			ch->sex == SEX_MALE ? "his" : ch->sex == SEX_FEMALE ? "her" : "its" );
		ch->pcdata->afkmess = str_dup( buf );
		return;
	}

	if( !str_cmp( argument, "dog" ) )
	{
		send_to_char( "Dog AFK message set.\r\n", ch );
		sprintf( buf, "&W[&zA&WF&zK&W] %s is walking the dog!", ch->name );
		ch->pcdata->afkmess = str_dup( buf );
		return;
	}

	if( !str_cmp( argument, "food" ) )
	{
		send_to_char( "Eating messege set.\r\n", ch );
		sprintf( buf, "&W[&zA&WF&zK&W] %s is currently enjoying food.", ch->name );
		ch->pcdata->afkmess = str_dup( buf );
		return;
	}

	mudstrlcpy( msg, argument, MSL );

	if( strlen_color( msg ) > 50 )
		msg[50] = '\0';

	smash_tilde( msg );
	sprintf( buf, "&W[&zA&WF&zK&W] %s", msg );
	ch->pcdata->afkmess = str_dup( buf );
	return;

}

#define ALL_ABIL -1
CMDF( do_slist )
{
	int sn, i, cnt;
	char skn[MAX_INPUT_LENGTH];
	int lowlev, hilev;
	int col = 0;
	int ability;
	int pClass;

	if( IS_NPC( ch ) )
		return;

	lowlev = 1;
	hilev = 1000;
	cnt = 0;

	if( argument[0] == '\0' )
	{
		send_to_char( "\r\n&RSyntax&r: &PSlist &p<&PType&p>\r\n", ch );
		send_to_char
		( "&RType&r: &pcombat&P,&p piloting&P,&p engineering&P,&p smuggling&P,&p bounty&P,\r\n &p diplomacy&P,&p leadership&P,&p espionage&P,&p all.\r\n",
			ch );
		return;
	}

	if( !str_cmp( argument, "combat" ) )
		pClass = COMBAT_ABILITY;
	else if( !str_cmp( argument, "piloting" ) )
		pClass = PILOTING_ABILITY;
	else if( !str_cmp( argument, "engineering" ) )
		pClass = ENGINEERING_ABILITY;
	else if( !str_cmp( argument, "smuggling" ) )
		pClass = SMUGGLING_ABILITY;
	else if( !str_cmp( argument, "bounty" ) )
		pClass = HUNTING_ABILITY;
	else if( !str_cmp( argument, "diplomacy" ) )
		pClass = DIPLOMACY_ABILITY;
	else if( !str_cmp( argument, "leadership" ) )
		pClass = LEADERSHIP_ABILITY;
	else if( !str_cmp( argument, "espionage" ) )
		pClass = ESPIONAGE_ABILITY;
	else if( !str_cmp( argument, "all" ) )
		pClass = 20;
	else
	{
		do_slist( ch, "" );
		return;
	}

	if( ( lowlev < 1 ) || ( lowlev > LEVEL_STAFF ) )
		lowlev = 1;

	if( lowlev > hilev )
		lowlev = hilev;

	set_pager_color( AT_MAGIC, ch );
	send_to_pager( "\r\n&R               ---------------&P[&pSkill List&P]&R---------------\r\n", ch );

	for( ability = -1; ability < MAX_ABILITY; ability++ )
	{

		if( pClass != MAX_ABILITY )
		{
			if( pClass != ALL_ABIL && ability != pClass )
				continue;
		}
		if( ability >= 0 )
			sprintf( skn, "&Y[&O>&R%s&O<&Y]", ability_name[ability] );
		else
			sprintf( skn, " " );

		send_to_pager( "\r\n", ch );
		send_to_pager( color_align( skn, 74, ALIGN_CENTER ), ch );
		send_to_pager( "\r\n", ch );
		for( i = lowlev; i <= hilev; i++ )
		{
			for( sn = 0; sn < top_sn; sn++ )
			{
				if( !skill_table[sn]->name )
					break;

				if( skill_table[sn]->guild != ability )
					continue;

				if( skill_table[sn]->guild == -1 )
					continue;

				if( ch->pcdata->learned[sn] == 0 && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
					continue;

				if( i == skill_table[sn]->min_level )
				{
					pager_printf( ch, "&C[%s%-18.18s &B%3d&C] ", !IS_IMMORTAL( ch ) ? "&c"
						: !get_help( ch, skill_table[sn]->name ) ? "&B" : "&c", capitalize( skill_table[sn]->name ), i );
					++cnt;
					if( ++col == 3 )
					{
						pager_printf( ch, "\r\n" );
						col = 0;
					}
				}
			}
		}
		if( col != 0 )
		{
			pager_printf( ch, "\r\n" );
			col = 0;
		}
	}
	if( cnt )
		pager_printf( ch, "\r\n               &R---------------&P[&pSkills&P:&p %3d&P]&R--------------\r\n", cnt );
	else
		send_to_char( "None found.\r\n", ch );

	return;
}

#undef ALL_ABIL

CMDF( do_whois )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char lev[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	bool ch_comlink = false;
	OBJ_DATA *obj;

	buf[0] = '\0';

	if( IS_NPC( ch ) )
		return;

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

	if( argument[0] == '\0' )
	{
		send_to_char( "You must input the name of a player online.\r\n", ch );
		return;
	}

	strcat( buf, "0." );
	strcat( buf, argument );
	if( ( ( victim = get_char_world( ch, buf ) ) == NULL ) || ( !can_see( ch, victim ) ) )
	{
		send_to_char( "\r\n&pChecking Name in Database&P...\r\n&P...\r\n&P...&pNo Record Online.", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "That's not a player!\r\n", ch );
		return;
	}

	send_to_char( "\r\n&pChecking Name in CDI Database&P...\r\n&P...\r\n&P...&pFound!\r\n", ch );
	ch_printf( ch, "\r\n&CName&c:&z %-13s", victim->name );

	ch_printf( ch, "   &CDating&c:&z %-10s", victim->pcdata->spouse ? victim->pcdata->spouse : "Single..." );
	/*
	else if ( victim->pcdata->preengaged == true )
	  {
		ch_printf(ch, "   &CFiancee&c:&z %-10s", victim->pcdata->tspouse );
	  }
	else if ( victim->pcdata->married == true )
	  {
		ch_printf(ch, "   &CSpouse&c:&z %-10s", victim->pcdata->tspouse );
	  }
	else
	*/
	{
		send_to_char( "", ch );
	}

	if( IS_IMMORTAL( victim ) )
		sprintf( lev, "%d", victim->top_level );
	else
		sprintf( lev, "%d", victim->skill_level[COMBAT_ABILITY] + victim->skill_level[HUNTING_ABILITY] );

	ch_printf( ch, "\r\n&CAge&c:&z %d              &CHighest Level&c:&z %-4s", get_age( victim ), lev );
	send_to_char( "\r\n&B----------------------------------------------------\r\n", ch );
	ch_printf( ch, "&CRace&c:&z %-9s          &CSex&c:&z %-6s\r\n",
		pc_race[victim->race],
		victim->sex == SEX_MALE ? "&BMale" : victim->sex == SEX_FEMALE ? "&PFemale" : "&WNonbinary" );

	ch_printf( ch, "&CBuild&c:&z %-9s         &CEye Colour&c:&z %-12s\r\n",
		build_list[victim->pcdata->build], eye_list[victim->pcdata->eye] );

	ch_printf( ch, "&CHair&c:&z %-17s  &CHighlights&c:&z %-17s\r\n",
		hair_list[victim->pcdata->hair], highlight_list[victim->pcdata->highlight] );

	if( xIS_SET( victim->act, PLR_PKER ) )
	{
		send_to_char( "&CPK&c:&z Yes", ch );
	}
	else if( !xIS_SET( victim->act, PLR_PKER ) )
	{
		send_to_char( "&CPK&c:&z No ", ch );
	}

	ch_printf( ch, "\r\n&CBasic&c: &CPkills&c:&z %-4d      &CDeaths&c:&z %d\r\n",
		victim->pcdata->pkills, victim->pcdata->pdeaths );

	ch_printf( ch, "&CArena&c: &CPkills&c:&z %-4d      &CDeaths&c:&z %d\r\n",
		victim->pcdata->apkills, victim->pcdata->apdeaths );

	ch_printf( ch, "&CF-Tag&c: &CHFroze&c:&z %-4d      &CBFroze&c:&z %d\r\n",
		victim->pcdata->hasfroze, victim->pcdata->beenfroze );

	if( victim->pcdata->clan )
	{
		if( victim->pcdata->clan->clan_type == CLAN_CRIME )
			send_to_char( "\r\n&CClan&c:&z ", ch );
		else if( victim->pcdata->clan->clan_type == CLAN_GUILD )
			send_to_char( "\r\n&CGuild&c:&z ", ch );
		else
			send_to_char( "\r\n&CClan&c:&z ", ch );
		send_to_char( victim->pcdata->clan->name, ch );
	}
	send_to_char( "\r\n", ch );
	if( victim->pcdata->bio && victim->pcdata->bio[0] != '\0' )
		ch_printf( ch, "\r\n&B----------------------------------------------------\r\n&CBio&c:&z\r\n%s", victim->pcdata->bio );
	send_to_char( "\r\n&B----------------------------------------------------\r\n", ch );

	/*  if (IS_IMMORTAL(ch))
		ch_printf(ch, "\r\n&CIn room&c:&z %d.\r\n",
		victim->in_room->vnum);
	  else
		ch_printf(ch, ".\r\n");
	*/

	if( victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0' )
		ch_printf( ch, "%s's homepage can be found at %s.\r\n", victim->name, victim->pcdata->homepage );

	if( IS_IMMORTAL( ch ) )
	{
		send_to_char( "----------------------------------------------------\r\n", ch );

		send_to_char( "Info for immortals:\r\n", ch );

		if( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
			ch_printf( ch, "%s was authorized by %s.\r\n", victim->name, victim->pcdata->authed_by );

		ch_printf( ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\r\n",
			victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
		if( victim->pcdata->pkills || victim->pcdata->pdeaths )
			ch_printf( ch, "%s has killed %d players, and been killed by a player %d times.\r\n",
				victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
		if( victim->pcdata->illegal_pk )
			ch_printf( ch, "%s has committed %d illegal player kills.\r\n", victim->name, victim->pcdata->illegal_pk );

		ch_printf( ch, "%s is %shelled at the moment.\r\n",
			victim->name, ( victim->pcdata->release_date == 0 ) ? "not " : "" );

		if( victim->pcdata->release_date != 0 )
			ch_printf( ch, "%s was helled by %s, and will be released on %24.24s.\r\n",
				victim->sex == SEX_MALE ? "He" :
				victim->sex == SEX_FEMALE ? "She" : "It",
				victim->pcdata->helled_by, ctime( &victim->pcdata->release_date ) );

		if( get_trust( victim ) < get_trust( ch ) )
		{
			sprintf( buf2, "list %s", buf );
			do_comment( ch, buf2 );
		}

		if( xIS_SET( victim->act, PLR_SILENCE ) || xIS_SET( victim->act, PLR_NO_EMOTE )
			|| xIS_SET( victim->act, PLR_NO_TELL ) )
		{
			sprintf( buf2, "This player has the following flags set:" );
			if( xIS_SET( victim->act, PLR_SILENCE ) )
				strcat( buf2, " silence" );
			if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
				strcat( buf2, " noemote" );
			if( xIS_SET( victim->act, PLR_NO_TELL ) )
				strcat( buf2, " notell" );
			strcat( buf2, ".\r\n" );
			send_to_char( buf2, ch );
		}
		if( victim->desc && victim->desc->host[0] != '\0' )   /* added by Gorog */
		{
			sprintf( buf2, "%s's IP info: ", victim->name );
			if( get_trust( ch ) >= LEVEL_LIAISON )
			{
				strcat( buf2, victim->desc->user );
				strcat( buf2, "@" );
				strcat( buf2, victim->desc->host );
			}
			strcat( buf2, "\r\n" );
			send_to_char( buf2, ch );
		}
		if( get_trust( ch ) >= LEVEL_LIAISON && get_trust( ch ) >= get_trust( victim ) && victim->pcdata )
		{
			sprintf( buf2, "Email: %s\r\n", victim->pcdata->email );
			send_to_char( buf2, ch );
		}
	}
	send_to_char( "&pLogging off&P...\r\n&P...\r\n&P..Disconnected.", ch );
}

CMDF( do_pager )
{
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
		return;
	argument = one_argument( argument, arg );
	if( !*arg )
	{
		if( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
			do_config( ch, "-pager" );
		else
			do_config( ch, "+pager" );
		return;
	}
	if( !is_number( arg ) )
	{
		send_to_char( "Set page pausing to how many lines?\r\n", ch );
		return;
	}
	ch->pcdata->pagerlen = atoi( arg );
	if( ch->pcdata->pagerlen < 5 )
		ch->pcdata->pagerlen = 5;
	ch_printf( ch, "Page pausing set to %d lines.\r\n", ch->pcdata->pagerlen );
	return;
}


CMDF( do_helpcheck )
{
	CMDTYPE *command;
	AREA_DATA *tArea;
	char arg[MAX_STRING_LENGTH];
	int hash, col = 0, sn = 0;

	argument = one_argument( argument, arg );

	if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( arg[0] == '\0' || !str_cmp( arg, "all" ) )
	{
		do_helpcheck( ch, "commands" );
		send_to_char( "\r\n", ch );
		do_helpcheck( ch, "skills" );
		send_to_char( "\r\n", ch );
		do_helpcheck( ch, "areas" );
		send_to_char( "\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "commands" ) )
	{
		send_to_char( "&C&YCommands for which there are no help files:\r\n\r\n", ch );

		for( hash = 0; hash < 126; hash++ )
		{
			for( command = command_hash[hash]; command; command = command->next )
			{
				if( !get_help( ch, command->name ) )
				{
					ch_printf( ch, "&W%-15s", command->name );
					if( ++col % 5 == 0 )
					{
						send_to_char( "\r\n", ch );
					}
				}
			}
		}

		send_to_char( "\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "skills" ) || !str_cmp( arg, "spells" ) )
	{
		send_to_char( "&CSkills/Spells for which there are no help files:\r\n\r\n", ch );

		for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
		{
			if( !get_help( ch, skill_table[sn]->name ) )
			{
				ch_printf( ch, "&W%-20s", skill_table[sn]->name );
				if( ++col % 4 == 0 )
				{
					send_to_char( "\r\n", ch );
				}
			}
		}

		send_to_char( "\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "areas" ) )
	{
		send_to_char( "&GAreas for which there are no help files:\r\n\r\n", ch );

		for( tArea = first_area; tArea; tArea = tArea->next )
		{
			if( !get_help( ch, tArea->name ) )
			{
				ch_printf( ch, "&W%-35s", tArea->name );
				if( ++col % 2 == 0 )
				{
					send_to_char( "\r\n", ch );
				}
			}
		}

		send_to_char( "\r\n", ch );
		return;
	}

	send_to_char( "Syntax:  nohelps <all|areas|commands|skills>\r\n", ch );
	return;
}

int strlen_codes( const char *argument );

CMDF( do_testground )
{
	ch_printf( ch, "Length of input string: %d\r\n", strlen_color( argument ) );
	ch_printf( ch, "Length of codes in string: %d\r\n", strlen_codes( argument ) );
	return;
}

CMDF( do_inform )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	HELP_DATA *pHelp;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );


	if( arg[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Inform whom of what?\r\n", ch );
		return;
	}

	if( ( pHelp = get_help( ch, arg2 ) ) == NULL )
	{
		send_to_char( "No such help topic.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "all" ) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		/*
		 * I don't especially want entrance level Imms to
		 * be able to inform EVERYONE of a help topic, do you? -Halcyon
		 */

		if( get_trust( ch ) < LEVEL_LIAISON || IS_NPC( ch ) )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}

		ch_printf( ch, "You inform everyone of the '%s' help topic.\r\n", arg2 );
		for( vch = first_char; vch; vch = vch_next )
		{
			vch_next = vch->next;

			if( ( !IS_NPC( vch ) && ( get_trust( ch ) > get_trust( vch ) ) ) )
			{
				ch_printf( vch, "%s would like to inform you of the '%s' help topic.\r\n", ch->name, arg2 );
				do_help( vch, arg2 );
			}
		}
	}

	else
	{
		CHAR_DATA *victim;

		/*
		 * Lower level Imms probably shouldn't be able to force
		 * people that are in a writing buffer to read help files. -Halcyon
		 */
		 /*
				 if(( victim = get_char_world( ch, arg )) == NULL
					 || IS_NPC( victim ) || ( !IS_NPC( victim ) && ( !victim->desc )
					 || (  victim->desc )&& (victim->desc->connected  == CON_EDITING) && ( get_trust(ch) < LEVEL_LIAISON) ) )
				 {
					 send_to_char( "That player isn't here.\r\n", ch );
					 return;
				 }
		 */

		if( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			send_to_char( "That player isn't here.\r\n", ch );
			return;
		}

		if( IS_NPC( victim ) || ( !IS_NPC( victim ) && ( !victim->desc ) ) )
		{
			send_to_char( "That player isn't here.\r\n", ch );
			return;
		}

		if( ( victim->desc ) && ( victim->desc->connected == CON_EDITING ) && ( get_trust( ch ) < LEVEL_LIAISON ) )
		{
			send_to_char( "That player isn't here.\r\n", ch );
			return;
		}

		if( get_trust( ch ) < get_trust( victim ) )
		{
			send_to_char( "You can't inform your superiors.\r\n", ch );
			return;
		}

		/*
		 * Ditto for AFK players. -Halcyon
		 */

		if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That player is AFK.\r\n", ch );
			return;
		}

		ch_printf( ch, "You inform %s of the '%s' help topic.\r\n", victim->name, arg2 );
		ch_printf( victim, "%s would like to inform you of the '%s' help topic.\r\n", ch->name, arg2 );
		do_help( victim, arg2 );
		return;
	}
}

CMDF( do_nohelps )
{
	CMDTYPE *command;
	char arg[MAX_STRING_LENGTH];
	int hash, col = 0, sn = 0;
	int ccount = 0, scount = 0;
	argument = one_argument( argument, arg );
	if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}
	if( arg[0] == '\0' || !str_cmp( arg, "all" ) )
	{
		do_nohelps( ch, "commands" );
		send_to_char( "\r\n", ch );
		do_nohelps( ch, "skills" );
		send_to_char( "\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "commands" ) )
	{
		send_to_char( "&C&YCommands for which there are no help files:\r\n\r\n", ch );
		for( hash = 0; hash < 126; hash++ )
		{
			for( command = command_hash[hash]; command; command = command->next )
			{
				if( !get_help( ch, command->name ) )
				{
					ch_printf( ch, "&W%-15s", command->name );
					ccount++;
					if( ++col % 5 == 0 )
					{
						send_to_char( "\r\n", ch );
					}
				}
			}
		}
		ch_printf( ch, "\r\n&CTotal&B: &W%2d\r\n", ccount - 1 );
		send_to_char( "\r\n", ch );
		return;
	}
	if( !str_cmp( arg, "skills" ) || !str_cmp( arg, "spells" ) )
	{
		send_to_char( "&CSkills/Spells for which there are no help files:\r\n\r\n", ch );
		for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
		{
			if( !get_help( ch, skill_table[sn]->name ) )
			{
				ch_printf( ch, "&W%-20s", skill_table[sn]->name );
				scount++;
				if( ++col % 4 == 0 )
				{
					send_to_char( "\r\n", ch );
				}
			}
		}
		ch_printf( ch, "\r\n&CTotal&B: &W%2d\r\n\r\n", scount );
	}
	send_to_char( "&CSyntax&B: &Wnohelps &B<&Wskills&B|&Wcommands&B|&Wall&B>&W\r\n", ch );
	return;
}
