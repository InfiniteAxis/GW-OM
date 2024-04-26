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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"

/* from swskills.c */
void add_reinforcements( CHAR_DATA *ch );

/*
 * Local functions.
 */
void mail_count( CHAR_DATA *ch );
int hit_gain( CHAR_DATA *ch );
int move_gain( CHAR_DATA *ch );
void gain_addiction( CHAR_DATA *ch );
void mobile_update( void );
void weather_update( void );
void bounty_update( void );
void update_taxes( void );
void char_update( void );
void obj_update( void );
void aggr_update( void );
void room_act_update( void );
void obj_act_update( void );
void char_check( void );
void drunk_randoms( CHAR_DATA *ch );
void halucinations( CHAR_DATA *ch );
void zeromares( CHAR_DATA *ch );
void subtract_times( struct timeval *etime, struct timeval *stime );
void spouse_update( void );
void healchar( void );
void suit_timer( void );
void reload_heartless( void );
void calc_season( void );
void save_sysdata( SYSTEM_DATA sys );
void update_charge( void );
void aris_affect( CHAR_DATA *ch, AFFECT_DATA *paf );

/*
 * Global Variables
 */

CHAR_DATA *gch_prev;
OBJ_DATA *gobj_prev;

CHAR_DATA *timechar;

const char *corpse_descs[] = {
   "The corpse of %s will soon be gone.",
   "The corpse of %s lies here.",
   "The corpse of %s lies here.",
   "The corpse of %s lies here.",
   "The corpse of %s lies here."
};

const char *d_corpse_descs[] = {
   "The shattered remains %s will soon be gone.",
   "The shattered remains %s are here.",
   "The shattered remains %s are here.",
   "The shattered remains %s are here.",
   "The shattered remains %s are here."
};

extern int top_exit;

/*
 * Advancement stuff.
 */
int max_level( CHAR_DATA *ch, int ability )
{
	int level = 1007;

	if( IS_NPC( ch ) )
		return 1000;

	level = URANGE( 1, level, 1000 );

	if( ability == ESPIONAGE_ABILITY )
		return 1;
	else
		return 1000;
}

void advance_level( CHAR_DATA *ch, int ability )
{
	int add_hp, add_move, add_dollars;
	bool moveGain = false;
	char buf[MSL], buf2[MSL];

	if( !IS_IMMORTAL( ch ) )
	{
		add_hp = con_app[get_curr_con( ch )].hitp + number_range( get_curr_con( ch ) / 5, get_curr_con( ch ) );
		add_move = number_range( 5, ( get_curr_con( ch ) + get_curr_dex( ch ) ) / 4 );
		add_dollars = ch->skill_level[ability];

		add_hp = UMAX( 1, add_hp );
		add_move = UMAX( 10, add_move );

		ch->max_hit += add_hp;

		if( ability == ENGINEERING_ABILITY || ability == SMUGGLING_ABILITY || ability == LEADERSHIP_ABILITY )
		{
			ch->max_move += add_move;
			moveGain = true;
		}
		if( ability == HUNTING_ABILITY || ability == SMUGGLING_ABILITY )
			add_dollars *= 10;

		ch->gold += add_dollars;

		snprintf( buf, MSL, "&WYour gain is" );
		snprintf( buf2, MSL, " &R%d&W/&r%ld &Whealth,", add_hp, ch->max_hit );
		mudstrlcat( buf, buf2, MSL );

		if( moveGain == true )
		{
			snprintf( buf2, MSL, " &G%d&W/&g%d &Wmoves,", add_move, ch->max_move );
			mudstrlcat( buf, buf2, MSL );
		}
		snprintf( buf2, MSL, " %s&Y%s &Wcredits!&D\r\n", "and ", num_punct( add_dollars ) );
		mudstrlcat( buf, buf2, MSL );
		send_to_char( buf, ch );
	}

	if( ch->top_level < ch->skill_level[ability] && ch->top_level < 1000 )
	{
		ch->top_level = URANGE( 1, ch->skill_level[ability], 1000 );
	}

	if( !IS_NPC( ch ) )
		xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );

	return;
}

void gain_exp( CHAR_DATA *ch, int gain, int ability )
{
	if( IS_NPC( ch ) )
		return;

	ch->experience[ability] = UMAX( 0, ch->experience[ability] + gain );

	if( NOT_AUTHED( ch ) && ch->experience[ability] >= exp_level( ch->skill_level[ability] + 1 ) )
	{
		send_to_char( "You can not ascend to a higher level until you are authorized.\r\n", ch );
		ch->experience[ability] = ( exp_level( ch->skill_level[ability] + 1 ) - 1 );
		return;
	}

	while( ch->experience[ability] >= exp_level( ch->skill_level[ability] + 1 ) )
	{
		if( ch->skill_level[ability] >= max_level( ch, ability ) )
		{
			ch->experience[ability] = ( exp_level( ch->skill_level[ability] + 1 ) - 1 );
			return;
		}
		set_char_color( AT_WHITE + AT_BLINK, ch );
		ch_printf( ch, "You have now obtained %s level %d!\r\n", ability_name[ability], ++ch->skill_level[ability] );
		advance_level( ch, ability );
	}
	return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
	int gain;

	if( IS_NPC( ch ) )
	{
		gain = ch->top_level;
	}
	else
	{
		gain = UMIN( 5, ch->top_level );

		switch( ch->position )
		{
		case POS_DEAD:
			return 0;
		case POS_MORTAL:
			return -25;
		case POS_INCAP:
			return -20;
		case POS_STUNNED:
			return get_curr_con( ch ) * 2;
		case POS_SLEEPING:
			gain += get_curr_con( ch ) * 1.5;
			break;
		case POS_RESTING:
			gain += get_curr_con( ch );
			break;
		}

		if( ch->pcdata->condition[COND_FULL] == 0 )
			gain /= 2;

		if( ch->pcdata->condition[COND_THIRST] == 0 )
			gain /= 2;

	}

	if( IS_AFFECTED( ch, AFF_POISON ) )
		gain /= 4;

	return UMIN( gain, ch->max_hit - ch->hit );
}

int move_gain( CHAR_DATA *ch )
{
	int gain;

	if( IS_NPC( ch ) )
	{
		gain = ch->top_level;
	}
	else
	{
		gain = UMAX( 15, 2 * ch->top_level );

		switch( ch->position )
		{
		case POS_DEAD:
			return 0;
		case POS_MORTAL:
			return -1;
		case POS_INCAP:
			return -1;
		case POS_STUNNED:
			return 1;
		case POS_SLEEPING:
			gain += get_curr_dex( ch ) * 2;
			break;
		case POS_RESTING:
			gain += get_curr_dex( ch );
			break;
		}


		if( ch->pcdata->condition[COND_FULL] == 0 )
			gain /= 2;

		if( ch->pcdata->condition[COND_THIRST] == 0 )
			gain /= 2;
	}

	if( IS_AFFECTED( ch, AFF_POISON ) )
		gain /= 4;

	if( get_age( ch ) > 500 )
		gain /= 10;
	else if( get_age( ch ) > 300 )
		gain /= 5;
	else if( get_age( ch ) > 200 )
		gain /= 2;

	return UMIN( gain, ch->max_move - ch->move );
}

void gain_addiction( CHAR_DATA *ch )
{
	short drug;
	AFFECT_DATA af;

	for( drug = 0; drug <= 9; drug++ )
	{

		if( ch->pcdata->addiction[drug] < ch->pcdata->drug_level[drug] )
			ch->pcdata->addiction[drug]++;

		if( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug] + 150 )
		{
			switch( ch->pcdata->addiction[drug] )
			{
			default:
			case SPICE_GLITTERSTIM:
				if( !IS_AFFECTED( ch, AFF_BLIND ) )
				{
					af.type = gsn_blindness;
					af.location = APPLY_AC;
					af.modifier = 10;
					af.duration = ch->pcdata->addiction[drug];
					af.bitvector = AFF_BLIND;
					affect_to_char( ch, &af );
				}
			case SPICE_CARSANUM:
				if( !IS_AFFECTED( ch, AFF_WEAKEN ) )
				{
					af.type = -1;
					af.location = APPLY_DAMROLL;
					af.modifier = -10;
					af.duration = ch->pcdata->addiction[drug];
					af.bitvector = AFF_WEAKEN;
					affect_to_char( ch, &af );
				}
			case SPICE_RYLL:
				if( !IS_AFFECTED( ch, AFF_WEAKEN ) )
				{
					af.type = -1;
					af.location = APPLY_DEX;
					af.modifier = -5;
					af.duration = ch->pcdata->addiction[drug];
					af.bitvector = AFF_WEAKEN;
					affect_to_char( ch, &af );
				}
			case SPICE_ANDRIS:
				if( !IS_AFFECTED( ch, AFF_WEAKEN ) )
				{
					af.type = -1;
					af.location = APPLY_CON;
					af.modifier = -5;
					af.duration = ch->pcdata->addiction[drug];
					af.bitvector = AFF_WEAKEN;
					affect_to_char( ch, &af );
				}
			}
		}

		if( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug] + 200 )
		{
			ch_printf( ch, "You feel like you are going to die. You NEED %s\r\n.", spice_table[drug] );
			worsen_mental_state( ch, 2 );
			damage( ch, ch, 5, TYPE_UNDEFINED );
		}
		else if( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug] + 100 )
		{
			ch_printf( ch, "You need some %s.\r\n", spice_table[drug] );
			worsen_mental_state( ch, 2 );
		}
		else if( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug] + 50 )
		{
			ch_printf( ch, "You really crave some %s.\r\n", spice_table[drug] );
			worsen_mental_state( ch, 1 );
		}
		else if( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug] + 25 )
		{
			ch_printf( ch, "Some more %s would feel quite nice.\r\n", spice_table[drug] );
		}
		else if( ch->pcdata->addiction[drug] < ch->pcdata->drug_level[drug] - 50 )
		{
			act( AT_POISON, "$n bends over and vomits.\r\n", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "You vomit.\r\n", ch, NULL, NULL, TO_CHAR );
			ch->pcdata->drug_level[drug] -= 10;
		}

		if( ch->pcdata->drug_level[drug] > 1 )
			ch->pcdata->drug_level[drug] -= 2;
		else if( ch->pcdata->drug_level[drug] > 0 )
			ch->pcdata->drug_level[drug] -= 1;
		else if( ch->pcdata->addiction[drug] > 0 && ch->pcdata->drug_level[drug] <= 0 )
			ch->pcdata->addiction[drug]--;
	}

}

