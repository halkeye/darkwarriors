/* vim: ts=8 et ft=cpp sw=8
 *****************************************************************************************
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
 * 2005 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
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
 *                                SWR Main Header                                        *
 *Last revised by $Author: halkeye $ on $Date: 2005/11/28 00:11:36 $ to $Revision: 1356 $*
 ****************************************************************************************/

#ifndef _MUD_H_
#define _MUD_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <math.h>
#ifdef MCCP
#include <zlib.h>
#endif
#include <list>
#include <map>
//#include <bits/stl_alloc.h>

typedef int ch_ret;
typedef int obj_ret;

#ifdef MCCP
/*
 * MCCP defines
 */
#define COMPRESS_BUF_SIZE 1024
#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
#endif

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int sh_int;
typedef int bool;

#define unix
#else
typedef short int sh_int;

#if !defined(__cplusplus)
typedef unsigned char bool;
#endif
#endif

#define REVISION __STRING($Revision: 1356 $)


/*
 * Structure types.
 */
class     BODY_DATA;
class     RACE_DATA;

typedef struct affect_data AFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct auction_data AUCTION_DATA;   /* auction data */
typedef struct watch_data WATCH_DATA;
typedef struct extracted_char_data EXTRACT_CHAR_DATA;
typedef struct char_data CHAR_DATA;
typedef struct hunt_hate_fear HHF_DATA;
typedef struct fighting_data FIGHT_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct comment_data COMMENT_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct reset_data RESET_DATA;
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct repairshop_data REPAIR_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct hour_min_sec HOUR_MIN_SEC;
typedef struct weather_data WEATHER_DATA;
typedef struct planet_data PLANET_DATA;
typedef struct space_data SPACE_DATA;
typedef struct clan_data CLAN_DATA;
typedef struct ship_data SHIP_DATA;
typedef struct missile_data MISSILE_DATA;
typedef struct mob_prog_data MPROG_DATA;
typedef struct mpsleep_data MPSLEEP_DATA;
typedef struct mob_prog_act_list MPROG_ACT_LIST;
typedef struct timer_data TIMER;
typedef struct godlist_data GOD_DATA;
typedef struct system_data SYSTEM_DATA;
typedef struct smaug_affect SMAUG_AFF;
typedef struct who_data WHO_DATA;
typedef struct skill_type SKILLTYPE;
typedef struct social_type SOCIALTYPE;
typedef struct cmd_type CMDTYPE;
typedef struct killed_data KILLED_DATA;
typedef struct wizent WIZENT;
typedef struct specfun_list SPEC_LIST;
typedef struct extended_bitvector EXT_BV;
typedef struct log_data LOG_DATA;
typedef struct web_descriptor WEB_DESCRIPTOR;

typedef std::list < BODY_DATA * >BODY_LIST;
typedef std::list < AREA_DATA * >AREA_LIST;
typedef std::list < CLAN_DATA * >CLAN_LIST;

extern BODY_LIST bodies;

typedef void DO_FUN(CHAR_DATA * ch, char *argument);
typedef bool SPEC_FUN(CHAR_DATA * ch);
typedef ch_ret SPELL_FUN(int sn, int level, CHAR_DATA * ch, void *vo);

// Banking
#define BANK_INTEREST	1.00028571428571
#define BANK_DIR	"../banks/"
#define BACCOUNT_DIR	"../banks/accounts/"
#define BACCOUNT_LIST	"accounts.lst"
typedef struct bank_account BANK_ACCOUNT;

struct bank_account
{
        BANK_ACCOUNT *next;
        BANK_ACCOUNT *prev;
        char     *code;
        char     *creator;
        char     *owner;
        char     *trustees;
        float interest;
        long flags;
        long amounthi;
        long amountlo;

};

#if !defined(__cplusplus)
#define CMDF void
#define SPELLF ch_ret
#define SPECF bool
#else
#define CMDF extern "C" void
#define SPELLF extern "C" ch_ret
#define SPECF extern "C" bool
#endif

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#else
#define args( list )			list
#endif

/* Duuude */
#include "commands.h"


#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

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
/* 32 USED! DO NOT ADD MORE! SB >*/

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096   /* buf */
#define MAX_INPUT_LENGTH	 1024   /* arg */
#define MAX_INBUF_SIZE		 1024

#define HASHSTR /* use string hashing */

#define	MAX_LAYERS		 8  /* maximum clothing layers */
#define MAX_NEST	       100  /* maximum container nesting */

#define MAX_KILLTRACK		20  /* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   25

#define MAX_REXITS		   20   /* Maximum exits allowed in 1 room */
#define MAX_SKILL		  400
#define MAX_ABILITY		    11
#define MAX_RACE		   29
#define MAX_NPC_RACE		   91
#define MAX_VNUMS		   2000000000
#define MAX_LEVEL		  155
#define MAX_CLAN		   50
#define MAX_PLANET		  100
#define MAX_SHIP                 1000
#define MAX_BOUNTY                255
#define MAX_GOV                   255
#define MAX_IGNORE				 1000
#define MAX_CHARACTERS 10

#define	MAX_HERB		   20

#define LEVEL_HERO		   (MAX_LEVEL - 5)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 4)
#define LEVEL_SUPREME		   MAX_LEVEL
#define LEVEL_INFINITE		   (MAX_LEVEL - 1)
#define LEVEL_ETERNAL		   (MAX_LEVEL - 1)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL - 1)
#define LEVEL_SUB_IMPLEM	   (MAX_LEVEL - 1)
#define LEVEL_ASCENDANT		   (MAX_LEVEL - 2)
#define LEVEL_GREATER		   (MAX_LEVEL - 2)
#define LEVEL_GOD		   (MAX_LEVEL - 2)
#define LEVEL_LESSER		   (MAX_LEVEL - 3)
#define LEVEL_TRUEIMM		   (MAX_LEVEL - 3)
#define LEVEL_DEMI		   (MAX_LEVEL - 3)
#define LEVEL_SAVIOR		   (MAX_LEVEL - 3)
#define LEVEL_CREATOR		   (MAX_LEVEL - 3)
#define LEVEL_NEOPHYTE		   (MAX_LEVEL - 4)
#define LEVEL_AVATAR		   (MAX_LEVEL - 5)
#define LEVEL_BUILDER      LEVEL_CREATOR

#include "shell.h"
#include "pfiles.h"
#include "autobuild.h"
#include "color.h"
#include "hotboot.h"
#include "implants.h"

#ifdef CALLOC
#undef CALLOC
#endif
#define CALLOC calloc

#ifdef MALLOC
#undef MALLOC
#endif
#define MALLOC malloc

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC realloc

#ifdef FREE
#undef FREE
#endif
#define FREE free

#define LEVEL_LOG		    LEVEL_LESSER
#define LEVEL_HIGOD		    LEVEL_GOD

#define MOB_VNUM_VENDOR		6   /*vnum of vendor */
#define LEVEL_BUY_VENDOR	50  /* minimum level to buy a vendor */
#define COST_BUY_VENDOR		75000   /* cost of a vendor */
#define OBJ_VNUM_DEED		5   /* vnum of deed */
#define ROOM_VNUM_VENSTOR	5   /* where messed up vendors go! */
#define VENDOR_DIR "../vendor/"

#define PULSE_PER_SECOND	    2
#define PULSE_MINUTE              ( 60 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE		  (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  (  4 * PULSE_PER_SECOND)
#define PULSE_TICK		  ( 70 * PULSE_PER_SECOND)
#define PULSE_AREA		  ( 60 * PULSE_PER_SECOND)
#define PULSE_AUCTION             ( 10 * PULSE_PER_SECOND)
#define PULSE_SPACE               ( 10 * PULSE_PER_SECOND)
#define PULSE_TAXES               ( 60 * PULSE_MINUTE)
#define PULSE_AUTH               ( 3 * PULSE_MINUTE)
#define PULSE_CRASHOVER           ( 45 * PULSE_PER_SECOND )

/*
 * Command logging types.
 */
typedef enum
{
        LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
        LOG_ALL
} log_types;

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
        rERROR = 255, rSTOP
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
 * Defines for extended bitvectors, added to SWR by Greven
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM		31  /* extended bitmask   ( INTBITS - 1 )   */
#define RSV		5   /* right-shift value  ( sqrt(XBM+1) )   */
#define XBI		4   /* integers in an extended bitvector    */
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
        int bits[XBI];
};

#include "olc.h"
#include "dns.h"
#ifdef IMC
#include "imc.h"
#endif

/*
 * do_who output structure -- Narn
 */
struct who_data
{
        WHO_DATA *prev;
        WHO_DATA *next;
        char     *text;
        int type;
};

/*
 * Player watch data structure  --Gorog
 */
struct watch_data
{
        WATCH_DATA *next;
        WATCH_DATA *prev;
        sh_int imm_level;
        char     *imm_name; /* imm doing the watching */
        char     *target_name;  /* player or command being watched   */
        char     *player_site;  /* site being watched     */
#ifdef ACCOUNT
        char     *player_account;   /* account being watched */
#endif
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

struct time_info_data
{
        int hour;
        int day;
        int month;
        int year;
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
};




/*
 * Structure used to build wizlist
 */
struct wizent
{
        WIZENT   *next;
        WIZENT   *last;
        char     *name;
        sh_int level;
        int flags;
};

/*
 * Connected state for a channel.
 */
typedef enum
{
        CON_GET_NAME, CON_GET_OLD_PASSWORD,
        CON_CONFIRM_NEW_NAME, CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD,
        CON_ACCOUNT_GET_EMAIL,
        CON_GET_NEW_SEX, CON_READ_MOTD, CON_GET_NEW_RACE,
        CON_GET_EMULATION, CON_GET_WANT_RIPANSI,
        CON_TITLE, CON_PRESS_ENTER, CON_WAIT_1,
        CON_WAIT_2, CON_WAIT_3, CON_ACCEPTED,
        CON_GET_PKILL, CON_READ_IMOTD, CON_GET_NEW_EMAIL,
        CON_GET_MSP, CON_GET_NEW_CLASS, CON_ROLL_STATS,
        CON_SHOW_STAT_OPTIONS, CON_EDIT_STATS, CON_STATS_OK,
        CON_COPYOVER_RECOVER, CON_FORKED, CON_IAFORKED,
        CON_WIZINVIS, CON_EDIT_STAT_NUM, CON_MENU
#ifdef ACCOUNT
                , CON_NEW_ACCOUNT, CON_GET_ACCOUNT,
        CON_GET_OLD_ACCOUNT_PASSWORD,
        CON_GET_ALT, CON_GET_NEW_ACCOUNT_PASSWORD,
        CON_CONFIRM_NEW_ACCOUNT_PASSWORD,
        CON_LINK_ALT, CON_GET_LINK_PASSWORD, CON_CONFIRM_NEW_ACCOUNT_NAME,
        CON_GET_ACC_OLDPASS, CON_GET_ACC_NEWPASS, CON_GET_ACC_CONFIRMPASS
#endif
        , CON_PLAYING, CON_EDITING
} connection_types;




/*
 * Character substates
 */
typedef enum
{
        SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
        SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_BAN_DESC,
        SUB_ROOM_DESC, SUB_ROOM_EXTRA,
        SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
        SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
        SUB_DEITYDESC, SUB_WRITING_EMAIL, SUB_ALIASMSG, SUB_ALIAS,
#ifdef RESTORE
        SUB_RESTOREVMSG, SUB_RESTORERMSG, SUB_RESTORECMSG,
#endif
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
        char     *host;
        char     *hostip;
        int descriptor;
        sh_int connected;
        sh_int idle;
        sh_int lines;
        sh_int scrlen;
        bool fcommand;
        char inbuf[MAX_INBUF_SIZE];
        char incomm[MAX_INPUT_LENGTH];
        char inlast[MAX_INPUT_LENGTH];
        int repeat;
        char     *outbuf;
        unsigned long outsize;
        int outtop;
        char     *pagebuf;
        long pagesize;
        int pagetop;
        char     *pagepoint;
        char pagecmd;
        char pagecolor;
        int auth_state;
        int atimes;
        int newstate;
        unsigned char prevcolor;
        pid_t process;  /* Samson 4-16-98 - For new command shell code */
        sh_int speed;   /* descriptor speed settings */
        char     *client;
#ifdef MCCP
        unsigned char compressing;
        z_stream *out_compress;
        unsigned char *out_compress_buf;
        unsigned char shellcompressing;
#endif
        bool mxp_detected;  /* player using MXP flag */
        bool msp_detected;  /* player using MSP flag */
#ifdef ACCOUNT
        struct account_data *account;
#endif
        int ifd;
        pid_t ipid;
};

struct web_descriptor
{
        int fd;
        char request[2048];
        struct sockaddr_in *their_addr;
        int sin_size;
        WEB_DESCRIPTOR *next;
        WEB_DESCRIPTOR *prev;
        bool valid;
};

/*
 * Attribute bonus structures.
 */
struct str_app_type
{
        sh_int tohit;
        sh_int todam;
        sh_int carry;
        sh_int wield;
};

struct int_app_type
{
        sh_int learn;
};

struct wis_app_type
{
        sh_int practice;
};

struct dex_app_type
{
        sh_int defensive;
};

struct con_app_type
{
        sh_int hitp;
        sh_int shock;
};

struct cha_app_type
{
        sh_int charm;
};

struct lck_app_type
{
        sh_int luck;
};

struct frc_app_type
{
        sh_int force;
};

/* ability classes */

#define ABILITY_NONE		-1
#define COMBAT_ABILITY 		0
#define PILOTING_ABILITY	1
#define ENGINEERING_ABILITY	2
#define HUNTING_ABILITY		3
#define SMUGGLING_ABILITY	4
#define DIPLOMACY_ABILITY	5
#define LEADERSHIP_ABILITY	6
#define FORCE_ABILITY		7
#define OCCUPATION_ABILITY	8
#define PIRACY_ABILITY	9
#define MEDIC_ABILITY	10


/* the races */
#define RACE_HUMAN	    	0
#define RACE_WOOKIEE		1
#define RACE_TWI_LEK		2
#define RACE_RODIAN		3
#define RACE_HUTT		4
#define RACE_MON_CALAMARI	5
#define RACE_NOGHRI		6
#define RACE_GAMORREAN		7
#define RACE_JAWA		8
#define RACE_ADARIAN            9
#define RACE_EWOK              	10
#define RACE_VERPINE           	11
#define RACE_DEFEL             	12
#define RACE_TRANDOSHAN        	13
#define RACE_SHISTAVANAN       	14
#define RACE_DUINUOGWUIN       	16
#define RACE_DROID           	15
#define RACE_CHISS           	17


/*
 * Languages -- Altrag
 */
#define LANG_BASIC        BV00  /* Human base language */
#define LANG_WOOKIEE      BV01
#define LANG_TWI_LEK      BV02
#define LANG_RODIAN       BV03
#define LANG_HUTT         BV04
#define LANG_MON_CALAMARI BV05
#define LANG_NOGHRI       BV06
#define LANG_EWOK         BV07
#define LANG_ITHORIAN     BV08
#define LANG_DEVARONIAN   BV09
#define LANG_GAMORREAN    BV10
#define LANG_JAWA         BV11
#define LANG_CLAN	      BV12
#define LANG_ADARIAN	  BV13
#define LANG_VERPINE	  BV14
#define LANG_DEFEL        BV15
#define LANG_TRANDOSHAN   BV16
#define LANG_SHISTAVANAN  BV17
#define LANG_BINARY       BV18
#define LANG_DUINUOGWUIN  BV19
#define LANG_CSILLIAN     BV20
#define LANG_KEL_DOR      BV21
#define LANG_BOTHAN       BV22
#define LANG_BARABEL      BV23
#define LANG_DUROSIAN     BV24
#define LANG_GOTAL        BV25
#define LANG_TALZ         BV26
#define LANG_HO_DIN       BV27
#define LANG_FALLEEN      BV28
#define LANG_GIVIN        BV29
#define LANG_UNKNOWN        0   /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_BASIC | LANG_WOOKIEE | LANG_TWI_LEK | LANG_RODIAN  \
		       | LANG_HUTT | LANG_MON_CALAMARI | LANG_NOGHRI | LANG_GAMORREAN \
		       | LANG_JAWA | LANG_ADARIAN | LANG_EWOK | LANG_VERPINE | LANG_DEFEL \
		       | LANG_TRANDOSHAN | LANG_SHISTAVANAN | LANG_BINARY | LANG_DUINUOGWUIN \
			   | LANG_CSILLIAN| LANG_KEL_DOR | LANG_BOTHAN | LANG_BARABEL | LANG_ITHORIAN \
			   | LANG_DEVARONIAN | LANG_DUROSIAN | LANG_GOTAL | LANG_TALZ | LANG_HO_DIN	\
			   | LANG_FALLEEN | LANG_GIVIN)
