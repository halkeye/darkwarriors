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
 *                           ^     +----- |  / ^     ^ |     | +-\                       *
 *                          / \    |      | /  |\   /| |     | |  \                      *
 *                         /   \   +---   |<   | \ / | |     | |  |                      *
 *                        /-----\  |      | \  |  v  | |     | |  /                      *
 *                       /       \ |      |  \ |     | +-----+ +-/                       *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                      Color Module -- Allow user customizable Colors.                  *
 *                                         --Matthew                                     *
 *                             Enhanced ANSI parser by Samson                            *
 ****************************************************************************************/

/*
* The following instructions assume you know at least a little bit about
* coding.  I firmly believe that if you can't code (at least a little bit),
* you don't belong running a mud.  So, with that in mind, I don't hold your
* hand through these instructions.
*
* You may use this code provided that:
*
*     1)  You understand that the authors _DO NOT_ support this code
*         Any help you need must be obtained from other sources.  The
*         authors will ignore any and all requests for help.
*     2)  You will mention the authors if someone asks about the code.
*         You will not take credit for the code, but you can take credit
*         for any enhancements you make.
*     3)  This message remains intact.
*
* If you would like to find out how to send the authors large sums of money,
* you may e-mail the following address:
*
* Matthew Bafford & Christopher Wigginton
* wiggy@mudservices.com
*/

/*
 * To add new color types:
 *
 * 1.  Edit color.h, and:
 *     1.  Add a new AT_ define.
 *     2.  Increment MAX_COLORS by however many AT_'s you added.
 * 2.  Edit color.c and:
 *     1.  Add the name(s) for your new color(s) to the end of the pc_displays array.
 *     2.  Add the default color(s) to the end of the default_set array.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mud.h"
#include "mxp.h"

extern char *const pc_displays[MAX_COLORS];

char     *const pc_displays[MAX_COLORS] = {
        "black", "dred", "dgreen", "orange",
        "dblue", "purple", "cyan", "grey",
        "dgrey", "red", "green", "yellow",
        "blue", "pink", "lblue", "white",
        "blink", "plain", "action", "say",
        "chat", "yells", "tell", "hit",
        "hitme", "immortal", "hurting", "falling",
        "danger", "magic", "consider", "report",
        "poison", "social", "dying", "dead",
        "skills", "lines", "damage", "fleeing",
        "rmname", "rmdesc", "objects", "people",
        "list", "bye", "gold", "gtells",
        "note", "hungry", "thirsty", "fire",
        "sober", "wearoff", "exits", "score",
        "reset", "log", "die_msg", "wartalk",
        "arena", "muse", "think", "aflags",
        "who", "racetalk", "ignore", "whisper",
        "divider", "morph", "shout", "rflags",
        "stype", "aname", "auction", "score2",
        "score3", "score4", "who2", "who3",
        "who4", "intermud", "helpfiles", "who5",
        "score5", "who6", "who7", "prac",
        "prac2", "prac3", "prac4", "mxpprompt",
        "guildtalk", "ship", "clan", "ooc",
        "avatar", "shuttle"
};

/* All defaults are set to Alsherok default scheme, if you don't 
like it, change it around to suite your own needs - Samson */
const sh_int default_set[MAX_COLORS] = {
        AT_BLACK, AT_BLOOD, AT_DGREEN, AT_ORANGE,   /*  3 */
        AT_DBLUE, AT_PURPLE, AT_CYAN, AT_GREY,  /*  7 */
        AT_DGREY, AT_RED, AT_GREEN, AT_YELLOW,  /* 11 */
        AT_BLUE, AT_PINK, AT_LBLUE, AT_WHITE,   /* 15 */

        AT_RED + AT_BLINK, AT_GREY, AT_GREY, AT_LBLUE,  /* 19 */
        AT_GREEN, AT_LBLUE, AT_WHITE, AT_GREY,  /* 23 */
        AT_GREY, AT_YELLOW, AT_GREY, AT_GREY,   /* 27 */
        AT_GREY, AT_BLUE, AT_GREY, AT_GREY, /* 31 */
        AT_DGREEN, AT_CYAN, AT_GREY, AT_GREY,   /* 35 */
        AT_BLUE, AT_BLUE, AT_GREY, AT_GREY, /* 39 */
        AT_WHITE, AT_DGREY, AT_GREEN, AT_PINK,  /* 43 */
        AT_GREY, AT_GREY, AT_YELLOW, AT_GREY,   /* 47 */
        AT_GREY, AT_ORANGE, AT_BLUE, AT_RED,    /* 51 */
        AT_GREY, AT_GREY, AT_GREEN, AT_DGREEN,  /* 55 */
        AT_DGREEN, AT_PURPLE, AT_GREY, AT_RED,  /* 59 */
        AT_GREY, AT_DGREEN, AT_RED, AT_BLUE,    /* 63 */
        AT_RED, AT_CYAN, AT_YELLOW, AT_PINK,    /* 67 */
        AT_DGREEN, AT_PINK, AT_WHITE, AT_BLUE,  /* 71 */
        AT_BLUE, AT_BLUE, AT_GREEN, AT_GREY,    /* 75 */
        AT_GREEN, AT_GREEN, AT_YELLOW, AT_DGREY,    /* 79 */
        AT_GREEN, AT_PINK, AT_DGREEN, AT_CYAN,  /* 83 */
        AT_RED, AT_WHITE, AT_BLUE, AT_DGREEN,   /* 87 */
        AT_CYAN, AT_BLOOD, AT_RED, AT_DGREEN,   /* 91 */
        AT_PINK, AT_CYAN, AT_PINK, AT_YELLOW,   /* 95 */
        AT_BLUE, AT_DGREEN
};

