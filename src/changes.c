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
 * Original code by Xkilla
 * Ported to SWR by Atrox
 */
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include "mud.h"
#include "changes.h"
#include "web-server.h"

/*
 * Globals
 */
char     *current_date args((void));

/*
 * Local Functions
 */

int       maxChanges;
int       immortal_changes;
CHANGE_DATA *changes_table;

/* Hacktastic to make sure warning doesn't show up.. thats what man pages says todo :D */
size_t my_strftime(char *s, size_t max, const char *fmt, const struct tm *tm)
{
        return strftime(s, max, fmt, tm);
}


void load_changes(void)
{
        FILE     *fp;
        int       i;

        if (!(fp = fopen(CHANGES_FILE, "r")))
        {
                bug("Could not open Changes File for reading.", 0);
                return;
        }

        immortal_changes = 0;

        fscanf(fp, "%d\n", &maxChanges);

        CREATE(changes_table, CHANGE_DATA, maxChanges + 1);

        for (i = 0; i < maxChanges; i++)
        {
                changes_table[i].change = fread_string(fp);
                changes_table[i].coder = fread_string(fp);
                changes_table[i].date = fread_string(fp);
                changes_table[i].mudtime = fread_number(fp);
                changes_table[i].immchange = fread_number(fp);
                if (changes_table[i].immchange == 1)
                        immortal_changes++;
        }
/*    changes_table[maxChanges].coder = str_dup("");    The hell?? */
        FCLOSE(fp);
        boot_log(" Done changes ");
        return;
}

char     *current_date()
{
        static char buf[128];
        struct tm *datetime;

        datetime = localtime(&current_time);
        my_strftime(buf, 128, "%x", datetime);
        return buf;
}

#ifdef WEB
void write_webchanges(void)
{
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        FILE     *fp;
        int       i, count = 1;

        if ((fp = fopen(WEBCHANGES_FILE, "w")) == NULL)
        {
                perror(WEBCHANGES_FILE);
                return;
        }
        fprintf(fp,
                "<html><head><title>Changes List</title><style>  pre { display: inline; } a:link { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:visited { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:hover { background-color: inherit; text-decoration: none; font-weight: bold; color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; color: inherit; }</style></head>\n");
        fprintf(fp,
                "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n");
        fprintf(fp, "<h1>Changes List</h1><br>\n");
        fprintf(fp, "<table cellpadding=0 cellspacing=0>\n");
        web_colourconv(buf,
                       "&z[&w  #&z]&B|&z[&wCoder   &z    ]&B|&z[&wDate &z   ]&B|&wChange");
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
        web_colourconv(buf,
                       "&B-----|--------------|----------&B|&B-----------------------------");
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);

        for (i = maxChanges - 1; i >= 0; i--)
        {
                if (changes_table[i].immchange == 1)
                        continue;
                if (NULLSTR(changes_table[i].coder)
                    || NULLSTR(changes_table[i].change)
                    || NULLSTR(changes_table[i].date))
                        continue;
                snprintf(buf2, MSL,
                         "&z[&w%3d&z]&B|&z[&w%-12s&z]&B|&z[&W%-6s&z]&B|&w%-55s",
                         count, capitalize(changes_table[i].coder),
                         changes_table[i].date, changes_table[i].change);
                web_colourconv(buf, buf2);
                fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
                count++;
        }
        web_colourconv(buf,
                       "&B--------------------------------------------------------------");
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
        snprintf(buf2, MSL,
                 "&zThere is a total of &B[&w %3d &B]&z changes in the database.",
                 count - 1);
        web_colourconv(buf, buf2);
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
        web_colourconv(buf,
                       "&zAlso see: &B'&wchanges all&B'&z for a list of all the changes.");
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
        web_colourconv(buf,
                       "&B--------------------------------------------------------------");
        fprintf(fp, "<tr><td><pre>%s</pre></td></tr>\n", buf);
        fprintf(fp, "</table></body></html>");
        FCLOSE(fp);
        return;
}
#endif
void save_changes(void)
{
        FILE     *fp;
        int       i;

        if (!(fp = fopen(CHANGES_FILE, "w")))
        {
                perror(CHANGES_FILE);
                return;
        }

        fprintf(fp, "%d\n", maxChanges);

        for (i = 0; i < maxChanges; i++)
        {
                fprintf(fp, "%s~\n", changes_table[i].change);
                fprintf(fp, "%s~\n", changes_table[i].coder);
                fprintf(fp, "%s~\n", changes_table[i].date);
                fprintf(fp, "%ld\n", changes_table[i].mudtime);
                fprintf(fp, "%d\n", changes_table[i].immchange);
                fprintf(fp, "\n");
        }
        FCLOSE(fp);
        /*
         * Write Webchanges 
         */
#ifdef WEB
        write_webchanges();
#endif
        return;
}

