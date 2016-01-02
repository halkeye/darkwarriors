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
 *                $Id: msp.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <string.h>
#include <arpa/telnet.h>
#include "mud.h"
#include "msp.h"
#include "account.h"

/* Good Example:
 * do_beep
 *
 * sound_to_char(victim, "beep");
 */

/* Mud Sound Protocol */
const unsigned char will_msp_str[] = { IAC, WILL, TELOPT_MSP, '\0' };
const unsigned char start_msp_str[] = { IAC, SB, TELOPT_MSP, IAC, SE, '\0' };

void start_msp(DESCRIPTOR_DATA * d)
{
/*	d->msp = TRUE; */
        write_to_buffer(d, (const char *) start_msp_str, 0);
}

CMDF do_testsound(CHAR_DATA * ch, char *argument)
{
        char      arg[MSL];

        if (argument[0] == '\0')
                return;
        one_argument(argument, arg);
        sound_to_char(ch, arg);
}

void reset_sound(CHAR_DATA * ch)
{
#ifdef ACCOUNT
        if (ch->pcdata->account
            && !IS_SET(ch->pcdata->account->flags, ACCOUNT_SOUND))
                return;
#endif
        if (!IS_SET(ch->act, PLR_SOUND))
                return;
        send_to_char("!!SOUND(Off)\n\r", ch);
}

void reset_music(CHAR_DATA * ch)
{
#ifdef ACCOUNT
        if (ch->pcdata->account
            && !IS_SET(ch->pcdata->account->flags, ACCOUNT_SOUND))
                return;
#endif
        if (!IS_SET(ch->act, PLR_SOUND))
                return;
        send_to_char("!!MUSIC(Off)\n\r", ch);
}

void sound_to_room(ROOM_INDEX_DATA * room, char *sound)
{
        CHAR_DATA *vch;

        if (room == NULL)
                return;

        for (vch = room->first_person; vch; vch = vch->next_in_room)
                sound_to_char(vch, sound);
}

void sound_to_ship(SHIP_DATA * ship, char *argument)
{
        int       roomnum;
        ROOM_INDEX_DATA *room;

        for (roomnum = ship->firstroom; roomnum <= ship->lastroom; roomnum++)
        {
                room = get_room_index(roomnum);
                if (room == NULL)
                        continue;
                sound_to_room(room, argument);
        }

}

void sound_to_char(CHAR_DATA * ch, char *sound)
{
        char     *type = "";
        char      url[255];

        /*
         * Bit checking is easier than sprintf, so do that first 
         */
        if (IS_NPC(ch))
                return;
#ifdef ACCOUNT
        if (ch->pcdata->account
            && !IS_SET(ch->pcdata->account->flags, ACCOUNT_SOUND))
                return;
#endif
        if (!IS_SET(ch->act, PLR_SOUND))
                return;
        snprintf(url, 255, "%sdownloads/sounds/", sysdata.mud_url);

        ch_printf(ch, "!!SOUND(%s.wav R=1 P=50 T=%s U=%s)", sound, type, url);
        return;
}

CMDF do_sound(CHAR_DATA * ch, char *argument)
{
        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                bool      sound = FALSE;

#ifdef ACCOUNT
                if (ch->pcdata->account)
                {
                        if (IS_SET(ch->pcdata->account->flags, ACCOUNT_SOUND))
                                sound = TRUE;
                }
                else
#endif
                if (IS_SET(ch->act, PLR_SOUND))
                        sound = TRUE;
                ch_printf(ch, "&BS&zound: &B[&w%s&B]\n\r",
                          sound ? "On " : "Off");
                return;
        }

        if (!str_cmp(argument, "on"))
        {
#ifdef ACCOUNT
                if (ch->pcdata->account)
                        SET_BIT(ch->pcdata->account->flags, ACCOUNT_SOUND);
                else
#endif
                        SET_BIT(ch->act, PLR_SOUND);
                set_char_color(AT_WHITE + AT_BLINK, ch);
                send_to_char("SOUND ON!!!\n\r", ch);
                sound_to_char(ch, "hopeknow");
                return;
        }
        else if (!str_cmp(argument, "off"))
        {
#ifdef ACCOUNT
                if (ch->pcdata->account)
                        REMOVE_BIT(ch->pcdata->account->flags, ACCOUNT_SOUND);
#endif
                REMOVE_BIT(ch->act, PLR_SOUND);
                send_to_char("Okay... SOUND support is now off\n\r", ch);
                return;
        }
        else if (!str_cmp(argument, "auto"))
        {
                if (!ch->desc)
                        return;
                if (ch->desc->msp_detected)
                        do_sound(ch, "on");
                else
                        do_sound(ch, "off");
                return;
        }
        else if (!str_cmp(argument, "toggle"))
        {
#ifdef ACCOUNT
                if (ch->pcdata->account)
                        TOGGLE_BIT(ch->pcdata->account->flags, ACCOUNT_SOUND);
                else
#endif
                        TOGGLE_BIT(ch->act, PLR_SOUND);
                send_to_char("Okay... SOUND support is now toggled\n\r", ch);
                return;
        }
}
