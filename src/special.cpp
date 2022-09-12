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
#include <dlfcn.h>
#include "mud.h"

/* jails for wanted flags */

#define ROOM_JAIL_CORUSCANT        0

bool remove_obj( CHAR_DATA * ch, int iWear, bool fReplace );

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN( spec_jedi );
DECLARE_SPEC_FUN( spec_dark_jedi );
DECLARE_SPEC_FUN( spec_gambler );
DECLARE_SPEC_FUN( spec_fido );
DECLARE_SPEC_FUN( spec_guardian );
DECLARE_SPEC_FUN( spec_janitor );
DECLARE_SPEC_FUN( spec_poison );
DECLARE_SPEC_FUN( spec_thief );
DECLARE_SPEC_FUN( spec_auth );
DECLARE_SPEC_FUN( spec_stormtrooper );
DECLARE_SPEC_FUN( spec_new_republic_trooper );
DECLARE_SPEC_FUN( spec_customs_smut );
DECLARE_SPEC_FUN( spec_customs_alcohol );
DECLARE_SPEC_FUN( spec_customs_weapons );
DECLARE_SPEC_FUN( spec_customs_spice );
DECLARE_SPEC_FUN( spec_police_attack );
DECLARE_SPEC_FUN( spec_police_jail );
DECLARE_SPEC_FUN( spec_police_fine );
DECLARE_SPEC_FUN( spec_police );
DECLARE_SPEC_FUN( spec_clan_guard );
DECLARE_SPEC_FUN( spec_newbie_pilot );
DECLARE_SPEC_FUN( spec_questmaster );
DECLARE_SPEC_FUN( spec_infected );
DECLARE_SPEC_FUN( spec_claude );
DECLARE_SPEC_FUN( spec_homunculus );

SPEC_LIST *first_specfun;
SPEC_LIST *last_specfun;

/* Simple load function - no OLC support for now.
 * This is probably something you DONT want builders playing with.
 */
void load_specfuns( void )
{
    SPEC_LIST *specfun;
    FILE *fp;
    char filename[256];
    const char *word;

    first_specfun = NULL;
    last_specfun = NULL;

    snprintf( filename, 256, "%sspecfuns.dat", SYSTEM_DIR );
    if( !( fp = FileOpen( filename, "r" ) ) )
    {
        bug( "%s: FATAL - cannot load specfuns.dat, exiting.", __func__ );
        perror( filename );
        exit( 1 );
    }
    else
    {
        for( ; ; )
        {
            if( feof( fp ) )
            {
                bug( "%s: Premature end of file!", __func__ );
                FileClose( fp );
                return;
            }
            word = fread_word( fp );
            if( !str_cmp( word, "$" ) )
                break;

            CREATE( specfun, SPEC_LIST, 1 );
            specfun->name = str_dup( word );
            LINK( specfun, first_specfun, last_specfun, next, prev );
        }
        FileClose( fp );
    }
}


/* Simple validation function to be sure a function can be used on mobs */
bool validate_spec_fun( const char *name )
{
    SPEC_LIST *specfun;

    for( specfun = first_specfun; specfun; specfun = specfun->next )
    {
        if( !str_cmp( specfun->name, name ) )
            return true;
    }
    return false;
}