void free_changes(void)
{
        int       i = 0;

        for (i = 0; i < maxChanges + 1; i++)
        {
                if (changes_table[i].change)
                        STRFREE(changes_table[i].change);
                if (changes_table[i].coder)
                        STRFREE(changes_table[i].coder);
                if (changes_table[i].date)
                        STRFREE(changes_table[i].date);
        }
        DISPOSE(changes_table);
}

void delete_change(int iChange)
{
        int       i, j;
        CHANGE_DATA *new_table;

        new_table =
                (CHANGE_DATA *) CALLOC(1, sizeof(CHANGE_DATA) * maxChanges);

        if (!new_table)
        {
                return;
        }

        for (i = 0, j = 0; i < maxChanges + 1; i++)
        {
                if (i != iChange)
                {
                        new_table[j] = changes_table[i];
                        j++;
                }
                else
                {
                        if (changes_table[i].change)
                                STRFREE(changes_table[i].change);
                        if (changes_table[i].coder)
                                STRFREE(changes_table[i].coder);
                        if (changes_table[i].date)
                                STRFREE(changes_table[i].date);
                }
        }

        free(changes_table);
        changes_table = new_table;

        maxChanges--;

        return;
}


CMDF do_addchange(CHAR_DATA * ch, char *argument)
{
        CHANGE_DATA *new_table;
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Syntax: addchange <change>\n\r", ch);
                send_to_char("Type 'changes' to view the list.\n\r", ch);
                return;
        }

        maxChanges++;
        new_table =
                (CHANGE_DATA *) REALLOC(changes_table,
                                        sizeof(CHANGE_DATA) * (maxChanges +
                                                               1));

        if (!new_table) /* REALLOC failed */
        {
                send_to_char
                        ("Memory allocation failed. Brace for impact.\n\r",
                         ch);
                return;
        }

        changes_table = new_table;

        changes_table[maxChanges - 1].change = STRALLOC(argument);
        changes_table[maxChanges - 1].coder = STRALLOC(ch->name);
        changes_table[maxChanges - 1].date = STRALLOC(current_date());
        changes_table[maxChanges - 1].mudtime = current_time;
        changes_table[maxChanges - 1].immchange = FALSE;

        send_to_char("Changes Created.\n\r", ch);
        send_to_char("Type 'changes' to see the changes.\n\r", ch);
        ch_printf(ch,
                  "New Change added to the mud, type 'changes' to see it\n\r");
        snprintf(buf, MSL, "%s",
                 "A new change has been added, type 'CHANGES' to see it.");
        info_chan(buf);
        save_changes();
        return;
}

CMDF do_addimmchange(CHAR_DATA * ch, char *argument)
{
        CHANGE_DATA *new_table;

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Syntax: addimmchange <change>\n\r", ch);
                send_to_char("Type 'changes' to view the list.\n\r", ch);
                return;
        }

        maxChanges++;
        new_table =
                (CHANGE_DATA *) REALLOC(changes_table,
                                        sizeof(CHANGE_DATA) * (maxChanges +
                                                               1));

        if (!new_table) /* REALLOC failed */
        {
                send_to_char
                        ("Memory allocation failed. Brace for impact.\n\r",
                         ch);
                return;
        }

        changes_table = new_table;

        changes_table[maxChanges - 1].change = STRALLOC(argument);
        changes_table[maxChanges - 1].coder = STRALLOC(ch->name);
        changes_table[maxChanges - 1].date = STRALLOC(current_date());
        changes_table[maxChanges - 1].mudtime = current_time;
        changes_table[maxChanges - 1].immchange = TRUE;
        immortal_changes++;
        send_to_char("Changes Created.\n\r", ch);
        send_to_char("Type 'changes' to see the changes.\n\r", ch);
        save_changes();
        return;
}

