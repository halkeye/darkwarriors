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
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                $Id: act_comm.c 1344 2005-12-10 21:25:53Z halkeye $                *
 ****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "mxp.h"
#include "msp.h"
#include "account.h"
#include "races.h"
/*
 *  Externals
 */
void      send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx,
                                char page);
void      send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx,
                                 char page);
void      send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void      send_control_page_to_char(CHAR_DATA * ch, char page);
extern bool is_ignoring(CHAR_DATA * ch, CHAR_DATA * victim);

/*
 * Local functions.
 */
void talk_channel args((CHAR_DATA * ch, char *argument,
                        int channel, const char *verb));

char     *scramble args((const char *argument, LANGUAGE_DATA * language));
char     *drunk_speech args((const char *argument, CHAR_DATA * ch));

/*
 * Generic channel function.
 */
bool has_comlink(CHAR_DATA * ch)
{
        OBJ_DATA *obj;

        if (IS_IMMORTAL(ch) || IS_NPC(ch))
                return TRUE;
#ifdef CYBER
        if (ch->pcdata->cyber & CYBER_COMM)
                return TRUE;
#endif
        if (ch->race && !str_cmp(ch->race->name(), "droid"))
                return TRUE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->pIndexData->item_type == ITEM_COMLINK)
                        return TRUE;
        }
        return FALSE;
}

/*
 * Get the com freq for a player.
 */
int get_comfreq(CHAR_DATA * ch)
{
        if (IS_NPC(ch))
                return -1;

        if (!has_comlink(ch))
                return 0;
        else if (ch->pcdata->comchan)
                return ch->pcdata->comchan;
        else
                return 0;
        return 0;
}

OBJ_DATA *get_comlink(CHAR_DATA * ch)
{
        OBJ_DATA *obj;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                if (obj->item_type == ITEM_COMLINK)
                        return obj;
        return NULL;
}

/*
 * Gocial added by Atrox
 */

CMDF do_xsocial(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        SOCIALTYPE *social;
        char      command[MAX_INPUT_LENGTH];
        int       iHash;
        int       col = 0;


        if (IS_NPC(ch))
        {
                return;
        }

        if (!ch->pcdata->realage != 1)
        {
                send_to_char
                        ("You must set your legal status before you can use this command",
                         ch);
                send_to_char("see: setself legal", ch);
                return;
        }

        argument = one_argument(argument, command);

        if (command[0] == '\0')
        {
                set_pager_color(AT_PLAIN, ch);
                send_to_pager
                        ("&w<>&B----------------------------------------------------------------------&w<>\n\r",
                         ch);
                send_to_pager
                        ("&B||----------------------------------------------------------------------||\n\r&B|",
                         ch);

                for (iHash = 0; iHash < 27; iHash++)
                        for (social = social_index[iHash]; social;
                             social = social->next)
                        {
                                if (social->minarousal != 0
                                    || social->arousal != 0)
                                {
                                        pager_printf(ch, "&B[&w%-22s&B]",
                                                     center_str(social->name,
                                                                22));
                                        if (++col % 3 == 0)
                                                send_to_pager("&B|\n\r&B|",
                                                              ch);
                                }
                        }
                if (col % 3 != 0)
                        pager_printf(ch, "%s&B]|\n\r",
                                     center_str(" ", (24 * (3 - (col % 3)))));
                send_to_pager
                        ("&B||----------------------------------------------------------------------||\n\r",
                         ch);
                send_to_pager
                        ("&w<>&B----------------------------------------------------------------------&w<>\n\r",
                         ch);
                return;
        }

        xREMOVE_BIT(ch->deaf, CHANNEL_TELLS);
        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("&BY&zou can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) && (IS_SET(ch->act, PLR_SILENCE)))
        {
                send_to_char("&BY&zou can't do that.\n\r", ch);
                return;
        }

        if ((social = find_xsocial(command)) == NULL)
        {
                send_to_char("&BP&zick a correct xsocial to use1.\n\r", ch);
                return;
        }
        if (social->arousal == 0)
        {
                send_to_char("&BP&zick a correct xsocial to use2.\n\r", ch);
                return;
        }

        switch (ch->position)
        {
        case POS_DEAD:
                send_to_char("&BL&zie still; you are DEAD.\n\r", ch);
                return;
        case POS_INCAP:
        case POS_MORTAL:
                send_to_char("&BY&zou are hurt far too bad for that.\n\r",
                             ch);
                return;
        case POS_STUNNED:
                send_to_char("&BY&zou are too stunned to do that.\n\r", ch);
                return;
        case POS_SLEEPING:
                break;
        default:
                {
                }
        }

        one_argument(argument, arg);
        victim = NULL;

        if (arg[0] == '\0')
        {
                snprintf(buf, MSL, "%s", social->others_no_arg);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_ROOM);
                snprintf(buf, MSL, "%s", social->char_no_arg);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
                ch->pcdata->arousal += (sh_int) (social->arousal * .25);
                return;
        }
        victim = get_char_room(ch, arg);

        if (victim == NULL || (!can_see(ch, victim)))
        {
                send_to_char("&BT&zhat person isn't here on!\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("&BT&zhat person isn't here on!\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_AFK) || !victim->pcdata->realage != 1)
        {
                send_to_char("&BY&zou cannot do that to that person!", ch);
                return;
        }

        if (social->minarousal > victim->pcdata->arousal)
        {
                send_to_char("&BT&zhat person is not aroused enough!", ch);
                return;
        }

        if (victim->sex != social->sex && social->sex != 3)
        {
                send_to_char("&BT&zhey are not the right sex for that!", ch);
                return;
        }

        if (victim == ch && social->arousal > 4)
        {
                send_to_char("&BY&zou can't do that to yourself!", ch);
                return;
        }

        if (victim == ch)
        {
                snprintf(buf, MSL, "%s", social->others_auto);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_ROOM);
                snprintf(buf, MSL, "%s", social->char_auto);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
                ch->pcdata->arousal += (sh_int) (social->arousal * .5);
                /*
                 * FIXME - 
                 * * Gavin - Uh... all 3 of these victim->sex lines had ; at the end.
                 * * ie, if (victim->sex == 0);
                 * * which if thats the case, could be alot easier to write another way.
                 */
                if (victim->sex == SEX_NEUTRAL)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (2 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SEX_MALE)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (3 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SEX_FEMALE)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (1 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                return;
        }
        else
        {
                snprintf(buf, MSL, "%s", social->others_found);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_NOTVICT);
                snprintf(buf, MSL, "%s", social->char_found);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
                snprintf(buf, MSL, "%s", social->vict_found);
                act(AT_SOCIAL, buf, ch, NULL, victim, TO_VICT);
                victim->pcdata->arousal += social->arousal;
                ch->pcdata->arousal += (sh_int) (social->arousal * .25);
                if (victim->sex == SEX_NEUTRAL)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (2 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SEX_MALE)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (3 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                if (victim->sex == SEX_FEMALE)
                {
                        if (victim->pcdata->arousal >=
                            (70 +
                             (1 * (get_curr_con(victim)) +
                              (0.1 * number_percent()))))
                        {
                        }
                }
                return;
        }
}

CMDF do_beep(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);

        xREMOVE_BIT(ch->deaf, CHANNEL_TELLS);
        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) &&
            (IS_SET(ch->act, PLR_SILENCE) || IS_SET(ch->act, PLR_NO_TELL)))
        {
                send_to_char("&BY&zou can't do that.\n\r", ch);
                return;
        }

        if (arg[0] == '\0')
        {
                send_to_char("&BB&zeep who?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL
            || (IS_NPC(victim) && victim->in_room != ch->in_room)
            || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
        {
                send_to_char("&BT&zhey aren't here.\n\r", ch);
                return;
        }

        if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim))
        {
                send_to_char
                        ("&BT&zhey can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IS_NPC(victim) && (victim->switched)
            && (get_trust(ch) > LEVEL_AVATAR))
        {
                send_to_char("&BT&zhat player is switched.\n\r", ch);
                return;
        }

        else if (!IS_NPC(victim) && (!victim->desc))
        {
                send_to_char("&BT&zhat player is link-dead.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, CHANNEL_TELLS)
            && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_WORKING) && !IS_IMMORTAL(ch))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_SILENCE)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
            || (!IS_NPC(victim)
                && xIS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
        {
                act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == CON_EDITING
            && get_trust(ch) < LEVEL_GOD)
        {
                act(AT_PLAIN,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, TO_CHAR);
                return;
        }

        if (IS_MXP(ch))
                act(AT_WHITE,
                    "\aYou beep " MXPTAG("player $G") "$N" MXPTAG("/player")
                    ": '$t'", ch, argument, victim, TO_CHAR_OOC);
        else
                act(AT_WHITE, "\aYou beep $N: '$t'", ch, argument, victim,
                    TO_CHAR_OOC);
        if (IS_MXP(victim))
                act(AT_WHITE,
                    "\a" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " beeps: '$t'", ch, argument, victim, TO_VICT_OOC);
        else
                act(AT_WHITE, "\a$n beeps: '$t'", ch, argument, victim,
                    TO_VICT_OOC);
}

/* Text scrambler -- Altrag */
char     *scramble(const char *argument, LANGUAGE_DATA * language)
{
        static char arg[MAX_INPUT_LENGTH];
        sh_int    position, modifier;
        sh_int    conversion = 0;

        language = NULL;

        modifier = number_range(80, 300);   /* Bitvectors get way too large #s */
        for (position = 0; position < MAX_INPUT_LENGTH; position++)
        {
                if (argument[position] == '\0')
                {
                        arg[position] = '\0';
                        return arg;
                }
                else if (argument[position] >= 'A'
                         && argument[position] <= 'Z')
                {
                        conversion =
                                -conversion + position - modifier +
                                argument[position] - 'A';
                        conversion =
                                number_range(conversion - 5, conversion + 5);
                        while (conversion > 25)
                                conversion -= 26;
                        while (conversion < 0)
                                conversion += 26;
                        arg[position] = conversion + 'A';
                }
                else if (argument[position] >= 'a'
                         && argument[position] <= 'z')
                {
                        conversion =
                                -conversion + position - modifier +
                                argument[position] - 'a';
                        conversion =
                                number_range(conversion - 5, conversion + 5);
                        while (conversion > 25)
                                conversion -= 26;
                        while (conversion < 0)
                                conversion += 26;
                        arg[position] = conversion + 'a';
                }
                else if (argument[position] >= '0'
                         && argument[position] <= '9')
                {
                        conversion =
                                -conversion + position - modifier +
                                argument[position] - '0';
                        conversion =
                                number_range(conversion - 2, conversion + 2);
                        while (conversion > 9)
                                conversion -= 10;
                        while (conversion < 0)
                                conversion += 10;
                        arg[position] = conversion + '0';
                }
                else
                        arg[position] = argument[position];
        }
        arg[position] = '\0';
        return arg;
}

/* I'll rewrite this later if its still needed.. -- Altrag */
char     *translate(CHAR_DATA * ch, CHAR_DATA * victim, const char *argument)
{
        ch = 0;
        victim = 0;
        argument = 0;
        return "";
}

char     *drunk_speech(const char *argument, CHAR_DATA * ch)
{
        const char *arg = argument;
        static char buf[MAX_INPUT_LENGTH * 2];
        char      buf1[MAX_INPUT_LENGTH * 2];
        sh_int    drunk;
        char     *txt;
        char     *txt1;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                mudstrlcpy(buf, argument, MIL * 2);
                return buf;
        }

        drunk = ch->pcdata->condition[COND_DRUNK];
        /*
         * Speak funny if you have a broken jaw - Gavin 
         */
        if (IS_SET(ch->bodyparts, BODY_JAW))
                drunk += 15;

        if (drunk <= 0)
        {
                mudstrlcpy(buf, argument, MIL * 2);
                return buf;
        }

        buf[0] = '\0';
        buf1[0] = '\0';

        if (!argument)
        {
                bug("Drunk_speech: NULL argument", 0);
                return "";
        }

        /*
         * if ( *arg == '\0' )
         * return (char *) argument;
         */

        txt = buf;
        txt1 = buf1;

        while (*arg != '\0')
        {
                if (toupper(*arg) == 'S')
                {
                        if (number_percent() < (drunk * 2)) /* add 'h' after an 's' */
                        {
                                *txt++ = *arg;
                                *txt++ = 'h';
                        }
                        else
                                *txt++ = *arg;
                }
                else if (toupper(*arg) == 'X')
                {
                        if (number_percent() < (drunk * 2 / 2))
                        {
                                *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
                        }
                        else
                                *txt++ = *arg;
                }
                else if (number_percent() < (drunk * 2 / 5))    /* slurred letters */
                {
                        sh_int    slurn = number_range(1, 2);
                        sh_int    currslur = 0;

                        while (currslur < slurn)
                                *txt++ = *arg, currslur++;
                }
                else
                        *txt++ = *arg;

                arg++;
        };

        *txt = '\0';

        txt = buf;

        while (*txt != '\0')    /* Let's mess with the string's caps */
        {
                if (number_percent() < (2 * drunk / 2.5))
                {
                        if (isupper(*txt))
                                *txt1 = tolower(*txt);
                        else if (islower(*txt))
                                *txt1 = toupper(*txt);
                        else
                                *txt1 = *txt;
                }
                else
                        *txt1 = *txt;

                txt1++, txt++;
        };

        *txt1 = '\0';
        txt1 = buf1;
        txt = buf;

        while (*txt1 != '\0')   /* Let's make them stutter */
        {
                if (*txt1 == ' ')   /* If there's a space, then there's gotta be a */
                {   /* along there somewhere soon */

                        while (*txt1 == ' ')    /* Don't stutter on spaces */
                                *txt++ = *txt1++;

                        if ((number_percent() < (2 * drunk / 4))
                            && *txt1 != '\0')
                        {
                                sh_int    offset = number_range(0, 2);
                                sh_int    pos = 0;

                                while (*txt1 != '\0' && pos < offset)
                                        *txt++ = *txt1++, pos++;

                                if (*txt1 == ' ')   /* Make sure not to stutter a space after */
                                {   /* the initial offset into the word */
                                        *txt++ = *txt1++;
                                        continue;
                                }

                                pos = 0;
                                offset = number_range(2, 4);
                                while (*txt1 != '\0' && pos < offset)
                                {
                                        *txt++ = *txt1;
                                        pos++;
                                        if (*txt1 == ' ' || pos == offset)  /* Make sure we don't stick */
                                        {   /* A hyphen right before a space */
                                                txt1--;
                                                break;
                                        }
                                        *txt++ = '-';
                                }
                                if (*txt1 != '\0')
                                        txt1++;
                        }
                }
                else
                        *txt++ = *txt1++;
        }

        *txt = '\0';

        return buf;
}

void info_chan(char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        snprintf(buf, MSL, "&B[&zINFO&B] &w%s&R&W", argument);
        echo_to_all(AT_GOSSIP, buf, ECHOTAR_ALL);
}

void to_channel(const char *argument, int channel, const char *verb,
                sh_int level)
{
        char      buf[MAX_STRING_LENGTH];
        DESCRIPTOR_DATA *d;

        if (!first_descriptor || argument[0] == '\0')
                return;

        snprintf(buf, MSL, "%s: %s\r\n", verb, argument);

        for (d = first_descriptor; d; d = d->next)
        {
                CHAR_DATA *och;
                CHAR_DATA *vch;

                och = d->original ? d->original : d->character;
                vch = d->character;

                if (!och || !vch)
                        continue;
                if (!IS_IMMORTAL(vch)
                    || (get_trust(vch) < sysdata.build_level
                        && channel == CHANNEL_BUILD)
                    || (get_trust(vch) < sysdata.log_level
                        && (channel == CHANNEL_LOG
                            || channel == CHANNEL_COMM)))
                        continue;

                if (IS_PLAYING(d)
                    && !xIS_SET(och->deaf, channel)
                    && get_trust(vch) >= level)
                {
                        set_char_color(AT_LOG, vch);
                        send_to_char(buf, vch);
                }
        }

        return;
}

CMDF do_shout(CHAR_DATA * ch, char *argument)
{
        ROOM_INDEX_DATA *room;
        EXIT_DATA *pexit = NULL;

        if (!ch->in_room)
        {
                bug("%s is not in a room", ch->name);
                send_to_char("Huh?\n\r", ch);
                return;
        }
        room = ch->in_room;

        /*
         * if (NOT_AUTHED(ch))
         * {
         * send_to_char("Huh?\n\r", ch);
         * return;
         * } - I don't think we need this - Gavin 
         */

        /*
         * Need to scrable 
         */
        act(AT_SHOUT, "You shout '$t&c'", ch, drunk_speech(argument, ch), ch,
            TO_CHAR);
        act(AT_SHOUT, "$n shouts '$t&c'", ch, drunk_speech(argument, ch), ch,
            TO_ROOM);
        char_from_room(ch);
        for (pexit = room->first_exit; pexit; pexit = pexit->next)
        {
                if (pexit->to_room && room != pexit->to_room
                    && !IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        char_to_room(ch, pexit->to_room);
                        act(AT_SHOUT, "$n shouts '$t&c'", ch,
                            drunk_speech(argument, ch), ch, TO_ROOM);
                        char_from_room(ch);
                }
        }
        char_to_room(ch, room);
/*	talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_SHOUT, "shout" );*/
        WAIT_STATE(ch, 12);
        return;
}


