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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <math.h>
#include "mud.h"
#include "bits.h"

extern int _filbuf( FILE * );

void init_supermob( );

/*
 * Globals.
 */

WIZENT *first_wiz;
WIZENT *last_wiz;

time_t last_restore_all_time = 0;

HELP_DATA *first_help;
HELP_DATA *last_help;

SHOP_DATA *first_shop;
SHOP_DATA *last_shop;

REPAIR_DATA *first_repair;
REPAIR_DATA *last_repair;

TELEPORT_DATA *first_teleport;
TELEPORT_DATA *last_teleport;

OBJ_DATA *extracted_obj_queue;
EXTRACT_CHAR_DATA *extracted_char_queue;

char bug_buf[2 * MAX_INPUT_LENGTH];
CHAR_DATA *first_char;
CHAR_DATA *last_char;
const char *help_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];
char str_empty[1];
char *string_space;
char *top_string;
OBJ_DATA *first_object;
OBJ_DATA *last_object;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;
WAR_DATA war_info;
void load_bits( void );

int FilesOpen;
File *first_filedata;
File *last_filedata;

int cur_qobjs;
int cur_qchars;
int nummobsloaded;
int numobjsloaded;
int physicalobjects;

MAP_INDEX_DATA *first_map;  /* maps */

AUCTION_DATA *auction;  /* auctions */
OBJ_DATA *supermob_obj;

FILE *fpLOG;

void free_ignores( CHAR_DATA *ch );

void start_spring( void );
void start_winter( void );
void start_summer( void );
void start_fall( void );
void free_aliases( CHAR_DATA *ch );

/* criminals */
//short   gsn_torture;
short gsn_disguise;
short gsn_beg;
short gsn_pickshiplock;
short gsn_hijack;

/* soldiers and officers */
short gsn_reinforcements;
short gsn_postguard;
short gsn_mine;
short gsn_grenades;
short gsn_first_aid;
short gsn_snipe;
short gsn_throw;

short gsn_eliteguard;
short gsn_specialforces;
short gsn_jail;
short gsn_smalltalk;
short gsn_propeganda;
short gsn_bribe;
short gsn_seduce;
short gsn_masspropeganda;
short gsn_gather_intelligence;
short gsn_sprint;

/* pilots and smugglers */
short gsn_mobilesuits;
short gsn_midships;
short gsn_capitalships;
short gsn_weaponsystems;
short gsn_navigation;
short gsn_shipsystems;
short gsn_tractorbeams;
short gsn_shipmaintenance;
short gsn_spacecombat;
short gsn_spacecombat2;
short gsn_spacecombat3;
short gsn_bulletweapons;
short gsn_missileweapons;
short gsn_beamsabers;
short gsn_lightenergy;
short gsn_heavyenergy;
short gsn_meleeweapons;
short gsn_transform;
short gsn_zerosystem;
short gsn_bomb;
short gsn_spacemines;
short gsn_landcombat;

/* player building skills */
short gsn_lightsaber_crafting;
short gsn_spice_refining;
short gsn_makeblade;
short gsn_makeblaster;
short gsn_makelight;
short gsn_makecomlink;
short gsn_makegrenade;
short gsn_makelandmine;
short gsn_makearmor;
short gsn_makeshield;
short gsn_makecontainer;
short gsn_sharpen;
short gsn_makesweapon;
//short   gsn_makemissile;
short gsn_gemcutting;
short gsn_makejewelry;

/* Espionage */
short gsn_makecamera;
short gsn_phase;
short gsn_hack;
short gsn_meddle;
short gsn_camset;
short gsn_lore;
short gsn_sabotage;
short gsn_codebreak;
short gsn_slip;
short gsn_fry;
short gsn_trap;
short gsn_gas;
short gsn_bear;
short gsn_acid;
short gsn_energynet;
short gsn_pdart;
short gsn_backlash;
short gsn_createdrug;

/* weaponry */
short gsn_firearms;
short gsn_bowandarrows;
short gsn_spears;
short gsn_lightsabers;
short gsn_swords;
short gsn_flexible_arms;
short gsn_talonous_arms;
short gsn_bludgeons;
short gsn_shieldwork;

/* thief */
short gsn_detrap;
short gsn_backstab;
short gsn_circle;
short gsn_strangle;
short gsn_dodge;
short gsn_hide;
short gsn_peek;
short gsn_pick_lock;
short gsn_sneak;
short gsn_steal;
short gsn_gouge;
short gsn_poison_weapon;
short gsn_rush;
short gsn_rub;
/* thief & warrior */
short gsn_disarm;
short gsn_enhanced_damage;
short gsn_offhand;
short gsn_kick;
short gsn_parry;
short gsn_rescue;
short gsn_second_attack;
short gsn_third_attack;
short gsn_fourth_attack;
short gsn_fifth_attack;
short gsn_dual_wield;
short gsn_punch;
short gsn_bash;
short gsn_dropkick;
short gsn_mace;
short gsn_pdust;
short gsn_stun;
short gsn_bashdoor;
short gsn_grip;
short gsn_berserk;
short gsn_hitall;

/* vampire */
short gsn_feed;

/* other   */
short gsn_aid;
short gsn_track;
short gsn_stalk;
short gsn_search;
short gsn_dig;
short gsn_mount;
short gsn_bite;
short gsn_blasta;
short gsn_blastb;
short gsn_blastc;
short gsn_blastd;
short gsn_blaste;
short gsn_blastf;
short gsn_blastg;
short gsn_claw;
short gsn_sting;
short gsn_tail;
short gsn_scribe;
short gsn_brew;
short gsn_climb;
short gsn_scan;
short gsn_slice;
short gsn_blackjack;
short gsn_pummel;
short gsn_assassinate;
short gsn_createvoodoo;
short gsn_catfight;
short gsn_hobo;
short gsn_pimp;
short gsn_drunken;
short gsn_cheapskate;
short gsn_style;
short gsn_counter;

/* spells */
short gsn_aqua_breath;
short gsn_blindness;
short gsn_charm_person;
short gsn_curse;
short gsn_invis;
short gsn_mass_invis;
short gsn_poison;
short gsn_nemesis;
short gsn_sleep;
short gsn_possess;
short gsn_fireball;
short gsn_chill_touch;
short gsn_lightning_bolt;

/* for searching */
short gsn_first_spell;
short gsn_first_skill;
short gsn_first_weapon;
short gsn_first_tongue;
short gsn_top_sn;


/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

AREA_DATA *first_area;
AREA_DATA *last_area;
AREA_DATA *first_area_name;   /*Used for alphanum. sort */
AREA_DATA *last_area_name;
AREA_DATA *first_build;
AREA_DATA *last_build;
AREA_DATA *first_asort;
AREA_DATA *last_asort;
AREA_DATA *first_bsort;
AREA_DATA *last_bsort;

SYSTEM_DATA sysdata;

int top_affect;
int top_area;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_repair;
int top_vroom;

/*
 * Semi-locals.
 */
bool fBootDb;
bool DONT_UPPER;
bool MOBtrigger;
bool MPSilent;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];


/*
 * Local booting procedures.
 */
void init_mm( void );

void boot_log( const char *str, ... );
AREA_DATA *load_area( FILE *fp, int aversion );
void load_author( AREA_DATA *tarea, FILE *fp );
void load_owned_by( AREA_DATA *tarea, FILE *fp );
void load_economy( AREA_DATA *tarea, FILE *fp );
void load_resetmsg( AREA_DATA *tarea, FILE *fp );    /* Rennard */
void load_flags( AREA_DATA *tarea, FILE *fp );
void load_helps( AREA_DATA *tarea, FILE *fp );
void load_mobiles( AREA_DATA *tarea, FILE *fp );
void load_objects( AREA_DATA *tarea, FILE *fp );
void load_resets( AREA_DATA *tarea, FILE *fp );
void load_rooms( AREA_DATA *tarea, FILE *fp );
void load_shops( AREA_DATA *tarea, FILE *fp );
void load_repairs( AREA_DATA *tarea, FILE *fp );
void load_specials( AREA_DATA *tarea, FILE *fp );
void load_ranges( AREA_DATA *tarea, FILE *fp );
void load_buildlist( void );
bool load_systemdata( SYSTEM_DATA *sys );
void load_banlist( void );
void load_reserved( void );
void load_mname( void );
void load_fname( void );
void initialize_economy( void );
void load_notes( void );
void fix_exits( void );
void load_specfuns( void );

/*
 * External booting function
 */
void load_corpses( void );
void renumber_put_resets( ROOM_INDEX_DATA *room );
void wipe_resets( ROOM_INDEX_DATA *room );
void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex );
void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex );
void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex );


void shutdown_mud( const char *reason )
{
	FILE *fp;

	if( ( fp = FileOpen( SHUTDOWN_FILE, "a" ) ) != NULL )
	{
		fprintf( fp, "%s\n", reason );
		FileClose( fp );
	}
}

 /*
  * Big mama top level function.
  */
