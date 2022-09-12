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
#include <time.h>
#include "mud.h"

/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

/*
 * Race table.
 */
const struct race_type race_table[MAX_RACE] = {
	/*
	 * race name|str|dex|wis|int|con|cha
	 */
	{"Colonist", -2, 4, -2, 4, 0, 0},
	{"American", 4, -2, 4, -2, 0, 0},
	{"Canadian", 2, 0, 2, 2, 2, -1},
	{"Japanese", 0, 3, 0, 0, 1, -1},
	{"Korean", 3, -1, -3, 3, 2, -1},
	{"Mexican", 0, -1, 2, 4, 0, 0},
	{"African", 0, 3, -2, -1, 1, -3},
	{"Australian", 5, 0, -5, -5, 5, -2},
	{"Russian", -3, 3, 1, 0, 1, -1},
	{"British", 1, 1, 0, 1, 1, 2},
	{"Chinese", -2, -1, -5, -5, -2, 5},
	{"Swedish", -1, 0, 1, 2, -3, 0},
	{"German", 1, 3, -3, -3, 1, 0},
	{"Brazillian", 2, 0, 0, 0, 5, -1},
	{"Irish", -3, 3, 0, 2, -1, 0},
	{"Scottish", -1, 1, 0, 1, -1, 0},
	{"Arabic", 0, -1, 3, 0, -2, 1}
};

const struct suit_type suitweapon_table[MAX_SUITWEAPON] = {
	/*
	 * Weapon Name,    Type, Damage, Energy Cost, Max Ammo, Ammo Type, Class, Weight, Price
	 */
	{
	 "Beam Rifle", WT_BEAM, 10, 1, 0, 0, 0, 0, 0},
	{
	 "Beam Saber", WT_SLASH, 15, 5, 0, 0, 0, 0, 0}
};

const char *const suitweapons[MAX_SUITWEAPON] = {
   "Beam Rifle", "Beam Saber"
};


const char *const hair_list[] = {
   "Brown", "Light Brown", "Red", "Light Red",
   "Blonde", "Dirty Blonde", "Strawberry Blonde",
   "Platinum Blonde", "Blue", "Light Blue",
   "Green", "Light Green", "Cyan", "Light Cyan",
   "Silver", "Gold", "Bronze", "White", "Black",
   "Grey", "Purple", "Light Purple", "Bald"
};

const char *const highlight_list[] = {
   "Brown", "Light Brown", "Red", "Light Red",
   "Blonde", "Dirty Blonde", "Strawberry Blonde",
   "Platinum Blonde", "Blue", "Light Blue",
   "Green", "Light Green", "Cyan", "Light Cyan",
   "Silver", "Gold", "Bronze", "White", "Black",
   "Grey", "Purple", "Light Purple", "None"
};

const char *const cargo_names[CTYPE_MAX] = {
   "none", "all", "cdi", "batteries", "iron", "steel",
   "titanium", "neotitanium", "gundanium"
};

const struct clan_titles clan_rank_table[] = {
   {
	{"&PR&pecruit", "&BR&becruit", "&WR&wecruit" }
	},
   {
	{"&PR&pecruit", "&BR&becruit", "&WR&wecruit"}
	},
   {
	{"&PP&private", "&BP&brivate", "&WP&wrivate"}
	},
   {
	{"&PC&porporal", "&BC&borporal", "&WC&worporal"}
	},
   {
	{"&PS&pergeant", "&BS&bergeant", "&WS&wergeant"}
	},
   {
	{"&PO&pfficer", "&BO&bfficer", "&WO&wfficer"}
	},
   {
	{"&PL&pieutenant", "&BL&bieutenant", "&WL&wieutenant"}
	},
   {
	{"&PC&paptain", "&BC&baptain", "&WC&waptain"}
	},
   {
	{"&PM&pajor", "&BM&bajor", "&WM&wajor"}
	},
   {
	{"&PC&polonel", "&BC&bolonel", "&WC&wolonel"}
	},
   {
	{"&PL&peader", "&BL&beader", "&WL&weader"}
	},
   {
	{"&PP&patron", "&BP&batron", "&WP&watron"}
	},
   {
	{NULL, NULL}
	}
};


const char *const eye_list[] = {
   "Green", "Light Green", "Brown", "Hazel",
   "Blue", "Light Blue", "Royal Blue",
   "Purple", "Light Purple", "Teal",
   "Red", "Blood Red", "Yellow",
   "Grey", "Silver", "Orange"
};

const char *const build_list[] = {
   "Frail", "Slim", "Thin", "Lean", "Slender",
   "Toned", "Athletic", "Muscular", "Fat"
};

