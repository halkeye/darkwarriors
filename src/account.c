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
 * This module allows users to attach different characters to a single account and share *
 * simple things such as aliases, role playing points, etc                               *
 *****************************************************************************************
 *                $Id: account.c 1330 2005-12-05 03:23:24Z halkeye $                *
 *****************************************************************************************/

#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mud.h"
#include "account.h"
#include "alias.h"
#include "boards.h"

ACCOUNT_DATA *first_account = NULL;
ACCOUNT_DATA *last_account = NULL;


sh_int check_playing args((DESCRIPTOR_DATA * d, char *name, bool kick));
bool check_reconnect args((DESCRIPTOR_DATA * d, char *name, bool fConn));
void      fwrite_comments(ACCOUNT_DATA * account, FILE * fp);
void      fread_comment(ACCOUNT_DATA * account, FILE * fp);

/*
 * Local functions.
 */
void fread_account args((ACCOUNT_DATA * account, FILE * fp));
ACCOUNT_DATA *get_account args((char *name));

ACCOUNT_DATA *create_account()
{
        ACCOUNT_DATA *account = NULL;

        CREATE(account, ACCOUNT_DATA, 1);
        account->rppoints = 0;
        account->rpcurrent = -1;
        account->qpoints = 0;
        account->name = NULL;
        account->password = NULL;
        account->inuse = 1;
		account->comments = NULL;

        return account;
}

void free_account(ACCOUNT_DATA * account)
{
        int       count;
		NOTE_DATA *pnote, *next_note;

        if (!account)
                return;
#ifdef DEBUG
        bug("Free'ing %s=%d", account->name, account->inuse - 1);
#endif
        /*
         * Check to see uses
         */
        if (--account->inuse > 0)
        {
                /*
                 * Still in use, not ready to delete yet 
                 */
                return;
        }
        else if (account->inuse < 0)
        {
                bug("Freeing err'd data");
                abort();
        }

        if (account->name)
                STRFREE(account->name);
        if (account->password)
                STRFREE(account->password);
        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (account->character[count] == NULL)
                        continue;
                if (account->character[count])
                        STRFREE(account->character[count]);
        }
		for(pnote = account->comments; pnote; pnote = next_note)
		{
			next_note = pnote->next;
			free_note(pnote);
		}
        free_aliases(account);
        UNLINK(account, first_account, last_account, next, prev);
        DISPOSE(account);
        account = NULL;
}

CMDF do_showalts(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        ACCOUNT_DATA *account = NULL;
        int       count;

        if (argument[0] == '\0')
        {
                send_to_char("&Bs&zhowalts <&wchar&B|&waccount&z>\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, argument)) != NULL)
        {
                if (!victim->pcdata || !victim->pcdata->account)
                {
                        send_to_char
                                ("&BN&zo &Raccount&z associated with that character.",
                                 ch);
                        return;
                }
                account = victim->pcdata->account;
        }
        else
        {
                if ((account = get_account(argument)) == NULL)
                {
                        send_to_char("That account is not around.", ch);
                        return;
                }
        }

        if (account == NULL)
        {
                send_to_char("That player is not around.", ch);
                return;
        }

        if (account == NULL)
                return;

		ch_printf(ch, "Account Name: %s\n\r", account->name);
		ch_printf(ch, "RP Points:    %d\n\r", account->rppoints);
		ch_printf(ch, "RP Current:   %d\n\r", account->rpcurrent);
		ch_printf(ch, "Quest Points: %d\n\r", account->qpoints);
		send_to_char("----------------------------------------------\n\r",ch);

        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (account->character[count] == NULL)
                        continue;
                ch_printf(ch, "&w%-2d&B] &z%-66s&z\n\r", count,
                          capitalize(account->character[count]));
        }

        if (count == 0)
        {
                send_to_char
                        ("&BT&zhat account has no &Rcharacters&z linked to it.",
                         ch);
                return;
        }
}

void show_account_characters(DESCRIPTOR_DATA * d)
{
        int       count;
        char      buf[MSL];

        if (!d->account)
        {
                bug("show_account_character: no account!", 0);
                return;
        }
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (d->account->character[count] == NULL)
                        break;
                snprintf(buf, MSL, "&z| &B[&w%-2d&B] &z%-65s&z|\n\r", count,
                         capitalize(d->account->character[count]));
                send_to_desc_color(buf, d);
        }
        send_to_desc_color
                ("&z|                                                                       |\n\r",
                 d);
        /*
         * Bah to buffering first - Gavin 
         */
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "New");
        send_to_desc_color(buf, d);
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "Link");
        send_to_desc_color(buf, d);
        snprintf(buf, MSL, "&z| &B[&wXX&B] &z%-65s&z|\n\r", "Password");
        send_to_desc_color(buf, d);
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
}