void boot_db( bool fCopyOver )
{
	short wear, x;

	show_hash( 32 );
	unlink( BOOTLOG_FILE );
	boot_log( "---------------------[ Boot Log ]--------------------" );

	log_string( "Initializing libdl support..." );
	/*
	 * Open up a handle to the executable's symbol table for later use
	 * when working with commands
	 */
	sysdata.dlHandle = dlopen( NULL, RTLD_LAZY );
	if( !sysdata.dlHandle )
	{
		log_string( "dl: Error opening local system executable as handle, please check compile flags." );
		shutdown_mud( "libdl failure" );
		exit( 26 );
	}

	log_string( "Loading commands..." );
	load_commands( );

	log_string( "Loading spec_funs..." );
	load_specfuns( );

	log_string( "Loading sysdata configuration..." );

	/*
	 * default values
	 */
	sysdata.read_all_mail = LEVEL_LIAISON;
	sysdata.read_mail_free = LEVEL_STAFF;
	sysdata.write_mail_free = LEVEL_STAFF;
	sysdata.take_others_mail = LEVEL_LIAISON;
	sysdata.muse_level = LEVEL_LIAISON;
	sysdata.think_level = LEVEL_HIGOD;
	sysdata.build_level = LEVEL_LIAISON;
	sysdata.log_level = LEVEL_LOG;
	sysdata.level_modify_proto = LEVEL_LIAISON;
	sysdata.level_override_private = LEVEL_LIAISON;
	sysdata.level_mset_player = LEVEL_LIAISON;
	sysdata.stun_plr_vs_plr = 15;
	sysdata.stun_regular = 15;
	sysdata.maxign = 6;
	sysdata.dam_plr_vs_plr = 100;
	sysdata.dam_plr_vs_mob = 100;
	sysdata.dam_mob_vs_plr = 100;
	sysdata.dam_mob_vs_mob = 100;
	sysdata.hoursperday = 28;
	sysdata.daysperweek = 13;
	sysdata.dayspermonth = 26;
	sysdata.monthsperyear = 12;
	sysdata.daysperyear = 360;
	//    sysdata.lotterynum                  = number_range( 0, 9999 );
	sysdata.level_getobjnotake = LEVEL_LIAISON;
	sysdata.save_frequency = 20; /* minutes */
	sysdata.save_flags = SV_DEATH | SV_PASSCHG | SV_AUTO | SV_PUT | SV_DROP | SV_GIVE | SV_AUCTION | SV_ZAPDROP | SV_IDLE;
	if( !load_systemdata( &sysdata ) )
	{
		log_string( "Not found.  Creating new configuration." );
		sysdata.alltimemax = 0;
	}

	log_string( "Loading socials" );
	load_socials( );

	log_string( "Loading skill table" );
	load_skill_table( );
	sort_skill_table( );

	gsn_first_spell = 0;
	gsn_first_skill = 0;
	gsn_first_weapon = 0;
	gsn_first_tongue = 0;
	gsn_top_sn = top_sn;

	for( x = 0; x < top_sn; x++ )
		if( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
			gsn_first_spell = x;
		else if( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
			gsn_first_skill = x;
		else if( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
			gsn_first_weapon = x;
		else if( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
			gsn_first_tongue = x;

	log_string( "Loading herb table" );
	load_herb_table( );

	/*
	 * abit/qbit code
	 */
	log_string( "Loading quest bit tables" );
	load_bits( );

	log_string( "Making wizlist" );
	make_wizlist( );

	fBootDb = true;

	nummobsloaded = 0;
	numobjsloaded = 0;
	physicalobjects = 0;
	sysdata.maxplayers = 0;
	first_object = NULL;
	last_object = NULL;
	first_char = NULL;
	last_char = NULL;
	first_area = NULL;
	last_area = NULL;
	first_build = NULL;
	last_area = NULL;
	first_shop = NULL;
	last_shop = NULL;
	first_repair = NULL;
	last_repair = NULL;
	first_teleport = NULL;
	last_teleport = NULL;
	first_asort = NULL;
	last_asort = NULL;
	extracted_obj_queue = NULL;
	extracted_char_queue = NULL;
	cur_qobjs = 0;
	cur_qchars = 0;
	cur_char = NULL;
	cur_obj = 0;
	cur_obj_serial = 0;
	cur_char_died = false;
	cur_obj_extracted = false;
	cur_room = NULL;
	quitting_char = NULL;
	loading_char = NULL;
	saving_char = NULL;
	immortal_host_start = NULL;
	immortal_host_end = NULL;
	CREATE( auction, AUCTION_DATA, 1 );
	auction->item = NULL;
	for( wear = 0; wear < MAX_WEAR; wear++ )
		for( x = 0; x < MAX_LAYERS; x++ )
			save_equipment[wear][x] = NULL;

	/*
	 * Init random number generator.
	 */
	log_string( "Initializing random number generator" );
	init_mm( );
	auction_list = NULL;

	/*
	 * Set time and weather.
	 */
	{
		long lhour, lday, lmonth;

		log_string( "Setting time and weather" );

		lhour = ( current_time - 650336715 ) / ( PULSE_TICK / PULSE_PER_SECOND );
		time_info.hour = lhour % sysdata.hoursperday;
		lday = lhour / sysdata.hoursperday;
		time_info.day = lday % sysdata.dayspermonth;
		lmonth = lday / sysdata.dayspermonth;
		time_info.month = lmonth % sysdata.monthsperyear;
		time_info.year = lmonth / sysdata.monthsperyear;

		if( time_info.hour < sysdata.hoursunrise )
			weather_info.sunlight = SUN_DARK;
		else if( time_info.hour < sysdata.hourdaybegin )
			weather_info.sunlight = SUN_RISE;
		else if( time_info.hour < sysdata.hoursunset )
			weather_info.sunlight = SUN_LIGHT;
		else if( time_info.hour < sysdata.hournightbegin )
			weather_info.sunlight = SUN_SET;
		else
			weather_info.sunlight = SUN_DARK;

		weather_info.change = 0;
		weather_info.mmhg = 960;
		if( time_info.month >= 7 && time_info.month <= 12 )
			weather_info.mmhg += number_range( 1, 50 );
		else
			weather_info.mmhg += number_range( 1, 80 );

		if( weather_info.mmhg <= 980 )
			weather_info.sky = SKY_LIGHTNING;
		else if( weather_info.mmhg <= 1000 )
			weather_info.sky = SKY_RAINING;
		else if( weather_info.mmhg <= 1020 )
			weather_info.sky = SKY_CLOUDY;
		else
			weather_info.sky = SKY_CLOUDLESS;

	}

	end_tag( );

	log_string( "Loading DNS cache..." ); /* Samson 1-30-02 */
	load_dns( );

	/*
	 * Assign gsn's for skills which need them.
	 */
	{
		log_string( "Assigning gsn's" );

		ASSIGN_GSN( gsn_eliteguard, "elite_guard" );
		ASSIGN_GSN( gsn_gather_intelligence, "gather_intelligence" );
		ASSIGN_GSN( gsn_specialforces, "special_forces" );
		ASSIGN_GSN( gsn_jail, "jail" );
		ASSIGN_GSN( gsn_smalltalk, "smalltalk" );
		ASSIGN_GSN( gsn_propeganda, "propeganda" );
		ASSIGN_GSN( gsn_bribe, "bribe" );
		ASSIGN_GSN( gsn_sprint, "sprint" );
		ASSIGN_GSN( gsn_seduce, "seduce" );
		ASSIGN_GSN( gsn_masspropeganda, "mass_propeganda" );
		ASSIGN_GSN( gsn_beg, "beg" );
		ASSIGN_GSN( gsn_hijack, "hijack" );
		ASSIGN_GSN( gsn_bulletweapons, "bulletweapons" );
		ASSIGN_GSN( gsn_missileweapons, "missileweapons" );
		ASSIGN_GSN( gsn_lightenergy, "lightenergy" );
		ASSIGN_GSN( gsn_heavyenergy, "heavyenergy" );
		ASSIGN_GSN( gsn_meleeweapons, "meleeweapons" );
		ASSIGN_GSN( gsn_transform, "transform" );
		ASSIGN_GSN( gsn_zerosystem, "zerosystem" );
		ASSIGN_GSN( gsn_bomb, "bomb" );
		ASSIGN_GSN( gsn_spacemines, "spacemines" );
		ASSIGN_GSN( gsn_landcombat, "landcombat" );
		ASSIGN_GSN( gsn_beamsabers, "beamsabers" );
		ASSIGN_GSN( gsn_makejewelry, "makejewelry" );
		ASSIGN_GSN( gsn_grenades, "grenades" );
		ASSIGN_GSN( gsn_makeblade, "makeblade" );
		ASSIGN_GSN( gsn_makeblaster, "makeblaster" );
		ASSIGN_GSN( gsn_makelight, "makeflashlight" );
		ASSIGN_GSN( gsn_makecomlink, "makecomlink" );
		ASSIGN_GSN( gsn_makegrenade, "makegrenade" );
		ASSIGN_GSN( gsn_makelandmine, "makelandmine" );
		ASSIGN_GSN( gsn_makearmor, "makearmor" );
		ASSIGN_GSN( gsn_makeshield, "makeshield" );
		ASSIGN_GSN( gsn_makecontainer, "makecontainer" );
		ASSIGN_GSN( gsn_sharpen, "sharpen" );
		ASSIGN_GSN( gsn_makesweapon, "makesweapon" );
		ASSIGN_GSN( gsn_gemcutting, "gemcutting" );
		ASSIGN_GSN( gsn_reinforcements, "reinforcements" );
		ASSIGN_GSN( gsn_postguard, "post guard" );
		ASSIGN_GSN( gsn_throw, "throw" );
		ASSIGN_GSN( gsn_snipe, "snipe" );
		ASSIGN_GSN( gsn_disguise, "disguise" );
		ASSIGN_GSN( gsn_mine, "mine" );
		ASSIGN_GSN( gsn_first_aid, "firstaid" );
		ASSIGN_GSN( gsn_spice_refining, "spice refining" );
		ASSIGN_GSN( gsn_spacecombat, "space combat 1" );
		ASSIGN_GSN( gsn_spacecombat2, "space combat 2" );
		ASSIGN_GSN( gsn_spacecombat3, "space combat 3" );
		ASSIGN_GSN( gsn_weaponsystems, "weapon systems" );
		ASSIGN_GSN( gsn_mobilesuits, "mobilesuits" );
		ASSIGN_GSN( gsn_navigation, "navigation" );
		ASSIGN_GSN( gsn_shipsystems, "ship systems" );
		ASSIGN_GSN( gsn_midships, "midships" );
		ASSIGN_GSN( gsn_capitalships, "capital ships" );
		ASSIGN_GSN( gsn_tractorbeams, "tractor beams" );
		ASSIGN_GSN( gsn_shipmaintenance, "ship maintenance" );
		ASSIGN_GSN( gsn_firearms, "firearms" );
		ASSIGN_GSN( gsn_bowandarrows, "bow and arrows" );
		ASSIGN_GSN( gsn_spears, "spears" );
		ASSIGN_GSN( gsn_swords, "swords" );
		ASSIGN_GSN( gsn_flexible_arms, "flexible arms" );
		ASSIGN_GSN( gsn_talonous_arms, "talonous arms" );
		ASSIGN_GSN( gsn_bludgeons, "bludgeons" );
		ASSIGN_GSN( gsn_detrap, "detrap" );
		ASSIGN_GSN( gsn_backstab, "backstab" );
		ASSIGN_GSN( gsn_phase, "phase" );
		ASSIGN_GSN( gsn_makecamera, "makecamera" );
		ASSIGN_GSN( gsn_sabotage, "sabotage" );
		ASSIGN_GSN( gsn_hack, "hack" );
		ASSIGN_GSN( gsn_meddle, "meddle" );
		ASSIGN_GSN( gsn_slip, "slip" );
		ASSIGN_GSN( gsn_codebreak, "codebreak" );
		ASSIGN_GSN( gsn_lore, "lore" );
		ASSIGN_GSN( gsn_camset, "camset" );
		ASSIGN_GSN( gsn_fry, "fry" );
		ASSIGN_GSN( gsn_trap, "trap" );
		ASSIGN_GSN( gsn_gas, "gas" );
		ASSIGN_GSN( gsn_bear, "bear" );
		ASSIGN_GSN( gsn_acid, "acid" );
		ASSIGN_GSN( gsn_energynet, "energynet" );
		ASSIGN_GSN( gsn_pdart, "pdart" );
		ASSIGN_GSN( gsn_backlash, "backlash" );
		ASSIGN_GSN( gsn_createdrug, "createdrug" );
		ASSIGN_GSN( gsn_assassinate, "assassinate" );
		ASSIGN_GSN( gsn_createvoodoo, "createvoodoo" );
		ASSIGN_GSN( gsn_counter, "counter" );
		ASSIGN_GSN( gsn_circle, "circle" );
		ASSIGN_GSN( gsn_strangle, "strangle" );
		ASSIGN_GSN( gsn_dodge, "dodge" );
		ASSIGN_GSN( gsn_hide, "hide" );
		ASSIGN_GSN( gsn_peek, "peek" );
		ASSIGN_GSN( gsn_pick_lock, "pick lock" );
		ASSIGN_GSN( gsn_pickshiplock, "pick ship lock" );
		ASSIGN_GSN( gsn_sneak, "sneak" );
		ASSIGN_GSN( gsn_steal, "steal" );
		ASSIGN_GSN( gsn_gouge, "gouge" );
		ASSIGN_GSN( gsn_rush, "rush" );
		ASSIGN_GSN( gsn_rub, "rub" );
		ASSIGN_GSN( gsn_poison_weapon, "poison weapon" );
		ASSIGN_GSN( gsn_enhanced_damage, "enhanced damage" );
		ASSIGN_GSN( gsn_kick, "kick" );
		ASSIGN_GSN( gsn_parry, "parry" );
		ASSIGN_GSN( gsn_rescue, "rescue" );
		ASSIGN_GSN( gsn_second_attack, "second attack" );
		ASSIGN_GSN( gsn_third_attack, "third attack" );
		ASSIGN_GSN( gsn_fourth_attack, "fourth attack" );
		ASSIGN_GSN( gsn_fifth_attack, "fifth attack" );
		ASSIGN_GSN( gsn_dual_wield, "dual wield" );
		ASSIGN_GSN( gsn_offhand, "offhand" );
		ASSIGN_GSN( gsn_punch, "punch" );
		ASSIGN_GSN( gsn_bash, "bash" );
		ASSIGN_GSN( gsn_dropkick, "dropkick" );
		ASSIGN_GSN( gsn_mace, "mace" );
		ASSIGN_GSN( gsn_pdust, "pdust" );
		ASSIGN_GSN( gsn_stun, "stun" );
		ASSIGN_GSN( gsn_bashdoor, "doorbash" );
		ASSIGN_GSN( gsn_berserk, "berserk" );
		ASSIGN_GSN( gsn_hitall, "hitall" );
		ASSIGN_GSN( gsn_lore, "lore" );
		ASSIGN_GSN( gsn_aid, "aid" );
		ASSIGN_GSN( gsn_track, "track" );
		ASSIGN_GSN( gsn_stalk, "stalk" );
		ASSIGN_GSN( gsn_search, "search" );
		ASSIGN_GSN( gsn_dig, "dig" );
		ASSIGN_GSN( gsn_mount, "mount" );
		ASSIGN_GSN( gsn_bite, "bite" );
		ASSIGN_GSN( gsn_blasta, "blasta" );
		ASSIGN_GSN( gsn_blastb, "blastb" );
		ASSIGN_GSN( gsn_blastc, "blastc" );
		ASSIGN_GSN( gsn_blastd, "blastd" );
		ASSIGN_GSN( gsn_blaste, "blaste" );
		ASSIGN_GSN( gsn_blastf, "blastf" );
		ASSIGN_GSN( gsn_blastg, "blastg" );
		ASSIGN_GSN( gsn_claw, "claw" );
		ASSIGN_GSN( gsn_scribe, "scribe" );
		ASSIGN_GSN( gsn_climb, "climb" );
		ASSIGN_GSN( gsn_scan, "scan" );
		ASSIGN_GSN( gsn_fireball, "fireball" );
		ASSIGN_GSN( gsn_lightning_bolt, "force bolt" );
		ASSIGN_GSN( gsn_aqua_breath, "aqua breath" );
		ASSIGN_GSN( gsn_blindness, "blindness" );
		ASSIGN_GSN( gsn_charm_person, "affect mind" );
		ASSIGN_GSN( gsn_invis, "mask" );
		ASSIGN_GSN( gsn_mass_invis, "group masking" );
		ASSIGN_GSN( gsn_poison, "poison" );
		ASSIGN_GSN( gsn_sleep, "sleep" );
		ASSIGN_GSN( gsn_possess, "possess" );
	}

	/*
	 * Read in all the area files.
	 */
   {
	   FILE *fpList;

	   log_string( "Reading in area files..." );
	   if( !( fpList = FileOpen( AREA_LIST, "r" ) ) )
	   {
		   perror( AREA_LIST );
		   shutdown_mud( "Unable to open area list" );
		   exit( 1 );
	   }

	   for( ;; )
	   {
		   if( feof( fpList ) )
		   {
			   bug( "%s: EOF encountered reading area list - no $ found at end of file.", __func__ );
			   break;
		   }
		   mudstrlcpy( strArea, fread_word( fpList ), MAX_INPUT_LENGTH );
		   if( strArea[0] == '$' )
			   break;

		   load_area_file( strArea, false );
	   }
	   FileClose( fpList );
   }

   /*
	 *   initialize supermob.
	 *    must be done before reset_area!
	 *
	 */
	init_supermob( );


	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the notes file.
	 */
	{
		log_string( "Fixing exits" );
		fix_exits( );
		fBootDb = false;
		log_string( "Initializing economy" );
		initialize_economy( );
		log_string( "Loading buildlist" );
		load_buildlist( );
		log_string( "Loading clans" );
		load_clans( );
		log_string( "Loading Boards" );
		load_boards( );
		log_string( "Loading senate" );
		load_senate( );
		log_string( "Loading bans" );
		load_banlist( );
		log_string( "Loading reserved names..." );
		load_reserved( );
		log_string( "Loading random mob names..." );
		load_mname( );
		load_fname( );
		log_string( "Loading Immortal Hosts" );
		load_imm_host( );
		log_string( "Loading corpses" );
		load_corpses( );
		log_string( "Loading space" );
		load_space( );
		log_string( "Loading ships" );
		load_ships( );
		log_string( "Loading bounties" );
		load_bounties( );
		log_string( "Loading hiscore tables" );
		load_hiscores( );
		log_string( "Loading governments" );
		load_planets( );
		if( fCopyOver )
		{
			log_string( "Loading world state..." );
			load_world( supermob );
		}
		log_string( "Resetting areas" );
		area_update( );
		log_string( "Loading Artifacts" );
		load_artifacts( );

		MOBtrigger = true;
		MPSilent = false;
	}

	/*
	 * init_maps ( );
	 */

	return;
}

const char *const season_name[] = {
   "Spring", "Summer", "Fall", "Winter"
};

/* PaB: Which season are we in?
 * Notes: Simply figures out which season the current month falls into
 * and returns a proper value.
 */
void calc_season( void )
{
	int day;
	/*
	 * How far along in the year are we, measured in days?
	 */
	 /*
	  * PaB: Am doing this in days to minimize roundoff impact
	  */

	day = time_info.month * sysdata.dayspermonth + time_info.day;

	if( day < ( sysdata.daysperyear / 4 ) )
	{
		time_info.season = SEASON_SPRING;
		if( time_info.hour == 0 && day == 0 )
			start_spring( );
	}
	else if( day < ( sysdata.daysperyear / 4 ) * 2 )
	{
		time_info.season = SEASON_SUMMER;
		if( time_info.hour == 0 && day == ( sysdata.daysperyear / 4 ) )
			start_summer( );
	}
	else if( day < ( sysdata.daysperyear / 4 ) * 3 )
	{
		time_info.season = SEASON_FALL;
		if( time_info.hour == 0 && day == ( ( sysdata.daysperyear / 4 ) * 2 ) )
			start_fall( );
	}
	else if( day < sysdata.daysperyear )
	{
		time_info.season = SEASON_WINTER;
		if( time_info.hour == 0 && day == ( ( sysdata.daysperyear / 4 ) * 3 ) )
			start_winter( );
	}
	else
	{
		time_info.season = SEASON_SPRING;
	}

	//   season_update(); /* Maintain the season in case of a reboot*/

	return;
}

void start_winter( void )
{

	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &wA crisp air gushes through, tickling your nose.", ECHOTAR_ALL );
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &wThe cold season of &CW&ci&Cn&ct&Ce&cr&B&w has begun.", ECHOTAR_ALL );

	return;
}

void start_spring( void )
{

	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &OThe crisp air fades, as a warmer breeze of wind passes through.",
		ECHOTAR_ALL );
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &OThe season of &BS&bp&Br&bi&Bn&bg&O has begun.", ECHOTAR_ALL );

	return;
}

void start_summer( void )
{
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &RThe weather turns very hot, barely any wind.", ECHOTAR_ALL );
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &RThe sunny season of &YS&Ou&Ym&Om&Ye&Or&R has begun.", ECHOTAR_ALL );
	return;
}

void start_fall( void )
{
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &GThe leaves begin fading in colour on all the trees.", ECHOTAR_ALL );
	echo_to_all( AT_SOCIAL, "&C(&zGW&c:&zOM&C) &GThe windy season of &OA&Yu&Ot&Yu&Om&Yn&G has begun.", ECHOTAR_ALL );
	return;
}

/*
 * Load an area file's version number for versioned load support.
 */
void load_version( AREA_DATA *tarea, FILE *fp )
{
	if( !tarea )
	{
		bug( "Load_author: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 28 );
		}
		else
			return;
	}

	tarea->version = fread_number( fp );
	return;
}


/*
 * Load an 'area' header line.
 */
AREA_DATA *load_area( FILE *fp, int aversion )
{
	AREA_DATA *pArea;

	CREATE( pArea, AREA_DATA, 1 );
	pArea->version = aversion;
	pArea->first_room = pArea->last_room = NULL;
	pArea->next_on_planet = NULL;
	pArea->prev_on_planet = NULL;
	pArea->planet = NULL;
	pArea->name = fread_string_nohash( fp );
	pArea->author = STRALLOC( "unknown" );
	pArea->owned_by = STRALLOC( "None" );
	pArea->filename = str_dup( strArea );
	pArea->age = 15;
	pArea->nplayer = 0;
	pArea->low_vnum = 0;
	pArea->hi_vnum = 0;
	pArea->low_soft_range = 0;
	pArea->hi_soft_range = MAX_LEVEL;
	pArea->low_hard_range = 0;
	pArea->hi_hard_range = MAX_LEVEL;
	pArea->reset_frequency = 15;

	LINK( pArea, first_area, last_area, next, prev );
	top_area++;
	return pArea;
}


/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA *tarea, FILE *fp )
{
	if( !tarea )
	{
		bug( "Load_author: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 29 );
		}
		else
			return;
	}

	if( tarea->author )
		STRFREE( tarea->author );
	tarea->author = fread_string( fp );
	return;
}

/*
 * Load who area is owned by.    -Cray
 */
void load_owned_by( AREA_DATA *tarea, FILE *fp )
{
	if( !tarea )
	{
		bug( "Load_owned_by: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 30 );
		}
		else
			return;
	}

	if( tarea->owned_by )
		STRFREE( tarea->owned_by );
	tarea->owned_by = fread_string( fp );
	return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA *tarea, FILE *fp )
{
	if( !tarea )
	{
		bug( "Load_economy: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 31 );
		}
		else
			return;
	}

	tarea->high_economy = fread_number( fp );
	tarea->low_economy = fread_number( fp );
	return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA *tarea, FILE *fp )
{
	if( !tarea )
	{
		bug( "Load_resetmsg: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 32 );
		}
		else
			return;
	}

	if( tarea->resetmsg )
		DISPOSE( tarea->resetmsg );
	tarea->resetmsg = fread_string_nohash( fp );
	return;
}

/*
 * Load area flags. Narn, Mar/96
 */
void load_flags( AREA_DATA *tarea, FILE *fp )
{
	char *ln;
	int x1, x2;

	if( !tarea )
	{
		bug( "Load_flags: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 33 );
		}
		else
			return;
	}
	ln = fread_line( fp );
	x1 = x2 = 0;
	sscanf( ln, "%d %d", &x1, &x2 );
	tarea->flags = x1;
	tarea->reset_frequency = x2;
	if( x2 )
		tarea->age = x2;
	return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
	HELP_DATA *tHelp;
	int match;

	for( tHelp = first_help; tHelp; tHelp = tHelp->next )
		if( pHelp->level == tHelp->level && strcmp( pHelp->keyword, tHelp->keyword ) == 0 )
		{
			bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
			STRFREE( pHelp->text );
			STRFREE( pHelp->keyword );
			DISPOSE( pHelp );
			return;
		}
		else
			if( ( match = strcmp( pHelp->keyword[0] == '\'' ? pHelp->keyword + 1 : pHelp->keyword,
				tHelp->keyword[0] == '\'' ? tHelp->keyword + 1 : tHelp->keyword ) ) < 0
				|| ( match == 0 && pHelp->level > tHelp->level ) )
			{
				if( !tHelp->prev )
					first_help = pHelp;
				else
					tHelp->prev->next = pHelp;
				pHelp->prev = tHelp->prev;
				pHelp->next = tHelp;
				tHelp->prev = pHelp;
				break;
			}

	if( !tHelp )
		LINK( pHelp, first_help, last_help, next, prev );

	top_help++;
}

/*
 * Load a help section.
 */
void load_helps( FILE *fp )
{
	HELP_DATA *pHelp;

	for( ;; )
	{
		CREATE( pHelp, HELP_DATA, 1 );
		pHelp->level = fread_number( fp );
		pHelp->keyword = fread_string( fp );
		if( pHelp->keyword[0] == '$' )
		{
			STRFREE( pHelp->keyword );
			DISPOSE( pHelp );
			break;
		}
		pHelp->text = fread_string( fp );
		if( pHelp->keyword[0] == '\0' )
		{
			STRFREE( pHelp->text );
			STRFREE( pHelp->keyword );
			DISPOSE( pHelp );
			continue;
		}

		if( !str_cmp( pHelp->keyword, "greeting" ) )
			help_greeting = pHelp->text;
		add_help( pHelp );
	}
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
	LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
	MOB_INDEX_DATA *pMobIndex;
	char *ln;
	int x1, x2, x3, x4, x5, x6, x7, x8;

	if( !tarea )
	{
		bug( "Load_mobiles: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 34 );
		}
		else
			return;
	}

	for( ;; )
	{
		char buf[MAX_STRING_LENGTH];
		int vnum;
		char letter;
		int iHash;
		bool oldmob;
		bool tmpBootDb;

		letter = fread_letter( fp );
		if( letter != '#' )
		{
			bug( "Load_mobiles: # not found." );
			if( fBootDb )
			{
				shutdown_mud( "# not found" );
				exit( 35 );
			}
			else
				return;
		}

		vnum = fread_number( fp );
		if( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = false;
		if( get_mob_index( vnum ) )
		{
			if( tmpBootDb )
			{
				bug( "Load_mobiles: vnum %d duplicated.", vnum );
				shutdown_mud( "duplicate vnum" );
				exit( 36 );
			}
			else
			{
				pMobIndex = get_mob_index( vnum );
				snprintf( buf, MAX_STRING_LENGTH, "Cleaning mobile: %d", vnum );
				log_string_plus( buf, LOG_BUILD, sysdata.log_level );
				clean_mob( pMobIndex );
				oldmob = true;
			}
		}
		else
		{
			oldmob = false;
			CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
		}
		fBootDb = tmpBootDb;

		pMobIndex->vnum = vnum;
		if( fBootDb )
		{
			if( !tarea->low_vnum )
				tarea->low_vnum = vnum;
			if( vnum > tarea->hi_vnum )
				tarea->hi_vnum = vnum;
		}
		pMobIndex->player_name = fread_string( fp );
		pMobIndex->short_descr = fread_string( fp );
		pMobIndex->long_descr = fread_string( fp );
		pMobIndex->description = fread_string( fp );

		// well, it's pretty nasty to cast, but we know that we own this
	    // memory because we just created it.
		( ( char * ) pMobIndex->long_descr )[0] = UPPER( pMobIndex->long_descr[0] );
		( ( char * ) pMobIndex->description )[0] = UPPER( pMobIndex->description[0] );
		pMobIndex->act = fread_bitvector( fp );
		xSET_BIT( pMobIndex->act, ACT_IS_NPC );
		pMobIndex->affected_by = fread_number( fp );
		pMobIndex->pShop = NULL;
		pMobIndex->rShop = NULL;
		pMobIndex->alignment = fread_number( fp );
		letter = fread_letter( fp );
		pMobIndex->level = fread_number( fp );

		pMobIndex->mobthac0 = fread_number( fp );
		pMobIndex->ac = fread_number( fp );
		pMobIndex->hitnodice = fread_number( fp );
		/*
		 * 'd'
		 */ fread_letter( fp );
		 pMobIndex->hitsizedice = fread_number( fp );
		 /*
		  * '+'
		  */ fread_letter( fp );
		  pMobIndex->hitplus = fread_number( fp );
		  pMobIndex->damnodice = fread_number( fp );
		  /*
		   * 'd'
		   */ fread_letter( fp );
		   pMobIndex->damsizedice = fread_number( fp );
		   /*
			* '+'
			*/ fread_letter( fp );
			pMobIndex->damplus = fread_number( fp );
			pMobIndex->gold = fread_number( fp );
			pMobIndex->exp = fread_number( fp );
			pMobIndex->position = fread_number( fp );
			pMobIndex->defposition = fread_number( fp );

			/*
			 * Back to meaningful values.
			 */
			pMobIndex->sex = fread_number( fp );

			if( letter != 'S' && letter != 'C' && letter != 'Z' )
			{
				bug( "Load_mobiles: vnum %d: letter '%c' not Z, S or C.", vnum, letter );
				shutdown_mud( "bad mob data" );
				exit( 37 );
			}
			if( letter == 'C' || letter == 'Z' )  /* Realms complex mob     -Thoric  */
			{
				pMobIndex->perm_str = fread_number( fp );
				pMobIndex->perm_int = fread_number( fp );
				pMobIndex->perm_wis = fread_number( fp );
				pMobIndex->perm_dex = fread_number( fp );
				pMobIndex->perm_con = fread_number( fp );
				pMobIndex->perm_cha = fread_number( fp );
				pMobIndex->perm_lck = fread_number( fp );
				pMobIndex->saving_poison_death = fread_number( fp );
				pMobIndex->saving_wand = fread_number( fp );
				pMobIndex->saving_para_petri = fread_number( fp );
				pMobIndex->saving_breath = fread_number( fp );
				pMobIndex->saving_spell_staff = fread_number( fp );
				ln = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
				sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
				pMobIndex->race = x1;
				pMobIndex->height = x3;
				pMobIndex->weight = x4;
				pMobIndex->numattacks = x7;

				ln = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
				sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
				pMobIndex->hitroll = x1;
				pMobIndex->damroll = x2;
				pMobIndex->xflags = x3;
				pMobIndex->resistant = x4;
				pMobIndex->immune = x5;
				pMobIndex->susceptible = x6;
				pMobIndex->attacks = x7;
				pMobIndex->defenses = x8;
			}
			else
			{
				pMobIndex->perm_str = 10;
				pMobIndex->perm_dex = 10;
				pMobIndex->perm_int = 10;
				pMobIndex->perm_wis = 10;
				pMobIndex->perm_cha = 10;
				pMobIndex->perm_con = 10;
				pMobIndex->perm_lck = 10;
				pMobIndex->race = 0;
				pMobIndex->xflags = 0;
				pMobIndex->resistant = 0;
				pMobIndex->immune = 0;
				pMobIndex->susceptible = 0;
				pMobIndex->numattacks = 0;
				pMobIndex->attacks = 0;
				pMobIndex->defenses = 0;
			}
			if( letter == 'Z' )   /*  STar Wars Reality Complex Mob  */
			{
				ln = fread_line( fp );
				x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
				sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
				pMobIndex->vip_flags = x1;
			}

			letter = fread_letter( fp );
			if( letter == '>' )
			{
				ungetc( letter, fp );
				mprog_read_programs( fp, pMobIndex );
			}
			else
				ungetc( letter, fp );

			if( !oldmob )
			{
				iHash = vnum % MAX_KEY_HASH;
				pMobIndex->next = mob_index_hash[iHash];
				mob_index_hash[iHash] = pMobIndex;
				top_mob_index++;
			}
	}

	return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA *tarea, FILE *fp )
{
	OBJ_INDEX_DATA *pObjIndex;
	char letter;
	char *ln;
	int x1, x2, x3, x4, x5, x6;

	if( !tarea )
	{
		bug( "Load_objects: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 38 );
		}
		else
			return;
	}

	for( ;; )
	{
		char buf[MAX_STRING_LENGTH];
		int vnum;
		int iHash;
		bool tmpBootDb;
		bool oldobj;

		letter = fread_letter( fp );
		if( letter != '#' )
		{
			bug( "Load_objects: # not found." );
			if( fBootDb )
			{
				shutdown_mud( "# not found" );
				exit( 39 );
			}
			else
				return;
		}

		vnum = fread_number( fp );
		if( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = false;
		if( get_obj_index( vnum ) )
		{
			if( tmpBootDb )
			{
				bug( "Load_objects: vnum %d duplicated.", vnum );
				shutdown_mud( "duplicate vnum" );
				exit( 40 );
			}
			else
			{
				pObjIndex = get_obj_index( vnum );
				snprintf( buf, MAX_STRING_LENGTH, "Cleaning object: %d", vnum );
				log_string_plus( buf, LOG_BUILD, sysdata.log_level );
				clean_obj( pObjIndex );
				oldobj = true;
			}
		}
		else
		{
			oldobj = false;
			CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
		}
		fBootDb = tmpBootDb;

		pObjIndex->vnum = vnum;
		if( fBootDb )
		{
			if( !tarea->low_vnum )
				tarea->low_vnum = vnum;
			if( vnum > tarea->hi_vnum )
				tarea->hi_vnum = vnum;
		}
		pObjIndex->name = fread_string( fp );
		pObjIndex->short_descr = fread_string( fp );
		pObjIndex->description = fread_string( fp );
		pObjIndex->action_desc = fread_string( fp );

		/*
		 * Commented out by Narn, Apr/96 to allow item short descs like
		 * Bonecrusher and Oblivion
		 */
		 /*
		  * pObjIndex->short_descr[0]    = LOWER(pObjIndex->short_descr[0]);
		  */
		( ( char * ) pObjIndex->description )[0] = UPPER( pObjIndex->description[0] );

		//      ln = fread_line( fp );
		x1 = x2 = x3 = x4 = 0;
		/*      sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );
			  pObjIndex->item_type = x1;
			  pObjIndex->extra_flags = x2;
			  pObjIndex->wear_flags = x3;
			  pObjIndex->layers = x4;*/

		pObjIndex->item_type = fread_number( fp );
		pObjIndex->extra_flags = fread_bitvector( fp );
		pObjIndex->wear_flags = fread_number( fp );
		pObjIndex->layers = fread_number( fp );

		ln = fread_line( fp );
		x1 = x2 = x3 = x4 = x5 = x6 = 0;
		sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
		pObjIndex->value[0] = x1;
		pObjIndex->value[1] = x2;
		pObjIndex->value[2] = x3;
		pObjIndex->value[3] = x4;
		pObjIndex->value[4] = x5;
		pObjIndex->value[5] = x6;
		pObjIndex->weight = fread_number( fp );
		pObjIndex->weight = UMAX( 1, pObjIndex->weight );
		pObjIndex->cost = fread_number( fp );
		pObjIndex->rent = fread_number( fp ); /* unused */

		for( ;; )
		{
			letter = fread_letter( fp );

			if( letter == 'A' )
			{
				AFFECT_DATA *paf;

				CREATE( paf, AFFECT_DATA, 1 );
				paf->type = -1;
				paf->duration = -1;
				paf->location = fread_number( fp );
				if( paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_STRIPSN )
					paf->modifier = slot_lookup( fread_number( fp ) );
				else
					paf->modifier = fread_number( fp );
				paf->bitvector = 0;
				LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
				top_affect++;
			}

			else if( letter == 'E' )
			{
				EXTRA_DESCR_DATA *ed;

				CREATE( ed, EXTRA_DESCR_DATA, 1 );
				ed->keyword = fread_string( fp );
				ed->description = fread_string( fp );
				LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
				top_ed++;
			}

			else if( letter == '>' )
			{
				ungetc( letter, fp );
				oprog_read_programs( fp, pObjIndex );
			}

			else
			{
				ungetc( letter, fp );
				break;
			}
		}

		/*
		 * Translate spell "slot numbers" to internal "skill numbers."
		 */
		switch( pObjIndex->item_type )
		{
		case ITEM_PILL:
		case ITEM_POTION:
			pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
			pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
			pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
			break;

		case ITEM_DEVICE:
			pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
			break;
		case ITEM_SALVE:
			pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
			pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
			break;
		}

		if( !oldobj )
		{
			iHash = vnum % MAX_KEY_HASH;
			pObjIndex->next = obj_index_hash[iHash];
			obj_index_hash[iHash] = pObjIndex;
			top_obj_index++;
		}
	}

	return;
}



/*
 * Load a reset section.
 */
void load_resets( AREA_DATA *tarea, FILE *fp )
{
	ROOM_INDEX_DATA *pRoomIndex = NULL;
	ROOM_INDEX_DATA *roomlist;
	bool not01 = false;
	int count = 0;

	if( !tarea )
	{
		bug( "%s", "Load_resets: no #AREA seen yet." );
		if( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 41 );
		}
		else
			return;
	}

	if( !tarea->first_room )
	{
		bug( "%s: No #ROOMS section found. Cannot load resets.", __FUNCTION__ );
		if( fBootDb )
		{
			shutdown_mud( "No #ROOMS" );
			exit( 42 );
		}
		else
			return;
	}

	for( ;; )
	{
		EXIT_DATA *pexit;
		char letter;
		int extra, arg1, arg2, arg3;

		if( ( letter = fread_letter( fp ) ) == 'S' )
			break;

		if( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		extra = fread_number( fp );
		if( letter == 'M' || letter == 'O' )
			extra = 0;
		arg1 = fread_number( fp );
		arg2 = fread_number( fp );
		arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
		fread_to_eol( fp );
		++count;

		/*
		 * Validate parameters.
		 * We're calling the index functions for the side effect.
		 */
		switch( letter )
		{
		default:
			bug( "%s: bad command '%c'.", __FUNCTION__, letter );
			if( fBootDb )
				boot_log( "%s: %s (%d) bad command '%c'.", __FUNCTION__, tarea->filename, count, letter );
			return;

		case 'M':
			if( get_mob_index( arg1 ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );

			if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) 'M': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg3 );
			else
				add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			break;

		case 'O':
			if( get_obj_index( arg1 ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );

			if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) '%c': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg3 );
			else
			{
				if( !pRoomIndex )
					bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
				else
					add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			}
			break;

		case 'P':
			if( get_obj_index( arg1 ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );
			if( arg3 > 0 )
			{
				if( get_obj_index( arg3 ) == NULL && fBootDb )
					boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __FUNCTION__, tarea->filename, count,
						arg3 );
				if( extra > 1 )
					not01 = true;
			}
			if( !pRoomIndex )
				bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
			else
			{
				if( arg3 == 0 )
					arg3 = OBJ_VNUM_MONEY_ONE;    // This may look stupid, but for some reason it works.
				add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			}
			break;

		case 'G':
		case 'E':
			if( get_obj_index( arg1 ) == NULL && fBootDb )
				boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );
			if( !pRoomIndex )
				bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
			else
				add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			break;

		case 'T':
			if( IS_SET( extra, TRAP_OBJ ) )
				bug( "%s: Unable to add legacy object trap reset. Must be converted manually.", __FUNCTION__ );
			else
			{
				if( !( pRoomIndex = get_room_index( arg3 ) ) )
					bug( "%s: Unable to add trap reset - room not found.", __FUNCTION__ );
				else
					add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			}
			break;

		case 'H':
			bug( "%s: Unable to convert legacy hide reset. Must be converted manually.", __FUNCTION__ );
			break;

		case 'D':
			if( !( pRoomIndex = get_room_index( arg1 ) ) )
			{
				bug( "%s: 'D': room %d doesn't exist.", __FUNCTION__, arg1 );
				bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
				if( fBootDb )
					boot_log( "%s: %s (%d) 'D': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );
				break;
			}

			if( arg2 < 0 || arg2 > MAX_DIR + 1
				|| !( pexit = get_exit( pRoomIndex, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
			{
				bug( "%s: 'D': exit %d not door.", __FUNCTION__, arg2 );
				bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
				if( fBootDb )
					boot_log( "%s: %s (%d) 'D': exit %d not door.", __FUNCTION__, tarea->filename, count, arg2 );
			}

			if( arg3 < 0 || arg3 > 2 )
			{
				bug( "%s: 'D': bad 'locks': %d.", __FUNCTION__, arg3 );
				if( fBootDb )
					boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __FUNCTION__, tarea->filename, count, arg3 );
			}
			add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			break;

		case 'R':
			if( !( pRoomIndex = get_room_index( arg1 ) ) && fBootDb )
				boot_log( "%s: %s (%d) 'R': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );
			else
				add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
			if( arg2 < 0 || arg2 > 10 )
			{
				bug( "%s: 'R': bad exit %d.", __FUNCTION__, arg2 );
				if( fBootDb )
					boot_log( "%s: %s (%d) 'R': bad exit %d.", __FUNCTION__, tarea->filename, count, arg2 );
				break;
			}
			break;
		}
	}
	if( !not01 )
	{
		for( roomlist = tarea->first_room; roomlist; roomlist = roomlist->next_aroom )
			renumber_put_resets( roomlist );
	}
	return;
}

void load_room_reset( ROOM_INDEX_DATA *room, FILE *fp )
{
	EXIT_DATA *pexit;
	char letter;
	int extra, arg1, arg2, arg3;
	bool not01 = false;
	int count = 0;

	letter = fread_letter( fp );
	extra = fread_number( fp );
	if( letter == 'M' || letter == 'O' )
		extra = 0;
	arg1 = fread_number( fp );
	arg2 = fread_number( fp );
	arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
	fread_to_eol( fp );
	++count;

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	switch( letter )
	{
	default:
		bug( "%s: bad command '%c'.", __FUNCTION__, letter );
		if( fBootDb )
			boot_log( "%s: %s (%d) bad command '%c'.", __FUNCTION__, room->area->filename, count, letter );
		return;

	case 'M':
		if( get_mob_index( arg1 ) == NULL && fBootDb )
			boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __FUNCTION__, room->area->filename, count, arg1 );
		break;

	case 'O':
		if( get_obj_index( arg1 ) == NULL && fBootDb )
			boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
				arg1 );
		break;

	case 'P':
		if( get_obj_index( arg1 ) == NULL && fBootDb )
			boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
				arg1 );

		if( arg3 <= 0 )
			arg3 = OBJ_VNUM_MONEY_ONE;  // This may look stupid, but for some reason it works.
		if( get_obj_index( arg3 ) == NULL && fBootDb )
			boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __FUNCTION__, room->area->filename, count,
				arg3 );
		if( extra > 1 )
			not01 = true;
		break;

	case 'G':
	case 'E':
		if( get_obj_index( arg1 ) == NULL && fBootDb )
			boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
				arg1 );
		break;

	case 'T':
	case 'H':
		break;

	case 'D':
		if( arg2 < 0 || arg2 > MAX_DIR + 1
			|| !( pexit = get_exit( room, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
		{
			bug( "%s: 'D': exit %d not door.", __FUNCTION__, arg2 );
			bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
			if( fBootDb )
				boot_log( "%s: %s (%d) 'D': exit %d not door.", __FUNCTION__, room->area->filename, count, arg2 );
		}

		if( arg3 < 0 || arg3 > 2 )
		{
			bug( "%s: 'D': bad 'locks': %d.", __FUNCTION__, arg3 );
			if( fBootDb )
				boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __FUNCTION__, room->area->filename, count, arg3 );
		}
		break;

	case 'R':
		if( arg2 < 0 || arg2 > 10 )
		{
			bug( "%s: 'R': bad exit %d.", __FUNCTION__, arg2 );
			if( fBootDb )
				boot_log( "%s: %s (%d) 'R': bad exit %d.", __FUNCTION__, room->area->filename, count, arg2 );
			break;
		}
		break;
	}
	add_reset( room, letter, extra, arg1, arg2, arg3 );

	if( !not01 )
		renumber_put_resets( room );
	return;
}

/*
 * Load a room section.
 */
void load_rooms( AREA_DATA *tarea, FILE *fp )
{
	ROOM_INDEX_DATA *pRoomIndex;
	char buf[MAX_STRING_LENGTH];
	char *ln;

	if( !tarea )
	{
		bug( "Load_rooms: no #AREA seen yet." );
		shutdown_mud( "No #AREA" );
		exit( 43 );
	}
	tarea->first_room = tarea->last_room = NULL;

	for( ;; )
	{
		int vnum;
		char letter;
		int door;
		int iHash;
		bool tmpBootDb;
		bool oldroom;
		int dummy;
		int x1, x2, x3, x4, x5, x6, x7, x8;

		letter = fread_letter( fp );
		if( letter != '#' )
		{
			bug( "Load_rooms: # not found." );
			if( fBootDb )
			{
				shutdown_mud( "# not found" );
				exit( 44 );
			}
			else
				return;
		}

		vnum = fread_number( fp );
		if( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = false;
		if( get_room_index( vnum ) != NULL )
		{
			if( tmpBootDb )
			{
				bug( "Load_rooms: vnum %d duplicated.", vnum );
				shutdown_mud( "duplicate vnum" );
				exit( 45 );
			}
			else
			{
				pRoomIndex = get_room_index( vnum );
				snprintf( buf, MAX_STRING_LENGTH, "Cleaning room: %d", vnum );
				log_string_plus( buf, LOG_BUILD, sysdata.log_level );
				clean_room( pRoomIndex );
				oldroom = true;
			}
		}
		else
		{
			oldroom = false;
			CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
			pRoomIndex->first_person = NULL;
			pRoomIndex->last_person = NULL;
			pRoomIndex->first_content = NULL;
			pRoomIndex->last_content = NULL;
		}

		fBootDb = tmpBootDb;
		pRoomIndex->area = tarea;
		pRoomIndex->vnum = vnum;
		pRoomIndex->first_extradesc = NULL;
		pRoomIndex->last_extradesc = NULL;

		if( fBootDb )
		{
			if( !tarea->low_vnum )
				tarea->low_vnum = vnum;
			if( vnum > tarea->hi_vnum )
				tarea->hi_vnum = vnum;
		}
		pRoomIndex->name = fread_string( fp );
		pRoomIndex->description = fread_string( fp );

		if( pRoomIndex->area->version < 1 )
		{
			/*
			 * Area number         fread_number( fp );
			 */
			ln = fread_line( fp );
			x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
			sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );

			pRoomIndex->room_flags.bits[0] = x2;
			pRoomIndex->room_flags.bits[1] = x3 >> 1;
			pRoomIndex->sector_type = x4;
			pRoomIndex->tele_delay = x6;
			pRoomIndex->tele_vnum = x7;
			pRoomIndex->tunnel = x8;
		}

		else
		{
			dummy = fread_number( fp );
			pRoomIndex->room_flags = fread_bitvector( fp );
			dummy = fread_number( fp );
			pRoomIndex->sector_type = fread_number( fp );
			dummy = fread_number( fp );
			pRoomIndex->tele_delay = fread_number( fp );
			pRoomIndex->tele_vnum = fread_number( fp );
			pRoomIndex->tunnel = fread_number( fp );
		}

		if( pRoomIndex->sector_type < 0 || pRoomIndex->sector_type >= SECT_MAX )
		{
			bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum, pRoomIndex->sector_type );
			pRoomIndex->sector_type = 1;
		}
		pRoomIndex->light = 0;
		pRoomIndex->first_exit = NULL;
		pRoomIndex->last_exit = NULL;
		pRoomIndex->guests = STRALLOC( "" );

		for( ;; )
		{
			letter = fread_letter( fp );

			if( letter == 'S' )
				break;

			else if( letter == 'D' )
			{
				EXIT_DATA *pexit;
				int locks;

				door = fread_number( fp );
				if( door < 0 || door > 10 )
				{
					bug( "Fread_rooms: vnum %d has bad door number %d.", vnum, door );
					if( fBootDb )
						exit( 46 );
				}
				else
				{
					pexit = make_exit( pRoomIndex, NULL, door );
					pexit->description = fread_string( fp );
					pexit->keyword = fread_string( fp );
					pexit->exit_info = 0;
					ln = fread_line( fp );
					x1 = x2 = x3 = x4 = 0;
					sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

					locks = x1;
					pexit->key = x2;
					pexit->vnum = x3;
					pexit->vdir = door;
					pexit->distance = x4;

					switch( locks )
					{
					case 1:
						pexit->exit_info = EX_ISDOOR;
						break;
					case 2:
						pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
						break;
					default:
						pexit->exit_info = locks;
					}
				}
			}

			else if( letter == 'O' )
			{
				STRFREE( pRoomIndex->owner );
				pRoomIndex->owner = STRALLOC( fread_string( fp ) );
			}

			else if( letter == 'G' )
			{
				STRFREE( pRoomIndex->guests );
				pRoomIndex->guests = STRALLOC( fread_string( fp ) );
			}

			else if( letter == 'E' )
			{
				EXTRA_DESCR_DATA *ed;

				CREATE( ed, EXTRA_DESCR_DATA, 1 );
				ed->keyword = fread_string( fp );
				ed->description = fread_string( fp );
				LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev );
				top_ed++;
			}
			else if( letter == 'R' )
				load_room_reset( pRoomIndex, fp );
			else if( letter == '>' )
			{
				ungetc( letter, fp );
				rprog_read_programs( fp, pRoomIndex );
			}
			else
			{
				bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum, letter );
				shutdown_mud( "Room flag not DES" );
				exit( 47 );
			}

		}

		if( !oldroom )
		{
			iHash = vnum % MAX_KEY_HASH;
			pRoomIndex->next = room_index_hash[iHash];
			room_index_hash[iHash] = pRoomIndex;
			LINK( pRoomIndex, tarea->first_room, tarea->last_room, next_aroom, prev_aroom );
			top_room++;
		}
		if( dummy != 0 )
			dummy = 0;
	}

	return;
}