const char *const hero_list[] = {
   "Heero", "Duo", "Quatre", "Trowa",
   "Wufei", "Treize", "Zechs", "Noin",
   "Lady Une", "Dorothy", "Catherine",
   "Hildy"
};

const char *const pc_race[MAX_RACE] = {
   "Colonist", "American", "Canadian", "Japanese", "Korean", "Mexican", "African",
   "Australian", "Russian", "British", "Chinese", "Swedish", "German", "Brazillian",
   "Irish", "Scottish", "Arabic"
};

const char *const npc_race[MAX_NPC_RACE] = {
   "Colonist", "American", "Canadian", "Japanese", "Korean", "Mexican", "African",
   "Australian", "Russian", "British", "Chinese", "Swedish", "German", "Brazillian",
   "Irish", "Scottish", "Arabic", "r17", "r18", "r19", "r20",
   "r21", "r22", "r23", "r24", "r25",
   "r26", "r27", "r28", "r29", "r30", "r31", "r32", "r33",
   "r34", "r35", "r36", "r37", "r38", "r39", "r40",
   "r41", "r42", "r43", "r44", "r45", "r46",
   "r47", "r48", "r49", "r50", "r51", "r52", "r53",
   "r54", "r55", "r56", "r57", "r58", "r59",
   "r60", "r61", "r62", "r63", "r64", "r65", "r66",
   "r67", "r68", "r69", "r70", "r71", "r72", "r73", "r74",
   "r75", "r76", "r77", "r78", "r79", "r80", "r81", "r82",
   "r83", "r84", "r85", "r86", "r87", "r88", "r89", "r90"
};


const char *const ability_name[MAX_ABILITY] = {
   "Combat", "Piloting", "Engineering", "Bounty Hunting", "Smuggling",
   "Diplomacy", "Leadership", "Espionage"
};

const char *const special_weapons[MAX_SWEAPON] = {
   "None", "Bubble Attack", "Smog Attack", "Tornado Attack", "Life Sap",
   "five", "six", "seven",
   "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen"
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
   {-5, -4, 0, 0},  /* 0  */
   {-5, -4, 3, 1},  /* 1  */
   {-3, -2, 3, 2},
   {-3, -1, 10, 3}, /* 3  */
   {-2, -1, 25, 4},
   {-2, -1, 55, 5}, /* 5  */
   {-1, 0, 80, 6},
   {-1, 0, 90, 8},
   {0, 0, 100, 10},
   {0, 0, 100, 12},
   {0, 0, 115, 14}, /* 10  */
   {0, 0, 115, 15},
   {0, 0, 140, 16},
   {0, 0, 140, 17}, /* 13  */
   {0, 1, 170, 18},
   {1, 1, 170, 19}, /* 15  */
   {1, 2, 195, 20},
   {2, 3, 220, 22},
   {2, 4, 250, 25}, /* 18  */
   {3, 5, 400, 30},
   {3, 6, 500, 35}, /* 20  */
   {4, 7, 600, 40},
   {5, 7, 700, 45},
   {6, 8, 800, 50},
   {8, 10, 900, 55},
   {10, 12, 999, 60}    /* 25   */
};



const struct int_app_type int_app[26] = {
   {3}, /*  0 */
   {5}, /*  1 */
   {7},
   {8}, /*  3 */
   {9},
   {10},    /*  5 */
   {11},
   {12},
   {13},
   {15},
   {17},    /* 10 */
   {19},
   {22},
   {25},
   {28},
   {31},    /* 15 */
   {34},
   {37},
   {40},    /* 18 */
   {44},
   {49},    /* 20 */
   {55},
   {60},
   {70},
   {85},
   {99} /* 25 */
};



const struct wis_app_type wis_app[26] = {
   {0}, /*  0 */
   {0}, /*  1 */
   {0},
   {0}, /*  3 */
   {0},
   {1}, /*  5 */
   {1},
   {1},
   {1},
   {2},
   {2}, /* 10 */
   {2},
   {2},
   {2},
   {2},
   {3}, /* 15 */
   {3},
   {4},
   {5}, /* 18 */
   {5},
   {5}, /* 20 */
   {6},
   {6},
   {6},
   {6},
   {7}  /* 25 */
};



const struct dex_app_type dex_app[26] = {
   {60},    /* 0 */
   {50},    /* 1 */
   {50},
   {40},
   {30},
   {20},    /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10},   /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50},   /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120}   /* 25 */
};



