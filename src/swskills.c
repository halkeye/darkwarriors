/* vim: ts=8 et ft=c sw=8
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
 *                $Id: swskills.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bounty.h"
#include "races.h"
#include "installations.h"
#include "space2.h"


ch_ret one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
int       xp_compute(CHAR_DATA * ch, CHAR_DATA * victim);
ROOM_INDEX_DATA *generate_exit(ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit);
int       ris_save(CHAR_DATA * ch, int percentage, int ris);
CHAR_DATA *get_char_room_mp(CHAR_DATA * ch, char *argument);
bool remove_obj args((CHAR_DATA * ch, int iWear, bool fReplace));
void      wear_obj
args((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace, sh_int wear_bit));

extern int top_affect;
extern char *const crystal_table[8];

CMDF do_makeblade(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, charge = 0;
        bool      checktool, checkdura, checkbatt, checkoven;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;
        AFFECT_DATA *paf2;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeblade <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;

                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a vibro-blade.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need something to make it out of.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your blade.\n\r",
                                 ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the metal.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeblade]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a vibroblade.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makeblade, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeblade);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makeblade]);
        vnum = 10422;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;
        checkbatt = FALSE;
        checkoven = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        charge = UMAX(5, obj->value[0]);
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeblade]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura)
            || (!checkbatt) || (!checkoven))
        {
                send_to_char
                        ("&RYou activate your newly created vibroblade.\n\r",
                         ch);
                send_to_char
                        ("&RIt hums softly for a few seconds then begins to shake violently.\n\r",
                         ch);
                send_to_char
                        ("&RIt finally shatters breaking apart into a dozen pieces.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeblade);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 3;
        stralloc_printf(&obj->name, "%s vibro-blade blade", smash_color(arg));
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was left here.", MSL);
        obj->description = STRALLOC(buf);
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("hitroll");
        paf2->modifier = -2;
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;
        obj->value[1] = (int) (level / 20 + 10);    /* min dmg  */
        obj->value[2] = (int) (level / 10 + 20);    /* max dmg */
        obj->value[3] = WEAPON_VIBRO_BLADE;
        obj->value[4] = charge;
        obj->value[5] = charge;
        obj->cost = obj->value[2] * 10;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created blade.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a vibro-blade.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 200,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makeblade);
}

CMDF do_makefurniture(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, type;
        bool      checksew, checkfab, checktool, checkdura;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;

        argument = one_argument(argument, arg);
        mudstrlcpy(arg2, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makefurniture <type> <Description>\n\r&w",
                                 ch);
                        return;
                }

                checksew = FALSE;
                checkfab = FALSE;
                checktool = FALSE;
                checkdura = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_FABRIC)
                                checkfab = TRUE;
                        if (obj->item_type == ITEM_THREAD)
                                checksew = TRUE;
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST
                            || obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;

                }

                if (!checkfab)
                {
                        send_to_char
                                ("&RYou need some sort of fabric or material.\n\r",
                                 ch);
                        return;
                }

                if (!checksew)
                {
                        send_to_char
                                ("&RYou need a needle and some thread.\n\r",
                                 ch);
                        return;
                }

                if (!checktool)
                {
                        send_to_char("&RYou need a toolkit.\n\r", ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char("&RYou need something for support.\n\r",
                                     ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makefurniture]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating a piece of furniture.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s sewing kit and some material and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makefurniture, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makefurniture);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned
                                                    [gsn_makefurniture]);
        vnum = 10433;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }



        checksew = FALSE;
        checkfab = FALSE;
        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_THREAD)
                        checksew = TRUE;
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
                {
                        checkfab = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if ((obj->item_type == ITEM_DURAPLAST
                     || obj->item_type == ITEM_DURASTEEL)
                    && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

        }
        obj = NULL;

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makefurniture]);

        if (number_percent() > percentage * 2 || (!checkfab) || (!checksew)
            || (!checkdura) || (!checktool))
        {
                send_to_char
                        ("&RYou look at your newly created piece of furniture.\n\r",
                         ch);
                send_to_char
                        ("&RIt suddenly dawns upon you that you have created the most useless\n\r",
                         ch);
                send_to_char
                        ("&Rthing you've ever seen. You quickly hide your mistake...\n\r",
                         ch);
                learn_from_failure(ch, gsn_makefurniture);
                return;
        }



        type = 0;
        if (!str_cmp(arg, "box"))
        {
                type = 3;
        }
        if (!str_cmp(arg, "chair"))
        {
                type = 2;
        }
        if (!str_cmp(arg, "table"))
        {
                type = 1;
        }

        obj = create_object(pObjIndex, level);
        obj->item_type = ITEM_FURNITURE;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = 1;
        obj->weight = 200;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg2, MIL);
        mudstrlcat(buf, " furniture", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg2, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        obj->description = STRALLOC(buf);
        obj->value[0] = (level / 20);
        obj->value[2] = type;
        obj->cost = level * 10;

        obj = obj_to_char(obj, ch);


        send_to_char
                ("&GYou finish your work and look at your new piece of furniture.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes creating some furniture.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 25,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makefurniture);
}



CMDF do_makebowcaster(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, charge = 0;
        bool      checktool, checkdura, checkbatt, checkoven, checkcond,
                checkcirc;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf2;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makebowcaster <name>\n\r&w",
                                     ch);
                        return;
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checkcond = TRUE;

                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a bowcaster.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need some durasteel to make it out of.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your blade.\n\r",
                                 ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the metal.\n\r",
                                 ch);
                        return;
                }
                if (!checkcirc)
                {
                        send_to_char
                                ("&RYou need a small circuit board to control the firing mechanism.\n\r",
                                 ch);
                        return;
                }

                if (!checkcond)
                {
                        send_to_char
                                ("&RYou still need a small superconductor.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makebowcaster]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a bowcaster.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makebowcaster, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makebowcaster);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned
                                                    [gsn_makebowcaster]);
        vnum = 10419;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;
        checkbatt = FALSE;
        checkoven = FALSE;
        checkcond = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        charge = UMAX(5, obj->value[0]);
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR
                    && checkcond == FALSE)
                {
                        checkcond = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }

        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makebowcaster]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura)
            || (!checkbatt) || (!checkoven) || (!checkcond) || (!checkcirc))
        {
                send_to_char
                        ("&RYou activate your newly created bowcaster.\n\r",
                         ch);
                send_to_char("&RIt activates, the gauges fill up..\n\r", ch);
                send_to_char
                        ("&RYou test the weapon, and it explodes in your hands.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makebowcaster);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 10;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " bowcaster", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was left here.", MSL);
        obj->description = STRALLOC(buf);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = get_atype("damroll");
        paf->modifier = +3;
        paf->bitvector = 0;
        paf->next = NULL;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("hitroll");
        paf2->modifier = +1;
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;
        obj->value[1] = (int) (level / 10 + 10);    /* min dmg  */
        obj->value[2] = (int) (level / 5 + 25); /* max dmg */
        obj->value[3] = WEAPON_BOWCASTER;
        obj->value[4] = charge;
        obj->value[5] = charge;
        obj->cost = obj->value[2] * 10;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created bowcaster.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a bowcaster.", ch, NULL, argument,
            TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 200,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makebowcaster);
}


CMDF do_makeblaster(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage;
        bool      checktool, checkdura, checkbatt, checkoven, checkcond,
                checkcirc, checkammo;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum, power, scope, ammo, canister;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf2;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeblaster <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST)
                                checkdura = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checkcond = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a blaster.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need something to make it out of.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your blaster.\n\r",
                                 ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the plastics.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char
                                ("&RYou need a small circuit board to control the firing mechanism.\n\r",
                                 ch);
                        return;
                }

                if (!checkcond)
                {
                        send_to_char
                                ("&RYou still need a small superconductor.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeblaster]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a blaster.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makeblaster, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeblaster);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makeblaster]);
        vnum = 10420;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checkammo = FALSE;
        checktool = FALSE;
        checkdura = FALSE;
        checkbatt = FALSE;
        checkoven = FALSE;
        checkcond = FALSE;
        checkcirc = FALSE;
        power = 0;
        scope = 0;
        ammo = 0;
        canister = 0;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_DURAPLAST && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_AMMO && checkammo == FALSE)
                {
                        ammo = obj->value[0];
                        checkammo = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_LENS && scope == 0)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        scope++;
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR && power < 2)
                {
                        power++;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcond = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        checkcirc = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_CANISTER && canister == 0)
                {
                        canister = obj->value[0] + 5;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeblaster]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura)
            || (!checkbatt) || (!checkoven) || (!checkcond) || (!checkcirc))
        {
                send_to_char
                        ("&RYou hold up your new blaster and aim at a leftover piece of plastic.\n\r",
                         ch);
                send_to_char
                        ("&RYou slowly squeeze the trigger hoping for the best...\n\r",
                         ch);
                send_to_char
                        ("&RYour blaster backfires destroying your weapon and burning your hand.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeblaster);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 2 + level / 10;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, " blaster", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = get_atype("hitroll");
        paf->modifier = URANGE(0, 1 + scope, level / 30);
        paf->bitvector = 0;
        paf->next = NULL;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("damroll");
        paf2->modifier = URANGE(0, power, level / 30);
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;  /* condition  */
        obj->value[1] = (int) ((level / 10 + 15) + (canister * 2)); /* min dmg  */
        obj->value[2] = (int) ((level / 5 + 25) + (canister * 2));  /* max dmg  */
        obj->value[3] = WEAPON_BLASTER;
        obj->value[4] = ammo;
        obj->value[5] = 2000;
        obj->cost = obj->value[2] * 50;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created blaster.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new blaster.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makeblaster);
}

CMDF do_makelightsaber(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        bool      checktool, checkdura, checkbatt,
                checkoven, checkcond, checkcirc, checklens, checkgems,
                checkmirr;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum, level, gems, charge, gemtype;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf2;

        if (ch->pcdata->learned[gsn_lightsaber_crafting] < 1)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makelightsaber <name>\n\r&w",
                                     ch);
                        return;
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;
                checklens = FALSE;
                checkgems = FALSE;
                checkmirr = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_SAFE)
                    || !xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
                {
                        send_to_char
                                ("&RYou need to be in a quiet peaceful place to craft a lightsaber.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_LENS)
                                checklens = TRUE;
                        if (obj->item_type == ITEM_CRYSTAL)
                                checkgems = TRUE;
                        if (obj->item_type == ITEM_MIRROR)
                                checkmirr = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST
                            || obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checkcond = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a lightsaber.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need something to make it out of.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your lightsaber.\n\r",
                                 ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat and shape the components.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char("&RYou need a small circuit board.\n\r",
                                     ch);
                        return;
                }

                if (!checkcond)
                {
                        send_to_char
                                ("&RYou still need a small superconductor for your lightsaber.\n\r",
                                 ch);
                        return;
                }

                if (!checklens)
                {
                        send_to_char
                                ("&RYou still need a lens to focus the beam.\n\r",
                                 ch);
                        return;
                }

                if (!checkgems)
                {
                        send_to_char
                                ("&RLightsabers require 1 to 3 gems to work properly.\n\r",
                                 ch);
                        return;
                }

                if (!checkmirr)
                {
                        send_to_char
                                ("&RYou need a high intesity reflective cup to create a lightsaber.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->
                                 learned[gsn_lightsaber_crafting]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a lightsaber.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makelightsaber, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_lightsaber_crafting);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned
                                                    [gsn_lightsaber_crafting]);
        vnum = 10421;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;
        checkbatt = FALSE;
        checkoven = FALSE;
        checkcond = FALSE;
        checkcirc = FALSE;
        checklens = FALSE;
        checkgems = FALSE;
        checkmirr = FALSE;
        gems = 0;
        charge = 0;
        gemtype = 0;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if ((obj->item_type == ITEM_DURAPLAST
                     || obj->item_type == ITEM_DURASTEEL)
                    && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        charge = obj->value[1];
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR
                    && checkcond == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcond = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }
                if (obj->item_type == ITEM_LENS && checklens == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checklens = TRUE;
                }
                if (obj->item_type == ITEM_MIRROR && checkmirr == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkmirr = TRUE;
                }
                if (obj->item_type == ITEM_CRYSTAL && gems < 3)
                {
                        gems++;
                        if (gemtype < obj->value[0])
                                gemtype = obj->value[0];
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkgems = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_lightsaber_crafting]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura)
            || (!checkbatt) || (!checkoven) || (!checkmirr) || (!checklens)
            || (!checkgems) || (!checkcond) || (!checkcirc))

        {
                send_to_char
                        ("&RYou hold up your new lightsaber and press the switch hoping for the best.\n\r",
                         ch);
                send_to_char
                        ("&RInstead of a blade of light, smoke starts pouring from the handle.\n\r",
                         ch);
                send_to_char
                        ("&RYou drop the hot handle and watch as it melts on away on the floor.\n\r",
                         ch);
                learn_from_failure(ch, gsn_lightsaber_crafting);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 5;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " lightsaber saber", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        STRFREE(obj->action_desc);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " ignites with a hum and a soft glow.", MSL);
        obj->action_desc = STRALLOC(buf);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = get_atype("hitroll");
        paf->modifier = URANGE(0, gems, level / 30);
        paf->bitvector = 0;
        paf->next = NULL;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("parry");
        paf2->modifier = (level / 3);
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;  /* condition  */
        obj->value[1] = (int) (level / 10 + gemtype * 2);   /* min dmg  */
        obj->value[2] = (int) (level / 5 + gemtype * 6);    /* max dmg */
        obj->value[3] = WEAPON_LIGHTSABER;
        obj->value[4] = charge;
        obj->value[5] = charge;
        obj->cost = obj->value[2] * 75;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created lightsaber.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new lightsaber.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level(ch->skill_level[FORCE_ABILITY] + 1) -
                               exp_level(ch->skill_level[FORCE_ABILITY])));
                /*
                 * stop minimum, 19800500 is most you can have ever 
                 */
                xpgain = URANGE(0, xpgain, 19800500);
                gain_exp(ch, xpgain, FORCE_ABILITY);
                ch_printf(ch, "You gain %d force experience.", xpgain);
        }
        learn_from_success(ch, gsn_lightsaber_crafting);
}


CMDF do_makespice(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        OBJ_DATA *obj;

        switch (ch->substate)
        {
        default:
                mudstrlcpy(arg, argument, MIL);

                if (arg[0] == '\0')
                {
                        send_to_char("&RFrom what?\n\r&w", ch);
                        return;
                }

                if (!xIS_SET(ch->in_room->room_flags, ROOM_REFINERY))
                {
                        send_to_char
                                ("&RYou need to be in a refinery to create drugs from spice.\n\r",
                                 ch);
                        return;
                }

                if (ms_find_obj(ch))
                        return;

                if ((obj = get_obj_carry(ch, arg)) == NULL)
                {
                        send_to_char("&RYou do not have that item.\n\r&w",
                                     ch);
                        return;
                }

                if (obj->item_type != ITEM_RAWSPICE)
                {
                        send_to_char
                                ("&RYou can't make a drug out of that\n\r&w",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_spice_refining]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of refining spice into a drug.\n\r",
                                 ch);
                        act(AT_PLAIN, "$n begins working on something.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 10, do_makespice, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out what to do with the stuff.\n\r",
                         ch);
                learn_from_failure(ch, gsn_spice_refining);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are distracted and are unable to finish your work.\n\r&w",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        if ((obj = get_obj_carry(ch, arg)) == NULL)
        {
                send_to_char("You seem to have lost your spice!\n\r", ch);
                return;
        }
        if (obj->item_type != ITEM_RAWSPICE)
        {
                send_to_char
                        ("&RYou get your tools mixed up and can't finish your work.\n\r&w",
                         ch);
                return;
        }
        separate_obj(obj);

        obj->value[1] = URANGE(10, obj->value[1], (IS_NPC(ch) ? ch->top_level
                                                   : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_spice_refining]))
                               + 10);
        mudstrlcpy(buf, obj->name, MSL);
        STRFREE(obj->name);
        mudstrlcat(buf, " drug spice", MSL);
        obj->name = STRALLOC(buf);
        mudstrlcpy(buf, "a drug made from ", MSL);
        mudstrlcat(buf, obj->short_descr, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        mudstrlcat(buf, " was foolishly left lying around here.", MSL);
        STRFREE(obj->description);
        obj->description = STRALLOC(buf);
        obj->item_type = ITEM_SPICE;

        send_to_char("&GYou finish your work.\n\r", ch);
        act(AT_PLAIN, "$n finishes $s work.", ch, NULL, argument, TO_ROOM);

        obj->cost += obj->value[1] * 10;
        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_spice_refining);

}

