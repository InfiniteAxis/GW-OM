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

#include <stdlib.h>
#include <limits.h>
#include <typeinfo> 
#include <sys/cdefs.h>
#include <sys/time.h>
#include <stdarg.h>
#include <math.h>

#ifdef __cplusplus
//#include <typeinfo>
#include <list>
//#include <vector>
//#include <map>
//#include <string>
#include <bitset>
//#include <algorithm>
#endif

#ifndef __cplusplus
typedef unsigned char bool;
#endif

typedef int ch_ret;
typedef int obj_ret;

#define List    std::list
#define FlagSet std::bitset

#define args( list )			list

#ifdef __cplusplus
#define DECLARE_DO_FUN( fun )    extern "C" { DO_FUN    fun; } DO_FUN fun##_mangled
#define DECLARE_SPEC_FUN( fun )  extern "C" { SPEC_FUN  fun; } SPEC_FUN fun##_mangled
#define DECLARE_SPELL_FUN( fun ) extern "C" { SPELL_FUN fun; } SPELL_FUN fun##_mangled
#else
#define DECLARE_DO_FUN( fun )     DO_FUN    fun; DO_FUN fun##_mangled
#define DECLARE_SPEC_FUN( fun )   SPEC_FUN  fun; SPEC_FUN fun##_mangled
#define DECLARE_SPELL_FUN( fun )  SPELL_FUN fun; SPELL_FUN fun##_mangled
#endif

/*
* Short scalar types.
* Diavolo reports AIX compiler has bugs with short types.
*/
#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(BERR)
#define BERR 255
#endif

#define KEY( literal, field, value )   \
   if ( !str_cmp( word, (literal) ) )     \
{                                      \
   (field) = (value);                  \
   fMatch = true;                      \
   break;                              \
}

#define CMDF( name ) extern "C" void (name)( CHAR_DATA *ch, const char *argument )
#define SPELLF( name ) extern "C" ch_ret (name)( int sn, int level, CHAR_DATA *ch, void *vo )
#define SPECF( name ) extern "C" bool (name)( CHAR_DATA *ch )

/*
 * Structure types.
 */
typedef struct affect_data AFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct ban_data BAN_DATA;
typedef struct extracted_char_data EXTRACT_CHAR_DATA;
typedef struct char_data CHAR_DATA;
typedef struct hunt_hate_fear HHF_DATA;
typedef struct fighting_data FIGHT_DATA;
typedef struct suitfighting_data SUITFIGHT_DATA;
typedef struct attacking_data ATTACK_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct menu_data MENU_DATA;
typedef struct mem_data MEM_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct note_data NOTE_DATA;
typedef struct comment_data COMMENT_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct reset_data RESET_DATA;
typedef struct map_index_data MAP_INDEX_DATA;   /* maps */
typedef struct map_data MAP_DATA;   /* maps */
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct repairshop_data REPAIR_DATA;
typedef struct reserve_data RESERVE_DATA;
typedef struct mname_data MNAME_DATA;
typedef struct fname_data FNAME_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct hour_min_sec HOUR_MIN_SEC;
typedef struct weather_data WEATHER_DATA;
typedef struct auction_data AUCTION_DATA;
typedef struct war_data WAR_DATA;
typedef struct tag_data TAG_DATA;
typedef struct bounty_data BOUNTY_DATA;
typedef struct planet_data PLANET_DATA;
typedef struct guard_data GUARD_DATA;
typedef struct space_data SPACE_DATA;
typedef struct clan_data CLAN_DATA;
typedef struct senate_data SENATE_DATA;
typedef struct ship_data SHIP_DATA;
typedef struct missile_data MISSILE_DATA;
typedef struct weapon_data WEAPON_DATA;
typedef struct tourney_data TOURNEY_DATA;
typedef struct mob_prog_data MPROG_DATA;
typedef struct mob_prog_act_list MPROG_ACT_LIST;
typedef struct editor_data EDITOR_DATA;
typedef struct teleport_data TELEPORT_DATA;
typedef struct timer_data TIMER;
typedef struct godlist_data GOD_DATA;
typedef struct system_data SYSTEM_DATA;
typedef struct smaug_affect SMAUG_AFF;
typedef struct who_data WHO_DATA;
typedef struct skill_type SKILLTYPE;
typedef struct weapon_type WEAPONTYPE;
typedef struct art_type ARTTYPE;
typedef struct social_type SOCIALTYPE;
typedef struct cmd_type CMDTYPE;
typedef struct killed_data KILLED_DATA;
typedef struct race_type RACE_TYPE;
typedef struct immortal_host IMMORTAL_HOST;
typedef struct wizent WIZENT;
typedef struct member_data MEMBER_DATA; /* Individual member data */
typedef struct ignore_data IGNORE_DATA;
typedef struct mpsleep_data MPSLEEP_DATA;   /* mpsleep snippet - Samson 6-1-99 */
typedef struct buf_type BUFFER;
typedef struct locker_data LOCKER_DATA;
typedef struct specfun_list SPEC_LIST;
typedef struct extended_bitvector EXT_BV;

struct File;

/*
* Function types.
*/
typedef void DO_FUN( CHAR_DATA *ch, const char *argument );
typedef bool SPEC_FUN( CHAR_DATA *ch );
typedef ch_ret SPELL_FUN( int sn, int level, CHAR_DATA *ch, void *vo );



struct buf_type
{
	BUFFER *next;
	bool valid;
	short state;    /* error state of the buffer */
	short size; /* size in k */
	const char *string;    /* buffer's string */
};


/* Magic number for memory allocation */
#define MAGIC_NUM 52571214

#define MAX_BUF	65536
#define BASE_BUF 	1024

/* valid states */
#define BUFFER_SAFE	     0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED 	2

/* note recycling */
NOTE_DATA *new_note( void );

#define DUR_CONV	   23.333333333333333333333333
#define HIDDEN_TILDE   '*'

#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * String and memory management parameters.
 */
const int MAX_KEY_HASH      = 2048;
const int MAX_STRING_LENGTH = 4096;  /* buf */
const int MAX_INPUT_LENGTH  = 1024;  /* arg */
const int MAX_LINE_LENGTH   = 252;
const int MAX_INBUF_SIZE    = 1024;
const int MSL               = MAX_STRING_LENGTH;
const int MIL               = MAX_INPUT_LENGTH;
const int MFL               = 256;

#define HASHSTR            /* use string hashing */

#define MAX_LAYERS       8  /* maximum clothing layers */
#define MAX_NEST         100  /* maximum container nesting */
#define LAST_FILE_SIZE   500

#define MAX_KILLTRACK    19  /* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH     500000
#define MIN_EXP_WORTH     500

#define MAX_REXITS		 20   /* Maximum exits allowed in 1 room */
#define MAX_SKILL		 500
#define MAX_SWEAPON       15   /* Special weapons for busters */
#define MAX_RACE		 17
#define MAX_SUITWEAPON    50
#define MAX_NPC_RACE	 91
#define MAX_ELEMENT       6
#define MAX_LEVEL 		 1007
#define MAX_CLAN		 50
#define MAX_PLANET		 100
#define MAX_SHIP          1000
#define MAX_BOUNTY        255
#define MAX_GOV           255
#define MAX_VNUMS         2000000000

#define MAX_HERB		 20
#define MAX_FORGET        10

#define LEVEL_HERO		 (MAX_LEVEL - 7)
#define LEVEL_AVATAR	 LEVEL_HERO
#define LEVEL_STAFF       (MAX_LEVEL - 6)
#define LEVEL_LIAISON     (MAX_LEVEL - 5)
#define LEVEL_ENFORCER    (MAX_LEVEL - 4)
#define LEVEL_BUILDER     (MAX_LEVEL - 3)
#define LEVEL_SHIPWRIGHT  (MAX_LEVEL - 2)
#define LEVEL_ADMIN       (MAX_LEVEL - 1)
#define LEVEL_OWNER       MAX_LEVEL

#include "dns.h"
#include "pfiles.h"
#include "hotboot.h"
#include "color.h"
#include "board.h"

#define LEVEL_LOG		    LEVEL_LIAISON
#define LEVEL_HIGOD		    LEVEL_ENFORCER

#define PULSE_PER_HALF_SECOND	 2
#define PULSE_PER_SECOND		 4
#define PULSE_PER_TENSECS	 39
#define PULSE_MINUTE		 236
#define PULSE_HOUR			 14164
#define PULSE_VIOLENCE		 ( 3 * PULSE_PER_SECOND )
#define PULSE_MOBILE		 ( 4 * PULSE_PER_SECOND )
#define PULSE_TICK			 275
#define PULSE_AREA			 PULSE_MINUTE
#define PULSE_AUCTION		 PULSE_PER_TENSECS
#define PULSE_SPACE			 PULSE_PER_TENSECS
#define PULSE_MAILCHECK        ( 60 *  PULSE_PER_SECOND )
#define PULSE_TAXES			 PULSE_HOUR
#define PULSE_WAR			 ( 3 * PULSE_PER_TENSECS )
#define PULSE_RELATIONSHIP	 ( 8 * PULSE_MINUTE )
#define PULSE_MUSIC			 ( 6 * PULSE_PER_SECOND )
#define PULSE_PREVTIMER		 PULSE_MINUTE
#define PULSE_HEAL			 ( 3 * PULSE_PER_TENSECS )
#define PULSE_HOURPROG		 PULSE_HOUR
#define PULSE_CHARGE		 ( 4 * PULSE_PER_SECOND )
#define PULSE_BOUNTY           ( 60 * PULSE_MINUTE )

/*
 * Stuff for area versions --Shaddai
 */
#define HAS_SPELL_INDEX     -1

 /*
 Old Smaug area version identifiers:

 Version 1: Stock 1.4a areas.
 Version 2: Skipped - Probably won't ever see these, but originated from Smaug 1.8.
 Version 3: Stock 1.8 areas.
 */

 // This value has been reset due to the new KEY/Value based area format.
 // It will not conflict with the above former area file versions.
#define AREA_VERSION_WRITE 2

/*
 * Command logging types.
 */
typedef enum
{
	LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, LOG_ALL
} log_types;

/* short cut crash bug fix provided by gfinello@mail.karmanet.it*/
typedef enum
{
	relMSET_ON, relOSET_ON
} relation_type;

typedef struct rel_data REL_DATA;

struct rel_data
{
	void *Actor;
	void *Subject;
	REL_DATA *next;
	REL_DATA *prev;
	relation_type Type;
};

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
	rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
	rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
	rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
	rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
	rERROR, rSTOP = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* defines for new do_who */
#define WT_MORTAL 0
#define WT_IMM    2
#define WT_AVATAR 1
#define WT_NEWBIE 3

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM       31 /* extended bitmask   ( INTBITS - 1 )  */
#define RSV       5  /* right-shift value  ( sqrt(XBM+1) )  */
#define XBI       4  /* integers in an extended bitvector   */
#define MAX_BITS  (XBI * INTBITS)

 /*
  * Structure for extended bitvectors -- Thoric
  */
struct extended_bitvector
{
	unsigned int bits[XBI];
};


/*
 * do_who output structure -- Narn
 */
struct who_data
{
	WHO_DATA *prev;
	WHO_DATA *next;
	const char *text;
	int type;
};

/*
 * Site ban structure.
 */
struct ban_data
{
	BAN_DATA *next;
	BAN_DATA *prev;
	const char *name;
	int level;
	const char *ban_time;
};

/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct reserve_data
{
	RESERVE_DATA *next;
	RESERVE_DATA *prev;
	const char *name;
};

/*
 * Random Mob Name structure
 */
struct mname_data
{
	MNAME_DATA *next;
	MNAME_DATA *prev;
	const char *name;
};

struct fname_data
{
	FNAME_DATA *next;
	FNAME_DATA *prev;
	const char *name;
};

FILE *__FileOpen( const char *filename, const char *mode, const char *file, const char *function, int line );
void FileClose( FILE *fp );
#define FileOpen( filename, mode)  __FileOpen(filename, mode, __FILE__, __FUNCTION__, __LINE__)

extern File *first_filedata;
extern File *last_filedata;
extern int FilesOpen;
struct File
{
	File *next;
	File *prev;
	FILE *fp;
	char *filename;
	char *mode;

	// *Where they were called from* //
	char *file;
	char *function;
	int line;
};

/*
 * Time and weather stuff.
 */
