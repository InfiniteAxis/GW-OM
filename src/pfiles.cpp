/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud (c)1997-2002 Alsherok. Contributors: Samson, Dwip, Whir,          *
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
 ****************************************************************************
 *                          Pfile Pruning Module                            *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mud.h"

 /* Globals */
time_t pfile_time;
HOUR_MIN_SEC set_pfile_time_struct;
HOUR_MIN_SEC *set_pfile_time;
struct tm *new_pfile_time;
struct tm new_pfile_struct;
time_t new_pfile_time_t;
short num_pfiles;  /* Count up number of pfiles */

void save_timedata( void )
{
	FILE *fp;
	char filename[MIL];

	snprintf( filename, MIL, "%stime.dat", SYSTEM_DIR );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_timedata: FileOpen" );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#TIME\n" );
		fprintf( fp, "Purgetime %ld\n", new_pfile_time_t );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
	}
	FileClose( fp );
	return;
}

/* Reads the actual time file from disk - Samson 1-21-99 */
void fread_timedata( FILE *fp )
{
	const char *word = NULL;
	bool fMatch = false;

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

		case 'E':
			if( !str_cmp( word, "End" ) )
				return;
			break;

		case 'P':
			KEY( "Purgetime", new_pfile_time_t, fread_number( fp ) );
			break;
		}

		if( !fMatch )
		{
			bug( "Fread_timedata: no match: %s", word );
			fread_to_eol( fp );
		}
	}
}

bool load_timedata( void )
{
	char filename[MIL];
	FILE *fp;
	bool found;

	found = false;
	snprintf( filename, MIL, "%stime.dat", SYSTEM_DIR );

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{

		found = true;
		for( ;; )
		{
			char letter = '\0';
			char *word = NULL;

			letter = fread_letter( fp );
			if( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_timedata: # not found." );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "TIME" ) )
			{
				fread_timedata( fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_timedata: bad section - %s.", word );
				break;
			}
		}
		FileClose( fp );
	}

	return found;
}

void init_pfile_scan_time( void )
{
	/*
	 * Init pfile scan time.
	 */
	set_pfile_time = &set_pfile_time_struct;

	new_pfile_time = update_time( localtime( &current_time ) );
	/*
	 * Copies *new_pfile_time to new_pfile_struct, and then points
	 * new_pfile_time to new_pfile_struct again. -- Alty
	 */
	new_pfile_struct = *new_pfile_time;
	new_pfile_time = &new_pfile_struct;
	new_pfile_time->tm_mday += 1;
	if( new_pfile_time->tm_hour > 12 )
		new_pfile_time->tm_mday += 1;
	new_pfile_time->tm_sec = 0;
	new_pfile_time->tm_min = 0;
	new_pfile_time->tm_hour = 3;

	/*
	 * Update new_pfile_time (due to day increment)
	 */
	new_pfile_time = update_time( new_pfile_time );
	new_pfile_struct = *new_pfile_time;
	new_pfile_time = &new_pfile_struct;
	/*
	 * Bug fix submitted by Gabe Yoder
	 */
	new_pfile_time_t = mktime( new_pfile_time );
	/*
	 * check_pfiles(mktime(new_pfile_time));
	 */

	if( !load_timedata( ) )
	{
		strcpy( log_buf, "Pfile scan time reset to default time of 3am." );
		log_string( log_buf );
	}
	return;
}

time_t now_time;
short deleted = 0;
short days = 0;

void fread_pfile( FILE *fp, time_t tdiff, const char *fname, bool count )
{
	const char *word;
	const char *name = NULL;
	const char *clan = NULL;
	short toplevel = 0;
	int pact;
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
			KEY( "Clan", clan, fread_string( fp ) );
			break;

		case 'E':
			if( !strcmp( word, "End" ) )
				goto timecheck;
			break;

		case 'F':
			KEY( "Flags", pact, fread_number( fp ) );
			break;

		case 'N':
			KEY( "Name", name, fread_string( fp ) );
			break;

		case 'T':
			KEY( "Toplevel", toplevel, fread_number( fp ) );
			break;
		}

		if( !fMatch )
			fread_to_eol( fp );
	}

timecheck:

	if( count == false && !IS_SET( pact, PCFLAG_EXEMPT ) )
	{
		if( toplevel < 5 && tdiff > sysdata.newbie_purge )
		{
			if( unlink( fname ) == -1 )
				perror( "Unlink" );
			else
			{
				days = sysdata.newbie_purge;
				snprintf( log_buf, MAX_STRING_LENGTH, "Player %s was deleted. Exceeded time limit of %d days.", name, days );
				log_string( log_buf );
#ifdef AUTO_AUTH
				remove_from_auth( name );
#endif
				deleted++;
				return;
			}
		}

		if( toplevel < LEVEL_STAFF && tdiff > sysdata.regular_purge )
		{
			if( toplevel < LEVEL_STAFF )
			{
				if( unlink( fname ) == -1 )
					perror( "Unlink" );
				else
				{
					days = sysdata.regular_purge;
					snprintf( log_buf, MAX_STRING_LENGTH, "Player %s was deleted. Exceeded time limit of %d days.", name, days );
					log_string( log_buf );
#ifdef AUTO_AUTH
					remove_from_auth( name );
#endif
					deleted++;
					return;
				}
			}
		}
	}

	if( clan != NULL )
	{
		CLAN_DATA *guild = get_clan( clan );
		char clanmemberlist[MAX_STRING_LENGTH];
		char buf[MAX_STRING_LENGTH];

		if( !guild )
			return;

		if( guild->shortname[0] != '\0' && toplevel < LEVEL_STAFF )
		{
			snprintf( clanmemberlist, MAX_STRING_LENGTH, "%s%s.list", CLAN_DIR, guild->shortname );
			snprintf( buf, MAX_STRING_LENGTH, "%s~", name );
			append_to_file( clanmemberlist, buf );
		}


		guild->members++;
		save_clan( guild );
	}

	return;
}

