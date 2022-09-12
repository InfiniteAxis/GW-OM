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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


#define QUEST_ITEM1 80
#define QUEST_ITEM2 81
#define QUEST_ITEM3 82
#define QUEST_ITEM4 83
#define QUEST_ITEM5 84


#define QUEST_VALUE1 1800
#define QUEST_VALUE2 2000
#define QUEST_VALUE3 1000
#define QUEST_VALUE4 2500
#define QUEST_VALUE5 5000

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 85
#define QUEST_OBJQUEST2 86
#define QUEST_OBJQUEST3 87
#define QUEST_OBJQUEST4 88
#define QUEST_OBJQUEST5 89

   /* Local functions */

void generate_quest( CHAR_DATA *ch, CHAR_DATA *questman );
void save_sysdata( SYSTEM_DATA sys );
bool qchance( int num );

bool qchance( int num )
{
	if( number_range( 1, 100 ) <= num )
		return true;
	else
		return false;
}

extern int top_exit;

CMDF( do_buyhome )
{
	ROOM_INDEX_DATA *room;
	AREA_DATA *pArea;

	if( !ch->in_room )
		return;

	if( IS_NPC( ch ) || !ch->pcdata )
		return;
	/*
		 if ( ch->plr_home != NULL )
		 {
			 send_to_char( "&RYou already have a home!\r\n&w", ch);
			 return;
		 }
	*/
	room = ch->in_room;

	for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
	{
		if( room->area == pArea )
		{
			send_to_char( "&RThis area isn't installed yet!\r\n&w", ch );
			return;
		}
	}

	if( !xIS_SET( room->room_flags, ROOM_EMPTY_HOME ) )
	{
		send_to_char( "&RThis room isn't for sale!\r\n&w", ch );
		return;
	}

	if( ch->gold < 75000 )
	{
		send_to_char( "&RThis room costs 75000 dollars, you don't have enough!\r\n&w", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Set the room name.  A very brief single line room description.\r\n", ch );
		send_to_char( "Usage: Buyhome <Room Name>\r\n", ch );
		return;
	}

	STRFREE( room->name );
	room->name = STRALLOC( argument );
	STRFREE( room->owner );
	room->owner = STRALLOC( ch->name );

	ch->gold -= 75000;

	xREMOVE_BIT( room->room_flags, ROOM_EMPTY_HOME );
	xSET_BIT( room->room_flags, ROOM_PLR_HOME );

	fold_area( room->area, room->area->filename, false );
	ch->plr_home = room;
	do_save( ch, "" );
}

CMDF( do_sellhome )
{
	/*
	 * Added by Ulysses, Dec '99/Jan '00
	 */

	 /*
	  * changed it so you can change the variable faster and easier. Darrik Vequir
	  */

	int sellHomeCreditReturn = 55000;

	ROOM_INDEX_DATA *room;

	if( ch->plr_home == NULL )
	{
		send_to_char( "&RYou don't own a home.\r\n", ch );
		return;
	}

	if( ( room = ch->in_room ) != ch->plr_home )
	{
		send_to_char( "&RYou need to be inside your home to sell it.\r\n", ch );
		return;
	}

	if( room != ch->plr_home )
	{
		send_to_char( "&RYou are not the owner of this home.\r\n", ch );
		return;
	}

	STRFREE( room->name );
	room->name = STRALLOC( "An Empty Apartment" );
	ch->gold += sellHomeCreditReturn;
	xREMOVE_BIT( room->room_flags, ROOM_PLR_HOME );
	xSET_BIT( room->room_flags, ROOM_EMPTY_HOME );
	STRFREE( room->guests );
	fold_area( room->area, room->area->filename, false );
	ch->plr_home = NULL;
	do_save( ch, "" );
	ch_printf( ch, "&WYou sell your home for %d dollars.\r\n", sellHomeCreditReturn );
}

CMDF( do_clone )
{
	long credits, bank;
	long played;
	char clanname[MAX_STRING_LENGTH];
	char bestowments[MAX_STRING_LENGTH];
	EXT_BV flags;
	ROOM_INDEX_DATA *home;

	xCLEAR_BITS( flags );

	if( IS_NPC( ch ) )
	{
		ch_printf( ch, "Yeah right!\r\n" );
		return;
	}

	if( ch->in_room->vnum != 2098 )
	{
		ch_printf( ch, "You can't do that here!\r\n" );
		return;
	}

	if( ch->gold < ch->top_level * 20 )
	{
		ch_printf( ch, "You don't have enough dollars... You need %d.\r\n", ch->top_level * 200 );
		return;
	}
	else
	{
		ch->gold -= ch->top_level * 20;

		ch_printf( ch, "You pay %d dollars for cloning.\r\n", ch->top_level * 20 );
		ch_printf( ch, "You are escorted into a small room.\r\n\r\n" );
	}

	char_from_room( ch );
	char_to_room( ch, get_room_index( 2099 ) );

	flags = ch->act;
	xREMOVE_BIT( ch->act, PLR_KILLER );
	credits = ch->gold;
	ch->gold = 0;
	played = ch->played;
	ch->played = ch->played / 2;
	bank = ch->pcdata->bank;
	ch->pcdata->bank = 0;
	home = ch->plr_home;
	ch->plr_home = NULL;
	if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
	{
		strcpy( clanname, ch->pcdata->clan_name );
		STRFREE( ch->pcdata->clan_name );
		ch->pcdata->clan_name = STRALLOC( "" );
		strcpy( bestowments, ch->pcdata->bestowments );
		DISPOSE( ch->pcdata->bestowments );
		ch->pcdata->bestowments = str_dup( "" );
		save_clone( ch );
		STRFREE( ch->pcdata->clan_name );
		ch->pcdata->clan_name = STRALLOC( clanname );
		DISPOSE( ch->pcdata->bestowments );
		ch->pcdata->bestowments = str_dup( clanname );
	}
	else
		save_clone( ch );
	ch->plr_home = home;
	ch->played = played;
	ch->gold = credits;
	ch->pcdata->bank = bank;
	ch->act = flags;
	char_from_room( ch );
	char_to_room( ch, get_room_index( 2099 ) );
	do_look( ch, "" );

	ch_printf( ch, "\r\n&WA small tissue sample is taken from your arm.\r\n" );
	ch_printf( ch, "&ROuch!\r\n\r\n" );
	ch_printf( ch, "&WYou have been succesfully cloned.\r\n" );

	ch->hit--;
}

CMDF( do_arm )
{
	OBJ_DATA *obj;

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		ch_printf( ch, "You have no idea how to do that.\r\n" );
		return;
	}

	if( ch->pcdata->learned[gsn_grenades] <= 0 )
	{
		ch_printf( ch, "You have no idea how to do that.\r\n" );
		return;
	}

	obj = get_eq_char( ch, WEAR_HOLD );

	if( !obj || obj->item_type != ITEM_GRENADE )
	{
		ch_printf( ch, "You don't seem to be holding a grenade!\r\n" );
		return;
	}

	obj->timer = 1;

	if( obj->armed_by )
	{
		STRFREE( obj->armed_by );
	}
	obj->armed_by = STRALLOC( ch->name );

	ch_printf( ch, "You arm %s.\r\n", obj->short_descr );
	act( AT_PLAIN, "$n arms $p.", ch, obj, NULL, TO_ROOM );

	learn_from_success( ch, gsn_grenades );
}

CMDF( do_ammo )
{
	OBJ_DATA *wield;
	OBJ_DATA *obj;
	bool checkammo = false;
	int charge = 0;

	obj = NULL;
	wield = get_eq_char( ch, WEAR_WIELD );
	if( wield )
	{
		obj = get_eq_char( ch, WEAR_DUAL_WIELD );
		if( !obj )
			obj = get_eq_char( ch, WEAR_HOLD );
	}
	else
	{
		wield = get_eq_char( ch, WEAR_HOLD );
		obj = NULL;
	}

	if( !wield || wield->item_type != ITEM_WEAPON )
	{
		send_to_char( "&RYou don't seem to be holding a weapon.\r\n&w", ch );
		return;
	}

	if( wield->value[3] == WEAPON_BLASTER )
	{

		if( obj && obj->item_type != ITEM_AMMO )
		{
			send_to_char( "&RYour hands are too full to reload your gun.\r\n&w", ch );
			return;
		}

		if( obj )
		{
			if( obj->value[0] > wield->value[5] )
			{
				send_to_char( "That cartridge is too big for your blaster.", ch );
				return;
			}
			unequip_char( ch, obj );
			checkammo = true;
			charge = obj->value[0];
			separate_obj( obj );
			extract_obj( obj );
		}
		else
		{
			for( obj = ch->last_carrying; obj; obj = obj->prev_content )
			{
				if( obj->item_type == ITEM_AMMO )
				{
					if( obj->value[0] > wield->value[5] )
					{
						send_to_char( "That cartridge is too big for your gun.", ch );
						continue;
					}
					checkammo = true;
					charge = obj->value[0];
					separate_obj( obj );
					extract_obj( obj );
					break;
				}
			}
		}

		if( !checkammo )
		{
			send_to_char( "&RYou don't seem to have any ammo to reload your gun with.\r\n&w", ch );
			return;
		}

		ch_printf( ch,
			"You replace your ammunition cartridge.\n\rYour gun is charged with %d shots at high power to %d shots on low.\r\n",
			charge / 5, charge );
		act( AT_PLAIN, "$n replaces the ammunition cell in $p.", ch, wield, NULL, TO_ROOM );

	}
	else if( wield->value[3] == WEAPON_BOWCASTER )
	{

		if( obj && obj->item_type != ITEM_BOLT )
		{
			send_to_char( "&RYour hands are too full to reload your bow.\r\n&w", ch );
			return;
		}

		if( obj )
		{
			if( obj->value[0] > wield->value[5] )
			{
				send_to_char( "That cartridge is too big for your bow.", ch );
				return;
			}
			unequip_char( ch, obj );
			checkammo = true;
			charge = obj->value[0];
			separate_obj( obj );
			extract_obj( obj );
		}
		else
		{
			for( obj = ch->last_carrying; obj; obj = obj->prev_content )
			{
				if( obj->item_type == ITEM_BOLT )
				{
					if( obj->value[0] > wield->value[5] )
					{
						send_to_char( "That cartridge is too big for your bow.", ch );
						continue;
					}
					checkammo = true;
					charge = obj->value[0];
					separate_obj( obj );
					extract_obj( obj );
					break;
				}
			}
		}

		if( !checkammo )
		{
			send_to_char( "&RYou don't seem to have any quarrels to reload your bow.\r\n&w", ch );
			return;
		}

		ch_printf( ch, "You replace your quarrel pack.\n\rYour bow is charged with %d arrows.\r\n", charge );
		act( AT_PLAIN, "$n replaces the quarrels in $p.", ch, wield, NULL, TO_ROOM );

	}
	else
	{
		send_to_char( "You don't seem to have anything you'd bother to load...\r\n", ch );
		return;
	}
	wield->value[4] = charge;

}

CMDF( do_setblaster )
{
	/*   OBJ_DATA *wield;
	   OBJ_DATA *wield2;

	   wield = get_eq_char( ch, WEAR_WIELD );
	   if( wield && !( wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_BLASTER ) )
		  wield = NULL;
	   wield2 = get_eq_char( ch, WEAR_DUAL_WIELD );
	   if( wield2 && !( wield2->item_type == ITEM_WEAPON && wield2->value[3] == WEAPON_BLASTER ) )
		  wield2 = NULL;

	   if( !wield && !wield2 )
	   {
		  send_to_char( "&RYou don't seem to be wielding a blaster.\r\n&w", ch );
		  return;
	   }

	   if( argument[0] == '\0' )
	   {
		  send_to_char( "&RUsage: setblaster <full|high|normal|half|low|stun>\r\n&w", ch );
		  return;
	   }

	   if( wield )
		  act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield, NULL, TO_ROOM );

	   if( wield2 )
		  act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield2, NULL, TO_ROOM );

	   if( !str_cmp( argument, "full" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_FULL;
			 send_to_char( "&YWielded blaster set to FULL Power\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_FULL;
			 send_to_char( "&YDual wielded blaster set to FULL Power\r\n&w", ch );
		  }
		  return;
	   }
	   if( !str_cmp( argument, "high" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_HIGH;
			 send_to_char( "&YWielded blaster set to HIGH Power\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_HIGH;
			 send_to_char( "&YDual wielded blaster set to HIGH Power\r\n&w", ch );
		  }
		  return;
	   }
	   if( !str_cmp( argument, "normal" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_NORMAL;
			 send_to_char( "&YWielded blaster set to NORMAL Power\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_NORMAL;
			 send_to_char( "&YDual wielded blaster set to NORMAL Power\r\n&w", ch );
		  }
		  return;
	   }
	   if( !str_cmp( argument, "half" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_HALF;
			 send_to_char( "&YWielded blaster set to HALF Power\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_HALF;
			 send_to_char( "&YDual wielded blaster set to HALF Power\r\n&w", ch );
		  }
		  return;
	   }
	   if( !str_cmp( argument, "low" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_LOW;
			 send_to_char( "&YWielded blaster set to LOW Power\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_LOW;
			 send_to_char( "&YDual wielded blaster set to LOW Power\r\n&w", ch );
		  }
		  return;
	   }
	   if( !str_cmp( argument, "stun" ) )
	   {
		  if( wield )
		  {
			 wield->blaster_setting = BLASTER_STUN;
			 send_to_char( "&YWielded blaster set to STUN\r\n&w", ch );
		  }
		  if( wield2 )
		  {
			 wield2->blaster_setting = BLASTER_STUN;
			 send_to_char( "&YDual wielded blaster set to STUN\r\n&w", ch );
		  }
		  return;
	   }
	   else
		  do_setblaster( ch, "" );*/

}

