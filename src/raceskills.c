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
 *                $Id: raceskills.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int top_affect;

CMDF do_craftpike(CHAR_DATA * ch, char *argument)
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
                        send_to_char("&RUsage: craftpike <name>\n\r&w", ch);
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
                        : (int) (ch->pcdata->learned[gsn_craftpike]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a pike.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_craftpike, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_craftpike);
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
                                                    learned[gsn_craftpike]);
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
                : (int) (ch->pcdata->learned[gsn_craftpike]);

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
                learn_from_failure(ch, gsn_craftpike);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 15;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " force pike", MSL);
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

        learn_from_success(ch, gsn_craftpike);
}
CMDF do_craftknife(CHAR_DATA * ch, char *argument)
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
                        send_to_char("&RUsage: craftknife <name>\n\r&w", ch);
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
                        : (int) (ch->pcdata->learned[gsn_craftknife]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of crafting a knife.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and a small oven and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_craftknife, 1);
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
                                                    learned[gsn_craftknife]);
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
                : (int) (ch->pcdata->learned[gsn_craftknife]);

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
                learn_from_failure(ch, gsn_craftknife);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_WEAPON;
        SET_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 3;
        STRFREE(obj->name);
        mudstrlcpy(buf, arg, MSL);
        mudstrlcat(buf, " knife", MSL);
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

        learn_from_success(ch, gsn_craftknife);
}