typedef enum
{
	SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
	SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

/* PaB: Seasons */
/* Notes: Each season will be arbitrarily set at 1/4 of the year.
 */
#define SEASON_SPRING		0
#define SEASON_SUMMER		1
#define SEASON_FALL		2
#define SEASON_WINTER		3
#define SEASON_MAX         	4

extern const char *const season_name[];
extern const char *const day_name[];

struct time_info_data
{
	int hour;
	int day;
	int month;
	int year;
	int season;
};

struct hour_min_sec
{
	int hour;
	int min;
	int sec;
	int manual;
};

struct weather_data
{
	int mmhg;
	int change;
	int sky;
	int sunlight;
	int precip;
};


/*
 * Structure used to build wizlist
 */
struct wizent
{
	WIZENT *next;
	WIZENT *last;
	const char *name;
	short level;
	const char *aim;
	const char *job;
};

/*
 * Structure to only allow immortals domains to access their chars.
 */
struct immortal_host
{
	IMMORTAL_HOST *next;
	IMMORTAL_HOST *prev;
	const char *name;
	const char *host;
	bool prefix;
	bool suffix;
};

/*
 * Connected state for a channel.
 */
typedef enum
{
	CON_GET_NAME = -99,
	CON_GET_OLD_PASSWORD, CON_CONFIRM_NEW_NAME,
	CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD, CON_GET_NEW_SEX,
	CON_READ_MOTD, CON_GET_NEW_RACE, CON_GET_EMULATION,
	CON_GET_WANT_RIPANSI, CON_TITLE, CON_PRESS_ENTER, CON_WAIT_1,
	CON_WAIT_2, CON_WAIT_3, CON_ACCEPTED, CON_GET_PKILL, CON_READ_IMOTD,
	CON_GET_NEW_EMAIL, CON_GET_MSP, CON_GET_NEW_CLASS, CON_ROLL_STATS,
	CON_STATS_OK, CON_GET_HAIR, CON_GET_EYE, CON_GET_BUILD, CON_GET_HERO, 
	CON_GET_HIGHLIGHT, CON_NOTE_TO, CON_NOTE_SUBJECT, CON_NOTE_EXPIRE, 
	CON_NOTE_TEXT, CON_NOTE_FINISH, CON_WANT_INVIS, CON_AGREEMENT, 

	CON_COPYOVER_RECOVER, CON_PLAYING = 0,

	CON_EDITING
} connection_types;

/*
 * Character substates
 */
typedef enum
{
	SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
	SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
	SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
	SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
	SUB_DEITYDESC, SUB_WRITE_MAIL,
	/*
	 * timer types ONLY below this point
	 */
	 SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
	DESCRIPTOR_DATA *next;
	DESCRIPTOR_DATA *prev;
	DESCRIPTOR_DATA *snoop_by;
	CHAR_DATA *character;
	CHAR_DATA *original;
	struct mccp_data *mccp;
	unsigned long outsize;
	unsigned long pagesize;
	unsigned char prevcolor;
	char inbuf[MAX_INBUF_SIZE];
	char incomm[MAX_INPUT_LENGTH];
	char inlast[MAX_INPUT_LENGTH];
	char abuf[256];
	char pagecmd;
	char pagecolor;
	const char *host;
	char *outbuf;
	char *pagebuf;
	const char *pagepoint;
	const char *user;
	bool fcommand;
	bool can_compress;
	bool ansi;
	int idle;
	int port;
	int descriptor;
	int repeat;
	int outtop;
	int pagetop;
	int atimes;
	int newstate;
	short connected;
	short lines;
	short scrlen;
	//Resolver
	int ifd;
	pid_t ipid;
};



/*
 * Attribute bonus structures.
 */
struct str_app_type
{
	short tohit;
	short todam;
	short carry;
	short wield;
};

struct int_app_type
{
	short learn;
};

struct wis_app_type
{
	short practice;
};

struct dex_app_type
{
	short defensive;
};

struct con_app_type
{
	short hitp;
	short shock;
};

struct cha_app_type
{
	short charm;
};

struct lck_app_type
{
	short luck;
};

/* ability classes */
enum Classes
{
	ABILITY_NONE = -1, COMBAT_ABILITY, PILOTING_ABILITY, ENGINEERING_ABILITY,
	HUNTING_ABILITY, SMUGGLING_ABILITY, DIPLOMACY_ABILITY, LEADERSHIP_ABILITY, 
	ESPIONAGE_ABILITY,

	MAX_ABILITY
};

/* the races */
#define RACE_COLONIST	      0
#define RACE_AMERICAN		 1
#define RACE_CANADIAN		 2
#define RACE_JAPANESE		 3
#define RACE_KOREAN		      4
#define RACE_MEXICAN	      5
#define RACE_AFRICAN		 6
#define RACE_AUSTRALIAN		 7
#define RACE_RUSSIAN	      8
#define RACE_BRITISH           9
#define RACE_CHINESE          10
#define RACE_SWEDISH          11
#define RACE_GERMAN           12
#define RACE_BRAZILLIAN       13
#define RACE_IRISH            14
#define RACE_SCOTTISH         16   /* big mistake was causing mass chaos */
#define RACE_ARABIC           15

/*
 * TO types for act.
 */
#define TO_ROOM	    0
#define TO_NOTVICT	    1
#define TO_VICT	    2
#define TO_CHAR	    3
#define TO_MUD          4


#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

 /*
  * Help table types.
  */
struct help_data
{
	HELP_DATA *next;
	HELP_DATA *prev;
	short level;
	const char *keyword;
	const char *text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct shop_data
{
	SHOP_DATA *next; /* Next shop in list        */
	SHOP_DATA *prev; /* Previous shop in list    */
	int keeper;  /* Vnum of shop keeper mob  */
	short buy_type[MAX_TRADE];  /* Item types shop will buy */
	short profit_buy;   /* Cost multiplier for buying   */
	short profit_sell;  /* Cost multiplier for selling  */
	short open_hour;    /* First opening hour       */
	short close_hour;   /* First closing hour       */
};

#define MAX_FIX		3
#define SHOP_FIX	     1
#define SHOP_RECHARGE	2

struct repairshop_data
{
	REPAIR_DATA *next;   /* Next shop in list        */
	REPAIR_DATA *prev;   /* Previous shop in list    */
	int keeper;  /* Vnum of shop keeper mob  */
	short fix_type[MAX_FIX];    /* Item types shop will fix */
	short profit_fix;   /* Cost multiplier for fixing   */
	short shop_type;    /* Repair shop type     */
	short open_hour;    /* First opening hour       */
	short close_hour;   /* First closing hour       */
};

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */
#define MAX_IFS     20  /* should always be generous */
#define IN_IF        0
#define IN_ELSE      1
#define DO_IF        2
#define DO_ELSE      3

#define MAX_PROG_NEST 20

   /* Mob program structures */
struct act_prog_data
{
	struct act_prog_data *next;
	void *vo;
};

struct mob_prog_act_list
{
	MPROG_ACT_LIST *next;
	const char *buf;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	void *vo;
};

struct mob_prog_data
{
	MPROG_DATA *next;
	int type;
	bool triggered;
	int resetdelay;
	const char *arglist;
	const char *comlist;
	bool fileprog;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum
{
	MP_MOB, MP_ROOM, MP_OBJ
} mp_types;

struct mpsleep_data
{
	MPSLEEP_DATA *next;
	MPSLEEP_DATA *prev;

	ROOM_INDEX_DATA *room;  /* Room when type is MP_ROOM */
	int timer;  /* Pulses to sleep */
	mp_types type; /* Mob, Room or Obj prog */

	/*
	 * mprog_driver state variables
	 */
	int ignorelevel;
	int iflevel;
	bool ifstate[MAX_IFS][DO_ELSE + 1];

	/*
	 * mprog_driver arguments
	 */
	const char *com_list;
	CHAR_DATA *mob;
	CHAR_DATA *actor;
	OBJ_DATA *obj;
	void *vo;
	bool single_step;
};

extern bool MOBtrigger;
extern bool MPSilent;

/* race dedicated stuff */
struct race_type
{
	char race_name[16];  /* Race name */
	short str_plus;  /* Str bonus/penalty */
	short dex_plus;  /* Dex */
	short wis_plus;  /* Wis */
	short int_plus;  /* Int */
	short con_plus;  /* Con */
	short cha_plus;  /* Cha */
};

struct suit_type
{
	char weapon_name[50];
	short weapon_type;
	short damage;
	short energy_cost;
	short ammo_max;
	short ammo_type;
	short suit_class;
	short weight;
	int price;
};

typedef enum
{
	CLAN_PLAIN, CLAN_CRIME, CLAN_GUILD, CLAN_SUBCLAN
} clan_types;

typedef enum
{
	SHIP_CIVILIAN, SHIP_REPUBLIC, SHIP_IMPERIAL, MOB_SHIP
} ship_types;

typedef enum
{
	SHIP_DOCKED, SHIP_READY, SHIP_BUSY, SHIP_BUSY_2, SHIP_BUSY_3, SHIP_REFUEL,
	SHIP_LAUNCH, SHIP_LAUNCH_2, SHIP_LAND, SHIP_LAND_2, SHIP_HYPERSPACE, SHIP_DISABLED, SHIP_FLYING
} ship_states;

typedef enum
{
	MISSILE_READY, MISSILE_FIRED, MISSILE_RELOAD, MISSILE_RELOAD_2, MISSILE_DAMAGED
} missile_states;

typedef enum
{
	MOBILE_SUIT, TRANSPORT_SHIP, CAPITAL_SHIP, SHIP_PLATFORM, CLOUD_CAR, OCEAN_SHIP, LAND_SPEEDER, WHEELED, LAND_CRAWLER,
	WALKER, SMINE
} ship_classes;

typedef enum
{
	CONCUSSION_MISSILE, PROTON_TORPEDO, HEAVY_ROCKET, HEAVY_BOMB
} missile_types;

typedef enum
{
	WEAPON_PHYSICAL, WEAPON_MISSILE, WEAPON_BEAM, WEAPON_BULLET, WEAPON_SABER
} weapon_types;

typedef enum
{
	GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD
} group_types;


#define LASER_DAMAGED    -1
#define LASER_READY       0

struct space_data
{
	SPACE_DATA *next;
	SPACE_DATA *prev;
	SHIP_DATA *first_ship;
	SHIP_DATA *last_ship;
	MISSILE_DATA *first_missile;
	MISSILE_DATA *last_missile;
	PLANET_DATA *first_planet;
	PLANET_DATA *last_planet;
	const char *filename;
	const char *name;
	const char *star1;
	const char *star2;
	const char *planet1;
	const char *planet2;
	const char *planet3;
	const char *planet4;
	const char *planet5;
	const char *planet6;
	const char *planet7;
	const char *planet8;
	const char *planet9;
	const char *location1a;
	const char *location2a;
	const char *location3a;
	const char *location4a;
	const char *location5a;
	const char *location6a;
	const char *location7a;
	const char *location8a;
	const char *location9a;
	const char *location1b;
	const char *location2b;
	const char *location3b;
	const char *location4b;
	const char *location5b;
	const char *location6b;
	const char *location7b;
	const char *location8b;
	const char *location9b;
	const char *location1c;
	const char *location2c;
	const char *location3c;
	const char *location4c;
	const char *location5c;
	const char *location6c;
	const char *location7c;
	const char *location8c;
	const char *location9c;
	int xpos;
	int ypos;
	int s1x;
	int s1y;
	int s1z;
	int s2x;
	int s2y;
	int s2z;
	int doc1a;
	int doc2a;
	int doc3a;
	int doc4a;
	int doc5a;
	int doc6a;
	int doc7a;
	int doc8a;
	int doc9a;
	int doc1b;
	int doc2b;
	int doc3b;
	int doc4b;
	int doc5b;
	int doc6b;
	int doc7b;
	int doc8b;
	int doc9b;
	int doc1c;
	int doc2c;
	int doc3c;
	int doc4c;
	int doc5c;
	int doc6c;
	int doc7c;
	int doc8c;
	int doc9c;
	int p1x;
	int p1y;
	int p1z;
	int p2x;
	int p2y;
	int p2z;
	int p3x;
	int p3y;
	int p3z;
	int p4x;
	int p4y;
	int p4z;
	int p5x;
	int p5y;
	int p5z;
	int p6x;
	int p6y;
	int p6z;
	int p7x;
	int p7y;
	int p7z;
	int p8x;
	int p8y;
	int p8z;
	int p9x;
	int p9y;
	int p9z;
	int gravitys1;
	int gravitys2;
	int gravityp1;
	int gravityp2;
	int gravityp3;
	int p1_low;
	int p1_high;
	int p2_low;
	int p2_high;
	int p3_low;
	int p3_high;
	int crash;
};

struct bounty_data
{
	BOUNTY_DATA *next;
	BOUNTY_DATA *prev;
	const char *target;
	long int amount;
	const char *poster;
};

struct guard_data
{
	GUARD_DATA *next;
	GUARD_DATA *prev;
	GUARD_DATA *next_on_planet;
	GUARD_DATA *prev_on_planet;
	CHAR_DATA *mob;
	ROOM_INDEX_DATA *reset_loc;
	PLANET_DATA *planet;
};

struct senate_data
{
	SENATE_DATA *next;
	SENATE_DATA *prev;
	const char *name;
};

struct planet_data
{
	PLANET_DATA *next;
	PLANET_DATA *prev;
	PLANET_DATA *next_in_system;
	PLANET_DATA *prev_in_system;
	GUARD_DATA *first_guard;
	GUARD_DATA *last_guard;
	SPACE_DATA *starsystem;
	AREA_DATA *first_area;
	AREA_DATA *last_area;
	const char *name;
	const char *filename;
	long base_value;
	CLAN_DATA *governed_by;
	int population;
	bool flags;
	float pop_support;
	int pImport[10];
	int pExport[10];
	int pBuy[10];
	int pSell[10];
};

#define PLANET_NOCAPTURE  BV00
#define PLANET_NOQUEST    BV01


/* cargo types */

#define CTYPE_NONE              0
#define CTYPE_ALL               1
#define CTYPE_CDI               2
#define CTYPE_BATTERIES         3
#define CTYPE_IRON              4
#define CTYPE_STEEL             5
#define CTYPE_TITANIUM          6
#define CTYPE_NEOTITANIUM       7
#define CTYPE_GUNDANIUM         8
#define CTYPE_MAX               9


struct clan_data
{
	CLAN_DATA *next; /* next clan in list            */
	CLAN_DATA *prev; /* previous clan in list        */
	CLAN_DATA *next_subclan;
	CLAN_DATA *prev_subclan;
	CLAN_DATA *first_subclan;
	CLAN_DATA *last_subclan;
	CLAN_DATA *mainclan;
	const char *filename;  /* Clan filename            */
	const char *name;  /* Clan name                */
	const char *description;   /* A brief description of the clan  */
	const char *leader;    /* Head clan leader         */
	const char *cone;  /* Colour One                           */
	const char *ctwo;  /* Colour Two                           */
	const char *motto; /* Clan Motto                           */
	const char *clandesc;  /* Clan Description                     */
	const char *ally;  /* Clan allies                          */
	const char *atwar; /* Clan At War With                     */
	const char *shortname; /* Clan Short Name                      */
	const char *number1;   /* First officer            */
	const char *number2;   /* Second officer           */
	int thug;    /* Amount of Thugs                      */
	int hitmen;  /* Amount of Hitmen                     */
	int drugaddict;  /* Amount of Drug Addicts               */
	int pimp;    /* Amount of Pimps                      */
	int scalper; /* Amount of Scalpers                   */
	int pkills;  /* Number of pkills on behalf of clan   */
	int pdeaths; /* Number of pkills against clan    */
	int mkills;  /* Number of mkills on behalf of clan   */
	int mdeaths; /* Number of clan deaths due to mobs    */
	short clan_type;    /* See clan type defines        */
	short members;  /* Number of clan members       */
	int board;   /* Vnum of clan board           */
	int storeroom;   /* Vnum of clan's store room        */
	int guard1;  /* Vnum of clan guard type 1        */
	int guard2;  /* Vnum of clan guard type 2        */
	int patrol1; /* vnum of patrol */
	int patrol2; /* vnum of patrol */
	int trooper1;    /* vnum of reinforcements */
	int trooper2;    /* vnum of elite troopers */
	long int funds;
	int spacecraft;
	int vehicles;
	int jail;
	bool gbeat;
	bool pbeat;
	const char *tmpstr;
	int bases;
	int income;
	char *chan_back[10]; /* For gangtalk chanback */
};

struct ship_data
{
	SHIP_DATA *next;
	SHIP_DATA *prev;
	SHIP_DATA *next_in_starsystem;
	SHIP_DATA *prev_in_starsystem;
	SHIP_DATA *next_in_room;
	SHIP_DATA *prev_in_room;
	ROOM_INDEX_DATA *in_room;
	SPACE_DATA *starsystem;
	SUITFIGHT_DATA *fighting;
	const char *filename;
	const char *name;
	const char *home;
	const char *description;
	const char *owner;
	const char *pilot;
	const char *copilot;
	const char *prevowner;
	const char *dest;
	const char *nickname;
	const char *colorone;
	const char *colortwo;
	short type;
	short ship_class;
	int stype;
	short comm;
	short sensor;
	short astro_array;
	short hyperspeed;
	int hyperdistance;
	short realspeed;
	short currspeed;
	short shipstate;
	short statet0;
	short statet1;
	short statet2;
	short missiletype;
	short missilestate;
	short missiles;
	short maxmissiles;
	short torpedos;
	short maxtorpedos;
	short rockets;
	short maxrockets;
	short lasers;
	short tractorbeam;
	short manuever;
	bool bayopen;
	bool hatchopen;
	bool autorecharge;
	bool autotrack;
	bool autospeed;
	float vx, vy, vz;
	float hx, hy, hz;
	float jx, jy, jz;
	int maxenergy;
	int energy;
	int shield;
	int maxshield;
	int armor;
	int maxarmor;
	int cockpit;
	int turret1;
	int turret2;
	int location;
	int lastdoc;
	int shipyard;
	int entrance;
	int hanger;
	int engineroom;
	int firstroom;
	int lastroom;
	int navseat;
	int pilotseat;
	int coseat;
	int gunseat;
	long collision;
	SHIP_DATA *target0;
	SHIP_DATA *target1;
	SHIP_DATA *target2;
	SPACE_DATA *currjump;
	short chaff;
	short maxchaff;
	short ammo;
	short maxammo;
	short firstweapon;
	short secondweapon;
	short thirdweapon;
	short chaff_released;
	bool autopilot;
	int armorhead;
	int maxarmorhead;
	int armorlarm;
	int maxarmorlarm;
	int armorrarm;
	int maxarmorrarm;
	int armorlegs;
	int maxarmorlegs;
	short targettype;
	short alloy;
	int offon;
	int mod;
	CHAR_DATA *ch;
	int flags;
	int code;
	int price;
	int pksuit;
	int autoboom;
	int mines;
	int maxmines;
	int bombs;
	int prevtimer;
	int hover;
	int radio;
	int station;
	int cargo;
	int maxcargo;
	int cargotype;
	int fstate;
	const char *wonename;
	int wonedam;
	int wonetype;
	int wonelvl;
	const char *wtwoname;
	int wtwodam;
	int wtwotype;
	int wtwolvl;
	const char *framename;
	int frame;
	int framemax;
	int shieldb;
	int shieldmaxb;
	int speed;
	int speedmax;
	int dpow;
	int parm;
	int agility;
	int agilitymax;
	int reserve;
	int reservemax;
	int extra;
	// New Weapon System
	int wrightarm;
	int wleftarm;
	int wrightarmname;
	int wrightarmtype;
	int wrightarmdam;
	int wleftarmname;
	int wleftarmtype;
	int wleftarmdam;
	int counter;
	int countertype;
	int armortype;
	//
	int sp;
	int spmax;
	int weight;
	int grid;
	int gridlvl;
	float tlength;
	int position;
	int fnum;
	int move[4];
};

#define POS_NORTH     1
#define POS_SOUTH     2
#define POS_WEST      3
#define POS_EAST      4
#define POS_UP        5
#define POS_DOWN      6

#define GRID_ONE      1
#define GRID_TWO      2
#define GRID_THREE    3
#define GRID_FOUR     4


struct weapon_data
{
	WEAPON_DATA *next;
	WEAPON_DATA *prev;
	SHIP_DATA *fired_from;
	SHIP_DATA *target;
	const char *fired_by;
	short weapontype;
	float time;
	int counter;
};

struct weapon_type
{
	const char *name;  /* Name of skill          */
	int slot;
	int type;    /* Type of weapon         */
	int ammo;    /* Amount of Current Ammo */
	int ammomax; /* Amount of Maximum Ammo */
	int damage;  /* Damage it can do       */
	int ammotype;    /* Type of ammo           */
	int price;   /* Cost of Weapon         */
};

struct missile_data
{
	MISSILE_DATA *next;
	MISSILE_DATA *prev;
	MISSILE_DATA *next_in_starsystem;
	MISSILE_DATA *prev_in_starsystem;
	SPACE_DATA *starsystem;
	SHIP_DATA *target;
	SHIP_DATA *fired_from;
	const char *fired_by;
	short missiletype;
	short age;
	int speed;
	int mx, my, mz;
};


struct tourney_data
{
	int open;
	int low_level;
	int hi_level;
};

/*
 * Data structure for notes.

struct	note_data
{
	NOTE_DATA *	next;
	NOTE_DATA * prev;
	char *	sender;
	char *	date;
	char *	to_list;
	char *	subject;
	int         voting;
	char *	yesvotes;
	char *	novotes;
	char *	abstentions;
	char *	text;

};
*/

/*
 * Data structure for notes.
 */
 /*
 #define NOTE_NOTE	0
 #define NOTE_IDEA	1
 #define NOTE_PENALTY	2
 #define NOTE_NEWS	3
 #define NOTE_CHANGES	4
 #define NOTE_SIGN	5
 #define NOTE_WEDDINGS	6
 */
struct note_data
{
	NOTE_DATA *next;
	NOTE_DATA *prev;
	bool valid;
	short type;
	const char *sender;
	const char *date;
	const char *to_list;
	const char *subject;
	time_t date_stamp;
	int voting;
	const char *yesvotes;
	const char *novotes;
	const char *abstentions;
	const char *text;
	time_t expire;
};


/*
 * An affect.
 */
struct affect_data
{
	AFFECT_DATA *next;
	AFFECT_DATA *prev;
	short type;
	int duration;
	short location;
	int modifier;
	int bitvector;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
	SMAUG_AFF *next;
	SMAUG_AFF *prev;
	const char *duration;
	short location;
	const char *modifier;
	int bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

 /*
  * Well known mob virtual numbers.
  * Defined in #MOBILES.
  */
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10

#define MOB_VNUM_STORMTROOPER	20
#define MOB_VNUM_IMP_GUARD	21
#define MOB_VNUM_NR_GUARD	22
#define MOB_VNUM_NR_TROOPER	23
#define MOB_VNUM_MERC   	24
#define MOB_VNUM_BOUNCER	25
#define MOB_VNUM_INFECTED       26
#define MOB_VNUM_HEARTLESS_LVL_ONE   60
#define MOB_VNUM_HEARTLESS_LVL_TWO   61
#define MOB_VNUM_HEARTLESS_LVL_THREE 62
#define MOB_VNUM_HEARTLESS_LVL_FOUR  63
#define MOB_VNUM_HEARTLESS_LVL_FIVE  64
#define MOB_VNUM_THUG           54
#define MOB_VNUM_HITMEN         55
#define MOB_VNUM_DRUG_ADDICT    56
#define MOB_VNUM_PIMP           57
#define MOB_VNUM_HOE            58

#define MOB_VNUM_SUPERMOB       3

  /*
   * ACT bits for mobs.
   * Used in #MOBILES.
   */
typedef enum
{
	ACT_IS_NPC, ACT_SENTINEL, ACT_SCAVENGER, ACT_NOQUEST, ACT_AGGRESSIVE, ACT_STAY_AREA, ACT_WIMPY,
	ACT_PET, ACT_TRAIN, ACT_PRACTICE, ACT_IMMORTAL, ACT_DEADLY, ACT_POLYSELF, ACT_META_AGGR, ACT_GUARDIAN,
	ACT_RUNNING, ACT_NOWANDER, ACT_MOUNTABLE, ACT_MOUNTED, ACT_SECRETIVE,	ACT_POLYMORPHED, ACT_MOBINVIS, 
	ACT_NOASSIST, ACT_NOKILL, ACT_DROID, ACT_NOCORPSE, ACT_PROTOTYPE,

	MAX_ACT_FLAG
} actFlags;



/* bits for vip flags */

#define VIP_CORUSCANT           BV00
#define VIP_KASHYYYK          	BV01
#define VIP_RYLOTH            	BV02
#define VIP_RODIA             	BV03
#define VIP_NAL_HUTTA           BV04
#define VIP_MON_CALAMARI       	BV05
#define VIP_HONOGHR             BV06
#define VIP_GAMORR              BV07
#define VIP_TATOOINE            BV08
#define VIP_ADARI           	BV09
#define VIP_BYSS		BV10
#define VIP_ENDOR		BV11
#define VIP_ROCHE		BV12
#define VIP_AF_EL		BV13
#define VIP_TRANDOSH		BV14
#define VIP_CHAD		BV15

/* player wanted bits */

#define WANTED_MON_CALAMARI   	VIP_MON_CALAMARI
#define WANTED_CORUSCANT   	VIP_CORUSCANT
#define WANTED_ADARI   		VIP_ADARI
#define WANTED_RODIA   		VIP_RODIA
#define WANTED_RYLOTH   	VIP_RYLOTH
#define WANTED_GAMORR   	VIP_GAMORR
#define WANTED_TATOOINE   	VIP_TATOOINE
#define WANTED_BYSS   		VIP_BYSS
#define WANTED_NAL_HUTTA   	VIP_NAL_HUTTA
#define WANTED_KASHYYYK   	VIP_KASHYYYK
#define WANTED_HONOGHR   	VIP_HONOGHR
#define WANTED_ENDOR		BV11
#define WANTED_ROCHE		BV12
#define WANTED_AF_EL		BV13
#define WANTED_TRANDOSH		BV14
#define WANTED_CHAD		BV15


/* Bits for Suit Flags */
#define SUIT_CLOAK              BV00
#define SUIT_INCOMBAT           BV01
#define SUIT_HASONE             BV02
#define SUIT_HASTWO             BV03
#define SUIT_ISGUARDING         BV04
#define SUIT_MOBSUIT            BV05
#define SUIT_NEWSYSTEM          BV06
#define SUIT_INGRID             BV07
#define SUIT_MOVING             BV08
#define SUIT_BOOSTING           BV09
#define SUIT_AR_PHASESHIFT      BV10
#define SUIT_AR_TRANSPHASE      BV11
#define SUIT_AR_PANZER          BV12
#define SUIT_AR_LAMINATED       BV13
#define SUIT_AR_ANTIBEAM        BV14
#define SUIT_PHASESHIFTON       BV15
#define SUIT_HASSHIELD          BV16
#define SUIT_ENGAGED            BV17

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */


#define AFF_NONE                  0

#define AFF_BLIND		  BV00
#define AFF_INVISIBLE		  BV01
#define AFF_DETECT_EVIL		  BV02
#define AFF_DETECT_INVIS	  BV03
#define AFF_DETECT_MAGIC	  BV04
#define AFF_DETECT_HIDDEN	  BV05
#define AFF_WEAKEN		  BV06
#define AFF_SANCTUARY		  BV07
#define AFF_MASS_HEAL		  BV08
#define AFF_INFRARED		  BV09
#define AFF_STALK		  BV10
#define AFF_CHANGE_SEX		  BV11
#define AFF_POISON		  BV12
#define AFF_NEMESIS		  BV13
#define AFF_PARALYSIS		  BV14
#define AFF_SNEAK		  BV15
#define AFF_HIDE		  BV16
#define AFF_SLEEP		  BV17
#define AFF_CHARM		  BV18
#define AFF_FLYING		  BV19
#define AFF_MONEY_MAKER		  BV20
#define AFF_FLOATING		  BV21
#define AFF_TRUESIGHT		  BV22
#define AFF_DETECTTRAPS		  BV23
#define AFF_SCRYING	          BV24
#define AFF_FIRESHIELD	          BV25
#define AFF_SHOCKSHIELD	          BV26
#define AFF_PROTECT_VOODOO        BV27
#define AFF_ICESHIELD  		  BV28
#define AFF_POSSESS		  BV29
#define AFF_BERSERK		  BV30
#define AFF_AQUA_BREATH		  BV31


 /* 31 aff's (1 left.. :P) */
 /* make that none - ugh - time for another field? :P */
 /*
  * Resistant Immune Susceptible flags
  */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
#define RIS_SHOT                  BV22
#define RIS_WHIP                  BV23
#define RIS_CLAW                  BV24
  /* 21 RIS's*/

  /*
   * Attack types
   */
#define ATCK_BITE		  BV00
#define ATCK_CLAWS		  BV01
#define ATCK_TAIL		  BV02
#define ATCK_STING		  BV03
#define ATCK_PUNCH		  BV04
#define ATCK_KICK		  BV05
#define ATCK_TRIP		  BV06
#define ATCK_BASH		  BV07
#define ATCK_STUN		  BV08
#define ATCK_GOUGE		  BV09
#define ATCK_BACKSTAB		  BV10
#define ATCK_FEED		  BV11
#define ATCK_DRAIN		  BV12
#define ATCK_FIREBREATH		  BV13
#define ATCK_FROSTBREATH	  BV14
#define ATCK_ACIDBREATH		  BV15
#define ATCK_LIGHTNBREATH	  BV16
#define ATCK_GASBREATH		  BV17
#define ATCK_POISON		  BV18
#define ATCK_NASTYPOISON	  BV19
#define ATCK_GAZE		  BV20
#define ATCK_BLINDNESS		  BV21
#define ATCK_CAUSESERIOUS	  BV22
#define ATCK_EARTHQUAKE		  BV23
#define ATCK_CAUSECRITICAL	  BV24
#define ATCK_CURSE		  BV25
#define ATCK_FLAMESTRIKE	  BV26
#define ATCK_HARM		  BV27
#define ATCK_FIREBALL		  BV28
#define ATCK_COLORSPRAY		  BV29
#define ATCK_WEAKEN		  BV30
#define ATCK_CIRCLE	          BV31
   /* 32 USED! DO NOT ADD MORE! SB */

   /*
	* Defense types
	*/
#define DFND_PARRY		  BV00
#define DFND_DODGE		  BV01
#define DFND_HEAL		  BV02
#define DFND_CURELIGHT		  BV03
#define DFND_CURESERIOUS	  BV04
#define DFND_CURECRITICAL	  BV05
#define DFND_DISPELMAGIC	  BV06
#define DFND_DISPELEVIL		  BV07
#define DFND_SANCTUARY		  BV08
#define DFND_FIRESHIELD		  BV09
#define DFND_SHOCKSHIELD	  BV10
#define DFND_SHIELD		  BV11
#define DFND_BLESS		  BV12
#define DFND_STONESKIN		  BV13
#define DFND_TELEPORT		  BV14
#define DFND_MONSUM1		  BV15
#define DFND_MONSUM2		  BV16
#define DFND_MONSUM3		  BV17
#define DFND_MONSUM4		  BV18
#define DFND_DISARM		  BV19
#define DFND_ICESHIELD 		  BV20
#define DFND_GRIP		  BV21
	/* 21 def's */

	/*
	 * Body parts
	 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
	 /* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00
#define SV_KILL			  BV01
#define SV_PASSCHG		  BV02
#define SV_DROP			  BV03
#define SV_PUT			  BV04
#define SV_GIVE			  BV05
#define SV_AUTO			  BV06
#define SV_ZAPDROP		  BV07
#define SV_AUCTION		  BV08
#define SV_GET			  BV09
#define SV_RECEIVE		  BV10
#define SV_IDLE			  BV11
#define SV_BACKUP		  BV12

 /*
  * Pipe flags
  */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

  /*
   * Skill/Spell flags	The minimum BV *MUST* be 11!
   */
#define SF_WATER  		  BV11
#define SF_ART  		  BV12
#define SF_AIR			  BV13
#define SF_ASTRAL		  BV14
#define SF_AREA			  BV15  /* is an area spell     */
#define SF_DISTANT		  BV16  /* affects something far away   */
#define SF_REVERSE		  BV17
#define SF_SAVE_HALF_DAMAGE	  BV18  /* save for half damage     */
#define SF_SAVE_NEGATES		  BV19  /* save negates affect      */
#define SF_ACCUMULATIVE		  BV20  /* is accumulative      */
#define SF_RECASTABLE		  BV21  /* can be refreshed     */
#define SF_NOSCRIBE		  BV22  /* cannot be scribed        */
#define SF_NOBREW		  BV23  /* cannot be brewed     */
#define SF_GROUPSPELL		  BV24  /* only affects group members   */
#define SF_OBJECT		  BV25  /* directed at an object    */
#define SF_CHARACTER		  BV26  /* directed at a character  */
#define SF_SECRETSKILL		  BV27  /* hidden unless learned    */
#define SF_PKSENSITIVE		  BV28  /* much harder for plr vs. plr  */
#define SF_STOPONFAIL		  BV29  /* stops spell on first failure */

typedef enum
{
	SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
	SS_BREATH, SS_SPELL_STAFF
} save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum
{
	SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
	SD_POISON, SD_DRAIN
} spell_dam_types;

typedef enum
{
	SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
	SA_DIVINATE, SA_OBSCURE, SA_CHANGE
} spell_act_types;

typedef enum
{
	SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR
} spell_power_types;

typedef enum
{
	SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
	SC_LIFE, SC_DEATH, SC_ILLUSION
} spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{
	SEX_NONBINARY, SEX_MALE, SEX_FEMALE
} sex_types;

typedef enum
{
	TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART, TRAP_TYPE_POISON_NEEDLE,
	TRAP_TYPE_POISON_DAGGER, TRAP_TYPE_POISON_ARROW, TRAP_TYPE_BLINDNESS_GAS,
	TRAP_TYPE_SLEEPING_GAS, TRAP_TYPE_FLAME, TRAP_TYPE_EXPLOSION,
	TRAP_TYPE_ACID_SPRAY, TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
	TRAP_TYPE_SEX_CHANGE
} trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define OBJ_VNUM_DROID_CORPSE        9
#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_SPILLED_GUTS	     16
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32

#define OBJ_VNUM_BLACK_POWDER	     33
#define OBJ_VNUM_SCROLL_SCRIBING     34
#define OBJ_VNUM_FLASK_BREWING       35
#define OBJ_VNUM_NOTE		     36
#define OBJ_VNUM_TICKET              62
#define OBJ_VNUM_VOODOO              75
#define OBJ_VNUM_ARTIFACT            77
#define OBJ_VNUM_PTRAP               90
#define OBJ_VNUM_DRUG               110
#define OBJ_VNUM_DRUG1              111
#define OBJ_VNUM_DRUG2              112
#define OBJ_VNUM_DRUG3              113
#define OBJ_VNUM_DRUG4              114
#define OBJ_VNUM_DRUG5              115
#define OBJ_VNUM_DRUG6              116
#define OBJ_VNUM_DRUG7              117
#define OBJ_VNUM_DRUG8              118
#define OBJ_VNUM_DRUG9              119
#define OBJ_VNUM_DRUG10             120
#define OBJ_VNUM_DRUG11             121
#define OBJ_VNUM_DRUG12             122


 /* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE	  10315
#define OBJ_VNUM_SCHOOL_DAGGER	  10312
#define OBJ_VNUM_SCHOOL_SWORD	  10313
#define OBJ_VNUM_SCHOOL_VEST	  10308
#define OBJ_VNUM_SCHOOL_SHIELD	  10310
#define OBJ_VNUM_SCHOOL_BANNER    10311
#define OBJ_VNUM_SCHOOL_DIPLOMA   10321

#define OBJ_VNUM_BLASTECH_E11     50
#define OBJ_VNUM_DESERT_EAGLE     52
#define OBJ_SHARD                 66
#define OBJ_SUITPART              91
#define OBJ_LEAF                 130

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
	ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
	ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
	ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
	ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
	ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
	ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
	ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
	ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
	ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
	ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
	ITEM_SHORT_BOW, ITEM_LONG_BOW, ITEM_CROSSBOW, ITEM_AMMO, ITEM_QUIVER,
	ITEM_SHOVEL, ITEM_SALVE, ITEM_RAWSPICE, ITEM_LENS, ITEM_CRYSTAL, ITEM_DURAPLAST,
	ITEM_BATTERY, ITEM_TOOLKIT, ITEM_DURASTEEL, ITEM_OVEN, ITEM_MIRROR,
	ITEM_CIRCUIT, ITEM_SUPERCONDUCTOR, ITEM_COMLINK, ITEM_MEDPAC, ITEM_FABRIC,
	ITEM_RARE_METAL, ITEM_MAGNET, ITEM_THREAD, ITEM_SPICE, ITEM_SMUT, ITEM_DEVICE, ITEM_SPACECRAFT,
	ITEM_GRENADE, ITEM_LANDMINE, ITEM_GOVERNMENT, ITEM_DROID_CORPSE, ITEM_BOLT,
	ITEM_CHEMICAL, ITEM_REMOTE, ITEM_JUKEBOX, ITEM_PTRAP, ITEM_DRUG,
	ITEM_MIXTURE, ITEM_WHOLDER, ITEM_FRAME, ITEM_SUITWEAPON, ITEM_WHETSTONE,
	ITEM_DEBIT_CARD, ITEM_RESTORE, ITEM_IMAX
} item_types;


#define MAX_ITEM_TYPE		     ITEM_IMAX
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
typedef enum
{
	ITEM_GLOW, ITEM_HUM, ITEM_DARK, ITEM_HUTT_SIZE, ITEM_CONTRABAND, ITEM_INVIS, ITEM_MAGIC, ITEM_NODROP,
	ITEM_BLESS, ITEM_ANTI_GOOD, ITEM_ANTI_EVIL, ITEM_ANTI_NEUTRAL, ITEM_NOREMOVE, ITEM_INVENTORY, ITEM_NOINVOKE,
	ITEM_ANTI_THIEF, ITEM_ANTI_HUNTER, ITEM_ARTIFACT, ITEM_SMALL_SIZE, ITEM_LARGE_SIZE, ITEM_DONATION, ITEM_CLANOBJECT,
	ITEM_ANTI_CITIZEN, ITEM_ANTI_SITH, ITEM_ANTI_PILOT, ITEM_HIDDEN, ITEM_POISONED, ITEM_COVERING, ITEM_DEATHROT,
	ITEM_BURRIED, ITEM_PROTOTYPE, ITEM_HUMAN_SIZE, ITEM_AUGMENTED, 
	
	MAX_ITEM_FLAG
} item_extra_flags;

 /* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_LOYAL		BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06

/* Blaster settings - only saves on characters */
#define BLASTER_NORMAL          0
#define BLASTER_HALF		2
#define BLASTER_FULL            5
#define BLASTER_LOW		1
#define	BLASTER_STUN		3
#define BLASTER_HIGH            4

/* Weapon Types */

#define WEAPON_NONE     	0
#define WEAPON_VIBRO_AXE	1
#define WEAPON_VIBRO_BLADE	2
#define WEAPON_LIGHTSABER	3
#define WEAPON_WHIP		4
#define WEAPON_CLAW		5
#define WEAPON_BLASTER		6
#define WEAPON_BLUDGEON		8
#define WEAPON_BOWCASTER        9
#define WEAPON_FORCE_PIKE	11
#define WEAPON_STRIKE           12

/* Furniture Types - Darrik Vequir 10/23/00 */

#define STAND_AT		1
#define STAND_ON		2
#define STAND_IN		3
#define SIT_AT			1
#define SIT_ON			2
#define SIT_IN			3
#define REST_AT			1
#define REST_ON			2
#define REST_IN			3
#define SLEEP_AT		1
#define SLEEP_ON		2
#define SLEEP_IN		3
#define PUT_AT			1
#define PUT_ON			2
#define PUT_IN			3
#define PUT_INSIDE		4

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02

/* drug types */
#define SPICE_GLITTERSTIM        0
#define SPICE_CARSANUM           1
#define SPICE_RYLL               2
#define SPICE_ANDRIS             3

/* crystal types */
#define GEM_NON_ADEGEN          0
#define GEM_KATHRACITE		1
#define GEM_RELACITE		2
#define GEM_DANITE		3
#define GEM_MEPHITE		4
#define GEM_PONITE		5
#define GEM_ILLUM               6
#define GEM_CORUSCA             7

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WIELD		BV13
#define ITEM_HOLD		BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_MISSILE_WIELD	BV18
#define ITEM_WEAR_FLOATING      BV19
#define ITEM_WEAR_ANKLE         BV20
#define ITEM_WEAR_BACK          BV21
#define ITEM_WEAR_RELIC         BV22

 /*
  * Apply types (for affects).
  * Used in #OBJECTS.
  */
typedef enum
{
	APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
	APPLY_SEX, APPLY_NULL, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
	APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
	APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
	APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
	APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
	APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
	APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
	APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
	APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
	APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
	APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
	APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_RECURRINGSPELL,
	APPLY_EXT_AFFECT,
	MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8

 /*
  * Well known room virtual numbers.
  * Defined in #ROOMS.
  */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_POLY		      3
#define ROOM_VNUM_CHAT		  32144
#define ROOM_VNUM_TEMPLE	  32144
#define ROOM_VNUM_ALTAR		  32144
#define ROOM_VNUM_SCHOOL	    300
#define ROOM_AUTH_START		    300
#define ROOM_START_HUMAN            211
#define ROOM_START_WOOKIEE        28600
#define ROOM_START_TWILEK         32148
#define ROOM_START_RODIAN         32148
#define ROOM_START_HUTT           32148
#define ROOM_START_MON_CALAMARIAN 21069
#define ROOM_START_NOGHRI          1015
#define ROOM_START_GAMORREAN      28100
#define ROOM_START_JAWA           31819
#define ROOM_START_ADARIAN        29000
#define ROOM_START_EWOK           32148
#define ROOM_START_VERPINE        32148
#define ROOM_START_DEFEL          32148
#define ROOM_START_TRANDOSHAN     32148
#define ROOM_START_CHADRA_FAN     32148
#define ROOM_START_DUINUOGWUIN    32148
#define ROOM_START_QUARREN        21069
#define ROOM_START_IMMORTAL         100
#define ROOM_LIMBO_SHIPYARD          45
#define ROOM_DEFAULT_CRASH        28025
#define ROOM_MORGUE                  46
#define ROOM_VNUM_HELL                6
#define ROOM_VNUM_WAITINGROOM        60

#define ROOM_PLUOGUS_QUIT          32410

#define ROOM_SHUTTLE_BUS           32140 /*PLUOGUS*/
#define ROOM_SHUTTLE_BUS_2         32410 /*TOCCA*/
#define ROOM_CORUSCANT_SHUTTLE     199
#define ROOM_SENATE_SHUTTLE      10197
#define ROOM_CORUSCANT_TURBOCAR   2080

  /*
   * Room flags.           Holy cow!  Talked about stripped away..
   * Used in #ROOMS.       Those merc guys know how to strip code down.
   *			 Lets put it all back... ;)
   */
typedef enum
{
	ROOM_DARK, BFS_MARK, ROOM_NO_MOB, ROOM_INDOORS, ROOM_CAN_LAND, ROOM_CAN_FLY, ROOM_NO_DRIVING,
	ROOM_NO_MAGIC, ROOM_BANK, ROOM_PRIVATE, ROOM_SAFE, ROOM_SOLITARY, ROOM_PET_SHOP, ROOM_NO_RECALL,
	ROOM_DONATION, ROOM_NODROPALL, ROOM_SILENCE, ROOM_LOGSPEECH, ROOM_NODROP, ROOM_CLANSTOREROOM,
	ROOM_PLR_HOME, ROOM_EMPTY_HOME, ROOM_TELEPORT, ROOM_HOTEL, ROOM_NOFLOOR, ROOM_REFINERY, ROOM_FACTORY,
	ROOM_R_RECRUIT, ROOM_E_RECRUIT, ROOM_SPACECRAFT, ROOM_PROTOTYPE, ROOM_ARENA, ROOM_HEAL, ROOM_DOCKING,
	ROOM_TAG, ROOM_CYBER, ROOM_LOTTERY, ROOM_UPGRADE_CENTER, ROOM_STORAGE, ROOM_BASE, ROOM_CAPTURE, 
	
	ROOM_MAX
} room_flags;

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
	DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
	DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define MAX_DIR			DIR_SOUTHWEST   /* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE   /* portal direction   */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_RES1                   BV09  /* are these res[1-4] important? */
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES2			  BV14
#define EX_RES3			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_RES4	  		  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define MAX_EXFLAG		  26

 /*
  * Sector types.
  * Used in #ROOMS.
  */
typedef enum
{
	SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
	SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
	SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
	WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
	WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
	WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
	WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
	WEAR_MISSILE_WIELD, WEAR_FLOATING, WEAR_ANKLE_L, WEAR_ANKLE_R, WEAR_BACK,
	WEAR_RELIC,
	MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum
{
	BOARD_NOTE, BOARD_MAIL
} board_types;

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

 /*
  * Conditions.
  */
typedef enum
{
	COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
	POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING,
	POS_RESTING, POS_SITTING, POS_FIGHTING,
	POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
 * ACT bits for players.
 */
typedef enum
{
	PLR_IS_NPC, PLR_BOUGHT_PET, PLR_SHOVEDRAG, PLR_AUTOEXIT, PLR_AUTOLOOT, PLR_AUTOSAC, PLR_QUESTOR,
	PLR_BLANK, PLR_BRIEF, PLR_COMBINE, PLR_PROMPT, PLR_TELNET_GA, PLR_HOLYLIGHT, PLR_WIZINVIS, PLR_ROOMVNUM,
	PLR_SILENCE, PLR_NO_EMOTE, PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE, PLR_KILLER,
	PLR_PKER, PLR_LITTERBUG, PLR_ANSI, PLR_SOUND, PLR_NICE, PLR_PPKER, PLR_AUTOGOLD, PLR_AUTOMAP, PLR_AFK,
	PLR_ASCENDED,

	MAX_PLR_FLAG
} player_flags;

#define	WAR_OFF     0
#define	WAR_WAITING 1
#define	WAR_RUNNING 2

struct war_data
{
	int min_level;
	int max_level;
	int inwar;
	int wartype;
	int timer;
	int iswar;
	int next;
	bool suddendeath;
};

/* Bits for pc_data->flags. */
#define PCFLAG_BOTTER              BV00
#define PCFLAG_FBALZHUR            BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_ROOM                BV11
#define PCFLAG_DND                 BV12
#define PCFLAG_NORESTORE           BV13
#define PCFLAG_ZERO                BV14
#define PCFLAG_ARENA               BV15
#define PCFLAG_NO_OOC              BV16
#define PCFLAG_WAR                 BV17
#define PCFLAG_NOAUCTION           BV18
#define PCFLAG_AUTOWHO             BV19
#define PCFLAG_STUN                BV20
#define PCFLAG_TWIT                BV21
#define PCFLAG_HASLOTTO            BV22
#define PCFLAG_LOTTOWAIT           BV23
#define PCFLAG_AUTOSCAN BV24
#define PCFLAG_EXEMPT              BV25
#define PCFLAG_CANLOOT             BV26
#define PCFLAG_NONOTE              BV27
#define PCFLAG_UNUSED              BV28
#define PCFLAG_ATTACKMODE          BV29
#define PCFLAG_SPIRITMODE          BV30
#define PCFLAG_INCOMBAT            BV31

#define EXTRA_EXP		BV00
#define EXTRA_DONE		BV01
#define EXTRA_MARRIED		BV02
#define EXTRA_PREGNANT		BV03
#define EXTRA_LABOR		BV04
#define EXTRA_BORN		BV05

struct tag_data
{
	int status;
	int timer;
	int next;
	int playing;
};

struct clan_titles
{
	const char *title_of_rank[3];
};

#define TAG_OFF 0
#define TAG_ISWAIT 1
#define TAG_ISPLAY 2

#define TAG_PLAYING		BV00
#define TAG_FROZEN              BV01
#define TAG_RED			BV02
#define TAG_BLUE	        BV03
#define TAG_WAITING             BV04

#define ROOM_FTAG_WAIT_ROOM             69
#define ROOM_FTAG_MIN_VNUM		70
#define ROOM_FTAG_MAX_VNUM		99


typedef enum
{
	TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
	TIMER_APPLIED, TIMER_PKILLED
} timer_types;

struct timer_data
{
	TIMER *prev;
	TIMER *next;
	DO_FUN *do_fun;
	int value;
	short type;
	short count;
};


/*
 * Channel bits.
 */

#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_INFO		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
 //#define CHANNEL_SWEAR        BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_QUOTE		   BV13
#define CHANNEL_BUG                BV14
#define CHANNEL_ARENA		   BV15
#define CHANNEL_FREEZE  	   BV16
#define CHANNEL_RANK               BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_OOC                BV23
#define CHANNEL_SHIP               BV24
#define CHANNEL_SYSTEM             BV25
#define CHANNEL_SPACE              BV26
#define CHANNEL_RP		   BV27
#define CHANNEL_GOCIAL             BV28
#define CHANNEL_i104               BV29
#define CHANNEL_PEEKAY BV30

#define MAX_CHANNEL 31

#define CHANNEL_CLANTALK	   CHANNEL_CLAN

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

 /* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_BASE                  BV01
#define AFLAG_PROTOTYPE             BV02

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
	MOB_INDEX_DATA *next;
	MOB_INDEX_DATA *next_sort;
	SPEC_FUN *spec_fun;
	SPEC_FUN *spec_2;
	SHOP_DATA *pShop;
	REPAIR_DATA *rShop;
	MPROG_DATA *mudprogs;
	EXT_BV progtypes;
	const char *player_name;
	const char *short_descr;
	const char *long_descr;
	const char *description;
	const char *spec_funname;
	const char *spec_funname2;
	int vnum;
	short count;
	short killed;
	short sex;
	short level;
	EXT_BV act;
	int affected_by;
	short alignment;
	short mobthac0; /* Unused */
	short ac;
	short hitnodice;
	short hitsizedice;
	int hitplus;
	short damnodice;
	short damsizedice;
	short damplus;
	short numattacks;
	int gold;
	int exp;
	int xflags;
	int resistant;
	int immune;
	int susceptible;
	int attacks;
	int defenses;
	short position;
	short defposition;
	short height;
	short weight;
	short race;
	short hitroll;
	short damroll;
	short perm_str;
	short perm_int;
	short perm_wis;
	short perm_dex;
	short perm_con;
	short perm_cha;
	short perm_lck;
	short saving_poison_death;
	short saving_wand;
	short saving_para_petri;
	short saving_breath;
	short saving_spell_staff;
	int vip_flags;
	long bank;
	long parts;
};


struct hunt_hate_fear
{
	const char *name;
	CHAR_DATA *who;
};

struct fighting_data
{
	CHAR_DATA *who;
	int xp;
	short align;
	short duration;
	short timeskilled;
};

struct attacking_data
{
	CHAR_DATA *who;
};

struct editor_data
{
	short numlines;
	short on_line;
	short size;
	char line[49][MAX_LINE_LENGTH + 2];
};

struct extracted_char_data
{
	EXTRACT_CHAR_DATA *next;
	CHAR_DATA *ch;
	ROOM_INDEX_DATA *room;
	ch_ret retcode;
	bool extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct char_data
{
	CHAR_DATA *next;
	CHAR_DATA *prev;
	CHAR_DATA *next_in_room;
	CHAR_DATA *prev_in_room;
	CHAR_DATA *master;
	CHAR_DATA *leader;
	FIGHT_DATA *fighting;
	ATTACK_DATA *attacking;
	CHAR_DATA *reply;
	CHAR_DATA *switched;
	CHAR_DATA *mount;
	const char *gang;
	HHF_DATA *hunting;
	HHF_DATA *fearing;
	HHF_DATA *hating;
	SPEC_FUN *spec_fun;
	SPEC_FUN *spec_2;
	const char *spec_funname;
	const char *spec_funname2;
	MPROG_ACT_LIST *mpact;
	int mpactnum;
	unsigned short mpscriptpos;
	MOB_INDEX_DATA *pIndexData;
	DESCRIPTOR_DATA *desc;
	struct bit_data *first_abit; /* abit/qbit code */
	struct bit_data *last_abit;
	AFFECT_DATA *first_affect;
	AFFECT_DATA *last_affect;
	NOTE_DATA *pnote;
	NOTE_DATA *comments;
	OBJ_DATA *first_carrying;
	OBJ_DATA *last_carrying;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *was_in_room;
	ROOM_INDEX_DATA *was_sentinel;
	ROOM_INDEX_DATA *plr_home;
	PC_DATA *pcdata;
	DO_FUN *last_cmd;
	DO_FUN *prev_cmd;    /* mapping */
	void *dest_buf;
	void *dest_buf_2;
	void *spare_ptr;
	int tempnum;
	EDITOR_DATA *editor;
	TIMER *first_timer;
	TIMER *last_timer;
	const char *name;
	const char *short_descr;
	const char *long_descr;
	const char *description;
	const char *mailbuf;
	short num_fighting;
	short substate;
	short sex;
	short race;
	short top_level;
	short skill_level[MAX_ABILITY];
	short trust;
	int played;
	time_t logon;
	time_t save_time;
	short timer;
	short wait;
	long hit;
	long max_hit;
	int move;
	int max_move;
	short numattacks;
	int gold;
	short wfm_timer;
	long experience[MAX_ABILITY];
	EXT_BV act;
	int affected_by;
	int carry_weight;
	int carry_number;
	int xflags;
	int resistant;
	int immune;
	int susceptible;
	int attacks;
	int defenses;
	short saving_poison_death;
	short saving_wand;
	short saving_para_petri;
	short saving_breath;
	short saving_spell_staff;
	short alignment;
	short barenumdie;
	short baresizedie;
	short mobthac0;
	short hitroll;
	short damroll;
	int hitplus;
	short damplus;
	short position;
	short defposition;
	short height;
	short weight;
	short armor;
	short wimpy;
	int deaf;
	int tells;
	short perm_str;
	short perm_int;
	short perm_wis;
	short perm_dex;
	short perm_con;
	short perm_cha;
	short perm_lck;
	short mod_str;
	short mod_int;
	short mod_wis;
	short mod_dex;
	short mod_con;
	short mod_cha;
	short mod_lck;
	short mental_state; /* simplified */
	short emotional_state;  /* simplified */
	int pagelen; /* BUILD INTERFACE */
	short inter_page;   /* BUILD INTERFACE */
	short inter_type;   /* BUILD INTERFACE */
	const char *inter_editing; /* BUILD INTERFACE */
	int inter_editing_vnum;  /* BUILD INTERFACE */
	short inter_substate;   /* BUILD INTERFACE */
	int retran;
	int regoto;
	short mobinvis; /* Mobinvis level SB */
	int vip_flags;
	short backup_wait;  /* reinforcements */
	int backup_mob;  /* reinforcements */
	short was_stunned;
	const char *mob_clan;  /* for spec_clan_guard.. set by postguard */
	GUARD_DATA *guard_data;
	short main_ability;
	CHAR_DATA *challenged;
	CHAR_DATA *betted_on;
	int bet_amt;
	ROOM_INDEX_DATA *buzzed_home;
	ROOM_INDEX_DATA *buzzed_from_room;
	OBJ_DATA *on;
	long parts;
	short questmob;
	short questobj;
	short questpoints;
	int nextquest;
	int countdown;
	CHAR_DATA *questgiver;
	int element; // Element
	short colors[MAX_COLORS];
	int home_vnum; /* hotboot tracker */
	int resetvnum;
	int resetnum;
 /* End end yadda yadda */
};

/*
 * Fight State
 */
typedef enum
{
	FS_OUT, FS_COMMAND, FS_WAITCOMMAND, FS_DEFEND, FS_FIGHT
} fightstates;

/*
 * Suit Weapon Types
 */
typedef enum
{
	WT_NONE, WT_SLASH, WT_PIERCE, WT_BULLET, WT_BEAM
} weapontypes;


struct killed_data
{
	int vnum;
	char count;
};


/* Structure for link list of ignored players */
struct ignore_data
{
	IGNORE_DATA *next;
	IGNORE_DATA *prev;
	const char *name;
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
	CLAN_DATA *clan;
	AREA_DATA *area;
	CHAR_DATA *partner;
	CHAR_DATA *propose;
	struct bit_data *firstqbit; /* abit/qbit code */
	struct bit_data *lastqbit;
	struct alias_type *first_alias;
	struct alias_type *last_alias;
	BOARD_DATA *board;   /* The current board */
	time_t last_note[MAX_BOARD]; /* last note for the boards */
	NOTE_DATA *in_progress;
	KILLED_DATA killed[MAX_KILLTRACK];
	time_t release_date; /* Auto-helling.. Altrag */
	SKILLTYPE *special_skills[5];    /* personalized skills/spells */
	IGNORE_DATA *first_ignored;  /* keep track of who to ignore */
	IGNORE_DATA *last_ignored;
	LOCKER_DATA *locker;
	ROOM_INDEX_DATA *locker_room;    /* Pointer to virtual room */
	BUFFER *buffer;
	const char *homepage;
	const char *clan_name;
	const char *pwd;
	const char *email;
	const char *msn;
	const char *aim;
	const char *yim;
	const char *enter;
	const char *exit;
	const char *real;
	const char *bamfin;
	const char *bamfout;
	const char *rank;
	const char *title;
	const char *job;
	const char *tspouse;
	const char *spouse;
	const char *lasthost;  /* Stores host info so it doesn't have to depend on descriptor, for things like finger */
	const char *bestowments;   /* Special bestowed commands       */
	const char *avatar;
	const char *helled_by;
	const char *bio;   /* Personal Bio */
	const char *authed_by; /* what crazy imm authed this name ;) */
	const char *prompt;    /* User config prompts */
	const char *subprompt; /* Substate prompt */
	const char *afkmess;
	const char *tellbuf;
	const char *inivictim;
	const char *iclan;
	const char *betted_on;
	long int outcast_time;   /* The time at which the char was outcast */
	long int restore_time;   /* The last time the char did a restore all */
	long int salary_date;
	long tag_flags;
	long bank;
	short wizinvis; /* wizinvis level */
	short min_snoop;    /* minimum snoop level */
	short condition[MAX_CONDS];
	short learned[MAX_SKILL];
	short quest_number; /* current *QUEST BEING DONE* DON'T REMOVE! */
	short quest_curr;   /* current number of quest points */
	short pagerlen; /* For pager (NOT menus) */
	short stage[3];
	short addiction[10];
	short drug_level[10];
	short clan_rank;
	short rprate;    /* New semi-enforced RP code */
	short lt_index;
	short cmd_recurse;
	bool hotboot; /* hotboot tracker */
	bool confirm_delete;
	bool dating;
	bool engaged;
	bool married;
	bool predating;
	bool preengaged;
	bool premarried;
	bool openedtourney;
	int rtime;   /*just how long have I been in this relationship */
	int flags;   /* Whether the player is deadly and whatever else we add.      */
	int pkills;  /* Number of pkills on behalf of clan */
	int pdeaths; /* Number of times pkilled (legally)  */
	int mkills;  /* Number of mobs killed           */
	int mdeaths; /* Number of deaths due to mobs       */
	int apkills;
	int apdeaths;
	int hasfroze;
	int beenfroze;
	int icq;
	int illegal_pk;  /* Number of illegal pk's committed   */
	int low_vnum;  /* vnum range */
	int hi_vnum;
	int quest_accum; /* quest points accumulated in players life */
	int auth_state;
	int wanted_flags;
	int bet_amt;
	int salary;
	int hair;
	int build;
	int eye;
	int hero;
	int highlight;
#ifdef I3
	char *i3_replyname;  /* Target for reply - Samson 1-23-01 */
	char *i3_listen; /* The I3 channels someone is listening to - Samson 1-30-01 */
	bool i3invis;    /* Invisible to I3? - Samson 2-7-01 */
	I3_IGNORE *i3first_ignore;   /* List of people to ignore stuff from - Samson 2-7-01 */
	I3_IGNORE *i3last_ignore;
#endif
	int tells;
	int cybaflags;
	int xenergy;
	int xenergymax;
	int silencetime;
	int chargelevel;
	int specialweapon;
	int hlevel;
	int hkills;
	int ragemeter;
	int ticketnumber;
	int ticketweek;
	int locker_vnum;
	int storagetimer;
	int storagecost;
	int lp;
	int version;
};


struct locker_data
{
	short capacity;
	short holding;
	int flags;
	int room;
};

#define CHARGE_NONE           0
#define CHARGE_ONE            1
#define CHARGE_TWO            2
#define CHARGE_THREE          3
#define CHARGE_FOUR           4
#define CHARGE_MAX            4

// Cyba Flags
/*
typedef enum
{
 CYBA_HASGUN, CYBA_HASBLADE, CYBA_HASCHARGER, CYBA_GUNOUT,
 CYBA_BLADEDRAWN, CYBA_ISCHARGING, CYBA_HASCHARGER2, CYBA_HASCHARGER3,
 CYBA_USINGSPECIAL, CYBA_R9, CYBA_REGULAR, CYBA_HASBUBBLE, CYBA_MAX
} xbuster_flags;
*/
#define CYBA_HASGUN           BV00
#define CYBA_HASBLADE         BV01
#define CYBA_HASCHARGER       BV02
#define CYBA_GUNOUT           BV03
#define CYBA_BLADEDRAWN       BV04
#define CYBA_ISCHARGING       BV05
#define CYBA_HASCHARGER2      BV06
#define CYBA_HASCHARGER3      BV07
#define CYBA_USINGSPECIAL     BV08
#define CYBA_REFILLEN         BV09
#define CYBA_REGULAR          BV10
#define CYBA_HASBUBBLE        BV11
#define CYBA_HASSMOG          BV12
#define CYBA_HASTORNADO       BV13
#define CYBA_HASLIFESAP       BV14
#define CYBA_HP_FIX           BV15
#define CYBA_NOLEVEL          BV16
#define CYBA_ASCENDED         BV17

// Special Attack number defines

#define NORMAL_ATTACK         0
#define BUBBLE_ATTACK         1
#define SMOG_ATTACK           2
#define TORNADO_ATTACK        3
#define LIFESAP_ATTACK        4

struct sweapon_table
{
	const char *sweapon_name;
};

/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		46

struct liq_type
{
	const char *liq_name;
	const char *liq_color;
	short liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
	EXTRA_DESCR_DATA *next;  /* Next in list                     */
	EXTRA_DESCR_DATA *prev;  /* Previous in list                 */
	const char *keyword;   /* Keyword in look/examine          */
	const char *description;   /* What to see                      */
};

/*
 * Prototype for an object.
 */
struct obj_index_data
{
	OBJ_INDEX_DATA *next;
	OBJ_INDEX_DATA *next_sort;
	EXTRA_DESCR_DATA *first_extradesc;
	EXTRA_DESCR_DATA *last_extradesc;
	AFFECT_DATA *first_affect;
	AFFECT_DATA *last_affect;
	MPROG_DATA *mudprogs;    /* objprogs */
	EXT_BV progtypes;   /* objprogs */
	EXT_BV extra_flags;
	const char *name;
	const char *short_descr;
	const char *description;
	const char *action_desc;
	short level;
	short item_type;
	short count;
	short weight;
	short layers;
	int wear_flags;
	int magic_flags; /*Need more bitvectors for spells - Scryn */
	int vnum;
	int cost;
	int value[6];
	int serial;
	int rent;    /* Unused */
};


/*
 * One object.
 */
struct obj_data
{
	OBJ_DATA *next;
	OBJ_DATA *prev;
	OBJ_DATA *next_content;
	OBJ_DATA *prev_content;
	OBJ_DATA *first_content;
	OBJ_DATA *last_content;
	OBJ_DATA *in_obj;
	CHAR_DATA *carried_by;
	EXTRA_DESCR_DATA *first_extradesc;
	EXTRA_DESCR_DATA *last_extradesc;
	AFFECT_DATA *first_affect;
	AFFECT_DATA *last_affect;
	OBJ_INDEX_DATA *pIndexData;
	ROOM_INDEX_DATA *in_room;
	EXT_BV extra_flags;
	const char *armed_by;
	const char *name;
	const char *short_descr;
	const char *description;
	const char *action_desc;
	const char *owner;
	const char *killer;
	short item_type;
	short mpscriptpos;
	short wear_loc;
	short weight;
	short level;
	short timer;
	short count;    /* support for object grouping */
	int magic_flags; /*Need more bitvectors for spells - Scryn */
	int wear_flags;
	int blaster_setting;
	MPROG_ACT_LIST *mpact;   /* mudprogs */
	int mpactnum;    /* mudprogs */
	int cost;
	int value[6];
	int serial;  /* serial number           */
	int room_vnum; /* hotboot tracker */
};


/*
 * Exit data.
 */
struct exit_data
{
	EXIT_DATA *prev; /* previous exit in linked list */
	EXIT_DATA *next; /* next exit in linked list */
	EXIT_DATA *rexit;    /* Reverse exit pointer     */
	ROOM_INDEX_DATA *to_room;    /* Pointer to destination room  */
	const char *keyword;   /* Keywords for exit or door    */
	const char *description;   /* Description of exit      */
	int vnum;    /* Vnum of room exit leads to   */
	int rvnum;   /* Vnum of room in opposite dir */
	int exit_info;   /* door states & other flags    */
	int key; /* Key vnum         */
	short vdir; /* Physical "direction"     */
	short distance; /* how far to the next room */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

 /*
  * Area-reset definition.
  */
struct reset_data
{
	RESET_DATA *next;
	RESET_DATA *prev;
	RESET_DATA *first_reset;
	RESET_DATA *last_reset;
	RESET_DATA *next_reset;
	RESET_DATA *prev_reset;
	char command;
	int extra;
	int arg1;
	int arg2;
	int arg3;
	bool sreset;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF    /* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00  /* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000    /* For reference */



/*
 * Area definition.
 */
struct area_data
{
	AREA_DATA *next;
	AREA_DATA *prev;
	AREA_DATA *next_sort;
	AREA_DATA *prev_sort;
	AREA_DATA *next_sort_name; /* Used for alphanum. sort */
	AREA_DATA *prev_sort_name; /* Ditto, Fireblade */
	PLANET_DATA *planet;
	AREA_DATA *next_on_planet;
	AREA_DATA *prev_on_planet;
	ROOM_INDEX_DATA *first_room;
	ROOM_INDEX_DATA *last_room;
	WEATHER_DATA *weather;
	const char *name;
	const char *filename;
	const char *author;    /* Scryn */
	const char *owned_by;  /* Cray */
	const char *resetmsg;  /* Rennard */
	short version;
	short status;    /* h, 8/11 */
	short age;
	short nplayer;
	short reset_frequency;
	short max_players;
	bool installed;
	int flags;
	int low_vnum;
	int hi_vnum;
	int low_soft_range;
	int hi_soft_range;
	int low_hard_range;
	int hi_hard_range;
	int mkills;
	int mdeaths;
	int pkills;
	int pdeaths;
	int gold_looted;
	int illegal_pk;
	int high_economy;
	int low_economy;	
};



/*
 * Load in the gods building data. -- Altrag
 */
struct godlist_data
{
	GOD_DATA *next;
	GOD_DATA *prev;
	int level;
	int low_vnum;
	int hi_vnum;
};

/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
	int maxplayers;  /* Maximum players this boot   */
	int alltimemax;  /* Maximum players ever   */
	const char *time_of_max;   /* Time of max ever */
	bool NO_NAME_RESOLVING;  /* Hostnames are not resolved  */
	bool DENY_NEW_PLAYERS;   /* New players cannot connect  */
	bool WAIT_FOR_AUTH;  /* New players must be auth'ed */
	short read_all_mail;    /* Read all player mail(was 54) */
	short read_mail_free;   /* Read mail for free (was 51) */
	short write_mail_free;  /* Write mail for free(was 51) */
	short take_others_mail; /* Take others mail (was 54)   */
	short muse_level;   /* Level of muse channel */
	short think_level;  /* Level of think channel LEVEL_HIGOD */
	short build_level;  /* Level of build channel LEVEL_BUILD */
	short log_level;    /* Level of log channel LEVEL LOG */
	short level_modify_proto;   /* Level to modify prototype stuff LEVEL_LIAISON */
	short level_override_private;   /* override private flag */
	short level_mset_player;    /* Level to mset a player */
	short stun_plr_vs_plr;  /* Stun mod player vs. player */
	short stun_regular; /* Stun difficult */
	short dam_plr_vs_plr;   /* Damage mod player vs. player */
	short dam_plr_vs_mob;   /* Damage mod player vs. mobile */
	short dam_mob_vs_plr;   /* Damage mod mobile vs. player */
	short dam_mob_vs_mob;   /* Damage mod mobile vs. mobile */
	short level_getobjnotake;   /* Get objects without take flag */
	short level_forcepc;    /* The level at which you can use force on players. */
	short max_sn;   /* Max skills */
	const char *guild_overseer;    /* Pointer to char containing the name of the */
	const char *guild_advisor; /* guild overseer and advisor. */
	int save_flags;  /* Toggles for saving conditions */
	short save_frequency;   /* How old to autosave someone */
	int maxign;
	int hoursperday;
	int daysperweek;
	int dayspermonth;
	int monthsperyear;
	int daysperyear;
	int hoursunrise;
	int hourdaybegin;
	int hournoon;
	int hoursunset;
	int hournightbegin;
	int hourmidnight;
	int jackpot;
	int lotterynum;
	int lotteryweek;
	int lotterytimer;
	int leafcount;
	const char *lastwinner;
	int storagetimer;
	short newbie_purge; /* Level to auto-purge newbies at - Samson 12-27-98 */
	short regular_purge;    /* Level to purge normal players at - Samson 12-27-98 */
	bool CLEANPFILES;    /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
	void *dlHandle; /* Dlysm support */
};

/*
 * Room type.
 */
struct room_index_data
{
	ROOM_INDEX_DATA *next;
	ROOM_INDEX_DATA *next_sort;
	CHAR_DATA *first_person;
	CHAR_DATA *last_person;
	OBJ_DATA *first_content;
	OBJ_DATA *last_content;
	EXTRA_DESCR_DATA *first_extradesc;
	EXTRA_DESCR_DATA *last_extradesc;
	AREA_DATA *area;
	EXIT_DATA *first_exit;
	EXIT_DATA *last_exit;
	SHIP_DATA *first_ship;
	SHIP_DATA *last_ship;
	RESET_DATA *first_reset;
	RESET_DATA *last_reset;
	RESET_DATA *last_mob_reset;
	RESET_DATA *last_obj_reset;
	ROOM_INDEX_DATA *next_aroom; /* Rooms within an area */
	ROOM_INDEX_DATA *prev_aroom;
	const char *name;
	MAP_DATA *map;   /* maps */
	const char *description;
	int vnum;
	EXT_BV room_flags;
	MPROG_ACT_LIST *mpact;   /* mudprogs */
	int mpactnum;    /* mudprogs */
	MPROG_DATA *mudprogs;    /* mudprogs */
	short mpscriptpos;
	EXT_BV progtypes;   /* mudprogs */
	short light;
	short sector_type;
	int tele_vnum;
	short tele_delay;
	short tunnel;    /* max people that will fit */
	const char *owner;
	const char *guests;
};

/*
 * Delayed teleport type.
 */
struct teleport_data
{
	TELEPORT_DATA *next;
	TELEPORT_DATA *prev;
	ROOM_INDEX_DATA *room;
	short timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000   /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000   /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000   /* allows for 1000 herb types    */

 /*
  *  Target types.
  */
typedef enum
{
	TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
	TAR_OBJ_INV
} target_types;

typedef enum
{
	SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
	SKILL_HERB, SKILL_ART
} skill_types;



struct timerset
{
	int num_uses;
	struct timeval total_time;
	struct timeval min_time;
	struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
	const char *name;  /* Name of skill        */
	SPELL_FUN *spell_fun;    /* Spell pointer (for spells)   */
	const char *spell_fun_name;
	DO_FUN *skill_fun;   /* Skill pointer (for skills)   */
	const char *skill_fun_name;
	short target;   /* Legal targets        */
	short minimum_position; /* Position for caster / user   */
	short slot; /* Slot for #OBJECT loading */
	short beats;    /* Rounds required to use skill */
	const char *noun_damage;   /* Damage message       */
	const char *msg_off;   /* Wear off message     */
	const char *artcombo;  /* Combo to perform Move        */
	short guild;    /* Which guild the skill belongs to */
	short min_level;    /* Minimum level to be able to cast */
	short type; /* Spell/Skill/Weapon/Tongue    */
	int info;
	int flags;   /* extra stuff          */
	const char *hit_char;  /* Success message to caster    */
	const char *hit_vict;  /* Success message to victim    */
	const char *hit_room;  /* Success message to room  */
	const char *miss_char; /* Failure message to caster    */
	const char *miss_vict; /* Failure message to victim    */
	const char *miss_room; /* Failure message to room  */
	const char *die_char;  /* Victim death msg to caster   */
	const char *die_vict;  /* Victim death msg to victim   */
	const char *die_room;  /* Victim death msg to room */
	const char *imm_char;  /* Victim immune msg to caster  */
	const char *imm_vict;  /* Victim immune msg to victim  */
	const char *imm_room;  /* Victim immune msg to room    */
	const char *dice;  /* Dice roll            */
	int value;   /* Misc value           */
	char saves;  /* What saving spell applies    */
	char difficulty; /* Difficulty of casting/learning */
	SMAUG_AFF *first_affect;  /* Spell affects, if any   */
	SMAUG_AFF *last_affect;
	const char *components;    /* Spell components, if any */
	const char *teachers;  /* Skill requires a special teacher */
	char participants;   /* # of required participants   */
	struct timerset userec;  /* Usage record         */
	int alignment;   /* for jedi powers */
	int lp;  /* Espionage skills */
};

/*
 *  Art register saving table.
 */
struct art_type
{
	const char *name;  /* Name of skill        */
	short slot; /* Slot for #OBJECT loading */
	short guild;    /* Which guild the skill belongs to */
	struct timerset userec;  /* Usage record         */
};

struct auction_data
{
	AUCTION_DATA *next;
	OBJ_DATA *item;
	CHAR_DATA *owner;
	CHAR_DATA *high_bidder;
	short status;
	long current_bid;
	long gold_held;
	long min_bid;
	bool valid;
};


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern short gsn_mobilesuits;
extern short gsn_midships;
extern short gsn_capitalships;
extern short gsn_weaponsystems;
extern short gsn_navigation;
extern short gsn_shipsystems;
extern short gsn_tractorbeams;
extern short gsn_shipmaintenance;
extern short gsn_spacecombat;
extern short gsn_spacecombat2;
extern short gsn_spacecombat3;
extern short gsn_bulletweapons;
extern short gsn_missileweapons;
extern short gsn_beamsabers;
extern short gsn_lightenergy;
extern short gsn_heavyenergy;
extern short gsn_meleeweapons;
extern short gsn_transform;
extern short gsn_zerosystem;
extern short gsn_bomb;
extern short gsn_spacemines;
extern short gsn_landcombat;

extern short gsn_reinforcements;
extern short gsn_postguard;

extern short gsn_addpatrol;
extern short gsn_eliteguard;
extern short gsn_specialforces;
extern short gsn_jail;
extern short gsn_smalltalk;
extern short gsn_propeganda;
extern short gsn_bribe;
extern short gsn_seduce;
extern short gsn_masspropeganda;
extern short gsn_gather_intelligence;
extern short gsn_sprint;

extern short gsn_torture;
extern short gsn_snipe;
extern short gsn_throw;
extern short gsn_disguise;
extern short gsn_mine;
extern short gsn_grenades;
extern short gsn_first_aid;
extern short gsn_rub;

extern short gsn_beg;
extern short gsn_camset;
extern short gsn_makecamera;
extern short gsn_makeblade;
extern short gsn_makejewelry;
extern short gsn_makeblaster;
extern short gsn_makelight;
extern short gsn_makecomlink;
extern short gsn_makegrenade;
extern short gsn_makelandmine;
extern short gsn_makearmor;
extern short gsn_makeshield;
extern short gsn_makecontainer;
extern short gsn_sharpen;
extern short gsn_makesweapon;
extern short gsn_gemcutting;
extern short gsn_lightsaber_crafting;
extern short gsn_spice_refining;

extern short gsn_detrap;
extern short gsn_backstab;
extern short gsn_circle;
extern short gsn_strangle;
extern short gsn_dodge;
extern short gsn_hide;
extern short gsn_peek;
extern short gsn_pick_lock;
extern short gsn_scan;
extern short gsn_sneak;
extern short gsn_steal;
extern short gsn_gouge;
extern short gsn_rush;
extern short gsn_track;
extern short gsn_stalk;
extern short gsn_search;
extern short gsn_dig;
extern short gsn_mount;
extern short gsn_bashdoor;
extern short gsn_dropkick;
extern short gsn_mace;
extern short gsn_pdust;
extern short gsn_berserk;
extern short gsn_hitall;
extern short gsn_pickshiplock;
extern short gsn_hijack;
extern short gsn_blackjack;
extern short gsn_slip;
extern short gsn_hack;
extern short gsn_meddle;
extern short gsn_codebreak;
extern short gsn_fry;
extern short gsn_sabotage;
extern short gsn_lore;
extern short gsn_phase;
extern short gsn_trap;
extern short gsn_gas;
extern short gsn_bear;
extern short gsn_acid;
extern short gsn_energynet;
extern short gsn_pdart;
extern short gsn_backlash;
extern short gsn_createdrug;
extern short gsn_pummel;
extern short gsn_assassinate;
//extern  short  gsn_voodoo;
extern short gsn_createvoodoo;
extern short gsn_catfight;
extern short gsn_hobo;
extern short gsn_pimp;
extern short gsn_drunken;
extern short gsn_cheapskate;
extern short gsn_style;
extern short gsn_counter;

extern short gsn_disarm;
extern short gsn_enhanced_damage;
extern short gsn_kick;
extern short gsn_parry;
extern short gsn_rescue;
extern short gsn_second_attack;
extern short gsn_third_attack;
extern short gsn_fourth_attack;
extern short gsn_fifth_attack;
extern short gsn_dual_wield;
extern short gsn_offhand;

extern short gsn_aid;

/* used to do specific lookups */
extern short gsn_first_spell;
extern short gsn_first_skill;
extern short gsn_first_weapon;
extern short gsn_first_tongue;
extern short gsn_top_sn;

/* spells */
extern short gsn_blindness;
extern short gsn_charm_person;
extern short gsn_aqua_breath;
extern short gsn_invis;
extern short gsn_mass_invis;
extern short gsn_poison;
extern short gsn_sleep;
extern short gsn_nemesis;
extern short gsn_possess;
extern short gsn_fireball; /* for fireshield  */
extern short gsn_lightning_bolt;   /* for shockshield */

/* newer attack skills */
extern short gsn_punch;
extern short gsn_bash;
extern short gsn_stun;
extern short gsn_bite;
extern short gsn_claw;

// Buster attack damage calls
extern short gsn_blasta;
extern short gsn_blastb;
extern short gsn_blastc;
extern short gsn_blastd;
extern short gsn_blaste;
extern short gsn_blastf;
extern short gsn_blastg;

extern short gsn_poison_weapon;
extern short gsn_climb;

extern short gsn_firearms;
extern short gsn_spears;
extern short gsn_bowandarrows;
extern short gsn_lightsabers;
extern short gsn_swords;
extern short gsn_flexible_arms;
extern short gsn_talonous_arms;
extern short gsn_bludgeons;

extern short gsn_grip;

/* Art GSN's */
extern short gsn_hyperelbow;
extern short gsn_chargingscorch;
extern short gsn_somersault;
extern short gsn_slashkick;
extern short gsn_powerpunch;
extern short gsn_crosskick;
extern short gsn_pyropummel;
extern short gsn_spincombo;
extern short gsn_pkcombo;
extern short gsn_hurricane;
extern short gsn_cyclone;
extern short gsn_tornadoflame;
extern short gsn_fireblow;
extern short gsn_burningflare;

extern short gsn_lizardtail;
extern short gsn_acrobaticblitz;
extern short gsn_sonicjavelin;
extern short gsn_blizzardbash;
extern short gsn_miragelancer;
extern short gsn_dolphinattack;
extern short gsn_birdstep;
extern short gsn_swandriver;
extern short gsn_rushinggale;
extern short gsn_tempestbreak;
extern short gsn_frostbreath;


/*
 * Utility macros.
 */
int umin( int check, int ncheck );
int umax( int check, int ncheck );
int urange( int mincheck, int check, int maxcheck );

#define UMIN( a, b )      ( umin( (a), (b) ) )
#define UMAX( a, b )      ( umax( (a), (b) ) )
#define URANGE(a, b, c )  ( urange( (a), (b), (c) ) )
#define LOWER( c )        ( (c) >= 'A' && (c) <= 'Z' ? (c) + 'a' - 'A' : (c) )
#define UPPER( c )        ( (c) >= 'a' && (c) <= 'z' ? (c) + 'A' - 'a' : (c) )

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define IS_EXTRA(ch, sn)	(IS_SET((ch)->extra, (sn)))
#define CH(d)			((d)->original ? (d)->original : (d)->character)
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = true)
#define INVALIDATE(data)	((data)->valid = false)

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

 /*
  * The functions for these prototypes can be found in misc.c
  * They are up here because they are used by the macros below
  */
bool ext_is_empty args( ( EXT_BV *bits ) );
void ext_clear_bits args( ( EXT_BV *bits ) );
int ext_has_bits args( ( EXT_BV *var, EXT_BV *bits ) );
bool ext_same_bits args( ( EXT_BV *var, EXT_BV *bits ) );
void ext_set_bits args( ( EXT_BV *var, EXT_BV *bits ) );
void ext_remove_bits args( ( EXT_BV *var, EXT_BV *bits ) );
void ext_toggle_bits args( ( EXT_BV *var, EXT_BV *bits ) );

/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/*
 * Macro to read in and ignore a keyword from a helpfile
 * to reduce the bug spam after removing features. --Halcyon
 */
#define DUMMYREAD( literal )						\
				if ( !str_cmp( word, literal ) )	\
				{									\
				    fMatch = true;					\
				    break;							\
				}

 /*
  * Memory allocation macros.
  */
#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }                                                                    \
} while(0)

#define RECREATE(result,type,number)                                    \
do                                                                      \
{                                                                       \
   if(!((result) = (type *)realloc((result), sizeof(type) * (number)))) \
   {                                                                    \
      perror("realloc failure");                                        \
      fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__); \
      abort();                                                          \
   }                                                                    \
} while(0)

#if defined(__FreeBSD__)
#define DISPOSE(point)                      \
do                                          \
{                                           \
   if( (point) )                            \
   {                                        \
      free( (void*) (point) );              \
      (point) = NULL;                       \
   }                                        \
} while(0)
#else
#define DISPOSE(point)                         \
do                                             \
{                                              \
   if( (point) )                               \
   {                                           \
      if( typeid((point)) == typeid(char*) || typeid((point)) == typeid(const char*) ) \
      {                                        \
         if( in_hash_table( (char*)(point) ) ) \
         {                                     \
            log_printf( "&RDISPOSE called on STRALLOC pointer: %s, line %d\n", __FILE__, __LINE__ ); \
            log_string( "Attempting to correct." ); \
            if( str_free( (char*)(point) ) == -1 ) \
               log_printf( "&RSTRFREEing bad pointer: %s, line %d\n", __FILE__, __LINE__ ); \
         }                                     \
         else                                  \
            free( (void*) (point) );           \
      }                                        \
      else                                     \
         free( (void*) (point) );              \
      (point) = NULL;                          \
   }                                           \
   else                                          \
      (point) = NULL;                            \
} while(0)
#endif

#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#if defined(__FreeBSD__)
#define STRFREE(point)                          \
do                                              \
{                                               \
   if((point))                                  \
   {                                            \
      if( str_free((point)) == -1 )             \
         bug( "&RSTRFREEing bad pointer: %s, line %d", __FILE__, __LINE__ ); \
      (point) = NULL;                           \
   }                                            \
} while(0)
#else
#define STRFREE(point)                           \
do                                               \
{                                                \
   if((point))                                   \
   {                                             \
      if( !in_hash_table( (point) ) )            \
      {                                          \
         log_printf( "&RSTRFREE called on str_dup pointer: %s, line %d\n", __FILE__, __LINE__ ); \
         log_string( "Attempting to correct." ); \
         free( (void*) (point) );                \
      }                                          \
      else if( str_free((point)) == -1 )         \
         log_printf( "&RSTRFREEing bad pointer: %s, line %d\n", __FILE__, __LINE__ ); \
      (point) = NULL;                            \
   }                                             \
   else                                          \
      (point) = NULL;                            \
} while(0)
#endif

  /* double-linked list handling macros -Thoric */
  /* Updated by Scion 8/6/1999 */
#define LINK(link, first, last, next, prev) \
do                                          \
{                                           \
   if ( !(first) )                          \
   {                                        \
      (first) = (link);                     \
      (last) = (link);                      \
   }                                        \
   else                                     \
      (last)->next = (link);                \
   (link)->next = NULL;                     \
   if ((first) == (link))                   \
      (link)->prev = NULL;                  \
   else                                     \
      (link)->prev = (last);                \
   (last) = (link);                         \
} while(0)

#define INSERT(link, insert, first, next, prev) \
do                                              \
{                                               \
   (link)->prev = (insert)->prev;               \
   if ( !(insert)->prev )                       \
      (first) = (link);                         \
   else                                         \
      (insert)->prev->next = (link);            \
   (insert)->prev = (link);                     \
   (link)->next = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)   \
do                                              \
{                                               \
   if ( !(link)->prev )                         \
   {                                            \
      (first) = (link)->next;                   \
      if ((first))                              \
         (first)->prev = NULL;                  \
   }                                            \
   else                                         \
   {                                            \
      (link)->prev->next = (link)->next;        \
   }                                            \
   if ( !(link)->next )                         \
   {                                            \
      (last) = (link)->prev;                    \
      if((last))                                \
         (last)->next = NULL;                   \
   }                                            \
   else                                         \
   {                                            \
      (link)->next->prev = (link)->prev;        \
   }                                            \
} while(0)

#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\r\n", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define GET_MAX_STR(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->str_plus ) )
 //    ( IS_SET((ch)->pcdata->cyber, CYBER_STRENGTH) ? 1 : 0 ) ) )
#define GET_MAX_DEX(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->dex_plus ) )
//    ( IS_SET((ch)->pcdata->cyber, CYBER_REFLEXES) ? 1 : 0 ) ) )
#define GET_MAX_INT(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->int_plus ) )
//    ( IS_SET((ch)->pcdata->cyber, CYBER_MIND) ? 1 : 0 ) ) )
#define GET_MAX_WIS(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->wis_plus ) )
#define GET_MAX_CON(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->con_plus ) )
#define GET_MAX_CHA(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->cha_plus ) )
#define GET_MAX_LCK(CH) ( IS_NPC((ch)) ? 20 : ( 20 + race_table[(ch)->race]->lck_plus ) )

#define IS_NPC(ch)		xIS_SET( (ch)->act, ACT_IS_NPC )
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_STAFF)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO)
#define IS_SUPREME(ch)         (get_trust((ch)) >= LEVEL_OWNER)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || !IS_SET((ch)->xflags, (part)))
#define GET_TIME_PLAYED(ch)     (((ch)->played + (current_time - (ch)->logon)) / 3600)

#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		( (ch)->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) \
				- ( (ch)->skill_level[COMBAT_ABILITY]/2 ) ) 
//                                + ( race_table[(ch)->race]->ac_plus ) ) ) )

#define GET_HITROLL(ch)		( (ch)->hitroll           		    \
				    +str_app[get_curr_str(ch)].tohit	    \
				    +(2-(abs((ch)->mental_state)/10)))
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +str_app[get_curr_str(ch)].todam	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(!xIS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS) && !xIS_SET(               \
				    (ch)->in_room->room_flags,              \
				    ROOM_SPACECRAFT) )

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    )

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait,(IS_IMMORTAL(ch) ? 0 : (npulse))))