/*
 * Given a name, return the appropriate spec_fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
    void *funHandle;
    const char *error;

    funHandle = dlsym( sysdata.dlHandle, name );
    if( ( error = dlerror( ) ) != NULL )
    {
        bug( "%s: Error locating function %s in symbol table.", __func__, name );
        return NULL;
    }
    return ( SPEC_FUN * ) funHandle;
}


SPECF( spec_questmaster )
{
     if( !IS_NPC( ch ) )
          return false;
     else
          return true;
}

SPECF( spec_newbie_pilot )
{
   int home = 32149;
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   bool diploma = false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || victim->position == POS_FIGHTING )
         continue;

      for( obj = victim->last_carrying; obj; obj = obj->prev_content )
         if( obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_DIPLOMA )
            diploma = true;

      if( !diploma )
         continue;

      switch ( victim->race )
      {
         case RACE_COLONIST:
            home = 201;
            strcpy( buf, "After a brief journey you arrive at Coruscants Menari Spaceport.\r\n\r\n" );
            echo_to_room( AT_ACTION, ch->in_room, buf );
            break;

         default:
            sprintf( buf, "Hmm, a %s.", race_table[victim->race].race_name );
            do_look( ch, victim->name );
            do_say( ch, buf );
            do_say( ch, "You're home planet is a little hard to get to right now." );
            do_say( ch, "I'll take you to the Pluogus instead." );
            echo_to_room( AT_ACTION, ch->in_room,
                          "After a brief journey the shuttle docks with the Serin Pluogus.\r\n\r\n" );
            break;
      }

      char_from_room( victim );
      char_to_room( victim, get_room_index( home ) );

      do_look( victim, "" );

      sprintf( buf, "%s steps out and the shuttle quickly returns to the academy.\r\n", victim->name );
      echo_to_room( AT_ACTION, ch->in_room, buf );
   }

   return false;
}

SPECF( spec_jedi )
{
   return false;
}

SPECF( spec_gambler ) /*You can add here special characteristics */
{
   if( !IS_NPC( ch ) )
      return false;
   else
      return true;
}

SPECF( spec_clan_guard )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( !can_see( ch, victim ) )
         continue;
      if( get_timer( victim, TIMER_RECENTFIGHT ) > 0 )
         continue;
      if( !IS_NPC( victim ) && victim->pcdata && victim->pcdata->clan && IS_AWAKE( victim )
          && str_cmp( ch->name, victim->pcdata->clan->name ) )
      {
         do_yell( ch, "Hey your not allowed in here!" );
         multi_hit( ch, victim, TYPE_UNDEFINED );
         return true;
      }
   }

   return false;
}

SPECF( spec_customs_smut )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   long ch_exp;

   if( !IS_AWAKE( ch ) || ch->position == POS_FIGHTING )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || victim->position == POS_FIGHTING )
         continue;

      for( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->pIndexData->item_type == ITEM_SMUT )
         {
            if( victim != ch && can_see( ch, victim ) && can_see_obj( ch, obj ) )
            {
               sprintf( buf, "%s is illegal contraband. I'm going to have to confiscate that.", obj->short_descr );
               do_say( ch, buf );
               if( obj->wear_loc != WEAR_NONE )
                  remove_obj( victim, obj->wear_loc, true );
               separate_obj( obj );
               obj_from_char( obj );
               act( AT_ACTION, "$n confiscates $p from $N.", ch, obj, victim, TO_NOTVICT );
               act( AT_ACTION, "$n takes $p from you.", ch, obj, victim, TO_VICT );
               obj = obj_to_char( obj, ch );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You lose %ld experience.\r\n ", ch_exp );
               gain_exp( victim, 0 - ch_exp, SMUGGLING_ABILITY );
               return true;
            }
            else if( can_see( ch, victim ) && !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 30,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );

               act( AT_ACTION, "$n looks at $N suspiciously.", ch, NULL, victim, TO_NOTVICT );
               act( AT_ACTION, "$n look at you suspiciously.", ch, NULL, victim, TO_VICT );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );

               return true;
            }
            else if( !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 30,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );

               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               return true;
            }
         }
         else if( obj->item_type == ITEM_CONTAINER )
         {
            OBJ_DATA *content;
            for( content = obj->first_content; content; content = content->next_content )
            {
               if( content->pIndexData->item_type == ITEM_SMUT && !xIS_SET( content->extra_flags, ITEM_CONTRABAND ) )
               {
                  ch_exp =
                     UMIN( content->cost * 30,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, content->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  xSET_BIT( content->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
            }
         }
      }
   }
   return false;
}