/*  26 Languages */

/*
 * TO types for act.
 */

/* if > IC_TO && < OOC_TO */
#define IS_IC_ACT(act) (act > IC_TO && act < OOC_TO)
#define IS_OOC_ACT(act) (act > OOC_TO)
#define IS_CONTRABAND(cargo) ((cargo) > CONTRABAND_NONE && (cargo) < CONTRABAND_MAX)
typedef enum
{
        IC_TO, TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_MUD,
        OOC_TO, TO_ROOM_OOC, TO_NOTVICT_OOC, TO_VICT_OOC, TO_CHAR_OOC,
        TO_MUD_OOC
} ACT_TO_TYPES;

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/*
 * Help table types.
 */
struct help_data
{
        HELP_DATA *next;
        HELP_DATA *prev;
        sh_int level;
        char     *keyword;
        char     *text;
        char     *author;
        char     *date;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct shop_data
{
        SHOP_DATA *next;    /* Next shop in list        */
        SHOP_DATA *prev;    /* Previous shop in list    */
        int keeper; /* Vnum of shop keeper mob  */
        sh_int buy_type[MAX_TRADE]; /* Item types shop will buy */
        sh_int profit_buy;  /* Cost multiplier for buying   */
        sh_int profit_sell; /* Cost multiplier for selling  */
        sh_int open_hour;   /* First opening hour       */
        sh_int close_hour;  /* First closing hour       */
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct repairshop_data
{
        REPAIR_DATA *next;  /* Next shop in list        */
        REPAIR_DATA *prev;  /* Previous shop in list    */
        int keeper; /* Vnum of shop keeper mob  */
        sh_int fix_type[MAX_FIX];   /* Item types shop will fix */
        sh_int profit_fix;  /* Cost multiplier for fixing   */
        sh_int shop_type;   /* Repair shop type     */
        sh_int open_hour;   /* First opening hour       */
        sh_int close_hour;  /* First closing hour       */
};


/* Mob program structures */
/* Mob program structures and defines */
/* Moved these defines here from mud_prog.c as I need them -rkb */
#define MAX_IFS 20  /* should always be generous */
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

struct act_prog_data
{
        struct act_prog_data *next;
        void     *vo;
};

struct mob_prog_act_list
{
        MPROG_ACT_LIST *next;
        char     *buf;
        CHAR_DATA *ch;
        OBJ_DATA *obj;
        void     *vo;
};

struct mob_prog_data
{
        MPROG_DATA *next;
        int type;
        bool triggered;
        int resetdelay;
        char     *arglist;
        char     *comlist;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum
{ MP_MOB, MP_ROOM, MP_OBJ } mp_types;
struct mpsleep_data
{
        MPSLEEP_DATA *next;
        MPSLEEP_DATA *prev;

        int timer;  /* Pulses to sleep */
        mp_types type;  /* Mob, Room or Obj prog */
        ROOM_INDEX_DATA *room;  /* Room when type is MP_ROOM */

        /*
         * mprog_driver state variables 
         */
        int ignorelevel;
        int iflevel;
        bool ifstate[MAX_IFS][DO_ELSE + 1];

        /*
         * mprog_driver arguments 
         */
        char     *com_list;
        CHAR_DATA *mob;
        CHAR_DATA *actor;
        OBJ_DATA *obj;
        void     *vo;
        bool single_step;
};


extern bool MOBtrigger;

/* race dedicated stuff*/
struct race_type
{
        char race_name[16];
        int affected;
        sh_int str_plus;
        sh_int dex_plus;
        sh_int wis_plus;
        sh_int int_plus;
        sh_int con_plus;
        sh_int cha_plus;
        sh_int lck_plus;
        sh_int frc_plus;
        sh_int hit;
        sh_int endurance;
        sh_int resist;
        sh_int suscept;
        int class_restriction;
        int language;
};

struct log_data
{
        char     *name;
        char     *message;
        time_t time;
        LANGUAGE_DATA *language;
};

typedef enum
{
        CLAN_PLAIN, CLAN_CRIME, CLAN_GUILD, CLAN_SUBCLAN
} clan_types;

#define HAS_CLAN_PERM(ch, clan, permission) \
		( (ch) && (ch)->pcdata && (clan) && (ch)->pcdata->bestowments && \
		 ( !str_cmp((ch)->name, (clan)->leader) || \
		   !str_cmp((ch)->name, (clan)->number1) || \
		   !str_cmp((ch)->name, (clan)->number2) ||  \
		   is_name((permission), (ch)->pcdata->bestowments)  \
		 ) \
		)

typedef enum
{ SHIP_CIVILIAN, SHIP_REPUBLIC, SHIP_IMPERIAL, MOB_SHIP, PLAYER_SHIP,
        CLAN_MOB_SHIP
} ship_types;
typedef enum
{ SHIP_DOCKED, SHIP_READY, SHIP_BUSY, SHIP_BUSY_2, SHIP_BUSY_3, SHIP_REFUEL,
        SHIP_LAUNCH, SHIP_LAUNCH_2, SHIP_LAND, SHIP_LAND_2, SHIP_HYPERSPACE,
        SHIP_DISABLED, SHIP_FLYING
} ship_states;
typedef enum
{ MISSILE_READY, MISSILE_FIRED, MISSILE_RELOAD, MISSILE_RELOAD_2,
        MISSILE_DAMAGED
} missile_states;
typedef enum
{ FIGHTER_SHIP, MIDSIZE_SHIP, CAPITAL_SHIP, SHIP_PLATFORM } ship_classes;
typedef enum
{ CONCUSSION_MISSILE, PROTON_TORPEDO, HEAVY_ROCKET,
        HEAVY_BOMB
} missile_types;

typedef enum
{ GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD } group_types;

#define LASER_DAMAGED    -1
#define LASER_READY       0

struct DOCK_DATA;
struct space_data
{
        space_data();
        ~space_data();
        SPACE_DATA *next;
        SPACE_DATA *prev;
        SHIP_DATA *first_ship;
        SHIP_DATA *last_ship;
        MISSILE_DATA *first_missile;
        MISSILE_DATA *last_missile;
        PLANET_DATA *first_planet;
        PLANET_DATA *last_planet;
        DOCK_DATA *first_dock;
        DOCK_DATA *last_dock;
        BODY_LIST bodies;
        char     *filename;
        char     *name;
        int xpos;
        int ypos;

};

/* cargo types */
typedef enum
{
        CARGO_NONE, CARGO_ORE, CARGO_PRODUCE, CARGO_MEAT, CARGO_METAL,
        CARGO_MINERALS, CARGO_COMPONENTS, CARGO_FUEL_CELLS, CARGO_TABANNA,
        CARGO_CULTURED, CARGO_PROCESSED, CARGO_DURACRETE, CARGO_DURASTEEL,
        CARGO_ELECTRONICS, CARGO_SERIND, CARGO_TETRALI, CARGO_COMPUTERS,
        CARGO_MONOTHELENE, CARGO_ATOMIC_CELLS, CARGO_BACTA,
        CARGO_TRANSPARISTEEL,
        CARGO_DROID_PARTS, CARGO_ARMOUR, CARGO_DELICACIES, CARGO_ALAZHI,
        CARGO_MAX,
        CONTRABAND_NONE, CONTRABAND_BLASTERS, CONTRABAND_SPICE,
        CONTRABAND_SLAVES,
        CONTRABAND_EXPLOSIVES, CONTRABAND_MAX
} cargo_types;

/* disease types */
#define ILLNESS_NONE            0
#define ILLNESS_SNIFFLES        1
#define ILLNESS_GAS             2
#define ILLNESS_COUGH           3
#define ILLNESS_COLD            4
#define ILLNESS_FLU             5
#define ILLNESS_STREP           6
#define ILLNESS_PNUMONIA        7
#define ILLNESS_INFECTEYE       8
#define ILLNESS_INFECTEAR       9
#define ILLNESS_INFECTTHROAT    10
#define ILLNESS_BACHULARIA      11
#define ILLNESS_ANGINA          12
#define ILLNESS_DEPRESSION      13
#define ILLNESS_CANCER          14
#define ILLNESS_BUBONIC_PLAGUE  15
#define ILLNESS_AIDS            16
#define ILLNESS_MAX             17


struct INSTALLATION_DATA;
struct planet_data
{
        PLANET_DATA *next;
        PLANET_DATA *prev;
        PLANET_DATA *next_in_system;
        PLANET_DATA *prev_in_system;
        INSTALLATION_DATA *first_install;
        INSTALLATION_DATA *last_install;
        SPACE_DATA *starsystem;
        AREA_DATA *first_area;
        AREA_DATA *last_area;
        BODY_DATA *body;
        char     *bodyname;
        char     *name;
        char     *filename;
        long base_value;
        CLAN_DATA *governed_by;
        int population;
        int turbolasers;
        int ioncannons;
        int shields;
        int budgetpolice;
        int budgetenter;
        int budgetmaintenance;
        int budgeteducation;
        int budgetantiesp;
        int flags;
        int pop_support;
        int cargoimport[CONTRABAND_MAX];
        int cargoexport[CONTRABAND_MAX];
        int resource[CONTRABAND_MAX];
        int consumes[CONTRABAND_MAX];
        int produces[CONTRABAND_MAX];
        int defbattalions;
        int attbattalions;
        CLAN_DATA *attgovern;
        int planet_type;
        int jail;
};


#define PLANET_NOCAPTURE  BV00
#define PLANET_SHIELD     BV01

/* NEVER USE MAGIC NUMBERS */
/* Pretty sure if we are doing 0-13, this can be 13 */
#define MAX_RANK 14

struct clan_data
{
        CLAN_DATA *next;    /* next clan in list            */
        CLAN_DATA *prev;    /* previous clan in list        */
        CLAN_LIST subclans;
        CLAN_DATA *next_subclan;
        CLAN_DATA *prev_subclan;
        CLAN_DATA *first_subclan;
        CLAN_DATA *last_subclan;
        CLAN_DATA *mainclan;
        char     *filename; /* Clan filename            */
        char     *name; /* Clan name                */
        char     *description;  /* A brief description of the clan  */
        char     *motto;    /* Clan Motto           */
        char     *leader;   /* Head clan leader         */
        char     *number1;  /* First officer            */
        char     *number2;  /* Second officer           */
        char     *enemy_name;   /* Enemy            */
        char     *ally_name;    /* Ally         */
        char     *roster;   /* A list of all members of that clan */
        CLAN_DATA *enemy;   /* Enemy            */
        CLAN_DATA *ally;    /* Ally         */
        int pkills; /* Number of pkills on behalf of clan   */
        int pdeaths;    /* Number of pkills against clan    */
        int mkills; /* Number of mkills on behalf of clan   */
        int mdeaths;    /* Number of clan deaths due to mobs    */
        sh_int clan_type;   /* See clan type defines        */
        sh_int members; /* Number of clan members       */
        int enlistroom; /* VNUM of the enlistment room */
        int board;  /* Vnum of clan board           */
        int storeroom;  /* Vnum of clan's store room        */
        int alignment;  /* Alignment requirement for enlistment */
        int enliston;   /* Enlist   */
        long int funds;
        int spacecraft;
        int jail;
        char     *tmpstr;
        char     *rank[MAX_RANK];
        int       salary[MAX_RANK];
		/*       rank.allowed should be a bitset
		 *       could contain flags for different commands
		 *       as well as maybe different areas
		 */
};

struct ship_data
{
        SHIP_DATA *next;
        SHIP_DATA *prev;
        /*
         * Next in body ? 
         */
        SHIP_DATA *next_in_starsystem;
        SHIP_DATA *prev_in_starsystem;
        SHIP_DATA *next_in_room;
        SHIP_DATA *prev_in_room;
        ROOM_INDEX_DATA *in_room;
        SPACE_DATA *starsystem;
        PROTOSHIP_DATA *prototype;
        SHIP_DATA *dockedto;
        SHIP_DATA *tractorby;
        SHIP_DATA *target0;
        SHIP_DATA *target1;
        SHIP_DATA *target2;
        SPACE_DATA *currjump;
        CLAN_DATA *clan;
		char     *filename;
        char     *name;
        char     *home;
        char     *description;
        char     *owner;
        char     *pilot;
        char     *copilot;
        char     *dest;
        char     *selfdestruct;
		int type;
        int ship_class;
        int comm;
        int sensor;
        int astro_array;
        int hyperspeed;
        int hyperdistance;
        int realspeed;
        int currspeed;
        int shipstate;
        int statet0;
        int statet1;
        int statet2;
        int statet0i;
        int missiletype;
        int missilestate;
        int missiles;
        int maxmissiles;
        int torpedos;
        int maxtorpedos;
        int rockets;
        int maxrockets;
        int lasers;
        int tractorbeam;
        int ions;
        int manuever;
        int selfdpass;
        int maxcargo;
        int cargo;
        int cargotype;
        bool bayopen;
        bool hatchopen;
        bool autorecharge;
        bool autotrack;
        bool autospeed;
        int stealth;
        int cloak;
        int interdictor;
        float vx, vy, vz;
        float hx, hy, hz;
        float jx, jy, jz;
        int maxenergy;
        int energy;
        int shield;
        int maxshield;
        int hull;
        int maxhull;
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
        int evasive;
        int gunseat;
        long collision;
        int chaff;
        int maxchaff;
        bool chaff_released;
        bool autopilot;
        int flags;
        int sim_vnum;
        int battalions;
        int maxbattalions;
		int lastbuilt;
        int bombs;
        int maxbombs;
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
        char     *fired_by;
        sh_int missiletype;
        sh_int age;
        int speed;
        int mx, my, mz;
};


/*
 * An affect.
 */
struct affect_data
{
        AFFECT_DATA *next;
        AFFECT_DATA *prev;
        sh_int type;
        int duration;
        sh_int location;
        int modifier;
        int bitvector;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
        SMAUG_AFF *next;
        char     *duration;
        sh_int location;
        char     *modifier;
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


#define MOB_VNUM_GUARD	21
#define MOB_VNUM_PATROL	23
#define MOB_VNUM_ELITE_GUARD	18
#define MOB_VNUM_SPECIAL_FORCES  	19
#define MOB_VNUM_PATROL	23
#define MOB_VNUM_MERCINARY	24
#define MOB_VNUM_BOUNCER	25


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		 BV00   /* Auto set for mobs    */
#define ACT_SENTINEL		 BV01   /* Stays in one room    */
#define ACT_SCAVENGER		 BV02   /* Picks up objects */
#define ACT_NORUNSNIPE       BV03   /* Won't run and snipe */
#define ACT_AGGRESSIVE		 BV05   /* Attacks PC's     */
#define ACT_STAY_AREA		 BV06   /* Won't leave area */
#define ACT_WIMPY		 BV07   /* Flees when hurt  */
#define ACT_PET			 BV08   /* Auto set for pets    */
#define ACT_TRAIN		 BV09   /* Can train PC's   */
#define ACT_PRACTICE		 BV10   /* Can practice PC's    */
#define ACT_IMMORTAL		 BV11   /* Cannot be killed */
#define ACT_DEADLY		 BV12   /* Has a deadly poison  */
#define ACT_SPEAKSALL		 BV13
#define ACT_META_AGGR		 BV14   /* Extremely aggressive */
#define ACT_GUARDIAN		 BV15   /* Protects master  */
#define ACT_RUNNING		 BV16   /* Hunts quickly    */
#define ACT_NOWANDER		 BV17   /* Doesn't wander   */
#define ACT_MOUNTABLE		 BV18   /* Can be mounted   */
#define ACT_MOUNTED		 BV19   /* Is mounted       */
#define ACT_SCHOLAR              BV20   /* Can teach languages  */
#define ACT_SECRETIVE		 BV21   /* actions aren't seen  */
#define ACT_POLYMORPHED		 BV22   /* Mob is a ch      */
#define ACT_MOBINVIS		 BV23   /* Like wizinvis    */
#define ACT_NOASSIST		 BV24   /* Doesn't assist mobs  */
#define ACT_NOKILL               BV25   /* Mob can't die */
#define ACT_DROID                BV26   /* mob is a droid */
#define ACT_NOCORPSE             BV27
#define ACT_MAIL			 BV28   /* Mail */
#define ACT_CITIZEN		 BV29   /* Planet Citizen */
#define ACT_PROTOTYPE		 BV30   /* A prototype mob  */
/* 20 acts */

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
#define VIP_BYSS		        BV10
#define VIP_ENDOR		        BV11
#define VIP_ROCHE		        BV12
#define VIP_AF_EL		        BV13
#define VIP_TRANDOSHA	 	    BV14
#define VIP_CHAD		        BV15
#define VIP_HOTH		        BV16

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
#define WANTED_TRANDOSHA		BV14
#define WANTED_CHAD		BV15
#define WANTED_HOTH		BV16

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
#define AFF_FAERIE_FIRE		  BV08
#define AFF_INFRARED		  BV09
#define AFF_CURSE		  BV10
#define AFF_SECRETIVE		  BV11  /* Unused   */
#define AFF_POISON		  BV12
#define AFF_PROTECT		  BV13
#define AFF_PARALYSIS		  BV14
#define AFF_SNEAK		  BV15
#define AFF_HIDE		  BV16
#define AFF_SLEEP		  BV17
#define AFF_CHARM		  BV18
#define AFF_FLYING		  BV19
#define AFF_PASS_DOOR		  BV20
#define AFF_FLOATING		  BV21
#define AFF_TRUESIGHT		  BV22
#define AFF_DETECTTRAPS		  BV23
#define AFF_SCRYING	          BV24
#define AFF_FIRESHIELD	          BV25
#define AFF_SHOCKSHIELD	          BV26
#define AFF_RESTRAINED                 BV27 /* not used */
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
/* 21 RIS's*/

/* 
 * Attack types
 */

#define ATCK_BITE          BV00
#define ATCK_CLAWS      BV01
#define ATCK_TAIL          BV02
#define ATCK_STING      BV03
#define ATCK_PUNCH      BV04
#define ATCK_KICK          BV05
#define ATCK_TRIP          BV06
#define ATCK_BACKSTAB      BV10


/*
 * Defense types
 */
#define DFND_PARRY      BV00
#define DFND_DODGE      BV01
#define DFND_DISARM      BV19
#define DFND_GRIP          BV21
/* 2 def's */

/*
 * Body parts
 */
typedef enum
{
        PART_HEAD, PART_ARMS, PART_LEGS, PART_HEART, PART_BRAINS,
        PART_GUTS, PART_HANGS, PART_FEET, PART_FINGERS, PART_EAR,
        PART_EYE, PART_LONG_TONGUE, PART_EYESTALKS, PART_TENTACLES, PART_FINS,
        PART_WINGS, PART_TAIL, PART_SCALES, PART_CLAWS, PART_FANGS,
        PART_HORNS, PART_TUSKS, PART_TAILATTACK, PART_SHARPSCALES, PART_BEAK,
        PART_HAUNCH, PART_HOOVES, PART_FORELEGS, PART_FEATHERS
} body_part;


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
#define SV_WHO		  	  BV13
#define SV_SCORE		  BV14
#define SV_LIST		  	  BV15
#define SV_N		  	  BV16
#define SV_E			  BV17
#define SV_S			  BV18
#define SV_W			  BV19
#define SV_NE		  	  BV20
#define SV_SE			  BV21
#define SV_NW			  BV22
#define SV_SW			  BV23
#define SV_LOOK			  BV24

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
#define SF_WATER		  BV11
#define SF_EARTH		  BV12
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
{ SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
        SS_BREATH, SS_SPELL_STAFF
} save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum
{ SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
        SD_POISON, SD_DRAIN
} spell_dam_types;

typedef enum
{ SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
        SA_DIVINATE, SA_OBSCURE, SA_CHANGE
} spell_act_types;

typedef enum
{ SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum
{ SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
        SC_LIFE, SC_DEATH, SC_ILLUSION
} spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{ SEX_NEUTRAL, SEX_MALE, SEX_FEMALE, SEX_MAX } sex_types;

typedef enum
{
        TRAP_TYPE_POISON_GAS =
                1, TRAP_TYPE_POISON_DART, TRAP_TYPE_POISON_NEEDLE,
        TRAP_TYPE_POISON_DAGGER, TRAP_TYPE_POISON_ARROW,
        TRAP_TYPE_BLINDNESS_GAS,
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
#define OBJ_VNUM_SHARPEN             39

#define OBJ_VNUM_DIAMOND_RING	     65
#define OBJ_VNUM_RESTRAINT           66
#define OBJ_VNUM_WEDDING_BAND        67
#define OBJ_VNUM_MEDKIT              67

/* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE	  10315
#define OBJ_VNUM_SCHOOL_DAGGER	  10312
#define OBJ_VNUM_SCHOOL_SWORD	  10313
#define OBJ_VNUM_SCHOOL_VEST	  10308
#define OBJ_VNUM_SCHOOL_SHIELD	  10310
#define OBJ_VNUM_SCHOOL_BANNER    10311
#define OBJ_VNUM_SCHOOL_DIPLOMA   10321
#define OBJ_VNUM_SCHOOL_MONEY     10431

#define OBJ_VNUM_BLASTECH_E11     50

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
        ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF,
        ITEM_WEAPON,
        ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
        ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
        ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
        ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
        ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
        ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH,
        ITEM_LEVER,
        ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_BEACON, ITEM_TRAP, ITEM_RUNEPOUCH,
        ITEM_MATCH, ITEM_RAW_METAL, ITEM_CANISTER, ITEM_PORTAL, ITEM_PAPER,
        ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL,
        ITEM_FUEL,
        ITEM_RESTRAINT, ITEM_LONG_BOW, ITEM_CROSSBOW, ITEM_AMMO, ITEM_QUIVER,
        ITEM_SHOVEL, ITEM_SALVE, ITEM_RAWSPICE, ITEM_LENS, ITEM_CRYSTAL,
        ITEM_DURAPLAST,
        ITEM_BATTERY, ITEM_TOOLKIT, ITEM_DURASTEEL, ITEM_OVEN, ITEM_MIRROR,
        ITEM_CIRCUIT, ITEM_SUPERCONDUCTOR, ITEM_COMLINK, ITEM_MEDPAC,
        ITEM_FABRIC,
        ITEM_RARE_METAL, ITEM_MAGNET, ITEM_THREAD, ITEM_SPICE, ITEM_SMUT,
        ITEM_DEVICE, ITEM_SPACECRAFT,
        ITEM_GRENADE, ITEM_LANDMINE, ITEM_GOVERNMENT, ITEM_DROID_CORPSE,
        ITEM_BOLT, ITEM_BOND,
        ITEM_IMPLANT, ITEM_CHEMICAL, ITEM_BINDING, ITEM_HOLSTER, 
        ITEM_LANDDEED,
        ITEM_MAX
} item_types;


#define MAX_ITEM_TYPE		     (ITEM_MAX-1)
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		BV00
#define ITEM_HUM		BV01
#define ITEM_DARK		BV02
#define ITEM_HUTT_SIZE		BV03
#define ITEM_CONTRABAND		BV04
#define ITEM_INVIS		BV05
#define ITEM_MAGIC		BV06
#define ITEM_NODROP		BV07
#define ITEM_BLESS		BV08
#define ITEM_ANTI_GOOD		BV09
#define ITEM_ANTI_EVIL		BV10
#define ITEM_ANTI_NEUTRAL	BV11
#define ITEM_NOREMOVE		BV12
#define ITEM_INVENTORY		BV13
#define ITEM_ANTI_SOLDIER	BV14
#define ITEM_ANTI_THIEF	        BV15
#define ITEM_ANTI_HUNTER	BV16
#define ITEM_ANTI_JEDI  	BV17
#define ITEM_SMALL_SIZE		BV18
#define ITEM_LARGE_SIZE		BV19
#define ITEM_DONATION		BV20
#define ITEM_CLANOBJECT		BV21
#define ITEM_ANTI_CITIZEN	BV22
#define ITEM_ANTI_SITH  	BV23
#define ITEM_ANTI_PILOT	        BV24
#define ITEM_HIDDEN		BV25
#define ITEM_POISONED		BV26
#define ITEM_COVERING		BV27
#define ITEM_DEATHROT		BV28
#define ITEM_BURRIED		BV29    /* item is underground */
#define ITEM_PROTOTYPE		BV30
#define ITEM_HUMAN_SIZE         BV31

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
#define WEAPON_WHIP  		4
#define WEAPON_KNIFE		5
#define WEAPON_BLASTER		6
#define WEAPON_BLUDGEON		8
#define WEAPON_BOWCASTER        9
#define WEAPON_FORCE_PIKE	11


/* Furniture Settings */
#define STAND_AT                1
#define STAND_ON                2
#define STAND_IN                3
#define SIT_AT                  1
#define SIT_ON                  2
#define SIT_IN                  3
#define REST_AT                 1
#define REST_ON                 2
#define REST_IN                 3
#define SLEEP_AT                1
#define SLEEP_ON                2
#define SLEEP_IN                3
#define PUT_AT                  1
#define PUT_ON                  2
#define PUT_IN                  3
#define PUT_INSIDE              4


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
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26    /* Unused */
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

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
#define ITEM_WEAR_BINDING       BV19
#define ITEM_WEAR_HOLSTER1	BV20

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
        APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
        APPLY_SEX, APPLY_NULL, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT,
        APPLY_WEIGHT,
        APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
        APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
        APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
        APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
        APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
        APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP,
        APPLY_DODGE,
        APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT,
        APPLY_DISARM,
        APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH,
        APPLY_CLIMB,
        APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL,
        APPLY_REMOVESPELL,
        APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE,
        APPLY_DIG,
        APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_SECRETIVE,
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
#define ROOM_CLONE_END	      10000
#define ROOM_CLONE_BEGIN	  10001
#define ROOM_VNUM_CHAT		  32144
#define ROOM_VNUM_TEMPLE	  32144
#define ROOM_VNUM_ALTAR		  32144
#define ROOM_VNUM_SCHOOL	  227
#define ROOM_AUTH_START		  227
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
#define ROOM_START_CSILLIAN       32148
#define ROOM_START_DEFEL          32148
#define ROOM_START_TRANDOSHAN     32148
#define ROOM_START_SHISTAVANAN     32148
#define ROOM_START_DUINUOGWUIN    32148
#define ROOM_START_DROID        21069
#define ROOM_START_IMMORTAL         100
#define ROOM_LIMBO_SHIPYARD          45
#define ROOM_DEFAULT_CRASH        28025

#define ROOM_PLUOGUS_QUIT         32148

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 * Leave ROOM_RESERVED alone, its reserved for tracking.
 */
typedef enum
{
        ROOM_DARK, ROOM_RESERVED, ROOM_NO_MOB, ROOM_INDOORS, ROOM_CAN_LAND,
        ROOM_CAN_FLY, ROOM_NO_DRIVING, ROOM_NO_MAGIC, ROOM_BANK,
        ROOM_PRIVATE, ROOM_SAFE, ROOM_INSTALLATION, ROOM_PET_SHOP,
        ROOM_NO_HAIL_TO, ROOM_DONATION, ROOM_NODROPALL,
        ROOM_SILENCE, ROOM_LOGSPEECH, ROOM_NODROP, ROOM_CLANSTOREROOM,
        ROOM_PLR_HOME, ROOM_EMPTY_HOME, ROOM_NOTHING, ROOM_HOTEL,
        ROOM_NOFLOOR, ROOM_REFINERY, ROOM_FACTORY, ROOM_RECRUIT,
        ROOM_E_RECRUIT,
        ROOM_SPACECRAFT, ROOM_PROTOTYPE, ROOM_AUCTION, ROOM_BAR, ROOM_INN,
        ROOM_OFFICE, ROOM_CAFE, ROOM_KITCHEN, ROOM_EXECUTIVE, ROOM_BOARDROOM,
        ROOM_BACTA, ROOM_IMPORT, ROOM_BACTA_CHARGE, ROOM_ARENA, ROOM_BOUNTY,
        ROOM_TEMP_COCKPIT, ROOM_TEMP_TURRET1, ROOM_TEMP_TURRET2,
        ROOM_TEMP_HANGAR,
        ROOM_TEMP_ENGINEROOM, ROOM_TEMP_NAVSEAT, ROOM_TEMP_PILOTSEAT,
        ROOM_TEMP_COSEAT,
        ROOM_TEMP_GUNSEAT, ROOM_TEMP_CORRIDOR, ROOM_TEMP_BEDROOM,
        ROOM_TEMP_WORKSHOP,
        ROOM_TEMP_TURBOLIFT, ROOM_PLAYERSHOP, ROOM_MONITOR,
        ROOM_EMPTYPLOT, MAX_ROOM_FLAG
} roomflags;

/*Ship Flags*/
#define SHIP_SIMULATOR		BV00
#define SHIP_CLOAK          BV01
#define SHIP_STEALTH        BV02
#define SHIP_INTERDICTOR    BV03
#define SHIP_REPUBLIC       BV04
#define SHIP_EMPIRE         BV05
#define SHIP_HUNTER         BV06
#define SHIP_SMUGGLER       BV07
#define SHIP_PIRATE         BV08
#define SHIP_POLICE         BV09
#define SHIP_CLAN1          BV10
#define SHIP_CLAN2          BV11
#define SHIP_CLAN3          BV12
#define SHIP_CLAN4          BV13
#define SHIP_CLAN5          BV14
#define SHIP_TROOP          BV15
#define SHIP_TAXI           BV16



/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
        DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
        DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST,
        DIR_SOMEWHERE
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
#define EX_RUBBLE      BV27
#define MAX_EXFLAG		  27

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
        SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS,
        SECT_MOUNTAIN,
        SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR,
        SECT_DESERT,
        SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_SPACECRAFT,
        SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
        WEAR_NONE = -1, WEAR_LIGHT =
                0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
        WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
        WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L,
        WEAR_WRIST_R,
        WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
        WEAR_MISSILE_WIELD, WEAR_BINDING, WEAR_HOLSTER_L, WEAR_HOLSTER_R,
        MAX_WEAR
} wear_locations;

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
        POS_RESTING,
        POS_SITTING, POS_FIGHTING, POS_STANDING, POS_MOUNTED, POS_SHOVE,
        POS_DRAG,
        POS_MAX
} positions;

/*
 * ACT bits for players.
 */


#define PLR_IS_NPC		      BV00  /* Don't EVER set.  */
#define PLR_BOUGHT_PET		      BV01
#define PLR_SHOVEDRAG		      BV02
#define PLR_AUTOEXIT		      BV03
#define PLR_AUTOLOOT		      BV04
#define PLR_FORSAKEN                  BV05
#define PLR_BLANK		      BV06
#define PLR_SECRETIVE 		      BV07
#define PLR_BRIEF		      BV08
#define PLR_COMBINE		      BV09
#define PLR_PROMPT		      BV10
#define PLR_TELNET_GA		      BV11
#define PLR_HOLYLIGHT		   BV12
#define PLR_WIZINVIS		   BV13
#define PLR_ROOMVNUM		   BV14
#define	PLR_SILENCE		   BV15
#define PLR_NO_EMOTE		   BV16
#define PLR_ATTACKER    	   BV17
#define PLR_NO_TELL		   BV18
#define PLR_LOG			   BV19
#define PLR_DENY		   BV20
#define PLR_FREEZE		   BV21
#define PLR_KILLER    	           BV22
#define PLR_QUESTOR  	           BV23
#define PLR_LITTERBUG	           BV24
#define PLR_ANSI	           BV25
#define PLR_SOUND	           BV26
#define PLR_NICE	           BV27
#define PLR_FLEE	           BV28
#define PLR_AUTOGOLD               BV29
#define PLR_MXP		           BV30
#define PLR_AFK                    BV31

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_NOHUNGER	    	   BV01
#define PCFLAG_UNAUTHED	    	   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		           BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		       BV08
#define PCFLAG_PAGERON		       BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_ROOM                BV11
#define PCFLAG_WORKING             BV12
#define PCFLAG_GOTMAIL             BV13
#define PCFLAG_EXEMPT              BV14
#define PCFLAG_FASTENGINEER        BV15
#define PCFLAG_WATCH               BV16 /* see function "do_watch" */
#define PCFLAG_MARRIED             BV17
#define PCFLAG_NEWBGUIDE           BV18
#define PCFLAG_AUTODRAW            BV19

/* Bits for ch->pcdata->godflags */
#define IMM_ADMIN                  BV00
#define IMM_BUILDER                BV01
#define IMM_HIGHBUILDER            BV02
#define IMM_ENFORCER               BV03
#define IMM_QUEST                  BV04
#define IMM_CODER                  BV05
#define IMM_ALL                    BV06
#define IMM_OWNER                  BV07
#define IMM_HIGHENFORCER           BV08

/* Bits for command->flags */
#define COMMAND_ADMIN                  BV00
#define COMMAND_BUILDER                BV01
#define COMMAND_HIGHBUILDER            BV02
#define COMMAND_ENFORCER               BV03
#define COMMAND_QUEST                  BV04
#define COMMAND_CODER                  BV05
#define COMMAND_ALL                    BV06
#define COMMAND_OWNER                  BV07
#define COMMAND_HIGHENFORCER           BV08



typedef enum
{
        TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
        TIMER_APPLIED, TIMER_PKILLED
} timer_types;

struct timer_data
{
        TIMER    *prev;
        TIMER    *next;
        DO_FUN   *do_fun;
        int value;
        sh_int type;
        sh_int count;
};


/*
 * Channel bits.
 */
typedef enum
{
        CHANNEL_LOG, CHANNEL_BUILD, CHANNEL_COMM, CHANNEL_TELLS,
        CHANNEL_AUCTION
} channels;

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_NOQUEST               BV01
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
        RACE_DATA *race;
        int progtypes;
        char     *player_name;
        char     *short_descr;
        char     *long_descr;
        char     *description;
        char     *spec_funname;
        char     *spec2_funname;
        char     *mob_clan;
        int vnum;
        int max_hit;
        sh_int count;
        sh_int killed;
        sh_int sex;
        sh_int level;
        int act;
        int affected_by;
        sh_int alignment;
        sh_int mobthac0;    /* Unused */
        sh_int ac;
        sh_int hitnodice;
        sh_int hitsizedice;
        sh_int hitplus;
        sh_int damnodice;
        sh_int damsizedice;
        sh_int damplus;
        sh_int numattacks;
        unsigned int gold;
        int exp;
        EXT_BV xflags;
        int resistant;
        int immune;
        int susceptible;
        int attacks;
        int defenses;
        LANGUAGE_DATA *speaking;
        int position;
        sh_int defposition;
        sh_int height;
        sh_int weight;
        sh_int hitroll;
        sh_int damroll;
        sh_int perm_str;
        sh_int perm_int;
        sh_int perm_wis;
        sh_int perm_dex;
        sh_int perm_con;
        sh_int perm_cha;
        sh_int perm_lck;
        sh_int perm_frc;
        sh_int saving_poison_death;
        sh_int saving_wand;
        sh_int saving_para_petri;
        sh_int saving_breath;
        sh_int saving_spell_staff;
};


struct hunt_hate_fear
{
        char     *name;
        CHAR_DATA *who;
};

struct fighting_data
{
        CHAR_DATA *who;
        int xp;
        sh_int align;
        sh_int duration;
        sh_int timeskilled;
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
 * Body Parts Stuff
 */
#define BODY_NONE	0
#define BODY_L_LEG	BV00
#define BODY_R_LEG	BV01
#define BODY_L_FOOT	BV02
#define BODY_R_FOOT	BV03
#define BODY_L_ARM	BV04
#define BODY_R_ARM	BV05
#define BODY_L_WRIST	BV06
#define BODY_R_WRIST	BV07
#define BODY_L_KNEE	BV08
#define BODY_R_KNEE	BV09
#define BODY_L_ANKLE	BV10
#define BODY_R_ANKLE	BV11
#define BODY_L_SHOLDER	BV12
#define BODY_R_SHOLDER	BV13
#define BODY_L_HAND	BV14
#define BODY_R_HAND	BV15
#define BODY_NOSE	BV16
#define BODY_RIBS	BV17
#define BODY_JAW	BV18
#define BODY_STOMACH	BV19
#define BODY_CHEST	BV20
#define MAX_BODY_PARTS  21

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
        CHAR_DATA *next_in_area;
        CHAR_DATA *prev_in_area;
        CHAR_DATA *master;
        CHAR_DATA *leader;
        FIGHT_DATA *fighting;
        CHAR_DATA *heldby;  /* Hold/bind/struggle/subdue. -Ulthrax */
        bool held;
        CHAR_DATA *holding;
        CHAR_DATA *reply;
        CHAR_DATA *oreply;
        CHAR_DATA *switched;
        CHAR_DATA *mount;
        CHAR_DATA *questgiver;  /* Vassago */
        HHF_DATA *hunting;
        HHF_DATA *fearing;
        HHF_DATA *hating;
        SPEC_FUN *spec_fun;
        SPEC_FUN *spec_2;
        RACE_DATA *race;
        char     *spec_funname;
        char     *spec2_funname;
        MPROG_ACT_LIST *mpact;
        int mpactnum;
        sh_int mpscriptpos;
        MOB_INDEX_DATA *pIndexData;
        DESCRIPTOR_DATA *desc;
        AFFECT_DATA *first_affect;
        AFFECT_DATA *last_affect;
        struct note_data *pnote;
        OBJ_DATA *first_carrying;
        OBJ_DATA *last_carrying;
        ROOM_INDEX_DATA *in_room;
        ROOM_INDEX_DATA *was_in_room;
        ROOM_INDEX_DATA *was_sentinel;
        ROOM_INDEX_DATA *plr_home;
        PC_DATA  *pcdata;
        DO_FUN   *last_cmd;
        DO_FUN   *prev_cmd; /* mapping */
        void     *dest_buf;
        void     *dest_buf_2;
        void     *dest_buf_3;
        void     *spare_ptr;
        char     *alloc_ptr;
        int tempnum;
        struct editor_data *editor;
        TIMER    *first_timer;
        TIMER    *last_timer;
        char     *name;
        char     *short_descr;
        char     *long_descr;
        char     *description;
        sh_int num_fighting;
        sh_int substate;
        sh_int sex;
        sh_int top_level;
        sh_int skill_level[MAX_ABILITY];
        sh_int trust;
        OBJ_DATA *on;
        int played;
        time_t logon;
        time_t save_time;
        sh_int timer;
        sh_int wait;
        sh_int hit;
        sh_int max_hit;
        sh_int endurance;
        sh_int max_endurance;
        sh_int numattacks;
        sh_int nextquest;   /* Vassago */
        sh_int countdown;   /* Vassago */
        sh_int questobj;    /* Vassago */
        sh_int questmob;    /* Vassago */
        sh_int questhp; /* Greven */
        long int gold;
        long experience[MAX_ABILITY];
        int act;
        int affected_by;
        int carry_weight;
        int carry_number;
        EXT_BV xflags;
        int resistant;
        int immune;
        int susceptible;
        int attacks;
        int defenses;
        LANGUAGE_DATA *speaking;
        sh_int saving_poison_death;
        sh_int saving_wand;
        sh_int saving_para_petri;
        sh_int saving_breath;
        sh_int saving_spell_staff;
        sh_int alignment;
        sh_int barenumdie;
        sh_int baresizedie;
        sh_int mobthac0;
        sh_int hitroll;
        sh_int damroll;
        sh_int hitplus;
        sh_int damplus;
        sh_int position;
        sh_int defposition;
        sh_int height;
        sh_int weight;
        sh_int armor;
        sh_int wimpy;
        EXT_BV deaf;
        sh_int perm_str;
        sh_int perm_int;
        sh_int perm_wis;
        sh_int perm_dex;
        sh_int perm_con;
        sh_int perm_cha;
        sh_int perm_lck;
        sh_int perm_frc;
        sh_int bonus_str;
        sh_int bonus_int;
        sh_int bonus_wis;
        sh_int bonus_dex;
        sh_int bonus_con;
        sh_int bonus_cha;
        sh_int bonus_lck;
        sh_int bonus_frc;
        sh_int mod_str;
        sh_int mod_int;
        sh_int mod_wis;
        sh_int mod_dex;
        sh_int mod_con;
        sh_int mod_cha;
        sh_int mod_lck;
        sh_int mod_frc;
        sh_int mental_state;    /* simplified */
        sh_int emotional_state; /* simplified */
        int retran;
        int regoto;
        sh_int mobinvis;    /* Mobinvis level SB */
        sh_int backup_wait; /* reinforcements */
        int backup_mob; /* reinforcements */
        sh_int was_stunned;
        char     *mob_clan; /* for spec_clan_guard.. set by postguard */
        sh_int main_ability;
        char     *owner;
        ROOM_INDEX_DATA *home;
        sh_int colors[MAX_COLORS];
        int bodyparts;
        int home_vnum;  /* hotboot tracker */
        char     *following;
        char     *groupleader;
        sh_int speed;
};

struct specfun_list
{
        SPEC_LIST *next;
        SPEC_LIST *prev;
        char     *name;
};

struct killed_data
{
        int vnum;
        char count;
};

struct temp_greet_ptr;

/*
 * Data which only PC's have.
 */
struct pc_data
{
        CLAN_DATA *clan;
        AREA_DATA *area;
        struct wanted_data *first_wanted;
        struct wanted_data *last_wanted;
        char     *full_name;
        char     *homepage;
        char     *pwd;
        char     *spouse;
        CHAR_DATA *propose;
        char     *bamfin;
        char     *bamfout;
        char     *rank;
        char     *title;
        char     *bestowments;  /* Special bestowed commands       */
        int flags;  /* Whether the player is deadly and whatever else we add.      */
        int pkills; /* Number of pkills on behalf of clan */
        int pdeaths;    /* Number of times pkilled (legally)  */
        int mkills; /* Number of mobs killed           */
        int mdeaths;    /* Number of deaths due to mobs       */
        int illegal_pk; /* Number of illegal pk's committed   */
        long int restore_time;  /* The last time the char did a restore all */
        int r_range_lo; /* room range */
        int r_range_hi;
        int m_range_lo; /* mob range  */
        int m_range_hi;
        int o_range_lo; /* obj range  */
        int o_range_hi;
        sh_int wizinvis;    /* wizinvis level */
        sh_int min_snoop;   /* minimum snoop level */
        sh_int condition[MAX_CONDS];
        sh_int learned[MAX_SKILL];
        KILLED_DATA killed[MAX_KILLTRACK];
        sh_int quest_number;    /* current *QUEST BEING DONE* DON'T REMOVE! */
        sh_int quest_curr;  /* current number of quest points */
        int quest_accum;    /* quest points accumulated in players life */
        int auth_state;
        time_t release_date;    /* Auto-helling.. Altrag */
        char     *helled_by;
        char     *bio;  /* Personal Bio */
        char     *authed_by;    /* what crazy imm authed this name ;) */
        char     *prompt;   /* User config prompts */
        char     *fprompt;  /* User config fighting prompts */
        char     *subprompt;    /* Substate prompt */
        sh_int pagerlen;    /* For pager (NOT menus) */
        sh_int addiction[10];
        sh_int drug_level[10];
        long bank;
        int clanrank;   /* current clan rank */
        int recall;
        int rp; /* rp points */
        int illness;    /* illness value */
        int realage;    /* rp points */
        sh_int arousal; /* arousal points */
        int weight; /* temper points */
        int height; /* temper points */
        int eye;    /* temper points */
        int hair;   /* temper points */
        int build;  /* temper points */
        int complextion;    /* temper points */
        char     *ignore[MAX_IGNORE];
        char     *sendmail; /*buffer used to send an email */
        bool hotboot;   /* hotboot tracker */
        int comchan;    /* for talk tune */
        int implants[MAX_IMPLANT_TYPES];
        sh_int statpoints;
        sh_int statedit;
#ifdef ACCOUNT
        struct account_data *account;
#endif
        TIME_INFO_DATA birthday;
        sh_int age;
        int godflags;
        char     *pose;
        char     *listening;
#ifdef IMC
        IMC_CHARDATA *imcchardata;
#endif
		  char     *email;
        char     *realname;
        char     *icq;
        char     *msn;
        char     *aolim;
        char     *yahoo;

        temp_greet_ptr * greet_info;

};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		19

struct liq_type
{
        char     *liq_name;
        char     *liq_color;
        sh_int liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
        EXTRA_DESCR_DATA *next; /* Next in list                     */
        EXTRA_DESCR_DATA *prev; /* Previous in list                 */
        char     *keyword;  /* Keyword in look/examine          */
        char     *description;  /* What to see                      */
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
        MPROG_DATA *mudprogs;   /* objprogs */
        int progtypes;  /* objprogs */
        char     *name;
        char     *short_descr;
        char     *description;
        char     *action_desc;
        int vnum;
        sh_int level;
        sh_int item_type;
        int extra_flags;
        int magic_flags;    /*Need more bitvectors for spells - Scryn */
        int wear_flags;
        sh_int count;
        sh_int weight;
        int cost;
        int value[6];
        sh_int layers;
        int rent;   /* Unused */
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
        char     *armed_by;
        char     *name;
        char     *short_descr;
        char     *description;
        char     *action_desc;
        sh_int item_type;
        sh_int mpscriptpos;
        int extra_flags;
        int magic_flags;    /*Need more bitvectors for spells - Scryn */
        int wear_flags;
        int blaster_setting;
        MPROG_ACT_LIST *mpact;  /* mudprogs */
        int mpactnum;   /* mudprogs */
        sh_int wear_loc;
        sh_int weight;
        int cost;
        sh_int level;
        sh_int timer;
        int value[6];
        sh_int count;   /* support for object grouping */
        int room_vnum;  /* hotboot tracker */
};


/*
 * Exit data.
 */
struct exit_data
{
        EXIT_DATA *prev;    /* previous exit in linked list */
        EXIT_DATA *next;    /* next exit in linked list */
        EXIT_DATA *rexit;   /* Reverse exit pointer     */
        ROOM_INDEX_DATA *to_room;   /* Pointer to destination room  */
        char     *keyword;  /* Keywords for exit or door    */
        char     *description;  /* Description of exit      */
        int vnum;   /* Vnum of room exit leads to   */
        int rvnum;  /* Vnum of room in opposite dir */
        int exit_info;  /* door states & other flags    */
        int key;    /* Key vnum         */
        sh_int vdir;    /* Physical "direction"     */
        sh_int distance;    /* how far to the next room */
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
        char command;
        int extra;
        int arg1;
        int arg2;
        int arg3;
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
#define AREA_VERSION 	2
struct area_data
{
        int version;
        AREA_DATA *next;
        AREA_DATA *prev;
        AREA_DATA *next_sort;
        AREA_DATA *prev_sort;
        RESET_DATA *first_reset;
        RESET_DATA *last_reset;
        PLANET_DATA *planet;
        BODY_DATA *body;
        AREA_DATA *next_on_planet;
        AREA_DATA *prev_on_planet;
        AREA_DATA *next_on_body;
        AREA_DATA *prev_on_body;
        /*
         * Person vs Player to be consistant 
         */
        CHAR_DATA *first_person;
        CHAR_DATA *last_person;
        char     *name;
        char     *filename;
        int flags;
        sh_int status;  /* h, 8/11 */
        sh_int age;
        sh_int nplayer;
        sh_int reset_frequency;
        int low_r_vnum;
        int hi_r_vnum;
        int low_o_vnum;
        int hi_o_vnum;
        int low_m_vnum;
        int hi_m_vnum;
        int low_soft_range;
        int hi_soft_range;
        int low_hard_range;
        int hi_hard_range;
        char     *author;   /* Scryn */
        char     *resetmsg; /* Rennard */
        RESET_DATA *last_mob_reset;
        RESET_DATA *last_obj_reset;
        sh_int max_players;
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
        int low_r_vnum;
        int hi_r_vnum;
        int low_o_vnum;
        int hi_o_vnum;
        int low_m_vnum;
        int hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
        int maxplayers; /* Maximum players this boot   */
        int alltimemax; /* Maximum players ever   */
        char     *time_of_max;  /* Time of max ever */
        bool NO_NAME_RESOLVING; /* Hostnames are not resolved  */
        bool DENY_NEW_PLAYERS;  /* New players cannot connect  */
        bool WAIT_FOR_AUTH; /* New players must be auth'ed */
        sh_int read_all_mail;   /* Read all player mail(was 54) */
        sh_int read_mail_free;  /* Read mail for free (was 51) */
        sh_int write_mail_free; /* Write mail for free(was 51) */
        sh_int take_others_mail;    /* Take others mail (was 54)   */
        sh_int muse_level;  /* Level of muse channel */
        sh_int think_level; /* Level of think channel LEVEL_HIGOD */
        sh_int build_level; /* Level of build channel LEVEL_BUILD */
        sh_int log_level;   /* Level of log channel LEVEL LOG */
        sh_int level_modify_proto;  /* Level to modify prototype stuff LEVEL_LESSER */
        sh_int level_override_private;  /* override private flag */
        sh_int level_mset_player;   /* Level to mset a player */
        sh_int stun_plr_vs_plr; /* Stun mod player vs. player */
        sh_int stun_regular;    /* Stun difficult */
        sh_int dam_plr_vs_plr;  /* Damage mod player vs. player */
        sh_int dam_plr_vs_mob;  /* Damage mod player vs. mobile */
        sh_int dam_mob_vs_plr;  /* Damage mod mobile vs. player */
        sh_int dam_mob_vs_mob;  /* Damage mod mobile vs. mobile */
        sh_int level_getobjnotake;  /* Get objects without take flag */
        sh_int level_forcepc;   /* The level at which you can use force on players. */
        sh_int max_sn;  /* Max skills */
        int save_flags; /* Toggles for saving conditions */
        sh_int save_frequency;  /* How old to autosave someone */
        void     *dlHandle;
        sh_int newbie_purge;    /* Level to auto-purge newbies at - Samson 12-27-98 */
        sh_int regular_purge;   /* Level to purge normal players at - Samson 12-27-98 */
        bool CLEANPFILES;   /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
        bool PORT;  /* Should the mud clean up pfiles daily? - Greven 9-13-03 */
        int log_size;   /*Added to control size of log files. */
        char     *mud_name; /* Name fo mud for the webserver, maybe more - Greven 11-20-03 */
        char     *mud_url;  /* Mud Websites URL. ie, http://darkwars.wolfpaw.net/ - Gavin 12-17-03 */
        char     *mud_email;    /* Mud email address. ie, darkwarsmud@hotmail.com - Gavin 12-22-03 */
        char     *mail_path;    /* Path to sendmail type program */
        sh_int ban_site_level;  /* Level to ban sites - Gavin 12-21-03 */
        sh_int ban_class_level; /* Level to ban classes - Gavin 12-21-03 */
        sh_int ban_race_level;  /* Level to ban races - Gavin 12-21-03 */
        sh_int channellog;  /* Amount of items to save into ooc log Green 09/03/04 */
        bool web;   /* Is the web server on? 20/10/04 */

        int DEBUG; /* Cset to toggle backtrace and other debugging information */
        int GREET; /* Toggle Greet System - 2005-11-26 - Gavin */


        /* Pulses */
        int pulse_taxes;
        int pulse_area;
        int pulse_mobile;
        int pulse_violence;
        int pulse_point;
        int pulse_second;
        int pulse_space;
        int pulse_ship;
        int pulse_recharge;
};


struct HOME_DATA;
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
#ifdef OLC_SHUTTLE
        struct shuttle_data *first_shuttle;
        struct shuttle_data *last_shuttle;
#endif
        char     *name;
        char     *description;
        int vnum;
        EXT_BV room_flags;
        MPROG_ACT_LIST *mpact;  /* mudprogs */
        int mpactnum;   /* mudprogs */
        MPROG_DATA *mudprogs;   /* mudprogs */
        sh_int mpscriptpos;
        int progtypes;  /* mudprogs */
        sh_int light;
        sh_int sector_type;
        sh_int tunnel;  /* max people that will fit */
        /* Eventually change this to a union or struct pointer for other types
         * such as room->isa->ship
         */
        HOME_DATA * home;
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
        SKILL_HERB
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
        char     *name; /* Name of skill        */
        SPELL_FUN *spell_fun;   /* Spell pointer (for spells)   */
        char     *spell_fun_name;   /* Spell function name - Trax */
        DO_FUN   *skill_fun;    /* Skill pointer (for skills)   */
        char     *skill_fun_name;   /* Skill function name - Trax */
        sh_int target;  /* Legal targets        */
        sh_int minimum_position;    /* Position for caster / user   */
        sh_int slot;    /* Slot for #OBJECT loading */
        sh_int min_endurance;   /* Minimum endurance used       */
        sh_int beats;   /* Rounds required to use skill */
        char     *noun_damage;  /* Damage message       */
        char     *msg_off;  /* Wear off message     */
        sh_int guild;   /* Which guild the skill belongs to */
        sh_int min_level;   /* Minimum level to be able to cast */
        sh_int type;    /* Spell/Skill/Weapon/Tongue    */
        int flags;  /* extra stuff          */
        char     *hit_char; /* Success message to caster    */
        char     *hit_vict; /* Success message to victim    */
        char     *hit_room; /* Success message to room  */
        char     *miss_char;    /* Failure message to caster    */
        char     *miss_vict;    /* Failure message to victim    */
        char     *miss_room;    /* Failure message to room  */
        char     *die_char; /* Victim death msg to caster   */
        char     *die_vict; /* Victim death msg to victim   */
        char     *die_room; /* Victim death msg to room */
        char     *imm_char; /* Victim immune msg to caster  */
        char     *imm_vict; /* Victim immune msg to victim  */
        char     *imm_room; /* Victim immune msg to room    */
        char     *dice; /* Dice roll            */
        int value;  /* Misc value           */
        char saves; /* What saving spell applies    */
        char difficulty;    /* Difficulty of casting/learning */
        SMAUG_AFF *affects; /* Spell affects, if any    */
        char     *components;   /* Spell components, if any */
        char     *teachers; /* Skill requires a special teacher */
        char     *races;    /* Races that can use this skill */
        char participants;  /* # of required participants   */
        struct timerset userec; /* Usage record         */
        int alignment;  /* for jedi powers */
        EXT_BV body_parts;
        bool held;
};


struct auction_data
{
        OBJ_DATA *item; /* a pointer to the item */
        CHAR_DATA *seller;  /* a pointer to the seller - which may NOT quit */
        CHAR_DATA *buyer;   /* a pointer to the buyer - which may NOT quit */
        int bet;    /* last bet - or 0 if noone has bet anything */
        sh_int going;   /* 1,2, sold */
        sh_int pulse;   /* how many pulses (.25 sec) until another call-out ? */
        int starting;
};


/*
 * These are skill_lookup return values for common skills and spells.
 */

/* Piloting */
extern sh_int gsn_starfighters;
extern sh_int gsn_midships;
extern sh_int gsn_capitalships;
extern sh_int gsn_weaponsystems;
extern sh_int gsn_navigation;
extern sh_int gsn_shipsystems;
extern sh_int gsn_tractorbeams;
extern sh_int gsn_shipmaintenance;
extern sh_int gsn_spacecombat;
extern sh_int gsn_spacecombat2;
extern sh_int gsn_spacecombat3;
extern sh_int gsn_bomb;
extern sh_int gsn_boardship;

extern sh_int gsn_recruit;
extern sh_int gsn_juke;
extern sh_int gsn_extrapolate;
extern sh_int gsn_evade;
extern sh_int gsn_roll;
extern sh_int gsn_blockdoor;
extern sh_int gsn_modifyexit;
extern sh_int gsn_reinforcements;
extern sh_int gsn_postguard;

extern sh_int gsn_add_patrol;
extern sh_int gsn_elite_guard;
extern sh_int gsn_special_forces;
extern sh_int gsn_jail;
extern sh_int gsn_smalltalk;
extern sh_int gsn_propeganda;
extern sh_int gsn_bribe;
extern sh_int gsn_seduce;
extern sh_int gsn_masspropeganda;
extern sh_int gsn_gather_intelligence;

extern sh_int gsn_torture;
extern sh_int gsn_snipe;
extern sh_int gsn_throwsaber;
extern sh_int gsn_throw;
extern sh_int gsn_disguise;
extern sh_int gsn_changesex;
extern sh_int gsn_mine;
extern sh_int gsn_grenades;
extern sh_int gsn_contraband;

extern sh_int gsn_beg;
extern sh_int gsn_makeblade;
extern sh_int gsn_implant;
extern sh_int gsn_makeimplant;
extern sh_int gsn_shipdesign;
extern sh_int gsn_dismantle_ship;
extern sh_int gsn_makejewelry;
extern sh_int gsn_makeblaster;
extern sh_int gsn_makelight;
extern sh_int gsn_makefurniture;
extern sh_int gsn_makebinding;
extern sh_int gsn_modifyship;
extern sh_int gsn_makebowcaster;
extern sh_int gsn_makecomlink;
extern sh_int gsn_makegrenade;
extern sh_int gsn_makelandmine;
extern sh_int gsn_makearmor;
extern sh_int gsn_makeshield;
extern sh_int gsn_makepike;
extern sh_int gsn_makeknife;
extern sh_int gsn_makegoggles;
extern sh_int gsn_makecontainer;
extern sh_int gsn_makebeacon;
extern sh_int gsn_gemcutting;
extern sh_int gsn_lightsaber_crafting;
extern sh_int gsn_spice_refining;
extern sh_int gsn_identify;
extern sh_int gsn_makebase;
extern sh_int gsn_roomconstruction;

extern sh_int gsn_detrap;
extern sh_int gsn_backstab;
extern sh_int gsn_circle;
extern sh_int gsn_dodge;
extern sh_int gsn_hide;
extern sh_int gsn_peek;
extern sh_int gsn_slight;
extern sh_int gsn_pick_lock;
extern sh_int gsn_scan;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;
extern sh_int gsn_gouge;
extern sh_int gsn_track;
extern sh_int gsn_search;
extern sh_int gsn_dig;
extern sh_int gsn_mount;
extern sh_int gsn_bashdoor;
extern sh_int gsn_berserk;
extern sh_int gsn_hitall;
extern sh_int gsn_pickshiplock;
extern sh_int gsn_hijack;
extern sh_int gsn_break;
extern sh_int gsn_imprison;

extern sh_int gsn_craftpike;
extern sh_int gsn_craftknife;
extern sh_int gsn_longcall;
extern sh_int gsn_cuteness;
extern sh_int gsn_trophy;
extern sh_int gsn_jimmyshiplock;
extern sh_int gsn_systemmaintenance;
extern sh_int gsn_hydrolic_strength;
extern sh_int gsn_camouflage;
extern sh_int gsn_hone;
extern sh_int gsn_gowithout;
extern sh_int gsn_gamorrean_strength;
extern sh_int gsn_jab;
extern sh_int gsn_savant_mid_ships;
extern sh_int gsn_savant_capital_ships;
extern sh_int gsn_coerce;
extern sh_int gsn_repair;
extern sh_int gsn_cajole;
extern sh_int gsn_pretend;
extern sh_int gsn_placebeacon;
extern sh_int gsn_fleet_command1;

extern sh_int gsn_disarm;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_kick;
extern sh_int gsn_trip;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;
extern sh_int gsn_second_attack;
extern sh_int gsn_third_attack;
extern sh_int gsn_dual_wield;


extern sh_int gsn_aid;

/* used to do specific lookups */
extern sh_int gsn_first_spell;
extern sh_int gsn_first_skill;
extern sh_int gsn_first_weapon;
extern sh_int gsn_first_tongue;
extern sh_int gsn_top_sn;

/* spells */
extern sh_int gsn_blindness;
extern sh_int gsn_potential;
extern sh_int gsn_force_healing;
extern sh_int gsn_sense_force;
extern sh_int gsn_charm_person;
extern sh_int gsn_aqua_breath;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_poison;
extern sh_int gsn_sleep;
extern sh_int gsn_possess;
extern sh_int gsn_fireball; /* for fireshield  */
extern sh_int gsn_lightning_bolt;   /* for shockshield */

/* newer attack skills */
extern sh_int gsn_punch;
extern sh_int gsn_bash;
extern sh_int gsn_stun;

extern sh_int gsn_poison_weapon;
extern sh_int gsn_climb;

extern sh_int gsn_blasters;
extern sh_int gsn_force_pikes;
extern sh_int gsn_addpersonel;
extern sh_int gsn_bowcasters;
extern sh_int gsn_sharpen;
extern sh_int gsn_lightsabers;
extern sh_int gsn_vibro_blades;
extern sh_int gsn_knives;
extern sh_int gsn_lockdoor;
extern sh_int gsn_makekey;
extern sh_int gsn_flexible_arms;
extern sh_int gsn_talonous_arms;
extern sh_int gsn_bludgeons;
extern sh_int gsn_sabotage;
extern sh_int gsn_meditate;
extern sh_int gsn_claw;
extern sh_int gsn_bite;
extern sh_int gsn_sting;
extern sh_int gsn_tail;

extern sh_int gsn_grip;

/* languages */
extern sh_int gsn_basic;
extern sh_int gsn_wookiee;
extern sh_int gsn_twilek;
extern sh_int gsn_rodian;
extern sh_int gsn_hutt;
extern sh_int gsn_bothan;
extern sh_int gsn_kel_dor;
extern sh_int gsn_mon_calamari;
extern sh_int gsn_noghri;
extern sh_int gsn_csillian;
extern sh_int gsn_gamorrean;
extern sh_int gsn_jawa;
extern sh_int gsn_adarian;
extern sh_int gsn_ewok;
extern sh_int gsn_verpine;
extern sh_int gsn_defel;
extern sh_int gsn_trandoshan;
extern sh_int gsn_shistavanan;
extern sh_int gsn_binary;
extern sh_int gsn_duinduogwuin;
extern sh_int gsn_barabel;
extern sh_int gsn_ithorian;
extern sh_int gsn_devaronian;
extern sh_int gsn_durosian;
extern sh_int gsn_gotal;
extern sh_int gsn_talz;
extern sh_int gsn_ho_din;
extern sh_int gsn_falleen;
extern sh_int gsn_givin;

/* Occupations */
extern sh_int gsn_dishwasher;
extern sh_int gsn_busser;
extern sh_int gsn_cook;
extern sh_int gsn_waiter;
extern sh_int gsn_chef;
extern sh_int gsn_bartender;
extern sh_int gsn_streetcleaner;
extern sh_int gsn_interiorcleaner;
extern sh_int gsn_hotelcleaner;
extern sh_int gsn_secretary;
extern sh_int gsn_clerk;
extern sh_int gsn_commmarketer;
extern sh_int gsn_marketer;
extern sh_int gsn_solicitor;
extern sh_int gsn_advertiser;
extern sh_int gsn_banker;
extern sh_int gsn_realitor;
extern sh_int gsn_accountant;
extern sh_int gsn_investor;
extern sh_int gsn_broker;
extern sh_int gsn_boardmember;
extern sh_int gsn_ceo;
extern sh_int gsn_youmom;

/* Pirate */
extern sh_int gsn_scan1;
extern sh_int gsn_scan2;
extern sh_int gsn_scan3;
extern sh_int gsn_jam;
extern sh_int gsn_transfercargo;
extern sh_int gsn_dock;
extern sh_int gsn_boardship;
extern sh_int gsn_cloak;
extern sh_int gsn_stealth;
extern sh_int gsn_disrupt;
extern sh_int gsn_causedesertion;
extern sh_int gsn_causedissension;
extern sh_int gsn_boostmorale;

/* Medical */
extern sh_int gsn_splint;
extern sh_int gsn_first_aid;
extern sh_int gsn_autopsy;
extern sh_int gsn_diagnose;
extern sh_int gsn_makemedkit;

#include "utils.h"

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

bool ext_is_empty args((EXT_BV * bits));
void ext_clear_bits args((EXT_BV * bits));
int ext_has_bits args((EXT_BV * var, EXT_BV * bits));
bool ext_same_bits args((EXT_BV * var, EXT_BV * bits));
void ext_set_bits args((EXT_BV * var, EXT_BV * bits));
void ext_remove_bits args((EXT_BV * var, EXT_BV * bits));
void ext_toggle_bits args((EXT_BV * var, EXT_BV * bits));
EXT_BV fread_bitvector args((FILE * fp));
char     *print_bitvector args((EXT_BV * bits));
void fwrite_bitvector args((EXT_BV * bits, FILE * fp));
EXT_BV meb args((int bit));
EXT_BV multimeb args((int bit, ...));
char     *ext_flag_string args((EXT_BV * bitvector, char *const flagarray[]));
char     *show_ext_flag_string args((int len, char *const flagarray[]));

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
#define __STRING(x) #x


/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC) || (ch)->pcdata == NULL)
#define IS_QUESTOR(ch)  (IS_SET((ch)->act, PLR_QUESTOR))
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO)
#define IS_PLAYING(d)		((d)->connected == CON_PLAYING || \
		(d)->connected == CON_FORKED || (d)->connected == CON_IAFORKED )
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING || IS_AFFECTED( (ch), AFF_CHARM ))
#define GET_AC(ch)		( (ch)->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) \
				- ( !str_cmp((ch)->race->name(), "defel") ? (ch)->skill_level[COMBAT_ABILITY]*2+5 : (ch)->skill_level[COMBAT_ABILITY]/2 ) )
