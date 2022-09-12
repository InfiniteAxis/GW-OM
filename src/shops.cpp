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


/*
 * Local functions
 */

#define	CD	CHAR_DATA
CD *find_keeper( CHAR_DATA *ch );
int get_cost( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy );
#undef CD

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
	CHAR_DATA *keeper;
	SHOP_DATA *pShop;

	pShop = NULL;
	for( keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room )
		if( IS_NPC( keeper ) && ( pShop = keeper->pIndexData->pShop ) != NULL )
			break;

	if( !pShop )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return NULL;
	}


	/*
	 * Shop hours.
	 */
	if( time_info.hour < pShop->open_hour )
	{
		do_say( keeper, "Sorry, come back later." );
		return NULL;
	}

	if( time_info.hour > pShop->close_hour )
	{
		do_say( keeper, "Sorry, come back tomorrow." );
		return NULL;
	}

	return keeper;
}

int get_cost( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
	SHOP_DATA *pShop;
	int cost;
	bool richcustomer;
	int profitmod;

	if( !obj || ( pShop = keeper->pIndexData->pShop ) == NULL )
		return 0;

	if( ch->gold > ( ch->top_level * ch->top_level * 1000 ) )
		richcustomer = true;
	else
		richcustomer = false;

	if( fBuy )
	{
		cost = ( int ) ( cost * ( 80 + UMIN( ch->top_level, LEVEL_AVATAR ) ) ) / 100;

		profitmod = 13 - get_curr_cha( ch ) + ( richcustomer ? 15 : 0 )
			+ ( ( URANGE( 5, ch->top_level, LEVEL_AVATAR ) - 20 ) / 2 );
		cost = ( int ) ( obj->cost * UMAX( ( pShop->profit_sell + 1 ), pShop->profit_buy + profitmod ) ) / 100;
	}
	else
	{
		OBJ_DATA *obj2;
		int itype;

		profitmod = get_curr_cha( ch ) - 13 - ( richcustomer ? 15 : 0 );
		cost = 0;
		for( itype = 0; itype < MAX_TRADE; itype++ )
		{
			if( obj->item_type == pShop->buy_type[itype] )
			{
				cost = ( int ) ( obj->cost * UMIN( ( pShop->profit_buy - 1 ), pShop->profit_sell + profitmod ) ) / 100;
				break;
			}
		}
		for( obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content )
		{
			if( obj->pIndexData == obj2->pIndexData )
			{
				cost /= ( obj2->count + 1 );
				break;
			}
		}

		cost = UMIN( cost, 2500 );

	}


	if( obj->item_type == ITEM_DEVICE )
		cost = ( int ) ( cost * obj->value[2] / obj->value[1] );

	return cost;
}

