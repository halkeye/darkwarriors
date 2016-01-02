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
 *                                SWR OLC Channel module                                 *
 *  Several functions in this module(hasname, the base for do_listen, etc) have been     *
 *  taken/modified from the AFMMUD olc channel code. This was done, and all licenses for *
 *  their code applies here as well                                                      *
 ****************************************************************************************/

#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"
#include "channels.h"
#include "color.h"

char     *const channel_type[] = {
        "IC", "IC Comlink", "OOC"
};

char     *const channel_range[] = {
        "Room", "Area", "Planet", "System", "Global", "Clan"
};

DECLARE_DO_FUN(do_history);
extern bool is_ignoring(CHAR_DATA * ch, CHAR_DATA * victim);
extern char *const valid_color[];
char     *scramble args((const char *argument, LANGUAGE_DATA * language));

CHANNEL_DATA *first_channel = NULL;
CHANNEL_DATA *last_channel = NULL;

CHANNEL_DATA *get_channel(char *name)
{
        CHANNEL_DATA *channel;

        for (channel = first_channel; channel; channel = channel->next)
                if (!str_cmp(name, channel->name))
                        return channel;
        for (channel = first_channel; channel; channel = channel->next)
                if (nifty_is_name_prefix(name, channel->name))
                        return channel;
        return NULL;
}

void free_channel(CHANNEL_DATA * channel)
{
        if (channel->name)
                STRFREE(channel->name);
        if (channel->actmessage)
                DISPOSE(channel->actmessage);
        if (channel->emotemessage)
                DISPOSE(channel->emotemessage);
        if (channel->socialmessage)
                DISPOSE(channel->socialmessage);
        if (channel->log)
        {
                int       x;

                for (x = 0; x <= channel->logpos; x++)
                {
                        if (channel->log[x].name)
                                STRFREE(channel->log[x].name);
                        if (channel->log[x].message)
                                DISPOSE(channel->log[x].message);
                }
                DISPOSE(channel->log);
        }
        UNLINK(channel, first_channel, last_channel, next, prev);
        DISPOSE(channel);
        return;
}

#define IC_CHANNEL(channel) (((channel)->type == CHANNEL_IC || (channel)->type == CHANNEL_IC_COM))
#define OOC_CHANNEL(channel) (!IC_CHANNEL((channel)))
char     *act_string(const char *format, CHAR_DATA * to, CHAR_DATA * ch,
                     void *arg1, void *arg2, bool OOC);

bool player_is_listening(CHAR_DATA * ch, CHANNEL_DATA * channel)
{
	if (IS_NPC(ch) || !ch->pcdata || !channel)
		return FALSE;
	if (!hasname(ch->pcdata->listening, channel->name)) 
		return FALSE;
        if (ch->top_level < channel->level)
                return FALSE;
	return true;
}

