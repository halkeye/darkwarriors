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
 *                $Id: installations.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "body.h"
#include "installations.h"
#include "space2.h"

INSTALLATION_DATA *first_installation;
INSTALLATION_DATA *last_installation;

/* Local Functions */

void fread_installations args((INSTALLATION_DATA * installation, FILE * fp));
bool load_installation_file args((char *installationfile));
void write_installation_list args((void));
void save_installations args((INSTALLATION_DATA * installation));

#define  NULLSTR( str )  ( str == NULL || str[0] == '\0' )

const struct installation_type installation_table[MAX_INSTALLATION] = {
        /*
         * Installation Name   Shields     Ions     Turbo    Rooms    Name of main room 
         */
        {"Battery", 3, 0, 0, 5, "Shield Generator"},
        {"Ion", 0, 3, 0, 5, "Ionic Generator"},
        {"Turbo Laser", 0, 0, 3, 5, "Laser Cannon"},
        {"Research", 0, 0, 0, 10, "Research Lab"},
        {"Head Quarters", 0, 0, 0, 100, "Office of the Leader"},
        {"Training", 0, 0, 0, 10, "War Room"},
        {"Government", 0, 0, 0, 50, "Conference Room"},
        {"Ship Yard", 0, 0, 0, 20, "Landing Pad"},
        {"Mining", 0, 0, 0, 10, "Shaft End"}
};

void write_installation_list()
{
        INSTALLATION_DATA *tinstallation;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", INSTALLATIONS_DIR, INSTALLATION_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open installations.lst for writing!\n\r",
                    0);
                return;
        }
        for (tinstallation = first_installation; tinstallation;
             tinstallation = tinstallation->next)
                fprintf(fpout, "%s\n", tinstallation->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

void save_installations(INSTALLATION_DATA * installation)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];

        if (!installation)
        {
                bug("ERROR in save_installations: null installation pointer!",
                    0);
                return;
        }

        if (!installation->filename || installation->filename[0] == '\0')
        {
                snprintf(buf, MSL,
                         "ERROR in save_installations: installation starting at %d has no filename",
                         installation->first_room);
                bug(buf, 0);
                return;
        }

        snprintf(filename, MSL, "%s%s", INSTALLATIONS_DIR,
                 installation->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_installation: fopen", 0);
                perror(filename);
        }
        else
        {
                DOCK_DATA *dock;

                fprintf(fp, "#INSTALLATION\n");
                fprintf(fp, "Planet       %s~\n", installation->planet->name);
                fprintf(fp, "Clan         %s~\n", installation->clan->name);
                fprintf(fp, "Filename     %s~\n", installation->filename);
                fprintf(fp, "Firstroom    %d\n", installation->first_room);
                fprintf(fp, "Lastroom     %d\n", installation->last_room);
                fprintf(fp, "Lastbuilt    %d\n", installation->last_built);
                fprintf(fp, "Type         %d\n", installation->type);
                fprintf(fp, "Flags        %d\n", installation->flags);
                fprintf(fp, "Mainroom     %d\n", installation->mainroom);
                fprintf(fp, "End\n\n");

                for (dock = installation->first_dock; dock;
                     dock = dock->next_in_installation)
                        fwrite_dock(fp, dock);

                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void fread_installations(INSTALLATION_DATA * installation, FILE * fp)
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

                case 'C':
                        KEY("Clan", installation->clan,
                            get_clan(fread_string_noalloc(fp)));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!installation->type)
                                        installation->type = 0;
                                installation->timer = 0;
                                installation->weapon_time = 8;
                                /*
                                 * if ( installation->type == SHIPYARD_INSTALLATION && installation->mainroom > 0)
                                 * makedock( installation ); 
                                 */
                                return;
                        }
                        break;

                case 'F':
                        KEY("Filename", installation->filename,
                            fread_string(fp));
                        KEY("Flags", installation->flags, fread_number(fp));
                        KEY("Firstroom", installation->first_room,
                            fread_number(fp));
                        break;

                case 'L':
                        KEY("Lastroom", installation->last_room,
                            fread_number(fp));
                        KEY("Lastbuilt", installation->last_built,
                            fread_number(fp));
                        break;

                case 'M':
                        KEY("Mainroom", installation->mainroom,
                            fread_number(fp));
                        break;

                case 'P':
                        if (!str_cmp(word, "Planet"))
                        {
                                PLANET_DATA *planet =
                                        get_planet(fread_string_noalloc(fp));
                                if (planet)
                                {
                                        installation->planet = planet;
                                        LINK(installation,
                                             planet->first_install,
                                             planet->last_install,
                                             next_on_planet, prev_on_planet);
                                }
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'T':
                        KEY("Type", installation->type, fread_number(fp));
                        break;
                }

                if (!fMatch)
                        bug("Fread_installations: no match: %s", word);
        }
}

bool load_installation_file(char *installationfile)
{
        char      filename[256];
        INSTALLATION_DATA *installation;
        FILE     *fp;
        bool      found;

        CREATE(installation, INSTALLATION_DATA, 1);
        installation->filename = STRALLOC("");

        found = FALSE;
        snprintf(filename, MSL, "%s%s", INSTALLATIONS_DIR, installationfile);

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
                                bug("Load_installation_file: # not found.",
                                    0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "INSTALLATION"))
                        {
                                fread_installations(installation, fp);
                        }
                        else if (!str_cmp(word, "DOCK"))
                        {
                                DOCK_DATA *dock;

                                CREATE(dock, DOCK_DATA, 1);
                                fread_dock(dock, fp);
                                DISPOSE(dock);
                                bug("found an installation dock");
                                continue;
                                dock->temporary = 1;
                                dock->body = installation->planet->body;
                                LINK(dock, first_dock, last_dock, next, prev);
                                installation->planet->body->add_dock(dock);
                                LINK(dock, installation->first_dock,
                                     installation->last_dock,
                                     next_in_installation,
                                     prev_in_installation);
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_installation_file: bad section: %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (found)
                LINK(installation, first_installation, last_installation,
                     next, prev);
        else
                DISPOSE(installation);
        return found;
}


