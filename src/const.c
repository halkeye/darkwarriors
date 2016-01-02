/*****************************************************************************************
 *                       DDDDD        A        RRRRRRR     K    K                        *
 *                       D    D      A A       R      R    K   K                         *
 *                       D     D    A   A      R      R    KK K                          *
 *                       D     D   A     A     RRRRRRR     K K                           *
 *                       D     D  AAAAAAAAA    R    R      K  K                          *
 *                       D    D  A         A   R     R     K   K                         *
 *                       DDDDD  A           A  R      R    K    K                        *
 *                                                                                       *
 *                                                                                       *
 *W      WW      W    A        RRRRRRR   RRRRRRR   IIIIIIII    OOOO   RRRRRRR     SSSSS  *
 * W    W  W    W    A A       R      R  R      R     II      O    O  R      R   S       *
 * W    W  W    W   A   A      R      R  R      R     II     O      O R      R   S       *
 * W    W  W    W  A     A     RRRRRRR   RRRRRRR      II     O      O RRRRRRR     SSSSS  *
 *  W  W    W  W  AAAAAAAAA    R    R    R    R       II     O      O R    R           S *
 *  W W     W W  A         A   R     R   R     R      II      O    O  R     R          S *
 *   W       W  A           A  R      R  R      R  IIIIIIII    OOOO   R      R    SSSSS  *
 *                                                                                       *
 *****************************************************************************************
 *                                                                                       *
 * Dark Warrior Code additions and changes from the Star Wars Reality code copyright (c) *
 * 2003 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
 *                                                                                       *
 * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
 * by Sean Cooper                                                                        *
 *                                                                                       *
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
 *****************************************************************************************
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                                 SWR HTML Header file                                  *
 ****************************************************************************************/

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
         * Race name      Racial Affects   str dex wis int con cha lck frc hp mn re su  RESTRICTION  LANGUAGE 
         */
        {"Human", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_BASIC},
        {"Wookiee", 0, 8, -1, -4, -1, 2, -4, 0, 0, 3, 0, 0, 0, 0,
         LANG_WOOKIEE},
        {"Twi'lek", 0, -2, 2, 0, 3, -3, 0, 0, 0, 0, 0, 0, 0, 0, LANG_TWI_LEK},
        {"Rodian", 0, 3, -1, 0, 0, -2, 0, 0, 0, 0, 0, 0, 0, 0, LANG_RODIAN},
        {"Hutt", 0, 3, -5, 1, 3, 3, -5, 0, 0, 3, 0, 0, 0, 0, LANG_HUTT},
        {"Mon Calamari", AFF_AQUA_BREATH, 0, -1, -1, 0, 4, 0, -2, 0, 0, 0, 0,
         0, 0, LANG_MON_CALAMARI},
        {"Noghri", AFF_SNEAK, 0, 8, -2, -1, 0, -5, 0, 0, 0, 0, 0, 0, 0,
         LANG_NOGHRI},
        {"Gamorrean", 0, 8, 0, -4, -5, 6, -5, 0, 0, 3, 0, 0, 0, 0,
         LANG_GAMORREAN},
        {"Jawa", 0, -3, 3, 2, 2, -1, -3, 0, 0, 0, 0, 0, 0, 0, LANG_JAWA},
        {"Adarian", 0, -2, -2, 2, 2, -2, 2, 0, 0, 0, 0, 0, 0, 0,
         LANG_ADARIAN},
        {"Ewok", 0, -3, -1, -1, 0, -3, 8, 0, 0, 0, 0, 0, 0, 0, LANG_EWOK},
        {"Verpine", 0, -1, 1, -1, 6, -3, -2, 0, 0, 0, 0, 0, 0, 0,
         LANG_VERPINE},
        {"Defel", AFF_INVISIBLE, 1, 4, -2, -2, 3, -4, 0, 0, 0, 0, 0, 0, 0,
         LANG_DEFEL},
        {"Trandoshan", AFF_INFRARED, 2, 0, -3, -2, 6, -3, 0, 0, 0, 0, 0, 0, 0,
         LANG_TRANDOSHAN},
        {"Shistavanan", 0, 1, 1, 1, 1, 0, -4, 0, 0, 0, 0, 0, 0, 0,
         LANG_SHISTAVANAN},
        {"Droid", AFF_INFRARED, -2, -2, 6, 6, -2, -2, -4, 0, 0, 0, 0, 0, 0,
         LANG_BINARY},
        {"Duinuogwuin", AFF_FLYING, 0, -3, 0, 0, 7, -4, 0, 0, 0, 10, 0, 0, 0,
         LANG_DUINUOGWUIN},
        {"Chiss", 0, -1, -2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_CSILLIAN},
        {"Kel Dor", 0, 0, -2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_KEL_DOR},
        {"Bothan", 0, -3, 2, 0, 1, -3, 3, 0, 0, 0, 0, 0, 0, 0, LANG_BOTHAN},
        {"Barabel", 0, 4, 1, -3, -3, 4, -3, 0, 0, 0, 0, 0, 0, 0,
         LANG_BARABEL},
        {"Ithorian", 0, -2, 1, 2, 2, -3, 0, 0, 0, 0, 0, 0, 0, 0,
         LANG_ITHORIAN},
        {"Devaronian", 0, 1, 1, -2, 1, 1, -3, 0, 0, 0, 0, 0, 0, 0,
         LANG_DEVARONIAN},
        {"Duro", 0, -2, 3, 1, 2, -2, -2, 0, 0, 0, 0, 0, 0, 0, LANG_DUROSIAN},
        {"Gotal", 0, 1, 1, 2, -2, 1, -3, 0, 0, 0, 0, 0, 0, 0, LANG_GOTAL},
        {"Talz", 0, 3, -1, -2, -1, 3, -2, 0, 0, 0, 0, 0, 0, 0, LANG_TALZ},
        {"Ho'Din", 0, -1, 1, 1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, LANG_HO_DIN},
        {"Falleen", 0, 2, -2, 0, 5, -2, -3, 0, 0, 0, 0, 0, 0, 0,
         LANG_FALLEEN},
        {"Givin", 0, -4, -4, 3, 6, 3, -4, 0, 0, 0, 0, 0, 0, 0, LANG_GIVIN}
};



