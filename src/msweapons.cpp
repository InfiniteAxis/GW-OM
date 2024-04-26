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


/*
Weapon List

00 - 100mm Machinegun
01 - 105mm Rifle
02 - Leo Bazooka
03 - Beamsabre
04 - Shoulder Mounted Energy Cannons
05 - Side Mounted Missile Launchers
06 - Beam Rifle
07 - Laser Cannon
08 - Twin Gattling-Gun
09 - Beam Cannon
10 - Buster Rifle
11 - Head Vulcan
12 - Beam Scythe
13 - Buster Shield
14 - Beam Gattling
15 - MultiBlast
16 - Army Knife
17 - Shatols
18 - Missiles
19 - Cross Crusher
20 - Big Beam Sabre
21 - Heat Rod
22 - Beam Glaive
23 - Dragon Flame
24 - Flamethrower
25 - Dober Gun
26 - Short Blast
27 - Long Blast
28 - Small Beam Cannon
29 - Beam Blade
30 - Planet Defensers
31 - Libra Main Cannon
39 - 250mm Rifle


*/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

bool autofly( SHIP_DATA *ship );

CMDF( do_i100machine )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 2 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );


	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "100mm Machine Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the 100mm Machine Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "100mm Machine Gun fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by 100mm Machine Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your 100mm Machine Gun fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 8, 15, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 8, 15, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 8, 15, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 8, 15, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 8, 15, ch );
		else
			damage_ship_ch( target, 8, 15, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 2;
		return;
	}

}

CMDF( do_i105rifle )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 3 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 800 ||
			abs( target->vy - ship->vy ) > 800 ||
			abs( target->vz - ship->vz ) > 800 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "105mm Rifle fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the 105mm Rifle  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "105mm Rifle fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by 105mm Rifle fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your 105mm Rifle fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 8, 17, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 8, 17, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 8, 17, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 8, 17, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 8, 17, ch );
		else
			damage_ship_ch( target, 1, 1, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 3;
		return;
	}

}

CMDF( do_leobazooka )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 10 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_missileweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 450 ||
			abs( target->vy - ship->vy ) > 450 ||
			abs( target->vz - ship->vz ) > 450 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Bazooka fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Bazooka  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_missileweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Bazooka fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Bazooka fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Bazooka fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_missileweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 10, 15, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 10, 15, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 10, 15, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 10, 15, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 10, 15, ch );
		else
			damage_ship_ch( target, 10, 15, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 10;
		return;
	}

}

CMDF( do_shouldercannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 1 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorhead < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_missileweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 650 ||
			abs( target->vy - ship->vy ) > 650 ||
			abs( target->vz - ship->vz ) > 650 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Laser fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Lasers  miss %s.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_missileweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Laser fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Laser fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Laser fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_missileweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 10, 13, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 10, 13, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 10, 13, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 10, 13, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 10, 13, ch );
		else
			damage_ship_ch( target, 10, 13, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 1;
		return;
	}

}

CMDF( do_sidemissilelauncher )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 5 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlegs < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_missileweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s launches misslies at you but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You launch missiles at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_missileweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Missiles launched from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Missiles launched from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You launch missiles and they hit %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_missileweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 12, 14, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 11, 14, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 11, 14, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 11, 14, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 11, 14, ch );
		else
			damage_ship_ch( target, 12, 14, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 5;
		return;
	}

}

CMDF( do_beamrifle )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 20 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Beam Rifle fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Beam Rifle  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Beam Rifle fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Beam Rifle fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Beam Rifle fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 14, 19, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 14, 19, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 14, 19, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 14, 19, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 14, 19, ch );
		else
			damage_ship_ch( target, 14, 19, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 20;
		return;
	}

}

CMDF( do_lasercannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 10 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 800 ||
			abs( target->vy - ship->vy ) > 800 ||
			abs( target->vz - ship->vz ) > 800 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Laser Cannon fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Laser Cannon  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Laser Cannon fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Laser Cannon fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Laser Cannon fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 14, 16, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 14, 16, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 14, 16, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 14, 16, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 14, 16, ch );
		else
			damage_ship_ch( target, 14, 16, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 10;
		return;
	}

}

