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
#include <sys/stat.h>
//#include <sys/dir.h>
#include "mud.h"

extern char lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *gch_prev;
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace );

bool iswar;
int lo_level;
int hi_level;
int inwar;
int wartimeleft;
int wartimer;
int arenatype;

/* From Skills.c */
int ris_save( CHAR_DATA *ch, int chance, int ris );

/*
 * Local functions.
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );
void group_gain( CHAR_DATA *ch, CHAR_DATA *victim );
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim );
int align_compute( CHAR_DATA *gch, CHAR_DATA *victim );
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
int obj_hitroll( OBJ_DATA *obj );
bool get_cover( CHAR_DATA *ch );
bool dual_flip = false;
bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );

DECLARE_DO_FUN( do_vdpi );
DECLARE_DO_FUN( do_vdtr );
DECLARE_DO_FUN( do_vdth );
DECLARE_DO_FUN( do_vdsc );
DECLARE_DO_FUN( do_vdbn );
int OFFHAND = 0;
/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	if( ( obj = get_eq_char( ch, WEAR_WIELD ) ) && ( xIS_SET( obj->extra_flags, ITEM_POISONED ) ) )
		return true;

	return false;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( !ch->hunting || ch->hunting->who != victim )
		return false;

	return true;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( !ch->hating || ch->hating->who != victim )
		return false;

	return true;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( !ch->fearing || ch->fearing->who != victim )
		return false;

	return true;
}


void stop_hunting( CHAR_DATA *ch )
{
	if( ch->hunting )
	{
		STRFREE( ch->hunting->name );
		DISPOSE( ch->hunting );
		ch->hunting = NULL;
	}
	return;
}

void stop_hating( CHAR_DATA *ch )
{
	if( ch->hating )
	{
		STRFREE( ch->hating->name );
		DISPOSE( ch->hating );
		ch->hating = NULL;
	}
	return;
}

void stop_fearing( CHAR_DATA *ch )
{
	if( ch->fearing )
	{
		STRFREE( ch->fearing->name );
		DISPOSE( ch->fearing );
		ch->fearing = NULL;
	}
	return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( ch->hunting )
		stop_hunting( ch );

	CREATE( ch->hunting, HHF_DATA, 1 );
	ch->hunting->name = QUICKLINK( victim->name );
	ch->hunting->who = victim;
	return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( ch->hating )
		stop_hating( ch );

	CREATE( ch->hating, HHF_DATA, 1 );
	ch->hating->name = QUICKLINK( victim->name );
	ch->hating->who = victim;
	return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if( ch->fearing )
		stop_fearing( ch );

	CREATE( ch->fearing, HHF_DATA, 1 );
	ch->fearing->name = QUICKLINK( victim->name );
	ch->fearing->who = victim;
	return;
}


int max_fight( CHAR_DATA *ch )
{
	return 8;
}

bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	int chance = 0;

	if( ( get_eq_char( victim, WEAR_WIELD ) == NULL ) ||
		( !IS_AWAKE( victim ) ) ||
		( !can_see( victim, ch ) ) || ( !IS_NPC( victim ) && victim->pcdata->learned[gsn_counter] < 1 ) )
		return false;

	chance += ( victim->skill_level[COMBAT_ABILITY] / 200 );
	chance -= ( ( ch->skill_level[COMBAT_ABILITY] ) / 200 );

	if( !IS_NPC( victim ) )
	{
		chance += ( victim->pcdata->learned[gsn_counter] / 5 );
	}
	else
	{
		chance += 13;
	}

	if( number_percent( ) >= chance )
		return false;

	dt = gsn_counter;

	/*
		   if ( dt >= TYPE_HIT && dt < TYPE_HIT +
			  sizeof(attack_table)/sizeof(attack_table[0]) )
			dt = attack_table[dt - TYPE_HIT];
	*/

	act( AT_GREEN, "You stop $n's attack and counter with your own!", ch, NULL, victim, TO_VICT );
	act( AT_GREEN, "$N counters your attack!", ch, NULL, victim, TO_CHAR );

	damage( victim, ch, dam * 2.1, dt );
	learn_from_success( victim, gsn_counter );

	return true;
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update( void )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	CHAR_DATA *lst_ch;
	CHAR_DATA *victim;
	CHAR_DATA *rch, *rch_next;
	AFFECT_DATA *paf, *paf_next;
	TIMER *timer, *timer_next;
	ch_ret retcode;
	SKILLTYPE *skill;

	lst_ch = NULL;
	for( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
	{
		set_cur_char( ch );

		if( ch == first_char && ch->prev )
		{
			bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;

		if( gch_prev && gch_prev->next != ch )
		{
			sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.", ch->name );
			bug( buf, 0 );
			bug( "Short-cutting here", 0 );
			ch->prev = NULL;
			gch_prev = NULL;
			do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
		}

		/*
		 * See if we got a pointer to someone who recently died...
		 * if so, either the pointer is bad... or it's a player who
		 * "died", and is back at the healer...
		 * Since he/she's in the char_list, it's likely to be the later...
		 * and should not already be in another fight already
		 */
		if( char_died( ch ) )
			continue;

		/*
		 * See if we got a pointer to some bad looking data...
		 */
		if( !ch->in_room || !ch->name )
		{
			log_string( "violence_update: bad ch record!  (Shortcutting.)" );
			sprintf( buf, "ch: %ld  ch->in_room: %ld  ch->prev: %ld  ch->next: %ld",
				( long ) ch, ( long ) ch->in_room, ( long ) ch->prev, ( long ) ch->next );
			log_string( buf );
			log_string( lastplayercmd );

			if( lst_ch )
				sprintf( buf, "lst_ch: %ld  lst_ch->prev: %ld  lst_ch->next: %ld",
					( long ) lst_ch, ( long ) lst_ch->prev, ( long ) lst_ch->next );
			else
				strcpy( buf, "lst_ch: NULL" );
			log_string( buf );
			gch_prev = NULL;
			continue;
		}

		/*
		 * Experience gained during battle deceases as battle drags on
		 */
		if( ch->fighting )
		{
			if( ( ++ch->fighting->duration % 24 ) == 0 )
				ch->fighting->xp = ( ( ch->fighting->xp * 9 ) / 10 );
		}


		for( timer = ch->first_timer; timer; timer = timer_next )
		{
			timer_next = timer->next;
			if( --timer->count <= 0 )
			{
				if( timer->type == TIMER_DO_FUN )
				{
					int tempsub;

					tempsub = ch->substate;
					ch->substate = timer->value;
					( timer->do_fun ) ( ch, "" );
					if( char_died( ch ) )
						break;
					ch->substate = tempsub;
				}
				extract_timer( ch, timer );
			}
		}

		if( char_died( ch ) )
			continue;

		/*
		 * We need spells that have shorter durations than an hour.
		 * So a melee round sounds good to me... -Thoric
		 */
		for( paf = ch->first_affect; paf; paf = paf_next )
		{
			paf_next = paf->next;
			if( paf->duration > 0 )
				paf->duration--;
			else if( paf->duration < 0 )
				;
			else
			{
				if( !paf_next || paf_next->type != paf->type || paf_next->duration > 0 )
				{
					skill = get_skilltype( paf->type );
					if( paf->type > 0 && skill && skill->msg_off )
					{
						set_char_color( AT_WEAROFF, ch );
						send_to_char( skill->msg_off, ch );
						send_to_char( "\r\n", ch );
					}
				}
				if( paf->type == gsn_possess )
				{
					ch->desc->character = ch->desc->original;
					ch->desc->original = NULL;
					ch->desc->character->desc = ch->desc;
					ch->desc->character->switched = NULL;
					ch->desc = NULL;
				}
				affect_remove( ch, paf );
			}
		}

		if( ( victim = who_fighting( ch ) ) == NULL || IS_AFFECTED( ch, AFF_PARALYSIS ) )
			continue;

		retcode = rNONE;

		if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		{
			sprintf( buf, "violence_update: %s fighting %s in a SAFE room.", ch->name, victim->name );
			log_string( buf );
			stop_fighting( ch, true );
		}
		else if( IS_AWAKE( ch ) && ch->in_room == victim->in_room )
			retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		else
			stop_fighting( ch, false );

		if( char_died( ch ) )
			continue;

		if( retcode == rCHAR_DIED || ( victim = who_fighting( ch ) ) == NULL )
			continue;

		/*
		 *  Mob triggers
		 */
		rprog_rfight_trigger( ch );
		if( char_died( ch ) )
			continue;
		mprog_hitprcnt_trigger( ch, victim );
		if( char_died( ch ) )
			continue;
		mprog_fight_trigger( ch, victim );
		if( char_died( ch ) )
			continue;

		/*
		 * Fun for the whole family!
		 */
		for( rch = ch->in_room->first_person; rch; rch = rch_next )
		{
			rch_next = rch->next_in_room;

			if( IS_AWAKE( rch ) && !rch->fighting )
			{
				/*
				 * PC's auto-assist others in their group.
				 */
				if( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
				{
					if( ( !IS_NPC( rch ) || IS_AFFECTED( rch, AFF_CHARM ) ) && is_same_group( ch, rch ) )
						multi_hit( rch, victim, TYPE_UNDEFINED );
					continue;
				}

				/*
				 * NPC's assist NPC's of same type or 12.5% chance regardless.
				 */
				if( IS_NPC( rch ) && !IS_AFFECTED( rch, AFF_CHARM ) && !xIS_SET( rch->act, ACT_NOASSIST ) )
				{
					if( char_died( ch ) )
						break;
					if( rch->pIndexData == ch->pIndexData || number_bits( 3 ) == 0 )
					{
						CHAR_DATA *vch;
						CHAR_DATA *target;
						int number;

						target = NULL;
						number = 0;
						for( vch = ch->in_room->first_person; vch; vch = vch->next )
						{
							if( can_see( rch, vch ) && is_same_group( vch, victim ) && number_range( 0, number ) == 0 )
							{
								target = vch;
								number++;
							}
						}

						if( target )
							multi_hit( rch, target, TYPE_UNDEFINED );
					}
				}
			}
		}
	}
	return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	int chance;
	int dual_bonus;
	ch_ret retcode;

	/*
	 * add timer if player is attacking another player
	 */
	if( !IS_NPC( ch ) && !IS_NPC( victim ) )
		add_timer( ch, TIMER_RECENTFIGHT, 50, NULL, 0 );

	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) )
		return rNONE;

	if( ( retcode = one_hit( ch, victim, dt ) ) != rNONE )
		return retcode;

	if( who_fighting( ch ) != victim || dt == gsn_backstab
		|| dt == gsn_circle || dt == gsn_strangle )
		return rNONE;

	/*
	 * Very high chance of hitting compared to chance of going berserk
	 */
	 /*
	  * 40% or higher is always hit.. don't learn anything here though.
	  */
	  /*
	   * -- Altrag
	   */
	chance = IS_NPC( ch ) ? 100 : ( ch->pcdata->learned[gsn_berserk] * 5 / 2 );
	if( IS_AFFECTED( ch, AFF_BERSERK ) && number_percent( ) < chance )
		if( ( retcode = one_hit( ch, victim, dt ) ) != rNONE || who_fighting( ch ) != victim )
			return retcode;

	if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
	{
		dual_bonus = IS_NPC( ch ) ? ( ch->skill_level[COMBAT_ABILITY] / 10 ) : ( ch->pcdata->learned[gsn_dual_wield] / 10 );
		chance = IS_NPC( ch ) ? ch->top_level : ch->pcdata->learned[gsn_dual_wield];
		if( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_dual_wield );
			retcode = one_hit( ch, victim, dt );
			if( retcode != rNONE || who_fighting( ch ) != victim )
				return retcode;
		}
		else
			learn_from_failure( ch, gsn_dual_wield );
	}
	else
		dual_bonus = 0;

	if( ch->move < 10 )
		dual_bonus = -20;

	if( !IS_NPC( ch ) )
	{
		int increase;
		increase = number_range( 5, 10 );
		ch->pcdata->ragemeter += increase;
	}

	/*
	 * NPC predetermined number of attacks          -Thoric
	 */
	if( IS_NPC( ch ) && ch->numattacks > 0 )
	{
		for( chance = 0; chance < ch->numattacks; chance++ )
		{
			retcode = one_hit( ch, victim, dt );
			if( retcode != rNONE || who_fighting( ch ) != victim )
				return retcode;
		}
		return retcode;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ( ch->pcdata->learned[gsn_second_attack] + dual_bonus ) / 1.5 );
	if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
	{
		learn_from_success( ch, gsn_second_attack );
		retcode = one_hit( ch, victim, dt );
		if( retcode != rNONE || who_fighting( ch ) != victim )
			return retcode;

		if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
		{
			if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_offhand] != 0 )
			{
				chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ( ch->pcdata->learned[gsn_offhand] + dual_bonus ) / 1.1 );
				if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
				{
					learn_from_success( ch, gsn_offhand );
					OFFHAND = 1;
					retcode = one_hit( ch, victim, dt );
					OFFHAND = 0;
					if( retcode != rNONE || who_fighting( ch ) != victim )
						return retcode;
				}
			}
		}
	}
	else
		learn_from_failure( ch, gsn_second_attack );

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ( ch->pcdata->learned[gsn_third_attack] + ( dual_bonus * 1.5 ) ) / 2 );
	if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
	{
		learn_from_success( ch, gsn_third_attack );
		retcode = one_hit( ch, victim, dt );
		if( retcode != rNONE || who_fighting( ch ) != victim )
			return retcode;

		if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
		{
			if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_offhand] != 0 )
			{
				chance = IS_NPC( ch ) ? ch->top_level
					: ( int ) ( ( ch->pcdata->learned[gsn_offhand] + dual_bonus ) / 1.2 );
				if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
				{
					learn_from_success( ch, gsn_offhand );
					OFFHAND = 1;
					retcode = one_hit( ch, victim, dt );
					OFFHAND = 0;
					if( retcode != rNONE || who_fighting( ch ) != victim )
						return retcode;
				}
			}
		}
	}
	else
		learn_from_failure( ch, gsn_third_attack );

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ( ch->pcdata->learned[gsn_fourth_attack] + ( dual_bonus * 1.5 ) ) / 2.5 );
	if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
	{
		learn_from_success( ch, gsn_fourth_attack );
		retcode = one_hit( ch, victim, dt );
		if( retcode != rNONE || who_fighting( ch ) != victim )
			return retcode;

		if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
		{
			if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_offhand] != 0 )
			{
				chance = IS_NPC( ch ) ? ch->top_level
					: ( int ) ( ( ch->pcdata->learned[gsn_offhand] + dual_bonus ) / 1.3 );
				if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
				{
					learn_from_success( ch, gsn_offhand );
					OFFHAND = 1;
					retcode = one_hit( ch, victim, dt );
					OFFHAND = 0;
					if( retcode != rNONE || who_fighting( ch ) != victim )
						return retcode;
				}
			}
		}
	}
	else
		learn_from_failure( ch, gsn_fourth_attack );

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ( ch->pcdata->learned[gsn_fifth_attack] + ( dual_bonus * 1.5 ) ) / 3 );
	if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
	{
		learn_from_success( ch, gsn_fifth_attack );
		retcode = one_hit( ch, victim, dt );
		if( retcode != rNONE || who_fighting( ch ) != victim )
			return retcode;

		if( get_eq_char( ch, WEAR_DUAL_WIELD ) )
		{
			if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_offhand] != 0 )
			{
				chance = IS_NPC( ch ) ? ch->top_level
					: ( int ) ( ( ch->pcdata->learned[gsn_offhand] + ( dual_bonus * 1.5 ) ) / 1.4 );
				if( number_percent( ) < chance || ( !IS_NPC( ch ) && IS_SET( ch->pcdata->cybaflags, CYBA_ASCENDED ) ) )
				{
					learn_from_success( ch, gsn_offhand );
					OFFHAND = 1;
					retcode = one_hit( ch, victim, dt );
					OFFHAND = 0;
					if( retcode != rNONE || who_fighting( ch ) != victim )
						return retcode;
				}
			}
		}
	}
	else
		learn_from_failure( ch, gsn_fifth_attack );

	retcode = rNONE;

	chance = IS_NPC( ch ) ? ( int ) ( ch->top_level / 4 ) : 0;
	if( number_percent( ) < chance )
		retcode = one_hit( ch, victim, dt );

	if( retcode == rNONE )
	{
		int move;

		if( !IS_AFFECTED( ch, AFF_FLYING ) && !IS_AFFECTED( ch, AFF_FLOATING ) )
			move = encumbrance( ch, movement_loss[UMIN( SECT_MAX - 1, ch->in_room->sector_type )] );
		else
			move = encumbrance( ch, 1 );
		if( ch->move )
			ch->move = UMAX( 0, ch->move - move );
	}

	return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
	int bonus;

	bonus = 0;
	*gsn_ptr = -1;
	if( !IS_NPC( ch ) && wield )
	{
		switch( wield->value[3] )
		{
		default:
			*gsn_ptr = -1;
			break;
		case 3:
			*gsn_ptr = gsn_lightsabers;
			break;
		case 2:
			*gsn_ptr = gsn_swords;
			break;
		case 4:
			*gsn_ptr = gsn_flexible_arms;
			break;
		case 5:
			*gsn_ptr = gsn_talonous_arms;
			break;
		case 6:
			*gsn_ptr = gsn_firearms;
			break;
		case 8:
			*gsn_ptr = gsn_bludgeons;
			break;
		case 9:
			*gsn_ptr = gsn_bowandarrows;
			break;
		case 11:
			*gsn_ptr = gsn_spears;
			break;

		}
		if( *gsn_ptr != -1 )
			bonus = ( int ) ( ch->pcdata->learned[*gsn_ptr] );

	}
	if( IS_NPC( ch ) && wield )
		bonus = get_trust( ch );
	return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;

	for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for( paf = obj->first_affect; paf; paf = paf->next )
		if( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
short off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
	short lvl;

	if( !IS_NPC( ch ) )  /* players get much less effect */
	{
		lvl = UMAX( 1, ( ch->skill_level[COMBAT_ABILITY] ) );
		if( number_percent( ) + ( victim->skill_level[COMBAT_ABILITY] - lvl ) < 35 )
			return lvl;
		else
			return 0;
	}
	else
	{
		lvl = ch->top_level;
		if( number_percent( ) + ( victim->skill_level[COMBAT_ABILITY] - lvl ) < 70 )
			return lvl;
		else
			return 0;
	}
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int plusris;
	int dam, x;
	int diceroll;
	int attacktype, cnt;
	int prof_bonus;
	int prof_gsn;
	ch_ret retcode;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if( victim->position == POS_DEAD || ch->in_room != victim->in_room )
		return rVICT_DIED;


	/*
	 * Figure out the weapon doing the damage           -Thoric
	 */
	if( ( wield = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
	{
		if( dual_flip == false )
		{
			dual_flip = true;
			wield = get_eq_char( ch, WEAR_WIELD );
		}
		else
			dual_flip = false;
	}
	else
		wield = get_eq_char( ch, WEAR_WIELD );

	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );

	if( ch->fighting /* make sure fight is already started */
		&& dt == TYPE_UNDEFINED && IS_NPC( ch ) && ch->attacks != 0 )
	{
		cnt = 0;
		for( ;; )
		{
			x = number_range( 0, 6 );
			attacktype = 1 << x;
			if( IS_SET( ch->attacks, attacktype ) )
				break;
			if( cnt++ > 16 )
			{
				attacktype = 0;
				break;
			}
		}
		if( attacktype == ATCK_BACKSTAB )
			attacktype = 0;
		if( wield && number_percent( ) > 25 )
			attacktype = 0;
		switch( attacktype )
		{
		default:
			break;
		case ATCK_BITE:
			do_bite( ch, "" );
			retcode = global_retcode;
			break;
		case ATCK_CLAWS:
			do_claw( ch, "" );
			retcode = global_retcode;
			break;
		case ATCK_TAIL:
			do_tail( ch, "" );
			retcode = global_retcode;
			break;
		case ATCK_STING:
			do_sting( ch, "" );
			retcode = global_retcode;
			break;
		case ATCK_PUNCH:
			do_punch( ch, "" );
			retcode = global_retcode;
			break;
			//         case ATCK_POISON:
			//            retcode = spell_smaug( gsn_poison, 1000, ch, victim );
			//            break;
		case ATCK_CIRCLE:
			do_circle( ch, "" );
			retcode = multi_hit( ch, victim, gsn_circle );
			break;

		case ATCK_KICK:
			do_kick( ch, "" );
			retcode = global_retcode;
			break;
		case ATCK_TRIP:
			attacktype = 0;
			break;
		}
		if( attacktype )
			return retcode;
	}

	if( dt == TYPE_UNDEFINED )
	{
		dt = TYPE_HIT;
		if( wield && wield->item_type == ITEM_WEAPON )
			dt += wield->value[3];
	}

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	thac0_00 = 20;
	thac0_32 = 10;
	thac0 = interpolate( ch->skill_level[COMBAT_ABILITY], thac0_00, thac0_32 ) - ( GET_HITROLL( ch ) / 1.4 );
	victim_ac = ( int ) ( GET_AC( victim ) / 10 );

	/*
	 * if you can't see what's coming...
	 */
	if( wield && !can_see_obj( victim, wield ) )
		victim_ac += 1;
	if( !can_see( ch, victim ) )
		victim_ac -= 4;

	if( !IS_AWAKE( victim ) )
		victim_ac += 5;

	 /*
	  * Weapon proficiency bonus
	  */
	victim_ac += prof_bonus / 20;

	/*
	 * The moment of excitement!
	 */
	diceroll = number_range( 1, 20 );

	if( diceroll == 1 || ( diceroll < 20 && diceroll < thac0 - victim_ac ) )
	{
		/*
		 * Miss.
		 */
		if( prof_gsn != -1 )
			learn_from_failure( ch, prof_gsn );
		damage( ch, victim, 0, dt );
		tail_chain( );
		return rNONE;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */

	if( !wield ) /* dice formula fixed by Thoric */
		dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie );
	else
		dam = number_range( wield->value[1], wield->value[2] );


	/*
	 * Bonuses.
	 */

	dam += GET_DAMROLL( ch );

	if( prof_bonus )
		dam *= ( 1 + prof_bonus / 100 );

	if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
	{
		dam += ( dam * ch->pcdata->learned[gsn_enhanced_damage] / 120 );
		learn_from_success( ch, gsn_enhanced_damage );
	}

	if( !IS_AWAKE( victim ) )
		dam *= 2;

	if( dt == gsn_backstab )
		dam *= ( 2 + URANGE( 2, ch->skill_level[HUNTING_ABILITY] - ( victim->skill_level[COMBAT_ABILITY] / 4 ), 30 ) / 8 );

	if( dt == gsn_circle )
		dam *= ( 2 + URANGE( 2, ch->skill_level[HUNTING_ABILITY] - ( victim->skill_level[COMBAT_ABILITY] / 4 ), 30 ) / 16 );
	if( dt == gsn_strangle )
		dam *= ( 2 + URANGE( 2, ch->skill_level[HUNTING_ABILITY] - ( victim->skill_level[COMBAT_ABILITY] / 4 ), 30 ) / 16 );

	plusris = 0;

	if( wield )
	{
		if( xIS_SET( wield->extra_flags, ITEM_MAGIC ) )
			dam = ris_damage( victim, dam, RIS_MAGIC );
		else
			dam = ris_damage( victim, dam, RIS_NONMAGIC );

		/*
		 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll -Thoric
		 */
		plusris = obj_hitroll( wield );
	}
	else
		dam = ris_damage( victim, dam, RIS_NONMAGIC );

	/*
	 * check for RIS_PLUSx                  -Thoric
	 */
	if( dam )
	{
		int res, imm, sus, mod;

		if( plusris )
			plusris = RIS_PLUS1 << UMIN( plusris, 7 );

		/*
		 * initialize values to handle a zero plusris
		 */
		imm = res = -1;
		sus = 1;

		/*
		 * find high ris
		 */
		for( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
		{
			if( IS_SET( victim->immune, x ) )
				imm = x;
			if( IS_SET( victim->resistant, x ) )
				res = x;
			if( IS_SET( victim->susceptible, x ) )
				sus = x;
		}
		mod = 10;
		if( imm >= plusris )
			mod -= 10;
		if( res >= plusris )
			mod -= 2;
		if( sus <= plusris )
			mod += 2;

		/*
		 * check if immune
		 */
		if( mod <= 0 )
			dam = -1;
		if( mod != 10 )
			dam = ( dam * mod ) / 10;
	}


	/*
	 * check to see if weapon is charged
	 */

	if( dt == ( TYPE_HIT + WEAPON_BLASTER ) && wield && wield->item_type == ITEM_WEAPON )
	{
		if( wield->value[4] < 1 )
		{
			act( AT_YELLOW, "$n points $s firearm at you, but nothing happens.", ch, NULL, victim, TO_VICT );
			act( AT_YELLOW, "*CLICK* ... Your firearm needs more ammunition!", ch, NULL, victim, TO_CHAR );
			if( IS_NPC( ch ) )
				do_remove( ch, wield->name );
			return rNONE;
		}
		wield->value[4]--;
	}
	else if( dt == ( TYPE_HIT + WEAPON_BOWCASTER ) && wield && wield->item_type == ITEM_WEAPON )
	{
		if( wield->value[4] < 1 )
		{
			act( AT_YELLOW, "$n points $s bow at you, but doesn't seem to have arrows.", ch, NULL, victim, TO_VICT );
			act( AT_YELLOW, "You raise your bow to fire, but... No arrows!", ch, NULL, victim, TO_CHAR );
			if( IS_NPC( ch ) )
				do_remove( ch, wield->name );
			return rNONE;
		}
		else
			wield->value[4]--;
	}

	if( dam <= 0 )
		dam = 1;

	if( prof_gsn != -1 )
	{
		if( dam > 0 )
			learn_from_success( ch, prof_gsn );
		else
			learn_from_failure( ch, prof_gsn );
	}

	/*
	 * immune to damage
	 */
	if( dam == -1 )
	{
		if( dt >= 0 && dt < top_sn )
		{
			SKILLTYPE *skill = skill_table[dt];
			bool found = false;

			if( skill->imm_char && skill->imm_char[0] != '\0' )
			{
				act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
				found = true;
			}
			if( skill->imm_vict && skill->imm_vict[0] != '\0' )
			{
				act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
				found = true;
			}
			if( skill->imm_room && skill->imm_room[0] != '\0' )
			{
				act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
				found = true;
			}
			if( found )
				return rNONE;
		}
		dam = 0;
	}


	/*
	 * //    if ( IS_NPC(victim) && (retcode = damage( ch, victim, dam, dt )) != rNONE )
	 * //        return retcode;
	 * if ( !check_counter( ch, victim, dam, dt ) )
	 * {
	 * retcode = damage( ch, victim, dam, dt );
	 * return retcode;
	 * }
	 *
	 *
	 * if ( char_died(ch) )
	 * return rCHAR_DIED;
	 * if ( char_died(victim) )
	 * return rVICT_DIED;
	 */

	 //    if ( IS_AFFECTED( ch, AFF_NEMESIS ) )
	 //        dam += 30000;

	if( ( retcode = damage( ch, victim, dam, dt ) ) != rNONE )
		return retcode;
	if( char_died( ch ) )
		return rCHAR_DIED;
	if( char_died( victim ) )
		return rVICT_DIED;

	retcode = rNONE;
	if( dam == 0 )
		return retcode;

	/*
	 * weapon spells    -Thoric
	 */
	 /*
	  * Weapon spell support            -Thoric
	  * Each successful hit casts a spell
	  */
	if( wield && !IS_SET( victim->immune, RIS_MAGIC ) && !xIS_SET( victim->in_room->room_flags, ROOM_NO_MAGIC ) )
	{
		AFFECT_DATA *aff;

		for( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
			if( aff->location == APPLY_WEAPONSPELL && IS_VALID_SN( aff->modifier ) && skill_table[aff->modifier]->spell_fun )
				retcode = ( *skill_table[aff->modifier]->spell_fun ) ( aff->modifier, ( wield->level + 3 ) / 3, ch, victim );

		if( retcode == rSPELL_FAILED ) retcode = rNONE; // Luc, 6/11/2007

		if( retcode != rNONE || char_died( ch ) || char_died( victim ) )
			return retcode;
		for( aff = wield->first_affect; aff; aff = aff->next )
			if( aff->location == APPLY_WEAPONSPELL && IS_VALID_SN( aff->modifier ) && skill_table[aff->modifier]->spell_fun )
				retcode = ( *skill_table[aff->modifier]->spell_fun ) ( aff->modifier, ( wield->level + 3 ) / 3, ch, victim );

		if( retcode == rSPELL_FAILED ) retcode = rNONE; // Luc, 6/11/2007

		if( retcode != rNONE || char_died( ch ) || char_died( victim ) )
			return retcode;
	}

	/*
	 * magic shields that retaliate             -Thoric
	 */
	if( IS_AFFECTED( victim, AFF_FIRESHIELD ) && !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
		retcode = spell_fireball( gsn_fireball, off_shld_lvl( victim, ch ), victim, ch );
	if( retcode != rNONE || char_died( ch ) || char_died( victim ) )
		return retcode;

	if( retcode != rNONE || char_died( ch ) || char_died( victim ) )
		return retcode;

	if( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) && !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
		retcode = spell_lightning_bolt( gsn_lightning_bolt, off_shld_lvl( victim, ch ), victim, ch );
	if( retcode != rNONE || char_died( ch ) || char_died( victim ) )
		return retcode;

	/*
	 *   folks with blasters move and snipe instead of getting neatin up in one spot.
	 */
	if( IS_NPC( victim ) )
	{
		wield = get_eq_char( victim, WEAR_WIELD );
		if( wield != NULL && wield->value[3] == WEAPON_BLASTER && get_cover( victim ) == true )
		{
			start_hating( victim, ch );
			start_hunting( victim, ch );
		}
	}

	tail_chain( );
	return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
int ris_damage( CHAR_DATA *ch, int dam, int ris )
{
	short modifier;

	modifier = 10;
	if( IS_SET( ch->immune, ris ) )
		return 45000;
	//      modifier -= 10;
	if( IS_SET( ch->resistant, ris ) )
		modifier -= 2;
	if( IS_SET( ch->susceptible, ris ) )
		modifier += 2;
	if( modifier <= 0 )
		return -1;
	if( modifier == 10 )
		return dam;
	return ( dam * modifier ) / 10;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	CHAR_DATA *gch;
	char buf1[MAX_STRING_LENGTH];
	short dameq;
	bool npcvict;
	bool loot;
	OBJ_DATA *damobj;
	ch_ret retcode;
	short dampmod;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *wield;

	int init_gold, new_gold, gold_diff;

	retcode = rNONE;

	if( !ch )
	{
		bug( "Damage: null ch!", 0 );
		return rERROR;
	}
	if( !victim )
	{
		bug( "Damage: null victim!", 0 );
		return rVICT_DIED;
	}

	if( victim->position == POS_DEAD )
		return rVICT_DIED;

	npcvict = IS_NPC( victim );

	/*
	 * Check damage types for RIS               -Thoric
	 */
	if( dam && dt != TYPE_UNDEFINED )
	{
		if( IS_FIRE( dt ) )
			dam = ris_damage( victim, dam, RIS_FIRE );
		else if( IS_COLD( dt ) )
			dam = ris_damage( victim, dam, RIS_COLD );
		else if( IS_ACID( dt ) )
			dam = ris_damage( victim, dam, RIS_ACID );
		else if( IS_ELECTRICITY( dt ) )
			dam = ris_damage( victim, dam, RIS_ELECTRICITY );
		else if( IS_ENERGY( dt ) )
			dam = ris_damage( victim, dam, RIS_ENERGY );
		else if( IS_DRAIN( dt ) )
			dam = ris_damage( victim, dam, RIS_DRAIN );
		else if( dt == gsn_poison || IS_POISON( dt ) )
			dam = ris_damage( victim, dam, RIS_POISON );
		else if( dt == ( TYPE_HIT + 7 ) || dt == ( TYPE_HIT + 8 ) )
			dam = ris_damage( victim, dam, RIS_BLUNT );
		else if( dt == ( TYPE_HIT + 2 ) || dt == ( TYPE_HIT + 11 ) || dt == ( TYPE_HIT + 10 ) || dt == gsn_circle || dt == gsn_backstab )
			dam = ris_damage( victim, dam, RIS_PIERCE );
		else if( dt == ( TYPE_HIT + 1 ) || dt == ( TYPE_HIT + 3 ) )
			dam = ris_damage( victim, dam, RIS_SLASH );
		else if( dt == ( TYPE_HIT + 9 ) || dt == ( TYPE_HIT + 6 ) )
			dam = ris_damage( victim, dam, RIS_SHOT );
		else if( dt == ( TYPE_HIT + 4 ) || dt == gsn_strangle )
			dam = ris_damage( victim, dam, RIS_WHIP );
		else if( dt == ( TYPE_HIT + 5 ) )
			dam = ris_damage( victim, dam, RIS_CLAW );

		if( dam == -1 )
		{
			if( dt >= 0 && dt < top_sn )
			{
				bool found = false;
				SKILLTYPE *skill = skill_table[dt];

				if( skill->imm_char && skill->imm_char[0] != '\0' )
				{
					act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
					found = true;
				}
				if( skill->imm_vict && skill->imm_vict[0] != '\0' )
				{
					act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
					found = true;
				}
				if( skill->imm_room && skill->imm_room[0] != '\0' )
				{
					act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
					found = true;
				}
				if( found )
					return rNONE;
			}
			dam = 0;
		}
	}

	if( dam && npcvict && ch != victim )
	{
		if( !xIS_SET( victim->act, ACT_SENTINEL ) )
		{
			if( victim->hunting )
			{
				if( victim->hunting->who != ch )
				{
					STRFREE( victim->hunting->name );
					victim->hunting->name = QUICKLINK( ch->name );
					victim->hunting->who = ch;
				}
			}
			else
				start_hunting( victim, ch );
		}

		if( victim->hating )
		{
			if( victim->hating->who != ch )
			{
				STRFREE( victim->hating->name );
				victim->hating->name = QUICKLINK( ch->name );
				victim->hating->who = ch;
			}
		}
		else
			start_hating( victim, ch );
	}

	if( victim != ch )
	{
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if( is_safe( ch, victim ) )
			return rNONE;


		if( victim->position > POS_STUNNED )
		{
			if( !victim->fighting && victim->in_room == ch->in_room )
				set_fighting( victim, ch );

			/*
			* vwas: victim->position = POS_FIGHTING;
			*/
			if( IS_NPC( victim ) && victim->fighting )
				victim->position = POS_FIGHTING;
			else if( victim->fighting )
				victim->position = POS_FIGHTING;
		}

		if( victim->position > POS_STUNNED )
		{
			if( !ch->fighting && victim->in_room == ch->in_room )
				set_fighting( ch, victim );

			/*
			* If victim is charmed, ch might attack victim's master.
			*/
			if( IS_NPC( ch )
				&& npcvict
				&& IS_AFFECTED( victim, AFF_CHARM )
				&& victim->master && victim->master->in_room == ch->in_room && number_bits( 3 ) == 0 )
			{
				stop_fighting( ch, false );
				retcode = multi_hit( ch, victim->master, TYPE_UNDEFINED );
				return retcode;
			}
		}


		/*
		 * More charm stuff.
		 */
		if( victim->master == ch )
			stop_follower( victim );


		/*
		 * Inviso attacks ... not.
		 */
		if( IS_AFFECTED( ch, AFF_INVISIBLE ) )
		{
			affect_strip( ch, gsn_invis );
			affect_strip( ch, gsn_mass_invis );
			REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
			act( AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
		}

		/*
		 * Take away Hide
		 */
		if( IS_AFFECTED( ch, AFF_HIDE ) )
			REMOVE_BIT( ch->affected_by, AFF_HIDE );
		/*
		 * Damage modifiers.
		 */
		if( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_SANCTUARY ) )
			dam /= 2;

		if( IS_AFFECTED( ch, AFF_NEMESIS ) )
		{
			sprintf( buf, "%s is NEMESIS AFFECTED -Delete the mother fucker-", IS_NPC( ch ) ? ch->short_descr : ch->name );
			append_to_file( BUG_FILE, buf );
		}

		if( dam < 0 )
			dam = 0;

		/*
		 * Check for disarm, trip, parry, and dodge.
		 */
		if( dt >= TYPE_HIT )
		{
			//        int dischance = 0;
			//       dischance = number_range(1,6);

			//         if( IS_NPC( ch ) && IS_SET( ch->defense, DFND_DISARM ) && dischance == 1 )
			if( IS_NPC( ch )
				&& IS_SET( ch->defenses, DFND_DISARM )
				&& number_percent( ) < ch->skill_level[COMBAT_ABILITY] / 2 )
				disarm( ch, victim );

			if( IS_NPC( ch ) && IS_SET( ch->attacks, ATCK_TRIP ) && number_percent( ) < ch->skill_level[COMBAT_ABILITY] )
				trip( ch, victim );

			if( check_parry( ch, victim ) )
				return rNONE;
			if( check_dodge( ch, victim ) )
				return rNONE;
		}



		/*
		 * Check control panel settings and modify damage
		 */
		if( IS_NPC( ch ) && dam != 45000 )
		{
			if( npcvict )
				dampmod = sysdata.dam_mob_vs_mob;
			else
				dampmod = sysdata.dam_mob_vs_plr;
		}
		else
		{
			if( npcvict && dam != 45000 )
				dampmod = sysdata.dam_plr_vs_mob;
			else
				dampmod = sysdata.dam_plr_vs_plr;
		}
		if( dampmod > 0 && dam != 45000 )
			dam = ( dam * dampmod ) / 100;
	}


	/*
	 * Code to handle equipment getting damaged, and also support  -Thoric
	 * bonuses/penalties for having or not having equipment where hit
	 */
	if( dam > 500 && dt != TYPE_UNDEFINED && dam != 45000 )
	{
		/*
		 * get a random body eq part
		 */
		dameq = number_range( WEAR_LIGHT, WEAR_EYES );
		damobj = get_eq_char( victim, dameq );
		if( damobj )
		{
			if( dam > get_obj_resistance( damobj ) )
			{
				set_cur_obj( damobj );
				damage_obj( damobj );
			}
			dam -= 50; /* add a bonus for having something to block the blow */
		}
		else
			dam += 50; /* add penalty for bare skin! */
	}

	if( ch != victim )
		dam_message( ch, victim, dam, dt );

	if( dam == 45000 )
		dam = 0;

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */

	if( !IS_NPC( ch ) )
	{
		wield = get_eq_char( ch, WEAR_WIELD );
	}

	if( !wield )
	{
		victim->hit -= dam;
	}
	else
	{
		if( !IS_NPC( ch ) && wield->pIndexData->vnum == 5126 )
		{
			if( IS_NPC( victim ) )
			{
				victim->hit -= dam;
			}
			else
			{
				victim->hit += dam;

				if( victim->hit > victim->max_hit )
				{
					ch->hit = victim->max_hit;
				}
			}

		}
		else
		{
			victim->hit -= dam;
		}
	}
	/*
	 * Get experience based on % of damage done         -Thoric
	 */
	 /*    if ( dam && ch != victim
		 &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
		 {
		 xp_gain = (int) (xp_compute( ch, victim ) * 0.1 * dam) / victim->max_hit;
		 gain_exp( ch, xp_gain, COMBAT_ABILITY );
		 }

		 if ( !IS_NPC(victim)
		 &&   victim->top_level >= LEVEL_STAFF
		 &&   victim->hit < 1 )
			victim->hit = 1;
	 */
	 /*
		 if ( !IS_NPC(victim)
		 &&   victim->top_level >= LEVEL_STAFF
		 &&   victim->hit < 1 )
			victim->hit = 1;
	 */
	 /*
	  * Make sure newbies dont die
	  */

	if( !IS_NPC( victim ) && NOT_AUTHED( victim ) && victim->hit < 1 )
		victim->hit = 1;

	if( ( dam > 0 && dt > TYPE_HIT
		&& !IS_AFFECTED( victim, AFF_POISON )
		&& is_wielding_poisoned( ch )
		&& !IS_SET( victim->immune, RIS_POISON ) && !saves_poison_death( ch->skill_level[COMBAT_ABILITY], victim ) )
		|| ( !IS_AFFECTED( victim, AFF_POISON )
			&& !IS_SET( victim->immune, RIS_POISON )
			&& ( IS_NPC( ch ) && ( IS_SET( ch->attacks, ATCK_POISON ) ) ) ) )
	{
		AFFECT_DATA af;

		af.type = gsn_poison;
		af.duration = 50;
		af.location = APPLY_STR;
		af.modifier = -5;
		af.bitvector = AFF_POISON;
		affect_join( victim, &af );
		victim->mental_state = 5;
		send_to_char( "&wYou feel &Rvery&w sick.\r\n", victim );
	}

	/*    if ( !npcvict
		&&   get_trust(victim) >= LEVEL_STAFF
		&&	 get_trust(ch)	   >= LEVEL_STAFF
		&&   victim->hit < 1 )
		victim->hit = 1; */
	update_pos( victim );

	switch( victim->position )
	{
	case POS_MORTAL:
		act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "&RYou are mortally wounded, and will die soon, if not aided.", victim );
		break;

	case POS_INCAP:
		act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "&RYou are incapacitated and will slowly die, if not aided.", victim );
		break;

	case POS_STUNNED:
		if( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
		{
			act( AT_ACTION, "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM );
			send_to_char( "&RYou are stunned, but will probably recover.", victim );
		}
		break;

	case POS_DEAD:
		if( dt >= 0 && dt < top_sn )
		{
			SKILLTYPE *skill = skill_table[dt];

			if( skill->die_char && skill->die_char[0] != '\0' )
				act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
			if( skill->die_vict && skill->die_vict[0] != '\0' )
				act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
			if( skill->die_room && skill->die_room[0] != '\0' )
				act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_NOKILL ) )
			act( AT_YELLOW, "$n flees for $s life ... barely escaping certain death!", victim, 0, 0, TO_ROOM );
		else if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_DROID ) )
			act( AT_DEAD, "$n EXPLODES into many small pieces!", victim, 0, 0, TO_ROOM );
		else
			//     act( AT_DEAD, "$n has been ANNIHILATED!", victim, 0, 0, TO_ROOM );

			if( !IS_NPC( ch ) && !IS_NPC( victim ) && !IS_SET( ch->pcdata->flags, PCFLAG_STUN ) && ( ch != victim ) )
			{
				send_to_char( "&WYou have been KILLED!\r\n", victim );
			}
			else
			{
				ch_printf( victim, "&B%s hits you nice and hard in the head, knocking you out!", ch->name );
			}
		break;

	default:
		if( dam > victim->max_hit / 4 )
		{
			act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
			if( number_bits( 3 ) == 0 )
				worsen_mental_state( victim, 1 );
		}
		if( victim->hit < victim->max_hit / 4 )

		{
			make_blood( victim );
			act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!", victim, 0, 0, TO_CHAR );
			if( number_bits( 2 ) == 0 )
				worsen_mental_state( victim, 1 );
		}
		break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if( !IS_AWAKE( victim )  /* lets make NPC's not slaughter PC's */
		&& !IS_AFFECTED( victim, AFF_PARALYSIS ) )
	{
		if( victim->fighting && victim->fighting->who->hunting && victim->fighting->who->hunting->who == victim )
			stop_hunting( victim->fighting->who );

		if( victim->fighting && victim->fighting->who->hating && victim->fighting->who->hating->who == victim )
			stop_hating( victim->fighting->who );

		stop_fighting( victim, true );
	}

	if( victim->hit <= 0 && !IS_NPC( victim ) )
	{
		OBJ_DATA *obj;
		//       OBJ_DATA *obj_next;
		//       int cnt=0;

		xREMOVE_BIT( victim->act, PLR_ATTACKER );

		stop_fighting( victim, true );

		if( ( obj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL )
			unequip_char( victim, obj );
		if( ( obj = get_eq_char( victim, WEAR_WIELD ) ) != NULL )
			unequip_char( victim, obj );
		if( ( obj = get_eq_char( victim, WEAR_HOLD ) ) != NULL )
			unequip_char( victim, obj );
		if( ( obj = get_eq_char( victim, WEAR_MISSILE_WIELD ) ) != NULL )
			unequip_char( victim, obj );
		if( ( obj = get_eq_char( victim, WEAR_LIGHT ) ) != NULL )
			unequip_char( victim, obj );
		/*
				for ( obj = victim->first_carrying; obj; obj = obj_next )
			{
				obj_next = obj->next_content;

				if ( obj->wear_loc == WEAR_NONE )
				{
				if ( HAS_PROG( obj->pIndexData, DROP_PROG ) && obj->count > 1 )
				{
				   ++cnt;
				   separate_obj( obj );
				   obj_from_char( obj );
				   if ( !obj_next )
					 obj_next = victim->first_carrying;
				}
				else
				{
				   cnt += obj->count;
				   obj_from_char( obj );
				}
				act( AT_ACTION, "$n drops $p.", victim, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You drop $p.", victim, obj, NULL, TO_CHAR );
				obj = obj_to_room( obj, victim->in_room );
				}
			}

		*/
		if( IS_NPC( ch ) && !IS_NPC( victim ) )
		{
			long lose_exp;
			lose_exp = UMAX( ( victim->experience[COMBAT_ABILITY] - exp_level( victim->skill_level[COMBAT_ABILITY] ) ), 0 );
			ch_printf( victim, "You lose %ld experience.\r\n", lose_exp );
			victim->experience[COMBAT_ABILITY] -= lose_exp;
		}

		//      add_timer( victim, TIMER_RECENTFIGHT, 100, NULL, 0 );

	}

	/*
	 * Payoff for killing things.
	 */
	if( victim->position == POS_DEAD )
	{

		if( IS_IN_WAR( ch ) && IS_IN_WAR( victim ) )
		{
			check_war( ch, victim );
			return true;
		}
		if( !IS_NPC( ch ) && !IS_NPC( victim ) &&
			xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) && xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
		{
			stop_fighting( victim, true );
			death_cry( victim );
			char_from_room( victim );
			char_to_room( victim, get_room_index( victim->retran ) );
			adjust_hiscore( "apkills", ch, ch->pcdata->apkills );
			adjust_hiscore( "apdeaths", ch, victim->pcdata->apdeaths );
			victim->hit = UMAX( 10, victim->hit );
			victim->move = UMAX( 10, victim->move );
			update_pos( victim );
			do_look( victim, "auto" );
			if( war_info.inwar == 1 )
			{
				send_to_char( "You emerge victorious in the arena!\r\n", ch );
				stop_fighting( ch, true );
				char_from_room( ch );
				char_to_room( ch, get_room_index( ch->retran ) );
				ch->hit = ch->max_hit;
				ch->move = ch->max_move;
				update_pos( ch );
				do_look( ch, "auto" );
			}
			return true;
		}


		group_gain( ch, victim );

		if( !IS_NPC( victim ) && !xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
		{
			sprintf( log_buf, "%s killed by %s&p at %d",
				victim->name, ( IS_NPC( ch ) ? ch->short_descr : ch->name ), victim->in_room->vnum );
			log_string( log_buf );
			to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_STAFF );
		}
		if( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) && arenatype == 0 )
		{
			sprintf( buf, "&B%s &Cjust smoked &B%s&C!", ch->name, victim->name );
			talk_arena( buf );
		}

		else if( !IS_NPC( victim ) && IS_SET( victim->pcdata->flags, PCFLAG_ARENA ) )
		{
			REMOVE_BIT( victim->pcdata->flags, PCFLAG_ARENA );
			inwar--;

			if( ( iswar == true ) && ( inwar >= 1 ) && ( arenatype == 1 ) )
			{
				sprintf( buf, "&B%s &Cjust knocked &B%s &Cfrom the competition!", ch->name, victim->name );
				talk_arena( buf );
			}

			if( iswar == false )
			{
				sprintf( buf, "&B%s &Cjust smoked &B%s&C!", ch->name, victim->name );
				//    echo_to_all (AT_IMMORT, buf, ECHOTAR_ALL);
				talk_arena( buf );
			}

			if( ( inwar <= 1 ) && ( iswar == true ) )
			{
				sprintf( buf, "&B%s &Chas won the competition!", ch->name );
				talk_arena( buf );
				lo_level = 0;
				hi_level = 0;
				iswar = false;
				inwar = 0;
				arenatype = 0;
				wartimer = 0;
				wartimeleft = 0;
				ch->hit = ch->max_hit;
				REMOVE_BIT( ch->pcdata->flags, PCFLAG_ARENA );
				char_from_room( ch );
				char_to_room( ch, get_room_index( ch->retran ) );
				do_look( ch, "auto" );
			}
		}

		else if( !IS_NPC( ch ) && IS_NPC( victim ) )  /* keep track of mob vnum killed */
		{
			add_kill( ch, victim );

			/*
			 * Add to kill tracker for grouped chars, as well. -Hal
			 */

			for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
				if( is_same_group( gch, ch ) && !IS_NPC( gch ) && gch != ch )
					add_kill( gch, victim );
		}

		check_killer( ch, victim );

		if( !IS_NPC( victim ) || !xIS_SET( victim->act, ACT_NOKILL ) )
			loot = legal_loot( ch, victim );
		else
			loot = false;

		set_cur_char( victim );
		raw_kill( ch, victim );
		victim = NULL;

		if( !IS_NPC( ch ) && loot )
		{
			/*
			 * Autogold by Scryn 8/12
			 */
			if( xIS_SET( ch->act, PLR_AUTOGOLD ) )
			{
				init_gold = ch->gold;
				do_get( ch, "dollars corpse" );
				new_gold = ch->gold;
				gold_diff = ( new_gold - init_gold );
				if( gold_diff > 0 )
				{
					sprintf( buf1, "%d", gold_diff );
					do_split( ch, buf1 );
				}
			}
			if( xIS_SET( ch->act, PLR_AUTOLOOT ) )
				do_get( ch, "all corpse" );
			else
				do_look( ch, "in corpse" );

			if( xIS_SET( ch->act, PLR_AUTOSAC ) )
				do_sacrifice( ch, "corpse" );
		}

		if( IS_SET( sysdata.save_flags, SV_KILL ) )
			save_char_obj( ch );
		return rVICT_DIED;
	}

	if( victim == ch )
		return rNONE;

	/*
	 * Take care of link dead people.
	 */
	if( !npcvict && !victim->desc && !victim->switched )
	{
		if( number_range( 0, victim->wait ) == 0 )
		{
			do_flee( victim, "" );
			do_flee( victim, "" );
			do_flee( victim, "" );
			do_flee( victim, "" );
			do_flee( victim, "" );
			do_hail( victim, "" );
			do_quit( victim, "" );
			return rNONE;
		}
	}

	/*
	 * Wimp out?
	 */
	if( npcvict && dam > 0 )
	{
		if( ( xIS_SET( victim->act, ACT_WIMPY ) && number_bits( 1 ) == 0
			&& victim->hit < victim->max_hit / 2 )
			|| ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master && victim->master->in_room != victim->in_room ) )
		{
			start_fearing( victim, ch );
			stop_hunting( victim );
			do_flee( victim, "" );
		}
	}

	if( !npcvict && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0 )
		do_flee( victim, "" );
	//   else
	//    if ( !npcvict && xIS_SET( victim->act, PLR_FLEE ) )
	//  do_flee( victim, "" );

	tail_chain( );
	return rNONE;
}

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{

	if( is_safe_war( ch, victim ) )
	{
		send_to_char( "They're on YOUR team.\r\n", ch );
		return true;
	}

	if( xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
		return false;

	if( IS_NPC( victim ) && ( xIS_SET( victim->act, ACT_TRAIN ) || xIS_SET( victim->act, ACT_PRACTICE ) ) )
	{
		send_to_char( "Can't attack that, sorry =)\r\n", ch );
		return true;
	}

	/*
	 * no killing in shops hack
	 */
	if( IS_NPC( victim ) && victim->pIndexData->pShop != NULL )
	{
		send_to_char( "&RDo you think the shopkeeper would like that??\r\n", ch );
		return true;
	}

	if( !victim )
		return false;

	/*
	 * Thx Josh!
	 */
	if( who_fighting( ch ) == ch )
		return false;

	if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You'll have to do that elswhere.\r\n", ch );
		return true;
	}

	if( !IS_NPC( ch ) && !IS_NPC( victim ) )
	{
		if( IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
			return false;


		if( !xIS_SET( ch->act, PLR_PKER ) )
		{
			send_to_char( "You can't do that. You are peaceful.\r\n", ch );
			return true;
		}

		if( !xIS_SET( victim->act, PLR_PKER ) )
		{
			send_to_char( "You can't do that. They are peaceful.\r\n", ch );
			return true;
		}

	}

	if( IS_NPC( ch ) )
	{
		/*
		 * charmed mobs and pets cannot attack players
		 */
		if( !IS_NPC( victim ) && ( IS_AFFECTED( ch, AFF_CHARM ) || xIS_SET( ch->act, ACT_PET ) ) )
			return true;

		return false;
	}

	else /* Not NPC */
	{
		if( IS_IMMORTAL( ch ) )
			return false;


		/*
		 * no pets
		 */
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PET ) )
		{
			send_to_char( "Its charmed! It can barely fight back!!\r\n", ch );
			return true;
		}

		/*
		 * no charmed mobs unless char is the the owner
		 */
		if( IS_AFFECTED( victim, AFF_CHARM ) && ch != victim->master )
		{
			send_to_char( "You don't own that monster.\r\n", ch );
			return true;
		}
	}
	/*
		if ( get_timer(victim, TIMER_PKILLED) > 0 )
		{
		set_char_color( AT_GREEN, ch );
			send_to_char( "No spam Pk! Wait till that player can cool down.\r\n", ch);
			return true;
		}

		if ( get_timer(ch, TIMER_PKILLED) > 0 )
		{
		set_char_color( AT_GREEN, ch );
			send_to_char( "You were just killed and you want to attack someone?? Try regrouping yourself first!\r\n", ch );
			return true;
		}
	*/
	if( get_trust( ch ) > LEVEL_HERO )
		return false;

	if( IS_NPC( ch ) || IS_NPC( victim ) )
		return false;


	return false;

}

/* checks is_safe but without the output
   cuts out imms and safe rooms as well
   for info only */

bool is_safe_nm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	return false;
}


