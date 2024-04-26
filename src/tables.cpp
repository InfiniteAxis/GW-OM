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


#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "mud.h"

/* global variables */
int top_sn;
int top_herb;
int top_wn;

SKILLTYPE *skill_table[MAX_SKILL];
SKILLTYPE *herb_table[MAX_HERB];

const char *const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Art" };

SPELL_FUN *spell_function( const char *name )
{
	SPELL_FUN *funHandle = 0;
	const char *error = 0;
	*( void ** ) ( &funHandle ) = dlsym( sysdata.dlHandle, name );

	if( ( error = dlerror( ) ) != NULL )
	{
		bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
		return spell_notfound;
	}

	return funHandle;
}

DO_FUN *skill_function( const char *name )
{
	const char *error = 0;
	DO_FUN *funHandle = 0;
	*( void ** ) ( &funHandle ) = dlsym( sysdata.dlHandle, name );

	if( ( error = dlerror( ) ) != NULL )
	{
		bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
		return skill_notfound;
	}

	return funHandle;
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE **sk1, SKILLTYPE **sk2 )
{
	SKILLTYPE *skill1 = ( *sk1 );
	SKILLTYPE *skill2 = ( *sk2 );

	if( !skill1 && skill2 )
		return 1;
	if( skill1 && !skill2 )
		return -1;
	if( !skill1 && !skill2 )
		return 0;
	if( skill1->type < skill2->type )
		return -1;
	if( skill1->type > skill2->type )
		return 1;
	return strcasecmp( skill1->name, skill2->name );
}

/*
 * Function used by qsort to sort weapons
 */