/*
 * Load a shop section.
 */
void load_shops( AREA_DATA *tarea, FILE *fp )
{
	SHOP_DATA *pShop;

	for( ;; )
	{
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		CREATE( pShop, SHOP_DATA, 1 );
		pShop->keeper = fread_number( fp );
		if( pShop->keeper == 0 )
			break;
		for( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			pShop->buy_type[iTrade] = fread_number( fp );
		pShop->profit_buy = fread_number( fp );
		pShop->profit_sell = fread_number( fp );
		pShop->profit_buy = URANGE( pShop->profit_sell + 5, pShop->profit_buy, 1000 );
		pShop->profit_sell = URANGE( 0, pShop->profit_sell, pShop->profit_buy - 5 );
		pShop->open_hour = fread_number( fp );
		pShop->close_hour = fread_number( fp );
		fread_to_eol( fp );
		pMobIndex = get_mob_index( pShop->keeper );
		pMobIndex->pShop = pShop;

		if( !first_shop )
			first_shop = pShop;
		else
			last_shop->next = pShop;
		pShop->next = NULL;
		pShop->prev = last_shop;
		last_shop = pShop;
		top_shop++;
	}
	return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( AREA_DATA *tarea, FILE *fp )
{
	REPAIR_DATA *rShop;

	for( ;; )
	{
		MOB_INDEX_DATA *pMobIndex;
		int iFix;

		CREATE( rShop, REPAIR_DATA, 1 );
		rShop->keeper = fread_number( fp );
		if( rShop->keeper == 0 )
			break;
		for( iFix = 0; iFix < MAX_FIX; iFix++ )
			rShop->fix_type[iFix] = fread_number( fp );
		rShop->profit_fix = fread_number( fp );
		rShop->shop_type = fread_number( fp );
		rShop->open_hour = fread_number( fp );
		rShop->close_hour = fread_number( fp );
		fread_to_eol( fp );
		pMobIndex = get_mob_index( rShop->keeper );
		pMobIndex->rShop = rShop;

		if( !first_repair )
			first_repair = rShop;
		else
			last_repair->next = rShop;
		rShop->next = NULL;
		rShop->prev = last_repair;
		last_repair = rShop;
		top_repair++;
	}
	return;
}


/*
 * Load spec proc declarations.
 */
void load_specials( AREA_DATA *tarea, FILE *fp )
{
	for( ;; )
	{
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch( letter = fread_letter( fp ) )
		{
		default:
			bug( "Load_specials: letter '%c' not *MS.", letter );
			exit( 48 );

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex = get_mob_index( fread_number( fp ) );
			if( !pMobIndex->spec_fun )
			{
				pMobIndex->spec_fun = spec_lookup( fread_word( fp ) );

				if( pMobIndex->spec_fun == 0 )
				{
					bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
					exit( 49 );
				}
			}
			else if( !pMobIndex->spec_2 )
			{
				pMobIndex->spec_2 = spec_lookup( fread_word( fp ) );

				if( pMobIndex->spec_2 == 0 )
				{
					bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
					exit( 50 );
				}
			}

			break;
		}

		fread_to_eol( fp );
	}
}


/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA *tarea, FILE *fp )
{
	int x1, x2, x3, x4;
	char *ln;

	if( !tarea )
	{
		bug( "Load_ranges: no #AREA seen yet." );
		shutdown_mud( "No #AREA" );
		exit( 51 );
	}

	for( ;; )
	{
		ln = fread_line( fp );

		if( ln[0] == '$' )
			break;

		x1 = x2 = x3 = x4 = 0;
		sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

		tarea->low_soft_range = x1;
		tarea->hi_soft_range = x2;
		tarea->low_hard_range = x3;
		tarea->hi_hard_range = x4;
	}
	return;

}

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
	AREA_DATA *tarea;
	MOB_INDEX_DATA *mob;
	int idx, gold, rng;

	for( tarea = first_area; tarea; tarea = tarea->next )
	{
		/*
		 * skip area if they already got some gold
		 */
		if( tarea->high_economy > 0 || tarea->low_economy > 10000 )
			continue;
		rng = tarea->hi_soft_range - tarea->low_soft_range;
		if( rng )
			rng /= 2;
		else
			rng = 25;
		gold = rng * rng * 10000;
		boost_economy( tarea, gold );
		for( idx = tarea->low_vnum; idx < tarea->hi_vnum; idx++ )
			if( ( mob = get_mob_index( idx ) ) != NULL )
				boost_economy( tarea, mob->gold * 10 );
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit, *pexit_next, *rev_exit;
	int iHash;

	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
		{
			bool fexit;

			fexit = false;
			for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
			{
				pexit_next = pexit->next;
				pexit->rvnum = pRoomIndex->vnum;
				if( pexit->vnum <= 0 || ( pexit->to_room = get_room_index( pexit->vnum ) ) == NULL )
				{
					if( fBootDb )
						boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
							pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );

					bug( "Deleting %s exit in room %d", dir_name[pexit->vdir], pRoomIndex->vnum );
					extract_exit( pRoomIndex, pexit );
				}
				else
					fexit = true;
			}
			if( !fexit )
				xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
		}
	}

	/*
	 * Set all the rexit pointers   -Thoric
	 */
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
		{
			for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
			{
				if( pexit->to_room && !pexit->rexit )
				{
					rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
					if( rev_exit )
					{
						pexit->rexit = rev_exit;
						rev_exit->rexit = pexit;
					}
				}
			}
		}
	}

	return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA *room, int xit )
{
	EXIT_DATA *pexit;
	int count;

	count = 0;
	for( pexit = room->first_exit; pexit; pexit = pexit->next )
		if( ++count == xit )
			return pexit;
	return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
	int d1, d2;

	d1 = ( *xit1 )->vdir;
	d2 = ( *xit2 )->vdir;

	if( d1 < d2 )
		return -1;
	if( d1 > d2 )
		return 1;
	return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
	EXIT_DATA *pexit;    /* *texit *//* Unused */
	EXIT_DATA *exits[MAX_REXITS];
	int x, nexits;

	nexits = 0;
	for( pexit = room->first_exit; pexit; pexit = pexit->next )
	{
		exits[nexits++] = pexit;
		if( nexits > MAX_REXITS )
		{
			bug( "sort_exits: more than %d exits in room... fatal", nexits );
			return;
		}
	}
	qsort( &exits[0], nexits, sizeof( EXIT_DATA * ), ( int ( * )( const void *, const void * ) )exit_comp );
	for( x = 0; x < nexits; x++ )
	{
		if( x > 0 )
			exits[x]->prev = exits[x - 1];
		else
		{
			exits[x]->prev = NULL;
			room->first_exit = exits[x];
		}
		if( x >= ( nexits - 1 ) )
		{
			exits[x]->next = NULL;
			room->last_exit = exits[x];
		}
		else
			exits[x]->next = exits[x + 1];
	}
}

void randomize_exits( ROOM_INDEX_DATA *room, short maxdir )
{
	EXIT_DATA *pexit;
	int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
	int vdirs[MAX_REXITS];

	nexits = 0;
	for( pexit = room->first_exit; pexit; pexit = pexit->next )
		vdirs[nexits++] = pexit->vdir;

	for( d0 = 0; d0 < nexits; d0++ )
	{
		if( vdirs[d0] > maxdir )
			continue;
		count = 0;
		while( vdirs[( d1 = number_range( d0, nexits - 1 ) )] > maxdir || ++count > 5 );
		if( vdirs[d1] > maxdir )
			continue;
		door = vdirs[d0];
		vdirs[d0] = vdirs[d1];
		vdirs[d1] = door;
	}
	count = 0;
	for( pexit = room->first_exit; pexit; pexit = pexit->next )
		pexit->vdir = vdirs[count++];

	sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
	AREA_DATA *pArea;

	for( pArea = first_area; pArea; pArea = pArea->next )
	{
		CHAR_DATA *pch;
		int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

		if( ( reset_age == -1 && pArea->age == -1 ) || ++pArea->age < ( reset_age - 1 ) )
			continue;

		/*
		 * Check for PC's.
		 */
		if( pArea->nplayer > 0 && pArea->age == ( reset_age - 1 ) )
		{
			char buf[MAX_STRING_LENGTH];

			/*
			 * Rennard
			 */
			if( pArea->resetmsg )
				snprintf( buf, MAX_STRING_LENGTH, "%s\r\n", pArea->resetmsg );
			else
				strcpy( buf, "You hear some squeaking sounds...\r\n" );
			for( pch = first_char; pch; pch = pch->next )
			{
				if( !IS_NPC( pch ) && IS_AWAKE( pch ) && pch->in_room && pch->in_room->area == pArea )
				{
					set_char_color( AT_RESET, pch );
					send_to_char( buf, pch );
				}
			}
		}

		/*
		 * Check age and reset.
		 * Note: Mud Academy resets every 3 minutes (not 15).
		 */
		if( pArea->nplayer == 0 || pArea->age >= reset_age )
		{
			ROOM_INDEX_DATA *pRoomIndex;

			//      fprintf( stderr, "Resetting: %s\n", pArea->filename );
			reset_area( pArea );
			if( reset_age == -1 )
				pArea->age = -1;
			else
				pArea->age = number_range( 0, reset_age / 5 );
			pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
			if( pRoomIndex != NULL && pArea == pRoomIndex->area && pArea->reset_frequency == 0 )
				pArea->age = 15 - 3;
		}
	}
	return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
	CHAR_DATA *mob;

	if( !pMobIndex )
	{
		bug( "Create_mobile: NULL pMobIndex." );
		exit( 52 );
	}

	CREATE( mob, CHAR_DATA, 1 );
	clear_char( mob );
	mob->pIndexData = pMobIndex;

	mob->editor = NULL;
	mob->name = QUICKLINK( pMobIndex->player_name );
	mob->short_descr = QUICKLINK( pMobIndex->short_descr );
	mob->long_descr = QUICKLINK( pMobIndex->long_descr );
	mob->description = QUICKLINK( pMobIndex->description );
	mob->spec_fun = pMobIndex->spec_fun;
	mob->spec_2 = pMobIndex->spec_2;
	if( pMobIndex->spec_funname )
		mob->spec_funname = QUICKLINK( pMobIndex->spec_funname );
	if( pMobIndex->spec_funname2 )
		mob->spec_funname2 = QUICKLINK( pMobIndex->spec_funname2 );
	mob->mpscriptpos = 0;
	mob->top_level = number_fuzzy( pMobIndex->level );
	{
		int ability;
		for( ability = 0; ability < MAX_ABILITY; ability++ )
			mob->skill_level[ability] = mob->top_level;
	}
	mob->act = pMobIndex->act;
	mob->home_vnum = -1;
	mob->resetvnum = -1;
	mob->resetnum = -1;
	mob->affected_by = pMobIndex->affected_by;
	mob->alignment = pMobIndex->alignment;
	mob->sex = pMobIndex->sex;
	mob->main_ability = 0;
	mob->mob_clan = STRALLOC( "" );
	mob->was_sentinel = NULL;
	mob->plr_home = NULL;
	mob->guard_data = NULL;

	if( pMobIndex->ac )
		mob->armor = pMobIndex->ac;
	else
		mob->armor = 100 - mob->top_level * 2.5;

	if( !pMobIndex->hitnodice )
		mob->max_hit = mob->top_level * 10 + number_range( mob->top_level, mob->top_level * 10 );
	else
		mob->max_hit = pMobIndex->hitnodice * number_range( 1, pMobIndex->hitsizedice ) + pMobIndex->hitplus;
	mob->hit = mob->max_hit;
	/*
	 * lets put things back the way they used to be! -Thoric
	 */
	mob->gold = pMobIndex->gold;
	mob->position = pMobIndex->position;
	mob->defposition = pMobIndex->defposition;
	mob->barenumdie = pMobIndex->damnodice;
	mob->baresizedie = pMobIndex->damsizedice;
	mob->mobthac0 = pMobIndex->mobthac0;
	mob->hitplus = pMobIndex->hitplus;
	mob->damplus = pMobIndex->damplus;

	mob->perm_str = pMobIndex->perm_str;
	mob->perm_dex = pMobIndex->perm_dex;
	mob->perm_wis = pMobIndex->perm_wis;
	mob->perm_int = pMobIndex->perm_int;
	mob->perm_con = pMobIndex->perm_con;
	mob->perm_cha = pMobIndex->perm_cha;
	mob->perm_lck = pMobIndex->perm_lck;
	mob->hitroll = pMobIndex->hitroll;
	mob->damroll = pMobIndex->damroll;
	mob->race = pMobIndex->race;
	mob->xflags = pMobIndex->xflags;
	mob->saving_poison_death = pMobIndex->saving_poison_death;
	mob->saving_wand = pMobIndex->saving_wand;
	mob->saving_para_petri = pMobIndex->saving_para_petri;
	mob->saving_breath = pMobIndex->saving_breath;
	mob->saving_spell_staff = pMobIndex->saving_spell_staff;
	mob->height = pMobIndex->height;
	mob->weight = pMobIndex->weight;
	mob->resistant = pMobIndex->resistant;
	mob->immune = pMobIndex->immune;
	mob->susceptible = pMobIndex->susceptible;
	mob->attacks = pMobIndex->attacks;
	mob->defenses = pMobIndex->defenses;
	mob->numattacks = pMobIndex->numattacks;
	mob->vip_flags = pMobIndex->vip_flags;

	/*
	 * Insert in list.
	 */
	add_char( mob );
	pMobIndex->count++;
	nummobsloaded++;
	return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
	OBJ_DATA *obj;

	if( !pObjIndex )
	{
		bug( "Create_object: NULL pObjIndex." );
		exit( 53 );
	}

	CREATE( obj, OBJ_DATA, 1 );

	obj->pIndexData = pObjIndex;
	obj->in_room = NULL;
	obj->level = level;
	obj->wear_loc = -1;
	obj->count = 1;
	cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
	obj->serial = obj->pIndexData->serial = cur_obj_serial;

	obj->armed_by = STRALLOC( "" );
	obj->name = QUICKLINK( pObjIndex->name );
	obj->short_descr = QUICKLINK( pObjIndex->short_descr );
	obj->description = QUICKLINK( pObjIndex->description );
	obj->action_desc = QUICKLINK( pObjIndex->action_desc );
	obj->item_type = pObjIndex->item_type;
	obj->extra_flags = pObjIndex->extra_flags;
	obj->wear_flags = pObjIndex->wear_flags;
	obj->value[0] = pObjIndex->value[0];
	obj->value[1] = pObjIndex->value[1];
	obj->value[2] = pObjIndex->value[2];
	obj->value[3] = pObjIndex->value[3];
	obj->value[4] = pObjIndex->value[4];
	obj->value[5] = pObjIndex->value[5];
	obj->weight = pObjIndex->weight;
	obj->cost = pObjIndex->cost;
	/*
	 * obj->cost        = number_fuzzy( 10 )
	 * * number_fuzzy( level ) * number_fuzzy( level );
	 */

	 /*
	  * Mess with object properties.
	  */
	switch( obj->item_type )
	{
	default:
		bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
		bug( "------------------------>     ", obj->item_type );
		break;

	case ITEM_GOVERNMENT:
	case ITEM_SPACECRAFT:
	case ITEM_RAWSPICE:
	case ITEM_LENS:
	case ITEM_CRYSTAL:
	case ITEM_DURAPLAST:
	case ITEM_DURASTEEL:
	case ITEM_SUPERCONDUCTOR:
	case ITEM_COMLINK:
	case ITEM_MEDPAC:
	case ITEM_FABRIC:
	case ITEM_RARE_METAL:
	case ITEM_MAGNET:
	case ITEM_THREAD:
	case ITEM_CHEMICAL:
	case ITEM_SPICE:
	case ITEM_SMUT:
	case ITEM_OVEN:
	case ITEM_MIRROR:
	case ITEM_CIRCUIT:
	case ITEM_TOOLKIT:
	case ITEM_LIGHT:
	case ITEM_TREASURE:
	case ITEM_FURNITURE:
	case ITEM_TRASH:
	case ITEM_WHOLDER:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_KEY:
		break;
	case ITEM_FOOD:
		/*
		 * optional food condition (rotting food)       -Thoric
		 * value1 is the max condition of the food
		 * value4 is the optional initial condition
		 */
		if( obj->value[4] )
			obj->timer = obj->value[4];
		else
			obj->timer = obj->value[1];
		break;

	case ITEM_DROID_CORPSE:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_FOUNTAIN:
	case ITEM_SCRAPS:
	case ITEM_GRENADE:
	case ITEM_LANDMINE:
	case ITEM_FIRE:
	case ITEM_BOOK:
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_BUTTON:
	case ITEM_DIAL:
	case ITEM_TRAP:
	case ITEM_MAP:
	case ITEM_REMOTE:
	case ITEM_JUKEBOX:
	case ITEM_PTRAP:
	case ITEM_DRUG:
	case ITEM_MIXTURE:
	case ITEM_PAPER:
	case ITEM_PEN:
	case ITEM_LOCKPICK:
	case ITEM_FUEL:
	case ITEM_MISSILE:
	case ITEM_SHOVEL:
	case ITEM_RESTORE:
		break;

	case ITEM_SALVE:
		obj->value[3] = number_fuzzy( obj->value[3] );
		break;

	case ITEM_DEVICE:
		obj->value[0] = number_fuzzy( obj->value[0] );
		obj->value[1] = number_fuzzy( obj->value[1] );
		obj->value[2] = obj->value[1];
		break;

	case ITEM_BATTERY:
		if( obj->value[0] <= 0 )
			obj->value[0] = number_fuzzy( 95 );
		break;


	case ITEM_BOLT:
		if( obj->value[0] <= 0 )
			obj->value[0] = number_fuzzy( 95 );
		break;

	case ITEM_AMMO:
		if( obj->value[0] <= 0 )
			obj->value[0] = number_fuzzy( 495 );
		break;

	case ITEM_WEAPON:
		if( obj->value[1] && obj->value[2] )
			obj->value[2] *= obj->value[1];
		else
		{
			obj->value[1] = number_fuzzy( number_fuzzy( 1 + level / 20 ) );
			obj->value[2] = number_fuzzy( number_fuzzy( 10 + level / 10 ) );
		}
		if( obj->value[1] > obj->value[2] )
			obj->value[1] = obj->value[2] / 3;
		if( obj->value[0] == 0 )
			obj->value[0] = INIT_WEAPON_CONDITION;
		switch( obj->value[3] )
		{
		case WEAPON_BLASTER:
		case WEAPON_LIGHTSABER:
		case WEAPON_VIBRO_BLADE:
		case WEAPON_FORCE_PIKE:
		case WEAPON_BOWCASTER:
			if( obj->value[5] <= 0 )
				obj->value[5] = number_fuzzy( 1000 );
		}
		obj->value[4] = obj->value[5];
		break;

	case ITEM_ARMOR:
		if( obj->value[0] == 0 )
			obj->value[0] = obj->value[1];
		obj->timer = obj->value[3];
		break;

	case ITEM_POTION:
	case ITEM_PILL:
		obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
		break;

	case ITEM_MONEY:
		obj->value[0] = obj->cost;
		break;
	}

	LINK( obj, first_object, last_object, next, prev );
	++pObjIndex->count;
	++numobjsloaded;
	++physicalobjects;

	return obj;
}

/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
	ch->editor = NULL;
	ch->hunting = NULL;
	ch->fearing = NULL;
	ch->hating = NULL;
	ch->name = NULL;
	ch->short_descr = NULL;
	ch->long_descr = NULL;
	ch->description = NULL;
	ch->next = NULL;
	ch->prev = NULL;
	ch->first_carrying = NULL;
	ch->last_carrying = NULL;
	ch->next_in_room = NULL;
	ch->prev_in_room = NULL;
	ch->fighting = NULL;
	ch->switched = NULL;
	ch->first_affect = NULL;
	ch->last_affect = NULL;
	ch->prev_cmd = NULL; /* maps */
	ch->last_cmd = NULL;
	ch->dest_buf = NULL;
	ch->dest_buf_2 = NULL;
	ch->spare_ptr = NULL;
	ch->mount = NULL;
	ch->affected_by = 0;
	ch->logon = current_time;
	ch->armor = 100;
	ch->position = POS_STANDING;
	ch->hit = 500;
	ch->max_hit = 500;
	ch->move = 1000;
	ch->max_move = 1000;
	ch->height = 72;
	ch->weight = 180;
	ch->xflags = 0;
	ch->race = 0;
	ch->barenumdie = 1;
	ch->baresizedie = 4;
	ch->substate = 0;
	ch->tempnum = 0;
	ch->perm_str = 10;
	ch->perm_dex = 10;
	ch->perm_int = 10;
	ch->perm_wis = 10;
	ch->perm_cha = 10;
	ch->perm_con = 10;
	ch->perm_lck = 10;
	ch->mod_str = 0;
	ch->mod_dex = 0;
	ch->mod_int = 0;
	ch->mod_wis = 0;
	ch->mod_cha = 0;
	ch->mod_con = 0;
	ch->mod_lck = 0;
	ch->pagelen = 24;    /* BUILD INTERFACE */
	ch->inter_page = NO_PAGE;    /* BUILD INTERFACE */
	ch->inter_type = NO_TYPE;    /* BUILD INTERFACE */
	ch->inter_editing = NULL;    /* BUILD INTERFACE */
	ch->inter_editing_vnum = -1; /* BUILD INTERFACE */
	ch->inter_substate = SUB_NORTH;  /* BUILD INTERFACE */
	ch->plr_home = NULL;
	return;
}


void free_ignores( CHAR_DATA *ch )
{
	IGNORE_DATA *temp, *next;

	/*
	 * free up memory allocated to stored ignored names
	 */
	for( temp = ch->pcdata->first_ignored; temp; temp = next )
	{
		next = temp->next;
		UNLINK( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
		STRFREE( temp->name );
		DISPOSE( temp );
	}
	return;
}

/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	TIMER *timer;
	MPROG_ACT_LIST *mpact, *mpact_next;
	NOTE_DATA *comments, *comments_next;

	if( !ch )
	{
		bug( "Free_char: null ch!" );
		return;
	}

	if( ch->desc )
		bug( "Free_char: char still has descriptor." );

	while( ( obj = ch->last_carrying ) != NULL )
		extract_obj( obj );

	while( ( paf = ch->last_affect ) != NULL )
		affect_remove( ch, paf );

	while( ( timer = ch->first_timer ) != NULL )
		extract_timer( ch, timer );

	if( ch->name )
		STRFREE( ch->name );
	if( ch->short_descr )
		STRFREE( ch->short_descr );
	if( ch->long_descr )
		STRFREE( ch->long_descr );
	if( ch->description )
		STRFREE( ch->description );
	if( ch->spec_funname )
		STRFREE( ch->spec_funname );
	if( ch->spec_funname2 )
		STRFREE( ch->spec_funname2 );
	if( ch->mob_clan )
		STRFREE( ch->mob_clan );
	if( ch->mailbuf )
		STRFREE( ch->mailbuf );
	if( ch->editor )
		stop_editing( ch );

	DISPOSE( ch->inter_editing );

	STRFREE( ch->gang );
	STRFREE( ch->mob_clan );

	stop_hunting( ch );
	stop_hating( ch );
	stop_fearing( ch );
	xCLEAR_BITS( ch->act );

	if( ch->pcdata )
	{
		BIT_DATA *qbit, *qbit_next;

		STRFREE( ch->pcdata->clan_name );
		DISPOSE( ch->pcdata->pwd );
		DISPOSE( ch->pcdata->email );
		DISPOSE( ch->pcdata->bamfin );
		DISPOSE( ch->pcdata->bamfout );
		DISPOSE( ch->pcdata->aim );
		DISPOSE( ch->pcdata->real );
		DISPOSE( ch->pcdata->msn );
		DISPOSE( ch->pcdata->yim );
		DISPOSE( ch->pcdata->enter );
		DISPOSE( ch->pcdata->exit );
		STRFREE( ch->pcdata->title );
		STRFREE( ch->pcdata->bio );
		STRFREE( ch->pcdata->rank );
		DISPOSE( ch->pcdata->bestowments );
		DISPOSE( ch->pcdata->homepage );
		STRFREE( ch->pcdata->authed_by );
		STRFREE( ch->pcdata->prompt );
		STRFREE( ch->pcdata->subprompt );
		DISPOSE( ch->pcdata->job );
		STRFREE( ch->pcdata->tspouse );
		STRFREE( ch->pcdata->spouse );
		STRFREE( ch->pcdata->lasthost );
		DISPOSE( ch->pcdata->avatar );
		STRFREE( ch->pcdata->helled_by );
		DISPOSE( ch->pcdata->afkmess );
		DISPOSE( ch->pcdata->tellbuf );
		STRFREE( ch->pcdata->inivictim );
		STRFREE( ch->pcdata->iclan );

		for( qbit = ch->pcdata->firstqbit; qbit; qbit = qbit_next )
		{
			qbit_next = qbit->next;

			UNLINK( qbit, ch->pcdata->firstqbit, ch->pcdata->lastqbit, next, prev );
			DISPOSE( qbit );
		}

		free_ignores( ch );
		free_aliases( ch );

		DISPOSE( ch->pcdata );
	}

	for( mpact = ch->mpact; mpact; mpact = mpact_next )
	{
		mpact_next = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact );
	}

	for( comments = ch->comments; comments; comments = comments_next )
	{
		comments_next = comments->next;
		STRFREE( comments->text );
		STRFREE( comments->to_list );
		STRFREE( comments->subject );
		STRFREE( comments->sender );
		STRFREE( comments->date );
		STRFREE( comments->yesvotes );
		STRFREE( comments->novotes );
		STRFREE( comments->abstentions );
		DISPOSE( comments );
	}
	DISPOSE( ch );
	return;
}