char     *const valid_color[] = {
        "black",
        "dred",
        "dgreen",
        "orange",
        "dblue",
        "purple",
        "cyan",
        "grey",
        "dgrey",
        "red",
        "green",
        "yellow",
        "blue",
        "pink",
        "lblue",
        "white",
        "\0"
};

/* Color align functions by Justice@Aaern */
int const_color_str_len(const char *argument)
{
        int       str, count = 0;
        bool      IS_COLOR = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else
                {
                        if (IS_COLOR == FALSE)
                                count++;
                        else
                                IS_COLOR = FALSE;
                }
        }

        return count;
}

int const_color_strnlen(const char *argument, int maxlength)
{
        int       str, count = 0;
        bool      IS_COLOR = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else
                {
                        if (IS_COLOR == FALSE)
                                count++;
                        else
                                IS_COLOR = FALSE;
                }

                if (count >= maxlength)
                        break;
        }
        if (count < maxlength)
                return ((str - count) + maxlength);

        str++;
        return str;
}

int color_str_len(char *argument)
{
        int       str, count = 0;
        bool      IS_COLOR = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else
                {
                        if (IS_COLOR == FALSE)
                                count++;
                        else
                                IS_COLOR = FALSE;
                }
        }

        return count;
}

int color_strnlen(char *argument, int maxlength)
{
        int       str, count = 0;
        bool      IS_COLOR = FALSE;

        for (str = 0; argument[str] != '\0'; str++)
        {
                if (argument[str] == '&')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
#ifdef OVERLANDCODE
                else if (argument[str] == '{')
#else
                else if (argument[str] == '^')
#endif
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else if (argument[str] == '}')
                {
                        if (IS_COLOR == TRUE)
                        {
                                count++;
                                IS_COLOR = FALSE;
                        }
                        else
                                IS_COLOR = TRUE;
                }
                else
                {
                        if (IS_COLOR == FALSE)
                                count++;
                        else
                                IS_COLOR = FALSE;
                }

                if (count >= maxlength)
                        break;
        }
        if (count < maxlength)
                return ((str - count) + maxlength);

        str++;
        return str;
}

const char *const_color_align(const char *argument, int size, int align)
{
        int       space = (size - const_color_str_len(argument));
        static char buf[MAX_STRING_LENGTH];

        if (align == ALIGN_RIGHT || const_color_str_len(argument) >= size)
                snprintf(buf, MSL, "%*.*s",
                         const_color_strnlen(argument, size),
                         const_color_strnlen(argument, size), argument);
        else if (align == ALIGN_CENTER)
                snprintf(buf, MSL, "%*s%s%*s", (space / 2), "", argument,
                         ((space / 2) * 2) ==
                         space ? (space / 2) : ((space / 2) + 1), "");
        else
                snprintf(buf, MSL, "%s%*s", argument, space, "");

        return buf;
}

