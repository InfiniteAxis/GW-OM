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


/* Local and Global Variables */
ch_ret drive_ship( CHAR_DATA *ch, SHIP_DATA *ship, EXIT_DATA *exit, int fall );
void transship( SHIP_DATA *ship, int destination );
void destroy_suit( CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *target );
void resetsuit( SHIP_DATA *ship );
void mobsuitattack( CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *target );
char *gridcheck( SHIP_DATA *ship, int gridnumber, int gridlevel );
bool can_move( SHIP_DATA *ship, int gnum );
bool can_attack( SHIP_DATA *ship, SHIP_DATA *target, int attacktype );
bool can_engage( SHIP_DATA *ship, SHIP_DATA *target, int attacktype );
void damagesuit( SHIP_DATA *ship, SHIP_DATA *target, int weapontype );

WEAPON_DATA *first_weapon;
WEAPON_DATA *last_weapon;

// Right = wone
// Left  = wtwo

#define PT_RIGHTARM      1
#define PT_LEFTARM       2

#define CT_EVADE         1
#define CT_BLOCK         2
#define CT_SHOOT         3

#define MOVE_UP          999
#define MOVE_DOWN        888
#define MOVE_STAY        777
#define MOVE_DONE        555

#define ARMOR_BASIC        0
#define ARMOR_PHASESHIFT   1
#define ARMOR_TRANSPHASE   2
#define ARMOR_PANZER       3
#define ARMOR_LAMINATED    4
#define ARMOR_ANTIBEAM     5

#define WT_NONE      0
#define WT_SLASH     1
#define WT_PIERCE    2
#define WT_BULLET    3
#define WT_BEAM      4
#define WT_MISSILE   5

const char *const suitweapon_type[] = {
   "None", "Slash", "Pierce", "Bullet", "Beam", "Missile"
};

const char *const suit_class[] = {
   "None?", "Leo", "Aries", "Taurus", "Serpent",
   "Clan", "Midship", "Gundam"
};


const char *armor_name( int vector )
{
	static char buf[512];

	buf[0] = '\0';
	if( vector & SUIT_AR_PHASESHIFT )
		strcat( buf, "| Phase Shift" );
	if( vector & SUIT_AR_TRANSPHASE )
		strcat( buf, "| Trans-Phase Armor" );
	if( vector & SUIT_AR_PANZER )
		strcat( buf, "| Geschmeidig Panzer" );
	if( vector & SUIT_AR_LAMINATED )
		strcat( buf, "| Laminated Armor" );
	if( vector & SUIT_AR_ANTIBEAM )
		strcat( buf, "| Anti-Beam Coating" );
	return ( buf[0] != '\0' ) ? buf + 1 : "| None";
}

const char *const weapon_list[] = {
   "None", "Beam Rifle"
};

int get_suit_weapon( const char *type )
{
	int x;

	for( x = 0; x < MAX_SUITWEAPON; x++ )
		if( !str_cmp( type, suitweapons[x] ) )
			return x;
	return -1;
}

struct suitfighting_data
{
	SHIP_DATA *who;
	/*
		int			xp;
		short		align;
		short		duration;
		short		timeskilled;
	*/
};

void set_suitfighting( SHIP_DATA *ship, SHIP_DATA *target )
{
	SUITFIGHT_DATA *suitfight;

	if( ship->fighting )
	{
		char buf[MAX_STRING_LENGTH];

		snprintf( buf, MSL, "Set_suitfighting: %s -> %s (already fighting %s)",
			ship->name, target->name, ship->fighting->who->name );
		bug( buf, 0 );
		return;
	}

	/*
	 * Limit attackers -Thoric
	 * if ( victim->num_fighting > max_fight(victim) )
	 * {
	 * send_to_char( "There are too many people fighting for you to join in.\r\n", ch );
	 * return;
	 * }
	 */

	CREATE( suitfight, SUITFIGHT_DATA, 1 );
	suitfight->who = target;

	ship->fighting = suitfight;

	SET_BIT( ship->flags, SUIT_INCOMBAT );
	SET_BIT( target->flags, SUIT_INCOMBAT );

	//    victim->num_fighting++;
	return;
}

SHIP_DATA *who_suitfighting( SHIP_DATA *ship )
{
	if( !ship )
	{
		bug( "who_suitfighting: null suit", 0 );
		return NULL;
	}
	if( !ship->fighting )
		return NULL;

	return ship->fighting->who;
}

void free_suitfight( SHIP_DATA *ship )
{
	if( !ship )
	{
		bug( "Free_suitfight: null suit!", 0 );
		return;
	}
	if( ship->fighting )
	{
		DISPOSE( ship->fighting );
	}
	return;
}

/*
 * Stop fights.
 */
void stop_suitfighting( SHIP_DATA *ship, bool fBoth )
{
	SHIP_DATA *fsuit;

	free_suitfight( ship );

	if( !fBoth ) /* major short cut here by Thoric */
		return;

	for( fsuit = first_ship; fsuit; fsuit = fsuit->next )
	{
		if( who_suitfighting( fsuit ) == ship )
		{
			free_suitfight( fsuit );
		}
	}
	return;
}

void extract_weapon( WEAPON_DATA *weapon )
{
	if( weapon == NULL )
		return;


	UNLINK( weapon, first_weapon, last_weapon, next, prev );

	weapon->target = NULL;
	weapon->fired_from = NULL;
	if( weapon->fired_by )
		STRFREE( weapon->fired_by );

	DISPOSE( weapon );

}

void new_weapon( SHIP_DATA *ship, SHIP_DATA *target, CHAR_DATA *ch, int weapontype )
{
	WEAPON_DATA *weapon;

	if( ship == NULL )
		return;

	if( target == NULL )
		return;

	CREATE( weapon, WEAPON_DATA, 1 );
	LINK( weapon, first_weapon, last_weapon, next, prev );

	weapon->target = target;
	weapon->fired_from = ship;
	weapon->time = 0;
	if( ch )
		weapon->fired_by = STRALLOC( ch->name );
	else
		weapon->fired_by = STRALLOC( "" );
	weapon->weapontype = weapontype;
}

//Handles -Everything- with the new suit battle system

