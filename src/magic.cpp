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
 * Local functions.
 */
void say_spell( CHAR_DATA *ch, int sn );
CHAR_DATA *make_poly_mob( CHAR_DATA *ch, int vnum );
SPELLF( spell_affect );
SPELLF( spell_affectchar );

/*
 * Is immune to a damage type
 */
bool is_immune( CHAR_DATA *ch, short damtype )
{
	switch( damtype )
	{
	case SD_FIRE:           return( IS_SET( ch->immune, RIS_FIRE ) );
	case SD_COLD:           return( IS_SET( ch->immune, RIS_COLD ) );
	case SD_ELECTRICITY:    return( IS_SET( ch->immune, RIS_ELECTRICITY ) );
	case SD_ENERGY:         return( IS_SET( ch->immune, RIS_ENERGY ) );
	case SD_ACID:           return( IS_SET( ch->immune, RIS_ACID ) );
	case SD_POISON:         return( IS_SET( ch->immune, RIS_POISON ) );
	case SD_DRAIN:          return( IS_SET( ch->immune, RIS_DRAIN ) );
	}
	return false;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup( CHAR_DATA *ch, const char *name )
{
	int sn;

	if( IS_NPC( ch ) )
		return skill_lookup( name );
	for( sn = 0; sn < top_sn; sn++ )
	{
		if( !skill_table[sn]->name )
			break;
		if( ch->pcdata->learned[sn] > 0
			&& LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
			return sn;
	}

	return -1;
}

/*
 * Lookup an herb by name.
 */
int herb_lookup( const char *name )
{
	int sn;

	for( sn = 0; sn < top_herb; sn++ )
	{
		if( !herb_table[sn] || !herb_table[sn]->name )
			return -1;
		if( LOWER( name[0] ) == LOWER( herb_table[sn]->name[0] ) && !str_prefix( name, herb_table[sn]->name ) )
			return sn;
	}
	return -1;
}

/*
 * Lookup a personal skill
 */
int personal_lookup( CHAR_DATA *ch, const char *name )
{
	return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
	int sn;

	if( ( sn = bsearch_skill( name, gsn_first_spell, gsn_first_skill - 1 ) ) == -1 )
		if( ( sn = bsearch_skill( name, gsn_first_skill, gsn_first_weapon - 1 ) ) == -1 )
			if( ( sn = bsearch_skill( name, gsn_first_weapon, gsn_first_tongue - 1 ) ) == -1 )
				if( ( sn = bsearch_skill( name, gsn_first_tongue, gsn_top_sn - 1 ) ) == -1 && gsn_top_sn < top_sn )
				{
					for( sn = gsn_top_sn; sn < top_sn; sn++ )
					{
						if( !skill_table[sn] || !skill_table[sn]->name )
							return -1;
						if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
							return sn;
					}
					return -1;
				}
	return sn;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SKILLTYPE *get_skilltype( int sn )
{
	if( sn >= TYPE_PERSONAL )
		return NULL;
	if( sn >= TYPE_HERB )
		return IS_VALID_HERB( sn - TYPE_HERB ) ? herb_table[sn - TYPE_HERB] : NULL;
	if( sn >= TYPE_HIT )
		return NULL;
	return IS_VALID_SN( sn ) ? skill_table[sn] : NULL;
}


/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 */
int bsearch_skill( const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;
		if( !IS_VALID_SN( sn ) )
			return -1;
		if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
			return sn;
		if( first >= top )
			return -1;
		if( strcasecmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for prefix matches
 */
int bsearch_skill_prefix( const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;

		if( sn < 0 )
			return -1;

		if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
			return sn;
		if( first >= top )
			return -1;
		if( strcmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
	return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 * Check for exact matches only
 */
int bsearch_skill_exact( const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;
		if( !IS_VALID_SN( sn ) )
			return -1;
		if( !strcasecmp( name, skill_table[sn]->name ) )
			return sn;
		if( first >= top )
			return -1;
		if( strcasecmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;

		if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
			&& !str_prefix( name, skill_table[sn]->name ) && ch->pcdata->learned[sn] > 0 )
			return sn;
		if( first >= top )
			return -1;
		if( strcmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
	return -1;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill_prefix( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;

		if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
			&& !str_prefix( name, skill_table[sn]->name ) && ch->pcdata->learned[sn] > 0 )
			return sn;
		if( first >= top )
			return -1;
		if( strcmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
	return -1;
}

int ch_bsearch_skill_exact( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	for( ;; )
	{
		sn = ( first + top ) >> 1;

		if( !str_cmp( name, skill_table[sn]->name ) && ch->pcdata->learned[sn] > 0 )
			return sn;
		if( first >= top )
			return -1;
		if( strcmp( name, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}
	return -1;
}


int find_spell( CHAR_DATA *ch, const char *name, bool know )
{
	if( IS_NPC( ch ) || !know )
		return bsearch_skill( name, gsn_first_spell, gsn_first_skill - 1 );
	else
		return ch_bsearch_skill( ch, name, gsn_first_spell, gsn_first_skill - 1 );
}

int find_skill( CHAR_DATA *ch, const char *name, bool know )
{
	if( IS_NPC( ch ) || !know )
		return bsearch_skill( name, gsn_first_skill, gsn_first_weapon - 1 );
	else
		return ch_bsearch_skill( ch, name, gsn_first_skill, gsn_first_weapon - 1 );
}

int find_weapon( CHAR_DATA *ch, const char *name, bool know )
{
	if( IS_NPC( ch ) || !know )
		return bsearch_skill( name, gsn_first_weapon, gsn_first_tongue - 1 );
	else
		return ch_bsearch_skill( ch, name, gsn_first_weapon, gsn_first_tongue - 1 );
}

int find_tongue( CHAR_DATA *ch, const char *name, bool know )
{
	if( IS_NPC( ch ) || !know )
		return bsearch_skill( name, gsn_first_tongue, gsn_top_sn - 1 );
	else
		return ch_bsearch_skill( ch, name, gsn_first_tongue, gsn_top_sn - 1 );
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
	extern bool fBootDb;
	int sn;

	if( slot <= 0 )
		return -1;

	for( sn = 0; sn < top_sn; sn++ )
		if( slot == skill_table[sn]->slot )
			return sn;

	if( fBootDb )
	{
		bug( "Slot_lookup: bad slot %d.", slot );
		abort( );
	}

	return -1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting( SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
	short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
	short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );

	if( skill->target != TAR_CHAR_OFFENSIVE )
	{
		chit = chitroom;
		chitme = chitroom;
	}

	if( ch && ch != victim )
	{
		if( skill->hit_char && skill->hit_char[0] != '\0' )
			act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
		else if( skill->type == SKILL_SPELL )
			act( chit, "Ok.", ch, NULL, NULL, TO_CHAR );
	}
	if( ch && skill->hit_room && skill->hit_room[0] != '\0' )
		act( chitroom, skill->hit_room, ch, obj, victim, TO_NOTVICT );
	if( ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0' )
	{
		if( ch != victim )
			act( chitme, skill->hit_vict, ch, obj, victim, TO_VICT );
		else
			act( chitme, skill->hit_vict, ch, obj, victim, TO_CHAR );
	}
	else if( ch && ch == victim && skill->type == SKILL_SPELL )
		act( chitme, "Ok.", ch, NULL, NULL, TO_CHAR );
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting( SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
	short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
	short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );

	if( skill->target != TAR_CHAR_OFFENSIVE )
	{
		chit = chitroom;
		chitme = chitroom;
	}

	if( ch && ch != victim )
	{
		if( skill->miss_char && skill->miss_char[0] != '\0' )
			act( chit, skill->miss_char, ch, obj, victim, TO_CHAR );
		else if( skill->type == SKILL_SPELL )
			act( chit, "You failed.", ch, NULL, NULL, TO_CHAR );
	}
	if( ch && skill->miss_room && skill->miss_room[0] != '\0' )
		act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
	if( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
	{
		if( ch != victim )
			act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
		else
			act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
	else if( ch && ch == victim )
	{
		if( skill->miss_char && skill->miss_char[0] != '\0' )
			act( chitme, skill->miss_char, ch, obj, victim, TO_CHAR );
		else if( skill->type == SKILL_SPELL )
			act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
	}
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting( SKILLTYPE *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
	short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
	short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );

	if( skill->target != TAR_CHAR_OFFENSIVE )
	{
		chit = chitroom;
		chitme = chitroom;
	}

	if( ch && ch != victim )
	{
		if( skill->imm_char && skill->imm_char[0] != '\0' )
			act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
		else if( skill->miss_char && skill->miss_char[0] != '\0' )
			act( chit, skill->miss_char, ch, obj, victim, TO_CHAR );
		else if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
			act( chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR );
	}
	if( ch && skill->imm_room && skill->imm_room[0] != '\0' )
		act( chitroom, skill->imm_room, ch, obj, victim, TO_NOTVICT );
	else if( ch && skill->miss_room && skill->miss_room[0] != '\0' )
		act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
	if( ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0' )
	{
		if( ch != victim )
			act( chitme, skill->imm_vict, ch, obj, victim, TO_VICT );
		else
			act( chitme, skill->imm_vict, ch, obj, victim, TO_CHAR );
	}
	else if( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
	{
		if( ch != victim )
			act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
		else
			act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
	else if( ch && ch == victim )
	{
		if( skill->imm_char && skill->imm_char[0] != '\0' )
			act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
		else if( skill->miss_char && skill->miss_char[0] != '\0' )
			act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
		else if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
			act( chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR );
	}
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
	CHAR_DATA *rch;

	for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
	{
		if( rch != ch )
			act( AT_MAGIC, "$n pauses and concentrates for a moment.", ch, NULL, rch, TO_VICT );
	}

	return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save( CHAR_DATA *ch, int chance, int ris )
{
	short modifier;

	modifier = 10;
	if( IS_SET( ch->immune, ris ) )
		modifier -= 10;
	if( IS_SET( ch->resistant, ris ) )
		modifier -= 2;
	if( IS_SET( ch->susceptible, ris ) )
		modifier += 2;
	if( modifier <= 0 )
		return 1000;
	if( modifier == 10 )
		return chance;
	return ( chance * modifier ) / 10;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = luck, A = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
int rd_parse( CHAR_DATA *ch, int level, char *texp )
{
	size_t x = 0, len = 0, total = 0;
	int lop = 0, gop = 0, eop = 0;
	char operation;
	char *sexp[2];

	/*
	 * take care of nulls coming in
	 */
	if( !texp || !strlen( texp ) )
		return 0;

	/*
	 * get rid of brackets if they surround the entire expresion
	 */
	if( ( *texp == '(' ) && texp[strlen( texp ) - 1] == ')' )
	{
		texp[strlen( texp ) - 1] = '\0';
		texp++;
	}

	/*
	 * check if the expresion is just a number
	 */
	len = strlen( texp );
	if( len == 1 && isalpha( texp[0] ) )
		switch( texp[0] )
		{
		case 'L':
		case 'l':
			return level;
		case 'H':
		case 'h':
			return ch->hit;
		case 'V':
		case 'v':
			return ch->move;
		case 'S':
		case 's':
			return get_curr_str( ch );
		case 'I':
		case 'i':
			return get_curr_int( ch );
		case 'W':
		case 'w':
			return get_curr_wis( ch );
		case 'X':
		case 'x':
			return get_curr_dex( ch );
		case 'C':
		case 'c':
			return get_curr_con( ch );
		case 'A':
		case 'a':
			return get_curr_cha( ch );
		case 'U':
		case 'u':
			return get_curr_lck( ch );
		case 'Y':
		case 'y':
			return get_age( ch );
		}

	for( x = 0; x < len; ++x )
		if( !isdigit( texp[x] ) && !isspace( texp[x] ) )
			break;
	if( x == len )
		return ( atoi( texp ) );

	/*
	 * break it into 2 parts
	 */
	for( x = 0; x < strlen( texp ); ++x )
		switch( texp[x] )
		{
		case '^':
			if( !total )
				eop = x;
			break;
		case '-':
		case '+':
			if( !total )
				lop = x;
			break;
		case '*':
		case '/':
		case '%':
		case 'd':
		case 'D':
			if( !total )
				gop = x;
			break;
		case '(':
			++total;
			break;
		case ')':
			--total;
			break;
		}
	if( lop )
		x = lop;
	else if( gop )
		x = gop;
	else
		x = eop;
	operation = texp[x];
	texp[x] = '\0';
	sexp[0] = texp;
	sexp[1] = ( char * ) ( texp + x + 1 );

	/*
	 * work it out
	 */
	total = rd_parse( ch, level, sexp[0] );
	switch( operation )
	{
	case '-':
		total -= rd_parse( ch, level, sexp[1] );
		break;
	case '+':
		total += rd_parse( ch, level, sexp[1] );
		break;
	case '*':
		total *= rd_parse( ch, level, sexp[1] );
		break;
	case '/':
		total /= rd_parse( ch, level, sexp[1] );
		break;
	case '%':
		total %= rd_parse( ch, level, sexp[1] );
		break;
	case 'd':
	case 'D':
		total = dice( total, rd_parse( ch, level, sexp[1] ) );
		break;
	case '^':
	{
		size_t y = rd_parse( ch, level, sexp[1] ), z = total;

		for( x = 1; x < y; ++x, z *= total );
		total = z;
		break;
	}
	}
	return total;
}

/* wrapper function so as not to destroy exp */
int dice_parse( CHAR_DATA *ch, int level, const char *texp )
{
	char buf[MAX_INPUT_LENGTH];

	mudstrlcpy( buf, texp, MAX_INPUT_LENGTH );
	return rd_parse( ch, level, buf );
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death( int level, CHAR_DATA *victim )
{
	int save;

	save = 50 + ( victim->top_level - level - victim->saving_poison_death ) * 2;
	save = URANGE( 5, save, 95 );
	return chance( victim, save );
}

bool saves_wands( int level, CHAR_DATA *victim )
{
	int save;

	if( IS_SET( victim->immune, RIS_MAGIC ) )
		return true;

	save = 50 + ( victim->top_level - level - victim->saving_wand ) * 2;
	save = URANGE( 5, save, 95 );
	return chance( victim, save );
}

bool saves_para_petri( int level, CHAR_DATA *victim )
{
	int save;

	save = 50 + ( victim->top_level - level - victim->saving_para_petri ) * 2;
	save = URANGE( 5, save, 95 );
	return chance( victim, save );
}

bool saves_breath( int level, CHAR_DATA *victim )
{
	int save;

	save = 50 + ( victim->top_level - level - victim->saving_breath ) * 2;
	save = URANGE( 5, save, 95 );
	return chance( victim, save );
}

bool saves_spell_staff( int level, CHAR_DATA *victim )
{
	int save;

	if( IS_SET( victim->immune, RIS_MAGIC ) )
		return true;

	if( IS_NPC( victim ) && level > 10 )
		level -= 5;
	save = 50 + ( victim->top_level - level - victim->saving_spell_staff ) * 2;
	save = URANGE( 5, save, 95 );
	return chance( victim, save );
}


/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components( CHAR_DATA *ch, int sn )
{
	SKILLTYPE *skill = get_skilltype( sn );
	const char *comp = skill->components;
	const char *check;
	char arg[MAX_INPUT_LENGTH];
	bool consume, fail, found;
	int val, value;
	OBJ_DATA *obj;

	/*
	 * if no components necessary, then everything is cool
	 */
	if( !comp || comp[0] == '\0' )
		return true;

	/* disable the whole damn shabang */

	return true;

	while( comp[0] != '\0' )
	{
		comp = one_argument( comp, arg );
		consume = true;
		fail = found = false;
		val = -1;
		switch( arg[1] )
		{
		default:
			check = arg + 1;
			break;
		case '!':
			check = arg + 2;
			fail = true;
			break;
		case '+':
			check = arg + 2;
			consume = false;
			break;
		case '@':
			check = arg + 2;
			val = 0;
			break;
		case '#':
			check = arg + 2;
			val = 1;
			break;
		case '$':
			check = arg + 2;
			val = 2;
			break;
		case '%':
			check = arg + 2;
			val = 3;
			break;
		case '^':
			check = arg + 2;
			val = 4;
			break;
		case '&':
			check = arg + 2;
			val = 5;
			break;
		}
		value = atoi( check );
		obj = NULL;
		switch( UPPER( arg[0] ) )
		{
		case 'T':
			for( obj = ch->first_carrying; obj; obj = obj->next_content )
				if( obj->item_type == value )
				{
					if( fail )
					{
						send_to_char( "Something disrupts the use of this power...\r\n", ch );
						return false;
					}
					found = true;
					break;
				}
			break;
		case 'V':
			for( obj = ch->first_carrying; obj; obj = obj->next_content )
				if( obj->pIndexData->vnum == value )
				{
					if( fail )
					{
						send_to_char( "Something disrupts the use of this power...\r\n", ch );
						return false;
					}
					found = true;
					break;
				}
			break;
		case 'K':
			for( obj = ch->first_carrying; obj; obj = obj->next_content )
				if( nifty_is_name( check, obj->name ) )
				{
					if( fail )
					{
						send_to_char( "Something disrupts the use of this power...\r\n", ch );
						return false;
					}
					found = true;
					break;
				}
			break;
		case 'G':
			if( ch->gold >= value )
			{
				if( fail )
				{
					send_to_char( "Something disrupts the use of this power...\r\n", ch );
					return false;
				}
				else
				{
					if( consume )
					{
						set_char_color( AT_GOLD, ch );
						send_to_char( "You feel a little lighter...\r\n", ch );
						ch->gold -= value;
					}
					continue;
				}
			}
			break;
		case 'H':
			if( ch->hit >= value )
			{
				if( fail )
				{
					send_to_char( "Something disrupts the use of this power...\r\n", ch );
					return false;
				}
				else
				{
					if( consume )
					{
						set_char_color( AT_BLOOD, ch );
						send_to_char( "You feel a little weaker...\r\n", ch );
						ch->hit -= value;
						update_pos( ch );
					}
					continue;
				}
			}
			break;
		}
		/*
		 * having this component would make the spell fail... if we get
		 * here, then the caster didn't have that component
		 */
		if( fail )
			continue;
		if( !found )
		{
			send_to_char( "Something is missing...\r\n", ch );
			return false;
		}
		if( obj )
		{
			if( val >= 0 && val < 6 )
			{
				separate_obj( obj );
				if( obj->value[val] <= 0 )
					return false;
				else if( --obj->value[val] == 0 )
				{
					act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
					act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
					extract_obj( obj );
				}
				else
					act( AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR );
			}
			else if( consume )
			{
				separate_obj( obj );
				act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
				act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
				extract_obj( obj );
			}
			else
			{
				int count = obj->count;

				obj->count = 1;
				act( AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR );
				obj->count = count;
			}
		}
	}
	return true;
}

int pAbort;

/*
 * Locate targets.
 */
void *locate_targets( CHAR_DATA *ch, char *arg, int sn, CHAR_DATA **victim, OBJ_DATA **obj )
{
	SKILLTYPE *skill = get_skilltype( sn );
	void *vo = NULL;

	*victim = NULL;
	*obj = NULL;

	switch( skill->target )
	{
	default:
		bug( "Do_cast: bad target for sn %d.", sn );
		return &pAbort;

	case TAR_IGNORE:
		break;

	case TAR_CHAR_OFFENSIVE:
		if( arg[0] == '\0' )
		{
			if( ( *victim = who_fighting( ch ) ) == NULL )
			{
				send_to_char( "Cast the spell on whom?\r\n", ch );
				return &pAbort;
			}
		}
		else
		{
			if( ( *victim = get_char_room( ch, arg ) ) == NULL )
			{
				send_to_char( "They aren't here.\r\n", ch );
				return &pAbort;
			}
		}

		if( is_safe( ch, *victim ) )
			return &pAbort;

		if( ch == *victim )
		{
			send_to_char( "Cast this on yourself?  Okay...\r\n", ch );
			/*
			 * send_to_char( "You can't do that to yourself.\r\n", ch );
			 * return &pAbort;
			 */
		}

		if( !IS_NPC( ch ) )
		{
			if( !IS_NPC( *victim ) )
			{
				/*
				 * Sheesh! can't do anything
				 * send_to_char( "You can't do that on a player.\r\n", ch );
				 * return &pAbort;
				 */

				if( get_timer( ch, TIMER_PKILLED ) > 0 )
				{
					send_to_char( "You have been killed in the last 5 minutes.\r\n", ch );
					return &pAbort;
				}

				if( get_timer( *victim, TIMER_PKILLED ) > 0 )
				{
					send_to_char( "This player has been killed in the last 5 minutes.\r\n", ch );
					return &pAbort;
				}

			}

			if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == *victim )
			{
				send_to_char( "You can't do that on your own follower.\r\n", ch );
				return &pAbort;
			}
		}

		vo = ( void * ) *victim;
		break;

	case TAR_CHAR_DEFENSIVE:
		if( arg[0] == '\0' )
			*victim = ch;
		else
		{
			if( ( *victim = get_char_room( ch, arg ) ) == NULL )
			{
				send_to_char( "They aren't here.\r\n", ch );
				return &pAbort;
			}
		}
		vo = ( void * ) *victim;
		break;

	case TAR_CHAR_SELF:
		if( arg[0] != '\0' && !nifty_is_name( arg, ch->name ) )
		{
			send_to_char( "You cannot cast this spell on another.\r\n", ch );
			return &pAbort;
		}

		vo = ( void * ) ch;
		break;

	case TAR_OBJ_INV:
		if( arg[0] == '\0' )
		{
			send_to_char( "What should the spell be cast upon?\r\n", ch );
			return &pAbort;
		}

		if( ( *obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			send_to_char( "You are not carrying that.\r\n", ch );
			return &pAbort;
		}

		vo = ( void * ) *obj;
		break;
	}

	return vo;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
const char *target_name;

/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	void *vo;
	ch_ret retcode = rNONE;
	int levdiff = ch->top_level - level;
	SKILLTYPE *skill = get_skilltype( sn );
	struct timeval time_used;

	if( sn == -1 )
		return retcode;
	if( !skill || !skill->spell_fun )
	{
		bug( "%s: bad sn %d.", __func__, sn );
		return rERROR;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "Nothing seems to happen...\r\n", ch );
		return rNONE;
	}

	/*
	 * Basically this was added to cut down on level 5 players using level
	 * 40 scrolls in battle too often ;)     -Thoric
	 */
	if( ( skill->target == TAR_CHAR_OFFENSIVE || number_bits( 7 ) == 1 ) /* 1/128 schance if non-offensive */
		&& skill->type != SKILL_HERB && !chance( ch, 95 + levdiff ) )
	{
		switch( number_bits( 2 ) )
		{
		case 0:
			failed_casting( skill, ch, victim, NULL );
			break;
		case 1:
			act( AT_MAGIC, "The $t backfires!", ch, skill->name, victim, TO_CHAR );
			if( victim )
				act( AT_MAGIC, "$n's $t backfires!", ch, skill->name, victim, TO_VICT );
			act( AT_MAGIC, "$n's $t backfires!", ch, skill->name, victim, TO_NOTVICT );
			return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
		case 2:
			failed_casting( skill, ch, victim, NULL );
			break;
		case 3:
			act( AT_MAGIC, "The $t backfires!", ch, skill->name, victim, TO_CHAR );
			if( victim )
				act( AT_MAGIC, "$n's $t backfires!", ch, skill->name, victim, TO_VICT );
			act( AT_MAGIC, "$n's $t backfires!", ch, skill->name, victim, TO_NOTVICT );
			return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
		}
		return rNONE;
	}

	target_name = "";
	switch( skill->target )
	{
	default:
		bug( "%s: bad target for sn %d.", __func__, sn );
		return rERROR;

	case TAR_IGNORE:
		vo = NULL;
		if( victim )
			target_name = victim->name;
		else if( obj )
			target_name = obj->name;
		break;

	case TAR_CHAR_OFFENSIVE:
		if( victim != ch )
		{
			if( !victim )
				victim = who_fighting( ch );
			if( !victim || !IS_NPC( victim ) )
			{
				send_to_char( "You can't do that.\r\n", ch );
				return rNONE;
			}
		}
		if( ch != victim && is_safe( ch, victim ) )
			return rNONE;
		vo = ( void * ) victim;
		break;

	case TAR_CHAR_DEFENSIVE:
		if( victim == NULL )
			victim = ch;
		vo = ( void * ) victim;
		if( skill->type != SKILL_HERB && IS_SET( victim->immune, RIS_MAGIC ) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rNONE;
		}
		break;

	case TAR_CHAR_SELF:
		vo = ( void * ) ch;
		if( skill->type != SKILL_HERB && IS_SET( ch->immune, RIS_MAGIC ) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rNONE;
		}
		break;

	case TAR_OBJ_INV:
		if( obj == NULL )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return rNONE;
		}
		vo = ( void * ) obj;
		break;
	}

	start_timer( &time_used );
	retcode = ( *skill->spell_fun ) ( sn, level, ch, vo );
	end_timer( &time_used );
	update_userec( &time_used, &skill->userec );

	if( retcode == rSPELL_FAILED )
		retcode = rNONE;

	if( retcode == rCHAR_DIED || retcode == rERROR )
		return retcode;

	if( char_died( ch ) )
		return rCHAR_DIED;

	if( skill->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died( victim ) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for( vch = ch->in_room->first_person; vch; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if( victim == vch && !victim->fighting && victim->master != ch )
			{
				retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
				break;
			}
		}
	}

	return retcode;
}

SPELLF( spell_charm_person )
{
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	AFFECT_DATA af;
	int chance;
	char buf[MAX_STRING_LENGTH];
	SKILLTYPE *skill = get_skilltype( sn );

	if( victim == ch )
	{
		send_to_char( "You like yourself even better!\r\n", ch );
		return rSPELL_FAILED;
	}

	if( !IS_NPC( ch ) && !IS_NPC( victim ) )
	{

		if( !xIS_SET( ch->act, PLR_PKER ) )
		{
			send_to_char( "You can't do that. You are peaceful.\r\n", ch );
			return rSPELL_FAILED;
		}

		if( !xIS_SET( victim->act, PLR_PKER ) )
		{
			send_to_char( "You can't do that. They are peaceful.\r\n", ch );
			return rSPELL_FAILED;
		}
	}

	if( IS_SET( victim->immune, RIS_MAGIC ) || IS_SET( victim->immune, RIS_CHARM ) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if( !IS_NPC( victim ) && !IS_NPC( ch ) )
	{
		send_to_char( "I don't think so...\r\n", ch );
		return rSPELL_FAILED;
	}

	chance = ris_save( victim, level, RIS_CHARM );

	if( IS_AFFECTED( victim, AFF_CHARM )
		|| chance == 10000
		|| IS_AFFECTED( ch, AFF_CHARM ) || circle_follow( victim, ch ) || saves_spell_staff( chance, victim ) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if( victim->master )
		stop_follower( victim );
	add_follower( victim, ch );
	af.type = sn;
	af.duration = ( number_fuzzy( ( level + 1 ) / 3 ) + 1 ) * DUR_CONV;
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char( victim, &af );
	act( AT_MAGIC, "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
	act( AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM );
	if( ch != victim )
		send_to_char( "Ok.\r\n", ch );

	snprintf( buf, MAX_STRING_LENGTH, "%s has charmed %s.", ch->name, victim->name );
	//   log_string_plus( buf, LOG_NORMAL, ch->top_level );
	return rNONE;
}

SPELLF( spell_fireball )
{
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	static const short dam_each[] = {
	   1,
	   1, 4, 7, 10, 13, 16, 19, 22, 25, 28,
	   31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
	   44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
	   51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
	   58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
	   64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
	   70, 71, 71, 72, 72, 73, 73, 74, 75, 75
	};
	int dam;

	level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
	level = UMAX( 0, level );
	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if( saves_spell_staff( level, victim ) )
		dam /= 2;
	return damage( ch, victim, dam, sn );
}

SPELLF( spell_lightning_bolt )
{
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	static const short dam_each[] = {
	   1,
	   2, 4, 6, 8, 10, 12, 14, 16, 18, 20,
	   22, 24, 26, 28, 30, 35, 40, 45, 50, 55,
	   60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
	   92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
	   112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
	   132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
	   152, 154, 156, 158, 160, 162, 164, 166, 168, 170
	};

	int dam;

	level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
	level = UMAX( 0, level );
	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if( saves_spell_staff( level, victim ) )
		dam /= 2;
	return damage( ch, victim, dam, sn );
}

/*
 * A spell as it should be				-Thoric
 */
SPELLF( spell_word_of_recall )
{
	do_recall( ( CHAR_DATA * ) vo, "" );
	return rNONE;
}

SPELLF( spell_null )
{
	send_to_char( "That's not a spell!\r\n", ch );
	return rNONE;
}

/* don't remove, may look redundant, but is important */
SPELLF( spell_notfound )
{
	send_to_char( "That's not a spell!\r\n", ch );
	return rNONE;
}

CHAR_DATA *make_poly_mob( CHAR_DATA *ch, int vnum )
{
	CHAR_DATA *mob;
	MOB_INDEX_DATA *pMobIndex;

	if( !ch )
	{
		bug( "Make_poly_mob: null ch!", 0 );
		return NULL;
	}

	if( vnum < 10 || vnum > 16 )
	{
		bug( "Make_poly_mob: Vnum not in polymorphing mobs range", 0 );
		return NULL;
	}

	if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
	{
		bug( "Make_poly_mob: Can't find mob %d", vnum );
		return NULL;
	}
	mob = create_mobile( pMobIndex );
	xSET_BIT( mob->act, ACT_POLYMORPHED );
	return mob;
}

CMDF( do_revert )
{

	CHAR_DATA *mob;

	if( !IS_NPC( ch ) || !xIS_SET( ch->act, ACT_POLYMORPHED ) )
	{
		send_to_char( "You are not polymorphed.\r\n", ch );
		return;
	}

	xREMOVE_BIT( ch->act, ACT_POLYMORPHED );

	char_from_room( ch->desc->original );

	if( ch->desc->character )
	{
		mob = ch->desc->character;
		char_to_room( ch->desc->original, ch->desc->character->in_room ); /*WORKS!! */
		ch->desc->character = ch->desc->original;
		ch->desc->original = NULL;
		ch->desc->character->desc = ch->desc;
		ch->desc->character->switched = NULL;
		ch->desc = NULL;
		extract_char( mob, true );
		return;
	}

	ch->desc->character = ch->desc->original;
	ch->desc->original = NULL;
	ch->desc->character->desc = ch->desc;
	ch->desc->character->switched = NULL;
	ch->desc = NULL;
	return;
}

/*******************************************************
 * Everything after this point is part of SMAUG SPELLS *
 *******************************************************/

 /*
  * saving throw check						-Thoric
  */
bool check_save( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
	SKILLTYPE *skill = get_skilltype( sn );
	bool saved = false;

	if( SPELL_FLAG( skill, SF_PKSENSITIVE ) && !IS_NPC( ch ) && !IS_NPC( victim ) )
		level /= 2;

	if( skill->saves )
		switch( skill->saves )
		{
		case SS_POISON_DEATH:
			saved = saves_poison_death( level, victim );
			break;
		case SS_ROD_WANDS:
			saved = saves_wands( level, victim );
			break;
		case SS_PARA_PETRI:
			saved = saves_para_petri( level, victim );
			break;
		case SS_BREATH:
			saved = saves_breath( level, victim );
			break;
		case SS_SPELL_STAFF:
			saved = saves_spell_staff( level, victim );
			break;
		}
	return saved;
}

/*
 * Generic offensive spell damage attack			-Thoric
 */
SPELLF( spell_attack )
{
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	SKILLTYPE *skill = get_skilltype( sn );
	bool saved = check_save( sn, level, ch, victim );
	int dam;
	ch_ret retcode;

	send_to_char( "You feel the hatred grow within you!\r\n", ch );
	ch->alignment = ch->alignment - 100;
	ch->alignment = URANGE( -1000, ch->alignment, 1000 );
	sith_penalty( ch );

	if( saved && !SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}
	if( skill->dice )
		dam = UMAX( 0, dice_parse( ch, level, skill->dice ) );
	else
		dam = dice( 1, level );
	if( saved )
		dam /= 2;
	retcode = damage( ch, victim, dam, sn );
	if( retcode == rNONE && skill->first_affect && !char_died( ch ) && !char_died( victim ) )
		retcode = spell_affectchar( sn, level, ch, victim );
	return retcode;
}

/*
 * Generic area attack						-Thoric
 */
SPELLF( spell_area_attack )
{
	CHAR_DATA *vch, *vch_next;
	SKILLTYPE *skill = get_skilltype( sn );
	bool saved;
	bool affects;
	int dam;
	ch_ret retcode = rNONE;

	send_to_char( "You feel the hatred grow within you!\r\n", ch );
	ch->alignment = ch->alignment - 100;
	ch->alignment = URANGE( -1000, ch->alignment, 1000 );
	sith_penalty( ch );

	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	affects = ( skill->first_affect ? true : false );
	if( skill->hit_char && skill->hit_char[0] != '\0' )
		act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
	if( skill->hit_room && skill->hit_room[0] != '\0' )
		act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );

	for( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) && vch->pcdata->wizinvis >= LEVEL_STAFF )
			continue;

		if( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) ) )
		{
			saved = check_save( sn, level, ch, vch );
			if( saved && !SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
			{
				failed_casting( skill, ch, vch, NULL );
				dam = 0;
			}
			else if( skill->dice )
				dam = dice_parse( ch, level, skill->dice );
			else
				dam = dice( 1, level );
			if( saved && SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
				dam /= 2;
			retcode = damage( ch, vch, dam, sn );
		}
		if( retcode == rNONE && affects && !char_died( ch ) && !char_died( vch ) )
			retcode = spell_affectchar( sn, level, ch, vch );
		if( retcode == rCHAR_DIED || char_died( ch ) )
		{
			break;
		}
	}
	return retcode;
}

SPELLF( spell_affectchar )
{
	AFFECT_DATA af;
	SMAUG_AFF *saf;
	SKILLTYPE *skill = get_skilltype( sn );
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	int chance;
	ch_ret retcode = rNONE;

	if( SPELL_FLAG( skill, SF_RECASTABLE ) )
		affect_strip( victim, sn );
	for( saf = skill->first_affect; saf; saf = saf->next )
	{
		if( saf->location >= REVERSE_APPLY )
			victim = ch;
		else
			victim = ( CHAR_DATA * ) vo;
		/*
		 * Check if char has this bitvector already
		 */
		if( ( af.bitvector = saf->bitvector ) != 0
			&& IS_AFFECTED( victim, af.bitvector ) && !SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
			continue;
		/*
		 * necessary for affect_strip to work properly...
		 */
		switch( af.bitvector )
		{
		default:
			af.type = sn;
			break;
		case AFF_POISON:
			af.type = gsn_poison;

			ch->alignment = ch->alignment - 100;
			ch->alignment = URANGE( -1000, ch->alignment, 1000 );

			chance = ris_save( victim, level, RIS_POISON );
			if( chance == 1000 )
			{
				retcode = rVICT_IMMUNE;
				if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
					return retcode;
				continue;
			}
			if( saves_poison_death( chance, victim ) )
			{
				if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
					return retcode;
				continue;
			}
			victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
			break;
		case AFF_BLIND:
			af.type = gsn_blindness;
			break;
		case AFF_INVISIBLE:
			af.type = gsn_invis;
			break;
		case AFF_SLEEP:
			af.type = gsn_sleep;
			chance = ris_save( victim, level, RIS_SLEEP );
			if( chance == 1000 )
			{
				retcode = rVICT_IMMUNE;
				if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
					return retcode;
				continue;
			}
			break;
		case AFF_CHARM:
			af.type = gsn_charm_person;
			chance = ris_save( victim, level, RIS_CHARM );
			if( chance == 1000 )
			{
				retcode = rVICT_IMMUNE;
				if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
					return retcode;
				continue;
			}
			break;
		case AFF_POSSESS:
			af.type = gsn_possess;
			break;
		}
		af.duration = dice_parse( ch, ( level / 14 ), saf->duration );
		af.modifier = dice_parse( ch, level, saf->modifier );
		af.location = saf->location % REVERSE_APPLY;
		if( af.duration == 0 )
		{

			switch( af.location )
			{
			case APPLY_HIT:
				if( ch != victim && victim->hit < victim->max_hit && af.modifier > 0 )
				{
					ch->alignment = ch->alignment + 20;
					ch->alignment = URANGE( -1000, ch->alignment, 1000 );
				}
				if( af.modifier > 0 && victim->hit >= victim->max_hit )
				{
					return rSPELL_FAILED;
				}
				victim->hit = URANGE( 0, victim->hit + af.modifier, victim->max_hit );
				update_pos( victim );
				break;
			case APPLY_MANA:
				break;
			case APPLY_MOVE:
				if( af.modifier > 0 && victim->move >= victim->max_move )
				{
					return rSPELL_FAILED;
				}
				victim->move = URANGE( 0, victim->move + af.modifier, victim->max_move );
				update_pos( victim );
				break;
			default:
				affect_modify( victim, &af, true );
				break;
			}
		}
		else if( SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
			affect_join( victim, &af );
		else
			affect_to_char( victim, &af );
	}
	update_pos( victim );
	return retcode;
}

/*
 * Generic spell affect						-Thoric
 */
SPELLF( spell_affect )
{
	SMAUG_AFF *saf;
	SKILLTYPE *skill = get_skilltype( sn );
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	bool groupsp;
	bool areasp;
	bool hitchar, hitroom, hitvict = false;
	ch_ret retcode;

	if( !skill->first_affect )
	{
		bug( "spell_affect has no affects sn %d", sn );
		return rNONE;
	}
	if( SPELL_FLAG( skill, SF_GROUPSPELL ) )
		groupsp = true;
	else
		groupsp = false;

	if( SPELL_FLAG( skill, SF_AREA ) )
		areasp = true;
	else
		areasp = false;
	if( !groupsp && !areasp )
	{
		/*
		 * Can't find a victim
		 */
		if( !victim )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if( ( skill->type != SKILL_HERB
			&& IS_SET( victim->immune, RIS_MAGIC ) ) || is_immune( victim, SPELL_DAMAGE( skill ) ) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		/*
		 * Spell is already on this guy
		 */
		if( is_affected( victim, sn ) && !SPELL_FLAG( skill, SF_ACCUMULATIVE ) && !SPELL_FLAG( skill, SF_RECASTABLE ) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if( ( saf = skill->first_affect ) && !saf->next
			&& saf->location == APPLY_STRIPSN && !is_affected( victim, dice_parse( ch, level, saf->modifier ) ) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if( check_save( sn, level, ch, victim ) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}
	}
	else
	{
		if( skill->hit_char && skill->hit_char[0] != '\0' )
		{
			if( strstr( skill->hit_char, "$N" ) )
				hitchar = true;
			else
				act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
		}
		if( skill->hit_room && skill->hit_room[0] != '\0' )
		{
			if( strstr( skill->hit_room, "$N" ) )
				hitroom = true;
			else
				act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
		}
		if( skill->hit_vict && skill->hit_vict[0] != '\0' )
			hitvict = true;
		if( victim )
			victim = victim->in_room->first_person;
		else
			victim = ch->in_room->first_person;
	}
	if( !victim )
	{
		bug( "spell_affect: could not find victim: sn %d", sn );
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	for( ; victim; victim = victim->next_in_room )
	{
		if( groupsp || areasp )
		{
			if( ( groupsp && !is_same_group( victim, ch ) )
				|| IS_SET( victim->immune, RIS_MAGIC )
				|| is_immune( victim, SPELL_DAMAGE( skill ) )
				|| check_save( sn, level, ch, victim ) || ( !SPELL_FLAG( skill, SF_RECASTABLE ) && is_affected( victim, sn ) ) )
				continue;

			if( hitvict && ch != victim )
			{
				act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
				if( hitroom )
				{
					act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
					act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR );
				}
			}
			else if( hitroom )
				act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
			if( ch == victim )
			{
				if( hitvict )
					act( AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR );
				else if( hitchar )
					act( AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR );
			}
			else if( hitchar )
				act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
		}
		retcode = spell_affectchar( sn, level, ch, victim );
		if( !groupsp && !areasp )
		{
			if( retcode == rSPELL_FAILED )
			{
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}
			if( retcode == rVICT_IMMUNE )
				immune_casting( skill, ch, victim, NULL );
			else
				successful_casting( skill, ch, victim, NULL );
			break;
		}
	}
	return rNONE;
}

/*
 * Generic inventory object spell				-Thoric
 */
SPELLF( spell_obj_inv )
{
	OBJ_DATA *obj = ( OBJ_DATA * ) vo;
	SKILLTYPE *skill = get_skilltype( sn );

	if( !obj )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}

	switch( SPELL_ACTION( skill ) )
	{
	default:
	case SA_NONE:
		return rNONE;

	case SA_CREATE:
		if( SPELL_FLAG( skill, SF_WATER ) )    /* create water */
		{
			int water;

			if( obj->item_type != ITEM_DRINK_CON )
			{
				send_to_char( "It is unable to hold water.\r\n", ch );
				return rSPELL_FAILED;
			}

			if( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
			{
				send_to_char( "It contains some other liquid.\r\n", ch );
				return rSPELL_FAILED;
			}

			water = UMIN( ( skill->dice ? dice_parse( ch, level, skill->dice ) : level )
				* ( weather_info.sky >= SKY_RAINING ? 2 : 1 ), obj->value[0] - obj->value[1] );

			if( water > 0 )
			{
				separate_obj( obj );
				obj->value[2] = LIQ_WATER;
				obj->value[1] += water;
				if( !is_name( "water", obj->name ) )
				{
					char buf[MAX_STRING_LENGTH];

					snprintf( buf, MAX_STRING_LENGTH, "%s water", obj->name );
					STRFREE( obj->name );
					obj->name = STRALLOC( buf );
				}
			}
			successful_casting( skill, ch, NULL, obj );
			return rNONE;
		}
		if( SPELL_DAMAGE( skill ) == SD_FIRE ) /* burn object */
		{
			/*
			 * return rNONE;
			 */
		}
		if( SPELL_DAMAGE( skill ) == SD_POISON /* poison object */
			|| SPELL_CLASS( skill ) == SC_DEATH )
		{
			switch( obj->item_type )
			{
			default:
				failed_casting( skill, ch, NULL, obj );
				break;
			case ITEM_FOOD:
			case ITEM_DRINK_CON:
				separate_obj( obj );
				obj->value[3] = 1;
				successful_casting( skill, ch, NULL, obj );
				break;
			}
			return rNONE;
		}
		if( SPELL_CLASS( skill ) == SC_LIFE    /* purify food/water */
			&& ( obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON ) )
		{
			switch( obj->item_type )
			{
			default:
				failed_casting( skill, ch, NULL, obj );
				break;
			case ITEM_FOOD:
			case ITEM_DRINK_CON:
				separate_obj( obj );
				obj->value[3] = 0;
				successful_casting( skill, ch, NULL, obj );
				break;
			}
			return rNONE;
		}

		if( SPELL_CLASS( skill ) != SC_NONE )
		{
			failed_casting( skill, ch, NULL, obj );
			return rNONE;
		}
		switch( SPELL_POWER( skill ) )    /* clone object */
		{
			OBJ_DATA *clone;

		default:
		case SP_NONE:
			if( obj->cost > ch->skill_level[COMBAT_ABILITY] * get_curr_int( ch ) * get_curr_wis( ch ) )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;
		case SP_MINOR:
			if( ch->skill_level[COMBAT_ABILITY] - obj->level < 20
				|| obj->cost > ch->skill_level[COMBAT_ABILITY] * get_curr_int( ch ) / 5 )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;
		case SP_GREATER:
			if( ch->skill_level[COMBAT_ABILITY] - obj->level < 5
				|| obj->cost > ch->skill_level[COMBAT_ABILITY] * 10 * get_curr_int( ch ) * get_curr_wis( ch ) )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;
		case SP_MAJOR:
			if( ch->skill_level[COMBAT_ABILITY] - obj->level < 0
				|| obj->cost > ch->skill_level[COMBAT_ABILITY] * 50 * get_curr_int( ch ) * get_curr_wis( ch ) )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			clone = clone_object( obj );
			clone->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;
			obj_to_char( clone, ch );
			successful_casting( skill, ch, NULL, obj );
			break;
		}
		return rNONE;

	case SA_DESTROY:
	case SA_RESIST:
	case SA_SUSCEPT:
	case SA_DIVINATE:
		if( SPELL_DAMAGE( skill ) == SD_POISON )   /* detect poison */
		{
			if( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
			{
				if( obj->value[3] != 0 )
					send_to_char( "You smell poisonous fumes.\r\n", ch );
				else
					send_to_char( "It looks very delicious.\r\n", ch );
			}
			else
				send_to_char( "It doesn't look poisoned.\r\n", ch );
			return rNONE;
		}
		return rNONE;
	case SA_OBSCURE: /* make obj invis */
		if( IS_OBJ_STAT( obj, ITEM_INVIS ) || chance( ch, skill->dice ? dice_parse( ch, level, skill->dice ) : 20 ) )
		{
			failed_casting( skill, ch, NULL, NULL );
			return rSPELL_FAILED;
		}
		successful_casting( skill, ch, NULL, obj );
		xSET_BIT( obj->extra_flags, ITEM_INVIS );
		return rNONE;

	case SA_CHANGE:
		return rNONE;
	}
	return rNONE;
}

/*
 * Generic object creating spell				-Thoric
 */
SPELLF( spell_create_obj )
{
	SKILLTYPE *skill = get_skilltype( sn );
	int lvl;
	int vnum = skill->value;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *oi;

	switch( SPELL_POWER( skill ) )
	{
	default:
	case SP_NONE:
		lvl = 10;
		break;
	case SP_MINOR:
		lvl = 0;
		break;
	case SP_GREATER:
		lvl = level / 2;
		break;
	case SP_MAJOR:
		lvl = level;
		break;
	}

	/*
	 * Add predetermined objects here
	 */
	if( vnum == 0 )
	{
		if( !str_cmp( target_name, "sword" ) )
			vnum = OBJ_VNUM_SCHOOL_SWORD;
		if( !str_cmp( target_name, "shield" ) )
			vnum = OBJ_VNUM_SCHOOL_SHIELD;
	}

	if( ( oi = get_obj_index( vnum ) ) == NULL || ( obj = create_object( oi, lvl ) ) == NULL )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}
	obj->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;
	successful_casting( skill, ch, NULL, obj );
	if( CAN_WEAR( obj, ITEM_TAKE ) )
		obj_to_char( obj, ch );
	else
		obj_to_room( obj, ch->in_room );
	return rNONE;
}

/*
 * Generic mob creating spell					-Thoric
 */
SPELLF( spell_create_mob )
{
	SKILLTYPE *skill = get_skilltype( sn );
	int lvl;
	int vnum = skill->value;
	CHAR_DATA *mob;
	MOB_INDEX_DATA *mi;
	AFFECT_DATA af;

	/*
	 * set maximum mob level
	 */
	switch( SPELL_POWER( skill ) )
	{
	default:
	case SP_NONE:
		lvl = 20;
		break;
	case SP_MINOR:
		lvl = 5;
		break;
	case SP_GREATER:
		lvl = level / 2;
		break;
	case SP_MAJOR:
		lvl = level;
		break;
	}

	/*
	 * Add predetermined mobiles here
	 */
	if( vnum == 0 )
	{
		return rNONE;
	}

	if( ( mi = get_mob_index( vnum ) ) == NULL || ( mob = create_mobile( mi ) ) == NULL )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}
	mob->top_level = UMIN( lvl, skill->dice ? dice_parse( ch, level, skill->dice ) : mob->top_level );
	mob->armor = interpolate( mob->top_level, 100, -100 );

	mob->max_hit = mob->top_level * 8 + number_range( mob->top_level * mob->top_level / 4, mob->top_level * mob->top_level );
	mob->hit = mob->max_hit;
	mob->gold = 0;
	successful_casting( skill, ch, mob, NULL );
	char_to_room( mob, ch->in_room );
	add_follower( mob, ch );
	af.type = sn;
	af.duration = ( number_fuzzy( ( level + 1 ) / 3 ) + 1 ) * DUR_CONV;
	af.location = 0;
	af.modifier = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char( mob, &af );
	return rNONE;
}

/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */
SPELLF( spell_smaug )
{
	struct skill_type *skill = get_skilltype( sn );

	/*
	 * Put this check in to prevent crashes from this getting a bad skill
	 */
	if( !skill )
	{
		bug( "spell_smaug: Called with a null skill for sn %d", sn );
		return rERROR;
	}

	switch( skill->target )
	{
	case TAR_IGNORE:

		/*
		 * offensive area spell
		 */
		if( SPELL_FLAG( skill, SF_AREA )
			&& ( ( SPELL_ACTION( skill ) == SA_DESTROY
				&& SPELL_CLASS( skill ) == SC_LIFE )
				|| ( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) ) )
			return spell_area_attack( sn, level, ch, vo );

		if( SPELL_ACTION( skill ) == SA_CREATE )
		{
			if( SPELL_FLAG( skill, SF_OBJECT ) )    /* create object */
				return spell_create_obj( sn, level, ch, vo );
			if( SPELL_CLASS( skill ) == SC_LIFE )   /* create mob */
				return spell_create_mob( sn, level, ch, vo );
		}

		/*
		 * affect a distant player
		 */
		if( SPELL_FLAG( skill, SF_DISTANT ) && SPELL_FLAG( skill, SF_CHARACTER ) )
			return spell_affect( sn, level, ch, get_char_world( ch, target_name ) );

		/*
		 * affect a player in this room (should have been TAR_CHAR_XXX)
		 */
		if( SPELL_FLAG( skill, SF_CHARACTER ) )
			return spell_affect( sn, level, ch, get_char_room( ch, target_name ) );

		/*
		 * will fail, or be an area/group affect
		 */
		return spell_affect( sn, level, ch, vo );

	case TAR_CHAR_OFFENSIVE:
		/*
		 * a regular damage inflicting spell attack
		 */
		if( ( SPELL_ACTION( skill ) == SA_DESTROY
			&& SPELL_CLASS( skill ) == SC_LIFE )
			|| ( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) )
			return spell_attack( sn, level, ch, vo );

		/*
		 * a nasty spell affect
		 */
		return spell_affect( sn, level, ch, vo );

	case TAR_CHAR_DEFENSIVE:

	case TAR_CHAR_SELF:
		if( vo && SPELL_ACTION( skill ) == SA_DESTROY )
		{
			CHAR_DATA *victim = ( CHAR_DATA * ) vo;

			/*
			 * cure poison
			 */
			if( SPELL_DAMAGE( skill ) == SD_POISON )
			{
				if( is_affected( victim, gsn_poison ) )
				{
					affect_strip( victim, gsn_poison );
					victim->mental_state = URANGE( -100, victim->mental_state, -10 );
					successful_casting( skill, ch, victim, NULL );
					return rNONE;
				}
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}
			/*
			 * cure blindness
			 */
			if( SPELL_CLASS( skill ) == SC_ILLUSION )
			{
				if( is_affected( victim, gsn_blindness ) )
				{
					affect_strip( victim, gsn_blindness );
					successful_casting( skill, ch, victim, NULL );
					return rNONE;
				}
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}
		}
		return spell_affect( sn, level, ch, vo );

	case TAR_OBJ_INV:
		return spell_obj_inv( sn, level, ch, vo );
	}
	return rNONE;
}

void spell_protection_voodoo( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	CHAR_DATA *victim = ( CHAR_DATA * ) vo;
	AFFECT_DATA af;

	if( IS_AFFECTED( victim, AFF_PROTECT_VOODOO ) )
	{
		return;
	}
	af.type = sn;
	af.duration = 5;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_PROTECT_VOODOO;
	affect_to_char( victim, &af );
	return;
}