const struct con_app_type con_app[26] = {
   {-4, 20},    /*  0 */
   {-3, 25},    /*  1 */
   {-2, 30},
   {-2, 35},    /*  3 */
   {-1, 40},
   {-1, 45},    /*  5 */
   {-1, 50},
   {0, 55},
   {0, 60},
   {0, 65},
   {0, 70}, /* 10 */
   {0, 75},
   {0, 80},
   {0, 85},
   {0, 88},
   {1, 90}, /* 15 */
   {2, 95},
   {2, 97},
   {3, 99}, /* 18 */
   {3, 99},
   {4, 99}, /* 20 */
   {4, 99},
   {5, 99},
   {6, 99},
   {7, 99},
   {8, 99}  /* 25 */
};


const struct cha_app_type cha_app[26] = {
   {-60},   /* 0 */
   {-50},   /* 1 */
   {-50},
   {-40},
   {-30},
   {-20},   /* 5 */
   {-10},
   {-5},
   {-1},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {1},
   {5}, /* 15 */
   {10},
   {20},
   {30},
   {40},
   {50},    /* 20 */
   {60},
   {70},
   {80},
   {90},
   {99} /* 25 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const struct lck_app_type lck_app[26] = {
   {60},    /* 0 */
   {50},    /* 1 */
   {50},
   {40},
   {30},
   {20},    /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10},   /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50},   /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120}   /* 25 */
};

/*
 * Liquid properties.
 * Used in #OBJECT section of area file.

 Drunk, Full, Thirst
 */
const struct liq_type liq_table[LIQ_MAX] = {
   {"water", "clear", {0, 1, 10}},  /*  0 */
   {"beer", "amber", {3, 2, 5}},
   {"wine", "rose", {5, 2, 5}},
   {"ale", "brown", {2, 2, 5}},
   {"dark ale", "dark", {1, 2, 5}},

   {"whisky", "golden", {6, 1, 4}}, /*  5 */
   {"lemonade", "pink", {0, 1, 8}},
   {"firebreather", "boiling", {10, 0, 0}},
   {"local specialty", "everclear", {3, 3, 3}},
   {"slime mold juice", "green", {0, 4, -8}},

   {"milk", "white", {0, 3, 6}},    /* 10 */
   {"tea", "tan", {0, 1, 6}},
   {"coffee", "black", {0, 1, 6}},
   {"blood", "red", {0, 2, -1}},
   {"salt water", "clear", {0, 1, -2}},

   {"cola", "cherry", {0, 1, 5}},   /* 15 */
   {"mead", "honey color", {4, 2, 5}},
   {"grog", "thick brown", {3, 2, 5}},
   {"milkshake", "creamy", {0, 8, 5}},

   {"pepsi", "fizzy", {0, 1, 5}},
   {"coke", "fizzy", {0, 1, 5}},    /* 20 */
   {"vanilla coke", "fizzy", {0, 2, 5}},
   {"sprite", "clear", {0, 1, 5}},
   {"cherry coke", "fizzy", {0, 1, 5}},
   {"pepsi blue", "blue", {0, 1, 5}},
   {"apple juice", "yellow", {0, 4, 6}},    /* 25 */
   {"slushy", "icey", {0, 3, 4}},
   {"chocolate milk", "chocolatey", {0, 3, 5}},
   {"Captain Morgan", "light yellow", {10, 0, 0}},
   {"root beer", "brown", {0, 1, 5}},
   {"hot chocolate", "chocolatey", {0, 4, 5}},  /* 30 */
   {"gin", "clear", {6, 1, 5}},
   {"brandy", "dark yellow", {7, 1, 5}},
   {"rum", "clear", {7, 2, 7}},
   {"heavy liquor", "clear", {8, 1, 5}},
   {"liquor", "clear", {5, 1, 5}},
   {"champagne", "golden", {6, 1, 1}},
   {"cognac", "rust", {6, 2, 4}},
   {"alcohol", "everclear", {6, 1, 6}},
   {"mixed alcohol", "everclear", {4, 1, 7}},
   {"frozen alcohol", "everclear", {6, 5, 5}},  /* 40 */
   {"blue alcohol", "blue", {5, 1, 1}},
   {"red alcohol", "red", {5, 1, 1}},
   {"orange alcohol", "orange", {5, 1, 1}},
   {"green alcohol", "green", {5, 1, 1}},
   {"mineral water", "clear", {0, 0, 10}},  /* 45 */
};

const char *const attack_table[14] = {
   "hit",
   "slice", "stab", "slash", "whip", "claw",
   "blast", "pound", "crush", "shot", "bite",
   "pierce", "suction", "strike"
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_STAFF

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI
