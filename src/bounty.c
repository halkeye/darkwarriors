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
 *		            Bounty Hunter Module    			   *   
 *                    (  and area capturing as well  )                      * 
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mud.h"
#include "bounty.h"
#include "olc_bounty.h"


BOUNTY_DATA *first_bounty;
BOUNTY_DATA *last_bounty;
BOUNTY_DATA *first_disintigration;
BOUNTY_DATA *last_disintigration;

#define BOUNTY_VERSION 1

void disintigration args((CHAR_DATA * ch, CHAR_DATA * victim, long amount));
void nodisintigration args((CHAR_DATA * ch, CHAR_DATA * victim, long amount));
int       xp_compute(CHAR_DATA * ch, CHAR_DATA * victim);

bool char_exists(char *player)
{
        char      fname[MSL];
        struct stat fst;

        if (!player)
                return FALSE;
        snprintf(fname, MSL, "%s%c/%s", PLAYER_DIR, tolower(player[0]),
                 capitalize(player));
        return (stat(fname, &fst) != -1);
}

void save_disintigrations()
{
        BOUNTY_DATA *tbounty;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open disintigration.lst for writing!\n\r",
                    0);
                return;
        }

        fprintf(fpout, "#VERSION %d\n", BOUNTY_VERSION);
        for (tbounty = first_disintigration; tbounty; tbounty = tbounty->next)
        {
                fprintf(fpout, "#BOUNTY\n");
                fprintf(fpout, "Target %s~\n", tbounty->target);
                fprintf(fpout, "Type %d\n", tbounty->type);
                fprintf(fpout, "Amount %ld\n", tbounty->amount);
                if (tbounty->type == BOUNTY_POLICE)
                        fprintf(fpout, "Source %s~\n", tbounty->source);
                fprintf(fpout, "End\n\n");
        }
        fprintf(fpout, "#END\n");
        FCLOSE(fpout);

}

BOUNTY_DATA *get_disintigration(char *target)
{
        BOUNTY_DATA *bounty;

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (!str_cmp(target, bounty->target))
                        return bounty;
        return NULL;
}

void fread_bounty(FILE * fp, int version)
{
        const char *word;
        bool      fMatch;
        BOUNTY_DATA *bounty = NULL;

        CREATE(bounty, BOUNTY_DATA, 1);

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

                case 'A':
                        KEY("Amount", bounty->amount, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!bounty->target
                                    || !char_exists(bounty->target)
                                    || (bounty->type == BOUNTY_POLICE
                                        && (bounty->source == NULL
                                            || !get_clan(bounty->source))))
                                {
                                        if (bounty->target)
                                                STRFREE(bounty->target);

                                        if (bounty->source != NULL)
                                                STRFREE(bounty->source);

                                        DISPOSE(bounty);
                                }
                                else
                                {
                                        LINK(bounty, first_disintigration,
                                             last_disintigration, next, prev);

                                        if (bounty->source == NULL)
                                                bounty->source = STRALLOC("");
                                }

                                return;
                        }
                        break;
                case 'S':
                        KEY("Source", bounty->source, fread_string(fp));
                        break;

                case 'T':
                        KEY("Target", bounty->target, fread_string(fp));
                        KEY("Type", bounty->type, fread_number(fp));
                        break;
                }
        }
        version = 0;
}