void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
	int condition;
	ch_ret retcode;

	if( value == 0 || IS_NPC( ch ) || get_trust( ch ) >= LEVEL_STAFF || NOT_AUTHED( ch ) )
		return;

	condition = ch->pcdata->condition[iCond];
	ch->pcdata->condition[iCond] = URANGE( 0, condition + value, 48 );

	if( ch->pcdata->condition[iCond] == 0 )
	{
		switch( iCond )
		{
		case COND_FULL:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_HUNGRY, ch );
				send_to_char( "You are REALLY hungry!\r\n", ch );
				act( AT_HUNGRY, "$n's stomach is growling quite loudly!", ch, NULL, NULL, TO_ROOM );
				worsen_mental_state( ch, 1 );
				retcode = damage( ch, ch, 5, TYPE_UNDEFINED );
			}
			break;

		case COND_BLOODTHIRST:
			if( ch->top_level < LEVEL_AVATAR )
			{
				set_char_color( AT_BLOOD, ch );
				send_to_char( "You are absolutely lusting for blood!\r\n", ch );
				act( AT_BLOOD, "$n looks around, lusting for blood!", ch, NULL, NULL, TO_ROOM );
				worsen_mental_state( ch, 2 );
				retcode = damage( ch, ch, ch->max_hit / 20, TYPE_UNDEFINED );
			}
			break;

		case COND_THIRST:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_THIRSTY, ch );
				send_to_char( "You REALLY need a drink!\r\n", ch );
				act( AT_THIRSTY, "$n is in utter need of a drink.", ch, NULL, NULL, TO_ROOM );
				worsen_mental_state( ch, 2 );
				retcode = damage( ch, ch, 5, TYPE_UNDEFINED );
			}
			break;

		case COND_DRUNK:
			if( condition != 0 )
			{
				set_char_color( AT_SOBER, ch );
				send_to_char( "You are sober.\r\n", ch );
			}
			retcode = rNONE;
			break;
		default:
			bug( "Gain_condition: invalid condition type %d", iCond );
			retcode = rNONE;
			break;
		}
	}

	if( retcode != rNONE )
		return;

	if( ch->pcdata->condition[iCond] == 1 )
	{
		switch( iCond )
		{
		case COND_FULL:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_HUNGRY, ch );
				send_to_char( "You should really consider eating.\r\n", ch );
				act( AT_HUNGRY, "A soft rumble can be heard from $n's stomach.", ch, NULL, NULL, TO_ROOM );
				if( number_bits( 1 ) == 0 )
					worsen_mental_state( ch, 1 );
			}
			break;

		case COND_BLOODTHIRST:
			if( ch->top_level < LEVEL_AVATAR )
			{
				set_char_color( AT_BLOOD, ch );
				send_to_char( "Your mouth is watering for the taste of blood!\r\n", ch );
				act( AT_BLOOD, "$n's mouth begins to water...", ch, NULL, NULL, TO_ROOM );
				worsen_mental_state( ch, 1 );
			}
			break;

		case COND_THIRST:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_THIRSTY, ch );
				send_to_char( "You could really use a drink.\r\n", ch );
				worsen_mental_state( ch, 1 );
				act( AT_THIRSTY, "$n could probably use a drink.", ch, NULL, NULL, TO_ROOM );
			}
			break;

		case COND_DRUNK:
			if( condition != 0 )
			{
				set_char_color( AT_SOBER, ch );
				send_to_char( "You are feeling a little less light headed.\r\n", ch );
			}
			break;
		}
	}


	if( ch->pcdata->condition[iCond] == 2 )
	{
		switch( iCond )
		{
		case COND_FULL:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_HUNGRY, ch );
				send_to_char( "You feel quite hungry.\r\n", ch );
			}
			break;

		case COND_THIRST:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_THIRSTY, ch );
				send_to_char( "You could really use a drink.\r\n", ch );
			}
			break;

		case COND_BLOODTHIRST:
			if( ch->top_level < LEVEL_AVATAR )
			{
				set_char_color( AT_BLOOD, ch );
				send_to_char( "You have a small need for blood.\r\n", ch );
			}
			break;

		}
	}

	if( ch->pcdata->condition[iCond] == 3 )
	{
		switch( iCond )
		{
		case COND_FULL:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_HUNGRY, ch );
				send_to_char( "You could use a snack.\r\n", ch );
			}
			break;

		case COND_THIRST:
			if( ch->top_level <= LEVEL_AVATAR )
			{
				set_char_color( AT_THIRSTY, ch );
				send_to_char( "You sure could use a nice cool drink.\r\n", ch );
			}
			break;

		case COND_BLOODTHIRST:
			if( ch->top_level < LEVEL_AVATAR )
			{
				set_char_color( AT_BLOOD, ch );
				send_to_char( "Your infection is slowly turning into a lust for blood...\r\n", ch );
			}
			break;

		}
	}
	return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update( void )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	EXIT_DATA *pexit;
	int door;
	ch_ret retcode;

	retcode = rNONE;

	/*
	 * Examine all mobs.
	 */
	for( ch = last_char; ch; ch = gch_prev )
	{
		set_cur_char( ch );
		if( ch == first_char && ch->prev )
		{
			bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;

		if( gch_prev && gch_prev->next != ch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name );
			bug( buf, 0 );
			bug( "Short-cutting here", 0 );
			gch_prev = NULL;
			ch->prev = NULL;
			echo_to_all( AT_RED, "**WARNING** Autocode Message: Crash likely, pleases save.", ECHOTAR_ALL );
		}

		if( !IS_NPC( ch ) )
		{
			drunk_randoms( ch );
			zeromares( ch );
			halucinations( ch );
			continue;
		}

		if( !ch->in_room || IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_PARALYSIS ) )
			continue;

		/* Clean up 'animated corpses' that are not charmed' - Scryn */

		if( ch->pIndexData->vnum == 5 && !IS_AFFECTED( ch, AFF_CHARM ) )
		{
			if( ch->in_room->first_person )
				act( AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM );

			if( IS_NPC( ch ) ) /* Guard against purging switched? */
				extract_char( ch, true );
			continue;
		}

		if( !xIS_SET( ch->act, ACT_RUNNING ) && !xIS_SET( ch->act, ACT_SENTINEL ) && !ch->fighting && ch->hunting )
		{
			if( ch->top_level < 20 )
				WAIT_STATE( ch, 6 * PULSE_PER_SECOND );
			else if( ch->top_level < 40 )
				WAIT_STATE( ch, 5 * PULSE_PER_SECOND );
			else if( ch->top_level < 60 )
				WAIT_STATE( ch, 4 * PULSE_PER_SECOND );
			else if( ch->top_level < 80 )
				WAIT_STATE( ch, 3 * PULSE_PER_SECOND );
			else if( ch->top_level < 100 )
				WAIT_STATE( ch, 2 * PULSE_PER_SECOND );
			else
				WAIT_STATE( ch, 1 * PULSE_PER_SECOND );
			hunt_victim( ch );
			continue;
		}
		else if( !ch->fighting && !ch->hunting
			&& !xIS_SET( ch->act, ACT_RUNNING ) && ch->was_sentinel && ch->position >= POS_STANDING )
		{
			act( AT_ACTION, "$n leaves.", ch, NULL, NULL, TO_ROOM );
			char_from_room( ch );
			char_to_room( ch, ch->was_sentinel );
			act( AT_ACTION, "$n arrives.", ch, NULL, NULL, TO_ROOM );
			xSET_BIT( ch->act, ACT_SENTINEL );
			ch->was_sentinel = NULL;
		}

		/*
		 * Examine call for special procedure
		 */
		if( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_fun )
		{
			if( ( *ch->spec_fun ) ( ch ) )
				continue;
			if( char_died( ch ) )
				continue;
		}

		if( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_2 )
		{
			if( ( *ch->spec_2 ) ( ch ) )
				continue;
			if( char_died( ch ) )
				continue;
		}

		/*
		 * Check for mudprogram script on mob
		 */
		if( HAS_PROG( ch->pIndexData, SCRIPT_PROG ) )
		{
			mprog_script_trigger( ch );
			continue;
		}

		if( ch != cur_char )
		{
			bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
			continue;
		}

		/*
		 * That's all for sleeping / busy monster
		 */
		if( ch->position != POS_STANDING )
			continue;


		if( xIS_SET( ch->act, ACT_MOUNTED ) )
		{
			if( xIS_SET( ch->act, ACT_AGGRESSIVE ) )
				do_emote( ch, "snarls and growls." );
			continue;
		}

		if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && xIS_SET( ch->act, ACT_AGGRESSIVE ) )
			do_emote( ch, "glares around and snarls." );


		/*
		 * MOBprogram random trigger
		 */
		if( ch->in_room->area->nplayer > 0 )
		{
			mprog_random_trigger( ch );
			if( char_died( ch ) )
				continue;
			if( ch->position < POS_STANDING )
				continue;
		}

		/*
		 * MOBprogram hour trigger: do something for an hour
		 */
		mprog_hour_trigger( ch );

		if( char_died( ch ) )
			continue;

		rprog_hour_trigger( ch );
		if( char_died( ch ) )
			continue;

		if( ch->position < POS_STANDING )
			continue;

		/*
		 * Scavenge
		 */
		if( xIS_SET( ch->act, ACT_SCAVENGER ) && ch->in_room->first_content && number_bits( 2 ) == 0 )
		{
			OBJ_DATA *obj;
			OBJ_DATA *obj_best;
			int max;

			max = 1;
			obj_best = NULL;
			for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
			{
				if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !xIS_SET( ch->act, ACT_PROTOTYPE ) )
					continue;
				if( CAN_WEAR( obj, ITEM_TAKE ) && obj->cost > max && !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
				{
					obj_best = obj;
					max = obj->cost;
				}
			}

			if( obj_best )
			{
				obj_from_room( obj_best );
				obj_to_char( obj_best, ch );
				act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
			}
		}

		/*
		 * Wander
		 */
		if( !xIS_SET( ch->act, ACT_RUNNING )
			&& !xIS_SET( ch->act, ACT_SENTINEL )
			&& !xIS_SET( ch->act, ACT_PROTOTYPE )
			&& ( door = number_bits( 5 ) ) <= 9
			&& ( pexit = get_exit( ch->in_room, door ) ) != NULL
			&& pexit->to_room
			&& !IS_SET( pexit->exit_info, EX_CLOSED )
			&& !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
			&& ( !xIS_SET( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
		{
			retcode = move_char( ch, pexit, 0 );
			/*
			 * If ch changes position due
			 * to it's or someother mob's
			 * movement via MOBProgs,
			 * continue - Kahn
			 */
			if( char_died( ch ) )
				continue;
			if( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
				continue;
		}

		/*
		 * Flee
		 */
		if( ch->hit < ch->max_hit / 2
			&& ( door = number_bits( 4 ) ) <= 9
			&& ( pexit = get_exit( ch->in_room, door ) ) != NULL
			&& pexit->to_room && !IS_SET( pexit->exit_info, EX_CLOSED ) && !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
		{
			CHAR_DATA *rch;
			bool found;

			found = false;
			for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
			{
				if( is_fearing( ch, rch ) )
				{
					switch( number_bits( 2 ) )
					{
					case 0:
						snprintf( buf, MAX_STRING_LENGTH, "Get away from me, %s!", rch->name );
						break;
					case 1:
						snprintf( buf, MAX_STRING_LENGTH, "Leave me be, %s!", rch->name );
						break;
					case 2:
						snprintf( buf, MAX_STRING_LENGTH, "%s is trying to kill me!  Help!", rch->name );
						break;
					case 3:
						snprintf( buf, MAX_STRING_LENGTH, "Someone save me from %s!", rch->name );
						break;
					}
					do_yell( ch, buf );
					found = true;
					break;
				}
			}
			if( found )
				retcode = move_char( ch, pexit, 0 );
		}
	}

	return;
}

void mail_check( void )
{
	CHAR_DATA *ch;

	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}
		mail_count( ch );
	}
}

void update_taxes( void )
{
	DESCRIPTOR_DATA *d;
	CLAN_DATA *clan;
	PLANET_DATA *cPlanet;
	int gain1, gain2, fgain;
	int pCount, average, tnum, pnum;
	int MAX_THUG, MAX_HITMAN, MAX_DRUGADDICT, MAX_PIMP, MAX_SCALPER;

	gain1 = 0;
	gain2 = 0;
	fgain = 0;
	tnum = 0;
	average = 0;
	MAX_THUG = 0;
	MAX_HITMAN = 0;
	MAX_DRUGADDICT = 0;
	MAX_PIMP = 0;
	MAX_SCALPER = 0;
	pnum = 0;

	for( clan = first_clan; clan; clan = clan->next )
	{
		gain1 += ( clan->pimp * 800 );
		gain2 += ( clan->scalper * 500 );
		fgain = ( gain1 + gain2 );
		clan->funds += fgain;
		clan->income = fgain;
		for( cPlanet = first_planet; cPlanet; cPlanet = cPlanet->next )
			if( clan == cPlanet->governed_by )
			{
				tnum += cPlanet->pop_support;
				pnum++;
				pCount++;
			}

		MAX_THUG = pnum * 4;
		MAX_HITMAN = pnum * 3;
		MAX_DRUGADDICT = pnum * 3;
		MAX_PIMP = pnum * 2;
		MAX_SCALPER = pnum * 2;

		if( pCount != 0 )  	// Added check to  make sure pCount isn't 0 to prevent crash. ~Arcturis
			average = ( tnum / pCount );

		if( average == 200 )
		{
			clan->thug += 3;
			clan->drugaddict += 3;
		}
		else if( average >= 100 && average <= 199 )
		{
			clan->thug += 2;
			clan->drugaddict += 2;
		}
		else if( average >= 1 && average <= 99 )
		{
			clan->thug += 1;
			clan->drugaddict += 1;
		}
		else if( average <= -1 && average >= -49 )
		{
			clan->thug -= 1;
			clan->drugaddict -= 1;
		}
		else if( average <= -50 && average >= -99 )
		{
			clan->thug -= 2;
			clan->drugaddict -= 2;
		}
		else if( average == -100 )
		{
			clan->thug -= 3;
			clan->drugaddict -= 3;
		}

		if( clan->thug > MAX_THUG )
		{
			clan->thug = MAX_THUG;
		}

		if( clan->hitmen > MAX_HITMAN )
		{
			clan->hitmen = MAX_HITMAN;
		}

		if( clan->drugaddict > MAX_DRUGADDICT )
		{
			clan->drugaddict = MAX_DRUGADDICT;
		}

		if( clan->pimp > MAX_PIMP )
		{
			clan->pimp = MAX_PIMP;
		}

		if( clan->scalper > MAX_SCALPER )
		{
			clan->scalper = MAX_SCALPER;
		}

		for( d = first_descriptor; d; d = d->next )
		{
			if( d->connected == CON_PLAYING )
			{
				if( IS_NPC( d->character ) )
					continue;
				if( !d->character->pcdata->clan )
					continue;

				if( !str_cmp( clan->name, d->character->pcdata->clan->name ) )
					ch_printf( d->character, "&w[GANG INFO] Your hustlers have brought in $%s!\r\n", num_punct( fgain ) );
			}
		}

	}


}


/*
 * Update the weather.
 */
void weather_update( void )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int diff;
	short AT_TEMP = AT_PLAIN;

	buf[0] = '\0';

	switch( ++time_info.hour )
	{
	case 5:
		weather_info.sunlight = SUN_LIGHT;
		strcat( buf, "The day has begun." );
		AT_TEMP = AT_YELLOW;
		break;

	case 6:
		weather_info.sunlight = SUN_RISE;
		strcat( buf, "The sun rises in the east." );
		AT_TEMP = AT_ORANGE;
		break;

	case 12:
		weather_info.sunlight = SUN_LIGHT;
		strcat( buf, "It's noon." );
		AT_TEMP = AT_YELLOW;
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		strcat( buf, "The sun slowly disappears in the west." );
		AT_TEMP = AT_BLOOD;
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		strcat( buf, "The night has begun." );
		AT_TEMP = AT_DGREY;
		break;

	case 24:
		time_info.hour = 0;
		time_info.day++;
		break;
	}

	if( time_info.day >= 30 )
	{
		time_info.day = 0;
		time_info.month++;
	}

	if( time_info.month >= 12 )
	{
		time_info.month = 0;
		time_info.year++;
	}

	if( buf[0] != '\0' )
	{
		for( d = first_descriptor; d; d = d->next )
		{
			if( d->connected == CON_PLAYING
				&& IS_OUTSIDE( d->character )
				&& IS_AWAKE( d->character )
				&& d->character->in_room
				&& d->character->in_room->sector_type != SECT_UNDERWATER
				&& d->character->in_room->sector_type != SECT_OCEANFLOOR
				&& d->character->in_room->sector_type != SECT_UNDERGROUND )
				act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
		}
		buf[0] = '\0';
	}
	if( time_info.month >= 9 && time_info.month <= 16 )
		diff = weather_info.mmhg > 985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change += diff * dice( 1, 4 ) + dice( 2, 6 ) - dice( 2, 6 );
	weather_info.change = UMAX( weather_info.change, -12 );
	weather_info.change = UMIN( weather_info.change, 12 );

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg = UMAX( weather_info.mmhg, 960 );
	weather_info.mmhg = UMIN( weather_info.mmhg, 1040 );

	AT_TEMP = AT_GREY;
	switch( weather_info.sky )
	{
	default:
		bug( "Weather_update: bad sky %d.", weather_info.sky );
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if( weather_info.mmhg < 990 || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
		{
			strcat( buf, "The sky is getting cloudy." );
			weather_info.sky = SKY_CLOUDY;
			AT_TEMP = AT_GREY;
		}
		break;

	case SKY_CLOUDY:
		if( weather_info.mmhg < 970 || ( weather_info.mmhg < 990 && number_bits( 2 ) == 0 ) )
		{
			strcat( buf, "It starts to rain." );
			weather_info.sky = SKY_RAINING;
			AT_TEMP = AT_BLUE;
		}

		if( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
		{
			strcat( buf, "The clouds disappear." );
			weather_info.sky = SKY_CLOUDLESS;
			AT_TEMP = AT_WHITE;
		}
		break;

	case SKY_RAINING:
		if( weather_info.mmhg < 970 && number_bits( 2 ) == 0 )
		{
			strcat( buf, "Lightning flashes in the sky." );
			weather_info.sky = SKY_LIGHTNING;
			AT_TEMP = AT_YELLOW;
		}

		if( weather_info.mmhg > 1030 || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
		{
			strcat( buf, "The rain stopped." );
			weather_info.sky = SKY_CLOUDY;
			AT_TEMP = AT_WHITE;
		}
		break;

	case SKY_LIGHTNING:
		if( weather_info.mmhg > 1010 || ( weather_info.mmhg > 990 && number_bits( 2 ) == 0 ) )
		{
			strcat( buf, "The lightning has stopped." );
			weather_info.sky = SKY_RAINING;
			AT_TEMP = AT_GREY;
			break;
		}
		break;
	}

	if( buf[0] != '\0' )
	{
		for( d = first_descriptor; d; d = d->next )
		{
			if( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
				act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
		}

	}

	calc_season( );
	return;
}



/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_save;
	short save_count = 0;

	ch_save = NULL;
	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}

		/*
		 *  Do a room_prog rand check right off the bat
		 *   if ch disappears (rprog might wax npc's), continue
		 */
		if( !IS_NPC( ch ) )
			rprog_random_trigger( ch );

		if( char_died( ch ) )
			continue;

		if( IS_NPC( ch ) )
			mprog_time_trigger( ch );

		if( char_died( ch ) )
			continue;

		rprog_time_trigger( ch );

		if( char_died( ch ) )
			continue;

		/*
		 * See if player should be auto-saved.
		 */
		if( !IS_NPC( ch ) && !NOT_AUTHED( ch ) && current_time - ch->save_time > ( sysdata.save_frequency * 60 ) )
			ch_save = ch;
		else
			ch_save = NULL;

		if( ch->position >= POS_STUNNED )
		{
			if( ch->hit < ch->max_hit )
				ch->hit += hit_gain( ch );

			if( ch->move < ch->max_move )
				ch->move += move_gain( ch );
		}

		if( ch->position == POS_STUNNED )
			update_pos( ch );

		if( ch->pcdata )
			gain_addiction( ch );

		if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_steal] > 50 )
		{
			ch->pcdata->learned[gsn_steal] = 50;
		}


		if( !IS_NPC( ch ) && ch->top_level < LEVEL_STAFF )
		{
			OBJ_DATA *obj;

			if( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0 )
			{
				if( --obj->value[2] == 0 && ch->in_room )
				{
					ch->in_room->light -= obj->count;
					act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
					if( obj->serial == cur_obj )
						global_objcode = rOBJ_EXPIRED;
					extract_obj( obj );
				}
			}

			if( ch->pcdata->condition[COND_DRUNK] > 8 )
				worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK] / 8 );
			if( ch->pcdata->condition[COND_FULL] > 1 )
			{
				switch( ch->position )
				{
				case POS_SLEEPING:
					better_mental_state( ch, 4 );
					break;
				case POS_RESTING:
					better_mental_state( ch, 3 );
					break;
				case POS_SITTING:
				case POS_MOUNTED:
					better_mental_state( ch, 2 );
					break;
				case POS_STANDING:
					better_mental_state( ch, 1 );
					break;
				case POS_FIGHTING:
					if( number_bits( 2 ) == 0 )
						better_mental_state( ch, 1 );
					break;
				}
			}
			if( ch->pcdata->condition[COND_THIRST] > 1 )
			{
				switch( ch->position )
				{
				case POS_SLEEPING:
					better_mental_state( ch, 5 );
					break;
				case POS_RESTING:
					better_mental_state( ch, 3 );
					break;
				case POS_SITTING:
				case POS_MOUNTED:
					better_mental_state( ch, 2 );
					break;
				case POS_STANDING:
					better_mental_state( ch, 1 );
					break;
				case POS_FIGHTING:
					if( number_bits( 2 ) == 0 )
						better_mental_state( ch, 1 );
					break;
				}
			}

			gain_condition( ch, COND_DRUNK, -1 );
			gain_condition( ch, COND_FULL, -1 );
			if( ch->in_room )
				switch( ch->in_room->sector_type )
				{
				default:
					gain_condition( ch, COND_THIRST, -1 );
					break;
				case SECT_DESERT:
					gain_condition( ch, COND_THIRST, -2 );
					break;
				case SECT_UNDERWATER:
				case SECT_OCEANFLOOR:
					if( number_bits( 1 ) == 0 )
						gain_condition( ch, COND_THIRST, -1 );
					break;
				}

		}

		if( !char_died( ch ) )
		{
			/*
			 * Careful with the damages here,
			 *   MUST NOT refer to ch after damage taken,
			 *   as it may be lethal damage (on NPC).
			 */
			if( IS_AFFECTED( ch, AFF_POISON ) )
			{
				act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
				act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
				ch->mental_state = URANGE( 20, ch->mental_state + 4, 1000 );
				damage( ch, ch, 6, gsn_poison );
			}
			else if( ch->position == POS_INCAP )
				damage( ch, ch, 1, TYPE_UNDEFINED );
			else if( ch->position == POS_MORTAL )
				damage( ch, ch, 4, TYPE_UNDEFINED );
			if( char_died( ch ) )
				continue;

			if( ch->mental_state >= 30 )
				switch( ( ch->mental_state + 5 ) / 10 )
				{
				case 3:
					send_to_char( "You feel feverish.\r\n", ch );
					act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
					break;
				case 4:
					send_to_char( "You do not feel well at all.\r\n", ch );
					act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
					break;
				case 5:
					send_to_char( "You need help!\r\n", ch );
					act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
					break;
				case 6:
					send_to_char( "Seekest thou a cleric.\r\n", ch );
					act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
					break;
				case 7:
					send_to_char( "You feel reality slipping away...\r\n", ch );
					act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
					break;
				case 8:
					send_to_char( "You begin to understand... everything.\r\n", ch );
					act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
					break;
				case 9:
					send_to_char( "You are ONE with the universe.\r\n", ch );
					act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL,
						TO_ROOM );
					break;
				case 10:
					send_to_char( "You feel the end is near.\r\n", ch );
					act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
					break;
				}
			if( ch->mental_state <= -30 )
				switch( ( abs( ch->mental_state ) + 5 ) / 10 )
				{
				case 10:
					if( ch->position > POS_SLEEPING )
					{
						if( ( ch->position == POS_STANDING
							|| ch->position < POS_FIGHTING ) && number_percent( ) + 10 < abs( ch->mental_state ) )
							do_sleep( ch, "" );
						else
							send_to_char( "You're barely conscious.\r\n", ch );
					}
					break;
				case 9:
					if( ch->position > POS_SLEEPING )
					{
						if( ( ch->position == POS_STANDING
							|| ch->position < POS_FIGHTING ) && ( number_percent( ) + 20 ) < abs( ch->mental_state ) )
							do_sleep( ch, "" );
						else
							send_to_char( "You can barely keep your eyes open.\r\n", ch );
					}
					break;
				case 8:
					if( ch->position > POS_SLEEPING )
					{
						if( ch->position < POS_SITTING && ( number_percent( ) + 30 ) < abs( ch->mental_state ) )
							do_sleep( ch, "" );
						else
							send_to_char( "You're extremely drowsy.\r\n", ch );
					}
					break;
				case 7:
					if( ch->position > POS_RESTING )
						send_to_char( "You feel very unmotivated.\r\n", ch );
					break;
				case 6:
					if( ch->position > POS_RESTING )
						send_to_char( "You feel sedated.\r\n", ch );
					break;
				case 5:
					if( ch->position > POS_RESTING )
						send_to_char( "You feel sleepy.\r\n", ch );
					break;
				case 4:
					if( ch->position > POS_RESTING )
						send_to_char( "You feel tired.\r\n", ch );
					break;
				case 3:
					if( ch->position > POS_RESTING )
						send_to_char( "You could use a rest.\r\n", ch );
					break;
				}

			if( ch->backup_wait > 0 )
			{
				--ch->backup_wait;
				if( ch->backup_wait == 0 )
					add_reinforcements( ch );
			}

			if( !IS_NPC( ch ) )
			{
				if( ++ch->timer > 15 && !ch->desc )
				{
					if( xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
					{
						ch->position = POS_RESTING;
						save_char_obj( ch );
						do_quit( ch, "" );
					}
					else if( !xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
					{
						if( ch->in_room )
							char_from_room( ch );
						char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
						ch->position = POS_RESTING;
						ch->hit = UMAX( 1, ch->hit );
						save_char_obj( ch );
						do_quit( ch, "" );
					}
				}
				else if( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO ) && ++save_count < 10 )  /* save max of 10 per tick */
					save_char_obj( ch );
			}
		}

	}

	return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
	OBJ_DATA *obj;
	short AT_TEMP;

	for( obj = last_object; obj; obj = gobj_prev )
	{
		CHAR_DATA *rch;
		const char *message;

		if( obj == first_object && obj->prev )
		{
			bug( "obj_update: first_object->prev != NULL... fixed", 0 );
			obj->prev = NULL;
		}
		gobj_prev = obj->prev;
		if( gobj_prev && gobj_prev->next != obj )
		{
			bug( "obj_update: obj->prev->next != obj", 0 );
			return;
		}
		set_cur_obj( obj );
		if( obj->carried_by )
			oprog_random_trigger( obj );
		else if( obj->in_room && obj->in_room->area->nplayer > 0 )
			oprog_random_trigger( obj );

		if( obj_extracted( obj ) )
			continue;

		if( obj->item_type == ITEM_PIPE )
		{
			if( IS_SET( obj->value[3], PIPE_LIT ) )
			{
				if( --obj->value[1] <= 0 )
				{
					obj->value[1] = 0;
					REMOVE_BIT( obj->value[3], PIPE_LIT );
				}
				else if( IS_SET( obj->value[3], PIPE_HOT ) )
					REMOVE_BIT( obj->value[3], PIPE_HOT );
				else
				{
					if( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
					{
						REMOVE_BIT( obj->value[3], PIPE_LIT );
						REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
					}
					else
						SET_BIT( obj->value[3], PIPE_GOINGOUT );
				}
				if( !IS_SET( obj->value[3], PIPE_LIT ) )
					SET_BIT( obj->value[3], PIPE_FULLOFASH );
			}
			else
				REMOVE_BIT( obj->value[3], PIPE_HOT );
		}


		/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

		if( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_DROID_CORPSE )
		{
			short timerfrac = UMAX( 1, obj->timer - 1 );
			if( obj->item_type == ITEM_CORPSE_PC )
				timerfrac = ( int ) ( obj->timer / 8 + 1 );

			if( obj->timer > 0 && obj->value[2] > timerfrac )
			{
				char buf[MAX_STRING_LENGTH];
				char name[MAX_STRING_LENGTH];
				const char *bufptr;
				bufptr = one_argument( obj->short_descr, name );
				bufptr = one_argument( bufptr, name );
				bufptr = one_argument( bufptr, name );

				separate_obj( obj );
				obj->value[2] = timerfrac;
				if( obj->item_type == ITEM_DROID_CORPSE )
					snprintf( buf, MAX_STRING_LENGTH, d_corpse_descs[UMIN( timerfrac - 1, 4 )], bufptr );
				else
					snprintf( buf, MAX_STRING_LENGTH, corpse_descs[UMIN( timerfrac - 1, 4 )], capitalize( bufptr ) );

				STRFREE( obj->description );
				obj->description = STRALLOC( buf );
			}
		}

		/*
		 * don't let inventory decay
		 */
		if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
			continue;

		if( obj->timer > 0 && obj->timer < 5 && obj->item_type == ITEM_ARMOR )
		{
			if( obj->carried_by )
			{
				act( AT_TEMP, "$p is almost dead.", obj->carried_by, obj, NULL, TO_CHAR );
			}
		}

		if( ( obj->timer <= 0 || --obj->timer > 0 ) )
			continue;


		/*
		 * if we get this far, object's timer has expired.
		 */

		AT_TEMP = AT_PLAIN;
		switch( obj->item_type )
		{
		default:
			message = "$p has depleted itself.";
			AT_TEMP = AT_PLAIN;
			break;

		case ITEM_GRENADE:
			explode( obj );
			return;
			break;

		case ITEM_PORTAL:
			message = "$p winks out of existence.";
			remove_portal( obj );
			obj->item_type = ITEM_TRASH;    /* so extract_obj    */
			AT_TEMP = AT_MAGIC; /* doesn't remove_portal */
			break;
		case ITEM_FOUNTAIN:
			message = "$p dries up.";
			AT_TEMP = AT_BLUE;
			break;
		case ITEM_CORPSE_NPC:
			message = "$p decays into dust and blows away.";
			AT_TEMP = AT_OBJECT;
			break;
		case ITEM_DROID_CORPSE:
			message = "$p rusts away into oblivion.";
			AT_TEMP = AT_OBJECT;
			break;
		case ITEM_CORPSE_PC:
			message = "$p decays into dust and is blown away...";
			AT_TEMP = AT_MAGIC;
			break;
		case ITEM_FOOD:
			message = "$p is devoured by a swarm of maggots.";
			AT_TEMP = AT_HUNGRY;
			break;
		case ITEM_BLOOD:
			message = "$p slowly seeps into the ground.";
			AT_TEMP = AT_BLOOD;
			break;
		case ITEM_BLOODSTAIN:
			message = "$p dries up into flakes and blows away.";
			AT_TEMP = AT_BLOOD;
			break;
		case ITEM_SCRAPS:
			message = "$p crumbles and decays into nothing.";
			AT_TEMP = AT_OBJECT;
			break;
		case ITEM_FIRE:
			if( obj->in_room )
				--obj->in_room->light;
			message = "$p burns out.";
			AT_TEMP = AT_FIRE;
		}

		if( obj->carried_by )
		{
			act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
		}
		else if( obj->in_room && ( rch = obj->in_room->first_person ) != NULL && !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
		{
			act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
			act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
		}

		if( obj->serial == cur_obj )
			global_objcode = rOBJ_EXPIRED;
		extract_obj( obj );
	}
	return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
	CHAR_DATA *ch, *ch_next;
	EXIT_DATA *pexit;
	static int cnt = 0;
	int door, retcode;

	cnt = ( cnt + 1 ) % 2;

	for( ch = first_char; ch; ch = ch_next )
	{
		set_cur_char( ch );
		ch_next = ch->next;
		will_fall( ch, 0 );

		if( char_died( ch ) )
			continue;

		if( IS_NPC( ch ) )
		{
			if( cnt != 0 )
				continue;

			/*
			 * running mobs -Thoric
			 */
			if( xIS_SET( ch->act, ACT_RUNNING ) )
			{
				if( !xIS_SET( ch->act, ACT_SENTINEL ) && !ch->fighting && ch->hunting )
				{
					WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
					hunt_victim( ch );
					continue;
				}

				if( ch->spec_fun )
				{
					if( ( *ch->spec_fun ) ( ch ) )
						continue;
					if( char_died( ch ) )
						continue;
				}
				if( ch->spec_2 )
				{
					if( ( *ch->spec_2 ) ( ch ) )
						continue;
					if( char_died( ch ) )
						continue;
				}

				if( !xIS_SET( ch->act, ACT_SENTINEL )
					&& !xIS_SET( ch->act, ACT_PROTOTYPE )
					&& ( door = number_bits( 4 ) ) <= 9
					&& ( pexit = get_exit( ch->in_room, door ) ) != NULL
					&& pexit->to_room
					&& !IS_SET( pexit->exit_info, EX_CLOSED )
					&& !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
					&& ( !xIS_SET( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
				{
					retcode = move_char( ch, pexit, 0 );
					if( char_died( ch ) )
						continue;
					if( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
						continue;
				}
			}
			continue;
		}
		else
		{
			if( ch->mount && ch->in_room != ch->mount->in_room )
			{
				xREMOVE_BIT( ch->mount->act, ACT_MOUNTED );
				ch->mount = NULL;
				ch->position = POS_STANDING;
				send_to_char( "No longer upon your mount, you fall to the ground...\r\nOUCH!\r\n", ch );
			}

			if( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
				|| ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
			{
				if( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
				{
					if( get_trust( ch ) < LEVEL_STAFF )
					{
						int dam;


						dam = number_range( ch->max_hit / 50, ch->max_hit / 30 );
						dam = UMAX( 1, dam );
						if( ch->hit <= 0 )
							dam = UMIN( 10, dam );
						if( number_bits( 3 ) == 0 )
							send_to_char( "You cough and choke as you try to breathe water!\r\n", ch );
						damage( ch, ch, dam, TYPE_UNDEFINED );
					}
				}
			}

			if( char_died( ch ) )
				continue;

			if( ch->in_room
				&& ( ( ch->in_room->sector_type == SECT_WATER_NOSWIM ) || ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
			{
				if( !IS_AFFECTED( ch, AFF_FLYING )
					&& !IS_AFFECTED( ch, AFF_FLOATING ) && !IS_AFFECTED( ch, AFF_AQUA_BREATH ) && !ch->mount )
				{
					if( get_trust( ch ) < LEVEL_STAFF )
					{
						int dam;

						if( ch->move > 0 )
							ch->move--;
						else
						{
							dam = number_range( ch->max_hit / 50, ch->max_hit / 30 );
							dam = UMAX( 1, dam );
							if( ch->hit <= 0 )
								dam = UMIN( 10, dam );
							if( number_bits( 3 ) == 0 )
								send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\r\n", ch );
							damage( ch, ch, dam, TYPE_UNDEFINED );
						}
					}
				}
			}

		}
	}
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update( void )
{
	DESCRIPTOR_DATA *d, *dnext;
	CHAR_DATA *wch, *ch, *ch_next, *victim;
	struct act_prog_data *apdtmp;

	/*
	 * check mobprog act queue
	 */
	while( ( apdtmp = mob_act_list ) != NULL )
	{
		wch = ( CHAR_DATA * ) mob_act_list->vo;
		if( !char_died( wch ) && wch->mpactnum > 0 )
		{
			MPROG_ACT_LIST *tmp_act;

			while( ( tmp_act = wch->mpact ) != NULL )
			{
				if( tmp_act->obj && obj_extracted( tmp_act->obj ) )
					tmp_act->obj = NULL;
				if( tmp_act->ch && !char_died( tmp_act->ch ) )
					mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
				wch->mpact = tmp_act->next;
				DISPOSE( tmp_act->buf );
				DISPOSE( tmp_act );
			}
			wch->mpactnum = 0;
			wch->mpact = NULL;
		}
		mob_act_list = apdtmp->next;
		DISPOSE( apdtmp );
	}


	/*
	 * Just check descriptors here for victims to aggressive mobs
	 * We can check for linkdead victims to mobile_update   -Thoric
	 */
	for( d = first_descriptor; d; d = dnext )
	{
		dnext = d->next;
		if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) || ( wch = d->character ) == NULL )
			continue;

		if( char_died( wch ) || IS_NPC( wch ) || wch->top_level >= LEVEL_STAFF || !wch->in_room )
			continue;

		for( ch = wch->in_room->first_person; ch; ch = ch_next )
		{
			int count = 0;

			ch_next = ch->next_in_room;

			if( !IS_NPC( ch ) || ch->fighting || IS_AFFECTED( ch, AFF_CHARM )
				|| !IS_AWAKE( ch ) || ( xIS_SET( ch->act, ACT_WIMPY ) ) || !can_see( ch, wch ) )
				continue;

			if( is_hating( ch, wch ) )
			{
				found_prey( ch, wch );
				continue;
			}

			if( !xIS_SET( ch->act, ACT_AGGRESSIVE )
				|| xIS_SET( ch->act, ACT_MOUNTED ) || xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
				continue;

			victim = wch;

			if( !victim )
			{
				bug( "Aggr_update: null victim.", count );
				continue;
			}

			if( get_timer( victim, TIMER_RECENTFIGHT ) > 0 )
				continue;

			if( IS_NPC( ch ) && IS_SET( ch->attacks, ATCK_BACKSTAB ) )
			{
				OBJ_DATA *obj;

				if( !ch->mount
					&& ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL
					&& obj->value[3] == 11 && !victim->fighting && victim->hit >= victim->max_hit )
				{
					WAIT_STATE( ch, skill_table[gsn_backstab]->beats );
					if( !IS_AWAKE( victim ) || number_percent( ) + 5 < ch->top_level )
					{
						global_retcode = multi_hit( ch, victim, gsn_backstab );
						continue;
					}
					else
					{
						global_retcode = damage( ch, victim, 0, gsn_backstab );
						continue;
					}
				}
			}
			global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		}
	}

	return;
}

/* From interp.c */
bool check_social( CHAR_DATA *ch, const char *command, const char *argument );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms( CHAR_DATA *ch )
{
	CHAR_DATA *rvch = NULL;
	CHAR_DATA *vch;
	short drunk;
	short position;

	if( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
		return;

	if( number_percent( ) < 30 )
		return;

	drunk = ch->pcdata->condition[COND_DRUNK];
	position = ch->position;
	ch->position = POS_STANDING;

	if( number_percent( ) < ( 2 * drunk / 20 ) )
		check_social( ch, "burp", "" );
	else if( number_percent( ) < ( 2 * drunk / 20 ) )
		check_social( ch, "hiccup", "" );
	else if( number_percent( ) < ( 2 * drunk / 20 ) )
		check_social( ch, "drool", "" );
	else if( number_percent( ) < ( 2 * drunk / 20 ) )
		check_social( ch, "fart", "" );
	else if( drunk > ( 10 + ( get_curr_con( ch ) / 5 ) ) && number_percent( ) < ( 2 * drunk / 18 ) )
	{
		for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
			if( number_percent( ) < 10 )
				rvch = vch;
		check_social( ch, "puke", ( rvch ? rvch->name : "" ) );
	}

	ch->position = position;
	return;
}

void zeromares( CHAR_DATA *ch )
{
	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ch->mental_state >= 30 && number_bits( 5 - ( ch->mental_state >= 50 ) - ( ch->mental_state >= 75 ) ) == 0 )
		{
			const char *t;

			switch( number_range( 1, UMIN( 10, ( ch->mental_state + 5 ) / 5 ) ) )
			{
			default:
			case 1:
				t = "&RYou feel your blood racing through your veins...\r\n";
				break;
			case 2:
				t = "&rYou are being targetted by Virgo V04.\r\n";
				break;
			case 3:
				t = "&rThe left arm of your suit has been blown off!\r\n";
				break;
			case 4:
				t = "&rThe right arm arm of your suit has been blown off!\r\n";
				break;
			case 5:
				t = "&rThe Head of your suit has been blown off!\r\n";
				break;
			case 6:
				t = "&rThe legs of your suit have been blown off!\r\n";
				break;
			case 7:
				t = "&BTo Self Destruct or to not Self Destruct...\r\n";
				break;
			case 8:
				t = "&RProximity alert: Wing Gundam\r\n";
				break;
			case 9:
				t = "&RA small explosion vibrates through the suit.\r\n";
				break;
			case 10:
				t = "Your heart is racing, you can't think straight.\r\n";
				break;
			}
			send_to_char( t, ch );
		}
		return;
	}
}


void halucinations( CHAR_DATA *ch )
{
	if( ch->mental_state >= 30 && number_bits( 5 - ( ch->mental_state >= 50 ) - ( ch->mental_state >= 75 ) ) == 0
		&& !xIS_SET( ch->act, PCFLAG_ZERO ) )
	{
		const char *t;

		switch( number_range( 1, UMIN( 20, ( ch->mental_state + 5 ) / 5 ) ) )
		{
		default:
		case 1:
			t = "You feel very restless... you can't sit still.\r\n";
			break;
		case 2:
			t = "You're tingling all over.\r\n";
			break;
		case 3:
			t = "Your skin is crawling.\r\n";
			break;
		case 4:
			t = "You suddenly feel that something is terribly wrong.\r\n";
			break;
		case 5:
			t = "Those damn little fairies keep laughing at you!\r\n";
			break;
		case 6:
			t = "You can hear your mother crying...\r\n";
			break;
		case 7:
			t = "Have you been here before, or not?  You're not sure...\r\n";
			break;
		case 8:
			t = "Painful childhood memories flash through your mind.\r\n";
			break;
		case 9:
			t = "You hear someone call your name in the distance...\r\n";
			break;
		case 10:
			t = "Your head is pulsating... you can't think straight.\r\n";
			break;
		case 11:
			t = "The ground... seems to be squirming...\r\n";
			break;
		case 12:
			t = "You're not quite sure what is real anymore.\r\n";
			break;
		case 13:
			t = "It's all a dream... or is it?\r\n";
			break;
		case 14:
			t = "They're coming to get you... coming to take you away...\r\n";
			break;
		case 15:
			t = "You begin to feel all powerful!\r\n";
			break;
		case 16:
			t = "You're light as air... the heavens are yours for the taking.\r\n";
			break;
		case 17:
			t = "Your whole life flashes by... and your future...\r\n";
			break;
		case 18:
			t = "You are everywhere and everything... you know all and are all!\r\n";
			break;
		case 19:
			t = "You feel immortal!\r\n";
			break;
		case 20:
			t = "Ahh... the power of a Supreme Entity... what to do...\r\n";
			break;
		}
		send_to_char( t, ch );
	}
	return;
}

void tele_update( void )
{
	TELEPORT_DATA *tele, *tele_next;

	if( !first_teleport )
		return;

	for( tele = first_teleport; tele; tele = tele_next )
	{
		tele_next = tele->next;
		if( --tele->timer <= 0 )
		{
			if( tele->room->first_person )
			{
				teleport( tele->room->first_person, tele->room->tele_vnum, TELE_TRANSALL );
			}
			UNLINK( tele, first_teleport, last_teleport, next, prev );
			DISPOSE( tele );
		}
	}
}

void auth_update( void )
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH], log[MAX_INPUT_LENGTH];
	bool found_hit = false;  /* was at least one found? */

	strcpy( log, "Pending authorizations:\r\n" );
	for( d = first_descriptor; d; d = d->next )
	{
		if( ( victim = d->character ) && IS_WAITING_FOR_AUTH( victim ) )
		{
			found_hit = true;
			snprintf( buf, MAX_STRING_LENGTH, " %s@%s new %s\r\n", victim->name, victim->desc->host, race_table[victim->race].race_name );
			strcat( log, buf );
		}
	}
	if( found_hit )
	{
		to_channel( log, CHANNEL_MONITOR, "Monitor", 1 );
	}
}

void interestgain( )
{
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	int interest;
	bool ch_comlink;
	ch_comlink = false;

	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}



		if( !IS_NPC( ch ) )
		{
			interest = ( ch->pcdata->bank * .0071428571428571 );
			if( !xIS_SET( ch->act, PLR_AFK ) )
			{
				ch->pcdata->bank *= 1.0071428571428571;
			}
			for( obj = ch->last_carrying; obj; obj = obj->prev_content )
			{
				if( obj->pIndexData->item_type == ITEM_COMLINK )
					ch_comlink = true;
			}

			if( ch_comlink )
			{
				if( !xIS_SET( ch->act, PLR_AFK ) )
				{
					ch_printf( ch, "&B&RCDI ALERT&r: &YYou made &O%s&Y dollars from interest.\r\n", num_punct( interest ) );
				}
			}
		}
	}
}

void healchar( )
{
	CHAR_DATA *ch;

	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}


		if( !IS_NPC( ch ) )
		{

			if( IS_SET( ch->affected_by, AFF_MONEY_MAKER ) )
			{
				ch->gold += 125;
			}

			if( xIS_SET( ch->in_room->room_flags, ROOM_HEAL ) )
			{
				if( IS_AFFECTED( ch, AFF_MASS_HEAL ) )
				{
					ch->hit += get_curr_con( ch ) * 1000;
				}
				else
				{
					ch->hit += get_curr_con( ch ) * 300;
				}
				if( ch->hit > ch->max_hit )
				{
					ch->hit = ch->max_hit;
				}
			}
		}
	}
}

void update_prevtimer( )
{
	SHIP_DATA *ship;
	CHAR_DATA *ch;

	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}

		if( !IS_NPC( ch ) )
		{
			if( xIS_SET( ch->act, PLR_SILENCE ) )
			{
				if( ch->pcdata->silencetime < 1000000 )
				{
					//ch->pcdata->silencetime -= 1;
					if( ch->pcdata->silencetime == 0 )
					{
						//xREMOVE_BIT(ch->act, PLR_SILENCE );
					}
				}
			}
		}

	}

	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->prevtimer >= 1 )
		{
			if( !str_cmp( ship->owner, "" ) )
				ship->prevtimer -= 1;
		}

		if( ship->prevtimer == 0 )
		{
			if( !ship->prevowner )
				ship->prevowner = STRALLOC( "" );

			ship->prevtimer = 0;
		}

	}
}

