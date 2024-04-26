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
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

#define INNO            0
#define INYES           1

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;

SENATE_DATA *first_senator;
SENATE_DATA *last_senator;

PLANET_DATA *first_planet;
PLANET_DATA *last_planet;

GUARD_DATA *first_guard;
GUARD_DATA *last_guard;

/* local routines */
void fread_clan( CLAN_DATA *clan, FILE *fp );
bool load_clan_file( const char *clanfile );
void write_clan_list( void );
void fread_planet( PLANET_DATA *planet, FILE *fp );
bool load_planet_file( const char *planetfile );
void write_planet_list( void );

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( const char *name )
{
	CLAN_DATA *clan;

	for( clan = first_clan; clan; clan = clan->next )
		if( !str_cmp( name, clan->name ) )
			return clan;
	return NULL;
}

PLANET_DATA *get_planet( const char *name )
{
	PLANET_DATA *planet;

	for( planet = first_planet; planet; planet = planet->next )
		if( !str_cmp( name, planet->name ) )
			return planet;
	return NULL;
}

void write_clan_list( )
{
	CLAN_DATA *tclan;
	FILE *fpout;
	char filename[256];

	snprintf( filename, 256, "%s%s", CLAN_DIR, CLAN_LIST );
	fpout = FileOpen( filename, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open clan.lst for writing!\r\n", 0 );
		return;
	}
	for( tclan = first_clan; tclan; tclan = tclan->next )
		fprintf( fpout, "\"%s\"\n", tclan->filename );
	fprintf( fpout, "$\n" );
	FileClose( fpout );
}

void write_planet_list( )
{
	PLANET_DATA *tplanet;
	FILE *fpout;
	char filename[256];

	snprintf( filename, 256, "%s%s", PLANET_DIR, PLANET_LIST );
	fpout = FileOpen( filename, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open planet.lst for writing!\r\n", 0 );
		return;
	}
	for( tplanet = first_planet; tplanet; tplanet = tplanet->next )
		fprintf( fpout, "%s\n", tplanet->filename );
	fprintf( fpout, "$\n" );
	FileClose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
	FILE *fp;
	char filename[256];
	char buf[MAX_STRING_LENGTH];

	if( !clan )
	{
		bug( "save_clan: null clan pointer!", 0 );
		return;
	}

	if( !clan->filename || clan->filename[0] == '\0' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "save_clan: %s has no filename", clan->name );
		bug( buf, 0 );
		return;
	}

	snprintf( filename, 256, "%s%s", CLAN_DIR, clan->filename );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_clan: FileOpen", 0 );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#CLAN\n" );
		fprintf( fp, "Name         %s~\n", clan->name );
		fprintf( fp, "Shortname    %s~\n", clan->shortname );
		fprintf( fp, "Filename     %s~\n", clan->filename );
		fprintf( fp, "Description  %s~\n", clan->description );
		fprintf( fp, "Leader       %s~\n", clan->leader );
		fprintf( fp, "NumberOne    %s~\n", clan->number1 );
		fprintf( fp, "NumberTwo    %s~\n", clan->number2 );
		fprintf( fp, "Atwar        %s~\n", clan->atwar );
		fprintf( fp, "Ally         %s~\n", clan->ally );
		fprintf( fp, "Motto        %s~\n", clan->motto );
		fprintf( fp, "ColorOne     %s~\n", clan->cone );
		fprintf( fp, "ColorTwo     %s~\n", clan->ctwo );
		fprintf( fp, "Thug         %d\n", clan->thug );
		fprintf( fp, "Hitmen       %d\n", clan->hitmen );
		fprintf( fp, "Drugaddict   %d\n", clan->drugaddict );
		fprintf( fp, "Pimp         %d\n", clan->pimp );
		fprintf( fp, "Scalper      %d\n", clan->scalper );
		fprintf( fp, "PKills       %d\n", clan->pkills );
		fprintf( fp, "PDeaths      %d\n", clan->pdeaths );
		fprintf( fp, "MKills       %d\n", clan->mkills );
		fprintf( fp, "MDeaths      %d\n", clan->mdeaths );
		fprintf( fp, "Type         %d\n", clan->clan_type );
		fprintf( fp, "Members      %d\n", clan->members );
		fprintf( fp, "Board        %d\n", clan->board );
		fprintf( fp, "Storeroom    %d\n", clan->storeroom );
		fprintf( fp, "GuardOne     %d\n", clan->guard1 );
		fprintf( fp, "GuardTwo     %d\n", clan->guard2 );
		fprintf( fp, "PatrolOne    %d\n", clan->patrol1 );
		fprintf( fp, "PatrolTwo    %d\n", clan->patrol2 );
		fprintf( fp, "TrooperOne   %d\n", clan->trooper1 );
		fprintf( fp, "TrooperTwo   %d\n", clan->trooper2 );
		fprintf( fp, "Funds        %ld\n", clan->funds );
		fprintf( fp, "Bases        %d\n", clan->bases );
		fprintf( fp, "Income       %d\n", clan->income );
		fprintf( fp, "Jail         %d\n", clan->jail );
		if( clan->mainclan )
			fprintf( fp, "MainClan     %s~\n", clan->mainclan->name );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
		FileClose( fp );
		fp = NULL;
	}
	return;
}

void save_planet( PLANET_DATA *planet )
{
	FILE *fp;
	char filename[256];
	char buf[MAX_STRING_LENGTH];

	if( !planet )
	{
		bug( "save_planet: null planet pointer!", 0 );
		return;
	}

	if( !planet->filename || planet->filename[0] == '\0' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "save_planet: %s has no filename", planet->name );
		bug( buf, 0 );
		return;
	}

	snprintf( filename, 256, "%s%s", PLANET_DIR, planet->filename );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_planet: FileOpen", 0 );
		perror( filename );
	}
	else
	{
		AREA_DATA *pArea;

		fprintf( fp, "#PLANET\n" );
		fprintf( fp, "Name         %s~\n", planet->name );
		fprintf( fp, "Filename     %s~\n", planet->filename );
		fprintf( fp, "BaseValue    %ld\n", planet->base_value );
		fprintf( fp, "Flags        %d\n", planet->flags );
		fprintf( fp, "PopSupport   %f\n", planet->pop_support );

		if( planet->pImport[0] || planet->pImport[1] || planet->pImport[2]
			|| planet->pImport[3] || planet->pImport[4] || planet->pImport[5]
			|| planet->pImport[6] || planet->pImport[7] || planet->pImport[8] || planet->pImport[9] )
			fprintf( fp, "Imports       %d %d %d %d %d %d %d %d %d %d\n",
				planet->pImport[0], planet->pImport[1], planet->pImport[2],
				planet->pImport[3], planet->pImport[4], planet->pImport[5],
				planet->pImport[6], planet->pImport[7], planet->pImport[8], planet->pImport[9] );

		if( planet->pExport[0] || planet->pExport[1] || planet->pExport[2]
			|| planet->pExport[3] || planet->pExport[4] || planet->pExport[5]
			|| planet->pExport[6] || planet->pExport[7] || planet->pExport[8] || planet->pExport[9] )
			fprintf( fp, "Exports       %d %d %d %d %d %d %d %d %d %d\n",
				planet->pExport[0], planet->pExport[1], planet->pExport[2],
				planet->pExport[3], planet->pExport[4], planet->pExport[5],
				planet->pExport[6], planet->pExport[7], planet->pExport[8], planet->pExport[9] );

		if( planet->pBuy[0] || planet->pBuy[1] || planet->pBuy[2]
			|| planet->pBuy[3] || planet->pBuy[4] || planet->pBuy[5]
			|| planet->pBuy[6] || planet->pBuy[7] || planet->pBuy[8] || planet->pBuy[9] )
			fprintf( fp, "Buy       %d %d %d %d %d %d %d %d %d %d\n",
				planet->pBuy[0], planet->pBuy[1], planet->pBuy[2],
				planet->pBuy[3], planet->pBuy[4], planet->pBuy[5],
				planet->pBuy[6], planet->pBuy[7], planet->pBuy[8], planet->pBuy[9] );

		if( planet->pSell[0] || planet->pSell[1] || planet->pSell[2]
			|| planet->pSell[3] || planet->pSell[4] || planet->pSell[5]
			|| planet->pSell[6] || planet->pSell[7] || planet->pSell[8] || planet->pSell[9] )
			fprintf( fp, "Sell       %d %d %d %d %d %d %d %d %d %d\n",
				planet->pSell[0], planet->pSell[1], planet->pSell[2],
				planet->pSell[3], planet->pSell[4], planet->pSell[5],
				planet->pSell[6], planet->pSell[7], planet->pSell[8], planet->pSell[9] );

		if( planet->starsystem && planet->starsystem->name )
			fprintf( fp, "Starsystem   %s~\n", planet->starsystem->name );
		if( planet->governed_by && planet->governed_by->name )
			fprintf( fp, "GovernedBy   %s~\n", planet->governed_by->name );
		for( pArea = planet->first_area; pArea; pArea = pArea->next_on_planet )
			if( pArea->filename )
				fprintf( fp, "Area         %s~\n", pArea->filename );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
	}
	FileClose( fp );
	return;
}


/*
 * Read in actual clan data.
 */

void fread_clan( CLAN_DATA *clan, FILE *fp )
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

		case 'A':
			KEY( "Ally", clan->ally, fread_string( fp ) );
			KEY( "Atwar", clan->atwar, fread_string( fp ) );
			break;

		case 'B':
			KEY( "Bases", clan->bases, fread_number( fp ) );
			KEY( "Board", clan->board, fread_number( fp ) );
			break;

		case 'C':
			KEY( "ColorOne", clan->cone, fread_string( fp ) );
			KEY( "ColorTwo", clan->ctwo, fread_string( fp ) );

		case 'D':
			KEY( "Description", clan->description, fread_string( fp ) );
			KEY( "Drugaddict", clan->drugaddict, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !clan->name )
					clan->name = STRALLOC( "" );
				if( !clan->leader )
					clan->leader = STRALLOC( "" );
				if( !clan->ally )
					clan->ally = STRALLOC( "" );
				if( !clan->atwar )
					clan->atwar = STRALLOC( "" );
				if( !clan->description )
					clan->description = STRALLOC( "" );
				if( !clan->number1 )
					clan->number1 = STRALLOC( "" );
				if( !clan->number2 )
					clan->number2 = STRALLOC( "" );
				if( !clan->tmpstr )
					clan->tmpstr = STRALLOC( "" );
				if( !clan->motto )
					clan->motto = str_dup( "" );
				if( !clan->cone )
					clan->cone = STRALLOC( "" );
				if( !clan->ctwo )
					clan->ctwo = STRALLOC( "" );
				return;
			}
			break;

		case 'F':
			KEY( "Funds", clan->funds, fread_number( fp ) );
			KEY( "Filename", clan->filename, fread_string_nohash( fp ) );
			break;

		case 'G':
			KEY( "GuardOne", clan->guard1, fread_number( fp ) );
			KEY( "GuardTwo", clan->guard2, fread_number( fp ) );
			break;

		case 'H':
			KEY( "Hitmen", clan->hitmen, fread_number( fp ) );
			break;

		case 'I':
			KEY( "Income", clan->income, fread_number( fp ) );
			break;

		case 'J':
			KEY( "Jail", clan->jail, fread_number( fp ) );
			break;

		case 'L':
			KEY( "Leader", clan->leader, fread_string( fp ) );
			break;

		case 'M':
			KEY( "MDeaths", clan->mdeaths, fread_number( fp ) );
			KEY( "Members", clan->members, fread_number( fp ) );
			KEY( "MKills", clan->mkills, fread_number( fp ) );
			KEY( "Motto", clan->motto, fread_string_nohash( fp ) );
			KEY( "MainClan", clan->tmpstr, fread_string( fp ) );
			break;

		case 'N':
			KEY( "Name", clan->name, fread_string( fp ) );
			KEY( "NumberOne", clan->number1, fread_string( fp ) );
			KEY( "NumberTwo", clan->number2, fread_string( fp ) );
			break;

		case 'P':
			KEY( "Pimp", clan->pimp, fread_number( fp ) );
			KEY( "PDeaths", clan->pdeaths, fread_number( fp ) );
			KEY( "PKills", clan->pkills, fread_number( fp ) );
			KEY( "PatrolOne", clan->patrol1, fread_number( fp ) );
			KEY( "PatrolTwo", clan->patrol2, fread_number( fp ) );
			break;

		case 'S':
			KEY( "Scalper", clan->scalper, fread_number( fp ) );
			KEY( "Storeroom", clan->storeroom, fread_number( fp ) );
			KEY( "Shortname", clan->shortname, fread_string_nohash( fp ) );
			break;

		case 'T':
			KEY( "Thug", clan->thug, fread_number( fp ) );
			KEY( "Type", clan->clan_type, fread_number( fp ) );
			KEY( "TrooperOne", clan->trooper1, fread_number( fp ) );
			KEY( "TrooperTwo", clan->trooper2, fread_number( fp ) );
			break;

		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_clan: no match: %s", word );
			bug( buf, 0 );
		}

	}
}

void fread_planet( PLANET_DATA *planet, FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	char *line;
	const char *word;
	int x1, x2, x3, x4, x5, x6, x7, x8, x9, x0;
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

		case 'A':
			if( !str_cmp( word, "Area" ) )
			{
				const char *aName;
				AREA_DATA *pArea;

				aName = fread_string( fp );
				for( pArea = first_area; pArea; pArea = pArea->next )
					if( pArea->filename && !str_cmp( pArea->filename, aName ) )
					{
						pArea->planet = planet;
						LINK( pArea, planet->first_area, planet->last_area, next_on_planet, prev_on_planet );
					}
				fMatch = true;
				STRFREE( aName );
			}
			break;

		case 'B':
			KEY( "BaseValue", planet->base_value, fread_number( fp ) );

			if( !str_cmp( word, "Buy" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				planet->pBuy[0] = x0;
				planet->pBuy[1] = x1;
				planet->pBuy[2] = x2;
				planet->pBuy[3] = x3;
				planet->pBuy[4] = x4;
				planet->pBuy[5] = x5;
				planet->pBuy[6] = x6;
				planet->pBuy[7] = x7;
				planet->pBuy[8] = x8;
				planet->pBuy[9] = x9;
				fMatch = true;
				break;
			}

			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !planet->name )
					planet->name = STRALLOC( "" );
				return;
			}

			if( !str_cmp( word, "Exports" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				planet->pExport[0] = x0;
				planet->pExport[1] = x1;
				planet->pExport[2] = x2;
				planet->pExport[3] = x3;
				planet->pExport[4] = x4;
				planet->pExport[5] = x5;
				planet->pExport[6] = x6;
				planet->pExport[7] = x7;
				planet->pExport[8] = x8;
				planet->pExport[9] = x9;
				fMatch = true;
				break;
			}

			break;

		case 'F':
			KEY( "Filename", planet->filename, fread_string_nohash( fp ) );
			KEY( "Flags", planet->flags, fread_number( fp ) );
			break;

		case 'G':
			if( !str_cmp( word, "GovernedBy" ) )
			{
				const char *clan_name = fread_string( fp );
				planet->governed_by = get_clan( clan_name );
				fMatch = true;
				STRFREE( clan_name );
			}
			break;

		case 'I':
			if( !str_cmp( word, "Imports" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				planet->pImport[0] = x0;
				planet->pImport[1] = x1;
				planet->pImport[2] = x2;
				planet->pImport[3] = x3;
				planet->pImport[4] = x4;
				planet->pImport[5] = x5;
				planet->pImport[6] = x6;
				planet->pImport[7] = x7;
				planet->pImport[8] = x8;
				planet->pImport[9] = x9;
				fMatch = true;
				break;
			}
			break;

		case 'N':
			KEY( "Name", planet->name, fread_string( fp ) );
			break;

		case 'P':
			KEY( "PopSupport", planet->pop_support, fread_float( fp ) );
			break;

		case 'S':
			if( !str_cmp( word, "Starsystem" ) )
			{
				const char *starsystem_name = fread_string( fp );
				planet->starsystem = starsystem_from_name( starsystem_name );
				if( planet->starsystem )
				{
					SPACE_DATA *starsystem = planet->starsystem;

					LINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
				}
				fMatch = true;
				STRFREE( starsystem_name );
			}

			if( !str_cmp( word, "Sell" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				planet->pSell[0] = x0;
				planet->pSell[1] = x1;
				planet->pSell[2] = x2;
				planet->pSell[3] = x3;
				planet->pSell[4] = x4;
				planet->pSell[5] = x5;
				planet->pSell[6] = x6;
				planet->pSell[7] = x7;
				planet->pSell[8] = x8;
				planet->pSell[9] = x9;
				fMatch = true;
				break;
			}

			break;

		case 'T':
			KEY( "Taxes", planet->base_value, fread_number( fp ) );
			break;

		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_planet: no match: %s", word );
			bug( buf, 0 );
		}

	}
}


/*
 * Load a clan file
 */

