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
 *                              Mud Extension Protocol Module                            *
 *****************************************************************************************
 *                $Id: mxp.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

/* strings */
#define MXP_SECURE "\x1B[1"
#define MXP_BEG "\x03"  /* becomes < */
#define MXP_END "\x04"  /* becomes > */
#define MXP_AMP "\x05"  /* becomes & */

/* characters */

#define MXP_BEGc '\x03' /* becomes < */
#define MXP_ENDc '\x04' /* becomes > */
#define MXP_AMPc '\x05' /* becomes & */

/* constructs an MXP tag with < and > around it */

#define MXPTAG(arg) MXP_BEG arg MXP_END

#define ESC "\x1B"  /* esc character */

#define MXPMODE(arg) ESC "[" #arg "z"

/* flags for show_list_to_char */

enum
{
        eItemNothing,   /* item is not readily accessible */
        eItemGet,   /* item on ground */
        eItemDrop,  /* item in inventory */
        eItemBid    /* auction item */
};

#define  TELOPT_MXP        '\x5B'
extern const unsigned char will_mxp_str[];
extern const unsigned char start_mxp_str[];
extern const unsigned char do_mxp_str[];
extern const unsigned char dont_mxp_str[];

/* DECLARE_DO_FUN(do_mxp); */
void      convert_mxp_tags
args((DESCRIPTOR_DATA * d, char *dest, const char *src, int length));
int count_mxp_tags args((DESCRIPTOR_DATA * d, const char *txt, int length));
void send_mxp_stylesheet args((DESCRIPTOR_DATA * d));

/*
 void free_mxpobj_cmds( void );
 */

#define MXP_STYLESHEET_FILE     "../system/mxp.style"

#define IS_MXP(ch) ( (ch) && IS_SET((ch)->act, PLR_MXP) && (ch)->desc && (ch)->desc->mxp_detected == TRUE )