CMDF do_osay(CHAR_DATA * ch, char *argument)
{
        if (argument[0] == '\0')
        {
                send_to_char("OSay what?\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        act(AT_FLEE, "$n osays &R[&Y$t&R]", ch, argument, ch, TO_ROOM_OOC);
        act(AT_SAY, "You osay &R[&Y$t&R]", ch, argument, ch, TO_CHAR_OOC);
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                char      buf[MAX_STRING_LENGTH];

                snprintf(buf, MSL, "%s: %s",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LOG_FILE, buf);
        }
}



CMDF do_whisper(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH], _last_char;
        CHAR_DATA *victim;
        int       arglen;
        int       actflags;
        char     *sbuf = NULL;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Say what to whom?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, arg)) == NULL
            || IS_NPC(victim)
            || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = strlen(argument) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;
        MOBtrigger = FALSE;

        sbuf = argument;

        if (IS_NPC(ch))
                REMOVE_BIT(ch->act, ACT_SECRETIVE);

        if (!knows_language(victim, ch->speaking, ch)
            && (!IS_NPC(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        switch (_last_char)
        {
        case '?':
                act(AT_WHITE, "You whisper to $N, '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, TO_CHAR);
                act(AT_WHITE, "$n quietly asks you '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, TO_VICT);
                break;

        default:
                act(AT_WHITE, "You whisper to $N '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, TO_CHAR);
                act(AT_WHITE, "$n whispers to you '$t&w'", ch,
                    drunk_speech(sbuf, ch), victim, TO_VICT);
                break;
        }
        act(AT_WHITE, "$n whispers something to $N", ch, NULL, victim,
            TO_NOTVICT);
        ch->act = actflags;
        MOBtrigger = TRUE;

        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                char      buf[MAX_STRING_LENGTH];

                snprintf(buf, MSL, "%s: %s",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LOG_FILE, buf);
        }
        mprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(argument, ch);
        return;
}

CMDF do_tell(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       position;
        char     *sbuf = argument;

        if (xIS_SET(ch->deaf, CHANNEL_TELLS) && !IS_IMMORTAL(ch))
        {
                act(AT_PLAIN,
                    "You have tells turned off... try chan +tells first", ch,
                    NULL, NULL, TO_CHAR);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) &&
            (IS_SET(ch->act, PLR_SILENCE) || IS_SET(ch->act, PLR_NO_TELL)))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Tell whom what?\n\r", ch);
                return;
        }
        sbuf = argument;

        if ((victim = get_char_world(ch, arg)) == NULL
            || IS_NPC(victim)
            || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }


        if (ch == victim)
        {
                send_to_char("You have a nice little chat with yourself.\n\r",
                             ch);
                return;
        }

        if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim))
        {
                send_to_char
                        ("They can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IS_NPC(victim) && (victim->switched) &&
            (get_trust(ch) > LEVEL_AVATAR) &&
            !IS_SET(victim->switched->act, ACT_POLYMORPHED) &&
            !IS_AFFECTED(victim->switched, AFF_POSSESS))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IS_NPC(victim) && (victim->switched) &&
                 (IS_SET(victim->switched->act, ACT_POLYMORPHED) ||
                  IS_AFFECTED(victim->switched, AFF_POSSESS)))
                victim = victim->switched;
        else if (!IS_NPC(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!has_comlink(ch) && !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
        {
                send_to_char("You do not have a comlink.", ch);
                return;
        }

        if (!has_comlink(victim) && !IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
        {
                send_to_char("They don't have a comlink.", ch);
                return;
        }


        if (IS_SET(victim->pcdata->flags, PCFLAG_WORKING)
            && get_trust(ch) <= get_trust(victim))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }


        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_AFK)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, CHANNEL_TELLS)
            && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_SILENCE)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
            || (!IS_NPC(victim)
                && xIS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
        {
                act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == CON_EDITING
            && get_trust(ch) < LEVEL_GOD)
        {
                act(AT_PLAIN,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, TO_CHAR);
                return;
        }


        if (IS_MXP(ch))
                act(AT_TELL,
                    "You tell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " '$t'", ch, argument, victim, TO_CHAR);
        else
                act(AT_TELL, "You tell $N '$t'", ch, argument, victim,
                    TO_CHAR);

        position = victim->position;
        victim->position = POS_STANDING;
        if (!IS_IMMORTAL(victim) && !knows_language(victim, ch->speaking, ch)
            && (!IS_NPC(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        if (IS_MXP(victim))
                act(AT_TELL,
                    MXPTAG("player $g") "$n" MXPTAG("/player")
                    " tells you '$t'", ch, drunk_speech(sbuf, ch), victim,
                    TO_VICT);
        else
                act(AT_TELL, "$n tells you '$t'", ch, drunk_speech(sbuf, ch),
                    victim, TO_VICT);

        victim->position = position;
        victim->reply = ch;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                char      buf[MAX_INPUT_LENGTH];

                snprintf(buf, MSL, "%s: %s (tell to) %s.",
                         IS_NPC(ch) ? ch->short_descr : ch->name,
                         argument,
                         IS_NPC(victim) ? victim->short_descr : victim->name);
                append_to_file(LOG_FILE, buf);
        }
        mprog_speech_trigger(argument, ch);
        return;
}



CMDF do_reply(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       position;
        char     *sbuf = argument;


        xREMOVE_BIT(ch->deaf, CHANNEL_TELLS);
        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
        {
                send_to_char("Your message didn't get through.\n\r", ch);
                return;
        }


        if (((victim = ch->reply) == NULL) || IS_NPC(victim))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && (victim->switched)
            && can_see(ch, victim) && (get_trust(ch) > LEVEL_AVATAR))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IS_NPC(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_AFK)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_WORKING))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }


        if (xIS_SET(victim->deaf, CHANNEL_TELLS)
            && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
            || (!IS_NPC(victim)
                && xIS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
        {
                act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
                return;
        }

        if (!IS_IMMORTAL(victim) && !knows_language(victim, ch->speaking, ch)
            && (!IS_NPC(ch) || ch->speaking != 0))
                sbuf = scramble(argument, ch->speaking);

        if (IS_MXP(ch))
                act(AT_TELL,
                    "You tell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " '$t'", ch, argument, victim, TO_CHAR);
        else
                act(AT_TELL, "You tell $N '$t'", ch, argument, victim,
                    TO_CHAR);
        position = victim->position;
        victim->position = POS_STANDING;
        if (IS_MXP(victim))
                act(AT_TELL,
                    MXPTAG("player $g") "$n" MXPTAG("/player")
                    " tells you '$t'", ch, drunk_speech(sbuf, ch), victim,
                    TO_VICT);
        else
                act(AT_TELL, "$n tells you '$t'", ch,
                    drunk_speech(sbuf, ch), victim, TO_VICT);
        victim->position = position;
        victim->reply = ch;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf, MSL, "%s: %s (reply to) %s.",
                         IS_NPC(ch) ? ch->short_descr : ch->name,
                         argument,
                         IS_NPC(victim) ? victim->short_descr : victim->name);
                append_to_file(LOG_FILE, buf);
        }

        return;
}

CMDF do_otell(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       position;
        CHAR_DATA *switched_victim;

        switched_victim = NULL;

        if (xIS_SET(ch->deaf, CHANNEL_TELLS) && !IS_IMMORTAL(ch))
        {
                send_to_char
                        ("You have tells turned off... try chan +tells first",
                         ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) &&
            (IS_SET(ch->act, PLR_SILENCE) || IS_SET(ch->act, PLR_NO_TELL)))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("OTell whom what?\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg)) == NULL
            || IS_NPC(victim)
            || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char
                        ("You have a nice little out of character chat with yourself.\n\r",
                         ch);
                return;
        }

        if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim))
        {
                send_to_char
                        ("They can't hear you because you are not authorized.\n\r",
                         ch);
                return;
        }

        if (!IS_NPC(victim) && (victim->switched)
            && (get_trust(ch) > LEVEL_AVATAR)
            && !IS_SET(victim->switched->act, ACT_POLYMORPHED)
            && !IS_AFFECTED(victim->switched, AFF_POSSESS))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }

        else if (!IS_NPC(victim) && (victim->switched)
                 && (IS_SET(victim->switched->act, ACT_POLYMORPHED)
                     || IS_AFFECTED(victim->switched, AFF_POSSESS)))
                switched_victim = victim->switched;

        else if (!IS_NPC(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_AFK)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }
        if (IS_SET(victim->pcdata->flags, PCFLAG_WORKING))
        {
                send_to_char
                        ("That person is working right now. Don't bother them.\n\r",
                         ch);
                return;
        }

        if (xIS_SET(victim->deaf, CHANNEL_TELLS)
            && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
                    TO_CHAR_OOC);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_SILENCE)))
        {
                send_to_char
                        ("That player is silenced.  They will receive your message but can not respond.\n\r",
                         ch);
        }

        if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
            || (!IS_NPC(victim)
                && xIS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
        {
                act(AT_PLAIN, "$E can't hear you.", ch, 0, victim,
                    TO_CHAR_OOC);
                return;
        }

        if (victim->desc    /* make sure desc exists first  -Thoric */
            && victim->desc->connected == CON_EDITING
            && get_trust(ch) < LEVEL_GOD)
        {
                act(AT_PLAIN,
                    "$E is currently in a writing buffer.  Please try again in a few minutes.",
                    ch, 0, victim, TO_CHAR_OOC);
                return;
        }

        if (is_ignoring(victim, ch))
        {
                snprintf(buf, MSL, "%s is ignoring you.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
        }

        if (switched_victim)
                victim = switched_victim;

        position = victim->position;
        victim->position = POS_STANDING;
        if (IS_MXP(ch))
                act(AT_TELL,
                    "&zYou Otell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " &B'&w$t&B'", ch, argument, victim, TO_CHAR_OOC);
        else
                act(AT_TELL, "&zYou Otell $N &B'&w$t&B'", ch, argument,
                    victim, TO_CHAR_OOC);
        if (IS_MXP(victim))
                act(AT_TELL,
                    "&B" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " &zOtells you &B'&w$t&B'", ch, argument, victim,
                    TO_VICT_OOC);
        else
                act(AT_TELL, "&B$n &zOtells you &B'&w$t&B'", ch, argument,
                    victim, TO_VICT_OOC);
        victim->position = position;
        victim->oreply = ch;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf, MSL, "%s: %s (otell to) %s.",
                         IS_NPC(ch) ? ch->short_descr : ch->name,
                         argument,
                         IS_NPC(victim) ? victim->short_descr : victim->name);
                append_to_file(LOG_FILE, buf);
        }
        return;
}

CMDF do_oreply(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       position;

        xREMOVE_BIT(ch->deaf, CHANNEL_TELLS);
        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
        {
                send_to_char("Your message didn't get through.\n\r", ch);
                return;
        }

        if (((victim = ch->oreply) == NULL) || IS_NPC(victim))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && (victim->switched)
            && can_see(ch, victim) && (get_trust(ch) > LEVEL_AVATAR))
        {
                send_to_char("That player is switched.\n\r", ch);
                return;
        }
        else if (!IS_NPC(victim) && (!victim->desc))
        {
                send_to_char("That player is link-dead.\n\r", ch);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_WORKING))
        {
                send_to_char
                        ("That person is working right now. Don't bother them!\n\r",
                         ch);
                return;
        }

        if (!IS_NPC(victim) && (IS_SET(victim->act, PLR_AFK)))
        {
                send_to_char("That player is afk.\n\r", ch);
                return;
        }

        if (xIS_SET(victim->deaf, CHANNEL_TELLS)
            && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
        {
                act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
                    TO_CHAR_OOC);
                return;
        }

        if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
            || (!IS_NPC(victim)
                && xIS_SET(victim->in_room->room_flags, ROOM_SILENCE)))
        {
                act(AT_PLAIN, "$E can't hear you.", ch, 0, victim,
                    TO_CHAR_OOC);
                return;
        }

        if (is_ignoring(victim, ch))
        {
                snprintf(buf, MSL, "%s is ignoring you.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
        }
        position = victim->position;
        victim->position = POS_STANDING;
        if (IS_MXP(ch))
                act(AT_TELL,
                    "&zYou Otell " MXPTAG("player $G") "$N" MXPTAG("/player")
                    " &B'&w$t&B'", ch, argument, victim, TO_CHAR_OOC);
        else
                act(AT_TELL, "&zYou Otell $N &B'&w$t&B'", ch, argument,
                    victim, TO_CHAR_OOC);
        if (IS_MXP(victim))
                act(AT_TELL,
                    "&B" MXPTAG("player $g") "$n" MXPTAG("/player")
                    " &zOtells you &B'&w$t&B'", ch, argument, victim,
                    TO_VICT_OOC);
        else
                act(AT_TELL, "&B$n &zOtells you &B'&w$t&B'", ch, argument,
                    victim, TO_VICT_OOC);
        victim->position = position;
        victim->oreply = ch;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf, MSL, "%s: %s (oreply to) %s.",
                         IS_NPC(ch) ? ch->short_descr : ch->name,
                         argument,
                         IS_NPC(victim) ? victim->short_descr : victim->name);
                append_to_file(LOG_FILE, buf);
        }

        return;
}

CMDF do_emote(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        int       actflags;
        char     *plast;

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE))
        {
                send_to_char("You can't show your emotions.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Emote what?\n\r", ch);
                return;
        }

        actflags = ch->act;
        if (IS_NPC(ch))
                REMOVE_BIT(ch->act, ACT_SECRETIVE);
        for (plast = argument; *plast != '\0'; plast++);

        mudstrlcpy(buf, argument, MSL);
        if (isalpha(plast[-1]))
                mudstrlcat(buf, ".", MSL);

        MOBtrigger = FALSE;
        if (buf[0] == 's' && buf[1] == ' ') 
        {
                act(AT_SOCIAL, "$n'$T", ch, NULL, buf, TO_ROOM);
                act(AT_SOCIAL, "$n'$T", ch, NULL, buf, TO_CHAR);
        }
        else {
                act(AT_SOCIAL, "$n $T", ch, NULL, buf, TO_ROOM);
                act(AT_SOCIAL, "$n $T", ch, NULL, buf, TO_CHAR);
        }
        ch->act = actflags;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf, MSL, "%s %s (emote)",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LOG_FILE, buf);
        }
        return;
}


CMDF do_bug(CHAR_DATA * ch, char *argument)
{
        set_char_color(AT_PLAIN, ch);
        if (argument[0] == '\0')
        {
                send_to_char("\n\rUsage:  'bug <message>'\n\r", ch);
                if (get_trust(ch) >= LEVEL_ASCENDANT)
                        send_to_char
                                ("Usage:  'bug list' or 'bug clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now")
            && get_trust(ch) >= LEVEL_ASCENDANT)
        {
                FILE     *fp = fopen(BUGS_FILE, "w");

                if (fp)
                        FCLOSE(fp);
                send_to_char("Bug file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list"))
        {
                if (get_trust(ch) >= LEVEL_ASCENDANT)
                {
                        send_to_char("\n\r VNUM \n\r.......\n\r", ch);
                        show_file(ch, BUGS_FILE);
                }
                else
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }
        }
        else
        {
                append_file(ch, BUGS_FILE, smash_color((argument)));
                send_to_char("Ok, thanks..\n\r", ch);
        }
        return;
}

CMDF do_idea(CHAR_DATA * ch, char *argument)
{
        set_char_color(AT_PLAIN, ch);
        if (argument[0] == '\0')
        {
                send_to_char("\n\rUsage:  'idea <message>'\n\r", ch);
                if (get_trust(ch) >= LEVEL_ASCENDANT)
                        send_to_char
                                ("Usage:  'idea list' or 'idea clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now")
            && get_trust(ch) >= LEVEL_ASCENDANT)
        {
                FILE     *fp = fopen(IDEA_FILE, "w");

                if (fp)
                        FCLOSE(fp);
                send_to_char("Idea file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list"))
        {
                if (get_trust(ch) >= LEVEL_ASCENDANT)
                {
                        send_to_char("\n\r VNUM \n\r.......\n\r", ch);
                        show_file(ch, IDEA_FILE);
                }
                else
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }
        }
        else
        {
                append_file(ch, IDEA_FILE, smash_color((argument)));
                send_to_char("Ok, thanks..\n\r", ch);
        }
        return;
}




CMDF do_typo(CHAR_DATA * ch, char *argument)
{
        set_char_color(AT_PLAIN, ch);
        if (argument[0] == '\0')
        {
                send_to_char
                        ("\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r",
                         ch);
                if (get_trust(ch) >= LEVEL_ASCENDANT)
                        send_to_char
                                ("Usage:  'typo list' or 'typo clear now'\n\r",
                                 ch);
                return;
        }
        if (!str_cmp(argument, "clear now") && get_trust(ch) >= LEVEL_SUPREME)
        {
                FILE     *fp = fopen(TYPO_FILE, "w");

                if (fp)
                        FCLOSE(fp);
                send_to_char("Typo file cleared.\n\r", ch);
                return;
        }
        if (!str_cmp(argument, "list") && get_trust(ch) >= LEVEL_ASCENDANT)
        {
                send_to_char("VNUM \n\r.......\n\r", ch);
                show_file(ch, TYPO_FILE);
        }
        else
        {
                append_file(ch, TYPO_FILE, smash_color((argument)));
                send_to_char("Ok, thanks.\n\r", ch);
        }
        return;
}

CMDF do_qui(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        set_char_color(AT_RED, ch);
        send_to_char("If you want to QUIT, you have to spell it out.\n\r",
                     ch);
        return;
}