CMDF do_makegrenade(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, strength = 0, weight = 0;
        bool      checktool, checkdrink, checkbatt, checkchem, checkcirc;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makegrenade <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkdrink = FALSE;
                checkbatt = FALSE;
                checkchem = FALSE;
                checkcirc = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DRINK_CON
                            && obj->value[1] == 0)
                                checkdrink = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_CHEMICAL)
                                checkchem = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a grenade.\n\r",
                                 ch);
                        return;
                }

                if (!checkdrink)
                {
                        send_to_char
                                ("&RYou will need an empty drink container to mix and hold the chemicals.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a small battery for the timer.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char
                                ("&RYou need a small circuit for the timer.\n\r",
                                 ch);
                        return;
                }

                if (!checkchem)
                {
                        send_to_char
                                ("&RSome explosive chemicals would come in handy!\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makegrenade]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a grenade.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a drink container and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makegrenade, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makegrenade);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makegrenade]);
        vnum = 10425;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdrink = FALSE;
        checkbatt = FALSE;
        checkchem = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE
                    && obj->value[1] == 0)
                {
                        checkdrink = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_CHEMICAL)
                {
                        strength = URANGE(10, obj->value[0], level * 5);
                        weight = obj->weight;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkchem = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makegrenade]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdrink)
            || (!checkbatt) || (!checkchem) || (!checkcirc))
        {
                send_to_char
                        ("&RJust as you are about to finish your work,\n\ryour newly created grenade explodes in your hands...doh!\n\r",
                         ch);
                learn_from_failure(ch, gsn_makegrenade);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_GRENADE;
        SET_BIT(obj->wear_flags, ITEM_HOLD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = weight;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " grenade", MSL);
        obj->name = STRALLOC(buf);
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = strength / 2;
        obj->value[1] = strength;
        obj->cost = obj->value[1] * 5;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created grenade.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new grenade.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makegrenade);
}

CMDF do_makelandmine(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, strength = 0, weight = 0;
        bool      checktool, checkdrink, checkbatt, checkchem, checkcirc;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makelandmine <name>\n\r&w",
                                     ch);
                        return;
                }

                checktool = FALSE;
                checkdrink = FALSE;
                checkbatt = FALSE;
                checkchem = FALSE;
                checkcirc = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DRINK_CON
                            && obj->value[1] == 0)
                                checkdrink = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_CHEMICAL)
                                checkchem = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a landmine.\n\r",
                                 ch);
                        return;
                }

                if (!checkdrink)
                {
                        send_to_char
                                ("&RYou will need an empty drink container to mix and hold the chemicals.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a small battery for the detonator.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char
                                ("&RYou need a small circuit for the detonator.\n\r",
                                 ch);
                        return;
                }

                if (!checkchem)
                {
                        send_to_char
                                ("&RSome explosive chemicals would come in handy!\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makelandmine]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a landmine.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a drink container and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makelandmine, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makelandmine);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned
                                                    [gsn_makelandmine]);
        vnum = 10427;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdrink = FALSE;
        checkbatt = FALSE;
        checkchem = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE
                    && obj->value[1] == 0)
                {
                        checkdrink = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_CHEMICAL)
                {
                        strength = URANGE(10, obj->value[0], level * 5);
                        weight = obj->weight;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkchem = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makelandmine]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdrink)
            || (!checkbatt) || (!checkchem) || (!checkcirc))
        {
                send_to_char
                        ("&RJust as you are about to finish your work,\n\ryour newly created landmine explodes in your hands...doh!\n\r",
                         ch);
                learn_from_failure(ch, gsn_makelandmine);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_LANDMINE;
        SET_BIT(obj->wear_flags, ITEM_HOLD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = weight;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " landmine", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MSL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = strength / 2;
        obj->value[1] = strength;
        obj->cost = obj->value[1] * 5;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created landmine.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new landmine.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makelandmine);
}
CMDF do_makelight(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, strength = 0;
        bool      checktool, checkbatt, checkchem, checkcirc, checklens;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeflashlight <name>\n\r&w",
                                     ch);
                        return;
                }

                checktool = FALSE;
                checkbatt = FALSE;
                checkchem = FALSE;
                checkcirc = FALSE;
                checklens = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_CHEMICAL)
                                checkchem = TRUE;
                        if (obj->item_type == ITEM_LENS)
                                checklens = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a light.\n\r",
                                 ch);
                        return;
                }

                if (!checklens)
                {
                        send_to_char("&RYou need a lens to make a light.\n\r",
                                     ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a battery for the light to work.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char("&RYou need a small circuit.\n\r", ch);
                        return;
                }

                if (!checkchem)
                {
                        send_to_char
                                ("&RSome chemicals to light would come in handy!\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makelight]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a light.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 10, do_makelight, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makelight);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makelight]);
        vnum = 10428;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checklens = FALSE;
        checkbatt = FALSE;
        checkchem = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        strength = obj->value[0];
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_CHEMICAL)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkchem = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }
                if (obj->item_type == ITEM_LENS && checklens == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checklens = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makelight]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checklens)
            || (!checkbatt) || (!checkchem) || (!checkcirc))
        {
                send_to_char
                        ("&RJust as you are about to finish your work,\n\ryour newly created light explodes in your hands...doh!\n\r",
                         ch);
                learn_from_failure(ch, gsn_makelight);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_LIGHT;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 3;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, " light", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[2] = strength;
        obj->cost = obj->value[2];

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created light.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new light.", ch, NULL, argument,
            TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makelight);
}

CMDF do_makejewelry(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage;
        bool      checktool, checkoven, checkmetal;
        OBJ_DATA *obj;
        OBJ_DATA *metal = NULL;
        int       value, cost;

        switch (ch->substate)
        {
        default:
                argument = one_argument(argument, arg);
                mudstrlcpy(arg2, argument, MIL);

                if (!str_cmp(arg, "body")
                                || !str_cmp(arg, "head")
                                || !str_cmp(arg, "legs")
                                || !str_cmp(arg, "arms")
                                || !str_cmp(arg, "about")
                                || !str_cmp(arg, "eyes")
                                || !str_cmp(arg, "waist")
                                || !str_cmp(arg, "hold")
                                || !str_cmp(arg, "feet") || !str_cmp(arg, "hands"))
                {
                        send_to_char
                                ("&RYou cannot make jewelry for that body part.\n\r&w",
                                 ch);
                        send_to_char("&RTry MAKEARMOR.\n\r&w", ch);
                        return;
                }
                else if (!str_cmp(arg, "shield"))
                {
                        send_to_char
                                ("&RYou cannot make jewelry worn as a shield.\n\r&w",
                                 ch);
                        send_to_char("&RTry MAKESHIELD.\n\r&w", ch);
                        return;
                }
                else if (!str_cmp(arg, "wield"))
                {
                        send_to_char
                                ("&RAre you going to fight with your jewelry?\n\r&w",
                                 ch);
                        send_to_char("&RTry MAKEBLADE...\n\r&w", ch);
                        return;
                }
                else if ((value = get_wflag(arg)) == -1) {
                        send_to_char("&RInvalid wear location try:\n\r", ch);
                        for (value = 0; value < MAX_WEAR; value++) {
                                char * wearloc = w_flags[value];
                                if (!str_cmp(wearloc, "body")
                                                || !str_cmp(wearloc, "head")
                                                || !str_cmp(wearloc, "legs")
                                                || !str_cmp(wearloc, "arms")
                                                || !str_cmp(wearloc, "about")
                                                || !str_cmp(wearloc, "eyes")
                                                || !str_cmp(wearloc, "waist")
                                                || !str_cmp(wearloc, "hold")
                                                || !str_cmp(wearloc, "feet")
                                                || !str_cmp(wearloc, "shield")
                                                || !str_cmp(wearloc, "binding")
                                                || !str_cmp(wearloc, "holster")
                                                || !str_cmp(wearloc, "wield")
                                                || !str_cmp(wearloc, "take")
                                                || !str_cmp(wearloc, "hands")
                                                || wearloc[0] == '_'
                                                || (wearloc[0] == 'r' && (wearloc[1]-'0' >= 0 && wearloc[1]-'0' <= 9))
                                   )
                                        continue;
                                ch_printf(ch, "\t%s\n\r", wearloc);
                        }
                }

                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makejewelry <wearloc> <name>\n\r&w",
                                 ch);
                        return;
                }

                checktool = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                        if (obj->item_type == ITEM_RARE_METAL)
                                checkmetal = TRUE;
                }

                if (!checktool)
                {
                        send_to_char("&RYou need a toolkit.\n\r", ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char("&RYou need an oven.\n\r", ch);
                        return;
                }

                if (!checkmetal)
                {
                        send_to_char("&RYou need some precious metal.\n\r",
                                     ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makejewelry]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating some jewelry.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s toolkit and some metal and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 15, do_makejewelry, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makejewelry);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makejewelry]);

        checkmetal = FALSE;
        checkoven = FALSE;
        checktool = FALSE;
        value = 0;
        cost = 0;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
                {
                        checkmetal = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        metal = obj;
                }
                if (obj->item_type == ITEM_CRYSTAL)
                {
                        cost += obj->cost;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makejewelry]);

        if (number_percent() > percentage * 2 || (!checkoven) || (!checktool)
            || (!checkmetal))
        {
                send_to_char("&RYou hold up your newly created jewelry.\n\r",
                             ch);
                send_to_char
                        ("&RIt suddenly dawns upon you that you have created the most useless\n\r",
                         ch);
                send_to_char
                        ("&Rpiece of junk you've ever seen. You quickly hide your mistake...\n\r",
                         ch);
                learn_from_failure(ch, gsn_makejewelry);
                return;
        }

        obj = metal;

        obj->item_type = ITEM_ARMOR;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        value = get_wflag(arg);
        if (value < 0 || value > 31)
                SET_BIT(obj->wear_flags, ITEM_WEAR_NECK);
        else
                SET_BIT(obj->wear_flags, 1 << value);
        if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
                REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);
        obj->level = level;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg2, MIL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg2, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was dropped here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = obj->value[1];
        obj->cost *= 10;
        obj->cost += cost;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created jewelry.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes sewing some new jewelry.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makejewelry);

}

CMDF do_makearmor(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage;
        bool      checksew, checkfab;
        OBJ_DATA *obj;
        OBJ_DATA *material = NULL;
        int       value;

        argument = one_argument(argument, arg);
        mudstrlcpy(arg2, argument, MIL);

        if (!str_cmp(arg, "eyes")
            || !str_cmp(arg, "ears")
            || !str_cmp(arg, "finger")
            || !str_cmp(arg, "neck") || !str_cmp(arg, "wrist"))
        {
                send_to_char
                        ("&RYou cannot make clothing for that body part.\n\r&w",
                         ch);
                send_to_char("&RTry MAKEJEWELRY.\n\r&w", ch);
                return;
        }
        if (!str_cmp(arg, "shield"))
        {
                send_to_char
                        ("&RYou cannot make clothing worn as a shield.\n\r&w",
                         ch);
                send_to_char("&RTry MAKESHIELD.\n\r&w", ch);
                return;
        }
        if (!str_cmp(arg, "wield"))
        {
                send_to_char
                        ("&RAre you going to fight with your clothing?\n\r&w",
                         ch);
                send_to_char("&RTry MAKEBLADE...\n\r&w", ch);
                return;
        }

        switch (ch->substate)
        {
        default:

                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makearmor <wearloc> <name>\n\r&w",
                                 ch);
                        return;
                }

                checksew = FALSE;
                checkfab = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_FABRIC)
                                checkfab = TRUE;
                        if (obj->item_type == ITEM_THREAD)
                                checksew = TRUE;
                }

                if (!checkfab)
                {
                        send_to_char
                                ("&RYou need some sort of fabric or material.\n\r",
                                 ch);
                        return;
                }

                if (!checksew)
                {
                        send_to_char
                                ("&RYou need a needle and some thread.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makearmor]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating some armor.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s sewing kit and some material and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makearmor, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makearmor);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makearmor]);

        checksew = FALSE;
        checkfab = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_THREAD)
                        checksew = TRUE;
                if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
                {
                        checkfab = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        material = obj;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makearmor]);

        if (number_percent() > percentage * 2 || (!checkfab) || (!checksew))
        {
                send_to_char("&RYou hold up your newly created armor.\n\r",
                             ch);
                send_to_char
                        ("&RIt suddenly dawns upon you that you have created the most useless\n\r",
                         ch);
                send_to_char
                        ("&Rgarment you've ever seen. You quickly hide your mistake...\n\r",
                         ch);
                learn_from_failure(ch, gsn_makearmor);
                return;
        }

        obj = material;

        obj->item_type = ITEM_ARMOR;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        value = get_wflag(arg);
        if (value < 0 || value > 31)
                SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
        else
                SET_BIT(obj->wear_flags, 1 << value);
        if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
                REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);
        obj->level = level;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg2, MIL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg2, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was dropped here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = obj->value[1];
        obj->cost *= 10;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created garment.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes sewing some new armor.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makearmor);
}


CMDF do_makecomlink(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        bool      checktool, checkgem, checkbatt, checkcirc;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;
        int       value;

        argument = one_argument(argument, arg);
        mudstrlcpy(arg2, argument, MIL);

        if (!str_cmp(arg, "eyes")
            || !str_cmp(arg, "neck1")
            || !str_cmp(arg, "neck2")
            || !str_cmp(arg, "finger1")
            || !str_cmp(arg, "finger2") || !str_cmp(arg, "shield"))
        {
                send_to_char
                        ("&RYou cannot make a comlink for that body part.\n\r&w",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:

                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makecomlink <wearloc> <name>\n\r&w",
                                 ch);
                        return;
                }

                checktool = FALSE;
                checkgem = FALSE;
                checkbatt = FALSE;
                checkcirc = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        else if (obj->item_type == ITEM_CRYSTAL)
                                checkgem = TRUE;
                        else if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        else if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a comlink.\n\r",
                                 ch);
                        return;
                }

                if (!checkgem)
                {
                        send_to_char("&RYou need a small crystal.\n\r", ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your comlink.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char("&RYou need a small circuit.\n\r", ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makecomlink]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of making a comlink.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 10, do_makecomlink, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makecomlink);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        /*
         * Magic Numners are bad - Gavin 
         */
        vnum = 10430;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkgem = FALSE;
        checkbatt = FALSE;
        checkcirc = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_CRYSTAL && checkgem == FALSE)
                {
                        checkgem = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        checkcirc = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makecomlink]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkcirc)
            || (!checkbatt) || (!checkgem))
        {
                act(AT_RED, "&RYou hold up your newly created comlink....\n\r"
                    "&Rand it falls apart in your hands.\n\r", ch, NULL, NULL,
                    TO_CHAR);
                act(AT_RED,
                    "&R$n hold up thier newly created comlink....\n\r"
                    "&Rand it falls apart in thier hands.\n\r", ch, NULL,
                    NULL, TO_ROOM);
                learn_from_failure(ch, gsn_makecomlink);
                return;
        }
        obj = create_object(pObjIndex, ch->top_level);

        obj->item_type = ITEM_COMLINK;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        value = get_wflag(arg);
        if (value < 0 || value > 31)
                SET_BIT(obj->wear_flags, ITEM_HOLD);
        else
                SET_BIT(obj->wear_flags, 1 << value);
        obj->weight = 3;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg2, MIL);
        mudstrlcat(buf, " comlink", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg2, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was left here.", MSL);
        obj->description = STRALLOC(buf);
        obj->cost = 50;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created comlink.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a comlink.", ch, NULL, NULL,
            TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makecomlink);
}