#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->flags     ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->flags >> 3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->flags >> 6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->flags >> 9) & 3 )
#define SET_SDAM(skill, val)	( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata->auth_state <= 3  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

   /*
	* Object macros.
	*/
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(xIS_SET((obj)->extra_flags, (stat)))


	/*
	 * MudProg macros.						-Thoric
	 */
#define HAS_PROG(what, prog)	(xIS_SET((what)->progtypes, (prog)))

	 /*
	  * Description macros.
	  */
#define IS_IN_WAR(ch)   ( xIS_SET((ch)->in_room->room_flags, ROOM_ARENA))

#define CAN_ACCESS(room)        ( !xIS_SET(room->room_flags, ROOM_PRIVATE) \
                                &&   !xIS_SET(room->room_flags, ROOM_ARENA) )

#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
			        : (ch)->name ) : IS_IMMORTAL(ch) ?      \
			        "An Immortal" :         \
			        "Someone")


#define log_string( txt )	( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )
#define log_bug( txt )          ( log_bug_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )

	  /*
	   * Structure for a command in the command lookup table.
	   */
struct cmd_type
{
	CMDTYPE *next;
	const char *name;
	DO_FUN *do_fun;
	const char *fun_name;
	short position;
	short level;
	short log;
	struct timerset userec;
	short cshow;    /* if command is shown in command list - Zarius */

};