#define GET_HITROLL(ch)		((ch)->hitroll				    \
				    +str_app[get_curr_str(ch)].tohit	    \
				    +(2-(abs((ch)->mental_state)/10)))
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +str_app[get_curr_str(ch)].todam	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(IS_OUTSIDE_ROOM((ch)->in_room))
    /*
     * (!xIS_SET(                   \
     * (ch)->in_room->room_flags,           \
     * ROOM_INDOORS) && !xIS_SET(               \
     * (ch)->in_room->room_flags,              \
     * ROOM_SPACECRAFT) )
     */

#define IS_OUTSIDE_ROOM(room)	(!xIS_SET((room)->room_flags,		    \
				    ROOM_INDOORS) && !xIS_SET(               \
				    (room)->room_flags,              \
				    ROOM_SPACECRAFT) )
#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    )

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (IS_IMMORTAL(ch) ? 0 :(npulse))))


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

#define DEF_IMM_FLAGS		IMM_ALL | IMM_OWNER
#define IS_IMM_BUILDER(ch)      ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_BUILDER | IMM_HIGHBUILDER | DEF_IMM_FLAGS ) )
#define IS_IMM_HIGH_ENFORCER(ch)     ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_HIGHENFORCER | DEF_IMM_FLAGS ) )
#define IS_IMM_ENFORCER(ch)     ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_ENFORCER | DEF_IMM_FLAGS ) )
#define IS_IMM_ADMIN(ch)        ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_ADMIN | DEF_IMM_FLAGS ) )
#define IS_IMM_HIGH_BUILDER(ch) ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_HIGHBUILDER | DEF_IMM_FLAGS ) )
#define IS_IMM_CODER(ch)        ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_CODER | DEF_IMM_FLAGS ) )
#define IS_IMM_QUEST(ch)        ( !IS_NPC((ch)) && (ch)->pcdata->godflags & ( IMM_QUEST | DEF_IMM_FLAGS ) )

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

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	    \
				{					                    \
				    field  = value;			            \
				    fMatch = TRUE;			            \
				    break;				                \
				}


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->pcdata->full_name ) : (IS_IMMORTAL(ch) ? "An Immortal" : "someone") )