CMDF do_makeshield(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        bool      checktool, checkbatt, checkcond, checkcirc, checkgems;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum, level, charge, gemtype = 0;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeshield <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkbatt = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;
                checkgems = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char("&RYou need to be in a workshop.\n\r",
                                     ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_CRYSTAL)
                                checkgems = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checkcond = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make an energy shield.\n\r",
                                 ch);
                        return;
                }

                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou need a power source for your energy shield.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char("&RYou need a small circuit board.\n\r",
                                     ch);
                        return;
                }

                if (!checkcond)
                {
                        send_to_char
                                ("&RYou still need a small superconductor for your energy shield.\n\r",
                                 ch);
                        return;
                }

                if (!checkgems)
                {
                        send_to_char("&RYou need a small crystal.\n\r", ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeshield]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting an energy shield.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 20, do_makeshield, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeshield);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makeshield]);
        vnum = 10429;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkbatt = FALSE;
        checkcond = FALSE;
        checkcirc = FALSE;
        checkgems = FALSE;
        charge = 0;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;

                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        charge = UMIN(obj->value[1], 10);
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkbatt = TRUE;
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR
                    && checkcond == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcond = TRUE;
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkcirc = TRUE;
                }
                if (obj->item_type == ITEM_CRYSTAL && checkgems == FALSE)
                {
                        gemtype = obj->value[0];
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkgems = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeshield]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkbatt)
            || (!checkgems) || (!checkcond) || (!checkcirc))

        {
                send_to_char
                        ("&RYou hold up your new energy shield and press the switch hoping for the best.\n\r",
                         ch);
                send_to_char
                        ("&RInstead of a field of energy being created, smoke starts pouring from the device.\n\r",
                         ch);
                send_to_char
                        ("&RYou drop the hot device and watch as it melts on away on the floor.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeshield);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_ARMOR;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_WEAR_SHIELD);
        obj->level = level;
        obj->weight = 2;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, "energy shield", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was carelessly misplaced here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = (int) (level / 10 + gemtype * 2);   /* condition */
        obj->value[1] = (int) (level / 10 + gemtype * 2);   /* armor */
        obj->value[4] = charge;
        obj->value[5] = charge;
        obj->cost = obj->level * 100;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created energy shield.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes making $s new energy shield.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 50,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makeshield);

}

CMDF do_makecontainer(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, capacity;
        bool      checksew, checkfab;
        OBJ_DATA *obj;
        OBJ_DATA *material = NULL;
        int       value;

        argument = one_argument(argument, arg);
        mudstrlcpy(arg2, argument, MIL);

        if (!str_cmp(arg, "eyes")
            || !str_cmp(arg, "ears")
            || !str_cmp(arg, "finger")
            || !str_cmp(arg, "neck")
            || !str_cmp(arg, "neck1")
            || !str_cmp(arg, "neck2")
            || !str_cmp(arg, "wrist1")
            || !str_cmp(arg, "wrist2")
            || !str_cmp(arg, "finger1")
            || !str_cmp(arg, "finger2") || !str_cmp(arg, "wrist"))
        {
                send_to_char
                        ("&RYou cannot make a container for that body part.\n\r&w",
                         ch);
                send_to_char("&RTry MAKEJEWELRY.\n\r&w", ch);
                return;
        }
        if (!str_cmp(arg, "feet")
            || !str_cmp(arg, "hands") || !str_cmp(arg, "head"))
        {
                send_to_char
                        ("&RYou cannot make a container for that body part.\n\r&w",
                         ch);
                send_to_char("&RTry MAKEARMOR.\n\r&w", ch);
                return;
        }
        if (!str_cmp(arg, "shield"))
        {
                send_to_char("&RYou cannot make a container a shield.\n\r&w",
                             ch);
                send_to_char("&RTry MAKESHIELD.\n\r&w", ch);
                return;
        }
        if (!str_cmp(arg, "wield"))
        {
                send_to_char
                        ("&RAre you going to fight with a container?\n\r&w",
                         ch);
                send_to_char("&RTry MAKEBLADE...\n\r&w", ch);
                return;
        }

        switch (ch->substate)
        {
        default:

                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makecontainer <wearloc> <name>\n\r&w",
                                 ch);
                        return;
                }

                checksew = FALSE;
                checkfab = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_FABRIC)
                                checkfab = TRUE;
                        if (obj->item_type == ITEM_THREAD)
                                checksew = TRUE;
                }

                if (!checkfab)
                {
                        send_to_char
                                ("&RYou need some sort of fabric or material.\n\r",
                                 ch);
                        return;
                }

                if (!checksew)
                {
                        send_to_char
                                ("&RYou need a needle and some thread.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makecontainer]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating a bag.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s sewing kit and some material and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 10, do_makecontainer, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makecontainer);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;


        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned
                                                    [gsn_makecontainer]);

        if (ch->pcdata->learned[gsn_makecontainer] == 100)
                capacity = level * 2;
        else
                capacity = level;

        checksew = FALSE;
        checkfab = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_THREAD)
                        checksew = TRUE;
                if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
                {
                        checkfab = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        material = obj;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makecontainer]);

        if (number_percent() > percentage * 2 || (!checkfab) || (!checksew))
        {
                send_to_char
                        ("&RYou hold up your newly created container.\n\r",
                         ch);
                send_to_char
                        ("&RIt suddenly dawns upon you that you have created the most useless\n\r",
                         ch);
                send_to_char
                        ("&Rcontainer you've ever seen. You quickly hide your mistake...\n\r",
                         ch);
                learn_from_failure(ch, gsn_makecontainer);
                return;
        }

        obj = material;

        obj->item_type = ITEM_CONTAINER;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
		value = get_wflag(arg);
		if (value < 0 || value > 31)
			SET_BIT(obj->wear_flags, ITEM_HOLD);
		else
			SET_BIT(obj->wear_flags, 1 << value);

		/* This should fix holster stuff */
		if (!str_cmp(arg, "holster")) {
			obj->item_type = ITEM_HOLSTER;
		}

        if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
                REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);
        obj->level = level;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg2, MIL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg2, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was dropped here.", MSL);
        obj->description = STRALLOC(buf);
        obj->value[0] = capacity;
        obj->value[1] = 0;
        obj->value[2] = 0;
        obj->value[3] = 10;
        obj->cost *= 2;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created container.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes sewing a new container.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makecontainer);
}

CMDF do_gemcutting(CHAR_DATA * ch, char *argument)
{

        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage, gemtype = 0;
        bool      checktool, checkgems;
        OBJ_DATA *obj;
        OBJ_DATA *crystal;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;
        AFFECT_DATA *paf = NULL;



        switch (ch->substate)
        {
        default:
                mudstrlcpy(arg, argument, MIL);

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: cut <crystal>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkgems = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }


                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_CRYSTAL)
                                checkgems = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to cut a crystal.\n\r",
                                 ch);
                        return;
                }

                if (!checkgems)
                {
                        send_to_char("&RYou need a crystal to cut.\n\r", ch);
                        return;
                }

                if (get_obj_carry(ch, arg) == NULL)
                {
                        send_to_char("You don't have that specific gem", ch);
                        return;
                }


                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_gemcutting]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of cutting a gem.\n\r",
                                 ch);
                        act(AT_PLAIN, "$n takes $s tools and starts working.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 5, do_gemcutting, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_gemcutting);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_gemcutting]);
        vnum = 10432;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkgems = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
        }


        if ((crystal = get_obj_carry_type(ch, arg, ITEM_CRYSTAL)))
        {
                gemtype = crystal->value[0];
                separate_obj(crystal);
                obj_from_char(crystal);
                extract_obj(crystal);
                checkgems = TRUE;
        }
        else
        {
                send_to_char("You seem to have lost your gem.\n\r", ch);
                return;
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_gemcutting]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkgems))
        {
                send_to_char("&RYou hold up your piece of metal.\n\r", ch);
                send_to_char("&RYou try to working it, but it cracks.\n\r",
                             ch);
                send_to_char("&RWhat a brittle piece of garbage.\n\r", ch);
                learn_from_failure(ch, gsn_gemcutting);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_RARE_METAL;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 10;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, " metal", MSL);
        obj->name = STRALLOC(smash_color(buf));
        stralloc_printf(&obj->name, "%s metal",
                        gemtype > -1 ? gemtype <
                        8 ? crystal_table[gemtype] : "crystaline" :
                        "crystaline");
        stralloc_printf(&obj->short_descr, "%s metal",
                        gemtype > -1 ? gemtype <
                        8 ? crystal_table[gemtype] : "crystaline" :
                        "crystaline");
        STRFREE(obj->description);
        stralloc_printf(&obj->description,
                        "A %s metal was carelessly dropped here.",
                        gemtype > -1 ? gemtype <
                        8 ? crystal_table[gemtype] : "crystaline" :
                        "crystaline");
        if (gemtype > -1 && gemtype < 8)
        {
                CREATE(paf, AFFECT_DATA, 1);
                paf->type = -1;
                paf->duration = -1;
                if (gemtype == 0)
                {
                        paf->location = get_atype("hitroll");
                        paf->modifier = 1;
                }
                else if (gemtype == 1)
                {
                        paf->location = get_atype("damroll");
                        paf->modifier = 1;
                }
                else if (gemtype == 2)
                {
                        paf->location = APPLY_STR;
                        paf->modifier = 1;
                }
                else if (gemtype == 3)
                {
                        paf->location = APPLY_DEX;
                        paf->modifier = 1;
                }
                else if (gemtype == 4)
                {
                        paf->location = APPLY_CON;
                        paf->modifier = 1;
                }
                else if (gemtype == 5)
                {
                        paf->location = APPLY_CHA;
                        paf->modifier = 1;
                }
                else if (gemtype == 6)
                {
                        paf->location = APPLY_LCK;
                        paf->modifier = 1;
                }
                else
                {
                        paf->location = APPLY_AC;
                        paf->modifier = -15;
                }
                paf->bitvector = 0;
                paf->next = NULL;
                LINK(paf, obj->first_affect, obj->last_affect, next, prev);
                ++top_affect;
        }
        obj->value[1] = level / 10;
        obj->cost = level * 50;


        obj = obj_to_char(obj, ch);

        send_to_char("&GYou hold up your new sheet of metal.&w\n\r", ch);
        act(AT_PLAIN, "$n finishes cutting a new piece of metal.",
            ch, NULL, argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 100,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_gemcutting);
}

CMDF do_reinforcements(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (ch->backup_wait)
                {
                        send_to_char
                                ("&RYour reinforcements are already on the way.\n\r",
                                 ch);
                        return;
                }
                /*
                 * players can now do this
                 * if ( !ch->pcdata->clan )
                 * {
                 * send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
                 * return;
                 * }    
                 * 
                 */

                if (!has_comlink(ch))
                {
                        send_to_char("You need a comlink to call someone!",
                                     ch);
                        return;
                }

                if (ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 50)
                {
                        ch_printf(ch,
                                  "&RYou dont have enough credits to send for reinforcements.\n\r");
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_reinforcements]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin making the call for reinforcements.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 1, do_reinforcements, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou call for reinforcements but nobody answers.\n\r",
                         ch);
                learn_from_failure(ch, gsn_reinforcements);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted before you can finish your call.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char("&GYour reinforcements are on the way.\n\r", ch);
        credits = ch->skill_level[LEADERSHIP_ABILITY] * 50;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->gold -= UMIN(credits, ch->gold);

        learn_from_success(ch, gsn_reinforcements);


/* removed   
    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_STORMTROOPER;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_TROOPER;
    else
       ch->backup_mob = MOB_VNUM_MERCINARY;
*/

/* This really is a player only thing so only mercs can be hired. */

        ch->backup_mob = MOB_VNUM_MERCINARY;


        ch->backup_wait = number_range(1, 2);

}


CMDF do_postguard(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (ch->backup_wait)
                {
                        send_to_char("&RYou already have backup coming.\n\r",
                                     ch);
                        return;
                }
                /*
                 * removed as players can now have guards.
                 * if ( !ch->pcdata->clan )
                 * {
                 * send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
                 * return;
                 * }    
                 */

                if (!has_comlink(ch))
                {
                        send_to_char("You need a comlink to call someone!",
                                     ch);
                        return;
                }

                if (ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30)
                {
                        ch_printf(ch, "&RYou dont have enough credits.\n\r",
                                  ch);
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_postguard]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin making the call for reinforcements.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 1, do_postguard, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou call for a guard but nobody answers.\n\r",
                             ch);
                learn_from_failure(ch, gsn_postguard);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted before you can finish your call.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char("&GYour guard is on the way.\n\r", ch);

        credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->gold -= UMIN(credits, ch->gold);

        learn_from_success(ch, gsn_postguard);


/* no longer use this
    
    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_GUARD;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_GUARD;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER;

*/


        ch->backup_mob = MOB_VNUM_GUARD;


        ch->backup_wait = 1;

}


CMDF do_add_patrol(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (ch->backup_wait)
                {
                        send_to_char
                                ("&RYou already have a patrol coming.\n\r",
                                 ch);
                        return;
                }
                /*
                 * removed as players can now have guards.
                 * if ( !ch->pcdata->clan )
                 * {
                 * send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
                 * return;
                 * }    
                 */

                if (!has_comlink(ch))
                {
                        send_to_char("You need a comlink to call someone!",
                                     ch);
                        return;
                }

                if (ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30)
                {
                        ch_printf(ch, "&RYou dont have enough credits.\n\r",
                                  ch);
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_add_patrol]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin making the call for a patrol.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 1, do_add_patrol, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou call for a patrol but nobody answers.\n\r",
                         ch);
                learn_from_failure(ch, gsn_add_patrol);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted before you can finish your call.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char("&GYour patrol is on the way.\n\r", ch);

        credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->gold -= UMIN(credits, ch->gold);

        learn_from_success(ch, gsn_add_patrol);


/* no longer use this
    
    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_GUARD;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_GUARD;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER;

*/


        ch->backup_mob = MOB_VNUM_PATROL;


        ch->backup_wait = 1;

}