void read_pfile( const char *dirname, const char *filename, bool count )
{
	FILE *fp;
	char fname[MSL];
	struct stat fst;
	time_t tdiff;

	now_time = time( 0 );

	snprintf( fname, MSL, "%s/%s", dirname, filename );

	if( !str_cmp( filename, ".clone" ) || !str_cmp( filename, ".home" ) )
		return;

	if( stat( fname, &fst ) != -1 )
	{
		tdiff = ( now_time - fst.st_mtime ) / 86400;

		if( ( fp = FileOpen( fname, "r" ) ) != NULL )
		{
			for( ;; )
			{
				char letter;
				const char *word;

				letter = fread_letter( fp );

				if( ( letter != '#' ) && ( !feof( fp ) ) )
					continue;

				word = feof( fp ) ? "End" : fread_word( fp );

				if( !str_cmp( word, "End" ) )
					break;

				if( !str_cmp( word, "PLAYER" ) )
					fread_pfile( fp, tdiff, fname, count );
				else if( !str_cmp( word, "END" ) )  /* Done     */
					break;
			}
			FileClose( fp );
		}
	}
	return;
}

void pfile_scan( bool count )
{
	DIR *dp;
	struct dirent *dentry;
	CLAN_DATA *clan;
	char directory_name[100];
	char buf[MAX_STRING_LENGTH];

	short alpha_loop;
	short cou = 0;
	deleted = 0;

	now_time = time( 0 );
	nice( 20 );

	/*
	 * Reset all clans to 0 members prior to scan - Samson 7-26-00
	 */

	for( clan = first_clan; clan; clan = clan->next )
	{
		clan->members = 0;
		snprintf( buf, MAX_STRING_LENGTH, "%s%s.list", CLAN_DIR, clan->shortname );
		remove( buf );
	}

	for( alpha_loop = 0; alpha_loop <= 25; alpha_loop++ )
	{
		snprintf( directory_name, 100, "%s%c", PLAYER_DIR, 'a' + alpha_loop );
		/*
		 * log_string( directory_name );
		 */
		dp = opendir( directory_name );
		dentry = readdir( dp );
		while( dentry )
		{
			if( dentry->d_name[0] != '.' && str_cmp( dentry->d_name, ".clone" ) && str_cmp( dentry->d_name, ".home" ) )
			{
				read_pfile( directory_name, dentry->d_name, count );
				cou++;
			}
			dentry = readdir( dp );
		}
		closedir( dp );
	}

	if( !count )
		log_string( "Pfile cleanup completed." );
	else
		log_string( "Pfile count completed." );

	snprintf( log_buf, MAX_STRING_LENGTH, "Total pfiles scanned: %d", cou );
	log_string( log_buf );

	if( !count )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "Total pfiles deleted: %d", deleted );
		log_string( log_buf );

		snprintf( log_buf, MAX_STRING_LENGTH, "Total pfiles remaining: %d", cou - deleted );
		num_pfiles = cou - deleted;
		log_string( log_buf );
	}
	else
		num_pfiles = cou;

	return;
}