CMDF do_fixship(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, change;
        SHIP_DATA *ship;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:
                if ((ship = ship_from_engine(ch->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("&RYou must be in the engine room of a ship to do that!\n\r",
                                 ch);
                        return;
                }

                if (str_cmp(argument, "hull") && str_cmp(argument, "drive") &&
                    str_cmp(argument, "launcher")
                    && str_cmp(argument, "laser")
                    && str_cmp(argument, "turret 1")
                    && str_cmp(argument, "turret 2"))
                {
                        send_to_char
                                ("&RYou need to spceify something to fix:\n\r",
                                 ch);
                        send_to_char
                                ("&rTry: hull, drive, launcher, laser, turret 1, or turret 2\n\r",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_systemmaintenance]);
                if (number_percent() < percentage)
                {
                        send_to_char("&GYou begin your fix\n\r", ch);
                        act(AT_PLAIN, "$n begins fixing the ships $T.", ch,
                            NULL, argument, TO_ROOM);
                        if (!str_cmp(arg, "hull"))
                                add_timer(ch, TIMER_DO_FUN, 15, do_fixship,
                                          1);
                        else
                                add_timer(ch, TIMER_DO_FUN, 5, do_fixship, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou fail to locate the source of the problem.\n\r",
                         ch);
                learn_from_failure(ch, gsn_systemmaintenance);
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
                if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
                        return;
                send_to_char
                        ("&RYou are distracted and fail to finish your fix.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        if ((ship = ship_from_engine(ch->in_room->vnum)) == NULL)
        {
                return;
        }

        if (!str_cmp(arg, "hull"))
        {
                change = URANGE(0,
                                number_range((int)
                                             (ch->pcdata->
                                              learned[gsn_systemmaintenance] /
                                              2),
                                             (int) (ch->pcdata->
                                                    learned
                                                    [gsn_systemmaintenance])),
                                (ship->maxhull - ship->hull));
                ship->hull += change;
                ch_printf(ch,
                          "&GFix complete.. Hull strength inreased by %d points.\n\r",
                          change);
        }

        if (!str_cmp(arg, "drive"))
        {
                if (ship->location == ship->lastdoc)
                        ship->shipstate = SHIP_DOCKED;
                else
                        ship->shipstate = SHIP_READY;
                send_to_char("&GShips drive fixed.\n\r", ch);
        }

        if (!str_cmp(arg, "launcher"))
        {
                ship->missilestate = MISSILE_READY;
                send_to_char("&GMissile launcher fixed.\n\r", ch);
        }

        if (!str_cmp(arg, "laser"))
        {
                ship->statet0 = LASER_READY;
                send_to_char("&GMain laser fixed.\n\r", ch);
        }

        if (!str_cmp(arg, "turret 1"))
        {
                ship->statet1 = LASER_READY;
                send_to_char("&GLaser Turret 1 fixed.\n\r", ch);
        }

        if (!str_cmp(arg, "turret 2"))
        {
                ship->statet2 = LASER_READY;
                send_to_char("&Laser Turret 2 fixed.\n\r", ch);
        }

        act(AT_PLAIN, "$n finishes the fix.", ch, NULL, argument, TO_ROOM);

        learn_from_success(ch, gsn_systemmaintenance);

}


CMDF do_jab(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        OBJ_DATA *obj;
        int       percent;

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
                send_to_char("Jab whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("How can you sneak up on yourself?\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        /*
         * Added stabbing weapon. -Narn 
         */
        if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
            || (obj->value[3] != WEAPON_KNIFE))
        {
                send_to_char("You need to wield a stabbing weapon.\n\r", ch);
                return;
        }

        if (victim->fighting)
        {
                send_to_char("You can't jab someone who is in combat.\n\r",
                             ch);
                return;
        }

        /*
         * Can backstab a char even if it's hurt as long as it's sleeping. -Narn 
         */
        if (victim->hit < victim->max_hit && IS_AWAKE(victim))
        {
                act(AT_PLAIN,
                    "$N is hurt and suspicious ... you can't sneak up.", ch,
                    NULL, victim, TO_CHAR);
                return;
        }

        percent = number_percent() - (get_curr_lck(ch) - 14)
                + (get_curr_lck(victim) - 13);

        WAIT_STATE(ch, skill_table[gsn_jab]->beats);
        if (!IS_AWAKE(victim)
            || IS_NPC(ch) || percent < ch->pcdata->learned[gsn_jab])
        {
                learn_from_success(ch, gsn_jab);
                global_retcode = multi_hit(ch, victim, gsn_jab);

        }
        else
        {
                learn_from_failure(ch, gsn_jab);
                global_retcode = damage(ch, victim, 0, gsn_jab);
        }
        return;
}

CMDF do_gowithout(CHAR_DATA * ch, char *argument)
{
        int       amount, condition, percentage;

        argument = NULL;

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (ch->endurance < 400)
        {
                send_to_char("You do not have the endurance to do that", ch);
                return;
        }

        percentage = number_percent() - get_curr_con(ch);
        WAIT_STATE(ch, skill_table[gsn_gowithout]->beats);
        if (IS_NPC(ch) || percentage < ch->pcdata->learned[gsn_gowithout])
        {
                learn_from_success(ch, gsn_gowithout);

                condition = ch->pcdata->condition[COND_FULL];
                amount = 40 - condition;

                gain_condition(ch, COND_FULL, amount);

                condition = ch->pcdata->condition[COND_THIRST];
                amount = 40 - condition;

                gain_condition(ch, COND_THIRST, amount);
                send_to_char
                        ("You steel your body against the need for food or drink!\n\r",
                         ch);
        }
        else
        {
                send_to_char
                        ("You try to steel your body against its needs but fail!\n\r",
                         ch);
                learn_from_failure(ch, gsn_gowithout);
        }
        ch->endurance -= 200 - (get_curr_con(ch) * 3);
        return;
}


CMDF do_cajole(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        PLANET_DATA *planet;
        CLAN_DATA *clan;
        int       percent = 0;

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


        if (IS_SET(victim->act, ACT_CITIZEN))
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
        ch_printf(ch, "You speak to them about the benifits of the %s%s.\n\r",
                  ch->pcdata->clan->name,
                  planet->governed_by == clan ? "" : buf);
        act(AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL,
            victim, TO_VICT);
        act(AT_ACTION, "$n tells $N about their organization.\n\r", ch, NULL,
            victim, TO_NOTVICT);

        WAIT_STATE(ch, skill_table[gsn_cajole]->beats);

        if (percent - get_curr_cha(ch) + victim->top_level >
            ch->pcdata->learned[gsn_cajole])
        {

                if (planet->governed_by != clan)
                {
                        command_printf(victim, "yell %s is a traitor!",
                                       ch->name);
                        global_retcode =
                                multi_hit(victim, ch, TYPE_UNDEFINED);
                }

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
                planet->pop_support += ch->top_level / 50;
                send_to_char
                        ("Popular support for the current government decreases.\n\r",
                         ch);
        }

        gain_exp(ch, victim->top_level * 100, DIPLOMACY_ABILITY);
        ch_printf(ch, "You gain %d diplomacy experience.\n\r",
                  victim->top_level * 100);

        learn_from_success(ch, gsn_cajole);

        if (planet->pop_support > 100)
                planet->pop_support = 100;
        if (planet->pop_support < -100)
                planet->pop_support = -100;

}

CMDF do_pretend(CHAR_DATA * ch, char *argument)
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
                send_to_char("Change your title to what?\n\r", ch);
                return;
        }

        percentage = (int) (ch->pcdata->learned[gsn_pretend]);

        if (number_percent() > percentage)
        {
                send_to_char("You try to disguise yourself but fail.\n\r",
                             ch);
                return;
        }

        if (strlen(argument) > 50)
                argument[50] = '\0';

        smash_tilde(argument);
        set_title(ch, argument);
        send_to_char("Ok.\n\r", ch);
}

CMDF do_jimmyshiplock(CHAR_DATA * ch, char *argument)
{
        do_pick(ch, argument);
}
