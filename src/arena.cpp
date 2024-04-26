/****************************************************************************
* Automated Warfare Code                                                   *
* Markanth : dlmud@dlmud.com                                               *
* Devil's Lament : dlmud.com port 3778                                     *
* Web Page : http://www.dlmud.com                                          *
*                                                                          *
* Provides 4 types of automated wars.                                      *
*                                                                          *
* All I ask in return is that you give me credit on your mud somewhere     *
* or email me if you use it.                                               *
****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mud.h"


const char *wartype_name( int type )
{
	switch( type )
	{
	case 1:
		return "Race";
	case 2:
		return "Genocide";
	case 3:
		return "Gang";
	default:
		return "Unknown";
	}
}

CMDF( do_startwar )
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *wch;

	int blevel, elevel, type;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		send_to_char( "&BSyntax&b: &zstartwar <MinLevel> <MaxLevel> <Type> <SD?>\r\n"
			"&CType&c: &B1 &z- &Brace war&z, &B2 &z- &BGenocide War&z, &B3 &z- &BGang War\r\n", ch );
		return;
	}

	blevel = atoi( arg1 );
	elevel = atoi( arg2 );
	type = atoi( arg3 );

	if( blevel <= 0 || blevel > MAX_LEVEL )
	{
		snprintf( buf, MAX_STRING_LENGTH, "Level must be between 1 and %d.\r\n", MAX_LEVEL );
		send_to_char( buf, ch );
		return;
	}

	if( blevel <= 6 || elevel > MAX_LEVEL )
	{
		snprintf( buf, MAX_STRING_LENGTH, "Level must be between 7 and %d.\r\n", MAX_LEVEL );
		send_to_char( buf, ch );
		return;
	}

	if( elevel < blevel )
	{
		send_to_char( "Max level must be greater than the min level.\r\n", ch );
		return;
	}

	if( elevel - blevel < 5 )
	{
		send_to_char( "Levels must have a difference of at least 5.\r\n", ch );
		return;
	}

	if( type < 1 || type > 3 )
	{
		send_to_char( "The type either has to be 1 (race), 2 (genocide) or 3 (gang).\r\n", ch );
		return;
	}

	if( war_info.iswar != WAR_OFF )
	{
		send_to_char( "There is already a war going!\r\n", ch );
		return;
	}

	if( !IS_IMMORTAL( ch ) )
	{
		send_to_char( "&ROnly Immortals can start a war!\r\n", ch );
		return;
	}
	war_info.suddendeath = false;


	if( !str_cmp( arg4, "sd" ) )
	{
		war_info.suddendeath = true;
	}

	war_info.iswar = WAR_WAITING;
	war_info.min_level = blevel;
	war_info.max_level = elevel;
	war_info.inwar = 0;
	war_info.wartype = type;
	snprintf( buf, MAX_STRING_LENGTH, "%s &CA War has begun! A &B%s&C war, for levels &B%d&C to &B%d&C.",
		war_info.suddendeath == true ? "&rx&RX&P|&BS&bD &BM&bO&BD&bE&P|&RX&rx" : "",
		wartype_name( war_info.wartype ), war_info.min_level, war_info.max_level );
	war_channel( buf );
	snprintf( buf, MAX_STRING_LENGTH, "&CYou announce a &B%s&C war for levels &B%d&C to&B %d&C.\r\n",
		wartype_name( war_info.wartype ), war_info.min_level, war_info.max_level );
	send_to_char( buf, ch );
	war_info.timer = 3;
	war_info.next = 0;
	for( wch = first_char; wch != NULL; wch = wch->next )
	{
		if( !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_WAR ) )
			REMOVE_BIT( wch->pcdata->flags, PCFLAG_WAR );
	}
	return;
}

CMDF( do_awho )
{
	CHAR_DATA *tch;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;

	argument = one_argument( argument, arg );

	if( ch->in_room == get_room_index( 6 ) )
	{
		send_to_char( "&RAhahaha, Yeah Right!\r\n", ch );
		return;
	}

	if( arg[0] == '\0' )
	{
		send_to_char( "&zSyntax: Arena <who/join/single/leave>.\r\n", ch );
		return;
	}

	switch( ch->position )
	{
	case POS_DEAD:
		send_to_char( "Lie still, you are DEAD!\r\n", ch );
		return;
	case POS_INCAP:
	case POS_MORTAL:
		send_to_char( "You are hurt far too bad for that.\r\n", ch );
		return;
	case POS_STUNNED:
		send_to_char( "You are too stunned to do that.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "end" ) && IS_IMMORTAL( ch ) )
	{
		end_war( );
		snprintf( buf, MAX_STRING_LENGTH, "%s has ended the war.", ch->name );
		send_to_char( buf, ch );
		return;
	}

	if( !str_cmp( arg, "who" ) )
	{
		if( war_info.inwar == 0 )
		{
			send_to_char( "No one is in the Arena.\r\n", ch );
			return;
		}

		snprintf( buf, MAX_STRING_LENGTH,
			"\r\n&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O- &GO&gp&Ge&gr&Ga&gt&Gi&go&Gn &CM&ce&Ct&ce&Co&cr &RA&rr&Re&rn&Ra &O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>&O-&Y<&O-&Y>\r\n\r\n" );
		snprintf( buf, MAX_STRING_LENGTH, "&B              &GTime To Start&g:&c %-3d      ", war_info.timer );
		snprintf( buf, MAX_STRING_LENGTH, "&GLevels&g:&c &c%d &Gto &c%d\r\n\r\n", war_info.min_level, war_info.max_level );
		send_to_char( buf, ch );
		send_to_char( "&G|&cLevel&G|    |&cName        &G|   |&cFighting   &G|\r\n", ch );
		send_to_char
		( "&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O-&Y-&O\r\n",
			ch );

		for( tch = first_char; tch; tch = tch->next )
			if( tch->in_room && xIS_SET( tch->in_room->room_flags, ROOM_ARENA ) )
			{
				snprintf( buf2, MAX_STRING_LENGTH, "&c|&G%-5d&c|    |&G%-12s&c|   |&G%-12s&c|\r\n", tch->top_level, tch->name,
					tch->fighting ? tch->fighting->who->name : "No One" );
				send_to_char( buf2, ch );
			}
		return;
	}

	if( !str_cmp( arg, "leave" ) )
	{
		if( iswar == true )
		{
			send_to_char( "You can't leave in the middle of a war!\r\n", ch );
			return;
		}

		if( ch->position == POS_FIGHTING )
		{
			send_to_char( "Not while you're fighting!\r\n", ch );
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
		}

		if( !IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
		{
			send_to_char( "You aren't in the arena!\r\n", ch );
			return;
		}

		REMOVE_BIT( ch->pcdata->flags, PCFLAG_WAR );
		char_from_room( ch );
		char_to_room( ch, get_room_index( 4114 ) );
		do_look( ch, "auto" );

	}

	else if( !str_cmp( arg, "join" ) )
	{
		if( war_info.iswar == WAR_RUNNING )
		{
			send_to_char( "The war has allready started, your too late.\r\n", ch );
			return;
		}

		if( ch->top_level < war_info.min_level || ch->top_level > war_info.max_level )
		{
			send_to_char( "Sorry, you can't join this war.\r\n", ch );
			return;
		}

		if( IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
		{
			send_to_char( "You are already in the war.\r\n", ch );
			return;
		}

		if( war_info.wartype == 3 && !ch->pcdata->clan )
		{
			send_to_char( "You aren't in a gang, you can't join this war.\r\n", ch );
			return;
		}

		if( ( location = get_room_index( ROOM_VNUM_WAITINGROOM ) ) == NULL )
		{
			send_to_char( "Arena is not yet completed, sorry.\r\n", ch );
			return;
		}
		else

		{

			act( AT_MAGIC, "$n dissapears in a flash!", ch, NULL, NULL, TO_ROOM );
			ch->retran = ch->in_room->vnum;
			char_from_room( ch );
			char_to_room( ch, location );
			SET_BIT( ch->pcdata->flags, PCFLAG_WAR );
			snprintf( buf, MAX_STRING_LENGTH, "&B%s &Chas entered the competition!", ch->name );
			talk_arena( buf );
			act( AT_MAGIC, "$n appears in a flash!", ch, NULL, NULL, TO_ROOM );
			war_info.inwar++;
			do_look( ch, "auto" );
			return;
		}
	}

	else if( !str_cmp( arg, "single" ) )

	{
		ROOM_INDEX_DATA *random;

		if( war_info.iswar == WAR_RUNNING )
		{
			send_to_char( "&RWait until the competition is done!\r\n", ch );
			return;
		}

		if( IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
		{
			send_to_char( "I don't think so.\r\n", ch );
			return;
		}

		if( ch->in_room != get_room_index( 4114 ) )
		{
			send_to_char( "&RYou aren't in the entrance room!\r\n", ch );
			return;
		}
		else
		{
			act( AT_MAGIC, "$n dissapears in a flash!", ch, NULL, NULL, TO_ROOM );
			ch->retran = ch->in_room->vnum;
			random = get_room_index( ( number_range( 29, 43 ) ) );
			char_from_room( ch );
			char_to_room( ch, random );
			SET_BIT( ch->pcdata->flags, PCFLAG_WAR );
			snprintf( buf, MAX_STRING_LENGTH, "&B%s &Chas entered the Arena!", ch->name );
			war_channel( buf );
			act( AT_MAGIC, "$n appears in a flash!", ch, NULL, NULL, TO_ROOM );
			do_look( ch, "auto" );
			return;
		}
	}
	else
	{
		do_awho( ch, "" );
	}
}

void end_war( void )
{
	CHAR_DATA *wch;

	war_info.wartype = 0;
	war_info.min_level = 0;
	war_info.max_level = 0;
	war_info.iswar = WAR_OFF;
	war_info.inwar = 0;
	war_info.timer = 0;
	war_info.suddendeath = false;
	war_info.next = number_range( 30, 60 );
	for( wch = first_char; wch != NULL; wch = wch->next )
	{
		if( !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_WAR ) )
		{
			stop_fighting( wch, true );
			REMOVE_BIT( wch->pcdata->flags, PCFLAG_WAR );
			if( xIS_SET( wch->in_room->room_flags, ROOM_ARENA ) || wch->in_room->vnum == ROOM_VNUM_WAITINGROOM )
			{
				char_from_room( wch );
				char_to_room( wch, get_room_index( wch->retran ) );
			}
			wch->hit = wch->max_hit;
			wch->move = wch->max_move;
			update_pos( wch );
			do_look( wch, "auto" );
		}
	}
}

CMDF( do_arenawar )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location;
	int i = 0;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobiles not supported yet.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax:  war start <minlev> <maxlev> <#type>\r\n", ch );
		send_to_char( "         war status\r\n", ch );
		send_to_char( "         war info\r\n", ch );
		send_to_char( "         war join\r\n", ch );
		if( IS_IMMORTAL( ch ) )
		{
			send_to_char( "         war end\r\n" "         war next\r\n", ch );
		}
		return;
	}
	/*
		else if (!str_cmp(arg, "start"))
		{
		start_war(ch, argument);
		return;
		}
		else if (!str_cmp(arg, "talk"))
		{
		war_talk(ch, argument);
		return;
		}
	*/
	else if( !str_cmp( arg, "next" ) && IS_IMMORTAL( ch ) )
	{
		if( war_info.iswar == true )
		{
			send_to_char( "Not while a war is running.\r\n", ch );
			return;
		}

		i = is_number( argument ) ? atoi( argument ) : number_range( 30, 100 );
		war_info.next = i;
		snprintf( buf, MAX_STRING_LENGTH, "The next war will start in %d minutes.\r\n", war_info.next );
		send_to_char( buf, ch );
		return;
	}

	if( war_info.iswar != true )
	{
		snprintf( buf, MAX_STRING_LENGTH, "There is no war going! The next war will start in %d minutes.\r\n", war_info.next );
		send_to_char( buf, ch );
		return;
	}

	if( !str_cmp( arg, "end" ) && IS_IMMORTAL( ch ) )
	{
		end_war( );
		snprintf( buf, MAX_STRING_LENGTH, "You end the war. Next war in %d minutes.\r\n", war_info.next );
		send_to_char( buf, ch );
		snprintf( buf, MAX_STRING_LENGTH, "$n has ended the war. The next autwar will start in %d minutes.", war_info.next );
		war_channel( buf );
		snprintf( buf, MAX_STRING_LENGTH, "You have ended the war. The next autwar will start in %d minutes.\r\n", war_info.next );
		send_to_char( buf, ch );
		return;
	}
	else if( !str_cmp( arg, "info" ) )
	{
		send_to_char( buf, ch );
		snprintf( buf, MAX_STRING_LENGTH, "Fighting    : %d player%s.\r\n", war_info.inwar, war_info.inwar == 1 ? "" : "s" );
		send_to_char( buf, ch );
		snprintf( buf, MAX_STRING_LENGTH, "Levels      : %d - %d\r\n", war_info.min_level, war_info.max_level );
		send_to_char( buf, ch );
		snprintf( buf, MAX_STRING_LENGTH, "Status      : %s for %d minutes.\r\n",
			war_info.iswar == WAR_WAITING ? "Waiting" : "Running", war_info.timer );
		send_to_char( buf, ch );
		snprintf( buf, MAX_STRING_LENGTH, "Type        : %s war.\r\n", wartype_name( war_info.wartype ) );
		send_to_char( buf, ch );
		return;
	}
	else if( !str_cmp( arg, "check" ) )
	{
		CHAR_DATA *wch;
		bool found = false;

		for( wch = first_char; wch != NULL; wch = wch->next )
		{
			if( !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_WAR ) )
			{
				snprintf( buf, MAX_STRING_LENGTH, "%-12s\r\n", wch->name );
				send_to_char( buf, ch );
				found = true;
			}
		}
		if( !found )
			send_to_char( "No one in the war yet.\r\n", ch );
		return;
	}
	else if( !str_cmp( arg, "join" ) )
	{
		if( war_info.iswar == WAR_RUNNING )
		{
			send_to_char( "The war has allready started, your too late.\r\n", ch );
			return;
		}

		if( ch->top_level < war_info.min_level || ch->top_level > war_info.max_level )
		{
			send_to_char( "Sorry, you can't join this war.\r\n", ch );
			return;
		}

		if( IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
		{
			send_to_char( "You are already in the war.\r\n", ch );
			return;
		}

		if( war_info.wartype == 3 && !ch->pcdata->clan )
		{
			send_to_char( "You aren't in a gang, you can't join this war.\r\n", ch );
			return;
		}

		if( ( location = get_room_index( ROOM_VNUM_WAITINGROOM ) ) == NULL )
		{
			send_to_char( "Arena is not yet completed, sorry.\r\n", ch );
			return;
		}
		else
		{
			act( AT_PLAIN, "$n goes to get $s ass whipped in war!", ch, NULL, NULL, TO_ROOM );
			ch->retran = ch->in_room->vnum;
			char_from_room( ch );
			char_to_room( ch, location );
			SET_BIT( ch->pcdata->flags, PCFLAG_WAR );
			snprintf( buf, MAX_STRING_LENGTH, "%s (Level %d) joins the war!", ch->name, ch->top_level );
			war_channel( buf );
			act( AT_PLAIN, "$n arrives to get $s ass whipped!", ch, NULL, NULL, TO_ROOM );
			war_info.inwar++;
			do_look( ch, "auto" );
		}
		return;
	}
	do_war( ch, "" );
	return;
}

