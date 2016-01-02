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
 *                $Id: special.c 1333 2005-12-05 04:08:46Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include "mud.h"
#include "bounty.h"
#include "races.h"
#include "olc_bounty.h"
#include "installations.h"

/* jails for wanted flags */

#define ROOM_JAIL_CORUSCANT        0

SPEC_LIST *first_specfun;
SPEC_LIST *last_specfun;

bool is_wielding_weapon args((CHAR_DATA * ch));

/* Simple load function - no OLC support for now.
 * This is probably something you DONT want builders playing with.
 */
void load_specfuns(void)
{
        SPEC_LIST *specfun;
        FILE     *fp;
        char      filename[256];

        first_specfun = NULL;
        last_specfun = NULL;

        snprintf(filename, 256, "%sspecfuns.dat", SYSTEM_DIR);
        if (!(fp = fopen(filename, "r")))
        {
                bug("%s",
                    "load_specfuns: FATAL - cannot load specfuns.dat, exiting.");
                perror(filename);
                exit(1);
        }
        else
        {
                for (;;)
                {
                        if (feof(fp))
                        {
                                bug("%s",
                                    "load_specfuns: Premature end of file!");
                                FCLOSE(fp);
                                return;
                        }
                        CREATE(specfun, SPEC_LIST, 1);
                        specfun->name = fread_string_nohash(fp);
                        if (!str_cmp(specfun->name, "$"))
                                break;
                        LINK(specfun, first_specfun, last_specfun, next,
                             prev);
                }
                DISPOSE(specfun->name);
                DISPOSE(specfun);
                FCLOSE(fp);
        }
        return;
}

/* Simple validation function to be sure a function can be used on mobs */
bool validate_spec_fun(char *name)
{
        SPEC_LIST *specfun;

        for (specfun = first_specfun; specfun; specfun = specfun->next)
        {
                if (!str_cmp(specfun->name, name))
                        return TRUE;
        }
        return FALSE;
}

bool remove_obj args((CHAR_DATA * ch, int iWear, bool fReplace));

/*
 * Given a name, return the appropriate spec_fun.
 */
SPEC_FUN *spec_lookup(char *name)
{
        void     *funHandle;
        const char *error;

        funHandle = dlsym(sysdata.dlHandle, name);
        if ((error = dlerror()) != NULL)
        {
                bug("spec_lookup: Error locating function %s in symbol table.", name);
                return NULL;
        }
        return (SPEC_FUN *) funHandle;
}



SPECF spec_newbie_pilot(CHAR_DATA * ch)
{
        int       home = 32149;
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        OBJ_DATA *obj;
        char      buf[MAX_STRING_LENGTH];
        bool      diploma = FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                        if (obj->pIndexData->vnum == OBJ_VNUM_SCHOOL_DIPLOMA)
                                diploma = TRUE;

                if (!diploma)
                        continue;

                if (victim->race && victim->race->home() != -1)
                {
                        ROOM_INDEX_DATA *room =
                                get_room_index(victim->race->home());
                        home = victim->race->home();
                        if (room && room->area && room->area->planet)
                        {
                                snprintf(buf, MSL,
                                         "After a brief journey you arrive at %s's spaceport.",
                                         room->area->planet->name);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "After a brief journey you arrive at %s's spaceport.",
                                         room->name);
                        }
                        echo_to_room(AT_ACTION, ch->in_room, buf);
                }
                else
                {
                        snprintf(buf, MSL, "Hmm, a %s.",
                                 victim->race->name());
                        do_look(ch, victim->name);
                        do_say(ch, buf);
                        do_say(ch,
                               "You're home planet is a little hard to get to right now.");
                        do_say(ch, "I'll take you to the Pluogus instead.");
                        echo_to_room(AT_ACTION, ch->in_room,
                                     "After a brief journey the shuttle docks with the Serin Pluogus.\n\r\n\r");
                }
                char_from_room(victim);
                char_to_room(victim, get_room_index(home));

                do_look(victim, "");

                snprintf(buf, MSL,
                         "%s steps out and the shuttle quickly returns to the academy.\n\r",
                         victim->name);
                echo_to_room(AT_ACTION, ch->in_room, buf);
        }

        return FALSE;
}

SPECF spec_player_elite_guard(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if (!is_clan_enemy(ch, victim))
                        continue;
                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TYPE_UNDEFINED);
                return TRUE;
        }
        return FALSE;
}

SPECF spec_installation_guard(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                if (!is_clan_enemy(ch, victim))
                        continue;

                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TYPE_UNDEFINED);
                return TRUE;
        }
        return FALSE;
}

