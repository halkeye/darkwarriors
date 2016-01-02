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
 *                $Id: misc.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#include "mud.h"
#include "editor.h"
#include "account.h"
#include "races.h"


extern int top_exit;

CMDF do_buyhome(CHAR_DATA * ch, char *argument)
{
        ROOM_INDEX_DATA *room;
        AREA_DATA *pArea;
        CHAR_DATA *vch;
        int       ppl = 0;


        if (!ch->in_room)
                return;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (ch->plr_home != NULL)
        {
                send_to_char("&RYou already have a home!\n\r&w", ch);
                return;
        }

        room = ch->in_room;

        for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
        {
                if (room->area == pArea)
                {
                        send_to_char("&RThis area isn't installed yet!\n\r&w",
                                     ch);
                        return;
                }
        }

        if (!xIS_SET(room->room_flags, ROOM_EMPTY_HOME))
        {
                send_to_char("&RThis room isn't for sale!\n\r&w", ch);
                return;
        }

        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
                if (vch != ch && !IS_IMMORTAL(vch))
                        ppl++;
        if (ppl > 0)
        {
                send_to_char("The room has to be empty first.\n\r", ch);
                return;
        }



        if (ch->gold < 100000)
        {
                send_to_char
                        ("&RThis room costs 100000 credits you don't have enough!\n\r&w",
                         ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char
                        ("Set the room name.  A very brief single line room description.\n\r",
                         ch);
                send_to_char("Usage: Buyhome <Room Name>\n\r", ch);
                return;
        }

        STRFREE(room->name);
        smash_tilde(argument);
        room->name = STRALLOC(argument);

        ch->gold -= 100000;

        send_to_char("You have just bought your home.\n\r", ch);

        xREMOVE_BIT(room->room_flags, ROOM_EMPTY_HOME);
        xSET_BIT(room->room_flags, ROOM_PLR_HOME);

        fold_area(room->area, room->area->filename, FALSE, TRUE);

        ch->plr_home = room;
        do_save(ch, "");

}


CMDF do_sellhome(CHAR_DATA * ch, char *argument)
{
        ROOM_INDEX_DATA *room;
        AREA_DATA *pArea;

        argument = NULL;

        if (!ch->in_room)
                return;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (ch->plr_home == NULL)
        {
                send_to_char("&RThis is not your apartment!\n\r&w", ch);
                return;
        }

        room = ch->in_room;

        for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
        {
                if (room->area == pArea)
                {
                        send_to_char("&RThis area isn't installed yet!\n\r&w",
                                     ch);
                        return;
                }
        }
        if (xIS_SET(room->room_flags, ROOM_PLR_HOME))
        {
                send_to_char("&RYou sell your apartment!\n\r&w", ch);
                STRFREE(room->name);
                room->name = STRALLOC("An Empty Home");
                ch->gold += 50000;
                send_to_char
                        ("&RYou receive 50000 credits for your apartment.\n\r&w",
                         ch);
                xREMOVE_BIT(room->room_flags, ROOM_PLR_HOME);
                xSET_BIT(room->room_flags, ROOM_EMPTY_HOME);
                fold_area(room->area, room->area->filename, FALSE, TRUE);
                ch->plr_home = NULL;
                do_save(ch, "");
                return;
        }
        else
        {
                send_to_char("&RThis isn't a home!\n\r&w", ch);
                return;
        }
        return;
}



CMDF do_clone(CHAR_DATA * ch, char *argument)
{
        long      credits, bank;
        long      played;
        CLAN_DATA *clan = NULL;
        char      bestowments[MAX_STRING_LENGTH];
        int       flags, i;
        int       bodyparts;
        ROOM_INDEX_DATA *home;
        int       implants[MAX_IMPLANT_TYPES];

        argument = NULL;

        if (IS_NPC(ch))
        {
                ch_printf(ch, "Yeah right!\n\r");
                return;
        }

        if (ch->in_room->vnum != ROOM_CLONE_BEGIN)
        {
                ch_printf(ch, "You can't do that here!\n\r");
                return;
        }

        if (ch->gold < ch->top_level * 200)
        {
                ch_printf(ch,
                          "You don't have enough credits... You need %d.\n\r",
                          ch->top_level * 200);
                return;
        }
        else
        {
                ch->gold -= ch->top_level * 200;

                ch_printf(ch, "You pay %d credits for cloning.\n\r",
                          ch->top_level * 200);
                ch_printf(ch, "You are escorted into a small room.\n\r\n\r");
        }

        char_from_room(ch);
        char_to_room(ch, get_room_index(ROOM_CLONE_END));

        flags = ch->act;
        REMOVE_BIT(ch->act, PLR_KILLER);
        credits = ch->gold;
        ch->gold = 5000;
        played = ch->played;
        ch->played = ch->played / 2;
        bank = ch->pcdata->bank;
        ch->pcdata->bank = 0;
        home = ch->plr_home;
        ch->plr_home = NULL;
        bodyparts = ch->bodyparts;
        ch->bodyparts = 0;
        for (i = 0; i < MAX_IMPLANT_TYPES; i++)
        {
                implants[i] = ch->pcdata->implants[i];
                ch->pcdata->implants[i] = -1;
        }
        if (ch->pcdata->clan)
        {
                clan = ch->pcdata->clan;
                ch->pcdata->clan = NULL;
                mudstrlcpy(bestowments, ch->pcdata->bestowments, MSL);
                STRFREE(ch->pcdata->bestowments);
                ch->pcdata->bestowments = STRALLOC("");
        }

        save_clone(ch);

        if (clan)
        {
                ch->pcdata->clan = clan;
                STRFREE(ch->pcdata->bestowments);
                ch->pcdata->bestowments = STRALLOC(bestowments);
        }
        for (i = 0; i < MAX_IMPLANT_TYPES; i++)
        {
                ch->pcdata->implants[i] = implants[i];
        }
        ch->bodyparts = bodyparts;
        ch->plr_home = home;
        ch->played = played;
        ch->gold = credits;
        ch->pcdata->bank = bank;
        ch->act = flags;
        char_from_room(ch);
        char_to_room(ch, get_room_index(10002));
        do_look(ch, "");


        if (!str_cmp(ch->race->name(), "droid"))
        {
                ch_printf(ch,
                          "\n\r&WThe facility droids make a careful copy of your physical structure.\n\r");
                ch_printf(ch, "&RYou are shut off!\n\r\n\r");
                ch_printf(ch,
                          "&WYou are re-activated and informed that you have been succesfully cloned.\n\r");
        }
        else
        {
                ch_printf(ch,
                          "\n\r&WA small tissue sample is taken from your arm.\n\r");
                ch_printf(ch, "&ROuch!\n\r\n\r");
                ch_printf(ch, "&WYou have been succesfully cloned.\n\r");

                ch->hit--;
        }
}

CMDF do_arm(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        char      arg1[MAX_INPUT_LENGTH];
        int       timer;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        one_argument(argument, arg1);
        if ((timer = atoi(arg1)) == -1 || timer < 1 || timer > 10)
        {
                timer = 1;
        }
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        if (IS_NPC(ch) || !ch->pcdata)
        {
                ch_printf(ch, "You have no idea how to do that.\n\r");
                return;
        }

        if (ch->pcdata->learned[gsn_grenades] <= 0)
        {
                ch_printf(ch, "You have no idea how to do that.\n\r");
                return;
        }

        obj = get_eq_char(ch, WEAR_HOLD);

        if (!obj || obj->item_type != ITEM_GRENADE)
        {
                ch_printf(ch, "You don't seem to be holding a grenade!\n\r");
                return;
        }

        obj->timer = timer;
        STRFREE(obj->armed_by);
        obj->armed_by = STRALLOC(ch->name);

        ch_printf(ch, "You arm %s.\n\r", obj->short_descr);
        act(AT_PLAIN, "$n arms $p.", ch, obj, NULL, TO_ROOM);

        learn_from_success(ch, gsn_grenades);
}

CMDF do_ammo(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *wield;
        OBJ_DATA *obj;
        bool      checkammo = FALSE;
        int       charge = 0;

        argument = NULL;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        obj = NULL;
        wield = get_eq_char(ch, WEAR_WIELD);
        if (wield)
        {
                obj = get_eq_char(ch, WEAR_DUAL_WIELD);
                if (!obj)
                        obj = get_eq_char(ch, WEAR_HOLD);
        }
        else
        {
                wield = get_eq_char(ch, WEAR_HOLD);
                obj = NULL;
        }

        if (!wield || wield->item_type != ITEM_WEAPON)
        {
                send_to_char("&RYou don't seem to be holding a weapon.\n\r&w",
                             ch);
                return;
        }

        if (wield->value[3] == WEAPON_BLASTER)
        {

                if (obj && obj->item_type != ITEM_AMMO)
                {
                        send_to_char
                                ("&RYour hands are too full to reload your blaster.\n\r&w",
                                 ch);
                        return;
                }

                if (obj)
                {
                        if (obj->value[0] > wield->value[5])
                        {
                                send_to_char
                                        ("That cartridge is too big for your blaster.\n\r",
                                         ch);
                                return;
                        }
                        unequip_char(ch, obj);
                        checkammo = TRUE;
                        charge = obj->value[0];
                        separate_obj(obj);
                        extract_obj(obj);
                }
                else
                {
                        for (obj = ch->last_carrying; obj;
                             obj = obj->prev_content)
                        {
                                if (obj->item_type == ITEM_AMMO)
                                {
                                        if (obj->value[0] > wield->value[5])
                                        {
                                                send_to_char
                                                        ("That cartridge is too big for your blaster.\n\r",
                                                         ch);
                                                continue;
                                        }
                                        checkammo = TRUE;
                                        charge = obj->value[0];
                                        separate_obj(obj);
                                        extract_obj(obj);
                                        break;
                                }
                        }
                }

                if (!checkammo)
                {
                        send_to_char
                                ("&RYou don't seem to have any ammo to reload your blaster with.\n\r&w",
                                 ch);
                        return;
                }

                ch_printf(ch,
                          "You replace your ammunition cartridge.\n\rYour blaster is charged with %d shots at high power to %d shots on low.\n\r",
                          charge / 5, charge);
                act(AT_PLAIN, "$n replaces the ammunition cell in $p.", ch,
                    wield, NULL, TO_ROOM);

        }
        else if (wield->value[3] == WEAPON_BOWCASTER)
        {

                if (obj && obj->item_type != ITEM_BOLT)
                {
                        send_to_char
                                ("&RYour hands are too full to reload your bowcaster.\n\r&w",
                                 ch);
                        return;
                }

                if (obj)
                {
                        if (obj->value[0] > wield->value[5])
                        {
                                send_to_char
                                        ("That cartridge is too big for your bowcaster.",
                                         ch);
                                return;
                        }
                        unequip_char(ch, obj);
                        checkammo = TRUE;
                        charge = obj->value[0];
                        separate_obj(obj);
                        extract_obj(obj);
                }
                else
                {
                        for (obj = ch->last_carrying; obj;
                             obj = obj->prev_content)
                        {
                                if (obj->item_type == ITEM_BOLT)
                                {
                                        if (obj->value[0] > wield->value[5])
                                        {
                                                send_to_char
                                                        ("That cartridge is too big for your bowcaster.",
                                                         ch);
                                                continue;
                                        }
                                        checkammo = TRUE;
                                        charge = obj->value[0];
                                        separate_obj(obj);
                                        extract_obj(obj);
                                        break;
                                }
                        }
                }

                if (!checkammo)
                {
                        send_to_char
                                ("&RYou don't seem to have any quarrels to reload your bowcaster with.\n\r&w",
                                 ch);
                        return;
                }

                ch_printf(ch,
                          "You replace your quarrel pack.\n\rYour bowcaster is charged with %d energy bolts.\n\r",
                          charge);
                act(AT_PLAIN, "$n replaces the quarrels in $p.", ch, wield,
                    NULL, TO_ROOM);

        }
        else
        {

                if (obj && obj->item_type != ITEM_BATTERY)
                {
                        send_to_char
                                ("&RYour hands are too full to replace the power cell.\n\r&w",
                                 ch);
                        return;
                }

                if (obj)
                {
                        unequip_char(ch, obj);
                        checkammo = TRUE;
                        charge = obj->value[0];
                        separate_obj(obj);
                        extract_obj(obj);
                }
                else
                {
                        for (obj = ch->last_carrying; obj;
                             obj = obj->prev_content)
                        {
                                if (obj->item_type == ITEM_BATTERY)
                                {
                                        checkammo = TRUE;
                                        charge = obj->value[0];
                                        separate_obj(obj);
                                        extract_obj(obj);
                                        break;
                                }
                        }
                }

                if (!checkammo)
                {
                        send_to_char
                                ("&RYou don't seem to have a power cell.\n\r&w",
                                 ch);
                        return;
                }

                if (wield->value[3] == WEAPON_LIGHTSABER)
                {
                        ch_printf(ch,
                                  "You replace your power cell.\n\rYour lightsaber is charged to %d/%d units.\n\r",
                                  charge, charge);
                        act(AT_PLAIN, "$n replaces the power cell in $p.", ch,
                            wield, NULL, TO_ROOM);
                        act(AT_PLAIN, "$p ignites with a bright glow.", ch,
                            wield, NULL, TO_ROOM);
                }
                else if (wield->value[3] == WEAPON_VIBRO_BLADE)
                {
                        ch_printf(ch,
                                  "You replace your power cell.\n\rYour vibro-blade is charged to %d/%d units.\n\r",
                                  charge, charge);
                        act(AT_PLAIN, "$n replaces the power cell in $p.", ch,
                            wield, NULL, TO_ROOM);
                }
                else if (wield->value[3] == WEAPON_FORCE_PIKE)
                {
                        ch_printf(ch,
                                  "You replace your power cell.\n\rYour force-pike is charged to %d/%d units.\n\r",
                                  charge, charge);
                        act(AT_PLAIN, "$n replaces the power cell in $p.", ch,
                            wield, NULL, TO_ROOM);
                }
                else
                {
                        ch_printf(ch, "You feel very foolish.\n\r");
                        act(AT_PLAIN, "$n tries to jam a power cell into $p.",
                            ch, wield, NULL, TO_ROOM);
                }
        }

        wield->value[4] = charge;

}

CMDF do_setblaster(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *wield;
        OBJ_DATA *wield2;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        wield = get_eq_char(ch, WEAR_WIELD);
        if (wield
            && !(wield->item_type == ITEM_WEAPON
                 && wield->value[3] == WEAPON_BLASTER))
                wield = NULL;
        wield2 = get_eq_char(ch, WEAR_DUAL_WIELD);
        if (wield2
            && !(wield2->item_type == ITEM_WEAPON
                 && wield2->value[3] == WEAPON_BLASTER))
                wield2 = NULL;

        if (!wield && !wield2)
        {
                send_to_char
                        ("&RYou don't seem to be wielding a blaster.\n\r&w",
                         ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char
                        ("&RUsage: setblaster <full|high|normal|half|low|stun>\n\r&w",
                         ch);
                return;
        }

        if (wield)
                act(AT_PLAIN, "$n adjusts the settings on $p.", ch, wield,
                    NULL, TO_ROOM);

        if (wield2)
                act(AT_PLAIN, "$n adjusts the settings on $p.", ch, wield2,
                    NULL, TO_ROOM);

        if (!str_cmp(argument, "full"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_FULL;
                        send_to_char
                                ("&YWielded blaster set to FULL Power\n\r&w",
                                 ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_FULL;
                        send_to_char
                                ("&YDual wielded blaster set to FULL Power\n\r&w",
                                 ch);
                }
                return;
        }
        if (!str_cmp(argument, "high"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_HIGH;
                        send_to_char
                                ("&YWielded blaster set to HIGH Power\n\r&w",
                                 ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_HIGH;
                        send_to_char
                                ("&YDual wielded blaster set to HIGH Power\n\r&w",
                                 ch);
                }
                return;
        }
        if (!str_cmp(argument, "normal"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_NORMAL;
                        send_to_char
                                ("&YWielded blaster set to NORMAL Power\n\r&w",
                                 ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_NORMAL;
                        send_to_char
                                ("&YDual wielded blaster set to NORMAL Power\n\r&w",
                                 ch);
                }
                return;
        }
        if (!str_cmp(argument, "half"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_HALF;
                        send_to_char
                                ("&YWielded blaster set to HALF Power\n\r&w",
                                 ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_HALF;
                        send_to_char
                                ("&YDual wielded blaster set to HALF Power\n\r&w",
                                 ch);
                }
                return;
        }
        if (!str_cmp(argument, "low"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_LOW;
                        send_to_char
                                ("&YWielded blaster set to LOW Power\n\r&w",
                                 ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_LOW;
                        send_to_char
                                ("&YDual wielded blaster set to LOW Power\n\r&w",
                                 ch);
                }
                return;
        }
        if (!str_cmp(argument, "stun"))
        {
                if (wield)
                {
                        wield->blaster_setting = BLASTER_STUN;
                        send_to_char("&YWielded blaster set to STUN\n\r&w",
                                     ch);
                }
                if (wield2)
                {
                        wield2->blaster_setting = BLASTER_STUN;
                        send_to_char
                                ("&YDual wielded blaster set to STUN\n\r&w",
                                 ch);
                }
                return;
        }
        else
                do_setblaster(ch, "");

}

CMDF do_use(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      argd[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        OBJ_DATA *device;
        OBJ_DATA *obj;
        ch_ret    retcode;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        argument = one_argument(argument, argd);
        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "on"))
                argument = one_argument(argument, arg);

        if (argd[0] == '\0')
        {
                send_to_char("Use what?\n\r", ch);
                return;
        }

        if ((device = get_eq_char(ch, WEAR_HOLD)) == NULL ||
            !nifty_is_name(argd, device->name))
        {
                do_takedrug(ch, argd);
                return;
        }

        if (device->item_type == ITEM_SPICE)
        {
                do_takedrug(ch, argd);
                return;
        }

        if (device->item_type != ITEM_DEVICE)
        {
                send_to_char
                        ("You can't figure out what it is your supposed to do with it.\n\r",
                         ch);
                return;
        }

        if (device->value[2] <= 0)
        {
                send_to_char("It has no more charge left.", ch);
                return;
        }

        obj = NULL;
        if (arg[0] == '\0')
        {
                if (ch->fighting)
                {
                        victim = who_fighting(ch);
                }
                else
                {
                        send_to_char("Use on whom or what?\n\r", ch);
                        return;
                }
        }
        else
        {
                if ((victim = get_char_room(ch, arg)) == NULL
                    && (obj = get_obj_here(ch, arg)) == NULL)
                {
                        send_to_char("You can't find your target.\n\r", ch);
                        return;
                }
        }

        WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

        if (device->value[2] > 0)
        {
                device->value[2]--;
                if (victim)
                {
                        if (!oprog_use_trigger
                            (ch, device, victim, NULL, NULL))
                        {
                                act(AT_MAGIC, "$n uses $p on $N.", ch, device,
                                    victim, TO_ROOM);
                                act(AT_MAGIC, "You use $p on $N.", ch, device,
                                    victim, TO_CHAR);
                        }
                }
                else
                {
                        if (!oprog_use_trigger(ch, device, NULL, obj, NULL))
                        {
                                act(AT_MAGIC, "$n uses $p on $P.", ch, device,
                                    obj, TO_ROOM);
                                act(AT_MAGIC, "You use $p on $P.", ch, device,
                                    obj, TO_CHAR);
                        }
                }

                retcode =
                        obj_cast_spell(device->value[3], device->value[0], ch,
                                       victim, obj);
                if (retcode == rCHAR_DIED || retcode == rBOTH_DIED)
                {
                        bug("do_use: char died", 0);
                        return;
                }
        }


        return;
}

CMDF do_takedrug(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        AFFECT_DATA af;
        int       drug;
        int       sn = 0;


        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        if (argument[0] == '\0' || !str_cmp(argument, ""))
        {
                send_to_char("Use what?\n\r", ch);
                return;
        }

        if ((obj = find_obj(ch, argument, TRUE)) == NULL)
                return;

        if (obj->item_type == ITEM_DEVICE)
        {
                send_to_char("Try holding it first.\n\r", ch);
                return;
        }


        if (!str_cmp(ch->race->name(), "droid"))
        {
                send_to_char
                        ("You are a droid. You don't need to eat. Stop it.\n\r",
                         ch);
                return;
        }

        if (obj->item_type != ITEM_SPICE)
        {
                act(AT_ACTION, "$n looks at $p and scratches $s head.", ch,
                    obj, NULL, TO_ROOM);
                act(AT_ACTION,
                    "You can't quite figure out what to do with $p.", ch, obj,
                    NULL, TO_CHAR);
                return;
        }

        separate_obj(obj);
        if (obj->in_obj)
        {
                act(AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj,
                    TO_CHAR);
                act(AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj,
                    TO_ROOM);
        }

        if (ch->fighting && number_percent() > (get_curr_dex(ch) * 2 + 48))
        {
                act(AT_MAGIC,
                    "$n accidentally drops $p rendering it useless.", ch, obj,
                    NULL, TO_ROOM);
                act(AT_MAGIC,
                    "Oops... $p gets knocked from your hands rendering it completely useless!",
                    ch, obj, NULL, TO_CHAR);
        }
        else
        {
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        act(AT_ACTION, "$n takes $p.", ch, obj, NULL,
                            TO_ROOM);
                        act(AT_ACTION, "You take $p.", ch, obj, NULL,
                            TO_CHAR);
                }

                if (IS_NPC(ch))
                {
                        extract_obj(obj);
                        return;
                }

                drug = obj->value[0];

                WAIT_STATE(ch, PULSE_PER_SECOND / 4);

                gain_condition(ch, COND_THIRST, 1);

                ch->pcdata->drug_level[drug] =
                        UMIN(ch->pcdata->drug_level[drug] + obj->value[1],
                             255);
                if (ch->pcdata->drug_level[drug] >= 255
                    || ch->pcdata->drug_level[drug] >
                    (ch->pcdata->addiction[drug] + 100))
                {
                        act(AT_POISON, "$n sputters and gags.", ch, NULL,
                            NULL, TO_ROOM);
                        act(AT_POISON,
                            "You feel sick. You may have taken too much.", ch,
                            NULL, NULL, TO_CHAR);
                        ch->mental_state =
                                URANGE(20, ch->mental_state + 5, 100);
                        af.type = gsn_poison;
                        af.location = APPLY_INT;
                        af.modifier = -5;
                        af.duration = ch->pcdata->drug_level[drug];
                        af.bitvector = AFF_POISON;
                        affect_to_char(ch, &af);
                        ch->hit = 1;
                }

                switch (drug)
                {
                default:
                case SPICE_GLITTERSTIM:

                        sn = skill_lookup("true sight");
                        if (sn < MAX_SKILL && !IS_AFFECTED(ch, AFF_TRUESIGHT))
                        {
                                af.type = sn;
                                af.location = APPLY_AC;
                                af.modifier = -10;
                                af.duration =
                                        URANGE(1,
                                               ch->pcdata->drug_level[drug] -
                                               ch->pcdata->addiction[drug],
                                               obj->value[1]);
                                af.bitvector = AFF_TRUESIGHT;
                                affect_to_char(ch, &af);
                        }
                        break;

                case SPICE_CARSANUM:

                        sn = skill_lookup("sanctuary");
                        if (sn < MAX_SKILL && !IS_AFFECTED(ch, AFF_SANCTUARY))
                        {
                                af.type = sn;
                                af.location = APPLY_NONE;
                                af.modifier = 0;
                                af.duration =
                                        URANGE(1,
                                               ch->pcdata->drug_level[drug] -
                                               ch->pcdata->addiction[drug],
                                               obj->value[1]);
                                af.bitvector = AFF_SANCTUARY;
                                affect_to_char(ch, &af);
                        }
                        break;

                case SPICE_RYLL:

                        af.type = -1;
                        af.location = APPLY_DEX;
                        af.modifier = 1;
                        af.duration =
                                URANGE(1,
                                       2 * (ch->pcdata->drug_level[drug] -
                                            ch->pcdata->addiction[drug]),
                                       2 * obj->value[1]);
                        af.bitvector = AFF_NONE;
                        affect_to_char(ch, &af);

                        af.type = -1;
                        af.location = APPLY_HITROLL;
                        af.modifier = 1;
                        af.duration =
                                URANGE(1,
                                       2 * (ch->pcdata->drug_level[drug] -
                                            ch->pcdata->addiction[drug]),
                                       2 * obj->value[1]);
                        af.bitvector = AFF_NONE;
                        affect_to_char(ch, &af);

                        break;

                case SPICE_ANDRIS:

                        af.type = -1;
                        af.location = APPLY_HIT;
                        af.modifier = 10;
                        af.duration =
                                URANGE(1,
                                       2 * (ch->pcdata->drug_level[drug] -
                                            ch->pcdata->addiction[drug]),
                                       2 * obj->value[1]);
                        af.bitvector = AFF_NONE;
                        affect_to_char(ch, &af);

                        af.type = sn;
                        af.location = APPLY_CON;
                        af.modifier = 1;
                        af.duration =
                                URANGE(1,
                                       2 * (ch->pcdata->drug_level[drug] -
                                            ch->pcdata->addiction[drug]),
                                       2 * obj->value[1]);
                        af.bitvector = AFF_NONE;
                        affect_to_char(ch, &af);

                        break;

                }

        }

        extract_obj(obj);
        return;
}

void jedi_bonus(CHAR_DATA * ch)
{
        if (number_range(1, 100) == 1)
        {
                ch->max_endurance++;
                send_to_char("&YYou are wise in your use of the force.\n\r",
                             ch);
                send_to_char
                        ("You feel a little stronger in your wisdom.&w\n\r",
                         ch);
        }
}

void sith_penalty(CHAR_DATA * ch)
{
        if (number_range(1, 100) == 1)
        {
                ch->max_endurance++;
                if (ch->max_hit > 100)
                        ch->max_hit--;
                ch->hit--;
                send_to_char
                        ("&zYour body grows weaker as your strength in the dark side grows.&w\n\r",
                         ch);
        }
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
CMDF do_fill(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        OBJ_DATA *source;
        sh_int    dest_item, src_item1, src_item2, src_item3, src_item4;
        int       diff = 0;
        bool      all = FALSE;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        /*
         * munch optional words 
         */
        if ((!str_cmp(arg2, "from") || !str_cmp(arg2, "with"))
            && argument[0] != '\0')
                argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Fill what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((obj = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }
        else
                dest_item = obj->item_type;

        src_item1 = src_item2 = src_item3 = src_item4 = -1;
        switch (dest_item)
        {
        default:
                act(AT_ACTION, "$n tries to fill $p... (Don't ask me how)",
                    ch, obj, NULL, TO_ROOM);
                send_to_char("You cannot fill that.\n\r", ch);
                return;
                /*
                 * place all fillable item types here 
                 */
        case ITEM_DRINK_CON:
                src_item1 = ITEM_FOUNTAIN;
                src_item2 = ITEM_BLOOD;
                break;
        case ITEM_CONTAINER:
                src_item1 = ITEM_CONTAINER;
                src_item2 = ITEM_CORPSE_NPC;
                src_item3 = ITEM_CORPSE_PC;
                src_item4 = ITEM_CORPSE_NPC;
                break;
        }

        if (dest_item == ITEM_CONTAINER)
        {
                if (IS_SET(obj->value[1], CONT_CLOSED))
                {
                        act(AT_PLAIN, "The $d is closed.", ch, NULL,
                            obj->name, TO_CHAR);
                        return;
                }
                if (get_obj_weight(obj) / obj->count >= obj->value[0])
                {
                        send_to_char("It's already full as it can be.\n\r",
                                     ch);
                        return;
                }
        }
        else
        {
                diff = obj->value[0] - obj->value[1];
                if (diff < 1 || obj->value[1] >= obj->value[0])
                {
                        send_to_char("It's already full as it can be.\n\r",
                                     ch);
                        return;
                }
        }

        if (dest_item == ITEM_PIPE && IS_SET(obj->value[3], PIPE_FULLOFASH))
        {
                send_to_char
                        ("It's full of ashes, and needs to be emptied first.\n\r",
                         ch);
                return;
        }

        if (arg2[0] != '\0')
        {
                if (dest_item == ITEM_CONTAINER
                    && (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)))
                {
                        all = TRUE;
                        source = NULL;
                }
                else
                        /*
                         * This used to let you fill a pipe from an object on the ground.  Seems
                         * to me you should be holding whatever you want to fill a pipe with.
                         * It's nitpicking, but I needed to change it to get a mobprog to work
                         * right.  Check out Lord Fitzgibbon if you're curious.  -Narn 
                         */
                if (dest_item == ITEM_PIPE)
                {
                        if ((source = get_obj_carry(ch, arg2)) == NULL)
                        {
                                send_to_char("You don't have that item.\n\r",
                                             ch);
                                return;
                        }
                        if (source->item_type != src_item1
                            && source->item_type != src_item2
                            && source->item_type != src_item3
                            && source->item_type != src_item4)
                        {
                                act(AT_PLAIN, "You cannot fill $p with $P!",
                                    ch, obj, source, TO_CHAR);
                                return;
                        }
                }
                else
                {
                        if ((source = get_obj_here(ch, arg2)) == NULL)
                        {
                                send_to_char("You cannot find that item.\n\r",
                                             ch);
                                return;
                        }
                }
        }
        else
                source = NULL;

        if (!source && dest_item == ITEM_PIPE)
        {
                send_to_char("Fill it with what?\n\r", ch);
                return;
        }

        if (!source)
        {
                bool      found = FALSE;
                OBJ_DATA *src_next;

                found = FALSE;
                separate_obj(obj);
                for (source = ch->in_room->first_content;
                     source; source = src_next)
                {
                        src_next = source->next_content;
                        if (dest_item == ITEM_CONTAINER)
                        {
                                if (!CAN_WEAR(source, ITEM_TAKE)
                                    || (IS_OBJ_STAT(source, ITEM_PROTOTYPE)
                                        && !can_take_proto(ch))
                                    || ch->carry_weight +
                                    get_obj_weight(source) > can_carry_w(ch)
                                    || (get_obj_weight(source) +
                                        get_obj_weight(obj) / obj->count) >
                                    obj->value[0])
                                        continue;
                                if (all && arg2[3] == '.'
                                    && !nifty_is_name(&arg2[4], source->name))
                                        continue;
                                obj_from_room(source);
                                if (source->item_type == ITEM_MONEY)
                                {
                                        ch->gold += source->value[0];
                                        extract_obj(source);
                                }
                                else
                                        obj_to_obj(source, obj);
                                found = TRUE;
                        }
                        else if (source->item_type == src_item1
                                 || source->item_type == src_item2
                                 || source->item_type == src_item3
                                 || source->item_type == src_item4)
                        {
                                found = TRUE;
                                break;
                        }
                }
                if (!found)
                {
                        switch (src_item1)
                        {
                        default:
                                send_to_char
                                        ("There is nothing appropriate here!\n\r",
                                         ch);
                                return;
                        case ITEM_FOUNTAIN:
                                send_to_char
                                        ("There is no fountain or pool here!\n\r",
                                         ch);
                                return;
                        case ITEM_BLOOD:
                                send_to_char
                                        ("There is no blood pool here!\n\r",
                                         ch);
                                return;
                        }
                }
                if (dest_item == ITEM_CONTAINER)
                {
                        act(AT_ACTION, "You fill $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n fills $p.", ch, obj, NULL,
                            TO_ROOM);
                        return;
                }
        }

        if (dest_item == ITEM_CONTAINER)
        {
                OBJ_DATA *otmp, *otmp_next;
                char      name[MAX_INPUT_LENGTH];
                CHAR_DATA *gch;
                char     *pd;
                bool      found = FALSE;

                if (source == obj)
                {
                        send_to_char
                                ("You can't fill something with itself!\n\r",
                                 ch);
                        return;
                }

                switch (source->item_type)
                {
                default:   /* put something in container */
                        if (!source->in_room    /* disallow inventory items */
                            || !CAN_WEAR(source, ITEM_TAKE)
                            || (IS_OBJ_STAT(source, ITEM_PROTOTYPE)
                                && !can_take_proto(ch))
                            || ch->carry_weight + get_obj_weight(source) >
                            can_carry_w(ch)
                            || (get_obj_weight(source) +
                                get_obj_weight(obj) / obj->count) >
                            obj->value[0])
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return;
                        }
                        separate_obj(obj);
                        act(AT_ACTION, "You take $P and put it inside $p.",
                            ch, obj, source, TO_CHAR);
                        act(AT_ACTION, "$n takes $P and puts it inside $p.",
                            ch, obj, source, TO_ROOM);
                        obj_from_room(source);
                        obj_to_obj(source, obj);
                        break;
                case ITEM_MONEY:
                        send_to_char("You can't do that... yet.\n\r", ch);
                        break;
                case ITEM_CORPSE_PC:
                        if (IS_NPC(ch))
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return;
                        }

                        pd = source->short_descr;
                        pd = one_argument(pd, name);
                        pd = one_argument(pd, name);
                        pd = one_argument(pd, name);
                        pd = one_argument(pd, name);

                        if (str_cmp(name, ch->name) && !IS_IMMORTAL(ch))
                        {
                                bool      fGroup;

                                fGroup = FALSE;
                                for (gch = first_char; gch; gch = gch->next)
                                {
                                        if (!IS_NPC(gch)
                                            && is_same_group(ch, gch)
                                            && !str_cmp(name, gch->name))
                                        {
                                                fGroup = TRUE;
                                                break;
                                        }
                                }
                                if (!fGroup)
                                {
                                        send_to_char
                                                ("That's someone else's corpse.\n\r",
                                                 ch);
                                        return;
                                }
                        }

                case ITEM_CONTAINER:
                        if (source->item_type == ITEM_CONTAINER /* don't remove */
                            && IS_SET(source->value[1], CONT_CLOSED))
                        {
                                act(AT_PLAIN, "The $d is closed.", ch, NULL,
                                    source->name, TO_CHAR);
                                return;
                        }
                case ITEM_DROID_CORPSE:
                case ITEM_CORPSE_NPC:
                        if ((otmp = source->first_content) == NULL)
                        {
                                send_to_char("It's empty.\n\r", ch);
                                return;
                        }
                        separate_obj(obj);
                        for (; otmp; otmp = otmp_next)
                        {
                                otmp_next = otmp->next_content;

                                if (!CAN_WEAR(otmp, ITEM_TAKE)
                                    || (IS_OBJ_STAT(otmp, ITEM_PROTOTYPE)
                                        && !can_take_proto(ch))
                                    || ch->carry_number + otmp->count >
                                    can_carry_n(ch)
                                    || ch->carry_weight +
                                    get_obj_weight(otmp) > can_carry_w(ch)
                                    || (get_obj_weight(source) +
                                        get_obj_weight(obj) / obj->count) >
                                    obj->value[0])
                                        continue;
                                obj_from_obj(otmp);
                                obj_to_obj(otmp, obj);
                                found = TRUE;
                        }
                        if (found)
                        {
                                act(AT_ACTION, "You fill $p from $P.", ch,
                                    obj, source, TO_CHAR);
                                act(AT_ACTION, "$n fills $p from $P.", ch,
                                    obj, source, TO_ROOM);
                        }
                        else
                                send_to_char
                                        ("There is nothing appropriate in there.\n\r",
                                         ch);
                        break;
                }
                return;
        }

        if (source->value[1] < 1)
        {
                send_to_char("There's none left!\n\r", ch);
                return;
        }
        if (source->count > 1 && source->item_type != ITEM_FOUNTAIN)
                separate_obj(source);
        separate_obj(obj);

        switch (source->item_type)
        {
        default:
                bug("do_fill: got bad item type: %d", source->item_type);
                send_to_char("Something went wrong...\n\r", ch);
                return;
        case ITEM_FOUNTAIN:
                if (obj->value[1] != 0 && obj->value[2] != 0)
                {
                        send_to_char
                                ("There is already another liquid in it.\n\r",
                                 ch);
                        return;
                }
                obj->value[2] = 0;
                obj->value[1] = obj->value[0];
                act(AT_ACTION, "You fill $p from $P.", ch, obj, source,
                    TO_CHAR);
                act(AT_ACTION, "$n fills $p from $P.", ch, obj, source,
                    TO_ROOM);
                return;
        case ITEM_BLOOD:
                if (obj->value[1] != 0 && obj->value[2] != 13)
                {
                        send_to_char
                                ("There is already another liquid in it.\n\r",
                                 ch);
                        return;
                }
                obj->value[2] = 13;
                if (source->value[1] < diff)
                        diff = source->value[1];
                obj->value[1] += diff;
                act(AT_ACTION, "You fill $p from $P.", ch, obj, source,
                    TO_CHAR);
                act(AT_ACTION, "$n fills $p from $P.", ch, obj, source,
                    TO_ROOM);
                if ((source->value[1] -= diff) < 1)
                {
                        extract_obj(source);
                        make_bloodstain(ch);
                }
                return;
        case ITEM_DRINK_CON:
                if (obj->value[1] != 0 && obj->value[2] != source->value[2])
                {
                        send_to_char
                                ("There is already another liquid in it.\n\r",
                                 ch);
                        return;
                }
                obj->value[2] = source->value[2];
                if (source->value[1] < diff)
                        diff = source->value[1];
                obj->value[1] += diff;
                source->value[1] -= diff;
                act(AT_ACTION, "You fill $p from $P.", ch, obj, source,
                    TO_CHAR);
                act(AT_ACTION, "$n fills $p from $P.", ch, obj, source,
                    TO_ROOM);
                return;
        }
}

CMDF do_drink(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        int       amount;
        int       liquid;

        /*
         * Droids can't drink 
         */
        if (!str_cmp(ch->race->name(), "droid"))
        {
                send_to_char
                        ("You are a droid. What are you trying to do, kill yourself?.\n\r",
                         ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_JAW))
        {
                send_to_char("Your jaw hurts too much to drink.", ch);
                return;
        }

        argument = one_argument(argument, arg);

        /*
         * munch optional words 
         */
        if (!str_cmp(arg, "from") && argument[0] != '\0')
                argument = one_argument(argument, arg);

        if (arg[0] == '\0' || !str_cmp(arg, "fountain"))
        {
                for (obj = ch->in_room->first_content; obj;
                     obj = obj->next_content)
                        if (obj->item_type == ITEM_FOUNTAIN
                            || obj->item_type == ITEM_BLOOD)
                                break;

                if (!obj)
                {
                        send_to_char("Drink what?\n\r", ch);
                        return;
                }
        }
        else
        {
                /*
                 * Should we check for wrist, arm, and sholder too? 
                 */
                if (IS_SET(ch->bodyparts, BODY_R_HAND)
                    || IS_SET(ch->bodyparts, BODY_L_HAND))
                {
                        send_to_char
                                ("How do you expect to drink with your hands broken?",
                                 ch);
                        return;
                }

                if ((obj = get_obj_here(ch, arg)) == NULL)
                {
                        send_to_char("You can't find it.\n\r", ch);
                        return;
                }
        }

        if (obj->count > 1 && obj->item_type != ITEM_FOUNTAIN)
                separate_obj(obj);

        if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 40)
        {
                send_to_char("You fail to reach your mouth.  *Hic*\n\r", ch);
                return;
        }

        switch (obj->item_type)
        {
        default:
                if (obj->carried_by == ch)
                {
                        act(AT_ACTION,
                            "$n lifts $p up to $s mouth and tries to drink from it...",
                            ch, obj, NULL, TO_ROOM);
                        act(AT_ACTION,
                            "You bring $p up to your mouth and try to drink from it...",
                            ch, obj, NULL, TO_CHAR);
                }
                else
                {
                        act(AT_ACTION,
                            "$n gets down and tries to drink from $p... (Is $e feeling ok?)",
                            ch, obj, NULL, TO_ROOM);
                        act(AT_ACTION,
                            "You get down on the ground and try to drink from $p...",
                            ch, obj, NULL, TO_CHAR);
                }
                break;

        case ITEM_POTION:
                if (obj->carried_by == ch)
                        do_quaff(ch, obj->name);
                else
                        send_to_char("You're not carrying that.\n\r", ch);
                break;

        case ITEM_FOUNTAIN:
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        act(AT_ACTION, "$n drinks from the fountain.", ch,
                            NULL, NULL, TO_ROOM);
                        send_to_char
                                ("You take a long thirst quenching drink.\n\r",
                                 ch);
                }

                if (!IS_NPC(ch))
                        ch->pcdata->condition[COND_THIRST] = 40;
                break;

        case ITEM_DRINK_CON:
                if (obj->value[1] <= 0)
                {
                        send_to_char("It is already empty.\n\r", ch);
                        return;
                }

                if ((liquid = obj->value[2]) >= LIQ_MAX)
                {
                        bug("Do_drink: bad liquid number %d.", liquid);
                        liquid = obj->value[2] = 0;
                }

                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        act(AT_ACTION, "$n drinks $T from $p.",
                            ch, obj, liq_table[liquid].liq_name, TO_ROOM);
                        act(AT_ACTION, "You drink $T from $p.",
                            ch, obj, liq_table[liquid].liq_name, TO_CHAR);
                }

                amount = 1; /* UMIN(amount, obj->value[1]); */
                /*
                 * what was this? concentrated drinks?  concentrated water
                 * * too I suppose... sheesh! 
                 */

                gain_condition(ch, COND_DRUNK,
                               amount *
                               liq_table[liquid].liq_affect[COND_DRUNK]);
                gain_condition(ch, COND_FULL,
                               amount *
                               liq_table[liquid].liq_affect[COND_FULL]);
                gain_condition(ch, COND_THIRST,
                               amount *
                               liq_table[liquid].liq_affect[COND_THIRST]);

                if (!IS_NPC(ch))
                {
                        if (ch->pcdata->condition[COND_DRUNK] > 24)
                                send_to_char("You feel quite sloshed.\n\r",
                                             ch);
                        else if (ch->pcdata->condition[COND_DRUNK] > 18)
                                send_to_char("You feel very drunk.\n\r", ch);
                        else if (ch->pcdata->condition[COND_DRUNK] > 12)
                                send_to_char("You feel drunk.\n\r", ch);
                        else if (ch->pcdata->condition[COND_DRUNK] > 8)
                                send_to_char("You feel a little drunk.\n\r",
                                             ch);
                        else if (ch->pcdata->condition[COND_DRUNK] > 5)
                                send_to_char("You feel light headed.\n\r",
                                             ch);

                        if (ch->pcdata->condition[COND_FULL] > 40)
                                send_to_char("You are full.\n\r", ch);

                        if (ch->pcdata->condition[COND_THIRST] > 40)
                                send_to_char("You feel bloated.\n\r", ch);
                        else if (ch->pcdata->condition[COND_THIRST] > 36)
                                send_to_char
                                        ("Your stomach is sloshing around.\n\r",
                                         ch);
                        else if (ch->pcdata->condition[COND_THIRST] > 30)
                                send_to_char("You do not feel thirsty.\n\r",
                                             ch);
                }

                if (obj->value[3])
                {
                        /*
                         * The drink was poisoned! 
                         */
                        AFFECT_DATA af;

                        act(AT_POISON, "$n sputters and gags.", ch, NULL,
                            NULL, TO_ROOM);
                        act(AT_POISON, "You sputter and gag.", ch, NULL, NULL,
                            TO_CHAR);
                        ch->mental_state =
                                URANGE(20, ch->mental_state + 5, 100);
                        af.type = gsn_poison;
                        af.duration = 3 * obj->value[3];
                        af.location = APPLY_NONE;
                        af.modifier = 0;
                        af.bitvector = AFF_POISON;
                        affect_join(ch, &af);
                }

                obj->value[1] -= amount;
                break;
        }

        WAIT_STATE(ch, PULSE_PER_SECOND);
        return;
}

CMDF do_eat(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        ch_ret    retcode;
        int       foodcond;

        if (argument[0] == '\0')
        {
                send_to_char("Eat what?\n\r", ch);
                return;
        }

        if (!str_cmp(ch->race->name(), "droid"))
        {
                send_to_char
                        ("You are a droid. You don't need to eat. Stop it.\n\r",
                         ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_JAW))
        {
                send_to_char("Your jaw hurts too much to eat.", ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to eat with your hands broken?",
                         ch);
                return;
        }

        if (IS_NPC(ch) || ch->pcdata->condition[COND_FULL] > 5)
                if (ms_find_obj(ch))
                        return;

        if ((obj = find_obj(ch, argument, TRUE)) == NULL)
                return;


        if (!IS_IMMORTAL(ch))
        {
                if (obj->item_type != ITEM_FOOD
                    && obj->item_type != ITEM_PILL)
                {
                        act(AT_ACTION,
                            "$n starts to nibble on $p... ($e must really be hungry)",
                            ch, obj, NULL, TO_ROOM);
                        act(AT_ACTION, "You try to nibble on $p...", ch, obj,
                            NULL, TO_CHAR);
                        return;
                }

                if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40)
                {
                        send_to_char("You are too full to eat more.\n\r", ch);
                        return;
                }
        }

        /*
         * required due to object grouping 
         */
        separate_obj(obj);

        WAIT_STATE(ch, PULSE_PER_SECOND / 2);

        if (obj->in_obj)
        {
                act(AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj,
                    TO_CHAR);
                act(AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj,
                    TO_ROOM);
        }
        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
        {
                if (!obj->action_desc || obj->action_desc[0] == '\0')
                {
                        act(AT_ACTION, "$n eats $p.", ch, obj, NULL, TO_ROOM);
                        act(AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR);
                }
                else
                        actiondesc(ch, obj, NULL);
        }

        switch (obj->item_type)
        {

        case ITEM_FOOD:
                if (obj->timer > 0 && obj->value[1] > 0)
                        foodcond = (obj->timer * 10) / obj->value[1];
                else
                        foodcond = 10;

                if (!IS_NPC(ch))
                {
                        int       condition;

                        condition = ch->pcdata->condition[COND_FULL];
                        gain_condition(ch, COND_FULL,
                                       (obj->value[0] * foodcond) / 10);
                        if (condition <= 1
                            && ch->pcdata->condition[COND_FULL] > 1)
                                send_to_char("You are no longer hungry.\n\r",
                                             ch);
                        else if (ch->pcdata->condition[COND_FULL] > 40)
                                send_to_char("You are full.\n\r", ch);
                }

                if (obj->value[3] != 0
                    || (foodcond < 4 && number_range(0, foodcond + 1) == 0))
                {
                        /*
                         * The food was poisoned! 
                         */
                        AFFECT_DATA af;

                        if (obj->value[3] != 0)
                        {
                                act(AT_POISON, "$n chokes and gags.", ch,
                                    NULL, NULL, TO_ROOM);
                                act(AT_POISON, "You choke and gag.", ch, NULL,
                                    NULL, TO_CHAR);
                                ch->mental_state =
                                        URANGE(20, ch->mental_state + 5, 100);
                        }
                        else
                        {
                                act(AT_POISON, "$n gags on $p.", ch, obj,
                                    NULL, TO_ROOM);
                                act(AT_POISON, "You gag on $p.", ch, obj,
                                    NULL, TO_CHAR);
                                ch->mental_state =
                                        URANGE(15, ch->mental_state + 5, 100);
                        }

                        af.type = gsn_poison;
                        af.duration =
                                2 * obj->value[0] * (obj->value[3] >
                                                     0 ? obj->value[3] : 1);
                        af.location = APPLY_NONE;
                        af.modifier = 0;
                        af.bitvector = AFF_POISON;
                        affect_join(ch, &af);
                }
                break;

        case ITEM_PILL:
                /*
                 * allow pills to fill you, if so desired 
                 */
                if (!IS_NPC(ch) && obj->value[4])
                {
                        int       condition;

                        condition = ch->pcdata->condition[COND_FULL];
                        gain_condition(ch, COND_FULL, obj->value[4]);
                        if (condition <= 1
                            && ch->pcdata->condition[COND_FULL] > 1)
                                send_to_char("You are no longer hungry.\n\r",
                                             ch);
                        else if (ch->pcdata->condition[COND_FULL] > 40)
                                send_to_char("You are full.\n\r", ch);
                }
                retcode =
                        obj_cast_spell(obj->value[1], obj->value[0], ch, ch,
                                       NULL);
                if (retcode == rNONE)
                        retcode =
                                obj_cast_spell(obj->value[2], obj->value[0],
                                               ch, ch, NULL);
                if (retcode == rNONE)
                        retcode =
                                obj_cast_spell(obj->value[3], obj->value[0],
                                               ch, ch, NULL);
                break;
        }

        extract_obj(obj);
        return;
}