char     *color_align(char *argument, int size, int align)
{
        int       space = (size - color_str_len(argument));
        static char buf[MAX_STRING_LENGTH];

        if (align == ALIGN_RIGHT || color_str_len(argument) >= size)
                snprintf(buf, MSL, "%*.*s", color_strnlen(argument, size),
                         color_strnlen(argument, size), argument);
        else if (align == ALIGN_CENTER)
                snprintf(buf, MSL, "%*s%s%*s", (space / 2), "", argument,
                         ((space / 2) * 2) ==
                         space ? (space / 2) : ((space / 2) + 1), "");
        else if (align == ALIGN_LEFT)
                snprintf(buf, MSL, "%s%*s", argument, space, "");

        return buf;
}

void show_colors(CHAR_DATA * ch)
{
        sh_int    count;

        send_to_pager_color
                ("&BSyntax: color [color type] [color] | default\n\r", ch);
        send_to_pager_color
                ("&BSyntax: color _reset_ (Resets all colors to default set)\n\r",
                 ch);
        send_to_pager_color
                ("&BSyntax: color _all_ [color] (Sets all color types to [color])\n\r\n\r",
                 ch);

        send_to_pager_color
                ("&W********************************[ COLORS ]*********************************\n\r",
                 ch);

        for (count = 0; count < 16; ++count)
        {
                if ((count % 8) == 0 && count != 0)
                {
                        send_to_pager("\n\r", ch);
                }
                pager_printf(ch, "%s%-10s", color_str(count, ch),
                             pc_displays[count]);
        }

        send_to_pager
                ("\n\r\n\r&W******************************[ COLOR TYPES ]******************************\n\r",
                 ch);

        for (count = 16; count < MAX_COLORS; ++count)
        {
                if ((count % 8) == 0 && count != 16)
                {
                        send_to_pager("\n\r", ch);
                }
                pager_printf(ch, "%s%-10s%s", color_str(count, ch),
                             pc_displays[count], ANSI_RESET);
        }
        send_to_pager("\n\r\n\r", ch);
        send_to_pager("&YAvailable colors are:\n\r", ch);

        for (count = 0; valid_color[count][0] != '\0'; ++count)
        {
                if ((count % 8) == 0 && count != 0)
                        send_to_pager("\n\r", ch);

                pager_printf(ch, "%s%-10s", color_str(AT_PLAIN, ch),
                             valid_color[count]);
        }
        send_to_pager("\n\r", ch);
        return;
}

