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
 *                $Id: space2.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "body.h"
#include <algorithm>
#include "space2.h"
#include "installations.h"

void write_starsystem_list args((void));

int get_rflag args((char *flag));
int get_vnum_range args((int range));
AREA_DATA *create_auto_area
args((int low_vnum, int high_vnum, BODY_DATA * body));

/*
 * Remove carriage returns from a line
 */
char     *strip_cr(char *str)
{
        static char newstr[MAX_STRING_LENGTH];
        int       i, j;

        for (i = j = 0; str[i] != '\0'; i++)
                if (str[i] != '\r')
                {
                        newstr[j++] = str[i];
                }
        newstr[j] = '\0';
        return newstr;
}

DOCK_DATA *dock_from_room(int vnum)
{
        DOCK_DATA *dock;

        for (dock = first_dock; dock; dock = dock->next)
                if (vnum == dock->vnum)
                        return dock;
        return NULL;
}

DOCK_DATA *get_dock(char *name)
{
        DOCK_DATA *dock;

        for (dock = first_dock; dock; dock = dock->next)
                if (!str_cmp(name, dock->name))
                        return dock;
        for (dock = first_dock; dock; dock = dock->next)
                if (nifty_is_name_prefix(name, dock->name))
                        return dock;
        return NULL;
}

DOCK_DATA *get_dock_isname(SHIP_DATA * ship, char *name)
{
        DOCK_DATA *dock = NULL;
        BODY_DATA *body = NULL;

        if (name == NULL)
        {
                bug("get_dock_isname: name: null");
                return NULL;
        }

        if (!ship->starsystem)
                return NULL;

        FOR_EACH_LIST(BODY_LIST, ship->starsystem->bodies, body)
        {
                FOR_EACH_LIST(DOCK_LIST, body->docks(), dock)
                        if (!str_cmp(name, dock->name))
                        return dock;
        }

        FOR_EACH_LIST(BODY_LIST, ship->starsystem->bodies, body)
        {
                FOR_EACH_LIST(DOCK_LIST, body->docks(), dock)
                        if (nifty_is_name_prefix(name, dock->name))
                        return dock;
        }
        return NULL;
}

char     *get_direction_ship(SHIP_DATA * target, SHIP_DATA * ship)
{
        static char buf[11];

        buf[0] = ' ';
        if (target->vx > ship->vx)
                buf[1] = 'E';
        else if (target->vx < ship->vx)
                buf[1] = 'W';
        else
                buf[1] = ' ';
        buf[2] = ' ';
        buf[3] = ' ';
        if (target->vy > ship->vy)
                buf[4] = 'N';
        else if (target->vy < ship->vy)
                buf[4] = 'S';
        else
                buf[4] = ' ';
        buf[5] = ' ';
        buf[6] = ' ';
        if (target->vz > ship->vz)
                buf[7] = 'U';
        else if (target->vz < ship->vz)
                buf[7] = 'D';
        else
                buf[7] = ' ';
        buf[8] = ' ';
        buf[9] = '\0';
        return buf;
}

int distance_ship_ship(SHIP_DATA * target, SHIP_DATA * ship)
{
        return (int) sqrt(pow(((int) (ship->vx - target->vx)), 2) +
                          pow(((int) (ship->vy - target->vy)),
                              2) + pow(((int) (ship->vz - target->vz)), 2));
}

int distance_missile_ship(MISSILE_DATA * missile, SHIP_DATA * ship)
{
        return (int) sqrt(pow(((int) (ship->vx - missile->mx)), 2) +
                          pow(((int) (ship->vy - missile->my)),
                              2) + pow(((int) (ship->vz - missile->mz)), 2));
}

void write_dock_list()
{
        bug("Still calling write_dock_list");
        return;
#if 0
        DOCK_DATA *tdock;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, 256, "%s%s", DOCK_DIR, DOCK_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open dock.lst for writing!\n\r", 0);
                return;
        }
        for (tdock = first_dock; tdock; tdock = tdock->next)
                fprintf(fpout, "%s\n", tdock->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
#endif
}

void fwrite_dock(FILE * fp, DOCK_DATA * dock)
{
        if (!dock || !fp)
        {
                bug("save_dock: null dock pointer!", 0);
                return;
        }

        fprintf(fp, "#DOCK\n");
        fprintf(fp, "Name         %s~\n", dock->name);
        fprintf(fp, "Vnum         %d\n", dock->vnum);
        fprintf(fp, "Hidden       %d\n", dock->hidden);
        if (dock->clan && dock->clan->name)
                fprintf(fp, "Clan   %s~\n", dock->clan->name);
        fprintf(fp, "End\n\n");
        return;
}

void fread_dock(DOCK_DATA * dock, FILE * fp)
{
        const char *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'B':
                        KEY("Body", dock->body,
                            get_body(fread_string_noalloc(fp)));
                        break;

                case 'C':
                        KEY("Clan", dock->clan,
                            get_clan(fread_string_noalloc(fp)));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!dock->name)
                                        dock->name = STRALLOC("Unnamed Dock");
                                return;
                        }
                        break;

                case 'H':
                        KEY("Hidden", dock->hidden, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", dock->name, fread_string(fp));
                        break;

                case 'V':
                        KEY("Vnum", dock->vnum, fread_number(fp));
                        break;

                }

                if (!fMatch)
                        bug("Fread_dock: no match: %s", word);

        }
}

bool load_dock_file(char *dockfile)
{
        char      filename[256];
        DOCK_DATA *dock;
        FILE     *fp;
        bool      found;

        CREATE(dock, DOCK_DATA, 1);

        found = FALSE;
        snprintf(filename, 256, "%s%s", DOCK_DIR, dockfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_dock_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "DOCK"))
                        {
                                fread_dock(dock, fp);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_dock_file: bad section: %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (!found)
                /*
                 * FIXME - free_dock 
                 */
                DISPOSE(dock);
        else
        {
                if (dock->body == NULL)
                {
                        bug("Dock '%s''s body doesn't list", dock->name);
                        /*
                         * FIXME - free_dock 
                         */
                        DISPOSE(dock);
                        return FALSE;
                }
                LINK(dock, first_dock, last_dock, next, prev);
                dock->body->add_dock(dock);
                remove(dockfile);   /* To prevent Double Loading ? */
        }

        return found;
}

void load_docks()
{
        FILE     *fpList;
        const char *filename;
        char      docklist[256];

        snprintf(docklist, 256, "%s%s", DOCK_DIR, FILE_DOCK_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(docklist, "r")) == NULL)
        {
                fpReserve = fopen(NULL_FILE, "r");
                perror(docklist);
                return;
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_dock_file((char *) filename))
                {
                        bug("Cannot load dock file: %s", filename);
                }
        }
        remove(docklist);   /* To prevent Double Loading ? */
        FCLOSE(fpList);
        boot_log(" Done docks ");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_makedock(CHAR_DATA * ch, char *argument)
{
        char      arg[MIL];
        DOCK_DATA *dock;
        BODY_DATA *body = NULL;

        argument = one_argument(argument, arg);
        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Usage: makedock <body> <name>\n\r", ch);
                return;
        }

        if ((body = get_body(arg)) == NULL)
        {
                send_to_char("That body doesn't exist.", ch);
                return;
        }

        CREATE(dock, DOCK_DATA, 1);
        dock->body = body;
        body->add_dock(dock);
        dock->name = STRALLOC(argument);
        dock->hidden = 0;
        LINK(dock, first_dock, last_dock, next, prev);
        body->save();
        ch_printf(ch,
                  "Dock creation complete. New dock created under name, \"%s\"",
                  dock->name);
}

void makedock(INSTALLATION_DATA * installation)
{
        char      buf[MSL];
        DOCK_DATA *dock;


        snprintf(buf, MSL, "Ship Yard %d", installation->first_room);
        CREATE(dock, DOCK_DATA, 1);
        dock->clan = installation->clan;
        dock->body = installation->planet->body;
        dock->body->add_dock(dock);
        dock->name = STRALLOC(buf);
        dock->vnum = installation->mainroom;
        dock->temporary = 1;
        LINK(dock, first_dock, last_dock, next, prev);
}

CMDF do_makebody(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        BODY_DATA *body;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makebody <name>\n\r", ch);
                return;
        }

        snprintf(filename, 256, "%s.%s", strlower(argument),
                 "body");

        NEW(body, BODY_DATA);
        bodies.push_back(body);
        body->name(argument);
        body->filename(smash_space(filename));
        while (body->xmove() > -10 && body->xmove() < 10)
                body->xmove(number_range(-50, 50));
        while (body->ymove() > -10 && body->ymove() < 10)
                body->ymove(number_range(-50, 50));
        while (body->zmove() > -10 && body->zmove() < 10)
                body->zmove(number_range(-50, 50));
        body->save();
        write_body_list();
        ch_printf(ch,
                  "Body creation complete. New body created under name, \"%s\"",
                  body->name());
}

CMDF do_setdock(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        DOCK_DATA *dock;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: setdock <dock> <field> [value]\n\r", ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char(" clan filename vnum hidden body\n\r", ch);
                send_to_char(" name starsystem\n\r", ch);
                return;
        }

        dock = get_dock(arg1);
        if (!dock)
        {
                send_to_char("No such dock.\n\r", ch);
                return;
        }


        if (!strcmp(arg2, "name"))
        {
                STRFREE(dock->name);
                dock->name = STRALLOC(arg3);
                send_to_char("Done.\n\r", ch);
                dock->body->save();
                return;
        }

        if (!strcmp(arg2, "body"))
        {
                BODY_DATA *body = get_body(arg3);

                if (body)
                {
                        send_to_char("No such celestial body.\n\r", ch);
                        return;
                }
                if (dock->body)
                        dock->body->remove_dock(dock);
                dock->body = body;
                dock->body->add_dock(dock);
                send_to_char("Done.\n\r", ch);
                dock->body->save();
                return;
        }

        if (!strcmp(arg2, "clan"))
        {
                CLAN_DATA *clan;

                clan = get_clan(arg3);
                if (clan)
                {
                        dock->clan = clan;
                        send_to_char("Done.\n\r", ch);
                        dock->body->save();
                }
                else
                        send_to_char("No such clan.\n\r", ch);
                return;
        }

        if (!strcmp(arg2, "hidden"))
        {
                if (str_cmp(arg3, "Yes") && str_cmp(arg3, "No"))
                {
                        send_to_char("&CChoices&R:&W Yes and No.\n\r", ch);
                        return;
                }
                if (!str_cmp(arg3, "Yes"))
                        dock->hidden = 1;
                if (!str_cmp(arg3, "No"))
                        dock->hidden = 0;
                send_to_char("Done.\n\r", ch);
                dock->body->save();
                return;
        }


        if (!strcmp(arg2, "vnum"))
        {
                if (get_room_index(atoi(arg3)))
                {
                        dock->vnum = atoi(arg3);
                        send_to_char("done.\n\r", ch);
                        dock->body->save();
                        return;
                }
                else
                {
                        send_to_char("No such Vnum.\n\r", ch);
                        return;
                }

        }

        do_setdock(ch, "");
        return;
}


