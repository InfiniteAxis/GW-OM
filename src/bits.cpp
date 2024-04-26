/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2002 Alsherok. Contributors: Samson, Dwip, Whir,   *
 * Cyberfox, Karangi, Rathian, Cam, Raine, and Tarl.                        *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************/

 /* bits.c -- Abits and Qbits for the Rogue Winds by Scion
	Copyright 2000 by Peter Keeler, All Rights Reserved. The content
	of this file may be used by anyone for any purpose so long as this
	original header remains entirely intact and credit is given to the
	original author(s).

	The concept for this was inspired by Mallory's mob scripting system
	from AntaresMUD.

	It is not required, but I'd appreciate hearing back from people
	who use this code. What are you using it for, what have you done
	to it, ideas, comments, etc. So while it's not necessary, I'd love
	to get a note from you at keeler@teleport.com. Thanks! -- Scion
 */

#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "bits.h"

/* QBITS save, ABITS do not save. There are enough of each to give a range
   of them to builders the same as their vnums. They are identifiable by mobs
   running mob progs, and can be used as little identifiers for players..
   player X has done this and this and this.. think of them like a huge array
   of boolean variables you can put on a player or mob with a mob prog. -- Scion
*/

BIT_DATA *first_abit;
BIT_DATA *first_qbit;
BIT_DATA *last_abit;
BIT_DATA *last_qbit;

/* Find an abit in the mud's listing */
BIT_DATA *find_abit( int number )
{
	BIT_DATA *bit;

	for( bit = first_abit; bit; bit = bit->next )
	{
		if( bit->number == number )
			return bit;
	}
	return NULL;
}

/* Find a qbit in the mud's listing */
BIT_DATA *find_qbit( int number )
{
	BIT_DATA *bit;

	for( bit = first_qbit; bit; bit = bit->next )
	{
		if( bit->number == number )
			return bit;
	}
	return NULL;
}

/* Find an abit on a character and return it */
BIT_DATA *get_abit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit = NULL;

	for( bit = ch->first_abit; bit; bit = bit->next )
	{
		if( bit->number == number )
			return bit;
	}
	return NULL;
}

/* Find a qbit on a character and return it */
BIT_DATA *get_qbit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit = NULL;

	if( IS_NPC( ch ) )
		return NULL;

	for( bit = ch->pcdata->firstqbit; bit; bit = bit->next )
	{
		if( bit->number == number )
			return bit;
	}
	return NULL;
}

/* Write out the abit and qbit files */
void save_bits( void )
{
	BIT_DATA *start_bit;
	BIT_DATA *bit;
	FILE *fpout;
	char filename[MAX_INPUT_LENGTH];
	int mode = 0;

	/*
	 * Print 2 files
	 */
	for( mode = 0; mode < 2; mode++ )
	{
		if( mode == 0 )
		{
			snprintf( filename, MAX_INPUT_LENGTH, "%sabit.lst", SYSTEM_DIR );
			start_bit = first_abit;
		}
		else
		{
			snprintf( filename, MAX_INPUT_LENGTH, "%sqbit.lst", SYSTEM_DIR );
			start_bit = first_qbit;
		}

		if( ( fpout = FileOpen( filename, "w" ) ) == NULL )
		{
			bug( "Cannot open bit list %d for writing", mode );
			return;
		}

		for( bit = start_bit; bit; bit = bit->next )
		{
			fprintf( fpout, "Number	%d\n", bit->number );
			fprintf( fpout, "Desc	%s~\n", bit->desc );
			fprintf( fpout, "%s", "End\n\n" );
		}
		fprintf( fpout, "%s", "#END\n" );
		FileClose( fpout );
	}
}