bool abort_race_war( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *vict;

	for( ch = first_char; ch != NULL; ch = ch->next )
	{
		if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
		{
			for( vict = first_char; vict != NULL; vict = vict->next )
			{
				if( !IS_NPC( vict ) && IS_SET( vict->pcdata->flags, PCFLAG_WAR ) )
				{
					if( ch->race == vict->race )
						continue;
					else
						return false;
				}
			}
		}
	}
	return true;
}

/*
bool abort_class_war(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *vict;

	for (ch = first_char; ch != NULL; ch = ch->next)
	{
	if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_WAR))
	{
		for (vict = first_char; vict != NULL; vict = vict->next)
		{
		if (!IS_NPC(vict) && IS_SET(vict->pcdata->flags, PCFLAG_WAR))
		{
			if (ch->class == vict->class)
			continue;
			else
			return false;
		}
		}
	}
	}
	return true;
}
*/

bool abort_clan_war( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *vict;

	for( ch = first_char; ch != NULL; ch = ch->next )
	{
		if( !IS_NPC( ch ) && IS_IN_WAR( ch ) && ch->pcdata->clan )
		{
			for( vict = first_char; vict != NULL; vict = vict->next )
			{
				if( !IS_NPC( vict ) && IS_IN_WAR( vict ) && vict->pcdata->clan )
				{
					if( ch->pcdata->clan->name == vict->pcdata->clan->name )
						continue;
					else
						return false;
				}
			}
		}
	}
	return true;
}