CMDF do_color(CHAR_DATA * ch, char *argument)
{
        bool      dMatch, cMatch;
        sh_int    count = 0, y = 0;
        char      arg[MIL];
        char      arg2[MIL];
        char      arg3[MIL];
        char      buf[MSL];

        dMatch = FALSE;
        cMatch = FALSE;

        if (IS_NPC(ch))
        {
                send_to_pager("Only PC's can change colors.\n\r", ch);
                return;
        }

        if (!argument || argument[0] == '\0')
        {
                show_colors(ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "ansitest"))
        {
                snprintf(log_buf, MSL, "%sBlack\n\r", ANSI_BLACK);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Red\n\r", ANSI_DRED);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Green\n\r", ANSI_DGREEN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sOrange/Brown\n\r", ANSI_ORANGE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Blue\n\r", ANSI_DBLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPurple\n\r", ANSI_PURPLE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sCyan\n\r", ANSI_CYAN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGrey\n\r", ANSI_GREY);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Grey\n\r", ANSI_DGREY);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sRed\n\r", ANSI_RED);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGreen\n\r", ANSI_GREEN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sYellow\n\r", ANSI_YELLOW);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlue\n\r", ANSI_BLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPink\n\r", ANSI_PINK);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sLight Blue\n\r", ANSI_LBLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sWhite\n\r", ANSI_WHITE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlack\n\r", BLINK_BLACK);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Red\n\r", BLINK_DRED);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Green\n\r", BLINK_DGREEN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sOrange/Brown\n\r", BLINK_ORANGE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Blue\n\r", BLINK_DBLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPurple\n\r", BLINK_PURPLE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sCyan\n\r", BLINK_CYAN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGrey\n\r", BLINK_GREY);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sDark Grey\n\r", BLINK_DGREY);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sRed\n\r", BLINK_RED);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sGreen\n\r", BLINK_GREEN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sYellow\n\r", BLINK_YELLOW);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sBlue\n\r", BLINK_BLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sPink\n\r", BLINK_PINK);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sLight Blue\n\r", BLINK_LBLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%sWhite\n\r", BLINK_WHITE);
                write_to_buffer(ch->desc, log_buf, 0);
                write_to_buffer(ch->desc, ANSI_RESET, 0);
                snprintf(log_buf, MSL, "%s%sBlack\n\r", ANSI_WHITE,
                         BACK_BLACK);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Red\n\r", ANSI_BLACK,
                         BACK_DRED);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Green\n\r", ANSI_BLACK,
                         BACK_DGREEN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sOrange/Brown\n\r", ANSI_BLACK,
                         BACK_ORANGE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sDark Blue\n\r", ANSI_BLACK,
                         BACK_DBLUE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sPurple\n\r", ANSI_BLACK,
                         BACK_PURPLE);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sCyan\n\r", ANSI_BLACK, BACK_CYAN);
                write_to_buffer(ch->desc, log_buf, 0);
                snprintf(log_buf, MSL, "%s%sGrey\n\r", ANSI_BLACK, BACK_GREY);
                write_to_buffer(ch->desc, log_buf, 0);
                write_to_buffer(ch->desc, ANSI_RESET, 0);

                return;
        }

        if (!str_prefix(arg, "_reset_"))
        {
                reset_colors(ch);
                send_to_pager("All color types reset to default colors.\n\r",
                              ch);
                return;
        }

        argument = one_argument(argument, arg2);

        if (arg[0] == '\0')
        {
                send_to_char("Change which color type?\r\n", ch);
                return;
        }

        argument = one_argument(argument, arg3);

        if (!str_prefix(arg, "_all_"))
        {
                dMatch = TRUE;
                count = -1;

                /*
                 * search for a valid color setting
                 */
                for (y = 0; y < 16; y++)
                {
                        if (!str_cmp(arg2, valid_color[y]))
                        {
                                cMatch = TRUE;
                                break;
                        }
                }
        }
        else if (arg2[0] == '\0')
        {
                cMatch = FALSE;
        }
        else
        {
                /*
                 * search for the display type and str_cmp
                 */
                for (count = 0; count < MAX_COLORS; count++)
                {
                        if (!str_prefix(arg, pc_displays[count]))
                        {
                                dMatch = TRUE;
                                break;
                        }
                }

                if (!dMatch)
                {
                        ch_printf(ch, "%s is an invalid color type.\n\r",
                                  arg);
                        send_to_char
                                ("Type color with no arguments to see available options.\n\r",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "default"))
                {
                        ch->colors[count] = default_set[count];
                        snprintf(buf, MSL,
                                 "Display %s set back to default.\n\r",
                                 pc_displays[count]);
                        send_to_pager(buf, ch);
                        return;
                }

                /*
                 * search for a valid color setting
                 */
                for (y = 0; y < 16; y++)
                {
                        if (!str_cmp(arg2, valid_color[y]))
                        {
                                cMatch = TRUE;
                                break;
                        }
                }
        }

        if (!cMatch)
        {
                if (arg[0])
                {
                        ch_printf(ch, "Invalid color for type %s.\n", arg);
                }
                else
                {
                        send_to_pager("Invalid color.\n\r", ch);
                }

                send_to_pager("Choices are:\n\r", ch);

                for (count = 0; count < 16; count++)
                {
                        if (count % 5 == 0 && count != 0)
                                send_to_pager("\r\n", ch);

                        pager_printf(ch, "%-10s", valid_color[count]);
                }

                pager_printf(ch, "%-10s\r\n", "default");
                return;
        }
        else
        {
                snprintf(buf, MSL, "Color type %s set to color %s.\n\r",
                         count == -1 ? "_all_" : pc_displays[count],
                         valid_color[y]);
        }

        if (!str_cmp(arg3, "blink"))
        {
                y += AT_BLINK;
        }

        if (count == -1)
        {
                int       ccount;

                for (ccount = 0; ccount < MAX_COLORS; ++ccount)
                {
                        ch->colors[ccount] = y;
                }

                set_pager_color(y, ch);

                snprintf(buf, MSL, "All color types set to color %s%s.%s\n\r",
                         valid_color[y > AT_BLINK ? y - AT_BLINK : y],
                         y > AT_BLINK ? " [BLINKING]" : "", ANSI_RESET);

                send_to_pager(buf, ch);
        }
        else
        {
                ch->colors[count] = y;

                set_pager_color(count, ch);

                if (!str_cmp(arg3, "blink"))
                        snprintf(buf, MSL,
                                 "Display %s set to color %s [BLINKING]%s\n\r",
                                 pc_displays[count],
                                 valid_color[y - AT_BLINK], ANSI_RESET);
                else
                        snprintf(buf, MSL, "Display %s set to color %s.\n\r",
                                 pc_displays[count], valid_color[y]);

                send_to_pager(buf, ch);
        }
        set_pager_color(AT_PLAIN, ch);

        return;
}