CMDF do_quaff(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        ch_ret    retcode;

        if (argument[0] == '\0' || !str_cmp(argument, ""))
        {
                send_to_char("Quaff what?\n\r", ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_JAW))
        {
                send_to_char("Your jaw hurts too much to quaff.", ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to drink with your hands broken?",
                         ch);
                return;
        }

        if ((obj = find_obj(ch, argument, TRUE)) == NULL)
                return;

        if (obj->item_type != ITEM_POTION)
        {
                if (obj->item_type == ITEM_DRINK_CON)
                        do_drink(ch, obj->name);
                else
                {
                        act(AT_ACTION,
                            "$n lifts $p up to $s mouth and tries to drink from it...",
                            ch, obj, NULL, TO_ROOM);
                        act(AT_ACTION,
                            "You bring $p up to your mouth and try to drink from it...",
                            ch, obj, NULL, TO_CHAR);
                }
                return;
        }

        /*
         * Fullness checking                    -Thoric
         */
        if (!IS_NPC(ch)
            && (ch->pcdata->condition[COND_FULL] >= 48
                || ch->pcdata->condition[COND_THIRST] >= 48))
        {
                send_to_char("Your stomach cannot contain any more.\n\r", ch);
                return;
        }

        separate_obj(obj);
        if (obj->in_obj)
        {
                act(AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj,
                    TO_CHAR);
                act(AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj,
                    TO_ROOM);
        }

        /*
         * If fighting, chance of dropping potion           -Thoric
         */
        if (ch->fighting && number_percent() > (get_curr_dex(ch) * 2 + 48))
        {
                act(AT_MAGIC,
                    "$n accidentally drops $p and it smashes into a thousand fragments.",
                    ch, obj, NULL, TO_ROOM);
                act(AT_MAGIC,
                    "Oops... $p gets knocked from your hands and smashes into pieces!",
                    ch, obj, NULL, TO_CHAR);
        }
        else
        {
                if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
                {
                        act(AT_ACTION, "$n quaffs $p.", ch, obj, NULL,
                            TO_ROOM);
                        act(AT_ACTION, "You quaff $p.", ch, obj, NULL,
                            TO_CHAR);
                }

                WAIT_STATE(ch, PULSE_PER_SECOND / 4);

                gain_condition(ch, COND_THIRST, 1);
                retcode =
                        obj_cast_spell(obj->value[1], obj->value[0], ch, ch,
                                       NULL);
                if (retcode == rNONE)
                        retcode =
                                obj_cast_spell(obj->value[2], obj->value[0],
                                               ch, ch, NULL);
                if (retcode == rNONE)
                        retcode =
                                obj_cast_spell(obj->value[3], obj->value[0],
                                               ch, ch, NULL);
        }

        extract_obj(obj);
        return;
}