/*
 * Get an extra description from a list.
 */
const char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
	for( ; ed; ed = ed->next )
		if( is_name( name, ed->keyword ) )
			return ed->description;

	return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
	MOB_INDEX_DATA *pMobIndex;

	if( vnum < 0 )
		vnum = 0;

	for( pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex; pMobIndex = pMobIndex->next )
		if( pMobIndex->vnum == vnum )
			return pMobIndex;

	if( fBootDb )
		bug( "Get_mob_index: bad vnum %d.", vnum );

	return NULL;
}

/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
	OBJ_INDEX_DATA *pObjIndex;

	if( vnum < 0 )
		vnum = 0;

	for( pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex; pObjIndex = pObjIndex->next )
		if( pObjIndex->vnum == vnum )
			return pObjIndex;

	if( fBootDb )
		bug( "Get_obj_index: bad vnum %d.", vnum );

	return NULL;
}

/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;

	if( vnum < 0 )
		vnum = 0;

	for( pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex; pRoomIndex = pRoomIndex->next )
		if( pRoomIndex->vnum == vnum )
			return pRoomIndex;

	if( fBootDb )
		bug( "Get_room_index: bad vnum %d.", vnum );

	return NULL;
}

/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */
 /*
  * Read a letter from a file.
  */
char fread_letter( FILE *fp )
{
	char c;

	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return '\0';
		}
		c = getc( fp );
	} while( isspace( c ) );

	return c;
}

/*
 * Read a float number from a file. Turn the result into a float value.
 */
float fread_float( FILE *fp )
{
	float number;
	bool sign, decimal;
	char c;
	double place = 0;

	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
			{
				shutdown_mud( "Corrupt file somewhere." );
				exit( 1 );
			}
			return 0;
		}
		c = getc( fp );
	} while( isspace( c ) );

	number = 0;

	sign = false;
	decimal = false;

	if( c == '+' )
		c = getc( fp );
	else if( c == '-' )
	{
		sign = true;
		c = getc( fp );
	}

	if( !isdigit( c ) )
	{
		bug( "%s: bad format. (%c)", __func__, c );
		if( fBootDb )
			exit( 1 );
		return 0;
	}

	while( 1 )
	{
		if( c == '.' || isdigit( c ) )
		{
			if( c == '.' )
			{
				decimal = true;
				c = getc( fp );
			}

			if( feof( fp ) )
			{
				bug( "%s: EOF encountered on read.", __func__ );
				if( fBootDb )
					exit( 1 );
				return number;
			}
			if( !decimal )
				number = number * 10 + c - '0';
			else
			{
				place++;
				number += pow( 10, ( -1 * place ) ) * ( c - '0' );
			}
			c = getc( fp );
		}
		else
			break;
	}

	if( sign )
		number = 0 - number;

	if( c == '|' )
		number += fread_float( fp );
	else if( c != ' ' )
		ungetc( c, fp );

	return number;
}

/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
	int number;
	bool sign;
	char c;

	do
	{
		if( feof( fp ) )
		{
			bug( "__func__: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return 0;
		}
		c = getc( fp );
	} while( isspace( c ) );

	number = 0;

	sign = false;
	if( c == '+' )
	{
		c = getc( fp );
	}
	else if( c == '-' )
	{
		sign = true;
		c = getc( fp );
	}

	if( !isdigit( c ) )
	{
		bug( "%s: bad format. (%c)", __func__, c );
		if( fBootDb )
			exit( 1 );
		return 0;
	}

	while( isdigit( c ) )
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return number;
		}
		number = number * 10 + c - '0';
		c = getc( fp );
	}

	if( sign )
		number = 0 - number;

	if( c == '|' )
		number += fread_number( fp );
	else if( c != ' ' )
		ungetc( c, fp );

	return number;
}

/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
	static char *ret;
	int len;

	if( !str )
		return NULL;

	len = strlen( str ) + 1;

	CREATE( ret, char, len );
	mudstrlcpy( ret, str, MAX_STRING_LENGTH );
	return ret;
}

bool is_valid_filename( CHAR_DATA *ch, const char *direct, const char *filename )
{
	char newfilename[256];
	struct stat fst;

	/* Length restrictions */
	if( !filename || filename[0] == '\0' || strlen( filename ) < 3 )
	{
		if( !filename || !str_cmp( filename, "" ) )
			send_to_char( "Empty filename is not valid.\r\n", ch );
		else
			ch_printf( ch, "%s: Filename is too short.\r\n", filename );
		return false;
	}

	/* Illegal characters */
	if( strstr( filename, ".." ) || strstr( filename, "/" ) || strstr( filename, "\\" ) )
	{
		send_to_char( "A filename may not contain a '..', '/', or '\\' in it.\r\n", ch );
		return false;
	}

	/* If that filename is already being used lets not allow it now to be on the safe side */
	snprintf( newfilename, sizeof( newfilename ), "%s%s", direct, filename );
	if( stat( newfilename, &fst ) != -1 )
	{
		ch_printf( ch, "%s is already an existing filename.\r\n", newfilename );
		return false;
	}

	/* If we got here assume its valid */
	return true;
}

/*
 * Read a string from file fp
 */
const char *fread_string( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	char c;
	int ln;

	plast = buf;
	buf[0] = '\0';
	ln = 0;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return STRALLOC( "" );
		}
		c = getc( fp );
	} while( isspace( c ) );

	if( ( *plast++ = c ) == '~' )
		return STRALLOC( "" );

	for( ;; )
	{
		if( ln >= ( MAX_STRING_LENGTH - 1 ) )
		{
			bug( "%s: string too long", __func__ );
			*plast = '\0';
			return STRALLOC( buf );
		}
		switch( *plast = getc( fp ) )
		{
		default:
			plast++;
			ln++;
			break;

		case EOF:
			bug( "%s: EOF", __func__ );
			if( fBootDb )
				exit( 1 );
			*plast = '\0';
			return STRALLOC( buf );
			break;

		case '\n':
			plast++;
			ln++;
			*plast++ = '\r';
			ln++;
			break;

		case '\r':
			break;

		case '~':
			*plast = '\0';
			return STRALLOC( buf );
		}
	}
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	char c;
	int ln;

	plast = buf;
	buf[0] = '\0';
	ln = 0;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return str_dup( "" );
		}
		c = getc( fp );
	} while( isspace( c ) );

	if( ( *plast++ = c ) == '~' )
		return str_dup( "" );

	for( ;; )
	{
		if( ln >= ( MAX_STRING_LENGTH - 1 ) )
		{
			bug( "%s: string too long", __func__ );
			*plast = '\0';
			return str_dup( buf );
		}
		switch( *plast = getc( fp ) )
		{
		default:
			plast++;
			ln++;
			break;

		case EOF:
			bug( "%s: EOF", __func__ );
			if( fBootDb )
				exit( 1 );
			*plast = '\0';
			return str_dup( buf );
			break;

		case '\n':
			plast++;
			ln++;
			*plast++ = '\r';
			ln++;
			break;

		case '\r':
			break;

		case '~':
			*plast = '\0';
			return str_dup( buf );
		}
	}
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
	char c;

	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			return;
		}
		c = getc( fp );
	} while( c != '\n' && c != '\r' );

	do
	{
		c = getc( fp );
	} while( c == '\n' || c == '\r' );

	ungetc( c, fp );
	return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp )
{
	static char line[MAX_STRING_LENGTH];
	char *pline;
	char c;
	int ln;

	pline = line;
	line[0] = '\0';
	ln = 0;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			mudstrlcpy( line, "", MAX_STRING_LENGTH );
			return line;
		}
		c = getc( fp );
	} while( isspace( c ) );

	ungetc( c, fp );
	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			*pline = '\0';
			return line;
		}
		c = getc( fp );
		*pline++ = c;
		ln++;
		if( ln >= ( MAX_STRING_LENGTH - 1 ) )
		{
			bug( "%s: line too long", __func__ );
			break;
		}
	} while( c != '\n' && c != '\r' );

	do
	{
		c = getc( fp );
	} while( c == '\n' || c == '\r' );

	ungetc( c, fp );
	*pline = '\0';
	return line;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			word[0] = '\0';
			return word;
		}
		cEnd = getc( fp );
	} while( isspace( cEnd ) );

	if( cEnd == '\'' || cEnd == '"' )
	{
		pword = word;
	}
	else
	{
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ' ';
	}

	for( ; pword < word + MAX_INPUT_LENGTH; pword++ )
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
				exit( 1 );
			word[0] = '\0';
			return word;
		}
		*pword = getc( fp );
		if( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
		{
			if( cEnd == ' ' )
				ungetc( *pword, fp );
			*pword = '\0';
			return word;
		}
	}

	bug( "%s: word too long", __func__ );
	exit( 1 );
	return NULL;
}

CMDF( do_memory )
{
	char arg[MAX_INPUT_LENGTH];
	int hash;

	argument = one_argument( argument, arg );
	ch_printf( ch, "Affects %5d    Areas   %5d\r\n", top_affect, top_area );
	ch_printf( ch, "ExtDes  %5d    Exits   %5d\r\n", top_ed, top_exit );
	ch_printf( ch, "Helps   %5d    Resets  %5d\r\n", top_help, top_reset );
	ch_printf( ch, "IdxMobs %5d    Mobs    %5d\r\n", top_mob_index, nummobsloaded );
	ch_printf( ch, "IdxObjs %5d    Objs    %5d (%d)\r\n", top_obj_index, numobjsloaded, physicalobjects );
	ch_printf( ch, "Rooms   %5d    VRooms  %5d\r\n", top_room, top_vroom );
	ch_printf( ch, "Shops   %5d    RepShps %5d\r\n", top_shop, top_repair );
	ch_printf( ch, "CurOq's %5d    CurCq's %5d\r\n", cur_qobjs, cur_qchars );
	ch_printf( ch, "Players %5d    Maxplrs %5d\r\n", num_descriptors, sysdata.maxplayers );
	ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\r\n", sysdata.alltimemax, top_sn, MAX_SKILL );
	ch_printf( ch, "MaxEver time recorded at:   %s\r\n", sysdata.time_of_max );
	if( !str_cmp( arg, "check" ) )
	{
#ifdef HASHSTR
		send_to_char( check_hash( argument ), ch );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
		return;
	}
	if( !str_cmp( arg, "showhigh" ) )
	{
#ifdef HASHSTR
		show_high_hash( atoi( argument ) );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
		return;
	}
	if( argument[0] != '\0' )
		hash = atoi( argument );
	else
		hash = -1;
	if( !str_cmp( arg, "hash" ) )
	{
#ifdef HASHSTR
		ch_printf( ch, "Hash statistics:\r\n%s", hash_stats( ) );
		if( hash != -1 )
			hash_dump( hash );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
	}
	return;
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
	switch( number_bits( 2 ) )
	{
	case 0:
		number -= 1;
		break;
	case 3:
		number += 1;
		break;
	}

	return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
	/*    int power;
		int number;*/

	if( ( to = to - from + 1 ) <= 1 )
		return from;

	/*    for ( power = 2; power < to; power <<= 1 )
		;

		while ( ( number = number_mm( ) & (power - 1) ) >= to )
		;

		return from + number;*/
	return ( number_mm( ) % to ) + from;
}



/*
 * Generate a percentile roll.
 * number_mm() % 100 only does 0-99, changed to do 1-100 -Shaddai
 */
int number_percent( void )
{
	return ( number_mm( ) % 100 ) + 1;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
	int door;

	while( ( door = number_mm( ) & ( 16 - 1 ) ) > 9 )
		;

	return door;
	/*    return number_mm() & 10; */
}



int number_bits( int width )
{
	return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm( )
{
	int *piState;
	int iState;

	piState = &rgiState[2];

	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;

	piState[0] = ( ( int ) current_time ) & ( ( 1 << 30 ) - 1 );
	piState[1] = 1;
	for( iState = 2; iState < 55; iState++ )
	{
		piState[iState] = ( piState[iState - 1] + piState[iState - 2] ) & ( ( 1 << 30 ) - 1 );
	}
	return;
}

void sort_fname( FNAME_DATA *pName )
{
	FNAME_DATA *fname = NULL;

	if( !pName )
	{
		bug( "Sort_fname: NULL pName" );
		return;
	}

	pName->next = NULL;
	pName->prev = NULL;

	for( fname = first_fname; fname; fname = fname->next )
	{
		if( strcasecmp( pName->name, fname->name ) > 0 )
		{
			INSERT( pName, fname, first_fname, next, prev );
			break;
		}
	}

	if( !fname )
	{
		LINK( pName, first_fname, last_fname, next, prev );
	}
	return;
}

void load_fname( void )
{
	FNAME_DATA *fname;
	FILE *fp;

	first_fname = NULL;
	last_fname = NULL;

	if( !( fp = FileOpen( SYSTEM_DIR FNAME_LIST, "r" ) ) )
		return;

	for( ;; )
	{
		if( feof( fp ) )
		{
			bug( "Load_fname: no $ found." );
			FileClose( fp );
			return;
		}
		CREATE( fname, FNAME_DATA, 1 );
		fname->name = fread_string_nohash( fp );
		if( *fname->name == '$' )
			break;
		sort_fname( fname );
	}
	DISPOSE( fname->name );
	DISPOSE( fname );
	FileClose( fp );
	return;
}

void sort_mname( MNAME_DATA *pName )
{
	MNAME_DATA *mname = NULL;

	if( !pName )
	{
		bug( "Sort_mname: NULL pName" );
		return;
	}

	pName->next = NULL;
	pName->prev = NULL;

	for( mname = first_mname; mname; mname = mname->next )
	{
		if( strcasecmp( pName->name, mname->name ) > 0 )
		{
			INSERT( pName, mname, first_mname, next, prev );
			break;
		}
	}

	if( !mname )
	{
		LINK( pName, first_mname, last_mname, next, prev );
	}
	return;
}

void load_mname( void )
{
	MNAME_DATA *mname;
	FILE *fp;

	first_mname = NULL;
	last_mname = NULL;

	if( !( fp = FileOpen( SYSTEM_DIR MNAME_LIST, "r" ) ) )
		return;

	for( ;; )
	{
		if( feof( fp ) )
		{
			bug( "Load_mname: no $ found." );
			FileClose( fp );
			return;
		}
		CREATE( mname, MNAME_DATA, 1 );
		mname->name = fread_string_nohash( fp );
		if( *mname->name == '$' )
			break;
		sort_mname( mname );
	}
	DISPOSE( mname->name );
	DISPOSE( mname );
	FileClose( fp );
	return;
}

/* Rebuilt from broken copy, but bugged - commented out for now - Blod */
void sort_reserved( RESERVE_DATA *pRes )
{
	RESERVE_DATA *res = NULL;

	if( !pRes )
	{
		bug( "Sort_reserved: NULL pRes" );
		return;
	}

	pRes->next = NULL;
	pRes->prev = NULL;

	for( res = first_reserved; res; res = res->next )
	{
		if( strcasecmp( pRes->name, res->name ) > 0 )
		{
			INSERT( pRes, res, first_reserved, next, prev );
			break;
		}
	}

	if( !res )
	{
		LINK( pRes, first_reserved, last_reserved, next, prev );
	}
	return;
}

void load_reserved( void )
{
	RESERVE_DATA *res;
	FILE *fp;

	first_reserved = NULL;   /* Bug fix - Samson 8-1-99 */
	last_reserved = NULL;

	if( !( fp = FileOpen( SYSTEM_DIR RESERVED_LIST, "r" ) ) )
		return;

	for( ;; )
	{
		if( feof( fp ) )
		{
			bug( "Load_reserved: no $ found." );
			FileClose( fp );
			return;
		}
		CREATE( res, RESERVE_DATA, 1 );
		res->name = fread_string_nohash( fp );
		if( *res->name == '$' )
			break;
		sort_reserved( res );
	}
	DISPOSE( res->name );
	DISPOSE( res );
	FileClose( fp );
	return;
}


int number_mm( void )
{
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState = &rgiState[2];
	iState1 = piState[-2];
	iState2 = piState[-1];
	iRand = ( piState[iState1] + piState[iState2] ) & ( ( 1 << 30 ) - 1 );
	piState[iState1] = iRand;
	if( ++iState1 == 55 )
		iState1 = 0;
	if( ++iState2 == 55 )
		iState2 = 0;
	piState[-2] = iState1;
	piState[-1] = iState2;
	return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
	int idice;
	int sum;

	switch( size )
	{
	case 0:
		return 0;
	case 1:
		return number;
	}

	for( idice = 0, sum = 0; idice < number; idice++ )
		sum += number_range( 1, size );

	return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
	return value_00 + level * ( value_32 - value_00 ) / 32;
}

void smash_percent( char *str )
{
	for( ; *str != '\0'; str++ )
		if( *str == '%' )
			*str = '-';

	return;
}

const char *smash_percent_static( const char *str )
{
	static char buf[MAX_STRING_LENGTH];
	mudstrlcpy( buf, str, MAX_STRING_LENGTH );
	smash_percent( buf );
	return buf;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
	for( ; *str != '\0'; str++ )
		if( *str == '~' )
			*str = '-';
}

/*
 * From SmaugFUSS
 */

const char *smash_tilde_static( const char *str )
{
	static char buf[MAX_STRING_LENGTH];
	mudstrlcpy( buf, str, MAX_STRING_LENGTH );
	smash_tilde( buf );
	return buf;
}

char *smash_tilde_copy( const char *str )
{
	char *result = strdup( str );
	smash_tilde( result );
	return result;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
	for( ; *str != '\0'; str++ )
		if( *str == '~' )
			*str = HIDDEN_TILDE;
}

const char *show_tilde( const char *str )
{
	static char buf[MAX_STRING_LENGTH];
	char *bufptr = buf;

	for( ; *str != '\0'; str++, bufptr++ )
	{
		if( *str == HIDDEN_TILDE )
			*bufptr = '~';
		else
			*bufptr = *str;
	}

	*bufptr = '\0';

	return buf;
}

/*
 * Compare strings, case insensitive.
 * Return true if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
	if( !astr )
	{
		bug( "%s: null astr.", __func__ );
		if( bstr )
			fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
		return true;
	}

	if( !bstr )
	{
		bug( "%s: null bstr.", __func__ );
		if( astr )
			fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
		return true;
	}

	for( ; *astr || *bstr; astr++, bstr++ )
	{
		if( LOWER( *astr ) != LOWER( *bstr ) )
			return true;
	}

	return false;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
	if( !astr )
	{
		bug( "%s: null astr.", __func__ );
		return true;
	}

	if( !bstr )
	{
		bug( "%s: null bstr.", __func__ );
		return true;
	}

	for( ; *astr; astr++, bstr++ )
	{
		if( LOWER( *astr ) != LOWER( *bstr ) )
			return true;
	}

	return false;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns true is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	if( ( c0 = LOWER( astr[0] ) ) == '\0' )
		return false;

	sstr1 = strlen( astr );
	sstr2 = strlen( bstr );

	for( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
		if( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
			return false;

	return true;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return true if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;

	sstr1 = strlen( astr );
	sstr2 = strlen( bstr );
	if( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
		return false;
	else
		return true;
}

char *str_replace( const char *seed, const char *apple, char *tree );


FILE *__FileOpen( const char *filename, const char *mode, const char *file, const char *function, int line )
{
	FILE *fp = NULL;
	char fbuf[256];

	if( !filename || filename[0] == '\0' || !mode || mode[0] == '\0' )
	{
		log_string( "FileOpen called improperly." );
		return NULL;
	}
	//If writing...first create a temp file, otherwise just open the file -->KeB 10/29/08
	if( strstr( mode, "w" ) )
		snprintf( fbuf, 256, "%s.temporary", filename );
	else
		snprintf( fbuf, 256, "%s", filename );

	if( ( fp = fopen( fbuf, mode ) ) == NULL )
	{
		perror( fbuf );
		return NULL;
	}
	else
	{
		// *If you want to be really picky, define this* //
#ifdef DEBUG_FILEDATA
		File *file_data;
		for( file_data = file_list; file_data; file_data->next )
		{
			if( file_data->fp == fp )
			{
				log_string( "FileOpen: Double opening of a file!" );
			}
		}
#endif

		File *filedata;

		CREATE( filedata, File, 1 );
		filedata->filename = str_dup( fbuf );
		filedata->mode = str_dup( mode );
		filedata->file = str_dup( file );
		filedata->function = str_dup( function );
		filedata->line = line;
		filedata->fp = fp;
		LINK( filedata, first_filedata, last_filedata, next, prev );
		FilesOpen++;
	}

	return fp;

}

void free_filedata( File *filedata )
{
	UNLINK( filedata, first_filedata, last_filedata, next, prev );
	DISPOSE( filedata->filename );
	DISPOSE( filedata->file );
	DISPOSE( filedata->function );
	DISPOSE( filedata->mode );
	DISPOSE( filedata );
}

void free_all_filedata( void )
{
	File *filedata, *filedata_next;

	for( filedata = first_filedata; filedata; filedata = filedata_next )
	{
		filedata_next = filedata->next;
		free_filedata( filedata );
	}
	return;
}

// *Close the file-data* //
void FileClose( FILE *fp )
{
	char new_fname[MIL];
	char old_fname[MIL];
	new_fname[0] = '\0';

	if( fp )
	{
		File *filedata, *filedata_next;
		for( filedata = first_filedata; filedata; filedata = filedata_next )
		{
			filedata_next = filedata->next;
			if( filedata->fp == fp )
			{
				if( !str_suffix( ".temporary", filedata->filename ) )
				{
					snprintf( old_fname, MIL, "%s", filedata->filename );
					snprintf( new_fname, MIL, "%s", filedata->filename );
					str_replace( ".temporary", "", new_fname );
				}
				free_filedata( filedata );
				break;
			}
		}

		fclose( fp );
		FilesOpen--;

		if( FilesOpen < 0 )
		{
			FilesOpen = 0;
			log_string( "FileClose passed a null fp somewhere and schewed the list." );
		}
		if( new_fname[0] != '\0' )
		{
			if( rename( old_fname, new_fname ) )
			{
				log_printf( "FileClose: Problem with renaming %s to %s", old_fname, new_fname );
				return;
			}
		}

	}

}

// *ALL files should be closed.  I mean that, there is 100% no need to have an open file, unless your a sloppy coder* //
// *Remember that, what this does is link all the open file-data, and display it to you, this will allow you to find out * //
// *Where, and why your mud is crapping out.   I plugged this into a base, i forget which one, and after it compiled * //
// *I booted the mud up, and when i typed fileio, the damned thing had almost 30 open files because they didn't * //
// *Close the files properly, so i recommend this to people, because, if you have open files.  This will be a perfect* //
// * Way to find them, see exactly where they were opened, and why (mode) thus allowing you to find why they.* //
// *didn't close* //

CMDF( do_fileio )
{
	File *filedata;
	char buf[MSL];
	int count = 0;

	pager_printf( ch, "        &YFilename             &wMode      &WOpened                     &CFunction         &OLine\r\n" );
	send_to_pager( "&c------------------------------------------------------------------------------------------\r\n", ch );
	if( !first_filedata )
		send_to_pager( "\r\n&RCongrats, &Yyou have no &WOpen &Yfiles!\r\n", ch );

	for( filedata = first_filedata; filedata; filedata = filedata->next )
	{
		snprintf( buf, MAX_STRING_LENGTH, "&Y%-25.25s     &w%-1.1s       &W%-20.20s     &C%-15.15s     &O%-4.4d\r\n", filedata->filename,
			filedata->mode, filedata->file, filedata->function, filedata->line );
		count++;
		send_to_pager( buf, ch );
	}

	// *Add to the evil* //
	send_to_pager( "\r\n", ch );

	// *Make sure the count is right.* //
	if( FilesOpen != count )
	{
		send_to_pager( "&RThats Odd, the FilesOpen and count don't match!!!!", ch );
	}
	return;
}

/**********************************************************************************
* If you need the str_replace() function, use this one, I did not code it and
* I don't remember who did.
**********************************************************************************/
char *str_replace( const char *seed, const char *apple, char *tree )
{
	static char buffer[MSL * 2]; // working buffer
	char *nail;   // pointer to location in tree
	char *bpt; // pointer to location in buffer

	buffer[0] = '\0';   // initialize static buffer
	bpt = &buffer[0];   // point bpt to first char of buffer
	nail = &tree[0]; // point nail to first char of tree
	while( *nail != '\0' ) // while nail doesn't point to end
	{
		if( strncasecmp( nail, seed, strlen( seed ) ) )  // if seed isn't found
		{
			strncat( bpt, nail, 1 );   // append first char of nail to bpt
			bpt++;
			nail++;  // increment them to the next char
		}
		else   // if seed was found
		{
			strcat( buffer, apple );   // tag apple onto buffer
			bpt += strlen( apple ); // increment bpt by length of apple
			nail += strlen( seed ); // increment nail by length of seed
		}
	}
	*bpt++ = '\0';   // add null character to end of buffer
	strcpy( tree, buffer );   // copy buffer into original string
	return buffer;   // and return buffer
}

/*
 * Returns an initial-capped string.
 * Rewritten by FearItself@AvP
 */
char *capitalize( const char *str )
{
	static char buf[MAX_STRING_LENGTH];
	char *dest = buf;
	enum { Normal, Color } state = Normal;
	bool bFirst = true;
	char c;

	while( ( c = *str++ ) )
	{
		if( state == Normal )
		{
			if( c == '&' || c == '^' || c == '}' )
			{
				state = Color;
			}
			else if( isalpha( c ) )
			{
				c = bFirst ? toupper( c ) : tolower( c );
				bFirst = false;
			}
		}
		else
		{
			state = Normal;
		}
		*dest++ = c;
	}
	*dest = c;

	return buf;
}

/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
	static char strlow[MAX_STRING_LENGTH];
	int i;

	for( i = 0; str[i] != '\0'; i++ )
		strlow[i] = LOWER( str[i] );
	strlow[i] = '\0';
	return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
	static char strup[MAX_STRING_LENGTH];
	int i;

	for( i = 0; str[i] != '\0'; i++ )
		strup[i] = UPPER( str[i] );
	strup[i] = '\0';
	return strup;
}

