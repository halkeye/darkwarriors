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
 *                            Ban module by Shaddai                                      *
 *****************************************************************************************
 * This module was originally for SMAUG coded by Shaddai, but has since been modified for* 
 * Dark Warriors and SWR.                                                                *
 *****************************************************************************************
 *                $Id: ban.c 1330 2005-12-05 03:23:24Z halkeye $                  *
 *****************************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"
#include "ban.h"
#include "editor.h"
#include "races.h"

/* Local functions */
void fread_ban args((FILE * fp, int type));
bool check_expire args((BAN_DATA * ban));
void dispose_ban args((BAN_DATA * ban, int type));
void free_ban args((BAN_DATA * pban));

/* Global Variables */

BAN_DATA *first_ban;
BAN_DATA *last_ban;
BAN_DATA *first_ban_class;
BAN_DATA *last_ban_class;
BAN_DATA *first_ban_race;
BAN_DATA *last_ban_race;

/*
 * Load all those nasty bans up :)
 * 	Shaddai
 */
void load_banlist(void)
{
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        FILE     *fp;
        bool      fMatch = FALSE;

        first_ban_class = NULL;
        last_ban_class = NULL;
        first_ban_race = NULL;
        last_ban_race = NULL;
        first_ban = NULL;
        last_ban = NULL;

        if (!(fp = fopen(SYSTEM_DIR BAN_LIST, "r")))
        {
                bug("Save_banlist: Cannot open " BAN_LIST, 0);
                perror(BAN_LIST);
                return;
        }
        for (;;)
        {
                word = feof(fp) ? "END" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case 'C':
                        if (!str_cmp(word, "CLASS"))
                        {
                                fread_ban(fp, BAN_CLASS);
                                fMatch = TRUE;
                        }
                        break;
                case 'E':
                        if (!str_cmp(word, "END"))  /*File should always contain END */
                        {
                                FCLOSE(fp);
                                boot_log("Done.");
                                return;
                        }
                case 'R':
                        if (!str_cmp(word, "RACE"))
                        {
                                fread_ban(fp, BAN_RACE);
                                fMatch = TRUE;
                        }
                        break;
                case 'S':
                        if (!str_cmp(word, "SITE"))
                        {
                                fread_ban(fp, BAN_SITE);
                                fMatch = TRUE;
                        }
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "Load_banlist: no match: %s",
                                 word);
                        bug(buf, 0);
                }   /* End of switch statement */
        }   /* End of for loop */
}

/*
 * Load up one class or one race ban structure.
 */

void fread_ban(FILE * fp, int type)
{
        BAN_DATA *pban;
        unsigned int i = 0;
        bool      fMatch = FALSE;

        CREATE(pban, BAN_DATA, 1);

        pban->name = fread_string_nohash(fp);
        pban->user = NULL;
        pban->level = fread_number(fp);
        pban->duration = fread_number(fp);
        pban->unban_date = fread_number(fp);
        if (type == BAN_SITE)
        {   /* Sites have 2 extra numbers written out */
                pban->prefix = fread_number(fp);
                pban->suffix = fread_number(fp);
        }
        pban->warn = fread_number(fp);
        pban->ban_by = fread_string_nohash(fp);
        pban->ban_time = fread_string_nohash(fp);
        pban->note = fread_string(fp);

        /*
         * Need to lookup the class or race number if it is of that type 
         */

        if (type == BAN_CLASS)
        {
                for (i = 0; i < MAX_ABILITY; i++)
                {
                        if (!str_cmp(ability_name[i], pban->name))
                        {
                                fMatch = TRUE;
                                break;
                        }
                }
        }
        else if (type == BAN_RACE)
        {
                RACE_DATA *race = NULL;

                FOR_EACH_LIST(RACE_LIST, races, race)
                {
                        if (!str_cmp(race->name(), pban->name))
                        {
                                fMatch = TRUE;
                                break;
                        }
                }
        }
        else if (type == BAN_SITE)
        {
                for (i = 0; i < strlen(pban->name); i++)
                {
                        if (pban->name[i] == '@')
                        {
                                char     *temp;
                                char     *temp2;

                                temp = str_dup(pban->name);
                                temp[i] = '\0';
                                temp2 = &pban->name[i + 1];
                                DISPOSE(pban->name);
                                pban->name = str_dup(temp2);
                                pban->user = str_dup(temp);
                                DISPOSE(temp);
                                break;
                        }
                }
        }

        if (type == BAN_RACE || type == BAN_CLASS)
        {
                if (fMatch)
                        pban->flag = i;
                else    /* The file is corupted throw out this ban structure */
                {
                        bug("Bad class structure %d.\n\r", i);
                        free_ban(pban);
                        return;
                }
        }
        if (type == BAN_CLASS)
                LINK(pban, first_ban_class, last_ban_class, next, prev);
        else if (type == BAN_RACE)
                LINK(pban, first_ban_race, last_ban_race, next, prev);
        else if (type == BAN_SITE)
                LINK(pban, first_ban, last_ban, next, prev);
        else    /* Bad type throw out the ban structure */
        {
                bug("Fread_ban: Bad type %d", type);
                free_ban(pban);
        }
        return;
}

/*
 * Saves all bans, for sites, classes and races.
 * Shaddai
 */

