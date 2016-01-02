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
 *                                SWR Change list module                                 *
 ****************************************************************************************/
/***************************************************************************
 *  Shadow Wars: the Revenge of Kalos                                      *
 ---------------------------------------------------------------------------
 *  You are allowd to change and modify this code, you may NOT take out    *
 *  the helpfile 'CREATORS', and you must leave in the Greeting,           *
 *  Shadow Wars: the Revenge of Kalos created by Xkilla and Sillviant      *
 *  (c)2001 Xkilla 'Ryan Waliany' (ryan@scv.net)                           *
 *  Sillviant 'Matt Burnett' (sillviant@charter.net)                       *
 ---------------------------------------------------------------------------
 *  Changes snippet by: Xkilla. This is a snippet, you may modify it, but  *
 *  you must leave in Xkilla's credit.                                     *
 ***************************************************************************/

#define CHANGES_SNIPPET

#define CHANGES_FILE	SYSTEM_DIR "changes.dat"
typedef struct changes_data CHANGE_DATA;

/* changes.c */
int num_changes args((void));
void save_changes args((void));

void load_changes args((void));
void delete_change args((int num));

struct changes_data
{

        char     *change;
        char     *coder;
        char     *date;
        time_t    mudtime;
        bool      immchange;
};

/* extern struct  changes_data * changes_table; */
extern int maxChanges;
extern int immortal_changes;

#define CHANGES_VERSION 0
extern CHANGE_DATA *changes_table;