CMDF( do_use )
{
	char arg[MAX_INPUT_LENGTH];
	char argd[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *device;
	OBJ_DATA *obj;
	OBJ_DATA *tetsu, *senbo;
	ch_ret retcode;
	int damv = 0;

	argument = one_argument( argument, argd );
	argument = one_argument( argument, arg );

	if( !str_cmp( arg, "on" ) )
		argument = one_argument( argument, arg );

	if( argd[0] == '\0' )
	{
		send_to_char( "Use what?\r\n", ch );
		return;
	}

	if( !str_cmp( argd, "ultima" ) )
	{
		tetsu = get_eq_char( ch, WEAR_HOLD );

		if( !tetsu )
			return;

		if( ( victim = who_fighting( ch ) ) == NULL )
		{
			send_to_char( "You aren't fighting anyone.\r\n", ch );
			return;
		}

		if( tetsu->pIndexData->vnum != 5127 )
		{
			send_to_char( "How can you use aren't holding?\r\n", ch );
			return;
		}

		damv = ( victim->hit / 2 );
		ch_printf( ch, "You let loose the green aura filled power of your %s!\r\n", tetsu->short_descr );
		ch_printf( victim, "%s lets insane power of %s around you!\r\n", ch->name, tetsu->short_descr );
		act( AT_SKILL, "$n watches as an insane green aura surrounds $N!", ch, NULL, victim, TO_NOTVICT );
		global_retcode = damage( ch, victim, damv, TYPE_UNDEFINED );
		WAIT_STATE( ch, PULSE_VIOLENCE );
		//     damage( ch, ch, ( ch->max_hit / 10 ), TYPE_HIT );
		return;
	}

	if( !str_cmp( argd, "scatter" ) )
	{
		senbo = get_eq_char( ch, WEAR_WIELD );

		if( !senbo )
			return;

		if( arg[0] == '\0' )
		{
			send_to_char( "On whom?\r\n", ch );
			return;
		}

		if( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			send_to_char( "That person isn't around!\r\n", ch );
			return;
		}

		if( senbo->pIndexData->vnum != 5128 )
		{
			send_to_char( "How can you use something you aren't wielding?\r\n", ch );
			return;
		}
		do_say( ch, "Scatter... Senbonzakura!!" );
		ch_printf( ch, "You unleash the power of your %s!\r\n", senbo->short_descr );
		act( AT_SKILL, "$n uses the power of Senbonzakura on $N!", ch, NULL, victim, TO_NOTVICT );
		damv = victim->hit / 4;
		damage( victim, victim, damv, TYPE_UNDEFINED );
		ch_printf( victim, "&PP&pi&Pn&Pk &Rc&rh&Re&rr&Rr&ry&w blossoms slowly flutter down from above... &b-&B< &C[&c%d&C] &B>&b-\r\n", damv );
		damv = victim->hit / 4;
		damage( victim, victim, damv, TYPE_UNDEFINED );
		ch_printf( victim, "&PP&pi&Pn&Pk &Rc&rh&Re&rr&Rr&ry&w blossoms continue to slash you... &b-&B< &C[&c%d&C] &B>&b-\r\n", damv );
		damv = victim->hit / 4;
		damage( victim, victim, damv, TYPE_UNDEFINED );
		ch_printf( victim, "&PP&pi&Pn&Pk &Rc&rh&Re&rr&Rr&ry&w blossoms slice you one last time... &b-&B< &C[&c%d&C] &B>&b-\r\n", damv );
		return;
	}

	if( !str_cmp( argd, "tetsusaiga" ) )
	{
		tetsu = get_eq_char( ch, WEAR_WIELD );

		if( !tetsu )
			return;

		if( ( victim = who_fighting( ch ) ) == NULL )
		{
			send_to_char( "You aren't fighting anyone.\r\n", ch );
			return;
		}

		if( tetsu->pIndexData->vnum != 5125 )
		{
			send_to_char( "How can you use something you aren't wielding?\r\n", ch );
			return;
		}

		damv = ( ch->hit / 5 );
		do_say( ch, "&RK&ra&pz&Pe &Bn&bo &YK&Oi&gz&Gu&G!&g!" );
		ch_printf( ch, "You unleash the power of your %s!\r\n", tetsu->short_descr );
		ch_printf( victim, "%s unleashes the power of %s on you!\r\n", ch->name, tetsu->short_descr );
		act( AT_SKILL, "$n unleashes the power of Tetsusaiga on $N!", ch, NULL, victim, TO_NOTVICT );
		global_retcode = damage( ch, victim, damv, TYPE_UNDEFINED );
		WAIT_STATE( ch, PULSE_VIOLENCE );
		//     damage( ch, ch, ( ch->max_hit / 10 ), TYPE_HIT );
		return;
	}

	if( ( device = get_eq_char( ch, WEAR_HOLD ) ) == NULL || !nifty_is_name( argd, device->name ) )
	{
		do_takedrug( ch, argd );
		return;
	}

	if( device->item_type == ITEM_SPICE )
	{
		do_takedrug( ch, argd );
		return;
	}

	if( device->item_type != ITEM_DEVICE )
	{
		send_to_char( "You can't figure out what it is your supposed to do with it.\r\n", ch );
		return;
	}

	if( device->value[2] <= 0 )
	{
		send_to_char( "It has no more charge left.", ch );
		return;
	}



	obj = NULL;
	if( arg[0] == '\0' )
	{
		if( ch->fighting )
		{
			victim = who_fighting( ch );
		}
		else
		{
			send_to_char( "Use on whom or what?\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
		{
			send_to_char( "You can't find your target.\r\n", ch );
			return;
		}
	}

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

	if( device->value[2] > 0 )
	{
		device->value[2]--;
		if( victim )
		{
			if( !oprog_use_trigger( ch, device, victim, NULL, NULL ) )
			{
				act( AT_MAGIC, "$n uses $p on $N.", ch, device, victim, TO_ROOM );
				act( AT_MAGIC, "You use $p on $N.", ch, device, victim, TO_CHAR );
			}
		}
		else
		{
			if( !oprog_use_trigger( ch, device, NULL, obj, NULL ) )
			{
				act( AT_MAGIC, "$n uses $p on $P.", ch, device, obj, TO_ROOM );
				act( AT_MAGIC, "You use $p on $P.", ch, device, obj, TO_CHAR );
			}
		}

		retcode = obj_cast_spell( device->value[3], device->value[0], ch, victim, obj );
		if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
		{
			bug( "do_use: char died", 0 );
			return;
		}
	}


	return;
}

CMDF( do_takedrug )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int drug;
	int sn = 0;

	if( argument[0] == '\0' || !str_cmp( argument, "" ) )
	{
		send_to_char( "Use what?\r\n", ch );
		return;
	}

	if( ( obj = find_obj( ch, argument, true ) ) == NULL )
		return;

	if( obj->item_type == ITEM_DEVICE )
	{
		send_to_char( "Try holding it first.\r\n", ch );
		return;
	}

	if( obj->item_type != ITEM_SPICE )
	{
		act( AT_ACTION, "$n looks at $p and scratches $s head.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You can't quite figure out what to do with $p.", ch, obj, NULL, TO_CHAR );
		return;
	}

	separate_obj( obj );
	if( obj->in_obj )
	{
		act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
	}

	if( ch->fighting && number_percent( ) > ( get_curr_dex( ch ) * 2 + 48 ) )
	{
		act( AT_MAGIC, "$n accidentally drops $p rendering it useless.", ch, obj, NULL, TO_ROOM );
		act( AT_MAGIC, "Oops... $p gets knocked from your hands rendering it completely useless!", ch, obj, NULL, TO_CHAR );
	}
	else
	{
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n takes $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You take $p.", ch, obj, NULL, TO_CHAR );
		}

		if( IS_NPC( ch ) )
		{
			extract_obj( obj );
			return;
		}

		drug = obj->value[0];

		WAIT_STATE( ch, PULSE_PER_SECOND / 4 );

		gain_condition( ch, COND_THIRST, 1 );

		ch->pcdata->drug_level[drug] = UMIN( ch->pcdata->drug_level[drug] + obj->value[1], 255 );
		if( ch->pcdata->drug_level[drug] >= 255 || ch->pcdata->drug_level[drug] > ( ch->pcdata->addiction[drug] + 100 ) )
		{
			act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "You feel sick. You may have taken too much.", ch, NULL, NULL, TO_CHAR );
			ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
			af.type = gsn_poison;
			af.location = APPLY_INT;
			af.modifier = -5;
			af.duration = ch->pcdata->drug_level[drug];
			af.bitvector = AFF_POISON;
			affect_to_char( ch, &af );
			ch->hit = 1;
		}

		switch( drug )
		{
		default:
		case SPICE_GLITTERSTIM:

			sn = skill_lookup( "true sight" );
			if( sn < MAX_SKILL && !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
			{
				af.type = sn;
				af.location = APPLY_AC;
				af.modifier = -10;
				af.duration = URANGE( 1, ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug], obj->value[1] );
				af.bitvector = AFF_TRUESIGHT;
				affect_to_char( ch, &af );
			}
			break;

		case SPICE_CARSANUM:

			sn = skill_lookup( "sanctuary" );
			if( sn < MAX_SKILL && !IS_AFFECTED( ch, AFF_SANCTUARY ) )
			{
				af.type = sn;
				af.location = APPLY_NONE;
				af.modifier = 0;
				af.duration = URANGE( 1, ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug], obj->value[1] );
				af.bitvector = AFF_SANCTUARY;
				affect_to_char( ch, &af );
			}
			break;

		case SPICE_RYLL:

			af.type = -1;
			af.location = APPLY_DEX;
			af.modifier = 1;
			af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af );

			af.type = -1;
			af.location = APPLY_HITROLL;
			af.modifier = 1;
			af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af );

			break;

		case SPICE_ANDRIS:

			af.type = -1;
			af.location = APPLY_HIT;
			af.modifier = 10;
			af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af );

			af.type = sn;
			af.location = APPLY_CON;
			af.modifier = 1;
			af.duration = URANGE( 1, 2 * ( ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ), 2 * obj->value[1] );
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af );

			break;

		}

	}
	if( cur_obj == obj->serial )
		global_objcode = rOBJ_EATEN;
	extract_obj( obj );
	return;
}

void jedi_bonus( CHAR_DATA *ch )
{
}