/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
	/*
	 * pc's can now loot .. why not .. death is pretty final
	 */
	if( !IS_NPC( ch ) )
		return true;
	/*
	 * non-charmed mobs can loot anything
	 */
	if( IS_NPC( ch ) && !ch->master )
		return true;

	return false;
}

/*
see if an attack justifies a KILLER flag --- edited so that none do but can't
murder a no pk person. --- edited again for planetary wanted flags -- well will be soon :p
 */

void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{

	int x;

	/*
	 * Charm-o-rama.
	 */

	if( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
		return;

	if( IS_SET( ch->affected_by, AFF_CHARM ) )
	{
		if( !ch->master )
		{
			char buf[MAX_STRING_LENGTH];

			sprintf( buf, "Check_killer: %s bad AFF_CHARM", IS_NPC( ch ) ? ch->short_descr : ch->name );
			bug( buf, 0 );
			affect_strip( ch, gsn_charm_person );
			REMOVE_BIT( ch->affected_by, AFF_CHARM );
			return;
		}

		/*
		 * stop_follower( ch );
		 */
		if( ch->master )
			check_killer( ch->master, victim );
	}

	if( IS_NPC( victim ) )
	{
		if( !IS_NPC( ch ) )
		{
			for( x = 0; x < 32; x++ )
			{
				if( IS_SET( victim->vip_flags, 1 << x ) )
				{
					SET_BIT( ch->pcdata->wanted_flags, 1 << x );
					ch_printf( ch, "&YYou are now wanted on %s.&w\r\n", planet_flags[x], victim->short_descr );
				}
			}
			if( ch->pcdata->clan )
				ch->pcdata->clan->mkills++;
			ch->pcdata->mkills++;
			//          adjust_hiscore( "mkills", ch, ch->pcdata->mkills );
			ch->in_room->area->mkills++;
		}
		return;
	}

	if( !IS_NPC( ch ) && !IS_NPC( victim ) )
	{
		if( ch != victim )
		{
			if( ch->pcdata->clan )
				ch->pcdata->clan->pkills++;
			ch->pcdata->pkills++;
			adjust_hiscore( "pkill", ch, ch->pcdata->pkills );
		}
		adjust_hiscore( "deaths", victim, victim->pcdata->pdeaths );
		update_pos( victim );
		if( victim->pcdata->clan )
			victim->pcdata->clan->pdeaths++;

		if( !xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
			add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	}


	if( IS_NPC( ch ) )
		if( !IS_NPC( victim ) )
			victim->in_room->area->mdeaths++;

	return;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
	if( !victim )
	{
		bug( "update_pos: null victim", 0 );
		return;
	}

	if( victim->hit > 0 )
	{
		if( victim->position <= POS_STUNNED )
			victim->position = POS_STANDING;
		if( IS_AFFECTED( victim, AFF_PARALYSIS ) )
			victim->position = POS_STUNNED;
		return;
	}

	if( IS_NPC( victim ) || victim->hit <= -5 )
	{
		if( victim->mount )
		{
			act( AT_ACTION, "$n falls from $N.", victim, NULL, victim->mount, TO_ROOM );
			xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
			victim->mount = NULL;
		}
		victim->position = POS_DEAD;
		return;
	}

	if( victim->hit <= -400 )
		victim->position = POS_MORTAL;
	else if( victim->hit <= -200 )
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;

	if( victim->position > POS_STUNNED && IS_AFFECTED( victim, AFF_PARALYSIS ) )
		victim->position = POS_STUNNED;

	if( victim->mount )
	{
		act( AT_ACTION, "$n falls unconscious from $N.", victim, NULL, victim->mount, TO_ROOM );
		xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
		victim->mount = NULL;
	}
	return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	FIGHT_DATA *fight;

	if( ch->fighting )
	{
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)", ch->name, victim->name, ch->fighting->who->name );
		bug( buf, 0 );
		return;
	}

	if( IS_AFFECTED( ch, AFF_SLEEP ) )
		affect_strip( ch, gsn_sleep );

	/*
	 * Limit attackers -Thoric
	 */
	if( victim->num_fighting > max_fight( victim ) )
	{
		send_to_char( "There are too many people fighting for you to join in.\r\n", ch );
		return;
	}

	CREATE( fight, FIGHT_DATA, 1 );
	fight->who = victim;
	fight->xp = ( int ) xp_compute( ch, victim );
	fight->align = align_compute( ch, victim );
	if( !IS_NPC( ch ) && IS_NPC( victim ) )
		fight->timeskilled = times_killed( ch, victim );
	ch->num_fighting = 1;
	ch->fighting = fight;
	victim->num_fighting++;
	if( victim->switched && IS_AFFECTED( victim->switched, AFF_POSSESS ) )
	{
		send_to_char( "You are disturbed!\r\n", victim->switched );
		do_return( victim->switched, "" );
	}
	return;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
	if( !ch )
	{
		bug( "who_fighting: null ch", 0 );
		return NULL;
	}
	if( !ch->fighting )
		return NULL;
	return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
	if( !ch )
	{
		bug( "Free_fight: null ch!", 0 );
		return;
	}
	if( ch->fighting )
	{
		if( !char_died( ch->fighting->who ) )
			--ch->fighting->who->num_fighting;
		DISPOSE( ch->fighting );
	}
	ch->fighting = NULL;
	if( ch->mount )
		ch->position = POS_MOUNTED;
	else
		ch->position = POS_STANDING;
	/*
	 * Berserk wears off after combat. -- Altrag
	 */
	if( IS_AFFECTED( ch, AFF_BERSERK ) )
	{
		affect_strip( ch, gsn_berserk );
		set_char_color( AT_WEAROFF, ch );
		send_to_char( skill_table[gsn_berserk]->msg_off, ch );
		send_to_char( "\r\n", ch );
	}
	return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
	CHAR_DATA *fch;

	free_fight( ch );
	update_pos( ch );

	if( !fBoth ) /* major short cut here by Thoric */
		return;

	for( fch = first_char; fch; fch = fch->next )
	{
		if( who_fighting( fch ) == ch )
		{
			free_fight( fch );
			update_pos( fch );

			if( !IS_NPC( ch ) && ch->pcdata->tells >= 1 )
			{
				ch_printf( ch, "\n&GYou have &g%d&G messeges waiting on your CDI.\r&GType &g'&Gcheck&g'&G, to read them.\r\n",
					ch->pcdata->tells );
			}


		}
	}
	return;
}

/* Vnums for the various bodyparts */
int part_vnums[] = { 12,    /* Head */
   14,  /* arms */
   15,  /* legs */
   13,  /* heart */
   16,  /* guts */
};

/* Messages for flinging off the various bodyparts */
const char *part_messages[] = {
   "$n's severed head plops from its neck.",
   "$n's arm is sliced from $s dead body.",
   "$n's leg is sliced from $s dead body.",
   "$n's heart is torn from $s chest.",
   "$n's guts spill grotesquely from $s torso.",
   "r1 message.",
   "r2 message."
};

/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)
 * Support for additional bodyparts by Fireblade
 */