CMDF do_recite(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        OBJ_DATA *scroll;
        OBJ_DATA *obj;
        ch_ret    retcode;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Activate what?\n\r", ch);
                return;
        }

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_JAW))
        {
                send_to_char
                        ("How do you expect to do that with your jaw broken?",
                         ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((scroll = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }

        if (scroll->item_type != ITEM_SCROLL)
        {
                act(AT_ACTION,
                    "$n attempts to activate $p ... the silly fool.", ch,
                    scroll, NULL, TO_ROOM);
                act(AT_ACTION, "You try to activate $p. (Now what?)", ch,
                    scroll, NULL, TO_CHAR);
                return;
        }

        if (IS_NPC(ch)
            && (scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING))
        {
                send_to_char("As a mob, this dialect is foreign to you.\n\r",
                             ch);
                return;
        }

        if ((scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING)
            && (ch->top_level + 10 < scroll->value[0]))
        {
                send_to_char
                        ("This item is too complex for you to understand.\n\r",
                         ch);
                return;
        }

        obj = NULL;
        if (arg2[0] == '\0')
                victim = ch;
        else
        {
                if ((victim = get_char_room(ch, arg2)) == NULL
                    && (obj = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char("You can't find it.\n\r", ch);
                        return;
                }
        }

        separate_obj(scroll);
        act(AT_MAGIC, "$n activate $p.", ch, scroll, NULL, TO_ROOM);
        act(AT_MAGIC, "You activate $p.", ch, scroll, NULL, TO_CHAR);


        WAIT_STATE(ch, PULSE_PER_SECOND / 2);

        retcode =
                obj_cast_spell(scroll->value[1], scroll->value[0], ch, victim,
                               obj);
        if (retcode == rNONE)
                retcode =
                        obj_cast_spell(scroll->value[2], scroll->value[0], ch,
                                       victim, obj);
        if (retcode == rNONE)
                retcode =
                        obj_cast_spell(scroll->value[3], scroll->value[0], ch,
                                       victim, obj);

        extract_obj(scroll);
        return;
}


/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush(CHAR_DATA * ch, OBJ_DATA * obj, bool pull)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *rch;
        bool      isup;
        ROOM_INDEX_DATA *room, *to_room = NULL;
        EXIT_DATA *pexit, *pexit_rev;
        int       edir;
        char     *txt;

        if (IS_SET(obj->value[0], TRIG_UP))
                isup = TRUE;
        else
                isup = FALSE;
        switch (obj->item_type)
        {
        default:
                snprintf(buf, 25, "You can't %s that!\n\r",
                         pull ? "pull" : "push");
                send_to_char(buf, ch);
                return;
                break;
        case ITEM_SWITCH:
        case ITEM_LEVER:
        case ITEM_PULLCHAIN:
                if ((!pull && isup) || (pull && !isup))
                {
                        snprintf(buf, MSL, "It is already %s.\n\r",
                                 isup ? "up" : "down");
                        send_to_char(buf, ch);
                        return;
                }
        case ITEM_BUTTON:
                if ((!pull && isup) || (pull & !isup))
                {
                        snprintf(buf, MSL, "It is already %s.\n\r",
                                 isup ? "in" : "out");
                        send_to_char(buf, ch);
                        return;
                }
                break;
        }
        if ((pull) && IS_SET(obj->pIndexData->progtypes, PULL_PROG))
        {
                if (!IS_SET(obj->value[0], TRIG_AUTORETURN))
                        REMOVE_BIT(obj->value[0], TRIG_UP);
                oprog_pull_trigger(ch, obj);
                return;
        }
        if ((!pull) && IS_SET(obj->pIndexData->progtypes, PUSH_PROG))
        {
                if (!IS_SET(obj->value[0], TRIG_AUTORETURN))
                        SET_BIT(obj->value[0], TRIG_UP);
                oprog_push_trigger(ch, obj);
                return;
        }

        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
        {
                snprintf(buf, MSL, "$n %s $p.", pull ? "pulls" : "pushes");
                act(AT_ACTION, buf, ch, obj, NULL, TO_ROOM);
                snprintf(buf, MSL, "You %s $p.", pull ? "pull" : "push");
                act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
        }

        if (!IS_SET(obj->value[0], TRIG_AUTORETURN))
        {
                if (pull)
                        REMOVE_BIT(obj->value[0], TRIG_UP);
                else
                        SET_BIT(obj->value[0], TRIG_UP);
        }

        if (IS_SET(obj->value[0], TRIG_RAND4)
            || IS_SET(obj->value[0], TRIG_RAND6))
        {
                int       maxd;

                if ((room = get_room_index(obj->value[1])) == NULL)
                {
                        bug("PullOrPush: obj points to invalid room %d",
                            obj->value[1]);
                        return;
                }

                if (IS_SET(obj->value[0], TRIG_RAND4))
                        maxd = 3;
                else
                        maxd = 5;

                randomize_exits(room, maxd);
                for (rch = room->first_person; rch; rch = rch->next_in_room)
                {
                        send_to_char("You hear a loud rumbling sound.\n\r",
                                     rch);
                        send_to_char("Something seems different...\n\r", rch);
                }
        }
        if (IS_SET(obj->value[0], TRIG_DOOR))
        {
                room = get_room_index(obj->value[1]);
                if (!room)
                        room = obj->in_room;
                if (!room)
                {
                        bug("PullOrPush: obj points to invalid room %d",
                            obj->value[1]);
                        return;
                }
                if (IS_SET(obj->value[0], TRIG_D_NORTH))
                {
                        edir = DIR_NORTH;
                        txt = "to the north";
                }
                else if (IS_SET(obj->value[0], TRIG_D_SOUTH))
                {
                        edir = DIR_SOUTH;
                        txt = "to the south";
                }
                else if (IS_SET(obj->value[0], TRIG_D_EAST))
                {
                        edir = DIR_EAST;
                        txt = "to the east";
                }
                else if (IS_SET(obj->value[0], TRIG_D_WEST))
                {
                        edir = DIR_WEST;
                        txt = "to the west";
                }
                else if (IS_SET(obj->value[0], TRIG_D_UP))
                {
                        edir = DIR_UP;
                        txt = "from above";
                }
                else if (IS_SET(obj->value[0], TRIG_D_DOWN))
                {
                        edir = DIR_DOWN;
                        txt = "from below";
                }
                else
                {
                        bug("PullOrPush: door: no direction flag set.", 0);
                        return;
                }
                pexit = get_exit(room, edir);
                if (!pexit)
                {
                        if (!IS_SET(obj->value[0], TRIG_PASSAGE))
                        {
                                bug("PullOrPush: obj points to non-exit %d",
                                    obj->value[1]);
                                return;
                        }
                        to_room = get_room_index(obj->value[2]);
                        if (!to_room)
                        {
                                bug("PullOrPush: dest points to invalid room %d", obj->value[2]);
                                return;
                        }
                        pexit = make_exit(room, to_room, edir);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                        pexit->exit_info = 0;
                        top_exit++;
                        act(AT_PLAIN, "A passage opens!", ch, NULL, NULL,
                            TO_CHAR);
                        act(AT_PLAIN, "A passage opens!", ch, NULL, NULL,
                            TO_ROOM);
                        return;
                }
                if (IS_SET(obj->value[0], TRIG_UNLOCK)
                    && IS_SET(pexit->exit_info, EX_LOCKED))
                {
                        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                        act(AT_PLAIN, "You hear a faint click $T.", ch, NULL,
                            txt, TO_CHAR);
                        act(AT_PLAIN, "You hear a faint click $T.", ch, NULL,
                            txt, TO_ROOM);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                                REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
                        return;
                }
                if (IS_SET(obj->value[0], TRIG_LOCK)
                    && !IS_SET(pexit->exit_info, EX_LOCKED))
                {
                        SET_BIT(pexit->exit_info, EX_LOCKED);
                        act(AT_PLAIN, "You hear a faint click $T.", ch, NULL,
                            txt, TO_CHAR);
                        act(AT_PLAIN, "You hear a faint click $T.", ch, NULL,
                            txt, TO_ROOM);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                                SET_BIT(pexit_rev->exit_info, EX_LOCKED);
                        return;
                }
                if (IS_SET(obj->value[0], TRIG_OPEN)
                    && IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
                        for (rch = room->first_person; rch;
                             rch = rch->next_in_room)
                                act(AT_ACTION, "The $d opens.", rch, NULL,
                                    pexit->keyword, TO_CHAR);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
                                for (rch = to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                        act(AT_ACTION, "The $d opens.", rch,
                                            NULL, pexit_rev->keyword,
                                            TO_CHAR);
                        }
                        check_room_for_traps(ch, trap_door[edir]);
                        return;
                }
                if (IS_SET(obj->value[0], TRIG_CLOSE)
                    && !IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        SET_BIT(pexit->exit_info, EX_CLOSED);
                        for (rch = room->first_person; rch;
                             rch = rch->next_in_room)
                                act(AT_ACTION, "The $d closes.", rch, NULL,
                                    pexit->keyword, TO_CHAR);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                SET_BIT(pexit_rev->exit_info, EX_CLOSED);
                                for (rch = to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                        act(AT_ACTION, "The $d closes.", rch,
                                            NULL, pexit_rev->keyword,
                                            TO_CHAR);
                        }
                        check_room_for_traps(ch, trap_door[edir]);
                        return;
                }
        }
}


CMDF do_pull(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Pull what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((obj = get_obj_here(ch, arg)) == NULL)
        {
                act(AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR);
                return;
        }

        pullorpush(ch, obj, TRUE);
}

CMDF do_push(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Push what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((obj = get_obj_here(ch, arg)) == NULL)
        {
                act(AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR);
                return;
        }

        pullorpush(ch, obj, FALSE);
}

CMDF do_empty(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];

        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (!str_cmp(arg2, "into") && argument[0] != '\0')
                argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Empty what?\n\r", ch);
                return;
        }
        if (ms_find_obj(ch))
                return;

        if ((obj = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You aren't carrying that.\n\r", ch);
                return;
        }
        if (obj->count > 1)
                separate_obj(obj);

        switch (obj->item_type)
        {
        default:
                act(AT_ACTION, "You shake $p in an attempt to empty it...",
                    ch, obj, NULL, TO_CHAR);
                act(AT_ACTION,
                    "$n begins to shake $p in an attempt to empty it...", ch,
                    obj, NULL, TO_ROOM);
                return;
        case ITEM_PIPE:
                act(AT_ACTION, "You gently tap $p and empty it out.", ch, obj,
                    NULL, TO_CHAR);
                act(AT_ACTION, "$n gently taps $p and empties it out.", ch,
                    obj, NULL, TO_ROOM);
                REMOVE_BIT(obj->value[3], PIPE_FULLOFASH);
                REMOVE_BIT(obj->value[3], PIPE_LIT);
                obj->value[1] = 0;
                return;
        case ITEM_DRINK_CON:
                if (obj->value[1] < 1)
                {
                        send_to_char("It's already empty.\n\r", ch);
                        return;
                }
                act(AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR);
                act(AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM);
                obj->value[1] = 0;
                return;
        case ITEM_CONTAINER:
                if (IS_SET(obj->value[1], CONT_CLOSED))
                {
                        act(AT_PLAIN, "The $d is closed.", ch, NULL,
                            obj->name, TO_CHAR);
                        return;
                }
                if (!obj->first_content)
                {
                        send_to_char("It's already empty.\n\r", ch);
                        return;
                }
                if (arg2[0] == '\0')
                {
                        if (xIS_SET(ch->in_room->room_flags, ROOM_NODROP)
                            || (!IS_NPC(ch)
                                && IS_SET(ch->act, PLR_LITTERBUG)))
                        {
                                set_char_color(AT_MAGIC, ch);
                                send_to_char("A magical force stops you!\n\r",
                                             ch);
                                set_char_color(AT_TELL, ch);
                                send_to_char
                                        ("Someone tells you, 'No littering here!'\n\r",
                                         ch);
                                return;
                        }
                        if (xIS_SET(ch->in_room->room_flags, ROOM_NODROPALL))
                        {
                                send_to_char
                                        ("You can't seem to do that here...\n\r",
                                         ch);
                                return;
                        }
                        if (empty_obj(obj, NULL, ch->in_room))
                        {
                                act(AT_ACTION, "You empty $p.", ch, obj, NULL,
                                    TO_CHAR);
                                act(AT_ACTION, "$n empties $p.", ch, obj,
                                    NULL, TO_ROOM);
                                if (IS_SET(sysdata.save_flags, SV_DROP))
                                        save_char_obj(ch);
                        }
                        else
                                send_to_char("Hmmm... didn't work.\n\r", ch);
                }
                else
                {
                        OBJ_DATA *dest = get_obj_here(ch, arg2);

                        if (!dest)
                        {
                                send_to_char("You can't find it.\n\r", ch);
                                return;
                        }
                        if (dest == obj)
                        {
                                send_to_char
                                        ("You can't empty something into itself!\n\r",
                                         ch);
                                return;
                        }
                        if (dest->item_type != ITEM_CONTAINER)
                        {
                                send_to_char("That's not a container!\n\r",
                                             ch);
                                return;
                        }
                        if (IS_SET(dest->value[1], CONT_CLOSED))
                        {
                                act(AT_PLAIN, "The $d is closed.", ch, NULL,
                                    dest->name, TO_CHAR);
                                return;
                        }
                        separate_obj(dest);
                        if (empty_obj(obj, dest, NULL))
                        {
                                act(AT_ACTION, "You empty $p into $P.", ch,
                                    obj, dest, TO_CHAR);
                                act(AT_ACTION, "$n empties $p into $P.", ch,
                                    obj, dest, TO_ROOM);
                                if (!dest->carried_by
                                    && IS_SET(sysdata.save_flags, SV_PUT))
                                        save_char_obj(ch);
                        }
                        else
                                act(AT_ACTION, "$P is too full.", ch, obj,
                                    dest, TO_CHAR);
                }
                return;
        }
}

/*
 * Apply a salve/ointment					-Thoric
 */
CMDF do_apply(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        ch_ret    retcode;

        if (argument[0] == '\0')
        {
                send_to_char("Apply what?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if ((obj = get_obj_carry(ch, argument)) == NULL)
        {
                send_to_char("You do not have that.\n\r", ch);
                return;
        }

        if (obj->item_type != ITEM_SALVE)
        {
                act(AT_ACTION, "$n starts to rub $p on $mself...", ch, obj,
                    NULL, TO_ROOM);
                act(AT_ACTION, "You try to rub $p on yourself...", ch, obj,
                    NULL, TO_CHAR);
                return;
        }

        separate_obj(obj);

        --obj->value[1];
        if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
        {
                if (!obj->action_desc || obj->action_desc[0] == '\0')
                {
                        act(AT_ACTION, "$n rubs $p onto $s body.", ch, obj,
                            NULL, TO_ROOM);
                        if (obj->value[1] <= 0)
                                act(AT_ACTION,
                                    "You apply the last of $p onto your body.",
                                    ch, obj, NULL, TO_CHAR);
                        else
                                act(AT_ACTION, "You apply $p onto your body.",
                                    ch, obj, NULL, TO_CHAR);
                }
                else
                        actiondesc(ch, obj, NULL);
        }

        WAIT_STATE(ch, obj->value[2]);
        retcode = obj_cast_spell(obj->value[4], obj->value[0], ch, ch, NULL);
        if (retcode == rNONE)
                retcode =
                        obj_cast_spell(obj->value[5], obj->value[0], ch, ch,
                                       NULL);

        if (!obj_extracted(obj) && obj->value[1] <= 0)
                extract_obj(obj);

        return;
}

void actiondesc(CHAR_DATA * ch, OBJ_DATA * obj, void *vo)
{
        char      charbuf[MAX_STRING_LENGTH];
        char      roombuf[MAX_STRING_LENGTH];
        char     *srcptr = obj->action_desc;
        char     *charptr = charbuf;
        char     *roomptr = roombuf;
        const char *ichar = NULL;
        const char *iroom = NULL;

        vo = NULL;

        while (*srcptr != '\0')
        {
                if (*srcptr == '$')
                {
                        srcptr++;
                        switch (*srcptr)
                        {
                        case 'e':
                                ichar = "you";
                                iroom = "$e";
                                break;

                        case 'm':
                                ichar = "you";
                                iroom = "$m";
                                break;

                        case 'n':
                                ichar = "you";
                                iroom = "$n";
                                break;

                        case 's':
                                ichar = "your";
                                iroom = "$s";
                                break;

                                /*
                                 * case 'q':
                                 * iroom = "s";
                                 * break;
                                 */

                        default:
                                srcptr--;
                                *charptr++ = *srcptr;
                                *roomptr++ = *srcptr;
                                break;
                        }
                }
                else if (*srcptr == '%' && *++srcptr == 's')
                {
                        ichar = "You";
                        iroom = IS_NPC(ch) ? ch->short_descr : ch->name;
                }
                else
                {
                        *charptr++ = *srcptr;
                        *roomptr++ = *srcptr;
                        srcptr++;
                        continue;
                }

                while ((*charptr = *ichar) != '\0')
                {
                        charptr++;
                        ichar++;
                }

                while ((*roomptr = *iroom) != '\0')
                {
                        roomptr++;
                        iroom++;
                }
                srcptr++;
        }

        *charptr = '\0';
        *roomptr = '\0';

/*
snprintf( buf, MSL, "Charbuf: %s", charbuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
snprintf( buf, MSL, "Roombuf: %s", roombuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
*/

        switch (obj->item_type)
        {
        case ITEM_BLOOD:
        case ITEM_FOUNTAIN:
                act(AT_ACTION, charbuf, ch, obj, ch, TO_CHAR);
                act(AT_ACTION, roombuf, ch, obj, ch, TO_ROOM);
                return;

        case ITEM_DRINK_CON:
                act(AT_ACTION, charbuf, ch, obj,
                    liq_table[obj->value[2]].liq_name, TO_CHAR);
                act(AT_ACTION, roombuf, ch, obj,
                    liq_table[obj->value[2]].liq_name, TO_ROOM);
                return;

        case ITEM_PIPE:
                return;

        case ITEM_ARMOR:
        case ITEM_WEAPON:
        case ITEM_LIGHT:
                act(AT_ACTION, charbuf, ch, obj, ch, TO_CHAR);
                act(AT_ACTION, roombuf, ch, obj, ch, TO_ROOM);
                return;

        case ITEM_FOOD:
        case ITEM_PILL:
                act(AT_ACTION, charbuf, ch, obj, ch, TO_CHAR);
                act(AT_ACTION, roombuf, ch, obj, ch, TO_ROOM);
                return;

        default:
                return;
        }
        return;
}

CMDF do_hail(CHAR_DATA * ch, char *argument)
{
        int       vnum, type;
        ROOM_INDEX_DATA *room = NULL;
        char      arg1[MAX_INPUT_LENGTH];
        PLANET_DATA *planet;
        AREA_DATA *area;
        int       home_vnum;

        argument = one_argument(argument, arg1);

        if (!ch->in_room)
                return;

        if (ch->position < POS_FIGHTING)
        {
                send_to_char("You might want to stop fighting first!\n\r",
                             ch);
                return;
        }

        if (ch->position < POS_STANDING)
        {
                send_to_char("You might want to stand up first!\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_INDOORS))
        {
                send_to_char("You'll have to go outside to do that!\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SPACECRAFT))
        {
                send_to_char("You can't do that on spacecraft!\n\r", ch);
                return;
        }

        if (IS_SET(ch->affected_by, AFF_RESTRAINED))
        {
                send_to_char
                        ("How do you expect to do that while restrained?\n\r",
                         ch);
                return;
        }

        if (ch->holding)
        {
                send_to_char
                        ("How do you expect to do that while holding someone?\n\r",
                         ch);
                return;
        }
        if (ch->gold < (ch->top_level - 9))
        {
                send_to_char("You don't have enough credits!\n\r", ch);
                return;
        }

        vnum = ch->in_room->vnum;

        if (arg1[0] == '\0')
        {
                type = ROOM_HOTEL;
        }
        else if (!str_cmp(arg1, "hotel"))
        {
                type = ROOM_HOTEL;
        }
        else if (!str_cmp(arg1, "bank"))
        {
                type = ROOM_BANK;
        }
        else if (!str_cmp(arg1, "factory"))
        {
                type = ROOM_FACTORY;
        }
        else if (!str_cmp(arg1, "auction"))
        {
                type = ROOM_AUCTION;
        }
        else if (!str_cmp(arg1, "home"))
        {
                if (ch->plr_home)
                        home_vnum = ch->plr_home->vnum;
                else
                {
                        send_to_char
                                ("You do not have a home to hail too.\n\r",
                                 ch);
                        return;
                }

                if ((planet = ch->in_room->area->planet) == NULL)
                {
                        send_to_char
                                ("You must be on a planet to try to hail to your home.\n\r",
                                 ch);
                        return;
                }
                for (area = planet->first_area; area;
                     area = area->next_on_planet)
                {
                        if (home_vnum >= area->low_r_vnum
                            && home_vnum <= area->hi_r_vnum)
                        {
                                ch->gold -= UMAX(ch->top_level, 0);

                                act(AT_ACTION,
                                    "$n hails a speederbike, and drives off to seek shelter.",
                                    ch, NULL, NULL, TO_ROOM);

                                char_from_room(ch);
                                char_to_room(ch, ch->plr_home);

                                send_to_char
                                        ("A speederbike picks you up and drives you to a you destination\n\rYou pay the driver some credits.\n\r\n\n",
                                         ch);
                                act(AT_ACTION, "$n $T", ch, NULL,
                                    (void *)
                                    "arrives on a speederbike, gets off and pays the driver before it leaves.",
                                    TO_ROOM);

                                do_look(ch, "auto");
                                return;
                        }
                }
                send_to_char("You do not have your home around here.\n\r\n\n",
                             ch);
                return;
        }
        else
        {
                type = ROOM_HOTEL;
        }

        for (vnum = ch->in_room->area->low_r_vnum;
             vnum <= ch->in_room->area->hi_r_vnum; vnum++)
        {
                room = get_room_index(vnum);

                if (room != NULL)
                {
                        if (xIS_SET(room->room_flags, ROOM_PLR_HOME))
                                continue;
                        else if (xIS_SET(room->room_flags, ROOM_EMPTY_HOME))
                                continue;
                        else if (xIS_SET(room->room_flags, ROOM_NO_HAIL_TO))
                                continue;
                        else if (xIS_SET(room->room_flags, type))
                                break;
                }
                /*
                 * If we didn't find anything, reset to NULL before quitting 
                 */
                room = NULL;
        }

        if (room == NULL)
        {
                send_to_char
                        ("There doesn't seem to be any taxis nearby to take you to your destination!\n\r",
                         ch);
                return;
        }

        ch->gold -= UMAX(ch->top_level, 0);

        act(AT_ACTION,
            "$n hails a speederbike, and drives off to seek shelter.", ch,
            NULL, NULL, TO_ROOM);

        char_from_room(ch);
        char_to_room(ch, room);

        send_to_char
                ("A speederbike picks you up and drives you to a you destination\n\rYou pay the driver some credits.\n\r\n\n",
                 ch);
        act(AT_ACTION, "$n $T", ch, NULL,
            (void *)
            "arrives on a speederbike, gets off and pays the driver before it leaves.",
            TO_ROOM);

        do_look(ch, "auto");
}

CMDF do_train(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        if (IS_NPC(ch))
                return;

        /*
         * Why not here too - Gavin 
         */
        /*
         * Should we check for wrist, arm, and sholder too? 
         */
        if (IS_SET(ch->bodyparts, BODY_R_HAND)
            || IS_SET(ch->bodyparts, BODY_L_HAND))
        {
                send_to_char
                        ("How do you expect to do that with your hands broken?",
                         ch);
                return;
        }

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("Train what?\n\r", ch);
                        send_to_char
                                ("\n\rChoices: strength, intelligence, wisdom, dexterity, constitution or charisma\n\r",
                                 ch);
                        return;
                }

                if (!IS_AWAKE(ch))
                {
                        send_to_char("In your dreams, or what?\n\r", ch);
                        return;
                }

                if (str_cmp(arg, "str") && str_cmp(arg, "strength")
                    && str_cmp(arg, "dex") && str_cmp(arg, "dexterity")
                    && str_cmp(arg, "con") && str_cmp(arg, "constitution")
                    && str_cmp(arg, "cha") && str_cmp(arg, "charisma")
                    && str_cmp(arg, "wis") && str_cmp(arg, "wisdom")
                    && str_cmp(arg, "int") && str_cmp(arg, "intelligence"))
                {
                        do_train(ch, "");
                        return;
                }

                if (!str_cmp(arg, "str") || !str_cmp(arg, "strength"))
                {
                        if (ch->perm_str >=
                            20 + ch->race->attr_modifier(ATTR_STRENGTH)
                            || ch->perm_str >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more strength than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char("&GYou begin your weight training.\n\r",
                                     ch);
                }
                if (!str_cmp(arg, "dex") || !str_cmp(arg, "dexterity"))
                {
                        if (ch->perm_dex >=
                            20 + ch->race->attr_modifier(ATTR_DEXTERITY)
                            || ch->perm_dex >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more dexterity than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char
                                ("&GYou begin to work at some challenging tests of coordination.\n\r",
                                 ch);
                }
                if (!str_cmp(arg, "int") || !str_cmp(arg, "intelligence"))
                {
                        if (ch->perm_int >=
                            20 + ch->race->attr_modifier(ATTR_INTELLIGENCE)
                            || ch->perm_int >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more intelligence than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char("&GYou begin your studies.\n\r", ch);
                }
                if (!str_cmp(arg, "wis") || !str_cmp(arg, "wisdom"))
                {
                        if (ch->perm_wis >=
                            20 + ch->race->attr_modifier(ATTR_WISDOM)
                            || ch->perm_wis >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more wisdom than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char
                                ("&GYou begin contemplating several ancient texts in an effort to gain wisdom.\n\r",
                                 ch);
                }
                if (!str_cmp(arg, "con") || !str_cmp(arg, "constitution"))
                {
                        if (ch->perm_con >=
                            20 + ch->race->attr_modifier(ATTR_CONSTITUTION)
                            || ch->perm_con >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more constitution than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char
                                ("&GYou begin your endurance training.\n\r",
                                 ch);
                }
                if (!str_cmp(arg, "cha") || !str_cmp(arg, "charisma"))
                {
                        if (ch->perm_cha >=
                            20 + ch->race->attr_modifier(ATTR_CHARISMA)
                            || ch->perm_cha >= 30)
                        {
                                send_to_char
                                        ("&BYou cannot gain more charisma than you already have!\n\r",
                                         ch);
                                return;
                        }
                        send_to_char
                                ("&GYou begin lessons in maners and ettiquite.\n\r",
                                 ch);
                }
                add_timer(ch, TIMER_DO_FUN, 1, do_train, 1);
                ch->dest_buf = str_dup(arg);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (const char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char("&RYou fail to complete your training.\n\r", ch);
                return;
        }

        ch->substate = SUB_NONE;

        ch->pcdata->account->rpcurrent -= 3;
        save_account(ch->pcdata->account);

        if (!str_cmp(arg, "str") || !str_cmp(arg, "strength"))
        {
                send_to_char
                        ("&GAfter much excercise you feel a little stronger.\n\r",
                         ch);
                ch->perm_str++;
                return;
        }

        if (!str_cmp(arg, "dex") || !str_cmp(arg, "dexterity"))
        {
                send_to_char
                        ("&GAfter working hard at many challenging tasks you feel a bit more coordinated.\n\r",
                         ch);
                ch->perm_dex++;
                return;
        }

        if (!str_cmp(arg, "int") || !str_cmp(arg, "intelligence"))
        {
                send_to_char
                        ("&GAfter much study you feel alot more knowledgeable.\n\r",
                         ch);
                ch->perm_int++;
                return;
        }

        if (!str_cmp(arg, "wis") || !str_cmp(arg, "wisdom"))
        {
                send_to_char
                        ("&GAfter contemplating several seemingly meaningless events you suddenly \n\rreceive a flash of insight into the workings of the universe.\n\r",
                         ch);
                ch->perm_wis++;
                return;
        }

        if (!str_cmp(arg, "con") || !str_cmp(arg, "constitution"))
        {
                send_to_char
                        ("&GAfter a long tiring excersise session you feel much healthier than before.\n\r",
                         ch);
                ch->perm_con++;
                return;
        }


        if (!str_cmp(arg, "cha") || !str_cmp(arg, "charisma"))
        {
                send_to_char
                        ("&GYou spend some time focusing on how to improve your personality and feel \n\rmuch better about yourself and the ways others see you.\n\r",
                         ch);
                ch->perm_cha++;
                return;
        }

}

CMDF do_suicide(CHAR_DATA * ch, char *argument)
{
        char      logbuf[MAX_STRING_LENGTH];

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("Yeah right!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char
                        ("&RIf you really want to delete this character type suicide and your password.\n\r",
                         ch);
                return;
        }

        if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
        {
                send_to_char("Sorry wrong password.\n\r", ch);
                snprintf(logbuf, MSL,
                         "%s attempting to commit suicide... WRONG PASSWORD!",
                         ch->name);
                log_string(logbuf);
                return;
        }

        if (!str_cmp(ch->race->name(), "droid"))
        {
                act(AT_BLOOD,
                    "You start your self destruct program, and your power source ignites and fuses all your circuits!",
                    ch, NULL, NULL, TO_CHAR);
                act(AT_BLOOD,
                    "You watch as $n starts to tremble, and they fall over, powerless and smoking!",
                    ch, NULL, NULL, TO_ROOM);
        }

        else
        {
                act(AT_BLOOD,
                    "With a sad determination and trembling hands you slit your own throat!",
                    ch, NULL, NULL, TO_CHAR);
                act(AT_BLOOD,
                    "Cold shivers run down your spine as you watch $n slit $s own throat!",
                    ch, NULL, NULL, TO_ROOM);
        }
        raw_kill(ch, ch);
        snprintf(logbuf, MSL, "%s has committed suicide!", ch->name);
        log_string(logbuf);

}

CMDF do_bank(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        long      amount = 0;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BANK))
        {
                send_to_char("You must be in a bank to do that!\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Usage: BANK <withdraw|balance> [amount]\n\r", ch);
                return;
        }

        if (arg2[0] != '\0')
                amount = atoi(arg2);

        if (!str_prefix(arg1, "deposit"))
        {
                send_to_char("&RYou may not deposit.\n\r", ch);
                return;
                if (amount <= 0)
                {
                        send_to_char
                                ("You may only deposit amounts greater than zero.\n\r",
                                 ch);
                        do_bank(ch, "");
                        return;
                }

                if (ch->gold < amount)
                {
                        send_to_char
                                ("You don't have that many credits on you.\n\r",
                                 ch);
                        return;
                }

                ch->gold -= amount;
                ch->pcdata->bank += amount;

                ch_printf(ch,
                          "You deposit %ld credits into your account.\n\r",
                          amount);
                return;
        }
        else if (!str_prefix(arg1, "withdraw"))
        {
                if (amount <= 0)
                {
                        send_to_char
                                ("You may only withdraw amounts greater than zero.\n\r",
                                 ch);
                        do_bank(ch, "");
                        return;
                }

                if (ch->pcdata->bank < amount)
                {
                        send_to_char
                                ("You don't have that many credits in your account.\n\r",
                                 ch);
                        return;
                }

                ch->gold += amount;
                ch->pcdata->bank -= amount;

                ch_printf(ch,
                          "You withdraw %ld credits from your account.\n\r",
                          amount);
                return;

        }
        else if (!str_prefix(arg1, "balance"))
        {
                ch_printf(ch, "You have %ld credits in your account.\n\r",
                          ch->pcdata->bank);
                return;
        }
        else
        {
                do_bank(ch, "");
                return;
        }


}

/*
CMDF do_invest( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    long amount = 0;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    vnum = 10437;
    
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }


    
    argument = one_argument( argument , arg1 );
    
    if ( IS_NPC(ch) || !ch->pcdata )
       return;
    
    if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "You must be in a bank to do that!\n\r", ch );
       return;
    }

    if ( arg1[0] == '\0' )
    {
       send_to_char( "Usage: INVEST <amount> [amount]\n\r", ch );
       return;
    }

        amount = atoi(arg1);

       if ( ch->gold < amount )
       {
          send_to_char( "You don't have that many credits on you.\n\r", ch );
          return;
       }
       
       ch->gold -= amount;
       

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
		ch->gold += amount;
    }

    obj = create_object( pObjIndex, level );
    
    obj->item_type = ITEM_BOND;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = 1;
    obj->weight = 1;
    STRFREE( obj->name );
    mudstrlcat( buf, " bond" ,MSL);
    obj->name = STRALLOC( buf );
    STRFREE( obj->short_descr );
    obj->short_descr = "A bond";        
    STRFREE( obj->description );
    mudstrlcat( buf, " was left here." ,MSL);
    obj->description = STRALLOC( buf );
    obj->value[0] = INIT_WEAPON_CONDITION;      
    obj->value[1] = (int) (level/20+10);      
    obj->value[2] = (int) (level/10+20);      
    obj->value[3] = WEAPON_VIBRO_BLADE;
    obj->value[4] = charge;
    obj->value[5] = charge;
    obj->cost = obj->value[2]*10;
                                                                    
    obj = obj_to_char( obj, ch );


       ch_printf( ch , "You purchase a bond for %ld credits.\n\r" ,amount );
       return;
    

    
        
}
*/

CMDF do_junk(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj, *obj_next;
        char      arg[MAX_INPUT_LENGTH];
        bool      found = FALSE;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Junk what?\n\r", ch);
                return;
        }

        for (obj = ch->first_carrying; obj; obj = obj_next)
        {
                obj_next = obj->next_content;
                if ((nifty_is_name(arg, obj->name)) && can_see_obj(ch, obj)
                    && obj->wear_loc == WEAR_NONE)
                {
                        found = TRUE;
                        break;
                }
        }
        if (found)
        {
                if (!can_drop_obj(ch, obj) && ch->top_level < LEVEL_IMMORTAL)
                {
                        send_to_char("You cannot junk that, it's cursed!\n\r",
                                     ch);
                        return;
                }
                separate_obj(obj);
                obj_from_char(obj);
                extract_obj(obj);
                act(AT_ACTION, "$n junks $p.", ch, obj, NULL, TO_ROOM);
                act(AT_ACTION, "You junk $p.", ch, obj, NULL, TO_CHAR);
        }
        else

                send_to_char
                        ("You don't seem to have that object in your inventory.\n\r",
                         ch);
        return;
}

char     *clean_message(char *str)
{
        static char cleanstring[MSL];
        int       x = 0;

        mudstrlcpy(cleanstring, "", MIL);

        for (;;)
        {
                if (*str == '\0')
                {
                        cleanstring[x] = *str;
                        return &cleanstring[0];
                }
                if (*str != '\r')
                {
                        cleanstring[x] = *str;
                        str++;
                        x++;
                }
                else
                        str++;
        }
        return NULL;
}

int check_email_syntax(char *arg1, char *arg2)
{
        int       x;

        for (x = 0;; x++)
        {
                if (arg1[x] == 34 || arg1[x] == 39)
                        return FALSE;
                if (arg1[x] == '\0')
                        break;
        }
        for (x = 0;; x++)
        {
                if (arg2[x] == 34 || arg2[x] == 39)
                        return FALSE;
                if (arg2[x] == '\0')
                        break;
        }
        return TRUE;
}

void send_email(char *subject, char *email, char *message, CHAR_DATA * ch)
{
        static char sendstring[1000];
        FILE     *fp;
        FILE     *mfp;

        mudstrlcpy(sendstring, "", 1000);

        message = clean_message(message);
        fp = fopen(EMAIL_FILE, "w");
        fprintf(fp, "%s", message);
        fprintf(fp, "\n\n---\n");
        fprintf(fp, "%s\n", ch ? ch->name : "Automated Email");
        fprintf(fp, "Dark Warriors Email System \n");
        fprintf(fp, "telnet://darkwars.wolfpaw.net:4848 \n");
        fprintf(fp, "http://darkwars.wolfpaw.net \n");
        FCLOSE(fp);

        snprintf(sendstring, MSL, "%s -s \"%s: %s\" \"%s\" < %s",
                 sysdata.mail_path, ch ? ch->name : "Automated Email",
                 subject, email, EMAIL_FILE);
        if ((mfp = popen(sendstring, "w")) == NULL)
        {
                if (ch)
                        send_to_char
                                ("The message was not sent because the mail program could not be found.\n\r",
                                 ch);
                bug("send_email:  Could not location mail.");
                return;
        }
        pclose(mfp);
        remove(EMAIL_FILE);
        if (ch)
                ch_printf(ch, "Your email has been sent to %s\n\r", email);
}

CMDF do_sendmail(CHAR_DATA * ch, char *argument)
{
        char      arg1[MSL];
        char      passargument[MSL];

        if (IS_NPC(ch))
        {
                send_to_char("Monsters are too dumb to do that!\n\r", ch);
                return;
        }

        if (!ch->desc)
        {
                bug("do_description: no descriptor", 0);
                return;
        }

        if (ch->dest_buf)
        {
                argument = (char *) ch->dest_buf;
        }

        mudstrlcpy(passargument, argument, MSL);

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        if (argument[0] == '\0' || arg1[0] == '\0')
        {
                send_to_char
                        ("Syntax:  sendmail <recepient's email address> <subject>\n\r",
                         ch);
                send_to_char
                        ("Once that is typed, you will be sent into the buffer.\n\r",
                         ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char("You need to supply a subject.\n\r", ch);
                return;
        }

        /*
         * WTF ARE THESE FOR 
         */
        if (strlen(argument) > 95)
        {
                send_to_char
                        ("Subject cannot be longer than 95 characters.\n\r",
                         ch);
                return;
        }

        if (strlen(arg1) > 95)
        {
                send_to_char
                        ("recepient's email address cannot be longer than 95 characters.\n\r",
                         ch);
                return;
        }

        /*
         * WTF IS THIS? - Gavin 
         */
        if (!check_email_syntax(argument, arg1))
        {
                send_to_char
                        ("You cannot supply any \" or \' in your subject or recepient.\n\r",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                bug("do_sendmail: illegal substate", 0);
                return;
        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot use this command from within another command.\n\r",
                         ch);
                return;

        case SUB_NONE:
                if (ch->pcdata->sendmail)
                        STRFREE(ch->pcdata->sendmail);
                ch->pcdata->sendmail = STRALLOC("");
                ch->substate = SUB_WRITING_EMAIL;
                ch->dest_buf = str_dup(passargument);
                start_editing(ch, ch->pcdata->sendmail);
                return;

        case SUB_WRITING_EMAIL:
                STRFREE(ch->pcdata->sendmail);
                if (ch->dest_buf)
                        DISPOSE(ch->dest_buf);
                ch->pcdata->sendmail = copy_buffer(ch);
                send_email(argument, arg1, ch->pcdata->sendmail, ch);
                bug("-------------%s is sending an email to %s-------------\n\r", ch->name, arg1);
                stop_editing(ch);
                return;
        }
}