void add_reinforcements(CHAR_DATA * ch)
{
        MOB_INDEX_DATA *pMobIndex;
        OBJ_DATA *blaster;
        OBJ_INDEX_DATA *pObjIndex;

        if ((pMobIndex = get_mob_index(ch->backup_mob)) == NULL)
                return;

        if (ch->backup_mob == MOB_VNUM_MERCINARY)
        {
                CHAR_DATA *mob[3];
                int       mob_cnt;

                send_to_char("Your reinforcements have arrived.\n\r", ch);
                for (mob_cnt = 0; mob_cnt < 3; mob_cnt++)
                {
                        int       ability;

                        mob[mob_cnt] = create_mobile(pMobIndex);
                        char_to_room(mob[mob_cnt], ch->in_room);
                        act(AT_IMMORT, "$N has arrived.", ch, NULL,
                            mob[mob_cnt], TO_ROOM);
                        mob[mob_cnt]->top_level =
                                ch->skill_level[LEADERSHIP_ABILITY] / 3;
                        for (ability = 0; ability < MAX_ABILITY; ability++)
                                mob[mob_cnt]->skill_level[ability] =
                                        mob[mob_cnt]->top_level;
                        mob[mob_cnt]->hit = mob[mob_cnt]->top_level * 15;
                        mob[mob_cnt]->max_hit = mob[mob_cnt]->hit;
                        mob[mob_cnt]->armor =
                                (int) (100 - mob[mob_cnt]->top_level * 2.5);
                        mob[mob_cnt]->damroll = mob[mob_cnt]->top_level / 5;
                        mob[mob_cnt]->hitroll = mob[mob_cnt]->top_level / 5;
                        if ((pObjIndex =
                             get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                        {
                                blaster =
                                        create_object(pObjIndex,
                                                      mob[mob_cnt]->
                                                      top_level);
                                obj_to_char(blaster, mob[mob_cnt]);
                                equip_char(mob[mob_cnt], blaster, WEAR_WIELD);
                        }
                        if (mob[mob_cnt]->master)
                                stop_follower(mob[mob_cnt]);
                        add_follower(mob[mob_cnt], ch);
                        SET_BIT(mob[mob_cnt]->affected_by, AFF_CHARM);
                        do_setblaster(mob[mob_cnt], "full");
                }
        }

        else if (ch->backup_mob == MOB_VNUM_SPECIAL_FORCES)
        {
                CHAR_DATA *mob[3];
                int       mob_cnt;

                send_to_char("Your special forces have arrived.\n\r", ch);
                for (mob_cnt = 0; mob_cnt < 3; mob_cnt++)
                {
                        int       ability;

                        mob[mob_cnt] = create_mobile(pMobIndex);
                        char_to_room(mob[mob_cnt], ch->in_room);
                        act(AT_IMMORT, "$N has arrived.", ch, NULL,
                            mob[mob_cnt], TO_ROOM);
                        mob[mob_cnt]->top_level =
                                ch->skill_level[LEADERSHIP_ABILITY];
                        for (ability = 0; ability < MAX_ABILITY; ability++)
                                mob[mob_cnt]->skill_level[ability] =
                                        mob[mob_cnt]->top_level;
                        mob[mob_cnt]->hit = mob[mob_cnt]->top_level * 15;
                        mob[mob_cnt]->max_hit = mob[mob_cnt]->hit;
                        mob[mob_cnt]->armor =
                                (int) (100 - mob[mob_cnt]->top_level * 2.5);
                        mob[mob_cnt]->damroll = mob[mob_cnt]->top_level / 5;
                        mob[mob_cnt]->hitroll = mob[mob_cnt]->top_level / 5;
                        if ((pObjIndex =
                             get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                        {
                                blaster =
                                        create_object(pObjIndex,
                                                      mob[mob_cnt]->
                                                      top_level);
                                obj_to_char(blaster, mob[mob_cnt]);
                                equip_char(mob[mob_cnt], blaster, WEAR_WIELD);
                        }
                        if (mob[mob_cnt]->master)
                                stop_follower(mob[mob_cnt]);
                        add_follower(mob[mob_cnt], ch);
                        SET_BIT(mob[mob_cnt]->affected_by, AFF_CHARM);
                        do_setblaster(mob[mob_cnt], "full");
                }
        }
        else if (ch->backup_mob == MOB_VNUM_ELITE_GUARD)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);
                if (ch->pcdata && ch->pcdata->clan)
                {

                        if (mob->name)
                                STRFREE(mob->name);
                        mob->name = STRALLOC("Elite guard");;
                        stralloc_printf(&mob->long_descr,
                                        "(%s) Elite Guard\n",
                                        ch->pcdata->clan->name);
                        if (mob->mob_clan)
                                STRFREE(mob->mob_clan);
                        mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                        mob->spec_fun = spec_lookup("spec_clan_guard");
                        if (mob->spec_funname)
                                STRFREE(mob->spec_funname);
                        mob->spec_funname = STRALLOC("spec_clan_guard");
                }
                else
                {


                        STRFREE(mob->name);
                        mob->name = STRALLOC("guard");
                        stralloc_printf(&mob->long_descr, "%ss guard\n",
                                        ch->name);
                        /*
                         * use leader because its handy for spec_player_guard
                         */
                        mob->leader = ch;
                        mob->spec_fun =
                                spec_lookup("spec_player_elite_guard");
                        if (mob->spec_funname)
                                STRFREE(mob->spec_funname);
                        mob->spec_funname =
                                STRALLOC("spec_player_elite_guard");
                }

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your elite guard has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 25;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 4.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }


        }

        else if (ch->backup_mob == MOB_VNUM_INSTALL_GUARD)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);


                STRFREE(mob->name);
                mob->name = STRALLOC("Installation guard");
                stralloc_printf(&mob->long_descr, "%s", "Installation Guard");
                if (mob->mob_clan)
                        STRFREE(mob->mob_clan);
                mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                mob->spec_fun = spec_lookup("spec_installation_guard");
                if (mob->spec_funname)
                        STRFREE(mob->spec_funname);
                mob->spec_funname = STRALLOC("spec_installation_guard");

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your installation guard has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 25;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 4.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }
                if (!mob_reset(mob, "installation", TRUE))
                        do_say(mob,
                               "Sir! I will be unable to stay in this position for long, sir!");
                else
                {
                        mob_reset(mob, "installation", FALSE);
                        fold_area(ch->in_room->area,
                                  ch->in_room->area->filename, FALSE, FALSE);
                }
        }

        else if (ch->backup_mob == MOB_VNUM_INSTALL_ENTERANCE_GUARD)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);


                STRFREE(mob->name);
                mob->name = STRALLOC("Installation Entrance guard");
                stralloc_printf(&mob->long_descr, "%s", "Installation Entrance Guard");
                if (mob->mob_clan)
                        STRFREE(mob->mob_clan);
                mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                mob->spec_fun =
                        spec_lookup("spec_installation_entrance_guard");
                if (mob->spec_funname)
                        STRFREE(mob->spec_funname);
                mob->spec_funname =
                        STRALLOC("spec_installation_entrance_guard");

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your installation guard has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 25;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 4.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }
                if (!mob_reset(mob, "installation", TRUE))
                        do_say(mob,
                               "Sir! I will be unable to stay in this position for long, sir!");
                else
                {
                        mob_reset(mob, "installation", FALSE);
                        fold_area(ch->in_room->area,
                                  ch->in_room->area->filename, FALSE, FALSE);
                }
        }

        else if (ch->backup_mob == MOB_VNUM_INSTALL_CUSTOMS)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);


                STRFREE(mob->name);
                mob->name = STRALLOC("Installation Customs Office");
                stralloc_printf(&mob->long_descr, "%s", "Installation Customs Officer");
                if (mob->mob_clan)
                        STRFREE(mob->mob_clan);
                mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                mob->spec_fun = spec_lookup("spec_customs_weapons");
                if (mob->spec_funname)
                        STRFREE(mob->spec_funname);
                mob->spec_funname = STRALLOC("spec_customs_weapons");

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your installation guard has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 25;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 4.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }
                if (!mob_reset(mob, "installation", TRUE))
                        do_say(mob,
                               "Sir! I will be unable to stay in this position for long, sir!");
                else
                {
                        mob_reset(mob, "installation", FALSE);
                        fold_area(ch->in_room->area,
                                  ch->in_room->area->filename, FALSE, FALSE);
                }
        }

        else if (ch->backup_mob == MOB_VNUM_INSTALL_DOCTOR)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);


                STRFREE(mob->name);
                mob->name = STRALLOC("Installation doctor");
                stralloc_printf(&mob->long_descr, "%s",
                                "Installation Doctor");
                if (mob->mob_clan)
                        STRFREE(mob->mob_clan);
                mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                mob->spec_fun = spec_lookup("spec_installation_doctor");
                if (mob->spec_funname)
                        STRFREE(mob->spec_funname);
                mob->spec_funname = STRALLOC("spec_installation_doctor");

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your installation doctor has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 25;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 4.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_INSTALL_BACTA_SPRAY)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                }
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_INSTALL_MEDPAC)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                }
                if (!mob_reset(mob, "installation", TRUE))
                        do_say(mob,
                               "Sir! I will be unable to stay in this position for long, sir!");
                else
                {
                        mob_reset(mob, "installation", FALSE);
                        fold_area(ch->in_room->area,
                                  ch->in_room->area->filename, FALSE, FALSE);
                }
        }



        else if (ch->backup_mob == MOB_VNUM_GUARD)
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);
                if (ch->pcdata && ch->pcdata->clan)
                {


                        STRFREE(mob->name);
                        mob->name = STRALLOC("guard");
                        stralloc_printf(&mob->long_descr, "(%s) Guard\n",
                                        ch->pcdata->clan->name);
                        if (mob->mob_clan)
                                STRFREE(mob->mob_clan);
                        mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                        mob->spec_fun = spec_lookup("spec_clan_guard");
                        if (mob->spec_funname)
                                STRFREE(mob->spec_funname);
                        mob->spec_funname = STRALLOC("spec_clan_guard");
                }
                else
                {


                        STRFREE(mob->name);
                        mob->name = STRALLOC("guard");
                        stralloc_printf(&mob->long_descr, "%ss Guard\n",
                                        ch->name);
                        /*
                         * use leader because its handy for spec_player_guard
                         */
                        mob->leader = ch;
                        mob->spec_fun = spec_lookup("spec_player_guard");
                }

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your guard has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 15;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 2.5);
                mob->damroll = mob->top_level / 5;
                mob->hitroll = mob->top_level / 5;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }


        }
        else
        {
                CHAR_DATA *mob;
                int       ability;

                mob = create_mobile(pMobIndex);
                char_to_room(mob, ch->in_room);
                if (ch->pcdata && ch->pcdata->clan)
                {


                        STRFREE(mob->name);
                        mob->name = STRALLOC("patrol");
                        stralloc_printf(&mob->long_descr, "(%s) Patrol\n",
                                        ch->pcdata->clan->name);
                        if (mob->mob_clan)
                                STRFREE(mob->mob_clan);
                        mob->mob_clan = STRALLOC(ch->pcdata->clan->name);
                        mob->spec_fun = spec_lookup("spec_clan_patrol");
                        if (mob->spec_funname)
                                STRFREE(mob->spec_funname);
                        mob->spec_funname = STRALLOC("spec_clan_patrol");
                }
                else
                {


                        STRFREE(mob->name);
                        mob->name = STRALLOC("patrol");
                        stralloc_printf(&mob->long_descr, "%ss Patrol\n",
                                        ch->name);
                        /*
                         * use leader because its handy for spec_player_guard
                         */
                        mob->leader = ch;
                        mob->spec_fun = spec_lookup("spec_player_patrol");
                        if (mob->spec_funname)
                                STRFREE(mob->spec_funname);
                        mob->spec_funname = STRALLOC("spec_player_patrol");
                }

                act(AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM);
                send_to_char("Your patrol has arrived.\n\r", ch);
                mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
                for (ability = 0; ability < MAX_ABILITY; ability++)
                        mob->skill_level[ability] = mob->top_level;
                mob->hit = mob->top_level * 10;
                mob->max_hit = mob->hit;
                mob->armor = (int) (100 - mob->top_level * 1.5);
                mob->damroll = mob->top_level / 3;
                mob->hitroll = mob->top_level / 3;
                if ((pObjIndex =
                     get_obj_index(OBJ_VNUM_BLASTECH_E11)) != NULL)
                {
                        blaster = create_object(pObjIndex, mob->top_level);
                        obj_to_char(blaster, mob);
                        equip_char(mob, blaster, WEAR_WIELD);
                        do_setblaster(mob, "full");
                }


        }

}


