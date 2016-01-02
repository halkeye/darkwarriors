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
 *                $Id: smuggling.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <string.h>
#include "mud.h"


CMDF do_placebeacon(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        int       percentage;

        if (argument[0] == '\0' || !is_number(argument))
        {
                send_to_char
                        ("&RSyntax: &Gplacebeacon &C<&csecret code&C>&w\n\r",
                         ch);
                return;
        }

        if (!ch->in_room->area->body)
        {
                send_to_char
                        ("You are not on a spacial body with which to place a beacon.",
                         ch);
                return;
        }

        if (!IS_OUTSIDE(ch))
        {
                send_to_char
                        ("Placing a beacon inside would not allow it to function properly.",
                         ch);
                return;
        }

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_BEACON)
                        break;
        }

        if (!obj)
        {
                send_to_char
                        ("What is it you want to place here, nothing? You have no beacon.",
                         ch);
                return;
        }
        separate_obj(obj);
        obj_from_char(obj);
        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_placebeacon]);
        if (number_percent() > percentage)
        {
                make_scraps(obj);
                send_to_char
                        ("You place your beacon into the ground, and try to set it.\n\r",
                         ch);
                send_to_char
                        ("The beacon emits a horrible screech of static, and you quickly remove it from its perch.\n\r",
                         ch);
                act(AT_PLAIN,
                    "The beacon emits a horrible screech of static, and $n quickly removes it from its perch.",
                    ch, NULL, NULL, TO_ROOM);
                /*
                 * act message here to ROOM 
                 */
                learn_from_failure(ch, gsn_placebeacon);
                return;

        }
        obj->value[0] = 1;  /* Activate beacon */
        obj->value[1] = atoi(argument); /* Secret Code */
        STRFREE(obj->armed_by);
        obj->armed_by = STRALLOC(ch->name);
        obj->timer =
                (percentage +
                 ch->skill_level[skill_table[gsn_placebeacon]->guild]) * 2;
        obj_to_room(obj, ch->in_room);  /* Should actually check to see if the code has been done for that body already? */
        send_to_char("You place the beacon on the ground.\n\r", ch);
        send_to_char
                ("You test it, and recognize that the signal bein emitted is the exact frequency you wanted.\n\r",
                 ch);
        act(AT_PLAIN, "$n places $s beacon carefully on the ground.", ch,
            NULL, NULL, TO_ROOM);
        /*
         * act message here to ROOM 
         */
        learn_from_success(ch, gsn_placebeacon);
        return;
}

CMDF do_makebeacon(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       level, percentage, cost;
        bool      checktool, checkoven, checkmetal, checkcom, checkbattery,
                checkcirc;
        OBJ_DATA *obj;
        OBJ_DATA *metal = NULL;


        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: Makebeacon <name>\n\r&w", ch);
                        return;
                }

                checktool = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;
                checkcom = FALSE;
                checkbattery = FALSE;
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
                        if (obj->item_type == ITEM_OVEN)
                                checkoven = TRUE;
                        if (obj->item_type == ITEM_RARE_METAL
                            || obj->item_type == ITEM_DURASTEEL)
                                checkmetal = TRUE;
                        if (obj->item_type == ITEM_COMLINK)
                                checkcom = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbattery = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcirc = TRUE;
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
                        send_to_char("&RYou need some metal.\n\r", ch);
                        return;
                }

                if (!checkcom)
                {
                        send_to_char("&RYou need a comlink!\n\r", ch);
                        return;
                }

                if (!checkbattery)
                {
                        send_to_char
                                ("&RYou need a battery to power your beacon.\n\r",
                                 ch);
                        return;
                }

                if (!checkcirc)
                {
                        send_to_char
                                ("&RYou need a small circuit to make this beacon.\n\r",
                                 ch);
                        return;
                }

                percentage =
                        IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_makebeacon]);

                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou grab your tools and equipment and start to craft a beacon.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s toolkit and some metal and begins to work.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 15, do_makebeacon, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }

                send_to_char("&RYou can't figure out what to do.\n\r", ch);
                learn_from_failure(ch, gsn_makebeacon);
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
                                                    learned[gsn_makebeacon]);

        checkmetal = FALSE;
        checkoven = FALSE;
        checktool = FALSE;
        checkcom = FALSE;
        checkbattery = FALSE;
        checkcirc = FALSE;
        cost = 0;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_OVEN)
                        checkoven = TRUE;
                if ((obj->item_type == ITEM_RARE_METAL
                     || obj->item_type == ITEM_DURASTEEL)
                    && checkmetal == FALSE)
                {
                        checkmetal = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        metal = obj;
                }
                if (obj->item_type == ITEM_COMLINK && checkcom == FALSE)
                {
                        checkcom = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbattery == FALSE)
                {
                        checkbattery = TRUE;
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


        }

        percentage =
                IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_makebeacon]);

        if (number_percent() > percentage * 2 || (!checkoven) || (!checktool)
            || (!checkmetal) || (!checkbattery) || (!checkcirc)
            || (!checkcom))
        {
                send_to_char("&RYou hold up your newly created beacon.\n\r",
                             ch);
                send_to_char
                        ("&RYou run a routine analysis on its broadcast system.\n\r",
                         ch);
                send_to_char
                        ("&RYou hear a comlink near by explode with static, so you quickly dispose of the unit.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makebeacon);
                return;
        }

        obj = metal;

        obj->item_type = ITEM_BEACON;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
                REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);

        stralloc_printf(&obj->name, "%s beacon", smash_color(arg));
        stralloc_printf(&obj->short_descr, "%s", smash_color(arg));
        stralloc_printf(&obj->description, "%s was carelessly dropped here.",
                        smash_color(arg));
        obj->value[0] = 0;
        obj->value[1] = 0;
        obj->cost *= 20;

        obj = obj_to_char(obj, ch);

        send_to_char
                ("&GYou finish your work and hold up your newly created beacon.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes working on a piece of metal.", ch, NULL,
            argument, TO_ROOM);
        {
                SKILLTYPE *skill = skill_table[gsn_makebeacon];
                long      xpgain;

                xpgain = UMIN((level * level * 2),  //This should equal anywhere from 20-40k exp,
                              // which should be about 1/3 of what they need for the next level
                              (exp_level
                               (ch->skill_level[skill->guild] + 1) -
                               exp_level(ch->skill_level[skill->guild])));
                gain_exp(ch, xpgain, skill->guild);
                ch_printf(ch, "You gain %d %s experience.\n", xpgain,
                          ability_name[skill->guild]);
        }
        learn_from_success(ch, gsn_makebeacon);
}
