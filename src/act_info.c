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
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#if defined(__CYGWIN__)
#include <crypt.h>
#endif
#include "mud.h"
#include "mxp.h"
#include "msp.h"
#include "web-server.h"
#ifdef OLC_SHUTTLE
#include "olc-shuttle.h"
#endif
#include "editor.h"
#include "boards.h"
#include "account.h"
#include "body.h"
#include "races.h"
#include "greet.h"

char     *trim(const char *str);
void      show_visible_affects_to_char(CHAR_DATA * victim, CHAR_DATA * ch);
char     *halucinated_object(int ms, bool fShort);
ROOM_INDEX_DATA *generate_exit(ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit);
HELP_DATA *get_help(CHAR_DATA * ch, char *argument);
char     *help_fix(char *text);
int       get_comfreq(CHAR_DATA * ch);
extern int top_help;

char     *const where_name[] = {
        "<used as light>     ",
        "<worn on finger>    ",
        "<worn on finger>    ",
        "<worn around neck>  ",
        "<worn around neck>  ",
        "<worn on body>      ",
        "<worn on head>      ",
        "<worn on legs>      ",
        "<worn on feet>      ",
        "<worn on hands>     ",
        "<worn on arms>      ",
        "<energy shield>     ",
        "<worn about body>   ",
        "<worn about waist>  ",
        "<worn around wrist> ",
        "<worn around wrist> ",
        "<wielded>           ",
        "<held>              ",
        "<dual wielded>      ",
        "<worn on ears>      ",
        "<worn on eyes>      ",
        "<missile wielded>   ",
        "<bound by>          ",
        "<left holster>      ",
        "<right holster>     "
};


/*
 * Local functions.
 */