#define log_string( txt )	( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )


#define CMD_OOC			BV00
#define CMD_HELD		BV01
#define CMD_WATCH               BV02
#define CMD_FULLNAME            BV03
/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
        CMDTYPE  *next;
        char     *name;
        DO_FUN   *do_fun;
        char     *fun_name;
        int flags;
        sh_int position;
        sh_int level;
        sh_int log;
        struct timerset userec;
/*        sh_int held;
        sh_int ooc;*/
        int perm_flags;
};



/*
 * Structure for a social in the socials table.
 */
struct social_type
{
        SOCIALTYPE *next;
        char     *name;
        char     *char_no_arg;
        char     *others_no_arg;
        char     *char_found;
        char     *others_found;
        char     *vict_found;
        char     *char_auto;
        char     *others_auto;
        sh_int arousal;
        sh_int minarousal;
        sh_int sex;
};



/*
 * Global constants.
 */
extern time_t last_restore_all_time;
extern time_t boot_time;    /* this should be moved down */
extern HOUR_MIN_SEC *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;

extern const struct str_app_type str_app[41];
extern const struct int_app_type int_app[41];
extern const struct wis_app_type wis_app[41];
extern const struct dex_app_type dex_app[41];
extern const struct con_app_type con_app[41];
extern const struct cha_app_type cha_app[41];
extern const struct lck_app_type lck_app[41];
extern const struct frc_app_type frc_app[41];

