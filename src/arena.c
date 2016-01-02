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
 * This module is for our own setup of an ooc and ic arena system. Betting and non deaths*
 * are included.                                                                         *
 *****************************************************************************************
 *                $Id: arena.c 1330 2005-12-05 03:23:24Z halkeye $                  *
 *****************************************************************************************/

#include <string.h>
#include "mud.h"

/* [22:16] greven145: k, basically, here is what I'm thinking: a global arena data,
like auction is, with fields for challenger, challenged, accepted, time_to_battle 
(how long is left for people to place bets), battle_length (how long it has lasted
for, pot, ummm... first_challenger_better, first_challenged_better? */

typedef struct arena_data ARENA_DATA;
typedef struct bet_data BET_DATA;

struct arena_data
{
        CHAR_DATA *challenger;
        CHAR_DATA *challenged;
        bool      accepted;
        bool      fighting;
        bool      ooc;
        int       pot;
        /*
         * how long is left for people to place bets 
         */
        int       time_to_battle;   /* Or how long until the challenge is canceled out */
        int       battle_length;    /* how long it has lasted for */
        int       bet_challenged;
        int       bet_challenger;
        CHAR_DATA *announcer;
        BET_DATA *first_better;
        BET_DATA *last_better;
};


struct bet_data
{
        BET_DATA *next;
        BET_DATA *prev;
        CHAR_DATA *better;
        CHAR_DATA *bet_on;
        int       amount;
};

#define ARENA_TIMEOUT           10
#define ARENA_BETTING_ROUND     25
ARENA_DATA *arena = NULL;

#define BET_CHALLENGER  0
#define BET_CHALLENGED  1

void free_arena args((void));
CMDF      do_tell(CHAR_DATA * ch, char *argument);

bool is_area_inprogress(void)
{
        if (arena)
                return TRUE;
        return FALSE;
}

void create_arena(void)
{
        MOB_INDEX_DATA *pMobIndex;

        CREATE(arena, ARENA_DATA, 1);

        if ((pMobIndex = get_mob_index(1)) == NULL)
        {
                bug("No mobile has a vnum of 1");
                DISPOSE(arena);
                return;
        }

        arena->announcer = create_mobile(pMobIndex);
        REMOVE_BIT(arena->announcer->act, ACT_PROTOTYPE);
        SET_BIT(arena->announcer->act, ACT_SPEAKSALL);
        arena->announcer->speaking = get_language("basic");
        STRFREE(arena->announcer->short_descr);
        char_to_room(arena->announcer, get_room_index(1));
        arena->announcer->short_descr = STRALLOC("Arena Announcer");
        REMOVE_BIT(arena->announcer->act, ACT_SECRETIVE);
        arena->ooc = FALSE;
        arena->pot = 0;
        arena->time_to_battle = -1;
        arena->battle_length = -1;
}

bool arena_can_fight(CHAR_DATA * ch, CHAR_DATA * victim)
{
        victim = NULL;
        if (xIS_SET(ch->in_room->room_flags, ROOM_ARENA))
        {
                if (arena && arena->fighting)
                        return TRUE;
                return FALSE;
        }
        return TRUE;
}

void add_bet(CHAR_DATA * better, int amount, CHAR_DATA * who)
{
        BET_DATA *bet;

        if (IS_NPC(better))
                return;
        if (!arena)
                return;

        for (bet = arena->first_better; bet; bet = bet->next)
        {
                if (bet->better == better)
                        break;
        }

        if (!bet)
        {
                CREATE(bet, BET_DATA, 1);
                LINK(bet, arena->first_better, arena->last_better, next,
                     prev);
                bet->amount = 0;
        }

        bet->bet_on = who;
        bet->better = better;
        bet->amount += amount;
        better->gold -= amount;
        arena->pot += amount;
}