void sith_penalty( CHAR_DATA *ch )
{
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
CMDF( do_fill )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *source;
	short dest_item, src_item1, src_item2, src_item3, src_item4;
	int diff;
	bool all = false;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	/*
	 * munch optional words
	 */
	if( ( !str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ) ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Fill what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	}
	else
		dest_item = obj->item_type;

	src_item1 = src_item2 = src_item3 = src_item4 = -1;
	switch( dest_item )
	{
	default:
		act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
		send_to_char( "You cannot fill that.\r\n", ch );
		return;
		/*
		 * place all fillable item types here
		 */
	case ITEM_DRINK_CON:
		src_item1 = ITEM_FOUNTAIN;
		src_item2 = ITEM_BLOOD;
		break;
	case ITEM_HERB_CON:
		src_item1 = ITEM_HERB;
		src_item2 = ITEM_HERB_CON;
		break;
	case ITEM_PIPE:
		src_item1 = ITEM_HERB;
		src_item2 = ITEM_HERB_CON;
		break;
	case ITEM_CONTAINER:
		src_item1 = ITEM_CONTAINER;
		src_item2 = ITEM_CORPSE_NPC;
		src_item3 = ITEM_CORPSE_PC;
		src_item4 = ITEM_CORPSE_NPC;
		break;
	}

	if( dest_item == ITEM_CONTAINER )
	{
		if( IS_SET( obj->value[1], CONT_CLOSED ) )
		{
			act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		if( get_obj_weight( obj ) / obj->count >= obj->value[0] )
		{
			send_to_char( "It's already full as it can be.\r\n", ch );
			return;
		}
	}
	else
	{
		diff = obj->value[0] - obj->value[1];
		if( diff < 1 || obj->value[1] >= obj->value[0] )
		{
			send_to_char( "It's already full as it can be.\r\n", ch );
			return;
		}
	}

	if( dest_item == ITEM_PIPE && IS_SET( obj->value[3], PIPE_FULLOFASH ) )
	{
		send_to_char( "It's full of ashes, and needs to be emptied first.\r\n", ch );
		return;
	}

	if( arg2[0] != '\0' )
	{
		if( dest_item == ITEM_CONTAINER && ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) )
		{
			all = true;
			source = NULL;
		}
		else
			/*
			 * This used to let you fill a pipe from an object on the ground.  Seems
			 * to me you should be holding whatever you want to fill a pipe with.
			 * It's nitpicking, but I needed to change it to get a mobprog to work
			 * right.  Check out Lord Fitzgibbon if you're curious.  -Narn
			 */
			if( dest_item == ITEM_PIPE )
			{
				if( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
				{
					send_to_char( "You don't have that item.\r\n", ch );
					return;
				}
				if( source->item_type != src_item1 && source->item_type != src_item2
					&& source->item_type != src_item3 && source->item_type != src_item4 )
				{
					act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
					return;
				}
			}
			else
			{
				if( ( source = get_obj_here( ch, arg2 ) ) == NULL )
				{
					send_to_char( "You cannot find that item.\r\n", ch );
					return;
				}
			}
	}
	else
		source = NULL;

	if( !source && dest_item == ITEM_PIPE )
	{
		send_to_char( "Fill it with what?\r\n", ch );
		return;
	}

	if( !source )
	{
		bool found = false;
		OBJ_DATA *src_next;

		found = false;
		separate_obj( obj );
		for( source = ch->in_room->first_content; source; source = src_next )
		{
			src_next = source->next_content;
			if( dest_item == ITEM_CONTAINER )
			{
				if( !CAN_WEAR( source, ITEM_TAKE )
					|| ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
					|| ( IS_OBJ_STAT( source, ITEM_ARTIFACT ) )
					|| ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
					|| ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
					continue;
				if( all && arg2[3] == '.' && !nifty_is_name( &arg2[4], source->name ) )
					continue;
				obj_from_room( source );
				if( source->item_type == ITEM_MONEY )
				{
					ch->gold += source->value[0];
					extract_obj( source );
				}
				else
					obj_to_obj( source, obj );
				found = true;
			}
			else
				if( source->item_type == src_item1
					|| source->item_type == src_item2 || source->item_type == src_item3 || source->item_type == src_item4 )
				{
					found = true;
					break;
				}
		}
		if( !found )
		{
			switch( src_item1 )
			{
			default:
				send_to_char( "There is nothing appropriate here!\r\n", ch );
				return;
			case ITEM_FOUNTAIN:
				send_to_char( "There is no fountain or pool here!\r\n", ch );
				return;
			case ITEM_BLOOD:
				send_to_char( "There is no blood pool here!\r\n", ch );
				return;
			case ITEM_HERB_CON:
				send_to_char( "There are no herbs here!\r\n", ch );
				return;
			case ITEM_HERB:
				send_to_char( "You cannot find any smoking herbs.\r\n", ch );
				return;
			}
		}
		if( dest_item == ITEM_CONTAINER )
		{
			act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
			return;
		}
	}

	if( dest_item == ITEM_CONTAINER )
	{
		OBJ_DATA *otmp, *otmp_next;
		char name[MAX_INPUT_LENGTH];
		CHAR_DATA *gch;
		const char *pd;
		bool found = false;

		if( source == obj )
		{
			send_to_char( "You can't fill something with itself!\r\n", ch );
			return;
		}

		switch( source->item_type )
		{
		default:  /* put something in container */
			if( !source->in_room    /* disallow inventory items */
				|| !CAN_WEAR( source, ITEM_TAKE )
				|| ( IS_OBJ_STAT( source, ITEM_ARTIFACT ) )
				|| ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
				|| ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
				|| ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
			{
				send_to_char( "You can't do that.\r\n", ch );
				return;
			}
			separate_obj( obj );
			act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
			act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
			obj_from_room( source );
			obj_to_obj( source, obj );
			break;
		case ITEM_MONEY:
			send_to_char( "You can't do that... yet.\r\n", ch );
			break;
		case ITEM_CORPSE_PC:
			if( IS_NPC( ch ) )
			{
				send_to_char( "You can't do that.\r\n", ch );
				return;
			}

			pd = source->short_descr;
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );

			if( str_cmp( name, ch->name ) && !IS_IMMORTAL( ch ) )
			{
				bool fGroup;

				fGroup = false;
				for( gch = first_char; gch; gch = gch->next )
				{
					if( !IS_NPC( gch ) && is_same_group( ch, gch ) && !str_cmp( name, gch->name ) )
					{
						fGroup = true;
						break;
					}
				}
				if( !fGroup )
				{
					send_to_char( "That's someone else's corpse.\r\n", ch );
					return;
				}
			}

		case ITEM_CONTAINER:
			if( source->item_type == ITEM_CONTAINER /* don't remove */
				&& IS_SET( source->value[1], CONT_CLOSED ) )
			{
				act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
				return;
			}
		case ITEM_DROID_CORPSE:
		case ITEM_CORPSE_NPC:
			if( ( otmp = source->first_content ) == NULL )
			{
				send_to_char( "It's empty.\r\n", ch );
				return;
			}
			separate_obj( obj );
			for( ; otmp; otmp = otmp_next )
			{
				otmp_next = otmp->next_content;

				if( !CAN_WEAR( otmp, ITEM_TAKE )
					|| ( IS_OBJ_STAT( otmp, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
					|| ch->carry_number + otmp->count > can_carry_n( ch )
					|| ch->carry_weight + get_obj_weight( otmp ) > can_carry_w( ch )
					|| ( get_obj_weight( source ) + get_obj_weight( obj ) / obj->count ) > obj->value[0] )
					continue;
				obj_from_obj( otmp );
				obj_to_obj( otmp, obj );
				found = true;
			}
			if( found )
			{
				act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
				act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
			}
			else
				send_to_char( "There is nothing appropriate in there.\r\n", ch );
			break;
		}
		return;
	}

	if( source->value[1] < 1 )
	{
		send_to_char( "There's none left!\r\n", ch );
		return;
	}
	if( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
		separate_obj( source );
	separate_obj( obj );

	switch( source->item_type )
	{
	default:
		bug( "do_fill: got bad item type: %d", source->item_type );
		send_to_char( "Something went wrong...\r\n", ch );
		return;
	case ITEM_FOUNTAIN:
		if( obj->value[1] != 0 && obj->value[2] != 0 )
		{
			send_to_char( "There is already another liquid in it.\r\n", ch );
			return;
		}
		obj->value[2] = 0;
		obj->value[1] = obj->value[0];
		act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		return;
	case ITEM_BLOOD:
		if( obj->value[1] != 0 && obj->value[2] != 13 )
		{
			send_to_char( "There is already another liquid in it.\r\n", ch );
			return;
		}
		obj->value[2] = 13;
		if( source->value[1] < diff )
			diff = source->value[1];
		obj->value[1] += diff;
		act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		if( ( source->value[1] -= diff ) < 1 )
		{
			extract_obj( source );
			make_bloodstain( ch );
		}
		return;
	case ITEM_HERB:
		if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
		{
			send_to_char( "There is already another type of herb in it.\r\n", ch );
			return;
		}
		obj->value[2] = source->value[2];
		if( source->value[1] < diff )
			diff = source->value[1];
		obj->value[1] += diff;
		act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
		if( ( source->value[1] -= diff ) < 1 )
			extract_obj( source );
		return;
	case ITEM_HERB_CON:
		if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
		{
			send_to_char( "There is already another type of herb in it.\r\n", ch );
			return;
		}
		obj->value[2] = source->value[2];
		if( source->value[1] < diff )
			diff = source->value[1];
		obj->value[1] += diff;
		source->value[1] -= diff;
		act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		return;
	case ITEM_DRINK_CON:
		if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
		{
			send_to_char( "There is already another liquid in it.\r\n", ch );
			return;
		}
		obj->value[2] = source->value[2];
		if( source->value[1] < diff )
			diff = source->value[1];
		obj->value[1] += diff;
		source->value[1] -= diff;
		act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		return;
	}
}

CMDF( do_drink )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int amount;
	int liquid;

	argument = one_argument( argument, arg );
	/*
	 * munch optional words
	 */
	if( !str_cmp( arg, "from" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
			if( ( obj->item_type == ITEM_FOUNTAIN ) || ( obj->item_type == ITEM_BLOOD ) )
				break;

		if( !obj )
		{
			send_to_char( "Drink what?\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( obj = get_obj_here( ch, arg ) ) == NULL )
		{
			send_to_char( "You can't find it.\r\n", ch );
			return;
		}
	}

	if( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
		separate_obj( obj );

	if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 40 )
	{
		send_to_char( "You fail to reach your mouth.  *Hic*\r\n", ch );
		return;
	}

	switch( obj->item_type )
	{
	default:
		if( obj->carried_by == ch )
		{
			act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
		}
		break;

	case ITEM_POTION:
		if( obj->carried_by == ch )
			do_quaff( ch, obj->name );
		else
			send_to_char( "You're not carrying that.\r\n", ch );
		break;

	case ITEM_BLOOD:
		if( ch->pcdata->condition[COND_BLOODTHIRST] < ( 10 + ch->top_level / 100 ) )
		{
			amount = ( 10 + ch->top_level / 10 ) - ch->pcdata->condition[COND_BLOODTHIRST];

			if( ch->pcdata->condition[COND_FULL] >= 48 || ch->pcdata->condition[COND_THIRST] >= 48 )
			{
				send_to_char( "You are too full to drink any blood.\r\n", ch );
				return;
			}

			if( amount > obj->value[1] )
				amount = obj->value[1];

			obj->value[1] = obj->value[1] - amount;

			gain_condition( ch, COND_BLOODTHIRST, amount );

			if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_BLOOD, "$n drinks from the spilled blood.", ch, NULL, NULL, TO_ROOM );
				set_char_color( AT_BLOOD, ch );
				send_to_char( "You savour every last drop from the puddle of blood.\r\n", ch );

				if( obj->value[1] <= 1 )
				{
					if( ch->pcdata->condition[COND_BLOODTHIRST] < ( ch->top_level + 10 / 100 ) )
					{
						set_char_color( AT_BLOOD, ch );
						send_to_char( "You still desire more blood.\r\n", ch );
						act( AT_BLOOD, "$n still desires more blood.", ch, NULL, NULL, TO_ROOM );
					}
					else
					{
						set_char_color( AT_BLOOD, ch );
						send_to_char( "You lick your lips, satisifed with your fill of blood.\r\n", ch );
					}
				}
			}

			gain_condition( ch, COND_FULL, 1 );
			gain_condition( ch, COND_THIRST, 1 );

			if( --obj->value[1] <= 0 )
			{
				if( obj->serial == cur_obj )
					global_objcode = rOBJ_DRUNK;

				extract_obj( obj );
				make_bloodstain( ch );
			}
		}
		else
			send_to_char( "You're too full.\r\n", ch );

		break;
	case ITEM_FOUNTAIN:
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
			send_to_char( "You take a long thirst quenching drink.\r\n", ch );
		}

		if( !IS_NPC( ch ) )
			ch->pcdata->condition[COND_THIRST] = 40;
		break;

	case ITEM_DRINK_CON:
		if( obj->value[1] <= 0 )
		{
			send_to_char( "It is already empty.\r\n", ch );
			return;
		}

		if( ( liquid = obj->value[2] ) >= LIQ_MAX )
		{
			bug( "Do_drink: bad liquid number %d.", liquid );
			liquid = obj->value[2] = 0;
		}

		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM );
			act( AT_ACTION, "You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR );
		}

		amount = 1;    /* UMIN(amount, obj->value[1]); */
		/*
		 * what was this? concentrated drinks?  concentrated water
		 * too I suppose... sheesh!
		 */

		gain_condition( ch, COND_DRUNK, amount * liq_table[liquid].liq_affect[COND_DRUNK] );
		gain_condition( ch, COND_FULL, amount * liq_table[liquid].liq_affect[COND_FULL] );
		gain_condition( ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] );

		if( !IS_NPC( ch ) )
		{
			if( ch->pcdata->condition[COND_DRUNK] > 24 )
				send_to_char( "You feel quite sloshed.\r\n", ch );
			else if( ch->pcdata->condition[COND_DRUNK] > 18 )
				send_to_char( "You feel very drunk.\r\n", ch );
			else if( ch->pcdata->condition[COND_DRUNK] > 12 )
				send_to_char( "You feel drunk.\r\n", ch );
			else if( ch->pcdata->condition[COND_DRUNK] > 8 )
				send_to_char( "You feel a little drunk.\r\n", ch );
			else if( ch->pcdata->condition[COND_DRUNK] > 5 )
				send_to_char( "You feel light headed.\r\n", ch );

			if( ch->pcdata->condition[COND_FULL] > 40 )
				send_to_char( "You are full.\r\n", ch );

			if( ch->pcdata->condition[COND_THIRST] > 40 )
				send_to_char( "You feel bloated.\r\n", ch );
			else if( ch->pcdata->condition[COND_THIRST] > 36 )
				send_to_char( "Your stomach is sloshing around.\r\n", ch );
			else if( ch->pcdata->condition[COND_THIRST] > 30 )
				send_to_char( "You do not feel thirsty.\r\n", ch );
		}

		if( obj->value[3] )
		{
			/*
			 * The drink was poisoned!
			 */
			AFFECT_DATA af;

			act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
			ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
			af.type = gsn_poison;
			af.duration = 3 * obj->value[3];
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_POISON;
			affect_join( ch, &af );
		}

		obj->value[1] -= amount;
		break;
	}
	WAIT_STATE( ch, PULSE_PER_SECOND );
	return;
}

CMDF( do_eat )
{
	OBJ_DATA *obj;
	ch_ret retcode;
	int foodcond;
	//    AFFECT_DATA af;

	if( argument[0] == '\0' )
	{
		send_to_char( "Eat what?\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) || ch->pcdata->condition[COND_FULL] > 5 )
		if( ms_find_obj( ch ) )
			return;

	if( ( obj = find_obj( ch, argument, true ) ) == NULL )
		return;

	if( !IS_IMMORTAL( ch ) )
	{
		if( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
		{
			act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
			return;
		}

		if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
		{
			send_to_char( "You are too full to eat more.\r\n", ch );
			return;
		}
	}

	/*
	 * required due to object grouping
	 */
	separate_obj( obj );

	WAIT_STATE( ch, PULSE_PER_SECOND / 2 );

	if( obj->in_obj )
	{
		act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
	}
	if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		if( !obj->action_desc || obj->action_desc[0] == '\0' )
		{
			act( AT_ACTION, "$n eats $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
		}
		else
			actiondesc( ch, obj, NULL );
	}

	switch( obj->item_type )
	{

	case ITEM_FOOD:
		if( obj->timer > 0 && obj->value[1] > 0 )
			foodcond = ( obj->timer * 10 ) / obj->value[1];
		else
			foodcond = 10;

		if( !IS_NPC( ch ) )
		{
			int condition;

			condition = ch->pcdata->condition[COND_FULL];
			gain_condition( ch, COND_FULL, ( obj->value[0] * foodcond ) / 10 );
			if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
				send_to_char( "You are no longer hungry.\r\n", ch );
			else if( ch->pcdata->condition[COND_FULL] > 40 )
				send_to_char( "You are full.\r\n", ch );
		}

		if( obj->value[3] != 0 || ( foodcond < 4 && number_range( 0, foodcond + 1 ) == 0 ) )
		{
			/*
			 * The food was poisoned!
			 */
			AFFECT_DATA af;

			if( obj->value[3] != 0 )
			{
				act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
				act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
				ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
			}
			else
			{
				act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
				act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
				ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
			}

			af.type = gsn_poison;
			af.duration = 2 * obj->value[0] * ( obj->value[3] > 0 ? obj->value[3] : 1 );
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_POISON;
			affect_join( ch, &af );
		}
		break;

	case ITEM_RESTORE:
		if( !IS_NPC( ch ) )
		{
			ch->hit = ch->max_hit;
			ch->move = ch->max_move;
			ch->pcdata->condition[COND_FULL] = 100;
			ch->pcdata->condition[COND_THIRST] = 100;
			ch->mental_state = 0;
			ch->emotional_state = 0;

			send_to_char( "&GYou feel fully restored!\r\n", ch );
		}
		break;

	case ITEM_PILL:
		/*
		 * allow pills to fill you, if so desired
		 */
		if( !IS_NPC( ch ) && obj->value[4] )
		{
			int condition;

			condition = ch->pcdata->condition[COND_FULL];
			gain_condition( ch, COND_FULL, obj->value[4] );
			if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
				send_to_char( "You are no longer hungry.\r\n", ch );
			else if( ch->pcdata->condition[COND_FULL] > 40 )
				send_to_char( "You are full.\r\n", ch );
		}
		retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
		if( retcode == rNONE )
			retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
		if( retcode == rNONE )
			retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
		break;
	}

	if( obj->serial == cur_obj )
		global_objcode = rOBJ_EATEN;
	extract_obj( obj );
	return;
}

CMDF( do_quaff )
{
	OBJ_DATA *obj;
	ch_ret retcode;

	if( !IS_NPC( ch ) && xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "&RNot while in the arena!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' || !str_cmp( argument, "" ) )
	{
		send_to_char( "Quaff what?\r\n", ch );
		return;
	}

	if( ( obj = find_obj( ch, argument, true ) ) == NULL )
		return;

	if( obj->item_type != ITEM_POTION )
	{
		if( obj->item_type == ITEM_DRINK_CON )
			do_drink( ch, obj->name );
		else
		{
			act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
		}
		return;
	}

	/*
	 * Fullness checking                    -Thoric
	 */
	if( !IS_NPC( ch ) && ( ch->pcdata->condition[COND_FULL] >= 48 || ch->pcdata->condition[COND_THIRST] >= 48 ) )
	{
		send_to_char( "Your stomach cannot contain any more.\r\n", ch );
		return;
	}

	separate_obj( obj );
	if( obj->in_obj )
	{
		act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
	}

	/*
	 * If fighting, chance of dropping potion           -Thoric
	 */
	if( ch->fighting && number_percent( ) > ( get_curr_dex( ch ) * 2 + 48 ) )
	{
		act( AT_MAGIC, "$n accidentally drops $p and it smashes into a thousand fragments.", ch, obj, NULL, TO_ROOM );
		act( AT_MAGIC, "Oops... $p gets knocked from your hands and smashes into pieces!", ch, obj, NULL, TO_CHAR );
	}
	else
	{
		if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You quaff $p.", ch, obj, NULL, TO_CHAR );
		}

		WAIT_STATE( ch, PULSE_PER_SECOND / 4 );

		gain_condition( ch, COND_THIRST, 1 );
		retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
		if( retcode == rNONE )
			retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
		if( retcode == rNONE )
			retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	}
	if( cur_obj == obj->serial )
		global_objcode = rOBJ_QUAFFED;
	extract_obj( obj );
	return;
}


CMDF( do_recite )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *scroll;
	OBJ_DATA *obj;
	ch_ret retcode;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Activate what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	}

	if( scroll->item_type != ITEM_SCROLL )
	{
		act( AT_ACTION, "$n attempts to activate $p ... the silly fool.", ch, scroll, NULL, TO_ROOM );
		act( AT_ACTION, "You try to activate $p. (Now what?)", ch, scroll, NULL, TO_CHAR );
		return;
	}

	if( IS_NPC( ch ) && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
	{
		send_to_char( "As a mob, this dialect is foreign to you.\r\n", ch );
		return;
	}

	if( ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) && ( ch->top_level + 10 < scroll->value[0] ) )
	{
		send_to_char( "This item is too complex for you to understand.\r\n", ch );
		return;
	}

	obj = NULL;
	if( arg2[0] == '\0' )
		victim = ch;
	else
	{
		if( ( victim = get_char_room( ch, arg2 ) ) == NULL && ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You can't find it.\r\n", ch );
			return;
		}
	}

	separate_obj( scroll );
	act( AT_MAGIC, "$n activate $p.", ch, scroll, NULL, TO_ROOM );
	act( AT_MAGIC, "You activate $p.", ch, scroll, NULL, TO_CHAR );


	WAIT_STATE( ch, PULSE_PER_SECOND / 2 );

	retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
	if( retcode == rNONE )
		retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
	if( retcode == rNONE )
		retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );

	if( scroll->serial == cur_obj )
		global_objcode = rOBJ_USED;
	extract_obj( scroll );
	return;
}


/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	bool isup;
	ROOM_INDEX_DATA *room, *to_room = NULL;
	EXIT_DATA *pexit, *pexit_rev;
	int edir;
	const char *txt;

	if( IS_SET( obj->value[0], TRIG_UP ) )
		isup = true;
	else
		isup = false;
	switch( obj->item_type )
	{
	default:
		ch_printf( ch, "You can't %s that!\r\n", pull ? "pull" : "push" );
		return;
		break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
		if( ( !pull && isup ) || ( pull && !isup ) )
		{
			ch_printf( ch, "It is already %s.\r\n", isup ? "up" : "down" );
			return;
		}
	case ITEM_BUTTON:
		if( ( !pull && isup ) || ( pull & !isup ) )
		{
			ch_printf( ch, "It is already %s.\r\n", isup ? "in" : "out" );
			return;
		}
		break;
	}

	if( ( pull ) && xIS_SET( obj->pIndexData->progtypes, PULL_PROG ) )
	{
		if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
			REMOVE_BIT( obj->value[0], TRIG_UP );
		oprog_pull_trigger( ch, obj );
		return;
	}

	if( ( !pull ) && xIS_SET( obj->pIndexData->progtypes, PUSH_PROG ) )
	{
		if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
			SET_BIT( obj->value[0], TRIG_UP );
		oprog_push_trigger( ch, obj );
		return;
	}

	if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		snprintf( buf, MAX_STRING_LENGTH, "$n %s $p.", pull ? "pulls" : "pushes" );
		act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
		snprintf( buf, MAX_STRING_LENGTH, "You %s $p.", pull ? "pull" : "push" );
		act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
	}

	if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
	{
		if( pull )
			REMOVE_BIT( obj->value[0], TRIG_UP );
		else
			SET_BIT( obj->value[0], TRIG_UP );
	}

	if( IS_SET( obj->value[0], TRIG_TELEPORT )
		|| IS_SET( obj->value[0], TRIG_TELEPORTALL ) || IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
	{
		int flags;

		if( ( room = get_room_index( obj->value[1] ) ) == NULL )
		{
			bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
			return;
		}
		flags = 0;
		if( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
			SET_BIT( flags, TELE_SHOWDESC );
		if( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
			SET_BIT( flags, TELE_TRANSALL );
		if( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
			SET_BIT( flags, TELE_TRANSALLPLUS );

		teleport( ch, obj->value[1], flags );
		return;
	}

	if( IS_SET( obj->value[0], TRIG_RAND4 ) || IS_SET( obj->value[0], TRIG_RAND6 ) )
	{
		int maxd;

		if( ( room = get_room_index( obj->value[1] ) ) == NULL )
		{
			bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
			return;
		}

		if( IS_SET( obj->value[0], TRIG_RAND4 ) )
			maxd = 3;
		else
			maxd = 5;

		randomize_exits( room, maxd );
		for( rch = room->first_person; rch; rch = rch->next_in_room )
		{
			send_to_char( "You hear a loud rumbling sound.\r\n", rch );
			send_to_char( "Something seems different...\r\n", rch );
		}
	}

	/* Death support added by Remcon */
	if( IS_SET( obj->value[0], TRIG_DEATH ) )
	{
		/* Should we really send a message to the room? */
		act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
		act( AT_DEAD, "Oopsie... you're dead!\r\n", ch, NULL, NULL, TO_CHAR );
		snprintf( buf, MAX_STRING_LENGTH, "%s hit a DEATH TRIGGER in room %d!", ch->name, ch->in_room->vnum );
		log_string( buf );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_STAFF );

		/* Personaly I fiqured if we wanted it to be a full DT we could just have it send them into a DT. */
		set_cur_char( ch );
		raw_kill( ch, ch );

		/* If you want it to be more like a room deathtrap use this instead */
  /*
		if( is_npc( ch ) )
		   extract_char( ch, true );
		else
		   extract_char( ch, false );
  */
		return;
	}

	/* Object loading added by Remcon */
	if( IS_SET( obj->value[0], TRIG_OLOAD ) )
	{
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_DATA *tobj;

		/* value[1] for the obj vnum */
		if( !( pObjIndex = get_obj_index( obj->value[1] ) ) )
		{
			bug( "%s: obj points to invalid object vnum %d", __func__, obj->value[1] );
			return;
		}
		/* Set room to NULL before the check */
		room = NULL;
		/* value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room */
		if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
		{
			bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
			return;
		}
		/* Uses value[3] for level */
		if( !( tobj = create_object( pObjIndex, URANGE( 0, obj->value[3], MAX_LEVEL ) ) ) )
		{
			bug( "%s: obj couldnt create_obj vnum %d at level %d", __func__, obj->value[1], obj->value[3] );
			return;
		}
		if( room )
			obj_to_room( tobj, room );
		else
		{
			if( CAN_WEAR( obj, ITEM_TAKE ) )
				obj_to_char( tobj, ch );
			else
				obj_to_room( tobj, ch->in_room );
		}
		return;
	}

	/* Mob loading added by Remcon */
	if( IS_SET( obj->value[0], TRIG_MLOAD ) )
	{
		MOB_INDEX_DATA *pMobIndex;
		CHAR_DATA *mob;

		/* value[1] for the obj vnum */
		if( !( pMobIndex = get_mob_index( obj->value[1] ) ) )
		{
			bug( "%s: obj points to invalid mob vnum %d", __func__, obj->value[1] );
			return;
		}
		/* Set room to current room before the check */
		room = ch->in_room;
		/* value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room */
		if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
		{
			bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
			return;
		}
		if( !( mob = create_mobile( pMobIndex ) ) )
		{
			bug( "%s: obj couldnt create_mobile vnum %d", __func__, obj->value[1] );
			return;
		}
		char_to_room( mob, room );
		return;
	}

	/* Spell casting support added by Remcon */
	if( IS_SET( obj->value[0], TRIG_CAST ) )
	{
		if( obj->value[1] <= 0 || !IS_VALID_SN( obj->value[1] ) )
		{
			bug( "%s: obj points to invalid sn [%d]", __func__, obj->value[1] );
			return;
		}
		obj_cast_spell( obj->value[1], URANGE( 1, ( obj->value[2] > 0 ) ? obj->value[2] : ch->top_level, MAX_LEVEL ), ch, ch, NULL );
		return;
	}

	/* Container support added by Remcon */
	if( IS_SET( obj->value[0], TRIG_CONTAINER ) )
	{
		OBJ_DATA *container = NULL;

		room = get_room_index( obj->value[1] );
		if( !room )
			room = obj->in_room;
		if( !room )
		{
			bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
			return;
		}

		for( container = ch->in_room->first_content; container; container = container->next_content )
		{
			if( container->pIndexData->vnum == obj->value[2] )
				break;
		}
		if( !container )
		{
			bug( "%s: obj points to a container [%d] thats not where it should be?", __func__, obj->value[2] );
			return;
		}
		if( container->item_type != ITEM_CONTAINER )
		{
			bug( "%s: obj points to object [%d], but it isn't a container.", __func__, obj->value[2] );
			return;
		}
		/* Could toss in some messages. Limit how it is handled etc... I'll leave that to each one to do */
		/* Started to use TRIG_OPEN, TRIG_CLOSE, TRIG_LOCK, and TRIG_UNLOCK like TRIG_DOOR does. */
		/* It limits it alot, but it wouldn't allow for an EATKEY change */
		if( IS_SET( obj->value[3], CONT_CLOSEABLE ) )
			TOGGLE_BIT( container->value[1], CONT_CLOSEABLE );
		if( IS_SET( obj->value[3], CONT_PICKPROOF ) )
			TOGGLE_BIT( container->value[1], CONT_PICKPROOF );
		if( IS_SET( obj->value[3], CONT_CLOSED ) )
			TOGGLE_BIT( container->value[1], CONT_CLOSED );
		if( IS_SET( obj->value[3], CONT_LOCKED ) )
			TOGGLE_BIT( container->value[1], CONT_LOCKED );
		return;
	}

	if( IS_SET( obj->value[0], TRIG_DOOR ) )
	{
		room = get_room_index( obj->value[1] );
		if( !room )
			room = obj->in_room;
		if( !room )
		{
			bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
			return;
		}
		if( IS_SET( obj->value[0], TRIG_D_NORTH ) )
		{
			edir = DIR_NORTH;
			txt = "to the north";
		}
		else if( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
		{
			edir = DIR_SOUTH;
			txt = "to the south";
		}
		else if( IS_SET( obj->value[0], TRIG_D_EAST ) )
		{
			edir = DIR_EAST;
			txt = "to the east";
		}
		else if( IS_SET( obj->value[0], TRIG_D_WEST ) )
		{
			edir = DIR_WEST;
			txt = "to the west";
		}
		else if( IS_SET( obj->value[0], TRIG_D_UP ) )
		{
			edir = DIR_UP;
			txt = "from above";
		}
		else if( IS_SET( obj->value[0], TRIG_D_DOWN ) )
		{
			edir = DIR_DOWN;
			txt = "from below";
		}
		else
		{
			bug( "%s: door: no direction flag set.", __func__ );
			return;
		}
		pexit = get_exit( room, edir );
		if( !pexit )
		{
			if( !IS_SET( obj->value[0], TRIG_PASSAGE ) )
			{
				bug( "%s: obj points to non-exit %d", __func__, obj->value[1] );
				return;
			}
			to_room = get_room_index( obj->value[2] );
			if( !to_room )
			{
				bug( "%s: dest points to invalid room %d", __func__, obj->value[2] );
				return;
			}
			pexit = make_exit( room, to_room, edir );
			pexit->keyword = STRALLOC( "" );
			pexit->description = STRALLOC( "" );
			pexit->key = -1;
			pexit->exit_info = 0;
			top_exit++;
			act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR );
			act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM );
			return;
		}
		if( IS_SET( obj->value[0], TRIG_UNLOCK ) && IS_SET( pexit->exit_info, EX_LOCKED ) )
		{
			REMOVE_BIT( pexit->exit_info, EX_LOCKED );
			act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
			act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
			if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
				REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
			return;
		}
		if( IS_SET( obj->value[0], TRIG_LOCK ) && !IS_SET( pexit->exit_info, EX_LOCKED ) )
		{
			SET_BIT( pexit->exit_info, EX_LOCKED );
			act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
			act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
			if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
				SET_BIT( pexit_rev->exit_info, EX_LOCKED );
			return;
		}
		if( IS_SET( obj->value[0], TRIG_OPEN ) && IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			REMOVE_BIT( pexit->exit_info, EX_CLOSED );
			for( rch = room->first_person; rch; rch = rch->next_in_room )
				act( AT_ACTION, "The $d opens.", rch, NULL, pexit->keyword, TO_CHAR );
			if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
			{
				REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
				for( rch = to_room->first_person; rch; rch = rch->next_in_room )
					act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
			check_room_for_traps( ch, trap_door[edir] );
			return;
		}
		if( IS_SET( obj->value[0], TRIG_CLOSE ) && !IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			SET_BIT( pexit->exit_info, EX_CLOSED );
			for( rch = room->first_person; rch; rch = rch->next_in_room )
				act( AT_ACTION, "The $d closes.", rch, NULL, pexit->keyword, TO_CHAR );
			if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
			{
				SET_BIT( pexit_rev->exit_info, EX_CLOSED );
				for( rch = to_room->first_person; rch; rch = rch->next_in_room )
					act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
			check_room_for_traps( ch, trap_door[edir] );
			return;
		}
	}
}


CMDF( do_pull )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Pull what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
		return;
	}

	pullorpush( ch, obj, true );
}

CMDF( do_push )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Push what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
		return;
	}

	pullorpush( ch, obj, false );
}

