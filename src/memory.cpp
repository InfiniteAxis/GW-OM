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
*                          MUD Memory Cleanup                              *
****************************************************************************/

/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2005 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 * Registered with the United States Copyright Office. TX 5-877-286         *
 *                                                                          *
 * External contributions from Xorith, Quixadhal, Zarius, and many others.  *
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
 *                          Memory Cleanup                                  *
 ****************************************************************************/

#include <stdio.h>

#include "mud.h"





 /* comm.c */
void free_desc( DESCRIPTOR_DATA *d );

/* ban.c */
void free_ban( BAN_DATA *pban );

#ifdef I3
/* i3.c */
void free_i3data( bool complete );
void destroy_I3_mud( I3_MUD *mud );
#endif

void free_teleports( void )
{
	TELEPORT_DATA *tele, *tele_next;

	for( tele = first_teleport; tele; tele = tele_next )
	{
		tele_next = tele->next;

		UNLINK( tele, first_teleport, last_teleport, next, prev );
		DISPOSE( tele );
	}
}

void free_help( HELP_DATA *pHelp )
{
	UNLINK( pHelp, first_help, last_help, next, prev );
	STRFREE( pHelp->text );
	STRFREE( pHelp->keyword );
	DISPOSE( pHelp );
	return;
}

void free_helps( void )
{
	HELP_DATA *pHelp, *pHelp_next;

	for( pHelp = first_help; pHelp; pHelp = pHelp_next )
	{
		pHelp_next = pHelp->next;
		free_help( pHelp );
	}
	return;
}

void close_all_areas( void )
{
	AREA_DATA *area, *area_next;

	for( area = first_area; area; area = area_next )
	{
		area_next = area->next;
		close_area( area );
	}
	for( area = first_build; area; area = area_next )
	{
		area_next = area->next;
		close_area( area );
	}
	return;
}

void free_socials( void )
{
	SOCIALTYPE *social, *social_next;
	int hash;

	for( hash = 0; hash < 27; hash++ )
	{
		for( social = social_index[hash]; social; social = social_next )
		{
			social_next = social->next;
			free_social( social );
		}
	}
	return;
}

void free_commands( void )
{
	CMDTYPE *command, *cmd_next;
	int hash;

	for( hash = 0; hash < 126; hash++ )
	{
		for( command = command_hash[hash]; command; command = cmd_next )
		{
			cmd_next = command->next;
			command->next = NULL;
			command->do_fun = NULL;
			free_command( command );
		}
	}
	return;
}

void free_one_clan( CLAN_DATA *clan )
{
	UNLINK( clan, first_clan, last_clan, next, prev );
	DISPOSE( clan->filename );
	STRFREE( clan->name );
	STRFREE( clan->description );
	STRFREE( clan->leader );
	STRFREE( clan->number1 );
	STRFREE( clan->number2 );
	DISPOSE( clan );
	return;
}

void free_clans( void )
{
	CLAN_DATA *clan, *clan_next;

	for( clan = first_clan; clan; clan = clan_next )
	{
		clan_next = clan->next;
		free_one_clan( clan );
	}
	return;
}

/*
 * Clean all memory on exit to help find leaks
 * Yeah I know, one big ugly function -Druid
 * Added to AFKMud by Samson on 5-8-03.
 */
void cleanup_memory( void )
{
	int hash;
#ifdef OLD_CRYPT
	char *cryptstr;
#endif
	CHAR_DATA *character;
	OBJ_DATA *object;
	DESCRIPTOR_DATA *desc, *desc_next;

#ifdef IMC
	fprintf( stdout, "%s", "IMC2 Data.\n" );
	free_imcdata( true );
	imc_delete_info( );
#endif
#ifdef I3
	fprintf( stdout, "%s", "I3 Data.\n" );
	free_i3data( true );
	destroy_I3_mud( this_i3mud );
#endif

	/*
	 * Commands
	 */
	fprintf( stdout, "%s", "Commands.\n" );
	free_commands( );

#ifdef MULTIPORT
	/*
	 * Shell Commands
	 */
	fprintf( stdout, "%s", "Shell Commands.\n" );
	free_shellcommands( );
#endif

	/*
	 * Clans
	 */
	fprintf( stdout, "%s", "Clans.\n" );
	free_clans( );

	/*
	 * socials
	 */
	fprintf( stdout, "%s", "Socials.\n" );
	free_socials( );

	/*
	 * Helps
	 */
	fprintf( stdout, "%s", "Helps.\n" );
	free_helps( );

	/*
	 * Whack supermob
	 */
	fprintf( stdout, "%s", "Whacking supermob.\n" );
	if( supermob )
	{
		char_from_room( supermob );
		UNLINK( supermob, first_char, last_char, next, prev );
		free_char( supermob );
	}

	/*
	 * Free Objects
	 */
	clean_obj_queue( );
	fprintf( stdout, "%s", "Objects.\n" );
	while( ( object = last_object ) != NULL )
		extract_obj( object );
	clean_obj_queue( );

	/*
	 * Free Characters
	 */
	clean_char_queue( );
	fprintf( stdout, "%s", "Characters.\n" );
	while( ( character = last_char ) != NULL )
		extract_char( character, true );
	clean_char_queue( );

	/*
	 * Descriptors
	 */
	fprintf( stdout, "%s", "Descriptors.\n" );
	for( desc = first_descriptor; desc; desc = desc_next )
	{
		desc_next = desc->next;
		UNLINK( desc, first_descriptor, last_descriptor, next, prev );
		free_desc( desc );
	}

	/*
	 * Teleport lists
	 */
	fprintf( stdout, "%s", "Teleport Data.\n" );
	free_teleports( );

	/*
	 * Areas - this includes killing off the hash tables and such
	 */
	fprintf( stdout, "%s", "Area Data Tables.\n" );
	close_all_areas( );

	/*
	 * Get rid of auction pointer  MUST BE AFTER OBJECTS DESTROYED
	 */
	fprintf( stdout, "%s", "Auction.\n" );
	DISPOSE( auction );

	/*
	 * System Data
	 */
	fprintf( stdout, "%s", "System data.\n" );
	if( sysdata.time_of_max )
		DISPOSE( sysdata.time_of_max );
	if( sysdata.guild_overseer )
		DISPOSE( sysdata.guild_overseer );
	if( sysdata.guild_advisor )
		DISPOSE( sysdata.guild_advisor );

#ifdef OLD_CRYPT
	fprintf( stdout, "%s", "Disposing of crypt.\n" );
	cryptstr = crypt( "cleaning", "$1$Cleanup" );
	free( cryptstr );
#endif

	fprintf( stdout, "%s", "Checking string hash for leftovers.\n" );
	{
		for( hash = 0; hash < 1024; hash++ )
			hash_dump( hash );
	}

	fprintf( stdout, "%s", "Cleanup complete, exiting.\n" );
	return;
}  /* cleanup memory */