int weapon_comp( WEAPONTYPE **we1, WEAPONTYPE **we2 )
{
	WEAPONTYPE *weapon1 = ( *we1 );
	WEAPONTYPE *weapon2 = ( *we2 );

	if( !weapon1 && weapon2 )
		return 1;
	if( weapon1 && !weapon2 )
		return -1;
	if( !weapon1 && !weapon2 )
		return 0;

	return strcmp( weapon1->name, weapon2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table( )
{
	log_string( "Sorting skill table..." );
	qsort( &skill_table[1], top_sn - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );
}

/*
 * Write skill data to a file
 */
void fwrite_skill( FILE *fpout, SKILLTYPE *skill )
{
	SMAUG_AFF *aff;

	fprintf( fpout, "Name         %s~\n", skill->name );
	fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
	fprintf( fpout, "Flags        %d\n", skill->flags );
	if( skill->target )
		fprintf( fpout, "Target       %d\n", skill->target );
	if( skill->minimum_position )
		fprintf( fpout, "Minpos       %d\n", skill->minimum_position );
	if( skill->saves )
		fprintf( fpout, "Saves        %d\n", skill->saves );
	if( skill->slot )
		fprintf( fpout, "Slot         %d\n", skill->slot );
	if( skill->beats )
		fprintf( fpout, "Rounds       %d\n", skill->beats );
	if( skill->guild != -1 )
		fprintf( fpout, "Guild        %d\n", skill->guild );
	if( skill->guild == ESPIONAGE_ABILITY && skill->lp )
		fprintf( fpout, "LP           %d\n", skill->lp );
	if( skill->skill_fun )
		fprintf( fpout, "Code         %s\n", skill->skill_fun_name );
	else if( skill->spell_fun )
		fprintf( fpout, "Code         %s\n", skill->spell_fun_name );
	fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
	if( skill->artcombo && skill->artcombo[0] != '\0' )
		fprintf( fpout, "Combo        %s~\n", skill->artcombo );
	if( skill->msg_off && skill->msg_off[0] != '\0' )
		fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );

	if( skill->hit_char && skill->hit_char[0] != '\0' )
		fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
	if( skill->hit_vict && skill->hit_vict[0] != '\0' )
		fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
	if( skill->hit_room && skill->hit_room[0] != '\0' )
		fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );

	if( skill->miss_char && skill->miss_char[0] != '\0' )
		fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
	if( skill->miss_vict && skill->miss_vict[0] != '\0' )
		fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
	if( skill->miss_room && skill->miss_room[0] != '\0' )
		fprintf( fpout, "Missroom     %s~\n", skill->miss_room );

	if( skill->die_char && skill->die_char[0] != '\0' )
		fprintf( fpout, "Diechar      %s~\n", skill->die_char );
	if( skill->die_vict && skill->die_vict[0] != '\0' )
		fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
	if( skill->die_room && skill->die_room[0] != '\0' )
		fprintf( fpout, "Dieroom      %s~\n", skill->die_room );

	if( skill->imm_char && skill->imm_char[0] != '\0' )
		fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
	if( skill->imm_vict && skill->imm_vict[0] != '\0' )
		fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
	if( skill->imm_room && skill->imm_room[0] != '\0' )
		fprintf( fpout, "Immroom      %s~\n", skill->imm_room );

	if( skill->dice && skill->dice[0] != '\0' )
		fprintf( fpout, "Dice         %s~\n", skill->dice );
	if( skill->value )
		fprintf( fpout, "Value        %d\n", skill->value );
	if( skill->difficulty )
		fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
	if( skill->participants )
		fprintf( fpout, "Participants %d\n", skill->participants );
	if( skill->components && skill->components[0] != '\0' )
		fprintf( fpout, "Components   %s~\n", skill->components );
	if( skill->teachers && skill->teachers[0] != '\0' )
		fprintf( fpout, "Teachers     %s~\n", skill->teachers );
	for( aff = skill->first_affect; aff; aff = aff->next )
		fprintf( fpout, "Affect       '%s' %d '%s' %d\n", aff->duration, aff->location, aff->modifier, aff->bitvector );
	if( skill->alignment )
		fprintf( fpout, "Alignment   %d\n", skill->alignment );

	if( skill->type != SKILL_HERB )
	{
		fprintf( fpout, "Minlevel     %d\n", skill->min_level );
	}
	fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table( )
{
	int x;
	FILE *fpout;

	if( ( fpout = FileOpen( SKILL_FILE, "w" ) ) == NULL )
	{
		bug( "Cannot open skills.dat for writting", 0 );
		perror( SKILL_FILE );
		return;
	}

	for( x = 0; x < top_sn; x++ )
	{
		if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
			break;
		fprintf( fpout, "#SKILL\n" );
		fwrite_skill( fpout, skill_table[x] );
	}
	fprintf( fpout, "#END\n" );
	FileClose( fpout );
}

/*
 * Write weapon data to a file
 */
void fwrite_weapon( FILE *fpout, WEAPONTYPE *weapon )
{
	fprintf( fpout, "Name         %s~\n", weapon->name );
	fprintf( fpout, "Type         %d\n", weapon->type );
	fprintf( fpout, "Ammo         %d\n", weapon->ammo );
	fprintf( fpout, "Ammomax      %d\n", weapon->ammomax );
	fprintf( fpout, "Ammotype     %d\n", weapon->ammotype );
	fprintf( fpout, "Damage       %d\n", weapon->damage );
	fprintf( fpout, "Price        %d\n", weapon->price );
	if( weapon->slot )
		fprintf( fpout, "Slot         %d\n", weapon->slot );
	fprintf( fpout, "End\n\n" );
}


/*
 * Save the herb table to disk
 */
void save_herb_table( )
{
	int x;
	FILE *fpout;

	if( ( fpout = FileOpen( HERB_FILE, "w" ) ) == NULL )
	{
		bug( "Cannot open herbs.dat for writting", 0 );
		perror( HERB_FILE );
		return;
	}

	for( x = 0; x < top_herb; x++ )
	{
		if( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
			break;
		fprintf( fpout, "#HERB\n" );
		fwrite_skill( fpout, herb_table[x] );
	}
	fprintf( fpout, "#END\n" );
	FileClose( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials( )
{
	FILE *fpout;
	SOCIALTYPE *social;
	int x;

	if( ( fpout = FileOpen( SOCIAL_FILE, "w" ) ) == NULL )
	{
		bug( "Cannot open socials.dat for writting", 0 );
		perror( SOCIAL_FILE );
		return;
	}

	for( x = 0; x < 27; x++ )
	{
		for( social = social_index[x]; social; social = social->next )
		{
			if( !social->name || social->name[0] == '\0' )
			{
				bug( "Save_socials: blank social in hash bucket %d", x );
				continue;
			}
			fprintf( fpout, "#SOCIAL\n" );
			fprintf( fpout, "Name        %s~\n", social->name );
			if( social->char_no_arg )
				fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
			else
				bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
			if( social->others_no_arg )
				fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
			if( social->char_found )
				fprintf( fpout, "CharFound   %s~\n", social->char_found );
			if( social->others_found )
				fprintf( fpout, "OthersFound %s~\n", social->others_found );
			if( social->vict_found )
				fprintf( fpout, "VictFound   %s~\n", social->vict_found );
			if( social->char_auto )
				fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
			if( social->others_auto )
				fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
			fprintf( fpout, "End\n\n" );
		}
	}
	fprintf( fpout, "#END\n" );
	FileClose( fpout );
}

int get_skill( const char *skilltype )
{
	if( !str_cmp( skilltype, "Spell" ) )
		return SKILL_SPELL;
	if( !str_cmp( skilltype, "Skill" ) )
		return SKILL_SKILL;
	if( !str_cmp( skilltype, "Weapon" ) )
		return SKILL_WEAPON;
	if( !str_cmp( skilltype, "Tongue" ) )
		return SKILL_TONGUE;
	if( !str_cmp( skilltype, "Herb" ) )
		return SKILL_HERB;
	return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 */
void save_commands( )
{
	FILE *fpout;
	CMDTYPE *command;
	int x;

	if( ( fpout = FileOpen( COMMAND_FILE, "w" ) ) == NULL )
	{
		bug( "Cannot open commands.dat for writing", 0 );
		perror( COMMAND_FILE );
		return;
	}

	for( x = 0; x < 126; x++ )
	{
		for( command = command_hash[x]; command; command = command->next )
		{
			if( !command->name || command->name[0] == '\0' )
			{
				bug( "Save_commands: blank command in hash bucket %d", x );
				continue;
			}
			fprintf( fpout, "#COMMAND\n" );
			fprintf( fpout, "Name        %s~\n", command->name );
			fprintf( fpout, "Code        %s\n", command->fun_name ? command->fun_name : "" );
			fprintf( fpout, "Position    %d\n", command->position );
			fprintf( fpout, "Level       %d\n", command->level );
			fprintf( fpout, "Log         %d\n", command->log );
			fprintf( fpout, "CShow       %d\n", command->cshow );
			fprintf( fpout, "End\n\n" );
		}
	}
	fprintf( fpout, "#END\n" );
	FileClose( fpout );
}

SKILLTYPE *fread_skill( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	SKILLTYPE *skill;

	CREATE( skill, SKILLTYPE, 1 );

	skill->guild = -1;

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

		case 'A':
			KEY( "Alignment", skill->alignment, fread_number( fp ) );
			if( !str_cmp( word, "Affect" ) )
			{
				SMAUG_AFF *aff;

				CREATE( aff, SMAUG_AFF, 1 );
				aff->duration = str_dup( fread_word( fp ) );
				aff->location = fread_number( fp );
				aff->modifier = str_dup( fread_word( fp ) );
				aff->bitvector = fread_number( fp );
				LINK( aff, skill->first_affect, skill->last_affect, next, prev );
				fMatch = true;
				break;
			}
			break;

		case 'C':
			if( !str_cmp( word, "Code" ) )
			{
				SPELL_FUN *spellfun;
				DO_FUN *dofun;
				const char *w = fread_word( fp );

				fMatch = true;
				if( !str_prefix( "do_", w ) && ( dofun = skill_function( w ) ) != skill_notfound )
				{
					skill->skill_fun = dofun;
					skill->spell_fun = NULL;
					skill->skill_fun_name = str_dup( w );
				}
				else if( str_prefix( "do_", w ) && ( spellfun = spell_function( w ) ) != spell_notfound )
				{
					skill->spell_fun = spellfun;
					skill->skill_fun = NULL;
					skill->spell_fun_name = str_dup( w );
				}
				else
				{
					bug( "fread_skill: unknown skill/spell %s", w );
					skill->spell_fun = spell_null;
				}
				break;
			}
			KEY( "Combo", skill->artcombo, fread_string_nohash( fp ) );
			KEY( "Components", skill->components, fread_string_nohash( fp ) );
			break;

		case 'D':
			KEY( "Dammsg", skill->noun_damage, fread_string_nohash( fp ) );
			KEY( "Dice", skill->dice, fread_string_nohash( fp ) );
			KEY( "Diechar", skill->die_char, fread_string_nohash( fp ) );
			KEY( "Dieroom", skill->die_room, fread_string_nohash( fp ) );
			KEY( "Dievict", skill->die_vict, fread_string_nohash( fp ) );
			KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
				return skill;
			break;

		case 'F':
			KEY( "Flags", skill->flags, fread_number( fp ) );
			break;

		case 'G':
			KEY( "Guild", skill->guild, fread_number( fp ) );
			break;

		case 'H':
			KEY( "Hitchar", skill->hit_char, fread_string_nohash( fp ) );
			KEY( "Hitroom", skill->hit_room, fread_string_nohash( fp ) );
			KEY( "Hitvict", skill->hit_vict, fread_string_nohash( fp ) );
			break;

		case 'I':
			KEY( "Immchar", skill->imm_char, fread_string_nohash( fp ) );
			KEY( "Immroom", skill->imm_room, fread_string_nohash( fp ) );
			KEY( "Immvict", skill->imm_vict, fread_string_nohash( fp ) );
			break;

		case 'L':
			KEY( "LP", skill->lp, fread_number( fp ) );
			break;

		case 'M':
			if( !str_cmp( word, "Mana" ) )
			{
				fMatch = true;
				break;
			}
			KEY( "Minlevel", skill->min_level, fread_number( fp ) );
			KEY( "Minpos", skill->minimum_position, fread_number( fp ) );
			KEY( "Misschar", skill->miss_char, fread_string_nohash( fp ) );
			KEY( "Missroom", skill->miss_room, fread_string_nohash( fp ) );
			KEY( "Missvict", skill->miss_vict, fread_string_nohash( fp ) );
			break;

		case 'N':
			KEY( "Name", skill->name, fread_string_nohash( fp ) );
			break;

		case 'P':
			KEY( "Participants", skill->participants, fread_number( fp ) );
			break;

		case 'R':
			KEY( "Rounds", skill->beats, fread_number( fp ) );
			break;

		case 'S':
			KEY( "Slot", skill->slot, fread_number( fp ) );
			KEY( "Saves", skill->saves, fread_number( fp ) );
			break;

		case 'T':
			KEY( "Target", skill->target, fread_number( fp ) );
			KEY( "Teachers", skill->teachers, fread_string_nohash( fp ) );
			KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
			break;

		case 'V':
			KEY( "Value", skill->value, fread_number( fp ) );
			break;

		case 'W':
			KEY( "Wearoff", skill->msg_off, fread_string_nohash( fp ) );
			break;
		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_skill: no match: %s", word );
			bug( buf, 0 );
		}
	}
}


void load_skill_table( )
{
	FILE *fp;

	if( ( fp = FileOpen( SKILL_FILE, "r" ) ) != NULL )
	{
		top_sn = 0;
		for( ;; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_skill_table: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "SKILL" ) )
			{
				if( top_sn >= MAX_SKILL )
				{
					bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
					FileClose( fp );
					return;
				}
				skill_table[top_sn++] = fread_skill( fp );
				continue;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_skill_table: bad section.", 0 );
				continue;
			}
		}
		FileClose( fp );
	}
	else
	{
		bug( "Cannot open skills.dat", 0 );
		exit( 111 );
	}
}

WEAPONTYPE *fread_weapon( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	WEAPONTYPE *weapon;

	CREATE( weapon, WEAPONTYPE, 1 );

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

		case 'A':
			KEY( "Ammo", weapon->ammo, fread_number( fp ) );
			KEY( "Ammomax", weapon->ammomax, fread_number( fp ) );
			KEY( "Ammotype", weapon->ammotype, fread_number( fp ) );
			break;

		case 'D':
			KEY( "Damage", weapon->damage, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
				return weapon;
			break;

		case 'N':
			KEY( "Name", weapon->name, fread_string_nohash( fp ) );
			break;

		case 'P':
			KEY( "Price", weapon->price, fread_number( fp ) );
			break;

		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_weapon: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

void load_herb_table( )
{
	FILE *fp;

	if( ( fp = FileOpen( HERB_FILE, "r" ) ) != NULL )
	{
		top_herb = 0;
		for( ;; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_herb_table: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "HERB" ) )
			{
				if( top_herb >= MAX_HERB )
				{
					bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
					FileClose( fp );
					return;
				}
				herb_table[top_herb++] = fread_skill( fp );
				if( herb_table[top_herb - 1]->slot == 0 )
					herb_table[top_herb - 1]->slot = top_herb - 1;
				continue;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_herb_table: bad section.", 0 );
				continue;
			}
		}
		FileClose( fp );
	}
	else
	{
		bug( "Cannot open herbs.dat", 0 );
		exit( 112 );
	}
}

void fread_social( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	SOCIALTYPE *social;

	CREATE( social, SOCIALTYPE, 1 );

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
			KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
			KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
			KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !social->name )
				{
					bug( "Fread_social: Name not found", 0 );
					free_social( social );
					return;
				}
				if( !social->char_no_arg )
				{
					bug( "Fread_social: CharNoArg not found", 0 );
					free_social( social );
					return;
				}
				add_social( social );
				return;
			}
			break;

		case 'N':
			KEY( "Name", social->name, fread_string_nohash( fp ) );
			break;

		case 'O':
			KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
			KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
			KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
			break;

		case 'V':
			KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
			break;
		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_social: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