char     *const hair_list[] = {
        "none", "bald", "hairless", "black", "brown",
        "red", "blonde", "argent",
        "golden", "platinum",
        "jet", "midnite", "charcoal", "expresso",
        "chocolate", "walnut",
        "pecan", "blonde",
        "auburn", "paprika", "mahogany", "pewter", "silver", "snowflake",
        "purple", "navy", "blue",
        "hunter green", "cyan"
};

char     *const eye_list[] = {
        "none", "blue", "green", "brown", "grey",
        "black", "hazel", "aqua", "royal blue",
        "baby blue", "evergreen", "jade",
        "midnite", "black", "purple", "blood",
        "charcoal", "sky", "lavender"
};

char     *const build_list[] = {
        "none", "thin", "lean", "muscular", "bulky",
        "fat", "frail", "toned", "slender",
        "willowy"
};

char     *const height_list[] = {
        "none", "tiny", "small", "short", "average",
        "tall", "large", "huge"
};

char     *const weight_list[] = {
        "none", "feathery", "light", "average", "heavy",
        "massive"
};

char     *const complextion_list[] = {
        "none", "pasty", "light", "drained", "dark",
        "leathery", "wrinkley", "reddened", "fair",
        "pale"
};


char     *const illness_list[] = {
        "none", "sniffles", "gas", "cough", "cold",
        "flu", "strep", "pneumonia",
        "eye infection", "ear infection",
        "throat infection", "bachularia", "angina", "depression",
        "cancer", "bubonic plague",
        "aids"
};