CMDF( do_twingattling )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 15 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 500 ||
			abs( target->vy - ship->vy ) > 500 ||
			abs( target->vz - ship->vz ) > 500 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Twin Gattling-Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Twin Gattling-Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Twin Gattling-Gun fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Twin Gattling-Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Twin Gattling-Gun fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 14, 17, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 14, 17, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 14, 17, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 14, 17, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 14, 17, ch );
		else
			damage_ship_ch( target, 14, 17, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 15;
		return;
	}

}

CMDF( do_beamcannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 20 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 750 ||
			abs( target->vy - ship->vy ) > 750 ||
			abs( target->vz - ship->vz ) > 750 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Beam Cannon fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Beam Cannon  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Beam Cannon fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Beam Cannon fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Beam Cannon fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 17, 19, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 17, 19, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 17, 19, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 17, 19, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 17, 19, ch );
		else
			damage_ship_ch( target, 17, 19, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 20;
		return;
	}

}


CMDF( do_i250machine )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 5 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "250mm Machine Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the 250mm Machine Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "250mm Machine Gun fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by 250mm Machine Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your 250mm Machine Gun fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 12, 14, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 12, 14, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 12, 14, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 12, 14, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 12, 14, ch );
		else
			damage_ship_ch( target, 12, 14, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 5;
		return;
	}

}
CMDF( do_busterrifle )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 30 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Buster Rifle blast from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Buster Rifle Blast races at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Buster Rifle Blast from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Buster Rifle blast from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Buster Rifle fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 25, 25, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 25, 25, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 25, 25, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 25, 25, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 25, 25, ch );
		else
			damage_ship_ch( target, 25, 25, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 30;
		return;
	}

}

CMDF( do_headvulcan )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 25 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorhead < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Head Vulcan burst from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Head Vulcan shoots bursts at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Head Vulcan Bursts from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, " %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Head Vulcan bursts out and hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 16, 24, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 16, 24, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 16, 24, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 16, 24, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 16, 24, ch );
		else
			damage_ship_ch( target, 16, 24, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 25;
		return;
	}

}


CMDF( do_beamscythe )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 15 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 300 ||
			abs( target->vy - ship->vy ) > 300 ||
			abs( target->vz - ship->vz ) > 300 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Beam Scythe slash from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Beam Scythe Slashes at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Beam Scythe Slash from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suit shakes as you are hit by the Beam Scythe from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Beam Scythe Slashes %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small tremor vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 22, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 22, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 22, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 22, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 22, 23, ch );
		else
			damage_ship_ch( target, 22, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 15;
		return;
	}

}


CMDF( do_bustershield )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 10 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Buster Shield from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Buster Shield flies at %s but misses, and returns to you.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Buster Shield from %s flies at and hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Buster Shield from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Buster Shield hits %s and returns to you!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 18, 19, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 18, 19, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 18, 19, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 18, 19, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 18, 19, ch );
		else
			damage_ship_ch( target, 18, 19, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 10;
		return;
	}

}

CMDF( do_beamgattling )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 25 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 20, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Beam Gattling fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Beam Gattling fires a spread of bullets at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Beam Gattling fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are pummeled by %s's Beam Gattling fire! %s!", ship->name, target->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Beam Gattling fires and pummels %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 19, 24, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 19, 24, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 19, 24, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 19, 24, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 19, 24, ch );
		else
			damage_ship_ch( target, 19, 24, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 25;
		return;
	}

}



CMDF( do_multiblast )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 30 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armor < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_missileweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Multiple Rockets from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Multiple Rocket Blast bursts at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_missileweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Multiple Rockets from %s pulverises %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, " %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Multiple Rockets shoot out and pulverise %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_missileweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 22, 24, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 22, 24, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 22, 24, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 22, 24, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 22, 24, ch );
		else
			damage_ship_ch( target, 22, 24, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 30;
		return;
	}

}


CMDF( do_armyknife )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 10 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 400 ||
			abs( target->vy - ship->vy ) > 400 ||
			abs( target->vz - ship->vz ) > 400 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Army Knife from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Army Knife slashes at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Army Knife slash from %s slashes %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Army Knife from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Army Knife slashes %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 15, 17, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 15, 17, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 15, 17, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 15, 17, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 15, 17, ch );
		else
			damage_ship_ch( target, 15, 17, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 10;
		return;
	}

}




CMDF( do_shatols )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 15 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 300 ||
			abs( target->vy - ship->vy ) > 300 ||
			abs( target->vz - ship->vz ) > 300 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 20, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The slash of the two Shatols from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You slash your Shatols at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Shatol's from %s slashes and strikes %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are struck by the Shatols on %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Shatols slash at %s and strike!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 18, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 18, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 18, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 18, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 18, 23, ch );
		else
			damage_ship_ch( target, 18, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 15;
		return;
	}

}