void show_char_to_char_0 args((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args((CHAR_DATA * list, CHAR_DATA * ch));
void show_ships_to_char args((SHIP_DATA * ship, CHAR_DATA * ch));
bool check_blind args((CHAR_DATA * ch));
void show_condition args((CHAR_DATA * ch, CHAR_DATA * victim));

/*Similar Helpfile Snippet Declarations*/
sh_int    str_similarity(const char *astr, const char *bstr);
sh_int    str_prefix_level(const char *astr, const char *bstr);
void      similar_help_files(CHAR_DATA * ch, char *argument);



char     *format_obj_to_char(OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
        static char buf[MAX_STRING_LENGTH];

        buf[0] = '\0';
        if (IS_OBJ_STAT(obj, ITEM_INVIS))
                mudstrlcat(buf, "(Invis) ", MSL);
        if ((IS_AFFECTED(ch, AFF_DETECT_MAGIC) || IS_IMMORTAL(ch))
            && IS_OBJ_STAT(obj, ITEM_MAGIC))
                mudstrlcat(buf, "&B(Blue Aura)&w ", MSL);
        if (IS_OBJ_STAT(obj, ITEM_GLOW))
                mudstrlcat(buf, "(Glowing) ", MSL);
        if (IS_OBJ_STAT(obj, ITEM_HUM))
                mudstrlcat(buf, "(Humming) ", MSL);
        if (IS_OBJ_STAT(obj, ITEM_HIDDEN))
                mudstrlcat(buf, "(Hidden) ", MSL);
        if (IS_OBJ_STAT(obj, ITEM_BURRIED))
                mudstrlcat(buf, "(Burried) ", MSL);
        if (IS_IMMORTAL(ch) && IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
                mudstrlcat(buf, "(PROTO) ", MSL);
        if (IS_AFFECTED(ch, AFF_DETECTTRAPS) && is_trapped(obj))
                mudstrlcat(buf, "(Trap) ", MSL);

        if (fShort)
        {
                if (obj->short_descr)
                        mudstrlcat(buf, obj->short_descr, MSL);
        }
        else
        {
                if (obj->description)
                        mudstrlcat(buf, obj->description, MSL);
        }

        return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
char     *halucinated_object(int ms, bool fShort)
{
        int       sms = URANGE(1, (ms + 10) / 5, 20);

        if (fShort)
                switch (number_range(6 - URANGE(1, sms / 2, 5), sms))
                {
                case 1:
                        return "a sword";
                case 2:
                        return "a stick";
                case 3:
                        return "something shiny";
                case 4:
                        return "something";
                case 5:
                        return "something interesting";
                case 6:
                        return "something colorful";
                case 7:
                        return "something that looks cool";
                case 8:
                        return "a nifty thing";
                case 9:
                        return "a cloak of flowing colors";
                case 10:
                        return "a mystical flaming sword";
                case 11:
                        return "a swarm of insects";
                case 12:
                        return "a deathbane";
                case 13:
                        return "a figment of your imagination";
                case 14:
                        return "your gravestone";
                case 15:
                        return "the long lost boots of Ranger Thoric";
                case 16:
                        return "a glowing tome of arcane knowledge";
                case 17:
                        return "a long sought secret";
                case 18:
                        return "the meaning of it all";
                case 19:
                        return "the answer";
                case 20:
                        return "the key to life, the universe and everything";
                default:
                        return "-error";
                }
        switch (number_range(6 - URANGE(1, sms / 2, 5), sms))
        {
        case 1:
                return "A nice looking sword catches your eye.";
        case 2:
                return "The ground is covered in small sticks.";
        case 3:
                return "Something shiny catches your eye.";
        case 4:
                return "Something catches your attention.";
        case 5:
                return "Something interesting catches your eye.";
        case 6:
                return "Something colorful flows by.";
        case 7:
                return "Something that looks cool calls out to you.";
        case 8:
                return "A nifty thing of great importance stands here.";
        case 9:
                return "A cloak of flowing colors asks you to wear it.";
        case 10:
                return "A mystical flaming sword awaits your grasp.";
        case 11:
                return "A swarm of insects buzzes in your face!";
        case 12:
                return "The extremely rare Deathbane lies at your feet.";
        case 13:
                return "A figment of your imagination is at your command.";
        case 14:
                return "You notice a gravestone here... upon closer examination, it reads your name.";
        case 15:
                return "The long lost boots of Ranger Thoric lie off to the side.";
        case 16:
                return "A glowing tome of arcane knowledge hovers in the air before you.";
        case 17:
                return "A long sought secret of all mankind is now clear to you.";
        case 18:
                return "The meaning of it all, so simple, so clear... of course!";
        case 19:
                return "The answer.  One.  It's always been One.";
        case 20:
                return "The key to life, the universe and everything awaits your hand.";
        default:
                return "-error";
        }
        return "Whoa!!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
                       bool fShowNothing)
{
        char    **prgpstrShow;
        int      *prgnShow;
        int      *pitShow;
        char     *pstrShow;
        OBJ_DATA *obj;
        int       nShow;
        int       iShow;
        int       count, offcount, tmp, ms, cnt;
        bool      fCombine;

        if (!ch->desc)
                return;

        /*
         * if there's no list... then don't do all this crap!  -Thoric
         */
        if (!list)
        {
                if (fShowNothing)
                {
                        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
                                send_to_char("     ", ch);
                        send_to_char("Nothing.\n\r", ch);
                }
                return;
        }
        /*
         * Alloc space for output lines.
         */
        count = 0;
        for (obj = list; obj; obj = obj->next_content)
                count++;

        ms = (ch->mental_state ? ch->mental_state : 1)
                *
                (IS_NPC(ch) ? 1
                 : (ch->pcdata->
                    condition[COND_DRUNK] ? (ch->pcdata->
                                             condition[COND_DRUNK] /
                                             12) : 1));

        /*
         * If not mentally stable...
         */
        if (abs(ms) > 40)
        {
                offcount = URANGE(-(count), (count * ms) / 100, count * 2);
                if (offcount < 0)
                        offcount += number_range(0, abs(offcount));
                else if (offcount > 0)
                        offcount -= number_range(0, offcount);
        }
        else
                offcount = 0;

        if (count + offcount <= 0)
        {
                if (fShowNothing)
                {
                        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
                                send_to_char("     ", ch);
                        send_to_char("Nothing.\n\r", ch);
                }
                return;
        }

        CREATE(prgpstrShow, char *, count + ((offcount > 0) ? offcount : 0));
        CREATE(prgnShow, int, count + ((offcount > 0) ? offcount : 0));
        CREATE(pitShow, int, count + ((offcount > 0) ? offcount : 0));

        nShow = 0;
        tmp = (offcount > 0) ? offcount : 0;
        cnt = 0;

        /*
         * Format the list of objects.
         */
        for (obj = list; obj; obj = obj->next_content)
        {
                if (offcount < 0 && ++cnt > (count + offcount))
                        break;
                if (tmp > 0 && number_bits(1) == 0)
                {
                        prgpstrShow[nShow] =
                                str_dup(halucinated_object(ms, fShort));
                        prgnShow[nShow] = 1;
                        pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
                        nShow++;
                        --tmp;
                }
                if (obj->wear_loc == WEAR_NONE
                    && can_see_obj(ch, obj)
                    && (obj->item_type != ITEM_TRAP
                        || IS_AFFECTED(ch, AFF_DETECTTRAPS)))
                {
                        pstrShow = format_obj_to_char(obj, ch, fShort);
                        fCombine = FALSE;

                        if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
                        {
                                /*
                                 * Look for duplicates, case sensitive.
                                 * Matches tend to be near end so run loop backwords.
                                 */
                                for (iShow = nShow - 1; iShow >= 0; iShow--)
                                {
                                        if (!strcmp
                                            (prgpstrShow[iShow], pstrShow))
                                        {
                                                prgnShow[iShow] += obj->count;
                                                fCombine = TRUE;
                                                break;
                                        }
                                }
                        }

                        pitShow[nShow] = obj->item_type;
                        /*
                         * Couldn't combine, or didn't want to.
                         */
                        if (!fCombine)
                        {
                                prgpstrShow[nShow] = str_dup(pstrShow);
                                prgnShow[nShow] = obj->count;
                                nShow++;
                        }
                }
        }
        if (tmp > 0)
        {
                int       x;

                for (x = 0; x < tmp; x++)
                {
                        prgpstrShow[nShow] =
                                str_dup(halucinated_object(ms, fShort));
                        prgnShow[nShow] = 1;
                        pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
                        nShow++;
                }
        }

        /*
         * Output the formatted list.       -Color support by Thoric
         */
        for (iShow = 0; iShow < nShow; iShow++)
        {
                switch (pitShow[iShow])
                {
                default:
                        set_char_color(AT_OBJECT, ch);
                        break;
                case ITEM_BLOOD:
                        set_char_color(AT_BLOOD, ch);
                        break;
                case ITEM_MONEY:
                case ITEM_TREASURE:
                        set_char_color(AT_YELLOW, ch);
                        break;
                case ITEM_FOOD:
                        set_char_color(AT_HUNGRY, ch);
                        break;
                case ITEM_DRINK_CON:
                case ITEM_FOUNTAIN:
                        set_char_color(AT_THIRSTY, ch);
                        break;
                case ITEM_FIRE:
                        set_char_color(AT_FIRE, ch);
                        break;
                case ITEM_SCROLL:
                case ITEM_WAND:
                case ITEM_STAFF:
                        set_char_color(AT_MAGIC, ch);
                        break;
                }
                if (fShowNothing)
                        send_to_char("     ", ch);
                send_to_char(prgpstrShow[iShow], ch);
/*	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) ) */
                {
                        if (prgnShow[iShow] != 1)
                                ch_printf(ch, " (%d)", prgnShow[iShow]);
                }

                send_to_char("\n\r", ch);
                DISPOSE(prgpstrShow[iShow]);
        }

        if (fShowNothing && nShow == 0)
        {
                if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
                        send_to_char("     ", ch);
                send_to_char("Nothing.\n\r", ch);
        }

        /*
         * Clean up.
         */
        DISPOSE(prgpstrShow);
        DISPOSE(prgnShow);
        DISPOSE(pitShow);
        return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char(CHAR_DATA * victim, CHAR_DATA * ch)
{
        char      buf[MAX_STRING_LENGTH];

        if (IS_AFFECTED(victim, AFF_SANCTUARY))
        {
                if (IS_GOOD(victim))
                {
                        set_char_color(AT_WHITE, ch);
                        ch_printf(ch,
                                  "%s glows with an aura of divine radiance.\n\r",
                                  IS_NPC(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
                else if (IS_EVIL(victim))
                {
                        set_char_color(AT_WHITE, ch);
                        ch_printf(ch,
                                  "%s shimmers beneath an aura of dark energy.\n\r",
                                  IS_NPC(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
                else
                {
                        set_char_color(AT_WHITE, ch);
                        ch_printf(ch,
                                  "%s is shrouded in flowing shadow and light.\n\r",
                                  IS_NPC(victim) ? capitalize(victim->
                                                              short_descr)
                                  : (victim->name));
                }
        }
        if (IS_AFFECTED(victim, AFF_FIRESHIELD))
        {
                set_char_color(AT_FIRE, ch);
                ch_printf(ch,
                          "%s is engulfed within a blaze of mystical flame.\n\r",
                          IS_NPC(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (IS_AFFECTED(victim, AFF_SHOCKSHIELD))
        {
                set_char_color(AT_BLUE, ch);
                ch_printf(ch,
                          "%s is surrounded by cascading torrents of energy.\n\r",
                          IS_NPC(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
/*Scryn 8/13*/
        if (IS_AFFECTED(victim, AFF_ICESHIELD))
        {
                set_char_color(AT_LBLUE, ch);
                ch_printf(ch,
                          "%s is ensphered by shards of glistening ice.\n\r",
                          IS_NPC(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (IS_AFFECTED(victim, AFF_CHARM))
        {
                set_char_color(AT_MAGIC, ch);
                ch_printf(ch, "%s looks ahead free of expression.\n\r",
                          IS_NPC(victim) ? capitalize(victim->
                                                      short_descr) : (victim->
                                                                      name));
        }
        if (!IS_NPC(victim) && !victim->desc
            && victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
        {
                set_char_color(AT_MAGIC, ch);
                mudstrlcpy(buf, PERS(victim, ch), MSL);
                mudstrlcat(buf, " appears to be in a deep trance...\n\r",
                           MSL);
        }
}

void show_char_to_char_0(CHAR_DATA * victim, CHAR_DATA * ch)
{
        char      buf[MAX_STRING_LENGTH];
        char      buf1[MAX_STRING_LENGTH];


        buf[0] = '\0';

        if (IS_NPC(victim))
                mudstrlcat(buf, " ", MSL);

        if (!IS_NPC(victim) && !victim->desc)
        {
                if (!victim->switched)
                        mudstrlcat(buf, "(Link Dead) ", MSL);
                else if (!IS_AFFECTED(victim->switched, AFF_POSSESS))
                        mudstrlcat(buf, "(Switched) ", MSL);
        }
        if (IS_NPC(victim) && ch->questmob > 0
            && victim->pIndexData->vnum == ch->questmob)
                mudstrlcat(buf, "[TARGET] ", MSL);
        if (!IS_NPC(victim) && IS_SET(victim->act, PLR_AFK))
                mudstrlcat(buf, "[AFK] ", MSL);

        if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
            || (IS_NPC(victim) && IS_SET(victim->act, ACT_MOBINVIS)))
        {
                if (!IS_NPC(victim))
                        snprintf(buf1, MSL, "(Invis %d) ",
                                 victim->pcdata->wizinvis);
                else
                        snprintf(buf1, MSL, "(Mobinvis %d) ",
                                 victim->mobinvis);
                mudstrlcat(buf, buf1, MSL);
        }
        if (IS_AFFECTED(victim, AFF_INVISIBLE))
                mudstrlcat(buf, "(Invis) ", MSL);
        if (IS_AFFECTED(victim, AFF_HIDE))
                mudstrlcat(buf, "(Hide) ", MSL);
        if (IS_AFFECTED(victim, AFF_PASS_DOOR))
                mudstrlcat(buf, "(Translucent) ", MSL);
        if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
                mudstrlcat(buf, "&P(Pink Aura)&w ", MSL);
        if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
                mudstrlcat(buf, "&R(Red Aura)&w ", MSL);
        if ((victim->perm_frc > 1 && victim->endurance > 10)
            && (IS_AFFECTED(ch, AFF_DETECT_MAGIC) || IS_IMMORTAL(ch)))
                mudstrlcat(buf, "&B(Blue Aura)&w ", MSL);
        if (!IS_NPC(victim) && IS_SET(victim->act, PLR_LITTERBUG))
                mudstrlcat(buf, "(LITTERBUG) ", MSL);
        if (IS_NPC(victim) && IS_IMMORTAL(ch)
            && IS_SET(victim->act, ACT_PROTOTYPE))
                mudstrlcat(buf, "(PROTO) ", MSL);
        if (victim->desc && victim->desc->connected == CON_EDITING)
                mudstrlcat(buf, "(Writing) ", MSL);

        set_char_color(AT_PERSON, ch);
        if (victim->position == victim->defposition
            && victim->long_descr[0] != '\0')
        {
                mudstrlcat(buf, victim->long_descr, MSL);
                send_to_char(buf, ch);
                show_visible_affects_to_char(victim, ch);
                return;
        }

        if (can_see(ch,victim)) {
                char * temp = get_char_desc(victim,ch);
                temp[0] = UPPER(temp[0]);
                mudstrlcat(buf, temp ,MSL);
        }
        else if (IS_IMMORTAL(victim)) {
                mudstrlcat(buf, "An immortal", MSL);
        }
        else {
                mudstrlcat(buf, "Someone", MSL);
        }

        switch (victim->position)
        {
        case POS_DEAD:
                mudstrlcat(buf, " is DEAD!!", MSL);
                break;
        case POS_MORTAL:
                mudstrlcat(buf, " is mortally wounded.", MSL);
                break;
        case POS_INCAP:
                mudstrlcat(buf, " is incapacitated.", MSL);
                break;
        case POS_STUNNED:
                mudstrlcat(buf, " is lying here stunned.", MSL);
                break;
        case POS_SLEEPING:
                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == SLEEP_AT)
                        {
                                mudstrlcat(buf, " &wis asleep at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SLEEP_ON)
                        {
                                mudstrlcat(buf, " &wis asleep on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SLEEP_IN)
                        {
                                mudstrlcat(buf, " &wis asleep in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                {
                        if (ch->position == POS_SITTING
                            || ch->position == POS_RESTING)
                                mudstrlcat(buf, " is sleeping nearby.", MSL);
                        else
                                mudstrlcat(buf, " is deep in slumber here.",
                                           MSL);
                }
                break;

        case POS_RESTING:

                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == REST_AT)
                        {
                                mudstrlcat(buf, " &wis resting at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == REST_ON)
                        {
                                mudstrlcat(buf, " &wis resting on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else
                        {
                                mudstrlcat(buf, " &wis resting in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                {
                        if (ch->position == POS_RESTING)
                                mudstrlcat(buf,
                                           " &wis sprawled out alongside you.",
                                           MSL);
                        else if (ch->position == POS_MOUNTED)
                                mudstrlcat(buf,
                                           " &wis sprawled out at the foot of your mount.",
                                           MSL);
                        else
                                mudstrlcat(buf, " &wis sprawled out here.",
                                           MSL);
                }
                break;
        case POS_SITTING:
                if (victim->on != NULL)
                {
                        if (victim->on->value[2] == SIT_AT)
                        {
                                mudstrlcat(buf, " &wis sitting at ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else if (victim->on->value[2] == SIT_ON)
                        {
                                mudstrlcat(buf, " &wis sitting on ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                        else
                        {
                                mudstrlcat(buf, " &wis sitting in ", MSL);
                                mudstrlcat(buf, victim->on->short_descr, MSL);
                                mudstrlcat(buf, ".", MSL);
                        }
                }
                else
                        mudstrlcat(buf, " &wis sitting here.", MSL);
                break;


        case POS_STANDING:
                if (IS_IMMORTAL(victim))
                        mudstrlcat(buf, " is here before you.", MSL);
                else if ((victim->in_room->sector_type == SECT_UNDERWATER)
                         && !IS_AFFECTED(victim, AFF_AQUA_BREATH)
                         && !IS_NPC(victim))
                        mudstrlcat(buf, " is drowning here.", MSL);
                else if (victim->in_room->sector_type == SECT_UNDERWATER)
                        mudstrlcat(buf, " is here in the water.", MSL);
                else if ((victim->in_room->sector_type == SECT_OCEANFLOOR)
                         && !IS_AFFECTED(victim, AFF_AQUA_BREATH)
                         && !IS_NPC(victim))
                        mudstrlcat(buf, " is drowning here.", MSL);
                else if (victim->in_room->sector_type == SECT_OCEANFLOOR)
                        mudstrlcat(buf, " is standing here in the water.",
                                   MSL);
                else if (IS_AFFECTED(victim, AFF_FLOATING)
                         || IS_AFFECTED(victim, AFF_FLYING))
                        mudstrlcat(buf, " is hovering here.", MSL);
                else
                        mudstrlcat(buf, " is standing here.", MSL);
                break;
        case POS_SHOVE:
                mudstrlcat(buf, " is being shoved around.", MSL);
                break;
        case POS_DRAG:
                mudstrlcat(buf, " is being dragged around.", MSL);
                break;
        case POS_MOUNTED:
                mudstrlcat(buf, " is here, upon ", MSL);
                if (!victim->mount)
                        mudstrlcat(buf, "thin air???", MSL);
                else if (victim->mount == ch)
                        mudstrlcat(buf, "your back.", MSL);
                else if (victim->in_room == victim->mount->in_room)
                {
                        mudstrlcat(buf, PERS(victim->mount, ch), MSL);
                        mudstrlcat(buf, ".", MSL);
                }
                else
                        mudstrlcat(buf, "someone who left??", MSL);
                break;
        case POS_FIGHTING:
                mudstrlcat(buf, " is here, fighting ", MSL);
                if (!victim->fighting)
                        mudstrlcat(buf, "thin air???", MSL);
                else if (who_fighting(victim) == ch)
                        mudstrlcat(buf, "YOU!", MSL);
                else if (victim->in_room == victim->fighting->who->in_room)
                {
                        mudstrlcat(buf, PERS(victim->fighting->who, ch), MSL);
                        mudstrlcat(buf, ".", MSL);
                }
                else
                        mudstrlcat(buf, "someone who left??", MSL);
                break;
        default:
                {
                }
        }

        mudstrlcat(buf, "\n\r", MSL);
        buf[0] = UPPER(buf[0]);
        send_to_char(buf, ch);
        show_visible_affects_to_char(victim, ch);
        return;
}



void show_char_to_char_1(CHAR_DATA * victim, CHAR_DATA * ch)
{
        OBJ_DATA *obj;
        int       iWear;
        bool      found;

        if (can_see(victim, ch))
        {
                act(AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT);
                act(AT_ACTION, "$n looks at $N.", ch, NULL, victim ,TO_NOTVICT);
        }
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        if (victim->description[0] != '\0')
                ch_printf(ch, "&z%s\n\r", wordwrap(victim->description, 72));
        else
        {
                if (victim->race && victim->race->name())
                        ch_printf(ch,
                                  "You see nothing special about the %s.\n\r",
                                  victim->race->name());
        }
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&w",
                 ch);
        show_condition(ch, victim);
        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        found = FALSE;
        for (iWear = 0; iWear < MAX_WEAR; iWear++)
        {
                if ((obj = get_eq_char(victim, iWear)) != NULL
                    && can_see_obj(ch, obj))
                {
                        if (!found)
                        {
                                send_to_char("\n\r", ch);
                                act(AT_PLAIN, "&w$N &zis using:&w", ch, NULL,
                                    victim, TO_CHAR);
                                found = TRUE;
                        }
                        // Fix color bleed. -- Kasji
                        ch_printf(ch, "&w%s", where_name[iWear]);
/*                      send_to_char(where_name[iWear], ch); */
                        send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
                        send_to_char("\n\r", ch);
                }
        }

        send_to_char
                ("&B-----------------------------------------------------------------------\n\r&z",
                 ch);
        /*
         * Crash fix here by Thoric
         */
        if (IS_NPC(ch) || victim == ch)
                return;

        if (number_percent() < ch->pcdata->learned[gsn_peek])
        {
                send_to_char("\n\r&BY&zou peek at the inventory:\n\r", ch);
                show_list_to_char(victim->first_carrying, ch, TRUE, TRUE);
                learn_from_success(ch, gsn_peek);
        }
        else if (ch->pcdata->learned[gsn_peek])
                learn_from_failure(ch, gsn_peek);
        return;
}


void show_char_to_char(CHAR_DATA * list, CHAR_DATA * ch)
{
        CHAR_DATA *rch;

        for (rch = list; rch; rch = rch->next_in_room)
        {
                if (rch == ch)
                        continue;

                if (can_see(ch, rch))
                {
                        show_char_to_char_0(rch, ch);
                }
                else if (!str_cmp(rch->race->name(), "defel"))
                {
                        set_char_color(AT_BLOOD, ch);
                        send_to_char
                                ("You see a pair of red eyes staring back at you.\n\r",
                                 ch);
                }
                else if (room_is_dark(ch->in_room)
                         && IS_AFFECTED(rch, AFF_INFRARED))
                {
                        set_char_color(AT_BLOOD, ch);
                        send_to_char
                                ("The red form of a living creature is here.\n\r",
                                 ch);
                }
        }

        return;
}



bool check_blind(CHAR_DATA * ch)
{
        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
                return TRUE;

        if (IS_AFFECTED(ch, AFF_TRUESIGHT))
                return TRUE;

        if (IS_AFFECTED(ch, AFF_BLIND))
        {
                send_to_char("You can't see a thing!\n\r", ch);
                return FALSE;
        }

        return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door(char *arg)
{
        int       door;

        if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
                door = 0;
        else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
                door = 1;
        else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
                door = 2;
        else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
                door = 3;
        else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
                door = 4;
        else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
                door = 5;
        else if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast"))
                door = 6;
        else if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest"))
                door = 7;
        else if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast"))
                door = 8;
        else if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest"))
                door = 9;
        else
                door = -1;
        return door;
}

CMDF do_look(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        EXIT_DATA *pexit;
        CHAR_DATA *victim;
        OBJ_DATA *obj;
        ROOM_INDEX_DATA *original;
        char     *pdesc;
        bool      doexaprog;
        sh_int    door;
        int       number, cnt;

        if (!ch->desc)
                return;

        if (ch->position < POS_SLEEPING)
        {
                send_to_char("You can't see anything but stars!\n\r", ch);
                return;
        }

        if (ch->position == POS_SLEEPING)
        {
                send_to_char("You can't see anything, you're sleeping!\n\r",
                             ch);
                return;
        }

        if (!check_blind(ch))
                return;

        if (!IS_NPC(ch)
            && !IS_SET(ch->act, PLR_HOLYLIGHT)
            && !IS_AFFECTED(ch, AFF_TRUESIGHT) && room_is_dark(ch->in_room))
        {
                set_char_color(AT_DGREY, ch);
                send_to_char("It is pitch black ... \n\r", ch);
                show_char_to_char(ch->in_room->first_person, ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        doexaprog = str_cmp("noprog", arg2) && str_cmp("noprog", arg3);

        if (arg1[0] == '\0' || !str_cmp(arg1, "auto"))
        {
                SHIP_DATA *ship;

                /*
                 * 'look' or 'look auto' 
                 */
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                send_to_char("&w", ch);
                if (IS_MXP(ch))
                        send_to_char(MXPTAG("RName"), ch);
                send_to_char(ch->in_room->name, ch);
                if (IS_MXP(ch))
                        send_to_char(MXPTAG("/RName"), ch);
                send_to_char(" ", ch);

                if (!ch->desc->original)
                {

                        if (get_trust(ch) >= LEVEL_IMMORTAL)
                        {
                                if (IS_SET(ch->act, PLR_ROOMVNUM))
                                {
                                        /*
                                         * Added 10/17 by Kuran of 
                                         */
                                        send_to_char("&B{&w", ch);  /* SWReality */
                                        if (IS_MXP(ch))
                                                send_to_char(MXPTAG("RNum"),
                                                             ch);
                                        ch_printf(ch, "%d",
                                                  ch->in_room->vnum);
                                        if (IS_MXP(ch))
                                                send_to_char(MXPTAG("/RNum"),
                                                             ch);
                                        send_to_char("&B}}", ch);
                                }
                                if (IS_SET(ch->pcdata->flags, PCFLAG_ROOM))
                                {
                                        send_to_char("[&z", ch);
                                        send_to_char(ext_flag_string
                                                     (&ch->in_room->
                                                      room_flags, r_flags),
                                                     ch);
                                        send_to_char("&B]", ch);
                                }
                        }

                }

                send_to_char("\n\r", ch);
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                send_to_char("&z", ch);
                if (arg1[0] == '\0'
                    || (!IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF)))
                {
                        if (IS_MXP(ch))
                                send_to_char(MXPTAG("RDesc"), ch);
                        send_to_char(ch->in_room->description, ch);
                        if (IS_MXP(ch))
                                send_to_char(MXPTAG("/RDesc"), ch);
                }

                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);
                if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT))
                        do_exits(ch, "");
                send_to_char
                        ("&B-----------------------------------------------------------------------&w\n\r",
                         ch);


                show_ships_to_char(ch->in_room->first_ship, ch);
#ifdef OLC_SHUTTLE
                show_shuttles_to_char(ch, ch->in_room->first_shuttle);
#endif
                show_list_to_char(ch->in_room->first_content, ch, FALSE,
                                  FALSE);
                show_char_to_char(ch->in_room->first_person, ch);

                if (str_cmp(arg1, "auto"))
                        if ((ship =
                             ship_from_cockpit(ch->in_room->vnum)) != NULL)
                        {
                                set_char_color(AT_WHITE, ch);
                                ch_printf(ch,
                                          "\n\rThrough the transparisteel windows you see:\n\r");
                                if (ship->starsystem)
                                {
                                        MISSILE_DATA *missile = NULL;
                                        SHIP_DATA *target = NULL;
                                        BODY_DATA *body = NULL;

                                        set_char_color(AT_GREEN, ch);
                                        /*
                                         * I really hate doing 3 for loops for sorting, we should time it 
                                         */
                                        FOR_EACH_LIST(BODY_LIST,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() == STAR_BODY)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        FOR_EACH_LIST(BODY_LIST,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() ==
                                                    PLANET_BODY)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        FOR_EACH_LIST(BODY_LIST,
                                                      ship->starsystem->
                                                      bodies, body)
                                        {
                                                if (body->type() == MOON_BODY)
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  body->
                                                                  name());
                                        }
                                        for (target =
                                             ship->starsystem->first_ship;
                                             target;
                                             target =
                                             target->next_in_starsystem)
                                        {
                                                if (target != ship
                                                    && !IS_SET(target->flags,
                                                               SHIP_CLOAK))
                                                        ch_printf(ch,
                                                                  "%s\n\r",
                                                                  target->
                                                                  name);
                                        }
                                        for (missile =
                                             ship->starsystem->first_missile;
                                             missile;
                                             missile =
                                             missile->next_in_starsystem)
                                        {
                                                ch_printf(ch, "%s\n\r",
                                                          missile->
                                                          missiletype ==
                                                          CONCUSSION_MISSILE ?
                                                          "A Concusion Missile"
                                                          : (missile->
                                                             missiletype ==
                                                             PROTON_TORPEDO ?
                                                             "A Torpedo"
                                                             : (missile->
                                                                missiletype ==
                                                                HEAVY_ROCKET ?
                                                                "A Heavy Rocket"
                                                                :
                                                                "A Heavy Bomb")));
                                        }

                                }
                                else if (ship->location == ship->lastdoc)
                                {
                                        ROOM_INDEX_DATA *to_room;

                                        if ((to_room =
                                             get_room_index(ship->
                                                            location)) !=
                                            NULL)
                                        {
                                                ch_printf(ch, "\n\r");
                                                original = ch->in_room;
                                                char_from_room(ch);
                                                char_to_room(ch, to_room);
                                                do_glance(ch, "");
                                                char_from_room(ch);
                                                char_to_room(ch, original);
                                        }
                                }


                        }

                return;
        }

        if (!str_cmp(arg1, "under"))
        {
                int       count;

                /*
                 * 'look under' 
                 */
                if (arg2[0] == '\0')
                {
                        send_to_char("Look beneath what?\n\r", ch);
                        return;
                }

                if ((obj = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char("You do not see that here.\n\r", ch);
                        return;
                }
                if (ch->carry_weight + obj->weight > can_carry_w(ch))
                {
                        send_to_char
                                ("It's too heavy for you to look under.\n\r",
                                 ch);
                        return;
                }
                count = obj->count;
                obj->count = 1;
                act(AT_PLAIN, "You lift $p and look beneath it:", ch, obj,
                    NULL, TO_CHAR);
                act(AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj,
                    NULL, TO_ROOM);
                obj->count = count;
                if (IS_OBJ_STAT(obj, ITEM_COVERING))
                        show_list_to_char(obj->first_content, ch, TRUE, TRUE);
                else
                        send_to_char("Nothing.\n\r", ch);
                if (doexaprog)
                        oprog_examine_trigger(ch, obj);
                return;
        }

        if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in"))
        {
                int       count;

                /*
                 * 'look in' 
                 */
                if (arg2[0] == '\0')
                {
                        send_to_char("Look in what?\n\r", ch);
                        return;
                }

                if ((obj = get_obj_here(ch, arg2)) == NULL)
                {
                        send_to_char("You do not see that here.\n\r", ch);
                        return;
                }

                switch (obj->item_type)
                {
                default:
                        send_to_char("That is not a container.\n\r", ch);
                        break;

                case ITEM_DRINK_CON:
                        if (obj->value[1] <= 0)
                        {
                                send_to_char("It is empty.\n\r", ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                break;
                        }

                        ch_printf(ch, "It's %s full of a %s liquid.\n\r",
                                  obj->value[1] < obj->value[0] / 4
                                  ? "less than" :
                                  obj->value[1] < 3 * obj->value[0] / 4
                                  ? "about" : "more than",
                                  liq_table[obj->value[2]].liq_color);

                        if (doexaprog)
                                oprog_examine_trigger(ch, obj);
                        break;

                case ITEM_PORTAL:
                        for (pexit = ch->in_room->first_exit; pexit;
                             pexit = pexit->next)
                        {
                                if (pexit->vdir == DIR_PORTAL
                                    && IS_SET(pexit->exit_info, EX_PORTAL))
                                {
                                        if (room_is_private
                                            (ch, pexit->to_room)
                                            && get_trust(ch) <
                                            sysdata.level_override_private)
                                        {
                                                set_char_color(AT_WHITE, ch);
                                                send_to_char
                                                        ("That room is private buster!\n\r",
                                                         ch);
                                                return;
                                        }
                                        original = ch->in_room;
                                        char_from_room(ch);
                                        char_to_room(ch, pexit->to_room);
                                        do_look(ch, "auto");
                                        char_from_room(ch);
                                        char_to_room(ch, original);
                                        return;
                                }
                        }
                        send_to_char("You see a swirling chaos...\n\r", ch);
                        break;
                case ITEM_HOLSTER:
                case ITEM_CONTAINER:
                case ITEM_CORPSE_NPC:
                case ITEM_CORPSE_PC:
                case ITEM_DROID_CORPSE:
                        if (IS_SET(obj->value[1], CONT_CLOSED))
                        {
                                send_to_char("It is closed.\n\r", ch);
                                break;
                        }

                        count = obj->count;
                        obj->count = 1;
                        act(AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR);
                        obj->count = count;
                        show_list_to_char(obj->first_content, ch, TRUE, TRUE);
                        if (doexaprog)
                                oprog_examine_trigger(ch, obj);
                        break;
                }
                return;
        }

        if ((pdesc =
             get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL)
        {
                send_to_char(pdesc, ch);
                return;
        }

        door = get_door(arg1);
        if ((pexit = find_door(ch, arg1, TRUE)) != NULL)
        {
                if (pexit->keyword)
                {
                        if (IS_SET(pexit->exit_info, EX_CLOSED)
                            && !IS_SET(pexit->exit_info, EX_WINDOW))
                        {
                                if (IS_SET(pexit->exit_info, EX_SECRET)
                                    && door != -1)
                                        send_to_char
                                                ("Nothing special there.\n\r",
                                                 ch);
                                else
                                        act(AT_PLAIN, "The $d is closed.", ch,
                                            NULL, pexit->keyword, TO_CHAR);
                                return;
                        }
                        if (IS_SET(pexit->exit_info, EX_BASHED))
                                act(AT_RED,
                                    "The $d has been bashed from its hinges!",
                                    ch, NULL, pexit->keyword, TO_CHAR);
                }

                if (pexit->description && pexit->description[0] != '\0')
                        send_to_char(pexit->description, ch);
                else
                        send_to_char("Nothing special there.\n\r", ch);

                /*
                 * Ability to look into the next room         -Thoric
                 */
                if (pexit->to_room
                    && (IS_AFFECTED(ch, AFF_SCRYING)
                        || IS_SET(pexit->exit_info, EX_xLOOK)
                        || get_trust(ch) >= LEVEL_IMMORTAL))
                {
                        if (!IS_SET(pexit->exit_info, EX_xLOOK)
                            && get_trust(ch) < LEVEL_IMMORTAL)
                        {
                                set_char_color(AT_MAGIC, ch);
                                send_to_char("You attempt to scry...\n\r",
                                             ch);
                                /*
                                 * Change by Narn, Sept 96 to allow characters who don't have the
                                 * scry spell to benefit from objects that are affected by scry.
                                 */
                                if (!IS_NPC(ch))
                                {
                                        int       percent =
                                                ch->pcdata->
                                                learned[skill_lookup("scry")];
                                        if (!percent)
                                                percent = 99;

                                        if (number_percent() > percent)
                                        {
                                                send_to_char("You fail.\n\r",
                                                             ch);
                                                return;
                                        }
                                }
                        }
                        if (room_is_private(ch, pexit->to_room)
                            && get_trust(ch) < sysdata.level_override_private)
                        {
                                set_char_color(AT_WHITE, ch);
                                send_to_char
                                        ("That room is private buster!\n\r",
                                         ch);
                                return;
                        }
                        original = ch->in_room;
                        if (pexit->distance > 1)
                        {
                                ROOM_INDEX_DATA *to_room;

                                if ((to_room =
                                     generate_exit(ch->in_room,
                                                   &pexit)) != NULL)
                                {
                                        char_from_room(ch);
                                        char_to_room(ch, to_room);
                                }
                                else
                                {
                                        char_from_room(ch);
                                        char_to_room(ch, pexit->to_room);
                                }
                        }
                        else
                        {
                                char_from_room(ch);
                                char_to_room(ch, pexit->to_room);
                        }
                        do_look(ch, "auto");
                        char_from_room(ch);
                        char_to_room(ch, original);
                }
                return;
        }
        else if (door != -1)
        {
                send_to_char("Nothing special there.\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) != NULL)
        {
                show_char_to_char_1(victim, ch);
                return;
        }


        /*
         * finally fixed the annoying look 2.obj desc bug   -Thoric 
         */
        number = number_argument(arg1, arg);
        for (cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (can_see_obj(ch, obj))
                {
                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->pIndexData->
                                             first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if (nifty_is_name_prefix(arg, obj->name))
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                pdesc = get_extra_descr(obj->name,
                                                        obj->pIndexData->
                                                        first_extradesc);
                                if (!pdesc)
                                        pdesc = get_extra_descr(obj->name,
                                                                obj->
                                                                first_extradesc);
                                if (!pdesc)
                                        send_to_char
                                                ("You see nothing special.\r\n",
                                                 ch);
                                else
                                        send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                }
        }

        for (obj = ch->in_room->last_content; obj; obj = obj->prev_content)
        {
                if (can_see_obj(ch, obj))
                {
                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }

                        if ((pdesc =
                             get_extra_descr(arg,
                                             obj->pIndexData->
                                             first_extradesc)) != NULL)
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                        if (nifty_is_name_prefix(arg, obj->name))
                        {
                                if ((cnt += obj->count) < number)
                                        continue;
                                pdesc = get_extra_descr(obj->name,
                                                        obj->pIndexData->
                                                        first_extradesc);
                                if (!pdesc)
                                        pdesc = get_extra_descr(obj->name,
                                                                obj->
                                                                first_extradesc);
                                if (!pdesc)
                                        send_to_char
                                                ("You see nothing special.\r\n",
                                                 ch);
                                else
                                        send_to_char(pdesc, ch);
                                if (doexaprog)
                                        oprog_examine_trigger(ch, obj);
                                return;
                        }
                }
        }

        send_to_char("You do not see that here.\n\r", ch);
        return;
}

void show_condition(CHAR_DATA * ch, CHAR_DATA * victim)
{
        char      buf[MAX_STRING_LENGTH];
        int       percent;

        if (!victim || !victim->name)
                return;
        if (victim->max_hit > 0)
                percent = (100 * victim->hit) / victim->max_hit;
        else
                percent = -1;


        mudstrlcpy(buf, "&R&W", MSL);
        mudstrlcat(buf, capitalize(PERS(victim, ch)), MSL);
        if ((IS_NPC(victim) && IS_SET(victim->act, ACT_DROID))
            || !str_cmp(victim->race->name(), "droid"))
        {

                if (percent >= 100)
                        mudstrlcat(buf, " is in perfect condition.\n\r", MSL);
                else if (percent >= 90)
                        mudstrlcat(buf, " is slightly scratched.\n\r", MSL);
                else if (percent >= 80)
                        mudstrlcat(buf, " has a few scrapes.\n\r", MSL);
                else if (percent >= 70)
                        mudstrlcat(buf, " has some dents.\n\r", MSL);
                else if (percent >= 60)
                        mudstrlcat(buf,
                                   " has a couple holes in its plating.\n\r",
                                   MSL);
                else if (percent >= 50)
                        mudstrlcat(buf, " has a many broken pieces.\n\r",
                                   MSL);
                else if (percent >= 40)
                        mudstrlcat(buf, " has many exposed circuits.\n\r",
                                   MSL);
                else if (percent >= 30)
                        mudstrlcat(buf, " is leaking oil.\n\r", MSL);
                else if (percent >= 20)
                        mudstrlcat(buf, " has smoke coming out of it.\n\r",
                                   MSL);
                else if (percent >= 10)
                        mudstrlcat(buf, " is almost completely broken.\n\r",
                                   MSL);
                else
                        mudstrlcat(buf, " is about to EXPLODE.\n\r", MSL);

        }
        else
        {

                if (percent >= 100)
                        mudstrlcat(buf, " is in perfect health.\n\r", MSL);
                else if (percent >= 90)
                        mudstrlcat(buf, " is slightly scratched.\n\r", MSL);
                else if (percent >= 80)
                        mudstrlcat(buf, " has a few bruises.\n\r", MSL);
                else if (percent >= 70)
                        mudstrlcat(buf, " has some cuts.\n\r", MSL);
                else if (percent >= 60)
                        mudstrlcat(buf, " has several wounds.\n\r", MSL);
                else if (percent >= 50)
                        mudstrlcat(buf, " has many nasty wounds.\n\r", MSL);
                else if (percent >= 40)
                        mudstrlcat(buf, " is bleeding freely.\n\r", MSL);
                else if (percent >= 30)
                        mudstrlcat(buf, " is covered in blood.\n\r", MSL);
                else if (percent >= 20)
                        mudstrlcat(buf, " is leaking guts.\n\r", MSL);
                else if (percent >= 10)
                        mudstrlcat(buf, " is almost dead.\n\r", MSL);
                else
                        mudstrlcat(buf, " is DYING.\n\r", MSL);

        }
        buf[0] = UPPER(buf[0]);
        send_to_char(buf, ch);
        return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
CMDF do_glance(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       save_act;

        if (!ch->desc)
                return;

        if (ch->position < POS_SLEEPING)
        {
                send_to_char("You can't see anything but stars!\n\r", ch);
                return;
        }

        if (ch->position == POS_SLEEPING)
        {
                send_to_char("You can't see anything, you're sleeping!\n\r",
                             ch);
                return;
        }

        if (!check_blind(ch))
                return;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                save_act = ch->act;
                SET_BIT(ch->act, PLR_BRIEF);
                do_look(ch, "auto");
                ch->act = save_act;
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They're not here.", ch);
                return;
        }
        else
        {
                if (can_see(victim, ch))
                {
                        act(AT_ACTION, "$n glances at you.", ch, NULL, victim,
                            TO_VICT);
                        act(AT_ACTION, "$n glances at $N.", ch, NULL, victim,
                            TO_NOTVICT);
                }

                show_condition(ch, victim);
                return;
        }

        return;
}


/* New command to view a player's skills - Samson 4-13-98 */
CMDF do_viewskills(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       sn;
        int       col;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("&zSyntax: skills <player>.\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("No such person in the game.\n\r", ch);
                return;
        }

        col = 0;

        if (!IS_NPC(victim))
        {
                sh_int    lasttype, cnt;

                col = cnt = 0;
                lasttype = SKILL_SPELL;
                set_pager_color(AT_MAGIC, ch);
                for (sn = 0; sn < top_sn; sn++)
                {
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        if (strcmp(skill_table[sn]->name, "reserved") == 0
                            && (IS_IMMORTAL(victim)))
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                send_to_pager
                                        ("&B--------------------------------[&zSpells&B]---------------------------------\n\r",
                                         ch);
                                send_to_pager("&w", ch);
                                col = 0;
                        }
                        if (skill_table[sn]->type != lasttype)
                        {
                                if (!cnt)
                                        send_to_pager
                                                ("                                (none)\n\r",
                                                 ch);
                                else if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                pager_printf(ch,
                                             "&B--------------------------------&B[&z%ss&B]---------------------------------\n\r",
                                             skill_tname[skill_table[sn]->
                                                         type]);
                                col = cnt = 0;
                        }

                        lasttype = skill_table[sn]->type;

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s", ch->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        continue;
                                }
                        }


                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        if (victim->pcdata->learned[sn] <= 0
                            && victim->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                                continue;

                        if (victim->pcdata->learned[sn] == 0
                            && SPELL_FLAG(skill_table[sn], SF_SECRETSKILL))
                                continue;

                        ++cnt;
                        pager_printf(ch, "&z%18.18s &W%3.3d%%  ",
                                     skill_table[sn]->name,
                                     victim->pcdata->learned[sn]);
                        if (++col % 3 == 0)
                                send_to_pager("\n\r", ch);
                }

                if (col % 3 != 0)
                        send_to_pager("\n\r", ch);
        }
        return;
}



CMDF do_examine(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        BOARD_DATA *board;
        sh_int    dam;

        if (!argument)
        {
                bug("do_examine: null argument.", 0);
                return;
        }

        if (!ch)
        {
                bug("do_examine: null ch.", 0);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Examine what?\n\r", ch);
                return;
        }

        snprintf(buf, MSL, "%s noprog", arg);
        do_look(ch, buf);

        /*
         * Support for looking at boards, checking equipment conditions,
         * and support for trigger positions by Thoric
         */
        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                if ((board = get_board(obj)) != NULL)
                {
                        if (board->num_posts)
                                ch_printf(ch,
                                          "There are about %d notes posted here.  Type 'note list' to list them.\n\r",
                                          board->num_posts);
                        else
                                send_to_char
                                        ("There aren't any notes posted here.\n\r",
                                         ch);
                }

                switch (obj->item_type)
                {
                default:
                        break;

                case ITEM_ARMOR:
                        if (obj->value[1] == 0)
                                obj->value[1] = obj->value[0];
                        if (obj->value[1] == 0)
                                obj->value[1] = 1;
                        dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
                        mudstrlcpy(buf,
                                   "As you look more closely, you notice that it is ",
                                   MSL);
                        if (dam >= 10)
                                mudstrlcat(buf, "in superb condition.", MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "in very good condition.",
                                           MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "in good shape.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "showing a bit of wear.",
                                           MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "a little run down.", MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "in need of repair.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "in great need of repair.",
                                           MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "in dire need of repair.",
                                           MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "very badly worn.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "practically worthless.",
                                           MSL);
                        else if (dam <= 0)
                                mudstrlcat(buf, "broken.", MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        break;

                case ITEM_WEAPON:
                        dam = INIT_WEAPON_CONDITION - obj->value[0];
                        mudstrlcpy(buf,
                                   "As you look more closely, you notice that it is ",
                                   MSL);
                        if (dam == 0)
                                mudstrlcat(buf, "in superb condition.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "in excellent condition.",
                                           MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "in very good condition.",
                                           MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "in good shape.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "showing a bit of wear.",
                                           MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "a little run down.", MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "in need of repair.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "in great need of repair.",
                                           MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "in dire need of repair.",
                                           MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "very badly worn.", MSL);
                        else if (dam == 10)
                                mudstrlcat(buf, "practically worthless.",
                                           MSL);
                        else if (dam == 11)
                                mudstrlcat(buf, "almost broken.", MSL);
                        else if (dam == 12)
                                mudstrlcat(buf, "broken.", MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        if (obj->value[3] == WEAPON_BLASTER)
                        {
                                if (obj->blaster_setting == BLASTER_FULL)
                                        ch_printf(ch,
                                                  "It is set on FULL power.\n\r");
                                else if (obj->blaster_setting == BLASTER_HIGH)
                                        ch_printf(ch,
                                                  "It is set on HIGH power.\n\r");
                                else if (obj->blaster_setting ==
                                         BLASTER_NORMAL)
                                        ch_printf(ch,
                                                  "It is set on NORMAL power.\n\r");
                                else if (obj->blaster_setting == BLASTER_HALF)
                                        ch_printf(ch,
                                                  "It is set on HALF power.\n\r");
                                else if (obj->blaster_setting == BLASTER_LOW)
                                        ch_printf(ch,
                                                  "It is set on LOW power.\n\r");
                                else if (obj->blaster_setting == BLASTER_STUN)
                                        ch_printf(ch,
                                                  "It is set on STUN.\n\r");
                                ch_printf(ch,
                                          "It has from %d to %d shots remaining.\n\r",
                                          obj->value[4] / 5, obj->value[4]);
                        }
                        else if ((obj->value[3] == WEAPON_LIGHTSABER ||
                                  obj->value[3] == WEAPON_VIBRO_BLADE ||
                                  obj->value[3] == WEAPON_FORCE_PIKE))
                        {
                                ch_printf(ch,
                                          "It has %d/%d units of charge remaining.\n\r",
                                          obj->value[4], obj->value[5]);
                        }
                        break;

                case ITEM_FOOD:
                        if (obj->timer > 0 && obj->value[1] > 0)
                                dam = (obj->timer * 10) / obj->value[1];
                        else
                                dam = 10;
                        mudstrlcpy(buf,
                                   "As you examine it carefully you notice that it ",
                                   MSL);
                        if (dam >= 10)
                                mudstrlcat(buf, "is fresh.", MSL);
                        else if (dam == 9)
                                mudstrlcat(buf, "is nearly fresh.", MSL);
                        else if (dam == 8)
                                mudstrlcat(buf, "is perfectly fine.", MSL);
                        else if (dam == 7)
                                mudstrlcat(buf, "looks good.", MSL);
                        else if (dam == 6)
                                mudstrlcat(buf, "looks ok.", MSL);
                        else if (dam == 5)
                                mudstrlcat(buf, "is a little stale.", MSL);
                        else if (dam == 4)
                                mudstrlcat(buf, "is a bit stale.", MSL);
                        else if (dam == 3)
                                mudstrlcat(buf, "smells slightly off.", MSL);
                        else if (dam == 2)
                                mudstrlcat(buf, "smells quite rank.", MSL);
                        else if (dam == 1)
                                mudstrlcat(buf, "smells revolting.", MSL);
                        else if (dam <= 0)
                                mudstrlcat(buf, "is crawling with maggots.",
                                           MSL);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        break;

                case ITEM_SWITCH:
                case ITEM_LEVER:
                case ITEM_PULLCHAIN:
                        if (IS_SET(obj->value[0], TRIG_UP))
                                send_to_char
                                        ("You notice that it is in the up position.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You notice that it is in the down position.\n\r",
                                         ch);
                        break;
                case ITEM_BUTTON:
                        if (IS_SET(obj->value[0], TRIG_UP))
                                send_to_char
                                        ("You notice that it is depressed.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("You notice that it is not depressed.\n\r",
                                         ch);
                        break;

/* Not needed due to check in do_look already
	case ITEM_PORTAL:
	    snprintf( buf, MSL, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
*/

                case ITEM_CORPSE_PC:
                case ITEM_CORPSE_NPC:
                        {
                                sh_int    timerfrac = obj->timer;

                                if (obj->item_type == ITEM_CORPSE_PC)
                                        timerfrac = (int) obj->timer / 8 + 1;

                                switch (timerfrac)
                                {
                                default:
                                        send_to_char
                                                ("This corpse has recently been slain.\n\r",
                                                 ch);
                                        break;
                                case 4:
                                        send_to_char
                                                ("This corpse was slain a little while ago.\n\r",
                                                 ch);
                                        break;
                                case 3:
                                        send_to_char
                                                ("A foul smell rises from the corpse, and it is covered in flies.\n\r",
                                                 ch);
                                        break;
                                case 2:
                                        send_to_char
                                                ("A writhing mass of maggots and decay, you can barely go near this corpse.\n\r",
                                                 ch);
                                        break;
                                case 1:
                                case 0:
                                        send_to_char
                                                ("Little more than bones, there isn't much left of this corpse.\n\r",
                                                 ch);
                                        break;
                                }
                        }
                        if (IS_OBJ_STAT(obj, ITEM_COVERING))
                                break;
                        send_to_char("When you look inside, you see:\n\r",
                                     ch);
                        snprintf(buf, MSL, "in %s noprog", arg);
                        do_look(ch, buf);
                        break;

                case ITEM_DROID_CORPSE:
                        {
                                sh_int    timerfrac = obj->timer;

                                switch (timerfrac)
                                {
                                default:
                                        send_to_char
                                                ("These remains are still smoking.\n\r",
                                                 ch);
                                        break;
                                case 4:
                                        send_to_char
                                                ("The parts of this droid have cooled down completely.\n\r",
                                                 ch);
                                        break;
                                case 3:
                                        send_to_char
                                                ("The broken droid components are beginning to rust.\n\r",
                                                 ch);
                                        break;
                                case 2:
                                        send_to_char
                                                ("The pieces are completely covered in rust.\n\r",
                                                 ch);
                                        break;
                                case 1:
                                case 0:
                                        send_to_char
                                                ("All that remains of it is a pile of crumbling rust.\n\r",
                                                 ch);
                                        break;
                                }
                        }

                case ITEM_CONTAINER:
                        if (IS_OBJ_STAT(obj, ITEM_COVERING))
                                break;

                case ITEM_DRINK_CON:
                        send_to_char("When you look inside, you see:\n\r",
                                     ch);
                        snprintf(buf, MSL, "in %s noprog", arg);
                        do_look(ch, buf);
                }
                if (IS_OBJ_STAT(obj, ITEM_COVERING))
                {
                        snprintf(buf, MSL, "under %s noprog", arg);
                        do_look(ch, buf);
                }
                oprog_examine_trigger(ch, obj);
                if (char_died(ch) || obj_extracted(obj))
                        return;

                check_for_trap(ch, obj, TRAP_EXAMINE);
        }
        return;
}


CMDF do_exits(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        EXIT_DATA *pexit;
        bool      found;
        bool      fAuto;

        set_char_color(AT_EXITS, ch);
        buf[0] = '\0';
        fAuto = !str_cmp(argument, "auto");

        if (!check_blind(ch))
                return;

        if (IS_MXP(ch))
                send_to_char(MXPTAG("RExits"), ch);
        mudstrlcpy(buf, fAuto ? "&BE&xits:" : "&BObvious exits:\n\r&w", MSL);

        found = FALSE;
        for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
        {
                if (pexit->to_room && !IS_SET(pexit->exit_info, EX_HIDDEN))
                {
                        found = TRUE;
                        if (!fAuto)
                        {
                                if (IS_SET(pexit->exit_info, EX_CLOSED))
                                {
                                        /*
                                         * I really dson't like copy and pasting, but its alot cleaner and easier right now than hacking together lines of mudstrlcat and stuff - Gavin 
                                         */
                                        if (IS_MXP(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - (closed)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - (closed)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                }
                                else if (IS_SET(pexit->exit_info, EX_WINDOW))
                                {
                                        if (IS_MXP(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - (window)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - (window)\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]));
                                }
                                else if (IS_SET(pexit->exit_info, EX_xAUTO))
                                {
                                        if (IS_MXP(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("ex")
                                                         "%-5s" MXPTAG("/ex")
                                                         " - %s\n\r",
                                                         pexit->keyword,
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - %s\n\r",
                                                         pexit->keyword,
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                }
                                else
                                {
                                        if (IS_MXP(ch))
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w" MXPTAG("Ex")
                                                         "%-5s" MXPTAG("/Ex")
                                                         " - %s\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]),
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "Too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                        else
                                                snprintf(buf + strlen(buf),
                                                         MSL,
                                                         "&w%-5s - %s\n\r",
                                                         capitalize(dir_name
                                                                    [pexit->
                                                                     vdir]),
                                                         room_is_dark(pexit->
                                                                      to_room)
                                                         ? "Too dark to tell"
                                                         : pexit->to_room->
                                                         name);
                                }
                        }
                        else
                        {
                                if (IS_MXP(ch))
                                        snprintf(buf + strlen(buf), MSL,
                                                 "&w " MXPTAG("Ex") "%s"
                                                 MXPTAG("/Ex"),
                                                 capitalize(dir_name
                                                            [pexit->vdir]));
                                else
                                        snprintf(buf + strlen(buf), MSL,
                                                 "&w %s",
                                                 capitalize(dir_name
                                                            [pexit->vdir]));
                        }
                }
        }

        if (!found)
                mudstrlcat(buf, fAuto ? " none.\n\r" : "None.\n\r", MSL);
        else if (fAuto)
                mudstrlcat(buf, ".\n\r", MSL);
        send_to_char(buf, ch);
        if (IS_MXP(ch))
                send_to_char(MXPTAG("/RExits"), ch);
        return;
}

char     *const day_name[] = {
        "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
        "the Great Gods", "the Sun"
};

char     *const month_name[] = {
        "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
        "the Grand Struggle", "the Spring", "Nature", "Futility",
        "the Dragon",
        "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
        "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

extern char str_boot_time[];
extern char reboot_time[];
CMDF do_time(CHAR_DATA * ch, char *argument)
{
        char     *suf;
        int       day;
        time_t    current;
        time_t    diff;

        current = time(0);
        diff = current - boot_time;

        argument = NULL;
        day = time_info.day + 1;

        if (day > 4 && day < 20)
                suf = "th";
        else if (day % 10 == 1)
                suf = "st";
        else if (day % 10 == 2)
                suf = "nd";
        else if (day % 10 == 3)
                suf = "rd";
        else
                suf = "th";

        set_char_color(AT_YELLOW, ch);
        ch_printf(ch,
                  "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"
                  "The mud started up at:    %s\r"
                  "The system time (M.S.T.): %s\r"
                  "Next Reboot is set for:   %s\r",
                  (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
                  time_info.hour >= 12 ? "pm" : "am",
                  day_name[day % 7],
                  day, suf,
                  month_name[time_info.month],
                  str_boot_time, (char *) ctime(&current_time), reboot_time);
        if (sysdata.CLEANPFILES)
                ch_printf(ch, "Next pfile cleanup :      %s",
                          (char *) ctime(&new_pfile_time_t));
        ch_printf(ch,
                  "The mud has been up for:  %d hours, %d minutes, %d seconds.\n\r",
                  diff / 3600, (diff / 60) % 60, diff % 60);

        return;
}



CMDF do_weather(CHAR_DATA * ch, char *argument)
{
        static char *const sky_look[4] = {
                "cloudless",
                "cloudy",
                "rainy",
                "lit by flashes of lightning"
        };

        argument = NULL;
        if (!IS_OUTSIDE(ch))
        {
                send_to_char("You can't see the sky from here.\n\r", ch);
                return;
        }

        set_char_color(AT_BLUE, ch);
        ch_printf(ch, "The sky is %s and %s.\n\r",
                  sky_look[weather_info.sky],
                  weather_info.change >= 0
                  ? "a warm southerly breeze blows"
                  : "a cold northern gust blows");
        return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help(CHAR_DATA * ch, char *argument)
{
        char      argall[MAX_INPUT_LENGTH];
        char      argone[MAX_INPUT_LENGTH];
        char      argnew[MAX_INPUT_LENGTH];
        HELP_DATA *pHelp;
        int       lev;

        if (argument[0] == '\0')
                argument = "summary";

        if (isdigit(argument[0]))
        {
                lev = number_argument(argument, argnew);
                argument = argnew;
        }
        else
                lev = -2;
        /*
         * Tricky argument handling so 'help a b' doesn't match a.
         */
        argall[0] = '\0';
        while (argument[0] != '\0')
        {
                argument = one_argument(argument, argone);
                if (argall[0] != '\0')
                        mudstrlcat(argall, " ", MIL);
                mudstrlcat(argall, argone, MIL);
        }

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                if (pHelp->level > get_trust(ch))
                        continue;
                if (lev != -2 && pHelp->level != lev)
                        continue;

                if (is_name(argall, pHelp->keyword))
                        return pHelp;
        }

        return NULL;
}


/*  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles */
/*  Snippet by Senir.*/
sh_int str_similarity(const char *astr, const char *bstr)
{
        sh_int    matches = 0;

        if (!astr || !bstr)
                return matches;

/*    for ( ; *astr; astr++) - Gavin */
        for (;;)
        {
                if (astr == NULL || bstr == NULL)
                        break;
                astr++;

                if (LOWER((*astr)) == LOWER((*bstr)))
                        matches++;
                ++bstr;

                if ((*astr) == '\0' || (*bstr) == '\0')
                        break;
        }

        return matches;
}

/*  Ranks by number of matches until there's a nonmatching character between two words.*/
/*  Coded for the Similar Helpfiles Snippet by Senir.*/
sh_int str_prefix_level(const char *astr, const char *bstr)
{
        sh_int    matches = 0;

        if (!astr || !bstr)
                return matches;

        for (; *astr; astr++)
        {
                if (LOWER(*astr) == LOWER(*bstr))
                        matches++;
                else
                        return matches;

                if (++bstr == '\0')
                        return matches;
        }

        return matches;
}

/* Main function of Similar Helpfiles Snippet by Senir. It loops through all of the*/
/* helpfiles, using the string matching function defined to find the closest matching*/
/* helpfiles to the argument. It then checks for singles. Then, if matching helpfiles*/
/* are found at all, it loops through and prints out the closest matching helpfiles.*/
/* If its a single(there's only one), it opens the helpfile.*/
void similar_help_files(CHAR_DATA * ch, char *argument)
{
        HELP_DATA *pHelp = NULL;
        char      buf[MAX_STRING_LENGTH];
        char     *extension;
        sh_int    lvl = 0;
        bool      single = FALSE;


        send_to_pager_color("&C&BSimilar Help Files:\n\r", ch);

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                buf[0] = '\0';
                extension = pHelp->keyword;

                if (pHelp->level > get_trust(ch))
                        continue;

                while (extension[0] != '\0')
                {
                        extension = one_argument(extension, buf);

                        if (str_similarity(argument, buf) > lvl)
                        {
                                lvl = str_similarity(argument, buf);
                                single = TRUE;
                        }
                        else if (str_similarity(argument, buf) == lvl
                                 && lvl > 0)
                        {
                                single = FALSE;
                        }
                }
        }

        if (lvl == 0)
        {
                send_to_pager_color("&C&GNo similar help files.\n\r", ch);
                return;
        }

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                buf[0] = '\0';
                extension = pHelp->keyword;

                while (extension[0] != '\0')
                {
                        extension = one_argument(extension, buf);

                        if (str_similarity(argument, buf) >= lvl
                            && pHelp->level <= get_trust(ch))
                        {
                                if (single)
                                {
                                        send_to_pager_color
                                                ("&C&GOpening only similar helpfile.&C\n\r",
                                                 ch);
                                        do_help(ch, buf);
                                        return;
                                }

                                pager_printf(ch, "&C&G   %s\n\r",
                                             pHelp->keyword);
                                break;

                        }

                }
        }
        return;
}




/*
 * Now this is cleaner
 *
CMDF do_help(CHAR_DATA * ch, char *argument)
{
        HELP_DATA *pHelp;
        int       skill_number;
        CMDTYPE  *command;

        if ((pHelp = get_help(ch, argument)) == NULL)
        {
                append_file(ch, HELP_FILE, argument);

                ch_printf(ch, "&C&wNo help on \'%s\' found.\n\r", argument);
		if ( !is_number(argument)) - I don't think this works... - Gavin
			similar_help_files(ch, argument); 
                return;
        }
        set_pager_color(AT_YELLOW, ch);
        if (pHelp->level >= 0 && str_cmp(argument, "imotd"))
        {
                send_to_pager(pHelp->keyword, ch);
                send_to_pager("\n\r", ch);
        }

        sound_to_char(ch, "help");
        if ((skill_number = skill_lookup(argument)) >= 0 &&
            nifty_is_name(skill_table[skill_number]->name, pHelp->keyword))
                pager_printf(ch, "Skill level: %d\n\r",
                             skill_table[skill_number]->min_level);
        if ((command = find_command(argument)) != NULL
            && nifty_is_name(command->name, pHelp->keyword))
                pager_printf(ch, "Command level: %d\n\r", command->level);

        set_pager_color(AT_HELP, ch);
        if (pHelp->text[0] == '.')
                send_to_pager(pHelp->text + 1, ch);
        else
                send_to_pager(pHelp->text, ch);

        if (pHelp->author != NULL && pHelp->level >= 0 &&
            pHelp->date != NULL && str_cmp(argument, "imotd") &&
            str_cmp(argument, "amotd") && str_cmp(argument, "_diemsg_"))
        {
                send_to_pager("&B[ E&zdited by&B ]&R&W:&w", ch);
                send_to_pager(pHelp->author, ch);
                send_to_pager("     &B[ L&zast Edited on&B ]&R&W:&w ", ch);
                send_to_pager(pHelp->date, ch);
        }
        send_to_pager("\n\r", ch);
        return;
}*/
/*
 * Now this is cleaner
 */

/* Updated do_help command provided by Remcon of The Lands of Pabulum 03/20/2004 */
void do_help(CHAR_DATA * ch, char *argument)
{
        HELP_DATA *pHelp;
        char     *keyword;
        char      oneword[MSL], lastmatch[MSL];
        sh_int    matched = 0, checked = 0, totalmatched = 0, found = 0;

        set_pager_color(AT_HELP, ch);

        if (!argument || argument[0] == '\0')
                argument = "summary";

        if (!(pHelp = get_help(ch, argument)))
        {
                pager_printf(ch, "&wNo help on \'%s\' found.&D\n\r",
                             argument);
                send_to_pager("&BSuggested Help Files:&D\n\r", ch);
                mudstrlcpy(lastmatch, " ", MSL);
                for (pHelp = first_help; pHelp; pHelp = pHelp->next)
                {
                        matched = 0;
                        if (!pHelp || !pHelp->keyword
                            || pHelp->keyword[0] == '\0'
                            || pHelp->level > get_trust(ch))
                                continue;
                        keyword = pHelp->keyword;
                        while (keyword && keyword[0] != '\0')
                        {
                                matched = 0;    /* Set to 0 for each time we check lol */
                                keyword = one_argument(keyword, oneword);
                                /*
                                 * Lets check only up to 10 spots 
                                 */
                                for (checked = 0; checked <= 10; checked++)
                                {
                                        if (!oneword[checked]
                                            || !argument[checked])
                                                break;
                                        if (LOWER(oneword[checked]) ==
                                            LOWER(argument[checked]))
                                                matched++;
                                }
                                if ((matched > 1 && matched > (checked / 2))
                                    || (matched > 0 && checked < 2))
                                {
                                        pager_printf(ch, "&G %-20s &D\n\r",
                                                     oneword);
                                        mudstrlcpy(lastmatch, oneword, MSL);
                                        totalmatched++;
                                        break;
                                }
                        }
                }
                if (totalmatched == 0)
                {
                        send_to_pager("&C&GNo suggested help files.\n\r", ch);
                        return;
                }
                if (totalmatched == 1 && lastmatch != NULL
                    && lastmatch[0] != '\0')
                {
                        send_to_pager
                                ("&COpening only suggested helpfile.&D\n\r",
                                 ch);
                        do_help(ch, lastmatch);
                        return;
                }
                if (found > 0 && found <= 3)
                        send_to_pager("\n\r", ch);
                return;
        }

        if (IS_IMMORTAL(ch))
                pager_printf(ch, "Help level: %d\n\r", pHelp->level);

        set_pager_color(AT_HELP, ch);

        /*
         * Strip leading '.' to allow initial blanks.
         */
        if (pHelp->text[0] == '.')
                send_to_pager(pHelp->text + 1, ch);
        else
                send_to_pager(pHelp->text, ch);

        if (pHelp->author != NULL && pHelp->level >= 0 &&
            pHelp->date != NULL && str_cmp(argument, "imotd") &&
            str_cmp(argument, "amotd") && str_cmp(argument, "_diemsg_"))
        {
                send_to_pager("&B[ E&zdited by&B ]&R&W:&w", ch);
                send_to_pager(pHelp->author, ch);
                send_to_pager("     &B[ L&zast Edited on&B ]&R&W:&w ", ch);
                send_to_pager(pHelp->date, ch);
        }
}


/*
 * Help editor							-Thoric
 */
CMDF do_hedit(CHAR_DATA * ch, char *argument)
{
        HELP_DATA *pHelp;

        if (!ch->desc)
        {
                send_to_char("You have no descriptor.\n\r", ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                break;
        case SUB_HELP_EDIT:
                if ((pHelp = (HELP_DATA *) ch->dest_buf) == NULL)
                {
                        bug("hedit: sub_help_edit: NULL ch->dest_buf", 0);
                        stop_editing(ch);
                        return;
                }
/* Why not hash the author, date, and keywords? Some will show up more than once, why not?
   Plus copy_buffer returns a STRALLOC, not a str_dup, so we have to hash text as well, waste of time, but whatever */
                STRFREE(pHelp->text);
                pHelp->text = copy_buffer(ch);
                stop_editing(ch);
                stralloc_printf(&pHelp->author, "%s", ch->name);
                stralloc_printf(&pHelp->date, "%s", ctime(&current_time));
/*	  pHelp->author  = ch->name;
      pHelp->date    = ctime( &current_time );*/
                return;
        }
        if ((pHelp = get_help(ch, argument)) == NULL)   /* new help */
        {
                char      argnew[MAX_INPUT_LENGTH];
                int       lev;

                if (isdigit(argument[0]))
                {
                        lev = number_argument(argument, argnew);
                        argument = argnew;
                }
                else
                        lev = get_trust(ch);
                CREATE(pHelp, HELP_DATA, 1);
                pHelp->keyword = STRALLOC(strupper(argument));
                pHelp->text = STRALLOC("");
                pHelp->level = lev;
                stralloc_printf(&pHelp->author, "%s", ch->name);
                stralloc_printf(&pHelp->date, "%s", ctime(&current_time));
/*	pHelp->author  = ch->name;
	pHelp->date    = ctime( &current_time );*/
                add_help(pHelp);
        }
        ch->substate = SUB_HELP_EDIT;
        ch->dest_buf = pHelp;
        start_editing(ch, pHelp->text);
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char     *help_fix(char *text)
{
        char     *fixed;

        if (!text)
                return "";
        fixed = strip_cr(text);
        if (fixed[0] == ' ')
                fixed[0] = '.';
        return fixed;
}

void save_help(void)
{
        FILE     *fpout;
        HELP_DATA *pHelp;
        char      date[25];

        rename("help.are", "help.are.bak");
        FCLOSE(fpReserve);
        if ((fpout = fopen("help.are", "w")) == NULL)
        {
                bug("save_help: fopen", 0);
                perror("help.are");
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        fprintf(fpout, "#HELPS\n\n");
        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                /*
                 * Hackish attempt to remove the extra lines from the date strings) 
                 */
                snprintf(date, 25, "%s", pHelp->date);
                fprintf(fpout, "%d %s~\n%s~\n%s~\n%s~\n\n",
                        pHelp->level, pHelp->keyword, help_fix(pHelp->text),
                        pHelp->author, date);
        }
        fprintf(fpout, "0 $~\n\n\n#$\n");
        FCLOSE(fpout);
        fpReserve = fopen(NULL_FILE, "r");

}
CMDF do_hset(CHAR_DATA * ch, char *argument)
{
        HELP_DATA *pHelp;
        char      arg1[MAX_INPUT_LENGTH];

        smash_tilde(argument);
        argument = one_argument(argument, arg1);
        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: hset <help page> <field> [value]\n\r",
                             ch);
                send_to_char("\n\r", ch);
                send_to_char("Field being one of:\n\r", ch);
                send_to_char
                        ("  level keyword remove save show addkeyword\n\r",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "save"))
        {
                log_string_plus("Saving help.are...", LOG_NORMAL,
                                LEVEL_GREATER);
                save_help();
                send_to_char("Saved.\n\r", ch);
                return;
        }

        if ((pHelp = get_help(ch, arg1)) == NULL)
        {
                send_to_char("Cannot find help on that subject.\n\r", ch);
                return;
        }


        if (!str_cmp(argument, "remove"))
        {
                /*
                 * Since we removed it from delete_help, we need this here 
                 */
                UNLINK(pHelp, first_help, last_help, next, prev);
                delete_help(pHelp);
                send_to_char("Removed.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "show"))
        {
                ch_printf(ch, "&BL&zevel&B:&w %d\n\r", pHelp->level);
                ch_printf(ch, "&BK&zeywords&B:&w %s\n\r", pHelp->keyword);
                ch_printf(ch, "&BA&zuthor&B:&w %s\n\r", pHelp->author);
                ch_printf(ch, "&BD&zate&B:&w %s\n\r", pHelp->date);
                return;
        }

        argument = one_argument(argument, arg1);
        if (!str_cmp(arg1, "level"))
        {
                pHelp->level = atoi(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "keyword"))
        {
                STRFREE(pHelp->keyword);
                pHelp->keyword = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "addkeyword"))
        {
                char      buf[MSL];

                snprintf(buf, MSL, "%s %s", pHelp->keyword, argument);
                STRFREE(pHelp->keyword);
                pHelp->keyword = STRALLOC(buf);
                send_to_char("Done.\n\r", ch);
                return;
        }

        do_hset(ch, "");
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
CMDF do_hlist(CHAR_DATA * ch, char *argument)
{
        int       min, max, minlimit, maxlimit, cnt;
        char      arg[MAX_INPUT_LENGTH];
        HELP_DATA *help;

        maxlimit = get_trust(ch);
        minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
        argument = one_argument(argument, arg);
        if (arg[0] != '\0')
        {
                min = URANGE(minlimit, atoi(arg), maxlimit);
                if (argument[0] != '\0')
                        max = URANGE(min, atoi(argument), maxlimit);
                else
                        max = maxlimit;
        }
        else
        {
                min = minlimit;
                max = maxlimit;
        }
        set_pager_color(AT_GREEN, ch);
        pager_printf(ch, "Help Topics in level range %d to %d:\n\r\n\r", min,
                     max);
        for (cnt = 0, help = first_help; help; help = help->next)
                if (help->level >= min && help->level <= max)
                {
                        one_argument(help->keyword, arg);
                        if (IS_MXP(ch))
                                pager_printf(ch,
                                             "   %3d " MXPTAG("help '%s'")
                                             "%s" MXPTAG("/help") "\n\r",
                                             help->level, arg, help->keyword);
                        else
                                pager_printf(ch, "   %3d %s\n\r",
                                             help->level, help->keyword);
                        ++cnt;
                }
        if (cnt)
                pager_printf(ch, "\n\r%d pages found.\n\r", cnt);
        else
                send_to_char("None found.\n\r", ch);
}


/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 * completely removing race and clan, what the hells with that.... - Greven
 *
 * This Homepage Support doesn't actually do anything.. Should we enable it? - Gavin
 */
CMDF do_who(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      invis_str[MAX_INPUT_LENGTH];
        char      extra_title[MAX_STRING_LENGTH];
        char      race_text[MAX_INPUT_LENGTH];
        char      clan_name[MAX_INPUT_LENGTH];
        DESCRIPTOR_DATA *d;
        int       iLevelLower;
        int       iLevelUpper;
        int       nNumber;
        int       nMatch;
        bool      fImmortalOnly;
        FILE     *whoout = NULL;

        /*
         * #define WT_IMM    0;
         * #define WT_MORTAL 1;
         */

        WHO_DATA *cur_who = NULL;
        WHO_DATA *next_who = NULL;
        WHO_DATA *first_mortal = NULL;
        WHO_DATA *first_newbie = NULL;
        WHO_DATA *first_imm = NULL;

        argument = NULL;
        /*
         * Set default arguments.
         */
        iLevelLower = 0;
        iLevelUpper = MAX_LEVEL;
        fImmortalOnly = FALSE;


        /*
         * Parse arguments.
         */
        nNumber = 0;

        /*
         * Now find matching chars.
         */
        nMatch = 0;
        buf[0] = '\0';
        if (ch)
                set_pager_color(AT_GREEN, ch);
#ifdef WEB
        else
        {
                whoout = fopen(WHO_FILE, "w");
                if (whoout)
                {
                        fprintf(whoout,
                                "<html><head><title> Who List</title><style> pre { display: inline; }  a:link { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:visited { background-color: inherit; text-decoration: none; color: inherit; }\n\ra:hover { background-color: inherit; text-decoration: none; font-weight: bold; color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; color: inherit; }</style></head>\n\r");
                        fprintf(whoout,
                                "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r");
                        fprintf(whoout, "<h1>Who List</h1><br>\n\r");
                        fprintf(whoout,
                                "<table cellpadding=0 cellspacing=0>\n\r");
                }
        }
#endif		

/* start from last to first to get it in the proper order */
        for (d = last_descriptor; d; d = d->prev)
        {
                CHAR_DATA *wch;
                char const *race;

                wch = CH(d);
                if ((!IS_PLAYING(d) && d->connected != CON_EDITING)
                    || (!can_see_ooc(ch, wch)
                        && IS_IMMORTAL(wch)) || d->original)
                        continue;
                if (wch->top_level < iLevelLower
                    || wch->top_level > iLevelUpper
                    || (fImmortalOnly && wch->top_level < LEVEL_IMMORTAL))
                        continue;
                if (IS_NPC(wch))
                        continue;


                /*****************************************
                 **	Showing Members Of Wch's Clan    	**
                 **	Using "clan_name" in who list    	**
                 ******************************************/
                clan_name[0] = '\0'; /* Reset this so it won't print on others */
                if ( ch && !IS_NPC(ch) && wch->pcdata->clan && (ch->pcdata->clan || IS_IMMORTAL(ch)))
                {
                        CLAN_DATA *pclan;
                        CLAN_DATA *zclan;

                        if ( wch->pcdata->clan ) 
                        {
                                pclan = wch->pcdata->clan;
                                if (pclan->mainclan) pclan = pclan->mainclan;
                        }

                        if ( ch->pcdata->clan ) { 
                                zclan = ch->pcdata->clan;
                                if (zclan->mainclan) zclan = zclan->mainclan;
                        }

                        if ( IS_IMMORTAL(ch) || (pclan && zclan && pclan == zclan) )  
                        {
                                if ( !str_cmp( wch->name, wch->pcdata->clan->leader ) )
                                {
                                        snprintf(clan_name, MSL, " &R(&BL&zeader,&B %c&z%s&R)&W",
                                                        wch->pcdata->clan->name[0],
                                                        (wch->pcdata->clan->name+1)
                                                );
                                }
                                else
                                {
                                        snprintf(clan_name, MSL, " &R(&B%c&z%s,&B %c&z%s&R)&W",
                                                        wch->pcdata->clan->rank[wch->pcdata->clanrank][0],
                                                        (wch->pcdata->clan->rank[wch->pcdata->clanrank]+1),
                                                        wch->pcdata->clan->name[0],
                                                        (wch->pcdata->clan->name+1)
                                                );
                                } 

                        } 
                }
                nMatch++;

                snprintf(race_text, MSL, "(%s) ", wch->race->name());
                race = race_text;

                switch (wch->top_level)
                {
                default:
                        break;
                case 200:
                        race = "The Ghost in the Machine";
                        break;
                case MAX_LEVEL - 0:
                        race = "Owner";
                        break;
                case MAX_LEVEL - 1:
                        race = "Admin";
                        break;
                case MAX_LEVEL - 2:
                        race = "Head Builder";
                        break;
                case MAX_LEVEL - 3:
                        race = "Builder";
                        break;
                case MAX_LEVEL - 4:
                        race = "Enforcer";
                        break;
                }

                if (!ch && wch->pcdata->homepage
                    && wch->pcdata->homepage[0] != '\0')
                {
                        if (get_trust(ch) > get_trust(wch)
                            && !nifty_is_name(wch->name,
                                              smash_color(wch->pcdata->
                                                          title)))
                                snprintf(extra_title, MSL,
                                         "<a href=\"%s\" target=_blank>%s [%s]</a>",
                                         wch->pcdata->homepage,
                                         wch->pcdata->title, wch->name);
                        else
                                snprintf(extra_title, MSL,
                                         "<a href=\"%s\" target=_blank>%s</a>",
                                         wch->pcdata->homepage,
                                         wch->pcdata->title);
                }
                else
                {
                        if (get_trust(ch) > get_trust(wch)
                            && !nifty_is_name(wch->name,
                                              smash_color(wch->pcdata->
                                                          title)))
                                snprintf(extra_title, MSL, "%s [%s]",
                                         wch->pcdata->title, wch->name);
                        else
                                mudstrlcpy(extra_title, wch->pcdata->title,
                                           MSL);
                }


                if (IS_RETIRED(wch))
                        race = "Retired";
                else if (IS_GUEST(wch))
                        race = "Guest";
                else if (IS_SET(wch->pcdata->flags, PCFLAG_NEWBGUIDE))
                        race = "&b[&zNewbie Guide&b]&D";
                else if (wch->pcdata->rank && wch->pcdata->rank[0] != '\0')
                        race = wch->pcdata->rank;


                if (IS_SET(wch->act, PLR_WIZINVIS))
                        snprintf(invis_str, 10, "(%d) ",
                                 wch->pcdata->wizinvis);
                else
                        invis_str[0] = '\0';

                snprintf(buf, MSL, "%s &W%s%s%s&W%s%s%s%s%s&w",
                         race,
                         invis_str,
                         NOT_AUTHED(wch) ? "&BN&W " : "",
                         IS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
                         extra_title,
                         clan_name,
                         IS_SET(wch->pcdata->flags, PCFLAG_WORKING) ? "&Y [&RWORKING&Y]&W" :"&W",
                         IS_SET(wch->act, PLR_SILENCE) ? "&Y [&BS&zilenced&Y]&W" : "&W",
                         wch->desc->connected == CON_EDITING ? "&Y [&cWRITING&Y]" : (wch->desc->
                                                             connected ==
                                                             CON_IAFORKED
                                                             || wch->desc->
                                                             connected ==
                                                             CON_FORKED) ?
                         "&Y [&cCOMPILING&Y]" : "");

                /*
                 * This is where the old code would display the found player to the ch.
                 * What we do instead is put the found data into a linked list
                 */

                /*
                 * First make the structure. 
                 */
                CREATE(cur_who, WHO_DATA, 1);
                cur_who->text = str_dup(buf);
                if (IS_IMMORTAL(wch))
                        cur_who->type = WT_IMM;
                else if (get_trust(wch) <= 10)
                        cur_who->type = WT_NEWBIE;
                else
                        cur_who->type = WT_MORTAL;

                /*
                 * Then put it into the appropriate list. 
                 */
                switch (cur_who->type)
                {
                case WT_MORTAL:
                        cur_who->next = first_mortal;
                        first_mortal = cur_who;
                        break;
                case WT_IMM:
                        cur_who->next = first_imm;
                        first_imm = cur_who;
                        break;
                case WT_NEWBIE:
                        cur_who->next = first_newbie;
                        first_newbie = cur_who;
                        break;
                default:
                        {   /* Really should be using enums */
                        }
                }

        }
        buf[0] = '\0';
        race_text[0] = '\0';

        /*
         * Ok, now we have three separate linked lists and what remains is to 
         * * display the information and clean up.
         */

        /*
         * Deadly list removed for swr ... now only 2 lists 
         */

        if (first_newbie)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Newbies &z]&B---------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
                else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                        {
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                        }
                }
#endif
		}

        for (cur_who = first_newbie; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }


        if (first_mortal)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Players &z]&B---------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
                else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
#endif
        }

        for (cur_who = first_mortal; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }

        if (first_imm)
        {
                mudstrlcpy(race_text,
                           "\n\r&B----------------------------&z[&w Dark Warrior Immortals &z]&B-------------------------&W\n\r\n\r",
                           MIL);
                if (ch)
                        send_to_pager(race_text, ch);
#ifdef WEB
				else
                {
                        web_colourconv(buf, race_text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
#endif
        }

        for (cur_who = first_imm; cur_who; cur_who = next_who)
        {
#ifdef WEB
                if (!ch)
                {
                        web_colourconv(buf, cur_who->text);
                        if (whoout)
                                fprintf(whoout,
                                        "<tr><td><pre>%s</pre></td></tr>\n\r",
                                        buf);
                }
                else
#endif
                {
                        send_to_pager(cur_who->text, ch);
                        send_to_pager("\n\r", ch);
                }
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
        }
#ifdef WEB
        if (!ch && whoout)
        {
                fprintf(whoout, "</table>\n\r");
                fprintf(whoout,
                        "<font color=#FFFF00><pre> %d player%s</pre></font>\n",
                        nMatch, nMatch == 1 ? "" : "s");
                fprintf(whoout, "</body></html>\n\n");
                FCLOSE(whoout);
        }
#endif
        if (!ch)
                return;

        set_char_color(AT_YELLOW, ch);
        ch_printf(ch, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s");
        return;
}


CMDF do_compare(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        OBJ_DATA *obj1;
        OBJ_DATA *obj2;
        int       value1;
        int       value2;
        char     *msg;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (arg1[0] == '\0')
        {
                send_to_char("Compare what to what?\n\r", ch);
                return;
        }

        if ((obj1 = get_obj_carry(ch, arg1)) == NULL)
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                for (obj2 = ch->first_carrying; obj2;
                     obj2 = obj2->next_content)
                {
                        if (obj2->wear_loc != WEAR_NONE
                            && can_see_obj(ch, obj2)
                            && obj1->item_type == obj2->item_type
                            && (obj1->wear_flags & obj2->
                                wear_flags & ~ITEM_TAKE) != 0)
                                break;
                }

                if (!obj2)
                {
                        send_to_char
                                ("You aren't wearing anything comparable.\n\r",
                                 ch);
                        return;
                }
        }
        else
        {
                if ((obj2 = get_obj_carry(ch, arg2)) == NULL)
                {
                        send_to_char("You do not have that item.\n\r", ch);
                        return;
                }
        }

        msg = NULL;
        value1 = 0;
        value2 = 0;

        if (obj1 == obj2)
        {
                msg = "You compare $p to itself.  It looks about the same.";
        }
        else if (obj1->item_type != obj2->item_type)
        {
                msg = "You can't compare $p and $P.";
        }
        else
        {
                switch (obj1->item_type)
                {
                default:
                        msg = "You can't compare $p and $P.";
                        break;

                case ITEM_ARMOR:
                        value1 = obj1->value[0];
                        value2 = obj2->value[0];
                        break;

                case ITEM_WEAPON:
                        value1 = obj1->value[1] + obj1->value[2];
                        value2 = obj2->value[1] + obj2->value[2];
                        break;
                }
        }

        if (!msg)
        {
                if (value1 == value2)
                        msg = "$p and $P look about the same.";
                else if (value1 > value2)
                        msg = "$p looks better than $P.";
                else
                        msg = "$p looks worse than $P.";
        }

        act(AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR);
        return;
}



CMDF do_where(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        DESCRIPTOR_DATA *d;
        bool      found;

        if (get_trust(ch) < LEVEL_IMMORTAL)
        {
                send_to_char("If only life were really that simple...\n\r",
                             ch);
                return;
        }

        one_argument(argument, arg);

        set_pager_color(AT_PERSON, ch);
        if (arg[0] == '\0')
        {
                if (get_trust(ch) >= LEVEL_IMMORTAL)
                        send_to_pager("Players logged in:\n\r", ch);
                else
                        pager_printf(ch, "Players near you in %s:\n\r",
                                     ch->in_room->area->name);
                found = FALSE;
                for (d = first_descriptor; d; d = d->next)
                        if ((IS_PLAYING(d) || d->connected == CON_EDITING)
                            && (victim = d->character) != NULL
                            && !IS_NPC(victim)
                            && victim->in_room
                            && (victim->in_room->area == ch->in_room->area
                                || get_trust(ch) >= LEVEL_IMMORTAL)
                            && can_see(ch, victim))
                        {
                                found = TRUE;
                                pager_printf(ch, "%-28s %s\n\r",
                                             victim->name,
                                             victim->in_room->name);
                        }
                if (!found)
                        send_to_char("None\n\r", ch);
        }
        else
        {
                found = FALSE;
                for (victim = first_char; victim; victim = victim->next)
                        if (victim->in_room
                            && victim->in_room->area == ch->in_room->area
                            && !IS_AFFECTED(victim, AFF_HIDE)
                            && !IS_AFFECTED(victim, AFF_SNEAK)
                            && can_see(ch, victim)
                            && is_name(arg, victim->name))
                        {
                                found = TRUE;
                                pager_printf(ch, "%-28s %s\n\r",
                                             PERS(victim, ch),
                                             victim->in_room->name);
                                break;
                        }
                if (!found)
                        act(AT_PLAIN, "You didn't find any $T.", ch, NULL,
                            arg, TO_CHAR);
        }

        return;
}




CMDF do_consider(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        char     *msg;
        int       diff;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Consider killing whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

        diff = (victim->top_level - ch->top_level) * 10;

        diff += (int) (victim->max_hit - ch->max_hit) / 10;

        if (diff <= -200)
                msg = "$N looks like a feather!";
        else if (diff <= -150)
                msg = "Hey! Where'd $N go?";
        else if (diff <= -100)
                msg = "Easy as picking off womp rats at beggers canyon!";
        else if (diff <= -50)
                msg = "$N is a wimp.";
        else if (diff <= 0)
                msg = "$N looks weaker than you.";
        else if (diff <= 50)
                msg = "$N looks about as strong as you.";
        else if (diff <= 100)
                msg = "It would take a bit of luck...";
        else if (diff <= 150)
                msg = "It would take a lot of luck, and a really big blaster!";
        else if (diff <= 200)
                msg = "Why don't you just attack a star destoyer with a vibroblade?";
        else
                msg = "$N is built like an AT-AT!";
        act(AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR);

        return;
}



/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

CMDF do_practice(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_STRING_LENGTH];
        int       sn, classtype = -1, iClass;
        bool      parts = FALSE;

        if (IS_NPC(ch))
                return;

        one_argument(argument, arg);
        if (argument[0] == '\0' || !str_cmp("class", arg))
        {
                int       col;
                sh_int    lasttype, cnt;

                if (!str_cmp("class", arg))
                {
                        argument = one_argument(argument, arg); /* Strip away the word class into arg */
                        argument = one_argument(argument, arg); /* Fill arg with class name, what we want */
                        if (arg && arg[0] != '\0')
                        {
                                for (iClass = 0; iClass < MAX_ABILITY;
                                     iClass++)
                                {
                                        if (!str_prefix
                                            (arg, ability_name[iClass]))
                                        {
                                                classtype = iClass;
                                                break;
                                        }
                                }
                        }
                }
                col = cnt = 0;
                lasttype = SKILL_SPELL;
                set_pager_color(AT_MAGIC, ch);
                for (sn = 0; sn < top_sn; sn++)
                {
                        parts = FALSE;
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        if (strcmp(skill_table[sn]->name, "reserved") == 0
                            && (IS_IMMORTAL(ch)))
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                send_to_pager
                                        ("&B--------------------------------[&zSpells&B]---------------------------------\n\r",
                                         ch);
                                send_to_pager("&w", ch);
                                col = 0;
                        }
                        if (skill_table[sn]->type != lasttype)
                        {
                                if (col % 3 != 0)
                                        send_to_pager("\n\r", ch);
                                if (skill_table[sn]->type != SKILL_UNKNOWN)
                                        pager_printf(ch,
                                                     "&B--------------------------------&B[&z%ss&B]---------------------------------\n\r",
                                                     skill_tname[skill_table
                                                                 [sn]->type]);
                                col = cnt = 0;
                        }

                        lasttype = skill_table[sn]->type;

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s", ch->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        continue;
                                }
                        }
                        if (!xIS_EMPTY(skill_table[sn]->body_parts)
                            && !xIS_EMPTY(ch->xflags))
                        {
                                sh_int    i = 0;

                                for (i = 0; i < MAX_BITS; i++)
                                        if (xIS_SET
                                            (skill_table[sn]->body_parts, i)
                                            && xIS_SET(ch->xflags, i))
                                                parts = TRUE;
                        }
                        if (!xIS_EMPTY(skill_table[sn]->body_parts)
                            && parts == FALSE)
                                continue;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        if (classtype != -1
                            && skill_table[sn]->guild != classtype)
                                continue;

                        if (ch->pcdata->learned[sn] <= 0
                            && ch->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                                continue;

                        if (ch->pcdata->learned[sn] == 0
                            && SPELL_FLAG(skill_table[sn], SF_SECRETSKILL))
                                continue;

                        ++cnt;
                        if (ch->pcdata->learned[sn] >= 100)
                        {
                                if (IS_MXP(ch))
                                {
                                        pager_printf(ch,
                                                     "&w"
                                                     MXPTAG("practice %s")
                                                     "%18.18s"
                                                     MXPTAG("/practice")
                                                     " &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     skill_table[sn]->name,
                                                     ch->pcdata->learned[sn]);
                                }
                                else
                                {
                                        pager_printf(ch,
                                                     "&w%18.18s &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     ch->pcdata->learned[sn]);
                                }
                        }
                        else
                        {
                                if (IS_MXP(ch))
                                {
                                        pager_printf(ch,
                                                     "&z"
                                                     MXPTAG("practice %s")
                                                     "%18.18s"
                                                     MXPTAG("/practice")
                                                     " &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     skill_table[sn]->name,
                                                     ch->pcdata->learned[sn]);
                                }
                                else
                                {
                                        pager_printf(ch,
                                                     "&z%18.18s &W%3.3d%%  ",
                                                     skill_table[sn]->name,
                                                     ch->pcdata->learned[sn]);
                                }
                        }
                        if (++col % 3 == 0)
                                send_to_pager("\n\r", ch);
                }

                if (col % 3 != 0)
                        send_to_pager("\n\r", ch);

        }
        else
        {
                CHAR_DATA *mob;
                int       adept;
                bool      can_prac = TRUE;

                if (!IS_AWAKE(ch))
                {
                        send_to_char("In your dreams, or what?\n\r", ch);
                        return;
                }

                for (mob = ch->in_room->first_person; mob;
                     mob = mob->next_in_room)
                        if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
                                break;

                if (!mob)
                {
                        send_to_char("You can't do that here.\n\r", ch);
                        return;
                }


                sn = skill_lookup(argument);

                if (sn == -1)
                {
                        act(AT_TELL,
                            "$n tells you 'I've never heard of that one...'",
                            mob, NULL, ch, TO_VICT);
                        return;
                }

                if (skill_table[sn]->guild < 0
                    || skill_table[sn]->guild >= MAX_ABILITY)
                {
                        act(AT_TELL,
                            "$n tells you 'I cannot teach you that...'", mob,
                            NULL, ch, TO_VICT);
                        return;
                }

                if (can_prac && !IS_NPC(ch)
                    && ch->skill_level[skill_table[sn]->guild] <
                    skill_table[sn]->min_level)
                {
                        act(AT_TELL,
                            "$n tells you 'You're not ready to learn that yet...'",
                            mob, NULL, ch, TO_VICT);
                        return;
                }

                if (is_name(skill_tname[skill_table[sn]->type], CANT_PRAC))
                {
                        act(AT_TELL,
                            "$n tells you 'I do not know how to teach that.'",
                            mob, NULL, ch, TO_VICT);
                        return;
                }

                /*
                 * Skill requires a special teacher
                 */
                if (skill_table[sn]->teachers
                    && skill_table[sn]->teachers[0] != '\0')
                {
                        snprintf(buf, MSL, "%d", mob->pIndexData->vnum);
                        if (!is_name(buf, skill_table[sn]->teachers))
                        {
                                act(AT_TELL,
                                    "$n tells you, 'I know not know how to teach that.'",
                                    mob, NULL, ch, TO_VICT);
                                return;
                        }
                }
                else
                {
                        act(AT_TELL,
                            "$n tells you, 'I know not know how to teach that.'",
                            mob, NULL, ch, TO_VICT);
                        return;
                }

                if (skill_table[sn]->races
                    && skill_table[sn]->races[0] != '\0')
                {
                        snprintf(buf, MSL, "%s", ch->race->name());
                        if (!is_name(buf, skill_table[sn]->races))
                        {
                                act(AT_TELL,
                                    "$n tells you, 'You are not the right race to learn that skill.'",
                                    mob, NULL, ch, TO_VICT);
                                return;
                        }
                }


                adept = 20;

                if (ch->gold < skill_table[sn]->min_level * 10)
                {
                        snprintf(buf, MSL,
                                 "$n tells you, 'I charge %d credits to teach that. You don't have enough.'",
                                 skill_table[sn]->min_level * 10);
                        act(AT_TELL,
                            "$n tells you 'You don't have enough credits.'",
                            mob, NULL, ch, TO_VICT);
                        return;
                }

                if (ch->pcdata->learned[sn] >= adept)
                {
                        snprintf(buf, MSL,
                                 "$n tells you, 'I've taught you everything I can about %s.'",
                                 skill_table[sn]->name);
                        act(AT_TELL, buf, mob, NULL, ch, TO_VICT);
                        act(AT_TELL,
                            "$n tells you, 'You'll have to practice it on your own now...'",
                            mob, NULL, ch, TO_VICT);
                }
                else
                {
                        ch->gold -= skill_table[sn]->min_level * 10;
                        ch->pcdata->learned[sn] +=
                                int_app[get_curr_int(ch)].learn;
                        act(AT_ACTION, "You practice $T.", ch, NULL,
                            skill_table[sn]->name, TO_CHAR);
                        act(AT_ACTION, "$n practices $T.", ch, NULL,
                            skill_table[sn]->name, TO_ROOM);
                        if (ch->pcdata->learned[sn] >= adept)
                        {
                                ch->pcdata->learned[sn] = adept;
                                act(AT_TELL,
                                    "$n tells you. 'You'll have to practice it on your own now...'",
                                    mob, NULL, ch, TO_VICT);
                        }
                }
        }
        return;
}

CMDF do_teach(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        int       sn;
        char      arg[MAX_INPUT_LENGTH];

        if (IS_NPC(ch))
                return;

        switch (ch->tempnum)
        {
        default:
                argument = one_argument(argument, arg);

                if (argument[0] == '\0')
                {
                        send_to_char("Teach who, what?\n\r", ch);
                        return;
                }
                else
                {
                        CHAR_DATA *victim;
                        int       adept;

                        if (!IS_AWAKE(ch))
                        {
                                send_to_char("In your dreams, or what?\n\r",
                                             ch);
                                return;
                        }

                        if ((victim = get_char_room(ch, arg)) == NULL)
                        {
                                send_to_char
                                        ("They don't seem to be here...\n\r",
                                         ch);
                                return;
                        }

                        if (IS_NPC(victim))
                        {
                                send_to_char
                                        ("You can't teach that to them!\n\r",
                                         ch);
                                return;
                        }

                        sn = skill_lookup(argument);

                        if (sn == -1)
                        {
                                act(AT_TELL, "You have no idea what that is.",
                                    victim, NULL, ch, TO_VICT);
                                return;
                        }

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                        {
                                act(AT_TELL,
                                    "Thats just not going to happen.", victim,
                                    NULL, ch, TO_VICT);
                                return;
                        }

                        if (is_name
                            (skill_tname[skill_table[sn]->type], CANT_PRAC))
                        {
                                act(AT_TELL,
                                    "You are unable to teach that skill.",
                                    victim, NULL, ch, TO_VICT);
                                return;
                        }

                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                        {
                                snprintf(buf, MSL, "%s",
                                         victim->race->name());
                                if (!is_name(buf, skill_table[sn]->races))
                                {
                                        act(AT_TELL,
                                            "They are not the right race to learn that skill.",
                                            victim, NULL, ch, TO_VICT);
                                        return;
                                }
                        }

                        if (ch->pcdata->learned[sn] < 100)
                        {
                                act(AT_TELL,
                                    "You must perfect that yourself before teaching others.",
                                    victim, NULL, ch, TO_VICT);
                                return;
                        }

                        add_request(ch, victim, argument, "teach");
                        return;

        case TRUE: // Can't be reached without consent.
                        victim = (CHAR_DATA *) ch->dest_buf;    // Should exist, this is a triggered response caused by victim.
                        sn = skill_lookup(argument);    // argument has already been parsed, should be fine.

                        if (victim->skill_level[skill_table[sn]->guild] <
                            skill_table[sn]->min_level)
                        {
                                act(AT_TELL,
                                    "$n isn't ready to learn that yet.",
                                    victim, NULL, ch, TO_VICT);
                                return;
                        }



                        adept = 20;

                        if (victim->pcdata->learned[sn] >= adept)
                        {
                                act(AT_TELL,
                                    "$n must practice that on their own.",
                                    victim, NULL, ch, TO_VICT);
                                return;
                        }
/*                else if (!str_cmp(ch->race->name(), "verpine"))
                {
                        victim->pcdata->learned[sn] += 99;
                        snprintf(buf, MSL, "You teach %s $T.", victim->name);
                        act(AT_ACTION, buf,
                            ch, NULL, skill_table[sn]->name, TO_CHAR);
                        snprintf(buf, MSL, "%s teaches you $T.", ch->name);
                        act(AT_ACTION, buf,
                            victim, NULL, skill_table[sn]->name, TO_CHAR);
                }*/
                        else
                        {
                                victim->pcdata->learned[sn] +=
                                        int_app[get_curr_int(ch)].learn;
                                snprintf(buf, MSL, "You teach %s $T.",
                                         victim->name);
                                act(AT_ACTION, buf, ch, NULL,
                                    skill_table[sn]->name, TO_CHAR);
                                snprintf(buf, MSL, "%s teaches you $T.",
                                         ch->name);
                                act(AT_ACTION, buf, victim, NULL,
                                    skill_table[sn]->name, TO_CHAR);
                        }
                }
                return;
        }
}


CMDF do_wimpy(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       wimpy;

        one_argument(argument, arg);

        if (arg[0] == '\0')
                wimpy = (int) ch->max_hit / 5;
        else
                wimpy = atoi(arg);

        if (wimpy < 0)
        {
                send_to_char("Your courage exceeds your wisdom.\n\r", ch);
                return;
        }

        if (wimpy > ch->max_hit)
        {
                send_to_char("Such cowardice ill becomes you.\n\r", ch);
                return;
        }

        ch->wimpy = wimpy;
        ch_printf(ch, "Wimpy set to %d hit points.\n\r", wimpy);
        return;
}



CMDF do_password(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char     *pArg;
        char     *pwdnew;
        char     *p;
        char      cEnd;

        if (IS_NPC(ch))
                return;

        /*
         * Can't use one_argument here because it smashes case.
         * So we just steal all its code.  Bleagh.
         */
        pArg = arg1;
        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0')
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *pArg++ = *argument++;
        }
        *pArg = '\0';

        pArg = arg2;
        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0')
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *pArg++ = *argument++;
        }
        *pArg = '\0';

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Syntax: password <old> <new>.\n\r", ch);
                return;
        }

        if (strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd))
        {
                WAIT_STATE(ch, 40);
                send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
                return;
        }

        if (strlen(arg2) < 5)
        {
                send_to_char
                        ("New password must be at least five characters long.\n\r",
                         ch);
                return;
        }
        if (arg1[0] == '!' || arg2[0] == '!')
        {
                send_to_char
                        ("New password cannot begin with the '!' character.",
                         ch);
                return;
        }

        /*
         * No tilde allowed because of player file format.
         */
        pwdnew = crypt(arg2, ch->name);
        for (p = pwdnew; *p != '\0'; p++)
        {
                if (*p == '~')
                {
                        send_to_char
                                ("New password not acceptable, try again.\n\r",
                                 ch);
                        return;
                }
        }

        DISPOSE(ch->pcdata->pwd);
        ch->pcdata->pwd = str_dup(pwdnew);
        if (IS_SET(sysdata.save_flags, SV_PASSCHG))
                save_char_obj(ch);
        send_to_char("Ok.\n\r", ch);
        return;
}



CMDF do_socials(CHAR_DATA * ch, char *argument)
{
        int       iHash;
        int       col = 0;
        SOCIALTYPE *social;

        argument = NULL;

        set_pager_color(AT_PLAIN, ch);
        for (iHash = 0; iHash < 27; iHash++)
                for (social = social_index[iHash]; social;
                     social = social->next)
                {
                        if (social->minarousal == 0)
                        {
                                pager_printf(ch, "%-12s", social->name);
                                if (++col % 6 == 0)
                                        send_to_pager("\n\r", ch);
                        }
                }

        if (col % 6 != 0)
                send_to_pager("\n\r", ch);
        return;
}



CMDF do_commands(CHAR_DATA * ch, char *argument)
{
        int       col;
        int       hash;
        CMDTYPE  *command;

        argument = NULL;

        col = 0;
        set_pager_color(AT_PLAIN, ch);
        for (hash = 0; hash < 126; hash++)
                for (command = command_hash[hash]; command;
                     command = command->next)
                        if (command->level < LEVEL_HERO
                            && command->level <= get_trust(ch)
                            && (command->name[0] != 'm'
                                || command->name[1] != 'p'))
                        {
                                pager_printf(ch,
                                             "&B[&w%-3d&B][&z%s&B]&z %-12.12s",
                                             command->level, get_help(ch,
                                                                      command->
                                                                      name) ?
                                             "H" : " ", command->name);
                                if (++col % 4 == 0)
                                        send_to_pager("\n\r", ch);
                        }
        if (col % 4 != 0)
                send_to_pager("\n\r", ch);

        return;
}

/*
 * display WIZLIST file						-Thoric
 */
CMDF do_wizlist(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        set_pager_color(AT_IMMORT, ch);
        show_file(ch, WIZLIST_FILE);
}

CMDF do_showhelp(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        set_pager_color(AT_IMMORT, ch);
        show_file(ch, HELP_FILE);
}

CMDF do_showlog(CHAR_DATA * ch, char *argument)
{
        bool      clear = FALSE;
        char      arg[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg);
        set_pager_color(AT_IMMORT, ch);

        if (arg[0] == '\0')
        {
                send_to_char("Show which log file?.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "clear"))
        {
                clear = TRUE;
                argument = one_argument(argument, arg);
        }

        if (!str_cmp(arg, "help"))
        {
                if (clear)
                        clear_file(ch, HELP_FILE);
                else
                        show_file(ch, HELP_FILE);
        }
        else if (!str_cmp(arg, "boot"))
        {
                if (clear)
                        clear_file(ch, BOOTLOG_FILE);
                else
                        show_file(ch, BOOTLOG_FILE);
        }
        else if (!str_cmp(arg, "usage"))
        {
                if (clear)
                        clear_file(ch, USAGE_FILE);
                else
                        show_file(ch, USAGE_FILE);
        }
        else if (!str_cmp(arg, "log"))
        {
                if (clear)
                        clear_file(ch, LOG_FILE);
                else
                        show_file(ch, LOG_FILE);
        }
        else if (!str_cmp(arg, "bug"))
        {
                if (clear)
                        clear_file(ch, BUGS_FILE);
                else
                        show_file(ch, BUGS_FILE);
        }
        else if (!str_cmp(arg, "idea"))
        {
                if (clear)
                        clear_file(ch, IDEA_FILE);
                else
                        show_file(ch, IDEA_FILE);
        }
        else if (!str_cmp(arg, "typo"))
        {
                if (clear)
                        clear_file(ch, TYPO_FILE);
                else
                        show_file(ch, TYPO_FILE);
        }
        else
        {
                send_to_char("Unknown File.\n\r", ch);
        }
        return;

}

/*
 * Contributed by Grodyn.
 */
CMDF do_config(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        if (IS_NPC(ch))
                return;

        one_argument(argument, arg);

        set_char_color(AT_WHITE, ch);
        if (arg[0] == '\0')
        {
                send_to_char("[ Keyword  ] Option\n\r", ch);

                send_to_char(IS_SET(ch->act, PLR_FLEE)
                             ?
                             "[+FLEE     ] You flee if you get attacked.\n\r"
                             :
                             "[-flee     ] You fight back if you get attacked.\n\r",
                             ch);

                send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_NORECALL)
                             ?
                             "[+NORECALL ] You fight to the death, link-dead or not.\n\r"
                             :
                             "[-norecall ] You try to recall if fighting link-dead.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_AUTOEXIT)
                             ? "[+AUTOEXIT ] You automatically see exits.\n\r"
                             :
                             "[-autoexit ] You don't automatically see exits.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_AUTOLOOT)
                             ?
                             "[+AUTOLOOT ] You automatically loot corpses.\n\r"
                             :
                             "[-autoloot ] You don't automatically loot corpses.\n\r",
                             ch);


                send_to_char(IS_SET(ch->act, PLR_AUTOGOLD)
                             ?
                             "[+AUTOCRED ] You automatically split credits from kills in groups.\n\r"
                             :
                             "[-autocred ] You don't automatically split credits from kills in groups.\n\r",
                             ch);

                send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_GAG)
                             ?
                             "[+GAG      ] You see only necessary battle text.\n\r"
                             : "[-gag      ] You see full battle text.\n\r",
                             ch);

                send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_PAGERON)
                             ? "[+PAGER    ] Long output is page-paused.\n\r"
                             :
                             "[-pager    ] Long output scrolls to the end.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_BLANK)
                             ?
                             "[+BLANK    ] You have a blank line before your prompt.\n\r"
                             :
                             "[-blank    ] You have no blank line before your prompt.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_BRIEF)
                             ? "[+BRIEF    ] You see brief descriptions.\n\r"
                             : "[-brief    ] You see long descriptions.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_COMBINE)
                             ?
                             "[+COMBINE  ] You see object lists in combined format.\n\r"
                             :
                             "[-combine  ] You see object lists in single format.\n\r",
                             ch);

                send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_NOINTRO)
                             ?
                             "[+NOINTRO  ] You don't see the ascii intro screen on login.\n\r"
                             :
                             "[-nointro  ] You see the ascii intro screen on login.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_PROMPT)
                             ? "[+PROMPT   ] You have a prompt.\n\r"
                             : "[-prompt   ] You don't have a prompt.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_TELNET_GA)
                             ?
                             "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
                             :
                             "[-telnetga ] You don't receive a telnet GA sequence.\n\r",
                             ch);

                send_to_char(IS_SET(ch->act, PLR_ANSI)
                             ?
                             "[+ANSI     ] You receive ANSI color sequences.\n\r"
                             :
                             "[-ansi     ] You don't receive receive ANSI colors.\n\r",
                             ch);
#ifdef ACCOUNT
                if (ch->pcdata->account)
                        send_to_char(IS_SET(ch->pcdata->flags, ACCOUNT_SOUND)
                                     ?
                                     "[+SOUND    ] You have MSP support.\n\r"
                                     :
                                     "[-sound    ] You don't have MSP support.\n\r",
                                     ch);
                else
#endif
                        send_to_char(IS_SET(ch->act, PLR_SOUND)
                                     ?
                                     "[+SOUND    ] You have MSP support.\n\r"
                                     :
                                     "[-sound    ] You don't have MSP support.\n\r",
                                     ch);
                send_to_char(IS_SET(ch->act, PLR_SHOVEDRAG) ?
                             "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\n\r"
                             :
                             "[-shovedrag] You'd rather not be shoved or dragged around.\n\r",
                             ch);

                send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_NOSUMMON)
                             ?
                             "[+NOSUMMON ] You do not allow other players to summon you.\n\r"
                             :
                             "[-nosummon ] You allow other players to summon you.\n\r",
                             ch);
                
				send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_AUTODRAW)
                             ?
                             "[+AUTODRAW ] You autodraw your holsters and put them back away after a fight.\n\r"
                             :
                             "[-autodraw ] No autodrawing, buisness as usual.\n\r",
                             ch);

                if (IS_IMMORTAL(ch)) {
                        send_to_char(IS_SET(ch->act, PLR_ROOMVNUM)
                                     ?
                                     "[+VNUM     ] You can see the VNUM of a room.\n\r"
                                     :
                                     "[-vnum     ] You do not see the VNUM of a room.\n\r",
                                     ch);

                        send_to_char(IS_SET(ch->pcdata->flags, PCFLAG_ROOM)
                                     ?
                                     "[+ROOMFLAGS] You will see room flags.\n\r"
                                     :
                                     "[-roomflags] You will not see room flags.\n\r",
                                     ch);
				}

                /*
                 * Added 12/16/2003 by Gavin of DW 
                 */
                send_to_char(IS_SET(ch->act, PLR_MXP)
                             ? "[+MXP      ] You have MXP support.\n\r"
                             : "[-mxp      ] You don't have MXP support.\n\r",
                             ch);
                send_to_char(IS_SET(ch->act, PLR_SILENCE) ?
                             "[+SILENCE  ] You are silenced.\n\r" : "", ch);

                send_to_char(!IS_SET(ch->act, PLR_NO_EMOTE)
                             ? "" : "[-emote    ] You can't emote.\n\r", ch);

                send_to_char(!IS_SET(ch->act, PLR_NO_TELL)
                             ? ""
                             : "[-tell     ] You can't use 'tell'.\n\r", ch);

                send_to_char(!IS_SET(ch->act, PLR_LITTERBUG)
                             ? ""
                             :
                             "[-litter  ] A convicted litterbug. You cannot drop anything.\n\r",
                             ch);
        }
        else
        {
                bool      fSet;
                int       bit = 0;

                if (arg[0] == '+')
                        fSet = TRUE;
                else if (arg[0] == '-')
                        fSet = FALSE;
                else
                {
                        send_to_char("Config -option or +option?\n\r", ch);
                        return;
                }

                if (!str_prefix(arg + 1, "autoexit"))
                        bit = PLR_AUTOEXIT;
                else if (!str_prefix(arg + 1, "autoloot"))
                        bit = PLR_AUTOLOOT;
                else if (!str_prefix(arg + 1, "autocred"))
                        bit = PLR_AUTOGOLD;
                else if (!str_prefix(arg + 1, "blank"))
                        bit = PLR_BLANK;
                else if (!str_prefix(arg + 1, "brief"))
                        bit = PLR_BRIEF;
                else if (!str_prefix(arg + 1, "combine"))
                        bit = PLR_COMBINE;
                else if (!str_prefix(arg + 1, "prompt"))
                        bit = PLR_PROMPT;
                else if (!str_prefix(arg + 1, "telnetga"))
                        bit = PLR_TELNET_GA;
                else if (!str_prefix(arg + 1, "ansi"))
                        bit = PLR_ANSI;
                else if (!str_prefix(arg + 1, "sound"))
                {
                        send_to_char("Please use sound command instead.", ch);
                        /*
                         * bit = PLR_SOUND; 
                         */
                        return;
                }
                else if (!str_prefix(arg + 1, "flee"))
                        bit = PLR_FLEE;
                else if (!str_prefix(arg + 1, "nice"))
                        bit = PLR_NICE;
                else if (!str_prefix(arg + 1, "shovedrag"))
                        bit = PLR_SHOVEDRAG;
                else if (IS_IMMORTAL(ch) && !str_prefix(arg + 1, "vnum"))
                        bit = PLR_ROOMVNUM;
                else if (!str_prefix(arg + 1, "mxp"))
                {
                        bit = PLR_MXP;  /* mxp */
                        if (fSet)
                                do_mxp(ch, "on");
                        else
                                do_mxp(ch, "off");
                }

                if (bit)
                {

                        if (fSet)
                                SET_BIT(ch->act, bit);
                        else
                                REMOVE_BIT(ch->act, bit);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
                else
                {
                        if (!str_prefix(arg + 1, "norecall"))
                                bit = PCFLAG_NORECALL;
                        else if (!str_prefix(arg + 1, "nointro"))
                                bit = PCFLAG_NOINTRO;
                        else if (!str_prefix(arg + 1, "nosummon"))
                                bit = PCFLAG_NOSUMMON;
                        else if (!str_prefix(arg + 1, "autodraw"))
                                bit = PCFLAG_AUTODRAW;
                        else if (!str_prefix(arg + 1, "gag"))
                                bit = PCFLAG_GAG;
                        else if (!str_prefix(arg + 1, "pager"))
                                bit = PCFLAG_PAGERON;
                        else if (!str_prefix(arg + 1, "roomflags")
                                 && (IS_IMMORTAL(ch)))
                                bit = PCFLAG_ROOM;
                        else
                        {
                                send_to_char("Config which option?\n\r", ch);
                                return;
                        }

                        if (fSet)
                                SET_BIT(ch->pcdata->flags, bit);
                        else
                                REMOVE_BIT(ch->pcdata->flags, bit);

                        send_to_char("Ok.\n\r", ch);
                        return;
                }
        }

        return;
}


CMDF do_credits(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        do_help(ch, "credits");
}


extern int top_area;

CMDF do_areas(CHAR_DATA * ch, char *argument)
{
        AREA_DATA *pArea;

        argument = NULL;

        set_pager_color(AT_PLAIN, ch);
        send_to_pager
                ("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r",
                 ch);
        send_to_pager
                ("-------------+--------------------------------------+-------------+-----------\n\r",
                 ch);

        for (pArea = first_area; pArea; pArea = pArea->next)
                pager_printf(ch,
                             "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r",
                             pArea->author, pArea->name,
                             pArea->low_soft_range, pArea->hi_soft_range,
                             pArea->low_hard_range, pArea->hi_hard_range);
        return;
}

CMDF do_afk(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        if (IS_NPC(ch))
                return;

        if IS_SET
                (ch->act, PLR_AFK)
        {
                REMOVE_BIT(ch->act, PLR_AFK);
                send_to_char("You are no longer afk.\n\r", ch);
                act(AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
        }
        else
        {
                SET_BIT(ch->act, PLR_AFK);
                send_to_char("You are now afk.\n\r", ch);
                act(AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_ROOM);
                return;
        }

}

CMDF do_slist(CHAR_DATA * ch, char *argument)
{
        int       sn, i, lFound;
        char      skn[MAX_INPUT_LENGTH];
        int       col = 0;

/* C++ conversion - Greven 31/12/03 
   int ability, class, iClass;*/
        int       ability, classtype, iClass;

        if (IS_NPC(ch))
                return;


        classtype = -1;
        for (iClass = 0; iClass < MAX_ABILITY; iClass++)
        {
                if (!str_prefix(argument, ability_name[iClass]))
                        classtype = iClass;
        }
        set_pager_color(AT_MAGIC, ch);
        send_to_pager("&BS&zPELL &w& &BS&zKILL &BL&zIST\n\r", ch);
        send_to_pager("------------------\n\r", ch);

        for (ability = -1; ability < MAX_ABILITY; ability++)
        {

                if (argument[0] && (ability != classtype))
                        continue;

                if (ch->perm_frc < 1 && ability == 7
                    && ch->skill_level[7] < 500)
                        ability++;

                if (ability >= 0)
                        snprintf(skn, MSL, "\n\r&B[&z%s&B]\n\r",
                                 capitalize(ability_name[ability]));
                else
                        snprintf(skn, MSL, "%s",
                                 "\n\r&B[&zGeneral Skills&B]\n\r");

                send_to_pager(skn, ch);
                for (i = 0; i <= 150; i++)
                {
                        lFound = 0;
                        for (sn = 0; sn < top_sn; sn++)
                        {
                                if (!skill_table[sn]->name)
                                        break;

                                if (skill_table[sn]->guild != ability)
                                        continue;

                                if (ch->pcdata->learned[sn] == 0
                                    && SPELL_FLAG(skill_table[sn],
                                                  SF_SECRETSKILL))
                                        continue;

                                if (i == skill_table[sn]->min_level)
                                {
                                        if (skill_table[sn]->races
                                            && skill_table[sn]->races[0] !=
                                            '\0')
                                        {
                                                pager_printf(ch,
                                                             "&B(&w%3d&B)[&zR&B]&B[&z%s&B]&w%-18.18s ",
                                                             i, get_help(ch,
                                                                         skill_table
                                                                         [sn]->
                                                                         name)
                                                             ? "H" : " ",
                                                             skill_table[sn]->
                                                             name);
                                        }
                                        else
                                        {
                                                pager_printf(ch,
                                                             "&B(&w%3d&B)[&z &B]&B[&z%s&B]&w%-18.18s ",
                                                             i, get_help(ch,
                                                                         skill_table
                                                                         [sn]->
                                                                         name)
                                                             ? "H" : " ",
                                                             skill_table[sn]->
                                                             name);
                                        }
                                        if (++col == 3)
                                        {
                                                pager_printf(ch, "\n\r");
                                                col = 0;
                                        }
                                }
                        }
                }
                if (col != 0)
                {
                        pager_printf(ch, "\n\r");
                        col = 0;
                }
        }
        return;
}

CMDF do_whois(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];

        buf[0] = '\0';

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char
                        ("You must input the name of a player online.\n\r",
                         ch);
                return;
        }

        mudstrlcat(buf, "0.", MSL);
        mudstrlcat(buf, argument, MSL);
        if (((victim = get_char_world(ch, buf)) == NULL))
        {
                send_to_char("No such player online.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("That's not a player!\n\r", ch);
                return;
        }

/*        if (!victim->desc)
        {
                send_to_char("That person does not have a descriptor!\n\r",
                             ch);
                return;
        } - Why is this here? */

        send_to_char
                ("&B---------------------------------------------------------&B\n\r",
                 ch);
        ch_printf(ch, "&B| &BN&zame:&w %-20s &B| &BL&zevel: &w%-17d &B|\n\r",
                  victim->name, victim->top_level);
        ch_printf(ch, "&B| &BA&zge:&w %-21d &B| %s &w%-16s &B|\n\r",
                  get_age(victim),
                  victim->pcdata->spouse[0] !=
                  '\0' ? (IS_SET(victim->pcdata->flags, PCFLAG_MARRIED) ?
                          "&BS&zpouse:" : "&BF&ziance:") : "       ",
                  victim->pcdata->spouse[0] !=
                  '\0' ? victim->pcdata->spouse : "");
        ch_printf(ch, "&B| &BS&zex:&w %-21s &B| &BR&zace: &w%-18s &B|\n\r",
                  victim->sex == SEX_MALE ? "male" : victim->sex ==
                  SEX_FEMALE ? "female" : "neutral", victim->race->name());
        if (IS_IMMORTAL(ch))
                ch_printf(ch,
                          "&B| &BI&zn Room:&w %-17d &B|                          &B|\n\r",
                          victim->in_room->vnum);
        if (victim->pcdata->clan)
        {
                ch_printf(ch, "&B| &BC&zlan:&w %-47s &B|\n\r",
                          victim->pcdata->clan->name);
        }
        send_to_char
                ("&B---------------------------------------------------------&B\n\r",
                 ch);

        if (victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
        {
                ch_printf(ch, "&z%s's personal bio:\n\r", victim->name);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);
                ch_printf(ch, "&w%s\n\r", victim->pcdata->bio);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);
        }
        if (IS_IMMORTAL(ch))
        {
                send_to_char("&zInfo for immortals:\n\r", ch);
                send_to_char
                        ("&B---------------------------------------------------------&B\n\r",
                         ch);

                if (victim->pcdata->authed_by
                    && victim->pcdata->authed_by[0] != '\0')
                        ch_printf(ch, "&z%s was authorized by %s.\n\r",
                                  victim->name, victim->pcdata->authed_by);

                ch_printf(ch,
                          "&z%s has killed &w%d &zmobiles, and been killed by a mobile &w%d &ztimes.\n\r",
                          victim->name, victim->pcdata->mkills,
                          victim->pcdata->mdeaths);
                if (victim->pcdata->pkills || victim->pcdata->pdeaths)
                        ch_printf(ch,
                                  "&z%s has killed &w%d &zplayers, and been killed by a player &w%d &ztimes.\n\r",
                                  victim->name, victim->pcdata->pkills,
                                  victim->pcdata->pdeaths);
                if (victim->pcdata->illegal_pk)
                        ch_printf(ch,
                                  "&z%s has committed &w%d &zillegal player kills.\n\r",
                                  victim->name, victim->pcdata->illegal_pk);

                ch_printf(ch, "&z%s is &w%s&zhelled at the moment.\n\r",
                          victim->name,
                          (victim->pcdata->release_date == 0) ? "not " : "");

                if (victim->pcdata->release_date != 0)
                        ch_printf(ch,
                                  "&z%s was helled by &w%s&z, and will be released on &w%24.24s.\n\r",
                                  victim->sex ==
                                  SEX_MALE ? "He" : victim->sex ==
                                  SEX_FEMALE ? "She" : "It",
                                  victim->pcdata->helled_by,
                                  ctime(&victim->pcdata->release_date));

                if (get_trust(victim) < get_trust(ch))
                {
                        snprintf(buf, MSL, "list %s", victim->name);
                        do_comment(ch, buf);
                }

                if (IS_SET(victim->act, PLR_SILENCE)
                    || IS_SET(victim->act, PLR_NO_EMOTE)
                    || IS_SET(victim->act, PLR_NO_TELL))
                {
                        snprintf(buf, MSL, "%s",
                                 "&zThis player has the following flags set:");
                        if (IS_SET(victim->act, PLR_SILENCE))
                                mudstrlcat(buf, "&w silence", MSL);
                        if (IS_SET(victim->act, PLR_NO_EMOTE))
                                mudstrlcat(buf, "&w noemote", MSL);
                        if (IS_SET(victim->act, PLR_NO_TELL))
                                mudstrlcat(buf, "&w notell", MSL);
                        mudstrlcat(buf, ".\n\r", MSL);
                        send_to_char(buf, ch);
                }
                if (victim->desc && victim->desc->host[0] != '\0')  /* added by Gorog */
                {
                        snprintf(buf, MSL, "&z%s's IP info:&w %s ",
                                 victim->name, victim->desc->host);
                        mudstrlcat(buf, "\n\r", MSL);
                        send_to_char(buf, ch);
                        if (victim->desc && victim->desc->client[0] != '\0')
                        {
                                ch_printf(ch, "&z%s's client is &w%s\n\r",
                                          victim->name, victim->desc->client);
                        }
                }
                if (get_trust(ch) >= LEVEL_GOD
                    && get_trust(ch) >= get_trust(victim) && victim->pcdata)
                {
                        ch_printf(ch, "&zEmail: &w%s\n\r",
                                  victim->pcdata->account->email);
                }
                if (victim->desc)
                {
                        ch_printf(ch, "&zMSP: &w%s  &zMXP:  &w%s"
#ifdef MCCP
                                  "  &zMCCP:   &w%s"
#endif
                                  "\n\r",
                                  victim->desc->msp_detected ? "On " : "Off",
                                  victim->desc->mxp_detected ? "On " : "Off"
#ifdef MCCP
                                  , victim->desc->compressing ? "On " : "Off"
#endif
                                );
                }
        }
}


CMDF do_pager(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        if (IS_NPC(ch))
                return;
        argument = one_argument(argument, arg);
        if (!*arg)
        {
                if (IS_SET(ch->pcdata->flags, PCFLAG_PAGERON))
                        do_config(ch, "-pager");
                else
                        do_config(ch, "+pager");
                return;
        }
        if (!is_number(arg))
        {
                send_to_char("Set page pausing to how many lines?\n\r", ch);
                return;
        }
        ch->pcdata->pagerlen = atoi(arg);
        if (ch->pcdata->pagerlen < 5)
                ch->pcdata->pagerlen = 5;
        ch_printf(ch, "Page pausing set to %d lines.\n\r",
                  ch->pcdata->pagerlen);
        return;
}

CMDF do_steacher(CHAR_DATA * ch, char *argument)
{

        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];
        char     *buf1;
        char      arg[MAX_STRING_LENGTH];
        int       sn, vnum;
        bool      fMob = FALSE, fSet = FALSE, fMI = FALSE;
        SKILLTYPE *skill = NULL;


        if (IS_NPC(ch))
                return;

        set_pager_color(AT_MAGIC, ch);
        send_to_pager("\n\r------------[ Missing Teachers ]-------------\n\r",
                      ch);
        if (str_cmp(argument, "existant"))
        {
                for (sn = 0;
                     sn < top_sn && skill_table[sn] && skill_table[sn]->name;
                     sn++)
                {
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        fMob = FALSE;
                        fSet = TRUE;
                        skill = skill_table[sn];
                        if (skill->teachers && skill->teachers[0] != '\0')
                        {
                                buf1 = skill->teachers;
                                for (;;)
                                {
                                        buf1 = one_argument(buf1, arg);
                                        if (arg[0] != '\0')
                                        {
                                                vnum = atoi(arg);
                                                if (vnum > 0)
                                                {
                                                        if ((victim =
                                                             get_char_world
                                                             (ch,
                                                              arg)) != NULL)
                                                                fMob = TRUE;
                                                }
                                        }
                                        else
                                                break;
                                }
                        }
                        else if (!skill->teachers
                                 || skill->teachers[0] == '\0')
                                fSet = FALSE;
                        else
                                continue;

                        if (fMob)
                                continue;
                        if (fSet && fMob)
                                continue;

                        if (!fMob)
                                snprintf(buf, MSL,
                                         "Skill: %-25s    [No Invoked Teachers]\n\r",
                                         skill_table[sn]->name);
                        if (!fSet)
                                snprintf(buf, MSL,
                                         "Skill: %-25s    [No Designated Teachers]\n\r",
                                         skill_table[sn]->name);
                        send_to_char(buf, ch);
                        continue;
                }
        }
        else
        {
                for (sn = 0;
                     sn < top_sn && skill_table[sn] && skill_table[sn]->name;
                     sn++)
                {
                        fMI = FALSE;
                        if (!skill_table[sn]->name)
                                break;

                        if (skill_table[sn]->guild < 0
                            || skill_table[sn]->guild >= MAX_ABILITY)
                                continue;

                        skill = skill_table[sn];
                        if (skill->teachers && skill->teachers[0] != '\0')
                        {
                                buf1 = skill->teachers;
                                for (;;)
                                {
                                        buf1 = one_argument(buf1, arg);
                                        if (arg[0] != '\0')
                                        {
                                                vnum = atoi(arg);
                                                if (vnum > 0)
                                                {
                                                        if (!get_mob_index
                                                            (vnum))
                                                                ch_printf(ch,
                                                                          "&BSkill: %-25s    [Invalid teacher: %7d]\n\r",
                                                                          skill_table
                                                                          [sn]->
                                                                          name,
                                                                          vnum);
                                                }
                                        }
                                        else
                                                break;
                                }
                        }
                }
        }

        return;
}

char     *revision(void)
{
        static char buf[MIL];
        char      buf1[MIL];
        int       i, j;

        mudstrlcpy(buf1, REVISION, MIL);
        for (i = 11, j = 0; i < ((int) strlen(buf1) - 2); i++, j++)
                buf[j] = buf1[i];
        buf[i++] = '\0';


        return buf;

}

CMDF do_mudinfo(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        ch_printf(ch, "&BM&zud name:             &w%-20s&D\n\r",
                  sysdata.mud_name);
        ch_printf(ch, "&BM&zud email:            &w%-20s&D\n\r",
                  sysdata.mud_email);
        ch_printf(ch, "&BM&zud url:              &w%-20s&D\n\r",
                  sysdata.mud_url);
        ch_printf(ch, "&BM&zud name:             &w%-20s&D\n\r",
                  sysdata.mud_name);
        ch_printf(ch, "&BW&zeb server enabled:   &w%-20s&D\n\r",
                  sysdata.web ? "Enabled" : "Disabled");
        ch_printf(ch, "&BA&zmount of help files: &w%-5d&D\n\r", top_help);
        ch_printf(ch, "&BM&zax players ever:     &w%-5d&D\n\r",
                  sysdata.alltimemax);
        ch_printf(ch, "&BL&zast compile time:    &w%-20s&D\n\r", __TIME__);
        ch_printf(ch, "&BL&zast compile date:    &w%-20s&D\n\r", __DATE__);
#if defined(__cplusplus)
        ch_printf(ch, "&BC&zompile type:         &w%-20s&D\n\r", "C++");
#else
        ch_printf(ch, "&BC&zompile type:         &w%-20s&D\n\r", "C");
#endif

#if defined(MCCP)
        ch_printf(ch, "&BM&zCCP:                 &w%-20s&D\n\r", "Enabled");
#else
        ch_printf(ch, "&BM&zCCP:                 &w%-20s&D\n\r", "Disabled");
#endif

#if defined(IMC)
        ch_printf(ch, "&BI&zMC2:                 &w%-20s&D\n\r", "Enabled");
#else
        ch_printf(ch, "&BI&zMC2:                 &w%-20s&D\n\r", "Disabled");;
#endif

#if defined(WIN)
        ch_printf(ch, "&BW&zindows compile:      &w%-20s&D\n\r", "Yes");
#else
        ch_printf(ch, "&BW&zindows compile:      &w%-20s&D\n\r", "No");
#endif
        ch_printf(ch, "&BL&zast version:         &w%-20s&D\n\r", revision());
}