void load_bounties()
{
        FILE     *fpList;
        const char *target;
        char      bountylist[256];
        BOUNTY_DATA *bounty;
        long int  amount;
        int       version = 0;
        char      letter;

        first_disintigration = NULL;
        last_disintigration = NULL;

        snprintf(bountylist, MSL, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(bountylist, "r")) == NULL)
        {
                perror(bountylist);
                exit(1);
        }


        letter = fread_letter(fpList);
        fseek(fpList, 0, 0);
        if (letter != '#')
        {
                for (;;)
                {
                        target = feof(fpList) ? "$" : fread_word(fpList);
                        if (target[0] == '$')
                                break;
                        bounty = get_disintigration((char *) target);
                        if (!bounty)
                        {
                                CREATE(bounty, BOUNTY_DATA, 1);
                                LINK(bounty, first_disintigration,
                                     last_disintigration, next, prev);
                                bounty->target = STRALLOC((char *) target);
                                bounty->amount = 0;
                        }
                        amount = fread_number(fpList);
                        bounty->amount += amount;
                        bounty->type = BOUNTY_PLAYER;
                }
        }
        else
        {
                char     *word = NULL;

                for (;;)
                {
                        fread_letter(fpList);
                        word = fread_word(fpList);
                        if (!str_cmp(word, "VERSION"))
                                version = fread_number(fpList);
                        else if (!str_cmp(word, "BOUNTY"))
                                fread_bounty(fpList, version);
                        else if (!str_cmp(word, "END"))
                                break;  /* Done */
                        else
                        {
                                bug("load_bounties: bad section: %s", word);
                                break;
                        }
                }
        }

        FCLOSE(fpList);
        boot_log(" Done bounties ");

        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_bounties(CHAR_DATA * ch, char *argument)
{
        BOUNTY_DATA *bounty;
        int       count = 0;

        argument = NULL;

        set_char_color(AT_WHITE, ch);
        send_to_char("\n\rBounty                      Amount\n\r", ch);
        if (!first_disintigration && !check_olc_bounties(ch->in_room))
        {
                set_char_color(AT_GREY, ch);
                send_to_char("There are no bounties set at this time.\n\r",
                             ch);
                return;
        }

        if (first_disintigration)
                for (bounty = first_disintigration; bounty;
                     bounty = bounty->next)
                {
                        set_char_color(AT_RED, ch);
                        if (bounty->type == BOUNTY_POLICE)
                                ch_printf(ch,
                                          "%-26s   %-14ld &B[&RJailing Only&B] &B[&RPolice&B]&w %s&w\n\r",
                                          bounty->target, bounty->amount,
                                          bounty->source);
                        else
                                ch_printf(ch, "%-26s   %-14ld\n\r",
                                          bounty->target, bounty->amount);
                        count++;
                }
        count += print_olc_bounties(ch);
        set_char_color(AT_GREY, ch);
        ch_printf(ch, "&BT&zhere are &W%d&z bounties at this time.\n\r",
                  count);

}

void disintigration(CHAR_DATA * ch, char *name, long amount)
{
        BOUNTY_DATA *bounty;
        bool      found;
        char      buf[MAX_STRING_LENGTH];

        found = FALSE;

        if (IS_NPC(ch))
        {
                send_to_char("You must be authorized to post a bounty.\n\r",
                             ch);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
        {
                if (bounty->type == BOUNTY_PLAYER
                    && !str_cmp(bounty->target, name))
                {
                        found = TRUE;
                        break;
                }
        }

        if (!found)
        {
                CREATE(bounty, BOUNTY_DATA, 1);
                LINK(bounty, first_disintigration, last_disintigration, next,
                     prev);

                bounty->target = STRALLOC(name);
                bounty->amount = 0;
        }

        bounty->type = BOUNTY_PLAYER;
        bounty->amount = bounty->amount + amount;
        save_disintigrations();

        snprintf(buf, MSL,
                 "%s has added %ld credits to the bounty on %s.",
                 ch->name, amount, name);
        log_string(buf);
        snprintf(buf, MSL, "%ld credits has been added to the bounty on %s.",
                 amount, name);
        echo_to_all(AT_RED, buf, 0);
}

CMDF do_addbounty(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];
        long int  amount;
        CHAR_DATA *victim;
        char      name[256];

        if (!argument || argument[0] == '\0')
        {
                do_bounties(ch, argument);
                return;
        }

        argument = one_argument(argument, arg);

        if (argument[0] == '\0')
        {
                send_to_char("Usage: Addbounty <target> <amount>\n\r", ch);
                return;
        }

/*        if (ch->pcdata && ch->pcdata->clan
            && !str_cmp(ch->pcdata->clan->name, "the hunters guild"))
        {
                send_to_char("Your job is to collect bounties not post them.",
                             ch);
                return;
        }*/

        if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BOUNTY))
        {
                send_to_char
                        ("You will have to go to your local Hunters Guild office to add a new bounty.",
                         ch);
                return;
        }

        if (argument[0] == '\0')
                amount = 0;
        else
                amount = atoi(argument);

        if (amount < 5000)
        {
                send_to_char("A bounty should be at least 5000 credits.\n\r",
                             ch);
                return;
        }

        if (!(victim = get_char_world(ch, arg)))
        {
                char      fname[1024];
                struct stat fst;

                arg[0] = UPPER(arg[0]);
                sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
                        capitalize(arg));

                if (stat(fname, &fst) != -1)
                {
                        strcpy(name, arg);
                }
                else
                {
                        send_to_char("No player exists by that name.\n\r",
                                     ch);
                        return;
                }
        }
        else
        {
                strcpy(name, victim->name);
                if (IS_NPC(victim))
                {
                        send_to_char
                                ("You can only set bounties on other players .. not mobs!\n\r",
                                 ch);
                        return;
                }
        }

        if (amount <= 0)
        {
                send_to_char
                        ("Nice try! How about 1 or more credits instead...\n\r",
                         ch);
                return;
        }

        if (ch->gold < amount)
        {
                send_to_char("You don't have that many credits!\n\r", ch);
                return;
        }

        ch->gold = ch->gold - amount;

        disintigration(ch, name, amount);

}