CMDF( do_missiles )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 35 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armor < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_missileweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 20, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Missile barrage from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your shoulders open up, and shoot out Missiles at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_missileweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Mssile Barrage from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, " %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your shoulders open up, and send out a barrage of missiles, smashing into %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_missileweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 16, 20, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 16, 20, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 16, 20, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 16, 20, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 16, 20, ch );
		else
			damage_ship_ch( target, 16, 20, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->ammo -= 35;
		return;
	}

}

CMDF( do_crosscrusher )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 35 )
	{
		send_to_char( "&ROh No! Not Enough Fuel!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorhead < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 400 ||
			abs( target->vy - ship->vy ) > 400 ||
			abs( target->vz - ship->vz ) > 400 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 20, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s dashes at you but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You dash at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s dashes at %s, capturing it in its Pincers and begins squeezing it.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "%s dashes at you, capturing you in its Pincers as it begins to crush you!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You dash at %s, capturing it in your pincers, as you begin to crush it!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 20, 24, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 20, 24, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 20, 24, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 20, 24, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 20, 24, ch );
		else
			damage_ship_ch( target, 20, 24, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 35;
		return;
	}

}

CMDF( do_bigbeamsabre )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->ammo < 30 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_beamsabers] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s flies at you but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You fly at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_beamsabers );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s flies at %s and slashes hard, hitting the suit with its Big Beam Sabre", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suit shudders as it is hit by %s's Big Beam Sabre!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You fly swiftly, and slash %s with your Big Beam Sabre!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_beamsabers );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 25, 25, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 25, 25, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 25, 25, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 25, 25, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 25, 25, ch );
		else
			damage_ship_ch( target, 25, 25, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 30;
		return;
	}

}


CMDF( do_heatrod )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 35 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}

		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s whips it Heat Rod at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You whip your Heat Rod at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s whips its Heat Rod at %s leaves melted armor where it hit", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suits armor melts slightly as it is struck by %s's Heat Rod!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You whip your Heat Rod at %s, melting the armor that you hit!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 25, 26, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 25, 26, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 25, 26, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 25, 26, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 25, 26, ch );
		else
			damage_ship_ch( target, 25, 26, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 35;
		return;
	}

}


CMDF( do_beamglaive )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 20 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s thrusts its Beam Glaive at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You thrust your Beam Glaive at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s thrusts its beam Glaive at %s, piercing the armor!", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You fly back slightly, as you are hit by %s's Beam Glaive!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You thrust your Beam Glaive at %s, slightly piercing its armor!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 21, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 21, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 21, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 21, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 21, 23, ch );
		else
			damage_ship_ch( target, 21, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 20;
		return;
	}

}

CMDF( do_dragonfang )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 30 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_meleeweapons] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s sends its Dragon Fang flying at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You send your Dragon Fang at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_meleeweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s sends its Dragon Fang at %s as it opens up and bites into the suit with its fangs!", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suit is clamped down, as it is struck by %s's Dragon Fang attack!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You lunge your Dragon Fang at %s, clamping down as it hits!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_meleeweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 18, 20, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 18, 20, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 18, 20, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 18, 20, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 18, 20, ch );
		else
			damage_ship_ch( target, 18, 20, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 30;
		return;
	}

}

CMDF( do_flamethrower )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 35 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s blasts its Flamethrower at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You blast your Flamethrower at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s blasts fire at %s enveloping it in flames", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suit heats up tremendously as its enveloped by %s's Flamethrower!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You blast your Flamethrower at %s, enveloping it in flames!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 20, 20, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 20, 20, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 20, 20, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 20, 20, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 20, 20, ch );
		else
			damage_ship_ch( target, 20, 20, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 35;
		return;
	}

}

CMDF( do_dobergun )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 30 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Dober Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Dober Gun fires a spread of bullets at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Dober Gun fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are pummeled by %s's Dober Gun fire!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Dober Gun fires and pummels %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 25, 25, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 25, 25, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 25, 25, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 25, 25, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 25, 25, ch );
		else
			damage_ship_ch( target, 25, 25, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 30;
		return;
	}

}

