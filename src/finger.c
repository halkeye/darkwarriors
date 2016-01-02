/* vim: ts=8 et ft=c sw=8
 *****************************************************************************************
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
 * Finger Modules                                                                        *
 *****************************************************************************************
 *                $Id: finger.c 1345 2005-12-11 03:17:37Z greven $                *
 *****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include "mud.h"
#include "races.h"
#include "account.h"

typedef struct finger_data
{
        char     *name;
        char     *race;
        char     *title;
        int       age;
        int       top_level;
        int       main_ability;
        int       sex;
        /*
         * Finger Data 
         */
        char     *icq;
        char     *msn;
        char     *realname;
        char     *homepage;
        char     *email;
        char     *aolim;
        char     *yahoo;
        char     *logon;
} FINGER_DATA;

#define QUICK_VALID_STRING(string) ((string) != NULL && (string)[0] != '\0')
#define FILE_VERSION 2

void output_finger(CHAR_DATA * ch, FINGER_DATA * finger)
{
        send_to_char
                ("&B------------------&w<&B<&C<&w<&B<&C< &wCharacter Info&B &w>&B>&C>&w>&B>&C>&B------------------\n\r",
                 ch);
        ch_printf(ch, "&BN&zame &w: %-20s &w\n\r",
                  finger->name);
		if (IS_IMMORTAL(ch))
		{
        ch_printf(ch, "&BL&zevel&w: %-20d &w  &BC&zlass&w: %-10s\n\r",
                  finger->top_level,
                  finger->main_ability ==
                  0 ? "Combatant" : finger->main_ability ==
                  1 ? "Pilot" : finger->main_ability ==
                  2 ? "Engineer" : finger->main_ability ==
                  3 ? "Bounty Hunter" : finger->main_ability ==
                  4 ? "Smuggler" : finger->main_ability ==
                  5 ? "Diplomat" : finger->main_ability ==
                  6 ? "Leader" : finger->main_ability ==
                  7 ? "Force User" : finger->main_ability ==
                  8 ? "Jack-of-all-Trades" : finger->main_ability ==
                  9 ? "Pirate" : finger->main_ability ==
                  10 ? "Medic" : "Unknown");
        ch_printf(ch, "&BS&zex&w  : %-20s &w  &BR&zace&w : %s\n\r",
                  finger->sex == SEX_MALE ? "Male" : finger->sex ==
                  SEX_FEMALE ? "Female" : "Neutral",
                  capitalize(finger->race));
        ch_printf(ch, "&BT&zitle&w: %s\n\r", finger->title);
		}
        send_to_char
                ("&B------------------&w<&B<&C<&w<&B<&C< &wReal Life Info&B &w>&B>&C>&w>&B>&C>&B------------------\n\r",
                 ch);
        ch_printf(ch, "&BR&zealname&w: &B[&w%-20s&B]\n\r",
                  QUICK_VALID_STRING(finger->realname) ? finger->
                  realname : "Not specified");
        ch_printf(ch, "&BH&zomepage&w: &B[&w%-20s&B]\n\r",
                  QUICK_VALID_STRING(finger->homepage) ? finger->
                  homepage : "Not specified");
        ch_printf(ch, "&BE&zmail   &w: &B[&w%-20s&B]\n\r",
                  QUICK_VALID_STRING(finger->email) ? finger->
                  email : "Not specified");
        ch_printf(ch,
                  "&BA&zIM    &w : &B[&w%-20s&B]&w &w &BI&zCQ&w: &B[&w%-20s&B]&w\n\r",
                  QUICK_VALID_STRING(finger->aolim) ? finger->
                  aolim : "Not specified",
                  QUICK_VALID_STRING(finger->icq) ? finger->
                  icq : "None specified");
        ch_printf(ch,
                  "&BY&zahoo IM&w: &B[&w%-20s&B]&w &w &BM&zSN&w: &B[&w%-20s&B]&w\n\r",
                  QUICK_VALID_STRING(finger->yahoo) ? finger->
                  yahoo : "Not specified",
                  QUICK_VALID_STRING(finger->msn) ? finger->
                  msn : "None specified");
        send_to_char
                ("&B----------------------------------------------------------------\n\r",
                 ch);
        ch_printf(ch, "&BL&zast on&w : %s\n\r", finger->logon);
        return;
}