/* Handles hour countdowns for:

  Lottery
  Unused house checks
*/

void update_hourprog( )
{
	CHAR_DATA *ch;
	AREA_DATA *tarea;
	ROOM_INDEX_DATA *room;
	int count, shardnum;
	OBJ_DATA *shard;
	count = 0;

	sysdata.storagetimer += 1;

	if( sysdata.lotterytimer >= 1 )
	{
		sysdata.lotterytimer -= 1;
		save_sysdata( sysdata );
	}

	if( sysdata.lotterytimer == 0 )
	{
		for( ch = last_char; ch; ch = gch_prev )
		{

			sysdata.lotterytimer = 50;
			sysdata.lotterynum = number_range( 0, 9999 );
			sysdata.lotteryweek += 1;
			save_sysdata( sysdata );

			if( ch == first_char && ch->prev )
			{
				bug( "char_update: first_char->prev != NULL... fixed", 0 );
				ch->prev = NULL;
			}
			gch_prev = ch->prev;

			if( gch_prev && gch_prev->next != ch )
			{
				bug( "char_update: ch->prev->next != ch", 0 );
				return;
			}

			if( !IS_NPC( ch ) )
			{
				update_storage( ch );
				check_lottery( ch );
			}

		}
	}

	for( shardnum = 0; shardnum <= 0; shardnum++ )
	{
		for( ;; )
		{
			room = get_room_index( number_range( 200, 20000 ) );
			if( room )
				if( ( str_cmp( room->name, "Floating in a void" ) )
					&& !xIS_SET( room->room_flags, ROOM_SPACECRAFT )
					&& !xIS_SET( room->room_flags, ROOM_PROTOTYPE ) && ( !xIS_SET( room->room_flags, ROOM_PLR_HOME ) )
					&& str_cmp( room->area->filename, "Homes.are" ) )
					break;
		}

		shard = create_object( get_obj_index( OBJ_SHARD ), 0 );
		shard = obj_to_room( shard, room );
		count++;
	}


	for( shardnum = 0; shardnum <= 0; shardnum++ )
	{
		for( ;; )
		{
			room = get_room_index( number_range( 200, 20000 ) );
			if( room )
				if( ( str_cmp( room->name, "Floating in a void" ) )
					&& !xIS_SET( room->room_flags, ROOM_SPACECRAFT )
					&& !xIS_SET( room->room_flags, ROOM_PROTOTYPE ) && ( !xIS_SET( room->room_flags, ROOM_PLR_HOME ) )
					&& str_cmp( room->area->filename, "Homes.are" ) )
					break;
		}
	}


	log_printf( "%d new shards loaded.\n", count );

	hidefoldmessage = true;

	for( tarea = first_area; tarea; tarea = tarea->next )
	{
		if( !IS_SET( tarea->status, AREA_LOADED ) )
			continue;

		fold_area( tarea, tarea->filename, false );
	}

	hidefoldmessage = false;
}