/*
 * Structure for a social in the socials table.
 */
struct social_type
{
	SOCIALTYPE *next;
	const char *name;
	const char *char_no_arg;
	const char *others_no_arg;
	const char *char_found;
	const char *others_found;
	const char *vict_found;
	const char *char_auto;
	const char *others_auto;
};

struct specfun_list
{
	SPEC_LIST *next;
	SPEC_LIST *prev;
	const char *name;
};

/*
 * Global constants.
 */

extern bool iswar;
extern bool hidefoldmessage;
extern int lo_level;
extern int hi_level;
extern int inwar;
extern int wartimeleft;
extern int wartimer;
extern int arenatype;

extern time_t last_restore_all_time;
extern time_t boot_time;    /* this should be moved down */
extern HOUR_MIN_SEC *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;
extern bool mud_down;
extern bool DONT_UPPER;
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

extern const struct str_app_type str_app[26];
extern const struct int_app_type int_app[26];
extern const struct wis_app_type wis_app[26];
extern const struct dex_app_type dex_app[26];
extern const struct con_app_type con_app[26];
extern const struct cha_app_type cha_app[26];
extern const struct lck_app_type lck_app[26];

extern const struct race_type race_table[MAX_RACE];
extern const struct suit_type suitweapon_table[MAX_SUITWEAPON];
extern const struct liq_type liq_table[LIQ_MAX];
extern const char *const attack_table[14];
extern const char *const ability_name[MAX_ABILITY];
extern const char *const special_weapons[MAX_SWEAPON];
extern const char *const curse_table[];
extern const char *const ignore_table[];