void war_update( void )
{
	char buf[MAX_STRING_LENGTH];
	/*
		if ( war_info.iswar == WAR_OFF && war_info.next > 0 )
		{
			if ( --war_info.next <= 0 )
				auto_war (  );
		}
	*/
	if( war_info.iswar == WAR_WAITING )
	{
		int randm = 0;

		war_info.timer--;

		if( war_info.timer > 0 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B%d &Cminute%s left to join the Levels &B%d &C- &B%d&C,&B %s&C War!",
				war_info.timer, war_info.timer == 1 ? "" : "s",
				war_info.min_level, war_info.max_level, wartype_name( war_info.wartype ) );
			war_channel( buf );
		}
		else
		{
			if( war_info.inwar < 2 )
			{
				end_war( );
				snprintf( buf, MAX_STRING_LENGTH, "&CNot enough people for war. Maybe next time!" );
				war_channel( buf );
			}
			else if( war_info.wartype == 1 && abort_race_war( ) )
			{
				end_war( );
				snprintf( buf, MAX_STRING_LENGTH, "&CNot enough races for war. Maybe next time!" );
				war_channel( buf );
			}
			/*
						else if ( war_info.wartype == 2 && abort_class_war (  ) )
						{
							end_war (  );
							sprintf ( buf, "Not enough classes for war.  Next autowar in %d minutes.",
									   war_info.next );
							war_channel(NULL, buf);
						}
			*/
			else if( war_info.wartype == 3 && abort_clan_war( ) )
			{
				end_war( );
				snprintf( buf, MAX_STRING_LENGTH, "&CNot enough gangs for war. Maybe next time!" );
				war_channel( buf );
			}
			else
			{
				CHAR_DATA *wch;

				snprintf( buf, MAX_STRING_LENGTH, "&CThe competition has begun! &B%d &Cplayers are fighting!", war_info.inwar );
				war_channel( buf );

				war_info.timer = number_range( 3 * war_info.inwar, 5 * war_info.inwar );
				war_info.iswar = WAR_RUNNING;
				for( wch = first_char; wch != NULL; wch = wch->next )
				{

					if( !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_WAR ) )
					{
						if( war_info.suddendeath == true )
							wch->hit = 5;

						randm = number_range( 29, 43 );
						char_from_room( wch );
						char_to_room( wch, get_room_index( randm ) );
						do_look( wch, "auto" );
					}
				}
			}
		}
	}

	else if( war_info.iswar == WAR_RUNNING )
	{
		if( war_info.inwar == 0 )
		{
			end_war( );
			snprintf( buf, MAX_STRING_LENGTH, "&CNo one left in the War." );
			war_channel( buf );
			return;
		}

		switch( war_info.timer )
		{
		case 0:
			end_war( );
			snprintf( buf, MAX_STRING_LENGTH, "Times up, you all are too slow!." );
			war_channel( buf );
			return;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 10:
		case 15:
			snprintf( buf, MAX_STRING_LENGTH, "&B%d &Cminute%s remaining in the war.", war_info.timer, war_info.timer > 1 ? "s" : "" );
			war_channel( buf );
		default:
			war_info.timer--;
			break;
		}
		return;
	}
}