CMDF( do_shortblast )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 35 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 400 ||
			abs( target->vy - ship->vy ) > 400 ||
			abs( target->vz - ship->vz ) > 400 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Short Range blast from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Short Range blast shoots at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Short Range blast from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Short Range blast from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You Power up your Beam Cannon, and fire a short, high powered blast at %s and hit!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 22, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 22, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 22, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 22, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 22, 23, ch );
		else
			damage_ship_ch( target, 22, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 35;
		return;
	}

}

CMDF( do_longblast )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 25 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 800 ||
			abs( target->vy - ship->vy ) > 800 ||
			abs( target->vz - ship->vz ) > 800 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Long Range blast from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Long Range Blast races at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Long Range blast from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Long Range blast from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You shoot a thin Long Range blast at %s and hit!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 18, 19, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 18, 19, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 18, 19, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 18, 19, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 18, 19, ch );
		else
			damage_ship_ch( target, 18, 19, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 25;
		return;
	}

}

CMDF( do_smallbeamcannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 30 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Small Beam Cannon fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Small Beam Cannon fires a spread of bullets at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Small Beam Cannon fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are pummeled by %s's Small Beam Cannon fire!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Small Beam Cannon fires and pummels %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 19, 21, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 19, 21, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 19, 21, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 19, 21, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 19, 21, ch );
		else
			damage_ship_ch( target, 19, 21, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 30;
		return;
	}

}

CMDF( do_beamblade )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 25 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_beamsabers] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s slashes its Beam Blade at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You slash your Beam Blade at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_beamsabers );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s slashes its Beam Blade at %s, slashing the armor!", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You shudder, as you are hit by %s's Beam Blade!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You slash your Beam Blade at %s, slightly slashing its armor!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_beamsabers );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 20, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 20, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 20, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 20, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 20, 23, ch );
		else
			damage_ship_ch( target, 20, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 25;
		return;
	}

}

CMDF( do_planetdefenser )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 20 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armor < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s flings its Planet Defensers at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You fling your Planet Defensers at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s flings its Planet Defensers at %s, pelting the armor!", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are pelted back, as you are hit by %s's Planet Defensers!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You fling your Planet Defensers at %s, hitting several times!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 15, 16, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 15, 16, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 15, 16, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 15, 16, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 15, 16, ch );
		else
			damage_ship_ch( target, 15, 16, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 10;
		return;
	}

}