void death_cry( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA *was_in_room;
	const char *msg;
	EXIT_DATA *pexit;
	int vnum, shift, index, i;

	if( !ch )
	{
		bug( "DEATH_CRY: null ch!", 0 );
		return;
	}

	vnum = 0;
	msg = NULL;

	switch( number_range( 0, 5 ) )
	{
	default:
		msg = "You hear $n's death cry.";
		break;
	case 0:
		msg = "$n screams furiously as $e falls to the ground in a heap!";
		break;
	case 1:
		msg = "$n hits the ground ... DEAD.";
		break;
	case 2:
		msg = "$n catches $s guts in $s hands as they pour through $s fatal" " wound!";
		break;
	case 3:
		msg = "$n splatters blood on your armor.";
		break;
	case 4:
		msg = "$n gasps $s last breath and blood spurts out of $s " "mouth and ears.";
		break;
	case 5:
		shift = number_range( 0, 4 );
		index = 1 << shift;

		for( i = 0; i < 5 && ch->xflags; i++ )
		{
			if( !HAS_BODYPART( ch, index ) )
			{
				msg = part_messages[shift];
				vnum = part_vnums[shift];
				break;
			}
			else
			{
				msg = part_messages[shift];
				vnum = part_vnums[shift];
			}
		}

		if( !msg )
			msg = "You hear $n's death cry.";
		break;
	}

	act( AT_DEAD, msg, ch, NULL, NULL, TO_ROOM );

	if( vnum )
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		const char *name;

		if( !get_obj_index( vnum ) )
		{
			bug( "death_cry: invalid vnum", 0 );
			return;
		}

		name = IS_NPC( ch ) ? ch->short_descr : ch->name;
		obj = create_object( get_obj_index( vnum ), 0 );
		obj->timer = number_range( 4, 7 );
		if( IS_AFFECTED( ch, AFF_POISON ) )
			obj->value[3] = 10;

		sprintf( buf, obj->short_descr, name );
		STRFREE( obj->short_descr );
		obj->short_descr = STRALLOC( buf );

		sprintf( buf, obj->description, name );
		STRFREE( obj->description );
		obj->description = STRALLOC( buf );

		if( IS_NPC( ch ) )
		{
			obj->value[4] = 0;
		}
		else
		{
			obj->value[4] = 1;
		}

		obj = obj_to_room( obj, ch->in_room );
	}

	if( IS_NPC( ch ) )
		msg = "";
	else
		msg = "";

	was_in_room = ch->in_room;
	for( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
	{
		if( pexit->to_room && pexit->to_room != was_in_room )
		{
			ch->in_room = pexit->to_room;
			act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = was_in_room;

	return;
}


void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{

	CHAR_DATA *victmp;

	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *obj_next;
	SHIP_DATA *ship;

	if( !victim )
	{
		bug( "raw_kill: null victim!", 0 );
		return;
	}

	strcpy( arg, victim->name );

	stop_fighting( victim, true );

	if( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
	{
		REMOVE_BIT( victim->pcdata->flags, PCFLAG_WAR );
		victim->pcdata->apdeaths += 1;
		ch->pcdata->apkills += 1;
		adjust_hiscore( "apkills", ch, ch->pcdata->apkills );
		adjust_hiscore( "apdeaths", ch, victim->pcdata->apdeaths );
		char_from_room( victim );
		char_to_room( victim, get_room_index( victim->retran ) );
		victim->hit = victim->max_hit;
		victim->position = POS_RESTING;
		do_look( victim, "auto" );
		return;
	}

	if( !IS_NPC( ch ) && !IS_NPC( victim ) && IS_SET( ch->pcdata->flags, PCFLAG_STUN ) && ( ch != victim ) )
	{
		act( AT_BLUE, "$n hits $N nice and hard in the head, knocking $M out!", ch, NULL, victim, TO_NOTVICT );
		act( AT_BLUE, "You hit $N nice and hard in the head, knocking $M out!", ch, NULL, victim, TO_CHAR );
		act( AT_BLUE, "$n hits you nice and hard in the head, knocking you out!", ch, NULL, victim, TO_VICT );

		victim->position = POS_STUNNED;
		victim->hit = -100;
		victim->move = 0;

		//    if ( ch != victim )
		//        act("You slam the butt of your weapon into $N's head, knocking $M senseless!",ch,NULL,victim,TO_CHAR);
		return;

	}
	/*
		if ( !IS_NPC( victim ) )
		{
		 wield = get_eq_char( victim, WEAR_WIELD );

		 if ( wield->pIndexData->vnum == 5126 )
		 {
		   victim->hit = victim->max_hit/2;
		   char_to_room( victim, get_room_index(ROOM_MORGUE));
		   act( AT_MAGIC, "$n's Tenseiga begins to glow, emitting a healing energy that saves $m from death, transporting $m to safety before disappearing.", victim, obj, NULL, TO_ROOM );
		   do_look( victim, "" );
		   send_to_char( "Tenseiga dissapears from your grip after saving your life.\r\n", victim );
	//       remove_obj( victim, wield->wear_loc, true );
		   obj_to_room( wield, roomvnum );
		   return;
		 }
		}
	*/
	if( ch && !IS_NPC( ch ) && !IS_NPC( victim ) )
		claim_disintigration( ch, victim );

	/* Take care of polymorphed chars */
	if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_POLYMORPHED ) )
	{
		char_from_room( victim->desc->original );
		char_to_room( victim->desc->original, victim->in_room );
		victmp = victim->desc->original;
		do_revert( victim, "" );
		raw_kill( ch, victmp );
		return;
	}

	if( victim->in_room && IS_NPC( victim ) && victim->vip_flags != 0 && victim->in_room->area
		&& victim->in_room->area->planet )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			victim->in_room->area->planet->population--;
			victim->in_room->area->planet->population = UMAX( victim->in_room->area->planet->population, 0 );
			victim->in_room->area->planet->pop_support -=
				( float ) ( 1 + 1 / ( victim->in_room->area->planet->population + 1 ) );
			if( victim->in_room->area->planet->pop_support < -100 )
				victim->in_room->area->planet->pop_support = -100;
		}
	}

	if( !IS_NPC( victim ) || !xIS_SET( victim->act, ACT_NOKILL ) )
		mprog_death_trigger( ch, victim );
	if( char_died( victim ) )
		return;

	if( !IS_NPC( victim ) || !xIS_SET( victim->act, ACT_NOKILL ) )
		rprog_death_trigger( ch, victim );
	if( char_died( victim ) )
		return;

	if( !IS_NPC( victim ) || ( !xIS_SET( victim->act, ACT_NOKILL ) && !xIS_SET( victim->act, ACT_NOCORPSE ) ) )
	{
		if( !IS_IMMORTAL( victim ) )
		{
			make_corpse( victim, ch );
		}
	}
	else
	{
		if( !IS_IMMORTAL( victim ) )
		{
			for( obj = victim->last_carrying; obj; obj = obj_next )
			{
				obj_next = obj->prev_content;
				obj_from_char( obj );
				extract_obj( obj );
			}
		}
	}

	/*    make_blood( victim ); */
	if( IS_NPC( victim ) )
	{
		if( victim->pIndexData->vnum == ch->questmob )
		{
			ch->questmob = -1;
			send_to_char( "&PYou have completed your quest! Return to your employer now to gain the reward!\r\n", ch );

		}

		victim->pIndexData->killed++;
		extract_char( victim, true );
		victim = NULL;
		return;
	}
	death_cry( victim );
	set_char_color( AT_DIEMSG, victim );
	do_help( victim, "_DIEMSG_" );


	/* swreality chnages begin here */


	for( ship = first_ship; ship; ship = ship->next )
	{
		if( !str_cmp( ship->owner, victim->name ) )
		{
			STRFREE( ship->owner );
			ship->owner = STRALLOC( "" );
			STRFREE( ship->pilot );
			ship->pilot = STRALLOC( "" );
			STRFREE( ship->copilot );
			ship->copilot = STRALLOC( "" );

			if( ship->stype == 7 )
			{
				ship->alloy = 5;
			}
			else
			{
				ship->alloy = 1;
			}

			if( ship->mod == 3 && ship->offon == 1 )
			{
				ship->offon = 0;
				ship->realspeed = ship->realspeed / 2;
				ship->hyperspeed = ship->hyperspeed / 2;
			}

			save_ship( ship );
		}
	}

	if( !IS_NPC( victim ) && !IS_IMMORTAL( victim ) )
	{
		if( victim->plr_home )
		{
			ROOM_INDEX_DATA *room = victim->plr_home;
			victim->plr_home = NULL;
			STRFREE( room->name );
			room->name = STRALLOC( "An Empty Apartment" );

			xREMOVE_BIT( room->room_flags, ROOM_PLR_HOME );
			xSET_BIT( room->room_flags, ROOM_EMPTY_HOME );
			STRFREE( room->guests );

			fold_area( room->area, room->area->filename, false );
		}
	}
	/*
		if ( victim->pcdata && victim->pcdata->clan )
		{
		   if ( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
		   {
			  STRFREE( victim->pcdata->clan->leader );
			  if ( victim->pcdata->clan->number1 )
			  {
				 victim->pcdata->clan->leader = STRALLOC( victim->pcdata->clan->number1 );
				 STRFREE( victim->pcdata->clan->number1 );
				 victim->pcdata->clan->number1 = STRALLOC( "" );
			  }
			  else if ( victim->pcdata->clan->number2 )
			  {
				 victim->pcdata->clan->leader = STRALLOC( victim->pcdata->clan->number2 );
				 STRFREE( victim->pcdata->clan->number2 );
				 victim->pcdata->clan->number2 = STRALLOC( "" );
			  }
			  else
				 victim->pcdata->clan->leader = STRALLOC( "" );
		   }

		   if ( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
		   {
			  STRFREE( victim->pcdata->clan->number1 );
			  if ( victim->pcdata->clan->number2 )
			  {
				 victim->pcdata->clan->number1 = STRALLOC( victim->pcdata->clan->number2 );
				 STRFREE( victim->pcdata->clan->number2 );
				 victim->pcdata->clan->number2 = STRALLOC( "" );
			  }
			  else
				 victim->pcdata->clan->number1 = STRALLOC( "" );
		   }

		   if ( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
		   {
			  STRFREE( victim->pcdata->clan->number2 );
			  victim->pcdata->clan->number1 = STRALLOC( "" );
		   }
		}
	*/
	if( !victim )
	{
		DESCRIPTOR_DATA *d;

		/*
		 * Make sure they aren't halfway logged in.
		 */
		for( d = first_descriptor; d; d = d->next )
			if( ( victim = d->character ) && !IS_NPC( victim ) )
				break;
		if( d )
			close_socket( d, true );
	}

	else
	{
		extract_char( victim, false );
		if( !victim )
		{
			bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
			return;
		}
		while( victim->first_affect )
			affect_remove( victim, victim->first_affect );
		if( victim->pcdata->clan )
			victim->gold -= victim->top_level * 50;
		if( !IS_NPC( victim ) && IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
			REMOVE_BIT( victim->pcdata->flags, PCFLAG_TWIT );
		victim->hit = 5;
		if( !IS_NPC( ch ) && ( xIS_SET( victim->act, PLR_KILLER ) ) )
			xREMOVE_BIT( victim->act, PLR_KILLER );
		victim->position = POS_RESTING;
		char_from_room( victim );
		char_to_room( victim, get_room_index( ROOM_MORGUE ) );

		if( victim == ch )
		{
			sprintf( buf, "&C%s &YH&Oas &BB&be&Be&bn &RK&ri&RLL&re&RD&P!", victim->name );
			info_chan( buf );
		}
		else
		{
			sprintf( buf, "&C%s &YH&Oas &BB&be&Be&bn &RK&ri&RLL&re&RD &GB&gy &C%s&R!", victim->name,
				( IS_NPC( ch ) ? ch->short_descr : ch->name ) );
			info_chan( buf );
		}

		if( !IS_NPC( ch ) && ch->pcdata->inivictim )
		{
			CLAN_DATA *clan;
			DESCRIPTOR_DATA *d;

			if( !str_cmp( ch->pcdata->inivictim, victim->name ) )
			{
				clan = get_clan( ch->pcdata->iclan );
				ch->pcdata->clan = clan;
				STRFREE( ch->pcdata->clan_name );
				ch->pcdata->clan_name = QUICKLINK( ch->pcdata->iclan );
				ch_printf( ch, "Congratulations, you're now a member of %s!\r\n", ch->pcdata->clan_name );
				add_member( ch->name, clan->shortname );
				clan->members++;
				for( d = first_descriptor; d; d = d->next )
				{
					if( d->connected == CON_PLAYING )
					{
						if( IS_NPC( d->character ) )
							continue;
						if( ch == d->character )
							continue;
						if( !d->character->pcdata->clan )
							continue;

						if( !str_cmp( ch->pcdata->clan->name, d->character->pcdata->clan->name ) )
							ch_printf( d->character, "[GANG INFO] %s has succeeded in their initiation!\r\n", ch->name );
					}
				}
				STRFREE( ch->pcdata->iclan );
				STRFREE( ch->pcdata->inivictim );

			}
		}

		if( xIS_SET( ch->act, PLR_PKER ) && xIS_SET( victim->act, PLR_PKER ) && ch != victim )
		{
			{
				if( victim->pcdata->lp >= 0 )
				{
					ch->pcdata->lp += 1;
					if( ch->pcdata->lp > 55 )
						ch->pcdata->lp = 55;
				}

				if( victim->pcdata->lp < 0 && ch->pcdata->lp < 0 )
				{
					ch->pcdata->lp += 1;
					if( ch->pcdata->lp > 55 )
						ch->pcdata->lp = 55;
				}

			}

			victim->pcdata->lp -= 1;
			if( victim->pcdata->lp < -4 )
				victim->pcdata->lp = -4;
		}

		send_to_char( "You died! You're in the Morgue.\r\n", victim );
		victim->mental_state = -10;
		victim->pcdata->condition[COND_FULL] = 12;
		victim->pcdata->condition[COND_THIRST] = 12;
		victim->pcdata->pdeaths += 1;
		//  adjust_hiscore( "pkill", ch, ch->pcdata->pkills );
		//  adjust_hiscore( "deaths", victim, victim->pcdata->pdeaths );
		do_look( victim, "auto" );

		if( IS_SET( sysdata.save_flags, SV_DEATH ) )
			save_char_obj( victim );
		return;

	}


	/*    int x, y;

		quitting_char = victim;
		save_char_obj( victim );
		saving_char = NULL;
		extract_char( victim, true );
		for ( x = 0; x < MAX_WEAR; x++ )
		for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;
	  }

	  sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
			  capitalize( arg ) );
	  sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
			  capitalize( arg ) );

	  rename( buf, buf2 );

	  sprintf( buf, "%s%c/%s.clone", PLAYER_DIR, tolower(arg[0]),
			  capitalize( arg ) );
	  sprintf( buf2, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
			  capitalize( arg ) );

	  rename( buf, buf2 );

	  return;

	*/
	/* original player kill started here

		extract_char( victim, false );
		if ( !victim )
		{
		  bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
		  return;
		}
		while ( victim->first_affect )
		affect_remove( victim, victim->first_affect );
		victim->affected_by	= race_table[victim->race].affected;
		victim->resistant   = 0;
		victim->susceptible = 0;
		victim->immune      = 0;
		victim->carry_weight= 0;
		victim->armor	= 100;
		victim->mod_str	= 0;
		victim->mod_dex	= 0;
		victim->mod_wis	= 0;
		victim->mod_int	= 0;
		victim->mod_con	= 0;
		victim->mod_cha	= 0;
		victim->mod_lck   	= 0;
		victim->damroll	= 0;
		victim->hitroll	= 0;
		victim->mental_state = -10;
		victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
		victim->saving_spell_staff = 0;
		victim->position	= POS_RESTING;
		victim->hit		= UMAX( 1, victim->hit  );
		victim->move	= UMAX( 1, victim->move );

		victim->pcdata->condition[COND_FULL]   = 12;
		victim->pcdata->condition[COND_THIRST] = 12;

		if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
		save_char_obj( victim );
		return;

	*/

}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;
	CHAR_DATA *lch;
	int xp;
	int members;

	/*
	 * Monsters don't get kill xp's or alignment changes.
	 * Dying of mortal wounds or poison doesn't give xp to anyone!
	 */
	if( IS_NPC( ch ) || victim == ch )
		return;

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
		return;

	if( !IS_NPC( ch ) && !IS_NPC( victim ) && IS_SET( ch->pcdata->flags, PCFLAG_STUN ) && ( ch != victim ) )
		return;

	members = 0;

	for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		if( is_same_group( gch, ch ) )
			members++;
	}

	if( members == 0 )
	{
		bug( "Group_gain: members.", members );
		members = 1;
	}

	lch = ch->leader ? ch->leader : ch;

	for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		if( !is_same_group( gch, ch ) )
			continue;

		xp = ( int ) ( xp_compute( gch, victim ) / members / 2 );

		gch->alignment = align_compute( gch, victim );

		if( !IS_NPC( gch ) && IS_NPC( victim ) && gch->pcdata && gch->pcdata->clan
			&& !str_cmp( gch->pcdata->clan->name, victim->mob_clan ) )
		{
			xp = 0;
			sprintf( buf, "&YYou receive no experience for killing your organizations resources.\r\n" );
			send_to_char( buf, gch );
		}
		else
		{
			sprintf( buf, "&YYou receive %d combat experience.\r\n", xp );
			send_to_char( buf, gch );
		}

		gain_exp( gch, xp, COMBAT_ABILITY );

		if( lch == gch && members > 1 )
		{
			xp =
				URANGE( members, xp * members,
					( exp_level( gch->skill_level[LEADERSHIP_ABILITY] + 1 ) -
						exp_level( gch->skill_level[LEADERSHIP_ABILITY] ) / 20 ) );
			sprintf( buf, "&YYou get %d leadership experience for leading your group to victory.\r\n", xp );
			send_to_char( buf, gch );
			gain_exp( gch, xp, LEADERSHIP_ABILITY );
		}


		for( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if( obj->wear_loc == WEAR_NONE )
				continue;

			if( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_EVIL( ch ) )
				|| ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) && IS_GOOD( ch ) )
				|| ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL( ch ) ) )
			{
				act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
				act( AT_MAGIC, "$n is zapped by $p.", ch, obj, NULL, TO_ROOM );

				obj_from_char( obj );
				obj = obj_to_room( obj, ch->in_room );
				oprog_zap_trigger( ch, obj );   /* mudprogs */
				if( char_died( ch ) )
					return;
			}
		}
	}

	return;
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{

	/* never cared much for this system

		int align, newalign;

		align = gch->alignment - victim->alignment;

		if ( align >  500 )
		newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
		else
		if ( align < -500 )
		newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
		else
		newalign  = gch->alignment - (int) (gch->alignment / 4);

		return newalign;

	make it simple instead */

	return URANGE( -1000, ( int ) ( gch->alignment - victim->alignment / 5 ), 1000 );

}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
	int align;
	int xp;

	xp = ( get_exp_worth( victim )
		* URANGE( 1, ( victim->skill_level[COMBAT_ABILITY] - gch->skill_level[COMBAT_ABILITY] ) + 10, 20 ) ) / 10;
	align = gch->alignment - victim->alignment;

	/*
	 * bonus for attacking opposite alignment
	 */
	if( align > 990 || align < -990 )
		xp = ( xp * 5 ) >> 2;
	else
		/*
		 * penalty for good attacking same alignment
		 */
		if( gch->alignment > 300 && align < 250 )
			xp = ( xp * 3 ) >> 2;

	xp = number_range( ( xp * 1 ) >> 2, ( xp * 5 ) >> 2 );

	/*
	 * reduce exp for killing the same mob repeatedly       -Thoric
	 */
	if( !IS_NPC( gch ) && IS_NPC( victim ) )
	{
		int times = times_killed( gch, victim );

		if( times >= 5 )
			xp = 0;
		else if( times )
			xp = ( xp * ( 5 - times ) ) / 5;
	}

	/*
	 * new xp cap for swreality
	 */

	return URANGE( 1, xp,
		( exp_level( gch->skill_level[COMBAT_ABILITY] + 1 ) - exp_level( gch->skill_level[COMBAT_ABILITY] ) ) );
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	char buf1[256], buf2[256], buf3[256];
	const char *vs;
	const char *vp;
	const char *attack;
	char punct;
	int dampc;
	struct skill_type *skill = NULL;
	bool gcflag = false;
	bool gvflag = false;

	if( !dam )
		dampc = 0;
	else
		dampc = 1000 * dam / victim->max_hit;
	//      dampc = (int)( ( dam * 1000 ) / victim->max_hit ) + (int)( 50 - ( ( victim->hit * 50 ) / victim->max_hit ) );

	   /*
		* 10 * percent
		*/
	if( dam == 45000 )
	{
		vs = "&CC&ca&CN&w'&ct &RD&ra&RM&ra&RG&re&W";
		vp = "&CC&ca&CN&w'&ct &RD&ra&RM&ra&RG&re&W";
	}
	else if( dam == 0 )
	{
		vs = "&Cm&ci&Cs&cs&W";
		vp = "&Cm&ci&Cs&cs&Ce&cs&W";
	}
	else if( dampc <= 5 )
	{
		vs = "&Rn&ri&Rc&rk&W";
		vp = "&Rn&ri&Rc&rk&Rs&W";
	}
	else if( dampc <= 10 )
	{
		vs = "&Yh&Oi&Ot&W";
		vp = "&Yh&Oi&Yt&Os&W";
	}
	else if( dampc <= 20 )
	{
		vs = "&Bi&bn&Bj&bu&Br&be&W";
		vp = "&Bi&bn&Bj&bu&Br&be&Bs&W";
	}
	else if( dampc <= 30 )
	{
		vs = "&Gt&gh&Gr&ga&Gs&gh&W";
		vp = "&Gt&gh&Gr&ga&Gs&gh&Ge&gs&W";
	}
	else if( dampc <= 40 )
	{
		vs = "&zw&Wo&zu&Wn&zd&W";
		vp = "&zw&Wo&zu&Wn&zd&Ws&W";
	}
	else if( dampc <= 60 )
	{
		vs = "&Pm&pa&Pu&pl&W";
		vp = "&Pm&pa&Pu&pl&Ps&W";
	}
	else if( dampc <= 70 )
	{
		vs = "&Bd&be&cc&Ci&Bm&Ca&ct&be&W";
		vp = "&Bd&be&cc&Ci&Bm&Ca&ct&Ce&Bs&W";
	}
	else if( dampc <= 80 )
	{
		vs = "&Yd&Be&Yv&Ba&Ys&Bt&Ya&Bt&Ye&W";
		vp = "&Yd&Be&Yv&Ba&Ys&Bt&Ya&Bt&Ye&Bs&W";
	}
	else if( dampc <= 90 )
	{
		vs = "&PM&Ra&PI&Rm&W";
		vp = "&PM&Ra&PI&Rm&PS&W";
	}
	else if( dampc <= 100 )
	{
		vs = "&RM&rU&OT&YIL&OA&rT&RE&W";
		vp = "&RM&rU&OT&YI&rL&YA&OT&rE&RS&W";
	}
	else if( dampc <= 125 )
	{
		vs = "&P/&p/&P/ &BD&cI&BS&cE&BMB&cO&BW&cE&BL &P/&p/&P/&W";
		vp = "&P/&p/&P/ &BD&cI&BS&cE&BMB&cO&BW&cE&BL&cE&BS &P/&p/&P/&W";
	}
	else if( dampc <= 150 )
	{
		vs = "&Y-&O=&Y<&RM&ra&RS&rs&RA&rc&RR&rE&Y>&O=&Y-&W";
		vp = "&Y-&O=&Y<&RM&ra&RS&rs&RA&rc&RR&rE&RS&Y>&O=&Y-&W";
	}
	else if( dampc <= 175 )
	{
		vs = "&Y* &O* &Y* &BV&ba&BN&bq&BU&bi&BS&bh &Y* &O* &Y*&W";
		vp = "&Y* &O* &Y* &BV&Pa&BN&Pq&BU&Pi&BS&Ph&BE&Ps &Y* &O* &Y*&W";
	}
	else if( dampc <= 200 )
	{
		vs = "&R<&Y|&r<&Y|&R<&Y|&PE&pV&PI&pS&PC&pE&PR&pA&PT&pE&Y|&R>&Y|&r>&Y|&R>&W";
		vp = "&R<&Y|&r<&Y|&R<&Y|&PE&pV&PI&pS&PC&pE&PR&pA&PT&pE&PS&Y|&R>&Y|&r>&Y|&R>&W";
	}
	else if( dampc <= 225 )
	{
		vs = "&C(&b-&C(&b-&C(&b- &YO&Rb&rL&Oi&rT&Re&rR&Oa&rT&Re &b-&C)&b-&C)&b-&C)&W";
		vp = "&C(&b-&C(&b-&C(&b- &YO&Rb&rL&Oi&rT&Re&rR&Oa&rT&Re&YS &b-&C)&b-&C)&b-&C)&W";
	}
	else
	{
		vs = "&P> &R| &p> &RM&re&RL&rt &PT&ph&PR&po&PU&pg&PH &p< &R| &P< &W";
		vp = "&P> &R| &p> &RM&re&RL&rt&RS &PT&ph&PR&po&PU&pg&PH &p< &R| &P<&W";
	}
	if( dam == 45000 )
		dam = 0;

	punct = ( dampc <= 30 ) ? '.' : '!';

	if( dam == 0 && ( !IS_NPC( ch ) && ( IS_SET( ch->pcdata->flags, PCFLAG_GAG ) ) ) )
		gcflag = true;

	if( dam == 0 && ( !IS_NPC( victim ) && ( IS_SET( victim->pcdata->flags, PCFLAG_GAG ) ) ) )
		gvflag = true;

	if( dt >= 0 && dt < top_sn )
		skill = skill_table[dt];

	if( dt == ( TYPE_HIT + WEAPON_BLASTER ) )
	{
		char sound[MAX_STRING_LENGTH];
		int vol = number_range( 20, 80 );

		sprintf( sound, "!!SOUND(blaster V=%d)", vol );
		sound_to_room( ch->in_room, sound );
	}

	if( dt == TYPE_HIT || dam == 0 )
	{
		sprintf( buf1, "$n %s $N%c &b-&B< &C[&c%d&C] &B>&b-", vp, punct, dam );
		sprintf( buf2, "You %s $N%c &b-&B< &C[&c%d&C] &B>&b-", vs, punct, dam );
		sprintf( buf3, "$n %s you%c &b-&B< &C[&c%d&C] &B>&b-", vp, punct, dam );
	}
	else if( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
	{
		// Slightly less than beautiful cast here.
		if( dt < TYPE_HIT + ( int ) ( sizeof( attack_table ) / sizeof( attack_table[0] ) ) )
			attack = attack_table[dt - TYPE_HIT];
		else
		{
			bug( "Dam_message: bad dt %d.", dt );
			dt = TYPE_HIT;
			attack = attack_table[0];
		}

		sprintf( buf1, "$n's poisoned %s %s $N%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
		sprintf( buf2, "Your poisoned %s %s $N%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
		sprintf( buf3, "$n's poisoned %s %s you%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
	}
	else
	{
		if( skill )
		{
			attack = skill->noun_damage;
			if( dam == 0 )
			{
				bool found = false;

				if( skill->miss_char && skill->miss_char[0] != '\0' )
				{
					act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
					found = true;
				}
				if( skill->miss_vict && skill->miss_vict[0] != '\0' )
				{
					act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
					found = true;
				}
				if( skill->miss_room && skill->miss_room[0] != '\0' )
				{
					act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
					found = true;
				}
				if( found ) /* miss message already sent */
					return;
			}
			else
			{
				if( skill->hit_char && skill->hit_char[0] != '\0' )
					act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
				if( skill->hit_vict && skill->hit_vict[0] != '\0' )
					act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
				if( skill->hit_room && skill->hit_room[0] != '\0' )
					act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
			}
		}
		// Not ideal, this cast. But we assume the attack_table never contains
	    // more than roughly 2 billion elements.
		else if( dt >= TYPE_HIT && dt < TYPE_HIT + ( int ) ( sizeof( attack_table ) / sizeof( attack_table[0] ) ) )
			attack = attack_table[dt - TYPE_HIT];
		else
		{
			bug( "Dam_message: bad dt %d.", dt );
			dt = TYPE_HIT;
			attack = attack_table[0];
		}

		sprintf( buf1, "$n's %s %s $N%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
		sprintf( buf2, "Your %s %s $N%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
		sprintf( buf3, "$n's %s %s you%c &b-&B< &C[&c%d&C] &B>&b-", OFFHAND == 1 ? "offhand" : attack, vp, punct, dam );
	}
	act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );
	if( !gcflag )
		act( AT_HIT, buf2, ch, NULL, victim, TO_CHAR );
	if( !gvflag )
		act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );

	return;
}

CMDF( do_voodoo )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *doll;

	if( IS_NPC( ch ) )
		return;

	doll = get_eq_char( ch, WEAR_HOLD );
	if( doll == NULL || ( doll->pIndexData->vnum != OBJ_VNUM_VOODOO ) )
	{
		send_to_char( "You are not holding a voodoo doll.\r\n", ch );
		return;
	}

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: voodoo <action>\r\n", ch );
		send_to_char( "Actions: sexchange pin trip\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "sexchange" ) )
	{
		do_vdsc( ch, doll->name );
		return;
	}

	if( !str_cmp( arg, "burn" ) )
	{
		do_vdbn( ch, doll->name );
		return;
	}

	if( !str_cmp( arg, "pin" ) )
	{
		do_vdpi( ch, doll->name );
		return;
	}

	if( !str_cmp( arg, "trip" ) )
	{
		do_vdtr( ch, doll->name );
		return;
	}

	if( !str_cmp( arg, "throw" ) )
	{
		do_vdth( ch, doll->name );
		return;
	}

	do_voodoo( ch, "" );
}

CMDF( do_vdbn )
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	OBJ_DATA *obj;
	bool found = false;
	int dam;

	argument = one_argument( argument, arg1 );

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_cmp( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( IS_IMMORTAL( wch ) && ( wch->top_level > ch->top_level ) )
			{
				send_to_char( "That's not a good idea.\r\n", ch );
				return;
			}

			if( ( wch->top_level < 20 ) && !IS_IMMORTAL( ch ) )
			{
				send_to_char( "They are a little too young for that.\r\n", ch );
				return;
			}

			if( get_timer( wch, TIMER_PKILLED ) > 0 )
			{
				set_char_color( AT_GREEN, ch );
				send_to_char( "Seriously, it isn't cool to do that.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_PROTECT_VOODOO ) )
			{
				send_to_char( "They are still realing from a previous voodoo.\r\n", ch );
				return;
			}

			found = true;

			send_to_char( "You take a match and set fire to the voodoo doll.\r\n", ch );
			act( AT_MAGIC, "$n takes a match, and sets fire to a voodoo doll.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "The voodoo doll burns away.\r\n", ch );
			act( AT_MAGIC, "$n's voodoo doll burns away.", ch, NULL, NULL, TO_ROOM );

			send_to_char( "&BYour whole body sparks up in &RF&rl&Ra&rM&Re&rS&B!\r\n", wch );
			act( AT_MAGIC, "$n sparks up and bursts into &RF&rl&Ra&rM&Re&rS&B!", wch, NULL, NULL, TO_ROOM );
			af.type = skill_lookup( "protection voodoo" );
			af.duration = 30;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_PROTECT_VOODOO;
			affect_to_char( wch, &af );
			dam = number_range( wch->hit, wch->max_hit ) / 4;
			damage( wch, wch, dam, TYPE_UNDEFINED );
			ch_printf( wch, "&RYou take &P%d&R damage from the flames before they distinguish!\r\n", dam );
			obj = get_eq_char( ch, WEAR_HOLD );
			extract_obj( obj );
			return;
		}
	}
	send_to_char( "Your victim doesn't seem to be in the realm.\r\n", ch );
	return;
}

CMDF( do_vdsc )
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	bool found = false;

	argument = one_argument( argument, arg1 );

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_cmp( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( IS_IMMORTAL( wch ) && ( wch->top_level > ch->top_level ) )
			{
				send_to_char( "That's not a good idea.\r\n", ch );
				return;
			}

			if( ( wch->top_level < 20 ) && !IS_IMMORTAL( ch ) )
			{
				send_to_char( "They are a little too young for that.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_PROTECT_VOODOO ) )
			{
				send_to_char( "They are still realing from a previous voodoo.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_CHANGE_SEX ) )
			{
				send_to_char( "They are already sexchanged! Give em a break, sheesh!\r\n", ch );
				return;
			}

			found = true;

			send_to_char( "&RYou wreak havoc upon the voodoo doll.\r\n", ch );
			act( AT_MAGIC, "$n wreaks havoc upon a voodoo doll.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "&RYou suddenly feel VERY strange...\r\n", wch );
			act( AT_MAGIC, "$n gets a wierd look.. wonder whats wrong...", wch, NULL, NULL, TO_ROOM );
			//            af.type      = sn;
			af.duration = 100;
			af.location = APPLY_SEX;
			do
			{
				af.modifier = number_range( 1, 2 ) - wch->sex;
			} while( af.modifier == 1 );
			af.bitvector = AFF_CHANGE_SEX;
			affect_to_char( wch, &af );
			return;
		}
	}
	send_to_char( "Your victim doesn't seem to be in the realm.\r\n", ch );
	return;
}

CMDF( do_vdpi )
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	bool found = false;

	argument = one_argument( argument, arg1 );

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_cmp( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( IS_IMMORTAL( wch ) && ( wch->top_level > ch->top_level ) )
			{
				send_to_char( "That's not a good idea.\r\n", ch );
				return;
			}

			if( ( wch->top_level < 20 ) && !IS_IMMORTAL( ch ) )
			{
				send_to_char( "They are a little too young for that.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_PROTECT_VOODOO ) )
			{
				send_to_char( "They are still realing from a previous voodoo.\r\n", ch );
				return;
			}

			found = true;

			send_to_char( "You stick a pin into your voodoo doll.\r\n", ch );
			act( AT_MAGIC, "$n sticks a pin into a voodoo doll.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "You double over with a sudden pain in your gut!\r\n", wch );
			act( AT_MAGIC, "$n suddenly doubles over with a look of extreme pain!", wch, NULL, NULL, TO_ROOM );
			af.type = skill_lookup( "protection voodoo" );
			af.duration = 30;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_PROTECT_VOODOO;
			affect_to_char( wch, &af );
			wch->hit -= 100;
			return;
		}
	}
	send_to_char( "Your victim doesn't seem to be in the realm.\r\n", ch );
	return;
}

CMDF( do_vdtr )
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	bool found = false;

	argument = one_argument( argument, arg1 );

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_cmp( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( IS_IMMORTAL( wch ) && ( wch->top_level > ch->top_level ) )
			{
				send_to_char( "That's not a good idea.\r\n", ch );
				return;
			}

			if( ( wch->top_level < 20 ) && !IS_IMMORTAL( ch ) )
			{
				send_to_char( "They are a little too young for that.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_PROTECT_VOODOO ) )
			{
				send_to_char( "They are still realing from a previous voodoo.\r\n", ch );
				return;
			}

			found = true;

			send_to_char( "You slam your voodoo doll against the ground.\r\n", ch );
			act( AT_MAGIC, "$n slams a voodoo doll against the ground.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "Your feet slide out from under you!\r\n", wch );
			send_to_char( "You hit the ground face first!\r\n", wch );
			act( AT_MAGIC, "$n trips over $s own feet, and does a nose dive into the ground!", wch, NULL, NULL, TO_ROOM );
			af.type = skill_lookup( "protection voodoo" );
			af.duration = 30;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_PROTECT_VOODOO;
			affect_to_char( wch, &af );
			wch->position = POS_RESTING;
			return;
		}
	}
	send_to_char( "Your victim doesn't seem to be in the realm.\r\n", ch );
	return;
}

CMDF( do_vdth )
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	bool found = false;
	int attempt;
	//    ROOM_INDEX_DATA *to_room;

	argument = one_argument( argument, arg1 );

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;

		if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
			continue;

		wch = ( d->original != NULL ) ? d->original : d->character;

		if( !can_see( ch, wch ) )
			continue;

		if( !str_cmp( arg1, wch->name ) && !found )
		{
			if( IS_NPC( wch ) )
				continue;

			if( IS_IMMORTAL( wch ) && ( wch->top_level > ch->top_level ) )
			{
				send_to_char( "That's not a good idea.\r\n", ch );
				return;
			}

			if( ( wch->top_level < 20 ) && !IS_IMMORTAL( ch ) )
			{
				send_to_char( "They are a little too young for that.\r\n", ch );
				return;
			}

			if( IS_AFFECTED( wch, AFF_PROTECT_VOODOO ) )
			{
				send_to_char( "They are still realing from a previous voodoo.\r\n", ch );
				return;
			}

			found = true;

			send_to_char( "You toss your voodoo doll into the air.\r\n", ch );
			act( AT_MAGIC, "$n tosses a voodoo doll into the air.", ch, NULL, NULL, TO_ROOM );
			af.type = skill_lookup( "protection voodoo" );
			af.duration = 30;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_PROTECT_VOODOO;
			affect_to_char( wch, &af );

			if( ( wch->fighting != NULL ) || ( number_percent( ) < 25 ) )
			{
				send_to_char( "A sudden gust of wind throws you through the air!\r\n", wch );
				send_to_char( "You slam face first into the nearest wall!\r\n", wch );
				act( AT_MAGIC, "A sudden gust of wind picks up $n and throws $m into a wall!", wch, NULL, NULL, TO_ROOM );
				return;
			}

			wch->position = POS_STANDING;
			was_in = wch->in_room;
			for( attempt = 0; attempt < 8; attempt++ )
			{
				EXIT_DATA *pexit;
				int door;
				door = number_door( );
				if( ( pexit = get_exit( was_in, door ) ) == NULL || !pexit->to_room || ( IS_SET( pexit->exit_info, EX_CLOSED ) )
					//  &&   !IS_AFFECTED( wch, AFF_PASS_DOOR ) )
					|| ( IS_NPC( wch ) && xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) ) )
					continue;

				move_char( wch, pexit, 0 );
				if( ( now_in = wch->in_room ) == was_in )
					continue;

				wch->in_room = was_in;
				sprintf( buf, "A sudden gust of wind picks up $n and throws $m to the %s.", dir_name[door] );
				act( AT_MAGIC, buf, wch, NULL, NULL, TO_ROOM );
				send_to_char( "A sudden gust of wind throws you through the air!\r\n", wch );
				wch->in_room = now_in;
				act( AT_MAGIC, "$n sails into the room and slams face first into a wall!", wch, NULL, NULL, TO_ROOM );
				do_look( wch, "auto" );
				send_to_char( "You slam face first into the nearest wall!\r\n", wch );
				return;
			}
			send_to_char( "A sudden gust of wind throws you through the air!\r\n", wch );
			send_to_char( "You slam face first into the nearest wall!\r\n", wch );
			act( AT_MAGIC, "A sudden gust of wind picks up $n and throws $m into a wall!", wch, NULL, NULL, TO_ROOM );
			return;
		}
	}
	send_to_char( "Your victim doesn't seem to be in the realm.\r\n", ch );
	return;
}

CMDF( do_kill )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Kill whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) )
	{
		if( !IS_SET( victim->pcdata->flags, PCFLAG_TWIT ) )
		{
			send_to_char( "You must MURDER a player.\r\n", ch );
			return;
		}
	}

	/*
	 *
	 else
	 {
	 if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	 {
	 send_to_char( "You must murder a charmed creature.\r\n", ch );
	 return;
	 }
	 }
	 *
	 */

	if( victim == ch )
	{
		send_to_char( "You hit yourself.  Ouch!\r\n", ch );
		multi_hit( ch, ch, TYPE_UNDEFINED );
		return;
	}

	if( is_safe( ch, victim ) )
		return;

	if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
	{
		act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You do the best you can!\r\n", ch );
		return;
	}

	if( victim->vip_flags != 0 )
		ch->alignment = URANGE( -1000, ch->alignment - 10, 1000 );

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
}



CMDF( do_murde )
{
	send_to_char( "If you want to MURDER, spell it out.\r\n", ch );
	return;
}



CMDF( do_murder )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wield;

	one_argument( argument, arg );

	if( !xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) && !xIS_SET( ch->act, PLR_PKER ) )
	{
		send_to_char( "If you want to MURDER, go PK.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Murder whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Suicide is a mortal sin.\r\n", ch );
		return;
	}

	if( ( !IS_NPC( victim ) && xIS_SET( victim->in_room->room_flags, ROOM_ARENA ) ) && is_safe( ch, victim ) )
		return;

	if( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		if( ch->master == victim )
		{
			act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
			return;
		}
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You do the best you can!\r\n", ch );
		return;
	}

	if( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
	{
		send_to_char( "You feel too nice to do that!\r\n", ch );
		return;
	}
	if( xIS_SET( victim->act, PLR_AFK ) )
	{
		sprintf( log_buf, "%s just attacked %s with an afk flag on!.", ch->name, victim->name );
		log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
	}
	ch->alignment -= 10;

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

	wield = get_eq_char( ch, WEAR_WIELD );
	if( wield )
	{
		if( wield->pIndexData->vnum == 5126 )
		{
			if( !xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
			{
				if( ( victim->position == POS_INCAP )
					|| ( victim->position == POS_MORTAL ) || ( victim->position == POS_STUNNED ) )
				{
					ch_printf( ch, "You draw out Tenseiga's hidden power!\n%s's near dead body is filled with life!\r\n",
						victim->name );
					ch_printf( victim, "Your body is filled with life!\nYou blink twice to see %s with Tenseiga.\r\n", ch->name );
					act( AT_ACTION,
						"$n's &RT&re&On&Yse&Oi&rg&Ra&w begins to glow. $N's body is filled with life after being hit with the powerful blade.",
						ch, NULL, victim, TO_NOTVICT );
					victim->hit = victim->max_hit;
					victim->position = POS_RESTING;
					return;
				}
			}
		}
	}
	if( !xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		if( IS_NPC( ch ) )
		{
			sprintf( buf, "&RHelp&r!&R!&r!&C %s is attacking me!!", ch->short_descr );
			do_chat( victim, buf );
		}
		else
		{
			sprintf( buf, "&RHELP&r!&R!&r!&C %s is attacking me!!", ch->name );
			do_chat( victim, buf );
			if( !IS_NPC( victim ) )
			{
				sprintf( log_buf, "%s: murder %s.", ch->name, victim->name );
				log_string( log_buf );
				to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_STAFF );
			}

		}

	}


	add_timer( ch, TIMER_RECENTFIGHT, 50, NULL, 0 );
	add_timer( victim, TIMER_RECENTFIGHT, 50, NULL, 0 );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return;
}

bool in_arena( CHAR_DATA *ch )
{

	if( !str_cmp( ch->in_room->area->filename, "arena.are" ) )
		return true;

	if( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 )
		return false;

	return true;
}


CMDF( do_flee )
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	char buf[MAX_STRING_LENGTH];
	int attempt;
	short door;
	EXIT_DATA *pexit;

	if( !who_fighting( ch ) )
	{
		if( ch->position == POS_FIGHTING )
		{
			if( ch->mount )
				ch->position = POS_MOUNTED;
			else
				ch->position = POS_STANDING;
		}
		send_to_char( "You aren't fighting anyone.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "&RNot while in the arena!\r\n", ch );
		return;
	}

	if( ch->move <= 0 )
	{
		send_to_char( "You're too exhausted to flee from combat!\r\n", ch );
		return;
	}

	/*
	 * No fleeing while stunned. - Narn
	 */
	if( ch->position < POS_FIGHTING )
		return;

	was_in = ch->in_room;
	for( attempt = 0; attempt < 8; attempt++ )
	{

		door = number_door( );
		if( ( pexit = get_exit( was_in, door ) ) == NULL || !pexit->to_room || ( IS_SET( pexit->exit_info, EX_CLOSED ) )
			//  &&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
			|| ( IS_NPC( ch ) && xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) ) )
			continue;

		affect_strip( ch, gsn_sneak );
		REMOVE_BIT( ch->affected_by, AFF_SNEAK );
		if( ch->mount && ch->mount->fighting )
			stop_fighting( ch->mount, true );
		move_char( ch, pexit, 0 );
		if( ( now_in = ch->in_room ) == was_in )
			continue;

		ch->in_room = was_in;
		act( AT_FLEE, "$n runs for cover!", ch, NULL, NULL, TO_ROOM );
		ch->in_room = now_in;
		act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );
		sprintf( buf, "You run for cover!" );
		send_to_char( buf, ch );

		stop_fighting( ch, true );
		return;
	}

	sprintf( buf, "You attempt to run for cover!\r\n" );
	send_to_char( buf, ch );
	return;
}