/* Load the abits and qbits */
void load_bits( void )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	int mode = 0;
	BIT_DATA *bit = NULL;
	FILE *fp;

	snprintf( buf, MAX_STRING_LENGTH, "%sabit.lst", SYSTEM_DIR );
	if( ( fp = FileOpen( buf, "r" ) ) == NULL )
	{
		perror( buf );
		return;
	}

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

		case '#':
			if( !str_cmp( word, "#END" ) )
			{
				FileClose( fp );
				if( mode == 0 )
				{
					mode = 1; /* We have two files to read, I reused the same code to read both */
					snprintf( buf, MAX_STRING_LENGTH, "%sqbit.lst", SYSTEM_DIR );
					if( ( fp = FileOpen( buf, "r" ) ) == NULL )
					{
						perror( buf );
						return;
					}
				}
				else
					return;
				fMatch = true;
			}
			break;

		case 'D':
			if( !str_cmp( word, "Desc" ) )
			{
				fMatch = true;
				mudstrlcpy( bit->desc, fread_string( fp ), MSL );
			}
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( mode == 0 )
					LINK( bit, first_abit, last_abit, next, prev );
				else
					LINK( bit, first_qbit, last_qbit, next, prev );
				bit = NULL;
				fMatch = true;
			}
			break;

		case 'N':
			if( !str_cmp( word, "Number" ) )
			{
				CREATE( bit, BIT_DATA, 1 );
				bit->number = fread_number( fp );
				fMatch = true;
			}
			break;
		}
		if( !fMatch )
			bug( "load_bits: no match: %s", word );
	}
}

/* Add an abit to a character */
void set_abit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit;
	BIT_DATA *proto_bit;

	if( number < 0 || number > MAX_xBITS )
		return;

	for( proto_bit = first_abit; proto_bit; proto_bit = proto_bit->next )
	{
		if( proto_bit->number == number )
			break;
	}

	if( proto_bit == NULL )
		return;

	if( ( bit = get_abit( ch, number ) ) == NULL )
	{
		CREATE( bit, BIT_DATA, 1 );

		bit->number = proto_bit->number;
		mudstrlcpy( bit->desc, proto_bit->desc, MSL );
		LINK( bit, ch->first_abit, ch->last_abit, next, prev );
	}
}

/* Add a qbit to a character */
void set_qbit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit;
	BIT_DATA *proto_bit;

	if( IS_NPC( ch ) )
		return;

	if( number < 0 || number > MAX_xBITS )
		return;

	for( proto_bit = first_qbit; proto_bit; proto_bit = proto_bit->next )
	{
		if( proto_bit->number == number )
			break;
	}

	if( proto_bit == NULL )
		return;

	if( ( bit = get_qbit( ch, number ) ) == NULL )
	{
		CREATE( bit, BIT_DATA, 1 );

		bit->number = proto_bit->number;
		mudstrlcpy( bit->desc, proto_bit->desc, MSL );
		LINK( bit, ch->pcdata->firstqbit, ch->pcdata->lastqbit, next, prev );
	}
}

/* Take an abit off a character */
void remove_abit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit, *bit_next;

	if( number < 0 || number > MAX_xBITS )
		return;

	if( !ch->first_abit )
		return;

	for( bit = ch->first_abit; bit; bit = bit_next )
	{
		bit_next = bit->next;

		if( bit->number == number )
		{
			UNLINK( bit, ch->first_abit, ch->last_abit, next, prev );
			DISPOSE( bit );
		}
	}
}

/* Take a qbit off a character */
void remove_qbit( CHAR_DATA *ch, int number )
{
	BIT_DATA *bit, *bit_next;

	if( IS_NPC( ch ) )
		return;

	if( number < 0 || number > MAX_xBITS )
		return;

	if( !ch->pcdata->firstqbit )
		return;

	for( bit = ch->pcdata->firstqbit; bit; bit = bit_next )
	{
		bit_next = bit->next;

		if( bit->number == number )
		{
			UNLINK( bit, ch->pcdata->firstqbit, ch->pcdata->lastqbit, next, prev );
			DISPOSE( bit );
		}
	}
}

/* Show an abit from the mud's linked list, or all of them if 'all' is the argument */
CMDF( do_showabit )
{
	int number;
	BIT_DATA *bit;

	if( !first_abit )
	{
		send_to_char( "There are no Abits defined.\r\n", ch );
		return;
	}

	if( !str_cmp( argument, "all" ) )
	{
		for( bit = first_abit; bit; bit = bit->next )
			ch_printf( ch, "&RABIT: &Y%d &G%s\r\n", bit->number, bit->desc );
		return;
	}

	number = atoi( argument );

	if( number < 0 || number > MAX_xBITS )
		return;

	for( bit = first_abit; bit; bit = bit->next )
	{
		if( bit->number == number )
			break;
	}

	if( bit == NULL )
	{
		send_to_char( "That abit does not exist.\r\n", ch );
		return;
	}

	ch_printf( ch, "&RABIT: &Y%d\r\n", bit->number );
	ch_printf( ch, "&G%s\r\n", bit->desc );
}