/* pipe commands (light, tamp, smoke) by Thoric */
CMDF( do_tamp )
{
	OBJ_DATA *pipe;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Tamp what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You aren't carrying that.\r\n", ch );
		return;
	}
	if( pipe->item_type != ITEM_PIPE )
	{
		send_to_char( "You can't tamp that.\r\n", ch );
		return;
	}
	if( !IS_SET( pipe->value[3], PIPE_TAMPED ) )
	{
		act( AT_ACTION, "You gently tamp $p.", ch, pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n gently tamps $p.", ch, pipe, NULL, TO_ROOM );
		SET_BIT( pipe->value[3], PIPE_TAMPED );
		return;
	}
	send_to_char( "It doesn't need tamping.\r\n", ch );
}

CMDF( do_smoke )
{
	OBJ_DATA *pipe;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Smoke what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You aren't carrying that.\r\n", ch );
		return;
	}
	if( pipe->item_type != ITEM_PIPE )
	{
		act( AT_ACTION, "You try to smoke $p... but it doesn't seem to work.", ch, pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n tries to smoke $p... (I wonder what $e's been putting his $s pipe?)", ch, pipe, NULL, TO_ROOM );
		return;
	}
	if( !IS_SET( pipe->value[3], PIPE_LIT ) )
	{
		act( AT_ACTION, "You try to smoke $p, but it's not lit.", ch, pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n tries to smoke $p, but it's not lit.", ch, pipe, NULL, TO_ROOM );
		return;
	}
	if( pipe->value[1] > 0 )
	{
		if( !oprog_use_trigger( ch, pipe, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "You draw thoughtfully from $p.", ch, pipe, NULL, TO_CHAR );
			act( AT_ACTION, "$n draws thoughtfully from $p.", ch, pipe, NULL, TO_ROOM );
		}

		if( IS_VALID_HERB( pipe->value[2] ) && pipe->value[2] < top_herb )
		{
			int sn = pipe->value[2] + TYPE_HERB;
			SKILLTYPE *skill = get_skilltype( sn );

			WAIT_STATE( ch, skill->beats );
			if( skill->spell_fun )
				obj_cast_spell( sn, UMIN( skill->min_level, ch->top_level ), ch, ch, NULL );
			if( obj_extracted( pipe ) )
				return;
		}
		else
			bug( "do_smoke: bad herb type %d", pipe->value[2] );

		SET_BIT( pipe->value[3], PIPE_HOT );
		if( --pipe->value[1] < 1 )
		{
			REMOVE_BIT( pipe->value[3], PIPE_LIT );
			SET_BIT( pipe->value[3], PIPE_DIRTY );
			SET_BIT( pipe->value[3], PIPE_FULLOFASH );
		}
	}
}

CMDF( do_light )
{
	OBJ_DATA *pipe;
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Light what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( pipe = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You aren't carrying that.\r\n", ch );
		return;
	}
	if( pipe->item_type != ITEM_PIPE )
	{
		send_to_char( "You can't light that.\r\n", ch );
		return;
	}
	if( !IS_SET( pipe->value[3], PIPE_LIT ) )
	{
		if( pipe->value[1] < 1 )
		{
			act( AT_ACTION, "You try to light $p, but it's empty.", ch, pipe, NULL, TO_CHAR );
			act( AT_ACTION, "$n tries to light $p, but it's empty.", ch, pipe, NULL, TO_ROOM );
			return;
		}
		act( AT_ACTION, "You carefully light $p.", ch, pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n carefully lights $p.", ch, pipe, NULL, TO_ROOM );
		SET_BIT( pipe->value[3], PIPE_LIT );
		return;
	}
	send_to_char( "It's already lit.\r\n", ch );
}

CMDF( do_empty )
{
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Empty what?\r\n", ch );
		return;
	}
	if( ms_find_obj( ch ) )
		return;

	if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You aren't carrying that.\r\n", ch );
		return;
	}
	if( obj->count > 1 )
		separate_obj( obj );

	switch( obj->item_type )
	{
	default:
		act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
		return;
	case ITEM_PIPE:
		act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
		REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
		REMOVE_BIT( obj->value[3], PIPE_LIT );
		obj->value[1] = 0;
		return;
	case ITEM_DRINK_CON:
		if( obj->value[1] < 1 )
		{
			send_to_char( "It's already empty.\r\n", ch );
			return;
		}
		act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
		obj->value[1] = 0;
		return;
	case ITEM_CONTAINER:
		if( IS_SET( obj->value[1], CONT_CLOSED ) )
		{
			act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		if( !obj->first_content )
		{
			send_to_char( "It's already empty.\r\n", ch );
			return;
		}
		if( arg2[0] == '\0' )
		{
			if( xIS_SET( ch->in_room->room_flags, ROOM_NODROP )
				|| ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LITTERBUG ) ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "A magical force stops you!\r\n", ch );
				set_char_color( AT_TELL, ch );
				send_to_char( "Someone tells you, 'No littering here!'\r\n", ch );
				return;
			}
			if( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
			{
				send_to_char( "You can't seem to do that here...\r\n", ch );
				return;
			}
			if( empty_obj( obj, NULL, ch->in_room ) )
			{
				act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
				act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
				if( IS_SET( sysdata.save_flags, SV_DROP ) )
					save_char_obj( ch );
			}
			else
				send_to_char( "Hmmm... didn't work.\r\n", ch );
		}
		else
		{
			OBJ_DATA *dest = get_obj_here( ch, arg2 );

			if( !dest )
			{
				send_to_char( "You can't find it.\r\n", ch );
				return;
			}
			if( dest == obj )
			{
				send_to_char( "You can't empty something into itself!\r\n", ch );
				return;
			}
			if( dest->item_type != ITEM_CONTAINER )
			{
				send_to_char( "That's not a container!\r\n", ch );
				return;
			}
			if( IS_SET( dest->value[1], CONT_CLOSED ) )
			{
				act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
				return;
			}
			separate_obj( dest );
			if( empty_obj( obj, dest, NULL ) )
			{
				act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
				act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
				if( !dest->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
					save_char_obj( ch );
			}
			else
				act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
		}
		return;
	}
}

/*
 * Apply a salve/ointment					-Thoric
 */
CMDF( do_apply )
{
	OBJ_DATA *obj;
	ch_ret retcode;

	if( argument[0] == '\0' )
	{
		send_to_char( "Apply what?\r\n", ch );
		return;
	}

	if( ms_find_obj( ch ) )
		return;

	if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		send_to_char( "You do not have that.\r\n", ch );
		return;
	}

	if( obj->item_type != ITEM_SALVE )
	{
		act( AT_ACTION, "$n starts to rub $p on $mself...", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You try to rub $p on yourself...", ch, obj, NULL, TO_CHAR );
		return;
	}

	separate_obj( obj );

	--obj->value[1];
	if( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		if( !obj->action_desc || obj->action_desc[0] == '\0' )
		{
			act( AT_ACTION, "$n rubs $p onto $s body.", ch, obj, NULL, TO_ROOM );
			if( obj->value[1] <= 0 )
				act( AT_ACTION, "You apply the last of $p onto your body.", ch, obj, NULL, TO_CHAR );
			else
				act( AT_ACTION, "You apply $p onto your body.", ch, obj, NULL, TO_CHAR );
		}
		else
			actiondesc( ch, obj, NULL );
	}

	WAIT_STATE( ch, obj->value[2] );
	retcode = obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
	if( retcode == rNONE )
		retcode = obj_cast_spell( obj->value[5], obj->value[0], ch, ch, NULL );

	if( !obj_extracted( obj ) && obj->value[1] <= 0 )
		extract_obj( obj );

	return;
}

void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
	char charbuf[MAX_STRING_LENGTH];
	char roombuf[MAX_STRING_LENGTH];
	const char *srcptr = obj->action_desc;
	char *charptr = charbuf;
	char *roomptr = roombuf;
	const char *ichar;
	const char *iroom;

	while( *srcptr != '\0' )
	{
		if( *srcptr == '$' )
		{
			srcptr++;
			switch( *srcptr )
			{
			case 'e':
				ichar = "you";
				iroom = "$e";
				break;

			case 'm':
				ichar = "you";
				iroom = "$m";
				break;

			case 'n':
				ichar = "you";
				iroom = "$n";
				break;

			case 's':
				ichar = "your";
				iroom = "$s";
				break;

				/*
				 * case 'q':
				 * iroom = "s";
				 * break;
				 */

			default:
				srcptr--;
				*charptr++ = *srcptr;
				*roomptr++ = *srcptr;
				break;
			}
		}
		else if( *srcptr == '%' && *++srcptr == 's' )
		{
			ichar = "You";
			iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
		}
		else
		{
			*charptr++ = *srcptr;
			*roomptr++ = *srcptr;
			srcptr++;
			continue;
		}

		while( ( *charptr = *ichar ) != '\0' )
		{
			charptr++;
			ichar++;
		}

		while( ( *roomptr = *iroom ) != '\0' )
		{
			roomptr++;
			iroom++;
		}
		srcptr++;
	}

	*charptr = '\0';
	*roomptr = '\0';

	/*
	sprintf( buf, "Charbuf: %s", charbuf );
	log_string_plus( buf, LOG_HIGH, LEVEL_LIAISON );
	sprintf( buf, "Roombuf: %s", roombuf );
	log_string_plus( buf, LOG_HIGH, LEVEL_LIAISON );
	*/

	switch( obj->item_type )
	{
	case ITEM_BLOOD:
	case ITEM_FOUNTAIN:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	case ITEM_DRINK_CON:
		act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
		return;

	case ITEM_PIPE:
		return;

	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_LIGHT:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	case ITEM_FOOD:
	case ITEM_PILL:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	default:
		return;
	}
	return;
}

/*
 * Extended Bitvector Routines					-Thoric
 */

 /* check to see if the extended bitvector is completely empty */
bool ext_is_empty( EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		if( bits->bits[x] != 0 )
			return FALSE;

	return TRUE;
}

void ext_clear_bits( EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		bits->bits[x] = 0;
}

/* for use by xHAS_BITS() -- works like IS_SET() */
int ext_has_bits( EXT_BV *var, EXT_BV *bits )
{
	int x, bit;

	for( x = 0; x < XBI; x++ )
		if( ( bit = ( var->bits[x] & bits->bits[x] ) ) != 0 )
			return bit;

	return 0;
}

/* for use by xSAME_BITS() -- works like == */
bool ext_same_bits( EXT_BV *var, EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		if( var->bits[x] != bits->bits[x] )
			return FALSE;

	return TRUE;
}

/* for use by xSET_BITS() -- works like SET_BIT() */
void ext_set_bits( EXT_BV *var, EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		var->bits[x] |= bits->bits[x];
}

/* for use by xREMOVE_BITS() -- works like REMOVE_BIT() */
void ext_remove_bits( EXT_BV *var, EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		var->bits[x] &= ~( bits->bits[x] );
}

/* for use by xTOGGLE_BITS() -- works like TOGGLE_BIT() */
void ext_toggle_bits( EXT_BV *var, EXT_BV *bits )
{
	int x;

	for( x = 0; x < XBI; x++ )
		var->bits[x] ^= bits->bits[x];
}

/*
 * Read an extended bitvector from a file.			-Thoric
 */
EXT_BV fread_bitvector( FILE *fp )
{
	EXT_BV ret;
	int c, x = 0;
	int num = 0;

	memset( &ret, '\0', sizeof( ret ) );
	for( ;; )
	{
		num = fread_number( fp );
		if( x < XBI )
			ret.bits[x] = num;
		++x;
		if( ( c = getc( fp ) ) != '&' )
		{
			ungetc( c, fp );
			break;
		}
	}

	return ret;
}

/* return a string for writing a bitvector to a file */
char *print_bitvector( EXT_BV *bits )
{
	static char buf[XBI * 12];
	char *p = buf;
	int x, cnt = 0;

	for( cnt = XBI - 1; cnt > 0; cnt-- )
		if( bits->bits[cnt] )
			break;
	for( x = 0; x <= cnt; x++ )
	{
		snprintf( p, ( XBI * 12 ) - ( p - buf ), "%ud", bits->bits[x] );
		p += strlen( p );
		if( x < cnt )
			*p++ = '&';
	}
	*p = '\0';

	return buf;
}

/*
 * Write an extended bitvector to a file			-Thoric
 */
void fwrite_bitvector( EXT_BV *bits, FILE *fp )
{
	fputs( print_bitvector( bits ), fp );
}

EXT_BV meb( int bit )
{
	EXT_BV bits;

	xCLEAR_BITS( bits );
	if( bit >= 0 )
		xSET_BIT( bits, bit );

	return bits;
}

EXT_BV multimeb( int bit, ... )
{
	EXT_BV bits;
	va_list param;
	int b;

	xCLEAR_BITS( bits );
	if( bit < 0 )
		return bits;

	xSET_BIT( bits, bit );

	va_start( param, bit );

	while( ( b = va_arg( param, int ) ) != -1 )
		xSET_BIT( bits, b );

	va_end( param );

	return bits;
}


CMDF( do_hail )
{
	int vnum;
	ROOM_INDEX_DATA *room;
	char arg[MAX_INPUT_LENGTH];

	if( !ch->in_room )
		return;

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "&zSyntax: Hail <Docking/Hospital/Hotel>.\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still; you are DEAD.\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	case POS_SLEEPING:
		send_to_char( "In your dreams, or what?\r\n", ch );
		return;
	case POS_FIGHTING:
		send_to_char( "You should stop fighting first!\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
	{
		send_to_char( "You'll have to go outside to do that!\r\n", ch );
		return;
	}

	if( xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) )
	{
		send_to_char( "You can't do that on spacecraft!\r\n", ch );
		return;
	}

	if( ch->gold < ( ch->top_level - 9 ) )
	{
		send_to_char( "You don't have enough dollars!\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "hotel" ) )
	{
		vnum = ch->in_room->vnum;

		for( vnum = ch->in_room->area->low_vnum; vnum <= ch->in_room->area->hi_vnum; vnum++ )
		{
			room = get_room_index( vnum );

			if( room != NULL )
			{
				if( xIS_SET( room->room_flags, ROOM_HOTEL ) && !xIS_SET( room->room_flags, ROOM_PLR_HOME ) )
					break;
				else
					room = NULL;
			}
		}

		if( room == NULL )
		{
			send_to_char( "There doesn't seem to be any taxis nearby!\r\n", ch );
			return;
		}

		ch->gold -= UMAX( ch->top_level - 9, 0 );

		act( AT_ACTION, "$n hails a taxi, and drives off to seek shelter.", ch, NULL, NULL, TO_ROOM );

		char_from_room( ch );
		char_to_room( ch, room );

		send_to_char( "A taxi picks you up and drives you to the nearest hotel.\n\rYou pay the driver and get off.\r\n\n\n",
			ch );
		act( AT_ACTION, "$n $T", ch, NULL, "arrives on a taxi, gets off and pays the driver before it leaves.", TO_ROOM );
		do_look( ch, "auto" );
		return;
	}

	if( !str_cmp( arg, "hospital" ) )
	{
		vnum = ch->in_room->vnum;

		for( vnum = ch->in_room->area->low_vnum; vnum <= ch->in_room->area->hi_vnum; vnum++ )
		{
			room = get_room_index( vnum );

			if( room != NULL )
			{
				if( xIS_SET( room->room_flags, ROOM_HEAL ) )
					break;
				else
					room = NULL;
			}
		}

		if( room == NULL )
		{
			send_to_char( "There doesn't seem to be any taxis nearby!\r\n", ch );
			return;
		}

		ch->gold -= UMAX( ch->top_level - 9, 0 );

		act( AT_ACTION, "$n hails a taxi, and gets in.", ch, NULL, NULL, TO_ROOM );

		char_from_room( ch );
		char_to_room( ch, room );

		send_to_char
		( "You wave down a Taxi, jump in and tell the driver to take you to the hospital.\n\rYou pay the driver and get off.\r\n\n\n",
			ch );
		act( AT_ACTION, "$n $T", ch, NULL, "arrives on a taxi, gets off and pays the driver before it leaves.", TO_ROOM );
		do_look( ch, "auto" );
		return;
	}

	if( !str_cmp( arg, "docking" ) )
	{
		vnum = ch->in_room->vnum;

		for( vnum = ch->in_room->area->low_vnum; vnum <= ch->in_room->area->hi_vnum; vnum++ )
		{
			room = get_room_index( vnum );

			if( room != NULL )
			{
				if( xIS_SET( room->room_flags, ROOM_DOCKING ) )
					break;
				else
					room = NULL;
			}
		}

		if( room == NULL )
		{
			send_to_char( "There doesn't seem to be any taxis nearby!\r\n", ch );
			return;
		}

		ch->gold -= UMAX( ch->top_level - 9, 0 );

		act( AT_ACTION, "$n hails a taxi, and gets in.", ch, NULL, NULL, TO_ROOM );

		char_from_room( ch );
		char_to_room( ch, room );

		send_to_char
		( "You wave down a Taxi, get in, and tell the driver to take you to the nearest Docking Bay.\n\rYou pay the driver and get off.\r\n\n\n",
			ch );
		act( AT_ACTION, "$n $T", ch, NULL, "arrives on a taxi, gets off and pays the driver before it leaves.", TO_ROOM );
		do_look( ch, "auto" );
		return;
	}

	send_to_char( "&zSyntax: Hail <Docking/Hospital/Hotel>.\r\n", ch );
	return;

}


CMDF( do_train )
{

	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *mob;
	bool tfound = false;
	bool successful = false;

	if( IS_NPC( ch ) )
		return;

	strcpy( arg, argument );

	switch( ch->substate )
	{
	default:

		if( arg[0] == '\0' )
		{
			send_to_char( "Train what?\r\n", ch );
			send_to_char( "\n\rChoices: strength, intelligence, wisdom, dexterity, constitution or charisma\r\n", ch );
			return;
		}

		if( !IS_AWAKE( ch ) )
		{
			send_to_char( "In your dreams, or what?\r\n", ch );
			return;
		}

		for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
			if( IS_NPC( mob ) && xIS_SET( mob->act, ACT_TRAIN ) )
			{
				tfound = true;
				break;
			}

		if( ( !mob ) || ( !tfound ) )
		{
			send_to_char( "You can't do that here.\r\n", ch );
			return;
		}

		if( str_cmp( arg, "str" ) && str_cmp( arg, "strength" )
			&& str_cmp( arg, "dex" ) && str_cmp( arg, "dexterity" )
			&& str_cmp( arg, "con" ) && str_cmp( arg, "constitution" )
			&& str_cmp( arg, "cha" ) && str_cmp( arg, "charisma" )
			&& str_cmp( arg, "wis" ) && str_cmp( arg, "wisdom" )
			&& str_cmp( arg, "int" ) && str_cmp( arg, "intelligence" ) )
		{
			do_train( ch, "" );
			return;
		}

		if( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
		{
			if( mob->perm_str <= ch->perm_str || ch->perm_str >= 20 + race_table[ch->race].str_plus || ch->perm_str >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you are already stronger than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin your weight training.\r\n", ch );
		}
		if( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
		{
			if( mob->perm_dex <= ch->perm_dex || ch->perm_dex >= 20 + race_table[ch->race].dex_plus || ch->perm_dex >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you are already more dextrous than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin to work at some challenging tests of coordination.\r\n", ch );
		}
		if( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
		{
			if( mob->perm_int <= ch->perm_int || ch->perm_int >= 20 + race_table[ch->race].int_plus || ch->perm_int >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you are already more educated than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin your studies.\r\n", ch );
		}
		if( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
		{
			if( mob->perm_wis <= ch->perm_wis || ch->perm_wis >= 20 + race_table[ch->race].wis_plus || ch->perm_wis >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you are already far wiser than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin contemplating several ancient texts in an effort to gain wisdom.\r\n", ch );
		}
		if( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
		{
			if( mob->perm_con <= ch->perm_con || ch->perm_con >= 20 + race_table[ch->race].con_plus || ch->perm_con >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you are already healthier than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin your endurance training.\r\n", ch );
		}
		if( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
		{
			if( mob->perm_cha <= ch->perm_cha || ch->perm_cha >= 20 + race_table[ch->race].cha_plus || ch->perm_cha >= 25 )
			{
				act( AT_TELL, "$n tells you 'I cannot help you... you already are more charming than I.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			send_to_char( "&GYou begin lessons in maners and ettiquite.\r\n", ch );
		}
		add_timer( ch, TIMER_DO_FUN, 3, do_train, 1 );
		ch->dest_buf = str_dup( arg );
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
		send_to_char( "&RYou fail to complete your training.\r\n", ch );
		return;
	}

	ch->substate = SUB_NONE;

	if( number_bits( 2 ) == 0 )
	{
		successful = true;
	}

	if( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
	{
		if( !successful )
		{
			send_to_char( "&RYou feel that you have wasted alot of energy for nothing...\r\n", ch );
			return;
		}
		send_to_char( "&GAfter much of excercise you feel a little stronger.\r\n", ch );
		ch->perm_str++;
		return;
	}

	if( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
	{
		if( !successful )
		{
			send_to_char( "&RAfter all that training you still feel like a clutz...\r\n", ch );
			return;
		}
		send_to_char( "&GAfter working hard at many challenging tasks you feel a bit more coordinated.\r\n", ch );
		ch->perm_dex++;
		return;
	}

	if( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
	{
		if( !successful )
		{
			send_to_char( "&RHitting the books leaves you only with sore eyes...\r\n", ch );
			return;
		}
		send_to_char( "&GAfter much study you feel alot more knowledgeable.\r\n", ch );
		ch->perm_int++;
		return;
	}

	if( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
	{
		if( !successful )
		{
			send_to_char( "&RStudying the ancient texts has left you more confused than wise...\r\n", ch );
			return;
		}
		send_to_char
		( "&GAfter contemplating several seemingly meaningless events you suddenly \n\rreceive a flash of insight into the workings of the universe.\r\n",
			ch );
		ch->perm_wis++;
		return;
	}

	if( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
	{
		if( !successful )
		{
			send_to_char
			( "&RYou spend long a long arobics session ecersising very hard but finish \n\rfeeling only tired and out of breath....\r\n",
				ch );
			return;
		}
		send_to_char( "&GAfter a long tiring excersise session you feel much healthier than before.\r\n", ch );
		ch->perm_con++;
		return;
	}


	if( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
	{
		if( !successful )
		{
			send_to_char( "&RYou finish your self improvement session feeling a little depressed.\r\n", ch );
			return;
		}
		send_to_char
		( "&GYou spend some time focusing on how to improve your personality and feel \n\rmuch better about yourself and the ways others see you.\r\n",
			ch );
		ch->perm_cha++;
		return;
	}

}

CMDF( do_suicide )
{

	if( IS_NPC( ch ) || !ch->pcdata )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( ( ch->in_room->vnum == ROOM_VNUM_HELL ) || ( xIS_SET( ch->in_room->room_flags, ROOM_ARENA ) ) )
	{
		send_to_char( "Nice try, Stupid.\r\n", ch );
		return;
	}



	if( argument[0] == '\0' )
	{
		send_to_char( "&RIf you want to suicide, type \"suicide yes\".\r\n", ch );
		return;
	}

	if( !strcmp( argument, "yes" ) )
	{


		if( ch->position == POS_FIGHTING )
		{
			set_char_color( AT_RED, ch );
			send_to_char( "No way! You are fighting.\r\n", ch );
			return;
		}

		if( get_timer( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL( ch ) )
		{
			if( xIS_SET( ch->act, PLR_PKER ) )
			{
				set_char_color( AT_RED, ch );
				send_to_char( "Cool down before thinking about suicide!\r\n", ch );
				return;
			}
		}

		act( AT_BLOOD, "You give up trying to live, delving into the evil world of death.", ch, NULL, NULL, TO_CHAR );
		act( AT_BLOOD, "$n's body stops moving.. collapsing to the ground..", ch, NULL, NULL, TO_ROOM );

		set_cur_char( ch );
		raw_kill( ch, ch );
		return;
	}

	do_suicide( ch, "" );
}

CMDF( do_bank )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	short multiplier = /*( has_rate( ch, RATE_EXBANK ) ? 2 : */ 1 /*) */;
	long amount = 0;
	CHAR_DATA *victim;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if( IS_NPC( ch ) || !ch->pcdata )
		return;

	/*	if( !has_rate( ch, RATE_BANK ) )
		{
			send_to_char( "You don't rate banking! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	if( !ch->in_room || !xIS_SET( ch->in_room->room_flags, ROOM_BANK ) )
	{
		send_to_char( "You must be in a bank to do that!\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ch->top_level < 1001 && ch->pcdata->bank >( 100000000 * multiplier ) )
	{
		ch->pcdata->bank = 100000000 * multiplier;
	}

	if( arg1[0] == '\0' )
	{
		send_to_char
		( "\r\n&BSyntax&b: &CBank &c<&Cdeposit&B|&Cwithdraw&B|&Cbalance&B|&Ctransfer&c> &c[&Camount&c] &c<&CPerson?&c>\r\n",
			ch );
		return;
	}

	if( arg2[0] != '\0' )
		amount = atoi( arg2 );

	if( !str_prefix( arg1, "deposit" ) )
	{

		if( str_cmp( arg2, "all" ) && !is_number( arg2 ) )
		{
			send_to_char( "How much money do you wish to deposit?\r\n", ch );
			return;
		}

		if( ch->gold < amount )
		{
			send_to_char( "You don't have that many dollars on you.\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "all" ) )
			amount = ch->gold;
		else
			amount = atoi( arg2 );

		if( amount <= 0 )
		{
			send_to_char( "You may only deposit amounts greater than zero.\r\n", ch );
			do_bank( ch, "" );
			return;
		}

		ch->gold -= amount;
		ch->pcdata->bank += amount;

		ch_printf( ch, "You deposit %ld dollars into your account.\r\n", amount );
		return;
	}
	else if( !str_prefix( arg1, "withdraw" ) )
	{
		if( str_cmp( arg2, "all" ) && !is_number( arg2 ) )
		{
			send_to_char( "How much money do you wish to withdraw?\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "all" ) )
			amount = ch->pcdata->bank;
		else
			amount = atoi( arg2 );

		if( amount <= 0 )
		{
			send_to_char( "You may only withdraw amounts greater than zero.\r\n", ch );
			do_bank( ch, "" );
			return;
		}

		if( ch->pcdata->bank < amount )
		{
			send_to_char( "You don't have that many dollars in your account.\r\n", ch );
			return;
		}

		ch->gold += amount;
		ch->pcdata->bank -= amount;

		ch_printf( ch, "You withdraw %ld dollars from your account.\r\n", amount );
		return;

	}
	else if( !str_prefix( arg1, "balance" ) )
	{
		ch_printf( ch, "You have %ld dollars in your account.\r\n", ch->pcdata->bank );
		return;
	}

	else if( !str_prefix( arg1, "transfer" ) )
	{

		if( ( ( victim = get_char_world( ch, arg3 ) ) == NULL ) )
		{
			send_to_char( "No such player online.\r\n", ch );
			return;
		}

		if( ( IS_NPC( victim ) ) )
		{
			send_to_char( "No such player online.\r\n", ch );
			return;
		}

		if( str_cmp( arg2, "all" ) && !is_number( arg2 ) )
		{
			send_to_char( "How much money do you wish to transfer?\r\n", ch );
			return;
		}

		if( !str_cmp( arg2, "all" ) )
			amount = ch->pcdata->bank;
		else
			amount = atoi( arg2 );

		if( amount <= 0 )
		{
			send_to_char( "You may only transfer amounts greater than zero.\r\n", ch );
			do_bank( ch, "" );
			return;
		}

		if( ch->pcdata->bank < amount )
		{
			send_to_char( "You don't have that many dollars in the bank.\r\n", ch );
			return;
		}

		ch->pcdata->bank -= amount;
		victim->pcdata->bank += amount;

		if( amount == 1 )
		{
			ch_printf( ch, "You deposit %ld dollar into %s's account.\r\n", amount, victim->name );
			ch_printf( victim, "%s deposits %ld dollar into your account.\r\n", ch->name, amount );
			return;
		}

		else
		{
			ch_printf( ch, "You deposit %ld dollars into %s's account.\r\n", amount, victim->name );
			ch_printf( victim, "%s deposits %ld dollars into your account.\r\n", ch->name, amount );
			return;
		}
	}

	else
	{
		do_bank( ch, "" );
		return;
	}
}

void do_buzz( CHAR_DATA *ch, char *arg )
{
	char buf[MAX_STRING_LENGTH];
	short exit;
	ROOM_INDEX_DATA *home;
	EXIT_DATA *exitdat;

	if( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
		exit = 0;
	else if( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
		exit = 1;
	else if( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
		exit = 2;
	else if( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
		exit = 3;
	else if( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
		exit = 4;
	else if( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
		exit = 5;
	else if( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
		exit = 6;
	else if( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
		exit = 7;
	else if( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
		exit = 8;
	else if( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
		exit = 9;
	else
	{
		send_to_char( "&YRing the doorbell of what direction?\r\n", ch );
		return;
	}

	exitdat = get_exit( ch->in_room, exit );

	if( exitdat == NULL )
	{
		send_to_char( "&YThere isn't a home in that direction!\r\n", ch );
		return;
	}

	home = exitdat->to_room;

	if( xIS_SET( home->room_flags, ROOM_EMPTY_HOME ) )
	{
		send_to_char( "&YNo one lives there!\r\n", ch );
		return;
	}

	if( !xIS_SET( home->room_flags, ROOM_PLR_HOME ) )
	{
		send_to_char( "&YThat's not a home.\r\n", ch );
		return;
	}

	ch->buzzed_from_room = ch->in_room;

	sprintf( buf, "%s rings the doorbell. Ding Dong!", ch->name );
	echo_to_room( AT_WHITE, home, buf );
	send_to_char( "You ring the door bell.\r\n", ch );
	act( AT_ACTION, "$n rings a door bell.", ch, NULL, NULL, TO_ROOM );


}

CMDF( do_invite )
{
	ROOM_INDEX_DATA *home;
	CHAR_DATA *victim;

	home = ch->in_room;

	if( !xIS_SET( home->room_flags, ROOM_PLR_HOME ) || home != ch->plr_home )
	{
		send_to_char( "&RThis isn't your home!\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "&RInvite who?\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "&RThey aren't here.\r\n", ch );
		return;
	}

	if( victim->buzzed_from_room == NULL && victim->buzzed_home != home )
	{
		send_to_char( "&RThey didn't buzz your home.\r\n", ch );
		return;
	}

	if( victim->buzzed_from_room != victim->in_room )
	{
		send_to_char( "&RThey aren't outside your home anymore.\r\n", ch );
		return;
	}

	act( AT_ACTION, "You invite $N to enter, and $E steps inside.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n invites you to enter, and you step inside.", ch, NULL, victim, TO_VICT );
	char_from_room( victim );
	char_to_room( victim, home );
	victim->buzzed_home = NULL;
	victim->buzzed_from_room = NULL;
}

CMDF( do_setguests )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char *buf4;
	char arg[MAX_INPUT_LENGTH];

	if( IS_NPC( ch ) )
		return;

	/*	if( !has_rate( ch, RATE_SETGUEST ) )
		{
			send_to_char( "You don't rate editing guest lists! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	if( !xIS_SET( ch->in_room->room_flags, ROOM_PLR_HOME ) )
	{
		send_to_char( "This isn't your home... Or anyone else's, for that matter!\r\n", ch );
		return;
	}

	if( !ch->plr_home || ch->plr_home != ch->in_room )
	{
		send_to_char( "This isn't your home!\r\n", ch );
		return;
	}

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: setguests add <names>\r\n", ch );
		send_to_char( "       setguests remove <names>\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( !argument || argument[0] == '\0' )
	{
		do_setguests( ch, "" );
		return;
	}

	if( !str_cmp( arg, "add" ) )
	{
		strcpy( buf, ch->plr_home->guests );

		while( *argument )
		{
			argument = one_argument( argument, arg );

			if( !nifty_is_name( arg, buf ) )
			{
				sprintf( buf2, "%s ", arg );
				strcat( buf, buf2 );
			}
		}
		smash_tilde( buf );
		STRFREE( ch->plr_home->guests );
		ch->plr_home->guests = STRALLOC( buf );
		hidefoldmessage = true;
		fold_area( ch->plr_home->area, ch->plr_home->area->filename, false );
		hidefoldmessage = false;
		send_to_char( "Guest list updated.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "remove" ) )
	{
		strcpy( buf, ch->plr_home->guests );

		buf4 = buf;
		buf2[0] = '\0';

		while( *buf4 )
		{
			buf4 = one_argument( buf4, arg );

			if( !nifty_is_name( arg, argument ) )
			{
				sprintf( buf3, "%s ", arg );
				strcat( buf2, buf3 );
			}
		}

		STRFREE( ch->plr_home->guests );
		ch->plr_home->guests = STRALLOC( buf2 );
		hidefoldmessage = true;
		fold_area( ch->plr_home->area, ch->plr_home->area->filename, false );
		hidefoldmessage = false;
		send_to_char( "Guest list updated.\r\n", ch );
		return;
	}

	do_setguests( ch, "" );
	return;
}



CMDF( do_aquest )
{
	CHAR_DATA *questman;
	OBJ_DATA *obj = NULL, *obj_next;
	OBJ_INDEX_DATA *obj1, *obj2, *obj3, *obj4, *obj5;
	OBJ_INDEX_DATA *questinfoobj;
	MOB_INDEX_DATA *questinfo;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( !strcmp( arg1, "info" ) )
	{
		if( xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			if( ch->questmob == -1 && ch->questgiver->short_descr != NULL )
			{
				sprintf( buf, "Your task is ALMOST complete!\n\rGet back to %s before your time limit is up!\r\n",
					ch->questgiver->short_descr );
				send_to_char( buf, ch );
			}
			else if( ch->questobj > 0 )
			{
				questinfoobj = get_obj_index( ch->questobj );
				if( questinfoobj != NULL )
				{
					sprintf( buf, "You are on a task to pick up the %s!\r\n", questinfoobj->name );
					send_to_char( buf, ch );
				}
				else
					send_to_char( "You aren't doing a task for anyone.\r\n", ch );
				return;
			}
			else if( ch->questmob > 0 )
			{
				questinfo = get_mob_index( ch->questmob );
				if( questinfo != NULL )
				{
					sprintf( buf, "You are out to punk off %s!\r\n", questinfo->short_descr );
					send_to_char( buf, ch );
				}
				else
					send_to_char( "You aren't undertaking a task at the moment.\r\n", ch );
				return;
			}
		}
		else
			send_to_char( "You aren't undertaking a task at the moment.\r\n", ch );
		return;
	}
	if( !strcmp( arg1, "points" ) )
	{
		sprintf( buf, "You have %d favor points.\r\n", ch->questpoints );
		send_to_char( buf, ch );
		return;
	}
	else if( !strcmp( arg1, "time" ) )
	{
		if( !xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			send_to_char( "You aren't under taking a Task at the moment.\r\n", ch );
			if( ch->nextquest > 1 )
			{
				sprintf( buf, "There are %d minutes left until you can do another Task\r\n", ch->nextquest );
				send_to_char( buf, ch );
			}
			else if( ch->nextquest == 1 )
			{
				sprintf( buf, "Only about a minute more until you can do another Task.\r\n" );
				send_to_char( buf, ch );
			}
		}
		else if( ch->countdown > 0 )
		{
			sprintf( buf, "Time left for Task: %d\r\n", ch->countdown );
			send_to_char( buf, ch );
		}
		return;
	}

	/* Checks for a character in the room with spec_questmaster set. This special
	   procedure must be defined in special.c. You could instead use an
	   ACT_QUESTMASTER flag instead of a special procedure. */

	for( questman = ch->in_room->first_person; questman != NULL; questman = questman->next_in_room )
	{
		if( !IS_NPC( questman ) )
			continue;
		if( questman->spec_fun == spec_lookup( "spec_questmaster" ) )
			break;
	}

	if( questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( questman->position == POS_FIGHTING )
	{
		send_to_char( "Wait until the fighting stops.\r\n", ch );
		return;
	}

	ch->questgiver = questman;

	/* And, of course, you will need to change the following lines for YOUR
	   quest item information. Quest items on Moongate are unbalanced, very
	   very nice items, and no one has one yet, because it takes awhile to
	   build up quest points :> Make the item worth their while. */


	obj1 = get_obj_index( QUEST_ITEM1 );
	obj2 = get_obj_index( QUEST_ITEM2 );
	obj3 = get_obj_index( QUEST_ITEM3 );
	obj4 = get_obj_index( QUEST_ITEM4 );
	obj5 = get_obj_index( QUEST_ITEM5 );

	if( obj1 == NULL || obj2 == NULL || obj3 == NULL || obj4 == NULL || obj5 == NULL )
	{
		bug( "Error loading quest objects. Char: ", ch->name );
		return;
	}

	if( !strcmp( arg1, "list" ) )
	{
		act( AT_PLAIN, "$n asks $N for a list of 'Goods'.", ch, NULL, questman, TO_ROOM );
		act( AT_PLAIN, "You ask $N for a list of 'Goods'.", ch, NULL, questman, TO_CHAR );
		sprintf( buf,
			"\r\n&B[&C++&c++&C++&c++&C++&B] &RS&rt&po&Pl&pe&rn &GM&ge&zr&Wc&gh&Ga&gn&z&Wd&zi&gs&Ge &B[&C++&c++&C++&c++&C++&B]\r\n\r\n"
			"&P|&pFP Cost&P|          |&pItem&P|\r\n" "&Y%-5d         %-30s\r\n" "&Y%-5d         %-30s\r\n"
			"&Y%-5d         %-30s\r\n" "&Y%-5d         %-30s\r\n" "&Y%-5d         %-30s\r\n", QUEST_VALUE1,
			obj1->short_descr, QUEST_VALUE2, obj2->short_descr, QUEST_VALUE3, obj3->short_descr, QUEST_VALUE4,
			obj4->short_descr, QUEST_VALUE5, obj5->short_descr );

		send_to_char( buf, ch );
		return;
	}

	else if( !strcmp( arg1, "buy" ) )
	{
		if( arg2[0] == '\0' )
		{
			send_to_char( "To buy an item, type 'TASK BUY <item>'.\r\n", ch );
			return;
		}
		if( is_name( arg2, "1" ) )
		{
			if( ch->questpoints >= QUEST_VALUE1 )
			{
				ch->questpoints -= QUEST_VALUE1;
				obj = create_object( get_obj_index( QUEST_ITEM1 ), ch->top_level );
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "2" ) )
		{
			if( ch->questpoints >= QUEST_VALUE2 )
			{
				ch->questpoints -= QUEST_VALUE2;
				obj = create_object( get_obj_index( QUEST_ITEM2 ), ch->top_level );
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "3" ) )
		{
			if( ch->questpoints >= QUEST_VALUE3 )
			{
				ch->questpoints -= QUEST_VALUE3;
				obj = create_object( get_obj_index( QUEST_ITEM3 ), ch->top_level );
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "4" ) )
		{
			if( ch->questpoints >= QUEST_VALUE4 )
			{
				ch->questpoints -= QUEST_VALUE4;
				obj = create_object( get_obj_index( QUEST_ITEM4 ), ch->top_level );
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "5" ) )
		{
			if( ch->questpoints >= QUEST_VALUE5 )
			{
				ch->questpoints -= QUEST_VALUE5;
				obj = create_object( get_obj_index( QUEST_ITEM5 ), ch->top_level );
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "766" ) )
		{
			if( ch->questpoints >= 500 )
			{
				ch->questpoints -= 500;
				ch->gold += 100000;
				act( AT_MAGIC, "$N gives a pouch of gold to $n.", ch, NULL, questman, TO_ROOM );
				act( AT_MAGIC, "$N hands you a case of money.", ch, NULL, questman, TO_CHAR );
				return;
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else if( is_name( arg2, "77778" ) )
		{
			if( ch->questpoints >= 750 )
			{
				ch->questpoints -= 750;
				ch->max_hit += 10;
				act( AT_MAGIC, "$N waves his hand over $n. $n looks stronger.", ch, NULL, questman, TO_ROOM );
				act( AT_MAGIC, "$N waves his hand over you. You feel stronger.", ch, NULL, questman, TO_CHAR );
				return;
			}
			else
			{
				sprintf( buf, "Who you tryin to kid, %s? You need more more Favor Points.", ch->name );
				do_say( questman, buf );
				return;
			}
		}
		else
		{
			sprintf( buf, "Where's this magical item you want, %s?", ch->name );
			do_say( questman, buf );
		}
		if( obj != NULL )
		{
			act( AT_PLAIN, "$N gives something to $n.", ch, obj, questman, TO_ROOM );
			act( AT_PLAIN, "$N gives you your reward.", ch, obj, questman, TO_CHAR );
			obj_to_char( obj, ch );
		}
		return;
	}
	else if( !strcmp( arg1, "request" ) )
	{
		/*
					sprintf(buf, "Do to extreme stupidity of its system, the quest system has been removed\n\rThere are other ways to gain quest points now!");
				do_say(questman, buf);
				return;
		*/
		act( AT_PLAIN, "$n asks $N for a Task.", ch, NULL, questman, TO_ROOM );
		act( AT_PLAIN, "You ask $N for a Task.", ch, NULL, questman, TO_CHAR );
		if( xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			sprintf( buf, "Dude, you're already on a task, finish it first!" );
			do_say( questman, buf );
			return;
		}
		if( ch->nextquest > 0 )
		{
			sprintf( buf, "We don't respect people who hog Tasks, %s.", ch->name );
			do_say( questman, buf );
			sprintf( buf, "Scram for a while!" );
			do_say( questman, buf );
			return;
		}

		sprintf( buf, "Aight, cool %s!", ch->name );
		do_say( questman, buf );

		generate_quest( ch, questman );

		if( ch->questmob > 0 || ch->questobj > 0 )
		{
			ch->countdown = number_range( 10, 30 );
			xSET_BIT( ch->act, PLR_QUESTOR );
			sprintf( buf, "%d minutes is all you have to finish this task for me.", ch->countdown );
			do_say( questman, buf );
			sprintf( buf, "G'luck bud." );
			do_say( questman, buf );
		}
		return;
	}

	else if( !strcmp( arg1, "quit" ) )
	{
		act( AT_PLAIN, "$n informs $N $e doesn't want to do this task.", ch, NULL, questman, TO_ROOM );
		act( AT_PLAIN, "You tell $N you can't do this task.", ch, NULL, questman, TO_CHAR );
		if( ch->questgiver != questman || !xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			sprintf( buf, "I haven't even sent you on one..." );
			do_say( questman, buf );
			return;
		}

		if( ch->questobj > 0 )
		{
			bool obj_found = false;

			for( obj = ch->first_carrying; obj != NULL; obj = obj_next )
			{
				obj_next = obj->next;

				if( obj->carried_by == ch )
					if( obj != NULL && obj->pIndexData->vnum == ch->questobj )
					{
						obj_found = true;
						break;
					}
			}
			if( obj_found == true )
			{
				sprintf( buf, "You idiot, you have what I wanted!" );
				do_say( questman, buf );
				sprintf( buf, "Oops..." );
				do_say( ch, buf );
				do_aquest( ch, "complete" );
				return;
			}
		}
		sprintf( buf, "Can't complete it? Pssh..." );
		do_say( questman, buf );

		xREMOVE_BIT( ch->act, PLR_QUESTOR );
		ch->questgiver = NULL;
		ch->countdown = 0;
		ch->questmob = 0;
		ch->questobj = 0;
		ch->nextquest = 5;
		return;
	}


	else if( !strcmp( arg1, "complete" ) )
	{
		act( AT_PLAIN, "$n informs $N $e task is complete.", ch, NULL, questman, TO_ROOM );
		act( AT_PLAIN, "You inform $N you have completed $s his task.", ch, NULL, questman, TO_CHAR );
		if( ch->questgiver != questman )
		{
			sprintf( buf, "I never sent you on a Task.. maybe ask for one first..." );
			do_say( questman, buf );
			return;
		}

		if( xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			if( ch->questmob == -1 && ch->countdown > 0 )
			{
				int reward, pointreward;

				reward = number_range( 5000, 10000 );
				pointreward = number_range( 10, 50 );

				sprintf( buf, "Bout time you finished your task!" );
				do_say( questman, buf );
				sprintf( buf, "As thanks I'm giving you %d favor points, and %d dollars.", pointreward, reward );
				do_say( questman, buf );

				xREMOVE_BIT( ch->act, PLR_QUESTOR );
				ch->questgiver = NULL;
				ch->countdown = 0;
				ch->questmob = 0;
				ch->questobj = 0;
				ch->nextquest = 7;
				ch->gold += reward;
				ch->questpoints += pointreward;

				return;
			}
			else if( ch->questobj > 0 && ch->countdown > 0 )
			{
				bool obj_found = false;

				for( obj = ch->first_carrying; obj != NULL; obj = obj_next )
				{
					obj_next = obj->next;

					if( obj->carried_by == ch )
						if( obj != NULL && obj->pIndexData->vnum == ch->questobj )
						{
							obj_found = true;
							break;
						}
				}
				if( obj_found == true )
				{
					int reward, pointreward;

					reward = number_range( 200, 2000 );
					pointreward = number_range( 10, 50 );

					act( AT_PLAIN, "You hand $p to $N.", ch, obj, questman, TO_CHAR );
					act( AT_PLAIN, "$n hands $p to $N.", ch, obj, questman, TO_ROOM );

					sprintf( buf, "Bout time you completed your task!" );
					do_say( questman, buf );
					sprintf( buf, "As thanks I'm giving you %d favor points, and %d dollars.", pointreward, reward );
					do_say( questman, buf );

					xREMOVE_BIT( ch->act, PLR_QUESTOR );
					ch->questgiver = NULL;
					ch->countdown = 0;
					ch->questmob = 0;
					ch->questobj = 0;
					ch->nextquest = 7;
					ch->gold += reward;
					ch->questpoints += pointreward;
					extract_obj( obj );
					return;
				}
				else
				{
					sprintf( buf, "You haven't completed the quest yet, but there is still time!" );
					do_say( questman, buf );
					return;
				}
				return;
			}
			else if( ( ch->questmob > 0 || ch->questobj > 0 ) && ch->countdown > 0 )
			{
				sprintf( buf, "You haven't completed your task yet, there's still time HURRY up!" );
				do_say( questman, buf );
				return;
			}
		}
		if( ch->nextquest > 0 )
			sprintf( buf, "Too slow, you should work faster next time!" );
		else
			sprintf( buf, "Try asking for a quest first, %s.", ch->name );
		do_say( questman, buf );
		return;
	}

	send_to_char( "\n\rTASK commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\r\n", ch );
	send_to_char( "For more information, type 'HELP TASK'.\r\n", ch );
	return;
}

void generate_quest( CHAR_DATA *ch, CHAR_DATA *questman )
{
	CHAR_DATA *victim;
	MOB_INDEX_DATA *vsearch;
	ROOM_INDEX_DATA *room;
	OBJ_DATA *questitem;
	char buf[MAX_STRING_LENGTH];
	long mcounter;
	int level_diff, mob_vnum;
	//   bool noquest = false;
	//   PLANET_DATA *planet;

	   /*
		* Randomly selects a mob from the world mob list. If you don't
		* want a mob to be selected, make sure it is immune to summon.
		* Or, you could add a new mob flag called ACT_NOQUEST. The mob
		* is selected for both mob and obj quests, even tho in the obj
		* quest the mob is not used. This is done to assure the level
		* of difficulty for the area isn't too great for the player.
		*/

	for( mcounter = 0; mcounter < 99999; mcounter++ )
	{
		mob_vnum = number_range( 50, 32600 );

		if( ( vsearch = get_mob_index( mob_vnum ) ) != NULL )
		{
			level_diff = vsearch->level - ch->top_level;

			/*
			 * Level differences to search for. Moongate has 350
			 * levels, so you will want to tweak these greater or
			 * less than statements for yourself. - Vassago
			 */


			if( ( ( level_diff < -100 && level_diff > -100 )
				|| ( ch->top_level > 100 && ch->top_level < 100 && vsearch->level > 100 && vsearch->level < 100 )
				|| ( ch->top_level > 100 && vsearch->level > 100 ) )
				&& vsearch->pShop == NULL
				&& vsearch->rShop == NULL
				&& !xIS_SET( vsearch->act, ACT_NOQUEST )
				&& !xIS_SET( vsearch->act, ACT_TRAIN ) && !xIS_SET( vsearch->act, ACT_PRACTICE ) && qchance( 35 ) )

				break;


			else
				vsearch = NULL;
		}
	}

	if( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL || !IS_NPC( victim ) )
	{
		if( number_range( 0, 50 ) <= 35 )
		{
			for( mcounter = 0; mcounter < 99999; mcounter++ )
			{
				mob_vnum = number_range( 50, 32600 );

				if( ( vsearch = get_mob_index( mob_vnum ) ) != NULL )
				{
					level_diff = vsearch->level - ch->top_level;

					/*
					 * Level differences to search for. Moongate has 350
					 * levels, so you will want to tweak these greater or
					 * less than statements for yourself. - Vassago
					 */


					if( ( ( level_diff < -100 && level_diff > -100 )
						|| ( ch->top_level > 100 && ch->top_level < 100 && vsearch->level > 100 && vsearch->level < 100 )
						|| ( ch->top_level > 100 && vsearch->level > 100 ) )
						&& vsearch->pShop == NULL
						&& vsearch->rShop == NULL
						&& !xIS_SET( vsearch->act, ACT_NOQUEST )
						&& !xIS_SET( vsearch->act, ACT_TRAIN )
						&& !xIS_SET( vsearch->act, ACT_PRACTICE ) && qchance( 35 ) )

						break;


					else
						vsearch = NULL;
				}
			}
		}

		if( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL || !IS_NPC( victim ) )
		{
			sprintf( buf, "Sorry man, no Tasks right now, come back later." );
			do_say( questman, buf );
			ch->nextquest = 5;
			return;
		}
	}

	if( ( room = find_location( ch, victim->name ) ) == NULL )
	{
		if( number_range( 0, 50 ) <= 35 )
		{
			for( mcounter = 0; mcounter < 99999; mcounter++ )
			{
				mob_vnum = number_range( 50, 32600 );

				if( ( vsearch = get_mob_index( mob_vnum ) ) != NULL )
				{
					level_diff = vsearch->level - ch->top_level;

					/*
					 * Level differences to search for. Moongate has 350
					 * levels, so you will want to tweak these greater or
					 * less than statements for yourself. - Vassago
					 */



					if( ( ( level_diff < -100 && level_diff > -100 )
						|| ( ch->top_level > 100 && ch->top_level < 100 && vsearch->level > 100 && vsearch->level < 100 )
						|| ( ch->top_level > 100 && vsearch->level > 100 ) )
						&& vsearch->pShop == NULL
						&& vsearch->rShop == NULL
						&& !xIS_SET( vsearch->act, ACT_NOQUEST )
						&& !xIS_SET( vsearch->act, ACT_TRAIN )
						&& !xIS_SET( vsearch->act, ACT_PRACTICE ) && qchance( 35 ) )

						break;


					else
						vsearch = NULL;
				}
			}
		}

		if( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL || !IS_NPC( victim ) )
		{
			sprintf( buf, "Sorry man, no Tasks right now, come back later." );
			do_say( questman, buf );
			ch->nextquest = 5;
			return;
		}
		if( ( room = find_location( ch, victim->name ) ) == NULL )
		{
			sprintf( buf, "Sorry man, no Tasks right now, come back later." );
			do_say( questman, buf );
			ch->nextquest = 5;
			return;
		}
	}

	/*
	 * 40% chance it will send the player on a 'recover item' quest.
	 */

	if( qchance( 40 ) )
	{
		int objvnum = 0;

		switch( number_range( 0, 4 ) )
		{
		case 0:
			objvnum = QUEST_OBJQUEST1;
			break;

		case 1:
			objvnum = QUEST_OBJQUEST2;
			break;

		case 2:
			objvnum = QUEST_OBJQUEST3;
			break;

		case 3:
			objvnum = QUEST_OBJQUEST4;
			break;

		case 4:
			objvnum = QUEST_OBJQUEST5;
			break;
		}

		questitem = create_object( get_obj_index( objvnum ), ch->top_level );
		obj_to_room( questitem, room );
		questitem->timer = 30;
		ch->questobj = questitem->pIndexData->vnum;

		sprintf( buf, "Some POS stole a %s from us! We want it back!", questitem->short_descr );
		do_say( questman, buf );

		/*
		 * I changed my area names so that they have just the name of the area
		 * and none of the level stuff. You may want to comment these next two
		 * lines. - Vassago
		 */

		sprintf( buf, "Resources tell us its at %s, on  %s!", room->name, room->area->name );
		do_say( questman, buf );
		return;
	}

	/*
	 * Quest to kill a mob
	 */

	else
	{
		switch( number_range( 0, 1 ) )
		{
		case 0:
			sprintf( buf, "Yeah, so this punk %s, wants to mess with us. Take him Out!", victim->short_descr );
			do_say( questman, buf );
			break;

		case 1:
			sprintf( buf, "This punk, %s, stole from us.. you know what to do.", victim->short_descr );
			do_say( questman, buf );
			break;
		}

		if( room->name != NULL )
		{
			sprintf( buf, "%s's punk ass is over by %s on %s!", victim->short_descr, room->name, room->area->name );
			do_say( questman, buf );

			/*
			 * I changed my area names so that they have just the name of the area
			 * and none of the level stuff. You may want to comment these next two
			 * lines. - Vassago
			 */

			 //  sprintf(buf, "That location is in the general area of %s.",room->area->name);
			 //  do_say(questman, buf);
		}
		ch->questmob = victim->pIndexData->vnum;
	}
	return;
}

/* Called from update_handler() by pulse_area */

void quest_update( void )
{
	CHAR_DATA *ch, *ch_next;

	for( ch = first_char; ch != NULL; ch = ch_next )
	{
		ch_next = ch->next;

		if( IS_NPC( ch ) )
			continue;

		if( ch->nextquest > 0 )
		{
			ch->nextquest--;

			if( ch->nextquest == 0 )
			{
				send_to_char( "You can now do another Task.\r\n", ch );
				return;
			}
		}
		else if( xIS_SET( ch->act, PLR_QUESTOR ) )
		{
			if( --ch->countdown <= 0 )
			{
				char buf[MAX_STRING_LENGTH];

				ch->nextquest = 6;
				//          sprintf(buf, "Duo won't be impressed... You have %d minutes until you can do another Task.\r\n",ch->nextquest);
				send_to_char( buf, ch );
				xREMOVE_BIT( ch->act, PLR_QUESTOR );
				ch->questgiver = NULL;
				ch->countdown = 0;
				ch->questmob = 0;
			}
			if( ch->countdown > 0 && ch->countdown < 6 )
			{
				send_to_char( "Hurry up! You're almost out of time to complete your Task!\r\n", ch );
				return;
			}
		}
	}
	return;
}

CMDF( do_taskfail )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: taskfail <player>\r\n", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		send_to_char( "Mobs can't task.\r\n", ch );
		return;
	}

	if( xIS_SET( victim->act, PLR_QUESTOR ) )
		xREMOVE_BIT( victim->act, PLR_QUESTOR );
	victim->questgiver = NULL;
	victim->countdown = 0;
	victim->questmob = 0;
	victim->questobj = 0;
	victim->nextquest = 0;

	ch_printf( ch, "You just cleared %s's Task.\r\n", victim->name );
	ch_printf( victim, "Your Task has been cleared by %s.\r\n", ch->name );
	return;
}

CMDF( do_lottery )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	value = is_number( arg2 ) ? atoi( arg2 ) : -1;

	if( atoi( arg2 ) < -1 && value == -1 )
		value = atoi( arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		sprintf( buf, "\r\n&GThe lottery prize money is already at $&Y%d&G.&g\r\n"
			"&GHours until next draw: &g%d\r\n"
			"&GThe last winner was: &g%s\r\n", sysdata.jackpot, sysdata.lotterytimer, sysdata.lastwinner );
		send_to_char( buf, ch );
		send_to_char( "&BUsage: lottery buy <number>\r\n", ch );
		return;
	}

	if( !xIS_SET( ch->in_room->room_flags, ROOM_LOTTERY ) )
	{
		send_to_char( "You can't buy a lottery ticket here!\r\n", ch );
		return;
	}

	if( IS_SET( ch->pcdata->flags, PCFLAG_LOTTOWAIT ) )
	{
		send_to_char( "You can't buy a ticket at this time, please wait until next time.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "buy" ) )
	{

		if( ch->gold < 5000 )
		{
			send_to_char( "&RA lottery ticket costs $5,000.\r\n", ch );
			return;
		}

		if( IS_SET( ch->pcdata->flags, PCFLAG_HASLOTTO ) )
		{
			send_to_char( "You already bought a ticket!\r\n", ch );
			return;
		}

		//     if ( value < 0 )
		//       value = atoi( arg2 );
		if( ( value < 0 || value > 9999 ) )
		{
			ch_printf( ch, "Lottery number range is 0 - 9999.\n" );
			return;
		}

		ch->gold -= 5000;
		sysdata.jackpot += 20000;
		obj = create_object( get_obj_index( OBJ_VNUM_TICKET ), 1 );
		sprintf( buf, "A ticket with the number '%d'", value );
		STRFREE( obj->short_descr );
		obj->short_descr = STRALLOC( buf );
		obj_to_char( obj, ch );
		sprintf( buf, "&GYou purchase a lottery ticket.\r\n" );
		send_to_char( buf, ch );
		ch->pcdata->ticketnumber = value;
		SET_BIT( ch->pcdata->flags, PCFLAG_HASLOTTO );
		ch->pcdata->ticketweek = sysdata.lotteryweek;
		save_sysdata( sysdata );
		do_save( ch, "" );
		return;
	}
}