/*
 * Returns true or false if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
	char c;

	c = tolower( letter );
	if( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
		return true;
	else
		return false;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
const char *aoran( const char *str )
{
	static char temp[MAX_STRING_LENGTH];

	if( !str )
	{
		bug( "%s: NULL str", __func__ );
		return "";
	}

	if( isavowel( str[0] ) || ( strlen( str ) > 1 && tolower( str[0] ) == 'y' && !isavowel( str[1] ) ) )
		mudstrlcpy( temp, "an ", MAX_STRING_LENGTH );
	else
		mudstrlcpy( temp, "a ", MAX_STRING_LENGTH );
	mudstrlcat( temp, str, MAX_STRING_LENGTH );
	return temp;
}

/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, const char *file, const char *str )
{
	FILE *fp;

	if( IS_NPC( ch ) || str[0] == '\0' )
		return;

	if( ( fp = FileOpen( file, "a" ) ) == NULL )
	{
		send_to_char( "Could not open the file!\r\n", ch );
	}
	else
	{
		fprintf( fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
		FileClose( fp );
	}
}

/*
 * Append a string to a file.
 */
void append_to_file( const char *file, const char *str )
{
	FILE *fp;

	if( ( fp = FileOpen( file, "a" ) ) == NULL )
	{
	}
	else
	{
		fprintf( fp, "%s\n", str );
		FileClose( fp );
	}
}

/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
	char buf[MAX_STRING_LENGTH];
	FILE *fp;
	struct stat fst;

	if( fpArea != NULL )
	{
		int iLine;
		int iChar;

		if( fpArea == stdin )
		{
			iLine = 0;
		}
		else
		{
			iChar = ftell( fpArea );
			fseek( fpArea, 0, 0 );
			for( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
			{
				int letter;

				while( ( letter = getc( fpArea ) ) && letter != EOF && letter != '\n' )
					;
			}
			fseek( fpArea, iChar, 0 );
		}

		snprintf( buf, MAX_STRING_LENGTH, "[*****] FILE: %s LINE: %d", strArea, iLine );
		log_string( buf );

		if( stat( SHUTDOWN_FILE, &fst ) != -1 )   /* file exists */
		{
			if( ( fp = FileOpen( SHUTDOWN_FILE, "a" ) ) != NULL )
			{
				fprintf( fp, "[*****] %s\n", buf );
				FileClose( fp );
			}
		}
	}

	mudstrlcpy( buf, "[*****] BUG: ", MAX_STRING_LENGTH );
	{
		va_list param;

		va_start( param, str );
		vsprintf( buf + strlen( buf ), str, param );
		va_end( param );
	}
	log_string( buf );
}

/*
* Add a string to the boot-up log				-Thoric
*/
void boot_log( const char *str, ... )
{
	char buf[MSL];
	FILE *fp;
	va_list param;

	mudstrlcpy( buf, "[*****] BOOT: ", MSL );
	va_start( param, str );
	vsnprintf( buf + strlen( buf ), ( MSL - strlen( buf ) ), str, param );
	va_end( param );
	log_string( buf );

	if( ( fp = FileOpen( BOOTLOG_FILE, "a" ) ) != NULL )
	{
		fprintf( fp, "%s\n", buf );
		FileClose( fp );
	}
	return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, const char *filename )
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	int c;
	int num = 0;

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{
		while( !feof( fp ) )
		{
			while( ( buf[num] = fgetc( fp ) ) != EOF
				&& buf[num] != '\n' && buf[num] != '\r' && num < ( MAX_STRING_LENGTH - 2 ) )
				num++;
			c = fgetc( fp );
			if( ( c != '\n' && c != '\r' ) || c == buf[num] )
				ungetc( c, fp );
			buf[num++] = '\n';
			buf[num++] = '\r';
			buf[num] = '\0';
			send_to_pager( buf, ch );
			num = 0;
		}
		FileClose( fp );
	}
}

/*
 * Show the boot log file					-Thoric
 */
CMDF( do_dmesg )
{
	set_pager_color( AT_LOG, ch );
	show_file( ch, BOOTLOG_FILE );
}

void log_bug_plus( const char *str, short log_type, short level )
{
	char *strtime;

	strtime = ctime( &current_time );
	strtime[strlen( strtime ) - 1] = '\0';
	fprintf( stderr, "%s :: %s\n", strtime, str );

	to_channel( str, CHANNEL_BUG, "Bug", level );
	return;
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, short log_type, short level )
{
	char *strtime;
	int offset;

	strtime = ctime( &current_time );
	strtime[strlen( strtime ) - 1] = '\0';
	fprintf( stderr, "%s :: %s\n", strtime, str );
	if( strncmp( str, "Log ", 4 ) == 0 )
		offset = 4;
	else
		offset = 0;
	switch( log_type )
	{
	default:
		to_channel( str + offset, CHANNEL_LOG, "Log", level );
		break;
	case LOG_BUILD:
		to_channel( str + offset, CHANNEL_BUILD, "Build", level );
		break;
	case LOG_COMM:
		to_channel( str + offset, CHANNEL_COMM, "Comm", level );
		break;
	case LOG_ALL:
		break;
	}
}

void log_printf_plus( short log_type, short level, const char *fmt, ... )
{
	char buf[MAX_STRING_LENGTH * 2];
	va_list args;

	va_start( args, fmt );
	vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
	va_end( args );

	log_string_plus( buf, log_type, level );
}

void log_printf( const char *fmt, ... )
{
	char buf[MAX_STRING_LENGTH * 2];
	va_list args;

	va_start( args, fmt );
	vsnprintf( buf, MAX_STRING_LENGTH * 2, fmt, args );
	va_end( args );

	log_string_plus( buf, LOG_NORMAL, LEVEL_LOG );
}

/*
 * wizlist builder!						-Thoric
 */

void towizfile( const char *line )
{
	int filler, xx;
	char outline[MAX_STRING_LENGTH];
	FILE *wfp;

	outline[0] = '\0';

	if( line && line[0] != '\0' )
	{
		filler = ( 78 - color_strlen( line ) );
		if( filler < 1 )
			filler = 1;
		filler /= 2;
		for( xx = 0; xx < filler; xx++ )
			mudstrlcat( outline, " ", MAX_STRING_LENGTH );
		mudstrlcat( outline, line, MAX_STRING_LENGTH );
	}
	mudstrlcat( outline, "\r\n", MAX_STRING_LENGTH );
	wfp = FileOpen( WIZLIST_FILE, "a" );
	if( wfp )
	{
		fputs( outline, wfp );
		FileClose( wfp );
	}
}

void towebwiz( const char *line )
{
	char outline[MAX_STRING_LENGTH];
	FILE *wfp;

	outline[0] = '\0';

	mudstrlcat( outline, " ", MSL );
	mudstrlcat( outline, line, MSL );

	//    strcat( outline, "\r\n" );
	wfp = FileOpen( WEBWIZ_FILE, "a" );
	if( wfp )
	{
		fputs( outline, wfp );
		FileClose( wfp );
	}
}

void add_to_wizlist( char *name, const char *aim, const char *job, int level )
{
	WIZENT *wiz, *tmp;

#ifdef DEBUG
	log_string( "Adding to wizlist..." );
#endif

	CREATE( wiz, WIZENT, 1 );
	wiz->name = str_dup( name );
	if( aim != NULL )
		wiz->aim = str_dup( aim );
	if( job != NULL )
		wiz->job = str_dup( job );
	wiz->level = level;
	if( !first_wiz )
	{
		wiz->last = NULL;
		wiz->next = NULL;
		first_wiz = wiz;
		last_wiz = wiz;
		return;
	}

	/*
	 * insert sort, of sorts
	 */
	for( tmp = first_wiz; tmp; tmp = tmp->next )
		if( level > tmp->level )
		{
			if( !tmp->last )
				first_wiz = wiz;
			else
				tmp->last->next = wiz;
			wiz->last = tmp->last;
			wiz->next = tmp;
			tmp->last = wiz;
			return;
		}

	wiz->last = last_wiz;
	wiz->next = NULL;
	last_wiz->next = wiz;
	last_wiz = wiz;
	return;
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist( )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *gfp;
	const char *word, *aim, *job;
	int ilevel, iflags;
	WIZENT *wiz, *wiznext;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int count = 0;

	first_wiz = NULL;
	last_wiz = NULL;

	dp = opendir( GOD_DIR );

	ilevel = 0;
	dentry = readdir( dp );
	while( dentry )
	{
		if( dentry->d_name[0] != '.' )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, dentry->d_name );
			gfp = FileOpen( buf, "r" );
			if( gfp )
			{
				word = feof( gfp ) ? "End" : fread_word( gfp );
				ilevel = fread_number( gfp );
				fread_to_eol( gfp );
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Pcflags" ) )
					iflags = fread_number( gfp );
				else
					iflags = 0;
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Aim" ) )
					aim = fread_string( gfp );
				else
					aim = NULL;
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Job" ) )
					job = fread_string( gfp );
				else
					job = NULL;
				FileClose( gfp );
				if( IS_SET( iflags, PCFLAG_RETIRED ) )
					ilevel = MAX_LEVEL - 6;
				if( IS_SET( iflags, PCFLAG_GUEST ) )
					ilevel = MAX_LEVEL - 6;
				add_to_wizlist( dentry->d_name, aim, job, ilevel );
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );

	buf[0] = '\0';
	unlink( WIZLIST_FILE );
	towizfile( "&B  /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B" );
	towizfile( "&B //&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B" );
	towizfile( "&B \\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B" );
	towizfile( "&B  \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B" );
	towizfile( "&Y    -----------------------------------------------------------" );

	ilevel = MAX_LEVEL + 1;
	for( wiz = first_wiz; wiz; wiz = wiz->next )
	{
		if( wiz->level > LEVEL_AVATAR )
		{
			if( wiz->level < ilevel )
			{
				if( buf[0] )
				{
					towizfile( buf );
					buf[0] = '\0';
				}
			}
			count++;

			mudstrlcat( buf, " ", MSL );
			if( count % 2 == 1 )
			{
				snprintf( buf2, MAX_STRING_LENGTH, "&Y   |&G%d&Y| &C%-10s &Y- %-23s &Y- &P%-17s &Y|", wiz->level, wiz->name, wiz->job,
					wiz->aim );
			}
			else
			{
				snprintf( buf2, MAX_STRING_LENGTH, "&Y   |&g%d&Y| &c%-10s &O- %-23s &O- &p%-17s &Y|", wiz->level, wiz->name, wiz->job,
					wiz->aim );
			}
			mudstrlcat( buf, buf2, MSL );
			if( color_strlen( buf ) > 100 )
			{
				towizfile( buf );
				buf[0] = '\0';
			}
		}
	}

	if( buf[0] )
		towizfile( buf );

	for( wiz = first_wiz; wiz; wiz = wiznext )
	{
		wiznext = wiz->next;
		DISPOSE( wiz->name );
		DISPOSE( wiz->aim );
		DISPOSE( wiz->job );
		DISPOSE( wiz );
	}
	first_wiz = NULL;
	last_wiz = NULL;
	towizfile( "&Y    -----------------------------------------------------------" );
	towizfile( "&B  /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B /&b\\ &B" );
	towizfile( "&B //&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B//&b\\\\&B" );
	towizfile( "&B \\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B\\\\&b//&B" );
	towizfile( "&B  \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B \\&b/ &B" );
}

void make_webwiz( void )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *gfp;
	const char *word, *aim, *job;
	int ilevel, iflags;
	WIZENT *wiz, *wiznext;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int count = 0;

	first_wiz = NULL;
	last_wiz = NULL;

	dp = opendir( GOD_DIR );

	ilevel = 0;
	dentry = readdir( dp );
	while( dentry )
	{
		if( dentry->d_name[0] != '.' )
		{
			snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, dentry->d_name );
			gfp = FileOpen( buf, "r" );
			if( gfp )
			{
				word = feof( gfp ) ? "End" : fread_word( gfp );
				ilevel = fread_number( gfp );
				fread_to_eol( gfp );
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Pcflags" ) )
					iflags = fread_number( gfp );
				else
					iflags = 0;
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Aim" ) )
					aim = fread_string( gfp );
				else
					aim = NULL;
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if( !str_cmp( word, "Job" ) )
					job = fread_string( gfp );
				else
					job = NULL;
				FileClose( gfp );
				if( IS_SET( iflags, PCFLAG_RETIRED ) )
					ilevel = MAX_LEVEL - 15;
				if( IS_SET( iflags, PCFLAG_GUEST ) )
					ilevel = MAX_LEVEL - 16;
				add_to_wizlist( dentry->d_name, aim, job, ilevel );
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );

	unlink( WEBWIZ_FILE );

	towebwiz( "<html>\n<head>\n<title>\n// Gundam Wing: Operation Meteor - Wizlist \\\\\n</title>\n" );
	towebwiz( "<BODY BGCOLOR=#000000>" );
	towebwiz( "<table border=0 cellpadding=0 cellspacing=0 align=center>" );
	towebwiz( "<font face=" "Courier New" "> <font size=3>" );
	towebwiz( "<tr><td colspan=5><font color=" "#FFFF00"
		">--------------------------------------------------------------------</td></tr>" );

	buf[0] = '\0';
	ilevel = 65535;
	for( wiz = first_wiz; wiz; wiz = wiz->next )
	{
		if( wiz->level < LEVEL_AVATAR )
		{
			if( buf[0] )
			{
				towebwiz( buf );
				buf[0] = '\0';
			}
			towebwiz( "" );
			ilevel = wiz->level;

			towebwiz( "</font></p>" );

			switch( ilevel )
			{
			case MAX_LEVEL - 0:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 1:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 2:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 3:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 4:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 5:
				towebwiz( "" );
				break;
			case MAX_LEVEL - 6:
				towebwiz( "" );
				break;
			default:
				towebwiz( "" );
				break;
			}

		}

		if( strlen( buf ) + strlen( wiz->name ) > 999 )
		{
			towebwiz( buf );
			buf[0] = '\0';
		}
		count++;

		strcat( buf, " " );

		if( count % 2 == 1 )
		{

			snprintf( buf2, MAX_STRING_LENGTH,
				"<tr>   <td width=35><font color=" "#FFFF00" " face=" "Courier New" ">|<font color=" "#00FF00"
				">%d<font color=" "#FFFF00" ">|</td> <td width=70><font color=" "#OOFFFF"
				">%-10s</td> <td width=150><font color=" "#FFFF00" ">- <font color=" "#FF0000"
				">%-17s</td> <td width=130><font color=" "#FFFF00" ">- <font color=" "#FF00FF"
				">%-17s</td> <td><font color=" "#FFFF00" ">|</td></tr>\r\n", wiz->level, wiz->name, wiz->job, wiz->aim );
		}
		else
		{
			snprintf( buf2, MAX_STRING_LENGTH,
				"<tr>   <td width=35><font color=" "#FFFF00" " face=" "Courier New" ">|<font color=" "006400"
				">%d<font color=" "#FFFF00" ">|</td> <td width=70><font color=" "#008B8B"
				">%-10s</td> <td width=120><font color=" "#808000" ">- <font color=" "#8B0000"
				">%-17s</td> <td width=130><font color=" "#808000" ">- <font color=" "#8B008B"
				">%-17s</td> <td><font color=" "#FFFF00" ">|</td></tr>\r\n", wiz->level, wiz->name, wiz->job, wiz->aim );
		}
		strcat( buf, buf2 );

		if( strlen( buf ) > 999 )
		{
			towebwiz( buf );
			buf[0] = '\0';
		}
	}

	towebwiz( "<tr><td colspan=5><font color=" "#FFFF00"
		">--------------------------------------------------------------------</td></tr>" );
	//  towebwiz( "<tr>" );
	if( buf[0] )
		towebwiz( buf );

	//  towebwiz( "</td>" );

	for( wiz = first_wiz; wiz; wiz = wiznext )
	{
		wiznext = wiz->next;
		DISPOSE( wiz->name );
		DISPOSE( wiz->aim );
		DISPOSE( wiz->job );
		DISPOSE( wiz );
	}
	first_wiz = NULL;
	last_wiz = NULL;
}

CMDF( do_makewizlist )
{
	make_wizlist( );

	{
		FILE *fp = FileOpen( WEBWIZ_FILE, "w" );
		if( fp )
			FileClose( fp );
	}

	make_webwiz( );
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type( const char *name )
{
	if( !str_cmp( name, "in_file_prog" ) )
		return IN_FILE_PROG;
	if( !str_cmp( name, "act_prog" ) )
		return ACT_PROG;
	if( !str_cmp( name, "speech_prog" ) )
		return SPEECH_PROG;
	if( !str_cmp( name, "rand_prog" ) )
		return RAND_PROG;
	if( !str_cmp( name, "fight_prog" ) )
		return FIGHT_PROG;
	if( !str_cmp( name, "hitprcnt_prog" ) )
		return HITPRCNT_PROG;
	if( !str_cmp( name, "death_prog" ) )
		return DEATH_PROG;
	if( !str_cmp( name, "entry_prog" ) )
		return ENTRY_PROG;
	if( !str_cmp( name, "greet_prog" ) )
		return GREET_PROG;
	if( !str_cmp( name, "all_greet_prog" ) )
		return ALL_GREET_PROG;
	if( !str_cmp( name, "give_prog" ) )
		return GIVE_PROG;
	if( !str_cmp( name, "bribe_prog" ) )
		return BRIBE_PROG;
	if( !str_cmp( name, "time_prog" ) )
		return TIME_PROG;
	if( !str_cmp( name, "hour_prog" ) )
		return HOUR_PROG;
	if( !str_cmp( name, "wear_prog" ) )
		return WEAR_PROG;
	if( !str_cmp( name, "remove_prog" ) )
		return REMOVE_PROG;
	if( !str_cmp( name, "sac_prog" ) )
		return SAC_PROG;
	if( !str_cmp( name, "look_prog" ) )
		return LOOK_PROG;
	if( !str_cmp( name, "exa_prog" ) )
		return EXA_PROG;
	if( !str_cmp( name, "zap_prog" ) )
		return ZAP_PROG;
	if( !str_cmp( name, "get_prog" ) )
		return GET_PROG;
	if( !str_cmp( name, "drop_prog" ) )
		return DROP_PROG;
	if( !str_cmp( name, "damage_prog" ) )
		return DAMAGE_PROG;
	if( !str_cmp( name, "repair_prog" ) )
		return REPAIR_PROG;
	if( !str_cmp( name, "greet_prog" ) )
		return GREET_PROG;
	if( !str_cmp( name, "randiw_prog" ) )
		return RANDIW_PROG;
	if( !str_cmp( name, "speechiw_prog" ) )
		return SPEECHIW_PROG;
	if( !str_cmp( name, "pull_prog" ) )
		return PULL_PROG;
	if( !str_cmp( name, "push_prog" ) )
		return PUSH_PROG;
	if( !str_cmp( name, "sleep_prog" ) )
		return SLEEP_PROG;
	if( !str_cmp( name, "rest_prog" ) )
		return REST_PROG;
	if( !str_cmp( name, "rfight_prog" ) )
		return FIGHT_PROG;
	if( !str_cmp( name, "enter_prog" ) )
		return ENTRY_PROG;
	if( !str_cmp( name, "leave_prog" ) )
		return LEAVE_PROG;
	if( !str_cmp( name, "rdeath_prog" ) )
		return DEATH_PROG;
	if( !str_cmp( name, "script_prog" ) )
		return SCRIPT_PROG;
	if( !str_cmp( name, "use_prog" ) )
		return USE_PROG;
	return ( ERROR_PROG );
}

void mobprog_file_read( MOB_INDEX_DATA *mob, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter == '|' )
			break;

		if( letter != '>' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		CREATE( mprg, MPROG_DATA, 1 );
		mprg->type = mprog_name_to_type( fread_word( progfile ) );
		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: mudprog file type error", __func__ );
			DISPOSE( mprg );
			continue;

		case IN_FILE_PROG:
			bug( "%s: Nested file programs are not allowed.", __func__ );
			DISPOSE( mprg );
			continue;

		default:
			mprg->arglist = fread_string( progfile );
			mprg->comlist = fread_string( progfile );
			mprg->fileprog = true;
			xSET_BIT( mob->progtypes, mprg->type );
			mprg->next = mob->mudprogs;
			mob->mudprogs = mprg;
			break;
		}
	}
	FileClose( progfile );
}

/* This procedure is responsible for reading any in_file MUDprograms.
 */
void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *mob )
{
	MPROG_DATA *mprg;
	char letter;
	const char *word;

	for( ;; )
	{
		letter = fread_letter( fp );

		if( letter == '|' )
			return;

		if( letter != '>' )
		{
			bug( "%s: vnum %d MUDPROG char", __func__, mob->vnum );
			exit( 1 );
		}
		CREATE( mprg, MPROG_DATA, 1 );
		mprg->next = mob->mudprogs;
		mob->mudprogs = mprg;

		word = fread_word( fp );
		mprg->type = mprog_name_to_type( word );

		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: vnum %d MUDPROG type.", __func__, mob->vnum );
			exit( 1 );

		case IN_FILE_PROG:
			mprg->arglist = fread_string( fp );
			mprg->fileprog = false;
			mobprog_file_read( mob, mprg->arglist );
			break;

		default:
			xSET_BIT( mob->progtypes, mprg->type );
			mprg->fileprog = false;
			mprg->arglist = fread_string( fp );
			mprg->comlist = fread_string( fp );
			break;
		}
	}
}

/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

 /* This routine reads in scripts of OBJprograms from a file */
void objprog_file_read( OBJ_INDEX_DATA *obj, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter == '|' )
			break;

		if( letter != '>' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		CREATE( mprg, MPROG_DATA, 1 );
		mprg->type = mprog_name_to_type( fread_word( progfile ) );
		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: mudprog file type error", __func__ );
			DISPOSE( mprg );
			continue;

		case IN_FILE_PROG:
			bug( "%s: Nested file programs are not allowed.", __func__ );
			DISPOSE( mprg );
			continue;

		default:
			mprg->arglist = fread_string( progfile );
			mprg->comlist = fread_string( progfile );
			mprg->fileprog = true;
			xSET_BIT( obj->progtypes, mprg->type );
			mprg->next = obj->mudprogs;
			obj->mudprogs = mprg;
			break;
		}
	}
	FileClose( progfile );
}

/* This procedure is responsible for reading any in_file OBJprograms.
 */
void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *obj )
{
	MPROG_DATA *mprg;
	char letter;
	const char *word;

	for( ;; )
	{
		letter = fread_letter( fp );

		if( letter == '|' )
			return;

		if( letter != '>' )
		{
			bug( "%s: vnum %d MUDPROG char", __func__, obj->vnum );
			exit( 1 );
		}
		CREATE( mprg, MPROG_DATA, 1 );
		mprg->next = obj->mudprogs;
		obj->mudprogs = mprg;

		word = fread_word( fp );
		mprg->type = mprog_name_to_type( word );

		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: vnum %d MUDPROG type.", __func__, obj->vnum );
			exit( 1 );

		case IN_FILE_PROG:
			mprg->arglist = fread_string( fp );
			mprg->fileprog = false;
			objprog_file_read( obj, mprg->arglist );
			break;

		default:
			xSET_BIT( obj->progtypes, mprg->type );
			mprg->fileprog = false;
			mprg->arglist = fread_string( fp );
			mprg->comlist = fread_string( fp );
			break;
		}
	}
}

/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

 /* This routine reads in scripts of OBJprograms from a file */
void roomprog_file_read( ROOM_INDEX_DATA *room, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter == '|' )
			break;

		if( letter != '>' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		CREATE( mprg, MPROG_DATA, 1 );
		mprg->type = mprog_name_to_type( fread_word( progfile ) );
		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: mudprog file type error", __func__ );
			DISPOSE( mprg );
			continue;

		case IN_FILE_PROG:
			bug( "%s: Nested file programs are not allowed.", __func__ );
			DISPOSE( mprg );
			continue;

		default:
			mprg->arglist = fread_string( progfile );
			mprg->comlist = fread_string( progfile );
			mprg->fileprog = true;
			xSET_BIT( room->progtypes, mprg->type );
			mprg->next = room->mudprogs;
			room->mudprogs = mprg;
			break;
		}
	}
	FileClose( progfile );
}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */
void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *room )
{
	MPROG_DATA *mprg;
	char letter;
	const char *word;

	for( ;; )
	{
		letter = fread_letter( fp );

		if( letter == '|' )
			return;

		if( letter != '>' )
		{
			bug( "%s: vnum %d MUDPROG char", __func__, room->vnum );
			exit( 1 );
		}
		CREATE( mprg, MPROG_DATA, 1 );
		mprg->next = room->mudprogs;
		room->mudprogs = mprg;

		word = fread_word( fp );
		mprg->type = mprog_name_to_type( word );

		switch( mprg->type )
		{
		case ERROR_PROG:
			bug( "%s: vnum %d MUDPROG type.", __func__, room->vnum );
			exit( 1 );

		case IN_FILE_PROG:
			mprg->arglist = fread_string( fp );
			mprg->fileprog = false;
			roomprog_file_read( room, mprg->arglist );
			break;

		default:
			xSET_BIT( room->progtypes, mprg->type );
			mprg->fileprog = false;
			mprg->arglist = fread_string( fp );
			mprg->comlist = fread_string( fp );
			break;
		}
	}
}