CMDF do_torture(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       percentage, dam;
        bool      fail;

        if (!IS_NPC(ch) && ch->pcdata->learned[gsn_torture] <= 0)
        {
                send_to_char
                        ("Your mind races as you realize you have no idea how to do that.\n\r",
                         ch);
                return;
        }

        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        {
                send_to_char("You can't do that right now.\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (ch->mount)
        {
                send_to_char("You can't get close enough while mounted.\n\r",
                             ch);
                return;
        }

        if (arg[0] == '\0')
        {
                send_to_char("Torture whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Are you masacistic or what...\n\r", ch);
                return;
        }

        if (!IS_AWAKE(victim))
        {
                send_to_char("You need to wake them first.\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (victim->fighting)
        {
                send_to_char("You can't torture someone whos in combat.\n\r",
                             ch);
                return;
        }

        ch->alignment = ch->alignment -= 100;
        ch->alignment = URANGE(-1000, ch->alignment, 1000);

        WAIT_STATE(ch, skill_table[gsn_torture]->beats);

        fail = FALSE;
        percentage =
                ris_save(victim, ch->skill_level[LEADERSHIP_ABILITY],
                         RIS_PARALYSIS);
        if (percentage == 1000)
                fail = TRUE;
        else
                fail = saves_para_petri(percentage, victim);

        if (!IS_NPC(ch) && !IS_NPC(victim))
                percentage = sysdata.stun_plr_vs_plr;
        else
                percentage = sysdata.stun_regular;
        if (!fail
            && (IS_NPC(ch)
                || (number_percent() + percentage) <
                ch->pcdata->learned[gsn_torture]))
        {
                learn_from_success(ch, gsn_torture);
                WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
                WAIT_STATE(victim, PULSE_VIOLENCE);
                act(AT_SKILL,
                    "$N slowly tortures you. The pain is excruciating.",
                    victim, NULL, ch, TO_CHAR);
                act(AT_SKILL, "You torture $N, leaving $M screaming in pain.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_SKILL,
                    "$n tortures $N, leaving $M screaming in agony!", ch,
                    NULL, victim, TO_NOTVICT);

                dam = dice(ch->skill_level[LEADERSHIP_ABILITY] / 10, 4);
                dam = URANGE(0, victim->max_hit - 10, dam);
                victim->hit -= dam;
                victim->max_hit -= dam;

                ch_printf(victim, "You lose %d permanent hit points.", dam);
                ch_printf(ch, "They lose %d permanent hit points.", dam);

        }
        else
        {
                act(AT_SKILL, "$N tries to cut off your finger!", victim,
                    NULL, ch, TO_CHAR);
                act(AT_SKILL, "You mess up big time.", ch, NULL, victim,
                    TO_CHAR);
                act(AT_SKILL, "$n tries to painfully torture $N.", ch, NULL,
                    victim, TO_NOTVICT);
                WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
                global_retcode = multi_hit(victim, ch, TYPE_UNDEFINED);
        }
        return;

}

void      set_name(CHAR_DATA * ch, char *name);
CMDF do_disguise(CHAR_DATA * ch, char *argument)
{
        int       percentage;

        if (IS_NPC(ch))
                return;

        if (IS_SET(ch->pcdata->flags, PCFLAG_NOTITLE))
        {
                send_to_char("You try but the Force resists you.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Change your disguise to what?\n\r", ch);
                return;
        }

        percentage = (int) (ch->pcdata->learned[gsn_disguise]);

        if (number_percent() > percentage)
        {
                send_to_char("You try to disguise yourself but fail.\n\r",
                             ch);
                set_name(ch, ch->name);
                WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
                return;
        }

        if (strlen(argument) > 50)
                argument[50] = '\0';

        learn_from_success(ch, gsn_disguise);
        argument = smash_color(argument);
        set_name(ch, argument);
        send_to_char("Ok.\n\r", ch);
}

CMDF do_mine(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        bool      shovel;
        sh_int    move;

        if (ch->pcdata->learned[gsn_mine] <= 0)
        {
                ch_printf(ch, "You have no idea how to do that.\n\r");
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("And what will you mine the room with?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        shovel = FALSE;
        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->item_type == ITEM_SHOVEL)
                {
                        shovel = TRUE;
                        break;
                }

        obj = get_obj_list_rev(ch, arg, ch->in_room->last_content);
        if (!obj)
        {
                send_to_char("You don't see on here.\n\r", ch);
                return;
        }

        separate_obj(obj);
        if (obj->item_type != ITEM_LANDMINE)
        {
                act(AT_PLAIN, "That's not a landmine!", ch, obj, 0, TO_CHAR);
                return;
        }

        if (!CAN_WEAR(obj, ITEM_TAKE))
        {
                act(AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR);
                return;
        }

        switch (ch->in_room->sector_type)
        {
        case SECT_CITY:
        case SECT_INSIDE:
                send_to_char("The floor is too hard to dig through.\n\r", ch);
                return;
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
        case SECT_UNDERWATER:
                send_to_char("You cannot bury a mine in the water.\n\r", ch);
                return;
        case SECT_AIR:
                send_to_char("What?  Bury a mine in the air?!\n\r", ch);
                return;
        }

        if (obj->weight > (UMAX(5, (can_carry_w(ch) / 10))) && !shovel)
        {
                send_to_char
                        ("You'd need a shovel to bury something that big.\n\r",
                         ch);
                return;
        }

        move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1,
                                                            can_carry_w(ch));
        move = URANGE(2, move, 1000);
        if (move > ch->endurance)
        {
                send_to_char
                        ("You don't have the energy to bury something of that size.\n\r",
                         ch);
                return;
        }
        ch->endurance -= move;

        SET_BIT(obj->extra_flags, ITEM_BURRIED);
        WAIT_STATE(ch, URANGE(10, move / 2, 100));

        STRFREE(obj->armed_by);
        obj->armed_by = STRALLOC(ch->name);

        ch_printf(ch, "You arm and bury %s.\n\r", obj->short_descr);
        act(AT_PLAIN, "$n arms and buries $p.", ch, obj, NULL, TO_ROOM);

        learn_from_success(ch, gsn_mine);

        return;
}


CMDF do_snipe(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *wield;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        sh_int    dir, dist;
        sh_int    max_dist = 3;
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *was_in_room;
        ROOM_INDEX_DATA *to_room;
        CHAR_DATA *victim = NULL;
        int       percentage;
        char      buf[MAX_STRING_LENGTH];
        bool      pfound = FALSE;

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("You'll have to do that elswhere.\n\r", ch);
                return;
        }

        if (get_eq_char(ch, WEAR_DUAL_WIELD) != NULL)
        {
                send_to_char("You can't do that while wielding two weapons.",
                             ch);
                return;
        }

        wield = get_eq_char(ch, WEAR_WIELD);
        if (!wield || wield->item_type != ITEM_WEAPON
            || wield->value[3] != WEAPON_BLASTER)
        {
                send_to_char("You don't seem to be holding a blaster", ch);
                return;
        }

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        if ((dir = get_door(arg)) == -1 || arg2[0] == '\0')
        {
                send_to_char("Usage: snipe <dir> <target>\n\r", ch);
                return;
        }

        if ((pexit = get_exit(ch->in_room, dir)) == NULL)
        {
                send_to_char("Are you expecting to fire through a wall!?\n\r",
                             ch);
                return;
        }

        if (IS_SET(pexit->exit_info, EX_CLOSED))
        {
                send_to_char("Are you expecting to fire through a door!?\n\r",
                             ch);
                return;
        }

        was_in_room = ch->in_room;

        for (dist = 0; dist <= max_dist; dist++)
        {
                if (IS_SET(pexit->exit_info, EX_CLOSED))
                        break;

                if (!pexit->to_room)
                        break;

                to_room = NULL;
                if (pexit->distance > 1)
                        to_room = generate_exit(ch->in_room, &pexit);

                if (to_room == NULL)
                        to_room = pexit->to_room;

                char_from_room(ch);
                char_to_room(ch, to_room);


                if (IS_NPC(ch)
                    && (victim = get_char_room_mp(ch, arg2)) != NULL)
                {
                        pfound = TRUE;
                        break;
                }
                else if (!IS_NPC(ch)
                         && (victim = get_char_room(ch, arg2)) != NULL)
                {
                        pfound = TRUE;
                        break;
                }


                if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                        break;

        }

        char_from_room(ch);
        char_to_room(ch, was_in_room);

        if (!pfound)
        {
                ch_printf(ch, "You don't see that person to the %s!\n\r",
                          dir_name[dir]);
                char_from_room(ch);
                char_to_room(ch, was_in_room);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Shoot yourself ... really?\n\r", ch);
                return;
        }

        if (xIS_SET(victim->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("You can't shoot them there.\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
        {
                act(AT_PLAIN, "$N is your beloved master.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && IS_SET(ch->act, PLR_NICE))
        {
                send_to_char("You feel too nice to do that!\n\r", ch);
                return;
        }

        percentage = IS_NPC(ch) ? 100
                : (int) (ch->pcdata->learned[gsn_snipe]);

        switch (dir)
        {
        case 0:
        case 1:
                dir += 2;
                break;
        case 2:
        case 3:
                dir -= 2;
                break;
        case 4:
        case 7:
                dir += 1;
                break;
        case 5:
        case 8:
                dir -= 1;
                break;
        case 6:
                dir += 3;
                break;
        case 9:
                dir -= 3;
                break;
        }

        char_from_room(ch);
        char_to_room(ch, victim->in_room);

        if (number_percent() < percentage)
        {
                snprintf(buf, MSL, "A blaster shot fires at you from the %s.",
                         dir_name[dir]);
                act(AT_ACTION, buf, victim, NULL, ch, TO_CHAR);
                act(AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR);
                snprintf(buf, MSL, "A blaster shot fires at $N from the %s.",
                         dir_name[dir]);
                act(AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT);

                one_hit(ch, victim, TYPE_UNDEFINED);

                if (char_died(ch))
                        return;

                stop_fighting(ch, TRUE);

                learn_from_success(ch, gsn_snipe);
        }
        else
        {
                act(AT_ACTION, "You fire at $N but don't even come close.",
                    ch, NULL, victim, TO_CHAR);
                snprintf(buf, MSL,
                         "A blaster shot fired from the %s barely misses you.",
                         dir_name[dir]);
                act(AT_ACTION, buf, ch, NULL, victim, TO_ROOM);
                learn_from_failure(ch, gsn_snipe);
        }

        char_from_room(ch);
        char_to_room(ch, was_in_room);

        if (IS_NPC(ch))
                WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
        else
        {
                if (number_percent() < ch->pcdata->learned[gsn_third_attack])
                        WAIT_STATE(ch, 1 * PULSE_PER_SECOND);
                else if (number_percent() <
                         ch->pcdata->learned[gsn_second_attack])
                        WAIT_STATE(ch, 2 * PULSE_PER_SECOND);
                else
                        WAIT_STATE(ch, 3 * PULSE_PER_SECOND);
        }
        if (IS_NPC(victim) && !char_died(victim))
        {
                if (IS_SET(victim->act, ACT_SENTINEL))
                {
                        victim->was_sentinel = victim->in_room;
                        REMOVE_BIT(victim->act, ACT_SENTINEL);
                }

                start_hating(victim, ch);
                start_hunting(victim, ch);

        }

}

/* syntax throw <obj> [direction] [target] */

CMDF do_throw(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        OBJ_DATA *tmpobj;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        sh_int    dir;
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *was_in_room;
        ROOM_INDEX_DATA *to_room;
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];


        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        was_in_room = ch->in_room;

        if (arg[0] == '\0')
        {
                send_to_char("Usage: throw <object> [direction] [target]\n\r",
                             ch);
                return;
        }


        obj = get_eq_char(ch, WEAR_MISSILE_WIELD);
        if (!obj || !nifty_is_name(arg, obj->name))
                obj = get_eq_char(ch, WEAR_HOLD);
        if (!obj || !nifty_is_name(arg, obj->name))
                obj = get_eq_char(ch, WEAR_WIELD);
        if (!obj || !nifty_is_name(arg, obj->name))
                obj = get_eq_char(ch, WEAR_DUAL_WIELD);
        if (!obj || !nifty_is_name(arg, obj->name))
                if (!obj || !nifty_is_name_prefix(arg, obj->name))
                        obj = get_eq_char(ch, WEAR_HOLD);
        if (!obj || !nifty_is_name_prefix(arg, obj->name))
                obj = get_eq_char(ch, WEAR_WIELD);
        if (!obj || !nifty_is_name_prefix(arg, obj->name))
                obj = get_eq_char(ch, WEAR_DUAL_WIELD);
        if (!obj || !nifty_is_name_prefix(arg, obj->name))
        {
                ch_printf(ch,
                          "You don't seem to be holding or wielding %s.\n\r",
                          arg);
                return;
        }

        if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
        {
                act(AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                victim = who_fighting(ch);
                if (char_died(victim))
                        return;
                act(AT_ACTION, "You throw $p at $N.", ch, obj, victim,
                    TO_CHAR);
                act(AT_ACTION, "$n throws $p at $N.", ch, obj, victim,
                    TO_NOTVICT);
                act(AT_ACTION, "$n throw $p at you.", ch, obj, victim,
                    TO_VICT);
        }
        else if (arg2[0] == '\0')
        {
                snprintf(buf, MSL, "$n throws %s at the floor.",
                         obj->short_descr);
                act(AT_ACTION, buf, ch, NULL, NULL, TO_ROOM);
                ch_printf(ch, "You throw %s at the floor.\n\r",
                          obj->short_descr);

                victim = NULL;
        }
        else if ((dir = get_door(arg2)) != -1)
        {
                if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                {
                        send_to_char
                                ("Are you expecting to throw it through a wall!?\n\r",
                                 ch);
                        return;
                }


                if (IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char
                                ("Are you expecting to throw it  through a door!?\n\r",
                                 ch);
                        return;
                }


                switch (dir)
                {
                case 0:
                case 1:
                        dir += 2;
                        break;
                case 2:
                case 3:
                        dir -= 2;
                        break;
                case 4:
                case 7:
                        dir += 1;
                        break;
                case 5:
                case 8:
                        dir -= 1;
                        break;
                case 6:
                        dir += 3;
                        break;
                case 9:
                        dir -= 3;
                        break;
                }

                to_room = NULL;
                if (pexit->distance > 1)
                        to_room = generate_exit(ch->in_room, &pexit);

                if (to_room == NULL)
                        to_room = pexit->to_room;


                char_from_room(ch);
                char_to_room(ch, to_room);

                victim = get_char_room(ch, arg3);

                if (victim)
                {
                        if (is_safe(ch, victim))
                                return;

                        if (IS_AFFECTED(ch, AFF_CHARM)
                            && ch->master == victim)
                        {
                                act(AT_PLAIN, "$N is your beloved master.",
                                    ch, NULL, victim, TO_CHAR);
                                return;
                        }

                        if (!IS_NPC(victim) && IS_SET(ch->act, PLR_NICE))
                        {
                                send_to_char
                                        ("You feel too nice to do that!\n\r",
                                         ch);
                                return;
                        }

                        char_from_room(ch);
                        char_to_room(ch, was_in_room);


                        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
                        {
                                set_char_color(AT_MAGIC, ch);
                                send_to_char
                                        ("You'll have to do that elswhere.\n\r",
                                         ch);
                                return;
                        }

                        to_room = NULL;
                        if (pexit->distance > 1)
                                to_room = generate_exit(ch->in_room, &pexit);

                        if (to_room == NULL)
                                to_room = pexit->to_room;


                        char_from_room(ch);
                        char_to_room(ch, to_room);

                        snprintf(buf, MSL,
                                 "Someone throws %s at you from the %s.",
                                 obj->short_descr, dir_name[dir]);
                        act(AT_ACTION, buf, victim, NULL, ch, TO_CHAR);
                        act(AT_ACTION, "You throw $p at $N.", ch, obj,
                            victim, TO_CHAR);
                        snprintf(buf, MSL, "%s is thrown at $N from the %s.",
                                 obj->short_descr, dir_name[dir]);
                        act(AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT);


                }
                else
                {
                        ch_printf(ch, "You throw %s %s.\n\r",
                                  obj->short_descr, dir_name[get_dir(arg2)]);
                        snprintf(buf, MSL, "%s is thrown from the %s.",
                                 obj->short_descr, dir_name[dir]);
                        act(AT_ACTION, buf, ch, NULL, NULL, TO_ROOM);

                }
        }
        else if ((victim = get_char_room(ch, arg2)) != NULL)
        {
                if (is_safe(ch, victim))
                        return;

                if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
                {
                        act(AT_PLAIN, "$N is your beloved master.", ch, NULL,
                            victim, TO_CHAR);
                        return;
                }

                if (!IS_NPC(victim) && IS_SET(ch->act, PLR_NICE))
                {
                        send_to_char("You feel too nice to do that!\n\r", ch);
                        return;
                }

        }
        else
        {
                ch_printf(ch, "They don't seem to be here!\n\r");
                return;
        }


        if (obj == get_eq_char(ch, WEAR_WIELD)
            && (tmpobj = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
                tmpobj->wear_loc = WEAR_WIELD;

        unequip_char(ch, obj);
        separate_obj(obj);
        obj_from_char(obj);
        obj = obj_to_room(obj, ch->in_room);

        if (obj->item_type != ITEM_GRENADE)
                damage_obj(obj);

/* NOT NEEDED UNLESS REFERING TO OBJECT AGAIN 

   if( obj_extracted(obj) )
      return;
*/
        if (ch->in_room != was_in_room)
        {
                char_from_room(ch);
                char_to_room(ch, was_in_room);
        }

        if (!victim || char_died(victim))
                learn_from_failure(ch, gsn_throw);
        else
        {

                WAIT_STATE(ch, skill_table[gsn_throw]->beats);
                if (IS_NPC(ch)
                    || number_percent() < ch->pcdata->learned[gsn_throw])
                {
                        learn_from_success(ch, gsn_throw);
                        global_retcode =
                                damage(ch, victim,
                                       number_range(obj->weight * 2,
                                                    (obj->weight * 2 +
                                                     ch->perm_str)),
                                       TYPE_HIT);
                }
                else
                {
                        learn_from_failure(ch, gsn_throw);
                        global_retcode = damage(ch, victim, 0, TYPE_HIT);
                }

                if (IS_NPC(victim) && !char_died(victim))
                {
                        if (IS_SET(victim->act, ACT_SENTINEL))
                        {
                                victim->was_sentinel = victim->in_room;
                                REMOVE_BIT(victim->act, ACT_SENTINEL);
                        }

                        start_hating(victim, ch);
                        start_hunting(victim, ch);

                }

        }

        return;

}


CMDF do_pickshiplock(CHAR_DATA * ch, char *argument)
{
        do_pick(ch, argument);
}

CMDF do_hijack(CHAR_DATA * ch, char *argument)
{
        int       percentage;
        SHIP_DATA *ship;
        char      buf[MAX_STRING_LENGTH];
        long      xpgain;
        CLAN_DATA *clan;
        CHAR_DATA *owner;
        ROOM_INDEX_DATA *room;

        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > SHIP_PLATFORM)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if ((ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL)
        {
                send_to_char("&RYou don't seem to be in the pilot seat!\n\r",
                             ch);
                return;
        }

        if (check_pilot(ch, ship))
        {
                send_to_char("&RWhat would be the point of that!\n\r", ch);
                return;
        }

        if (ship->type == MOB_SHIP && get_trust(ch) < 102)
        {
                send_to_char
                        ("&RThis ship isn't pilotable by mortals at this point in time...\n\r",
                         ch);
                return;
        }

        if (ship->ship_class == SHIP_PLATFORM)
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (ship->lastdoc != ship->location)
        {
                send_to_char("&rYou don't seem to be docked right now.\n\r",
                             ch);
                return;
        }

        if (ship->shipstate != SHIP_DOCKED
            && ship->shipstate != SHIP_DISABLED)
        {
                send_to_char("The ship is not docked right now.\n\r", ch);
                return;
        }

        if (ship->shipstate == SHIP_DISABLED)
        {
                send_to_char("The ships drive is disabled .\n\r", ch);
                return;
        }
        if (ship->type == PLAYER_SHIP && ship->lastbuilt != ship->lastroom)
        {
                send_to_char
                        ("This ship is not fully constructed, you can't launch!",
                         ch);
                return;
        }


        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_hijack]);
        if (number_percent() > percentage)
        {
                send_to_char
                        ("You fail to figure out the correct launch code.\n\r",
                         ch);
                learn_from_failure(ch, gsn_hijack);
                return;
        }

        if (ship->ship_class == FIGHTER_SHIP)
                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_starfighters]);
        if (ship->ship_class == MIDSIZE_SHIP)
                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_midships]);
        if (ship->ship_class == CAPITAL_SHIP)
                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_capitalships]);
        if (number_percent() < percentage)
        {

                if (ship->hatchopen)
                {
                        ship->hatchopen = FALSE;
                        snprintf(buf, MSL, "The hatch on %s closes.",
                                 ship->name);
                        echo_to_room(AT_YELLOW,
                                     get_room_index(ship->location), buf);
                        echo_to_room(AT_YELLOW,
                                     get_room_index(ship->entrance),
                                     "The hatch slides shut.");
                }
                room = ship->in_room;
                set_char_color(AT_GREEN, ch);
                send_to_char("Launch sequence initiated.\n\r", ch);
                act(AT_PLAIN,
                    "$n starts up the ship and begins the launch sequence.",
                    ch, NULL, argument, TO_ROOM);
                echo_to_ship(AT_YELLOW, ship,
                             "The ship hums as it lifts off the ground.");
                snprintf(buf, MSL, "%s begins to launch.", ship->name);
                echo_to_room(AT_YELLOW, get_room_index(ship->location), buf);
                ship->shipstate = SHIP_LAUNCH;
                ship->currspeed = ship->realspeed;
                if (ship->ship_class == FIGHTER_SHIP)
                        learn_from_success(ch, gsn_starfighters);
                if (ship->ship_class == MIDSIZE_SHIP)
                        learn_from_success(ch, gsn_midships);
                if (ship->ship_class == CAPITAL_SHIP)
                        learn_from_success(ch, gsn_capitalships);
                xpgain = UMIN(200,
                              (exp_level(ch->skill_level[PIRACY_ABILITY] + 1)
                               - exp_level(ch->skill_level[PIRACY_ABILITY])));
                gain_exp(ch, xpgain, PIRACY_ABILITY);
                ch_printf(ch, "You gain %d piracy experience.", xpgain);
                learn_from_success(ch, gsn_hijack);
                snprintf(buf, MSL,
                         "&CSpaceport Security exclaims, 'Hey! The %s has been hijacked!",
                         ship->name);
                echo_to_room(AT_RED, room, buf);
                if ((clan = get_clan(ship->owner)) != NULL)
                {
                        snprintf(buf, MSL,
                                 "&PSpaceport Security speaks over the organizations network, 'The clan ship %s has been hijacked!'\n\r",
                                 ship->name);
                        echo_to_clan(AT_PINK, buf, clan);
                }
                else if ((owner = get_char_world(ch, ship->owner)) != NULL)
                        ch_printf(owner,
                                  "&b&BSpaceport Security tells you 'Excuse me, but I'm afraid that your ship, %s, has been hijacked at %s.'\n\r",
                                  ship->name, room->name);
                return;
        }
        set_char_color(AT_RED, ch);
        send_to_char("You fail to work the controls properly!\n\r", ch);
        if (ship->ship_class == FIGHTER_SHIP)
                learn_from_failure(ch, gsn_starfighters);
        if (ship->ship_class == MIDSIZE_SHIP)
                learn_from_failure(ch, gsn_midships);
        if (ship->ship_class == CAPITAL_SHIP)
                learn_from_failure(ch, gsn_capitalships);
        return;

}


CMDF do_special_forces(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (ch->backup_wait)
                {
                        send_to_char
                                ("&RYour special forces are already on the way.\n\r",
                                 ch);
                        return;
                }
                /*
                 * players can now do this
                 * if ( !ch->pcdata->clan )
                 * {
                 * send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
                 * return;
                 * }    
                 * 
                 */

                if (!has_comlink(ch))
                {
                        send_to_char("You need a comlink to call someone!",
                                     ch);
                        return;
                }

                if (ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 70)
                {
                        ch_printf(ch,
                                  "&RYou dont have enough credits to send for reinforcements.\n\r");
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_special_forces]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin making the call for reinforcements.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 1, do_special_forces, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou call for reinforcements but nobody answers.\n\r",
                         ch);
                learn_from_failure(ch, gsn_special_forces);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted before you can finish your call.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char("&GYour forces are on the way.\n\r", ch);
        credits = ch->skill_level[LEADERSHIP_ABILITY] * 50;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->gold -= UMIN(credits, ch->gold);

        learn_from_success(ch, gsn_special_forces);


/* removed   
    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_STORMTROOPER;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_TROOPER;
    else
       ch->backup_mob = MOB_VNUM_MERCINARY;
*/

/* This really is a player only thing so only mercs can be hired. */

        ch->backup_mob = MOB_VNUM_SPECIAL_FORCES;


        ch->backup_wait = number_range(1, 2);

}


CMDF do_elite_guard(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if (ch->backup_wait)
                {
                        send_to_char("&RYou already have a guard coming.\n\r",
                                     ch);
                        return;
                }
                /*
                 * removed as players can now have guards.
                 * if ( !ch->pcdata->clan )
                 * {
                 * send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
                 * return;
                 * }    
                 */

                if (!has_comlink(ch))
                {
                        send_to_char("You need a comlink to call someone!",
                                     ch);
                        return;
                }

                if (ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30)
                {
                        ch_printf(ch, "&RYou dont have enough credits.\n\r",
                                  ch);
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_elite_guard]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin making the call for reinforcements.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 1, do_elite_guard, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char("&RYou call for a guard but nobody answers.\n\r",
                             ch);
                learn_from_failure(ch, gsn_elite_guard);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted before you can finish your call.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char("&GYour guard is on the way.\n\r", ch);

        credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->gold -= UMIN(credits, ch->gold);

        learn_from_success(ch, gsn_elite_guard);


/* no longer use this
    
    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_GUARD;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_GUARD;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER;

*/


        ch->backup_mob = MOB_VNUM_ELITE_GUARD;


        ch->backup_wait = 1;

}


CMDF do_jail(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        CLAN_DATA *clan = NULL;
        BOUNTY_DATA *bounty = NULL;
        ROOM_INDEX_DATA *jail = NULL;

        if (IS_NPC(ch))
                return;

        if (!ch->pcdata || (clan = ch->pcdata->clan) == NULL)
        {
                send_to_char
                        ("Only members of organizations can jail their enemies.\n\r",
                         ch);
                return;
        }

        jail = get_room_index(clan->jail);
        if (!jail && clan->mainclan)
                jail = get_room_index(clan->mainclan->jail);

        if (!jail)
        {
                send_to_char
                        ("Your orginization does not have a suitable prison.\n\r",
                         ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Jail who?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("That would be a waste of time.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position >= POS_SLEEPING)
        {
                send_to_char("You will have to stun them first.\n\r", ch);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BOUNTY_POLICE &&
                    !str_cmp(victim->name, bounty->target) &&
                    !str_cmp(clan->name, bounty->source))
                        break;

        if (bounty)
        {
                ch_printf(ch,
                          "You collect the %ld bounty and escort them off to jail.\n\r",
                          bounty->amount);
                ch->gold += bounty->amount;
                remove_wanted(victim, clan);
        }


        send_to_char("You have them escorted off to jail.\n\r", ch);
        act(AT_ACTION,
            "You have a strange feeling that you've been moved.\n\r", ch,
            NULL, victim, TO_VICT);
        act(AT_ACTION, "$n has $N escorted away.\n\r", ch, NULL, victim,
            TO_NOTVICT);

        char_from_room(victim);
        char_to_room(victim, jail);

        act(AT_ACTION,
            "The door opens briefly as $n is shoved into the room.\n\r",
            victim, NULL, NULL, TO_ROOM);
        do_look(victim, "");

        learn_from_success(ch, gsn_jail);

        return;
}

CMDF do_smalltalk(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim = NULL;
        PLANET_DATA *planet = NULL;
        CLAN_DATA *clan = NULL;
        int       percent;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("What would be the point of that.\n\r", ch);
        }

        argument = one_argument(argument, arg1);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char("Create smalltalk with whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (victim->position == POS_FIGHTING)
        {
                send_to_char("They're a little busy right now.\n\r", ch);
                return;
        }


        if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_CITIZEN))
        {
                send_to_char("Diplomacy would be wasted on them.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position <= POS_SLEEPING)
        {
                send_to_char("You might want to wake them first...\n\r", ch);
                return;
        }

        WAIT_STATE(ch, skill_table[gsn_smalltalk]->beats);

        percent = number_percent();

        if (percent - ch->skill_level[DIPLOMACY_ABILITY] + victim->top_level >
            ch->pcdata->learned[gsn_smalltalk])
        {
                /*
                 * Failure.
                 */
                send_to_char
                        ("You attempt to make smalltalk with them.. but are ignored.\n\r",
                         ch);
                act(AT_ACTION,
                    "$n is really getting on your nerves with all this chatter!\n\r",
                    ch, NULL, victim, TO_VICT);
                act(AT_ACTION,
                    "$n asks $N about the weather but is ignored.\n\r", ch,
                    NULL, victim, TO_NOTVICT);

                if (victim->alignment < -500
                    && victim->top_level >= ch->top_level + 5)
                        command_printf(ch, "yell SHUT UP %s!", ch->name);

                return;
        }

        send_to_char("You strike up a short conversation with them.\n\r", ch);
        act(AT_ACTION, "$n smiles at you and says, 'hello'.\n\r", ch, NULL,
            victim, TO_VICT);
        act(AT_ACTION, "$n chats briefly with $N.\n\r", ch, NULL, victim,
            TO_NOTVICT);

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || !ch->in_room->area || !ch->in_room->area->planet)
                return;

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;

        if (clan != planet->governed_by)
                return;

        planet->pop_support += 2;
        send_to_char
                ("Popular support for your organization increases slightly.\n\r",
                 ch);

        gain_exp(ch, victim->top_level * 10, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  victim->top_level * 10);

        learn_from_success(ch, gsn_smalltalk);

        if (planet->pop_support > 100)
                planet->pop_support = 100;
}

CMDF do_propeganda(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percent;

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || !ch->in_room->area || !ch->in_room->area->planet)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char("Spread propeganda to who?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (victim->position == POS_FIGHTING)
        {
                send_to_char("They're a little busy right now.\n\r", ch);
                return;
        }


        if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_CITIZEN))
        {
                send_to_char("Diplomacy would be wasted on them.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position <= POS_SLEEPING)
        {
                send_to_char("You might want to wake them first...\n\r", ch);
                return;
        }

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;

        snprintf(buf, MSL, ", and the evils of %s",
                 planet->governed_by ? planet->governed_by->
                 name : "their current leaders");
        ch_printf(ch, "You speak to them about the benefits of the %s%s.\n\r",
                  ch->pcdata->clan->name,
                  planet->governed_by == clan ? "" : buf);
        act(AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL,
            victim, TO_VICT);
        act(AT_ACTION, "$n tells $N about their organization.\n\r", ch, NULL,
            victim, TO_NOTVICT);

        WAIT_STATE(ch, skill_table[gsn_propeganda]->beats);

        percent = number_percent();

        if (percent - get_curr_cha(ch) + victim->top_level / 2 >
            ch->pcdata->learned[gsn_propeganda])
        {

/*                if (planet->governed_by != clan)
                {
                        snprintf(buf, MSL, "%s is a traitor!", ch->name);
                }*/

                return;
        }

        if (planet->governed_by == clan)
        {
                planet->pop_support += (int) (.5 + ch->top_level / 50);
                send_to_char
                        ("Popular support for your organization increases.\n\r",
                         ch);
        }
        else
        {
                planet->pop_support -= ch->top_level / 50;
                send_to_char
                        ("Popular support for the current government decreases.\n\r",
                         ch);
        }

        gain_exp(ch, victim->top_level * 100, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  victim->top_level * 100);

        learn_from_success(ch, gsn_propeganda);

        if (planet->pop_support > 100)
                planet->pop_support = 100;
        if (planet->pop_support < -100)
                planet->pop_support = -100;

}

CMDF do_bribe(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percent, amount;

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || !ch->in_room->area || !ch->in_room->area->planet)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Bribe who how much?\n\r", ch);
                return;
        }

        amount = atoi(argument);

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (amount <= 0)
        {
                send_to_char
                        ("A little bit more money would be a good plan.\n\r",
                         ch);
                return;
        }

        if (ch->gold < amount)
        {
                send_to_char("You don't have that many credits!\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (victim->position == POS_FIGHTING)
        {
                send_to_char("They're a little busy right now.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position <= POS_SLEEPING)
        {
                send_to_char("You might want to wake them first...\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_CITIZEN))
        {
                send_to_char("Diplomacy would be wasted on them.\n\r", ch);
                return;
        }

        ch->gold -= amount;
        victim->gold += amount;

        ch_printf(ch, "You give them a small gift on behalf of %s.\n\r",
                  ch->pcdata->clan->name);
        act(AT_ACTION, "$n offers you a small bribe.\n\r", ch, NULL, victim,
            TO_VICT);
        act(AT_ACTION, "$n gives $N some money.\n\r", ch, NULL, victim,
            TO_NOTVICT);

        if (!IS_NPC(victim))
                return;

        WAIT_STATE(ch, skill_table[gsn_bribe]->beats);


        percent = number_percent();
        if (percent - amount + victim->top_level >
            ch->pcdata->learned[gsn_bribe])
                return;

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;


        if (clan == planet->governed_by)
        {
                planet->pop_support += URANGE((int) 0.1, amount / 1000, 2);
                send_to_char
                        ("Popular support for your organization increases slightly.\n\r",
                         ch);

                amount = UMIN(amount,
                              (exp_level
                               (ch->skill_level[DIPLOMACY_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[DIPLOMACY_ABILITY])));

                gain_exp(ch, amount, DIPLOMACY_ABILITY);
                ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                          amount);

                learn_from_success(ch, gsn_bribe);
        }

        if (planet->pop_support > 100)
                planet->pop_support = 100;
}

CMDF do_seduce(CHAR_DATA * ch, char *argument)
{
        AFFECT_DATA af;
        int       percentage;
        int       level;
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        CHAR_DATA *rch;

        if (argument[0] == '\0')
        {
                send_to_char("Seduce who?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("You like yourself even better!\n\r", ch);
                return;
        }

        if (IS_SET(victim->immune, RIS_CHARM))
        {
                send_to_char("They seem to be immune to such acts.\n\r", ch);
                act(AT_MAGIC,
                    "$n is trying to seduce you but just looks sleazy.", ch,
                    NULL, victim, TO_NOTVICT);
                act(AT_MAGIC,
                    "$n is trying to seduce $n but just looks sleazy.", ch,
                    NULL, victim, TO_VICT);
                return;
        }

        if (find_keeper(victim) != NULL)
        {
                send_to_char
                        ("They have been trained against such things!\n\r",
                         ch);
                act(AT_MAGIC,
                    "$n is trying to seduce $n but just looks sleazy.", ch,
                    NULL, victim, TO_VICT);
                return;
        }

        if (!IS_NPC(victim) && !IS_NPC(ch))
        {
                act(AT_MAGIC,
                    "$n is trying to seduce you but just looks sleazy.", ch,
                    NULL, victim, TO_NOTVICT);
                act(AT_MAGIC,
                    "$n is trying to seduce $n but just looks sleazy.", ch,
                    NULL, victim, TO_VICT);
                send_to_char("I don't think so...\n\r", ch);
                return;
        }

        level = !IS_NPC(ch) ? (int) ch->pcdata->learned[gsn_seduce] : ch->
                top_level;
        percentage = ris_save(victim, level, RIS_CHARM);

        if (IS_AFFECTED(victim, AFF_CHARM) || percentage == 1000
            || IS_AFFECTED(ch, AFF_CHARM) || ch->top_level < victim->top_level
            || circle_follow(victim, ch)
            || saves_spell_staff(percentage, victim)
            || ch->sex == victim->sex)
        {
                send_to_char("&w&BYou failed.\n\r", ch);
                act(AT_MAGIC,
                    "$n is trying to seduce you but just looks sleazy.", ch,
                    NULL, victim, TO_NOTVICT);
                act(AT_MAGIC,
                    "$n is trying to seduce $n but just looks sleazy.", ch,
                    NULL, victim, TO_VICT);
                learn_from_failure(ch, gsn_seduce);
                return;
        }

        if (victim->master)
                stop_follower(victim);
        add_follower(victim, ch);
        for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
        {
                if (rch->master == ch && IS_AFFECTED(rch, AFF_CHARM)
                    && rch != victim)
                {
                        send_to_char("You snap out of it.\n\r", rch);
                        ch_printf(ch, "&B%s becomes less dazed.\n\r",
                                  PERS(rch, ch));
                        stop_follower(rch);
                }
        }
        af.type = gsn_seduce;
        af.duration = (int) ((number_fuzzy((level + 1) / 3) + 1) * DUR_CONV);
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);
        act(AT_MAGIC, "$n just seems so attractive...", ch, NULL, victim,
            TO_VICT);
        act(AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM);
        send_to_char("Ok.\n\r", ch);
        learn_from_success(ch, gsn_seduce);
        snprintf(buf, MSL, "%s has seduced %s.", ch->name, victim->name);
        log_string_plus(buf, LOG_NORMAL, ch->top_level);

        return;

}


CMDF do_mass_propeganda(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *rch;
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       victims = 0;

        argument = NULL;

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || !ch->in_room->area || !ch->in_room->area->planet)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }


        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;

        snprintf(buf, MSL, ", and the evils of %s",
                 planet->governed_by ? planet->governed_by->
                 name : "their current leaders");
        ch_printf(ch,
                  "You speak to the people about the benefits of the %s%s.\n\r",
                  ch->pcdata->clan->name,
                  planet->governed_by == clan ? "" : buf);
        act(AT_ACTION, "$n speaks about their organization.\n\r", ch, NULL,
            NULL, TO_ROOM);

        WAIT_STATE(ch, skill_table[gsn_masspropeganda]->beats);

        if (number_percent() < ch->pcdata->learned[gsn_masspropeganda])
        {
                for (rch = ch->in_room->first_person; rch;
                     rch = rch->next_in_room)
                {
                        if (rch == ch)
                                continue;

                        if (!IS_NPC(rch) || !IS_SET(rch->act, ACT_CITIZEN))
                                continue;

                        if (can_see(ch, rch))
                                victims++;
                        else
                                continue;
                }

                if (planet->governed_by == clan)
                {
                        planet->pop_support +=
                                (int) (.5 + ch->top_level / 10) * victims;
                        send_to_char
                                ("Popular support for your organization increases.\n\r",
                                 ch);
                }
                else
                {
                        planet->pop_support -= (ch->top_level / 10) * victims;
                        send_to_char
                                ("Popular support for the current government decreases.\n\r",
                                 ch);
                }

                gain_exp(ch, ch->top_level * 100, DIPLOMACY_ABILITY);
                ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                          ch->top_level * 100);

                learn_from_success(ch, gsn_masspropeganda);

                if (planet->pop_support > 100)
                        planet->pop_support = 100;
                if (planet->pop_support < -100)
                        planet->pop_support = -100;
                save_planet(planet, FALSE);
                return;
        }
        else
        {
                send_to_char
                        ("They don't even seem interested in what you have to say.\n\r",
                         ch);
                return;
        }
        return;
}


CMDF do_changesex(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage;

        one_argument(argument, arg);
        if (IS_NPC(ch))
                return;


        if (argument[0] == '\0')
        {
                send_to_char
                        ("Change your sex to what? Either Male, Female, or Neutral.\n\r",
                         ch);
                return;
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_changesex]);


        if (number_percent() > percentage)
        {
                send_to_char("You try to disguise your sex but fail.\n\r",
                             ch);
                return;
        }

        if (!str_cmp(argument, "male"))
        {
                ch->sex = 1;
        }
        else if (!str_cmp(argument, "female"))
        {
                ch->sex = 2;
        }
        else if (!str_cmp(arg, "neutral"))
        {
                ch->sex = 0;
        }
        else
                send_to_char("You can not change your sex to that.\n\r", ch);


        send_to_char("Ok.\n\r", ch);

        learn_from_success(ch, gsn_changesex);
}

CMDF do_research(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        SHIP_DATA *ship;
        int       count;


        if (argument[0] == '\0')
        {
                send_to_char
                        ("You must input the name of a player online.\n\r",
                         ch);
                return;
        }

        snprintf(buf, MSL, "0.%s", argument);

        if ((victim = get_char_world(ch, buf)) == NULL)
        {
                send_to_char
                        ("You can't find that entry in the galactic net.\n\r",
                         ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("That's not a player!\n\r", ch);
                return;
        }

		if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
			send_to_char("Your spy cannot find them.\n\r", ch);
			return;
		}
        if (ch->gold < 2000)
        {
                send_to_char
                        ("You do not have enough money to pay for your spy!\n\r",
                         ch);
                return;
        }

        WAIT_STATE(ch, skill_table[gsn_gather_intelligence]->beats);

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_gather_intelligence]);

        if (number_percent() > percentage)
        {
                send_to_char("&RYour spy comes to you with information.\n\r",
                             ch);
                send_to_char
                        ("&RThey start telling you something, and you detect their lie.\n\r",
                         ch);
                send_to_char
                        ("&RWhile you accuse them of lying, they steal some of your money.\n\r",
                         ch);
                learn_from_failure(ch, gsn_gather_intelligence);
                ch->gold -= 1000;
                return;
        }

        send_to_char("&b[&B|\n\r", ch);
        send_to_char("&b[&B|                  &c Intelligence Report\n\r",
                     ch);
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        if (victim->pcdata->clan)
                ch_printf(ch,
                          "&b[&B|&cName&C:&z %-23s&B| &cClan&C:&z %-27s\n\r",
                          victim->name, victim->pcdata->clan->name);
        else
                ch_printf(ch, "&b[&B|&cName&C:&z %-23s&B\n\r", victim->name);
        ch_printf(ch, "&b[&B|&cSex&C:&z %-24s&B| &cAge&C:&z %-28d\n\r",
                  victim->sex == SEX_MALE ? "Male" : victim->sex ==
                  SEX_FEMALE ? "Female" : "Neuter", get_age(victim));
        ch_printf(ch, "&b[&B|&cRace&C:&z %-23s&B| &cSpouse&C:&z %-22s\n\r",
                  victim->race->name(), victim->pcdata->spouse);
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        ch_printf(ch,
                  "&b[&B|&cBank Amount&C:&z %-16s&B|&cAccount number range 1&C: &z??? - ???\n\r",
                  num_punct(victim->pcdata->bank));
        ch_printf(ch,
                  "&b[&B|&cCredits Held&C:&z %-15s&B|&b====================&B|&c2&C: &z??? - ???\n\r",
                  num_punct(victim->gold));
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        send_to_char("&b[&B|&cShips: &b\n\r", ch);
        count = 0;
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (str_cmp(ship->owner, victim->name))
                {
                        continue;
                }

                if (ship->type == MOB_SHIP)
                        continue;
                ch_printf(ch, "&b[&B|&c%s \n\r", ship->name);

                count++;
        }

        if (!count)
        {
                send_to_char
                        ("&b[&B|&cThere are no ships owned by your target.\n\r",
                         ch);
        }
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        send_to_char("&b[&B|                      &cTracking Target... \n\r",
                     ch);
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        ch_printf(ch, "&b[&B|&cLocation&C:&z %-68s\n\r",
                  victim->in_room->name);
        send_to_char
                ("&b[&B|&b================================================================\n\r",
                 ch);
        send_to_char("&b[&B|                          &cEnd Report\n\r\n\r",
                     ch);
        ch->gold -= 2000;

        {
                long      xpgain;

                xpgain = UMIN((ch->pcdata->learned[gsn_gather_intelligence]) *
                              200,
                              (exp_level
                               (ch->skill_level[LEADERSHIP_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[LEADERSHIP_ABILITY])));
                gain_exp(ch, xpgain, LEADERSHIP_ABILITY);
                ch_printf(ch, "You gain %d leadership experience.", xpgain);
        }

        learn_from_success(ch, gsn_gather_intelligence);
        return;

}

CMDF do_makeknife(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage;
        bool      checktool, checksteel, checkplast, checkoven;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf2;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makeknife <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checksteel = FALSE;
                checkplast = FALSE;
                checkoven = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checksteel = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST)
                                checkplast = TRUE;

                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a vibro-blade.\n\r",
                                 ch);
                        return;
                }

                if (!checksteel)
                {
                        send_to_char
                                ("&RYou need something to make the blade of.\n\r",
                                 ch);
                        return;
                }

                if (!checkplast)
                {
                        send_to_char("&RYou need something for a handle.\n\r",
                                     ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the metal.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeknife]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a knife.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makeknife, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeblade);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makeknife]);
        vnum = 10434;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checksteel = FALSE;
        checkplast = FALSE;
        checkoven = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checksteel == FALSE)
                {
                        checksteel = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_DURAPLAST && checkplast == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkplast = TRUE;
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeknife]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checksteel)
            || (!checkplast) || (!checkoven))
        {
                send_to_char("&RYou wield your newly created knife.\n\r", ch);
                send_to_char
                        ("&RYou hold it up to the light to see the blade glitter.\n\r",
                         ch);
                send_to_char
                        ("&RHowever, you notice a very large crack in the metal.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makeknife);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 3;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, " knife", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was left here.", MSL);
        obj->description = STRALLOC(buf);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = get_atype("backstab");
        paf->modifier = level / 3;
        paf->bitvector = 0;
        paf->next = NULL;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("hitroll");
        paf2->modifier = +2;
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;
        obj->value[1] = (int) (level / 10 + 10);    /* min dmg  */
        obj->value[2] = (int) (level / 5 + 20); /* max dmg */
        obj->value[3] = 5;
        obj->cost = obj->value[2] * 10;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created knife.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a knife.", ch, NULL, argument,
            TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 200,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makeknife);
}