SPECF spec_installation_entrance_guard(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        INSTALLATION_DATA *installation;
        EXIT_DATA *pexit;


        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

				if (!can_see(ch, victim))
                        continue;

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) == NULL)
                        return FALSE;

                if (is_clan_ally(ch, victim) || is_same_clan(ch, victim))
                        continue;

                for (pexit =
                     get_room_index(installation->first_room)->first_exit;
                     pexit; pexit = pexit->next)
                        if (pexit->to_room->vnum > installation->last_room
                            || pexit->to_room->vnum <
                            installation->first_room)
                                break;

                do_say(ch, "Hey you're not allowed in here! Leave, NOW!");
                char_from_room(victim);
                char_to_room(victim, pexit->to_room);
                do_look(victim, "auto");
                return TRUE;

        }

        return FALSE;
}

SPECF spec_installation_doctor(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        INSTALLATION_DATA *installation;


        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                if (!is_clan_ally(ch, victim))
                        continue;

                if ((installation =
                     installation_from_room(ch->in_room->vnum)) == NULL)
                        return FALSE;

                if (victim->hit < victim->max_hit)
                {
                        do_say(ch,
                               "Hello there, you clearly need healing, lemme see what I can do.");
                        ch_printf(victim,
                                  "The doctor heals you, replenishing %d hit points.\n\r ",
                                  victim->max_hit / 10);
                        victim->hit += victim->max_hit / 10;
                        if (victim->hit > victim->max_hit)
                                victim->hit = victim->max_hit;
                }
                return TRUE;
        }

        return FALSE;
}

SPECF spec_jedi(CHAR_DATA * ch)
{
        ch = NULL;
        return FALSE;
}



SPECF spec_clan_guard(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;
        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;

                if (!is_clan_enemy(ch, victim))
                        continue;
                if (is_same_clan(ch, victim))
                        continue;
                do_say(ch, "Hey you're not allowed in here!");
                multi_hit(ch, victim, TYPE_UNDEFINED);
                return TRUE;
        }

        return FALSE;
}

SPECF spec_player_guard(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if (!IS_NPC(victim) && victim->pcdata && IS_AWAKE(victim)
                    && str_cmp(ch->leader->name, victim->name))
                {
                        do_say(ch, "Hey you're not allowed in here!");
                        multi_hit(ch, victim, TYPE_UNDEFINED);
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_clan_patrol(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        if (!get_clan(ch->mob_clan))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if (!IS_NPC(victim) && victim->pcdata && victim->pcdata->clan
                    && IS_AWAKE(victim) && (victim->top_level > 29))
                {
                        if (victim->pcdata->clan->mainclan
                            && get_clan(ch->mob_clan)->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->mainclan->name,
                                     victim->pcdata->clan->mainclan->name))
                                        return FALSE;
                        }
                        else if (victim->pcdata->clan->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->name,
                                     victim->pcdata->clan->mainclan->name))
                                        return FALSE;
                        }
                        else if (get_clan(ch->mob_clan)->mainclan)
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->mainclan->name,
                                     victim->pcdata->clan->name))
                                        return FALSE;
                        }
                        else
                        {
                                if (!str_cmp
                                    (get_clan(ch->mob_clan)->name,
                                     victim->pcdata->clan->name))
                                        return FALSE;
                        }


                        do_say(ch, "Hey you're not allowed around here!");
                        multi_hit(ch, victim, TYPE_UNDEFINED);
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_player_patrol(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if (!IS_NPC(victim) && victim->pcdata && IS_AWAKE(victim)
                    && (victim->top_level > 29)
                    && str_cmp(ch->leader->name, victim->name))
                {
                        do_say(ch, "Hey you're not allowed around here!");
                        multi_hit(ch, victim, TYPE_UNDEFINED);
                        return TRUE;
                }
        }

        return FALSE;
}