void load_installations()
{
        FILE     *fpList;
        const char *filename;
        char      installationlist[256];
        char      buf[MAX_STRING_LENGTH];

        first_installation = NULL;
        last_installation = NULL;

        snprintf(installationlist, MSL, "%s%s", INSTALLATIONS_DIR,
                 INSTALLATION_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(installationlist, "r")) == NULL)
        {
                perror(installationlist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_installation_file((char *) filename))
                {
                        snprintf(buf, MSL,
                                 "Cannot load installation file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done installations ");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

int reserve_rooms_installation(int firstroom, int numrooms)
{
        AREA_DATA *tarea;
        ROOM_INDEX_DATA *room;
        int       i;

        /*
         * This should be done once in um, db.c? or something that db.c calls 
         */

        for (tarea = first_area; tarea; tarea = tarea->next)
                if (!str_cmp(INSTALLATION_AREA, tarea->filename))
                        break;

        for (i = firstroom; i < firstroom + numrooms; i++)
        {
                room = make_room(i, tarea);
                if (!room)
                {
                        bug("reserve_rooms: make_room failed");
                        return -1;
                }
                room->area = tarea;
                xSET_BIT(room->room_flags, ROOM_INSTALLATION);
                xSET_BIT(room->room_flags, ROOM_NO_MOB);
                xSET_BIT(room->room_flags, ROOM_INDOORS);
        }
        fold_area(tarea, tarea->filename, TRUE, FALSE);
        return i;
}

INSTALLATION_DATA *installation_from_room(int vnum)
{
        INSTALLATION_DATA *installation;

        for (installation = first_installation; installation;
             installation = installation->next)
                if (vnum >= installation->first_room
                    && vnum <= installation->last_room)
                        return installation;
        return NULL;
}

int planetary_installations(PLANET_DATA * planet, int type)
{
        INSTALLATION_DATA *installation;
        int       count = 0;

        for (installation = first_installation; installation;
             installation = installation->next)
                if (installation->planet == planet
                    && installation->type == type
                    && installation->mainroom > 0)
                        count++;
        return count;
}

void destroy_installation(INSTALLATION_DATA * installation)
{
        ROOM_INDEX_DATA *room;
        AREA_DATA *area;
        AREA_DATA *parea;
        int       roomnum;
        char      file[MAX_STRING_LENGTH];
        EXIT_DATA *xit;
        EXIT_DATA *pexit;
        CHAR_DATA *rch;
        OBJ_DATA *robj;

        if (!installation)
                return;
        if ((room = get_room_index(installation->first_room)) == NULL)
                return;

        if ((area = room->area) == NULL)
                return;

        for (roomnum = installation->first_room;
             roomnum <= installation->last_room; roomnum++)
        {
                room = get_room_index(roomnum);
                if (room != NULL)
                {
                        if (area->first_reset)
                                wipe_resets(area, room);
                        rch = room->first_person;
                        while (rch)
                        {
                                if (IS_IMMORTAL(rch))
                                {
                                        char_from_room(rch);
                                        char_to_room(rch,
                                                     get_room_index(wherehome
                                                                    (rch)));
                                }
                                else
                                        raw_kill(rch, rch);
                                rch = room->first_person;
                        }
                        for (robj = room->first_content; robj;
                             robj = robj->next_content)
                        {
                                separate_obj(robj);
                                extract_obj(robj);
                        }
                }
        }
        for (xit = get_room_index(installation->first_room)->first_exit; xit;
             xit = xit->next)
        {
                if (xit->to_room->vnum > installation->last_room
                    || xit->to_room->vnum < installation->first_room)
                {
                        parea = xit->to_room->area;
                        if (((pexit = xit->rexit) != NULL) && pexit != xit)
                        {
                                extract_exit(xit->to_room, pexit);
                                fold_area(parea, parea->filename, FALSE,
                                          FALSE);
                        }
                }
        }

        for (roomnum = installation->first_room;
             roomnum <= installation->last_room; roomnum++)
        {
                if ((room = get_room_index(roomnum)) == NULL)
                        continue;
                clean_room(room);
                delete_room(room);
                room = NULL;
        }

        fold_area(area, area->filename, FALSE, FALSE);

        snprintf(file, MSL, "%s%s", INSTALLATIONS_DIR,
                 installation->filename);

        if (installation->filename)
                STRFREE(installation->filename);
        if (installation->clan)
                installation->clan = NULL;
        if (installation->planet)
                installation->planet = NULL;
        installation->first_room = 0;
        installation->last_room = 0;
        installation->flags = 0;
        installation->last_built = 0;
        installation->type = 0;
        installation->mainroom = 0;
        installation->timer = 0;
        installation->weapon_time = 0;
        remove(file);

        UNLINK(installation, first_installation, last_installation, next,
               prev);
        DISPOSE(installation);
        installation = NULL;
        write_installation_list();
        return;
}

void echo_to_installation(sh_int AT_COLOR, char *argument,
                          INSTALLATION_DATA * installation)
{
        DESCRIPTOR_DATA *d;

        if (!argument || argument[0] == '\0')
                return;

        for (d = first_descriptor; d; d = d->next)
        {
                /*
                 * Added showing echoes to players who are editing, so they won't
                 * * miss out on important info like upcoming reboots. --Narn 
                 */
                if ((IS_PLAYING(d) || d->connected == CON_EDITING)
                    &&
                    ((d->character->in_room->vnum >= installation->first_room)
                     && (d->character->in_room->vnum <=
                         installation->last_room)))
                {
                        set_char_color(AT_COLOR, d->character);
                        send_to_char(argument, d->character);
                        send_to_char("\n\r", d->character);
                }
        }
        return;
}


bool mob_reset(CHAR_DATA * ch, char *type, bool check)
{
        ROOM_INDEX_DATA *room;
        OBJ_DATA *obj;

        if (!IS_NPC(ch) && !check)
        {
                send_to_char("You cannot do that. What the hell? Tell Greven",
                             ch);
                return FALSE;
        }

        room = ch->in_room;

        if (!str_cmp(type, "installation"))
        {
                if (installation_from_room(ch->in_room->vnum) == NULL)
                        return FALSE;
        }

        else if (!str_cmp(type, "ship"))
                if (ship_from_room(ch->in_room->vnum) == NULL)
                        return FALSE;

        if (!check)
	    {
                add_reset(ch->in_room->area, 'M', 1, ch->pIndexData->vnum,
                          ch->pIndexData->count, ch->in_room->vnum);
                for (obj = ch->first_carrying; obj; obj = obj->next_content)
                {
                    if (obj->wear_loc == WEAR_NONE)
                        add_obj_reset(ch->in_room->area, 'G', obj, 1, 0);
                    else
                        add_obj_reset(ch->in_room->area, 'E', obj, 1,
                                      obj->wear_loc);
                }
        }
        return TRUE;

}

CMDF do_makeinstallation(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MSL];
        int       percentage;
        bool      checktool, checkcir, checksuper, checkbatt;
        ROOM_INDEX_DATA *room;
        OBJ_DATA *obj;
        PLANET_DATA *planet;
        int       vnum, duracrete, electronics, cost, iInstall, type =
                0, count;
        CLAN_DATA *clan;
        INSTALLATION_DATA *installation;
        EXIT_DATA *pexit;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (IS_NPC(ch))
                return;

        if ((clan = ch->pcdata->clan) == NULL || !ch->in_room->area->planet
            || ch->in_room->area->planet->governed_by != clan)
        {
                send_to_char("You do not belong to this planet's government.",
                             ch);
                return;
        }

        if (clan->funds < 5000)
        {
                send_to_char
                        ("Your clan doesn't have enough money to attempt this.",
                         ch);
                return;
        }

        planet = ch->in_room->area->planet;
        if (!planet)
        {
                send_to_char
                        ("&RYou are not in a valid place to build that. Try a large planet.&C&w",
                         ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_INDOORS) ||
            xIS_SET(ch->in_room->room_flags, ROOM_SPACECRAFT))
        {
                send_to_char("You can't build that here!! Try elsewhere.\r\n",
                             ch);
                return;
        }


        if ((ch->pcdata && ch->pcdata->bestowments &&
             is_name("installations", ch->pcdata->bestowments)) ||
            !str_cmp(ch->name, clan->leader) ||
            !str_cmp(ch->name, clan->number1) ||
            !str_cmp(ch->name, clan->number2))
                ;
        else
        {
                send_to_char
                        ("Your clan has not empowered you with that ability.\n\r",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                if (NULLSTR(arg1) || NULLSTR(arg2))
                {
                        send_to_char
                                ("&RUsage: &Gmakeinstallation &C<&ctype&C> <&cdirection&C>&w\r\n",
                                 ch);
                        buf[0] = '\0';
                        mudstrlcat(buf, "&RTypes:&w", MSL);
                        for (iInstall = 0; iInstall < MAX_INSTALLATION - 1;
                             iInstall++)
                        {
                                if (installation_table[iInstall].
                                    installation_name
                                    && installation_table[iInstall].
                                    installation_name[0] != '\0')
                                {
                                        mudstrlcat(buf, "\t", MSL);
                                        mudstrlcat(buf,
                                                   installation_table
                                                   [iInstall].
                                                   installation_name, MSL);
                                        mudstrlcat(buf, "\n\r", MSL);
                                }
                        }
                        send_to_char(buf, ch);
                        return;
                }

                for (iInstall = 0; iInstall < MAX_INSTALLATION - 1;
                     iInstall++)
                {
                        if (!str_prefix
                            (arg1,
                             installation_table[iInstall].installation_name))
                        {
                                type = iInstall;
                                break;
                        }
                }
                if (iInstall == MAX_INSTALLATION ||
                    !installation_table[iInstall].installation_name ||
                    installation_table[iInstall].installation_name[0] == '\0')
                {
                        send_to_char
                                ("That is not an installation, your choices are:\n\r",
                                 ch);
                        buf[0] = '\0';
                        mudstrlcat(buf, "&RTypes:&w", MSL);
                        for (iInstall = 0; iInstall < MAX_INSTALLATION - 1;
                             iInstall++)
                        {
                                if (installation_table[iInstall].
                                    installation_name
                                    && installation_table[iInstall].
                                    installation_name[0] != '\0')
                                {
                                        mudstrlcat(buf, "\t", MSL);
                                        mudstrlcat(buf,
                                                   installation_table
                                                   [iInstall].
                                                   installation_name, MSL);
                                        mudstrlcat(buf, "\n\r", MSL);
                                }
                        }
                        send_to_char(buf, ch);
                        return;
                }

                if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north") ||
                    !str_cmp(arg2, "s") || !str_cmp(arg2, "south") ||
                    !str_cmp(arg2, "e") || !str_cmp(arg2, "east") ||
                    !str_cmp(arg2, "w") || !str_cmp(arg2, "west") ||
                    !str_cmp(arg2, "sw") || !str_cmp(arg2, "southwest") ||
                    !str_cmp(arg2, "nw") || !str_cmp(arg2, "northwest") ||
                    !str_cmp(arg2, "se") || !str_cmp(arg2, "southeast") ||
                    !str_cmp(arg2, "ne") || !str_cmp(arg2, "northeast") ||
                    !str_cmp(arg2, "u") || !str_cmp(arg2, "up") ||
                    !str_cmp(arg2, "d") || !str_cmp(arg2, "down"))
                {
                        if (get_exit(ch->in_room, get_dir(arg2)))
                        {
                                send_to_char
                                        ("There is already an exit in that direction\r\n",
                                         ch);
                                return;
                        }
                }
                else
                {
                        send_to_char("Not a valid direction\r\n", ch);
                        return;
                }

                checktool = FALSE;
                checkcir = FALSE;
                checksuper = FALSE;
                checkbatt = FALSE;

                duracrete = 30 * installation_table[type].rooms;
                electronics = 10 * installation_table[type].rooms;

                if (planet->resource[CARGO_DURACRETE] < duracrete)
                {
                        send_to_char
                                ("&RYou'll have to wait till this planet either import or produce more duracrete&C&w\r\n",
                                 ch);
                        return;
                }

                if (planet->resource[CARGO_ELECTRONICS] < electronics)
                {
                        send_to_char
                                ("&RYou'll have to wait till this planet either import or produce more electronics&C&w\r\n",
                                 ch);
                        return;
                }

                cost = 10;
                if (planet->cargoimport[CARGO_DURACRETE] > 0)
                        cost += planet->cargoimport[CARGO_DURACRETE] +
                                planet->cargoimport[CARGO_DURACRETE] / 2;
                else if (planet->cargoexport[CARGO_DURACRETE] > 0)
                        cost += planet->cargoexport[CARGO_DURACRETE];
                else
                        cost += 10;

                if (planet->cargoimport[CARGO_ELECTRONICS] > 0)
                        cost += planet->cargoimport[CARGO_ELECTRONICS] +
                                planet->cargoimport[CARGO_ELECTRONICS] / 2;
                else if (planet->cargoexport[CARGO_ELECTRONICS] > 0)
                        cost += planet->cargoexport[CARGO_ELECTRONICS];
                else
                        cost += 10;

                cost *= duracrete + electronics;
                if (clan->funds < cost)
                {
                        send_to_char
                                ("&RYour clan can't afford the materials to build that.\r\n",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcir = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checksuper = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RI'd like to see you build a installation with no tools.\r\n",
                                 ch);
                        return;
                }
                if (!checkcir)
                {
                        send_to_char
                                ("&RYou could really use a circuit to for the control systems.\r\n",
                                 ch);
                        return;
                }
                if (!checksuper)
                {
                        send_to_char
                                ("&RSuch advanced circuitry requires a superconducter to work properly.\r\n",
                                 ch);
                        return;
                }
                if (!checkbatt)
                {
                        send_to_char
                                ("&RYou must have a battery to power your conductor.\r\n",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makebase]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of starting an installation.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and starts the production of an installation.\r\n",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 35, do_makeinstallation,
                                  1);
                        ch->dest_buf = str_dup(arg1);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;

                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makebase);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }

                mudstrlcpy(arg1, (char *) ch->dest_buf, MIL);
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

        checktool = FALSE;
        checkcir = FALSE;
        checksuper = FALSE;
        checkbatt = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_CIRCUIT && checkcir == FALSE)
                {
                        checkcir = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR
                    && checksuper == FALSE)
                {
                        checksuper = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
                {
                        checkbatt = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makebase]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkbatt)
            || (!checksuper) || (!checkcir))
        {
                send_to_char
                        ("&RYou look around at the enterance to your new installation.\n\r",
                         ch);
                send_to_char
                        ("&RYou smile with satisfaction, and notice something on the floor.\n\r",
                         ch);
                send_to_char
                        ("&RBits of dust fall from the ceiling, and you jump back as the ceiling collapses.\n\r",
                         ch);
                send_to_char
                        ("&RYou quickly hire someone to dispose of the rubble.\n\r",
                         ch);
                clan->funds -= 5000;
                send_to_char("&BYou pay the worker 5000 credits.\n\r", ch);
                learn_from_failure(ch, gsn_makebase);
                return;
        }

        for (iInstall = 0; iInstall < MAX_INSTALLATION; iInstall++)
        {
                if (toupper(arg1[0]) ==
                    toupper(installation_table[iInstall].installation_name[0])
                    && !str_prefix(arg1,
                                   installation_table[iInstall].
                                   installation_name))
                {
                        type = iInstall;
                        break;
                }
        }
        if (iInstall == MAX_INSTALLATION - 1 ||
            !installation_table[iInstall].installation_name ||
            installation_table[iInstall].installation_name[0] == '\0')
        {
                send_to_char
                        ("That is not an installation, your choices are:\n\r",
                         ch);
                send_to_char("&RTypes:&w", ch);

                for (iInstall = 0; iInstall < MAX_INSTALLATION - 1;
                     iInstall++)
                {
                        if (installation_table[iInstall].installation_name &&
                            installation_table[iInstall].
                            installation_name[0] != '\0')
                                ch_printf(ch, "\t%s\n\r",
                                          installation_table[iInstall].
                                          installation_name);
                }
                return;
        }

        duracrete = 30 * installation_table[type].rooms;
        electronics = 10 * installation_table[type].rooms;

        if (planet->resource[CARGO_DURACRETE] < duracrete)
        {
                send_to_char
                        ("&RYou'll have to wait till this planet either import or produce more duracrete&C&w\r\n",
                         ch);
                return;
        }

        if (planet->resource[CARGO_ELECTRONICS] < electronics)
        {
                send_to_char
                        ("&RYou'll have to wait till this planet either import or produce more electronics&C&w\r\n",
                         ch);
                return;
        }

        cost = 10;
        if (planet->cargoimport[CARGO_DURACRETE] > 0)
                cost += planet->cargoimport[CARGO_DURACRETE] +
                        planet->cargoimport[CARGO_DURACRETE] / 2;
        else if (planet->cargoexport[CARGO_DURACRETE] > 0)
                cost += planet->cargoexport[CARGO_DURACRETE];
        else
                cost += 10;

        if (planet->cargoimport[CARGO_ELECTRONICS] > 0)
                cost += planet->cargoimport[CARGO_ELECTRONICS] +
                        planet->cargoimport[CARGO_ELECTRONICS] / 2;
        else if (planet->cargoexport[CARGO_ELECTRONICS] > 0)
                cost += planet->cargoexport[CARGO_ELECTRONICS];
        else
                cost += 10;

        cost *= duracrete + electronics;
        if (clan->funds < cost)
        {
                send_to_char
                        ("&RYour clan can't afford the materials to build that.\r\n",
                         ch);
                return;
        }

        clan->funds -= cost;
        save_clan(clan);
        planet->resource[CARGO_DURACRETE] -= duracrete;
        planet->resource[CARGO_ELECTRONICS] -= electronics;

        vnum = find_pvnum_block(installation_table[type].rooms,
                                INSTALLATION_AREA);
        if (vnum < 0)
        {
                bug("player installation area out of vnums", 0);
                send_to_char("Not enough vnums report to a coder.\r\n", ch);
                return;
        }

        if ((count =
             reserve_rooms_installation(vnum,
                                        installation_table[type].rooms)) < 0)
        {
                bug("do_makeinstallation: reserve_rooms failed", 0);
                send_to_char
                        ("Couldn't build your rooms. Report to Greven.\r\n",
                         ch);
                return;
        }
        CREATE(installation, INSTALLATION_DATA, 1);
        LINK(installation, first_installation, last_installation, next, prev);
        stralloc_printf(&installation->filename, "%d.pinstall", vnum);
        installation->clan = clan;
        installation->flags = 0;
        installation->first_room = vnum;
        installation->last_room = vnum + installation_table[type].rooms - 1;
        installation->last_built = vnum;
        installation->planet = planet;
        installation->type = type;
        installation->timer = 0;
        installation->weapon_time = 8;

        ch_printf(ch,
                  "&GYour clan pays a total of &R%d&G credits for the construction of this installation.\r\n",
                  cost);
        learn_from_success(ch, gsn_makebase);
        act(AT_PLAIN,
            "$n finishes constructing the entrance to the installation, and enters.",
            ch, NULL, argument, TO_ROOM);
        room = get_room_index(vnum);
        if (!room)
        {
                bug("makeinstallation..no such room", 0);
                return;
        }

        stralloc_printf(&room->name, "Entrance to a %s Facility",
                        installation_table[type].installation_name);

        if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north") ||
            !str_cmp(arg2, "s") || !str_cmp(arg2, "south") ||
            !str_cmp(arg2, "e") || !str_cmp(arg2, "east") ||
            !str_cmp(arg2, "w") || !str_cmp(arg2, "west") ||
            !str_cmp(arg2, "sw") || !str_cmp(arg2, "southwest") ||
            !str_cmp(arg2, "nw") || !str_cmp(arg2, "northwest") ||
            !str_cmp(arg2, "se") || !str_cmp(arg2, "southeast") ||
            !str_cmp(arg2, "ne") || !str_cmp(arg2, "northeast") ||
            !str_cmp(arg2, "u") || !str_cmp(arg2, "up") ||
            !str_cmp(arg2, "d") || !str_cmp(arg2, "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg2)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
                else
                {
                        pexit = make_exit(ch->in_room, room, get_dir(arg2));
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                        pexit = make_exit(room, ch->in_room,
                                          rev_dir[get_dir(arg2)]);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }
        }

        else
        {
                send_to_char("Not a valid direction\r\n", ch);
                return;
        }

        fold_area(room->area, room->area->filename, TRUE, FALSE);
        fold_area(ch->in_room->area, ch->in_room->area->filename, TRUE,
                  FALSE);
        save_installations(installation);
        write_installation_list();
        char_from_room(ch);
        char_to_room(ch, get_room_index(vnum));
        ch_printf(ch, "&GYou enter your new installation.&R&W\r\n");
        {
                long      xpgain;

                /*
                 * Another XP BUG HERE 
                 */
                xpgain = UMIN(installation_table[type].rooms * 1000,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }
}

