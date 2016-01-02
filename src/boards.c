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
#include <time.h>
#include <sys/stat.h>
#include "mud.h"
#ifdef MXP
#include "mxp.h"
#endif
#include "editor.h"
#include "boards.h"

void free_board args((BOARD_DATA * board));

char     *const board_types[BOARD_MAX] = { "Note", "Mail", "Idea", "Global" };
char     *const vote_types[VOTE_MAX] = { "None", "Open", "Closed" };

BOARD_DATA *first_board;
BOARD_DATA *last_board;

int get_boardtypes(char *flag)
{
        unsigned int x;

        for (x = 0; x < BOARD_MAX; x++)
                if (!str_cmp(flag, board_types[x]))
                        return x;
        return -1;
}

bool can_remove(CHAR_DATA * ch, BOARD_DATA * board)
{
        /*
         * If your trust is high enough, you can remove it. 
         */
        if (get_trust(ch) >= board->min_remove_level)
                return TRUE;

        if (board->extra_removers[0] != '\0')
        {
                if (is_name(ch->name, board->extra_removers))
                        return TRUE;
        }
        return FALSE;
}

bool can_read(CHAR_DATA * ch, BOARD_DATA * board)
{
        /*
         * If your trust is high enough, you can read it. 
         */
        if (get_trust(ch) >= board->min_read_level)
                return TRUE;

        /*
         * Your trust wasn't high enough, so check if a read_group or extra
         * * readers have been set up. 
         */
        if (board->read_group[0] != '\0')
        {
                if (ch->pcdata->clan
                    && !str_cmp(ch->pcdata->clan->name, board->read_group))
                        return TRUE;
                if (ch->pcdata->clan && ch->pcdata->clan->mainclan
                    && !str_cmp(ch->pcdata->clan->mainclan->name,
                                board->read_group))
                        return TRUE;

        }
        if (board->extra_readers[0] != '\0')
        {
                if (is_name(ch->name, board->extra_readers))
                        return TRUE;
        }
        return FALSE;
}

bool can_post(CHAR_DATA * ch, BOARD_DATA * board)
{
        /*
         * If your trust is high enough, you can post. 
         */
        if (get_trust(ch) >= board->min_post_level)
                return TRUE;

        /*
         * Your trust wasn't high enough, so check if a post_group has been set up. 
         */
        if (board->post_group[0] != '\0')
        {
                if (ch->pcdata->clan
                    && !str_cmp(ch->pcdata->clan->name, board->post_group))
                        return TRUE;
                if (ch->pcdata->clan && ch->pcdata->clan->mainclan
                    && !str_cmp(ch->pcdata->clan->mainclan->name,
                                board->post_group))
                        return TRUE;
        }
        return FALSE;
}

/*
 * board commands.
 */
void write_boards_txt(void)
{
        BOARD_DATA *tboard;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", BOARD_DIR, BOARD_FILE);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open board.txt for writing!\n\r", 0);
                return;
        }
        for (tboard = first_board; tboard; tboard = tboard->next)
        {
                fprintf(fpout, "Filename          %s~\n", tboard->note_file);
                if (tboard->board_name && tboard->board_name[0] != '\0')
                        fprintf(fpout, "Name              %s~\n",
                                tboard->board_name);
                fprintf(fpout, "Vnum              %d\n", tboard->board_obj);
                fprintf(fpout, "Min_read_level    %d\n",
                        tboard->min_read_level);
                fprintf(fpout, "Min_post_level    %d\n",
                        tboard->min_post_level);
                fprintf(fpout, "Min_remove_level  %d\n",
                        tboard->min_remove_level);
                fprintf(fpout, "Max_posts         %d\n", tboard->max_posts);
                fprintf(fpout, "Type              %d\n", tboard->type);
                fprintf(fpout, "Read_group        %s~\n", tboard->read_group);
                fprintf(fpout, "Post_group        %s~\n", tboard->post_group);
                fprintf(fpout, "Extra_readers     %s~\n",
                        tboard->extra_readers);
                fprintf(fpout, "Extra_removers    %s~\n",
                        tboard->extra_removers);

                fprintf(fpout, "End\n");
        }
        FCLOSE(fpout);
}

BOARD_DATA *get_board(OBJ_DATA * obj)
{
        BOARD_DATA *board;

        for (board = first_board; board; board = board->next)
                if (board->board_obj == obj->pIndexData->vnum)
                        return board;
        return NULL;
}

BOARD_DATA *find_board(CHAR_DATA * ch)
{
        OBJ_DATA *obj;
        BOARD_DATA *board;

        for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
        {
                if ((board = get_board(obj)) != NULL)
                        return board;
        }

        return NULL;
}

bool is_note_to(CHAR_DATA * ch, NOTE_DATA * pnote)
{
        if (!ch || !pnote)
                return FALSE;

        if (!str_cmp(ch->name, pnote->sender))
                return TRUE;

        return is_note_to_def(ch, pnote);
}

bool is_note_to_def(CHAR_DATA * ch, NOTE_DATA * pnote)
{
        if (is_name("all", pnote->to_list))
                return TRUE;

        if (IS_HERO(ch) && is_name("immortal", pnote->to_list))
                return TRUE;

        if (is_name(ch->name, pnote->to_list))
                return TRUE;

        return FALSE;
}



void note_attach(CHAR_DATA * ch)
{
        NOTE_DATA *pnote;

        if (ch->pnote)
                return;

        CREATE(pnote, NOTE_DATA, 1);
        pnote->next = NULL;
        pnote->prev = NULL;
        pnote->sender = QUICKLINK(ch->name);
        pnote->date = STRALLOC("");
        pnote->to_list = STRALLOC("");
        pnote->subject = STRALLOC("");
        pnote->text = STRALLOC("");
        ch->pnote = pnote;
        return;
}

void write_board(BOARD_DATA * board)
{
        FILE     *fp;
        char      filename[256];
        NOTE_DATA *pnote;

        /*
         * Rewrite entire list. 
         */
        FCLOSE(fpReserve);
        snprintf(filename, MSL, "%s%s", BOARD_DIR, board->note_file);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                perror(filename);
        }
        else
        {
                for (pnote = board->first_note; pnote; pnote = pnote->next)
                {
                        fprintf(fp,
                                "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nVoting %d\nYesvotes %s~\nNovotes %s~\nAbstentions %s~\nText\n%s~\n\n",
                                pnote->sender, pnote->date, pnote->to_list,
                                pnote->subject, pnote->voting,
                                pnote->yesvotes, pnote->novotes,
                                pnote->abstentions, pnote->text);
                }
                FCLOSE(fp);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}


