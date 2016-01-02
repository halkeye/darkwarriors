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
 *                                SWR OLC Channel module                                 *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"
#include <unistd.h>

CMDF do_create_list(CHAR_DATA * ch, char *argument)
{
        FILE     *fp;
        char      filename[256];

        snprintf(filename, 256, "%s%s", LIST_DIR, argument);

        if (file_exist(filename))
        {
                send_to_char("File already exists.\n\r", ch);
                return;
        }

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_body: fopen", 0);
                perror(filename);
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        send_to_char("File created sucessfully.\n\r", ch);

}

CMDF do_addto_list(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        char      arg1[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);

        if ((arg1[0] == '\0') || (argument[0] == '\0'))
        {
                send_to_char
                        ("Syntax: addtolist <list filename> <text to add>\n\r",
                         ch);
                return;
        }

        snprintf(filename, 256, "%s%s", LIST_DIR, arg1);

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }

        append_to_file(filename, argument);
        send_to_char("Text added to list.\n\r", ch);

}

CMDF do_showlist(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        FILE     *fp;
        int       line = 0;
        char     *string;

        if (argument[0] == '\0')
        {
                send_to_char("Syntax: showlist <list filename>\n\r", ch);
                return;
        }

        snprintf(filename, 256, "%s%s", LIST_DIR, argument);

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }
        else
        {
                send_to_char("&w", ch);
                if ((fp = fopen(filename, "r")) != NULL)
                {
                        for (;;)
                        {
                                string = feof(fp) ? (char *) "end" :
                                        fread_line(fp);
                                if (!str_cmp(string, "end"))
                                        break;
                                ch_printf(ch, "%4d) %s", ++line, string);
                        }
                }
                FCLOSE(fp);
                fpReserve = fopen(NULL_FILE, "r");
                ch_printf(ch, "&B[&w%d&B] &zlines in %s\n\r", line, filename);
        }
}

CMDF do_remlist(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        char      fntemp[256];
        FILE     *fp = NULL;
        FILE     *fptemp = NULL;
        int       line = 0;
        int       linetemp = 0;
        char     *string;
        char      arg1[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);

        if ((arg1[0] == '\0') || (argument[0] == '\0'))
        {
                send_to_char
                        ("Syntax: remtolist <list filename> <line to remove>\n\r",
                         ch);
                return;
        }

        if (!is_number(argument))
        {
                send_to_char
                        ("Syntax: line must be referenced by a number\n\r",
                         ch);
                return;
        }

        snprintf(filename, 256, "%s%s", LIST_DIR, arg1);
        snprintf(fntemp, 256, "%s%s.tmp", LIST_DIR, arg1);

        if (!file_exist(filename))
        {
                send_to_char("File does not exist.\n\r", ch);
                return;
        }

        if ((fp = fopen(filename, "r")) != NULL
            && (fptemp = fopen(fntemp, "w")) != NULL)
        {
                for (;;)
                {
                        string = feof(fp) ? (char *) "end" : fread_line(fp);
                        if (!str_cmp(string, "end"))
                                break;
                        ++line;
                        fprintf(fptemp, "%s", string);
                }
        }

        if (fp)
                FCLOSE(fp);
        if (fptemp)
                FCLOSE(fptemp);

        if ((atoi(argument)) > line)
        {
                send_to_char("Line number invalid.\n\r", ch);
                return;
        }

        if ((fp = fopen(filename, "w")) != NULL
            && (fptemp = fopen(fntemp, "r")) != NULL)
        {
                for (;;)
                {
                        string = feof(fptemp) ? (char *) "end" :
                                fread_line(fptemp);
                        if (!str_cmp(string, "end"))
                                break;
                        ++linetemp;
                        if (linetemp == atoi(argument))
                                continue;
                        fprintf(fp, "%s", string);
                }
        }

        if (fp)
                FCLOSE(fp);
        if (fptemp)
                FCLOSE(fptemp);
        fpReserve = fopen(NULL_FILE, "r");
        unlink(fntemp);

}