int unread_notes( CHAR_DATA *ch, BOARD_DATA *board );

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
	static int pulse_taxes;
	static int pulse_area;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_second;
	static int pulse_half_second;
	static int pulse_minute;
	static int pulse_space;
	static int pulse_ship;
	static int pulse_mailcheck;
	static int pulse_recharge;
	static int pulse_auction;
	struct timeval stime;
	struct timeval etime;
	static int pulse_war;
	static int pulse_prevtimer;
	static int pulse_heal;
	static int pulse_hourprog;
	static int pulse_music;
	static int pulse_charge;
	static int pulse_articles;

	if( --pulse_music <= 0 )
	{
		pulse_music = PULSE_MUSIC;
		//  update_radio();
	}

	if( --pulse_charge <= 0 )
	{
		pulse_charge = PULSE_CHARGE;
		update_charge( );
	}

	if( --pulse_articles <= 0 )
	{
		CHAR_DATA *ach;

		for( ach = first_char; ach; ach = ach->next )
		{
			if( IS_NPC( ach ) )
				continue;
			if( ach->top_level < 7 || ach->top_level >= LEVEL_STAFF )
				continue;

			if( unread_notes( ach, &boards[board_lookup( "Articles" )] ) > 0 )
				send_to_char( "&P( &YNotice! &P) &zYou have unread notes on the \"&CArticles&z\" board. Please read them!\r\n", ach );
		}
		pulse_articles = PULSE_MINUTE * 2;
	}

	if( timechar )
	{
		set_char_color( AT_PLAIN, timechar );
		send_to_char( "Starting update timer.\r\n", timechar );
		gettimeofday( &stime, NULL );
	}

	/*
	 * Don't put anything in here that you don't want checked EXACTLY
	 * every minute. This thing can fire anywhere from a half minute
	 * to ever minute and a half. --Halcyon
	 */
	if( --pulse_area <= 0 )
	{
		pulse_area = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
		area_update( );
	}

	if( --pulse_minute <= 0 )
	{
		quest_update( );
		spouse_update( );
		bounty_update( );
		pulse_minute = PULSE_MINUTE;
	}

	if( --pulse_taxes <= 0 )
	{
		pulse_taxes = PULSE_TAXES;
		update_taxes( );
		interestgain( );     
	}

	if( --pulse_mobile <= 0 )
	{
		pulse_mobile = PULSE_MOBILE;
		mobile_update( );
	}

	if( --pulse_heal <= 0 )
	{
		pulse_heal = PULSE_HEAL;
		healchar( );
	}

	if( --pulse_space <= 0 )
	{
		pulse_space = PULSE_SPACE;
		update_space( );
		update_bus( );
		update_traffic( );
	}

	if( --pulse_prevtimer <= 0 )
	{
		pulse_prevtimer = PULSE_PREVTIMER;
		update_prevtimer( );
	}

	if( --pulse_hourprog <= 0 )
	{
		pulse_hourprog = PULSE_HOURPROG;
		update_hourprog( );
	}

	if( --pulse_recharge <= 0 )
	{
		pulse_recharge = PULSE_SPACE / 3;
		recharge_ships( );
	}

	if( --pulse_ship <= 0 )
	{
		pulse_ship = PULSE_SPACE / 10;
		move_ships( );
	}

	if( --pulse_mailcheck <= 0 )
	{
		pulse_mailcheck = PULSE_MAILCHECK;
		mail_check( );
	}

	if( --pulse_war <= 0 )
	{
		pulse_war = PULSE_WAR;
		//        war_update ( );
	}

	if( --pulse_violence <= 0 )
	{
		pulse_violence = PULSE_VIOLENCE;
		violence_update( );
	}

	if( --pulse_point <= 0 )
	{
		pulse_point = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

		auth_update( );  /* Gorog */
		weather_update( );
		char_update( );
		obj_update( );
		clear_vrooms( ); /* remove virtual rooms */
		tag_update( );
		war_update( );
	}

	if( --pulse_half_second <= 0 )
	{
		pulse_half_second = PULSE_PER_HALF_SECOND;
		suit_timer( );
	}

	if( --pulse_second <= 0 )
	{
		pulse_second = PULSE_PER_SECOND;
		char_check( );
		check_dns( );
//		check_pfiles( 0 );
		reboot_check( 0 );
	}

	if( --pulse_auction <= 0 )
	{
		pulse_auction = PULSE_AUCTION;
		auction_update( );
	}

	mpsleep_update( );
	tele_update( );
	aggr_update( );
	obj_act_update( );
	room_act_update( );
	clean_obj_queue( ); /* dispose of extracted objects */
	clean_char_queue( );    /* dispose of dead mobs/quitting chars */
	if( timechar )
	{
		gettimeofday( &etime, NULL );
		set_char_color( AT_PLAIN, timechar );
		send_to_char( "Update timing complete.\r\n", timechar );
		subtract_times( &etime, &stime );
		ch_printf( timechar, "Timing took %d.%06d seconds.\r\n", etime.tv_sec, etime.tv_usec );
		timechar = NULL;
	}
	tail_chain( );
	return;
}

