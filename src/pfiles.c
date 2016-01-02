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
 *                                 Pfile Pruning Module                                  *
 ****************************************************************************************/

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#include "account.h"

/* Globals */
time_t    pfile_time;
HOUR_MIN_SEC set_pfile_time_struct;
HOUR_MIN_SEC *set_pfile_time;
struct tm *new_pfile_time;
struct tm new_pfile_struct;
time_t    new_pfile_time_t;
sh_int    num_pfiles;   /* Count up number of pfiles */
char     *smash_extension args((char *filename));
bool notify_deletion args((char *email));
void      send_email
args((char *subject, char *email, char *message, CHAR_DATA * ch));


void save_timedata(void)
{
        FILE     *fp;
        char      filename[MIL];

        snprintf(filename, MSL, "%stime.dat", SYSTEM_DIR);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_timedata: fopen");
                perror(filename);
        }
        else
        {
                fprintf(fp, "%s", "#TIME\n");
                fprintf(fp, "Purgetime %ld\n", new_pfile_time_t);
                fprintf(fp, "%s", "End\n\n");
                fprintf(fp, "%s", "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

/* Reads the actual time file from disk - Samson 1-21-99 */
void fread_timedata(FILE * fp)
{
        const char *word = NULL;
        bool      fMatch = FALSE;

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

                case 'E':
                        if (!str_cmp(word, "End"))
                                return;
                        break;

                case 'P':
                        KEY("Purgetime", new_pfile_time_t, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_timedata: no match: %s", word);
                        fread_to_eol(fp);
                }
        }
}

bool load_timedata(void)
{
        char      filename[MIL];
        FILE     *fp;
        bool      found;

        found = FALSE;
        snprintf(filename, MSL, "%stime.dat", SYSTEM_DIR);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                for (;;)
                {
                        char      letter = '\0';
                        char     *word = NULL;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("%s", "Load_timedata: # not found.");
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "TIME"))
                        {
                                fread_timedata(fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_timedata: bad section - %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        return found;
}

void init_pfile_scan_time(void)
{
        /*
         * Init pfile scan time.
         */
        set_pfile_time = &set_pfile_time_struct;

        new_pfile_time = update_time(localtime(&current_time));
        /*
         * Copies *new_pfile_time to new_pfile_struct, and then points
         * new_pfile_time to new_pfile_struct again. -- Alty 
         */
        new_pfile_struct = *new_pfile_time;
        new_pfile_time = &new_pfile_struct;
        new_pfile_time->tm_mday += 1;
        if (new_pfile_time->tm_hour > 12)
                new_pfile_time->tm_mday += 1;
        new_pfile_time->tm_sec = 0;
        new_pfile_time->tm_min = 0;
        new_pfile_time->tm_hour = 3;

        /*
         * Update new_pfile_time (due to day increment) 
         */
        new_pfile_time = update_time(new_pfile_time);
        new_pfile_struct = *new_pfile_time;
        new_pfile_time = &new_pfile_struct;
        /*
         * Bug fix submitted by Gabe Yoder 
         */
        new_pfile_time_t = mktime(new_pfile_time);
        /*
         * check_pfiles(mktime(new_pfile_time)); 
         */

        if (!load_timedata())
                log_string("Pfile scan time reset to default time of 3am.");
        return;
}

time_t    now_time;
sh_int    deleted = 0;
sh_int    days = 0;

/****
 *
 * FIXME -- Should use account->email instead of each char thing
 * thta way we only need to set an email once.
 *
 * Should load up the pfile properly
 * then when we save, set a flag / extra line in pfile that says they've been warned
 * then on normal save, it is not written (or in flag case, removed on login)
 */
void fread_pfile(FILE * fp, time_t tdiff, char *fname, bool count)
{
        ACCOUNT_DATA *account = NULL;
        char     *word;
        char     *name = NULL;
        char     *clan = NULL;
        char     *email = NULL;
        sh_int    level = 0;
        sh_int    file_ver = 0;
        int       pact = 0;
        bool      fMatch;
        char      buf[MSL];
        char      accountfile[255];

        for (;;)
        {
                word = feof(fp) ? '\0' : fread_word(fp);
                /*
                 * word   = feof( fp ) ? "End" : fread_word( fp ); -- Lets make this faster 
                 */
                fMatch = FALSE;

                if (word[0] == '\0' || !strcmp(word, "End"))
                        break;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Act", pact, fread_number(fp));
#ifdef ACCOUNT
                        KEY("Account", account,
                            load_account(fread_string_noalloc(fp)));
#endif
                        break;

                case 'C':
                        KEY("Clan", clan, fread_string(fp));
                        break;

                case 'E':
                        KEY("Email", email, fread_string(fp));
                        break;

                case 'N':
                        KEY("Name", name, fread_string(fp));
                        break;

                case 'T':
                        KEY("Toplevel", level, fread_number(fp));
                        break;
                case 'V':
                        KEY("Version", file_ver, fread_number(fp));
                        break;
                }

                if (!fMatch)
                        fread_to_eol(fp);
        }

        /*
         * Lets add all names to clan roster 
         */
        if (clan)
        {
                CLAN_DATA *guild;

                if ((guild = get_clan(clan)) != NULL)
                {
                        if (guild->roster)
                        {
                                if (!hasname(guild->roster, name))
                                        addname(&guild->roster, name);
                        }
                        else
                                guild->roster = STRALLOC(name);
                }

        }

        if (count == FALSE && !IS_SET(pact, PCFLAG_EXEMPT))
        {
                if ((level < 10 && tdiff > sysdata.newbie_purge)
                    || (level < LEVEL_IMMORTAL
                        && tdiff > sysdata.regular_purge))
                {
                        if (unlink(fname) == -1)
                                bug("pfiles: File %s not found.", fname);
                        else
                        {
                                snprintf(buf, MSL, "%s%c/%s.F", PLAYER_DIR,
                                         tolower(name[0]), capitalize(name));
                                if (access(buf, F_OK) == 0)
                                        unlink(buf);
                                snprintf(buf, MSL, "%s%c/%s.home", PLAYER_DIR,
                                         tolower(name[0]), capitalize(name));
                                if (access(buf, F_OK) == 0)
                                        unlink(buf);
                                snprintf(buf, MSL, "%s%c/%s.clone",
                                         PLAYER_DIR, tolower(name[0]),
                                         capitalize(name));
                                if (access(buf, F_OK) == 0)
                                        unlink(buf);
                                if (level < 10)
                                        days = sysdata.newbie_purge;
                                else
                                        days = sysdata.regular_purge;
                                snprintf(log_buf, MSL,
                                         "Player %s was deleted. Exceeded time limit of %d days.",
                                         name, days);
                                log_string(log_buf);
                                if (email)
                                {
                                        char      subject[MIL];
                                        char      message[MSL];

                                        snprintf(subject, MIL,
                                                 "Automated Pfile Email for %s at Dark Warriors Mud",
                                                 name ? name : "character");
                                        snprintf(message, MSL,
                                                 "This is an automated message from Dark Warriors mud notifying you about the deletion\n"
                                                 "of your pfile due it inactivity. If you wish your pfile restored in full, please contact\n"
                                                 "the administration of Dark Warriors by logging on at telnet://darkwars.wolfpaw.net:4848\n"
                                                 "or by emailing them at this email adress.\n\n"
                                                 "Thank you for your time and for playing Dark Warriors.\n");
                                        send_email(subject, email, message,
                                                   NULL);
                                        log_string
                                                ("Email Notification was send");
                                }
#ifdef AUTO_AUTH
                                remove_from_auth(name);
#endif
#ifdef ACCOUNT
                                /*
                                 * SUCH A HACK 
                                 */
                                if (account)
                                {
                                        CHAR_DATA *ch;

                                        CREATE(ch, CHAR_DATA, 1);
                                        ch->name = name;
                                        del_from_account(account, ch);
                                        DISPOSE(ch);
                                        /*
                                         * To remove empty accounts, check the very first slot
                                         */
                                        if (account->character[0] == NULL)
                                        {
                                                snprintf(accountfile, 255,
                                                         "%s%c/%s.account",
                                                         ACCOUNT_DIR,
                                                         tolower(account->
                                                                 name[0]),
                                                         capitalize(account->
                                                                    name));
                                                if (access(accountfile, F_OK)
                                                    == 0)
                                                        unlink(accountfile);
                                        }
                                        /*
                                         * free_account(account);
                                         * account = NULL; 
                                         */
                                }
#endif
                                if (clan != NULL)
                                {
                                        CLAN_DATA *guild = get_clan(clan);

                                        if (guild != NULL)
                                        {
                                                if (guild->roster)
                                                        if (hasname
                                                            (guild->roster,
                                                             name))
                                                                removename
                                                                        (&guild->
                                                                         roster,
                                                                         name);
                                                if (!str_cmp
                                                    (name, guild->leader))
                                                {
                                                        if (guild->leader)
                                                                STRFREE(guild->leader);
                                                        guild->leader =
                                                                STRALLOC("");
                                                }
                                                if (!str_cmp
                                                    (name, guild->number1))
                                                {
                                                        if (guild->number1)
                                                                STRFREE(guild->number1);
                                                        guild->number1 =
                                                                STRALLOC("");
                                                }
                                                if (!str_cmp
                                                    (name, guild->number2))
                                                {
                                                        if (guild->number2)
                                                                STRFREE(guild->number2);
                                                        guild->number2 =
                                                                STRALLOC("");
                                                }
                                                save_clan(guild);
                                        }
                                }
                                deleted++;
                                if (name)
                                        STRFREE(name);
                                if (clan)
                                        STRFREE(clan);
                                if (email)
                                        STRFREE(email);
#if ACCOUNT
                                if (account)
                                        free_account(account);
#endif
                                return;
                        }
                }
        }
#if ACCOUNT
        if (account)
                free_account(account);
#endif
        if (clan != NULL)
        {
                CLAN_DATA *guild = get_clan(clan);

                if (guild != NULL)
                {
                        guild->members++;
                        save_clan(guild);
                }
        }

        if (name)
                STRFREE(name);
        if (clan)
                STRFREE(clan);
        if (email)
                STRFREE(email);

        return;
}

void read_pfile(char *dirname, char *filename, bool count)
{
        FILE     *fp;
        char      fname[MSL];
        struct stat fst;
        time_t    tdiff;

        now_time = time(0);

        snprintf(fname, MSL, "%s/%s", dirname, filename);

        if (stat(fname, &fst) != -1)
        {
                tdiff = (now_time - fst.st_mtime) / 86400;

                if ((fp = fopen(fname, "r")) != NULL)
                {
                        for (;;)
                        {
                                char      letter;
                                const char *word;

                                letter = fread_letter(fp);

                                if ((letter != '#') && (!feof(fp)))
                                        continue;

                                word = feof(fp) ? "End" : fread_word(fp);

                                if (!str_cmp(word, "End"))
                                        break;

                                if (!str_cmp(word, "PLAYER"))
                                        fread_pfile(fp, tdiff, fname, count);
                                else if (!str_cmp(word, "END")) /* Done     */
                                        break;
                        }
                        FCLOSE(fp);
                }
        }
        return;
}

void pfile_scan(bool count)
{
        DIR      *dp;
        struct dirent *dentry;
        CLAN_DATA *clan;
        char      directory_name[100];
        char      player_name[MIL];
        char      buf[MSL];
        char      file_name[MIL];

        sh_int    alpha_loop;
        sh_int    cou = 0;

        deleted = 0;

        now_time = time(0);
        nice(20);

        /*
         * Reset all clans to 0 members prior to scan - Samson 7-26-00 
         */
        if (!count)
                for (clan = first_clan; clan; clan = clan->next)
                        clan->members = 0;


        for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
        {
                snprintf(directory_name, MSL, "%s%c", PLAYER_DIR,
                         'a' + alpha_loop);
                /*
                 * log_string( directory_name ); 
                 */
                dp = opendir(directory_name);
                dentry = readdir(dp);
                while (dentry)
                {
                        /*
                         * Added by Tarl 3 Dec 02 because we are now using CVS 
                         */
                        if (!str_cmp(dentry->d_name, "CVS"))
                        {
                                dentry = readdir(dp);
                                continue;
                        }
                        if (dentry->d_name[0] != '.')
                        {
                                int       length = strlen(dentry->d_name);

                                if (!count)
                                {

                                        if (!strstr(dentry->d_name, "."))
                                                read_pfile(directory_name,
                                                           dentry->d_name,
                                                           count);
                                        if (!str_suffix
                                            (".clone", dentry->d_name)
                                            || !str_suffix(".F",
                                                           dentry->d_name)
                                            || !str_suffix(".home",
                                                           dentry->d_name))
                                        {
                                                snprintf(file_name, MSL,
                                                         "%s/%s",
                                                         directory_name,
                                                         dentry->d_name);
                                                snprintf(player_name, MSL,
                                                         "%s/%s",
                                                         directory_name,
                                                         smash_extension
                                                         (dentry->d_name));
                                                if (access(player_name, F_OK)
                                                    != 0)
                                                {
                                                        snprintf(buf, MSL,
                                                                 "Pfile \"%s\" does not exist for corresponding \"%s\", unlinking.",
                                                                 player_name,
                                                                 file_name);
                                                        unlink(file_name);
                                                        log_string(buf);
                                                }
                                        }
                                }
                                if (dentry->
                                    d_name[length - 2] != '.'
                                    && dentry->d_name[length -
                                                      5] != '.'
                                    && dentry->d_name[length - 6] != '.')
                                        cou++;
                        }
                        dentry = readdir(dp);
                }
                closedir(dp);
        }

        if (!count)
                log_string("Pfile cleanup completed.");
        else
                log_string("Pfile count completed.");

        snprintf(log_buf, MSL, "Total pfiles scanned: %d", cou);
        log_string(log_buf);

        if (!count)
        {
                snprintf(log_buf, MSL, "Total pfiles deleted: %d", deleted);
                log_string(log_buf);

                snprintf(log_buf, MSL, "Total pfiles remaining: %d",
                         cou - deleted);
                num_pfiles = cou - deleted;
                log_string(log_buf);
        }
        else
                num_pfiles = cou;

        /*
         * If a clan is 0 members, make sure to clear leaders and stuff - Gavinn 01-04-04 
         */
        for (clan = first_clan; clan; clan = clan->next)
        {
                if (clan->members == 0)
                {
                        if (clan->leader)
                                STRFREE(clan->leader);
                        clan->leader = STRALLOC("");
                        if (clan->number1)
                                STRFREE(clan->number1);
                        clan->number1 = STRALLOC("");
                        if (clan->number2)
                                STRFREE(clan->number2);
                        clan->number2 = STRALLOC("");
                }
        }
        return;
}

CMDF do_pfiles(CHAR_DATA * ch, char *argument)
{
        char      buf[MSL];

        if (IS_NPC(ch))
        {
                send_to_char("Mobs cannot use this command!\n\r", ch);
                return;
        }

        if (argument[0] == '\0' || !argument)
        {
                /*
                 * Makes a backup copy of existing pfiles just in case - Samson 
                 */
                snprintf(buf, MSL, "tar -czf %spfiles.tgz %s*", PLAYER_DIR,
                         PLAYER_DIR);

                /*
                 * GAH, the shell pipe won't process the command that gets pieced
                 * together in the preceeding lines! God only knows why. - Samson 
                 */
                system(buf);

                snprintf(log_buf, MSL, "Manual pfile cleanup started by %s.",
                         ch->name);
                log_string(log_buf);
                pfile_scan(FALSE);
#ifdef SAMSONRENT
                rent_update();
#endif
                return;
        }

        if (!str_cmp(argument, "settime"))
        {
                new_pfile_time_t = current_time + 86400;
                save_timedata();
                send_to_char("New cleanup time set for 24 hrs from now.\n\r",
                             ch);
                return;
        }

        if (!str_cmp(argument, "count"))
        {
                snprintf(log_buf, MSL, "Pfile count started by %s.",
                         ch->name);
                log_string(log_buf);
                pfile_scan(TRUE);
                return;
        }

        send_to_char("Invalid argument.\n\r", ch);
        return;
}

void check_pfiles(time_t reset)
{
        /*
         * This only counts them up on reboot if the cleanup isn't needed - Samson 1-2-00 
         */
        if (reset == 255 && new_pfile_time_t > current_time)
        {
                reset = 0;  /* Call me paranoid, but it might be meaningful later on */
                log_string("Counting pfiles.....");
                pfile_scan(TRUE);
                return;
        }

        if (new_pfile_time_t <= current_time)
        {
                if (sysdata.CLEANPFILES == TRUE)
                {

                        char      buf[MSL];

                        /*
                         * Makes a backup copy of existing pfiles just in case - Samson 
                         */
                        snprintf(buf, MSL, "tar -cf %spfiles.tar %s*",
                                 PLAYER_DIR, PLAYER_DIR);

                        /*
                         * Would use the shell pipe for this, but alas, it requires a ch in order
                         * to work, this also gets called during boot_db before the rare item
                         * checks for the rent code - Samson 
                         */
                        system(buf);

                        new_pfile_time_t = current_time + 86400;
                        save_timedata();
                        log_string("Automated pfile cleanup beginning....");
                        pfile_scan(FALSE);
#ifdef SAMSONRENT
                        if (reset == 0)
                                rent_update();
#endif
                }
                else
                {
                        new_pfile_time_t = current_time + 86400;
                        save_timedata();
                        log_string("Counting pfiles.....");
                        pfile_scan(TRUE);
#ifdef SAMSONRENT
                        if (reset == 0)
                                rent_update();
#endif
                }
        }
        return;
}

char     *smash_extension(char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '.')
                {
                        *retptr = '\0';
                        break;
                }
                else
                {
                        *retptr = *str;
                        retptr++;
                }
        }
        *retptr = '\0';
        return ret;
}