CMDF do_istat(CHAR_DATA * ch, char *argument)
{
        INSTALLATION_DATA *installation;
        PLANET_DATA *planet = NULL;
        int       count = 0;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] != '\0')
                planet = get_planet(argument);
        if (!planet)
                planet = ch->in_room->area->planet;

		installation = installation_from_room(ch->in_room->vnum);

        if (installation && installation->planet)
        {
			planet = installation->planet;
        }

        if (!planet)
        {
                send_to_char
                        ("You are not on a planet, and only planets may have installations.\n\r",
                         ch);
                return;
        }


        if (!installation)
        {
                send_to_char
                        ("There is no installation in this room.\n\rListed are the installations on this planet:\n\r",
                         ch);
                for (installation = planet->first_install; installation;
                     installation = installation->next_on_planet)
                {
                        count++;
                        ch_printf(ch,
                                  "&BFirst Room:&w %d &BLast Room:&w %d &BType:&w %s\n\r",
                                  installation->first_room,
                                  installation->last_room,
                                  installation_table[installation->type].
                                  installation_name);
                }
                ch_printf(ch,
                          "&BThere are a total of &R%d&B installations on this planet.",
                          count);
                return;
        }

        send_to_char("&BI&znstallation &BD&zata\n\r", ch);
        send_to_char("&B------------------------------------------------\n\r",
                     ch);
        ch_printf(ch, "&BI&znstallation &BT&zype:&w       %s\n\r",
                  installation_table[installation->type].installation_name);
        ch_printf(ch, "&BI&znstallation &BF&zilename:&w   %s\n\r",
                  installation->filename);
        ch_printf(ch, "&BI&znstallation &BF&zirstroom:&w  %d\n\r",
                  installation->first_room);
        ch_printf(ch, "&BI&znstallation &BL&zastroom: &w  %d\n\r",
                  installation->last_room);
        ch_printf(ch, "&BI&znstallation &BM&zainroom: &w  %d\n\r",
                  installation->mainroom);
        ch_printf(ch, "&BI&znstallation &BL&zastbuilt:&w  %d\n\r",
                  installation->last_built);
        ch_printf(ch, "&BI&znstallation &BP&zlanet:&w     %s\n\r",
                  installation->planet->name);
		if (installation->clan)
            ch_printf(ch, "&BI&znstallation &BC&zlan:&w       %s\n\r",
                  installation->clan->name);
		else
            send_to_char("&BI&znstallation &BC&zlan:&w       none\n\r",ch);
        ch_printf(ch, "&BI&znstallation &BS&zhields:&w    %d\n\r",
                  installation_table[installation->type].shields);
        ch_printf(ch, "&BI&znstallation &BT&zurbolasers:&w%d\n\r",
                  installation_table[installation->type].turbo);
        ch_printf(ch, "&BI&znstallation &BI&zons:&w       %d\n\r",
                  installation_table[installation->type].ions);
        ch_printf(ch, "&BI&znstallation &BT&zimer:&w      %d\n\r",
                  installation->timer);
        ch_printf(ch, "&BI&znstallation &BW&zweapons:&w   %d\n\r",
                  installation->weapon_time);
        ch_printf(ch, "&BR&zooms &BL&zeft:&w              %d\n\r",
                  (installation->last_room - installation->last_built));
        return;
}