bool load_clan_file( const char *clanfile )
{
	char filename[256];
	CLAN_DATA *clan;
	FILE *fp;
	bool found;

	CREATE( clan, CLAN_DATA, 1 );
	clan->next_subclan = NULL;
	clan->prev_subclan = NULL;
	clan->last_subclan = NULL;
	clan->first_subclan = NULL;
	clan->mainclan = NULL;

	found = false;
	snprintf( filename, 256, "%s%s", CLAN_DIR, clanfile );

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{

		found = true;
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
				bug( "Load_clan_file: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "CLAN" ) )
			{
				fread_clan( clan, fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				char buf[MAX_STRING_LENGTH];

				snprintf( buf, MAX_STRING_LENGTH, "Load_clan_file: bad section: %s.", word );
				bug( buf, 0 );
				break;
			}
		}
		FileClose( fp );
	}

	if( found )
	{
		ROOM_INDEX_DATA *storeroom;

		LINK( clan, first_clan, last_clan, next, prev );

		if( clan->storeroom == 0 || ( storeroom = get_room_index( clan->storeroom ) ) == NULL )
		{
			log_string( "Storeroom not found" );
			return found;
		}

		snprintf( filename, 256, "%s%s.vault", CLAN_DIR, clan->filename );
		if( ( fp = FileOpen( filename, "r" ) ) != NULL )
		{
			int iNest;
			OBJ_DATA *tobj, *tobj_next;

			log_string( "Loading clan storage room" );
			rset_supermob( storeroom );
			for( iNest = 0; iNest < MAX_NEST; iNest++ )
				rgObjNest[iNest] = NULL;

			found = true;
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
					bug( "Load_clan_vault: # not found.", 0 );
					bug( clan->name, 0 );
					break;
				}

				word = fread_word( fp );
				if( !str_cmp( word, "OBJECT" ) )    /* Objects  */
					fread_obj( supermob, fp, OS_CARRY );
				else if( !str_cmp( word, "END" ) )  /* Done     */
					break;
				else
				{
					bug( "Load_clan_vault: bad section.", 0 );
					bug( clan->name, 0 );
					break;
				}
			}
			FileClose( fp );
			for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
			{
				tobj_next = tobj->next_content;
				obj_from_char( tobj );
				obj_to_room( tobj, storeroom );
			}
			release_supermob( );
		}
		else
			log_string( "Cannot open clan vault" );
	}
	else
		DISPOSE( clan );

	return found;
}

bool load_planet_file( const char *planetfile )
{
	char filename[256];
	PLANET_DATA *planet;
	FILE *fp;
	bool found;

	CREATE( planet, PLANET_DATA, 1 );

	planet->governed_by = NULL;
	planet->next_in_system = NULL;
	planet->prev_in_system = NULL;
	planet->starsystem = NULL;
	planet->first_area = NULL;
	planet->last_area = NULL;
	planet->first_guard = NULL;
	planet->last_guard = NULL;

	found = false;
	snprintf( filename, 256, "%s%s", PLANET_DIR, planetfile );

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{

		found = true;
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
				bug( "Load_planet_file: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "PLANET" ) )
			{
				fread_planet( planet, fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				char buf[MAX_STRING_LENGTH];

				snprintf( buf, MAX_STRING_LENGTH, "Load_planet_file: bad section: %s.", word );
				bug( buf, 0 );
				break;
			}
		}
		FileClose( fp );
	}

	if( !found )
		DISPOSE( planet );
	else
		LINK( planet, first_planet, last_planet, next, prev );

	return found;
}


/*
 * Load in all the clan files.
 */
void load_clans( )
{
	FILE *fpList;
	const char *filename;
	char clanlist[256];
	char buf[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	CLAN_DATA *bosclan;

	first_clan = NULL;
	last_clan = NULL;

	log_string( "Loading clans..." );

	snprintf( clanlist, 256, "%s%s", CLAN_DIR, CLAN_LIST );
	if( ( fpList = FileOpen( clanlist, "r" ) ) == NULL )
	{
		perror( clanlist );
		exit( 9 );
	}

	for( ;; )
	{
		filename = feof( fpList ) ? "$" : fread_word( fpList );
		log_string( filename );
		if( filename[0] == '$' )
			break;

		if( !load_clan_file( filename ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Cannot load clan file: %s", filename );
			bug( buf, 0 );
		}
	}
	FileClose( fpList );
	log_string( " Done clans\n\rSorting clans...." );

	for( clan = first_clan; clan; clan = clan->next )
	{
		if( !clan->tmpstr || clan->tmpstr[0] == '\0' )
			continue;

		bosclan = get_clan( clan->tmpstr );
		if( !bosclan )
			continue;

		LINK( clan, bosclan->first_subclan, bosclan->last_subclan, next_subclan, prev_subclan );
		clan->mainclan = bosclan;
	}

	log_string( " Done sorting" );
	return;
}

void load_planets( )
{
	FILE *fpList;
	const char *filename;
	char planetlist[256];
	char buf[MAX_STRING_LENGTH];

	first_planet = NULL;
	last_planet = NULL;

	log_string( "Loading planets..." );

	snprintf( planetlist, 256, "%s%s", PLANET_DIR, PLANET_LIST );
	if( ( fpList = FileOpen( planetlist, "r" ) ) == NULL )
	{
		perror( planetlist );
		exit( 10 );
	}

	for( ;; )
	{
		filename = feof( fpList ) ? "$" : fread_word( fpList );
		log_string( filename );
		if( filename[0] == '$' )
			break;

		if( !load_planet_file( filename ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Cannot load planet file: %s", filename );
			bug( buf, 0 );
		}
	}
	FileClose( fpList );
	log_string( " Done planets " );
	return;
}

CMDF( do_make )
{
	send_to_char( "Huh?\r\n", ch );
	return;
}

CMDF( do_initiate )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char name[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	DESCRIPTOR_DATA *d;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !ch->pcdata->clan && !ch->pcdata->inivictim )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !ch->pcdata->clan && ch->pcdata->inivictim )
	{
		if( !str_cmp( arg, "decline" ) )
		{
			STRFREE( ch->pcdata->inivictim );
			send_to_char( "Your initiation is cleared.\r\n", ch );
			return;
		}

		ch_printf( ch, "Your target is: %s.\n\rType: <Initiate decline> to remove your initation target.\r\n",
			ch->pcdata->inivictim );
		return;
	}

	clan = ch->pcdata->clan;


	if( ( ch->pcdata && ch->pcdata->bestowments
		&& is_name( "initiation", ch->pcdata->bestowments ) )
		|| !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) )
		;
	else
	{
		send_to_char( "You aren't able to initiate new prospects.\r\n", ch );
		return;
	}

	if( arg[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: Initiate <prospect> <target>\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You're going to initiate yourself, huh?\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't initiate that!\r\n", ch );
		return;
	}

	if( victim->pcdata->inivictim )
	{
		send_to_char( "They already have a target!\r\n", ch );
		return;
	}

	if( ( victim->position ) != POS_STANDING )
	{
		act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( victim->pcdata->clan )
	{
		send_to_char( "That person is already in a gang!\r\n", ch );
		return;
	}

	if( ( !str_cmp( arg2, ch->name ) ) || ( !str_cmp( arg2, victim->name ) ) )
	{
		send_to_char( "That's not an acceptable target for their initiation!\r\n", ch );
		return;
	}

	mudstrlcpy( name, capitalize( arg2 ), MSL );

	if( !exists_player( name ) )
	{
		send_to_char( "No one exists by that name to target!\r\n", ch );
		return;
	}

	ch_printf( victim, "%s has given you the following initiation: Eliminate %s.\r\n", ch->name, name );
	victim->pcdata->inivictim = STRALLOC( name );
	victim->pcdata->iclan = QUICKLINK( ch->pcdata->clan->name );
	save_char_obj( victim );
	for( d = first_descriptor; d; d = d->next )
	{
		if( d->connected == CON_PLAYING )
		{
			if( IS_NPC( d->character ) )
				continue;
			if( !d->character->pcdata->clan )
				continue;

			if( !str_cmp( ch->pcdata->clan->name, d->character->pcdata->clan->name ) )
				ch_printf( d->character, "[GANG INFO] %s has given %s the following initiation: Eliminate %s!\r\n", ch->name,
					victim->name, name );
		}
	}
	return;
}

CMDF( do_outcast )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if( ( ch->pcdata && ch->pcdata->bestowments
		&& is_name( "outcast", ch->pcdata->bestowments ) )
		|| !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
		;
	else
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}


	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Outcast whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Kick yourself out of your own clan?\r\n", ch );
		return;
	}

	if( victim->pcdata->clan != ch->pcdata->clan )
	{
		send_to_char( "This player does not belong to your clan!\r\n", ch );
		return;
	}


	--clan->members;
	if( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
	{
		STRFREE( ch->pcdata->clan->number1 );
		ch->pcdata->clan->number1 = STRALLOC( "" );
	}
	if( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
	{
		STRFREE( ch->pcdata->clan->number2 );
		ch->pcdata->clan->number2 = STRALLOC( "" );
	}

	remove_member( victim->name, victim->pcdata->clan->shortname );
	clan->members -= 1;
	if( victim->pcdata->salary )
		victim->pcdata->salary = 0;
	victim->pcdata->clan = NULL;
	STRFREE( victim->pcdata->clan_name );
	victim->pcdata->clan_name = STRALLOC( "" );
	act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
	act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
	act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
	snprintf( buf, MAX_STRING_LENGTH, "%s has been outcast from %s!", victim->name, clan->name );
	echo_to_all( AT_MAGIC, buf, ECHOTAR_ALL );

	DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup( "" );

	save_char_obj( victim ); /* clan gets saved when pfile is saved */
	return;
}

CMDF( psetclan )
{
	char arg1[MAX_INPUT_LENGTH];
	CLAN_DATA *clan;

	argument = one_argument( argument, arg1 );

	clan = get_clan( ch->pcdata->clan_name );
	if( !clan )
	{
		send_to_char( "You aren't even in a gang.\r\n", ch );
		return;
	}

	if( str_cmp( ch->name, clan->leader ) && str_cmp( ch->name, clan->number1 ) )
	{
		send_to_char( "Only the gang leader or number one can change this information.\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Usage: setgang <field> <player/text>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		if( !str_cmp( ch->name, clan->leader ) )
		{
			send_to_char( "numberone motto colorone colortwo desc\r\n", ch );
		}
		return;
	}

	if( !str_cmp( arg1, "colorone" ) )
	{
		char clr[MSL];
		mudstrlcpy( clr, argument, MSL );
		if( strlen( clr ) > 2 )
			clr[2] = '\0';

		if( !clan->cone )
			STRFREE( clan->cone );

		clan->cone = STRALLOC( clr );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}


	if( !strcmp( arg1, "numberone" ) )
	{
		STRFREE( clan->number1 );
		clan->number1 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !str_cmp( arg1, "desc" ) )
	{
		STRFREE( clan->description );
		clan->description = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !str_cmp( arg1, "colortwo" ) )
	{
		char clr[MSL];
		mudstrlcpy( clr, argument, MSL );
		if( strlen( clr ) > 2 )
			clr[2] = '\0';

		if( !clan->ctwo )
			STRFREE( clan->ctwo );

		clan->ctwo = STRALLOC( clr );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !str_cmp( arg1, "motto" ) )
	{
		DISPOSE( clan->motto );
		clan->motto = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	do_setclan( ch, "" );
	return;

}

CMDF( do_setclan )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CLAN_DATA *clan;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) )
	{
		psetclan( ch, argument );
		return;
	}

	if( !IS_SUPREME( ch ) )
	{
		send_to_char( "Only two immortals can use this command, and I highly doubt you're one of them.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Usage: setgang <gang> <field> <player>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( " leader leader2 leader3\r\n", ch );
		send_to_char( " members board recall storage\r\n", ch );
		send_to_char( " thug hitmen drugaddict pimp scalper\r\n", ch );
		send_to_char( " funds trooper1 trooper2 jail", ch );
		send_to_char( " guard1 guard2 patrol1 patrol2 atwar ally\r\n", ch );
		send_to_char( " name filename desc shortname\r\n", ch );
		return;
	}

	clan = get_clan( arg1 );
	if( !clan )
	{
		send_to_char( "No such gang.\r\n", ch );
		return;
	}

	if( !strcmp( arg2, "leader" ) )
	{
		STRFREE( clan->leader );
		clan->leader = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !str_cmp( arg2, "motto" ) )
	{
		DISPOSE( clan->motto );
		clan->motto = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "subclan" ) )
	{
		CLAN_DATA *subclan;
		subclan = get_clan( argument );
		if( !subclan )
		{
			send_to_char( "Subclan is not a clan.\r\n", ch );
			return;
		}
		if( subclan->clan_type == CLAN_SUBCLAN || subclan->mainclan )
		{
			send_to_char( "Subclan is already part of another organization.\r\n", ch );
			return;
		}
		if( subclan->first_subclan )
		{
			send_to_char( "Subclan has subclans of its own that need removing first.\r\n", ch );
			return;
		}
		subclan->clan_type = CLAN_SUBCLAN;
		subclan->mainclan = clan;
		LINK( subclan, clan->first_subclan, clan->last_subclan, next_subclan, prev_subclan );
		save_clan( clan );
		save_clan( subclan );
		return;
	}

	if( !str_cmp( arg2, "colorone" ) )
	{
		char clr[MSL];
		mudstrlcpy( clr, argument, MSL );
		if( strlen( clr ) > 2 )
			clr[2] = '\0';

		if( !clan->cone )
			STRFREE( clan->cone );

		clan->cone = STRALLOC( clr );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !str_cmp( arg2, "colortwo" ) )
	{
		char clr[MSL];
		mudstrlcpy( clr, argument, MSL );
		if( strlen( clr ) > 2 )
			clr[2] = '\0';

		if( !clan->ctwo )
			STRFREE( clan->ctwo );

		clan->ctwo = STRALLOC( clr );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}


	if( !strcmp( arg2, "leader2" ) )
	{
		STRFREE( clan->number1 );
		clan->number1 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "leader3" ) )
	{
		STRFREE( clan->number2 );
		clan->number2 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "board" ) )
	{
		clan->board = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "members" ) )
	{
		clan->members = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "thug" ) )
	{
		clan->thug = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "hitmen" ) )
	{
		clan->hitmen = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "drugaddict" ) )
	{
		clan->drugaddict = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "pimp" ) )
	{
		clan->pimp = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "scalper" ) )
	{
		clan->scalper = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "funds" ) )
	{
		clan->funds = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "storage" ) )
	{
		clan->storeroom = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "guard1" ) )
	{
		clan->guard1 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "jail" ) )
	{
		clan->jail = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "guard2" ) )
	{
		clan->guard2 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "trooper1" ) )
	{
		clan->trooper1 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "trooper2" ) )
	{
		clan->trooper2 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}
	if( !strcmp( arg2, "patrol1" ) )
	{
		clan->patrol1 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "patrol2" ) )
	{
		clan->patrol2 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( get_trust( ch ) < LEVEL_BUILDER )
	{
		do_setclan( ch, "" );
		return;
	}

	if( !strcmp( arg2, "type" ) )
	{
		if( clan->mainclan )
		{
			UNLINK( clan, clan->mainclan->first_subclan, clan->mainclan->last_subclan, next_subclan, prev_subclan );
			clan->mainclan = NULL;
		}
		if( !str_cmp( argument, "crime" ) )
			clan->clan_type = CLAN_CRIME;
		else if( !str_cmp( argument, "crime family" ) )
			clan->clan_type = CLAN_CRIME;
		else if( !str_cmp( argument, "guild" ) )
			clan->clan_type = CLAN_GUILD;
		else
			clan->clan_type = 0;
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "name" ) )
	{
		STRFREE( clan->name );
		clan->name = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "filename" ) )
	{
		DISPOSE( clan->filename );
		clan->filename = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		write_clan_list( );
		return;
	}

	if( !strcmp( arg2, "shortname" ) )
	{
		DISPOSE( clan->shortname );
		clan->shortname = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		write_clan_list( );
		return;
	}

	if( !strcmp( arg2, "atwar" ) )
	{
		STRFREE( clan->atwar );
		clan->atwar = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	if( !strcmp( arg2, "desc" ) )
	{
		STRFREE( clan->description );
		clan->description = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_clan( clan );
		return;
	}

	do_setclan( ch, "" );
	return;
}