void suit_timer( )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	WEAPON_DATA *weapon;
	WEAPON_DATA *next_weapon;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	float time = 0;
	int cnum;
	int wtype;

	for( weapon = first_weapon; weapon; weapon = next_weapon )
	{
		next_weapon = weapon->next;

		ship = weapon->fired_from;
		target = weapon->target;

		weapon->time += 0.5;

		if( weapon->weapontype == PT_RIGHTARM )
		{
			snprintf( name, MAX_STRING_LENGTH, "%s", suitweapon_table[ship->wrightarm].weapon_name );
			wtype = suitweapon_table[ship->wrightarm].weapon_type;
		}
		if( weapon->weapontype == PT_LEFTARM )
		{
			snprintf( name, MAX_STRING_LENGTH, "%s", suitweapon_table[ship->wleftarm].weapon_name );
			wtype = suitweapon_table[ship->wleftarm].weapon_type;
		}

		if( weapon->time == 1 )
		{
			cnum = number_range( 1, 3 );

			snprintf( buf, MAX_STRING_LENGTH, "%s begins an attack with its %s! &z[&g%s&z] &z[&g%s&z] &z[&g%s&z]", ship->name, name,
				cnum == 1 ? "&G1" : "1", cnum == 2 ? "&G2" : "2", cnum == 3 ? "&G3" : "3" );
			weapon->counter = cnum;
			echo_to_ship( AT_WHITE, target, buf );
		}
		if( weapon->time == 3 )
		{
			if( target->counter == weapon->counter )
			{
				if( ( wtype == WT_SLASH && target->countertype == CT_BLOCK )
					|| ( wtype == WT_BEAM && target->countertype == CT_BLOCK )
					|| ( wtype == WT_PIERCE && target->countertype == CT_BLOCK )
					|| ( wtype == WT_BULLET && target->countertype == CT_BLOCK ) )
				{
					snprintf( buf, MAX_STRING_LENGTH, "Your attack is blocked by %s's shield!\r\n", target->name );
					echo_to_ship( AT_WHITE, ship, buf );
					snprintf( buf, MAX_STRING_LENGTH, "You quicklyblock %s's attack with your shield!", ship->name );
					echo_to_ship( AT_WHITE, target, buf );
				}
				else if( wtype == WT_MISSILE && target->countertype == CT_SHOOT )
				{
					snprintf( buf, MAX_STRING_LENGTH, "Your missiles are shot out of the air by %s!\r\n", target->name );
					echo_to_ship( AT_WHITE, ship, buf );
					snprintf( buf, MAX_STRING_LENGTH, "You shoot %s's missiles out of the sky!", ship->name );
					echo_to_ship( AT_WHITE, target, buf );
				}
				else if( ( wtype == WT_BEAM && target->countertype == CT_EVADE )
					|| ( wtype == WT_BULLET && target->countertype == CT_EVADE )
					|| ( wtype == WT_MISSILE && target->countertype == CT_EVADE ) )
				{
					snprintf( buf, MAX_STRING_LENGTH, "Your attack is completely evaded by %s!\r\n", target->name );
					echo_to_ship( AT_WHITE, ship, buf );
					snprintf( buf, MAX_STRING_LENGTH, "You evade %s's attack!", ship->name );
					echo_to_ship( AT_WHITE, target, buf );
				}
				else
				{
					damagesuit( ship, target, weapon->weapontype );
				}
			}
			else
			{
				damagesuit( ship, target, weapon->weapontype );
			}
			extract_weapon( weapon );
		}
	}

	for( ship = first_ship; ship; ship = ship->next )
	{
		if( !IS_SET( ship->flags, SUIT_INGRID ) )
			continue;


		if( IS_SET( ship->flags, SUIT_MOVING ) )
		{
			if( ship->weight >= 0 )
			{
				time += ( 0.5 );
			}
			if( ship->weight >= 40 )
			{
				time += ( 0.55 );
			}
			if( ship->weight >= 50 )
			{
				time += ( 0.6 );
			}
			if( ship->weight >= 60 )
			{
				time += ( 0.65 );
			}
			if( ship->weight >= 70 )
			{
				time += ( 0.7 );
			}
			if( ship->weight >= 80 )
			{
				time += ( 0.75 );
			}

			if( IS_SET( ship->flags, SUIT_BOOSTING ) )
			{
				time += 0.5;
			}

			ship->tlength += time;

			if( ship->tlength > 4 )
			{
				ship->tlength = 4;
			}

			if( ship->tlength == 4 )
			{
				if( ship->move[1] > 0 )
				{
					if( can_move( ship, ship->move[1] ) )
					{
						if( ship->move[1] == MOVE_UP )
						{
							ship->gridlvl += 1;
						}
						else if( ship->move[1] == MOVE_DOWN )
						{
							ship->gridlvl -= 1;
						}
						else if( ship->move[1] == MOVE_STAY )
						{
							//         ship->grid = ship->grid;
						}
						else
						{
							ship->grid = ship->move[1];
						}
						ship->move[1] = ship->move[2];
						ship->move[2] = ship->move[3];
						ship->move[3] = ship->move[4];
						ship->move[4] = MOVE_DONE;
						ship->tlength = 4;
						snprintf( buf, MAX_STRING_LENGTH, "Moved To: %d/%d", ship->grid, ship->gridlvl );
						echo_to_ship( AT_WHITE, ship, buf );
						if( ship->move[1] == MOVE_DONE )
						{
							REMOVE_BIT( ship->flags, SUIT_MOVING );
							snprintf( buf, MAX_STRING_LENGTH, "Your movement is finished." );
							echo_to_ship( AT_WHITE, ship, buf );
							ship->move[1] = 0;
							ship->move[2] = 0;
							ship->move[3] = 0;
							ship->move[4] = 0;
						}
					}
					else
					{
						snprintf( buf, MAX_STRING_LENGTH, "WARNING: Can't leave Battle Grid!" );
						echo_to_ship( AT_WHITE, ship, buf );
						REMOVE_BIT( ship->flags, SUIT_MOVING );
						snprintf( buf, MAX_STRING_LENGTH, "Your movement is finished." );
						echo_to_ship( AT_WHITE, ship, buf );
						ship->move[1] = 0;
						ship->move[2] = 0;
						ship->move[3] = 0;
						ship->move[4] = 0;
					}
				}
			}
		}
	}

}

void damagesuit( SHIP_DATA *ship, SHIP_DATA *target, int weapontype )
{
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	int critical;
	float damage;

	if( weapontype == PT_RIGHTARM )
	{
		damage = suitweapon_table[ship->wrightarm].damage;
		snprintf( name, MAX_STRING_LENGTH, "%s", suitweapon_table[ship->wrightarm].weapon_name );
	}
	if( weapontype == PT_LEFTARM )
	{
		damage = suitweapon_table[ship->wleftarm].damage;
		snprintf( name, MAX_STRING_LENGTH, "%s", suitweapon_table[ship->wleftarm].weapon_name );
	}


	damage = number_range( ( damage / 1.1 ), ( damage * 1.1 ) );

	if( target->alloy == 0 )
		damage /= 1.1;
	else if( target->alloy == 1 )
		damage /= 1.2;
	else if( target->alloy == 2 )
		damage /= 1.4;
	else if( target->alloy == 3 )
		damage /= 1.6;
	else if( target->alloy == 4 )
		damage /= 1.7;
	else if( target->alloy == 5 )
		damage /= 2;

	critical = number_range( 1, 20 );

	if( critical == 1 )
	{
		damage *= 2;
	}

	target->frame -= damage;
	snprintf( buf, MAX_STRING_LENGTH, "%sYour %s damages %s! =%.0f=\r\n", critical == 1 ? "[Crt]" : "", name, target->name, damage );
	echo_to_ship( AT_WHITE, ship, buf );
	snprintf( buf1, MAX_STRING_LENGTH, "%s%s damages you with its %s! =%.0f=", critical == 1 ? "[Crt]" : "", ship->name, name, damage );
	echo_to_ship( AT_WHITE, target, buf1 );
	//    target->frame -= atk;
	if( target->frame <= 0 )
	{
		return;
	}
}

void destroy_suit( CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *target )
{
	char buf[MAX_STRING_LENGTH];
	int roomnum;
	ROOM_INDEX_DATA *room;
	OBJ_DATA *robj;
	CHAR_DATA *rch;

	if( !str_cmp( "Holosuit", target->owner ) )
	{
		echo_to_ship( AT_WHITE, ship, "&GThe image of battle flickers out as your fight comes to an end.\r\n" );
		snprintf( buf, MAX_STRING_LENGTH, "The image of %s flickers out of sight.", target->name );
		echo_to_system( AT_WHITE + AT_BLINK, target, buf, NULL );
		resetsuit( target );
		return;
	}

	snprintf( buf, MAX_STRING_LENGTH, "%s's frame collapses from all of the damage!", target->name );
	echo_to_system( AT_WHITE + AT_BLINK, target, buf, NULL );

	echo_to_ship( AT_WHITE + AT_BLINK, target, "Your console blinks bright red..." );
	echo_to_ship( AT_WHITE, target,
		"Your body is ripped apart along with your suit, the energy core exploding from all of the damage!" );

	for( roomnum = target->firstroom; roomnum <= target->lastroom; roomnum++ )
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

	resetsuit( target );

}

void resetsuit( SHIP_DATA *ship )
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

	if( IS_SET( ship->flags, SUIT_INCOMBAT ) )
		REMOVE_BIT( ship->flags, SUIT_INCOMBAT );

	//      if ( IS_SET( ship->flags, SUIT_ISGUARDING ) )
	//        REMOVE_BIT(ship->flags, SUIT_ISGUARDING );

	ship->fstate = FS_OUT;
	free_suitfight( ship );

	ship->frame = 1;
	save_ship( ship );
}

void clearturn( CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *target )
{
	char buf[MAX_STRING_LENGTH];

	if( ship->speed == target->speed )
	{
		ship->speed -= 1;
	}

	if( ship->speed > target->speedmax * 2 )
	{
		echo_to_ship( AT_WHITE, ship, "-=Ex-Turn=- Go again!" );
		snprintf( buf, MAX_STRING_LENGTH, "%s gets an Ex-Turn.", ship->name );
		echo_to_ship( AT_WHITE, target, buf );
		ship->agility += 4;
		if( ship->agility > 6 )
		{
			ship->agility = 6;
		}
		ship->speed -= target->speedmax;
		if( ship->speed <= 0 )
		{
			ship->speed = ship->speedmax;
		}
		return;
	}
	else
	{
		echo_to_ship( AT_WHITE, ship, "&RYour turn is over." );
		echo_to_ship( AT_WHITE, target, "&RIts your turn now!" );
		ship->fstate = FS_WAITCOMMAND;
		target->fstate = FS_COMMAND;
		ship->agility += 4;
		ship->speed = ship->speedmax;
	}

	if( IS_SET( target->flags, SUIT_MOBSUIT ) )
	{
		mobsuitattack( ch, ship, target );
	}
	return;
}