SPECF spec_customs_smut(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        OBJ_DATA *obj;
        char      buf[MAX_STRING_LENGTH];
        long      ch_exp;

        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ITEM_SMUT)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "%s is illegal contraband. I'm going to have to confiscate that.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WEAR_NONE)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        act(AT_ACTION,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n takes $p from you.", ch, obj,
                                            victim, TO_VICT);
                                        obj = obj_to_char(obj, ch);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You lose %ld experience.\n\r ",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SMUGGLING_ABILITY);
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IS_SET(obj->extra_flags,
                                                    ITEM_CONTRABAND))
                                {
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        act(AT_ACTION,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, TO_VICT);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);

                                        return TRUE;
                                }
                                else if (!IS_SET
                                         (obj->extra_flags, ITEM_CONTRABAND))
                                {
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ITEM_CONTAINER)
                        {
                                OBJ_DATA *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ITEM_SMUT
                                            && !IS_SET(content->extra_flags,
                                                       ITEM_CONTRABAND))
                                        {
                                                ch_exp = UMIN(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);
                                                SET_BIT(content->extra_flags,
                                                        ITEM_CONTRABAND);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_weapons(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
		char *mob_clan;
        OBJ_DATA *obj,*obj_prev;
        char      buf[MAX_STRING_LENGTH];
        long      ch_exp;

        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        mob_clan = ch->mob_clan;
        if (ch->in_room && ch->in_room->area->planet && ch->in_room->area->planet->governed_by)
                ch->mob_clan = ch->in_room->area->planet->governed_by->name;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                for (obj = victim->last_carrying; obj; obj = obj_prev)
                {
                        obj_prev = obj->prev_content;

                        if (obj->item_type == ITEM_HOLSTER && obj->first_content != NULL)
                             obj = obj->first_content;

                        if (obj->pIndexData->item_type == ITEM_WEAPON
                            || obj->pIndexData->item_type == ITEM_GRENADE)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "Weapons are banned from non-military usage. I'm going to have to confiscate %s.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WEAR_NONE)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        if (obj->in_obj) 
                                                obj_from_obj(obj);
                                        else 
                                                obj_from_char(obj);
                                        act(AT_ACTION,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n takes $p from you.", ch, obj,
                                            victim, TO_VICT);
                                        obj = obj_to_char(obj, ch);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You lose %ld experience.\n\r ",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SMUGGLING_ABILITY);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IS_SET(obj->extra_flags,
                                                    ITEM_CONTRABAND))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %d.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        act(AT_ACTION,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, TO_VICT);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (!IS_SET
                                         (obj->extra_flags, ITEM_CONTRABAND))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s.\n\r ",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
										ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ITEM_CONTAINER)
                        {
                                OBJ_DATA *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if ((content->pIndexData->item_type ==
                                             ITEM_WEAPON
                                             || content->pIndexData->
                                             item_type == ITEM_GRENADE)
                                            && !IS_SET(content->extra_flags,
                                                       ITEM_CONTRABAND))
                                        {
                                                separate_obj(content);
                                                ch_exp = UMIN(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);
                                                SET_BIT(content->extra_flags,
                                                        ITEM_CONTRABAND);
												ch->mob_clan = mob_clan;
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_alcohol(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        OBJ_DATA *obj;
        char      buf[MAX_STRING_LENGTH];
        int       liquid;
        long      ch_exp;

        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ITEM_DRINK_CON)
                        {
                                if ((liquid = obj->value[2]) >= LIQ_MAX)
                                        liquid = obj->value[2] = 0;

                                if (liq_table[liquid].liq_affect[COND_DRUNK] >
                                    0)
                                {
                                        if (victim != ch
                                            && can_see(ch, victim)
                                            && can_see_obj(ch, obj))
                                        {
                                                snprintf(buf, MSL,
                                                         "%s is illegal contraband. I'm going to have to confiscate that.",
                                                         obj->short_descr);
                                                do_say(ch, buf);
                                                if (obj->wear_loc !=
                                                    WEAR_NONE)
                                                        remove_obj(victim,
                                                                   obj->
                                                                   wear_loc,
                                                                   TRUE);
                                                separate_obj(obj);
                                                obj_from_char(obj);
                                                act(AT_ACTION,
                                                    "$n confiscates $p from $N.",
                                                    ch, obj, victim,
                                                    TO_NOTVICT);
                                                act(AT_ACTION,
                                                    "$n takes $p from you.",
                                                    ch, obj, victim, TO_VICT);
                                                obj = obj_to_char(obj, ch);
                                                SET_BIT(obj->extra_flags,
                                                        ITEM_CONTRABAND);
                                                ch_exp = UMIN(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You lose %ld experience. \n\r",
                                                          ch_exp);
                                                gain_exp(victim, 0 - ch_exp,
                                                         SMUGGLING_ABILITY);
                                                return TRUE;
                                        }
                                        else if (can_see(ch, victim)
                                                 && !IS_SET(obj->extra_flags,
                                                            ITEM_CONTRABAND))
                                        {
                                                ch_exp = UMIN(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d. \n\r",
                                                          ch_exp,
                                                          obj->short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);

                                                act(AT_ACTION,
                                                    "$n looks at $N suspiciously.",
                                                    ch, NULL, victim,
                                                    TO_NOTVICT);
                                                act(AT_ACTION,
                                                    "$n look at you suspiciously.",
                                                    ch, NULL, victim,
                                                    TO_VICT);
                                                SET_BIT(obj->extra_flags,
                                                        ITEM_CONTRABAND);
                                                return TRUE;
                                        }
                                        else if (!IS_SET
                                                 (obj->extra_flags,
                                                  ITEM_CONTRABAND))
                                        {
                                                ch_exp = UMIN(obj->cost * 10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d. \n\r",
                                                          ch_exp,
                                                          obj->short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);

                                                SET_BIT(obj->extra_flags,
                                                        ITEM_CONTRABAND);
                                                return TRUE;
                                        }
                                }
                        }
                        else if (obj->item_type == ITEM_CONTAINER)
                        {
                                OBJ_DATA *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ITEM_DRINK_CON
                                            && !IS_SET(content->extra_flags,
                                                       ITEM_CONTRABAND))
                                        {
                                                if ((liquid =
                                                     obj->value[2]) >=
                                                    LIQ_MAX)
                                                        liquid = obj->
                                                                value[2] = 0;
                                                if (liq_table[liquid].
                                                    liq_affect[COND_DRUNK] <=
                                                    0)
                                                        continue;
                                                ch_exp = UMIN(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %d.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);
                                                SET_BIT(content->extra_flags,
                                                        ITEM_CONTRABAND);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_customs_spice(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        OBJ_DATA *obj;
        char      buf[MAX_STRING_LENGTH];
        long      ch_exp;

        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                for (obj = victim->last_carrying; obj;
                     obj = obj->prev_content)
                {
                        if (obj->pIndexData->item_type == ITEM_SPICE
                            || obj->pIndexData->item_type == ITEM_RAWSPICE)
                        {
                                if (victim != ch && can_see(ch, victim)
                                    && can_see_obj(ch, obj))
                                {
                                        snprintf(buf, MSL,
                                                 "%s is illegal contraband. I'm going to have to confiscate that.",
                                                 obj->short_descr);
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WEAR_NONE)
                                                remove_obj(victim,
                                                           obj->wear_loc,
                                                           TRUE);
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        act(AT_ACTION,
                                            "$n confiscates $p from $N.", ch,
                                            obj, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n takes $p from you.", ch, obj,
                                            victim, TO_VICT);
                                        obj = obj_to_char(obj, ch);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You lose %ld experience. \n\r",
                                                  ch_exp);
                                        gain_exp(victim, 0 - ch_exp,
                                                 SMUGGLING_ABILITY);
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)
                                         && !IS_SET(obj->extra_flags,
                                                    ITEM_CONTRABAND))
                                {
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s. \n\r",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        act(AT_ACTION,
                                            "$n looks at $N suspiciously.",
                                            ch, NULL, victim, TO_NOTVICT);
                                        act(AT_ACTION,
                                            "$n look at you suspiciously.",
                                            ch, NULL, victim, TO_VICT);
                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        return TRUE;
                                }
                                else if (!IS_SET
                                         (obj->extra_flags, ITEM_CONTRABAND))
                                {
                                        ch_exp = UMIN(obj->cost * 10,
                                                      (exp_level
                                                       (victim->
                                                        skill_level
                                                        [SMUGGLING_ABILITY] +
                                                        1) -
                                                       exp_level(victim->
                                                                 skill_level
                                                                 [SMUGGLING_ABILITY])));
                                        ch_printf(victim,
                                                  "You receive %ld experience for smuggling %s. \n\r",
                                                  ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,
                                                 SMUGGLING_ABILITY);

                                        SET_BIT(obj->extra_flags,
                                                ITEM_CONTRABAND);
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ITEM_CONTAINER)
                        {
                                OBJ_DATA *content;

                                for (content = obj->first_content; content;
                                     content = content->next_content)
                                {
                                        if (content->pIndexData->item_type ==
                                            ITEM_SPICE
                                            && !IS_SET(content->extra_flags,
                                                       ITEM_CONTRABAND))
                                        {
                                                ch_exp = UMIN(content->cost *
                                                              10,
                                                              (exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY]
                                                                + 1) -
                                                               exp_level
                                                               (victim->
                                                                skill_level
                                                                [SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,
                                                          content->
                                                          short_descr);
                                                gain_exp(victim, ch_exp,
                                                         SMUGGLING_ABILITY);
                                                SET_BIT(content->extra_flags,
                                                        ITEM_CONTRABAND);
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}

SPECF spec_police(CHAR_DATA * ch)
{
        ROOM_INDEX_DATA *jail = NULL;
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        char      buf[MAX_STRING_LENGTH];

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        if (!ch->in_room || !ch->in_room->area || !ch->in_room->area->planet)
                return FALSE;

        if (ch->in_room->area->planet->governed_by)
        {
                CLAN_DATA *clan = ch->in_room->area->planet->governed_by;

                jail = get_room_index(clan->jail);
        }

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (IS_NPC(victim))
                        continue;
                if (!can_see(ch, victim))
                        continue;
                if (number_bits(1) == 0)
                        continue;

                if (is_wanted(victim, ch->in_room->area->planet))
                {
                        snprintf(buf, MSL, "Hey you're wanted on %s!",
                                 ch->in_room->area->planet->name);
                        do_say(ch, buf);

                        /*
                         * currently no jails 
                         */

                        if (jail
                            && (victim->position == POS_INCAP
                                || victim->position == POS_STUNNED
                                || victim->position == POS_SLEEPING))
                        {
                                act(AT_ACTION, "$n ushers $N off to jail.",
                                    ch, NULL, victim, TO_NOTVICT);
                                act(AT_ACTION, "$n escorts you to jail.", ch,
                                    NULL, victim, TO_VICT);
                                char_from_room(victim);
                                char_to_room(victim, jail);
                                remove_wanted_planet(victim,
                                                     ch->in_room->area->
                                                     planet);
                        }
                        else if (ch->top_level >= victim->top_level
                                 && !xIS_SET(ch->in_room->room_flags,
                                             ROOM_SAFE))
                        {
                                command_printf(ch,
                                               "yell Hey %s is wanted on %s!",
                                               victim->name,
                                               ch->in_room->area->planet->
                                               name);
                                multi_hit(ch, victim, TYPE_UNDEFINED);
                                /*
                                 * remove_wanted_planet(victim,
                                 * ch->in_room->area->
                                 * planet);
                                 */
                        }
                        else
                        {
                                act(AT_ACTION,
                                    "$n fines $N an enormous amount of money.",
                                    ch, NULL, victim, TO_NOTVICT);
                                act(AT_ACTION,
                                    "$n fines you an enourmous amount of money.",
                                    ch, NULL, victim, TO_VICT);
                                victim->gold *= 9;
                                victim->gold /= 10;
                                remove_wanted_planet(victim,
                                                     ch->in_room->area->
                                                     planet);
                        }

                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_jedi_healer(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch))
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim != ch && can_see(ch, victim)
                    && number_bits(1) == 0)
                        break;
        }

        if (!victim)
                return FALSE;

        switch (number_bits(12))
        {
        case 0:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_smaug(skill_lookup("armor"), ch->top_level, ch, victim);
                return TRUE;

        case 1:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_smaug(skill_lookup("good fortune"), ch->top_level, ch,
                            victim);
                return TRUE;

        case 2:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_cure_blindness(skill_lookup("cure blindness"),
                                     ch->top_level, ch, victim);
                return TRUE;

        case 3:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_smaug(skill_lookup("cure light"), ch->top_level, ch,
                            victim);
                return TRUE;

        case 4:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_cure_poison(skill_lookup("cure poison"), ch->top_level,
                                  ch, victim);
                return TRUE;

        case 5:
                act(AT_MAGIC, "$n pauses and concentrates for a moment.", ch,
                    NULL, NULL, TO_ROOM);
                spell_smaug(skill_lookup("refresh"), ch->top_level, ch,
                            victim);
                return TRUE;

        }

        return FALSE;
}



SPECF spec_dark_jedi(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        char     *spell;
        int       sn;


        if (ch->position != POS_FIGHTING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (who_fighting(victim) && number_bits(2) == 0)
                        break;
        }

        if (!victim || victim == ch)
                return FALSE;

        for (;;)
        {
                int       min_level;

                switch (number_bits(4))
                {
                case 0:
                        min_level = 5;
                        spell = "blindness";
                        break;
                case 1:
                        min_level = 5;
                        spell = "fingers of the force";
                        break;
                case 2:
                        min_level = 9;
                        spell = "choke";
                        break;
                case 3:
                        min_level = 8;
                        spell = "invade essence";
                        break;
                case 4:
                        min_level = 11;
                        spell = "force projectile";
                        break;
                case 6:
                        min_level = 13;
                        spell = "drain essence";
                        break;
                case 7:
                        min_level = 4;
                        spell = "force whip";
                        break;
                case 8:
                        min_level = 13;
                        spell = "harm";
                        break;
                case 9:
                        min_level = 9;
                        spell = "force bolt";
                        break;
                case 10:
                        min_level = 1;
                        spell = "force spray";
                        break;
                default:
                        return FALSE;
                }

                if (ch->top_level >= min_level)
                        break;
        }

        if ((sn = skill_lookup(spell)) < 0)
                return FALSE;
        (*skill_table[sn]->spell_fun) (sn, ch->top_level, ch, victim);
        return TRUE;
}



SPECF spec_fido(CHAR_DATA * ch)
{
        OBJ_DATA *corpse;
        OBJ_DATA *c_next;
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;

        if (!IS_AWAKE(ch))
                return FALSE;

        for (corpse = ch->in_room->first_content; corpse; corpse = c_next)
        {
                c_next = corpse->next_content;
                if (corpse->item_type != ITEM_CORPSE_NPC)
                        continue;

                act(AT_ACTION, "$n savagely devours a corpse.", ch, NULL,
                    NULL, TO_ROOM);
                for (obj = corpse->first_content; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        obj_from_obj(obj);
                        obj_to_room(obj, ch->in_room);
                }
                extract_obj(corpse);
                return TRUE;
        }

        return FALSE;
}

SPECF spec_stormtrooper(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if ((IS_NPC(victim) && nifty_is_name("republic", victim->name)
                     && victim->fighting && who_fighting(victim) != ch) ||
                    (!IS_NPC(victim) && victim->pcdata && victim->pcdata->clan
                     && IS_AWAKE(victim)
                     && nifty_is_name("republic",
                                      victim->pcdata->clan->name)))
                {
                        do_say(ch, "Die Rebel Scum!");
                        multi_hit(ch, victim, TYPE_UNDEFINED);
                        return TRUE;
                }

        }

        return FALSE;

}

SPECF spec_new_republic_trooper(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (!can_see(ch, victim))
                        continue;
                if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                        continue;
                if ((IS_NPC(victim) && nifty_is_name("imperial", victim->name)
                     && victim->fighting && who_fighting(victim) != ch) ||
                    (!IS_NPC(victim) && victim->pcdata && victim->pcdata->clan
                     && IS_AWAKE(victim)
                     && nifty_is_name("empire", victim->pcdata->clan->name)))
                {
                        do_say(ch, "Long live the New Republic!");
                        multi_hit(ch, victim, TYPE_UNDEFINED);
                        return TRUE;
                }

        }

        return FALSE;

}


SPECF spec_guardian(CHAR_DATA * ch)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        CHAR_DATA *ech;
        char     *crime;
        int       max_evil;

        if (!IS_AWAKE(ch) || ch->fighting)
                return FALSE;

        max_evil = 300;
        ech = NULL;
        crime = "";

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim->fighting
                    && who_fighting(victim) != ch
                    && victim->alignment < max_evil)
                {
                        max_evil = victim->alignment;
                        ech = victim;
                }
        }

        if (victim && xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
                snprintf(buf, MSL, "%s is a %s!  As well as a COWARD!",
                         victim->name, crime);
                do_say(ch, buf);
                return TRUE;
        }

        if (victim)
        {
                snprintf(buf, MSL, "%s is a %s!  PROTECT THE INNOCENT!!",
                         victim->name, crime);
                do_shout(ch, buf);
                multi_hit(ch, victim, TYPE_UNDEFINED);
                return TRUE;
        }

        if (ech)
        {
                act(AT_YELL, "$n screams 'PROTECT THE INNOCENT!!",
                    ch, NULL, NULL, TO_ROOM);
                multi_hit(ch, ech, TYPE_UNDEFINED);
                return TRUE;
        }

        return FALSE;
}



