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
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "editor.h"
#include "boards.h"
#include "account.h"

void      note_attach(CHAR_DATA * ch);

void comment_remove(CHAR_DATA * ch, CHAR_DATA * victim, NOTE_DATA * pnote)
{
        ch = NULL;
        if (!victim->pcdata || !victim->pcdata->account)
        {
                bug("comment remove: null account", 0);
                return;
        }
        if (!victim->pcdata->account->comments)
        {
                bug("comment remove: null board", 0);
                return;
        }

        if (!pnote)
        {
                bug("comment remove: null pnote", 0);
                return;
        }

        /*
         * Remove comment from linked list.
         */
        if (!pnote->prev)
                victim->pcdata->account->comments = pnote->next;
        else
                pnote->prev->next = pnote->next;

        STRFREE(pnote->text);
        STRFREE(pnote->subject);
        STRFREE(pnote->to_list);
        STRFREE(pnote->date);
        STRFREE(pnote->sender);
        DISPOSE(pnote);

        /*
         * Rewrite entire list.
         */
        save_char_obj(victim);

        return;
}

CMDF do_comment(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        NOTE_DATA *pnote;
        CHAR_DATA *victim;
        int       vnum;
        int       anum;

        if (IS_NPC(ch))
        {
                send_to_char("Mobs can't use the comment command.\n\r", ch);
                return;
        }

        if (!ch->desc)
        {
                bug("do_comment: no descriptor", 0);
                return;
        }

        /*
         * Put in to prevent crashing when someone issues a comment command
         * from within the editor. -Narn 
         */
        if (ch->desc->connected == CON_EDITING)
        {
                send_to_char
                        ("You can't use the comment command from within the editor.\n\r",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                break;
        case SUB_WRITING_NOTE:
                if (!ch->pnote)
                {
                        bug("do_comment: note got lost?", 0);
                        send_to_char("Your note got lost!\n\r", ch);
                        stop_editing(ch);
                        return;
                }
                if (ch->dest_buf != ch->pnote)
                        bug("do_comment: sub_writing_note: ch->dest_buf != ch->pnote", 0);
                STRFREE(ch->pnote->text);
                ch->pnote->text = copy_buffer(ch);
                stop_editing(ch);
                return;
        }

        set_char_color(AT_NOTE, ch);
        argument = one_argument(argument, arg);
        smash_tilde(argument);

        if (!str_cmp(arg, "about"))
        {
                victim = get_char_world(ch, argument);
                if (!victim)
                {
                        send_to_char("They're not logged on!\n\r", ch); /* maybe fix this? */
                        return;
                }

                if (IS_NPC(victim))
                {
                        send_to_char("No comments about mobs\n\r", ch);
                        return;
                }
        }


        if (!str_cmp(arg, "list"))
        {
                victim = get_char_world(ch, argument);
                if (!victim)
                {
                        send_to_char("They're not logged on!\n\r", ch); /* maybe fix this? */
                        return;
                }

                if (IS_NPC(victim))
                {
                        send_to_char("No comments about mobs\n\r", ch);
                        return;
                }

                if (get_trust(victim) >= get_trust(ch))
                {
                        send_to_char
                                ("You're not of the right caliber to do this...\n\r",
                                 ch);
                        return;
                }

                if (!victim->pcdata->account->comments)
                {
                        send_to_char("There are no relevant comments.\n\r",
                                     ch);
                        return;
                }

                vnum = 0;
                for (pnote = victim->pcdata->account->comments; pnote;
                     pnote = pnote->next)
                {
                        vnum++;
                        snprintf(buf, MSL, "%2d) %-10s [%s] %s\n\r",
                                 vnum, pnote->sender, pnote->date,
                                 pnote->subject);
/* Brittany added date to comment list and whois with above change */
                        send_to_char(buf, ch);
                }

                /*
                 * act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_ROOM ); 
                 */
                return;
        }

        if (!str_cmp(arg, "read"))
        {
                bool      fAll;

                argument = one_argument(argument, arg1);
                victim = get_char_world(ch, arg1);
                if (!victim)
                {
                        send_to_char("They're not logged on!\n\r", ch); /* maybe fix this? */
                        return;
                }

                if (IS_NPC(victim))
                {
                        send_to_char("No comments about mobs\n\r", ch);
                        return;
                }

                if (get_trust(victim) >= get_trust(ch))
                {
                        send_to_char
                                ("You're not of the right caliber to do this...\n\r",
                                 ch);
                        return;
                }

                if (!victim->pcdata->account->comments)
                {
                        send_to_char("There are no relevant comments.\n\r",
                                     ch);
                        return;
                }



                if (!str_cmp(argument, "all"))
                {
                        fAll = TRUE;
                        anum = 0;
                }
                else if (is_number(argument))
                {
                        fAll = FALSE;
                        anum = atoi(argument);
                }
                else
                {
                        send_to_char("Note read which number?\n\r", ch);
                        return;
                }

                vnum = 0;
                for (pnote = victim->pcdata->account->comments; pnote;
                     pnote = pnote->next)
                {
                        vnum++;
                        if (vnum == anum || fAll)
                        {
                                snprintf(buf, MSL,
                                         "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                                         vnum, pnote->sender, pnote->subject,
                                         pnote->date, pnote->to_list);
                                send_to_char(buf, ch);
                                send_to_char(pnote->text, ch);
                                /*
                                 * act( AT_ACTION, "$n reads a note.", ch, NULL, NULL, TO_ROOM ); 
                                 */
                                return;
                        }
                }

                send_to_char("No such comment.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "write"))
        {
                note_attach(ch);
                ch->substate = SUB_WRITING_NOTE;
                ch->dest_buf = ch->pnote;
                start_editing(ch, ch->pnote->text);
                return;
        }

        if (!str_cmp(arg, "subject"))
        {
                note_attach(ch);
                STRFREE(ch->pnote->subject);
                ch->pnote->subject = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "to"))
        {
                note_attach(ch);
                STRFREE(ch->pnote->to_list);
                ch->pnote->to_list = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "clear"))
        {
                if (ch->pnote)
                {
                        STRFREE(ch->pnote->text);
                        STRFREE(ch->pnote->subject);
                        STRFREE(ch->pnote->to_list);
                        STRFREE(ch->pnote->date);
                        STRFREE(ch->pnote->sender);
                        DISPOSE(ch->pnote);
                }
                ch->pnote = NULL;

                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "show"))
        {
                if (!ch->pnote)
                {
                        send_to_char("You have no comment in progress.\n\r",
                                     ch);
                        return;
                }

                snprintf(buf, MSL, "%s: %s\n\rTo: %s\n\r",
                         ch->pnote->sender,
                         ch->pnote->subject, ch->pnote->to_list);
                send_to_char(buf, ch);
                send_to_char(ch->pnote->text, ch);
                return;
        }

        if (!str_cmp(arg, "post"))
        {
                char     *strtime;

                if (!ch->pnote)
                {
                        send_to_char("You have no comment in progress.\n\r",
                                     ch);
                        return;
                }

                argument = one_argument(argument, arg1);
                victim = get_char_world(ch, arg1);
                if (!victim)
                {
                        send_to_char("They're not logged on!\n\r", ch); /* maybe fix this? */
                        return;
                }

                if (IS_NPC(victim))
                {
                        send_to_char("No comments about mobs\n\r", ch);
                        return;
                }

                if (get_trust(victim) > get_trust(ch))
                {
                        send_to_char
                                ("You're not of the right caliber to do this...\n\r",
                                 ch);
                        return;
                }

                /*
                 * act( AT_ACTION, "$n posts a note.", ch, NULL, NULL, TO_ROOM ); 
                 */

                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                ch->pnote->date = STRALLOC(strtime);

                pnote = ch->pnote;
                ch->pnote = NULL;


                /*
                 * LIFO to make life easier 
                 */
                pnote->next = victim->pcdata->account->comments;
                if (victim->pcdata->account->comments)
                        victim->pcdata->account->comments->prev = pnote;
                pnote->prev = NULL;
                victim->pcdata->account->comments = pnote;

                save_char_obj(victim);


#ifdef NOTDEFD
                FCLOSE(fpReserve);
                snprintf(notefile, MSL, "%s/%s", BOARD_DIR, board->note_file);
                if ((fp = fopen(notefile, "a")) == NULL)
                {
                        perror(notefile);
                }
                else
                {
                        fprintf(fp,
                                "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nText\n%s~\n\n",
                                pnote->sender, pnote->date, pnote->to_list,
                                pnote->subject, pnote->text);
                        FCLOSE(fp);
                }
                fpReserve = fopen(NULL_FILE, "r");
#endif

                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "remove"))
        {
                argument = one_argument(argument, arg1);
                victim = get_char_world(ch, arg1);
                if (!victim)
                {
                        send_to_char("They're not logged on!\n\r", ch); /* maybe fix this? */
                        return;
                }

                if (IS_NPC(victim))
                {
                        send_to_char("No comments about mobs\n\r", ch);
                        return;
                }

                if ((get_trust(victim) >= get_trust(ch)) || (get_trust(ch) < LEVEL_IMMORTAL))   /* switch to some LEVEL_ thingie */
                {
                        send_to_char
                                ("You're not of the right caliber to do this...\n\r",
                                 ch);
                        return;
                }

                /*
                 * argument = one_argument(argument, arg); 
                 */
                if (!is_number(argument))
                {
                        send_to_char("Comment remove which number?\n\r", ch);
                        return;
                }

                anum = atoi(argument);
                vnum = 0;
                for (pnote = victim->pcdata->account->comments; pnote;
                     pnote = pnote->next)
                {
                        vnum++;
                        if ((LEVEL_IMMORTAL <= get_trust(ch))   /* switch to some LEVEL_ thingie */
                            && (vnum == anum))
                        {
                                comment_remove(ch, victim, pnote);
                                send_to_char("Ok.\n\r", ch);
                                /*
                                 * act( AT_ACTION, "$n removes a note.", ch, NULL, NULL, TO_ROOM ); 
                                 */
                                return;
                        }
                }

                send_to_char("No such comment.\n\r", ch);
                return;
        }

        send_to_char("Huh?  Type 'help comment' for usage (i hope!).\n\r",
                     ch);
        return;
}