CMDF( do_buyposse )
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CLAN_DATA *clan;
	PLANET_DATA *planet;
	int ttype, cost, pnum;
	int MAX_THUG, MAX_HITMAN, MAX_DRUGADDICT, MAX_PIMP, MAX_SCALPER;

	argument = one_argument( argument, arg1 );

	MAX_THUG = 0;
	MAX_HITMAN = 0;
	MAX_DRUGADDICT = 0;
	MAX_PIMP = 0;
	MAX_SCALPER = 0;
	pnum = 0;

	clan = ch->pcdata->clan;
	if( clan == NULL )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: Buyposse <thug/hitmen/drugaddict/pimp/scalper>\r\n", ch );
		return;
	}
	else if( !str_cmp( arg1, "thug" ) )
		ttype = 1;
	else if( !str_cmp( arg1, "hitmen" ) )
		ttype = 2;
	else if( !str_cmp( arg1, "drugaddict" ) )
		ttype = 3;
	else if( !str_cmp( arg1, "pimp" ) )
		ttype = 4;
	else if( !str_cmp( arg1, "scalper" ) )
		ttype = 5;
	else
	{
		do_buyposse( ch, "" );
		return;
	}

	for( planet = first_planet; planet; planet = planet->next )
		if( clan == planet->governed_by )
		{
			pnum++;
		}

	MAX_THUG = pnum * 4;
	MAX_HITMAN = pnum * 3;
	MAX_DRUGADDICT = pnum * 3;
	MAX_PIMP = pnum * 2;
	MAX_SCALPER = pnum * 2;

	if( ttype == 1 )
	{
		cost = ( 50000 );
		if( clan->funds < cost )
		{
			send_to_char( "Your gang can't afford to purchase that many!\r\n", ch );
			return;
		}
		if( clan->thug >= MAX_THUG )
		{
			send_to_char( "You already have the max amount of thugs roaming the streets.\r\n", ch );
			return;
		}
		clan->thug += 1;
	}
	if( ttype == 2 )
	{
		cost = ( 200000 );
		if( clan->funds < cost )
		{
			send_to_char( "Your gang can't afford to purchase that many!\r\n", ch );
			return;
		}
		if( clan->hitmen >= MAX_HITMAN )
		{
			send_to_char( "You already have the max amount of hitmen patrolling the streets.\r\n", ch );
			return;
		}
		clan->hitmen += 1;
	}
	if( ttype == 3 )
	{
		cost = ( 100000 );
		if( clan->funds < cost )
		{
			send_to_char( "Your gang can't afford to purchase that many!\r\n", ch );
			return;
		}
		if( clan->drugaddict >= MAX_DRUGADDICT )
		{
			send_to_char( "Don't you have enough drug addicts asleep on the streets..?\r\n", ch );
			return;
		}
		clan->drugaddict += 1;
	}
	if( ttype == 4 )
	{
		cost = ( 750000 );
		if( clan->funds < cost )
		{
			send_to_char( "Your gang can't afford to purchase that many!\r\n", ch );
			return;
		}
		if( clan->pimp >= MAX_PIMP )
		{
			send_to_char( "You already have the max amount of pimps roaming the streets.\r\n", ch );
			return;
		}
		clan->pimp += 1;
	}
	if( ttype == 5 )
	{
		cost = ( 500000 );
		if( clan->funds < cost )
		{
			send_to_char( "Your gang can't afford to purchase that many!\r\n", ch );
			return;
		}
		if( clan->scalper >= MAX_SCALPER )
		{
			send_to_char( "You already have the max amount of scalpers roaming the streets.\r\n", ch );
			return;
		}
		clan->scalper += 1;
	}
	if( ttype >= 6 )
	{
		send_to_char( "Error.\r\n", ch );
		return;
	}

	clan->funds -= cost;
	snprintf( buf, MAX_STRING_LENGTH, "&GYou have purchased 1 %s for %s.\r\n",
		ttype == 1 ? "Thug" : ttype == 2 ? "Hitman" : ttype == 3 ? "Drug Dealer" : ttype == 4 ? "Pimp" : ttype ==
		5 ? "Scalper" : "", clan->name );
	send_to_char( buf, ch );
	save_clan( clan );
}