/* for do quit */
void remove_better(CHAR_DATA * better)
{
        BET_DATA *bet, *bet_next;

        if (!arena)
                return;

        if (better == arena->challenger)
        {
                command_printf(arena->challenger, "achat %s",
                               "The Challenger has quit");
                char_from_room(arena->challenger);
                char_to_room(arena->challenger,
                             get_room_index(arena->challenger->retran));
                do_look(arena->challenger, "");
                free_arena();
                return;
        }
        if (better == arena->challenged)
        {
                command_printf(arena->challenger, "achat %s",
                               "The Challenged has quit");
                char_from_room(arena->challenged);
                char_to_room(arena->challenged,
                             get_room_index(arena->challenged->retran));
                do_look(arena->challenged, "");
                free_arena();
                return;
        }

        for (bet = arena->first_better; bet; bet = bet_next)
        {
                bet_next = bet->next;
                if (bet->better == better)
                {
                        UNLINK(bet, arena->first_better, arena->last_better,
                               next, prev);
                        DISPOSE(bet);
                        return;
                }
        }
}

void win_fight(CHAR_DATA * winner, CHAR_DATA * looser)
{
        BET_DATA *bet, *next_bet = NULL;
        char      buf[MIL];
        int       amountbet, bettercount = 0;

        if (!arena)
                return;
        if (winner != arena->challenger && winner != arena->challenged)
        {
                bug("Winner(%s) is not Challenger(%s) or Challenged(%s)",
                    winner->name, arena->challenger->name,
                    arena->challenged->name);
                free_arena();
                return;
        }

        arena->challenger->hit = arena->challenger->max_hit;
        arena->challenged->hit = arena->challenged->max_hit;
        char_from_room(arena->challenged);
        char_to_room(arena->challenged,
                     get_room_index(arena->challenged->retran));
        do_look(arena->challenged, "");
        char_from_room(arena->challenger);
        char_to_room(arena->challenger,
                     get_room_index(arena->challenger->retran));
        do_look(arena->challenger, "");
        update_pos(arena->challenged);
        update_pos(arena->challenger);
        act(AT_PLAIN, "You have been defeated in combat by $N", looser, NULL,
            winner, TO_CHAR);
        act(AT_PLAIN, "You have defeated $n", looser, NULL, winner, TO_VICT);

        snprintf(buf, MIL, "%s has defeated %s in combat",
                 winner->pcdata->full_name, looser->pcdata->full_name);
        command_printf(arena->challenger, "achat %s", buf);;

        /*
         * Figure out winners, remove all winner cash from pot 
         */

        for (bet = arena->first_better; bet; bet = next_bet)
                if (bet->bet_on == winner)
                {
                        bettercount++;
                        arena->pot -= bet->amount;
                }

        if (bettercount == 0)
                bettercount = 1;
        for (bet = arena->first_better; bet; bet = next_bet)
        {
                next_bet = bet->next;
                amountbet = 0;

                /*
                 * Do Stuff - FIXME Grev 
                 */
                /*
                 * Ok, lets see: winner gets all money bet on loser, and winning
                 * betters get % of the pot their original bet 
                 */
                if (bet->bet_on == winner)
                {
                        snprintf(buf, MIL, "%s Your bet on %s paid off.",
                                 bet->better->name,
                                 bet->better->pcdata->full_name);
                        do_tell(arena->announcer, buf);
                        snprintf(buf, MIL, "%s You get %d credits.",
                                 bet->better->name,
                                 (bet->amount +
                                  ((arena->pot / 2) / bettercount)));
                        do_tell(arena->announcer, buf);
                        bet->better->gold +=
                                (bet->amount +
                                 ((arena->pot / 2) / bettercount));
                }
                else if (bet->bet_on == looser)
                {
                        snprintf(buf, MIL,
                                 "%s You have lost your bet on the recent battle.",
                                 bet->better->name);
                        do_tell(arena->announcer, buf);
                        snprintf(buf, MIL, "%s You lose %d credits.",
                                 bet->better->name, bet->amount);
                        do_tell(arena->announcer, buf);
                }
                UNLINK(bet, arena->first_better, arena->last_better, next,
                       prev);
                DISPOSE(bet);
        }
        /*
         * Reward winner - Ok, all bets are dealt with, we have total to give to winner, lets do so.
         */

        snprintf(buf, MIL, "%s Your victory over %s has been rewarded!.",
                 winner->name, looser->pcdata->full_name);
        do_tell(arena->announcer, buf);
        snprintf(buf, MIL, "%s You get %d credits.", winner->name,
                 arena->pot / 2);
        do_tell(arena->announcer, buf);
        winner->gold += arena->pot / 2;

        snprintf(buf, MIL, "%s Your loss against %s has gained you nothing!",
                 looser->name, winner->pcdata->full_name);
        do_tell(arena->announcer, buf);

        /*
         * We are now Done 
         */
        free_arena();
}