extern const char *const skill_tname[];
extern short const movement_loss[SECT_MAX];
extern const char *const dir_name[];
extern const char *const where_name[];
extern const short rev_dir[];
extern const int trap_door[];
extern const char *const r_flags[];
extern const char *const r_flags2[];
extern const char *const w_flags[];
extern const char *const o_flags[];
extern const char *const a_flags[];
extern const char *const o_types[];
extern const char *const a_types[];
extern const char *const act_flags[];
extern const char *const planet_flags[];
extern const char *const weapon_table[13];
extern const char *const spice_table[];
extern const char *const plr_flags[];
extern const char *const pc_flags[];
extern const char *const cyba_flags[];
extern const char *const trap_flags[];
extern const char *const ris_flags[];
extern const char *const trig_flags[];
extern const char *const part_flags[];
extern const char *const npc_race[];
extern const char *const pc_race[];
extern const char *const suitweapons[];
extern const char *const element_table[];
extern const char *const defense_flags[];
extern const char *const attack_flags[];
extern const char *const area_flags[];
extern const char *const npc_position[];
extern const char *const wear_locs[];
extern const char *const hair_list[];
extern const char *const eye_list[];
extern const char *const build_list[];
extern const char *const hero_list[];
extern const char *const highlight_list[];
extern const char *const suitweapon_type[];
extern const struct clan_titles clan_rank_table[];
extern const char *const cargo_names[CTYPE_MAX];

extern int const lang_array[];
extern const char *const lang_names[];

/*
 * Global variables.
 */
extern int numobjsloaded;
extern int nummobsloaded;
extern int physicalobjects;
extern int num_descriptors;
extern struct system_data sysdata;
extern int top_sn;
extern int top_vroom;
extern int top_herb;
extern bool lotto;

extern MPSLEEP_DATA *first_mpwait;  /* Storing sleeping mud progs */
extern MPSLEEP_DATA *last_mpwait;   /* - */
extern MPSLEEP_DATA *current_mpwait;    /* - *//* mpsleep snippet - Samson 6-1-99 */

extern CMDTYPE *command_hash[126];

extern SKILLTYPE *skill_table[MAX_SKILL];
extern SOCIALTYPE *social_index[27];
extern CHAR_DATA *cur_char;
extern ROOM_INDEX_DATA *cur_room;
extern bool cur_char_died;
extern ch_ret global_retcode;
extern SKILLTYPE *herb_table[MAX_HERB];

extern int cur_obj;
extern int cur_obj_serial;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;

extern bool mud_down;
extern HELP_DATA *first_help;
extern HELP_DATA *last_help;
extern SHOP_DATA *first_shop;
extern SHOP_DATA *last_shop;
extern REPAIR_DATA *first_repair;
extern REPAIR_DATA *last_repair;
extern SPEC_LIST *first_specfun;
extern SPEC_LIST *last_specfun;
extern BAN_DATA *first_ban;
extern BAN_DATA *last_ban;
extern RESERVE_DATA *first_reserved;
extern RESERVE_DATA *last_reserved;
extern MNAME_DATA *first_mname;
extern MNAME_DATA *last_mname;
extern FNAME_DATA *first_fname;
extern FNAME_DATA *last_fname;
extern CHAR_DATA *first_char;
extern CHAR_DATA *last_char;
extern DESCRIPTOR_DATA *first_descriptor;
extern DESCRIPTOR_DATA *last_descriptor;
extern BOARD_DATA *first_board;
extern BOARD_DATA *last_board;
extern OBJ_DATA *first_object;
extern OBJ_DATA *last_object;
extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;
extern GUARD_DATA *first_guard;
extern GUARD_DATA *last_guard;
extern SHIP_DATA *first_ship;
extern SHIP_DATA *last_ship;
extern SPACE_DATA *first_starsystem;
extern SPACE_DATA *last_starsystem;
extern PLANET_DATA *first_planet;
extern PLANET_DATA *last_planet;
extern SENATE_DATA *first_senator;
extern SENATE_DATA *last_senator;
extern BOUNTY_DATA *first_bounty;
extern BOUNTY_DATA *last_bounty;
extern BOUNTY_DATA *first_disintigration;
extern BOUNTY_DATA *last_disintigration;
extern AREA_DATA *first_area;
extern AREA_DATA *last_area;
extern AREA_DATA *first_build;
extern AREA_DATA *last_build;
extern AREA_DATA *first_asort;
extern AREA_DATA *last_asort;
extern AREA_DATA *first_bsort;
extern AREA_DATA *last_bsort;
/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern TELEPORT_DATA *first_teleport;
extern TELEPORT_DATA *last_teleport;
extern OBJ_DATA *extracted_obj_queue;
extern EXTRACT_CHAR_DATA *extracted_char_queue;
extern OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
extern CHAR_DATA *quitting_char;
extern CHAR_DATA *loading_char;
extern CHAR_DATA *saving_char;
extern OBJ_DATA *all_obj;

extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpLOG;
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;
extern AUCTION_DATA *auction_list;
extern char last_command[MAX_STRING_LENGTH];
extern WAR_DATA war_info;
extern int weath_unit;
extern TAG_DATA tag_game;

extern AUCTION_DATA *auction;
extern struct act_prog_data *mob_act_list;
extern IMMORTAL_HOST *immortal_host_start;
extern IMMORTAL_HOST *immortal_host_end;

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */

DECLARE_DO_FUN( do_pcrename );
DECLARE_DO_FUN( do_speed );
DECLARE_DO_FUN( do_testground );
DECLARE_DO_FUN( do_logsearch );
DECLARE_DO_FUN( do_awho );
DECLARE_DO_FUN( do_evade );
DECLARE_DO_FUN( do_wizslap );
DECLARE_DO_FUN( do_ignore );
DECLARE_DO_FUN( do_upgradesuit );
DECLARE_DO_FUN( do_remember );
DECLARE_DO_FUN( do_rub );
DECLARE_DO_FUN( do_forget );
DECLARE_DO_FUN( do_storage );
DECLARE_DO_FUN( do_startgang );
DECLARE_DO_FUN( do_store );
DECLARE_DO_FUN( do_buystorage );
DECLARE_DO_FUN( do_makestorage );
DECLARE_DO_FUN( do_sellhome );
DECLARE_DO_FUN( do_freeships );
DECLARE_DO_FUN( do_two );
DECLARE_DO_FUN( do_twit );
DECLARE_DO_FUN( do_pardon );
DECLARE_DO_FUN( do_paratroopers );
DECLARE_DO_FUN( do_voodoo );
DECLARE_DO_FUN( do_note );
DECLARE_DO_FUN( do_newspaper );
DECLARE_DO_FUN( do_idea );
DECLARE_DO_FUN( do_news );
DECLARE_DO_FUN( do_penalty );
DECLARE_DO_FUN( do_weddings );
DECLARE_DO_FUN( do_changes );
DECLARE_DO_FUN( do_createvoodoo );
DECLARE_DO_FUN( do_charge );
DECLARE_DO_FUN( do_vload );
DECLARE_DO_FUN( do_startwar );
DECLARE_DO_FUN( do_arenawar );
DECLARE_DO_FUN( do_finger );
DECLARE_DO_FUN( do_slot );
DECLARE_DO_FUN( do_alias );
DECLARE_DO_FUN( do_taskfail );
DECLARE_DO_FUN( do_reserve );
DECLARE_DO_FUN( do_style );
DECLARE_DO_FUN( do_promote );
DECLARE_DO_FUN( do_award );
DECLARE_DO_FUN( do_lore );
DECLARE_DO_FUN( do_phase );
DECLARE_DO_FUN( do_engage );
DECLARE_DO_FUN( do_entex );
DECLARE_DO_FUN( do_camset );
DECLARE_DO_FUN( do_renumber );
DECLARE_DO_FUN( do_run );
DECLARE_DO_FUN( do_attack );
DECLARE_DO_FUN( do_spirit );
DECLARE_DO_FUN( do_high );
DECLARE_DO_FUN( do_right );
DECLARE_DO_FUN( do_left );
DECLARE_DO_FUN( do_hiscoset );  /* cronel hiscores */
DECLARE_DO_FUN( do_hiscore );   /* cronel hiscores */
DECLARE_DO_FUN( do_pissoff );
DECLARE_DO_FUN( do_spacemine );
DECLARE_DO_FUN( do_replay );
DECLARE_DO_FUN( do_compress );
DECLARE_DO_FUN( do_counter );
DECLARE_DO_FUN( do_appset );
DECLARE_DO_FUN( do_stalk );
DECLARE_DO_FUN( do_lottery );
DECLARE_DO_FUN( do_rage );
DECLARE_DO_FUN( do_ucargo );
DECLARE_DO_FUN( do_cargo );
DECLARE_DO_FUN( do_imports );
DECLARE_DO_FUN( do_artifacts );
DECLARE_DO_FUN( do_shoot );
DECLARE_DO_FUN( do_sheath );
DECLARE_DO_FUN( do_draw );
DECLARE_DO_FUN( do_add_imm_host );
DECLARE_DO_FUN( do_hover );
DECLARE_DO_FUN( do_createdrug );
DECLARE_DO_FUN( do_rmaffect );
DECLARE_DO_FUN( do_trap );
DECLARE_DO_FUN( do_seize );
DECLARE_DO_FUN( do_fry );
DECLARE_DO_FUN( do_dropkick );
DECLARE_DO_FUN( do_mace );
DECLARE_DO_FUN( do_pdust );
DECLARE_DO_FUN( do_ftag );
DECLARE_DO_FUN( do_tag );
DECLARE_DO_FUN( do_red );
DECLARE_DO_FUN( do_blue );
//DECLARE_DO_FUN( do_replay       );
//DECLARE_DO_FUN( do_punish       );
DECLARE_DO_FUN( do_noooc );
DECLARE_DO_FUN( do_aexit );
DECLARE_DO_FUN( do_whisper );
DECLARE_DO_FUN( do_unread );
DECLARE_DO_FUN( do_checkskills );
DECLARE_DO_FUN( do_buzz );
DECLARE_DO_FUN( do_input );
DECLARE_DO_FUN( do_invite );
DECLARE_DO_FUN( do_inform );
DECLARE_DO_FUN( do_convert );
DECLARE_DO_FUN( do_convertap );
DECLARE_DO_FUN( do_awardap );
DECLARE_DO_FUN( do_multicheck );
DECLARE_DO_FUN( do_war );
DECLARE_DO_FUN( do_dnd );
DECLARE_DO_FUN( do_transform );
DECLARE_DO_FUN( do_zerosystem );
DECLARE_DO_FUN( do_addsalary );
DECLARE_DO_FUN( do_spunch );
DECLARE_DO_FUN( do_targetlock );
DECLARE_DO_FUN( do_lagout );
DECLARE_DO_FUN( do_customize );
DECLARE_DO_FUN( do_makeimm );
DECLARE_DO_FUN( do_agree );
DECLARE_DO_FUN( do_breakup );
DECLARE_DO_FUN( do_consent );
DECLARE_DO_FUN( do_divorce );
DECLARE_DO_FUN( do_xbuster );
DECLARE_DO_FUN( do_marry );
DECLARE_DO_FUN( do_propose );
DECLARE_DO_FUN( do_askout );
DECLARE_DO_FUN( do_decline );
DECLARE_DO_FUN( do_clearwed );
DECLARE_DO_FUN( do_accept );
DECLARE_DO_FUN( do_selfdestruct );
DECLARE_DO_FUN( do_ltarget );
DECLARE_DO_FUN( do_setplanet );
DECLARE_DO_FUN( do_setguests );
DECLARE_DO_FUN( do_makeplanet );
DECLARE_DO_FUN( do_planets );
DECLARE_DO_FUN( do_teach );
DECLARE_DO_FUN( do_gather_intelligence );
DECLARE_DO_FUN( do_guard );
DECLARE_DO_FUN( do_add_patrol );
DECLARE_DO_FUN( do_special_forces );
DECLARE_DO_FUN( do_jail );
DECLARE_DO_FUN( do_elite_guard );
DECLARE_DO_FUN( do_smalltalk );
DECLARE_DO_FUN( do_propeganda );
DECLARE_DO_FUN( do_bribe );
DECLARE_DO_FUN( do_seduce );
DECLARE_DO_FUN( do_mass_propeganda );
DECLARE_DO_FUN( do_copyship );
DECLARE_DO_FUN( do_sound );
DECLARE_DO_FUN( do_autopilot );
DECLARE_DO_FUN( do_allspeeders );
DECLARE_DO_FUN( do_speeders );
DECLARE_DO_FUN( do_suicide );
DECLARE_DO_FUN( do_gain );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_beg );
DECLARE_DO_FUN( do_bank );
DECLARE_DO_FUN( do_helpcheck );
DECLARE_DO_FUN( do_hijack );
DECLARE_DO_FUN( do_pickshiplock );
DECLARE_DO_FUN( do_shiptalk );
DECLARE_DO_FUN( do_clone );
DECLARE_DO_FUN( do_systemtalk );
DECLARE_DO_FUN( do_spacetalk );
DECLARE_DO_FUN( do_hail );
DECLARE_DO_FUN( do_allships );
DECLARE_DO_FUN( do_newclan );
DECLARE_DO_FUN( do_appoint );
DECLARE_DO_FUN( do_demote );
DECLARE_DO_FUN( do_empower );
DECLARE_DO_FUN( do_capture );
DECLARE_DO_FUN( do_arts );
DECLARE_DO_FUN( do_arm );
DECLARE_DO_FUN( do_pkchange );
DECLARE_DO_FUN( do_fakequit );
DECLARE_DO_FUN( do_firstweapon );
DECLARE_DO_FUN( do_secondweapon );
DECLARE_DO_FUN( do_thirdweapon );
DECLARE_DO_FUN( do_i100machine );
DECLARE_DO_FUN( do_i105rifle );
DECLARE_DO_FUN( do_leobazooka );
DECLARE_DO_FUN( do_shouldercannon );
DECLARE_DO_FUN( do_sidemissilelauncher );
DECLARE_DO_FUN( do_blackjack );
DECLARE_DO_FUN( do_makecamera );
DECLARE_DO_FUN( do_sabotage );
DECLARE_DO_FUN( do_slip );
DECLARE_DO_FUN( do_hack );
DECLARE_DO_FUN( do_meddle );
DECLARE_DO_FUN( do_codebreak );
DECLARE_DO_FUN( do_beamrifle );
DECLARE_DO_FUN( do_lasercannon );
DECLARE_DO_FUN( do_twingattling );
DECLARE_DO_FUN( do_beamcannon );
DECLARE_DO_FUN( do_i250machine );
DECLARE_DO_FUN( do_chaff );
DECLARE_DO_FUN( do_clan_donate );
DECLARE_DO_FUN( do_clan_withdraw );
DECLARE_DO_FUN( do_clanstat );
DECLARE_DO_FUN( do_fly );
DECLARE_DO_FUN( do_drive );
DECLARE_DO_FUN( do_bomb );
DECLARE_DO_FUN( do_setblaster );
DECLARE_DO_FUN( do_ammo );
DECLARE_DO_FUN( do_takedrug );
DECLARE_DO_FUN( do_use );
DECLARE_DO_FUN( do_enlist );
DECLARE_DO_FUN( do_resign );
DECLARE_DO_FUN( do_pluogus );
DECLARE_DO_FUN( do_tractorbeam );
DECLARE_DO_FUN( do_minigun );
DECLARE_DO_FUN( do_chaingun );
DECLARE_DO_FUN( do_beamsabre );
DECLARE_DO_FUN( do_lasergun );
DECLARE_DO_FUN( do_transship );
DECLARE_DO_FUN( do_makearmor );
DECLARE_DO_FUN( do_makejewelry );
DECLARE_DO_FUN( do_makegrenade );
DECLARE_DO_FUN( do_makelandmine );
DECLARE_DO_FUN( do_makelight );
DECLARE_DO_FUN( do_makecomlink );
DECLARE_DO_FUN( do_makeshield );
DECLARE_DO_FUN( do_makecontainer );
DECLARE_DO_FUN( do_sharpen );
DECLARE_DO_FUN( do_makesweapon );
DECLARE_DO_FUN( do_makemissile );
DECLARE_DO_FUN( do_gemcutting );
DECLARE_DO_FUN( do_reinforcements );
DECLARE_DO_FUN( do_postguard );
DECLARE_DO_FUN( do_torture );
DECLARE_DO_FUN( do_snipe );
DECLARE_DO_FUN( do_throw );
DECLARE_DO_FUN( do_pipe );
DECLARE_DO_FUN( do_disguise );
DECLARE_DO_FUN( do_mine );
DECLARE_DO_FUN( do_first_aid );
DECLARE_DO_FUN( do_makeblade );
DECLARE_DO_FUN( do_makeblaster );
DECLARE_DO_FUN( do_makelightsaber );
DECLARE_DO_FUN( do_makespice );
DECLARE_DO_FUN( do_closebay );
DECLARE_DO_FUN( do_openbay );
DECLARE_DO_FUN( do_one );
DECLARE_DO_FUN( do_request );
DECLARE_DO_FUN( do_rembounty );
DECLARE_DO_FUN( do_remmember );
DECLARE_DO_FUN( do_autotrack );
DECLARE_DO_FUN( do_jumpvector );
DECLARE_DO_FUN( do_reload );
//DECLARE_DO_FUN( do_tractorbeam ); 
DECLARE_DO_FUN( do_radar );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_buyship );
DECLARE_DO_FUN( do_buyhome );
DECLARE_DO_FUN( do_buyposse );
DECLARE_DO_FUN( do_clanbuyship );
DECLARE_DO_FUN( do_sellship );
DECLARE_DO_FUN( do_autorecharge );
DECLARE_DO_FUN( do_openhatch );
DECLARE_DO_FUN( do_closehatch );
DECLARE_DO_FUN( do_status );
DECLARE_DO_FUN( do_info );
DECLARE_DO_FUN( do_hyperspace );
DECLARE_DO_FUN( do_target );
DECLARE_DO_FUN( do_fire );
DECLARE_DO_FUN( do_calculate );
DECLARE_DO_FUN( do_recharge );
DECLARE_DO_FUN( do_refuel );
DECLARE_DO_FUN( do_addpilot );
DECLARE_DO_FUN( do_rempilot );
DECLARE_DO_FUN( do_trajectory );
DECLARE_DO_FUN( do_accelerate );
DECLARE_DO_FUN( do_launch );
DECLARE_DO_FUN( do_land );
DECLARE_DO_FUN( do_delete );
DECLARE_DO_FUN( do_leaveship );
DECLARE_DO_FUN( do_setstarsystem );
DECLARE_DO_FUN( do_makestarsystem );
DECLARE_DO_FUN( do_starsystems );
DECLARE_DO_FUN( do_showstarsystem );
DECLARE_DO_FUN( skill_notfound );
DECLARE_DO_FUN( do_abit );
DECLARE_DO_FUN( do_aassign );
DECLARE_DO_FUN( do_aquest );
DECLARE_DO_FUN( do_addbounty );
DECLARE_DO_FUN( do_vassign );
DECLARE_DO_FUN( do_rassign );
DECLARE_DO_FUN( do_massign );
DECLARE_DO_FUN( do_oassign );
DECLARE_DO_FUN( do_advance );
DECLARE_DO_FUN( do_affected );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_aid );
DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_ansi );
DECLARE_DO_FUN( do_answer );
DECLARE_DO_FUN( do_apply );
DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_aset );
DECLARE_DO_FUN( do_ask );
DECLARE_DO_FUN( do_astat );
DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_auction );
DECLARE_DO_FUN( do_authorize );
DECLARE_DO_FUN( do_avtalk );
DECLARE_DO_FUN( do_assassinate );
DECLARE_DO_FUN( do_backstab );
DECLARE_DO_FUN( do_balzhur );
DECLARE_DO_FUN( do_bamfin );
DECLARE_DO_FUN( do_bamfout );
DECLARE_DO_FUN( do_ban );
DECLARE_DO_FUN( do_banswer );
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_bashdoor );
DECLARE_DO_FUN( do_beatdown );
DECLARE_DO_FUN( do_beep );
DECLARE_DO_FUN( do_berserk );
DECLARE_DO_FUN( do_bestow );
DECLARE_DO_FUN( do_bestowarea );
DECLARE_DO_FUN( do_bio );
DECLARE_DO_FUN( do_bite );
DECLARE_DO_FUN( do_board );
DECLARE_DO_FUN( do_boards );
DECLARE_DO_FUN( do_bodybag );
DECLARE_DO_FUN( do_bounties );
DECLARE_DO_FUN( do_brandish );
DECLARE_DO_FUN( do_brew );
DECLARE_DO_FUN( do_bset );
DECLARE_DO_FUN( do_bstat );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_bury );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_cast );
DECLARE_DO_FUN( do_cedit );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_chat );
DECLARE_DO_FUN( do_ooc );
DECLARE_DO_FUN( do_check_vnums );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_strangle );
DECLARE_DO_FUN( do_clans );
DECLARE_DO_FUN( do_ships );
DECLARE_DO_FUN( do_clantalk );
DECLARE_DO_FUN( do_claw );
DECLARE_DO_FUN( do_climb );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_cmdtable );
DECLARE_DO_FUN( do_cmenu );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_comment );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_config );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_senate );
DECLARE_DO_FUN( do_addsenator );
DECLARE_DO_FUN( do_remsenator );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_cset );
DECLARE_DO_FUN( do_deities );
DECLARE_DO_FUN( do_deny );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_destro );
DECLARE_DO_FUN( do_pldestroy );
DECLARE_DO_FUN( do_detrap );
DECLARE_DO_FUN( do_devote );
DECLARE_DO_FUN( do_dig );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_disconnect );
DECLARE_DO_FUN( do_dismount );
DECLARE_DO_FUN( do_dmesg );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_drag );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_diagnose );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_empty );
DECLARE_DO_FUN( do_enter );
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_feed );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_fixchar );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_foldarea );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_for );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_forceclose );
DECLARE_DO_FUN( do_fquit ); /* Gorog */
DECLARE_DO_FUN( do_form_password );
DECLARE_DO_FUN( do_freeze );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_gmote );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_glance );
DECLARE_DO_FUN( do_gocial );
DECLARE_DO_FUN( do_gold );
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_gouge );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_grub );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_guilds );
DECLARE_DO_FUN( do_guildtalk );
DECLARE_DO_FUN( do_hedit );
DECLARE_DO_FUN( do_hell );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_hide );
DECLARE_DO_FUN( do_hitall );
DECLARE_DO_FUN( do_hlist );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_homepage );
DECLARE_DO_FUN( do_hset );
DECLARE_DO_FUN( do_i103 );
DECLARE_DO_FUN( do_i104 );
DECLARE_DO_FUN( do_i105 );
DECLARE_DO_FUN( do_ide );
DECLARE_DO_FUN( do_immortalize );
DECLARE_DO_FUN( do_immtalk );
DECLARE_DO_FUN( do_initiate );
DECLARE_DO_FUN( do_installarea );
DECLARE_DO_FUN( do_instaroom );
DECLARE_DO_FUN( do_instazone );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_kill );
DECLARE_DO_FUN( do_last );
DECLARE_DO_FUN( do_leave );
DECLARE_DO_FUN( do_level );
DECLARE_DO_FUN( do_light );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_litterbug );
//DECLARE_DO_FUN( do_loadarea );
DECLARE_DO_FUN( do_loadup );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_log );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_low );
DECLARE_DO_FUN( do_low_purge );
DECLARE_DO_FUN( do_mailroom );
DECLARE_DO_FUN( do_make );
DECLARE_DO_FUN( do_makeboard );
DECLARE_DO_FUN( do_makeclan );
DECLARE_DO_FUN( do_makeship );
DECLARE_DO_FUN( do_makeguild );
DECLARE_DO_FUN( do_makeshop );
DECLARE_DO_FUN( do_makewizlist );
DECLARE_DO_FUN( do_members );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_mcreate );
DECLARE_DO_FUN( do_mdelete );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_minvoke );
DECLARE_DO_FUN( do_mlist );
DECLARE_DO_FUN( do_mount );
DECLARE_DO_FUN( do_move );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_murde );
DECLARE_DO_FUN( do_murder );
DECLARE_DO_FUN( do_music );
DECLARE_DO_FUN( do_mwhere );
DECLARE_DO_FUN( do_name );
DECLARE_DO_FUN( do_newbiechat );
DECLARE_DO_FUN( do_newbieset );
DECLARE_DO_FUN( do_newzones );
DECLARE_DO_FUN( do_noemote );
DECLARE_DO_FUN( do_noresolve );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_northeast );
DECLARE_DO_FUN( do_northwest );
DECLARE_DO_FUN( do_notell );
DECLARE_DO_FUN( do_notitle );
DECLARE_DO_FUN( do_noteroom );
DECLARE_DO_FUN( do_ocreate );
DECLARE_DO_FUN( do_odelete );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_ogrub );
DECLARE_DO_FUN( do_oinvoke );
DECLARE_DO_FUN( do_oldscore );
DECLARE_DO_FUN( do_olist );
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_opentourney );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_orders );
DECLARE_DO_FUN( do_ordertalk );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_ot );
DECLARE_DO_FUN( do_outcast );
DECLARE_DO_FUN( do_owhere );
DECLARE_DO_FUN( do_pager );
//DECLARE_DO_FUN(   do_pardon   );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_pktalk );
DECLARE_DO_FUN( do_poison_weapon );
DECLARE_DO_FUN( do_pose );
//DECLARE_DO_FUN( do_ppktalk );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_pull );
DECLARE_DO_FUN( do_punch );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_push );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_qbit );
DECLARE_DO_FUN( do_qpset );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_quest );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_rank );
DECLARE_DO_FUN( do_rat );
DECLARE_DO_FUN( do_rdelete );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
//DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_recite );
DECLARE_DO_FUN( do_redit );
DECLARE_DO_FUN( do_regoto );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_rent );
DECLARE_DO_FUN( do_reply );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_bet );
DECLARE_DO_FUN( do_uncover );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_reset );
DECLARE_DO_FUN( do_resetship );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_restoretime );
DECLARE_DO_FUN( do_restrict );
DECLARE_DO_FUN( do_retire );
DECLARE_DO_FUN( do_retran );
DECLARE_DO_FUN( do_retreat );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_revert );
DECLARE_DO_FUN( do_rip );
DECLARE_DO_FUN( do_rlist );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_rush );
DECLARE_DO_FUN( do_sacrifice );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_savearea );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_search );
DECLARE_DO_FUN( do_sedit );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_set_boot_time );
DECLARE_DO_FUN( do_setclan );
DECLARE_DO_FUN( do_setabit );
DECLARE_DO_FUN( do_setqbit );
DECLARE_DO_FUN( do_setship );
DECLARE_DO_FUN( do_shops );
DECLARE_DO_FUN( do_shopset );
DECLARE_DO_FUN( do_shopstat );
DECLARE_DO_FUN( do_shout );
DECLARE_DO_FUN( do_shove );
DECLARE_DO_FUN( do_showabit );
DECLARE_DO_FUN( do_showqbit );
DECLARE_DO_FUN( do_showclan );
DECLARE_DO_FUN( do_showship );
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_silence );
DECLARE_DO_FUN( do_sit );
DECLARE_DO_FUN( do_sla );
DECLARE_DO_FUN( do_slay );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_slice );
DECLARE_DO_FUN( do_slist );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_smoke );
DECLARE_DO_FUN( do_sneak );
DECLARE_DO_FUN( do_snoop );
DECLARE_DO_FUN( do_sober );
DECLARE_DO_FUN( do_socials );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_southeast );
DECLARE_DO_FUN( do_southwest );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_starttourney );
DECLARE_DO_FUN( do_steal );
DECLARE_DO_FUN( do_sting );
DECLARE_DO_FUN( do_stun );
DECLARE_DO_FUN( do_supplicate );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_tail );
DECLARE_DO_FUN( do_tamp );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_timecmd );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_track );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_tune );
DECLARE_DO_FUN( do_trust );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_unfoldarea );
DECLARE_DO_FUN( do_unhell );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_unsilence );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_users );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_visible );
DECLARE_DO_FUN( do_vnums );
DECLARE_DO_FUN( do_vsearch );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_wartalk );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_wset );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_whois );
DECLARE_DO_FUN( do_wimpy );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_wizlist );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_wlookup );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_zap );
DECLARE_DO_FUN( do_zones );

/* mob prog stuff */
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_damage );
DECLARE_DO_FUN( do_mp_restore );
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_practice );
DECLARE_DO_FUN( do_mp_slay );
DECLARE_DO_FUN( do_mpadvance );
DECLARE_DO_FUN( do_mpasound );
DECLARE_DO_FUN( do_mpat );
DECLARE_DO_FUN( do_mpaset );
DECLARE_DO_FUN( do_mpqset );
DECLARE_DO_FUN( do_mpdream );
DECLARE_DO_FUN( do_mp_deposit );
DECLARE_DO_FUN( do_mp_withdraw );
DECLARE_DO_FUN( do_mpecho );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat );
DECLARE_DO_FUN( do_mpedit );
DECLARE_DO_FUN( do_mrange );
DECLARE_DO_FUN( do_opedit );
DECLARE_DO_FUN( do_orange );
DECLARE_DO_FUN( do_rpedit );
DECLARE_DO_FUN( do_mpforce );
DECLARE_DO_FUN( do_mpdelay );
DECLARE_DO_FUN( do_mpinvis );
DECLARE_DO_FUN( do_mpgoto );
DECLARE_DO_FUN( do_mpjunk );
DECLARE_DO_FUN( do_mpkill );
DECLARE_DO_FUN( do_mpmload );
DECLARE_DO_FUN( do_mpmset );
DECLARE_DO_FUN( do_mpnothing );
DECLARE_DO_FUN( do_mpoload );
DECLARE_DO_FUN( do_mposet );
DECLARE_DO_FUN( do_mppurge );
DECLARE_DO_FUN( do_mpstat );
DECLARE_DO_FUN( do_opstat );
DECLARE_DO_FUN( do_rpstat );
DECLARE_DO_FUN( do_mptransfer );
DECLARE_DO_FUN( do_mpapply );
DECLARE_DO_FUN( do_mpapplyb );
DECLARE_DO_FUN( do_mppkset );
DECLARE_DO_FUN( do_mpgain );
//DECLARE_DO_FUN( do_accept       );
DECLARE_DO_FUN( do_koolboot );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN( spell_null );
DECLARE_SPELL_FUN( spell_notfound );
DECLARE_SPELL_FUN( spell_acid_blast );
DECLARE_SPELL_FUN( spell_animate_dead );
DECLARE_SPELL_FUN( spell_astral_walk );
DECLARE_SPELL_FUN( spell_blindness );
DECLARE_SPELL_FUN( spell_burning_hands );
DECLARE_SPELL_FUN( spell_call_lightning );
DECLARE_SPELL_FUN( spell_cause_critical );
DECLARE_SPELL_FUN( spell_cause_light );
DECLARE_SPELL_FUN( spell_cause_serious );
DECLARE_SPELL_FUN( spell_change_sex );
DECLARE_SPELL_FUN( spell_charm_person );
DECLARE_SPELL_FUN( spell_chill_touch );
DECLARE_SPELL_FUN( spell_colour_spray );
DECLARE_SPELL_FUN( spell_control_weather );
DECLARE_SPELL_FUN( spell_create_food );
DECLARE_SPELL_FUN( spell_create_water );
DECLARE_SPELL_FUN( spell_cure_blindness );
DECLARE_SPELL_FUN( spell_cure_poison );
DECLARE_SPELL_FUN( spell_curse );
DECLARE_SPELL_FUN( spell_detect_poison );
DECLARE_SPELL_FUN( spell_dispel_evil );
DECLARE_SPELL_FUN( spell_dispel_magic );
DECLARE_SPELL_FUN( spell_dream );
DECLARE_SPELL_FUN( spell_earthquake );
DECLARE_SPELL_FUN( spell_enchant_weapon );
DECLARE_SPELL_FUN( spell_energy_drain );
DECLARE_SPELL_FUN( spell_faerie_fire );
DECLARE_SPELL_FUN( spell_faerie_fog );
DECLARE_SPELL_FUN( spell_farsight );
DECLARE_SPELL_FUN( spell_fireball );
DECLARE_SPELL_FUN( spell_flamestrike );
DECLARE_SPELL_FUN( spell_gate );
DECLARE_SPELL_FUN( spell_knock );
DECLARE_SPELL_FUN( spell_harm );
DECLARE_SPELL_FUN( spell_identify );
DECLARE_SPELL_FUN( spell_invis );
DECLARE_SPELL_FUN( spell_know_alignment );
DECLARE_SPELL_FUN( spell_lightning_bolt );
DECLARE_SPELL_FUN( spell_locate_object );
DECLARE_SPELL_FUN( spell_magic_missile );
DECLARE_SPELL_FUN( spell_mist_walk );
DECLARE_SPELL_FUN( spell_pass_door );
DECLARE_SPELL_FUN( spell_plant_pass );
DECLARE_SPELL_FUN( spell_poison );
DECLARE_SPELL_FUN( spell_polymorph );
DECLARE_SPELL_FUN( spell_possess );
DECLARE_SPELL_FUN( spell_recharge );
DECLARE_SPELL_FUN( spell_remove_curse );
DECLARE_SPELL_FUN( spell_remove_invis );
DECLARE_SPELL_FUN( spell_remove_trap );
DECLARE_SPELL_FUN( spell_shocking_grasp );
DECLARE_SPELL_FUN( spell_sleep );
DECLARE_SPELL_FUN( spell_smaug );
DECLARE_SPELL_FUN( spell_solar_flight );
DECLARE_SPELL_FUN( spell_summon );
DECLARE_SPELL_FUN( spell_teleport );
DECLARE_SPELL_FUN( spell_ventriloquate );
DECLARE_SPELL_FUN( spell_weaken );
DECLARE_SPELL_FUN( spell_word_of_recall );
DECLARE_SPELL_FUN( spell_acid_breath );
DECLARE_SPELL_FUN( spell_fire_breath );
DECLARE_SPELL_FUN( spell_frost_breath );
DECLARE_SPELL_FUN( spell_gas_breath );
DECLARE_SPELL_FUN( spell_lightning_breath );
DECLARE_SPELL_FUN( spell_spiral_blast );
DECLARE_SPELL_FUN( spell_scorching_surge );
DECLARE_SPELL_FUN( spell_helical_flow );
DECLARE_SPELL_FUN( spell_transport );
DECLARE_SPELL_FUN( spell_portal );