void free_note(NOTE_DATA * pnote)
{
	    if (pnote->text)
            STRFREE(pnote->text);
	    if (pnote->subject)
            STRFREE(pnote->subject);
	    if (pnote->to_list)
            STRFREE(pnote->to_list);
	    if (pnote->date)
            STRFREE(pnote->date);
	    if (pnote->sender)
            STRFREE(pnote->sender);
        if (pnote->yesvotes)
                DISPOSE(pnote->yesvotes);
        if (pnote->novotes)
                DISPOSE(pnote->novotes);
        if (pnote->abstentions)
                DISPOSE(pnote->abstentions);
        DISPOSE(pnote);
}

void note_remove(CHAR_DATA * ch, BOARD_DATA * board, NOTE_DATA * pnote)
{
        ch = NULL;
        if (!board)
        {
                bug("note remove: null board", 0);
                return;
        }

        if (!pnote)
        {
                bug("note remove: null pnote", 0);
                return;
        }

        /*
         * Remove note from linked list.
         */
        UNLINK(pnote, board->first_note, board->last_note, next, prev);

        --board->num_posts;
        free_note(pnote);
        write_board(board);
}


CMDF do_noteroom(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;

        if (!str_cmp(argument, "write") ||
            !str_cmp(argument, "show") ||
            !str_cmp(argument, "to") ||
            !str_cmp(argument, "subject") || ch->substate == SUB_WRITING_NOTE)
        {
                note(ch, argument, NULL);
                return;
        }

        board = find_board(ch);

        if (!board)
        {
                send_to_char
                        ("There is no bulletin board here to look at.\n\r",
                         ch);
                return;
        }

        if (board->type != BOARD_NOTE && board->type != BOARD_IDEA)
        {
                send_to_char
                        ("You can only use note commands on a message terminal.\n\r",
                         ch);
                return;
        }

        note(ch, argument, board);
}

CMDF do_mailroom(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;

        if (argument[0] == '\0' && ch->substate != SUB_WRITING_NOTE)
        {
                mudstrlcpy(argument, "list", MIL);
        }

        if (!str_cmp(argument, "write") ||
            !str_cmp(argument, "show") ||
            !str_cmp(argument, "to") ||
            !str_cmp(argument, "subject") || ch->substate == SUB_WRITING_NOTE)
        {
                note(ch, argument, NULL);
                return;
        }

        board = find_board(ch);
        if (!board || board->type != BOARD_MAIL)
        {
                for (board = first_board; board; board = board->next)
                        if (board->board_obj == VNUM_MAIL_BOARD)
                                break;
        }

        if (!board)
        {
                bug("Can't find the mail board", 0);
                send_to_char("There is no mail facility here.\n\r", ch);
                return;
        }

        if (board->type != BOARD_MAIL)
        {
                bug("Mail board has wrong type", 0);
                send_to_char
                        ("You can only use mail commands in a post office.\n\r",
                         ch);
                return;
        }

        note(ch, argument, board);
}

CMDF do_idearoom(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0' && ch->substate != SUB_WRITING_NOTE)
        {
                mudstrlcpy(argument, "list", MIL);
        }

        if (!str_cmp(argument, "write") ||
            !str_cmp(argument, "show") ||
            !str_cmp(argument, "to") ||
            !str_cmp(argument, "subject") || ch->substate == SUB_WRITING_NOTE)
        {
                note(ch, argument, NULL);
                return;
        }

        board = find_board(ch);
        if (!board || board->type != BOARD_IDEA)
        {
                for (board = first_board; board; board = board->next)
                        if (board->board_obj == VNUM_IDEA_BOARD)
                                break;
        }

        if (!board)
        {
                bug("Can't find the idea board", 0);
                send_to_char("There is no mail facility available.\n\r", ch);
                return;
        }

        if (board->type != BOARD_IDEA)
        {
                bug("Idea board has wrong type", 0);
                send_to_char
                        ("You can only use idea commands if an idea board exists.\n\r",
                         ch);
                return;
        }

        note(ch, argument, board);
}