void free_arena(void)
{
        BET_DATA *bet, *bet_next;

        for (bet = arena->first_better; bet; bet = bet_next)
        {
                bet_next = bet->next;
                UNLINK(bet, arena->first_better, arena->last_better, next,
                       prev);
                DISPOSE(bet);
        }
        extract_char(arena->announcer, TRUE);
        DISPOSE(arena);
}

CMDF do_bet(CHAR_DATA * ch, char *argument)
{
        char      arg[MIL];
        int       amount;
        CHAR_DATA *victim = NULL;

        if (IS_NPC(ch))
                return;

        set_char_color(AT_PLAIN, ch);
        if (arena == NULL || arena->accepted == FALSE)
        {
                send_to_char
                        ("No challenge going on right now for you to bet on.",
                         ch);
                return;
        }

        if (ch == arena->challenger || ch == arena->challenged)
        {
                send_to_char("You are not allowed to bet.", ch);
                return;
        }

        argument = one_argument(argument, arg);
        if (argument[0] == '\0' || arg[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gbet &C<&cwinner&C> &C<&camt&C>&w\n\r",
                         ch);
                ch_printf(ch,
                          "&RBets are being accepted for the next &C%d&R rounds",
                          arena->time_to_battle);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They don't exist.", ch);
                return;
        }

        /*
         * No need to check if npc, 'cause challenge does that 
         */
        if (victim != arena->challenger && victim != arena->challenged)
        {
                send_to_char("You can't bet on them.", ch);
                return;
        }

        amount = atoi(argument);
        amount = UMAX(0, amount);

        if (amount == 0)
        {
                send_to_char("Nice try.", ch);
                return;
        }

        if ((ch->gold - amount) < 0)
        {
                send_to_char("You don't have enough money for that.", ch);
                return;
        }

        if (arena->ooc == TRUE)
        {
                send_to_char("You cannot bet on an ooc battle!", ch);
                return;
        }

        add_bet(ch, amount, victim);
        ch_printf(ch, "You place a %d credit bet on %s.\n\r", amount,
                  victim->pcdata->full_name);
        return;
}

CMDF do_bets(CHAR_DATA * ch, char *argument)
{
        BET_DATA *bet;

        argument = NULL;

        set_char_color(AT_PLAIN, ch);
        if (arena == NULL || arena->accepted == FALSE || !arena->first_better)
        {
                send_to_char("No bets on right now.", ch);
                return;
        }

        set_pager_color(AT_PLAIN, ch);
        for (bet = arena->first_better; bet; bet = bet->next)
        {
                pager_printf(ch, "&G%s&w bet &G%d&w on &G%s&w\n\r",
                             bet->better->pcdata->full_name,
                             bet->amount, bet->bet_on->pcdata->full_name);
        }
        return;
}