CMDF( do_pfiles )
{
	char buf[MSL];

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobs cannot use this command!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' || !argument )
	{
		/*
		 * Makes a backup copy of existing pfiles just in case - Samson
		 */
		snprintf( buf, MAX_STRING_LENGTH, "tar -cf %spfiles.tar %s*", PLAYER_DIR, PLAYER_DIR );

		/*
		 * GAH, the shell pipe won't process the command that gets pieced
		 * together in the preceeding lines! God only knows why. - Samson
		 */
		system( buf );

		snprintf( log_buf, MAX_STRING_LENGTH, "Manual pfile cleanup started by %s.", ch->name );
		log_string( log_buf );
		pfile_scan( false );
#ifdef SAMSONRENT
		rent_update( );
#endif
		return;
	}

	if( !str_cmp( argument, "settime" ) )
	{
		new_pfile_time_t = current_time + 86400;
		save_timedata( );
		send_to_char( "New cleanup time set for 24 hrs from now.\r\n", ch );
		return;
	}

	if( !str_cmp( argument, "count" ) )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "Pfile count started by %s.", ch->name );
		log_string( log_buf );
		pfile_scan( true );
		return;
	}

	send_to_char( "Invalid argument.\r\n", ch );
	return;
}

void check_pfiles( time_t reset )
{
	/*
	 * This only counts them up on reboot if the cleanup isn't needed - Samson 1-2-00
	 */
	if( reset == 255 && new_pfile_time_t > current_time )
	{
		reset = 0;    /* Call me paranoid, but it might be meaningful later on */
		log_string( "Counting pfiles....." );
		pfile_scan( true );
		return;
	}

	if( new_pfile_time_t <= current_time )
	{
		if( sysdata.CLEANPFILES == true )
		{

			char buf[MSL];

			/*
			 * Makes a backup copy of existing pfiles just in case - Samson
			 */
			snprintf( buf, MAX_STRING_LENGTH, "tar -cf %spfiles.tar %s*", PLAYER_DIR, PLAYER_DIR );

			/*
			 * Would use the shell pipe for this, but alas, it requires a ch in order
			 * to work, this also gets called during boot_db before the rare item
			 * checks for the rent code - Samson
			 */
			system( buf );

			new_pfile_time_t = current_time + 86400;
			save_timedata( );
			log_string( "Automated pfile cleanup beginning...." );
			pfile_scan( false );
#ifdef SAMSONRENT
			if( reset == 0 )
				rent_update( );
#endif
		}
		else
		{
			new_pfile_time_t = current_time + 86400;
			save_timedata( );
			log_string( "Counting pfiles....." );
			pfile_scan( true );
#ifdef SAMSONRENT
			if( reset == 0 )
				rent_update( );
#endif
		}
	}
	return;
}

void add_member( const char *name, const char *shortname )
{
	char buf[MAX_STRING_LENGTH];
	char fbuf[MAX_STRING_LENGTH];

	if( name[0] == '\0' || !name )
	{
		bug( "add_member: No name!\r\n" );
		return;
	}

	if( shortname[0] == '\0' || !shortname )
	{
		bug( "add_member: No shortname!\r\n" );
		return;
	}

	snprintf( fbuf, MSL, "%s%s.list", CLAN_DIR, shortname );
	snprintf( buf, MAX_STRING_LENGTH, "%s~", name );
	append_to_file( fbuf, buf );

}

void remove_member( const char *name, const char *shortname )
{
	FILE *fpList;
	FILE *fpNew;
	const char *buf;
	char list[MAX_STRING_LENGTH];
	char temp[MAX_STRING_LENGTH];

	if( name[0] == '\0' )
	{
		bug( "remove_member: No name!\r\n" );
		return;
	}

	if( shortname[0] == '\0' || !shortname )
	{
		bug( "remove_member: No shortname!\r\n" );
		return;
	}

	snprintf( list, MAX_STRING_LENGTH, "%s%s.list", CLAN_DIR, shortname );
	snprintf( temp, MAX_STRING_LENGTH, "%s.temp", list );

	if( ( fpList = FileOpen( list, "r" ) ) == NULL )
	{
		bug( "Unable to open member list" );
		return;
	}

	if( ( fpNew = FileOpen( temp, "w" ) ) == NULL )
	{
		bug( "remove_member: Unable to write temp list" );
		return;
	}

	for( ;; )
	{
		if( feof( fpList ) )
			break;
		buf = feof( fpList ) ? "End" : fread_string( fpList );
		if( !str_cmp( buf, "End" ) || buf[0] == '\0' )
			break;
		if( str_cmp( name, buf ) && strlen( buf ) > 2 )
			fprintf( fpNew, "%s~\n", buf );
	}
	FileClose( fpNew );
	FileClose( fpList );
	rename( temp, list );
}
