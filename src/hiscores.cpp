/*
 *  Hiscore code
 *  Author: Cronel (cronel_kal@hotmail.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *  nov 98
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 *
 */
#include <stdio.h>
#include <string.h>
#include "mud.h"

 /* data */
typedef struct hiscore_entry HISCORE_ENTRY;
typedef struct hiscore HISCORE;
struct hiscore_entry
{
	const char *name;
	int score;

	HISCORE_ENTRY *next;
	HISCORE_ENTRY *prev;
};
struct hiscore
{
	const char *keyword;
	const char *desc;
	int vnum;
	short max_length;
	short length;
	HISCORE_ENTRY *first_entry;  /* first entry is number 1, highest score */
	HISCORE_ENTRY *last_entry;   /* last entry is number 10 or last, lowest */

	HISCORE *next;
	HISCORE *prev;
};

HISCORE *first_table;
HISCORE *last_table;

/* local function declarations */
bool add_hiscore( const char *keyword, const char *name, int score );
short get_position( const char *keyword, const char *name );

void fix_table_length( HISCORE *table );
void create_hitable( const char *keyword, const char *desc );
bool destroy_hitable( const char *keyword );
HISCORE *find_table( const char *keyword );
HISCORE *load_hiscore_table( FILE *fp );



CMDF( do_hiscoset )
/* Hiscoset command */
{
	char cmd[MAX_INPUT_LENGTH];
	char kwd[MAX_INPUT_LENGTH];
	char set_val[MAX_INPUT_LENGTH];
	HISCORE *table;
	HISCORE_ENTRY *entry;
	short i;
	bool cmd_found;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: hiscoset <command> [table keyword] [field] [value]\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "    Command being one of:\r\n", ch );
		send_to_char( "    list                     Lists all hiscore tables.\r\n", ch );
		send_to_char( "    show <kwd>               Shows the data for that table.\r\n", ch );
		send_to_char( "    create <kwd>             Create new table with that keyword.\r\n", ch );
		send_to_char( "    destroy <kwd>            Destroy table with that keyword.\r\n", ch );
		send_to_char( "    set <kwd> <field> <val>  Sets data for that table.\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "    Field being one of:\r\n", ch );
		send_to_char( "    keyword <str>            Set new keyword for the table.\r\n", ch );
		send_to_char( "    vnum <num>               Set an object vnum for this table.\r\n", ch );
		send_to_char( "    desc <str>               Set the description.\r\n", ch );
		send_to_char( "    maxlength <num>          Set Max entries in the table.\r\n", ch );
		send_to_char( "    entry <name> <score>     Change the score for a player.\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "    See HELP HISCOSET for more details.\r\n", ch );
		return;
	}

	argument = one_argument( argument, cmd );

	if( !str_cmp( cmd, "list" ) )
	{
		send_to_char( "List of hiscore tables: \r\n", ch );
		for( table = first_table; table; table = table->next )
		{
			ch_printf( ch, "    Table '%s' (%s), %d entries, vnum is %d\r\n",
				table->keyword, table->desc, table->length, table->vnum );
		}
		if( !first_table )
			send_to_char( "    No tables found.\r\n", ch );
		return;
	}

	argument = one_argument( argument, kwd );
	if( kwd[0] != '\0' )
		table = find_table( kwd );
	else
		table = NULL;
	cmd_found = false;

	if( !str_cmp( cmd, "create" ) && ( cmd_found = true ) && kwd[0] != '\0' )
	{
		if( table != NULL )
		{
			send_to_char( "A table with that keyword already exists.\r\n", ch );
			return;
		}
		create_hitable( kwd, "Description not set yet" );
		ch_printf( ch, "Table '%s' created.\r\n", kwd );
		save_hiscores( );
		return;
	}
	else if( !str_cmp( cmd, "show" ) && ( cmd_found = true ) && table != NULL )
	{
		ch_printf( ch, "Hiscore table, keyword '%s':\r\n", kwd );
		ch_printf( ch, "Description: %s\r\n", table->desc );
		ch_printf( ch, "Vnum: %d    MaxLength: %d Length: %d\r\n", table->vnum, table->max_length, table->length );
		for( entry = table->first_entry, i = 1; entry; entry = entry->next, i++ )
			ch_printf( ch, "%d. Name: %-15.15s    Score: %d\r\n", i, capitalize( entry->name ), entry->score );
		return;
	}
	else if( !str_cmp( cmd, "destroy" ) && ( cmd_found = true ) && table != NULL )
	{
		if( destroy_hitable( kwd ) )
			ch_printf( ch, "Table '%s' destroyed.\r\n", kwd );
		else
		{
			ch_printf( ch, "ERROR: Couldn't destroy table '%s'.\r\n", kwd );
			LINK( table, first_table, last_table, next, prev );
		}
		save_hiscores( );
		return;
	}
	else if( !str_cmp( cmd, "set" ) )
		cmd_found = true;

	if( !cmd_found )
	{
		ch_printf( ch, "Invalid command '%s' for hiscoset.\r\n", cmd );
		return;
	}
	if( kwd[0] == '\0' )
	{
		send_to_char( "Please specify a table keyword.\r\n", ch );
		return;
	}
	if( table == NULL )
	{
		ch_printf( ch, "No such table '%s'.\r\n", kwd );
		return;
	}

	/*
	 * If it reached here, its "hiscoset set" and table is valid
	 */
	argument = one_argument( argument, cmd );
	if( cmd[0] == '\0' )
	{
		send_to_char( "Hiscoset set what?\r\n", ch );
		return;
	}

	cmd_found = false;
	if( !str_cmp( cmd, "desc" ) && ( cmd_found = true ) && argument[0] != '\0' )
	{
		STRFREE( table->desc );
		table->desc = STRALLOC( argument );
		ch_printf( ch, "Ok. Description for table '%s' is now '%s'.\r\n", kwd, argument );
		save_hiscores( );
		return;
	}
	argument = one_argument( argument, set_val );
	if( !str_cmp( cmd, "keyword" ) && ( cmd_found = true ) && set_val[0] != '\0' )
	{
		STRFREE( table->keyword );
		table->keyword = STRALLOC( set_val );
		ch_printf( ch, "Ok. Keyword for table '%s' is now '%s'.\r\n", kwd, set_val );
		save_hiscores( );
		return;
	}
	else if( !str_cmp( cmd, "vnum" ) && ( cmd_found = true ) && set_val[0] != '\0' )
	{
		if( is_number( set_val ) )
		{
			table->vnum = atoi( set_val );
			ch_printf( ch, "Ok. Vnum for table '%s' is now '%d'.\r\n", kwd, table->vnum );
			save_hiscores( );
		}
		else
			send_to_char( "Argument for set vnum must be numeric.\r\n", ch );
		return;
	}
	else if( !str_cmp( cmd, "maxlength" ) && ( cmd_found = true ) && set_val[0] != '\0' )
	{
		if( is_number( set_val ) )
		{
			table->max_length = atoi( set_val );
			ch_printf( ch, "Ok. Max length for table '%s' is now '%d'.\r\n", kwd, table->max_length );
			fix_table_length( table );
			save_hiscores( );
		}
		else
			send_to_char( "Argument for set maxlength must be numeric.\r\n", ch );
		return;
	}
	else if( !str_cmp( cmd, "entry" ) )
		cmd_found = true;

	if( !cmd_found )
	{
		ch_printf( ch, "Invalid value '%s' for hiscoset set.\r\n", cmd );
		return;
	}
	if( set_val[0] == '\0' )
	{
		ch_printf( ch, "Hiscoset set %s to what?.\r\n", cmd );
		return;
	}

	/*
	 * at this point, its hiscoset set entry ..
	 */
	if( *argument == '\0' || !is_number( argument ) )
	{
		send_to_char( "Second argument to set entry must be numberic.\r\n", ch );
		return;
	}

	ch_printf( ch, "New score for %s set to %s.\r\n", set_val, argument );

	if( add_hiscore( kwd, set_val, atoi( argument ) ) )
	{
		ch_printf( ch, "New position is %d.\r\n", get_position( kwd, set_val ) );
	}
	else
		ch_printf( ch, "They are out of the table.\r\n" );
	save_hiscores( );
}

