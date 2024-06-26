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

#ifndef __BITS_H__
#define __BITS_H__

 /* bits.h -- Abits and Qbits for the Rogue Winds by Scion
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

typedef struct bit_data BIT_DATA;

#define MAX_xBITS 32000

/* abit and qbit struct */
struct bit_data
{
	BIT_DATA *next;
	BIT_DATA *prev;
	int number;
	char desc[MAX_STRING_LENGTH];
};

extern BIT_DATA *first_abit;
extern BIT_DATA *first_qbit;
extern BIT_DATA *last_abit;
extern BIT_DATA *last_qbit;

#endif