bool check_channel(CHAR_DATA * ch, char *command, char *argument)
{
        CHANNEL_DATA *channel;
        CHAR_DATA *victim = NULL;
        PLANET_DATA *planet = NULL;
        CLAN_DATA *clan = NULL;
        DESCRIPTOR_DATA *d;
        SHIP_DATA *ship = NULL;
        bool      emote = false;
        SOCIALTYPE *social = NULL;
        char      buf[MSL], buf2[MSL];
        char      arg[MSL];

        char     *messagetype;

        if (!ch || !command)
                return FALSE;
        if ((channel = get_channel(command)) == NULL)
                return FALSE;
        if (channel->name[0] == '\0' || channel->actmessage[0] == '\0')
                return FALSE;
        if (ch->top_level < channel->level)
                return FALSE;

        if (!channel->enabled)
        {
                ch_printf(ch,
                          "You can't %s right not, that channel has been disabled.\n\r",
                          command);
                return TRUE;
        }

		if (!IS_NPC(ch) && !player_is_listening(ch, channel)) {
			send_to_char("You can't do that if you are not listening to that channel.\n\r", ch);
			return TRUE;
		}
			

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
        {
                ch_printf(ch, "You can't %s.\n\r", command);
                return TRUE;
        }

        if (ch->position < POS_RESTING
            && (channel->type == CHANNEL_IC
                || channel->type == CHANNEL_IC_COM))
                return TRUE;

        if (ch->in_room && IC_CHANNEL(channel)
            && xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return TRUE;
        }

        if (channel->type == CHANNEL_IC_COM && !has_comlink(ch))
        {
                send_to_char("You need a comlink to do that!\n\r", ch);
                return TRUE;
        }

        if (channel->range == CHANNEL_PLANET
            && (!ch->in_room->area
                || (planet = ch->in_room->area->planet) == NULL))
        {
                send_to_char("That channel only works on planets...\n\r", ch);
                return TRUE;
        }

        if (channel->range == CHANNEL_CLAN)
        {
                if (!ch->pcdata->clan)
                {
                        send_to_char("You are not in a clan!\n\r", ch);
                        return TRUE;
                }

                if (ch->pcdata->clan->mainclan)
                        clan = ch->pcdata->clan->mainclan;
                else
                        clan = ch->pcdata->clan;
        }

        if (channel->range == CHANNEL_SYSTEM)
        {
                if ((ship = ship_from_room(ch->in_room->vnum)) == NULL)
                {
                        send_to_char("You are not on a ship!", ch);
                        return TRUE;
                }

                if (ship_from_cockpit(ch->in_room->vnum) == NULL)
                {
                        send_to_char
                                ("You must be in the cockpit of a ship to do that!\n\r",
                                 ch);
                        return TRUE;
                }

                if (ship->shipstate == SHIP_DOCKED)
                {
                        send_to_char
                                ("You can't do that until after you've launched!\n\r",
                                 ch);
                        return TRUE;
                }

                if (ship->starsystem == NULL)
                {
                        send_to_char
                                ("You can only do that in a star system.\n\r",
                                 ch);
                        return TRUE;
                }
        }

        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        {
                if (ch->master)
                        send_to_char("I don't think so...\n\r", ch->master);
                return TRUE;
        }

        if (argument[0] == '\0')
        {
				/* history */
				do_history(ch, command);
                /* snprintf(buf, MSL, "%s what?\n\r", command);
                buf[0] = UPPER(buf[0]);
                send_to_char(buf, ch); */  /* where'd this line go? */
                return TRUE;
        }

        if (argument[0] == '@')
        {
                if (channel->socialmessage[0] == '\0')
                {
                        send_to_char
                                ("No socials allowed for this channel.\n\r",
                                 ch);
                        return TRUE;
                }
                argument++;
                argument = one_argument(argument, arg);
                if ((social = find_social(arg)) == NULL)
                        return FALSE;
                else if (OOC_CHANNEL(channel) && argument[0] != '\0'
                         && (victim = get_char_world(ch, argument)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return TRUE;
                }
                if (social->char_no_arg == NULL
                    || social->char_no_arg[0] == '\0')
                {
                        send_to_char(act_string
                                     (social->char_no_arg, ch, ch, NULL, NULL,
                                      0), ch);
                        return TRUE;
                }
                emote = TRUE;
        }
        else if (argument[0] == ',')
        {
                if (channel->emotemessage[0] == '\0')
                {
                        send_to_char
                                ("No emotes allowed for this channel.\n\r",
                                 ch);
                        return TRUE;
                }
                argument++;
                while (argument[0] != '\0' && isspace(*argument))
                {
                        argument++;
                }
                if (argument[0] == '\0')
                {
                        send_to_char("emote what?.\n\r", ch);
                        return TRUE;
                }
                emote = TRUE;

        }


        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf2, MSL, "%s: %s (%s)",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument,
                         command);
                append_to_file(LOG_FILE, buf2);
        }

        if ((channel->type == CHANNEL_IC || channel->type == CHANNEL_IC_COM)
            && channel->cost > 0)
        {
                if (ch->gold < channel->cost)
                {
                        send_to_char
                                ("You do not have enough credits to use that channel!",
                                 ch);
                        return TRUE;
                }
                ch->gold -= channel->cost;
        }


        strncpy(buf, argument, MSL);
        if (social)
        {
                char     *sbuf = argument;

                if (IC_CHANNEL(channel))
                {
                        sbuf = act_string(social->char_no_arg, ch, ch, NULL,
                                          victim, 0);
                }
                else
                {
                        if (victim == ch)
                                sbuf = act_string(social->char_auto, ch, ch,
                                                  NULL, victim, 1);
                        else if (victim)
                                sbuf = act_string(social->char_found, ch, ch,
                                                  NULL, victim, 1);
                        else
                                sbuf = act_string(social->char_no_arg, ch, ch,
                                                  NULL, victim, 1);
                }
                strncpy(buf, sbuf, MSL);
        }
        else if (emote)
        {
                strncpy(buf, argument, MSL);
        }

        add_channel_log(ch, buf, channel);
        if (social)
                messagetype = channel->socialmessage;
        else if (emote)
                messagetype = channel->emotemessage;
        else
                messagetype = channel->actmessage;
        if (channel->type == CHANNEL_IC || channel->type == CHANNEL_IC_COM)
                act(channel->color, messagetype, ch, buf, NULL, TO_CHAR);
        else
                act(channel->color, messagetype, ch, buf, NULL, TO_CHAR_OOC);

        for (d = first_descriptor; d; d = d->next)
        {
                CHAR_DATA *och;
                CHAR_DATA *vch;

                och = d->original ? d->original : d->character;
                vch = d->character;

                if (IS_PLAYING(d) && vch != ch
                    && hasname(och->pcdata->listening, channel->name))
                {
                        /*
                         * Ignoring Publicly 
                         */
                        char     *sbuf = argument;

                        if (channel->type == CHANNEL_IC_COM
                            && !has_comlink(och))
                                continue;
                        if (channel->level > ch->top_level)
                                continue;
                        if (channel->type != CHANNEL_OOC
                            && xIS_SET(vch->in_room->room_flags,
                                       ROOM_SILENCE))
                                continue;
                        if (channel->range == CHANNEL_PLANET)
                        {
                                if (!vch->in_room || !vch->in_room->area
                                    || !vch->in_room->area->planet
                                    || (vch->in_room->area->planet != planet)
                                    || xIS_SET(vch->in_room->room_flags,
                                               ROOM_INDOORS))
                                        continue;
                        }
                        if (channel->range == CHANNEL_ROOM)
                        {
                                if (ch->in_room != och->in_room)
                                        continue;
                        }
                        if (channel->range == CHANNEL_CLAN)
                        {
                                if (!vch->pcdata->clan)
                                        continue;
                                if (vch->pcdata->clan != clan
                                    && vch->pcdata->clan->mainclan != clan)
                                        continue;
                        }
                        if (channel->range == CHANNEL_SYSTEM)
                        {
                                SHIP_DATA *target;

                                if (!ship)
                                        continue;

                                if (!vch->in_room)
                                        continue;

                                target = ship_from_cockpit(vch->in_room->
                                                           vnum);

                                if (!target)
                                        continue;

                                if (target->starsystem != ship->starsystem)
                                        continue;
                                snprintf(buf, MSL, "%s %s&D: $t",
                                         channel->name, ship->name);
                        }

                        MOBtrigger = FALSE;
                        if (social)
                        {
                                if (IC_CHANNEL(channel))
                                {
                                        sbuf = act_string(social->
                                                          others_no_arg, vch,
                                                          ch, NULL, victim,
                                                          0);
                                        strncpy(buf, sbuf, MSL);
                                        sbuf = buf;
                                }
                                else
                                {
                                        if (victim == ch)
                                                sbuf = act_string(social->
                                                                  others_auto,
                                                                  vch, ch,
                                                                  NULL,
                                                                  victim, 1);
                                        else if (victim)
                                        {
                                                if (victim == vch)
                                                        sbuf = act_string
                                                                (social->
                                                                 vict_found,
                                                                 vch, ch,
                                                                 NULL, victim,
                                                                 1);
                                                else
                                                        sbuf = act_string
                                                                (social->
                                                                 others_found,
                                                                 vch, ch,
                                                                 NULL, victim,
                                                                 1);
                                        }
                                        else
                                                sbuf = act_string(social->
                                                                  others_no_arg,
                                                                  vch, ch,
                                                                  NULL,
                                                                  victim, 1);
                                }
                                strncpy(buf, sbuf, MSL);
                                sbuf = buf;
                        }
                        else
                        {
                                if ((channel->type == CHANNEL_IC
                                     || channel->type == CHANNEL_IC_COM)
                                    && !knows_language(vch, ch->speaking, vch)
                                    && (!IS_NPC(ch) || ch->speaking != 0))
                                        sbuf = scramble(argument,
                                                        ch->speaking);
                        }

                        if (IC_CHANNEL(channel))
                                act(channel->color, messagetype, ch, sbuf,
                                    vch, TO_VICT);
                        else
                                act(channel->color, messagetype, ch, sbuf,
                                    vch, TO_VICT_OOC);
                        MOBtrigger = TRUE;
                }
        }
        return TRUE;
}