CMDF( do_hiscore )
/* Hiscore command */
{
	HISCORE *table;
	bool some_table;
	OBJ_INDEX_DATA *oindex;

	if( argument[0] == '\0' )
	{
		some_table = false;
		//      send_to_char( "Hi\r\n", ch );
		send_to_char( "\r\n&B================================================\r\n", ch );
		for( table = first_table; table; table = table->next )
		{
			if( table->vnum == -1 )
			{
				ch_printf( ch, "&B|  &G%-10s  &g- &G%-30s&B|\r\n", capitalize( table->keyword ), table->desc );
				some_table = true;
			}
			else
			{
				oindex = get_obj_index( table->vnum );
				if( oindex != NULL )
				{
					ch_printf( ch, "  %s: in object %s\r\n", table->desc, oindex->short_descr );
					some_table = true;
				}
			}
		}
		send_to_char( "&B================================================\r\n", ch );
		if( !some_table )
			send_to_char( "  None.\r\n", ch );
		return;
	}

	table = find_table( argument );
	if( table == NULL )
	{
		send_to_char( "No such hiscore table.\r\n", ch );
		return;
	}

	if( table->vnum != -1 && !IS_IMMORTAL( ch ) )
	{
		send_to_char( "That hiscore table is attached to an object. Go see the object.\r\n", ch );
		return;
	}

	show_hiscore( table->keyword, ch );
}