void save_banlist(void)
{
        BAN_DATA *pban;
        FILE     *fp;

        FCLOSE(fpReserve);
        if (!(fp = fopen(SYSTEM_DIR BAN_LIST, "w")))
        {
                bug("Save_banlist: Cannot open " BAN_LIST, 0);
                perror(BAN_LIST);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        /*
         * Print out all the site bans 
         */

        for (pban = first_ban; pban; pban = pban->next)
        {
                fprintf(fp, "SITE\n");
                if (pban->user)
                        fprintf(fp, "%s@%s~\n", pban->user, pban->name);
                else
                        fprintf(fp, "%s~\n", pban->name);
                fprintf(fp, "%d %d %d %d %d %d\n", pban->level,
                        pban->duration, pban->unban_date, pban->prefix,
                        pban->suffix, pban->warn);
                fprintf(fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time,
                        pban->note);
        }

        /*
         * Print out all the race bans 
         */

        for (pban = first_ban_race; pban; pban = pban->next)
        {
                fprintf(fp, "RACE\n");
                fprintf(fp, "%s~\n", pban->name);
                fprintf(fp, "%d %d %d %d\n", pban->level, pban->duration,
                        pban->unban_date, pban->warn);
                fprintf(fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time,
                        pban->note);
        }

        /*
         * Print out all the class bans 
         */

        for (pban = first_ban_class; pban; pban = pban->next)
        {
                fprintf(fp, "CLASS\n");
                fprintf(fp, "%s~\n", pban->name);
                fprintf(fp, "%d %d %d %d\n", pban->level, pban->duration,
                        pban->unban_date, pban->warn);
                fprintf(fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time,
                        pban->note);
        }
        fprintf(fp, "END\n");   /* File must have an END even if empty */
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

/*
 * The main command for ban, lots of arguments so be carefull what you
 * change here.		Shaddai
 */

CMDF do_ban(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        char      arg4[MAX_INPUT_LENGTH];
        char     *temp;
        BAN_DATA *pban;
        int       value = 0, time;

        if (IS_NPC(ch)) /* Don't want mobs banning sites ;) */
        {
                send_to_char("Monsters are too dumb to do that!\n\r", ch);
                return;
        }

        if (!ch->desc)  /* No desc means no go :) */
        {
                bug("do_ban: no descriptor", 0);
                return;
        }

        set_char_color(AT_IMMORT, ch);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg4);

        /*
         * Do we have a time duration for the ban? 
         */

        if (arg4[0] != '\0' && is_number(arg4))
                time = atoi(arg4);
        else
                time = -1;


        /*
         * -1 is default, but no reason the time should be greater than 1000
         * * or less than 1, after all if it is greater than 1000 you are talking
         * * around 3 years.
         */

        if (time != -1 && (time < 1 || time > 1000))
        {
                send_to_char
                        ("Time value is -1 (forever) or from 1 to 1000.\n\r",
                         ch);
                return;
        }

        /*
         * Need to be carefull with sub-states or everything will get messed up.
         */

        switch (ch->substate)
        {
        default:
                bug("do_ban: illegal substate", 0);
                return;
        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot use this command from within another command.\n\r",
                         ch);
                return;
        case SUB_NONE:
                ch->tempnum = SUB_NONE;
                break;
                /*
                 * Returning to end the editing of the note 
                 */
        case SUB_BAN_DESC:
                add_ban(ch, "", "", 0, 0);
                return;
        }
        if (arg1[0] == '\0')
                goto syntax_message;

        /*
         * If no args are sent after the class/site/race, show the current banned
         * * items.  Shaddai
         */

        if (!str_cmp(arg1, "site"))
        {
                if (arg2[0] == '\0')
                {
                        show_bans(ch, BAN_SITE);
                        return;
                }

                /*
                 * Are they high enough to ban sites? 
                 */

                if (get_trust(ch) < sysdata.ban_site_level)
                {
                        ch_printf(ch, "You must be %d level to add bans.\n\r",
                                  sysdata.ban_site_level);
                        return;
                }
                if (arg3[0] == '\0')
                        goto syntax_message;
                if (!add_ban(ch, arg2, arg3, time, BAN_SITE))
                        return;
        }
        else if (!str_cmp(arg1, "race"))
        {
                if (arg2[0] == '\0')
                {
                        show_bans(ch, BAN_RACE);
                        return;
                }

                /*
                 * Are they high enough level to ban races? 
                 */

                if (get_trust(ch) < sysdata.ban_race_level)
                {
                        ch_printf(ch, "You must be %d level to add bans.\n\r",
                                  sysdata.ban_race_level);
                        return;
                }
                if (arg3[0] == '\0')
                        goto syntax_message;
                if (!add_ban(ch, arg2, arg3, time, BAN_RACE))
                        return;
        }
        else if (!str_cmp(arg1, "class"))
        {
                if (arg2[0] == '\0')
                {
                        show_bans(ch, BAN_CLASS);
                        return;
                }

                /*
                 * Are they high enough to ban classes? 
                 */

                if (get_trust(ch) < sysdata.ban_class_level)
                {
                        ch_printf(ch, "You must be %d level to add bans.\n\r",
                                  sysdata.ban_class_level);
                        return;
                }
                if (arg3[0] == '\0')
                        goto syntax_message;
                if (!add_ban(ch, arg2, arg3, time, BAN_CLASS))
                        return;
        }
        else if (!str_cmp(arg1, "show"))
        {
                /*
                 * This will show the note attached to a ban 
                 */

                if (arg2[0] == '\0' || arg3[0] == '\0')
                        goto syntax_message;
                temp = arg3;
                if (arg3[0] == '#') /* Use #1 to show the first ban */
                {
                        temp = arg3;
                        temp++;
                        if (!is_number(temp))
                        {
                                send_to_char("Which ban # to show?\n\r", ch);
                                return;
                        }
                        value = atoi(temp);
                        if (value < 1)
                        {
                                send_to_char
                                        ("You must specify a number greater than 0.\n\r",
                                         ch);
                                return;
                        }
                }
                if (!str_cmp(arg2, "site"))
                {
                        pban = first_ban;
                        if (temp[0] == '*')
                                temp++;
                        if (temp[strlen(temp) - 1] == '*')
                                temp[strlen(temp) - 1] = '\0';
                }
                else if (!str_cmp(arg2, "class"))
                        pban = first_ban_class;
                else if (!str_cmp(arg2, "race"))
                        pban = first_ban_race;
                else
                        goto syntax_message;
                for (; pban; pban = pban->next)
                        if (value == 1 || !str_cmp(pban->name, temp))
                                break;
                        else if (value > 1)
                                value--;

                if (!pban)
                {
                        send_to_char("No such ban.\n\r", ch);
                        return;
                }
                ch_printf(ch, "Banned by: %s\n\r", pban->ban_by);
                send_to_char(pban->note, ch);
                return;
        }
        else
                goto syntax_message;
        return;

        /*
         * Catch all syntax message, make sure that return stays above this or you
         * * will get the syntax message everytime you issue the command even if it
         * * is a valid one.  Shaddai
         */

      syntax_message:
        send_to_char("Syntax: ban site  <address> <type> <duration>\n\r", ch);
        send_to_char("Syntax: ban race  <race>    <type> <duration>\n\r", ch);
        send_to_char("Syntax: ban class <class>   <type> <duration>\n\r", ch);
        send_to_char("Syntax: ban show  <field>   <number>\n\r", ch);
        send_to_char("Ban site lists current bans.\n\r", ch);
        send_to_char("Duration is the length of the ban in days.\n\r", ch);
        send_to_char("Type can be:  newbie, mortal, all, warn or level.\n\r",
                     ch);
        send_to_char("In ban show, the <field> is site, race or class,", ch);
        send_to_char("  and the <number> is the ban number.\n\r", ch);
        return;
}