SPECF( spec_customs_weapons )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   long ch_exp;

   if( !IS_AWAKE( ch ) || ch->position == POS_FIGHTING )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || victim->position == POS_FIGHTING )
         continue;

      if( victim->pcdata && victim->pcdata->clan && !str_cmp( victim->pcdata->clan->name, ch->mob_clan ) )
         continue;

      for( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->pIndexData->item_type == ITEM_WEAPON && !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            if( victim != ch && can_see( ch, victim ) && can_see_obj( ch, obj ) )
            {
               sprintf( buf, "Weapons are banned from non-military usage. I'm going to have to confiscate %s.",
                        obj->short_descr );
               do_say( ch, buf );
               if( obj->wear_loc != WEAR_NONE )
                  remove_obj( victim, obj->wear_loc, true );
               separate_obj( obj );
               obj_from_char( obj );
               act( AT_ACTION, "$n confiscates $p from $N.", ch, obj, victim, TO_NOTVICT );
               act( AT_ACTION, "$n takes $p from you.", ch, obj, victim, TO_VICT );
               obj = obj_to_char( obj, ch );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You lose %ld experience.\r\n ", ch_exp );
               gain_exp( victim, 0 - ch_exp, SMUGGLING_ABILITY );
               return true;
            }
            else if( can_see( ch, victim ) && !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %d.\r\n ", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
               separate_obj( obj );
               act( AT_ACTION, "$n looks at $N suspiciously.", ch, NULL, victim, TO_NOTVICT );
               act( AT_ACTION, "$n look at you suspiciously.", ch, NULL, victim, TO_VICT );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               return true;
            }
            else if( !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
               separate_obj( obj );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               return true;
            }
         }
         else if( obj->item_type == ITEM_CONTAINER )
         {
            OBJ_DATA *content;
            for( content = obj->first_content; content; content = content->next_content )
            {
               if( content->pIndexData->item_type == ITEM_WEAPON && !xIS_SET( content->extra_flags, ITEM_CONTRABAND ) )
               {
                  ch_exp =
                     UMIN( content->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, content->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  separate_obj( content );
                  xSET_BIT( content->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
            }
         }
      }
   }
   return false;
}