SPECF spec_janitor(CHAR_DATA * ch)
{
        OBJ_DATA *trash;
        OBJ_DATA *trash_next;

        if (!IS_AWAKE(ch))
                return FALSE;

        for (trash = ch->in_room->first_content; trash; trash = trash_next)
        {
                trash_next = trash->next_content;
                if (!IS_SET(trash->wear_flags, ITEM_TAKE)
                    || IS_OBJ_STAT(trash, ITEM_BURRIED))
                        continue;
                if (trash->item_type == ITEM_DRINK_CON
                    || trash->item_type == ITEM_TRASH
                    || trash->cost < 10
                    || (trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG
                        && !trash->first_content))
                {
                        act(AT_ACTION, "$n picks up some trash.", ch, NULL,
                            NULL, TO_ROOM);
                        obj_from_room(trash);
                        obj_to_char(trash, ch);
                        return TRUE;
                }
        }

        return FALSE;
}



SPECF spec_poison(CHAR_DATA * ch)
{
        CHAR_DATA *victim;

        if (ch->position != POS_FIGHTING
            || (victim = who_fighting(ch)) == NULL
            || number_percent() > 2 * ch->top_level)
                return FALSE;

        act(AT_HIT, "You bite $N!", ch, NULL, victim, TO_CHAR);
        act(AT_ACTION, "$n bites $N!", ch, NULL, victim, TO_NOTVICT);
        act(AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT);
        spell_poison(gsn_poison, ch->top_level, ch, victim);
        return TRUE;
}