extern char *const wear_locs[];
extern char *const ex_flags[];

extern const struct race_type race_table[];
extern const struct liq_type liq_table[LIQ_MAX];
extern char *const attack_table[13];
extern char *const ability_name[MAX_ABILITY];

extern char *const skill_tname[];
extern sh_int const movement_loss[SECT_MAX];
extern char *const dir_name[];
extern char *const where_name[];
extern const sh_int rev_dir[];
extern const int trap_door[];
extern char *const r_flags[];
extern char *const part_flags[];
extern char *const w_flags[];
extern char *const o_flags[];
extern char *const a_flags[];
extern char *const o_types[];
extern char *const a_types[];
extern char *const act_flags[];
extern char *const planet_flags[];
extern char *const weapon_table[13];
extern char *const spice_table[];
extern char *const plr_flags[];
extern char *const pc_flags[];
extern char *const trap_flags[];
extern char *const ris_flags[];
extern char *const trig_flags[];
extern char *const part_flags[];
extern char *const npc_race[];
extern char *const defense_flags[];
extern char *const attack_flags[];
extern char *const area_flags[];
extern char *const ship_flags[];
extern char *const cargo_names[CONTRABAND_MAX];
extern char *const hair_list[];
extern char *const eye_list[];
extern char *const build_list[];
extern char *const height_list[];
extern char *const weight_list[];
extern char *const complextion_list[];
extern char *const illness_list[];
extern char *const body_parts[MAX_BODY_PARTS];
extern char *const npc_sex[SEX_MAX];
extern char *const npc_position[POS_MAX];
extern char *const log_flag[];
extern char *const true_false[];