void mobsuitattack( CHAR_DATA *ch, SHIP_DATA *ship, SHIP_DATA *target )
{
	//  if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_PROMPT) )
	//      display_prompt(victim);

	//       attacksuit( ch, target, ship, PT_RIGHTARM );
	//       attacksuit( ch, target, ship, PT_RIGHTARM );
	clearturn( ch, target, ship );
	/*
	   for( atknum = ship->agility; atknum; atknum-- )
	   {
		 randomattack = number_range( 1, 2 );

		 switch( randomattack )
		 {
	//       default: attacksuit( ch, target, ship, PT_RIGHTARM ); break;
	//       case 1:  attacksuit( ch, target, ship, PT_RIGHTARM ); break;
	//       case 2:  attacksuit( ch, target, ship, PT_LEFTARM );break;
		 }
	   }
	*/
}

CMDF( do_attack )
{

	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	SHIP_DATA *target;

	one_argument( argument, arg );
	one_argument( argument, arg2 );

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship!\r\n", ch );
		return;
	}

	/*****************************************************************/

	if( IS_SET( ship->flags, SUIT_INGRID ) )
	{
		if( arg[0] == '\0' || arg2[0] == '\0' )
		{
			send_to_char( "Syntax: Attack <Weapon> <Target?>\r\n", ch );
			send_to_char( "Weapon: Right\r\n", ch );
			return;
		}

		//     if ( !ship->target0 )
		//        target = get_ship( arg2 );
		//     else
		target = ship->target0;

		if( !target )
		{
			send_to_char( "You don't have a target!\r\n", ch );
			return;
		}

		if( ship->fnum != target->fnum )
		{
			send_to_char( "Your target isn't even here!\r\n", ch );
			return;
		}

		if( !str_cmp( arg, "right" ) )
		{
			if( !can_attack( ship, target, suitweapon_table[ship->wrightarm].weapon_type ) )
			{
				send_to_char( "That weapon won't do!\r\n", ch );
				return;
			}

			new_weapon( ship, target, ch, PT_RIGHTARM );
			send_to_char( "You launch your attack!\r\n", ch );
			return;
		}

		return;
	}

	/*****************************************************************/

	if( arg[0] == '\0' )
	{
		send_to_char( "Attack what suit?\r\n", ch );
		return;
	}

	target = ship_in_room( ship->in_room, arg );
	if( target == NULL )
	{
		send_to_char( "&RThat ship isn't here!\r\n", ch );
		return;
	}

	if( target == ship )
	{
		send_to_char( "&RYou can't do that!\r\n", ch );
		return;
	}

	if( !str_cmp( ship->owner, "Public" ) && str_cmp( target->owner, "Public" ) )
	{
		send_to_char( "&RPublic suits are for transport, NOT fighting!!\r\n", ch );
		return;
	}
	if( str_cmp( ship->owner, "Public" ) && !str_cmp( target->owner, "Public" ) )
	{
		send_to_char( "&RYou can't target Public Suits!!\r\n", ch );
		return;
	}
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

	ch_printf( ch, "You engage %s in combat!\r\n", target->name );
	snprintf( buf, MAX_STRING_LENGTH, "%s engages you in combat!\r\n", ch->name );
	echo_to_ship( AT_WHITE, target, buf );

	SET_BIT( ship->flags, SUIT_INGRID );
	ship->grid = 9;
	ship->gridlvl = GRID_ONE;
	ship->fnum = number_range( 1, 32000 );
	ship->position = POS_SOUTH;
	SET_BIT( target->flags, SUIT_INGRID );
	ship->target0 = target;
	target->grid = 28;
	target->gridlvl = GRID_ONE;
	target->fnum = ship->fnum;
	target->position = POS_NORTH;
	target->target0 = ship;
	return;
}

CMDF( do_retreat )
{
	char arg[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;

	one_argument( argument, arg );

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INGRID ) )
	{
		send_to_char( "You're not in combat!\r\n", ch );
		return;
	}

	if( IS_SET( ship->flags, SUIT_MOVING ) )
	{
		send_to_char( "Wait until your current movement is finished before that!\r\n", ch );
		return;
	}

	if( IS_SET( ship->flags, SUIT_ENGAGED ) )
	{
		send_to_char( "You're still engaged!\r\n", ch );
		return;
	}



	ch_printf( ch, "You retreat from battle!\r\n" );
	//    snprintf( buf, MAX_STRING_LENGTH, "%s retreats from combat!\r\n", ch->name );
	//    echo_to_ship( AT_WHITE, target, buf );
	REMOVE_BIT( ship->flags, SUIT_INGRID );
	ship->grid = 0;
	ship->gridlvl = 1;
	ship->fnum = 0;
	ship->target0 = NULL;
	/*
		REMOVE_BIT(target->flags, SUIT_INGRID );
		target->grid = 0;
		target->fnum = 0;
	*/
	return;
}