void note(CHAR_DATA * ch, char *arg_passed, BOARD_DATA * board)
{
        bool      IS_MAIL = board ? (board->type == BOARD_MAIL) : FALSE;
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        NOTE_DATA *pnote;
        int       vnum;
        int       anum;
        int       first_plist;
        OBJ_DATA *paper = NULL, *tmpobj = NULL;
        EXTRA_DESCR_DATA *ed = NULL;
        char      notebuf[MAX_STRING_LENGTH];
        char      short_desc_buf[MAX_STRING_LENGTH];
        char      long_desc_buf[MAX_STRING_LENGTH];
        char      keyword_buf[MAX_STRING_LENGTH];
        bool      wasfound = FALSE;

        if (IS_NPC(ch))
                return;

        if (!ch->desc)
        {
                bug("do_note: no descriptor", 0);
                return;
        }

        CHECK_SUBRESTRICTED(ch);
        switch (ch->substate)
        {
        default:
                break;
        case SUB_WRITING_NOTE:
                {
                        if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                            || paper->item_type != ITEM_PAPER)
                        {
                                bug("do_note: player not holding paper", 0);
                                stop_editing(ch);
                                return;
                        }
                        ed = (EXTRA_DESCR_DATA *) ch->dest_buf;
                        STRFREE(ed->description);
                        ed->description = copy_buffer(ch);
                        stop_editing(ch);
                        return;
                }
        }

        set_char_color(AT_NOTE, ch);
        arg_passed = one_argument(arg_passed, arg);
        smash_tilde(arg_passed);

        if (!str_cmp(arg, "list"))
        {
                if (!board)
                {
                        send_to_char("There is no board here to look at.\n\r",
                                     ch);
                        return;
                }

                if (!can_read(ch, board))
                {
                        send_to_char
                                ("You cannot make any sense of the cryptic scrawl on this board...\n\r",
                                 ch);
                        return;
                }

                first_plist = atoi(arg_passed);
                if (first_plist)
                {
                        if (IS_MAIL)
                        {
                                send_to_char
                                        ("You cannot use a list number (at this time) with mail.\n\r",
                                         ch);
                                return;
                        }

                        if (first_plist < 1)
                        {
                                send_to_char
                                        ("You can't read a message before 1!\n\r",
                                         ch);
                                return;
                        }
                }

                vnum = 0;
                set_pager_color(AT_NOTE, ch);
                for (pnote = board->first_note; pnote; pnote = pnote->next)
                {
                        if (IS_MAIL
                            && (!is_note_to(ch, pnote)
                                && !(get_trust(ch) > sysdata.read_all_mail)))
                                continue;

                        vnum++;
                        if ((first_plist && vnum >= first_plist)
                            || !first_plist)
                        {
                                pager_printf(ch, "%2d%c %-12s%c %-12s ",
                                             vnum,
                                             is_note_to(ch,
                                                        pnote) ? ')' : '}',
                                             pnote->sender,
                                             (pnote->voting !=
                                              VOTE_NONE) ? (pnote->voting ==
                                                            VOTE_OPEN ? 'V' :
                                                            'C') : ':',
                                             pnote->to_list);
#ifdef MXP
                                if (IS_MXP(ch))
                                {
                                        pager_printf(ch,
                                                     MXPTAG("note '%s' '%d'"),
                                                     board_types[board->type],
                                                     vnum);
                                }
#endif
                                send_to_pager(pnote->subject, ch);
#ifdef MXP
                                if (IS_MXP(ch))
                                        send_to_pager(MXPTAG("/note"), ch);
#endif
                                send_to_pager("\n\r", ch);
                        }
                }
                if (!board->first_note || vnum == 0)
                {
                        send_to_char("There are no notes on this board.\n\r",
                                     ch);
                }

                if (IS_MAIL && vnum == 0)
                {
                        send_to_char("You have no mail.\n\r", ch);
                        return;
                }
                if (board->type == BOARD_NOTE)
                        act(AT_ACTION, "$n glances over the messages.", ch,
                            NULL, NULL, TO_ROOM);
                return;
        }
        else if (!str_cmp(arg, "read"))
        {
                bool      fAll;

                if (!board)
                {
                        send_to_char("There is no board here to look at.\n\r",
                                     ch);
                        return;
                }

                if (!can_read(ch, board))
                {
                        send_to_char
                                ("You cannot make any sense of the cryptic scrawl on this board...\n\r",
                                 ch);
                        return;
                }

                if (!str_cmp(arg_passed, "all"))
                {
                        fAll = TRUE;
                        anum = 0;
                }
                else if (is_number(arg_passed))
                {
                        fAll = FALSE;
                        anum = atoi(arg_passed);
                }
                else
                {
                        send_to_char("Note read which number?\n\r", ch);
                        return;
                }

                set_pager_color(AT_NOTE, ch);
                vnum = 0;
                for (pnote = board->first_note; pnote; pnote = pnote->next)
                {
                        if (IS_MAIL && !is_note_to(ch, pnote)
                            && get_trust(ch) < sysdata.read_all_mail)
                                continue;

                        vnum++;
                        if (vnum == anum || fAll)
                        {
                                wasfound = TRUE;
                                if (IS_MAIL
                                    && get_trust(ch) < sysdata.read_mail_free)
                                {
                                        if (ch->gold < 10)
                                        {
                                                send_to_char
                                                        ("It costs 10 credits to read a message.\n\r",
                                                         ch);
                                                return;
                                        }
                                        ch->gold -= 10;
                                }

                                pager_printf(ch,
                                             "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r%s",
                                             vnum, pnote->sender,
                                             pnote->subject, pnote->date,
                                             pnote->to_list, pnote->text);

                                if (pnote->yesvotes[0] != '\0'
                                    || pnote->novotes[0] != '\0'
                                    || pnote->abstentions[0] != '\0')
                                {
                                        send_to_pager
                                                ("------------------------------------------------------------\n\r",
                                                 ch);
                                        pager_printf(ch,
                                                     "Votes:\n\rYes:     %s\n\rNo:      %s\n\rAbstain: %s\n\r",
                                                     pnote->yesvotes,
                                                     pnote->novotes,
                                                     pnote->abstentions);
                                }
                                if (board->type == BOARD_NOTE)
                                        act(AT_ACTION, "$n reads a message.",
                                            ch, NULL, NULL, TO_ROOM);
                        }
                }
                if (!wasfound)
                        ch_printf(ch, "No such message: %d\n\r", anum);
                return;
        }
        /*
         * Voting added by Narn, June '96 
         */
        else if (!str_cmp(arg, "vote"))
        {
                char      arg2[MAX_INPUT_LENGTH];

                if (!board)
                {
                        send_to_char("There is no board here to look at.\n\r",
                                     ch);
                        return;
                }

                if (!can_read(ch, board))
                {
                        send_to_char
                                ("You cannot make any sense of the cryptic scrawl on this board...\n\r",
                                 ch);
                        return;
                }
                if (IS_MAIL)
                {
                        send_to_char("You can not vote on a mail.\n\r", ch);
                        return;
                }

                arg_passed = one_argument(arg_passed, arg2);
                if (is_number(arg2))
                        anum = atoi(arg2);
                else
                {
                        send_to_char("Note vote which number?\n\r", ch);
                        return;
                }

                vnum = 1;
                for (pnote = board->first_note; pnote && vnum < anum;
                     pnote = pnote->next)
                        vnum++;
                if (!pnote)
                {
                        send_to_char("No such note.\n\r", ch);
                        return;
                }

                /*
                 * Options: open close yes no abstain 
                 */
                /*
                 * If you're the author of the note and can read the board you can open 
                 * * and close voting, if you can read it and voting is open you can vote.
                 */
                if (!str_cmp(arg_passed, "open"))
                {
                        if (str_cmp(ch->name, pnote->sender))
                        {
                                send_to_char
                                        ("You are not the author of this message.\n\r",
                                         ch);
                                return;
                        }
                        pnote->voting = VOTE_OPEN;
                        if (board->type == BOARD_NOTE)
                                act(AT_ACTION, "$n opens voting on a note.",
                                    ch, NULL, NULL, TO_ROOM);
                        send_to_char("Voting opened.\n\r", ch);
                        write_board(board);
                        return;
                }
                if (!str_cmp(arg_passed, "close"))
                {
                        if (str_cmp(ch->name, pnote->sender))
                        {
                                send_to_char
                                        ("You are not the author of this message.\n\r",
                                         ch);
                                return;
                        }
                        pnote->voting = VOTE_CLOSED;
                        if (board->type == BOARD_NOTE)
                                act(AT_ACTION, "$n closes voting on a note.",
                                    ch, NULL, NULL, TO_ROOM);
                        send_to_char("Voting closed.\n\r", ch);
                        write_board(board);
                        return;
                }

                /*
                 * Make sure the note is open for voting before going on. 
                 */
                if (pnote->voting != VOTE_OPEN)
                {
                        send_to_char("Voting is not open on this note.\n\r",
                                     ch);
                        return;
                }

                /*
                 * Can only vote once on a note. 
                 */
                snprintf(buf, MSL, "%s %s %s",
                         pnote->yesvotes, pnote->novotes, pnote->abstentions);
                if (is_name(ch->name, buf))
                {
                        send_to_char
                                ("You have already voted on this note.\n\r",
                                 ch);
                        return;
                }

                if (!str_cmp(arg_passed, "yes"))
                {
                        snprintf(buf, MSL, "%s %s", pnote->yesvotes,
                                 ch->name);
                        DISPOSE(pnote->yesvotes);
                        pnote->yesvotes = str_dup(buf);
                }
                else if (!str_cmp(arg_passed, "no"))
                {
                        snprintf(buf, MSL, "%s %s", pnote->novotes, ch->name);
                        DISPOSE(pnote->novotes);
                        pnote->novotes = str_dup(buf);
                }
                else if (!str_cmp(arg_passed, "abstain"))
                {
                        snprintf(buf, MSL, "%s %s", pnote->abstentions,
                                 ch->name);
                        DISPOSE(pnote->abstentions);
                        pnote->abstentions = str_dup(buf);
                }
                else
                {

                        bug("note( ch, \"\", board );");
                        note(ch, "", board);
                        return;
                }
                if (board->type == BOARD_NOTE)
                        act(AT_ACTION, "$n votes on a note.", ch, NULL, NULL,
                            TO_ROOM);
                send_to_char("Ok.\n\r", ch);
                write_board(board);
                return;
        }
        else if (!str_cmp(arg, "write"))
        {
                if (ch->substate == SUB_RESTRICTED)
                {
                        send_to_char
                                ("You cannot write a note from within another command.\n\r",
                                 ch);
                        return;
                }
                if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                    || paper->item_type != ITEM_PAPER)
                {
                        if (get_trust(ch) < sysdata.write_mail_free)
                        {
                                send_to_char
                                        ("You need to be holding a message disk to write a note.\n\r",
                                         ch);
                                return;
                        }
                        paper = create_object(get_obj_index(OBJ_VNUM_NOTE),
                                              0);
                        if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
                                unequip_char(ch, tmpobj);
                        paper = obj_to_char(paper, ch);
                        equip_char(ch, paper, WEAR_HOLD);
                        act(AT_MAGIC,
                            "$n grabs a message disk to record a note.", ch,
                            NULL, NULL, TO_ROOM);
                        act(AT_MAGIC,
                            "You get a message disk to record your note.", ch,
                            NULL, NULL, TO_CHAR);
                }

                if (paper->value[0] < 2)
                {
                        paper->value[0] = 1;
                        ed = SetOExtra(paper, "_text_");
                        ch->substate = SUB_WRITING_NOTE;
                        ch->dest_buf = ed;
                        start_editing(ch, ed->description);
                        return;
                }
                else
                {
                        send_to_char("You cannot modify this message.\n\r",
                                     ch);
                        return;
                }
        }
        else if (!str_cmp(arg, "subject"))
        {
                if (!arg_passed || arg_passed[0] == '\0')
                {
                        send_to_char
                                ("What do you wish the subject to be?\n\r",
                                 ch);
                        return;
                }
                if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                    || paper->item_type != ITEM_PAPER)
                {
                        if (get_trust(ch) < sysdata.write_mail_free)
                        {
                                send_to_char
                                        ("You need to be holding a message disk to record a note.\n\r",
                                         ch);
                                return;
                        }
                        paper = create_object(get_obj_index(OBJ_VNUM_NOTE),
                                              0);
                        if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
                                unequip_char(ch, tmpobj);
                        paper = obj_to_char(paper, ch);
                        equip_char(ch, paper, WEAR_HOLD);
                        act(AT_MAGIC, "$n grabs a message disk.",
                            ch, NULL, NULL, TO_ROOM);
                        act(AT_MAGIC,
                            "You get a message disk to record your note.", ch,
                            NULL, NULL, TO_CHAR);
                }
                if (paper->value[1] > 1)
                {
                        send_to_char("You cannot modify this message.\n\r",
                                     ch);
                        return;
                }
                else
                {
                        paper->value[1] = 1;
                        ed = SetOExtra(paper, "_subject_");
                        STRFREE(ed->description);
                        ed->description = STRALLOC(arg_passed);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
        }
        else if (!str_cmp(arg, "to"))
        {
                struct stat fst;
                char      fname[1024];

                if (!arg_passed || arg_passed[0] == '\0')
                {
                        send_to_char("Please specify an addressee.\n\r", ch);
                        return;
                }
                if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                    || paper->item_type != ITEM_PAPER)
                {
                        if (get_trust(ch) < sysdata.write_mail_free)
                        {
                                send_to_char
                                        ("You need to be holding a message disk to record a note.\n\r",
                                         ch);
                                return;
                        }
                        paper = create_object(get_obj_index(OBJ_VNUM_NOTE),
                                              0);
                        if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
                                unequip_char(ch, tmpobj);
                        paper = obj_to_char(paper, ch);
                        equip_char(ch, paper, WEAR_HOLD);
                        act(AT_MAGIC,
                            "$n gets a message disk to record a note.", ch,
                            NULL, NULL, TO_ROOM);
                        act(AT_MAGIC,
                            "You grab a message disk to record your note.",
                            ch, NULL, NULL, TO_CHAR);
                }

                if (paper->value[2] > 1)
                {
                        send_to_char("You cannot modify this message.\n\r",
                                     ch);
                        return;
                }

                arg_passed[0] = UPPER(arg_passed[0]);

                snprintf(fname, MSL, "%s%c/%s", PLAYER_DIR,
                         tolower(arg_passed[0]), capitalize(arg_passed));

                if (!IS_MAIL || stat(fname, &fst) != -1
                    || !str_cmp(arg_passed, "all"))
                {
                        paper->value[2] = 1;
                        ed = SetOExtra(paper, "_to_");
                        STRFREE(ed->description);
                        ed->description = STRALLOC(arg_passed);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                else
                {
                        send_to_char("No player exists by that name.\n\r",
                                     ch);
                        return;
                }

        }

        if (!str_cmp(arg, "show"))
        {
                char     *subject, *to_list, *text;

                if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                    || paper->item_type != ITEM_PAPER)
                {
                        send_to_char
                                ("You are not holding a message disk.\n\r",
                                 ch);
                        return;
                }

                if ((subject =
                     get_extra_descr("_subject_",
                                     paper->first_extradesc)) == NULL)
                        subject = "(no subject)";
                if ((to_list =
                     get_extra_descr("_to_", paper->first_extradesc)) == NULL)
                        to_list = "(nobody)";
                snprintf(buf, MSL, "%s: %s\n\rTo: %s\n\r",
                         ch->name, subject, to_list);
                send_to_char(buf, ch);
                if ((text =
                     get_extra_descr("_text_",
                                     paper->first_extradesc)) == NULL)
                        text = "The disk is blank.\n\r";
                send_to_char(text, ch);
                return;
        }
        else if (!str_cmp(arg, "post"))
        {
                char     *strtime, *text;

                if (!board)
                {
                        send_to_char("There is no board here to look at.\n\r",
                                     ch);
                        return;
                }

                if (!can_read(ch, board))
                {
                        send_to_char
                                ("You cannot make any sense of the cryptic scrawl on this board...\n\r",
                                 ch);
                        return;
                }
                if ((paper = get_eq_char(ch, WEAR_HOLD)) == NULL
                    || paper->item_type != ITEM_PAPER)
                {
                        send_to_char
                                ("You are not holding a message disk.\n\r",
                                 ch);
                        return;
                }

                if (paper->value[0] == 0)
                {
                        send_to_char
                                ("There is nothing written on this disk.\n\r",
                                 ch);
                        return;
                }

                if (paper->value[1] == 0)
                {
                        send_to_char
                                ("This message has no subject... using 'none'.\n\r",
                                 ch);
                        paper->value[1] = 1;
                        ed = SetOExtra(paper, "_subject_");
                        STRFREE(ed->description);
                        ed->description = STRALLOC("none");
                }

                if (paper->value[2] == 0)
                {
                        if (IS_MAIL)
                        {
                                send_to_char
                                        ("This message is addressed to no one!\n\r",
                                         ch);
                                return;
                        }
                        else
                        {
                                send_to_char
                                        ("This message is addressed to no one... sending to 'all'!\n\r",
                                         ch);
                                paper->value[2] = 1;
                                ed = SetOExtra(paper, "_to_");
                                STRFREE(ed->description);
                                ed->description = STRALLOC("All");
                        }
                }

                if (!can_post(ch, board))
                {
                        send_to_char
                                ("You cannot use this terminal. It is encrypted...\n\r",
                                 ch);
                        return;
                }

                if (board->max_posts != -1
                    && board->num_posts >= board->max_posts)
                {
                        send_to_char
                                ("This terminal is full. There is no room for your message.\n\r",
                                 ch);
                        return;
                }

                /*
                 * Make sure the note is open for voting before going on. 
                 */

                if (board->type == BOARD_NOTE)
                        act(AT_ACTION, "$n uploads a message.", ch, NULL,
                            NULL, TO_ROOM);

                strtime = ctime(&current_time);
                strtime[strlen(strtime) - 1] = '\0';
                CREATE(pnote, NOTE_DATA, 1);
                pnote->date = STRALLOC(strtime);

                text = get_extra_descr("_text_", paper->first_extradesc);
                pnote->text = text ? STRALLOC(text) : STRALLOC("");
                text = get_extra_descr("_to_", paper->first_extradesc);
                pnote->to_list = text ? STRALLOC(text) : STRALLOC("all");
                text = get_extra_descr("_subject_", paper->first_extradesc);
                pnote->subject = text ? STRALLOC(text) : STRALLOC("");
                pnote->sender = QUICKLINK(ch->name);
                pnote->voting = 0;
                pnote->yesvotes = str_dup("");
                pnote->novotes = str_dup("");
                pnote->abstentions = str_dup("");
                if (board->type == BOARD_IDEA)
                        pnote->voting = VOTE_OPEN;

                LINK(pnote, board->first_note, board->last_note, next, prev);
                board->num_posts++;
                write_board(board);
                send_to_char("You upload your message to the terminal.\n\r",
                             ch);
                extract_obj(paper);
                return;
        }
        else if (!str_cmp(arg, "remove")
                 || !str_cmp(arg, "take") || !str_cmp(arg, "copy"))
        {
                char      take;

                if (!board)
                {
                        send_to_char("There is no board here to look at.\n\r",
                                     ch);
                        return;
                }

                if (!can_read(ch, board))
                {
                        send_to_char
                                ("You cannot make any sense of the cryptic scrawl on this board...\n\r",
                                 ch);
                        return;
                }
                if (!str_cmp(arg, "take"))
                        take = 1;
                else if (!str_cmp(arg, "copy"))
                {
                        if (!IS_IMMORTAL(ch))
                        {
                                send_to_char
                                        ("Huh?  Type 'help note' for usage.\n\r",
                                         ch);
                                return;
                        }
                        take = 2;
                }
                else
                        take = 0;

                if (!is_number(arg_passed))
                {
                        send_to_char("Note remove which number?\n\r", ch);
                        return;
                }

                anum = atoi(arg_passed);
                vnum = 0;
                for (pnote = board->first_note; pnote; pnote = pnote->next)
                {
                        if (IS_MAIL && ((is_note_to(ch, pnote))
                                        || get_trust(ch) >=
                                        sysdata.take_others_mail))
                                vnum++;
                        else if (!IS_MAIL)
                                vnum++;
                        if ((is_note_to(ch, pnote)
                             || can_remove(ch, board)) && (vnum == anum))
                        {
                                if ((is_name("all", pnote->to_list))
                                    && (get_trust(ch) <
                                        sysdata.take_others_mail)
                                    && (take == 1))
                                {
                                        send_to_char
                                                ("Notes addressed to 'all' can not be taken.\n\r",
                                                 ch);
                                        return;
                                }

                                if (take != 0)
                                {
                                        if (ch->gold < 50
                                            && get_trust(ch) <
                                            sysdata.read_mail_free)
                                        {
                                                if (take == 1)
                                                        send_to_char
                                                                ("It costs 50 credits to take your mail.\n\r",
                                                                 ch);
                                                else
                                                        send_to_char
                                                                ("It costs 50 credits to copy your mail.\n\r",
                                                                 ch);
                                                return;
                                        }
                                        if (get_trust(ch) <
                                            sysdata.read_mail_free)
                                                ch->gold -= 50;
                                        paper = create_object(get_obj_index
                                                              (OBJ_VNUM_NOTE),
                                                              0);
                                        ed = SetOExtra(paper, "_sender_");
                                        STRFREE(ed->description);
                                        ed->description =
                                                QUICKLINK(pnote->sender);
                                        ed = SetOExtra(paper, "_text_");
                                        STRFREE(ed->description);
                                        ed->description =
                                                QUICKLINK(pnote->text);
                                        ed = SetOExtra(paper, "_to_");
                                        STRFREE(ed->description);
                                        ed->description =
                                                QUICKLINK(pnote->to_list);
                                        ed = SetOExtra(paper, "_subject_");
                                        STRFREE(ed->description);
                                        ed->description =
                                                QUICKLINK(pnote->subject);
                                        ed = SetOExtra(paper, "_date_");
                                        STRFREE(ed->description);
                                        ed->description =
                                                QUICKLINK(pnote->date);
                                        ed = SetOExtra(paper, "note");
                                        STRFREE(ed->description);
                                        snprintf(notebuf, MSL, "%s",
                                                 "From: ");
                                        mudstrlcat(notebuf, pnote->sender,
                                                   MSL);
                                        mudstrlcat(notebuf, "\n\rTo: ", MSL);
                                        mudstrlcat(notebuf, pnote->to_list,
                                                   MSL);
                                        mudstrlcat(notebuf, "\n\rSubject: ",
                                                   MSL);
                                        mudstrlcat(notebuf, pnote->subject,
                                                   MSL);
                                        mudstrlcat(notebuf, "\n\r\n\r", MSL);
                                        mudstrlcat(notebuf, pnote->text, MSL);
                                        mudstrlcat(notebuf, "\n\r", MSL);
                                        ed->description = STRALLOC(notebuf);
                                        paper->value[0] = 2;
                                        paper->value[1] = 2;
                                        paper->value[2] = 2;

                                        snprintf(short_desc_buf, MSL,
                                                 "a note from %s to %s",
                                                 pnote->sender,
                                                 pnote->to_list);
                                        STRFREE(paper->short_descr);
                                        paper->short_descr =
                                                STRALLOC(short_desc_buf);
                                        snprintf(long_desc_buf, MSL,
                                                 "A note from %s to %s lies on the ground.",
                                                 pnote->sender,
                                                 pnote->to_list);
                                        STRFREE(paper->description);
                                        paper->description =
                                                STRALLOC(long_desc_buf);
                                        snprintf(keyword_buf, MSL,
                                                 "note parchment paper %s",
                                                 pnote->to_list);
                                        STRFREE(paper->name);
                                        paper->name = STRALLOC(keyword_buf);
                                }
                                if (take != 2)
                                        note_remove(ch, board, pnote);
                                send_to_char("Ok.\n\r", ch);
                                if (take == 1)
                                {
                                        act(AT_ACTION,
                                            "$n downloads a message.", ch,
                                            NULL, NULL, TO_ROOM);
                                        obj_to_char(paper, ch);
                                }
                                else if (take == 2)
                                {
                                        act(AT_ACTION, "$n copies a message.",
                                            ch, NULL, NULL, TO_ROOM);
                                        obj_to_char(paper, ch);
                                }
                                else
                                        act(AT_ACTION,
                                            "$n removes a message.", ch, NULL,
                                            NULL, TO_ROOM);
                                return;
                        }
                }

                send_to_char("No such message.\n\r", ch);
                return;
        }

        send_to_char("Huh?  Type 'help note' for usage.\n\r", ch);
        return;
}