extern int const lang_array[];
extern char *const lang_names[];

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

extern CMDTYPE *command_hash[126];

extern SKILLTYPE *skill_table[MAX_SKILL];
extern SOCIALTYPE *social_index[27];
extern ch_ret global_retcode;
extern SKILLTYPE *herb_table[MAX_HERB];

extern int cur_obj;
extern int cur_obj_serial;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;

extern HELP_DATA *first_help;
extern HELP_DATA *last_help;
extern SHOP_DATA *first_shop;
extern SHOP_DATA *last_shop;
extern REPAIR_DATA *first_repair;
extern REPAIR_DATA *last_repair;
extern BANK_ACCOUNT *first_baccount;
extern BANK_ACCOUNT *last_baccount;
extern WATCH_DATA *first_watch;
extern WATCH_DATA *last_watch;
extern CHAR_DATA *first_char;
extern CHAR_DATA *last_char;
extern DESCRIPTOR_DATA *first_descriptor;
extern DESCRIPTOR_DATA *last_descriptor;
extern OBJ_DATA *first_object;
extern OBJ_DATA *last_object;
extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;
extern SHIP_DATA *first_ship;
extern SHIP_DATA *last_ship;
extern SPACE_DATA *first_starsystem;
extern SPACE_DATA *last_starsystem;
extern PLANET_DATA *first_planet;
extern PLANET_DATA *last_planet;
extern AREA_DATA *first_area;
extern AREA_DATA *last_area;
extern AREA_DATA *first_build;
extern AREA_DATA *last_build;
extern AREA_DATA *first_asort;
extern AREA_DATA *last_asort;
extern AREA_DATA *first_bsort;
extern AREA_DATA *last_bsort;
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
extern FILE *fpReserve;
extern FILE *fpLOG;
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;
extern MPSLEEP_DATA *first_mpwait;
extern MPSLEEP_DATA *last_mpwait;
extern MPSLEEP_DATA *current_mpwait;
extern AUCTION_DATA *auction;
extern struct act_prog_data *mob_act_list;
extern SPEC_LIST *first_specfun;
extern SPEC_LIST *last_specfun;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(apollo)
int atoi args((const char *string));
void     *calloc args((unsigned nelem, size_t size));
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(hpux)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
#if !defined(__cplusplus)
char     *crypt args((const char *key, const char *salt));
#endif
#endif

#if	defined(MIPS_OS)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(NeXT)
char     *crypt args((const char *key, const char *salt));
#endif

#if	defined(sequent)
char     *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));
int fread args((void *ptr, int size, int n, FILE * stream));
int fseek args((FILE * stream, long offset, int ptrname));
void perror args((const char *s));
int ungetc args((int c, FILE * stream));
#endif

#if	defined(sun)
char     *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));

/*
#if 	defined(SYSV)
*/
size_t fread args((void *ptr, size_t size, size_t n,
/*				FILE *stream ) );
#else
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
*/
                   int fseek args((FILE * stream, long offset, int ptrname));
                   void perror args((const char *s));
                   int ungetc args((int c, FILE * stream));
#endif
#if	defined(ultrix)
                   char *crypt args((const char *key, const char *salt));
#endif
/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif
/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define WATCH_DIR    "../watch/"    /* Imm watch files --Gorog */
#define PLAYER_DIR	"../player/"    /* Player files         */
#define BACKUP_DIR	"../backup/"    /* Backup Player files      */
#define GOD_DIR		"../gods/"  /* God Info Dir         */
#define BOARD_DIR	"../boards/"    /* Board data dir       */
#define CLAN_DIR	"../clans/" /* Clan data dir        */
#define SHIP_DIR        "../space/"
#define SPACE_DIR       "../space/"
#define PLANET_DIR      "../planets/"
#define GUARD_DIR       "../planets/"
#define AREA_DIR       "../areas/"
#define GUILD_DIR       "../guilds/"    /* Guild data dir               */
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR	"../system/"    /* Main system files        */
#define LOG_DIR	"../log/"   /* Main system files        */
#define PROG_DIR    "../mudprogs/"  /* MUDProg files     */
#define CORPSE_DIR	"../corpses/"   /* Corpses          */
#define NULL_FILE	"/dev/null" /* To reserve one stream    */
#define MAIL_DIR      "../mail/"
#define BUG_FILE	SYSTEM_DIR "bugs.txt"   /* For bug( ) */
#define BUGS_FILE	SYSTEM_DIR "reported_bugs.txt"  /* For 'bug' */
#define EMAIL_FILE              SYSTEM_DIR "email.dat"
#define FILE_AREA_LIST	"area.lst"  /* List of areas        */
#define WATCH_LIST      "watch.lst" /* List of watches                        */
#define BAN_LIST        "ban.lst"   /* List of bans                 */
#define CLAN_LIST	"clan.lst"  /* List of clans        */
#define SHIP_LIST       "ship.lst"
#define PLANET_LIST      "planet.lst"
#define SPACE_LIST      "space.lst"
#define DISINTIGRATION_LIST	"disintigration.lst"
#define GUILD_LIST      "guild.lst" /* List of guilds               */
#define GOD_LIST	"gods.lst"  /* List of gods         */
#define GUARD_LIST	"guard.lst"
#define PLANET_HTML_LIST	"planets.html"
#define CLAN_HTML_LIST	"clans.html"
#define SHIP_HTML_LIST	"ships.html"
#define COPYOVER_FILE	SYSTEM_DIR "copyover.dat"   /* for warm reboots    */
#define EXE_FILE	"../src/swr"    /*  Executable Path */
#define MAIN_EXE_FILE	"/home/mud/darkwars/dwadmins/src/swr"   /*  Executable Path */
#define PORT_EXE_FILE	"../port/src/swr"   /*  Executable Path */
#define PSHIP_AREA            "pships.are"
#define BOARD_FILE	"boards.txt"    /* For bulletin boards   */
#define SHUTDOWN_FILE	"../log/shutdown/shutdown.txt"  /* For 'shutdown'    */
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"   /* Boot up error file  */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"  /* For 'idea'      */
#define MEMORY_FILE	SYSTEM_DIR "memory.txt" /* For 'wiznet'        */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"  /* For 'typo'      */
#define PLANETS_HTML_FILE	SYSTEM_DIR "planets.html"   /* For beginning html test         */
#define LOG_FILE	SYSTEM_DIR "log.txt"    /* For talking in logged rooms */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"    /* Wizlist         */
#define SKILL_FILE	SYSTEM_DIR "skills.dat" /* Skill table     */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"  /* Herb table      */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"    /* Socials         */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat"   /* Commands        */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"  /* How many people are on 
                                             * every half hour - trying to
                                             * determine best reboot time */
#define HELP_FILE      SYSTEM_DIR "help.txt"    /* For undefined helps */
#define TEMP_FILE      VENDOR_DIR "temp.txt"    /* For undefined helps */
#define PID_FILE	"../swr.pid"    /* Pid File */
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
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE
#define SH      SHIP_DATA
#ifdef MCCP
/* mccp.c */
                   bool compressStart
                   args((DESCRIPTOR_DATA * d, unsigned char telopt));
                   bool compressEnd args((DESCRIPTOR_DATA * d));
#endif
/* act_comm.c */
                   void log_printf args((char *fmt,...));
                   void copyover_recover args((void));
                   bool has_comlink args((CHAR_DATA * ch));
                   bool circle_follow
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void add_follower
                   args((CHAR_DATA * ch, CHAR_DATA * master));
                   void stop_follower args((CHAR_DATA * ch));
                   void die_follower args((CHAR_DATA * ch));
                   bool is_same_group
                   args((CHAR_DATA * ach, CHAR_DATA * bch));
                   void send_rip_screen args((CHAR_DATA * ch));
                   void send_rip_title args((CHAR_DATA * ch));
                   void send_ansi_title args((CHAR_DATA * ch));
                   void send_ascii_title args((CHAR_DATA * ch));
                   void to_channel
                   args((const char *argument, int channel, const char *verb,
                         sh_int level));
                   void info_chan args((char *argument));
                   void web_broadcast args((char *argument));
                   void auto_save args((CHAR_DATA * ch));
                   void talk_auction args((char *argument));
                   bool knows_language
                   args((CHAR_DATA * ch, LANGUAGE_DATA * lang,
                         CHAR_DATA * cch));
                   bool can_learn_lang
                   args((CHAR_DATA * ch, LANGUAGE_DATA * lang));
                   char *translate
                   args((CHAR_DATA * ch, CHAR_DATA * victim,
                         const char *argument));
                   char *obj_short args((OBJ_DATA * obj));
/* act_info.c */
                   int get_door args((char *arg));
                   char *format_obj_to_char
                   args((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
                   void show_list_to_char
                   args((OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
                         bool fShowNothing));
                   void save_help args((void));
/* finger2.c */
                   void read_finger args((CHAR_DATA * ch, char *argument));
                   void fread_finger
                   args((CHAR_DATA * ch, FILE * fp, char *laston));
                   void save_finger args((CHAR_DATA * ch));
/* act_move.c */
                   void clear_vrooms args((void));
                   EXIT_DATA * find_door args((CHAR_DATA * ch, char *arg, bool quiet));
                   EXIT_DATA * get_exit args((ROOM_INDEX_DATA * room, sh_int dir));
                   EXIT_DATA * get_exit_to args((ROOM_INDEX_DATA * room, sh_int dir, int vnum));
                   EXIT_DATA * get_exit_num args((ROOM_INDEX_DATA * room, sh_int count));
                   ch_ret move_char args((CHAR_DATA * ch, EXIT_DATA * pexit, int fall,bool running));
                   sh_int encumbrance args((CHAR_DATA * ch, sh_int move));
                   bool will_fall args((CHAR_DATA * ch, int fall));
                   int wherehome args((CHAR_DATA * ch));
                   CMDF do_hold_person args((CHAR_DATA * ch, char *argument));
                   CMDF do_release args((CHAR_DATA * ch, char *argument));
                   char *wordwrap args((char *txt, sh_int wrap));
/* act_obj.c */
                   obj_ret damage_obj args((OBJ_DATA * obj));
                   sh_int get_obj_resistance args((OBJ_DATA * obj));
                   void save_clan_storeroom
                   args((CHAR_DATA * ch, CLAN_DATA * clan));
                   void obj_fall args((OBJ_DATA * obj, bool through));
/* act_wiz.c */
                   void close_area args((AREA_DATA * pArea));
                   RID * find_location args((CHAR_DATA * ch, char *arg));
                   void echo_to_room
                   args((sh_int AT_COLOR, ROOM_INDEX_DATA * room,
                         char *argument));
                   void echo_to_all
                   args((sh_int AT_COLOR, char *argument, sh_int tar));
                   void echo_to_clan
                   args((sh_int AT_COLOR, char *argument, CLAN_DATA * clan));
                   void add_to_wizlist
                   args((char *name, int level, int flags));
                   void really_destroy_ship args((SHIP_DATA * ship));
                   void really_destroy_mob_ship args((SHIP_DATA * ship));
                   CMDF do_webroom(char *argument);
                   void transship(SHIP_DATA * ship, int destination);
                   bool validate_spec_fun(char *name);
                   void get_reboot_string args((void));
                   struct tm * update_time args((struct tm * old_time));
                   void free_social args((SOCIALTYPE * social));
                   void add_social args((SOCIALTYPE * social));
                   void free_command args((CMDTYPE * command));
                   void unlink_command args((CMDTYPE * command));
                   void add_command args((CMDTYPE * command));
/* build.c */
                   char *flag_string
                   args((int bitvector, char *const flagarray[]));
                   int get_mpflag args((char *flag));
                   int get_dir args((char *txt));
                   char *strip_cr args((char *str));
                   int get_vip_flag args((char *flag));
                   int get_wanted_flag args((char *flag));
                   void set_bexit_flag args((EXIT_DATA * pexit, int flag));
/* clans.c */
                   CL * get_clan args((char *name));
                   void load_clans args((void));
                   void save_clan args((CLAN_DATA * clan));
                   long get_taxes args((PLANET_DATA * planet));
                   bool is_clan_ally
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_clan_enemy
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_same_clan
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
/* space.c */
                   SH * get_ship args((char *name));
                   void load_ships args((void));
                   void save_ship args((SHIP_DATA * ship));
                   void load_space args((void));
                   void save_starsystem args((SPACE_DATA * starsystem));
                   SPACE_DATA * starsystem_from_name args((char *name));
                   SPACE_DATA * starsystem_from_vnum args((int vnum));
                   SHIP_DATA * ship_from_obj args((int vnum));
                   SHIP_DATA * ship_from_entrance args((int vnum));
                   SHIP_DATA * ship_from_hanger args((int vnum));
                   SHIP_DATA * ship_from_cockpit args((int vnum));
                   SHIP_DATA * ship_from_navseat args((int vnum));
                   SHIP_DATA * ship_from_coseat args((int vnum));
                   SHIP_DATA * ship_from_pilotseat args((int vnum));
                   SHIP_DATA * ship_from_gunseat args((int vnum));
                   SHIP_DATA * ship_from_turret args((int vnum));
                   SHIP_DATA * ship_from_engine args((int vnum));
                   SHIP_DATA *
                   get_ship_here args((char *name, SPACE_DATA * starsystem));
                   void update_space args((void));
                   void recharge_ships args((void));
                   void move_ships args((void));
                   void update_traffic args((void));
                   bool check_pilot args((CHAR_DATA * ch, SHIP_DATA * ship));
                   bool is_rental args((CHAR_DATA * ch, SHIP_DATA * ship));
                   void echo_to_ship
                   args((int color, SHIP_DATA * ship, char *argument));
                   void echo_to_cockpit
                   args((int color, SHIP_DATA * ship, char *argument));
                   void echo_to_system
                   args((int color, SHIP_DATA * ship, char *argument,
                         SHIP_DATA * ignore));
                   bool extract_ship args((SHIP_DATA * ship));
                   bool ship_to_room args((SHIP_DATA * ship, int vnum));
                   long get_ship_value args((SHIP_DATA * ship));
                   bool rent_ship args((CHAR_DATA * ch, SHIP_DATA * ship));
                   void damage_ship
                   args((SHIP_DATA * ship, int min, int max));
                   void damage_ship_ch
                   args((SHIP_DATA * ship, int min, int max, CHAR_DATA * ch));
                   void destroy_ship args((SHIP_DATA * ship, CHAR_DATA * ch));
                   void ship_to_starsystem
                   args((SHIP_DATA * ship, SPACE_DATA * starsystem));
                   void ship_from_starsystem
                   args((SHIP_DATA * ship, SPACE_DATA * starsystem));
                   void new_missile
                   args((SHIP_DATA * ship, SHIP_DATA * target, CHAR_DATA * ch,
                         int missiletype));
                   void extract_missile args((MISSILE_DATA * missile));
                   SHIP_DATA *
                   ship_in_room args((ROOM_INDEX_DATA * room, char *name));
                   void damage_ship_ch_ion
                   args((SHIP_DATA * ship, int min, int max, CHAR_DATA * ch));
                   void damage_ship_ion
                   args((SHIP_DATA * ship, int min, int max));
                   SHIP_DATA * ship_from_room args((int vnum));
                   void wipe_resets
                   args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom));
                   bool autofly args((SHIP_DATA * ship));
/* comm.c */
                   void close_socket
                   args((DESCRIPTOR_DATA * dclose, bool force));
                   bool write_to_buffer
                   args((DESCRIPTOR_DATA * d, const char *txt, int length));
                   void write_to_pager
                   args((DESCRIPTOR_DATA * d, const char *txt, int length));
                   void send_to_char args((const char *txt, CHAR_DATA * ch));
                   void center_to_char
                   args((char *argument, CHAR_DATA * ch, int columns));
                   char *center_str args((char *argument, int columns));
                   void send_to_char_color
                   args((const char *txt, CHAR_DATA * ch));
                   void send_to_pager args((const char *txt, CHAR_DATA * ch));
                   void send_to_pager_color
                   args((const char *txt, CHAR_DATA * ch));
                   void set_char_color args((sh_int AType, CHAR_DATA * ch));
                   void set_pager_color args((sh_int AType, CHAR_DATA * ch));
                   void ch_printf args((CHAR_DATA * ch, char *fmt,...));
                   void pager_printf args((CHAR_DATA * ch, char *fmt,...));
                   void act
                   args((sh_int AType, const char *format, CHAR_DATA * ch,
                         void *arg1, void *arg2, int type));
                   int strlen_color args((char *argument));
                   extern const unsigned char do_termtype_str[];
                   extern const unsigned char will_compress_str[];
                   extern const unsigned char will_compress2_str[];
/* reset.c */
                   RD *
                   make_reset
                   args((char letter, int extra, int arg1, int arg2,
                         int arg3));
                   RD *
                   add_reset
                   args((AREA_DATA * tarea, char letter, int extra, int arg1,
                         int arg2, int arg3));
                   RD *
                   place_reset
                   args((AREA_DATA * tarea, char letter, int extra, int arg1,
                         int arg2, int arg3));
                   void reset_area args((AREA_DATA * pArea));
                   void instaroom
                   args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom,
                         bool dodoors));
                   void add_obj_reset
                   args((AREA_DATA * pArea, char cm, OBJ_DATA * obj, int v2,
                         int v3));