void remove_bounties(char *target)
{
        BOUNTY_DATA *bounty, *bt_next;

        for (bounty = first_disintigration; bounty; bounty = bt_next)
        {
                bt_next = bounty->next;
                if (!str_cmp(bounty->target, target))
                {
                        UNLINK(bounty, first_disintigration,
                               last_disintigration, next, prev);
                        STRFREE(bounty->target);
                        if (bounty->source)
                                STRFREE(bounty->source);
                        DISPOSE(bounty);
                }
        }

        save_disintigrations();
}

void remove_disintigration(BOUNTY_DATA * bounty)
{
        UNLINK(bounty, first_disintigration, last_disintigration, next, prev);
        STRFREE(bounty->target);
        if (bounty->source)
                STRFREE(bounty->source);
        DISPOSE(bounty);

        save_disintigrations();
}

void claim_disintigration(CHAR_DATA * ch, CHAR_DATA * victim)
{
        BOUNTY_DATA *bounty;
        long int  experience;
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(victim))
                return;

        if (IS_NPC(ch))
                return;

        bounty = get_disintigration(victim->name);

        if (ch == victim)
        {
                if (bounty != NULL)
                        remove_bounties(bounty->target);
                snprintf(buf, MSL, "%s is Dead!", victim->name);
                echo_to_all(AT_RED, buf, 0);
                return;
        }

        if (bounty
            && (!ch->pcdata || !ch->pcdata->clan
                || str_cmp(ch->pcdata->clan->name, "the hunters guild")))
        {
			    bug("%s is not in the hunters guild", ch->name);
                remove_bounties(bounty->target);
                bounty = NULL;
        }

        if (bounty == NULL || bounty->type == BOUNTY_POLICE)
        {
                if (IS_SET(victim->act, PLR_KILLER) && !IS_NPC(ch))
                {
                        experience =
                                URANGE(1, xp_compute(ch, victim),
                                       (exp_level
                                        (ch->skill_level[HUNTING_ABILITY] +
                                         1) -
                                        exp_level(ch->
                                                  skill_level
                                                  [HUNTING_ABILITY])));
						experience *= 5;
                        gain_exp(ch, experience, HUNTING_ABILITY);
                        set_char_color(AT_BLOOD, ch);
                        ch_printf(ch,
                                  "You receive %ld hunting experience for executing a wanted killer.\n\r",
                                  exp);
                }
                else if (!IS_NPC(ch)
                         && (!ch->pcdata || !ch->pcdata->clan
                             || str_cmp(ch->pcdata->clan->name,
                                        "the hunters guild")))
                {
                        SET_BIT(ch->act, PLR_KILLER);
                        ch_printf(ch,
                                  "You are now wanted for the murder of %s.\n\r",
                                  victim->name);
                }
                snprintf(buf, MSL, "%s is Dead!", victim->name);
                echo_to_all(AT_RED, buf, 0);
                if (bounty)
                        remove_bounties(bounty->target);
                return;
        }

        ch->gold += bounty->amount;

        experience =
                URANGE(1, bounty->amount + xp_compute(ch, victim),
                       (exp_level(ch->skill_level[HUNTING_ABILITY] + 1) -
                        exp_level(ch->skill_level[HUNTING_ABILITY])));
        gain_exp(ch, experience, HUNTING_ABILITY);

        experience *= 5;

        set_char_color(AT_BLOOD, ch);
        ch_printf(ch,
                  "You receive %ld bounty experience and %ld credits,\n\r from the bounty on %s\n\r",
                  experience, bounty->amount, bounty->target);

        snprintf(buf, MSL, "%s has claimed the disintigration bounty on %s!",
                 ch->name, victim->name);
        echo_to_all(AT_RED, buf, 0);
        snprintf(buf, MSL, "%s is Dead!", victim->name);
        echo_to_all(AT_RED, buf, 0);

        if (!IS_SET(victim->act, PLR_KILLER)
            && (!ch->pcdata || !ch->pcdata->clan
                || str_cmp(ch->pcdata->clan->name, "the hunters guild")))
                SET_BIT(ch->act, PLR_KILLER);
        remove_bounties(bounty->target);
}