/*
 * Allow a already banned site/class or race.  Shaddai
 */

CMDF do_allow(CHAR_DATA * ch, char *argument)
{
        BAN_DATA *pban;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char     *temp = NULL;
        bool      fMatch = FALSE;
        int       value = 0;

        if (IS_NPC(ch)) /* No mobs allowing sites */
        {
                send_to_char("Monsters are too dumb to do that!\n\r", ch);
                return;
        }

        if (!ch->desc)  /* No desc is a bad thing */
        {
                bug("do_allow: no descriptor", 0);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        set_char_color(AT_IMMORT, ch);

        if (arg1[0] == '\0' || arg2[0] == '\0')
                goto syntax_message;
        if (arg2[0] == '#') /* Use #1 to ban the first ban in the list specified */
        {
                temp = arg2;
                temp++;
                if (!is_number(temp))
                {
                        send_to_char("Which ban # to allow?\n\r", ch);
                        return;
                }
                value = atoi(temp);
        }
        if (!str_cmp(arg1, "site"))
        {
                if (!value)
                {
                        if (strlen(arg2) < 2)
                        {
                                send_to_char
                                        ("You have to have at least 2 chars for a ban\n\r",
                                         ch);
                                send_to_char
                                        ("If you are trying to allow by number use #\n\r",
                                         ch);
                                return;
                        }

                        temp = arg2;
                        if (arg2[0] == '*')
                                temp++;
                        if (temp[strlen(temp) - 1] == '*')
                                temp[strlen(temp) - 1] = '\0';
                }

                for (pban = first_ban; pban; pban = pban->next)
                {
                        /*
                         * Need to make sure we dispose properly of the ban_data 
                         * * Or memory problems will be created.
                         * * Shaddai
                         */

                        if (value == 1 || !str_cmp(pban->name, temp))
                        {
                                fMatch = TRUE;
                                dispose_ban(pban, BAN_SITE);
                                break;
                        }
                        if (value > 1)
                                value--;
                }
        }
        else if (!str_cmp(arg1, "race"))
        {

                arg2[0] = toupper(arg2[0]);
                for (pban = first_ban_race; pban; pban = pban->next)
                {
                        /*
                         * Need to make sure we dispose properly of the ban_data 
                         * * Or memory problems will be created.
                         * * Shaddai
                         */

                        if (value == 1 || !str_cmp(pban->name, arg2))
                        {
                                fMatch = TRUE;
                                dispose_ban(pban, BAN_RACE);
                                break;
                        }
                        if (value > 1)
                                value--;
                }
        }
        else if (!str_cmp(arg1, "class"))
        {

                arg2[0] = toupper(arg2[0]);
                for (pban = first_ban_class; pban; pban = pban->next)
                {
                        /*
                         * Need to make sure we dispose properly of the ban_data 
                         * * Or memory problems will be created.
                         * * Shaddai
                         */

                        if (value == 1 || !str_cmp(pban->name, arg2))
                        {
                                fMatch = TRUE;
                                dispose_ban(pban, BAN_CLASS);
                                break;
                        }
                        if (value > 1)
                                value--;
                }
        }
        else
                goto syntax_message;

        if (fMatch)
        {
                save_banlist();
                ch_printf(ch, "%s is now allowed.\n\r", arg2);
        }
        else
                ch_printf(ch, "%s was not banned.\n\r", arg2);
        return;

        /*
         * Make sure that return above stays in!
         */

      syntax_message:
        send_to_char("Syntax: allow site  <address>\n\r", ch);
        send_to_char("Syntax: allow race  <race>\n\r", ch);
        send_to_char("Syntax: allow class <class>\n\r", ch);
        return;
}

/* 
 *  Sets the warn flag on bans.
 */
CMDF do_warn(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_STRING_LENGTH];
        char      arg2[MAX_STRING_LENGTH];
        char     *name;
        int       count = -1, type;
        BAN_DATA *pban, *start, *end;

        /*
         * Don't want mobs or link-deads doing this.
         */

        if (IS_NPC(ch))
        {
                send_to_char("Monsters are too dumb to do that!\n\r", ch);
                return;
        }

        if (!ch->desc)
        {
                bug("do_warn: no descriptor", 0);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
                goto syntax_message;

        if (arg2[0] == '#')
        {
                name = arg2;
                name++;
                if (!is_number(name))
                        goto syntax_message;
                count = atoi(name);
                if (count < 1)
                {
                        send_to_char("The number has to be above 0.\n\r", ch);
                        return;
                }
        }

        /*
         * We simply set up which ban list we will be looking at here.
         */
        if (!str_cmp(arg1, "class"))
                type = BAN_CLASS;
        else if (!str_cmp(arg1, "race"))
                type = BAN_RACE;
        else if (!str_cmp(arg1, "site"))
                type = BAN_SITE;
        else
                type = -1;

        if (type == BAN_CLASS)
        {
                pban = first_ban_class;
                start = first_ban_class;
                end = last_ban_class;
                arg2[0] = toupper(arg2[0]);
        }
        else if (type == BAN_RACE)
        {
                pban = first_ban_race;
                start = first_ban_race;
                end = last_ban_race;
                arg2[0] = toupper(arg2[0]);
        }
        else if (type == BAN_SITE)
        {
                pban = first_ban;
                start = first_ban;
                end = last_ban;
        }
        else
                goto syntax_message;

        for (; pban && count != 0; count--, pban = pban->next)
                if (count == -1 && !str_cmp(pban->name, arg2))
                        break;
        if (pban)
        {
                /*
                 * If it is just a warn delete it, otherwise remove the warn flag. 
                 */

                if (pban->warn)
                {
                        if (pban->level == BAN_WARN)
                        {
                                dispose_ban(pban, type);
                                send_to_char("Warn has been deleted.\n\r",
                                             ch);
                        }
                        else
                        {
                                pban->warn = FALSE;
                                send_to_char("Warn turned off.\n\r", ch);
                        }
                }
                else
                {
                        pban->warn = TRUE;
                        send_to_char("Warn turned on.\n\r", ch);
                }
                save_banlist();
        }
        else
        {
                ch_printf(ch, "%s was not found in the ban list.\n\r", arg2);
                return;
        }
        return;

        /*
         * The above return has to stay in! 
         */
      syntax_message:
        send_to_char("Syntax: warn class <field>\n\r", ch);
        send_to_char("Syntax: warn race  <field>\n\r", ch);
        send_to_char("Syntax: warn site  <field>\n\r", ch);
        send_to_char
                ("Field is either #(ban_number) or the site/class/race.\n\r",
                 ch);
        send_to_char("Example:  warn class #1\n\r", ch);
        return;
}

/*
 *  This actually puts the new ban into the proper linked list and
 *  initializes its data.  Shaddai
 */

int add_ban(CHAR_DATA * ch, char *arg1, char *arg2, int time, int type)
{
        char      arg[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        BAN_DATA *pban, *temp;
        struct tm *tms;
        char     *name;
        int       level, i, value;

        /*
         * Should we check to see if they have dropped link sometime in between 
         * * writing the note and now?  Not sure but for right now we won't since
         * * do_ban checks for that.  Shaddai
         */

        switch (ch->substate)
        {
        default:
                bug("add_ban: illegal substate", 0);
                return 0;
        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot use this command from within another command.\n\r",
                         ch);
                return 0;
        case SUB_NONE:
                {
                        one_argument(arg1, arg);
                        smash_tilde(arg);   /* Make sure the immortals don't put a ~ in it. */

                        if (arg[0] == '\0' || arg2[0] == '\0')
                                return 0;

                        if (is_number(arg2))
                        {
                                level = atoi(arg2);
                                if (level < 0 || level > LEVEL_SUPREME)
                                {
                                        ch_printf(ch,
                                                  "Level range is from 0 to %d.\n\r",
                                                  LEVEL_SUPREME);
                                        return 0;
                                }
                        }
                        else if (!str_cmp(arg2, "all"))
                                level = LEVEL_SUPREME;
                        else if (!str_cmp(arg2, "newbie"))
                                level = 1;
                        else if (!str_cmp(arg2, "mortal"))
                                level = LEVEL_AVATAR;
                        else if (!str_cmp(arg2, "warn"))
                                level = BAN_WARN;
                        else
                        {
                                bug("Bad string for flag in add_ban.", 0);
                                return 0;
                        }

                        switch (type)
                        {
                        case BAN_CLASS:
                                {
                                        if (arg[0] == '\0')
                                                return 0;
                                        if (is_number(arg))
                                                value = atoi(arg);
                                        else
                                        {
                                                for (i = 0; i < MAX_ABILITY;
                                                     i++)
                                                {
                                                        if (!str_cmp
                                                            (ability_name[i],
                                                             arg))
                                                                break;
                                                }
                                                value = i;
                                        }
                                        if (value < 0 || value >= MAX_ABILITY)
                                        {
                                                send_to_char
                                                        ("Unknown ability.\n\r",
                                                         ch);
                                                return 0;
                                        }
                                        for (temp = first_ban_class; temp;
                                             temp = temp->next)
                                        {
                                                if (temp->flag == value)
                                                {
                                                        if (temp->level ==
                                                            level)
                                                        {
                                                                send_to_char
                                                                        ("That entry already exists.\n\r",
                                                                         ch);
                                                                return 0;
                                                        }
                                                        else
                                                        {
                                                                temp->level =
                                                                        level;
                                                                if (temp->
                                                                    level ==
                                                                    BAN_WARN)
                                                                        temp->warn = TRUE;
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "%24.24s",
                                                                         ctime
                                                                         (&current_time));
                                                                temp->ban_time
                                                                        =
                                                                        str_dup
                                                                        (buf);
                                                                if (temp->
                                                                    ban_by)
                                                                        DISPOSE(temp->ban_by);
                                                                temp->ban_by =
                                                                        str_dup
                                                                        (ch->
                                                                         name);
                                                                send_to_char
                                                                        ("Updated entry.\n\r",
                                                                         ch);
                                                                return 1;
                                                        }
                                                }
                                        }
                                        CREATE(pban, BAN_DATA, 1);
                                        pban->name =
                                                str_dup(ability_name[value]);
                                        pban->flag = value;
                                        pban->level = level;
                                        pban->ban_by = str_dup(ch->name);
                                        LINK(pban, first_ban_class,
                                             last_ban_class, next, prev);
                                        break;
                                }
                        case BAN_RACE:
                                {
                                        RACE_DATA *race = NULL;

                                        FOR_EACH_LIST(RACE_LIST, races, race)
                                                if (!str_cmp
                                                    (race->name(), arg))
                                                break;
                                        if (race == NULL)
                                        {
                                                send_to_char
                                                        ("Unknown race.\n\r",
                                                         ch);
                                                return 0;
                                        }
                                        for (temp = first_ban_race; temp;
                                             temp = temp->next)
                                        {
                                                if (!str_cmp
                                                    (temp->name,
                                                     race->name()))
                                                {
                                                        if (temp->level ==
                                                            level)
                                                        {
                                                                send_to_char
                                                                        ("That entry already exists.\n\r",
                                                                         ch);
                                                                return 0;
                                                        }
                                                        else
                                                        {
                                                                temp->level =
                                                                        level;
                                                                if (temp->
                                                                    level ==
                                                                    BAN_WARN)
                                                                        temp->warn = TRUE;
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "%24.24s",
                                                                         ctime
                                                                         (&current_time));
                                                                temp->ban_time
                                                                        =
                                                                        str_dup
                                                                        (buf);
                                                                if (temp->
                                                                    ban_by)
                                                                        DISPOSE(temp->ban_by);
                                                                temp->ban_by =
                                                                        str_dup
                                                                        (ch->
                                                                         name);
                                                                send_to_char
                                                                        ("Updated entry.\n\r",
                                                                         ch);
                                                                return 1;
                                                        }
                                                }
                                        }
                                        CREATE(pban, BAN_DATA, 1);
                                        pban->name = str_dup(race->name());
                                        pban->flag = 0;
                                        pban->level = level;
                                        pban->ban_by = str_dup(ch->name);
                                        LINK(pban, first_ban_race,
                                             last_ban_race, next, prev);
                                        break;
                                }
                        case BAN_SITE:
                                {
                                        bool      prefix = FALSE, suffix =
                                                FALSE, user_name = FALSE;
                                        char     *temp_host =
                                                NULL, *temp_user = NULL;
                                        unsigned int x;

                                        for (x = 0; x < strlen(arg); x++)
                                        {
                                                if (arg[x] == '@')
                                                {
                                                        user_name = TRUE;
                                                        temp_host =
                                                                str_dup(&arg
                                                                        [x +
                                                                         1]);
                                                        arg[x] = '\0';
                                                        temp_user =
                                                                str_dup(arg);
                                                        break;
                                                }
                                        }
                                        if (!user_name)
                                                name = arg;
                                        else
                                                name = temp_host;
                                        if (name[0] == '*')
                                        {
                                                prefix = TRUE;
                                                name++;
                                        }

                                        if (name[strlen(name) - 1] == '*')
                                        {
                                                suffix = TRUE;
                                                name[strlen(name) - 1] = '\0';
                                        }
                                        for (temp = first_ban; temp;
                                             temp = temp->next)
                                        {
                                                if (!str_cmp
                                                    (temp->name, name))
                                                {
                                                        if (temp->level ==
                                                            level && (prefix
                                                                      &&
                                                                      temp->
                                                                      prefix)
                                                            && (suffix
                                                                && temp->
                                                                suffix)
                                                            && (!user_name
                                                                || (user_name
                                                                    &&
                                                                    !str_cmp
                                                                    (temp->
                                                                     user,
                                                                     temp_user))))
                                                        {
                                                                send_to_char
                                                                        ("That entry already exists.\n\r",
                                                                         ch);
                                                                return 0;
                                                        }
                                                        else
                                                        {
                                                                temp->suffix =
                                                                        suffix;
                                                                temp->prefix =
                                                                        prefix;
                                                                if (temp->
                                                                    level ==
                                                                    BAN_WARN)
                                                                        temp->warn = TRUE;
                                                                temp->level =
                                                                        level;
                                                                snprintf(buf,
                                                                         MSL,
                                                                         "%24.24s",
                                                                         ctime
                                                                         (&current_time));
                                                                temp->ban_time
                                                                        =
                                                                        str_dup
                                                                        (buf);
                                                                if (temp->
                                                                    ban_by)
                                                                        DISPOSE(temp->ban_by);
                                                                if (user_name)
                                                                {
                                                                        DISPOSE(temp_host);
                                                                        DISPOSE(temp_user);
                                                                }
                                                                temp->ban_by =
                                                                        str_dup
                                                                        (ch->
                                                                         name);
                                                                send_to_char
                                                                        ("Updated entry.\n\r",
                                                                         ch);
                                                                return 1;
                                                        }
                                                }
                                        }
                                        CREATE(pban, BAN_DATA, 1);
                                        pban->ban_by = str_dup(ch->name);
                                        pban->suffix = suffix;
                                        pban->prefix = prefix;
                                        pban->name = str_dup(name);
                                        pban->level = level;
                                        if (user_name)
                                        {
                                                pban->user =
                                                        str_dup(temp_user);
                                                DISPOSE(temp_host);
                                                DISPOSE(temp_user);
                                        }
                                        LINK(pban, first_ban, last_ban, next,
                                             prev);
                                        break;
                                }
                        default:
                                {
                                        bug("Bad type in add_ban: %d.", type);
                                        return 0;
                                }
                        }
                        snprintf(buf, MSL, "%24.24s", ctime(&current_time));
                        pban->ban_time = str_dup(buf);
                        if (time > 0)
                        {
                                pban->duration = time;
                                tms = localtime(&current_time);
                                tms->tm_mday += time;
                                pban->unban_date = mktime(tms);
                        }
                        else
                        {
                                pban->duration = -1;
                                pban->unban_date = -1;
                        }
                        if (pban->level == BAN_WARN)
                                pban->warn = TRUE;
                        ch->substate = SUB_BAN_DESC;
                        ch->dest_buf = pban;
                        if (!pban->note)
                                pban->note = STRALLOC("");
                        start_editing(ch, pban->note);
                        return 1;
                }
        case SUB_BAN_DESC:
                {
                        pban = (BAN_DATA *) ch->dest_buf;
                        if (!pban)
                        {
                                bug("do_ban: sub_ban_desc: NULL ch->dest_buf",
                                    0);
                                ch->substate = SUB_NONE;
                                return 0;
                        }
                        if (pban->note)
                                STRFREE(pban->note);
                        pban->note = copy_buffer(ch);
                        stop_editing(ch);
                        ch->substate = ch->tempnum;
                        save_banlist();
                        if (pban->duration > 0)
                        {
                                if (!pban->user)
                                        ch_printf(ch,
                                                  "%s banned for %d days.\n\r",
                                                  pban->name, pban->duration);
                                else
                                        ch_printf(ch,
                                                  "%s@%s banned for %d days.\n\r",
                                                  pban->user, pban->name,
                                                  pban->duration);
                        }
                        else
                        {
                                if (!pban->user)
                                        ch_printf(ch,
                                                  "%s banned forever.\n\r",
                                                  pban->name);
                                else
                                        ch_printf(ch,
                                                  "%s@%s banned forever.\n\r",
                                                  pban->user, pban->name);
                        }
                        return 1;
                }
        }
        return 1;
}