CMDF( do_libramaincannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != CAPITAL_SHIP )
	{
		send_to_char( "&RThis isn't a Cap Ship!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 2000 )
	{
		send_to_char( "&ROh No! Not enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}


	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_spacecombat] / 3
			+ ch->pcdata->learned[gsn_spacecombat2] / 3 + ch->pcdata->learned[gsn_spacecombat3] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 1000 ||
			abs( target->vy - ship->vy ) > 1000 ||
			abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Main Cannon from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Main Cannon fires at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_spacecombat );
			learn_from_failure( ch, gsn_spacecombat2 );
			learn_from_failure( ch, gsn_spacecombat3 );
			snprintf( buf, MAX_STRING_LENGTH, "The Main Cannon fire from %s barely misses %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Main Cannon fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You're sent flying from %s's Main Cannon fire!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Main Cannon sends %s flying!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 100, 100, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 100, 100, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 100, 100, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 100, 100, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 100, 100, ch );
		else
			damage_ship_ch( target, 100, 100, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 2000;
		return;
	}

}

CMDF( do_tbeamscythe )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 25 )
	{
		send_to_char( "&RNot enough energy is left to use this!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 300 ||
			abs( target->vy - ship->vy ) > 300 ||
			abs( target->vz - ship->vz ) > 300 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Twin Beam Scythe slash from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Twin Beam Scythe Slashes at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Twin Beam Scythe Slash from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "Your suit shakes as you are hit by the Twin Beam Scythe from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Twin Beam Scythe Slashes %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small tremor vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 23, 24, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 23, 24, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 23, 24, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 23, 24, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 23, 24, ch );
		else
			damage_ship_ch( target, 23, 24, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 25;
		return;
	}

}

CMDF( do_tbusterrifle )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 50 )
	{
		send_to_char( "&ROh No! Not Enough Ammo!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Twin Buster Rifle blast from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Twin Buster Rifle Blast races at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Twin Buster Rifle Blast from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Twin Buster Rifle blast from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Twin Buster Rifle fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 27, 28, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 27, 28, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 27, 28, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 27, 28, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 27, 28, ch );
		else
			damage_ship_ch( target, 27, 28, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 50;
		return;
	}

}

CMDF( do_megacannon )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 50 )
	{
		send_to_char( "&ROh No! Not enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 700 ||
			abs( target->vy - ship->vy ) > 700 ||
			abs( target->vz - ship->vz ) > 700 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "The Mega Cannon blast from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Your Mega Cannon Blast races at %s but misses.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "The Mega Cannon Blast from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by the Mega Cannon blast from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Mega Cannon blast hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 27, 28, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 27, 28, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 27, 28, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 27, 28, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 27, 28, ch );
		else
			damage_ship_ch( target, 27, 28, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 50;
		return;
	}

}

CMDF( do_tbeamglaive )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that until after you've finished launching!\r\n", ch );
		return;
	}
	if( ship->energy < 25 )
	{
		send_to_char( "&ROh No! Not Enough energy!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		send_to_char( "&RThat weapon is destroyed!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_heavyenergy] / 3 );

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "lasers" ) )
	{

		if( ship->statet0 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships weapons are damaged.\r\n", ch );
			return;
		}
		if( ship->target0 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target0;
		if( ship->target0->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target0 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 600 ||
			abs( target->vy - ship->vy ) > 600 ||
			abs( target->vz - ship->vz ) > 600 )
		{
			send_to_char( "&RThat ship is out of range.\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 50, chance, 250 );
		act( AT_PLAIN, "$n presses the fire button.", ch,
			NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, " %s thrusts its Twin Beam Trident at you, but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You thrust your Twin Beam Trident at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_heavyenergy );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s thrusts its Twin Beam Trident at %s, piercing the armor!", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You thrust back slightly, as you are hit by %s's Twin Beam Trident!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You thrust your Twin Beam Trident at %s, slightly piercing its armor!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_heavyenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 23, 23, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 23, 23, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 23, 23, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 23, 23, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 23, 23, ch );
		else
			damage_ship_ch( target, 23, 23, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}
		ship->energy -= 25;
		return;
	}

}

/* weapon system */
CMDF( do_firstweapon )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}
	if( ship->targettype == 0 )
	{
		send_to_char( "&RChoose a suit part to target first!.\r\n", ch );
		return;
	}

	if( ship->mod == 3 && ship->offon == 1 )
	{
		send_to_char( "&RYou can't fight while transformed!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		ch->mental_state += 1;
		if( ch->mental_state == 85 )
		{
			send_to_char( "&CYou're getting closer to the edge.. Your mind can't handle much more..\r\n", ch );
		}
		if( ch->mental_state == 100 )
		{
			REMOVE_BIT( ch->pcdata->flags, PCFLAG_ZERO );
			snprintf( buf, MAX_STRING_LENGTH, "%s explodes in a blinding flash of light!", ship->name );
			echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
			snprintf( buf, MAX_STRING_LENGTH, "&YT&Oh&Ye &zZ&We&zr&Wo &CS&cy&Cs&ct&Ce&cm &GH&ga&Gs &BJ&bu&Bs&bt &P*&p*&P*&RR&ri&RPP&re&RD&P*&p*&P* &BA&bp&ca&br&Bt &C%s's &YM&Oin&Yd&P!&p!&P!", ch->name );
			info_chan( buf );
			snprintf( log_buf, MAX_STRING_LENGTH, "%s killed by the Zero System.", ch->name );
			log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
			destroy_ship( ship, NULL );

		}

	}

	if( ship->firstweapon == 0 )
	{
		do_i100machine( ch, "" );
	}
	if( ship->firstweapon == 1 )
	{
		do_i105rifle( ch, "" );
	}
	if( ship->firstweapon == 2 )
	{
		do_leobazooka( ch, "" );
	}
	if( ship->firstweapon == 3 )
	{
		do_beamsabre( ch, "" );
	}
	if( ship->firstweapon == 4 )
	{
		do_shouldercannon( ch, "" );
	}
	if( ship->firstweapon == 5 )
	{
		do_sidemissilelauncher( ch, "" );
	}
	if( ship->firstweapon == 6 )
	{
		do_beamrifle( ch, "" );
	}
	if( ship->firstweapon == 7 )
	{
		do_lasercannon( ch, "" );
	}
	if( ship->firstweapon == 8 )
	{
		do_twingattling( ch, "" );
	}
	if( ship->firstweapon == 9 )
	{
		do_beamcannon( ch, "" );
	}
	if( ship->firstweapon == 10 )
	{
		do_busterrifle( ch, "" );
	}
	if( ship->firstweapon == 11 )
	{
		do_headvulcan( ch, "" );
	}
	if( ship->firstweapon == 12 )
	{
		do_beamscythe( ch, "" );
	}
	if( ship->firstweapon == 13 )
	{
		do_bustershield( ch, "" );
	}
	if( ship->firstweapon == 14 )
	{
		do_beamgattling( ch, "" );
	}
	if( ship->firstweapon == 15 )
	{
		do_multiblast( ch, "" );
	}
	if( ship->firstweapon == 16 )
	{
		do_armyknife( ch, "" );
	}
	if( ship->firstweapon == 17 )
	{
		do_shatols( ch, "" );
	}
	if( ship->firstweapon == 18 )
	{
		do_missiles( ch, "" );
	}
	if( ship->firstweapon == 19 )
	{
		do_crosscrusher( ch, "" );
	}
	if( ship->firstweapon == 20 )
	{
		do_bigbeamsabre( ch, "" );
	}
	if( ship->firstweapon == 21 )
	{
		do_heatrod( ch, "" );
	}
	if( ship->firstweapon == 22 )
	{
		do_beamglaive( ch, "" );
	}
	if( ship->firstweapon == 23 )
	{
		do_dragonfang( ch, "" );
	}
	if( ship->firstweapon == 24 )
	{
		do_flamethrower( ch, "" );
	}
	if( ship->firstweapon == 25 )
	{
		do_dobergun( ch, "" );
	}
	if( ship->firstweapon == 26 )
	{
		do_shortblast( ch, "" );
	}
	if( ship->firstweapon == 27 )
	{
		do_longblast( ch, "" );
	}
	if( ship->firstweapon == 28 )
	{
		do_smallbeamcannon( ch, "" );
	}
	if( ship->firstweapon == 29 )
	{
		do_beamblade( ch, "" );
	}
	if( ship->firstweapon == 30 )
	{
		do_planetdefenser( ch, "" );
	}
	if( ship->firstweapon == 31 )
	{
		do_libramaincannon( ch, "" );
	}
	if( ship->firstweapon == 32 )
	{
		do_tbeamscythe( ch, "" );
	}
	if( ship->firstweapon == 33 )
	{
		do_tbusterrifle( ch, "" );
	}
	if( ship->firstweapon == 34 )
	{
		do_megacannon( ch, "" );
	}
	if( ship->firstweapon == 35 )
	{
		do_tbeamglaive( ch, "" );
	}
	if( ship->firstweapon == 39 )
	{
		do_i250machine( ch, "" );
	}
	if( ship->firstweapon == 40 )
	{
		send_to_char( "&RNo weapon equiped\r\n", ch );
	}
	return;
}

CMDF( do_secondweapon )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->targettype == 0 )
	{
		send_to_char( "&RChoose a suit part to target first!.\r\n", ch );
		return;
	}

	if( ship->mod == 3 && ship->offon == 1 )
	{
		send_to_char( "&RYou can't fight while transformed!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		ch->mental_state += 1;
		if( ch->mental_state == 85 )
		{
			send_to_char( "&CYou're getting closer to the edge.. Your mind can't handle much more..\r\n", ch );
		}
		if( ch->mental_state == 100 )
		{
			REMOVE_BIT( ch->pcdata->flags, PCFLAG_ZERO );
			snprintf( buf, MAX_STRING_LENGTH, "%s explodes in a blinding flash of light!", ship->name );
			echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
			snprintf( buf, MAX_STRING_LENGTH, "&YT&Oh&Ye &zZ&We&zr&Wo &CS&cy&Cs&ct&Ce&cm &GH&ga&Gs &BJ&bu&Bs&bt &P*&p*&P*&RR&ri&RPP&re&RD&P*&p*&P* &BA&bp&ca&br&Bt &C%s's &YM&Oin&Yd&P!&p!&P!", ch->name );
			info_chan( buf );
			snprintf( log_buf, MAX_STRING_LENGTH, "%s killed by the Zero System.", ch->name );
			log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
			destroy_ship( ship, NULL );

		}

	}

	if( ship->secondweapon == 0 )
	{
		do_i100machine( ch, "" );
	}
	if( ship->secondweapon == 1 )
	{
		do_i105rifle( ch, "" );
	}
	if( ship->secondweapon == 2 )
	{
		do_leobazooka( ch, "" );
	}
	if( ship->secondweapon == 3 )
	{
		do_beamsabre( ch, "" );
	}
	if( ship->secondweapon == 4 )
	{
		do_shouldercannon( ch, "" );
	}
	if( ship->secondweapon == 5 )
	{
		do_sidemissilelauncher( ch, "" );
	}
	if( ship->secondweapon == 6 )
	{
		do_beamrifle( ch, "" );
	}
	if( ship->secondweapon == 7 )
	{
		do_lasercannon( ch, "" );
	}
	if( ship->secondweapon == 8 )
	{
		do_twingattling( ch, "" );
	}
	if( ship->secondweapon == 9 )
	{
		do_beamcannon( ch, "" );
	}
	if( ship->secondweapon == 10 )
	{
		do_busterrifle( ch, "" );
	}
	if( ship->secondweapon == 11 )
	{
		do_headvulcan( ch, "" );
	}
	if( ship->secondweapon == 12 )
	{
		do_beamscythe( ch, "" );
	}
	if( ship->secondweapon == 13 )
	{
		do_bustershield( ch, "" );
	}
	if( ship->secondweapon == 14 )
	{
		do_beamgattling( ch, "" );
	}
	if( ship->secondweapon == 15 )
	{
		do_multiblast( ch, "" );
	}
	if( ship->secondweapon == 16 )
	{
		do_armyknife( ch, "" );
	}
	if( ship->secondweapon == 17 )
	{
		do_shatols( ch, "" );
	}
	if( ship->secondweapon == 18 )
	{
		do_missiles( ch, "" );
	}
	if( ship->secondweapon == 19 )
	{
		do_crosscrusher( ch, "" );
	}
	if( ship->secondweapon == 20 )
	{
		do_bigbeamsabre( ch, "" );
	}
	if( ship->secondweapon == 21 )
	{
		do_heatrod( ch, "" );
	}
	if( ship->secondweapon == 22 )
	{
		do_beamglaive( ch, "" );
	}
	if( ship->secondweapon == 23 )
	{
		do_dragonfang( ch, "" );
	}
	if( ship->secondweapon == 24 )
	{
		do_flamethrower( ch, "" );
	}
	if( ship->secondweapon == 25 )
	{
		do_dobergun( ch, "" );
	}
	if( ship->secondweapon == 26 )
	{
		do_shortblast( ch, "" );
	}
	if( ship->secondweapon == 27 )
	{
		do_longblast( ch, "" );
	}
	if( ship->secondweapon == 28 )
	{
		do_smallbeamcannon( ch, "" );
	}
	if( ship->secondweapon == 29 )
	{
		do_beamblade( ch, "" );
	}
	if( ship->secondweapon == 30 )
	{
		do_planetdefenser( ch, "" );
	}
	if( ship->secondweapon == 31 )
	{
		do_libramaincannon( ch, "" );
	}
	if( ship->secondweapon == 32 )
	{
		do_tbeamscythe( ch, "" );
	}
	if( ship->secondweapon == 33 )
	{
		do_tbusterrifle( ch, "" );
	}
	if( ship->secondweapon == 34 )
	{
		do_megacannon( ch, "" );
	}
	if( ship->secondweapon == 35 )
	{
		do_tbeamglaive( ch, "" );
	}
	if( ship->secondweapon == 39 )
	{
		do_i250machine( ch, "" );
	}
	if( ship->secondweapon == 40 )
	{
		send_to_char( "&RNo weapon equiped\r\n", ch );
	}
	return;
}

CMDF( do_thirdweapon )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a mobile suit to do that!\r\n", ch );
		return;
	}

	if( ship->targettype == 0 )
	{
		send_to_char( "&RChoose a suit part to target first!\r\n", ch );
		return;
	}

	if( ship->mod == 3 && ship->offon == 1 )
	{
		send_to_char( "&RYou can't fight while transformed!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		ch->mental_state += 1;
		if( ch->mental_state == 85 )
		{
			send_to_char( "&CYou're getting closer to the edge.. Your mind can't handle much more..\r\n", ch );
		}
		if( ch->mental_state == 100 )
		{
			REMOVE_BIT( ch->pcdata->flags, PCFLAG_ZERO );
			snprintf( buf, MAX_STRING_LENGTH, "%s explodes in a blinding flash of light!", ship->name );
			echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
			snprintf( buf, MAX_STRING_LENGTH, "&YT&Oh&Ye &zZ&We&zr&Wo &CS&cy&Cs&ct&Ce&cm &GH&ga&Gs &BJ&bu&Bs&bt &P*&p*&P*&RR&ri&RPP&re&RD&P*&p*&P* &BA&bp&ca&br&Bt &C%s's &YM&Oin&Yd&P!&p!&P!", ch->name );
			info_chan( buf );
			snprintf( log_buf, MAX_STRING_LENGTH, "%s killed by the Zero System.", ch->name );
			log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
			destroy_ship( ship, NULL );

		}

	}

	if( ship->thirdweapon == 0 )
	{
		do_i100machine( ch, "" );
	}
	if( ship->thirdweapon == 1 )
	{
		do_i105rifle( ch, "" );
	}
	if( ship->thirdweapon == 2 )
	{
		do_leobazooka( ch, "" );
	}
	if( ship->thirdweapon == 3 )
	{
		do_beamsabre( ch, "" );
	}
	if( ship->thirdweapon == 4 )
	{
		do_shouldercannon( ch, "" );
	}
	if( ship->thirdweapon == 5 )
	{
		do_sidemissilelauncher( ch, "" );
	}
	if( ship->thirdweapon == 6 )
	{
		do_beamrifle( ch, "" );
	}
	if( ship->thirdweapon == 7 )
	{
		do_lasercannon( ch, "" );
	}
	if( ship->thirdweapon == 8 )
	{
		do_twingattling( ch, "" );
	}
	if( ship->thirdweapon == 9 )
	{
		do_beamcannon( ch, "" );
	}
	if( ship->thirdweapon == 10 )
	{
		do_busterrifle( ch, "" );
	}
	if( ship->thirdweapon == 11 )
	{
		do_headvulcan( ch, "" );
	}
	if( ship->thirdweapon == 12 )
	{
		do_beamscythe( ch, "" );
	}
	if( ship->thirdweapon == 13 )
	{
		do_bustershield( ch, "" );
	}
	if( ship->thirdweapon == 14 )
	{
		do_beamgattling( ch, "" );
	}
	if( ship->thirdweapon == 15 )
	{
		do_multiblast( ch, "" );
	}
	if( ship->thirdweapon == 16 )
	{
		do_armyknife( ch, "" );
	}
	if( ship->thirdweapon == 17 )
	{
		do_shatols( ch, "" );
	}
	if( ship->thirdweapon == 18 )
	{
		do_missiles( ch, "" );
	}
	if( ship->thirdweapon == 19 )
	{
		do_crosscrusher( ch, "" );
	}
	if( ship->thirdweapon == 20 )
	{
		do_bigbeamsabre( ch, "" );
	}
	if( ship->thirdweapon == 21 )
	{
		do_heatrod( ch, "" );
	}
	if( ship->thirdweapon == 22 )
	{
		do_beamglaive( ch, "" );
	}
	if( ship->thirdweapon == 23 )
	{
		do_dragonfang( ch, "" );
	}
	if( ship->thirdweapon == 24 )
	{
		do_flamethrower( ch, "" );
	}
	if( ship->thirdweapon == 25 )
	{
		do_dobergun( ch, "" );
	}
	if( ship->thirdweapon == 26 )
	{
		do_shortblast( ch, "" );
	}
	if( ship->thirdweapon == 27 )
	{
		do_longblast( ch, "" );
	}
	if( ship->thirdweapon == 28 )
	{
		do_smallbeamcannon( ch, "" );
	}
	if( ship->thirdweapon == 29 )
	{
		do_beamblade( ch, "" );
	}
	if( ship->thirdweapon == 30 )
	{
		do_planetdefenser( ch, "" );
	}
	if( ship->thirdweapon == 31 )
	{
		do_libramaincannon( ch, "" );
	}
	if( ship->thirdweapon == 32 )
	{
		do_tbeamscythe( ch, "" );
	}
	if( ship->thirdweapon == 33 )
	{
		do_tbusterrifle( ch, "" );
	}
	if( ship->thirdweapon == 34 )
	{
		do_megacannon( ch, "" );
	}
	if( ship->thirdweapon == 35 )
	{
		do_tbeamglaive( ch, "" );
	}
	if( ship->thirdweapon == 39 )
	{
		do_i250machine( ch, "" );
	}
	if( ship->thirdweapon == 40 )
	{
		send_to_char( "&RNo weapon equiped\r\n", ch );
	}
	return;
}