CMDF do_makepike(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       level, percentage;
        bool      checktool, checksteel, checkplast, checkoven, checkbatt;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum, bonus = 0;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf2;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makepike <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checksteel = FALSE;
                checkplast = FALSE;
                checkoven = FALSE;
                checkbatt = FALSE;
                bonus = 0;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checksteel = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST)
                                checkplast = TRUE;

                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                }

                if (!checktool)
                {
                        send_to_char("&RYou need toolkit to make a pike.\n\r",
                                     ch);
                        return;
                }

                if (!checksteel)
                {
                        send_to_char
                                ("&RYou need something to make the blade of.\n\r",
                                 ch);
                        return;
                }

                if (!checkplast)
                {
                        send_to_char("&RYou need something for a handle.\n\r",
                                     ch);
                        return;
                }

                if (!checkoven)
                {
                        send_to_char
                                ("&RYou need a small furnace to heat the metal.\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makepike]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a pike.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makepike, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makepike);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makepike]);
        vnum = 10435;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checksteel = FALSE;
        checkplast = FALSE;
        checkoven = FALSE;
        checkbatt = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checksteel == FALSE)
                {
                        checksteel = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_DURAPLAST && checkplast == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkplast = TRUE;
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                        checkplast = TRUE;
                        bonus = level / 5;
                }

        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makepike]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checksteel)
            || (!checkplast) || (!checkoven))
        {
                send_to_char("&RYou wield your newly created pike.\n\r", ch);
                send_to_char
                        ("&RYou hold it up to the light to see the blade glitter.\n\r",
                         ch);
                send_to_char
                        ("&RHowever, you notice a very large crack in the metal.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makepike);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 15;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MIL);
        mudstrlcat(buf, " force pike", MSL);
        obj->name = STRALLOC(smash_color(buf));
        mudstrlcpy(buf, arg, MIL);
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(buf);
        STRFREE(obj->description);
        mudstrlcat(buf, " was left here.", MSL);
        obj->description = STRALLOC(buf);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = get_atype("grip");
        paf->modifier = level;
        paf->bitvector = 0;
        paf->next = NULL;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        CREATE(paf2, AFFECT_DATA, 1);
        paf2->type = -1;
        paf2->duration = -1;
        paf2->location = get_atype("hitroll");
        paf2->modifier = +2;
        paf2->bitvector = 0;
        paf2->next = NULL;
        LINK(paf2, obj->first_affect, obj->last_affect, next, prev);
        ++top_affect;
        obj->value[0] = INIT_WEAPON_CONDITION;
        obj->value[1] = (int) (level / 10 + 10 + bonus);    /* min dmg  */
        obj->value[2] = (int) (level / 5 + 20 + bonus); /* max dmg */
        obj->value[3] = 11;
        obj->cost = obj->value[2] * 10;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created pike.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a pike.", ch, NULL, argument,
            TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 200,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makepike);
}