void load_socials( )
{
	FILE *fp;

	if( ( fp = FileOpen( SOCIAL_FILE, "r" ) ) != NULL )
	{
		for( ;; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_socials: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "SOCIAL" ) )
			{
				fread_social( fp );
				continue;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_socials: bad section.", 0 );
				continue;
			}
		}
		FileClose( fp );
	}
	else
	{
		bug( "Cannot open socials.dat", 0 );
		exit( 113 );
	}
}

void fread_command( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	CMDTYPE *command;

	CREATE( command, CMDTYPE, 1 );
	command->cshow = 0;

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
			KEY( "Code", command->fun_name, str_dup( fread_word( fp ) ) );
			KEY( "CShow", command->cshow, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !command->name )
				{
					bug( "%s", "Fread_command: Name not found" );
					free_command( command );
					return;
				}
				if( !command->fun_name )
				{
					bug( "fread_command: No function name supplied for %s", command->name );
					free_command( command );
					return;
				}
				/*
				 * Mods by Trax
				 * Fread in code into char* and try linkage here then
				 * deal in the "usual" way I suppose..
				 */
				command->do_fun = skill_function( command->fun_name );
				if( command->do_fun == skill_notfound )
				{
					bug( "Fread_command: Function %s not found for %s", command->fun_name, command->name );
					free_command( command );
					return;
				}
				add_command( command );
				return;
			}
			break;

		case 'L':
			KEY( "Level", command->level, fread_number( fp ) );
			KEY( "Log", command->log, fread_number( fp ) );
			break;

		case 'N':
			KEY( "Name", command->name, fread_string_nohash( fp ) );
			break;

		case 'P':
			KEY( "Position", command->position, fread_number( fp ) );
			break;
		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_command: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