void add_wanted(CHAR_DATA * ch, PLANET_DATA * planet)
{
        WANTED_DATA *wanted = NULL;

        if (planet == NULL || planet->governed_by == NULL)
                return;

        if (IS_NPC(ch))
                return;

        for (wanted = ch->pcdata->first_wanted; wanted; wanted = wanted->next)
        {
                if (wanted->government == planet->governed_by)
                        break;
        }

        if (!wanted)
        {
                CREATE(wanted, WANTED_DATA, 1);
                LINK(wanted, ch->pcdata->first_wanted,
                     ch->pcdata->last_wanted, next, prev);
                /*
                 * If we are now wanted 
                 */
                ch_printf(ch, "&YYou are now wanted on %s.&w\n\r",
                          planet->name);
                wanted->amount = STARTING_WANTED;
        }
        else
        {
                wanted->amount = UMAX(wanted->amount + 10, 100);
        }
        /*
         * if we hit a certain amount. add_bounty_police (which just does add_bounty and sets a flag?) 
         */
        if (wanted->amount >= 50)
                add_police_bounty(ch, planet);
        wanted->government = planet->governed_by;
}

CMDF do_payfee(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        BOUNTY_DATA *bounty;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: payfeee <clan>\n\r", ch);
                return;
        }

        if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BOUNTY))
        {
                send_to_char
                        ("You will have to go to your local Hunters Guild office to pay the fee for a bounty.\n\r",
                         ch);
                return;
        }

        if ((clan = get_clan(argument)) == NULL)
        {
                send_to_char("There is no government by that name!\n\r", ch);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
        {
                if (bounty->type != BOUNTY_POLICE)
                        continue;
                if (!str_cmp(ch->name, bounty->target)
                    && !str_cmp(clan->name, bounty->source))
                        break;
        }

        if (!bounty)
        {
                send_to_char("There is no bounty for you by that clan.\n\r",
                             ch);
                return;
        }

        if (ch->gold < bounty->amount * 3)
        {
                send_to_char("You don't have that many credits!\n\r", ch);
                return;
        }

        ch->gold -= bounty->amount * 3;
        clan->funds += bounty->amount * 3;
        remove_wanted(ch, clan);
        save_disintigrations();
        send_to_char
                ("You pay the fee, and the Hunters Guild removes your bounty.",
                 ch);

}