CMDF( do_buy )
{
	CHAR_DATA *keeper;
	OBJ_DATA *obj, *buy_obj;
	int cost;
	int noi = 1; /* Number of items */
	short mnoi = 20;    /* Max number of items to be bought at once */
	char arg[MAX_INPUT_LENGTH];
	int maxgold;
	bool debit;

	argument = one_argument( argument, arg );

	if( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "Buy what?\r\n", ch );
		return;
	}

	if( ( keeper = find_keeper( ch ) ) == NULL )
		return;

	maxgold = keeper->top_level * 10;

	if( is_number( arg ) )
	{
		noi = atoi( arg );
		argument = one_argument( argument, arg );
		if( noi > mnoi )
		{
			act( AT_TELL, "$n tells you 'I don't sell that many items at" " once.'", keeper, NULL, ch, TO_VICT );
			ch->reply = keeper;
			return;
		}
	}

	if( argument[0] == '\0' )
		debit = false;
	else if( !str_cmp( "atm", argument ) || !str_cmp( "debit", argument ) )
	{
		bool has_card = false;

		for( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->item_type == ITEM_DEBIT_CARD )
				has_card = true;
		}

		if( has_card == true )
			debit = true;
		else
		{
			send_to_char( "&RYou are not carrying a debit card!\r\n", ch );
			return;
		}
	}

	obj = get_obj_carry( keeper, arg );

	if( !obj && arg[0] == '#' )
	{
		int onum, oref;
		bool ofound = false;

		onum = 0;
		oref = atoi( arg + 1 );
		for( obj = keeper->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
				onum++;
			if( onum == oref )
			{
				ofound = true;
				break;
			}
			else if( onum > oref )
				break;
		}
		if( !ofound )
			obj = NULL;
	}

	cost = ( get_cost( ch, keeper, obj, true ) * noi );
	if( cost <= 0 )
	{
		act( AT_TELL, "$n tells you 'I don't sell that -- try 'list'.'", keeper, NULL, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) && ( noi > 1 ) )
	{
		interpret( keeper, "laugh" );
		act( AT_TELL, "$n tells you 'I don't have enough of those in stock"
			" to sell more than one at a time.'", keeper, NULL, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( ch->gold < cost && debit == false )
	{
		act( AT_TELL, "$n tells you 'You can't afford to buy $p.'", keeper, obj, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( ch->pcdata->bank < cost && debit == true )
	{
		send_to_char( "&RYou swipe your card and the machine displayed INSUFFICIENT FUNDS!\r\n", ch );
		return;
	}

	if( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) && get_trust( ch ) < LEVEL_STAFF )
	{
		act( AT_TELL, "$n tells you 'This is a only a prototype!  I can't sell you that...'", keeper, NULL, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
		send_to_char( "You can't carry that many items.\r\n", ch );
		return;
	}

	if( ch->carry_weight + ( get_obj_weight( obj ) * noi ) + ( noi > 1 ? 2 : 0 ) > can_carry_w( ch ) )
	{
		send_to_char( "You can't carry that much weight.\r\n", ch );
		return;
	}

	if( noi == 1 )
	{
		if( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
			separate_obj( obj );
		act( AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You buy $p.", ch, obj, NULL, TO_CHAR );
	}
	else
	{
		sprintf( arg, "$n buys %d $p%s.", noi, ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's' ? "" : "s" ) );
		act( AT_ACTION, arg, ch, obj, NULL, TO_ROOM );
		sprintf( arg, "You buy %d $p%s.", noi, ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's' ? "" : "s" ) );
		act( AT_ACTION, arg, ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$N puts them into a bag and hands it to you.", ch, NULL, keeper, TO_CHAR );
	}

	if( debit == false )
		ch->gold -= cost;
	else if( debit == true )
		ch->pcdata->bank -= cost;

	keeper->gold += cost;

	if( keeper->gold > maxgold )
	{
		boost_economy( keeper->in_room->area, keeper->gold - maxgold / 2 );
		keeper->gold = maxgold / 2;
		act( AT_ACTION, "$n puts some money into a large safe.", keeper, NULL, NULL, TO_ROOM );
	}

	buy_obj = create_object( obj->pIndexData, obj->level );

	if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
	{
		OBJ_DATA *bag;

		/*
		 * Due to grouped objects and carry limitations in SMAUG
		 * The shopkeeper gives you a bag with multiple-buy,
		 * and also, only one object needs be created with a count
		 * set to the number bought.        -Thoric
		 */
		if( noi > 1 )
		{
			bag = create_object( get_obj_index( OBJ_VNUM_SHOPPING_BAG ), 1 );
			/*
			 * perfect size bag ;)
			 */
			bag->value[0] = bag->weight + ( buy_obj->weight * noi );
			buy_obj->count = noi;
			obj->pIndexData->count += ( noi - 1 );
			numobjsloaded += ( noi - 1 );
			obj_to_obj( buy_obj, bag );
			obj_to_char( bag, ch );
		}
		else
			obj_to_char( buy_obj, ch );
	}
	else
		obj_to_char( buy_obj, ch );

	return;
}


CMDF( do_list )
{
	if( xIS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
	{
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA *pet;
		bool found;

		pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
		if( !pRoomIndexNext )
		{
			bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
			send_to_char( "You can't do that here.\r\n", ch );
			return;
		}

		found = false;
		for( pet = pRoomIndexNext->first_person; pet; pet = pet->next_in_room )
		{
			if( xIS_SET( pet->act, ACT_PET ) && IS_NPC( pet ) )
			{
				if( !found )
				{
					found = true;
					send_to_char( "Pets for sale:\r\n", ch );
				}
				ch_printf( ch, "[%2d] %8d - %s\r\n", pet->top_level, 10 * pet->top_level * pet->top_level, pet->short_descr );
			}
		}
		if( !found )
			send_to_char( "Sorry, we're out of pets right now.\r\n", ch );
		return;
	}
	else
	{
		char arg[MAX_INPUT_LENGTH];
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost;
		int oref = 0;
		bool found;

		one_argument( argument, arg );

		if( ( keeper = find_keeper( ch ) ) == NULL )
			return;

		found = false;
		for( obj = keeper->last_carrying; obj; obj = obj->prev_content )
		{
			if( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
			{
				oref++;
				if( ( cost = get_cost( ch, keeper, obj, true ) ) > 0 && ( arg[0] == '\0' || nifty_is_name( arg, obj->name ) ) )
				{
					if( !found )
					{
						found = true;
						send_to_char( "\r\n&z[&YCost&z]    &G|&cref&G| &wItem\r\n", ch );
					}
					ch_printf( ch, "&z[&Y%-7d&z] &G|&c%3d&G|&w %s%s.\r\n", cost, oref, obj->short_descr,
						//          cost, oref, capitalize( obj->short_descr ),
						xIS_SET( obj->extra_flags, ITEM_HUTT_SIZE ) ? " (hutt size)" :
						( xIS_SET( obj->extra_flags, ITEM_LARGE_SIZE ) ? " (large)" :
							( xIS_SET( obj->extra_flags, ITEM_HUMAN_SIZE ) ? " (medium)" :
								( xIS_SET( obj->extra_flags, ITEM_SMALL_SIZE ) ? " (small)" : "" ) ) ) );
				}
			}
		}

		if( !found )
		{
			if( arg[0] == '\0' )
				send_to_char( "You can't buy anything here.\r\n", ch );
			else
				send_to_char( "You can't buy that here.\r\n", ch );
		}
		return;
	}
}


CMDF( do_sell )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Sell what?\r\n", ch );
		return;
	}

	if( ( keeper = find_keeper( ch ) ) == NULL )
		return;

	if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( !can_see_obj( keeper, obj ) )
	{
		send_to_char( "What are you trying to sell me? I don't buy thin air!\r\n", ch );
		return;
	}

	if( !can_drop_obj( ch, obj ) )
	{
		send_to_char( "You can't let go of it!\r\n", ch );
		return;
	}

	if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
	{
		send_to_char( "You can't sell that!\r\n", ch );
		return;
	}

	if( obj->timer > 0 )
	{
		act( AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", keeper, obj, ch, TO_VICT );
		return;
	}

	if( ( cost = get_cost( ch, keeper, obj, false ) ) <= 0 )
	{
		act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		return;
	}

	if( cost > keeper->gold )
	{
		act( AT_TELL, "$n makes a transaction.", keeper, obj, ch, TO_VICT );
		lower_economy( ch->in_room->area, cost - keeper->gold );
	}

	separate_obj( obj );
	act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
	sprintf( buf, "You sell $p for %d dollar%s.", cost, cost == 1 ? "" : "s" );
	act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
	ch->gold += cost;
	keeper->gold -= cost;
	if( keeper->gold < 0 )
		keeper->gold = 0;

	if( obj->item_type == ITEM_TRASH )
		extract_obj( obj );
	else if( xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
	{
		long ch_exp;

		ch_exp =
			UMIN( obj->cost * 10,
				( exp_level( ch->skill_level[SMUGGLING_ABILITY] + 1 ) -
					exp_level( ch->skill_level[SMUGGLING_ABILITY] ) ) / 10 );
		ch_printf( ch, "You receive %ld smuggling experience for unloading your contraband.\r\n ", ch_exp );
		gain_exp( ch, ch_exp, SMUGGLING_ABILITY );
		if( obj->item_type == ITEM_SPICE || obj->item_type == ITEM_RAWSPICE )
			extract_obj( obj );
		else
		{
			xREMOVE_BIT( obj->extra_flags, ITEM_CONTRABAND );
			obj_from_char( obj );
			obj_to_char( obj, keeper );
		}
	}
	else if( obj->item_type == ITEM_SPICE || obj->item_type == ITEM_RAWSPICE )
		extract_obj( obj );
	else
	{
		obj_from_char( obj );
		obj_to_char( obj, keeper );
	}

	return;
}



CMDF( do_value )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;

	if( argument[0] == '\0' )
	{
		send_to_char( "Value what?\r\n", ch );
		return;
	}

	if( ( keeper = find_keeper( ch ) ) == NULL )
		return;

	if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
		ch->reply = keeper;
		return;
	}

	if( !can_drop_obj( ch, obj ) )
	{
		send_to_char( "You can't let go of it!\r\n", ch );
		return;
	}

	if( ( cost = get_cost( ch, keeper, obj, false ) ) <= 0 )
	{
		act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		return;
	}

	sprintf( buf, "$n tells you 'I'll give you %d dollars for $p.'", cost );
	act( AT_TELL, buf, keeper, obj, ch, TO_VICT );
	ch->reply = keeper;

	return;
}

/* ------------------ Shop Building and Editing Section ----------------- */


CMDF( do_makeshop )
{
	SHOP_DATA *shop;
	int vnum;
	MOB_INDEX_DATA *mob;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeshop <mobvnum>\r\n", ch );
		return;
	}

	vnum = atoi( argument );

	if( ( mob = get_mob_index( vnum ) ) == NULL )
	{
		send_to_char( "Mobile not found.\r\n", ch );
		return;
	}

	if( !can_medit( ch, mob ) )
		return;

	if( mob->pShop )
	{
		send_to_char( "This mobile already has a shop.\r\n", ch );
		return;
	}

	CREATE( shop, SHOP_DATA, 1 );

	LINK( shop, first_shop, last_shop, next, prev );
	shop->keeper = vnum;
	shop->profit_buy = 120;
	shop->profit_sell = 90;
	shop->open_hour = 0;
	shop->close_hour = 23;
	mob->pShop = shop;
	send_to_char( "Done.\r\n", ch );
	return;
}


CMDF( do_shopset )
{
	SHOP_DATA *shop;
	MOB_INDEX_DATA *mob, *mob2;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum;
	int value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Usage: shopset <mob vnum> <field> value\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( "  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper\r\n", ch );
		return;
	}

	vnum = atoi( arg1 );

	if( ( mob = get_mob_index( vnum ) ) == NULL )
	{
		send_to_char( "Mobile not found.\r\n", ch );
		return;
	}

	if( !can_medit( ch, mob ) )
		return;

	if( !mob->pShop )
	{
		send_to_char( "This mobile doesn't keep a shop.\r\n", ch );
		return;
	}
	shop = mob->pShop;
	value = atoi( argument );

	if( !str_cmp( arg2, "buy0" ) )
	{
		if( !is_number( argument ) )
			value = get_otype( argument );
		if( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\r\n", ch );
			return;
		}
		shop->buy_type[0] = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "buy1" ) )
	{
		if( !is_number( argument ) )
			value = get_otype( argument );
		if( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\r\n", ch );
			return;
		}
		shop->buy_type[1] = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "buy2" ) )
	{
		if( !is_number( argument ) )
			value = get_otype( argument );
		if( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\r\n", ch );
			return;
		}
		shop->buy_type[2] = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "buy3" ) )
	{
		if( !is_number( argument ) )
			value = get_otype( argument );
		if( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\r\n", ch );
			return;
		}
		shop->buy_type[3] = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "buy4" ) )
	{
		if( !is_number( argument ) )
			value = get_otype( argument );
		if( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\r\n", ch );
			return;
		}
		shop->buy_type[4] = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "buy" ) )
	{
		if( value <= ( shop->profit_sell + 5 ) || value > 1000 )
		{
			send_to_char( "Out of range.\r\n", ch );
			return;
		}
		shop->profit_buy = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "sell" ) )
	{
		if( value < 0 || value >= ( shop->profit_buy - 5 ) )
		{
			send_to_char( "Out of range.\r\n", ch );
			return;
		}
		shop->profit_sell = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "open" ) )
	{
		if( value < 0 || value > 23 )
		{
			send_to_char( "Out of range.\r\n", ch );
			return;
		}
		shop->open_hour = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "close" ) )
	{
		if( value < 0 || value > 23 )
		{
			send_to_char( "Out of range.\r\n", ch );
			return;
		}
		shop->close_hour = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "keeper" ) )
	{
		if( ( mob2 = get_mob_index( vnum ) ) == NULL )
		{
			send_to_char( "Mobile not found.\r\n", ch );
			return;
		}
		if( !can_medit( ch, mob ) )
			return;
		if( mob2->pShop )
		{
			send_to_char( "That mobile already has a shop.\r\n", ch );
			return;
		}
		mob->pShop = NULL;
		mob2->pShop = shop;
		shop->keeper = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	do_shopset( ch, "" );
	return;
}