const char *is_hiscore_obj( OBJ_DATA *obj )
/* If the given object is marked as a hiscore table object, it returns
 * a pointer to the keyword for that table; otherwise returns NULL */
{
	HISCORE *table;

	for( table = first_table; table; table = table->next )
	{
		if( obj->pIndexData->vnum == table->vnum )
			return table->keyword;
	}
	return NULL;
}

/*
(@)                                                 (@)
| |-------------------------------------------------| |
| |                                                 | |
| |      Greatest Pkillers of Frozen Wasteland      | |
| |                                                 | |
| |      1. Darklord ..........|.......... 1000     | |
| |      2. Pepto .............|............ 50     | |
| |     10. 12345678901234567890123456789012345     | |
| |     1234                                        | |
| |-------------------------------------------------| |
(@)                                                 (@)
   width =
01234567890123456789012345678901234567890123456789012345678901234567890123456789
0         1         2         3         4         5         6         7
3 + 5 + text + 5 + 3 = 16 + text
text = 4 + 20 + 15 = 39
39 + 16 = 49+6 = 55
*/
/* this is width of text only,
 * add 13 to get actual width */

void show_hiscore( const char *keyword, CHAR_DATA *ch )
/* Shows a hiscore table to a player in a (more or less) nicely formatted
 * way. */
{
	HISCORE *table;
	HISCORE_ENTRY *entry;
	int num, len;
	char buf[MAX_STRING_LENGTH];

	table = find_table( keyword );
	if( table == NULL )
	{
		bug( "show_hiscore: no such table '%s'", keyword );
		return;
	}

	ch_printf( ch,
		"\r\n&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-\r\n" );

	snprintf( buf, MAX_STRING_LENGTH, "&O[&Y %s &O]", table->desc );
	len = strlen_color( buf );
	len = len / 2;
	len = 32 - len;
	if( len < 0 )
		len = 0;
	for( num = 0; num < len; num++ )
		buf[num] = ' ';
	buf[num] = '\0';
	ch_printf( ch, "%s&O[&Y %s &O]\r\r\n\n", buf, table->desc );

	num = 1;
	for( entry = table->first_entry; entry; entry = entry->next )
	{
		ch_printf( ch, "            &G%s%2d&g. &R%-20.20s&P%15d     \r\n",
			num <= 3 ? "" : "", num, capitalize( entry->name ), entry->score );
		num++;
	}

	ch_printf( ch,
		"\r\n&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-&c-&C=&B/&z><&B\\&C=&c-\r\n" );
	return;
}

void adjust_hiscore( const char *keyword, CHAR_DATA *ch, int score )
/* Adjusts the hiscore for that character in that table
 * and sends message to the character and to the whole mud
 * if the player entered the table */
{
	//  char buf[ MAX_STRING_LENGTH ];
	short pos, old_pos;
	HISCORE *table;

	old_pos = get_position( keyword, ch->name );
	add_hiscore( keyword, ch->name, score );
	pos = get_position( keyword, ch->name );
	table = find_table( keyword );
	if( pos != -1 && pos != old_pos && table != NULL )
	{
		//      ch_printf( ch, "&B&WYou are now &w%d&B&W in the &w%s&B&W chart&B&W.\r\n",
		//          pos, table->desc );
			  /*
			   * replace for info channel
			   */
			   //      snprintf( buf, MAX_STRING_LENGTH, "&B&w%s&B&W is now &B&w%d&B&W in the &B&w%s&B&W chart&B&w!",
			   //          ch->name, pos, table->desc );
			   //      rank_chan( buf );
	}
	else if( pos != -1 && table != NULL )
	{
		//      ch_printf( ch, "&B&WYou remain &B&w%d&B&W in the &B&w%s&B&W chart&B&W!\r\n",
		//          pos, table->desc );
	}
}