CMDF do_quit(CHAR_DATA * ch, char *argument)
{

        char      buf[MAX_INPUT_LENGTH];

/*   int x, y; */
        int       level;

        argument = NULL;

        if (IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
        {
                send_to_char("You can't quit while polymorphed.\n\r", ch);
                return;
        }

        if (IS_NPC(ch))
                return;

        if (ch->position == POS_FIGHTING)
        {
                set_char_color(AT_RED, ch);
                send_to_char("No way! You are fighting.\n\r", ch);
                return;
        }

        if (ch->position < POS_STUNNED)
        {
                set_char_color(AT_BLOOD, ch);
                send_to_char("You're not DEAD yet.\n\r", ch);
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

        set_char_color(AT_WHITE, ch);
        send_to_char
                ("You close your eyes, and you slowly get tired. Lights change and swirl\n\ras your consiousness floats away.\n\r\n\r",
                 ch);
        act(AT_SAY,
            "You feel someone or something calling to you, '$n... come back.\n\rYou're our only hope'",
            ch, NULL, NULL, TO_CHAR);
        act(AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM);
        set_char_color(AT_GREY, ch);
        if (!IS_SET(ch->act, PLR_WIZINVIS) && ch->desc)
        {
                snprintf(buf, MSL, "%s has left %s", ch->name,
                         sysdata.mud_name);
                info_chan(buf);
        }
        snprintf(log_buf, MSL, "%s has quit.", ch->name);
        quitting_char = ch;
        save_char_obj(ch);
        save_home(ch);
        saving_char = NULL;

        level = get_trust(ch);
        /*
         * After extract_char the ch is no longer valid!
         */
#ifdef ACCOUNT
        if (!IS_NPC(ch) && ch->pcdata && ch->pcdata->account)
        {
                free_account(ch->pcdata->account);
                ch->pcdata->account = NULL;
                if (ch->desc && ch->desc->account)
                        ch->desc->account = NULL;
        }
#endif
        extract_char(ch, TRUE);
/*    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL; - WTF */

        /*
         * don't show who's logging off to leaving player 
         */
        log_string_plus(log_buf, LOG_COMM, level);
        return;
}

CMDF do_ansi(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("ANSI ON or OFF?\n\r", ch);
                return;
        }
        if ((strcmp(arg, "on") == 0) || (strcmp(arg, "ON") == 0))
        {
                SET_BIT(ch->act, PLR_ANSI);
                set_char_color(AT_WHITE + AT_BLINK, ch);
                send_to_char("ANSI ON!!!\n\r", ch);
                return;
        }

        if ((strcmp(arg, "off") == 0) || (strcmp(arg, "OFF") == 0))
        {
                REMOVE_BIT(ch->act, PLR_ANSI);
                send_to_char("Okay... ANSI support is now off\n\r", ch);
                return;
        }
}

CMDF do_save(CHAR_DATA * ch, char *argument)
{
        bool      silent = FALSE;

        if (!str_cmp(argument, "-silentsave"))
                silent = TRUE;
        argument = NULL;
        if (IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
        {
                send_to_char("You can't save while polymorphed.\n\r", ch);
                return;
        }

        if (IS_NPC(ch))
                return;

        if (!IS_SET(ch->affected_by, ch->race->affected()))
                SET_BIT(ch->affected_by, ch->race->affected());

        if (NOT_AUTHED(ch))
        {
                send_to_char
                        ("You can't save until after you've graduated from the acadamey.\n\r",
                         ch);
                return;
        }
        if (IS_IMMORTAL(ch) && ch->pcdata && ch->pcdata->area
            && IS_SET(ch->pcdata->area->status, AREA_LOADED))
                fold_area(ch->pcdata->area, ch->pcdata->area->filename, FALSE,
                          FALSE);
        save_char_obj(ch);
        save_home(ch);
#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->account)
                save_account(ch->pcdata->account);
#endif
        saving_char = NULL;
        save_finger(ch);
        if (silent == FALSE)
                send_to_char("Ok.\n\r", ch);

        return;
}

void auto_save(CHAR_DATA * ch)
{
        if (IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
        {
                return;
        }

        if (IS_NPC(ch))
                return;

        if (!IS_SET(ch->affected_by, ch->race->affected()))
                SET_BIT(ch->affected_by, ch->race->affected());

        if (NOT_AUTHED(ch))
        {
                return;
        }
        save_char_obj(ch);
#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->account)
                save_account(ch->pcdata->account);
#endif
        save_home(ch);
        saving_char = NULL;
        save_finger(ch);
        return;
}



/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow(CHAR_DATA * ch, CHAR_DATA * victim)
{
        CHAR_DATA *tmp;

        for (tmp = victim; tmp; tmp = tmp->master)
                if (tmp == ch)
                        return TRUE;
        return FALSE;
}


CMDF do_follow(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Follow whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master)
        {
                act(AT_PLAIN, "But you'd rather follow $N!", ch, NULL,
                    ch->master, TO_CHAR);
                return;
        }

        if (victim == ch)
        {
                if (!ch->master)
                {
                        send_to_char("You already follow yourself.\n\r", ch);
                        return;
                }
                stop_follower(ch);
                return;
        }

        if (circle_follow(ch, victim))
        {
                send_to_char
                        ("Following in loops is not allowed... sorry.\n\r",
                         ch);
                return;
        }

        if (ch->master)
                stop_follower(ch);

        add_follower(ch, victim);
        return;
}



void add_follower(CHAR_DATA * ch, CHAR_DATA * master)
{
        if (ch->master)
        {
                bug("Add_follower: non-null master.", 0);
                return;
        }

        ch->master = master;
        ch->leader = NULL;

        if (can_see(master, ch))
                act(AT_ACTION, "$n now follows you.", ch, NULL, master,
                    TO_VICT);

        act(AT_ACTION, "You now follow $N.", ch, NULL, master, TO_CHAR);

        return;
}



void stop_follower(CHAR_DATA * ch)
{
        if (!ch->master)
        {
                bug("Stop_follower: null master.", 0);
                return;
        }

        if (IS_AFFECTED(ch, AFF_CHARM))
        {
                REMOVE_BIT(ch->affected_by, AFF_CHARM);
                affect_strip(ch, gsn_charm_person);
        }

        if (can_see(ch->master, ch) && !char_died(ch->master))
                act(AT_ACTION, "$n stops following you.", ch, NULL,
                    ch->master, TO_VICT);
        act(AT_ACTION, "You stop following $N.", ch, NULL, ch->master,
            TO_CHAR);

        ch->master = NULL;
        ch->leader = NULL;
        if (ch->holding)
        {
                ch->holding->heldby = NULL;
                ch->holding->held = FALSE;
        }
        ch->holding = NULL;
        return;
}



void die_follower(CHAR_DATA * ch)
{
        CHAR_DATA *fch;

        if (ch->master)
                stop_follower(ch);

        ch->leader = NULL;

        CHECK_LINKS(first_char, last_char, next, prev, CHAR_DATA);
        for (fch = first_char; fch; fch = fch->next)
        {
                if (fch->master == ch)
                        stop_follower(fch);
                if (fch->leader == ch)
                        fch->leader = fch;
        }
        if (ch->holding)
        {
                ch->holding->heldby = NULL;
                ch->holding->held = FALSE;
        }
        ch->holding = NULL;
        return;
}



CMDF do_order(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      argbuf[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CHAR_DATA *och;
        CHAR_DATA *och_next;
        bool      found;
        bool      fAll;

        mudstrlcpy(argbuf, argument, MIL);
        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Order whom to do what?\n\r", ch);
                return;
        }

        if (IS_AFFECTED(ch, AFF_CHARM))
        {
                send_to_char("You feel like taking, not giving, orders.\n\r",
                             ch);
                return;
        }

        if (!str_cmp(arg, "all"))
        {
                fAll = TRUE;
                victim = NULL;
        }
        else
        {
                fAll = FALSE;
                if ((victim = get_char_room(ch, arg)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (victim == ch)
                {
                        send_to_char("Aye aye, right away!\n\r", ch);
                        return;
                }

                if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch)
                {
                        send_to_char("Do it yourself!\n\r", ch);
                        return;
                }
        }

        found = FALSE;
        for (och = ch->in_room->first_person; och; och = och_next)
        {
                och_next = och->next_in_room;

                if (IS_AFFECTED(och, AFF_CHARM)
                    && och->master == ch && (fAll || och == victim))
                {
                        found = TRUE;
                        act(AT_ACTION, "$n orders you to '$t'.", ch, argument,
                            och, TO_VICT);
                        interpret(och, argument);
                }
        }

        if (found)
        {
                snprintf(log_buf, MSL, "%s: order %s.", ch->name, argbuf);
                log_string_plus(log_buf, LOG_NORMAL, ch->top_level);
                send_to_char("Ok.\n\r", ch);
                WAIT_STATE(ch, 12);
        }
        else
                send_to_char("You have no followers here.\n\r", ch);
        return;
}

CMDF do_group(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim = NULL;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                CHAR_DATA *gch;
                CHAR_DATA *leader;

                leader = ch->leader ? ch->leader : ch;
                set_char_color(AT_GREEN, ch);
                ch_printf(ch, "%s's group:\n\r", PERS(leader, ch));

/* Changed so that no info revealed on possess */
                for (gch = first_char; gch; gch = gch->next)
                {
                        if (is_same_group(gch, ch))
                        {
                                set_char_color(AT_DGREEN, ch);
                                if (IS_AFFECTED(gch, AFF_POSSESS))
                                        ch_printf(ch,
                                                  "[%2d %s] %-16s %4s/%4s hp %4s/%4s mv %5s xp\n\r",
                                                  gch->top_level,
                                                  IS_NPC(gch) ? "Mob" : gch->
                                                  race->name(),
                                                  capitalize(PERS(gch, ch)),
                                                  "????", "????", "????",
                                                  "????", "?????");

                                else
                                        ch_printf(ch,
                                                  "[%2d %s] %-16s %4d/%4d hp %4d/%4d mv\n\r",
                                                  gch->top_level,
                                                  IS_NPC(gch) ? "Mob" : gch->
                                                  race->name(),
                                                  capitalize(PERS(gch, ch)),
                                                  gch->hit, gch->max_hit,
                                                  gch->endurance,
                                                  gch->max_endurance);
                        }
                }
                return;
        }

        if (!strcmp(arg, "disband"))
        {
                CHAR_DATA *gch;
                int       count = 0;

                if (ch->leader || ch->master)
                {
                        send_to_char
                                ("You cannot disband a group if you're following someone.\n\r",
                                 ch);
                        return;
                }

                for (gch = first_char; gch; gch = gch->next)
                {
                        if (is_same_group(ch, gch) && (ch != gch))
                        {
                                gch->leader = NULL;
                                gch->master = NULL;
                                count++;
                                send_to_char("Your group is disbanded.\n\r",
                                             gch);
                        }
                }

                if (count == 0)
                        send_to_char
                                ("You have no group members to disband.\n\r",
                                 ch);
                else
                        send_to_char("You disband your group.\n\r", ch);

                return;
        }

        if (!strcmp(arg, "all"))
        {
                CHAR_DATA *rch;
                int       count = 0;

                for (rch = ch->in_room->first_person; rch;
                     rch = rch->next_in_room)
                {
                        if (ch != rch
                            && !IS_NPC(rch)
                            && rch->master == ch
                            && !ch->master
                            && !ch->leader && !is_same_group(rch, ch))
                        {
                                rch->leader = ch;
                                count++;
                        }
                }

                if (count == 0)
                        send_to_char
                                ("You have no eligible group members.\n\r",
                                 ch);
                else
                {
                        /*
                         * FIXME - victim is un-initalized 
                         */
                        /*
                         * act( AT_ACTION, "$n groups $s followers.", ch, NULL, victim, TO_ROOM ); - Gavin. lets try this 
                         */
                        act(AT_ACTION, "$n groups $s followers.", ch, NULL,
                            NULL, TO_ROOM);
                        send_to_char("You group your followers.\n\r", ch);
                }
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch->master || (ch->leader && ch->leader != ch))
        {
                send_to_char("But you are following someone else!\n\r", ch);
                return;
        }

        if (victim->master != ch && ch != victim)
        {
                act(AT_PLAIN, "$N isn't following you.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (is_same_group(victim, ch) && ch != victim)
        {
                victim->leader = NULL;
                act(AT_ACTION, "$n removes $N from $s group.", ch, NULL,
                    victim, TO_NOTVICT);
                act(AT_ACTION, "$n removes you from $s group.", ch, NULL,
                    victim, TO_VICT);
                act(AT_ACTION, "You remove $N from your group.", ch, NULL,
                    victim, TO_CHAR);
                return;
        }

        victim->leader = ch;
        act(AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
        act(AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT);
        act(AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR);
        return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
CMDF do_split(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *gch;
        int       members;
        int       amount;
        int       share;
        int       extra;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Split how much?\n\r", ch);
                return;
        }

        amount = atoi(arg);

        if (amount < 0)
        {
                send_to_char("Your group wouldn't like that.\n\r", ch);
                return;
        }

        if (amount == 0)
        {
                send_to_char
                        ("You hand out zero credits, but no one notices.\n\r",
                         ch);
                return;
        }

        if (ch->gold < amount)
        {
                send_to_char("You don't have that many credits.\n\r", ch);
                return;
        }

        members = 0;
        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (is_same_group(gch, ch))
                        members++;
        }


        if ((IS_SET(ch->act, PLR_AUTOGOLD)) && (members < 2))
                return;

        if (members < 2)
        {
                send_to_char("Just keep it all.\n\r", ch);
                return;
        }

        share = amount / members;
        extra = amount % members;

        if (share == 0)
        {
                send_to_char("Don't even bother, cheapskate.\n\r", ch);
                return;
        }

        ch->gold -= amount;
        ch->gold += share + extra;

        set_char_color(AT_GOLD, ch);
        ch_printf(ch,
                  "You split %d credits.  Your share is %d credits.\n\r",
                  amount, share + extra);

        snprintf(buf, MSL, "$n splits %d credits.  Your share is %d credits.",
                 amount, share);

        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (gch != ch && is_same_group(gch, ch))
                {
                        act(AT_GOLD, buf, ch, NULL, gch, TO_VICT);
                        gch->gold += share;
                }
        }
        return;
}



CMDF do_gtell(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *gch;

        if (argument[0] == '\0')
        {
                send_to_char("Tell your group what?\n\r", ch);
                return;
        }

        if (IS_SET(ch->act, PLR_NO_TELL))
        {
                send_to_char("Your message didn't get through!\n\r", ch);
                return;
        }

        /*
         * Note use of send_to_char, so gtell works on sleepers.
         */

        for (gch = first_char; gch; gch = gch->next)
        {
                if (is_same_group(gch, ch))
                {
                        set_char_color(AT_GTELL, gch);
                        /*
                         * Groups unscrambled regardless of clan language.  Other languages
                         * still garble though. -- Altrag 
                         */
                        if (knows_language(gch, ch->speaking, gch)
                            || (IS_NPC(ch) && !ch->speaking))
                                ch_printf(gch, "%s tells the group '%s'.\n\r",
                                          ch->name, argument);
                        else
                                ch_printf(gch, "%s tells the group '%s'.\n\r",
                                          ch->name, scramble(argument,
                                                             ch->speaking));
                }
        }

        return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CHAR_DATA * ach, CHAR_DATA * bch)
{
        if (ach->leader)
                ach = ach->leader;
        if (bch->leader)
                bch = bch->leader;
        return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction(char *argument)
{
        DESCRIPTOR_DATA *d;
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *original;

        snprintf(buf, MSL, "Auction: %s", argument);    /* last %s to reset color */

        for (d = first_descriptor; d; d = d->next)
        {
                original = d->original ? d->original : d->character;    /* if switched */
                if (IS_PLAYING(d) && !xIS_SET(original->deaf, CHANNEL_AUCTION)
                    && !xIS_SET(original->in_room->room_flags, ROOM_SILENCE)
                    && !NOT_AUTHED(original))
                        act(AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR);
        }
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 */
bool knows_language(CHAR_DATA * ch, LANGUAGE_DATA * lang, CHAR_DATA * cch)
{
        sh_int    sn;

        if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        {
                return TRUE;
        }

        if (IS_IMMORTAL(ch) || IS_IMMORTAL(cch))
                return TRUE;

/* Hack for OLC Languages, force mobs to understand everything */
/* Allows certain mobs to speak EVERYTHING, like in the academy for newbies. */
        if (IS_NPC(ch) || (IS_NPC(cch) && IS_SET(cch->act, ACT_SPEAKSALL)))
        {
                return TRUE;
        }
        if (!str_cmp(lang->name, "clan"))
        {
                /*
                 * Clan = basic for mobs.. snicker.. -- Altrag 
                 */
                if (IS_NPC(ch) || IS_NPC(cch))
                        return TRUE;
                if (ch->pcdata->clan == cch->pcdata->clan &&
                    ch->pcdata->clan != NULL)
                        return TRUE;
        }
        if (!IS_NPC(ch))
        {
                /*
                 * Racial languages for PCs 
                 */
                /*
                 * Major Overhaul by Greven for OLC languages 
                 */
                if (ch->race->language() == lang)
                        return TRUE;
                if (ch->speaking == lang)
                        return TRUE;

                if ((sn = skill_lookup(lang->name)) != -1
                    && ch->pcdata->learned[sn] >= 60)
                        return TRUE;
        }
        return FALSE;
}

bool can_learn_lang(CHAR_DATA * ch, LANGUAGE_DATA * lang)
{
        if (!str_cmp(lang->name, "clan"))
                return FALSE;
        if (IS_NPC(ch))
                return FALSE;
        {
                int       sn;

                if ((sn = skill_lookup(lang->name)) < 0)
                {
                        bug("Can_learn_lang: valid language without sn: %s",
                            lang->name);
                }
                if (ch->pcdata->learned[sn] >= 99)
                        return FALSE;
        }

        return TRUE;
}

char     *const lang_names[] =
        { "basic", "wookiee", "twilek", "rodian", "hutt",
        "mon calamari", "noghri", "gamorrean",
        "jawa", "adarian", "ewok", "verpine", "defel",
        "trandoshan", "shistavanan", "binary", "duinuogwuin", "csillian",
        "kel dor",
        "bothan", "barabel", "ithorian", "devaronian", "durosian", "gotal",
        "talzzi", "ho'din",
        "falleen", "givin", "clan", ""
};

CMDF do_speak(CHAR_DATA * ch, char *argument)
{
        LANGUAGE_DATA *language;
        char      arg[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "all") && IS_IMMORTAL(ch))
        {
                set_char_color(AT_SAY, ch);
                send_to_char("Now speaking all languages.\n\r", ch);
                return;
        }
        if (!str_prefix(arg, "basic")
            && !str_cmp(ch->race->name(), "wookiee"))
        {
                set_char_color(AT_SAY, ch);
                send_to_char
                        ("Wookiees cannot speak basic even though some can understand it.\n\r",
                         ch);
                return;
        }
        if (!str_prefix(arg, "twilek")
            && str_cmp(ch->race->name(), "twi'lek"))
        {
                set_char_color(AT_SAY, ch);
                send_to_char
                        ("To speak the Twi'lek language requires body parts that you don't have.\n\r",
                         ch);
                return;
        }
        if (!str_prefix(arg, "binary") && str_cmp(ch->race->name(), "droid"))
        {
                set_char_color(AT_SAY, ch);
                send_to_char
                        ("To speak binary, your brain would have to be a billion times faster.\n\r",
                         ch);
                return;
        }
        for (language = first_language; language; language = language->next)
        {
                if (!str_prefix(arg, language->name))
                {
                        if (knows_language(ch, language, ch))
                        {
                                if (!str_cmp(language->name, "clan") &&
                                    (IS_NPC(ch) || !ch->pcdata->clan))
                                        continue;
                                ch->speaking = language;
                                set_char_color(AT_SAY, ch);
                                ch_printf(ch, "You now speak %s.\n\r",
                                          language->name);
                                return;
                        }
                }
        }
        set_char_color(AT_SAY, ch);
        send_to_char("You do not know that language.\n\r", ch);
}

CMDF do_languages(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        LANGUAGE_DATA *language = NULL, *lang2 = NULL;
        int       sn2, sn;
        int       numlang = 0;
        CHAR_DATA *victim = NULL;

        if (IS_NPC(ch))
                return;
        argument = one_argument(argument, arg);
        if (arg[0] != '\0' && !str_prefix(arg, "learn") &&
            !IS_IMMORTAL(ch) && !IS_NPC(ch))
        {
                CHAR_DATA *sch;
                char      arg2[MAX_INPUT_LENGTH];
                int       prct;

                argument = one_argument(argument, arg2);
                if (arg2[0] == '\0')
                {
                        send_to_char("Learn which language?\n", ch);
                        return;
                }

                if (!str_prefix(arg2, "clan"))
                {
                        send_to_char("Can't learn clan\n", ch);
                        return;
                }

                for (lang2 = first_language; lang2; lang2 = lang2->next)
                {
                        if (!str_cmp(lang2->name, "clan"))
                                continue;

                        if (!str_prefix(arg2, lang2->name))
                                language = lang2;

                        if (str_prefix(arg2, lang2->name) &&
                            (sn2 = skill_lookup(lang2->name)) != -1)
                        {
                                if (ch->pcdata->learned[sn2] >= 1)
                                        numlang++;
                        }
                }

                if (numlang >= max_languages(ch))
                {
                        send_to_char
                                ("You are not smart enough to learn any more languages.\n\r",
                                 ch);
                        return;
                }
                if (language == NULL)
                {
                        send_to_char("That is not a language.\n\r", ch);
                        return;
                }
                if ((sn = skill_lookup(language->name)) < 0)
                {
                        send_to_char("That is not a language.\n\r", ch);
                        return;
                }
                if (ch->race->language() == language ||
                    ch->pcdata->learned[sn] >= 99)
                {
                        act(AT_PLAIN, "You are already fluent in $t.", ch,
                            language->name, NULL, TO_CHAR);
                        return;
                }
                for (sch = ch->in_room->first_person; sch;
                     sch = sch->next_in_room)
                        if (IS_NPC(sch) && IS_SET(sch->act, ACT_SCHOLAR))
                                break;
                if (!sch)
                {
                        send_to_char
                                ("There is no one who can teach that language here.\n\r",
                                 ch);
                        return;
                }
                if (ch->gold < 25)
                {
                        send_to_char
                                ("language lessons cost 25 credits... you don't have enough.\n\r",
                                 ch);
                        return;
                }
                ch->gold -= 25;
                /*
                 * Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag 
                 */
                prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);
                ch->pcdata->learned[sn] += prct;
                ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 99);
                if (ch->pcdata->learned[sn] == prct)
                        act(AT_PLAIN, "You begin lessons in $t.", ch,
                            language->name, NULL, TO_CHAR);
                else if (ch->pcdata->learned[sn] < 60)
                        act(AT_PLAIN, "You continue lessons in $t.", ch,
                            language->name, NULL, TO_CHAR);
                else if (ch->pcdata->learned[sn] < 60 + prct)
                        act(AT_PLAIN,
                            "You feel you can start communicating in $t.", ch,
                            language->name, NULL, TO_CHAR);
                else if (ch->pcdata->learned[sn] < 99)
                        act(AT_PLAIN, "You become more fluent in $t.", ch,
                            language->name, NULL, TO_CHAR);
                else
                        act(AT_PLAIN, "You now speak perfect $t.", ch,
                            language->name, NULL, TO_CHAR);
                return;
        }

        if (arg && arg[0] != '\0')
                victim = get_char_world(ch, arg);
        if (!victim || !IS_IMMORTAL(ch))
                victim = ch;
        if (IS_NPC(victim))
                return;

        for (language = first_language; language; language = language->next)
        {
                if (!str_cmp(language->name, "clan"))
                        continue;
                if ((sn = skill_lookup(language->name)) < 0)
                        send_to_char("&B(&w  0&B) ", ch);
                else
                        ch_printf(ch, "&B(&w%3d&B) ",
                                  victim->pcdata->learned[sn]);
                if (victim->speaking == language)
                        ch_printf(ch, "&R%s", capitalize(language->name));
                else
                        ch_printf(ch, "&B%c&z%s", UPPER(language->name[0]),
                                  (language->name + 1));
                send_to_char("\n\r", ch);
        }
        send_to_char("\n\r", ch);
        return;
}

bool is_ignoring(CHAR_DATA * ch, CHAR_DATA * victim)
{
        int       pos;
        CHAR_DATA *rch;

        if (ch->desc == NULL)
                rch = ch;
        else
                rch = ch->desc->original ? ch->desc->original : ch;

        if (IS_NPC(rch) || IS_NPC(victim))
                return FALSE;

        for (pos = 0; pos < MAX_IGNORE; pos++)
        {
                if (!IS_NPC(rch))
                        if (rch->pcdata->ignore[pos] == NULL)
                                break;

                if (!str_cmp(rch->pcdata->ignore[pos], victim->name))
                        return TRUE;
        }

        return FALSE;
}

CMDF do_ignore(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim, *rch;
        int       pos;

        if (ch->desc == NULL)
                rch = ch;
        else
                rch = ch->desc->original ? ch->desc->original : ch;

        if (IS_NPC(rch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("Who do you want to ignore?\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "list"))
        {
                for (pos = 0; pos < MAX_IGNORE; pos++)
                {
                        if (rch->pcdata->ignore[pos] == NULL)
                                break;
                        ch_printf(ch, "&C[&c%d&C] &G%s&w\n\r", pos,
                                  rch->pcdata->ignore[pos]);
                }
                return;
        }


        if ((victim = get_char_world(rch, argument)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Ignore a mob?  I don't think so.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char
                        ("I don't think you really want to ignore yourself.\n\r",
                         ch);
                return;
        }

        for (pos = 0; pos < MAX_IGNORE; pos++)
        {
                if (rch->pcdata->ignore[pos] == NULL)
                        break;

                if (!str_cmp(argument, rch->pcdata->ignore[pos]))
                {
                        STRFREE(rch->pcdata->ignore[pos]);
                        rch->pcdata->ignore[pos] = NULL;
                        ch_printf(ch, "You stop ignoring %s.\n\r",
                                  victim->name);
                        return;
                }
        }

        if (pos >= MAX_IGNORE)
        {
                send_to_char("You can't ignore anymore people\n\r", ch);
                return;
        }

        rch->pcdata->ignore[pos] = STRALLOC(argument);
        ch_printf(ch, "You now ignore %s.\n\r", victim->name);
        return;

}

/* tunes a characters comlink to a certain channel */
CMDF do_tune(CHAR_DATA * ch, char *argument)
{

        char      arg[MAX_INPUT_LENGTH];
        int       station;

        if (IS_NPC(ch))
                return;

        if (!has_comlink(ch))
        {
                send_to_char("&RYou don't have a comlink to set!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                ch_printf(ch, "&BYour comlink display reads&G&W %d &B.\n\r",
                          ch->pcdata->comchan);
                return;
        }

        argument = one_argument(argument, arg);
        station = atoi(arg);

        if (station > 101 || station < 0)
        {
                send_to_char("&RAvailable stations are 0 through 100.\n\r",
                             ch);
                return;
        }

        if (station == 101 && !IS_IMMORTAL(ch))
        {
                send_to_char("&RAvailable stations are 0 through 100.\n\r",
                             ch);
                return;
        }

        ch_printf(ch, "&BYour comlink clicks as you set it to &W%d&B.\n\r",
                  station);
        act(AT_SOCIAL, "&B$n fiddles with a comlink for a second.", ch, NULL,
            NULL, TO_ROOM);

        ch->pcdata->comchan = station;
        return;
}

/* sends a message to the station your tuned too */
CMDF do_talk(CHAR_DATA * ch, char *argument)
{
        DESCRIPTOR_DATA *d;
        int       station;
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(ch))
                return;

        if (!has_comlink(ch))
        {
                send_to_char("&RYou don't seem to have a comlink!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Talk what?\n\r", ch);
                return;
        }

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
        {
                send_to_char("Your silenced.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        station = ch->pcdata->comchan;

        if (station <= 0)
        {
                send_to_char("&RYour comlink is not tuned to a station!\n\r",
                             ch);
                return;
        }
        ch_printf(ch,
                  "&B[&Woutgoing message&B]&W:&B [&W%d&B] &Wreads&B, &W'%s'\n\r",
                  station, drunk_speech(argument, ch));

        snprintf(buf, MSL,
                 "&B[&Wincoming message&B]&W:&B [&W%d&B/&W$n&B] &Wreads&B, &W'$t'",
                 station);
        for (d = first_descriptor; d; d = d->next)
        {
                CHAR_DATA *och;
                CHAR_DATA *vch;

                och = d->original ? d->original : d->character;
                vch = d->character;

                if (IS_PLAYING(d) && vch != ch && !IS_NPC(vch))
                {
                        char     *sbuf = argument;

                        if (!has_comlink(vch))
                                continue;

                        if (xIS_SET(vch->in_room->room_flags, ROOM_SILENCE))
                                continue;

                        if (vch->pcdata && vch->pcdata->comchan != station
                            && vch->pcdata->comchan != 101)
                                continue;

                        if (!knows_language(vch, ch->speaking, ch) &&
                            (!IS_NPC(ch) || ch->speaking != 0))
                                sbuf = scramble(argument, ch->speaking);

                        act(AT_GOSSIP, buf, ch, drunk_speech(sbuf, ch), vch,
                            TO_VICT);
                }
        }

        return;
}

/*
 * Just pop this into act_comm.c somewhere. (Or anywhere else)
 * It's pretty much say except modified to take args.
 *
 * Written by Kratas (moon@deathmoon.com)
 */

char     *append_lang(const char *argument, CHAR_DATA * ch,
                      CHAR_DATA * victim)
{
        static char buf[MAX_INPUT_LENGTH * 2];

        buf[0] = '\0';
        if (!IS_NPC(victim) && !IS_SET(victim->act, PLR_BRIEF)
            && knows_language(victim, ch->speaking, ch))
        {
                snprintf(buf, MSL, "(%s) ", ch->speaking->name);
        }
        mudstrlcat(buf, argument, MIL * 2);
        return buf;
}


CMDF do_say_to_char(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH], _last_char;
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *vch;
        CHAR_DATA *victim;
        int       actflags;
        int       arglen;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Say what to whom?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, arg)) == NULL
            || (IS_NPC(victim) && victim->in_room != ch->in_room)
            || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = strlen(argument) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;
        if (IS_NPC(ch))
                REMOVE_BIT(ch->act, ACT_SECRETIVE);
        MOBtrigger = FALSE;
        {
                char     *sbuf = argument;

                if (!knows_language(victim, ch->speaking, ch)
                    && (!IS_NPC(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);

                switch (_last_char)
                {
                case '?':
                        act(AT_SAY, "You ask $N, '$t&c'", ch,
                            drunk_speech(argument, ch), victim, TO_CHAR);
                        act(AT_SAY, "$n asks you '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, TO_VICT);
                        break;

                case '!':
                        act(AT_SAY, "You exclaim at $N, '$t&c'", ch,
                            drunk_speech(argument, ch), victim, TO_CHAR);
                        act(AT_SAY, "$n exclaims to you, '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, TO_VICT);
                        break;

                default:
                        act(AT_SAY, "You say to $N '$t&c'", ch,
                            drunk_speech(argument, ch), victim, TO_CHAR);
                        act(AT_SAY, "$n says to you '$t&c'", ch,
                            drunk_speech(sbuf, ch), victim, TO_VICT);
                        break;
                }
        }

        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
        {
                char     *sbuf = argument;

                if (vch == ch || vch == victim)
                        continue;

                if (!knows_language(vch, ch->speaking, ch)
                    && (!IS_NPC(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);

                switch (_last_char)
                {
                case '?':
                        snprintf(buf, MSL, "$n asks %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AT_SAY, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;

                case '!':
                        snprintf(buf, MSL, "$n exclaims at %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AT_SAY, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;

                default:
                        snprintf(buf, MSL, "$n says to %s, '$t&c'",
                                 can_see(vch,
                                         victim) ? victim->name : "someone");
                        act(AT_SAY, buf, ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;
                }
        }

        ch->act = actflags;
        MOBtrigger = TRUE;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                snprintf(buf, MSL, "%s: %s",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LOG_FILE, buf);
        }
        mprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(argument, ch);
        return;
}

CMDF do_say(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vch;
        char      _last_char;
        int       actflags;
        int       arglen;

        if (argument[0] == '\0')
        {
                send_to_char("Say what?\n\r", ch);
                return;
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        arglen = strlen(argument) - 1;
        /*
         * Remove whitespace and tabs. 
         */
        while (argument[arglen] == ' ' || argument[arglen] == '\t')
                --arglen;
        _last_char = argument[arglen];

        actflags = ch->act;

        if (IS_NPC(ch))
                REMOVE_BIT(ch->act, ACT_SECRETIVE);
        MOBtrigger = FALSE;
        switch (_last_char)
        {
        case '?':
                act(AT_SAY, "You ask, '$t&c'", ch, drunk_speech(argument, ch),
                    ch, TO_CHAR);
                break;

        case '!':
                act(AT_SAY, "You exclaim, '$t&c'", ch,
                    drunk_speech(argument, ch), ch, TO_CHAR);
                break;

        default:
                act(AT_SAY, "You say, '$t&c'", ch, drunk_speech(argument, ch),
                    ch, TO_CHAR);
                break;
        }
        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
        {
                char     *sbuf = argument;

                if (vch == ch)
                        continue;
                if (!knows_language(vch, ch->speaking, ch) &&
                    (!IS_NPC(ch) || ch->speaking != 0))
                        sbuf = scramble(argument, ch->speaking);
                else
                {
                        ;
                }

                switch (_last_char)
                {
                case '?':
                        act(AT_SAY, "$n asks, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;

                case '!':
                        act(AT_SAY, "$n exclaims, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;

                default:
                        act(AT_SAY, "$n says, '$t&c'", ch,
                            append_lang(drunk_speech(sbuf, ch), ch, vch), vch,
                            TO_VICT);
                        break;
                }

        }


        ch->act = actflags;
        MOBtrigger = FALSE;
        if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
        {
                char      buf[MAX_STRING_LENGTH];

                snprintf(buf, MSL, "%s: %s",
                         IS_NPC(ch) ? ch->short_descr : ch->name, argument);
                append_to_file(LOG_FILE, buf);
        }
        mprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        oprog_speech_trigger(argument, ch);
        if (char_died(ch))
                return;
        rprog_speech_trigger(argument, ch);
        return;
}
