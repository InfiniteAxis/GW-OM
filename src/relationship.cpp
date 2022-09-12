/*--------------------------------------------------------------------------
			  .88b  d88. db    db d8888b.   .d888b. db   dD
			  88'YbdP`88 88    88 88  `8D   VP  `8D 88 ,8P'
			  88  88  88 88    88 88   88      odD' 88,8P
			  88  88  88 88    88 88   88    .88'   88`8b
			  88  88  88 88b  d88 88  .8D   j88.    88 `88.
			  YP  YP  YP ~Y8888P' Y8888D'   888888D YP   YD
This material is copyrighted (c) Thomas J Whiting (twhiting6@home.com).
Usage of this material  means that you have read and agree to all of the
licenses in the ../licenses directory. None of these licenses may ever be
removed. In addition, these headers may never be removed.
----------------------------------------------------------------------------
A LOT of time has gone into this code by a LOT of people. Not just on
this individual code, but on all of the codebases this even takes a piece
of. I hope that you find this code in some way useful and you decide to
contribute a small bit to it. There's still a lot of work yet to do.
---------------------------------------------------------------------------*/
/*
	mud relationships.. Ahh, the only thing for heros to do is
	play house. Right?? Well, not necessarily so. They should
	have more to do than this, but this is in for 'em now.
*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "mud.h"
//#include "interp.h"
//#include "recycle.h"  
//#include "tables.h"
#include "relationship.h"
CMDF( do_askout )
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MSL];
	//    char buf2[MSL];
	CHAR_DATA *victim;
	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Who would you like to ask out on a date?\r\n", ch );
		return;
	}
	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They're not here.\r\n", ch );
		return;
	}
	if( IS_NPC( victim ) )
	{
		stc( "You'd get declined right away, don't try!\r\n", ch );
		return;
	}
	if( victim->pcdata->spouse != NULL )
	{
		send_to_char( "They're already involved with someone!\r\n", ch );
		return;
	}
	if( ch->pcdata->spouse != NULL )
	{
		sprintf( buf, "I don't think that %s would like that very much\r\n", ch->pcdata->spouse );
		stc( buf, ch );
		return;
	}
	else
	{
		act( AT_WHITE, "You ask $M out on a date.", ch, NULL, victim, TO_CHAR );
		act( AT_WHITE, "$n asks you on a date..\n\rTo accept, type accept $n, to decline, or type decline $n, to decline.", ch,
			NULL, victim, TO_VICT );
		act( AT_WHITE, "$n asks $N out on a date.", ch, NULL, victim, TO_NOTVICT );

		ch->pcdata->tspouse = victim->name;
		victim->pcdata->tspouse = ch->name;
		ch->pcdata->predating = true;
		victim->pcdata->predating = true;

		return;
	}
}

CMDF( do_propose )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int dtime;
	//    char buf[MSL];
	dtime = ch->pcdata->rtime / 7;
	argument = one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Who would you like to propose to?\r\n", ch );
		return;
	}
	if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That person isn't in the room.\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		stc( "Proposing to a mob? You sad.. sad person..\r\n", ch );
		return;
	}

	if( !ch->pcdata->dating )
	{
		stc( "Try actually going out on a date with someone first dumbass\r\n", ch );
		return;
	}

	if( ( victim->pcdata->tspouse != ch->pcdata->tspouse ) )

	{

		stc( "That's not your spouse.. OOOOH, wait till I tell on you!!\r\n", ch );
		return;
	}

	if( !victim->pcdata->dating )
	{
		stc( "Hmmm, They're not dating anyone...\r\n", ch );
		return;
	}
	else if( dtime < 20 )
	{
		stc( "Why not get to know the person a bit first\r\n", ch );
		stc( "You MUST be dating 20 weeks before you can propose\r\n", ch );
		return;
	}
	else if( victim->desc == NULL )
	{
		act( AT_WHITE, "$N seems to have misplaced $S link...try again later.", ch, NULL, victim, TO_CHAR );
		return;
	}

	else
	{
		ch->pcdata->tspouse = victim->name;
		victim->pcdata->tspouse = ch->name;
		do_save( ch, "" );
		do_save( victim, "" );
		ch->pcdata->preengaged = true;
		victim->pcdata->preengaged = true;
		act( AT_WHITE, "You ask $M to marry you", ch, NULL, victim, TO_CHAR );
		act( AT_WHITE,
			"$n gets on $m knees before you, a ring in $m hand and asks for your hand in marriage .\n\rTo accept, type accept $n, or type decline $n to decline.",
			ch, NULL, victim, TO_VICT );
		act( AT_WHITE, "$n gets down one $m knees before $N and asks $M hand in marriage.", ch, NULL, victim, TO_NOTVICT );
	}
}
CMDF( do_marry )
{
	char arg1[MIL], arg2[MIL];
	char buf[MSL];
	CHAR_DATA *spouse1;
	CHAR_DATA *spouse2;
	int dtime = 0;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	/*
	 Before we can do the deed, there's a few recursive checks we gotta go
	 through.. Okay, QUITE a few, but at least this way we KNOW that these
	 two actually want to get married.. Shouldn't be at this point without
	 knowing that they want to get married, but, still.. This is the last
	 and final point for bailout.. So, give them the benefit of the doubt
	 */

	if( !IS_IMMORTAL( ch ) )
	{
		stc( "You've not got the marrying powers\r\n", ch );
		return;
	}
	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		stc( "Syntax: Marry <person1> <person2>\r\n", ch );
		return;
	}

	if( ( spouse1 = get_char_room( ch, arg1 ) ) == NULL )
	{
		sprintf( buf, "But %s isn't in the room!!\r\n", arg1 );
		stc( buf, ch );
		return;
	}
	if( ( spouse2 = get_char_room( ch, arg2 ) ) == NULL )
	{
		sprintf( buf, "But %s isn't in the room!!\r\n", arg2 );
		stc( buf, ch );
		return;
	}

	if( IS_NPC( spouse1 ) || IS_NPC( spouse2 ) )
	{
		stc( "Mobs can't get married!\r\n", ch );
		return;
	}

	if( ( ch == spouse1 ) || ( ch == spouse2 ) )
	{
		stc( "Err, you can't marry yourself.\r\n", ch );
		return;
	}

	dtime = spouse1->pcdata->rtime / 7;
	if( dtime < 50 )
	{
		stc( "They MUST be dating 50 weeks before they can get married\r\n", ch );
		return;
	}
	dtime = spouse2->pcdata->rtime / 7;
	if( dtime < 50 )
	{
		stc( "They MUST be dating 50 weeks before they can get married\r\n", ch );
		return;
	}

	if( !ISDATING( spouse1, spouse2 ) )
	{
		stc( "They must actually be dating before they can be married\r\n", ch );
		return;
	}
	if( !ISDATING( spouse2, spouse1 ) )
	{
		stc( "They must actually be dating before they can be married\r\n", ch );
		return;
	}
	if( !spouse1->pcdata->engaged )
	{
		sprintf( buf, "%s isn't engaged, though", spouse1->name );
		stc( buf, ch );
		return;
	}
	if( !spouse2->pcdata->engaged )
	{
		sprintf( buf, "%s isn't engaged, though", spouse2->name );
		stc( buf, ch );
		return;
	}
	/*
	Okay, we made it through all the nasties, now time to do the deed..
	Why not make this a global announcement when it's done as well, ehh?
	*/
	else
	{
		stc( "You pronounce them Man and Wife ??\r\n", ch );
		spouse1->pcdata->married = true;
		spouse1->pcdata->engaged = false;
		spouse2->pcdata->married = true;
		spouse2->pcdata->engaged = false;


		send_to_char( "You are now officially married. Congratulations\r\n", spouse1 );
		send_to_char( "You are now officially married. Congratulations\r\n", spouse2 );
		sprintf( buf, "%s and %s{have finally been married. Everyone congratulate them.\r\n", spouse1->name, spouse2->name );
		do_echo( ch, buf );
		do_save( spouse1, "" );
		do_save( spouse2, "" );
		return;

	}



}
CMDF( do_divorce )
{
	char arg1[MIL], arg2[MIL];
	char buf[MSL];
	CHAR_DATA *spouse1;
	CHAR_DATA *spouse2;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	/*
	 Before we can do the deed, there's a few recursive checks we gotta go
	 through.. Okay, QUITE a few, but at least this way we KNOW that these
	 two actually want to get divorced.. Shouldn't be at this point without
	 knowing that they want to get married, but, still.. This is the last
	 and final point for bailout.. So, give them the benefit of the doubt
	 */

	if( !IS_IMMORTAL( ch ) )
	{
		stc( "You've not got the divorcing powers\r\n", ch );
		return;
	}
	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		stc( "Syntax: Divorce <person1> <person2>\r\n", ch );
		return;
	}

	if( ( spouse1 = get_char_room( ch, arg1 ) ) == NULL )
	{
		sprintf( buf, "But %s isn't in the room!!\r\n", arg1 );
		stc( buf, ch );
		return;
	}
	if( ( spouse2 = get_char_room( ch, arg2 ) ) == NULL )
	{
		sprintf( buf, "But %s isn't in the room!!\r\n", arg2 );
		stc( buf, ch );
		return;
	}

	if( IS_NPC( spouse1 ) || IS_NPC( spouse2 ) )
	{
		stc( "Mobs can't get married!\r\n", ch );
		return;
	}

	if( ( ch == spouse1 ) || ( ch == spouse2 ) )
	{
		stc( "Err, you can't marry yourself.\r\n", ch );
		return;
	}

	if( !ISDATING( spouse1, spouse2 ) )
	{
		stc( "They must actually be married before they can be divorced dumbass.\r\n", ch );
		return;
	}
	if( !ISDATING( spouse2, spouse1 ) )
	{
		stc( "They must actually be dating before they can be divorced dumbass\r\n", ch );
		return;
	}
	if( !spouse1->pcdata->married )
	{
		sprintf( buf, "%s isn't married, though", spouse1->name );
		stc( buf, ch );
		return;
	}
	if( !spouse2->pcdata->married )
	{
		sprintf( buf, "%s isn't married, though", spouse2->name );
		stc( buf, ch );
		return;
	}
	/*
	Okay, we made it through all the nasties, now time to do the deed..
	Why not make this a global announcement when it's done as well, ehh?
	*/
	else
	{
		stc( "You pronounce them divorced ??\r\n", ch );
		spouse1->pcdata->married = false;
		spouse1->pcdata->engaged = false;
		spouse2->pcdata->married = false;
		spouse2->pcdata->engaged = false;
		spouse2->pcdata->spouse = NULL;
		spouse1->pcdata->spouse = NULL;
		spouse1->pcdata->rtime = 0;
		spouse2->pcdata->rtime = 0;
		send_to_char( "You are now officially divorced.\r\n", spouse1 );
		send_to_char( "You are now officially divorced.\r\n", spouse2 );
		do_save( spouse1, "" );
		do_save( spouse2, "" );
		return;

	}



}