/* swskills.c */
                   void add_reinforcements args((CHAR_DATA * ch));
                   void load_races args((void));
/* web-server.c */
                   void init_web(int port); void handle_web(void);
                   void shutdown_web(void);

/* db.c */
                   void boot_log args((const char *str,...));
                   void clear_file args((CHAR_DATA * ch, char *filename));
                   void show_file args((CHAR_DATA * ch, char *filename));
                   int file_size args((char *buf));
                   bool file_exist args((char *name));
                   char *str_dup args((char const *str));
                   void boot_db args((bool fCopyOver));
                   void area_update args((void));
                   void add_char args((CHAR_DATA * ch));
                   CD * create_mobile args((MOB_INDEX_DATA * pMobIndex));
                   OD *
                   create_object
                   args((OBJ_INDEX_DATA * pObjIndex, int level));
                   void clear_char args((CHAR_DATA * ch));
                   void free_char args((CHAR_DATA * ch));
                   char *get_extra_descr
                   args((const char *name, EXTRA_DESCR_DATA * ed));
                   MID * get_mob_index args((int vnum));
                   OID * get_obj_index args((int vnum));
                   RID * get_room_index args((int vnum));
                   char fread_letter args((FILE * fp));
                   int fread_number args((FILE * fp));
                   float fread_float args((FILE * fp));
                   char *fread_string args((FILE * fp));
                   char *fread_string_nohash args((FILE * fp));
                   char *fread_string_noalloc args((FILE * fp));
                   void fread_to_eol args((FILE * fp));
                   char *fread_word args((FILE * fp));
                   char *fread_line args((FILE * fp));
                   float fread_float args((FILE * fp));
                   long fread_long args((FILE * fp));
                   int number_fuzzy args((int number));
                   int number_range args((int from, int to));
                   int number_percent args((void));
                   int number_door args((void));
                   int number_bits args((int width));
                   int number_mm args((void));
                   int dice args((int number, int size));
                   int interpolate
                   args((int level, int value_00, int value_32));
                   void smash_tilde args((char *str));
                   void hide_tilde args((char *str));
                   char *show_tilde args((char *str));
                   bool str_cmp args((const char *astr, const char *bstr));
                   bool str_prefix args((const char *astr, const char *bstr));
                   bool str_infix args((const char *astr, const char *bstr));
                   bool str_suffix args((const char *astr, const char *bstr));
                   char *capitalize args((const char *str));
                   char *strlower args((const char *str));
                   char *strupper args((const char *str));
                   char *aoran args((const char *str));
                   void append_file
                   args((CHAR_DATA * ch, char *file, char *str));
                   void append_to_file args((char *file, char *str));
                   void bug args((const char *str,...));
                   void log_string_plus
                   args((const char *str, sh_int log_type, sh_int level));
                   RID *make_room( int vnum, AREA_DATA *area );
                   OID * make_object args((int vnum, int cvnum, char *name));
                   MID * make_mobile args((int vnum, int cvnum, char *name));
                   EXIT_DATA * make_exit args((ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA* to_room, sh_int door));
                   void delete_help args((HELP_DATA * pHelp));
                   void add_help args((HELP_DATA * pHelp));
                   void fix_area_exits args((AREA_DATA * tarea));
                   void load_area_file
                   args((AREA_DATA * tarea, char *filename));
                   void randomize_exits
                   args((ROOM_INDEX_DATA * room, sh_int maxdir));
                   void make_wizlist args((void));
                   void tail_chain args((void));
                   bool delete_room args((ROOM_INDEX_DATA * room));
                   bool delete_obj args((OBJ_INDEX_DATA * obj));
                   bool delete_mob args((MOB_INDEX_DATA * mob));
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
                   void sort_area args((AREA_DATA * pArea, bool proto));
                   void strdup_printf args((char **pointer, char *fmt,...));
                   void stralloc_printf args((char **pointer, char *fmt,...));
                   void command_printf args((CHAR_DATA * ch, char *fmt,...));
                   size_t mudstrlcpy
                   args((char *dst, const char *src, size_t siz));
                   size_t mudstrlcat
                   args((char *dst, const char *src, size_t siz));
/* build.c */
                   bool can_rmodify
                   args((CHAR_DATA * ch, ROOM_INDEX_DATA * room));
                   bool can_omodify args((CHAR_DATA * ch, OBJ_DATA * obj));
                   bool can_mmodify args((CHAR_DATA * ch, CHAR_DATA * mob));
                   bool can_medit
                   args((CHAR_DATA * ch, MOB_INDEX_DATA * mob));
                   void free_reset args((AREA_DATA * are, RESET_DATA * res));
                   void free_area args((AREA_DATA * are));
                   void assign_area args((CHAR_DATA * ch));
                   EDD *
                   SetRExtra args((ROOM_INDEX_DATA * room, char *keywords));
                   bool DelRExtra
                   args((ROOM_INDEX_DATA * room, char *keywords));
                   EDD * SetOExtra args((OBJ_DATA * obj, char *keywords));
                   bool DelOExtra args((OBJ_DATA * obj, char *keywords));
                   EDD *
                   SetOExtraProto
                   args((OBJ_INDEX_DATA * obj, char *keywords));
                   bool DelOExtraProto
                   args((OBJ_INDEX_DATA * obj, char *keywords));
                   void fold_area
                   args((AREA_DATA * tarea, char *filename, bool install,
                         bool dolog));
                   int get_otype args((char *type));
                   int get_atype args((char *type));
                   int get_aflag args((char *flag));
                   int get_oflag args((char *flag));
                   int get_wflag args((char *flag));
                   RACE_DATA * get_race args((char *string));
                   LANGUAGE_DATA * get_language args((char *string));
                   RACE_DATA * get_race_number args((int number));
/* channels.c */
                   void removename args((char **list, const char *name));
                   void addname args((char **list, const char *name));
                   int hasname args((const char *list, const char *name));
                   const char *getarg
                   args((const char *argument, char *arg, int length));
