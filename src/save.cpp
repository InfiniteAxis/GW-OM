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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <sys/dir.h>
#include <dirent.h>
#include "mud.h"
#include "bits.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	6


 /*
  * Array to keep track of equipment temporarily.		-Thoric
  */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA *ch, FILE *fp );
void fread_comment( CHAR_DATA *ch, FILE *fp );
BIT_DATA *find_qbit( int number );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp );
void fread_char( CHAR_DATA *ch, FILE *fp, bool preload, bool copyover );
void write_corpses( CHAR_DATA *ch, const char *name );
int get_obj_room_vnum_recursive( OBJ_DATA *obj );
bool check_parse_name( const char *name );

void save_home( CHAR_DATA *ch )
{

	return;

	if( ch->plr_home )
	{
		FILE *fp;
		char filename[256];
		short templvl;
		OBJ_DATA *contents;


		sprintf( filename, "%s%c/%s.home", PLAYER_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
		if( ( fp = FileOpen( filename, "w" ) ) == NULL )
		{
		}
		else
		{
			templvl = ch->top_level;
			ch->top_level = LEVEL_HERO;    /* make sure EQ doesn't get lost */
			contents = ch->plr_home->last_content;
			if( contents )
				fwrite_obj( ch, contents, fp, 0, OS_CARRY, false );
			fprintf( fp, "#END\n" );
			ch->top_level = templvl;
			FileClose( fp );
		}
	}
}


void load_home( CHAR_DATA *ch )
{
	char filename[256];
	FILE *fph;
	ROOM_INDEX_DATA *storeroom = ch->plr_home;

	sprintf( filename, "%s%c/%s.home", PLAYER_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
	if( ( fph = FileOpen( filename, "r" ) ) != NULL )
	{
		int iNest;
		OBJ_DATA *tobj, *tobj_next;

		rset_supermob( storeroom );
		for( iNest = 0; iNest < MAX_NEST; iNest++ )
			rgObjNest[iNest] = NULL;

		for( ;; )
		{
			char letter;
			char *word;

			letter = fread_letter( fph );
			if( letter == '*' )
			{
				fread_to_eol( fph );
				continue;
			}

			if( letter != '#' )
			{
				bug( "Load_plr_home: # not found.", 0 );
				bug( ch->name, 0 );
				break;
			}

			word = fread_word( fph );
			if( !str_cmp( word, "OBJECT" ) )   /* Objects  */
				fread_obj( supermob, fph, OS_CARRY );
			else if( !str_cmp( word, "END" ) ) /* Done     */
				break;
			else
			{
				bug( "Load_plr_home: bad section.", 0 );
				bug( ch->name, 0 );
				break;
			}
		}

		FileClose( fph );

		for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
		{
			tobj_next = tobj->next_content;
			obj_from_char( tobj );
			obj_to_room( tobj, storeroom );
		}

		release_supermob( );

	}
}


/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int x, y;

	for( x = 0; x < MAX_WEAR; x++ )
		for( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;
	for( obj = ch->first_carrying; obj; obj = obj->next_content )
		if( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
		{

			for( x = 0; x < MAX_LAYERS; x++ )
				if( !save_equipment[obj->wear_loc][x] )
				{
					save_equipment[obj->wear_loc][x] = obj;
					break;
				}
			if( x == MAX_LAYERS )
			{
				sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
					ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
				bug( buf, 0 );
			}

			unequip_char( ch, obj );
		}
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
	int x, y;

	for( x = 0; x < MAX_WEAR; x++ )
		for( y = 0; y < MAX_LAYERS; y++ )
			if( save_equipment[x][y] != NULL )
			{
				if( quitting_char != ch )
					equip_char( ch, save_equipment[x][y], x );
				save_equipment[x][y] = NULL;
			}
			else
				break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
	char strsave[MAX_INPUT_LENGTH];
	char strback[MAX_INPUT_LENGTH];
	FILE *fp;

	if( !ch )
	{
		bug( "Save_char_obj: null ch!", 0 );
		return;
	}

	if( IS_NPC( ch ) || NOT_AUTHED( ch ) )
		return;

	saving_char = ch;
	/*
	 * save pc's clan's data while we're at it to keep the data in sync
	 */
	if( !IS_NPC( ch ) && ch->pcdata->clan )
		save_clan( ch->pcdata->clan );

	if( ch->desc && ch->desc->original )
		ch = ch->desc->original;

	de_equip_char( ch );

	ch->save_time = current_time;
	sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	/*
	 * Auto-backup pfile (can cause lag with high disk access situtations
	 */
	if( IS_SET( sysdata.save_flags, SV_BACKUP ) )
	{
		sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
		rename( strsave, strback );
	}

	/*
	 * Save immortal stats, level & vnums for wizlist       -Thoric
	 * and do_vnums command
	 *
	 * Also save the player flags so we the wizlist builder can see
	 * who is a guest and who is retired.
	 */
	if( get_trust( ch ) > LEVEL_HERO )
	{
		const char *jobtitle;

		switch( ch->top_level )
		{
		case MAX_LEVEL - 0:
			jobtitle = "&wO&zwner&D";
			break;
		case MAX_LEVEL - 1:
			jobtitle = "&RA&rdmin&D";
			break;
		case MAX_LEVEL - 2:
			jobtitle = "&CS&chipwright&D";
			break;
		case MAX_LEVEL - 3:
			jobtitle = "&YB&Ouilder&D";
			break;
		case MAX_LEVEL - 4:
			jobtitle = "&RE&rnforcer&D";
			break;
		case MAX_LEVEL - 5:
			jobtitle = "&PL&piaison&D";
			break;
		case MAX_LEVEL - 6:
			jobtitle = "&GS&gtaff";
			break;
		default:
			jobtitle = "&WU&wndisclosed&D";
			break;
		}
		sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->name ) );

		if( ( fp = FileOpen( strback, "w" ) ) == NULL )
		{
			bug( "Save_god_level: FileOpen", 0 );
			perror( strback );
		}
		else
		{
			fprintf( fp, "Level        %d\n", ch->top_level );
			fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
			fprintf( fp, "Aim          %s~\n", ch->pcdata->aim ? ch->pcdata->aim : "N/A" );
			fprintf( fp, "Job          %s~\n", ch->pcdata->job ? ch->pcdata->job : jobtitle );
			if( ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
				fprintf( fp, "VnumRange    %d %d\n", ch->pcdata->low_vnum, ch->pcdata->hi_vnum );
			fprintf( fp, "%s", "End\n" );
			FileClose( fp );
		}
	}

	if( ( fp = FileOpen( strsave, "w" ) ) == NULL )
	{
		bug( "Save_char_obj: FileOpen", 0 );
		perror( strsave );
	}
	else
	{
		fwrite_char( ch, fp );
		if( ch->first_carrying )
			fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY, ch->pcdata->hotboot );
		if( ch->comments )    /* comments */
			fwrite_comments( ch, fp ); /* comments */
		fprintf( fp, "#END\n" );
		FileClose( fp );
	}

	re_equip_char( ch );

	write_corpses( ch, NULL );
	quitting_char = NULL;
	saving_char = NULL;
	return;
}

void save_clone( CHAR_DATA *ch )
{
	char strsave[MAX_INPUT_LENGTH];
	char strback[MAX_INPUT_LENGTH];
	FILE *fp;

	if( !ch )
	{
		bug( "Save_char_obj: null ch!", 0 );
		return;
	}

	if( IS_NPC( ch ) || NOT_AUTHED( ch ) )
		return;

	if( ch->desc && ch->desc->original )
		ch = ch->desc->original;

	de_equip_char( ch );

	ch->save_time = current_time;
	sprintf( strsave, "%s%c/%s.clone", PLAYER_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );

	/*
	 * Auto-backup pfile (can cause lag with high disk access situtations
	 */
	if( IS_SET( sysdata.save_flags, SV_BACKUP ) )
	{
		sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
		rename( strsave, strback );
	}

	if( ( fp = FileOpen( strsave, "w" ) ) == NULL )
	{
		bug( "Save_char_obj: FileOpen", 0 );
		perror( strsave );
	}
	else
	{
		fwrite_char( ch, fp );
		if( ch->comments )    /* comments */
			fwrite_comments( ch, fp ); /* comments */
		fprintf( fp, "#END\n" );
		FileClose( fp );
	}

	re_equip_char( ch );

	write_corpses( ch, NULL );
	quitting_char = NULL;
	saving_char = NULL;
	return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
	AFFECT_DATA *paf;
	ALIAS_DATA *pal;
	int sn, track, drug, i;
	SKILLTYPE *skill;

	fprintf( fp, "#%s\n", IS_NPC( ch ) ? "MOB" : "PLAYER" );

	fprintf( fp, "Version      %d\n", SAVEVERSION );
	fprintf( fp, "Name         %s~\n", capitalize( ch->name ) );
	if( ch->short_descr && ch->short_descr[0] != '\0' )
		fprintf( fp, "ShortDescr   %s~\n", ch->short_descr );
	if( ch->long_descr && ch->long_descr[0] != '\0' )
		fprintf( fp, "LongDescr    %s~\n", ch->long_descr );
	if( ch->description && ch->description[0] != '\0' )
		fprintf( fp, "Description  %s~\n", ch->description );
	fprintf( fp, "Sex          %d\n", ch->sex );
	fprintf( fp, "Race         %d\n", ch->race );
	fprintf( fp, "MainAbility  %d\n", ch->main_ability );
	fprintf( fp, "Toplevel     %d\n", ch->top_level );
	if( ch->trust )
		fprintf( fp, "Trust        %d\n", ch->trust );
	fprintf( fp, "Played       %d\n", ch->played + ( int ) ( current_time - ch->logon ) );
	fprintf( fp, "Room         %d\n",
		( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
			&& ch->was_in_room ) ? ch->was_in_room->vnum : ch->in_room->vnum );
	if( ch->plr_home != NULL )
		fprintf( fp, "PlrHome      %d\n", ch->plr_home->vnum );

	fprintf( fp, "HpManaMove   %ld %ld 0 0 %d %d\n", ch->hit, ch->max_hit, ch->move, ch->max_move );
	fprintf( fp, "Gold         %d\n", ch->gold );
	fprintf( fp, "WFMTimer     %d\n", ch->wfm_timer );
	fprintf( fp, "LP           %d\n", ch->pcdata->lp );
	fprintf( fp, "Bank         %ld\n", ch->pcdata->bank );
	{
		int ability;
		for( ability = 0; ability < MAX_ABILITY; ability++ )
			fprintf( fp, "Ability        %d %d %ld\n", ability, ch->skill_level[ability], ch->experience[ability] );
	}
	fprintf( fp, "Energy         %d\n", ch->pcdata->xenergy );
	fprintf( fp, "Energymax      %d\n", ch->pcdata->xenergymax );
	fprintf( fp, "SpecialWeapon  %d\n", ch->pcdata->specialweapon );
	fprintf( fp, "Salary_time     %ld\n", ch->pcdata->salary_date );
	fprintf( fp, "Salary          %d\n", ch->pcdata->salary );
	if( ch->pcdata->spouse )
	{
		fprintf( fp, "Spou %s~\n", ch->pcdata->spouse );
	}

	if( ch->pcdata->tspouse && ch->pcdata->tspouse[0] != '\0' )
		fprintf( fp, "TSpouse         %s~\n", ch->pcdata->tspouse );

	fprintf( fp, "Dat             %d\n", ch->pcdata->dating );
	fprintf( fp, "Eng             %d\n", ch->pcdata->engaged );
	fprintf( fp, "Mar             %d\n", ch->pcdata->married );
	fprintf( fp, "Rtm             %d\n", ch->pcdata->rtime );

	if( ch->pcdata->ticketnumber )
		fprintf( fp, "TicketNumber    %d\n", ch->pcdata->ticketnumber );

	if( ch->pcdata->ticketweek )
		fprintf( fp, "TicketWeek      %d\n", ch->pcdata->ticketweek );

	fprintf( fp, "Questpoints     %d\n", ch->questpoints );
	fprintf( fp, "Nextquest       %d\n", ch->nextquest );
	if( !xIS_EMPTY( ch->act ) )
		fprintf( fp, "Act           %s\n", print_bitvector( &ch->act ) );
	if( ch->affected_by )
		fprintf( fp, "AffectedBy    %d\n", ch->affected_by );
	fprintf( fp, "Position     %d\n", ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

	fprintf( fp, "SavingThrows %d %d %d %d %d\n",
		ch->saving_poison_death, ch->saving_wand, ch->saving_para_petri, ch->saving_breath, ch->saving_spell_staff );
	fprintf( fp, "Alignment    %d\n", ch->alignment );
	fprintf( fp, "Glory        %d\n", ch->pcdata->quest_curr );
	fprintf( fp, "MGlory       %d\n", ch->pcdata->quest_accum );
	fprintf( fp, "Hitroll      %d\n", ch->hitroll );
	fprintf( fp, "Damroll      %d\n", ch->damroll );
	fprintf( fp, "Armor        %d\n", ch->armor );
	fprintf( fp, "Storagetimer %d\n", ch->pcdata->storagetimer );
	fprintf( fp, "Storagecost  %d\n", ch->pcdata->storagecost );

	if( ch->wimpy )
		fprintf( fp, "Wimpy        %d\n", ch->wimpy );
	if( ch->deaf )
		fprintf( fp, "Deaf         %d\n", ch->deaf );

	if( ch->resistant )
		fprintf( fp, "Resistant    %d\n", ch->resistant );
	if( ch->immune )
		fprintf( fp, "Immune       %d\n", ch->immune );
	if( ch->susceptible )
		fprintf( fp, "Susceptible  %d\n", ch->susceptible );
	if( ch->pcdata && ch->pcdata->outcast_time )
		fprintf( fp, "Outcast_time %ld\n", ch->pcdata->outcast_time );
	if( ch->pcdata && ch->pcdata->restore_time )
		fprintf( fp, "Restore_time %ld\n", ch->pcdata->restore_time );
	if( ch->mental_state != -10 )
		fprintf( fp, "Mentalstate  %d\n", ch->mental_state );

	if( IS_NPC( ch ) )
	{
		fprintf( fp, "Vnum         %d\n", ch->pIndexData->vnum );
		fprintf( fp, "Mobinvis     %d\n", ch->mobinvis );
	}
	else
	{
		fprintf( fp, "Password     %s~\n", ch->pcdata->pwd );
		fprintf( fp, "Lastplayed   %d\n", ( int ) current_time );
		if( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
			fprintf( fp, "Bamfin       %s~\n", ch->pcdata->bamfin );
		//  if ( ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0' )
		//    fprintf( fp, "Spouse       %s~\n", ch->pcdata->spouse );
		if( ch->pcdata->build )
			fprintf( fp, "Build       %d\n", ch->pcdata->build );
		if( ch->pcdata->eye )
			fprintf( fp, "Eye	   %d\n", ch->pcdata->eye );
		if( ch->pcdata->hair )
			fprintf( fp, "Hair	   %d\n", ch->pcdata->hair );
		if( ch->pcdata->highlight )
			fprintf( fp, "Highlight   %d\n", ch->pcdata->highlight );
		if( ch->pcdata->hero )
			fprintf( fp, "Hero	   %d\n", ch->pcdata->hero );
		if( ch->pcdata->email && ch->pcdata->email[0] != '\0' )
			fprintf( fp, "Email       %s~\n", ch->pcdata->email );

		if( ch->pcdata->icq )
			fprintf( fp, "ICQ       %d\n", ch->pcdata->icq );

		if( ch->pcdata->aim && ch->pcdata->aim[0] != '\0' )
			fprintf( fp, "Aim       %s~\n", ch->pcdata->aim );

		if( ch->pcdata->msn && ch->pcdata->msn[0] != '\0' )
			fprintf( fp, "MSN       %s~\n", ch->pcdata->msn );

		if( ch->pcdata->real && ch->pcdata->real[0] != '\0' )
			fprintf( fp, "Real      %s~\n", ch->pcdata->real );

		if( ch->pcdata->yim && ch->pcdata->yim[0] != '\0' )
			fprintf( fp, "YIM       %s~\n", ch->pcdata->yim );

		if( ch->pcdata->enter && ch->pcdata->enter[0] != '\0' )
			fprintf( fp, "Enter       %s~\n", ch->pcdata->enter );

		if( ch->pcdata->exit && ch->pcdata->exit[0] != '\0' )
			fprintf( fp, "Exit       %s~\n", ch->pcdata->exit );

		if( ch->pcdata->avatar && ch->pcdata->avatar[0] != '\0' )
			fprintf( fp, "Avatar       %s~\n", ch->pcdata->avatar );

		if( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
			fprintf( fp, "Bamfout      %s~\n", ch->pcdata->bamfout );
		if( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
			fprintf( fp, "Rank         %s~\n", ch->pcdata->rank );
		if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
			fprintf( fp, "Bestowments  %s~\n", ch->pcdata->bestowments );
		fprintf( fp, "Title        %s~\n", ch->pcdata->title );
		fprintf( fp, "Ragemeter      %d\n", ch->pcdata->ragemeter );
		if( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
			fprintf( fp, "Homepage     %s~\n", ch->pcdata->homepage );
		if( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
			fprintf( fp, "Bio          %s~\n", ch->pcdata->bio );
		if( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
			fprintf( fp, "AuthedBy     %s~\n", ch->pcdata->authed_by );
		if( ch->pcdata->min_snoop )
			fprintf( fp, "Minsnoop     %d\n", ch->pcdata->min_snoop );
		if( ch->pcdata->prompt && *ch->pcdata->prompt )
			fprintf( fp, "Prompt       %s~\n", ch->pcdata->prompt );

		/*
		 * Save note board status
		 */
		 /*
		  * Save number of boards in case that number changes
		  */
		fprintf( fp, "Boards       %d ", MAX_BOARD );
		for( i = 0; i < MAX_BOARD; i++ )
			fprintf( fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i] );
		fprintf( fp, "\n" );

		if( ch->pcdata->pagerlen != 24 )
			fprintf( fp, "Pagerlen     %d\n", ch->pcdata->pagerlen );
		{
			IGNORE_DATA *temp;
			for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
			{
				fprintf( fp, "Ignored      %s~\n", temp->name );
			}
		}

		for( pal = ch->pcdata->first_alias; pal; pal = pal->next )
		{
			if( !pal->name || !pal->cmd || !*pal->name || !*pal->cmd )
				continue;
			fprintf( fp, "Aliases      %s~ %s~\n", pal->name, pal->cmd );
		}

		fprintf( fp, "Addiction   " );
		for( drug = 0; drug <= 9; drug++ )
			fprintf( fp, " %d", ch->pcdata->addiction[drug] );
		fprintf( fp, "\n" );
		fprintf( fp, "Druglevel   " );
		for( drug = 0; drug <= 9; drug++ )
			fprintf( fp, " %d", ch->pcdata->drug_level[drug] );
		fprintf( fp, "\n" );
		if( ch->pcdata->wanted_flags )
			fprintf( fp, "Wanted       %d\n", ch->pcdata->wanted_flags );

		if( IS_IMMORTAL( ch ) || ch->pcdata->area )
		{
			fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );

			if( ch->pcdata->job && ch->pcdata->job[0] != '\0' )
				fprintf( fp, "Job       %s~\n", ch->pcdata->job );

			if( ch->pcdata->tspouse && ch->pcdata->tspouse[0] != '\0' )
				fprintf( fp, "TSpouse       %s~\n", ch->pcdata->tspouse );

			if( ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0' )
				fprintf( fp, "Spouse       %s~\n", ch->pcdata->spouse );

			fprintf( fp, "Predating    %d\n", ch->pcdata->predating );
			fprintf( fp, "Dating       %d\n", ch->pcdata->dating );
			fprintf( fp, "PreEngaged   %d\n", ch->pcdata->preengaged );
			fprintf( fp, "PreMarried   %d\n", ch->pcdata->premarried );

			if( ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
				fprintf( fp, "VnumRange    %d %d\n", ch->pcdata->low_vnum, ch->pcdata->hi_vnum );
		}
		if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
			fprintf( fp, "Clan         %s~\n", ch->pcdata->clan_name );
		fprintf( fp, "CRank         %d\n", ch->pcdata->clan_rank );
		fprintf( fp, "Flags        %d\n", ch->pcdata->flags );
		fprintf( fp, "Cybaflags    %d\n", ch->pcdata->cybaflags );
		fprintf( fp, "HLevel       %d\n", ch->pcdata->hlevel );
		fprintf( fp, "HKills       %d\n", ch->pcdata->hkills );
		if( ch->pcdata->inivictim && ch->pcdata->inivictim[0] != '\0' )
			fprintf( fp, "Inivictim    %s~\n", ch->pcdata->inivictim );
		if( ch->pcdata->iclan && ch->pcdata->iclan[0] != '\0' )
			fprintf( fp, "Iclan    %s~\n", ch->pcdata->iclan );
		if( ch->pcdata->release_date > current_time )
			fprintf( fp, "Helled       %d %s~\n", ( int ) ch->pcdata->release_date, ch->pcdata->helled_by );
		if( ch->pcdata->pkills )
			fprintf( fp, "PKills       %d\n", ch->pcdata->pkills );
		if( ch->pcdata->pdeaths )
			fprintf( fp, "PDeaths      %d\n", ch->pcdata->pdeaths );
		if( ch->pcdata->apkills )
			fprintf( fp, "APKills       %d\n", ch->pcdata->apkills );
		if( ch->pcdata->apdeaths )
			fprintf( fp, "APDeaths      %d\n", ch->pcdata->apdeaths );
		if( ch->pcdata->beenfroze )
			fprintf( fp, "Beenfroze       %d\n", ch->pcdata->beenfroze );
		if( ch->pcdata->hasfroze )
			fprintf( fp, "Hasfroze      %d\n", ch->pcdata->hasfroze );
		if( get_timer( ch, TIMER_PKILLED ) && ( get_timer( ch, TIMER_PKILLED ) > 0 ) )
			fprintf( fp, "PTimer       %d\n", get_timer( ch, TIMER_PKILLED ) );
		fprintf( fp, "MKills       %d\n", ch->pcdata->mkills );
		fprintf( fp, "MDeaths      %d\n", ch->pcdata->mdeaths );
		if( ch->pcdata->illegal_pk )
			fprintf( fp, "IllegalPK    %d\n", ch->pcdata->illegal_pk );
		fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
			ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );

		fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
			ch->mod_str, ch->mod_int, ch->mod_wis, ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck );

		fprintf( fp, "Condition    %d %d %d %d\n",
			ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], ch->pcdata->condition[3] );
		if( ch->desc && ch->desc->host )
			fprintf( fp, "Site         %s\n", ch->desc->host );
		else
			fprintf( fp, "Site         (Link-Dead)\n" );

		fprintf( fp, "LastHost   %s\n", ch->pcdata->lasthost );

		for( sn = 1; sn < top_sn; sn++ )
		{
			if( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
				switch( skill_table[sn]->type )
				{
				default:
					fprintf( fp, "Skill        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
					break;
				case SKILL_SPELL:
					fprintf( fp, "Spell        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
					break;
				case SKILL_ART:
					fprintf( fp, "Art          %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
					break;
				case SKILL_WEAPON:
					fprintf( fp, "Weapon       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
					break;
				case SKILL_TONGUE:
					fprintf( fp, "Tongue       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
					break;
				}
		}
	}

	for( paf = ch->first_affect; paf; paf = paf->next )
	{
		if( paf->type >= 0 && ( skill = get_skilltype( paf->type ) ) == NULL )
			continue;

		if( paf->type >= 0 && paf->type < TYPE_PERSONAL )
			fprintf( fp, "AffectData   '%s' %3d %3d %3d %d\n",
				skill->name, paf->duration, paf->modifier, paf->location, paf->bitvector );
		else
			fprintf( fp, "Affect       %3d %3d %3d %3d %d\n",
				paf->type, paf->duration, paf->modifier, paf->location, paf->bitvector );
	}

	track = URANGE( 2, ( ( ch->top_level + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
	for( sn = 0; sn < track; sn++ )
	{
		if( ch->pcdata->killed[sn].vnum == 0 )
			break;
		fprintf( fp, "Killed       %d %d\n", ch->pcdata->killed[sn].vnum, ch->pcdata->killed[sn].count );
	}
	/*
	 * Save color values - Samson 9-29-98
	 */
	{
		int x;
		fprintf( fp, "MaxColors    %d\n", MAX_COLORS );
		fprintf( fp, "Colors       " );
		for( x = 0; x < MAX_COLORS; x++ )
			fprintf( fp, "%d ", ch->colors[x] );
		fprintf( fp, "\n" );
	}

	if( ch->pcdata->firstqbit )
	{
		BIT_DATA *bit;

		for( bit = ch->pcdata->firstqbit; bit; bit = bit->next )
			fprintf( fp, "Qbit	%d %s~\n", bit->number, bit->desc );
	}

	fprintf( fp, "End\n\n" );
	return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest, short os_type, bool hotboot )
{
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	short wear, wear_loc, x;

	if( iNest >= MAX_NEST )
	{
		bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
		return;
	}

	if( !obj )
	{
		bug( "%s: NULL obj", __FUNCTION__ );
		return;
	}

	/*
	 * Slick recursion to write lists backwards,
	 *   so loading them will load in forwards order.
	 */
	if( obj->prev_content && ( os_type != OS_CORPSE && os_type != OS_LOCKER ) )
	{
		if( os_type == OS_CARRY )
			fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY, hotboot );
		else
			fwrite_obj( ch, obj->prev_content, fp, iNest, OS_GROUND, hotboot );
	}

	/*
	 * Castrate storage characters.
	 */
	if( !hotboot )
	{
		if( obj->item_type == ITEM_KEY && !IS_OBJ_STAT( obj, ITEM_CLANOBJECT ) )
			return;
	}

	/*
	* Catch deleted objects              -Thoric
	*/
	if( obj_extracted( obj ) )
		return;

	/*
	* Do NOT save prototype items!          -Thoric
	*/
	if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		return;

	/*
	* DO NOT save corpses lying on the ground as a hotboot item, they already saved elsewhere! - Samson
	*/
	if( hotboot && obj->item_type == ITEM_CORPSE_PC )
		return;

	if( hotboot && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		return;

	/*
	* Corpse saving. -- Altrag
	*/
	fprintf( fp, ( os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n" ) );

	if( iNest )
		fprintf( fp, "Nest         %d\n", iNest );
	if( obj->count > 1 && !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
		fprintf( fp, "Count        %d\n", obj->count );
	if( obj->name && ( !obj->pIndexData->name || str_cmp( obj->name, obj->pIndexData->name ) ) )
		fprintf( fp, "Name         %s~\n", obj->name );
	if( obj->short_descr && ( !obj->pIndexData->short_descr || str_cmp( obj->short_descr, obj->pIndexData->short_descr ) ) )
		fprintf( fp, "ShortDescr   %s~\n", obj->short_descr );
	if( obj->description && ( !obj->pIndexData->description || str_cmp( obj->description, obj->pIndexData->description ) ) )
		fprintf( fp, "Description  %s~\n", obj->description );
	if( obj->action_desc && ( !obj->pIndexData->action_desc || str_cmp( obj->action_desc, obj->pIndexData->action_desc ) ) )
		fprintf( fp, "ActionDesc   %s~\n", obj->action_desc );
	fprintf( fp, "Vnum         %d\n", obj->pIndexData->vnum );
	if( ( ( os_type == OS_GROUND ) || ( os_type == OS_CORPSE || os_type == OS_LOCKER || hotboot ) ) && obj->in_room )
	{
		fprintf( fp, "Room         %d\n", obj->in_room->vnum );
		fprintf( fp, "Rvnum	     %d\n", obj->room_vnum );
	}
	else if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) && !obj->in_room )
		fprintf( fp, "Room		%d\n", get_obj_room_vnum_recursive( obj ) );
	if( !xSAME_BITS( obj->extra_flags, obj->pIndexData->extra_flags ) )
		fprintf( fp, "ExtraFlags   %s\n", print_bitvector( &obj->extra_flags ) );
	if( obj->wear_flags != obj->pIndexData->wear_flags )
		fprintf( fp, "WearFlags    %d\n", obj->wear_flags );
	wear_loc = -1;
	for( wear = 0; wear < MAX_WEAR; wear++ )
		for( x = 0; x < MAX_LAYERS; x++ )
			if( obj == save_equipment[wear][x] )
			{
				wear_loc = wear;
				break;
			}
			else if( !save_equipment[wear][x] )
				break;
	if( wear_loc != -1 )
		fprintf( fp, "WearLoc      %d\n", wear_loc );
	if( obj->item_type != obj->pIndexData->item_type )
		fprintf( fp, "ItemType     %d\n", obj->item_type );
	if( obj->weight != obj->pIndexData->weight )
		fprintf( fp, "Weight       %d\n", obj->weight );
	if( obj->level )
		fprintf( fp, "Level        %d\n", obj->level );
	if( obj->timer )
		fprintf( fp, "Timer        %d\n", obj->timer );
	if( obj->cost != obj->pIndexData->cost )
		fprintf( fp, "Cost         %d\n", obj->cost );
	if( obj->value[0] || obj->value[1] || obj->value[2] || obj->value[3] || obj->value[4] || obj->value[5] )
		fprintf( fp, "Values       %d %d %d %d %d %d\n",
			obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

	switch( obj->item_type )
	{
	case ITEM_PILL:  /* was down there with staff and wand, wrongly - Scryn */
	case ITEM_POTION:
		if( IS_VALID_SN( obj->value[1] ) )
			fprintf( fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name );

		if( IS_VALID_SN( obj->value[2] ) )
			fprintf( fp, "Spell 2      '%s'\n", skill_table[obj->value[2]]->name );

		if( IS_VALID_SN( obj->value[3] ) )
			fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

		break;

	case ITEM_DEVICE:
		if( IS_VALID_SN( obj->value[3] ) )
			fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

		break;
	case ITEM_SALVE:
		if( IS_VALID_SN( obj->value[4] ) )
			fprintf( fp, "Spell 4      '%s'\n", skill_table[obj->value[4]]->name );

		if( IS_VALID_SN( obj->value[5] ) )
			fprintf( fp, "Spell 5      '%s'\n", skill_table[obj->value[5]]->name );
		break;
	}

	for( paf = obj->first_affect; paf; paf = paf->next )
	{
		/*
		 * Save extra object affects                -Thoric
		 */
		if( paf->type < 0 || paf->type >= top_sn )
		{
			fprintf( fp, "Affect       %d %d %d %d %d\n",
				paf->type,
				paf->duration,
				( ( paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL
					|| paf->location == APPLY_REMOVESPELL
					|| paf->location == APPLY_STRIPSN )
					&& IS_VALID_SN( paf->modifier ) )
				? skill_table[paf->modifier]->slot : paf->modifier, paf->location, paf->bitvector );
		}
		else
			fprintf( fp, "AffectData   '%s' %d %d %d %d\n",
				skill_table[paf->type]->name,
				paf->duration,
				( ( paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL
					|| paf->location == APPLY_REMOVESPELL
					|| paf->location == APPLY_STRIPSN )
					&& IS_VALID_SN( paf->modifier ) )
				? skill_table[paf->modifier]->slot : paf->modifier, paf->location, paf->bitvector );
	}

	for( ed = obj->first_extradesc; ed; ed = ed->next )
		fprintf( fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description );

	fprintf( fp, "End\n\n" );

	if( obj->first_content )
		fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY, hotboot );

	return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, const char *name, bool preload, bool copyover )
{
	char strsave[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	FILE *fp;
	bool found;
	struct stat fst;
	int i, x;
	extern FILE *fpArea;
	extern char strArea[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	CREATE( ch, CHAR_DATA, 1 );
	for( x = 0; x < MAX_WEAR; x++ )
	{
		for( i = 0; i < MAX_LAYERS; i++ )
			save_equipment[x][i] = NULL;
	}
	clear_char( ch );
	loading_char = ch;

	CREATE( ch->pcdata, PC_DATA, 1 );
	d->character = ch;
	ch->on = NULL;
	ch->desc = d;
	ch->name = STRALLOC( name );
	ch->act = multimeb( PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1 );
	ch->pcdata->board = &boards[DEFAULT_BOARD];
	ch->pcdata->confirm_delete = false;
	ch->perm_str = 10;
	ch->perm_int = 10;
	ch->perm_wis = 10;
	ch->perm_dex = 10;
	ch->perm_con = 10;
	ch->perm_cha = 10;
	ch->perm_lck = 10;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL] = 48;
	ch->pcdata->condition[COND_BLOODTHIRST] = 10;
	ch->pcdata->wizinvis = 0;
	ch->mental_state = -10;
	ch->mobinvis = 0;
	for( i = 0; i < MAX_SKILL; i++ )
		ch->pcdata->learned[i] = 0;
	ch->pcdata->release_date = 0;
	ch->pcdata->helled_by = NULL;
	ch->saving_poison_death = 0;
	ch->saving_wand = 0;
	ch->saving_para_petri = 0;
	ch->saving_breath = 0;
	ch->pcdata->storagetimer = 0;
	ch->pcdata->storagecost = 0;
	ch->pcdata->icq = 0;
	ch->pcdata->lasthost = STRALLOC( "Unknown" );
	ch->saving_spell_staff = 0;
	ch->comments = NULL; /* comments */
	ch->pcdata->pagerlen = 24;
	ch->pcdata->first_ignored = NULL;    /* Ignore list */
	ch->pcdata->last_ignored = NULL;
	ch->mob_clan = STRALLOC( "" );
	ch->was_sentinel = NULL;
	ch->plr_home = NULL;
	ch->pcdata->lt_index = 0;    /* last tell index */
	ch->pcdata->hotboot = false;
	found = false;
	sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
	if( stat( strsave, &fst ) != -1 )
	{
		if( fst.st_size == 0 )
		{
			sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower( name[0] ), capitalize( name ) );
			send_to_char( "Restoring your backup player file...", ch );
		}
		else
		{
			sprintf( buf, "%s Pdata for: %s (%dK)  From: %s",
				preload ? "Preloading" : "Loading", ch->name, ( int ) fst.st_size / 1024, d->host );
			log_string_plus( buf, LOG_COMM, LEVEL_LIAISON );
		}
	}
	/*
	 * else no player file
	 */

	if( ( fp = FileOpen( strsave, "r" ) ) != NULL )
	{
		int iNest;

		for( iNest = 0; iNest < MAX_NEST; iNest++ )
			rgObjNest[iNest] = NULL;

		found = true;
		/*
		 * Cheat so that bug will show line #'s -- Altrag
		 */
		fpArea = fp;
		strcpy( strArea, strsave );
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
				bug( "Load_char_obj: # not found.", 0 );
				bug( name, 0 );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "PLAYER" ) )
			{
				fread_char( ch, fp, preload, copyover );
				if( preload )
					break;
			}
			else if( !str_cmp( word, "OBJECT" ) )  /* Objects  */
				fread_obj( ch, fp, OS_CARRY );
			else if( !str_cmp( word, "COMMENT" ) )
				fread_comment( ch, fp );    /* Comments */
			else if( !str_cmp( word, "END" ) ) /* Done     */
				break;
			else
			{
				bug( "Load_char_obj: bad section.", 0 );
				bug( name, 0 );
				break;
			}
		}
		FileClose( fp );
		fpArea = NULL;
		strcpy( strArea, "$" );
	}


	if( !found )
	{
		ch->short_descr = STRALLOC( "" );
		ch->long_descr = STRALLOC( "" );
		ch->description = STRALLOC( "" );
		ch->editor = NULL;
		ch->nextquest = 0;
		ch->pcdata->afkmess = str_dup( " is currently AFK." );
		ch->questpoints = 0;
		ch->pcdata->clan_name = STRALLOC( "" );
		ch->pcdata->clan = NULL;
		ch->pcdata->pwd = str_dup( "" );
		ch->pcdata->email = str_dup( "" );
		ch->pcdata->msn = str_dup( "" );
		ch->pcdata->aim = str_dup( "" );
		ch->pcdata->yim = str_dup( "" );
		ch->pcdata->real = str_dup( "" );
		ch->pcdata->enter = str_dup( "" );
		ch->pcdata->job = str_dup( "" );
		ch->pcdata->exit = str_dup( "" );
		ch->pcdata->icq = 0;
		ch->pcdata->tells = 0;
		ch->pcdata->specialweapon = 0;
		ch->pcdata->hlevel = 0;
		ch->pcdata->hkills = 0;
		ch->pcdata->tellbuf = str_dup( "" );
		ch->pcdata->bamfin = str_dup( "" );
		ch->pcdata->bamfout = str_dup( "" );
		ch->pcdata->bestowments = str_dup( "" );
		ch->pcdata->title = STRALLOC( "" );
		ch->pcdata->ragemeter = 0;
		ch->pcdata->propose = NULL;
		ch->pcdata->homepage = str_dup( "" );
		ch->pcdata->iclan = STRALLOC( "" );
		ch->pcdata->bio = STRALLOC( "" );
		ch->pcdata->authed_by = STRALLOC( "" );
		if( !ch->mailbuf )
			ch->mailbuf = STRALLOC( "" );
		ch->pcdata->prompt = STRALLOC( "" );
		ch->pcdata->low_vnum = 0;
		ch->pcdata->hi_vnum = 0;
		ch->pcdata->wizinvis = 0;
		ch->pcdata->partner = NULL;
		ch->on = NULL;
		ch->pcdata->wanted_flags = 0;
	}
	else
	{
		ch->on = NULL;
		if( !ch->pcdata->clan_name )
		{
			ch->pcdata->clan_name = STRALLOC( "" );
			ch->pcdata->clan = NULL;
		}
		if( !ch->pcdata->bio )
			ch->pcdata->bio = STRALLOC( "" );

		if( !ch->pcdata->authed_by )
			ch->pcdata->authed_by = STRALLOC( "" );

		/*      if( IS_IMMORTAL( ch ) )
			  {
				 if( ch->pcdata->wizinvis < ch->top_level )
					ch->pcdata->wizinvis = ch->top_level;
			  }*/

		if( IS_SET( ch->pcdata->cybaflags, CYBA_ISCHARGING ) )
			REMOVE_BIT( ch->pcdata->cybaflags, CYBA_ISCHARGING );

		ch->pcdata->specialweapon = 0;

		if( !IS_NPC( ch ) && get_trust( ch ) > LEVEL_AVATAR )
		{
			if( ch->pcdata->wizinvis < 2 )
				ch->pcdata->wizinvis = ch->top_level;
			assign_area( ch );
		}
		if( file_ver > 1 )
		{
			for( i = 0; i < MAX_WEAR; i++ )
				for( x = 0; x < MAX_LAYERS; x++ )
					if( save_equipment[i][x] )
					{
						equip_char( ch, save_equipment[i][x], i );
						save_equipment[i][x] = NULL;
					}
					else
						break;
		}
	}
	//    update_aris(ch);       
	loading_char = NULL;
	return found;
}



/*
 * Read in a char.
 */

void fread_char( CHAR_DATA *ch, FILE *fp, bool preload, bool copyover )
{
	char buf[MAX_STRING_LENGTH];
	const char *line;
	const char *word;
	int x1, x2, x3, x4, x5, x6, x7, x8, x9, x0;
	short killcnt;
	bool fMatch;
	int max_colors = 0;  /* Color code */
	time_t lastplayed;
	int sn, extra;

	file_ver = 0;
	killcnt = 0;
	for( ;; )
	{
		word = ( feof( fp ) ? "End" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			bug( "%s: EOF encountered reading file!", __FUNCTION__ );
			word = "End";
		}
		fMatch = false;

		switch( UPPER( word[0] ) )
		{
		case '*':
			fMatch = true;
			fread_to_eol( fp );
			break;

		case 'A':
			KEY( "Act", ch->act, fread_bitvector( fp ) );
			KEY( "Aim", ch->pcdata->aim, fread_string_nohash( fp ) );
			KEY( "AffectedBy", ch->affected_by, fread_number( fp ) );
			KEY( "Alignment", ch->alignment, fread_number( fp ) );
			DUMMYREAD( "AP" );
			KEY( "APDeaths", ch->pcdata->apdeaths, fread_number( fp ) );
			KEY( "APKills", ch->pcdata->apkills, fread_number( fp ) );
			KEY( "Armor", ch->armor, fread_number( fp ) );
			KEY( "Avatar", ch->pcdata->avatar, fread_string( fp ) );

			if( !str_cmp( word, "Addiction" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				ch->pcdata->addiction[0] = x0;
				ch->pcdata->addiction[1] = x1;
				ch->pcdata->addiction[2] = x2;
				ch->pcdata->addiction[3] = x3;
				ch->pcdata->addiction[4] = x4;
				ch->pcdata->addiction[5] = x5;
				ch->pcdata->addiction[6] = x6;
				ch->pcdata->addiction[7] = x7;
				ch->pcdata->addiction[8] = x8;
				ch->pcdata->addiction[9] = x9;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Ability" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = 0;
				sscanf( line, "%d %d %d", &x0, &x1, &x2 );
				if( x0 >= 0 && x0 < MAX_ABILITY )
				{
					ch->skill_level[x0] = x1;
					ch->experience[x0] = x2;
				}
				fMatch = true;
				break;
			}


			if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
			{
				AFFECT_DATA *paf;

				if( preload )
				{
					fMatch = true;
					fread_to_eol( fp );
					break;
				}
				CREATE( paf, AFFECT_DATA, 1 );
				if( !str_cmp( word, "Affect" ) )
				{
					paf->type = fread_number( fp );
				}
				else
				{
					const char *sname = fread_word( fp );

					if( ( sn = skill_lookup( sname ) ) < 0 )
					{
						if( ( sn = herb_lookup( sname ) ) < 0 )
							bug( "Fread_char: unknown skill.", 0 );
						else
							sn += TYPE_HERB;
					}
					paf->type = sn;
				}

				paf->duration = fread_number( fp );
				paf->modifier = fread_number( fp );
				paf->location = fread_number( fp );
				paf->bitvector = fread_number( fp );
				LINK( paf, ch->first_affect, ch->last_affect, next, prev );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "AttrMod" ) )
			{
				line = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
				sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
				ch->mod_str = x1;
				ch->mod_int = x2;
				ch->mod_wis = x3;
				ch->mod_dex = x4;
				ch->mod_con = x5;
				ch->mod_cha = x6;
				ch->mod_lck = x7;
				if( !x7 )
					ch->mod_lck = 0;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Aliases" ) )
			{
				/*		    if ( file_ver < 7 )
							{
							fread_to_eol( fp );
							fMatch = true;
							break;
							}
							else
							{
				*/ ALIAS_DATA *pal;

				if( preload )
				{
					fMatch = true;
					fread_to_eol( fp );
					break;
				}
				CREATE( pal, ALIAS_DATA, 1 );

				pal->name = fread_string_nohash( fp );
				pal->cmd = fread_string_nohash( fp );
				LINK( pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
				fMatch = true;
				break;
				//          }
			}

			if( !str_cmp( word, "AttrPerm" ) )
			{
				line = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
				sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
				ch->perm_str = x1;
				ch->perm_int = x2;
				ch->perm_wis = x3;
				ch->perm_dex = x4;
				ch->perm_con = x5;
				ch->perm_cha = x6;
				ch->perm_lck = x7;
				if( !x7 || x7 == 0 )
					ch->perm_lck = 13;
				fMatch = true;
				break;
			}
			DUMMYREAD( "Atk" );
			DUMMYREAD( "AtkMax" );
			DUMMYREAD( "Attribute" );
			KEY( "AuthedBy", ch->pcdata->authed_by, fread_string( fp ) );
			DUMMYREAD( "Accuracy" );
			DUMMYREAD( "AccuracyMax" );
			DUMMYREAD( "Agility" );
			DUMMYREAD( "AgilityMax" );
			DUMMYREAD( "Apf" );
			DUMMYREAD( "ApMax" );

			break;

		case 'B':
			KEY( "Bamfin", ch->pcdata->bamfin, fread_string_nohash( fp ) );
			KEY( "Bamfout", ch->pcdata->bamfout, fread_string_nohash( fp ) );
			KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
			KEY( "Bio", ch->pcdata->bio, fread_string( fp ) );
			KEY( "Bank", ch->pcdata->bank, fread_number( fp ) );
			KEY( "Beenfroze", ch->pcdata->beenfroze, fread_number( fp ) );
			KEY( "Build", ch->pcdata->build, fread_number( fp ) );
			if( !str_cmp( word, "BodyParts" ) )
				break;

			/*
			 * Read in board status
			 */
			if( !str_cmp( word, "Boards" ) )
			{
				int i, num = fread_number( fp );
				char *boardname;

				for( ; num; num-- )
				{
					boardname = fread_word( fp );
					i = board_lookup( boardname );

					if( i == BOARD_NOTFOUND )
					{
						sprintf( buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname );
						log_string( buf );
						fread_number( fp );
					}
					else
						ch->pcdata->last_note[i] = fread_number( fp );
				}

				fMatch = true;
			}

			break;

		case 'C':
			KEY( "CRank", ch->pcdata->clan_rank, fread_number( fp ) );
			KEY( "Cybaflags", ch->pcdata->cybaflags, fread_number( fp ) );
			if( !str_cmp( word, "Clan" ) )
			{
				ch->pcdata->clan_name = fread_string( fp );

				if( !preload
					&& ch->pcdata->clan_name[0] != '\0' && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
				{
					sprintf( buf,
						"Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\r\n",
						ch->pcdata->clan_name );
					send_to_char( buf, ch );
					STRFREE( ch->pcdata->clan_name );
					ch->pcdata->clan_name = STRALLOC( "" );
				}
				else
					fMatch = true;
				break;
			}

			if( !str_cmp( word, "Colors" ) )
			{
				int x;

				for( x = 0; x < max_colors; x++ )
					ch->colors[x] = fread_number( fp );
				fread_to_eol( fp );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Condition" ) )
			{
				line = fread_line( fp );
				sscanf( line, "%d %d %d %d", &x1, &x2, &x3, &x4 );
				ch->pcdata->condition[0] = x1;
				ch->pcdata->condition[1] = x2;
				ch->pcdata->condition[2] = x3;
				ch->pcdata->condition[3] = x4;
				fMatch = true;
				break;
			}
			break;

		case 'D':
			KEY( "Damroll", ch->damroll, fread_number( fp ) );
			KEY( "Dat", ch->pcdata->dating, fread_number( fp ) );
			KEY( "Deaf", ch->deaf, fread_number( fp ) );
			KEY( "Description", ch->description, fread_string( fp ) );
			if( !str_cmp( word, "Druglevel" ) )
			{
				line = fread_line( fp );
				x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
				sscanf( line, "%d %d %d %d %d %d %d %d %d %d", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );
				ch->pcdata->drug_level[0] = x0;
				ch->pcdata->drug_level[1] = x1;
				ch->pcdata->drug_level[2] = x2;
				ch->pcdata->drug_level[3] = x3;
				ch->pcdata->drug_level[4] = x4;
				ch->pcdata->drug_level[5] = x5;
				ch->pcdata->drug_level[6] = x6;
				ch->pcdata->drug_level[7] = x7;
				ch->pcdata->drug_level[8] = x8;
				ch->pcdata->drug_level[9] = x9;
				fMatch = true;
				break;
			}
			break;

			/*
			 * 'E' was moved to after 'S'
			 */
		case 'F':
			KEY( "Flags", ch->pcdata->flags, fread_number( fp ) );
			if( !str_cmp( word, "Force" ) )
			{
				fMatch = true;
				break;
			}
			DUMMYREAD( "Fstate" );
			break;

		case 'G':
			KEY( "Glory", ch->pcdata->quest_curr, fread_number( fp ) );
			KEY( "Gold", ch->gold, fread_number( fp ) );
			/*
			 * temporary measure
			 */
			if( !str_cmp( word, "Guild" ) )
			{
				ch->pcdata->clan_name = fread_string( fp );

				if( !preload
					&& ch->pcdata->clan_name[0] != '\0' && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
				{
					sprintf( buf,
						"Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\r\n",
						ch->pcdata->clan_name );
					send_to_char( buf, ch );
					STRFREE( ch->pcdata->clan_name );
					ch->pcdata->clan_name = STRALLOC( "" );
				}
				fMatch = true;
				break;
			}
			break;

		case 'H':
			KEY( "Hair", ch->pcdata->hair, fread_number( fp ) );
			KEY( "Hasfroze", ch->pcdata->hasfroze, fread_number( fp ) );
			KEY( "Hero", ch->pcdata->hero, fread_number( fp ) );
			KEY( "Highlight", ch->pcdata->highlight, fread_number( fp ) );
			if( !str_cmp( word, "Helled" ) )
			{
				ch->pcdata->release_date = fread_number( fp );
				ch->pcdata->helled_by = fread_string( fp );
				if( ch->pcdata->release_date < current_time )
				{
					STRFREE( ch->pcdata->helled_by );
					ch->pcdata->helled_by = NULL;
					ch->pcdata->release_date = 0;
				}
				fMatch = true;
				break;
			}

			KEY( "Hitroll", ch->hitroll, fread_number( fp ) );
			KEY( "HLevel", ch->pcdata->hlevel, fread_number( fp ) );
			KEY( "HKills", ch->pcdata->hkills, fread_number( fp ) );
			KEY( "Homepage", ch->pcdata->homepage, fread_string_nohash( fp ) );

			if( !str_cmp( word, "HpManaMove" ) )
			{
				line = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = 0;
				sscanf( line, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
				ch->hit = x1;
				ch->max_hit = x2;
				ch->move = x5;
				ch->max_move = x6;
				fMatch = true;
				break;
			}

			break;

		case 'I':
			KEY( "ICQ", ch->pcdata->icq, fread_number( fp ) );
			KEY( "Iclan", ch->pcdata->iclan, fread_string( fp ) );
			KEY( "Inivictim", ch->pcdata->inivictim, fread_string( fp ) );
			KEY( "IllegalPK", ch->pcdata->illegal_pk, fread_number( fp ) );
			KEY( "Immune", ch->immune, fread_number( fp ) );

			if( !strcmp( word, "Ignored" ) )
			{
				const char *temp;
				char fname[1024];
				struct stat fst;
				int ign;
				IGNORE_DATA *inode;

				/*
				 * Get the name
				 */
				temp = fread_string( fp );

				sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( temp[0] ), capitalize( temp ) );

				/*
				 * If there isn't a pfile for the name
				 */
				 /*
				  * then don't add it to the list
				  */
				if( stat( fname, &fst ) == -1 )
				{
					fMatch = true;
					break;
				}

				/*
				 * Count the number of names already ignored
				 */
				for( ign = 0, inode = ch->pcdata->first_ignored; inode; inode = inode->next )
				{
					ign++;
				}

				/*
				 * Add the name unless the limit has been reached
				 */
				if( ign >= sysdata.maxign )
				{
					bug( "fread_char: too many ignored names" );
				}
				else
				{
					/*
					 * Add the name to the list
					 */
					CREATE( inode, IGNORE_DATA, 1 );
					inode->name = STRALLOC( temp );
					inode->next = NULL;
					inode->prev = NULL;

					LINK( inode, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
				}

				fMatch = true;

				break;

		case 'J':
			KEY( "Job", ch->pcdata->job, fread_string_nohash( fp ) );
			break;

		case 'K':
			if( !str_cmp( word, "Killed" ) )
			{
				fMatch = true;
				if( killcnt >= MAX_KILLTRACK )
					bug( "fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt );
				else
				{
					ch->pcdata->killed[killcnt].vnum = fread_number( fp );
					ch->pcdata->killed[killcnt++].count = fread_number( fp );
				}
			}
			break;

		case 'L':

			DUMMYREAD( "LoDef" );
			DUMMYREAD( "LoDefMax" );
			KEY( "LP", ch->pcdata->lp, fread_number( fp ) );
			if( !str_cmp( word, "Lastplayed" ) )
			{
				lastplayed = fread_number( fp );
				fMatch = true;
				break;
			}
			KEY( "LongDescr", ch->long_descr, fread_string( fp ) );
			break;

		case 'M':
			KEY( "MainAbility", ch->main_ability, fread_number( fp ) );
			KEY( "MDeaths", ch->pcdata->mdeaths, fread_number( fp ) );
			KEY( "Mentalstate", ch->mental_state, fread_number( fp ) );
			KEY( "MGlory", ch->pcdata->quest_accum, fread_number( fp ) );
			KEY( "Minsnoop", ch->pcdata->min_snoop, fread_number( fp ) );
			KEY( "MSN", ch->pcdata->msn, fread_string_nohash( fp ) );
			KEY( "MKills", ch->pcdata->mkills, fread_number( fp ) );
			KEY( "Mobinvis", ch->mobinvis, fread_number( fp ) );
			if( !str_cmp( word, "MaxColors" ) )
			{
				int tempNum = fread_number( fp );

				max_colors = UMIN( tempNum, MAX_COLORS );

				fMatch = true;
				break;
			}
			break;

		case 'N':
			KEY( "Nextquest", ch->nextquest, fread_number( fp ) );
			if( !str_cmp( word, "Name" ) )
			{
				/*
				 * Name already set externally.
				 */
				fread_to_eol( fp );
				fMatch = true;
				break;
			}
			break;

		case 'O':
			KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
			break;

		case 'P':
			KEY( "Pagerlen", ch->pcdata->pagerlen, fread_number( fp ) );
			KEY( "Password", ch->pcdata->pwd, fread_string_nohash( fp ) );
			KEY( "PDeaths", ch->pcdata->pdeaths, fread_number( fp ) );
			KEY( "PKills", ch->pcdata->pkills, fread_number( fp ) );
			KEY( "Played", ch->played, fread_number( fp ) );
			KEY( "Position", ch->position, fread_number( fp ) );
			/*
						if (!strcmp ( word, "Position" ) )
						{
						   ch->position          = fread_number( fp );
						   if(ch->position<100){
							  switch(ch->position){
								  default: ;
								  case 0: ;
								  case 1: ;
								  case 2: ;
								  case 3: ;
								  case 4: break;
								  case 5: ch->position=6; break;
								  case 6: ch->position=8; break;
								  case 7: ch->position=9; break;
								  case 8: ch->position=12; break;
								  case 9: ch->position=13; break;
								  case 10: ch->position=14; break;
								  case 11: ch->position=15; break;
							  }
							  fMatch = true;
						   } else {
							  ch->position-=100;
							  fMatch = true;
						   }
						}
			*/
			KEY( "Practice", extra, fread_number( fp ) );
			KEY( "Prompt", ch->pcdata->prompt, fread_string( fp ) );
			if( !str_cmp( word, "PTimer" ) )
			{
				add_timer( ch, TIMER_PKILLED, fread_number( fp ), NULL, 0 );
				fMatch = true;
				break;
			}
			if( !str_cmp( word, "PlrHome" ) )
			{
				ch->plr_home = get_room_index( fread_number( fp ) );
				if( !ch->plr_home )
					ch->plr_home = NULL;
				fMatch = true;
				break;
			}
			break;

		case 'Q':
			KEY( "Questpoints", ch->questpoints, fread_number( fp ) );
			if( !str_cmp( word, "Qbit" ) )
			{
				BIT_DATA *bit;
				BIT_DATA *desc;

				CREATE( bit, BIT_DATA, 1 );

				bit->number = fread_number( fp );
				if( ( desc = find_qbit( bit->number ) ) == NULL )
					strcpy( bit->desc, fread_string( fp ) );
				else
				{
					strcpy( bit->desc, desc->desc );
					fread_string( fp );
				}
				LINK( bit, ch->pcdata->firstqbit, ch->pcdata->lastqbit, next, prev );
				fMatch = true;
			}
			break;

		case 'R':
			KEY( "Race", ch->race, fread_number( fp ) );
			KEY( "Ragemeter", ch->pcdata->ragemeter, fread_number( fp ) );
			KEY( "Rank", ch->pcdata->rank, fread_string( fp ) );
			KEY( "Real", ch->pcdata->real, fread_string_nohash( fp ) );
			KEY( "Resistant", ch->resistant, fread_number( fp ) );
			KEY( "Restore_time", ch->pcdata->restore_time, fread_number( fp ) );

			if( !str_cmp( word, "Room" ) )
			{
				ch->in_room = get_room_index( fread_number( fp ) );
				if( !ch->in_room )
					ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
				fMatch = true;
				break;
			}
			if( !str_cmp( word, "RoomRange" ) )
			{
				ch->pcdata->low_vnum = fread_number( fp );
				ch->pcdata->hi_vnum = fread_number( fp );
				fMatch = true;
			}
			KEY( "Rtm", ch->pcdata->rtime, fread_number( fp ) );
			break;

		case 'S':
			KEY( "Salary", ch->pcdata->salary, fread_number( fp ) );
			KEY( "Salary_time", ch->pcdata->salary_date, fread_number( fp ) );
			KEY( "Sex", ch->sex, fread_number( fp ) );
			KEY( "ShortDescr", ch->short_descr, fread_string( fp ) );
			KEY( "SpecialWeapon", ch->pcdata->specialweapon, fread_number( fp ) );
			KEY( "Spou", ch->pcdata->spouse, fread_string( fp ) );
			KEY( "Storagecost", ch->pcdata->storagecost, fread_number( fp ) );
			KEY( "Storagetimer", ch->pcdata->storagetimer, fread_number( fp ) );
			DUMMYREAD( "Style" );
			KEY( "Susceptible", ch->susceptible, fread_number( fp ) );
			DUMMYREAD( "Speed" );
			DUMMYREAD( "SpeedMax" );
			if( !str_cmp( word, "SavingThrow" ) )
			{
				ch->saving_wand = fread_number( fp );
				ch->saving_poison_death = ch->saving_wand;
				ch->saving_para_petri = ch->saving_wand;
				ch->saving_breath = ch->saving_wand;
				ch->saving_spell_staff = ch->saving_wand;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "SavingThrows" ) )
			{
				ch->saving_poison_death = fread_number( fp );
				ch->saving_wand = fread_number( fp );
				ch->saving_para_petri = fread_number( fp );
				ch->saving_breath = fread_number( fp );
				ch->saving_spell_staff = fread_number( fp );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Site" ) )
			{
				if( !preload && !copyover )
				{
					sprintf( buf, "&CL&cast &PC&ponnected &BF&brom&R:&w %s\r\n", fread_word( fp ) );
					send_to_char( buf, ch );
				}
				else
					fread_to_eol( fp );
				fMatch = true;
				if( preload )
					word = "End";
				else
					break;
			}

			if( !str_cmp( word, "Skill" ) )
			{
				int value;

				if( preload )
					word = "End";
				else
				{
					value = fread_number( fp );
					if( file_ver < 3 )
						sn = skill_lookup( fread_word( fp ) );
					else
						sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_first_weapon - 1 );
					if( sn < 0 )
						bug( "Fread_char: unknown skill.", 0 );
					else
					{
						ch->pcdata->learned[sn] = value;

					}
					fMatch = true;
					break;
				}
			}

			if( !str_cmp( word, "Spell" ) )
			{
				int value;

				if( preload )
					word = "End";
				else
				{
					value = fread_number( fp );

					sn = bsearch_skill_exact( fread_word( fp ), gsn_first_spell, gsn_first_skill - 1 );
					if( sn < 0 )
						bug( "Fread_char: unknown spell.", 0 );
					else
					{
						ch->pcdata->learned[sn] = value;

					}
					fMatch = true;
					break;
				}
			}
			if( str_cmp( word, "End" ) )
				break;

		case 'E':
			KEY( "Eng", ch->pcdata->engaged, fread_number( fp ) );
			KEY( "Enter", ch->pcdata->enter, fread_string_nohash( fp ) );
			KEY( "Energy", ch->pcdata->xenergy, fread_number( fp ) );
			KEY( "Energymax", ch->pcdata->xenergymax, fread_number( fp ) );
			KEY( "Exit", ch->pcdata->exit, fread_string_nohash( fp ) );
			KEY( "Eye", ch->pcdata->eye, fread_number( fp ) );
			if( !str_cmp( word, "End" ) )
			{
				if( !ch->short_descr )
					ch->short_descr = STRALLOC( "" );
				if( !ch->long_descr )
					ch->long_descr = STRALLOC( "" );
				if( !ch->description )
					ch->description = STRALLOC( "" );
				if( !ch->pcdata->pwd )
					ch->pcdata->pwd = str_dup( "" );
				if( !ch->pcdata->email )
					ch->pcdata->email = str_dup( "" );
				if( !ch->pcdata->msn )
					ch->pcdata->msn = str_dup( "" );
				if( !ch->pcdata->real )
					ch->pcdata->real = str_dup( "" );
				if( !ch->pcdata->yim )
					ch->pcdata->yim = str_dup( "" );
				if( !ch->pcdata->exit )
					ch->pcdata->exit = str_dup( "" );
				if( !ch->pcdata->enter )
					ch->pcdata->enter = str_dup( "" );
				if( !ch->pcdata->aim )
					ch->pcdata->aim = str_dup( "" );
				if( !ch->pcdata->bamfin )
					ch->pcdata->bamfin = str_dup( "" );
				if( !ch->pcdata->bamfout )
					ch->pcdata->bamfout = str_dup( "" );
				if( !ch->pcdata->bio )
					ch->pcdata->bio = STRALLOC( "" );
				if( !ch->pcdata->rank )
					ch->pcdata->rank = STRALLOC( "" );
				if( !ch->pcdata->bestowments )
					ch->pcdata->bestowments = str_dup( "" );
				if( !ch->pcdata->title )
					ch->pcdata->title = STRALLOC( "" );
				if( !ch->pcdata->homepage )
					ch->pcdata->homepage = str_dup( "" );
				if( !ch->pcdata->authed_by )
					ch->pcdata->authed_by = STRALLOC( "" );
				if( !ch->pcdata->prompt )
					ch->pcdata->prompt = STRALLOC( "" );
				ch->editor = NULL;
				killcnt = URANGE( 2, ( ( ch->top_level + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
				if( killcnt < MAX_KILLTRACK )
					ch->pcdata->killed[killcnt].vnum = 0;
				{
					int ability;
					for( ability = 0; ability < MAX_ABILITY; ability++ )
					{
						if( ch->skill_level[ability] == 0 )
							ch->skill_level[ability] = 1;
					}
				}
				if( !ch->pcdata->prompt )
					ch->pcdata->prompt = STRALLOC( "" );

				if( lastplayed != 0 )
				{
					int hitgain;
					hitgain = ( ( int ) ( current_time - lastplayed ) / 60 );
					ch->hit = URANGE( 1, ch->hit + hitgain, ch->max_hit );
					ch->move = URANGE( 1, ch->move + hitgain, ch->max_move );
					better_mental_state( ch, hitgain );
				}
				for( sn = 0; sn < top_sn; sn++ )
				{
					if( !skill_table[sn]->name )
						break;

					if( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
						continue;

					if( ch->pcdata->learned[sn] > 0
						&& ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
					{
						if( skill_table[sn]->guild != ESPIONAGE_ABILITY )
						{
							ch->pcdata->learned[sn] = 0;
						}
					}
				}
				return;
			}
			KEY( "Email", ch->pcdata->email, fread_string_nohash( fp ) );
			break;

		case 'T':
			KEY( "TSpouse", ch->pcdata->tspouse, fread_string( fp ) );
			KEY( "TicketNumber", ch->pcdata->ticketnumber, fread_number( fp ) );
			KEY( "TicketWeek", ch->pcdata->ticketweek, fread_number( fp ) );
			KEY( "Toplevel", ch->top_level, fread_number( fp ) );
			if( !str_cmp( word, "Tongue" ) )
			{
				int value;

				if( preload )
					word = "End";
				else
				{
					value = fread_number( fp );

					sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, gsn_top_sn - 1 );
					if( sn < 0 )
						bug( "Fread_char: unknown tongue.", 0 );
					else
					{
						ch->pcdata->learned[sn] = value;

					}
					fMatch = true;
				}
				break;
			}
			KEY( "Trust", ch->trust, fread_number( fp ) );
			/*
			 * Let no character be trusted higher than one below maxlevel -- Narn
			 */
			 //      ch->trust = UMIN( ch->trust, MAX_LEVEL - 1 );

			if( !str_cmp( word, "Title" ) )
			{
				ch->pcdata->title = fread_string( fp );
				if( isalpha( ch->pcdata->title[0] ) || isdigit( ch->pcdata->title[0] ) )
				{
					sprintf( buf, " %s", ch->pcdata->title );
					if( ch->pcdata->title )
						STRFREE( ch->pcdata->title );
					ch->pcdata->title = STRALLOC( buf );
				}
				fMatch = true;
				break;
			}

			break;

		case 'U':
			DUMMYREAD( "UpDef" );
			DUMMYREAD( "UpDefMax" );
			break;

		case 'V':
			if( !str_cmp( word, "Version" ) )
			{
				file_ver = fread_number( fp );
				ch->pcdata->version = file_ver;
				fMatch = true;
				break;
			}
			if( !str_cmp( word, "VnumRange" ) )
			{
				ch->pcdata->low_vnum = fread_number( fp );
				ch->pcdata->hi_vnum = fread_number( fp );
				fMatch = true;
			}
			break;

		case 'W':
			if( !str_cmp( word, "Weapon" ) )
			{
				int value;

				if( preload )
					word = "End";
				else
				{
					value = fread_number( fp );

					sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, gsn_first_tongue - 1 );
					if( sn < 0 )
						bug( "Fread_char: unknown weapon.", 0 );
					else
					{
						ch->pcdata->learned[sn] = value;

					}
					fMatch = true;
				}
				break;
			}
			KEY( "WFMTimer", ch->wfm_timer, fread_number( fp ) );
			KEY( "Wimpy", ch->wimpy, fread_number( fp ) );
			KEY( "WizInvis", ch->pcdata->wizinvis, fread_number( fp ) );
			KEY( "Wanted", ch->pcdata->wanted_flags, fread_number( fp ) );
			break;
			}

		case 'Y':
			KEY( "YIM", ch->pcdata->yim, fread_string_nohash( fp ) );
			break;

			if( !fMatch )
			{
				sprintf( buf, "Fread_char: no match: %s", word );
				bug( buf, 0 );
			}
		}
	}
}


void fread_obj( CHAR_DATA *ch, FILE *fp, short os_type )
{
	OBJ_DATA *obj;
	const char *word;
	int iNest;
	bool fMatch;
	bool fNest;
	bool fVnum;
	ROOM_INDEX_DATA *room = NULL;

	if( ch )
	{
		room = ch->in_room;
		if( ch->tempnum == -9999 )
			file_ver = 0;
	}

	CREATE( obj, OBJ_DATA, 1 );
	obj->count = 1;
	obj->wear_loc = -1;
	obj->weight = 1;

	fNest = true;    /* Requiring a Nest 0 is a waste */
	fVnum = true;
	iNest = 0;

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
			if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
			{
				AFFECT_DATA *paf;
				int pafmod;

				CREATE( paf, AFFECT_DATA, 1 );
				if( !str_cmp( word, "Affect" ) )
				{
					paf->type = fread_number( fp );
				}
				else
				{
					int sn;

					sn = skill_lookup( fread_word( fp ) );
					if( sn < 0 )
						bug( "Fread_obj: unknown skill.", 0 );
					else
						paf->type = sn;
				}
				paf->duration = fread_number( fp );
				pafmod = fread_number( fp );
				paf->location = fread_number( fp );
				paf->bitvector = fread_number( fp );
				if( paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL || paf->location == APPLY_REMOVESPELL )
					paf->modifier = slot_lookup( pafmod );
				else
					paf->modifier = pafmod;
				LINK( paf, obj->first_affect, obj->last_affect, next, prev );
				fMatch = true;
				break;
			}
			KEY( "Actiondesc", obj->action_desc, fread_string( fp ) );
			break;

		case 'C':
			KEY( "Cost", obj->cost, fread_number( fp ) );
			KEY( "Count", obj->count, fread_number( fp ) );
			break;

		case 'D':
			KEY( "Description", obj->description, fread_string( fp ) );
			break;

		case 'E':
			KEY( "ExtraFlags", obj->extra_flags, fread_bitvector( fp ) );

			if( !str_cmp( word, "ExtraDescr" ) )
			{
				EXTRA_DESCR_DATA *ed;

				CREATE( ed, EXTRA_DESCR_DATA, 1 );
				ed->keyword = fread_string( fp );
				ed->description = fread_string( fp );
				LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
				fMatch = true;
			}

			if( !str_cmp( word, "End" ) )
			{
				if( !fNest || !fVnum || !obj->pIndexData )
				{
					bug( "Fread_obj: incomplete object.", 0 );
					free_obj( obj );
					return;
				}
				else
				{
					short wear_loc = obj->wear_loc;

					if( !obj->name )
						obj->name = QUICKLINK( obj->pIndexData->name );
					if( !obj->description )
						obj->description = QUICKLINK( obj->pIndexData->description );
					if( !obj->short_descr )
						obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
					if( !obj->action_desc )
						obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
					LINK( obj, first_object, last_object, next, prev );
					obj->pIndexData->count += obj->count;
					if( !obj->serial )
					{
						cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
						obj->serial = obj->pIndexData->serial = cur_obj_serial;
					}
					if( fNest )
						rgObjNest[iNest] = obj;
					numobjsloaded += obj->count;
					++physicalobjects;
					if( file_ver > 1 || obj->wear_loc < -1 || obj->wear_loc >= MAX_WEAR )
						obj->wear_loc = -1;
					/*
					 * Corpse saving. -- Altrag
					 */
					if( os_type == OS_CORPSE )
					{
						if( !room )
						{
							bug( "Fread_obj: Corpse without room", 0 );
							room = get_room_index( ROOM_VNUM_LIMBO );
						}
						obj = obj_to_room( obj, room );
					}
					else if( os_type == OS_LOCKER )
					{
						obj = obj_to_room( obj, ch->pcdata->locker_room );
					}
					else if( os_type == OS_GROUND )
					{
						if( !room )
						{
							bug( "Fread_obj: OS_GROUND vnum %d with no room Skipping!", obj->pIndexData->vnum );
						}
						else
							obj_to_room( obj, room );
					}
					else if( iNest == 0 || rgObjNest[iNest] == NULL )
					{
						int slot;
						bool reslot = false;

						if( file_ver > 1 && wear_loc > -1 && wear_loc < MAX_WEAR )
						{
							int x;

							for( x = 0; x < MAX_LAYERS; x++ )
								if( !save_equipment[wear_loc][x] )
								{
									save_equipment[wear_loc][x] = obj;
									slot = x;
									reslot = true;
									break;
								}
							if( x == MAX_LAYERS )
								bug( "Fread_obj: too many layers %d", wear_loc );
						}
						obj = obj_to_char( obj, ch );
						if( reslot )
							save_equipment[wear_loc][slot] = obj;
					}
					else
					{
						if( rgObjNest[iNest - 1] )
						{
							separate_obj( rgObjNest[iNest - 1] );
							obj = obj_to_obj( obj, rgObjNest[iNest - 1] );
						}
						else
							bug( "Fread_obj: nest layer missing %d", iNest - 1 );
					}
					if( fNest )
						rgObjNest[iNest] = obj;
					return;
				}
			}
			break;

		case 'I':
			KEY( "ItemType", obj->item_type, fread_number( fp ) );
			break;

		case 'L':
			KEY( "Level", obj->level, fread_number( fp ) );
			break;

		case 'N':
			KEY( "Name", obj->name, fread_string( fp ) );

			if( !str_cmp( word, "Nest" ) )
			{
				iNest = fread_number( fp );
				if( iNest < 0 || iNest >= MAX_NEST )
				{
					bug( "Fread_obj: bad nest %d.", iNest );
					iNest = 0;
					fNest = false;
				}
				fMatch = true;
			}
			break;

		case 'R':
			KEY( "Room", room, get_room_index( fread_number( fp ) ) );
			KEY( "Rvnum", obj->room_vnum, fread_number( fp ) );

		case 'S':
			KEY( "ShortDescr", obj->short_descr, fread_string( fp ) );

			if( !str_cmp( word, "Spell" ) )
			{
				int iValue;
				int sn;

				iValue = fread_number( fp );
				sn = skill_lookup( fread_word( fp ) );
				if( iValue < 0 || iValue > 5 )
					bug( "Fread_obj: bad iValue %d.", iValue );
				else if( sn < 0 )
					bug( "Fread_obj: unknown skill.", 0 );
				else
					obj->value[iValue] = sn;
				fMatch = true;
				break;
			}

			break;

		case 'T':
			KEY( "Timer", obj->timer, fread_number( fp ) );
			break;

		case 'V':
			if( !str_cmp( word, "Values" ) )
			{
				int x1, x2, x3, x4, x5, x6;
				char *ln = fread_line( fp );

				x1 = x2 = x3 = x4 = x5 = x6 = 0;
				sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
				/*
				 * clean up some garbage
				 */
				if( file_ver < 3 && os_type != OS_CORPSE )
					x5 = x6 = 0;

				obj->value[0] = x1;
				obj->value[1] = x2;
				obj->value[2] = x3;
				obj->value[3] = x4;
				obj->value[4] = x5;
				obj->value[5] = x6;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Vnum" ) )
			{
				int vnum;

				vnum = fread_number( fp );
				if( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
				{
					fVnum = false;
					bug( "Fread_obj: bad vnum %d.", vnum );
				}
				else
				{
					fVnum = true;
					obj->cost = obj->pIndexData->cost;
					obj->weight = obj->pIndexData->weight;
					obj->item_type = obj->pIndexData->item_type;
					obj->wear_flags = obj->pIndexData->wear_flags;
					obj->extra_flags = obj->pIndexData->extra_flags;
				}
				fMatch = true;
				break;
			}
			break;

		case 'W':
			KEY( "WearFlags", obj->wear_flags, fread_number( fp ) );
			KEY( "WearLoc", obj->wear_loc, fread_number( fp ) );
			KEY( "Weight", obj->weight, fread_number( fp ) );
			break;

		}

		if( !fMatch )
		{
			EXTRA_DESCR_DATA *ed;
			AFFECT_DATA *paf;

			bug( "Fread_obj: no match.", 0 );
			bug( word, 0 );
			fread_to_eol( fp );
			if( obj->name )
				STRFREE( obj->name );
			if( obj->description )
				STRFREE( obj->description );
			if( obj->short_descr )
				STRFREE( obj->short_descr );
			while( ( ed = obj->first_extradesc ) != NULL )
			{
				STRFREE( ed->keyword );
				STRFREE( ed->description );
				UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
				DISPOSE( ed );
			}
			while( ( paf = obj->first_affect ) != NULL )
			{
				UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
				DISPOSE( paf );
			}
			DISPOSE( obj );
			return;
		}
	}
}

void set_alarm( long seconds )
{
	alarm( seconds );
}

void write_corpses( CHAR_DATA *ch, const char *name )
{
	OBJ_DATA *corpse;
	FILE *fp = NULL;

	/*
	 * Name and ch support so that we dont have to have a char to save their
	 * corpses.. (ie: decayed corpses while offline)
	 */
	if( ch && IS_NPC( ch ) )
	{
		bug( "Write_corpses: writing NPC corpse.", 0 );
		return;
	}
	if( IS_IMMORTAL( ch ) )
	{
		return;
	}
	if( ch )
		name = ch->name;
	/*
	 * Go by vnum, less chance of screwups. -- Altrag
	 */
	for( corpse = first_object; corpse; corpse = corpse->next )
		if( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
			corpse->in_room != NULL && !str_cmp( corpse->short_descr + 14, name ) )
		{
			if( !fp )
			{
				char buf[256];

				sprintf( buf, "%s%s", CORPSE_DIR, capitalize( name ) );
				if( !( fp = FileOpen( buf, "w" ) ) )
				{
					bug( "Write_corpses: Cannot open file.", 0 );
					perror( buf );
					return;
				}
			}
			fwrite_obj( ch, corpse, fp, 0, OS_CORPSE, false );
		}
	if( fp )
	{
		fprintf( fp, "#END\n\n" );
		FileClose( fp );
	}
	else
	{
		char buf[256];

		sprintf( buf, "%s%s", CORPSE_DIR, capitalize( name ) );
		remove( buf );
	}
	return;
}

void load_corpses( void )
{
	DIR *dp;
	struct dirent *de;
	extern FILE *fpArea;
	extern char strArea[MAX_INPUT_LENGTH];
	extern int falling;

	if( !( dp = opendir( CORPSE_DIR ) ) )
	{
		bug( "Load_corpses: can't open CORPSE_DIR", 0 );
		perror( CORPSE_DIR );
		return;
	}

	falling = 1; /* Arbitrary, must be >0 though. */
	while( ( de = readdir( dp ) ) != NULL )
	{
		if( de->d_name[0] != '.' )
		{
			sprintf( strArea, "%s%s", CORPSE_DIR, de->d_name );
			fprintf( stderr, "Corpse -> %s\n", strArea );
			if( !( fpArea = FileOpen( strArea, "r" ) ) )
			{
				perror( strArea );
				continue;
			}
			for( ;; )
			{
				char letter;
				char *word;

				letter = fread_letter( fpArea );
				if( letter == '*' )
				{
					fread_to_eol( fpArea );
					continue;
				}
				if( letter != '#' )
				{
					bug( "Load_corpses: # not found.", 0 );
					break;
				}
				word = fread_word( fpArea );
				if( !str_cmp( word, "CORPSE" ) )
					fread_obj( NULL, fpArea, OS_CORPSE );
				else if( !str_cmp( word, "OBJECT" ) )
					fread_obj( NULL, fpArea, OS_CARRY );
				else if( !str_cmp( word, "END" ) )
					break;
				else
				{
					bug( "Load_corpses: bad section.", 0 );
					break;
				}
			}
			FileClose( fpArea );
		}
	}
	fpArea = NULL;
	strcpy( strArea, "$" );
	closedir( dp );
	falling = 0;
	return;
}

/* Return the vnum the obj is in, regardless of being carried or inside containers. -- Scion */
int get_obj_room_vnum_recursive( OBJ_DATA *obj )
{
	if( !obj )
		return ROOM_VNUM_LIMBO;

	if( obj->in_obj )
	{
		// bug( "Debug 0", 0 );
		return get_obj_room_vnum_recursive( obj->in_obj );
	}
	else if( obj->carried_by )
	{
		CHAR_DATA *ch = obj->carried_by;

		if( ch->in_room )
		{
			//bug( "Debug 1", 0 );
			return ch->in_room->vnum;
		}
		else
		{
			bug( "get_obj_room_vnum_recursive: Char \"%s\" not in room?!", ch->name );
			return ROOM_VNUM_LIMBO;
		}
	}
	else if( obj->in_room )
	{
		//bug( "Debug 2", 0 );
		return obj->in_room->vnum;
	}
	else
	{
		bug( "get_obj_room_vnum_recursive: obj isn't carried, inside containers, or inside a room!", 0 );
		return ROOM_VNUM_LIMBO;
	}
}

void fread_finger( CHAR_DATA *ch, FILE *fp, char *laston )
{
	const char *word;
	const char *email = NULL;
	const char *name = NULL;
	const char *site = NULL;
	const char *title = NULL;
	const char *bio = NULL;
	const char *authed = NULL;
	const char *aim = NULL;
	const char *msn = NULL;
	const char *real = NULL;
	const char *yim = NULL;
	const char *temp = NULL;
	short level = 0, race = 0, sex = 0;
	int icq = 0, flags = 0, played = 0;
	short fing_ver = 0;
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

		case 'A':
			KEY( "AuthedBy", authed, fread_string( fp ) );
			KEY( "Aim", aim, fread_string( fp ) );
			break;

		case 'B':
			KEY( "Bio", bio, fread_string( fp ) );
			break;

		case 'D':
			KEY( "Description", temp, fread_string( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
				goto finger_display;
			KEY( "Email", email, fread_string_nohash( fp ) );
			break;

		case 'F':
			KEY( "Flags", flags, fread_number( fp ) );
			break;

		case 'I':
			KEY( "ICQ", icq, fread_number( fp ) );
			break;

		case 'M':
			KEY( "MSN", msn, fread_string( fp ) );
			break;

		case 'N':
			KEY( "Name", name, fread_string( fp ) );
			break;

		case 'P':
			KEY( "Played", played, fread_number( fp ) );
			break;
		case 'R':
			KEY( "Real", real, fread_string( fp ) );
			if( !str_cmp( word, "Race" ) )
			{
				if( fing_ver < 8 )
				{
					race = fread_number( fp );
					fMatch = true;
				}
				else
				{
					race = get_pc_race( fread_string( fp ) );
					if( race < 0 || race > MAX_RACE )
						race = RACE_COLONIST;
					fMatch = true;
				}
			}
			break;

		case 'S':
			KEY( "Sex", sex, fread_number( fp ) );
			if( !str_cmp( word, "Site" ) )
			{
				site = STRALLOC( fread_word( fp ) );
				fMatch = true;
			}
			break;

		case 'T':
			KEY( "Title", title, fread_string( fp ) );
			KEY( "Toplevel", level, fread_number( fp ) );
			break;

		case 'V':
			KEY( "Version", file_ver, fread_number( fp ) );
			break;

		case 'Y':
			KEY( "YIM", yim, fread_string( fp ) );
			break;

		}
		if( !fMatch )
			fread_to_eol( fp );
	}

	/* Extremely ugly and disgusting goto hack, if there's a better way to do
	this, I'd sure like to know - Samson */

finger_display:

	if( level >= LEVEL_STAFF && !IS_IMMORTAL( ch ) )
	{
		send_to_char( "Cannot finger an immortal.\r\n", ch );
		return;
	}

	send_to_char( "\r\n&z------------------------------------\r\n", ch );
	send_to_char( "&z|          &CMud Chara Info          &z|\r\n", ch );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	ch_printf( ch, "&GName&g: &c%-15s &GRace&g:&c %s\r\n", name, capitalize( pc_race[race] ) );
	ch_printf( ch, "&GLevel&g: &c%-4d           &GSex&g: &c%s\r\n", level,
		sex == SEX_MALE ? "Male" : sex == SEX_FEMALE ? "Female" : "neutral" );
	ch_printf( ch, "&GLast on : &c%s", laston );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	send_to_char( "&z|          &CReal Life Info          &z|\r\n", ch );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	ch_printf( ch, "&GName&g:&c %s\r\n", real ? real : "N/A" );
	ch_printf( ch, "&GEmail&g:&c %s\r\n", email ? email : "N/A" );
	ch_printf( ch, "&GAIM&g:&c %s\r\n", aim ? aim : "N/A" );
	ch_printf( ch, "&GICQ&g:&c %d\r\n", icq );
	ch_printf( ch, "&GYIM&g:&c %s\r\n", yim ? yim : "N/A" );
	ch_printf( ch, "&GMSN&g:&c %s\r\n", msn ? msn : "N/A" );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	if( IS_IMMORTAL( ch ) )
	{
		send_to_char( "&z|           &CImm Info               &z|\r\n", ch );
		send_to_char( "&z|&W==================================&z|\r\n", ch );
		ch_printf( ch, "&GIP&g: &c%s\r\n", site );
		ch_printf( ch, "&GTime played&g: &c%d hours\r\n", ( played / 3600 ) );
		ch_printf( ch, "&GAuthed by&g: &c%s\r\n", authed ? authed : ( sysdata.WAIT_FOR_AUTH ? "Not Authed" : "The Code" ) );
	}
	ch_printf( ch, "&GBio&g:\r\n&c%s\r\n", bio ? bio : "Bio not set." );
	send_to_char( "&z------------------------------------\r\n", ch );

	STRFREE( site );
	return;
}

void read_finger( CHAR_DATA *ch, const char *argument )
{
	FILE *fpFinger;
	char fingload[MAX_INPUT_LENGTH];
	char *laston = NULL;
	struct stat fst;

	fingload[0] = '\0';

	sprintf( fingload, "%s%c/%s", PLAYER_DIR, tolower( argument[0] ), capitalize( argument ) );

	/*
	 * Bug fix here provided by Senir to stop /dev/null crash
	 */
	if( ( stat( fingload, &fst ) == -1 ) || ( !check_parse_name( argument ) ) )
	{
		send_to_char( "&YNo such player exists.\r\n", ch );
		return;
	}

	laston = ctime( &fst.st_mtime );

	if( stat( fingload, &fst ) != -1 )
	{
		if( ( fpFinger = FileOpen( fingload, "r" ) ) != NULL )
		{
			for( ;; )
			{
				char letter;
				char *word;

				letter = fread_letter( fpFinger );

				if( letter != '#' )
					continue;

				word = fread_word( fpFinger );

				if( !str_cmp( word, "End" ) )
					break;

				if( !str_cmp( word, "PLAYER" ) )
					fread_finger( ch, fpFinger, laston );
				else if( !str_cmp( word, "END" ) )  /* Done     */
					break;
			}
			FileClose( fpFinger );
		}
	}
	return;
}

/* Finger snippet courtesy of unknown author. Installed by Samson 4-6-98 */
/* File read/write code redone using standard Smaug I/O routines - Samson 9-12-98 */
/* Data gathering now done via the pfiles, eliminated separate finger files - Samson 12-21-98 */
CMDF( do_finger )
{
	CHAR_DATA *victim = NULL;
	char buf[MAX_STRING_LENGTH];

	if( IS_NPC( ch ) )
		return;

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Finger whom?\r\n", ch );
		return;
	}

	strcpy( buf, "0." );
	strcat( buf, argument );
	victim = get_char_world( ch, buf );

	if( ( !victim ) )
	{
		read_finger( ch, argument );
		return;
	}

	if( IS_IMMORTAL( victim ) && !IS_IMMORTAL( ch ) )
	{
		send_to_char( "Cannot finger an immortal.\r\n", ch );
		return;
	}

	send_to_char( "\r\n&z------------------------------------\r\n", ch );
	send_to_char( "&z|          &CMud Chara Info          &z|\r\n", ch );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	ch_printf( ch, "&GName&g: &c%-15s &GRace&g:&c %s\r\n", victim->name, capitalize( pc_race[victim->race] ) );
	ch_printf( ch, "&GLevel&g: &c%-4d           &GSex&g: &c%s\r\n", victim->top_level,
		victim->sex == SEX_MALE ? "Male" : victim->sex == SEX_FEMALE ? "Female" : "neutral" );
	ch_printf( ch, "&GLast on : &c%s", ( char * ) ctime( &victim->logon ) );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	send_to_char( "&z|          &CReal Life Info          &z|\r\n", ch );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	ch_printf( ch, "&GName&g:&c %s\r\n", victim->pcdata->real ? victim->pcdata->real : "N/A" );
	ch_printf( ch, "&GEmail&g:&c %s\r\n", victim->pcdata->email ? victim->pcdata->email : "N/A" );
	ch_printf( ch, "&GAIM&g:&c %s\r\n", victim->pcdata->aim ? victim->pcdata->aim : "N/A" );
	ch_printf( ch, "&GICQ&g:&c %d\r\n", victim->pcdata->icq );
	ch_printf( ch, "&GYIM&g:&c %s\r\n", victim->pcdata->yim ? victim->pcdata->yim : "N/A" );
	ch_printf( ch, "&GMSN&g:&c %s\r\n", victim->pcdata->msn ? victim->pcdata->msn : "N/A" );
	send_to_char( "&z|&W==================================&z|\r\n", ch );
	if( IS_IMMORTAL( ch ) )
	{
		send_to_char( "&z|           &CImm Info               &z|\r\n", ch );
		send_to_char( "&z|&W==================================&z|\r\n", ch );
		ch_printf( ch, "&GIP&g: &c%s\r\n", victim->pcdata->lasthost );
		ch_printf( ch, "&GTime played&g: &c%d hours\r\n", GET_TIME_PLAYED( victim ) );
		ch_printf( ch, "&GAuthed by&g: &c%s\r\n",
			victim->pcdata->authed_by ? victim->pcdata->authed_by : ( sysdata.
				WAIT_FOR_AUTH ? "Not Authed" : "The Code" ) );
	}
	ch_printf( ch, "&GBio&g:\r\n&c%s\r\n", victim->pcdata->bio ? victim->pcdata->bio : "Bio not set." );
	send_to_char( "&z------------------------------------\r\n", ch );

	return;
}

void save_artifacts( void )
{
	FILE *objfp;
	int objfile = 0;
	char filename[256];
	OBJ_DATA *obj;


	sprintf( filename, "%s%s", SYSTEM_DIR, ARTIFACT_FILE );
	if( ( objfp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_artifact: FileOpen artifact file", 0 );
		perror( filename );
	}
	else
		objfile++;

	if( objfile )
	{
		for( obj = first_object; obj; obj = obj->next )
		{
			if( ( !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) ) || ( obj->in_room == NULL ) )
				continue;

			if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
			{
				obj->pIndexData->count = 1;
				fwrite_obj( NULL, obj, objfp, 0, OS_CORPSE, false );
			}
		}

		fprintf( objfp, "#END" );
		FileClose( objfp );
	}

}

/*
 * Rewritten by Amras
 */
void load_artifacts( void )
{
	FILE *fp;
	char filename[MAX_INPUT_LENGTH];

	sprintf( filename, "%s%s", SYSTEM_DIR, ARTIFACT_FILE );

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{
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
				bug( "Load_artifacts: # not found." );
				break;
			}

			word = fread_word( fp );
			if( str_cmp( word, "#END" ) )
				fread_obj( NULL, fp, OS_GROUND );
			else
			{
				bug( "Load_artifact: bad section." );
				break;
			}
		}
		FileClose( fp );
	}
}

/*
 * Based on last time modified, show when a player was last on -Thoric
 */
CMDF( do_last )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];
	struct stat fst;
	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Usage: last <playername>\r\n", ch );
		send_to_char( "Usage: last <# of entries OR \'-1\' for all entries OR \'today\' forall of today's entries>\r\n", ch );
		send_to_char( "Usage: last <playername> <count>\r\n", ch );
		return;
	}
	if( isdigit( arg[0] ) || atoi( arg ) == -1 || !str_cmp( arg, "today" ) ) //View list instead of players
	{
		send_to_char
		( "&w&CName Time Host&B/&CIp\r\n&c&B---------------------------------------------------------------------------&W\r\n",
			ch );
		if( !str_cmp( arg, "today" ) )
			read_last_file( ch, -2, NULL );
		else
			read_last_file( ch, atoi( arg ), NULL );
		return;
	}
	strcpy( name, capitalize( arg ) );
	if( argument[0] != '\0' )
	{
		send_to_char
		( "&w&CName Time Host&B/&CIp\r\n&c&B---------------------------------------------------------------------------&W\r\n",
			ch );
		read_last_file( ch, atoi( argument ), name );
		return;
	}
	sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
	if( stat( buf, &fst ) != -1 )
		sprintf( buf, "&C%s was last on&B: &W%s\r", name, ctime( &fst.st_mtime ) );
	else
		sprintf( buf, "&R%s was not found.\r\n", name );
	send_to_char( buf, ch );
}

void read_last_file( CHAR_DATA *ch, int count, const char *name )
{
	FILE *fpout;
	char filename[MAX_INPUT_LENGTH];
	char charname[100];
	int cnt = 0;
	int letter = 0;
	char *ln;
	char *c;
	char d, e;
	struct tm *tme;
	time_t now;
	char day[MAX_INPUT_LENGTH];
	char sday[5];
	int fnd = 0;
	sprintf( filename, "%s", LAST_LIST );
	if( ( fpout = FileOpen( filename, "r" ) ) == NULL )
	{
		send_to_char( "&RThere is no last file to look at.\r\n", ch );
		return;
	}
	for( ;; )
	{
		if( feof( fpout ) )
		{
			FileClose( fpout );
			ch_printf( ch,
				"&B---------------------------------------------------------------------------\r\n&W%d &CEntries Listed.\r\n",
				cnt );
			return;
		}
		else
		{
			if( count == -2 || ++cnt <= count || count == -1 )
			{
				ln = fread_line( fpout );
				strcpy( charname, "" );
				if( name ) //looking for a certain name
				{
					c = ln;
					for( ;; )
					{
						if( isalpha( *c ) && !isspace( *c ) )
						{
							charname[letter] = *c;
							letter++;
							c++;
						}
						else
						{
							charname[letter] = '\0';
							if( !str_cmp( charname, name ) )
							{
								ch_printf( ch, "%s", ln );
								letter = 0;
								strcpy( charname, "" );
								break;
							}
							else
							{
								if( !feof( fpout ) )
								{
									fread_line( fpout );
									c = ln;
									letter = 0;
									strcpy( charname, "" );
									continue;
								}
								else
								{
									cnt--;
									break;
								}
							}
						}
					}
				}
				else if( count == -2 ) //only today's entries
				{
					c = ln;
					now = time( 0 );
					tme = localtime( &now );
					strftime( day, 10, "%d", tme );
					for( ;; )
					{
						if( !isdigit( *c ) )
						{
							c++;
						}
						else
						{
							d = *c;
							c++;
							e = *c;
							sprintf( sday, "%c%c", d, e );
							if( !str_cmp( sday, day ) )
							{
								fnd = 1;
								cnt++;
								ch_printf( ch, "%s", ln );
								break;
							}
							else
							{
								if( fnd == 1 )
								{
									FileClose( fpout );
									ch_printf( ch,
										"&B---------------------------------------------------------------------------\r\n&W%d &CEntries Listed.\r\n",
										cnt );
									return;
								}
								else
									break;
							}
						}
					}
				}
				else
				{
					ch_printf( ch, "&W%s", ln );
				}
			}
			else
			{
				FileClose( fpout );
				ch_printf( ch,
					"&B--------------------------------------------------------------------------\r\n&W%d &CEntries Listed.\r\n",
					count );
				return;
			}
		}
	}
}

void copy_files_contents( FILE *fsource, FILE *fdestination )
{
	int ch;
	int cnt = 1;
	for( ;; )
	{
		ch = fgetc( fsource );
		if( !feof( fsource ) )
		{
			fputc( ch, fdestination );
			if( ch == '\n' )
			{
				cnt++;
				if( cnt >= LAST_FILE_SIZE ) //limit size of this file please :-)
					break;
			}
		}
		else
			break;
	}
}