CMDF do_finger(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        FINGER_DATA *finger;

        if (IS_NPC(ch))
        {
                send_to_char("Mobs can't use the finger command.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Finger whom?\n\r", ch);
                return;
        }

        victim = get_char_world(ch, argument);

        if ((victim == NULL) || (!victim))
        {
                read_finger(ch, argument);
                return;
        }

        if (IS_NPC(victim))
        {
                read_finger(ch, argument);
                return;
        }

        CREATE(finger, FINGER_DATA, 1);
        finger->name = victim->name;
        finger->race = victim->race->name();
        finger->title = victim->pcdata->full_name;
        finger->age = IS_IMMORTAL(victim) ? 0 : get_age(victim);
        finger->top_level = victim->top_level;
        finger->main_ability = victim->main_ability;
        finger->sex = victim->sex;
        finger->realname = victim->pcdata->realname;
        finger->homepage = victim->pcdata->homepage;
        finger->email = victim->pcdata->email;
        finger->icq = victim->pcdata->icq;
        finger->msn = victim->pcdata->msn;
        finger->aolim = victim->pcdata->aolim;
        finger->yahoo = victim->pcdata->yahoo;
        finger->logon = (char *) ctime(&victim->logon);
        output_finger(ch, finger);
        DISPOSE(finger);
        return;
}

void read_finger(CHAR_DATA * ch, char *argument)
{
        FILE     *fpFinger;
        char      fingload[MAX_INPUT_LENGTH];
        char      laston[250];
        struct stat fst;

        snprintf(fingload, MSL, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),
                 capitalize(argument));
        if (!file_exist(fingload))
        {
                send_to_char("&YNo such player exists.\n\r", ch);
                return;
        }


        snprintf(fingload, MSL, "%s%c/%s.F", PLAYER_DIR, tolower(argument[0]),
                 capitalize(argument));

        if (stat(fingload, &fst) != -1)
        {
                mudstrlcpy(laston, ctime(&fst.st_mtime), 250);
                if ((fpFinger = fopen(fingload, "r")) != NULL)
                {
                        for (;;)
                        {
                                char      letter;
                                char     *word;

                                letter = fread_letter(fpFinger);
                                if (letter == '*')
                                {
                                        fread_to_eol(fpFinger);
                                        continue;
                                }

                                if (letter != '#')
                                {
                                        bug("finger.c: read_finger: # not found in finger file.", 0);
                                        bug(fingload, 0);
                                        ch_printf(ch,
                                                  "&YThe finger file for %s is unreadable.\n\r",
                                                  capitalize(argument));
                                        break;
                                }

                                word = fread_word(fpFinger);
                                if (!strcmp(word, "FINGER"))
                                        fread_finger(ch, fpFinger, laston);
                                else if (!strcmp(word, "END"))  /* Done     */
                                        break;
                                else
                                {
                                        bug("finger.c: read_finger: bad section.", 0);
                                        bug(fingload, 0);
                                        ch_printf(ch,
                                                  "&YThe finger file for %s is unreadable.\n\r",
                                                  capitalize(argument));
                                        break;
                                }
                        }
                        FCLOSE(fpFinger);
                }
        }
        else
                ch_printf(ch, "&G%s does not have a finger file to read.\n\r",
                          capitalize(argument));
        return;
}