/* fight.c */
                   int max_fight args((CHAR_DATA * ch));
                   void violence_update args((void));
                   ch_ret multi_hit
                   args((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
                   sh_int ris_damage
                   args((CHAR_DATA * ch, sh_int dam, int ris));
                   ch_ret damage
                   args((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
                         int dt));
                   void update_pos args((CHAR_DATA * victim));
                   void set_fighting
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void stop_fighting args((CHAR_DATA * ch, bool fBoth));
                   void free_fight args((CHAR_DATA * ch));
                   CD * who_fighting args((CHAR_DATA * ch));
                   void check_killer
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void check_attacker
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void death_cry args((CHAR_DATA * ch));
                   void stop_hunting args((CHAR_DATA * ch));
                   void stop_hating args((CHAR_DATA * ch));
                   void stop_fearing args((CHAR_DATA * ch));
                   void start_hunting
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void start_hating
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void start_fearing
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_hunting args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_hating args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_fearing args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_safe args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool is_safe_nm args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool legal_loot args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool check_illegal_pk
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void raw_kill args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool in_arena args((CHAR_DATA * ch));
                   int in_hash_table args((char *str));
/* makeobjs.c */
                   void make_corpse
                   args((CHAR_DATA * ch, CHAR_DATA * killer));
                   void make_blood args((CHAR_DATA * ch));
                   void make_bloodstain args((CHAR_DATA * ch));
                   void make_scraps args((OBJ_DATA * obj));
                   void make_fire
                   args((ROOM_INDEX_DATA * in_room, sh_int timer));
                   OD * make_trap args((int v0, int v1, int v2, int v3));
                   OD * create_money args((int amount));
/* misc.c */
                   void actiondesc
                   args((CHAR_DATA * ch, OBJ_DATA * obj, void *vo));
                   void jedi_checks args((CHAR_DATA * ch));
                   void jedi_bonus args((CHAR_DATA * ch));
                   void sith_penalty args((CHAR_DATA * ch));
/* mud_comm.c */
                   char *mprog_type_to_name args((int type));
/* mud_prog.c */
#ifdef DUNNO_STRSTR
                   char *strstr args((const char *s1, const char *s2));
#endif
                   void mprog_wordlist_check args((char *arg, CHAR_DATA * mob,
                                                   CHAR_DATA * actor,
                                                   OBJ_DATA * object,
                                                   void *vo, int type));
                   void mprog_percent_check
                   args((CHAR_DATA * mob, CHAR_DATA * actor,
                         OBJ_DATA * object, void *vo, int type));
                   void mprog_act_trigger
                   args((char *buf, CHAR_DATA * mob, CHAR_DATA * ch,
                         OBJ_DATA * obj, void *vo));
                   void mprog_bribe_trigger
                   args((CHAR_DATA * mob, CHAR_DATA * ch, int amount));
                   void mprog_entry_trigger args((CHAR_DATA * mob));
                   void mprog_give_trigger
                   args((CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj));
                   void mprog_greet_trigger args((CHAR_DATA * mob));
                   void mprog_fight_trigger
                   args((CHAR_DATA * mob, CHAR_DATA * ch));
                   void mprog_hitprcnt_trigger
                   args((CHAR_DATA * mob, CHAR_DATA * ch));
                   void mprog_death_trigger
                   args((CHAR_DATA * killer, CHAR_DATA * mob));
                   void mprog_random_trigger args((CHAR_DATA * mob));
                   void mprog_speech_trigger
                   args((char *txt, CHAR_DATA * mob));
                   void mprog_script_trigger args((CHAR_DATA * mob));
                   void mprog_hour_trigger args((CHAR_DATA * mob));
                   void mprog_time_trigger args((CHAR_DATA * mob));
                   void progbug args((char *str, CHAR_DATA * mob));
                   void rset_supermob args((ROOM_INDEX_DATA * room));
                   void release_supermob args((void));
                   void mpsleep_update args((void));
/* player.c */
                   void set_title args((CHAR_DATA * ch, char *title));
                   char *convert_newline args((char *str));
/* skills.c */
                   bool check_skill
                   args((CHAR_DATA * ch, char *command, char *argument));
                   void learn_from_success args((CHAR_DATA * ch, int sn));
                   void learn_from_failure args((CHAR_DATA * ch, int sn));
                   bool check_parry
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool check_dodge
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool check_grip args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void disarm args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void trip args((CHAR_DATA * ch, CHAR_DATA * victim));
/* handler.c */
                   void explode args((OBJ_DATA * obj));
                   int get_exp args((CHAR_DATA * ch, int ability));
                   int get_exp_worth args((CHAR_DATA * ch));
                   int exp_level args((sh_int level));
                   sh_int get_trust args((CHAR_DATA * ch));
                   sh_int get_age args((CHAR_DATA * ch));
                   sh_int get_curr_str args((CHAR_DATA * ch));
                   sh_int get_curr_int args((CHAR_DATA * ch));
                   sh_int get_curr_wis args((CHAR_DATA * ch));
                   sh_int get_curr_dex args((CHAR_DATA * ch));
                   sh_int get_curr_con args((CHAR_DATA * ch));
                   sh_int get_curr_cha args((CHAR_DATA * ch));
                   sh_int get_curr_lck args((CHAR_DATA * ch));
                   sh_int get_curr_frc args((CHAR_DATA * ch));
                   sh_int max_languages args((CHAR_DATA * ch));
                   bool can_take_proto args((CHAR_DATA * ch));
                   int can_carry_n args((CHAR_DATA * ch));
                   int can_carry_w args((CHAR_DATA * ch));
                   bool is_name args((const char *str, char *namelist));
                   bool is_name_prefix
                   args((const char *str, char *namelist));
                   bool nifty_is_name args((char *str, char *namelist));
                   bool nifty_is_name_prefix
                   args((char *str, char *namelist));
                   void affect_modify
                   args((CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd));
                   void affect_to_char
                   args((CHAR_DATA * ch, AFFECT_DATA * paf));
                   void affect_remove
                   args((CHAR_DATA * ch, AFFECT_DATA * paf));
                   void affect_strip args((CHAR_DATA * ch, int sn));
                   bool is_affected args((CHAR_DATA * ch, int sn));
                   void affect_join args((CHAR_DATA * ch, AFFECT_DATA * paf));
                   void char_from_room args((CHAR_DATA * ch));
                   void char_to_room
                   args((CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex));
                   OD * obj_to_char args((OBJ_DATA * obj, CHAR_DATA * ch));
                   void obj_from_char args((OBJ_DATA * obj));
                   int apply_ac args((OBJ_DATA * obj, int iWear));
                   OD * get_eq_char args((CHAR_DATA * ch, int iWear));
                   void equip_char
                   args((CHAR_DATA * ch, OBJ_DATA * obj, int iWear));
                   void unequip_char args((CHAR_DATA * ch, OBJ_DATA * obj));
                   int count_obj_list
                   args((OBJ_INDEX_DATA * obj, OBJ_DATA * list));
                   void obj_from_room args((OBJ_DATA * obj));
                   OD *
                   obj_to_room
                   args((OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex));
                   OD * obj_to_obj args((OBJ_DATA * obj, OBJ_DATA * obj_to));
                   void obj_from_obj args((OBJ_DATA * obj));
                   void extract_obj args((OBJ_DATA * obj));
                   void extract_exit
                   args((ROOM_INDEX_DATA * room, EXIT_DATA * pexit));
                   void extract_room args((ROOM_INDEX_DATA * room));
                   void clean_room args((ROOM_INDEX_DATA * room));
                   void clean_obj args((OBJ_INDEX_DATA * obj));
                   void clean_mob args((MOB_INDEX_DATA * mob));
                   void clean_resets args((AREA_DATA * tarea));
                   void extract_char args((CHAR_DATA * ch, bool fPull));
                   CD * get_char_room args((CHAR_DATA * ch, char *argument));
                   CD * get_char_world args((CHAR_DATA * ch, char *argument));
                   CD * get_char_world_nocheck args((char *argument));
                   OD * get_obj_type args((OBJ_INDEX_DATA * pObjIndexData));
                   OD *
                   get_obj_list
                   args((CHAR_DATA * ch, char *argument, OBJ_DATA * list));
                   OD *
                   get_obj_list_rev
                   args((CHAR_DATA * ch, char *argument, OBJ_DATA * list));
                   OD * get_obj_carry args((CHAR_DATA * ch, char *argument));
                   OD *
                   get_obj_carry_type
                   args((CHAR_DATA * ch, char *argument, int type));
                   OD * get_obj_wear args((CHAR_DATA * ch, char *argument));
                   OD * get_obj_here args((CHAR_DATA * ch, char *argument));
                   OD * get_obj_world args((CHAR_DATA * ch, char *argument));
                   int get_obj_number args((OBJ_DATA * obj));
                   int get_obj_weight args((OBJ_DATA * obj));
                   bool room_is_dark args((ROOM_INDEX_DATA * pRoomIndex));
                   bool room_is_private
                   args((CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex));
                   bool can_see args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool can_see_ooc
                   args((CHAR_DATA * ch, CHAR_DATA * victim));
                   bool can_see_obj args((CHAR_DATA * ch, OBJ_DATA * obj));
                   bool can_drop_obj args((CHAR_DATA * ch, OBJ_DATA * obj));
                   char *item_type_name args((OBJ_DATA * obj));
                   char *affect_loc_name args((int location));
                   char *affect_bit_name args((int vector));
                   char *extra_bit_name args((int extra_flags));
                   char *magic_bit_name args((int magic_flags));
                   ch_ret check_for_trap
                   args((CHAR_DATA * ch, OBJ_DATA * obj, int flag));
                   ch_ret check_room_for_traps
                   args((CHAR_DATA * ch, int flag));
                   bool is_trapped args((OBJ_DATA * obj));
                   OD * get_trap args((OBJ_DATA * obj));
                   ch_ret spring_trap args((CHAR_DATA * ch, OBJ_DATA * obj));
                   void name_stamp_stats args((CHAR_DATA * ch));
                   void fix_char args((CHAR_DATA * ch));
                   void showaffect args((CHAR_DATA * ch, AFFECT_DATA * paf));
                   void set_cur_obj args((OBJ_DATA * obj));
                   bool obj_extracted args((OBJ_DATA * obj));
                   void queue_extracted_obj args((OBJ_DATA * obj));
                   void clean_obj_queue args((void));
                   bool char_died args((CHAR_DATA * ch));
                   void queue_extracted_char
                   args((CHAR_DATA * ch, bool extract));
                   void clean_char_queue args((void));
                   void add_timer
                   args((CHAR_DATA * ch, sh_int type, sh_int count,
                         DO_FUN * fun, int value));
                   TIMER * get_timerptr args((CHAR_DATA * ch, sh_int type));
                   sh_int get_timer args((CHAR_DATA * ch, sh_int type));
                   void extract_timer args((CHAR_DATA * ch, TIMER * timer));
                   void remove_timer args((CHAR_DATA * ch, sh_int type));
                   bool in_soft_range
                   args((CHAR_DATA * ch, AREA_DATA * tarea));
                   bool in_hard_range
                   args((CHAR_DATA * ch, AREA_DATA * tarea));
                   bool chance args((CHAR_DATA * ch, sh_int percent));
                   bool chance_attrib
                   args((CHAR_DATA * ch, sh_int percent, sh_int attrib));
                   OD * clone_object args((OBJ_DATA * obj));
                   void split_obj args((OBJ_DATA * obj, int num));
                   void separate_obj args((OBJ_DATA * obj));
                   bool empty_obj
                   args((OBJ_DATA * obj, OBJ_DATA * destobj,
                         ROOM_INDEX_DATA * destroom));
                   OD *
                   find_obj
                   args((CHAR_DATA * ch, char *argument, bool carryonly));
                   bool ms_find_obj args((CHAR_DATA * ch));
                   void worsen_mental_state args((CHAR_DATA * ch, int mod));
                   void better_mental_state args((CHAR_DATA * ch, int mod));
                   void boost_economy args((AREA_DATA * tarea, int gold));
                   void lower_economy args((AREA_DATA * tarea, int gold));
                   void economize_mobgold args((CHAR_DATA * mob));
                   bool economy_has args((AREA_DATA * tarea, int gold));
                   void add_kill args((CHAR_DATA * ch, CHAR_DATA * mob));
                   int times_killed args((CHAR_DATA * ch, CHAR_DATA * mob));
                   char *num_punct args((int foo));
                   char *num_punct_long args((long int foo));
                   int count_users args((OBJ_DATA * obj));
                   char *smash_color args((char *str));
                   char *full_color args((char *str));
                   char *smash_space args((const char *str));
                   void add_request
                   args((CHAR_DATA * ch, CHAR_DATA * victim, char *argument,
                         char *syntax));
/* interp.c */
                   bool check_pos args((CHAR_DATA * ch, sh_int position));
                   void interpret args((CHAR_DATA * ch, char *argument));
                   bool is_number args((char *arg));
                   int number_argument args((char *argument, char *arg));
                   char *one_argument args((char *argument, char *arg_first));
                   char *one_argument2
                   args((char *argument, char *arg_first));
                   ST * find_social args((char *command));
                   ST * find_xsocial args((char *command));
                   CMDTYPE * find_command args((char *command));
                   void hash_commands args((void));
                   void start_timer args((struct timeval * _stime));
                   time_t end_timer args((struct timeval * _stime));
                   void send_timer
                   args((struct timerset * vtime, CHAR_DATA * ch));
                   void update_userec
                   args((struct timeval * time_used,
                         struct timerset * userec));
                   bool check_command
                   args((CHAR_DATA * ch, CMDTYPE * command));
/* magic.c */
                   bool process_spell_components
                   args((CHAR_DATA * ch, int sn));
                   int ch_slookup args((CHAR_DATA * ch, const char *name));
                   int find_spell
                   args((CHAR_DATA * ch, const char *name, bool know));
                   int find_skill
                   args((CHAR_DATA * ch, const char *name, bool know));
                   int find_weapon
                   args((CHAR_DATA * ch, const char *name, bool know));
                   int find_tongue
                   args((CHAR_DATA * ch, const char *name, bool know));
                   int skill_lookup args((const char *name));
                   int herb_lookup args((const char *name));
                   int personal_lookup
                   args((CHAR_DATA * ch, const char *name));
                   int slot_lookup args((int slot));
                   int bsearch_skill
                   args((const char *name, int first, int top));
                   int bsearch_skill_exact
                   args((const char *name, int first, int top));
                   bool saves_poison_death
                   args((int level, CHAR_DATA * victim));
                   bool saves_wand args((int level, CHAR_DATA * victim));
                   bool saves_para_petri
                   args((int level, CHAR_DATA * victim));
                   bool saves_breath args((int level, CHAR_DATA * victim));
                   bool saves_spell_staff
                   args((int level, CHAR_DATA * victim));
                   ch_ret obj_cast_spell
                   args((int sn, int level, CHAR_DATA * ch,
                         CHAR_DATA * victim, OBJ_DATA * obj));
                   int dice_parse
                   args((CHAR_DATA * ch, int level, char *_exp));
                   SK * get_skilltype args((int sn));
/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	(sh_int) 0
#define OS_CORPSE	(sh_int) 1
                   void save_char_obj args((CHAR_DATA * ch));
                   void save_clone args((CHAR_DATA * ch));
                   bool load_char_obj
                   args((DESCRIPTOR_DATA * d, char *name, bool preload,
                         bool copyover));
                   void set_alarm args((long seconds));
                   void requip_char args((CHAR_DATA * ch));
                   void fwrite_obj
                   args((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest,
                         sh_int os_type, bool hotboot));
                   void fread_obj
                   args((CHAR_DATA * ch, FILE * fp, sh_int os_type));
                   void de_equip_char args((CHAR_DATA * ch));
                   void re_equip_char args((CHAR_DATA * ch));
                   void save_home args((CHAR_DATA * ch));
                   void load_home args((CHAR_DATA * ch));
/* shops.c */
                   CD * find_keeper args((CHAR_DATA * ch));
/* special.c */
                   SF * spec_lookup args((char *name));
/* tables.c */
                   int get_skill args((char *skilltype));
                   char *spell_name args((SPELL_FUN * spell));
                   char *skill_name args((DO_FUN * skill));
                   void load_skill_table args((void));
                   void save_skill_table args((void));
                   void sort_skill_table args((void));
                   void load_socials args((void));
                   void save_socials args((void));
                   void load_commands args((void));
                   void save_commands args((void));
                   SPELL_FUN * spell_function args((char *name));
                   DO_FUN * skill_function args((char *name));
                   void load_herb_table args((void));
                   void save_herb_table args((void));
/* track.c */
                   void found_prey args((CHAR_DATA * ch, CHAR_DATA * victim));
                   void hunt_victim args((CHAR_DATA * ch));
/* update.c */
                   void advance_level args((CHAR_DATA * ch, int ability));
                   void gain_exp
                   args((CHAR_DATA * ch, int gain, int ability));
                   void gain_exp_new
                   args((CHAR_DATA * ch, int gain, int ability,
                         bool outtext));
                   void gain_condition
                   args((CHAR_DATA * ch, int iCond, int value));
                   void update_handler args((void));
                   void reboot_check args((time_t reset));
#if 0
                   void reboot_check args((char *arg));
#endif
                   void auction_update args((void));
                   void remove_portal args((OBJ_DATA * portal));
                   int max_level(CHAR_DATA * ch, int ability);
/* hashstr.c */
                   char *str_alloc args((char *str));
                   char *quick_link args((char *str));
                   int str_free args((char *str));
                   int allocated_strings args((void));
                   void show_hash args((int count));
                   char *hash_stats args((void));
                   char *check_hash args((char *str));
                   void hash_dump args((int hash));
                   void show_high_hash args((int top));
/* newscore.c */
                   void web_colourconv(char *buffer, const char *txt);

/* vendor.c*/
                   void fwrite_vendor args((FILE * fp, CHAR_DATA * mob));
                   CHAR_DATA * fread_vendor args((FILE * fp));
                   void load_vendors args((void));
                   void save_vendor args((CHAR_DATA * ch));
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
 * mudprograms stuff
 */
                   extern CHAR_DATA * supermob;
                   void oprog_speech_trigger(char *txt, CHAR_DATA * ch);
                   void oprog_random_trigger(OBJ_DATA * obj);
                   void oprog_wear_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   bool oprog_use_trigger(CHAR_DATA * ch, OBJ_DATA * obj,
                                          CHAR_DATA * vict, OBJ_DATA * targ,
                                          void *vo);
                   void oprog_remove_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_sac_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_damage_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_repair_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_drop_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_zap_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   char *oprog_type_to_name(int type);
/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
                   void oprog_greet_trigger(CHAR_DATA * ch);
                   void oprog_get_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_examine_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_pull_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
                   void oprog_push_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
/* mud prog defines */
#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           BV00
#define SPEECH_PROG        BV01
#define RAND_PROG          BV02
#define FIGHT_PROG         BV03
#define RFIGHT_PROG        BV03
#define DEATH_PROG         BV04
#define RDEATH_PROG        BV04
#define HITPRCNT_PROG      BV05
#define ENTRY_PROG         BV06
#define ENTER_PROG         BV06
#define GREET_PROG         BV07
#define RGREET_PROG	   BV07
#define OGREET_PROG        BV07
#define ALL_GREET_PROG	   BV08
#define GIVE_PROG	   BV09
#define BRIBE_PROG	   BV10
#define HOUR_PROG	   BV11
#define TIME_PROG	   BV12
#define WEAR_PROG          BV13
#define REMOVE_PROG        BV14
#define SAC_PROG           BV15
#define LOOK_PROG          BV16
#define EXA_PROG           BV17
#define ZAP_PROG           BV18
#define GET_PROG 	   BV19
#define DROP_PROG	   BV20
#define DAMAGE_PROG	   BV21
#define REPAIR_PROG	   BV22
#define RANDIW_PROG	   BV23
#define SPEECHIW_PROG	   BV24
#define PULL_PROG	   BV25
#define PUSH_PROG	   BV26
#define SLEEP_PROG         BV27
#define REST_PROG          BV28
#define LEAVE_PROG         BV29
#define SCRIPT_PROG	   BV30
#define USE_PROG           BV31
                   void rprog_leave_trigger(CHAR_DATA * ch);
                   void rprog_enter_trigger(CHAR_DATA * ch);
                   void rprog_sleep_trigger(CHAR_DATA * ch);
                   void rprog_rest_trigger(CHAR_DATA * ch);
                   void rprog_rfight_trigger(CHAR_DATA * ch);
                   void rprog_death_trigger(CHAR_DATA * killer,
                                            CHAR_DATA * ch);
                   void rprog_speech_trigger(char *txt, CHAR_DATA * ch);
                   void rprog_random_trigger(CHAR_DATA * ch);
                   void rprog_time_trigger(CHAR_DATA * ch);
                   void rprog_hour_trigger(CHAR_DATA * ch);
                   char *rprog_type_to_name(int type);
#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
                   void oprog_act_trigger(char *buf, OBJ_DATA * mobj,
                                          CHAR_DATA * ch, OBJ_DATA * obj,
                                          void *vo);
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
                   void rprog_act_trigger(char *buf, ROOM_INDEX_DATA * room,
                                          CHAR_DATA * ch, OBJ_DATA * obj,
                                          void *vo);
#endif
#endif