void check_war( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *wch;
	char buf[MAX_STRING_LENGTH];
	int reward = number_range( 5000, 10000 );

	if( IS_NPC( ch ) || IS_NPC( victim ) )
		return;

	REMOVE_BIT( victim->pcdata->flags, PCFLAG_WAR );
	war_info.inwar--;
	stop_fighting( victim, true );
	char_from_room( victim );
	char_to_room( victim, get_room_index( victim->retran ) );
	victim->hit = victim->max_hit;
	victim->move = victim->max_move;
	victim->pcdata->apdeaths += 1;
	ch->pcdata->apkills += 1;
	update_pos( victim );
	do_look( victim, "auto" );
	send_to_char( "\r\n", ch );
	send_to_char( "\r\n", victim );
	snprintf( buf, MAX_STRING_LENGTH, "&B%s &Cwas knocked out of the arena by &B%s&C!", victim->name, ch->name );
	war_channel( buf );
	switch( war_info.wartype )
	{
	case 1:
		if( abort_race_war( ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&CThe &B%s's&C have won the Competition!", capitalize( get_race( ch ) ) );
			war_channel( buf );
			for( wch = first_char; wch != NULL; wch = wch->next )
			{
				if( !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_WAR ) )
					continue;

				if( wch->race == ch->race )
				{
					wch->gold += reward;
					snprintf( buf, MAX_STRING_LENGTH, "&zYou recieve &Y%d &zdollars for emerging victorious!\r\n", reward );
					send_to_char( buf, wch );
				}

			}
			end_war( );
			return;
		}  // end abort
		break;
	case 3:
		if( abort_clan_war( ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B%s &Chas won the Competition!", ch->pcdata->clan->name );
			war_channel( buf );
			/*
					for (wch = first_char; wch != NULL; wch = wch->next)
					{
					if (!IS_NPC(wch) && IS_IN_WAR(wch) )
						continue;
					if (ch->pcdata->clan->name == wch->pcdata->clan->name )
					{
						wch->gold += reward;
						sprintf(buf,
							 "&zYou recieve &Y%d&z dollars for emerging victorious!\r\n",
							 reward);
						send_to_char(buf, wch);
					}
					}
			*/
			end_war( );
			return;
		}
		break;
	case 2:
		if( war_info.inwar == 1 )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B%s &Chas won the Competition!", ch->name );
			war_channel( buf );
			ch->gold += reward;
			snprintf( buf, MAX_STRING_LENGTH, "&zYou recieve &Y%d&z dollars for emerging victorious!\r\n", reward );
			send_to_char( buf, ch );
			end_war( );
			return;
		}
		break;
	}
	return;
}