SPECF( spec_customs_alcohol )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   int liquid;
   long ch_exp;

   if( !IS_AWAKE( ch ) || ch->position == POS_FIGHTING )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || victim->position == POS_FIGHTING )
         continue;

      for( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->pIndexData->item_type == ITEM_DRINK_CON )
         {
            if( ( liquid = obj->value[2] ) >= LIQ_MAX )
               liquid = obj->value[2] = 0;

            if( liq_table[liquid].liq_affect[COND_DRUNK] > 0 )
            {
               if( victim != ch && can_see( ch, victim ) && can_see_obj( ch, obj ) )
               {
                  sprintf( buf, "%s is illegal contraband. I'm going to have to confiscate that.", obj->short_descr );
                  do_say( ch, buf );
                  if( obj->wear_loc != WEAR_NONE )
                     remove_obj( victim, obj->wear_loc, true );
                  separate_obj( obj );
                  obj_from_char( obj );
                  act( AT_ACTION, "$n confiscates $p from $N.", ch, obj, victim, TO_NOTVICT );
                  act( AT_ACTION, "$n takes $p from you.", ch, obj, victim, TO_VICT );
                  obj = obj_to_char( obj, ch );
                  xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
                  ch_exp =
                     UMIN( obj->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You lose %ld experience. \r\n", ch_exp );
                  gain_exp( victim, 0 - ch_exp, SMUGGLING_ABILITY );
                  return true;
               }
               else if( can_see( ch, victim ) && !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
               {
                  ch_exp =
                     UMIN( obj->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %d. \r\n", ch_exp, obj->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  separate_obj( obj );
                  act( AT_ACTION, "$n looks at $N suspiciously.", ch, NULL, victim, TO_NOTVICT );
                  act( AT_ACTION, "$n look at you suspiciously.", ch, NULL, victim, TO_VICT );
                  xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
               else if( !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
               {
                  ch_exp =
                     UMIN( obj->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %d. \r\n", ch_exp, obj->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  separate_obj( obj );
                  xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
            }
         }
         else if( obj->item_type == ITEM_CONTAINER )
         {
            OBJ_DATA *content;
            for( content = obj->first_content; content; content = content->next_content )
            {
               if( content->pIndexData->item_type == ITEM_DRINK_CON && !xIS_SET( content->extra_flags, ITEM_CONTRABAND ) )
               {
                  if( ( liquid = obj->value[2] ) >= LIQ_MAX )
                     liquid = obj->value[2] = 0;
                  if( liq_table[liquid].liq_affect[COND_DRUNK] <= 0 )
                     continue;
                  ch_exp =
                     UMIN( content->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %d.\r\n ", ch_exp, content->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  separate_obj( content );
                  xSET_BIT( content->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
            }
         }
      }
   }
   return false;
}

SPECF( spec_customs_spice )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   long ch_exp;

   if( !IS_AWAKE( ch ) || ch->position == POS_FIGHTING )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || victim->position == POS_FIGHTING )
         continue;

      for( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
         if( obj->pIndexData->item_type == ITEM_SPICE || obj->pIndexData->item_type == ITEM_RAWSPICE )
         {
            if( victim != ch && can_see( ch, victim ) && can_see_obj( ch, obj ) )
            {
               sprintf( buf, "%s is illegal contraband. I'm going to have to confiscate that.", obj->short_descr );
               do_say( ch, buf );
               if( obj->wear_loc != WEAR_NONE )
                  remove_obj( victim, obj->wear_loc, true );
               separate_obj( obj );
               obj_from_char( obj );
               act( AT_ACTION, "$n confiscates $p from $N.", ch, obj, victim, TO_NOTVICT );
               act( AT_ACTION, "$n takes $p from you.", ch, obj, victim, TO_VICT );
               obj = obj_to_char( obj, ch );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You lose %ld experience. \r\n", ch_exp );
               gain_exp( victim, 0 - ch_exp, SMUGGLING_ABILITY );
               return true;
            }
            else if( can_see( ch, victim ) && !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %s. \r\n", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
               separate_obj( obj );
               act( AT_ACTION, "$n looks at $N suspiciously.", ch, NULL, victim, TO_NOTVICT );
               act( AT_ACTION, "$n look at you suspiciously.", ch, NULL, victim, TO_VICT );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               return true;
            }
            else if( !xIS_SET( obj->extra_flags, ITEM_CONTRABAND ) )
            {
               ch_exp =
                  UMIN( obj->cost * 10,
                        ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                          exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
               ch_printf( victim, "You receive %ld experience for smuggling %s. \r\n", ch_exp, obj->short_descr );
               gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
               separate_obj( obj );
               xSET_BIT( obj->extra_flags, ITEM_CONTRABAND );
               return true;
            }
         }
         else if( obj->item_type == ITEM_CONTAINER )
         {
            OBJ_DATA *content;
            for( content = obj->first_content; content; content = content->next_content )
            {
               if( content->pIndexData->item_type == ITEM_SPICE && !xIS_SET( content->extra_flags, ITEM_CONTRABAND ) )
               {
                  ch_exp =
                     UMIN( content->cost * 10,
                           ( exp_level( victim->skill_level[SMUGGLING_ABILITY] + 1 ) -
                             exp_level( victim->skill_level[SMUGGLING_ABILITY] ) ) );
                  ch_printf( victim, "You receive %ld experience for smuggling %s.\r\n ", ch_exp, content->short_descr );
                  gain_exp( victim, ch_exp, SMUGGLING_ABILITY );
                  separate_obj( content );
                  xSET_BIT( content->extra_flags, ITEM_CONTRABAND );
                  return true;
               }
            }
         }
      }
   }
   return false;
}

SPECF( spec_police )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int vip;
   char buf[MAX_STRING_LENGTH];

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( IS_NPC( victim ) )
         continue;
      if( !can_see( ch, victim ) )
         continue;
      if( number_bits( 1 ) == 0 )
         continue;
      for( vip = 0; vip < 32; vip++ )
         if( IS_SET( ch->vip_flags, 1 << vip ) && IS_SET( victim->pcdata->wanted_flags, 1 << vip ) )
         {
            sprintf( buf, "Hey you're wanted on %s!", planet_flags[vip] );
            do_say( ch, buf );
            REMOVE_BIT( victim->pcdata->wanted_flags, 1 << vip );
            if( ch->top_level >= victim->top_level )
               multi_hit( ch, victim, TYPE_UNDEFINED );
            else
            {
               act( AT_ACTION, "$n fines $N an enormous amount of money.", ch, NULL, victim, TO_NOTVICT );
               act( AT_ACTION, "$n fines you an enourmous amount of money.", ch, NULL, victim, TO_VICT );
               victim->gold /= 2;
            }
            return true;
         }

   }

   return false;

}

SPECF( spec_police_attack )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int vip;
   char buf[MAX_STRING_LENGTH];

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( IS_NPC( victim ) )
         continue;
      if( !can_see( ch, victim ) )
         continue;
      if( number_bits( 1 ) == 0 )
         continue;
      for( vip = 0; vip < 32; vip++ )
         if( IS_SET( ch->vip_flags, 1 << vip ) && IS_SET( victim->pcdata->wanted_flags, 1 << vip ) )
         {
            sprintf( buf, "Hey you're wanted on %s!", planet_flags[vip] );
            do_say( ch, buf );
            REMOVE_BIT( victim->pcdata->wanted_flags, 1 << vip );
            multi_hit( ch, victim, TYPE_UNDEFINED );
            return true;
         }

   }

   return false;

}

SPECF( spec_police_fine )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int vip;
   char buf[MAX_STRING_LENGTH];

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( IS_NPC( victim ) )
         continue;
      if( !can_see( ch, victim ) )
         continue;
      if( number_bits( 1 ) == 0 )
         continue;
      for( vip = 0; vip <= 31; vip++ )
         if( IS_SET( ch->vip_flags, 1 << vip ) && IS_SET( victim->pcdata->wanted_flags, 1 << vip ) )
         {
            sprintf( buf, "Hey you're wanted on %s!", planet_flags[vip] );
            do_say( ch, buf );
            act( AT_ACTION, "$n fines $N an enormous amount of money.", ch, NULL, victim, TO_NOTVICT );
            act( AT_ACTION, "$n fines you an enourmous amount of money.", ch, NULL, victim, TO_VICT );
            victim->gold /= 2;
            REMOVE_BIT( victim->pcdata->wanted_flags, 1 << vip );
            return true;
         }

   }

   return false;

}