CMDF( do_accept )
{

	CHAR_DATA *victim;
	char buf[MSL];
	//char buf2[MSL];

	if( IS_NPC( ch ) )
		return;
	if( argument[0] == '\0' )
	{
		send_to_char( "Accept to who??\r\n", ch );
		return;
	}
	if( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char( "But they aren't here!!\r\n", ch );
		return;
	}
	if( !ISTDATING( ch, victim ) )
	{
		sprintf( buf, "%s hasn't asked for your acceptance to do anything!\r\n", victim->name );
		stc( buf, ch );
		return;
	}

	else
	{

		if( ch->pcdata->predating )
		{
			sprintf( buf, "You accept %s's invitation\r\n", ch->pcdata->tspouse );
			stc( buf, ch );
			sprintf( buf, "%s has accepted your invitation\r\n", ch->name );
			stc( buf, victim );
			victim->pcdata->spouse = ch->name;
			ch->pcdata->spouse = victim->name;
			victim->pcdata->tspouse = NULL;
			ch->pcdata->tspouse = NULL;
			victim->pcdata->dating = true;
			ch->pcdata->dating = true;
			victim->pcdata->predating = false;
			ch->pcdata->predating = false;
			do_save( ch, "" );
			do_save( victim, "" );
			return;
		}


		if( ch->pcdata->preengaged )
		{
			sprintf( buf, "You accept %s's proposal\r\n", ch->pcdata->tspouse );
			stc( buf, ch );
			sprintf( buf, "%s has accepted your proposal\r\n", ch->name );
			stc( buf, victim );
			victim->pcdata->dating = false;
			ch->pcdata->dating = false;
			victim->pcdata->predating = false;
			ch->pcdata->predating = false;
			ch->pcdata->preengaged = false;
			victim->pcdata->preengaged = false;
			ch->pcdata->engaged = true;
			victim->pcdata->engaged = true;
			do_save( ch, "" );
			do_save( victim, "" );
			return;
		}
	}
}
CMDF( do_decline )
{

	CHAR_DATA *victim;
	char buf[MSL];

	if( IS_NPC( ch ) )
		return;
	if( argument[0] == '\0' )
	{
		send_to_char( "Decline who??\r\n", ch );
		return;
	}
	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't logged in!\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		stc( "You'd get declined right away, don't try!\r\n", ch );
		return;
	}

	if( !ISTDATING( ch, victim ) )
	{
		sprintf( buf, "%s hasn't given you anything to decline.\r\n", buf );
		stc( buf, ch );
		return;
	}


	if( ch->pcdata->predating )
	{
		sprintf( buf, "You decline %s's invitation\r\n", ch->pcdata->tspouse );
		stc( buf, ch );
		sprintf( buf, "%s has declined your invitation\r\n", ch->name );
		stc( buf, victim );
		victim->pcdata->tspouse = NULL;
		ch->pcdata->tspouse = NULL;
		victim->pcdata->spouse = NULL;
		ch->pcdata->spouse = NULL;
		victim->pcdata->dating = false;
		ch->pcdata->dating = false;
		victim->pcdata->predating = false;
		ch->pcdata->predating = false;
		do_save( ch, NULL );
		do_save( victim, NULL );
		return;
	}

	if( ch->pcdata->preengaged )
	{
		sprintf( buf, "You decline %s's proposal.\r\n", ch->pcdata->tspouse );
		stc( buf, ch );
		sprintf( buf, "%s has declined your proposal.\r\n", ch->name );
		stc( buf, victim );
		//they were dating before, correct?? Just because we don't wanna get married don't mean we don't wanna keep dating 'em.

		ch->pcdata->dating = true;
		victim->pcdata->dating = true;
		victim->pcdata->spouse = ch->name;
		ch->pcdata->spouse = victim->name;
		victim->pcdata->tspouse = NULL;
		ch->pcdata->tspouse = NULL;
		victim->pcdata->engaged = false;
		ch->pcdata->engaged = false;
		victim->pcdata->preengaged = false;
		ch->pcdata->preengaged = false;
		do_save( ch, NULL );
		do_save( victim, NULL );
		return;
	}
}