void reset_colors(CHAR_DATA * ch)
{
        memcpy(&ch->colors, &default_set, sizeof(default_set));
}

char     *color_str(sh_int AType, CHAR_DATA * ch)
{
        if (!ch)
        {
                bug("%s", "color_str: NULL ch!");
                return ("");
        }

        if (IS_NPC(ch) || !IS_SET(ch->act, PLR_ANSI))
                return ("");

        switch (ch->colors[AType])
        {
        case 0:
                return (ANSI_BLACK);
                break;
        case 1:
                return (ANSI_DRED);
                break;
        case 2:
                return (ANSI_DGREEN);
                break;
        case 3:
                return (ANSI_ORANGE);
                break;
        case 4:
                return (ANSI_DBLUE);
                break;
        case 5:
                return (ANSI_PURPLE);
                break;
        case 6:
                return (ANSI_CYAN);
                break;
        case 7:
                return (ANSI_GREY);
                break;
        case 8:
                return (ANSI_DGREY);
                break;
        case 9:
                return (ANSI_RED);
                break;
        case 10:
                return (ANSI_GREEN);
                break;
        case 11:
                return (ANSI_YELLOW);
                break;
        case 12:
                return (ANSI_BLUE);
                break;
        case 13:
                return (ANSI_PINK);
                break;
        case 14:
                return (ANSI_LBLUE);
                break;
        case 15:
                return (ANSI_WHITE);
                break;

                /*
                 * 16 thru 31 are for blinking colors 
                 */
        case 16:
                return (BLINK_BLACK);
                break;
        case 17:
                return (BLINK_DRED);
                break;
        case 18:
                return (BLINK_DGREEN);
                break;
        case 19:
                return (BLINK_ORANGE);
                break;
        case 20:
                return (BLINK_DBLUE);
                break;
        case 21:
                return (BLINK_PURPLE);
                break;
        case 22:
                return (BLINK_CYAN);
                break;
        case 23:
                return (BLINK_GREY);
                break;
        case 24:
                return (BLINK_DGREY);
                break;
        case 25:
                return (BLINK_RED);
                break;
        case 26:
                return (BLINK_GREEN);
                break;
        case 27:
                return (BLINK_YELLOW);
                break;
        case 28:
                return (BLINK_BLUE);
                break;
        case 29:
                return (BLINK_PINK);
                break;
        case 30:
                return (BLINK_LBLUE);
                break;
        case 31:
                return (BLINK_WHITE);
                break;

        default:
                return (ANSI_RESET);
                break;
        }
}

