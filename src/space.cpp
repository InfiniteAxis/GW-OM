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


#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

SHIP_DATA *first_ship;
SHIP_DATA *last_ship;

MISSILE_DATA *first_missile;
MISSILE_DATA *last_missile;

SPACE_DATA *first_starsystem;
SPACE_DATA *last_starsystem;

int bus_pos = 0;
int bus_planet = 0;
int bus2_planet = 4;
int turbocar_stop = 0;
int corus_shuttle = 0;
int turbo_planet = 0;

#define MAX_COORD 15000000
#define MAX_COORD_S 13000000

#define MAX_STATION     13
#define MAX_BUS_STOP     5

#define STOP_PLANET     500
#define STOP_SHIPYARD   32015

int const station_vnum[MAX_STATION] = {
   500, 5100, 2000, 6800, 1300, 807,
   6000, 8000, 6200, 200, 4001, 4100,
   1627
};

const char *const station_name[MAX_STATION] = {
   "G42", "Z64", "R28", "X-18999", "S19", "Tokyo",
   "K46", "Ruined Earth", "M9", "L3", "New Saturn",
   "Rommefellar", "Moon"
};

int const bus_vnum[MAX_BUS_STOP] = {
   500, 5100, 2000, 6800, 1300
};

const char *const bus_stop[MAX_BUS_STOP + 1] = {
   "G42 Primary Pad", "Z64 Docking bay", "R28 Landing Pad",
   "Colony X-18999", "S19 Landing Pad", "G42 Primary Pad"
   /*
	* last should always be same as first
	*/
};

const char *const ship_flags[] = {
   "cloak", "incombat", "hasone", "hastwo", "guarding", "mobsuit", "newsystem",
   "ingrid", "moving", "boosting", "phaseshift", "transphase", "panzer",
   "laminated", "antibeam", "phaseshifton",
   "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

int get_shipflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, ship_flags[x] ) )
			return x;
	return -1;
}

/* local routines */
void fread_ship( SHIP_DATA *ship, FILE *fp );
void landstat( CHAR_DATA *ch, SHIP_DATA *ship );
bool load_ship_file( const char *shipfile );
void write_ship_list( void );
void fread_starsystem( SPACE_DATA *starsystem, FILE *fp );
bool load_starsystem( const char *starsystemfile );
void write_starsystem_list( void );
void resetship( SHIP_DATA *ship );
void landship( SHIP_DATA *ship, const char *arg );
void launchship( SHIP_DATA *ship );
bool land_bus( SHIP_DATA *ship, int destination );
void launch_bus( SHIP_DATA *ship );
void echo_to_room_dnr( int ecolor, ROOM_INDEX_DATA *room, const char *argument );
ch_ret drive_ship( CHAR_DATA *ch, SHIP_DATA *ship, EXIT_DATA *exit, int fall );
bool autofly( SHIP_DATA *ship );
bool is_facing( SHIP_DATA *ship, SHIP_DATA *target );
void sound_to_ship( SHIP_DATA *ship, const char *argument );
int get_suit_weapon( const char *type );

ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );

void echo_to_room_dnr( int ecolor, ROOM_INDEX_DATA *room, const char *argument )
{
	CHAR_DATA *vic;

	if( room == NULL )
		return;

	for( vic = room->first_person; vic; vic = vic->next_in_room )
	{
		set_char_color( ecolor, vic );
		send_to_char( argument, vic );
	}
}


bool land_bus( SHIP_DATA *ship, int destination )
{
	char buf[MAX_STRING_LENGTH];

	if( !ship_to_room( ship, destination ) )
	{
		return false;
	}
	echo_to_ship( AT_YELLOW, ship, "You feel a slight thud as your suit lands on the ground." );
	ship->location = destination;
	ship->lastdoc = ship->location;
	ship->shipstate = SHIP_DOCKED;
	if( ship->starsystem )
		ship_from_starsystem( ship, ship->starsystem );
	snprintf( buf, MAX_STRING_LENGTH, "%s lands on the platform.", ship->name );
	echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
	snprintf( buf, MAX_STRING_LENGTH, "The pilot hatch on %s opens.", ship->name );
	echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
	echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The pilot hatch opens." );
	ship->hatchopen = true;
	return true;
}

void launch_bus( SHIP_DATA *ship )
{
	char buf[MAX_STRING_LENGTH];

	snprintf( buf, MAX_STRING_LENGTH, "The pilot hatch on %s closes and it begins to launch.", ship->name );
	echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
	echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The pilot hatch slides shut." );
	ship->hatchopen = false;
	extract_ship( ship );
	echo_to_ship( AT_YELLOW, ship, "The mobile suit begins to launch." );
	ship->location = 0;
	ship->shipstate = SHIP_READY;
}

void update_traffic( )
{
	SHIP_DATA *shuttle;
	SHIP_DATA *turbocar;
	char buf[MAX_STRING_LENGTH];

	shuttle = ship_from_cockpit( ROOM_CORUSCANT_SHUTTLE );
	if( shuttle != NULL )
	{
		switch( corus_shuttle )
		{
		default:
			corus_shuttle++;
			break;

		case 0:
			land_bus( shuttle, STOP_PLANET );
			corus_shuttle++;
			echo_to_ship( AT_CYAN, shuttle, "Welcome to Menari Spaceport." );
			break;

		case 4:
			launch_bus( shuttle );
			corus_shuttle++;
			break;

		case 5:
			land_bus( shuttle, STOP_SHIPYARD );
			echo_to_ship( AT_CYAN, shuttle, "Welcome to Coruscant Shipyard." );
			corus_shuttle++;
			break;

		case 9:
			launch_bus( shuttle );
			corus_shuttle++;
			break;

		}

		if( corus_shuttle >= 10 )
			corus_shuttle = 0;
	}

	turbocar = ship_from_cockpit( ROOM_CORUSCANT_TURBOCAR );
	if( turbocar != NULL )
	{
		snprintf( buf, MAX_STRING_LENGTH, "The Star Shuttle hatch closes and it launches back into space." );
		echo_to_room( AT_YELLOW, get_room_index( turbocar->location ), buf );
		extract_ship( turbocar );
		turbocar->location = 0;
		ship_to_room( turbocar, station_vnum[turbocar_stop] );
		echo_to_ship( AT_YELLOW, turbocar, "The Star Shuttle makes fast journey to the next Landing Pad." );
		turbocar->location = station_vnum[turbocar_stop];
		turbocar->lastdoc = turbocar->location;
		turbocar->shipstate = SHIP_DOCKED;
		if( turbocar->starsystem )
			ship_from_starsystem( turbocar, turbocar->starsystem );
		snprintf( buf, MAX_STRING_LENGTH, "The Star Shuttle lands on the Landing Pad." );
		echo_to_room( AT_YELLOW, get_room_index( turbocar->location ), buf );
		snprintf( buf, MAX_STRING_LENGTH, "Welcome to: %s.", station_name[turbocar_stop] );
		echo_to_ship( AT_CYAN, turbocar, buf );
		turbocar->hatchopen = true;

		turbocar_stop++;
		if( turbocar_stop >= MAX_STATION )
			turbocar_stop = 0;
	}

}

void update_bus( )
{
	SHIP_DATA *ship;
	SHIP_DATA *ship2;
	SHIP_DATA *target;
	int destination;
	char buf[MAX_STRING_LENGTH];

	ship = ship_from_cockpit( ROOM_SHUTTLE_BUS );
	ship2 = ship_from_cockpit( ROOM_SHUTTLE_BUS_2 );

	if( ship == NULL && ship2 == NULL )
		return;

	switch( bus_pos )
	{

	case 0:
		target = ship_from_hanger( bus_vnum[bus_planet] );
		if( target != NULL && !target->starsystem )
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Cannot land suit at %s ... it seems to have dissapeared.'",
				bus_stop[bus_planet] );
			echo_to_ship( AT_CYAN, ship, buf );
			bus_pos = 5;
		}

		target = ship_from_hanger( bus_vnum[bus2_planet] );
		if( target != NULL && !target->starsystem )
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Cannot land suit at %s ... it seems to have dissapeared.'",
				bus_stop[bus_planet] );
			echo_to_ship( AT_CYAN, ship2, buf );
			bus_pos = 5;
		}

		bus_pos++;
		break;

	case 6:
		launch_bus( ship );
		launch_bus( ship2 );
		bus_pos++;
		break;

	case 7:
		echo_to_ship( AT_YELLOW, ship, "Your suit jumps forward as you jump into hyperspace." );
		echo_to_ship( AT_YELLOW, ship2, "Your suit jumps forward as you jump into hyperspace." );
		bus_pos++;
		break;

	case 9:

		echo_to_ship( AT_YELLOW, ship, "The suit lurches slightly as it comes out of hyperspace.." );
		echo_to_ship( AT_YELLOW, ship2, "The suit lurches slightly as it comes out of hyperspace.." );
		bus_pos++;
		break;

	case 1:
		destination = bus_vnum[bus_planet];
		if( !land_bus( ship, destination ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus_planet] );
			echo_to_ship( AT_CYAN, ship, buf );
			echo_to_ship( AT_CYAN, ship, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
		}
		else
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Welcome to %s'", bus_stop[bus_planet] );
			echo_to_ship( AT_CYAN, ship, buf );
			echo_to_ship( AT_CYAN, ship, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
		}
		destination = bus_vnum[bus2_planet];
		if( !land_bus( ship2, destination ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", bus_stop[bus_planet] );
			echo_to_ship( AT_CYAN, ship2, buf );
			echo_to_ship( AT_CYAN, ship2, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
		}
		else
		{
			snprintf( buf, MAX_STRING_LENGTH, "An electronic voice says, 'Welcome to %s'", bus_stop[bus2_planet] );
			echo_to_ship( AT_CYAN, ship2, buf );
			echo_to_ship( AT_CYAN, ship2, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
		}
		bus_pos++;
		break;

	case 5:
		snprintf( buf, MAX_STRING_LENGTH, "It continues, 'Next stop, %s'", bus_stop[bus_planet + 1] );
		echo_to_ship( AT_CYAN, ship, "An electronic voice says, 'Preparing for launch.'" );
		echo_to_ship( AT_CYAN, ship, buf );
		snprintf( buf, MAX_STRING_LENGTH, "It continues, 'Next stop, %s'", bus_stop[bus2_planet + 1] );
		echo_to_ship( AT_CYAN, ship2, "An electronic voice says, 'Preparing for launch.'" );
		echo_to_ship( AT_CYAN, ship2, buf );
		bus_pos++;
		break;

	default:
		bus_pos++;
		break;
	}

	if( bus_pos >= 10 )
	{
		bus_pos = 0;
		bus_planet++;
		bus2_planet++;
	}

	if( bus_planet >= MAX_BUS_STOP )
		bus_planet = 0;
	if( bus2_planet >= MAX_BUS_STOP )
		bus2_planet = 0;

}

void move_ships( )
{
	SHIP_DATA *ship;
	MISSILE_DATA *missile;
	MISSILE_DATA *m_next;
	SHIP_DATA *target;
	float dx, dy, dz, change;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	bool ch_found = false;



	for( missile = first_missile; missile; missile = m_next )
	{
		m_next = missile->next;

		ship = missile->fired_from;
		target = missile->target;

		if( target->starsystem && target->starsystem == missile->starsystem )
		{
			if( missile->mx < target->vx )
				missile->mx += UMIN( missile->speed / 5, target->vx - missile->mx );
			else if( missile->mx > target->vx )
				missile->mx -= UMIN( missile->speed / 5, missile->mx - target->vx );
			if( missile->my < target->vy )
				missile->my += UMIN( missile->speed / 5, target->vy - missile->my );
			else if( missile->my > target->vy )
				missile->my -= UMIN( missile->speed / 5, missile->my - target->vy );
			if( missile->mz < target->vz )
				missile->mz += UMIN( missile->speed / 5, target->vz - missile->mz );
			else if( missile->mz > target->vz )
				missile->mz -= UMIN( missile->speed / 5, missile->mz - target->vz );

			if( abs( missile->mx ) - abs( target->vx ) <= 20 && abs( missile->mx ) - abs( target->vx ) >= -20
				&& abs( missile->my ) - abs( target->vy ) <= 20 && abs( missile->my ) - abs( target->vy ) >= -20
				&& abs( missile->mz ) - abs( target->vz ) <= 20 && abs( missile->mz ) - abs( target->vz ) >= -20 )
			{
				if( target->chaff_released <= 0 )
				{
					echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Your missile hits its target dead on!" );
					echo_to_cockpit( AT_BLOOD, target, "The ship is smashed by a Beam Cannon blast!" );
					echo_to_ship( AT_RED, target, "A loud explosion shakes your suit violently!" );
					snprintf( buf, MAX_STRING_LENGTH, "You see a small explosion as %s is hit by a Beam Cannon", target->name );
					echo_to_system( AT_ORANGE, target, buf, ship );
					for( ch = first_char; ch; ch = ch->next )
						if( !IS_NPC( ch ) && nifty_is_name( missile->fired_by, ch->name ) )
						{
							ch_found = true;
							damage_ship_ch( target, 20 + missile->missiletype * missile->missiletype * 20,
								30 + missile->missiletype * missile->missiletype * missile->missiletype * 30, ch );
						}
					if( !ch_found )
						damage_ship( target, 20 + missile->missiletype * missile->missiletype * 20,
							30 + missile->missiletype * missile->missiletype * ship->missiletype * 30 );
					extract_missile( missile );
				}
				else
				{
					echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ),
						"Your missile explodes harmlessly in a cloud of chaff!" );
					echo_to_cockpit( AT_YELLOW, target, "A missile explodes in your chaff." );
					extract_missile( missile );
				}
				continue;
			}
			else
			{
				missile->age++;
				if( missile->age >= 50 )
				{
					extract_missile( missile );
					continue;
				}
			}
		}
		else
		{
			extract_missile( missile );
			continue;
		}

	}

	for( ship = first_ship; ship; ship = ship->next )
	{

		if( !ship->starsystem )
			continue;

		if( ship->currspeed > 0 )
		{

			change = sqrt( ship->hx * ship->hx + ship->hy * ship->hy + ship->hz * ship->hz );

			if( change > 0 )
			{
				dx = ship->hx / change;
				dy = ship->hy / change;
				dz = ship->hz / change;
				ship->vx += ( dx * ship->currspeed / 5 );
				ship->vy += ( dy * ship->currspeed / 5 );
				ship->vz += ( dz * ship->currspeed / 5 );
			}

		}

		if( ship->vx > MAX_COORD )
			ship->vx = -MAX_COORD_S;
		if( ship->vy > MAX_COORD )
			ship->vy = -MAX_COORD_S;
		if( ship->vz > MAX_COORD )
			ship->vz = -MAX_COORD_S;
		if( ship->vx < -MAX_COORD )
			ship->vx = MAX_COORD_S;
		if( ship->vy < -MAX_COORD )
			ship->vy = MAX_COORD_S;
		if( ship->vz < -MAX_COORD )
			ship->vz = MAX_COORD_S;

		if( autofly( ship ) )
			continue;

		/*
				  if ( ship->ship_class != SHIP_PLATFORM && !autofly(ship) )
				  {
					if ( ship->starsystem->star1 && strcmp(ship->starsystem->star1,"") )
					{
					  if (ship->vx >= ship->starsystem->s1x + 1 || ship->vx <= ship->starsystem->s1x - 1 )
						ship->vx -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vx - ship->starsystem->s1x)/2,3);
					  if (ship->vy >= ship->starsystem->s1y + 1 || ship->vy <= ship->starsystem->s1y - 1 )
						ship->vy -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vy - ship->starsystem->s1y)/2,3);
					  if (ship->vz >= ship->starsystem->s1z + 1 || ship->vz <= ship->starsystem->s1z - 1 )
						ship->vz -= URANGE(-3,(ship->starsystem->gravitys1)/(ship->vz - ship->starsystem->s1z)/2,3);
					}

					if ( ship->starsystem->star2 && strcmp(ship->starsystem->star2,"") )
					{
					  if (ship->vx >= ship->starsystem->s2x + 1 || ship->vx <= ship->starsystem->s2x - 1 )
						ship->vx -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vx - ship->starsystem->s2x)/2,3);
					  if (ship->vy >= ship->starsystem->s2y + 1 || ship->vy <= ship->starsystem->s2y - 1 )
						ship->vy -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vy - ship->starsystem->s2y)/2,3);
					  if (ship->vz >= ship->starsystem->s2z + 1 || ship->vz <= ship->starsystem->s2z - 1 )
						ship->vz -= URANGE(-3,(ship->starsystem->gravitys2)/(ship->vz - ship->starsystem->s2z)/2,3);
					}

					if ( ship->starsystem->planet1 && strcmp(ship->starsystem->planet1,"") )
					{
					  if (ship->vx >= ship->starsystem->p1x + 1 || ship->vx <= ship->starsystem->p1x - 1 )
						ship->vx -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vx - ship->starsystem->p1x)/2,3);
					  if (ship->vy >= ship->starsystem->p1y + 1 || ship->vy <= ship->starsystem->p1y - 1 )
						ship->vy -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vy - ship->starsystem->p1y)/2,3);
					  if (ship->vz >= ship->starsystem->p1z + 1 || ship->vz <= ship->starsystem->p1z - 1 )
						ship->vz -= URANGE(-3,(ship->starsystem->gravityp1)/(ship->vz - ship->starsystem->p1z)/2,3);
					}

					if ( ship->starsystem->planet2 && strcmp(ship->starsystem->planet2,"") )
					{
					  if (ship->vx >= ship->starsystem->p2x + 1 || ship->vx <= ship->starsystem->p2x - 1 )
						ship->vx -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vx - ship->starsystem->p2x)/2,3);
					  if (ship->vy >= ship->starsystem->p2y + 1 || ship->vy <= ship->starsystem->p2y - 1 )
						ship->vy -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vy - ship->starsystem->p2y)/2,3);
					  if (ship->vz >= ship->starsystem->p2z + 1 || ship->vz <= ship->starsystem->p2z - 1 )
						ship->vz -= URANGE(-3,(ship->starsystem->gravityp2)/(ship->vz - ship->starsystem->p2z)/2,3);
					}

					if ( ship->starsystem->planet3 && strcmp(ship->starsystem->planet3,"") )
					{
					  if (ship->vx >= ship->starsystem->p3x + 1 || ship->vx <= ship->starsystem->p3x - 1 )
						ship->vx -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vx - ship->starsystem->p3x)/2,3);
					  if (ship->vy >= ship->starsystem->p3y + 1 || ship->vy <= ship->starsystem->p3y - 1 )
						ship->vy -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vy - ship->starsystem->p3y)/2,3);
					  if (ship->vz >= ship->starsystem->p3z + 1 || ship->vz <= ship->starsystem->p3z - 1 )
						ship->vz -= URANGE(-3,(ship->starsystem->gravityp3)/(ship->vz - ship->starsystem->p3z)/2,3);
					}
				  }

		*/
		/*
				  for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem)
				  {
						if ( target != ship &&
							abs(ship->vx - target->vx) < 1 &&
							abs(ship->vy - target->vy) < 1 &&
							abs(ship->vz - target->vz) < 1 )
						{
							ship->collision = target->maxarmor;
							target->collision = ship->maxarmor;
						}
				  }
		*/
		if( ship->starsystem->star1 && strcmp( ship->starsystem->star1, "" ) &&
			abs( ship->vx - ship->starsystem->s1x ) < 10 &&
			abs( ship->vy - ship->starsystem->s1y ) < 10 && abs( ship->vz - ship->starsystem->s1z ) < 10 )
		{
			echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "You fly directly into the sun." );
			snprintf( buf, MAX_STRING_LENGTH, "%s flys directly into %s!", ship->name, ship->starsystem->star1 );
			echo_to_system( AT_ORANGE, ship, buf, NULL );
			destroy_ship( ship, NULL );
			continue;
		}
		if( ship->starsystem->star2 && strcmp( ship->starsystem->star2, "" ) &&
			abs( ship->vx - ship->starsystem->s2x ) < 10 &&
			abs( ship->vy - ship->starsystem->s2y ) < 10 && abs( ship->vz - ship->starsystem->s2z ) < 10 )
		{
			echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "You fly directly into the sun." );
			snprintf( buf, MAX_STRING_LENGTH, "%s flys directly into %s!", ship->name, ship->starsystem->star2 );
			echo_to_system( AT_ORANGE, ship, buf, NULL );
			destroy_ship( ship, NULL );
			continue;
		}

		if( ship->currspeed > 0 )
		{
			if( ship->starsystem->planet1 && strcmp( ship->starsystem->planet1, "" ) &&
				abs( ship->vx - ship->starsystem->p1x ) < 10 &&
				abs( ship->vy - ship->starsystem->p1y ) < 10 && abs( ship->vz - ship->starsystem->p1z ) < 10 )
			{
				snprintf( buf, MAX_STRING_LENGTH, "You begin orbitting %s.", ship->starsystem->planet1 );
				echo_to_cockpit( AT_YELLOW, ship, buf );
				snprintf( buf, MAX_STRING_LENGTH, "%s begins orbiting %s.", ship->name, ship->starsystem->planet1 );
				echo_to_system( AT_ORANGE, ship, buf, NULL );
				ship->currspeed = 0;
				continue;
			}
			if( ship->starsystem->planet2 && strcmp( ship->starsystem->planet2, "" ) &&
				abs( ship->vx - ship->starsystem->p2x ) < 10 &&
				abs( ship->vy - ship->starsystem->p2y ) < 10 && abs( ship->vz - ship->starsystem->p2z ) < 10 )
			{
				snprintf( buf, MAX_STRING_LENGTH, "You begin orbitting %s.", ship->starsystem->planet2 );
				echo_to_cockpit( AT_YELLOW, ship, buf );
				snprintf( buf, MAX_STRING_LENGTH, "%s begins orbiting %s.", ship->name, ship->starsystem->planet2 );
				echo_to_system( AT_ORANGE, ship, buf, NULL );
				ship->currspeed = 0;
				continue;
			}
			if( ship->starsystem->planet3 && strcmp( ship->starsystem->planet3, "" ) &&
				abs( ship->vx - ship->starsystem->p3x ) < 10 &&
				abs( ship->vy - ship->starsystem->p3y ) < 10 && abs( ship->vz - ship->starsystem->p3z ) < 10 )
			{
				snprintf( buf, MAX_STRING_LENGTH, "You begin orbitting %s.", ship->starsystem->planet2 );
				echo_to_cockpit( AT_YELLOW, ship, buf );
				snprintf( buf, MAX_STRING_LENGTH, "%s begins orbiting %s.", ship->name, ship->starsystem->planet2 );
				echo_to_system( AT_ORANGE, ship, buf, NULL );
				ship->currspeed = 0;
				continue;
			}
		}

		if( ship->vx > MAX_COORD )
			ship->vx = -MAX_COORD_S;
		if( ship->vy > MAX_COORD )
			ship->vy = -MAX_COORD_S;
		if( ship->vz > MAX_COORD )
			ship->vz = -MAX_COORD_S;
		if( ship->vx > MAX_COORD )
			ship->vx = MAX_COORD_S;
		if( ship->vy > MAX_COORD )
			ship->vy = MAX_COORD_S;
		if( ship->vz > MAX_COORD )
			ship->vz = MAX_COORD_S;

	}

	for( ship = first_ship; ship; ship = ship->next )
		if( ship->collision )
		{
			echo_to_cockpit( AT_WHITE + AT_BLINK, ship, "You have collided with another suit!" );
			echo_to_ship( AT_RED, ship, "A loud explosion shakes your suit violently!" );
			damage_ship( ship, ship->collision, ship->collision );
			ship->collision = 0;
		}
}

void recharge_ships( )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	for( ship = first_ship; ship; ship = ship->next )
	{

		if( ship->statet0 > 0 )
		{
			ship->energy -= ship->statet0;
			ship->statet0 = 0;
		}
		if( ship->statet1 > 0 )
		{
			ship->energy -= ship->statet1;
			ship->statet1 = 0;
		}
		if( ship->statet2 > 0 )
		{
			ship->energy -= ship->statet2;
			ship->statet2 = 0;
		}

		if( ship->missilestate == MISSILE_RELOAD_2 )
		{
			ship->missilestate = MISSILE_READY;
			if( ship->missiles > 0 )
				echo_to_room( AT_YELLOW, get_room_index( ship->gunseat ), "Missile launcher reloaded." );
		}

		if( ship->missilestate == MISSILE_RELOAD )
		{
			ship->missilestate = MISSILE_RELOAD_2;
		}

		if( ship->missilestate == MISSILE_FIRED )
			ship->missilestate = MISSILE_RELOAD;

		if( autofly( ship ) )
		{
			if( ship->starsystem )
			{
				if( ship->target0 && ship->statet0 != LASER_DAMAGED )
				{
					int chance = 50;
					SHIP_DATA *target = ship->target0;
					int shots;

					for( shots = 0; shots <= ship->lasers; shots++ )
					{
						if( ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
							&& ship->target0->starsystem == ship->starsystem
							&& abs( target->vx - ship->vx ) <= 1000
							&& abs( target->vy - ship->vy ) <= 1000
							&& abs( target->vz - ship->vz ) <= 1000 && ship->statet0 < ship->lasers )
						{
							if( ship->ship_class > 1 || is_facing( ship, target ) )
							{
								chance += target->ship_class * 25;
								chance -= target->manuever / 10;
								chance -= target->currspeed / 20;
								chance -= ( abs( target->vx - ship->vx ) / 70 );
								chance -= ( abs( target->vy - ship->vy ) / 70 );
								chance -= ( abs( target->vz - ship->vz ) / 70 );
								chance = URANGE( 10, chance, 90 );
								if( number_percent( ) > chance )
								{
									snprintf( buf, MAX_STRING_LENGTH, "%s fires its Machine Guns at you, but misses.", ship->name );
									echo_to_cockpit( AT_ORANGE, target, buf );
									snprintf( buf, MAX_STRING_LENGTH, "%s fires its Machine Guns at %s but misses.", ship->name, target->name );
									echo_to_system( AT_ORANGE, target, buf, NULL );
								}
								else
								{
									snprintf( buf, MAX_STRING_LENGTH, "%s Machine Gun fire blasts %s!", ship->name, target->name );
									echo_to_system( AT_ORANGE, target, buf, NULL );
									snprintf( buf, MAX_STRING_LENGTH, "%s blasts you with its Machine Guns!", ship->name );
									echo_to_cockpit( AT_BLOOD, target, buf );
									echo_to_ship( AT_RED, target, "A small explosion vibrates through the suit." );
									damage_ship( target, 15, 20 );
								}
								ship->statet0++;
							}
						}
					}
				}
			}
		}

	}
}

void update_space( )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];
	int too_close, target_too_close;
	int recharge;

	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->starsystem )
		{
			if( ship->energy > 0 && ship->shipstate == SHIP_DISABLED && ship->ship_class != SHIP_PLATFORM )
				ship->energy -= 100;
			else if( ship->energy > 0 )
				ship->energy += ( 5 + ship->ship_class * 5 );
			else
				destroy_ship( ship, NULL );
		}

		if( ship->chaff_released > 0 )
			ship->chaff_released--;

		if( ship->shipstate == SHIP_HYPERSPACE )
		{
			ship->hyperdistance -= ship->hyperspeed * 2;
			if( ship->hyperdistance <= 0 )
			{
				ship_to_starsystem( ship, ship->currjump );

				if( ship->starsystem == NULL )
				{
					echo_to_cockpit( AT_RED, ship, "Ship lost in Hyperspace. Make new calculations." );
				}
				else
				{
					echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Hyperjump complete." );
					echo_to_ship( AT_YELLOW, ship, "Your mobile suit lurches slightly as it comes out of hyperspace." );
					snprintf( buf, MAX_STRING_LENGTH, "%s enters the starsystem at %.0f %.0f %.0f", ship->name, ship->vx, ship->vy, ship->vz );
					echo_to_system( AT_YELLOW, ship, buf, NULL );
					ship->shipstate = SHIP_READY;
					STRFREE( ship->home );
					ship->home = STRALLOC( ship->starsystem->name );
					if( str_cmp( "Public", ship->owner ) )
						save_ship( ship );

				}
			}
			else
			{
				snprintf( buf, MAX_STRING_LENGTH, "%d", ship->hyperdistance );
				echo_to_room_dnr( AT_YELLOW, get_room_index( ship->pilotseat ), "Remaining jump distance: " );
				echo_to_room( AT_WHITE, get_room_index( ship->pilotseat ), buf );

			}
		}

		/*
		 * following was originaly to fix ships that lost their pilot
		 * in the middle of a manuever and are stuck in a busy state
		 * but now used for timed manouevers such as turning
		 */

		if( ship->shipstate == SHIP_BUSY_3 )
		{
			echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Manuever complete." );
			ship->shipstate = SHIP_READY;
		}
		if( ship->shipstate == SHIP_BUSY_2 )
			ship->shipstate = SHIP_BUSY_3;
		if( ship->shipstate == SHIP_BUSY )
			ship->shipstate = SHIP_BUSY_2;

		if( ship->shipstate == SHIP_LAND_2 )
			landship( ship, ship->dest );
		if( ship->shipstate == SHIP_LAND )
			ship->shipstate = SHIP_LAND_2;

		if( ship->shipstate == SHIP_LAUNCH_2 )
			launchship( ship );
		if( ship->shipstate == SHIP_LAUNCH )
			ship->shipstate = SHIP_LAUNCH_2;


		ship->shield = UMAX( 0, ship->shield - 1 - ship->ship_class );

		if( ship->autorecharge && ship->maxshield > ship->shield && ship->energy > 100 )
		{
			recharge = UMIN( ship->maxshield - ship->shield, 10 + ship->ship_class * 10 );
			recharge = UMIN( recharge, ship->energy / 2 - 100 );
			recharge = UMAX( 1, recharge );
			ship->shield += recharge;
			ship->energy -= recharge;
		}

		if( ship->shield > 0 )
		{
			if( ship->energy < 200 )
			{
				ship->shield = 0;
				echo_to_cockpit( AT_RED, ship,
					"The shields around your suit pulse quickly and then die as they run out of power." );
				ship->autorecharge = false;
			}
		}

		if( ship->starsystem && ship->currspeed > 0 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%d", ship->currspeed );
			echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "Speed: " );
			echo_to_room_dnr( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
			snprintf( buf, MAX_STRING_LENGTH, "%.0f %.0f %.0f", ship->vx, ship->vy, ship->vz );
			echo_to_room_dnr( AT_BLUE, get_room_index( ship->pilotseat ), "  Coords: " );
			echo_to_room( AT_LBLUE, get_room_index( ship->pilotseat ), buf );
			if( ship->pilotseat != ship->coseat )
			{
				snprintf( buf, MAX_STRING_LENGTH, "%d", ship->currspeed );
				echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "Speed: " );
				echo_to_room_dnr( AT_LBLUE, get_room_index( ship->coseat ), buf );
				snprintf( buf, MAX_STRING_LENGTH, "%.0f %.0f %.0f", ship->vx, ship->vy, ship->vz );
				echo_to_room_dnr( AT_BLUE, get_room_index( ship->coseat ), "  Coords: " );
				echo_to_room( AT_LBLUE, get_room_index( ship->coseat ), buf );
			}
		}

		if( ship->starsystem )
		{
			too_close = ship->currspeed + 50;
			for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
			{
				target_too_close = too_close + target->currspeed;
				if( target != ship &&
					abs( ship->vx - target->vx ) < target_too_close &&
					abs( ship->vy - target->vy ) < target_too_close && abs( ship->vz - target->vz ) < target_too_close )
				{
					snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %.0f %.0f %.0f", target->name, target->vx, target->vy, target->vz );
					echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
				}
			}
			too_close = ship->currspeed + 100;
			if( ship->starsystem->star1 && strcmp( ship->starsystem->star1, "" ) &&
				abs( ship->vx - ship->starsystem->s1x ) < too_close &&
				abs( ship->vy - ship->starsystem->s1y ) < too_close && abs( ship->vz - ship->starsystem->s1z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->star1,
					ship->starsystem->s1x, ship->starsystem->s1y, ship->starsystem->s1z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->star2 && strcmp( ship->starsystem->star2, "" ) &&
				abs( ship->vx - ship->starsystem->s2x ) < too_close &&
				abs( ship->vy - ship->starsystem->s2y ) < too_close && abs( ship->vz - ship->starsystem->s2z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->star2,
					ship->starsystem->s2x, ship->starsystem->s2y, ship->starsystem->s2z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet1 && strcmp( ship->starsystem->planet1, "" ) &&
				abs( ship->vx - ship->starsystem->p1x ) < too_close &&
				abs( ship->vy - ship->starsystem->p1y ) < too_close && abs( ship->vz - ship->starsystem->p1z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet1,
					ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet2 && strcmp( ship->starsystem->planet2, "" ) &&
				abs( ship->vx - ship->starsystem->p2x ) < too_close &&
				abs( ship->vy - ship->starsystem->p2y ) < too_close && abs( ship->vz - ship->starsystem->p2z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet2,
					ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet3 && strcmp( ship->starsystem->planet3, "" ) &&
				abs( ship->vx - ship->starsystem->p3x ) < too_close &&
				abs( ship->vy - ship->starsystem->p3y ) < too_close && abs( ship->vz - ship->starsystem->p3z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet3,
					ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet4 && strcmp( ship->starsystem->planet4, "" ) &&
				abs( ship->vx - ship->starsystem->p4x ) < too_close &&
				abs( ship->vy - ship->starsystem->p4y ) < too_close && abs( ship->vz - ship->starsystem->p4z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet4,
					ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet5 && strcmp( ship->starsystem->planet5, "" ) &&
				abs( ship->vx - ship->starsystem->p5x ) < too_close &&
				abs( ship->vy - ship->starsystem->p5y ) < too_close && abs( ship->vz - ship->starsystem->p5z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet5,
					ship->starsystem->p5x, ship->starsystem->p5y, ship->starsystem->p5z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet6 && strcmp( ship->starsystem->planet6, "" ) &&
				abs( ship->vx - ship->starsystem->p6x ) < too_close &&
				abs( ship->vy - ship->starsystem->p6y ) < too_close && abs( ship->vz - ship->starsystem->p6z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet6,
					ship->starsystem->p6x, ship->starsystem->p6y, ship->starsystem->p6z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}

			if( ship->starsystem->planet7 && strcmp( ship->starsystem->planet7, "" ) &&
				abs( ship->vx - ship->starsystem->p7x ) < too_close &&
				abs( ship->vy - ship->starsystem->p7y ) < too_close && abs( ship->vz - ship->starsystem->p7z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet7,
					ship->starsystem->p7x, ship->starsystem->p7y, ship->starsystem->p7z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet8 && strcmp( ship->starsystem->planet8, "" ) &&
				abs( ship->vx - ship->starsystem->p8x ) < too_close &&
				abs( ship->vy - ship->starsystem->p8y ) < too_close && abs( ship->vz - ship->starsystem->p8z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet8,
					ship->starsystem->p8x, ship->starsystem->p8y, ship->starsystem->p8z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}
			if( ship->starsystem->planet9 && strcmp( ship->starsystem->planet9, "" ) &&
				abs( ship->vx - ship->starsystem->p9x ) < too_close &&
				abs( ship->vy - ship->starsystem->p9y ) < too_close && abs( ship->vz - ship->starsystem->p9z ) < too_close )
			{
				snprintf( buf, MAX_STRING_LENGTH, "Proximity alert: %s  %d %d %d", ship->starsystem->planet9,
					ship->starsystem->p9x, ship->starsystem->p9y, ship->starsystem->p9z );
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), buf );
			}

			too_close = ship->currspeed + 300;
			for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
			{
				target_too_close = too_close + target->currspeed;
				if( target != ship &&
					abs( ship->vx - target->vx ) < target_too_close &&
					abs( ship->vy - target->vy ) < target_too_close && abs( ship->vz - target->vz ) < target_too_close )
				{
					if( target->ship_class == SMINE )
					{
						if( str_cmp( target->owner, ship->owner ) )

						{
							damage_ship( ship, 100, 100 );
							destroy_ship( target, NULL );
						}
					}
				}
			}
		}


		if( ship->target0 )
		{
			if( ship->lastdoc != ship->location )
			{
				snprintf( buf, MAX_STRING_LENGTH, "%s   %.0f %.0f %.0f", ship->target0->name,
					ship->target0->vx, ship->target0->vy, ship->target0->vz );
				echo_to_room_dnr( AT_BLUE, get_room_index( ship->gunseat ), "Target: " );
				echo_to_room( AT_LBLUE, get_room_index( ship->gunseat ), buf );
				if( ship->starsystem != ship->target0->starsystem )
					ship->target0 = NULL;
			}
		}

		if( ship->target1 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s   %.0f %.0f %.0f", ship->target1->name, ship->target1->vx, ship->target1->vy, ship->target1->vz );
			echo_to_room_dnr( AT_BLUE, get_room_index( ship->turret1 ), "Target: " );
			echo_to_room( AT_LBLUE, get_room_index( ship->turret1 ), buf );
			if( ship->starsystem != ship->target1->starsystem )
				ship->target1 = NULL;
		}

		if( ship->target2 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s   %.0f %.0f %.0f", ship->target2->name, ship->target2->vx, ship->target2->vy, ship->target2->vz );
			echo_to_room_dnr( AT_BLUE, get_room_index( ship->turret2 ), "Target: " );
			echo_to_room( AT_LBLUE, get_room_index( ship->turret2 ), buf );
			if( ship->starsystem != ship->target2->starsystem )
				ship->target2 = NULL;
		}

		if( ship->energy < 100 && ship->starsystem )
		{
			echo_to_cockpit( AT_RED, ship, "Warning: Energy Diminished, seek HELP!" );
		}

		ship->energy = URANGE( 0, ship->energy, ship->maxenergy );
	}

	for( ship = first_ship; ship; ship = ship->next )
	{

		if( ship->autotrack && ship->target0 && ship->ship_class < 3 )
		{
			target = ship->target0;
			too_close = ship->currspeed + 10;
			target_too_close = too_close + target->currspeed;
			if( target != ship && ship->shipstate == SHIP_READY &&
				abs( ship->vx - target->vx ) < target_too_close &&
				abs( ship->vy - target->vy ) < target_too_close && abs( ship->vz - target->vz ) < target_too_close )
			{
				ship->hx = 0 - ( ship->target0->vx - ship->vx );
				ship->hy = 0 - ( ship->target0->vy - ship->vy );
				ship->hz = 0 - ( ship->target0->vz - ship->vz );
				ship->energy -= ship->currspeed / 10;
				echo_to_room( AT_RED, get_room_index( ship->pilotseat ), "Autotrack: Evading to avoid collision!\r\n" );
				if( ship->ship_class == MOBILE_SUIT || ( ship->ship_class == TRANSPORT_SHIP && ship->manuever > 50 ) )
					ship->shipstate = SHIP_BUSY_3;
				else if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP && ship->manuever > 50 ) )
					ship->shipstate = SHIP_BUSY_2;
				else
					ship->shipstate = SHIP_BUSY;
			}
			else if( !is_facing( ship, ship->target0 ) )
			{
				ship->hx = ship->target0->vx - ship->vx;
				ship->hy = ship->target0->vy - ship->vy;
				ship->hz = ship->target0->vz - ship->vz;
				ship->energy -= ship->currspeed / 10;
				echo_to_room( AT_BLUE, get_room_index( ship->pilotseat ), "Autotracking target ... setting new course.\r\n" );
				if( ship->ship_class == MOBILE_SUIT || ( ship->ship_class == TRANSPORT_SHIP && ship->manuever > 50 ) )
					ship->shipstate = SHIP_BUSY_3;
				else if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP && ship->manuever > 50 ) )
					ship->shipstate = SHIP_BUSY_2;
				else
					ship->shipstate = SHIP_BUSY;
			}
		}

		if( autofly( ship ) )
		{
			if( ship->starsystem )
			{
				if( ship->target0 )
				{
					int chance = 50;

					/*
					 * auto assist ships
					 */

					for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
					{
						if( autofly( target ) )
							if( !str_cmp( target->owner, ship->owner ) && target != ship )
								if( target->target0 == NULL && ship->target0 != target )
								{
									target->target0 = ship->target0;
									snprintf( buf, MAX_STRING_LENGTH, "Your suit is being targetted by %s.", target->name );
									echo_to_cockpit( AT_BLOOD, target->target0, buf );
									break;
								}
					}

					target = ship->target0;
					ship->autotrack = true;
					if( ship->ship_class != SHIP_PLATFORM )
						ship->currspeed = ship->realspeed;
					if( ship->energy > 200 )
						ship->autorecharge = true;


					if( ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
						&& ship->missilestate == MISSILE_READY && ship->target0->starsystem == ship->starsystem
						&& abs( target->vx - ship->vx ) <= 1200
						&& abs( target->vy - ship->vy ) <= 1200 && abs( target->vz - ship->vz ) <= 1200 && ship->missiles > 0 )
					{
						if( ship->ship_class > 1 || is_facing( ship, target ) )
						{
							chance -= target->manuever / 5;
							chance -= target->currspeed / 20;
							chance += target->ship_class * target->ship_class * 25;
							chance -= ( abs( target->vx - ship->vx ) / 100 );
							chance -= ( abs( target->vy - ship->vy ) / 100 );
							chance -= ( abs( target->vz - ship->vz ) / 100 );
							chance += ( 30 );
							chance = URANGE( 10, chance, 90 );

							if( number_percent( ) > chance )
							{
							}
							else
							{
								new_missile( ship, target, NULL, CONCUSSION_MISSILE );
								ship->missiles--;
								snprintf( buf, MAX_STRING_LENGTH, "%s fires its Beam Cannon at you.", ship->name );
								echo_to_cockpit( AT_BLOOD, target, buf );
								snprintf( buf, MAX_STRING_LENGTH, "%s fires its Beam Cannon at %s.", ship->name, target->name );
								echo_to_system( AT_ORANGE, target, buf, NULL );

								if( ship->ship_class == CAPITAL_SHIP || ship->ship_class == SHIP_PLATFORM )
									ship->missilestate = MISSILE_RELOAD_2;
								else
									ship->missilestate = MISSILE_FIRED;
							}
						}
					}

					if( ship->missilestate == MISSILE_DAMAGED )
						ship->missilestate = MISSILE_READY;
					if( ship->statet0 == LASER_DAMAGED )
						ship->statet0 = LASER_READY;
					if( ship->shipstate == SHIP_DISABLED )
						ship->shipstate = SHIP_READY;

				}
				else
				{
					ship->currspeed = 0;

					if( !str_cmp( ship->owner, "The Empire" ) )
						for( target = first_ship; target; target = target->next )
							if( ship->starsystem == target->starsystem )
								if( !str_cmp( target->owner, "The New Republic" ) )
								{
									ship->target0 = target;
									snprintf( buf, MAX_STRING_LENGTH, "Your suit is being targetted by %s.", ship->name );
									echo_to_cockpit( AT_BLOOD, target, buf );
									break;
								}
					if( !str_cmp( ship->owner, "The New Republic" ) )
						for( target = first_ship; target; target = target->next )
							if( ship->starsystem == target->starsystem )
								if( !str_cmp( target->owner, "The Empire" ) )
								{
									snprintf( buf, MAX_STRING_LENGTH, "Your suit is being targetted by %s.", ship->name );
									echo_to_cockpit( AT_BLOOD, target, buf );
									ship->target0 = target;
									break;
								}

					if( !str_cmp( ship->owner, "Pirates" ) )
						for( target = first_ship; target; target = target->next )
							if( ship->starsystem == target->starsystem )
							{
								snprintf( buf, MAX_STRING_LENGTH, "Your suit is being targetted by %s.", ship->name );
								echo_to_cockpit( AT_BLOOD, target, buf );
								ship->target0 = target;
								break;
							}

				}
			}
			else
			{
				if( number_range( 1, 25 ) == 25 )
				{
					ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
					ship->vx = number_range( -5000, 5000 );
					ship->vy = number_range( -5000, 5000 );
					ship->vz = number_range( -5000, 5000 );
					ship->hx = 1;
					ship->hy = 1;
					ship->hz = 1;
				}
			}
		}

		if( ( ship->ship_class == CAPITAL_SHIP || ship->ship_class == SHIP_PLATFORM ) && ship->target0 == NULL )
		{
			if( ship->missiles < ship->maxmissiles )
				ship->missiles++;
			if( ship->torpedos < ship->maxtorpedos )
				ship->torpedos++;
			if( ship->rockets < ship->maxrockets )
				ship->rockets++;
		}
	}
}



void write_starsystem_list( )
{
	SPACE_DATA *tstarsystem;
	FILE *fpout;
	char filename[256];

	snprintf( filename, sizeof(filename), "%s%s", SPACE_DIR, SPACE_LIST );
	fpout = FileOpen( filename, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open starsystem.lst for writing!\r\n", 0 );
		return;
	}
	for( tstarsystem = first_starsystem; tstarsystem; tstarsystem = tstarsystem->next )
		fprintf( fpout, "%s\n", tstarsystem->filename );
	fprintf( fpout, "$\n" );
	FileClose( fpout );
}


/*
 * Get pointer to space structure from starsystem name.
 */
SPACE_DATA *starsystem_from_name( const char *name )
{
	SPACE_DATA *starsystem;

	for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
		if( !str_cmp( name, starsystem->name ) )
			return starsystem;

	for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
		if( !str_prefix( name, starsystem->name ) )
			return starsystem;

	return NULL;
}

/*
 * Get pointer to space structure from the dock vnun.
 */
SPACE_DATA *starsystem_from_vnum( int vnum )
{
	SPACE_DATA *starsystem;
	SHIP_DATA *ship;

	for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
		if( vnum == starsystem->doc1a || vnum == starsystem->doc2a || vnum == starsystem->doc3a ||
			vnum == starsystem->doc1b || vnum == starsystem->doc2b || vnum == starsystem->doc3b ||
			vnum == starsystem->doc1c || vnum == starsystem->doc2c || vnum == starsystem->doc3c ||
			vnum == starsystem->doc4a || vnum == starsystem->doc5a || vnum == starsystem->doc6a ||
			vnum == starsystem->doc4b || vnum == starsystem->doc5b || vnum == starsystem->doc6b ||
			vnum == starsystem->doc4c || vnum == starsystem->doc5c || vnum == starsystem->doc6c ||
			vnum == starsystem->doc7a || vnum == starsystem->doc8a || vnum == starsystem->doc9a ||
			vnum == starsystem->doc7b || vnum == starsystem->doc8b || vnum == starsystem->doc9b ||
			vnum == starsystem->doc7c || vnum == starsystem->doc8c || vnum == starsystem->doc9c )
			return starsystem;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->hanger )
			return ship->starsystem;

	return NULL;
}


/*
 * Save a starsystem's data to its data file
 */
void save_starsystem( SPACE_DATA *starsystem )
{
	FILE *fp;
	char filename[256];
	char buf[MAX_STRING_LENGTH];

	if( !starsystem )
	{
		bug( "save_starsystem: null starsystem pointer!", 0 );
		return;
	}

	if( !starsystem->filename || starsystem->filename[0] == '\0' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "save_starsystem: %s has no filename", starsystem->name );
		bug( buf, 0 );
		return;
	}

	snprintf( filename, sizeof(filename), "%s%s", SPACE_DIR, starsystem->filename );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_starsystem: FileOpen", 0 );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#SPACE\n" );
		fprintf( fp, "Name         %s~\n", starsystem->name );
		fprintf( fp, "Filename     %s~\n", starsystem->filename );
		fprintf( fp, "Planet1      %s~\n", starsystem->planet1 );
		fprintf( fp, "Planet2      %s~\n", starsystem->planet2 );
		fprintf( fp, "Planet3      %s~\n", starsystem->planet3 );
		fprintf( fp, "Planet4      %s~\n", starsystem->planet4 );
		fprintf( fp, "Planet5      %s~\n", starsystem->planet5 );
		fprintf( fp, "Planet6      %s~\n", starsystem->planet6 );
		fprintf( fp, "Planet7      %s~\n", starsystem->planet7 );
		fprintf( fp, "Planet8      %s~\n", starsystem->planet8 );
		fprintf( fp, "Planet9      %s~\n", starsystem->planet9 );
		fprintf( fp, "Star1        %s~\n", starsystem->star1 );
		fprintf( fp, "Star2        %s~\n", starsystem->star2 );
		fprintf( fp, "Location1a      %s~\n", starsystem->location1a );
		fprintf( fp, "Location1b      %s~\n", starsystem->location1b );
		fprintf( fp, "Location1c      %s~\n", starsystem->location1c );
		fprintf( fp, "Location2a       %s~\n", starsystem->location2a );
		fprintf( fp, "Location2b      %s~\n", starsystem->location2b );
		fprintf( fp, "Location2c      %s~\n", starsystem->location2c );
		fprintf( fp, "Location3a      %s~\n", starsystem->location3a );
		fprintf( fp, "Location3b      %s~\n", starsystem->location3b );
		fprintf( fp, "Location3c      %s~\n", starsystem->location3c );
		fprintf( fp, "Location4a      %s~\n", starsystem->location4a );
		fprintf( fp, "Location4b      %s~\n", starsystem->location4b );
		fprintf( fp, "Location4c      %s~\n", starsystem->location4c );
		fprintf( fp, "Location5a      %s~\n", starsystem->location5a );
		fprintf( fp, "Location5b      %s~\n", starsystem->location5b );
		fprintf( fp, "Location5c      %s~\n", starsystem->location5c );
		fprintf( fp, "Location6a      %s~\n", starsystem->location6a );
		fprintf( fp, "Location6b      %s~\n", starsystem->location6b );
		fprintf( fp, "Location6c      %s~\n", starsystem->location6c );
		fprintf( fp, "Location7a      %s~\n", starsystem->location7a );
		fprintf( fp, "Location7b      %s~\n", starsystem->location7b );
		fprintf( fp, "Location7c      %s~\n", starsystem->location7c );
		fprintf( fp, "Location8a      %s~\n", starsystem->location8a );
		fprintf( fp, "Location8b      %s~\n", starsystem->location8b );
		fprintf( fp, "Location8c      %s~\n", starsystem->location8c );
		fprintf( fp, "Location9a      %s~\n", starsystem->location9a );
		fprintf( fp, "Location9b      %s~\n", starsystem->location9b );
		fprintf( fp, "Location9c      %s~\n", starsystem->location9c );
		fprintf( fp, "Doc1a          %d\n", starsystem->doc1a );
		fprintf( fp, "Doc2a          %d\n", starsystem->doc2a );
		fprintf( fp, "Doc3a          %d\n", starsystem->doc3a );
		fprintf( fp, "Doc4a          %d\n", starsystem->doc4a );
		fprintf( fp, "Doc5a          %d\n", starsystem->doc5a );
		fprintf( fp, "Doc6a          %d\n", starsystem->doc6a );
		fprintf( fp, "Doc7a          %d\n", starsystem->doc7a );
		fprintf( fp, "Doc8a          %d\n", starsystem->doc8a );
		fprintf( fp, "Doc9a          %d\n", starsystem->doc9a );
		fprintf( fp, "Doc1b          %d\n", starsystem->doc1b );
		fprintf( fp, "Doc2b          %d\n", starsystem->doc2b );
		fprintf( fp, "Doc3b          %d\n", starsystem->doc3b );
		fprintf( fp, "Doc4b          %d\n", starsystem->doc4b );
		fprintf( fp, "Doc5b          %d\n", starsystem->doc5b );
		fprintf( fp, "Doc6b          %d\n", starsystem->doc6b );
		fprintf( fp, "Doc7b          %d\n", starsystem->doc7b );
		fprintf( fp, "Doc8b          %d\n", starsystem->doc8b );
		fprintf( fp, "Doc9b          %d\n", starsystem->doc9b );
		fprintf( fp, "Doc1c          %d\n", starsystem->doc1c );
		fprintf( fp, "Doc2c          %d\n", starsystem->doc2c );
		fprintf( fp, "Doc3c          %d\n", starsystem->doc3c );
		fprintf( fp, "Doc4c          %d\n", starsystem->doc4c );
		fprintf( fp, "Doc5c          %d\n", starsystem->doc5c );
		fprintf( fp, "Doc6c          %d\n", starsystem->doc6c );
		fprintf( fp, "Doc7c          %d\n", starsystem->doc7c );
		fprintf( fp, "Doc8c          %d\n", starsystem->doc8c );
		fprintf( fp, "Doc9c          %d\n", starsystem->doc9c );
		fprintf( fp, "P1x          %d\n", starsystem->p1x );
		fprintf( fp, "P1y          %d\n", starsystem->p1y );
		fprintf( fp, "P1z          %d\n", starsystem->p1z );
		fprintf( fp, "P2x          %d\n", starsystem->p2x );
		fprintf( fp, "P2y          %d\n", starsystem->p2y );
		fprintf( fp, "P2z          %d\n", starsystem->p2z );
		fprintf( fp, "P3x          %d\n", starsystem->p3x );
		fprintf( fp, "P3y          %d\n", starsystem->p3y );
		fprintf( fp, "P3z          %d\n", starsystem->p3z );
		fprintf( fp, "P4x          %d\n", starsystem->p4x );
		fprintf( fp, "P4y          %d\n", starsystem->p4y );
		fprintf( fp, "P4z          %d\n", starsystem->p4z );
		fprintf( fp, "P5x          %d\n", starsystem->p5x );
		fprintf( fp, "P5y          %d\n", starsystem->p5y );
		fprintf( fp, "P5z          %d\n", starsystem->p5z );
		fprintf( fp, "P6x          %d\n", starsystem->p6x );
		fprintf( fp, "P6y          %d\n", starsystem->p6y );
		fprintf( fp, "P6z          %d\n", starsystem->p6z );
		fprintf( fp, "P7x          %d\n", starsystem->p7x );
		fprintf( fp, "P7y          %d\n", starsystem->p7y );
		fprintf( fp, "P7z          %d\n", starsystem->p7z );
		fprintf( fp, "P8x          %d\n", starsystem->p8x );
		fprintf( fp, "P8y          %d\n", starsystem->p8y );
		fprintf( fp, "P8z          %d\n", starsystem->p8z );
		fprintf( fp, "P9x          %d\n", starsystem->p9x );
		fprintf( fp, "P9y          %d\n", starsystem->p9y );
		fprintf( fp, "P9z          %d\n", starsystem->p9z );
		fprintf( fp, "S1x          %d\n", starsystem->s1x );
		fprintf( fp, "S1y          %d\n", starsystem->s1y );
		fprintf( fp, "S1z          %d\n", starsystem->s1z );
		fprintf( fp, "S2x          %d\n", starsystem->s2x );
		fprintf( fp, "S2y          %d\n", starsystem->s2y );
		fprintf( fp, "S2z          %d\n", starsystem->s2z );
		fprintf( fp, "Gravitys1     %d\n", starsystem->gravitys1 );
		fprintf( fp, "Gravitys2     %d\n", starsystem->gravitys2 );
		fprintf( fp, "Gravityp1     %d\n", starsystem->gravityp1 );
		fprintf( fp, "Gravityp2     %d\n", starsystem->gravityp2 );
		fprintf( fp, "Gravityp3     %d\n", starsystem->gravityp3 );
		fprintf( fp, "Xpos          %d\n", starsystem->xpos );
		fprintf( fp, "Ypos          %d\n", starsystem->ypos );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
	}
	FileClose( fp );
	return;
}


/*
 * Read in actual starsystem data.
 */

void fread_starsystem( SPACE_DATA *starsystem, FILE *fp )
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

		case 'D':
			KEY( "Doc1a", starsystem->doc1a, fread_number( fp ) );
			KEY( "Doc2a", starsystem->doc2a, fread_number( fp ) );
			KEY( "Doc3a", starsystem->doc3a, fread_number( fp ) );
			KEY( "Doc4a", starsystem->doc4a, fread_number( fp ) );
			KEY( "Doc5a", starsystem->doc5a, fread_number( fp ) );
			KEY( "Doc6a", starsystem->doc6a, fread_number( fp ) );
			KEY( "Doc7a", starsystem->doc7a, fread_number( fp ) );
			KEY( "Doc8a", starsystem->doc8a, fread_number( fp ) );
			KEY( "Doc9a", starsystem->doc9a, fread_number( fp ) );
			KEY( "Doc1b", starsystem->doc1b, fread_number( fp ) );
			KEY( "Doc2b", starsystem->doc2b, fread_number( fp ) );
			KEY( "Doc3b", starsystem->doc3b, fread_number( fp ) );
			KEY( "Doc4b", starsystem->doc4b, fread_number( fp ) );
			KEY( "Doc5b", starsystem->doc5b, fread_number( fp ) );
			KEY( "Doc6b", starsystem->doc6b, fread_number( fp ) );
			KEY( "Doc7b", starsystem->doc7b, fread_number( fp ) );
			KEY( "Doc8b", starsystem->doc8b, fread_number( fp ) );
			KEY( "Doc9b", starsystem->doc9b, fread_number( fp ) );
			KEY( "Doc1c", starsystem->doc1c, fread_number( fp ) );
			KEY( "Doc2c", starsystem->doc2c, fread_number( fp ) );
			KEY( "Doc3c", starsystem->doc3c, fread_number( fp ) );
			KEY( "Doc4c", starsystem->doc4c, fread_number( fp ) );
			KEY( "Doc5c", starsystem->doc5c, fread_number( fp ) );
			KEY( "Doc6c", starsystem->doc6c, fread_number( fp ) );
			KEY( "Doc7c", starsystem->doc7c, fread_number( fp ) );
			KEY( "Doc8c", starsystem->doc8c, fread_number( fp ) );
			KEY( "Doc9c", starsystem->doc9c, fread_number( fp ) );


			break;


		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !starsystem->name )
					starsystem->name = STRALLOC( "" );
				if( !starsystem->location1a )
					starsystem->location1a = STRALLOC( "" );
				if( !starsystem->location2a )
					starsystem->location2a = STRALLOC( "" );
				if( !starsystem->location3a )
					starsystem->location3a = STRALLOC( "" );
				if( !starsystem->location4a )
					starsystem->location4a = STRALLOC( "" );
				if( !starsystem->location5a )
					starsystem->location5a = STRALLOC( "" );
				if( !starsystem->location6a )
					starsystem->location6a = STRALLOC( "" );
				if( !starsystem->location1b )
					starsystem->location1b = STRALLOC( "" );
				if( !starsystem->location2b )
					starsystem->location2b = STRALLOC( "" );
				if( !starsystem->location3b )
					starsystem->location3b = STRALLOC( "" );
				if( !starsystem->location4b )
					starsystem->location4b = STRALLOC( "" );
				if( !starsystem->location5b )
					starsystem->location5b = STRALLOC( "" );
				if( !starsystem->location6b )
					starsystem->location6b = STRALLOC( "" );
				if( !starsystem->location1c )
					starsystem->location1c = STRALLOC( "" );
				if( !starsystem->location2c )
					starsystem->location2c = STRALLOC( "" );
				if( !starsystem->location3c )
					starsystem->location3c = STRALLOC( "" );
				if( !starsystem->location4c )
					starsystem->location4c = STRALLOC( "" );
				if( !starsystem->location5c )
					starsystem->location5c = STRALLOC( "" );
				if( !starsystem->location6c )
					starsystem->location6c = STRALLOC( "" );
				if( !starsystem->location7c )
					starsystem->location7c = STRALLOC( "" );
				if( !starsystem->location8c )
					starsystem->location8c = STRALLOC( "" );
				if( !starsystem->location9c )
					starsystem->location9c = STRALLOC( "" );
				if( !starsystem->planet1 )
					starsystem->planet1 = STRALLOC( "" );
				if( !starsystem->planet2 )
					starsystem->planet2 = STRALLOC( "" );
				if( !starsystem->planet3 )
					starsystem->planet3 = STRALLOC( "" );
				if( !starsystem->planet4 )
					starsystem->planet4 = STRALLOC( "" );
				if( !starsystem->planet5 )
					starsystem->planet5 = STRALLOC( "" );
				if( !starsystem->planet6 )
					starsystem->planet6 = STRALLOC( "" );
				if( !starsystem->planet7 )
					starsystem->planet7 = STRALLOC( "" );
				if( !starsystem->planet8 )
					starsystem->planet8 = STRALLOC( "" );
				if( !starsystem->planet9 )
					starsystem->planet9 = STRALLOC( "" );
				if( !starsystem->star1 )
					starsystem->star1 = STRALLOC( "" );
				if( !starsystem->star2 )
					starsystem->star2 = STRALLOC( "" );
				return;
			}
			break;

		case 'F':
			KEY( "Filename", starsystem->filename, fread_string_nohash( fp ) );
			break;

		case 'G':
			KEY( "Gravitys1", starsystem->gravitys1, fread_number( fp ) );
			KEY( "Gravitys2", starsystem->gravitys2, fread_number( fp ) );
			KEY( "Gravityp1", starsystem->gravityp1, fread_number( fp ) );
			KEY( "Gravityp2", starsystem->gravityp2, fread_number( fp ) );
			KEY( "Gravityp3", starsystem->gravityp3, fread_number( fp ) );
			break;

		case 'L':
			KEY( "Location1a", starsystem->location1a, fread_string( fp ) );
			KEY( "Location2a", starsystem->location2a, fread_string( fp ) );
			KEY( "Location3a", starsystem->location3a, fread_string( fp ) );
			KEY( "Location4a", starsystem->location4a, fread_string( fp ) );
			KEY( "Location5a", starsystem->location5a, fread_string( fp ) );
			KEY( "Location6a", starsystem->location6a, fread_string( fp ) );
			KEY( "Location7a", starsystem->location7a, fread_string( fp ) );
			KEY( "Location8a", starsystem->location8a, fread_string( fp ) );
			KEY( "Location9a", starsystem->location9a, fread_string( fp ) );
			KEY( "Location1b", starsystem->location1b, fread_string( fp ) );
			KEY( "Location2b", starsystem->location2b, fread_string( fp ) );
			KEY( "Location3b", starsystem->location3b, fread_string( fp ) );
			KEY( "Location4b", starsystem->location4b, fread_string( fp ) );
			KEY( "Location5b", starsystem->location5b, fread_string( fp ) );
			KEY( "Location6b", starsystem->location6b, fread_string( fp ) );
			KEY( "Location7b", starsystem->location7b, fread_string( fp ) );
			KEY( "Location8b", starsystem->location8b, fread_string( fp ) );
			KEY( "Location9b", starsystem->location9b, fread_string( fp ) );
			KEY( "Location1c", starsystem->location1c, fread_string( fp ) );
			KEY( "Location2c", starsystem->location2c, fread_string( fp ) );
			KEY( "Location3c", starsystem->location3c, fread_string( fp ) );
			KEY( "Location4c", starsystem->location4c, fread_string( fp ) );
			KEY( "Location5c", starsystem->location5c, fread_string( fp ) );
			KEY( "Location6c", starsystem->location6c, fread_string( fp ) );
			KEY( "Location7c", starsystem->location7c, fread_string( fp ) );
			KEY( "Location8c", starsystem->location8c, fread_string( fp ) );
			KEY( "Location9c", starsystem->location9c, fread_string( fp ) );
			break;

		case 'N':
			KEY( "Name", starsystem->name, fread_string( fp ) );
			break;

		case 'P':
			KEY( "Planet1", starsystem->planet1, fread_string( fp ) );
			KEY( "Planet2", starsystem->planet2, fread_string( fp ) );
			KEY( "Planet3", starsystem->planet3, fread_string( fp ) );
			KEY( "Planet4", starsystem->planet4, fread_string( fp ) );
			KEY( "Planet5", starsystem->planet5, fread_string( fp ) );
			KEY( "Planet6", starsystem->planet6, fread_string( fp ) );
			KEY( "Planet7", starsystem->planet7, fread_string( fp ) );
			KEY( "Planet8", starsystem->planet8, fread_string( fp ) );
			KEY( "Planet9", starsystem->planet9, fread_string( fp ) );
			KEY( "P1x", starsystem->p1x, fread_number( fp ) );
			KEY( "P1y", starsystem->p1y, fread_number( fp ) );
			KEY( "P1z", starsystem->p1z, fread_number( fp ) );
			KEY( "P2x", starsystem->p2x, fread_number( fp ) );
			KEY( "P2y", starsystem->p2y, fread_number( fp ) );
			KEY( "P2z", starsystem->p2z, fread_number( fp ) );
			KEY( "P3x", starsystem->p3x, fread_number( fp ) );
			KEY( "P3y", starsystem->p3y, fread_number( fp ) );
			KEY( "P3z", starsystem->p3z, fread_number( fp ) );
			KEY( "P4x", starsystem->p4x, fread_number( fp ) );
			KEY( "P4y", starsystem->p4y, fread_number( fp ) );
			KEY( "P4z", starsystem->p4z, fread_number( fp ) );
			KEY( "P5x", starsystem->p5x, fread_number( fp ) );
			KEY( "P5y", starsystem->p5y, fread_number( fp ) );
			KEY( "P5z", starsystem->p5z, fread_number( fp ) );
			KEY( "P6x", starsystem->p6x, fread_number( fp ) );
			KEY( "P6y", starsystem->p6y, fread_number( fp ) );
			KEY( "P6z", starsystem->p6z, fread_number( fp ) );
			KEY( "P7x", starsystem->p7x, fread_number( fp ) );
			KEY( "P7y", starsystem->p7y, fread_number( fp ) );
			KEY( "P7z", starsystem->p7z, fread_number( fp ) );
			KEY( "P8x", starsystem->p8x, fread_number( fp ) );
			KEY( "P8y", starsystem->p8y, fread_number( fp ) );
			KEY( "P8z", starsystem->p8z, fread_number( fp ) );
			KEY( "P9x", starsystem->p9x, fread_number( fp ) );
			KEY( "P9y", starsystem->p9y, fread_number( fp ) );
			KEY( "P9z", starsystem->p9z, fread_number( fp ) );

			break;

		case 'S':
			KEY( "Star1", starsystem->star1, fread_string( fp ) );
			KEY( "Star2", starsystem->star2, fread_string( fp ) );
			KEY( "S1x", starsystem->s1x, fread_number( fp ) );
			KEY( "S1y", starsystem->s1y, fread_number( fp ) );
			KEY( "S1z", starsystem->s1z, fread_number( fp ) );
			KEY( "S2x", starsystem->s2x, fread_number( fp ) );
			KEY( "S2y", starsystem->s2y, fread_number( fp ) );
			KEY( "S2z", starsystem->s2z, fread_number( fp ) );

		case 'X':
			KEY( "Xpos", starsystem->xpos, fread_number( fp ) );

		case 'Y':
			KEY( "Ypos", starsystem->ypos, fread_number( fp ) );

		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_starsystem: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

/*
 * Load a starsystem file
 */

bool load_starsystem( const char *starsystemfile )
{
	char filename[256];
	SPACE_DATA *starsystem;
	FILE *fp;
	bool found;

	CREATE( starsystem, SPACE_DATA, 1 );

	found = false;
	snprintf( filename, sizeof(filename), "%s%s", SPACE_DIR, starsystemfile );

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{

		found = true;
		LINK( starsystem, first_starsystem, last_starsystem, next, prev );
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
				bug( "Load_starsystem_file: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "SPACE" ) )
			{
				fread_starsystem( starsystem, fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				char buf[MAX_STRING_LENGTH];

				snprintf( buf, MAX_STRING_LENGTH, "Load_starsystem_file: bad section: %s.", word );
				bug( buf, 0 );
				break;
			}
		}
		FileClose( fp );
	}

	if( !( found ) )
		DISPOSE( starsystem );

	return found;
}

/*
 * Load in all the starsystem files.
 */
void load_space( )
{
	FILE *fpList;
	const char *filename;
	char starsystemlist[256];
	char buf[MAX_STRING_LENGTH];


	first_starsystem = NULL;
	last_starsystem = NULL;

	log_string( "Loading space..." );

	snprintf( starsystemlist, sizeof(starsystemlist), "%s%s", SPACE_DIR, SPACE_LIST );
	if( ( fpList = FileOpen( starsystemlist, "r" ) ) == NULL )
	{
		perror( starsystemlist );
		exit( 108 );
	}

	for( ;; )
	{
		filename = feof( fpList ) ? "$" : fread_word( fpList );
		if( filename[0] == '$' )
			break;


		if( !load_starsystem( filename ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Cannot load starsystem file: %s", filename );
			bug( buf, 0 );
		}
	}
	FileClose( fpList );
	log_string( " Done starsystems " );
	return;
}

CMDF( do_setstarsystem )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SPACE_DATA *starsystem;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg2[0] == '\0' || arg1[0] == '\0' )
	{
		send_to_char( "Usage: setstarsystem <starsystem> <field> <values>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( "name filename xpos ypos,\r\n", ch );
		send_to_char( "star1 s1x s1y s1z gravitys1\r\n", ch );
		send_to_char( "star2 s2x s2y s2z gravitys2\r\n", ch );
		send_to_char( "planet1 p1x p1y p1z gravityp1\r\n", ch );
		send_to_char( "planet2 p2x p2y p2z gravityp2\r\n", ch );
		send_to_char( "planet3 p3x p3y p3z gravityp3\r\n", ch );
		send_to_char( "location1a location1b location1c doc1a doc1b doc1c\r\n", ch );
		send_to_char( "location2a location2b location2c doc2a doc2b doc2c\r\n", ch );
		send_to_char( "location3a location3b location3c doc3a doc3b doc3c\r\n", ch );
		send_to_char( "Planet Max Now Increased To 9\r\n", ch );
		send_to_char( "", ch );
		return;
	}

	starsystem = starsystem_from_name( arg1 );
	if( !starsystem )
	{
		send_to_char( "No such starsystem.\r\n", ch );
		return;
	}


	if( !str_cmp( arg2, "doc1a" ) )
	{
		starsystem->doc1a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc1b" ) )
	{
		starsystem->doc1b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc1c" ) )
	{
		starsystem->doc1c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc2a" ) )
	{
		starsystem->doc2a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc2b" ) )
	{
		starsystem->doc2b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc2c" ) )
	{
		starsystem->doc2c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc3a" ) )
	{
		starsystem->doc3a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc3b" ) )
	{
		starsystem->doc3b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc3c" ) )
	{
		starsystem->doc3c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc4a" ) )
	{
		starsystem->doc4a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc4b" ) )
	{
		starsystem->doc4b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc4c" ) )
	{
		starsystem->doc4c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc5a" ) )
	{
		starsystem->doc5a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc5b" ) )
	{
		starsystem->doc5b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc5c" ) )
	{
		starsystem->doc5c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc6a" ) )
	{
		starsystem->doc6a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc6b" ) )
	{
		starsystem->doc6b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc6c" ) )
	{
		starsystem->doc6c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc7a" ) )
	{
		starsystem->doc7a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc7b" ) )
	{
		starsystem->doc7b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc7c" ) )
	{
		starsystem->doc7c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc8a" ) )
	{
		starsystem->doc8a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc8b" ) )
	{
		starsystem->doc8b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc8c" ) )
	{
		starsystem->doc8c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "doc9a" ) )
	{
		starsystem->doc9a = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc9b" ) )
	{
		starsystem->doc9b = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "doc9c" ) )
	{
		starsystem->doc9c = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "s1x" ) )
	{
		starsystem->s1x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "s1y" ) )
	{
		starsystem->s1y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "s1z" ) )
	{
		starsystem->s1z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "s2x" ) )
	{
		starsystem->s2x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "s2y" ) )
	{
		starsystem->s2y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "s2z" ) )
	{
		starsystem->s2z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p1x" ) )
	{
		starsystem->p1x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p1y" ) )
	{
		starsystem->p1y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p1z" ) )
	{
		starsystem->p1z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p2x" ) )
	{
		starsystem->p2x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p2y" ) )
	{
		starsystem->p2y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p2z" ) )
	{
		starsystem->p2z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p3x" ) )
	{
		starsystem->p3x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p3y" ) )
	{
		starsystem->p3y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p3z" ) )
	{
		starsystem->p3z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "xpos" ) )
	{
		starsystem->xpos = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p4x" ) )
	{
		starsystem->p4x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p4y" ) )
	{
		starsystem->p4y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p4z" ) )
	{
		starsystem->p4z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p5x" ) )
	{
		starsystem->p5x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p5y" ) )
	{
		starsystem->p5y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p5z" ) )
	{
		starsystem->p5z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p6x" ) )
	{
		starsystem->p6x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p6y" ) )
	{
		starsystem->p6y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p6z" ) )
	{
		starsystem->p6z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "p7x" ) )
	{
		starsystem->p7x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p7y" ) )
	{
		starsystem->p7y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p7z" ) )
	{
		starsystem->p7z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p8x" ) )
	{
		starsystem->p8x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p8y" ) )
	{
		starsystem->p8y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p8z" ) )
	{
		starsystem->p8z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p9x" ) )
	{
		starsystem->p9x = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p9y" ) )
	{
		starsystem->p9y = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "p9z" ) )
	{
		starsystem->p9z = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}


	if( !str_cmp( arg2, "ypos" ) )
	{
		starsystem->ypos = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "gravitys1" ) )
	{
		starsystem->gravitys1 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "gravitys2" ) )
	{
		starsystem->gravitys2 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "gravityp1" ) )
	{
		starsystem->gravityp1 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "gravityp2" ) )
	{
		starsystem->gravityp2 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "gravityp3" ) )
	{
		starsystem->gravityp3 = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		STRFREE( starsystem->name );
		starsystem->name = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "star1" ) )
	{
		STRFREE( starsystem->star1 );
		starsystem->star1 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "star2" ) )
	{
		STRFREE( starsystem->star2 );
		starsystem->star2 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet1" ) )
	{
		STRFREE( starsystem->planet1 );
		starsystem->planet1 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet2" ) )
	{
		STRFREE( starsystem->planet2 );
		starsystem->planet2 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet3" ) )
	{
		STRFREE( starsystem->planet3 );
		starsystem->planet3 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet4" ) )
	{
		STRFREE( starsystem->planet4 );
		starsystem->planet4 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet5" ) )
	{
		STRFREE( starsystem->planet5 );
		starsystem->planet5 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet6" ) )
	{
		STRFREE( starsystem->planet6 );
		starsystem->planet6 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet7" ) )
	{
		STRFREE( starsystem->planet7 );
		starsystem->planet7 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet8" ) )
	{
		STRFREE( starsystem->planet8 );
		starsystem->planet8 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "planet9" ) )
	{
		STRFREE( starsystem->planet9 );
		starsystem->planet9 = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}



	if( !str_cmp( arg2, "location1a" ) )
	{
		STRFREE( starsystem->location1a );
		starsystem->location1a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location1b" ) )
	{
		STRFREE( starsystem->location1b );
		starsystem->location1b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location1c" ) )
	{
		STRFREE( starsystem->location1c );
		starsystem->location1c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "location2a" ) )
	{
		STRFREE( starsystem->location2a );
		starsystem->location2a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location2b" ) )
	{
		STRFREE( starsystem->location2a );
		starsystem->location2b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location2c" ) )
	{
		STRFREE( starsystem->location2c );
		starsystem->location2c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "location3a" ) )
	{
		STRFREE( starsystem->location3a );
		starsystem->location3a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location3b" ) )
	{
		STRFREE( starsystem->location3b );
		starsystem->location3b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location3c" ) )
	{
		STRFREE( starsystem->location3c );
		starsystem->location3c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "location4a" ) )
	{
		STRFREE( starsystem->location4a );
		starsystem->location4a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location4b" ) )
	{
		STRFREE( starsystem->location4b );
		starsystem->location4b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location4c" ) )
	{
		STRFREE( starsystem->location4c );
		starsystem->location4c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "location5a" ) )
	{
		STRFREE( starsystem->location5a );
		starsystem->location5a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location5b" ) )
	{
		STRFREE( starsystem->location5b );
		starsystem->location5b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location5c" ) )
	{
		STRFREE( starsystem->location5c );
		starsystem->location5c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}

	if( !str_cmp( arg2, "location6a" ) )
	{
		STRFREE( starsystem->location6a );
		starsystem->location6a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location6b" ) )
	{
		STRFREE( starsystem->location6b );
		starsystem->location6b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location6c" ) )
	{
		STRFREE( starsystem->location6c );
		starsystem->location6c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location7a" ) )
	{
		STRFREE( starsystem->location7a );
		starsystem->location7a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location7b" ) )
	{
		STRFREE( starsystem->location7b );
		starsystem->location7b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location7c" ) )
	{
		STRFREE( starsystem->location7c );
		starsystem->location7c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location8a" ) )
	{
		STRFREE( starsystem->location8a );
		starsystem->location8a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location8b" ) )
	{
		STRFREE( starsystem->location8b );
		starsystem->location8b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location8c" ) )
	{
		STRFREE( starsystem->location8c );
		starsystem->location8c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location9a" ) )
	{
		STRFREE( starsystem->location9a );
		starsystem->location9a = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location9b" ) )
	{
		STRFREE( starsystem->location9b );
		starsystem->location9b = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}
	if( !str_cmp( arg2, "location9c" ) )
	{
		STRFREE( starsystem->location9c );
		starsystem->location9c = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_starsystem( starsystem );
		return;
	}


	do_setstarsystem( ch, "" );
	return;
}

void showstarsystem( CHAR_DATA *ch, SPACE_DATA *starsystem )
{
	ch_printf( ch, "Starsystem:%s     Filename: %s    Xpos: %d   Ypos: %d\r\n",
		starsystem->name, starsystem->filename, starsystem->xpos, starsystem->ypos );
	ch_printf( ch, "Star1: %s   Gravity: %d   Coordinates: %d %d %d\r\n",
		starsystem->star1, starsystem->gravitys1, starsystem->s1x, starsystem->s1y, starsystem->s1z );
	ch_printf( ch, "Star2: %s   Gravity: %d   Coordinates: %d %d %d\r\n",
		starsystem->star2, starsystem->gravitys2, starsystem->s2x, starsystem->s2y, starsystem->s2z );
	ch_printf( ch, "Planet1: %s   Gravity: %d   Coordinates: %d %d %d\r\n",
		starsystem->planet1, starsystem->gravityp1, starsystem->p1x, starsystem->p1y, starsystem->p1z );
	ch_printf( ch, "     Doc1a: %5d (%s)\r\n", starsystem->doc1a, starsystem->location1a );
	ch_printf( ch, "     Doc1b: %5d (%s)\r\n", starsystem->doc1b, starsystem->location1b );
	ch_printf( ch, "     Doc1c: %5d (%s)\r\n", starsystem->doc1c, starsystem->location1c );
	ch_printf( ch, "Planet2: %s   Gravity: %d   Coordinates: %d %d %d\r\n",
		starsystem->planet2, starsystem->gravityp2, starsystem->p2x, starsystem->p2y, starsystem->p2z );
	ch_printf( ch, "     Doc2a: %5d (%s)\r\n", starsystem->doc2a, starsystem->location2a );
	ch_printf( ch, "     Doc2b: %5d (%s)\r\n", starsystem->doc2b, starsystem->location2b );
	ch_printf( ch, "     Doc2c: %5d (%s)\r\n", starsystem->doc2c, starsystem->location2c );
	ch_printf( ch, "Planet3: %s   Gravity: %d   Coordinates: %d %d %d\r\n",
		starsystem->planet3, starsystem->gravityp3, starsystem->p3x, starsystem->p3y, starsystem->p3z );
	ch_printf( ch, "     Doc3a: %5d (%s)\r\n", starsystem->doc3a, starsystem->location3a );
	ch_printf( ch, "     Doc3b: %5d (%s)\r\n", starsystem->doc3b, starsystem->location3b );
	ch_printf( ch, "     Doc3c: %5d (%s)\r\n", starsystem->doc3c, starsystem->location3c );

	ch_printf( ch, "Planet4: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet4, starsystem->p4x, starsystem->p4y, starsystem->p4z );
	ch_printf( ch, "     Doc4a: %5d (%s)\r\n", starsystem->doc4a, starsystem->location4a );
	ch_printf( ch, "     Doc4b: %5d (%s)\r\n", starsystem->doc4b, starsystem->location4b );
	ch_printf( ch, "     Doc4c: %5d (%s)\r\n", starsystem->doc4c, starsystem->location4c );

	ch_printf( ch, "Planet5: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet5, starsystem->p5x, starsystem->p5y, starsystem->p5z );
	ch_printf( ch, "     Doc5a: %5d (%s)\r\n", starsystem->doc5a, starsystem->location5a );
	ch_printf( ch, "     Doc5b: %5d (%s)\r\n", starsystem->doc5b, starsystem->location5b );
	ch_printf( ch, "     Doc5c: %5d (%s)\r\n", starsystem->doc5c, starsystem->location5c );

	ch_printf( ch, "Planet6: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet6, starsystem->p6x, starsystem->p6y, starsystem->p6z );
	ch_printf( ch, "     Doc6a: %5d (%s)\r\n", starsystem->doc6a, starsystem->location6a );
	ch_printf( ch, "     Doc6b: %5d (%s)\r\n", starsystem->doc6b, starsystem->location6b );
	ch_printf( ch, "     Doc6c: %5d (%s)\r\n", starsystem->doc6c, starsystem->location6c );
	ch_printf( ch, "Planet7: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet7, starsystem->p7x, starsystem->p7y, starsystem->p7z );
	ch_printf( ch, "     Doc7a: %5d (%s)\r\n", starsystem->doc7a, starsystem->location7a );
	ch_printf( ch, "     Doc7b: %5d (%s)\r\n", starsystem->doc7b, starsystem->location7b );
	ch_printf( ch, "     Doc7c: %5d (%s)\r\n", starsystem->doc7c, starsystem->location7c );

	ch_printf( ch, "Planet8: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet8, starsystem->p8x, starsystem->p8y, starsystem->p8z );
	ch_printf( ch, "     Doc8a: %5d (%s)\r\n", starsystem->doc8a, starsystem->location8a );
	ch_printf( ch, "     Doc8b: %5d (%s)\r\n", starsystem->doc8b, starsystem->location8b );
	ch_printf( ch, "     Doc8c: %5d (%s)\r\n", starsystem->doc8c, starsystem->location8c );

	ch_printf( ch, "Planet9: %s    Coordinates: %d %d %d\r\n",
		starsystem->planet9, starsystem->p9x, starsystem->p9y, starsystem->p9z );
	ch_printf( ch, "     Doc9a: %5d (%s)\r\n", starsystem->doc9a, starsystem->location9a );
	ch_printf( ch, "     Doc9b: %5d (%s)\r\n", starsystem->doc9b, starsystem->location9b );
	ch_printf( ch, "     Doc9c: %5d (%s)\r\n", starsystem->doc9c, starsystem->location9c );

	return;
}

CMDF( do_showstarsystem )
{
	SPACE_DATA *starsystem;

	starsystem = starsystem_from_name( argument );

	if( starsystem == NULL )
		send_to_char( "&RNo such starsystem.\r\n", ch );
	else
		showstarsystem( ch, starsystem );

}

CMDF( do_makestarsystem )
{
	char arg[MAX_INPUT_LENGTH];
	char filename[256];
	SPACE_DATA *starsystem;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makestarsystem <starsystem name>\r\n", ch );
		return;
	}



	CREATE( starsystem, SPACE_DATA, 1 );
	LINK( starsystem, first_starsystem, last_starsystem, next, prev );

	starsystem->name = STRALLOC( argument );

	starsystem->location1a = STRALLOC( "" );
	starsystem->location2a = STRALLOC( "" );
	starsystem->location3a = STRALLOC( "" );
	starsystem->location4a = STRALLOC( "" );
	starsystem->location5a = STRALLOC( "" );
	starsystem->location6a = STRALLOC( "" );
	starsystem->location7a = STRALLOC( "" );
	starsystem->location8a = STRALLOC( "" );
	starsystem->location9a = STRALLOC( "" );
	starsystem->location1b = STRALLOC( "" );
	starsystem->location2b = STRALLOC( "" );
	starsystem->location3b = STRALLOC( "" );
	starsystem->location4b = STRALLOC( "" );
	starsystem->location5b = STRALLOC( "" );
	starsystem->location6b = STRALLOC( "" );
	starsystem->location7b = STRALLOC( "" );
	starsystem->location8b = STRALLOC( "" );
	starsystem->location9b = STRALLOC( "" );
	starsystem->location1c = STRALLOC( "" );
	starsystem->location2c = STRALLOC( "" );
	starsystem->location3c = STRALLOC( "" );
	starsystem->location4c = STRALLOC( "" );
	starsystem->location5c = STRALLOC( "" );
	starsystem->location6c = STRALLOC( "" );
	starsystem->location7c = STRALLOC( "" );
	starsystem->location8c = STRALLOC( "" );
	starsystem->location9c = STRALLOC( "" );
	starsystem->planet1 = STRALLOC( "" );
	starsystem->planet2 = STRALLOC( "" );
	starsystem->planet3 = STRALLOC( "" );
	starsystem->planet4 = STRALLOC( "" );
	starsystem->planet5 = STRALLOC( "" );
	starsystem->planet6 = STRALLOC( "" );
	starsystem->planet7 = STRALLOC( "" );
	starsystem->planet8 = STRALLOC( "" );
	starsystem->planet9 = STRALLOC( "" );
	starsystem->star1 = STRALLOC( "" );
	starsystem->star2 = STRALLOC( "" );

	argument = one_argument( argument, arg );
	snprintf( filename, sizeof(filename), "%s.system", strlower( arg ) );
	starsystem->filename = str_dup( filename );
	save_starsystem( starsystem );
	write_starsystem_list( );
}

CMDF( do_starsystems )
{
	SPACE_DATA *starsystem;
	int count = 0;

	for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
	{
		set_char_color( AT_NOTE, ch );
		ch_printf( ch, "%s\r\n", starsystem->name );
		count++;
	}

	if( !count )
	{
		send_to_char( "There are no starsystems currently formed.\r\n", ch );
		return;
	}
}

void echo_to_ship( int color, SHIP_DATA *ship, const char *argument )
{
	int room;

	for( room = ship->firstroom; room <= ship->lastroom; room++ )
	{
		echo_to_room( color, get_room_index( room ), argument );
	}

}

void sound_to_ship( SHIP_DATA *ship, const char *argument )
{
	int roomnum;
	ROOM_INDEX_DATA *room;
	CHAR_DATA *vic;

	for( roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++ )
	{
		room = get_room_index( roomnum );
		if( room == NULL )
			continue;

		for( vic = room->first_person; vic; vic = vic->next_in_room )
		{
			//     if ( !IS_NPC(vic) && xIS_SET( vic->act, PLR_SOUND ) )
			//       send_to_char( argument, vic );
		}
	}

}

void echo_to_cockpit( int color, SHIP_DATA *ship, const char *argument )
{
	int room;

	for( room = ship->firstroom; room <= ship->lastroom; room++ )
	{
		if( room == ship->cockpit || room == ship->navseat
			|| room == ship->pilotseat || room == ship->coseat
			|| room == ship->gunseat || room == ship->engineroom || room == ship->turret1 || room == ship->turret2 )
			echo_to_room( color, get_room_index( room ), argument );
	}

}

void echo_to_system( int color, SHIP_DATA *ship, const char *argument, SHIP_DATA *ignore )
{
	SHIP_DATA *target;

	if( !ship->starsystem )
		return;

	for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
	{
		if( target != ship && target != ignore )
			if( abs( target->vx - ship->vx ) < 1000 + ship->sensor
				&& abs( target->vy - ship->vy ) < 1000 + ship->sensor && abs( target->vz - ship->vz ) < 1000 + ship->sensor )
				echo_to_cockpit( color, target, argument );
	}

}

bool is_facing( SHIP_DATA *ship, SHIP_DATA *target )
{
	float dy, dx, dz, hx, hy, hz;
	float cosofa;

	hx = ship->hx;
	hy = ship->hy;
	hz = ship->hz;

	dx = target->vx - ship->vx;
	dy = target->vy - ship->vy;
	dz = target->vz - ship->vz;

	cosofa = ( hx * dx + hy * dy + hz * dz ) / ( sqrt( hx * hx + hy * hy + hz * hz ) + sqrt( dx * dx + dy * dy + dz * dz ) );

	if( cosofa > 0.75 )
		return true;

	return false;
}


long int get_ship_value( SHIP_DATA *ship )
{

	/*
		 long int price;

		 if (ship->ship_class == MOBILE_SUIT)
			price = 5000;
		 else if (ship->ship_class == TRANSPORT_SHIP)
			price = 50000;
		 else if (ship->ship_class == CAPITAL_SHIP)
			price = 500000;
		 else
			price = 2000;

		 if ( ship->ship_class <= CAPITAL_SHIP )
		   price += ( ship->manuever*100*(1+ship->ship_class) );

		 price += ( ship->tractorbeam * 100 );
		 price += ( ship->realspeed * 10 );
		 price += ( ship->astro_array *5 );
		 price += ( 5 * ship->maxarmor );
		 price += ( 2 * ship->maxenergy );
		 price += ( 100 * ship->maxchaff );

		 if (ship->maxenergy > 5000 )
			  price += ( (ship->maxenergy-5000)*20 ) ;

		 if (ship->maxenergy > 10000 )
			  price += ( (ship->maxenergy-10000)*50 );

		 if (ship->maxarmor > 1000)
			price += ( (ship->maxarmor-1000)*10 );

		 if (ship->maxarmor > 10000)
			price += ( (ship->maxarmor-10000)*20 );

		 if (ship->maxshield > 200)
			  price += ( (ship->maxshield-200)*50 );

		 if (ship->maxshield > 1000)
			  price += ( (ship->maxshield-1000)*100 );

		 if (ship->realspeed > 100 )
			price += ( (ship->realspeed-100)*500 ) ;

		 if (ship->lasers > 5 )
			price += ( (ship->lasers-5)*500 );

		 if (ship->maxshield)
			price += ( 1000 + 10 * ship->maxshield);

		 if (ship->lasers)
			price += ( 500 + 500 * ship->lasers );

		 if (ship->maxmissiles)
			price += ( 1000 + 100 * ship->maxmissiles );
		 if (ship->maxrockets)
			price += ( 2000 + 200 * ship->maxmissiles );
		 if (ship->maxtorpedos)
			price += ( 1500 + 150 * ship->maxmissiles );

		 if (ship->missiles )
			price += ( 250 * ship->missiles );
		 else if (ship->torpedos )
			price += ( 500 * ship->torpedos );
		 else if (ship->rockets )
			price += ( 1000 * ship->rockets );

		 if (ship->turret1)
			price += 5000;

		 if (ship->turret2)
			price += 5000;

		 if (ship->hyperspeed)
			price += ( 1000 + ship->hyperspeed * 10 );

		 if (ship->hanger)
			price += ( ship->ship_class == TRANSPORT_SHIP ? 50000 : 100000 );

		 price *= 1.5;
	*/
	return ship->price;

}

void write_ship_list( )
{
	SHIP_DATA *tship;
	FILE *fpout;
	char filename[256];

	snprintf( filename, sizeof(filename), "%s%s", SHIP_DIR, SHIP_LIST );
	fpout = FileOpen( filename, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open ship.lst for writing!\r\n", 0 );
		return;
	}
	for( tship = first_ship; tship; tship = tship->next )
	{
		if( tship->ship_class != SMINE )
			fprintf( fpout, "%s\n", tship->filename );
	}
	fprintf( fpout, "$\n" );
	FileClose( fpout );
}

SHIP_DATA *ship_in_room( ROOM_INDEX_DATA *room, const char *name )
{
	SHIP_DATA *ship;

	if( !room )
		return NULL;

	for( ship = room->first_ship; ship; ship = ship->next_in_room )
		if( !str_cmp( name, ship->name ) )
			return ship;

	for( ship = room->first_ship; ship; ship = ship->next_in_room )
		if( nifty_is_name_prefix( name, ship->name ) )
			return ship;

	return NULL;
}

/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *get_ship( const char *name )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( !str_cmp( name, ship->name ) )
			return ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( nifty_is_name_prefix( name, ship->name ) )
			return ship;

	return NULL;
}

/*
 * Checks if ships in a starsystem and returns poiner if it is.
 */
SHIP_DATA *get_ship_here( const char *name, SPACE_DATA *starsystem )
{
	SHIP_DATA *ship;

	if( starsystem == NULL )
		return NULL;

	for( ship = starsystem->first_ship; ship; ship = ship->next_in_starsystem )
		if( !str_cmp( name, ship->name ) )
			return ship;

	for( ship = starsystem->first_ship; ship; ship = ship->next_in_starsystem )
		if( nifty_is_name_prefix( name, ship->name ) )
			return ship;

	return NULL;
}


/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *ship_from_pilot( const char *name )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( !str_cmp( name, ship->pilot ) )
			return ship;
	if( !str_cmp( name, ship->copilot ) )
		return ship;
	if( !str_cmp( name, ship->owner ) )
		return ship;
	return NULL;
}


/*
 * Get pointer to ship structure from cockpit, turret, or entrance ramp vnum.
 */

SHIP_DATA *ship_from_cockpit( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->cockpit || vnum == ship->turret1 || vnum == ship->turret2
			|| vnum == ship->pilotseat || vnum == ship->coseat || vnum == ship->navseat
			|| vnum == ship->gunseat || vnum == ship->engineroom )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_pilotseat( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->pilotseat )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_coseat( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->coseat )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_navseat( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->navseat )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_gunseat( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->gunseat )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_engine( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->engineroom )
		{
			if( vnum == ship->engineroom )
				return ship;
		}
		else
		{
			if( vnum == ship->cockpit )
				return ship;
		}
	}

	return NULL;
}



SHIP_DATA *ship_from_turret( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->gunseat || vnum == ship->turret1 || vnum == ship->turret2 )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_entrance( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->entrance )
			return ship;
	return NULL;
}

SHIP_DATA *ship_from_hanger( int vnum )
{
	SHIP_DATA *ship;

	for( ship = first_ship; ship; ship = ship->next )
		if( vnum == ship->hanger )
			return ship;
	return NULL;
}


void save_ship( SHIP_DATA *ship )
{
	FILE *fp;
	char filename[256];
	char buf[MAX_STRING_LENGTH];

	if( !ship )
	{
		bug( "save_ship: null ship pointer!", 0 );
		return;
	}

	if( ship->ship_class == SMINE )
		return;

	if( !ship->filename || ship->filename[0] == '\0' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "save_ship: %s has no filename", ship->name );
		bug( buf, 0 );
		return;
	}

	snprintf( filename, sizeof(filename), "%s%s", SHIP_DIR, ship->filename );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_ship: FileOpen", 0 );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#SHIP\n" );
		fprintf( fp, "Name         %s~\n", ship->name );
		fprintf( fp, "Filename     %s~\n", ship->filename );
		fprintf( fp, "Description  %s~\n", ship->description );
		fprintf( fp, "Owner        %s~\n", ship->owner );
		fprintf( fp, "Pilot        %s~\n", ship->pilot );
		fprintf( fp, "Copilot      %s~\n", ship->copilot );
		fprintf( fp, "PrevOwner    %s~\n", ship->prevowner );
		fprintf( fp, "Nickname     %s~\n", ship->nickname );
		fprintf( fp, "Colorone     %s~\n", ship->colorone );
		fprintf( fp, "Colortwo     %s~\n", ship->colortwo );
		fprintf( fp, "Class        %d\n", ship->ship_class );
		fprintf( fp, "Tractorbeam  %d\n", ship->tractorbeam );
		fprintf( fp, "Shipyard     %d\n", ship->shipyard );
		fprintf( fp, "Hanger       %d\n", ship->hanger );
		fprintf( fp, "Turret1      %d\n", ship->turret1 );
		fprintf( fp, "Turret2      %d\n", ship->turret2 );
		fprintf( fp, "Statet0      %d\n", ship->statet0 );
		fprintf( fp, "Statet1      %d\n", ship->statet1 );
		fprintf( fp, "Statet2      %d\n", ship->statet2 );
		fprintf( fp, "Lasers       %d\n", ship->lasers );
		fprintf( fp, "Missiles     %d\n", ship->missiles );
		fprintf( fp, "Maxmissiles  %d\n", ship->maxmissiles );
		fprintf( fp, "Rockets     %d\n", ship->rockets );
		fprintf( fp, "Maxrockets  %d\n", ship->maxrockets );
		fprintf( fp, "Torpedos     %d\n", ship->torpedos );
		fprintf( fp, "Maxtorpedos  %d\n", ship->maxtorpedos );
		fprintf( fp, "Lastdoc      %d\n", ship->lastdoc );
		fprintf( fp, "Firstroom    %d\n", ship->firstroom );
		fprintf( fp, "Lastroom     %d\n", ship->lastroom );
		fprintf( fp, "Shield       %d\n", ship->shield );
		fprintf( fp, "Maxshield    %d\n", ship->maxshield );
		fprintf( fp, "Armor        %d\n", ship->armor );
		fprintf( fp, "Maxarmor     %d\n", ship->maxarmor );
		fprintf( fp, "Armorhead    %d\n", ship->armorhead );
		fprintf( fp, "Maxarmorhead %d\n", ship->maxarmorhead );
		fprintf( fp, "Armorlarm    %d\n", ship->armorlarm );
		fprintf( fp, "Maxarmorlarm %d\n", ship->maxarmorlarm );
		fprintf( fp, "Armorrarm    %d\n", ship->armorrarm );
		fprintf( fp, "Maxarmorrarm %d\n", ship->maxarmorrarm );
		fprintf( fp, "Armorlegs    %d\n", ship->armorlegs );
		fprintf( fp, "Maxarmorlegs %d\n", ship->maxarmorlegs );
		fprintf( fp, "Targettype   %d\n", ship->targettype );
		fprintf( fp, "Offon        %d\n", ship->offon );
		fprintf( fp, "Mod          %d\n", ship->mod );
		fprintf( fp, "Code         %d\n", ship->code );
		fprintf( fp, "Price        %d\n", ship->price );
		fprintf( fp, "ABoom        %d\n", ship->autoboom );
		fprintf( fp, "Prevtimer    %d\n", ship->prevtimer );
		fprintf( fp, "Hover        %d\n", ship->hover );
		fprintf( fp, "Radio        %d\n", ship->radio );
		fprintf( fp, "Station      %d\n", ship->station );
		fprintf( fp, "Maxenergy    %d\n", ship->maxenergy );
		fprintf( fp, "Mines        %d\n", ship->mines );
		fprintf( fp, "MaxMines     %d\n", ship->maxmines );
		fprintf( fp, "Bombs        %d\n", ship->bombs );
		fprintf( fp, "Maxammo      %d\n", ship->maxammo );
		fprintf( fp, "Ammo         %d\n", ship->ammo );
		fprintf( fp, "Hyperspeed   %d\n", ship->hyperspeed );
		fprintf( fp, "Comm         %d\n", ship->comm );
		fprintf( fp, "Chaff        %d\n", ship->chaff );
		fprintf( fp, "Maxchaff     %d\n", ship->maxchaff );
		fprintf( fp, "Sensor       %d\n", ship->sensor );
		fprintf( fp, "Astro_array  %d\n", ship->astro_array );
		fprintf( fp, "Realspeed    %d\n", ship->realspeed );
		fprintf( fp, "Type         %d\n", ship->type );
		fprintf( fp, "Cockpit      %d\n", ship->cockpit );
		fprintf( fp, "Coseat       %d\n", ship->coseat );
		fprintf( fp, "Pilotseat    %d\n", ship->pilotseat );
		fprintf( fp, "Gunseat      %d\n", ship->gunseat );
		fprintf( fp, "Navseat      %d\n", ship->navseat );
		fprintf( fp, "Flags        %d\n", ship->flags );
		fprintf( fp, "MaxCargo     %d\n", ship->maxcargo );
		fprintf( fp, "Cargo        %d\n", ship->cargo );
		fprintf( fp, "CargoType    %d\n", ship->cargotype );
		fprintf( fp, "Engineroom   %d\n", ship->engineroom );
		fprintf( fp, "Entrance     %d\n", ship->entrance );
		fprintf( fp, "Shipstate    %d\n", ship->shipstate );
		fprintf( fp, "Missilestate %d\n", ship->missilestate );
		fprintf( fp, "Stype        %d\n", ship->stype );
		fprintf( fp, "Firstweapon  %d\n", ship->firstweapon );
		fprintf( fp, "Secondweapon %d\n", ship->secondweapon );
		fprintf( fp, "Thirdweapon  %d\n", ship->thirdweapon );
		fprintf( fp, "Energy       %d\n", ship->energy );
		fprintf( fp, "Alloy        %d\n", ship->alloy );
		fprintf( fp, "Manuever     %d\n", ship->manuever );
		fprintf( fp, "Home         %s~\n", ship->home );
		fprintf( fp, "Weight       %d\n", ship->weight );
		fprintf( fp, "Frame        %d\n", ship->frame );
		fprintf( fp, "Framemax     %d\n", ship->framemax );
		fprintf( fp, "Framename    %s~\n", ship->framename );
		fprintf( fp, "Dpower       %d\n", ship->dpow );
		fprintf( fp, "Parm         %d\n", ship->parm );
		fprintf( fp, "WoneName     %s~\n", ship->wonename );
		fprintf( fp, "WoneDam      %d\n", ship->wonedam );
		fprintf( fp, "WoneType     %d\n", ship->wonetype );
		fprintf( fp, "WoneLvl      %d\n", ship->wonelvl );

		fprintf( fp, "WtwoName     %s~\n", ship->wtwoname );
		fprintf( fp, "WtwoDam      %d\n", ship->wtwodam );
		fprintf( fp, "WtwoType     %d\n", ship->wtwotype );
		fprintf( fp, "WtwoLvl      %d\n", ship->wtwolvl );

		fprintf( fp, "Speed        %d\n", ship->speed );
		fprintf( fp, "Speedmax     %d\n", ship->speedmax );
		fprintf( fp, "Agility      %d\n", ship->agility );
		fprintf( fp, "Agilitymax   %d\n", ship->agilitymax );
		fprintf( fp, "Shieldb      %d\n", ship->shieldb );
		fprintf( fp, "Shieldmaxb    %d\n", ship->shieldmaxb );
		fprintf( fp, "Reserve      %d\n", ship->reserve );
		fprintf( fp, "Reservemax   %d\n", ship->reservemax );
		fprintf( fp, "WRightarm %d\n", ship->wrightarm );
		fprintf( fp, "WLeftarm      %d\n", ship->wleftarm );
		fprintf( fp, "Sp           %d\n", ship->sp );
		fprintf( fp, "Spmax        %d\n", ship->spmax );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
	}
	FileClose( fp );
	return;
}


/*
 * Read in actual ship data.
 */

void fread_ship( SHIP_DATA *ship, FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	int dummy_number;

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
			KEY( "ABoom", ship->autoboom, fread_number( fp ) );
			KEY( "Astro_array", ship->astro_array, fread_number( fp ) );
			KEY( "Alloy", ship->alloy, fread_number( fp ) );
			KEY( "Agility", ship->agility, fread_number( fp ) );
			KEY( "Agilitymax", ship->agilitymax, fread_number( fp ) );
			KEY( "Armor", ship->armor, fread_number( fp ) );
			KEY( "Armorhead", ship->armorhead, fread_number( fp ) );
			KEY( "Armorlarm", ship->armorlarm, fread_number( fp ) );
			KEY( "Armorrarm", ship->armorrarm, fread_number( fp ) );
			KEY( "Armorlegs", ship->armorlegs, fread_number( fp ) );


			KEY( "Ammo", ship->ammo, fread_number( fp ) );
			break;

		case 'B':
			KEY( "Bombs", ship->bombs, fread_number( fp ) );
			break;

		case 'C':
			KEY( "Cargo", ship->cargo, fread_number( fp ) );
			KEY( "CargoType", ship->cargotype, fread_number( fp ) );
			KEY( "Cockpit", ship->cockpit, fread_number( fp ) );
			KEY( "Coseat", ship->coseat, fread_number( fp ) );
			KEY( "Class", ship->ship_class, fread_number( fp ) );
			KEY( "Code", ship->code, fread_number( fp ) );
			KEY( "Colorone", ship->colorone, fread_string( fp ) );
			KEY( "Colortwo", ship->colortwo, fread_string( fp ) );
			KEY( "Copilot", ship->copilot, fread_string( fp ) );
			KEY( "Comm", ship->comm, fread_number( fp ) );
			KEY( "Chaff", ship->chaff, fread_number( fp ) );
			break;


		case 'D':
			KEY( "Dpower", ship->dpow, fread_number( fp ) );
			KEY( "Description", ship->description, fread_string( fp ) );
			break;

		case 'E':
			KEY( "Engineroom", ship->engineroom, fread_number( fp ) );
			KEY( "Entrance", ship->entrance, fread_number( fp ) );
			KEY( "Energy", ship->energy, fread_number( fp ) );
			KEY( "Ammo", ship->ammo, fread_number( fp ) );
			if( !str_cmp( word, "End" ) )
			{
				if( !ship->home )
					ship->home = STRALLOC( "" );
				if( !ship->name )
					ship->name = STRALLOC( "" );
				if( !ship->owner )
					ship->owner = STRALLOC( "" );
				if( !ship->description )
					ship->description = STRALLOC( "" );
				if( !ship->copilot )
					ship->copilot = STRALLOC( "" );
				if( !ship->pilot )
					ship->pilot = STRALLOC( "" );
				if( !ship->nickname )
					ship->nickname = STRALLOC( "" );
				if( !ship->colorone )
					ship->colorone = STRALLOC( "" );
				if( !ship->colortwo )
					ship->colortwo = STRALLOC( "" );
				if( !ship->framename )
					ship->framename = STRALLOC( "VX-10000" );
				if( !ship->wonename )
					ship->wonename = STRALLOC( "Punch" );
				if( !ship->wtwoname )
					ship->wtwoname = STRALLOC( "Punch" );

				if( ship->cargotype != CTYPE_NONE && ship->cargo < 1 )
					ship->cargotype = CTYPE_NONE;

				if( ship->shipstate != SHIP_DISABLED )
					ship->shipstate = SHIP_DOCKED;
				if( ship->statet0 != LASER_DAMAGED )
					ship->statet0 = LASER_READY;
				if( ship->statet1 != LASER_DAMAGED )
					ship->statet1 = LASER_READY;
				if( ship->statet2 != LASER_DAMAGED )
					ship->statet2 = LASER_READY;
				if( ship->missilestate != MISSILE_DAMAGED )
					ship->missilestate = MISSILE_READY;
				if( ship->shipyard <= 0 )
					ship->shipyard = ROOM_LIMBO_SHIPYARD;
				if( ship->lastdoc <= 0 )
					ship->lastdoc = ship->shipyard;
				ship->bayopen = false;
				ship->autopilot = false;
				ship->hatchopen = false;
				if( ship->navseat <= 0 )
					ship->navseat = ship->cockpit;
				if( ship->gunseat <= 0 )
					ship->gunseat = ship->cockpit;
				if( ship->coseat <= 0 )
					ship->coseat = ship->cockpit;
				if( ship->pilotseat <= 0 )
					ship->pilotseat = ship->cockpit;
				if( ship->missiletype == 1 )
				{
					ship->torpedos = ship->missiles;  /* for back compatability */
					ship->missiles = 0;
				}
				ship->starsystem = NULL;
				ship->energy = ship->maxenergy;
				ship->ammo = ship->maxammo;
				ship->armor = ship->maxarmor;
				ship->armorhead = ship->maxarmorhead;
				ship->armorlarm = ship->maxarmorlarm;
				ship->armorrarm = ship->maxarmorrarm;
				ship->armorlegs = ship->maxarmorlegs;
				ship->targettype = 0;
				ship->in_room = NULL;
				ship->next_in_room = NULL;
				ship->prev_in_room = NULL;

				return;
			}
			break;

		case 'F':
			KEY( "Filename", ship->filename, fread_string_nohash( fp ) );
			KEY( "Firstroom", ship->firstroom, fread_number( fp ) );
			KEY( "Firstweapon", ship->firstweapon, fread_number( fp ) );
			KEY( "Flags", ship->flags, fread_number( fp ) );
			KEY( "Frame", ship->frame, fread_number( fp ) );
			KEY( "Framemax", ship->framemax, fread_number( fp ) );
			KEY( "Framename", ship->framename, fread_string( fp ) );

			break;

		case 'G':
			KEY( "Gunseat", ship->gunseat, fread_number( fp ) );
			break;

		case 'H':
			KEY( "Home", ship->home, fread_string( fp ) );
			KEY( "Hyperspeed", ship->hyperspeed, fread_number( fp ) );
			KEY( "Hanger", ship->hanger, fread_number( fp ) );
			KEY( "Hover", ship->hover, fread_number( fp ) );
			break;

		case 'L':
			KEY( "Laserstr", ship->lasers, ( short ) ( fread_number( fp ) / 10 ) );
			KEY( "Lasers", ship->lasers, fread_number( fp ) );
			KEY( "Lastdoc", ship->lastdoc, fread_number( fp ) );
			KEY( "Lastroom", ship->lastroom, fread_number( fp ) );
			break;

		case 'M':
			KEY( "Manuever", ship->manuever, fread_number( fp ) );
			KEY( "MaxCargo", ship->maxcargo, fread_number( fp ) );
			KEY( "Maxmissiles", ship->maxmissiles, fread_number( fp ) );
			KEY( "Maxtorpedos", ship->maxtorpedos, fread_number( fp ) );
			KEY( "Maxrockets", ship->maxrockets, fread_number( fp ) );
			KEY( "Mines", ship->mines, fread_number( fp ) );
			KEY( "MaxMines", ship->maxmines, fread_number( fp ) );
			KEY( "Missiles", ship->missiles, fread_number( fp ) );
			KEY( "Missiletype", ship->missiletype, fread_number( fp ) );
			KEY( "Maxshield", ship->maxshield, fread_number( fp ) );
			KEY( "Maxenergy", ship->maxenergy, fread_number( fp ) );
			KEY( "Maxammo", ship->maxammo, fread_number( fp ) );
			KEY( "Missilestate", ship->missilestate, fread_number( fp ) );
			KEY( "Maxarmor", ship->maxarmor, fread_number( fp ) );
			KEY( "Maxarmorhead", ship->maxarmorhead, fread_number( fp ) );
			KEY( "Maxarmorlarm", ship->maxarmorlarm, fread_number( fp ) );
			KEY( "Maxarmorrarm", ship->maxarmorrarm, fread_number( fp ) );
			KEY( "Maxarmorlegs", ship->maxarmorlegs, fread_number( fp ) );
			KEY( "Maxchaff", ship->maxchaff, fread_number( fp ) );
			KEY( "Mod", ship->mod, fread_number( fp ) );
			break;

		case 'N':
			KEY( "Name", ship->name, fread_string( fp ) );
			KEY( "Navseat", ship->navseat, fread_number( fp ) );
			KEY( "Nickname", ship->nickname, fread_string( fp ) );
			break;

		case 'O':
			KEY( "Owner", ship->owner, fread_string( fp ) );
			KEY( "Objectnum", dummy_number, fread_number( fp ) );
			KEY( "Offon", ship->offon, fread_number( fp ) );
			break;

		case 'P':
			KEY( "Parm", ship->parm, fread_number( fp ) );
			KEY( "Pilot", ship->pilot, fread_string( fp ) );
			KEY( "Pilotseat", ship->pilotseat, fread_number( fp ) );
			KEY( "PrevOwner", ship->prevowner, fread_string( fp ) );
			KEY( "Prevtimer", ship->prevtimer, fread_number( fp ) );
			KEY( "Price", ship->price, fread_number( fp ) );
			break;

		case 'R':
			KEY( "Radio", ship->radio, fread_number( fp ) );
			KEY( "Realspeed", ship->realspeed, fread_number( fp ) );
			KEY( "Reserve", ship->reserve, fread_number( fp ) );
			KEY( "Reservemax", ship->reservemax, fread_number( fp ) );
			KEY( "Rockets", ship->rockets, fread_number( fp ) );
			break;

		case 'S':
			KEY( "Shipyard", ship->shipyard, fread_number( fp ) );
			KEY( "Sensor", ship->sensor, fread_number( fp ) );
			KEY( "Secondweapon", ship->secondweapon, fread_number( fp ) );
			KEY( "Shield", ship->shield, fread_number( fp ) );
			KEY( "Shipstate", ship->shipstate, fread_number( fp ) );
			KEY( "Sp", ship->sp, fread_number( fp ) );
			KEY( "Spmax", ship->spmax, fread_number( fp ) );
			KEY( "Speed", ship->speed, fread_number( fp ) );
			KEY( "Speedmax", ship->speedmax, fread_number( fp ) );
			KEY( "Shieldb", ship->shieldb, fread_number( fp ) );
			KEY( "Shieldmaxb", ship->shieldmaxb, fread_number( fp ) );
			KEY( "Stype", ship->stype, fread_number( fp ) );
			KEY( "Statet0", ship->statet0, fread_number( fp ) );
			KEY( "Statet1", ship->statet1, fread_number( fp ) );
			KEY( "Statet2", ship->statet2, fread_number( fp ) );
			KEY( "Station", ship->station, fread_number( fp ) );
			break;

		case 'T':
			KEY( "Targettype", ship->targettype, fread_number( fp ) );
			KEY( "Type", ship->type, fread_number( fp ) );
			KEY( "Tractorbeam", ship->tractorbeam, fread_number( fp ) );
			KEY( "Turret1", ship->turret1, fread_number( fp ) );
			KEY( "Turret2", ship->turret2, fread_number( fp ) );
			KEY( "Torpedos", ship->torpedos, fread_number( fp ) );
			KEY( "Thirdweapon", ship->thirdweapon, fread_number( fp ) );
			break;

		case 'W':
			KEY( "Weight", ship->weight, fread_number( fp ) );
			KEY( "WoneName", ship->wonename, fread_string( fp ) );
			KEY( "WoneDam", ship->wonedam, fread_number( fp ) );
			KEY( "WoneLvl", ship->wonelvl, fread_number( fp ) );
			KEY( "WoneType", ship->wonetype, fread_number( fp ) );
			KEY( "WtwoName", ship->wtwoname, fread_string( fp ) );
			KEY( "WtwoDam", ship->wtwodam, fread_number( fp ) );
			KEY( "WtwoLvl", ship->wtwolvl, fread_number( fp ) );
			KEY( "WtwoType", ship->wtwotype, fread_number( fp ) );
			KEY( "WRightarm", ship->wrightarm, fread_number( fp ) );
			/*
						KEY( "WRightarmtype", ship->wrightarmtype, fread_number( fp ) );
						KEY( "WRightarmdam",  ship->wrightarmdam,  fread_number( fp ) );
			*/
			KEY( "WLeftarm", ship->wleftarm, fread_number( fp ) );
			/*
						KEY( "WLeftarmtype", ship->wleftarmtype, fread_number( fp ) );
						KEY( "WLeftarmdam",  ship->wleftarmdam,  fread_number( fp ) );
			*/
			break;

		}

		if( !fMatch )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Fread_ship: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

/*
 * Load a ship file
 */

bool load_ship_file( const char *shipfile )
{
	char filename[256];
	SHIP_DATA *ship = NULL;
	FILE *fp;
	bool found;
	ROOM_INDEX_DATA *pRoomIndex;
	CLAN_DATA *clan;

	CREATE( ship, SHIP_DATA, 1 );

	found = false;
	snprintf( filename, sizeof(filename), "%s%s", SHIP_DIR, shipfile );

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
				bug( "Load_ship_file: # not found.", 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "SHIP" ) )
			{
				fread_ship( ship, fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				char buf[MAX_STRING_LENGTH];

				snprintf( buf, MAX_STRING_LENGTH, "Load_ship_file: bad section: %s.", word );
				bug( buf, 0 );
				break;
			}
		}
		FileClose( fp );
	}
	if( !( found ) )
		DISPOSE( ship );
	else
	{
		LINK( ship, first_ship, last_ship, next, prev );
		if( !str_cmp( "Public", ship->owner ) || ship->type == MOB_SHIP )
		{

			if( ship->ship_class != SHIP_PLATFORM && ship->type != MOB_SHIP && ship->ship_class != CAPITAL_SHIP )
			{
				extract_ship( ship );
				ship_to_room( ship, ship->shipyard );

				ship->location = ship->shipyard;
				ship->lastdoc = ship->shipyard;
				ship->shipstate = SHIP_DOCKED;
			}

			ship->currspeed = 0;
			ship->energy = ship->maxenergy;
			ship->ammo = ship->maxammo;
			ship->chaff = ship->maxchaff;
			ship->armor = ship->maxarmor;
			ship->armorhead = ship->maxarmorhead;
			ship->armorlarm = ship->maxarmorlarm;
			ship->armorrarm = ship->maxarmorrarm;
			ship->armorlegs = ship->maxarmorlegs;
			ship->targettype = 0;
			ship->autoboom = ship->autoboom;
			ship->offon = ship->offon;
			ship->mod = ship->mod;
			ship->code = ship->code;
			ship->price = ship->price;
			ship->shield = 0;
			ship->mines = ship->mines;
			ship->maxmines = ship->maxmines;
			ship->bombs = ship->bombs;
			ship->hover = 0;
			ship->radio = 1;
			ship->station = ship->station;
			ship->maxcargo = ship->maxcargo;
			ship->cargo = ship->cargo;
			ship->cargotype = ship->cargotype;
			ship->grid = 0;
			ship->gridlvl = 1;
			ship->tlength = 4;
			ship->fnum = 0;
			ship->move[0] = 0;
			ship->move[1] = 0;
			ship->move[2] = 0;
			ship->move[3] = 0;

			if( IS_SET( ship->flags, SUIT_INCOMBAT ) )
				REMOVE_BIT( ship->flags, SUIT_INCOMBAT );

			if( IS_SET( ship->flags, SUIT_ISGUARDING ) )
				REMOVE_BIT( ship->flags, SUIT_ISGUARDING );

			if( IS_SET( ship->flags, SUIT_INGRID ) )
				REMOVE_BIT( ship->flags, SUIT_INGRID );

			if( IS_SET( ship->flags, SUIT_MOVING ) )
				REMOVE_BIT( ship->flags, SUIT_MOVING );

			if( ship->mod == 3 )
			{
				if( ship->offon == 1 )
				{
					ship->offon = 0;
					ship->realspeed = ship->realspeed / 2;
					ship->hyperspeed = ship->hyperspeed / 2;
				}
			}

			ship->statet1 = LASER_READY;
			ship->statet2 = LASER_READY;
			ship->statet0 = LASER_READY;
			ship->missilestate = LASER_READY;

			ship->currjump = NULL;
			ship->target0 = NULL;
			ship->target1 = NULL;
			ship->target2 = NULL;

			ship->hatchopen = false;
			ship->bayopen = false;

			ship->missiles = ship->maxmissiles;
			ship->torpedos = ship->maxtorpedos;
			ship->rockets = ship->maxrockets;
			ship->autorecharge = false;
			ship->autotrack = false;
			ship->autospeed = false;


		}

		else if( ship->cockpit == ROOM_SHUTTLE_BUS ||
			ship->cockpit == ROOM_SHUTTLE_BUS_2 ||
			ship->cockpit == ROOM_SENATE_SHUTTLE ||
			ship->cockpit == ROOM_CORUSCANT_TURBOCAR || ship->cockpit == ROOM_CORUSCANT_SHUTTLE )
		{
		}
		else if( ( pRoomIndex = get_room_index( ship->lastdoc ) ) != NULL
			&& ship->ship_class != CAPITAL_SHIP && ship->ship_class != SHIP_PLATFORM )
		{
			LINK( ship, pRoomIndex->first_ship, pRoomIndex->last_ship, next_in_room, prev_in_room );
			ship->in_room = pRoomIndex;
			ship->location = ship->lastdoc;
		}


		if( ship->ship_class == SHIP_PLATFORM || ship->type == MOB_SHIP || ship->ship_class == CAPITAL_SHIP )
		{
			ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
			ship->vx = number_range( -5000, 5000 );
			ship->vy = number_range( -5000, 5000 );
			ship->vz = number_range( -5000, 5000 );
			ship->hx = 1;
			ship->hy = 1;
			ship->hz = 1;
			ship->shipstate = SHIP_READY;
			ship->autopilot = true;
			ship->autorecharge = true;
			ship->shield = ship->maxshield;
			ship->pksuit = 1;
		}

		if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
		{
			if( ship->ship_class <= SHIP_PLATFORM )
				clan->spacecraft++;
			else
				clan->vehicles++;
		}

	}

	return found;
}

/*
 * Load in all the ship files.
 */
void load_ships( )
{
	FILE *fpList;
	const char *filename;
	char shiplist[256];
	char buf[MAX_STRING_LENGTH];


	first_ship = NULL;
	last_ship = NULL;
	first_missile = NULL;
	last_missile = NULL;
	first_weapon = NULL;
	last_weapon = NULL;

	log_string( "Loading ships..." );

	snprintf( shiplist, sizeof(shiplist), "%s%s", SHIP_DIR, SHIP_LIST );
	if( ( fpList = FileOpen( shiplist, "r" ) ) == NULL )
	{
		perror( shiplist );
		exit( 109 );
	}

	for( ;; )
	{

		filename = feof( fpList ) ? "$" : fread_word( fpList );

		if( filename[0] == '$' )
			break;

		if( !load_ship_file( filename ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Cannot load ship file: %s", filename );
			bug( buf, 0 );
		}

	}
	FileClose( fpList );
	log_string( " Done ships " );
	return;
}

void resetship( SHIP_DATA *ship )
{
	ship->shipstate = SHIP_READY;

	if( ship->ship_class != SHIP_PLATFORM && ship->type != MOB_SHIP )
	{
		extract_ship( ship );
		ship_to_room( ship, ship->shipyard );

		ship->location = ship->shipyard;
		ship->lastdoc = ship->shipyard;
		ship->shipstate = SHIP_DOCKED;
	}

	if( ship->starsystem )
		ship_from_starsystem( ship, ship->starsystem );

	ship->currspeed = 0;
	ship->energy = ship->maxenergy;
	ship->ammo = ship->maxammo;
	ship->chaff = ship->maxchaff;
	ship->armor = ship->maxarmor;
	ship->armorhead = ship->maxarmorhead;
	ship->armorlarm = ship->maxarmorlarm;
	ship->armorrarm = ship->maxarmorrarm;
	ship->armorlegs = ship->maxarmorlegs;
	ship->alloy = 0;
	ship->targettype = 0;
	ship->shield = 0;

	ship->statet1 = LASER_READY;
	ship->statet2 = LASER_READY;
	ship->statet0 = LASER_READY;
	ship->missilestate = LASER_READY;

	ship->currjump = NULL;
	ship->target0 = NULL;
	ship->target1 = NULL;
	ship->target2 = NULL;

	ship->hatchopen = false;
	ship->bayopen = false;

	ship->missiles = ship->maxmissiles;
	ship->torpedos = ship->maxtorpedos;
	ship->rockets = ship->maxrockets;
	ship->autorecharge = false;
	ship->autotrack = false;
	ship->autospeed = false;

	if( str_cmp( "Public", ship->owner ) && ship->type != MOB_SHIP )
	{
		CLAN_DATA *clan;

		if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
		{
			if( ship->ship_class <= SHIP_PLATFORM )
				clan->spacecraft--;
			else
				clan->vehicles--;

			if( ship->stype != 5 )
			{
				STRFREE( ship->owner );
				ship->owner = STRALLOC( "" );
				STRFREE( ship->pilot );
				ship->pilot = STRALLOC( "" );
				STRFREE( ship->copilot );
				ship->copilot = STRALLOC( "" );
				STRFREE( ship->prevowner );
				ship->prevowner = STRALLOC( "" );
			}
		}
	}

	if( ship->type == SHIP_REPUBLIC || ( ship->type == MOB_SHIP && !str_cmp( ship->owner, "the new republic" ) ) )
	{
		STRFREE( ship->home );
		ship->home = STRALLOC( "coruscant" );
	}
	else if( ship->type == SHIP_IMPERIAL || ( ship->type == MOB_SHIP && !str_cmp( ship->owner, "the empire" ) ) )
	{
		STRFREE( ship->home );
		ship->home = STRALLOC( "byss" );
	}
	else if( ship->type == SHIP_CIVILIAN )
	{
		STRFREE( ship->home );
		ship->home = STRALLOC( "G42" );
	}
	save_ship( ship );
	return;
}

CMDF( do_resetship )
{
	SHIP_DATA *ship;

	ship = get_ship( argument );
	if( ship == NULL )
	{
		send_to_char( "&RNo such ship!", ch );
		return;
	}

	resetship( ship );

	if( ( ship->ship_class == SHIP_PLATFORM || ship->type == MOB_SHIP || ship->ship_class == CAPITAL_SHIP ) && ship->home )
	{
		ship_to_starsystem( ship, starsystem_from_name( ship->home ) );
		ship->vx = number_range( -5000, 5000 );
		ship->vy = number_range( -5000, 5000 );
		ship->vz = number_range( -5000, 5000 );
		ship->shipstate = SHIP_READY;
		ship->autopilot = true;
		ship->autorecharge = true;
		ship->shield = ship->maxshield;
		ship->offon = 0;
		ship->mod = 0;
		ship->code = 55;
		ship->price = ship->price;
		ship->autoboom = 0;
		ship->mines = ship->mines;
		ship->maxmines = ship->maxmines;
		ship->bombs = ship->bombs;
	}

}

CMDF( do_setship )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	int tempnum;
	int value;
	ROOM_INDEX_DATA *roomindex;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( IS_SUPREME( ch ) && !str_cmp( arg1, "clear" ) )
	{
		for( ship = first_ship; ship; ship = ship->next )
		{
			if( ship->stype == 5 )
				continue;
			if( ship->type == MOB_SHIP )
				continue;
			if( !str_cmp( "Public", ship->owner ) )
				continue;
			if( !str_cmp( "", ship->owner ) )
				continue;
			if( !str_cmp( "Holosuit", ship->owner ) )
				continue;
			if( !str_cmp( "Star Shuttle", ship->owner ) )
				continue;

			if( !exists_player( ship->owner ) )
			{
				log_printf( "Ship %s previously owned by %s cleared.", ship->name, ship->owner );
				STRFREE( ship->owner );
				ship->owner = STRALLOC( "" );
				STRFREE( ship->pilot );
				ship->pilot = STRALLOC( "" );
				STRFREE( ship->copilot );
				ship->copilot = STRALLOC( "" );
				STRFREE( ship->description );
				ship->description = STRALLOC( "" );
				STRFREE( ship->nickname );
				ship->nickname = STRALLOC( "" );
				STRFREE( ship->colorone );
				ship->colorone = STRALLOC( "" );
				STRFREE( ship->colortwo );
				ship->colortwo = STRALLOC( "" );
				STRFREE( ship->framename );
				ship->framename = STRALLOC( "VX-10000" );
				save_ship( ship );
			}
		}
		return;
	}

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
	{
		send_to_char( "Usage: setship <ship> <field> <values>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( "filename name owner copilot pilot prevowner description\r\n", ch );
		send_to_char( "home cockpit entrance turret1 turret2 prevtimer hanger\r\n", ch );
		send_to_char( "engineroom firstroom lastroom shipyard\r\n", ch );
		send_to_char( "manuever speed hyperspeed tractorbeam\r\n", ch );
		send_to_char( "lasers missiles shield energy chaff\r\n", ch );
		send_to_char( "comm sensor astroarray class torpedos bombs\r\n", ch );
		send_to_char( "pilotseat coseat gunseat navseat rockets ammo maxmines\r\n", ch );
		send_to_char( "firstweapon secondweapon thirdweapon alloy mod code price\r\n", ch );
		send_to_char( "headarmor legarmor rightarmarmor leftarmarmor torsoarmor\r\n", ch );
		return;
	}

	ship = get_ship( arg1 );
	if( !ship )
	{
		send_to_char( "No such ship.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "owner" ) )
	{
		CLAN_DATA *clan;
		if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
		{
			if( ship->ship_class <= SHIP_PLATFORM )
				clan->spacecraft--;
			else
				clan->vehicles--;
		}
		STRFREE( ship->owner );
		ship->owner = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		if( ship->type != MOB_SHIP && ( clan = get_clan( ship->owner ) ) != NULL )
		{
			if( ship->ship_class <= SHIP_PLATFORM )
				clan->spacecraft++;
			else
				clan->vehicles++;
		}
		return;
	}

	if( !str_cmp( arg2, "offon" ) )
	{
		if( !str_cmp( argument, "off" ) )
			ship->offon = 0;
		else if( !str_cmp( argument, "on" ) )
			ship->offon = 1;
		else
		{
			send_to_char( "Set ship's mod <off> or <on>?\r\n", ch );
			return;
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "home" ) )
	{
		STRFREE( ship->home );
		ship->home = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "pilot" ) )
	{
		STRFREE( ship->pilot );
		ship->pilot = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "copilot" ) )
	{
		STRFREE( ship->copilot );
		ship->copilot = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "clear" ) )
	{
		if( IS_SET( ship->flags, SUIT_INCOMBAT ) )
			REMOVE_BIT( ship->flags, SUIT_INCOMBAT );

		if( IS_SET( ship->flags, SUIT_ISGUARDING ) )
			REMOVE_BIT( ship->flags, SUIT_ISGUARDING );

		free_suitfight( ship );
		ship->fstate = FS_OUT;
		ship->target0 = NULL;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;

	}

	if( !str_cmp( arg2, "prevowner" ) )
	{
		STRFREE( ship->prevowner );
		ship->prevowner = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "firstroom" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		ship->firstroom = tempnum;
		ship->lastroom = tempnum;
		ship->cockpit = tempnum;
		ship->coseat = tempnum;
		ship->pilotseat = tempnum;
		ship->gunseat = tempnum;
		ship->navseat = tempnum;
		ship->entrance = tempnum;
		ship->turret1 = 0;
		ship->turret2 = 0;
		ship->hanger = 0;
		send_to_char( "You will now need to set the other rooms in the ship.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "maxcargo" ) )
	{
		ship->maxcargo = URANGE( 0, atoi( argument ), 500 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "lastroom" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom )
		{
			send_to_char( "The last room on a ship must be greater than or equal to the first room.\r\n", ch );
			return;
		}
		if( ship->ship_class == MOBILE_SUIT && ( tempnum - ship->firstroom ) > 5 )
		{
			send_to_char( "Starfighters may have up to 5 rooms only.\r\n", ch );
			return;
		}
		if( ship->ship_class == TRANSPORT_SHIP && ( tempnum - ship->firstroom ) > 25 )
		{
			send_to_char( "Midships may have up to 25 rooms only.\r\n", ch );
			return;
		}
		if( ship->ship_class == CAPITAL_SHIP && ( tempnum - ship->firstroom ) > 100 )
		{
			send_to_char( "Capital Ships may have up to 100 rooms only.\r\n", ch );
			return;
		}
		ship->lastroom = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "cockpit" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->cockpit = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "pilotseat" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->pilotseat = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}
	if( !str_cmp( arg2, "coseat" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->coseat = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}
	if( !str_cmp( arg2, "navseat" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->navseat = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}
	if( !str_cmp( arg2, "gunseat" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->gunseat = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "entrance" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		ship->entrance = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "turret1" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( ship->ship_class == MOBILE_SUIT )
		{
			send_to_char( "Starfighters can't have extra laser turrets.\r\n", ch );
			return;
		}
		if( tempnum == ship->cockpit || tempnum == ship->entrance ||
			tempnum == ship->turret2 || tempnum == ship->hanger || tempnum == ship->engineroom )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->turret1 = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "turret2" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( ship->ship_class == MOBILE_SUIT )
		{
			send_to_char( "Starfighters can't have extra laser turrets.\r\n", ch );
			return;
		}
		if( tempnum == ship->cockpit || tempnum == ship->entrance ||
			tempnum == ship->turret1 || tempnum == ship->hanger || tempnum == ship->engineroom )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->turret2 = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "hanger" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->cockpit || tempnum == ship->entrance ||
			tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->engineroom )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		if( ship->ship_class == MOBILE_SUIT )
		{
			send_to_char( "Starfighters are to small to have hangers for other ships!\r\n", ch );
			return;
		}
		ship->hanger = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "engineroom" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.\r\n", ch );
			return;
		}
		if( tempnum < ship->firstroom || tempnum > ship->lastroom )
		{
			send_to_char( "That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\r\n", ch );
			return;
		}
		if( tempnum == ship->cockpit || tempnum == ship->entrance ||
			tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hanger )
		{
			send_to_char( "That room is already being used by another part of the ship\r\n", ch );
			return;
		}
		ship->engineroom = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "shipyard" ) )
	{
		tempnum = atoi( argument );
		roomindex = get_room_index( tempnum );
		if( roomindex == NULL )
		{
			send_to_char( "That room doesn't exist.", ch );
			return;
		}
		ship->shipyard = tempnum;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "type" ) )
	{
		if( !str_cmp( argument, "republic" ) )
			ship->type = SHIP_REPUBLIC;
		else if( !str_cmp( argument, "imperial" ) )
			ship->type = SHIP_IMPERIAL;
		else if( !str_cmp( argument, "civilian" ) )
			ship->type = SHIP_CIVILIAN;
		else if( !str_cmp( argument, "mob" ) )
			ship->type = MOB_SHIP;
		else
		{
			send_to_char( "Ship type must be either: republic, imperial, civilian or mob.\r\n", ch );
			return;
		}
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "alloy" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Syntax: setship <ship> alloy <number>\r\n", ch );
			send_to_char( "Alloy: 1 - Iron        2 - Steel\r\n", ch );
			send_to_char( "       3 - Titanium    4 - Neo-Titanium\r\n", ch );
			send_to_char( "       5 - Gundanium   6 - Z-Gundanium\r\n", ch );
			send_to_char( "Setting to 0 makes armor Basic.\r\n", ch );
			return;
		}
		ship->alloy = URANGE( 0, atoi( argument ), 6 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "mod" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Syntax: setship <ship> mod <number>\r\n", ch );
			send_to_char( "Mods : 0 - None        1 - Zero System \r\n", ch );
			send_to_char( "       2 - Cloaking    3 - Transform\r\n", ch );
			send_to_char( "       4 - DeathBlow\r\n", ch );
			return;
		}
		ship->mod = URANGE( 0, atoi( argument ), 4 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "stype" ) )
	{
		ship->stype = URANGE( 0, atoi( argument ), 7 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "radio" ) )
	{
		ship->radio = URANGE( 0, atoi( argument ), 1 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	value = is_number( argument ) ? atoi( argument ) : -1;

	if( atoi( argument ) < -1 && value == -1 )
		value = atoi( argument );

	if( !str_cmp( arg2, "rightarm" ) )
	{
		value = get_suit_weapon( argument );
		if( value < 0 )
			value = atoi( argument );
		if( ( value < 0 || value >= MAX_SUITWEAPON ) )
		{
			ch_printf( ch, "Suit weapon range is 0 to %d.\n", MAX_SUITWEAPON - 1 );
			return;
		}
		ship->wrightarm = value;
		return;
	}

	if( !str_cmp( arg2, "leftarm" ) )
	{
		value = get_suit_weapon( argument );
		if( value < 0 )
			value = atoi( argument );
		if( ( value < 0 || value >= MAX_SUITWEAPON ) )
		{
			ch_printf( ch, "Suit weapon range is 0 to %d.\n", MAX_SUITWEAPON - 1 );
			return;
		}
		ship->wleftarm = value;
		return;
	}

	if( !str_cmp( arg2, "grid" ) )
	{
		ship->grid = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "target" ) )
	{
		SHIP_DATA *ttarget;

		ttarget = get_ship( argument );
		if( !ship )
		{
			send_to_char( "No such ship.\r\n", ch );
			return;
		}
		ship->target0 = ttarget;
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}


	if( !str_cmp( arg2, "prevtimer" ) )
	{
		ship->prevtimer = URANGE( 0, atoi( argument ), 60 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "code" ) )
	{
		ship->code = URANGE( 1, atoi( argument ), 99 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "bombs" ) )
	{
		ship->bombs = URANGE( 0, atoi( argument ), 20 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "maxmines" ) )
	{
		ship->maxmines = URANGE( 0, atoi( argument ), 20 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "price" ) )
	{
		ship->price = URANGE( 0, atoi( argument ), 10000000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "firstweapon" ) )
	{
		ship->firstweapon = URANGE( 0, atoi( argument ), 40 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}
	if( !str_cmp( arg2, "secondweapon" ) )
	{
		ship->secondweapon = URANGE( 0, atoi( argument ), 40 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}
	if( !str_cmp( arg2, "thirdweapon" ) )
	{
		ship->thirdweapon = URANGE( 0, atoi( argument ), 40 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		STRFREE( ship->name );
		ship->name = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "filename" ) )
	{
		DISPOSE( ship->filename );
		ship->filename = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		write_ship_list( );
		return;
	}

	if( !str_cmp( arg2, "desc" ) )
	{
		STRFREE( ship->description );
		ship->description = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "manuever" ) )
	{
		ship->manuever = URANGE( 0, atoi( argument ), 1000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "lasers" ) )
	{
		ship->lasers = URANGE( 0, atoi( argument ), 10 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "class" ) )
	{
		ship->ship_class = URANGE( 0, atoi( argument ), 9 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "missiles" ) )
	{
		ship->maxmissiles = URANGE( 0, atoi( argument ), 255 );
		ship->missiles = URANGE( 0, atoi( argument ), 255 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "torpedos" ) )
	{
		ship->maxtorpedos = URANGE( 0, atoi( argument ), 255 );
		ship->torpedos = URANGE( 0, atoi( argument ), 255 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "rockets" ) )
	{
		ship->maxrockets = URANGE( 0, atoi( argument ), 255 );
		ship->rockets = URANGE( 0, atoi( argument ), 255 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "speed" ) )
	{
		ship->realspeed = URANGE( 0, atoi( argument ), 1500 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "tractorbeam" ) )
	{
		ship->tractorbeam = URANGE( 0, atoi( argument ), 255 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "hyperspeed" ) )
	{
		ship->hyperspeed = URANGE( 0, atoi( argument ), 500 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "shield" ) )
	{
		ship->maxshield = URANGE( 0, atoi( argument ), 10000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "torsoarmor" ) )
	{
		ship->armor = URANGE( 1, atoi( argument ), 50000 );
		ship->maxarmor = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "headarmor" ) )
	{
		ship->armorhead = URANGE( 1, atoi( argument ), 50000 );
		ship->maxarmorhead = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "leftarmarmor" ) )
	{
		ship->armorlarm = URANGE( 1, atoi( argument ), 50000 );
		ship->maxarmorlarm = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "rightarmarmor" ) )
	{
		ship->armorrarm = URANGE( 1, atoi( argument ), 50000 );
		ship->maxarmorrarm = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "legarmor" ) )
	{
		ship->armorlegs = URANGE( 1, atoi( argument ), 50000 );
		ship->maxarmorlegs = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "dpow" ) )
	{
		ship->dpow = URANGE( 1, atoi( argument ), 5000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "parm" ) )
	{
		ship->parm = URANGE( 1, atoi( argument ), 5000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "weight" ) )
	{
		ship->weight = URANGE( 30, atoi( argument ), 100 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "frame" ) )
	{
		ship->frame = URANGE( 1, atoi( argument ), 50000 );
		ship->framemax = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "ap" ) )
	{
		ship->agility = URANGE( 1, atoi( argument ), 500 );
		ship->agilitymax = URANGE( 1, atoi( argument ), 500 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "agility" ) )
	{
		ship->speed = URANGE( 1, atoi( argument ), 500 );
		ship->speedmax = URANGE( 1, atoi( argument ), 500 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "framename" ) )
	{
		STRFREE( ship->framename );
		ship->framename = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "energy" ) )
	{
		ship->energy = URANGE( 1, atoi( argument ), 50000 );
		ship->maxenergy = URANGE( 1, atoi( argument ), 50000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "ammo" ) )
	{
		ship->ammo = URANGE( 1, atoi( argument ), 10000 );
		ship->maxammo = URANGE( 1, atoi( argument ), 10000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "sensor" ) )
	{
		ship->sensor = URANGE( 0, atoi( argument ), 10000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "astroarray" ) )
	{
		ship->astro_array = URANGE( 0, atoi( argument ), 1000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "comm" ) )
	{
		ship->comm = URANGE( 0, atoi( argument ), 1000 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "chaff" ) )
	{
		ship->chaff = URANGE( 0, atoi( argument ), 250 );
		ship->maxchaff = URANGE( 0, atoi( argument ), 250 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "flags" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: setship <ship> flags <flag> [flag]...\r\n", ch );
			send_to_char( "Flags:\r\n", ch );
			send_to_char( "Cloak\r\n", ch );
			return;
		}

		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			tempnum = get_shipflag( arg3 );

			if( tempnum < 0 || tempnum > 31 )
			{
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
				return;
			}
			TOGGLE_BIT( ship->flags, 1 << tempnum );
		}
		save_ship( ship );
		return;
	}

	do_setship( ch, "" );
	return;
}

CMDF( do_showship )
{
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: showship <ship>\r\n", ch );
		return;
	}

	ship = get_ship( argument );
	if( !ship )
	{
		send_to_char( "No such ship.\r\n", ch );
		return;
	}
	set_char_color( AT_YELLOW, ch );
	ch_printf( ch, "%s %s : %s\n\rFilename: %s\r\n",
		ship->type == SHIP_REPUBLIC ? "New Republic" :
		( ship->type == SHIP_IMPERIAL ? "Imperial" :
			( ship->type == SHIP_CIVILIAN ? "Civilian" : "Mob" ) ),
		ship->ship_class == MOBILE_SUIT ? "Mobile Suit" :
		( ship->ship_class == TRANSPORT_SHIP ? "Midship" :
			( ship->ship_class == CAPITAL_SHIP ? "Capital Ship" :
				( ship->ship_class == SHIP_PLATFORM ? "Platform" :
					( ship->ship_class == CLOUD_CAR ? "Cloudcar" :
						( ship->ship_class == OCEAN_SHIP ? "Boat" :
							( ship->ship_class == LAND_SPEEDER ? "Speeder" :
								( ship->ship_class == WHEELED ? "Wheeled Transport" :
									( ship->ship_class == LAND_CRAWLER ? "Crawler" :
										( ship->ship_class == WALKER ? "Walker" : "Unknown" ) ) ) ) ) ) ) ) ), ship->name, ship->filename );
	ch_printf( ch, "Home: %s   Description: %s\n\rOwner: %s   Pilot: %s   Copilot: %s Prevowner: %s Price: %d\r\n",
		ship->home, ship->description, ship->owner, ship->pilot, ship->copilot, ship->prevowner, ship->price );
	ch_printf( ch, "Firstroom: %d   Lastroom: %d", ship->firstroom, ship->lastroom );
	ch_printf( ch, "Cockpit: %d   Entrance: %d   Hanger: %d  Engineroom: %d\r\n",
		ship->cockpit, ship->entrance, ship->hanger, ship->engineroom );
	ch_printf( ch, "Pilotseat: %d   Coseat: %d   Navseat: %d  Gunseat: %d\r\n",
		ship->pilotseat, ship->coseat, ship->navseat, ship->gunseat );
	ch_printf( ch, "Location: %d   Lastdoc: %d   Shipyard: %d\r\n", ship->location, ship->lastdoc, ship->shipyard );
	ch_printf( ch, "Tractor Beam: %d   Comm: %d   Sensor: %d   Astro Array: %d\r\n",
		ship->tractorbeam, ship->comm, ship->sensor, ship->astro_array );
	ch_printf( ch, "Lasers: %d  Laser Condition: %s\r\n", ship->lasers, ship->statet0 == LASER_DAMAGED ? "Damaged" : "Good" );
	ch_printf( ch, "Turret One: %d  Condition: %s\r\n", ship->turret1, ship->statet1 == LASER_DAMAGED ? "Damaged" : "Good" );
	ch_printf( ch, "Turret Two: %d  Condition: %s\r\n", ship->turret2, ship->statet2 == LASER_DAMAGED ? "Damaged" : "Good" );
	ch_printf( ch, "Missiles: %d/%d  Torpedos: %d/%d  Rockets: %d/%d  Condition: %s\r\n",
		ship->missiles,
		ship->maxmissiles,
		ship->torpedos,
		ship->maxtorpedos,
		ship->rockets, ship->maxrockets, ship->missilestate == MISSILE_DAMAGED ? "Damaged" : "Good" );
	ch_printf( ch, "Torso Armor: %d/%d  Leg Armor: %d/%d  Ship Condition: %s\r\n",
		ship->armor,
		ship->maxarmor,
		ship->armorlegs, ship->maxarmorlegs, ship->shipstate == SHIP_DISABLED ? "Disabled" : "Running" );

	ch_printf( ch, "Head Armor: %d/%d  Left Arm Armor: %d/%d  Right Arm Armor: %d/%d",
		ship->armorhead,
		ship->maxarmorhead, ship->armorlarm, ship->maxarmorlarm, ship->armorrarm, ship->maxarmorrarm );

	ch_printf( ch, "\n\rFlags: %s", flag_string( ship->flags, ship_flags ) );

	ch_printf( ch, "\n\rShields: %d/%d   Energy(fuel): %d/%d   Chaff: %d/%d\r\n",
		ship->shield, ship->maxshield, ship->energy, ship->maxenergy, ship->chaff, ship->maxchaff );
	ch_printf( ch, "Current Coordinates: %.0f %.0f %.0f\r\n", ship->vx, ship->vy, ship->vz );
	ch_printf( ch, "Current Heading: %.0f %.0f %.0f\r\n", ship->hx, ship->hy, ship->hz );
	ch_printf( ch, "Speed: %d/%d   Hyperspeed: %d\n\rManueverability: %d  Targeted Part: %d  PrevTimer: %d\r\n",
		ship->currspeed, ship->realspeed, ship->hyperspeed, ship->manuever, ship->targettype, ship->prevtimer );

	ch_printf( ch, "&GAmmo: %d/%d  ", ship->ammo, ship->maxammo );
	ch_printf( ch, "&BType: %d  &zCode: %d   ", ship->stype, ship->code );
	ch_printf( ch, "&CMod: %s   &PAlloy: %d\r\n",
		ship->mod == 1 ? "Zero System" :
		ship->mod == 2 ? "Cloaking" :
		ship->mod == 3 ? "Transform" : ship->mod == 4 ? "Deathblow" : "None", ship->alloy );

	ch_printf( ch, "&wDPOW: %d   PARM %d\r\n", ship->dpow, ship->parm );

	ch_printf( ch, "&wFrame: %s(%d/%d)   Agility: %d/%d(max)\r\n",
		ship->framename, ship->frame, ship->framemax, ship->speed, ship->speedmax );

	ch_printf( ch, "Right Arm: %s (Type: %s   Av Dam: %d)\r\n", ship->wonename, suitweapon_type[ship->wonetype],
		ship->wonedam );
	ch_printf( ch, "Left Arm:  %s (Type: %s   Av Dam: %d)\r\n", ship->wtwoname, suitweapon_type[ship->wtwotype],
		ship->wtwodam );

	ch_printf( ch, "&YCargo: %d/%d, Cargo Type: %s \r\n", ship->cargo, ship->maxcargo, cargo_names[ship->cargotype] );

	ch_printf( ch, "&Rfirstweapon: %s\r\n",
		ship->firstweapon == 0 ? "100mm Machine Gun" :
		( ship->firstweapon == 1 ? "105mm Rifle" :
			( ship->firstweapon == 2 ? "Leo Bazooka" :
				( ship->firstweapon == 3 ? "Beam Sabre" :
					( ship->firstweapon == 4 ? "Shoulder Mounted Energy Cannon" :
						( ship->firstweapon == 5 ? "Side Mounted Missile Launcher" :
							( ship->firstweapon == 6 ? "Beam Rifle" :
								( ship->firstweapon == 7 ? "Laser Cannon" :
									( ship->firstweapon == 8 ? "Twin-Gattling Gun" :
										( ship->firstweapon == 9 ? "Beam Cannon" :
											( ship->firstweapon == 10 ? "Buster Rifle" :
												( ship->firstweapon == 11 ? "Head Vulcan" :
													( ship->firstweapon == 12 ? "Beam Scythe" :
														( ship->firstweapon == 13 ? "Buster Shield" :
															( ship->firstweapon == 14 ? "Beam Gattling" :
																( ship->firstweapon == 15 ? "Multi Blast" :
																	( ship->firstweapon == 16 ? "Army Knife" :
																		( ship->firstweapon == 17 ? "Shatols" :
																			( ship->firstweapon == 18 ? "Shoulder Missiles" :
																				( ship->firstweapon == 19 ? "Cross Crusher" :
																					( ship->firstweapon == 20 ? "Big Beam Sabre" :
																						( ship->firstweapon == 21 ? "Heat Rod" :
																							( ship->firstweapon == 22 ? "Beam Glaive" :
																								( ship->firstweapon == 23 ? "Dragon Fang" :
																									( ship->firstweapon == 24 ? "Flamethrower" :
																										( ship->firstweapon == 25 ? "Dober Gun" :
																											( ship->firstweapon == 26 ? "Short Blast" :
																												( ship->firstweapon == 27 ? "Long Blast" :
																													( ship->firstweapon == 28 ? "Small Beam Cannon" :
																														( ship->firstweapon == 29 ? "Beam Blade" :
																															( ship->firstweapon == 30 ? "Planet Denfensers" :
																																( ship->firstweapon == 31 ? "Libra Main Cannon" :
																																	( ship->firstweapon == 32 ? "Twin Beam Scythe" :
																																		( ship->firstweapon ==
																																			33 ? "Twin Buster Rifle" : ( ship->
																																				firstweapon ==
																																				34 ?
																																				"Mega Cannon"
																																				: ( ship->
																																					firstweapon
																																					==
																																					35 ?
																																					"Twin Beam Trident"
																																					: ( ship->
																																						firstweapon
																																						==
																																						39 ?
																																						"250mm Machine Gun"
																																						:
																																						"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );
	ch_printf( ch, "secondweapon: %s\r\n",
		ship->secondweapon == 0 ? "100mm Machine Gun" : ( ship->secondweapon ==
			1 ? "105mm Rifle" : ( ship->secondweapon ==
				2 ? "Leo Bazooka" : ( ship->
					secondweapon ==
					3 ? "Beam Sabre"
					: ( ship->
						secondweapon
						==
						4 ?
						"Shoulder Mounted Energy Cannon"
						: ( ship->
							secondweapon
							==
							5 ?
							"Side Mounted Missile Launcher"
							:
							( ship->
								secondweapon
								==
								6 ?
								"Beam Rifle"
								:
								( ship->
									secondweapon
									==
									7 ?
									"Laser Cannon"
									:
									( ship->
										secondweapon
										==
										8 ?
										"Twin-Gattling Gun"
										:
										( ship->
											secondweapon
											==
											9
											?
											"Beam Cannon"
											:
											( ship->
												secondweapon
												==
												10
												?
												"Buster Rifle"
												:
												( ship->
													secondweapon
													==
													11
													?
													"Head Vulcan"
													:
													( ship->
														secondweapon
														==
														12
														?
														"Beam Scythe"
														:
														( ship->
															secondweapon
															==
															13
															?
															"Buster Shield"
															:
															( ship->
																secondweapon
																==
																14
																?
																"Beam Gattling"
																:
																( ship->
																	secondweapon
																	==
																	15
																	?
																	"Multi Blast"
																	:
																	( ship->
																		secondweapon
																		==
																		16
																		?
																		"Army Knife"
																		:
																		( ship->
																			secondweapon
																			==
																			17
																			?
																			"Shatols"
																			:
																			( ship->
																				secondweapon
																				==
																				18
																				?
																				"Shoulder Missiles"
																				:
																				( ship->
																					secondweapon
																					==
																					19
																					?
																					"Cross Crusher"
																					:
																					( ship->
																						secondweapon
																						==
																						20
																						?
																						"Big Beam Sabre"
																						:
																						( ship->
																							secondweapon
																							==
																							21
																							?
																							"Heat Rod"
																							:
																							( ship->
																								secondweapon
																								==
																								22
																								?
																								"Beam Glaive"
																								:
																								( ship->
																									secondweapon
																									==
																									23
																									?
																									"Dragon Fang"
																									:
																									( ship->
																										secondweapon
																										==
																										24
																										?
																										"Flamethrower"
																										:
																										( ship->
																											secondweapon
																											==
																											25
																											?
																											"Dober Gun"
																											:
																											( ship->
																												secondweapon
																												==
																												26
																												?
																												"Short Blast"
																												:
																												( ship->
																													secondweapon
																													==
																													27
																													?
																													"Long Blast"
																													:
																													( ship->
																														secondweapon
																														==
																														28
																														?
																														"Small Beam Cannon"
																														:
																														( ship->
																															secondweapon
																															==
																															29
																															?
																															"Beam Blade"
																															:
																															( ship->
																																secondweapon
																																==
																																30
																																?
																																"Planet Denfensers"
																																:
																																( ship->
																																	secondweapon
																																	==
																																	31
																																	?
																																	"Libra Main Cannon"
																																	:
																																	( ship->
																																		secondweapon
																																		==
																																		32
																																		?
																																		"Twin Beam Scythe"
																																		:
																																		( ship->
																																			secondweapon
																																			==
																																			33
																																			?
																																			"Twin Buster Rifle"
																																			:
																																			( ship->
																																				secondweapon
																																				==
																																				34
																																				?
																																				"Mega Cannon"
																																				:
																																				( ship->
																																					secondweapon
																																					==
																																					35
																																					?
																																					"Twin Beam Trident"
																																					:
																																					( ship->
																																						secondweapon
																																						==
																																						39
																																						?
																																						"250mm Machine Gun"
																																						:
																																						"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );
	ch_printf( ch, "thirdweapon: %s\r\n",
		ship->thirdweapon == 0 ? "100mm Machine Gun" : ( ship->thirdweapon ==
			1 ? "105mm Rifle" : ( ship->thirdweapon ==
				2 ? "Leo Bazooka" : ( ship->
					thirdweapon ==
					3 ? "Beam Sabre"
					: ( ship->
						thirdweapon ==
						4 ?
						"Shoulder Mounted Energy Cannon"
						: ( ship->
							thirdweapon
							==
							5 ?
							"Side Mounted Missile Launcher"
							: ( ship->
								thirdweapon
								==
								6 ?
								"Beam Rifle"
								:
								( ship->
									thirdweapon
									==
									7 ?
									"Laser Cannon"
									:
									( ship->
										thirdweapon
										==
										8
										?
										"Twin-Gattling Gun"
										:
										( ship->
											thirdweapon
											==
											9
											?
											"Beam Cannon"
											:
											( ship->
												thirdweapon
												==
												10
												?
												"Buster Rifle"
												:
												( ship->
													thirdweapon
													==
													11
													?
													"Head Vulcan"
													:
													( ship->
														thirdweapon
														==
														12
														?
														"Beam Scythe"
														:
														( ship->
															thirdweapon
															==
															13
															?
															"Buster Shield"
															:
															( ship->
																thirdweapon
																==
																14
																?
																"Beam Gattling"
																:
																( ship->
																	thirdweapon
																	==
																	15
																	?
																	"Multi Blast"
																	:
																	( ship->
																		thirdweapon
																		==
																		16
																		?
																		"Army Knife"
																		:
																		( ship->
																			thirdweapon
																			==
																			17
																			?
																			"Shatols"
																			:
																			( ship->
																				thirdweapon
																				==
																				18
																				?
																				"Shoulder Missiles"
																				:
																				( ship->
																					thirdweapon
																					==
																					19
																					?
																					"Cross Crusher"
																					:
																					( ship->
																						thirdweapon
																						==
																						20
																						?
																						"Big Beam Sabre"
																						:
																						( ship->
																							thirdweapon
																							==
																							21
																							?
																							"Heat Rod"
																							:
																							( ship->
																								thirdweapon
																								==
																								22
																								?
																								"Beam Glaive"
																								:
																								( ship->
																									thirdweapon
																									==
																									23
																									?
																									"Dragon Fang"
																									:
																									( ship->
																										thirdweapon
																										==
																										24
																										?
																										"Flamethrower"
																										:
																										( ship->
																											thirdweapon
																											==
																											25
																											?
																											"Dober Gun"
																											:
																											( ship->
																												thirdweapon
																												==
																												26
																												?
																												"Short Blast"
																												:
																												( ship->
																													thirdweapon
																													==
																													27
																													?
																													"Long Blast"
																													:
																													( ship->
																														thirdweapon
																														==
																														28
																														?
																														"Small Beam Cannon"
																														:
																														( ship->
																															thirdweapon
																															==
																															29
																															?
																															"Beam Blade"
																															:
																															( ship->
																																thirdweapon
																																==
																																30
																																?
																																"Planet Denfensers"
																																:
																																( ship->
																																	thirdweapon
																																	==
																																	31
																																	?
																																	"Libra Main Cannon"
																																	:
																																	( ship->
																																		thirdweapon
																																		==
																																		32
																																		?
																																		"Twin Beam Scythe"
																																		:
																																		( ship->
																																			thirdweapon
																																			==
																																			33
																																			?
																																			"Twin Buster Rifle"
																																			:
																																			( ship->
																																				thirdweapon
																																				==
																																				34
																																				?
																																				"Mega Cannon"
																																				:
																																				( ship->
																																					thirdweapon
																																					==
																																					35
																																					?
																																					"Twin Beam Trident"
																																					:
																																					( ship->
																																						thirdweapon
																																						==
																																						39
																																						?
																																						"250mm Machine Gun"
																																						:
																																						"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );


	return;
}

CMDF( do_makeship )
{
	SHIP_DATA *ship;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeship <filename> <ship name>\r\n", ch );
		return;
	}

	CREATE( ship, SHIP_DATA, 1 );
	LINK( ship, first_ship, last_ship, next, prev );

	ship->name = STRALLOC( argument );
	ship->description = STRALLOC( "" );
	ship->owner = STRALLOC( "" );
	ship->copilot = STRALLOC( "" );
	ship->pilot = STRALLOC( "" );
	ship->prevowner = STRALLOC( "" );
	ship->home = STRALLOC( "" );
	ship->framename = STRALLOC( "VX-10000" );
	ship->wonename = STRALLOC( "Punch" );
	ship->wtwoname = STRALLOC( "Punch" );
	ship->type = SHIP_CIVILIAN;
	ship->starsystem = NULL;
	ship->energy = ship->maxenergy;
	ship->ammo = ship->maxammo;
	ship->armor = ship->maxarmor;
	ship->armorhead = ship->maxarmorhead;
	ship->armorlarm = ship->maxarmorlarm;
	ship->armorrarm = ship->maxarmorrarm;
	ship->armorlegs = ship->maxarmorlegs;
	ship->targettype = 0;
	ship->alloy = 0;
	ship->weight = 30;
	ship->offon = 0;
	ship->mod = 0;
	ship->code = 33;
	ship->price = 0;
	ship->autoboom = 0;
	ship->maxmines = 0;
	ship->bombs = 0;

	ship->maxcargo = 0;
	ship->cargo = 0;
	ship->cargotype = 0;

	ship->in_room = NULL;
	ship->next_in_room = NULL;
	ship->prev_in_room = NULL;
	ship->currjump = NULL;
	ship->target0 = NULL;
	ship->target1 = NULL;
	ship->target2 = NULL;

	ship->filename = str_dup( arg );
	save_ship( ship );
	write_ship_list( );

}

CMDF( do_copyship )
{
	SHIP_DATA *ship;
	SHIP_DATA *old;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: copyship <oldshipname> <filename> <newshipname>\r\n", ch );
		return;
	}

	old = get_ship( arg );

	if( !old )
	{
		send_to_char( "Thats not a ship!\r\n", ch );
		return;
	}

	CREATE( ship, SHIP_DATA, 1 );
	LINK( ship, first_ship, last_ship, next, prev );

	ship->name = STRALLOC( argument );
	ship->description = STRALLOC( "" );
	ship->owner = STRALLOC( "" );
	ship->copilot = STRALLOC( "" );
	ship->pilot = STRALLOC( "" );
	ship->prevowner = STRALLOC( "" );
	ship->home = STRALLOC( "" );
	ship->type = old->type;
	ship->ship_class = old->ship_class;
	ship->lasers = old->lasers;
	ship->maxmissiles = old->maxmissiles;
	ship->maxrockets = old->maxrockets;
	ship->maxtorpedos = old->maxtorpedos;
	ship->maxshield = old->maxshield;
	ship->maxarmor = old->maxarmor;
	ship->maxarmorhead = old->maxarmorhead;
	ship->maxarmorlarm = old->maxarmorlarm;
	ship->maxarmorrarm = old->maxarmorrarm;
	ship->maxarmorlegs = old->maxarmorlegs;
	ship->targettype = 0;
	ship->alloy = 0;
	ship->offon = 0;
	ship->mod = 0;
	ship->code = ship->code;
	ship->price = ship->price;

	ship->maxenergy = old->maxenergy;
	ship->maxammo = old->maxammo;
	ship->hyperspeed = old->hyperspeed;
	ship->maxchaff = old->maxchaff;
	ship->realspeed = old->realspeed;
	ship->manuever = old->manuever;
	ship->in_room = NULL;
	ship->next_in_room = NULL;
	ship->prev_in_room = NULL;
	ship->currjump = NULL;
	ship->target0 = NULL;
	ship->target1 = NULL;
	ship->target2 = NULL;

	ship->filename = STRALLOC( arg2 );
	save_ship( ship );
	write_ship_list( );
}

CMDF( do_ships )
{
	SHIP_DATA *ship;
	int count;

	if( !IS_NPC( ch ) )
	{
		count = 0;
		send_to_char( "&YThe following ships are owned by you or by your organization:\r\n", ch );
		send_to_char( "\r\n&WShip                               Owner\r\n", ch );
		for( ship = first_ship; ship; ship = ship->next )
		{
			if( str_cmp( ship->owner, ch->name ) )
			{
				if( !ch->pcdata || !ch->pcdata->clan || str_cmp( ship->owner, ch->pcdata->clan->name )
					|| ship->ship_class > SHIP_PLATFORM )
					continue;
			}

			if( ship->type == MOB_SHIP )
				continue;
			else if( ship->type == SHIP_REPUBLIC )
				set_char_color( AT_BLOOD, ch );
			else if( ship->type == SHIP_IMPERIAL )
				set_char_color( AT_DGREEN, ch );
			else
				set_char_color( AT_BLUE, ch );

			if( ship->in_room )
				ch_printf( ch, "%s - %s\r\n", ship->name, ship->in_room->name );
			else
				ch_printf( ch, "%s\r\n", ship->name );

			count++;
		}

		if( !count )
		{
			send_to_char( "There are no ships owned by you.\r\n", ch );
		}

	}


	count = 0;
	send_to_char( "&Y\n\rThe following ships are docked here:\r\n", ch );

	send_to_char( "\r\n&WShip                               Owner          Cost/Rent\r\n", ch );
	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->location != ch->in_room->vnum || ship->ship_class > SHIP_PLATFORM )
			continue;

		if( ship->type == MOB_SHIP )
			continue;
		else if( ship->type == SHIP_REPUBLIC )
			set_char_color( AT_BLOOD, ch );
		else if( ship->type == SHIP_IMPERIAL )
			set_char_color( AT_DGREEN, ch );
		else
			set_char_color( AT_BLUE, ch );

		ch_printf( ch, "%-35s %-15s", ship->name, ship->owner );
		if( ship->type == MOB_SHIP || ship->ship_class == SHIP_PLATFORM )
		{
			ch_printf( ch, "\r\n" );
			continue;
		}
		if( !str_cmp( ship->owner, "Public" ) )
		{
			ch_printf( ch, "%ld to rent.\r\n", get_ship_value( ship ) / 100 );
		}
		else if( str_cmp( ship->owner, "" ) )
			ch_printf( ch, "%s", "\r\n" );
		else
			ch_printf( ch, "%ld to buy.\r\n", get_ship_value( ship ) );

		count++;
	}

	if( !count )
	{
		send_to_char( "There are no ships docked here.\r\n", ch );
	}
}

CMDF( do_speeders )
{
	SHIP_DATA *ship;
	int count;

	if( !IS_NPC( ch ) )
	{
		count = 0;
		send_to_char( "&YThe following are owned by you or by your organization:\r\n", ch );
		send_to_char( "\r\n&WVehicle                            Owner\r\n", ch );
		for( ship = first_ship; ship; ship = ship->next )
		{
			if( str_cmp( ship->owner, ch->name ) )
			{
				if( !ch->pcdata || !ch->pcdata->clan || str_cmp( ship->owner, ch->pcdata->clan->name )
					|| ship->ship_class <= SHIP_PLATFORM )
					continue;
			}
			if( ship->location != ch->in_room->vnum || ship->ship_class <= SHIP_PLATFORM )
				continue;

			if( ship->type == MOB_SHIP )
				continue;
			else if( ship->type == SHIP_REPUBLIC )
				set_char_color( AT_BLOOD, ch );
			else if( ship->type == SHIP_IMPERIAL )
				set_char_color( AT_DGREEN, ch );
			else
				set_char_color( AT_BLUE, ch );

			ch_printf( ch, "%-35s %-15s\r\n", ship->name, ship->owner );

			count++;
		}

		if( !count )
		{
			send_to_char( "There are no land or air vehicles owned by you.\r\n", ch );
		}

	}


	count = 0;
	send_to_char( "&Y\n\rThe following vehicles are parked here:\r\n", ch );

	send_to_char( "\r\n&WVehicle                            Owner          Cost/Rent\r\n", ch );
	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->location != ch->in_room->vnum || ship->ship_class <= SHIP_PLATFORM )
			continue;

		if( ship->type == MOB_SHIP )
			continue;
		else if( ship->type == SHIP_REPUBLIC )
			set_char_color( AT_BLOOD, ch );
		else if( ship->type == SHIP_IMPERIAL )
			set_char_color( AT_DGREEN, ch );
		else
			set_char_color( AT_BLUE, ch );


		ch_printf( ch, "%-35s %-15s", ship->name, ship->owner );

		if( !str_cmp( ship->owner, "Public" ) )
		{
			ch_printf( ch, "%ld to rent.\r\n", get_ship_value( ship ) / 100 );
		}
		else if( str_cmp( ship->owner, "" ) )
			ch_printf( ch, "%s", "\r\n" );
		else
			ch_printf( ch, "%ld to buy.\r\n", get_ship_value( ship ) );

		count++;
	}

	if( !count )
	{
		send_to_char( "There are no sea air or land vehicles here.\r\n", ch );
	}
}

CMDF( do_allspeeders )
{
	SHIP_DATA *ship;
	int count = 0;

	count = 0;
	send_to_char( "&Y\n\rThe following sea/land/air vehicles are currently formed:\r\n", ch );

	send_to_char( "\r\n&WVehicle                            Owner\r\n", ch );
	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->ship_class <= SHIP_PLATFORM )
			continue;

		if( ship->type == MOB_SHIP )
			continue;
		else if( ship->type == SHIP_REPUBLIC )
			set_char_color( AT_BLOOD, ch );
		else if( ship->type == SHIP_IMPERIAL )
			set_char_color( AT_DGREEN, ch );
		else
			set_char_color( AT_BLUE, ch );


		ch_printf( ch, "%-35s %-15s ", ship->name, ship->owner );

		if( !str_cmp( ship->owner, "Public" ) )
		{
			ch_printf( ch, "%ld to rent.\r\n", get_ship_value( ship ) / 100 );
		}
		else if( str_cmp( ship->owner, "" ) )
			ch_printf( ch, "%s", "\r\n" );
		else
			ch_printf( ch, "%ld to buy.\r\n", get_ship_value( ship ) );

		count++;
	}

	if( !count )
	{
		send_to_char( "There are none currently formed.\r\n", ch );
		return;
	}

}

CMDF( do_allships )
{
	SHIP_DATA *ship;
	int count = 0;

	count = 0;
	send_to_char( "&Y\n\rThe following ships are currently formed:\r\n", ch );

	send_to_char( "\r\n&WShip/Suit                          Owner\r\n", ch );

	if( IS_IMMORTAL( ch ) )
		for( ship = first_ship; ship; ship = ship->next )
			if( ship->type == MOB_SHIP )
				ch_printf( ch, "&w%-25s %-15s \r\n", ship->name, ship->owner );


	for( ship = first_ship; ship; ship = ship->next )
	{
		if( ship->ship_class > SHIP_PLATFORM )
			continue;

		if( ship->type == MOB_SHIP )
			continue;
		else if( ship->type == SHIP_REPUBLIC )
			set_char_color( AT_BLOOD, ch );
		else if( ship->type == SHIP_IMPERIAL )
			set_char_color( AT_DGREEN, ch );
		else
			set_char_color( AT_BLUE, ch );

		ch_printf( ch, "%-25s %-15s ", ship->name, ship->owner );

		if( ship->type == MOB_SHIP || ship->ship_class == SHIP_PLATFORM )
		{
			ch_printf( ch, "\r\n" );
			continue;
		}
		if( !str_cmp( ship->owner, "Public" ) )
		{
			ch_printf( ch, "%ld to rent.\r\n", get_ship_value( ship ) / 100 );
		}
		else if( str_cmp( ship->owner, "" ) )
			ch_printf( ch, "%s", "\r\n" );
		else
			ch_printf( ch, "%ld to buy.\r\n", get_ship_value( ship ) );

		count++;
	}

	if( !count )
	{
		send_to_char( "There are no ships currently formed.\r\n", ch );
		return;
	}

}


void ship_to_starsystem( SHIP_DATA *ship, SPACE_DATA *starsystem )
{
	if( starsystem == NULL )
		return;

	if( ship == NULL )
		return;

	if( starsystem->first_ship == NULL )
		starsystem->first_ship = ship;

	if( starsystem->last_ship )
	{
		starsystem->last_ship->next_in_starsystem = ship;
		ship->prev_in_starsystem = starsystem->last_ship;
	}

	starsystem->last_ship = ship;

	ship->starsystem = starsystem;

}

void new_missile( SHIP_DATA *ship, SHIP_DATA *target, CHAR_DATA *ch, int missiletype )
{
	SPACE_DATA *starsystem;
	MISSILE_DATA *missile;

	if( ship == NULL )
		return;

	if( target == NULL )
		return;

	if( ( starsystem = ship->starsystem ) == NULL )
		return;

	CREATE( missile, MISSILE_DATA, 1 );
	LINK( missile, first_missile, last_missile, next, prev );

	missile->target = target;
	missile->fired_from = ship;
	if( ch )
		missile->fired_by = STRALLOC( ch->name );
	else
		missile->fired_by = STRALLOC( "" );
	missile->missiletype = missiletype;
	missile->age = 0;
	if( missile->missiletype == HEAVY_BOMB )
		missile->speed = 20;
	else if( missile->missiletype == PROTON_TORPEDO )
		missile->speed = 200;
	else if( missile->missiletype == CONCUSSION_MISSILE )
		missile->speed = 300;
	else
		missile->speed = 50;

	missile->mx = ship->vx;
	missile->my = ship->vy;
	missile->mz = ship->vz;

	if( starsystem->first_missile == NULL )
		starsystem->first_missile = missile;

	if( starsystem->last_missile )
	{
		starsystem->last_missile->next_in_starsystem = missile;
		missile->prev_in_starsystem = starsystem->last_missile;
	}

	starsystem->last_missile = missile;

	missile->starsystem = starsystem;

}

void ship_from_starsystem( SHIP_DATA *ship, SPACE_DATA *starsystem )
{

	if( starsystem == NULL )
		return;

	if( ship == NULL )
		return;

	if( starsystem->last_ship == ship )
		starsystem->last_ship = ship->prev_in_starsystem;

	if( starsystem->first_ship == ship )
		starsystem->first_ship = ship->next_in_starsystem;

	if( ship->prev_in_starsystem )
		ship->prev_in_starsystem->next_in_starsystem = ship->next_in_starsystem;

	if( ship->next_in_starsystem )
		ship->next_in_starsystem->prev_in_starsystem = ship->prev_in_starsystem;

	ship->starsystem = NULL;
	ship->next_in_starsystem = NULL;
	ship->prev_in_starsystem = NULL;

}

void extract_missile( MISSILE_DATA *missile )
{
	SPACE_DATA *starsystem;

	if( missile == NULL )
		return;

	if( ( starsystem = missile->starsystem ) != NULL )
	{

		if( starsystem->last_missile == missile )
			starsystem->last_missile = missile->prev_in_starsystem;

		if( starsystem->first_missile == missile )
			starsystem->first_missile = missile->next_in_starsystem;

		if( missile->prev_in_starsystem )
			missile->prev_in_starsystem->next_in_starsystem = missile->next_in_starsystem;

		if( missile->next_in_starsystem )
			missile->next_in_starsystem->prev_in_starsystem = missile->prev_in_starsystem;

		missile->starsystem = NULL;
		missile->next_in_starsystem = NULL;
		missile->prev_in_starsystem = NULL;

	}

	UNLINK( missile, first_missile, last_missile, next, prev );

	missile->target = NULL;
	missile->fired_from = NULL;
	STRFREE( missile->fired_by );

	DISPOSE( missile );

}

bool is_rental( CHAR_DATA *ch, SHIP_DATA *ship )
{
	if( !str_cmp( "Public", ship->owner ) )
		return true;
	if( !str_cmp( "Holosuit", ship->owner ) )
		return true;

	return false;
}

bool check_pilot( CHAR_DATA *ch, SHIP_DATA *ship )
{
	if( !str_cmp( ch->name, ship->owner ) || !str_cmp( ch->name, ship->pilot )
		|| !str_cmp( ch->name, ship->copilot ) || !str_cmp( "Public", ship->owner ) || !str_cmp( "Holosuit", ship->owner ) )
		return true;

	if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan )
	{
		if( !str_cmp( ch->pcdata->clan->name, ship->owner ) )
		{
			if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
				return true;
			if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
				return true;
			if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
				return true;
			if( ch->pcdata->bestowments && is_name( "pilot", ch->pcdata->bestowments ) )
				return true;
		}
	}

	return false;
}

bool extract_ship( SHIP_DATA *ship )
{
	ROOM_INDEX_DATA *room;

	if( ( room = ship->in_room ) != NULL )
	{
		UNLINK( ship, room->first_ship, room->last_ship, next_in_room, prev_in_room );
		ship->in_room = NULL;
	}
	return true;
}

void damage_ship_chhead( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch )
{
	int damage;
	long xp;

	if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP ) )
	{
		send_to_char( "&RThats a ship, it has no limbs to target!. \r\n", ch );
		do_targetlock( ch, "none" );
		return;
	}

	if( ship->armorhead < 1 )
	{
		ch_printf( ch, "&R%s's head is already destroyed! Pick a different target!\r\n", ship->name );
		do_targetlock( ch, "none" );
		return;
	}

	damage = number_range( min, max );

	xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 60;
	xp = UMIN( get_ship_value( ship ) / 1000, xp );
	gain_exp( ch, xp, PILOTING_ABILITY );

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ship->alloy == 0 )
			ship->armorhead -= damage * 9.5;
		else if( ship->alloy == 1 )
			ship->armorhead -= damage * 9.5;
		else if( ship->alloy == 2 )
			ship->armorhead -= damage * 9;
		else if( ship->alloy == 3 )
			ship->armorhead -= damage * 8.5;
		else if( ship->alloy == 4 )
			ship->armorhead -= damage * 8;
		else if( ship->alloy == 5 )
			ship->armorhead -= damage * 7.5;
		else
			ship->armorhead -= damage * 7;
	}
	else
	{
		if( ship->alloy == 0 )
			ship->armorhead -= damage * 7.5;
		else if( ship->alloy == 1 )
			ship->armorhead -= damage * 7.5;
		else if( ship->alloy == 2 )
			ship->armorhead -= damage * 7;
		else if( ship->alloy == 3 )
			ship->armorhead -= damage * 6.5;
		else if( ship->alloy == 4 )
			ship->armorhead -= damage * 6;
		else if( ship->alloy == 5 )
			ship->armorhead -= damage * 5.5;
		else
			ship->armorhead -= damage * 5;
	}

	if( ship->armorhead <= 0 )
	{
		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) / 80 );
		xp = UMIN( get_ship_value( ship ) / 50, xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&GYou blow off the head of %s!\r\n&WYou gain %ld piloting experience!\r\n", ship->name, xp );
		send_to_char( "&RPick a new body part to target!\r\n", ch );
		ship->armorhead = 0;
		do_targetlock( ch, "none" );
		echo_to_cockpit( AT_BLOOD, ship, "The Head of your suit has been blown off!" );

		return;
	}

	if( ship->armorhead <= ship->maxarmorhead / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Suit head severely damaged!" );

}

void damage_ship_chlarm( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch )
{
	int damage;
	long xp;

	if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP ) )
	{
		send_to_char( "&RThats a ship, it has no limbs to target!. \r\n", ch );
		do_targetlock( ch, "none" );
		return;
	}

	if( ship->armorlarm < 1 )
	{
		ch_printf( ch, "&R%s's left arm is already destroyed! Pick a different target!\r\n", ship->name );
		do_targetlock( ch, "none" );
		return;
	}

	damage = number_range( min, max );

	xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 60;
	xp = UMIN( get_ship_value( ship ) / 100, xp );
	gain_exp( ch, xp, PILOTING_ABILITY );

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ship->alloy == 0 )
			ship->armorlarm -= damage * 9.5;
		else if( ship->alloy == 1 )
			ship->armorlarm -= damage * 9.5;
		else if( ship->alloy == 2 )
			ship->armorlarm -= damage * 9;
		else if( ship->alloy == 3 )
			ship->armorlarm -= damage * 8.5;
		else if( ship->alloy == 4 )
			ship->armorlarm -= damage * 8;
		else if( ship->alloy == 5 )
			ship->armorlarm -= damage * 7.5;
		else
			ship->armorlarm -= damage * 7;
	}
	else
	{
		if( ship->alloy == 0 )
			ship->armorlarm -= damage * 7.5;
		else if( ship->alloy == 1 )
			ship->armorlarm -= damage * 7.5;
		else if( ship->alloy == 2 )
			ship->armorlarm -= damage * 7;
		else if( ship->alloy == 3 )
			ship->armorlarm -= damage * 6.5;
		else if( ship->alloy == 4 )
			ship->armorlarm -= damage * 6;
		else if( ship->alloy == 5 )
			ship->armorlarm -= damage * 5.5;
		else
			ship->armorlarm -= damage * 5;
	}

	if( ship->armorlarm <= 0 )
	{
		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) / 80 );
		xp = UMIN( get_ship_value( ship ) / 50, xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&GYou blow off the left arm of %s!\r\n&WYou gain %ld piloting experience!\r\n", ship->name, xp );
		send_to_char( "&RPick a new body part to target!\r\n", ch );
		ship->armorlarm = 0;
		do_targetlock( ch, "none" );
		echo_to_cockpit( AT_BLOOD, ship, "The left arm of your suit has been blown off!" );

		return;
	}

	if( ship->armorlarm <= ship->maxarmorlarm / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Suit left arm severely damaged!" );

}


void damage_ship_chrarm( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch )
{
	int damage;
	long xp;

	if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP ) )
	{
		send_to_char( "&RThats a ship, it has no limbs to target!. \r\n", ch );
		do_targetlock( ch, "none" );
		return;
	}

	if( ship->armorrarm < 1 )
	{
		ch_printf( ch, "&R%s's right arm is already destroyed! Pick a different target!\r\n", ship->name );
		do_targetlock( ch, "none" );
		return;
	}

	damage = number_range( min, max );

	xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 60;
	xp = UMIN( get_ship_value( ship ) / 100, xp );
	gain_exp( ch, xp, PILOTING_ABILITY );

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ship->alloy == 0 )
			ship->armorrarm -= damage * 9.5;
		else if( ship->alloy == 1 )
			ship->armorrarm -= damage * 9.5;
		else if( ship->alloy == 2 )
			ship->armorrarm -= damage * 9;
		else if( ship->alloy == 3 )
			ship->armorrarm -= damage * 8.5;
		else if( ship->alloy == 4 )
			ship->armorrarm -= damage * 8;
		else if( ship->alloy == 5 )
			ship->armorrarm -= damage * 7.5;
		else
			ship->armorrarm -= damage * 7;
	}
	else
	{
		if( ship->alloy == 0 )
			ship->armorrarm -= damage * 7.5;
		else if( ship->alloy == 1 )
			ship->armorrarm -= damage * 7.5;
		else if( ship->alloy == 2 )
			ship->armorrarm -= damage * 7;
		else if( ship->alloy == 3 )
			ship->armorrarm -= damage * 6.5;
		else if( ship->alloy == 4 )
			ship->armorrarm -= damage * 6;
		else if( ship->alloy == 5 )
			ship->armorrarm -= damage * 5.5;
		else
			ship->armorrarm -= damage * 5;
	}


	if( ship->armorrarm <= 0 )
	{
		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) / 80 );
		xp = UMIN( get_ship_value( ship ) / 50, xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&GYou blow off the right arm of %s!\r\n&WYou gain %ld piloting experience!\r\n", ship->name, xp );
		send_to_char( "&RPick a new body part to target!\r\n", ch );
		ship->armorrarm = 0;
		do_targetlock( ch, "none" );
		echo_to_cockpit( AT_BLOOD, ship, "The right arm of your suit has been blown off!" );

		return;
	}

	if( ship->armorrarm <= ship->maxarmorrarm / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Right arm severely damaged!" );

}


void damage_ship_chlegs( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch )
{
	int damage;
	long xp;

	if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP ) )
	{
		send_to_char( "&RThats a ship, it has no limbs to target!. \r\n", ch );
		do_targetlock( ch, "none" );
		return;
	}

	if( ship->armorlegs < 1 )
	{
		ch_printf( ch, "&R%s's legs are already destroyed! Pick a different target!\r\n", ship->name );
		do_targetlock( ch, "none" );
		return;
	}

	damage = number_range( min, max );

	xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 60;
	xp = UMIN( get_ship_value( ship ) / 100, xp );
	gain_exp( ch, xp, PILOTING_ABILITY );

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ship->alloy == 0 )
			ship->armorlegs -= damage * 9.5;
		else if( ship->alloy == 1 )
			ship->armorlegs -= damage * 9.5;
		else if( ship->alloy == 2 )
			ship->armorlegs -= damage * 9;
		else if( ship->alloy == 3 )
			ship->armorlegs -= damage * 8.5;
		else if( ship->alloy == 4 )
			ship->armorlegs -= damage * 8;
		else if( ship->alloy == 5 )
			ship->armorlegs -= damage * 7.5;
		else
			ship->armorlegs -= damage * 7;
	}
	else
	{
		if( ship->alloy == 0 )
			ship->armorlegs -= damage * 7.5;
		else if( ship->alloy == 1 )
			ship->armorlegs -= damage * 7.5;
		else if( ship->alloy == 2 )
			ship->armorlegs -= damage * 7;
		else if( ship->alloy == 3 )
			ship->armorlegs -= damage * 6.5;
		else if( ship->alloy == 4 )
			ship->armorlegs -= damage * 6;
		else if( ship->alloy == 5 )
			ship->armorlegs -= damage * 5.5;
		else
			ship->armorlegs -= damage * 5;
	}

	if( ship->armorlegs <= 0 )
	{
		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) / 80 );
		xp = UMIN( get_ship_value( ship ) / 50, xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&GYou blow off the legs of %s!\r\n&WYou gain %ld piloting experience!\r\n", ship->name, xp );
		send_to_char( "&RPick a new body part to target!\r\n", ch );
		ship->armorlegs = 0;
		ship->currspeed = 0;
		do_targetlock( ch, "none" );
		echo_to_cockpit( AT_BLOOD, ship, "The legs of your suit have been blown off!" );

		return;
	}

	if( ship->armorlegs <= ship->maxarmorlegs / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Suit legs severely damaged!" );

}

void damage_ship_ch( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch )
{
	int damage, shield_dmg;
	long xp;

	damage = number_range( min, max );

	xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 25;
	xp = UMIN( get_ship_value( ship ) / 100, xp );
	gain_exp( ch, xp, PILOTING_ABILITY );

	if( ship->shield > 0 )
	{
		shield_dmg = UMIN( ship->shield, damage );
		damage -= shield_dmg;
		ship->shield -= shield_dmg;
		if( ship->shield == 0 )
			echo_to_cockpit( AT_BLOOD, ship, "Shields drained..." );
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		if( ship->alloy == 0 )
			ship->armor -= damage * 9.5;
		else if( ship->alloy == 1 )
			ship->armor -= damage * 9.5;
		else if( ship->alloy == 2 )
			ship->armor -= damage * 9;
		else if( ship->alloy == 3 )
			ship->armor -= damage * 8.5;
		else if( ship->alloy == 4 )
			ship->armor -= damage * 8;
		else if( ship->alloy == 5 )
			ship->armor -= damage * 7.5;
		else
			ship->armor -= damage * 7;
	}
	else
	{
		if( ship->alloy == 0 )
			ship->armor -= damage * 7.5;
		else if( ship->alloy == 1 )
			ship->armor -= damage * 7.5;
		else if( ship->alloy == 2 )
			ship->armor -= damage * 7;
		else if( ship->alloy == 3 )
			ship->armor -= damage * 6.5;
		else if( ship->alloy == 4 )
			ship->armor -= damage * 6;
		else if( ship->alloy == 5 )
			ship->armor -= damage * 5.5;
		else
			ship->armor -= damage * 5;
	}

	if( ship->armor <= 0 )
	{
		destroy_ship( ship, ch );

		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) );
		xp = UMIN( get_ship_value( ship ), xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&WYou gain %ld piloting experience!\r\n", xp );
		return;
	}

	if( ship->armor <= ship->maxarmor / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Ship armor severely damaged!" );

}

void damage_ship( SHIP_DATA *ship, int min, int max )
{
	int damage, shield_dmg;

	damage = number_range( min, max );

	if( ship->shield > 0 )
	{
		shield_dmg = UMIN( ship->shield, damage );
		damage -= shield_dmg;
		ship->shield -= shield_dmg;
		if( ship->shield == 0 )
			echo_to_cockpit( AT_BLOOD, ship, "Shields down..." );
	}

	ship->armor -= damage * 5;

	if( ship->armor <= 0 )
	{
		destroy_ship( ship, NULL );
		return;
	}

	if( ship->armor <= ship->maxarmor / 20 )
		echo_to_cockpit( AT_BLOOD + AT_BLINK, ship, "WARNING! Ship armor severely damaged!" );

}

void destroy_ship( SHIP_DATA *ship, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	int roomnum;
	ROOM_INDEX_DATA *room;
	OBJ_DATA *robj;
	CHAR_DATA *rch;

	if( !str_cmp( "Holosuit", ship->owner ) )
	{
		echo_to_ship( AT_WHITE, ship, "&GThe image of space and battle flickers out as your fight comes to an end.\r\n" );
		snprintf( buf, MAX_STRING_LENGTH, "The image of %s flickers out of sight.", ship->name );
		echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
		resetship( ship );
		return;
	}

	snprintf( buf, MAX_STRING_LENGTH, "%s explodes in a blinding flash of light!", ship->name );
	echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );

	if( ship->ship_class == MOBILE_SUIT )

		echo_to_ship( AT_WHITE + AT_BLINK, ship, "A blinding flash of light burns your eyes..." );
	echo_to_ship( AT_WHITE, ship,
		"But before you have a chance to scream...\n\rYou are ripped apart as your spacecraft explodes..." );

	for( roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++ )
	{
		room = get_room_index( roomnum );

		if( room != NULL )
		{
			rch = room->first_person;
			while( rch )
			{
				if( IS_IMMORTAL( rch ) )
				{
					char_from_room( rch );
					char_to_room( rch, get_room_index( wherehome( rch ) ) );
					do_look( rch, "auto" );
				}
				else
				{
					if( ch )
						raw_kill( ch, rch );
					else
						raw_kill( rch, rch );
				}
				rch = room->first_person;
			}

			for( robj = room->first_content; robj; robj = robj->next_content )
			{
				separate_obj( robj );
				if( !IS_OBJ_STAT( robj, ITEM_ARTIFACT ) )
				{
					extract_obj( robj );
				}
			}
		}

	}

	if( ship->mod == 3 )
	{
		ship->mod = 0;
		if( ship->offon == 1 )
		{
			ship->offon = 0;
			ship->realspeed = ship->realspeed / 2;
			ship->hyperspeed = ship->hyperspeed / 2;
		}
	}

	resetship( ship );
	return;
}

bool ship_to_room( SHIP_DATA *ship, int vnum )
{
	ROOM_INDEX_DATA *shipto;

	if( ( shipto = get_room_index( vnum ) ) == NULL )
		return false;
	LINK( ship, shipto->first_ship, shipto->last_ship, next_in_room, prev_in_room );
	ship->in_room = shipto;
	return true;
}


CMDF( do_board )
{
	ROOM_INDEX_DATA *toroom;
	SHIP_DATA *ship;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Board what?\r\n", ch );
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

	if( ( toroom = get_room_index( ship->entrance ) ) != NULL )
	{
		if( !ship->hatchopen )
		{
			send_to_char( "&RThe hatch is closed!\r\n", ch );
			return;
		}

		if( toroom->tunnel > 0 )
		{
			CHAR_DATA *ctmp;
			int count = 0;

			for( ctmp = toroom->first_person; ctmp; ctmp = ctmp->next_in_room )
				if( ++count >= toroom->tunnel )
				{
					send_to_char( "There is no room for you in there.\r\n", ch );
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
		char_to_room( ch, toroom );
		act( AT_PLAIN, "$n enters the ship.", ch, NULL, argument, TO_ROOM );
		do_look( ch, "auto" );

	}
	else
		send_to_char( "That ship has no entrance!\r\n", ch );
}

bool rent_ship( CHAR_DATA *ch, SHIP_DATA *ship )
{

	long price;

	if( IS_NPC( ch ) )
		return false;

	price = get_ship_value( ship ) / 100;

	if( ch->gold < price )
	{
		ch_printf( ch, "&RRenting this ship costs %ld. You don't have enough money!\r\n", price );
		return false;
	}

	ch->gold -= price;
	ch_printf( ch, "&GYou pay %ld dollars to rent the ship.\r\n", price );
	return true;

}

CMDF( do_leaveship )
{
	ROOM_INDEX_DATA *fromroom;
	ROOM_INDEX_DATA *toroom;
	SHIP_DATA *ship;

	fromroom = ch->in_room;

	if( ( ship = ship_from_entrance( fromroom->vnum ) ) == NULL )
	{
		send_to_char( "I see no exit here.\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( ship->lastdoc != ship->location )
	{
		send_to_char( "&rMaybe you should wait until the ship lands.\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "&rPlease wait till the ship is properly docked.\r\n", ch );
		return;
	}

	if( !ship->hatchopen )
	{
		send_to_char( "&RYou need to open the hatch first", ch );
		return;
	}

	if( ( toroom = get_room_index( ship->location ) ) != NULL )
	{
		act( AT_PLAIN, "$n exits the ship.", ch, NULL, argument, TO_ROOM );
		act( AT_PLAIN, "You exit the ship.", ch, NULL, argument, TO_CHAR );
		char_from_room( ch );
		char_to_room( ch, toroom );
		act( AT_PLAIN, "$n steps out of a ship.", ch, NULL, argument, TO_ROOM );
		do_look( ch, "auto" );
	}
	else
		send_to_char( "The exit doesn't seem to be working properly.\r\n", ch );
}

CMDF( do_launch )
{
	int chance;
	long price = 0;
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];
	int rand;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RThe ship is set on autopilot, you'll have to turn it off first.\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship! Try renting a public one.\r\n", ch );
		return;
	}

	if( ship->lastdoc != ship->location )
	{
		send_to_char( "&rYou don't seem to be docked right now.\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "The ship is not docked right now.\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( ship->ship_class == CAPITAL_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_capitalships] );
	if( number_percent( ) < chance )
	{
		if( is_rental( ch, ship ) )
			if( !rent_ship( ch, ship ) )
				return;
		if( !is_rental( ch, ship ) )
		{
			if( ship->ship_class == MOBILE_SUIT )
				price = 20;
			if( ship->ship_class == TRANSPORT_SHIP )
				price = 50;
			if( ship->ship_class == CAPITAL_SHIP )
				price = 500;

			price += ( ship->maxarmor - ship->armor );
			if( ship->missiles )
				price += ( 50 * ( ship->maxmissiles - ship->missiles ) );
			else if( ship->torpedos )
				price += ( 75 * ( ship->maxtorpedos - ship->torpedos ) );
			else if( ship->rockets )
				price += ( 150 * ( ship->maxrockets - ship->rockets ) );

			if( ship->shipstate == SHIP_DISABLED )
				price += 200;
			if( ship->missilestate == MISSILE_DAMAGED )
				price += 100;
			if( ship->statet0 == LASER_DAMAGED )
				price += 50;
			if( ship->statet1 == LASER_DAMAGED )
				price += 50;
			if( ship->statet2 == LASER_DAMAGED )
				price += 50;
		}

		if( ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
		{
			if( ch->pcdata->clan->funds < price )
			{
				ch_printf( ch, "&R%s doesn't have enough funds to prepare this ship for launch.\r\n", ch->pcdata->clan->name );
				return;
			}

			ch->pcdata->clan->funds -= price;
			ch_printf( ch, "&GIt costs %s %ld dollars to ready this ship for launch.\r\n", ch->pcdata->clan->name, price );
		}
		else if( str_cmp( ship->owner, "Public" ) || ( str_cmp( ship->owner, "Holosuit" ) ) )
		{
			if( ch->gold < price )
			{
				ch_printf( ch, "&RYou don't have enough funds to prepare this ship for launch.\r\n" );
				return;
			}

			ch->gold -= price;
			ch_printf( ch, "&GYou pay %ld dollars to ready the ship for launch.\r\n", price );

		}

		ship->energy = ship->maxenergy;
		ship->ammo = ship->maxammo;
		ship->chaff = ship->maxchaff;
		ship->missiles = ship->maxmissiles;
		ship->torpedos = ship->maxtorpedos;
		ship->rockets = ship->maxrockets;
		ship->shield = 0;
		ship->autorecharge = false;
		ship->autotrack = false;
		ship->autospeed = false;
		ship->armor = ship->maxarmor;
		ship->armorhead = ship->maxarmorhead;
		ship->armorlarm = ship->maxarmorlarm;
		ship->armorrarm = ship->maxarmorrarm;
		ship->armorlegs = ship->maxarmorlegs;

		ship->missilestate = MISSILE_READY;
		ship->statet0 = LASER_READY;
		ship->statet1 = LASER_READY;
		ship->statet2 = LASER_READY;
		ship->shipstate = SHIP_DOCKED;

		if( ship->hatchopen )
		{
			ship->hatchopen = false;
			snprintf( buf, MAX_STRING_LENGTH, "The hatch on %s closes.", ship->name );
			echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
			echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch slides shut." );
		}

		if( xIS_SET( ch->act, PLR_PKER ) )
		{
			ship->pksuit = 1;
		}
		else if( !str_cmp( "Holosuit", ship->owner ) )
		{
			ship->pksuit = 1;
		}
		else
		{
			ship->pksuit = 0;
		}
		rand = number_range( 10, 99 );
		ship->code = rand;
		set_char_color( AT_GREEN, ch );
		send_to_char( "Launch sequence initiated.\r\n", ch );
		act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch, NULL, argument, TO_ROOM );
		echo_to_ship( AT_YELLOW, ship, "The ship hums as it lifts off the ground." );
		snprintf( buf, MAX_STRING_LENGTH, "%s begins to launch.", ship->name );
		echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
		ship->shipstate = SHIP_LAUNCH;
		ship->currspeed = ship->realspeed;
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_success( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_success( ch, gsn_midships );
		if( ship->ship_class == CAPITAL_SHIP )
			learn_from_success( ch, gsn_capitalships );
		return;
	}
	set_char_color( AT_RED, ch );
	send_to_char( "You fail to work the controls properly!\r\n", ch );
	if( ship->ship_class == MOBILE_SUIT )
		learn_from_failure( ch, gsn_mobilesuits );
	if( ship->ship_class == TRANSPORT_SHIP )
		learn_from_failure( ch, gsn_midships );
	if( ship->ship_class == CAPITAL_SHIP )
		learn_from_failure( ch, gsn_capitalships );
	return;

}

void launchship( SHIP_DATA *ship )
{
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *target;
	int plusminus;
	int percent;
	percent = number_range( 0, 100 );

	ship_to_starsystem( ship, starsystem_from_vnum( ship->location ) );


	if( ship->starsystem == NULL )
	{
		echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Launch path blocked .. Launch aborted." );
		echo_to_ship( AT_YELLOW, ship, "The ship slowly sets back back down on the landing pad." );
		snprintf( buf, MAX_STRING_LENGTH, "%s slowly sets back down.", ship->name );
		echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
		ship->shipstate = SHIP_DOCKED;
		return;
	}

	extract_ship( ship );

	ship->location = 0;

	if( ship->shipstate != SHIP_DISABLED )
		ship->shipstate = SHIP_READY;

	plusminus = number_range( -1, 2 );
	if( plusminus > 0 )
		ship->hx = 1;
	else
		ship->hx = -1;

	plusminus = number_range( -1, 2 );
	if( plusminus > 0 )
		ship->hy = 1;
	else
		ship->hy = -1;

	plusminus = number_range( -1, 2 );
	if( plusminus > 0 )
		ship->hz = 1;
	else
		ship->hz = -1;

	if( ship->lastdoc == ship->starsystem->doc1a ||
		ship->lastdoc == ship->starsystem->doc1b || ship->lastdoc == ship->starsystem->doc1c )
	{
		ship->vx = ship->starsystem->p1x;
		ship->vy = ship->starsystem->p1y;
		ship->vz = ship->starsystem->p1z;
	}
	else if( ship->lastdoc == ship->starsystem->doc2a ||
		ship->lastdoc == ship->starsystem->doc2b || ship->lastdoc == ship->starsystem->doc2c )
	{
		ship->vx = ship->starsystem->p2x;
		ship->vy = ship->starsystem->p2y;
		ship->vz = ship->starsystem->p2z;
	}
	else if( ship->lastdoc == ship->starsystem->doc3a ||
		ship->lastdoc == ship->starsystem->doc3b || ship->lastdoc == ship->starsystem->doc3c )
	{
		ship->vx = ship->starsystem->p3x;
		ship->vy = ship->starsystem->p3y;
		ship->vz = ship->starsystem->p3z;
	}
	else if( ship->lastdoc == ship->starsystem->doc4a ||
		ship->lastdoc == ship->starsystem->doc4b || ship->lastdoc == ship->starsystem->doc4c )
	{
		ship->vx = ship->starsystem->p4x;
		ship->vy = ship->starsystem->p4y;
		ship->vz = ship->starsystem->p4z;
	}
	else if( ship->lastdoc == ship->starsystem->doc5a ||
		ship->lastdoc == ship->starsystem->doc5b || ship->lastdoc == ship->starsystem->doc5c )
	{
		ship->vx = ship->starsystem->p5x;
		ship->vy = ship->starsystem->p5y;
		ship->vz = ship->starsystem->p5z;
	}
	else if( ship->lastdoc == ship->starsystem->doc6a ||
		ship->lastdoc == ship->starsystem->doc6b || ship->lastdoc == ship->starsystem->doc6c )
	{
		ship->vx = ship->starsystem->p6x;
		ship->vy = ship->starsystem->p6y;
		ship->vz = ship->starsystem->p6z;
	}


	else

	{
		for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
		{
			if( ship->lastdoc == target->hanger )
			{
				ship->vx = target->vx;
				ship->vy = target->vy;
				ship->vz = target->vz;
			}
		}
	}

	ship->energy -= ( 100 + 100 * ship->ship_class );

	ship->vx += ( ship->hx * ship->currspeed * 2 );
	ship->vy += ( ship->hy * ship->currspeed * 2 );
	ship->vz += ( ship->hz * ship->currspeed * 2 );

	if( ship->autoboom == 1 )
	{
		if( percent < 50 )
		{
			echo_to_ship( AT_GREEN, ship,
				"&RA wierd noise is heard just before your suit explodes into a million pieces!\r\n" );
			snprintf( buf, MAX_STRING_LENGTH, "&GE&gveryone &Yl&Oa&Yu&Og&Yh! &P%s &Gw&ga&Gs &RF&rr&RI&re&RD&P!", ship->name );
			info_chan( buf );
			ship->autoboom = 0;
			destroy_ship( ship, NULL );
			return;
		}
		else if( percent > 50 )
		{
			echo_to_ship( AT_GREEN, ship,
				"&RWARNING: Ship systems just repaired a sabotage attempt to the launch circuits!\r\n" );
			ship->autoboom = 0;
		}
	}

	echo_to_room( AT_GREEN, get_room_index( ship->pilotseat ), "Launch complete.\r\n" );
	echo_to_ship( AT_YELLOW, ship, "The ship leaves the platform far behind as it flies into space." );
	snprintf( buf, MAX_STRING_LENGTH, "%s enters the starsystem at %.0f %.0f %.0f", ship->name, ship->vx, ship->vy, ship->vz );
	echo_to_system( AT_YELLOW, ship, buf, NULL );
	snprintf( buf, MAX_STRING_LENGTH, "%s lifts off into space.", ship->name );
	echo_to_room( AT_YELLOW, get_room_index( ship->lastdoc ), buf );

}


CMDF( do_land )
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	int vx, vy, vz;

	strcpy( arg, argument );

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou need to be in the pilot seat!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't land platforms\r\n", ch );
		return;
	}

	if( ship->ship_class == CAPITAL_SHIP )
	{
		send_to_char( "&RCapital ships are to big to land. You'll have to take a shuttle.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. Unable to land.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RThe ship is already docked!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_READY )
	{
		send_to_char( "&RPlease wait until the ship has finished its current manouver.\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RThere's nowhere to land around here!", ch );
		return;
	}

	if( ship->energy < ( 25 + 25 * ship->ship_class ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_ZERO ) )
	{
		send_to_char( "&RTurn off the Zero System first!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		set_char_color( AT_CYAN, ch );
		ch_printf( ch, "%s", "Land where?\r\n\r\n&CChoices&c:&W " );

		if( ship->starsystem->doc1a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location1a,
				ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
		if( ship->starsystem->doc1b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location1b,
				ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
		if( ship->starsystem->doc1c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location1c,
				ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
		if( ship->starsystem->doc2a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location2a,
				ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
		if( ship->starsystem->doc2b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location2b,
				ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
		if( ship->starsystem->doc2c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location2c,
				ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
		if( ship->starsystem->doc3a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location3a,
				ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
		if( ship->starsystem->doc3b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location3b,
				ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
		if( ship->starsystem->doc3c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location3c,
				ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
		if( ship->starsystem->doc4a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location4a,
				ship->starsystem->planet4, ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z );
		if( ship->starsystem->doc4b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location4b,
				ship->starsystem->planet4, ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z );
		if( ship->starsystem->doc4c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location4c,
				ship->starsystem->planet4, ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z );
		if( ship->starsystem->doc5a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location5a,
				ship->starsystem->planet5, ship->starsystem->p5x, ship->starsystem->p5y, ship->starsystem->p5z );
		if( ship->starsystem->doc5b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location5b,
				ship->starsystem->planet5, ship->starsystem->p5x, ship->starsystem->p5y, ship->starsystem->p5z );
		if( ship->starsystem->doc5c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location5c,
				ship->starsystem->planet5, ship->starsystem->p5x, ship->starsystem->p5y, ship->starsystem->p5z );
		if( ship->starsystem->doc6a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location6a,
				ship->starsystem->planet6, ship->starsystem->p6x, ship->starsystem->p6y, ship->starsystem->p6z );
		if( ship->starsystem->doc6b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location6b,
				ship->starsystem->planet6, ship->starsystem->p6x, ship->starsystem->p6y, ship->starsystem->p6z );
		if( ship->starsystem->doc6c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location6c,
				ship->starsystem->planet6, ship->starsystem->p6x, ship->starsystem->p6y, ship->starsystem->p6z );
		if( ship->starsystem->doc7a )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location7a,
				ship->starsystem->planet7, ship->starsystem->p7x, ship->starsystem->p7y, ship->starsystem->p7z );
		if( ship->starsystem->doc8b )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location8b,
				ship->starsystem->planet8, ship->starsystem->p8x, ship->starsystem->p8y, ship->starsystem->p8z );
		if( ship->starsystem->doc9c )
			ch_printf( ch, "%s (%s)  %d %d %d\r\n         ",
				ship->starsystem->location9c,
				ship->starsystem->planet9, ship->starsystem->p9x, ship->starsystem->p9y, ship->starsystem->p9z );

		for( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
		{
			if( target->hanger > 0 && target != ship )
				ch_printf( ch, "%s    %.0f %.0f %.0f\r\n         ", target->name, target->vx, target->vy, target->vz );
		}

		ch_printf( ch, "\n\rYour Coordinates: %.0f %.0f %.0f\r\n", ship->vx, ship->vy, ship->vz );
		return;
	}

	if( str_prefix( argument, ship->starsystem->location1a ) &&
		str_prefix( argument, ship->starsystem->location2a ) &&
		str_prefix( argument, ship->starsystem->location3a ) &&
		str_prefix( argument, ship->starsystem->location4a ) &&
		str_prefix( argument, ship->starsystem->location5a ) &&
		str_prefix( argument, ship->starsystem->location6a ) &&
		str_prefix( argument, ship->starsystem->location7a ) &&
		str_prefix( argument, ship->starsystem->location8a ) &&
		str_prefix( argument, ship->starsystem->location9a ) &&
		str_prefix( argument, ship->starsystem->location1b ) &&
		str_prefix( argument, ship->starsystem->location2b ) &&
		str_prefix( argument, ship->starsystem->location3b ) &&
		str_prefix( argument, ship->starsystem->location4b ) &&
		str_prefix( argument, ship->starsystem->location5b ) &&
		str_prefix( argument, ship->starsystem->location6b ) &&
		str_prefix( argument, ship->starsystem->location7b ) &&
		str_prefix( argument, ship->starsystem->location8b ) &&
		str_prefix( argument, ship->starsystem->location9b ) &&
		str_prefix( argument, ship->starsystem->location1c ) &&
		str_prefix( argument, ship->starsystem->location2c ) &&
		str_prefix( argument, ship->starsystem->location3c ) &&
		str_prefix( argument, ship->starsystem->location4c ) &&
		str_prefix( argument, ship->starsystem->location5c ) &&
		str_prefix( argument, ship->starsystem->location6c ) &&
		str_prefix( argument, ship->starsystem->location7c ) &&
		str_prefix( argument, ship->starsystem->location8c ) && str_prefix( argument, ship->starsystem->location9c ) )
	{
		target = get_ship_here( argument, ship->starsystem );
		if( target == NULL )
		{
			send_to_char( "&RI don't see that here. Type land by itself for a list\r\n", ch );
			return;
		}
		if( target == ship )
		{
			send_to_char( "&RYou can't land your ship inside itself!\r\n", ch );
			return;
		}
		if( !target->hanger )
		{
			send_to_char( "&RThat ship has no hanger for you to land in!\r\n", ch );
			return;
		}
		if( ship->ship_class == TRANSPORT_SHIP && target->ship_class == TRANSPORT_SHIP )
		{
			send_to_char( "&RThat ship is not big enough for your ship to land in!\r\n", ch );
			return;
		}
		if( !target->bayopen )
		{
			send_to_char( "&RTheir hanger is closed. You'll have to ask them to open it for you\r\n", ch );
			return;
		}
		if( ( target->vx > ship->vx + 200 ) || ( target->vx < ship->vx - 200 ) ||
			( target->vy > ship->vy + 200 ) || ( target->vy < ship->vy - 200 ) ||
			( target->vz > ship->vz + 200 ) || ( target->vz < ship->vz - 200 ) )
		{
			send_to_char( "&R That ship is too far away! You'll have to fly a little closer.\r\n", ch );
			return;
		}
	}
	else
	{

		if( !str_prefix( argument, ship->starsystem->location9a ) ||
			!str_prefix( argument, ship->starsystem->location9b ) || !str_prefix( argument, ship->starsystem->location9c ) )
		{
			vx = ship->starsystem->p9x;
			vy = ship->starsystem->p9y;
			vz = ship->starsystem->p9z;
		}
		if( !str_prefix( argument, ship->starsystem->location8a ) ||
			!str_prefix( argument, ship->starsystem->location8b ) || !str_prefix( argument, ship->starsystem->location8c ) )
		{
			vx = ship->starsystem->p8x;
			vy = ship->starsystem->p8y;
			vz = ship->starsystem->p8z;
		}
		if( !str_prefix( argument, ship->starsystem->location7a ) ||
			!str_prefix( argument, ship->starsystem->location7b ) || !str_prefix( argument, ship->starsystem->location7c ) )
		{
			vx = ship->starsystem->p7x;
			vy = ship->starsystem->p7y;
			vz = ship->starsystem->p7z;
		}
		if( !str_prefix( argument, ship->starsystem->location6a ) ||
			!str_prefix( argument, ship->starsystem->location6b ) || !str_prefix( argument, ship->starsystem->location6c ) )
		{
			vx = ship->starsystem->p6x;
			vy = ship->starsystem->p6y;
			vz = ship->starsystem->p6z;
		}
		if( !str_prefix( argument, ship->starsystem->location5a ) ||
			!str_prefix( argument, ship->starsystem->location5b ) || !str_prefix( argument, ship->starsystem->location5c ) )
		{
			vx = ship->starsystem->p5x;
			vy = ship->starsystem->p5y;
			vz = ship->starsystem->p5z;
		}
		if( !str_prefix( argument, ship->starsystem->location4a ) ||
			!str_prefix( argument, ship->starsystem->location4b ) || !str_prefix( argument, ship->starsystem->location4c ) )
		{
			vx = ship->starsystem->p4x;
			vy = ship->starsystem->p4y;
			vz = ship->starsystem->p4z;
		}
		if( !str_prefix( argument, ship->starsystem->location3a ) ||
			!str_prefix( argument, ship->starsystem->location3b ) || !str_prefix( argument, ship->starsystem->location3c ) )
		{
			vx = ship->starsystem->p3x;
			vy = ship->starsystem->p3y;
			vz = ship->starsystem->p3z;
		}
		if( !str_prefix( argument, ship->starsystem->location2a ) ||
			!str_prefix( argument, ship->starsystem->location2b ) || !str_prefix( argument, ship->starsystem->location2c ) )
		{
			vx = ship->starsystem->p2x;
			vy = ship->starsystem->p2y;
			vz = ship->starsystem->p2z;
		}
		if( !str_prefix( argument, ship->starsystem->location1a ) ||
			!str_prefix( argument, ship->starsystem->location1b ) || !str_prefix( argument, ship->starsystem->location1c ) )
		{
			vx = ship->starsystem->p1x;
			vy = ship->starsystem->p1y;
			vz = ship->starsystem->p1z;
		}
		if( ( vx > ship->vx + 200 ) || ( vx < ship->vx - 200 ) ||
			( vy > ship->vy + 200 ) || ( vy < ship->vy - 200 ) || ( vz > ship->vz + 200 ) || ( vz < ship->vz - 200 ) )
		{
			send_to_char( "&R That platform is too far away! You'll have to fly a little closer.\r\n", ch );
			return;
		}
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( number_percent( ) < chance )
	{
		set_char_color( AT_GREEN, ch );
		send_to_char( "Landing sequence initiated.\r\n", ch );
		act( AT_PLAIN, "$n begins the landing sequence.", ch, NULL, argument, TO_ROOM );
		echo_to_ship( AT_YELLOW, ship, "The ship slowly begins its landing approach." );
		ship->dest = STRALLOC( arg );
		ship->shipstate = SHIP_LAND;
		ship->currspeed = 0;
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_success( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_success( ch, gsn_midships );
		if( starsystem_from_vnum( ship->lastdoc ) != ship->starsystem )
			/*                   {
								  int xp =  (exp_level( ch->skill_level[PILOTING_ABILITY]+1) - exp_level( ch->skill_level[PILOTING_ABILITY])) / 3 ;
								  xp = UMIN( get_ship_value( ship ) , xp );
								  gain_exp( ch , xp , PILOTING_ABILITY );
								  ch_printf( ch, "&WYou gain %ld points of flight experience!\r\n", UMIN( get_ship_value( ship ) , xp ) );
							   }*/
			ship->ch = ch;
		return;
	}
	send_to_char( "You fail to work the controls properly.\r\n", ch );
	if( ship->ship_class == MOBILE_SUIT )
		learn_from_failure( ch, gsn_mobilesuits );
	else
		learn_from_failure( ch, gsn_midships );
	return;
}

void landship( SHIP_DATA *ship, const char *arg )
{
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];
	int destination;
	CHAR_DATA *ch;

	if( !str_prefix( arg, ship->starsystem->location9a ) )
		destination = ship->starsystem->doc9a;
	if( !str_prefix( arg, ship->starsystem->location9b ) )
		destination = ship->starsystem->doc9b;
	if( !str_prefix( arg, ship->starsystem->location9c ) )
		destination = ship->starsystem->doc9c;
	if( !str_prefix( arg, ship->starsystem->location8a ) )
		destination = ship->starsystem->doc8a;
	if( !str_prefix( arg, ship->starsystem->location8b ) )
		destination = ship->starsystem->doc8b;
	if( !str_prefix( arg, ship->starsystem->location8c ) )
		destination = ship->starsystem->doc8c;
	if( !str_prefix( arg, ship->starsystem->location7a ) )
		destination = ship->starsystem->doc7a;
	if( !str_prefix( arg, ship->starsystem->location7b ) )
		destination = ship->starsystem->doc7b;
	if( !str_prefix( arg, ship->starsystem->location7c ) )
		destination = ship->starsystem->doc7c;
	if( !str_prefix( arg, ship->starsystem->location6a ) )
		destination = ship->starsystem->doc6a;
	if( !str_prefix( arg, ship->starsystem->location6b ) )
		destination = ship->starsystem->doc6b;
	if( !str_prefix( arg, ship->starsystem->location6c ) )
		destination = ship->starsystem->doc6c;
	if( !str_prefix( arg, ship->starsystem->location5a ) )
		destination = ship->starsystem->doc5a;
	if( !str_prefix( arg, ship->starsystem->location5b ) )
		destination = ship->starsystem->doc5b;
	if( !str_prefix( arg, ship->starsystem->location5c ) )
		destination = ship->starsystem->doc5c;
	if( !str_prefix( arg, ship->starsystem->location4a ) )
		destination = ship->starsystem->doc4a;
	if( !str_prefix( arg, ship->starsystem->location4b ) )
		destination = ship->starsystem->doc4b;
	if( !str_prefix( arg, ship->starsystem->location4c ) )
		destination = ship->starsystem->doc4c;
	if( !str_prefix( arg, ship->starsystem->location3a ) )
		destination = ship->starsystem->doc3a;
	if( !str_prefix( arg, ship->starsystem->location3b ) )
		destination = ship->starsystem->doc3b;
	if( !str_prefix( arg, ship->starsystem->location3c ) )
		destination = ship->starsystem->doc3c;
	if( !str_prefix( arg, ship->starsystem->location2a ) )
		destination = ship->starsystem->doc2a;
	if( !str_prefix( arg, ship->starsystem->location2b ) )
		destination = ship->starsystem->doc2b;
	if( !str_prefix( arg, ship->starsystem->location2c ) )
		destination = ship->starsystem->doc2c;
	if( !str_prefix( arg, ship->starsystem->location1a ) )
		destination = ship->starsystem->doc1a;
	if( !str_prefix( arg, ship->starsystem->location1b ) )
		destination = ship->starsystem->doc1b;
	if( !str_prefix( arg, ship->starsystem->location1c ) )
		destination = ship->starsystem->doc1c;



	target = get_ship_here( arg, ship->starsystem );
	if( target != ship && target != NULL && target->bayopen
		&& ( ship->ship_class != TRANSPORT_SHIP || target->ship_class != TRANSPORT_SHIP ) )
		destination = target->hanger;

	if( !ship_to_room( ship, destination ) )
	{
		echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Could not complete approach. Landing aborted." );
		echo_to_ship( AT_YELLOW, ship, "The ship pulls back up out of its landing sequence." );
		if( ship->shipstate != SHIP_DISABLED )
			ship->shipstate = SHIP_READY;
		return;
	}

	echo_to_room( AT_YELLOW, get_room_index( ship->pilotseat ), "Landing sequence complete." );
	echo_to_ship( AT_YELLOW, ship, "You feel a slight thud as the ship sets down on the ground." );
	snprintf( buf, MAX_STRING_LENGTH, "%s disapears from your scanner.", ship->name );
	echo_to_system( AT_YELLOW, ship, buf, NULL );

	ship->location = destination;
	ship->lastdoc = ship->location;
	if( ship->shipstate != SHIP_DISABLED )
		ship->shipstate = SHIP_DOCKED;
	ship_from_starsystem( ship, ship->starsystem );

	snprintf( buf, MAX_STRING_LENGTH, "%s lands on the platform.", ship->name );
	echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );

	if( ship->ch && ship->ch->desc )
	{
		int xp;

		ch = ship->ch;
		xp = ( exp_level( ch->skill_level[PILOTING_ABILITY] + 1 ) - exp_level( ch->skill_level[PILOTING_ABILITY] ) ) / 3;
		xp = UMIN( get_ship_value( ship ), xp );
		gain_exp( ch, xp, PILOTING_ABILITY );
		ch_printf( ch, "&WYou gain %ld points of flight experience!\r\n", UMIN( get_ship_value( ship ), xp ) );
		ship->ch = NULL;
	}

	ship->energy = ship->energy - 25 - 25 * ship->ship_class;

	if( !str_cmp( "Public", ship->owner ) || ( !str_cmp( "Holosuit", ship->owner ) ) )
	{
		ship->energy = ship->maxenergy;
		ship->ammo = ship->maxammo;
		ship->chaff = ship->maxchaff;
		ship->missiles = ship->maxmissiles;
		ship->torpedos = ship->maxtorpedos;
		ship->rockets = ship->maxrockets;
		ship->shield = 0;
		ship->autorecharge = false;
		ship->autotrack = false;
		ship->autospeed = false;
		ship->armor = ship->maxarmor;
		ship->armorhead = ship->maxarmorhead;
		ship->armorlarm = ship->maxarmorlarm;
		ship->armorrarm = ship->maxarmorrarm;
		ship->armorlegs = ship->maxarmorlegs;
		ship->targettype = 0;

		ship->missilestate = MISSILE_READY;
		ship->statet0 = LASER_READY;
		ship->statet1 = LASER_READY;
		ship->statet2 = LASER_READY;
		ship->shipstate = SHIP_DOCKED;

		echo_to_cockpit( AT_YELLOW, ship, "Repairing and refueling ship..." );
	}

	save_ship( ship );
}

CMDF( do_accelerate )
{
	int chance;
	int change;
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RThe controls must be at the pilots chair...\r\n", ch );
		return;
	}

	if( ship->armorlegs < 1 )
	{
		send_to_char( "&RYour legs are destroyed! Thrusters are out!\r\n", ch );
		return;
	}
	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms can't move!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. Unable to accelerate.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}
	if( ship->energy < abs( ( atoi( argument ) - abs( ship->currspeed ) ) / 10 ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( ship->ship_class == CAPITAL_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_capitalships] );
	if( number_percent( ) >= chance )
	{
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_failure( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_failure( ch, gsn_midships );
		if( ship->ship_class == CAPITAL_SHIP )
			learn_from_failure( ch, gsn_capitalships );
		return;
	}

	change = atoi( argument );

	act( AT_PLAIN, "$n manipulates the ships controls.", ch, NULL, argument, TO_ROOM );

	if( change > ship->currspeed )
	{
		send_to_char( "&GAccelerating\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "The ship begins to accelerate." );
		snprintf( buf, MAX_STRING_LENGTH, "%s begins to speed up.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
	}

	if( change < ship->currspeed )
	{
		send_to_char( "&GDecelerating\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "The ship begins to slow down." );
		snprintf( buf, MAX_STRING_LENGTH, "%s begins to slow down.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
	}

	ship->energy -= abs( ( change - abs( ship->currspeed ) ) / 10 );

	ship->currspeed = URANGE( 0, change, ship->realspeed );

	if( ship->ship_class == MOBILE_SUIT )
		learn_from_success( ch, gsn_mobilesuits );
	if( ship->ship_class == TRANSPORT_SHIP )
		learn_from_success( ch, gsn_midships );
	if( ship->ship_class == CAPITAL_SHIP )
		learn_from_success( ch, gsn_capitalships );

}

CMDF( do_trajectory )
{
	char buf[MAX_STRING_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int chance;
	float vx, vy, vz;
	SHIP_DATA *ship;


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYour not in the pilots seat.\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. Unable to manuever.\r\n", ch );
		return;
	}
	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms can't turn!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}
	if( ship->shipstate != SHIP_READY )
	{
		send_to_char( "&RPlease wait until the ship has finished its current manouver.\r\n", ch );
		return;
	}
	if( ship->energy < ( ship->currspeed / 10 ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( ship->ship_class == CAPITAL_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_capitalships] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_failure( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_failure( ch, gsn_midships );
		if( ship->ship_class == CAPITAL_SHIP )
			learn_from_failure( ch, gsn_capitalships );
		return;
	}

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	vx = atof( arg2 );
	vy = atof( arg3 );
	vz = atof( argument );

	if( vx == ship->vx && vy == ship->vy && vz == ship->vz )
	{
		ch_printf( ch, "The ship is already at %.0f %.0f %.0f !", vx, vy, vz );
	}
	ship->hx = vx - ship->vx;
	ship->hy = vy - ship->vy;
	ship->hz = vz - ship->vz;

	ship->energy -= ( ship->currspeed / 10 );

	ch_printf( ch, "&GNew course set, approaching %.0f %.0f %.0f.\r\n", vx, vy, vz );
	act( AT_PLAIN, "$n manipulates the ships controls.", ch, NULL, argument, TO_ROOM );

	echo_to_cockpit( AT_YELLOW, ship, "The ship begins to turn.\r\n" );
	snprintf( buf, MAX_STRING_LENGTH, "%s turns altering its present course.", ship->name );
	echo_to_system( AT_ORANGE, ship, buf, NULL );

	if( ship->ship_class == MOBILE_SUIT || ( ship->ship_class == TRANSPORT_SHIP && ship->manuever > 50 ) )
		ship->shipstate = SHIP_BUSY_3;
	else if( ship->ship_class == TRANSPORT_SHIP || ( ship->ship_class == CAPITAL_SHIP && ship->manuever > 50 ) )
		ship->shipstate = SHIP_BUSY_2;
	else
		ship->shipstate = SHIP_BUSY;

	if( ship->ship_class == MOBILE_SUIT )
		learn_from_success( ch, gsn_mobilesuits );
	if( ship->ship_class == TRANSPORT_SHIP )
		learn_from_success( ch, gsn_midships );
	if( ship->ship_class == CAPITAL_SHIP )
		learn_from_success( ch, gsn_capitalships );

}


CMDF( do_buyship )
{
	long price;
	SHIP_DATA *ship;

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "&ROnly players can do that!\r\n", ch );
		return;
	}

	ship = ship_in_room( ch->in_room, argument );
	if( !ship )
	{
		ship = ship_from_cockpit( ch->in_room->vnum );

		if( !ship )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
			return;
		}
	}

	if( str_cmp( ship->owner, "" ) || ship->type == MOB_SHIP )
	{
		send_to_char( "&RThat suit isn't for sale!\r\n", ch );
		return;
	}

	if( ship->type == SHIP_IMPERIAL )
	{
		if( !ch->pcdata->clan || str_cmp( ch->pcdata->clan->name, "the empire" ) )
		{
			if( !ch->pcdata->clan || !ch->pcdata->clan->mainclan || str_cmp( ch->pcdata->clan->mainclan->name, "The Empire" ) )
			{
				send_to_char( "&RThat ship may only be purchaced by the Empire!\r\n", ch );
				return;
			}
		}
	}
	else if( ship->type == SHIP_REPUBLIC )
	{
		if( !ch->pcdata->clan || str_cmp( ch->pcdata->clan->name, "the new republic" ) )
		{
			if( !ch->pcdata->clan || !ch->pcdata->clan->mainclan
				|| str_cmp( ch->pcdata->clan->mainclan->name, "The New Republic" ) )
			{
				send_to_char( "&RThat ship may only be purchaced by The New Republic!\r\n", ch );
				return;
			}
		}
	}
	else
	{
		if( ch->pcdata->clan &&
			( !str_cmp( ch->pcdata->clan->name, "the new republic" ) ||
				( ch->pcdata->clan->mainclan && !str_cmp( ch->pcdata->clan->mainclan->name, "the new republic" ) ) ) )
		{
			send_to_char( "&RAs a member of the New Republic you may only purchase NR Ships!\r\n", ch );
			return;
		}
		if( ch->pcdata->clan &&
			( !str_cmp( ch->pcdata->clan->name, "the empire" ) ||
				( ch->pcdata->clan->mainclan && !str_cmp( ch->pcdata->clan->mainclan->name, "the empire" ) ) ) )
		{
			send_to_char( "&RAs a member of the Empire you may only purchase Imperial Ships!\r\n", ch );
			return;
		}
	}

	if( !str_cmp( ch->name, ship->prevowner ) )
	{
		price = get_ship_value( ship ) / 2;
	}
	else
	{
		price = get_ship_value( ship );
	}
	/*
	   if ( !str_cmp(ch->name, ship->prevowner) && ship->prevtimer >= 1 )
	   {
	 ch_printf( ch, "Sorry, %s won't be for sale for another %d minutes.\r\n", ship->name,ship->prevtimer );
			return;
	   }
	*/
	if( ch->gold < price )
	{
		ch_printf( ch, "&RThis ship costs %ld. You don't have enough money!\r\n", price );
		return;
	}

	ch->gold -= price;
	ch_printf( ch, "&GYou pay %ld dollars to purchace the ship.\r\n", price );

	act( AT_PLAIN, "$n walks over to a terminal and purchases something.", ch, NULL, argument, TO_ROOM );

	STRFREE( ship->owner );
	ship->owner = STRALLOC( ch->name );

	if( ship->stype == 7 )
	{
		STRFREE( ship->prevowner );
		ship->prevowner = STRALLOC( ch->name );
		ship->prevtimer = 60;
	}
	save_ship( ship );

}

CMDF( do_clanbuyship )
{
	long price;
	SHIP_DATA *ship;
	CLAN_DATA *clan;
	CLAN_DATA *mainclan;

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "&ROnly players can do that!\r\n", ch );
		return;
	}
	if( !ch->pcdata->clan )
	{
		send_to_char( "&RYou aren't a member of any organizations!\r\n", ch );
		return;
	}

	clan = ch->pcdata->clan;
	mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

	if( ( ch->pcdata->bestowments
		&& is_name( "clanbuyship", ch->pcdata->bestowments ) ) || !str_cmp( ch->name, clan->leader ) )
		;
	else
	{
		send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.\r\n", ch );
		return;
	}

	ship = ship_in_room( ch->in_room, argument );
	if( !ship )
	{
		ship = ship_from_cockpit( ch->in_room->vnum );

		if( !ship )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
			return;
		}
	}

	if( str_cmp( ship->owner, "" ) || ship->type == MOB_SHIP )
	{
		send_to_char( "&RThat ship isn't for sale!\r\n", ch );
		return;
	}

	if( str_cmp( mainclan->name, "The Empire" ) && ship->type == SHIP_IMPERIAL )
	{
		send_to_char( "&RThat ship may only be purchaced by the Empire!\r\n", ch );
		return;
	}

	if( str_cmp( mainclan->name, "The New Republic" ) && ship->type == SHIP_REPUBLIC )
	{
		send_to_char( "&RThat ship may only be purchaced by The New Republic!\r\n", ch );
		return;
	}

	if( !str_cmp( mainclan->name, "The Empire" ) && ship->type != SHIP_IMPERIAL )
	{
		send_to_char( "&RDue to contractual agreements that ship may not be purchaced by the empire!\r\n", ch );
		return;
	}

	if( !str_cmp( mainclan->name, "The New Republic" ) && ship->type != SHIP_REPUBLIC )
	{
		send_to_char( "&RBecause of contractual agreements, the NR can only purchase NR ships!\r\n", ch );
		return;
	}

	price = get_ship_value( ship );

	if( ch->pcdata->clan->funds < price )
	{
		ch_printf( ch, "&RThis ship costs %ld. You don't have enough money!\r\n", price );
		return;
	}

	clan->funds -= price;
	ch_printf( ch, "&G%s pays %ld dollars to purchace the ship.\r\n", clan->name, price );

	act( AT_PLAIN, "$n walks over to a terminal and buys something.", ch, NULL, argument, TO_ROOM );

	STRFREE( ship->owner );
	ship->owner = STRALLOC( clan->name );
	save_ship( ship );

	if( ship->ship_class <= SHIP_PLATFORM )
		clan->spacecraft++;
	else
		clan->vehicles++;
}

CMDF( do_sellship )
{
	long price;
	SHIP_DATA *ship;

	ship = ship_in_room( ch->in_room, argument );
	if( !ship )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
	}

	if( str_cmp( ship->owner, ch->name ) )
	{
		send_to_char( "&RThat isn't your ship!", ch );
		return;
	}

	price = get_ship_value( ship );

	ch->gold += ( price - price / 10 );
	ch_printf( ch, "&GYou receive %ld dollars from selling your ship.\r\n", price - price / 10 );

	act( AT_PLAIN, "$n walks over to a terminal and sells something.", ch, NULL, argument, TO_ROOM );

	STRFREE( ship->owner );
	ship->owner = STRALLOC( "" );
	STRFREE( ship->pilot );
	ship->pilot = STRALLOC( "" );
	STRFREE( ship->copilot );
	ship->copilot = STRALLOC( "" );
	save_ship( ship );
	STRFREE( ship->prevowner );
	ship->prevowner = STRALLOC( "" );
	ship->prevtimer = 0;
}

CMDF( do_info )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		if( argument[0] == '\0' )
		{
			act( AT_PLAIN, "Which suit do you want info on?.", ch, NULL, NULL, TO_CHAR );
			return;
		}

		ship = ship_in_room( ch->in_room, argument );
		if( !ship )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
			return;
		}

		target = ship;
	}
	else if( argument[0] == '\0' )
		target = ship;
	else
		target = get_ship_here( argument, ship->starsystem );

	if( target == NULL )
	{
		send_to_char( "&RI don't see that here.\n\rTry the radar, or type info by itself for info on this ship.\r\n", ch );
		return;
	}

	if( abs( target->vx - ship->vx ) > 500 + ship->sensor * 2 ||
		abs( target->vy - ship->vy ) > 500 + ship->sensor * 2 || abs( target->vz - ship->vz ) > 500 + ship->sensor * 2 )
	{
		send_to_char( "&RThat ship is to far away to scan.\r\n", ch );
		return;
	}

	ch_printf( ch, "\r\n&z%s %s : %s\r\n&B",
		target->type == SHIP_REPUBLIC ? "New Republic" :
		( target->type == SHIP_IMPERIAL ? "Imperial" : "Civilian" ),
		target->ship_class == MOBILE_SUIT ? "Mobile Suit" :
		( target->ship_class == TRANSPORT_SHIP ? "Mobile Transport" :
			( target->ship_class == CAPITAL_SHIP ? "Capital Ship" :
				( ship->ship_class == SHIP_PLATFORM ? "Platform" :
					( ship->ship_class == CLOUD_CAR ? "Cloudcar" :
						( ship->ship_class == OCEAN_SHIP ? "Boat" :
							( ship->ship_class == LAND_SPEEDER ? "Speeder" :
								( ship->ship_class == WHEELED ? "Wheeled Transport" :
									( ship->ship_class == LAND_CRAWLER ? "Crawler" :
										( ship->ship_class == WALKER ? "Walker" : "Unknown" ) ) ) ) ) ) ) ) ),
		target->name, target->filename );
	ch_printf( ch,
		"&B====================================================\r\n&zDescription&B:&W %s\r\n&zNickname&B:&W %s\r\n&zColors&B:&W %s &Band&W %s \r\n&B====================================================\r\n&zOwner&B:&W %s         &zPilot&B:&W %s         &zCopilot&B:&W %s\r\n",
		target->description, target->nickname, target->colorone, target->colortwo, target->owner, target->pilot,
		target->copilot );
	ch_printf( ch, "&zArmor&B:&W %-5d           &zAmmo&B:&W %d\r\n", target->maxarmor, target->maxammo );
	ch_printf( ch, "&zMod&B:&W %-11s       &zAlloy&B:&W %s\r\n",
		ship->mod == 1 ? "Zero System" :
		ship->mod == 2 ? "Cloaking" :
		ship->mod == 3 ? "Transform" :
		ship->mod == 4 ? "Deathblow" : "None",
		ship->alloy == 1 ? "Iron" :
		ship->alloy == 2 ? "Steel" :
		ship->alloy == 3 ? "Titanium" :
		ship->alloy == 4 ? "Neo-Titanium" :
		ship->alloy == 5 ? "Gundanium" : ship->alloy == 6 ? "Z-Gundanium" : "Iron" );

	ch_printf( ch, "&zFuel&B:&W %-7d          &zSensor Range&B:&W %d\r\n", target->maxenergy, target->sensor );

	ch_printf( ch, "&zMines&B:&W %2d&B/&W%-2d           &zBombs&B:&W %d\r\n",
		target->mines, target->maxmines, target->bombs );

	ch_printf( ch, "&zTop Speed&B:&W %-3d         &zHyper Speed&B:&W %d\r\n", target->realspeed, target->hyperspeed );
	ch_printf( ch, "&zFirst Weapon&B:&W %s\r\n",
		target->firstweapon == 0 ? "100mm Machine Gun" :
		( target->firstweapon == 1 ? "105mm Rifle" :
			( target->firstweapon == 2 ? "Leo Bazooka" :
				( target->firstweapon == 3 ? "Beam Sabre" :
					( target->firstweapon == 4 ? "Shoulder Mounted Energy Cannon" :
						( target->firstweapon == 5 ? "Side Mounted Missile Launchers" :
							( target->firstweapon == 6 ? "Beam Rifle" :
								( target->firstweapon == 7 ? "Laser Cannon" :
									( target->firstweapon == 8 ? "Twin-Gattling Gun" :
										( target->firstweapon == 9 ? "Beam Cannon" :
											( target->firstweapon == 10 ? "Buster Rifle" :
												( target->firstweapon == 11 ? "Head Vulcan" :
													( target->firstweapon == 12 ? "Beam Scythe" :
														( target->firstweapon == 13 ? "Buster Shield" :
															( target->firstweapon == 14 ? "Beam Gattling" :
																( target->firstweapon == 15 ? "Multi Blast" :
																	( target->firstweapon == 16 ? "Army Knife" :
																		( target->firstweapon == 17 ? "Shatols" :
																			( target->firstweapon == 18 ? "Shoulder Missiles" :
																				( target->firstweapon == 19 ? "Cross Crusher" :
																					( target->firstweapon == 20 ? "Big Beam Sabre" :
																						( target->firstweapon == 21 ? "Heat Rod" :
																							( target->firstweapon == 22 ? "Beam Glaive" :
																								( target->firstweapon == 23 ? "Dragon Fang" :
																									( target->firstweapon == 24 ? "Flamethrower" :
																										( target->firstweapon == 25 ? "Dober Gun" :
																											( target->firstweapon == 26 ? "Short Blast" :
																												( target->firstweapon == 27 ? "Long Blast" :
																													( target->firstweapon == 28 ? "Small Beam Cannon" :
																														( target->firstweapon == 29 ? "Beam Blade" :
																															( target->firstweapon == 30 ? "Planet Denfensers" :
																																( target->firstweapon == 31 ? "Libra Main Cannon" :
																																	( target->firstweapon ==
																																		32 ? "Twin Beam Scythe" : ( target->
																																			firstweapon ==
																																			33 ?
																																			"Twin Buster Rifle"
																																			: ( target->
																																				firstweapon ==
																																				34 ?
																																				"Mega Cannon"
																																				: ( target->
																																					firstweapon
																																					==
																																					35 ?
																																					"Twin Beam Trident"
																																					:
																																					( target->
																																						firstweapon
																																						==
																																						39 ?
																																						"250mm Machine Gun"
																																						:
																																						"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );

	ch_printf( ch, "&zSecond Weapon&B:&W %s\r\n",
		target->secondweapon == 0 ? "100mm Machine Gun" :
		( target->secondweapon == 1 ? "105mm Rifle" :
			( target->secondweapon == 2 ? "Leo Bazooka" :
				( target->secondweapon == 3 ? "Beam Sabre" :
					( target->secondweapon == 4 ? "Shoulder Mounted Energy Cannon" :
						( target->secondweapon == 5 ? "Side Mounted Missile Launchers" :
							( target->secondweapon == 6 ? "Beam Rifle" :
								( target->secondweapon == 7 ? "Laser Cannon" :
									( target->secondweapon == 8 ? "Twin-Gattling Gun" :
										( target->secondweapon == 9 ? "Beam Cannon" :
											( target->secondweapon == 10 ? "Buster Rifle" :
												( target->secondweapon == 11 ? "Head Vulcan" :
													( target->secondweapon == 12 ? "Beam Scythe" :
														( target->secondweapon == 13 ? "Buster Shield" :
															( target->secondweapon == 14 ? "Beam Gattling" :
																( target->secondweapon == 15 ? "Multi Blast" :
																	( target->secondweapon == 16 ? "Army Knife" :
																		( target->secondweapon == 17 ? "Shatols" :
																			( target->secondweapon == 18 ? "Shoulder Missiles" :
																				( target->secondweapon == 19 ? "Cross Crusher" :
																					( target->secondweapon == 19 ? "Cross Crusher" :
																						( target->secondweapon == 20 ? "Big Beam Sabre" :
																							( target->secondweapon == 21 ? "Heat Rod" :
																								( target->secondweapon == 22 ? "Beam Glaive" :
																									( target->secondweapon == 23 ? "Dragon Fang" :
																										( target->secondweapon == 24 ? "Flamethrower" :
																											( target->secondweapon == 25 ? "Dober Gun" :
																												( target->secondweapon == 26 ? "Short Blast" :
																													( target->secondweapon == 27 ? "Long Blast" :
																														( target->secondweapon == 28 ? "Small Beam Cannon" :
																															( target->secondweapon == 29 ? "Beam Blade" :
																																( target->secondweapon ==
																																	30 ? "Planet Denfensers" : ( target->
																																		secondweapon ==
																																		31 ?
																																		"Libra Main Cannon"
																																		: ( target->
																																			secondweapon ==
																																			32 ?
																																			"Twin Beam Scythe"
																																			: ( target->
																																				secondweapon
																																				==
																																				33 ?
																																				"Twin Buster Rifle"
																																				: ( target->
																																					secondweapon
																																					==
																																					34 ?
																																					"Mega Cannon"
																																					:
																																					( target->
																																						secondweapon
																																						==
																																						35 ?
																																						"Twin Beam Trident"
																																						:
																																						( target->
																																							secondweapon
																																							==
																																							39 ?
																																							"250mm Machine Gun"
																																							:
																																							"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );


	ch_printf( ch, "&zThird Weapon&B:&W %s\r\n&B====================================================\r\n",
		target->thirdweapon == 0 ? "100mm Machine Gun" :
		( target->thirdweapon == 1 ? "105mm Rifle" :
			( target->thirdweapon == 2 ? "Leo Bazooka" :
				( target->thirdweapon == 3 ? "Beam Sabre" :
					( target->thirdweapon == 4 ? "Shoulder Mounted Energy Cannon" :
						( target->thirdweapon == 5 ? "Side Mounted Missile Launchers" :
							( target->thirdweapon == 6 ? "Beam Rifle" :
								( target->thirdweapon == 7 ? "Laser Cannon" :
									( target->thirdweapon == 8 ? "Twin-Gattling Gun" :
										( target->thirdweapon == 9 ? "Beam Cannon" :
											( target->thirdweapon == 10 ? "Buster Rifle" :
												( target->thirdweapon == 11 ? "Head Vulcan" :
													( target->thirdweapon == 12 ? "Beam Scythe" :
														( target->thirdweapon == 13 ? "Buster Shield" :
															( target->thirdweapon == 14 ? "Beam Gattling" :
																( target->thirdweapon == 15 ? "Multi Blast" :
																	( target->thirdweapon == 16 ? "Army Knife" :
																		( target->thirdweapon == 17 ? "Shatols" :
																			( target->thirdweapon == 18 ? "Shoulder Missiles" :
																				( target->thirdweapon == 19 ? "Cross Crusher" :
																					( target->thirdweapon == 19 ? "Cross Crusher" :
																						( target->thirdweapon == 20 ? "Big Beam Sabre" :
																							( target->thirdweapon == 21 ? "Heat Rod" :
																								( target->thirdweapon == 22 ? "Beam Glaive" :
																									( target->thirdweapon == 23 ? "Dragon Fang" :
																										( target->thirdweapon == 24 ? "Flamethrower" :
																											( target->thirdweapon == 25 ? "Dober Gun" :
																												( target->thirdweapon == 26 ? "Short Blast" :
																													( target->thirdweapon == 27 ? "Long Blast" :
																														( target->thirdweapon == 28 ? "Small Beam Cannon" :
																															( target->thirdweapon == 29 ? "Beam Blade" :
																																( target->thirdweapon == 30 ? "Planet Denfensers" :
																																	( target->thirdweapon ==
																																		31 ? "Libra Main Cannon" : ( target->
																																			thirdweapon ==
																																			32 ?
																																			"Twin Beam Scythe"
																																			: ( target->
																																				thirdweapon ==
																																				33 ?
																																				"Twin Buster Rifle"
																																				: ( target->
																																					thirdweapon
																																					==
																																					34 ?
																																					"Mega Cannon"
																																					:
																																					( target->
																																						thirdweapon
																																						==
																																						35 ?
																																						"Twin Beam Trident"
																																						:
																																						( target->
																																							thirdweapon
																																							==
																																							39 ?
																																							"250mm Machine Gun"
																																							:
																																							"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );

	act( AT_PLAIN, "$n glances at a moniter screen.", ch, NULL, argument, TO_ROOM );

}


CMDF( do_autorecharge )
{
	int chance;
	SHIP_DATA *ship;
	int recharge;


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the co-pilots seat!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_shipsystems] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		learn_from_failure( ch, gsn_shipsystems );
		return;
	}

	act( AT_PLAIN, "$n flips a switch on the control panell.", ch, NULL, argument, TO_ROOM );

	if( !str_cmp( argument, "on" ) )
	{
		ship->autorecharge = true;
		send_to_char( "&GYou power up the shields.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "Shields ON. Autorecharge ON." );
	}
	else if( !str_cmp( argument, "off" ) )
	{
		ship->autorecharge = false;
		send_to_char( "&GYou shutdown the shields.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "Shields OFF. Shield strength set to 0. Autorecharge OFF." );
		ship->shield = 0;
	}
	else if( !str_cmp( argument, "idle" ) )
	{
		ship->autorecharge = false;
		send_to_char( "&GYou let the shields idle.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "Autorecharge OFF. Shields IDLEING." );
	}
	else
	{
		if( ship->autorecharge == true )
		{
			ship->autorecharge = false;
			send_to_char( "&GYou toggle the shields.\r\n", ch );
			echo_to_cockpit( AT_YELLOW, ship, "Autorecharge OFF. Shields IDLEING." );
		}
		else
		{
			ship->autorecharge = true;
			send_to_char( "&GYou toggle the shields.\r\n", ch );
			echo_to_cockpit( AT_YELLOW, ship, "Shields ON. Autorecharge ON" );
		}
	}

	if( ship->autorecharge )
	{
		recharge = URANGE( 1, ship->maxshield - ship->shield, 25 + ship->ship_class * 25 );
		recharge = UMIN( recharge, ship->energy * 5 + 100 );
		ship->shield += recharge;
		ship->energy -= ( recharge * 2 + recharge * ship->ship_class );
	}

	learn_from_success( ch, gsn_shipsystems );
}

CMDF( do_autopilot )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the pilots seat!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey! Thats not your ship!\r\n", ch );
		return;
	}

	if( ship->target0 || ship->target1 || ship->target2 )
	{
		send_to_char( "&RNot while the ship is engaged with an enemy!\r\n", ch );
		return;
	}


	act( AT_PLAIN, "$n flips a switch on the control panell.", ch, NULL, argument, TO_ROOM );

	if( ship->autopilot == true )
	{
		ship->autopilot = false;
		send_to_char( "&GYou toggle the autopilot.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "Autopilot OFF." );
	}
	else
	{
		ship->autopilot = true;
		ship->autorecharge = true;
		send_to_char( "&GYou toggle the autopilot.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "Autopilot ON." );
	}

}

CMDF( do_openhatch )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( !argument || argument[0] == '\0' || !str_cmp( argument, "hatch" ) )
	{
		ship = ship_from_entrance( ch->in_room->vnum );
		if( ship == NULL )
		{
			send_to_char( "&ROpen what?\r\n", ch );
			return;
		}
		else
		{
			if( !ship->hatchopen )
			{

				if( ship->ship_class == SHIP_PLATFORM )
				{
					send_to_char( "&RTry one of the docking bays!\r\n", ch );
					return;
				}
				if( ship->location != ship->lastdoc || ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED ) )
				{
					send_to_char( "&RPlease wait till the ship lands!\r\n", ch );
					return;
				}
				ship->hatchopen = true;
				send_to_char( "&GYou open the hatch.\r\n", ch );
				act( AT_PLAIN, "$n opens the hatch.", ch, NULL, argument, TO_ROOM );
				snprintf( buf, MAX_STRING_LENGTH, "The hatch on %s opens.", ship->name );
				echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
				return;
			}
			else
			{
				send_to_char( "&RIt's already open.\r\n", ch );
				return;
			}
		}
	}

	ship = ship_in_room( ch->in_room, argument );
	if( !ship )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "&RThat ship has already started to launch", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey! Thats not your ship!\r\n", ch );
		return;
	}

	if( !ship->hatchopen )
	{
		ship->hatchopen = true;
		act( AT_PLAIN, "You open the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
		act( AT_PLAIN, "$n opens the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
		echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch opens from the outside." );
		return;
	}

	send_to_char( "&GIts already open!\r\n", ch );

}


CMDF( do_closehatch )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( !argument || argument[0] == '\0' || !str_cmp( argument, "hatch" ) )
	{
		ship = ship_from_entrance( ch->in_room->vnum );
		if( ship == NULL )
		{
			send_to_char( "&RClose what?\r\n", ch );
			return;
		}
		else
		{

			if( ship->ship_class == SHIP_PLATFORM )
			{
				send_to_char( "&RTry one of the docking bays!\r\n", ch );
				return;
			}
			if( ship->hatchopen )
			{
				ship->hatchopen = false;
				send_to_char( "&GYou close the hatch.\r\n", ch );
				act( AT_PLAIN, "$n closes the hatch.", ch, NULL, argument, TO_ROOM );
				snprintf( buf, MAX_STRING_LENGTH, "The hatch on %s closes.", ship->name );
				echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
				return;
			}
			else
			{
				send_to_char( "&RIt's already closed.\r\n", ch );
				return;
			}
		}
	}

	ship = ship_in_room( ch->in_room, argument );
	if( !ship )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "&RThat ship has already started to launch", ch );
		return;
	}
	else
	{
		if( ship->hatchopen )
		{
			ship->hatchopen = false;
			act( AT_PLAIN, "You close the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
			act( AT_PLAIN, "$n closes the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
			echo_to_room( AT_YELLOW, get_room_index( ship->entrance ), "The hatch is closed from outside." );

			return;
		}
		else
		{
			send_to_char( "&RIts already closed.\r\n", ch );
			return;
		}
	}


}


CMDF( do_hyperspace )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *eShip;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}


	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou aren't in the pilots seat.\r\n", ch );
		return;
	}

	if( ship->armorlegs < 1 )
	{
		send_to_char( "&RYour legs are destroyed! Thrusters are out!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first.\r\n", ch );
		return;
	}


	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms can't move!\r\n", ch );
		return;
	}
	if( ship->hyperspeed == 0 )
	{
		send_to_char( "&RThis ship is not equipped with a hyperdrive!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou are already travelling lightspeed!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. Unable to manuever.\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}
	if( ship->shipstate != SHIP_READY )
	{
		send_to_char( "&RPlease wait until the ship has finished its current manouver.\r\n", ch );
		return;
	}
	if( !ship->currjump )
	{
		send_to_char( "&RYou need to calculate your jump first!\r\n", ch );
		return;
	}


	if( ship->energy < ( 200 + ship->hyperdistance * ( 1 + ship->ship_class ) / 3 ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( ship->currspeed <= 0 )
	{
		send_to_char( "&RYou need to speed up a little first!\r\n", ch );
		return;
	}

	for( eShip = ship->starsystem->first_ship; eShip; eShip = eShip->next_in_starsystem )
	{
		if( eShip == ship )
			continue;

		if( abs( eShip->vx - ship->vx ) < 500 && abs( eShip->vy - ship->vy ) < 500 && abs( eShip->vz - ship->vz ) < 500 )
		{
			ch_printf( ch, "&RYou are too close to %s to make the jump to lightspeed.\r\n", eShip->name );
			return;
		}
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_mobilesuits] );
	if( ship->ship_class == TRANSPORT_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_midships] );
	if( ship->ship_class == CAPITAL_SHIP )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_capitalships] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou can't figure out which lever to use.\r\n", ch );
		if( ship->ship_class == MOBILE_SUIT )
			learn_from_failure( ch, gsn_mobilesuits );
		if( ship->ship_class == TRANSPORT_SHIP )
			learn_from_failure( ch, gsn_midships );
		if( ship->ship_class == CAPITAL_SHIP )
			learn_from_failure( ch, gsn_capitalships );
		return;
	}
	snprintf( buf, MAX_STRING_LENGTH, "%s disapears from your scanner.", ship->name );
	echo_to_system( AT_YELLOW, ship, buf, NULL );

	ship_from_starsystem( ship, ship->starsystem );
	ship->shipstate = SHIP_HYPERSPACE;

	send_to_char( "&GYou push forward the hyperspeed lever.\r\n", ch );
	act( AT_PLAIN, "$n pushes a lever forward on the control panel.", ch, NULL, argument, TO_ROOM );
	echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it makes the jump to lightspeed." );
	echo_to_cockpit( AT_YELLOW, ship, "The stars become streaks of light as you enter hyperspace." );

	ship->energy -= ( 100 + ship->hyperdistance * ( 1 + ship->ship_class ) / 3 );

	ship->vx = ship->jx;
	ship->vy = ship->jy;
	ship->vz = ship->jz;

	if( ship->ship_class == MOBILE_SUIT )
		learn_from_success( ch, gsn_mobilesuits );
	if( ship->ship_class == TRANSPORT_SHIP )
		learn_from_success( ch, gsn_midships );
	if( ship->ship_class == CAPITAL_SHIP )
		learn_from_success( ch, gsn_capitalships );

}


CMDF( do_target )
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	strcpy( arg, argument );

	switch( ch->substate )
	{
	default:
		if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in the gunners seat or turret of a ship to do that!\r\n", ch );
			return;
		}

		if( ship->ship_class > SHIP_PLATFORM )
		{
			send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
			return;
		}

		if( ship->shipstate == SHIP_HYPERSPACE )
		{
			send_to_char( "&RYou can only do that in realspace!\r\n", ch );
			return;
		}
		if( !ship->starsystem )
		{
			send_to_char( "&RYou can't do that until you've finished launching!\r\n", ch );
			return;
		}

		if( autofly( ship ) )
		{
			send_to_char( "&RYou'll have to turn off the ships autopilot first....\r\n", ch );
			return;
		}

		if( arg[0] == '\0' )
		{
			send_to_char( "&RYou need to specify a target!\r\n", ch );
			return;
		}

		if( !str_cmp( arg, "none" ) )
		{
			send_to_char( "&GTarget set to none.\r\n", ch );
			if( ch->in_room->vnum == ship->gunseat )
				ship->target0 = NULL;
			if( ch->in_room->vnum == ship->turret1 )
				ship->target1 = NULL;
			if( ch->in_room->vnum == ship->turret2 )
				ship->target2 = NULL;
			return;
		}

		target = get_ship_here( arg, ship->starsystem );
		if( target == NULL )
		{
			send_to_char( "&RThat ship isn't here!\r\n", ch );
			return;
		}

		if( target == ship )
		{
			send_to_char( "&RYou can't target your own ship!\r\n", ch );
			return;
		}
		/*
						if ( !str_cmp(ship->owner, "Public") && str_cmp(target->owner, "Public") )
						{
							send_to_char("&RPublic suits are for transport, NOT fighting!!\r\n",ch);
							return;
						}
						if ( str_cmp(ship->owner, "Public") && !str_cmp(target->owner, "Public") )
						{
							send_to_char("&RYou can't target Public Suits!!\r\n",ch);
							return;
						}
		*/
		if( !str_cmp( ship->owner, "Holosuit" ) && str_cmp( target->owner, "Holosuit" ) )
		{
			send_to_char( "&RYou can only target other Holosuits!\r\n", ch );
			return;
		}
		if( str_cmp( ship->owner, "Holosuit" ) && !str_cmp( target->owner, "Holosuit" ) )
		{
			send_to_char( "&RIf you want to target a Holosuit, get in another Holosuit!\r\n", ch );
			return;
		}

		if( target->pksuit == 0 )
		{
			send_to_char( "&RDork, thats a Non-PK suit!\r\n", ch );
			return;
		}

		if( ship->pksuit == 0 && target->type != MOB_SHIP )
		{
			send_to_char( "&RDork, you're a Non-PK!\r\n", ch );
			return;
		}

		if( abs( ship->vx - target->vx ) > 5000 ||
			abs( ship->vy - target->vy ) > 5000 || abs( ship->vz - target->vz ) > 5000 )
		{
			send_to_char( "&RThat ship is too far away to target.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_weaponsystems] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GTracking target.\r\n", ch );
			act( AT_PLAIN, "$n makes some adjustments on the targeting computer.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 1, do_target, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		learn_from_failure( ch, gsn_weaponsystems );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
			return;
		send_to_char( "&RYour concentration is broken. You fail to lock onto your target.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		return;
	}

	target = get_ship_here( arg, ship->starsystem );
	if( target == NULL || target == ship )
	{
		send_to_char( "&RThe ship has left the starsytem. Targeting aborted.\r\n", ch );
		return;
	}

	if( ch->in_room->vnum == ship->gunseat )
		ship->target0 = target;

	if( ch->in_room->vnum == ship->turret1 )
		ship->target1 = target;

	if( ch->in_room->vnum == ship->turret2 )
		ship->target2 = target;

	send_to_char( "&GTarget Locked.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", ship->name );
	echo_to_cockpit( AT_BLOOD, target, buf );

	learn_from_success( ch, gsn_weaponsystems );

	if( autofly( target ) && !target->target0 )
	{
		snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
		echo_to_cockpit( AT_BLOOD, ship, buf );
		target->target0 = ship;
	}
}


CMDF( do_fire )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the gunners chair or turret of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
	{
		send_to_char( "&R HEY! This is Mobile Suit! Type Help  Suits\r\n", ch );
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
	if( ship->energy < 5 )
	{
		send_to_char( "&RTheres not enough energy left to fire!\r\n", ch );
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
			send_to_char( "&RThe ships main laser is damaged.\r\n", ch );
			return;
		}
		if( ship->statet0 >= ship->lasers )
		{
			send_to_char( "&RThe lasers are still recharging.\r\n", ch );
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
		if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of laser range.\r\n", ch );
			return;
		}
		if( ship->ship_class < 2 && !is_facing( ship, target ) )
		{
			send_to_char( "&RThe main laser can only fire forward. You'll need to turn your ship!\r\n", ch );
			return;
		}
		ship->statet0++;
		chance += target->ship_class * 25;
		chance -= target->manuever / 10;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Lasers fire from %s at you but miss.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "The ships lasers fire at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_spacecombat );
			learn_from_failure( ch, gsn_spacecombat2 );
			learn_from_failure( ch, gsn_spacecombat3 );
			snprintf( buf, MAX_STRING_LENGTH, "Laserfire from %s barely misses %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Laserfire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by lasers from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your ships lasers hit %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		damage_ship_ch( target, 5, 10, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "missile" ) )
	{
		if( ship->missilestate == MISSILE_DAMAGED )
		{
			send_to_char( "&RThe ships missile launchers are dammaged.\r\n", ch );
			return;
		}
		if( ship->missiles <= 0 )
		{
			send_to_char( "&RYou have no missiles to fire!\r\n", ch );
			return;
		}
		if( ship->missilestate != MISSILE_READY )
		{
			send_to_char( "&RThe missiles are still reloading.\r\n", ch );
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
		if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of missile range.\r\n", ch );
			return;
		}
		if( ship->ship_class < 2 && !is_facing( ship, target ) )
		{
			send_to_char( "&RMissiles can only fire in a forward. You'll need to turn your ship!\r\n", ch );
			return;
		}
		chance -= target->manuever / 5;
		chance -= target->currspeed / 20;
		chance += target->ship_class * target->ship_class * 25;
		chance -= ( abs( target->vx - ship->vx ) / 100 );
		chance -= ( abs( target->vy - ship->vy ) / 100 );
		chance -= ( abs( target->vz - ship->vz ) / 100 );
		chance += ( 30 );
		chance = URANGE( 20, chance, 80 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			send_to_char( "&RYou fail to lock onto your target!", ch );
			ship->missilestate = MISSILE_RELOAD_2;
			return;
		}
		new_missile( ship, target, ch, CONCUSSION_MISSILE );
		ship->missiles--;
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		echo_to_cockpit( AT_YELLOW, ship, "Missiles launched." );
		snprintf( buf, MAX_STRING_LENGTH, "Incoming missile from %s.", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires a missile towards %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		learn_from_success( ch, gsn_weaponsystems );
		if( ship->ship_class == CAPITAL_SHIP || ship->ship_class == SHIP_PLATFORM )
			ship->missilestate = MISSILE_RELOAD;
		else
			ship->missilestate = MISSILE_FIRED;

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}
	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "torpedo" ) )
	{
		if( ship->missilestate == MISSILE_DAMAGED )
		{
			send_to_char( "&RThe ships missile launchers are dammaged.\r\n", ch );
			return;
		}
		if( ship->torpedos <= 0 )
		{
			send_to_char( "&RYou have no torpedos to fire!\r\n", ch );
			return;
		}
		if( ship->missilestate != MISSILE_READY )
		{
			send_to_char( "&RThe torpedos are still reloading.\r\n", ch );
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
		if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of torpedo range.\r\n", ch );
			return;
		}
		if( ship->ship_class < 2 && !is_facing( ship, target ) )
		{
			send_to_char( "&RTorpedos can only fire in a forward direction. You'll need to turn your ship!\r\n", ch );
			return;
		}
		chance -= target->manuever / 5;
		chance -= target->currspeed / 20;
		chance += target->ship_class * target->ship_class * 25;
		chance -= ( abs( target->vx - ship->vx ) / 100 );
		chance -= ( abs( target->vy - ship->vy ) / 100 );
		chance -= ( abs( target->vz - ship->vz ) / 100 );
		chance = URANGE( 20, chance, 80 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			send_to_char( "&RYou fail to lock onto your target!", ch );
			ship->missilestate = MISSILE_RELOAD_2;
			return;
		}
		new_missile( ship, target, ch, PROTON_TORPEDO );
		ship->torpedos--;
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		echo_to_cockpit( AT_YELLOW, ship, "Missiles launched." );
		snprintf( buf, MAX_STRING_LENGTH, "Incoming torpedo from %s.", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires a torpedo towards %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		learn_from_success( ch, gsn_weaponsystems );
		if( ship->ship_class == CAPITAL_SHIP || ship->ship_class == SHIP_PLATFORM )
			ship->missilestate = MISSILE_RELOAD;
		else
			ship->missilestate = MISSILE_FIRED;

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

	if( ch->in_room->vnum == ship->gunseat && !str_prefix( argument, "rocket" ) )
	{
		if( ship->missilestate == MISSILE_DAMAGED )
		{
			send_to_char( "&RThe ships missile launchers are damaged.\r\n", ch );
			return;
		}
		if( ship->rockets <= 0 )
		{
			send_to_char( "&RYou have no rockets to fire!\r\n", ch );
			return;
		}
		if( ship->missilestate != MISSILE_READY )
		{
			send_to_char( "&RThe missiles are still reloading.\r\n", ch );
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
		if( abs( target->vx - ship->vx ) > 800 || abs( target->vy - ship->vy ) > 800 || abs( target->vz - ship->vz ) > 800 )
		{
			send_to_char( "&RThat ship is out of rocket range.\r\n", ch );
			return;
		}
		if( ship->ship_class < 2 && !is_facing( ship, target ) )
		{
			send_to_char( "&RRockets can only fire forward. You'll need to turn your ship!\r\n", ch );
			return;
		}
		chance -= target->manuever / 5;
		chance -= target->currspeed / 20;
		chance += target->ship_class * target->ship_class * 25;
		chance -= ( abs( target->vx - ship->vx ) / 100 );
		chance -= ( abs( target->vy - ship->vy ) / 100 );
		chance -= ( abs( target->vz - ship->vz ) / 100 );
		chance -= 30;
		chance = URANGE( 20, chance, 80 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			send_to_char( "&RYou fail to lock onto your target!", ch );
			ship->missilestate = MISSILE_RELOAD_2;
			return;
		}
		new_missile( ship, target, ch, HEAVY_ROCKET );
		ship->rockets--;
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		echo_to_cockpit( AT_YELLOW, ship, "Rocket launched." );
		snprintf( buf, MAX_STRING_LENGTH, "Incoming rocket from %s.", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires a heavy rocket towards %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		learn_from_success( ch, gsn_weaponsystems );
		if( ship->ship_class == CAPITAL_SHIP || ship->ship_class == SHIP_PLATFORM )
			ship->missilestate = MISSILE_RELOAD;
		else
			ship->missilestate = MISSILE_FIRED;

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

	if( ch->in_room->vnum == ship->turret1 && !str_prefix( argument, "lasers" ) )
	{
		if( ship->statet1 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships turret is damaged.\r\n", ch );
			return;
		}
		if( ship->statet1 > ship->ship_class )
		{
			send_to_char( "&RThe turbolaser is recharging.\r\n", ch );
			return;
		}
		if( ship->target1 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target1;
		if( ship->target1->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target1 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of laser range.\r\n", ch );
			return;
		}
		ship->statet1++;
		chance -= target->manuever / 10;
		chance += target->ship_class * 25;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from %s at you but miss.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from the ships turret at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			snprintf( buf, MAX_STRING_LENGTH, "%s fires at %s but misses.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			learn_from_failure( ch, gsn_spacecombat );
			learn_from_failure( ch, gsn_spacecombat2 );
			learn_from_failure( ch, gsn_spacecombat3 );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Turboasers fire from %s, hitting %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by turbolasers from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from the turret, hitting %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		damage_ship_ch( target, 10, 25, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

	if( ch->in_room->vnum == ship->turret2 && !str_prefix( argument, "lasers" ) )
	{
		if( ship->statet2 == LASER_DAMAGED )
		{
			send_to_char( "&RThe ships turret is damaged.\r\n", ch );
			return;
		}
		if( ship->statet2 > ship->ship_class )
		{
			send_to_char( "&RThe turbolaser is still recharging.\r\n", ch );
			return;
		}
		if( ship->target2 == NULL )
		{
			send_to_char( "&RYou need to choose a target first.\r\n", ch );
			return;
		}
		target = ship->target2;
		if( ship->target2->starsystem != ship->starsystem )
		{
			send_to_char( "&RYour target seems to have left.\r\n", ch );
			ship->target2 = NULL;
			return;
		}
		if( abs( target->vx - ship->vx ) > 1000 || abs( target->vy - ship->vy ) > 1000 || abs( target->vz - ship->vz ) > 1000 )
		{
			send_to_char( "&RThat ship is out of laser range.\r\n", ch );
			return;
		}
		ship->statet2++;
		chance -= target->manuever / 10;
		chance += target->ship_class * 25;
		chance -= target->currspeed / 20;
		chance -= ( abs( target->vx - ship->vx ) / 70 );
		chance -= ( abs( target->vy - ship->vy ) / 70 );
		chance -= ( abs( target->vz - ship->vz ) / 70 );
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from %s barely missing %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from %s at you but miss.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from the turret missing %s.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_spacecombat );
			learn_from_failure( ch, gsn_spacecombat2 );
			learn_from_failure( ch, gsn_spacecombat3 );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Turbolasers fire from %s, hitting %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by turbolasers from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "turbolasers fire from the turret hitting %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		damage_ship_ch( target, 10, 25, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

	send_to_char( "&RYou can't fire that!\r\n", ch );

}


CMDF( do_calculate )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int chance, count;
	SHIP_DATA *ship;
	SPACE_DATA *starsystem;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_navseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be at a nav computer to calculate jumps.\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first....\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RAnd what exactly are you going to calculate...?\r\n", ch );
		return;
	}
	if( ship->hyperspeed == 0 )
	{
		send_to_char( "&RThis ship is not equipped with a hyperdrive!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}
	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can only do that in realspace.\r\n", ch );
		return;
	}
	if( argument[0] == '\0' )
	{
		send_to_char( "&WFormat: Calculate <starsystem> <entry x> <entry y> <entry z>\r\n&wPossible destinations:\r\n", ch );
		for( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
		{
			set_char_color( AT_NOTE, ch );
			ch_printf( ch, "%-30s %d\r\n", starsystem->name,
				( abs( starsystem->xpos - ship->starsystem->xpos ) +
					abs( starsystem->ypos - ship->starsystem->ypos ) ) / 2 );
			count++;
		}
		if( !count )
		{
			send_to_char( "No Starsystems found.\r\n", ch );
		}
		return;
	}
	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_navigation] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou cant seem to figure the charts out today.\r\n", ch );
		learn_from_failure( ch, gsn_navigation );
		return;
	}


	ship->currjump = starsystem_from_name( arg1 );
	ship->jx = atoi( arg2 );
	ship->jy = atoi( arg3 );
	ship->jz = atoi( argument );

	if( ship->currjump == NULL )
	{
		send_to_char( "&RYou can't seem to find that starsytem on your charts.\r\n", ch );
		return;
	}
	else
	{
		starsystem = ship->currjump;

		if( starsystem->star1 && strcmp( starsystem->star1, "" ) &&
			abs( ship->jx - starsystem->s1x ) < 300 &&
			abs( ship->jy - starsystem->s1y ) < 300 && abs( ship->jz - starsystem->s1z ) < 300 )
		{
			echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
			echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
			ship->currjump = NULL;
			return;
		}
		else if( starsystem->star2 && strcmp( starsystem->star2, "" ) &&
			abs( ship->jx - starsystem->s2x ) < 300 &&
			abs( ship->jy - starsystem->s2y ) < 300 && abs( ship->jz - starsystem->s2z ) < 300 )
		{
			echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
			echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
			ship->currjump = NULL;
			return;
		}
		else if( starsystem->planet1 && strcmp( starsystem->planet1, "" ) &&
			abs( ship->jx - starsystem->p1x ) < 300 &&
			abs( ship->jy - starsystem->p1y ) < 300 && abs( ship->jz - starsystem->p1z ) < 300 )
		{
			echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
			echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
			ship->currjump = NULL;
			return;
		}
		else if( starsystem->planet2 && strcmp( starsystem->planet2, "" ) &&
			abs( ship->jx - starsystem->p2x ) < 300 &&
			abs( ship->jy - starsystem->p2y ) < 300 && abs( ship->jz - starsystem->p2z ) < 300 )
		{
			echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
			echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
			ship->currjump = NULL;
			return;
		}
		else if( starsystem->planet3 && strcmp( starsystem->planet3, "" ) &&
			abs( ship->jx - starsystem->p3x ) < 300 &&
			abs( ship->jy - starsystem->p3y ) < 300 && abs( ship->jz - starsystem->p3z ) < 300 )
		{
			echo_to_cockpit( AT_RED, ship, "WARNING.. Jump coordinates too close to stellar object." );
			echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
			ship->currjump = NULL;
			return;
		}
		else
		{
			ship->jx += number_range( -250, 250 );
			ship->jy += number_range( -250, 250 );
			ship->jz += number_range( -250, 250 );
		}
	}

	if( ship->jx > MAX_COORD_S || ship->jy > MAX_COORD_S || ship->jz > MAX_COORD_S ||
		ship->jx < -MAX_COORD_S || ship->jy < -MAX_COORD_S || ship->jz < -MAX_COORD_S )
	{
		echo_to_cockpit( AT_RED, ship, "WARNING.. Jump too far out of known Galaxy!" );
		echo_to_cockpit( AT_RED, ship, "WARNING.. Hyperjump NOT set." );
		ship->currjump = NULL;
		return;
	}

	ship->hyperdistance = abs( ship->starsystem->xpos - ship->currjump->xpos );
	ship->hyperdistance += abs( ship->starsystem->ypos - ship->currjump->ypos );
	ship->hyperdistance /= 5;

	if( ship->hyperdistance < 100 )
		ship->hyperdistance = 100;

	ship->hyperdistance += number_range( 0, 200 );

	ch_printf( ch, "&GHyper jump set! Distance: %d, Ready to go!\r\n", ship->hyperdistance );
	act( AT_PLAIN, "$n does some calculations using the ships computer.", ch, NULL, argument, TO_ROOM );

	learn_from_success( ch, gsn_navigation );

	WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
}

CMDF( do_repairship )
{
	char arg[MAX_INPUT_LENGTH];
	int chance, change;
	SHIP_DATA *ship;

	switch( ch->substate )
	{
	default:
		if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
		{
			send_to_char( "&RYou must be in the engine room of a ship to do that!\r\n", ch );
			return;
		}

		if( ship->shipstate == SHIP_HYPERSPACE )
		{
			send_to_char( "&RYou can only do that in realspace!\r\n", ch );
			return;
		}

		if( str_cmp( argument, "armor" ) && str_cmp( argument, "drive" ) &&
			str_cmp( argument, "launcher" ) && str_cmp( argument, "laser" ) &&
			str_cmp( argument, "turret 1" ) && str_cmp( argument, "turret 2" ) )
		{
			send_to_char( "&RYou need to spceify something to repair:\r\n", ch );
			send_to_char( "&rTry: armor, drive, launcher, laser, turret 1, or turret 2\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_shipmaintenance] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin your repairs\r\n", ch );
			act( AT_PLAIN, "$n begins repairing the ships $T.", ch, NULL, argument, TO_ROOM );
			if( !str_cmp( arg, "armor" ) )
				add_timer( ch, TIMER_DO_FUN, 15, do_repairship, 1 );
			else
				add_timer( ch, TIMER_DO_FUN, 5, do_repairship, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou fail to locate the source of the problem.\r\n", ch );
		learn_from_failure( ch, gsn_shipmaintenance );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( const char * ) ch->dest_buf, MAX_INPUT_LENGTH );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
			return;
		send_to_char( "&RYou are distracted and fail to finish your repairs.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( ship = ship_from_engine( ch->in_room->vnum ) ) == NULL )
	{
		return;
	}

	if( !str_cmp( arg, "armor" ) )
	{
		change = URANGE( 0,
			number_range( ( int ) ( ch->pcdata->learned[gsn_shipmaintenance] / 2 ),
				( int ) ( ch->pcdata->learned[gsn_shipmaintenance] ) ),
			( ship->maxarmor - ship->armor ) );
		ship->armor += change;
		ch_printf( ch, "&GRepair complete.. Armor strength inreased by %d points.\r\n", change );
	}

	if( !str_cmp( arg, "drive" ) )
	{
		if( ship->location == ship->lastdoc )
			ship->shipstate = SHIP_DOCKED;
		else
			ship->shipstate = SHIP_READY;
		send_to_char( "&GShips drive repaired.\r\n", ch );
	}

	if( !str_cmp( arg, "launcher" ) )
	{
		ship->missilestate = MISSILE_READY;
		send_to_char( "&GMissile launcher repaired.\r\n", ch );
	}

	if( !str_cmp( arg, "laser" ) )
	{
		ship->statet0 = LASER_READY;
		send_to_char( "&GMain laser repaired.\r\n", ch );
	}

	if( !str_cmp( arg, "turret 1" ) )
	{
		ship->statet1 = LASER_READY;
		send_to_char( "&GLaser Turret 1 repaired.\r\n", ch );
	}

	if( !str_cmp( arg, "turret 2" ) )
	{
		ship->statet2 = LASER_READY;
		send_to_char( "&Laser Turret 2 repaired.\r\n", ch );
	}

	act( AT_PLAIN, "$n finishes the repairs.", ch, NULL, argument, TO_ROOM );

	learn_from_success( ch, gsn_shipmaintenance );

}


CMDF( do_refuel )
{
}

CMDF( do_addpilot )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't do that here.\r\n", ch );
		return;
	}

	if( str_cmp( ship->owner, ch->name ) )
	{

		if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
			if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
				;
			else if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
				;
			else if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
				;
			else
			{
				send_to_char( "&RThat isn't your ship!", ch );
				return;
			}
		else
		{
			send_to_char( "&RThat isn't your ship!", ch );
			return;
		}

	}

	if( ship->stype == 7 )
	{
		send_to_char( "Nice try! You can't addpilots to a Gundam!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "&RAdd which pilot?\r\n", ch );
		return;
	}

	if( str_cmp( ship->pilot, "" ) )
	{
		if( str_cmp( ship->copilot, "" ) )
		{
			send_to_char( "&RYou are ready have a pilot and copilot..\r\n", ch );
			send_to_char( "&RTry rempilot first.\r\n", ch );
			return;
		}

		STRFREE( ship->copilot );
		ship->copilot = STRALLOC( argument );
		send_to_char( "Copilot Added.\r\n", ch );
		save_ship( ship );
		return;

		return;
	}

	STRFREE( ship->pilot );
	ship->pilot = STRALLOC( argument );
	send_to_char( "Pilot Added.\r\n", ch );
	save_ship( ship );

}

CMDF( do_rempilot )
{
	SHIP_DATA *ship;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't do that here.\r\n", ch );
		return;
	}

	if( str_cmp( ship->owner, ch->name ) )
	{

		if( !IS_NPC( ch ) && ch->pcdata && ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, ship->owner ) )
			if( !str_cmp( ch->pcdata->clan->leader, ch->name ) )
				;
			else if( !str_cmp( ch->pcdata->clan->number1, ch->name ) )
				;
			else if( !str_cmp( ch->pcdata->clan->number2, ch->name ) )
				;
			else
			{
				send_to_char( "&RThat isn't your ship!", ch );
				return;
			}
		else
		{
			send_to_char( "&RThat isn't your ship!", ch );
			return;
		}

	}

	if( argument[0] == '\0' )
	{
		send_to_char( "&RRemove which pilot?\r\n", ch );
		return;
	}

	if( !str_cmp( ship->pilot, argument ) )
	{
		STRFREE( ship->pilot );
		ship->pilot = STRALLOC( "" );
		send_to_char( "Pilot Removed.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( ship->copilot, argument ) )
	{
		STRFREE( ship->copilot );
		ship->copilot = STRALLOC( "" );
		send_to_char( "Copilot Removed.\r\n", ch );
		save_ship( ship );
		return;
	}

	send_to_char( "&RThat person isn't listed as one of the ships pilots.\r\n", ch );

}

CMDF( do_radar )
{
	SHIP_DATA *target;
	int chance;
	SHIP_DATA *ship;
	MISSILE_DATA *missile;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit or turret of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RWait until after you launch!\r\n", ch );
		return;
	}

	if( ship->armorhead < 1 )
	{
		send_to_char( "&RYou can't use that! Your suits head is destroyed!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that unless the ship is flying in realspace!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_navigation] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		learn_from_failure( ch, gsn_navigation );
		return;
	}


	act( AT_PLAIN, "$n checks the radar.", ch, NULL, argument, TO_ROOM );

	set_char_color( AT_WHITE, ch );
	ch_printf( ch, "%s\r\n\r\n", ship->starsystem->name );
	set_char_color( AT_LBLUE, ch );

	if( ship->starsystem->star1 && str_cmp( ship->starsystem->star1, "" ) )
		ch_printf( ch, "&R%s   %d %d %d\r\n",
			ship->starsystem->star1, ship->starsystem->s1x, ship->starsystem->s1y, ship->starsystem->s1z );

	if( ship->starsystem->star2 && str_cmp( ship->starsystem->star2, "" ) )
		ch_printf( ch, "&R%s   %d %d %d\r\n",
			ship->starsystem->star2, ship->starsystem->s2x, ship->starsystem->s2y, ship->starsystem->s2z );

	if( ship->starsystem->planet1 && str_cmp( ship->starsystem->planet1, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet1, ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z );
	if( ship->starsystem->planet2 && str_cmp( ship->starsystem->planet2, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet2, ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z );
	if( ship->starsystem->planet3 && str_cmp( ship->starsystem->planet3, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet3, ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z );
	if( ship->starsystem->planet4 && str_cmp( ship->starsystem->planet4, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet4, ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z );
	if( ship->starsystem->planet5 && str_cmp( ship->starsystem->planet5, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet5, ship->starsystem->p5x, ship->starsystem->p5y, ship->starsystem->p5z );
	if( ship->starsystem->planet6 && str_cmp( ship->starsystem->planet6, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet6, ship->starsystem->p6x, ship->starsystem->p6y, ship->starsystem->p6z );

	if( ship->starsystem->planet7 && str_cmp( ship->starsystem->planet7, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet7, ship->starsystem->p7x, ship->starsystem->p7y, ship->starsystem->p7z );

	if( ship->starsystem->planet8 && str_cmp( ship->starsystem->planet8, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet8, ship->starsystem->p8x, ship->starsystem->p8y, ship->starsystem->p8z );

	if( ship->starsystem->planet9 && str_cmp( ship->starsystem->planet9, "" ) )
		ch_printf( ch, "&Y%s   %d %d %d\r\n",
			ship->starsystem->planet9, ship->starsystem->p9x, ship->starsystem->p9y, ship->starsystem->p9z );

	for( target = first_ship; target; target = target->next_in_starsystem )
	{
		if( target != ship )
		{
			if( target->ship_class == SMINE )
				ch_printf( ch, "&P%s    %.0f %.0f %.0f\r\n", target->name, target->vx, target->vy, target->vz );
			else
				ch_printf( ch, "%C%s    %.0f %.0f %.0f\r\n", target->name, target->vx, target->vy, target->vz );
		}
	}

	for( missile = ship->starsystem->first_missile; missile; missile = missile->next_in_starsystem )
	{
		if( abs( missile->mx - ship->vx ) > 750 + ship->sensor || abs( missile->my - ship->vy ) >
			750 + ship->sensor || abs( missile->mz - ship->vz ) > 750 + ship->sensor )
		{
			ch_printf( ch, "&Y%s    %d %d %d\r\n",
				missile->missiletype == CONCUSSION_MISSILE ? "A Concusion missile" :
				( missile->missiletype == PROTON_TORPEDO ? "A Torpedo" :
					( missile->missiletype == HEAVY_ROCKET ? "A Heavy Rocket" : "A Heavy Bomb" ) ),
				missile->mx, missile->my, missile->mz );
		}
	}

	ch_printf( ch, "\r\n&WCurrent Coords: %.0f %.0f %.0f\r\n", ship->vx, ship->vy, ship->vz );


	learn_from_success( ch, gsn_navigation );

}

CMDF( do_autotrack )
{
	SHIP_DATA *ship;
	int chance;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}


	if( ship->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms don't have autotracking systems!\r\n", ch );
		return;
	}
	if( ship->ship_class == CAPITAL_SHIP )
	{
		send_to_char( "&RThis ship is too big for autotracking!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou aren't in the pilots chair!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first....\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_shipsystems] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYour notsure which switch to flip.\r\n", ch );
		learn_from_failure( ch, gsn_shipsystems );
		return;
	}

	act( AT_PLAIN, "$n flips a switch on the control panel.", ch, NULL, argument, TO_ROOM );
	if( ship->autotrack )
	{
		ship->autotrack = false;
		echo_to_cockpit( AT_YELLOW, ship, "Autotracking off." );
	}

	else
	{
		ship->autotrack = true;
		echo_to_cockpit( AT_YELLOW, ship, "Autotracking on." );
	}

	learn_from_success( ch, gsn_shipsystems );

}

CMDF( do_jumpvector )
{
}
CMDF( do_reload )
{
}
CMDF( do_closebay )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];
	if( ship_from_pilotseat( ch->in_room->vnum ) == NULL && ship_from_hanger( ch->in_room->vnum ) == NULL )
	{
		send_to_char( "&RYou aren't in the pilots chair or hanger of a ship!\r\n", ch );
		return;
	}

	if( ship_from_pilotseat( ch->in_room->vnum ) )
		ship = ship_from_pilotseat( ch->in_room->vnum );
	else
		ship = ship_from_hanger( ch->in_room->vnum );

	if( ship->hanger == 0 )
	{
		send_to_char( "&RThis ship has no hanger!\r\n", ch );
		return;
	}

	if( ship->bayopen == false )
	{
		send_to_char( "Bay doors are already closed!\r\n", ch );
		return;
	}

	act( AT_PLAIN, "$n flips a switch on the control panel.\r\n", ch, NULL, argument, TO_ROOM );
	ship->bayopen = false;

	echo_to_cockpit( AT_YELLOW, ship, "Bay Doors close.\r\n" );
	send_to_char( "You close the bay doors.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "%s's bay doors close.\r\n", ship->name );
	echo_to_system( AT_YELLOW, ship, buf, NULL );

}


CMDF( do_openbay )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ship_from_pilotseat( ch->in_room->vnum ) == NULL && ship_from_hanger( ch->in_room->vnum ) == NULL )
	{
		send_to_char( "&RYou aren't in the pilots chair or hanger of a ship!\r\n", ch );
		return;
	}

	if( ship_from_pilotseat( ch->in_room->vnum ) )
		ship = ship_from_pilotseat( ch->in_room->vnum );
	else
		ship = ship_from_hanger( ch->in_room->vnum );

	if( ship->hanger == 0 )
	{
		send_to_char( "&RThis ship has no hanger!\r\n", ch );
		return;
	}

	if( ship->bayopen == true )
	{
		send_to_char( "Bay doors are already open!\r\n", ch );
		return;
	}

	act( AT_PLAIN, "$n flips a switch on the control panel.\r\n", ch, NULL, argument, TO_ROOM );
	ship->bayopen = true;

	echo_to_cockpit( AT_YELLOW, ship, "Bay Doors Open" );
	send_to_char( "You open the bay doors", ch );
	snprintf( buf, MAX_STRING_LENGTH, "%s's bay doors open.\r\n", ship->name );
	echo_to_system( AT_YELLOW, ship, buf, NULL );

}


CMDF( do_tractorbeam )
{

	char arg[MAX_INPUT_LENGTH];
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	strcpy( arg, argument );

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	/*
		if ( !check_pilot( ch , ship ) )
		{
			send_to_char("This isn't your ship!\r\n" , ch );
			return;
		}
	*/
	if( ship->tractorbeam == 0 )
	{
		send_to_char( "You might want to install a tractorbeam!\r\n", ch );
		return;
	}

	if( ship->hanger == 0 )
	{
		send_to_char( "No hanger available.\r\n", ch );
		return;
	}

	if( !ship->bayopen )
	{
		send_to_char( "Your hanger is closed.\r\n", ch );
		return;
	}


	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou need to be in the pilot seat!\r\n", ch );
		return;
	}


	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe ships drive is disabled. No power available.\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYour ship is docked!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{

		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_READY )
	{
		send_to_char( "&RPlease wait until the ship has finished its current manouver.\r\n", ch );
		return;
	}




	if( argument[0] == '\0' )
	{
		send_to_char( "&RCapture what?\r\n", ch );
		return;
	}

	target = get_ship_here( argument, ship->starsystem );

	if( target == NULL )
	{
		send_to_char( "&RI don't see that here.\r\n", ch );
		return;
	}

	if( target == ship )
	{
		send_to_char( "&RYou can't yourself!\r\n", ch );
		return;
	}

	if( target->shipstate == SHIP_LAND )
	{
		send_to_char( "&RThat ship is already in a landing sequence.\r\n", ch );
		return;
	}

	if( ( target->vx > ship->vx + 200 ) || ( target->vx < ship->vx - 200 ) ||
		( target->vy > ship->vy + 200 ) || ( target->vy < ship->vy - 200 ) ||
		( target->vz > ship->vz + 200 ) || ( target->vz < ship->vz - 200 ) )
	{
		send_to_char( "&R That ship is too far away! You'll have to fly a little closer.\r\n", ch );
		return;
	}

	if( ship->ship_class <= target->ship_class )
	{
		send_to_char( "&RThat ship is too big for your hanger.\r\n", ch );
		return;
	}

	if( target->ship_class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't capture platforms.\r\n", ch );
		return;
	}

	if( target->ship_class == CAPITAL_SHIP )
	{
		send_to_char( "&RYou can't capture capital ships.\r\n", ch );
		return;
	}

	if( target->type == MOB_SHIP )
	{
		send_to_char( "&RSomething is stoping you from caputuring that.\r\n", ch );
		return;
	}


	if( ship->energy < ( 25 + 25 * target->ship_class ) )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}




	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_tractorbeams] );

	/*
	 * This is just a first guess chance modifier, feel free to change if needed
	 */

	chance = chance * ( ship->tractorbeam / ( target->currspeed + 1 ) );

	if( number_percent( ) < chance )
	{
		set_char_color( AT_GREEN, ch );
		send_to_char( "Capture sequence initiated.\r\n", ch );
		act( AT_PLAIN, "$n begins the capture sequence.", ch, NULL, argument, TO_ROOM );
		echo_to_ship( AT_YELLOW, ship, "ALERT: Ship is being captured, all hands to docking bay." );
		echo_to_ship( AT_YELLOW, target, "The ship shudders as a tractorbeam locks on." );
		snprintf( buf, MAX_STRING_LENGTH, "You are being captured by %s.", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );

		if( autofly( target ) && !target->target0 )
			target->target0 = ship;

		target->dest = STRALLOC( ship->name );
		target->shipstate = SHIP_LAND;
		target->currspeed = 0;

		learn_from_success( ch, gsn_tractorbeams );
		return;

	}
	send_to_char( "You fail to work the controls properly.\r\n", ch );
	echo_to_ship( AT_YELLOW, target, "The ship shudders and then stops as a tractorbeam attemps to lock on." );
	snprintf( buf, MAX_STRING_LENGTH, "The %s attempted to capture your ship!", ship->name );
	echo_to_cockpit( AT_BLOOD, target, buf );
	if( autofly( target ) && !target->target0 )
		target->target0 = ship;


	learn_from_failure( ch, gsn_tractorbeams );

	return;
}

CMDF( do_request )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance = 0;
	SHIP_DATA *ship;
	SHIP_DATA *eShip = NULL;

	strcpy( arg, argument );

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!", ch );
		return;
	}

	if( !ship->starsystem )
	{
		send_to_char( "&RYou can't do that until you've finished launching!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "&RRequest the opening of the baydoors of what ship?\r\n", ch );
		return;
	}

	eShip = get_ship_here( arg, ship->starsystem );

	if( eShip == NULL )
	{
		send_to_char( "&RThat ship isn't here!\r\n", ch );
		return;
	}

	if( eShip == ship )
	{
		send_to_char( "&RIf you have bay doors, why not open them yourself?\r\n", ch );
		return;
	}

	if( eShip->hanger == 0 )
	{
		send_to_char( "&RThat ship has no hanger!", ch );
		return;
	}

	if( !autofly( eShip ) )
	{
		send_to_char( "&RThe other ship needs to have its autopilot turned on.\r\n", ch );
		return;
	}
	if( abs( eShip->vx - ship->vx ) > 100 * ( ( ship->comm ) + ( eShip->comm ) + 20 ) ||
		abs( eShip->vy - ship->vy ) > 100 * ( ( ship->comm ) + ( eShip->comm ) + 20 ) ||
		abs( eShip->vz - ship->vz ) > 100 * ( ( ship->comm ) + ( eShip->comm ) + 20 ) )

	{
		send_to_char( "&RThat ship is out of the range of your comm system.\r\n&w", ch );
		return;
	}

	if( abs( eShip->vx - ship->vx ) > 100 * ( ship->sensor + 10 ) * ( ( eShip->ship_class ) + 1 ) ||
		abs( eShip->vy - ship->vy ) > 100 * ( ship->sensor + 10 ) * ( ( eShip->ship_class ) + 1 )
		|| abs( eShip->vz - ship->vz ) > 100 * ( ship->sensor + 10 ) * ( ( eShip->ship_class ) + 1 ) )
	{
		send_to_char( "&RThat ship is too far away to remotely open bay doors.\r\n", ch );
		return;
	}


	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_shipsystems] );
	if( ( eShip->ship_class == SHIP_PLATFORM ? 1 : ( number_percent( ) >= chance ) ) && !check_pilot( ch, eShip ) )
	{
		send_to_char( "&RHey! That's not your ship!", ch );
		return;
	}

	if( eShip->bayopen == true )
	{
		send_to_char( "&RThat ship's bay doors are already open!\r\n", ch );
		return;
	}
	if( chance && !check_pilot( ch, eShip ) )
		learn_from_success( ch, gsn_shipsystems );

	send_to_char( "&RYou open the bay doors of the remote ship.", ch );
	act( AT_PLAIN, "$n flips a switch on the control panel.", ch, NULL, argument, TO_ROOM );
	eShip->bayopen = true;
	snprintf( buf, MAX_STRING_LENGTH, "%s's bay doors open.", eShip->name );
	echo_to_system( AT_YELLOW, ship, buf, NULL );
}

CMDF( do_pluogus )
{
	bool ch_comlink = false;
	OBJ_DATA *obj;
	int next_planet, itt;

	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if( obj->pIndexData->item_type == ITEM_COMLINK )
			ch_comlink = true;
	}

	if( !ch_comlink )
	{
		send_to_char( "You'll need a CDI for that Information.\r\n", ch );
		return;
	}

	send_to_char( "\r\n&pGrabbing Online Info&P..\r\n...\r\n..&pFound!", ch );
	send_to_char( "\r\n\r\n&YStar Shuttle next stops:\r\n", ch );
	//    turbocar = ship_from_cockpit( ROOM_CORUSCANT_TURBOCAR );     
	//     if ( bus_pos < 7 && bus_pos > 1 )

	ch_printf( ch, "\r\n&GCurrently on&g: " );

	//  if ( turbocar_stop == MAX_STATION )
	//    ch_printf( ch, "&CNew Saturn" );
	//station_name[turbocar_stop+MAX_STATION-1] );
	//  else

	ch_printf( ch, "&C%s", station_name[turbocar_stop - 1] );


	next_planet = turbocar_stop;
	send_to_char( "\r\n&GNext stops&g:&C ", ch );

	//     if ( bus_pos <= 1 )
	ch_printf( ch, "%s  ", station_name[turbocar_stop] );


	for( itt = 0; itt < 5; itt++ )
	{
		next_planet++;
		if( next_planet >= MAX_STATION )
			next_planet = 0;
		ch_printf( ch, "%s  ", station_name[next_planet] );
	}

	ch_printf( ch, "\r\n\r\n&pLogging off&P...\r\n" );

	/*
		 send_to_char( "Serin Tocca Schedule Information:\r\n", ch );


		 if ( bus_pos < 7 && bus_pos > 1 )
			ch_printf( ch, "The Tocca is Currently docked at %s.\r\n", bus_stop[bus2_planet] );


		 next_planet = bus2_planet;
		 send_to_char( "Next stops: ", ch);

		 if ( bus_pos <= 1 )
			ch_printf( ch, "%s  ", bus_stop[next_planet] );

		 for ( itt = 0 ; itt < 3 ; itt++ )
		 {
			 next_planet++;
			 if ( next_planet >= MAX_BUS_STOP )
				next_planet = 0;
			 ch_printf( ch, "%s  ", bus_stop[next_planet] );
		 }

		 ch_printf( ch, "\r\n" );
	*/
}


CMDF( do_fly )
{
}

CMDF( do_chaff )
{
	int chance;
	SHIP_DATA *ship;


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}


	if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RThe controls are at the copilots seat!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn the autopilot off first...\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}
	if( ship->chaff <= 0 )
	{
		send_to_char( "&RYou don't have any chaff to release!\r\n", ch );
		return;
	}
	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_weaponsystems] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou can't figure out which switch it is.\r\n", ch );
		learn_from_failure( ch, gsn_weaponsystems );
		return;
	}

	ship->chaff--;

	ship->chaff_released++;

	send_to_char( "You flip the chaff release switch.\r\n", ch );
	act( AT_PLAIN, "$n flips a switch on the control pannel", ch, NULL, argument, TO_ROOM );
	echo_to_cockpit( AT_YELLOW, ship, "A burst of chaff is released from the ship." );

	learn_from_success( ch, gsn_weaponsystems );

}

bool autofly( SHIP_DATA *ship )
{

	if( !ship )
		return false;

	if( ship->type == MOB_SHIP )
		return true;

	if( ship->autopilot )
		return true;

	return false;

}

CMDF( do_transship )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int arg3;
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	ship = get_ship( arg1 );
	if( !ship )
	{
		send_to_char( "No such ship.\r\n", ch );
		return;
	}

	arg3 = atoi( arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
	{
		send_to_char( "Usage: transship <ship> <vnum>\r\n", ch );
		return;
	}

	ship->shipyard = arg3;
	ship->shipstate = SHIP_READY;

	if( ship->ship_class != SHIP_PLATFORM && ship->type != MOB_SHIP )
	{
		extract_ship( ship );
		ship_to_room( ship, ship->shipyard );

		ship->location = ship->shipyard;
		ship->lastdoc = ship->shipyard;
		ship->shipstate = SHIP_DOCKED;
	}

	if( ship->starsystem )
		ship_from_starsystem( ship, ship->starsystem );

	save_ship( ship );
	send_to_char( "Ship Transfered.\r\n", ch );
}

CMDF( do_chaingun )
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
	/*
			if ( ship->stype > ARIES_SUIT )
					{
						send_to_char("&RThis isn't A Leo Or Aries!\r\n",ch);
						return;
					}
	*/
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
		send_to_char( "&RTheres not enough ammo left to fire!\r\n", ch );
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

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

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
		if( abs( target->vx - ship->vx ) > 800 || abs( target->vy - ship->vy ) > 800 || abs( target->vz - ship->vz ) > 800 )
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
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Chain Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Chain Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			snprintf( buf, MAX_STRING_LENGTH, "Chain Gun from %s barely misses %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Chain Gun from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Chain Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Chain Gun fire  hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 10, 11, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 10, 11, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 10, 11, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 10, 11, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 10, 11, ch );
		else
			damage_ship_ch( target, 10, 11, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

}



CMDF( do_beamsabre )
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
		send_to_char( "&RTheres not enough energy left to fire!\r\n", ch );
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

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_beamsabers] / 3 );

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
		if( abs( target->vx - ship->vx ) > 175 || abs( target->vy - ship->vy ) > 175 || abs( target->vz - ship->vz ) > 175 )
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
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s swings at you but misses.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You swing the Beam Sabre at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_beamsabers );
			snprintf( buf, MAX_STRING_LENGTH, "%s swings a Beam Sabre at %s but misses.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "%s hits %s with a Beam Sabre.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit a Beam Sabre from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "You  hit %s with the Beam Sabre!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_beamsabers );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 7, 14, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 7, 14, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 7, 14, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 7, 14, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 7, 14, ch );
		else
			damage_ship_ch( target, 7, 14, ch );

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


CMDF( do_lasergun )
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
	/*
			if ( ship->stype != TAURUS_SUIT )
					{
						send_to_char("&RThis isn't A TAURUS!\r\n",ch);
						return;
					}
	*/
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
	if( ship->energy < 5 )
	{
		send_to_char( "&RTheres not enough energy left to fire!\r\n", ch );
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

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_lightenergy] / 3 );

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
		if( abs( target->vx - ship->vx ) > 950 || abs( target->vy - ship->vy ) > 950 || abs( target->vz - ship->vz ) > 950 )
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
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Laser Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Laser Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_lightenergy );
			snprintf( buf, MAX_STRING_LENGTH, "Laser Gun fire from %s barely misses %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Laser Gun from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Laser Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Laser Gun fire  hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_lightenergy );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
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
		ship->energy -= 5;
		return;
	}

}
CMDF( do_minigun )
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
	/*
			if ( ship->stype != SERPENT_SUIT )
					{
						send_to_char("&RThis isn't A Serpent!\r\n",ch);
						return;
					}
	*/
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
		send_to_char( "&RTheres not enough energy left to fire!\r\n", ch );
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

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_bulletweapons] / 3 );

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
		if( abs( target->vx - ship->vx ) > 500 || abs( target->vy - ship->vy ) > 500 || abs( target->vz - ship->vz ) > 500 )
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
		chance = URANGE( 10, chance, 90 );
		act( AT_PLAIN, "$n presses the fire button.", ch, NULL, argument, TO_ROOM );
		if( number_percent( ) > chance )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Mini Gun fire from %s misses you.", ship->name );
			echo_to_cockpit( AT_ORANGE, target, buf );
			snprintf( buf, MAX_STRING_LENGTH, "You shoot the Mini Gun  at %s but miss.", target->name );
			echo_to_cockpit( AT_ORANGE, ship, buf );
			learn_from_failure( ch, gsn_bulletweapons );
			snprintf( buf, MAX_STRING_LENGTH, "Mini Gun from %s barely misses %s.", ship->name, target->name );
			echo_to_system( AT_ORANGE, ship, buf, target );
			return;
		}
		snprintf( buf, MAX_STRING_LENGTH, "Mini Gun fire from %s hits %s.", ship->name, target->name );
		echo_to_system( AT_ORANGE, ship, buf, target );
		snprintf( buf, MAX_STRING_LENGTH, "You are hit by Mini Gun fire from %s!", ship->name );
		echo_to_cockpit( AT_BLOOD, target, buf );
		snprintf( buf, MAX_STRING_LENGTH, "Your Mini Gun fire hits %s!.", target->name );
		echo_to_cockpit( AT_YELLOW, ship, buf );
		learn_from_success( ch, gsn_bulletweapons );
		echo_to_ship( AT_RED, target, "A small explosion vibrates through the ship." );
		if( ship->targettype == 1 )
			damage_ship_ch( target, 13, 15, ch );
		else if( ship->targettype == 2 )
			damage_ship_chhead( target, 13, 15, ch );
		else if( ship->targettype == 3 )
			damage_ship_chlarm( target, 13, 15, ch );
		else if( ship->targettype == 4 )
			damage_ship_chrarm( target, 13, 15, ch );
		else if( ship->targettype == 5 )
			damage_ship_chlegs( target, 13, 15, ch );
		else
			damage_ship_ch( target, 13, 15, ch );

		if( autofly( target ) && target->target0 != ship )
		{
			target->target0 = ship;
			snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", target->name );
			echo_to_cockpit( AT_BLOOD, ship, buf );
		}

		return;
	}

}

CMDF( do_selfdestruct )
{
	SHIP_DATA *ship;
	//    SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];


	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in your suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, self destruct in a suit you own!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ch->position == POS_FIGHTING )
	{
		send_to_char( "You can't do that while fighting!\r\n", ch );
		return;
	}

	if( !str_cmp( "Public", ship->owner ) )
	{
		send_to_char( "You can't do that in a public shuttle!\r\n", ch );
		return;
	}

	if( !ship->starsystem )
	{
		send_to_char( "&RYou can't do that until you've finished launching!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn off the ships autopilot first....\r\n", ch );
		return;
	}

	if( !str_cmp( "Holosuit", ship->owner ) )
	{
		echo_to_ship( AT_WHITE, ship, "&GThe image of space and battle flickers out as your fight comes to an end.\r\n" );
		snprintf( buf, MAX_STRING_LENGTH, "The image of %s flickers out of sight.", ship->name );
		echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );
		resetship( ship );
		return;
	}

	if( !xIS_SET( ch->act, PLR_PKER ) )
	{
		send_to_char( "&RIf you want to selfdestruct, go PK!!\r\n", ch );
		return;
	}

	snprintf( buf, MAX_STRING_LENGTH, "%s explodes in a blinding flash of light!", ship->name );
	echo_to_system( AT_WHITE + AT_BLINK, ship, buf, NULL );

	if( ship->ship_class == MOBILE_SUIT )

		snprintf( buf, MAX_STRING_LENGTH, "&G&pA &zF&Wl&za&Ws&zh &pof &YL&Oi&Yg&Oh&Yt &GA&gp&cp&Ce&ca&gr&Gs &RI&rn &PT&ph&Pe &BS&bk&By&C.&c.&C." );
	info_chan( buf );
	snprintf( buf, MAX_STRING_LENGTH,
		"&GS&gadly&B.. &z%s &Yc&Oh&Yo&Os&Ye &RD&re&Ra&rt&Rh&B, &PS&pe&Pl&pf&B-&CD&ce&Cs&ct&Cr&cu&Cc&ct&Ci&cn&Cg &Yi&On &G%s.\r\n",
		ch->name, ship->name );
	info_chan( buf );
	/*
	 * for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
	 * {
	 * if ( target != ship )
	 * {
	 * if ( abs(target->vx - ship->vx) < 1000+ship->sensor
	 * && abs(target->vy - ship->vy) < 1000+ship->sensor
	 * && abs(target->vz - ship->vz) < 1000+ship->sensor
	 * && target->pksuit == 1 )
	 *
	 * {
	 * damage_ship_chhead( target , 5000 , 5000 , ch );
	 * damage_ship_chlarm( target , 5000 , 5000 , ch );
	 * damage_ship_chrarm( target , 5000 , 5000 , ch );
	 * damage_ship_chlegs( target , 5000 , 5000 , ch );
	 * damage_ship_ch( target , 5000 , 5000 , ch );
	 * }
	 * else
	 * {
	 * ch_printf( ch, "%s was unaffected by your selfdestruct!\r\n", target->name );
	 * }
	 * }
	 * }
	 */
	snprintf( log_buf, MAX_STRING_LENGTH, "%s has just Self Destructed.", ch->name );

	log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
	destroy_ship( ship, NULL );

}

CMDF( do_targetlock )
{
	char arg1[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "\n\rSyntax: targetlock <Suit Part>\r\n", ch );
		send_to_char( "Parts: Torso, Head, Leftarm, Rightarm, Legs, None.\r\n", ch );
		return;
	}
	/*
		if ( !check_pilot( ch , ship ) )
		{
		send_to_char("&RDo it in your own suit!\r\n",ch);
		return;
		}
	*/
	if( !str_cmp( arg1, "None" ) )
	{
		ship->targettype = 0;
		send_to_char( "Targetlock cleared.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to nothing.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg1, "Torso" ) )
	{
		ship->targettype = 1;
		send_to_char( "Target locked on suit torso's.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to torso's.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg1, "Head" ) )
	{
		ship->targettype = 2;
		send_to_char( "Target locked on suit heads.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to heads.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg1, "Leftarm" ) )
	{
		ship->targettype = 3;
		send_to_char( "Target locked on suit Left Arms.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to left arms.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg1, "Rightarm" ) )
	{
		ship->targettype = 4;
		send_to_char( "Target locked on suit Right Arms.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to right arms.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg1, "Legs" ) )
	{
		ship->targettype = 5;
		send_to_char( "Target locked on suit Legss.\r\n", ch );
		act( AT_PLAIN, "$n adjusts targetting to legs.", ch, NULL, argument, TO_ROOM );
		save_ship( ship );
		return;
	}

	do_targetlock( ch, "" );
	return;
}


CMDF( do_customize )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
	{
		send_to_char( "\n\rSyntax: customize <suit> <field> <value>\r\n", ch );
		send_to_char( "Fields: Description, Nickname, Colorone, Colortwo, Code.\r\n", ch );
		return;
	}

	ship = get_ship( arg1 );
	if( !ship )
	{
		send_to_char( "No such ship.\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RCustomize your own suit!\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "description" ) )
	{
		STRFREE( ship->description );
		ship->description = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "code" ) )
	{
		ship->code = URANGE( 00, atoi( argument ), 99 );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "nickname" ) )
	{
		STRFREE( ship->nickname );
		ship->nickname = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "colorone" ) )
	{
		STRFREE( ship->colorone );
		ship->colorone = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	if( !str_cmp( arg2, "colortwo" ) )
	{
		STRFREE( ship->colortwo );
		ship->colortwo = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		save_ship( ship );
		return;
	}

	do_customize( ch, "" );
	return;
}




CMDF( do_spunch )
{
}

CMDF( do_zerosystem )
{
	int chance;
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey! Thats not your ship!\r\n", ch );
		return;
	}

	if( ship->mod != 1 )
	{
		send_to_char( "&RThis suit isn't equipped with the Zero System!!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RWait until after you launch!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that unless the ship is flying in realspace!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_zerosystem] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou cant figure out how to turn it on!\r\n", ch );
		learn_from_failure( ch, gsn_zerosystem );
		return;
	}

	if( ship->offon == 0 )
	{
		ship->offon = 1;
		SET_BIT( ch->pcdata->flags, PCFLAG_ZERO );
		send_to_char( "You flip the emergency switch up and smash the button.\n", ch );
		send_to_char( "&YThe lights dim.. as you engaged the &zZ&We&zr&Wo &CS&cy&Cs&ct&Ce&cm&Y..\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s seems to move and act differently...", ship->name );
		echo_to_system( AT_GREEN, ship, buf, NULL );
		save_ship( ship );
		learn_from_success( ch, gsn_zerosystem );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}
	else
	{
		ship->offon = 0;
		REMOVE_BIT( ch->pcdata->flags, PCFLAG_ZERO );
		send_to_char( "You push a button, turning off the Zero System.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s slows down slightly, resuming its normal stance.", ship->name );
		echo_to_system( AT_YELLOW, ship, buf, NULL );
		save_ship( ship );
		learn_from_success( ch, gsn_zerosystem );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	learn_from_success( ch, gsn_zerosystem );
	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
}

CMDF( do_transform )
{
	int chance;
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey! Thats not your ship!\r\n", ch );
		return;
	}

	if( ship->mod != 3 )
	{
		send_to_char( "&RThis suit can't Transform!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a Mobile Suit!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RWait until after you launch!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that unless the ship is flying in realspace!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_transform] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou cant figure out what button to press!\r\n", ch );
		learn_from_failure( ch, gsn_transform );
		return;
	}

	if( ship->offon == 0 )
	{
		ship->offon = 1;
		ship->realspeed = ship->realspeed * 2;
		ship->hyperspeed = ship->hyperspeed * 2;
		send_to_char( "Your suit transforms into Jet Mode", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s transforms into &GJ&get &CM&code&Y.", ship->name );
		echo_to_system( AT_YELLOW, ship, buf, NULL );
		save_ship( ship );
		learn_from_success( ch, gsn_transform );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}
	else
	{
		ship->offon = 0;
		ship->realspeed = ship->realspeed / 2;
		ship->currspeed = ship->realspeed;
		ship->hyperspeed = ship->hyperspeed / 2;
		send_to_char( "Your suit transforms into Battle Mode!", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s transforms into &GB&gattle &CM&code&Y.", ship->name );
		echo_to_system( AT_YELLOW, ship, buf, NULL );
		save_ship( ship );
		learn_from_success( ch, gsn_transform );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	learn_from_success( ch, gsn_transform );
	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
}

CMDF( do_status )
{
	int chance;
	SHIP_DATA *ship;
	SHIP_DATA *target;

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit, turret or engineroom of a ship to do that!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
		target = ship;
	else
		target = get_ship_here( argument, ship->starsystem );

	if( target == NULL )
	{
		send_to_char( "&RI don't see that here.\n\rTry the radar, or type status by itself for your ships status.\r\n", ch );
		return;
	}

	if( ship->armorhead < 1 )
	{
		send_to_char( "&RYou can't use that! Your suits head is destroyed!\r\n", ch );
		return;
	}

	if( abs( target->vx - ship->vx ) > 500 + ship->sensor * 2 ||
		abs( target->vy - ship->vy ) > 500 + ship->sensor * 2 || abs( target->vz - ship->vz ) > 500 + ship->sensor * 2 )
	{
		send_to_char( "&RThat ship is to far away to scan.\r\n", ch );
		return;
	}

	if( IS_SET( ship->flags, SUIT_NEWSYSTEM ) )
	{
		landstat( ch, ship );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_shipsystems] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou cant figure out what the readout means.\r\n", ch );
		learn_from_failure( ch, gsn_shipsystems );
		return;
	}

	act( AT_PLAIN, "$n checks various gages and displays on the control panel.", ch, NULL, argument, TO_ROOM );

	if( ship->ship_class == MOBILE_SUIT )
	{
		send_to_char( "\r\n&c|----------------------------------|-----------------|\r\n", ch );
		ch_printf( ch, "&c|          &CHE&B: &z%5d&B/&z%-5d         &c| &CEN&B: &z%-5d&B/&z%5d &c|\r\n",
			ship->armorhead, ship->maxarmorhead, ship->energy, ship->maxenergy );
		send_to_char( "&c|                &Y|                 &c|-----------------|\r\n", ch );
		ch_printf( ch, "&c| &CLA&B: &z%4d&B/&z%-4d &Y-|- &CRA&B: &z%4d&B/&z%-4d  &c| &CSP&B:     &z%3d&B/&z%-3d &c|\r\n",
			ship->armorlarm, ship->maxarmorlarm, ship->armorrarm,
			ship->maxarmorrarm, ship->currspeed, ship->realspeed );
		send_to_char( "&c|                &Y|                 &c|-----------------|\r\n", ch );
		ch_printf( ch, "&c|          &CTO&B: &z%5d&B/&z%-5d         &c| &CAM&B:  &z%4d&B/&z%-4d  &c|\r\n",
			ship->armor, ship->maxarmor, ship->ammo, ship->maxammo );
		send_to_char( "&c|              &Y / \\                &c|-----------------|\r\n", ch );
		ch_printf( ch, "&c|          &CLE&B: &z%4d&B/&z%-4d           &c| &CTP&B: &G%-9s   &c|\r\n",
			ship->armorlegs, ship->maxarmorlegs,
			ship->targettype == 1 ? "Torso" : ship->targettype == 2 ? "Head" :
			ship->targettype == 3 ? "Left Arm" : ship->targettype == 4 ? "Right Arm" :
			ship->targettype == 5 ? "Legs" : "None" );

		send_to_char( "&c|__________________________________|_________________|\r\n", ch );
		ch_printf( ch, " &CTarget&B:&z %s \r\n", ship->target0 ? ship->target0->name : "None" );
		ch_printf( ch, " &CCurrent Cords&B: &z%.0f %.0f %.0f           \r\n", ship->vx, ship->vy, ship->vz );
		ch_printf( ch, " &CHeading Cords&B: &z%.0f %.0f %.0f           \r\n", target->hx, target->hy, target->hz );

		send_to_char( "&c|----------------------------------------------------|\r\n", ch );
		learn_from_success( ch, gsn_shipsystems );
	}
	else
	{

		ch_printf( ch, "\r\n&z%s Status&B:\r\n&B====================================================\r\n", target->name );
		ch_printf( ch, "&zSpeed&B:&W %3d&B/&W%3d   &zCoordinates&B:&W %.0f %.0f %.0f\r\n",
			target->currspeed, target->realspeed, target->vx, target->vy, target->vz );
		ch_printf( ch, "&z                     Heading&B:&W %.0f %.0f %.0f\r\n", target->hx, target->hy, target->hz );
		ch_printf( ch, "&zArmor&B:&W %d&B/&W%d     &zShields&B:&W %d&B/&W%d\r\n",
			target->armor, target->maxarmor, target->shield, target->maxshield );
		ch_printf( ch, "&zFuel&B:&W %d&B/&W%d    &zCondition&B:&W %s\r\n",
			target->energy, target->maxenergy, target->shipstate == SHIP_DISABLED ? "Disabled" : "Running" );
		if( target->turret1 )
			ch_printf( ch, "&zTurret One&B:&W %s  &zCurrent Target&B:&W %s\r\n",
				target->statet1 == LASER_DAMAGED ? "Damaged" : "Good",
				target->target1 ? target->target1->name : "none" );
		if( target->turret2 )
			ch_printf( ch, "&zTurret Two&B:&W %s  &zCurrent Target&B:&W %s\r\n",
				target->statet2 == LASER_DAMAGED ? "Damaged" : "Good",
				target->target2 ? target->target2->name : "none" );
		ch_printf( ch, "&zTarget&B:&W %s\r\n", target->target0 ? target->target0->name : "None" );

		ch_printf( ch, "&zAmmo&B:&W %d&B/&W%d\r\n", target->ammo, target->maxammo );

		ch_printf( ch, "&zFirstweapon&B:&W %s\r\n",
			target->firstweapon == 0 ? "100mm Machine Gun" :
			( target->firstweapon == 1 ? "105mm Rifle" :
				( target->firstweapon == 2 ? "Leo Bazooka" :
					( target->firstweapon == 3 ? "Beam Sabre" :
						( target->firstweapon == 4 ? "Shoulder Mounted Energy Cannon" :
							( target->firstweapon == 5 ? "Side Mounted Missile Launchers" :
								( target->firstweapon == 6 ? "Beam Rifle" :
									( target->firstweapon == 7 ? "Laser Cannon" :
										( target->firstweapon == 8 ? "Twin-Gattling Gun" :
											( target->firstweapon == 9 ? "Beam Cannon" :
												( target->firstweapon == 10 ? "Buster Rifle" :
													( target->firstweapon == 11 ? "Head Vulcan" :
														( target->firstweapon == 12 ? "Beam Scythe" :
															( target->firstweapon == 13 ? "Buster Shield" :
																( target->firstweapon == 14 ? "Beam Gattling" :
																	( target->firstweapon == 15 ? "Multi Blast" :
																		( target->firstweapon == 16 ? "Army Knife" :
																			( target->firstweapon == 17 ? "Shatols" :
																				( target->firstweapon == 18 ? "Shoulder Missiles" :
																					( target->firstweapon == 19 ? "Cross Crusher" :
																						( target->firstweapon == 20 ? "Big Beam Sabre" :
																							( target->firstweapon == 21 ? "Heat Rod" :
																								( target->firstweapon == 22 ? "Beam Glaive" :
																									( target->firstweapon == 23 ? "Dragon Fang" :
																										( target->firstweapon == 24 ? "Flamethrower" :
																											( target->firstweapon == 25 ? "Dober Gun" :
																												( target->firstweapon == 26 ? "Short Blast" :
																													( target->firstweapon == 27 ? "Long Blast" :
																														( target->firstweapon == 28 ? "Small Beam Cannon" :
																															( target->firstweapon == 29 ? "Beam Blade" :
																																( target->firstweapon ==
																																	30 ? "Planet Denfensers" : ( target->
																																		firstweapon ==
																																		31 ?
																																		"Libra Main Cannon"
																																		: ( target->
																																			firstweapon ==
																																			39 ?
																																			"250mm Machine Gun"
																																			:
																																			"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );


		ch_printf( ch, "&zSecondweapon&B:&W %s\r\n",
			target->secondweapon == 0 ? "100mm Machine Gun" :
			( target->secondweapon == 1 ? "105mm Rifle" :
				( target->secondweapon == 2 ? "Leo Bazooka" :
					( target->secondweapon == 3 ? "Beam Sabre" :
						( target->secondweapon == 4 ? "Shoulder Mounted Energy Cannon" :
							( target->secondweapon == 5 ? "Side Mounted Missile Launchers" :
								( target->secondweapon == 6 ? "Beam Rifle" :
									( target->secondweapon == 7 ? "Laser Cannon" :
										( target->secondweapon == 8 ? "Twin-Gattling Gun" :
											( target->secondweapon == 9 ? "Beam Cannon" :
												( target->secondweapon == 10 ? "Buster Rifle" :
													( target->secondweapon == 11 ? "Head Vulcan" :
														( target->secondweapon == 12 ? "Beam Scythe" :
															( target->secondweapon == 13 ? "Buster Shield" :
																( target->secondweapon == 14 ? "Beam Gattling" :
																	( target->secondweapon == 15 ? "Multi Blast" :
																		( target->secondweapon == 16 ? "Army Knife" :
																			( target->secondweapon == 17 ? "Shatols" :
																				( target->secondweapon == 18 ? "Shoulder Missiles" :
																					( target->secondweapon == 19 ? "Cross Crusher" :
																						( target->secondweapon == 19 ? "Cross Crusher" :
																							( target->secondweapon == 20 ? "Big Beam Sabre" :
																								( target->secondweapon == 21 ? "Heat Rod" :
																									( target->secondweapon == 22 ? "Beam Glaive" :
																										( target->secondweapon == 23 ? "Dragon Fang" :
																											( target->secondweapon == 24 ? "Flamethrower" :
																												( target->secondweapon == 25 ? "Dober Gun" :
																													( target->secondweapon == 26 ? "Short Blast" :
																														( target->secondweapon == 27 ? "Long Blast" :
																															( target->secondweapon == 28 ? "Small Beam Cannon" :
																																( target->secondweapon == 29 ? "Beam Blade" :
																																	( target->secondweapon ==
																																		30 ? "Planet Denfensers" : ( target->
																																			secondweapon ==
																																			31 ?
																																			"Libra Main Cannon"
																																			: ( target->
																																				secondweapon
																																				==
																																				39 ?
																																				"250mm Machine Gun"
																																				:
																																				"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );


		ch_printf( ch, "&zThirdweapon&B:&W %s\r\n&B====================================================\r\n",
			target->thirdweapon == 0 ? "100mm Machine Gun" :
			( target->thirdweapon == 1 ? "105mm Rifle" :
				( target->thirdweapon == 2 ? "Leo Bazooka" :
					( target->thirdweapon == 3 ? "Beam Sabre" :
						( target->thirdweapon == 4 ? "Shoulder Mounted Energy Cannon" :
							( target->thirdweapon == 5 ? "Side Mounted Missile Launchers" :
								( target->thirdweapon == 6 ? "Beam Rifle" :
									( target->thirdweapon == 7 ? "Laser Cannon" :
										( target->thirdweapon == 8 ? "Twin-Gattling Gun" :
											( target->thirdweapon == 9 ? "Beam Cannon" :
												( target->thirdweapon == 10 ? "Buster Rifle" :
													( target->thirdweapon == 11 ? "Head Vulcan" :
														( target->thirdweapon == 12 ? "Beam Scythe" :
															( target->thirdweapon == 13 ? "Buster Shield" :
																( target->thirdweapon == 14 ? "Beam Gattling" :
																	( target->thirdweapon == 15 ? "Multi Blast" :
																		( target->thirdweapon == 16 ? "Army Knife" :
																			( target->thirdweapon == 17 ? "Shatols" :
																				( target->thirdweapon == 18 ? "Shoulder Missiles" :
																					( target->thirdweapon == 19 ? "Cross Crusher" :
																						( target->thirdweapon == 19 ? "Cross Crusher" :
																							( target->thirdweapon == 20 ? "Big Beam Sabre" :
																								( target->thirdweapon == 21 ? "Heat Rod" :
																									( target->thirdweapon == 22 ? "Beam Glaive" :
																										( target->thirdweapon == 23 ? "Dragon Fang" :
																											( target->thirdweapon == 24 ? "Flamethrower" :
																												( target->thirdweapon == 25 ? "Dober Gun" :
																													( target->thirdweapon == 26 ? "Short Blast" :
																														( target->thirdweapon == 27 ? "Long Blast" :
																															( target->thirdweapon == 28 ? "Small Beam Cannon" :
																																( target->thirdweapon == 29 ? "Beam Blade" :
																																	( target->thirdweapon ==
																																		30 ? "Planet Denfensers" : ( target->
																																			thirdweapon ==
																																			31 ?
																																			"Libra Main Cannon"
																																			: ( target->
																																				thirdweapon
																																				==
																																				39 ?
																																				"250mm Machine Gun"
																																				:
																																				"None" ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );

		learn_from_success( ch, gsn_shipsystems );
	}
}


CMDF( do_evade )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	int chance;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit or turret of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Evade to where?\r\n", ch );
		send_to_char( "Back, up, down, left, right?\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RWait until after you launch!\r\n", ch );
		return;
	}

	if( ship->currspeed <= 10 )
	{
		send_to_char( "&RYou'll have to speed up for that!\r\n", ch );
		return;
	}

	if( ship->energy <= 10 )
	{
		send_to_char( "&RYou don't have enough energy for that!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that unless the ship is flying in realspace!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level
		: ( int ) ( ch->perm_dex * 2 + ch->pcdata->learned[gsn_spacecombat] / 3
			+ ch->pcdata->learned[gsn_spacecombat2] / 3 + ch->pcdata->learned[gsn_spacecombat3] / 3 );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		learn_from_failure( ch, gsn_navigation );
		learn_from_failure( ch, gsn_spacecombat2 );
		learn_from_failure( ch, gsn_spacecombat3 );
		return;
	}

	if( !str_cmp( arg, "back" ) )
	{
		send_to_char( "&GYou fire up the thrusters and evade backwards.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "&gThe thrusters propel the suit backwards." );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, flying backwards.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
		ship->vx -= 200;
		ship->energy -= 10;
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	if( !str_cmp( arg, "up" ) )
	{
		send_to_char( "&GYou fire up the thrusters and evade upwards.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "&gThe thrusters propel the suit upwards." );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, flying upwards.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
		ship->vy -= 200;
		ship->energy -= 10;
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	if( !str_cmp( arg, "down" ) )
	{
		send_to_char( "&GYou fire up the thrusters and evade downwards.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "&gThe thrusters propel the suit downwards." );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, flying downwards.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
		ship->vy += 200;
		ship->energy -= 10;
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	if( !str_cmp( arg, "left" ) )
	{
		send_to_char( "&GYou fire up the thrusters and evade to the left.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "&gThe thrusters propel the suit to the left." );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, flying to the left.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
		ship->vz -= 200;
		ship->energy -= 10;
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	if( !str_cmp( arg, "right" ) )
	{
		send_to_char( "&GYou fire up the thrusters and evade to the right.\r\n", ch );
		echo_to_cockpit( AT_YELLOW, ship, "&gThe thrusters propel the suit to the right." );
		snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, flying to the right.", ship->name );
		echo_to_system( AT_ORANGE, ship, buf, NULL );
		ship->vz += 200;
		ship->energy -= 10;
		learn_from_success( ch, gsn_spacecombat );
		learn_from_success( ch, gsn_spacecombat2 );
		learn_from_success( ch, gsn_spacecombat3 );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
		return;
	}

	do_targetlock( ch, "" );
	return;
}

void makemine( SHIP_DATA *ship )
{
	SHIP_DATA *mine;

	//  if ( ship->ship_class == SMINE )
	//    return;

	CREATE( mine, SHIP_DATA, 1 );

	LINK( mine, first_ship, last_ship, next, prev );

	mine->owner = ship->owner;
	mine->copilot = STRALLOC( "" );
	mine->pilot = STRALLOC( "" );
	mine->home = STRALLOC( "" );
	mine->type = SHIP_CIVILIAN;
	mine->ship_class = SMINE;
	mine->lasers = 0;
	mine->missiles = 0;
	mine->rockets = 0;
	mine->torpedos = 0;
	mine->maxshield = 0;
	mine->armor = 500;
	mine->maxenergy = 5000;
	mine->hyperspeed = 0;
	mine->chaff = 0;
	mine->realspeed = 0;
	mine->currspeed = 0;
	mine->manuever = 100;
	mine->pksuit = 1;

	mine->energy = 1000;
	mine->maxarmor = 500;
	mine->in_room = NULL;
	mine->next_in_room = NULL;
	mine->prev_in_room = NULL;
	mine->currjump = NULL;
	mine->target0 = NULL;
	mine->target1 = NULL;
	mine->target2 = NULL;
	mine->autopilot = false;
	mine->shipstate = SHIP_READY;
	mine->name = STRALLOC( "Space Mine" );
	mine->description = STRALLOC( "A Space Mine" );
	ship_to_starsystem( mine, ship->starsystem );

	mine->vx = ship->vx;
	mine->vy = ship->vy;
	mine->vz = ship->vz;
	mine->hx = ship->hx;
	mine->hy = ship->hy;
	mine->hz = ship->hz;
	return;

}

CMDF( do_spacemine )
{
	int chance;
	SHIP_DATA *ship;


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a ship to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}


	if( ( ship = ship_from_coseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RThe controls are at the copilots seat!\r\n", ch );
		return;
	}

	if( autofly( ship ) )
	{
		send_to_char( "&RYou'll have to turn the autopilot off first...\r\n", ch );
		return;
	}

	if( ship->starsystem == NULL )
	{
		send_to_char( "&RYou can't do that unless the ship is flying in realspace!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_HYPERSPACE )
	{
		send_to_char( "&RYou can only do that in realspace!\r\n", ch );
		return;
	}
	if( ship->shipstate == SHIP_DOCKED )
	{
		send_to_char( "&RYou can't do that until after you've launched!\r\n", ch );
		return;
	}

	if( ship->mines <= 0 )
	{
		send_to_char( "&RYou don't have any space mines to plant!\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_spacemines] );
	if( number_percent( ) > chance )
	{
		send_to_char( "&RYou can't figure out which switch it is.\r\n", ch );
		learn_from_failure( ch, gsn_spacemines );
		return;
	}

	send_to_char( "You push the space mine plant trigger.\r\n", ch );
	act( AT_PLAIN, "$n pushes the space mine trigger.", ch, NULL, argument, TO_ROOM );
	echo_to_cockpit( AT_YELLOW, ship, "A space mine is planted, as it starts to glow red." );
	makemine( ship );
	ship->mines -= 1;
	learn_from_success( ch, gsn_spacemines );

}