void load_commands( )
{
	FILE *fp;

	if( ( fp = FileOpen( COMMAND_FILE, "r" ) ) != NULL )
	{
		for( ;; )
		{
			char letter;
			char *word;

			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_commands: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "COMMAND" ) )
			{
				fread_command( fp );
				continue;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_commands: bad section.", 0 );
				continue;
			}
		}
		FileClose( fp );
	}
	else
	{
		bug( "Cannot open commands.dat", 0 );
		exit( 115 );
	}

}

void write_last_file( const char *entry )
{
	FILE *fpout;
	FILE *fptemp;
	char filename[MAX_INPUT_LENGTH];
	char tempname[MAX_INPUT_LENGTH];
	snprintf( filename, sizeof(filename), "%s", LAST_LIST );
	snprintf( tempname, sizeof(tempname), "%s", LAST_TEMP_LIST );
	if( ( fptemp = FileOpen( tempname, "w" ) ) == NULL )
	{
		bug( "Cannot open: %s for writing", tempname );
		return;
	}
	fprintf( fptemp, "%s\n", entry ); //adds new entry to top of the file
	if( ( fpout = FileOpen( filename, "r" ) ) != NULL )
	{
		copy_files_contents( fpout, fptemp ); //copy the rest to the file
		FileClose( fpout ); //close the files since writing is done
	}
	FileClose( fptemp );
	if( remove( filename ) != 0 && FileOpen( filename, "r" ) != NULL )
	{
		bug( "Do not have permission to delete the %s file", filename );
		return;
	}
	if( rename( tempname, filename ) != 0 )
	{
		bug( "Do not have permission to rename the %s file", tempname );
		return;
	}
	return;
}
