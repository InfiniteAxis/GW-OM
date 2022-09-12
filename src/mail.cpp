/***************************************************************************
*                        STAR WARS REALITY 1.0 FUSS                        *
* ------------------------------------------------------------------------ *
* Star Wars:                                                               *
*      ____.          .___.__                  _________.__  __  .__       *
*     |    | ____   __| _/|__| ___  ________  /   _____/|__|/  |_|  |__    *
*     |    |/ __ \ / __ | |  | \  \/ /  ___/  \_____  \ |  \   __\  |  \   *
* /\__|    \  ___// /_/ | |  |  \   /\___ \   /        \|  ||  | |   Y  \  *
* \________|\___  >____ | |__|   \_//____  > /_______  /|__||__| |___|  /  *
*               \/     \/                \/          \/               \/   *
*                                               ____      ________         *
*                 .--.                 ___  __ /_   |     \_____  \        *
*       ::\`--._,'.::.`._.--'/::::     \  \/ /  |   |       _<__  <        *
*       ::::.  ` __::__ '  .::::::      \   /   |   |      /       \       *
*       ::::::-:.`'..`'.:-::::::::       \_/    |___|  /\ /______  /       *
*       ::::::::\ `--' /::::::::::                     \/        \/        *
*                                                                          *
* ------------------------------------------------------------------------ *
* Star Wars: Jedi vs Sith was created by Diablo (Michael Francis) in 2005  *
* ------------------------------------------------------------------------ *
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
* ------------------------------------------------------------------------ *
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
* ------------------------------------------------------------------------ *
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
*                    Player-to-Player Mail Module                          *
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mud.h"

/* Locally defined functions */
void save_mail( CHAR_DATA *ch, char *filename );
void load_mail( CHAR_DATA *ch );
void mail_count( CHAR_DATA *ch );

/* Functions defined in other files */

void save_mail( CHAR_DATA *ch, char *filename )
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];

	if( !ch->mailbuf )
	{
		bug( "Error: %s writing mail without mailbuf\n", ch->name, 1 );
		return;
	}

	sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( filename[0] ), capitalize( filename ) );

	if( ( fp = FileOpen( buf, "a" ) ) == NULL )
	{
		bug( "Error: cant write to fie %s\n", buf, 1 );
		return;
	}

	fprintf( fp, "To: %s\n", filename );
	fprintf( fp, "Sender: %s\n\n", ch->name );
	fprintf( fp, "%s", ch->mailbuf );
	fprintf( fp, "#END\n" );
	FileClose( fp );

	STRFREE( ch->mailbuf );
	ch->mailbuf = STRALLOC( "" );
	return;
}

void load_mail( CHAR_DATA *ch )
{
	FILE *fp, *wfp, *wwfp;
	EXTRA_DESCR_DATA *ed;
	OBJ_DATA *paper;
	char buf[MAX_STRING_LENGTH], notebuf[MAX_STRING_LENGTH];
	bool deleting = false;

	sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	if( ( fp = FileOpen( buf, "r" ) ) == NULL )
	{
		bug( "Load_mail: Null mailfile.", 1 );
		return;
	}

	sprintf( buf, "%s%c/%s.dlt", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	if( ( wfp = FileOpen( buf, "w" ) ) == NULL )
	{
		bug( "Delete_mail: Null mailfile.", 1 );
		return;
	}

	sprintf( buf, " " );
	sprintf( notebuf, "---------- Gundam Mail ----------\n" );

	while( fgets( buf, sizeof( buf ), fp ) != NULL )
	{
		if( strstr( buf, "#END" ) && !deleting )
		{
			deleting = true;
			continue;
		}

		if( !deleting )
		{
			strcat( notebuf, buf );
		}
		else
		{
			fprintf( wfp, "%s", buf );
		}
	}

	paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
	ed = SetOExtra( paper, "mail" );
	STRFREE( ed->description );
	ed->description = STRALLOC( notebuf );
	paper->value[0] = 2;
	paper->value[1] = 2;
	paper->value[2] = 2;
	STRFREE( paper->short_descr );
	paper->short_descr = STRALLOC( "a small mail disk" );
	STRFREE( paper->description );
	paper->description = STRALLOC( "A small mail disk lies on the ground." );
	STRFREE( paper->name );
	paper->name = STRALLOC( "disk mail" );
	obj_to_char( paper, ch );

	sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
	sprintf( notebuf, "%s%c/%s.dlt", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	rename( notebuf, buf );

	FileClose( fp );
	FileClose( wfp );

	sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	wwfp = FileOpen( buf, "r" );

	if( fgetc( wwfp ) == EOF )
		remove( buf );

	FileClose( wwfp );
	return;
}

void mail_count( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	struct stat fst;

	sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	if( stat( buf, &fst ) != -1 )
		ch_printf( ch, "&GYou've got mail!\n" );

	return;
}

CMDF( do_mail )
{
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	struct stat fst;

	switch( ch->substate )
	{
	default:
		break;
	case SUB_WRITE_MAIL:
		mudstrlcpy( name, ( char * ) ch->dest_buf, MSL );
		if( name[0] == '\0' )
		{
			bug( "mail: write_to: NULL ch->dest_buf", 0 );
			return;
		}

		if( ch->mailbuf )
			STRFREE( ch->mailbuf );
		ch->mailbuf = copy_buffer( ch );
		save_mail( ch, name );
		stop_editing( ch );
		if( ch->dest_buf )
			DISPOSE( ch->dest_buf );
		ch->substate = ch->tempnum;
		if( ch->mailbuf )
			STRFREE( ch->mailbuf );
		ch->mailbuf = STRALLOC( "" );
		return;
	}

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "&CUsage&B: &Wmail &B(&Creceive&B|&Csend&B) &B[&Wplayers name&B]&D\r\n", ch );
		return;
	}

	if( argument[0] == '\0' && !nifty_is_name_prefix( arg, "receive" ) )
	{
		send_to_char( "&CUsage&B: &Wmail &B(&Creceive&B|&Csend&B) &B[&Wplayers name&B]&D\r\n", ch );
		return;
	}

	if( nifty_is_name_prefix( arg, "receive" ) )
	{
		sprintf( buf, "%s%c/%s", MAIL_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

		if( stat( buf, &fst ) == -1 )
		{
			send_to_char( "&RYou have no mail waiting.\r\n", ch );
			return;
		}

		load_mail( ch );
		send_to_char( "&GA small mail droid delivers your mail to you.\r\n", ch );
		return;
	}

	if( nifty_is_name_prefix( arg, "send" ) )
	{
		if( strchr( argument, '/' ) )
		{
			send_to_char( "&RThat player does not exist.\r\n", ch );
			return;
		}

		sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( argument[0] ), capitalize( argument ) );

		if( stat( buf, &fst ) == -1 )
		{
			send_to_char( "&RThat player does not exist.\r\n", ch );
			return;
		}

		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;

		ch->substate = SUB_WRITE_MAIL;
		ch->dest_buf = str_dup( argument );
		start_editing( ch, ch->mailbuf );
		return;
	}

	send_to_char( "&CUsage&B: &Wmail &B(&Creceive&B|&Csend&B) &B[&Wplayers name&B]&D\r\n", ch );
	return;
}