CMDF do_challenge(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        char      arg[MSL];

        argument = one_argument(argument, arg);

        set_char_color(AT_PLAIN, ch);

        if (arena && arena->accepted)
        {
                send_to_char("Fight already in progress.", ch);
                return;
        }

        if (arg[0] == '\0')
        {
                send_to_char("&RSyntax: &Gchallenge &C<&cvictim&C>&w\n\r"
                             "        &Gchallenge &C<&caccept/decline&C>&w\n\r",
                             ch);
                return;
        }

        if (!str_cmp("accept", arg))
        {
                if (!arena || arena->challenged != ch)
                {
                        send_to_char("You havn't been challenged.", ch);
                        return;
                }
                arena->accepted = TRUE;
                if (IS_IMMORTAL(arena->challenger)
                    || IS_IMMORTAL(arena->challenged))
                        arena->ooc = TRUE;
                /*
                 * If its an ooc battle, no reason to wait that long for bets, but should let people prepare 
                 */
                if (arena->ooc == TRUE)
                        arena->time_to_battle = ARENA_TIMEOUT;
                else
                        arena->time_to_battle = ARENA_BETTING_ROUND;
                command_printf(ch, "achat %s", "I accept");

                /*
                 * transfer to arena, hardcoded, or planetary set, or whatever 
                 */

                arena->challenger->retran = arena->challenger->in_room->vnum;
                arena->challenger->regoto = arena->challenger->in_room->vnum;
                act(AT_PLAIN, "$n is shipped off to the arena.",
                    arena->challenger, NULL, NULL, TO_ROOM);
                char_from_room(arena->challenger);
                char_to_room(arena->challenger, get_room_index(29));
                do_look(arena->challenger, "");

                act(AT_PLAIN, "$n is shipped off to the arena.",
                    arena->challenged, NULL, NULL, TO_ROOM);
                arena->challenged->regoto = arena->challenged->in_room->vnum;
                arena->challenged->retran = arena->challenged->in_room->vnum;
                char_from_room(arena->challenged);
                char_to_room(arena->challenged, get_room_index(40));
                do_look(arena->challenged, "");
                if (arena->ooc == TRUE)
                        command_printf(arena->announcer, "achat %s",
                                       "Betting is suspended for OOC battles");
                else
                        command_printf(ch, "achat %s", "Betting now Open");
                return;
        }
        else if (!str_cmp("decline", arg) || !str_cmp("refuse", arg))
        {
                if (!arena || arena->challenged != ch)
                {
                        send_to_char("You havn't been challenged.", ch);
                        return;
                }
                command_printf(ch, "achat %s", "NO THANKS!");
                free_arena();
                return;
        }
        else if (arena && arena->challenger)
        {
                send_to_char
                        ("A challenge has already been made. Wait for it to be accepted or canceled",
                         ch);
                return;
        }
        else if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They are not here.", ch);
                return;
        }
        else if (victim == ch)
        {
                send_to_char
                        ("You cann't challenge yourself to full combat!\n\r",
                         ch);
                return;
        }
        else if (victim)
        {
                char      buf[150];

                if (IS_NPC(victim))
                {
                        send_to_char("Not on NPCs.\n\r", ch);
                        return;
                }
                if (IS_SET(victim->act, PLR_AFK))
                {
                        send_to_char("Not on afk players.\n\r", ch);
                        return;
                }
                create_arena();
                if (!arena)
                {
                        bug("we have a problem with arenas");
                        return;
                }
                if (argument && argument[0] != '\0'
                    && !str_cmp(argument, "ooc"))
                        arena->ooc = TRUE;
                snprintf(buf, 150, "I CHOOSE YOU %s%s",
                         strupper(victim->pcdata->full_name),
                         arena->ooc ? " FOR AN OOC BATTLE" : "");
                arena->challenger = ch;
                arena->challenged = victim;
                arena->time_to_battle = ARENA_TIMEOUT;
                command_printf(ch, "achat %s", buf);
                return;
        }
        else
        {
                do_challenge(ch, "");
                return;
        }
}

