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
 *                           STAR WARS UNKNOWN REGIONS                                   *
 *---------------------------------------------------------------------------------------*
 * Star Wars Unknown Regions Code Addtions and changes from the SWR Code                 *
 * copyright (c) 2000 by Gavin Mogan                                                     *
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
 *                $Id: restore.h 1330 2005-12-05 03:23:24Z halkeye $                *
 *****************************************************************************************
 *                                 Restores V1.0 header information			             *
 *****************************************************************************************/

/* Capability to create, edit and delete restore messages added to original code
   by Gavin(halkeye@halkeye.net) 5-26-2000 */
#define RESTORE_ADDON

typedef struct restore_data RESTORE_DATA;

extern RESTORE_DATA *first_restore;
extern RESTORE_DATA *last_restore;

/* Improved data structure for online restore editing - Gavin 5-26-2000 */
struct restore_data
{
        RESTORE_DATA *next;
        RESTORE_DATA *prev;
        char     *owner;
        char     *type;
        char     *cmsg;
        char     *vmsg;
        char     *rmsg;
        int       color;
        float     boost;
        int       flags;
};

#define RESTORE_FILE		"restore.dat"   /* Restore data file for online editing - Gavin 5-26-2000 */
#define RESTORE_INTERVAL 21600

/* Maxsrestoretypes variable - 50 should be WAY more than enough */
#define MAX_RESTORE_TYPES		50

void load_restores args((void));