CMDF( do_move )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	char arg5[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	int gnum, gnum2;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	argument = one_argument( argument, arg5 );

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not yours!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INGRID ) )
	{
		send_to_char( "You're not in combat!\r\n", ch );
		return;
	}

	if( IS_SET( ship->flags, SUIT_MOVING ) )
	{
		send_to_char( "You're still in your current movement!\r\n", ch );
		return;
	}

	if( IS_SET( ship->flags, SUIT_ENGAGED ) )
	{
		send_to_char( "You're still engaged!\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' )
	{
		send_to_char( "Syntax: Move <dir> <dir> <dir> <dir> <Boost?>\r\n", ch );
		send_to_char( "Directions: &BF&borward, &BB&back, &BL&beft, &BR&bight, &BU&bp, &BD&bown, &BS&btay.\r\n", ch );
		return;
	}

	gnum = ship->grid;
	gnum2 = 0;
	ship->move[1] = 0;
	ship->move[2] = 0;
	ship->move[3] = 0;
	ship->move[4] = 0;


	// FIRST MOVE
	if( !str_cmp( arg1, "f" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 0;
				glvl  = MOVE_UP;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
				glvl  = MOVE_DOWN;
			  }
		*/
	}

	if( !str_cmp( arg1, "b" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 16;
			  }
		*/
	}

	if( !str_cmp( arg1, "l" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 6;
		}
		/*G75
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg1, "r" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	gnum2 = gnum;

	if( !str_cmp( arg1, "u" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_UP;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -4;
			  }
		*/
	}

	if( !str_cmp( arg1, "d" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_DOWN;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 4;
			  }
		*/
	}
	if( !str_cmp( arg1, "s" ) )
	{
		send_to_char( "No point in moving if you're staying put.\r\n", ch );
		return;
	}

	//FIRST MOVE

	if( !can_move( ship, gnum ) )
	{
		send_to_char( "That would move you off of the battle grid!\r\n", ch );
		return;
	}

	ship->move[1] = gnum;
	gnum = gnum2;

	//SECOND MOVE
	if( !str_cmp( arg2, "f" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg2, "b" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 16;
			  }
		*/
	}

	if( !str_cmp( arg2, "l" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg2, "r" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	gnum2 = gnum;

	if( !str_cmp( arg2, "u" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_UP;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -4;
			  }
		*/
	}

	if( !str_cmp( arg2, "d" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_DOWN;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 4;
			  }
		*/
	}
	if( !str_cmp( arg2, "s" ) )
	{
		gnum = MOVE_STAY;
	}

	//SECOND MOVE

	if( !can_move( ship, gnum ) )
	{
		send_to_char( "That would move you off of the battle grid!\r\n", ch );
		return;
	}

	ship->move[2] = gnum;
	gnum = gnum2;

	//THIRD MOVE
	if( !str_cmp( arg3, "f" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg3, "b" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 16;
			  }
		*/
	}

	if( !str_cmp( arg3, "l" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg3, "r" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	gnum2 = gnum;

	if( !str_cmp( arg3, "u" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_UP;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -6;
			  }
		*/
	}

	if( !str_cmp( arg3, "d" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_DOWN;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 4;
			  }
		*/
	}
	if( !str_cmp( arg3, "s" ) )
	{
		gnum = MOVE_STAY;
	}

	//THIRD MOVE

	if( !can_move( ship, gnum ) )
	{
		send_to_char( "That would move you off of the battle grid!\r\n", ch );
		return;
	}
	ship->move[3] = gnum;
	gnum = gnum2;

	//FOURTH MOVE
	if( !str_cmp( arg4, "f" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg4, "b" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 6;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -6;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -1;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 1;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 16;
			  }
		*/
	}

	if( !str_cmp( arg4, "l" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += -6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += 6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}

	if( !str_cmp( arg4, "r" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum += 1;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum += -1;
		}
		else if( ship->position == POS_EAST )
		{
			gnum += 6;
		}
		else if( ship->position == POS_WEST )
		{
			gnum += -6;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 16;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -16;
			  }
		*/
	}
	gnum2 = gnum;

	if( !str_cmp( arg4, "u" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_UP;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_UP;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += 4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += -4;
			  }
		*/
	}

	if( !str_cmp( arg4, "d" ) )
	{
		if( ship->position == POS_NORTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_SOUTH )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_EAST )
		{
			gnum = MOVE_DOWN;
		}
		else if( ship->position == POS_WEST )
		{
			gnum = MOVE_DOWN;
		}
		/*
			  else if ( ship->position == POS_UP )
			  {
				gnum += -4;
			  }
			  else if ( ship->position == POS_DOWN )
			  {
				gnum += 4;
			  }
		*/
	}
	if( !str_cmp( arg4, "s" ) )
	{
		gnum = MOVE_STAY;
	}

	//FOURTH MOVE

	if( !can_move( ship, gnum ) )
	{
		send_to_char( "That would move you off of the battle grid!\r\n", ch );
		return;
	}

	ship->move[4] = gnum;

	ship->tlength = 4;

	if( !str_cmp( arg5, "b" ) )
	{
		SET_BIT( ship->flags, SUIT_BOOSTING );
		send_to_char( "You flare up the suits boosters, before beginning your move.\r\n", ch );
	}
	else
	{
		send_to_char( "The suit flies into motion.\r\n", ch );
	}


	SET_BIT( ship->flags, SUIT_MOVING );
	return;
}

CMDF( do_one )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RmYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INCOMBAT ) )
	{
		send_to_char( "You're not in a land fight!\r\n", ch );
		return;
	}

	if( ship->fstate != FS_COMMAND )
	{
		send_to_char( "It's not your turn!\r\n", ch );
		return;
	}

	if( ( target = who_suitfighting( ship ) ) == NULL )
	{
		send_to_char( "Wierd Bug - Report to Cray Immediately.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "do_one: %s has no target.", ch->name );
		bug( buf, 0 );
		return;
	}

	if( IS_SET( ship->flags, SUIT_ISGUARDING ) )
		REMOVE_BIT( ship->flags, SUIT_ISGUARDING );

	ship->agility -= 2;

	//   attacksuit( ch, ship, target, PT_RIGHTARM );

	if( ship->agility <= 0 )
	{
		clearturn( ch, ship, target );
	}
	return;
}

CMDF( do_two )
{
	SHIP_DATA *ship;
	SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INCOMBAT ) )
	{
		send_to_char( "You're not in a land fight!\r\n", ch );
		return;
	}

	if( ship->fstate != FS_COMMAND )
	{
		send_to_char( "It's not your turn!\r\n", ch );
		return;
	}

	if( ( target = who_suitfighting( ship ) ) == NULL )
	{
		send_to_char( "Wierd Bug - Report to Cray Immediately.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "do_two: %s has no target.", ch->name );
		bug( buf, 0 );
		return;
	}

	if( IS_SET( ship->flags, SUIT_ISGUARDING ) )
		REMOVE_BIT( ship->flags, SUIT_ISGUARDING );

	ship->agility -= 2;

	//   attacksuit( ch, ship, target, PT_LEFTARM );

	if( ship->agility <= 0 )
	{
		clearturn( ch, ship, target );
	}
	return;
}

CMDF( do_ltarget )
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

		if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
		{
			send_to_char( "&RYou need to be on land to do that!\r\n", ch );
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

		target = ship_in_room( ship->in_room, arg );
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
		/*
						if ( target->pksuit == 0 )
						{
							send_to_char("&RDork, thats a Non-PK suit!\r\n", ch );
							return;
						}

						if ( ship->pksuit == 0 && target->type != MOB_SHIP )
						{
							send_to_char("&RDork, you're a Non-PK!\r\n", ch );
							return;
						}
		*/

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_weaponsystems] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GTracking target.\r\n", ch );
			act( AT_PLAIN, "$n makes some adjustments on the targeting computer.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 1, do_ltarget, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
		learn_from_failure( ch, gsn_weaponsystems );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, (char *) ch->dest_buf, MSL );
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

	target = ship_in_room( ship->in_room, arg );
	if( ch->in_room->vnum == ship->gunseat )
		ship->target0 = target;

	if( ch->in_room->vnum == ship->turret1 )
		ship->target1 = target;

	if( ch->in_room->vnum == ship->turret2 )
		ship->target2 = target;

	send_to_char( "&GTarget Locked.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "You are being targetted by %s.", ship->name );
	echo_to_cockpit( AT_BLOOD, target, buf );

	sound_to_room( ch->in_room, "!!SOUND(targetlock)" );
	learn_from_success( ch, gsn_weaponsystems );

}

CMDF( do_drive )
{
	int dir;
	SHIP_DATA *ship, *target;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );
	strcpy( arg2, argument );

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the drivers seat of a land vehicle to do that!\r\n", ch );
		return;
	}

	if( ship->shipstate == SHIP_DISABLED )
	{
		send_to_char( "&RThe drive is disabled.\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RDo it in a suit you own!\r\n", ch );
		return;
	}

	if( ship->energy < 1 )
	{
		send_to_char( "&RTheres not enough fuel!\r\n", ch );
		return;
	}

	if( !strcmp( arg, "in" ) )
	{
		target = ship_in_room( ship->in_room, arg2 );
		if( !target )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
			return;
		}

		if( !target->hanger )
		{
			send_to_char( "That ship does not have any room.\r\n", ch );
			return;
		}

		if( ship->lastdoc != ship->location )
		{
			send_to_char( "&rYou don't seem to be docked right now.\r\n", ch );
			return;
		}

		if( ship->ship_class == TRANSPORT_SHIP && target->ship_class == TRANSPORT_SHIP )
		{
			send_to_char( "&RThat ship is not big enough for your suit to drive into!\r\n", ch );
			return;
		}

		if( !target->bayopen )
		{
			send_to_char( "The ship's bay doors must be open.\r\n", ch );
			return;
		}

		if( xIS_SET( target->in_room->room_flags, ROOM_INDOORS ) || target->in_room->sector_type == SECT_INSIDE )
		{
			send_to_char( "You can't drive indoors!\r\n", ch );
			return;
		}

		send_to_char( "You drive the vehicle into the bay.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s drives into %s.", ship->name, target->name );
		echo_to_room( AT_GREY, ship->in_room, buf );
		snprintf( buf, MAX_STRING_LENGTH, "%s drives %s into %s.", ch->name, ship->name, target->name );
		log_string( buf );

		transship( ship, target->hanger );

		snprintf( buf, MAX_STRING_LENGTH, "%s drives into the bay", ship->name );
		echo_to_room( AT_GREY, ship->in_room, buf );
		do_look( ch, "auto" );
		return;
	}

	if( !strcmp( arg, "out" ) )
	{
		target = ship_from_hanger( ship->in_room->vnum );
		if( !target )
		{
			send_to_char( "You have to be in a ship's hanger to drive out of one.\r\n", ch );
			return;
		}

		if( target->lastdoc != target->location )
		{
			send_to_char( "The ship must be landed before you drive out of its hanger!\r\n", ch );
			return;
		}

		if( !target->bayopen )
		{
			send_to_char( "The ship's bay doors must be open.\r\n", ch );
			return;
		}


		if( xIS_SET( target->in_room->room_flags, ROOM_INDOORS ) || target->in_room->sector_type == SECT_INSIDE )
		{
			send_to_char( "You can't drive indoors!\r\n", ch );
			return;
		}

		send_to_char( "You drive the vehicle out of the bay.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "%s drives out of the ship.", ship->name );
		echo_to_room( AT_GREY, ship->in_room, buf );

		transship( ship, target->in_room->vnum );

		snprintf( buf, MAX_STRING_LENGTH, "%s drives out of %s", ship->name, target->name );
		do_look( ch, "auto" );
		echo_to_room( AT_GREY, ship->in_room, buf );
		return;
	}


	if( ( dir = get_door( arg ) ) == -1 )
	{
		send_to_char( "Usage: walk <direction>\r\n", ch );
		return;
	}

	drive_ship( ch, ship, get_exit( get_room_index( ship->location ), dir ), 0 );

}

void transship( SHIP_DATA *ship, int destination )
{
	int origShipyard;


	if( !ship )
		return;

	origShipyard = ship->shipyard;

	ship->shipyard = destination;
	ship->shipstate = SHIP_READY;

	extract_ship( ship );
	ship_to_room( ship, ship->shipyard );

	ship->location = ship->shipyard;
	ship->lastdoc = ship->shipyard;
	ship->shipstate = SHIP_DOCKED;
	ship->shipyard = origShipyard;

	save_ship( ship );
}

ch_ret drive_ship( CHAR_DATA *ch, SHIP_DATA *ship, EXIT_DATA *pexit, int fall )
{
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *original;
	char buf[MAX_STRING_LENGTH];
	const char *txt;
	const char *dtxt;
	ch_ret retcode;
	short door;
	bool drunk = false;
	CHAR_DATA *rch;
	CHAR_DATA *next_rch;


	if( !IS_NPC( ch ) )
		if( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE ) && ( ch->position != POS_DRAG ) )
			drunk = true;

	if( drunk && !fall )
	{
		door = number_door( );
		pexit = get_exit( get_room_index( ship->location ), door );
	}