int colorcode(const char *col, char *code, CHAR_DATA * ch)
{
        const char *ctype = col;
        int       ln;
        bool      ansi;


        if (!ch || ch->gold == 0)
                ansi = TRUE;
        else
                ansi = (!IS_NPC(ch) && IS_SET(ch->act, PLR_ANSI));

        col++;

        if (!*col)
                ln = -1;
#ifdef OVERLANDCODE
        else if (*ctype != '&' && *ctype != '{' && *ctype != '}')
        {
                bug("colorcode: command '%c' not '&', '{' or '}'", *ctype);
                ln = -1;
        }
#else
        else if (*ctype != '&' && *ctype != '^' && *ctype != '}')
        {
                bug("colorcode: command '%c' not '&', '^' or '}'", *ctype);
                ln = -1;
        }
#endif
        else if (*col == *ctype)
        {
                code[0] = *col;
                code[1] = '\0';
                ln = 1;
        }
        else if (!ansi)
                ln = 0;
        else
        {
                /*
                 * Foreground text - non-blinking 
                 */
                if (*ctype == '&')
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'i':  /* Italic text */
                        case 'I':
                                mudstrlcpy(code, ANSI_ITALIC, 20);
                                break;
                        case 'v':  /* Reverse colors */
                        case 'V':
                                mudstrlcpy(code, ANSI_REVERSE, 20);
                                break;
                        case 'u':  /* Underline */
                        case 'U':
                                mudstrlcpy(code, ANSI_UNDERLINE, 20);
                                break;
                        case 's':  /* Strikeover */
                        case 'S':
                                mudstrlcpy(code, ANSI_STRIKEOUT, 20);
                                break;
                        case 'd':  /* Player's client default color */
                                mudstrlcpy(code, ANSI_RESET, 20);
                                break;
                        case 'D':  /* Reset to custom color for whatever is being displayed */
                                mudstrlcpy(code, ANSI_RESET, 20);   /* Yes, this reset here is quite necessary to cancel out other things */
                                mudstrlcat(code,
                                           color_str(ch->desc->pagecolor, ch),
                                           20);
                                break;
                        case 'x':  /* Black */
                                mudstrlcpy(code, ANSI_BLACK, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, ANSI_ORANGE, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, ANSI_CYAN, 20);
                                break;
                        case 'z':  /* Dark Grey */
                                mudstrlcpy(code, ANSI_DGREY, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, ANSI_DGREEN, 20);
                                break;
                        case 'G':  /* Light Green */
                                mudstrlcpy(code, ANSI_GREEN, 20);
                                break;
                        case 'P':  /* Pink/Light Purple */
                                mudstrlcpy(code, ANSI_PINK, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, ANSI_DRED, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, ANSI_DBLUE, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, ANSI_GREY, 20);
                                break;
                        case 'Y':  /* Yellow */
                                mudstrlcpy(code, ANSI_YELLOW, 20);
                                break;
                        case 'C':  /* Light Blue */
                                mudstrlcpy(code, ANSI_LBLUE, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, ANSI_PURPLE, 20);
                                break;
                        case 'R':  /* Red */
                                mudstrlcpy(code, ANSI_RED, 20);
                                break;
                        case 'B':  /* Blue */
                                mudstrlcpy(code, ANSI_BLUE, 20);
                                break;
                        case 'W':  /* White */
                                mudstrlcpy(code, ANSI_WHITE, 20);
                                break;
                        }
                }
                /*
                 * Foreground text - blinking 
                 */
                if (*ctype == '}')
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'x':  /* Black */
                                mudstrlcpy(code, BLINK_BLACK, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, BLINK_ORANGE, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, BLINK_CYAN, 20);
                                break;
                        case 'z':  /* Dark Grey */
                                mudstrlcpy(code, BLINK_DGREY, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, BLINK_DGREEN, 20);
                                break;
                        case 'G':  /* Light Green */
                                mudstrlcpy(code, BLINK_GREEN, 20);
                                break;
                        case 'P':  /* Pink/Light Purple */
                                mudstrlcpy(code, BLINK_PINK, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, BLINK_DRED, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, BLINK_DBLUE, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, BLINK_GREY, 20);
                                break;
                        case 'Y':  /* Yellow */
                                mudstrlcpy(code, BLINK_YELLOW, 20);
                                break;
                        case 'C':  /* Light Blue */
                                mudstrlcpy(code, BLINK_LBLUE, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, BLINK_PURPLE, 20);
                                break;
                        case 'R':  /* Red */
                                mudstrlcpy(code, BLINK_RED, 20);
                                break;
                        case 'B':  /* Blue */
                                mudstrlcpy(code, BLINK_BLUE, 20);
                                break;
                        case 'W':  /* White */
                                mudstrlcpy(code, BLINK_WHITE, 20);
                                break;
                        }
                }
                /*
                 * Background color 
                 */