void add_police_bounty(CHAR_DATA * ch, PLANET_DATA * planet)
{
        BOUNTY_DATA *bounty;

        if (planet == NULL)
                return;
        if (planet->governed_by == NULL)
                return;
        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BOUNTY_POLICE &&
                    !str_cmp(ch->name, bounty->target) &&
                    !str_cmp(planet->governed_by->name, bounty->source))
                        break;

        if (bounty == NULL)
        {
                CREATE(bounty, BOUNTY_DATA, 1);
                LINK(bounty, first_disintigration, last_disintigration, next,
                     prev);

                bounty->target = STRALLOC(ch->name);
                bounty->amount = 1000;
        }
        else
        {
                bounty->amount = bounty->amount + 500;
        }
        bounty->source = STRALLOC(planet->governed_by->name);
        bounty->type = BOUNTY_POLICE;
        save_disintigrations();
        return;
}

bool is_wanted(CHAR_DATA * ch, PLANET_DATA * pl)
{
        WANTED_DATA *wanted;

        if (IS_NPC(ch))
                return FALSE;
        if (!ch->pcdata)
                return FALSE;
        if (pl->governed_by == NULL)
                return FALSE;

        for (wanted = ch->pcdata->first_wanted; wanted; wanted = wanted->next)
        {
                if (wanted->government == pl->governed_by)
                        return TRUE;
        }
        return FALSE;
}

void fwrite_wanted(CHAR_DATA * ch, FILE * fp)
{
        WANTED_DATA *wanted;

        if (!ch->pcdata)
                return;

        for (wanted = ch->pcdata->first_wanted; wanted; wanted = wanted->next)
        {
                fprintf(fp, "#WANTED\n");
                fprintf(fp, "Amount %d\n", wanted->amount);
                if (wanted->government)
                        fprintf(fp, "Government %s~\n",
                                wanted->government->name);
                fprintf(fp, "End\n\n");
        }
}

void fread_wanted(CHAR_DATA * ch, FILE * fp)
{
        const char *word;
        bool      fMatch;
        WANTED_DATA *wanted;

        CREATE(wanted, WANTED_DATA, 1);

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

                case 'A':
                        KEY("Amount", wanted->amount, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!wanted->government)
                                        DISPOSE(wanted);
                                else
                                        LINK(wanted, ch->pcdata->first_wanted,
                                             ch->pcdata->last_wanted, next,
                                             prev);
                                return;
                        }
                        break;
                case 'G':
                        if (!str_cmp(word, "Government"))
                        {
                                char     *temp = fread_string(fp);

                                wanted->government = get_clan(temp);
                                STRFREE(temp);
                                fMatch = TRUE;
                        }
                        break;
                }
        }
}

void remove_wanted_planet(CHAR_DATA * ch, PLANET_DATA * planet)
{
        remove_wanted(ch, planet->governed_by);
}

void remove_wanted(CHAR_DATA * ch, CLAN_DATA * clan)
{
        WANTED_DATA *wanted;
        BOUNTY_DATA *bounty;

        if (IS_NPC(ch))
                return;
        if (!clan)
                return;


        for (wanted = ch->pcdata->first_wanted; wanted; wanted = wanted->next)
        {
                if (clan == wanted->government)
                        break;
        }

        if (wanted)
        {
                UNLINK(wanted, ch->pcdata->first_wanted,
                       ch->pcdata->last_wanted, next, prev);
                DISPOSE(wanted);
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BOUNTY_POLICE &&
                    !str_cmp(ch->name, bounty->target) &&
                    !str_cmp(clan->name, bounty->source))
                        break;

        if (bounty)
        {
                UNLINK(bounty, first_disintigration, last_disintigration,
                       next, prev);
                STRFREE(bounty->target);
                STRFREE(bounty->source);
                DISPOSE(bounty);
        }
}

