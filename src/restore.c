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
 *                $Id: restore.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "restore.h"
#include "editor.h"

RESTORE_DATA *first_restore;
RESTORE_DATA *last_restore;

RESTORE_DATA *get_restore(char *name)
{
        RESTORE_DATA *restore;

        for (restore = first_restore; restore; restore = restore->next)
                if (!str_cmp(name, restore->type))
                        return restore;

        for (restore = first_restore; restore; restore = restore->next)
                if (nifty_is_name(name, restore->type))
                        return restore;

        for (restore = first_restore; restore; restore = restore->next)
                if (!str_prefix(name, restore->type))
                        return restore;

        for (restore = first_restore; restore; restore = restore->next)
                if (nifty_is_name_prefix(name, restore->type))
                        return restore;
        return NULL;
}

/* Online editing of restore added by Gavin 5-26-2000 */
/* Read in an individual restoretype */
void fread_restore(RESTORE_DATA * restore, FILE * fp)
{
        char      buf[MAX_STRING_LENGTH];
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
                        KEY("Boost", restore->boost, fread_float(fp));
                        break;

                case 'C':
                        KEY("Cmessage", restore->cmsg, fread_string(fp));
                        KEY("Color", restore->color, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!restore->owner)
                                        restore->owner = STRALLOC("Any");
                                if (!restore->rmsg)
                                        restore->rmsg = STRALLOC("");
                                if (!restore->vmsg)
                                        restore->vmsg = STRALLOC("");
                                if (!restore->cmsg)
                                        restore->cmsg = STRALLOC("");
                                if (!restore->boost)
                                        restore->boost = 1;
                                return;
                        }
                        break;

                case 'F':
                        KEY("Flags", restore->flags, fread_number(fp));
                        break;

                case 'O':
                        KEY("Owner", restore->owner, fread_string(fp));
                        break;

                case 'R':
                        KEY("Rmessage", restore->rmsg, fread_string(fp));
                        break;

                case 'T':
                        KEY("Type", restore->type, fread_string(fp));
                        break;

                case 'V':
                        KEY("Vmessage", restore->vmsg, fread_string(fp));
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_restore: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

/* Load the restore file */
void load_restores()
{
        char      filename[256];
        RESTORE_DATA *restore;
        FILE     *fp;
        int       restorecount;

        first_restore = NULL;
        last_restore = NULL;

        snprintf(filename, 256, "%s%s", SYSTEM_DIR, RESTORE_FILE);

        if ((fp = fopen(filename, "r")) != NULL)
        {
                restorecount = 0;
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
                                bug("Load_restore_file: # not found.", 0);
                                break;
                        }
                        word = fread_word(fp);
                        if (!str_cmp(word, "RESTORE"))
                        {
                                if (restorecount >= MAX_RESTORE_TYPES)
                                {
                                        bug("load_restores: more restoretypes than MAX_RESTORE_TYPES %d", MAX_RESTORE_TYPES);
                                        FCLOSE(fp);
                                        return;
                                }
                                CREATE(restore, RESTORE_DATA, 1);
                                fread_restore(restore, fp);
                                restorecount++;
                                LINK(restore, first_restore, last_restore,
                                     next, prev);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_restore_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                continue;
                        }
                }
                FCLOSE(fp);
        }
        return;
}