#ifdef OVERLANDCODE
                if (*ctype == '{')
#else
                if (*ctype == '^')
#endif
                {
                        switch (*col)
                        {
                        default:
                                code[0] = *ctype;
                                code[1] = *col;
                                code[2] = '\0';
                                return 2;
                        case 'x':  /* Black */
                                mudstrlcpy(code, BACK_BLACK, 20);
                                break;
                        case 'r':  /* Dark Red */
                                mudstrlcpy(code, BACK_DRED, 20);
                                break;
                        case 'g':  /* Dark Green */
                                mudstrlcpy(code, BACK_DGREEN, 20);
                                break;
                        case 'O':  /* Orange/Brown */
                                mudstrlcpy(code, BACK_ORANGE, 20);
                                break;
                        case 'b':  /* Dark Blue */
                                mudstrlcpy(code, BACK_DBLUE, 20);
                                break;
                        case 'p':  /* Purple */
                                mudstrlcpy(code, BACK_PURPLE, 20);
                                break;
                        case 'c':  /* Cyan */
                                mudstrlcpy(code, BACK_CYAN, 20);
                                break;
                        case 'w':  /* Grey */
                                mudstrlcpy(code, BACK_GREY, 20);
                                break;
                        }
                }
                ln = strlen(code);
        }
        if (ln <= 0)
                *code = '\0';
        return ln;
}

/* Moved from comm.c */
void set_char_color(sh_int AType, CHAR_DATA * ch)
{
        if (!ch || !ch->desc)
                return;

        write_to_buffer(ch->desc, color_str(AType, ch), 0);
        ch->desc->pagecolor = ch->colors[AType];
}

void set_pager_color(sh_int AType, CHAR_DATA * ch)
{
        if (!ch || !ch->desc)
                return;

        write_to_pager(ch->desc, color_str(AType, ch), 0);
        ch->desc->pagecolor = ch->colors[AType];
}

void write_to_pager(DESCRIPTOR_DATA * d, const char *txt, int length)
{
        int       pageroffset;  /* Pager fix by thoric */
        int       origlength = 0;

        if (length <= 0)
                length = strlen(txt);

        /*
         * Find length in case caller didn't. 
         */
        if (length == 0)
                return;

        origlength = length;
        /*
         * How much space do we need to expand stuff 
         */
        length += count_mxp_tags(d, txt, length);

        if (!d->pagebuf)
        {
                d->pagesize = MAX_STRING_LENGTH;
                CREATE(d->pagebuf, char, d->pagesize);
        }
        if (!d->pagepoint)
        {
                d->pagepoint = d->pagebuf;
                d->pagetop = 0;
                d->pagecmd = '\0';
        }
        if (d->pagetop == 0 && !d->fcommand)
        {
                d->pagebuf[0] = '\n';
                d->pagebuf[1] = '\r';
                d->pagetop = 2;
        }
        pageroffset = d->pagepoint - d->pagebuf;    /* pager fix (goofup fixed 08/21/97) */
        while (d->pagetop + length >= d->pagesize)
        {
                if (d->pagesize > MSL * 16)
                {
                        bug("%s", "Pager overflow.  Ignoring.\n\r");
                        d->pagetop = 0;
                        d->pagepoint = NULL;
                        DISPOSE(d->pagebuf);
                        d->pagesize = MSL;
                        return;
                }
                d->pagesize *= 2;
                RECREATE(d->pagebuf, char, d->pagesize);
        }
        d->pagepoint = d->pagebuf + pageroffset;    /* pager fix (goofup fixed 08/21/97) */
/*   mudstrlcpy( d->pagebuf + d->pagetop, txt, length ); */
        convert_mxp_tags(d, d->pagebuf + d->pagetop, txt, origlength);
        d->pagetop += length;
        d->pagebuf[d->pagetop] = '\0';
        return;
}

