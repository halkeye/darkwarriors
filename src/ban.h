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
 * This module was originally for SMAUG coded by Shaddai, but has since been modified for* 
 * Dark Warriors and SWR.                                                                *
 *****************************************************************************************
 *                $Id: ban.h 1330 2005-12-05 03:23:24Z halkeye $                  *
 ****************************************************************************************/

#ifndef _BAN_H_
#define _BAN_H_

typedef struct ban_data BAN_DATA;
typedef struct reserve_data RESERVE_DATA;

extern RESERVE_DATA *first_reserved;
extern RESERVE_DATA *last_reserved;

#define RESERVED_LIST	"reserved.lst"  /* List of reserved names   */

/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct reserve_data
{
        RESERVE_DATA *next;
        RESERVE_DATA *prev;
        char     *name;
};

/*
 * Ban Types --- Shaddai
 */
#define BAN_SITE        1
#define BAN_CLASS       2
#define BAN_RACE        3
#define BAN_WARN        -1

/*
 * Site ban structure.
 */
struct ban_data
{
        BAN_DATA *next;
        BAN_DATA *prev;
        char     *name; /* Name of site/class/race banned */
        char     *user; /* Name of user from site */
        char     *note; /* Why it was banned */
        char     *ban_by;   /* Who banned this site */
        char     *ban_time; /* Time it was banned */
        int       flag; /* Class or Race number */
        int       unban_date;   /* When ban expires */
        sh_int    duration; /* How long it is banned for */
        sh_int    level;    /* Level that is banned */
        bool      warn; /* Echo on warn channel */
        bool      prefix;   /* Use of *site */
        bool      suffix;   /* Use of site* */
};

extern BAN_DATA *first_ban;
extern BAN_DATA *last_ban;
extern BAN_DATA *first_ban_class;
extern BAN_DATA *last_ban_class;
extern BAN_DATA *first_ban_race;
extern BAN_DATA *last_ban_race;

/* ban.c */
int       add_ban
args((CHAR_DATA * ch, char *arg1, char *arg2, int time, int type));
void show_bans args((CHAR_DATA * ch, int type));
void save_banlist args((void));
void load_banlist args((void));
bool check_total_bans args((DESCRIPTOR_DATA * d));
bool check_bans args((CHAR_DATA * ch, int type));

/* reserve */
void sort_reserved args((RESERVE_DATA * pRes));
bool is_reserved_name args((char *name));
void load_reserved args((void));
#endif