char     *const npc_race[MAX_NPC_RACE] = {
        "Human", "Wookiee", "Twi'lek", "Rodian", "Hutt", "Mon Calamari",
        "Noghri",
        "Gamorrean", "Jawa", "Adarian", "Ewok", "Verpine", "Defel",
        "Trandoshan", "Shistavanan", "Droid",
        "Duinuogwuin", "Chiss", "Kel Dor", "Bothan", "Barabel", "Ithorian",
        "Devaronian", "Duro", "Gotal", "Talz",
        "Ho'din", "Falleen", "Givin", "r29", "r30", "r31", "r32", "r33",
        "r34", "r35", "r36", "r37", "r38", "r39", "r40",
        "r41", "r42", "r43", "r44", "r45", "r46",
        "r47", "r48", "r49", "r50", "r51", "r52", "r53",
        "r54", "r55", "r56", "r57", "r58", "r59",
        "r60", "r61", "r62", "r63", "r64", "r65", "r66",
        "r67", "r68", "r69", "r70", "r71", "r72", "r73", "r74",
        "r75", "r76", "r77", "r78", "r79", "r80", "r81", "r82",
        "r83", "r84", "r85", "r86", "r87", "r88", "r89", "r90"
};


char     *const ability_name[MAX_ABILITY] = {
        "combat", "piloting", "engineering", "bounty hunting", "smuggling",
        "diplomacy",
        "leadership", "force", "occupation", "piracy", "medic"
};


/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[41] = {
        {-5, -4, 0, 0}, /* 0  */
        {-5, -4, 3, 1}, /* 1  */
        {-3, -2, 3, 2},
        {-3, -1, 10, 3},    /* 3  */
        {-2, -1, 25, 4},
        {-2, -1, 55, 5},    /* 5  */
        {-1, 0, 80, 6},
        {-1, 0, 90, 8},
        {0, 0, 100, 10},
        {0, 0, 100, 12},
        {0, 0, 115, 14},    /* 10  */
        {0, 0, 115, 15},
        {0, 0, 140, 16},
        {0, 0, 140, 17},    /* 13  */
        {0, 1, 170, 18},
        {1, 1, 170, 19},    /* 15  */
        {1, 2, 195, 20},
        {2, 3, 220, 22},
        {2, 4, 250, 25},    /* 18  */
        {3, 5, 400, 30},
        {3, 6, 500, 35},    /* 20  */
        {4, 7, 600, 40},
        {5, 7, 700, 45},
        {6, 8, 800, 50},
        {8, 10, 900, 55},
        {10, 12, 999, 60},  /* 25   */
        {12, 14, 999, 65},  /* 26   */
        {14, 16, 999, 70},  /* 27   */
        {16, 18, 999, 75},  /* 28   */
        {18, 20, 999, 80},  /* 29   */
        {20, 22, 999, 85},  /* 30   */
        {21, 24, 999, 85},  /* 31   */
        {22, 26, 999, 90},  /* 32   */
        {23, 28, 999, 95},  /* 33   */
        {24, 30, 999, 95},  /* 34   */
        {25, 32, 999, 95},  /* 35   */
        {26, 34, 999, 95},  /* 36   */
        {27, 35, 999, 95},  /* 37   */
        {28, 36, 999, 95},  /* 38   */
        {29, 37, 999, 95},  /* 39   */
        {30, 38, 999, 95},  /* 39   */
};



const struct int_app_type int_app[41] = {
        {3},    /*  0 */
        {5},    /*  1 */
        {7},
        {8},    /*  3 */
        {9},
        {10},   /*  5 */
        {11},
        {12},
        {13},
        {15},
        {17},   /* 10 */
        {19},
        {22},
        {25},
        {28},
        {31},   /* 15 */
        {34},
        {37},
        {40},   /* 18 */
        {44},
        {49},   /* 20 */
        {55},
        {60},
        {70},
        {85},
        {99},   /* 25 */
        {99},   /* 26 */
        {99},   /* 27 */
        {99},   /* 28 */
        {99},   /* 29 */
        {99},   /* 30 */
        {99},   /* 31 */
        {99},   /* 32 */
        {99},   /* 33 */
        {99},   /* 34 */
        {99},   /* 35 */
        {99},   /* 36 */
        {99},   /* 37 */
        {99},   /* 38 */
        {99},   /* 39 */
        {99},   /* 39 */

};