#ifdef DEBUG
	if( pexit )
	{
		snprintf( buf, MAX_STRING_LENGTH, "drive_ship: %s to door %d", ch->name, pexit->vdir );
		log_string( buf );
	}
#endif

	retcode = rNONE;
	txt = NULL;

	in_room = get_room_index( ship->location );
	if( !pexit || ( to_room = pexit->to_room ) == NULL )
	{
		if( drunk )
			send_to_char( "You drive into a wall in your drunken state.\r\n", ch );
		else
			send_to_char( "Alas, you cannot go that way.\r\n", ch );
		return rNONE;
	}

	door = pexit->vdir;

	if( IS_SET( pexit->exit_info, EX_WINDOW ) && !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	{
		send_to_char( "Alas, you cannot go that way.\r\n", ch );
		return rNONE;
	}

	if( IS_SET( pexit->exit_info, EX_PORTAL ) && IS_NPC( ch ) )
	{
		act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
		return rNONE;
	}

	if( IS_SET( pexit->exit_info, EX_NOMOB ) && IS_NPC( ch ) )
	{
		act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
		return rNONE;
	}

	if( IS_SET( pexit->exit_info, EX_CLOSED ) && ( IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
	{
		if( !IS_SET( pexit->exit_info, EX_SECRET ) && !IS_SET( pexit->exit_info, EX_DIG ) )
		{
			if( drunk )
			{
				act( AT_PLAIN, "$n drives into the $d in $s drunken state.", ch, NULL, pexit->keyword, TO_ROOM );
				act( AT_PLAIN, "You drive into the $d in your drunken state.", ch, NULL, pexit->keyword, TO_CHAR );
			}
			else
				act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		}
		else
		{
			if( drunk )
				send_to_char( "You hit a wall in your drunken state.\r\n", ch );
			else
				send_to_char( "Alas, you cannot go that way.\r\n", ch );
		}

		return rNONE;
	}

	if( room_is_private( ch, to_room ) )
	{
		send_to_char( "That room is private right now.\r\n", ch );
		return rNONE;
	}

	if( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && ch->in_room->area != to_room->area )
	{
		if( ch->top_level < to_room->area->low_hard_range )
		{
			set_char_color( AT_TELL, ch );
			switch( to_room->area->low_hard_range - ch->top_level )
			{
			case 1:
				send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
				break;
			case 2:
				send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
				break;
			case 3:
				send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\r\n", ch );
				break;
			default:
				send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\r\n", ch );
			}
			return rNONE;
		}
		else if( ch->top_level > to_room->area->hi_hard_range )
		{
			set_char_color( AT_TELL, ch );
			send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
			return rNONE;
		}
	}

	if( !fall )
	{
		if( xIS_SET( to_room->room_flags, ROOM_INDOORS )
			|| xIS_SET( to_room->room_flags, ROOM_SPACECRAFT ) || to_room->sector_type == SECT_INSIDE )
		{
			send_to_char( "You can't drive indoors!\r\n", ch );
			return rNONE;
		}

		if( xIS_SET( to_room->room_flags, ROOM_NO_DRIVING ) )
		{
			send_to_char( "You can't take a vehicle through there!\r\n", ch );
			return rNONE;
		}

		if( in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR || IS_SET( pexit->exit_info, EX_FLY ) )
		{
			if( ship->ship_class > CLOUD_CAR )
			{
				send_to_char( "You'd need to fly to go there.\r\n", ch );
				return rNONE;
			}
		}

		if( in_room->sector_type == SECT_WATER_NOSWIM
			|| to_room->sector_type == SECT_WATER_NOSWIM
			|| to_room->sector_type == SECT_WATER_SWIM
			|| to_room->sector_type == SECT_UNDERWATER || to_room->sector_type == SECT_OCEANFLOOR )
		{

			if( ship->ship_class != OCEAN_SHIP )
			{
				send_to_char( "You'd need a boat to go there.\r\n", ch );
				return rNONE;
			}

		}

		if( IS_SET( pexit->exit_info, EX_CLIMB ) )
		{

			if( ship->ship_class < CLOUD_CAR )
			{
				send_to_char( "You need to fly or climb to get up there.\r\n", ch );
				return rNONE;
			}
		}

	}

	if( to_room->tunnel > 0 )
	{
		CHAR_DATA *ctmp;
		int count = 0;

		for( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
			if( ++count >= to_room->tunnel )
			{
				send_to_char( "There is no room for you in there.\r\n", ch );
				return rNONE;
			}
	}

	if( fall )
		txt = "falls";
	else if( !txt )
	{
		if( ship->hover == 0 )
			txt = "walk";
		else
			txt = "fly";
	}
	snprintf( buf, MAX_STRING_LENGTH, "$n %ss the suit $T.", txt );
	act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
	snprintf( buf, MAX_STRING_LENGTH, "You %s the suit $T.", txt );
	act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_CHAR );
	snprintf( buf, MAX_STRING_LENGTH, "%s %ss %s.", ship->name, txt, dir_name[door] );
	echo_to_room( AT_ACTION, get_room_index( ship->location ), buf );

	extract_ship( ship );
	ship_to_room( ship, to_room->vnum );

	ship->location = to_room->vnum;
	ship->lastdoc = ship->location;

	if( fall )
		txt = "falls";
	else if( ship->ship_class < OCEAN_SHIP )
		txt = "flys in";
	else if( ship->ship_class == OCEAN_SHIP )
	{
		txt = "floats in";
	}
	else if( ship->ship_class > OCEAN_SHIP )
	{
		txt = "walks in";
	}

	switch( door )
	{
	default:
		dtxt = "somewhere";
		break;
	case 0:
		dtxt = "the south";
		break;
	case 1:
		dtxt = "the west";
		break;
	case 2:
		dtxt = "the north";
		break;
	case 3:
		dtxt = "the east";
		break;
	case 4:
		dtxt = "below";
		break;
	case 5:
		dtxt = "above";
		break;
	case 6:
		dtxt = "the south-west";
		break;
	case 7:
		dtxt = "the south-east";
		break;
	case 8:
		dtxt = "the north-west";
		break;
	case 9:
		dtxt = "the north-east";
		break;
	}

	snprintf( buf, MAX_STRING_LENGTH, "%s %s from %s.", ship->name, txt, dtxt );
	echo_to_room( AT_ACTION, get_room_index( ship->location ), buf );

	for( rch = ch->in_room->last_person; rch; rch = next_rch )
	{
		next_rch = rch->prev_in_room;
		original = rch->in_room;
		char_from_room( rch );
		char_to_room( rch, to_room );
		do_look( rch, "auto" );
		char_from_room( rch );
		char_to_room( rch, original );
	}

	/*
		if (  CHECK FOR FALLING HERE
		&&   fall > 0 )
		{
		if (!IS_AFFECTED( ch, AFF_FLOATING )
		|| ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
		{
		  set_char_color( AT_HURT, ch );
		  send_to_char( "OUCH! You hit the ground!\r\n", ch );
		  WAIT_STATE( ch, 20 );
		  retcode = damage( ch, ch, 50 * fall, TYPE_UNDEFINED );
		}
		else
		{
		  set_char_color( AT_MAGIC, ch );
		  send_to_char( "You lightly float down to the ground.\r\n", ch );
		}
		}

	*/
	return retcode;

}


CMDF( do_hover )
{
	int chance;
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a mobile suit!\r\n", ch );
		return;
	}

	if( ( ship = ship_from_pilotseat( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou don't seem to be in the pilot seat!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship! Try renting a public one.\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
	{
		send_to_char( "&RYou need to be on land to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class == MOBILE_SUIT )
		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_landcombat] );

	if( number_percent( ) < chance )
	{

		if( ship->hover == 0 )
		{
			send_to_char( "&GThe suits thrusters fire up as you fly up into the air.\r\n", ch );
			act( AT_PLAIN, "$n presses a lever, the suit fires its thrusters and flies into the air.", ch,
				NULL, argument, TO_ROOM );
			learn_from_success( ch, gsn_landcombat );
			snprintf( buf, MAX_STRING_LENGTH, "%s fires up its thrusters, you're pushed back by the power of its quick lift off.", ship->name );
			echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
			ship->hover = 1;
			return;
		}
		else
		{
			send_to_char( "&GThe suit lands back on the ground as you turn your thrusters off.\r\n", ch );
			act( AT_PLAIN, "$n sets the suit back down onto the ground.", ch, NULL, argument, TO_ROOM );
			learn_from_success( ch, gsn_landcombat );
			snprintf( buf, MAX_STRING_LENGTH, "%s lands slowly back on the ground, a loud thud booms through the area.", ship->name );
			echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
			ship->hover = 0;
			return;
		}
	}
	send_to_char( "&RYou fail to work the controls properly.\r\n", ch );
	learn_from_failure( ch, gsn_landcombat );
	return;

}

void remove_part( CHAR_DATA *ch, SHIP_DATA *ship, int part )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *piece;


	if( part == PT_RIGHTARM || PT_LEFTARM )
	{
		if( !IS_SET( ship->flags, SUIT_HASONE ) )
		{
			send_to_char( "You don't have a weapon to remove!\r\n", ch );
			return;
		}

		piece = create_object( get_obj_index( OBJ_SUITPART ), 0 );
		STRFREE( piece->name );
		piece->name = STRALLOC( ship->wonename );
		snprintf( buf, MAX_STRING_LENGTH, "a %s", ship->wonename );
		STRFREE( piece->short_descr );
		piece->short_descr = STRALLOC( buf );
		snprintf( buf, MAX_STRING_LENGTH, "A %s was left here.", ship->wonename );
		STRFREE( piece->description );
		piece->description = STRALLOC( buf );
		piece->item_type = ITEM_SUITWEAPON;
		piece->value[0] = ship->wonedam;
		piece->value[1] = ship->wonetype;
		piece->value[2] = ship->wonelvl;
		obj_to_char( piece, ch );
		ch_printf( ch, "Your suits weapon is removed, and given to you.\r\n" );
		if( part == PT_RIGHTARM )
		{
			REMOVE_BIT( ship->flags, SUIT_HASONE );
		}
		if( part == PT_LEFTARM )
		{
			REMOVE_BIT( ship->flags, SUIT_HASTWO );
		}
		save_ship( ship );
		return;
	}
}

CMDF( do_tune )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	OBJ_DATA *piece;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

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
		send_to_char( "&RTune your own suit!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: Tune <Part> <Piece>\r\n", ch );
		send_to_char( "Parts: Rightarm, Leftarm, Frame\nPiece being what you want to install to the part.\r\n", ch );
		return;
	}

	if( !xIS_SET( ship->in_room->room_flags, ROOM_UPGRADE_CENTER ) )
	{
		send_to_char( "&RBring your suit to a Tuning Center!\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "rightarm" ) )
	{

		if( arg2[0] == '\0' )
		{
			send_to_char( "Item or remove.\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "remove" ) )
		{
			remove_part( ch, ship, PT_RIGHTARM );
			return;
		}

		if( ( piece = find_obj( ch, arg2, true ) ) == NULL )
			return;

		if( piece->item_type != ITEM_SUITWEAPON )
		{
			send_to_char( "That isn't an equippable weapon!\r\n", ch );
			return;
		}

		if( ship->wonelvl < piece->value[2] )
		{
			ch_printf( ch, "You can't add a level %d to your suit.\r\n", piece->value[2] );
			return;
		}
		SET_BIT( ship->flags, SUIT_HASONE );
		ch_printf( ch, "Your %s is placed into %s's right hand.\r\n", piece->name, ship->name );
		STRFREE( ship->wonename );
		ship->wonename = STRALLOC( piece->name );
		ship->wonedam = piece->value[0];
		ship->wonetype = piece->value[1];
		ship->wonelvl = piece->value[2];
		extract_obj( piece );
		save_ship( ship );
	}

	if( !str_cmp( arg, "leftarm" ) )
	{

		if( arg2[0] == '\0' )
		{
			send_to_char( "Item or remove.\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "remove" ) )
		{
			remove_part( ch, ship, PT_LEFTARM );
			return;
		}

		if( ( piece = find_obj( ch, arg2, true ) ) == NULL )
			return;

		if( piece->item_type != ITEM_SUITWEAPON )
		{
			send_to_char( "That isn't an equippable weapon!\r\n", ch );
			return;
		}

		if( ship->wtwolvl < piece->value[2] )
		{
			ch_printf( ch, "You can't add a level %d to your suit.\r\n", piece->value[2] );
			return;
		}
		SET_BIT( ship->flags, SUIT_HASTWO );
		ch_printf( ch, "Your %s is placed into %s's left hand.\r\n", piece->name, ship->name );
		STRFREE( ship->wtwoname );
		ship->wtwoname = STRALLOC( piece->name );
		ship->wtwodam = piece->value[0];
		ship->wtwotype = piece->value[1];
		ship->wtwolvl = piece->value[2];
		extract_obj( piece );
		save_ship( ship );
	}

}

CMDF( do_guard )
{
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	SHIP_DATA *target;


	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INCOMBAT ) )
	{
		send_to_char( "You're not in a land fight!\r\n", ch );
		return;
	}

	if( ship->fstate != FS_COMMAND )
	{
		send_to_char( "It's not your turn!\r\n", ch );
		return;
	}

	if( ( target = who_suitfighting( ship ) ) == NULL )
	{
		send_to_char( "Wierd Bug - Report to Cray Immediately.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "do_guard: %s has no target.", ch->name );
		bug( buf, 0 );
		return;
	}

	if( ship->agility < 4 )
	{
		send_to_char( "You can't guard now, finish attacking!\r\n", ch );
		return;
	}

	send_to_char( "You go into guard mode.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "%s takes a guard position.\r\n", ch->name );
	ship->agility -= 4;
	echo_to_ship( AT_WHITE, target, buf );
	SET_BIT( ship->flags, SUIT_ISGUARDING );
	clearturn( ch, ship, target );
}

CMDF( do_makesweapon )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance;
	OBJ_DATA *piece;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum = 0, wtype = 0;

	mudstrlcpy( arg, argument, MSL );

	if( !IS_SUPREME( ch ) )
	{
		send_to_char( "Sorry, you can't use this yet =)\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:

		if( arg[0] == '\0' )
		{
			send_to_char( "&RUsage: Makesweapon <name>\r\n&w", ch );
			return;
		}

		if( !xIS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
		{
			send_to_char( "&RYou need to be in a factory or workshop to do that.\r\n", ch );
			return;
		}

		chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makesweapon] );
		if( number_percent( ) < chance )
		{
			send_to_char( "&GYou begin the task of making your suit weapon.\r\n", ch );
			act( AT_PLAIN, "$n uses the machinery to make something.", ch, NULL, argument, TO_ROOM );
			add_timer( ch, TIMER_DO_FUN, 10, do_makesweapon, 1 );
			ch->dest_buf = str_dup( arg );
			return;
		}
		send_to_char( "&RYou can't figure out how to fit the parts together.\r\n", ch );
		learn_from_failure( ch, gsn_makesweapon );
		return;

	case 1:
		if( !ch->dest_buf )
			return;
		mudstrlcpy( arg, ( char * ) ch->dest_buf, MSL );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "&RYou are interupted and fail to finish your work.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
	{
		send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\r\n", ch );
		return;
	}

	chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_makesweapon] );

	if( number_percent( ) > chance * 2 )
	{
		send_to_char( "&RYou fail miserably at making your suit weapon....\r\n", ch );
		learn_from_failure( ch, gsn_makesweapon );
		return;
	}

	piece = create_object( get_obj_index( OBJ_SUITPART ), 0 );

	piece->item_type = ITEM_SUITWEAPON;
	SET_BIT( piece->wear_flags, ITEM_HOLD );
	SET_BIT( piece->wear_flags, ITEM_TAKE );
	piece->weight = 3;
	STRFREE( piece->name );
	strcpy( buf, arg );
	piece->name = STRALLOC( buf );
	strcpy( buf, arg );
	STRFREE( piece->short_descr );
	piece->short_descr = STRALLOC( buf );
	STRFREE( piece->description );
	strcat( buf, " was left here." );
	piece->description = STRALLOC( buf );
	piece->cost = 10;
	piece->value[0] = 15;
	piece->value[1] = wtype;
	piece->value[2] = 1;
	piece = obj_to_char( piece, ch );

	send_to_char( "&GYou finish your suit weapon&w\r\n", ch );
	act( AT_PLAIN, "$n finishes making their suit weapon.", ch, NULL, argument, TO_ROOM );

	{
		long xpgain;

		xpgain =
			UMIN( piece->cost * 100,
				( exp_level( ch->skill_level[ENGINEERING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[ENGINEERING_ABILITY] ) ) );
		gain_exp( ch, xpgain, ENGINEERING_ABILITY );
		ch_printf( ch, "You gain %d engineering experience.", xpgain );
	}
	learn_from_success( ch, gsn_makesweapon );

}

void landstat( CHAR_DATA *ch, SHIP_DATA *ship )
{


	send_to_char( "\r\n", ch );
	ch_printf( ch, "&z____                                      ____\r\n" );
	ch_printf( ch, "&z|  |                                      |  |\r\n" );
	ch_printf( ch, "&z|  |        &w____  ____  ____  ____        &z|  |\r\n" );
	ch_printf( ch, "&z|  |&W\\_____&z_" );
	ch_printf( ch, "&w/%s", gridcheck( ship, 8, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 9, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 10, GRID_ONE ) );
	ch_printf( ch, "%s&w\\", gridcheck( ship, 11, GRID_ONE ) );
	ch_printf( ch, "&w/%s", gridcheck( ship, 8, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 9, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 10, GRID_TWO ) );
	ch_printf( ch, "%s&w\\", gridcheck( ship, 11, GRID_TWO ) );
	ch_printf( ch, "&w/%s", gridcheck( ship, 8, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 9, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 10, GRID_THREE ) );
	ch_printf( ch, "%s&w\\", gridcheck( ship, 11, GRID_THREE ) );
	ch_printf( ch, "&w/%s", gridcheck( ship, 8, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 9, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 10, GRID_FOUR ) );
	ch_printf( ch, "%s&w\\", gridcheck( ship, 11, GRID_FOUR ) );
	ch_printf( ch, "&z_&W_____/&z|  &z|\r\n" );

	ch_printf( ch, "&z|  &z| &W|&BOUT&W| " );
	ch_printf( ch, "&w|%s", gridcheck( ship, 14, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 15, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 16, GRID_ONE ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 17, GRID_ONE ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 14, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 15, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 16, GRID_TWO ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 17, GRID_TWO ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 14, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 15, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 16, GRID_THREE ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 17, GRID_THREE ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 14, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 15, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 16, GRID_FOUR ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 17, GRID_FOUR ) );
	ch_printf( ch, " &W|&BPOW&W| &z|  |\r\n" );

	ch_printf( ch, "&z|  | &W|===&W| " );
	ch_printf( ch, "&w|%s", gridcheck( ship, 20, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 21, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 22, GRID_ONE ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 23, GRID_ONE ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 20, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 21, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 22, GRID_TWO ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 23, GRID_TWO ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 20, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 21, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 22, GRID_THREE ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 23, GRID_THREE ) );
	ch_printf( ch, "&w|%s", gridcheck( ship, 20, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 21, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 22, GRID_FOUR ) );
	ch_printf( ch, "%s&w|", gridcheck( ship, 23, GRID_FOUR ) );
	ch_printf( ch, " &W|===&W| &z|  &z|\r\n" );

	ch_printf( ch, "&z|&W--&z| &W|&c67&C%&W| " );
	ch_printf( ch, "&w\\%s", gridcheck( ship, 26, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 27, GRID_ONE ) );
	ch_printf( ch, "%s", gridcheck( ship, 28, GRID_ONE ) );
	ch_printf( ch, "%s&w/", gridcheck( ship, 29, GRID_ONE ) );
	ch_printf( ch, "&w\\%s", gridcheck( ship, 26, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 27, GRID_TWO ) );
	ch_printf( ch, "%s", gridcheck( ship, 28, GRID_TWO ) );
	ch_printf( ch, "%s&w/", gridcheck( ship, 29, GRID_TWO ) );
	ch_printf( ch, "&w\\%s", gridcheck( ship, 26, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 27, GRID_THREE ) );
	ch_printf( ch, "%s", gridcheck( ship, 28, GRID_THREE ) );
	ch_printf( ch, "%s&w/", gridcheck( ship, 29, GRID_THREE ) );
	ch_printf( ch, "&w\\%s", gridcheck( ship, 26, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 27, GRID_FOUR ) );
	ch_printf( ch, "%s", gridcheck( ship, 28, GRID_FOUR ) );
	ch_printf( ch, "%s&w/", gridcheck( ship, 29, GRID_FOUR ) );
	ch_printf( ch, " &W|&c90&C%&W| &z|&W--&z|\r\n" );

	ch_printf( ch, "&z|  | &W>---<&z--&w----&z--&w----&z--&w----&z--&w----&z--&W>---< &z|  &z|\r\n" );
	ch_printf( ch, "&z|&W--&z|&w/                                    \\&z|&W--&z|\r\n" );
	ch_printf( ch, "&z|&OEN&W|                                      &z|&OSP&z|\r\n" );
	ch_printf( ch, "&z====                                      ====\r\n" );
	ch_printf( ch, "\r\n\n\rGNum: %d  GLvl: %d  FNum: %d  Tlength: %d\r\n", ship->grid, ship->gridlvl, ship->fnum,
		ship->tlength );
	ch_printf( ch, "1: %d  2: %d  3: %d  4: %d\r\n", ship->move[1], ship->move[2], ship->move[3], ship->move[4] );
	return;
}

bool can_engage( SHIP_DATA *ship, SHIP_DATA *target, int attacktype )
{
	if( ship->gridlvl != target->gridlvl )
	{
		return false;
	}

	if( ( attacktype == WT_SLASH ) || ( attacktype == WT_PIERCE ) )
	{
		if( ship->grid - target->grid == -6 )
			return true;
		else if( ship->grid - target->grid == 6 )
			return true;
		else if( ship->grid - target->grid == -1 )
			return true;
		else if( ship->grid - target->grid == 1 )
			return true;
		else
			return false;
	}
	return false;
}

bool can_attack( SHIP_DATA *ship, SHIP_DATA *target, int attacktype )
{
	if( attacktype == WT_SLASH )
	{
		if( ship->gridlvl != target->gridlvl )
		{
			return false;
		}

		if( ship->grid - target->grid == -6 )
			return true;
		else if( ship->grid - target->grid == 6 )
			return true;
		else if( ship->grid - target->grid == -1 )
			return true;
		else if( ship->grid - target->grid == 1 )
			return true;
		else
			return false;
	}
	else
	{
		return true;
	}
	return true;
}

bool can_move( SHIP_DATA *ship, int gnum )
{
	if( ( ( gnum == MOVE_UP ) && ( ship->gridlvl == 4 ) ) )
	{
		return false;
	}

	if( ( gnum == MOVE_DOWN ) && ( ship->gridlvl == 1 ) )
	{
		return false;
	}

	if( ( gnum == 2 ) || ( gnum == 3 ) || ( gnum == 4 ) || ( gnum == 5 )
		|| ( gnum == 6 ) || ( gnum == 7 ) || ( gnum == 12 ) || ( gnum == 13 )
		|| ( gnum == 18 ) || ( gnum == 19 ) || ( gnum == 24 ) || ( gnum == 25 )
		|| ( gnum == 30 ) || ( gnum == 32 ) || ( gnum == 33 ) || ( gnum == 34 ) || ( gnum == 35 ) )
	{
		return false;
	}
	if( gnum >= 36 && gnum <= 100 )
	{
		return false;
	}
	return true;
}

char *gridcheck( SHIP_DATA *ship, int gridnumber, int gridlevel )
{
	static char buf[MAX_STRING_LENGTH];
	SHIP_DATA *target;

	snprintf( buf, MAX_STRING_LENGTH, " " );

	if( ship->grid == gridnumber && ship->gridlvl == gridlevel )
	{
		if( ship->position == POS_NORTH )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B^^&W" );
		}
		else if( ship->position == POS_SOUTH )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&Bv&W" );
		}
		else if( ship->position == POS_WEST )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B<&W" );
		}
		else if( ship->position == POS_EAST )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B>&W" );
		}
	}

	for( target = first_ship; target; target = target->next )
	{
		if( !IS_SET( target->flags, SUIT_INGRID ) )
			continue;

		if( target->fnum != ship->fnum )
			continue;

		if( target->gridlvl != gridlevel )
			continue;

		if( target == ship )
			continue;

		if( target->grid != gridnumber )
			continue;

		if( target->position == POS_NORTH )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&R^^&W" );
		}
		else if( target->position == POS_SOUTH )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&Rv&W" );
		}
		else if( target->position == POS_WEST )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&R<&W" );
		}
		else if( target->position == POS_EAST )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&R>&W" );
		}

	}

	return buf;
}


CMDF( do_convert )
{
	SHIP_DATA *ship;


	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit or turret of a suit to do that!\r\n", ch );
		return;
	}

	if( ship->ship_class > SHIP_PLATFORM )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( ship->shipstate != SHIP_DOCKED )
	{
		send_to_char( "&RBe on the ground for that!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Which aspect would you like to convert to?\r\n", ch );
		send_to_char( "[Frame, Strength, PArmor, EArmor, Speed, Dodge, Critical]\r\n", ch );
		return;
	}
}

CMDF( do_store )
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	SHIP_DATA *target;
	int amount = 0;

	argument = one_argument( argument, arg1 );

	if( IS_NPC( ch ) )
		return;

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INCOMBAT ) )
	{
		send_to_char( "You're not in a land fight!\r\n", ch );
		return;
	}

	if( ship->fstate != FS_COMMAND )
	{
		send_to_char( "It's not your turn!\r\n", ch );
		return;
	}

	if( ( target = who_suitfighting( ship ) ) == NULL )
	{
		send_to_char( "Wierd Bug - Report to Axis Immediately.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "do_store: %s has no target.", ch->name );
		bug( buf, 0 );
		return;
	}

	if( ship->agility < 4 )
	{
		send_to_char( "You can't store now, finish attacking!\r\n", ch );
		return;
	}

	if( arg1[0] != '\0' )
		amount = atoi( arg1 );

	if( !is_number( arg1 ) )
	{
		send_to_char( "How much reserve energy do you want to use?\r\n", ch );
		return;
	}

	if( amount < 500 )
	{
		send_to_char( "You can only pull 500 from your reserves at a time.\r\n", ch );
		return;
	}

	if( amount < ship->reserve )
	{
		send_to_char( "You don't have that much in your reserves!\r\n", ch );
		return;
	}

	send_to_char( "You pull energy in from your reserves.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "%s's arms lower, as it pulls in reserve energy.\r\n", ship->name );
	ship->agility -= 4;
	ship->reserve -= amount;
	ship->extra += amount;
	echo_to_ship( AT_WHITE, target, buf );
	clearturn( ch, ship, target );
}

CMDF( do_recharge )
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	SHIP_DATA *target;
	int amount = 0;

	argument = one_argument( argument, arg1 );

	if( IS_NPC( ch ) )
		return;

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INCOMBAT ) )
	{
		send_to_char( "You're not in a land fight!\r\n", ch );
		return;
	}

	if( ship->fstate != FS_COMMAND )
	{
		send_to_char( "It's not your turn!\r\n", ch );
		return;
	}

	if( ( target = who_suitfighting( ship ) ) == NULL )
	{
		send_to_char( "Wierd Bug - Report to Axis Immediately.\r\n", ch );
		snprintf( buf, MAX_STRING_LENGTH, "do_store: %s has no target.", ch->name );
		bug( buf, 0 );
		return;
	}

	if( ship->agility < 4 )
	{
		send_to_char( "You can't recharge now, finish attacking!\r\n", ch );
		return;
	}

	if( ship->reserve < ( ship->reservemax / 4 ) )
	{
		send_to_char( "You don't have enough reserve energy to recharge!\r\n", ch );
		return;
	}

	send_to_char( "You recharge your suit, repairing frame damage.\r\n", ch );
	snprintf( buf, MAX_STRING_LENGTH, "%s's recharges, repairing frame damage.\r\n", ship->name );
	ship->agility -= 4;
	ship->reserve -= amount;
	ship->frame += ( ship->framemax * 0.3 );
	if( ship->frame > ship->framemax )
	{
		ship->frame = ship->framemax;
	}
	echo_to_ship( AT_WHITE, target, buf );
	clearturn( ch, ship, target );
}