CMDF do_addpersonel(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       percentage, credits;
        INSTALLATION_DATA *installation;

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (ch->pcdata->clan == NULL)
        {
                send_to_char("You are not in a clan.", ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                argument = one_argument(argument, arg);

                if (arg[0] == '\0')
                {
                        send_to_char
                                ("What kind of personel do you want to add?",
                                 ch);
                        return;
                }

                if ((ch->pcdata->bestowments
                     && is_name("installations", ch->pcdata->bestowments))
                    || !str_cmp(ch->name, ch->pcdata->clan->leader)
                    || !str_cmp(ch->name, ch->pcdata->clan->number1)
                    || str_cmp(ch->name, ch->pcdata->clan->number2));
                else
                {
                        send_to_char
                                ("Your clan has not empowered you with that ability.\n\r",
                                 ch);
                        return;
                }
                if (ch->backup_wait)
                {
                        send_to_char
                                ("&RYou are already trying to fortify this place.\n\r",
                                 ch);
                        return;
                }

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) != NULL)
                {
                        if (installation->clan != ch->pcdata->clan)
                        {
                                send_to_char
                                        ("&RYou can not place personel in another clans installation.\n\r",
                                         ch);
                                return;
                        }
                }

                if (!mob_reset(ch, "installation", TRUE))
                {
                        send_to_char
                                ("&RCannot place personel into a room with other NPC's or objects already in it!\n\r",
                                 ch);
                        send_to_char
                                ("&RNote: this will be checked several times, and the resets will not be installed if this case happens.\n\r",
                                 ch);
                        return;
                }

				if (str_cmp(arg, "entranceguard") && str_cmp(arg, "guard") && str_cmp(arg, "customs") && str_cmp(arg, "doctor"))
				{
					send_to_char("You must specific what type of personel.\n\r", ch);
					send_to_char("Options are:\n\r\tentranceguard\n\r\tguard\n\r\tcustoms\n\r\tdoctor\n\r", ch);
					return;
				}
                if (ch->pcdata->clan->funds <
                    ch->skill_level[LEADERSHIP_ABILITY] * 30)
                {
                        ch_printf(ch, "&RYou dont have enough credits.\n\r",
                                  ch);
                        return;
                }

                percentage = (int) (ch->pcdata->learned[gsn_addpersonel]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin to arrange for personel.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n begins issuing orders into $s comlink.", ch,
                            NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_addpersonel, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou were not able to arrange for personel.\n\r",
                         ch);
                learn_from_failure(ch, gsn_addpersonel);
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
                        ("&RYou are interupted before you can finish your arrangements.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        send_to_char
                ("&GThe arrangement have been made. They will be here shortly.\n\r",
                 ch);

        /*
         * Its actually better todo this with no money? ... 
         */
        credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
        ch_printf(ch, "It cost you %d credits.\n\r", credits);
        ch->pcdata->clan->funds -= UMIN(credits, ch->pcdata->clan->funds);

        learn_from_success(ch, gsn_addpersonel);

        if (!str_cmp("guard", arg))
        {
                ch->backup_mob = MOB_VNUM_INSTALL_GUARD;
                add_reinforcements(ch);
                return;
        }
        if (!str_cmp("entranceguard", arg))
        {
                ch->backup_mob = MOB_VNUM_INSTALL_ENTERANCE_GUARD;
                add_reinforcements(ch);
                return;
        }
        if (!str_cmp("doctor", arg))
        {
                ch->backup_mob = MOB_VNUM_INSTALL_DOCTOR;
                add_reinforcements(ch);
                return;
        }
        if (!str_cmp("customs", arg))
        {
                ch->backup_mob = MOB_VNUM_INSTALL_CUSTOMS;
                add_reinforcements(ch);
                return;
        }
}

CMDF do_lockdoor(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *xit;
        EXIT_DATA *pexit;
        char      arg[MAX_INPUT_LENGTH];
        int       level, percentage, lock;
        bool      checktool, checkdura;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_DATA *obj;
        int       vnum;
        INSTALLATION_DATA *installation;
        SHIP_DATA *ship;

        argument = one_argument(argument, arg);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char("&RUsage: lockdoor <direction>\n\r&w",
                                     ch);
                        return;
                }

                if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
                    !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
                    !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
                    !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
                    !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
                    !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
                    !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
                    !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
                    !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
                    !str_cmp(arg, "d") || !str_cmp(arg, "down"))
                {
                        if (!get_exit(ch->in_room, get_dir(arg)))
                        {
                                send_to_char
                                        ("There is no an exit in that direction\r\n",
                                         ch);
                                return;
                        }
                }

                else
                {
                        send_to_char("Not a valid direction\r\n", ch);
                        return;
                }


                checktool = FALSE;
                checkdura = FALSE;

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a new door and lock.\n\r",
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

                if ((pexit = get_exit(ch->in_room, get_door(arg))) != NULL && IS_SET(pexit->exit_info, EX_LOCKED))
                {
                         act(AT_GREY, "The exit $t is already locked.", ch, dir_name[get_door(arg)], NULL, TO_CHAR);
                         return;
                }

				if (!ship_from_room(ch->in_room->vnum) && !installation_from_room(ch->in_room->vnum) ) 
				{
					send_to_char("&RThis exit is too visible to modify like this, try a ship or installation.\n\r", ch);
					return;
				}


                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_lockdoor]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating a locked door.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_lockdoor, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_lockdoor);
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
                                                    learned[gsn_lockdoor]);
        vnum = 10438;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_lockdoor]);

        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura))
        {
                send_to_char
                        ("&RYou stand there and try to check the lock on this door you installed.\n\r",
                         ch);
                send_to_char
                        ("&RYou check the keypad to makesure everything is set.\n\r",
                         ch);
                send_to_char
                        ("&RAs you press the keys, it shorts out and fuses the circuits.\n\r",
                         ch);
                learn_from_failure(ch, gsn_lockdoor);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_KEY;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 1;
        stralloc_printf(&obj->name, "key card to door in %s",
                        smash_color(ch->in_room->name));
        stralloc_printf(&obj->short_descr, "key card to door in %s",
                        ch->in_room->name);
        stralloc_printf(&obj->description, "%s", "A key card was left here.");
        lock = number_range(1, 2000000);
        obj->value[0] = lock;
        if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
            !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
            !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
            !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
            !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
            !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
            !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
            !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
            !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
            !str_cmp(arg, "d") || !str_cmp(arg, "down"))
        {
                if ((xit = find_door(ch, arg, TRUE)) == NULL)
                {
                        send_to_char
                                ("There is already an exit in that direction, but this should not have happened. Talk to Greven.\r\n",
                                 ch);
                        return;
                }
                else
                {
                        set_bexit_flag(xit, EX_ISDOOR);
                        set_bexit_flag(xit, EX_CLOSED);
                        set_bexit_flag(xit, EX_LOCKED);
                        xit->key = lock;
                        if ((pexit = xit->rexit) != NULL && pexit != xit)
                                pexit->key = lock;
                }
        }

        else
        {
                send_to_char
                        ("Not a valid direction, but this should not have happened. Talk to Greven.\r\n",
                         ch);
                return;
        }
        obj = obj_to_char(obj, ch);


        send_to_char
                ("&GYou finish your work and hold up your newly created key card, and you check your keypad.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes crafting a door and lock.", ch, NULL,
            argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(20000,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_lockdoor);

        ship = ship_from_room(ch->in_room->vnum);
        if (ship)
                if (ship->type == PLAYER_SHIP)
                        fold_area(ch->in_room->area,
                                  ch->in_room->area->filename, FALSE, FALSE);

        installation = installation_from_room(ch->in_room->vnum);
        if (installation)
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);
}