/*
 * Print the bans out to the screen.  Shaddai
 */

void show_bans(CHAR_DATA * ch, int type)
{
        BAN_DATA *pban;
        int       bnum;

        set_pager_color(AT_IMMORT, ch);

        switch (type)
        {
        case BAN_SITE:
                send_to_pager("Banned sites:\n\r", ch);
                send_to_pager
                        ("[ #] Warn (Lv) Time                     By              For   Site\n\r",
                         ch);
                send_to_pager
                        ("---- ---- ---- ------------------------ --------------- ----  ---------------\n\r",
                         ch);
                pban = first_ban;
                set_pager_color(AT_PLAIN, ch);
                for (bnum = 1; pban; pban = pban->next, bnum++)
                {
                        if (!pban->user)
                                pager_printf(ch,
                                             "[%2d] %-4s (%2d) %-24s %-15s %4d  %c%s%c\n\r",
                                             bnum,
                                             (pban->warn) ? "YES" : "no",
                                             pban->level, pban->ban_time,
                                             pban->ban_by, pban->duration,
                                             (pban->prefix) ? '*' : ' ',
                                             pban->name,
                                             (pban->suffix) ? '*' : ' ');
                        else
                                pager_printf(ch,
                                             "[%2d] %-4s (%2d) %-24s %-15s %4d  %s@%c%s%c\n\r",
                                             bnum,
                                             (pban->warn) ? "YES" : "no",
                                             pban->level, pban->ban_time,
                                             pban->ban_by, pban->duration,
                                             pban->user,
                                             (pban->prefix) ? '*' : ' ',
                                             pban->name,
                                             (pban->suffix) ? '*' : ' ');
                }
                return;
        case BAN_RACE:
                send_to_pager("Banned races:\n\r", ch);
                send_to_pager
                        ("[ #] Warn (Lv) Time                     By              For   Race\n\r",
                         ch);
                pban = first_ban_race;
                break;
        case BAN_CLASS:
                send_to_pager("Banned classes:\n\r", ch);
                send_to_pager
                        ("[ #] Warn (Lv) Time                     By              For   Class\n\r",
                         ch);
                pban = first_ban_class;
                break;
        default:
                bug("Bad type in show_bans: %d", type);
                return;
        }
        send_to_pager
                ("---- ---- ---- ------------------------ --------------- ----  ---------------\n\r",
                 ch);
        set_pager_color(AT_PLAIN, ch);
        for (bnum = 1; pban; pban = pban->next, bnum++)
                pager_printf(ch, "[%2d] %-4s (%2d) %-24s %-15s %4d  %s\n\r",
                             bnum, (pban->warn) ? "YES" : "no", pban->level,
                             pban->ban_time, pban->ban_by, pban->duration,
                             pban->name);
        return;
}