bool get_cover( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	int attempt;
	short door;
	EXIT_DATA *pexit;

	if( !who_fighting( ch ) )
		return false;

	if( ch->position < POS_FIGHTING )
		return false;

	was_in = ch->in_room;
	for( attempt = 0; attempt < 10; attempt++ )
	{

		door = number_door( );
		if( ( pexit = get_exit( was_in, door ) ) == NULL || !pexit->to_room || ( IS_SET( pexit->exit_info, EX_CLOSED ) )
			//  &&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
			|| ( IS_NPC( ch ) && xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) ) )
			continue;

		affect_strip( ch, gsn_sneak );
		REMOVE_BIT( ch->affected_by, AFF_SNEAK );
		if( ch->mount && ch->mount->fighting )
			stop_fighting( ch->mount, true );
		move_char( ch, pexit, 0 );
		if( ( now_in = ch->in_room ) == was_in )
			continue;

		ch->in_room = was_in;
		act( AT_FLEE, "$n sprints for cover!", ch, NULL, NULL, TO_ROOM );
		ch->in_room = now_in;
		act( AT_FLEE, "$n spins around and takes aim.", ch, NULL, NULL, TO_ROOM );

		stop_fighting( ch, true );

		return true;
	}

	return false;
}



CMDF( do_sla )
{
	send_to_char( "If you want to SLAY, spell it out.\r\n", ch );
	return;
}