void remove_portal( OBJ_DATA *portal )
{
	ROOM_INDEX_DATA *fromRoom, *toRoom;
	CHAR_DATA *ch;
	EXIT_DATA *pexit;
	bool found;

	if( !portal )
	{
		bug( "remove_portal: portal is NULL", 0 );
		return;
	}

	fromRoom = portal->in_room;
	found = false;
	if( !fromRoom )
	{
		bug( "remove_portal: portal->in_room is NULL", 0 );
		return;
	}

	for( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
		if( IS_SET( pexit->exit_info, EX_PORTAL ) )
		{
			found = true;
			break;
		}

	if( !found )
	{
		bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
		return;
	}

	if( pexit->vdir != DIR_PORTAL )
		bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

	if( ( toRoom = pexit->to_room ) == NULL )
		bug( "remove_portal: toRoom is NULL", 0 );

	extract_exit( fromRoom, pexit );
	/*
	 * rendunancy
	 */
	 /*
	  * send a message to fromRoom
	  */
	  /*
	   * ch = fromRoom->first_person;
	   */
	   /*
		* if(ch!=NULL)
		*/
		/*
		 * act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM );
		 */

		 /*
		  * send a message to toRoom
		  */
	if( toRoom && ( ch = toRoom->first_person ) != NULL )
		act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

	/*
	 * remove the portal obj: looks better to let update_obj do this
	 */
	 /*
	  * extract_obj(portal);
	  */

	return;
}

void reboot_check( time_t reset )
{
	static const char *tmsg[] = { "SYSTEM: Reboot in 10 seconds.",
	   "SYSTEM: Reboot in 30 seconds.",
	   "SYSTEM: Reboot in 1 minute.",
	   "SYSTEM: Reboot in 2 minutes.",
	   "SYSTEM: Reboot in 3 minutes.",
	   "SYSTEM: Reboot in 4 minutes.",
	   "SYSTEM: Reboot in 5 minutes.",
	   "SYSTEM: Reboot in 10 minutes.",
	};
	static const int times[] = { 10, 30, 60, 120, 180, 240, 300, 600 };
	static const int timesize = 8; // UMIN( sizeof( times ) / sizeof( *times ), sizeof( tmsg ) / sizeof( *tmsg ) );
	char buf[MAX_STRING_LENGTH];
	static int trun;
	static bool init;

	if( !init || reset >= current_time )
	{
		for( trun = timesize - 1; trun >= 0; trun-- )
			if( reset >= current_time + times[trun] )
				break;
		init = true;
		return;
	}

	if( ( current_time % 1800 ) == 0 )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%.24s: %d players", ctime( &current_time ), num_descriptors );
		append_to_file( USAGE_FILE, buf );
	}

	if( new_boot_time_t - boot_time < 60 * 60 * 18 && !set_boot_time->manual )
		return;

	if( new_boot_time_t <= current_time )
	{
		CHAR_DATA *vch;
		extern bool mud_down;

		echo_to_all( AT_YELLOW, "You are forced from these realms by a strong "
			"presence\n\ras life here is reconstructed.", ECHOTAR_ALL );

		for( vch = first_char; vch; vch = vch->next )
			if( !IS_NPC( vch ) )
				save_char_obj( vch );
		mud_down = true;
		return;
	}

	if( trun != -1 && new_boot_time_t - current_time <= times[trun] )
	{
		echo_to_all( AT_YELLOW, tmsg[trun], ECHOTAR_ALL );
		if( trun <= 5 )
			sysdata.DENY_NEW_PLAYERS = true;
		--trun;
		return;
	}
	return;
}