SPECF spec_thief(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        int       gold, maxgold;

        if (ch->position != POS_STANDING)
                return FALSE;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || get_trust(victim) >= LEVEL_IMMORTAL || number_bits(2) != 0 || !can_see(ch, victim))   /* Thx Glop */
                        continue;

                if (IS_AWAKE(victim) && number_range(0, ch->top_level) == 0)
                {
                        act(AT_ACTION,
                            "You discover $n's hands in your wallet!", ch,
                            NULL, victim, TO_VICT);
                        act(AT_ACTION,
                            "$N discovers $n's hands in $S wallet!", ch, NULL,
                            victim, TO_NOTVICT);
                        return TRUE;
                }
                else
                {
                        maxgold = ch->top_level * ch->top_level * 1000;
                        gold = victim->gold
                                * number_range(1,
                                               URANGE(2, ch->top_level / 4,
                                                      10)) / 100;
                        ch->gold += 9 * gold / 10;
                        victim->gold -= gold;
                        if (ch->gold > maxgold)
                        {
                                boost_economy(ch->in_room->area,
                                              ch->gold - maxgold / 2);
                                ch->gold = maxgold / 2;
                        }
                        return TRUE;
                }
        }

        return FALSE;
}

SPECF spec_auth(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
        char      buf[MAX_STRING_LENGTH];
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_DATA *obj;
        bool      hasdiploma;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (IS_NPC(victim))
                        continue;

                /*
                 * Prevent people who need name changes from being authed 
                 */
                if (victim->pcdata->auth_state == 2)
                        continue;
                if (!IS_NPC(victim)
                    && (pObjIndex =
                        get_obj_index(OBJ_VNUM_SCHOOL_DIPLOMA)) != NULL)
                {
                        hasdiploma = FALSE;

                        for (obj = victim->last_carrying; obj;
                             obj = obj->prev_content)
                                if (obj->pIndexData ==
                                    get_obj_index(OBJ_VNUM_SCHOOL_DIPLOMA))
                                        hasdiploma = TRUE;

                        if (!hasdiploma)
                        {
                                obj = create_object(pObjIndex, 1);
                                obj = obj_to_char(obj, victim);
                                send_to_char
                                        ("&cThe schoolmaster gives you a diploma, and shakes your hand.\n\r&w",
                                         victim);
                        }
                }

                if (IS_NPC(victim)
                    || !IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED))
                        continue;

                victim->pcdata->auth_state = 3;
                REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
                if (victim->pcdata->authed_by)
                        STRFREE(victim->pcdata->authed_by);
                victim->pcdata->authed_by = QUICKLINK(ch->name);
                snprintf(buf, MSL, "%s authorized %s", ch->name,
                         victim->name);


        }
        return FALSE;

}

