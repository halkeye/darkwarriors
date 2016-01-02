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
 * This module allows users to attach different characters to a single account and share *
 * simple things such as aliases, role playing points, etc                               *
 *****************************************************************************************
 *                $Id: account.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

typedef struct account_data ACCOUNT_DATA;

extern ACCOUNT_DATA *first_account;
extern ACCOUNT_DATA *last_account;

#define ACCOUNT_DIR "../account/"

#define  ACCOUNT_SOUND ACCOUNT_MSP
#define  ACCOUNT_MSP BV00
#define  ACCOUNT_MXP BV01

struct account_data
{
        ACCOUNT_DATA *prev;
        ACCOUNT_DATA *next;
        struct alias_data *first_alias;
        struct alias_data *last_alias;
        char     *name;
        char     *password;
        char     *character[MAX_CHARACTERS];
        int       rppoints;
        int       rpcurrent;
        int       qpoints;
        int       inuse;    /* To prevent deleting one that is active */
        int       flags;
        struct note_data *comments;
        
		char     *email;
};

ACCOUNT_DATA *load_account args((char *name));
ACCOUNT_DATA *create_account args((void));
void save_account args((ACCOUNT_DATA * account));
bool add_to_account args((ACCOUNT_DATA * account, CHAR_DATA * ch));
bool del_from_account args((ACCOUNT_DATA * account, CHAR_DATA * ch));
void show_account_characters args((DESCRIPTOR_DATA * d));
void free_account args((ACCOUNT_DATA * account));
void fread_account args((ACCOUNT_DATA * account, FILE * fp));

#endif