CMDF( do_setplanet )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	PLANET_DATA *planet;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Usage: setplanet <planet> <field> [value]\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( " base_value flags\r\n", ch );
		send_to_char( " name filename starsystem governed_by\r\n", ch );
		send_to_char( " import0 - import9\r\n", ch );
		send_to_char( " export0 - export9\r\n", ch );
		send_to_char( " sell0 - sell9\r\n", ch );
		send_to_char( " buy0 - buy9\r\n", ch );
		return;
	}

	planet = get_planet( arg1 );
	if( !planet )
	{
		send_to_char( "No such planet.\r\n", ch );
		return;
	}


	if( !strcmp( arg2, "name" ) )
	{
		PLANET_DATA *tplanet;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "You must choose a name.\r\n", ch );
			return;
		}
		if( ( tplanet = get_planet( argument ) ) != NULL )
		{
			send_to_char( "A planet with that name already Exists!\r\n", ch );
			return;
		}

		STRFREE( planet->name );
		planet->name = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "governed_by" ) )
	{
		CLAN_DATA *clan;
		clan = get_clan( argument );
		if( clan )
		{
			planet->governed_by = clan;
			send_to_char( "Done.\r\n", ch );
			save_planet( planet );
		}
		else
			send_to_char( "No such clan.\r\n", ch );
		return;
	}

	if( !strcmp( arg2, "import0" ) )
	{
		planet->pImport[0] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import1" ) )
	{
		planet->pImport[1] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import2" ) )
	{
		planet->pImport[2] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import3" ) )
	{
		planet->pImport[3] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import4" ) )
	{
		planet->pImport[4] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import5" ) )
	{
		planet->pImport[5] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import6" ) )
	{
		planet->pImport[6] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import7" ) )
	{
		planet->pImport[7] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import8" ) )
	{
		planet->pImport[8] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "import9" ) )
	{
		planet->pImport[9] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export0" ) )
	{
		planet->pExport[0] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export1" ) )
	{
		planet->pExport[1] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export2" ) )
	{
		planet->pExport[2] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export3" ) )
	{
		planet->pExport[3] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export4" ) )
	{
		planet->pExport[4] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export5" ) )
	{
		planet->pExport[5] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export6" ) )
	{
		planet->pExport[6] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export7" ) )
	{
		planet->pExport[7] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export8" ) )
	{
		planet->pExport[8] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export9" ) )
	{
		planet->pExport[9] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy0" ) )
	{
		planet->pBuy[0] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy1" ) )
	{
		planet->pBuy[1] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy2" ) )
	{
		planet->pBuy[2] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy3" ) )
	{
		planet->pBuy[3] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy4" ) )
	{
		planet->pBuy[4] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy5" ) )
	{
		planet->pBuy[5] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy6" ) )
	{
		planet->pBuy[6] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy7" ) )
	{
		planet->pBuy[7] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy8" ) )
	{
		planet->pBuy[8] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "buy9" ) )
	{
		planet->pBuy[9] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell0" ) )
	{
		planet->pSell[0] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "export1" ) )
	{
		planet->pExport[1] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell2" ) )
	{
		planet->pSell[2] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell3" ) )
	{
		planet->pSell[3] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell4" ) )
	{
		planet->pSell[4] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell5" ) )
	{
		planet->pSell[5] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell6" ) )
	{
		planet->pSell[6] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell7" ) )
	{
		planet->pSell[7] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell8" ) )
	{
		planet->pSell[8] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "sell9" ) )
	{
		planet->pSell[9] = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "starsystem" ) )
	{
		SPACE_DATA *starsystem;

		if( ( starsystem = planet->starsystem ) != NULL )
			UNLINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
		if( ( planet->starsystem = starsystem_from_name( argument ) ) )
		{
			starsystem = planet->starsystem;
			LINK( planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
			send_to_char( "Done.\r\n", ch );
		}
		else
			send_to_char( "No such starsystem.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "filename" ) )
	{
		PLANET_DATA *tplanet;

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "You must choose a file name.\r\n", ch );
			return;
		}
		for( tplanet = first_planet; tplanet; tplanet = tplanet->next )
		{
			if( !str_cmp( tplanet->filename, argument ) )
			{
				send_to_char( "A planet with that filename already exists!\r\n", ch );
				return;
			}
		}

		DISPOSE( planet->filename );
		planet->filename = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		write_planet_list( );
		return;
	}

	if( !strcmp( arg2, "base_value" ) )
	{
		planet->base_value = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	if( !strcmp( arg2, "flags" ) )
	{
		char farg[MAX_INPUT_LENGTH];

		argument = one_argument( argument, farg );

		if( farg[0] == '\0' )
		{
			send_to_char( "Possible flags: nocapture noquest\r\n", ch );
			return;
		}

		for( ; farg[0] != '\0'; argument = one_argument( argument, farg ) )
		{
			if( !str_cmp( farg, "nocapture" ) )
				TOGGLE_BIT( planet->flags, PLANET_NOCAPTURE );
			if( !str_cmp( farg, "noquest" ) )
				TOGGLE_BIT( planet->flags, PLANET_NOQUEST );
			else
				ch_printf( ch, "No such flag: %s\r\n", farg );
		}
		send_to_char( "Done.\r\n", ch );
		save_planet( planet );
		return;
	}

	do_setplanet( ch, "" );
	return;
}

CMDF( do_showclan )
{
	CLAN_DATA *clan;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: showclan <clan>\r\n", ch );
		return;
	}

	clan = get_clan( argument );
	if( !clan )
	{
		send_to_char( "No such clan.\r\n", ch );
		return;
	}

	ch_printf( ch, "%s      : %s\n\rFilename: %s\r\n",
		clan->clan_type == CLAN_CRIME ? "Crime Family " :
		clan->clan_type == CLAN_GUILD ? "Guild " : "Organization ", clan->name, clan->filename );
	ch_printf( ch, "Sname: %s\n\rDescription: %s\n\rLeader: %s\r\n", clan->shortname, clan->description, clan->leader );
	ch_printf( ch, "Leader2: %s\n\rLeader3: %s\n\rPKills: %6d    PDeaths: %6d\r\n",
		clan->number1, clan->number2, clan->pkills, clan->pdeaths );
	ch_printf( ch, "MKills: %6d    MDeaths: %6d\r\n", clan->mkills, clan->mdeaths );
	ch_printf( ch, "Type: %d\r\n", clan->clan_type );
	ch_printf( ch, "Members: %3d\r\n", clan->members );
	ch_printf( ch, "Board: %5d   Jail: %5d\r\n", clan->board, clan->jail );
	ch_printf( ch, "Guard1: %5d  Guard2: %5d\r\n", clan->guard1, clan->guard2 );
	ch_printf( ch, "Patrol1: %5d  Patrol2: %5d\r\n", clan->patrol1, clan->patrol2 );
	ch_printf( ch, "Trooper1: %5d  Trooper2: %5d\r\n", clan->trooper1, clan->trooper2 );
	ch_printf( ch, "Funds: %ld\r\n", clan->funds );
	return;
}

CMDF( do_showplanet )
{
	PLANET_DATA *planet;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: showplanet <planet>\r\n", ch );
		return;
	}

	planet = get_planet( argument );
	if( !planet )
	{
		send_to_char( "No such planet.\r\n", ch );
		return;
	}

	ch_printf( ch, "%s\n\rFilename: %s\r\n", planet->name, planet->filename );
	return;
}

CMDF( do_makeclan )
{
	char filename[256];
	CLAN_DATA *clan;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeclan <clan name>\r\n", ch );
		return;
	}

	/*  Otherwise it would be possible to create multiple clans
		with the same name. Bad thing... */

	set_char_color( AT_PLAIN, ch );
	clan = get_clan( argument );
	if( clan )
	{
		send_to_char( "There is already a clan with that name.\r\n", ch );
		return;
	}
	snprintf( filename, 256, "%s%s", CLAN_DIR, strlower( argument ) );

	CREATE( clan, CLAN_DATA, 1 );
	LINK( clan, first_clan, last_clan, next, prev );
	clan->next_subclan = NULL;
	clan->prev_subclan = NULL;
	clan->last_subclan = NULL;
	clan->first_subclan = NULL;
	clan->mainclan = NULL;
	clan->name = STRALLOC( argument );
	clan->description = STRALLOC( "" );
	clan->leader = STRALLOC( "" );
	clan->number1 = STRALLOC( "" );
	clan->number2 = STRALLOC( "" );
	clan->ally = STRALLOC( "" );
	clan->atwar = STRALLOC( "" );
	clan->tmpstr = STRALLOC( "" );
	clan->filename = str_dup( filename );
	clan->shortname = str_dup( "temp" );
	send_to_char( "Done. Shortname set to temp. Be sure to reset this.", ch );
}

CMDF( do_makeplanet )
{
	char filename[256];
	PLANET_DATA *planet;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeplanet <planet name>\r\n", ch );
		return;
	}

	/*  Otherwise it would be possible to create multiple clans
		with the same name. Bad thing... */

	set_char_color( AT_PLAIN, ch );
	planet = get_planet( argument );
	if( planet )
	{
		send_to_char( "There is already a planet with that name.\r\n", ch );
		return;
	}

	snprintf( filename, 256, "%s%s", PLANET_DIR, strlower( argument ) );

	CREATE( planet, PLANET_DATA, 1 );
	LINK( planet, first_planet, last_planet, next, prev );
	planet->governed_by = NULL;
	planet->next_in_system = NULL;
	planet->prev_in_system = NULL;
	planet->starsystem = NULL;
	planet->first_area = NULL;
	planet->last_area = NULL;
	planet->first_guard = NULL;
	planet->last_guard = NULL;
	planet->name = STRALLOC( argument );
	planet->filename = str_dup( filename );
	planet->flags = 0;
}
/*
CMDF( do_clans )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   int count = 0;
   int pCount = 0;
   int support;
   long revenue;

   if( !argument || argument[0] == '\0' )
   {
	  send_to_char( "Syntax: Gangs <Clan Name/All>\r\n", ch );
	  return;
   }

   if( !str_cmp( argument, "load" ) )
   {
	  load_gang(  );
	  return;
   }

   if( !str_cmp( argument, "all" ) )
   {

	  for( clan = first_clan; clan; clan = clan->next )
	  {
		 if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
			continue;

		 pCount = 0;
		 support = 0;
		 revenue = 0;

		 ch_printf( ch, "\r\n                          %s %s ^x &O<&Y[&z%s&Y]&O> &w%s %s ^z&w\r\n", clan->cone, clan->ctwo,
					clan->name, clan->ctwo, clan->cone );
		 ch_printf( ch, "&c[&gDesc&c] &c%s\r\n", clan->description );
		 ch_printf( ch, "&c[&gCommand&c]&G   &GLeader&g: &c%-12s  &GSecond&g: &c%s\r\n", clan->leader, clan->number1 );
		 ch_printf( ch, "&c[&gMotto&c] &c%s\r\n", clan->motto );

		 ch_printf( ch, "&g------------------------------------------------------------------------\r\n" );
		 count++;
	  }
	  return;
   }

   if( ( clan = get_clan( argument ) ) == NULL )
   {
	  send_to_char( "No such gang.\r\n", ch );
	  return;
   }

   for( planet = first_planet; planet; planet = planet->next )
	  if( clan == planet->governed_by )
	  {
		 pCount++;
	  }



   ch_printf( ch, "\r\n                          %s %s ^z &O<&Y[&z%s&Y]&O> &w%s %s ^z&w\r\n", clan->cone, clan->ctwo,
			  clan->name, clan->ctwo, clan->cone );
   ch_printf( ch, "&c[&gDesc&c] &c%s\r\n", clan->description );
   ch_printf( ch, "&c[&gCommand&c]&G   &GLeader&g: &c%-12s  &GSecond&g: &c%s\r\n", clan->leader, clan->number1 );
   ch_printf( ch, "&c[&gMotto&c] &c%s\r\n", clan->motto );
   ch_printf( ch, "&c[&gInformation&c] &GMembers&g:&c %-2d   &GSuits&g:&c %-3d     &GFunds&g:&c %d\r\n",
			  clan->members, clan->spacecraft, clan->funds );
   ch_printf( ch, "&c[&gConflicts&c]&G %s ", clan->atwar );
   send_to_char( "\r\n", ch );
   ch_printf( ch, "&c[&gAlliances&c]&G %s ", clan->ally );
   send_to_char( "\r\n", ch );
   ch_printf( ch, "&c[&gPosse&c]       &GThugs&g: &c%-2d          &GHitmen&g: &c%-2d      &GDrug Addicts&g: &c%d\r\n",
			  clan->thug, clan->hitmen, clan->drugaddict );
   ch_printf( ch, "&c[&gHustlers&c]    &GPimps&g: &c%-2d        &GScalpers&g: &c%-2d\r\n", clan->pimp, clan->scalper );
   ch_printf( ch,
			  "&c[&gArea Info&c]    &GTurf&g: &c%-2d         &GRespect&g:&c 0      &GIncome&g: &c%d   &GBases&g: &c%d\r\n",
			  pCount, clan->income, clan->bases );


   ch_printf( ch, "&g------------------------------------------------------------------------\r\n" );

}
*/

CMDF( do_clans )
{
	CLAN_DATA *clan;
	//   OBJ_DATA *datapad;
	PLANET_DATA *planet;
	int count = 0;
	int pCount = 0;
	int support;
	long revenue;

	/*   if( mud_data.DATAPAD_REQUIRED == 1 )
	   {
		  if( !IS_IMMORTAL( ch ) )
		  {
			 datapad = get_eq_char( ch, WEAR_HOLD );
			 if( datapad && !( datapad->item_type == ITEM_DATAPAD ) )
				datapad = NULL;

			 if( !datapad && !IS_SET( ch->pcdata->cyber, CYBER_DATAPAD ) )
			 {
				send_to_char( "&RYou would need to be holding a datapad to access that kind of information.\r\n", ch );
				return;
			 }

			 if( !IS_SET( ch->pcdata->cyber, CYBER_DATAPAD )
				 && ( datapad->datapad_setting == DATAPAD_OFF || datapad->datapad_setting == DATAPAD_RECHARGE ) )
			 {
				send_to_char( "&RYou will need to turn your datapad on first.\r\n", ch );
				return;
			 }
		  }
		  act( AT_LBLUE, "$n checks $s CDI.", ch, NULL, NULL, TO_NOTVICT );
		  send_to_char
			 ( "&wYou log into the galactic network with your datapad.\n\rAccessing .......\n\rConnected.\r\n&B------------------------ &CGalactic Net Database&$
			   ch );
	   }*/

	if( argument[0] == '\0' )
	{
		ch_printf( ch, "&R  ___________________________________________________________\r\n" );
		ch_printf( ch, "&R /                                                           \\\r\n" );
		ch_printf( ch, "&R|&B-------------------------------------------------------------&R|\r\n" );
		ch_printf( ch, "&R| &PPublic Files &p- &WOrganizations&R                                |\r\n" );
		ch_printf( ch, "&R|&B-------------------------------------------------------------&R|\r\n" );
		ch_printf( ch, "&R|       &GName             Leader     First       Second        &R|&W\r\n" );
		ch_printf( ch, "&R|   ------------        -------     -----       ------        |\r\n" );
		for( clan = first_clan; clan; clan = clan->next )
		{
			if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
				continue;

			pCount = 0;
			support = 0;
			revenue = 0;

			for( planet = first_planet; planet; planet = planet->next )
				if( clan == planet->governed_by )
				{
					support += planet->pop_support;
					pCount++;
					revenue += get_taxes( planet );
				}

			if( pCount > 1 )
				support /= pCount;
			ch_printf( ch, "&R|&W   %-20s %-10s %-10s %-10s     &R|\r\n", clan->name,
				clan->leader, clan->number1, clan->number2 );


			/*      if( clan->first_subclan )
				  {
					 CLAN_DATA *subclan;

					 for( subclan = clan->first_subclan; subclan; subclan = subclan->next_subclan )
						ch_printf( ch,
								   "&B|  &G&W%-18s&G&B|&W %-10s &G&B|&W %-10s &G&B|&W %-9s &G&B|&W %3d &G&B|&Y %12.12s&W&C &B|\r\n",
								   subclan->name, subclan->leader, subclan->number1, subclan->number2, subclan->members,
								   num_punct( subclan->funds ) );
				  }*/
			count++;
		}
		ch_printf( ch, "&R|                                                             |\r\n" );
		ch_printf( ch, "&R|&B-------------------------------------------------------------&R|\r\n" );
		ch_printf( ch, "&R|                                                             |\r\n" );
		ch_printf( ch, "&R|       &WFor more information type: CLANS <CLAN NAME>          &R|\r\n" );
		ch_printf( ch, "&R \\___________________________________________________________/\r\n" );


		/*   ch_printf( ch,
					  "&G&B+--------------------&B+------------&B+------------&B+-----------&B+&W\r\n" );

		   for( clan = first_clan; clan; clan = clan->next )
		   {
			  if( clan->clan_type != CLAN_CRIME && clan->clan_type != CLAN_GUILD )
				 continue;

			  ch_printf( ch, "&G&B|&w%-20s&G&B|&W %-10s &G&B|&W %-10s &G&B|&W %-9s &G&B|&W %3d &G&B|&Y %12.12s&W&C &B|\r\n",
						 clan->name, clan->leader, clan->number1, clan->number2, clan->members, num_punct( clan->funds ) );
			  count++;
		   }
		*/
		if( !count )
		{
			set_char_color( AT_BLOOD, ch );
			send_to_char( "There are no organizations currently formed.\r\n", ch );
		}

		//   ch_printf( ch, "&B+--------------------&B+------------&B+------------&B+-----------&B+&W\r\n" );

		/*   if( mud_data.DATAPAD_REQUIRED == 1 )
		   {
			  send_to_char
				 ( "\r\n&B------------------------ &CGalactic Net Database&B ------------------------&w\r\n&wDisconnected.\r\n",
				   ch );

		   }*/
		return;
	}
	clan = get_clan( argument );
	if( !clan )
	{
		send_to_char( "That is not an organization. Type CLANS for a list.\r\n", ch );
		return;
	}

	if( !str_cmp( argument, clan->name ) )
	{
		ch_printf( ch, "&R  ___________________________________________________________\r\n" );
		ch_printf( ch, "&R /                                                           \\\r\n" );
		/*            ch_printf( ch, "&R|&w.....Connecting to the Intergalactic Information System......&R|\r\n" );
					ch_printf( ch, "&R|&w.....Connected.                                              &R|\r\n" );
					ch_printf( ch, "&R|&w.....Accessing Public Files.......                           &R|\r\n" );*/
		ch_printf( ch, "&R|&B-------------------------------------------------------------&R|\r\n" );
		ch_printf( ch, "&R|&PClan Name&p: &w%-50.50s&R|\r\n", clan->name );
		ch_printf( ch, "&R|&B-------------------------------------------------------------&R|\r\n" );
		ch_printf( ch, "&R|&BLeadership                                                   &R|\r\n" );
		ch_printf( ch, "&R|--------------------                                         |\r\n" );
		ch_printf( ch, "&R|&GLeader&g:      &w%-48.48s&R|\r\n", clan->leader );
		ch_printf( ch, "&R|&G1st Officer&g: &w%-48.48s&R|\r\n", clan->number1 );
		ch_printf( ch, "&R|&G2nd Officer&g: &w%-48.48s&R|\r\n", clan->number2 );
		ch_printf( ch, "&R|--------------------                                         |\r\n" );
		ch_printf( ch, "&R|&BOrganization Info                                            &R|\r\n" );
		ch_printf( ch, "&R|--------------------                                         |\r\n" );
		ch_printf( ch, "&R|&GMission&g: &w%-52.52s&R|\r\n", clan->description );
		ch_printf( ch, "&R|&GAt war with &R%-49.49s&R|\r\n", clan->atwar );
		if( clan->first_subclan )
		{
			CLAN_DATA *subclan;

			ch_printf( ch, "&R|--------------------                                         |\r\n" );
			ch_printf( ch, "&R|&BDivisions                                                    &R|\r\n" );
			ch_printf( ch, "&R|--------------------                                         |\r\n" );

			for( subclan = clan->first_subclan; subclan; subclan = subclan->next_subclan )
			{
				ch_printf( ch, "&R|&W%-61.61s&R|\r\n", subclan->name );
				count++;
			}
		}
		ch_printf( ch, "&R|                                                             |\r\n" );
		ch_printf( ch, "&R \\___________________________________________________________/\r\n" );
		return;
	}

}

/*
 * New planets command - Amras
 */
CMDF( do_planets )
{
	PLANET_DATA *planet;
	int count = 0;

	set_char_color( AT_WHITE, ch );
	send_to_char( "\r\n&B[     &WTurf         &b|   &W Of    &B]  &YRespect  \r\n", ch );
	send_to_char( "&B[&b==========================================&B]\r\n", ch );

	for( planet = first_planet; planet; planet = planet->next )
	{
		if( planet->governed_by && !str_cmp( planet->governed_by->name, "White Fang" ) )
		{
			ch_printf( ch, "&B[&W%-17s &b: &W%10s&B] &Y%.1f\r\n",
				planet->name, planet->governed_by ? planet->governed_by->name : "No Owner", planet->pop_support );
			count++;
		}
	}

	for( planet = first_planet; planet; planet = planet->next )
	{
		if( planet->governed_by && !str_cmp( planet->governed_by->name, "RAID" ) )
		{
			ch_printf( ch, "&B[&W%-17s &b: &W%10s&B] &Y%.1f\r\n",
				planet->name, planet->governed_by ? planet->governed_by->name : "No Owner", planet->pop_support );
			count++;
		}
	}

	for( planet = first_planet; planet; planet = planet->next )
	{
		if( !planet->governed_by )
		{
			ch_printf( ch, "&B[&W%-17s &b: &W%10s&B] &Y%.1f\r\n",
				planet->name, planet->governed_by ? planet->governed_by->name : "No Owner", planet->pop_support );

			count++;
		}
	}

	if( !count )
	{
		set_char_color( AT_BLOOD, ch );
		send_to_char( "There are no planets currently formed.\r\n", ch );
	}

}

CMDF( do_shove )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int exit_dir;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	bool nogo;
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *fromroom;
	SHIP_DATA *ship;
	int chance;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );


	if( arg[0] == '\0' )
	{
		send_to_char( "Shove whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You shove yourself around, to no avail.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		return;
	}

	if( ( victim->position ) != POS_STANDING )
	{
		act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if( arg2[0] == '\0' )
	{
		send_to_char( "Shove them in which direction?\r\n", ch );
		return;
	}

	exit_dir = get_dir( arg2 );
	if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
	{
		send_to_char( "That character cannot be shoved right now.\r\n", ch );
		return;
	}
	victim->position = POS_SHOVE;
	nogo = false;
	if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
	{
		if( !strcmp( arg2, "in" ) )
		{
			if( !argument || argument[0] == '\0' )
			{
				send_to_char( "Drag him into what?\r\n", ch );
				return;
			}

			if( ( ship = ship_in_room( ch->in_room, argument ) ) == NULL )
			{
				act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
				return;
			}

			if( xIS_SET( ch->act, ACT_MOUNTED ) )
			{
				act( AT_PLAIN, "You can't go in there riding THAT.", ch, NULL, argument, TO_CHAR );
				return;
			}

			fromroom = ch->in_room;

			if( ( to_room = get_room_index( ship->entrance ) ) != NULL )
			{
				if( !ship->hatchopen )
				{
					send_to_char( "&RThe hatch is closed!\r\n", ch );
					return;
				}

				if( to_room->tunnel > 0 )
				{
					CHAR_DATA *ctmp;
					int count = 0;

					for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
						if( count + 2 >= to_room->tunnel )
						{
							send_to_char( "There is no room for you both in there.\r\n", ch );
							return;
						}
				}
				if( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 )
				{
					send_to_char( "&rThat ship has already started launching!\r\n", ch );
					return;
				}

				act( AT_PLAIN, "$n enters $T.", ch, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You enter $T.", ch, NULL, ship->name, TO_CHAR );
				char_from_room( ch );
				char_to_room( ch, to_room );
				act( AT_PLAIN, "$n enters the ship.", ch, NULL, argument, TO_ROOM );
				do_look( ch, "auto" );

				act( AT_PLAIN, "$n enters $T.", victim, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You enter $T.", victim, NULL, ship->name, TO_CHAR );
				char_from_room( victim );
				char_to_room( victim, to_room );
				act( AT_PLAIN, "$n enters the ship.", victim, NULL, argument, TO_ROOM );
				do_look( victim, "auto" );
				victim->position = POS_STANDING;
				return;
			}
			else
			{
				send_to_char( "That ship has no entrance!\r\n", ch );
				return;
			}
		}
		if( !strcmp( arg2, "out" ) )
		{
			fromroom = ch->in_room;

			if( ( ship = ship_from_entrance( fromroom->vnum ) ) == NULL )
			{
				send_to_char( "I see no exit here.\r\n", ch );
				return;
			}

			if( xIS_SET( ch->act, ACT_MOUNTED ) )
			{
				act( AT_PLAIN, "You can't go out there riding THAT.", ch, NULL, argument, TO_CHAR );
				return;
			}

			if( ship->lastdoc != ship->location )
			{
				send_to_char( "&rMaybe you should wait until the ship lands.\r\n", ch );
				return;
			}
			/*
				if ( ship->shipstate != SHIP_LANDED && ship->shipstate != SHIP_DISABLED )
				{
					send_to_char("&rPlease wait till the ship is properly docked.\r\n",ch);
					return;
				}
			*/
			if( !ship->hatchopen )
			{
				send_to_char( "&RYou need to open the hatch first", ch );
				return;
			}

			if( ( to_room = get_room_index( ship->location ) ) != NULL )
			{

				if( to_room->tunnel > 0 )
				{
					CHAR_DATA *ctmp;
					int count = 0;

					for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
						if( count + 2 >= to_room->tunnel )
						{
							send_to_char( "There is no room for you both in there.\r\n", ch );
							return;
						}
				}
				if( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 )
				{
					send_to_char( "&rThat ship has already started launching!\r\n", ch );
					return;
				}

				act( AT_PLAIN, "$n exits the ship.", ch, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You exits the ship.", ch, NULL, ship->name, TO_CHAR );
				char_from_room( ch );
				char_to_room( ch, to_room );
				act( AT_PLAIN, "$n exits $T.", ch, NULL, ship->name, TO_ROOM );
				do_look( ch, "auto" );

				act( AT_PLAIN, "$n exits the ship.", victim, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You exits the ship.", victim, NULL, ship->name, TO_CHAR );
				char_from_room( victim );
				char_to_room( victim, to_room );
				act( AT_PLAIN, "$n exits $T.", victim, NULL, ship->name, TO_ROOM );
				do_look( victim, "auto" );
				victim->position = POS_STANDING;
				return;
			}
			else
			{
				send_to_char( "That ship has no entrance!\r\n", ch );
				return;
			}
		}
		nogo = true;
	}
	else if( IS_SET( pexit->exit_info, EX_CLOSED )
		//    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
		|| IS_SET( pexit->exit_info, EX_NOPASSDOOR ) )
		nogo = true;
	if( nogo )
	{
		send_to_char( "There's no exit in that direction.\r\n", ch );
		victim->position = POS_STANDING;
		return;
	}
	to_room = pexit->to_room;

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can only shove player characters.\r\n", ch );
		return;
	}

	if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
	{
		send_to_char( "That character cannot enter that area.\r\n", ch );
		victim->position = POS_STANDING;
		return;
	}

	chance = 50;

	/* Add 3 points to chance for every str point above 15, subtract for below 15 */

	chance += ( ( get_curr_str( ch ) - 15 ) * 3 );

	chance += ( ch->top_level - victim->top_level );

	/* Debugging purposes - show percentage for testing */

	/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
	*/

	if( chance < number_percent( ) )
	{
		send_to_char( "You failed.\r\n", ch );
		victim->position = POS_STANDING;
		return;
	}
	act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
	move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
	if( !char_died( victim ) )
		victim->position = POS_STANDING;
	WAIT_STATE( ch, 12 );
	/*
	 * Remove protection from shove/drag if char shoves -- Blodkai
	 */
	if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
		add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

CMDF( do_drag )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int exit_dir;
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *to_room;
	bool nogo;
	int chance;
	ROOM_INDEX_DATA *fromroom;
	SHIP_DATA *ship;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( arg[0] == '\0' )
	{
		send_to_char( "Drag whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "You take yourself by the scruff of your neck, but go nowhere.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can only drag player characters.\r\n", ch );
		return;
	}

	if( victim->fighting )
	{
		send_to_char( "You try, but can't get close enough.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		send_to_char( "Drag them in which direction?\r\n", ch );
		return;
	}

	exit_dir = get_dir( arg2 );

	if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
	{
		send_to_char( "That character cannot be dragged right now.\r\n", ch );
		return;
	}

	nogo = false;
	if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
	{
		if( !strcmp( arg2, "in" ) )
		{
			if( !argument || argument[0] == '\0' )
			{
				send_to_char( "Drag him into what?\r\n", ch );
				return;
			}

			if( ( ship = ship_in_room( ch->in_room, argument ) ) == NULL )
			{
				act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
				return;
			}

			if( xIS_SET( ch->act, ACT_MOUNTED ) )
			{
				act( AT_PLAIN, "You can't go in there riding THAT.", ch, NULL, argument, TO_CHAR );
				return;
			}

			fromroom = ch->in_room;

			if( ( to_room = get_room_index( ship->entrance ) ) != NULL )
			{
				if( !ship->hatchopen )
				{
					send_to_char( "&RThe hatch is closed!\r\n", ch );
					return;
				}

				if( to_room->tunnel > 0 )
				{
					CHAR_DATA *ctmp;
					int count = 0;

					for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
						if( count + 2 >= to_room->tunnel )
						{
							send_to_char( "There is no room for you both in there.\r\n", ch );
							return;
						}
				}
				if( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 )
				{
					send_to_char( "&rThat ship has already started launching!\r\n", ch );
					return;
				}

				act( AT_PLAIN, "$n enters $T.", ch, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You enter $T.", ch, NULL, ship->name, TO_CHAR );
				char_from_room( ch );
				char_to_room( ch, to_room );
				act( AT_PLAIN, "$n enters the ship.", ch, NULL, argument, TO_ROOM );
				do_look( ch, "auto" );

				act( AT_PLAIN, "$n enters $T.", victim, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You enter $T.", victim, NULL, ship->name, TO_CHAR );
				char_from_room( victim );
				char_to_room( victim, to_room );
				act( AT_PLAIN, "$n enters the ship.", victim, NULL, argument, TO_ROOM );
				do_look( victim, "auto" );
				return;
			}
			else
			{
				send_to_char( "That ship has no entrance!\r\n", ch );
				return;
			}
		}
		if( !strcmp( arg2, "out" ) )
		{
			fromroom = ch->in_room;

			if( ( ship = ship_from_entrance( fromroom->vnum ) ) == NULL )
			{
				send_to_char( "I see no exit here.\r\n", ch );
				return;
			}

			if( xIS_SET( ch->act, ACT_MOUNTED ) )
			{
				act( AT_PLAIN, "You can't go out there riding THAT.", ch, NULL, argument, TO_CHAR );
				return;
			}

			if( ship->lastdoc != ship->location )
			{
				send_to_char( "&rMaybe you should wait until the ship lands.\r\n", ch );
				return;
			}
			/*
				if ( ship->shipstate != SHIP_LANDED && ship->shipstate != SHIP_DISABLED )
				{
					send_to_char("&rPlease wait till the ship is properly docked.\r\n",ch);
					return;
				}
			*/
			if( !ship->hatchopen )
			{
				send_to_char( "&RYou need to open the hatch first", ch );
				return;
			}

			if( ( to_room = get_room_index( ship->location ) ) != NULL )
			{

				if( to_room->tunnel > 0 )
				{
					CHAR_DATA *ctmp;
					int count = 0;

					for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
						if( count + 2 >= to_room->tunnel )
						{
							send_to_char( "There is no room for you both in there.\r\n", ch );
							return;
						}
				}
				if( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 )
				{
					send_to_char( "&rThat ship has already started launching!\r\n", ch );
					return;
				}

				act( AT_PLAIN, "$n exits the ship.", ch, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You exits the ship.", ch, NULL, ship->name, TO_CHAR );
				char_from_room( ch );
				char_to_room( ch, to_room );
				act( AT_PLAIN, "$n exits $T.", ch, NULL, ship->name, TO_ROOM );
				do_look( ch, "auto" );

				act( AT_PLAIN, "$n exits the ship.", victim, NULL, ship->name, TO_ROOM );
				act( AT_PLAIN, "You exits the ship.", victim, NULL, ship->name, TO_CHAR );
				char_from_room( victim );
				char_to_room( victim, to_room );
				act( AT_PLAIN, "$n exits $T.", victim, NULL, ship->name, TO_ROOM );
				do_look( victim, "auto" );
				return;
			}
			else
			{
				send_to_char( "That ship has no entrance!\r\n", ch );
				return;
			}
		}
		nogo = true;
	}
	else if( IS_SET( pexit->exit_info, EX_CLOSED )
		//    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
		|| IS_SET( pexit->exit_info, EX_NOPASSDOOR ) )
		nogo = true;
	if( nogo )
	{
		send_to_char( "There's no exit in that direction.\r\n", ch );
		return;
	}

	to_room = pexit->to_room;

	if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
	{
		send_to_char( "That character cannot enter that area.\r\n", ch );
		return;
	}

	chance = 50;


	/*
	sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
	*/
	if( chance < number_percent( ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}
	if( victim->position < POS_STANDING )
	{
		short temp;

		temp = victim->position;
		victim->position = POS_DRAG;
		act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
		act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
		move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
		if( !char_died( victim ) )
			victim->position = temp;
		/* Move ch to the room too.. they are doing dragging - Scryn */
		move_char( ch, get_exit( ch->in_room, exit_dir ), 0 );
		WAIT_STATE( ch, 12 );
		return;
	}
	send_to_char( "You cannot do that to someone who is standing.\r\n", ch );
	return;
}


/* void do_enlist( CHAR_DATA *ch, char *argument )
{

		CLAN_DATA *clan;

	if ( IS_NPC(ch) || !ch->pcdata )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

		if ( ch->pcdata->clan )
		{
			ch_printf( ch , "You will have to resign from %s before you can join a new organization.\r\n", ch->pcdata->clan->name );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags , ROOM_R_RECRUIT ) )
		   clan = get_clan( "The New Republic" );
		else if ( IS_SET( ch->in_room->room_flags , ROOM_E_RECRUIT ) )
		   clan = get_clan( "The Empire" );
		else
		{
			send_to_char( "You don't seem to be in a recruitment office.\r\n", ch );
		return;
		}

		if ( !clan )
		{
			send_to_char( "They don't seem to be recruiting right now.\r\n", ch );
		return;
		}

		++clan->members;

	STRFREE( ch->pcdata->clan_name );
	ch->pcdata->clan_name = QUICKLINK( clan->name );
	ch->pcdata->clan = clan;
	ch_printf( ch, "Welcome to %s.\r\n", clan->name );

		save_clan ( clan );
	return;

}
*/
CMDF( do_resign )
{

	CLAN_DATA *clan;
	long lose_exp;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if( clan == NULL )
	{
		send_to_char( "You have to join an organization before you can quit it.\r\n", ch );
		return;
	}

	if( !str_cmp( ch->name, ch->pcdata->clan->leader ) )
	{
		ch_printf( ch, "You can't resign from %s ... you are the leader!\r\n", clan->name );
		return;
	}

	--clan->members;
	if( !str_cmp( ch->name, ch->pcdata->clan->number1 ) )
	{
		STRFREE( ch->pcdata->clan->number1 );
		ch->pcdata->clan->number1 = STRALLOC( "" );
	}
	if( !str_cmp( ch->name, ch->pcdata->clan->number2 ) )
	{
		STRFREE( ch->pcdata->clan->number2 );
		ch->pcdata->clan->number2 = STRALLOC( "" );
	}

	remove_member( ch->name, ch->pcdata->clan->shortname );
	clan->members -= 1;
	ch->pcdata->clan = NULL;
	STRFREE( ch->pcdata->clan_name );
	ch->pcdata->clan_name = STRALLOC( "" );
	act( AT_MAGIC, "You resign your position in $t", ch, clan->name, NULL, TO_CHAR );
	snprintf( buf, MAX_STRING_LENGTH, "%s has quit %s!", ch->name, clan->name );
	echo_to_all( AT_MAGIC, buf, ECHOTAR_ALL );

	lose_exp = UMAX( ch->experience[DIPLOMACY_ABILITY] - exp_level( ch->skill_level[DIPLOMACY_ABILITY] ), 0 );
	ch_printf( ch, "You lose %ld diplomacy experience.\r\n", lose_exp );
	ch->experience[DIPLOMACY_ABILITY] -= lose_exp;

	DISPOSE( ch->pcdata->bestowments );
	ch->pcdata->bestowments = str_dup( "" );

	if( ch->pcdata->salary )
		ch->pcdata->salary = 0;

	save_char_obj( ch ); /* clan gets saved when pfile is saved */

	return;

}

CMDF( do_clan_withdraw )
{
	CLAN_DATA *clan;
	long amount;

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "You don't seem to belong to an organization to withdraw funds from...\r\n", ch );
		return;
	}

	if( !ch->in_room || !xIS_SET( ch->in_room->room_flags, ROOM_BANK ) )
	{
		send_to_char( "You must be in a bank to do that!\r\n", ch );
		return;
	}

	if( ( ch->pcdata && ch->pcdata->bestowments
		&& is_name( "withdraw", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, ch->pcdata->clan->leader ) )
		;
	else
	{
		send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.", ch );
		return;
	}

	clan = ch->pcdata->clan;

	amount = atoi( argument );

	if( !amount )
	{
		send_to_char( "How much would you like to withdraw?\r\n", ch );
		return;
	}

	if( amount > clan->funds )
	{
		ch_printf( ch, "%s doesn't have that much!\r\n", clan->name );
		return;
	}

	if( amount < 0 )
	{
		ch_printf( ch, "Nice try...\r\n" );
		return;
	}

	ch_printf( ch, "You withdraw %ld dollars from %s's funds.\r\n", amount, clan->name );

	clan->funds -= amount;
	ch->gold += amount;
	save_clan( clan );
	save_char_obj( ch );
}


CMDF( do_clan_donate )
{
	CLAN_DATA *clan;
	long amount;

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "You don't seem to belong to an organization to donate to...\r\n", ch );
		return;
	}

	if( !ch->in_room || !xIS_SET( ch->in_room->room_flags, ROOM_BANK ) )
	{
		send_to_char( "You must be in a bank to do that!\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	amount = atoi( argument );

	if( !amount )
	{
		send_to_char( "How much would you like to donate?\r\n", ch );
		return;
	}

	if( amount < 0 )
	{
		ch_printf( ch, "Nice try...\r\n" );
		return;
	}

	if( amount > ch->gold )
	{
		send_to_char( "You don't have that much!\r\n", ch );
		return;
	}

	ch_printf( ch, "You donate %ld dollars to %s's funds.\r\n", amount, clan->name );

	clan->funds += amount;
	ch->gold -= amount;
	save_clan( clan );
	save_char_obj( ch );
}

CMDF( do_newclan )
{
	send_to_char( "This command is being recycled to conserve thought.\r\n", ch );
	return;
}

CMDF( do_appoint )
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( str_cmp( ch->name, ch->pcdata->clan->leader ) )
	{
		send_to_char( "Only your leader can do that!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Useage: appoint <name> < first | second >\r\n", ch );
		return;
	}

	if( !str_cmp( argument, "first" ) )
	{
		if( ch->pcdata->clan->number1 && str_cmp( ch->pcdata->clan->number1, "" ) )
		{
			send_to_char( "You already have someone in that position ... demote them first.\r\n", ch );
			return;
		}

		STRFREE( ch->pcdata->clan->number1 );
		ch->pcdata->clan->number1 = STRALLOC( arg );
	}
	else if( !str_cmp( argument, "second" ) )
	{
		if( ch->pcdata->clan->number2 && str_cmp( ch->pcdata->clan->number2, "" ) )
		{
			send_to_char( "You already have someone in that position ... demote them first.\r\n", ch );
			return;
		}

		STRFREE( ch->pcdata->clan->number2 );
		ch->pcdata->clan->number2 = STRALLOC( arg );
	}
	else
		do_appoint( ch, "" );
	save_clan( ch->pcdata->clan );

}

CMDF( do_demote )
{

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	if( !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( str_cmp( ch->name, ch->pcdata->clan->leader ) )
	{
		send_to_char( "Only your leader can do that!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Demote who?\r\n", ch );
		return;
	}

	if( !str_cmp( argument, ch->pcdata->clan->number1 ) )
	{
		send_to_char( "Player Demoted!", ch );

		STRFREE( ch->pcdata->clan->number1 );
		ch->pcdata->clan->number1 = STRALLOC( "" );
	}
	else if( !str_cmp( argument, ch->pcdata->clan->number2 ) )
	{
		send_to_char( "Player Demoted!", ch );

		STRFREE( ch->pcdata->clan->number2 );
		ch->pcdata->clan->number2 = STRALLOC( "" );
	}
	else
	{
		send_to_char( "They seem to have been demoted already.\r\n", ch );
		return;
	}
	save_clan( ch->pcdata->clan );

}

CMDF( do_capture )
{
	CLAN_DATA *clan;
	DESCRIPTOR_DATA *d;
	PLANET_DATA *planet;
	PLANET_DATA *cPlanet;
	float support = 0.0;
	int pCount = 0;
	char buf[MAX_STRING_LENGTH];


	if( !ch->in_room || !ch->in_room->area )
		return;

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "huh?\r\n", ch );
		return;
	}

	if( !ch->pcdata->clan )
	{
		send_to_char( "You need to be a member of an organization to do that!\r\n", ch );
		return;
	}

	if( ch->pcdata->clan->mainclan )
		clan = ch->pcdata->clan->mainclan;
	else
		clan = ch->pcdata->clan;

	if( xIS_SET( ch->in_room->room_flags, ROOM_BASE ) )
	{
		AREA_DATA *tarea;
		tarea = ch->in_room->area;

		if( !xIS_SET( ch->in_room->room_flags, ROOM_CAPTURE ) )
		{
			send_to_char( "You have to be in the control room for this!\r\n", ch );
			return;
		}
		if( tarea->author == clan->name )
		{
			send_to_char( "Your gang already owns this base!\r\n", ch );
			return;
		}

		ch_printf( ch, "This base is now under control of %s!.\r\n", clan->name );
		STRFREE( tarea->owned_by );
		tarea->owned_by = STRALLOC( clan->name );
		/*
			 for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
			 {
			if ( (room = get_room_index( vnum )) == NULL )
			  continue;

			 }
		*/

		for( d = first_descriptor; d; d = d->next )
		{
			if( d->connected == CON_PLAYING )
			{
				if( IS_NPC( d->character ) )
					continue;
				//      if ( ch == d->character )
				//        continue;
				if( !d->character->pcdata->clan )
					continue;

				if( !str_cmp( ch->pcdata->clan->name, d->character->pcdata->clan->name ) )
					ch_printf( d->character, "[GANG INFO] %s secured us a base!\r\n", ch->name );
			}
		}

	}

	if( ( planet = ch->in_room->area->planet ) == NULL )
	{
		send_to_char( "You must be on a planet to capture it.\r\n", ch );
		return;
	}

	if( clan == planet->governed_by )
	{
		send_to_char( "Your organization already controls this planet.\r\n", ch );
		return;
	}

	if( planet->starsystem )
	{
		SHIP_DATA *ship;
		CLAN_DATA *sClan;

		for( ship = planet->starsystem->first_ship; ship; ship = ship->next_in_starsystem )
		{
			sClan = get_clan( ship->owner );
			if( !sClan )
				continue;
			if( sClan->mainclan )
				sClan = sClan->mainclan;
			if( sClan == planet->governed_by )
			{
				send_to_char( "A planet cannot be captured while protected by orbiting spacecraft.\r\n", ch );
				return;
			}
		}
	}

	if( IS_SET( planet->flags, PLANET_NOCAPTURE ) )
	{
		send_to_char( "This planet cannot be captured.\r\n", ch );
		return;
	}

	if( planet->pop_support > 0 )
	{
		send_to_char( "The people still respect the turf owners right now.\r\n", ch );
		return;
	}

	for( cPlanet = first_planet; cPlanet; cPlanet = cPlanet->next )
		if( clan == cPlanet->governed_by )
		{
			pCount++;
			support += cPlanet->pop_support;
		}

	if( support < 0 )
	{
		send_to_char
		( "There is not enough popular support for your organization!\n\rTry improving loyalty on the planets that you already control.\r\n",
			ch );
		return;
	}

	planet->governed_by = clan;
	//   area->owned_by    = clan;
	planet->pop_support = 50;

	snprintf( buf, MAX_STRING_LENGTH, "%s has been captured for %s by %s!", planet->name, clan->name, ch->name );
	echo_to_all( AT_RED, buf, 0 );
	snprintf( buf, MAX_STRING_LENGTH, "%s captured %s.", ch->name, planet->name );
	log_string( buf );

	save_planet( planet );

	return;
}

CMDF( do_empower )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if( ( ch->pcdata && ch->pcdata->bestowments
		&& is_name( "withdraw", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
		;
	else
	{
		send_to_char( "You clan hasn't seen fit to bestow that ability to you!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( arg[0] == '\0' )
	{
		send_to_char( "Empower whom to do what?\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Nice try.\r\n", ch );
		return;
	}

	if( victim->pcdata->clan != ch->pcdata->clan )
	{
		send_to_char( "This player does not belong to your clan!\r\n", ch );
		return;
	}



	if( !victim->pcdata->bestowments )
		victim->pcdata->bestowments = str_dup( "" );

	if( !victim->pcdata->bestowments )
		victim->pcdata->bestowments = str_dup( "" );

	if( arg2[0] == '\0' || !str_cmp( arg2, "list" ) )
	{
		ch_printf( ch, "Current bestowed commands on %s: %s.\r\n", victim->name, victim->pcdata->bestowments );
		ch_printf( ch, "Current salary on %s: %d.\r\n", victim->name, victim->pcdata->salary );

		return;
	}

	if( str_cmp( ch->name, clan->leader ) && !str_cmp( ch->name, clan->number1 ) )
	{
		if( !is_name( arg2, ch->pcdata->bestowments ) )
		{
			send_to_char( "&RI don't think you're even allowed to do that.&W\r\n", ch );
			return;
		}
	}

	if( !str_cmp( arg2, "none" ) )
	{
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( "" );
		ch_printf( ch, "Bestowments removed from %s.\r\n", victim->name );
		ch_printf( victim, "%s has removed your bestowed clan abilities.\r\n", ch->name );
		return;
	}
	else if( !str_cmp( arg2, "pilot" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg2 );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		ch_printf( victim, "%s has given you permission to fly clan ships.\r\n", ch->name );
		send_to_char( "Ok, they now have the ability to fly clan ships.\r\n", ch );
	}
	else if( !str_cmp( arg2, "withdraw" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg2 );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		ch_printf( victim, "%s has given you permission to withdraw clan funds.\r\n", ch->name );
		send_to_char( "Ok, they now have the ablitity to withdraw clan funds.\r\n", ch );
	}
	else if( !str_cmp( arg2, "clanbuyship" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg2 );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		ch_printf( victim, "%s has given you permission to buy clan ships.\r\n", ch->name );
		send_to_char( "Ok, they now have the ablitity to use clanbuyship.\r\n", ch );
	}

	else if( !str_cmp( arg2, "initiation" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg2 );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		ch_printf( victim, "%s has given you the power to initiate people.\r\n", ch->name );
		send_to_char( "Ok, they now have the ability to initiate.\r\n", ch );
	}
	else if( !str_cmp( arg2, "salary" ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg2 );
		DISPOSE( victim->pcdata->bestowments );
		victim->pcdata->bestowments = str_dup( buf );
		ch_printf( victim, "%s has given you permission to assign salaries.\r\n", ch->name );
		send_to_char( "Ok, they now have the ablitity to assign salaries.\r\n", ch );
	}
	else
	{
		send_to_char( "Currently you may empower members with only the following:\r\n", ch );
		send_to_char( "\n\rpilot:       ability to fly clan ships\r\n", ch );
		send_to_char( "withdraw:    ability to withdraw clan funds\r\n", ch );
		send_to_char( "clanbuyship: ability to buy clan ships\r\n", ch );
		send_to_char( "salary:      ability to assign salaries\r\n", ch );
		send_to_char( "initiation   ability to initiate people\r\n", ch );
		send_to_char( "none:        removes bestowed abilities\r\n", ch );
	}

	save_char_obj( victim ); /* clan gets saved when pfile is saved */
	return;


}

void save_senate( )
{
	/*
		BOUNTY_DATA *tbounty;
		FILE *fpout;
		char filename[256];

		snprintf( filename, 256, "%s%s", SYSTEM_DIR, BOUNTY_LIST );
		fpout = FileOpen( filename, "w" );
		if ( !fpout )
		{
			 bug( "FATAL: cannot open bounty.lst for writing!\r\n", 0 );
			 return;
		}
		for ( tbounty = first_bounty; tbounty; tbounty = tbounty->next )
		{
			fprintf( fpout, "%s\n", tbounty->target );
			fprintf( fpout, "%ld\n", tbounty->amount );
		}
		fprintf( fpout, "$\n" );
		FileClose( fpout );
	*/
}

void load_senate( )
{
	first_senator = NULL;
	last_senator = NULL;
	/*
		FILE *fpList;
		char *target;
		char bountylist[256];
		BOUNTY_DATA *bounty;
		long int  amount;

		first_bounty = NULL;
		last_bounty	= NULL;

		first_disintigration = NULL;
		last_disintigration	= NULL;

		log_string( "Loading disintigrations..." );

		snprintf( bountylist, 256, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST );
		if ( ( fpList = FileOpen( bountylist, "r" ) ) == NULL )
		{
		perror( bountylist );
		exit( 11 );
		}

		for ( ; ; )
		{
			target = feof( fpList ) ? "$" : fread_word( fpList );
			if ( target[0] == '$' )
			break;
		CREATE( bounty, BOUNTY_DATA, 1 );
			LINK( bounty, first_disintigration, last_disintigration, next, prev );
		bounty->target = STRALLOC(target);
		amount = fread_number( fpList );
		bounty->amount = amount;
		}
		FileClose( fpList );
		log_string(" Done bounties " );

		return;
	*/
}

CMDF( do_senate )
{
	/*
		GOV_DATA *gov;
		int count = 0;

		set_char_color( AT_WHITE, ch );
		send_to_char( "\n\rGoverning Area                 Controlled By             Value\r\n", ch );
		for ( gov = first_gov; gov; gov = gov->next )
		{
			set_char_color( AT_YELLOW, ch );
			ch_printf( ch, "%-30s %-25s %-15ld\r\n", gov->name, gov->controlled_by , gov->value );
			count++;
		}

		if ( !count )
		{
			set_char_color( AT_GREY, ch );
			send_to_char( "There are no governments to capture at this time.\r\n", ch );
		return;
		}
	*/
}

CMDF( do_addsenator )
{
	/*
		GOVE_DATA *gov;

		CREATE( gov, GOV_DATA, 1 );
		LINK( gov, first_gov, last_gov, next, prev );

		gov->name		= STRALLOC( argument );
		gov->value          = atoi( arg2 );
		gov->vnum           = object;
		gov->controlled_by  = STRALLOC( "" );

		ch_printf( ch, "OK, making %s.\r\n", argument );
		save_govs();
	*/
}

CMDF( do_remsenator )
{
	/*
		UNLINK( bounty, first_bounty, last_bounty, next, prev );
		STRFREE( bounty->target );
		DISPOSE( bounty );

		save_bounties();
	*/
}

long get_taxes( PLANET_DATA *planet )
{
	long gain;

	gain = planet->base_value;
	gain += planet->base_value * planet->pop_support / 100;
	gain += UMAX( 0, planet->pop_support / 10 * planet->population );

	return gain;
}

/*
	(link)->prev		= (insert)->prev;
	if ( !(insert)->prev )
	  (first)			= (link);
	else
	  (insert)->prev->next	= (link);
	(insert)->prev		= (link);
	(link)->next		= (insert);
*/

CMDF( do_addsalary )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	int salary;

	if( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if( ( ch->pcdata && ch->pcdata->bestowments
		&& is_name( "salary", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
		;
	else
	{
		send_to_char( "You clan hasn't seen fit to bestow that ability to you!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	salary = atoi( arg2 );

	if( arg[0] == '\0' )
	{
		send_to_char( "Assign a salary to whom?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "That player is not here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Not on NPC's.\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		send_to_char( "Nice try.\r\n", ch );
		return;
	}

	if( victim->pcdata->clan != ch->pcdata->clan )
	{
		send_to_char( "This player does not belong to your clan!\r\n", ch );
		return;
	}

	if( salary < 0 || salary > 10000 )
	{
		ch_printf( ch, "Salary range is from 0 to 10,000!\r\n", victim->name );
		return;
	}

	victim->pcdata->salary = salary;
	ch_printf( ch, "%s has been assigned %d dollars for a salary.\r\n", victim->name, salary );
	ch_printf( victim, "%s has give you a %d dollar salary.\r\n", ch->name, salary );
	return;

}

CMDF( do_promote )
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( IS_NPC( ch ) )
	{
		send_to_char( "NPC's can not promote someone.\r\n", ch );
		return;
	}

	if( ( ch->pcdata->clan_rank < 9 ) && ( !IS_IMMORTAL( ch ) ) )
	{
		send_to_char( "You must be a clan Leader or rank of atleast 10 to promote someone.\r\n", ch );
		return;
	}
	if( arg1[0] == '\0' || arg2[0] == '\0' || atoi( arg2 ) < 1 || atoi( arg2 ) > 10 )
	{
		send_to_char( "\r\n&BSyntax&b: &cPromote <char> <1-10>\n\r1 - Lowest\n\r10 - Highest\r\n", ch );
		return;
	}

	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They must be present to be promoted.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "You can't promote NPC's!\r\n", ch );
		return;
	}

	if( ( victim->pcdata->clan != ch->pcdata->clan ) && ( !IS_IMMORTAL( ch ) ) )
	{
		send_to_char( "You can not promote a player who is not in your clan.\r\n", ch );
		return;
	}
	/*
		if ( if ( !str_cmp( ch->name, ch->pcdata->clan->leader ) ) && atoi(arg2)==10 )
		{
		send_to_char("This player is not qualified to lead.\r\n",ch);
		return;
		}
	*/
	victim->pcdata->clan_rank = atoi( arg2 );

	send_to_char( "Rank changed.\r\n", ch );
	send_to_char( "Your rank has been changed!\r\n", victim );

	return;
}


CMDF( do_war )
{
	CLAN_DATA *wclan;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if( ch->pcdata->clan_rank <= 8 )
	{
		send_to_char( "&RYou must be atleast rank 8 to use this!\r\n", ch );
		return;
	}


	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: Declare <ally/war> <clan>\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "war" ) )
	{
		if( ( wclan = get_clan( arg2 ) ) == NULL )
		{
			send_to_char( "No such clan.\r\n", ch );
			return;
		}

		if( wclan == clan )
		{
			send_to_char( "Declare war on yourself?!\r\n", ch );
			return;
		}

		if( nifty_is_name( wclan->name, clan->ally ) )
		{
			send_to_char( "Declare war on an Ally!?\r\n", ch );
			return;
		}

		if( nifty_is_name( wclan->name, clan->atwar ) )
		{
			CLAN_DATA *tclan;
			mudstrlcpy( buf, "", MSL );

			for( tclan = first_clan; tclan; tclan = tclan->next )
				if( nifty_is_name( tclan->name, clan->atwar ) && tclan != wclan )
				{
					mudstrlcat( buf, "\r\n ", MSL );
					mudstrlcat( buf, tclan->name, MSL );
					mudstrlcat( buf, " ", MSL );
				}

			STRFREE( clan->atwar );
			clan->atwar = STRALLOC( buf );

			snprintf( buf, MAX_STRING_LENGTH, "%s is no longer at war with %s!", clan->name, wclan->name );
			echo_to_all( AT_RED, buf, ECHOTAR_ALL );

			save_char_obj( ch );   /* clan gets saved when pfile is saved */

			return;
		}

		mudstrlcpy( buf, clan->atwar, MSL );
		mudstrlcat( buf, "\r\n ", MSL );
		mudstrlcat( buf, wclan->name, MSL );
		mudstrlcat( buf, " ", MSL );

		STRFREE( clan->atwar );
		clan->atwar = STRALLOC( buf );

		snprintf( buf, MAX_STRING_LENGTH, "%s declares war on %s!", clan->name, wclan->name );
		echo_to_all( AT_RED, buf, ECHOTAR_ALL );

		save_char_obj( ch );  /* clan gets saved when pfile is saved */
	}

	if( !str_cmp( arg1, "ally" ) )
	{
		if( ( wclan = get_clan( arg2 ) ) == NULL )
		{
			send_to_char( "No such clan.\r\n", ch );
			return;
		}

		if( wclan == clan )
		{
			send_to_char( "Consider yourself an ally?!\r\n", ch );
			return;
		}

		if( nifty_is_name( wclan->name, clan->atwar ) )
		{
			send_to_char( "Declare allies with someone you're at war with!?\r\n", ch );
			return;
		}

		if( nifty_is_name( wclan->name, clan->ally ) )
		{
			CLAN_DATA *tclan;
			mudstrlcpy( buf, "", MSL );

			for( tclan = first_clan; tclan; tclan = tclan->next )
				if( nifty_is_name( tclan->name, clan->ally ) && tclan != wclan )
				{
					mudstrlcat( buf, "\r\n ", MSL );
					mudstrlcat( buf, tclan->name, MSL );
					mudstrlcat( buf, " ", MSL );
				}

			STRFREE( clan->ally );
			clan->ally = STRALLOC( buf );

			snprintf( buf, MAX_STRING_LENGTH, "%s no longer considers %s an ally!", clan->name, wclan->name );
			echo_to_all( AT_RED, buf, ECHOTAR_ALL );

			save_char_obj( ch );   /* clan gets saved when pfile is saved */

			return;
		}

		mudstrlcpy( buf, clan->ally, MSL );
		mudstrlcat( buf, "\r\n ", MSL );
		mudstrlcat( buf, wclan->name, MSL );
		mudstrlcat( buf, " ", MSL );

		STRFREE( clan->ally );
		clan->ally = STRALLOC( buf );

		snprintf( buf, MAX_STRING_LENGTH, "%s now considers %s an ally!", clan->name, wclan->name );
		echo_to_all( AT_RED, buf, ECHOTAR_ALL );

		save_char_obj( ch );  /* clan gets saved when pfile is saved */
	}


}

CMDF( do_ucargo )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	int cost;
	PLANET_DATA *planet;

	if( argument[0] == '\0' )
	{
		act( AT_PLAIN, "Which ship do you want to unload?.", ch, NULL, NULL, TO_CHAR );
		return;
	}

	target = ship_in_room( ch->in_room, argument );

	if( !target )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
	}
	if( !check_pilot( ch, target ) )
	{
		send_to_char( "Hey, that's not your ship!\r\n", ch );
		return;
	}

	if( target->cargo == 0 )
	{
		send_to_char( "You don't have any cargo.\r\n", ch );
		return;
	}

	if( !xIS_SET( ch->in_room->room_flags, ROOM_DOCKING ) && !xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) )
	{
		send_to_char( "You can't do that here!", ch );
		return;
	}
	planet = ch->in_room->area->planet;

	if( !planet )
	{
		ship = ship_from_hanger( ch->in_room->vnum );

		if( !ship )
		{
			send_to_char( "You can't do that here!", ch );
			return;
		}
		if( ( ship->maxcargo - ship->cargo ) < 1 )
		{
			send_to_char( "There is no room for anymore cargo\r\n", ch );
			return;
		}
		if( ship->cargo == 0 )
			ship->cargotype = CTYPE_NONE;

		if( ( ship->cargo > 0 ) && ( ship->cargotype != target->cargo ) )
		{
			send_to_char( "They have a differnt type of cargo.\r\n", ch );
			return;
		}
		if( ship->cargotype == CTYPE_NONE )
			ship->cargotype = target->cargotype;
		if( ( ship->maxcargo - ship->cargo ) >= target->cargo )
		{
			ship->cargo += target->cargo;
			target->cargo = 0;
			target->cargo = CTYPE_NONE;
			send_to_char( "Cargo unloaded.\r\n", ch );
			return;
		}
		else
		{
			target->cargo -= ship->maxcargo - ship->cargo;
			ship->cargo = ship->maxcargo;
			ch_printf( ch, "%s Loaded, %d tons still in %s hold.\r\n", ship->name, target->cargo, target->name );
			return;
		}
	}
	if( ( target->cargotype != CTYPE_NONE ) && ( target->cargotype !=
		planet->pImport[0] ) && ( planet->pImport[0] != CTYPE_ALL ) )
	{
		send_to_char( "You can't deliver that here.\r\n", ch );
		return;
	}
	cost = target->cargo;
	cost *= ( planet->base_value / 1000 ) * target->cargotype;

	ch->gold += cost;
	target->cargo = 0;
	target->cargotype = CTYPE_NONE;
	ch_printf( ch, "You recieve %d credits for a load of %s.\r\n", cost, cargo_names[planet->pImport[0]] );
	return;
}

CMDF( do_lcargo )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	int cost;
	PLANET_DATA *planet;

	if( argument[0] == '\0' )
	{
		act( AT_PLAIN, "Which ship do you want to load?.", ch, NULL, NULL, TO_CHAR );
		return;
	}

	target = ship_in_room( ch->in_room, argument );

	if( !target )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
	}

	if( !check_pilot( ch, target ) )
	{
		send_to_char( "Hey, that's not your ship!\r\n", ch );
		return;
	}

	if( !xIS_SET( ch->in_room->room_flags, ROOM_DOCKING ) && !xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) )
	{
		send_to_char( "You can't do that here!", ch );
		return;
	}
	planet = ch->in_room->area->planet;

	if( !planet )
	{
		ship = ship_from_hanger( ch->in_room->vnum );
		if( !ship )
		{
			send_to_char( "You can't do that here!", ch );
			return;
		}
		if( ship->cargo == 0 )
		{
			send_to_char( "They don't have any cargo\r\n", ch );
			return;
		}
		if( ( target->maxcargo - target->cargo ) < 1 )
		{
			send_to_char( "There is no room for anymore cargo\r\n", ch );
			return;
		}
		if( ( target->cargotype = !CTYPE_NONE ) && ( ship->cargotype != target->cargotype ) );
		{
			send_to_char( "Maybe you should deliver your cargo first.\r\n", ch );
			return;
		}
		if( target->cargotype == CTYPE_NONE )
			target->cargotype = ship->cargotype;

		if( ( target->maxcargo - target->cargo ) >= ship->cargo )
		{
			target->cargo += ship->cargo;
			ship->cargo = 0;
			send_to_char( "Cargo loaded.\r\n", ch );
			return;
		}
		else
		{
			ship->cargo -= target->maxcargo - target->cargo;
			target->cargo = target->maxcargo;
			send_to_char( "Cargo Loaded.\r\n", ch );
			return;
		}

	}
	if( target->maxcargo - target->cargo <= 0 )
	{
		send_to_char( "There is no room for more Cargo.\r\n", ch );
		return;
	}
	if( ( target->cargotype != CTYPE_NONE ) && ( target->cargo != planet->pExport[0] ) )
	{
		send_to_char( "Maybe you should deliver your cargo first\r\n", ch );
		return;
	}
	if( planet->pExport[0] == CTYPE_NONE )
	{
		send_to_char( "We don't export goods here\r\n", ch );
		return;
	}

	cost = ( target->maxcargo - target->cargo );
	cost *= ( planet->base_value / 10000 ) * planet->pExport[0];
	if( ch->gold < cost )
	{
		send_to_char( "You can't afford it!\r\n", ch );
		return;
	}
	ch->gold -= cost;
	target->cargo = target->maxcargo;
	target->cargotype = planet->pExport[0];
	ch_printf( ch, "You pay %d credits for a load of %s.\r\n", cost, cargo_names[planet->pExport[0]] );
	return;
}

CMDF( do_imports )
{
	PLANET_DATA *planet;

	set_char_color( AT_WHITE, ch );
	for( planet = first_planet; planet; planet = planet->next )
	{
		ch_printf( ch, "&wPlanet: &B%-15s\r\n", planet->name );
		ch_printf( ch, "&wImports:\r\n"
			"CDI's: %-3s       Batteries: %-3s       Iron: %-3s     Steel: %-3s\r\n"
			"Titanium: %-3s    Neo-Titanium: %-3s    Gundanium: %-3s\r\n",
			planet->pImport[2] == 1 ? "Yes" : "No", planet->pImport[3] == 1 ? "Yes" : "No",
			planet->pImport[4] == 1 ? "Yes" : "No", planet->pImport[5] == 1 ? "Yes" : "No",
			planet->pImport[6] == 1 ? "Yes" : "No", planet->pImport[7] == 1 ? "Yes" : "No",
			planet->pImport[8] == 1 ? "Yes" : "No" );

		ch_printf( ch, "&P==&wExports:\r\n"
			"CDI's: %-3s       Batteries: %-3s       Iron: %-3s     Steel: %-3s\r\n"
			"Titanium: %-3s    Neo-Titanium: %-3s    Gundanium: %-3s\r\n\r\n",
			planet->pExport[2] == 1 ? "Yes" : "No", planet->pExport[3] == 1 ? "Yes" : "No",
			planet->pExport[4] == 1 ? "Yes" : "No", planet->pExport[5] == 1 ? "Yes" : "No",
			planet->pExport[6] == 1 ? "Yes" : "No", planet->pExport[7] == 1 ? "Yes" : "No",
			planet->pExport[8] == 1 ? "Yes" : "No" );



		ch_printf( ch, "\r\n" );

	}
}

CMDF( do_cargo )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	PLANET_DATA *planet;
	SHIP_DATA *ship;
	int cargot = 0, selltype, cost;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "\n\rSyntax:\n\rCargo load <type>\n\rCargo unload\r\n", ch );
		send_to_char( "", ch );
		return;
	}

	if( !str_cmp( arg1, "load" ) )
	{

		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in your suit to do that!\r\n", ch );
			return;
		}

		if( ship->ship_class != MOBILE_SUIT )
		{
			send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
			return;
		}

		if( !check_pilot( ch, ship ) )
		{
			send_to_char( "&RUse your own suit!\r\n", ch );
			return;
		}

		if( ship->lastdoc != ship->location )
		{
			send_to_char( "&RYou don't seem to be docked right now.\r\n", ch );
			return;
		}

		if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
		{
			send_to_char( "&RYou don't seem to be docked right now..\r\n", ch );
			return;
		}

		if( xIS_SET( ship->in_room->room_flags, ROOM_DOCKING ) )
		{
			send_to_char( "You can't load cargo here.\r\n", ch );
			return;
		}

		if( arg2[0] == '\0' )
		{
			send_to_char( "Load what cargo onto your suit?\r\n", ch );
			return;
		}
		else if( !str_cmp( arg2, "cdi" ) )
			cargot = 2;
		else if( !str_cmp( arg2, "batteries" ) )
			cargot = 3;
		else if( !str_cmp( arg2, "iron" ) )
			cargot = 4;
		else if( !str_cmp( arg2, "steel" ) )
			cargot = 5;
		else if( !str_cmp( arg2, "titanium" ) )
			cargot = 6;
		else if( !str_cmp( arg2, "neotitanium" ) )
			cargot = 7;
		else if( !str_cmp( arg2, "gundanium" ) )
			cargot = 8;
		else
		{
			send_to_char( "No such cargo.\r\n", ch );
			return;
		}


		planet = ship->in_room->area->planet;


		if( ( ship->cargotype != CTYPE_NONE ) )
		{
			send_to_char( "You already have cargo. Deliver it first!\r\n", ch );
			return;
		}

		if( planet->pExport[cargot] == 0 )
		{
			ch_printf( ch, "%s doesn't have that type of export.\r\n", planet->name );
			return;
		}

		cost = ( cargot * 375 );
		cost *= ( planet->pBuy[cargot] );

		if( ch->gold < cost )
		{
			ch_printf( ch, "&RThis cargo will cost you $%d, and you can't afford it.\r\n", cost );
			return;
		}

		ch->gold -= cost;
		ship->cargotype = cargot;
		ship->cargo = 1;

		if( planet->pBuy[cargot] <= 5 )
		{
			planet->pBuy[cargot] += 1;
		}

		if( planet->pSell[cargot] >= 0 )
		{
			planet->pSell[cargot] -= 1;
		}

		ch_printf( ch, "&GYou load your suit with %s for $%d.\r\n", cargo_names[planet->pExport[cargot]], cost );
		return;
	}

	if( !str_cmp( arg1, "unload" ) )
	{

		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in your suit to do that!\r\n", ch );
			return;
		}

		if( ship->ship_class != MOBILE_SUIT )
		{
			send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
			return;
		}

		if( !check_pilot( ch, ship ) )
		{
			send_to_char( "&RUse your own suit!\r\n", ch );
			return;
		}

		if( ship->lastdoc != ship->location )
		{
			send_to_char( "&RYou don't seem to be docked right now.\r\n", ch );
			return;
		}

		if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
		{
			send_to_char( "&RYou don't seem to be docked right now..\r\n", ch );
			return;
		}

		if( xIS_SET( ship->in_room->room_flags, ROOM_DOCKING ) )
		{
			send_to_char( "You can't unload cargo here.\r\n", ch );
			return;
		}

		if( ship->cargotype == 2 )
			selltype = 2;
		else if( ship->cargotype == 3 )
			selltype = 3;
		else if( ship->cargotype == 4 )
			selltype = 4;
		else if( ship->cargotype == 5 )
			selltype = 5;
		else if( ship->cargotype == 6 )
			selltype = 6;
		else if( ship->cargotype == 7 )
			selltype = 7;
		else if( ship->cargotype == 8 )
			selltype = 8;
		else
		{
			selltype = 2;
		}

		planet = ship->in_room->area->planet;

		if( planet->pImport[selltype] == 0 )
		{
			ch_printf( ch, "%s doesn't import %s!\r\n", planet->name, cargo_names[planet->pImport[selltype]] );
			return;
		}

		if( ( ship->cargotype == CTYPE_NONE ) )
		{
			send_to_char( "You don't have anything to deliver!\r\n", ch );
			return;
		}

		cost = ( cargot * 375 );
		cost *= ( planet->pBuy[cargot] );

		ch->gold += cost;
		ship->cargotype = CTYPE_NONE;
		ship->cargo = 0;

		if( planet->pSell[cargot] <= 5 )
		{
			planet->pSell[cargot] += 1;
		}

		if( planet->pBuy[cargot] >= 0 )
		{
			planet->pBuy[cargot] -= 1;
		}

		ch_printf( ch, "&GYour shipment of %s makes a payoff of $%d for you.\r\n", cargo_names[planet->pImport[cargot]], cost );
		return;
	}

	if( !str_cmp( arg1, "stats" ) )
	{
		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in your suit to do that!\r\n", ch );
			return;
		}

		planet = ship->in_room->area->planet;

		ch_printf( ch, "&B%s&b:&w\r\n", planet->name );

		if( planet->pImport[2] == 1 )
			ch_printf( ch, "[CDI] Selling: %d%\r\n", planet->pSell[2] * 20 );

		if( planet->pExport[2] == 1 )
			ch_printf( ch, "[CDI] Buying: %d%\r\n", planet->pBuy[2] * 20 );

		if( planet->pImport[3] == 1 )
			ch_printf( ch, "[Batteries] Selling: %d%\r\n", planet->pSell[3] * 20 );

		if( planet->pExport[3] == 1 )
			ch_printf( ch, "[Batteries] Buying: %d%\r\n", planet->pBuy[3] * 20 );

		if( planet->pImport[4] == 1 )
			ch_printf( ch, "[Iron] Selling: %d%\r\n", planet->pSell[4] * 20 );

		if( planet->pExport[4] == 1 )
			ch_printf( ch, "[Iron] Buying: %d%\r\n", planet->pBuy[4] * 20 );

		if( planet->pImport[5] == 1 )
			ch_printf( ch, "[Steel] Selling: %d%\r\n", planet->pSell[5] * 20 );

		if( planet->pExport[5] == 1 )
			ch_printf( ch, "[Steel] Buying: %d%\r\n", planet->pBuy[5] * 20 );

		if( planet->pImport[6] == 1 )
			ch_printf( ch, "[Titanium] Selling: %d%\r\n", planet->pSell[6] * 20 );

		if( planet->pExport[6] == 1 )
			ch_printf( ch, "[Titanium] Buying: %d%\r\n", planet->pBuy[6] * 20 );

		if( planet->pImport[7] == 1 )
			ch_printf( ch, "[Neo-Titanium] Selling: %d%\r\n", planet->pSell[7] * 20 );

		if( planet->pExport[7] == 1 )
			ch_printf( ch, "[Neo-Titanium] Buying: %d%\r\n", planet->pBuy[7] * 20 );

		if( planet->pImport[8] == 1 )
			ch_printf( ch, "[Gundanium] Selling: %d%\r\n", planet->pSell[8] * 20 );

		if( planet->pExport[8] == 1 )
			ch_printf( ch, "[Gundanium] Buying: %d%\r\n", planet->pBuy[8] * 20 );
		return;
	}

	do_cargo( ch, "" );
}

CMDF( do_members )
{
	FILE *fpList;
	const char *buf;
	char thebuf[MAX_STRING_LENGTH];
	char list[MAX_STRING_LENGTH];
	char display[MAX_STRING_LENGTH];
	char prefix[MAX_STRING_LENGTH];
	int i = 0;
	CLAN_DATA *clan;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: members <clan>\r\n", ch );
		return;
	}

	clan = get_clan( argument );
	if( !clan )
	{
		send_to_char( "No such clan.\r\n", ch );
		return;
	}

	if( !clan->shortname )
	{
		send_to_char( "&RYour clan doesn't have a shortname. Tell an imm this.\r\n", ch );
		return;
	}

	snprintf( list, sizeof(list), "%s%s.list", CLAN_DIR, clan->shortname );

	if( ( fpList = FileOpen( list, "r" ) ) == NULL )
	{
		send_to_char( "Something wen't wrong. The imms have been notified.\r\n", ch );
		bug( "Do_members: Unable to open member list" );
		return;
	}


	snprintf( thebuf, sizeof(thebuf), "\n          &P%s&B\r\n", clan->name );
	send_to_char( thebuf, ch );

	for( ;; )
	{
		if( feof( fpList ) )
			break;
		buf = feof( fpList ) ? "End" : fread_string( fpList );
		if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
			break;
		if( strlen( buf ) < 3 )
			break;

		if( i % 3 == 0 )
			send_to_char( "&W     ", ch );

		if( !str_cmp( buf, clan->leader ) )
			snprintf( prefix, sizeof(prefix), "&B*&w" );
		else if( !str_cmp( buf, clan->number1 ) )
			snprintf( prefix, sizeof(prefix), "&B*&w" );
		else if( !str_cmp( buf, clan->number2 ) )
			snprintf( prefix, sizeof(prefix), "&B*&w" );
		else
			snprintf( prefix, sizeof(prefix), "   &w" );

		snprintf( display, sizeof(display), "%s%-10s", prefix, buf );
		send_to_char( display, ch );
		++i;

		if( i % 3 == 0 )
			send_to_char( "\r\n", ch );

	}
	send_to_char( "\r\n", ch );
	FileClose( fpList );
}

CMDF( do_paratroopers )
{
	/*
	  int num,vnum,i;
	  SHIP_DATA *ship;
	  CHAR_DATA *mob;
	  MOB_INDEX_DATA  * pMobIndex;
	  OBJ_DATA        * blaster;
	  OBJ_INDEX_DATA  * pObjIndex;
	  ROOM_INDEX_DATA *room;
	  char tmpbuf[MAX_STRING_LENGTH];
	  if (ch->pcdata->clan == NULL)
	  {
		send_to_char("You must be in a clan to drop Paratroopers.\r\n", ch);
		return;
	  }

	  if (ch->pcdata->clan->paratroopers < 1)
	  {
		send_to_char("Your clan has no Paratroopers.\r\n", ch);
		return;
	  }

	  if(argument[0] == '\0' || isalpha(argument[0]))
	  {
		send_to_char("&RUnload how many Paratroopers?\r\n", ch);
		return;
	  }

	  num = atoi(argument);
	  if (num > ch->pcdata->clan->paratroopers)
	  {
		send_to_char("Your clan doesn't have that many Paratroopers.\r\n", ch);
		return;
	  }

	  if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
	  {
		send_to_char("You must be in the cockpit of the suit to do this.\r\n", ch);
		return;
	  }

	  vnum = ship->location;
	  if ((room = get_room_index(vnum)) == NULL)
	  {
		send_to_char("This ship is not in a room.\r\n", ch);
		return;
	  }

	  if (num > 10)
		num = 10;
	  if (num > ch->pcdata->clan->paratroopers)
		num = ch->pcdata->clan->paratroopers;

	  if ( ( pMobIndex = get_mob_index( 24 ) ) == NULL )
		  return;
	//  if (room->area == NULL)
	//    return;
	  for (i=1;i<=num;i++)
	  {
		vnum = number_range(room->area->low_vnum, room->area->hi_vnum);
		if ((room = get_room_index(vnum)) == NULL)
		  continue;
		mob = create_mobile( pMobIndex );
		char_to_room( mob, room );
		if ( ch->pcdata && ch->pcdata->clan )
		//STRFREE( mob->name );
		//mob->name = STRALLOC( ch->pcdata->clan->name );
		snprintf( tmpbuf, sizeof( tmpbuf ), "(%s) %s" , ch->pcdata->clan->name  , mob->long_descr );
		STRFREE( mob->long_descr );
		mob->mob_clan = ch->pcdata->clan->name;
		mob->long_descr = STRALLOC( tmpbuf );
		act( AT_IMMORT, "$N lands on the ground after parachuting in.", ch, NULL, mob, TO_ROOM );
		if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
		{
		  blaster = create_object( pObjIndex, mob->top_level );
		  obj_to_char( blaster, mob );
		  equip_char( mob, blaster, WEAR_WIELD );
		}
	  }
	  sprintf(tmpbuf, "&R%d Paratroopers leap out into the below area, floating to the ground as you soar over the area.\r\n", num);
	  ch->pcdata->clan->paratroopers-=num;
	  echo_to_cockpit(AT_RED, ship, tmpbuf);
	*/
}

void load_gang( void )
{
	char tmpbuf[MAX_STRING_LENGTH];
	CHAR_DATA *mob;
	CHAR_DATA *vch;
	CLAN_DATA *clan;
	MNAME_DATA *mname;
	FNAME_DATA *fname;
	MNAME_DATA *mrndm = NULL;
	FNAME_DATA *frndm = NULL;
	int count;
	ROOM_INDEX_DATA *pRoomIndex;
	int ability, schance;
	int thugcount, thugexact, thugs;
	int hitcount, hitexact, hits;
	int drugcount, drugexact, drugs;

	thugcount = 0;
	thugexact = 0;
	hitcount = 0;
	hitexact = 0;
	drugcount = 0;
	drugexact = 0;

	for( clan = first_clan; clan; clan = clan->next )
	{

		for( vch = first_char; vch; vch = vch->next )
		{
			if( !IS_NPC( vch ) )
				continue;

			if( vch->pIndexData->vnum == MOB_VNUM_THUG )
			{
				if( vch->gang == clan->name )
					thugcount++;
			}

			if( vch->pIndexData->vnum == MOB_VNUM_HITMEN )
			{
				if( vch->gang == clan->name )
					hitcount++;
			}

			if( vch->pIndexData->vnum == MOB_VNUM_DRUG_ADDICT )
			{
				if( vch->gang == clan->name )
					hitcount++;
			}

		}

		thugexact = ( clan->thug - thugcount );
		hitexact = ( clan->hitmen - hitcount );
		drugexact = ( clan->drugaddict - drugcount );

		for( thugs = 0; thugs < thugexact; thugs++ )
		{

			for( ;; )
			{
				pRoomIndex = get_room_index( number_range( 200, 13000 ) );
				if( pRoomIndex )
					if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_PLR_HOME )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_SPACECRAFT )
						&& str_cmp( pRoomIndex->name, "Floating in a void" )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && ( pRoomIndex->area->owned_by == clan->name ) )
						break;
			}
			mob = create_mobile( get_mob_index( MOB_VNUM_THUG ) );
			schance = number_range( 1, 3 );
			if( schance == 1 )
				mob->sex = 2;
			else
				mob->sex = 1;

			if( mob->sex == 1 )
			{
				count = 0;
				for( mname = first_mname; mname; mname = mname->next )
				{
					if( number_range( 0, count ) == 0 )
						mrndm = mname;
					count++;
				}
				snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, mrndm->name );
				STRFREE( mob->name );
				mob->name = STRALLOC( tmpbuf );
				STRFREE( mob->short_descr );
				mob->short_descr = STRALLOC( mrndm->name );
			}
			if( mob->sex == 2 )
			{
				count = 0;
				for( fname = first_fname; fname; fname = fname->next )
				{
					if( number_range( 0, count ) == 0 )
						frndm = fname;
					count++;
				}
				snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, frndm->name );
				STRFREE( mob->name );
				mob->name = STRALLOC( tmpbuf );
				STRFREE( mob->short_descr );
				mob->short_descr = STRALLOC( frndm->name );
			}

			snprintf( tmpbuf, MAX_STRING_LENGTH, "%s|%s|&w %s", clan->cone, clan->ctwo, mob->long_descr );
			STRFREE( mob->long_descr );
			mob->gang = STRALLOC( clan->name );
			mob->top_level = number_range( 100, 250 );
			for( ability = 0; ability < MAX_ABILITY; ability++ )
				mob->skill_level[ability] = mob->top_level;
			mob->long_descr = STRALLOC( tmpbuf );
			mob->hit = mob->top_level * 100;
			mob->max_hit = mob->hit;
			mob->armor = 100 - mob->top_level * 10.5;
			mob->damroll = mob->top_level / 5;
			mob->hitroll = mob->top_level / 5;
			char_to_room( mob, pRoomIndex );

		}

		for( hits = 0; hits < hitexact; hits++ )
		{

			for( ;; )
			{
				pRoomIndex = get_room_index( number_range( 200, 13000 ) );
				if( pRoomIndex )
					if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_PLR_HOME )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_SPACECRAFT )
						&& str_cmp( pRoomIndex->name, "Floating in a void" )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
						&& !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && ( pRoomIndex->area->owned_by == clan->name ) )
						break;
			}
			mob = create_mobile( get_mob_index( MOB_VNUM_HITMEN ) );
			schance = number_range( 1, 3 );
			if( schance == 1 )
				mob->sex = 2;
			else
				mob->sex = 1;

			if( mob->sex == 1 )
			{
				count = 0;
				for( mname = first_mname; mname; mname = mname->next )
				{
					if( number_range( 0, count ) == 0 )
						mrndm = mname;
					count++;
				}
				snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, mrndm->name );
				STRFREE( mob->name );
				mob->name = STRALLOC( tmpbuf );
				STRFREE( mob->short_descr );
				mob->short_descr = STRALLOC( mrndm->name );
			}
			if( mob->sex == 2 )
			{
				count = 0;
				for( fname = first_fname; fname; fname = fname->next )
				{
					if( number_range( 0, count ) == 0 )
						frndm = fname;
					count++;
				}
				snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, frndm->name );
				STRFREE( mob->name );
				mob->name = STRALLOC( tmpbuf );
				STRFREE( mob->short_descr );
				mob->short_descr = STRALLOC( frndm->name );
			}

			snprintf( tmpbuf, MAX_STRING_LENGTH, "%s|%s|&w %s", clan->cone, clan->ctwo, mob->long_descr );
			STRFREE( mob->long_descr );
			mob->gang = STRALLOC( clan->name );
			mob->top_level = number_range( 250, 650 );
			for( ability = 0; ability < MAX_ABILITY; ability++ )
				mob->skill_level[ability] = mob->top_level;
			mob->long_descr = STRALLOC( tmpbuf );
			mob->hit = mob->top_level * 100;
			mob->max_hit = mob->hit;
			mob->armor = 100 - mob->top_level * 10.5;
			mob->damroll = mob->top_level / 5;
			mob->hitroll = mob->top_level / 5;
			char_to_room( mob, pRoomIndex );
		}
	}

	for( drugs = 0; drugs < drugexact; drugs++ )
	{

		for( ;; )
		{
			pRoomIndex = get_room_index( number_range( 200, 13000 ) );
			if( pRoomIndex )
				if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
					&& !xIS_SET( pRoomIndex->room_flags, ROOM_PLR_HOME )
					&& !xIS_SET( pRoomIndex->room_flags, ROOM_SPACECRAFT )
					&& str_cmp( pRoomIndex->name, "Floating in a void" )
					&& !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )
					&& !xIS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) && ( pRoomIndex->area->owned_by == clan->name ) )
					break;
		}
		mob = create_mobile( get_mob_index( MOB_VNUM_DRUG_ADDICT ) );
		schance = number_range( 1, 3 );
		if( schance == 1 )
			mob->sex = 2;
		else
			mob->sex = 1;

		if( mob->sex == 1 )
		{
			count = 0;
			for( mname = first_mname; mname; mname = mname->next )
			{
				if( number_range( 0, count ) == 0 )
					mrndm = mname;
				count++;
			}
			snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, mrndm->name );
			STRFREE( mob->name );
			mob->name = STRALLOC( tmpbuf );
			STRFREE( mob->short_descr );
			mob->short_descr = STRALLOC( mrndm->name );
		}
		if( mob->sex == 2 )
		{
			count = 0;
			for( fname = first_fname; fname; fname = fname->next )
			{
				if( number_range( 0, count ) == 0 )
					frndm = fname;
				count++;
			}
			snprintf( tmpbuf, MAX_STRING_LENGTH, "%s %s", clan->name, frndm->name );
			STRFREE( mob->name );
			mob->name = STRALLOC( tmpbuf );
			STRFREE( mob->short_descr );
			mob->short_descr = STRALLOC( frndm->name );
		}

		snprintf( tmpbuf, MAX_STRING_LENGTH, "%s|%s|&w %s", clan->cone, clan->ctwo, mob->long_descr );
		STRFREE( mob->long_descr );
		mob->gang = STRALLOC( clan->name );
		mob->top_level = number_range( 500, 750 );
		for( ability = 0; ability < MAX_ABILITY; ability++ )
			mob->skill_level[ability] = mob->top_level;
		mob->long_descr = STRALLOC( tmpbuf );
		mob->hit = mob->top_level * 100;
		mob->max_hit = mob->hit;
		mob->armor = 100 - mob->top_level * 10.5;
		mob->damroll = mob->top_level / 5;
		mob->hitroll = mob->top_level / 5;
		char_to_room( mob, pRoomIndex );
	}
	return;
}

CMDF( do_startgang )
{
	char filename[256];
	char buf[MAX_STRING_LENGTH];
	CLAN_DATA *clan;

	if( IS_NPC( ch ) )
		return;

	if( ch->pcdata->clan )
	{
		send_to_char( "Resign from your current gang first!\r\n", ch );
		return;
	}

	if( ( ch->skill_level[COMBAT_ABILITY]
		+ ch->skill_level[PILOTING_ABILITY]
		+ ch->skill_level[ENGINEERING_ABILITY]
		+ ch->skill_level[HUNTING_ABILITY]
		+ ch->skill_level[SMUGGLING_ABILITY]
		+ ch->skill_level[DIPLOMACY_ABILITY]
		+ ch->skill_level[LEADERSHIP_ABILITY] + ch->skill_level[ESPIONAGE_ABILITY] ) < 3500 )
	{
		send_to_char( "You need atleast 3500 combined levels to start a gang.\r\n", ch );
		return;
	}

	if( ch->gold < 25000000 )
	{
		send_to_char( "You need 25,000,000 to start a gang!\r\n", ch );
		return;
	}

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: Startgang <Gang Name>\r\n", ch );
		return;
	}

	clan = get_clan( argument );
	if( clan )
	{
		send_to_char( "There is already a gang with that name.\r\n", ch );
		return;
	}

	snprintf( filename, sizeof( filename ), "%s%s", CLAN_DIR, strlower( argument ) );

	CREATE( clan, CLAN_DATA, 1 );
	LINK( clan, first_clan, last_clan, next, prev );
	clan->next_subclan = NULL;
	clan->prev_subclan = NULL;
	clan->last_subclan = NULL;
	clan->first_subclan = NULL;
	clan->mainclan = NULL;
	clan->name = STRALLOC( argument );
	snprintf( buf, MAX_STRING_LENGTH, "%s.clan", argument );
	clan->filename = str_dup( buf );
	clan->description = STRALLOC( "" );
	clan->leader = STRALLOC( ch->name );
	clan->number1 = STRALLOC( "" );
	clan->number2 = STRALLOC( "" );
	clan->ally = STRALLOC( "" );
	clan->atwar = STRALLOC( "" );
	clan->tmpstr = STRALLOC( "" );
	clan->description = STRALLOC( "We need a desc!" );
	clan->motto = str_dup( "We need a motto!" );
	clan->cone = STRALLOC( "^w" );
	clan->ctwo = STRALLOC( "^w" );
	snprintf( buf, MAX_STRING_LENGTH, "%s.list", clan->name );
	clan->shortname = str_dup( buf );

	ch->pcdata->clan = clan;
	ch->pcdata->clan_name = QUICKLINK( clan->name );
	snprintf( buf, MAX_STRING_LENGTH, "%s created clan: %s\r\n", ch->name, clan->name );
	log_string( buf );
	ch->gold -= 25000000;
	send_to_char( "Gang created.", ch );
	save_clan( clan );
	write_clan_list( );
}

/*
CMDF( do_clanstat )
{
   char buf[MAX_STRING_LENGTH];
   char atwar[MAX_STRING_LENGTH];
   CLAN_DATA *clan;
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   OBJ_DATA *datapad;
   bool found;
   PLANET_DATA *planet;
   long revenue = 0;
   int pCount = 0, support = 0;

   if( argument[0] == '\0' )
   {
	  if( mud_data.DATAPAD_REQUIRED == 1 )
	  {
		 if( !IS_IMMORTAL( ch ) )
		 {
			datapad = get_eq_char( ch, WEAR_HOLD );
			if( datapad && !( datapad->item_type == ITEM_DATAPAD ) )
			   datapad = NULL;

			if( !datapad && !IS_SET( ch->pcdata->cyber, CYBER_DATAPAD ) )
			{
			   send_to_char( "&RYou would need to be holding a datapad to access that kind of information.\r\n", ch );
			   return;
			}

			if( !IS_SET( ch->pcdata->cyber, CYBER_DATAPAD )
				&& ( datapad->datapad_setting == DATAPAD_OFF || datapad->datapad_setting == DATAPAD_RECHARGE ) )
			{
			   send_to_char( "&RYou will need to turn your datapad on first.\r\n", ch );
			   return;
			}
		 }

		 act( AT_LBLUE, "$n accesses the galactic network with $s datapad.", ch, NULL, NULL, TO_NOTVICT );
		 send_to_char
			( "&wYou log into the galactic network with your datapad.\n\rAccessing .......\n\rConnected.\r\n&B------------------------ &CGalactic Net Database&B ------------------------&w\r\n",
			  ch );
	  }

	  if( !ch->pcdata->clan )
	  {
		 if( IS_IMMORTAL( ch ) )
		 {
			send_to_char( "&CSyntax&B: &Wclanstat &B<&wclan name&B>\r\n", ch );
			send_to_char
			   ( "&B------------------------ &CGalactic Net Database&B ------------------------\r\n&wDisconnected.\r\n",
				 ch );
			return;
		 }
		 else
		 {
			send_to_char( "&RYou are not even IN a clan!\r\n", ch );
			send_to_char
			   ( "&B------------------------ &CGalactic Net Database&B ------------------------\r\n&wDisconnected.\r\n",
				 ch );
			return;
		 }
	  }

	  for( planet = first_planet; planet; planet = planet->next )
		 if( ch->pcdata->clan == planet->governed_by )
		 {
			support += planet->pop_support;
			pCount++;
			revenue += get_taxes( planet );
		 }

	  if( pCount > 1 )
		 support /= pCount;

	  send_to_char( "\r\n", ch );
	  ch_printf( ch, "&w%s\r\n", ch->pcdata->clan->name );
	  send_to_char( "&B--------------------------------------------------------\r\n", ch );
	  ch_printf( ch, "&CRevenue&B:     &Y%-18d &CFunds&B:   &Y%d\r\n", revenue, ch->pcdata->clan->funds );
	  ch_printf( ch, "&CLeader&B:      &w%-18.18s &CPlanets&B: &w%d\r\n", ch->pcdata->clan->leader, pCount );
	  ch_printf( ch, "&C1st Officer&B: &w%-18.18s &CSupport&B: &w%d\r\n", ch->pcdata->clan->number1, support );
	  ch_printf( ch, "&C2nd Officer&B: &w%-18.18s &CShips&B:   &w%d\r\n", ch->pcdata->clan->number2,
				 ch->pcdata->clan->spacecraft );
	  ch_printf( ch, "&CSenator&B:     &w%-18.18s &CMembers&B: &w%d\r\n", ch->pcdata->clan->senator,
				 ch->pcdata->clan->members );
	  ch_printf( ch, "&CBadge&B:       &w{&D%s&w}}\r\n", ch->pcdata->clan->badge );

	  if( ( ch->pcdata && ch->pcdata->bestowments
			&& is_name( "roster", ch->pcdata->bestowments ) )
		  || !str_cmp( ch->name, ch->pcdata->clan->leader )
		  || !str_cmp( ch->name, ch->pcdata->clan->number1 ) || !str_cmp( ch->name, ch->pcdata->clan->number2 )
		  || IS_IMMORTAL( ch ) )
		 do_roster( ch, "" );

	  if( !ch->pcdata->clan->atwar[0] == '\0' )
	  {
		 send_to_char( "&B------- &CCurrently at War &B-------\r\n", ch );
		 snprintf( atwar, sizeof( atwar ), "&W%s\r\n", ch->pcdata->clan->atwar );
		 send_to_char( atwar, ch );
	  }

	  send_to_char( "&B------- &CMembers Logged into the Galactic Network &B-------\r\n", ch );
	  found = false;
	  for( d = first_descriptor; d; d = d->next )
		 if( ( d->connected == CON_PLAYING )
			 && ( victim = d->character ) != NULL
			 && !IS_NPC( victim )
			 && ( victim->pcdata->clan ) != NULL && ( victim->pcdata->clan->name == ch->pcdata->clan->name ) )
		 {
			found = true;
			pager_printf( ch, "&C%-22.22s&B&W%s\r\n",
						  victim->name,
						  victim->pcdata->clan->leader == victim->name ? "&B(&CLeader&B)" :
						  victim->pcdata->clan->number1 == victim->name ? "&B(&CFirst&B)" :
						  victim->pcdata->clan->number2 == victim->name ? "&B(&CSecond&B)" :
						  victim->pcdata->clan->ambassador == victim->name ? "&B(&CAmbassador&B)" :
						  victim->pcdata->clan->senator == victim->name ? "&B(&CSenator&B)" :
						  victim->pcdata->clan->overseer == victim->name ? "&B(&COverseer&B)&W " : "" );
		 }
	  if( !found )
		 send_to_char( "None.\r\n", ch );

	  send_to_char( "&B------------------------- &CGalactic Net Database &B-------------------------\r\n", ch );
	  send_to_char( "&wDisconnected.\r\n", ch );
	  return;
   }

   clan = get_clan( argument );

   send_to_char
	  ( "&wYou log into the galactic network with your datapad.\n\rAccessing .......\n\rConnected.\r\n&B------------------------ &CGalactic Net Database &B------------------------\r\n",
		ch );

   if( !clan )
   {
	  send_to_char( "&RNo such clan!\r\n", ch );
	  send_to_char( "&B------------------------ &CGalactic Net Database &B-------------------------\r\n", ch );
	  send_to_char( "&wDisconnected.\r\n", ch );
	  return;
   }

   for( planet = first_planet; planet; planet = planet->next )
	  if( clan == planet->governed_by )
	  {
		 support += planet->pop_support;
		 pCount++;
		 revenue += get_taxes( planet );
	  }

   if( pCount > 1 )
	  support /= pCount;

   send_to_char( "\r\n", ch );
   ch_printf( ch, "&w%s\r\n", clan->name );
   send_to_char( "&B--------------------------------------------------------\r\n", ch );
   ch_printf( ch, "&CRevenue&B:     &Y%-18d &CFunds&B:   &Y%d\r\n", revenue, clan->funds );
   ch_printf( ch, "&CLeader&B:      &w%-18.18s &CPlanets&B: &w%d\r\n", clan->leader, pCount );
   ch_printf( ch, "&C1st Officer&B: &w%-18.18s &CSupport&B: &w%d\r\n", clan->number1, support );
   ch_printf( ch, "&C2nd Officer&B: &w%-18.18s &CShips&B:   &w%d\r\n", clan->number2, clan->spacecraft );
   ch_printf( ch, "&CSenator&B:     &w%-18.18s &CMembers&B: &w%d\r\n", clan->senator, clan->members );
   ch_printf( ch, "&CBadge&B:       &w{&D%s&w}}\r\n\r\n", clan->badge );

   send_to_char( "&B------------------- &CMembers &B------------------------\r\n", ch );
   ch_printf( ch, "&CLevel 5&B: &w%-24.24s &CWage&B: &Y%s\r\n", clan->r5, num_punct( clan->level5wage ) );
   if( clan->roster_5[0] == '\0' )
	  send_to_char( "\r\n", ch );
   else
   {
	  snprintf( buf, MAX_STRING_LENGTH, "&w%s\r\n", commify_list( clan->roster_5 ) );
	  send_to_char( buf, ch );
   }
   ch_printf( ch, "&CLevel 4&B: &w%-24.24s &CWage&B: &Y%s\r\n", clan->r4, num_punct( clan->level4wage ) );
   if( clan->roster_4[0] == '\0' )
	  send_to_char( "\r\n", ch );
   else
   {
	  snprintf( buf, MAX_STRING_LENGTH, "&w%s\r\n", commify_list( clan->roster_4 ) );
	  send_to_char( buf, ch );
   }
   ch_printf( ch, "&CLevel 3&B: &w%-24.24s &CWage&B: &Y%s\r\n", clan->r3, num_punct( clan->level3wage ) );
   if( clan->roster_3[0] == '\0' )
	  send_to_char( "\r\n", ch );
   else
   {
	  snprintf( buf, MAX_STRING_LENGTH, "&w%s\r\n", commify_list( clan->roster_3 ) );
	  send_to_char( buf, ch );
   }
   ch_printf( ch, "&CLevel 2&B: &w%-24.24s &CWage&B: &Y%s\r\n", clan->r2, num_punct( clan->level2wage ) );
   if( clan->roster_2[0] == '\0' )
	  send_to_char( "\r\n", ch );
   else
   {
	  snprintf( buf, MAX_STRING_LENGTH, "&w%s\r\n", commify_list( clan->roster_2 ) );
	  send_to_char( buf, ch );
   }
   ch_printf( ch, "&CLevel 1&B: &w%-24.24s &CWage&B: &Y%s\r\n", clan->r1, num_punct( clan->level1wage ) );
   if( clan->roster_1[0] == '\0' )
	  send_to_char( "\r\n", ch );
   else
   {
	  snprintf( buf, MAX_STRING_LENGTH, "&w%s\r\n", commify_list( clan->roster_1 ) );
	  send_to_char( buf, ch );
   }

   send_to_char( "&B------- &CMembers Logged into the Galactic Network &B-------\r\n", ch );
   found = false;
   for( d = first_descriptor; d; d = d->next )
	  if( ( d->connected == CON_PLAYING )
		  && ( victim = d->character ) != NULL
		  && !IS_NPC( victim ) && ( victim->pcdata->clan ) != NULL && ( victim->pcdata->clan->name == clan->name ) )
	  {
		 found = true;
		 pager_printf( ch, "&C%-22.22s&B&W%s\r\n",
					   victim->name,
					   victim->pcdata->clan->leader == victim->name ? "&B(&CLeader&B)" :
					   victim->pcdata->clan->number1 == victim->name ? "&B(&CFirst&B)" :
					   victim->pcdata->clan->number2 == victim->name ? "&B(&CSecond&B)" :
					   victim->pcdata->clan->ambassador == victim->name ? "&B(&CAmbassador&B)" :
					   victim->pcdata->clan->senator == victim->name ? "&B(&CSenator&B)" :
					   victim->pcdata->clan->overseer == victim->name ? "&B(&COverseer&B)" : "" );
	  }
   if( !found )
	  send_to_char( "&CNone.\r\n", ch );

   send_to_char( "&B------------------------- &CGalactic Net Database &B-------------------------\r\n", ch );
   send_to_char( "&wDisconnected.\r\n", ch );
   return;

}
*/