SPECF spec_questmaster(CHAR_DATA * ch)
{
        if (!IS_NPC(ch))
                return FALSE;
        else
                return TRUE;
}

/**
 * Checks to see if a mob is in the room, or the corpse is given to it?
 * if the mob is bound, then reward and dropped bounds,
 * reward is based on olc list (mob vnum, corpse vnum, bounty alive, bounty dead)
 * 
 * If the player enters the room and is bound by someone, and person is bountied, pay out bounder and jail bounty.
 *
 * (Halkeye) OOC: found bound mobs, its easy
 * (Halkeye) OOC: but for corpses, should it only be if they are given to the guy right? or should he automaticly assume any corpses in a players inv are for him?
 * (Halkeye) OOC: and since when can you put like a corpse in a backpack?
 * (Cinna) OOC: he should assume that the corpses are for him
 * (Cinna) OOC: because who else are they going too?
 * (Halkeye) OOC: yea .. well its kinda weird for a mob to just take your corpses from you
 * (Halkeye) OOC: and for that matter for you to pick up more than one corpse at all
 *
 * Decided that its a per mob list
 * map<vnum> => list
 *
 * @param npc -> char_data pointer to the npc with the spec flagged turned on
 */
SPECF spec_bountyhunter(CHAR_DATA * npc)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next = NULL;
        OLC_BOUNTY_DATA *bounty;
        CHAR_DATA *imob;
        char      buf[MSL];

        /*
         * Only NPC suppport for now 
         */
        /*
         * Check for mobs that are being held by a player.
         */
        for (victim = npc->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;
                if (victim->holding == NULL)
                        continue;
                if (!IS_NPC(victim->holding))
                        continue;
                if ((bounty = has_olc_bounty(victim->holding)) == NULL)
                        continue;
                if (bounty->type() == BOUNTY_ALIVE)
                {
                        imob = victim->holding;
                        do_release(victim, "");
                        command_printf(npc,
                                       "sayto %s Well done %s, you've captured %s.",
                                       victim->name, victim->race->name(),
                                       imob->short_descr);
                        act(AT_ACTION, "$n ushers $N off to be processed.",
                            npc, NULL, imob, TO_ROOM);
                        command_printf(npc,
                                       "sayto %s Here is the payment for the capture.",
                                       victim->name);
                        sprintf(buf, "$n give you %d credits.",
                                bounty->amount());
                        act(AT_ACTION, buf, npc, NULL, victim, TO_VICT);
                        victim->gold += bounty->amount();
                        gain_exp(victim, bounty->experience(),
                                 HUNTING_ABILITY);
                        ch_printf(victim,
                                  "You gain %d bounty hunting experience.",
                                  bounty->experience());
                        command_printf(npc, "mppurge %s", imob->short_descr);
                        return TRUE;
                }
        }
        return TRUE;
}