void subtract_times( struct timeval *etime, struct timeval *stime )
{
	etime->tv_sec -= stime->tv_sec;
	etime->tv_usec -= stime->tv_usec;
	while( etime->tv_usec < 0 )
	{
		etime->tv_usec += 1000000;
		etime->tv_sec--;
	}
	return;
}

/*
 * Update affecteds and RIS for a character in case things get messed.
 * This should only really be used as a quick fix until the cause
 * of the problem can be hunted down. - FB
 * Last modified: June 30, 1997
 *
 * Quick fix?  Looks like a good solution for a lot of problems.
 */

 /* Temp mod to bypass immortals so they can keep their mset affects,
  * just a band-aid until we get more time to look at it -- Blodkai */
void update_aris( CHAR_DATA *ch )
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int hiding;

	if( IS_NPC( ch ) || IS_IMMORTAL( ch ) )
		return;

	/*
	 * So chars using hide skill will continue to hide
	 */
	hiding = IS_AFFECTED( ch, AFF_HIDE );

	ch->affected_by = 0;
	ch->resistant = 0;
	ch->immune = 0;
	ch->susceptible = 0;
	/*
		xCLEAR_BITS(ch->no_affected_by);
		ch->no_resistant = 0;
		ch->no_immune = 0;
		ch->no_susceptible = 0;
	*/
	/*
	 * Add in effects from race
	 */

	 //  xSET_BITS(ch->affected_by, race_table[ch->race]->affected);
	 //  SET_BIT(ch->resistant, race_table[ch->race]->resist);
	 //  SET_BIT(ch->susceptible, race_table[ch->race]->suscept);

		/*
		 * Add in effects from deities
		 */
		 /*
			 if (ch->pcdata->deity)
			 {
				 if (ch->pcdata->favor > ch->pcdata->deity->affectednum)
				 SET_BIT(ch->affected_by, ch->pcdata->deity->affected);
				 if (ch->pcdata->favor > ch->pcdata->deity->elementnum)
				 SET_BIT(ch->resistant, ch->pcdata->deity->element);
				 if (ch->pcdata->favor < ch->pcdata->deity->susceptnum)
				 SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);
			 }
		 */
		 /*
		  * Add in effect from spells
		  */
	for( paf = ch->first_affect; paf; paf = paf->next )
		aris_affect( ch, paf );

	/*
	 * Add in effects from equipment
	 */
	for( obj = ch->first_carrying; obj; obj = obj->next_content )
	{
		if( obj->wear_loc != WEAR_NONE )
		{
			for( paf = obj->first_affect; paf; paf = paf->next )
				aris_affect( ch, paf );

			for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
				aris_affect( ch, paf );
		}
	}

	/*
	 * If they were hiding before, make them hiding again
	 */
	if( hiding )
		SET_BIT( ch->affected_by, AFF_HIDE );

	return;
}