CMDF do_setbody(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        BODY_DATA *body;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        one_argument(argument, arg3);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: setbody <body> <field> [value]\n\r", ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char(" gravity xpos ypos zpos type\n\r", ch);
                send_to_char(" planet starsystem name filename\n\r", ch);
                return;
        }

        body = get_body(arg1);
        if (!body)
        {
                send_to_char("No such body.\n\r", ch);
                return;
        }


        if (!body->filename())
        {
                send_to_char("This body can not be edited.\n\r", ch);
                return;
        }

        if (!strcmp(arg2, "name"))
        {
                body->name(capitalize(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "planet"))
        {
                PLANET_DATA *planet;

                planet = get_planet(arg3);
                if (planet)
                {
                        body->planet(planet);
                        send_to_char("Done.\n\r", ch);
                        body->save();
                }
                else
                        send_to_char("No such planet.\n\r", ch);
                return;
        }

        if (!strcmp(arg2, "starsystem"))
        {
                SPACE_DATA *starsystem;

                if ((starsystem = starsystem_from_name(arg3)))
                {
                        body->starsystem(starsystem);
                        send_to_char("Done.\n\r", ch);
                }
                else
                        send_to_char("No such starsystem.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "filename"))
        {
                body->filename(argument);
                send_to_char("Done.\n\r", ch);
                body->save();
                write_body_list();
                return;
        }

        if (!str_cmp(arg2, "gravity"))
        {
                body->gravity(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!str_cmp(arg2, "xpos"))
        {
                body->xpos(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!str_cmp(arg2, "ypos"))
        {
                body->ypos(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "zpos"))
        {
                body->zpos(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "xmove"))
        {
                body->xmove(atoi(arg3));
                if (body->xmove() < 5)
                {
                        send_to_char
                                ("&RWarning: &YSetting movement at less than 5 MAY cause\n\r",
                                 ch);
                        send_to_char
                                ("&R         &Ybodies to pass too close to sun.\n\r",
                                 ch);
                }
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "ymove"))
        {
                body->ymove(atoi(arg3));
                if (body->ymove() < 5)
                {
                        send_to_char
                                ("&RWarning: &YSetting movement at less than 5 MAY cause\n\r",
                                 ch);
                        send_to_char
                                ("&R         &Ybodies to pass too close to sun.\n\r",
                                 ch);
                }
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "zmove"))
        {
                body->zmove(atoi(arg3));
                if (body->zmove() < 5)
                {
                        send_to_char
                                ("&RWarning: &YSetting movement at less than 5 MAY cause\n\r",
                                 ch);
                        send_to_char
                                ("&R         &Ybodies to pass too close to sun.\n\r",
                                 ch);
                }
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "centerx"))
        {
                body->centerx(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "centery"))
        {
                body->centery(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "centerz"))
        {
                body->centerz(atoi(arg3));
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        if (!strcmp(arg2, "delete"))
        {
                bodies.erase(find(bodies.begin(), bodies.end(), body));
                DELETE(body);
                write_body_list();
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "type"))
        {
                if (!str_cmp(arg3, "star"))
                        body->type(STAR_BODY);
                else if (!str_cmp(arg3, "planet"))
                        body->type(PLANET_BODY);
                else if (!str_cmp(arg3, "moon"))
                        body->type(MOON_BODY);
                else if (!str_cmp(arg3, "comet"))
                        body->type(COMET_BODY);
                else if (!str_cmp(arg3, "asteroid"))
                        body->type(ASTEROID_BODY);
                else if (!str_cmp(arg3, "blackhole"))
                        body->type(BLACKHOLE_BODY);
                else if (!str_cmp(arg3, "nebula"))
                        body->type(NEBULA_BODY);
                else
                {
                        send_to_char
                                ("Body type must be one of: star, planet, moon, comet, asteroid, blackhole, nebula.\n\r",
                                 ch);
                        return;
                }
                send_to_char("Done.\n\r", ch);
                body->save();
                return;
        }

        do_setbody(ch, "");
        return;
}

CMDF do_showdock(CHAR_DATA * ch, char *argument)
{
        DOCK_DATA *dock;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showdock <dock>\n\r", ch);
                return;
        }

        dock = get_dock(argument);
        if (!dock)
        {
                send_to_char("No such dock.\n\r", ch);
                return;
        }


        ch_printf(ch, "&W%s\n\r", dock->name);
        if (dock->body->starsystem() && dock->body->starsystem()->name)
                ch_printf(ch, "&WStar system:&G%s\n\r",
                          dock->body->starsystem()->name);
        if (dock->clan && dock->clan->name)
                ch_printf(ch, "&WClan:&G%s\n\r", dock->clan->name);
        if (dock->body && dock->body->name())
                ch_printf(ch, "&WBody:&G%s\n\r", dock->body->name());
        ch_printf(ch, "&Wvnum:&G%d\n\r", dock->vnum);
        ch_printf(ch, "&WHidden:&G%s\n\r", dock->hidden ? "Yes" : "No");
        return;
}

CMDF do_showbody(CHAR_DATA * ch, char *argument)
{
        BODY_DATA *body = NULL;
        AREA_DATA *area = NULL;

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showbody <body>\n\r", ch);
                return;
        }

        body = get_body(argument);
        if (!body)
        {
                send_to_char("No such body.\n\r", ch);
                return;
        }


        ch_printf(ch, "&W%s\n\r", body->name());
        ch_printf(ch, "&WFilename:    &G%s\n\r", body->filename());
        if (body->starsystem() && body->starsystem()->name)
                ch_printf(ch, "&WStar system: &G%s\n\r",
                          body->starsystem()->name);
        if (body->planet() && body->planet()->name)
                ch_printf(ch, "&WPlanet:      &G%s\n\r",
                          body->planet()->name);
        ch_printf(ch, "&WGravity:     &G%d\n\r", body->gravity());
        ch_printf(ch, "&WXpos:        &G%d\n\r", body->xpos());
        ch_printf(ch, "&WYpos:        &G%d\n\r", body->ypos());
        ch_printf(ch, "&WZpos:        &G%d\n\r", body->zpos());
        ch_printf(ch, "&WXmove:       &G%d\n\r", body->xmove());
        ch_printf(ch, "&WYmove:       &G%d\n\r", body->ymove());
        ch_printf(ch, "&WZmove:       &G%d\n\r", body->zmove());
        ch_printf(ch, "&WCenterpoint: &G%d, %d, %d\n\r", body->centerx(),
                  body->centery(), body->centerz());
        ch_printf(ch, "&WOrbit Tick:  &G%d\n\r", body->orbitcount());
        ch_printf(ch, "&WType:        &G%s\n\r", body->type_name());
        send_to_char("&WAreas:       &G", ch);
        FOR_EACH_LIST(AREA_LIST, body->areas(), area)
                ch_printf(ch, "%s, ", area->filename);
        send_to_char("\n\r", ch);
        return;
}


CMDF do_listdock(CHAR_DATA * ch, char *argument)
{
        DOCK_DATA *tdock;

        argument = NULL;

        for (tdock = first_dock; tdock; tdock = tdock->next)
                ch_printf(ch, "%-25s - %s\n\r", tdock->name,
                          tdock->body->name());
        return;
}

CMDF do_listbody(CHAR_DATA * ch, char *argument)
{
        BODY_DATA *tbody = NULL;

        argument = NULL;

        FOR_EACH_LIST(BODY_LIST, bodies, tbody)
                ch_printf(ch, "%-25s - %s\n\r", tbody->name(),
                          tbody->filename());
        return;
}

CMDF do_resetbody(CHAR_DATA * ch, char *argument)
{
        BODY_DATA *tbody = NULL;

        argument = NULL;

        FOR_EACH_LIST(BODY_LIST, bodies, tbody)
        {
                tbody->xpos(tbody->centerx() - (tbody->xmove() * 0));
                tbody->ypos(tbody->centery() - (tbody->ymove() * 0));
                tbody->zpos(tbody->centerz() - (tbody->zmove() * 1800));
                tbody->orbitcount(0);
                tbody->save();
        }
        send_to_char("All Body Coordinates Set.\n\r", ch);
}


CMDF do_placebody(CHAR_DATA * ch, char *argument)
{
        /*
         * this function will determine the starting coordinate of a planet
         * * * it will be called once makebody is done 
         */
        char      arg1[MAX_INPUT_LENGTH];
        BODY_DATA *body;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: Placebody <body> \n\r"
                             "\n\rThis function calculates the\n\r"
                             " starting coordinates of a body\n\r"
                             " based on its movement values.\n\r", ch);
                return;
        }

        body = get_body(arg1);
        if (!body)
        {
                send_to_char("No such body.\n\r", ch);
                return;
        }


        if (!body->filename())
        {
                send_to_char("This body can not be edited.\n\r", ch);
                return;
        }

        if (body)
        {
                body->xpos(body->centerx() - (body->xmove() * 0));
                body->ypos(body->centery() - (body->ymove() * 0));
                body->zpos(body->centerz() - (body->zmove() * 1800));
                body->orbitcount(0);
                send_to_char("Body Coordinates Set.\n\r", ch);
                body->save();
                return;
        }
}