CMDF( do_input )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	WEAPON_DATA *weapon;
	WEAPON_DATA *w_next;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in the cockpit of a suit to do that!\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: Input <Command>\r\n", ch );
		send_to_char( "Command: Power (On/Off), Specs, Armaments.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "specs" ) )
	{
		ch_printf( ch, "Classification: %s\r\n", suit_class[ship->stype] );
		ch_printf( ch, "Armor: %s\r\n", armor_name( ship->flags ) );
		ch_printf( ch, "Height: 17.45 meters.\n\rWeight: %d tons.\r\n", ship->weight );
		return;
	}

	if( !str_cmp( arg1, "phaseshift" ) )
	{
		if( !IS_SET( ship->flags, SUIT_AR_PHASESHIFT ) )
		{
			send_to_char( "Your suit doesn't even have Phase Shift capabilities!\r\n", ch );
			return;
		}
		if( arg2[0] == '\0' )
		{
			send_to_char( "Turn Phase Shift on or off?\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "on" ) )
		{
			if( IS_SET( ship->flags, SUIT_PHASESHIFTON ) )
			{
				send_to_char( "You suit already has Phase Shift Activated!\r\n", ch );
				return;
			}
			act( AT_PLAIN, "$n hits a button, activating Phase Shift.", ch, NULL, argument, TO_ROOM );
			SET_BIT( ship->flags, SUIT_PHASESHIFTON );
			send_to_char( "You press a button, activating Phase Shift.\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "off" ) )
		{
			if( !IS_SET( ship->flags, SUIT_PHASESHIFTON ) )
			{
				send_to_char( "You suit doesn't have Phase Shift Activated!\r\n", ch );
				return;
			}
			act( AT_PLAIN, "$n hits a button, deactivating Phase Shift.", ch, NULL, argument, TO_ROOM );
			REMOVE_BIT( ship->flags, SUIT_PHASESHIFTON );
			send_to_char( "You press a button, deactivating Phase Shift.\r\n", ch );
			return;
		}

	}

	if( !str_cmp( arg1, "armaments" ) )
	{
		ch_printf( ch, "\n&YRight Arm&O: &w%s&z(&W%s&z)\r\n", suitweapon_table[ship->wrightarm].weapon_name,
			suitweapon_type[suitweapon_table[ship->wrightarm].weapon_type] );
		ch_printf( ch, "&YLeft Arm&O:  &w%s&z(&W%s&z)\r\n", suitweapon_table[ship->wleftarm].weapon_name,
			suitweapon_type[suitweapon_table[ship->wleftarm].weapon_type] );
		return;
	}
	if( !str_cmp( arg1, "stat" ) )
	{
		for( weapon = first_weapon; weapon; weapon = w_next )
		{
			w_next = weapon->next;

			ch_printf( ch, "Weapon:   From: %s  To: %s  Time: %d\r\n",
				weapon->fired_from->name, weapon->target->name, weapon->time );
		}
	}
	do_input( ch, "" );
	return;
}

CMDF( do_counter )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	int ct, amount;

	amount = 0;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INGRID ) )
	{
		send_to_char( "You're not in combat!\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: Counter <Type> <Number>\r\n", ch );
		send_to_char( "Type: Evade, Block, Shoot\r\n", ch );
		return;
	}

	if( arg2[0] != '\0' )
		amount = atoi( arg2 );

	if( !str_cmp( arg1, "evade" ) )
	{
		send_to_char( "You attempt to evade the attack.\r\n", ch );
		ct = CT_EVADE;
	}
	else if( !str_cmp( arg1, "block" ) )
	{
		send_to_char( "You attempt to block the attack.\r\n", ch );
		ct = CT_BLOCK;
	}
	else if( !str_cmp( arg1, "shoot" ) )
	{
		send_to_char( "You attempt to shoot down the attack.\r\n", ch );
		ct = CT_SHOOT;
	}
	ship->counter = amount;
	ship->countertype = ct;
	return;
}

CMDF( do_engage )
{
	char arg1[MAX_INPUT_LENGTH];
	SHIP_DATA *ship;
	SHIP_DATA *target;

	argument = one_argument( argument, arg1 );

	if( ( ship = ship_from_turret( ch->in_room->vnum ) ) == NULL )
	{
		send_to_char( "&RYou must be in a cockpit!\r\n", ch );
		return;
	}

	if( ship->ship_class != MOBILE_SUIT )
	{
		send_to_char( "&RThis isn't a spacecraft!\r\n", ch );
		return;
	}

	if( !check_pilot( ch, ship ) )
	{
		send_to_char( "&RHey, thats not your ship!\r\n", ch );
		return;
	}

	if( !IS_SET( ship->flags, SUIT_INGRID ) )
	{
		send_to_char( "You're not in combat!\r\n", ch );
		return;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: Engage <target>\r\n", ch );
		return;
	}

	target = get_ship( arg1 );

	if( ( !target ) || ( ship->fnum != target->fnum ) )
	{
		send_to_char( "That target isn't even here!\r\n", ch );
		return;
	}

	if( can_engage( ship, target, suitweapon_table[ship->wrightarm].weapon_type ) )
	{
		ch_printf( ch, "You engage %s!\r\n", target->name );
		//       ship->enum = number_range( 1, 3000 );
		ship->target1 = target;
		SET_BIT( ship->flags, SUIT_ENGAGED );
		//       target->enum = ship->enum;
		target->target1 = ship;
		SET_BIT( target->flags, SUIT_ENGAGED );
	}
	if( !can_engage( ship, target, suitweapon_table[ship->wleftarm].weapon_type ) )
	{
		ch_printf( ch, "You engage %s!\r\n", target->name );
		//       ship->enum = number_range( 1, 3000 );
		ship->target1 = target;
		SET_BIT( ship->flags, SUIT_ENGAGED );
		//       target->enum = ship->enum;
		target->target1 = ship;
		SET_BIT( target->flags, SUIT_ENGAGED );
	}
	send_to_char( "You don't have a suitable weapon for engaging!\r\n", ch );
	return;
}