void delete_room( ROOM_INDEX_DATA *room )
{
	int hash;
	ROOM_INDEX_DATA *prev, *limbo = get_room_index( ROOM_VNUM_LIMBO );
	OBJ_DATA *o;
	CHAR_DATA *ch;
	EXTRA_DESCR_DATA *ed;
	EXIT_DATA *ex;
	MPROG_ACT_LIST *mpact;
	MPROG_DATA *mp;

	UNLINK( room, room->area->first_room, room->area->last_room, next_aroom, prev_aroom );

	while( ( ch = room->first_person ) != NULL )
	{
		if( !IS_NPC( ch ) )
		{
			char_from_room( ch );
			char_to_room( ch, limbo );
		}
		else
			extract_char( ch, true );
	}

	for( ch = first_char; ch; ch = ch->next )
	{
		if( ch->was_in_room == room )
			ch->was_in_room = ch->in_room;
		if( ch->substate == SUB_ROOM_DESC && ch->dest_buf == room )
		{
			send_to_char( "The room is no more.\r\n", ch );
			stop_editing( ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
		}
		else if( ch->substate == SUB_ROOM_EXTRA && ch->dest_buf )
		{
			for( ed = room->first_extradesc; ed; ed = ed->next )
			{
				if( ed == ch->dest_buf )
				{
					send_to_char( "The room is no more.\r\n", ch );
					stop_editing( ch );
					ch->substate = SUB_NONE;
					ch->dest_buf = NULL;
					break;
				}
			}
		}
	}

	while( ( o = room->first_content ) != NULL )
		extract_obj( o );
	wipe_resets( room );
	while( ( ed = room->first_extradesc ) != NULL )
	{
		room->first_extradesc = ed->next;
		STRFREE( ed->keyword );
		STRFREE( ed->description );
		DISPOSE( ed );
		--top_ed;
	}
	while( ( ex = room->first_exit ) != NULL )
		extract_exit( room, ex );
	while( ( mpact = room->mpact ) != NULL )
	{
		room->mpact = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact );
	}
	while( ( mp = room->mudprogs ) != NULL )
	{
		room->mudprogs = mp->next;
		STRFREE( mp->arglist );
		STRFREE( mp->comlist );
		DISPOSE( mp );
	}
	STRFREE( room->name );
	STRFREE( room->description );
	STRFREE( room->owner );
	STRFREE( room->guests );

	hash = room->vnum % MAX_KEY_HASH;
	if( room == room_index_hash[hash] )
		room_index_hash[hash] = room->next;
	else
	{
		for( prev = room_index_hash[hash]; prev; prev = prev->next )
			if( prev->next == room )
				break;
		if( prev )
			prev->next = room->next;
		else
			bug( "delete_room: room %d not in hash bucket %d.", room->vnum, hash );
	}
	DISPOSE( room );
	--top_room;
	return;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
	int hash;
	OBJ_INDEX_DATA *prev;
	OBJ_DATA *o, *o_next;
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *af;
	MPROG_DATA *mp;
	CHAR_DATA *ch;

	/*
	 * Remove references to object index
	 */
	for( o = first_object; o; o = o_next )
	{
		o_next = o->next;
		if( o->pIndexData == obj )
			extract_obj( o );
	}

	for( ch = first_char; ch; ch = ch->next )
	{
		if( ch->substate == SUB_OBJ_EXTRA && ch->dest_buf )
		{
			for( ed = obj->first_extradesc; ed; ed = ed->next )
			{
				if( ed == ch->dest_buf )
				{
					send_to_char( "You suddenly forget which object you were editing!\r\n", ch );
					stop_editing( ch );
					ch->substate = SUB_NONE;
					break;
				}
			}
		}
		else if( ch->substate == SUB_MPROG_EDIT && ch->dest_buf )
		{
			for( mp = obj->mudprogs; mp; mp = mp->next )
			{
				if( mp == ch->dest_buf )
				{
					send_to_char( "You suddenly forget which object you were working on.\r\n", ch );
					stop_editing( ch );
					ch->dest_buf = NULL;
					ch->substate = SUB_NONE;
					break;
				}
			}
		}
	}

	while( ( ed = obj->first_extradesc ) != NULL )
	{
		obj->first_extradesc = ed->next;
		STRFREE( ed->keyword );
		STRFREE( ed->description );
		DISPOSE( ed );
		--top_ed;
	}
	while( ( af = obj->first_affect ) != NULL )
	{
		obj->first_affect = af->next;
		DISPOSE( af );
		--top_affect;
	}
	while( ( mp = obj->mudprogs ) != NULL )
	{
		obj->mudprogs = mp->next;
		STRFREE( mp->arglist );
		STRFREE( mp->comlist );
		DISPOSE( mp );
	}
	STRFREE( obj->name );
	STRFREE( obj->short_descr );
	STRFREE( obj->description );
	STRFREE( obj->action_desc );

	hash = obj->vnum % MAX_KEY_HASH;
	if( obj == obj_index_hash[hash] )
		obj_index_hash[hash] = obj->next;
	else
	{
		for( prev = obj_index_hash[hash]; prev; prev = prev->next )
			if( prev->next == obj )
				break;
		if( prev )
			prev->next = obj->next;
		else
			bug( "delete_obj: object %d not in hash bucket %d.", obj->vnum, hash );
	}
	DISPOSE( obj );
	--top_obj_index;
	return true;

}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
	int hash;
	MOB_INDEX_DATA *prev;
	CHAR_DATA *ch, *ch_next;
	MPROG_DATA *mp;

	for( ch = first_char; ch; ch = ch_next )
	{
		ch_next = ch->next;
		if( ch->pIndexData == mob )
			extract_char( ch, true );
	}

	for( ch = first_char; ch; ch = ch_next )
	{
		ch_next = ch->next;

		if( ch->pIndexData == mob )
			extract_char( ch, true );
		else if( ch->substate == SUB_MPROG_EDIT && ch->dest_buf )
		{
			for( mp = mob->mudprogs; mp; mp = mp->next )
			{
				if( mp == ch->dest_buf )
				{
					send_to_char( "Your victim has departed.\r\n", ch );
					stop_editing( ch );
					ch->dest_buf = NULL;
					ch->substate = SUB_NONE;
					break;
				}
			}
		}
	}

	while( ( mp = mob->mudprogs ) != NULL )
	{
		mob->mudprogs = mp->next;
		STRFREE( mp->arglist );
		STRFREE( mp->comlist );
		DISPOSE( mp );
	}

	if( mob->pShop )
	{
		UNLINK( mob->pShop, first_shop, last_shop, next, prev );
		DISPOSE( mob->pShop );
		--top_shop;
	}

	if( mob->rShop )
	{
		UNLINK( mob->rShop, first_repair, last_repair, next, prev );
		DISPOSE( mob->rShop );
		--top_repair;
	}

	STRFREE( mob->player_name );
	STRFREE( mob->short_descr );
	STRFREE( mob->long_descr );
	STRFREE( mob->description );

	hash = mob->vnum % MAX_KEY_HASH;
	if( mob == mob_index_hash[hash] )
		mob_index_hash[hash] = mob->next;
	else
	{
		for( prev = mob_index_hash[hash]; prev; prev = prev->next )
			if( prev->next == mob )
				break;
		if( prev )
			prev->next = mob->next;
		else
			bug( "delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash );
	}
	DISPOSE( mob );
	--top_mob_index;
	return true;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum, AREA_DATA *area )
{
	ROOM_INDEX_DATA *pRoomIndex;
	int iHash;

	CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	pRoomIndex->first_person = NULL;
	pRoomIndex->last_person = NULL;
	pRoomIndex->first_content = NULL;
	pRoomIndex->last_content = NULL;
	pRoomIndex->first_reset = pRoomIndex->last_reset = NULL;
	pRoomIndex->first_extradesc = NULL;
	pRoomIndex->last_extradesc = NULL;
	pRoomIndex->first_ship = NULL;
	pRoomIndex->last_ship = NULL;
	pRoomIndex->area = area;
	pRoomIndex->vnum = vnum;
	pRoomIndex->name = STRALLOC( "Floating in a void" );
	pRoomIndex->description = STRALLOC( "" );
	xCLEAR_BITS( pRoomIndex->room_flags );
	xSET_BIT( pRoomIndex->room_flags, ROOM_PROTOTYPE );
	pRoomIndex->sector_type = 1;
	pRoomIndex->light = 0;
	pRoomIndex->first_exit = NULL;
	pRoomIndex->last_exit = NULL;
	pRoomIndex->guests = STRALLOC( "" );

	iHash = vnum % MAX_KEY_HASH;
	pRoomIndex->next = room_index_hash[iHash];
	room_index_hash[iHash] = pRoomIndex;
	top_room++;

	return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, const char *name )
{
	OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
	char buf[MAX_STRING_LENGTH];
	int iHash;

	if( cvnum > 0 )
		cObjIndex = get_obj_index( cvnum );
	else
		cObjIndex = NULL;
	CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	pObjIndex->vnum = vnum;
	pObjIndex->name = STRALLOC( name );
	pObjIndex->first_affect = NULL;
	pObjIndex->last_affect = NULL;
	pObjIndex->first_extradesc = NULL;
	pObjIndex->last_extradesc = NULL;
	if( !cObjIndex )
	{
		snprintf( buf, MAX_STRING_LENGTH, "A %s", name );
		pObjIndex->short_descr = STRALLOC( buf );
		snprintf( buf, MAX_STRING_LENGTH, "A %s is here.", name );
		pObjIndex->description = STRALLOC( buf );
		pObjIndex->action_desc = STRALLOC( "" );
		// it's safe to cast these because we just created the object
		( ( char * ) pObjIndex->short_descr )[0] = LOWER( pObjIndex->short_descr[0] );
		( ( char * ) pObjIndex->description )[0] = UPPER( pObjIndex->description[0] );
		pObjIndex->item_type = ITEM_TRASH;
		xCLEAR_BITS( pObjIndex->extra_flags );
		xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
		pObjIndex->wear_flags = 0;
		pObjIndex->value[0] = 0;
		pObjIndex->value[1] = 0;
		pObjIndex->value[2] = 0;
		pObjIndex->value[3] = 0;
		pObjIndex->value[4] = 0;
		pObjIndex->value[5] = 0;
		pObjIndex->weight = 1;
		pObjIndex->cost = 0;
	}
	else
	{
		EXTRA_DESCR_DATA *ed, *ced;
		AFFECT_DATA *paf, *cpaf;

		pObjIndex->short_descr = QUICKLINK( cObjIndex->short_descr );
		pObjIndex->description = QUICKLINK( cObjIndex->description );
		pObjIndex->action_desc = QUICKLINK( cObjIndex->action_desc );
		pObjIndex->item_type = cObjIndex->item_type;
		pObjIndex->extra_flags = cObjIndex->extra_flags;
		xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
		pObjIndex->wear_flags = cObjIndex->wear_flags;
		pObjIndex->value[0] = cObjIndex->value[0];
		pObjIndex->value[1] = cObjIndex->value[1];
		pObjIndex->value[2] = cObjIndex->value[2];
		pObjIndex->value[3] = cObjIndex->value[3];
		pObjIndex->value[4] = cObjIndex->value[4];
		pObjIndex->value[5] = cObjIndex->value[5];
		pObjIndex->weight = cObjIndex->weight;
		pObjIndex->cost = cObjIndex->cost;
		for( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
		{
			CREATE( ed, EXTRA_DESCR_DATA, 1 );
			ed->keyword = QUICKLINK( ced->keyword );
			ed->description = QUICKLINK( ced->description );
			LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
			top_ed++;
		}
		for( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
		{
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type = cpaf->type;
			paf->duration = cpaf->duration;
			paf->location = cpaf->location;
			paf->modifier = cpaf->modifier;
			paf->bitvector = cpaf->bitvector;
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
			top_affect++;
		}
	}
	pObjIndex->count = 0;
	iHash = vnum % MAX_KEY_HASH;
	pObjIndex->next = obj_index_hash[iHash];
	obj_index_hash[iHash] = pObjIndex;
	top_obj_index++;

	return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, short cvnum, const char *name )
{
	MOB_INDEX_DATA *pMobIndex, *cMobIndex;
	char buf[MAX_STRING_LENGTH];
	int iHash;

	if( cvnum > 0 )
		cMobIndex = get_mob_index( cvnum );
	else
		cMobIndex = NULL;
	CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	pMobIndex->vnum = vnum;
	pMobIndex->count = 0;
	pMobIndex->killed = 0;
	pMobIndex->player_name = STRALLOC( name );
	if( !cMobIndex )
	{
		snprintf( buf, MAX_STRING_LENGTH, "A newly created %s", name );
		pMobIndex->short_descr = STRALLOC( buf );
		snprintf( buf, MAX_STRING_LENGTH, "Some god abandoned a newly created %s here.\r\n", name );
		pMobIndex->long_descr = STRALLOC( buf );
		pMobIndex->description = STRALLOC( "" );
		// it's safe to cast these because we just created the object
		( ( char * ) pMobIndex->short_descr )[0] = LOWER( pMobIndex->short_descr[0] );
		( ( char * ) pMobIndex->long_descr )[0] = UPPER( pMobIndex->long_descr[0] );
		( ( char * ) pMobIndex->description )[0] = UPPER( pMobIndex->description[0] );
		xCLEAR_BITS( pMobIndex->act );
		xSET_BIT( pMobIndex->act, ACT_IS_NPC );
		xSET_BIT( pMobIndex->act, ACT_PROTOTYPE );
		pMobIndex->affected_by = 0;
		pMobIndex->pShop = NULL;
		pMobIndex->rShop = NULL;
		pMobIndex->spec_fun = NULL;
		pMobIndex->spec_2 = NULL;
		pMobIndex->mudprogs = NULL;
		xCLEAR_BITS( pMobIndex->progtypes );
		pMobIndex->alignment = 0;
		pMobIndex->level = 1;
		pMobIndex->mobthac0 = 0;
		pMobIndex->ac = 0;
		pMobIndex->hitnodice = 0;
		pMobIndex->hitsizedice = 0;
		pMobIndex->hitplus = 0;
		pMobIndex->damnodice = 0;
		pMobIndex->damsizedice = 0;
		pMobIndex->damplus = 0;
		pMobIndex->gold = 0;
		pMobIndex->exp = 0;
		pMobIndex->position = 8;
		pMobIndex->defposition = 8;
		pMobIndex->sex = 0;
		pMobIndex->perm_str = 10;
		pMobIndex->perm_dex = 10;
		pMobIndex->perm_int = 10;
		pMobIndex->perm_wis = 10;
		pMobIndex->perm_cha = 10;
		pMobIndex->perm_con = 10;
		pMobIndex->perm_lck = 10;
		pMobIndex->race = 0;
		pMobIndex->xflags = 0;
		pMobIndex->resistant = 0;
		pMobIndex->immune = 0;
		pMobIndex->susceptible = 0;
		pMobIndex->numattacks = 1;
		pMobIndex->attacks = 0;
		pMobIndex->defenses = 0;
	}
	else
	{
		pMobIndex->short_descr = QUICKLINK( cMobIndex->short_descr );
		pMobIndex->long_descr = QUICKLINK( cMobIndex->long_descr );
		pMobIndex->description = QUICKLINK( cMobIndex->description );
		pMobIndex->act = cMobIndex->act;
		xSET_BIT( pMobIndex->act, ACT_PROTOTYPE );
		pMobIndex->affected_by = cMobIndex->affected_by;
		pMobIndex->pShop = NULL;
		pMobIndex->rShop = NULL;
		pMobIndex->spec_fun = cMobIndex->spec_fun;
		pMobIndex->spec_2 = cMobIndex->spec_2;
		pMobIndex->mudprogs = NULL;
		xCLEAR_BITS( pMobIndex->progtypes );
		pMobIndex->alignment = cMobIndex->alignment;
		pMobIndex->level = cMobIndex->level;
		pMobIndex->mobthac0 = cMobIndex->mobthac0;
		pMobIndex->ac = cMobIndex->ac;
		pMobIndex->hitnodice = cMobIndex->hitnodice;
		pMobIndex->hitsizedice = cMobIndex->hitsizedice;
		pMobIndex->hitplus = cMobIndex->hitplus;
		pMobIndex->damnodice = cMobIndex->damnodice;
		pMobIndex->damsizedice = cMobIndex->damsizedice;
		pMobIndex->damplus = cMobIndex->damplus;
		pMobIndex->gold = cMobIndex->gold;
		pMobIndex->exp = cMobIndex->exp;
		pMobIndex->position = cMobIndex->position;
		pMobIndex->defposition = cMobIndex->defposition;
		pMobIndex->sex = cMobIndex->sex;
		pMobIndex->perm_str = cMobIndex->perm_str;
		pMobIndex->perm_dex = cMobIndex->perm_dex;
		pMobIndex->perm_int = cMobIndex->perm_int;
		pMobIndex->perm_wis = cMobIndex->perm_wis;
		pMobIndex->perm_cha = cMobIndex->perm_cha;
		pMobIndex->perm_con = cMobIndex->perm_con;
		pMobIndex->perm_lck = cMobIndex->perm_lck;
		pMobIndex->race = cMobIndex->race;
		pMobIndex->xflags = cMobIndex->xflags;
		pMobIndex->resistant = cMobIndex->resistant;
		pMobIndex->immune = cMobIndex->immune;
		pMobIndex->susceptible = cMobIndex->susceptible;
		pMobIndex->numattacks = cMobIndex->numattacks;
		pMobIndex->attacks = cMobIndex->attacks;
		pMobIndex->defenses = cMobIndex->defenses;
	}
	iHash = vnum % MAX_KEY_HASH;
	pMobIndex->next = mob_index_hash[iHash];
	mob_index_hash[iHash] = pMobIndex;
	top_mob_index++;

	return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, short door )
{
	EXIT_DATA *pexit, *texit;
	bool broke;

	CREATE( pexit, EXIT_DATA, 1 );
	pexit->vdir = door;
	pexit->rvnum = pRoomIndex->vnum;
	pexit->to_room = to_room;
	pexit->distance = 1;
	if( to_room )
	{
		pexit->vnum = to_room->vnum;
		texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
		if( texit )   /* assign reverse exit pointers */
		{
			texit->rexit = pexit;
			pexit->rexit = texit;
		}
	}
	broke = false;
	for( texit = pRoomIndex->first_exit; texit; texit = texit->next )
		if( door < texit->vdir )
		{
			broke = true;
			break;
		}
	if( !pRoomIndex->first_exit )
		pRoomIndex->first_exit = pexit;
	else
	{
		/*
		 * keep exits in incremental order - insert exit into list
		 */
		if( broke && texit )
		{
			if( !texit->prev )
				pRoomIndex->first_exit = pexit;
			else
				texit->prev->next = pexit;
			pexit->prev = texit->prev;
			pexit->next = texit;
			texit->prev = pexit;
			top_exit++;
			return pexit;
		}
		pRoomIndex->last_exit->next = pexit;
	}
	pexit->next = NULL;
	pexit->prev = pRoomIndex->last_exit;
	pRoomIndex->last_exit = pexit;
	top_exit++;
	return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit, *rev_exit;
	int rnum;
	bool fexit;

	for( rnum = tarea->low_vnum; rnum <= tarea->hi_vnum; rnum++ )
	{
		if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
			continue;

		fexit = false;
		for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
		{
			fexit = true;
			pexit->rvnum = pRoomIndex->vnum;
			if( pexit->vnum <= 0 )
				pexit->to_room = NULL;
			else
				pexit->to_room = get_room_index( pexit->vnum );
		}
		if( !fexit )
			xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}


	for( rnum = tarea->low_vnum; rnum <= tarea->hi_vnum; rnum++ )
	{
		if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
			continue;

		for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
		{
			if( pexit->to_room && !pexit->rexit )
			{
				rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
				if( rev_exit )
				{
					pexit->rexit = rev_exit;
					rev_exit->rexit = pexit;
				}
			}
		}
	}
}

void process_sorting( AREA_DATA *tarea, bool isproto )
{
	sort_area_by_name( tarea );
	sort_area( tarea, isproto );
	if( isproto )
	{
		SET_BIT( tarea->flags, AFLAG_PROTOTYPE );
		tarea->installed = false;
	}
	log_printf( "%-20s: Version %-3d Vnums: %5d - %-5d", tarea->filename, tarea->version, tarea->low_vnum, tarea->hi_vnum );
	if( tarea->low_vnum < 0 || tarea->hi_vnum < 0 )
		log_printf( "%-20s: Bad Vnum Range", tarea->filename );
	if( !tarea->author )
		tarea->author = STRALLOC( "GW:OM" );
	SET_BIT( tarea->status, AREA_LOADED );
	return;
}

EXTRA_DESCR_DATA *fread_fuss_exdesc( FILE *fp )
{
	EXTRA_DESCR_DATA *ed;
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	CREATE( ed, EXTRA_DESCR_DATA, 1 );

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDEXDESC" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDEXDESC";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#ENDEXDESC" ) )
			{
				if( !ed->keyword )
				{
					bug( "%s: Missing ExDesc keyword. Returning NULL.", __func__ );
					STRFREE( ed->description );
					DISPOSE( ed );
					return NULL;
				}

				if( !ed->description )
					ed->description = STRALLOC( "" );

				return ed;
			}
			break;

		case 'E':
			KEY( "ExDescKey", ed->keyword, fread_string( fp ) );
			KEY( "ExDesc", ed->description, fread_string( fp ) );
			break;
		}

		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}

	// Reach this point, you fell through somehow. The data is no longer valid.
	bug( "%s: Reached fallout point! ExtraDesc data invalid.", __func__ );
	DISPOSE( ed );
	return NULL;
}

AFFECT_DATA *fread_fuss_affect( FILE *fp, const char *word )
{
	AFFECT_DATA *paf;
	int pafmod;

	CREATE( paf, AFFECT_DATA, 1 );
	if( !strcmp( word, "Affect" ) )
	{
		paf->type = fread_number( fp );
	}
	else
	{
		int sn;

		sn = skill_lookup( fread_word( fp ) );
		if( sn < 0 )
			bug( "%s: unknown skill.", __func__ );
		else
			paf->type = sn;
	}
	paf->duration = fread_number( fp );
	pafmod = fread_number( fp );
	paf->location = fread_number( fp );
	paf->bitvector = fread_number( fp );

	if( paf->location == APPLY_WEAPONSPELL
		|| paf->location == APPLY_WEARSPELL
		|| paf->location == APPLY_STRIPSN || paf->location == APPLY_REMOVESPELL )
		paf->modifier = slot_lookup( pafmod );
	else
		paf->modifier = pafmod;

	++top_affect;
	return paf;
}

void fread_fuss_exit( FILE *fp, ROOM_INDEX_DATA *pRoomIndex )
{
	EXIT_DATA *pexit = NULL;
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDEXIT" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
			word = "#ENDEXIT";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __FUNCTION__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#ENDEXIT" ) )
			{
				if( !pexit->description )
					pexit->description = STRALLOC( "" );
				if( !pexit->keyword )
					pexit->keyword = STRALLOC( "" );

				return;
			}
			break;

		case 'D':
			KEY( "Desc", pexit->description, fread_string( fp ) );
			KEY( "Distance", pexit->distance, fread_number( fp ) );
			if( !str_cmp( word, "Direction" ) )
			{
				int door = get_dir( fread_flagstring( fp ) );

				if( door < 0 || door > DIR_SOMEWHERE )
				{
					bug( "%s: vnum %d has bad door number %d.", __func__, pRoomIndex->vnum, door );
					if( fBootDb )
						return;
				}
				fMatch = true;
				pexit = make_exit( pRoomIndex, NULL, door );
			}
			break;

		case 'F':
			if( !str_cmp( word, "Flags" ) )
			{
				const char *exitflags = NULL;
				char flag[MAX_INPUT_LENGTH];
				int value;

				exitflags = fread_flagstring( fp );

				while( exitflags[0] != '\0' )
				{
					exitflags = one_argument( exitflags, flag );
					value = get_exflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown exitflag: %s", __func__, flag );
					else
						SET_BIT( pexit->exit_info, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'K':
			KEY( "Key", pexit->key, fread_number( fp ) );
			KEY( "Keywords", pexit->keyword, fread_string( fp ) );
			break;

		case 'T':
			KEY( "ToRoom", pexit->vnum, fread_number( fp ) );
			break;
		}

		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}

	// Reach this point, you fell through somehow. The data is no longer valid.
	bug( "%s: Reached fallout point! Exit data invalid.", __func__ );
	if( pexit )
		extract_exit( pRoomIndex, pexit );
}

void rprog_file_read( ROOM_INDEX_DATA *prog_target, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter != '#' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "ENDFILE";
		}

		fMatch = false;
		if( !str_cmp( word, "ENDFILE" ) )
			break;

		if( !str_cmp( word, "MUDPROG" ) )
		{
			fMatch = true;
			CREATE( mprg, MPROG_DATA, 1 );

			for( ;; )
			{
				word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

				if( word[0] == '\0' )
				{
					log_printf( "%s: EOF encountered reading file!", __func__ );
					word = "#ENDPROG";
				}

				if( !str_cmp( word, "#ENDPROG" ) )
				{
					mprg->next = prog_target->mudprogs;
					prog_target->mudprogs = mprg;
					break;
				}

				switch( word[0] )
				{
				default:
					log_printf( "%s: no match: %s", __FUNCTION__, word );
					fread_to_eol( progfile );
					break;

				case 'A':
					if( !str_cmp( word, "Arglist" ) )
					{
						mprg->arglist = fread_string( progfile );
						mprg->fileprog = true;

						switch( mprg->type )
						{
						case IN_FILE_PROG:
							bug( "%s: Nested file programs are not allowed.", __func__ );
							DISPOSE( mprg );
							break;

						default:
							break;
						}
						fMatch = true;
						break;
					}
					break;

				case 'C':
					KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
					break;

				case 'P':
					if( !str_cmp( word, "Progtype" ) )
					{
						mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
						fMatch = true;
						break;
					}
					break;
				}
			}
		}

		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( progfile );
		}
	}
	FileClose( progfile );
}