CMDF( do_slay )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );
	one_argument( argument, arg2 );
	if( arg[0] == '\0' )
	{
		send_to_char( "Slay whom?\r\n", ch );
		return;
	}
	/*
	   if( !str_cmp( arg, "Gilthanas" ) )
	   {
		  send_to_char( "Nice Try!", ch );
		  return;
	   }
	*/
	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( ch == victim )
	{
		send_to_char( "Suicide is a mortal sin.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "immolate" ) )
	{
		act( AT_FIRE, "Your fireball turns $N into a blazing inferno.", ch, NULL, victim, TO_CHAR );
		act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT );
		act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.", ch, NULL, victim, TO_NOTVICT );
	}

	else if( !str_cmp( arg2, "shatter" ) )
	{
		act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.", ch, NULL, victim,
			TO_CHAR );
		act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim,
			TO_VICT );
		act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.", ch, NULL, victim,
			TO_NOTVICT );
	}

	else if( !str_cmp( arg2, "demon" ) )
	{
		act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim, TO_CHAR );
		act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
			TO_CHAR );
		act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on", ch, NULL, victim, TO_VICT );
		act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.", ch, NULL, victim,
			TO_VICT );
		act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim,
			TO_NOTVICT );
		act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
			TO_NOTVICT );
	}

	else if( !str_cmp( arg2, "transform" ) )
	{
		act( AT_IMMORT,
			"&zYou &Gt&gr&ca&Cn&Bs&Cf&co&gr&Gm&z into a &CW&ce&CR&ce&RT&ri&RG&re&RR &zin the &Yl&Oi&Yg&Oh&Yt &zof the &PF&pu&Pl&pl &wM&Wo&wo&Wn&z. You fly at",
			ch, NULL, victim, TO_CHAR );
		act( AT_IMMORT,
			"&W$N&z, and &Rt&re&Ra&rr &zevery &Gl&gi&Gm&gb&z from their &Yb&Oo&Yd&Oy&z before &RM&ra&pu&Pl&pi&rn&Rg&z them to &Bd&be&ca&bt&Bh&z.",
			ch, NULL, victim, TO_CHAR );
		act( AT_IMMORT,
			"&z$n &Gt&gr&ca&Cn&Bs&Cf&co&gr&Gm&z into a &CW&ce&CR&ce&RT&ri&RG&re&RR &zfrom the &Yl&Oi&Yg&Oh&Yt &zof the &PF&pu&Pl&pl &wM&Wo&wo&Wn&z. He flies",
			ch, NULL, victim, TO_VICT );
		act( AT_IMMORT,
			"at you, &Rt&re&Ra&rr&Ri&rn&Rg &zevery &Gl&gi&Gm&gb&z from your &Yb&Oo&Yd&Oy&z before &RM&ra&pu&Pl&pi&rn&Rg&z you to &Bd&be&ca&bt&Bh&z.",
			ch, NULL, victim, TO_VICT );
		act( AT_IMMORT,
			"&z$n &Gt&gr&ca&Cn&Bs&Cf&co&gr&Gm&z into a &CW&ce&CR&ce&RT&ri&RG&re&RR &zfrom the &Yl&Oi&Yg&Oh&Yt &zof the &PF&pu&Pl&pl &wM&Wo&wo&Wn&z. He flies",
			ch, NULL, victim, TO_NOTVICT );
		act( AT_IMMORT,
			"at &W$N&z, &Rt&re&Ra&rr&Ri&rn&Rg &zevery &Gl&gi&Gm&gb&z from their &Yb&Oo&Yd&Oy&z before& RM&ra&pu&Pl&pi&rn&Rg&z them to &Bd&be&ca&bt&Bh&z.",
			ch, NULL, victim, TO_NOTVICT );

	}

	else if( !str_cmp( arg2, "pounce" ) && get_trust( ch ) >= LEVEL_LIAISON )
	{
		act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...", ch,
			NULL, victim, TO_CHAR );
		act( AT_BLOOD,
			"In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",
			ch, NULL, victim, TO_VICT );
		act( AT_BLOOD,
			"Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",
			ch, NULL, victim, TO_NOTVICT );
	}

	else if( !str_cmp( arg2, "slit" ) && get_trust( ch ) >= LEVEL_LIAISON )
	{
		act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
		act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
		act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
	}

	else
	{
		act( AT_IMMORT, "You slay $N in cold blood!", ch, NULL, victim, TO_CHAR );
		act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT );
		act( AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT );
	}

	set_cur_char( victim );
	raw_kill( ch, victim );
	return;
}