void update_orbit(void)
{
        BODY_DATA *tbody = NULL;
        char      buf[MSL];
        SHIP_DATA *ship;
        BODY_DATA *body = NULL;
        int       phaze = 0, xtravel, ytravel, ztravel;

        FOR_EACH_LIST(BODY_LIST, bodies, body)
        {
                if (!body->starsystem())
                        continue;

                /*
                 * Check to see if orbitcount is valid, reset if the orbit is done its rotation. 
                 */
                if (body->orbitcount() < 0)
                        body->orbitcount(0);

                if (body->orbitcount() >= 360)
                {
                        body->orbitcount(0);
                        body->xpos(body->centerx() - (body->xmove() * 0));
                        body->ypos(body->centery() - (body->ymove() * 0));
                        body->zpos(body->centerz() - (body->zmove() * 1800));
                }

                /*
                 * Set the planet's phaze to determine movement multipliers. 
                 */
                if ((body->orbitcount() >= 0) && (body->orbitcount() <= 45))
                        phaze = 1;
                else if ((body->orbitcount() >= 46)
                         && (body->orbitcount() <= 90))
                        phaze = 2;
                else if ((body->orbitcount() >= 91)
                         && (body->orbitcount() <= 135))
                        phaze = 3;
                else if ((body->orbitcount() >= 136)
                         && (body->orbitcount() <= 180))
                        phaze = 4;
                else if ((body->orbitcount() >= 181)
                         && (body->orbitcount() <= 225))
                        phaze = 5;
                else if ((body->orbitcount() >= 226)
                         && (body->orbitcount() <= 270))
                        phaze = 6;
                else if ((body->orbitcount() >= 271)
                         && (body->orbitcount() <= 315))
                        phaze = 7;
                else if ((body->orbitcount() >= 316)
                         && (body->orbitcount() <= 360))
                        phaze = 8;

                /*
                 * Use phaze to calculate the acctual travel distance for each coord. 
                 */
                xtravel = 0;
                ytravel = 0;
                ztravel = 0;

                if (phaze == 1)
                {
                        xtravel = (body->xmove() * 20);
                        ytravel = (body->ymove() * 10);
                        ztravel = (body->zmove() * 20);
                }
                else if (phaze == 2)
                {
                        xtravel = (body->xmove() * 10);
                        ytravel = (body->ymove() * 20);
                        ztravel = (body->zmove() * 20);
                }
                else if (phaze == 3)
                {
                        xtravel = (body->xmove() * -10);
                        ytravel = (body->ymove() * 20);
                        ztravel = (body->zmove() * -20);
                }
                else if (phaze == 4)
                {
                        xtravel = (body->xmove() * -20);
                        ytravel = (body->ymove() * 10);
                        ztravel = (body->zmove() * -20);
                }
                else if (phaze == 5)
                {
                        xtravel = (body->xmove() * -20);
                        ytravel = (body->ymove() * -10);
                        ztravel = (body->zmove() * 20);
                }
                else if (phaze == 6)
                {
                        xtravel = (body->xmove() * -10);
                        ytravel = (body->ymove() * -20);
                        ztravel = (body->zmove() * 20);
                }
                else if (phaze == 7)
                {
                        xtravel = (body->xmove() * 10);
                        ytravel = (body->ymove() * -20);
                        ztravel = (body->zmove() * -20);
                }
                else if (phaze == 8)
                {
                        xtravel = (body->xmove() * 20);
                        ytravel = (body->ymove() * -10);
                        ztravel = (body->zmove() * -20);
                }

                if (xtravel == 0 && ytravel == 0 && ztravel == 0)
                        continue;

                if (body->starsystem())
                {
                        SPACE_DATA *starsystem = body->starsystem();

                        for (ship = starsystem->first_ship; ship;
                             ship = ship->next_in_starsystem)
                        {
                                if (body->distance(ship) < body->gravity()
                                    && (body->type() == PLANET_BODY
                                        || body->type() == MOON_BODY))
                                {
                                        ship->vx += xtravel;
                                        ship->vy += ytravel;
                                        ship->vz += ztravel;
                                }
                        }
                }
                /*
                 * Acctually preform the function of moving the body. 
                 */
                body->xpos(body->xpos() + xtravel);
                body->ypos(body->ypos() + ytravel);
                body->zpos(body->zpos() + ztravel);

                /*
                 * Advance the orbit count so the planet changes directions 
                 */
                body->orbitcount(body->orbitcount() + 1);

                /*
                 * Send a message to imms if planet is too close to sun 
                 */
                FOR_EACH_LIST(BODY_LIST, body->starsystem()->bodies, tbody)
                {
                        if (tbody == body)
                                continue;
                        if ((body->type() == PLANET_BODY
                             && tbody->type() == PLANET_BODY)
                            || (body->type() == STAR_BODY
                                && tbody->type() == STAR_BODY)
                            || (body->type() == STAR_BODY)
                            || (body->type() == ASTEROID_BODY)
                            || (body->type() == NEBULA_BODY)
                            || (tbody->type() == ASTEROID_BODY)
                            || (tbody->type() == NEBULA_BODY))
                                continue;
                        if ((body->distance(tbody)) <
                            ((tbody->gravity()) + (100)))
                        {
                                snprintf(buf, MSL,
                                         "Body too close to sun: %s near %s\n\r Distance: %d Gavity: %d",
                                         body->name(), tbody->name(),
                                         body->distance(tbody),
                                         (tbody->gravity() + 100));
                                log_string(buf);
                        }
                }

                /*
                 * Save the body, go on to the next 
                 */
                body->save();   /* I don't think you should constnatly disk write on an update - Gavin */
                continue;
        }
}


