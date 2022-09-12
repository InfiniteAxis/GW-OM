/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"
/* #include "do.h" *//*
 * My do_XXX functions are declared in this file
 */
#define is_full_name is_name

 /*

  Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@andreasen.org
  =====================================================================

  Basically, the notes are split up into several boards. The boards do not
  exist physically, they can be read anywhere and in any position.

  Each of the note boards has its own file. Each of the boards can have its own
  "rights": who can read/write.

  Each character has an extra field added, namele the timestamp of the last note
  read by him/her on a certain board.

  The note entering system is changed too, making it more interactive. When
  entering a note, a character is put AFK and into a special CON_ state.
  Everything typed goes into the note.

  For the immortals it is possible to purge notes based on age. An Archive
  options is available which moves the notes older than X days into a special
  board. The file of this board should then be moved into some other directory
  during e.g. the startup script and perhaps renamed depending on date.

  Note that write_level MUST be >= read_level or else there will be strange
  output in certain functions.

  Board DEFAULT_BOARD must be at least readable by *everyone*.

 */
NOTE_DATA *noteFree;

#define L_SUP (MAX_LEVEL - 1)   /* if not already defined */

BOARD_DATA boards[MAX_BOARD] = {
   { "Notes",		"", 0, 2,				"all", DEF_INCLUDE, 21, NULL, NULL, false },
   { "Ideas",		"", 0, 2,				"all", DEF_NORMAL,  60, NULL, NULL, false },
   { "Articles",	"", 0, LEVEL_STAFF,	"all", DEF_NORMAL,  60, NULL, NULL, false },
   { "Mistakes",	"", 0, 1,				"imm", DEF_NORMAL,  60, NULL, NULL, false },
   { "Personal",	"", 0, 1,				"all", DEF_EXCLUDE, 28, NULL, NULL, false }
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char *szFinishPrompt = "[C]ontinue, [V]iew, [P]ost, or [F]orget it?";
//const char * szFinishPrompt = GREY "[" HPURPLE "C" GREY "]" SPURPLE "continue";
long last_note_stamp = 0;   /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

// static bool next_board (CHAR_DATA *ch);

/* recycle a note */
void free_note( NOTE_DATA *note )
{
	STRFREE( note->sender );
	STRFREE( note->to_list );
	STRFREE( note->subject );
	STRFREE( note->date );
	STRFREE( note->text );
	DISPOSE( note->yesvotes );
	DISPOSE( note->novotes );
	DISPOSE( note->abstentions );

	note->next = noteFree;
	noteFree = note;
}

/* allocate memory for a new note or recycle */
NOTE_DATA *new_note( )
{
	NOTE_DATA *note;

	if( noteFree )
	{
		note = noteFree;
		noteFree = noteFree->next;
	}
	else
		CREATE( note, NOTE_DATA, 1 );

	note->next = NULL;
	note->sender = NULL;
	note->expire = 0;
	note->to_list = NULL;
	note->subject = NULL;
	note->date = NULL;
	note->date_stamp = 0;
	note->text = NULL;

	return note;
}

/* append this note to the given file */
void append_note( FILE *fp, NOTE_DATA *note )
{
	fprintf( fp, "Sender  %s~\n", note->sender );
	fprintf( fp, "Date    %s~\n", note->date );
	fprintf( fp, "Stamp   %ld\n", note->date_stamp );
	fprintf( fp, "Expire  %ld\n", note->expire );
	fprintf( fp, "To      %s~\n", note->to_list );
	fprintf( fp, "Subject %s~\n", note->subject );
	fprintf( fp, "Text\n%s~\n\n", note->text );
}

/* Save a note in a given board */
void finish_note( BOARD_DATA *board, NOTE_DATA *note )
{
	FILE *fp;
	CHAR_DATA *vch;
	NOTE_DATA *p;
	char filename[200];
	char buf[MAX_STRING_LENGTH];

	/*
	 * The following is done in order to generate unique date_stamps
	 */

	if( last_note_stamp >= current_time )
		note->date_stamp = ++last_note_stamp;
	else
	{
		note->date_stamp = current_time;
		last_note_stamp = current_time;
	}

	if( board->note_first )  /* are there any notes in there now? */
	{
		for( p = board->note_first; p->next; p = p->next )
			;  /* empty */

		p->next = note;
	}
	else /* nope. empty list. */
		board->note_first = note;

	/*
	 * append note to note file
	 */

	sprintf( filename, "%s%s", NOTE_DIR, board->short_name );

	fp = FileOpen( filename, "a" );
	if( !fp )
	{
		bug( "Could not open one of the note files in append mode", 0 );
		board->changed = true;    /* set it to true hope it will be OK later? */
		return;
	}

	sprintf( buf, "&GA &BN&be&Bw&G &CP&co&Cs&ct&G has been added to &P|&p%s&P|\r\n", board->short_name );
	for( vch = first_char; vch; vch = vch->next )
	{
		if( IS_NPC( vch ) )
			continue;
		if( is_note_to( vch, note ) )
			send_to_char( buf, vch );
	}

	append_note( fp, note );
	FileClose( fp );
}

/* Find the number of a board */
int board_number( const BOARD_DATA *board )
{
	int i;

	for( i = 0; i < MAX_BOARD; i++ )
		if( board == &boards[i] )
			return i;

	return -1;
}

/* Find a board number based on  a string */
int board_lookup( const char *name )
{
	int i;

	for( i = 0; i < MAX_BOARD; i++ )
		if( !str_cmp( boards[i].short_name, name ) )
			return i;

	return -1;
}

/* Remove list from the list. Do not free note */
void unlink_note( BOARD_DATA *board, NOTE_DATA *note )
{
	NOTE_DATA *p;

	if( board->note_first == note )
		board->note_first = note->next;
	else
	{
		for( p = board->note_first; p && p->next != note; p = p->next );
		if( !p )
			bug( "unlink_note: could not find note.", 0 );
		else
			p->next = note->next;
	}
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
NOTE_DATA *find_note( CHAR_DATA *ch, BOARD_DATA *board, int num )
{
	int count = 0;
	NOTE_DATA *p;

	for( p = board->note_first; p; p = p->next )
		if( ++count == num )
			break;

	if( ( count == num ) && is_note_to( ch, p ) )
		return p;
	else
		return NULL;

}

/* save a single board */
static void save_board( BOARD_DATA *board )
{
	FILE *fp;
	char filename[200];
	char buf[200];
	NOTE_DATA *note;

	sprintf( filename, "%s%s", NOTE_DIR, board->short_name );

	fp = FileOpen( filename, "w" );
	if( !fp )
	{
		sprintf( buf, "Error writing to: %s", filename );
		bug( buf, 0 );
	}
	else
	{
		for( note = board->note_first; note; note = note->next )
			append_note( fp, note );

		FileClose( fp );
	}
}

/* Show one not to a character */
static void show_note_to_char( CHAR_DATA *ch, NOTE_DATA *note, int num )
{
	char buf[4 * MAX_STRING_LENGTH];

	/*
	 * Ugly colors ?
	 */
	sprintf( buf,
		"&C[&c%4d&C] &B%s&b: &p%s\r\n&BDate&b:&p  %s\r\n"
		"&BTo&b:&p    %s\r\n"
		"&G===========================================================================\r\n"
		"&z%s\r\n", num, note->sender, note->subject, note->date, note->to_list, note->text );

	send_to_char( buf, ch );
}

/* Save changed boards */
void save_notes( )
{
	int i;

	for( i = 0; i < MAX_BOARD; i++ )
		if( boards[i].changed )   /* only save changed boards */
			save_board( &boards[i] );
}

/* Load a single board */
void load_board( BOARD_DATA *board )
{
	FILE *fp, *fp_archive;
	NOTE_DATA *last_note;
	char filename[200];

	sprintf( filename, "%s%s", NOTE_DIR, board->short_name );

	fp = FileOpen( filename, "r" );

	/*
	 * Silently return
	 */
	if( !fp )
		return;

	/*
	 * Start note fetching. copy of db.c:load_notes()
	 */

	last_note = NULL;

	for( ;; )
	{
		NOTE_DATA *pnote;
		char letter;

		do
		{
			letter = getc( fp );
			if( feof( fp ) )
			{
				FileClose( fp );
				return;
			}
		} while( isspace( letter ) );
		ungetc( letter, fp );

		CREATE( pnote, NOTE_DATA, sizeof( *pnote ) );

		if( str_cmp( fread_word( fp ), "sender" ) )
			break;
		pnote->sender = fread_string( fp );

		if( str_cmp( fread_word( fp ), "date" ) )
			break;
		pnote->date = fread_string( fp );

		if( str_cmp( fread_word( fp ), "stamp" ) )
			break;
		pnote->date_stamp = fread_number( fp );

		if( str_cmp( fread_word( fp ), "expire" ) )
			break;
		pnote->expire = fread_number( fp );

		if( str_cmp( fread_word( fp ), "to" ) )
			break;
		pnote->to_list = fread_string( fp );

		if( str_cmp( fread_word( fp ), "subject" ) )
			break;
		pnote->subject = fread_string( fp );

		if( str_cmp( fread_word( fp ), "text" ) )
			break;
		pnote->text = fread_string( fp );

		pnote->next = NULL;   /* jic */

		/*
		 * Should this note be archived right now ?
		 */

		if( pnote->expire < current_time )
		{
			char archive_name[200];

			sprintf( archive_name, "%s%s.old", NOTE_DIR, board->short_name );
			fp_archive = FileOpen( archive_name, "a" );
			if( !fp_archive )
				bug( "Could not open archive boards for writing", 0 );
			else
			{
				append_note( fp_archive, pnote );
				FileClose( fp_archive );   /* it might be more efficient to close this later */
			}

			free_note( pnote );
			board->changed = true;
			continue;

		}


		if( board->note_first == NULL )
			board->note_first = pnote;
		else
			last_note->next = pnote;

		last_note = pnote;
	}

	bug( "Load_notes: bad key word.", 0 );
	return;  /* just return */
}

/* Initialize structures. Load all boards. */
void load_boards( )
{
	int i;

	for( i = 0; i < MAX_BOARD; i++ )
		load_board( &boards[i] );
}

/* Returns true if the specified note is address to ch */
bool is_note_to( CHAR_DATA *ch, NOTE_DATA *note )
{
	if( !str_cmp( ch->name, note->sender ) )
		return true;

	if( ch->pcdata->clan )
	{
		if( !str_cmp( ch->pcdata->clan->name, note->to_list ) )
			return true;
	}
	if( is_full_name( "all", note->to_list ) )
		return true;

	if( IS_IMMORTAL( ch ) && ( is_full_name( "imm", note->to_list ) ||
		is_full_name( "imms", note->to_list ) ||
		is_full_name( "immortal", note->to_list ) ||
		is_full_name( "god", note->to_list ) ||
		is_full_name( "gods", note->to_list ) || is_full_name( "immortals", note->to_list ) ) )
		return true;

	if( ( get_trust( ch ) == MAX_LEVEL ) && ( is_full_name( "imp", note->to_list ) ||
		is_full_name( "imps", note->to_list ) ||
		is_full_name( "implementor", note->to_list ) ||
		is_full_name( "implementors", note->to_list ) ) )
		return true;

	if( is_full_name( ch->name, note->to_list ) )
		return true;

	/*
	 * Allow a note to e.g. 40 to send to characters level 40 and above
	 */
	if( is_number( note->to_list ) && get_trust( ch ) >= atoi( note->to_list ) )
		return true;

	return false;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes( CHAR_DATA *ch, BOARD_DATA *board )
{
	NOTE_DATA *note;
	time_t last_read;
	int count = 0;

	if( board->read_level > get_trust( ch ) )
		return BOARD_NOACCESS;

	last_read = ch->pcdata->last_note[board_number( board )];

	for( note = board->note_first; note; note = note->next )
		if( is_note_to( ch, note ) && ( ( long ) last_read < ( long ) note->date_stamp ) )
			count++;

	return count;
}

/*
 * COMMANDS
 */

 /* Start writing a note */
CMDF( do_nwrite )
{
	char *strtime;
	char buf[200];

	if( IS_NPC( ch ) )   /* NPC cannot post notes */
		return;

	if( get_trust( ch ) < ch->pcdata->board->write_level )
	{
		send_to_char( "You cannot post notes on this board.\r\n", ch );
		return;
	}

	/*
	 * continue previous note, if any text was written
	 */
	if( ch->pcdata->in_progress && ( !ch->pcdata->in_progress->text ) )
	{
		send_to_char( "Note in progress cancelled because you did not manage to write any text \r\n"
			"before losing link.\r\n\r\n", ch );
		free_note( ch->pcdata->in_progress );
		ch->pcdata->in_progress = NULL;
	}

	if( !ch->pcdata->in_progress )
	{
		xSET_BIT( ch->act, PLR_AFK );
		ch->pcdata->in_progress = new_note( );
		ch->pcdata->in_progress->sender = STRALLOC( ch->name );

		/*
		 * convert to ascii. ctime returns a string which last character is \n, so remove that
		 */
		strtime = ctime( &current_time );
		strtime[strlen( strtime ) - 1] = '\0';

		ch->pcdata->in_progress->date = STRALLOC( strtime );
	}

	act( AT_GREEN, "$n starts writing a note.", ch, NULL, NULL, TO_ROOM );

	/*
	 * Begin writing the note !
	 */
	sprintf( buf, "\r\n&zYou are now %s a new note on the &B%s &zboard.\r\n",
		ch->pcdata->in_progress->text ? "continuing" : "posting", ch->pcdata->board->short_name );
	send_to_char( buf, ch );

	sprintf( buf, "&YFrom&O:&G  %s\r\n\r\n", ch->name );
	send_to_char( buf, ch );

	if( !ch->pcdata->in_progress->text ) /* Are we continuing an old note or not? */
	{
		switch( ch->pcdata->board->force_type )
		{
		case DEF_NORMAL:
			sprintf( buf, "&zIf you press enter, default recipient \" %s \" will be chosen.\r\n",
				ch->pcdata->board->names );
			break;
		case DEF_INCLUDE:
			sprintf( buf,
				"&zThe recipient list &RMUST&z include \"%s\". If not, it will be added automatically.\r\n", ch->pcdata->board->names );
			break;

		case DEF_EXCLUDE:
			sprintf( buf, "&zThe recipient of this note must &RNOT&z include: \"%s\".",
				ch->pcdata->board->names );

			break;
		}

		send_to_char( buf, ch );
		send_to_char( "\r\n&YTo&O:&G      ", ch );

		ch->desc->connected = CON_NOTE_TO;
		/*
		 * nanny takes over from here
		 */

	}
	else /* we are continuing, print out all the fields and the note so far */
	{
		sprintf( buf, "&YTo&O:&G      %s\r\n"
			"&YExpires&O:&G %s\r\n"
			"&YSubject&O:&G %s\r\n",
			ch->pcdata->in_progress->to_list,
			ctime( &ch->pcdata->in_progress->expire ), ch->pcdata->in_progress->subject );
		send_to_char( buf, ch );
		send_to_char( "&zYour note so far&W:&w\r\n", ch );
		send_to_char( ch->pcdata->in_progress->text, ch );

		send_to_char( "\r\n&zEnter text. Type &B~ &zor &BEND&z on an empty line to end note.\r\n"
			"&G=======================================================&w\r\n", ch );


		ch->desc->connected = CON_NOTE_TEXT;

	}

}


/* Read next note in current group. If no more notes, go to next board */
CMDF( do_nread )
{
	NOTE_DATA *p;
	int count = 0, number;
	time_t *last_note = &ch->pcdata->last_note[board_number( ch->pcdata->board )];

	if( !str_cmp( argument, "again" ) )
	{    /* read last note again */

	}
	else if( is_number( argument ) )
	{
		number = atoi( argument );

		for( p = ch->pcdata->board->note_first; p; p = p->next )
			if( ++count == number )
				break;

		if( !p || !is_note_to( ch, p ) )
			send_to_char( "No such note.\r\n", ch );
		else
		{
			show_note_to_char( ch, p, count );
			*last_note = UMAX( *last_note, p->date_stamp );
		}
	}
	else /* just next one */
	{

		count = 1;
		for( p = ch->pcdata->board->note_first; p; p = p->next, count++ )
			if( ( p->date_stamp > * last_note ) && is_note_to( ch, p ) )
			{
				show_note_to_char( ch, p, count );
				/*
				 * Advance if new note is newer than the currently newest for that char
				 */
				*last_note = UMAX( *last_note, p->date_stamp );
				return;
			}

		send_to_char( "No new notes in this board.\r\n", ch );
		do_unread( ch, "" );
		/*
				if (next_board (ch))
					sprintf (buf, "Changed to next board, %s.\r\n", ch->pcdata->board->short_name);
				else
					sprintf (buf, "There are no more boards.\r\n");

				send_to_char (buf,ch);
		*/
	}
}

/* Remove a note */
CMDF( do_nremove )
{
	NOTE_DATA *p;

	if( !is_number( argument ) )
	{
		send_to_char( "Remove which note?\r\n", ch );
		return;
	}

	p = find_note( ch, ch->pcdata->board, atoi( argument ) );
	if( !p )
	{
		send_to_char( "No such note.\r\n", ch );
		return;
	}

	if( str_cmp( ch->name, p->sender ) && ( get_trust( ch ) < MAX_LEVEL ) )
	{
		send_to_char( "You are not authorized to remove this note.\r\n", ch );
		return;
	}

	unlink_note( ch->pcdata->board, p );
	free_note( p );
	send_to_char( "Note removed!\r\n", ch );

	save_board( ch->pcdata->board ); /* save the board */
}


/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
CMDF( do_nlist )
{
	int count = 0, show = 0, num = 0, has_shown = 0;
//	time_t last_note;
	NOTE_DATA *p;
	char buf[MAX_STRING_LENGTH];


	if( is_number( argument ) )  /* first, count the number of notes */
	{
		show = atoi( argument );

		for( p = ch->pcdata->board->note_first; p; p = p->next )
			if( is_note_to( ch, p ) )
				count++;
	}

	send_to_char( "\r\n&B###  &CAuthor        &GSubject\r\n", ch );
	send_to_char( "&z===========================================\r\n", ch );

//	last_note = ch->pcdata->last_note[board_number( ch->pcdata->board )];

	for( p = ch->pcdata->board->note_first; p; p = p->next )
	{
		num++;
		if( is_note_to( ch, p ) )
		{
			has_shown++;   /* note that we want to see X VISIBLE note, not just last X */
			if( !show || ( ( count - show ) < has_shown ) )
			{
				sprintf( buf, "&B%3d  &C%-13s &G%s\r\n", num, p->sender, p->subject );
				send_to_char( buf, ch );
			}
		}

	}
}

/* catch up with some notes */
CMDF( do_ncatchup )
{
	NOTE_DATA *p;

	/*
	 * Find last note
	 */
	for( p = ch->pcdata->board->note_first; p && p->next; p = p->next );

	if( !p )
		send_to_char( "Alas, there are no notes in that board.\r\n", ch );
	else
	{
		ch->pcdata->last_note[board_number( ch->pcdata->board )] = p->date_stamp;
		send_to_char( "All mesages skipped.\r\n", ch );
	}
}

/* Dispatch function for backwards compatibility */
CMDF( do_note )
{
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
		return;

	if( ch->pcdata->board == NULL )
	{
		ch->pcdata->board = &boards[DEFAULT_BOARD];
	}

	argument = one_argument( argument, arg );

	if( ( !arg[0] ) || ( !str_cmp( arg, "read" ) ) )
		do_nread( ch, argument );

	else if( !str_cmp( arg, "list" ) )
		do_nlist( ch, argument );

	else if( !str_cmp( arg, "write" ) )
	{
		if( IS_SET( ch->pcdata->flags, PCFLAG_NONOTE ) )
		{
			send_to_char( "You're not allowed to write notes!\r\n", ch );
			return;
		}

		if( ch->in_room == get_room_index( 6 ) )
		{
			send_to_char( "&RYou can't write notes from hell!\r\n", ch );
			return;
		}
		do_nwrite( ch, argument );
	}

	else if( !str_cmp( arg, "remove" ) )
		do_nremove( ch, argument );

	else if( !str_cmp( arg, "catchup" ) )
		do_ncatchup( ch, argument );
	else
		do_help( ch, "note" );
}

CMDF( do_unread )
{
	int i, count;
	char buf[200];
	int unread;

	if( IS_NPC( ch ) )
		return;


	count = 1;
	send_to_char( "\r\n&G===========================================\r\n", ch );
	for( i = 0; i < MAX_BOARD; i++ )
	{
		unread = unread_notes( ch, &boards[i] );  /* how many unread notes? */
		if( unread != BOARD_NOACCESS )
		{
			sprintf( buf, "&zYou have &B%d &znew &C%s&z to read.\r\n", unread, boards[i].short_name );
			send_to_char( buf, ch );
			count++;
		}
	}
	send_to_char( "&G===========================================\r\n", ch );
	return;
}

CMDF( do_boards )
{
	int i, count, number;
	char buf[200];

	if( IS_NPC( ch ) )
		return;

	if( ch->pcdata->board == NULL )
	{
		ch->pcdata->board = &boards[DEFAULT_BOARD];
	}

	if( !argument[0] )   /* show boards */
	{
		send_to_char
		( "\r\n&BSyntax&b: &zSubject <Subject>\r\n&BSubjects&b:&z Notes, Ideas, Articles, Mistakes, Personnel.\r\n\r\n",
			ch );

		sprintf( buf, "\r\n&zYou current subject is &B%s&z.\r\n", ch->pcdata->board->short_name );
		send_to_char( buf, ch );

		/*
		 * Inform of rights
		 */
		if( ch->pcdata->board->read_level > get_trust( ch ) )
			send_to_char( "You can't read or write on this subject.\r\n", ch );
		else if( ch->pcdata->board->write_level > get_trust( ch ) )
			send_to_char( "You can only read this subject.\r\n", ch );
		else
			send_to_char( "You can read and write on this subject.\r\n", ch );

		return;
	}    /* if empty argument */

	if( ch->pcdata->in_progress )
	{
		send_to_char( "Please finish your interrupted note first.\r\n", ch );
		return;
	}

	/*
	 * Change board based on its number
	 */
	if( is_number( argument ) )
	{
		count = 0;
		number = atoi( argument );
		for( i = 0; i < MAX_BOARD; i++ )
			if( unread_notes( ch, &boards[i] ) != BOARD_NOACCESS )
				if( ++count == number )
					break;

		if( count == number ) /* found the board.. change to it */
		{
			ch->pcdata->board = &boards[i];
			sprintf( buf, "\r\n&zCurrent subject changed to &B%s&z. %s.\r\n", boards[i].short_name,
				( get_trust( ch ) < boards[i].write_level )
				? "You can only read here" : "You can both read and write here" );
			send_to_char( buf, ch );
		}
		else  /* so such board */
			send_to_char( "No such subject.\r\n", ch );

		return;
	}

	/*
	 * Non-number given, find board with that name
	 */

	for( i = 0; i < MAX_BOARD; i++ )
		if( !str_cmp( boards[i].short_name, argument ) )
			break;

	if( i == MAX_BOARD )
	{
		send_to_char( "No such Subject.\r\n", ch );
		return;
	}

	/*
	 * Does ch have access to this board?
	 */
	if( unread_notes( ch, &boards[i] ) == BOARD_NOACCESS )
	{
		send_to_char( "No such Subject.\r\n", ch );
		return;
	}

	ch->pcdata->board = &boards[i];
	sprintf( buf, "&zCurrent subject changed to &B%s&z. %s.\r\n", boards[i].short_name,
		( get_trust( ch ) < boards[i].write_level ) ? "You can only read here" : "You can both read and write here" );
	send_to_char( buf, ch );
}

/* Send a note to someone on the personal board */
void personal_message( const char *sender, const char *to, const char *subject, const int expire_days, const char *text )
{
	make_note( "Personal", sender, to, subject, expire_days, text );
}

void make_note( const char *board_name, const char *sender, const char *to, const char *subject, const int expire_days,
	const char *text )
{
	int board_index = board_lookup( board_name );
	BOARD_DATA *board;
	NOTE_DATA *note;
	char *strtime;

	if( board_index == BOARD_NOTFOUND )
	{
		bug( "make_note: board not found", 0 );
		return;
	}

	if( strlen( text ) > MAX_NOTE_TEXT )
	{
		bug( "make_note: text too long (%d bytes)", strlen( text ) );
		return;
	}


	board = &boards[board_index];

	note = new_note( ); /* allocate new note */

	note->sender = STRALLOC( ( char * ) sender );
	note->to_list = STRALLOC( ( char * ) to );
	note->subject = STRALLOC( ( char * ) subject );
	note->expire = current_time + expire_days * 60 * 60 * 24;
	note->text = STRALLOC( ( char * ) text );

	/*
	 * convert to ascii. ctime returns a string which last character is \n, so remove that
	 */
	strtime = ctime( &current_time );
	strtime[strlen( strtime ) - 1] = '\0';

	note->date = STRALLOC( strtime );

	finish_note( board, note );

}

/* tries to change to the next accessible board */
/*
bool next_board (CHAR_DATA *ch)
{
	int i = board_number (ch->pcdata->board) + 1;

	while ((i < MAX_BOARD) && (unread_notes(ch,&boards[i]) == BOARD_NOACCESS))
		i++;

	if (i == MAX_BOARD)
		return false;
	else
	{
		ch->pcdata->board = &boards[i];
		return true;
	}
}
*/
void handle_con_note_to( DESCRIPTOR_DATA *d, const char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch = d->character;

	if( !ch->pcdata->in_progress )
	{
		d->connected = CON_PLAYING;
		bug( "nanny: In CON_NOTE_TO, but no note in progress", 0 );
		return;
	}

	mudstrlcpy( buf, argument, MSL );
	smash_tilde( buf );  /* change ~ to - as we save this field as a string later */

	switch( ch->pcdata->board->force_type )
	{
	case DEF_NORMAL: /* default field */
		if( !buf[0] )  /* empty string? */
		{
			ch->pcdata->in_progress->to_list = STRALLOC( ch->pcdata->board->names );
			sprintf( buf, "Assumed default recipient: %s\r\n", ch->pcdata->board->names );
			write_to_buffer( d, buf, 0 );
		}
		else
			ch->pcdata->in_progress->to_list = STRALLOC( buf );

		break;

	case DEF_INCLUDE:    /* forced default */
		if( !is_full_name( ch->pcdata->board->names, buf ) )
		{
			strcat( buf, " " );
			strcat( buf, ch->pcdata->board->names );
			ch->pcdata->in_progress->to_list = STRALLOC( buf );

			sprintf( buf, "\n\rYou did not specify %s as recipient, so it was automatically added.\r\n"
				"New To :  %s\r\n",
				ch->pcdata->board->names, ch->pcdata->in_progress->to_list );
			write_to_buffer( d, buf, 0 );
		}
		else
			ch->pcdata->in_progress->to_list = STRALLOC( buf );
		break;

	case DEF_EXCLUDE:    /* forced exclude */
		if( !buf[0] )
		{
			write_to_buffer( d, "You must specify a recipient.\n\rTo:      ", 0 );
			return;
		}

		if( is_full_name( ch->pcdata->board->names, buf ) )
		{
			sprintf( buf, "You are not allowed to send notes to %s on this board. Try again.\r\n"
				"To:      ", ch->pcdata->board->names );
			write_to_buffer( d, buf, 0 );
			return; /* return from nanny, not changing to the next state! */
		}
		else
			ch->pcdata->in_progress->to_list = STRALLOC( buf );
		break;

	}

	write_to_buffer( d, "\n\rSubject: ", 0 );
	d->connected = CON_NOTE_SUBJECT;
}

void handle_con_note_subject( DESCRIPTOR_DATA *d, const char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch = d->character;

	if( !ch->pcdata->in_progress )
	{
		d->connected = CON_PLAYING;
		bug( "nanny: In CON_NOTE_SUBJECT, but no note in progress", 0 );
		return;
	}

	mudstrlcpy( buf, argument, MSL );
	smash_tilde( buf );  /* change ~ to - as we save this field as a string later */

	/*
	 * Do not allow empty subjects
	 */

	if( !buf[0] )
	{
		write_to_buffer( d, "Please find a meaningful subject!\r\n", 0 );
		write_to_buffer( d, "Subject: ", 0 );
	}
	else if( strlen( buf ) > 60 )
	{
		write_to_buffer( d, "No, no. This is just the Subject. You're note writing the note yet. Twit.\r\n", 0 );
	}
	else
		/*
		 * advance to next stage
		 */
	{
		ch->pcdata->in_progress->subject = STRALLOC( buf );
		if( IS_IMMORTAL( ch ) )   /* immortals get to choose number of expire days */
		{
			sprintf( buf, "\n\rHow many days do you want this note to expire in?\r\n"
				"Press Enter for default value for this board, %d days.\r\n"
				"Expire:  ", ch->pcdata->board->purge_days );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_NOTE_EXPIRE;
		}
		else
		{
			ch->pcdata->in_progress->expire = current_time + ch->pcdata->board->purge_days * 24L * 3600L;
			sprintf( buf, "This note will expire %s\r", ctime( &ch->pcdata->in_progress->expire ) );
			write_to_buffer( d, buf, 0 );
			write_to_buffer( d,
				"\n\rEnter text. Type ~ or END on an empty line to end note.\r\n"
				"=======================================================\r\n", 0 );
			d->connected = CON_NOTE_TEXT;
		}
	}
}

void handle_con_note_expire( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	time_t expire;
	int days;

	if( !ch->pcdata->in_progress )
	{
		d->connected = CON_PLAYING;
		bug( "nanny: In CON_NOTE_EXPIRE, but no note in progress", 0 );
		return;
	}

	/*
	 * Numeric argument. no tilde smashing
	 */
	mudstrlcpy( buf, argument, MSL );
	if( !buf[0] )    /* assume default expire */
		days = ch->pcdata->board->purge_days;
	else /* use this expire */ if( !is_number( buf ) )
	{
		write_to_buffer( d, "Write the number of days!\r\n", 0 );
		write_to_buffer( d, "Expire:  ", 0 );
		return;
	}
	else
	{
		days = atoi( buf );
		if( days <= 0 )
		{
			write_to_buffer( d, "This is a positive MUD. Use positive numbers only! :)\r\n", 0 );
			write_to_buffer( d, "Expire:  ", 0 );
			return;
		}
	}

	expire = current_time + ( days * 24L * 3600L );  /* 24 hours, 3600 seconds */

	ch->pcdata->in_progress->expire = expire;

	/*
	 * note that ctime returns XXX\n so we only need to add an \r
	 */

	write_to_buffer( d,
		"\n\rEnter text. Type ~ or END on an empty line to end note.\r\n"
		"=======================================================\r\n", 0 );

	d->connected = CON_NOTE_TEXT;
}



void handle_con_note_text( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	char letter[4 * MAX_STRING_LENGTH];

	if( !ch->pcdata->in_progress )
	{
		d->connected = CON_PLAYING;
		bug( "nanny: In CON_NOTE_TEXT, but no note in progress", 0 );
		return;
	}

	/*
	 * First, check for EndOfNote marker
	 */

	mudstrlcpy( buf, argument, MSL );
	if( ( !str_cmp( buf, "~" ) ) || ( !str_cmp( buf, "END" ) ) )
	{
		write_to_buffer( d, "\r\n\r\n", 0 );
		//                send_to_desc_color( szFinishPrompt, 0 );
		write_to_buffer( d, szFinishPrompt, 0 );
		write_to_buffer( d, "\r\n", 0 );
		d->connected = CON_NOTE_FINISH;
		return;
	}

	smash_tilde( buf );  /* smash it now */

	/*
	 * Check for too long lines. Do not allow lines longer than 80 chars
	 */

	if( strlen( buf ) > MAX_STRING_LENGTH )
	{
		write_to_buffer( d, "Line too long!\r\n", 0 );
		return;
	}

	/*
	 * Not end of note. Copy current text into temp buffer, add new line, and copy back
	 */

	 /*
	  * How would the system react to strcpy( , NULL) ?
	  */
	if( ch->pcdata->in_progress->text )
	{
		mudstrlcpy( letter, ch->pcdata->in_progress->text, MSL );
		STRFREE( ch->pcdata->in_progress->text );
		/*		ch->pcdata->in_progress->text = NULL; *//*
		 * be sure we don't free it twice
		 */
	}
	else
		mudstrlcpy( letter, "", MSL );

	/*
	 * Check for overflow
	 */

	if( ( strlen( letter ) + strlen( buf ) ) > MAX_NOTE_TEXT )
	{    /* Note too long, take appropriate steps */
		write_to_buffer( d, "Note too long!\r\n", 0 );
		free_note( ch->pcdata->in_progress );
		ch->pcdata->in_progress = NULL;   /* important */
		d->connected = CON_PLAYING;
		return;
	}

	/*
	 * Add new line to the buffer
	 */

	strcat( letter, buf );
	strcat( letter, "\r\n" );    /* new line. \r first to make note files better readable */

	/*
	 * allocate dynamically
	 */
	ch->pcdata->in_progress->text = STRALLOC( letter );
}

void handle_con_note_finish( DESCRIPTOR_DATA *d, const char *argument )
{

	CHAR_DATA *ch = d->character;

	if( !ch->pcdata->in_progress )
	{
		d->connected = CON_PLAYING;
		bug( "nanny: In CON_NOTE_FINISH, but no note in progress", 0 );
		return;
	}

	switch( tolower( argument[0] ) )
	{
	case 'c':    /* keep writing */
		write_to_buffer( d, "Continuing note...\r\n", 0 );
		d->connected = CON_NOTE_TEXT;
		break;

	case 'v':    /* view note so far */
		if( ch->pcdata->in_progress->text )
		{
			write_to_buffer( d, "Text of your note so far:\r\n", 0 );
			write_to_buffer( d, ch->pcdata->in_progress->text, 0 );
		}
		else
			write_to_buffer( d, "You haven't written a thing!\r\n\r\n", 0 );
		send_to_char( szFinishPrompt, ch );
		write_to_buffer( d, "\r\n", 0 );
		break;

	case 'p':    /* post note */
		finish_note( ch->pcdata->board, ch->pcdata->in_progress );
		send_to_desc_color( "&GNote posted.\r\n", d );
		d->connected = CON_PLAYING;
		xREMOVE_BIT( ch->act, PLR_AFK );
		/*
		 * remove AFK status
		 */
		ch->pcdata->in_progress = NULL;
		act( AT_GREEN, "$n finishes $s note.", ch, NULL, NULL, TO_ROOM );
		break;

	case 'f':
		write_to_buffer( d, "Note cancelled!\r\n", 0 );
		free_note( ch->pcdata->in_progress );
		ch->pcdata->in_progress = NULL;
		d->connected = CON_PLAYING;
		xREMOVE_BIT( ch->act, PLR_AFK );
		/*
		 * remove afk status
		 */
		break;

	default: /* invalid response */
		write_to_buffer( d, "Huh? Valid answers are:\r\n\r\n", 0 );
		//              write_to_buffer (d, szFinishPrompt, 0);
		send_to_char( szFinishPrompt, ch );
		write_to_buffer( d, "\r\n", 0 );

	}
}

AUCTION_DATA *auction_list;
#define MINIMUM_BID	100
#define AUCTION_LENGTH	  5

AUCTION_DATA *auction_free;

AUCTION_DATA *new_auction( void )
{
	AUCTION_DATA *auc;

	if( auction_free == NULL )
		CREATE( auc, AUCTION_DATA, sizeof( *auc ) );
	else
	{
		auc = auction_free;
		auction_free = auction_free->next;
	}
	VALIDATE( auc );
	return auc;
}

void free_auction( AUCTION_DATA *auc )
{
	if( !IS_VALID( auc ) )
		return;

	auc->current_bid = 0;
	auc->gold_held = 0;
	auc->high_bidder = NULL;
	auc->item = NULL;
	auc->min_bid = 0;
	auc->owner = NULL;
	auc->status = 0;
	INVALIDATE( auc );

	auc->next = auction_free;
	auction_free = auc;
}


CMDF( do_auction )
{
	AUCTION_DATA *auc;
	AUCTION_DATA *p;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *cdi;
	long minbid = 0;
	int count;
	char arg1[MAX_INPUT_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	bool ch_comlink;
	argument = one_argument( argument, arg1 );

	if( ch == NULL || IS_NPC( ch ) )
		return;

	/*	if( !has_rate( ch, RATE_AUCTION ) )
		{
			send_to_char( "You don't rate "auction!" HELP RATE for more info.\r\n", ch );
			return;
		}*/


	for( cdi = ch->last_carrying; cdi; cdi = cdi->prev_content )
	{
		if( cdi->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI to do that!\r\n", ch );
		return;
	}

	if( ch->in_room == get_room_index( 6 ) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		return;
	}


	/*
	 * Requesting info on the item
	 */
	if( arg1[0] == '\0' || !str_cmp( arg1, "info" ) )
	{
		if( auction_list == NULL )
		{
			send_to_char( "\n\rThere's nothing up for auction right now.\r\n", ch );
			send_to_char( "&BSyntax&b: &BAuction &b<&Bitem&b> <&Bstarting bid&b>\r\n", ch );
			send_to_char( "        &BAuction &b<&Blist&b/&Binfo&b>\r\n" "        &BAuction bid &b<&Bamount&b>\r\n", ch );
			return;
		}

		obj = auction_list->item;

		if( !obj )
		{
			send_to_char( "There's nothing up for auction right now.\r\n", ch );
			return;
		}

		if( ch == auction_list->owner && !IS_IMMORTAL( ch ) )
		{
			sprintf( buf1, "You're auctioning %s&w.\r\n", obj->short_descr );
			send_to_char( buf1, ch );
			return;
		}

		do_auction( ch, "list" );
		return;
	}    /* Aborting the auction */
	else if( !str_cmp( arg1, "stop" ) && IS_IMMORTAL( ch ) )
	{
		if( auction_list == NULL )
		{
			send_to_char( "There's nothing up for auction right now.\r\n", ch );
			return;
		}

		talk_auction( "An Immortal has stopped the auction!\r\n" );

		if( auction_list->high_bidder != NULL )
			auction_list->high_bidder->gold += auction_list->gold_held;

		if( auction_list->item != NULL )
		{
			obj_to_char( auction_list->item, ch );
			sprintf( buf1, "%s&w appears in your hands.\r\n", auction_list->item->short_descr );
			send_to_char( buf1, ch );
		}

		reset_auc( auction_list );
		return;
	}    /* Listing the items for sale */
	else if( !str_cmp( arg1, "list" ) )
	{
		if( auction_list == NULL )
		{
			send_to_char( "There is nothing up for auction right now!\r\n", ch );
			return;
		}
		/*
				if(auction_list->next == NULL)
				{
					do_auction(ch,"info");
					return;
				}
		*/
		for( count = 1, auc = auction_list; auc; auc = auc->next, count++ )
		{
			if( auc->item != NULL && auc->item->short_descr != NULL )
			{
				ch_printf( ch, "\r\n&B  %d&b.&w %-15s   &YCurrent Bid&O: &P%d &Oby &P%s&O.\r\n",
					count, auc->item->short_descr, auc->current_bid,
					auc->high_bidder ? auc->high_bidder->name : "No One" );
				send_to_char( "&G===========================================\r\n", ch );
				ch_printf( ch, "&YType&O:&P %-12s   &YWorn on&O:&P %s\r\n", aoran( item_type_name( auc->item ) ),
					flag_string( auc->item->wear_flags - 1, w_flags ) );
				ch_printf( ch, "&YWeight&O:&P %-5d        &YValue&O:&P %d\r\n", auc->item->weight, auc->item->cost );

				if( auc->item->item_type == ITEM_WEAPON )
				{
					ch_printf( ch, "&YAverage Dam&O:&P %-8d", auc->item->value[2] );
					if( auc->item->value[3] == WEAPON_BLASTER
						|| auc->item->value[3] == WEAPON_BOWCASTER )
						ch_printf( ch, "&YCharges&O: &P%d&O/&P%d", auc->item->value[4], auc->item->value[5] );
					send_to_char( "\r\n", ch );
				}

				if( auc->item->item_type == ITEM_ARMOR )
				{
					ch_printf( ch, "&YArmor Conditioner&O: &P%d&O/&P%d\r\n", auc->item->value[0], auc->item->value[1] );
				}

				for( paf = auc->item->pIndexData->first_affect; paf; paf = paf->next )
					showaffect( ch, paf );

				for( paf = auc->item->first_affect; paf; paf = paf->next )
					showaffect( ch, paf );
				if( ( auc->item->item_type == ITEM_CONTAINER ) && ( auc->item->first_content ) )
				{
					set_char_color( AT_OBJECT, ch );
					send_to_char( "&YContents&O:&P\r\n", ch );
					show_list_to_char( auc->item->first_content, ch, true, false );
				}


				ch_printf( ch, "&YAuctioned by&O: &P%s.\r\n", auc->owner->name );
				ch_printf( ch, "&YWith a Min Bid&O: &P%ld\r\n", auc->min_bid );
				//      send_to_char(buf1,ch);
			}
		}
		return;
	}    /* Bidding on an item */
	else if( !str_cmp( arg1, "bid" ) )
	{
		long bid = 0;

		if( auction_list == NULL )
		{
			send_to_char( "There's nothing up for auction right now.\r\n", ch );
			return;
		}

		obj = auction_list->item;

		if( !obj )
		{
			send_to_char( "There's nothing up for auction right now.\r\n", ch );
			return;
		}

		if( auction_list->owner == ch )
		{
			send_to_char( "Oh seriously, bidding on your own items? &RLoser&w.\r\n", ch );
			return;
		}

		if( argument[0] == '\0' )
		{
			send_to_char( "How much would you like to bid?\r\n", ch );
			return;
		}

		bid = atol( argument );

		if( ch->gold < bid )
		{
			send_to_char( "You can't cover that bid.\r\n", ch );
			return;
		}

		if( bid < auction_list->min_bid )
		{
			sprintf( buf1, "The minimum bid is &Y%ld&w dollars.\r\n", auction_list->min_bid );
			send_to_char( buf1, ch );
			return;
		}

		if( bid <= auction_list->current_bid )
		{
			if( bid < ( auction_list->current_bid + 10 ) )
			{
				send_to_char( "You have to outbid the current amount by atleast &Y10&w dollars.", ch );
				return;
			}

			sprintf( buf1, "You must bid above the current bid of &Y%ld dollars&w.\r\n", auction_list->current_bid );
			return;
		}

		sprintf( buf1, "&Y%ld&C dollars has been offered for %s&C.\r\n", bid, auction_list->item->short_descr );
		talk_auction( buf1 );

		if( auction_list->high_bidder != NULL )
			auction_list->high_bidder->gold += auction_list->gold_held;

		ch->gold -= bid;

		auction_list->gold_held = bid;
		auction_list->high_bidder = ch;
		auction_list->current_bid = bid;
		auction_list->status = 0;
		return;
	}
	/*
	 * Putting up an item for sale
	 */

	if( ms_find_obj( ch ) )
		return;

	obj = get_obj_carry( ch, arg1 ); /* does char have the item ? */

	if( obj == NULL )
	{
		send_to_char( "You aren't carrying that.\r\n", ch );
		return;
	}

	if( ( obj->item_type == ITEM_CORPSE_NPC )
		|| ( obj->item_type == ITEM_DROID_CORPSE ) || ( obj->item_type == ITEM_CORPSE_PC ) )
	{
		send_to_char( "You can't auction that.\r\n", ch );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
	{
		send_to_char( "You can't auction off such a priceless object!\r\n", ch );
		return;
	}

	if( get_timer( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL( ch ) )
	{
		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			set_char_color( AT_RED, ch );
			send_to_char( "Sorry. Cool down first.\r\n", ch );
			return;
		}
	}


	if( count_auc( ch ) >= 3 )
	{
		send_to_char( "You are only allowed to auction 3 items a time!\r\n", ch );
		return;
	}

	if( argument[0] != '\0' )
		minbid = atol( argument );

	if( minbid > 1000000 )
		minbid = 1000000;

	if( auction_list == NULL )
	{
		auc = new_auction( );
		auction_list = auc;
		auction_list->next = NULL;
	}
	else
	{
		auc = new_auction( );

		for( p = auction_list; p; p = p->next )
		{
			if( p->next == NULL )
				p->next = auc;
			auc->next = NULL;
		}
	}
	separate_obj( obj );
	obj_from_char( obj );

	auc->owner = ch;
	auc->item = obj;
	auc->current_bid = 0;
	auc->status = -1;
	if( minbid > 0 )
		auc->min_bid = minbid;
	else
		auc->min_bid = 0;

	//    obj_from_char( obj );
	return;
}

void auction_update( void )
{
	char buf1[MAX_STRING_LENGTH];
	char temp[MAX_STRING_LENGTH];

	if( auction_list != NULL && auction_list->item != NULL )
	{
		auction_list->status++;

		if( auction_list->current_bid == 0 )
		{
			if( auction_list->status == 0 )
			{
				sprintf( buf1, "Now taking bids on %s%s&C", auction_list->item->short_descr,
					auction_list->min_bid > 0 ? "" : ".\r\n" );
				if( auction_list->min_bid > 0 )
				{
					sprintf( temp, ". &C[Min Bid &Y%ld&C]\r\n", auction_list->min_bid );
					strcat( buf1, temp );
				}

				talk_auction( buf1 );
			}

			if( auction_list->status == AUCTION_LENGTH )
			{
				sprintf( buf1, "No bids on %s&C - item removed.\r\n", auction_list->item->short_descr );
				talk_auction( buf1 );

				obj_to_char( auction_list->item, auction_list->owner );

				sprintf( buf1, "%s&C is returned to you.\r\n", auction_list->item->short_descr );
				send_to_char( buf1, auction_list->owner );

				reset_auc( auction_list );
				return;
			}

			if( auction_list->status == AUCTION_LENGTH - 1 )
			{
				sprintf( buf1, "%s&C - going twice (No Bids Received).\r\n", auction_list->item->short_descr );
				talk_auction( buf1 );
				return;
			}

			if( auction_list->status == AUCTION_LENGTH - 2 )
			{
				sprintf( buf1, "%s&C - going once (No Bids Received).\r\n", auction_list->item->short_descr );
				talk_auction( buf1 );
				return;
			}
		}
		else
		{
			if( auction_list->status == AUCTION_LENGTH )
			{
				sprintf( buf1, "%s&C sold to &P%s&C for &Y%ld &Cdollars.\r\n",
					auction_list->item->short_descr, auction_list->high_bidder->name, auction_list->current_bid );
				talk_auction( buf1 );

				auction_list->owner->gold += ( auction_list->gold_held * 9 ) / 10;

				sprintf( temp, "%ld dollars", ( auction_list->gold_held * 9 ) / 10 );
				sprintf( buf1, "You receive %s.\r\n", auction_list->gold_held > 0 ? temp : "0 dollars" );
				send_to_char( buf1, auction_list->owner );

				obj_to_char( auction_list->item, auction_list->high_bidder );

				sprintf( buf1, "%s&C appears in your hands.\r\n", auction_list->item->short_descr );
				send_to_char( buf1, auction_list->high_bidder );

				reset_auc( auction_list );
				return;
			}

			if( auction_list->status == AUCTION_LENGTH - 1 )
			{
				sprintf( buf1, "%s&C - going twice at &Y%ld&C dollars.\r\n",
					auction_list->item->short_descr, auction_list->current_bid );
				talk_auction( buf1 );
				return;
			}

			if( auction_list->status == AUCTION_LENGTH - 2 )
			{
				sprintf( buf1, "%s&C - going once at &Y%ld&C dollars.\r\n", auction_list->item->short_descr,
					auction_list->current_bid );
				talk_auction( buf1 );
				return;
			}
		}
	}

	return;
}

void reset_auc( AUCTION_DATA *auc )
{
	if( !IS_VALID( auc ) )
		return;

	auc->current_bid = 0;
	auc->gold_held = 0;
	auc->high_bidder = NULL;
	auc->item = NULL;
	auc->min_bid = 0;
	auc->owner = NULL;
	auc->status = 0;

	if( auc == auction_list )
	{
		if( auc->next != NULL )
			auction_list = auc->next;
		else
			auction_list = NULL;

		free_auction( auc );
		return;
	}

	free_auction( auc );
	return;
}

int count_auc( CHAR_DATA *ch )
{
	AUCTION_DATA *q;
	int count;

	for( count = 0, q = auction_list; q; q = q->next )
	{
		if( q->owner == ch )
			count++;
	}

	return count;
}