SPECF( spec_police_jail )
{

   ROOM_INDEX_DATA *jail = NULL;
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int vip;
   char buf[MAX_STRING_LENGTH];

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( IS_NPC( victim ) )
         continue;
      if( !can_see( ch, victim ) )
         continue;
      if( number_bits( 1 ) == 0 )
         continue;
      for( vip = 0; vip <= 31; vip++ )
         if( IS_SET( ch->vip_flags, 1 << vip ) && IS_SET( victim->pcdata->wanted_flags, 1 << vip ) )
         {
            sprintf( buf, "Hey you're wanted on %s!", planet_flags[vip] );
            do_say( ch, buf );

/* currently no jails */

            if( jail )
            {
               REMOVE_BIT( victim->pcdata->wanted_flags, 1 << vip );
               act( AT_ACTION, "$n ushers $N off to jail.", ch, NULL, victim, TO_NOTVICT );
               act( AT_ACTION, "$n escorts you to jail.", ch, NULL, victim, TO_VICT );
               char_from_room( victim );
               char_to_room( victim, jail );
            }
            return true;
         }

   }

   return false;

}

SPECF( spec_jedi_healer )
{
   return false;
}



SPECF( spec_dark_jedi )
{
   return false;
}



SPECF( spec_fido )
{
   OBJ_DATA *corpse;
   OBJ_DATA *c_next;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;

   if( !IS_AWAKE( ch ) )
      return false;

   for( corpse = ch->in_room->first_content; corpse; corpse = c_next )
   {
      c_next = corpse->next_content;
      if( corpse->item_type != ITEM_CORPSE_NPC )
         continue;

      act( AT_ACTION, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
      for( obj = corpse->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         obj_from_obj( obj );
         obj_to_room( obj, ch->in_room );
      }
      extract_obj( corpse );
      return true;
   }

   return false;
}

SPECF( spec_stormtrooper )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( !can_see( ch, victim ) )
         continue;
      if( get_timer( victim, TIMER_RECENTFIGHT ) > 0 )
         continue;
      if( ( IS_NPC( victim ) && nifty_is_name( "republic", victim->name )
            && victim->fighting && who_fighting( victim ) != ch ) ||
          ( !IS_NPC( victim ) && victim->pcdata && victim->pcdata->clan && IS_AWAKE( victim )
            && nifty_is_name( "republic", victim->pcdata->clan->name ) ) )
      {
         do_yell( ch, "Die Rebel Scum!" );
         multi_hit( ch, victim, TYPE_UNDEFINED );
         return true;
      }

   }

   return false;

}

