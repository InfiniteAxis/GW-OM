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
#include "mud.h"
#include "sha256.h"

#define CURRENT_AREA_VERSION	1

extern int top_affect;
extern int top_reset;
extern int top_ed;
extern bool fBootDb;

bool hidefoldmessage;

bool check_area_conflict( AREA_DATA *carea, int low_range, int hi_range );
char *sprint_reset( RESET_DATA *pReset, short *num );
void fix_exits( void );
bool validate_spec_fun( const char *name );

REL_DATA *first_relation = NULL;
REL_DATA *last_relation = NULL;

/*
 * Local functions. -Orion
 */
int generate_hp( int level, int num, int size, int plus );

/* planet constants for vip and wanted flags */

const char *const planet_flags[] = {
   "G42", "R28", "L3", "Z64", "X45", "K46", "TriColonies", 
   "X18999", "Earth ", "Mars", "New Saturn", "Russia", 
   "China", "Moon", "Rommefellar", "M9", "p15", "p16", 
   "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", 
   "p25", "p26", "p27", "p28", "p29", "p30", "p31"
};

const char *const weapon_table[13] = {
   "none",
   "axe", "blade", "lightsaber", "whip", "claw",
   "blaster", "pound", "crush", "bowcaster", "bite",
   "force pike", "strike"
};

const char *const spice_table[] = {
   "glitterstim", "carsanum", "ryll", "andris", "s4", "s5", "s6", "s7", "s8", "s9"
};

const char *const crystal_table[8] = {
   "non-adegan", "kathracite", "relacite", "danite", "mephite", "ponite", "illum", "corusca"
};


const char *const ex_flags[] = {
   "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
   "dig", "r1", "nopassdoor", "hidden", "passage", "portal", "r2", "r3",
   "can_climb", "can_enter", "can_leave", "auto", "r4", "searchable",
   "bashed", "bashproof", "nomob", "window", "can_look"
};

const char *const r_flags[] = {
   "dark", "reserved", "nomob", "indoors", "can_land", "can_fly", "no_drive",
   "nomagic", "bank", "private", "safe", "remove_this_flag", "petshop", "norecall",
   "donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
   "plr_home", "empty_home", "teleport", "hotel", "nofloor", "refinery", "factory",
   "republic_recruit", "blank", "spacecraft", "prototype", "arena", "heal", "docking",
   "tag", "cyber", "lottery", "upgrade", "storage", "base", "capture",
   "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22",
   "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "r32", "r33",
   "r34", "r35", "r36", "r37", "r38", "r39", "r40", "r41", "r42", "r43", "r44",
   "r45", "r46", "r47", "r48", "r49", "r50"
};

const char *const o_flags[] = {
   "glow", "hum", "dark", "hutt_size", "contraband", "invis", "magic", "nodrop", "bless",
   "antigood", "antievil", "antineutral", "noremove", "inventory",
   "noinvoke", "antithief", "antihunter", "artifact", "small_size", "large_size",
   "donation", "clanobject", "anticitizen", "antisith", "antipilot",
   "hidden", "poisoned", "covering", "deathrot", "buried", "prototype", "human_size",
   "augmented"
};

const char *const mag_flags[] = {
   "returning", "backstabber", "bane", "loyal", "haste", "drain",
   "lightning_blade"
};

const char *const w_flags[] = {
   "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
   "shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
   "_missile_", "protofloating", "protoankle", "protoback", "relic", "r5", "r6",
   "r7", "r8", "r9", "r10", "r11", "r12", "r13"
};

const char *const area_flags[] = {
   "nopkill", "base", "prototype", "r3", "r4", "r5", "r6", "r7", "r8",
   "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
   "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

const char *const o_types[] = {
   "none", "light", "_scroll", "_wand", "staff", "weapon", "_fireweapon", "missile",
   "treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
   "container", "_note", "drinkcon", "key", "food", "money", "pen", "_boat",
   "corpse", "corpse_pc", "fountain", "pill", "_blood", "_bloodstain",
   "scraps", "_pipe", "_herbcon", "_herb", "_incense", "fire", "book", "switch",
   "lever", "_pullchain", "button", "dial", "_rune", "_runepouch", "_match", "trap",
   "map", "_portal", "paper", "_tinder", "lockpick", "_spike", "_disease", "_oil",
   "fuel", "_shortbow", "_longbow", "_crossbow", "ammo", "_quiver", "shovel",
   "salve", "rawspice", "lens", "crystal", "duraplast", "battery",
   "toolkit", "durasteel", "oven", "mirror", "circuit", "superconductor", "comlink", "medpac",
   "fabric", "rare_metal", "magnet", "thread", "spice", "smut", "device", "spacecraft",
   "grenade", "landmine", "government", "droid_corpse", "bolt", "chemical",
   "remote", "jukebox", "ptrap", "drug", "mixture", "wholder", "frame",
   "suitweapon", "whetstone", "debitcard", "restore"
};

const char *const a_types[] = {
   "none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
   "sex", "null", "level", "age", "height", "weight", "force", "hit", "move",
   "dollars", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
   "save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
   "immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
   "steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
   "search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
   "grip", "scribe", "brew", "wearspell", "removespell", "emotion", "mentalstate",
   "stripsn", "remove", "dig", "full", "thirst", "drunk", "blood",
   "recurringspell", "xaffected"
};

const char *const a_flags[] = {
   "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
   "detect_hidden", "weaken", "sanctuary", "mass_heal", "infrared", "stalk",
   "_flaming", "poison", "nemesis", "paralysis", "sneak", "hide", "sleep",
   "charm", "flying", "money_maker", "floating", "truesight", "detect_traps",
   "money_maker", "fireshield", "shockshield", "r1", "iceshield", "possess",
   "berserk", "aqua_breath", "recurringspell", "scrying"
};

const char *const act_flags[] = {
   "npc", "sentinel", "scavenger", "noquest", "aggressive", "stayarea",
   "wimpy", "pet", "train", "practice", "immortal", "deadly", "polyself", 
   "metaaggr", "guardian", "running", "nowander", "mountable", "mounted", 
   "secretive", "polymorphed", "mobinvis", "noassist", "nokill", "droid", 
   "nocorpse", "prototype"
};

const char *const pc_flags[] = {
   "botter", "fbalzhur", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
   "nosummon", "pageron", "notitled", "room", "dnd", "norestore", "zerosystem",
   "arena", "noooc", "war", "noauction", "autowho", "stun", "twit", "haslotto",
   "lottowait", "autoscan", "exempt", "canloot", "nonote", "unused",
   "attackmode", "spiritmode", "incombat"
};

const char *const cyba_flags[] = {
   "hasgun", "hasblade", "hascharger", "gunout", "bladedrawn", "ischarging",
   "hascharger2", "hascharger3", "usingspecial", "refillen", "regularammo",
   "hasbubble", "hassmog", "hastornado", "haslifesap", "hp_fix", "nolevel",
   "ascended",
   "r18", "r19",
   "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
   "r30", "r31"
};


const char *const plr_flags[] = {
   "npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "quester",
   "blank", "brief", "combine", "prompt", "telnet_ga", "holylight",
   "wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
   "deny", "freeze", "killer", "pk", "litterbug", "ansi", "rip", "nice",
   "ppk", "flee", "autocred", "automap", "afk"
};

const char *const trap_flags[] = {
   "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
   "unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
   "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
   "r14", "r15"
};

const char *const wear_locs[] = {
   "light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
   "feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
   "wield", "hold", "dual_wield", "ears", "eyes", "missile_wield", "floating",
   "ankle1", "ankle2", "back", "relic"
};

const char *const ris_flags[] = {
   "fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
   "poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
   "plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "shot",
   "whip", "claw",
   "r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

const char *const trig_flags[] = {
   "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
   "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
   "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
   "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
   "showroomdesc", "autoreturn", "r2", "r3"
};

const char *const part_flags[] = {
   "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
   "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
   "tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
   "sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
   "r1", "r2"
};

const char *const attack_flags[] = {
   "bite", "claws", "tail", "sting", "punch", "kick",
   "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
   "poison", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
"r27", "r28", "r29",
   "r30", "circle"
};

const char *const defense_flags[] = {
   "parry", "dodge", "r2", "r3", "r4", "r5",
   "r6", "r7", "r8", "r9", "r10", "r11", "r12",
 "r13", "r14", "r15", "r16", "r17",
   "r18", "disarm", "r20", "grip", "r22",
 "r23", "r24", "r25", "r26", "r27", "r28", "r29",
   "r30", "r31"
};

const char *const element_table[] = {
   "Fire", "Wind", "Lightning", "Earth", "Water", "Ice", "None"
};

const char *const npc_position[] = {
   "dead", "mortal", "incapacitated", "stunned", "sleeping",
   "resting", "sitting", "fighting", "standing", "mounted", 
   "shove", "drag"
};

const char *const npc_sex[] = {
   "nonbinary", "male", "female"
};

const char *sector_name[SECT_MAX] = {
   "inside", "city", "field", "forest", "hills", "mountain", "water swim", "water noswim",
   "underwater", "air", "desert", "unknown", "ocean floor", "underground"
};


/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 *
 */
const char *const mprog_flags[] = {
   "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
   "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
   "look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
   "speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use",
   "put"
};


const char *flag_string( int bitv, const char *const flagarray[] )
{
	static char buf[MAX_STRING_LENGTH];
	int x;

	buf[0] = '\0';
	for( x = 0; x < 32; x++ )
		if( IS_SET( bitv, 1 << x ) )
		{
			mudstrlcat( buf, flagarray[x], MAX_STRING_LENGTH );
			mudstrlcat( buf, " ", MAX_STRING_LENGTH );
		}
	if( ( x = strlen( buf ) ) > 0 )
		buf[--x] = '\0';

	return buf;
}

const char *ext_flag_string( EXT_BV *bitvector, const char *const flagarray[] )
{
	static char buf[MAX_STRING_LENGTH];
	int x;

	buf[0] = '\0';
	for( x = 0; x < MAX_BITS; ++x )
		if( xIS_SET( *bitvector, x ) )
		{
			mudstrlcat( buf, flagarray[x], MAX_STRING_LENGTH );
			mudstrlcat( buf, " ", MAX_STRING_LENGTH );
		}
	if( ( x = strlen( buf ) ) > 0 )
		buf[--x] = '\0';

	return buf;
}


bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	int vnum = room->vnum;
	AREA_DATA *pArea;

	if( IS_NPC( ch ) )
		return false;
	if( get_trust( ch ) >= sysdata.level_modify_proto )
		return true;
	if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
	{
		send_to_char( "You must have an assigned area to modify this room.\r\n", ch );
		return false;
	}
	if( vnum >= pArea->low_vnum && vnum <= pArea->hi_vnum )
		return true;

	send_to_char( "That room is not in your allocated range.\r\n", ch );
	return false;
}

bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj )
{
	int vnum = obj->pIndexData->vnum;
	AREA_DATA *pArea;

	if( IS_NPC( ch ) )
		return false;
	if( get_trust( ch ) >= sysdata.level_modify_proto )
		return true;
	if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
	{
		send_to_char( "You must have an assigned area to modify this object.\r\n", ch );
		return false;
	}
	if( vnum >= pArea->low_vnum && vnum <= pArea->hi_vnum )
		return true;

	send_to_char( "That object is not in your allocated range.\r\n", ch );
	return false;
}

bool can_oedit( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
	int vnum = obj->vnum;
	AREA_DATA *pArea;

	if( IS_NPC( ch ) )
		return false;
	if( get_trust( ch ) >= LEVEL_LIAISON )
		return true;
	if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
	{
		send_to_char( "You must have an assigned area to modify this object.\r\n", ch );
		return false;
	}
	if( vnum >= pArea->low_vnum && vnum <= pArea->hi_vnum )
		return true;

	send_to_char( "That object is not in your allocated range.\r\n", ch );
	return false;
}


bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob )
{
	int vnum;
	AREA_DATA *pArea;

	if( mob == ch )
		return true;

	if( !IS_NPC( mob ) )
	{
		if( get_trust( ch ) >= sysdata.level_modify_proto && get_trust( ch ) > get_trust( mob ) )
			return true;
		else
			send_to_char( "You can't do that.\r\n", ch );
		return false;
	}

	vnum = mob->pIndexData->vnum;

	if( IS_NPC( ch ) )
		return false;
	if( get_trust( ch ) >= sysdata.level_modify_proto )
		return true;
	if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
	{
		send_to_char( "You must have an assigned area to modify this mobile.\r\n", ch );
		return false;
	}
	if( vnum >= pArea->low_vnum && vnum <= pArea->hi_vnum )
		return true;

	send_to_char( "That mobile is not in your allocated range.\r\n", ch );
	return false;
}

bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob )
{
	int vnum = mob->vnum;
	AREA_DATA *pArea;

	if( IS_NPC( ch ) )
		return false;
	if( get_trust( ch ) >= LEVEL_LIAISON )
		return true;
	if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
	{
		send_to_char( "You must have an assigned area to modify this mobile.\r\n", ch );
		return false;
	}
	if( vnum >= pArea->low_vnum && vnum <= pArea->hi_vnum )
		return true;

	send_to_char( "That mobile is not in your allocated range.\r\n", ch );
	return false;
}

int get_otype( const char *type )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( o_types ) / sizeof( o_types[0] ) ); x++ )
		if( !str_cmp( type, o_types[x] ) )
			return x;
	return -1;
}

int get_aflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, a_flags[x] ) )
			return x;
	return -1;
}

int get_trapflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, trap_flags[x] ) )
			return x;
	return -1;
}

int get_cybaflags( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, cyba_flags[x] ) )
			return x;
	return -1;
}

int get_atype( const char *type )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( a_types ) / sizeof( a_types[0] ) ); x++ )
		if( !str_cmp( type, a_types[x] ) )
			return x;
	return -1;
}

int get_pc_race( const char *type )
{
	int x;

	for( x = 0; x < MAX_RACE; x++ )
		if( !str_cmp( type, pc_race[x] ) )
			return x;
	return -1;
}

int get_npc_race( const char *type )
{
	int x;

	for( x = 0; x < MAX_NPC_RACE; x++ )
		if( !str_cmp( type, npc_race[x] ) )
			return x;
	return -1;
}

int get_element( const char *type )
{
	int i;

	for( i = 0; i < MAX_ELEMENT; i++ )
		if( !str_cmp( type, element_table[i] ) )
			return i;
	return -1;
}

int get_wearloc( const char *type )
{
	int x;

	for( x = 0; x < MAX_WEAR; x++ )
		if( !str_cmp( type, wear_locs[x] ) )
			return x;
	return -1;
}

int get_exflag( const char *flag )
{
	int x;

	for( x = 0; x <= MAX_EXFLAG; x++ )
		if( !str_cmp( flag, ex_flags[x] ) )
			return x;
	return -1;
}

int get_rflag( const char *flag )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( r_flags ) / sizeof( r_flags[0] ) ); x++ )
		if( !str_cmp( flag, r_flags[x] ) )
			return x;
	return -1;
}

int get_mpflag( const char *flag )
{
	unsigned int x;

	for( x = 0; ( sizeof( mprog_flags ) / sizeof( mprog_flags[0] ) ); x++ )
		if( !str_cmp( flag, mprog_flags[x] ) )
			return x;
	return -1;
}

int get_oflag( const char *flag )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( o_flags ) / sizeof( o_flags[0] ) ); x++ )
		if( !str_cmp( flag, o_flags[x] ) )
			return x;
	return -1;
}

int get_areaflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, area_flags[x] ) )
			return x;
	return -1;
}

int get_wflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, w_flags[x] ) )
			return x;
	return -1;
}

int get_actflag( const char *flag )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( act_flags ) / sizeof( act_flags[0] ) ); x++ )
		if( !str_cmp( flag, act_flags[x] ) )
			return x;
	return -1;
}

int get_vip_flag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, planet_flags[x] ) )
			return x;
	return -1;
}

int get_wanted_flag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, planet_flags[x] ) )
			return x;
	return -1;
}

int get_pcflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, pc_flags[x] ) )
			return x;
	return -1;
}
int get_plrflag( const char *flag )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( plr_flags ) / sizeof( plr_flags[0] ) ); x++ )
		if( !str_cmp( flag, plr_flags[x] ) )
			return x;
	return -1;
}

int get_risflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, ris_flags[x] ) )
			return x;
	return -1;
}

int get_trigflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, trig_flags[x] ) )
			return x;
	return -1;
}

int get_partflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, part_flags[x] ) )
			return x;
	return -1;
}

int get_attackflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, attack_flags[x] ) )
			return x;
	return -1;
}

int get_defenseflag( const char *flag )
{
	int x;

	for( x = 0; x < 32; x++ )
		if( !str_cmp( flag, defense_flags[x] ) )
			return x;
	return -1;
}

int get_secflag( const char *flag )
{
	unsigned int x;

	for( x = 0; x < ( sizeof( sector_name ) / sizeof( sector_name[0] ) ); x++ )
		if( !str_cmp( flag, sector_name[x] ) )
			return x;
	return -1;
}

int get_npc_position( const char *position )
{
	size_t x;

	for( x = 0; x <= POS_DRAG; x++ )
		if( !str_cmp( position, npc_position[x] ) )
			return x;
	return -1;
}

int get_npc_sex( const char *sex )
{
	size_t x;

	for( x = 0; x <= SEX_FEMALE; x++ )
		if( !str_cmp( sex, npc_sex[x] ) )
			return x;
	return -1;
}


/*
 * Remove carriage returns from a line
 */
char *strip_cr( const char *str )
{
	static char newstr[MAX_STRING_LENGTH];
	int i, j;

	if( !str || str[0] == '\0' )
	{
		newstr[0] = '\0';
		return newstr;
	}

	for( i = j = 0; str[i] != '\0'; i++ )
	{
		if( str[i] != '\r' )
		{
			newstr[j++] = str[i];
		}
	}
	newstr[j] = '\0';
	return newstr;
}

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
	int len;
	char last;
	char *strptr;

	strptr = str;

	len = strlen( str );
	if( len )
		last = strptr[len - 1];
	else
		last = '\0';

	for( ; *str != '\0'; str++ )
	{
		if( *str == '~' )
			*str = '-';
	}
	if( len )
		strptr[len - 1] = last;

	return;
}


void start_editing( CHAR_DATA *ch, const char *data )
{
	EDITOR_DATA *edit;
	short lines, size, lpos;
	char c;

	if( !ch->desc )
	{
		bug( "%s: Fatal: start_editing: no desc", __func__ );
		return;
	}
	if( ch->substate == SUB_RESTRICTED )
		bug( "%s: NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", __func__ );

	set_char_color( AT_GREEN, ch );
	send_to_char( "Begin entering your text (/? =help /s =save /c =clear /l =list /f =format)\r\n", ch );
	send_to_char( "--------------------------------------------------------------------------\r\n> ", ch );
	if( ch->editor )
		stop_editing( ch );

	CREATE( edit, EDITOR_DATA, 1 );
	edit->numlines = 0;
	edit->on_line = 0;
	edit->size = 0;
	size = 0;
	lpos = 0;
	lines = 0;
	if( !data )
		bug( "%s: data is NULL!", __func__ );
	else
		for( ;; )
		{
			c = data[size++];
			if( c == '\0' )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
			else if( c == '\r' );
			else if( c == '\n' || lpos > 78 )
			{
				edit->line[lines][lpos] = '\0';
				lines++;
				lpos = 0;
			}
			else
				edit->line[lines][lpos++] = c;
			if( lines >= 49 || size > 4096 )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
		}
	edit->numlines = lines;
	edit->size = size;
	edit->on_line = lines;
	ch->editor = edit;
	ch->desc->connected = CON_EDITING;
}

const char *copy_buffer( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	char tmp[100];
	short x, len;

	if( !ch )
	{
		bug( "%s: null ch", __func__ );
		return STRALLOC( "" );
	}

	if( !ch->editor )
	{
		bug( "%s: null editor", __func__ );
		return STRALLOC( "" );
	}

	buf[0] = '\0';
	for( x = 0; x < ch->editor->numlines; x++ )
	{
		mudstrlcpy( tmp, ch->editor->line[x], 100 );
		smush_tilde( tmp );
		len = strlen( tmp );
		if( tmp[len - 1] == '~' )
			tmp[len - 1] = '\0';
		else
			mudstrlcat( tmp, "\n", 100 );
		mudstrlcat( buf, tmp, MAX_STRING_LENGTH );
	}
	return STRALLOC( buf );
}

void stop_editing( CHAR_DATA *ch )
{
	set_char_color( AT_PLAIN, ch );
	DISPOSE( ch->editor );
	ch->editor = NULL;
	send_to_char( "Done.\r\n", ch );
	ch->dest_buf = NULL;
	ch->spare_ptr = NULL;
	ch->substate = SUB_NONE;
	if( !ch->desc )
	{
		bug( "Fatal: %s: no desc", __func__ );
		return;
	}
	ch->desc->connected = CON_PLAYING;
}

CMDF( do_goto )
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;
	ROOM_INDEX_DATA *in_room;
	AREA_DATA *pArea;
	int vnum;

	if( IS_NPC( ch ) )
		return;

	one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Goto where?\r\n", ch );
		return;
	}


	if( ( location = find_location( ch, arg ) ) == NULL )
	{
		vnum = atoi( arg );
		if( vnum < 0 || get_room_index( vnum ) )
		{
			send_to_char( "You cannot find that...\r\n", ch );
			return;
		}

		if( vnum < 1 || IS_NPC( ch ) || !ch->pcdata->area )
		{
			send_to_char( "No such location.\r\n", ch );
			return;
		}
		if( get_trust( ch ) < sysdata.level_modify_proto )
		{
			if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
			{
				send_to_char( "You must have an assigned area to create rooms.\r\n", ch );
				return;
			}
			if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
			{
				send_to_char( "That room is not within your assigned range.\r\n", ch );
				return;
			}
		}
		if( !is_valid_vnum( vnum, VCHECK_ROOM ) )
		{
			ch_printf( ch, "&RSorry, %d is not a valid vnum!\r\n", vnum );
			return;
		}
		location = make_room( vnum, ch->pcdata->area );
		if( !location )
		{
			bug( "Goto: make_room failed", 0 );
			return;
		}
		location->area = ch->pcdata->area;
		set_char_color( AT_WHITE, ch );
		send_to_char( "Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\r\n", ch );
	}

	if( room_is_private( ch, location ) )
	{
		if( get_trust( ch ) < sysdata.level_override_private )
		{
			send_to_char( "That room is private right now.\r\n", ch );
			return;
		}
		else
			send_to_char( "Overriding private flag!\r\n", ch );
	}

	if( get_trust( ch ) < LEVEL_STAFF )
	{
		vnum = atoi( arg );

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to goto.\r\n", ch );
			return;
		}

		if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
		{
			send_to_char( "That room is not within your assigned range.\r\n", ch );
			return;
		}

		if( ( ch->in_room->vnum < pArea->low_vnum
			|| ch->in_room->vnum > pArea->hi_vnum ) && !xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
		{
			send_to_char( "Builders can only use goto from a hotel or in their zone.\r\n", ch );
			return;
		}

	}

	in_room = ch->in_room;
	if( ch->fighting )
		stop_fighting( ch, true );

	if( !xIS_SET( ch->act, PLR_WIZINVIS ) )
	{
		if( ch->pcdata && ch->pcdata->bamfout[0] != '\0' )
			act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfout, TO_ROOM );
		else
			act( AT_IMMORT, "$n $T", ch, NULL, "leaves in a swirl of mist.", TO_ROOM );

	}
	ch->regoto = ch->in_room->vnum;
	char_from_room( ch );
	if( ch->mount )
	{
		char_from_room( ch->mount );
		char_to_room( ch->mount, location );
	}
	char_to_room( ch, location );

	if( !xIS_SET( ch->act, PLR_WIZINVIS ) )
	{
		if( ch->pcdata && ch->pcdata->bamfin[0] != '\0' )
			act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfin, TO_ROOM );
		else
			act( AT_IMMORT, "$n $T", ch, NULL, "enters in a swirl of mist.", TO_ROOM );
	}


	do_look( ch, "auto" );

	if( ch->in_room == in_room )
		return;
	for( fch = in_room->first_person; fch; fch = fch_next )
	{
		fch_next = fch->next_in_room;
		if( fch->master == ch && IS_IMMORTAL( fch ) )
		{
			act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
			do_goto( fch, argument );
		}
	}
	return;
}

