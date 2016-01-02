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
 * 2005 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
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
 *                                SWR Hotboot module                                     *
 *****************************************************************************************
 *                   $Id: editor.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <string.h>
#include "mud.h"
#include "editor.h"

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde(char *str)
{
        int       len;
        char      last;
        char     *strptr;

        strptr = str;

        len = strlen(str);
        if (len)
                last = strptr[len - 1];
        else
                last = '\0';

        for (; *str != '\0'; str++)
        {
                if (*str == '~')
                        *str = '-';
        }
        if (len)
                strptr[len - 1] = last;

        return;
}

void start_editing(CHAR_DATA * ch, char *data)
{
        EDITOR_DATA *edit;
        sh_int    lines, size, lpos;
        char      c;

        if (!ch->desc)
        {
                bug("Fatal: start_editing: no desc", 0);
                return;
        }
        if (ch->substate == SUB_RESTRICTED)
                bug("NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED",
                    0);

        set_char_color(AT_GREEN, ch);
        send_to_char
                ("Begin entering your text (/? =help /s =save /c =clear /l =list /f =format)\n\r",
                 ch);
        send_to_char
                ("--------------------------------------------------------------------------\n\r> ",
                 ch);
        if (ch->editor)
                stop_editing(ch);

        CREATE(edit, EDITOR_DATA, 1);
        edit->numlines = 0;
        edit->on_line = 0;
        edit->size = 0;
        size = 0;
        lpos = 0;
        lines = 0;
        if (!data)
                bug("editor: data is NULL!\n\r", 0);
        else
                for (;;)
                {
                        c = data[size++];
                        if (c == '\0')
                        {
                                edit->line[lines][lpos] = '\0';
                                break;
                        }
                        else if (c == '\r');
                        else if (c == '\n' || lpos > 160)
                        {
                                edit->line[lines][lpos] = '\0';
                                lines++;
                                lpos = 0;
                        }
                        else
                                edit->line[lines][lpos++] = c;
                        if (lines >= 99 || size > 8192)
                        {
                                edit->line[lines][lpos] = '\0';
                                break;
                        }
                }
        edit->numlines = lines;
        edit->size = size;
        edit->on_line = lines;
        ch->editor = edit;
        ch->desc->connected = CON_EDITING;
}

char     *copy_buffer(CHAR_DATA * ch)
{
        char      buf[MAX_STRING_LENGTH];
        char      tmp[160];
        sh_int    x, len;

        if (!ch)
        {
                bug("copy_buffer: null ch", 0);
                return STRALLOC("");
        }

        if (!ch->editor)
        {
                bug("copy_buffer: null editor", 0);
                return STRALLOC("");
        }

        buf[0] = '\0';
        for (x = 0; x < ch->editor->numlines; x++)
        {
                mudstrlcpy(tmp, ch->editor->line[x], 100);
                smush_tilde(tmp);
                len = strlen(tmp);
                if (tmp && tmp[len - 1] == '~')
                        tmp[len - 1] = '\0';
                else
                        mudstrlcat(tmp, "\n\r", 100);
                mudstrlcat(buf, tmp, MSL);
        }
        return STRALLOC(buf);
}

void stop_editing(CHAR_DATA * ch)
{
        set_char_color(AT_PLAIN, ch);
        DISPOSE(ch->editor);
        ch->editor = NULL;
        send_to_char("Done.\n\r", ch);
        ch->dest_buf = NULL;
        ch->spare_ptr = NULL;
        ch->substate = SUB_NONE;
        if (!ch->desc)
        {
                bug("Fatal: stop_editing: no desc", 0);
                return;
        }
        ch->desc->connected = CON_PLAYING;
}

/*
 * Simple but nice and handle line editor.			-Thoric
 */
