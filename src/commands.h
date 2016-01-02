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
 *                                SWR Command Definitions                                *
 ****************************************************************************************/

/* Only need to define functions that other functions call */

#define DECLARE_DO_FUN( fun ) CMDF (fun)( CHAR_DATA * ch, char * argument)
#define DECLARE_SPELL_FUN( fun ) SPELLF (fun)( int sn, int level, CHAR_DATA *ch, void *vo )

DECLARE_DO_FUN(skill_notfound);

DECLARE_DO_FUN(do_at);
DECLARE_DO_FUN(do_auction);
DECLARE_DO_FUN(do_board);
DECLARE_DO_FUN(do_boardlook);
DECLARE_DO_FUN(do_cedit);
DECLARE_DO_FUN(do_closehatch);
DECLARE_DO_FUN(do_command);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_glance);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_hail);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_leaveship);
DECLARE_DO_FUN(do_loadarea);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_memory);
DECLARE_DO_FUN(do_openhatch);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_restoretime);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_revert);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_quaff);
DECLARE_DO_FUN(do_takedrug);
DECLARE_DO_FUN(do_oldscore);
DECLARE_DO_FUN(do_rreset);
DECLARE_DO_FUN(do_reset);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_hide);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_setblaster);
DECLARE_DO_FUN(do_pick);
DECLARE_DO_FUN(do_makeshop);
DECLARE_DO_FUN(do_redit);
DECLARE_DO_FUN(do_backup);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_comment);
DECLARE_DO_FUN(do_affected);
DECLARE_DO_FUN(do_immscore);
DECLARE_DO_FUN(do_who);
DECLARE_DO_FUN(do_train);
DECLARE_DO_FUN(do_savearea);

/*
 * Fight Commands
 */
DECLARE_DO_FUN(do_bite);
DECLARE_DO_FUN(do_claw);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_punch);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_sting);
DECLARE_DO_FUN(do_tail);



/* MXP */
DECLARE_DO_FUN(do_mxp);

/* Spells */
DECLARE_SPELL_FUN(spell_notfound);
DECLARE_SPELL_FUN(spell_smaug);
DECLARE_SPELL_FUN(spell_null);
DECLARE_SPELL_FUN(spell_fireball);
DECLARE_SPELL_FUN(spell_lightning_bolt);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_cure_blindness);