CMDF( do_mset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	int num, size, plus;
	char char1, char2;
	CHAR_DATA *victim;
	int value;
	int minattr, maxattr;
	bool lockvictim;
	const char *origarg = argument;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't mset\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;
	case SUB_MOB_DESC:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		victim = ( CHAR_DATA * ) ch->dest_buf;
		if( char_died( victim ) )
		{
			send_to_char( "Your victim died!\r\n", ch );
			stop_editing( ch );
			return;
		}
		STRFREE( victim->description );
		victim->description = copy_buffer( ch );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE( victim->pIndexData->description );
			victim->pIndexData->description = QUICKLINK( victim->description );
		}
		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	}

	victim = NULL;
	lockvictim = false;
	argument = smash_tilde_static( argument );

	if( ch->substate == SUB_REPEATCMD )
	{
		victim = ( CHAR_DATA * ) ch->dest_buf;
		if( char_died( victim ) )
		{
			send_to_char( "Your victim died!\r\n", ch );
			victim = NULL;
			argument = "done";
		}
		if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
		{
			if( victim )
				do_mstat( ch, victim->name );
			else
				send_to_char( "No victim selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
		{
			send_to_char( "Mset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			return;
		}
	}
	if( victim )
	{
		lockvictim = true;
		mudstrlcpy( arg1, victim->name, MSL );
		argument = one_argument( argument, arg2 );
		mudstrlcpy( arg3, argument, MSL );
	}
	else
	{
		lockvictim = false;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		mudstrlcpy( arg3, argument, MSL );
	}
	/*
		if ( !str_cmp( arg1, "on" ) )
		{
		send_to_char( "Syntax: mset <victim|vnum> on.\r\n", ch );
		return;
		}
	*/
	if( arg1[0] == '\0' || ( arg2[0] == '\0' && ch->substate != SUB_REPEATCMD ) || !str_cmp( arg1, "?" ) )
	{
		if( ch->substate == SUB_REPEATCMD )
		{
			if( victim )
				send_to_char( "Syntax: <field>  <value>\r\n", ch );
			else
				send_to_char( "Syntax: <victim> <field>  <value>\r\n", ch );
		}
		else
			send_to_char( "Syntax: mset <victim> <field>  <value>\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Field being one of:\r\n", ch );
		send_to_char( "  str int wis dex con cha lck sex\r\n", ch );
		send_to_char( "  dollars hp force move align race\r\n", ch );
		send_to_char( "  hitroll damroll armor affected level\r\n", ch );
		send_to_char( "  thirst drunk full blood flags\r\n", ch );
		send_to_char( "  pos defpos\r\n", ch );
		send_to_char( "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\r\n", ch );
		send_to_char( "  resistant immune susceptible (see RIS)\r\n", ch );
		send_to_char( "  attack defense numattacks\r\n", ch );
		send_to_char( "  name short long description title spec spec2\r\n", ch );
		send_to_char( "  clan vip wanted\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "For editing index/prototype mobiles:\r\n", ch );
		send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\r\n", ch );
		send_to_char( "  damnumdie damsizedie damplus (damage roll)\r\n", ch );
		send_to_char( "To toggle area flag: aloaded\r\n", ch );
		return;
	}

	if( !victim && get_trust( ch ) <= LEVEL_STAFF )
	{
		if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}
	else if( !victim )
	{
		if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}

	if( get_trust( ch ) <= LEVEL_STAFF && !IS_NPC( victim ) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		ch->dest_buf = NULL;
		return;
	}
	if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		ch->dest_buf = NULL;
		return;
	}
	if( lockvictim )
		ch->dest_buf = victim;

	if( IS_NPC( victim ) )
	{
		minattr = 1;
		maxattr = 25;
	}
	else
	{
		minattr = 3;
		maxattr = 18;
	}

	if( !str_cmp( arg2, "on" ) )
	{
		CHECK_SUBRESTRICTED( ch );
		ch_printf( ch, "Mset mode on. (Editing %s).\r\n", victim->name );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = victim;
		if( ch->pcdata )
		{
			if( ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			if( IS_NPC( victim ) )
				sprintf( buf, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum );
			else
				sprintf( buf, "<&CMset &W%s&w> %%i", victim->name );
			ch->pcdata->subprompt = STRALLOC( buf );
		}
		RelCreate( relMSET_ON, ch, victim );
		return;
	}

	value = is_number( arg3 ) ? atoi( arg3 ) : -1;

	if( atoi( arg3 ) < -1 && value == -1 )
		value = atoi( arg3 );

	if( !str_cmp( arg2, "str" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Strength range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_str = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_str = value;
		return;
	}

	if( !str_cmp( arg2, "int" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Intelligence range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_int = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_int = value;
		return;
	}

	if( !str_cmp( arg2, "wis" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Wisdom range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_wis = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_wis = value;
		return;
	}

	if( !str_cmp( arg2, "dex" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Dexterity range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_dex = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_dex = value;
		return;
	}

	if( !str_cmp( arg2, "con" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Constitution range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_con = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_con = value;
		return;
	}

	if( !str_cmp( arg2, "cha" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Charisma range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_cha = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_cha = value;
		return;
	}

	if( !str_cmp( arg2, "lck" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Luck range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_lck = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_lck = value;
		return;
	}

	if( !str_cmp( arg2, "sav1" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range vs poison is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_poison_death = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_poison_death = value;
		return;
	}

	if( !str_cmp( arg2, "sav2" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range vs wands is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_wand = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_wand = value;
		return;
	}

	if( !str_cmp( arg2, "sav3" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range vs para is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_para_petri = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_para_petri = value;
		return;
	}

	if( !str_cmp( arg2, "sav4" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range vs bad breath is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_breath = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_breath = value;
		return;
	}

	if( !str_cmp( arg2, "sav5" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range vs force powers is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_spell_staff = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_spell_staff = value;
		return;
	}

	if( !str_cmp( arg2, "toplevel" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 1 || value > 1000 )
		{
			send_to_char( "Top level range: 1 - 1000.\r\n", ch );
			return;
		}
		victim->top_level = value;
		return;
	}

	if( !str_cmp( arg2, "sex" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 2 )
		{
			send_to_char( "Sex range is 0 to 2.\r\n", ch );
			return;
		}
		victim->sex = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->sex = value;
		return;
	}

	if( !str_cmp( arg2, "race" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( IS_NPC( victim ) )
			value = get_npc_race( arg3 );
		else
			value = get_pc_race( arg3 );
		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= MAX_RACE ) )
		{
			ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE - 1 );
			return;
		}
		if( IS_NPC( victim ) && ( value < 0 || value >= MAX_NPC_RACE ) )
		{
			ch_printf( ch, "Race range is 0 to %d.\n", MAX_NPC_RACE - 1 );
			return;
		}
		victim->race = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->race = value;
		return;
	}

	if( !str_cmp( arg2, "hrank" ) )
	{

		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= 5 ) )
		{
			ch_printf( ch, "Argument must be numerical.\n0 = E   1 = D   2 = C\n3 = B   4 = A   5 = S\r\n" );

			return;
		}
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		victim->pcdata->hlevel = value;
		return;
	}

	if( !str_cmp( arg2, "hair" ) )
	{
		value = get_npc_race( arg3 );
		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= 23 ) )
		{
			ch_printf( ch, "Hair number range is 0 to 23.\n" );
			ch_printf( ch, "Help hair for number ranges.\n" );
			return;
		}
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		victim->pcdata->hair = value;
		return;
	}

	if( !str_cmp( arg2, "highlights" ) )
	{
		value = get_npc_race( arg3 );
		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= 23 ) )
		{
			ch_printf( ch, "Highlight number range is 0 to 23.\n" );
			ch_printf( ch, "Help highlight for number ranges.\n" );
			return;
		}
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		victim->pcdata->highlight = value;
		return;
	}

	if( !str_cmp( arg2, "eye" ) )
	{
		value = get_npc_race( arg3 );
		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= 15 ) )
		{
			ch_printf( ch, "Eye colour range is 0 to 15.\n" );
			ch_printf( ch, "Help eye for number ranges.\n" );
			return;
		}
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		victim->pcdata->eye = value;
		return;
	}

	if( !str_cmp( arg2, "build" ) )
	{
		value = get_npc_race( arg3 );
		if( value < 0 )
			value = atoi( arg3 );
		if( !IS_NPC( victim ) && ( value < 0 || value >= 8 ) )
		{
			ch_printf( ch, "Build colour range is 0 to 8.\n" );
			ch_printf( ch, "Help bodysize for number ranges.\n" );
			return;
		}
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}
		victim->pcdata->build = value;
		return;
	}


	if( !str_cmp( arg2, "armor" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -30000 || value > 30000 )
		{
			send_to_char( "AC range is -30000 to 30000.\r\n", ch );
			return;
		}
		victim->armor = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->ac = value;
		return;
	}

	if( !str_cmp( arg2, "level" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > LEVEL_AVATAR + 5 )
		{
			ch_printf( ch, "Level range is 0 to %d.\r\n", LEVEL_AVATAR + 5 );
			return;
		}
		{
			int ability;
			for( ability = 0; ability < MAX_ABILITY; ability++ )
				victim->skill_level[ability] = value;
		}
		victim->top_level = value;
		victim->armor = 100 - value * 2.5;
		victim->hitroll = value / 5;
		victim->damroll = value / 5;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->level = value;
			victim->pIndexData->ac = 100 - value * 2.5;
			victim->pIndexData->hitroll = victim->hitroll;
			victim->pIndexData->damroll = victim->damroll;
		}
		sprintf( outbuf, "%s damnumdie %d", arg1, value / 10 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s damsizedie %d", arg1, 4 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s damplus %d", arg1, 2 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s hitnumdie %d", arg1, value / 5 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s hitsizedie %d", arg1, 10 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s hitplus %d", arg1, value * 10 );
		do_mset( ch, outbuf );

		return;
	}

	if( !str_cmp( arg2, "numattacks" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 20 )
		{
			send_to_char( "Attacks range is 0 to 20.\r\n", ch );
			return;
		}
		victim->numattacks = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->numattacks = value;
		return;
	}

	if( !str_cmp( arg2, "dollars" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		victim->gold = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->gold = value;
		return;
	}

	if( !str_cmp( arg2, "bank" ) )
	{
		if( IS_NPC( victim ) )
			return;

		if( value < 0 || value > 5000 )
		{
			send_to_char( "Sorry bucko, money is from 0 to 5000!\r\n", ch );
			return;
		}
		victim->pcdata->bank = value;
		return;
	}

	if( !str_cmp( arg2, "home" ) )
	{
		if( IS_NPC( victim ) )
			return;

		victim->plr_home = NULL;
		return;
	}

	if( !str_cmp( arg2, "hitroll" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		victim->hitroll = URANGE( 0, value, 85 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitroll = victim->hitroll;
		return;
	}

	if( !str_cmp( arg2, "damroll" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		victim->damroll = URANGE( 0, value, 65 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damroll = victim->damroll;
		return;
	}

	if( !str_cmp( arg2, "hp" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 1 || value > 1000000 )
		{
			send_to_char( "Hp range is 1 to 1,000,000 hit points.\r\n", ch );
			return;
		}
		victim->max_hit = value;
		return;
	}

	if( !str_cmp( arg2, "nextquest" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 100 )
		{
			send_to_char( "Let's be responible, hmm?", ch );
			victim->nextquest = 0;
			return;
		}
		victim->nextquest = value;
		return;
	}

	if( !str_cmp( arg2, "questpoints" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 2000 )
		{
			send_to_char( "Let's be responible, hmm?", ch );
			victim->questpoints = 0;
			return;
		}
		victim->questpoints = value;
		return;
	}

	if( !str_cmp( arg2, "energy" ) )
	{
		if( IS_NPC( victim ) )
			return;
		if( value < 0 || value > 30000 )
		{
			send_to_char( "Energy is 0 to 10,000.\r\n", ch );
			return;
		}
		victim->pcdata->xenergy = value;
		victim->pcdata->xenergymax = value;
		return;
	}

	if( !str_cmp( arg2, "move" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 30000 )
		{
			send_to_char( "Move range is 0 to 30,000 move points.\r\n", ch );
			return;
		}
		victim->max_move = value;
		return;
	}

	if( !str_cmp( arg2, "align" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < -1000 || value > 1000 )
		{
			send_to_char( "Alignment range is -1000 to 1000.\r\n", ch );
			return;
		}
		victim->alignment = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->alignment = value;
		return;
	}

	if( !str_cmp( arg2, "password" ) )
	{
		const char *pwdnew;

		if( get_trust( ch ) < LEVEL_BUILDER )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) )
		{
			send_to_char( "Mobs don't have passwords.\r\n", ch );
			return;
		}

		if( strlen( arg3 ) < 5 )
		{
			send_to_char( "New password must be at least five characters long.\r\n", ch );
			return;
		}

		pwdnew = sha256_crypt( arg3 );    /* SHA-256 Encryption */

		DISPOSE( victim->pcdata->pwd );
		victim->pcdata->pwd = str_dup( pwdnew );
		if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
			save_char_obj( victim );
		send_to_char( "Ok.\r\n", ch );
		ch_printf( victim, "Your password has been changed by %s.\r\n", ch->name );
		return;
	}

	if( !str_cmp( arg2, "quest" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 500 )
		{
			send_to_char( "The current quest range is 0 to 500.\r\n", ch );
			return;
		}

		victim->pcdata->quest_number = value;
		return;
	}

	if( !str_cmp( arg2, "qpa" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		victim->pcdata->quest_accum = value;
		return;
	}

	if( !str_cmp( arg2, "qp" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 5000 )
		{
			send_to_char( "The current quest point range is 0 to 5000.\r\n", ch );
			return;
		}

		victim->pcdata->quest_curr = value;
		return;
	}

	if( !str_cmp( arg2, "mentalstate" ) )
	{
		if( value < -100 || value > 100 )
		{
			send_to_char( "Value must be in range -100 to +100.\r\n", ch );
			return;
		}
		victim->mental_state = value;
		return;
	}

	if( !str_cmp( arg2, "emotion" ) )
	{
		if( value < -100 || value > 100 )
		{
			send_to_char( "Value must be in range -100 to +100.\r\n", ch );
			return;
		}
		victim->emotional_state = value;
		return;
	}

	if( !str_cmp( arg2, "thirst" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 100 )
		{
			send_to_char( "Thirst range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		return;
	}

	if( !str_cmp( arg2, "drunk" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 100 )
		{
			send_to_char( "Drunk range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}

	if( !str_cmp( arg2, "full" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > 100 )
		{
			send_to_char( "Full range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		return;
	}

	if( !str_cmp( arg2, "blood" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( value < 0 || value > MAX_LEVEL + 10 )
		{
			ch_printf( ch, "Blood range is 0 to %d.\r\n", MAX_LEVEL + 10 );
			return;
		}

		victim->pcdata->condition[COND_BLOODTHIRST] = value;
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		if( !can_mmodify( ch, victim ) )
			return;
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if( arg3[0] == '\0' )
		{
			send_to_char( "Names can not be set to an empty string.\r\n", ch );
			return;
		}

		STRFREE( victim->name );
		victim->name = STRALLOC( arg3 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE( victim->pIndexData->player_name );
			victim->pIndexData->player_name = QUICKLINK( victim->name );
		}
		return;
	}

	if( !str_cmp( arg2, "minsnoop" ) )
	{
		if( get_trust( ch ) < LEVEL_BUILDER )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}
		if( victim->pcdata )
		{
			victim->pcdata->min_snoop = value;
			return;
		}
	}

	if( !str_cmp( arg2, "gang" ) )
	{
		CLAN_DATA *clan;

		if( get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if( arg3[0] == '\0' )
		{
			if( victim->pcdata->clan )
			{
				remove_member( victim->name, victim->pcdata->clan->shortname );
			}
			if( victim->pcdata->salary )
				victim->pcdata->salary = 0;
			STRFREE( victim->pcdata->clan_name );
			victim->pcdata->clan_name = STRALLOC( "" );
			victim->pcdata->clan = NULL;
			send_to_char( "Removed from gang.\r\n", ch );
			return;
		}
		clan = get_clan( arg3 );
		if( !clan )
		{
			send_to_char( "No such gang.\r\n", ch );
			return;
		}
		if( victim->pcdata->clan )
			remove_member( victim->name, victim->pcdata->clan->shortname );
		clan->members -= 1;

		STRFREE( victim->pcdata->clan_name );
		victim->pcdata->clan_name = QUICKLINK( clan->name );
		victim->pcdata->clan = clan;
		add_member( victim->name, victim->pcdata->clan->shortname );
		clan->members += 1;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "spouse" ) )
	{
		if( IS_NPC( victim ) )
		{
			ch_printf( ch, "Not on NPC's.\n" );
			return;
		}

		if( arg3[0] == '\0' )
		{
			STRFREE( victim->pcdata->spouse );
			victim->pcdata->spouse = STRALLOC( "" );
			send_to_char( "Spouse set to None.\r\n", ch );
			return;
		}

		STRFREE( victim->pcdata->spouse );
		victim->pcdata->spouse = STRALLOC( arg3 );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "short" ) )
	{
		STRFREE( victim->short_descr );
		victim->short_descr = STRALLOC( arg3 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE( victim->pIndexData->short_descr );
			victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
		}
		return;
	}

	if( !str_cmp( arg2, "long" ) )
	{
		STRFREE( victim->long_descr );
		mudstrlcpy( buf, arg3, MSL );
		mudstrlcat( buf, "\r\n", MSL );
		victim->long_descr = STRALLOC( buf );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE( victim->pIndexData->long_descr );
			victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
		}
		return;
	}

	if( !str_cmp( arg2, "job" ) )
	{
		if( IS_NPC( victim ) || !IS_IMMORTAL( victim ) )
			return;

		DISPOSE( victim->pcdata->job );
		victim->pcdata->job = str_dup( arg3 );
		send_to_char( "Done.", ch );
		return;
	}

	if( !str_cmp( arg2, "description" ) )
	{
		if( arg3[0] )
		{
			STRFREE( victim->description );
			victim->description = STRALLOC( arg3 );
			if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			{
				STRFREE( victim->pIndexData->description );
				victim->pIndexData->description = QUICKLINK( victim->description );
			}
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_MOB_DESC;
		ch->dest_buf = victim;
		start_editing( ch, victim->description );
		return;
	}

	if( !str_cmp( arg2, "title" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		set_title( victim, arg3 );
		return;
	}

	if( !str_cmp( arg2, "rank" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}
		argument = smash_tilde_static( argument );
		STRFREE( victim->pcdata->rank );
		if( !str_cmp( argument, "none" ) )
			victim->pcdata->rank = STRALLOC( "" );
		else
			victim->pcdata->rank = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
	}

	if( !str_cmp( arg2, "spec" ) || !str_cmp( arg2, "spec_fun" ) )
	{
		SPEC_FUN *specfun;

		if( !can_mmodify( ch, victim ) )
			return;

		if( !IS_NPC( victim ) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if( !str_cmp( arg3, "none" ) )
		{
			victim->spec_fun = NULL;
			STRFREE( victim->spec_funname );
			send_to_char( "Special function removed.\r\n", ch );
			if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			{
				victim->pIndexData->spec_fun = NULL;
				STRFREE( victim->pIndexData->spec_funname );
			}
			return;
		}

		if( ( specfun = spec_lookup( arg3 ) ) == NULL )
		{
			send_to_char( "No such function.\r\n", ch );
			return;
		}

		if( !validate_spec_fun( arg3 ) )
		{
			ch_printf( ch, "%s is not a valid spec_fun for mobiles.\r\n", arg3 );
			return;
		}

		victim->spec_fun = specfun;
		STRFREE( victim->spec_funname );
		victim->spec_funname = STRALLOC( arg3 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->spec_fun = victim->spec_fun;
			STRFREE( victim->pIndexData->spec_funname );
			victim->pIndexData->spec_funname = STRALLOC( arg3 );
		}
		send_to_char( "Victim special function set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "spec2" ) || !str_cmp( arg2, "spec_fun2" ) )
	{
		SPEC_FUN *specfun;

		if( !can_mmodify( ch, victim ) )
			return;

		if( !IS_NPC( victim ) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if( !str_cmp( arg3, "none" ) )
		{
			victim->spec_2 = NULL;
			STRFREE( victim->spec_funname2 );
			send_to_char( "Special function 2 removed.\r\n", ch );
			if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			{
				victim->pIndexData->spec_2 = NULL;
				STRFREE( victim->pIndexData->spec_funname2 );
			}
			return;
		}

		if( ( specfun = spec_lookup( arg3 ) ) == NULL )
		{
			send_to_char( "No such function.\r\n", ch );
			return;
		}

		if( !validate_spec_fun( arg3 ) )
		{
			ch_printf( ch, "%s is not a valid spec_fun for mobiles.\r\n", arg3 );
			return;
		}

		victim->spec_2 = specfun;
		STRFREE( victim->spec_funname2 );
		victim->spec_funname2 = STRALLOC( arg3 );
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->spec_2 = victim->spec_2;
			STRFREE( victim->pIndexData->spec_funname2 );
			victim->pIndexData->spec_funname2 = STRALLOC( arg3 );
		}
		send_to_char( "Victim special function set.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "xbuster" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "XBuster flags are for player only.\r\n", ch );
			return;
		}

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> xbuster <attachment>\r\n", ch );
			return;
		}

		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_cybaflags( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->pcdata->cybaflags, 1 << value );
		}
		return;
	}

	if( !str_cmp( arg2, "flags" ) )
	{
		bool protoflag = false, ftoggle = false;

		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> flags <flag> [flag]...\r\n", ch );
			send_to_char( "sentinal, scavenger, aggressive, stayarea, wimpy, practice, immortal,\r\n", ch );
			send_to_char( "deadly, mountable, guardian, nokill, scholar, noassist, droid, nocorpse,\r\n", ch );
			send_to_char( "pk", ch );
			return;
		}

		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			protoflag = true;

		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );

			if( IS_NPC( victim ) )
			{
				value = get_actflag( arg3 );

				if( value < 0 || value >= 31 )
					ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
				else if( 1 << value == ACT_PROTOTYPE && ch->top_level < sysdata.level_modify_proto )
					send_to_char( "You cannot change the prototype flag.\r\n", ch );
				else if( 1 << value == ACT_IS_NPC )
					send_to_char( "If the NPC flag could be changed, it would cause many problems.\r\n", ch );
				else
					xTOGGLE_BIT( victim->act, 1 << value );
			}
			else
			{
				value = get_plrflag( arg3 );

				if( value < 0 || value >= 31 )
					ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
				else if( 1 << value == ACT_IS_NPC )
					send_to_char( "If the NPC flag could be changed, it would cause many problems.\r\n", ch );
				else
				{
					ftoggle = true;
					xTOGGLE_BIT( victim->act, 1 << value );
				}
			}
		}
		if( ftoggle )
			send_to_char( "Flags set.\r\n", ch );
		if( IS_NPC( victim ) && ( xIS_SET( victim->act, ACT_PROTOTYPE ) || ( value == ACT_PROTOTYPE && protoflag ) ) )
			victim->pIndexData->act = victim->act;
		return;
	}


	if( !str_cmp( arg2, "wanted" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Wanted flags are for players only.\r\n", ch );
			return;
		}

		if( get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You are not a high enough level to do that.\r\n", ch );
			return;
		}

		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> wanted <planet> [planet]...\r\n", ch );
			return;
		}

		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_wanted_flag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->pcdata->wanted_flags, 1 << value );
		}
		return;
	}

	if( !str_cmp( arg2, "vip" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "VIP flags are for mobs only.\r\n", ch );
			return;
		}

		if( !can_mmodify( ch, victim ) )
			return;

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> vip <planet> [planet]...\r\n", ch );
			return;
		}

		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_vip_flag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->vip_flags, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->vip_flags = victim->vip_flags;
		return;
	}


	if( !str_cmp( arg2, "affected" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}

		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> affected <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_aflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->affected_by, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->affected_by = victim->affected_by;
		return;
	}

	/*
	 * save some more finger-leather for setting RIS stuff
	 */
	if( !str_cmp( arg2, "r" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "i" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;


		sprintf( outbuf, "%s immune %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "s" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "ri" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s immune %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "rs" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "is" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s immune %s", arg1, arg3 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "ris" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sprintf( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s immune %s", arg1, arg3 );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "resistant" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's resistancies.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> resistant <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_risflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->resistant, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->resistant = victim->resistant;
		return;
	}

	if( !str_cmp( arg2, "immune" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's immunities.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> immune <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_risflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->immune, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->immune = victim->immune;
		return;
	}

	if( !str_cmp( arg2, "susceptible" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's susceptibilities.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> susceptible <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_risflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->susceptible, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->susceptible = victim->susceptible;
		return;
	}

	if( !str_cmp( arg2, "part" ) )
	{
		if( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only modify a mobile's parts.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> part <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_partflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->xflags, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->xflags = victim->xflags;
		return;
	}

	if( !str_cmp( arg2, "attack" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "You can only modify a mobile's attacks.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> attack <flag> [flag]...\r\n", ch );
			send_to_char( "bite          claws        tail        sting      punch        kick\r\n", ch );
			send_to_char( "trip          bash         stun        gouge      backstab\r\n", ch );
			send_to_char( "circle        poison\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_attackflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->attacks, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->attacks = victim->attacks;
		return;
	}

	if( !str_cmp( arg2, "defense" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "You can only modify a mobile's defenses.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> defense <flag> [flag]...\r\n", ch );
			send_to_char( "parry, dodge, disarm, grip\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_defenseflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( victim->defenses, 1 << value );
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defenses = victim->defenses;
		return;
	}

	if( !str_cmp( arg2, "pos" ) )
	{
		/*
			if ( !IS_NPC(victim) )
			{
				send_to_char( "Mobiles only.\r\n", ch );
				return;
			}
		*/
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > POS_STANDING )
		{
			ch_printf( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->position = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->position = victim->position;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "defpos" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > POS_STANDING )
		{
			ch_printf( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->defposition = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defposition = victim->defposition;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	 * save some finger-leather
	 */
	if( !str_cmp( arg2, "hitdie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
		sprintf( outbuf, "%s hitnumdie %d", arg1, num );
		do_mset( ch, outbuf );

		sprintf( outbuf, "%s hitsizedie %d", arg1, size );
		do_mset( ch, outbuf );

		sprintf( outbuf, "%s hitplus %d", arg1, plus );
		do_mset( ch, outbuf );
		return;
	}
	/*
	 * save some more finger-leather
	 */
	if( !str_cmp( arg2, "damdie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;

		sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
		sprintf( outbuf, "%s damnumdie %d", arg1, num );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s damsizedie %d", arg1, size );
		do_mset( ch, outbuf );
		sprintf( outbuf, "%s damplus %d", arg1, plus );
		do_mset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "hitnumdie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 32767 )
		{
			send_to_char( "Number of hitpoint dice range is 0 to 30000.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->hitnodice = value;
			victim->max_hit =
				generate_hp( victim->top_level, victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice,
					victim->pIndexData->hitplus );
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "hitsizedie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 32767 )
		{
			send_to_char( "Hitpoint dice size range is 0 to 30000.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->hitsizedice = value;
			victim->max_hit =
				generate_hp( victim->top_level, victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice,
					victim->pIndexData->hitplus );
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "hitplus" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 3000000 )
		{
			send_to_char( "Hitpoint bonus range is 0 to 3,000,000.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->hitplus = value;
			victim->max_hit =
				generate_hp( victim->top_level, victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice,
					victim->pIndexData->hitplus );
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "damnumdie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 100 )
		{
			send_to_char( "Number of damage dice range is 0 to 100.\r\n", ch );
			return;
		}
		victim->barenumdie = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damnodice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "damsizedie" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 100 )
		{
			send_to_char( "Damage dice size range is 0 to 100.\r\n", ch );
			return;
		}
		victim->baresizedie = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damsizedice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "damplus" ) )
	{
		if( !IS_NPC( victim ) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if( !can_mmodify( ch, victim ) )
			return;
		if( value < 0 || value > 1000 )
		{
			send_to_char( "Damage bonus range is 0 to 1000.\r\n", ch );
			return;
		}
		victim->damplus = value;
		if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damplus = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}


	if( !str_cmp( arg2, "aloaded" ) )
	{
		if( IS_NPC( victim ) )
		{
			send_to_char( "Player Characters only.\r\n", ch );
			return;
		}


		if( !can_mmodify( ch, victim ) )
			return;

		if( !IS_SET( victim->pcdata->area->status, AREA_LOADED ) )
		{
			SET_BIT( victim->pcdata->area->status, AREA_LOADED );
			send_to_char( "Your area set to LOADED!\r\n", victim );
			if( ch != victim )
				send_to_char( "Area set to LOADED!\r\n", ch );
			return;
		}
		else
		{
			REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
			send_to_char( "Your area set to NOT-LOADED!\r\n", victim );
			if( ch != victim )
				send_to_char( "Area set to NON-LOADED!\r\n", ch );
			return;
		}
	}

	/*
	 * Generate usage message.
	 */
	if( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_mset;
	}
	else
		do_mset( ch, "" );
	return;
}


CMDF( do_oset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *tmpobj;
	EXTRA_DESCR_DATA *ed;
	bool lockobj;
	const char *origarg = argument;

	int value, tmp;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't oset\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;

	case SUB_OBJ_EXTRA:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		/*
		 * hopefully the object didn't get extracted...
		 * if you're REALLY paranoid, you could always go through
		 * the object and index-object lists, searching through the
		 * extra_descr lists for a matching pointer...
		 */
		ed = ( EXTRA_DESCR_DATA * ) ch->dest_buf;
		STRFREE( ed->description );
		ed->description = copy_buffer( ch );
		tmpobj = ( OBJ_DATA * ) ch->spare_ptr;
		stop_editing( ch );
		ch->dest_buf = tmpobj;
		ch->substate = ch->tempnum;
		return;

	case SUB_OBJ_LONG:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		obj = ( OBJ_DATA * ) ch->dest_buf;
		if( obj && obj_extracted( obj ) )
		{
			send_to_char( "Your object was extracted!\r\n", ch );
			stop_editing( ch );
			return;
		}
		STRFREE( obj->description );
		obj->description = copy_buffer( ch );
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			STRFREE( obj->pIndexData->description );
			obj->pIndexData->description = QUICKLINK( obj->description );
		}
		tmpobj = ( OBJ_DATA * ) ch->spare_ptr;
		stop_editing( ch );
		ch->substate = ch->tempnum;
		ch->dest_buf = tmpobj;
		return;
	}

	obj = NULL;
	argument = smash_tilde_static( argument );

	if( ch->substate == SUB_REPEATCMD )
	{
		obj = ( OBJ_DATA * ) ch->dest_buf;
		if( obj && obj_extracted( obj ) )
		{
			send_to_char( "Your object was extracted!\r\n", ch );
			obj = NULL;
			argument = "done";
		}
		if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
		{
			if( obj )
				do_ostat( ch, obj->name );
			else
				send_to_char( "No object selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
		{
			send_to_char( "Oset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			return;
		}
	}
	if( obj )
	{
		lockobj = true;
		mudstrlcpy( arg1, obj->name, MAX_INPUT_LENGTH );
		argument = one_argument( argument, arg2 );
		mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
	}
	else
	{
		lockobj = false;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		mudstrlcpy( arg3, argument, MAX_INPUT_LENGTH );
	}

	/*
		if ( !str_cmp( arg1, "on" ) )
		{
		send_to_char( "Syntax: oset <object|vnum> on.\r\n", ch );
		return;
		}
	*/

	if( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
	{
		if( ch->substate == SUB_REPEATCMD )
		{
			if( obj )
				send_to_char( "Syntax: <field>  <value>\r\n", ch );
			else
				send_to_char( "Syntax: <object> <field>  <value>\r\n", ch );
		}
		else
			send_to_char( "Syntax: oset <object> <field>  <value>\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Field being one of:\r\n", ch );
		send_to_char( "  flags wear level weight cost rent timer\r\n", ch );
		send_to_char( "  name short long desc ed rmed actiondesc\r\n", ch );
		send_to_char( "  type value0 value1 value2 value3 value4 value5\r\n", ch );
		send_to_char( "  affect rmaffect layers\r\n", ch );
		send_to_char( "For weapons:             For armor:\r\n", ch );
		send_to_char( "  weapontype condition     ac condition\r\n", ch );
		send_to_char( "  numdamdie sizedamdie                  \r\n", ch );
		send_to_char( "  charges   maxcharges                  \r\n", ch );
		send_to_char( "For potions, pills:\r\n", ch );
		send_to_char( "  slevel spell1 spell2 spell3\r\n", ch );
		send_to_char( "For devices:\r\n", ch );
		send_to_char( "  slevel spell maxcharges charges\r\n", ch );
		send_to_char( "For salves:\r\n", ch );
		send_to_char( "  slevel spell1 spell2 maxdoses delay (keep low - delay is anoying)\r\n", ch );
		send_to_char( "For containers:          For levers and switches:\r\n", ch );
		send_to_char( "  cflags key capacity      tflags\r\n", ch );
		send_to_char( "For rawspice:            For ammo and batteries:\r\n", ch );
		send_to_char( "  spicetype  grade         charges (at least 1000 for ammo)\r\n", ch );
		send_to_char( "For crystals:\r\n", ch );
		send_to_char( "  gemtype\r\n", ch );
		return;
	}

	if( !obj && get_trust( ch ) <= LEVEL_STAFF )
	{
		if( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You can't find that here.\r\n", ch );
			return;
		}
	}
	else if( !obj )
	{
		if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "There is nothing like that in all the realms.\r\n", ch );
			return;
		}
	}
	if( lockobj )
		ch->dest_buf = obj;
	else
		ch->dest_buf = NULL;

	separate_obj( obj );
	value = atoi( arg3 );

	if( !str_cmp( arg2, "on" ) )
	{
		CHECK_SUBRESTRICTED( ch );
		ch_printf( ch, "Oset mode on. (Editing '%s' vnum %d).\r\n", obj->name, obj->pIndexData->vnum );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = obj;
		if( ch->pcdata )
		{
			if( ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			sprintf( buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
			ch->pcdata->subprompt = STRALLOC( buf );
		}
		RelCreate( relOSET_ON, ch, obj );
		return;
	}

	if( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[0] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[0] = value;
		return;
	}

	if( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[1] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[1] = value;
		return;
	}

	if( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[2] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->value[2] = value;
			if( obj->item_type == ITEM_WEAPON && value != 0 )
				obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
		}
		return;
	}

	if( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[3] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[3] = value;
		return;
	}

	if( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[4] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[4] = value;
		return;
	}

	if( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[5] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[5] = value;
		return;
	}

	if( !str_cmp( arg2, "type" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> type <type>\r\n", ch );
			send_to_char( "Possible Types:\r\n", ch );
			send_to_char( "None        Light\r\n", ch );
			send_to_char( "Treasure    Armor      Comlink    Fabric      Grenade\r\n", ch );
			send_to_char( "Furniture   Trash      Container  Drink_con   Landmine\r\n", ch );
			send_to_char( "Key         Food       Money      Pen         Fuel\r\n", ch );
			send_to_char( "Fountain    Pill       Weapon     Medpac      Missile\r\n", ch );
			send_to_char( "Fire        Book       Superconductor         Rare_metal\r\n", ch );
			send_to_char( "Switch      Lever      Button     Dial        Government\r\n", ch );
			send_to_char( "Trap        Map        Portal     Paper       Magnet\r\n", ch );
			send_to_char( "Lockpick    Shovel     Thread     Smut        Ammo\r\n", ch );
			send_to_char( "Rawspice    Lens       Crystal    Duraplast   Battery\r\n", ch );
			send_to_char( "Toolkit     Durasteel  Oven       Mirror      Circuit\r\n", ch );
			send_to_char( "Potion      Salve      Pill       Device      Spacecraft\r\n", ch );
			send_to_char( "Bolt        Chemical   Debitcard  Restore\r\n", ch );
			return;
		}
		value = get_otype( argument );
		if( value < 1 )
		{
			ch_printf( ch, "Unknown type: %s\r\n", arg3 );
			return;
		}
		obj->item_type = ( short ) value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->item_type = obj->item_type;
		return;
	}

	if( !str_cmp( arg2, "flags" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> flags <flag> [flag]...\r\n", ch );
			send_to_char( "glow, dark, magic, bless, antievil, noremove, antisith, antisoldier,\r\n", ch );
			send_to_char( "donation, covering, hum, invis, nodrop, antigood, antipilot, anticitizen\r\n", ch );
			send_to_char( "antineutral, inventory, antithief, antijedi, clanobject, antihunter\r\n", ch );
			send_to_char( "small_size, human_size, large_size, hutt_size, contraband\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_oflag( arg3 );
			if( value < 0 || value >= MAX_ITEM_FLAG )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				xTOGGLE_BIT( obj->extra_flags, value );
				if( value == ITEM_PROTOTYPE )
					obj->pIndexData->extra_flags = obj->extra_flags;
			}
		}
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->extra_flags = obj->extra_flags;
		return;
	}

	if( !str_cmp( arg2, "wear" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> wear <flag> [flag]...\r\n", ch );
			send_to_char( "Possible locations:\r\n", ch );
			send_to_char( "take   finger   neck    body    head   legs\r\n", ch );
			send_to_char( "feet   hands    arms    shield  about  waist\r\n", ch );
			send_to_char( "wrist  wield    hold    ears    eyes   floating\r\n", ch );
			send_to_char( "ankle\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_wflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT( obj->wear_flags, 1 << value );
		}

		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->wear_flags = obj->wear_flags;
		return;
	}

	if( !str_cmp( arg2, "level" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->level = value;
		return;
	}

	if( !str_cmp( arg2, "weight" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->weight = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->weight = value;
		return;
	}

	if( !str_cmp( arg2, "cost" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->cost = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->cost = value;
		return;
	}

	if( !str_cmp( arg2, "rent" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->rent = value;
		else
			send_to_char( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "layers" ) )
	{
		if( !IS_SUPREME( ch ) )
		{
			send_to_char( "Sorry. Layers are always 0.\r\n", ch );
			return;
		}

		if( !can_omodify( ch, obj ) )
			return;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->layers = value;
		else
			send_to_char( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "timer" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->timer = value;
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		if( !can_omodify( ch, obj ) )
			return;

		if( arg3[0] == '\0' )
		{
			send_to_char( "&WYou must choose a new name!\r\n", ch );
			return;
		}
		STRFREE( obj->name );
		obj->name = STRALLOC( arg3 );
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			STRFREE( obj->pIndexData->name );
			obj->pIndexData->name = QUICKLINK( obj->name );
		}
		return;
	}

	if( !str_cmp( arg2, "short" ) )
	{
		STRFREE( obj->short_descr );
		obj->short_descr = STRALLOC( arg3 );
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			STRFREE( obj->pIndexData->short_descr );
			obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
		}
		else
			/*
			 * Feature added by Narn, Apr/96
			 * * If the item is not proto, add the word 'rename' to the keywords
			 * * if it is not already there.
			 */
		{
			if( str_infix( "rename", obj->name ) )
			{
				sprintf( buf, "%s %s", obj->name, "rename" );
				STRFREE( obj->name );
				obj->name = STRALLOC( buf );
			}
		}
		return;
	}

	if( !str_cmp( arg2, "actiondesc" ) )
	{
		if( strstr( arg3, "%n" ) || strstr( arg3, "%d" ) || strstr( arg3, "%l" ) )
		{
			send_to_char( "Illegal characters!\r\n", ch );
			return;
		}
		STRFREE( obj->action_desc );
		obj->action_desc = STRALLOC( arg3 );
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			STRFREE( obj->pIndexData->action_desc );
			obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
		}
		return;
	}

	if( !str_cmp( arg2, "long" ) )
	{
		if( arg3[0] )
		{
			STRFREE( obj->description );
			obj->description = STRALLOC( arg3 );
			if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			{
				STRFREE( obj->pIndexData->description );
				obj->pIndexData->description = QUICKLINK( obj->description );
			}
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_LONG;
		ch->dest_buf = obj;
		start_editing( ch, obj->description );
		return;
	}

	if( !str_cmp( arg2, "affect" ) )
	{
		AFFECT_DATA *paf;
		short loc;
		int bitv;

		argument = one_argument( argument, arg2 );
		if( arg2[0] == '\0' || !argument || argument[0] == 0 )
		{
			send_to_char( "Usage: oset <object> affect <field> <value>\r\n", ch );
			send_to_char( "Affect Fields:\r\n", ch );
			send_to_char( "none        strength    dexterity   intelligence  wisdom       constitution\r\n", ch );
			send_to_char( "sex         level       age         height        weight       force\r\n", ch );
			send_to_char( "hit         move        dollars     experience    armor        hitroll\r\n", ch );
			send_to_char( "damroll     save_para   save_rod    save_poison   save_breath  save_power\r\n", ch );
			send_to_char( "charisma    resistant   immune      susceptible   affected     luck\r\n", ch );
			send_to_char( "backstab    pick        track       steal         sneak        hide\r\n", ch );
			send_to_char( "detrap      dodge       peek        scan          gouge        search\r\n", ch );
			send_to_char( "mount       disarm      kick        parry         bash         stun\r\n", ch );
			send_to_char( "punch       climb       grip        scribe        brew\r\n", ch );
			return;
		}
		loc = get_atype( arg2 );
		if( loc < 1 )
		{
			ch_printf( ch, "Unknown field: %s\r\n", arg2 );
			return;
		}
		if( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
		{
			bitv = 0;
			while( argument[0] != '\0' )
			{
				argument = one_argument( argument, arg3 );
				if( loc == APPLY_AFFECT )
					value = get_aflag( arg3 );
				else
					value = get_risflag( arg3 );
				if( value < 0 || value > 31 )
					ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
				else
					SET_BIT( bitv, 1 << value );
			}
			if( !bitv )
				return;
			value = bitv;
		}
		else
		{
			argument = one_argument( argument, arg3 );
			value = atoi( arg3 );
		}
		CREATE( paf, AFFECT_DATA, 1 );
		paf->type = -1;
		paf->duration = -1;
		paf->location = loc;
		paf->modifier = value;
		paf->bitvector = 0;

		paf->next = NULL;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			if( loc != APPLY_WEARSPELL && loc != APPLY_REMOVESPELL && loc != APPLY_STRIPSN && loc != APPLY_WEAPONSPELL )
			{
				CHAR_DATA *vch;
				OBJ_DATA *eq;

				for( vch = first_char; vch; vch = vch->next )
				{
					for( eq = vch->first_carrying; eq; eq = eq->next_content )
					{
						if( eq->pIndexData == obj->pIndexData && eq->wear_loc != WEAR_NONE )
							affect_modify( vch, paf, true );
					}
				}
			}
			LINK( paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
		}
		else
			LINK( paf, obj->first_affect, obj->last_affect, next, prev );
		++top_affect;

		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "rmaffect" ) )
	{
		AFFECT_DATA *paf;
		short loc, count;

		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> rmaffect <affect#>\r\n", ch );
			return;
		}
		loc = atoi( argument );
		if( loc < 1 )
		{
			send_to_char( "Invalid number.\r\n", ch );
			return;
		}

		count = 0;

		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			OBJ_INDEX_DATA *pObjIndex;

			pObjIndex = obj->pIndexData;
			for( paf = pObjIndex->first_affect; paf; paf = paf->next )
			{
				if( ++count == loc )
				{
					UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
					if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL && paf->location != APPLY_STRIPSN
						&& paf->location != APPLY_WEAPONSPELL )
					{
						CHAR_DATA *vch;
						OBJ_DATA *eq;

						for( vch = first_char; vch; vch = vch->next )
						{
							for( eq = vch->first_carrying; eq; eq = eq->next_content )
							{
								if( eq->pIndexData == pObjIndex && eq->wear_loc != WEAR_NONE )
									affect_modify( vch, paf, false );
							}
						}
					}
					DISPOSE( paf );
					send_to_char( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
		}
		else
		{
			for( paf = obj->first_affect; paf; paf = paf->next )
			{
				if( ++count == loc )
				{
					UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
					DISPOSE( paf );
					send_to_char( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
			send_to_char( "Not found.\r\n", ch );
			return;
		}
	}

	if( !str_cmp( arg2, "ed" ) )
	{
		if( arg3[0] == '\0' )
		{
			send_to_char( "Syntax: oset <object> ed <keywords>\r\n", ch );
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if( obj->timer )
		{
			send_to_char( "It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\r\n",
				ch );
			return;
		}
		if( obj->item_type == ITEM_PAPER )
		{
			send_to_char( "You can not add an extra description to a note paper at the moment.\r\n", ch );
			return;
		}
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			ed = SetOExtraProto( obj->pIndexData, arg3 );
		else
			ed = SetOExtra( obj, arg3 );
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description );
		return;
	}

	if( !str_cmp( arg2, "desc" ) )
	{
		CHECK_SUBRESTRICTED( ch );
		if( obj->timer )
		{
			send_to_char( "It's not safe to edit a description on an object with a timer.\n\rTurn it off first.\r\n", ch );
			return;
		}
		if( obj->item_type == ITEM_PAPER )
		{
			send_to_char( "You can not add a description to a note paper at the moment.\r\n", ch );
			return;
		}
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			ed = SetOExtraProto( obj->pIndexData, obj->name );
		else
			ed = SetOExtra( obj, obj->name );
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description );
		return;
	}



	if( !str_cmp( arg2, "rmed" ) )
	{
		if( arg3[0] == '\0' )
		{
			send_to_char( "Syntax: oset <object> rmed <keywords>\r\n", ch );
			return;
		}
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			if( DelOExtraProto( obj->pIndexData, arg3 ) )
				send_to_char( "Deleted.\r\n", ch );
			else
				send_to_char( "Not found.\r\n", ch );
			return;
		}
		if( DelOExtra( obj, arg3 ) )
			send_to_char( "Deleted.\r\n", ch );
		else
			send_to_char( "Not found.\r\n", ch );
		return;
	}
	/*
	* save some finger-leather
	*/
	if( !str_cmp( arg2, "ris" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
		do_oset( ch, outbuf );
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
		do_oset( ch, outbuf );
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "r" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "i" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}
	if( !str_cmp( arg2, "s" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "ri" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
		do_oset( ch, outbuf );
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "rs" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect resistant %s", arg1, arg3 );
		do_oset( ch, outbuf );
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	if( !str_cmp( arg2, "is" ) )
	{
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect immune %s", arg1, arg3 );
		do_oset( ch, outbuf );
		snprintf( outbuf, MAX_STRING_LENGTH, "%s affect susceptible %s", arg1, arg3 );
		do_oset( ch, outbuf );
		return;
	}

	/*
	 * Make it easier to set special object values by name than number
	 *                      -Thoric
	 */
	tmp = -1;
	switch( obj->item_type )
	{
	case ITEM_WEAPON:
		if( !str_cmp( arg2, "weapontype" ) )
		{
			size_t x;

			value = -1;
			for( x = 0; x < sizeof( weapon_table ) / sizeof( weapon_table[0] ); x++ )
				if( !str_cmp( arg3, weapon_table[x] ) )
					value = x;
			if( value < 0 )
			{
				send_to_char( "Unknown weapon type.\r\n", ch );
				send_to_char( "\n\rChoices:\r\n", ch );
				send_to_char( "   none, lightsaber, vibro-blade, blaster, force pike, bowcaster, bludgeon\r\n", ch );
				return;
			}
			tmp = 3;
			break;
		}
		if( !str_cmp( arg2, "condition" ) )
			tmp = 0;
		if( !str_cmp( arg2, "numdamdie" ) )
			tmp = 1;
		if( !str_cmp( arg2, "sizedamdie" ) )
			tmp = 2;
		if( !str_cmp( arg2, "charges" ) )
			tmp = 4;
		if( !str_cmp( arg2, "maxcharges" ) )
			tmp = 5;
		if( !str_cmp( arg2, "charge" ) )
			tmp = 4;
		if( !str_cmp( arg2, "maxcharge" ) )
			tmp = 5;
		break;
	case ITEM_BOLT:
	case ITEM_AMMO:
		if( !str_cmp( arg2, "charges" ) )
			tmp = 0;
		if( !str_cmp( arg2, "charge" ) )
			tmp = 0;
		break;
	case ITEM_BATTERY:
		if( !str_cmp( arg2, "charges" ) )
			tmp = 0;
		if( !str_cmp( arg2, "charge" ) )
			tmp = 0;
		break;
	case ITEM_RAWSPICE:
	case ITEM_SPICE:
		if( !str_cmp( arg2, "grade" ) )
			tmp = 1;
		if( !str_cmp( arg2, "spicetype" ) )
		{
			size_t x;

			value = -1;
			for( x = 0; x < sizeof( spice_table ) / sizeof( spice_table[0] ); x++ )
				if( !str_cmp( arg3, spice_table[x] ) )
					value = x;
			if( value < 0 )
			{
				send_to_char( "Unknown spice type.\r\n", ch );
				send_to_char( "\n\rChoices:\r\n", ch );
				send_to_char( "   glitterstim, carsanum, ryll, andris\r\n", ch );
				return;
			}
			tmp = 0;
			break;
		}
		break;
	case ITEM_CRYSTAL:
		if( !str_cmp( arg2, "gemtype" ) )
		{
			size_t x;

			value = -1;
			for( x = 0; x < sizeof( crystal_table ) / sizeof( crystal_table[0] ); x++ )
				if( !str_cmp( arg3, crystal_table[x] ) )
					value = x;
			if( value < 0 )
			{
				send_to_char( "Unknown gem type.\r\n", ch );
				send_to_char( "\n\rChoices:\r\n", ch );
				send_to_char( "   non-adegan, kathracite, relacite, danite, mephite, ponite, illum, corusca\r\n", ch );
				return;
			}
			tmp = 0;
			break;
		}
		break;
	case ITEM_ARMOR:
		if( !str_cmp( arg2, "condition" ) )
			tmp = 0;
		if( !str_cmp( arg2, "ac" ) )
			tmp = 1;
		break;
	case ITEM_SALVE:
		if( !str_cmp( arg2, "slevel" ) )
			tmp = 0;
		if( !str_cmp( arg2, "maxdoses" ) )
			tmp = 1;
		if( !str_cmp( arg2, "doses" ) )
			tmp = 2;
		if( !str_cmp( arg2, "delay" ) )
			tmp = 3;
		if( !str_cmp( arg2, "spell1" ) )
			tmp = 4;
		if( !str_cmp( arg2, "spell2" ) )
			tmp = 5;
		if( tmp >= 4 && tmp <= 5 )
			value = skill_lookup( arg3 );
		break;
	case ITEM_POTION:
	case ITEM_PILL:
		if( !str_cmp( arg2, "slevel" ) )
			tmp = 0;
		if( !str_cmp( arg2, "spell1" ) )
			tmp = 1;
		if( !str_cmp( arg2, "spell2" ) )
			tmp = 2;
		if( !str_cmp( arg2, "spell3" ) )
			tmp = 3;
		if( tmp >= 1 && tmp <= 3 )
			value = skill_lookup( arg3 );
		break;
	case ITEM_DEVICE:
		if( !str_cmp( arg2, "slevel" ) )
			tmp = 0;
		if( !str_cmp( arg2, "spell" ) )
		{
			tmp = 3;
			value = skill_lookup( arg3 );
		}
		if( !str_cmp( arg2, "maxcharges" ) )
			tmp = 1;
		if( !str_cmp( arg2, "charges" ) )
			tmp = 2;
		break;
	case ITEM_CONTAINER:
		if( !str_cmp( arg2, "capacity" ) )
			tmp = 0;
		if( !str_cmp( arg2, "cflags" ) )
			tmp = 1;
		if( !str_cmp( arg2, "key" ) )
			tmp = 2;
		break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_BUTTON:
		if( !str_cmp( arg2, "tflags" ) )
		{
			tmp = 0;
			value = get_trigflag( arg3 );
		}
		break;
	}
	if( tmp >= 0 && tmp <= 5 )
	{
		if( !can_omodify( ch, obj ) )
			return;
		obj->value[tmp] = value;
		if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[tmp] = value;
		return;
	}

	/*
	 * Generate usage message.
	 */
	if( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_oset;
	}
	else
		do_oset( ch, "" );
	return;
}


/*
 * Obsolete Merc room editing routine
 */
CMDF( do_rset )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	mudstrlcpy( arg3, argument, MSL );

	if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		send_to_char( "Syntax: rset <location> <field> value\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Field being one of:\r\n", ch );
		send_to_char( "  flags sector\r\n", ch );
		return;
	}

	if( ( location = find_location( ch, arg1 ) ) == NULL )
	{
		send_to_char( "No such location.\r\n", ch );
		return;
	}

	if( !can_rmodify( ch, location ) )
		return;

	if( !is_number( arg3 ) )
	{
		send_to_char( "Value must be numeric.\r\n", ch );
		return;
	}
	value = atoi( arg3 );

	/*
	 * Set something.
	 */
	if( !str_cmp( arg2, "flags" ) )
	{
		/*
		 * Protect from messing up prototype flag
		 */
		 /*      if( xIS_SET( location->room_flags, ROOM_PROTOTYPE ) )
				  proto = true;
			   else
				  proto = false;
			   location->room_flags = value;
			   if( proto )
				  xSET_BIT( location->room_flags, ROOM_PROTOTYPE );*/
		send_to_char( "Use 'redit' for flags.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "sector" ) )
	{
		location->sector_type = value;
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset( ch, "" );
	return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir( const char *txt )
{
	int edir;
	char c1, c2;

	if( !str_cmp( txt, "northeast" ) )
		return DIR_NORTHEAST;
	if( !str_cmp( txt, "northwest" ) )
		return DIR_NORTHWEST;
	if( !str_cmp( txt, "southeast" ) )
		return DIR_SOUTHEAST;
	if( !str_cmp( txt, "southwest" ) )
		return DIR_SOUTHWEST;
	if( !str_cmp( txt, "somewhere" ) )
		return 10;

	c1 = txt[0];
	if( c1 == '\0' )
		return 0;
	c2 = txt[1];
	edir = 0;
	switch( c1 )
	{
	case 'n':
		switch( c2 )
		{
		default:
			edir = 0;
			break;   /* north */
		case 'e':
			edir = 6;
			break;   /* ne    */
		case 'w':
			edir = 7;
			break;   /* nw    */
		}
		break;
	case '0':
		edir = 0;
		break; /* north */
	case 'e':
	case '1':
		edir = 1;
		break; /* east  */
	case 's':
		switch( c2 )
		{
		default:
			edir = 2;
			break;   /* south */
		case 'e':
			edir = 8;
			break;   /* se    */
		case 'w':
			edir = 9;
			break;   /* sw    */
		}
		break;
	case '2':
		edir = 2;
		break; /* south */
	case 'w':
	case '3':
		edir = 3;
		break; /* west  */
	case 'u':
	case '4':
		edir = 4;
		break; /* up    */
	case 'd':
	case '5':
		edir = 5;
		break; /* down  */
	case '6':
		edir = 6;
		break; /* ne    */
	case '7':
		edir = 7;
		break; /* nw    */
	case '8':
		edir = 8;
		break; /* se    */
	case '9':
		edir = 9;
		break; /* sw    */
	case '?':
		edir = 10;
		break; /* somewhere */
	}
	return edir;
}

CMDF( do_redit )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location, *tmp;
	EXTRA_DESCR_DATA *ed;
	char dir;
	EXIT_DATA *xit, *texit;
	int value;
	int edir, ekey, evnum;
	const char *origarg = argument;

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor.\r\n", ch );
		return;
	}

	if( !can_rmodify( ch, ch->in_room ) )
		return;

	switch( ch->substate )
	{
	default:
		break;
	case SUB_ROOM_DESC:
		location = ( ROOM_INDEX_DATA * ) ch->dest_buf;
		if( !location )
		{
			bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
			location = ch->in_room;
		}
		STRFREE( location->description );
		location->description = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	case SUB_ROOM_EXTRA:
		ed = ( EXTRA_DESCR_DATA * ) ch->dest_buf;
		if( !ed )
		{
			bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
			stop_editing( ch );
			return;
		}
		STRFREE( ed->description );
		ed->description = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	}

	location = ch->in_room;

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg );
	if( ch->substate == SUB_REPEATCMD )
	{
		if( arg[0] == '\0' )
		{
			do_rstat( ch, "" );
			return;
		}
		if( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
		{
			send_to_char( "Redit mode off.\r\n", ch );
			if( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			ch->substate = SUB_NONE;
			return;
		}
	}
	if( arg[0] == '\0' || !str_cmp( arg, "?" ) )
	{
		if( ch->substate == SUB_REPEATCMD )
			send_to_char( "Syntax: <field> value\r\n", ch );
		else
			send_to_char( "Syntax: redit <field> value\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Field being one of:\r\n", ch );
		send_to_char( "  name desc ed rmed\r\n", ch );
		send_to_char( "  exit bexit exdesc exflags exname exkey\r\n", ch );
		send_to_char( "  flags flags2 sector teledelay televnum\r\n", ch );
		send_to_char( "  tunnel rlist exdistance\r\n", ch );
		return;
	}

	if( !can_rmodify( ch, location ) )
		return;

	/*
		if ( !str_cmp( arg, "on" ) )
		{
		send_to_char( "Redit mode on.\r\n", ch );
		ch->substate = SUB_REPEATCMD;
		if ( ch->pcdata )
		{
		   if ( ch->pcdata->subprompt )
			STRFREE( ch->pcdata->subprompt );
		   ch->pcdata->subprompt = STRALLOC( "<&CRedit &W#%r&w> %i" );
		}
		return;
		}
	*/

	if( !str_cmp( arg, "substate" ) )
	{
		argument = one_argument( argument, arg2 );
		if( !str_cmp( arg2, "north" ) )
		{
			ch->inter_substate = SUB_NORTH;
			return;
		}
		if( !str_cmp( arg2, "east" ) )
		{
			ch->inter_substate = SUB_EAST;
			return;
		}
		if( !str_cmp( arg2, "south" ) )
		{
			ch->inter_substate = SUB_SOUTH;
			return;
		}
		if( !str_cmp( arg2, "west" ) )
		{
			ch->inter_substate = SUB_WEST;
			return;
		}
		if( !str_cmp( arg2, "up" ) )
		{
			ch->inter_substate = SUB_UP;
			return;
		}
		if( !str_cmp( arg2, "down" ) )
		{
			ch->inter_substate = SUB_DOWN;
			return;
		}
		send_to_char( " unrecognized substate in redit\r\n", ch );
		return;
	}


	if( !str_cmp( arg, "name" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Set the room name.  A very brief single line room description.\r\n", ch );
			send_to_char( "Usage: redit name <Room summary>\r\n", ch );
			return;
		}
		STRFREE( location->name );
		location->name = STRALLOC( argument );
		return;
	}

	if( !str_cmp( arg, "desc" ) )
	{
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_DESC;
		ch->dest_buf = location;
		start_editing( ch, location->description );
		return;
	}

	if( !str_cmp( arg, "tunnel" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\r\n", ch );
			send_to_char( "Usage: redit tunnel <value>\r\n", ch );
			return;
		}
		location->tunnel = URANGE( 0, atoi( argument ), 1000 );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "ed" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Create an extra description.\r\n", ch );
			send_to_char( "You must supply keyword(s).\r\n", ch );
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		ed = SetRExtra( location, argument );
		if( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description );
		return;
	}

	if( !str_cmp( arg, "rmed" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Remove an extra description.\r\n", ch );
			send_to_char( "You must supply keyword(s).\r\n", ch );
			return;
		}
		if( DelRExtra( location, argument ) )
			send_to_char( "Deleted.\r\n", ch );
		else
			send_to_char( "Not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "rlist" ) )
	{
		RESET_DATA *pReset;
		char *rbuf;
		short num;

		if( !location->first_reset )
		{
			send_to_char( "This room has no resets to list.\r\n", ch );
			return;
		}
		num = 0;
		for( pReset = location->first_reset; pReset; pReset = pReset->next )
		{
			++num;
			if( !( rbuf = sprint_reset( pReset, &num ) ) )
				continue;
			send_to_char( rbuf, ch );
		}
		return;
	}

	if( !str_cmp( arg, "flags" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Toggle the room flags.\r\n", ch );
			send_to_char( "Usage: redit flags <flag> [flag]...\r\n", ch );
			send_to_char( "\n\rPossible Flags: \r\n", ch );
			send_to_char( "dark, nomob, indoors, nomagic, bank,\r\n", ch );
			send_to_char( "private, safe, petshop, norecall, donation, nodropall, silence,\r\n", ch );
			send_to_char( "logspeach, nodrop, clanstoreroom, plr_home, empty_home, teleport\r\n", ch );
			send_to_char( "nofloor, prototype, refinery, factory, republic_recruit\r\n", ch );
			send_to_char( "spacecraft, auction, no_drive, can_land, can_fly, hotel\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			value = get_rflag( arg2 );
			if( value < 0 || value >= ROOM_MAX )
				ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
			else
				xTOGGLE_BIT( location->room_flags, value );
		}
		return;
	}

	if( !str_cmp( arg, "flags2" ) )
	{
		send_to_char( "Flags2 is no longer used. All room flags are toggled using 'flags'.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "teledelay" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the delay of the teleport. (0 = off).\r\n", ch );
			send_to_char( "Usage: redit teledelay <value>\r\n", ch );
			return;
		}
		location->tele_delay = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "televnum" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the vnum of the room to teleport to.\r\n", ch );
			send_to_char( "Usage: redit televnum <vnum>\r\n", ch );
			return;
		}
		location->tele_vnum = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "sector" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the sector type.\r\n", ch );
			send_to_char( "Usage: redit sector <value>\r\n", ch );
			send_to_char( "\n\rSector Values:\r\n", ch );
			send_to_char( "0:dark, 1:city, 2:field, 3:forest, 4:hills, 5:mountain, 6:water_swim\r\n", ch );
			send_to_char( "7:water_noswim, 8:underwater, 9:air, 10:desert, 11:unkown, 12:oceanfloor, 13:underground\r\n", ch );

			return;
		}
		location->sector_type = atoi( argument );
		if( location->sector_type < 0 || location->sector_type >= SECT_MAX )
		{
			location->sector_type = 1;
			send_to_char( "Out of range\r\n.", ch );
		}
		else
			send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "exkey" ) )
	{
		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if( arg2[0] == '\0' || arg3[0] == '\0' )
		{
			send_to_char( "Usage: redit exkey <dir> <key vnum>\r\n", ch );
			return;
		}
		if( arg2[0] == '#' )
		{
			edir = atoi( arg2 + 1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		value = atoi( arg3 );
		if( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		xit->key = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "exname" ) )
	{
		argument = one_argument( argument, arg2 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Change or clear exit keywords.\r\n", ch );
			send_to_char( "Usage: redit exname <dir> [keywords]\r\n", ch );
			return;
		}
		if( arg2[0] == '#' )
		{
			edir = atoi( arg2 + 1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		STRFREE( xit->keyword );
		xit->keyword = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "exflags" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Toggle or display exit flags.\r\n", ch );
			send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\r\n", ch );
			send_to_char( "\n\rExit flags:\r\n", ch );
			send_to_char( "isdoor, closed, locked, can_look, searchable, can_leave, can_climb,\r\n", ch );
			send_to_char( "nopassdoor, secret, pickproof, fly, climb, dig, window, auto, can_enter\r\n", ch );
			send_to_char( "hidden, no_mob, bashproof, bashed\r\n", ch );

			return;
		}
		argument = one_argument( argument, arg2 );
		if( arg2[0] == '#' )
		{
			edir = atoi( arg2 + 1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		if( argument[0] == '\0' )
		{
			sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\r\n[ ", xit->vdir, xit->keyword, xit->key );
			for( value = 0; value <= MAX_EXFLAG; value++ )
			{
				if( IS_SET( xit->exit_info, 1 << value ) )
				{
					strcat( buf, ex_flags[value] );
					strcat( buf, " " );
				}
			}
			strcat( buf, "]\r\n" );
			send_to_char( buf, ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg2 );
			value = get_exflag( arg2 );
			if( value < 0 || value > MAX_EXFLAG )
				ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
			else
				TOGGLE_BIT( xit->exit_info, 1 << value );
		}
		return;
	}



	if( !str_cmp( arg, "ex_flags" ) )
	{
		argument = one_argument( argument, arg2 );
		switch( ch->inter_substate )
		{
		case SUB_EAST:
			dir = 'e';
			edir = 1;
			break;
		case SUB_WEST:
			dir = 'w';
			edir = 3;
			break;
		case SUB_SOUTH:
			dir = 's';
			edir = 2;
			break;
		case SUB_UP:
			dir = 'u';
			edir = 4;
			break;
		case SUB_DOWN:
			dir = 'd';
			edir = 5;
			break;
		default:
		case SUB_NORTH:
			dir = 'n';
			edir = 0;
			break;
		}

		value = get_exflag( arg2 );
		if( value < 0 )
		{
			send_to_char( "Bad exit flag. \r\n", ch );
			return;
		}
		if( ( xit = get_exit( location, edir ) ) == NULL )
		{
			sprintf( buf, "exit %c 1", dir );
			do_redit( ch, buf );
			xit = get_exit( location, edir );
		}
		TOGGLE_BIT( xit->exit_info, value );
		return;
	}


	if( !str_cmp( arg, "ex_to_room" ) )
	{
		argument = one_argument( argument, arg2 );
		switch( ch->inter_substate )
		{
		case SUB_EAST:
			dir = 'e';
			edir = 1;
			break;
		case SUB_WEST:
			dir = 'w';
			edir = 3;
			break;
		case SUB_SOUTH:
			dir = 's';
			edir = 2;
			break;
		case SUB_UP:
			dir = 'u';
			edir = 4;
			break;
		case SUB_DOWN:
			dir = 'd';
			edir = 5;
			break;
		default:
		case SUB_NORTH:
			dir = 'n';
			edir = 0;
			break;
		}
		evnum = atoi( arg2 );
		if( evnum < 1 || evnum > MAX_VNUMS )
		{
			send_to_char( "Invalid room number.\r\n", ch );
			return;
		}
		if( ( tmp = get_room_index( evnum ) ) == NULL )
		{
			send_to_char( "Non-existant room.\r\n", ch );
			return;
		}
		if( ( xit = get_exit( location, edir ) ) == NULL )
		{
			sprintf( buf, "exit %c 1", dir );
			do_redit( ch, buf );
			xit = get_exit( location, edir );
		}
		xit->vnum = evnum;
		return;
	}

	if( !str_cmp( arg, "ex_key" ) )
	{
		argument = one_argument( argument, arg2 );
		switch( ch->inter_substate )
		{
		case SUB_EAST:
			dir = 'e';
			edir = 1;
			break;
		case SUB_WEST:
			dir = 'w';
			edir = 3;
			break;
		case SUB_SOUTH:
			dir = 's';
			edir = 2;
			break;
		case SUB_UP:
			dir = 'u';
			edir = 4;
			break;
		case SUB_DOWN:
			dir = 'd';
			edir = 5;
			break;
		default:
		case SUB_NORTH:
			dir = 'n';
			edir = 0;
			break;
		}
		if( ( xit = get_exit( location, edir ) ) == NULL )
		{
			sprintf( buf, "exit %c 1", dir );
			do_redit( ch, buf );
			xit = get_exit( location, edir );
		}
		xit->key = atoi( arg2 );
		return;
	}

	if( !str_cmp( arg, "ex_exdesc" ) )
	{
		switch( ch->inter_substate )
		{
		case SUB_EAST:
			dir = 'e';
			edir = 1;
			break;
		case SUB_WEST:
			dir = 'w';
			edir = 3;
			break;
		case SUB_SOUTH:
			dir = 's';
			edir = 2;
			break;
		case SUB_UP:
			dir = 'u';
			edir = 4;
			break;
		case SUB_DOWN:
			dir = 'd';
			edir = 5;
			break;
		default:
		case SUB_NORTH:
			dir = 'n';
			edir = 0;
			break;
		}
		if( ( xit = get_exit( location, edir ) ) == NULL )
		{
			sprintf( buf, "exit %c 1", dir );
			do_redit( ch, buf );
		}
		sprintf( buf, "exdesc %c %s", dir, argument );
		do_redit( ch, buf );
		return;
	}

	if( !str_cmp( arg, "ex_keywords" ) ) /* not called yet */
	{
		switch( ch->inter_substate )
		{
		case SUB_EAST:
			dir = 'e';
			edir = 1;
			break;
		case SUB_WEST:
			dir = 'w';
			edir = 3;
			break;
		case SUB_SOUTH:
			dir = 's';
			edir = 2;
			break;
		case SUB_UP:
			dir = 'u';
			edir = 4;
			break;
		case SUB_DOWN:
			dir = 'd';
			edir = 5;
			break;
		default:
		case SUB_NORTH:
			dir = 'n';
			edir = 0;
			break;
		}
		if( ( xit = get_exit( location, edir ) ) == NULL )
		{
			sprintf( buf, "exit %c 1", dir );
			do_redit( ch, buf );
			if( ( xit = get_exit( location, edir ) ) == NULL )
				return;
		}
		sprintf( buf, "%s %s", xit->keyword, argument );
		STRFREE( xit->keyword );
		xit->keyword = STRALLOC( buf );
		return;
	}

	if( !str_cmp( arg, "exit" ) )
	{
		bool addexit, numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Create, change or remove an exit.\r\n", ch );
			send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		addexit = numnotdir = false;
		switch( arg2[0] )
		{
		default:
			edir = get_dir( arg2 );
			break;
		case '+':
			edir = get_dir( arg2 + 1 );
			addexit = true;
			break;
		case '#':
			edir = atoi( arg2 + 1 );
			numnotdir = true;
			break;
		}
		if( arg3[0] == '\0' )
			evnum = 0;
		else
			evnum = atoi( arg3 );
		if( numnotdir )
		{
			if( ( xit = get_exit_num( location, edir ) ) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit( location, edir );
		if( !evnum )
		{
			if( xit )
			{
				extract_exit( location, xit );
				send_to_char( "Exit removed.\r\n", ch );
				return;
			}
			send_to_char( "No exit in that direction.\r\n", ch );
			return;
		}
		if( evnum < 1 || evnum > MAX_VNUMS - 1 )
		{
			send_to_char( "Invalid room number.\r\n", ch );
			return;
		}
		if( ( tmp = get_room_index( evnum ) ) == NULL )
		{
			send_to_char( "Non-existant room.\r\n", ch );
			return;
		}
		if( get_trust( ch ) <= LEVEL_STAFF && tmp->area != location->area )
		{
			send_to_char( "You can't make an exit to that room.\r\n", ch );
			return;
		}
		if( addexit || !xit )
		{
			if( numnotdir )
			{
				send_to_char( "Cannot add an exit by number, sorry.\r\n", ch );
				return;
			}
			if( addexit && xit && get_exit_to( location, edir, tmp->vnum ) )
			{
				send_to_char( "There is already an exit in that direction leading to that location.\r\n", ch );
				return;
			}
			xit = make_exit( location, tmp, edir );
			xit->keyword = STRALLOC( "" );
			xit->description = STRALLOC( "" );
			xit->key = -1;
			xit->exit_info = 0;
			act( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
		}
		else
			act( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );
		if( xit->to_room != tmp )
		{
			xit->to_room = tmp;
			xit->vnum = evnum;
			texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );
			if( texit )
			{
				texit->rexit = xit;
				xit->rexit = texit;
			}
		}
		argument = one_argument( argument, arg3 );
		if( arg3[0] != '\0' )
			xit->exit_info = atoi( arg3 );
		if( argument && argument[0] != '\0' )
		{
			one_argument( argument, arg3 );
			ekey = atoi( arg3 );
			if( ekey != 0 || arg3[0] == '0' )
			{
				argument = one_argument( argument, arg3 );
				xit->key = ekey;
			}
			if( argument && argument[0] != '\0' )
			{
				STRFREE( xit->keyword );
				xit->keyword = STRALLOC( argument );
			}
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	 * Twisted and evil, but works              -Thoric
	 * Makes an exit, and the reverse in one shot.
	 */
	if( !str_cmp( arg, "bexit" ) )
	{
		EXIT_DATA *rxit;
		char tmpcmd[MAX_INPUT_LENGTH];
		ROOM_INDEX_DATA *tmploc;
		int vnum, exnum;
		char rvnum[MAX_INPUT_LENGTH];
		bool numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Create, change or remove a two-way exit.\r\n", ch );
			send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		numnotdir = false;
		switch( arg2[0] )
		{
		default:
			edir = get_dir( arg2 );
			break;
		case '#':
			numnotdir = true;
			edir = atoi( arg2 + 1 );
			break;
		case '+':
			edir = get_dir( arg2 + 1 );
			break;
		}
		tmploc = location;
		exnum = edir;
		if( numnotdir )
		{
			if( ( xit = get_exit_num( tmploc, edir ) ) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit( tmploc, edir );
		rxit = NULL;
		vnum = 0;
		rvnum[0] = '\0';
		if( xit )
		{
			vnum = xit->vnum;
			if( arg3[0] != '\0' )
				sprintf( rvnum, "%d", tmploc->vnum );
			if( xit->to_room )
				rxit = get_exit( xit->to_room, rev_dir[edir] );
			else
				rxit = NULL;
		}
		sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
		do_redit( ch, tmpcmd );
		if( numnotdir )
			xit = get_exit_num( tmploc, exnum );
		else
			xit = get_exit( tmploc, edir );
		if( !rxit && xit )
		{
			vnum = xit->vnum;
			if( arg3[0] != '\0' )
				sprintf( rvnum, "%d", tmploc->vnum );
			if( xit->to_room )
				rxit = get_exit( xit->to_room, rev_dir[edir] );
			else
				rxit = NULL;
		}
		if( vnum )
		{
			sprintf( tmpcmd, "%d redit exit %d %s %s", vnum, rev_dir[edir], rvnum, argument );
			do_at( ch, tmpcmd );
		}
		return;
	}

	if( !str_cmp( arg, "exdistance" ) )
	{
		argument = one_argument( argument, arg2 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Set the distance (in rooms) between this room, and the destination room.\r\n", ch );
			send_to_char( "Usage: redit exdistance <dir> [distance]\r\n", ch );
			return;
		}
		if( arg2[0] == '#' )
		{
			edir = atoi( arg2 + 1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if( xit )
		{
			xit->distance = URANGE( 1, atoi( argument ), 50 );
			send_to_char( "Done.\r\n", ch );
			return;
		}
		send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
		return;
	}

	if( !str_cmp( arg, "exdesc" ) )
	{
		argument = one_argument( argument, arg2 );
		if( arg2[0] == '\0' )
		{
			send_to_char( "Create or clear a description for an exit.\r\n", ch );
			send_to_char( "Usage: redit exdesc <dir> [description]\r\n", ch );
			return;
		}
		if( arg2[0] == '#' )
		{
			edir = atoi( arg2 + 1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if( xit )
		{
			STRFREE( xit->description );
			if( !argument || argument[0] == '\0' )
				xit->description = STRALLOC( "" );
			else
			{
				sprintf( buf, "%s\r\n", argument );
				xit->description = STRALLOC( buf );
			}
			send_to_char( "Done.\r\n", ch );
			return;
		}
		send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
		return;
	}

	/*
	 * Generate usage message.
	 */
	if( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_redit;
	}
	else
		do_redit( ch, "" );
	return;
}

CMDF( do_ocreate )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int vnum, cvnum;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobiles cannot create.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	vnum = is_number( arg ) ? atoi( arg ) : -1;

	if( vnum == -1 || !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: ocreate <vnum> [copy vnum] <item name>\r\n", ch );
		return;
	}

	if( vnum < 1 || vnum > MAX_VNUMS )
	{
		send_to_char( "Bad number.\r\n", ch );
		return;
	}

	one_argument( argument, arg2 );
	cvnum = atoi( arg2 );
	if( cvnum != 0 )
		argument = one_argument( argument, arg2 );
	if( cvnum < 1 )
		cvnum = 0;

	if( get_obj_index( vnum ) )
	{
		send_to_char( "An object with that number already exists.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) )
		return;
	if( get_trust( ch ) <= LEVEL_STAFF )
	{
		AREA_DATA *pArea;

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to create objects.\r\n", ch );
			return;
		}
		if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
		{
			send_to_char( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}
	if( !is_valid_vnum( vnum, VCHECK_OBJ ) )
	{
		ch_printf( ch, "&RSorry, %d is not a valid vnum!\r\n", vnum );
		return;
	}

	pObjIndex = make_object( vnum, cvnum, argument );
	if( !pObjIndex )
	{
		send_to_char( "Error.\r\n", ch );
		log_string( "do_ocreate: make_object failed." );
		return;
	}
	obj = create_object( pObjIndex, get_trust( ch ) );
	obj_to_char( obj, ch );
	act( AT_IMMORT, "$n makes some ancient arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
	act( AT_IMMORT, "You make some ancient arcane gestures, and open your hands to reveal $p!", ch, obj, NULL, TO_CHAR );
}

CMDF( do_mcreate )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *mob;
	int vnum, cvnum;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mobiles cannot create.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	vnum = is_number( arg ) ? atoi( arg ) : -1;

	if( vnum == -1 || !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: mcreate <vnum> [cvnum] <mobile name>\r\n", ch );
		return;
	}

	if( vnum < 1 || vnum > MAX_VNUMS )
	{
		send_to_char( "Bad number.\r\n", ch );
		return;
	}

	one_argument( argument, arg2 );
	cvnum = atoi( arg2 );
	if( cvnum != 0 )
		argument = one_argument( argument, arg2 );
	if( cvnum < 1 )
		cvnum = 0;

	if( get_mob_index( vnum ) )
	{
		send_to_char( "A mobile with that number already exists.\r\n", ch );
		return;
	}

	if( IS_NPC( ch ) )
		return;
	if( get_trust( ch ) <= LEVEL_STAFF )
	{
		AREA_DATA *pArea;

		if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
		{
			send_to_char( "You must have an assigned area to create mobiles.\r\n", ch );
			return;
		}
		if( vnum < pArea->low_vnum || vnum > pArea->hi_vnum )
		{
			send_to_char( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}
	if( !is_valid_vnum( vnum, VCHECK_MOB ) )
	{
		ch_printf( ch, "&RSorry, %d is not a valid vnum!\r\n", vnum );
		return;
	}

	pMobIndex = make_mobile( vnum, cvnum, argument );
	if( !pMobIndex )
	{
		send_to_char( "Error.\r\n", ch );
		log_string( "do_mcreate: make_mobile failed." );
		return;
	}
	mob = create_mobile( pMobIndex );
	char_to_room( mob, ch->in_room );
	act( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
	act( AT_IMMORT, "You wave your arms about, and $N appears at your command!", ch, NULL, mob, TO_CHAR );
}

int strlen_codes( const char *argument );

/*
 * Simple but nice and handle line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	EDITOR_DATA *edit;
	char cmd[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	short x, line, max_buf_lines;
	bool save;

	if( ( d = ch->desc ) == NULL )
	{
		send_to_char( "You have no descriptor.\r\n", ch );
		return;
	}

	if( d->connected != CON_EDITING )
	{
		send_to_char( "You can't do that!\r\n", ch );
		bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
		return;
	}

	if( ch->substate <= SUB_PAUSE )
	{
		send_to_char( "You can't do that!\r\n", ch );
		bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
		d->connected = CON_PLAYING;
		return;
	}

	if( !ch->editor )
	{
		send_to_char( "You can't do that!\r\n", ch );
		bug( "Edit_buffer: null editor", 0 );
		d->connected = CON_PLAYING;
		return;
	}

	edit = ch->editor;
	save = false;
	max_buf_lines = 24;

	if( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT )
		max_buf_lines = 48;

	if( argument[0] == '/' || argument[0] == '\\' )
	{
		one_argument( argument, cmd );
		if( !str_cmp( cmd + 1, "?" ) )
		{
			send_to_char( "Editing commands\r\n---------------------------------\r\n", ch );
			send_to_char( "/l              list buffer\r\n", ch );
			send_to_char( "/c              clear buffer\r\n", ch );
			send_to_char( "/d [line]       delete line\r\n", ch );
			send_to_char( "/g <line>       goto line\r\n", ch );
			send_to_char( "/i <line>       insert line\r\n", ch );
			send_to_char( "/r <old> <new>  global replace\r\n", ch );
			send_to_char( "/a              abort editing\r\n", ch );
			send_to_char( "/f              format text ( to fit screen )\r\n", ch );
			if( get_trust( ch ) > LEVEL_STAFF )
				send_to_char( "/! <command>    execute command (do not use another editing command)\r\n", ch );
			send_to_char( "/s              save buffer\r\r\n\n> ", ch );
			return;
		}

		if( !str_cmp( cmd + 1, "c" ) )
		{

			memset( edit, '\0', sizeof( EDITOR_DATA ) );
			edit->numlines = 0;
			edit->on_line = 0;
			send_to_char( "Buffer cleared.\r\n> ", ch );
			return;
		}
		if( !str_cmp( cmd + 1, "r" ) )
		{
			char word1[MAX_INPUT_LENGTH];
			char word2[MAX_INPUT_LENGTH];
			char *sptr, *wptr, *lwptr;
			int xx, count, wordln, lineln;

			sptr = one_argument( argument, word1 );
			sptr = one_argument( sptr, word1 );
			sptr = one_argument( sptr, word2 );
			if( word1[0] == '\0' || word2[0] == '\0' )
			{
				send_to_char( "Need word to replace, and replacement.\r\n> ", ch );
				return;
			}
			if( strcmp( word1, word2 ) == 0 )
			{
				send_to_char( "Done.\r\n> ", ch );
				return;
			}
			count = 0;
			wordln = strlen( word1 );
			ch_printf( ch, "Replacing all occurrences of %s with %s...\r\n", word1, word2 );
			for( xx = 0; xx < edit->numlines; xx++ )
			{
				lwptr = edit->line[xx];
				while( ( wptr = strstr( lwptr, word1 ) ) != NULL )
				{
					sptr = lwptr;
					lwptr = wptr + wordln;
					sprintf( buf, "%s%s", word2, wptr + wordln );
					lineln = wptr - edit->line[xx] - wordln;
					++count;
					if( strlen( buf ) + lineln > 79 )
					{
						lineln = UMAX( 0, ( 79 - strlen( buf ) ) );
						buf[lineln] = '\0';
						break;
					}
					else
						lineln = strlen( buf );
					buf[lineln] = '\0';
					mudstrlcpy( wptr, buf, MSL );
				}
			}
			ch_printf( ch, "Found and replaced %d occurrence(s).\r\n> ", count );
			return;
		}

		if( !str_cmp( cmd + 1, "f" ) )
		{
			char temp_buf[5000];
			int xx, ep, old_p, end_mark;
			int p = 0;

			for( xx = 0; xx < edit->numlines; xx++ )
			{
				mudstrlcpy( temp_buf + p, edit->line[xx], MSL );
				p += strlen( edit->line[xx] );
				temp_buf[p] = ' ';
				p++;
			}

			temp_buf[p] = '\0';
			end_mark = p;
			p = 75;
			old_p = 0;
			edit->on_line = 0;
			edit->numlines = 0;

			while( old_p < end_mark )
			{
				while( temp_buf[p] != ' ' && p > old_p )
					p--;

				if( p == old_p )
					p += 75;

				if( p > end_mark )
					p = end_mark;

				ep = 0;
				for( xx = old_p; xx < p; xx++ )
				{
					edit->line[edit->on_line][ep] = temp_buf[xx];
					ep++;
				}
				edit->line[edit->on_line][ep] = '\0';

				edit->on_line++;
				edit->numlines++;

				old_p = p + 1;
				p += 75;

			}

			send_to_char( "OK.\r\n> ", ch );
			return;
		}

		if( !str_cmp( cmd + 1, "i" ) )
		{
			if( edit->numlines >= max_buf_lines )
				send_to_char( "Buffer is full.\r\n> ", ch );
			else
			{
				if( argument[2] == ' ' )
					line = atoi( argument + 2 ) - 1;
				else
					line = edit->on_line;
				if( line < 0 )
					line = edit->on_line;
				if( line < 0 || line > edit->numlines )
					send_to_char( "Out of range.\r\n> ", ch );
				else
				{
					for( x = ++edit->numlines; x > line; x-- )
						mudstrlcpy( edit->line[x], edit->line[x - 1], MSL );
					mudstrlcpy( edit->line[line], "", MSL );
					send_to_char( "Line inserted.\r\n> ", ch );
				}
			}
			return;
		}
		if( !str_cmp( cmd + 1, "d" ) )
		{
			if( edit->numlines == 0 )
				send_to_char( "Buffer is empty.\r\n> ", ch );
			else
			{
				if( argument[2] == ' ' )
					line = atoi( argument + 2 ) - 1;
				else
					line = edit->on_line;
				if( line < 0 )
					line = edit->on_line;
				if( line < 0 || line > edit->numlines )
					send_to_char( "Out of range.\r\n> ", ch );
				else
				{
					if( line == 0 && edit->numlines == 1 )
					{
						memset( edit, '\0', sizeof( EDITOR_DATA ) );
						edit->numlines = 0;
						edit->on_line = 0;
						send_to_char( "Line deleted.\r\n> ", ch );
						return;
					}
					for( x = line; x < ( edit->numlines - 1 ); x++ )
						mudstrlcpy( edit->line[x], edit->line[x + 1], MSL );
					mudstrlcpy( edit->line[edit->numlines--], "", MSL );
					if( edit->on_line > edit->numlines )
						edit->on_line = edit->numlines;
					send_to_char( "Line deleted.\r\n> ", ch );
				}
			}
			return;
		}
		if( !str_cmp( cmd + 1, "g" ) )
		{
			if( edit->numlines == 0 )
				send_to_char( "Buffer is empty.\r\n> ", ch );
			else
			{
				if( argument[2] == ' ' )
					line = atoi( argument + 2 ) - 1;
				else
				{
					send_to_char( "Goto what line?\r\n> ", ch );
					return;
				}
				if( line < 0 )
					line = edit->on_line;
				if( line < 0 || line > edit->numlines )
					send_to_char( "Out of range.\r\n> ", ch );
				else
				{
					edit->on_line = line;
					ch_printf( ch, "(On line %d)\r\n> ", line + 1 );
				}
			}
			return;
		}
		if( !str_cmp( cmd + 1, "l" ) )
		{
			if( edit->numlines == 0 )
				send_to_char( "Buffer is empty.\r\n> ", ch );
			else
			{
				send_to_char( "------------------\r\n", ch );
				for( x = 0; x < edit->numlines; x++ )
					ch_printf( ch, "%2d> %s\r\n", x + 1, edit->line[x] );
				send_to_char( "------------------\r\n> ", ch );
			}
			return;
		}
		if( !str_cmp( cmd + 1, "a" ) )
		{
			send_to_char( "\r\nAborting... ", ch );
			stop_editing( ch );
			return;
		}
		if( get_trust( ch ) > LEVEL_STAFF && !str_cmp( cmd + 1, "!" ) )
		{
			DO_FUN *last_cmd;
			int substate = ch->substate;

			last_cmd = ch->last_cmd;
			ch->substate = SUB_RESTRICTED;
			interpret( ch, argument + 3 );
			ch->substate = substate;
			ch->last_cmd = last_cmd;
			set_char_color( AT_GREEN, ch );
			send_to_char( "\r\n> ", ch );
			return;
		}
		if( !str_cmp( cmd + 1, "s" ) )
		{
			d->connected = CON_PLAYING;
			if( !ch->last_cmd )
				return;
			( *ch->last_cmd ) ( ch, "" );
			return;
		}
	}

	if( edit->size + strlen( argument ) + 1 >= MAX_STRING_LENGTH - 1 )
		send_to_char( "Your buffer is full.\r\n", ch );
	else
	{
		int normal = strlen( argument );
		int nocolor = strlen_color( argument );
		int max = UMIN( MAX_LINE_LENGTH, ( 79 + ( normal - nocolor ) ) );

		if( nocolor > 79 )
		{
			strncpy( buf, argument, max );
			buf[max] = 0;
			send_to_char( "(Long line trimmed)\r\n> ", ch );
		}
		else
			strncpy( buf, argument, ( MAX_LINE_LENGTH + 2 ) );
		mudstrlcpy( edit->line[edit->on_line++], buf, MSL );
		if( edit->on_line > edit->numlines )
			edit->numlines++;
		if( edit->numlines > max_buf_lines )
		{
			edit->numlines = max_buf_lines;
			send_to_char( "Buffer full.\r\n", ch );
			save = true;
		}
	}

	if( save )
	{
		d->connected = CON_PLAYING;
		if( !ch->last_cmd )
			return;
		( *ch->last_cmd ) ( ch, "" );
		return;
	}
	send_to_char( "> ", ch );
}

void assign_area( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char taf[1024];
	AREA_DATA *tarea, *tmp;
	bool created = false;

	if( IS_NPC( ch ) )
		return;
	if( get_trust( ch ) >= LEVEL_AVATAR && ch->pcdata->low_vnum && ch->pcdata->hi_vnum )
	{
		tarea = ch->pcdata->area;
		sprintf( taf, "%s.are", capitalize( ch->name ) );
		if( !tarea )
		{
			for( tmp = first_build; tmp; tmp = tmp->next )
				if( !str_cmp( taf, tmp->filename ) )
				{
					tarea = tmp;
					break;
				}
		}
		if( !tarea )
		{
			sprintf( buf, "Creating area entry for %s", ch->name );
			log_string_plus( buf, LOG_NORMAL, ch->top_level );
			CREATE( tarea, AREA_DATA, 1 );
			LINK( tarea, first_build, last_build, next, prev );
			tarea->first_room = tarea->last_room = NULL;
			sprintf( buf, "{PROTO} %s's area in progress", ch->name );
			tarea->name = str_dup( buf );
			tarea->filename = str_dup( taf );
			sprintf( buf2, "%s", ch->name );
			tarea->author = STRALLOC( buf2 );
			tarea->owned_by = STRALLOC( "None" );
			tarea->age = 0;
			tarea->nplayer = 0;
			created = true;
		}
		else
		{
			sprintf( buf, "Updating area entry for %s", ch->name );
			log_string_plus( buf, LOG_NORMAL, ch->top_level );
		}
		tarea->low_vnum = ch->pcdata->low_vnum;
		tarea->hi_vnum = ch->pcdata->hi_vnum;
		ch->pcdata->area = tarea;
		if( created )
			sort_area( tarea, true );
	}
}

CMDF( do_aassign )
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *tarea, *tmp;

	if( IS_NPC( ch ) )
		return;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: aassign <filename.are>\r\n", ch );
		return;
	}

	if( !str_cmp( "none", argument ) || !str_cmp( "null", argument ) || !str_cmp( "clear", argument ) )
	{
		ch->pcdata->area = NULL;
		assign_area( ch );
		if( !ch->pcdata->area )
			send_to_char( "Area pointer cleared.\r\n", ch );
		else
			send_to_char( "Originally assigned area restored.\r\n", ch );
		return;
	}

	sprintf( buf, "%s", argument );
	tarea = NULL;

	/*	if ( get_trust(ch) >= sysdata.level_modify_proto )   */

	if( get_trust( ch ) >= LEVEL_LIAISON
		|| ( is_name( buf, ch->pcdata->bestowments ) && get_trust( ch ) >= sysdata.level_modify_proto ) )
		for( tmp = first_area; tmp; tmp = tmp->next )
			if( !str_cmp( buf, tmp->filename ) )
			{
				tarea = tmp;
				break;
			}

	if( !tarea )
		for( tmp = first_build; tmp; tmp = tmp->next )
			if( !str_cmp( buf, tmp->filename ) )
			{
				/*		if ( get_trust(ch) >= sysdata.level_modify_proto  */
				if( get_trust( ch ) >= LEVEL_LIAISON || is_name( tmp->filename, ch->pcdata->bestowments ) )
				{
					tarea = tmp;
					break;
				}
				else
				{
					send_to_char( "You do not have permission to use that area.\r\n", ch );
					return;
				}
			}

	if( !tarea )
	{
		if( get_trust( ch ) >= sysdata.level_modify_proto )
			send_to_char( "No such area.  Use 'zones'.\r\n", ch );
		else
			send_to_char( "No such area.  Use 'newzones'.\r\n", ch );
		return;
	}
	ch->pcdata->area = tarea;
	ch_printf( ch, "Assigning you: %s\r\n", tarea->name );
	return;
}


EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA *room, const char *keywords )
{
	EXTRA_DESCR_DATA *ed;

	for( ed = room->first_extradesc; ed; ed = ed->next )
	{
		if( is_name( keywords, ed->keyword ) )
			break;
	}
	if( !ed )
	{
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
		ed->keyword = STRALLOC( keywords );
		ed->description = STRALLOC( "" );
		top_ed++;
	}
	return ed;
}

bool DelRExtra( ROOM_INDEX_DATA *room, const char *keywords )
{
	EXTRA_DESCR_DATA *rmed;

	for( rmed = room->first_extradesc; rmed; rmed = rmed->next )
	{
		if( is_name( keywords, rmed->keyword ) )
			break;
	}
	if( !rmed )
		return false;
	UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
	STRFREE( rmed->keyword );
	STRFREE( rmed->description );
	DISPOSE( rmed );
	top_ed--;
	return true;
}

EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA *obj, const char *keywords )
{
	EXTRA_DESCR_DATA *ed;

	for( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if( is_name( keywords, ed->keyword ) )
			break;
	}
	if( !ed )
	{
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword = STRALLOC( keywords );
		ed->description = STRALLOC( "" );
		top_ed++;
	}
	return ed;
}

bool DelOExtra( OBJ_DATA *obj, const char *keywords )
{
	EXTRA_DESCR_DATA *rmed;

	for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if( is_name( keywords, rmed->keyword ) )
			break;
	}
	if( !rmed )
		return false;
	UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	STRFREE( rmed->keyword );
	STRFREE( rmed->description );
	DISPOSE( rmed );
	top_ed--;
	return true;
}

EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords )
{
	EXTRA_DESCR_DATA *ed;

	for( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if( is_name( keywords, ed->keyword ) )
			break;
	}
	if( !ed )
	{
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword = STRALLOC( keywords );
		ed->description = STRALLOC( "" );
		top_ed++;
	}
	return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords )
{
	EXTRA_DESCR_DATA *rmed;

	for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if( is_name( keywords, rmed->keyword ) )
			break;
	}
	if( !rmed )
		return false;
	UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	STRFREE( rmed->keyword );
	STRFREE( rmed->description );
	DISPOSE( rmed );
	top_ed--;
	return true;
}

void fwrite_fuss_exdesc( FILE *fpout, EXTRA_DESCR_DATA *ed )
{
	fprintf( fpout, "%s", "#EXDESC\n" );
	fprintf( fpout, "ExDescKey    %s~\n", ed->keyword );
	if( ed->description && ed->description[0] != '\0' )
		fprintf( fpout, "ExDesc       %s~\n", strip_cr( ed->description ) );
	fprintf( fpout, "%s", "#ENDEXDESC\n\n" );
}

void fwrite_fuss_exit( FILE *fpout, EXIT_DATA *pexit )
{
	fprintf( fpout, "%s", "#EXIT\n" );
	fprintf( fpout, "Direction %s~\n", strip_cr( dir_name[pexit->vdir] ) );
	fprintf( fpout, "ToRoom    %d\n", pexit->vnum );
	if( pexit->key != -1 && pexit->key > 0 )
		fprintf( fpout, "Key       %d\n", pexit->key );
	if( pexit->distance > 1 )
		fprintf( fpout, "Distance  %d\n", pexit->distance );
	if( pexit->description && pexit->description[0] != '\0' )
		fprintf( fpout, "Desc      %s~\n", strip_cr( pexit->description ) );
	if( pexit->keyword && pexit->keyword[0] != '\0' )
		fprintf( fpout, "Keywords  %s~\n", strip_cr( pexit->keyword ) );
	if( pexit->exit_info )
		fprintf( fpout, "Flags     %s~\n", flag_string( pexit->exit_info, ex_flags ) );
	fprintf( fpout, "%s", "#ENDEXIT\n\n" );
}

void fwrite_fuss_affect( FILE *fp, AFFECT_DATA *paf )
{
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
	{
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
}

// Write a prog
bool mprog_write_prog( FILE *fpout, MPROG_DATA *mprog )
{
	if( ( mprog->arglist && mprog->arglist[0] != '\0' ) )
	{
		fprintf( fpout, "%s", "#MUDPROG\n" );
		fprintf( fpout, "Progtype  %s~\n", mprog_type_to_name( mprog->type ) );
		fprintf( fpout, "Arglist   %s~\n", mprog->arglist );

		if( mprog->comlist && mprog->comlist[0] != '\0' && !mprog->fileprog )
			fprintf( fpout, "Comlist   %s~\n", strip_cr( mprog->comlist ) );

		fprintf( fpout, "%s", "#ENDPROG\n\n" );
		return true;
	}
	return false;
}

void save_reset_level( FILE *fpout, RESET_DATA *start_reset, const int level )
{
	int spaces = level * 2;

	RESET_DATA *reset;

	for( reset = start_reset; reset; )
	{
		switch( UPPER( reset->command ) )  /* extra arg1 arg2 arg3 */
		{
		case '*':
			break;

		default:
			fprintf( fpout, "%*.sReset %c %d %d %d %d\n", spaces, "",
				UPPER( reset->command ), reset->extra, reset->arg1, reset->arg2, reset->arg3 );
			break;

		case 'G':
		case 'R':
			fprintf( fpout, "%*.sReset %c %d %d %d\n", spaces, "",
				UPPER( reset->command ), reset->extra, reset->arg1, reset->arg2 );
			break;
		}  /* end of switch on command */

		/*
		 * recurse to save nested resets
		 */
		save_reset_level( fpout, reset->first_reset, level + 1 );

		/*
		 * where we go next depends on if this is a top-level reset or not - for some reason
		 */
		if( level == 0 )
			reset = reset->next;
		else
			reset = reset->next_reset;
	}  /* end of looping through resets */
}  /* end of save_reset_level */

void fwrite_fuss_room( FILE *fpout, ROOM_INDEX_DATA *room, bool install )
{
	EXIT_DATA *xit;
	//AFFECT_DATA *paf;
	EXTRA_DESCR_DATA *ed;
	MPROG_DATA *mprog;

	if( install )
	{
		CHAR_DATA *victim, *vnext;
		OBJ_DATA *obj, *obj_next;

		// remove prototype flag from room 
		xREMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );

		// purge room of (prototyped) mobiles 
		for( victim = room->first_person; victim; victim = vnext )
		{
			vnext = victim->next_in_room;
			if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
				extract_char( victim, true );
		}

		// purge room of (prototyped) objects 
		for( obj = room->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) )
				extract_obj( obj );
		}
	}

	// Get rid of the track markers before saving.
	//REMOVE_BIT( room->room_flags, ROOM_BFS_MARK );

	fprintf( fpout, "%s", "#ROOM\n" );
	fprintf( fpout, "Vnum     %d\n", room->vnum );
	fprintf( fpout, "Name     %s~\n", room->name );
	fprintf( fpout, "Sector   %s~\n", strip_cr( sector_name[room->sector_type] ) );
	if( !xIS_EMPTY( room->room_flags ) )
		fprintf( fpout, "Flags    %s~\n", ext_flag_string( &room->room_flags, r_flags ) );
	if( room->tele_delay > 0 || room->tele_vnum > 0 || room->tunnel > 0 )
		fprintf( fpout, "Stats    %d %d %d\n", room->tele_delay, room->tele_vnum, room->tunnel );
	if( room->description && room->description[0] != '\0' )
		fprintf( fpout, "Desc     %s~\n", strip_cr( room->description ) );
	if( room->owner && room->owner[0] != '\0' )
		fprintf( fpout, "Owner    %s~\n", room->owner );
	if( room->guests && room->guests[0] != '\0' )
		fprintf( fpout, "Guests   %s~\n", room->guests );

	for( xit = room->first_exit; xit; xit = xit->next )
	{
		if( IS_SET( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
			continue;

		fwrite_fuss_exit( fpout, xit );
	}

	save_reset_level( fpout, room->first_reset, 0 );

	// for( paf = room->first_permaffect; paf; paf = paf->next )
	 //   fwrite_fuss_affect( fpout, paf );

	for( ed = room->first_extradesc; ed; ed = ed->next )
		fwrite_fuss_exdesc( fpout, ed );

	if( room->mudprogs )
	{
		for( mprog = room->mudprogs; mprog; mprog = mprog->next )
			mprog_write_prog( fpout, mprog );
	}
	fprintf( fpout, "%s", "#ENDROOM\n\n" );
}

void fwrite_fuss_object( FILE *fpout, OBJ_INDEX_DATA *pObjIndex, bool install )
{
	AFFECT_DATA *paf;
	EXTRA_DESCR_DATA *ed;
	MPROG_DATA *mprog;
	int val0, val1, val2, val3, val4, val5;

	if( install )
		xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );

	fprintf( fpout, "%s", "#OBJECT\n" );
	fprintf( fpout, "Vnum     %d\n", pObjIndex->vnum );
	fprintf( fpout, "Keywords %s~\n", pObjIndex->name );
	fprintf( fpout, "Type     %s~\n", o_types[pObjIndex->item_type] );
	fprintf( fpout, "Short    %s~\n", pObjIndex->short_descr );
	if( pObjIndex->description && pObjIndex->description[0] != '\0' )
		fprintf( fpout, "Long     %s~\n", pObjIndex->description );
	if( pObjIndex->action_desc && pObjIndex->action_desc[0] != '\0' )
		fprintf( fpout, "Action   %s~\n", pObjIndex->action_desc );
	if( !xIS_EMPTY( pObjIndex->extra_flags ) )
		fprintf( fpout, "Flags    %s~\n", ext_flag_string( &pObjIndex->extra_flags, o_flags ) );
	if( pObjIndex->wear_flags )
		fprintf( fpout, "WFlags   %s~\n", flag_string( pObjIndex->wear_flags, w_flags ) );

	val0 = pObjIndex->value[0];
	val1 = pObjIndex->value[1];
	val2 = pObjIndex->value[2];
	val3 = pObjIndex->value[3];
	val4 = pObjIndex->value[4];
	val5 = pObjIndex->value[5];

	switch( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		if( IS_VALID_SN( val1 ) )
		{
			val1 = HAS_SPELL_INDEX;
		}
		if( IS_VALID_SN( val2 ) )
		{
			val2 = HAS_SPELL_INDEX;
		}
		if( IS_VALID_SN( val3 ) )
		{
			val3 = HAS_SPELL_INDEX;
		}
		break;

	case ITEM_STAFF:
	case ITEM_WAND:
		if( IS_VALID_SN( val3 ) )
		{
			val3 = HAS_SPELL_INDEX;
		}
		break;
	case ITEM_SALVE:
		if( IS_VALID_SN( val4 ) )
		{
			val4 = HAS_SPELL_INDEX;
		}
		if( IS_VALID_SN( val5 ) )
		{
			val5 = HAS_SPELL_INDEX;
		}
		break;
	}
	fprintf( fpout, "Values   %d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
	fprintf( fpout, "Stats    %d %d %d %d %d\n", pObjIndex->weight,
		pObjIndex->cost, pObjIndex->rent ? pObjIndex->rent : ( int ) ( pObjIndex->cost / 10 ),
		pObjIndex->level, pObjIndex->layers );

	for( paf = pObjIndex->first_affect; paf; paf = paf->next )
		fwrite_fuss_affect( fpout, paf );

	switch( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
		fprintf( fpout, "Spells   '%s' '%s' '%s'\n",
			IS_VALID_SN( pObjIndex->value[1] ) ?
			skill_table[pObjIndex->value[1]]->name : "NONE",
			IS_VALID_SN( pObjIndex->value[2] ) ?
			skill_table[pObjIndex->value[2]]->name : "NONE",
			IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
		break;
	case ITEM_STAFF:
	case ITEM_WAND:
		fprintf( fpout, "Spells   '%s'\n",
			IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );

		break;
	case ITEM_SALVE:
		fprintf( fpout, "Spells   '%s' '%s'\n",
			IS_VALID_SN( pObjIndex->value[4] ) ?
			skill_table[pObjIndex->value[4]]->name : "NONE",
			IS_VALID_SN( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
		break;
	}

	for( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
		fwrite_fuss_exdesc( fpout, ed );

	if( pObjIndex->mudprogs )
	{
		for( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
			mprog_write_prog( fpout, mprog );
	}

	fprintf( fpout, "%s", "#ENDOBJECT\n\n" );
}

void fwrite_fuss_mobile( FILE *fpout, MOB_INDEX_DATA *pMobIndex, bool install )
{
	SHOP_DATA *pShop;
	REPAIR_DATA *pRepair;
	MPROG_DATA *mprog;

	if( install )
		xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );

	fprintf( fpout, "%s", "#MOBILE\n" );

	fprintf( fpout, "Vnum       %d\n", pMobIndex->vnum );
	fprintf( fpout, "Keywords   %s~\n", pMobIndex->player_name );
	fprintf( fpout, "Short      %s~\n", pMobIndex->short_descr );
	if( pMobIndex->long_descr && pMobIndex->long_descr[0] != '\0' )
		fprintf( fpout, "Long       %s~\n", strip_cr( pMobIndex->long_descr ) );
	if( pMobIndex->description && pMobIndex->description[0] != '\0' )
		fprintf( fpout, "Desc       %s~\n", strip_cr( pMobIndex->description ) );
	fprintf( fpout, "Race       %s~\n", npc_race[pMobIndex->race] );
	fprintf( fpout, "Position   %s~\n", npc_position[pMobIndex->position] );
	fprintf( fpout, "DefPos     %s~\n", npc_position[pMobIndex->defposition] );
	if( pMobIndex->spec_fun && pMobIndex->spec_funname && pMobIndex->spec_funname[0] != '\0' )
		fprintf( fpout, "Specfun    %s~\n", pMobIndex->spec_funname );
	if( pMobIndex->spec_2 && pMobIndex->spec_funname2 && pMobIndex->spec_funname2[0] != '\0' )
		fprintf( fpout, "Specfun2    %s~\n", pMobIndex->spec_funname2 );
	fprintf( fpout, "Gender     %s~\n", npc_sex[pMobIndex->sex] );
	fprintf( fpout, "Actflags   %s~\n", ext_flag_string( &pMobIndex->act, act_flags ) );
	if( pMobIndex->affected_by )
		fprintf( fpout, "Affected   %s~\n", flag_string( pMobIndex->affected_by, a_flags ) );
	fprintf( fpout, "Stats1     %d %d %d %d %d %d\n", pMobIndex->alignment, pMobIndex->level, pMobIndex->mobthac0,
		pMobIndex->ac, pMobIndex->gold, pMobIndex->exp );
	fprintf( fpout, "Stats2     %d %d %d\n", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
	fprintf( fpout, "Stats3     %d %d %d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
	fprintf( fpout, "Stats4     %d %d %d %d %d\n",
		pMobIndex->height, pMobIndex->weight, pMobIndex->numattacks, pMobIndex->hitroll, pMobIndex->damroll );
	fprintf( fpout, "Attribs    %d %d %d %d %d %d %d\n",
		pMobIndex->perm_str,
		pMobIndex->perm_int,
		pMobIndex->perm_wis, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_cha, pMobIndex->perm_lck );
	fprintf( fpout, "Saves      %d %d %d %d %d\n",
		pMobIndex->saving_poison_death,
		pMobIndex->saving_wand, pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff );
	if( pMobIndex->xflags )
		fprintf( fpout, "Bodyparts  %s~\n", flag_string( pMobIndex->xflags, part_flags ) );
	if( pMobIndex->resistant )
		fprintf( fpout, "Resist     %s~\n", flag_string( pMobIndex->resistant, ris_flags ) );
	if( pMobIndex->immune )
		fprintf( fpout, "Immune     %s~\n", flag_string( pMobIndex->immune, ris_flags ) );
	if( pMobIndex->susceptible )
		fprintf( fpout, "Suscept    %s~\n", flag_string( pMobIndex->susceptible, ris_flags ) );
	if( pMobIndex->attacks )
		fprintf( fpout, "Attacks    %s~\n", flag_string( pMobIndex->attacks, attack_flags ) );
	if( pMobIndex->defenses )
		fprintf( fpout, "Defenses   %s~\n", flag_string( pMobIndex->defenses, defense_flags ) );
	if( pMobIndex->vip_flags )
		fprintf( fpout, "VIPFlags   %s~\n", flag_string( pMobIndex->vip_flags, planet_flags ) );

	// Mob has a shop? Add that data to the mob index.
	if( ( pShop = pMobIndex->pShop ) != NULL )
	{
		fprintf( fpout, "ShopData   %d %d %d %d %d %d %d %d %d\n",
			pShop->buy_type[0], pShop->buy_type[1], pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4],
			pShop->profit_buy, pShop->profit_sell, pShop->open_hour, pShop->close_hour );
	}

	// Mob is a repair shop? Add that data to the mob index.
	if( ( pRepair = pMobIndex->rShop ) != NULL )
	{
		fprintf( fpout, "RepairData %d %d %d %d %d %d %d\n",
			pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type,
			pRepair->open_hour, pRepair->close_hour );
	}

	if( pMobIndex->mudprogs )
	{
		for( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
			mprog_write_prog( fpout, mprog );
	}
	fprintf( fpout, "%s", "#ENDMOBILE\n\n" );
}

void fwrite_area_header( FILE *fpout, AREA_DATA *tarea, bool install )
{
	if( install )
		REMOVE_BIT( tarea->flags, AFLAG_PROTOTYPE );

	fprintf( fpout, "%s", "#AREADATA\n" );
	fprintf( fpout, "Version      %d\n", tarea->version );
	fprintf( fpout, "Name         %s~\n", tarea->name );
	fprintf( fpout, "Author       %s~\n", tarea->author );
	fprintf( fpout, "Ranges       %d %d %d %d\n",
		tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
	if( tarea->high_economy || tarea->low_economy )
		fprintf( fpout, "Economy      %d %d\n", tarea->high_economy, tarea->low_economy );
	if( tarea->resetmsg )
		fprintf( fpout, "ResetMsg     %s~\n", tarea->resetmsg );
	if( tarea->reset_frequency )
		fprintf( fpout, "ResetFreq    %d\n", tarea->reset_frequency );
	if( tarea->flags )
		fprintf( fpout, "Flags        %s~\n", flag_string( tarea->flags, area_flags ) );

	fprintf( fpout, "%s", "#ENDAREADATA\n\n" );
}

void fold_area( AREA_DATA *tarea, const char *fname, bool install )
{
	char buf[256];
	FILE *fpout;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *pRoomIndex;
	int vnum;

	snprintf( buf, 256, "%s.bak", fname );
	rename( fname, buf );
	if( !( fpout = FileOpen( fname, "w" ) ) )
	{
		bug( "%s: FileOpen", __func__ );
		perror( fname );
		return;
	}

	tarea->version = AREA_VERSION_WRITE;

	fprintf( fpout, "%s", "#FUSSAREA\n" );

	fwrite_area_header( fpout, tarea, install );

	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; ++vnum )
	{
		if( !( pMobIndex = get_mob_index( vnum ) ) )
			continue;
		fwrite_fuss_mobile( fpout, pMobIndex, install );
	}

	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; ++vnum )
	{
		if( !( pObjIndex = get_obj_index( vnum ) ) )
			continue;
		fwrite_fuss_object( fpout, pObjIndex, install );
	}

	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; ++vnum )
	{
		if( !( pRoomIndex = get_room_index( vnum ) ) )
			continue;
		fwrite_fuss_room( fpout, pRoomIndex, install );
	}

	fprintf( fpout, "%s", "#ENDAREA\n" );
	FileClose( fpout );
}

void old_fold_area( AREA_DATA *tarea, const char *filename, bool install )
{
	RESET_DATA *pReset, *tReset, *gReset;
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	MPROG_DATA *mprog;
	EXIT_DATA *xit;
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	SHOP_DATA *pShop;
	REPAIR_DATA *pRepair;
	char buf[MAX_STRING_LENGTH];
	FILE *fpout;
	int vnum;
	int val0, val1, val2, val3, val4, val5;
	bool complexmob;

	if( hidefoldmessage == false )
	{
		sprintf( buf, "Saving %s...", tarea->filename );
		log_string_plus( buf, LOG_NORMAL, LEVEL_LIAISON );
	}

	sprintf( buf, "%s.bak", filename );
	rename( filename, buf );
	if( ( fpout = FileOpen( filename, "w" ) ) == NULL )
	{
		bug( "fold_area: FileOpen", 0 );
		perror( filename );
		return;
	}
	if( install )
		REMOVE_BIT( tarea->flags, AFLAG_PROTOTYPE );

	fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );
	fprintf( fpout, "#VERSION %d\n\n", CURRENT_AREA_VERSION );

	fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
	fprintf( fpout, "#OWNED_BY %s~\n\n", tarea->owned_by );
	fprintf( fpout, "#RANGES\n" );
	fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range,
		tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
	fprintf( fpout, "$\n\n" );
	if( tarea->resetmsg )    /* Rennard */
		fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );
	if( tarea->reset_frequency )
		fprintf( fpout, "#FLAGS\n%d %d\n\n", tarea->flags, tarea->reset_frequency );
	else
		fprintf( fpout, "#FLAGS\n%d\n\n", tarea->flags );

	fprintf( fpout, "#ECONOMY %d %d\n\n", tarea->high_economy, tarea->low_economy );

	/*
	 * save mobiles
	 */
	fprintf( fpout, "#MOBILES\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
			continue;
		if( install )
			xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );
		if( pMobIndex->perm_str != 13 || pMobIndex->perm_int != 13
			|| pMobIndex->perm_wis != 13 || pMobIndex->perm_dex != 13
			|| pMobIndex->perm_con != 13 || pMobIndex->perm_cha != 13
			|| pMobIndex->perm_lck != 13
			|| pMobIndex->hitroll != 0 || pMobIndex->damroll != 0
			|| pMobIndex->race != 0
			|| pMobIndex->attacks != 0 || pMobIndex->defenses != 0
			|| pMobIndex->height != 0 || pMobIndex->weight != 0
			|| pMobIndex->xflags != 0 || pMobIndex->numattacks != 1 || pMobIndex->vip_flags != 0 )
			complexmob = true;
		else
			complexmob = false;
		fprintf( fpout, "#%d\n", vnum );
		fprintf( fpout, "%s~\n", pMobIndex->player_name );
		fprintf( fpout, "%s~\n", pMobIndex->short_descr );
		fprintf( fpout, "%s~\n", strip_cr( pMobIndex->long_descr ) );
		fprintf( fpout, "%s~\n", strip_cr( pMobIndex->description ) );
		fprintf( fpout, "%s %d %d %c\n", print_bitvector( &pMobIndex->act ),
			pMobIndex->affected_by, pMobIndex->alignment, complexmob ? 'Z' : 'S' );
		/*
		 * C changed to Z for swreality vip_flags
		 */

		fprintf( fpout, "%d %d %d ", pMobIndex->level, pMobIndex->mobthac0, pMobIndex->ac );
		fprintf( fpout, "%dd%d+%d ", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
		fprintf( fpout, "%dd%d+%d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
		fprintf( fpout, "%d 0\n", pMobIndex->gold );
		fprintf( fpout, "%d %d %d\n", pMobIndex->position, pMobIndex->defposition, pMobIndex->sex );
		if( complexmob )
		{
			fprintf( fpout, "%d %d %d %d %d %d %d\n",
				pMobIndex->perm_str,
				pMobIndex->perm_int,
				pMobIndex->perm_wis, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_cha, pMobIndex->perm_lck );
			fprintf( fpout, "%d %d %d %d %d\n",
				pMobIndex->saving_poison_death,
				pMobIndex->saving_wand,
				pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff );
			fprintf( fpout, "%d 0 %d %d 0 0 %d\n",
				pMobIndex->race, pMobIndex->height, pMobIndex->weight, pMobIndex->numattacks );
			fprintf( fpout, "%d %d %d %d %d %d %d %d\n",
				pMobIndex->hitroll,
				pMobIndex->damroll,
				pMobIndex->xflags,
				pMobIndex->resistant, pMobIndex->immune, pMobIndex->susceptible, pMobIndex->attacks, pMobIndex->defenses );
			fprintf( fpout, "%d 0 0 0 0 0 0 0\n", pMobIndex->vip_flags );
		}
		if( pMobIndex->mudprogs )
		{
			for( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
			fprintf( fpout, "|\n" );
		}
	}
	fprintf( fpout, "#0\n\n\n" );
	if( install && vnum < tarea->hi_vnum )
		tarea->hi_vnum = vnum - 1;

	/*
	 * save objects
	 */
	fprintf( fpout, "#OBJECTS\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
			continue;
		if( install )
			xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
		fprintf( fpout, "#%d\n", vnum );
		fprintf( fpout, "%s~\n", pObjIndex->name );
		fprintf( fpout, "%s~\n", pObjIndex->short_descr );
		fprintf( fpout, "%s~\n", pObjIndex->description );
		fprintf( fpout, "%s~\n", pObjIndex->action_desc );
		//        fprintf( fpout, "%s~\n",        pObjIndex->killer               );
		//        fprintf( fpout, "%s~\n",        pObjIndex->owner                );
  //      if( pObjIndex->layers )
		fprintf( fpout, "%d %s %d %d\n", pObjIndex->item_type,
			print_bitvector( &pObjIndex->extra_flags ), pObjIndex->wear_flags, pObjIndex->layers );
		//      else
		//         fprintf( fpout, "%d %s %d\n", pObjIndex->item_type, print_bitvector( &pObjIndex->extra_flags ),
		//		          pObjIndex->wear_flags );

		val0 = pObjIndex->value[0];
		val1 = pObjIndex->value[1];
		val2 = pObjIndex->value[2];
		val3 = pObjIndex->value[3];
		val4 = pObjIndex->value[4];
		val5 = pObjIndex->value[5];
		switch( pObjIndex->item_type )
		{
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_SCROLL:
			if( IS_VALID_SN( val1 ) )
				val1 = skill_table[val1]->slot;
			if( IS_VALID_SN( val2 ) )
				val2 = skill_table[val2]->slot;
			if( IS_VALID_SN( val3 ) )
				val3 = skill_table[val3]->slot;
			break;
		case ITEM_DEVICE:
			if( IS_VALID_SN( val3 ) )
				val3 = skill_table[val3]->slot;
			break;
		case ITEM_SALVE:
			if( IS_VALID_SN( val4 ) )
				val4 = skill_table[val4]->slot;
			if( IS_VALID_SN( val5 ) )
				val5 = skill_table[val5]->slot;
			break;
		}
		if( val4 || val5 )
			fprintf( fpout, "%d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
		else
			fprintf( fpout, "%d %d %d %d\n", val0, val1, val2, val3 );

		fprintf( fpout, "%d %d %d\n", pObjIndex->weight,
			pObjIndex->cost, pObjIndex->rent ? pObjIndex->rent : ( int ) ( pObjIndex->cost / 10 ) );

		for( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
			fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

		for( paf = pObjIndex->first_affect; paf; paf = paf->next )
			fprintf( fpout, "A\n%d %d\n", paf->location,
				( ( paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL
					|| paf->location == APPLY_REMOVESPELL
					|| paf->location == APPLY_STRIPSN
					|| paf->location == APPLY_RECURRINGSPELL )
					&& IS_VALID_SN( paf->modifier ) ) ? skill_table[paf->modifier]->slot : paf->modifier );

		if( pObjIndex->mudprogs )
		{
			for( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
			fprintf( fpout, "|\n" );
		}
	}
	fprintf( fpout, "#0\n\n\n" );
	if( install && vnum < tarea->hi_vnum )
		tarea->hi_vnum = vnum - 1;

	/*
	 * save rooms
	 */
	fprintf( fpout, "#ROOMS\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( room = get_room_index( vnum ) ) == NULL )
			continue;
		if( install )
		{
			CHAR_DATA *victim, *vnext;
			OBJ_DATA *obj, *obj_next;

			/*
			 * remove prototype flag from room
			 */
			xREMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );
			/*
			 * purge room of (prototyped) mobiles
			 */
			for( victim = room->first_person; victim; victim = vnext )
			{
				vnext = victim->next_in_room;
				if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
					extract_char( victim, true );
			}
			/*
			 * purge room of (prototyped) objects
			 */
			for( obj = room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) )
					extract_obj( obj );
			}
		}
		fprintf( fpout, "#%d\n", vnum );
		fprintf( fpout, "%s~\n", room->name );
		fprintf( fpout, "%s~\n", strip_cr( room->description ) );
		fprintf( fpout, "0 %s 0 %d %d %d %d %d\n", print_bitvector( &room->room_flags ),
			room->sector_type, 0, room->tele_delay, room->tele_vnum, room->tunnel );

		for( xit = room->first_exit; xit; xit = xit->next )
		{
			if( IS_SET( xit->exit_info, EX_PORTAL ) )  /* don't fold portals */
				continue;
			fprintf( fpout, "D%d\n", xit->vdir );
			fprintf( fpout, "%s~\n", strip_cr( xit->description ) );
			fprintf( fpout, "%s~\n", strip_cr( xit->keyword ) );
			if( xit->distance > 1 )
				fprintf( fpout, "%d %d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance );
			else
				fprintf( fpout, "%d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum );
		}
		for( pReset = room->first_reset; pReset; pReset = pReset->next )
		{
			switch( pReset->command ) /* extra arg1 arg2 arg3 */
			{
			default:
			case '*':
				break;
			case 'm':
			case 'M':
			case 'o':
			case 'O':
				fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
					pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );

				for( tReset = pReset->first_reset; tReset; tReset = tReset->next_reset )
				{
					switch( tReset->command )
					{
					case 'p':
					case 'P':
					case 'e':
					case 'E':
						fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
							tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
						if( tReset->first_reset )
						{
							for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
							{
								if( gReset->command != 'p' && gReset->command != 'P' )
									continue;
								fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
									gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
							}
						}
						break;

					case 'g':
					case 'G':
						fprintf( fpout, "  R %c %d %d %d\n", UPPER( tReset->command ),
							tReset->extra, tReset->arg1, tReset->arg2 );
						if( tReset->first_reset )
						{
							for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
							{
								if( gReset->command != 'p' && gReset->command != 'P' )
									continue;
								fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
									gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
							}
						}
						break;

					case 't':
					case 'T':
					case 'h':
					case 'H':
						fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
							tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
						break;
					}
				}
				break;

			case 'd':
			case 'D':
			case 't':
			case 'T':
			case 'h':
			case 'H':
				fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
					pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
				break;

			case 'r':
			case 'R':
				fprintf( fpout, "R %c %d %d %d\n", UPPER( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2 );
				break;
			}
		}
		for( ed = room->first_extradesc; ed; ed = ed->next )
			fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

		if( room->map )   /* maps */
		{
#ifdef OLDMAPS
			fprintf( fpout, "M\n" );
			fprintf( fpout, "%s~\n", strip_cr( room->map ) );
#endif
			fprintf( fpout, "M %d %d %d %c\n", room->map->vnum, room->map->x, room->map->y, room->map->entry );
		}

		if( room->owner && room->owner[0] != '\0' )
			fprintf( fpout, "O %s~", room->owner );
		if( room->guests && room->guests[0] != '\0' )
			fprintf( fpout, "G %s~", room->guests );

		if( room->mudprogs )
		{
			for( mprog = room->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
					mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
			fprintf( fpout, "|\n" );
		}
		fprintf( fpout, "S\n" );
	}
	fprintf( fpout, "#0\n\n\n" );
	if( install && vnum < tarea->hi_vnum )
		tarea->hi_vnum = vnum - 1;

	/*
	 * save shops
	 */
	fprintf( fpout, "#SHOPS\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
			continue;
		if( ( pShop = pMobIndex->pShop ) == NULL )
			continue;
		fprintf( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
			pShop->keeper,
			pShop->buy_type[0],
			pShop->buy_type[1],
			pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4], pShop->profit_buy, pShop->profit_sell );
		fprintf( fpout, "        %2d %2d    ; %s\n", pShop->open_hour, pShop->close_hour, pMobIndex->short_descr );
	}
	fprintf( fpout, "0\n\n\n" );

	/*
	 * save repair shops
	 */
	fprintf( fpout, "#REPAIRS\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
			continue;
		if( ( pRepair = pMobIndex->rShop ) == NULL )
			continue;
		fprintf( fpout, " %d   %2d %2d %2d         %3d %3d",
			pRepair->keeper,
			pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type );
		fprintf( fpout, "        %2d %2d    ; %s\n", pRepair->open_hour, pRepair->close_hour, pMobIndex->short_descr );
	}
	fprintf( fpout, "0\n\n\n" );

	/*
	 * save specials
	 */
	fprintf( fpout, "#SPECIALS\n" );
	for( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
	{
		if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
			continue;
		if( pMobIndex->spec_fun )
			fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, pMobIndex->spec_funname );
		if( pMobIndex->spec_2 )
			fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, pMobIndex->spec_funname2 );
	}
	fprintf( fpout, "S\n\n\n" );

	/*
	 * END
	 */
	fprintf( fpout, "#$\n" );
	FileClose( fpout );
	return;
}

CMDF( do_savearea )
{
	AREA_DATA *tarea;
	char filename[256];

	if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
	{
		send_to_char( "You don't have an assigned area to save.\r\n", ch );
		return;
	}

	if( argument[0] == '\0' )
		tarea = ch->pcdata->area;
	else
	{
		bool found;

		if( get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "You can only save your own area.\r\n", ch );
			return;
		}
		for( found = false, tarea = first_build; tarea; tarea = tarea->next )
			if( !str_cmp( tarea->filename, argument ) )
			{
				found = true;
				break;
			}
		if( !found )
		{
			send_to_char( "Area not found.\r\n", ch );
			return;
		}
	}

	if( !tarea )
	{
		send_to_char( "No area to save.\r\n", ch );
		return;
	}

	/* Ensure not wiping out their area with save before load - Scryn 8/11 */
	if( !IS_SET( tarea->status, AREA_LOADED ) )
	{
		send_to_char( "Your area is not loaded!\r\n", ch );
		return;
	}

	sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
	fold_area( tarea, filename, false );
	send_to_char( "Done.\r\n", ch );
}

//CMDF( do_loadarea )
//{
//	AREA_DATA *tarea;
//	char filename[256];
//	int tmp;
//
//	if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
//	{
//		send_to_char( "You don't have an assigned area to load.\r\n", ch );
//		return;
//	}
//
//	if( argument[0] == '\0' )
//		tarea = ch->pcdata->area;
//	else
//	{
//		bool found;
//
//		if( get_trust( ch ) < LEVEL_LIAISON )
//		{
//			send_to_char( "You can only load your own area.\r\n", ch );
//			return;
//		}
//		for( found = false, tarea = first_build; tarea; tarea = tarea->next )
//			if( !str_cmp( tarea->filename, argument ) )
//			{
//				found = true;
//				break;
//			}
//		if( !found )
//		{
//			send_to_char( "Area not found.\r\n", ch );
//			return;
//		}
//	}
//
//	if( !tarea )
//	{
//		send_to_char( "No area to load.\r\n", ch );
//		return;
//	}
//
//	/*
//	 * Stops char from loading when already loaded - Scryn 8/11
//	 */
//	if( IS_SET( tarea->status, AREA_LOADED ) )
//	{
//		send_to_char( "Your area is already loaded.\r\n", ch );
//		return;
//	}
//	sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
//	send_to_char( "Loading...\r\n", ch );
//	load_area_file( tarea, filename, true );
//	send_to_char( "Linking exits...\r\n", ch );
//	fix_area_exits( tarea );
//	if( tarea->first_room )
//	{
//		tmp = tarea->nplayer;
//		tarea->nplayer = 0;
//		send_to_char( "Resetting area...\r\n", ch );
//		reset_area( tarea );
//		tarea->nplayer = tmp;
//	}
//	send_to_char( "Done.\r\n", ch );
//}

CMDF( do_foldarea )
{
	AREA_DATA *tarea;

	set_char_color( AT_IMMORT, ch );

	if( !argument || argument[0] == '\0' )
	{
		ch_printf( ch, "Fold what?\r\n" );
		return;
	}

	if( !str_cmp( argument, "all" ) )
	{
		for( tarea = first_area; tarea; tarea = tarea->next )
			if( tarea->installed )
				fold_area( tarea, tarea->filename, false );
		ch_printf( ch, "All installed areas have been folded.\r\n" );
		return;
	}

	for( tarea = first_area; tarea; tarea = tarea->next )
	{
		if( !str_cmp( tarea->filename, argument ) )
		{
			if( !tarea->installed )
			{
				ch_printf( ch, "Cannot fold %s, use savearea instead.\r\n", tarea->filename );
				return;
			}
			ch_printf( ch, "Folding area...\r\n" );
			fold_area( tarea, tarea->filename, false );
			set_char_color( AT_IMMORT, ch );
			ch_printf( ch, "Done.\r\n" );
			return;
		}
	}
	ch_printf( ch, "No such area exists.\r\n" );
}

extern int top_area;

void write_area_list( )
{
	AREA_DATA *tarea;
	FILE *fpout;

	fpout = FileOpen( AREA_LIST, "w" );
	if( !fpout )
	{
		bug( "FATAL: cannot open area.lst for writing!\r\n", 0 );
		return;
	}
	fprintf( fpout, "help.are\n" );
	for( tarea = first_area; tarea; tarea = tarea->next )
		if( tarea->installed )
			fprintf( fpout, "%s\n", tarea->filename );
	fprintf( fpout, "$\n" );
	FileClose( fpout );
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
CMDF( do_installarea )
{
	AREA_DATA *tarea;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int num;
	DESCRIPTOR_DATA *d;

	argument = one_argument( argument, arg );
	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: installarea <filename> [Area title]\r\n", ch );
		return;
	}

	for( tarea = first_build; tarea; tarea = tarea->next )
	{
		if( !str_cmp( tarea->filename, arg ) )
		{
			if( argument && argument[0] != '\0' )
			{
				DISPOSE( tarea->name );
				tarea->name = str_dup( argument );
			}

			/*
			 * Fold area with install flag -- auto-removes prototype flags
			 */
			send_to_char( "Saving and installing file...\r\n", ch );
			fold_area( tarea, tarea->filename, true );

			/*
			 * Remove from prototype area list
			 */
			UNLINK( tarea, first_build, last_build, next, prev );

			/*
			 * Add to real area list
			 */
			LINK( tarea, first_area, last_area, next, prev );

			/*
			 * Remove it from the prototype sort list. BUGFIX: Samson 4-15-03
			 */
			UNLINK( tarea, first_bsort, last_bsort, next_sort, prev_sort );

			/*
			 * Sort the area into it's proper sort list. BUGFIX: Samson 4-15-03
			 */
			sort_area( tarea, false );

			/*
			 * Fix up author if online
			 */
			for( d = first_descriptor; d; d = d->next )
			{
				if( d->character && d->character->pcdata && d->character->pcdata->area == tarea )
				{
					/*
					 * remove area from author
					 */
					d->character->pcdata->area = NULL;
					/*
					 * clear out author vnums
					 */
					d->character->pcdata->low_vnum = 0;
					d->character->pcdata->hi_vnum = 0;
				}
			}

			top_area++;
			send_to_char( "Writing area.lst...\r\n", ch );
			write_area_list( );
			send_to_char( "Resetting new area.\r\n", ch );
			num = tarea->nplayer;
			tarea->nplayer = 0;
			reset_area( tarea );
			tarea->nplayer = num;
			tarea->installed = true;
			send_to_char( "Renaming author's building file.\r\n", ch );
			sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
			sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
			rename( arg, buf );
			send_to_char( "Done.\r\n", ch );
			return;
		}
	}
	send_to_char( "No such area exists.\r\n", ch );
	return;
}

CMDF( do_astat )
{
	AREA_DATA *tarea;
	bool proto, found;


	found = false;
	proto = false;
	for( tarea = first_area; tarea; tarea = tarea->next )
		if( !str_cmp( tarea->filename, argument ) )
		{
			found = true;
			break;
		}

	if( !found )
		for( tarea = first_build; tarea; tarea = tarea->next )
			if( !str_cmp( tarea->filename, argument ) )
			{
				found = true;
				proto = true;
				break;
			}

	if( !found )
	{
		if( argument && argument[0] != '\0' )
		{
			send_to_char( "Area not found.  Check 'zones'.\r\n", ch );
			return;
		}
		else
		{
			tarea = ch->in_room->area;
		}
	}

	ch_printf( ch, "Name: %s\n\rFilename: %-15s  Prototype: %s     Owned By: %s\r\n",
		tarea->name, tarea->filename, proto ? "yes" : "no", tarea->owned_by );
	if( !proto )
	{
		ch_printf( ch, "Max players: %d  IllegalPks: %d  dollars Looted: %d\r\n",
			tarea->max_players, tarea->illegal_pk, tarea->gold_looted );
		if( tarea->high_economy )
			ch_printf( ch, "Area economy: %d billion and %d dollars.\r\n", tarea->high_economy, tarea->low_economy );
		else
			ch_printf( ch, "Area economy: %d dollars.\r\n", tarea->low_economy );
		if( tarea->planet )
			ch_printf( ch, "Planet: %s.\r\n", tarea->planet->name );
		ch_printf( ch, "Mdeaths: %d  Mkills: %d  Pdeaths: %d  Pkills: %d\r\n",
			tarea->mdeaths, tarea->mkills, tarea->pdeaths, tarea->pkills );
	}
	ch_printf( ch, "Author: %s\n\rAge: %d   Number of players: %d\r\n", tarea->author, tarea->age, tarea->nplayer );
	ch_printf( ch, "Area flags: %s\r\n", flag_string( tarea->flags, area_flags ) );
	ch_printf( ch, "Low Vnum  : %5d     Hi Vnum   : %5d\r\n", tarea->low_vnum, tarea->hi_vnum );
	ch_printf( ch, "soft range: %d - %d.  hard range: %d - %d.\r\n",
		tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
	ch_printf( ch, "Resetmsg: %s\r\n", tarea->resetmsg ? tarea->resetmsg : "(default)" );    /* Rennard */
	ch_printf( ch, "Reset frequency: %d minutes.\r\n", tarea->reset_frequency ? tarea->reset_frequency : 15 );
}

/* check other areas for a conflict while ignoring the current area */
bool check_for_area_conflicts( AREA_DATA *carea, int lo, int hi )
{
	AREA_DATA *area;

	for( area = first_area; area; area = area->next )
		if( area != carea && check_area_conflict( area, lo, hi ) )
			return true;
	return false;
}


CMDF( do_aset )
{
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	bool found;
	int vnum, value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	vnum = atoi( argument );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Usage: aset <area filename> <field> <value>\r\n", ch );
		send_to_char( "\n\rField being one of:\r\n", ch );
		send_to_char( "  low_vnum hi_vnum name filename planet\r\n", ch );
		send_to_char( "  low_soft hi_soft low_hard hi_hard\r\n", ch );
		send_to_char( "  author owned_by resetmsg resetfreq flags\r\n", ch );
		return;
	}

	found = false;
	for( tarea = first_area; tarea; tarea = tarea->next )
		if( !str_cmp( tarea->filename, arg1 ) )
		{
			found = true;
			break;
		}

	if( !found )
		for( tarea = first_build; tarea; tarea = tarea->next )
			if( !str_cmp( tarea->filename, arg1 ) )
			{
				found = true;
				break;
			}

	if( !found )
	{
		send_to_char( "Area not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "name" ) )
	{
		DISPOSE( tarea->name );
		tarea->name = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "planet" ) )
	{
		PLANET_DATA *planet;
		planet = get_planet( argument );
		if( planet )
		{
			if( tarea->planet )
			{
				PLANET_DATA *old_planet;

				old_planet = tarea->planet;
				UNLINK( tarea, old_planet->first_area, old_planet->last_area, next_on_planet, prev_on_planet );
			}
			tarea->planet = planet;
			LINK( tarea, planet->first_area, planet->last_area, next_on_planet, prev_on_planet );
			save_planet( planet );
		}
		return;
	}

	if( !str_cmp( arg2, "filename" ) )
	{
		DISPOSE( tarea->filename );
		tarea->filename = str_dup( argument );
		write_area_list( );
		fold_area( tarea, tarea->filename, true );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "low_economy" ) )
	{
		tarea->low_economy = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "high_economy" ) )
	{
		tarea->high_economy = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "low_vnum" ) )
	{
		if( check_for_area_conflicts( tarea, tarea->low_vnum, vnum ) )
		{
			ch_printf( ch, "That would conflict with another area.\r\n" );
			return;
		}
		if( tarea->hi_vnum < vnum )
		{
			ch_printf( ch, "Can't set low_vnum higher than the hi_vnum.\r\n" );
			return;
		}
		tarea->low_vnum = vnum;
		ch_printf( ch, "Done.\r\n" );
		return;
	}

	if( !str_cmp( arg2, "hi_vnum" ) )
	{
		if( check_for_area_conflicts( tarea, tarea->hi_vnum, vnum ) )
		{
			ch_printf( ch, "That would conflict with another area.\r\n" );
			return;
		}
		if( tarea->low_vnum > vnum )
		{
			ch_printf( ch, "Can't set low_vnum lower than the low_vnum.\r\n" );
			return;
		}
		tarea->hi_vnum = vnum;
		ch_printf( ch, "Done.\r\n" );
		return;
	}

	if( !str_cmp( arg2, "low_soft" ) )
	{
		if( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch );
			return;
		}

		tarea->low_soft_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "hi_soft" ) )
	{
		if( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch );
			return;
		}

		tarea->hi_soft_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "low_hard" ) )
	{
		if( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch );
			return;
		}

		tarea->low_hard_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "hi_hard" ) )
	{
		if( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch );
			return;
		}

		tarea->hi_hard_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "author" ) )
	{
		STRFREE( tarea->author );
		tarea->author = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "owned_by" ) )
	{
		CLAN_DATA *clan;
		clan = get_clan( argument );
		if( clan )
		{
			STRFREE( tarea->owned_by );
			tarea->owned_by = STRALLOC( argument );
			send_to_char( "Done.\r\n", ch );
			return;
		}
		else
			send_to_char( "No such clan.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "resetmsg" ) )
	{
		if( tarea->resetmsg )
			DISPOSE( tarea->resetmsg );
		if( str_cmp( argument, "clear" ) )
			tarea->resetmsg = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}    /* Rennard */

	if( !str_cmp( arg2, "resetfreq" ) )
	{
		tarea->reset_frequency = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "flags" ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: aset <filename> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_areaflag( arg3 );
			if( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if( IS_SET( tarea->flags, 1 << value ) )
					REMOVE_BIT( tarea->flags, 1 << value );
				else
					SET_BIT( tarea->flags, 1 << value );
			}
		}
		return;
	}

	do_aset( ch, "" );
	return;
}


CMDF( do_rlist )
{
	ROOM_INDEX_DATA *room;
	int vnum;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	AREA_DATA *tarea;
	int lrange;
	int trange;

	if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata
		|| ( !ch->pcdata->area && get_trust( ch ) < LEVEL_LIAISON ) )
	{
		send_to_char( "You don't have an assigned area.\r\n", ch );
		return;
	}

	tarea = ch->pcdata->area;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( tarea )
	{
		if( arg1[0] == '\0' ) /* cleaned a big scary mess */
			lrange = tarea->low_vnum;    /* here.        -Thoric */
		else
			lrange = atoi( arg1 );
		if( arg2[0] == '\0' )
			trange = tarea->hi_vnum;
		else
			trange = atoi( arg2 );

		if( ( lrange < tarea->low_vnum || trange > tarea->hi_vnum ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
		trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
	}

	for( vnum = lrange; vnum <= trange; vnum++ )
	{
		if( ( room = get_room_index( vnum ) ) == NULL )
			continue;
		ch_printf( ch, "&w%5d&W) &z%s\r\n", vnum, room->name );
	}
	return;
}

CMDF( do_olist )
{
	OBJ_INDEX_DATA *obj;
	int vnum;
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lrange;
	int trange;

	/*
	 * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
	 */
	if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_LIAISON || !ch->pcdata
		|| ( !ch->pcdata->area && get_trust( ch ) < LEVEL_LIAISON ) )
	{
		send_to_char( "You don't have an assigned area.\r\n", ch );
		return;
	}
	tarea = ch->pcdata->area;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( tarea )
	{
		if( arg1[0] == '\0' ) /* cleaned a big scary mess */
			lrange = tarea->low_vnum;    /* here.        -Thoric */
		else
			lrange = atoi( arg1 );
		if( arg2[0] == '\0' )
			trange = tarea->hi_vnum;
		else
			trange = atoi( arg2 );

		if( ( lrange < tarea->low_vnum || trange > tarea->hi_vnum ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
		trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3 );
	}

	for( vnum = lrange; vnum <= trange; vnum++ )
	{
		if( ( obj = get_obj_index( vnum ) ) == NULL )
			continue;
		ch_printf( ch, "&w%5d&W) %-20s &z(%s&z)\r\n", vnum, obj->name, obj->short_descr );
	}
	return;
}

CMDF( do_mlist )
{
	MOB_INDEX_DATA *mob;
	int vnum;
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lrange;
	int trange;

	if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_LIAISON || !ch->pcdata
		|| ( !ch->pcdata->area && get_trust( ch ) < LEVEL_LIAISON ) )
	{
		send_to_char( "You don't have an assigned area.\r\n", ch );
		return;
	}

	tarea = ch->pcdata->area;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( tarea )
	{
		if( arg1[0] == '\0' ) /* cleaned a big scary mess */
			lrange = tarea->low_vnum;    /* here.        -Thoric */
		else
			lrange = atoi( arg1 );
		if( arg2[0] == '\0' )
			trange = tarea->hi_vnum;
		else
			trange = atoi( arg2 );

		if( ( lrange < tarea->low_vnum || trange > tarea->hi_vnum ) && get_trust( ch ) < LEVEL_LIAISON )
		{
			send_to_char( "That is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
		trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
	}

	for( vnum = lrange; vnum <= trange; vnum++ )
	{
		if( ( mob = get_mob_index( vnum ) ) == NULL )
			continue;
		ch_printf( ch, "&w%5d&W) %-20s &z'%s&z'\r\n", vnum, mob->player_name, mob->short_descr );
	}
}

void mpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, const char *argument )
{
	if( mptype != -1 )
	{
		mprg->type = mptype;
		if( mprg->arglist )
			STRFREE( mprg->arglist );
		mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if( !mprg->comlist )
		mprg->comlist = STRALLOC( "" );
	start_editing( ch, mprg->comlist );
	return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
CMDF( do_mpedit )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, mptype = -1, cnt;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't mpedit\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;
	case SUB_RESTRICTED:
		send_to_char( "You can't use this command from within another command.\r\n", ch );
		return;
	case SUB_MPROG_EDIT:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		mprog = ( MPROG_DATA * ) ch->dest_buf;
		if( mprog->comlist )
			STRFREE( mprog->comlist );
		mprog->comlist = copy_buffer( ch );
		stop_editing( ch );
		return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	value = atoi( arg3 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Command being one of:\r\n", ch );
		send_to_char( "  add delete insert edit list\r\n", ch );
		send_to_char( "Program being one of:\r\n", ch );
		send_to_char( "  act speech rand fight hitprcnt greet allgreet\r\n", ch );
		send_to_char( "  entry give bribe death time hour script\r\n", ch );
		return;
	}

	if( get_trust( ch ) < LEVEL_LIAISON )
	{
		if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}

	if( get_trust( ch ) < get_trust( victim ) || !IS_NPC( victim ) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		return;
	}

	if( !can_mmodify( ch, victim ) )
		return;

	if( !xIS_SET( victim->act, ACT_PROTOTYPE ) )
	{
		send_to_char( "A mobile must have a prototype flag to be mpset.\r\n", ch );
		return;
	}

	mprog = victim->pIndexData->mudprogs;

	set_char_color( AT_GREEN, ch );

	if( !str_cmp( arg2, "list" ) )
	{
		cnt = 0;
		if( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		for( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
		return;
	}

	if( !str_cmp( arg2, "edit" ) )
	{
		if( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if( arg4[0] != '\0' )
		{
			mptype = get_mpflag( arg4 );
			if( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				xCLEAR_BITS( victim->pIndexData->progtypes );
				for( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT( victim->pIndexData->progtypes, mprg->type );
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "delete" ) )
	{
		int num;
		bool found;

		if( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = false;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mptype = mprg->type;
				found = true;
				break;
			}
		}
		if( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
			if( mprg->type == mptype )
				num++;
		if( value == 1 )
		{
			mprg_next = victim->pIndexData->mudprogs;
			victim->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if( num <= 1 )
			xREMOVE_BIT( victim->pIndexData->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "insert" ) )
	{
		if( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		mptype = get_mpflag( arg4 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			xSET_BIT( victim->pIndexData->progtypes, mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			victim->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				xSET_BIT( victim->pIndexData->progtypes, mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "add" ) )
	{
		mptype = get_mpflag( arg3 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( mprog != NULL )
			for( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if( mprog )
			mprog->next = mprg;
		else
			victim->pIndexData->mudprogs = mprg;
		xSET_BIT( victim->pIndexData->progtypes, mptype );
		mpedit( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}

	do_mpedit( ch, "" );
}

CMDF( do_opedit )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, mptype = -1, cnt;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't opedit\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;
	case SUB_RESTRICTED:
		send_to_char( "You can't use this command from within another command.\r\n", ch );
		return;
	case SUB_MPROG_EDIT:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		mprog = ( MPROG_DATA * ) ch->dest_buf;
		if( mprog->comlist )
			STRFREE( mprog->comlist );
		mprog->comlist = copy_buffer( ch );
		stop_editing( ch );
		return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	value = atoi( arg3 );

	if( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Command being one of:\r\n", ch );
		send_to_char( "  add delete insert edit list\r\n", ch );
		send_to_char( "Program being one of:\r\n", ch );
		send_to_char( "  act speech rand wear remove sac zap get\r\n", ch );
		send_to_char( "  drop damage repair greet exa use\r\n", ch );
		send_to_char( "  pull push (for levers,pullchains,buttons)\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Object should be in your inventory to edit.\r\n", ch );
		return;
	}

	if( get_trust( ch ) < LEVEL_LIAISON )
	{
		if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You aren't carrying that.\r\n", ch );
			return;
		}
	}
	else
	{
		if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "Nothing like that in all the realms.\r\n", ch );
			return;
		}
	}

	if( !can_omodify( ch, obj ) )
		return;

	if( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char( "An object must have a prototype flag to be opset.\r\n", ch );
		return;
	}

	mprog = obj->pIndexData->mudprogs;

	set_char_color( AT_GREEN, ch );

	if( !str_cmp( arg2, "list" ) )
	{
		cnt = 0;
		if( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		for( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
		return;
	}

	if( !str_cmp( arg2, "edit" ) )
	{
		if( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if( arg4[0] != '\0' )
		{
			mptype = get_mpflag( arg4 );
			if( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				xCLEAR_BITS( obj->pIndexData->progtypes );
				for( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT( obj->pIndexData->progtypes, mprg->type );
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "delete" ) )
	{
		int num;
		bool found;

		if( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = false;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mptype = mprg->type;
				found = true;
				break;
			}
		}
		if( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
			if( mprg->type == mptype )
				num++;
		if( value == 1 )
		{
			mprg_next = obj->pIndexData->mudprogs;
			obj->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if( num <= 1 )
			xREMOVE_BIT( obj->pIndexData->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "insert" ) )
	{
		if( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		mptype = get_mpflag( arg4 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			xSET_BIT( obj->pIndexData->progtypes, mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			obj->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				xSET_BIT( obj->pIndexData->progtypes, mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "add" ) )
	{
		mptype = get_mpflag( arg3 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( mprog != NULL )
			for( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if( mprog )
			mprog->next = mprg;
		else
			obj->pIndexData->mudprogs = mprg;
		xSET_BIT( obj->pIndexData->progtypes, mptype );
		mpedit( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}

	do_opedit( ch, "" );
}



/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, const char *argument )
{
	if( mptype != -1 )
	{
		mprg->type = mptype;
		if( mprg->arglist )
			STRFREE( mprg->arglist );
		mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if( !mprg->comlist )
		mprg->comlist = STRALLOC( "" );
	start_editing( ch, mprg->comlist );
	return;
}

CMDF( do_rpedit )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, mptype = -1, cnt;

	if( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't rpedit\r\n", ch );
		return;
	}

	if( !ch->desc )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
	default:
		break;
	case SUB_RESTRICTED:
		send_to_char( "You can't use this command from within another command.\r\n", ch );
		return;
	case SUB_MPROG_EDIT:
		if( !ch->dest_buf )
		{
			send_to_char( "Fatal error: report to Thoric.\r\n", ch );
			bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
			ch->substate = SUB_NONE;
			return;
		}
		mprog = ( MPROG_DATA * ) ch->dest_buf;
		if( mprog->comlist )
			STRFREE( mprog->comlist );
		mprog->comlist = copy_buffer( ch );
		stop_editing( ch );
		return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	value = atoi( arg2 );
	/*
	 * argument = one_argument( argument, arg3 );
	 */

	if( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: rpedit <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "Command being one of:\r\n", ch );
		send_to_char( "  add delete insert edit list\r\n", ch );
		send_to_char( "Program being one of:\r\n", ch );
		send_to_char( "  act speech rand sleep rest rfight enter\r\n", ch );
		send_to_char( "  leave death\r\n", ch );
		send_to_char( "\r\n", ch );
		send_to_char( "You should be standing in room you wish to edit.\r\n", ch );
		return;
	}

	if( !can_rmodify( ch, ch->in_room ) )
		return;

	mprog = ch->in_room->mudprogs;

	set_char_color( AT_GREEN, ch );

	if( !str_cmp( arg1, "list" ) )
	{
		cnt = 0;
		if( !mprog )
		{
			send_to_char( "This room has no room programs.\r\n", ch );
			return;
		}
		for( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
		return;
	}

	if( !str_cmp( arg1, "edit" ) )
	{
		if( !mprog )
		{
			send_to_char( "This room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		if( arg3[0] != '\0' )
		{
			mptype = get_mpflag( arg3 );
			if( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				xCLEAR_BITS( ch->in_room->progtypes );
				for( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT( ch->in_room->progtypes, mprg->type );
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "delete" ) )
	{
		int num;
		bool found;

		if( !mprog )
		{
			send_to_char( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = false;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value )
			{
				mptype = mprg->type;
				found = true;
				break;
			}
		}
		if( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for( mprg = mprog; mprg; mprg = mprg->next )
			if( mprg->type == mptype )
				num++;
		if( value == 1 )
		{
			mprg_next = ch->in_room->mudprogs;
			ch->in_room->mudprogs = mprg_next->next;
		}
		else
			for( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if( num <= 1 )
			xREMOVE_BIT( ch->in_room->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if( !str_cmp( arg2, "insert" ) )
	{
		if( !mprog )
		{
			send_to_char( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		mptype = get_mpflag( arg2 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			xSET_BIT( ch->in_room->progtypes, mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			ch->in_room->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for( mprg = mprog; mprg; mprg = mprg->next )
		{
			if( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				xSET_BIT( ch->in_room->progtypes, mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if( !str_cmp( arg1, "add" ) )
	{
		mptype = get_mpflag( arg2 );
		if( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if( mprog )
			for( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if( mprog )
			mprog->next = mprg;
		else
			ch->in_room->mudprogs = mprg;
		xSET_BIT( ch->in_room->progtypes, mptype );
		mpedit( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}

	do_rpedit( ch, "" );
}

CMDF( do_rdelete )
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;

	argument = one_argument( argument, arg );

	/*
	 * Temporarily disable this command.
	 */
	 //return;

	if( arg[0] == '\0' )
	{
		send_to_char( "Delete which room?\r\n", ch );
		return;
	}

	/*
	 * Find the room.
	 */
	if( ( location = find_location( ch, arg ) ) == NULL )
	{
		send_to_char( "No such location.\r\n", ch );
		return;
	}

	/*
	 * Does the player have the right to delete this room?
	 */
	if( get_trust( ch ) < sysdata.level_modify_proto
		&& ( location->vnum < ch->pcdata->low_vnum || location->vnum > ch->pcdata->hi_vnum ) )
	{
		send_to_char( "That room is not in your assigned range.\r\n", ch );
		return;
	}

	/*
	 * We could go to the trouble of clearing out the room, but why?
	 */
	if( location->first_person || location->first_content )
	{
		send_to_char( "The room must be empty first.\r\n", ch );
		return;
	}

	/*
	 * Ok, we've determined that the room exists, it is empty and the
	 * player has the authority to delete it, so let's dump the thing.
	 * The function to do it is in db.c so it can access the top-room
	 * variable.
	 */
	delete_room( location );
	fix_exits( );

	send_to_char( "Room deleted.\r\n", ch );
	return;
}

CMDF( do_odelete )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *obj;
	OBJ_DATA *temp;

	argument = one_argument( argument, arg );

	/*
	 * Temporarily disable this command.
	 */
	 /*    return;*/

	if( arg[0] == '\0' )
	{
		send_to_char( "Delete which object?\r\n", ch );
		return;
	}

	/*
	 * Find the object.
	 */
	if( !( obj = get_obj_index( atoi( arg ) ) ) )
	{
		if( !( temp = get_obj_here( ch, arg ) ) )
		{
			send_to_char( "No such object.\r\n", ch );
			return;
		}
		obj = temp->pIndexData;
	}

	/*
	 * Does the player have the right to delete this room?
	 */
	if( get_trust( ch ) < sysdata.level_modify_proto
		&& ( obj->vnum < ch->pcdata->low_vnum || obj->vnum > ch->pcdata->hi_vnum ) )
	{
		send_to_char( "That object is not in your assigned range.\r\n", ch );
		return;
	}

	/*
	 * Ok, we've determined that the room exists, it is empty and the
	 * player has the authority to delete it, so let's dump the thing.
	 * The function to do it is in db.c so it can access the top-room
	 * variable.
	 */
	delete_obj( obj );

	send_to_char( "Object deleted.\r\n", ch );
	return;
}
CMDF( do_mdelete )
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *mob;
	CHAR_DATA *temp;

	argument = one_argument( argument, arg );

	/*
	 * Temporarily disable this command.
	 */
	 /*    return;*/

	if( arg[0] == '\0' )
	{
		send_to_char( "Delete which mob?\r\n", ch );
		return;
	}

	/*
	 * Find the mob.
	 */
	if( !( mob = get_mob_index( atoi( arg ) ) ) )
	{
		if( !( temp = get_char_room( ch, arg ) ) || !IS_NPC( temp ) )
		{
			send_to_char( "No such mob.\r\n", ch );
			return;
		}
		mob = temp->pIndexData;
	}

	/*
	 * Does the player have the right to delete this room?
	 */
	if( get_trust( ch ) < sysdata.level_modify_proto
		&& ( mob->vnum < ch->pcdata->low_vnum || mob->vnum > ch->pcdata->hi_vnum ) )
	{
		send_to_char( "That mob is not in your assigned range.\r\n", ch );
		return;
	}

	/*
	 * Ok, we've determined that the mob exists and the player has the
	 * authority to delete it, so let's dump the thing.
	 * The function to do it is in db.c so it can access the top_mob_index
	 * variable.
	 */
	delete_mob( mob );

	send_to_char( "Mob deleted.\r\n", ch );
	return;

}

CMDF( do_aexit )
{
	ROOM_INDEX_DATA *room;
	int i, vnum;
	AREA_DATA *tarea;
	AREA_DATA *otherarea;
	EXIT_DATA *pexit;
#ifdef OVERLANDCODE
	ENTRANCE_DATA *enter;
#endif
	int lrange;
	int trange;
	bool found = false;

	if( argument[0] == '\0' )
		tarea = ch->in_room->area;
	else
	{
		for( tarea = first_area; tarea; tarea = tarea->next )
			if( !str_cmp( tarea->filename, argument ) )
			{
				found = true;
				break;
			}

		if( !found )
		{
			for( tarea = first_build; tarea; tarea = tarea->next )
				if( !str_cmp( tarea->filename, argument ) )
				{
					found = true;
					break;
				}
		}

		if( !found )
		{
			send_to_char( "Area not found. Check 'zones' for the filename.\r\n", ch );
			return;
		}
	}

	trange = tarea->hi_vnum;
	lrange = tarea->low_vnum;

	for( vnum = lrange; vnum <= trange; vnum++ )
	{
		if( ( room = get_room_index( vnum ) ) == NULL )
			continue;

		if( xIS_SET( room->room_flags, ROOM_TELEPORT ) && ( room->tele_vnum < lrange || room->tele_vnum > trange ) )
			pager_printf( ch, "From: %-10.10s Room: %5d To: Room: %5d (Teleport)\r\n",
				tarea->filename, vnum, room->tele_vnum );

		for( i = 0; i < MAX_DIR + 1; i++ )
		{
			if( ( pexit = get_exit( room, i ) ) == NULL )
				continue;
#ifdef OVERLANDCODE
			if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
			{
				pager_printf( ch, "To: Overland %4dX %4dY From: %10.10s Room: %5d (%s)\r\n",
					pexit->x, pexit->y, tarea->filename, vnum, dir_name[i] );
				continue;
			}
#endif
			if( pexit->to_room->area != tarea )
			{
				pager_printf( ch, "To: %-10.10s Room: %5d From: %-10.10s Room: %5d (%s)\r\n",
					pexit->to_room->area->filename, pexit->vnum, tarea->filename, vnum, dir_name[i] );
			}
		}
	}

	for( otherarea = first_area; otherarea; otherarea = otherarea->next )
	{
		if( tarea == otherarea )
			continue;
		trange = otherarea->hi_vnum;
		lrange = otherarea->low_vnum;
		for( vnum = lrange; vnum <= trange; vnum++ )
		{
			if( ( room = get_room_index( vnum ) ) == NULL )
				continue;

			if( xIS_SET( room->room_flags, ROOM_TELEPORT ) )
			{
				if( room->tele_vnum >= tarea->low_vnum && room->tele_vnum <= tarea->hi_vnum )
					pager_printf( ch, "From: %-10.10s Room: %5d To: %-10.10s Room: %5d (Teleport)\r\n",
						otherarea->filename, vnum, tarea->filename, room->tele_vnum );
			}

			for( i = 0; i < MAX_DIR + 1; i++ )
			{
				if( ( pexit = get_exit( room, i ) ) == NULL )
					continue;

#ifdef OVERLANDCODE
				if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
					continue;
#endif
				if( pexit->to_room->area == tarea )
				{
					pager_printf( ch, "From: %-10.10s Room: %5d To: %-10.10s Room: %5d (%s)\r\n",
						otherarea->filename, vnum, pexit->to_room->area->filename, pexit->vnum, dir_name[i] );
				}
			}
		}
	}

#ifdef OVERLANDCODE
	for( enter = first_entrance; enter; enter = enter->next )
	{
		if( enter->vnum >= tarea->low_vnum && enter->vnum <= tarea->hi_vnum )
		{
			pager_printf( ch, "From: Overland %4dX %4dY To: Room: %5d\r\n", enter->herex, enter->herey, enter->vnum );
		}
	}
#endif
	return;
}

int generate_hp( int level, int num, int size, int plus )
{
	int max_hit = 0;

	if( !num )
		max_hit = level * 8 + number_range( level * level / 4, level * level );
	else
		max_hit = num * number_range( 1, size ) + plus;

	return max_hit;
}

/* Is valid vnum checks to make sure an area has the valid vnum for any type
   types: 0=room, 1=obj, 2=mob                     -->Keberus 12/03/08 */
bool is_valid_vnum( int vnum, short type )
{
	AREA_DATA *area;
	int low_value = -1, hi_value = -1;
	bool isValid = false;

	if( ( type < VCHECK_ROOM ) || ( type > VCHECK_MOB ) )
	{
		bug( "is_valid_vnum: bad type %d", type );
		return false;
	}
	for( area = first_area; area; area = area->next )
	{
		if( type == VCHECK_ROOM )
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}
		else if( type == VCHECK_OBJ )
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}
		else
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}

		if( ( vnum >= low_value ) && ( vnum <= hi_value ) )
		{
			isValid = true;
			break;
		}
	}
	for( area = first_build; area; area = area->next )
	{
		if( type == VCHECK_ROOM )
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}
		else if( type == VCHECK_OBJ )
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}
		else
		{
			low_value = area->low_vnum;
			hi_value = area->hi_vnum;
		}

		if( ( vnum >= low_value ) && ( vnum <= hi_value ) )
		{
			isValid = true;
			break;
		}
	}
	return isValid;
}

/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelCreate( relation_type tp, void *actor, void *subject )
{
	REL_DATA *tmp;

	if( tp < relMSET_ON || tp > relOSET_ON )
	{
		bug( "RelCreate: invalid type (%d)", tp );
		return;
	}
	if( !actor )
	{
		bug( "RelCreate: NULL actor" );
		return;
	}
	if( !subject )
	{
		bug( "RelCreate: NULL subject" );
		return;
	}
	for( tmp = first_relation; tmp; tmp = tmp->next )
		if( tmp->Type == tp && tmp->Actor == actor && tmp->Subject == subject )
		{
			bug( "RelCreate: duplicated relation" );
			return;
		}
	CREATE( tmp, REL_DATA, 1 );
	tmp->Type = tp;
	tmp->Actor = actor;
	tmp->Subject = subject;
	LINK( tmp, first_relation, last_relation, next, prev );
}


/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelDestroy( relation_type tp, void *actor, void *subject )
{
	REL_DATA *rq;

	if( tp < relMSET_ON || tp > relOSET_ON )
	{
		bug( "RelDestroy: invalid type (%d)", tp );
		return;
	}
	if( !actor )
	{
		bug( "RelDestroy: NULL actor" );
		return;
	}
	if( !subject )
	{
		bug( "RelDestroy: NULL subject" );
		return;
	}
	for( rq = first_relation; rq; rq = rq->next )
		if( rq->Type == tp && rq->Actor == actor && rq->Subject == subject )
		{
			UNLINK( rq, first_relation, last_relation, next, prev );
			/*
			 * Dispose will also set to NULL the passed parameter
			 */
			DISPOSE( rq );
			break;
		}
}