BOARD_DATA *read_board(char *boardfile, FILE * fp)
{
        BOARD_DATA *board;
        const char *word;
        bool      fMatch;
        char      letter;

        boardfile = NULL;

        do
        {
                letter = getc(fp);
                if (feof(fp))
                {
                        FCLOSE(fp);
                        return NULL;
                }
        }
        while (isspace(letter));
        ungetc(letter, fp);

        CREATE(board, BOARD_DATA, 1);

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
                        KEY("Extra_readers", board->extra_readers,
                            fread_string_nohash(fp));
                        KEY("Extra_removers", board->extra_removers,
                            fread_string_nohash(fp));
                        if (!str_cmp(word, "End"))
                        {
                                board->num_posts = 0;
                                board->first_note = NULL;
                                board->last_note = NULL;
                                board->next = NULL;
                                board->prev = NULL;
                                if (!board->read_group)
                                        board->read_group = str_dup("");
                                if (!board->post_group)
                                        board->post_group = str_dup("");
                                if (!board->extra_readers)
                                        board->extra_readers = str_dup("");
                                if (!board->extra_removers)
                                        board->extra_removers = str_dup("");
                                if (!board->board_name)
                                        board->board_name =
                                                str_dup(board->note_file);

                                return board;
                        }
                        break;

                case 'F':
                        KEY("Filename", board->note_file,
                            fread_string_nohash(fp));
                        break;

                case 'N':
                        KEY("Name", board->board_name,
                            fread_string_nohash(fp));
                        break;

                case 'M':
                        KEY("Min_read_level", board->min_read_level,
                            fread_number(fp));
                        KEY("Min_post_level", board->min_post_level,
                            fread_number(fp));
                        KEY("Min_remove_level", board->min_remove_level,
                            fread_number(fp));
                        KEY("Max_posts", board->max_posts, fread_number(fp));
                        break;

                case 'P':
                        KEY("Post_group", board->post_group,
                            fread_string_nohash(fp));
                        break;

                case 'R':
                        KEY("Read_group", board->read_group,
                            fread_string_nohash(fp));
                        break;

                case 'T':
                        KEY("Type", board->type, fread_number(fp));
                        break;
                case 'V':
                        KEY("Vnum", board->board_obj, fread_number(fp));
                        break;

                }
                if (!fMatch)
                {
                        bug("read_board: no match: %s", word);
                }
        }

        return board;
}