void fwrite_comments(ACCOUNT_DATA * account, FILE * fp)
{
        NOTE_DATA *pnote;

        if (!account->comments)
                return;

        for (pnote = account->comments; pnote; pnote = pnote->next)
        {
                fprintf(fp, "#COMMENT\n");
                fprintf(fp, "sender	%s~\n", pnote->sender);
                fprintf(fp, "date  	%s~\n", pnote->date);
                fprintf(fp, "to     	%s~\n", pnote->to_list);
                fprintf(fp, "subject	%s~\n", pnote->subject);
                fprintf(fp, "text\n%s~\n", pnote->text);
        }
        return;
}

void fread_comment(ACCOUNT_DATA * account, FILE * fp)
{
        NOTE_DATA *pnote;

        for (;;)
        {
                char      letter;

                do
                {
                        letter = getc(fp);
                        if (feof(fp))
                        {
                                FCLOSE(fp);
                                return;
                        }
                }
                while (isspace(letter));
                ungetc(letter, fp);

                CREATE(pnote, NOTE_DATA, 1);

                if (str_cmp(fread_word(fp), "sender"))
                        break;
                pnote->sender = fread_string(fp);

                if (str_cmp(fread_word(fp), "date"))
                        break;
                pnote->date = fread_string(fp);

                if (str_cmp(fread_word(fp), "to"))
                        break;
                pnote->to_list = fread_string(fp);

                if (str_cmp(fread_word(fp), "subject"))
                        break;
                pnote->subject = fread_string(fp);

                if (str_cmp(fread_word(fp), "text"))
                        break;
                pnote->text = fread_string(fp);

                pnote->next = account->comments;
                pnote->prev = NULL;
                account->comments = pnote;
                return;
        }

        bug("fread_comment: bad key word. strap in!", 0);
        free_note(pnote);
}