bool add_hiscore( const char *keyword, const char *name, int score )
/* Sets the score for 'name'. If name is already on the table, the old
 * score is discarded and the new one takes effect.
 * Returns true if as a result of this call, name was added to the
 * table or remained there. */
{
	HISCORE *table;
	HISCORE_ENTRY *entry, *new_entry;
	short i;
	bool added;

	table = find_table( keyword );
	if( table == NULL )
	{
		bug( "add_hiscore: table '%s' not found", keyword );
		return false;
	}

	for( entry = table->first_entry; entry; entry = entry->next )
	{
		if( !str_cmp( entry->name, name ) )
		{
			UNLINK( entry, table->first_entry, table->last_entry, next, prev );
			STRFREE( entry->name );
			DISPOSE( entry );
			table->length--;
			break;
		}
	}

	added = false;
	new_entry = NULL;
	for( i = 1, entry = table->first_entry; i <= table->max_length; entry = entry->next, i++ )
	{
		if( !entry )
			/*
			 * there are empty slots at end of list, add there
			 */
		{
			CREATE( new_entry, HISCORE_ENTRY, 1 );
			new_entry->name = STRALLOC( name );
			new_entry->score = score;
			LINK( new_entry, table->first_entry, table->last_entry, next, prev );
			table->length++;
			added = true;
			break;
		}
		else if( score > entry->score )
		{
			CREATE( new_entry, HISCORE_ENTRY, 1 );
			new_entry->name = STRALLOC( name );
			new_entry->score = score;
			INSERT( new_entry, entry, table->first_entry, next, prev );
			table->length++;
			added = true;
			break;
		}
	}

	fix_table_length( table );

	if( added )
		save_hiscores( );

	return added;
}


void fix_table_length( HISCORE *table )
/* Chops entries at the end of the table if the table
 * has exceeded its maximum length */
{
	HISCORE_ENTRY *entry;

	while( table->length > table->max_length )
	{
		table->length--;
		entry = table->last_entry;
		UNLINK( entry, table->first_entry, table->last_entry, next, prev );
		STRFREE( entry->name );
		DISPOSE( entry );
	}
}

short get_position( const char *keyword, const char *name )
/* Returns the position of 'name' within the table of that keyword, or
 * -1 if 'name' is not in that table */
{
	short i;
	HISCORE *table;
	HISCORE_ENTRY *entry;

	table = find_table( keyword );
	if( !table )
		return -1;

	i = 1;
	for( entry = table->first_entry; entry; entry = entry->next )
	{
		if( !str_cmp( entry->name, name ) )
			return i;
		i++;
	}

	return -1;
}

HISCORE *find_table( const char *keyword )
/* Returns a pointer to the table for the given keyword
 * or NULL if there's no such table */
{
	HISCORE *table;

	for( table = first_table; table; table = table->next )
	{
		if( !str_cmp( table->keyword, keyword ) )
			return table;
	}
	return NULL;
}

void create_hitable( const char *keyword, const char *desc )
/* Creates a new hiscore table with the given keyword and description */
{
	HISCORE *new_table;

	CREATE( new_table, HISCORE, 1 );

	new_table->keyword = STRALLOC( keyword );
	new_table->desc = STRALLOC( desc );
	new_table->vnum = -1;
	new_table->max_length = 10;
	new_table->length = 0;
	new_table->first_entry = NULL;
	new_table->last_entry = NULL;
	LINK( new_table, first_table, last_table, next, prev );
}

bool destroy_hitable( const char *keyword )
/* Destroyes a given hiscore table. Returns false if error */
{
	HISCORE *table;
	HISCORE_ENTRY *entry, *nentry;

	table = find_table( keyword );
	if( !table )
		return false;

	UNLINK( table, first_table, last_table, next, prev );
	STRFREE( table->keyword );
	STRFREE( table->desc );
	for( entry = table->first_entry; entry; entry = nentry )
	{
		nentry = entry->next;
		UNLINK( entry, table->first_entry, table->last_entry, next, prev );
		STRFREE( entry->name );
		DISPOSE( entry );
	}
	DISPOSE( table );
	return true;
}