CMDF do_makekey(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *xit;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        int       level, percentage;
        bool      checktool, checkdura;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_DATA *obj;
        OBJ_DATA *key;
        int       vnum;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0'
                    || (str_cmp(arg, "lock") && str_cmp(arg, "copy")))
                {
                        send_to_char
                                ("&RUsage: makekey copy <copykey name>\n\r&w",
                                 ch);
                        send_to_char
                                ("&RUsage: makekey lock <direction>\n\r&w",
                                 ch);
                        return;
                }


                checktool = FALSE;
                checkdura = FALSE;

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                }

                if (!checktool)
                {
                        send_to_char("&RYou need toolkit to make a key.\n\r",
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

                if (!str_cmp(arg, "lock"))
                {
                        if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north") ||
                            !str_cmp(arg2, "s") || !str_cmp(arg2, "south") ||
                            !str_cmp(arg2, "e") || !str_cmp(arg2, "east") ||
                            !str_cmp(arg2, "w") || !str_cmp(arg2, "west") ||
                            !str_cmp(arg2, "sw")
                            || !str_cmp(arg2, "southwest")
                            || !str_cmp(arg2, "nw")
                            || !str_cmp(arg2, "northwest")
                            || !str_cmp(arg2, "se")
                            || !str_cmp(arg2, "southeast")
                            || !str_cmp(arg2, "ne")
                            || !str_cmp(arg2, "northeast")
                            || !str_cmp(arg2, "u") || !str_cmp(arg2, "up")
                            || !str_cmp(arg2, "d") || !str_cmp(arg2, "down"))
                        {
                                if ((xit = find_door(ch, arg2, TRUE)) == NULL)
                                {
                                        send_to_char
                                                ("There is no an exit in that direction\r\n",
                                                 ch);
                                        return;
                                }
                                if (!IS_SET(xit->exit_info, EX_ISDOOR))
                                {
                                        send_to_char
                                                ("&RThat exit does not have a door.\n\r",
                                                 ch);
                                        return;
                                }
                                if (!IS_SET(xit->exit_info, EX_CLOSED))
                                {
                                        send_to_char("&RIt's not closed.\n\r",
                                                     ch);
                                        return;
                                }
                                if (xit->key < 0)
                                {
                                        send_to_char
                                                ("&RIt can't be locked.\n\r",
                                                 ch);
                                        return;
                                }
                                if (!IS_SET(xit->exit_info, EX_LOCKED))
                                {
                                        send_to_char
                                                ("&RThe door must be locked for you to get a proper scan.\n\r",
                                                 ch);
                                        return;
                                }
                        }

                        else
                        {
                                send_to_char("Not a valid direction\r\n", ch);
                                return;
                        }
                }
                if (!str_cmp(arg, "copy"))
                {
                        if ((key = get_obj_here(ch, arg2)) == NULL)
                        {
                                send_to_char
                                        ("&RYou must have a copy of the key to copy it!.\n\r",
                                         ch);
                                return;
                        }
                        if (key->item_type != ITEM_KEY)
                        {
                                send_to_char("&RThat is not a key!.\n\r", ch);
                                return;
                        }
                        if (key->value[0] < 1)
                        {
                                send_to_char
                                        ("&RThat key cannot be copied!.\n\r",
                                         ch);
                                return;
                        }
                }
                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makekey]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the long process of creating a key.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_makekey, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_makekey);
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
                                                    learned[gsn_makekey]);
        vnum = 10438;

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char
                        ("&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r",
                         ch);
                return;
        }

        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        if (!str_cmp(arg, "copy"))
                key = get_obj_here(ch, arg2);

        if (!str_cmp(arg, "lock"))
                percentage =
                        IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_makekey] /
                                                            4);
        else
                percentage =
                        IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_makekey]);
        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura))
        {
                send_to_char("&RYou inspect your new key card closely.\n\r",
                             ch);
                send_to_char
                        ("&RIt is clear that something is not right, and then it snaps in half.\n\r",
                         ch);
                send_to_char("&RYou throw away the scraps in disgust.\n\r",
                             ch);
                learn_from_failure(ch, gsn_makekey);
                return;
        }

        obj = create_object(pObjIndex, level);

        obj->item_type = ITEM_KEY;
        SET_BIT(obj->wear_flags, ITEM_TAKE);
        obj->level = level;
        obj->weight = 1;
        stralloc_printf(&obj->name, "a copy of a key card to door in %s",
                        smash_color(ch->in_room->name));
        if (!str_cmp(arg, "copy"))
                stralloc_printf(&obj->short_descr,
                                "a copy of a key card to door in %s",
                                key->name);

        else
                stralloc_printf(&obj->short_descr,
                                "a copy of a key card to door in %s",
                                ch->in_room->name);

        stralloc_printf(&obj->description, "%s", "A key card was left here.");
        if (!str_cmp(arg, "copy"))
        {
                if ((key = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char
                                ("&RYSomething went terribly wrong here, tell Greven!.\n\r",
                                 ch);
                        return;
                }
                send_to_char
                        ("&GYou finish your work and hold up a copy of the key card.&w\n\r",
                         ch);
                act(AT_PLAIN, "$n finishes copying a key card.", ch, NULL,
                    argument, TO_ROOM);
                obj->value[0] = key->value[0];
        }
        if (!str_cmp(arg, "lock"))
        {
                if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north") ||
                    !str_cmp(arg2, "s") || !str_cmp(arg2, "south") ||
                    !str_cmp(arg2, "e") || !str_cmp(arg2, "east") ||
                    !str_cmp(arg2, "w") || !str_cmp(arg2, "west") ||
                    !str_cmp(arg2, "sw") || !str_cmp(arg2, "southwest") ||
                    !str_cmp(arg2, "nw") || !str_cmp(arg2, "northwest") ||
                    !str_cmp(arg2, "se") || !str_cmp(arg2, "southeast") ||
                    !str_cmp(arg2, "ne") || !str_cmp(arg2, "northeast") ||
                    !str_cmp(arg2, "u") || !str_cmp(arg2, "up") ||
                    !str_cmp(arg2, "d") || !str_cmp(arg2, "down"))
                {
                        if ((xit = find_door(ch, arg2, TRUE)) == NULL)
                        {
                                send_to_char
                                        ("There is already an exit in that direction, but this should not have happened. Talk to Greven.\r\n",
                                         ch);
                                return;
                        }
                        else
                        {
                                send_to_char
                                        ("&GYou finish your work and hold up a the key card.&w\n\r",
                                         ch);
                                act(AT_PLAIN,
                                    "$n finishes making a new key card.", ch,
                                    NULL, argument, TO_ROOM);
                                obj->value[0] = xit->key;

                        }
                }
                else
                {
                        send_to_char
                                ("Not a valid direction, but this should not have happened. Talk to Greven.\r\n",
                                 ch);
                        return;
                }
        }
        obj = obj_to_char(obj, ch);

        {
                long      xpgain;

                xpgain = UMIN(20000,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_makekey);
}

CMDF do_purgeinstallation(CHAR_DATA * ch, char *argument)
{
        INSTALLATION_DATA *installation;
        CHAR_DATA *victim;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *room;
        int       roomnum;
        OBJ_DATA *robj;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        if (arg[0] == '\0' || !is_number(arg))
        {
                send_to_char("Syntax: purgeinstallation <vnum>\n\r", ch);
                return;
        }

        installation = installation_from_room(atoi(arg));

        if (!installation)
        {
                send_to_char("There is no installation at the vnum\n\r", ch);
                return;
        }

        for (roomnum = installation->first_room;
             roomnum <= installation->last_room; roomnum++)
        {
                room = get_room_index(roomnum);

                if (room != NULL)
                {
                        victim = room->first_person;
                        while (victim)
                        {
                                if (victim == ch)
                                {
                                        send_to_char
                                                ("Why would you destroy an installation you're standing in? Get the hell out of it!\n\r",
                                                 ch);
                                        return;
                                }
                                else
                                        send_to_char
                                                ("You sick fuck, there are people in there! BAD IMMORTAL!\n\r",
                                                 ch);
                                return;
                                victim = room->first_person;
                        }
                        for (robj = room->first_content; robj;
                             robj = robj->next_content)
                        {
                                if (str_cmp(arg2, "purge"))
                                {
                                        send_to_char
                                                ("There are objects in there. If you want to purge the objects as well, use:\n\r",
                                                 ch);
                                        send_to_char
                                                ("&Gpurgeinstallation <vnum> purge&R&W\n\r",
                                                 ch);
                                        return;
                                }
                        }
                }
        }
        send_to_char("&GPurging Installation data....\n\r", ch);
        if (!str_cmp(arg2, "purge"))
                send_to_char("&GPurging Objects....\n\r", ch);
        destroy_installation(installation);
        send_to_char("&GInstallation destroyed.\n\r", ch);
        if (!str_cmp(arg2, "purge"))
                send_to_char("&GObjects Purged.\n\r", ch);
        return;
}