bool is_safe_war( CHAR_DATA *ch, CHAR_DATA *wch )
{
	if( war_info.iswar == WAR_OFF )
		return false;

	if( !IS_IN_WAR( ch ) || !IS_IN_WAR( wch ) )
		return false;

	if( war_info.wartype == 2 )
		return false;

	if( war_info.wartype == 1 && ch->race == wch->race )
		return true;

	if( war_info.wartype == 3 && ch->pcdata->clan == wch->pcdata->clan )
		return true;

	return false;
}

/*
void war_talk(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0')
	{
	send_to_char
	  ("Wartalk what?\r\n",
	   ch);
	return;
	}
	sprintf(buf, "(WarTalk) You drum: %s\r\n", argument);
	send_to_char(buf, ch);

	for (d = first_descriptor; d != NULL; d = d->next)
	{
	CHAR_DATA *victim;

	if (d->connected == CON_PLAYING && (victim = d->character) != ch &&
		!IS_SET(victim->comm, COMM_QUIET) && IS_IN_WAR(victim))
	{
		sprintf(buf, "(WarTalk) %s drums: %s\r\n",
			 PERS(ch, victim), argument);
		send_to_char(buf, victim);
	}
	}
	return;
}
*/
void extract_war( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];

	if( war_info.iswar != WAR_OFF && IS_SET( ch->pcdata->flags, PCFLAG_WAR ) )
	{
		REMOVE_BIT( ch->pcdata->flags, PCFLAG_WAR );
		war_info.inwar--;
		if( war_info.iswar == WAR_RUNNING )
		{
			if( war_info.inwar == 0 || war_info.inwar == 1 )
			{
				snprintf( buf, MAX_STRING_LENGTH, "&B%s&C has left. War over.", ch->name );
				war_channel( buf );
				end_war( );
			}
			if( abort_race_war( ) )
			{
				snprintf( buf, MAX_STRING_LENGTH, "&B%s&C has left. War over.", ch->name );
				war_channel( buf );
				end_war( );
			}
			/*
						else if ( abort_class_war (  ) )
						{
							war_channel ( ch, "$n has left. War over." );
							end_war (  );
						}
			*/
			else if( abort_clan_war( ) )
			{
				snprintf( buf, MAX_STRING_LENGTH, "&B%s&C has left. War over.", ch->name );
				war_channel( buf );
				end_war( );
			}
			else
			{
				snprintf( buf, MAX_STRING_LENGTH, "&B$n &Chas left. &B%d&C players in the war.", war_info.inwar );
				war_channel( buf );
			}
		}
		char_from_room( ch );
		char_to_room( ch, get_room_index( ch->retran ) );
	}
}

void war_channel( const char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *original;
	char buf[MAX_INPUT_LENGTH];

	snprintf( buf, MAX_STRING_LENGTH, "&P&p-&P=&RA&rr&Re&rn&Ra&P=&p- %s", argument );

	for( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */
		if( ( d->connected == CON_PLAYING ) && !IS_SET( original->deaf, CHANNEL_ARENA )
			&& !xIS_SET( original->in_room->room_flags, ROOM_SILENCE ) && !NOT_AUTHED( original ) )
			act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}