CMDF( do_shopstat )
{
	SHOP_DATA *shop;
	MOB_INDEX_DATA *mob;
	int vnum;

	if( argument[0] == '\0' )
	{
		send_to_char( "Usage: shopstat <keeper vnum>\r\n", ch );
		return;
	}

	vnum = atoi( argument );

	if( ( mob = get_mob_index( vnum ) ) == NULL )
	{
		send_to_char( "Mobile not found.\r\n", ch );
		return;
	}

	if( !mob->pShop )
	{
		send_to_char( "This mobile doesn't keep a shop.\r\n", ch );
		return;
	}
	shop = mob->pShop;

	ch_printf( ch, "Keeper: %d  %s\r\n", shop->keeper, mob->short_descr );
	ch_printf( ch, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]\r\n",
		o_types[shop->buy_type[0]],
		o_types[shop->buy_type[1]],
		o_types[shop->buy_type[2]], o_types[shop->buy_type[3]], o_types[shop->buy_type[4]] );
	ch_printf( ch, "Profit:  buy %3d%%  sell %3d%%\r\n", shop->profit_buy, shop->profit_sell );
	ch_printf( ch, "Hours:   open %2d  close %2d\r\n", shop->open_hour, shop->close_hour );
	return;
}


CMDF( do_shops )
{
	SHOP_DATA *shop;

	if( !first_shop )
	{
		send_to_char( "There are no shops.\r\n", ch );
		return;
	}

	set_char_color( AT_NOTE, ch );
	for( shop = first_shop; shop; shop = shop->next )
		ch_printf( ch, "Keeper: %5d Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d\r\n",
			shop->keeper, shop->profit_buy, shop->profit_sell,
			shop->open_hour, shop->close_hour,
			shop->buy_type[0], shop->buy_type[1], shop->buy_type[2], shop->buy_type[3], shop->buy_type[4] );
	return;
}