/* Online restore editing, save the restore table to disk - Gavin 5-26-2000 */
void save_restores(void)
{
        RESTORE_DATA *trestore;
        FILE     *fp;
        char      filename[256];

        snprintf(filename, 256, "%s%s", SYSTEM_DIR, RESTORE_FILE);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_restore: fopen", 0);
                perror(filename);
        }
        else
        {
                for (trestore = first_restore; trestore;
                     trestore = trestore->next)
                {
                        fprintf(fp, "#RESTORE\n");
                        fprintf(fp, "Type         %s~\n", trestore->type);
                        fprintf(fp, "Owner        %s~\n", trestore->owner);
                        fprintf(fp, "Cmessage     %s~\n", trestore->cmsg);
                        fprintf(fp, "Vmessage     %s~\n", trestore->vmsg);
                        fprintf(fp, "Rmessage     %s~\n", trestore->rmsg);
                        fprintf(fp, "Color        %d\n", trestore->color);
                        fprintf(fp, "Boost        %f\n", trestore->boost);
                        fprintf(fp, "Flags        %d\n", trestore->flags);
                        fprintf(fp, "End\n\n");
                }
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

/** Function: do_restore
  * Descr   : Restores (heals) a player, optionally sending one of several
  *           predefined "restore option" messages to those involved.
  * Returns : (void)
  * Syntax  : restore (who/all) [option]
  * Written : v1.1 March/2004
  * Author  : Gavin <haleye@halkeye.net>
  */

CMDF do_restore(CHAR_DATA * ch, char *argument)
{
        RESTORE_DATA *restore;
        char      type[MAX_INPUT_LENGTH];
        char      who[MAX_INPUT_LENGTH];
        int       color = AT_IMMORT;
        bool      found = FALSE;
        CHAR_DATA *victim;

        if (IS_NPC(ch))
        {
                send_to_char("Mobs can't use the restore command.\n\r", ch);
                return;
        }

        set_char_color(AT_IMMORT, ch);

        argument = one_argument(argument, who);
        argument = one_argument(argument, type);

        if (!str_prefix(who, "list") || who == NULL)
        {
                set_char_color(AT_GREEN, ch);
                send_to_char("Syntax: restore <victim/all> [type]\n\r", ch);
                send_to_char("Where type is one of the above...\n\r", ch);

                send_to_pager_color
                        ("&YRestore                    &ROwner\n\r", ch);
                send_to_pager_color
                        ("&g-------------------------+---------------\n\r",
                         ch);
                for (restore = first_restore; restore;
                     restore = restore->next)
                        pager_printf(ch, "&G%-14s      &g%13s\n\r",
                                     restore->type, restore->owner);

                send_to_char
                        ("\n\rTyping just 'restore <player>' will work too...\n\r",
                         ch);
                return;
        }

        if (!str_cmp(who, "all"))
        {
                CHAR_DATA *vch;
                CHAR_DATA *vch_next;

                if (!ch->pcdata)
                        return;

                if (get_trust(ch) < LEVEL_NEOPHYTE)
                {
                        if (IS_NPC(ch))
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return;
                        }
                        else
                        {
                                /*
                                 * Check if the player did a restore all within the last 18 hours. 
                                 */
                                if (current_time - last_restore_all_time <
                                    RESTORE_INTERVAL)
                                {
                                        send_to_char
                                                ("Sorry, you can't do a restore all yet.\n\r",
                                                 ch);
                                        do_restoretime(ch, "");
                                        return;
                                }
                        }
                }
                last_restore_all_time = current_time;
                ch->pcdata->restore_time = current_time;
                save_char_obj(ch);
                send_to_char("Beginning 'restore all' ...\n\r", ch);
                for (vch = first_char; vch; vch = vch_next)
                {
                        vch_next = vch->next;

                        if (!IS_NPC(vch) && !IS_IMMORTAL(vch))
                        {
                                if (type[0] == '\0')
                                {
                                        act(AT_IMMORT, "$n has restored you.",
                                            ch, NULL, vch, TO_VICT);
                                        vch->hit = vch->max_hit;
                                        vch->endurance = vch->max_endurance;
                                        update_pos(vch);
                                        found = TRUE;
                                }
                                else
                                {
                                        for (restore = first_restore; restore;
                                             restore = restore->next)
                                        {
                                                if ((!str_cmp
                                                     (type, restore->type)
                                                     && !str_cmp("Any",
                                                                 restore->
                                                                 owner))
                                                    ||
                                                    (!str_cmp
                                                     (restore->owner,
                                                      ch->name)
                                                     && !str_cmp(type,
                                                                 restore->
                                                                 type))
                                                    || (get_trust(ch) ==
                                                        MAX_LEVEL
                                                        && !str_cmp(type,
                                                                    restore->
                                                                    type)))
                                                {
                                                        found = TRUE;
                                                        color = restore->
                                                                color;
                                                        act(AT_WHITE,
                                                            restore->vmsg, ch,
                                                            NULL, vch,
                                                            TO_VICT);
                                                        if (restore->boost !=
                                                            0)
                                                                vch->hit =
                                                                        (sh_int)
                                                                        (vch->
                                                                         max_hit
                                                                         *
                                                                         restore->
                                                                         boost);
                                                        else
                                                                vch->hit =
                                                                        vch->
                                                                        max_hit;
                                                        vch->endurance =
                                                                vch->
                                                                max_endurance;
                                                        update_pos(vch);
                                                        /*
                                                         * return;
                                                         */
                                                }
                                        }
                                }
                                if (!found)
                                        send_to_char
                                                ("&RRestore type not defined, or not owned by you. Type \"restore list\" for a complete listing of types available to you.\n\r",
                                                 ch);
                        }
                }
                send_to_char("Restored.\n\r", ch);
        }
        else
        {

                if ((victim = get_char_world(ch, who)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (type[0] == '\0')
                {
                        act(AT_IMMORT, "You restore $N!", ch, NULL, victim,
                            TO_CHAR);
                        act(AT_IMMORT, "$n has restored you.", ch, NULL,
                            victim, TO_VICT);
                        act(AT_IMMORT, "$n restores $N!", ch, NULL, victim,
                            TO_NOTVICT);
                        victim->hit = victim->max_hit;
                        victim->endurance = victim->max_endurance;
                        update_pos(victim);
                        return;
                }
                else
                {
                        for (restore = first_restore; restore;
                             restore = restore->next)
                        {
                                if ((!str_cmp(type, restore->type)
                                     && !str_cmp("Any", restore->owner))
                                    || (!str_cmp(restore->owner, ch->name)
                                        && !str_cmp(type, restore->type))
                                    || (get_trust(ch) == MAX_LEVEL
                                        && !str_cmp(type, restore->type)))
                                {
                                        found = TRUE;
                                        color = restore->color;
                                        act(AT_WHITE, restore->cmsg, ch, NULL,
                                            victim, TO_CHAR);
                                        act(AT_WHITE, restore->vmsg, ch, NULL,
                                            victim, TO_VICT);
                                        act(AT_WHITE, restore->rmsg, ch, NULL,
                                            victim, TO_NOTVICT);
                                        if (restore->boost != 0)
                                                victim->hit =
                                                        (sh_int) (victim->
                                                                  max_hit *
                                                                  restore->
                                                                  boost);
                                        else
                                                victim->hit = victim->max_hit;
                                        victim->endurance =
                                                victim->max_endurance;
                                        update_pos(victim);
                                        return;
                                }
                        }
                }
                if (!found)
                        send_to_char
                                ("&RRestore type not defined, or not owned by you. Type \"restore list\" for a complete listing of types available to you.\n\r",
                                 ch);
                send_to_char("Restored.\n\r", ch);
                return;
        }
}

/* Create a restoretype online - Gavin 5-26-2000 */
CMDF do_makerestore(CHAR_DATA * ch, char *argument)
{
        RESTORE_DATA *restore;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makerestore <restoretype>\n\r", ch);
                return;
        }

        if ((restore = get_restore(argument)) != NULL)
        {
                send_to_char("That restore type already exists.\n\r", ch);
                return;
        }

        CREATE(restore, RESTORE_DATA, 1);
        LINK(restore, first_restore, last_restore, next, prev);
        restore->type = STRALLOC(argument);
        restore->owner = STRALLOC("Any");
        restore->color = AT_IMMORT;
        restore->boost = 1;
        restore->cmsg = STRALLOC("You restore $N!");
        restore->vmsg = STRALLOC("$n has restored you");
        restore->rmsg = STRALLOC("$n restores $N!");
        ch_printf(ch, "New restoretype %s added. Set to default values.\n\r",
                  restore->type);
        return;
}

/* Set restore values online - Gavin 5-26-2000 */
CMDF do_setrestore(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        RESTORE_DATA *restore;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                break;

        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot do this while in another command.\n\r",
                         ch);
                return;

        case SUB_RESTORECMSG:
                restore = (RESTORE_DATA *) ch->dest_buf;
                STRFREE(restore->cmsg);
                restore->cmsg = copy_buffer(ch);
                stop_editing(ch);
                ch->substate = ch->tempnum;
                return;

        case SUB_RESTOREVMSG:
                restore = (RESTORE_DATA *) ch->dest_buf;
                STRFREE(restore->vmsg);
                restore->vmsg = copy_buffer(ch);
                stop_editing(ch);
                ch->substate = ch->tempnum;
                return;

        case SUB_RESTORERMSG:
                restore = (RESTORE_DATA *) ch->dest_buf;
                STRFREE(restore->rmsg);
                restore->rmsg = copy_buffer(ch);
                stop_editing(ch);
                ch->substate = ch->tempnum;
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Usage: setrestore <restoretype> <field> <value>\n\r",
                         ch);
                send_to_char("Usage: setrestore save\n\r", ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char("owner cmsg vmsg rmsg flags\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "save"))
        {
                save_restores();
                send_to_char("Restore table saved.\n\r", ch);
                return;
        }

        if ((restore = get_restore(arg1)) == NULL)
        {
                send_to_char("No such restoretype.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "owner"))
        {
                STRFREE(restore->owner);
                restore->owner = STRALLOC(argument);
                send_to_char("New owner set.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg2, "name") || !str_cmp(arg2, "type"))
        {
                STRFREE(restore->type);
                restore->type = STRALLOC(argument);
                send_to_char("Set.", ch);
                return;
        }
        else if (!str_cmp(arg2, "flags"))
        {
                /*
                 * hunger/thirst/ 
                 */
                ;
        }
        if (!str_cmp(arg2, "cmsg"))
        {
                if (ch->substate == SUB_REPEATCMD)
                        ch->tempnum = SUB_REPEATCMD;
                else
                        ch->tempnum = SUB_NONE;
                ch->substate = SUB_RESTORECMSG;
                ch->dest_buf = restore;
                start_editing(ch, restore->cmsg);
#ifdef SOLANCODE
                set_editor_desc(ch, "A custom restore message.");
#endif
                return;
        }

        if (!str_cmp(arg2, "vmsg"))
        {
                if (ch->substate == SUB_REPEATCMD)
                        ch->tempnum = SUB_REPEATCMD;
                else
                        ch->tempnum = SUB_NONE;
                ch->substate = SUB_RESTOREVMSG;
                ch->dest_buf = restore;
                start_editing(ch, restore->vmsg);
#ifdef SOLANCODE
                set_editor_desc(ch, "A custom restore message.");
#endif
                return;
        }

        if (!str_cmp(arg2, "rmsg"))
        {
                if (ch->substate == SUB_REPEATCMD)
                        ch->tempnum = SUB_REPEATCMD;
                else
                        ch->tempnum = SUB_NONE;
                ch->substate = SUB_RESTORERMSG;
                ch->dest_buf = restore;
                start_editing(ch, restore->rmsg);
#ifdef SOLANCODE
                set_editor_desc(ch, "A custom restore message.");
#endif
                return;
        }

        if (!str_cmp(arg2, "color"))
        {
                if (atoi(argument) < 0)
                {
                        send_to_char
                                ("Please Enter A Number greater then 0\n\r",
                                 ch);
                        return;
                }
                restore->color = atoi(argument);
                send_to_char("Restore color set.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "boost"))
        {
                if (get_trust(ch) <= (MAX_LEVEL - 2))
                {
                        send_to_char
                                ("Your not able to set this, ask a higher imm\n\r",
                                 ch);
                        return;
                }
                if (atoi(argument) < 0)
                {
                        send_to_char
                                ("Please Enter A Number greater then 0\n\r",
                                 ch);
                        return;
                }
                restore->boost = atof(argument);
                send_to_char("Restore boost set.\n\r", ch);
                return;
        }

        do_setrestore(ch, "");
        return;
}

/* Online restore editor, show details of a restoretype - Gavin 5-26-2000 */
CMDF do_showrestore(CHAR_DATA * ch, char *argument)
{
        RESTORE_DATA *restore;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                set_char_color(AT_GREEN, ch);
                send_to_char("Syntax: restore <victim/all> [type]\n\r", ch);
                send_to_char("Where type is one of the above...\n\r", ch);

                send_to_pager_color
                        ("&YRestore                    &ROwner\n\r", ch);
                send_to_pager_color
                        ("&g-------------------------+---------------\n\r",
                         ch);
                for (restore = first_restore; restore;
                     restore = restore->next)
                        pager_printf(ch, "&G%-14s      &g%13s\n\r",
                                     restore->type, restore->owner);

                return;
        }

        if ((restore = get_restore(argument)) == NULL)
        {
                send_to_char("No such restoretype.\n\r", ch);
                return;
        }

        ch_printf(ch, "\n\rRestoretype: %s\n\r", restore->type);
        ch_printf(ch, "Owner:\t%s\n\r", restore->owner);
        ch_printf(ch, "Color:\t%d\n\r", restore->color);
        ch_printf(ch, "Boost:\t%f\n\r", restore->boost);
        ch_printf(ch, "&RCmessage: \n\r%s\n\r", restore->cmsg);
        ch_printf(ch, "&YVmessage: \n\r%s\n\r", restore->vmsg);
        ch_printf(ch, "&GRmessage: \n\r%s\n\r", restore->rmsg);

        return;
}

/* Of course, to create means you need to be able to destroy as well :P - Gavin 5-26-2000 */
CMDF do_destroyrestore(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        RESTORE_DATA *prestore;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Destroy which restoretype?\n\r", ch);
                return;
        }

        for (prestore = first_restore; prestore; prestore = prestore->next)
        {
                if (!str_cmp(arg, prestore->type))
                {
                        UNLINK(prestore, first_restore, last_restore, next,
                               prev);
                        STRFREE(prestore->owner);
                        STRFREE(prestore->type);
                        STRFREE(prestore->cmsg);
                        STRFREE(prestore->vmsg);
                        STRFREE(prestore->rmsg);
                        DISPOSE(prestore);
                        ch_printf(ch,
                                  "Restoretype \"%s\" has beed deleted. Be sure to save the restore table.\n\r",
                                  arg);
                        return;
                }
        }

        send_to_char("No such restoretype.\n\r", ch);
        return;
}

void free_restore(RESTORE_DATA * restore)
{
        STRFREE(restore->owner);
        STRFREE(restore->type);
        STRFREE(restore->cmsg);
        STRFREE(restore->vmsg);
        STRFREE(restore->rmsg);
        DISPOSE(restore);
}