void save_account(ACCOUNT_DATA * account)
{
        char      accountsave[MIL];
        FILE     *fp;

        if (!account)
        {
                bug("Save_account: null account!", 0);
                return;
        }

        snprintf(accountsave, 255, "%s%c/%s.account", ACCOUNT_DIR,
                 tolower(account->name[0]), capitalize(account->name));

        if ((fp = fopen(accountsave, "w")) == NULL)
        {
                bug("save_account: fopen", 0);
                perror(accountsave);
        }
        else
        {
                int       count;

                fprintf(fp, "#%s\n", "ACCOUNT");
                fprintf(fp, "Name      %s~\n", account->name);
                fprintf(fp, "Password  %s~\n", account->password);
                fprintf(fp, "Email     %s~\n", account->email);
                fprintf(fp, "RPpoints  %d\n", account->rppoints);
                fprintf(fp, "RPcurrent %d\n", account->rpcurrent);
                fprintf(fp, "Qpoints   %d\n", account->qpoints);
                for (count = 0; count < MAX_CHARACTERS; count++)
                {
                        if (account->character[count] == NULL)
                                continue;

                        fprintf(fp, "Character %s~\n",
                                account->character[count]);
                }
                fprintf(fp, "End\n\n");

                fwrite_alias(account, fp);
                if (account->comments)  /* comments */
                        fwrite_comments(account, fp);   /* comments */
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        return;
}

ACCOUNT_DATA *get_account(char *name)
{
        ACCOUNT_DATA *account = NULL;

        for (account = first_account; account; account = account->next)
        {
                if (!account->name)
                {
                        bug("Account with invalid Name", 0);
                        free_account(account);
                }
                else if (!str_cmp(account->name, name))
                {
#ifdef DEBUG
                        bug("Returned %s=%d", account->name, account->inuse);
#endif
                        return account;
                }
        }
        return NULL;
}

ACCOUNT_DATA *load_account(char *name)
{
        ACCOUNT_DATA *account;
        char      accountsave[MIL];
        bool      found;
        FILE     *fp;

        found = FALSE;

        account = get_account(name);
        if (account)
        {
                account->inuse++;
                return account;
        }

        snprintf(accountsave, 255, "%s%c/%s.account", ACCOUNT_DIR,
                 tolower(name[0]), capitalize(name));

        if ((fp = fopen(accountsave, "r")) != NULL)
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
                                bug("Load_clan_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "ACCOUNT"))
                        {
                                /*
                                 * Create a create_account that sets defaults 
                                 */
                                account = create_account();
                                fread_account(account, fp);
                                continue;
                        }
                        else if (!str_cmp(word, "COMMENT"))
                        {
                                account->comments = NULL;   /* comments */
                                fread_comment(account, fp); /* Comments */
                        }
                        else if (!str_cmp(word, "ALIAS"))   /* Aliases */
                        {
                                fread_alias(account, fp);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_account_file: bad section: %s.",
                                    word);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
                return NULL;
        }
        if (account != NULL)
        {
#ifdef DEBUG
                bug("Returned %s=%d", account->name, account->inuse);
#endif
                LINK(account, first_account, last_account, next, prev);
        }
        else
        {
                bug("Account is null");
        }
        return account;
}

void fread_account(ACCOUNT_DATA * account, FILE * fp)
{
        char      buf[MSL];
        const char *word;
        bool      fMatch;
        int       count = 0;

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
                        if (!str_cmp(word, "Character"))
                        {
                                char     *name = fread_string(fp);

                                snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR,
                                         tolower(name[0]), capitalize(name));
                                if (access(buf, F_OK) != 0)
                                {
                                        /*
                                         * If their player file is NOT there, do not add it to the account 
                                         */
                                        STRFREE(name);
                                        fMatch = TRUE;
                                        break;
                                }
                                account->character[count] = name;
                                count++;
                                fMatch = TRUE;
                                break;
                        }
                        break;
                case 'E':
                        KEY("Email", account->email, fread_string(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (account->rpcurrent == -1)
                                        account->rpcurrent =
                                                account->rppoints;
                                return;
                        }
                        break;
                case 'N':
                        KEY("Name", account->name, fread_string(fp));
                        break;
                case 'P':
                        KEY("Password", account->password, fread_string(fp));
                        break;
                case 'Q':
                        KEY("Qpoints", account->qpoints, fread_number(fp));
                        break;
                case 'R':
                        KEY("RPpoints", account->rppoints, fread_number(fp));
                        KEY("RPcurrent", account->rpcurrent,
                            fread_number(fp));
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "load_account: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

bool add_to_account(ACCOUNT_DATA * account, CHAR_DATA * ch)
{
        int       count;

        if (!account || !ch)
        {
                bug("add_to_account: null ch or account!", 0);
                return FALSE;
        }
        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (account->character[count] == NULL)
                        continue;
                /*
                 * Prevent Duplicates anywhere in the list, but already in the list, so it was successful 
                 */
                if (!str_cmp(account->character[count], ch->name))
                        return TRUE;
        }
        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (account->character[count] == NULL)
                        break;
        }
        if (count >= MAX_CHARACTERS)
                return FALSE;

        account->character[count] = STRALLOC(ch->name);
        if (ch->pcdata && ch->pcdata->rp)
        {
                account->rppoints += ch->pcdata->rp;
                ch->pcdata->rp = 0;
        }
/*      save_char_obj(ch); */

        return TRUE;
}