CMDF do_itemrepair(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj, *cobj;
        char      arg[MAX_STRING_LENGTH];
        int       percentage;
        bool      checktool, checksew;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("Repair what?\n\r", ch);
                        return;
                }

                if ((obj = get_obj_carry(ch, arg)) == NULL)
                {
                        send_to_char("&RYou do not have that item.\n\r&w",
                                     ch);
                        return;
                }

                checktool = FALSE;
                checksew = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                if (obj->item_type != ITEM_WEAPON
                    && obj->item_type != ITEM_ARMOR)
                {
                        send_to_char
                                ("&RYou can only repair weapons and armor.&w\n\r",
                                 ch);
                        return;
                }

                if (obj->item_type == ITEM_WEAPON
                    && obj->value[0] == INIT_WEAPON_CONDITION)
                {
                        send_to_char
                                ("&WIt does not appear to be in need of repair.\n\r",
                                 ch);
                        return;
                }
                else if (obj->item_type == ITEM_ARMOR
                         && obj->value[0] == obj->value[1])
                {
                        send_to_char
                                ("&WIt does not appear to be in need of repair.\n\r",
                                 ch);
                        return;
                }

                for (cobj = ch->last_carrying; cobj;
                     cobj = cobj->prev_content)
                {
                        if (cobj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (cobj->item_type == ITEM_THREAD)
                                checksew = TRUE;
                }

                if (!checktool && obj->item_type == ITEM_WEAPON)
                {
                        send_to_char
                                ("&w&RYou need a toolkit to repair weapons.\n\r",
                                 ch);
                        return;
                }

                if (!checksew && obj->item_type == ITEM_ARMOR)
                {
                        send_to_char
                                ("&w&RYou need a needle and thread to repair armor.\n\r",
                                 ch);
                        return;
                }

                send_to_char
                        ("&W&GYou begin to repair your equipment...&W\n\r",
                         ch);
                act(AT_PLAIN,
                    "$n takes $s tools and begins to repair something.", ch,
                    NULL, argument, TO_ROOM);
                add_timer(ch, TIMER_DO_FUN, 5, do_itemrepair, 1);
                ch->dest_buf = str_dup(arg);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interrupted and fail to finish your work.\n\r",
                         ch);
                return;

        }

        ch->substate = SUB_NONE;

        percentage =
                IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_repair]);

        if (number_percent() > percentage * 2)
        {
                send_to_char
                        ("&RYou realize your attempts to repair your equipment have had no effect.\n\r",
                         ch);
                learn_from_failure(ch, gsn_repair);
                return;
        }

        if ((obj = get_obj_carry(ch, arg)) == NULL)
        {
                send_to_char("&RError S3. Report to Administration\n\r&w",
                             ch);
                return;
        }

        switch (obj->item_type)
        {
        default:
                send_to_char("Error S4. Contact Administration.\n\r", ch);
                return;
        case ITEM_ARMOR:
                obj->value[0] = obj->value[1];
                break;
        case ITEM_WEAPON:
                obj->value[0] = INIT_WEAPON_CONDITION;
                break;
        case ITEM_DEVICE:
                obj->value[2] = obj->value[1];
                break;
        }

        send_to_char
                ("&GYou repair your equipment back to fine condition.&W\n\r",
                 ch);

        {
                long      xpgain;

                xpgain = UMIN(number_percent() * 600,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_repair);
}