void arena_update(void)
{
        if (!arena)
                return;


        if (arena->time_to_battle == -1)
        {
                arena->battle_length++;
                return;
        }

        if (--arena->time_to_battle <= 0)
        {
                if (!arena->accepted)
                {
                        command_printf(arena->announcer, "achat %s",
                                       "Challenge Timed out");
                        free_arena();
                        return;
                }

                if (arena->ooc == TRUE)
                        command_printf(arena->announcer, "achat %s",
                                       "Let the battle begin");
                else
                        command_printf(arena->announcer, "achat %s",
                                       "Betting Now Closed. Let the battle begin");
                arena->time_to_battle = -1;
                arena->fighting = TRUE;
                return;
        }
}

CMDF do_arena(CHAR_DATA * ch, char *argument)
{
        if (!argument || argument[0] == '\0')
        {
                send_to_char("Current Arena Status\n\r", ch);

                if (!arena)
                {
                        send_to_char
                                ("There is no current arena event taking place\n\r",
                                 ch);
                        return;
                }


                ch_printf(ch, "&BC&zhallenger:    &w %s\n\r",
                          arena->challenger ? arena->challenger->
                          name : "None");
                ch_printf(ch, "&BC&zhallenged:    &w %s\n\r",
                          arena->challenged ? arena->challenged->
                          name : "None");
                ch_printf(ch, "&BA&zccepted?:     &w %s\n\r",
                          arena->accepted ? "Yes" : "No");
                ch_printf(ch, "&BF&zighting       &w %s\n\r",
                          arena->fighting ? "Yes" : "No");
                ch_printf(ch, "&BO&zoc            &w %s\n\r",
                          arena->ooc ? "Yes" : "No");
                ch_printf(ch, "&BB&zattle Length: &w %d\n\r",
                          arena->battle_length);
                ch_printf(ch, "&BB&zet Challenged:&w %d\n\r",
                          arena->bet_challenged);
                ch_printf(ch, "&BB&zet Challenger:&w %d\n\r",
                          arena->bet_challenger);
                ch_printf(ch, "&BT&zime to battle:&w %d\n\r",
                          arena->time_to_battle);

                send_to_char("Valid arguments: stop\n\r", ch);
        }


        if (!str_cmp(argument, "stop"))
        {
                BET_DATA *bet, *bet_next;
                char      buf[MSL];

                if (!arena)
                {
                        send_to_char
                                ("There is no current arena event taking place\n\r",
                                 ch);
                        return;
                }

                for (bet = arena->first_better; bet; bet = bet_next)
                {
                        bet_next = bet->next;
                        snprintf(buf, MIL,
                                 "%s Your bet of %d on %s has been refunded.",
                                 bet->better->name, bet->amount,
                                 bet->bet_on->name);
                        do_tell(arena->announcer, buf);
                        bet->better->gold += bet->amount;
                        UNLINK(bet, arena->first_better, arena->last_better,
                               next, prev);
                        DISPOSE(bet);
                }
                command_printf(arena->announcer, "achat %s",
                               "The current battle has been called off!");
                if (arena->challenger)
                {
                        arena->challenger->hit = arena->challenger->max_hit;
						if (arena->accepted)
						{
                            char_from_room(arena->challenger);
                            char_to_room(arena->challenger,
                                         get_room_index(arena->challenger->
                                                        retran));
                            do_look(arena->challenger, "");
						}
                        update_pos(arena->challenger);

                }

                if (arena->challenged)
                {
                        arena->challenged->hit = arena->challenged->max_hit;
						if (arena->accepted)
						{
    						char_from_room(arena->challenged);
                            char_to_room(arena->challenged,
                                         get_room_index(arena->challenged->
                                                        retran));
                            do_look(arena->challenged, "");
						}
                        update_pos(arena->challenged);
                }
                free_arena();
        }
}

bool in_arena(CHAR_DATA * ch)
{

        if (!str_cmp(ch->in_room->area->filename, "arena.are")
            || xIS_SET(ch->in_room->room_flags, ROOM_ARENA))
                return TRUE;

        if (ch->in_room->vnum < 29 || ch->in_room->vnum > 43)
                return FALSE;

        return TRUE;
}