char     *one_argument3(char *argument, char *arg_first);
void edit_buffer(CHAR_DATA * ch, char *argument)
{
        DESCRIPTOR_DATA *d;
        EDITOR_DATA *edit;
        char      cmd[MAX_INPUT_LENGTH];
        char      buf[MAX_INPUT_LENGTH];
        sh_int    x, line, max_buf_lines;
        bool      save;

        if ((d = ch->desc) == NULL)
        {
                send_to_char("You have no descriptor.\n\r", ch);
                return;
        }

        if (d->connected != CON_EDITING)
        {
                send_to_char("You can't do that!\n\r", ch);
                bug("Edit_buffer: d->connected != CON_EDITING", 0);
                return;
        }

        if (ch->substate <= SUB_PAUSE)
        {
                send_to_char("You can't do that!\n\r", ch);
                bug("Edit_buffer: illegal ch->substate (%d)", ch->substate);
                d->connected = CON_PLAYING;
                return;
        }

        if (!ch->editor)
        {
                send_to_char("You can't do that!\n\r", ch);
                bug("Edit_buffer: null editor", 0);
                d->connected = CON_PLAYING;
                return;
        }

        edit = ch->editor;
        save = FALSE;
        max_buf_lines = 95;

        if (argument[0] == '/' || argument[0] == '\\')
        {
                set_char_color(AT_NOTE, ch);
                one_argument(argument, cmd);
                if (!str_cmp(cmd + 1, "?"))
                {
                        send_to_char
                                ("Editing commands\n\r---------------------------------\n\r",
                                 ch);
                        send_to_char("/l              list buffer\n\r", ch);
                        send_to_char("/w              list buffer raw\n\r",
                                     ch);
                        send_to_char("/c              clear buffer\n\r", ch);
                        send_to_char("/d [line]       delete line\n\r", ch);
                        send_to_char("/g <line>       goto line\n\r", ch);
                        send_to_char("/i <line>       insert line\n\r", ch);
                        send_to_char("/r <old> <new>  global replace\n\r",
                                     ch);
                        send_to_char("/n              force a new line\n\r",
                                     ch);
                        send_to_char("/a              abort editing\n\r", ch);
                        send_to_char
                                ("/f              format text ( to fit screen )\n\r",
                                 ch);
                        if (get_trust(ch) > LEVEL_IMMORTAL)
                                send_to_char
                                        ("/! <command>    execute command (do not use another editing command)\n\r",
                                         ch);
                        send_to_char("/s              save buffer\n\r\n\r> ",
                                     ch);
                        return;
                }

                if (!str_cmp(cmd + 1, "c"))
                {

                        memset(edit, '\0', sizeof(EDITOR_DATA));
                        edit->numlines = 0;
                        edit->on_line = 0;
                        send_to_char("Buffer cleared.\n\r> ", ch);
                        return;
                }
                if (!str_cmp(cmd + 1, "r"))
                {
                        char      word1[MAX_INPUT_LENGTH];
                        char      word2[MAX_INPUT_LENGTH];
                        char     *sptr, *wptr, *lwptr;
                        int       count, wordln, word2ln, lineln;

                        sptr = one_argument(argument, word1);
                        sptr = one_argument(sptr, word1);
                        sptr = one_argument(sptr, word2);
                        if (word1[0] == '\0' || word2[0] == '\0')
                        {
                                send_to_char
                                        ("Need word to replace, and replacement.\n\r> ",
                                         ch);
                                return;
                        }
                        if (strcmp(word1, word2) == 0)
                        {
                                send_to_char("Done.\n\r> ", ch);
                                return;
                        }
                        count = 0;
                        wordln = strlen(word1);
                        word2ln = strlen(word2);
                        ch_printf(ch,
                                  "Replacing all occurrences of %s with %s...\n\r",
                                  word1, word2);
                        for (x = 0; x < edit->numlines; x++)
                        {
                                lwptr = edit->line[x];
                                while ((wptr = strstr(lwptr, word1)) != NULL)
                                {
                                        sptr = lwptr;
                                        lwptr = wptr + wordln;
                                        snprintf(buf, MSL, "%s%s", word2,
                                                 wptr + wordln);
                                        lineln = wptr - edit->line[x] -
                                                wordln;
                                        ++count;
                                        if (strlen(buf) + lineln > 79)
                                        {
                                                int       str_len =
                                                        strlen(buf);
                                                lineln = UMAX(0,
                                                              (79 - str_len));
                                                buf[lineln] = '\0';
                                                break;
                                        }
                                        else
                                                lineln = strlen(buf);
                                        buf[lineln] = '\0';
                                        mudstrlcpy(wptr, buf, MIL);
                                }
                        }
                        ch_printf(ch,
                                  "Found and replaced %d occurrence(s).\n\r> ",
                                  count);
                        return;
                }

                if (!str_cmp(cmd + 1, "f"))
                {
                        char      temp_buf[5000];
                        int       ep, old_p, end_mark;
                        int       p = 0;

                        temp_buf[0] = '\0';
                        for (x = 0; x < edit->numlines; x++)
                        {
                                mudstrlcpy(temp_buf + p, edit->line[x], 5000);
                                p += strlen(edit->line[x]);
                                temp_buf[p] = ' ';
                                p++;
                        }

                        temp_buf[p] = '\0';
                        end_mark = p;
                        p = 79;
                        old_p = 0;
                        edit->on_line = 0;
                        edit->numlines = 0;

                        while (old_p < end_mark)
                        {
                                while (temp_buf[p] != ' ' && p > old_p)
                                        p--;

                                if (p == old_p)
                                        p += 79;

                                if (p > end_mark)
                                        p = end_mark;

                                ep = 0;
                                for (x = old_p; x < p; x++)
                                {
                                        edit->line[edit->on_line][ep] =
                                                temp_buf[x];
                                        ep++;
                                }
                                edit->line[edit->on_line][ep] = '\0';

                                edit->on_line++;
                                edit->numlines++;

                                old_p = p + 1;
                                p += 79;

                        }

                        send_to_char("OK.\n\r> ", ch);
                        return;
                }

                if (!str_cmp(cmd + 1, "i"))
                {
                        if (edit->numlines >= max_buf_lines)
                                send_to_char("Buffer is full.\n\r> ", ch);
                        else
                        {
                                if (argument[2] == ' ')
                                        line = atoi(argument + 2) - 1;
                                else
                                        line = edit->on_line;
                                if (line < 0)
                                        line = edit->on_line;
                                if (line < 0 || line > edit->numlines)
                                        send_to_char("Out of range.\n\r> ",
                                                     ch);
                                else
                                {
                                        for (x = ++edit->numlines; x > line;
                                             x--)
                                                mudstrlcpy(edit->line[x],
                                                           edit->line[x - 1],
                                                           160);
                                        mudstrlcpy(edit->line[line], "", 160);
                                        send_to_char("Line inserted.\n\r> ",
                                                     ch);
                                }
                        }
                        return;
                }
                if (!str_cmp(cmd + 1, "d"))
                {
                        if (edit->numlines == 0)
                                send_to_char("Buffer is empty.\n\r> ", ch);
                        else
                        {
                                if (argument[2] == ' ')
                                        line = atoi(argument + 2) - 1;
                                else
                                        line = edit->on_line;
                                if (line < 0)
                                        line = edit->on_line;
                                if (line < 0 || line > edit->numlines)
                                        send_to_char("Out of range.\n\r> ",
                                                     ch);
                                else
                                {
                                        if (line == 0 && edit->numlines == 1)
                                        {
                                                memset(edit, '\0',
                                                       sizeof(EDITOR_DATA));
                                                edit->numlines = 0;
                                                edit->on_line = 0;
                                                send_to_char
                                                        ("Line deleted.\n\r> ",
                                                         ch);
                                                return;
                                        }
                                        for (x = line;
                                             x < (edit->numlines - 1); x++)
                                                mudstrlcpy(edit->line[x],
                                                           edit->line[x + 1],
                                                           160);
                                        mudstrlcpy(edit->
                                                   line[edit->numlines--], "",
                                                   160);
                                        if (edit->on_line > edit->numlines)
                                                edit->on_line =
                                                        edit->numlines;
                                        send_to_char("Line deleted.\n\r> ",
                                                     ch);
                                }
                        }
                        return;
                }
                if (!str_cmp(cmd + 1, "g"))
                {
                        if (edit->numlines == 0)
                                send_to_char("Buffer is empty.\n\r> ", ch);
                        else
                        {
                                if (argument[2] == ' ')
                                        line = atoi(argument + 2) - 1;
                                else
                                {
                                        send_to_char("Goto what line?\n\r> ",
                                                     ch);
                                        return;
                                }
                                if (line < 0)
                                        line = edit->on_line;
                                if (line < 0 || line > edit->numlines)
                                        send_to_char("Out of range.\n\r> ",
                                                     ch);
                                else
                                {
                                        edit->on_line = line;
                                        ch_printf(ch, "(On line %d)\n\r> ",
                                                  line + 1);
                                }
                        }
                        return;
                }
                if (!str_cmp(cmd + 1, "w"))
                {
                        if (edit->numlines == 0)
                                send_to_char("Buffer is empty.\n\r> ", ch);
                        else
                        {
                                char      tmpline[MSL];

                                send_to_char("&G------------------\n\r", ch);
                                for (x = 0; x < edit->numlines; x++)
                                {
                                        snprintf(tmpline, MSL, "%2d> %s\n\r",
                                                 x + 1, edit->line[x]);
                                        write_to_buffer(ch->desc, tmpline, 0);
                                }
                                send_to_char("------------------\n\r> ", ch);
                        }
                        return;
                }
                if (!str_cmp(cmd + 1, "l"))
                {
                        if (edit->numlines == 0)
                                send_to_char("Buffer is empty.\n\r> ", ch);
                        else
                        {
                                send_to_char("&G------------------\n\r", ch);
                                for (x = 0; x < edit->numlines; x++)
                                        ch_printf(ch, "%2d> %s\n\r", x + 1,
                                                  edit->line[x]);
                                send_to_char("&G------------------\n\r> ",
                                             ch);
                        }
                        return;
                }
                if (!str_cmp(cmd + 1, "a"))
                {
                        send_to_char("\n\rAborting... ", ch);
                        stop_editing(ch);
                        return;
                }
                if (get_trust(ch) > LEVEL_IMMORTAL && !str_cmp(cmd + 1, "!"))
                {
                        DO_FUN   *last_cmd;
                        int       substate = ch->substate;

                        last_cmd = ch->last_cmd;
                        ch->substate = SUB_RESTRICTED;
                        interpret(ch, argument + 3);
                        ch->substate = substate;
                        ch->last_cmd = last_cmd;
                        set_char_color(AT_GREEN, ch);
                        send_to_char("\n\r> ", ch);
                        return;
                }
                if (!str_cmp(cmd + 1, "s"))
                {
                        d->connected = CON_PLAYING;
                        if (!ch->last_cmd)
                                return;
                        (*ch->last_cmd) (ch, "");
                        return;
                }
                if (!str_cmp(cmd + 1, "n"))
                {
                        if (edit->numlines >= max_buf_lines)
                                send_to_char("Buffer is full.\n\r> ", ch);
                        else
                        {
                                edit->line[edit->on_line][0] = '\0';
                                edit->on_line++;
                        }
                        return;
                }
        }

        if (edit->size + strlen(argument) + 1 >= MAX_STRING_LENGTH - 1)
                send_to_char("You buffer is full.\n\r", ch);
        else
        {
                if (strlen_color(argument) > 79)
                {
                        char      arg[MSL];

                        buf[0] = '\0';


                        while (argument[0] != '\0')
                        {
                                argument = one_argument3(argument, arg);

                                if (strlen_color(arg) > 79)
                                        continue;

                                if ((strlen_color(buf) + strlen_color(arg) +
                                     1) <= 79)
                                {
                                        char      tmp_buf[MSL];

                                        mudstrlcpy(tmp_buf, buf, MSL);
                                        snprintf(buf, MSL, "%s%s%s", tmp_buf,
                                                 tmp_buf[0] ==
                                                 '\0' ? "" : " ", arg);
                                }
                                else
                                {
                                        if (ch->substate == SUB_MPROG_EDIT)
                                                mudstrlcat(buf, "+", MIL);
                                        mudstrlcpy(edit->
                                                   line[edit->on_line++], buf,
                                                   160);
                                        if (edit->on_line > edit->numlines)
                                                edit->numlines++;
                                        buf[0] = '\0';
                                        mudstrlcat(buf, arg, MIL);
                                }
                                if (edit->numlines > max_buf_lines)
                                {
                                        edit->numlines = max_buf_lines;
                                        send_to_char
                                                ("You've run out of room in the editing buffer.\n\r",
                                                 ch);
                                        save = TRUE;
                                }
                        }
                        mudstrlcpy(edit->line[edit->on_line++], buf, 160);
                        if (edit->on_line > edit->numlines)
                                edit->numlines++;
                        buf[0] = '\0';
                        if (edit->numlines > max_buf_lines)
                        {
                                edit->numlines = max_buf_lines;
                                send_to_char
                                        ("You've run out of room in the editing buffer.\n\r",
                                         ch);
                                save = TRUE;
                        }
                }
                else
                        mudstrlcpy(buf, argument, MIL);
                if (buf[0] != '\0')
                        mudstrlcpy(edit->line[edit->on_line++], buf, 160);
                if (edit->on_line > edit->numlines)
                        edit->numlines++;
                if (edit->numlines > max_buf_lines)
                {
                        edit->numlines = max_buf_lines;
                        send_to_char("Buffer full.\n\r", ch);
                        save = TRUE;
                }
        }


        if (save)
        {
                d->connected = CON_PLAYING;
                if (!ch->last_cmd)
                        return;
                (*ch->last_cmd) (ch, "");
                return;
        }
        send_to_char("> ", ch);
}

CMDF do_notepeek(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char     *buf;

        if ((victim = get_char_world(ch, argument)) == NULL)
        {
                ch_printf(ch, "No such player as %s.\n\r", argument);
                return;
        }
        if (IS_NPC(victim) || victim->editor == NULL)
        {
                ch_printf(ch, "They are not writing anything.\n\r");
                return;
        }
        buf = copy_buffer(victim);
        ch_printf(ch, "%s's buffer says:\n\r%s", victim->name, buf);
        STRFREE(buf);
        return;
}