void fwrite_channel(CHANNEL_DATA * channel, FILE * fp)
{
        if (!channel)
                return;

        fprintf(fp, "#CHANNEL\n");
        fprintf(fp, "Name         %s~\n", channel->name);
        fprintf(fp, "Actmessage   %s~\n", channel->actmessage);
        fprintf(fp, "EmoteMessage %s~\n", channel->emotemessage);
        fprintf(fp, "SocialMessage %s~\n", channel->socialmessage);
        fprintf(fp, "Logtype      %d\n", channel->logtype);
        fprintf(fp, "Type         %d\n", channel->type);
        fprintf(fp, "Color        %d\n", channel->color);
        fprintf(fp, "Range        %d\n", channel->range);
        fprintf(fp, "Level        %d\n", channel->level);
        fprintf(fp, "History      %d\n", channel->history);
        fprintf(fp, "Cost         %d\n", channel->cost);
        fprintf(fp, "Enabled      %d\n", channel->enabled);
        fprintf(fp, "End\n\n");
}

void save_channels(void)
{
        CHANNEL_DATA *channel;
        FILE     *fp;

        if ((fp = fopen(CHANNEL_FILE, "w")) == NULL)
        {
                bug("Cannot open channel.dat for writting", 0);
                perror(CHANNEL_FILE);
                return;
        }

        for (channel = first_channel; channel; channel = channel->next)
                fwrite_channel(channel, fp);
        fprintf(fp, "#END\n");
        FCLOSE(fp);
}