CMDF do_chedit(CHAR_DATA * ch, char *argument)
{
        int       change;
        char      arg1[MAX_INPUT_LENGTH];

        if (IS_NPC(ch))
                return;

        if (!ch->desc || NULLSTR(argument))
        {
                send_to_char("&RSyntax: &Gchedit &C<&cload&C/&csave&C>&w\n\r"
                             "        &Gchedit &C<&cchange #&C> &C<&cdelete&C>&w\n\r"
                             "        &Gchedit &C<&cchange #&C> &C<&cimmflag&C>&w\n\r"
                             "        &Gchedit &C<&cchange #&C> &C<&ccoder&C> &C<&cnew coder&C>&w\n\r"
                             "        &Gchedit &C<&cchange #&C> &C<&cchange&C> &C<&cnew change&C>&w\n\r",
                             ch);
                return;
        }

        if (!str_cmp(argument, "load"))
        {
                free_changes();
                load_changes();
                send_to_char("Changes Loaded.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "save"))
        {
                save_changes();
                send_to_char("Changes Saved.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        if (!is_number(arg1))
        {
                send_to_char("Which change?\n\r", ch);
                return;
        }
        change = atoi(arg1);
        if (change < 0 || change > maxChanges)
        {
                ch_printf(ch, "Valid changes are from 1 to %d.\n\r",
                          maxChanges);
                return;
        }
        change--;

        argument = one_argument(argument, arg1);
        if (NULLSTR(arg1))
        {
                send_to_char("do what?\n\r", ch);
                do_chedit(ch, "");
        }

        if (!str_cmp(arg1, "coder"))
        {
                if (changes_table[change].coder)
                        STRFREE(changes_table[change].coder);
                changes_table[change].coder = STRALLOC(argument);
                send_to_char("Coder changed.\n\r", ch);
        }
        else if (!str_cmp(arg1, "coder"))
        {
                if (changes_table[change].change)
                        STRFREE(changes_table[change].change);
                changes_table[change].change = STRALLOC(argument);
                send_to_char("Change changed.\n\r", ch);
        }
        else if (!str_cmp(arg1, "immchange"))
        {
                changes_table[change].immchange =
                        !changes_table[change].immchange;
                if (changes_table[change].immchange)
                        immortal_changes++;
                else
                        immortal_changes--;
                send_to_char("Change flag changed.\n\r", ch);
        }
        else if (!str_cmp(arg1, "delete"))
        {
                delete_change(change);
                send_to_char("Change deleted.\n\r", ch);
        }
        else
        {
                send_to_char("I don't know how todo that.\n\r", ch);
                return;
        }
        save_changes();

        return;
}

CMDF do_news(CHAR_DATA * ch, char *argument)
{
        char     *test;
        int       today;
        int       i, immchanges = 0;
        int       start = 0, count;

        if (!ch && IS_NPC(ch))
                return;

        if (maxChanges < 1)
                return;

        i = 0;
        test = current_date();
        today = 0;
        for (i = 0; i < maxChanges; i++)
        {
                if (!IS_IMMORTAL(ch) && changes_table[i].immchange == 1)
                        continue;
                if (NULLSTR(changes_table[i].coder)
                    || NULLSTR(changes_table[i].change)
                    || NULLSTR(changes_table[i].date))
                        continue;
                if (!str_cmp(test, changes_table[i].date))
                        today++;
        }
        if (!str_cmp(argument, "all"))
                start = 0;
        else
                start = maxChanges - 10;

        pager_printf(ch,
                     "&z[&w  #&z]&B|&z[&wCoder   &z    ]&B|&z%s[&wDate &z   ]&B|&wChange\n\r",
                     ch && IS_IMMORTAL(ch) ? "[&wImm&z]&B|&z" : "");
        pager_printf(ch,
                     "&B-----|--------------|%s----------&B|&B-----------------------------\n\r",
                     ch && IS_IMMORTAL(ch) ? "-----|" : "");

        if (!IS_IMMORTAL(ch) && start != 0)
        {
                for (count = 0, i = maxChanges - 1; count < 9; i--)
                        if (changes_table[i].immchange == 0)
                                count++;
                start = i;
        }



        for (i = 0; i < maxChanges; i++)
        {
                if (changes_table[i].immchange == 1
                    && (!ch || !IS_IMMORTAL(ch)))
                {
                        immchanges++;
                        continue;
                }
                if (NULLSTR(changes_table[i].coder)
                    || NULLSTR(changes_table[i].change)
                    || NULLSTR(changes_table[i].date))
                        continue;
                if (i < start)
                        continue;
                pager_printf(ch,
                             "&z[&w%3d&z]&B|&z[&w%-12s&z]&B|&z%s[&W%-6s&z]&B|&w%-55s\n\r",
                             (i + 1 -
                              (ch
                               && IS_IMMORTAL(ch) ? 0 : immchanges)),
                             capitalize(changes_table[i].coder), ch
                             && IS_IMMORTAL(ch) ? changes_table[i].
                             immchange ==
                             1 ? "[&w * &z]&B|&z" : "[&w   &z]&B|&z" : "",
                             changes_table[i].date, changes_table[i].change);
        }
        send_to_pager
                ("&B--------------------------------------------------------------\n\r",
                 ch);
        pager_printf(ch,
                     "&zThere is a total of &B[&w %3d &B]&z changes in the database.\n\r",
                     maxChanges - (ch
                                   && IS_IMMORTAL(ch) ? 0 :
                                   immortal_changes));
        send_to_pager
                ("&zAlso see: &B'&wchanges all&B'&z for a list of all the changes.\n\r",
                 ch);
        send_to_pager
                ("&B--------------------------------------------------------------\n\r",
                 ch);
        pager_printf(ch,
                     "&zThere is a total of &B[&w %3d&B ] &znew changes that have been added today.\n\r",
                     today);
        send_to_pager
                ("&B--------------------------------------------------------------\n\r",
                 ch);
        return;
}