void spouse_update( void )
{
	DESCRIPTOR_DATA *d;
	DESCRIPTOR_DATA *b;
	CHAR_DATA *ch;
	CHAR_DATA *victim;

	for( d = first_descriptor; d != NULL; d = d->next )
	{
		if( d->character != NULL && d->connected == CON_PLAYING )
		{
			ch = d->character; //this is me
			for( b = first_descriptor; b != NULL; b = b->next )
			{

				victim = b->original ? b->original : b->character;  //this is my spouse

				if( b->connected == CON_PLAYING && victim != ch && victim->pcdata->spouse == ch->name )

					//are we online together?? If so, update the rtime
				{
					ch->pcdata->rtime++;
					victim->pcdata->rtime++;
				}
			}
		}
	}
	return;
}

CMDF( do_breakup )
{
	CHAR_DATA *victim;
	char buf[MSL];

	if( IS_NPC( ch ) )
		return;
	if( argument[0] == '\0' )
	{
		send_to_char( "Breakup with who??\r\n", ch );
		return;
	}
	if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't logged in!\r\n", ch );
		return;
	}

	if( ch == victim )
	{
		send_to_char( "Ahaha.. Yeah...\r\n", ch );
		return;
	}

	if( IS_NPC( victim ) )
	{
		stc( "You aren't going out with them..\r\n", ch );
		return;
	}

	if( !ISDATING( ch, victim ) )
	{
		stc( "You're not in a relationship with them.\r\n", ch );
		return;
	}

	if( ch->pcdata->married )
	{
		stc( "Nope, gotta get an official divorce.. Sorry\r\n", ch );
		return;
	}

	if( ch->pcdata->dating )
	{
		sprintf( buf, "You break up with %s\r\n", ch->pcdata->spouse );
		stc( buf, ch );
		sprintf( buf, "%s has broken up with you\r\n", ch->name );
		stc( buf, victim );
		victim->pcdata->tspouse = NULL;
		ch->pcdata->tspouse = NULL;
		victim->pcdata->spouse = NULL;
		ch->pcdata->spouse = NULL;
		victim->pcdata->dating = false;
		ch->pcdata->dating = false;
		victim->pcdata->predating = false;
		ch->pcdata->predating = false;
		victim->pcdata->rtime = 0;
		ch->pcdata->rtime = 0;
		return;
	}

	if( ch->pcdata->engaged )
	{
		sprintf( buf, "You break up with %s\r\n", ch->pcdata->spouse );
		stc( buf, ch );
		sprintf( buf, "%s has broken up with you\r\n", ch->name );
		stc( buf, victim );
		victim->pcdata->tspouse = NULL;
		ch->pcdata->tspouse = NULL;
		victim->pcdata->spouse = NULL;
		ch->pcdata->spouse = NULL;
		victim->pcdata->engaged = false;
		ch->pcdata->engaged = false;
		victim->pcdata->preengaged = false;
		ch->pcdata->preengaged = false;
		victim->pcdata->rtime = 0;
		ch->pcdata->rtime = 0;
		return;
	}

}



CMDF( do_clearwed )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Who gets the lucky priviledge ", ch );
		return;
	}

	if( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}
	if( get_trust( victim ) > get_trust( ch ) )
	{
		send_to_char( "You failed.\r\n", ch );
		return;
	}
	victim->pcdata->spouse = NULL;
	victim->pcdata->rtime = 0;
	victim->pcdata->dating = false;
	victim->pcdata->engaged = false;
	victim->pcdata->married = false;
	victim->pcdata->predating = false;
	victim->pcdata->preengaged = false;
	victim->pcdata->premarried = false;

	stc( "make it so\r\n", ch );
	stc( "You're entire dating status has been cleared.\r\n", victim );
	return;
}