/* Show a qbit from the mud's linked list or all of them if 'all' is the argument */
CMDF( do_showqbit )
{
	int number;
	BIT_DATA *bit;

	if( !first_qbit )
	{
		send_to_char( "There are no Qbits defined.\r\n", ch );
		return;
	}

	if( !str_cmp( argument, "all" ) )
	{
		for( bit = first_qbit; bit; bit = bit->next )
			ch_printf( ch, "&RQBIT: &Y%d &G%s\r\n", bit->number, bit->desc );
		return;
	}

	number = atoi( argument );

	if( number < 0 || number > MAX_xBITS )
		return;

	for( bit = first_qbit; bit; bit = bit->next )
	{
		if( bit->number == number )
			break;
	}

	if( bit == NULL )
	{
		send_to_char( "That qbit does not exist.\r\n", ch );
		return;
	}

	ch_printf( ch, "&RQBIT: &Y%d\r\n", bit->number );
	ch_printf( ch, "&G %s\r\n", bit->desc );
}

/* setabit <number> <desc> */
/* Set the description for a particular abit */
CMDF( do_setabit )
{
	BIT_DATA *bit;
	int number;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );

	number = atoi( arg );

	mudstrlcpy( arg, argument, MSL );

	if( number < 0 || number > MAX_xBITS )
	{
		send_to_char( "That is not a valid number for an abit.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: setabit <number> <description>\r\n", ch );
		send_to_char( "Syntax: setabit <number> delete\r\n", ch );
		return;
	}

	for( bit = first_abit; bit; bit = bit->next )
	{
		if( bit->number == number )
			break;
	}

	if( !str_cmp( arg, "delete" ) && bit )
	{
		UNLINK( bit, first_abit, last_abit, next, prev );
		DISPOSE( bit );
		ch_printf( ch, "Abit %d has been destroyed.\r\n", number );
		save_bits( );
		return;
	}

	if( bit == NULL )
	{
		CREATE( bit, BIT_DATA, 1 );
		bit->number = number;
		mudstrlcpy( bit->desc, "", MSL );
		LINK( bit, first_abit, last_abit, next, prev );
		ch_printf( ch, "Abit %d created.\r\n", bit->number );
	}

	mudstrlcpy( bit->desc, arg, MSL );
	ch_printf( ch, "Description for abit %d set to '%s'.\r\n", bit->number, bit->desc );

	save_bits( );
}

/* setqbit <number> <desc> */
/* Set the description for a particular qbit */
CMDF( do_setqbit )
{
	BIT_DATA *bit;
	int number;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );

	number = atoi( arg );

	mudstrlcpy( arg, argument, MSL );

	if( number < 0 || number > MAX_xBITS )
	{
		send_to_char( "That is not a valid number for a qbit.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: setqbit <number> <description>\r\n", ch );
		send_to_char( "Syntax: setqbit <number> delete\r\n", ch );
		return;
	}

	for( bit = first_qbit; bit; bit = bit->next )
	{
		if( bit->number == number )
			break;
	}

	if( !str_cmp( arg, "delete" ) && bit )
	{
		UNLINK( bit, first_qbit, last_qbit, next, prev );
		DISPOSE( bit );
		ch_printf( ch, "Qbit %d has been destroyed.\r\n", number );
		save_bits( );
		return;
	}

	if( bit == NULL )
	{
		CREATE( bit, BIT_DATA, 1 );
		bit->number = number;
		mudstrlcpy( bit->desc, "", MSL );
		LINK( bit, first_qbit, last_qbit, next, prev );
		ch_printf( ch, "Qbit %d created.\r\n", bit->number );
	}

	mudstrlcpy( bit->desc, arg, MSL );
	ch_printf( ch, "Description for qbit %d set to '%s'.\r\n", bit->number, bit->desc );

	save_bits( );
}

