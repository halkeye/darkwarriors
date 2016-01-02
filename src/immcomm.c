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
 *                $Id: immcomm.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"
#include "homes.h"
#include "changes.h"
#include "body.h"
#include "space2.h"

void list_resets args((CHAR_DATA * ch, AREA_DATA * pArea,
                       ROOM_INDEX_DATA * pRoom, int start, int end));
void save_sysdata args((SYSTEM_DATA sys));
void save_banlist args((void));


CMDF do_fakequit(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_INPUT_LENGTH];

        argument = NULL;
        set_char_color(AT_WHITE, ch);
        send_to_char
                ("You have quit, how funny\n\rYou left the game, well not completly\n\r",
                 ch);
        act(AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM);
        set_char_color(AT_GREY, ch);
        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        snprintf(buf, MSL, "%s has left %s", ch->name, sysdata.mud_name);
        info_chan(buf);


        if (!IS_SET(ch->act, PLR_WIZINVIS))
        {
                SET_BIT(ch->act, PLR_WIZINVIS);
        }


}
CMDF do_fakeenter(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_INPUT_LENGTH];

        argument = NULL;
        set_char_color(AT_WHITE, ch);
        act(AT_WHITE, "$n has entered the game.", ch, NULL, NULL, TO_ROOM);
        set_char_color(AT_GREY, ch);
        snprintf(buf, MSL, "%s has entered %s", ch->name, sysdata.mud_name);
        info_chan(buf);


        if (IS_SET(ch->act, PLR_WIZINVIS))
        {
                REMOVE_BIT(ch->act, PLR_WIZINVIS);
        }


}

CMDF do_lagout(CHAR_DATA * ch, char *argument)
{

        CHAR_DATA *victim;
        char      arg1[MAX_STRING_LENGTH];
        int       x;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: Lagout <victim> <amount>\n\r", ch);
                return;
        }

        if (!str_cmp("badhash", argument))
        {
                char     *hashstr = STRALLOC("Hash test");
                char     *nohashstr = str_dup("Non-hashed test");

                DISPOSE(hashstr);
                STRFREE(nohashstr);
                return;
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

        if ((x = atoi(argument)) <= 0)
        {
                send_to_char("So your giving them 0 lag?\n\r", ch);
                return;
        }

        if (x > 1000)
        {
                send_to_char("That # is too high.\n\r", ch);
                return;
        }

        send_to_char("Your getting some lag right about now...\n\r", victim);
        WAIT_STATE(victim, x);
        send_to_char("Adding lag...\n\r", ch);
        return;
}


CMDF do_rseek(CHAR_DATA * ch, char *argument)
{
        RESET_DATA *pReset;
        AREA_DATA *pArea;
        int       x, counter = 1;

        if (!is_number(argument))
        {
                ch_printf(ch,
                          "%s is not a number, and only vnums can be searched for in resets.\n\r",
                          argument);
                return;
        }
        x = atoi(argument);
        for (pArea = first_area; pArea; pArea = pArea->next)
        {
                for (pReset = pArea->first_reset; pReset;
                     pReset = pReset->next)
                {
                        if (pReset->arg1 == x || pReset->arg2 == x
                            || pReset->arg3 == x)
                        {
                                pager_printf(ch,
                                             "[%-6d] %-20s Reset Number: %d\n\r",
                                             x, pArea->filename, counter);
                                list_resets(ch, pArea, NULL, counter,
                                            counter);
                                send_to_char("\n\r", ch);
                        }
                        counter++;
                }
                counter = 1;
        }
        return;
}

CMDF do_mudsave(CHAR_DATA * ch, char *argument)
{

        CHAR_DATA *wch;
        SHIP_DATA *ship;
        SPACE_DATA *starsystem;
        CLAN_DATA *clan;
        PLANET_DATA *planet;
        char      arg[MAX_INPUT_LENGTH];
        AREA_DATA *tarea;
        BODY_DATA *body = NULL;

        if (IS_NPC(ch))
                return;

        one_argument(argument, arg);

        send_to_char("Saving Socials..................Done.\n\r", ch);
        save_socials();
        send_to_char("Saving Skill Table..............Done.\n\r", ch);
        save_skill_table();
        send_to_char("Saving Ban List.................Done.\n\r", ch);
        save_banlist();
        send_to_char("Saving System Data..............Done.\n\r", ch);
        save_sysdata(sysdata);
        send_to_char("Saving Command Table............Done.\n\r", ch);
        save_commands();
        send_to_char("Saving Changes..................Done.\n\r", ch);
        save_changes();
        send_to_char("Saving Fingers..................Done.\n\r", ch);
        send_to_char("Saving Homes....................Done.\n\r", ch);
        for (HOME_DATA * home = first_home; home; home = home->next)
                home->save();

        for (wch = first_char; wch; wch = wch->next)
        {
                do_save(wch, "");
                save_finger(wch);
                save_home(wch);
        }

        send_to_char("Saving Clans....................Done.\n\r", ch);
        for (clan = first_clan; clan; clan = clan->next)
        {
                save_clan(clan);
        }

        send_to_char("Saving Planets..................Done.\n\r", ch);
        for (planet = first_planet; planet; planet = planet->next)
        {
                save_planet(planet, FALSE);
        }

        send_to_char("Saving Ships....................Done.\n\r", ch);
        for (ship = first_ship; ship; ship = ship->next)
        {
                save_ship(ship);
        }
        send_to_char("Saving Star System..............Done.\n\r", ch);
        for (starsystem = first_starsystem; starsystem;
             starsystem = starsystem->next)
        {
                save_starsystem(starsystem);
        }
        send_to_char("Saving Body.....................Done.\n\r", ch);
        FOR_EACH_LIST(BODY_LIST, bodies, body) body->save();
        send_to_char("Done.\n\r", ch);
#ifdef OLC_HOMES
        send_to_char("Saving Homes.....................", ch);
        {
                HOME_DATA *home = NULL;

                for (home = first_home; home; home = home->next)
                        home->save();
        }
        send_to_char("Done.\n\r", ch);
#endif
        if (!str_cmp(arg, "areas"))
        {
                send_to_char("Saving Area Files...............Done.\n\r", ch);
                for (tarea = first_area; tarea; tarea = tarea->next)
                        fold_area(tarea, tarea->filename, FALSE, FALSE);
                for (tarea = first_bsort; tarea; tarea = tarea->next_sort)
                        if (IS_SET(tarea->status, AREA_LOADED))
                                fold_area(tarea, tarea->filename, FALSE,
                                          TRUE);

        }
        send_to_char("&W&wMud save completed.", ch);
        return;

}

CMDF do_working(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;
        if (IS_NPC(ch))
                return;


        if (IS_SET(ch->pcdata->flags, PCFLAG_WORKING))
        {
                REMOVE_BIT(ch->pcdata->flags, PCFLAG_WORKING);
                send_to_char("You relax after working so hard!&R&W\n\r", ch);
                if (!IS_SET(ch->act, PLR_WIZINVIS))
                {
                        snprintf(buf, MSL, "%s is no longer working.",
                                 ch->name);
                        info_chan(buf);
                }
        }
        else
        {
                SET_BIT(ch->pcdata->flags, PCFLAG_WORKING);
                send_to_char
                        ("You get ready to start working really hard!\n\r",
                         ch);
                if (!IS_SET(ch->act, PLR_WIZINVIS))
                {
                        snprintf(buf, MSL, "%s is now working.", ch->name);
                        info_chan(buf);
                }
        }

        return;
}