CMDF do_causedissension(CHAR_DATA * ch, char *argument)
{
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percent;

        argument = NULL;
        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (!ch->pcdata->clan)
        {
                send_to_char("You do not belong to a clan.\n\r", ch);
                return;
        }

        if (!ch->in_room || !ch->in_room->area)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;
        if (!planet || !planet->governed_by)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        switch (ch->substate)
        {
        default:

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }

                if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
                {
                        send_to_char
                                ("This isn't a good place to do that.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == POS_FIGHTING)
                {
                        send_to_char("Interesting combat technique.\n\r", ch);
                        return;
                }

                if (planet->governed_by == clan)
                {
                        send_to_char
                                ("Why would you do that to your own planet?!?.\n\r",
                                 ch);
                        return;
                }

                if (ch->position <= POS_SLEEPING)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }

                if (planet->attgovern)
                {
                        send_to_char
                                ("This planet is in a battle, you can not speak with the troops?\n\r",
                                 ch);
                        return;
                }

                if (planet->defbattalions <= 0)
                {
                        send_to_char("This planet has no troops!\n\r", ch);
                        return;
                }

                percent =
                        IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_causedissension]);

                if (number_percent() < percent)
                {
                        send_to_char
                                ("&GYou sneak around the barracks of this planet, speaking with the soldiers.\n\r",
                                 ch);
                        act(AT_PLAIN, "$n sneaks around suspiciously.", ch,
                            NULL, NULL, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 8, do_causedissension, 1);
                        return;
                }

                send_to_char
                        ("&RYou try to get into a barracks, but are denied entry.\n\r",
                         ch);
                learn_from_failure(ch, gsn_causedissension);
                return;

        case 1:
                break;

        case SUB_TIMER_DO_ABORT:
                ch->substate = SUB_NONE;
                send_to_char("&RYou stop and leave your task.\n\r", ch);
                return;
        }

        ch->substate = SUB_NONE;

        ch_printf(ch,
                  "You speak to the troops about %s's tyranical reign.\n\r",
                  planet->governed_by->name ? planet->governed_by->
                  name : "their");

        percent = number_percent();

        if (percent - get_curr_cha(ch) >
            ch->pcdata->learned[gsn_causedissension])
        {
                send_to_char
                        ("The troops yell at you, and you flee for your life.",
                         ch);
                return;
        }

        planet->defbattalions -= 1;
        send_to_char("A small faction of troops desert their commands.\n\r",
                     ch);
        save_planet(planet, FALSE);

        gain_exp(ch, ch->top_level * 100, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  ch->top_level * 100);

        learn_from_success(ch, gsn_causedissension);

}

CMDF do_causedessertion(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percentage;

        argument = NULL;

        switch (ch->substate)
        {
        default:

                if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
                    || !ch->in_room->area || !ch->in_room->area->planet)
                {
                        send_to_char("What would be the point of that.\n\r",
                                     ch);
                        return;
                }

                if ((clan = ch->pcdata->clan->mainclan) == NULL)
                        clan = ch->pcdata->clan;

                planet = ch->in_room->area->planet;

                if (ch->mount)
                {
                        send_to_char("You can't do that while mounted.\n\r",
                                     ch);
                        return;
                }

                if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
                {
                        send_to_char
                                ("This isn't a good place to do that.\n\r",
                                 ch);
                        return;
                }

                if (ch->position == POS_FIGHTING)
                {
                        send_to_char("Interesting combat technique.\n\r", ch);
                        return;
                }

                if (ch->in_room->area->planet->governed_by == clan)
                {
                        send_to_char
                                ("Why would you do that to your own planet?!?.\n\r",
                                 ch);
                        return;
                }
                if (ch->position <= POS_SLEEPING)
                {
                        send_to_char("In your dreams or what?\n\r", ch);
                        return;
                }
                if (ch->in_room->area->planet->attgovern)
                {
                        send_to_char
                                ("This planet is in a battle, you can not speak with the troops?\n\r",
                                 ch);
                        return;
                }
                if (ch->in_room->area->planet->defbattalions <= 0)
                {
                        send_to_char("This planet has no troops!\n\r", ch);
                        return;
                }
                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_causedesertion]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou sneak around the barracks of this planet, speaking with the soldiers angrily.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n sneaks around suspiciously, with a look of death in $s eyes.",
                            ch, NULL, NULL, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 8, do_causedessertion, 1);
                        return;
                }
                send_to_char
                        ("&RYou try to get into a barracks, but are denied entry.\n\r",
                         ch);
                learn_from_failure(ch, gsn_causedesertion);
                return;

        case 1:
                break;

        case SUB_TIMER_DO_ABORT:
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou stop and leave your task, fuming with anger.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;

        ch_printf(ch,
                  "You speak to the troops about %s's tyranical reign, and the right of the troops to rise up.\n\r",
                  planet->governed_by->name ? planet->governed_by->
                  name : "their");

        percentage = number_percent();

        if (percentage - get_curr_cha(ch) + 20 >
            ch->pcdata->learned[gsn_causedesertion])
        {
                send_to_char
                        ("The troops yell at you, and you flee for your life.",
                         ch);
                return;
        }

        planet->defbattalions -= 1;
        planet->attbattalions += 1;
        planet->attgovern = clan;
        send_to_char
                ("A small faction of troops desert their commands, and become guerilla fighters for your cause.\n\r",
                 ch);
        snprintf(buf, MSL,
                 "A group of troops on %s has desserted to the %s cause!",
                 planet->name, clan->name);
        echo_to_all(AT_RED, buf, 0);
        save_planet(planet, FALSE);

        gain_exp(ch, ch->top_level * 100, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  ch->top_level * 100);

        learn_from_success(ch, gsn_causedesertion);
}

CMDF do_boostmorale(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percent;

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || !ch->in_room->area || !ch->in_room->area->planet)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }


        if (ch->in_room->area->planet->governed_by != ch->pcdata->clan)
        {
                send_to_char
                        ("Why would you do that on another clans planets?",
                         ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char("Who do you want to boost morale with?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (victim->position == POS_FIGHTING)
        {
                send_to_char("They're a little busy right now.\n\r", ch);
                return;
        }


        if (!IS_SET(victim->act, ACT_CITIZEN))
        {
                send_to_char("Diplomacy would be wasted on them.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (victim->position <= POS_SLEEPING)
        {
                send_to_char("You might want to wake them first...\n\r", ch);
                return;
        }

        if ((clan = ch->pcdata->clan->mainclan) == NULL)
                clan = ch->pcdata->clan;

        planet = ch->in_room->area->planet;

        ch_printf(ch,
                  "You speak to them about the benifits of the %s, and their family and friends.\n\r",
                  ch->pcdata->clan->name,
                  planet->governed_by == clan ? "" : buf);
        act(AT_ACTION, "$n speaks about $s organization.\n\r", ch, NULL,
            victim, TO_VICT);
        act(AT_ACTION, "$n tells $N about their organization.\n\r", ch, NULL,
            victim, TO_NOTVICT);


        percent = number_percent();

        if (percent - get_curr_cha(ch) + victim->top_level / 2 >
            ch->pcdata->learned[gsn_boostmorale])
        {
                send_to_char
                        ("You mumble your words too much, and they walk away",
                         ch);
                return;
        }

        for (planet = first_planet; planet; planet = planet->next)
        {
                if (planet->governed_by != clan)
                        continue;
                planet->pop_support += (int) (.5 + ch->top_level / 50);
                if (planet->pop_support > 100)
                        planet->pop_support = 100;
        }
        send_to_char
                ("Popular support for your organization increases throughout the galaxy.\n\r",
                 ch);

        gain_exp(ch, victim->top_level * 100, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  victim->top_level * 100);

        learn_from_success(ch, gsn_boostmorale);
}


CMDF do_throwsaber(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *wield;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        sh_int    dir, dist;
        sh_int    max_dist = 3;
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *was_in_room;
        ROOM_INDEX_DATA *to_room;
        CHAR_DATA *victim = NULL;
        int       percentage;
        char      buf[MAX_STRING_LENGTH];
        bool      pfound = FALSE;

        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("You'll have to do that elswhere.\n\r", ch);
                return;
        }

        wield = get_eq_char(ch, WEAR_WIELD);
        if (!wield || wield->item_type != ITEM_WEAPON
            || wield->value[3] != WEAPON_LIGHTSABER)
        {
                wield = get_eq_char(ch, WEAR_DUAL_WIELD);
                if (!wield || wield->item_type != ITEM_WEAPON
                    || wield->value[3] != WEAPON_LIGHTSABER)
                {
                        send_to_char
                                ("You don't seem to be holding a lightsaber with which to throw",
                                 ch);
                        return;
                }
        }

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        if ((dir = get_door(arg)) == -1 || arg2[0] == '\0')
        {
                send_to_char("Usage: throwsaber <dir> <target>\n\r", ch);
                return;
        }

        if ((pexit = get_exit(ch->in_room, dir)) == NULL)
        {
                send_to_char
                        ("Are you expecting to throw it through a wall!?\n\r",
                         ch);
                return;
        }

        if (IS_SET(pexit->exit_info, EX_CLOSED))
        {
                send_to_char
                        ("Are you expecting to throw it through a door!?\n\r",
                         ch);
                return;
        }

        was_in_room = ch->in_room;

        for (dist = 0; dist <= max_dist; dist++)
        {
                if (IS_SET(pexit->exit_info, EX_CLOSED))
                        break;

                if (!pexit->to_room)
                        break;

                to_room = NULL;
                if (pexit->distance > 1)
                        to_room = generate_exit(ch->in_room, &pexit);

                if (to_room == NULL)
                        to_room = pexit->to_room;

                char_from_room(ch);
                char_to_room(ch, to_room);


                if (IS_NPC(ch)
                    && (victim = get_char_room_mp(ch, arg2)) != NULL)
                {
                        pfound = TRUE;
                        break;
                }
                else if (!IS_NPC(ch)
                         && (victim = get_char_room(ch, arg2)) != NULL)
                {
                        pfound = TRUE;
                        break;
                }


                if ((pexit = get_exit(ch->in_room, dir)) == NULL)
                        break;

        }

        char_from_room(ch);
        char_to_room(ch, was_in_room);

        if (!pfound)
        {
                ch_printf(ch, "You don't see that person to the %s!\n\r",
                          dir_name[dir]);
                char_from_room(ch);
                char_to_room(ch, was_in_room);
                return;
        }

        if (victim == ch)
        {
                send_to_char
                        ("Throw it at yourself? What are you, retarded?\n\r",
                         ch);
                return;
        }

        if (xIS_SET(victim->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char
                        ("You can't throw your saber at them while they are there.\n\r",
                         ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
        {
                act(AT_PLAIN, "$N is your beloved master.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && IS_SET(ch->act, PLR_NICE))
        {
                send_to_char("You feel too nice to do that!\n\r", ch);
                return;
        }

        percentage = IS_NPC(ch) ? 100
                : (int) (ch->pcdata->learned[gsn_throwsaber]);

        switch (dir)
        {
        case 0:
        case 1:
                dir += 2;
                break;
        case 2:
        case 3:
                dir -= 2;
                break;
        case 4:
        case 7:
                dir += 1;
                break;
        case 5:
        case 8:
                dir -= 1;
                break;
        case 6:
                dir += 3;
                break;
        case 9:
                dir -= 3;
                break;
        }

        char_from_room(ch);
        char_to_room(ch, victim->in_room);

        if (number_percent() < percentage)
        {
                snprintf(buf, MSL,
                         "A lightsaber comes flying at you from the %s.",
                         dir_name[dir]);
                act(AT_ACTION, buf, victim, NULL, ch, TO_CHAR);
                act(AT_ACTION, "You throw your saber at $N.", ch, NULL,
                    victim, TO_CHAR);
                snprintf(buf, MSL, "A lightsaber flies at $N from the %s.",
                         dir_name[dir]);
                act(AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT);

                one_hit(ch, victim, TYPE_UNDEFINED);

                if (char_died(ch))
                        return;

                stop_fighting(ch, TRUE);

                learn_from_success(ch, gsn_throwsaber);
        }
        else
        {
                act(AT_ACTION,
                    "You throw your saber $N but miss them, and the saber returns to you.",
                    ch, NULL, victim, TO_CHAR);
                snprintf(buf, MSL,
                         "A lightsaber comes flying at you from the %s and barely misses you.",
                         dir_name[dir]);
                act(AT_ACTION, buf, ch, NULL, victim, TO_ROOM);
                learn_from_failure(ch, gsn_throwsaber);
        }

        char_from_room(ch);
        char_to_room(ch, was_in_room);

        if (IS_NPC(ch))
                WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
        else
        {
                if (number_percent() < ch->pcdata->learned[gsn_third_attack])
                        WAIT_STATE(ch, 1 * PULSE_PER_SECOND);
                else if (number_percent() <
                         ch->pcdata->learned[gsn_second_attack])
                        WAIT_STATE(ch, 2 * PULSE_PER_SECOND);
                else
                        WAIT_STATE(ch, 3 * PULSE_PER_SECOND);
        }
        if (IS_NPC(victim) && !char_died(victim))
        {
                if (IS_SET(victim->act, ACT_SENTINEL))
                {
                        victim->was_sentinel = victim->in_room;
                        REMOVE_BIT(victim->act, ACT_SENTINEL);
                }

                start_hating(victim, ch);
                start_hunting(victim, ch);

        }

}

CMDF do_makebinding(CHAR_DATA * ch, char *argument)
{
        char      buf[MSL], arg[MSL];
        int       level, percentage;
        bool      checkcirc, checktool, checkdura;
        OBJ_DATA *obj;
        OBJ_INDEX_DATA *pObjIndex;
        int       vnum;

        mudstrlcpy(arg, argument, MSL);

        switch (ch->substate)
        {
        default:

                if (!argument || argument[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: Makebinding <Description>\n\r&w",
                                 ch);
                        return;
                }

                checkcirc = FALSE;
                checktool = FALSE;
                checkdura = FALSE;

                if (!xIS_SET(ch->in_room->room_flags, ROOM_FACTORY))
                {
                        send_to_char
                                ("&RYou need to be in a factory or workshop to do that.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURAPLAST
                            || obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;

                }

                if (!checkcirc)
                {
                        send_to_char("&RYou need a circuit board.\n\r", ch);
                        return;
                }

                if (!checktool)
                {
                        send_to_char("&RYou need a toolkit.\n\r", ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char("&RYou need something for support.\n\r",
                                     ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makebinding]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating a binding.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s construction material and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makebinding, 1);
                        ch->dest_buf = str_dup(argument);
                        return;
                }
                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makebinding);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makebinding]);
        vnum = 10440;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }



        checkcirc = FALSE;
        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
                {
                        checkcirc = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if ((obj->item_type == ITEM_DURAPLAST
                     || obj->item_type == ITEM_DURASTEEL)
                    && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makebinding]);

        if (number_percent() > percentage * 2 || (!checkcirc)
            || (!checkdura) || (!checktool))
        {
                send_to_char
                        ("&RYou look at your newly created binding.\n\r", ch);
                send_to_char
                        ("&RIt suddenly dawns upon you that you have created the most useless\n\r",
                         ch);
                send_to_char
                        ("&Rthing you've ever seen. You quickly hide your mistake...\n\r",
                         ch);
                learn_from_failure(ch, gsn_makebinding);
                return;
        }


        obj = create_object(pObjIndex, level);
        {
                char      str1[MAX_INPUT_LENGTH];

                strcat(str1, "binding binder ");
                strcat(str1, arg);
                obj->item_type = ITEM_BINDING;
                SET_BIT(obj->wear_flags, ITEM_TAKE);
                SET_BIT(obj->wear_flags, ITEM_HOLD);
                obj->level = 1;
                obj->weight = 5;
                STRFREE(obj->name);
                obj->name = STRALLOC(str1);
                mudstrlcpy(buf, arg, MIL);
                STRFREE(obj->short_descr);
                obj->short_descr = STRALLOC(buf);
                STRFREE(obj->description);
                mudstrlcat(buf, " lies here.", MIL);
                obj->description = STRALLOC(buf);
                obj->value[4] = percentage > 90 ? 1 : 0;
                obj->cost = level * 10;
        }
        obj = obj_to_char(obj, ch);


        send_to_char
                ("&GYou finish your work and look at your new binding.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes creating some equipment.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(obj->cost * 25,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
        learn_from_success(ch, gsn_makebinding);
}