SPECF( spec_new_republic_trooper )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( !can_see( ch, victim ) )
         continue;
      if( get_timer( victim, TIMER_RECENTFIGHT ) > 0 )
         continue;
      if( ( IS_NPC( victim ) && nifty_is_name( "imperial", victim->name )
            && victim->fighting && who_fighting( victim ) != ch ) ||
          ( !IS_NPC( victim ) && victim->pcdata && victim->pcdata->clan && IS_AWAKE( victim )
            && nifty_is_name( "empire", victim->pcdata->clan->name ) ) )
      {
         do_yell( ch, "Long live the New Republic!" );
         multi_hit( ch, victim, TYPE_UNDEFINED );
         return true;
      }

   }

   return false;

}


SPECF( spec_guardian )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   CHAR_DATA *ech;
   const char *crime;
   int max_evil;

   if( !IS_AWAKE( ch ) || ch->fighting )
      return false;

   max_evil = 300;
   ech = NULL;
   crime = "";

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;
      if( victim->fighting && who_fighting( victim ) != ch && victim->alignment < max_evil )
      {
         max_evil = victim->alignment;
         ech = victim;
      }
   }

   if( victim && xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
   {
      sprintf( buf, "%s is a %s!  As well as a COWARD!", victim->name, crime );
      do_yell( ch, buf );
      return true;
   }

   if( victim )
   {
      sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!", victim->name, crime );
      do_shout( ch, buf );
      multi_hit( ch, victim, TYPE_UNDEFINED );
      return true;
   }

   if( ech )
   {
      act( AT_YELL, "$n screams 'PROTECT THE INNOCENT!!", ch, NULL, NULL, TO_ROOM );
      multi_hit( ch, ech, TYPE_UNDEFINED );
      return true;
   }

   return false;
}



SPECF( spec_janitor )
{
   OBJ_DATA *trash;
   OBJ_DATA *trash_next;

   if( !IS_AWAKE( ch ) )
      return false;

   for( trash = ch->in_room->first_content; trash; trash = trash_next )
   {
      trash_next = trash->next_content;
      if( !IS_SET( trash->wear_flags, ITEM_TAKE ) || IS_OBJ_STAT( trash, ITEM_BURRIED ) )
         continue;
      if( IS_OBJ_STAT( trash, ITEM_PROTOTYPE ) && !xIS_SET( ch->act, ACT_PROTOTYPE ) )
         continue;
      if( trash->item_type == ITEM_DRINK_CON
          || trash->item_type == ITEM_TRASH
          || trash->cost < 10 || ( trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG && !trash->first_content ) )
      {
         act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
         obj_from_room( trash );
         obj_to_char( trash, ch );
         return true;
      }
   }

   return false;
}



SPECF( spec_poison )
{
   CHAR_DATA *victim;

   if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL || number_percent(  ) > 2 * ch->top_level )
      return false;

   act( AT_HIT, "You bite $N!", ch, NULL, victim, TO_CHAR );
   act( AT_ACTION, "$n bites $N!", ch, NULL, victim, TO_NOTVICT );
   act( AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT );
   return true;
}



SPECF( spec_thief )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int gold, maxgold;

   if( ch->position != POS_STANDING )
      return false;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( IS_NPC( victim ) || get_trust( victim ) >= LEVEL_STAFF || number_bits( 2 ) != 0 || !can_see( ch, victim ) )    /* Thx Glop */
         continue;

      if( IS_AWAKE( victim ) && number_range( 0, ch->top_level ) == 0 )
      {
         act( AT_ACTION, "You discover $n's hands in your wallet!", ch, NULL, victim, TO_VICT );
         act( AT_ACTION, "$N discovers $n's hands in $S wallet!", ch, NULL, victim, TO_NOTVICT );
         return true;
      }
      else
      {
         maxgold = ch->top_level * ch->top_level * 1000;
         gold = victim->gold * number_range( 1, URANGE( 2, ch->top_level / 4, 10 ) ) / 100;
         ch->gold += 9 * gold / 10;
         victim->gold -= gold;
         if( ch->gold > maxgold )
         {
            boost_economy( ch->in_room->area, ch->gold - maxgold / 2 );
            ch->gold = maxgold / 2;
         }
         return true;
      }
   }

   return false;
}

