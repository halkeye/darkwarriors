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
 *                                 SWR HTML Header file                                  *
 ****************************************************************************************/
/*
 * New combatant stuff, or just general combat related stuff.
 */

#include <string.h>
#include "mud.h"

extern bool could_dual(CHAR_DATA * ch);
extern void separate_obj(OBJ_DATA * obj);

CMDF do_draw(CHAR_DATA * ch, char *argument)
{
        bool      silent = FALSE;
        OBJ_DATA *holster1 = NULL, *holster2 = NULL;
        OBJ_DATA *wield1, *wield2;
        OBJ_DATA *hold;

        if (!strcmp(argument, "-silence"))
                silent = TRUE;

        holster1 = get_eq_char(ch, WEAR_HOLSTER_L);
        if (!holster1)
        {
                holster1 = get_eq_char(ch, WEAR_HOLSTER_R);
                if (!holster1)
                {
                        if (!silent)
                                send_to_char
                                        ("You aren't wearing a holster.\n\r",
                                         ch);
                        return;
                }
        }
        else
                holster2 = get_eq_char(ch, WEAR_HOLSTER_R);

        if ((holster1->item_type != ITEM_HOLSTER &&
             (holster2 && holster2->item_type != ITEM_HOLSTER)) ||
            (holster1->item_type != ITEM_HOLSTER && !holster2))
        {
                if (!silent)
                        send_to_char
                                ("That's not a holster you're wearing!\n\r",
                                 ch);
                return;
        }

        if ((!holster1->first_content
             && (holster2 && !holster2->first_content))
            || (!holster1->first_content && !holster2))
        {
                if (!silent)
                        send_to_char("Your holster(s) are empty!\n\r", ch);
                return;
        }

        wield1 = get_eq_char(ch, WEAR_WIELD);
        wield2 = get_eq_char(ch, WEAR_DUAL_WIELD);

        if (wield1 != NULL)
                unequip_char(ch, wield1);
        if (wield2 != NULL)
                unequip_char(ch, wield2);

        wield1 = holster1->first_content;
        separate_obj(wield1);
        if (holster2)
        {
                wield2 = holster2->first_content;
                separate_obj(wield2);
        }

        if (wield1 != NULL)
        {
                obj_from_obj(wield1);
                wield1 = obj_to_char(wield1, ch);
                equip_char(ch, wield1, WEAR_WIELD);
                act(AT_ACTION, "$n draws $p.", ch, wield1, holster1, TO_ROOM);
                act(AT_ACTION, "You draw $p.", ch, wield1, holster1, TO_CHAR);
        }

        if (wield2 != NULL)
        {
                hold = get_eq_char(ch, WEAR_HOLD);
                if (hold != NULL)
                        unequip_char(ch, hold);

                obj_from_obj(wield2);
                wield2 = obj_to_char(wield2, ch);
                equip_char(ch, wield2, WEAR_DUAL_WIELD);
                act(AT_ACTION, "$n draws $p.", ch, wield2, holster2, TO_ROOM);
                act(AT_ACTION, "You draw $p.", ch, wield2, holster2, TO_CHAR);
        }

        return;
}

CMDF do_holster(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *wield1 = get_eq_char(ch, WEAR_WIELD),
                *wield2 = get_eq_char(ch, WEAR_DUAL_WIELD);
        OBJ_DATA *holster1 = get_eq_char(ch, WEAR_HOLSTER_L),
                *holster2 = get_eq_char(ch, WEAR_HOLSTER_R);
        argument = NULL;    /* Added by greven to silence compile warnings. */

        if (!(holster1 = get_eq_char(ch, WEAR_HOLSTER_L)))
        {
                if (!(holster1 = get_eq_char(ch, WEAR_HOLSTER_R)))
                {
                        send_to_char("You aren't wearing a holster!\n\r", ch);
                        return;
                }
        }
        else
                holster2 = get_eq_char(ch, WEAR_HOLSTER_R);

        if (holster1->item_type != ITEM_HOLSTER &&
            (holster2 && holster2->item_type != ITEM_HOLSTER))
        {
                send_to_char("That's not a holster you're wearing.\n\r", ch);
                return;
        }

        if (holster1->first_content && holster2 && holster2->first_content)
        {
                send_to_char("There's already something in the holster.\n\r",
                             ch);
                return;
        }

        if (!wield1 && !wield2)
        {
                send_to_char("You aren't wielding anything to holster.\n\r",
                             ch);
                return;
        }

        if (wield1 != NULL)
        {
                char      buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

                if (!holster1->first_content)
                {
                        unequip_char(ch, wield1);
                        obj_from_char(wield1);
                        wield1 = obj_to_obj(wield1, holster1);
                        separate_obj(wield1);
                        if (holster1->wear_loc == WEAR_HOLSTER_L)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s left holster.");
                                sprintf(buf2,
                                        "You holster $p in your left holster.");
                        }
                        else if (holster1->wear_loc == WEAR_HOLSTER_R)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s right holster.");
                                sprintf(buf2,
                                        "You holster $p in your right holster.");
                        }
                        else
                        {
                                sprintf(buf, "$n holsters $p in $s holster.");
                                sprintf(buf2, "You holster $p in a holster.");
                        }
                        act(AT_ACTION, buf, ch, wield1, holster1, TO_ROOM);
                        act(AT_ACTION, buf2, ch, wield1, holster1, TO_CHAR);
                }
                else if (holster2 && !holster2->first_content)
                {
                        unequip_char(ch, wield1);
                        obj_from_char(wield1);
                        wield1 = obj_to_obj(wield1, holster2);
                        separate_obj(wield1);
                        act(AT_ACTION, "$n holsters $p in $s right holster.",
                            ch, wield1, holster2, TO_ROOM);
                        act(AT_ACTION,
                            "You holster $p in your right holster.", ch,
                            wield1, holster2, TO_ROOM);
                }
                else
                {
                        send_to_char("Your holster(s) are full!\n\r", ch);
                        return;
                }
        }

        if (wield2 != NULL)
        {
                char      buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

                if (!holster1->first_content)
                {
                        unequip_char(ch, wield2);
                        obj_from_char(wield2);
                        wield2 = obj_to_obj(wield2, holster1);
                        separate_obj(wield2);
                        if (holster1->wear_loc == WEAR_HOLSTER_L)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s left holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in your left holster.\n\r");
                        }
                        else if (holster1->wear_loc == WEAR_HOLSTER_R)
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s right holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in your right holster.\n\r");
                        }
                        else
                        {
                                sprintf(buf,
                                        "$n holsters $p in $s holster.\n\r");
                                sprintf(buf2,
                                        "You holster $p in a holster.\n\r");
                        }
                        act(AT_ACTION, buf, ch, wield2, holster1, TO_ROOM);
                        act(AT_ACTION, buf2, ch, wield2, holster1, TO_CHAR);
                }
                else if (holster2 && !holster2->first_content)
                {
                        unequip_char(ch, wield2);
                        obj_from_char(wield2);
                        wield2 = obj_to_obj(wield2, holster2);
                        separate_obj(wield2);
                        act(AT_ACTION, "$n holsters $p in $s right holster.",
                            ch, wield2, holster2, TO_ROOM);
                        act(AT_ACTION,
                            "You holster $p in your right holster.", ch,
                            wield2, holster2, TO_CHAR);
                }
                else
                {
                        unequip_char(ch, wield2);
                        equip_char(ch, wield2, WEAR_WIELD);
                        send_to_char("Your holster(s) are full!\n\r", ch);
                        return;
                }
        }

        return;
}