const struct wis_app_type wis_app[41] = {
        {0},    /*  0 */
        {0},    /*  1 */
        {0},
        {0},    /*  3 */
        {0},
        {1},    /*  5 */
        {1},
        {1},
        {1},
        {2},
        {2},    /* 10 */
        {2},
        {2},
        {2},
        {2},
        {3},    /* 15 */
        {3},
        {4},
        {5},    /* 18 */
        {5},
        {5},    /* 20 */
        {6},
        {6},
        {6},
        {6},
        {7},    /* 25 */
        {7},    /* 26 */
        {8},    /* 27 */
        {8},    /* 28 */
        {8},    /* 29 */
        {9},    /* 30 */
        {9},    /* 31 */
        {9},    /* 32 */
        {10},   /* 33 */
        {10},   /* 34 */
        {11},   /* 35 */
        {11},   /* 36 */
        {12},   /* 37 */
        {12},   /* 38 */
        {13},   /* 39 */
        {13},   /* 39 */
};



const struct dex_app_type dex_app[41] = {
        {60},   /* 0 */
        {50},   /* 1 */
        {50},
        {40},
        {30},
        {20},   /* 5 */
        {10},
        {0},
        {0},
        {0},
        {0},    /* 10 */
        {0},
        {0},
        {0},
        {0},
        {-10},  /* 15 */
        {-15},
        {-20},
        {-30},
        {-40},
        {-50},  /* 20 */
        {-60},
        {-75},
        {-90},
        {-105},
        {-120}, /* 25 */
        {-130}, /* 26 */
        {-140}, /* 27 */
        {-150}, /* 28 */
        {-160}, /* 29 */
        {-200}, /* 30 */
        {-210}, /* 31 */
        {-220}, /* 32 */
        {-230}, /* 33 */
        {-240}, /* 34 */
        {-250}, /* 35 */
        {-260}, /* 36 */
        {-270}, /* 37 */
        {-280}, /* 38 */
        {-290}, /* 39 */
        {-300}, /* 39 */
};



const struct con_app_type con_app[41] = {
        {-4, 20},   /*  0 */
        {-3, 25},   /*  1 */
        {-2, 30},
        {-2, 35},   /*  3 */
        {-1, 40},
        {-1, 45},   /*  5 */
        {-1, 50},
        {0, 55},
        {0, 60},
        {0, 65},
        {0, 70},    /* 10 */
        {0, 75},
        {0, 80},
        {0, 85},
        {0, 88},
        {1, 90},    /* 15 */
        {2, 95},
        {2, 97},
        {3, 99},    /* 18 */
        {3, 99},
        {4, 99},    /* 20 */
        {4, 99},
        {5, 99},
        {6, 99},
        {7, 99},
        {8, 99},    /* 25 */
        {8, 99},    /* 26 */
        {9, 99},    /* 27 */
        {9, 99},    /* 28 */
        {10, 99},   /* 29 */
        {10, 99},   /* 30 */
        {11, 99},   /* 31 */
        {11, 99},   /* 32 */
        {12, 99},   /* 33 */
        {12, 99},   /* 34 */
        {13, 99},   /* 35 */
        {13, 99},   /* 36 */
        {14, 99},   /* 37 */
        {14, 99},   /* 38 */
        {15, 99},   /* 39 */
        {15, 99},   /* 39 */
};


const struct cha_app_type cha_app[41] = {
        {-60},  /* 0 */
        {-50},  /* 1 */
        {-50},
        {-40},
        {-30},
        {-20},  /* 5 */
        {-10},
        {-5},
        {-1},
        {0},
        {0},    /* 10 */
        {0},
        {0},
        {0},
        {1},
        {5},    /* 15 */
        {10},
        {20},
        {30},
        {40},
        {50},   /* 20 */
        {60},
        {70},
        {80},
        {90},
        {99},   /* 25 */
        {99},   /* 26 */
        {99},   /* 27 */
        {99},   /* 28 */
        {99},   /* 29 */
        {99},   /* 30 */
        {99},   /* 31 */
        {99},   /* 32 */
        {99},   /* 33 */
        {99},   /* 34 */
        {99},   /* 35 */
        {99},   /* 36 */
        {99},   /* 37 */
        {99},   /* 38 */
        {99},   /* 39 */
        {99},   /* 39 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const struct lck_app_type lck_app[41] = {
        {60},   /* 0 */
        {50},   /* 1 */
        {50},
        {40},
        {30},
        {20},   /* 5 */
        {10},
        {0},
        {0},
        {0},
        {0},    /* 10 */
        {0},
        {0},
        {0},
        {0},
        {-10},  /* 15 */
        {-15},
        {-20},
        {-30},
        {-40},
        {-50},  /* 20 */
        {-60},
        {-75},
        {-90},
        {-105},
        {-120}, /* 25 */
        {-120}, /* 26 */
        {-120}, /* 27 */
        {-120}, /* 28 */
        {-120}, /* 29 */
        {-120}, /* 30 */
        {-120}, /* 31 */
        {-120}, /* 32 */
        {-120}, /* 33 */
        {-120}, /* 34 */
        {-120}, /* 35 */
        {-120}, /* 36 */
        {-120}, /* 37 */
        {-120}, /* 38 */
        {-120}, /* 39 */
        {-120}, /* 39 */
};

