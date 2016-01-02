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
 *                $Id: webroom.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "act_html.h"

void room_to_html args((ROOM_INDEX_DATA * room, bool complete));
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

CMDF do_webroom(char *argument)
{

        ROOM_INDEX_DATA *room;
        int       hash;
        bool      complete = FALSE;

        if (!str_cmp(argument, "complete"))
                complete = TRUE;

        for (hash = 0; hash < MAX_KEY_HASH; hash++)
                for (room = room_index_hash[hash]; room; room = room->next)
                {
                        if ((room->vnum >= 300 && room->vnum <= 499)
                            || (room->vnum >= 199 && room->vnum <= 226)
                            || (room->vnum >= 10100 && room->vnum <= 10299)
                            || (room->vnum >= 9000 && room->vnum <= 9049))
                                room_to_html(room, complete);
                }
        return;

}

void room_to_html(ROOM_INDEX_DATA * room, bool complete)
{
        FILE     *fp = NULL;
        char      filename[256];
        char      buf[MAX_INPUT_LENGTH * 10];
        EXIT_DATA *pexit;
        bool      found = FALSE;

        if (!room)
                return;

        snprintf(filename, 256, "%s%d.html", WEB_ROOMS, room->vnum);
        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) != NULL)
        {
                snprintf(buf, MSL,
                         "<HTML><HEAD><TITLE>%s</TITLE></HEAD><BODY bgcolor=#000000><b>\n\r",
                         room->name);
                fprintf(fp, "%s", buf);
                snprintf(buf, MSL,
                         "<font color=#00FFFF font=Couriernew>%s</font><br>\n",
                         room->name);
                fprintf(fp, "%s", buf);
                fprintf(fp,
                        "<font color=#FF0000 font=Courier>-----------------------------------------------------------------------<br></font>\n\r");
                snprintf(buf, MSL,
                         "<pre><font color=#7F7F00>%s</font></pre>\n",
                         room->description);
                fprintf(fp, "%s", buf);
                fprintf(fp,
                        "<font color=#FF0000 font=Courier>-----------------------------------------------------------------------<br></font>\n\r");
                fprintf(fp,
                        "<BR><FONT color=#0000FF font=Courier>Obvious Exits:<br>");
                for (pexit = room->first_exit; pexit; pexit = pexit->next)
                {
                        if (pexit->to_room) /* Set any controls you want here, ie: not closed doors, etc */
                        {
                                found = TRUE;
                                snprintf(buf, MSL,
                                         " <A HREF=\"%d.html\">%s - %s</A><br>",
                                         pexit->to_room->vnum,
                                         capitalize(dir_name[pexit->vdir]),
                                         room_is_dark(pexit->
                                                      to_room) ?
                                         "Too dark to tell" : pexit->to_room->
                                         name);
                                fprintf(fp, "%s", buf);
                        }
                }
                if (!found)
                        fprintf(fp, " None.\n</P>");
                else
                        fprintf(fp, "\n</P>");
                fprintf(fp,
                        "<font color=#FF0000 font=Courier>-----------------------------------------------------------------------<br></font>\n\r");
                if (complete)
                {
                        OBJ_DATA *obj, *obj_next = NULL;
                        CHAR_DATA *rch;

                        for (obj = room->first_content; obj; obj = obj_next)
                        {
                                snprintf(buf, MSL,
                                         "%s%s<FONT color=#000000 font=Courier><BR>",
                                         "<FONT color=#00FF00>",
                                         obj->description);
                                fprintf(fp, "%s", buf);
                        }
                        for (rch = room->first_person; rch;
                             rch = rch->next_in_room)
                        {
                                if (IS_NPC(rch))
                                        snprintf(buf, MSL,
                                                 "<FONT color=#FF00FF font=Courier>%s<BR><FONT color=#000000 font=Courier>",
                                                 rch->long_descr);
                                else
                                        snprintf(buf, MSL,
                                                 "<FONT color=#00FFFF font=Courier>%s<BR><FONT color=#000000 font=Courier>",
                                                 rch->name);
                                fprintf(fp, "%s", buf);
                        }
                }

                fprintf(fp, "</b></BODY></HTML>\n");
                FCLOSE(fp);
        }
        else
        {
                bug("Error Opening room to html index stream!", 0);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}