/*
 * Check for totally banned sites.  Need this because we don't have a
 * char struct yet.  Shaddai
 */

bool check_total_bans(DESCRIPTOR_DATA * d)
{
        BAN_DATA *pban;
        char      new_host[MAX_STRING_LENGTH];
        int       i;

        for (i = 0; i < (int) strlen(d->host); i++)
                new_host[i] = LOWER(d->host[i]);
        new_host[i] = '\0';

        for (pban = first_ban; pban; pban = pban->next)
        {
                if (pban->level != LEVEL_SUPREME)
                        continue;
                if (pban->prefix && pban->suffix &&
                    strstr(pban->name, new_host))
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_SITE);
                                save_banlist();
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
                /*
                 * Bug of switched checks noticed by Cronel
                 */
                if (pban->suffix && !str_prefix(pban->name, new_host))
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_SITE);
                                save_banlist();
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
                if (pban->prefix && !str_suffix(pban->name, new_host))
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_SITE);
                                save_banlist();
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
                if (!str_cmp(pban->name, new_host))
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_SITE);
                                save_banlist();
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
        }
        return FALSE;
}

/*
 * The workhose, checks for bans on sites/classes and races. Shaddai
 */

bool check_bans(CHAR_DATA * ch, int type)
{
        char      buf[MAX_STRING_LENGTH];
        BAN_DATA *pban;
        char      new_host[MAX_STRING_LENGTH];
        int       i;
        bool      fMatch = FALSE;

        switch (type)
        {
        case BAN_RACE:
                pban = first_ban_race;
                break;
        case BAN_CLASS:
                pban = first_ban_class;
                break;
        case BAN_SITE:
                pban = first_ban;
                for (i = 0; i < (int) (strlen(ch->desc->host)); i++)
                        new_host[i] = LOWER(ch->desc->host[i]);
                new_host[i] = '\0';
                break;
        default:
                bug("Ban type in check_bans: %d.", type);
                return FALSE;
        }
        for (; pban; pban = pban->next)
        {
                if (type == BAN_CLASS && pban->flag == ch->main_ability)
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_CLASS);
                                save_banlist();
                                return FALSE;
                        }
                        if (ch->top_level > pban->level)
                        {
                                if (pban->warn)
                                {
                                        snprintf(buf, MSL,
                                                 "%s class logging in from %s.",
                                                 pban->name, ch->desc->host);
                                        log_string_plus(buf, LOG_HIGH,
                                                        sysdata.log_level);
                                }
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
                if (type == BAN_RACE && ch->race
                    && !str_cmp(pban->name, ch->race->name()))
                {
                        if (check_expire(pban))
                        {
                                dispose_ban(pban, BAN_RACE);
                                save_banlist();
                                return FALSE;
                        }
                        if (ch->top_level > pban->level)
                        {
                                if (pban->warn)
                                {
                                        snprintf(buf, MSL,
                                                 "%s race logging in from %s.",
                                                 pban->name, ch->desc->host);
                                        log_string_plus(buf, LOG_HIGH,
                                                        sysdata.log_level);
                                }
                                return FALSE;
                        }
                        else
                                return TRUE;
                }
                if (type == BAN_SITE)
                {
                        if (pban->prefix && pban->suffix &&
                            strstr(pban->name, new_host))
                                fMatch = TRUE;
                        else if (pban->prefix
                                 && !str_suffix(pban->name, new_host))
                                fMatch = TRUE;
                        else if (pban->suffix
                                 && !str_prefix(pban->name, new_host))
                                fMatch = TRUE;
                        else if (!str_cmp(pban->name, new_host))
                                fMatch = TRUE;
                        if (fMatch)
                        {
                                if (check_expire(pban))
                                {
                                        dispose_ban(pban, BAN_SITE);
                                        save_banlist();
                                        return FALSE;
                                }
                                if (ch->top_level > pban->level)
                                {
                                        if (pban->warn)
                                        {
                                                snprintf(buf, MSL,
                                                         "%s logging in from site %s.",
                                                         ch->name,
                                                         ch->desc->host);
                                                log_string_plus(buf, LOG_HIGH,
                                                                sysdata.
                                                                log_level);
                                        }
                                        return FALSE;
                                }
                                else
                                        return TRUE;
                        }
                }
        }
        return FALSE;
}