/*
 * Apply only affected and RIS on a char
 */
void aris_affect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	//    SET_BIT(ch->affected_by, paf->bitvector);
	switch( paf->location % REVERSE_APPLY )
	{
	case APPLY_AFFECT:
		SET_BIT( ch->affected_by, paf->modifier );
		break;
	case APPLY_RESISTANT:
		SET_BIT( ch->resistant, paf->modifier );
		break;
	case APPLY_IMMUNE:
		SET_BIT( ch->immune, paf->modifier );
		break;
	case APPLY_SUSCEPTIBLE:
		SET_BIT( ch->susceptible, paf->modifier );
		break;
	}
}

void update_charge( )
{
	CHAR_DATA *ch;

	for( ch = last_char; ch; ch = gch_prev )
	{
		if( ch == first_char && ch->prev )
		{
			bug( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if( gch_prev && gch_prev->next != ch )
		{
			bug( "char_update: ch->prev->next != ch", 0 );
			return;
		}


		if( !IS_NPC( ch ) )
		{

			if( IS_SET( ch->pcdata->cybaflags, CYBA_ISCHARGING ) )
			{
				if( IS_SET( ch->pcdata->cybaflags, CYBA_REFILLEN ) )
				{
					ch->pcdata->xenergy += 100;

					if( ch->pcdata->xenergy > ch->pcdata->xenergymax )
					{
						ch->pcdata->xenergy = ch->pcdata->xenergymax;
						REMOVE_BIT( ch->pcdata->cybaflags, CYBA_REFILLEN );
						REMOVE_BIT( ch->pcdata->cybaflags, CYBA_ISCHARGING );
					}
					return;
				}

				ch->pcdata->chargelevel += 1;

				if( ch->pcdata->chargelevel == 1 )
				{
					send_to_char( "&wA &Ggreen&w aura surrounds your buster.\r\n", ch );
					act( AT_BYE, "&wA cluster of energy surrounds $n's buster.", ch, NULL, NULL, TO_ROOM );
				}

				if( IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER ) && ( ch->pcdata->chargelevel == CHARGE_TWO ) )
				{
					send_to_char( "&wA &Clight blue&w aura surrounds you.\r\n", ch );
					act( AT_BYE, "&w$n is surrounded by a &Clight blue&w aura.", ch, NULL, NULL, TO_ROOM );
					if( !IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER2 )
						&& ( !IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER3 ) ) )
					{
						REMOVE_BIT( ch->pcdata->cybaflags, CYBA_ISCHARGING );
						return;
					}
				}
				if( IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER2 ) && ( ch->pcdata->chargelevel == CHARGE_THREE ) )
				{
					send_to_char( "&wA &ppurple&w aura surrounds you.\r\n", ch );
					act( AT_BYE, "&w$n is enveloped by a &ppurple&w aura..", ch, NULL, NULL, TO_ROOM );
					if( !IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER3 ) )
					{
						REMOVE_BIT( ch->pcdata->cybaflags, CYBA_ISCHARGING );
						return;
					}
				}
				if( IS_SET( ch->pcdata->cybaflags, CYBA_HASCHARGER3 ) && ( ch->pcdata->chargelevel == CHARGE_FOUR ) )
				{
					send_to_char( "&wA &Rred&w aura surrounds you.\r\n", ch );
					act( AT_BYE, "&w$n is engulfed in a &Rred&w aura.", ch, NULL, NULL, TO_ROOM );
					REMOVE_BIT( ch->pcdata->cybaflags, CYBA_ISCHARGING );
					return;
				}



			}

		}
	}
}

void bounty_update( void )
{
	CHAR_DATA *ch, *ch_next;

	for( ch = first_char; ch != NULL; ch = ch_next )
	{
		ch_next = ch->next;

		if( IS_NPC( ch ) )
			continue;

		if( ch->wfm_timer > 0 )
		{
			ch->wfm_timer--;

			if( ch->wfm_timer == 0 )
			{
				send_to_char( "&GYou may now kill for bounty experience again.\r\n", ch );
				return;
			}
		}
	}
	return;
}