void save_hiscores( void )
/* Saves all hiscore tables */
{
	FILE *fp;
	char filename[MAX_INPUT_LENGTH];
	HISCORE *table;
	HISCORE_ENTRY *entry;

	snprintf( filename, MAX_INPUT_LENGTH, "%shiscores.dat", SYSTEM_DIR );

	fp = FileOpen( filename, "w" );
	if( fp == NULL )
	{
		bug( "save_hiscores: FileOpen" );
		return;
	}

	fprintf( fp, "#HISCORES\n" );

	for( table = first_table; table; table = table->next )
	{
		fprintf( fp, "#TABLE\n" );
		fprintf( fp, "Keyword    %s\n", table->keyword );
		fprintf( fp, "Desc       %s~\n", table->desc );
		fprintf( fp, "Vnum       %d\n", table->vnum );
		fprintf( fp, "MaxLength  %d\n", table->max_length );
		for( entry = table->first_entry; entry; entry = entry->next )
		{
			fprintf( fp, "Entry      %s %d\n", entry->name, entry->score );
		}
		fprintf( fp, "End\n\n" );
	}

	fprintf( fp, "#END\n" );

	FileClose( fp );
}

void load_hiscores( void )
/* Loads all hiscore tables */
{
	char filename[MAX_INPUT_LENGTH];
	FILE *fp;
	HISCORE *new_table;

	snprintf( filename, MAX_INPUT_LENGTH, "%shiscores.dat", SYSTEM_DIR );

	fp = FileOpen( filename, "r" );
	if( fp == NULL )
		return;

	for( ;; )
	{
		char letter;
		char *word;

		letter = fread_letter( fp );

		if( letter != '#' )
		{
			bug( "load_hiscores: # not found" );
			return;
		}

		word = fread_word( fp );

		if( !str_cmp( word, "HISCORES" ) )
			;
		else if( !str_cmp( word, "END" ) )
			break;
		else if( !str_cmp( word, "TABLE" ) )
		{
			new_table = load_hiscore_table( fp );
			if( !new_table )
				continue;
			LINK( new_table, first_table, last_table, next, prev );
		}
		else
		{
			bug( "load_hiscores: unknown field" );
			break;
		}
	}

	FileClose( fp );
	return;
}

HISCORE *load_hiscore_table( FILE *fp )
/* Loads one hiscore table from the file and returns it */
{
	char *word;
	HISCORE *new_table;
	HISCORE_ENTRY *new_entry;
	short entry_count;

	CREATE( new_table, HISCORE, 1 );
	entry_count = 0;
	for( ;; )
	{
		word = fread_word( fp );
		if( !str_cmp( word, "End" ) )
			break;
		else if( !str_cmp( word, "Keyword" ) )
			new_table->keyword = STRALLOC( fread_word( fp ) );
		else if( !str_cmp( word, "Desc" ) )
			new_table->desc = fread_string( fp );
		else if( !str_cmp( word, "Vnum" ) )
			new_table->vnum = fread_number( fp );
		else if( !str_cmp( word, "MaxLength" ) )
			new_table->max_length = fread_number( fp );
		else if( !str_cmp( word, "Entry" ) )
		{
			CREATE( new_entry, HISCORE_ENTRY, 1 );
			new_entry->name = STRALLOC( fread_word( fp ) );
			new_entry->score = fread_number( fp );
			/*
			 * LINK adds at the end of the table.
			 */
			LINK( new_entry, new_table->first_entry, new_table->last_entry, next, prev );
			entry_count++;
		}
		else
		{
			bug( "load_hiscore_table: unknown field" );
			break;
		}
	}
	new_table->length = entry_count;
	if( entry_count > new_table->max_length )
	{
		bug( "load_hiscore_table: extra entries in table. fixed max_length" );
		new_table->max_length = entry_count;
	}

	if( new_table->keyword == NULL )
		new_table->keyword = STRALLOC( "error" );
	if( new_table->desc == NULL )
		new_table->desc = STRALLOC( "Error: no description" );

	return new_table;
}