DECLARE_SPELL_FUN( spell_ethereal_fist );
DECLARE_SPELL_FUN( spell_spectral_furor );
DECLARE_SPELL_FUN( spell_hand_of_chaos );
DECLARE_SPELL_FUN( spell_disruption );
DECLARE_SPELL_FUN( spell_sonic_resonance );
DECLARE_SPELL_FUN( spell_mind_wrack );
DECLARE_SPELL_FUN( spell_mind_wrench );
DECLARE_SPELL_FUN( spell_revive );
DECLARE_SPELL_FUN( spell_sulfurous_spray );
DECLARE_SPELL_FUN( spell_caustic_fount );
DECLARE_SPELL_FUN( spell_acetum_primus );
DECLARE_SPELL_FUN( spell_galvanic_whip );
DECLARE_SPELL_FUN( spell_magnetic_thrust );
DECLARE_SPELL_FUN( spell_quantum_spike );
DECLARE_SPELL_FUN( spell_black_hand );
DECLARE_SPELL_FUN( spell_black_fist );
DECLARE_SPELL_FUN( spell_black_lightning );
DECLARE_SPELL_FUN( spell_midas_touch );

DECLARE_SPELL_FUN( spell_suggest );
DECLARE_SPELL_FUN( spell_cure_addiction );

/*
  * Data files used by the server.
  *
  * AREA_LIST contains a list of areas to boot.
  * All files are read in completely at bootup.
  * Most output files (bug, idea, typo, shutdown) are append-only.
  */
#define LAST_COMMAND		"../system/last_command.txt"    /*For the signal handler. */
#define PLAYER_DIR			"../player/"    /* Player files         */
#define MAIL_DIR			"../mail/"      /* Mail Directory */
#define BACKUP_DIR			"../backup/"    /* Backup Player files      */
#define MOTD_DIR			"../motd/"
#define GOD_DIR				"../gods/"  /* God Info Dir         */
#define BOARD_DIR			"../boards/"    /* Board data dir       */
#define CLAN_DIR			"../clans/" /* Clan data dir        */
#define SHIP_DIR			"../space/"
#define SPACE_DIR			"../space/"
#define PLANET_DIR			"../planets/"
#define GUARD_DIR			"../planets/"
#define GUILD_DIR			"../guilds/"    /* Guild data dir               */
#define BUILD_DIR			"../building/"  /* Online building save dir     */
#define SYSTEM_DIR			"../system/"    /* Main system files        */
#define PROG_DIR			"mudprogs/" /* MUDProg files        */
#define CORPSE_DIR			"../corpses/"   /* Corpses          */
#define STORAGE_DIR			"../storage/"  /* Player Storage              */
#define AREA_LIST			"area.lst"  /* List of areas        */
#define BAN_LIST			"ban.lst"   /* List of bans                 */
#define RESERVED_LIST		"reserved.lst"  /* List of reserved names   */
#define MNAME_LIST			"mnames.lst"    /* List of Male Names           */
#define FNAME_LIST			"fnames.lst"    /* List of Female Names         */
#define CLAN_LIST			"clan.lst"  /* List of clans        */
#define SHIP_LIST			"ship.lst"
#define PLANET_LIST			"planet.lst"
#define SPACE_LIST			"space.lst"
#define BOUNTY_LIST			"bounty.lst"
#define DISINTIGRATION_LIST	"disintigration.lst"
#define SENATE_LIST			"senate.lst"    /* List of senators     */
#define GUILD_LIST			"guild.lst" /* List of guilds               */
#define GOD_LIST			"gods.lst"  /* List of gods         */
#define GUARD_LIST			"guard.lst"
#define BOARD_FILE			"boards.txt"    /* For bulletin boards   */
#define SHUTDOWN_FILE		"shutdown.txt"  /* For 'shutdown'    */
#define IMM_HOST_FILE       SYSTEM_DIR "immortal.host"  /* For stoping hackers */
#define ARTIFACT_FILE		"artifacts.dat" /* For Saving Artifacts */
#define PASS_FILE			"pass.dat"
#define RIPSCREEN_FILE	    SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	    SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	    SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	    SYSTEM_DIR "mudtitle.asc"
#define NOTE_FILE           SYSTEM_DIR "notes.not"  /* For 'notes' */
#define IDEAN_FILE	        SYSTEM_DIR "ideas.not"
#define PENALTY_FILE	    SYSTEM_DIR "penal.not"
#define NEWS_FILE           SYSTEM_DIR "news.not"
#define CHANGES_FILE	    SYSTEM_DIR "chang.not"
#define WEDDINGS_FILE	    SYSTEM_DIR "wedds.not"
#define BOOTLOG_FILE	    SYSTEM_DIR "boot.txt"   /* Boot up error file  */
#define BUG_FILE	        SYSTEM_DIR "bugs.txt"   /* For 'bug' and bug( ) */
#define IDEA_FILE	        SYSTEM_DIR "ideas.txt"  /* For 'idea'      */
#define TYPO_FILE	        SYSTEM_DIR "typos.txt"  /* For 'typo'      */
#define ARTI_FILE           SYSTEM_DIR "artipast.txt"   /* Who picked up the last artifacts */
#define LOG_FILE	        SYSTEM_DIR "log.txt"    /* For talking in logged rooms */
#define WIZLIST_FILE	    SYSTEM_DIR "WIZLIST"    /* Wizlist         */
#define WEBWIZ_FILE			"../../../gundamomw/public_html/wizlist.html"
#define WHO_FILE	        SYSTEM_DIR "WHO"    /* Who output file     */
#define WEBWHO_FILE	        SYSTEM_DIR "WEBWHO" /* WWW Who output file */
#define REQUEST_PIPE	    SYSTEM_DIR "REQUESTS"   /* Request FIFO    */
#define SKILL_FILE	        SYSTEM_DIR "skills.dat" /* Skill table     */
#define HERB_FILE	        SYSTEM_DIR "herbs.dat"  /* Herb table      */
#define SOCIAL_FILE	        SYSTEM_DIR "socials.dat"    /* Socials         */
#define COMMAND_FILE	    SYSTEM_DIR "commands.dat"   /* Commands        */
#define USAGE_FILE	        SYSTEM_DIR "usage.txt"  /* How many people are on 
											 * every half hour - trying to
											 * determine best reboot time */
#define MUSIC_FILE          SYSTEM_DIR "music.txt"
#define LAST_LIST           SYSTEM_DIR "last.lst"
#define LAST_TEMP_LIST      SYSTEM_DIR "temp_last.lst"
#define EXE_FILE			"../src/gwom"
/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE
#define SH      SHIP_DATA

void identify_obj( CHAR_DATA *ch, OBJ_DATA *obj );
int count_auc( CHAR_DATA *ch );
void reset_auc( AUCTION_DATA *auc );
void auction_channel( const char *msg );
void auction_update( void );

/* pfiles.c */
void remove_member( const char *name, const char *shortname );
void add_member( const char *name, const char *shortname );
extern WEAPON_DATA *first_weapon;
extern WEAPON_DATA *last_weapon;

/* act_comm.c */
void sound_to_room( ROOM_INDEX_DATA *room, const char *argument );
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim );
void add_follower( CHAR_DATA *ch, CHAR_DATA *master );
void stop_follower( CHAR_DATA *ch );
void die_follower( CHAR_DATA *ch );
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch );
void send_rip_screen( CHAR_DATA *ch );
void send_rip_title( CHAR_DATA *ch );
void send_ansi_title( CHAR_DATA *ch );
void send_ascii_title( CHAR_DATA *ch );
void to_channel( const char *argument, int channel, const char *verb, short level );
void talk_auction( const char *argument );
void talk_arena( const char *argument );
void rank_chan( const char *argument );
void info_chan( const char *argument );
const char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument );
const char *obj_short( OBJ_DATA *obj );
const char *remand( const char *arg );

/* act_info.c */
int get_door( const char *arg );
char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort );
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing );
char *num_punct( int foo );
char *num_punct( long foo );
bool is_ignoring( CHAR_DATA *ch, CHAR_DATA *ign_ch );

/* act_move.c */
void clear_vrooms( void );
ED *find_door( CHAR_DATA *ch, const char *arg, bool quiet );
ED *get_exit( ROOM_INDEX_DATA *room, short dir );
ED *get_exit_to( ROOM_INDEX_DATA *room, short dir, int vnum );
ED *get_exit_num( ROOM_INDEX_DATA *room, short count );
ch_ret move_char ( CHAR_DATA *ch, EXIT_DATA *pexit, int fall );
void teleport( CHAR_DATA *ch, int room, int flags );
short encumbrance( CHAR_DATA *ch, short move );
bool will_fall( CHAR_DATA *ch, int fall );
int wherehome( CHAR_DATA *ch );

/* act_obj.c */
obj_ret damage_obj( OBJ_DATA *obj );
short get_obj_resistance( OBJ_DATA *obj );
void save_clan_storeroom( CHAR_DATA *ch, CLAN_DATA *clan );
void obj_fall( OBJ_DATA *obj, bool through );

/* act_wiz.c */
void close_area( AREA_DATA *pArea );
RID *find_location( CHAR_DATA *ch, const char *arg );
void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA *room, const char *argument );
void echo_to_all( short AT_COLOR, const char *argument, short tar );
RID *get_random_room ( CHAR_DATA *ch );
void update_calendar( void );
void get_reboot_string( void );
struct tm *update_time( struct tm *old_time );
void free_social( SOCIALTYPE *social );
void add_social( SOCIALTYPE *social );
void free_command( CMDTYPE *command );
void unlink_command( CMDTYPE *command );
void add_command( CMDTYPE *command );

/* alias.c */
typedef struct alias_type ALIAS_DATA; 
struct alias_type
{
	ALIAS_DATA *next;
	ALIAS_DATA *prev;
	char *name;
	char *cmd;
};

/* boards.c */
//void  load_boards( void );
//BD *  get_board( OBJ_DATA *obj );
//void  free_note( NOTE_DATA *pnote );
extern NOTE_DATA *noteFree;

/* build.c */
#define VCHECK_ROOM 0
#define VCHECK_OBJ 1
#define VCHECK_MOB 2
bool is_valid_vnum( int vnum, short type );
void RelCreate( relation_type, void *, void * );
void RelDestroy( relation_type, void *, void * );
const char *flag_string( int bitvector, const char *const flagarray[] );
const char *ext_flag_string( EXT_BV *bitvector, const char *const flagarray[] );
int get_dir( const char *txt );
char *strip_cr( const char *str );

/* clans.c */
CL *get_clan( const char *name );
void load_clans( void );
void save_clan( CLAN_DATA *clan );
void load_senate( void );
void save_senate( void );
PLANET_DATA *get_planet( const char *name );
void load_planets( void );
void save_planet( PLANET_DATA *planet ); 
long get_taxes( PLANET_DATA *planet );

/* bounty.c */
BOUNTY_DATA *get_disintigration( const char *target );
void load_bounties( void );
void save_bounties( void );
void save_disintigrations( void );
void remove_disintigration( BOUNTY_DATA *bounty );
void claim_disintigration( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_disintigration ( CHAR_DATA *victim );

/* player.c */
void update_storage( CHAR_DATA *ch );

/* space.c */
bool exists_player( const char *name );
SH *get_ship( const char *name );
void load_ships( void );
void save_ship( SHIP_DATA *ship );
void load_space( void );
void save_starsystem( SPACE_DATA *starsystem );
SPACE_DATA *starsystem_from_name( const char *name );
SPACE_DATA *starsystem_from_vnum( int vnum );
SHIP_DATA *ship_from_obj( int vnum );
SHIP_DATA *ship_from_entrance( int vnum );
SHIP_DATA *ship_from_hanger ( int vnum );
SHIP_DATA *ship_from_cockpit( int vnum );
SHIP_DATA *ship_from_navseat( int vnum );
SHIP_DATA *ship_from_coseat( int vnum );
SHIP_DATA *ship_from_pilotseat( int vnum );
SHIP_DATA *ship_from_gunseat( int vnum );
SHIP_DATA *ship_from_turret( int vnum );
SHIP_DATA *ship_from_engine( int vnum );
SHIP_DATA *ship_from_pilot( char *name );
SHIP_DATA *get_ship_here( const char *name, SPACE_DATA *starsystem );
void showstarsystem( CHAR_DATA *ch, SPACE_DATA *starsystem );
void update_space( void );
void quest_update( void );
void recharge_ships( void );
void move_ships( void );
void update_bus ( void );
void update_traffic( void );
bool check_pilot( CHAR_DATA *ch, SHIP_DATA *ship );
bool is_rental( CHAR_DATA *ch, SHIP_DATA *ship );
void echo_to_ship( int color, SHIP_DATA *ship, const char *argument );
void echo_to_cockpit( int color, SHIP_DATA *ship, const char *argument );
void echo_to_system( int color, SHIP_DATA *ship, const char *argument, SHIP_DATA *ignore );
bool extract_ship ( SHIP_DATA *ship );
bool ship_to_room( SHIP_DATA *ship, int vnum );
long get_ship_value( SHIP_DATA *ship );
bool rent_ship( CHAR_DATA *ch, SHIP_DATA *ship );
void damage_ship_chhead( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch );
void damage_ship_chlarm( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch );
void damage_ship_chrarm( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch );
void damage_ship_chlegs( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch );
void damage_ship( SHIP_DATA *ship, int min, int max );
void damage_ship_ch( SHIP_DATA *ship, int min, int max, CHAR_DATA *ch );
void destroy_ship( SHIP_DATA *ship, CHAR_DATA *ch );
void ship_to_starsystem( SHIP_DATA *ship, SPACE_DATA *starsystem );
void ship_from_starsystem( SHIP_DATA *ship, SPACE_DATA *starsystem );
void new_missile( SHIP_DATA *ship, SHIP_DATA *target, CHAR_DATA *ch, int missiletype );
void new_weapon( SHIP_DATA *ship, SHIP_DATA *target, CHAR_DATA *ch, int weapontype );
void extract_missile( MISSILE_DATA *missile );
void extract_weapon( WEAPON_DATA *weapon );
SHIP_DATA *ship_in_room( ROOM_INDEX_DATA *room, const char *name );
bool space_in_range( SHIP_DATA *ship, SPACE_DATA *ignore );

/* comm.c */
void close_socket( DESCRIPTOR_DATA *dclose, bool force );
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, size_t length );
void act( short AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type );
bool write_to_descriptor( DESCRIPTOR_DATA *d, const char *txt, int length );
void descriptor_printf( DESCRIPTOR_DATA *d, const char *fmt, ... ) __attribute__( ( format( printf, 2, 3 ) ) );
void buffer_printf( DESCRIPTOR_DATA *d, const char *fmt, ... ) __attribute__( ( format( printf, 2, 3 ) ) );

//bool has_rate( CHAR_DATA *ch, int rate );

/* reset.c */
RD *make_reset( char letter, int extra, int arg1, int arg2, int arg3 );
RD *add_reset( ROOM_INDEX_DATA *room, char letter, int extra, int arg1, int arg2, int arg3 );
void reset_area( AREA_DATA *pArea );

/* db.c */
char *fread_flagstring( FILE *fp );
size_t mudstrlcat( char *__restrict dst, const char *__restrict src, size_t dsize );
size_t mudstrlcpy( char *__restrict dst, const char *__restrict src, size_t dsize );
void show_file( CHAR_DATA *ch, const char *filename );
bool is_valid_filename( CHAR_DATA *ch, const char *direct, const char *filename );
char *str_dup( char const *str );
void boot_db( bool fCopyOver );
void area_update( void );
void add_char( CHAR_DATA *ch );
CD *create_mobile( MOB_INDEX_DATA *pMobIndex );
OD *create_object( OBJ_INDEX_DATA *pObjIndex, int level );
void clear_char( CHAR_DATA *ch );
void free_char( CHAR_DATA *ch );
const char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed );
MID *get_mob_index( int vnum );
OID *get_obj_index( int vnum );
RID *get_room_index( int vnum );
char fread_letter( FILE *fp );
int fread_number( FILE *fp );
EXT_BV fread_bitvector( FILE *fp );
void fwrite_bitvector( EXT_BV *bits, FILE *fp );
char *print_bitvector( EXT_BV *bits );
float fread_float( FILE *fp );
const char *fread_string( FILE *fp );
char *fread_string_nohash( FILE *fp );
char *fread_string_eol( FILE *fp );
void fread_to_eol( FILE *fp );
char *fread_word( FILE *fp );
char *fread_line( FILE *fp );
int number_fuzzy( int number );
int number_range( int from, int to );
int number_percent( void );
int number_door( void );
int number_bits( int width );
int number_mm( void );
int dice( int number, int size );
int interpolate( int level, int value_00, int value_32 );
void smash_percent( char *str );
const char *smash_percent_static( const char *str );
void smash_tilde( char *str );
const char *smash_tilde_static( const char *str );
char *smash_tilde_copy( const char *str );
void hide_tilde( char *str );
const char *show_tilde( const char *str );
bool str_cmp( const char *astr, const char *bstr );
bool str_prefix( const char *astr, const char *bstr );
bool str_infix( const char *astr, const char *bstr );
bool str_suffix( const char *astr, const char *bstr );
char *capitalize( const char *str );
char *strlower( const char *str );
char *strupper( const char *str );
const char *aoran( const char *str );
void append_file( CHAR_DATA *ch, const char *file, const char *str );
void append_to_file( const char *file, const char *str );
void bug( const char *str, ... );
void log_string_plus( const char *str, short log_type, short level );
void log_bug_plus( const char *str, short log_type, short level );
void log_printf_plus( short log_type, short level, const char *fmt, ... ) __attribute__( ( format( printf, 3, 4 ) ) );
void log_printf( const char *fmt, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
ROOM_INDEX_DATA *make_room( int vnum, AREA_DATA *area );
OID *make_object( int vnum, int cvnum, const char *name );
MID *make_mobile( int vnum, short cvnum, const char *name );
ED *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, short door );
void add_help( HELP_DATA *pHelp );
void fix_area_exits( AREA_DATA *tarea );
void load_area_file( const char *filename, bool isproto );
void randomize_exits( ROOM_INDEX_DATA *room, short maxdir );
void make_wizlist( void );
void tail_chain( void );
void delete_room( ROOM_INDEX_DATA *room );
bool delete_obj( OBJ_INDEX_DATA *obj ); 
bool delete_mob( MOB_INDEX_DATA *mob );
/* Functions to add to sorting lists. -- Altrag */
/*void mob_sort( MOB_INDEX_DATA *pMob );
void obj_sort( OBJ_INDEX_DATA *pObj );
void room_sort( ROOM_INDEX_DATA *pRoom );*/
void sort_area( AREA_DATA *pArea, bool proto );
void sort_area_by_name( AREA_DATA *pArea ); /* Fireblade */