bool del_from_account(ACCOUNT_DATA * account, CHAR_DATA * ch)
{
        int       count;

        if (!account || !ch)
        {
                bug("del_from_account: null ch or account!", 0);
                return FALSE;
        }

        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (account->character[count] == NULL)
                        continue;
                if (!str_cmp(account->character[count], ch->name))
                {
                        STRFREE(account->character[count]);
                        for (; count < MAX_CHARACTERS - 1; count++)
                                account->character[count] =
                                        account->character[count + 1];

                        break;
                }

        }

        return TRUE;
}

/*
 * Transfers one player from one account to another
 * Currently both accounts and players need to be online
 * But soon to be fixed
 *
 * Actually, currently does nothing :D
 */
CMDF do_transaccount(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = get_char_world(ch, argument);

        if (victim == NULL)
        {
                send_to_char("No such player online", ch);
                return;
        }

        return;
}

CMDF do_showaccounts(CHAR_DATA * ch, char *argument)
{
        ACCOUNT_DATA *account = NULL;
        CHAR_DATA *victim = get_char_world(ch, argument);

        if (victim && !IS_NPC(victim))
        {
                account = victim->pcdata->account;
                ch_printf(ch, "Account Name: %s\n\r", account->name);
                ch_printf(ch, "RP Points:    %d\n\r", account->rppoints);
                ch_printf(ch, "RP Current:   %d\n\r", account->rpcurrent);
                ch_printf(ch, "Quest Points: %d\n\r", account->qpoints);
                return;
        }
        else
        {
                send_to_pager("All Chracters online:\n\r", ch);
                for (account = first_account; account;
                     account = account->next)
                {
                        pager_printf(ch, "%s: Count %d\n\r", account->name,
                                     account->inuse);
                }
        }
        send_to_char("&Bs&zhowalts <&wchar&B|&waccount&z>\n\r", ch);
        return;
}