bool check_expire(BAN_DATA * pban)
{
        char      buf[MAX_STRING_LENGTH];

        if (pban->unban_date < 0)
                return FALSE;
        if (pban->unban_date <= current_time)
        {
                snprintf(buf, MSL, "%s ban has expired.", pban->name);
                log_string_plus(buf, LOG_HIGH, sysdata.log_level);
                return TRUE;
        }
        return FALSE;
}

void dispose_ban(BAN_DATA * pban, int type)
{
        if (!pban)
                return;

        if (type != BAN_SITE && type != BAN_CLASS && type != BAN_RACE)
        {
                bug("Dispose_ban: Unknown Ban Type %d.", type);
                return;
        }

        switch (type)
        {
        case BAN_SITE:
                UNLINK(pban, first_ban, last_ban, next, prev);
                break;
        case BAN_CLASS:
                UNLINK(pban, first_ban_class, last_ban_class, next, prev);
                break;
        case BAN_RACE:
                UNLINK(pban, first_ban_race, last_ban_race, next, prev);
                break;
        }
        free_ban(pban);
        return;
}

void free_ban(BAN_DATA * pban)
{
        if (pban->name)
                DISPOSE(pban->name);
        if (pban->ban_time)
                DISPOSE(pban->ban_time);
        if (pban->note)
                STRFREE(pban->note);
        if (pban->user)
                DISPOSE(pban->user);
        if (pban->ban_by)
                DISPOSE(pban->ban_by);
        if (pban->ban_time)
                DISPOSE(pban->ban_time);
        DISPOSE(pban);
}