sh_int    get_age_timeinfo(TIME_INFO_DATA * birthday);
void fread_finger(CHAR_DATA * ch, FILE * fp, char *laston)
{
        FINGER_DATA *finger;
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        int       version = 1;
        bool      fMatch;
        int       age = 0;
        TIME_INFO_DATA birthday;

        CREATE(finger, FINGER_DATA, 1);
        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;
                if (!strcmp(word, "End"))
                        break;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Aim", finger->aolim, fread_string(fp));
                        KEY("Age", age, fread_number(fp));
                        break;
                case 'B':
                        if (!strcmp(word, "BDay"))  /* Voltecs player birthdays! */
                        {
                                birthday.hour = fread_number(fp);
                                birthday.day = fread_number(fp);
                                birthday.month = fread_number(fp);
                                birthday.year = fread_number(fp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'M':
                        KEY("Msn", finger->msn, fread_string(fp));
                        break;

                case 'Y':
                        KEY("Yahoo", finger->yahoo, fread_string(fp));
                        break;

                case 'I':
                        KEY("Icq", finger->icq, fread_string(fp));
                        break;

                case 'C':
                        KEY("Class", finger->main_ability, fread_number(fp));
                        break;

                case 'E':
                        KEY("Email", finger->email, fread_string(fp));
                        break;

                case 'H':
                        KEY("Homepage", finger->homepage, fread_string(fp));
                        break;

                case 'L':
                        KEY("Level", finger->top_level, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", finger->name, fread_string(fp));
                        break;

                case 'R':
                        if (!str_cmp(word, "race"))
                        {
                                fMatch = TRUE;
                                if (version == 2)
                                {
                                        char     *temp =
                                                fread_string_nohash(fp);
                                        finger->race = get_race(temp)->name();
                                        DISPOSE(temp);
                                }
                                else
                                        finger->race =
                                                get_race_number(fread_number
                                                                (fp))->name();
                        }
                        KEY("Realname", finger->realname,
                            fread_string_nohash(fp));
                        break;

                case 'S':
                        KEY("Sex", finger->sex, fread_number(fp));
                        break;

                case 'T':
                        KEY("Title", finger->title, fread_string(fp));
                        break;

                case 'V':
                        KEY("Version", version, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL,
                                 "finger.c: fread_finger: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }

        finger->age = 0;
        if (age != 0)
                finger->age = age + get_age_timeinfo(&birthday);

        finger->logon = laston;
        output_finger(ch, finger);

        if (finger->email)
                STRFREE(finger->email);
        if (finger->homepage)
                STRFREE(finger->homepage);
        if (finger->name)
                STRFREE(finger->name);
        if (finger->title)
                STRFREE(finger->title);
        if (finger->aolim)
                STRFREE(finger->aolim);
        if (finger->realname)
                STRFREE(finger->realname);
        if (finger->icq)
                STRFREE(finger->icq);
        if (finger->msn)
                STRFREE(finger->msn);
        if (finger->yahoo)
                STRFREE(finger->yahoo);
        DISPOSE(finger);
        return;
}


void save_finger(CHAR_DATA * ch)
{
        FILE     *fp;
        char      fng[MAX_INPUT_LENGTH];

        if (IS_NPC(ch) || NOT_AUTHED(ch))
                return;

        if (ch->desc && ch->desc->original)
                ch = ch->desc->original;

        snprintf(fng, MSL, "%s%c/%s.F", PLAYER_DIR, tolower(ch->name[0]),
                 capitalize(ch->name));

        FCLOSE(fpReserve);
        if ((fp = fopen(fng, "w")) == NULL)
        {
                bug("finger.c: save_finger: fopen", 0);
                perror("save_finger");
        }
        else
        {
                fprintf(fp, "#FINGER\n");
                fprintf(fp, "Version\t\t%d\n", FILE_VERSION);
                if (ch->name[0] != '\0')
                        fprintf(fp, "Name\t\t%s~\n", ch->name);
                fprintf(fp, "Level\t\t%d\n", ch->top_level);
                fprintf(fp, "Sex\t\t%d\n", ch->sex);
                fprintf(fp, "Class\t\t%d\n", ch->main_ability);
                fprintf(fp, "Age          %d\n", ch->pcdata->age);
                fprintf(fp, "BDay %d %d %d %d\n", ch->pcdata->birthday.hour,
                        ch->pcdata->birthday.day, ch->pcdata->birthday.month,
                        ch->pcdata->birthday.year);
                if (ch->race && ch->race->name()[0] != '\0')
                        fprintf(fp, "Race\t\t%s~\n", ch->race->name());
                if (QUICK_VALID_STRING(ch->pcdata->full_name))
                        fprintf(fp, "Title\t\t%s~\n", ch->pcdata->full_name);
                if (QUICK_VALID_STRING(ch->pcdata->homepage))
                        fprintf(fp, "Homepage\t\t%s~\n",
                                ch->pcdata->homepage);
                if (QUICK_VALID_STRING(ch->pcdata->email))
                        fprintf(fp, "Email\t\t%s~\n", ch->pcdata->email);
                if (QUICK_VALID_STRING(ch->pcdata->aolim))
                        fprintf(fp, "Aim\t\t%s~\n", ch->pcdata->aolim);
                if (QUICK_VALID_STRING(ch->pcdata->icq))
                        fprintf(fp, "Icq\t\t%s~\n", ch->pcdata->icq);
                if (QUICK_VALID_STRING(ch->pcdata->msn))
                        fprintf(fp, "Msn\t\t%s~\n", ch->pcdata->msn);
                if (QUICK_VALID_STRING(ch->pcdata->yahoo))
                        fprintf(fp, "Yahoo\t\t%s~\n", ch->pcdata->yahoo);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_setself(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("&zS&wyntax&Y: &zsetself  \n\r", ch);
                send_to_char("&zF&wields&Y: &zemail realname icq\n\r", ch);
                send_to_char("        &zrealemail homepage aim msn yahoo &w\n\r", ch);
                return;
        }

		if (IS_NPC(ch)) return;

        if (!str_cmp(arg, "email"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself email <your email address>\n\r",
                                 ch);
                        return;
                }
                hide_tilde(argument);
                if (ch->pcdata->email)
                        STRFREE(ch->pcdata->email);
                ch->pcdata->email = STRALLOC(argument);
                send_to_char("Email address set.\n\r", ch);
                return;
        }
        
		if (!str_cmp(arg, "realemail"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself realemail <your email address>\n\r",
                                 ch);
                        return;
                }
                hide_tilde(argument);
                if (ch->pcdata->account->email)
                        STRFREE(ch->pcdata->account->email);
                ch->pcdata->account->email = STRALLOC(argument);
                send_to_char("Email address set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "realname"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';

                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself realname <your real name>\n\r",
                                 ch);
                        return;
                }

                hide_tilde(argument);
                if (ch->pcdata->realname)
                        STRFREE(ch->pcdata->realname);
                ch->pcdata->realname = STRALLOC(argument);
                send_to_char("Realname set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "icq"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';

                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself icq <your icq number>\n\r",
                                 ch);
                        return;
                }

                hide_tilde(argument);
                if (ch->pcdata->icq)
                        STRFREE(ch->pcdata->icq);
                ch->pcdata->icq = STRALLOC(argument);
                send_to_char("ICQ set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "homepage"))
        {
                if (!str_cmp("none", argument))
                {
                        if (ch->pcdata->homepage)
                                STRFREE(ch->pcdata->homepage);
                        send_to_char("Unset.\n\r", ch);
                        return;
                }

                if (strstr(argument, "://"))
                        mudstrlcpy(buf, argument, MSL);
                else
                        snprintf(buf, MSL, "http://%s", argument);

                if (strlen(buf) > 70)
                        buf[70] = '\0';
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself homepage <your website address>\n\r",
                                 ch);
                        return;
                }

                hide_tilde(buf);
                if (ch->pcdata->homepage)
                        STRFREE(ch->pcdata->homepage);
                ch->pcdata->homepage = STRALLOC(buf);
                send_to_char("Homepage set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "aim"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';

                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself aim <your aim name>\n\r",
                                 ch);
                        return;
                }
                hide_tilde(argument);
                if (ch->pcdata->aolim)
                        STRFREE(ch->pcdata->aolim);
                ch->pcdata->aolim = STRALLOC(argument);
                send_to_char("Aim set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "legal"))
        {

                if (argument[0] == '\0')
                {
                        send_to_char("&zS&wyntax&Y: legal <yes/no>\n\r", ch);
                        send_to_char
                                ("*Note: If you choose to set yourself as legal,\n\r you swear/affirm that as of this moment,\n\r you are an adult, at least 18 \n\ror in some countries/regions 21 years old,\n\r as well as consenting to view pornographic content.\n\r",
                                 ch);
                        return;
                }
                if (!str_cmp(argument, "yes") || !str_cmp(argument, "Yes"))
                {
                        ch->pcdata->realage = 0;
                }
                if (!str_cmp(argument, "No") || !str_cmp(argument, "no"))
                {
                        ch->pcdata->realage = 1;
                }
                send_to_char("Legal status set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "msn"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself msn <your msn account>\n\r",
                                 ch);
                        return;
                }

                hide_tilde(argument);
                if (ch->pcdata->msn)
                        STRFREE(ch->pcdata->msn);
                ch->pcdata->msn = STRALLOC(argument);
                send_to_char("MSN set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "yahoo"))
        {
                if (strlen(argument) > 70)
                        buf[70] = '\0';
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("&zS&wyntax&Y: &zsetself yahoo <your yahoo name>\n\r",
                                 ch);
                        return;
                }

                hide_tilde(argument);
                if (ch->pcdata->yahoo)
                        STRFREE(ch->pcdata->yahoo);
                ch->pcdata->yahoo = STRALLOC(argument);
                send_to_char("Yahoo set.\n\r", ch);
                return;
        }


        do_setself(ch, "");
        save_finger(ch);
        return;
}