/* Imm command to toggle an abit on a character or to list the abits already on a character */
CMDF( do_abit )
{
	BIT_DATA *bit;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, buf );

	if( buf[0] == '\0' )
	{
		send_to_char( "Whose bits do you want to examine?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, buf ) ) == NULL )
	{
		send_to_char( "They are not in the game.\r\n", ch );
		return;
	}

	argument = one_argument( argument, buf );

	if( buf[0] == '\0' )
	{
		if( !victim->first_abit )
		{
			send_to_char( "They have no abits set on them.\r\n", ch );
			return;
		}

		ch_printf( ch, "&RABITS for %s:\r\n", IS_NPC( victim ) ? victim->short_descr : victim->name );

		for( bit = victim->first_abit; bit; bit = bit->next )
			ch_printf( ch, "&Y%4.4d: &G%s\r\n", bit->number, bit->desc );
	}
	else
	{
		int abit;

		abit = atoi( buf );

		if( abit < 0 || abit > MAX_xBITS )
		{
			send_to_char( "That is an invalid abit number.\r\n", ch );
			return;
		}

		if( get_abit( victim, abit ) != NULL )
		{
			remove_abit( victim, abit );
			ch_printf( ch, "Removed abit %d from %s.\r\n", abit, IS_NPC( victim ) ? victim->short_descr : victim->name );
		}
		else
		{
			set_abit( victim, abit );
			ch_printf( ch, "Added abit %d to %s.\r\n", abit, IS_NPC( victim ) ? victim->short_descr : victim->name );
		}
	}
}

/* Immortal command to toggle a qbit on a character or to list the qbits already on a character */
CMDF( do_qbit )
{
	BIT_DATA *bit;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument( argument, buf );

	if( buf[0] == '\0' )
	{
		send_to_char( "Whose bits do you want to examine?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, buf ) ) == NULL )
	{
		send_to_char( "They are not in the game.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "NPCs cannot have qbits.\r\n", ch );
		return;
	}

	argument = one_argument( argument, buf );

	if( buf[0] == '\0' )
	{
		if( !victim->pcdata->firstqbit )
		{
			send_to_char( "They do not have any qbits.\r\n", ch );
			return;
		}

		ch_printf( ch, "&RQBITS for %s:\r\n", victim->name );

		for( bit = victim->pcdata->firstqbit; bit; bit = bit->next )
			ch_printf( ch, "&Y%4.4d: &G%s\r\n", bit->number, bit->desc );
	}
	else
	{
		int qbit;

		qbit = atoi( buf );

		if( qbit < 0 || qbit > MAX_xBITS )
		{
			send_to_char( "That is an invalid qbit number.\r\n", ch );
			return;
		}

		if( get_qbit( victim, qbit ) != NULL )
		{
			remove_qbit( victim, qbit );
			ch_printf( ch, "Removed qbit %d from %s.\r\n", qbit, victim->name );
		}
		else
		{
			set_qbit( victim, qbit );
			ch_printf( ch, "Added qbit %d to %s.\r\n", qbit, victim->name );
		}
	}
}

/* mpaset <char> */
/* Mob prog version of do_abit */
/* Don't use this with death_progs or anything else that has the potential for the target to
 * be in another room. If the target will be in a different location after the prog, set the
 * bit BEFORE they move.
 */
CMDF( do_mpaset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int number;

	if( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		progbug( "Mpaset: missing victim", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		progbug( "Mpaset: missing bit", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		progbug( "Mpaset: victim not in room", ch );
		return;
	}

	number = atoi( arg2 );
	if( get_abit( victim, number ) != NULL )
		remove_abit( victim, number );
	else
		set_abit( victim, number );
}

/* mpqset <char> */
/* Mob prog version of do_qbit */
/* Because this can be used on death_progs, be SURE your victim is correct or you could
 * end up setting a bit on the wrong person. Use 0.$n as your victim target in progs.
 */
CMDF( do_mpqset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int number;

	if( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		progbug( "Mpqset: missing victim", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		progbug( "Mpqset: missing bit", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		progbug( "Mpqset: victim not in game", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		progbug( "Mpqset: setting Qbit on NPC", ch );
		return;
	}

	number = atoi( arg2 );
	if( get_qbit( victim, number ) != NULL )
		remove_qbit( victim, number );
	else
		set_qbit( victim, number );
}