bool obj_is_contraband(OBJ_DATA * obj) 
{
        if (obj->pIndexData->item_type == ITEM_SMUT) {
                return TRUE;
        }

        if (obj->pIndexData->item_type == ITEM_DRINK_CON)
        {
                int liquid;
                if ((liquid = obj->value[2]) >= LIQ_MAX)
                        liquid = obj->value[2] = 0;
                
                if (liq_table[liquid].liq_affect[COND_DRUNK] > 0)
                {
                        // Put message here
                        return TRUE;
                }
        }

        if (obj->pIndexData->item_type == ITEM_SPICE || obj->pIndexData->item_type == ITEM_RAWSPICE)
        {
                return TRUE;
        }

        if (obj->pIndexData->item_type == ITEM_WEAPON || obj->pIndexData->item_type == ITEM_GRENADE)
        {
                return TRUE;
        }
        return FALSE;
}

SPECF spec_customs(CHAR_DATA * ch)
{
        CHAR_DATA *victim;
        CHAR_DATA *v_next;
		char *mob_clan;
        OBJ_DATA *obj,*obj_prev;
        char      buf[MAX_STRING_LENGTH];
        long      ch_exp;

        if (!IS_AWAKE(ch) || ch->position == POS_FIGHTING)
                return FALSE;

        mob_clan = ch->mob_clan;
        if (ch->in_room && ch->in_room->area->planet && ch->in_room->area->planet->governed_by)
                ch->mob_clan = ch->in_room->area->planet->governed_by->name;

        for (victim = ch->in_room->first_person; victim; victim = v_next)
        {
                v_next = victim->next_in_room;

                if (IS_NPC(victim) || victim->position == POS_FIGHTING)
                        continue;

                if (is_clan_ally(ch, victim))
                        continue;

                for (obj = victim->last_carrying; obj; obj = obj_prev)
                {
                        obj_prev = obj->prev_content;

                        if (obj->item_type == ITEM_HOLSTER && obj->first_content != NULL)
                             obj = obj->first_content;
                        
                        if (obj_is_contraband(obj)) {
                                if (victim != ch && can_see(ch, victim) && can_see_obj(ch, obj))
                                {
                                        if (ch->in_room->area->planet) {
                                                snprintf(buf, MSL,
                                                         "%s's are banned from this planet. I'm going to have to confiscate %s.",
                                                         capitalize(o_types[obj->item_type]), obj->short_descr);
                                        }
                                        else {
                                                snprintf(buf, MSL,
                                                         "%s's are banned from here. I'm going to have to confiscate %s.",
                                                         capitalize(o_types[obj->item_type]), obj->short_descr);
                                        }
                                        do_say(ch, buf);
                                        if (obj->wear_loc != WEAR_NONE)
                                                remove_obj(victim,obj->wear_loc,TRUE);
                                        separate_obj(obj);
                                        if (obj->in_obj) 
                                                obj_from_obj(obj);
                                        else 
                                                obj_from_char(obj);
                                        act(AT_ACTION,"$n confiscates $p from $N.", ch,obj, victim, TO_NOTVICT);
                                        act(AT_ACTION,"$n takes $p from you.", ch, obj,victim, TO_VICT);
                                        obj = obj_to_char(obj, ch);
                                        SET_BIT(obj->extra_flags,ITEM_CONTRABAND);
                                        ch_exp = UMIN(obj->cost * 10,(exp_level(victim->skill_level[SMUGGLING_ABILITY] +1) -exp_level(victim->skill_level[SMUGGLING_ABILITY])));
                                        ch_printf(victim,"You lose %ld experience.\n\r ",ch_exp);
                                        gain_exp(victim, 0 - ch_exp,SMUGGLING_ABILITY);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (can_see(ch, victim)&& !IS_SET(obj->extra_flags,ITEM_CONTRABAND))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMIN(obj->cost * 10,(exp_level(victim->skill_level[SMUGGLING_ABILITY] +1) -exp_level(victim->skill_level[SMUGGLING_ABILITY])));
                                        ch_printf(victim,"You receive %ld experience for smuggling %d.\n\r ",ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,SMUGGLING_ABILITY);

                                        act(AT_ACTION,"$n looks at $N suspiciously.",ch, NULL, victim, TO_NOTVICT);
                                        act(AT_ACTION,"$n look at you suspiciously.",ch, NULL, victim, TO_VICT);
                                        SET_BIT(obj->extra_flags,ITEM_CONTRABAND);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                                else if (!IS_SET(obj->extra_flags, ITEM_CONTRABAND))
                                {
                                        separate_obj(obj);
                                        ch_exp = UMIN(obj->cost * 10,(exp_level(victim->skill_level[SMUGGLING_ABILITY] +1) -exp_level(victim->skill_level[SMUGGLING_ABILITY])));
                                        ch_printf(victim,"You receive %ld experience for smuggling %s.\n\r ",ch_exp, obj->short_descr);
                                        gain_exp(victim, ch_exp,SMUGGLING_ABILITY);

                                        SET_BIT(obj->extra_flags,ITEM_CONTRABAND);
                                        ch->mob_clan = mob_clan;
                                        return TRUE;
                                }
                        }
                        else if (obj->item_type == ITEM_CONTAINER)
                        {
                                OBJ_DATA *content;

                                for (content = obj->first_content; content; content = content->next_content)
                                {
                                        if (obj_is_contraband(content) && !IS_SET(content->extra_flags, ITEM_CONTRABAND))
                                        {
                                                separate_obj(content);
                                                ch_exp = UMIN(content->cost *10,(exp_level(victim->skill_level[SMUGGLING_ABILITY] + 1) -
                                                               exp_level(victim->skill_level[SMUGGLING_ABILITY])));
                                                ch_printf(victim,
                                                          "You receive %ld experience for smuggling %s.\n\r ",
                                                          ch_exp,content->short_descr);
                                                gain_exp(victim, ch_exp,SMUGGLING_ABILITY);
                                                SET_BIT(content->extra_flags,ITEM_CONTRABAND);
                                                ch->mob_clan = mob_clan;
                                                return TRUE;
                                        }
                                }
                        }
                }

        }

        return FALSE;
}