/* Writes to a descriptor, usually best used when there's no character to send to ( like logins ) */
void send_to_desc_color(const char *txt, DESCRIPTOR_DATA * d)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (!d)
        {
                bug("%s", "send_to_desc_color: NULL *d");
                return;
        }

        if (!txt || !d->descriptor)
                return;

        while ((colstr = strpbrk(prevstr, "&^}")) != NULL)
        {
                if (colstr > prevstr)
                        write_to_buffer(d, prevstr, (colstr - prevstr));

                ln = colorcode(colstr, colbuf, d->character);
                if (ln < 0)
                {
                        prevstr = colstr + 1;
                        break;
                }

                else if (ln > 0)
                        write_to_buffer(d, colbuf, ln);
                prevstr = colstr + 2;
        }
        if (*prevstr)
                write_to_buffer(d, prevstr, 0);

        return;
}

/*
 * Write to one char. Convert color into ANSI sequences.
 */
void send_to_char_color(const char *txt, CHAR_DATA * ch)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (!ch)
        {
                bug("%s", "send_to_char_color: NULL ch!");
                return;
        }

        if (txt && ch->desc)
        {
#ifdef OVERLANDCODE
                while ((colstr = strpbrk(prevstr, "&{}")) != NULL)
#else
                while ((colstr = strpbrk(prevstr, "&^}")) != NULL)
#endif
                {
                        if (colstr > prevstr)
                                write_to_buffer(ch->desc, prevstr,
                                                (colstr - prevstr));
                        if (!ch->desc)
                                return;
                        ln = colorcode(colstr, colbuf, ch);
                        if (ln < 0)
                        {
                                prevstr = colstr + 1;
                                break;
                        }
                        else if (ln > 0)
                                write_to_buffer(ch->desc, colbuf, ln);
                        prevstr = colstr + 2;
                }
                if (*prevstr)
                        write_to_buffer(ch->desc, prevstr, 0);
        }
        return;
}

void send_to_pager_color(const char *txt, CHAR_DATA * ch)
{
        char     *colstr;
        const char *prevstr = txt;
        char      colbuf[20];
        int       ln;

        if (IS_NPC(ch)) /* NPCs can't do pager */
                send_to_char_color(txt, ch);

        if (!ch)
        {
                bug("%s", "send_to_pager_color: NULL ch!");
                return;
        }

        if (txt && ch->desc)
        {
                DESCRIPTOR_DATA *d = ch->desc;

                ch = d->original ? d->original : d->character;
                if (IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON))
                {
                        send_to_char_color(txt, d->character);
                        return;
                }
#ifdef OVERLANDCODE
                while ((colstr = strpbrk(prevstr, "&{}")) != NULL)
#else
                while ((colstr = strpbrk(prevstr, "&^}")) != NULL)
#endif
                {
                        if (colstr > prevstr)
                                write_to_pager(ch->desc, prevstr,
                                               (colstr - prevstr));
                        ln = colorcode(colstr, colbuf, ch);
                        if (ln < 0)
                        {
                                prevstr = colstr + 1;
                                break;
                        }
                        else if (ln > 0)
                                write_to_pager(ch->desc, colbuf, ln);
                        prevstr = colstr + 2;
                }
                if (*prevstr)
                        write_to_pager(ch->desc, prevstr, 0);
        }
        return;
}

void send_to_char(const char *txt, CHAR_DATA * ch)
{
        send_to_char_color(txt, ch);
        return;
}

void send_to_pager(const char *txt, CHAR_DATA * ch)
{
        if (IS_NPC(ch))
                send_to_char_color(txt, ch);
        else
                send_to_pager_color(txt, ch);
        return;
}

void ch_printf(CHAR_DATA * ch, char *fmt, ...)
{
        char      buf[MSL * 4];
        va_list   args;

        va_start(args, fmt);
        vsnprintf(buf, MSL * 4, fmt, args);
        va_end(args);

        send_to_char_color(buf, ch);
}

void pager_printf(CHAR_DATA * ch, char *fmt, ...)
{
        char      buf[MSL * 2];
        va_list   args;

        va_start(args, fmt);
        vsnprintf(buf, MSL, fmt, args);
        va_end(args);

        send_to_pager_color(buf, ch);
}