CMDF do_adjship(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        SHIP_DATA *ship;
        int       randchek;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
        {
                send_to_char
                        ("Usage: adjship <ship> <field> <enable/disable>\n\r",
                         ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char("shields drive lasers ions missiles\n\r", ch);
                send_to_char("turret1 turret2 energy hull\n\r", ch);
                send_to_char("random all\n\r", ch);
                return;
        }

        ship = get_ship(arg1);
        if (!ship)
        {
                send_to_char("No such ship.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "random"))
        {
                randchek = number_range(1, 9);
                if (randchek == 1)
                        snprintf(arg2, MSL, "%s", "shields");
                if (randchek == 2)
                        snprintf(arg2, MSL, "%s", "drive");
                if (randchek == 3)
                        snprintf(arg2, MSL, "%s", "lasers");
                if (randchek == 4)
                        snprintf(arg2, MSL, "%s", "ions");
                if (randchek == 5)
                        snprintf(arg2, MSL, "%s", "missiles");
                if (randchek == 6)
                        snprintf(arg2, MSL, "%s", "turret1");
                if (randchek == 7)
                        snprintf(arg2, MSL, "%s", "turret2");
                if (randchek == 8)
                        snprintf(arg2, MSL, "%s", "energy");
                if (randchek == 9)
                        snprintf(arg2, MSL, "%s", "hull");
        }

        if (!str_cmp(arg2, "shields"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->shield = 0;
                        echo_to_cockpit(AT_BLOOD, ship, "Shields down...");
                        send_to_char("&R Ship Shields Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->shield = (ship->maxshield * 1);
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "The shields re-initialize themselves");
                        send_to_char("&R Ship Shields Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "drive"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->shipstate = SHIP_DISABLED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ships drive disabled...");
                        send_to_char("&R Ship Drive Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->shipstate = SHIP_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "The drive restores itself");
                        send_to_char("&R Ship Drive Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "missiles"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->missilestate = MISSILE_DAMAGED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ships missiles damaged...");
                        send_to_char("&R Ship Missile Launchers Disabled",
                                     ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->missilestate = MISSILE_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Missile launchers repaired");
                        send_to_char("&R Ship Missiles Launchers Enabled",
                                     ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "lasers"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->statet0 = LASER_DAMAGED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ships lasers damaged...");
                        send_to_char("&R Ship Lasers Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->statet0 = LASER_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ship lasers repaired");
                        send_to_char("&R Ship Lasers Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "ions"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->statet0i = LASER_DAMAGED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ships ion cannons damaged...");
                        send_to_char("&R Ship Ion Cannons Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->statet0i = LASER_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Ship ion cannons repaired");
                        send_to_char("&R Ship Ions Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "turret1"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->statet1 = LASER_DAMAGED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Turret one damaged...");
                        send_to_char("&R Ship Turret One Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->statet1 = LASER_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "First turret repaired");
                        send_to_char("&R Ship Turret One Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "turret2"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->statet2 = LASER_DAMAGED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Turret two damaged...");
                        send_to_char("&R Ship Turret Two Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->statet2 = LASER_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Second turret repaired");
                        send_to_char("&R Ship Turret Two Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "energy"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->energy = (ship->maxenergy / 10);
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "WARNING: ENERGY DEPLETED...");
                        send_to_char
                                ("&R Energy Depleted (Warning: can cause ship destruction)",
                                 ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->energy = ship->maxenergy;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "The ships energy is restored");
                        send_to_char("&R Ship Energy Restored", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "hull"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->hull = (ship->maxhull / 10);
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "WARNING: HULL DAMAGED...");
                        send_to_char
                                ("&R Hull Damaged (Warning: can cause ship destruction)",
                                 ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->hull = ship->maxhull;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "The ships hull is restored");
                        send_to_char("&R Ship Hull Restored", ch);
                        save_ship(ship);
                        return;
                }
        }

        if (!str_cmp(arg2, "all"))
        {
                if (!str_cmp(argument, "disable"))
                {
                        ship->statet2 = LASER_DAMAGED;
                        ship->statet1 = LASER_DAMAGED;
                        ship->statet0i = LASER_DAMAGED;
                        ship->statet0 = LASER_DAMAGED;
                        ship->missilestate = MISSILE_DAMAGED;
                        ship->shipstate = SHIP_DISABLED;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "Your ship suddenly loses all function");
                        send_to_char("&R Ship Disabled", ch);
                        save_ship(ship);
                        return;
                }
                if (!str_cmp(argument, "enable"))
                {
                        ship->statet2 = LASER_READY;
                        ship->statet1 = LASER_READY;
                        ship->statet0i = LASER_READY;
                        ship->statet0 = LASER_READY;
                        ship->missilestate = MISSILE_READY;
                        ship->shipstate = SHIP_READY;
                        echo_to_cockpit(AT_BLOOD, ship,
                                        "The ship suddenly regains function");
                        send_to_char("&R Ship Enabled", ch);
                        save_ship(ship);
                        return;
                }
        }

}


CMDF do_makessrand(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char
                ("Gavin - Removed this function as good in theory, nto so useful in practice",
                 ch);
        return;
}

CMDF do_getvnumrange(CHAR_DATA * ch, char *argument)
{
        ch_printf(ch, "Start: %d End: %d", get_vnum_range(atoi(argument)),
                  get_vnum_range(atoi(argument)) + atoi(argument));
}

int get_vnum_range(int range)
{
        AREA_DATA *pArea;
        bool      a_conflict;
        int       count = 0;
        int       curr = 0;
        int       reset_amount = 0;

        for (curr = 0; curr <= MAX_VNUMS; curr++)
        {
                a_conflict = FALSE;
                for (pArea = first_asort; pArea; pArea = pArea->next_sort)
                {
                        if ((curr >= pArea->low_r_vnum
                             && curr <= pArea->hi_r_vnum)
                            || (curr >= pArea->low_m_vnum
                                && curr <= pArea->hi_m_vnum)
                            || (curr >= pArea->low_o_vnum
                                && curr <= pArea->hi_o_vnum))
                        {
                                a_conflict = TRUE;
                                curr = pArea->hi_r_vnum;
                                if (pArea->hi_m_vnum > curr)
                                        curr = pArea->hi_m_vnum;
                                if (pArea->hi_o_vnum > curr)
                                        curr = pArea->hi_o_vnum;
                        }
                }
                if (a_conflict == FALSE)
                {
                        for (pArea = first_bsort; pArea;
                             pArea = pArea->next_sort)
                        {
                                if ((curr >= pArea->low_r_vnum
                                     && curr <= pArea->hi_r_vnum)
                                    || (curr >= pArea->low_m_vnum
                                        && curr <= pArea->hi_m_vnum)
                                    || (curr >= pArea->low_o_vnum
                                        && curr <= pArea->hi_o_vnum))
                                {
                                        a_conflict = TRUE;
                                        curr = pArea->hi_r_vnum;
                                        if (pArea->hi_m_vnum > curr)
                                                curr = pArea->hi_m_vnum;
                                        if (pArea->hi_o_vnum > curr)
                                                curr = pArea->hi_o_vnum;
                                }
                        }
                        count++;
                }
                if (a_conflict == TRUE)
                {
                        count = 0;
                        reset_amount++;
                }
                else
                {
                        if (count == range)
                        {
                                return (curr - count + 1);
                        }
                }
        }
        return -1;
}

AREA_DATA *create_auto_area(int low_vnum, int high_vnum, BODY_DATA * body)
{
        int       firstvnum;
        AREA_DATA *pArea;
        ROOM_INDEX_DATA *location;
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        CHAR_DATA *mob;
        OBJ_DATA *obj;

        CREATE(pArea, AREA_DATA, 1);
        pArea->first_reset = NULL;
        pArea->last_reset = NULL;
        pArea->next_on_planet = NULL;
        pArea->prev_on_planet = NULL;
        pArea->next_on_body = NULL;
        pArea->prev_on_body = NULL;
        if (body != NULL)
        {
                pArea->planet = body->planet();
                pArea->body = body;
                pArea->name = str_dup(body->name());
                strdup_printf(&pArea->filename, "%s.are",
                              smash_space(body->name()));
        }
        else
        {
                pArea->name = str_dup("TestArea");
                strdup_printf(&pArea->filename, "%s.are", "TestArea");
        }
        pArea->author = STRALLOC("Autobuild");
        pArea->age = 15;
        pArea->nplayer = 0;
        pArea->low_r_vnum = low_vnum;
        pArea->low_o_vnum = low_vnum;
        pArea->low_m_vnum = low_vnum;
        pArea->hi_r_vnum = high_vnum;
        pArea->hi_o_vnum = high_vnum;
        pArea->hi_m_vnum = high_vnum;
        pArea->version = 1;
        pArea->flags = 0;
        pArea->max_players = 0;
        pArea->mkills = 0;
        pArea->mdeaths = 0;
        pArea->pkills = 0;
        pArea->pdeaths = 0;
        pArea->gold_looted = 0;
        pArea->illegal_pk = 0;
        pArea->high_economy = 0;
        pArea->low_economy = 0;
        pArea->reset_frequency = 10;
        pArea->resetmsg = str_dup("You hear a buzzing sound in the distance");
        SET_BIT(pArea->status, AREA_LOADED);
        pArea->low_soft_range = 0;
        pArea->hi_soft_range = MAX_LEVEL;
        pArea->low_hard_range = 0;
        pArea->hi_hard_range = MAX_LEVEL;
        LINK(pArea, first_area, last_area, next, prev);
        for (firstvnum = pArea->low_r_vnum; firstvnum <= pArea->hi_r_vnum;
             firstvnum++)
        {
                if (firstvnum < pArea->low_r_vnum
                    || firstvnum > pArea->hi_r_vnum)
                {
                        for (firstvnum = pArea->low_r_vnum;
                             firstvnum <= pArea->hi_r_vnum; firstvnum++)
                                if ((location =
                                     get_room_index(firstvnum)) != NULL)
                                        delete_room(location);
                        DISPOSE(pArea->name);
                        DISPOSE(pArea->filename);
                        STRFREE(pArea->author);
                        DISPOSE(pArea);
                        return NULL;
                }
                location = make_room(firstvnum, pArea);
                if (!location)
                {
                        bug("makessrand: make_room failed", 0);
                        for (firstvnum = pArea->low_r_vnum;
                             firstvnum <= pArea->hi_r_vnum; firstvnum++)
                                if ((location =
                                     get_room_index(firstvnum)) != NULL)
                                        delete_room(location);
                        DISPOSE(pArea->name);
                        DISPOSE(pArea->filename);
                        STRFREE(pArea->author);
                        DISPOSE(pArea);
                        return NULL;
                }
                location->area = pArea;
        }
        if ((pMobIndex = make_mobile(pArea->low_m_vnum, -1, "A dud")) == NULL)
        {
                for (firstvnum = pArea->low_r_vnum;
                     firstvnum <= pArea->hi_r_vnum; firstvnum++)
                        if ((location = get_room_index(firstvnum)) != NULL)
                                delete_room(location);
                DISPOSE(pArea->name);
                DISPOSE(pArea->filename);
                STRFREE(pArea->author);
                DISPOSE(pArea);
                return NULL;
        }
        else
        {
                mob = create_mobile(pMobIndex);
                char_to_room(mob, get_room_index(pArea->hi_r_vnum));
                add_reset(pArea, 'M', 1, mob->pIndexData->vnum,
                          mob->pIndexData->count, mob->in_room->vnum);
        }
        if ((pMobIndex = make_mobile(pArea->hi_m_vnum, -1, "A dud")) == NULL)
        {
                for (firstvnum = pArea->low_r_vnum;
                     firstvnum <= pArea->hi_r_vnum; firstvnum++)
                        if ((location = get_room_index(firstvnum)) != NULL)
                                delete_room(location);
                delete_mob(get_mob_index(pArea->low_m_vnum));
                DISPOSE(pArea->name);
                DISPOSE(pArea->filename);
                STRFREE(pArea->author);
                DISPOSE(pArea);
                return NULL;
        }
        else
        {
                mob = create_mobile(pMobIndex);
                char_to_room(mob, get_room_index(pArea->hi_r_vnum));
                add_reset(pArea, 'M', 1, mob->pIndexData->vnum,
                          mob->pIndexData->count, mob->in_room->vnum);
        }
        if ((pObjIndex = make_object(pArea->low_o_vnum, 0, "A dud")) == NULL)
        {
                for (firstvnum = pArea->low_r_vnum;
                     firstvnum <= pArea->hi_r_vnum; firstvnum++)
                        if ((location = get_room_index(firstvnum)) != NULL)
                                delete_room(location);
                delete_mob(get_mob_index(pArea->low_m_vnum));
                delete_mob(get_mob_index(pArea->hi_m_vnum));
                DISPOSE(pArea->name);
                DISPOSE(pArea->filename);
                STRFREE(pArea->author);
                DISPOSE(pArea);
                return NULL;
        }
        else
        {
                obj = create_object(pObjIndex, 1);
                obj_to_room(obj, get_room_index(pArea->hi_r_vnum));
                add_obj_reset(pArea, 'O', obj, 1, pArea->hi_r_vnum);
        }
        if ((pObjIndex = make_object(pArea->hi_o_vnum, 0, "A dud")) == NULL)
        {
                for (firstvnum = pArea->low_r_vnum;
                     firstvnum <= pArea->hi_r_vnum; firstvnum++)
                        if ((location = get_room_index(firstvnum)) != NULL)
                                delete_room(location);
                delete_mob(get_mob_index(pArea->low_m_vnum));
                delete_mob(get_mob_index(pArea->hi_m_vnum));
                delete_obj(get_obj_index(pArea->low_o_vnum));
                DISPOSE(pArea->name);
                DISPOSE(pArea->filename);
                STRFREE(pArea->author);
                DISPOSE(pArea);
                return NULL;
        }
        else
        {
                obj = create_object(pObjIndex, 1);
                obj_to_room(obj, get_room_index(pArea->hi_r_vnum));
                add_obj_reset(pArea, 'O', obj, 1, pArea->hi_r_vnum);
        }
        sort_area(pArea, FALSE);
        return pArea;
}

void free_dock(DOCK_DATA * dock)
{
        if (dock == NULL)
                return;
        if (dock->body)
        {
                BODY_DATA *body = dock->body;

                body->docks().
                        erase(find
                              (body->docks().begin(), body->docks().end(),
                               dock));
        }
        if (first_dock && (dock->next || dock->prev))
                UNLINK(dock, first_dock, last_dock, next, prev);
		if (dock->name)
			STRFREE(dock->name);
        DISPOSE(dock);
}

void generate_exits(int x, AREA_DATA * area)
{
        int       vnum, first, last, row = 0, column = 0;
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *room, *toroom;

        first = area->low_r_vnum;
        last = area->hi_r_vnum;

        for (vnum = area->low_r_vnum; vnum <= last; vnum++)
        {
                room = get_room_index(vnum);    //This room

                /*
                 * Check for north exit 
                 */
                if (vnum >= (first + x))    // Top row
                {
                        toroom = get_room_index(vnum - x);  //Room directly above
                        pexit = make_exit(room, toroom, DIR_NORTH);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }

                /*
                 * Check for south exit 
                 */
                if (vnum <= (last - x)) // Bottom row
                {
                        toroom = get_room_index(vnum + x);  //Room directly below
                        pexit = make_exit(room, toroom, DIR_SOUTH);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }

                /*
                 * Check for west exit 
                 */
                if (vnum != (first + (x * row)))    // Left column
                {
                        bug(" Vnum: %d first: %d x: %d row: %d column: %d",
                            vnum, first, x, row, column);
                        toroom = get_room_index(vnum - 1);  //Room directlyleft
                        pexit = make_exit(room, toroom, DIR_WEST);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }

                /*
                 * Check for east exit 
                 */
                if (vnum != (first + ((x * (row + 1)) - 1)))    // Right column
                {
                        toroom = get_room_index(vnum + 1);  //Room directly right
                        pexit = make_exit(room, toroom, DIR_EAST);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }

                if (column++ == (x - 1))    //We have hit how wide it is
                {
                        column = 0;
                        row++;
                }
        }
}

CMDF do_testexits(CHAR_DATA * ch, char *argument)
{
        char      arg1[MSL];
        char      arg2[MSL];
        AREA_DATA *area;
        int       x, y, firstvnum;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        x = atoi(arg1);
        y = atoi(arg2);

        if ((firstvnum = get_vnum_range(x * y)) == -1)
        {
                send_to_char("ERROR: NO VALID VNUM RANGE", ch);
                return;
        }

        if ((area =
             create_auto_area(firstvnum, firstvnum + x * y - 1,
                              NULL)) == NULL)
        {
                send_to_char("ERROR, FAILED TO MAKE AREA", ch);
                return;
        }

        generate_exits(x, area);
}

#ifdef IMAGES
void do_fastship(CHAR_DATA * ch, char *argument)
{
        SHIP_DATA *ship = NULL;
        PROTOSHIP_DATA *proto = NULL;
        char      arg1[MAX_STRING_LENGTH];
        char      arg2[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: FASTSHIP <owner or none> <prototype name> <ship name>\n\r",
                         ch);
                return;
        }

        if ((proto = get_protoship(arg2)) == NULL)
        {
                send_to_char
                        ("&RUnable to locate that ship ship image. Recheck your spelling.\n\r",
                         ch);
                return;
        }

        if (!shipimage_exist(proto->shipclass))
        {
                send_to_char
                        ("&RUnable to locate that ship image file. Recheck your spelling.\n\r",
                         ch);
                return;
        }

        /*
         * Generate the ship. This is tight.
         *  Pass the character, ship ship image and ship name.
         */
        ship = make_ship(ch, proto, argument);
        if (get_trust(ch) > LEVEL_HERO)
        {
                if (ship != NULL)
                {
                        send_to_char
                                ("&YFASTSHIP Complete. Have a nice day.\n\r",
                                 ch);
                }
                else
                {
                        send_to_char
                                ("&R[WARNING]: Fault in FASTSHIP system. No ship returned.\n\r",
                                 ch);
                }
        }
        if (str_cmp(arg1, "none"))
                ship->owner = STRALLOC(arg1);
        return;
}

void do_viewship(CHAR_DATA * ch, char *argument)
{
        SHIP_DATA *ship = NULL;
        char      arg1[MAX_STRING_LENGTH];

        /*
         * char arg2[MAX_STRING_LENGTH]; 
         */

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("&RSyntax: VIEWSHIP <ship ship image>\n\r", ch);
                return;
        }

        if (!shipimage_exist(arg1))
        {
                send_to_char
                        ("&RUnable to locate that ship ship image. Recheck your spelling.\n\r",
                         ch);
                return;
        }

        /*
         * Generate the ship. This is tight.
         *  Pass the character and ship shipimage.
         */
        ship = view_ship(arg1);

        if (ship != NULL)
        {
                /*
                 * Display ship
                 */
                ch_printf(ch, "&zClass: &C%s&z\n\r&B",
                          ship->prototype->shipclass);
                ch_printf(ch, "&zValue: &C%ld &zcredit(s)\n\r",
                          get_ship_value(ship));
                ch_printf(ch, "&zTotal Rooms: &C%d\n\r",
                          (ship->lastroom - ship->firstroom) + 1);
                if (ship->description[0] != '\0')
                        ch_printf(ch, "&zDescription: &C%s\n\r",
                                  ship->description);
                ch_printf(ch,
                          "&z--&CWeapons&z-------------------------------------------------------\n\r");
                ch_printf(ch,
                          "&zAutoblasters: &C%-5d       &zLaser cannons:    &C%-5d   &zTurbolasers: &C%-5d\n\r",
                          ship->blasters, ship->lasers, ship->turbolasers);
                ch_printf(ch,
                          "&zIon cannons:  &C%-5d       &zMaximum Missiles: &C%-5d   &zMaximum Torpedos: &C%-5d\n\r",
                          ship->ion, ship->maxmissiles, ship->maxtorpedos);
                ch_printf(ch,
                          "&zMaximum Rockets: &C%-5d    &zMaximum Pulses:   &C%-5d   &zMaximum Chaff: &C%-5d\n\r",
                          ship->maxrockets, ship->maxpulses, ship->maxchaff);
                ch_printf(ch, "&zMissile Tubes: &C%-5d\n\r",
                          ship->missiletubes);
                ch_printf(ch,
                          "&z--&CSystems&z-------------------------------------------------------\n\r");
                ch_printf(ch,
                          "&zMax Hull:      &C%-7d    &zMax Shields: &C%-7d      &zMax Energy(fuel): &C%-5d\n\r",
                          ship->maxhull, ship->maxshield, ship->maxenergy);
                ch_printf(ch,
                          "&zMaximum Speed: &C%-7d    &zHyperspeed:  &C%-7d      &zManuever: &C%-5d\n\r",
                          ship->realspeed, ship->hyperspeed, ship->manuever);
                ch_printf(ch, "&zCloaking Device: %s\n\r",
                          ship->cloakstatus !=
                          CLOAK_NONE ? "&CInstalled" : "&zNot Installed");

                /*
                 * Junk it 
                 */
                DISPOSE(ship);
                send_to_char("&YVIEWSHIP Complete. Have a nice day.\n\r", ch);
        }
        else
        {
                send_to_char
                        ("&R[WARNING]: Fault in FASTSHIP system. Unable to view that ship image.\n\r",
                         ch);
        }

        return;
}

void finish_ship(CHAR_DATA * ch, SHIP_DATA * ship)
{
        ROOM_INDEX_DATA *dock;

        if (!ship || ship == NULL)
        {
                bug("*ERROR* finish_ship: NULL POINTER!");
                return;
        }

        ship->currspeed = 0;
        ship->energy = ship->maxenergy;
        ship->charge = ship->maxcharge;
        ship->chaff = ship->maxchaff;
        ship->hull = ship->maxhull;
        ship->spent_tubes = 0;
        ship->missiles = ship->maxmissiles;
        ship->torpedos = ship->maxtorpedos;
        ship->rockets = ship->maxrockets;
        ship->pulses = ship->maxpulses;
        ship->ion_state = ION_READY;
        ship->statet0 = LASER_READY;
        ship->statet1 = LASER_READY;
        ship->statet2 = LASER_READY;
        ship->missilestate = LASER_READY;
        ship->cloakstatus = 0;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->hatchopen = FALSE;
        ship->hatchtimer = 0;
        ship->autorecharge = FALSE;
        ship->autotrack = FALSE;
        ship->autospeed = FALSE;
        ship->code[0] = number_range(11111, 99999);
        ship->code[1] = number_range(11111, 99999);
        ship->code[2] = number_range(11111, 99999);
        ship->code[3] = number_range(11111, 99999);
        ship->create_date = time(0);

        if (ch == NULL)
                dock = g_r_i(ROOM_LIMBO_SHIPYARD);
        else if (!ch->in_room)
                dock = g_r_i(ROOM_LIMBO_SHIPYARD);
        else if (!xIS_SET(ch->in_room->room_flags, ROOM_CAN_FLY))
                dock = g_r_i(ROOM_LIMBO_SHIPYARD);
        else
                dock = ch->in_room;

        ship_to_room(ship, dock->vnum);
        ship->location = dock->vnum;
        ship->lastdoc = dock->vnum;

        if (ship->filename != NULL && ship->filename[0] != '\0')
        {
                fold_vships();
                save_ship(ship);
                write_ship_list();
        }

        return;
}

/*
 * Finds a FREE Ship filename!
 * Converts a normal extentoion (IE: XG)
 * Into a real usable filename  (IE: XG01)
 */
char     *get_free_ship(char *arg)
{
        SHIP_DATA *ship;
        char      buf[MAX_STRING_LENGTH];
        int       snum = 0;
        bool      sExists = FALSE;

        for (snum = 1; snum <= 999; snum++) /* Start into the loop  */
        {
                sprintf(buf, "%s%-2.2d", arg, snum);    /* Form the filename    */
                sExists = FALSE;    /* Clear the found flag */

                for (ship = first_ship; ship; ship = ship->next)
                        if (ship && ship != NULL)
                                if (!str_cmp(ship->filename, buf))
                                        sExists = TRUE;
                for (ship = first_stored; ship; ship = ship->next)
                        if (ship && ship != NULL)
                                if (!str_cmp(ship->filename, buf))
                                        sExists = TRUE;
                if (!sExists)
                        return STRALLOC(buf);
        }
        bug("Fatal Error: Could not locate free ship number.");
        return NULL;
}

/*
 * All *FASTSHIPS* are created in vships.are
 */
ROOM_INDEX_DATA *make_ship_room(SHIP_DATA * ship, int svnum)
{
        ROOM_INDEX_DATA *pRoom = NULL;
        AREA_DATA *sArea;
        int       vnum;
        bool      aFound, rFound;

        ship = NULL;    /*Unused, must have been a book mark for his code - Orion */

        aFound = FALSE;
        rFound = FALSE;

        for (sArea = first_area; sArea; sArea = sArea->next)
                if (!str_cmp("vships.are", sArea->filename))
                {
                        aFound = TRUE;
                        break;
                }

        if (!aFound)
        {
                bug("----<> WARNING : CANNOT FIND VSHIPS.ARE : WARNING <>----");
                return NULL;
        }

        /*
         * for ( vnum = sArea->low_r_vnum ; vnum <= sArea->hi_r_vnum && !rFound; vnum++ ) 
         */
        for (vnum = svnum; vnum <= sArea->hi_r_vnum && !rFound; vnum++)
        {
                if (get_room_index(vnum) == NULL)
                {
                        rFound = TRUE;
                        pRoom = make_room(vnum);
                        pRoom->area = sArea;
                        xSET_BIT(pRoom->room_flags, ROOM_SPACECRAFT);
                        /*
                         * xSET_BIT( pRoom->room_flags, ROOM_INDOORS ); 
                         */
                        xREMOVE_BIT(pRoom->room_flags, ROOM_PROTOTYPE);
                }
        }

        if (!rFound)
        {
                bug("CANNOT LOCATE ROOM-->ABORTING");
                return NULL;
        }

        return pRoom;
}

int find_room_range(int total)
{
        char      buf[MAX_STRING_LENGTH];
        AREA_DATA *sArea;
        int       vnum, cnt = 0, fvnum = 0;
        bool      aFound;

        aFound = FALSE;

        for (sArea = first_area; sArea; sArea = sArea->next)
                if (!str_cmp("vships.are", sArea->filename))
                {
                        aFound = TRUE;
                        break;
                }

        if (!aFound)
        {
                bug("Could not locate vships.are, aborting range lookup.");
                return -1;
        }

        for (vnum = sArea->low_r_vnum; vnum <= sArea->hi_r_vnum; vnum++)
        {
                if (get_room_index(vnum) == NULL)
                {
                        cnt++;
                        if (fvnum == 0)
                                fvnum = vnum;
                        if (cnt >= total)
                        {
                                sprintf(buf,
                                        "Find_room_range: Found range of %d rooms",
                                        cnt);
                                /*
                                 * log_string(buf); 
                                 */
                                return fvnum;
                        }
                }
                else
                {
                        cnt = 0;
                        fvnum = 0;
                }
        }

        if (cnt <= total)
        {
                sprintf(buf,
                        "Warning: Unable to locate %d rooms in vships.are.",
                        total);
                bug(buf);
                return -1;
        }

        return -1;
}

void fold_vships(void)
{
        AREA_DATA *area;
        bool      aFound = FALSE;

        for (area = first_area; area; area = area->next)
                if (!str_cmp("vships.are", area->filename))
                {
                        aFound = TRUE;
                        break;
                }

        if (!aFound)
        {
                bug("----<> WARNING : CANNOT FIND VSHIPS.ARE : WARNING <>----");
                return;
        }

        fold_area(area, area->filename, FALSE);

        return;
}

SHIP_DATA *make_ship(CHAR_DATA * ch, PROTOSHIP_DATA * proto, char *arg2)
{
        FILE     *fp;
        SHIP_DATA *ship;
        char      shipname[MAX_STRING_LENGTH];
        char      filename[MAX_STRING_LENGTH];

        CREATE(ship, SHIP_DATA, 1);

        ship->description = STRALLOC("");
        ship->owner = STRALLOC("");
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->starsystem = NULL;
        ship->quota = 0;
        ship->energy = ship->maxenergy;
        ship->charge = ship->maxcharge;
        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->next_in_room = NULL;
        ship->prev_in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->cloakstatus = CLOAK_NONE;
        ship->lasers = 0;
        ship->turbolasers = 0;
        ship->blasters = 0;
        ship->act_flags = 0;
        ship->mod_flags = 0;
        ship->dock_state = 0;
        ship->dock_install = FALSE;

        /*
         * Now we have a ship. Now the scary part --- Start loading the shipimage
         */
        sprintf(filename, "%s%s", SHIPIMMAGE_DIR, protoship->shipclass);
        if ((fp = fopen(filename, "r")) != NULL)
        {
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Make_ship: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "SHIPIMMAGE"))
                        {
                                fread_shipimmage(ship, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                sprintf(buf, "Make_ship: bad section: %s.",
                                        word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
                /*
                 * Failed to load shipimage 
                 */
                bug("Make_ship: Failed to load shipimage!", 0);
                DISPOSE(ship);
                return NULL;
        }

        if (!ship->filename || ship->filename == NULL)
        {
                bug("Make_ship: Ship missing filename!", 0);
                DISPOSE(ship);
                return NULL;
        }

        LINK(ship, first_ship, last_ship, next, prev);

        sprintf(shipname, "%s %s", proto->shipclass,
                strupper(ship->filename));
        ship->name = STRALLOC(shipname);

        finish_ship(ch, ship);

        return ship;
}

SHIP_DATA *view_ship(char *arg1)
{
        FILE     *fp;
        SHIP_DATA *ship;
        char      filename[MAX_STRING_LENGTH];

        CREATE(ship, SHIP_DATA, 1);

        ship->description = STRALLOC("");
        ship->owner = STRALLOC("");
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->homeship = STRALLOC("");
        ship->ident = STRALLOC("");
        ship->starsystem = NULL;
        ship->energy = ship->maxenergy;
        ship->charge = ship->maxcharge;
        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->next_in_room = NULL;
        ship->prev_in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->cloakstatus = CLOAK_NONE;
        ship->lasers = 0;
        ship->turbolasers = 0;
        ship->blasters = 0;
        ship->act_flags = 0;
        ship->mod_flags = 0;
        ship->dock_state = 0;
        ship->dock_install = FALSE;

        /*
         * Now we have a ship. Now the scary part --- Start loading the shipimage
         */
        sprintf(filename, "%s%s", SHIPIMMAGE_DIR, arg1);
        if ((fp = fopen(filename, "r")) != NULL)
        {
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("View_ship: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "SHIPIMAGE"))
                        {
                                fread_viewshipimage(ship, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                sprintf(buf, "View_ship: bad section: %s.",
                                        word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
                /*
                 * Failed to load shipimage 
                 */
                bug("View_ship: Failed to load shipimage!", 0);
                DISPOSE(ship);
                return NULL;
        }

        /*
         * Finish setup 
         */
        ship->currspeed = 0;
        ship->energy = ship->maxenergy;
        ship->charge = ship->maxcharge;
        ship->chaff = ship->maxchaff;
        ship->hull = ship->maxhull;
        ship->spent_tubes = 0;
        ship->missiles = ship->maxmissiles;
        ship->torpedos = ship->maxtorpedos;
        ship->rockets = ship->maxrockets;
        ship->pulses = ship->maxpulses;
        ship->ion_state = ION_READY;
        ship->statet0 = LASER_READY;
        ship->statet1 = LASER_READY;
        ship->statet2 = LASER_READY;
        ship->missilestate = LASER_READY;
        ship->cloakstatus = 0;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->hatchopen = FALSE;
        ship->hatchtimer = 0;
        ship->autorecharge = FALSE;
        ship->autotrack = FALSE;
        ship->autospeed = FALSE;

        return ship;
}

/* if ( !lock_rprog( to, mprg->comlist, mprg->arglist, mprg->type ) ) */

bool lock_rprog(ROOM_INDEX_DATA * room, char *prog, char *argument,
                int mptype)
{
        MPROG_DATA *mprog = NULL, *mprg = NULL;

        mprog = room->mudprogs;
        if (mprog)
        {
                for (; mprog->next; mprog = mprog->next)
                {
                }
        }

        CREATE(mprg, MPROG_DATA, 1);

        if (mprog)
        {
                mprog->next = mprg;
        }
        else
        {
                room->mudprogs = mprg;
        }

        if (mprg == NULL)
                return FALSE;
        room->progtypes |= mptype;
        mprg->type = mptype;
        if (mprg->arglist)
                STRFREE(mprg->arglist);
        mprg->arglist = STRALLOC(argument);
        mprg->comlist = STRALLOC(prog);
        mprg->next = NULL;

        return TRUE;
}

/*
 * Creates a shipimage map of a specific ship
 *  Owner: FASTSHIP Module
 *  Author: Ghost
 */
void do_imageship(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_STRING_LENGTH];
        char      arg2[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        SHIP_DATA *ship = NULL;
        bool      rtn = FALSE;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("&RSyntax: imageship <ship>\n\r", ch);
                send_to_char
                        ("&R IE: imageship 'Firespray FS01' firespray.img\n\r",
                         ch);
                return;
        }

        /*
         * Try to locate the ship
         */
        if ((ship = get_ship(arg1)) == NULL)
        {
                send_to_char
                        ("&RUnable to find that ship. Try a diffrent name.\n\r",
                         ch);
                return;
        }

        if (shipimage_exist(ship->prototype->shipclass))
        {
                send_to_char
                        ("&RTheres already a ship image with that name!\n\r",
                         ch);
                return;
        }

        /*
         * This bit is part of the filing process for ship images.
         *
         * Might add a check for bestowment to avoid the warning, but shouldn't need it. - Orion
         */
        /*
         * Save the ship shipimage 
         */
        rtn = save_shipimage(ship, arg2);

        if (!rtn)
        {
                send_to_char
                        ("&RWarning: Fault in Save_shipimage module, Aborting IMAGESHIP.\n\r",
                         ch);
                return;
        }
        sprintf(buf, "%s.img", arg2);
        ch_printf(ch, "Finished imaging: %s\n\r", buf);
        do_viewship(ch, buf);
        send_to_char("&YIMAGESHIP Complete. Have a nice day.\n\r", ch);
        return;
}

bool save_shipimage(SHIP_DATA * ship, char *fname)
{
        FILE     *fp;
        char      filename[256];

        /*
         * char buf[MAX_STRING_LENGTH]; 
         */
        TURRET_DATA *turret;
        HANGAR_DATA *hangar;
        ESCAPE_DATA *escape;
        CARGO_DATA *cargo;
        ROOM_INDEX_DATA *room = NULL;
        EXIT_DATA *pexit = NULL;
        int       vnum;

        if (!ship)
        {
                bug("save_shipimage: null ship pointer!", 0);
                return FALSE;
        }

        if ((ship->lastroom - ship->firstroom) < 0)
        {
                bug("save_shipimage: invalid firstroom/lastroom!", 0);
                return FALSE;
        }

        sprintf(filename, "%s%s.img", SHIPIMAGE_DIR, fname);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_shipimage: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#SHIPIMAGE\n");
                fprintf(fp, "Prototype    %s~\n", ship->prototype->name)
                        for (vnum = ship->firstroom; vnum <= ship->lastroom;
                             vnum++)
                {
                        if ((room = get_room_index(vnum)) != NULL)
                        {
                                fprintf(fp, "RSector      %d\n",
                                        room->sector_type);
                                fprintf(fp, "RFlags       %s\n",
                                        print_bitvector(&room->room_flags));
                                fprintf(fp, "RName        %s~\n", room->name);
                                fprintf(fp, "RDesc        %s~\n",
                                        strip_cr(room->description));
                        }
                        else
                        {
                                fprintf(fp, "RSector      %d\n", 0);
                                fprintf(fp, "RFlags       0\n");
                                fprintf(fp,
                                        "RName        Worthlessly Pointless Room~\n");
                                fprintf(fp, "RDesc        ~\n");
                        }
                }
                /*
                 * List the exits afterwards 
                 */
                for (vnum = ship->firstroom; vnum <= ship->lastroom; vnum++)
                {
                        if ((room = get_room_index(vnum)) != NULL)
                        {
                                if (!room->first_exit)
                                        continue;
                                for (pexit = room->first_exit; pexit;
                                     pexit = pexit->next)
                                {
                                        fprintf(fp, "RExit        %d\n",
                                                vnum - fRm);
                                        fprintf(fp, "%d %d %d %d\n",
                                                pexit->vdir, pexit->key,
                                                pexit->distance,
                                                pexit->to_room ? pexit->
                                                to_room->vnum - fRm : 0);
                                        fprintf(fp, "%s~\n",
                                                strip_cr(pexit->description));
                                        fprintf(fp, "%s~\n",
                                                strip_cr(pexit->keyword));
                                        fprintf(fp, "%s\n",
                                                print_bitvector(&pexit->
                                                                exit_info));
                                }
                        }
                }

                /*
                 * Store the new turrets 
                 */
                if (ship->first_turret)
                        for (turret = ship->first_turret; turret;
                             turret = turret->next)
                                fprintf(fp, "Turret       %d %d\n",
                                        turret->room !=
                                        NULL ? turret->room->vnum - fRm : 0,
                                        turret->laserstate);

                /*
                 * Store the new hangars 
                 */
                if (ship->first_hangar)
                        for (hangar = ship->first_hangar; hangar;
                             hangar = hangar->next)
                                fprintf(fp, "Hangar       %d %d %d\n",
                                        hangar->room !=
                                        NULL ? hangar->room->vnum - fRm : 0,
                                        hangar->size,
                                        hangar->bayopen ? 1 : 0);

                /*
                 * Store the new escape pods 
                 */
                if (ship->first_escape)
                        for (escape = ship->first_escape; escape;
                             escape = escape->next)
                                fprintf(fp, "Escape       %d %d %d\n",
                                        escape->room !=
                                        NULL ? escape->room->vnum - fRm : 0,
                                        escape->pods, escape->max_pods);

                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return TRUE;
}

void fread_shipimage(SHIP_DATA * ship, FILE * fp)
{
        ROOM_INDEX_DATA *room = NULL;
        EXIT_DATA *pexit = NULL;
        char      buf[MAX_STRING_LENGTH];
        char     *line;
        char     *word;
        bool      fMatch;
        bool      rSet = FALSE;

        /*
         * int dummy_number; 
         */
        int       x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
        int       fRm = 0, rPnt = 0, rStart = 0;

        x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;

        ship->act_flags = 0;
        ship->mod_flags = 0;
        xCLEAR_BITS(ship->flags);
        xCLEAR_BITS(ship->aflags);
        ship->first_turret = NULL;
        ship->last_turret = NULL;
        ship->first_hangar = NULL;
        ship->last_hangar = NULL;
        ship->first_escape = NULL;
        ship->last_escape = NULL;
        ship->first_cargo = NULL;
        ship->last_cargo = NULL;

        fRm = 0;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Astro_array", ship->astro_array,
                            fread_number(fp));
                        KEY("Actflags", ship->act_flags, fread_number(fp));
                        KEY("AFlags", ship->aflags, fread_bitvector(fp));
                        break;

                case 'B':
                        KEY("Blasters", ship->blasters, fread_number(fp));
                        break;

                case 'C':
                        KEY("Cockpit", ship->cockpit, fread_number(fp) + fRm);
                        KEY("Coseat", ship->coseat, fread_number(fp) + fRm);
                        KEY("Class", ship->class, fread_number(fp));
                        KEY("Comm", ship->comm, fread_number(fp));
                        KEY("Cloakstatus", ship->cloakstatus,
                            fread_number(fp));
                        if (!str_cmp(word, "Cargo"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = 0;
                                sscanf(line, "%d %d %d %d %d", &x0, &x1, &x2,
                                       &x3, &x4);
                                new_cargo(ship);
                                ship->last_cargo->size = x0;
                                ship->last_cargo->type = x3;
                                ship->last_cargo->amount = x4;
                                fMatch = TRUE;
                                break;
                        }
                        break;


                case 'D':
                        KEY("Description", ship->description,
                            fread_string(fp));
                        KEY("Dockstate", ship->dock_state, fread_number(fp));
                        KEY("Dockroom", ship->dockroom,
                            fread_number(fp) + fRm);
                        break;

                case 'E':
                        KEY("Engineroom", ship->engineroom,
                            fread_number(fp) + fRm);
                        KEY("Entrance", ship->entrance,
                            fread_number(fp) + fRm);
                        if (!str_cmp(word, "Escape"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = 0;
                                sscanf(line, "%d %d %d", &x0, &x1, &x2);
                                if (x0 == 0 && x1 == 0 && x2 == 0)
                                {
                                        continue;
                                }
                                else
                                {
                                        new_escape(ship);
                                        ship->last_escape->room =
                                                get_room_index(x0 + fRm);
                                        ship->last_escape->pods = x1;
                                        ship->last_escape->max_pods = x2;
                                        fMatch = TRUE;
                                        break;
                                }
                        }
                        else if (!str_cmp(word, "End"))
                        {
                                ship->home = STRALLOC("");
                                ship->name = STRALLOC("");
                                ship->owner = STRALLOC("");
                                if (!ship->description)
                                        ship->description = STRALLOC("");
                                ship->copilot = STRALLOC("");
                                ship->pilot = STRALLOC("");
                                ship->homeship = STRALLOC("");
                                if (!ship->ident)
                                        ship->ident = STRALLOC("PS");
                                if (ship->ident[0] == '\0')
                                {
                                        STRFREE(ship->ident);
                                        ship->ident = STRALLOC("PS");
                                }
                                ship->filename =
                                        STRALLOC(get_free_ship(ship->ident));
                                ship->code[0] = number_range(11111, 99999);
                                ship->code[1] = number_range(11111, 99999);
                                ship->code[2] = number_range(11111, 99999);
                                ship->code[3] = number_range(11111, 99999);
                                ship->code[4] = number_range(11111, 99999);
                                ship->code[5] = number_range(11111, 99999);

                                if (ship->maxcharge <= 0)
                                        ship->maxcharge =
                                                10 * (ship->class + 1);

                                if (ship->dock_state != DOCK_NONE)
                                {
                                        ship->dock_install = TRUE;
                                        ship->dock_state = DOCK_READY;
                                }

                                if (ship->missiletubes <= 0)
                                {
                                        ship->missiletubes = 0;
                                        if (ship->maxmissiles > 0
                                            || ship->maxtorpedos > 0
                                            || ship->maxrockets > 0)
                                                ship->missiletubes = 1;
                                }

                                if (ship->shipstate != SHIP_DISABLED)
                                        ship->shipstate = SHIP_DOCKED;
                                if (ship->ion_state != ION_DAMAGED)
                                        ship->ion_state = ION_READY;
                                if (ship->statet0 != LASER_DAMAGED)
                                        ship->statet0 = LASER_READY;
                                if (ship->statet1 != LASER_DAMAGED)
                                        ship->statet1 = LASER_READY;
                                if (ship->statet2 != LASER_DAMAGED)
                                        ship->statet2 = LASER_READY;
                                if (ship->missilestate != MISSILE_DAMAGED)
                                        ship->missilestate = MISSILE_READY;
                                if (ship->shipyard <= 0)
                                        ship->shipyard = ROOM_LIMBO_SHIPYARD;
                                if (ship->lastdoc <= 0)
                                        ship->lastdoc = ship->shipyard;
                                ship->autopilot = FALSE;
                                ship->hatchopen = FALSE;
                                ship->hatchtimer = 0;
                                if (ship->coseat < ship->firstroom
                                    || ship->coseat > ship->lastroom)
                                        ship->coseat = ship->cockpit;
                                if (ship->gunseat < ship->firstroom
                                    || ship->gunseat > ship->lastroom)
                                        ship->gunseat = ship->cockpit;
                                if (ship->navseat < ship->firstroom
                                    || ship->navseat > ship->lastroom)
                                        ship->navseat = ship->cockpit;
                                if (ship->entrance < ship->firstroom
                                    || ship->entrance > ship->lastroom)
                                        ship->entrance = ship->cockpit;
                                if (ship->dockroom < ship->firstroom
                                    || ship->dockroom > ship->lastroom)
                                        ship->dockroom = 0;
                                if (ship->pilotseat < ship->firstroom
                                    || ship->pilotseat > ship->lastroom)
                                        ship->pilotseat = ship->cockpit;
                                if (ship->engineroom < ship->firstroom
                                    || ship->engineroom > ship->lastroom)
                                        ship->engineroom = ship->cockpit;
                                ship->starsystem = NULL;
                                ship->energy = ship->maxenergy;
                                ship->charge = ship->maxcharge;
                                ship->hull = ship->maxhull;
                                ship->in_room = NULL;
                                ship->next_in_room = NULL;
                                ship->prev_in_room = NULL;
                                return;
                        }
                        break;

                case 'F':
                        KEY("Flags", ship->flags, fread_bitvector(fp));
                        break;

                case 'G':
                        KEY("Gunseat", ship->gunseat, fread_number(fp) + fRm);
                        break;

                case 'H':
                        KEY("Hyperspeed", ship->hyperspeed, fread_number(fp));
                        if (!str_cmp(word, "Hangar")
                            || !str_cmp(word, "Hanger"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = 0;
                                sscanf(line, "%d %d %d", &x0, &x1, &x2);
                                if (x0 == 0 && x1 == 0 && x2 == 0)
                                {
                                        continue;
                                }
                                else
                                {
                                        new_hangar(ship);
                                        ship->last_hangar->room =
                                                get_room_index(x0 + fRm);
                                        ship->last_hangar->size = x1;
                                        ship->last_hangar->bayopen = FALSE;
                                        fMatch = TRUE;
                                        break;
                                }
                        }
                        break;

                case 'I':
                        KEY("Ident", ship->ident, fread_string(fp));
                        KEY("Ion", ship->ion, fread_number(fp));
                        break;

                case 'L':
                        KEY("Lasers", ship->lasers, fread_number(fp));
                        break;

                case 'M':
                        KEY("Manuever", ship->manuever, fread_number(fp));
                        KEY("Magnetic", ship->mines[MAGNETIC_MINE],
                            fread_number(fp));
                        KEY("Missiletubes", ship->missiletubes,
                            fread_number(fp));
                        KEY("Maxmissiles", ship->maxmissiles,
                            fread_number(fp));
                        KEY("Maxtorpedos", ship->maxtorpedos,
                            fread_number(fp));
                        KEY("Maxrockets", ship->maxrockets, fread_number(fp));
                        KEY("Maxpulses", ship->maxpulses, fread_number(fp));
                        KEY("Maxshield", ship->maxshield, fread_number(fp));
                        KEY("Maxenergy", ship->maxenergy, fread_number(fp));
                        KEY("Maxcharge", ship->maxcharge, fread_number(fp));
                        KEY("Modflags", ship->mod_flags, fread_number(fp));
                        KEY("Maxhull", ship->maxhull, fread_number(fp));
                        KEY("Maxchaff", ship->maxchaff, fread_number(fp));
                        KEY("Maxtroops", ship->maxtroops, fread_number(fp));
                        break;

                case 'N':
                        KEY("Navseat", ship->navseat, fread_number(fp) + fRm);
                        break;

                case 'P':
                        KEY("Pilotseat", ship->pilotseat,
                            fread_number(fp) + fRm);
                        KEY("Proximity", ship->mines[PROXIMITY_MINE],
                            fread_number(fp));
                        break;

                case 'R':
                        if (!str_cmp(word, "ROffset"))
                        {
                                line = fread_line(fp);
                                sscanf(line, "%d", &rPnt);
                                room = make_ship_room(ship, rStart);
                                fMatch = TRUE;
                                break;
                        }
                        if (!str_cmp(word, "RExit"))
                        {
                                line = fread_line(fp);
                                x0 = 0;
                                sscanf(line, "%d", &x0);
                                room = get_room_index(x0 + fRm);
                                if (room == NULL)
                                {
                                        sprintf(buf,
                                                "fread_room: Failed room lookup on %d (First was %d) for exit creation!",
                                                x0 + fRm, fRm);
                                        bug(buf, 0);
                                        return;
                                }
                                x0 = x1 = x2 = x3 = 0;
                                line = fread_line(fp);
                                sscanf(line, "%d %d %d %d", &x0, &x1, &x2,
                                       &x3);
                                pexit = make_exit(room, NULL, x0);
                                pexit->vdir = x0;
                                pexit->key = x1;
                                pexit->distance = x2;
                                pexit->vnum = x3 + fRm;
                                pexit->to_room = get_room_index(pexit->vnum);
                                pexit->description = fread_string(fp);
                                pexit->keyword = fread_string(fp);
                                xCLEAR_BITS(pexit->exit_info);
                                pexit->exit_info = fread_bitvector(fp);
                                fMatch = TRUE;

                                break;
                        }
                        if (!str_cmp(word, "Rooms") && rSet == FALSE)
                        {
                                line = fread_line(fp);
                                sscanf(line, "%d", &ship->lastroom);

                                /*
                                 * Lets make some rooms to work with here 
                                 */
                                rStart = find_room_range(ship->lastroom + 1);
                                if (rStart <= 0)
                                {
                                        bug("fread_shipimage: Not enough rooms for spacecraft!", 0);
                                        return;
                                }

                                ship->firstroom = rStart;
                                fRm = ship->firstroom;
                                ship->lastroom += fRm;
                                fMatch = TRUE;
                                rSet = TRUE;
                                break;
                        }
                        KEY("RName", room->name, fread_string(fp));
                        KEY("RDesc", room->description, fread_string(fp));
                        KEY("RFlags", room->room_flags, fread_bitvector(fp));
                        KEY("RSector", room->sector_type, fread_number(fp));
                        KEY("RTunnel", room->tunnel, fread_number(fp));
                        KEY("Realspeed", ship->realspeed, fread_number(fp));
                        /*
                         * KEY( "Rooms",          ship->lastroom,        fread_number( fp )+fRm ); 
                         */
                        break;

                case 'S':
                        KEY("Sensor", ship->sensor, fread_number(fp));
                        KEY("Seeker", ship->mines[SEEKER_MINE],
                            fread_number(fp));
                        break;

                case 'T':
                        KEY("Tractorbeam", ship->tractorbeam,
                            fread_number(fp));
                        KEY("Turbolasers", ship->turbolasers,
                            fread_number(fp));
                        if (!str_cmp(word, "Turret"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = 0;
                                sscanf(line, "%d %d", &x0, &x1);
                                new_turret(ship);
                                ship->last_turret->room =
                                        get_room_index(x0 + fRm);
                                /*
                                 * ship->last_turret->laserstate = x1; 
                                 */
                                ship->last_turret->laserstate = 0;
                                fMatch = TRUE;
                                break;
                        }

                case 'Q':
                        KEY("Quota", ship->quota, fread_number(fp));
                        break;

                        break;
                }
                if (!fMatch)
                {
                        sprintf(buf, "Fread_shipimage: no match: %s", word);
                        bug(buf, 0);
                }
        }
}

void fread_viewshipimage(SHIP_DATA * ship, FILE * fp)
{
        EXT_BV    stuff;
        char     *trash;
        char      buf[MAX_STRING_LENGTH];
        char     *line;
        char     *word;
        bool      fMatch;
        bool      rSet = FALSE;

        /*
         * int dummy_number; 
         */
        int       x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, junk = 0;
        int       fRm = 0, rStart = 0;

        x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;

        ship->act_flags = 0;
        ship->mod_flags = 0;
        xCLEAR_BITS(ship->flags);
        xCLEAR_BITS(ship->aflags);
        ship->first_turret = NULL;
        ship->last_turret = NULL;
        ship->first_hangar = NULL;
        ship->last_hangar = NULL;
        ship->first_escape = NULL;
        ship->last_escape = NULL;
        ship->first_cargo = NULL;
        ship->last_cargo = NULL;

        fRm = 0;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Astro_array", ship->astro_array,
                            fread_number(fp));
                        KEY("Actflags", ship->act_flags, fread_number(fp));
                        KEY("AFlags", ship->aflags, fread_bitvector(fp));
                        break;

                case 'B':
                        KEY("Blasters", ship->blasters, fread_number(fp));
                        break;

                case 'C':
                        KEY("Cockpit", ship->cockpit, fread_number(fp) + fRm);
                        KEY("Coseat", ship->coseat, fread_number(fp) + fRm);
                        KEY("Class", ship->class, fread_number(fp));
                        KEY("Comm", ship->comm, fread_number(fp));
                        KEY("Cloakstatus", ship->cloakstatus,
                            fread_number(fp));
                        if (!str_cmp(word, "Cargo"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = 0;
                                sscanf(line, "%d %d %d %d %d", &x0, &x1, &x2,
                                       &x3, &x4);
                                new_cargo(ship);
                                ship->last_cargo->size = x0;
                                ship->last_cargo->type = x3;
                                ship->last_cargo->amount = x4;
                                fMatch = TRUE;
                                break;
                        }
                        break;


                case 'D':
                        KEY("Description", ship->description,
                            fread_string(fp));
                        KEY("Dockstate", ship->dock_state, fread_number(fp));
                        KEY("Dockroom", ship->dockroom,
                            fread_number(fp) + fRm);
                        break;

                case 'E':
                        KEY("Engineroom", ship->engineroom,
                            fread_number(fp) + fRm);
                        KEY("Entrance", ship->entrance,
                            fread_number(fp) + fRm);
                        if (!str_cmp(word, "Escape"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = 0;
                                sscanf(line, "%d %d %d", &x0, &x1, &x2);
                                if (x0 == 0 && x1 == 0 && x2 == 0)
                                {
                                        continue;
                                }
                                else
                                {
                                        new_escape(ship);
                                        ship->last_escape->room =
                                                get_room_index(x0 + fRm);
                                        ship->last_escape->pods = x1;
                                        ship->last_escape->max_pods = x2;
                                        fMatch = TRUE;
                                        break;
                                }
                        }
                        else if (!str_cmp(word, "End"))
                        {
                                ship->home = STRALLOC("");
                                ship->name = STRALLOC("");
                                ship->owner = STRALLOC("");
                                if (!ship->description)
                                        ship->description = STRALLOC("");
                                ship->copilot = STRALLOC("");
                                ship->pilot = STRALLOC("");
                                ship->homeship = STRALLOC("");
                                if (!ship->ident)
                                        ship->ident = STRALLOC("PS");
                                if (ship->ident[0] == '\0')
                                {
                                        STRFREE(ship->ident);
                                        ship->ident = STRALLOC("PS");
                                }
                                ship->code[0] = number_range(11111, 99999);
                                ship->code[1] = number_range(11111, 99999);
                                ship->code[2] = number_range(11111, 99999);
                                ship->code[3] = number_range(11111, 99999);
                                ship->code[4] = number_range(11111, 99999);
                                ship->code[5] = number_range(11111, 99999);

                                if (ship->maxcharge <= 0)
                                        ship->maxcharge =
                                                10 * (ship->class + 1);

                                if (ship->dock_state != DOCK_NONE)
                                {
                                        ship->dock_install = TRUE;
                                        ship->dock_state = DOCK_READY;
                                }

                                if (ship->missiletubes <= 0)
                                {
                                        ship->missiletubes = 0;
                                        if (ship->maxmissiles > 0
                                            || ship->maxtorpedos > 0
                                            || ship->maxrockets > 0)
                                                ship->missiletubes = 1;
                                }

                                if (ship->shipstate != SHIP_DISABLED)
                                        ship->shipstate = SHIP_DOCKED;
                                if (ship->ion_state != ION_DAMAGED)
                                        ship->ion_state = ION_READY;
                                if (ship->statet0 != LASER_DAMAGED)
                                        ship->statet0 = LASER_READY;
                                if (ship->statet1 != LASER_DAMAGED)
                                        ship->statet1 = LASER_READY;
                                if (ship->statet2 != LASER_DAMAGED)
                                        ship->statet2 = LASER_READY;
                                if (ship->missilestate != MISSILE_DAMAGED)
                                        ship->missilestate = MISSILE_READY;
                                if (ship->shipyard <= 0)
                                        ship->shipyard = ROOM_LIMBO_SHIPYARD;
                                if (ship->lastdoc <= 0)
                                        ship->lastdoc = ship->shipyard;
                                ship->autopilot = FALSE;
                                ship->hatchopen = FALSE;
                                ship->hatchtimer = 0;
                                if (ship->navseat < ship->firstroom
                                    || ship->navseat > ship->lastroom)
                                        ship->navseat = ship->cockpit;
                                if (ship->gunseat < ship->firstroom
                                    || ship->gunseat > ship->lastroom)
                                        ship->gunseat = ship->cockpit;
                                if (ship->coseat < ship->firstroom
                                    || ship->coseat > ship->lastroom)
                                        ship->coseat = ship->cockpit;
                                if (ship->pilotseat < ship->firstroom
                                    || ship->pilotseat > ship->lastroom)
                                        ship->pilotseat = ship->cockpit;
                                ship->starsystem = NULL;
                                ship->energy = ship->maxenergy;
                                ship->charge = ship->maxcharge;
                                ship->hull = ship->maxhull;
                                ship->in_room = NULL;
                                ship->next_in_room = NULL;
                                ship->prev_in_room = NULL;
                                return;
                        }
                        break;

                case 'F':
                        KEY("Flags", ship->flags, fread_bitvector(fp));
                        break;

                case 'G':
                        KEY("Gunseat", ship->gunseat, fread_number(fp) + fRm);
                        break;

                case 'H':
                        KEY("Hyperspeed", ship->hyperspeed, fread_number(fp));
                        if (!str_cmp(word, "Hangar")
                            || !str_cmp(word, "Hanger"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = 0;
                                sscanf(line, "%d %d %d", &x0, &x1, &x2);
                                if (x0 == 0 && x1 == 0 && x2 == 0)
                                {
                                        continue;
                                }
                                else
                                {
                                        new_hangar(ship);
                                        ship->last_hangar->room =
                                                get_room_index(x0 + fRm);
                                        ship->last_hangar->size = x1;
                                        ship->last_hangar->bayopen = FALSE;
                                        fMatch = TRUE;
                                        break;
                                }
                        }
                        break;

                case 'I':
                        KEY("Ident", ship->ident, fread_string(fp));
                        KEY("Ion", ship->ion, fread_number(fp));
                        break;

                case 'L':
                        KEY("Lasers", ship->lasers, fread_number(fp));
                        break;

                case 'M':
                        KEY("Manuever", ship->manuever, fread_number(fp));
                        KEY("Magnetic", ship->mines[MAGNETIC_MINE],
                            fread_number(fp));
                        KEY("Missiletubes", ship->missiletubes,
                            fread_number(fp));
                        KEY("Maxmissiles", ship->maxmissiles,
                            fread_number(fp));
                        KEY("Maxtorpedos", ship->maxtorpedos,
                            fread_number(fp));
                        KEY("Maxrockets", ship->maxrockets, fread_number(fp));
                        KEY("Maxpulses", ship->maxpulses, fread_number(fp));
                        KEY("Maxshield", ship->maxshield, fread_number(fp));
                        KEY("Maxenergy", ship->maxenergy, fread_number(fp));
                        KEY("Maxcharge", ship->maxcharge, fread_number(fp));
                        KEY("Modflags", ship->mod_flags, fread_number(fp));
                        KEY("Maxhull", ship->maxhull, fread_number(fp));
                        KEY("Maxchaff", ship->maxchaff, fread_number(fp));
                        KEY("Maxtroops", ship->maxtroops, fread_number(fp));
                        break;

                case 'N':
                        KEY("Navseat", ship->navseat, fread_number(fp) + fRm);
                        break;

                case 'P':
                        KEY("Pilotseat", ship->pilotseat,
                            fread_number(fp) + fRm);
                        KEY("Proximity", ship->mines[PROXIMITY_MINE],
                            fread_number(fp));
                        break;

                case 'R':
                        if (!str_cmp(word, "ROffset"))
                        {
                                line = fread_line(fp);
                                fMatch = TRUE;
                                break;
                        }
                        if (!str_cmp(word, "RExit"))
                        {
                                line = fread_line(fp);
                                line = fread_line(fp);
                                trash = fread_string(fp);
                                trash = fread_string(fp);
                                stuff = fread_bitvector(fp);
                                fMatch = TRUE;
                                break;
                        }
                        if (!str_cmp(word, "Rooms") && rSet == FALSE)
                        {
                                line = fread_line(fp);
                                sscanf(line, "%d", &ship->lastroom);

                                /*
                                 * Lets make some rooms to work with here 
                                 */
                                rStart = find_room_range(ship->lastroom + 1);
                                if (rStart <= 0)
                                {
                                        bug("fread_viewshipimage: Not enough rooms for spacecraft!", 0);
                                        return;
                                }

                                ship->firstroom = rStart;
                                fRm = ship->firstroom;
                                ship->lastroom += fRm;
                                fMatch = TRUE;
                                rSet = TRUE;
                                break;
                        }
                        KEY("RName", trash, fread_string(fp));
                        KEY("RDesc", trash, fread_string(fp));
                        KEY("RFlags", stuff, fread_bitvector(fp));
                        KEY("RSector", junk, fread_number(fp));
                        KEY("RTunnel", junk, fread_number(fp));
                        KEY("Realspeed", ship->realspeed, fread_number(fp));
                        /*
                         * KEY( "Rooms",          ship->lastroom,        fread_number( fp )+fRm ); 
                         */
                        break;

                case 'S':
                        KEY("Sensor", ship->sensor, fread_number(fp));
                        KEY("Seeker", ship->mines[SEEKER_MINE],
                            fread_number(fp));
                        break;

                case 'T':
                        KEY("Tractorbeam", ship->tractorbeam,
                            fread_number(fp));
                        KEY("Turbolasers", ship->turbolasers,
                            fread_number(fp));
                        if (!str_cmp(word, "Turret"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = 0;
                                sscanf(line, "%d %d", &x0, &x1);
                                new_turret(ship);
                                ship->last_turret->room =
                                        get_room_index(x0 + fRm);
                                /*
                                 * ship->last_turret->laserstate = x1; 
                                 */
                                ship->last_turret->laserstate = 0;
                                fMatch = TRUE;
                                break;
                        }

                case 'Q':
                        KEY("Quota", ship->quota, fread_number(fp));
                        break;

                        break;
                }
                if (!fMatch)
                {
                        sprintf(buf, "Fread_viewshipimage: no match: %s",
                                word);
                        bug(buf, 0);
                }
        }
}

bool shipimage_exist(char *name)
{
        char      strsave[MAX_INPUT_LENGTH];

        /*
         * FILE *fp; 
         */

        sprintf(strsave, "%s/%s", SHIPIMAGE_DIR, name);

        return file_exist(strsave);
}


void do_images(CHAR_DATA * ch, char *argument)
{
        DIR      *directory;
        int       count = 0;
        int       wrap = 0;
        struct dirent *dentry;
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;
        /*
         * Open the directory based on image directory 
         */
        directory = opendir(SHIPIMAGE_DIR);

        /*
         * Set the structure based directory, so we can target files individually 
         */
        if ((dentry = readdir(directory)) == NULL)
        {
                send_to_char("Unable to open directory for images!\n\r", ch);
                return;
        }

        /*
         * Set buf to nothing 
         */
        strncpy(buf, "", MAX_STRING_LENGTH);

        send_to_char("&z&W&YScanning images directory:&z\n\r", ch);
        /*
         * Keep going untel dentry == NULL, this will happen at the readdir function call 
         */
        while (dentry)
        {
                /*
                 * If its not a bad file, like another directory, set it into the list 
                 */
                if (dentry->d_name[0] != '.')
                {
                        strncat(buf, dentry->d_name, MAX_STRING_LENGTH);
                        strncat(buf, "        ", MAX_STRING_LENGTH);
                        count++;
                        if (++wrap > 3)
                        {
                                strncat(buf, "\n\r", MAX_STRING_LENGTH);
                                wrap = 0;
                        }
                }
                dentry = readdir(directory);
        }
        strncat(buf, "\n\r", MAX_STRING_LENGTH);
        if (count < 0)
        {
                send_to_char
                        ("No images found. Contact Orion immediately.\n\r",
                         ch);
                return;
        }
        closedir(directory);
        send_to_char(buf, ch);
        if (count > 1)
                ch_printf(ch,
                          "&YImages folder scanned, found a total of &W%d &Yimages.\n\r",
                          count);
        return;
}

#endif