SPECF( spec_auth )
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   char buf[MAX_STRING_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   bool hasdiploma;

   for( victim = ch->in_room->first_person; victim; victim = v_next )
   {
      v_next = victim->next_in_room;

      if( !IS_NPC( victim ) && ( pObjIndex = get_obj_index( OBJ_VNUM_SCHOOL_DIPLOMA ) ) != NULL )
      {
         hasdiploma = false;

         for( obj = victim->last_carrying; obj; obj = obj->prev_content )
            if( obj->pIndexData == get_obj_index( OBJ_VNUM_SCHOOL_DIPLOMA ) )
               hasdiploma = true;

         if( !hasdiploma )
         {
            obj = create_object( pObjIndex, 1 );
            obj = obj_to_char( obj, victim );
            send_to_char( "&cZechs gives you a diploma, and shakes your hand.\r\n&w", victim );
         }
      }

      if( IS_NPC( victim ) || !IS_SET( victim->pcdata->flags, PCFLAG_UNAUTHED ) )
         continue;

      victim->pcdata->auth_state = 3;
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_UNAUTHED );
      if( victim->pcdata->authed_by )
         STRFREE( victim->pcdata->authed_by );
      victim->pcdata->authed_by = QUICKLINK( ch->name );
      sprintf( buf, "%s authorized %s", ch->name, victim->name );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );


   }
   return false;

}

SPECF( spec_infected )
{
   return false;
}

SPECF( spec_claude )
{
   CHAR_DATA *victim;
   int chance;
   int chancey;

   if( ch->position == POS_STANDING )
   {
      chancey = number_range( 1, 3 );

      if( chancey == 1 )
      {
         if( ch->hit < ch->max_hit )
         {
            do_say( ch, "&YE&On&per&Og&Yy &PS&pw&Oo&pr&Pd&C!" );
            act( AT_GREEN, "$n gleams with energy, regaining needed hp!", ch, NULL, NULL, TO_ROOM );
            ch->hit += 100000;
            if( ch->hit > ch->max_hit )
            {
               ch->hit = ch->max_hit;
            }
            return true;
         }
         return false;
      }
   }

   if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL )
      return false;

   chance = number_range( 1, 8 );

   if( chance != 1 )
      return false;

   chancey = number_range( 1, 2 );

   if( chancey == 1 )
   {
      act( AT_RED, "$n lashes out with his Mirror Circle Slash!", ch, NULL, victim, TO_VICT );
      act( AT_RED, "$n lashes out with his Mirror Circle Slash on $N!", ch, NULL, victim, TO_NOTVICT );
      global_retcode = damage( ch, victim, number_range( 5000, 15000 ), gsn_circle );
      global_retcode = damage( ch, victim, number_range( 5000, 15000 ), gsn_circle );
      return true;
   }
   if( chancey == 2 )
   {
      do_say( ch, "&YE&On&per&Og&Yy &PS&pw&Oo&pr&Pd&C!" );
      act( AT_GREEN, "$n gleams with energy, regaining needed hp!", ch, NULL, NULL, TO_ROOM );
      ch->hit += 100000;
      return true;
   }

   return false;
}

SPECF( spec_blahb )
{
   CHAR_DATA *victim;
   int chance;
   int chancey;

   if( ch->position == POS_STANDING )
   {
      chancey = number_range( 1, 3 );

      if( chancey == 1 )
      {
         if( ch->hit < ch->max_hit )
         {
            do_say( ch, "&zD&Wa&wr&zk &RE&rn&zer&rg&Ry&Y!" );
            act( AT_GREEN, "$n is surrounded by a dark aura, using dark energy to draw in health!",
                 ch, NULL, NULL, TO_ROOM );
            ch->hit += 100000;
            if( ch->hit > ch->max_hit )
            {
               ch->hit = ch->max_hit;
            }
            return true;
         }
         return false;
      }
   }

   if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL )
      return false;

   chance = number_range( 1, 8 );

   if( chance != 1 )
      return false;

   chancey = number_range( 1, 2 );

   if( chancey == 1 )
   {
      act( AT_RED, "$n one!", ch, NULL, victim, TO_VICT );
      act( AT_RED, "$n one $N!", ch, NULL, victim, TO_NOTVICT );
      act( AT_RED, "$n one!", ch, NULL, victim, TO_VICT );
      act( AT_RED, "$n one $N!", ch, NULL, victim, TO_NOTVICT );
      global_retcode = damage( ch, victim, number_range( 5000, 15000 ), gsn_circle );
      global_retcode = damage( ch, victim, number_range( 5000, 15000 ), gsn_circle );
      return true;
   }
   if( chancey == 2 )
   {
      do_say( ch, "&YE&On&per&Og&Yy &PS&pw&Oo&pr&Pd&C!" );
      act( AT_GREEN, "$n gleams with energy, regaining needed hp!", ch, NULL, victim, TO_VICT );
      act( AT_GREEN, "$n gleams with energy, regaining needed hp!", ch, NULL, victim, TO_NOTVICT );
      ch->hit += 100000;
      return true;
   }

   return false;
}

SPECF( spec_homunculus )
{
   CHAR_DATA *victim;
   CHAR_DATA *vch;
   DESCRIPTOR_DATA *d;
   int atk = 0;
   int chance = 0;
   short dameq;
   OBJ_DATA *damobj;

   if( ch->pIndexData->vnum == 540 )
   {
      if( ch->position == POS_STANDING )
      {
         ch->hit += 400000;
         if( ch->hit > ch->max_hit )
         {
            ch->hit = ch->max_hit;
         }
         return true;
      }
      if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL )
         return false;

      for( d = first_descriptor; d; d = d->next )
      {
         if( ( d->connected == CON_PLAYING ) && ( vch = d->character ) != NULL )

            if( !IS_NPC( vch ) )
            {

               if( who_fighting( vch ) == ch )
               {
                  atk += 1;
               }

            }
      }

      act( AT_RED, "$n uses his ultimate eye technique, using your advantage to your disadvantage!", ch, NULL, victim,
           TO_VICT );
      act( AT_RED, "$n uses his ultimate eye, using $N's advantage against $M!", ch, NULL, victim, TO_NOTVICT );
      damage( ch, victim, number_range( ( atk * 9000 ), ( atk * 10000 ) ), gsn_punch );
      return true;
   }

   if( ch->pIndexData->vnum == 2030 )
   {
      if( ch->position == POS_STANDING )
      {
         ch->hit += 400000;
         if( ch->hit > ch->max_hit )
         {
            ch->hit = ch->max_hit;
         }
         return true;
      }

      if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL )
         return false;

      /*
       * get a random body eq part 
       */
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }

      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }
      dameq = number_range( WEAR_LIGHT, WEAR_EYES );
      damobj = get_eq_char( victim, dameq );
      if( damobj )
      {
         set_cur_obj( damobj );
         damage_obj( damobj );
      }

      act( AT_RED, "$n lunges forward, biting right through anything you're wearing!", ch, NULL, victim, TO_VICT );
      act( AT_RED, "$n lunges forward, biting through anything $N is wearing!", ch, NULL, victim, TO_NOTVICT );
      global_retcode = damage( ch, victim, number_range( 10000, 30000 ), gsn_bite );
      return true;

   }

   if( ch->pIndexData->vnum == 5190 )
   {

      if( ch->position == POS_STANDING )
      {
         ch->hit += 400000;
         if( ch->hit > ch->max_hit )
         {
            ch->hit = ch->max_hit;
         }
         return true;
      }

      if( ch->position != POS_FIGHTING || ( victim = who_fighting( ch ) ) == NULL )
         return false;

      chance = number_range( 1, 2 );

      if( chance == 1 )
      {
         act( AT_RED, "$n concentrates, applying energy to his fist!", ch, NULL, victim, TO_VICT );
         act( AT_RED, "$n concentrates, applying energy to his fist on $N!", ch, NULL, victim, TO_NOTVICT );
         global_retcode = damage( ch, victim, number_range( 10000, 30000 ), gsn_punch );
         return true;
      }

      return false;

   }
   return false;
}