void fread_fuss_roomprog( FILE *fp, MPROG_DATA *mprg, ROOM_INDEX_DATA *prog_target )
{
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __FUNCTION__ );
			word = "#ENDPROG";
		}

		if( !str_cmp( word, "#ENDPROG" ) )
			return;

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __FUNCTION__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case 'A':
			if( !str_cmp( word, "Arglist" ) )
			{
				mprg->arglist = fread_string( fp );
				mprg->fileprog = false;

				switch( mprg->type )
				{
				case IN_FILE_PROG:
					rprog_file_read( prog_target, mprg->arglist );
					break;
				default:
					break;
				}
				fMatch = true;
				break;
			}
			break;

		case 'C':
			KEY( "Comlist", mprg->comlist, fread_string( fp ) );
			break;

		case 'P':
			if( !str_cmp( word, "Progtype" ) )
			{
				mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
				xSET_BIT( prog_target->progtypes, mprg->type );
				fMatch = true;
				break;
			}
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void fread_fuss_room( FILE *fp, AREA_DATA *tarea )
{
	ROOM_INDEX_DATA *pRoomIndex = NULL;
	bool oldroom = false;
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDROOM" : fread_word( fp ) );
		char flag[MAX_INPUT_LENGTH];
		int value = 0;


		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDROOM";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			bug( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#ENDROOM" ) )
			{
				if( !pRoomIndex->description )
					pRoomIndex->description = STRALLOC( "" );

				if( !oldroom )
				{
					int iHash = pRoomIndex->vnum % MAX_KEY_HASH;
					pRoomIndex->next = room_index_hash[iHash];
					room_index_hash[iHash] = pRoomIndex;
					LINK( pRoomIndex, tarea->first_room, tarea->last_room, next_aroom, prev_aroom );
					++top_room;
				}
				return;
			}

			if( !str_cmp( word, "#EXIT" ) )
			{
				fread_fuss_exit( fp, pRoomIndex );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "#EXDESC" ) )
			{
				EXTRA_DESCR_DATA *ed = fread_fuss_exdesc( fp );

				if( ed )
					LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "#MUDPROG" ) )
			{
				MPROG_DATA *mprg;

				CREATE( mprg, MPROG_DATA, 1 );
				fread_fuss_roomprog( fp, mprg, pRoomIndex );
				mprg->next = pRoomIndex->mudprogs;
				pRoomIndex->mudprogs = mprg;
				fMatch = true;
				break;
			}
			break;

			/*case 'A':
			   if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
			   {
				  AFFECT_DATA *af = fread_fuss_affect( fp, word );

				  if( af )
					 LINK( af, pRoomIndex->first_permaffect, pRoomIndex->last_permaffect, next, prev );

				  fMatch = true;
				  break;
			   }
			   break; */

		case 'D':
			KEY( "Desc", pRoomIndex->description, fread_string( fp ) );
			break;

		case 'F':
			if( !str_cmp( word, "Flags" ) )
			{
				const char *rflags = fread_flagstring( fp );

				while( rflags[0] != '\0' )
				{
					rflags = one_argument( rflags, flag );
					value = get_rflag( flag );
					if( value < 0 || value > ROOM_MAX )
						bug( "%s: Unknown room flag: %s", __func__, flag );
					else
						xSET_BIT( pRoomIndex->room_flags, value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'G':
			KEY( "Guests", pRoomIndex->guests, fread_string( fp ) );
			break;

		case 'N':
			KEY( "Name", pRoomIndex->name, fread_string( fp ) );
			break;

		case 'O':
			KEY( "Owner", pRoomIndex->owner, fread_string( fp ) );
			break;

		case 'R':
			if( !str_cmp( word, "Reset" ) )
			{
				load_room_reset( pRoomIndex, fp );
				fMatch = true;
				break;
			}
			break;

		case 'S':
			if( !str_cmp( word, "Sector" ) )
			{
				int sector = get_secflag( fread_flagstring( fp ) );

				if( sector < 0 || sector >= SECT_MAX )
				{
					bug( "%s: Room #%d has bad sector type.", __func__, pRoomIndex->vnum );
					sector = 1;
				}

				pRoomIndex->sector_type = sector;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3;

				x1 = x2 = x3 = 0;
				sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

				pRoomIndex->tele_delay = x1;
				pRoomIndex->tele_vnum = x2;
				pRoomIndex->tunnel = x3;

				fMatch = true;
				break;
			}
			break;

		case 'V':
			if( !str_cmp( word, "Vnum" ) )
			{
				bool tmpBootDb = fBootDb;
				fBootDb = false;

				int vnum = fread_number( fp );

				if( get_room_index( vnum ) )
				{
					if( tmpBootDb )
					{
						fBootDb = tmpBootDb;
						bug( "%s: vnum %d duplicated.", __func__, vnum );

						// Try to recover, read to end of duplicated room and then bail out
						for( ;; )
						{
							word = feof( fp ) ? "#ENDROOM" : fread_word( fp );

							if( !str_cmp( word, "#ENDROOM" ) )
								return;
						}
					}
					else
					{
						pRoomIndex = get_room_index( vnum );
						log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning room: %d", vnum );
						clean_room( pRoomIndex );
						oldroom = true;
					}
				}
				else
				{
					CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
					oldroom = false;
				}
				pRoomIndex->vnum = vnum;
				pRoomIndex->area = tarea;
				fBootDb = tmpBootDb;

				if( fBootDb )
				{
					if( !tarea->low_vnum )
						tarea->low_vnum = vnum;
					if( vnum > tarea->hi_vnum )
						tarea->hi_vnum = vnum;
				}
				fMatch = true;
				break;
			}
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void oprog_file_read( OBJ_INDEX_DATA *prog_target, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;
	bool fMatch;   // Unused, but needed to shut the compiler up about the KEY macro

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter != '#' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "ENDFILE";
		}

		if( !str_cmp( word, "ENDFILE" ) )
			break;

		if( !str_cmp( word, "MUDPROG" ) )
		{
			fMatch = true;
			CREATE( mprg, MPROG_DATA, 1 );

			for( ;; )
			{
				word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

				if( word[0] == '\0' )
				{
					log_printf( "%s: EOF encountered reading file!", __func__ );
					word = "#ENDPROG";
				}

				if( !str_cmp( word, "#ENDPROG" ) )
				{
					mprg->next = prog_target->mudprogs;
					prog_target->mudprogs = mprg;
					break;
				}

				switch( word[0] )
				{
				default:
					log_printf( "%s: no match: %s", __func__, word );
					fread_to_eol( progfile );
					break;

				case 'A':
					if( !str_cmp( word, "Arglist" ) )
					{
						mprg->arglist = fread_string( progfile );
						mprg->fileprog = true;

						switch( mprg->type )
						{
						case IN_FILE_PROG:
							bug( "%s: Nested file programs are not allowed.", __func__ );
							DISPOSE( mprg );
							break;

						default:
							break;
						}
						break;
					}
					break;

				case 'C':
					KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
					break;

				case 'P':
					if( !str_cmp( word, "Progtype" ) )
					{
						mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
						break;
					}
					break;
				}
			}
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( progfile );
		}
	}
	FileClose( progfile );
}

void fread_fuss_objprog( FILE *fp, MPROG_DATA *mprg, OBJ_INDEX_DATA *prog_target )
{
	bool fMatch;

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDPROG";
		}

		if( !str_cmp( word, "#ENDPROG" ) )
			return;

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case 'A':
			if( !str_cmp( word, "Arglist" ) )
			{
				mprg->arglist = fread_string( fp );
				mprg->fileprog = false;

				switch( mprg->type )
				{
				case IN_FILE_PROG:
					oprog_file_read( prog_target, mprg->arglist );
					break;
				default:
					break;
				}
				fMatch = true;
				break;
			}
			break;

		case 'C':
			KEY( "Comlist", mprg->comlist, fread_string( fp ) );
			break;

		case 'P':
			if( !str_cmp( word, "Progtype" ) )
			{
				mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
				xSET_BIT( prog_target->progtypes, mprg->type );
				fMatch = true;
				break;
			}
			break;
		}

		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void fread_fuss_object( FILE *fp, AREA_DATA *tarea )
{
	OBJ_INDEX_DATA *pObjIndex = NULL;
	bool oldobj = false;
	bool fMatch;

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDOBJECT" : fread_word( fp ) );
		char flag[MAX_INPUT_LENGTH];
		int value = 0;

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDOBJECT";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			bug( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#ENDOBJECT" ) )
			{
				if( !pObjIndex->description )
					pObjIndex->description = STRALLOC( "" );
				if( !pObjIndex->action_desc )
					pObjIndex->action_desc = STRALLOC( "" );

				if( !oldobj )
				{
					int iHash = pObjIndex->vnum % MAX_KEY_HASH;
					pObjIndex->next = obj_index_hash[iHash];
					obj_index_hash[iHash] = pObjIndex;
					++top_obj_index;
				}
				return;
			}

			if( !str_cmp( word, "#EXDESC" ) )
			{
				EXTRA_DESCR_DATA *ed = fread_fuss_exdesc( fp );
				if( ed )
					LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "#MUDPROG" ) )
			{
				MPROG_DATA *mprg;

				CREATE( mprg, MPROG_DATA, 1 );
				fread_fuss_objprog( fp, mprg, pObjIndex );
				mprg->next = pObjIndex->mudprogs;
				pObjIndex->mudprogs = mprg;

				fMatch = true;
				break;
			}
			break;

		case 'A':
			KEY( "Action", pObjIndex->action_desc, fread_string( fp ) );

			if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
			{
				AFFECT_DATA *af = fread_fuss_affect( fp, word );

				if( af )
					LINK( af, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
				fMatch = true;
				break;
			}
			break;

		case 'F':
			if( !str_cmp( word, "Flags" ) )
			{
				const char *eflags = fread_flagstring( fp );

				while( eflags[0] != '\0' )
				{
					eflags = one_argument( eflags, flag );
					value = get_oflag( flag );
					if( value < 0 || value > MAX_ITEM_FLAG )
						bug( "%s: Unknown object extraflag: %s", __func__, flag );
					else
						xSET_BIT( pObjIndex->extra_flags, value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'K':
			KEY( "Keywords", pObjIndex->name, fread_string( fp ) );
			break;

		case 'L':
			KEY( "Long", pObjIndex->description, fread_string( fp ) );
			break;

		case 'S':
			KEY( "Short", pObjIndex->short_descr, fread_string( fp ) );
			if( !str_cmp( word, "Spells" ) )
			{
				switch( pObjIndex->item_type )
				{
				default:
					break;

				case ITEM_PILL:
				case ITEM_POTION:
				case ITEM_SCROLL:
					pObjIndex->value[1] = skill_lookup( fread_word( fp ) );
					pObjIndex->value[2] = skill_lookup( fread_word( fp ) );
					pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
					break;

				case ITEM_STAFF:
				case ITEM_WAND:
					pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
					break;

				case ITEM_SALVE:
					pObjIndex->value[4] = skill_lookup( fread_word( fp ) );
					pObjIndex->value[5] = skill_lookup( fread_word( fp ) );
					break;
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5;

				x1 = x2 = x3 = x4 = x5 = 0;
				sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

				pObjIndex->weight = x1;
				pObjIndex->cost = x2;
				pObjIndex->rent = x3;
				pObjIndex->level = x4;
				pObjIndex->layers = x5;

				fMatch = true;
				break;
			}
			break;

		case 'T':
			if( !str_cmp( word, "Type" ) )
			{
				value = get_otype( fread_flagstring( fp ) );

				if( value < 0 )
				{
					bug( "%s: vnum %d: Object has invalid type! Defaulting to trash.", __func__, pObjIndex->vnum );
					value = get_otype( "trash" );
				}
				pObjIndex->item_type = value;
				fMatch = true;
				break;
			}
			break;

		case 'V':
			if( !str_cmp( word, "Values" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5, x6;
				x1 = x2 = x3 = x4 = x5 = x6 = 0;

				sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

				pObjIndex->value[0] = x1;
				pObjIndex->value[1] = x2;
				pObjIndex->value[2] = x3;
				pObjIndex->value[3] = x4;
				pObjIndex->value[4] = x5;
				pObjIndex->value[5] = x6;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Vnum" ) )
			{
				bool tmpBootDb = fBootDb;
				fBootDb = false;

				int vnum = fread_number( fp );

				if( get_obj_index( vnum ) )
				{
					if( tmpBootDb )
					{
						fBootDb = tmpBootDb;
						bug( "%s: vnum %d duplicated.", __func__, vnum );

						// Try to recover, read to end of duplicated object and then bail out
						for( ;; )
						{
							word = feof( fp ) ? "#ENDOBJECT" : fread_word( fp );

							if( !str_cmp( word, "#ENDOBJECT" ) )
								return;
						}
					}
					else
					{
						pObjIndex = get_obj_index( vnum );
						log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning object: %d", vnum );
						clean_obj( pObjIndex );
						oldobj = true;
					}
				}
				else
				{
					CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
					oldobj = false;
				}
				pObjIndex->vnum = vnum;
				fBootDb = tmpBootDb;

				if( fBootDb )
				{
					if( !tarea->low_vnum )
						tarea->low_vnum = vnum;
					if( vnum > tarea->hi_vnum )
						tarea->hi_vnum = vnum;
				}
				fMatch = true;
				break;
			}
			break;

		case 'W':
			if( !str_cmp( word, "WFlags" ) )
			{
				const char *wflags = fread_flagstring( fp );

				while( wflags[0] != '\0' )
				{
					wflags = one_argument( wflags, flag );
					value = get_wflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown wear flag: %s", __func__, flag );
					else
						SET_BIT( pObjIndex->wear_flags, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void mprog_file_read( MOB_INDEX_DATA *prog_target, const char *f )
{
	MPROG_DATA *mprg = NULL;
	char MUDProgfile[256];
	FILE *progfile;
	char letter;
	bool fMatch;

	snprintf( MUDProgfile, 256, "%s%s", PROG_DIR, f );

	if( !( progfile = FileOpen( MUDProgfile, "r" ) ) )
	{
		bug( "%s: couldn't open mudprog file", __func__ );
		return;
	}

	for( ;; )
	{
		letter = fread_letter( progfile );

		if( letter != '#' )
		{
			bug( "%s: MUDPROG char", __func__ );
			break;
		}

		const char *word = ( feof( progfile ) ? "ENDFILE" : fread_word( progfile ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "ENDFILE";
		}

		fMatch = false;

		if( !str_cmp( word, "ENDFILE" ) )
			break;

		if( !str_cmp( word, "MUDPROG" ) )
		{
			CREATE( mprg, MPROG_DATA, 1 );

			fMatch = true;

			for( ;; )
			{
				word = ( feof( progfile ) ? "#ENDPROG" : fread_word( progfile ) );

				if( word[0] == '\0' )
				{
					log_printf( "%s: EOF encountered reading file!", __func__ );
					word = "#ENDPROG";
				}

				if( !str_cmp( word, "#ENDPROG" ) )
				{
					mprg->next = prog_target->mudprogs;
					prog_target->mudprogs = mprg;
					break;
				}

				switch( word[0] )
				{
				default:
					log_printf( "%s: no match: %s", __func__, word );
					fread_to_eol( progfile );
					break;

				case 'A':
					if( !str_cmp( word, "Arglist" ) )
					{
						mprg->arglist = fread_string( progfile );
						mprg->fileprog = true;

						switch( mprg->type )
						{
						case IN_FILE_PROG:
							bug( "%s: Nested file programs are not allowed.", __func__ );
							DISPOSE( mprg );
							break;

						default:
							break;
						}
						break;
					}
					break;

				case 'C':
					KEY( "Comlist", mprg->comlist, fread_string( progfile ) );
					break;

				case 'P':
					if( !str_cmp( word, "Progtype" ) )
					{
						mprg->type = mprog_name_to_type( fread_flagstring( progfile ) );
						break;
					}
					break;
				}
			}
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( progfile );
		}
	}
	FileClose( progfile );
}

void fread_fuss_mobprog( FILE *fp, MPROG_DATA *mprg, MOB_INDEX_DATA *prog_target )
{
	bool fMatch;

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDPROG" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDPROG";
		}

		if( !str_cmp( word, "#ENDPROG" ) )
			return;

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case 'A':
			if( !str_cmp( word, "Arglist" ) )
			{
				mprg->arglist = fread_string( fp );
				mprg->fileprog = false;

				switch( mprg->type )
				{
				case IN_FILE_PROG:
					mprog_file_read( prog_target, mprg->arglist );
					break;
				default:
					break;
				}
				fMatch = true;
				break;
			}
			break;

		case 'C':
			KEY( "Comlist", mprg->comlist, fread_string( fp ) );
			break;

		case 'P':
			if( !str_cmp( word, "Progtype" ) )
			{
				mprg->type = mprog_name_to_type( fread_flagstring( fp ) );
				xSET_BIT( prog_target->progtypes, mprg->type );
				fMatch = true;
				break;
			}
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void fread_fuss_mobile( FILE *fp, AREA_DATA *tarea )
{
	MOB_INDEX_DATA *pMobIndex = NULL;
	bool oldmob = false;
	bool fMatch;

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDMOBILE" : fread_word( fp ) );
		char flag[MAX_INPUT_LENGTH];
		int value = 0;

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDMOBILE";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#MUDPROG" ) )
			{
				MPROG_DATA *mprg;
				CREATE( mprg, MPROG_DATA, 1 );
				fread_fuss_mobprog( fp, mprg, pMobIndex );
				mprg->next = pMobIndex->mudprogs;
				pMobIndex->mudprogs = mprg;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "#ENDMOBILE" ) )
			{
				if( !pMobIndex->long_descr )
					pMobIndex->long_descr = STRALLOC( "" );
				if( !pMobIndex->description )
					pMobIndex->description = STRALLOC( "" );

				if( !oldmob )
				{
					int iHash = pMobIndex->vnum % MAX_KEY_HASH;
					pMobIndex->next = mob_index_hash[iHash];
					mob_index_hash[iHash] = pMobIndex;
					++top_mob_index;
				}
				return;
			}
			break;

		case 'A':
			if( !str_cmp( word, "Actflags" ) )
			{
				const char *actflags = NULL;

				actflags = fread_flagstring( fp );

				while( actflags[0] != '\0' )
				{
					actflags = one_argument( actflags, flag );
					value = get_actflag( flag );
					if( value < 0 || value > MAX_ACT_FLAG )
						bug( "%s: Unknown actflag: %s", __func__, flag );
					else
						xSET_BIT( pMobIndex->act, value );
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Affected" ) )
			{
				const char *affectflags = NULL;

				affectflags = fread_flagstring( fp );

				while( affectflags[0] != '\0' )
				{
					affectflags = one_argument( affectflags, flag );
					value = get_aflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown affectflag: %s", __func__, flag );
					else
						SET_BIT( pMobIndex->affected_by, 1 << value );
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Attacks" ) )
			{
				const char *attacks = fread_flagstring( fp );

				while( attacks[0] != '\0' )
				{
					attacks = one_argument( attacks, flag );
					value = get_attackflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown attackflag: %s", __func__, flag );
					else
						SET_BIT( pMobIndex->attacks, 1 << value );
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Attribs" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5, x6, x7;

				x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
				sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );

				pMobIndex->perm_str = x1;
				pMobIndex->perm_int = x2;
				pMobIndex->perm_wis = x3;
				pMobIndex->perm_dex = x4;
				pMobIndex->perm_con = x5;
				pMobIndex->perm_cha = x6;
				pMobIndex->perm_lck = x7;

				fMatch = true;

				break;
			}
			break;

		case 'B':
			if( !str_cmp( word, "Bodyparts" ) )
			{
				const char *bodyparts = fread_flagstring( fp );

				while( bodyparts[0] != '\0' )
				{
					bodyparts = one_argument( bodyparts, flag );
					value = get_partflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown bodypart: %s", __func__, flag );
					else
						SET_BIT( pMobIndex->xflags, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'D':
			if( !str_cmp( word, "Defenses" ) )
			{
				const char *defenses = fread_flagstring( fp );

				while( defenses[0] != '\0' )
				{
					defenses = one_argument( defenses, flag );
					value = get_defenseflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown defenseflag: %s", __func__, flag );
					else
						SET_BIT( pMobIndex->defenses, 1 << value );
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "DefPos" ) )
			{
				short position = get_npc_position( fread_flagstring( fp ) );

				if( position < 0 || position > POS_DRAG )
				{
					bug( "%s: vnum %d: Mobile in invalid default position! Defaulting to standing.", __func__, pMobIndex->vnum );
					position = POS_STANDING;
				}
				pMobIndex->defposition = position;
				fMatch = true;
				break;
			}

			KEY( "Desc", pMobIndex->description, fread_string( fp ) );
			break;

		case 'G':
			if( !str_cmp( word, "Gender" ) )
			{
				short sex = get_npc_sex( fread_flagstring( fp ) );

				if( sex < 0 || sex > SEX_FEMALE )
				{
					bug( "%s: vnum %d: Mobile has invalid sex! Defaulting to nonbinary.", __func__, pMobIndex->vnum );
					sex = SEX_NONBINARY;
				}
				pMobIndex->sex = sex;
				fMatch = true;
				break;
			}
			break;

		case 'I':
			if( !str_cmp( word, "Immune" ) )
			{
				const char *immune = fread_flagstring( fp );

				while( immune[0] != '\0' )
				{
					immune = one_argument( immune, flag );
					value = get_risflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown RIS flag (I): %s", __func__, flag );
					else
						SET_BIT( pMobIndex->immune, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'K':
			KEY( "Keywords", pMobIndex->player_name, fread_string( fp ) );
			break;

		case 'L':
			KEY( "Long", pMobIndex->long_descr, fread_string( fp ) );
			break;

		case 'P':
			if( !str_cmp( word, "Position" ) )
			{
				short position = get_npc_position( fread_flagstring( fp ) );

				if( position < 0 || position > POS_DRAG )
				{
					bug( "%s: vnum %d: Mobile in invalid position! Defaulting to standing.", __func__, pMobIndex->vnum );
					position = POS_STANDING;
				}
				pMobIndex->position = position;
				fMatch = true;
				break;
			}
			break;

		case 'R':
			if( !str_cmp( word, "Race" ) )
			{
				short race = get_npc_race( fread_flagstring( fp ) );

				if( race < 0 || race >= MAX_NPC_RACE )
				{
					bug( "%s: vnum %d: Mob has invalid race! Defaulting to Colonist.", __func__, pMobIndex->vnum );
					race = get_npc_race( "Colonist" );
				}

				pMobIndex->race = race;
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "RepairData" ) )
			{
				int iFix;
				REPAIR_DATA *rShop;

				CREATE( rShop, REPAIR_DATA, 1 );
				rShop->keeper = pMobIndex->vnum;
				for( iFix = 0; iFix < MAX_FIX; ++iFix )
					rShop->fix_type[iFix] = fread_number( fp );
				rShop->profit_fix = fread_number( fp );
				rShop->shop_type = fread_number( fp );
				rShop->open_hour = fread_number( fp );
				rShop->close_hour = fread_number( fp );

				pMobIndex->rShop = rShop;
				LINK( rShop, first_repair, last_repair, next, prev );
				++top_repair;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Resist" ) )
			{
				const char *resist = fread_flagstring( fp );

				while( resist[0] != '\0' )
				{
					resist = one_argument( resist, flag );
					value = get_risflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown RIS flag (R): %s", __func__, flag );
					else
						SET_BIT( pMobIndex->resistant, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'S':
			if( !str_cmp( word, "Saves" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5;

				x1 = x2 = x3 = x4 = x5 = 0;
				sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

				pMobIndex->saving_poison_death = x1;
				pMobIndex->saving_wand = x2;
				pMobIndex->saving_para_petri = x3;
				pMobIndex->saving_breath = x4;
				pMobIndex->saving_spell_staff = x5;

				fMatch = true;
				break;
			}

			KEY( "Short", pMobIndex->short_descr, fread_string( fp ) );

			if( !str_cmp( word, "ShopData" ) )
			{
				int iTrade;
				SHOP_DATA *pShop;

				CREATE( pShop, SHOP_DATA, 1 );
				pShop->keeper = pMobIndex->vnum;
				for( iTrade = 0; iTrade < MAX_TRADE; ++iTrade )
					pShop->buy_type[iTrade] = fread_number( fp );
				pShop->profit_buy = fread_number( fp );
				pShop->profit_sell = fread_number( fp );
				pShop->profit_buy = URANGE( pShop->profit_sell + 5, pShop->profit_buy, 1000 );
				pShop->profit_sell = URANGE( 0, pShop->profit_sell, pShop->profit_buy - 5 );
				pShop->open_hour = fread_number( fp );
				pShop->close_hour = fread_number( fp );

				pMobIndex->pShop = pShop;
				LINK( pShop, first_shop, last_shop, next, prev );
				++top_shop;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Specfun" ) )
			{
				const char *temp = fread_flagstring( fp );
				if( !pMobIndex )
				{
					bug( "%s: Specfun: Invalid mob vnum!", __func__ );
					break;
				}
				if( !( pMobIndex->spec_fun = spec_lookup( temp ) ) )
				{
					bug( "%s: Specfun: vnum %d, no spec_fun called %s.", __func__, pMobIndex->vnum, temp );
					pMobIndex->spec_funname = NULL;
				}
				else
					pMobIndex->spec_funname = STRALLOC( temp );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Specfun2" ) )
			{
				const char *temp = fread_flagstring( fp );
				if( !pMobIndex )
				{
					bug( "%s: Specfun: Invalid mob vnum!", __func__ );
					break;
				}
				if( !( pMobIndex->spec_2 = spec_lookup( temp ) ) )
				{
					bug( "%s: Specfun: vnum %d, no spec_fun called %s.", __func__, pMobIndex->vnum, temp );
					pMobIndex->spec_funname2 = NULL;
				}
				else
					pMobIndex->spec_funname2 = STRALLOC( temp );
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats1" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5, x6;

				x1 = x2 = x3 = x4 = x5 = x6 = 0;
				sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

				pMobIndex->alignment = x1;
				pMobIndex->level = x2;
				pMobIndex->mobthac0 = x3;
				pMobIndex->ac = x4;
				pMobIndex->gold = x5;
				pMobIndex->exp = x6;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats2" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3;
				x1 = x2 = x3 = 0;
				sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

				pMobIndex->hitnodice = x1;
				pMobIndex->hitsizedice = x2;
				pMobIndex->hitplus = x3;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats3" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3;
				x1 = x2 = x3 = 0;
				sscanf( ln, "%d %d %d", &x1, &x2, &x3 );

				pMobIndex->damnodice = x1;
				pMobIndex->damsizedice = x2;
				pMobIndex->damplus = x3;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Stats4" ) )
			{
				char *ln = fread_line( fp );
				int x1, x2, x3, x4, x5;

				x1 = x2 = x3 = x4 = x5 = 0;
				sscanf( ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );

				pMobIndex->height = x1;
				pMobIndex->weight = x2;
				pMobIndex->numattacks = x3;
				pMobIndex->hitroll = x4;
				pMobIndex->damroll = x5;

				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Suscept" ) )
			{
				const char *suscep = fread_flagstring( fp );

				while( suscep[0] != '\0' )
				{
					suscep = one_argument( suscep, flag );
					value = get_risflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown RIS flag (S): %s", __func__, flag );
					else
						SET_BIT( pMobIndex->susceptible, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'V':
			if( !str_cmp( word, "VIPFlags" ) )
			{
				const char *vip = fread_flagstring( fp );

				while( vip[0] != '\0' )
				{
					vip = one_argument( vip, flag );
					value = get_vip_flag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown VIP flag: %s", __func__, flag );
					else
						SET_BIT( pMobIndex->vip_flags, 1 << value );
				}
				fMatch = true;
				break;
			}

			if( !str_cmp( word, "Vnum" ) )
			{
				bool tmpBootDb = fBootDb;
				fBootDb = false;

				int vnum = fread_number( fp );

				if( get_mob_index( vnum ) )
				{
					if( tmpBootDb )
					{
						fBootDb = tmpBootDb;
						bug( "%s: vnum %d duplicated.", __func__, vnum );

						// Try to recover, read to end of duplicated mobile and then bail out
						for( ;; )
						{
							word = feof( fp ) ? "#ENDMOBILE" : fread_word( fp );

							if( !str_cmp( word, "#ENDMOBILE" ) )
								return;
						}
					}
					else
					{
						pMobIndex = get_mob_index( vnum );
						log_printf_plus( LOG_BUILD, sysdata.build_level, "Cleaning mobile: %d", vnum );
						clean_mob( pMobIndex );
						oldmob = true;
					}
				}
				else
				{
					CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
					oldmob = false;
				}
				pMobIndex->vnum = vnum;
				fBootDb = tmpBootDb;

				if( fBootDb )
				{
					if( !tarea->low_vnum )
						tarea->low_vnum = vnum;
					if( vnum > tarea->hi_vnum )
						tarea->hi_vnum = vnum;
				}
				fMatch = true;
				break;
			}
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

void fread_fuss_areadata( FILE *fp, AREA_DATA *tarea )
{
	bool fMatch;

	for( ;; )
	{
		const char *word = ( feof( fp ) ? "#ENDAREADATA" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			log_printf( "%s: EOF encountered reading file!", __func__ );
			word = "#ENDAREADATA";
		}

		fMatch = false;

		switch( word[0] )
		{
		default:
			log_printf( "%s: no match: %s", __func__, word );
			fread_to_eol( fp );
			fMatch = true;
			break;

		case '#':
			if( !str_cmp( word, "#ENDAREADATA" ) )
			{
				tarea->age = tarea->reset_frequency;
				return;
			}
			break;

		case 'A':
			KEY( "Author", tarea->author, fread_string( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "Economy" ) )
			{
				tarea->high_economy = fread_number( fp );
				tarea->low_economy = fread_number( fp );
				fMatch = true;
				break;
			}
			break;

		case 'F':
			if( !str_cmp( word, "Flags" ) )
			{
				const char *areaflags = NULL;
				char flag[MAX_INPUT_LENGTH];
				int value;

				areaflags = fread_flagstring( fp );

				while( areaflags[0] != '\0' )
				{
					areaflags = one_argument( areaflags, flag );
					value = get_areaflag( flag );
					if( value < 0 || value > 31 )
						bug( "%s: Unknown area flag: %s", __func__, flag );
					else
						SET_BIT( tarea->flags, 1 << value );
				}
				fMatch = true;
				break;
			}
			break;

		case 'N':
			KEY( "Name", tarea->name, fread_string_nohash( fp ) );
			break;

		case 'R':
			if( !str_cmp( word, "Ranges" ) )
			{
				int x1, x2, x3, x4;
				char *ln;

				ln = fread_line( fp );

				x1 = x2 = x3 = x4 = 0;
				sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

				tarea->low_soft_range = x1;
				tarea->hi_soft_range = x2;
				tarea->low_hard_range = x3;
				tarea->hi_hard_range = x4;

				fMatch = true;
				break;
			}
			KEY( "ResetMsg", tarea->resetmsg, fread_string_nohash( fp ) );
			KEY( "ResetFreq", tarea->reset_frequency, fread_number( fp ) );
			break;

		case 'V':
			KEY( "Version", tarea->version, fread_number( fp ) );
			break;
		}
		if( !fMatch )
		{
			bug( "%s: unknown word: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
}

/*
 * Load an 'area' header line.
 */
AREA_DATA *create_area( void )
{
	AREA_DATA *pArea;

	CREATE( pArea, AREA_DATA, 1 );
	pArea->first_room = pArea->last_room = NULL;
	pArea->name = NULL;
	pArea->author = NULL;
	pArea->filename = str_dup( strArea );
	pArea->age = 15;
	pArea->reset_frequency = 15;
	pArea->nplayer = 0;
	pArea->low_vnum = 0;
	pArea->hi_vnum = 0;
	pArea->low_soft_range = 0;
	pArea->hi_soft_range = MAX_LEVEL;
	pArea->low_hard_range = 0;
	pArea->hi_hard_range = MAX_LEVEL;
	pArea->version = 1;

	LINK( pArea, first_area, last_area, next, prev );
	++top_area;
	return pArea;
}

AREA_DATA *fread_fuss_area( AREA_DATA *tarea, FILE *fp )
{
	for( ;; )
	{
		char letter;
		const char *word;

		letter = fread_letter( fp );
		if( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if( letter != '#' )
		{
			bug( "%s: # not found. Invalid format.", __func__ );
			if( fBootDb )
				exit( 1 );
			break;
		}

		word = ( feof( fp ) ? "ENDAREA" : fread_word( fp ) );

		if( word[0] == '\0' )
		{
			bug( "%s: EOF encountered reading file!", __func__ );
			word = "ENDAREA";
		}

		if( !str_cmp( word, "AREADATA" ) )
		{
			if( !tarea )
				tarea = create_area( );
			fread_fuss_areadata( fp, tarea );
		}
		else if( !str_cmp( word, "MOBILE" ) )
			fread_fuss_mobile( fp, tarea );
		else if( !str_cmp( word, "OBJECT" ) )
			fread_fuss_object( fp, tarea );
		else if( !str_cmp( word, "ROOM" ) )
			fread_fuss_room( fp, tarea );
		else if( !str_cmp( word, "ENDAREA" ) )
			break;
		else
		{
			bug( "%s: Bad section header: %s", __func__, word );
			fread_to_eol( fp );
		}
	}
	return tarea;
}

void load_area_file( const char *filename, bool isproto )
{
	AREA_DATA *tarea = NULL;
	char *word;
	int aversion = 0;

	if( !( fpArea = FileOpen( filename, "r" ) ) )
	{
		perror( filename );
		bug( "%s: error loading file (can't open) %s", __func__, filename );
		return;
	}

	if( fread_letter( fpArea ) != '#' )
	{
		if( fBootDb )
		{
			bug( "%s: No # found at start of area file.", __func__ );
			exit( 1 );
		}
		else
		{
			bug( "%s: No # found at start of area file.", __func__ );
			FileClose( fpArea );
			return;
		}
	}

	word = fread_word( fpArea );

	// New FUSS area format support -- Samson 7/5/07
	if( !str_cmp( word, "FUSSAREA" ) )
	{
		tarea = fread_fuss_area( tarea, fpArea );
		FileClose( fpArea );

		if( tarea )
			process_sorting( tarea, isproto );
		return;
	}

	// Drop through to the old format processor
	if( !str_cmp( word, "AREA" ) )
	{
		if( fBootDb )
			tarea = load_area( fpArea, 0 );
		else
		{
			DISPOSE( tarea->name );
			tarea->name = fread_string_nohash( fpArea );
		}
	}
	// Only seen at this stage for help.are
	else if( !str_cmp( word, "HELPS" ) )
		load_helps( fpArea );
	// Only seen at this stage for SmaugWiz areas
	else if( !str_cmp( word, "VERSION" ) )
		aversion = fread_number( fpArea );

	for( ;; )
	{
		if( fread_letter( fpArea ) != '#' )
		{
			bug( "%s: # not found", __func__ );
			exit( 1 );
		}

		word = fread_word( fpArea );

		if( word[0] == '$' )
			break;
		// Only seen at this stage for SmaugWiz areas. The format had better be right or there'll be trouble here!
		else if( !str_cmp( word, "AREA" ) )
			tarea = load_area( fpArea, aversion );
		else if( !str_cmp( word, "VERSION" ) )
			load_version( tarea, fpArea );
		else if( !str_cmp( word, "AUTHOR" ) )
			load_author( tarea, fpArea );
		else if( !str_cmp( word, "OWNED_BY" ) )
			load_owned_by( tarea, fpArea );
		else if( !str_cmp( word, "FLAGS" ) )
			load_flags( tarea, fpArea );
		else if( !str_cmp( word, "RANGES" ) )
			load_ranges( tarea, fpArea );
		else if( !str_cmp( word, "ECONOMY" ) )
			load_economy( tarea, fpArea );
		else if( !str_cmp( word, "RESETMSG" ) )
			load_resetmsg( tarea, fpArea );
		/*
		 * Rennard
		 */
		else if( !str_cmp( word, "HELPS" ) )
			load_helps( fpArea );
		else if( !str_cmp( word, "MOBILES" ) )
			load_mobiles( tarea, fpArea );
		else if( !str_cmp( word, "OBJECTS" ) )
			load_objects( tarea, fpArea );
		else if( !str_cmp( word, "RESETS" ) )
			load_resets( tarea, fpArea );
		else if( !str_cmp( word, "ROOMS" ) )
			load_rooms( tarea, fpArea );
		else if( !str_cmp( word, "SHOPS" ) )
			load_shops( tarea, fpArea );
		else if( !str_cmp( word, "REPAIRS" ) )
			load_repairs( tarea, fpArea );
		else if( !str_cmp( word, "SPECIALS" ) )
			load_specials( tarea, fpArea );
		else
		{
			bug( "%s: bad section name: %s", __func__, word );
			if( fBootDb )
				exit( 1 );
			else
			{
				FileClose( fpArea );
				return;
			}
		}
	}
	FileClose( fpArea );

	if( tarea )
		process_sorting( tarea, isproto );
	else
		fprintf( stderr, "(%s)\n", filename );
}

/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist( void )
{
	DIR *dp;
	struct dirent *dentry;
	char buf[256];

	dp = opendir( BUILD_DIR );
	dentry = readdir( dp );
	while( dentry )
	{
		if( dentry->d_name[0] != '.' )
		{
			/*
			* Added by Tarl 3 Dec 02 because we are now using CVS
			*/
			if( str_cmp( dentry->d_name, "CVS" ) && !str_infix( ".are", dentry->d_name ) )
			{
				if( str_infix( ".bak", dentry->d_name ) && str_infix( ".installed", dentry->d_name ) )
				{
					snprintf( buf, 256, "%s%s", BUILD_DIR, dentry->d_name );
					fBootDb = true;
					snprintf( strArea, MIL, "%s", dentry->d_name );
					load_area_file( buf, true );
					fBootDb = false;
				}
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );
}

/*
 * Sort areas by name alphanumercially
 *      - 4/27/97, Fireblade
 */
void sort_area_by_name( AREA_DATA *pArea )
{
	AREA_DATA *temp_area;

	if( !pArea )
	{
		bug( "%s: NULL pArea", __func__ );
		return;
	}
	for( temp_area = first_area_name; temp_area; temp_area = temp_area->next_sort_name )
	{
		if( strcmp( pArea->name, temp_area->name ) < 0 )
		{
			INSERT( pArea, temp_area, first_area_name, next_sort_name, prev_sort_name );
			break;
		}
	}
	if( !temp_area )
	{
		LINK( pArea, first_area_name, last_area_name, next_sort_name, prev_sort_name );
	}
}

/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
	AREA_DATA *area = NULL;
	AREA_DATA *first_sort, *last_sort;
	bool found;

	if( !pArea )
	{
		bug( "%s: NULL pArea", __func__ );
		return;
	}

	if( proto )
	{
		first_sort = first_bsort;
		last_sort = last_bsort;
	}
	else
	{
		first_sort = first_asort;
		last_sort = last_asort;
	}

	found = false;
	pArea->next_sort = NULL;
	pArea->prev_sort = NULL;

	if( !first_sort )
	{
		pArea->prev_sort = NULL;
		pArea->next_sort = NULL;
		first_sort = pArea;
		last_sort = pArea;
		found = true;
	}
	else
		for( area = first_sort; area; area = area->next_sort )
			if( pArea->low_vnum < area->low_vnum )
			{
				if( !area->prev_sort )
					first_sort = pArea;
				else
					area->prev_sort->next_sort = pArea;
				pArea->prev_sort = area->prev_sort;
				pArea->next_sort = area;
				area->prev_sort = pArea;
				found = true;
				break;
			}

	if( !found )
	{
		pArea->prev_sort = last_sort;
		pArea->next_sort = NULL;
		last_sort->next_sort = pArea;
		last_sort = pArea;
	}

	if( proto )
	{
		first_bsort = first_sort;
		last_bsort = last_sort;
	}
	else
	{
		first_asort = first_sort;
		last_asort = last_sort;
	}
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl, const char *loadst, const char *notloadst )
{
	AREA_DATA *pArea, *first_sort;
	int count, loaded;

	count = 0;
	loaded = 0;
	set_pager_color( AT_PLAIN, ch );
	if( proto )
		first_sort = first_bsort;
	else
		first_sort = first_asort;
	for( pArea = first_sort; pArea; pArea = pArea->next_sort )
	{
		if( IS_SET( pArea->status, AREA_DELETED ) )
			continue;
		if( pArea->low_vnum < low )
			continue;
		if( pArea->hi_vnum > high )
			break;
		if( IS_SET( pArea->status, AREA_LOADED ) )
			loaded++;
		else if( !shownl )
			continue;
		pager_printf( ch, "%-15s| Rooms: %5d - %-5d"
			" Objs: %5d - %-5d Mobs: %5d - %-5d%s\r\n",
			( pArea->filename ? pArea->filename : "(invalid)" ),
			pArea->low_vnum, pArea->hi_vnum,
			pArea->low_vnum, pArea->hi_vnum,
			pArea->low_vnum, pArea->hi_vnum, IS_SET( pArea->status, AREA_LOADED ) ? loadst : notloadst );
		count++;
	}
	pager_printf( ch, "Areas listed: %d  Loaded: %d\r\n", count, loaded );
	return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums( CHAR_DATA *ch, const char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int low, high;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	low = 0;
	high = 32766;
	if( arg1[0] != '\0' )
	{
		low = atoi( arg1 );
		if( arg2[0] != '\0' )
			high = atoi( arg2 );
	}
	show_vnums( ch, low, high, true, true, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA *ch, const char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int low, high;

	do_vnums( ch, argument );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	low = 0;
	high = 32766;

	if( arg1[0] != '\0' )
	{
		low = atoi( arg1 );
		if( arg2[0] != '\0' )
			high = atoi( arg2 );
	}

	show_vnums( ch, low, high, false, true, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones( CHAR_DATA *ch, const char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int low, high;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	low = 0;
	high = 32766;
	if( arg1[0] != '\0' )
	{
		low = atoi( arg1 );
		if( arg2[0] != '\0' )
			high = atoi( arg2 );
	}
	show_vnums( ch, low, high, true, false, "", " X" );
}


/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
	FILE *fp;
	char filename[MAX_INPUT_LENGTH];

	snprintf( filename, MAX_INPUT_LENGTH, "%ssysdata.dat", SYSTEM_DIR );

	if( ( fp = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "save_sysdata: FileOpen" );
	}
	else
	{
		fprintf( fp, "#SYSTEM\n" );
		fprintf( fp, "Highplayers    %d\n", sys.alltimemax );
		fprintf( fp, "Highplayertime %s~\n", sys.time_of_max );
		fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING );
		fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH );
		fprintf( fp, "Readallmail    %d\n", sys.read_all_mail );
		fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free );
		fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free );
		fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail );
		fprintf( fp, "Muse           %d\n", sys.muse_level );
		fprintf( fp, "Think          %d\n", sys.think_level );
		fprintf( fp, "Build          %d\n", sys.build_level );
		fprintf( fp, "Log            %d\n", sys.log_level );
		fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto );
		fprintf( fp, "Overridepriv   %d\n", sys.level_override_private );
		fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player );
		fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr );
		fprintf( fp, "Stunregular    %d\n", sys.stun_regular );
		fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr );
		fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob );
		fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr );
		fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob );
		fprintf( fp, "Forcepc        %d\n", sys.level_forcepc );
		fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer );
		fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor );
		fprintf( fp, "Maxignore	     %d\n", sys.maxign );
		fprintf( fp, "Hoursperday    %d\n", sys.hoursperday );
		fprintf( fp, "Daysperweek    %d\n", sys.daysperweek );
		fprintf( fp, "Dayspermonth   %d\n", sys.dayspermonth );
		fprintf( fp, "Monthsperyear  %d\n", sys.monthsperyear );
		fprintf( fp, "Daysperyear    %d\n", sys.daysperyear );
		fprintf( fp, "Saveflags      %d\n", sys.save_flags );
		fprintf( fp, "Savefreq       %d\n", sys.save_frequency );
		fprintf( fp, "Lotterynum     %d\n", sys.lotterynum );
		fprintf( fp, "Lotteryweek    %d\n", sys.lotteryweek );
		fprintf( fp, "Lotterytimer   %d\n", sys.lotterytimer );
		fprintf( fp, "Leafcount      %d\n", sys.leafcount );
		fprintf( fp, "Storagetimer   %d\n", sys.storagetimer );
		fprintf( fp, "Jackpot        %d\n", sys.jackpot );
		fprintf( fp, "LastWinner     %s~\n", sys.lastwinner );
		fprintf( fp, "Newbie_purge	%d\n", sys.newbie_purge );
		fprintf( fp, "Regular_purge	%d\n", sys.regular_purge );
		fprintf( fp, "Autopurge		%d\n", sys.CLEANPFILES );
		fprintf( fp, "End\n\n" );
		fprintf( fp, "#END\n" );
	}
	FileClose( fp );
	return;
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
	const char *word;
	bool fMatch;

	sys->time_of_max = NULL;
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
			KEY( "Autopurge", sys->CLEANPFILES, fread_number( fp ) );
			break;
		case 'B':
			KEY( "Build", sys->build_level, fread_number( fp ) );
			break;

		case 'D':
			KEY( "Damplrvsplr", sys->dam_plr_vs_plr, fread_number( fp ) );
			KEY( "Damplrvsmob", sys->dam_plr_vs_mob, fread_number( fp ) );
			KEY( "Dammobvsplr", sys->dam_mob_vs_plr, fread_number( fp ) );
			KEY( "Dammobvsmob", sys->dam_mob_vs_mob, fread_number( fp ) );
			KEY( "Daysperweek", sys->daysperweek, fread_number( fp ) );
			KEY( "Dayspermonth", sys->dayspermonth, fread_number( fp ) );
			KEY( "Daysperyear", sys->daysperyear, fread_number( fp ) );
			break;

		case 'E':
			if( !str_cmp( word, "End" ) )
			{
				if( !sys->time_of_max )
					sys->time_of_max = str_dup( "(not recorded)" );
				return;
			}
			break;

		case 'F':
			KEY( "Forcepc", sys->level_forcepc, fread_number( fp ) );
			break;

		case 'G':
			KEY( "Guildoverseer", sys->guild_overseer, fread_string_nohash( fp ) );
			KEY( "Guildadvisor", sys->guild_advisor, fread_string_nohash( fp ) );
			break;

		case 'H':
			KEY( "Highplayers", sys->alltimemax, fread_number( fp ) );
			KEY( "Highplayertime", sys->time_of_max, fread_string_nohash( fp ) );
			KEY( "Hoursperday", sys->hoursperday, fread_number( fp ) );
			break;

		case 'J':
			KEY( "Jackpot", sys->jackpot, fread_number( fp ) );
			break;

		case 'L':
			KEY( "LastWinner", sys->lastwinner, fread_string( fp ) );
			KEY( "Leafcount", sys->leafcount, fread_number( fp ) );
			KEY( "Lotterynum", sys->lotterynum, fread_number( fp ) );
			KEY( "Lotteryweek", sys->lotteryweek, fread_number( fp ) );
			KEY( "Lotterytimer", sys->lotterytimer, fread_number( fp ) );
			KEY( "Log", sys->log_level, fread_number( fp ) );
			break;

		case 'M':
			KEY( "Maxignore", sys->maxign, fread_number( fp ) );
			KEY( "Msetplayer", sys->level_mset_player, fread_number( fp ) );
			KEY( "Muse", sys->muse_level, fread_number( fp ) );
			KEY( "Monthsperyear", sys->monthsperyear, fread_number( fp ) );
			break;

		case 'N':
			KEY( "Nameresolving", sys->NO_NAME_RESOLVING, fread_number( fp ) );
			KEY( "Newbie_purge", sys->newbie_purge, fread_number( fp ) );
			break;

		case 'O':
			KEY( "Overridepriv", sys->level_override_private, fread_number( fp ) );
			break;

		case 'P':
			KEY( "Protoflag", sys->level_modify_proto, fread_number( fp ) );
			break;

		case 'R':
			KEY( "Readallmail", sys->read_all_mail, fread_number( fp ) );
			KEY( "Readmailfree", sys->read_mail_free, fread_number( fp ) );
			KEY( "Regular_purge", sys->regular_purge, fread_number( fp ) );
			break;

		case 'S':
			KEY( "Storagetimer", sys->storagetimer, fread_number( fp ) );
			KEY( "Stunplrvsplr", sys->stun_plr_vs_plr, fread_number( fp ) );
			KEY( "Stunregular", sys->stun_regular, fread_number( fp ) );
			KEY( "Saveflags", sys->save_flags, fread_number( fp ) );
			KEY( "Savefreq", sys->save_frequency, fread_number( fp ) );
			break;

		case 'T':
			KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
			KEY( "Think", sys->think_level, fread_number( fp ) );
			break;


		case 'W':
			KEY( "Waitforauth", sys->WAIT_FOR_AUTH, fread_number( fp ) );
			KEY( "Writemailfree", sys->write_mail_free, fread_number( fp ) );
			break;
		}


		if( !fMatch )
		{
			bug( "Fread_sysdata: no match: %s", word );
		}
	}
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
	char filename[MAX_INPUT_LENGTH];
	FILE *fp;
	bool found;

	found = false;
	snprintf( filename, sizeof(filename), "%ssysdata.dat", SYSTEM_DIR );

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
				bug( "Load_sysdata_file: # not found." );
				break;
			}

			word = fread_word( fp );
			if( !str_cmp( word, "SYSTEM" ) )
			{
				fread_sysdata( sys, fp );
				break;
			}
			else if( !str_cmp( word, "END" ) )
				break;
			else
			{
				bug( "Load_sysdata_file: bad section." );
				break;
			}
		}
		FileClose( fp );
		update_calendar( );
	}

	if( !sysdata.guild_overseer )
		sysdata.guild_overseer = str_dup( "" );
	if( !sysdata.guild_advisor )
		sysdata.guild_advisor = str_dup( "" );
	if( !sysdata.lastwinner )
		sysdata.lastwinner = str_dup( "No one" );
	return found;
}


void load_banlist( void )
{
	BAN_DATA *pban;
	FILE *fp;
	int number;
	char letter;

	if( !( fp = FileOpen( SYSTEM_DIR BAN_LIST, "r" ) ) )
		return;

	for( ;; )
	{
		if( feof( fp ) )
		{
			bug( "Load_banlist: no -1 found." );
			FileClose( fp );
			return;
		}
		number = fread_number( fp );
		if( number == -1 )
		{
			FileClose( fp );
			return;
		}
		CREATE( pban, BAN_DATA, 1 );
		pban->level = number;
		pban->name = fread_string_nohash( fp );
		if( ( letter = fread_letter( fp ) ) == '~' )
			pban->ban_time = fread_string_nohash( fp );
		else
		{
			ungetc( letter, fp );
			pban->ban_time = str_dup( "(unrecorded)" );
		}
		LINK( pban, first_ban, last_ban, next, prev );
	}
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */
CMDF( do_check_vnums )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	bool room, mob, obj, all, area_conflict;
	int low_range, high_range;

	room = false;
	mob = false;
	obj = false;
	all = false;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "Please specify room, mob, object, or all as your first argument.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "room" ) )
		room = true;

	else if( !str_cmp( arg1, "mob" ) )
		mob = true;

	else if( !str_cmp( arg1, "object" ) )
		obj = true;

	else if( !str_cmp( arg1, "all" ) )
		all = true;
	else
	{
		send_to_char( "Please specify room, mob, or object as your first argument.\r\n", ch );
		return;
	}

	if( arg2[0] == '\0' )
	{
		send_to_char( "Please specify the low end of the range to be searched.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Please specify the high end of the range to be searched.\r\n", ch );
		return;
	}

	low_range = atoi( arg2 );
	high_range = atoi( argument );

	if( low_range < 1 || low_range > MAX_VNUMS )
	{
		send_to_char( "Invalid argument for bottom of range.\r\n", ch );
		return;
	}

	if( high_range < 1 || high_range > MAX_VNUMS )
	{
		send_to_char( "Invalid argument for top of range.\r\n", ch );
		return;
	}

	if( high_range < low_range )
	{
		send_to_char( "Bottom of range must be below top of range.\r\n", ch );
		return;
	}

	if( all )
	{
		snprintf( buf, MAX_STRING_LENGTH, "room %d %d", low_range, high_range );
		do_check_vnums( ch, buf );
		snprintf( buf, MAX_STRING_LENGTH, "mob %d %d", low_range, high_range );
		do_check_vnums( ch, buf );
		snprintf( buf, MAX_STRING_LENGTH, "object %d %d", low_range, high_range );
		do_check_vnums( ch, buf );
		return;
	}
	set_char_color( AT_PLAIN, ch );

	for( pArea = first_asort; pArea; pArea = pArea->next_sort )
	{
		area_conflict = false;
		if( IS_SET( pArea->status, AREA_DELETED ) )
			continue;
		else if( room )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;

			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( mob )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;
			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( obj )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;

			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( area_conflict )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
			if( room )
				snprintf( buf2, MAX_STRING_LENGTH, "Rooms: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );
			if( mob )
				snprintf( buf2, MAX_STRING_LENGTH, "Mobs: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );
			if( obj )
				snprintf( buf2, MAX_STRING_LENGTH, "Objects: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );

			mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
			send_to_char( buf, ch );
		}
	}
	for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
	{
		area_conflict = false;
		if( IS_SET( pArea->status, AREA_DELETED ) )
			continue;
		else if( room )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;

			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( mob )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;
			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( obj )
		{
			if( low_range < pArea->low_vnum && pArea->low_vnum < high_range )
				area_conflict = true;

			if( low_range < pArea->hi_vnum && pArea->hi_vnum < high_range )
				area_conflict = true;

			if( ( low_range >= pArea->low_vnum ) && ( low_range <= pArea->hi_vnum ) )
				area_conflict = true;

			if( ( high_range <= pArea->hi_vnum ) && ( high_range >= pArea->low_vnum ) )
				area_conflict = true;
		}

		if( area_conflict )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
			if( room )
				snprintf( buf2, MAX_STRING_LENGTH, "Rooms: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );
			if( mob )
				snprintf( buf2, MAX_STRING_LENGTH, "Mobs: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );
			if( obj )
				snprintf( buf2, MAX_STRING_LENGTH, "Objects: %5d - %-5d\r\n", pArea->low_vnum, pArea->hi_vnum );

			mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
			send_to_char( buf, ch );
		}
	}

	/*
		for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
		{
			area_conflict = false;
		if ( IS_SET( pArea->status, AREA_DELETED ) )
		   continue;
		else
		if (room)
		  if((pArea->low_r_vnum >= low_range)
		  && (pArea->hi_r_vnum <= high_range))
			area_conflict = true;

		if (mob)
		  if((pArea->low_m_vnum >= low_range)
		  && (pArea->hi_m_vnum <= high_range))
			area_conflict = true;

		if (obj)
		  if((pArea->low_o_vnum >= low_range)
		  && (pArea->hi_o_vnum <= high_range))
			area_conflict = true;

		if (area_conflict)
		  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
				 " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
			(pArea->filename ? pArea->filename : "(invalid)"),
			pArea->low_r_vnum, pArea->hi_r_vnum,
			pArea->low_o_vnum, pArea->hi_o_vnum,
			pArea->low_m_vnum, pArea->hi_m_vnum );
		}

		for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
		{
			area_conflict = false;
		if ( IS_SET( pArea->status, AREA_DELETED ) )
		   continue;
		else
		if (room)
		  if((pArea->low_r_vnum >= low_range)
		  && (pArea->hi_r_vnum <= high_range))
			area_conflict = true;

		if (mob)
		  if((pArea->low_m_vnum >= low_range)
		  && (pArea->hi_m_vnum <= high_range))
			area_conflict = true;

		if (obj)
		  if((pArea->low_o_vnum >= low_range)
		  && (pArea->hi_o_vnum <= high_range))
			area_conflict = true;

		if (area_conflict)
		  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
				 " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
			(pArea->filename ? pArea->filename : "(invalid)"),
			pArea->low_r_vnum, pArea->hi_r_vnum,
			pArea->low_o_vnum, pArea->hi_o_vnum,
			pArea->low_m_vnum, pArea->hi_m_vnum );
		}
	*/
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
	return;
}

/*
 * Read a string of text based flags from file fp. Ending in ~
 */
char *fread_flagstring( FILE *fp )
{
	static char buf[MAX_STRING_LENGTH];
	char *plast;
	char c;
	int ln;

	plast = buf;
	buf[0] = '\0';
	ln = 0;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if( feof( fp ) )
		{
			bug( "%s: EOF encountered on read.", __func__ );
			if( fBootDb )
			{
				shutdown_mud( "Corrupt file somewhere." );
				exit( 1 );
			}
			snprintf( buf, MAX_STRING_LENGTH, "%s", "" );
			return buf;
		}
		c = getc( fp );
	} while( isspace( c ) );

	if( ( *plast++ = c ) == '~' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s", "" );
		return buf;
	}

	for( ;; )
	{
		if( ln >= ( MAX_STRING_LENGTH - 1 ) )
		{
			bug( "%s: string too long", __func__ );
			*plast = '\0';
			return ( buf );
		}
		switch( *plast = getc( fp ) )
		{
		default:
			plast++;
			ln++;
			break;

		case EOF:
			bug( "%s: EOF", __func__ );
			if( fBootDb )
				exit( 1 );
			*plast = '\0';
			return ( buf );

		case '\n':
			++plast;
			++ln;
			*plast++ = '\r';
			++ln;
			break;

		case '\r':
			break;

		case '~':
			*plast = '\0';
			return ( buf );
		}
	}
}

// The following 2 functions are taken from FreeBSD under the following license terms:

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 /*
  * Copy string src to buffer dst of size dsize.  At most dsize-1
  * chars will be copied.  Always NUL terminates (unless dsize == 0).
  * Returns strlen(src); if retval >= dsize, truncation occurred.
  *
  * Renamed so it can play itself system independent.
  * Samson 10-12-03
  */
size_t mudstrlcpy( char *__restrict dst, const char *__restrict src, size_t dsize )
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if( nleft != 0 )
	{
		while( --nleft != 0 )
		{
			if( ( *dst++ = *src++ ) == '\0' )
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if( nleft == 0 )
	{
		if( dsize != 0 )
			*dst = '\0'; /* NUL-terminate dst */
		while( *src++ )
			;
	}

	return( src - osrc - 1 ); /* count does not include NUL */
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 *
 * Renamed so it can play itself system independent.
 * Samson 10-12-03
 */
size_t mudstrlcat( char *__restrict dst, const char *__restrict src, size_t dsize )
{
	const char *odst = dst;
	const char *osrc = src;
	size_t n = dsize;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while( n-- != 0 && *dst != '\0' )
		dst++;

	dlen = dst - odst;
	n = dsize - dlen;

	if( n-- == 0 )
		return( dlen + strlen( src ) );

	while( *src != '\0' )
	{
		if( n != 0 )
		{
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = '\0';

	return( dlen + ( src - osrc ) ); /* count does not include NUL */
}