bool notify_deletion(char *email)
{
        static char sendstring[1000];
        FILE     *fp;
        FILE     *mfp;

        mudstrlcpy(sendstring, "", 1000);

        fp = fopen(EMAIL_FILE, "w");
        fprintf(fp,
                "This is an automated message from Dark Warriors mud notifying you about the deletion\n");
        fprintf(fp,
                "of your pfile due it inactivity. If you wish your pfile restored in full, please contact\n");
        fprintf(fp,
                "the administration of Dark Warriors by logging on at telnet://darkwars.wolfpaw.net:4848\n");
        fprintf(fp, "or by emailing them at this email adress.\n\n");
        fprintf(fp,
                "Thank you for your time and for playing Dark Warriors.\n");
        fprintf(fp, "\n\n---\n");
        fprintf(fp, "%s\n", "Automated Email");
        fprintf(fp, "Dark Warriors Email System \n");
        fprintf(fp, "telnet://darkwars.wolfpaw.net:4848 \n");
        fprintf(fp, "http://darkwars.wolfpaw.net \n");
        FCLOSE(fp);

        snprintf(sendstring, MSL, "%s -s \"%s: %s\" \"%s\" < %s",
                 sysdata.mail_path, "Automated Email",
                 "Dark Warriors Player File", email, EMAIL_FILE);
        log_string(sendstring);
        if ((mfp = popen(sendstring, "w")) == NULL)
        {
                bug("send_email:  Could not location mail.");
                return FALSE;
        }
        pclose(mfp);
        remove(EMAIL_FILE);
        return TRUE;
}