const struct frc_app_type frc_app[41] = {
        {0},    /* 0 */
        {0},    /* 1 */
        {0},
        {0},
        {0},
        {0},    /* 5 */
        {0},
        {0},
        {0},
        {0},
        {0},    /* 10 */
        {0},
        {0},
        {0},
        {0},
        {0},    /* 15 */
        {0},
        {0},
        {0},
        {0},
        {0},    /* 20 */
        {0},
        {0},
        {0},
        {0},
        {0},    /* 25 */
        {0},    /* 26 */
        {0},    /* 27 */
        {0},    /* 28 */
        {0},    /* 29 */
        {0},    /* 30 */
        {0},    /* 31 */
        {0},    /* 32 */
        {0},    /* 33 */
        {0},    /* 34 */
        {0},    /* 35 */
        {0},    /* 36 */
        {0},    /* 37 */
        {0},    /* 38 */
        {0},    /* 39 */
        {0},    /* 39 */
};



/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const struct liq_type liq_table[LIQ_MAX] = {
        {"water", "clear", {0, 1, 10}}, /*  0 */
        {"beer", "amber", {3, 2, 5}},
        {"wine", "rose", {5, 2, 5}},
        {"ale", "brown", {2, 2, 5}},
        {"dark ale", "dark", {1, 2, 5}},

        {"whisky", "golden", {6, 1, 4}},    /*  5 */
        {"lemonade", "pink", {0, 1, 8}},
        {"firebreather", "boiling", {10, 0, 0}},
        {"local specialty", "everclear", {3, 3, 3}},
        {"slime mold juice", "green", {0, 4, -8}},

        {"milk", "white", {0, 3, 6}},   /* 10 */
        {"tea", "tan", {0, 1, 6}},
        {"coffee", "black", {0, 1, 6}},
        {"blood", "red", {0, 2, -1}},
        {"salt water", "clear", {0, 1, -2}},

        {"cola", "cherry", {0, 1, 5}},  /* 15 */
        {"mead", "honey color", {4, 2, 5}}, /* 16 */
        {"grog", "thick brown", {3, 2, 5}}, /* 17 */
        {"milkshake", "creamy", {0, 8, 5}}  /* 18 */
};

char     *const attack_table[13] = {
        "hit",
        "slice", "stab", "slash", "whip", "stab",
        "blast", "pound", "crush", "shot", "bite",
        "pierce", "suction"
};

char     *const body_parts[MAX_BODY_PARTS] = {
        "left leg", "right leg", "left foot", "right foot", "left arm",
        "right arm", "left wrist", "right wrist", "left knee", "right knee",
        "left ankle", "right ankle", "left sholder", "right sholder",
        "left hand",
        "right hand", "nose", "ribs", "jaw", "stomach", "chest"
};


char     *const npc_sex[SEX_MAX] = {
        "neuter", "male", "female"
};

char     *const npc_position[POS_MAX] = {
        "dead", "mortal", "incapacitated", "stunned", "sleeping",
        "resting", "sitting", "fighting", "standing", "mounted",
        "shove", "drag"
};

char     *const log_flag[] = {
        "normal", "always", "never", "build", "high", "comm", "all"
};

char     *const true_false[] = {
        "false", "true"
};


/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI
