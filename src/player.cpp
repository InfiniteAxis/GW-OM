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
#include <sys/stat.h>
//#include <sys/dir.h>
#include <time.h>
#include "mud.h"
#include <dlfcn.h>
#include <dirent.h>

/*
 *  Locals
 */
const char *tiny_affect_loc_name( int location );

LOCKER_DATA *load_locker( CHAR_DATA *ch );
void fread_locker( CHAR_DATA *ch, LOCKER_DATA *locker, FILE *fp );
void fwrite_locker( CHAR_DATA *ch );
ROOM_INDEX_DATA *generate_virtual_room( ROOM_INDEX_DATA *in_room );
void delete_locker( CHAR_DATA *ch );

/* Declared in act_move.c */
extern ROOM_INDEX_DATA *vroom_hash[64];

#define STORAGE_PRICE    500000
#define STORAGE_CAPACITY 20

CMDF( do_gold )
{

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ch->top_level < 1001 && ch->pcdata->bank > 100000000 )
	{
		ch->pcdata->bank = 100000000;
	}

	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		send_to_char( "&YMoney: &O0&Y.\r\n&YBank: &O0&Y.\r\n", ch );
		return;
	}

	ch_printf( ch, "&YMoney: &O%s&Y.\r\n", num_punct( ch->gold ) );
	ch_printf( ch, "&YBank: &O%s&Y.\r\n", num_punct( ch->pcdata->bank ) );
	if( ch->pcdata->clan )
		ch_printf( ch, "&Y%s: &O%s&Y.\r\n", ch->pcdata->clan->name, num_punct( ch->pcdata->clan->funds ) );
	return;
}

const char *posString( CHAR_DATA *ch )
{
	switch( ch->position )
	{
	case POS_DEAD:
		return( "slowly decomposing" );
	case POS_MORTAL:
		return( "mortally wounded" );
	case POS_INCAP:
		return( "incapacitated" );
	case POS_STUNNED:
		return( "stunned" );
	case POS_SLEEPING:
		return( "sleeping" );
	case POS_RESTING:
		return( "resting" );
	case POS_STANDING:
		return( "standing" );
	case POS_FIGHTING:
		return( "fighting" );
	case POS_MOUNTED:
		return( "mounted" );
	case POS_SITTING:
		return( "sitting" );
	default:
		bug( "Incorrect Position setting %s: Char: %s. Pos: %d (%s) (Defaulting to resting.)", __FUNCTION__,
			ch->name, ch->position, npc_position[ch->position] );
		ch->position = POS_RESTING;
		return( "resting" );
	}
}

char *drawalign( int align )
{
	static char buf[MSL];
	if( align >= 1000 )
		snprintf( buf, MSL, "&W[&C============&W|&r&W]" );
	else if( align >= 900 && align < 1000 )
		snprintf( buf, MSL, "&W[&C===========&W|&r=&W]" );
	else if( align >= 600 && align < 900 )
		snprintf( buf, MSL, "&W[&C==========&W|&r==&W]" );
	else if( align >= 400 && align < 600 )
		snprintf( buf, MSL, "&W[&C=========&W|&r===&W]" );
	else if( align >= 200 && align < 400 )
		snprintf( buf, MSL, "&W[&C========&W|&r====&W]" );
	else if( align >= 100 && align < 200 )
		snprintf( buf, MSL, "&W[&C=======&W|&r=====&W]" );
	else if( align > -100 && align < 100 )
		snprintf( buf, MSL, "&W[&C======&W|&r======&W]" );
	else if( align <= -100 && align > -200 )
		snprintf( buf, MSL, "&W[&C=====&W|&r=======&W]" );
	else if( align <= -200 && align > -400 )
		snprintf( buf, MSL, "&W[&C====&W|&r========&W]" );
	else if( align <= -400 && align > -600 )
		snprintf( buf, MSL, "&W[&C===&W|&r=========&W]" );
	else if( align <= -600 && align > -900 )
		snprintf( buf, MSL, "&W[&C==&W|&r==========&W]" );
	else if( align <= -900 && align > -1000 )
		snprintf( buf, MSL, "&W[&C=&W|&r===========&W]" );
	else if( align <= -1000 )
		snprintf( buf, MSL, "&W[&C&W|&r============&W]" );
	return buf;
}

/*
 * New score command by Taypeon!
 */

CMDF( do_score )
{
	char arg[MIL];
	AFFECT_DATA *paf;
	int mobperc, pkperc, illperc, ability;

	if( IS_NPC( ch ) )
		return;

	if( ch->pcdata->mkills != 0 )
		mobperc = ( int ) ( ( ( ch->pcdata->mkills + ch->pcdata->pkills + ch->pcdata->illegal_pk ) * 100 ) / ( ch->pcdata->mkills ) );
	else
		mobperc = 0;
	if( ch->pcdata->pkills != 0 )
		pkperc = ( int ) ( ( ( ch->pcdata->mkills + ch->pcdata->pkills + ch->pcdata->illegal_pk ) * 100 ) / ( ch->pcdata->pkills ) );
	else
		pkperc = 0;
	if( ch->pcdata->illegal_pk != 0 )
		illperc = ( int ) ( ( ( ch->pcdata->mkills + ch->pcdata->pkills + ch->pcdata->illegal_pk ) * 100 ) / ( ch->pcdata->illegal_pk ) );
	else
		illperc = 0;

	set_pager_color( AT_SCORE, ch );
	ch_printf( ch, "\r\n" );
	ch_printf( ch, "&[divider] .----------------------------------------------------------------------------.&D\r\n" );
	ch_printf( ch, "&[divider] | &[score]Name&[score3]: &[score2]%-25.25s  &[score]Nationality&[score3]: &[score2]%10.10s %-15.15s   &[divider]|&D\r\n", ch->name, ch->sex == SEX_MALE ? "Male"
		: ch->sex == SEX_FEMALE ? "Female" : "Non-binary", capitalize( get_race( ch ) ) );
	if( get_trust( ch ) != ch->top_level )
		ch_printf( ch, "&[divider] | &[score]You are trusted at level &[score2]%3.3d&[score].                                              &[divider]|&D\r\n", get_trust( ch ) );
	if( !IS_NPC( ch ) )
		ch_printf( ch, "&[divider] | &[score]Title&[score3]:&[score2] %68s &[divider]|&D\r\n", color_align( ch->pcdata->title, 67, ALIGN_LEFT ) );
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	ch_printf( ch, "&[divider] | &[score]Age&[score3]: &[score2]%-6d      &[score]     FP&[score3]: &[score2]%-4d                            &[score]Strength: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		get_age( ch ), ch->questpoints, get_curr_str( ch ), ch->perm_str );
	ch_printf( ch, "&[divider] | &[score]Hitroll&[score3]: &[score2]%4d         &[score]Damroll&[score3]: &[score2]%4d                      &[score]Dexterity&[score3]: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		GET_HITROLL( ch ), GET_DAMROLL( ch ), get_curr_dex( ch ), ch->perm_dex );
	ch_printf( ch, "&[divider] | &[score]HP&[score3]: &[score2]%-7d&[score4]/&[score2]%-7d   &[score]Armor Class&[score3]: &[score2]%+5d&[score]              &[score]Intelligence&[score3]: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		ch->hit, ch->max_hit, GET_AC( ch ), get_curr_int( ch ), ch->perm_int );
	ch_printf( ch, "&[divider] | &[score]Mv&[score3]:   &[score2]%-5d&[score4]/&[score2]%5d     &[score]Align&[score3]: &[score2]%-25.25s                &[score]Wisdom&[score3]: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		ch->move, ch->max_move, drawalign( ch->alignment ), get_curr_wis( ch ), ch->perm_wis );
	ch_printf( ch, "&[divider] | &[score]Wimpy&[score3]: &[score2]%5d          &[score]Position&[score3]: &[score2]%-21.21s &[score]Constitution&[score3]: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		ch->wimpy, posString( ch ), get_curr_con( ch ), ch->perm_con );
	ch_printf( ch, "&[divider] | &[score]Clan&[score3]: &[score2]%-25.25s                           &[score]Charisma&[score3]: &[score2]%2.2d&[score4](&[score3]%2.2d&[score4]) &[divider]|&D\r\n",
		ch->pcdata->clan ? ch->pcdata->clan->name : "Civilian", get_curr_cha( ch ), ch->perm_cha );
	if( !IS_NPC( ch ) )
		ch_printf( ch, "&[divider] | &[score]You have accrued &[score2]%3.3d&[score] LP.                                                   &[divider]|&D\r\n", ch->pcdata->lp );
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	if( ch->pcdata->clan )
	{
		ch_printf( ch, "&[divider] |            &[score]Clan Rank&[score3]: %-10s  &[score]Salary: &[score3]%-6d                               &[divider]|&D\r\n",
			clan_rank_table[ch->pcdata->clan_rank].title_of_rank[ch->sex], ch->pcdata->salary );
		ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	}
	ch_printf( ch, "&[divider] |       &[score]Objects&[score3]: &[score2]%6.6d &[score4](&[score]Max&[score3]: &[score2]%6.6d&[score4])        &[score]Weight&[score3]: &[score2]%5.5d &[score4](&[score]Max&[score3]: &[score2]%7.7d&[score4])    &[divider]|&D\r\n",
		ch->carry_number, can_carry_n( ch ), ch->carry_weight, can_carry_w( ch ) );
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	ch_printf( ch, "&[divider] | &[score]Kills&[score3]: &[score]Mobs&[score3]: &[score2]%6d &[score4](&[score3]%3d&[score]%&[score4]) &[score]Players&[score3]: &[score2]%6d &[score4](&[score3]%3d&[score]%&[score4]) &[score]Illegal&[score3]: &[score2]%6d &[score4](&[score3]%3d&[score]%&[score4])   &[divider]|&D\r\n",
		ch->pcdata->mkills, mobperc, ch->pcdata->pkills, pkperc, ch->pcdata->illegal_pk, illperc );
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	for( ability = 0; ability < MAX_ABILITY - 1; ability++ )
	{
		ch_printf( ch, "&[divider] |     %s&[score2]%-15s      &[score]Level&[score3]: &[score2]%-4d &[score2]of &[score2]%-4d       &[score]Exp&[score3]:&[score2] %-10ld        &[divider]|&D\r\n",
			ch->main_ability == ability ? "&R+" :  " ", 	ability_name[ability], ch->skill_level[ability], max_level( ch, ability ), ch->experience[ability] );
	}
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	ch_printf( ch, "&[divider] |                            &[score]Wallet&[score3]:     &[score2]%-25.25s           &[divider]|&D\r\n", num_punct( ch->gold ) );
	if( !IS_NPC( ch ) )
		ch_printf( ch, "&[divider] |                           &[score]   Bank&[score3]:     &[score2]%-25.25s           &[divider]|&D\r\n", num_punct( ch->pcdata->bank ) );
	ch_printf( ch, "&[divider] |----------------------------------------------------------------------------|&D\r\n" );
	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
		ch_printf( ch, "&[divider] | &[score2]You are drunk.                                                             &[divider]|&D\r\n" );
	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
		ch_printf( ch, "&[divider] | &[score2]You are in danger of dehydrating.                                          &[divider]|&D\r\n" );
	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
		ch_printf( ch, "&[divider] | &[score2]You are starving to death.                                                 &[divider]|&D\r\n" );
	if( ch->position != POS_SLEEPING )
		switch( ch->mental_state / 10 )
		{
		default:
			ch_printf( ch, "&[divider] | &[score2]You're completely messed up!                                               &[divider]|&D\r\n" );
			break;
		case -10:
			ch_printf( ch, "&[divider] | &[score2]You're barely conscious.                                                   &[divider]|&D\r\n" );
			break;
		case -9:
			ch_printf( ch, "&[divider] | &[score2]You can barely keep your eyes open.                                        &[divider]|&D\r\n" );
			break;
		case -8:
			ch_printf( ch, "&[divider] | &[score2]You're extremely drowsy.                                                   &[divider]|&D\r\n" );
			break;
		case -7:
			ch_printf( ch, "&[divider] | &[score2]You feel very unmotivated.                                                 &[divider]|&D\r\n" );
			break;
		case -6:
			ch_printf( ch, "&[divider] | &[score2]You feel sedated.                                                          &[divider]|&D\r\n" );
			break;
		case -5:
			ch_printf( ch, "&[divider] | &[score2]You feel sleepy.                                                           &[divider]|&D\r\n" );
			break;
		case -4:
			ch_printf( ch, "&[divider] | &[score2]You feel tired.                                                            &[divider]|&D\r\n" );
			break;
		case -3:
			ch_printf( ch, "&[divider] | &[score2]You could use a rest.                                                      &[divider]|&D\r\n" );
			break;
		case -2:
			ch_printf( ch, "&[divider] | &[score2]You feel a little under the weather.                                       &[divider]|&D\r\n" );
			break;
		case -1:
			ch_printf( ch, "&[divider] | &[score2]You feel fine.                                                             &[divider]|&D\r\n" );
			break;
		case 0:
			ch_printf( ch, "&[divider] | &[score2]You feel great.                                                            &[divider]|&D\r\n" );
			break;
		case 1:
			ch_printf( ch, "&[divider] | &[score2]You feel energetic.                                                        &[divider]|&D\r\n" );
			break;
		case 2:
			ch_printf( ch, "&[divider] | &[score2]Your mind is racing.                                                       &[divider]|&D\r\n" );
			break;
		case 3:
			ch_printf( ch, "&[divider] | &[score2]You can't think straight.                                                  &[divider]|&D\r\n" );
			break;
		case 4:
			ch_printf( ch, "&[divider] | &[score2]Your mind is going 100 miles an hour.                                      &[divider]|&D\r\n" );
			break;
		case 5:
			ch_printf( ch, "&[divider] | &[score2]You're high as a kite.                                                     &[divider]|&D\r\n" );
			break;
		case 6:
			ch_printf( ch, "&[divider] | &[score2]Your mind and body are slipping apart.                                     &[divider]|&D\r\n" );
			break;
		case 7:
			ch_printf( ch, "&[divider] | &[score2]Reality is slipping away.                                                  &[divider]|&D\r\n" );
			break;
		case 8:
			ch_printf( ch, "&[divider] | &[score2]You have no idea what is real, and what is not.                            &[divider]|&D\r\n" );
			break;
		case 9:
			ch_printf( ch, "&[divider] | &[score2]You feel immortal.                                                         &[divider]|&D\r\n" );
			break;
		case 10:
			ch_printf( ch, "&[divider] | &[score2]You are a Supreme Entity.                                                  &[divider]|&D\r\n" );
			break;
		}
	else if( ch->mental_state > 45 )
		ch_printf( ch, "&[divider] | &[score2]Your sleep is filled with strange and vivid dreams.                        &[divider]|&D\r\n" );
	else if( ch->mental_state > 25 )
		ch_printf( ch, "&[divider] | &[score2]Your sleep is uneasy.                                                      &[divider]|&D\r\n" );
	else if( ch->mental_state < -35 )
		ch_printf( ch, "&[divider] | &[score2]You are deep in a much needed sleep.                                       &[divider]|&D\r\n" );
	else if( ch->mental_state < -25 )
		ch_printf( ch, "&[divider] | &[score2]You are in deep slumber.                                                   &[divider]|&D\r\n" );
	ch_printf( ch, "&[divider] '----------------------------------------------------------------------------'&D\r\n" );
	if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
		ch_printf( ch, "&[score2]You are bestowed with the command(s): %s.\r\n", ch->pcdata->bestowments );
	if( IS_IMMORTAL( ch ) )
	{
		ch_printf( ch, "&[score2]Immortal Data&[score3]:\r\n  &[score]Wizinvis &[score4][&[score2]%s&[score4]]  &[score]Wizlevel &[score4](&[score2]%d&[score4])\r\n",
			xIS_SET( ch->act, PLR_WIZINVIS ) ? "X" : " ", ch->pcdata->wizinvis );

		if( ch->pcdata->bamfin[0] != '\0' )
			ch_printf( ch, "  &[score]Bamfin&[score3]:&[immortal]  %s\r\n", ch->pcdata->bamfin );
		else
			ch_printf( ch, "  &[score]Bamfin&[score3]:&[immortal]  %s %s\r\n", ch->name, "appears in a swirling mist." );
		if( ch->pcdata->bamfout[0] != '\0' )
			ch_printf( ch, "  &[score]Bamfout&[score3]:&[immortal] %s\r\n", ch->pcdata->bamfout );
		else
			ch_printf( ch, "  &[score]Bamfout&[score3]:&[immortal] %s %s\r\n", ch->name, "leaves in a swirling mist." );
		/*
		 * Area Loaded info - Scryn 8/11
		 */
		if( ch->pcdata->area )
		{
			ch_printf( ch, "  &[score]Vnums&[score3]:  &[score2]%-5.5d &[score4]- &[score2]%-5.5d&D\r\n", ch->pcdata->area->low_vnum, ch->pcdata->area->hi_vnum );
			ch_printf( ch, "  &[score]Area Loaded &[score4][&[score2]%s&[score4]]&D\r\n", ( IS_SET( ch->pcdata->area->status, AREA_LOADED ) ) ? "yes" : "no" );
		}
	}
	argument = one_argument( argument, arg );
	if( ch->first_affect && !str_cmp( arg, "affects" ) )
	{
		int i;
		SKILLTYPE *sktmp;

		i = 0;
		ch_printf( ch, "AFFECT DATA:                            " );
		for( paf = ch->first_affect; paf; paf = paf->next )
		{
			if( ( sktmp = get_skilltype( paf->type ) ) == NULL )
				continue;
			if( ch->top_level < 20 )
			{
				ch_printf( ch, "&[score4][&[score2]%-34.34s&[score4]]    ", sktmp->name );
				if( i == 0 )
					i = 2;
				if( ( ++i % 3 ) == 0 )
					ch_printf( ch, "\r\n" );
			}
			if( ch->top_level >= 20 )
			{
				if( paf->modifier == 0 )
					ch_printf( ch, "&[score4][&[score2]%-24.24s;%5d rds&[score4]]    ", sktmp->name, paf->duration );
				else if( paf->modifier > 999 )
					ch_printf( ch, "&[score4][&[score2]%-15.15s; %7.7s;%5d rds&[score4]]    ",
						sktmp->name, tiny_affect_loc_name( paf->location ), paf->duration );
				else
					ch_printf( ch, "&[score4][&[score2]%-11.11s;%+-3.3d %7.7s;%5d rds&[score4]]    ",
						sktmp->name, paf->modifier, tiny_affect_loc_name( paf->location ), paf->duration );
				if( i == 0 )
					i = 1;
				if( ( ++i % 2 ) == 0 )
					ch_printf( ch, "\r\n" );
			}
		}
	}
	send_to_char( "\r\n", ch );
	return;
}

/*
 * Return ascii name of an affect location.
 */
const char *tiny_affect_loc_name( int location )
{
	switch( location )
	{
	case APPLY_NONE:
		return "NIL";
	case APPLY_STR:
		return " STR  ";
	case APPLY_DEX:
		return " DEX  ";
	case APPLY_INT:
		return " INT  ";
	case APPLY_WIS:
		return " WIS  ";
	case APPLY_CON:
		return " CON  ";
	case APPLY_CHA:
		return " CHA  ";
	case APPLY_LCK:
		return " LCK  ";
	case APPLY_SEX:
		return " SEX  ";
	case APPLY_LEVEL:
		return " LVL  ";
	case APPLY_AGE:
		return " AGE  ";
	case APPLY_MANA:
		return " MANA ";
	case APPLY_HIT:
		return " HV   ";
	case APPLY_MOVE:
		return " MOVE ";
	case APPLY_GOLD:
		return " GOLD ";
	case APPLY_EXP:
		return " EXP  ";
	case APPLY_AC:
		return " AC   ";
	case APPLY_HITROLL:
		return " HITRL";
	case APPLY_DAMROLL:
		return " DAMRL";
	case APPLY_SAVING_POISON:
		return "SV POI";
	case APPLY_SAVING_ROD:
		return "SV ROD";
	case APPLY_SAVING_PARA:
		return "SV PARA";
	case APPLY_SAVING_BREATH:
		return "SV BRTH";
	case APPLY_SAVING_SPELL:
		return "SV SPLL";
	case APPLY_HEIGHT:
		return "HEIGHT";
	case APPLY_WEIGHT:
		return "WEIGHT";
	case APPLY_AFFECT:
		return "AFF BY";
	case APPLY_RESISTANT:
		return "RESIST";
	case APPLY_IMMUNE:
		return "IMMUNE";
	case APPLY_SUSCEPTIBLE:
		return "SUSCEPT";
	case APPLY_WEAPONSPELL:
		return " WEAPON";
	case APPLY_BACKSTAB:
		return "BACKSTB";
	case APPLY_PICK:
		return " PICK  ";
	case APPLY_TRACK:
		return " TRACK ";
	case APPLY_STEAL:
		return " STEAL ";
	case APPLY_SNEAK:
		return " SNEAK ";
	case APPLY_HIDE:
		return " HIDE  ";
	case APPLY_PALM:
		return " PALM  ";
	case APPLY_DETRAP:
		return " DETRAP";
	case APPLY_DODGE:
		return " DODGE ";
	case APPLY_PEEK:
		return " PEEK  ";
	case APPLY_SCAN:
		return " SCAN  ";
	case APPLY_GOUGE:
		return " GOUGE ";
	case APPLY_SEARCH:
		return " SEARCH";
	case APPLY_MOUNT:
		return " MOUNT ";
	case APPLY_DISARM:
		return " DISARM";
	case APPLY_KICK:
		return " KICK  ";
	case APPLY_PARRY:
		return " PARRY ";
	case APPLY_BASH:
		return " BASH  ";
	case APPLY_STUN:
		return " STUN  ";
	case APPLY_PUNCH:
		return " PUNCH ";
	case APPLY_CLIMB:
		return " CLIMB ";
	case APPLY_GRIP:
		return " GRIP  ";
	case APPLY_SCRIBE:
		return " SCRIBE";
	case APPLY_BREW:
		return " BREW  ";
	case APPLY_WEARSPELL:
		return " WEAR  ";
	case APPLY_REMOVESPELL:
		return " REMOVE";
	case APPLY_EMOTION:
		return "EMOTION";
	case APPLY_MENTALSTATE:
		return " MENTAL";
	case APPLY_STRIPSN:
		return " DISPEL";
	case APPLY_REMOVE:
		return " REMOVE";
	case APPLY_DIG:
		return " DIG   ";
	case APPLY_FULL:
		return " HUNGER";
	case APPLY_THIRST:
		return " THIRST";
	case APPLY_DRUNK:
		return " DRUNK ";
	case APPLY_BLOOD:
		return " BLOOD ";
	}

	bug( "Affect_location_name: unknown location %d.", location );
	return ( " ??? " );
}

const char *get_race( CHAR_DATA *ch )
{
	if( ch->race < MAX_RACE && ch->race >= 0 )
		return ( pc_race[ch->race] );
	return ( "Unknown" );
}

CMDF( do_oldscore )
{
	AFFECT_DATA *paf;
	SKILLTYPE *skill;

	if( IS_NPC( ch ) )
		return;

	if( IS_AFFECTED( ch, AFF_POSSESS ) )
	{
		send_to_char( "You can't do that in your current state of mind!\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		do_score( ch, "" );
		return;
	}

	set_char_color( AT_SCORE, ch );
	ch_printf( ch,
		"You are %s %s&C, level %d, %d years old.\r\n",
		ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title, ch->top_level, get_age( ch ) );

	if( get_trust( ch ) != ch->top_level )
		ch_printf( ch, "&CYou are trusted at level %d.\r\n", get_trust( ch ) );

	if( xIS_SET( ch->act, ACT_MOBINVIS ) )
		ch_printf( ch, "You are mobinvis at level %d.\r\n", ch->mobinvis );


	ch_printf( ch, "You have %d/%d hit, %d/%d movement.\r\n", ch->hit, ch->max_hit, ch->move, ch->max_move );

	ch_printf( ch,
		"You are carrying %d/%d items with weight %d/%d kg.\r\n",
		ch->carry_number, can_carry_n( ch ), ch->carry_weight, can_carry_w( ch ) );

	ch_printf( ch,
		"Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: ??\r\n",
		get_curr_str( ch ),
		get_curr_int( ch ), get_curr_wis( ch ), get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ) );

	ch_printf( ch, "You have %d dollars.\r\n", ch->gold );

	if( !IS_NPC( ch ) )
		ch_printf( ch,
			"You have achieved %d glory during your life, and currently have %d.\r\n",
			ch->pcdata->quest_accum, ch->pcdata->quest_curr );

	ch_printf( ch,
		"Autoexit: %s   Autoloot: %s   Autosac: %s   Autocred: %s\r\n",
		( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes" : "no",
		( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes" : "no",
		( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOSAC ) ) ? "yes" : "no",
		( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes" : "no" );

	ch_printf( ch, "Wimpy set to %d hit points.\r\n", ch->wimpy );

	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
		send_to_char( "You are drunk.\r\n", ch );
	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
		send_to_char( "You are thirsty.\r\n", ch );
	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
		send_to_char( "You are hungry.\r\n", ch );

	switch( ch->mental_state / 10 )
	{
	default:
		send_to_char( "You're completely messed up!\r\n", ch );
		break;
	case -10:
		send_to_char( "You're barely conscious.\r\n", ch );
		break;
	case -9:
		send_to_char( "You can barely keep your eyes open.\r\n", ch );
		break;
	case -8:
		send_to_char( "You're extremely drowsy.\r\n", ch );
		break;
	case -7:
		send_to_char( "You feel very unmotivated.\r\n", ch );
		break;
	case -6:
		send_to_char( "You feel sedated.\r\n", ch );
		break;
	case -5:
		send_to_char( "You feel sleepy.\r\n", ch );
		break;
	case -4:
		send_to_char( "You feel tired.\r\n", ch );
		break;
	case -3:
		send_to_char( "You could use a rest.\r\n", ch );
		break;
	case -2:
		send_to_char( "You feel a little under the weather.\r\n", ch );
		break;
	case -1:
		send_to_char( "You feel fine.\r\n", ch );
		break;
	case 0:
		send_to_char( "You feel great.\r\n", ch );
		break;
	case 1:
		send_to_char( "You feel energetic.\r\n", ch );
		break;
	case 2:
		send_to_char( "Your mind is racing.\r\n", ch );
		break;
	case 3:
		send_to_char( "You can't think straight.\r\n", ch );
		break;
	case 4:
		send_to_char( "Your mind is going 100 miles an hour.\r\n", ch );
		break;
	case 5:
		send_to_char( "You're high as a kite.\r\n", ch );
		break;
	case 6:
		send_to_char( "Your mind and body are slipping appart.\r\n", ch );
		break;
	case 7:
		send_to_char( "Reality is slipping away.\r\n", ch );
		break;
	case 8:
		send_to_char( "You have no idea what is real, and what is not.\r\n", ch );
		break;
	case 9:
		send_to_char( "You feel immortal.\r\n", ch );
		break;
	case 10:
		send_to_char( "You are a Supreme Entity.\r\n", ch );
		break;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "You are DEAD!!\r\n", ch );
		break;
	case POS_MORTAL:
		send_to_char( "You are mortally wounded.\r\n", ch );
		break;
	case POS_INCAP:
		send_to_char( "You are incapacitated.\r\n", ch );
		break;
	case POS_STUNNED:
		send_to_char( "You are stunned.\r\n", ch );
		break;
	case POS_SLEEPING:
		send_to_char( "You are sleeping.\r\n", ch );
		break;
	case POS_RESTING:
		send_to_char( "You are resting.\r\n", ch );
		break;
	case POS_STANDING:
		send_to_char( "You are standing.\r\n", ch );
		break;
	case POS_FIGHTING:
		send_to_char( "You are fighting.\r\n", ch );
		break;
	case POS_MOUNTED:
		send_to_char( "Mounted.\r\n", ch );
		break;
	case POS_SHOVE:
		send_to_char( "Being shoved.\r\n", ch );
		break;
	case POS_DRAG:
		send_to_char( "Being dragged.\r\n", ch );
		break;
	}

	if( ch->top_level >= 25 )
		ch_printf( ch, "AC: %d.  ", GET_AC( ch ) );

	send_to_char( "You are ", ch );
	if( GET_AC( ch ) >= 101 )
		send_to_char( "WORSE than naked!\r\n", ch );
	else if( GET_AC( ch ) >= 80 )
		send_to_char( "naked.\r\n", ch );
	else if( GET_AC( ch ) >= 60 )
		send_to_char( "wearing clothes.\r\n", ch );
	else if( GET_AC( ch ) >= 40 )
		send_to_char( "slightly armored.\r\n", ch );
	else if( GET_AC( ch ) >= 20 )
		send_to_char( "somewhat armored.\r\n", ch );
	else if( GET_AC( ch ) >= 0 )
		send_to_char( "armored.\r\n", ch );
	else if( GET_AC( ch ) >= -20 )
		send_to_char( "well armored.\r\n", ch );
	else if( GET_AC( ch ) >= -40 )
		send_to_char( "strongly armored.\r\n", ch );
	else if( GET_AC( ch ) >= -60 )
		send_to_char( "heavily armored.\r\n", ch );
	else if( GET_AC( ch ) >= -80 )
		send_to_char( "superbly armored.\r\n", ch );
	else if( GET_AC( ch ) >= -100 )
		send_to_char( "divinely armored.\r\n", ch );
	else
		send_to_char( "invincible!\r\n", ch );

	if( ch->top_level >= 15 )
		ch_printf( ch, "Hitroll: %d  Damroll: %d.\r\n", GET_HITROLL( ch ), GET_DAMROLL( ch ) );

	if( ch->top_level >= 10 )
		ch_printf( ch, "Alignment: %d.  ", ch->alignment );

	send_to_char( "You are ", ch );
	if( ch->alignment > 900 )
		send_to_char( "angelic.\r\n", ch );
	else if( ch->alignment > 700 )
		send_to_char( "saintly.\r\n", ch );
	else if( ch->alignment > 350 )
		send_to_char( "good.\r\n", ch );
	else if( ch->alignment > 100 )
		send_to_char( "kind.\r\n", ch );
	else if( ch->alignment > -100 )
		send_to_char( "neutral.\r\n", ch );
	else if( ch->alignment > -350 )
		send_to_char( "mean.\r\n", ch );
	else if( ch->alignment > -700 )
		send_to_char( "evil.\r\n", ch );
	else if( ch->alignment > -900 )
		send_to_char( "demonic.\r\n", ch );
	else
		send_to_char( "satanic.\r\n", ch );

	if( ch->first_affect )
	{
		send_to_char( "You are affected by:\r\n", ch );
		for( paf = ch->first_affect; paf; paf = paf->next )
			if( ( skill = get_skilltype( paf->type ) ) != NULL )
			{
				ch_printf( ch, "Spell: '%s'", skill->name );

				if( ch->top_level >= 20 )
					ch_printf( ch,
						" modifies %s by %d for %d rounds",
						affect_loc_name( paf->location ), paf->modifier, paf->duration );

				send_to_char( ".\r\n", ch );
			}
	}

	if( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
	{
		ch_printf( ch, "WizInvis level: %d   WizInvis is %s\r\n",
			ch->pcdata->wizinvis, xIS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
		if( ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
			ch_printf( ch, "Room Range: %d - %d\r\n", ch->pcdata->low_vnum, ch->pcdata->hi_vnum );
		if( ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
			ch_printf( ch, "Obj Range : %d - %d\r\n", ch->pcdata->low_vnum, ch->pcdata->hi_vnum );
		if( ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
			ch_printf( ch, "Mob Range : %d - %d\r\n", ch->pcdata->low_vnum, ch->pcdata->hi_vnum );
	}

	return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
CMDF( do_level )
{
	int ability;

	if( IS_NPC( ch ) )
		return;
	/*
	   if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	   {
		  for( ability = 0; ability < MAX_ABILITY; ability++ )
			 ch_printf( ch, "\r\n&C%-15s   &cLevel&B:&C 1   &cMax&B:&C %-3d  &cExp&B:&C 0   &cNext&B:&C 500",
						ability_name[ability], max_level( ch, ability ) );
		  send_to_char( "\r\n&cCombined Levels&B: &C7", ch );
		  return;
	   }
	*/
	for( ability = 0; ability < MAX_ABILITY - 1; ability++ )
	{
		ch_printf( ch, "&C%-15s   &cLevel&B:&C %-4d   &cMax&B:&C %-3d  &cExp&B:&C %-10ld   &cNext&B:&C %-10d\r\n",
			ability_name[ability], ch->skill_level[ability], max_level( ch, ability ),
			ch->experience[ability], exp_level( ch->skill_level[ability] + 1 ) );
	}

	ch_printf( ch, "\r\n&cCombined Levels&B:&C %d",
		ch->skill_level[COMBAT_ABILITY] + ch->skill_level[PILOTING_ABILITY] + ch->skill_level[ENGINEERING_ABILITY] +
		ch->skill_level[HUNTING_ABILITY] + ch->skill_level[SMUGGLING_ABILITY] + ch->skill_level[DIPLOMACY_ABILITY] +
		ch->skill_level[LEADERSHIP_ABILITY] + ch->skill_level[ESPIONAGE_ABILITY] );
}

CMDF( do_affected )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	SKILLTYPE *skill;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg );

	if( !str_cmp( arg, "by" ) )
	{
		set_char_color( AT_BLUE, ch );
		send_to_char( "\n\rImbued with:\r\n", ch );
		set_char_color( AT_SCORE, ch );
		ch_printf( ch, "%s\r\n", affect_bit_name( ch->affected_by ) );
		if( ch->top_level >= 20 )
		{
			send_to_char( "\r\n", ch );
			if( ch->resistant > 0 )
			{
				set_char_color( AT_BLUE, ch );
				send_to_char( "Resistances:  ", ch );
				set_char_color( AT_SCORE, ch );
				ch_printf( ch, "%s\r\n", flag_string( ch->resistant, ris_flags ) );
			}
			if( ch->immune > 0 )
			{
				set_char_color( AT_BLUE, ch );
				send_to_char( "Immunities:   ", ch );
				set_char_color( AT_SCORE, ch );
				ch_printf( ch, "%s\r\n", flag_string( ch->immune, ris_flags ) );
			}
			if( ch->susceptible > 0 )
			{
				set_char_color( AT_BLUE, ch );
				send_to_char( "Suscepts:     ", ch );
				set_char_color( AT_SCORE, ch );
				ch_printf( ch, "%s\r\n", flag_string( ch->susceptible, ris_flags ) );
			}
		}
		return;
	}

	if( !ch->first_affect )
	{
		set_char_color( AT_SCORE, ch );
		send_to_char( "\n\rNo cantrip or skill affects you.\r\n", ch );
	}
	else
	{
		send_to_char( "\r\n", ch );
		for( paf = ch->first_affect; paf; paf = paf->next )
			if( ( skill = get_skilltype( paf->type ) ) != NULL )
			{
				set_char_color( AT_BLUE, ch );
				send_to_char( "Affected:  ", ch );
				set_char_color( AT_SCORE, ch );
				if( ch->top_level >= 20 )
				{
					if( paf->duration < 25 )
						set_char_color( AT_WHITE, ch );
					if( paf->duration < 6 )
						set_char_color( AT_WHITE + AT_BLINK, ch );
					ch_printf( ch, "(%5d)   ", paf->duration );
				}
				ch_printf( ch, "%-18s\r\n", skill->name );
			}
	}
	return;
}


CMDF( do_inventory )
{
	set_char_color( AT_RED, ch );
	send_to_char( "You are carrying:\r\n", ch );
	show_list_to_char( ch->first_carrying, ch, true, true );
	return;
}


CMDF( do_equipment )
{
	OBJ_DATA *obj;
	int iWear, dam;
	bool found;
	char buf[MAX_STRING_LENGTH];

	set_char_color( AT_RED, ch );
	send_to_char( "You are using:\r\n", ch );
	found = false;


	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		send_to_char( "&GNothing.\r\n", ch );
		return;
	}

	set_char_color( AT_OBJECT, ch );
	for( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		for( obj = ch->first_carrying; obj; obj = obj->next_content )
			if( obj->wear_loc == iWear )
			{
				send_to_char( where_name[iWear], ch );
				if( can_see_obj( ch, obj ) )
				{
					send_to_char( format_obj_to_char( obj, ch, true ), ch );
					strcpy( buf, "" );
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
						if( dam >= 10 )
							strcat( buf, " &G(&gSuperb&G) " );
						else if( dam >= 7 )
							strcat( buf, " &G(&gGood&G) " );
						else if( dam >= 5 )
							strcat( buf, " &G(&gWorn&G) " );
						else if( dam >= 3 )
							strcat( buf, " &G(&gPoor&G) " );
						else if( dam >= 1 )
							strcat( buf, " &G(&gAwful&G) " );
						else if( dam == 0 )
							strcat( buf, " &G(&gBroken&G) " );
						send_to_char( buf, ch );
						break;

					case ITEM_WEAPON:
						dam = INIT_WEAPON_CONDITION - obj->value[0];
						if( dam < 2 )
							strcat( buf, " &G(&gSuperb&G) " );
						else if( dam < 4 )
							strcat( buf, " &G(&gGood&G) " );
						else if( dam < 7 )
							strcat( buf, " &G(&gWorn&G) " );
						else if( dam < 10 )
							strcat( buf, " &G(&gPoor&G) " );
						else if( dam < 12 )
							strcat( buf, " &G(&gAwful&G) " );
						else if( dam == 12 )
							strcat( buf, " &G(&gBroken&G) " );
						send_to_char( buf, ch );
						if( obj->value[3] == WEAPON_BLASTER )
							ch_printf( ch, " &Y%d", obj->value[4] );
						break;
					}
					send_to_char( "\r\n", ch );
				}
				else
					send_to_char( "something.\r\n", ch );
				found = true;
			}
	}

	if( !found )
		send_to_char( "Nothing.\r\n", ch );

	return;
}




void set_title( CHAR_DATA *ch, const char *title )
{
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) )
	{
		bug( "%s: NPC.", __func__ );
		return;
	}

	if( isalpha( title[0] ) || isdigit( title[0] ) )
	{
		buf[0] = ' ';
		mudstrlcpy( buf + 1, title, MAX_STRING_LENGTH );
	}
	else
		mudstrlcpy( buf, title, MAX_STRING_LENGTH );

	STRFREE( ch->pcdata->title );
	ch->pcdata->title = STRALLOC( buf );
}




CMDF( do_title )
{
	if( IS_NPC( ch ) )
		return;

	if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
	{
		send_to_char( "You try but the Force resists you.\r\n", ch );
		return;
	}


	if( argument[0] == '\0' )
	{
		send_to_char( "Change your title to what?\r\n", ch );
		return;
	}

	if( get_trust( ch ) < LEVEL_STAFF && !nifty_is_name( ch->name, remand( argument ) ) )
	{
		send_to_char( "You must include your name somewhere in your title!", ch );
		return;
	}

	argument = smash_tilde_static( argument );
	set_title( ch, argument );
	send_to_char( "Ok.\r\n", ch );
	return;
}

CMDF( do_homepage )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int icq;

	if( IS_NPC( ch ) )
		return;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "\r\n&BSyntax&c:&C Setself &c<&BFlag&c> &c<&BValue&c>\r\n", ch );
		send_to_char( "&BFlag&c:&C Aim, ICQ, Msn, Yim, Real, Homepage, Email.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "icq" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->icq )
				ch->pcdata->icq = 0;
			ch_printf( ch, "Your ICQ number is: %d\r\n", ch->pcdata->icq );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			ch->pcdata->icq = 0;
			send_to_char( "ICQ number cleared.\r\n", ch );
			return;
		}


		icq = atoi( argument );

		if( icq < 1 )
		{
			send_to_char( "Everyone knows ICQ numbers are positive...\r\n", ch );
			return;
		}

		ch->pcdata->icq = icq;
		send_to_char( "ICQ number set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "aim" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->aim )
				ch->pcdata->aim = str_dup( "" );
			ch_printf( ch, "Your Aim name is: %s\r\n", show_tilde( ch->pcdata->aim ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->aim )
				DISPOSE( ch->pcdata->aim );
			ch->pcdata->aim = str_dup( "" );
			send_to_char( "Aim name cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->aim )
			DISPOSE( ch->pcdata->aim );
		ch->pcdata->aim = str_dup( buf );
		send_to_char( "Aim name set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "avatar" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->avatar )
				ch->pcdata->avatar = str_dup( "" );
			ch_printf( ch, "Your Avatar desc is: %s\r\n", show_tilde( ch->pcdata->avatar ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->avatar )
				DISPOSE( ch->pcdata->avatar );
			ch->pcdata->avatar = str_dup( "" );
			send_to_char( "Avatar name cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->avatar )
			DISPOSE( ch->pcdata->avatar );
		ch->pcdata->avatar = str_dup( buf );
		send_to_char( "Avatar name set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "msn" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->msn )
				ch->pcdata->msn = str_dup( "" );
			ch_printf( ch, "Your MSN name is: %s\r\n", show_tilde( ch->pcdata->msn ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->msn )
				DISPOSE( ch->pcdata->msn );
			ch->pcdata->msn = str_dup( "" );
			send_to_char( "MSN name cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->msn )
			DISPOSE( ch->pcdata->msn );
		ch->pcdata->msn = str_dup( buf );
		send_to_char( "MSN name set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "yim" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->yim )
				ch->pcdata->yim = str_dup( "" );
			ch_printf( ch, "Your YIM name is: %s\r\n", show_tilde( ch->pcdata->yim ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->yim )
				DISPOSE( ch->pcdata->yim );
			ch->pcdata->yim = str_dup( "" );
			send_to_char( "YIM name cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->yim )
			DISPOSE( ch->pcdata->yim );
		ch->pcdata->yim = str_dup( buf );
		send_to_char( "YIM name set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "real" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->real )
				ch->pcdata->real = str_dup( "" );
			ch_printf( ch, "Your real name is: %s\r\n", show_tilde( ch->pcdata->real ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->real )
				DISPOSE( ch->pcdata->real );
			ch->pcdata->real = str_dup( "" );
			send_to_char( "Real name cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->real )
			DISPOSE( ch->pcdata->real );
		ch->pcdata->real = str_dup( buf );
		send_to_char( "Real name set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "homepage" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->homepage )
				ch->pcdata->homepage = str_dup( "" );
			ch_printf( ch, "Your homepage is: %s\r\n", show_tilde( ch->pcdata->homepage ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->homepage )
				DISPOSE( ch->pcdata->homepage );
			ch->pcdata->homepage = str_dup( "" );
			send_to_char( "Homepage cleared.\r\n", ch );
			return;
		}

		if( strstr( argument, "://" ) )
			strcpy( buf, argument );
		else
			snprintf( buf, MAX_STRING_LENGTH, "http://%s", argument );
		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->homepage )
			DISPOSE( ch->pcdata->homepage );
		ch->pcdata->homepage = str_dup( buf );
		send_to_char( "Homepage set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "email" ) )
	{
		if( argument[0] == '\0' )
		{
			if( !ch->pcdata->email )
				ch->pcdata->email = str_dup( "" );
			ch_printf( ch, "Your email address is: %s\r\n", show_tilde( ch->pcdata->email ) );
			return;
		}

		if( !str_cmp( argument, "clear" ) )
		{
			if( ch->pcdata->email )
				DISPOSE( ch->pcdata->email );
			ch->pcdata->email = str_dup( "" );
			send_to_char( "Email address cleared.\r\n", ch );
			return;
		}

		strcpy( buf, argument );

		if( strlen( buf ) > 70 )
			buf[70] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->email )
			DISPOSE( ch->pcdata->email );
		ch->pcdata->email = str_dup( buf );
		send_to_char( "Email address set.\r\n", ch );
		return;
	}
	do_homepage( ch, "" );
}


/*
{
	char buf[MAX_STRING_LENGTH];

	if ( IS_NPC(ch) )
	return;

	if ( argument[0] == '\0' )
	{
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Your homepage is: %s\r\n",
		show_tilde( ch->pcdata->homepage ) );
	return;
	}

	if ( !str_cmp( argument, "clear" ) )
	{
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage cleared.\r\n", ch );
	return;
	}

	if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
	else
	snprintf( buf, MAX_STRING_LENGTH, "http://%s", argument );
	if ( strlen(buf) > 70 )
	buf[70] = '\0';

	hide_tilde( buf );
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup(buf);
	send_to_char( "Homepage set.\r\n", ch );
}
*/

/*
 * Set your personal description				-Thoric
 */
CMDF( do_description )
{
	if( IS_NPC( ch ) )
	{
		send_to_char( "Monsters are too dumb to do that!\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		bug( "do_description: no descriptor", 0 );
		return;
	}

	switch( ch->substate )
	{
	default:
		bug( "do_description: illegal substate", 0 );
		return;

	case SUB_RESTRICTED:
		send_to_char( "You cannot use this command from within another command.\r\n", ch );
		return;

	case SUB_NONE:
		ch->substate = SUB_PERSONAL_DESC;
		ch->dest_buf = ch;
		start_editing( ch, ch->description );
		return;

	case SUB_PERSONAL_DESC:
		STRFREE( ch->description );
		ch->description = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
}

/* Ripped off do_description for whois bio's -- Scryn*/
CMDF( do_bio )
{
	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobs can't set bio's!\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		bug( "do_bio: no descriptor", 0 );
		return;
	}

	switch( ch->substate )
	{
	default:
		bug( "do_bio: illegal substate", 0 );
		return;

	case SUB_RESTRICTED:
		send_to_char( "You cannot use this command from within another command.\r\n", ch );
		return;

	case SUB_NONE:
		ch->substate = SUB_PERSONAL_BIO;
		ch->dest_buf = ch;
		start_editing( ch, ch->pcdata->bio );
		return;

	case SUB_PERSONAL_BIO:
		STRFREE( ch->pcdata->bio );
		ch->pcdata->bio = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
}



CMDF( do_report )
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );

	if( IS_AFFECTED( ch, AFF_POSSESS ) )
	{
		send_to_char( "You can't do that in your current state of mind!\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && !str_cmp( arg, "money" ) )
	{
		ch_printf( ch, "You announce you have %s on hand.\r\n", num_punct( ch->gold ) );
		snprintf( buf, MAX_STRING_LENGTH, "$n announces $e has %s on hand.\r\n", num_punct( ch->gold ) );
		act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
		return;
	}

	if( !IS_NPC( ch ) && !str_cmp( arg, "bank" ) )
	{
		ch_printf( ch, "You announce you have %s dollars in the bank.\r\n", num_punct( ch->pcdata->bank ) );
		snprintf( buf, MAX_STRING_LENGTH, "$n announce $e has %s dollars in the bank.\r\n", num_punct( ch->pcdata->bank ) );
		act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
		return;
	}

	if( !IS_NPC( ch ) && !str_cmp( arg, "levels" ) )
	{
		ch_printf( ch, "\n\rYou announce your combined levels: %d.\r\n",
			ch->skill_level[COMBAT_ABILITY] + ch->skill_level[PILOTING_ABILITY] + ch->skill_level[ENGINEERING_ABILITY] +
			ch->skill_level[HUNTING_ABILITY] + ch->skill_level[SMUGGLING_ABILITY] + ch->skill_level[DIPLOMACY_ABILITY] +
			ch->skill_level[LEADERSHIP_ABILITY] + ch->skill_level[ESPIONAGE_ABILITY] );
		snprintf( buf, MAX_STRING_LENGTH, "$n announces $e has %d combined levels.\r\n",
			ch->skill_level[COMBAT_ABILITY] + ch->skill_level[PILOTING_ABILITY] + ch->skill_level[ENGINEERING_ABILITY] +
			ch->skill_level[HUNTING_ABILITY] + ch->skill_level[SMUGGLING_ABILITY] + ch->skill_level[DIPLOMACY_ABILITY] +
			ch->skill_level[LEADERSHIP_ABILITY] + ch->skill_level[ESPIONAGE_ABILITY] );
		act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
		return;
	}

	ch_printf( ch, "You report: %ld/%ldhp   %d/%dmv.\r\n", ch->hit, ch->max_hit, ch->move, ch->max_move );
	snprintf( buf, MAX_STRING_LENGTH, "$n reports: %ld/%ldhp   %d/%dmv.\r\n", ch->hit, ch->max_hit, ch->move, ch->max_move );
	act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

	return;
}

CMDF( do_entex )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[300];
	char buf1[MAX_STRING_LENGTH];
	char entmess[MAX_STRING_LENGTH];
	char extmess[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	/*	if( !has_rate( ch, RATE_ENTEX ) )
		{
			send_to_char( "You don't rate enter/exit text! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	argument = smash_tilde_static( argument );
	argument = smash_percent_static( argument );

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		snprintf( entmess, MAX_STRING_LENGTH, "&B%s &RH&ra&Rs &GC&go&Gm&ge &YT&Oo &CP&cl&Ca&cy&z!", ch->name );
		snprintf( extmess, MAX_STRING_LENGTH, "&B%s &GD&ge&cc&Ci&cd&ge&Gd &YT&Oo &RR&re&pjo&ri&Rn &CT&ch&Ce &zR&Wea&zl &PW&po&Pr&pl&Pd&z.",
			ch->name );

		send_to_char( "\r\n&CSyntax: Entex <enter/exit> <messege>\r\n", ch );
		send_to_char( "&G----------------------------------------------\r\n", ch );
		ch_printf( ch, "&BEnter&b: %s\r\n", ( ch->pcdata->enter[0] != '\0' ) ? ch->pcdata->enter : entmess );
		ch_printf( ch, "&BExit&b: %s\r\n", ( ch->pcdata->exit[0] != '\0' ) ? ch->pcdata->exit : extmess );

		return;
	}
	if( !str_cmp( arg, "enter" ) )
	{

		mudstrlcpy( buf, argument, MSL );

		if( strlen_color( buf ) > 128 )
			buf[128] = '\0';

		/*
		 * Can add a list of pre-set prompts here if wanted.. perhaps
		 * 'prompt 1' brings up a different, pre-set prompt
		 */
		if( !str_cmp( argument, "default" ) )
		{
			if( ch->pcdata->enter )
				DISPOSE( ch->pcdata->enter );
			snprintf( buf1, MAX_STRING_LENGTH, "&B%s &RH&ra&Rs &GC&go&Gm&ge &YT&Oo &CP&cl&Ca&cy&z!", ch->name );
			ch->pcdata->enter = str_dup( buf1 );
			send_to_char( "Set.\r\n", ch );
			return;
		}

		mudstrlcpy( buf, argument, MSL );

		if( strlen_color( buf ) > 128 )
			buf[128] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->enter )
			DISPOSE( ch->pcdata->enter );
		ch->pcdata->enter = str_dup( buf );
		send_to_char( "Set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "exit" ) )
	{

		mudstrlcpy( buf, argument, MSL );

		if( strlen_color( buf ) > 128 )
			buf[128] = '\0';

		/*
		 * Can add a list of pre-set prompts here if wanted.. perhaps
		 * 'prompt 1' brings up a different, pre-set prompt
		 */
		if( !str_cmp( argument, "default" ) )
		{
			if( ch->pcdata->exit )
				DISPOSE( ch->pcdata->exit );
			snprintf( buf1, MAX_STRING_LENGTH, "&B%s &GD&ge&cc&Ci&cd&ge&Gd &YT&Oo &RR&re&pjo&ri&Rn &CT&ch&Ce &zR&Wea&zl &PW&po&Pr&pl&Pd&z.",
				ch->name );
			ch->pcdata->exit = str_dup( buf1 );
			send_to_char( "Set.\r\n", ch );
			return;
		}

		mudstrlcpy( buf, argument, MSL );

		if( strlen_color( buf ) > 128 )
			buf[128] = '\0';

		hide_tilde( buf );
		if( ch->pcdata->exit )
			DISPOSE( ch->pcdata->exit );
		ch->pcdata->exit = str_dup( buf );
		send_to_char( "Set.\r\n", ch );
		return;
	}

}


CMDF( do_prompt )
{
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
	{
		send_to_char( "NPC's can't change their prompt..\r\n", ch );
		return;
	}
	argument = smash_tilde_static( argument );
	one_argument( argument, arg );
	if( !*arg )
	{
		send_to_char( "Set prompt to what? (try help prompt)\r\n", ch );
		return;
	}
	if( ch->pcdata->prompt )
		STRFREE( ch->pcdata->prompt );

	char prompt[128];
	mudstrlcpy( prompt, argument, 128 );

	/*
	 * Can add a list of pre-set prompts here if wanted.. perhaps
	 * 'prompt 1' brings up a different, pre-set prompt
	 */
	if( !str_cmp( arg, "default" ) )
		ch->pcdata->prompt = STRALLOC( "" );
	else
		ch->pcdata->prompt = STRALLOC( prompt );
	send_to_char( "Ok.\r\n", ch );
}

CMDF( do_pkchange )
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg );

	if( IS_NPC( ch ) )
		return;

	if( arg[0] == '\0' )
	{
		send_to_char( "&wSyntax: gopk \"Yes\"\r\n", ch );
		send_to_char( "&YRemember to read help PK first!\r\n", ch );
		send_to_char( "\r\n&wFor Pure PK:\n\rSyntax: gopk \"Pure\"\r\n", ch );
		return;
	}

	if( !strcmp( arg, "yes" ) )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			send_to_char( "You're already PK!\r\n", ch );
			return;
		}

		xSET_BIT( ch->act, PLR_PKER );
		send_to_char( "&zYou are now &RPK&z. No going back now...\r\n", ch );
		return;
	}


	if( !strcmp( arg, "pure" ) )
	{
		if( xIS_SET( ch->act, PLR_PPKER ) )
		{
			send_to_char( "You're already PPK!\r\n", ch );
			return;
		}

		if( !xIS_SET( ch->act, PLR_PKER ) )
		{
			xSET_BIT( ch->act, PLR_PKER );
		}

		xSET_BIT( ch->act, PLR_PPKER );
		send_to_char( "&zYou are now &RPPK&z. Enjoy the havoc!\r\n", ch );
		return;
	}


	do_pkchange( ch, "" );
	return;

}

/*
void do_accept( CHAR_DATA *ch, char *argument )
{
	char	 arg[MAX_INPUT_LENGTH];
	CHAR_DATA	*victim;

	one_argument( argument, arg );

	if ( IS_NPC( ch ) ) return;

	if ( ch->pcdata->spouse != NULL && ch->pcdata->spouse[0] != '\0' )
	{
	if ( IS_EXTRA( ch, EXTRA_MARRIED ) )
		send_to_char( "But you are already married!\r\n", ch );
	else
		send_to_char( "But you are already engaged!\r\n", ch );
	return;
	}

	if ( arg[0] == '\0' )
	{
	send_to_char( "Who's proposal of marriage do you wish to accept?\r\n", ch );
	return;
	}

	if ( ( victim = get_char_room( ch, arg) ) == NULL )
	{
	send_to_char( "They are not here.\r\n", ch );
	return;
	}

	if ( IS_NPC( victim ) )
	{
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
	}

	if ( victim->pcdata->spouse != NULL && victim->pcdata->spouse[0] != '\0' )
	{
	if ( IS_EXTRA( victim, EXTRA_MARRIED ) )
		send_to_char( "But they are already married!\r\n", ch );
	else
		send_to_char( "But they are already engaged!\r\n", ch );
	return;
	}

	if ( victim->pcdata->propose != ch )
	{
	send_to_char( "But they haven't proposed to you!\r\n", ch );
	return;
	}

	if ( ( ch->sex == SEX_MALE && victim->sex == SEX_FEMALE) ||
	 ( ch->sex == SEX_FEMALE && victim->sex == SEX_MALE) )
	{
	victim->pcdata->propose = NULL;
	ch->pcdata->propose = NULL;
	STRFREE( victim->pcdata->spouse );
	victim->pcdata->spouse = STRALLOC( ch->name );
	STRFREE( ch->pcdata->spouse );
	ch->pcdata->spouse = STRALLOC( victim->name );
	act( AT_SOCIAL, "You accept $S offer of marriage.", ch, NULL, victim, TO_CHAR );
	act( AT_SOCIAL, "$n accepts $N's offer of marriage.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SOCIAL, "$n accepts your offer of marriage.", ch, NULL, victim, TO_VICT );
	return;
	}
	send_to_char( "I don't think that would be a very good idea...\r\n",
ch );
	return;
}

void do_breakup( CHAR_DATA *ch, char *argument )
{
	char	 arg[MAX_INPUT_LENGTH];
	CHAR_DATA	*victim;

	one_argument( argument, arg );

	if ( IS_NPC( ch ) )
	return;

	if ( ch->pcdata->spouse != NULL && ch->pcdata->spouse[0] != '\0' )
	{
	if ( IS_EXTRA( ch, EXTRA_MARRIED ) )
	{
		send_to_char( "You'll have to get divorced.\r\n", ch );
		return;
	}
	}
	else
	{
	send_to_char( "But you are not even engaged!\r\n", ch );
	return;
	}

	if ( arg[0] == '\0' )
	{
	send_to_char( "Who do you wish to break up with?\r\n", ch );
	return;
	}

	if ( ( victim = get_char_room( ch, arg) ) == NULL )
	{
	send_to_char( "They are not here.\r\n", ch );
	return;
	}

	if ( IS_NPC( victim ) )
	{
	send_to_char( "Not on NPC's.\r\n", ch );
	return;
	}

	if ( victim->pcdata->spouse != NULL && victim->pcdata->spouse[0] != '\0' )
	{
	if ( IS_EXTRA( victim, EXTRA_MARRIED ) )
	{
		send_to_char( "They'll have to get divorced.\r\n", ch );
		return;
	}
	}
	else
	{
	send_to_char( "But they are not even engaged!\r\n", ch );
	return;
	}

	if (!str_cmp( ch->name, victim->pcdata->spouse ) &&
	!str_cmp( victim->name, ch->pcdata->spouse ) )
	{
	STRFREE(victim->pcdata->spouse);
	victim->pcdata->spouse = STRALLOC( "" );
	STRFREE( ch->pcdata->spouse);
	ch->pcdata->spouse = STRALLOC( "" );
	act( AT_SOCIAL, "You break off your engagement with $M.", ch, NULL, victim, TO_CHAR );
	act( AT_SOCIAL, "$n breaks off $n engagement with $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SOCIAL, "$n breaks off $s engagement with you.", ch, NULL, victim, TO_VICT );
	return;
	}
	send_to_char( "You are not engaged to them.\r\n", ch );
	return;
}
*/

void do_fight( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *random;

	if( iswar == true )
	{
		send_to_char( "&RWait until the competition is done!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ARENA ) )
	{
		send_to_char( "I don't think so.\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	if( ch->in_room != get_room_index( 61 ) )
	{
		send_to_char( "&RYou aren't in the entrance room!\r\n", ch );
		return;
	}
	else
	{
		act( AT_MAGIC, "$n dissapears in a flash!", ch, NULL, NULL, TO_ROOM );
		ch->retran = ch->in_room->vnum;
		random = get_room_index( ( number_range( 29, 43 ) ) );
		char_from_room( ch );
		char_to_room( ch, random );
		SET_BIT( ch->pcdata->flags, PCFLAG_ARENA );
		snprintf( buf, MAX_STRING_LENGTH, "&P&p-&P=&RA&rr&Re&rn&Ra&P=&p- &B%s &has entered the Arena!", ch->name );
		echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
		act( AT_MAGIC, "$n appears in a flash!", ch, NULL, NULL, TO_ROOM );
		inwar++;
		do_look( ch, "auto" );
		return;
	}
}

//Global
int slotland;


CMDF( do_slot )
{
	int num1;
	int num2;
	int num3;
	int slotwin;

	if( IS_NPC( ch ) )
		return;

	if( ( ch->in_room != get_room_index( 4125 ) )
		&& ( ch->in_room != get_room_index( 4126 ) ) && ( ch->in_room != get_room_index( 4127 ) ) )
	{
		send_to_char( "&RThere aren't any slot machines here!\r\n", ch );
		return;
	}

	if( ch->gold < 100 )
	{
		send_to_char( "You need 100 dollars to use the slot machine.\r\n", ch );
		return;
	}

	ch->gold -= 100;

	act( AT_PLAIN, "$n just pulled the lever on a slot machine.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You insert 100 dollars and pull the slot machine lever.\r\n", ch );
	pull_slot( ch );
	num1 = slotland;
	pull_slot( ch );
	num2 = slotland;
	pull_slot( ch );
	num3 = slotland;
	send_to_char( "\r\n\r\n", ch );

	if( ( num1 == num2 || num2 == num3 ) && ( num1 != num3 ) )
	{
		if( num1 == 9 || num2 == 9 || num3 == 9 )
		{
			send_to_char( "You recieved atleast 1 bomb! Instant LOSER!\r\n", ch );
			return;
		}
		send_to_char( "&RD&rI&RN&rG &GD&gI&GN&gG &YD&OI&YN&OG&P!&p!&P!\r\n", ch );
		send_to_char( "&PYou win &Y300 &Pdollars!\r\n", ch );
		act( AT_GREEN, "$n's slot machine starts ringing!", ch, NULL, NULL, TO_ROOM );
		ch->gold += 300;
		return;
	}

	if( num1 == num2 && num2 == num3 )
	{
		switch( num1 )
		{
		case 0:
			slotwin = 10000;
			break;  /* bar */
		case 1:
			slotwin = 2000;
			break;  /* Lime */
		case 2:
			slotwin = 50000;
			break;  /* 777 */
		case 3:
			slotwin = 2500;
			break;  /* Cherry */
		case 4:
			slotwin = 1500;
			break;  /* Grapes */
		case 5:
			slotwin = 2250;
			break;  /* Water Melon */
		case 6:
			slotwin = 1000;
			break;  /* Blue Berries */
		case 7:
			slotwin = 3500;
			break;  /* Diamond */
		case 8:
			slotwin = 1200;
			break;  /* ^_^ */
		case 9:
			if( ch->gold > 30000 )
			{
				slotwin = -30000;
			}
			else
			{
				slotwin = -ch->gold;
			}
			break;  /* Bomb */
		default:
			slotwin = 0;
			send_to_char( "Machine is broken. Please contact an IMM.\r\n", ch );
			break;
		}

		ch->gold += slotwin;
		send_to_char( "&RD&rI&RN&rG &GD&gI&GN&gG &YD&OI&YN&OG&P!&p!&P!\r\n", ch );
		ch_printf( ch, "&PYou win &Y%d&P dollars!\r\n", slotwin );
		act( AT_GREEN, "$n's slot machine starts ringing!", ch, NULL, NULL, TO_ROOM );
		return;
	}

	send_to_char( "You Lost!\r\n", ch );
	return;
}

/* Roll the slots for do_slot - Talon */
void pull_slot( CHAR_DATA *ch )
{
	slotland = ( number_range( 0, 9 ) );
	switch( slotland )
	{
	case 0:
		send_to_char( "&z[&WBAR&z] ", ch );
		break;
	case 1:
		send_to_char( "&G(&O'&G) ", ch );
		break;
	case 2:
		send_to_char( "&Y7&O7&Y7 ", ch );
		break;
	case 3:
		send_to_char( "&R(&z'&R) ", ch );
		break;
	case 4:
		send_to_char( "&P&po8&G- ", ch );
		break;
	case 5:
		send_to_char( "&G[&g=&G] ", ch );
		break;
	case 6:
		send_to_char( "&bo&B8&z- ", ch );
		break;
	case 7:
		send_to_char( "&c<&C> ", ch );
		break;
	case 8:
		send_to_char( "&B^^&R_&B^^ ", ch );
		break;
	case 9:
		send_to_char( "&z[&RT&rN&RT&z]&w-&Y* ", ch );
		break;
	default:
		//        default:
		send_to_char( "&W&wMachine Broken ", ch );
		break;
		break;
	}
	return;
}

LOCKER_DATA *load_locker( CHAR_DATA *ch )
{
	LOCKER_DATA *locker;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	char strsave[MAX_INPUT_LENGTH];
	FILE *fp;
	char buf[MAX_INPUT_LENGTH];
	int count = 0;

	locker = NULL;

	/*
	 * Find the locker file
	 */

	snprintf( strsave, MAX_INPUT_LENGTH, "%s%s", STORAGE_DIR, capitalize( ch->name ) );

	if( ( fp = FileOpen( strsave, "r" ) ) != NULL )
	{
		CREATE( locker, LOCKER_DATA, 1 );
		locker->capacity = 0;
		locker->holding = 0;
		locker->flags = 0;
		locker->room = 0;

		for( ;; )
		{
			char letter;
			const char *word;
			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}
			if( letter != '#' )
			{
				bug( "Load_locker: # not found.", 0 );
				bug( ch->name, 0 );
				break;
			}
			word = fread_word( fp );
			if( !str_cmp( word, "STORAGE" ) )
			{
				fread_locker( ch, locker, fp );
				room = generate_virtual_room( ch->in_room );
				ch->pcdata->locker_vnum = ch->in_room->vnum;
				ch->pcdata->locker_room = room;

				/*
				 * Move the character to the locker room
				 */
				char_from_room( ch );
				//                ch->retran = ch->in_room->vnum;
				char_to_room( ch, room );
			}
			else if( !str_cmp( word, "OBJECT" ) )  /* Objects  */
				fread_obj( ch, fp, OS_LOCKER );
			else if( !str_cmp( word, "END" ) ) /* Done     */
				break;
			else
			{
				bug( "Load_locker: bad section.", 0 );
				snprintf( buf, MAX_STRING_LENGTH, "[%s]", word );
				bug( buf, 0 );
				bug( ch->name, 0 );
				break;
			}
		}

		FileClose( fp );

		/*
		 * Total the weight of the contents
		 */
		for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
		{
			count++;
			locker->holding = count;
		}
	}

	return locker;
}

void fread_locker( CHAR_DATA *ch, LOCKER_DATA *locker, FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	for( ;; )
	{
		word = feof( fp ) ? "End" : fread_word( fp );
		fMatch = false;

		switch( UPPER( word[0] ) )
		{
		case '*':
			fMatch = true;
			fread_to_eol( fp );
			break;

		case 'C':
			KEY( "Capacity", locker->capacity, fread_number( fp ) );
			break;

		case 'F':
			KEY( "Flags", locker->flags, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
				return;
			break;

		case 'R':
			KEY( "Room", locker->room, fread_number( fp ) );
			break;
		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_locker: no match: %s", word );
			bug( buf, 0 );
		}
	}

}

void fwrite_locker( CHAR_DATA *ch )
{
	/*
	 * Variables
	 */
	FILE *fp = NULL;
	OBJ_DATA *obj;
	char strsave[MAX_INPUT_LENGTH];

	if( !ch->pcdata->locker )
	{
		bug( "Fwrite_storage: NULL object.", 0 );
		bug( ch->name, 0 );
		return;
	}

	snprintf( strsave, MIL, "%s%s", STORAGE_DIR, capitalize( ch->name ) );

	if( ( fp = FileOpen( strsave, "w" ) ) != NULL )
	{
		/*
		 * Save the locker details
		 */
		fprintf( fp, "#STORAGE\n" );
		fprintf( fp, "Capacity     %d\n", ch->pcdata->locker->capacity );
		fprintf( fp, "Flags        %d\n", ch->pcdata->locker->flags );
		fprintf( fp, "End\n\n" );

		/*
		 * Save the objects in the locker room
		 */
		for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
		{
			fwrite_obj( ch, obj, fp, 0, OS_LOCKER, false );
		}

		fprintf( fp, "#END\n" );
		FileClose( fp );
	}
	return;
}

CMDF( do_storage )
{
	/*
	 * Variables
	 */
	char buf[MAX_INPUT_LENGTH];
	LOCKER_DATA *locker;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int count = 0;

	if( IS_NPC( ch ) )
		return;

	/*	if( !has_rate( ch, RATE_STORAGE ) )
		{
			send_to_char( "You don't rate storage! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	if( get_timer( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL( ch ) )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			set_char_color( AT_RED, ch );
			send_to_char( "Cool down first!\r\n", ch );
			return;
		}
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You wish!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: storage <enter/leave/pay>\r\n", ch );
		ch_printf( ch, "You have $%d owed.\r\n", ch->pcdata->storagecost );
		return;
	}

	if( !str_cmp( arg, "pay" ) )
	{
		if( ch->gold < ch->pcdata->storagecost )
		{
			send_to_char( "You don't have enough money on hand to pay that!\r\n", ch );
			return;
		}
		ch->gold -= ch->pcdata->storagecost;
		ch->pcdata->storagecost = 0;
		send_to_char( "Your storage cost has been paid.\r\n", ch );
		do_save( ch, "" );
		return;
	}

	else if( !str_cmp( arg, "enter" ) )
	{
		if( ch->pcdata->locker )
		{
			send_to_char( "Your storage space is already open.\r\n", ch );
			return;
		}

		if( !xIS_SET( ch->in_room->room_flags, ROOM_STORAGE ) )
		{
			send_to_char( "This isn't a storage room.\r\n", ch );
			return;
		}

		if( ch->pcdata->storagecost >= 25000 )
		{
			send_to_char( "Pay off your storage debt, you lousy non-debt payer type person!\r\n", ch );
			return;
		}

		locker = load_locker( ch );

		if( !locker )
		{
			send_to_char( "You don't have a storage space.\r\n", ch );
			return;
		}

		ch->pcdata->locker = locker;

		do_look( ch, "auto" );
		send_to_char( "You are now in your storage space.\r\n", ch );
	}
	else if( !str_cmp( arg, "leave" ) )
	{
		if( !ch->pcdata->locker )
		{
			send_to_char( "You are not currently in your storage space.\r\n", ch );
			return;
		}

		locker = ch->pcdata->locker;

		/*
		 * Total the weight of the contents
		 */
		locker->holding = 0;

		for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
		{
			locker->holding = locker->holding + ( obj->count );
			count += 1;
			if( obj->item_type == ITEM_CONTAINER )
			{
				send_to_char( "You may not leave containers in the storage room.\r\n", ch );
				return;
			}

			if( obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_DROID_CORPSE )
			{
				send_to_char( "A corpse? Do you know what'll happen to that thing in here?\r\n", ch );
				return;
			}

			if( obj->item_type == ITEM_MONEY )
			{
				send_to_char( "Use a bank for money, retard.\r\n", ch );
				return;
			}
			if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
			{
				send_to_char( "You can't leave an artifact in here.\r\n", ch );
				return;
			}

			if( count > 50 )
			{
				snprintf( buf, MAX_STRING_LENGTH, "You have too many items in your storage space. Pick an item or two up then leave.\r\n" );
				send_to_char( buf, ch );
				return;
			}

			if( locker->holding > 50 )
			{
				send_to_char( "You have too many items in your storage space, pick an item or two up, then leave.\r\n", ch );
				return;
			}

		}


		/*
		 * Save the locker
		 */
		fwrite_locker( ch );

		/*
		 * Return the player to the real world.
		 */
		char_from_room( ch );
		//        char_to_room( ch, get_room_index( ch->retran ) );
		char_to_room( ch, get_room_index( ch->pcdata->locker_vnum ) );

		delete_locker( ch );
		save_char_obj( ch );
		do_look( ch, "auto" );
		send_to_char( "Your storage space has been saved.\r\n", ch );
	}
	else
	{
		send_to_char( "Syntax: Storage <enter/leave/pay>\r\n", ch );
	}

	return;
}

ROOM_INDEX_DATA *generate_virtual_room( ROOM_INDEX_DATA *in_room )
{
	ROOM_INDEX_DATA *room = NULL;
	int serial;
	short hash;
	bool found;
	int vnum = in_room->vnum;

	do
	{
		found = false;
		serial = ( vnum << 16 ) | vnum;

		hash = serial % 64;

		for( room = vroom_hash[hash]; room; room = room->next )
		{
			if( room->vnum == serial )
			{
				found = true;
				break;
			}
		}
		vnum++;
	} while( found );

	if( !found )
	{
		CREATE( room, ROOM_INDEX_DATA, 1 );
		room->area = in_room->area;
		room->vnum = serial;
		room->tele_vnum = 1;
		room->sector_type = SECT_INSIDE;
		xCLEAR_BITS( room->room_flags );
		xSET_BIT( room->room_flags, ROOM_INDOORS ); 
		xSET_BIT( room->room_flags, ROOM_PRIVATE );
		xSET_BIT( room->room_flags, ROOM_NO_RECALL );
		room->next = vroom_hash[hash];
		vroom_hash[hash] = room;
		++top_vroom;
	}

	if( room )
	{
		if( room->name )
			STRFREE( room->name );
		if( room->description )
			STRFREE( room->description );

		room->name = STRALLOC( "Storage Room" );
		room->description = STRALLOC( "Type STORAGE LEAVE to leave this room.\r\n"
			"After leaving all of your items will be saved,\r\n" "except for containers.\r\n" );
	}
	return room;
}

void delete_locker( CHAR_DATA *ch )
{
	/*
	 * Clean-up the locker room -- Remove the loaded objects
	 */
	while( ch->pcdata->locker_room->first_content )
	{
		extract_obj( ch->pcdata->locker_room->first_content );
	}

	/*
	 * Kill the locker object
	 */
	DISPOSE( ch->pcdata->locker );
	ch->pcdata->locker = NULL;

	ch->pcdata->locker_vnum = 0;
	ch->pcdata->locker_room = NULL;

	/*
	 * Need to delete the virtual room
	 */
	clear_vrooms( );
}

CMDF( do_makestorage )
{
	FILE *fp;
	char filename[256];
	LOCKER_DATA *locker;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_INPUT_LENGTH];
	struct stat fst;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makestorage <player name>\r\n", ch );
		return;
	}

	/*
	 * Is this a valid player?
	 */
	strcpy( name, capitalize( argument ) );
	snprintf( buf, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( argument[0] ), name );
	if( stat( buf, &fst ) == -1 )
	{
		snprintf( buf, MAX_STRING_LENGTH, "Player file for %s does not exist!\r\n", name );
		send_to_char( buf, ch );
		return;
	}

	snprintf( filename, sizeof(filename), "%s%s", STORAGE_DIR, name );

	if( stat( filename, &fst ) != -1 )
	{
		snprintf( buf, MAX_STRING_LENGTH, "Storage file for %s already exists!\r\n", name );
		send_to_char( buf, ch );
		return;
	}

	CREATE( locker, LOCKER_DATA, 1 );
	locker->capacity = STORAGE_CAPACITY;
	locker->flags = 0;

	if( ( fp = FileOpen( filename, "w" ) ) != NULL )
	{
		/*
		 * Save the locker details
		 */
		fprintf( fp, "#STORAGE\n" );
		fprintf( fp, "Capacity     %d\n", locker->capacity );
		fprintf( fp, "Flags        %d\n", locker->flags );
		fprintf( fp, "End\n\n" );

		fprintf( fp, "#END\n" );
		FileClose( fp );

		snprintf( buf, MAX_STRING_LENGTH, "Storage space [%s] created for %s\r\n", filename, argument );
		log_printf( buf, ch );
	}

	return;
}

CMDF( do_buystorage )
{
	char buf[MAX_STRING_LENGTH];
	struct stat fst;

	if( !xIS_SET( ch->in_room->room_flags, ROOM_STORAGE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( ch->gold >= STORAGE_PRICE )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s%s", STORAGE_DIR, ch->name );

		if( stat( buf, &fst ) != -1 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "You already have storage space!\r\n" );
			send_to_char( buf, ch );
		}
		else
		{
			ch->gold -= STORAGE_PRICE;

			do_makestorage( ch, ch->name );

			send_to_char( "Your storage space has now been created.\r\n", ch );
			ch->pcdata->storagetimer = sysdata.storagetimer;
		}
	}
	else
	{
		send_to_char( "You do not have the money to purchase storage space!\r\n", ch );
	}
	return;
}

void update_storage( CHAR_DATA *ch )
{
	int diff;

	if( ch->pcdata->storagetimer == 0 )
		return;

	if( ch->pcdata->storagetimer != sysdata.storagetimer )
	{
		diff = ( sysdata.storagetimer - ch->pcdata->storagetimer );
		ch->pcdata->storagecost = ( diff * 500 );
		ch->pcdata->storagetimer = sysdata.storagetimer;
	}

	return;
}

CMDF( do_pktimer )
{
	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !xIS_SET( ch->act, PLR_PKER ) || !xIS_SET( ch->act, PLR_PPKER ) )
	{
		send_to_char( "&YYou must either be PK or PPK to check your status.\r\n", ch );
		return;
	}

	if( ch->wfm_timer == 0 )
		send_to_char( "&GYou can PK for bounty experience!\r\n", ch );
	else
		ch_printf( ch, "&RYou must wait %d minutes to PK for bounty experience.\r\n", ch->wfm_timer );

	return;
}