CMDF do_imprison(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        CLAN_DATA *clan = NULL;
        ROOM_INDEX_DATA *jail = NULL;
        BOUNTY_DATA *bounty = NULL;
        int       chance = 0;

        if (IS_NPC(ch))
                return;
        if (!ch->in_room || !ch->in_room->area || !ch->in_room->area->planet)
                return;
        clan = ch->in_room->area->planet->governed_by;

        if (argument[0] == '\0')
        {
                send_to_char("Jail who?\n\r", ch);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Interesting combat technique.\n\r", ch);
                return;
        }

        if (ch->position <= POS_SLEEPING)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (ch->mount)
        {
                send_to_char("You can't do that while mounted.\n\r", ch);
                return;
        }

        if (!clan)
        {
                send_to_char
                        ("Nobody owns this planet. How do you expect to turn them into the authorities",
                         ch);
                return;
        }

        jail = get_room_index(clan->jail);

        if (!jail)
        {
                send_to_char
                        ("This government does not have a suitable prison.\n\r",
                         ch);
                return;
        }

        if (is_wanted(ch, ch->in_room->area->planet))
        {
                send_to_char("You are wanted by the current government.\n\r",
                             ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("That's pointless.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("That would be a waste of time.\n\r", ch);
                return;
        }

        if (victim->position != POS_INCAP &&
            victim->position != POS_STUNNED &&
            victim->position != POS_SLEEPING && !victim->held)
        {
                send_to_char("You will have to incapacitate them first.\n\r",
                             ch);
                return;
        }

        chance = (int) (ch->pcdata->learned[gsn_imprison]);
        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE) && chance < 80)
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("This isn't a good place to do that.\n\r", ch);
                return;
        }
        if (number_percent() >= chance)
        {
                act(AT_ACTION,
                    "Your failed attempt to jail $n has allowed them to recover.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_ACTION,
                    "$n's failed attempt to jail you has allowed time for you to recover.",
                    ch, NULL, victim, TO_VICT);
                act(AT_ACTION,
                    "$n's failed attempt to jail $N has allowed time for them to recover.",
                    ch, NULL, victim, TO_NOTVICT);
                WAIT_STATE(ch, 10 * PULSE_VIOLENCE);
                victim->hit = 75;
                victim->position = POS_STANDING;
                update_pos(victim);
                return;
        }

        for (bounty = first_disintigration; bounty; bounty = bounty->next)
                if (bounty->type == BOUNTY_POLICE &&
                    !str_cmp(victim->name, bounty->target) &&
                    !str_cmp(clan->name, bounty->source))
                        break;

        if (!bounty)
        {
                send_to_char
                        ("They are not wanted by the current government.\n\r",
                         ch);
                return;
        }

        if (victim->hit < 0)
                victim->hit = 100;
        victim->position = POS_STANDING;
        update_pos(victim);

        ch_printf(ch,
                  "You collect the %ld bounty and escort them off to jail.\n\r",
                  bounty->amount);
        ch->gold += bounty->amount;

        act(AT_ACTION,
            "You have a strange feeling that you've been moved.\n\r", ch,
            NULL, victim, TO_VICT);
        act(AT_ACTION, "$n has $N escorted away.\n\r", ch, NULL, victim,
            TO_NOTVICT);

        char_from_room(victim);
        char_to_room(victim, jail);
        do_look(victim, "");
        act(AT_ACTION,
            "The door opens briefly as $n is shoved into the room.\n\r",
            victim, NULL, NULL, TO_ROOM);

        learn_from_success(ch, gsn_imprison);
        gain_exp(ch, bounty->amount * 3, HUNTING_ABILITY);
        remove_wanted_planet(victim, ch->in_room->area->planet);

        return;
}

CMDF do_rembounty(CHAR_DATA * ch, char *argument)
{
        BOUNTY_DATA *bounty;

        if (!argument || argument[0] == '\0')
        {
                do_bounties(ch, argument);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: rembounty <target>\n\r", ch);
                return;
        }

//  argument[0] = UPPER(argument[0]);
        bounty = get_disintigration(argument);
        if (bounty)
        {
                remove_disintigration(bounty);
                ch_printf(ch, "Done\n\r");
        }
        else
        {
                ch_printf(ch, "No Such Bounty\n\r");
                return;
        }
}

void free_bounty(BOUNTY_DATA * bounty)
{
        if (bounty->target)
                STRFREE(bounty->target);
        if (bounty->source)
                STRFREE(bounty->source);
        DISPOSE(bounty);
        bounty = NULL;
}

/*
 *  Sharpen skill
 *  Author: Rantic (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */
/*
#include <stdio.h>
#include "mud.h"
Moved to bounty.c since it's much more appropriate
*/

CMDF do_sharpen(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        OBJ_DATA *pobj;
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        int       percent;
        int       level;

        one_argument(argument, arg);


        if (arg[0] == '\0')
        {
                send_to_char("What do you wish to sharpen?\n\r", ch);
                return;
        }

        if (ms_find_obj(ch))
                return;

        if (!(obj = get_obj_carry(ch, arg)))
        {
                send_to_char("You do not have that weapon.\n\r", ch);
                return;
        }

        if (obj->item_type != ITEM_WEAPON)
        {
                send_to_char
                        ("You can't sharpen something that's not a weapon.\n\r",
                         ch);
                return;
        }

        /*
         * Let's not allow people to sharpen bludgeons and the like ;) 
         */
        /*
         * small mods to make it more generic.. --Cronel 
         */
        if (obj->value[3] != WEAPON_KNIFE)
        {
                send_to_char("You can't sharpen that type of weapon!\n\r",
                             ch);
                return;
        }

        if (obj->value[5] == 1) /* see reason below when setting */
        {
                send_to_char
                        ("It is already as sharp as it's going to get.\n\r",
                         ch);
                return;
        }

        for (pobj = ch->first_carrying; pobj; pobj = pobj->next_content)
        {
                if (pobj->pIndexData->vnum == OBJ_VNUM_SHARPEN)
                        break;
        }

        if (!pobj)
        {
                send_to_char("You do not have a sharpening stone.\n\r", ch);
                return;
        }

        WAIT_STATE(ch, skill_table[gsn_sharpen]->beats);
        /*
         * Character must have the dexterity to sharpen the weapon nicely, 
         * * if not, damage weapon 
         */
        if (!IS_NPC(ch) && get_curr_dex(ch) < 17)
        {
                separate_obj(obj);
                if (obj->value[0] <= 1)
                {
                        act(AT_OBJECT,
                            "$p breaks apart and falls to the ground in pieces!.",
                            ch, obj, NULL, TO_CHAR);
                        extract_obj(obj);
                        learn_from_failure(ch, gsn_sharpen);
                        return;
                }
                else
                {
                        obj->value[0]--;
                        act(AT_GREEN, "You clumsily slip and damage $p!", ch,
                            obj, NULL, TO_CHAR);
                        return;
                }
        }

        percent = (number_percent() - get_curr_lck(ch) - 15);   /* too low a chance to damage? */

        separate_obj(pobj);
        if (!IS_NPC(ch) && percent > ch->pcdata->learned[gsn_sharpen])
        {
                act(AT_OBJECT,
                    "You fail to sharpen $p correctly, damaging the stone.",
                    ch, obj, NULL, TO_CHAR);
                if (pobj->value[0] <= 1)
                {
                        act(AT_OBJECT,
                            "The sharpening stone crumbles apart from misuse.",
                            ch, pobj, NULL, TO_CHAR);
                        extract_obj(pobj);
                        learn_from_failure(ch, gsn_sharpen);
                        return;
                }
                pobj->value[0]--;
                learn_from_failure(ch, gsn_sharpen);
                return;
        }

        level = ch->top_level;
        separate_obj(obj);
        act(AT_SKILL,
            "With skill and precision, you sharpen $p to a fine edge.", ch,
            obj, NULL, TO_CHAR);
        act(AT_SKILL, "With skill and precision, $n sharpens $p.", ch, obj,
            NULL, TO_ROOM);
        CREATE(paf, AFFECT_DATA, 1);
        paf->type = -1;
        paf->duration = -1;
        paf->location = APPLY_DAMROLL;
        paf->modifier = level / 10;
        paf->bitvector = 0;
        LINK(paf, obj->first_affect, obj->last_affect, next, prev);

        obj->value[5] = 1;
        /*
         * originaly a sharpened object flag was used, but took up a BV, 
         * * so I switched to giving it a value5, which is not used in weapons 
         * * besides to check for this 
         */
        learn_from_success(ch, gsn_sharpen);
		learn_from_success(ch, gsn_hone);
        return;

}