CMDF do_sabotage(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       percentage;
        bool      checktool, checksteel, checkchem, checksuper, checkdrink,
                checkcircuit, checkbatt;
        OBJ_DATA *obj;
        INSTALLATION_DATA *installation;

        mudstrlcpy(arg, argument, MIL);

        switch (ch->substate)
        {
        default:

                checktool = FALSE;
                checksteel = FALSE;
                checkchem = FALSE;
                checksuper = FALSE;
                checkdrink = FALSE;
                checkcircuit = FALSE;
                checkbatt = FALSE;

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) == NULL)
                {
                        send_to_char
                                ("&RYou need to be in an installation to do that.\n\r",
                                 ch);
                        return;
                }

                if (installation->mainroom != ch->in_room->vnum
                    && !IS_IMMORTAL(ch))
                {
                        send_to_char
                                ("&RThis room is not critical enough to destroy the whole facility. Try else where.\n\r",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checksteel = TRUE;
                        if (obj->item_type == ITEM_CHEMICAL)
                                checkchem = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checksuper = TRUE;
                        if (obj->item_type == ITEM_DRINK_CON
                            && obj->value[1] == 0)
                                checkdrink = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcircuit = TRUE;
                        if (obj->item_type == ITEM_BATTERY)
                                checkbatt = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need a toolkit to set up your explosives.\n\r",
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

                if (!checkcircuit)
                {
                        send_to_char
                                ("&RYou need a small circuit for the timer.\n\r",
                                 ch);
                        return;
                }

                if (!checksteel)
                {
                        send_to_char
                                ("&RYou need some durasteel for the casing.\n\r",
                                 ch);
                        return;
                }

                if (!checksuper)
                {
                        send_to_char
                                ("&RYou need a superconductor for the mechanism.\n\r",
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
                        : (int) (ch->pcdata->learned[gsn_sabotage]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou start to construct your bomb.\n\r",
                                 ch);
                        add_timer(ch, TIMER_DO_FUN, 35, do_sabotage, 1);
                        ch->dest_buf = str_dup(arg);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_sabotage);
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
                if ((installation =
                     installation_from_room(ch->in_room->vnum)) != NULL)
                {
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the organizations network, 'The clan installation on %s is being sabotaged by %s!'\n\r",
                                 installation_table[installation->type].
                                 installation_name,
                                 installation->planet->name, ch->name);
                        echo_to_clan(AT_PINK, buf, installation->clan);
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the installation intercom, 'This installation is being sabotaged by %s!'\n\r",
                                 installation_table[installation->type].
                                 installation_name, ch->name);
                        echo_to_installation(AT_RED, buf, installation);
                }
                return;
        }

        ch->substate = SUB_NONE;

        checktool = FALSE;
        checksteel = FALSE;
        checkchem = FALSE;
        checksuper = FALSE;
        checkdrink = FALSE;
        checkcircuit = FALSE;
        checkbatt = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DURASTEEL)
                {
                        checksteel = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_CHEMICAL)
                {
                        checkchem = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR)
                {
                        checksuper = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

                if (obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0)
                {
                        checkdrink = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

                if (obj->item_type == ITEM_CIRCUIT)
                {
                        checkcircuit = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

                if (obj->item_type == ITEM_BATTERY)
                {
                        checkbatt = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }

        }

        if ((installation =
             installation_from_room(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("You are not in an installation, but you should be. Tell greven.",
                         ch);
                return;
        }

        percentage =
                IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_sabotage]);

        if (number_percent() > percentage / 2 || (!checkchem) || (!checksuper)
            || (!checkdrink) || (!checktool) || (!checksteel) || (!checkbatt)
            || (!checkcircuit))
        {
                send_to_char("&RYou finish constructing your explosives.\n\r",
                             ch);
                send_to_char
                        ("&RYou go to adjust the timer, but you knock the connection pin.\n\r",
                         ch);
                send_to_char
                        ("&RYou jump back the circuitry short circuits, alerting the facility.\n\r",
                         ch);
                learn_from_failure(ch, gsn_sabotage);
                snprintf(buf, MSL,
                         "&P%s Security speaks over the organizations network, 'The clan installation on %s is being sabotaged by %s!'\n\r",
                         installation_table[installation->type].
                         installation_name, installation->planet->name,
                         ch->name);
                echo_to_clan(AT_PINK, buf, installation->clan);
                snprintf(buf, MSL,
                         "&P%s Security speaks over the installation intercom, 'This installation is being sabotaged by %s!'\n\r",
                         installation_table[installation->type].
                         installation_name, ch->name);
                echo_to_installation(AT_RED, buf, installation);

                return;
        }

        send_to_char
                ("&GYou set the timer on your explosives. Get out, quick!&w\n\r",
                 ch);
        installation->timer =
                installation_table[installation->type].rooms * 10;

        {
                long      xpgain;

                xpgain = UMIN(installation_table[installation->type].rooms *
                              200,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_sabotage);

}


void addroominstallation(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *room;
        INSTALLATION_DATA *installation;
        bool      match;
        EXIT_DATA *pexit;

        match = FALSE;
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        installation = installation_from_room(ch->in_room->vnum);

        if (!installation)
        {
                send_to_char
                        ("You can only add rooms to you an installation.\r\n",
                         ch);
                return;
        }

        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
            || ch->pcdata->clan != installation->clan)
        {
                send_to_char
                        ("You must be in this installations clan to do that.\r\n",
                         ch);
                return;
        }

        if ((ch->pcdata && ch->pcdata->bestowments
             && is_name("installations", ch->pcdata->bestowments))
            || !str_cmp(ch->name, ch->pcdata->clan->leader)
            || !str_cmp(ch->name, ch->pcdata->clan->number1)
            || !str_cmp(ch->name, ch->pcdata->clan->number2))
                ;
        else
        {
                send_to_char
                        ("Your clan has not empowered you with that ability.\n\r",
                         ch);
                return;
        }

        if (installation->last_built == installation->last_room
            || installation->last_built < installation->first_room)
        {
                send_to_char
                        ("You have no more rooms to build this installation.\r\n",
                         ch);
                if (installation->last_built < installation->first_room)
                {
                        installation->last_built = installation->last_room;
                        save_installations(installation);
                }
                return;
        }
        room = get_room_index(installation->last_built + 1);

        if (!room)
        {
                bug("addroominstallation: no such room", 0);
                return;
        }

        if (str_cmp(arg2, "main") && room->vnum == installation->last_room)
        {
                send_to_char
                        ("You cannot put any room but the \"main\" room at the end of your installation",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north") ||
            !str_cmp(arg1, "s") || !str_cmp(arg1, "south") ||
            !str_cmp(arg1, "e") || !str_cmp(arg1, "east") ||
            !str_cmp(arg1, "w") || !str_cmp(arg1, "west") ||
            !str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest") ||
            !str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest") ||
            !str_cmp(arg1, "se") || !str_cmp(arg1, "southeast") ||
            !str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast") ||
            !str_cmp(arg1, "u") || !str_cmp(arg1, "up") ||
            !str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
        }
        else
        {
                send_to_char("Not a valid direction\r\n", ch);
                return;
        }

        if (!str_cmp(arg2, "workshop"))
        {
                if (installation->type != RESEARCH_INSTALLATION &&
                    installation->type != CLANHQ_INSTALLATION &&
                    installation->type != TRAINING_INSTALLATION &&
                    installation->type != SHIPYARD_INSTALLATION &&
                    installation->type != MINING_INSTALLATION)
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 10000;
                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_FACTORY);
                xSET_BIT(room->room_flags, ROOM_REFINERY);
                stralloc_printf(&room->name, "%s", "A Workshop");
        }
        else if (!str_cmp(arg2, "bacta"))
        {
                if ((installation->type != CLANHQ_INSTALLATION) &&
                    (installation->type != TRAINING_INSTALLATION) &&
                    (installation->type != GOVERNMENT_INSTALLATION) &&
                    (installation->type != MINING_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 25000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 25000;
                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_BACTA);
                STRFREE(room->name);
                room->name = STRALLOC("Bacta Tank");
        }
        else if (!str_cmp(arg2, "meditate"))
        {
                if ((installation->type != CLANHQ_INSTALLATION) &&
                    (installation->type != TRAINING_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }

                if (installation->clan->funds < 100000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 100000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_SILENCE);
                xSET_BIT(room->room_flags, ROOM_SAFE);
                stralloc_printf(&room->name, "%s",
                                "A Quiet Meditation Chamber");
        }
        else if (!str_cmp(arg2, "hotel"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 5000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_HOTEL);
                xSET_BIT(room->room_flags, ROOM_INN);
                stralloc_printf(&room->name, "%s", "The Passenger's Lounge");
        }
        else if (!str_cmp(arg2, "bank"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 5000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_BANK);
                stralloc_printf(&room->name, "%s",
                                "Local Office of the Galactic Banking Guild");
        }
        else if (!str_cmp(arg2, "main"))
        {
                if (installation->mainroom > 0)
                {
                        send_to_char("You already have the main room set up!",
                                     ch);
                        return;
                }
                if (room->vnum != installation->last_room)
                {
                        send_to_char
                                ("You can not deploy this room until you have completed the rest of your installation",
                                 ch);
                        return;
                }

                if (installation->clan->funds < 50000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 5000;

                match = TRUE;
                stralloc_printf(&room->name, "%s",
                                installation_table[installation->type].
                                main_name);
                installation->mainroom = room->vnum;
                if (installation->type == SHIPYARD_INSTALLATION)
                {
                        xSET_BIT(room->room_flags, ROOM_IMPORT);
                        /*
                         * makedock ( installation); 
                         */
                }
        }
        else if (!str_cmp(arg2, "auction"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 5000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_AUCTION);
                stralloc_printf(&room->name, "%s", "Local Auction Hall");
        }
        else if (!str_cmp(arg2, "arena"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != TRAINING_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 75000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_ARENA);
                stralloc_printf(&room->name, "%s", "Training Arena");
        }
        else if (!str_cmp(arg2, "recruitment"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }
                if (installation->clan->funds < 500000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 500000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_RECRUIT);
                stralloc_printf(&room->name, "%s", "A Recruitment Office");
        }
        else if (!str_cmp(arg2, "home"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }

                if (installation->clan->funds < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 10000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_EMPTY_HOME);
                xSET_BIT(room->room_flags, ROOM_HOTEL);
                stralloc_printf(&room->name, "%s", "An Empty Apartment");
        }
        else if (!str_cmp(arg2, "restaurant"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }


                if (installation->clan->funds < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 1000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_BAR);
                xSET_BIT(room->room_flags, ROOM_KITCHEN);
                xSET_BIT(room->room_flags, ROOM_CAFE);
                stralloc_printf(&room->name, "%s", "A Restaurant");
        }
        else if (!str_cmp(arg2, "office"))
        {
                if ((installation->type != CLANHQ_INSTALLATION)
                    && (installation->type != GOVERNMENT_INSTALLATION))
                {
                        send_to_char
                                ("You cannot put that into this type of installation!\r\n",
                                 ch);
                        return;
                }

                if (installation->clan->funds < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 1000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_OFFICE);
                xSET_BIT(room->room_flags, ROOM_EXECUTIVE);
                xSET_BIT(room->room_flags, ROOM_BOARDROOM);
                stralloc_printf(&room->name, "%s", "An Office");
        }
        else if (!str_cmp(arg2, "turbolift"))
        {
                if (installation->clan->funds < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 500;

                match = TRUE;
                stralloc_printf(&room->name, "%s", "A Turbolift");
        }
        else if (!str_cmp(arg2, "corridor"))
        {
                if (installation->clan->funds < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                installation->clan->funds -= 500;

                match = TRUE;
                stralloc_printf(&room->name, "%s", "A Corridor");
        }
        else
                match = FALSE;

        if (!match)
        {
                send_to_char("&RNo such room type\r\n", ch);
                return;
        }
        if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north") ||
            !str_cmp(arg1, "s") || !str_cmp(arg1, "south") ||
            !str_cmp(arg1, "e") || !str_cmp(arg1, "east") ||
            !str_cmp(arg1, "w") || !str_cmp(arg1, "west") ||
            !str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest") ||
            !str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest") ||
            !str_cmp(arg1, "se") || !str_cmp(arg1, "southeast") ||
            !str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast") ||
            !str_cmp(arg1, "u") || !str_cmp(arg1, "up") ||
            !str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
                else
                {
                        pexit = make_exit(ch->in_room, room, get_dir(arg1));
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                        pexit = make_exit(room, ch->in_room,
                                          rev_dir[get_dir(arg1)]);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }
        }
        else
        {
                send_to_char("Not a valid direction\r\n", ch);
                return;
        }
        installation->last_built = room->vnum;
        save_installations(installation);
        fold_area(room->area, room->area->filename, TRUE, FALSE);
        ch_printf(ch,
                  "Room installed. You have %d room left in your installation.\n\r",
                  (installation->last_room - installation->last_built));
        if ((installation->last_built + 1) == installation->last_room)
                send_to_char
                        ("Remember, your last room MUST be the \"Main\" room.",
                         ch);
}

void fireplanet_update()
{
        INSTALLATION_DATA *installation;
        SHIP_DATA *ship;
        int       wepnum, hit, percentage;
        char      buf[MSL];
        bool      fired;

        for (installation = first_installation; installation;
             installation = installation->next)
        {
                fired = FALSE;

                if ((installation_table[installation->type].ions < 1
                     && installation_table[installation->type].turbo < 1)
                    || !installation->planet || !installation->planet->body
                    || installation->mainroom < 1)
                        continue;

                if (installation->weapon_time < 0)
                        installation->weapon_time = 0;

                if (installation->weapon_time > 0)
                {
                        --installation->weapon_time;
                        continue;
                }

                if (installation->weapon_time == 0)
                {
                        if (!installation)
                        {
                                bug("fireplanet_update: No Installation.");
                                continue;
                        }
                        if (!installation->planet)
                        {
                                bug("fireplanet_update: Installation %d has no planet.", installation->first_room);
                                continue;
                        }
                        if (!installation->planet->starsystem)
                        {
                                bug("fireplanet_update: Planet %s has no starsystem.", installation->planet->name);
                                continue;
                        }
                        for (ship =
                             installation->planet->starsystem->first_ship;
                             ship; ship = ship->next_in_starsystem)
                        {

                                if (installation->planet->body->
                                    distance(ship) > 2000)
                                        continue;
                                else
                                {
                                        if (!ship->clan)
                                                continue;
                                        if (!installation->clan)
                                                continue;
                                        if (!installation->clan->enemy)
                                                continue;
                                        if (ship->clan !=
                                            installation->clan->enemy)
                                                continue;

                                        if (installation_table
                                            [installation->type].ions > 1)
                                        {
                                                wepnum = installation_table
                                                        [installation->type].
                                                        ions;
                                                hit = 0;
                                                hit += (int) (ship->
                                                              ship_class *
                                                              25);
                                                hit -= ship->manuever / 10;
                                                hit -= ship->currspeed / 20;
                                                hit -= (installation->planet->
                                                        body->
                                                        distance(ship)) / 70;
                                                hit -= ship->evasive / 10;
                                                hit = URANGE(10, hit, 90);
                                                percentage = number_percent();
                                                if (percentage > hit)
                                                {
                                                        snprintf(buf, MSL,
                                                                 "Ion cannons fire from %s at you, but miss",
                                                                 installation->
                                                                 planet->
                                                                 body->
                                                                 name());
                                                        echo_to_cockpit
                                                                (AT_ORANGE,
                                                                 ship, buf);
                                                        snprintf(buf, MSL,
                                                                 "The installations ion cannons fire at %s, but miss",
                                                                 ship->name);
                                                        echo_to_installation
                                                                (AT_ORANGE,
                                                                 buf,
                                                                 installation);
                                                        snprintf(buf, MSL,
                                                                 "Ion cannon discharge fires from %s, barely missing %s",
                                                                 installation->
                                                                 planet->
                                                                 body->name(),
                                                                 ship->name);
                                                        echo_to_system
                                                                (AT_ORANGE,
                                                                 ship, buf,
                                                                 ship);
                                                        installation->
                                                                weapon_time +=
                                                                8 * wepnum;
                                                        fired = TRUE;
                                                }
                                                else
                                                {
                                                        snprintf(buf, MSL,
                                                                 "Ion cannon fire from %s hits your ship",
                                                                 installation->
                                                                 planet->
                                                                 body->
                                                                 name());
                                                        echo_to_cockpit
                                                                (AT_ORANGE,
                                                                 ship, buf);
                                                        snprintf(buf, MSL,
                                                                 "Ion cannon fire from the installation hits %s",
                                                                 ship->name);
                                                        echo_to_installation
                                                                (AT_ORANGE,
                                                                 buf,
                                                                 installation);
                                                        snprintf(buf, MSL,
                                                                 "Ion cannon discharge from %s hits %s",
                                                                 installation->
                                                                 planet->
                                                                 body->name(),
                                                                 ship->name);
                                                        echo_to_system
                                                                (AT_ORANGE,
                                                                 ship, buf,
                                                                 ship);
                                                        damage_ship_ion(ship,
                                                                        7 *
                                                                        wepnum,
                                                                        14 *
                                                                        wepnum);
                                                        installation->
                                                                weapon_time +=
                                                                8 * wepnum;
                                                        fired = TRUE;
                                                }
                                        }
                                        if (installation_table
                                            [installation->type].turbo > 1)
                                        {
                                                wepnum = installation_table
                                                        [installation->type].
                                                        turbo;
                                                hit = 0;
                                                hit += ship->ship_class * 50;
                                                hit -= ship->manuever / 10;
                                                hit -= ship->currspeed / 20;
                                                hit -= (installation->planet->
                                                        body->
                                                        distance(ship)) / 70;
                                                hit -= ship->evasive / 10;
                                                hit = URANGE(10, hit, 90);
                                                percentage = number_percent();
                                                if (percentage > hit)
                                                {
                                                        snprintf(buf, MSL,
                                                                 "Turbolaser fire from %s at you, but miss",
                                                                 installation->
                                                                 planet->
                                                                 body->
                                                                 name());
                                                        echo_to_cockpit
                                                                (AT_ORANGE,
                                                                 ship, buf);
                                                        snprintf(buf, MSL,
                                                                 "The installations turbolasers fire at %s, but miss",
                                                                 ship->name);
                                                        echo_to_installation
                                                                (AT_ORANGE,
                                                                 buf,
                                                                 installation);
                                                        snprintf(buf, MSL,
                                                                 "Turbolsers fire from %s, barely missing %s",
                                                                 installation->
                                                                 planet->
                                                                 body->name(),
                                                                 ship->name);
                                                        echo_to_system
                                                                (AT_ORANGE,
                                                                 ship, buf,
                                                                 ship);
                                                        installation->
                                                                weapon_time +=
                                                                8 * wepnum;
                                                }
                                                else
                                                {
                                                        snprintf(buf, MSL,
                                                                 "Turbolaser fire from %s hits your ship",
                                                                 installation->
                                                                 planet->
                                                                 body->
                                                                 name());
                                                        echo_to_cockpit
                                                                (AT_ORANGE,
                                                                 ship, buf);
                                                        snprintf(buf, MSL,
                                                                 "Turbolaser fire from the installation hits %s",
                                                                 ship->name);
                                                        echo_to_installation
                                                                (AT_ORANGE,
                                                                 buf,
                                                                 installation);
                                                        snprintf(buf, MSL,
                                                                 "Turbolaser fire from %s hits %s",
                                                                 installation->
                                                                 planet->
                                                                 body->name(),
                                                                 ship->name);
                                                        echo_to_system
                                                                (AT_ORANGE,
                                                                 ship, buf,
                                                                 ship);
                                                        damage_ship(ship,
                                                                    7 *
                                                                    wepnum,
                                                                    14 *
                                                                    wepnum);
                                                        installation->
                                                                weapon_time +=
                                                                8 * wepnum;
                                                }
                                        }
                                }
                        }
                }
                if (!fired && installation->weapon_time == 0)
                        installation->weapon_time = 8;
        }

        /*
         * These two loops should be combined 
         */
        for (installation = first_installation; installation;
             installation = installation->next)
        {
                if (installation->timer == 0)
                        continue;

                --installation->timer;
                if (installation->timer == 20)
                {
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the installation intercom, 'Explosives have been detected inside this faciltiy!'",
                                 installation_table[installation->type].
                                 installation_name);
                        echo_to_installation(AT_RED, buf, installation);
                }

                if (installation->timer == 15)
                {
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the installation intercom, 'The timer has been identified as having approximately 15 seconds left!'",
                                 installation_table[installation->type].
                                 installation_name);
                        echo_to_installation(AT_RED, buf, installation);
                }

                if (installation->timer == 10)
                {
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the installation intercom, 'Installation destruction imminent, explosion in 10 seconds!'",
                                 installation_table[installation->type].
                                 installation_name);
                        echo_to_installation(AT_RED, buf, installation);
                }

                if (installation->timer == 5)
                {
                        snprintf(buf, MSL,
                                 "&P%s Security speaks over the installation intercom, 'Installation destruction imminent, explosion in 5 seconds! I'm leaving, now!'",
                                 installation_table[installation->type].
                                 installation_name);
                        echo_to_installation(AT_RED, buf, installation);
                }

                if (installation->timer == 1)
                {
                        snprintf(buf, MSL,
                                 "&PThe installation shakes as the explosion begins!\n\r&w&WThe heat and flames rush at you, and in a last moment of sensation,\n\ryou scream in horrible agony.");
                        echo_to_installation(AT_RED, buf, installation);
                        destroy_installation(installation);
                }
        }
}

/* shields stop landing, planet captured installations become new clans */