CMDF do_switchchar(CHAR_DATA * ch, char *argument)
{
        DESCRIPTOR_DATA *d = ch->desc;
        int       count = 0;
        bool      loaded;

        if (d == NULL)
                return;
        if (d->account == NULL)
                return;

        if (IS_NPC(ch))
                return;

        set_char_color(AT_PLAIN, ch);
        if (argument[0] == '\0')
        {
                set_pager_color(AT_PLAIN, ch);
                send_to_pager
                        ("&RSyntax: &Gswitchchar &C<&ccharacter name&C>&w\n\r\n\r",
                         ch);
                send_to_pager("Your choices are:\n\r", ch);
                for (count = 0; count < MAX_CHARACTERS; count++)
                {
                        if (d->account->character[count] == NULL)
                                continue;
                        if (!str_cmp(ch->name, d->account->character[count]))
                                continue;
                        pager_printf(ch, "\t&G%s\n\r",
                                     d->account->character[count]);
                }
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                set_char_color(AT_RED, ch);
                send_to_char("No way! You are fighting.\n\r", ch);
                return;
        }

        if (auction->item != NULL
            && ((ch == auction->buyer) || (ch == auction->seller)))
        {
                send_to_char
                        ("Wait until you have bought/sold the item on auction.\n\r",
                         ch);
                return;
        }

        if (!IS_IMMORTAL(ch) && ch->in_room
            && !xIS_SET(ch->in_room->room_flags, ROOM_HOTEL)
            && !NOT_AUTHED(ch))
        {
                send_to_char("You may not quit here.\n\r", ch);
                send_to_char
                        ("You will have to find a safer resting place such as a hotel...\n\r",
                         ch);
                send_to_char("Maybe you could HAIL a speeder.\n\r", ch);
                return;
        }

        for (count = 0; count < MAX_CHARACTERS; count++)
        {
                if (d->account->character[count] == NULL)
                        continue;
                if (!str_cmp(argument, d->account->character[count]))
                        break;
        }
        if (d->account->character[count] == NULL || count == MAX_CHARACTERS)
        {
                send_to_char("You do not have that character linked.\n\r",
                             ch);
                return;
        }
        if (!str_cmp(ch->name, d->account->character[count]))
        {
                send_to_char("You are already on as them.\n\r", ch);
                return;
        }

        save_char_obj(ch);
        save_account(ch->pcdata->account);
        save_home(ch);

        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        log_string_plus(log_buf, LOG_COMM, get_trust(ch));
        if (!IS_SET(ch->act, PLR_WIZINVIS))
        {
                snprintf(log_buf, MSL, "%s has left %s", ch->name,
                         sysdata.mud_name);
                info_chan(log_buf);
        }
        ch->desc = NULL;
        d->character = NULL;
        d->account = NULL;

        /*
         * So and so has left the game 
         */
        extract_char(ch, TRUE);
        /*
         * Check here to see if it exists in account 
         */
        argument = capitalize(argument);
        loaded = load_char_obj(d, argument, FALSE, FALSE);
        if (loaded)
        {
                d->account = d->character->pcdata->account;
                ch = d->character;

				if (!check_reconnect(d, argument, TRUE)) {
					add_char(d->character);
				}

                if (ch->next)
                        ch->next->prev = ch;
                if (ch->prev)
                        ch->prev->next = ch;

                if (!IS_SET(ch->act, PLR_WIZINVIS))
                {
                        snprintf(log_buf, MSL, "%s has entered %s", ch->name,
                                 sysdata.mud_name);
                        info_chan(log_buf);
                }
                snprintf(log_buf, MSL, "%s@%s has connected.", ch->name,
                         d->host);
                if (ch->top_level < LEVEL_DEMI)
                {
                        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                }
                else
                        log_string_plus(log_buf, LOG_COMM, ch->top_level);
                if (ch->pcdata->area)
                        do_loadarea(ch, "");


				if (!IS_IMMORTAL(ch)
						&& ch->pcdata->release_date > current_time)
				{
					if (ch->in_room)
						char_from_room(ch);
					char_to_room(ch, get_room_index(6));
				}
				else if (ch->in_room && !IS_IMMORTAL(ch)
						&& ch->in_room->vnum != 6)
				{
					ROOM_INDEX_DATA * room = ch->in_room;
					if (ch->in_room)
						char_from_room(ch);
					char_to_room(ch, room);
				}
				else
				{
					if (ch->in_room)
						char_from_room(ch);
					ch->in_room = get_room_index(wherehome(ch));
					char_to_room(ch, ch->in_room);
				}
                return;
        }
        else
        {
                write_to_buffer(d,
                                "\n\rYour player file could not be loaded.\n\r",
                                0);
                extract_char(ch, TRUE);
                close_socket(d, FALSE);
        }

}