NOTE_DATA *read_note(char *notefile, FILE * fp)
{
        NOTE_DATA *pnote;
        char     *word;

        notefile = NULL;

        for (;;)
        {
                char      letter;

                do
                {
                        letter = getc(fp);
                        if (feof(fp))
                        {
                                FCLOSE(fp);
                                return NULL;
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

                word = fread_word(fp);
                if (!str_cmp(word, "voting"))
                {
                        pnote->voting = fread_number(fp);

                        if (str_cmp(fread_word(fp), "yesvotes"))
                                break;
                        pnote->yesvotes = fread_string_nohash(fp);

                        if (str_cmp(fread_word(fp), "novotes"))
                                break;
                        pnote->novotes = fread_string_nohash(fp);

                        if (str_cmp(fread_word(fp), "abstentions"))
                                break;
                        pnote->abstentions = fread_string_nohash(fp);

                        word = fread_word(fp);
                }

                if (str_cmp(word, "text"))
                        break;
                pnote->text = fread_string(fp);

                if (!pnote->yesvotes)
                        pnote->yesvotes = str_dup("");
                if (!pnote->novotes)
                        pnote->novotes = str_dup("");
                if (!pnote->abstentions)
                        pnote->abstentions = str_dup("");
                pnote->next = NULL;
                pnote->prev = NULL;
                return pnote;
        }

        bug("read_note: bad key word.", 0);
        exit(1);
}

/*
 * Load boards file.
 */
void load_boards(void)
{
        FILE     *board_fp;
        FILE     *note_fp;
        BOARD_DATA *board;
        NOTE_DATA *pnote;
        char      boardfile[256];
        char      notefile[256];

        first_board = NULL;
        last_board = NULL;

        snprintf(boardfile, MSL, "%s%s", BOARD_DIR, BOARD_FILE);
        if ((board_fp = fopen(boardfile, "r")) == NULL)
                return;

        while ((board = read_board(boardfile, board_fp)) != NULL)
        {
                LINK(board, first_board, last_board, next, prev);
                snprintf(notefile, MSL, "%s%s", BOARD_DIR, board->note_file);
                boot_log(notefile);
                if ((note_fp = fopen(notefile, "r")) != NULL)
                {
                        while ((pnote = read_note(notefile, note_fp)) != NULL)
                        {
                                LINK(pnote, board->first_note,
                                     board->last_note, next, prev);
                                board->num_posts++;
                        }
                }
        }
        return;
}

CMDF do_makeboard(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;
        char      arg[MAX_INPUT_LENGTH];

        /*
         * To prevent spaces in filenames 
         */
        argument = one_argument(argument, arg);
        if (arg[0] == '\0' || argument[0] != '\0')
        {
                send_to_char("Usage: makeboard <filename>\n\r", ch);
                return;
        }

        smash_tilde(arg);

        CREATE(board, BOARD_DATA, 1);

        LINK(board, first_board, last_board, next, prev);
        board->note_file = str_dup(strlower(arg));
        board->board_name = str_dup(strlower(arg));
        board->read_group = str_dup("");
        board->post_group = str_dup("");
        board->extra_readers = str_dup("");
        board->extra_removers = str_dup("");
}

CMDF do_bset(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;
        bool      found;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        int       value;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Usage: bset <board filename> <field> value\n\r",
                             ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char
                        ("  vnum read post remove maxpost filename type\n\r",
                         ch);
                send_to_char
                        ("  read_group post_group extra_readers extra_removers\n\r",
                         ch);
                send_to_char("  name delete\n\r", ch);
                return;
        }

        value = atoi(argument);
        found = FALSE;
        for (board = first_board; board; board = board->next)
                if (!str_cmp(arg1, board->note_file))
                {
                        found = TRUE;
                        break;
                }
        if (!found)
        {
                send_to_char("Board not found.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "delete"))
        {
                free_board(board);
        }
        else if (!str_cmp(arg2, "vnum"))
        {
                if (!get_obj_index(value))
                {
                        send_to_char("No such object.\n\r", ch);
                        return;
                }
                board->board_obj = value;
        }

        else if (!str_cmp(arg2, "read"))
        {
                if (value < 0 || value > MAX_LEVEL)
                {
                        send_to_char("Value out of range.\n\r", ch);
                        return;
                }
                board->min_read_level = value;
        }
        else if (!str_cmp(arg2, "read_group"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No group specified.\n\r", ch);
                        return;
                }
                DISPOSE(board->read_group);
                if (!str_cmp(argument, "none"))
                        board->read_group = str_dup("");
                else
                        board->read_group = str_dup(argument);
        }

        else if (!str_cmp(arg2, "post_group"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No group specified.\n\r", ch);
                        return;
                }
                DISPOSE(board->post_group);
                if (!str_cmp(argument, "none"))
                        board->post_group = str_dup("");
                else
                        board->post_group = str_dup(argument);
        }
        else if (!str_cmp(arg2, "extra_removers"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No names specified.\n\r", ch);
                        return;
                }
                if (!str_cmp(argument, "none"))
                        buf[0] = '\0';
                else
                        snprintf(buf, MSL, "%s %s", board->extra_removers,
                                 argument);
                DISPOSE(board->extra_removers);
                board->extra_removers = str_dup(buf);
        }

        else if (!str_cmp(arg2, "extra_readers"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No names specified.\n\r", ch);
                        return;
                }
                if (!str_cmp(argument, "none"))
                        buf[0] = '\0';
                else
                        snprintf(buf, MSL, "%s %s", board->extra_readers,
                                 argument);
                DISPOSE(board->extra_readers);
                board->extra_readers = str_dup(buf);
        }

        else if (!str_cmp(arg2, "filename"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No filename specified.\n\r", ch);
                        return;
                }
                DISPOSE(board->note_file);
                board->note_file = str_dup(argument);
        }
        else if (!str_cmp(arg2, "name"))
        {
                if (!argument || argument[0] == '\0')
                {
                        send_to_char("No Name specified.\n\r", ch);
                        return;
                }
                DISPOSE(board->board_name);
                board->board_name = str_dup(argument);
        }

        else if (!str_cmp(arg2, "post"))
        {
                if (value < 0 || value > MAX_LEVEL)
                {
                        send_to_char("Value out of range.\n\r", ch);
                        return;
                }
                board->min_post_level = value;
        }

        else if (!str_cmp(arg2, "remove"))
        {
                if (value < 0 || value > MAX_LEVEL)
                {
                        send_to_char("Value out of range.\n\r", ch);
                        return;
                }
                board->min_remove_level = value;
        }

        else if (!str_cmp(arg2, "maxpost"))
        {
                if (value < 1 || value > 1000)
                {
                        send_to_char("Value out of range.\n\r", ch);
                        return;
                }
                board->max_posts = value;
        }

        else if (!str_cmp(arg2, "type"))
        {
                if (!is_number(argument))
                        value = get_boardtypes(argument);
                if (value < 0 || value > BOARD_MAX)
                {
                        send_to_char("Value out of range.\n\r", ch);
                        return;
                }
                board->type = value;
        }
        else
        {
                send_to_char("What?\n\r", ch);
                do_bset(ch, "");
                return;
        }

        write_boards_txt();
        send_to_char("Done.\n\r", ch);
        return;
}


CMDF do_bstat(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;
        char      arg[MAX_INPUT_LENGTH];

        set_char_color(AT_NOTE, ch);
        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Usage: bstat <board filename>\n\r", ch);
                return;
        }

        for (board = first_board; board; board = board->next)
                if (!str_prefix(arg, board->note_file))
                        break;

        if (!board)
        {
                send_to_char("Board not found.\n\r", ch);
                return;
        }

        set_char_color(AT_NOTE, ch);
        ch_printf(ch, "Name:            \t%s\n\r"
                  "Filename:        \t%s\n\r"
                  "\tVnum:           %d\n\r"
                  "\tRead:           %d\n\r"
                  "\tPost:           %d\n\r"
                  "\tRmv:            %d\n\r"
                  "\tMax:            %d\n\r"
                  "\tPosts:          %d\n\r"
                  "\tType:           %s\n\r"
                  "\tRead_group:     %s\n\r"
                  "\tPost_group:     %s\n\r"
                  "\tExtra_readers:  %s\n\r",
                  board->board_name, board->note_file,
                  board->board_obj, board->min_read_level,
                  board->min_post_level, board->min_remove_level,
                  board->max_posts, board->num_posts,
                  board_types[board->type], board->read_group,
                  board->post_group, board->extra_readers);

        return;
}


CMDF do_boards(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;

        argument = NULL;

        if (!first_board)
        {
                send_to_char("There are no boards.\n\r", ch);
                return;
        }

        set_char_color(AT_NOTE, ch);
        for (board = first_board; board; board = board->next)
                ch_printf(ch,
                          "%-16s Vnum: %5d Read: %3d Post: %3d Rmv: %3d Max: %3d Posts: %3d Type: %s\n\r",
                          board->note_file, board->board_obj,
                          board->min_read_level, board->min_post_level,
                          board->min_remove_level, board->max_posts,
                          board->num_posts, board_types[board->type]);
}

void mail_count(CHAR_DATA * ch)
{
        BOARD_DATA *board;
        NOTE_DATA *note;
        int       cnt = 0;

        for (board = first_board; board; board = board->next)
                if (board->type == BOARD_MAIL && can_read(ch, board))
                        for (note = board->first_note; note;
                             note = note->next)
                                if (is_note_to_def(ch, note))
                                        ++cnt;
        if (cnt)
                ch_printf(ch, "You have %d mail messages waiting.\n\r", cnt);
        return;
}

CMDF do_global(CHAR_DATA * ch, char *argument)
{
        BOARD_DATA *board;
        char      arg1[MAX_INPUT_LENGTH];

        if (!str_cmp(argument, "write") ||
            !str_cmp(argument, "show") ||
            !str_cmp(argument, "to") ||
            !str_cmp(argument, "subject") || ch->substate == SUB_WRITING_NOTE)
        {
                note(ch, argument, NULL);
                return;
        }

        set_char_color(AT_NOTE, ch);
        if (!first_board)
        {
                send_to_char("There are no boards.\n\r", ch);
                return;
        }

        if (ch->substate == SUB_WRITING_NOTE)
        {
                note(ch, "", first_board);
                return;
        }

        if (argument[0] == '\0')
        {
                int       count = 0;

                set_char_color(AT_PLAIN, ch);
                send_to_char("Syntax: global <board> <command>\n\r", ch);
                send_to_char("Choices:\n\r", ch);
                set_char_color(AT_NOTE, ch);
                for (board = first_board; board; board = board->next)
                {
                        if (board->type != BOARD_GLOBAL)
                                continue;
                        ch_printf(ch, "\t %s\n\r", board->board_name);
                        count++;
                }

                if (count == 0)
                        send_to_char("No boards\n\r", ch);
                else
                        ch_printf(ch, "&G%d&w board%c total.\n\r", count,
                                  count == 1 ? ' ' : 's');
                return;
        }

        argument = one_argument(argument, arg1);
        /*
         */
        for (board = first_board; board; board = board->next)
        {
                if (nifty_is_name_prefix(arg1, board->board_name))
                        break;
        }

        if (!board)
        {
                send_to_char("Board not found.\n\r", ch);
                return;
        }

        note(ch, argument, board);
}

void free_board(BOARD_DATA * board)
{
        NOTE_DATA *pnote, *note_next;

        UNLINK(board, first_board, last_board, next, prev);
        DISPOSE(board->note_file);
        DISPOSE(board->board_name);
        DISPOSE(board->read_group);
        DISPOSE(board->post_group);
        DISPOSE(board->extra_readers);
        DISPOSE(board->extra_removers);
        for (pnote = board->first_note; pnote; pnote = note_next)
        {
                note_next = pnote->next;
                free_note(pnote);
        }
}