CHANNEL_DATA *create_channel(void)
{
        CHANNEL_DATA *channel;

        CREATE(channel, CHANNEL_DATA, 1);
        channel->next = NULL;
        channel->prev = NULL;
        channel->log = NULL;
        channel->name = NULL;
        channel->actmessage = NULL;
        channel->emotemessage = NULL;
        channel->socialmessage = NULL;
        channel->logtype = 0;
        channel->type = 0;
        channel->color = 0;
        channel->range = 0;
        channel->level = 0;
        channel->history = FALSE;
        channel->logpos = 0;
        channel->cost = 0;
        channel->enabled = TRUE;
        return channel;
}

CHANNEL_DATA *fread_channel(FILE * fp)
{
        const char *word;
        bool      fMatch;
        CHANNEL_DATA *channel;
        char      buf[MSL];

        channel = create_channel();

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
                        KEY("Actmessage", channel->actmessage,
                            fread_string_nohash(fp));
                        break;

                case 'C':
                        KEY("Color", channel->color, fread_number(fp));
                        KEY("Cost", channel->cost, fread_number(fp));
                        break;

                case 'E':
                        KEY("EmoteMessage", channel->emotemessage,
                            fread_string_nohash(fp));
                        KEY("Enabled", channel->enabled, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (!channel->name)
                                        channel->name = STRALLOC("");
                                if (!channel->actmessage)
                                        strdup_printf(&channel->actmessage,
                                                      "%s $n: $t",
                                                      channel->name);
                                if (!channel->socialmessage)
                                {
                                        if (channel->emotemessage)
                                        {
                                                /*
                                                 * Backwars compatibility 
                                                 */
                                                channel->socialmessage =
                                                        channel->emotemessage;
                                                channel->emotemessage = NULL;
                                        }
                                        else
                                        {
                                                strdup_printf(&channel->
                                                              socialmessage,
                                                              "(%s) * $t",
                                                              channel->name);
                                        }
                                }
                                if (!channel->emotemessage)
                                        strdup_printf(&channel->emotemessage,
                                                      "(%s) * $n $t",
                                                      channel->name);
                                return channel;
                        }
                        break;
                case 'H':
                        KEY("History", channel->history, fread_number(fp));
                        break;

                case 'L':
                        KEY("Level", channel->level, fread_number(fp));
                        KEY("Logtype", channel->logtype, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", channel->name, fread_string(fp));
                        break;


                case 'R':
                        KEY("Range", channel->range, fread_number(fp));
                        break;
                case 'S':
                        KEY("SocialMessage", channel->socialmessage,
                            fread_string_nohash(fp));
                        break;

                case 'T':
                        KEY("Type", channel->type, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "fread_channel: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

void load_channels(void)
{
        FILE     *fp;
        CHANNEL_DATA *channel;

        if ((fp = fopen(CHANNEL_FILE, "r")) != NULL)
        {
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
                                bug("Load_channels: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "CHANNEL"))
                        {
                                channel = fread_channel(fp);
                                LINK(channel, first_channel, last_channel,
                                     next, prev);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("load_channels: bad section.", 0);
                                continue;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
                bug("Cannot open channel.dat", 0);
                exit(0);
        }
}

CMDF do_showchannels(CHAR_DATA * ch, char *argument)
{
        CHANNEL_DATA *channel;

        if (!argument || argument[0] == '\0'
            || (channel = get_channel(argument)) == NULL)
        {
                send_to_char
                        ("That is not a valid channel. Choose one of the following:\n\r",
                         ch);
                for (channel = first_channel; channel;
                     channel = channel->next)
                        if (channel->level <= ch->top_level)
                                ch_printf(ch, "\t&R%-25s Level: %d\n\r",
                                          channel->name, channel->level);
                return;
        }
        ch_printf(ch, "&BN&zame:&w          %s\n\r", channel->name);
        ch_printf(ch, "&BA&zctmessage:&w    %s\n\r",
                  full_color(channel->actmessage));
        ch_printf(ch, "&BE&zmoteMessage:&w  %s\n\r",
                  full_color(channel->emotemessage));
        ch_printf(ch, "&BS&zocialMessage:&w %s\n\r",
                  full_color(channel->socialmessage));
        ch_printf(ch, "&BL&zevel:&w         %d\n\r", channel->level);
        ch_printf(ch, "&BL&zog Type:&w      %s\n\r",
                  log_flag[channel->logtype]);
        ch_printf(ch, "&BC&zolor:&w         %s\n\r",
                  pc_displays[channel->color]);
        ch_printf(ch, "&BR&zange:&w         %s\n\r",
                  channel_range[channel->range]);
        ch_printf(ch, "&BT&zype:&w          %s\n\r",
                  channel_type[channel->type]);
        ch_printf(ch, "&BH&zistory:&w       %s\n\r",
                  capitalize(true_false[channel->history]));
        ch_printf(ch, "&BE&znabled:&w       %s\n\r",
                  capitalize(true_false[channel->enabled]));
        ch_printf(ch, "&BC&zost:&w          %d\n\r", channel->cost);
}

CMDF do_makechannel(CHAR_DATA * ch, char *argument)
{
        CHANNEL_DATA *channel;

        if (!argument || argument[0] == '\0'
            || (channel = get_channel(argument)) != NULL)
        {
                send_to_char("&RSyntax: &Gmakechannel &C<&cname&C>&W\n\r",
                             ch);
                return;
        }

        channel = create_channel();
        if (channel->name)
                STRFREE(channel->name);
        channel->name = STRALLOC(argument);
        strdup_printf(&channel->actmessage, "%s $n: $t", channel->name);
        strdup_printf(&channel->emotemessage, "(%s) * $n $t", channel->name);
        strdup_printf(&channel->socialmessage, "(%s) * $t", channel->name);
        LINK(channel, first_channel, last_channel, next, prev);
        save_channels();
        send_to_char("Done.\n\r", ch);
}


CMDF do_setchannel(CHAR_DATA * ch, char *argument)
{
        CHANNEL_DATA *channel;
        char      arg[MSL];
        char     *arg2 = arg;
        int       x;

        argument = one_argument(argument, arg);
        if (argument[0] == '\0' || arg[0] == '\0')
        {
                interpret(ch, "showchannel");
                send_to_char
                        ("Valid fields are:\n\r\tname, socialmessage, emotemessage, actmessage, logtype, type, range, color, level, history, cost, enable",
                         ch);
                return;
        }


        if ((channel = get_channel(arg)) == NULL)
        {
                send_to_char("No such channel.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "delete"))
        {
                free_channel(channel);
                send_to_char("Done.\n\r", ch);
                save_channels();
                return;
        }

        else if (!str_cmp(argument, "history"))
        {
                channel->history = !channel->history;
                send_to_char("Done.\n\r", ch);
                save_channels();
                return;
        }

        else if (!str_cmp(argument, "enable"))
        {
                channel->enabled = !channel->enabled;
                send_to_char("Done.\n\r", ch);
                save_channels();
                return;
        }

        argument = one_argument(argument, arg);

        if (arg2[0] == '\0')
        {
                send_to_char("Do what?\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "name"))
        {
                if (channel->name)
                        STRFREE(channel->name);
                channel->name = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "actmessage"))
        {
                if (channel->actmessage)
                        DISPOSE(channel->actmessage);
                channel->actmessage = str_dup(argument);
                send_to_char("Done.\n\r", ch);
        }
        else if (!str_cmp(arg2, "emotemessage"))
        {
                if (channel->emotemessage)
                        DISPOSE(channel->emotemessage);
                channel->emotemessage = str_dup(argument);
                send_to_char("Done.\n\r", ch);
        }
        else if (!str_cmp(arg2, "socialmessage"))
        {
                if (channel->socialmessage)
                        DISPOSE(channel->socialmessage);
                channel->socialmessage = str_dup(argument);
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "logtype"))
        {
                channel->logtype = atoi(argument);
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "cost"))
        {
                channel->cost = atoi(argument);
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "type"))
        {
                for (x = 0; x < 3; x++)
                        if (!str_cmp(channel_type[x], argument))
                                break;
                if (x == 3)
                        x = 0;
                channel->type = x;
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "color"))
        {
                for (x = 0; x < MAX_COLORS; x++)
                        if (!str_cmp(pc_displays[x], argument))
                                break;
                if (x == MAX_COLORS)
                        x = 0;
                channel->color = x;
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "range"))
        {
                for (x = 0; x < 6; x++)
                        if (!str_cmp(channel_range[x], argument))
                                break;
                if (x == 6)
                        x = 0;
                channel->range = x;
                send_to_char("Done.\n\r", ch);
        }

        else if (!str_cmp(arg2, "level"))
        {
                channel->level = atoi(argument);
                send_to_char("Done.\n\r", ch);
        }

        else
        {
                send_to_char("That is not a valid argument. Options are:\n\r",
                             ch);
                send_to_char
                        ("\tname, socialmessage,emotemessage, actmessage, logtype, type, range, color, level, history, cost, enable\n\r",
                         ch);
                return;
        }
        save_channels();
}

/*  changetarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
const char *getarg(const char *argument, char *arg, int length)
{
        int       len = 0;

        if (!argument || argument[0] == '\0')
        {
                if (arg)
                        arg[0] = '\0';

                return argument;
        }

        while (*argument && isspace(*argument))
                argument++;

        if (arg)
                while (*argument && !isspace(*argument) && len < length - 1)
                        *arg++ = *argument++, len++;
        else
                while (*argument && !isspace(*argument))
                        argument++;

        while (*argument && !isspace(*argument))
                argument++;

        while (*argument && isspace(*argument))
                argument++;

        if (arg)
                *arg = '\0';

        return argument;
}

/* Check for a name in a list */
int hasname(const char *list, const char *name)
{
        const char *p;
        char      arg[MIL];

        if (!list)
                return (0);

        p = getarg(list, arg, MIL);
        while (arg[0])
        {
                if (!strcmp(name, arg))
                        return 1;
                p = getarg(p, arg, MIL);
        }

        return 0;
}

/* Add a name to a list */
void addname(char **list, const char *name)
{
        char      buf[MSL];

        if (hasname(*list, name))
                return;

        if (*list && *list[0] != '\0')
                snprintf(buf, MSL, "%s %s", *list, name);
        else
                mudstrlcpy(buf, name, MSL);

        STRFREE(*list);
        *list = STRALLOC(buf);
}

/* Remove a name from a list */
void removename(char **list, const char *name)
{
        char      buf[MSL];
        char      arg[MIL];
        const char *p;

        buf[0] = '\0';
        p = getarg(*list, arg, MIL);
        while (arg[0])
        {
                if (strcmp(arg, name))
                {
                        if (buf[0])
                                mudstrlcat(buf, " ", MSL);
                        mudstrlcat(buf, arg, MSL);
                }
                p = getarg(p, arg, MIL);
        }

        STRFREE(*list);
        *list = STRALLOC(buf);
}

CMDF do_listen(CHAR_DATA * ch, char *argument)
{
        CHANNEL_DATA *channel;

        if (IS_NPC(ch))
                return;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Syntax: listen <channel>\n\r", ch);
                send_to_char("Syntax: listen all\n\r", ch);
                send_to_char("Syntax: listen none\n\r", ch);
                send_to_char("For a list of channels, type channels\n\r", ch);
                send_to_char
                        ("You are listening to the following local mud channels:\n\r\n\r",
                         ch);
                ch_printf(ch, "%s\n\r", ch->pcdata->listening);
                return;
        }

        if (!str_cmp(argument, "all"))
        {
                for (channel = first_channel; channel;
                     channel = channel->next)
                {
                        if (ch->top_level >= channel->level
                            && !hasname(ch->pcdata->listening, channel->name))
                                addname(&ch->pcdata->listening,
                                        channel->name);
                }
                send_to_char
                        ("You are now listening to all available channels.\n\r",
                         ch);
                return;
        }

        if (!str_cmp(argument, "none"))
        {
                for (channel = first_channel; channel;
                     channel = channel->next)
                {
                        if (hasname(ch->pcdata->listening, channel->name))
                                removename(&ch->pcdata->listening,
                                           channel->name);
                }
                send_to_char
                        ("You no longer listen to any available channels.\n\r",
                         ch);
                return;
        }

        if (hasname(ch->pcdata->listening, argument))
        {
                removename(&ch->pcdata->listening, argument);
                ch_printf(ch, "You no longer listen to %s\n\r", argument);
        }
        else
        {
                if (IS_IMMORTAL(ch))
                {
                        if (!str_cmp(argument, "build"))
                        {
                                xTOGGLE_BIT(ch->deaf, CHANNEL_BUILD);
                                send_to_char
                                        ("The build channel has been toggled",
                                         ch);
                                return;
                        }
                        if (!str_cmp(argument, "log"))
                        {
                                xTOGGLE_BIT(ch->deaf, CHANNEL_LOG);
                                send_to_char
                                        ("The log channel has been toggled",
                                         ch);
                                return;
                        }
                        if (!str_cmp(argument, "comm"))
                        {
                                xTOGGLE_BIT(ch->deaf, CHANNEL_COMM);
                                send_to_char
                                        ("The comm channel has been toggled",
                                         ch);
                                return;
                        }
                }
                if (!str_cmp(argument, "tells"))
                {
                        xTOGGLE_BIT(ch->deaf, CHANNEL_TELLS);
                        send_to_char("The tell channels has been toggled",
                                     ch);
                        return;
                }
                if (!str_cmp(argument, "auction"))
                {
                        xTOGGLE_BIT(ch->deaf, CHANNEL_AUCTION);
                        send_to_char("The auction channel has been toggled",
                                     ch);
                        return;
                }

                if (!(channel = get_channel(argument)))
                {
                        send_to_char("No such channel.\n\r", ch);
                        return;
                }
                if (channel->level > ch->top_level)
                {
                        send_to_char("That channel is above your level.\n\r",
                                     ch);
                        return;
                }
                addname(&ch->pcdata->listening, channel->name);
                ch_printf(ch, "You now listen to %s\n\r", channel->name);
        }
        return;
}

CMDF do_channels(CHAR_DATA * ch, char *argument)
{
        CHANNEL_DATA *channel;
        CHAR_DATA *victim = NULL;

        if ( argument[0] == '-'  || argument[0] == '+') {
                /* add in support for +|-all */
                if ((channel = get_channel(argument+1)) == NULL)
                {
                        send_to_char( "Set or clear which channel?\n\r", ch );
                        return;
                }

                if (argument[0] == '-') {
                        if (hasname(ch->pcdata->listening, argument))
                                removename(&ch->pcdata->listening, argument);
                        ch_printf(ch, "You no longer listen to %s\n\r", channel->name);
                }
                else {
                        if (channel->level > get_trust(ch))
                        {
                                send_to_char("That channel is above your level.\n\r",
                                                ch);
                                return;
                        }
                        if (!hasname(ch->pcdata->listening, argument))
                                addname(&ch->pcdata->listening, channel->name);
                        ch_printf(ch, "You now listen to %s\n\r", channel->name);
                }
                return;
        }

        if (!IS_IMMORTAL(ch)
                        || (victim = get_char_world(ch, argument)) == NULL
                        || IS_NPC(victim))
                victim = ch;

        if (!victim) {
                if (IS_IMMORTAL(ch)) {
                        send_to_char("Who?\n\r", ch);
                        return;
                }
                victim = ch;
        }

        send_to_char("The following channels are available:\n\r", ch);
        send_to_char("To toggle a channel, use the listen command.\n\r\n\r",
                     ch);
        for (channel = first_channel; channel; channel = channel->next)
                if (victim->top_level >= channel->level)
                        ch_printf(ch, "&B%-c&z%-16s &z[&w%9s&z]&D\n\r",
                                  UPPER(channel->name[0]), channel->name + 1,
                                  (hasname
                                   (victim->pcdata->listening,
                                    channel->name)) ? "Listening" : "");
        ch_printf(ch, "&BT&zells             &z[&w%9s&z]&D\n\r",
                  !xIS_SET(victim->deaf, CHANNEL_TELLS) ? "Listening" : "");
        ch_printf(ch, "&BA&zuction           &z[&w%9s&z]&D\n\r",
                  !xIS_SET(victim->deaf, CHANNEL_AUCTION) ? "Listening" : "");
        if (IS_IMMORTAL(victim))
        {
                ch_printf(ch, "&BL&zog               &z[&w%9s&z]&D\n\r",
                          !xIS_SET(victim->deaf,
                                   CHANNEL_LOG) ? "Listening" : "");
                ch_printf(ch, "&BB&zuild             &z[&w%9s&z]&D\n\r",
                          !xIS_SET(victim->deaf,
                                   CHANNEL_BUILD) ? "Listening" : "");
                ch_printf(ch, "&BC&zomm              &z[&w%9s&z]&D\n\r",
                          !xIS_SET(victim->deaf,
                                   CHANNEL_COMM) ? "Listening" : "");
        }

        return;
}

void add_channel_log(CHAR_DATA * from, char *message, CHANNEL_DATA * channel)
{
        int       i;

        if (!channel->history)
                return;

        if (channel->log == NULL)
        {
                CREATE(channel->log, LOG_DATA, sysdata.channellog);
                channel->logpos = -1;
        }

        smash_tilde(message);
        if (++channel->logpos == sysdata.channellog)
        {
                if (channel->log[0].name)
                        STRFREE(channel->log[0].name);
                if (channel->log[0].message)
                        DISPOSE(channel->log[0].message);
                channel->log[0].time = 0;
                channel->log[0].language = NULL;
                for (i = 0; i < (sysdata.channellog - 1); i++)
                {
                        if (channel->log[i].name)
                                STRFREE(channel->log[i].name);
                        channel->log[i].name =
                                STRALLOC(channel->log[i + 1].name);
                        if (channel->log[i].message)
                                DISPOSE(channel->log[i].message);
                        channel->log[i].message =
                                str_dup(channel->log[i + 1].message);
                        channel->log[i].time = channel->log[i + 1].time;
                        channel->log[i].language =
                                channel->log[i + 1].language;
                }
                --channel->logpos;
        }
        if (channel->log[channel->logpos].name)
                STRFREE(channel->log[channel->logpos].name);
        channel->log[channel->logpos].name =
                STRALLOC(IS_SET(from->act, PLR_WIZINVIS) ? (char *)
                         "An Immortal" : from->name);
        if (channel->log[channel->logpos].message)
                DISPOSE(channel->log[channel->logpos].message);
        channel->log[channel->logpos].message = str_dup(message);
        channel->log[channel->logpos].time = current_time;
        channel->log[channel->logpos].language = from->speaking;
}

CMDF do_history(CHAR_DATA * ch, char *argument)
{
        int       count = 0;
        int       pos = 0;
        CHANNEL_DATA *channel;
        char      buf[MSL];
        char      chan[100];

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Syntanx: history <channel>", ch);
                return;
        }

        if ((channel = get_channel(argument)) == NULL)
        {
                send_to_char("That is not a valid channel", ch);
                return;
        }

        if (channel->history == FALSE || !channel->log
            || ch->top_level < channel->level)
        {
                send_to_char("That channel does not have a log.", ch);
                return;
        }

        ch_printf(ch, "&B%c&z%s History\n\r", channel->name[0],
                  channel->name + 1);
        send_to_char("&B-----------\n\r", ch);

        while (1)
        {
                if (count++ >= sysdata.channellog)
                        break;
                if (pos > channel->logpos)
                        break;
                buf[0] = '\0';
                chan[0] = '\0';

                if (channel->type != CHANNEL_OOC &&
                    !knows_language(ch, channel->log[pos].language, ch))
                {
                        snprintf(buf, MSL, "%s",
                                 scramble(channel->log[pos].message,
                                          channel->log[pos].language));
                }
                else
                {
                        snprintf(buf, MSL, "%s", channel->log[pos].message);
                        if (channel->type != CHANNEL_OOC)
                                snprintf(chan, 100, "(%s) ",
                                         channel->log[pos].language->name);
                }

                set_char_color(channel->color, ch);
                ch_printf(ch,
                          "&B[&W%2d&B][&W%.24s&B]&D %s %s&W: &Y%s&W%s&w\n\r",
                          count,
                          ctime(&channel->log[pos].time),
                          channel->name, channel->log[pos].name, chan, buf);
                pos++;
        }
}

char     *full_color(char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '&')
                {
                        *retptr = *str;
                        retptr++;
                        *retptr = '&';
                        retptr++;
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

#if 0
void send_social(CHAR_DATA * from, CHAR_DATA * to, char *argument)
{
        if (argument[0] == '\0')
        {
                act(AT_SOCIAL, social->others_no_arg, ch, NULL, victim,
                    TO_ROOM);
                act(AT_SOCIAL, social->char_no_arg, ch, NULL, victim,
                    TO_CHAR);
        }
        else if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
        }
        else if (victim == ch)
        {
                act(AT_SOCIAL, social->others_auto, ch, NULL, victim,
                    TO_ROOM);
                act(AT_SOCIAL, social->char_auto, ch, NULL, victim, TO_CHAR);
        }
        else
        {
                act(AT_SOCIAL, social->others_found, ch, NULL, victim,
                    TO_NOTVICT);
                act(AT_SOCIAL, social->char_found, ch, NULL, victim, TO_CHAR);
                act(AT_SOCIAL, social->vict_found, ch, NULL, victim, TO_VICT);
        }
}
#endif