/* Reserve stuff */

RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;

void save_reserved(void)
{
        RESERVE_DATA *res;
        FILE     *fp;

        FCLOSE(fpReserve);
        if (!(fp = fopen(SYSTEM_DIR RESERVED_LIST, "w")))
        {
                bug("Save_reserved: cannot open " RESERVED_LIST, 0);
                perror(RESERVED_LIST);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }
        for (res = first_reserved; res; res = res->next)
                fprintf(fp, "%s~\n", res->name);
        fprintf(fp, "$~\n");
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_reserve(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        RESERVE_DATA *res;

        set_char_color(AT_PLAIN, ch);

        argument = one_argument(argument, arg);
        if (!*arg)
        {
                int       wid = 0;

                send_to_char("-- Reserved Names --\n\r", ch);
                for (res = first_reserved; res; res = res->next)
                {
                        ch_printf(ch, "%c%-17s ",
                                  (*res->name == '*' ? '*' : ' '),
                                  (*res->name ==
                                   '*' ? res->name + 1 : res->name));
                        if (++wid % 4 == 0)
                                send_to_char("\n\r", ch);
                }
                if (wid % 4 != 0)
                        send_to_char("\n\r", ch);
                return;
        }
        for (res = first_reserved; res; res = res->next)
                if (!str_cmp(arg, res->name))
                {
                        UNLINK(res, first_reserved, last_reserved, next,
                               prev);
                        DISPOSE(res->name);
                        DISPOSE(res);
                        save_reserved();
                        send_to_char("Name no longer reserved.\n\r", ch);
                        return;
                }
        CREATE(res, RESERVE_DATA, 1);
        res->name = str_dup(arg);
        sort_reserved(res);
        save_reserved();
        send_to_char("Name reserved.\n\r", ch);
        return;
}

bool is_reserved_name(char *name)
{
        RESERVE_DATA *res;

        for (res = first_reserved; res; res = res->next)
                if ((*res->name == '*' && !str_infix(res->name + 1, name)) ||
                    !str_cmp(res->name, name))
                        return TRUE;
        return FALSE;
}

void load_reserved(void)
{
        RESERVE_DATA *res;
        FILE     *fp;

        if (!(fp = fopen(SYSTEM_DIR RESERVED_LIST, "r")))
                return;

        for (;;)
        {
                if (feof(fp))
                {
                        bug("Load_reserved: no $ found.");
                        FCLOSE(fp);
                        return;
                }
                CREATE(res, RESERVE_DATA, 1);
                res->name = fread_string_nohash(fp);
                if (*res->name == '$')
                        break;
                sort_reserved(res);
        }
        DISPOSE(res->name);
        DISPOSE(res);
        FCLOSE(fp);
        return;
}

/* Rebuilt from broken copy, but bugged - commented out for now - Blod */
void sort_reserved(RESERVE_DATA * pRes)
{
        RESERVE_DATA *res = NULL;

        if (!pRes)
        {
                bug("Sort_reserved: NULL pRes");
                return;
        }

        pRes->next = NULL;
        pRes->prev = NULL;

        for (res = first_reserved; res; res = res->next)
        {
                if (strcasecmp(pRes->name, res->name) > 0)
                {
                        INSERT(pRes, res, first_reserved, next, prev);
                        break;
                }
        }

        if (!res)
        {
                LINK(pRes, first_reserved, last_reserved, next, prev);
        }

        return;
}