/* build.c */
void start_editing( CHAR_DATA *ch, const char *data );
void stop_editing( CHAR_DATA *ch );
void edit_buffer( CHAR_DATA *ch, char *argument );
const char *copy_buffer( CHAR_DATA *ch );
bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room );
bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj );
bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob );
bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob );
void free_reset( AREA_DATA *are, RESET_DATA *res );
void free_area( AREA_DATA *are );
void assign_area( CHAR_DATA *ch );
EDD *SetRExtra( ROOM_INDEX_DATA *room, const char *keywords );
bool DelRExtra( ROOM_INDEX_DATA *room, const char *keywords );
EDD *SetOExtra( OBJ_DATA *obj, const char *keywords );
bool DelOExtra( OBJ_DATA *obj, const char *keywords );
EDD *SetOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords );
bool DelOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords );
void fold_area( AREA_DATA *tarea, const char *filename, bool install );
int get_otype( const char *type );
int get_aflag( const char *flag );
int get_trapflag( const char *flag );
int get_atype( const char *type );
int get_npc_race( const char *type );
int get_pc_race( const char *type );
int get_wearloc( const char *type );
int get_exflag( const char *flag );
int get_rflag( const char *flag );
int get_mpflag( const char *flag );
int get_oflag( const char *flag );
int get_areaflag( const char *flag );
int get_wflag( const char *flag );
int get_actflag( const char *flag );
int get_vip_flag( const char *flag );
int get_wanted_flag( const char *flag );
int get_pcflag( const char *flag );
int get_plrflag( const char *flag );
int get_risflag( const char *flag );
int get_trigflag( const char *flag );
int get_partflag( const char *flag );
int get_attackflag( const char *flag );
int get_defenseflag( const char *flag );
int get_secflag( const char *flag );
int get_npc_position( const char *position );
int get_npc_sex( const char *sex );

/* fight.c */
int max_fight( CHAR_DATA *ch );
void violence_update( void );
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
int ris_damage( CHAR_DATA *ch, int dam, int ris );
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );
void set_suitfighting( SHIP_DATA *suit, SHIP_DATA *target );
void stop_suitfighting( SHIP_DATA *suit, bool fBoth );
void free_suitfight( SHIP_DATA *suit );
SHIP_DATA *who_suitfighting( SHIP_DATA *suit );
void update_pos( CHAR_DATA *victim );
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim );
void stop_fighting( CHAR_DATA *ch, bool fBoth );
void free_fight( CHAR_DATA *ch );
CD *who_fighting( CHAR_DATA *ch );
void set_attacking( CHAR_DATA *ch, CHAR_DATA *victim );
void stop_attacking( CHAR_DATA *ch, bool fBoth );
void clear_attack( CHAR_DATA *ch );
void free_attack( CHAR_DATA *ch );
CD *who_attacking( CHAR_DATA *ch );
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim );
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim );
void death_cry( CHAR_DATA *ch );
void stop_hunting( CHAR_DATA *ch );
void stop_hating( CHAR_DATA *ch );
void stop_fearing( CHAR_DATA *ch );
void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim );
void start_hating( CHAR_DATA *ch, CHAR_DATA *victim );
void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_safe_nm( CHAR_DATA *ch, CHAR_DATA *victim );
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim );
bool check_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim );
void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim );
bool in_arena( CHAR_DATA *ch );
bool is_voodood( CHAR_DATA *ch, CHAR_DATA *victim );
void limitbreak( CHAR_DATA *ch, CHAR_DATA *victim );

/* makeobjs.c */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer );
void make_blood( CHAR_DATA *ch );
void make_bloodstain( CHAR_DATA *ch );
void make_scraps( OBJ_DATA *obj );
void make_fire( ROOM_INDEX_DATA *in_room, short timer );
OD *make_trap( int v0, int v1, int v2, int v3 ); 
OD *create_money( int amount );

/* misc.c */
void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo );
EXT_BV meb( int bit );
EXT_BV multimeb( int bit, ... );
void jedi_checks( CHAR_DATA *ch );
void jedi_bonus( CHAR_DATA *ch );
void sith_penalty( CHAR_DATA *ch );

/* mud_comm.c */
const char *mprog_type_to_name( int type );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *strstr( const char *s1, const char *s2 );
#endif
void mpsleep_update( void );
void mprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *object, void *vo, int type );
void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor,	OBJ_DATA *object, void *vo,	int type );
void mprog_act_trigger( const char *buf, CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj, void *vo );
void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount );
void mprog_entry_trigger( CHAR_DATA *mob );
void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch,	OBJ_DATA *obj );
void mprog_greet_trigger( CHAR_DATA *mob );
void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch );
void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch );
void mprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *mob );
void mprog_random_trigger( CHAR_DATA *mob );
void mprog_speech_trigger( const char *txt, CHAR_DATA *mob );
void mprog_script_trigger( CHAR_DATA *mob );
void mprog_hour_trigger( CHAR_DATA *mob );
void mprog_time_trigger( CHAR_DATA *mob );
void progbug( const char *str, CHAR_DATA *mob );
void rset_supermob( ROOM_INDEX_DATA *room ); 
void release_supermob( void );

/* player.c */
void set_title( CHAR_DATA *ch, const char *title );

/* skills.c */
bool check_skill( CHAR_DATA *ch, const char *command, const char *argument );
void learn_from_success( CHAR_DATA *ch, int sn );
void learn_from_limiter( CHAR_DATA *ch, int sn );
void learn_from_failure( CHAR_DATA *ch, int sn );
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim );
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim );
bool check_grip( CHAR_DATA *ch, CHAR_DATA *victim );
void disarm( CHAR_DATA *ch, CHAR_DATA *victim );
void trip( CHAR_DATA *ch, CHAR_DATA *victim );

/* freeze.c */
void tag_update( void );
void end_tag( void );
void start_tag( void );
void check_team_frozen( CHAR_DATA *ch );
void auto_tag( void );
bool is_tagging( CHAR_DATA *ch );
void tag_channel( CHAR_DATA *ch, const char *message );

/* hiscores.c (addit.c) */
#define HISCORE_CODE
const char *is_hiscore_obj( OBJ_DATA *obj );
void show_hiscore( const char *keyword, CHAR_DATA *ch );
void adjust_hiscore( const char *keyword, CHAR_DATA *ch, int score );
void save_hiscores( void ); 
void load_hiscores( void );

/* host.c */
bool check_immortal_domain( CHAR_DATA *ch, const char *host );
int load_imm_host( void );
int fread_imm_host( FILE *fp, IMMORTAL_HOST *data ); 
void do_write_imm_host( void );
//void do_add_imm_host( CHAR_DATA *ch, char *argument );

/* handler.c */
void free_obj( OBJ_DATA *obj );
void explode( OBJ_DATA *obj );
int get_exp( CHAR_DATA *ch, int ability );
int get_exp_worth( CHAR_DATA *ch );
int exp_level( short level );
short get_trust( CHAR_DATA *ch );
short get_age( CHAR_DATA *ch );
short get_curr_str( CHAR_DATA *ch );
short get_curr_int( CHAR_DATA *ch );
short get_curr_wis( CHAR_DATA *ch );
short get_curr_dex( CHAR_DATA *ch );
short get_curr_con( CHAR_DATA *ch );
short get_curr_cha( CHAR_DATA *ch );
short get_curr_lck( CHAR_DATA *ch );
bool can_take_proto( CHAR_DATA *ch );
int can_carry_n( CHAR_DATA *ch );
int can_carry_w( CHAR_DATA *ch );
bool is_name( const char *str, const char *namelist );
bool is_name_prefix( const char *str, const char *namelist );
bool nifty_is_name( const char *str, const char *namelist );
bool nifty_is_name_prefix( const char *str, const char *namelist ); 
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd );
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf );
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf );
void affect_strip( CHAR_DATA *ch, int sn );
bool is_affected( CHAR_DATA *ch, int sn );
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf );
void char_from_room( CHAR_DATA *ch );
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex );
OD *obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch );
void obj_from_char( OBJ_DATA *obj );
int apply_ac( OBJ_DATA *obj, int iWear );
OD *get_eq_char( CHAR_DATA *ch, int iWear );
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear );
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj );
int count_obj_list( OBJ_INDEX_DATA *obj, OBJ_DATA *list );
void obj_from_room( OBJ_DATA *obj );
OD *obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex );
OD *obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to );
void obj_from_obj( OBJ_DATA *obj );
void extract_obj( OBJ_DATA *obj );
void extract_exit( ROOM_INDEX_DATA *room, EXIT_DATA *pexit );
void extract_room( ROOM_INDEX_DATA *room );
void clean_room( ROOM_INDEX_DATA *room );
void clean_obj( OBJ_INDEX_DATA *obj );
void clean_mob( MOB_INDEX_DATA *mob );
void clean_resets( ROOM_INDEX_DATA *room );
void extract_char( CHAR_DATA *ch, bool fPull );
CD *get_char_room( CHAR_DATA *ch, const char *argument );
CD *get_char_world( CHAR_DATA *ch, const char *argument );
OD *get_obj_type( OBJ_INDEX_DATA *pObjIndexData );
OD *get_obj_list( CHAR_DATA *ch, const char *argument, OBJ_DATA *list );
OD *get_obj_list_rev( CHAR_DATA *ch, const char *argument, OBJ_DATA *list );
OD *get_obj_carry( CHAR_DATA *ch, const char *argument );
OD *get_obj_wear( CHAR_DATA *ch, const char *argument );
OD *get_obj_here( CHAR_DATA *ch, const char *argument );
OD *get_obj_world( CHAR_DATA *ch, const char *argument );
int get_obj_number( OBJ_DATA *obj );
int get_obj_weight( OBJ_DATA *obj );
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex );
bool room_is_private( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex );
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim );
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj );
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj );
const char *item_type_name( OBJ_DATA *obj );
const char *affect_loc_name( int location );
const char *affect_bit_name( int vector );
//char *extra_bit_name( int extra_flags );
const char *magic_bit_name( int magic_flags );
ch_ret check_for_trap( CHAR_DATA *ch, OBJ_DATA *obj, int flag );
ch_ret check_room_for_traps( CHAR_DATA *ch, int flag );
bool is_trapped( OBJ_DATA *obj );
OD *get_trap( OBJ_DATA *obj );
ch_ret spring_trap( CHAR_DATA *ch, OBJ_DATA *obj );
void name_stamp_stats( CHAR_DATA *ch );
void fix_char( CHAR_DATA *ch );
void showaffect( CHAR_DATA *ch, AFFECT_DATA *paf );
void set_cur_obj( OBJ_DATA *obj );
bool obj_extracted( OBJ_DATA *obj );
void queue_extracted_obj( OBJ_DATA *obj );
void clean_obj_queue( void );
void set_cur_char( CHAR_DATA *ch );
bool char_died( CHAR_DATA *ch );
void queue_extracted_char( CHAR_DATA *ch, bool extract );
void clean_char_queue( void );
void add_timer( CHAR_DATA *ch, short type, short count, DO_FUN *fun, int value );
TIMER *get_timerptr( CHAR_DATA *ch, short type );
short get_timer( CHAR_DATA *ch, short type );
void extract_timer( CHAR_DATA *ch, TIMER *timer );
void remove_timer( CHAR_DATA *ch, short type );
bool in_soft_range( CHAR_DATA *ch, AREA_DATA *tarea );
bool in_hard_range( CHAR_DATA *ch, AREA_DATA *tarea );
bool chance( CHAR_DATA *ch, short percent );
bool chance_attrib( CHAR_DATA *ch, short percent, short attrib );
OD *clone_object( OBJ_DATA *obj );
void split_obj( OBJ_DATA *obj, int num );
void separate_obj( OBJ_DATA *obj );
bool empty_obj( OBJ_DATA *obj, OBJ_DATA *destobj, ROOM_INDEX_DATA *destroom );
OD *find_obj( CHAR_DATA *ch, const char *argument, bool carryonly );
bool ms_find_obj( CHAR_DATA *ch );
void worsen_mental_state( CHAR_DATA *ch, int mod );
void better_mental_state( CHAR_DATA *ch, int mod );
void boost_economy( AREA_DATA *tarea, int gold );
void lower_economy( AREA_DATA *tarea, int gold );
void economize_mobgold( CHAR_DATA *mob );
bool economy_has( AREA_DATA *tarea, int gold );
void add_kill( CHAR_DATA *ch, CHAR_DATA *mob );
int times_killed( CHAR_DATA *ch, CHAR_DATA *mob );
int count_users( OBJ_DATA *obj );
bool remove_voodoo( CHAR_DATA *ch );
int strlen_color( const char *argument );
void check_lottery( CHAR_DATA *ch );
void update_aris( CHAR_DATA *ch );

/* interp.c */
bool check_pos( CHAR_DATA *ch, short position );
void interpret( CHAR_DATA *ch, const char *argument );
bool is_number( const char *arg );
int number_argument( const char *argument, char *arg );
char *one_argument( char *argument, char *arg_first );
const char *one_argument( const char *argument, char *arg_first );
const char *one_argument2( const char *argument, char *arg_first );
ST *find_social( const char *command );
CMDTYPE *find_command( const char *command );
void hash_commands( void );
void start_timer( struct timeval *stime );
time_t end_timer( struct timeval *stime );
void send_timer( struct timerset *vtime, CHAR_DATA *ch );
void update_userec( struct timeval *time_used, struct timerset *userec );
void stage_update( CHAR_DATA *ch, CHAR_DATA *victim, int stage );

/* magic.c */
bool process_spell_components( CHAR_DATA *ch, int sn );
int ch_slookup( CHAR_DATA *ch, const char *name );
int find_spell( CHAR_DATA *ch, const char *name, bool know );
int find_skill( CHAR_DATA *ch, const char *name, bool know );
int find_weapon( CHAR_DATA *ch, const char *name, bool know );
int find_tongue( CHAR_DATA *ch, const char *name, bool know );
int skill_lookup( const char *name );
int weapon_lookup( const char *name );
int herb_lookup( const char *name );
int personal_lookup( CHAR_DATA *ch, const char *name );
int slot_lookup( int slot );
int bsearch_skill( const char *name, int first, int top );
int bsearch_weapon( const char *name, int first, int top );
int bsearch_skill_exact( const char *name, int first, int top );
int bsearch_skill_prefix( const char *name, int first, int top );
bool saves_poison_death( int level, CHAR_DATA *victim );
bool saves_wand( int level, CHAR_DATA *victim );
bool saves_para_petri( int level, CHAR_DATA *victim );
bool saves_breath( int level, CHAR_DATA *victim );
bool saves_spell_staff( int level, CHAR_DATA *victim );
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj );
int dice_parse( CHAR_DATA *ch, int level, char *exp ); 
SK *get_skilltype( int sn );

/* note.c */
void expire_notes( void );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
#define OS_GROUND	2
#define OS_LOCKER	3
void copy_files_contents( FILE *fsource, FILE *fdestination );
void read_last_file( CHAR_DATA *ch, int count, const char *name );
void write_last_file( const char *entry );
void save_char_obj( CHAR_DATA *ch );
void save_clone( CHAR_DATA *ch );
bool load_char_obj( DESCRIPTOR_DATA *d, const char *name, bool preload, bool copyover );
void set_alarm( long seconds );
void requip_char( CHAR_DATA *ch );
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest, short os_type, bool hotboot );
void fread_obj( CHAR_DATA *ch, FILE *fp, short os_type );
void de_equip_char( CHAR_DATA *ch );
void re_equip_char( CHAR_DATA *ch );
void save_home( CHAR_DATA *ch );
void load_home( CHAR_DATA *ch );
void save_finger( CHAR_DATA *ch );
void fwrite_finger( CHAR_DATA *ch, FILE *fp );
void read_finger( CHAR_DATA *ch, const char *argument );
void fread_finger( CHAR_DATA *ch, FILE *fp, const char *laston );
void pull_slot( CHAR_DATA *ch );
extern int slotland;
void save_artifacts( void );
void load_artifacts( void );

/* shops.c */

/* special.c */
SF *spec_lookup( const char *name ); 

/* tables.c */
extern int top_wn;
int get_skill( const char *skilltype );
void load_skill_table( void );
void save_skill_table( void );
void load_weapon_table( void );
void save_weapon_table( void );
void sort_skill_table( void );
void sort_weapon_table( void );
void load_socials( void );
void save_socials( void );
void load_commands( void );
void save_commands( void );
SPELL_FUN *spell_function( const char *name );
DO_FUN *skill_function( const char *name );
void load_herb_table( void ); 
void save_herb_table( void );

/* track.c */
void found_prey( CHAR_DATA *ch, CHAR_DATA *victim );
void hunt_victim( CHAR_DATA *ch );

/* update.c */
void advance_level( CHAR_DATA *ch, int ability );
void gain_exp( CHAR_DATA *ch, int gain, int ability );
void gain_condition( CHAR_DATA *ch, int iCond, int value );
void update_handler( void ); 
void reboot_check( time_t reset );
#if 0
void reboot_check( const char *arg );
#endif
//void auction_update ( void );
void load_gang( void );
void remove_portal( OBJ_DATA *portal );
int max_level( CHAR_DATA *ch, int ability ); 
void pulse_relationship( void );

/* war.c */
void war_channel( const char *argument );
void war_update( void );
void auto_war( void );
void check_war( CHAR_DATA *ch, CHAR_DATA *victim );
bool is_safe_war( CHAR_DATA *ch, CHAR_DATA *wch );
void war_talk( CHAR_DATA *ch, const char *argument );
void end_war( void );
bool abort_race_war( void );
bool abort_class_war( void );
bool abort_clan_war( void ); 
void extract_war( CHAR_DATA *ch );

/* hashstr.c */
const char *str_alloc( const char *str );
const char *quick_link( const char *str );
int str_free( const char *str );
void show_hash( int count );
char *hash_stats( void );
char *check_hash( const char *str );
void hash_dump( int hash );
void show_high_hash( int top );
bool in_hash_table( const char *str );

/* newscore.c */
const char *get_race( CHAR_DATA *ch );

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */
 /*
  *  Data for a menu page
  */
struct menu_data
{
	char *sectionNum;
	char *charChoice;
	int x;
	int y;
	char *outFormat;
	void *data;
	int ptrType;
	int cmdArgs;
	char *cmdString;
};
DECLARE_DO_FUN( do_redraw_page );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen );
DECLARE_DO_FUN( do_omenu );
DECLARE_DO_FUN( do_rmenu );
DECLARE_DO_FUN( do_mmenu );
DECLARE_DO_FUN( do_clear );
extern MENU_DATA room_page_a_data[];
extern MENU_DATA room_page_b_data[];
extern MENU_DATA room_page_c_data[];
extern MENU_DATA room_help_page_data[];
extern MENU_DATA mob_page_a_data[];
extern MENU_DATA mob_page_b_data[];
extern MENU_DATA mob_page_c_data[];
extern MENU_DATA mob_page_d_data[];
extern MENU_DATA mob_page_e_data[];
extern MENU_DATA mob_page_f_data[];
extern MENU_DATA mob_help_page_data[];
extern MENU_DATA obj_page_a_data[];
extern MENU_DATA obj_page_b_data[];
extern MENU_DATA obj_page_c_data[];
extern MENU_DATA obj_page_d_data[];
extern MENU_DATA obj_page_e_data[];
extern MENU_DATA obj_help_page_data[];
extern MENU_DATA control_page_a_data[];
extern MENU_DATA control_help_page_data[];
extern const char room_page_a[];
extern const char room_page_b[];
extern const char room_page_c[];
extern const char room_help_page[];
extern const char obj_page_a[];
extern const char obj_page_b[];
extern const char obj_page_c[];
extern const char obj_page_d[];
extern const char obj_page_e[];
extern const char obj_help_page[];
extern const char mob_page_a[];
extern const char mob_page_b[];
extern const char mob_page_c[];
extern const char mob_page_d[];
extern const char mob_page_e[];
extern const char mob_page_f[];
extern const char mob_help_page[];
extern const char *ris_strings[];
extern const char control_page_a[];
extern const char control_help_page[];

//#define short 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5
#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19
#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4
#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
 * defines for use with this get_affect function
 */
#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07
#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30

/*
 *   Map Structures
 */
DECLARE_DO_FUN( do_mapout );
DECLARE_DO_FUN( do_lookmap );

struct map_data /* contains per-room data */
{
	int vnum;  /* which map this room belongs to */
	int x; /* horizontal coordinate */
	int y; /* vertical coordinate */
	char entry;    /* code that shows up on map */
};

struct map_index_data
{
	MAP_INDEX_DATA *next;
	int vnum;   /* vnum of the map */
	int map_of_vnums[49][81];  /* room vnums aranged as a map */
};

MAP_INDEX_DATA *get_map_index( int vnum ); 
void init_maps( void );

/*
 * mudprograms stuff
 */
extern CHAR_DATA *supermob;
extern OBJ_DATA *supermob_obj;

void oprog_speech_trigger( const char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj ); 
char *oprog_type_to_name( int type );
/*
* MUD_PROGS START HERE
* (object stuff)
*/
void oprog_greet_trigger( CHAR_DATA *ch );
//void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
//void oprog_random_trigger( OBJ_DATA *obj );
//void oprog_random_trigger( OBJ_DATA *obj );
//void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
//void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
//void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
//void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
//void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
//void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj );

/* mud prog defines */
#define ERROR_PROG        -1
#define IN_FILE_PROG       -2
#define ACT_PROG           0
#define SPEECH_PROG        1
#define RAND_PROG          2
#define FIGHT_PROG         3
#define RFIGHT_PROG        3
#define DEATH_PROG         4
#define RDEATH_PROG        4
#define HITPRCNT_PROG      5
#define ENTRY_PROG         6
#define ENTER_PROG         6
#define GREET_PROG         7
#define RGREET_PROG	   7
#define OGREET_PROG        7
#define ALL_GREET_PROG	   8
#define GIVE_PROG	   9
#define BRIBE_PROG	   10
#define HOUR_PROG	   11
#define TIME_PROG	   12
#define WEAR_PROG          13
#define REMOVE_PROG        14
#define SAC_PROG           15
#define LOOK_PROG          16
#define EXA_PROG           17
#define ZAP_PROG           18
#define GET_PROG 	   19
#define DROP_PROG	   20
#define DAMAGE_PROG	   21
#define REPAIR_PROG	   22
#define RANDIW_PROG	   23
#define SPEECHIW_PROG	   24
#define PULL_PROG	   25
#define PUSH_PROG	   26
#define SLEEP_PROG         27
#define REST_PROG          28
#define LEAVE_PROG         29
#define SCRIPT_PROG	   30
#define USE_PROG           31
#define PUT_PROG			32

void rprog_leave_trigger( CHAR_DATA *ch );
void rprog_enter_trigger( CHAR_DATA *ch );
void rprog_sleep_trigger( CHAR_DATA *ch );
void rprog_rest_trigger( CHAR_DATA *ch );
void rprog_rfight_trigger( CHAR_DATA *ch );
void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger( const char *txt, CHAR_DATA *ch );
void rprog_random_trigger( CHAR_DATA *ch );
void rprog_time_trigger( CHAR_DATA *ch );
void rprog_hour_trigger( CHAR_DATA *ch ); 
char *rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( const char *buf, OBJ_DATA *mobj, CHAR_DATA *ch, OBJ_DATA *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( const char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch, OBJ_DATA *obj, void *vo );
#endif