/*
<758hp 100m 690mv> <#10316> loadup boo
Log: Haus: loadup boo
Log: Reading in player data for: Boo
Done.
<758hp 100m 690mv> <#10316> poke boo
You poke him in the ribs.
<758hp 100m 690mv> <#10316> comment subject boo's a nutcase!
Ok.
<758hp 100m 690mv> <#10316> comment to all
Ok.
<758hp 100m 690mv> <#10316> comment write
Begin entering your text now (/? = help /s = save /c = clear /l = list)
-----------------------------------------------------------------------
> He transed shimmy to temple square!
> /s
Done.
<758hp 100m 690mv> <#10316> comment post boo
Ok.
<758hp 100m 690mv> <#10316> comment list boo
 1) Haus: boo's a nutcase!
<758hp 100m 690mv> <#10316> comment read boo 1
[  1] Haus: boo's a nutcase!
Sun Jun 25 18:26:54 1995
To: all
He transed shimmy to temple square!
<758hp 100m 690mv> <#10316> comment remove boo 1
Ok.
<758hp 100m 690mv> <#10316> comment list boo
There are no relevent comments.
*/


void comment_add_comment(CHAR_DATA * from, ACCOUNT_DATA * account, char * subject, char * text)
{
	NOTE_DATA * pnote;
	char     *strtime;
	CREATE(pnote, NOTE_DATA, 1);
	pnote->sender = STRALLOC(from->name);;
	pnote->to_list = STRALLOC("");
	pnote->subject = STRALLOC(subject);
	pnote->text = STRALLOC(text);

	strtime = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';
	pnote->date = STRALLOC(strtime);
	pnote->next = account->comments;
	if (account->comments)
		account->comments->prev = pnote;
	pnote->prev = NULL;
	account->comments = pnote;
	save_account(account);
	return;
}
